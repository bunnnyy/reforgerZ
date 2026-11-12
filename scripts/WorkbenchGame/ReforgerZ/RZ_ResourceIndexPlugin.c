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
				// The prefab, then each ancestor (i.txt, i_1.txt, i_2.txt ...), so inherited parts can be read too
				BaseContainer level = res.GetResource().ToBaseContainer();
				int depth = 0;
				while (level && depth < 10)
				{
					string levelFile = outFile;
					if (depth > 0)
						levelFile = m_sOutDir + i.ToString() + "_" + depth.ToString() + ".txt";
					BaseContainerTools.SaveContainer(level, "", levelFile);
					level = level.GetAncestor();
					depth++;
				}
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

//! Registers (imports) files listed one per line, as addon paths ("UI/Textures/ReforgerZ/X.png"),
//! building their runtime resource; with Rebuild on it rebuilds already registered ones instead
//! (after their .meta was edited, e.g. to the TextureUIAtlas preset).
[WorkbenchPluginAttribute(name: "ReforgerZ: Register or rebuild resources", description: "Import listed files (PNG and the like) or rebuild them after a .meta change", wbModules: {"ResourceManager"}, awesomeFontCode: 0xF093)]
class RZ_RegisterPlugin : WorkbenchPlugin
{
	[Attribute("$profile:RZ_RegisterList.txt", desc: "The list: addon paths, one per line")]
	protected string m_sListPath;

	[Attribute("0", desc: "Rebuild already registered files instead of registering")]
	protected bool m_bRebuild;

	override void Run()
	{
		ResourceManager rm = Workbench.GetModule(ResourceManager);
		FileHandle list = FileIO.OpenFile(m_sListPath, FileMode.READ);
		if (!rm || !list)
			return;
		string line;
		int done;
		while (list.ReadLine(line) >= 0)
		{
			line.Trim();
			if (line.IsEmpty())
				continue;
			string absPath;
			if (!RZ_WB.AbsolutePath("$ReforgerZ:" + line, absPath, true))
			{
				Print("[ReforgerZ] Register: not found " + line, LogLevel.WARNING);
				continue;
			}
			// Already registered (registering fails): rebuild it, so a changed .meta (a preset) takes effect.
			if (m_bRebuild || !rm.RegisterResourceFile(absPath, true))
			{
				foreach (string config : {"PC", "XBOX_ONE", "XBOX_SERIES", "PS4", "PS5", "HEADLESS"})
				{
					rm.RebuildResourceFile(absPath, config, false);
				}
			}
			done++;
		}
		list.Close();
		string verb = "registered";
		if (m_bRebuild)
			verb = "rebuilt";
		Print("[ReforgerZ] Register: " + done.ToString() + " files " + verb);
	}

	override void Configure()
	{
		Workbench.ScriptDialog("ReforgerZ: Register or rebuild resources", "", this);
	}
}

//! Lays out a group of compositions in the open world for a look (or a screenshot), and points the
//! camera at them. The list ($profile:RZ_ShowList.txt) holds "# Title" lines that start a group and
//! ResourceNames under them. Each run removes the last group it placed and shows the next one;
//! after the last group it clears up. Nothing is saved: don't save the world afterwards.
[WorkbenchPluginAttribute(name: "ReforgerZ: Show the next composition group", description: "Place the next group of compositions in front of the camera for a look", wbModules: {"WorldEditor"}, awesomeFontCode: 0xF06E)]
class RZ_ShowcasePlugin : WorkbenchPlugin
{
	[Attribute("$profile:RZ_ShowList.txt", desc: "The list")]
	protected string m_sListPath;

	[Attribute("6326 0 6418", desc: "Where the first piece goes (x and z; y is the ground). 0 0 0: 40 m in front of the camera when the first group is shown")]
	protected vector m_vOrigin;

	protected static ref array<IEntitySource> s_aPlaced = {};
	protected static int s_iGroup;
	protected static vector s_vStart;

