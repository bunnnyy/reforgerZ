//! PC-side helpers for building prefabs as text (docs/PC_SETUP.md). Workbench only.

//! Writes every resource of the chosen extensions (ResourceName per line) to a text file.
[WorkbenchPluginAttribute(name: "ReforgerZ: Write resource index", description: "List every prefab (and sound, layout, texture) with its GUID in a text file", wbModules: {"WorldEditor", "ResourceManager"}, awesomeFontCode: 0xF03A)]
class RZ_ResourceIndexPlugin : WorkbenchPlugin
{
	[Attribute("$profile:RZ_ResourceIndex.txt", desc: "Where to write the index")]
	protected string m_sIndexPath;

	[Attribute("et,acp,layout,edds,imageset,conf", desc: "Extensions, comma-separated")]
	protected string m_sExtensions;

	override void Run()
	{
		array<string> extensions = {};
		m_sExtensions.Split(",", extensions, true);
		array<string> words = {};
		RZ_WBSearch results = new RZ_WBSearch();
		Workbench.SearchResources(results.OnFound, extensions, words);
		array<string> lines = {};
		foreach (ResourceName found : results.m_aFound)
		{
			lines.Insert(found);
		}
		RZ_WB.WriteLines(m_sIndexPath, lines);
		Print("[ReforgerZ] Resource index: " + lines.Count().ToString() + " resources written to " + m_sIndexPath);
	}

	override void Configure()
	{
		Workbench.ScriptDialog("ReforgerZ: Write resource index", "", this);
	}
}

//! Reads a list (one per line) of prefab ResourceNames and class names. Each prefab is saved as text
//! to the dump folder (its own values, as its .et file holds them); each class name gets a file
//! listing its properties, their types and defaults.
[WorkbenchPluginAttribute(name: "ReforgerZ: Dump prefabs and classes", description: "Save packed prefabs as text and list a class's properties", wbModules: {"WorldEditor", "ResourceManager"}, awesomeFontCode: 0xF019)]
class RZ_DumpPlugin : WorkbenchPlugin
{
	[Attribute("$profile:RZ_DumpList.txt", desc: "The list: prefab ResourceNames and class names, one per line")]
	protected string m_sListPath;

	[Attribute("$profile:RZ_Dump/", desc: "The output folder")]
	protected string m_sOutDir;

	override void Run()
	{
		FileHandle list = FileIO.OpenFile(m_sListPath, FileMode.READ);
		if (!list)
		{
			Print("[ReforgerZ] Dump: no list at " + m_sListPath, LogLevel.WARNING);
			return;
		}
		FileIO.MakeDirectory(m_sOutDir);
		array<string> entries = {};
		string line;
		while (list.ReadLine(line) >= 0)
		{
			line.Trim();
			if (!line.IsEmpty())
				entries.Insert(line);
		}
		list.Close();

		int done;
		foreach (int i, string entry : entries)
		{
			string outFile = m_sOutDir + i.ToString() + ".txt";
			if (entry.Contains("/"))
			{
				Resource res = BaseContainerTools.LoadContainer(entry);
				if (!res || !res.IsValid())
					continue;
				if (BaseContainerTools.SaveContainer(res.GetResource().ToBaseContainer(), "", outFile))
					done++;
			}
			else
			{
				Resource made = BaseContainerTools.CreateContainer(entry);
				if (!made || !made.IsValid())
					continue;
				BaseContainer cont = made.GetResource().ToBaseContainer();
				array<string> lines = {"class " + entry};
				for (int v = 0; v < cont.GetNumVars(); v++)
				{
					string varName = cont.GetVarName(v);
					string def;
					cont.GetDefaultAsString(varName, def);
					string extra = "";
					array<string> names = {};
					array<int> values = {};
					cont.GetEnumValues(v, names, values);
					if (!names.IsEmpty())
						extra = " enum: " + RZ_DumpJoin(names);
					string objClass = cont.GetObjectBaseClass(v);
					if (!objClass.IsEmpty())
						extra = extra + " object: " + objClass;
					lines.Insert(string.Format("%1 type=%2 default=%3%4", varName, typename.EnumToString(DataVarType, cont.GetDataVarType(v)), def, extra));
				}
				if (RZ_WB.WriteLines(outFile, lines))
					done++;
			}
		}
		Print("[ReforgerZ] Dump: " + done.ToString() + " of " + entries.Count().ToString() + " written to " + m_sOutDir);
	}

	static string RZ_DumpJoin(array<string> names)
	{
		string joined = "";
		foreach (int n, string s : names)
		{
			if (n > 0)
				joined = joined + ",";
			joined = joined + s;
		}
		return joined;
	}

	override void Configure()
	{
		Workbench.ScriptDialog("ReforgerZ: Dump prefabs and classes", "", this);
	}
}
