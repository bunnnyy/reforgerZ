//! ReforgerZ: Create prefabs. A World Editor plugin that reads the prefab manifest
//! (Configs/ReforgerZ/PrefabManifest.json) and, idempotently:
//!   - creates each prefab from its vanilla base (or an empty entity) at its target path,
//!     skipping prefabs that already exist;
//!   - adds the listed components and actions with their attribute values;
//!   - wires the prefabs into the RZ_GameMode prefab's settings objects (empty slots only);
//!   - places the game mode in the open world, adds the faction variants, relations and loadouts
//!     where it can, and prints exact manual steps where it can't;
//!   - reports what it created, skipped and failed (with a reason), and the steps left.
//! A dry run only reports. "ReforgerZ: Write prefab checklist" writes the same work as a
//! checklist file to follow by hand.
//!
//! Status: it compiles in Workbench 1.9 and its dry run works; a real run hasn't been tried.
//! What only a real run can show is marked VERIFY.
//! Each call sits in a small helper in RZ_WB below, so fixing one is one edit. Expect a round of fixes the first time; use the
//! dry run first, and fall back to the checklist (or docs/WORKBENCH_SETUP.md) if it fails.

//------------------------------------------------------------------------------------------------
// Manifest data. Member names match the JSON keys exactly (the JSON avoids words that are
// reserved in Enforce Script, such as "set", "class" and "auto"). Keys the plugin doesn't need
// (settings, settingsTable, topLevelDefaults) have no member and are ignored.
//------------------------------------------------------------------------------------------------
class RZ_MfPart
{
	string cls;
	ref array<string> values = {};
	string name;
}

class RZ_MfPrefab
{
	string id;
	string kind;		// "create", "external" (an existing prefab fills the slot) or "manual"
	string system;
	string path;
	string basePrefab;	// a ResourceName ("{GUID}path.et"), a plain path, or empty; lists may hold several, comma-separated
	string baseFind;	// a vanilla file name to search for when base is empty
	string baseHint;
	ref array<ref RZ_MfPart> components = {};
	ref array<ref RZ_MfPart> actions = {};
	ref array<string> slots = {};
	bool listSlot;
	string slotLabel;
	bool required;
	bool minimum;
	string use;
	string notes;
}

class RZ_MfGameMode
{
	string id;
	string path;
	string basePrefab;
	string baseFind;
	string baseHint;
	ref array<ref RZ_MfPart> components = {};
}

class RZ_MfLoadout
{
	string name;
	bool unlockable;
	string path;
	string basePrefab;
	string baseFind;
	string baseHint;
	string gear;
	ref array<ref RZ_MfPart> actions = {};
}

class RZ_MfAddition
{
	string target;
	ref array<ref RZ_MfPart> add = {};
	string why;
	bool required;
	bool minimum;
}

class RZ_MfBaseFaction
{
	string key;
	string name;
	bool playable;
	string use;
}

class RZ_MfOutlaw
{
	string key;
	bool optional;
	string rule;
}

class RZ_MfFactions
{
	string playerKey;
	ref array<ref RZ_MfBaseFaction> main = {};
	string zombieFaction;
	ref array<string> friendly = {};	// "SURV MIL": friendly both ways
	ref array<string> hostile = {};		// "MIL BAND": not in each other's friendly lists
	ref array<string> variants = {};
	string variantRule;
	ref RZ_MfOutlaw outlaw;
}

class RZ_MfWorldStep
{
	string step;
	string plugin;		// "yes" (the plugin does it), "check" (it checks), "no" (manual)
	string find;
	bool minimum;
}

class RZ_Manifest
{
	int version;
	string addon;
	ref RZ_MfGameMode gameMode;
	ref array<ref RZ_MfPrefab> prefabs = {};
	ref array<ref RZ_MfAddition> additions = {};
	ref array<ref RZ_MfLoadout> loadouts = {};
	ref RZ_MfFactions factions;
	ref array<ref RZ_MfWorldStep> world = {};
}

//------------------------------------------------------------------------------------------------
//! What happened, for the summary and the log.
class RZ_PluginReport
{
	ref array<string> m_aCreated = {};
	ref array<string> m_aSkipped = {};
	ref array<string> m_aFailed = {};
	ref array<string> m_aManual = {};
	bool m_bDryRun;

	void Created(string what)
	{
		m_aCreated.Insert(what);
		Print("[ReforgerZ] " + Verb() + what, LogLevel.NORMAL);
	}

	void Skipped(string what)
	{
		m_aSkipped.Insert(what);
		Print("[ReforgerZ] skipped: " + what, LogLevel.NORMAL);
	}

	void Failed(string what)
	{
		m_aFailed.Insert(what);
		Print("[ReforgerZ] FAILED: " + what, LogLevel.WARNING);
	}

	void Manual(string what)
	{
		m_aManual.Insert(what);
		Print("[ReforgerZ] manual: " + what, LogLevel.NORMAL);
	}

	string Verb()
	{
		if (m_bDryRun)
			return "would do: ";
		return "done: ";
	}

	string Summary()
	{
		string head = "ReforgerZ: Create prefabs";
		if (m_bDryRun)
			head = head + " (dry run: nothing was changed)";
		return string.Format("%1\n\n%2 done, %3 skipped (already there), %4 failed, %5 manual steps left.\nThe full list is in the log (search for [ReforgerZ]).",
			head, m_aCreated.Count(), m_aSkipped.Count(), m_aFailed.Count(), m_aManual.Count());
	}

	void PrintAll()
	{
		Print("[ReforgerZ] ================ summary ================", LogLevel.NORMAL);
		PrintList("Done", m_aCreated);
		PrintList("Skipped", m_aSkipped);
		PrintList("Failed", m_aFailed);
		PrintList("Manual steps left", m_aManual);
		Print("[ReforgerZ] " + Summary(), LogLevel.NORMAL);
	}