	override void Run()
	{
		WorldEditorAPI api = RZ_WB.Api();
		if (!api)
			return;
		api.BeginEntityAction("ReforgerZ: Showcase");
		foreach (IEntitySource placed : s_aPlaced)
		{
			if (placed)
				api.DeleteEntity(placed);
		}
		s_aPlaced.Clear();

		array<string> items = {};
		string title = ReadGroup(s_iGroup, items);
		if (items.IsEmpty())
		{
			api.EndEntityAction();
			s_iGroup = 0;
			Print("[ReforgerZ] Showcase: done, all groups shown and cleared");
			return;
		}

		// The start: the attribute, else a spot ahead of the camera, fixed at the first group
		if (s_iGroup == 0 || s_vStart == vector.Zero)
		{
			s_vStart = m_vOrigin;
			if (s_vStart == vector.Zero)
			{
				vector cam[4];
				api.GetWorld().GetCurrentCamera(cam);
				vector fwd = cam[2];
				fwd[1] = 0;
				fwd.Normalize();
				s_vStart = cam[3] + fwd * 40;
			}
		}
		vector origin = s_vStart;
		int cols = 4;
		float spaceX = 9;
		float spaceZ = 10;
		// "# Title @3" lays the group out 3 m apart (people, small items)
		int at = title.IndexOf("@");
		if (at > 0)
		{
			spaceX = title.Substring(at + 1, title.Length() - at - 1).ToFloat();
			spaceZ = spaceX;
			cols = 6;
		}
		foreach (int i, string prefab : items)
		{
			int row = i / cols;
			int col = i - row * cols;
			float x = origin[0] + col * spaceX;
			float z = origin[2] + row * spaceZ;
			vector pos = Vector(x, api.GetTerrainSurfaceY(x, z), z);
			vector ang = vector.Zero;
			if (at > 0)
				ang = Vector(0, 180, 0);	// people face the camera
			IEntitySource src = api.CreateEntity(prefab, "", api.GetCurrentEntityLayerId(), null, pos, ang);
			if (src)
				s_aPlaced.Insert(src);
		}
		api.EndEntityAction();

		int rows = (items.Count() + cols - 1) / cols;
		float cx = origin[0] + (cols - 1) * spaceX * 0.5;
		float cz = origin[2] + (rows - 1) * spaceZ * 0.5;
		float back = 11 + rows * 6;
		float up = 5 + rows * 3;
		if (at > 0)
		{
			// people and small items: close, at chest height
			back = 4 + rows * 2.5;
			up = 1.6 + rows * 0.7;
		}
		vector eye = Vector(cx, api.GetTerrainSurfaceY(cx, cz) + up, cz - back);
		vector target = Vector(cx, api.GetTerrainSurfaceY(cx, cz), cz);
		if (at > 0)
			target[1] = target[1] + 1.0;
		api.ClearEntitySelection();
		api.SetCamera(eye, (target - eye).Normalized());
		Print(string.Format("[ReforgerZ] Showcase: group %1 '%2', %3 pieces", s_iGroup, title, items.Count()));
		s_iGroup++;
	}

	protected string ReadGroup(int wanted, notnull array<string> items)
	{
		FileHandle file = FileIO.OpenFile(m_sListPath, FileMode.READ);
		if (!file)
			return "";
		string line;
		string title = "";
		int group = -1;
		while (file.ReadLine(line) >= 0)
		{
			line.Trim();
			if (line.IsEmpty())
				continue;
			if (line.IndexOf("#") == 0)
			{
				group++;
				if (group == wanted)
					title = line.Substring(1, line.Length() - 1);
				continue;
			}
			if (group == wanted)
				items.Insert(line);
		}
		file.Close();
		return title;
	}

	override void Configure()
	{
		Workbench.ScriptDialog("ReforgerZ: Show the next composition group", "", this);
	}
}

//! Moves the World Editor camera to the next place in $profile:RZ_Goto.txt ("name x z" per line),
//! standing 30 m south of it at eye height and looking north at it, so Play from Camera Position
//! starts there. Each run takes the next line, then starts over.
[WorkbenchPluginAttribute(name: "ReforgerZ: Camera to the next place", description: "Point the camera at the next place in the list, for Play from Camera Position", wbModules: {"WorldEditor"}, awesomeFontCode: 0xF030)]
class RZ_GotoPlugin : WorkbenchPlugin
{
	[Attribute("$profile:RZ_Goto.txt", desc: "The list: name x z, one per line")]
	protected string m_sListPath;

	protected static int s_iNext;

	override void Run()
	{
		WorldEditorAPI api = RZ_WB.Api();
		FileHandle file = FileIO.OpenFile(m_sListPath, FileMode.READ);
		if (!api || !file)
			return;
		array<string> lines = {};
		string line;
		while (file.ReadLine(line) >= 0)
		{
			line.Trim();
			if (!line.IsEmpty())
				lines.Insert(line);
		}
		file.Close();
		if (lines.IsEmpty())
			return;
		if (s_iNext >= lines.Count())
			s_iNext = 0;
		array<string> bits = {};
		lines[s_iNext].Split(" ", bits, true);
		s_iNext++;
		if (bits.Count() < 3)
			return;
		float x = bits[1].ToFloat();
		float z = bits[2].ToFloat();
		vector target = Vector(x, api.GetTerrainSurfaceY(x, z) + 1.5, z);
		vector eye = Vector(x, api.GetTerrainSurfaceY(x, z - 30) + 1.8, z - 30);
		api.SetCamera(eye, (target - eye).Normalized());
		Print("[ReforgerZ] Camera: " + bits[0] + " at " + x.ToString() + " " + z.ToString());
	}

	override void Configure()
	{
		Workbench.ScriptDialog("ReforgerZ: Camera to the next place", "", this);
	}
}
