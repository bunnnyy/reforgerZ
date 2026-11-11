//! PC-side helper: writes every resource of the chosen extensions (ResourceName per line) to a text
//! file, so prefab bases can be picked and written into prefabs as text. Workbench only.
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