	protected void PrintList(string title, array<string> items)
	{
		Print(string.Format("[ReforgerZ] %1 (%2):", title, items.Count()), LogLevel.NORMAL);
		foreach (string s : items)
		{
			Print("[ReforgerZ]   - " + s, LogLevel.NORMAL);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! Collects resources found by a Workbench search.
class RZ_WBSearch
{
	ref array<ResourceName> m_aFound = {};

	void OnFound(ResourceName resName, string filePath = "")
	{
		m_aFound.Insert(resName);
	}
}

//------------------------------------------------------------------------------------------------
//! Every Workbench editor call, one small helper each. If an API name is wrong, fix it here.
class RZ_WB
{
	// Editor names used in prefab and world data. VERIFY: each name against a vanilla .et/.conf.
	static const string ACTIONS_MANAGER = "ActionsManagerComponent";
	static const string ACTION_CONTEXTS = "ActionContexts";
	static const string ACTION_CONTEXT_CLASS = "UserActionContext";
	static const string CONTEXT_NAME = "ContextName";
	static const string ADDITIONAL_ACTIONS = "additionalActions";
	static const string PARENT_CONTEXTS = "ParentContextList";
	static const string UI_INFO = "UIInfo";
	static const string UI_INFO_CLASS = "UIInfo";
	static const string UI_NAME = "Name";
	static const string RPL = "RplComponent";
	static const string FACTIONS_LIST = "Factions";
	static const string FACTION_CLASS = "SCR_Faction";
	static const string FACTION_KEY = "FactionKey";
	static const string FACTION_COLOR = "FactionColor";
	static const string FACTION_PLAYABLE = "m_bIsPlayable";
	static const string FACTION_FRIENDS = "m_aFriendlyFactionsIds";
	static const string LOADOUT_LIST = "m_aPlayerLoadouts";
	static const string LOADOUT_CLASS = "SCR_FactionPlayerLoadout";
	static const string LOADOUT_NAME = "m_sLoadoutName";
	static const string LOADOUT_RESOURCE = "m_sLoadoutResource";
	static const string LOADOUT_FACTION = "m_sAffiliatedFaction";

	//------------------------------------------------------------------------------------------
	static WorldEditorAPI Api()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor); // modules don't support Type.Cast
		if (!worldEditor)
			return null;
		return worldEditor.GetApi();
	}

	//------------------------------------------------------------------------------------------
	static bool AbsolutePath(string path, out string absPath, bool mustExist)
	{
		return Workbench.GetAbsolutePath(path, absPath, mustExist);
	}

	//------------------------------------------------------------------------------------------
	static bool Exists(string path)
	{
		string absPath;
		return AbsolutePath(path, absPath, true);
	}

	//------------------------------------------------------------------------------------------
	// VERIFY: FileIO.MakeDirectory works on absolute paths and on existing folders
	static void MakeFolders(string absFile)
	{
		array<string> bits = {};
		absFile.Replace("\\", "/");
		absFile.Split("/", bits, false);
		string dir;
		for (int i = 0; i < bits.Count() - 1; i++)
		{
			if (i > 0)
				dir = dir + "/";
			dir = dir + bits[i];
			if (dir.Length() > 2)
				FileIO.MakeDirectory(dir);
		}
	}

	//------------------------------------------------------------------------------------------
	static ResourceName ResourceNameOf(string path)
	{
		string absPath;
		if (!AbsolutePath(path, absPath, true))
			return "";
		ResourceManager rm = Workbench.GetModule(ResourceManager); // modules don't support Type.Cast
		if (rm)
		{
			MetaFile meta = rm.GetMetaFile(absPath);
			if (meta)
				return meta.GetResourceID();
		}
		// No meta file yet (or the call is wrong): search for it instead.
		return Find(path.Substring(path.IndexOf(":") + 1, path.Length() - path.IndexOf(":") - 1));
	}

	//------------------------------------------------------------------------------------------
	// Workbench.SearchResources is marked obsolete in 1.9 (ResourceDatabase.SearchResources replaces
	// it) but still exists with this signature.
	static void Search(string fileName, notnull RZ_WBSearch results)
	{
		array<string> extensions = {"et"};
		array<string> words = {fileName};
		Workbench.SearchResources(results.OnFound, extensions, words);
	}

	//------------------------------------------------------------------------------------------
	//! The first resource whose path ends with fileName (a file name, or a path without GUID).
	static ResourceName Find(string fileName)
	{
		RZ_WBSearch results = new RZ_WBSearch();
		Search(FileNameOf(fileName), results);
		foreach (ResourceName found : results.m_aFound)
		{
			string foundPath = found.GetPath();	// the path without the GUID
			if (EndsWith(foundPath, "/" + fileName) || foundPath == fileName)
				return found;
		}
		return "";
	}

	//------------------------------------------------------------------------------------------
	static string FileNameOf(string path)
	{
		array<string> bits = {};
		path.Split("/", bits, true);
		if (bits.IsEmpty())
			return path;
		return bits[bits.Count() - 1];
	}

	//------------------------------------------------------------------------------------------
	static bool EndsWith(string s, string tail)
	{
		int n = s.Length();
		int m = tail.Length();
		if (m > n)
			return false;
		return s.Substring(n - m, m) == tail;
	}

	//------------------------------------------------------------------------------------------
	static void Begin(WorldEditorAPI api)
	{
		api.BeginEntityAction("ReforgerZ: Create prefabs");
	}

	static void End(WorldEditorAPI api)
	{
		api.EndEntityAction();
	}

	//------------------------------------------------------------------------------------------
	static IEntitySource CreateEntity(WorldEditorAPI api, string prefabOrClass, string entityName)
	{
		return api.CreateEntity(prefabOrClass, entityName, api.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);
	}

	//------------------------------------------------------------------------------------------
	static void DeleteEntity(WorldEditorAPI api, IEntitySource src)
	{
		api.DeleteEntity(src);
	}

	//------------------------------------------------------------------------------------------
	// VERIFY: CreateEntityTemplate(src, absolute .et path) saves a new prefab that inherits from
	// the entity's prefab and writes its .meta (GUID)
	static bool SaveAsPrefab(WorldEditorAPI api, IEntitySource src, string absPath)
	{
		return api.CreateEntityTemplate(src, absPath);
	}

	//------------------------------------------------------------------------------------------
	static Resource LoadPrefab(ResourceName prefab)
	{
		return BaseContainerTools.LoadContainer(prefab);
	}

	static IEntitySource PrefabSource(Resource res)
	{
		if (!res || !res.IsValid())
			return null;
		BaseResourceObject obj = res.GetResource();
		if (!obj)
			return null;
		return obj.ToEntitySource();
	}

	//------------------------------------------------------------------------------------------
	static bool SavePrefab(IEntitySource src, ResourceName prefab)
	{
		return BaseContainerTools.SaveContainer(src, prefab);
	}

	//------------------------------------------------------------------------------------------
	//! The class name of the first component that is cls or inherits from it ("" if none).
	static string FindComponent(IEntitySource src, string cls)
	{
		typename wanted = cls.ToType();
		int count = src.GetComponentCount();
		for (int i = 0; i < count; i++)
		{
			IEntityComponentSource comp = src.GetComponent(i);
			if (!comp)
				continue;
			string name = comp.GetClassName();
			if (name == cls)
				return name;
			typename t = name.ToType();
			if (t && wanted && t.IsInherited(wanted))
				return name;
		}
		return "";
	}

	//------------------------------------------------------------------------------------------
	static IEntityComponentSource GetComponent(IEntitySource src, string exactClass)
	{
		int count = src.GetComponentCount();
		for (int i = 0; i < count; i++)
		{
			IEntityComponentSource comp = src.GetComponent(i);
			if (comp && comp.GetClassName() == exactClass)
				return comp;
		}
		return null;
	}

	//------------------------------------------------------------------------------------------
	static bool AddComponent(WorldEditorAPI api, IEntitySource src, string cls)
	{
		return api.CreateComponent(src, cls) != null;
	}

	//------------------------------------------------------------------------------------------
	static bool SetValue(WorldEditorAPI api, IEntitySource src, array<ref ContainerIdPathEntry> path, string key, string value)
	{
		return api.SetVariableValue(src, path, key, value);
	}

	//------------------------------------------------------------------------------------------
	static bool CreateObject(WorldEditorAPI api, IEntitySource src, array<ref ContainerIdPathEntry> path, string key, string cls)
	{
		return api.CreateObjectVariableMember(src, path, key, cls);
	}

	//------------------------------------------------------------------------------------------
	static bool AddArrayObject(WorldEditorAPI api, IEntitySource src, array<ref ContainerIdPathEntry> path, string key, string cls, int index)
	{
		return api.CreateObjectArrayVariableMember(src, path, key, cls, index);
	}

	//------------------------------------------------------------------------------------------
	// A string or ResourceName array is set as one comma-separated value, as vanilla plugins do.
	static bool SetStringArray(WorldEditorAPI api, IEntitySource src, array<ref ContainerIdPathEntry> path, string key, array<string> items)
	{
		string joined;
		foreach (int i, string item : items)
		{
			if (i > 0)
				joined = joined + ",";
			joined = joined + item;
		}
		return SetValue(api, src, path, key, joined);
	}

	//------------------------------------------------------------------------------------------
	static int ArrayCount(BaseContainer container, string key)
	{
		if (!container)
			return 0;
		BaseContainerList list = container.GetObjectArray(key);
		if (!list)
			return 0;
		return list.Count();
	}

	static BaseContainer ArrayItem(BaseContainer container, string key, int index)
	{
		BaseContainerList list = container.GetObjectArray(key);
		if (!list || index >= list.Count())
			return null;
		return list.Get(index);
	}

	//------------------------------------------------------------------------------------------
	static BaseContainer GetObject(BaseContainer container, string key)
	{
		if (!container)
			return null;
		return container.GetObject(key);
	}

	//------------------------------------------------------------------------------------------
	// VERIFY: BaseContainer.Get(key, string) reads a string, ResourceName, number or colour as text
	static string GetText(BaseContainer container, string key)
	{
		string value;
		if (container)
			container.Get(key, value);
		return value;
	}

	//------------------------------------------------------------------------------------------
	// VERIFY: BaseContainer.Get(key, array<string>) reads a string or ResourceName array
	static void GetTextArray(BaseContainer container, string key, notnull array<string> items)
	{
		if (container)
			container.Get(key, items);
	}

	//------------------------------------------------------------------------------------------
	static void WorldEntities(WorldEditorAPI api, notnull array<IEntitySource> entities)
	{
		int count = api.GetEditorEntityCount();
		for (int i = 0; i < count; i++)
		{
			IEntitySource src = api.GetEditorEntity(i);
			if (src)
				entities.Insert(src);
		}
	}

	//------------------------------------------------------------------------------------------
	static void Dialog(string caption, string text)
	{
		Workbench.Dialog(caption, text);
	}

	//------------------------------------------------------------------------------------------
	// Shows the object's attributes and its [ButtonAttribute] methods, and returns the pressed
	// button's result (0 = cancel), as vanilla's WorkbenchDialog_OKCancel
	static int OptionsDialog(string caption, string text, Managed options)
	{
		return Workbench.ScriptDialog(caption, text, options);
	}

	//------------------------------------------------------------------------------------------
	// JsonLoadContext reads nested script classes and arrays of them by member name, and ignores
	// JSON keys that have no member (seen in Workbench 1.9)
	static RZ_Manifest LoadManifest(string path)
	{
		JsonLoadContext ctx = new JsonLoadContext();
		if (!ctx.LoadFromFile(path))
			return null;
		RZ_Manifest mf = new RZ_Manifest();
		ctx.ReadValue("version", mf.version);
		ctx.ReadValue("addon", mf.addon);
		mf.gameMode = new RZ_MfGameMode();
		ctx.ReadValue("gameMode", mf.gameMode);
		ctx.ReadValue("prefabs", mf.prefabs);
		ctx.ReadValue("additions", mf.additions);
		ctx.ReadValue("loadouts", mf.loadouts);
		mf.factions = new RZ_MfFactions();
		ctx.ReadValue("factions", mf.factions);
		ctx.ReadValue("world", mf.world);
		return mf;
	}

	//------------------------------------------------------------------------------------------
	static bool WriteLines(string path, array<string> lines)
	{
		FileHandle file = FileIO.OpenFile(path, FileMode.WRITE);
		if (!file)
			return false;
		foreach (string line : lines)
		{
			file.WriteLine(line);
		}
		file.Close();
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! The work itself, shared by both menu entries.
class RZ_PrefabBuilder
{
	protected ref RZ_Manifest m_Manifest;
	protected ref RZ_PluginReport m_Report;
	protected WorldEditorAPI m_Api;
	protected ref map<string, ResourceName> m_mDone = new map<string, ResourceName>();	// manifest id -> prefab
	protected ref array<string> m_aNewObjects = {};	// settings objects created (or, in a dry run, to create)
	protected string m_sPrefix;
	protected bool m_bDryRun;
	protected bool m_bMinimumOnly;
	protected bool m_bEmptyWhenNoBase;

	//------------------------------------------------------------------------------------------
	void RZ_PrefabBuilder(RZ_Manifest manifest, RZ_PluginReport report, string prefix, bool dryRun, bool minimumOnly, bool emptyWhenNoBase)
	{
		m_Manifest = manifest;
		m_Report = report;
		m_sPrefix = prefix;
		m_bDryRun = dryRun;
		m_bMinimumOnly = minimumOnly;
		m_bEmptyWhenNoBase = emptyWhenNoBase;
		m_Api = RZ_WB.Api();
	}

	//------------------------------------------------------------------------------------------
	bool HasEditor()
	{
		return m_Api != null;
	}

	//------------------------------------------------------------------------------------------
	//! A prefab path inside the mod ("$ReforgerZ:Prefabs/...").
	string ModPath(string path)
	{
		return m_sPrefix + path;
	}

	//------------------------------------------------------------------------------------------
	//! The base to build from: the manifest's base, else a search for baseFind. Empty if none.
	ResourceName ResolveBase(string basePrefab, string baseFind)
	{
		if (basePrefab != "")
		{
			if (basePrefab.IndexOf("{") == 0)
				return basePrefab;
			return RZ_WB.Find(basePrefab);
		}
		if (baseFind != "")
			return RZ_WB.Find(baseFind);
		return "";
	}

	//------------------------------------------------------------------------------------------
	//! Creates one prefab with its parts, or reports why not. Returns its ResourceName (empty if
	//! not made, or in a dry run).
	ResourceName MakePrefab(string id, string path, string basePrefab, string baseFind, string baseHint, array<ref RZ_MfPart> components, array<ref RZ_MfPart> actions)
	{
		string shortName = RZ_WB.FileNameOf(path);

		if (RZ_WB.Exists(ModPath(path)))
		{
			m_Report.Skipped(shortName + " exists");
			return RZ_WB.ResourceNameOf(ModPath(path));
		}

		ResourceName baseRes = ResolveBase(basePrefab, baseFind);
		string startFrom = baseRes;
		if (baseRes == "")
		{
			if (!m_bEmptyWhenNoBase)
			{
				m_Report.Manual(string.Format("%1: pick a base (%2) and set \"basePrefab\" for %3 in the manifest, or make it by hand at %4", shortName, baseHint, id, path));
				return "";
			}
			startFrom = "GenericEntity";
			m_Report.Manual(string.Format("%1 was made from an empty entity: give it a mesh and collision (%2)", shortName, baseHint));
		}

		if (m_bDryRun)
		{
			m_Report.Created(string.Format("create %1 from %2 with %3", path, startFrom, PartsText(components, actions)));
			return "";
		}

		if (!m_Api)
		{
			m_Report.Failed(shortName + ": no World Editor API (open a world first)");
			return "";
		}

		string absPath;
		RZ_WB.AbsolutePath(ModPath(path), absPath, false);
		if (absPath == "")
		{
			m_Report.Failed(shortName + ": couldn't resolve " + ModPath(path) + " to a file path (check the Addon prefix option)");
			return "";
		}
		RZ_WB.MakeFolders(absPath);

		RZ_WB.Begin(m_Api);
		string tempName = "RZ_Temp_" + id;
		tempName.Replace(" ", "_");
		IEntitySource src = RZ_WB.CreateEntity(m_Api, startFrom, tempName);
		if (!src)
		{
			RZ_WB.End(m_Api);
			m_Report.Failed(shortName + ": couldn't place a temporary " + startFrom + " in the world");
			return "";
		}

		string problems = AddParts(src, components, actions);
		bool saved = RZ_WB.SaveAsPrefab(m_Api, src, absPath);
		RZ_WB.DeleteEntity(m_Api, src);
		RZ_WB.End(m_Api);

		if (!saved)
		{
			m_Report.Failed(shortName + ": CreateEntityTemplate failed for " + absPath);
			return "";
		}

		ResourceName made = RZ_WB.ResourceNameOf(ModPath(path));
		if (problems != "")
			m_Report.Failed(shortName + " was created, but: " + problems);
		else
			m_Report.Created(string.Format("created %1 from %2", path, startFrom));
		return made;
	}

	//------------------------------------------------------------------------------------------
	//! Adds components, then actions (with RplComponent and an actions manager when needed).
	//! Returns "" or a list of what didn't work.
	protected string AddParts(IEntitySource src, array<ref RZ_MfPart> components, array<ref RZ_MfPart> actions)
	{
		string problems;
		bool needsRpl = false;
		foreach (RZ_MfPart comp : components)
		{
			if (comp.cls.IndexOf("RZ_") == 0)
				needsRpl = true;
			string have = RZ_WB.FindComponent(src, comp.cls);
			if (have == "")
			{
				if (!RZ_WB.AddComponent(m_Api, src, comp.cls))
				{
					problems = problems + comp.cls + " not added; ";
					continue;
				}
				have = comp.cls;
			}
			array<ref ContainerIdPathEntry> compPath = {new ContainerIdPathEntry(have)};
			problems = problems + SetValues(src, compPath, comp);
		}

		if (needsRpl && RZ_WB.FindComponent(src, RZ_WB.RPL) == "")
		{
			if (!RZ_WB.AddComponent(m_Api, src, RZ_WB.RPL))
				problems = problems + "RplComponent not added; ";
		}

		if (actions.IsEmpty())
			return problems;

		string manager = RZ_WB.FindComponent(src, RZ_WB.ACTIONS_MANAGER);
		if (manager == "")
		{
			if (!RZ_WB.AddComponent(m_Api, src, RZ_WB.ACTIONS_MANAGER))
				return problems + "ActionsManagerComponent not added, so no actions; ";
			manager = RZ_WB.ACTIONS_MANAGER;
			if (RZ_WB.FindComponent(src, RZ_WB.RPL) == "")
				RZ_WB.AddComponent(m_Api, src, RZ_WB.RPL);
		}

		string context = EnsureContext(src, manager);
		if (context == "")
			problems = problems + "no action context; ";

		IEntityComponentSource managerSrc = RZ_WB.GetComponent(src, manager);
		int index = RZ_WB.ArrayCount(managerSrc, RZ_WB.ADDITIONAL_ACTIONS);
		array<ref ContainerIdPathEntry> managerPath = {new ContainerIdPathEntry(manager)};
		foreach (RZ_MfPart action : actions)
		{
			if (!RZ_WB.AddArrayObject(m_Api, src, managerPath, RZ_WB.ADDITIONAL_ACTIONS, action.cls, index))
			{
				problems = problems + action.cls + " not added; ";
				continue;
			}
			array<ref ContainerIdPathEntry> actionPath = {new ContainerIdPathEntry(manager), new ContainerIdPathEntry(RZ_WB.ADDITIONAL_ACTIONS, index)};
			if (context != "")
			{
				array<string> contexts = {context};
				RZ_WB.SetStringArray(m_Api, src, actionPath, RZ_WB.PARENT_CONTEXTS, contexts);
			}
			if (action.name != "")
			{
				if (RZ_WB.CreateObject(m_Api, src, actionPath, RZ_WB.UI_INFO, RZ_WB.UI_INFO_CLASS))
				{
					array<ref ContainerIdPathEntry> infoPath = {new ContainerIdPathEntry(manager), new ContainerIdPathEntry(RZ_WB.ADDITIONAL_ACTIONS, index), new ContainerIdPathEntry(RZ_WB.UI_INFO)};
					RZ_WB.SetValue(m_Api, src, infoPath, RZ_WB.UI_NAME, action.name);
				}
			}
			problems = problems + SetValues(src, actionPath, action);
			index++;
		}
		return problems;
	}

	//------------------------------------------------------------------------------------------
	//! The name of the actions manager's first context, creating "default" if it has none.
	protected string EnsureContext(IEntitySource src, string manager)
	{
		IEntityComponentSource managerSrc = RZ_WB.GetComponent(src, manager);
		if (RZ_WB.ArrayCount(managerSrc, RZ_WB.ACTION_CONTEXTS) > 0)
		{
			string existing = RZ_WB.GetText(RZ_WB.ArrayItem(managerSrc, RZ_WB.ACTION_CONTEXTS, 0), RZ_WB.CONTEXT_NAME);
			if (existing != "")
				return existing;
		}
		array<ref ContainerIdPathEntry> managerPath = {new ContainerIdPathEntry(manager)};
		if (!RZ_WB.AddArrayObject(m_Api, src, managerPath, RZ_WB.ACTION_CONTEXTS, RZ_WB.ACTION_CONTEXT_CLASS, 0))
			return "";
		array<ref ContainerIdPathEntry> contextPath = {new ContainerIdPathEntry(manager), new ContainerIdPathEntry(RZ_WB.ACTION_CONTEXTS, 0)};
		RZ_WB.SetValue(m_Api, src, contextPath, RZ_WB.CONTEXT_NAME, "default");
		return "default";
	}

	//------------------------------------------------------------------------------------------
	//! Sets "key=value" pairs; "@id" values become that manifest prefab's ResourceName.
	protected string SetValues(IEntitySource src, array<ref ContainerIdPathEntry> path, RZ_MfPart p)
	{
		string problems;
		foreach (string pair : p.values)
		{
			int eq = pair.IndexOf("=");
			if (eq <= 0)
				continue;
			string key = pair.Substring(0, eq);
			string value = pair.Substring(eq + 1, pair.Length() - eq - 1);
			if (value.IndexOf("@") == 0)
			{
				string refId = value.Substring(1, value.Length() - 1);
				ResourceName target;
				if (!m_mDone.Find(refId, target) || target == "")
				{
					problems = problems + string.Format("%1.%2 left empty (%3 isn't made yet); ", p.cls, key, refId);
					continue;
				}
				value = target;
			}
			if (!RZ_WB.SetValue(m_Api, src, path, key, value))
				problems = problems + string.Format("%1.%2 not set; ", p.cls, key);
		}
		return problems;
	}

	//------------------------------------------------------------------------------------------
	static string PartsText(array<ref RZ_MfPart> components, array<ref RZ_MfPart> actions)
	{
		string text;
		foreach (RZ_MfPart c : components)
		{
			text = text + c.cls + " ";
		}
		foreach (RZ_MfPart a : actions)
		{
			text = text + a.cls + " ";
		}
		if (text == "")
			return "no parts";
		return text;
	}

	//------------------------------------------------------------------------------------------
	//! Step 1: every prefab entry, in manifest order.
	void Prefabs()
	{
		foreach (RZ_MfPrefab e : m_Manifest.prefabs)
		{
			if (m_bMinimumOnly && !e.minimum)
				continue;

			if (e.kind == "create")
			{
				ResourceName made = MakePrefab(e.id, e.path, e.basePrefab, e.baseFind, e.baseHint, e.components, e.actions);
				if (made != "")
					m_mDone.Insert(e.id, made);
				if (e.notes != "")
					m_Report.Manual(e.path + ": " + e.notes);
			}
			else if (e.kind == "external")
			{
				ResourceName found;
				if (!e.listSlot)
					found = ResolveBase(e.basePrefab, e.baseFind);
				else if (e.basePrefab != "")
					found = e.basePrefab;
				if (found != "")
					m_mDone.Insert(e.id, found);
				else
					m_Report.Manual(string.Format("%1: fill it with %2 (%3)", e.slotLabel, e.baseHint, e.use));
			}
			else
			{
				m_Report.Manual(string.Format("%1: %2 (%3)", e.slotLabel, e.baseHint, e.use));
			}
		}
	}

	//------------------------------------------------------------------------------------------
	//! Step 2: the loadout characters, each with RZ_TreatOtherAction.
	void Loadouts()
	{
		foreach (RZ_MfLoadout l : m_Manifest.loadouts)
		{
			array<ref RZ_MfPart> none = {};
			ResourceName made = MakePrefab("Loadout " + l.name, l.path, l.basePrefab, l.baseFind, l.baseHint, none, l.actions);
			if (made != "")
				m_mDone.Insert("Loadout " + l.name, made);
			m_Report.Manual(string.Format("%1: give it the %2 gear (%3) in its inventory", l.path, l.name, l.gear));
		}
	}

	//------------------------------------------------------------------------------------------
	//! Step 3: the game mode prefab, then its slots.
	ResourceName GameMode()
	{
		RZ_MfGameMode gm = m_Manifest.gameMode;
		array<ref RZ_MfPart> none = {};
		ResourceName made = MakePrefab(gm.id, gm.path, gm.basePrefab, gm.baseFind, gm.baseHint, gm.components, none);
		if (made == "")
		{
			if (!m_bDryRun)
				m_Report.Failed("RZ_GameMode.et isn't there, so no slots were wired");
			WireReport();
			return made;
		}
		Wire(made);
		return made;
	}

	//------------------------------------------------------------------------------------------
	//! Dry run (or no game mode): list what would be wired.
	protected void WireReport()
	{
		foreach (RZ_MfPrefab e : m_Manifest.prefabs)
		{
			if (m_bMinimumOnly && !e.minimum)
				continue;
			if (e.kind == "manual" || (e.kind == "external" && !m_mDone.Contains(e.id)))
				continue;
			foreach (string slot : e.slots)
			{
				if (m_bDryRun)
					m_Report.Created(string.Format("wire %1 into %2", e.id, slot));
			}
		}
	}

	//------------------------------------------------------------------------------------------
	//! Fills each empty slot of the game mode prefab's RZ_GameModeComponent.
	// VERIFY: the WorldEditorAPI edits (SetVariableValue, CreateObjectVariableMember) also work on a
	// prefab's entity source loaded from its file, not only on world entities. If not: place the
	// prefab, wire the instance the same way, and apply the instance's changes to the prefab by hand.
	protected void Wire(ResourceName gmPrefab)
	{
		Resource res = RZ_WB.LoadPrefab(gmPrefab);
		IEntitySource src = RZ_WB.PrefabSource(res);
		if (!src)
		{
			m_Report.Failed("couldn't open " + gmPrefab.GetPath() + " to wire its slots");
			return;
		}
		if (!m_bDryRun && !m_Api)
		{
			m_Report.Failed("no World Editor API to wire " + gmPrefab.GetPath());
			return;
		}
		IEntityComponentSource comp = RZ_WB.GetComponent(src, "RZ_GameModeComponent");
		if (!comp)
		{
			m_Report.Failed(gmPrefab.GetPath() + " has no RZ_GameModeComponent");
			return;
		}

		int changed = 0;
		if (!m_bDryRun && m_Api)
			RZ_WB.Begin(m_Api);

		foreach (RZ_MfPrefab e : m_Manifest.prefabs)
		{
			if (m_bMinimumOnly && !e.minimum)
				continue;
			if (e.kind == "manual")
				continue;
			ResourceName value;
			if (!m_mDone.Find(e.id, value) || value == "")
				continue;
			foreach (string slot : e.slots)
			{
				if (WireSlot(src, comp, slot, value, e.listSlot))
					changed++;
			}
		}

		if (!m_bDryRun && m_Api)
			RZ_WB.End(m_Api);

		if (changed > 0 && !m_bDryRun)
		{
			if (RZ_WB.SavePrefab(src, gmPrefab))
				m_Report.Created(string.Format("saved %1 with %2 slots filled", gmPrefab.GetPath(), changed));
			else
				m_Report.Failed("couldn't save " + gmPrefab.GetPath() + " after wiring (fill the slots by hand)");
		}
	}

	//------------------------------------------------------------------------------------------
	//! One slot: "m_sField" or "m_SettingsObject.m_sField". True if it was (or would be) set.
	protected bool WireSlot(IEntitySource src, IEntityComponentSource comp, string slot, ResourceName value, bool isList)
	{
		if (slot.Contains("[]") || slot.IndexOf("RZ_") == 0)
			return false;

		array<string> bits = {};
		slot.Split(".", bits, true);
		BaseContainer holder = comp;
		array<ref ContainerIdPathEntry> path = {new ContainerIdPathEntry("RZ_GameModeComponent")};
		string field = bits[0];
		if (bits.Count() == 2)
		{
			string objectField = bits[0];
			field = bits[1];
			holder = RZ_WB.GetObject(comp, objectField);
			if (!holder)
			{
				string cls = SettingsClass(objectField);
				if (m_bDryRun)
				{
					if (!m_aNewObjects.Contains(objectField))
						m_Report.Created(string.Format("create the %1 settings object (%2)", objectField, cls));
					m_aNewObjects.Insert(objectField);
				}
				else if (!m_Api || !RZ_WB.CreateObject(m_Api, src, path, objectField, cls))
				{
					m_Report.Failed(string.Format("couldn't create %1 (%2) on the game mode: fill %3 by hand", objectField, cls, slot));
					return false;
				}
				holder = RZ_WB.GetObject(comp, objectField);
			}
			path.Insert(new ContainerIdPathEntry(objectField));
		}

		if (isList)
		{
			array<string> current = {};
			RZ_WB.GetTextArray(holder, field, current);
			if (!current.IsEmpty())
			{
				m_Report.Skipped(slot + " already has prefabs");
				return false;
			}
		}
		else if (RZ_WB.GetText(holder, field) != "")
		{
			m_Report.Skipped(slot + " is already set");
			return false;
		}

		if (m_bDryRun)
		{
			m_Report.Created(string.Format("set %1 = %2", slot, value));
			return true;
		}

		bool ok;
		if (isList)
		{
			array<string> items = {};
			string listText = value;
			listText.Split(",", items, true);
			ok = RZ_WB.SetStringArray(m_Api, src, path, field, items);
		}
		else
		{
			ok = RZ_WB.SetValue(m_Api, src, path, field, value);
		}
		if (!ok)
		{
			m_Report.Failed(string.Format("couldn't set %1: set it by hand to %2", slot, value));
			return false;
		}
		m_Report.Created(string.Format("set %1", slot));
		return true;
	}

	//------------------------------------------------------------------------------------------
	//! "m_ZombieSettings" -> "RZ_ZombieSettings" (the settings objects follow this naming).
	static string SettingsClass(string field)
	{
		if (field.IndexOf("m_") == 0)
			return "RZ_" + field.Substring(2, field.Length() - 2);
		return field;
	}

	//------------------------------------------------------------------------------------------
	//! Step 4: the open world. Places the game mode, checks the managers.
	void World(ResourceName gmPrefab)
	{
		if (!m_Api)
		{
			m_Report.Manual("Open your Arland sub-scene in the World Editor and run the plugin again to set up the world");
			return;
		}

		array<IEntitySource> entities = {};
		RZ_WB.WorldEntities(m_Api, entities);
		bool hasMode = false;
		bool hasFactions = false;
		bool hasLoadouts = false;
		foreach (IEntitySource ent : entities)
		{
			if (RZ_WB.FindComponent(ent, "RZ_GameModeComponent") != "")
				hasMode = true;
			string cn = ent.GetClassName();
			if (cn.Contains("FactionManager"))
				hasFactions = true;
			if (cn.Contains("LoadoutManager"))
				hasLoadouts = true;
		}

		if (hasMode)
		{
			m_Report.Skipped("the world already has a game mode with RZ_GameModeComponent");
		}
		else if (m_bDryRun)
		{
			m_Report.Created("place RZ_GameMode.et in the open world");
		}
		else if (gmPrefab == "")
		{
			m_Report.Manual("Place RZ_GameMode.et in the world once it exists");
		}
		else
		{
			RZ_WB.Begin(m_Api);
			IEntitySource placed = RZ_WB.CreateEntity(m_Api, gmPrefab, "RZ_GameMode");
			RZ_WB.End(m_Api);
			if (placed)
				m_Report.Created("placed RZ_GameMode.et in the open world");
			else
				m_Report.Failed("couldn't place RZ_GameMode.et: drag it into the world by hand");
		}

		if (!hasFactions)
			m_Report.Manual("Place a vanilla FactionManager prefab in the world, then run the plugin again for the factions");
		if (!hasLoadouts)
			m_Report.Manual("Place a vanilla LoadoutManager prefab in the world, then run the plugin again for the loadouts");

		foreach (RZ_MfWorldStep w : m_Manifest.world)
		{
			if (w.plugin == "no" && (!m_bMinimumOnly || w.minimum))
				m_Report.Manual(w.step);
		}
		m_Report.Manual("Save the world (Ctrl+S) after the plugin has changed it");
	}

	//------------------------------------------------------------------------------------------
	//! Step 5: the faction variants and relations in the world's FactionManager.
	void Factions()
	{
		RZ_MfFactions f = m_Manifest.factions;
		if (!f || !m_Api)
		{
			FactionSteps("no World Editor API");
			return;
		}

		IEntitySource manager = FindWorldEntity("FactionManager");
		if (!manager)
		{
			FactionSteps("no FactionManager in the open world");
			return;
		}

		// VERIFY: the factions are an object array on the FactionManager entity itself (not only in a
		// referenced .conf); if they live in a .conf, open it and add the variants there by hand.
		map<string, int> index = new map<string, int>();
		int count = RZ_WB.ArrayCount(manager, RZ_WB.FACTIONS_LIST);
		for (int i = 0; i < count; i++)
		{
			string key = RZ_WB.GetText(RZ_WB.ArrayItem(manager, RZ_WB.FACTIONS_LIST, i), RZ_WB.FACTION_KEY);
			if (key != "")
				index.Insert(key, i);
		}
		if (index.IsEmpty() && count > 0)
		{
			FactionSteps("couldn't read the faction keys");
			return;
		}

		foreach (RZ_MfBaseFaction bf : f.main)
		{
			if (!index.Contains(bf.key))
				m_Report.Manual(string.Format("Add the faction %1 (%2) to the FactionManager: %3", bf.key, bf.name, bf.use));
		}
		m_Report.Manual("Make the zombie faction hostile to SURV, MIL, BAND and INST: " + f.zombieFaction);

		int playerIndex;
		if (!index.Find(f.playerKey, playerIndex))
		{
			FactionSteps("no " + f.playerKey + " faction yet to copy");
			return;
		}

		// Hidden variants: copies of the player faction with only the key changed.
		BaseContainer player = RZ_WB.ArrayItem(manager, RZ_WB.FACTIONS_LIST, playerIndex);
		string color = RZ_WB.GetText(player, RZ_WB.FACTION_COLOR);
		BaseContainer playerInfo = RZ_WB.GetObject(player, RZ_WB.UI_INFO);
		string infoClass;
		string infoName;
		if (playerInfo)
		{
			infoClass = playerInfo.GetClassName();
			infoName = RZ_WB.GetText(playerInfo, RZ_WB.UI_NAME);
		}

		array<string> variants = {};
		variants.Copy(f.variants);
		if (f.outlaw && f.outlaw.key != "")
			variants.Insert(f.outlaw.key);

		if (!m_bDryRun)
			RZ_WB.Begin(m_Api);
		int next = count;
		array<ref ContainerIdPathEntry> managerPath = {};
		foreach (string variant : variants)
		{
			if (index.Contains(variant))
			{
				m_Report.Skipped("faction " + variant + " exists");
				continue;
			}
			if (m_bDryRun)
			{
				m_Report.Created("add the faction " + variant + " (a copy of " + f.playerKey + ", not playable)");
				continue;
			}
			if (!RZ_WB.AddArrayObject(m_Api, manager, managerPath, RZ_WB.FACTIONS_LIST, RZ_WB.FACTION_CLASS, next))
			{
				m_Report.Failed("couldn't add the faction " + variant + ": " + f.variantRule);
				continue;
			}
			array<ref ContainerIdPathEntry> facPath = {new ContainerIdPathEntry(RZ_WB.FACTIONS_LIST, next)};
			RZ_WB.SetValue(m_Api, manager, facPath, RZ_WB.FACTION_KEY, variant);
			RZ_WB.SetValue(m_Api, manager, facPath, RZ_WB.FACTION_PLAYABLE, "0");
			if (color != "")
				RZ_WB.SetValue(m_Api, manager, facPath, RZ_WB.FACTION_COLOR, color);
			if (infoClass != "" && RZ_WB.CreateObject(m_Api, manager, facPath, RZ_WB.UI_INFO, infoClass))
			{
				array<ref ContainerIdPathEntry> infoPath = {new ContainerIdPathEntry(RZ_WB.FACTIONS_LIST, next), new ContainerIdPathEntry(RZ_WB.UI_INFO)};
				RZ_WB.SetValue(m_Api, manager, infoPath, RZ_WB.UI_NAME, infoName);
			}
			index.Insert(variant, next);
			next++;
			m_Report.Created("added the faction " + variant);
		}
		if (!m_bDryRun)
			m_Report.Manual(string.Format("Check the new %1_H factions look exactly like %1 (flag, icon, identity) and have empty loadout lists", f.playerKey));
		if (f.outlaw && f.outlaw.key != "")
			m_Report.Manual(f.outlaw.key + " (only for Hostility > Mode = Outlaw): " + f.outlaw.rule);

		Relations(manager, index, f);

		if (!m_bDryRun)
			RZ_WB.End(m_Api);
	}

	//------------------------------------------------------------------------------------------
	//! Adds missing friendly keys both ways; reports hostile pairs that are friendly.
	protected void Relations(IEntitySource manager, map<string, int> index, RZ_MfFactions f)
	{
		foreach (string pair : f.friendly)
		{
			array<string> two = {};
			pair.Split(" ", two, true);
			if (two.Count() != 2)
				continue;
			MakeFriendly(manager, index, two[0], two[1]);
			MakeFriendly(manager, index, two[1], two[0]);
		}
		foreach (string hostilePair : f.hostile)
		{
			array<string> foes = {};
			hostilePair.Split(" ", foes, true);
			if (foes.Count() != 2)
				continue;
			if (IsFriendly(manager, index, foes[0], foes[1]) || IsFriendly(manager, index, foes[1], foes[0]))
				m_Report.Manual(string.Format("%1 and %2 must be hostile: remove each from the other's friendly list", foes[0], foes[1]));
		}
	}

	//------------------------------------------------------------------------------------------
	protected bool IsFriendly(IEntitySource manager, map<string, int> index, string a, string b)
	{
		int i;
		if (!index.Find(a, i))
			return false;
		array<string> friends = {};
		RZ_WB.GetTextArray(RZ_WB.ArrayItem(manager, RZ_WB.FACTIONS_LIST, i), RZ_WB.FACTION_FRIENDS, friends);
		return friends.Contains(b);
	}

	//------------------------------------------------------------------------------------------
	protected void MakeFriendly(IEntitySource manager, map<string, int> index, string a, string b)
	{
		int i;
		if (!index.Find(a, i) || !index.Contains(b))
		{
			m_Report.Manual(string.Format("Make %1 friendly with %2 (both ways) once both factions exist", a, b));
			return;
		}
		array<string> friends = {};
		RZ_WB.GetTextArray(RZ_WB.ArrayItem(manager, RZ_WB.FACTIONS_LIST, i), RZ_WB.FACTION_FRIENDS, friends);
		if (friends.Contains(b))
		{
			m_Report.Skipped(a + " is already friendly with " + b);
			return;
		}
		if (m_bDryRun)
		{
			m_Report.Created(string.Format("add %1 to %2's friendly list", b, a));
			return;
		}
		friends.Insert(b);
		array<ref ContainerIdPathEntry> facPath = {new ContainerIdPathEntry(RZ_WB.FACTIONS_LIST, i)};
		if (RZ_WB.SetStringArray(m_Api, manager, facPath, RZ_WB.FACTION_FRIENDS, friends))
			m_Report.Created(string.Format("added %1 to %2's friendly list", b, a));
		else
			m_Report.Failed(string.Format("couldn't edit %1's friendly list: add %2 by hand", a, b));
	}

	//------------------------------------------------------------------------------------------
	//! The whole faction setup as manual steps.
	protected void FactionSteps(string why)
	{
		RZ_MfFactions f = m_Manifest.factions;
		m_Report.Manual("Factions by hand (" + why + "): see docs/WORKBENCH_SETUP.md, Factions and loadouts");
		if (!f)
			return;
		foreach (RZ_MfBaseFaction bf : f.main)
		{
			m_Report.Manual(string.Format("Faction %1 (%2): %3", bf.key, bf.name, bf.use));
		}
		foreach (string pair : f.friendly)
		{
			m_Report.Manual("Friendly both ways: " + pair);
		}
		foreach (string hostilePair : f.hostile)
		{
			m_Report.Manual("Hostile (not in each other's friendly lists): " + hostilePair);
		}
		m_Report.Manual(string.Format("Variants %1_H1..%1_H15: %2", f.playerKey, f.variantRule));
		if (f.outlaw)
			m_Report.Manual(f.outlaw.key + " (optional): " + f.outlaw.rule);
	}

	//------------------------------------------------------------------------------------------
	//! Step 6: the 13 loadouts in the world's LoadoutManager, on the player faction.
	void LoadoutEntries()
	{
		string faction = "SURV";
		if (m_Manifest.factions)
			faction = m_Manifest.factions.playerKey;

		IEntitySource manager;
		if (m_Api)
			manager = FindWorldEntity("LoadoutManager");
		if (!manager)
		{
			foreach (RZ_MfLoadout l : m_Manifest.loadouts)
			{
				m_Report.Manual(string.Format("LoadoutManager: a loadout named exactly \"%1\" on %2 using %3", l.name, faction, l.path));
			}
			return;
		}

		// VERIFY: the loadouts are an object array on the LoadoutManager entity (SCR_LoadoutManager)
		array<string> have = {};
		int count = RZ_WB.ArrayCount(manager, RZ_WB.LOADOUT_LIST);
		for (int i = 0; i < count; i++)
		{
			have.Insert(RZ_WB.GetText(RZ_WB.ArrayItem(manager, RZ_WB.LOADOUT_LIST, i), RZ_WB.LOADOUT_NAME));
		}

		if (!m_bDryRun)
			RZ_WB.Begin(m_Api);
		int next = count;
		array<ref ContainerIdPathEntry> root = {};
		foreach (RZ_MfLoadout lo : m_Manifest.loadouts)
		{
			if (have.Contains(lo.name))
			{
				m_Report.Skipped("loadout " + lo.name + " exists");
				continue;
			}
			ResourceName character;
			m_mDone.Find("Loadout " + lo.name, character);
			if (m_bDryRun)
			{
				m_Report.Created(string.Format("add the loadout \"%1\" on %2", lo.name, faction));
				continue;
			}
			if (character == "")
			{
				m_Report.Manual(string.Format("LoadoutManager: add \"%1\" on %2 once %3 exists", lo.name, faction, lo.path));
				continue;
			}
			if (!RZ_WB.AddArrayObject(m_Api, manager, root, RZ_WB.LOADOUT_LIST, RZ_WB.LOADOUT_CLASS, next))
			{
				m_Report.Failed(string.Format("couldn't add the loadout \"%1\": add it by hand (%2 on %3)", lo.name, lo.path, faction));
				continue;
			}
			array<ref ContainerIdPathEntry> entry = {new ContainerIdPathEntry(RZ_WB.LOADOUT_LIST, next)};
			RZ_WB.SetValue(m_Api, manager, entry, RZ_WB.LOADOUT_NAME, lo.name);
			RZ_WB.SetValue(m_Api, manager, entry, RZ_WB.LOADOUT_RESOURCE, character);
			RZ_WB.SetValue(m_Api, manager, entry, RZ_WB.LOADOUT_FACTION, faction);
			next++;
			m_Report.Created(string.Format("added the loadout \"%1\"", lo.name));
		}
		if (!m_bDryRun)
			RZ_WB.End(m_Api);
	}

	//------------------------------------------------------------------------------------------
	//! Step 7: changes to prefabs the plugin doesn't own.
	void Additions()
	{
		foreach (RZ_MfAddition a : m_Manifest.additions)
		{
			if (m_bMinimumOnly && !a.minimum)
				continue;
			array<ref RZ_MfPart> none = {};
			m_Report.Manual(string.Format("%1: add %2(%3)", a.target, PartsText(none, a.add), a.why));
		}
	}

	//------------------------------------------------------------------------------------------
	protected IEntitySource FindWorldEntity(string classPart)
	{
		array<IEntitySource> entities = {};
		RZ_WB.WorldEntities(m_Api, entities);
		foreach (IEntitySource ent : entities)
		{
			string cn = ent.GetClassName();
			if (cn.Contains(classPart))
				return ent;
		}
		return null;
	}

	//------------------------------------------------------------------------------------------
	//! The checklist: the same work as lines to tick off, with what exists already ticked.
	void Checklist(notnull array<string> lines)
	{
		lines.Insert("# ReforgerZ prefab checklist");
		lines.Insert("");
		lines.Insert("Written by \"ReforgerZ: Write prefab checklist\" from Configs/ReforgerZ/PrefabManifest.json.");
		lines.Insert("Ticked lines already exist. Details and the doorway blocker recipe: docs/WORKBENCH_SETUP.md.");
		lines.Insert("");
		lines.Insert("## World");
		foreach (RZ_MfWorldStep w : m_Manifest.world)
		{
			lines.Insert("- [ ] " + w.step);
		}
		lines.Insert("");
		lines.Insert("## Prefabs");
		string system;
		foreach (RZ_MfPrefab e : m_Manifest.prefabs)
		{
			if (m_bMinimumOnly && !e.minimum)
				continue;
			if (e.system != system)
			{
				system = e.system;
				lines.Insert("");
				lines.Insert("### " + system);
			}
			string tick = "- [ ] ";
			string what;
			if (e.kind == "create")
			{
				if (RZ_WB.Exists(ModPath(e.path)))
					tick = "- [x] ";
				what = string.Format("%1 from %2. Add: %3. Slot: %4", e.path, e.baseHint, PartsDetail(e.components, e.actions), e.slotLabel);
			}
			else
			{
				what = string.Format("%1: %2", e.slotLabel, e.baseHint);
			}
			if (e.minimum)
				what = what + " (minimum)";
			lines.Insert(tick + what);
		}
		lines.Insert("");
		lines.Insert("## Game mode");
		RZ_MfGameMode gm = m_Manifest.gameMode;
		string gmTick = "- [ ] ";
		if (RZ_WB.Exists(ModPath(gm.path)))
			gmTick = "- [x] ";
		lines.Insert(gmTick + gm.path + " from " + gm.baseHint + " with RZ_GameModeComponent; fill the slots above");
		lines.Insert("");
		lines.Insert("## Loadouts (on SURV, named exactly)");
		foreach (RZ_MfLoadout l : m_Manifest.loadouts)
		{
			string lTick = "- [ ] ";
			if (RZ_WB.Exists(ModPath(l.path)))
				lTick = "- [x] ";
			lines.Insert(string.Format("%1\"%2\": %3 (a civilian character with RZ_TreatOtherAction); gear: %4", lTick, l.name, l.path, l.gear));
		}
		lines.Insert("");
		lines.Insert("## Factions");
		if (m_Manifest.factions)
		{
			RZ_MfFactions f = m_Manifest.factions;
			foreach (RZ_MfBaseFaction bf : f.main)
			{
				lines.Insert(string.Format("- [ ] %1 (%2): %3", bf.key, bf.name, bf.use));
			}
			foreach (string pair : f.friendly)
			{
				lines.Insert("- [ ] friendly both ways: " + pair);
			}
			foreach (string hostilePair : f.hostile)
			{
				lines.Insert("- [ ] hostile: " + hostilePair);
			}
			lines.Insert(string.Format("- [ ] %1_H1 to %1_H15: %2", f.playerKey, f.variantRule));
			if (f.outlaw)
				lines.Insert("- [ ] optional " + f.outlaw.key + ": " + f.outlaw.rule);
			lines.Insert("- [ ] " + f.zombieFaction);
		}
		lines.Insert("");
		lines.Insert("## Add to prefabs you already have");
		foreach (RZ_MfAddition a : m_Manifest.additions)
		{
			array<ref RZ_MfPart> none = {};
			lines.Insert(string.Format("- [ ] %1: %2(%3)", a.target, PartsDetail(none, a.add), a.why));
		}
	}

	//------------------------------------------------------------------------------------------
	static string PartsDetail(array<ref RZ_MfPart> components, array<ref RZ_MfPart> actions)
	{
		array<ref RZ_MfPart> all = {};
		foreach (RZ_MfPart c : components)
		{
			all.Insert(c);
		}
		foreach (RZ_MfPart a : actions)
		{
			all.Insert(a);
		}
		string text;
		foreach (RZ_MfPart p : all)
		{
			text = text + p.cls;
			if (!p.values.IsEmpty())
			{
				string joined;
				foreach (int i, string v : p.values)
				{
					if (i > 0)
						joined = joined + ", ";
					joined = joined + v;
				}
				text = text + " (" + joined + ")";
			}
			text = text + "; ";
		}
		if (text == "")
			return "nothing";
		return text;
	}
}

//------------------------------------------------------------------------------------------------
//! Options shared by both menu entries.
class RZ_PrefabPluginBase : WorkbenchPlugin
{
	[Attribute("$ReforgerZ:Configs/ReforgerZ/PrefabManifest.json", desc: "The manifest file")]
	protected string m_sManifestPath;

	// "$ReforgerZ:" is this addon's file system name (the ID in addon.gproj)
	[Attribute("$ReforgerZ:", desc: "File system prefix of this mod: new prefabs are saved under it")]
	protected string m_sAddonPrefix;

	[Attribute("0", UIWidgets.CheckBox, "Only the minimum playable subset")]
	protected bool m_bMinimumOnly;

	//------------------------------------------------------------------------------------------
	protected RZ_Manifest Load()
	{
		RZ_Manifest mf = RZ_WB.LoadManifest(m_sManifestPath);
		if (!mf || !mf.prefabs || mf.prefabs.IsEmpty() || !mf.gameMode)
		{
			RZ_WB.Dialog("ReforgerZ", "Couldn't read " + m_sManifestPath + ".\nCheck the path in the plugin options, or follow docs/WORKBENCH_SETUP.md by hand.");
			return null;
		}
		return mf;
	}
}

//------------------------------------------------------------------------------------------------
[WorkbenchPluginAttribute(name: "ReforgerZ: Create prefabs", description: "Create the ReforgerZ prefabs from the manifest and wire them into the game mode", wbModules: {"WorldEditor"}, awesomeFontCode: 0xF1B3)]
class RZ_CreatePrefabsPlugin : RZ_PrefabPluginBase
{
	[Attribute("1", UIWidgets.CheckBox, "Dry run: only report what would be done (turn off to make changes)")]
	protected bool m_bDryRun;

	[Attribute("0", UIWidgets.CheckBox, "Make prefabs with no known base from an empty entity (you add the mesh later)")]
	protected bool m_bEmptyWhenNoBase;

	[Attribute("1", UIWidgets.CheckBox, "Set up the open world: place the game mode, add the faction variants, relations and loadouts")]
	protected bool m_bWorld;

	//------------------------------------------------------------------------------------------
	override void Run()
	{
		if (RZ_WB.OptionsDialog("ReforgerZ: Create prefabs", "Open your Arland sub-scene first. Start with a dry run.", this) == 0)
			return;

		RZ_Manifest mf = Load();
		if (!mf)
			return;

		RZ_PluginReport report = new RZ_PluginReport();
		report.m_bDryRun = m_bDryRun;
		RZ_PrefabBuilder builder = new RZ_PrefabBuilder(mf, report, m_sAddonPrefix, m_bDryRun, m_bMinimumOnly, m_bEmptyWhenNoBase);
		if (!builder.HasEditor())
		{
			if (!m_bDryRun)
			{
				RZ_WB.Dialog("ReforgerZ", "No World Editor API. Open your Arland sub-scene in the World Editor and run this from its Plugins menu.\nA dry run works without it.");
				return;
			}
			report.Failed("no World Editor API: open a world in the World Editor and run this from its Plugins menu");
		}

		builder.Prefabs();
		builder.Loadouts();
		ResourceName gm = builder.GameMode();
		if (m_bWorld)
		{
			builder.World(gm);
			builder.Factions();
			builder.LoadoutEntries();
		}
		builder.Additions();

		report.PrintAll();
		RZ_WB.Dialog("ReforgerZ", report.Summary());
	}

	//------------------------------------------------------------------------------------------
	override void Configure()
	{
		RZ_WB.OptionsDialog("ReforgerZ: Create prefabs", "", this);
	}

	//------------------------------------------------------------------------------------------
	[ButtonAttribute("Run", true)]
	protected int ButtonRun()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected int ButtonCancel()
	{
		return 0;
	}
}

//------------------------------------------------------------------------------------------------
//! The fallback: writes the manifest as a checklist file to follow by hand.
[WorkbenchPluginAttribute(name: "ReforgerZ: Write prefab checklist", description: "Write the ReforgerZ prefab manifest as a checklist file to follow by hand", wbModules: {"WorldEditor", "ResourceManager"}, awesomeFontCode: 0xF0AE)]
class RZ_PrefabChecklistPlugin : RZ_PrefabPluginBase
{
	[Attribute("$ReforgerZ:PrefabChecklist.md", desc: "Where to write the checklist (git ignores this file)")]
	protected string m_sChecklistPath;

	//------------------------------------------------------------------------------------------
	override void Run()
	{
		if (RZ_WB.OptionsDialog("ReforgerZ: Write prefab checklist", "", this) == 0)
			return;

		RZ_Manifest mf = Load();
		if (!mf)
			return;

		RZ_PluginReport report = new RZ_PluginReport();
		report.m_bDryRun = true;
		RZ_PrefabBuilder builder = new RZ_PrefabBuilder(mf, report, m_sAddonPrefix, true, m_bMinimumOnly, false);
		array<string> lines = {};
		builder.Checklist(lines);

		string absPath;
		RZ_WB.AbsolutePath(m_sChecklistPath, absPath, false);
		if (absPath == "")
			absPath = m_sChecklistPath;
		if (RZ_WB.WriteLines(absPath, lines))
			RZ_WB.Dialog("ReforgerZ", string.Format("Wrote %1 lines to %2", lines.Count(), absPath));
		else
			RZ_WB.Dialog("ReforgerZ", "Couldn't write " + absPath + ". Follow docs/WORKBENCH_SETUP.md instead.");
	}

	//------------------------------------------------------------------------------------------
	override void Configure()
	{
		RZ_WB.OptionsDialog("ReforgerZ: Write prefab checklist", "", this);
	}

	//------------------------------------------------------------------------------------------
	[ButtonAttribute("Write", true)]
	protected int ButtonWrite()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected int ButtonCancel()
	{
		return 0;
	}
}
