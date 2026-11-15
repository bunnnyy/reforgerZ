//! Loot rules, locks and keys, readable things, and what the bodies of infected killed near
//! players carry (docs/DESIGN.md, "World interactions"). Loot in houses lies loose (RZ_Loot.c)
//! and is rolled here (PickHouseLoot), as is what's in a wreck's boot (PickBootLoot, for
//! RZ_WreckBoots.c); bodies are looted with vanilla's inventory (OnZombiesKilled). There are no
//! search prompts left.
//!
//! - Loot suits the building: sixteen kinds from the prefab name of the building (BuildingType:
//!   house, shop, farm, pharmacy, hospital, garage, fire station, police, school, office, church,
//!   industrial, hunting, coast, prison, army post). Tables are configurable per kind; anything
//!   left empty is filled from one flat loot pool by keywords (bandage words for pharmacies, tool
//!   and fuel words for garages...) plus the built-in vanilla items for its kind (RZ_LootDefaults),
//!   so a table is wide and no kind is bare when the pool is short or empty; a list set by hand
//!   gets them only while it's thin; Extra finds add items per kind on top (not onto a list set by
//!   hand). Everyday finds go by the kind's mix of shared groups (RZ_LootGroups: food 4, drink 2...);
//!   the high medicine only at a hospital or a locked pharmacy. No prefab twice in one roll unless
//!   it stacks (PickFresh). Scaled by the town's loot level (RZ_TownMemory.GetLootLevel);
//!   electronics get the power grid's multiplier (RZ_PowerGrid.LootMult); rare finds x1.5 near
//!   the line (NearLineMult).
//! - A wreck's boot is a container (RZ_WreckBoots.c), rolled when it's first put out near a player;
//!   one on the old evacuation road (near a sealed checkpoint) is better (IsEvacRoad).
//!   A body is rolled when it dies and its things go into its inventory (or beside it).
//! - Locks: some pharmacies, garages, police stations, shops and houses are locked (seeded
//!   from the building's position, so the same ones stay locked). The lock is on the building's
//!   outside doors (RZ_DoorLocks.c); a building whose doors aren't found isn't locked. A key
//!   opens that kind of building in that town (houses: that colour of door); a crowbar forces
//!   it, loudly, and breaks it until the reset. Locked places have better loot.
//! - Keys are real items (RZ_KeyItemComponent, the Key prefab): they lie loose in houses or are
//!   handed over (GiveKey).
//! - Readable things: ID cards, letters, receipts and photos, in the Zone's voice, sometimes
//!   with a key taped inside, a stash on the back, or a puzzle circled on a map.

//------------------------------------------------------------------------------------------------
//! A loot table for one building type. Empty lists are filled from the loot pool by keywords.
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sType")]
class RZ_LootTable
{
	[Attribute("house", desc: "Building kind: house, shop, farm, pharmacy, hospital, garage, firestation, police, school, office, church, industrial, hunting, coast, prison, army; body (infected bodies), car (wrecks' boots), evac (a boot on the evacuation road, on top of car) or drone (a downed Perimeter drone)")]
	string m_sType;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Everyday finds", "et")]
	ref array<ResourceName> m_aCommon;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Rare finds (more likely behind a lock)", "et")]
	ref array<ResourceName> m_aRare;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Electronics (radios, batteries, torches): more likely where the power is on", "et")]
	ref array<ResourceName> m_aElectronics;
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Interactions > Search.
[BaseContainerProps()]
class RZ_SearchSettings
{
	[Attribute("1", desc: "What wrecks' boots and infected bodies hold, locks and keys, readable things (loot in houses: Interactions > Loot)")]
	bool m_bEnabled;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Loot pool: every item that can lie in a house or turn up in a wreck's boot or on a body. Tables left empty are filled from it by keywords in the prefab names (see docs/WORKBENCH_SETUP.md); a list still thin after that gets the built-in vanilla items for its building type. Each must be a real item with vanilla's pick-up action", "et")]
	ref array<ResourceName> m_aLootPool;

	[Attribute(desc: "Loot tables per building type (optional; empty lists use the pool)")]
	ref array<ref RZ_LootTable> m_aTables;

	[Attribute(desc: "Extra finds per building type: added to that type's table on top of the loot pool's keyword matches and the built-in items. For an item no keyword reaches, or one that belongs in one kind of place only (a fire axe in garages). A list set by hand in Loot tables is kept as it is and gets none")]
	ref array<ref RZ_LootTable> m_aExtras;

	[Attribute("0.6", desc: "At full town loot (scaled by what the town has left): on an infected body, the chance it carries anything; a one-off find's chance per slot (loose loot in houses: Interactions > Loot > Slot chance; wrecks' boots: Boot chance)")]
	float m_fFindChance;

	[Attribute("0.8", desc: "At full town loot (scaled by what the town has left): the chance a wreck's boot holds anything")]
	float m_fBootChance;

	[Attribute("0.4", desc: "At full town loot: the chance a wreck's boot that holds anything holds a second thing")]
	float m_fBootTwoChance;

	[Attribute(desc: "Loot mixes (optional): a kind's everyday mix of groups by weight, e.g. house \"food 4, drink 2, clothes 3, medlow 2\". Food, drink and the low medicine are shared lists (every kind draws the same food); the other groups are the kind's own items. Empty: the built-in mixes (docs/DESIGN.md, \"Groups and mixes\")")]
	ref array<ref RZ_LootMix> m_aMixes;

	[Attribute("1.5", desc: "Near the line: rare finds are this many times as likely within Near line reach of the Perimeter (people fled inward; the snipers and drones are there). 1 = no change. Only with the Perimeter on")]
	float m_fNearLineMult;

	[Attribute("1000", desc: "Near line reach (m): how far inside the Perimeter's edge rare finds are better")]
	float m_fNearLineReach;

	[Attribute("500", desc: "The old evacuation road (m): a wreck's boot this close to a sealed road checkpoint is fuller (the last people who tried to drive out) and gets one thing from the evac table. Only with the Perimeter on. 0 = off")]
	float m_fEvacReach;

	[Attribute("pier,bollard,retainingwall,brickpile,woodpile,houseruin,well_", desc: "Not buildings: keywords in the names of things the map counts as destructible buildings that aren't rooms to loot (piers, bollards, retaining walls, ruins' rubble, wells). They get no loose loot")]
	string m_sNotBuildingWords;

	[Attribute("0.1", desc: "Chance a find is rare (doubled behind a lock)")]
	float m_fRareChance;

	[Attribute("0.1", desc: "Chance a find is electronics (x the power grid's electronics multiplier)")]
	float m_fElectronicsChance;

	// ---------------------------------------------------------------- Locks and keys
	[Attribute("1", desc: "Some buildings are locked (seeded from their position): their outside doors (the vanilla door action refuses; Unlock, Lock and Force at the door)")]
	bool m_bLocks;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Key item: a small item with RZ_KeyItemComponent, an InventoryItemComponent (vanilla's pick-up) and optionally RZ_ReadKeyTagAction. Keys lie in houses and can be dropped, traded and stolen. Empty = no keys", "et")]
	ResourceName m_sKeyPrefab;

	[Attribute("0.5", desc: "Share of pharmacies that are locked")]
	float m_fLockPharmacy;

	[Attribute("0.5", desc: "Share of police stations that are locked")]
	float m_fLockPolice;

	[Attribute("0.35", desc: "Share of garages that are locked")]
	float m_fLockGarage;

	[Attribute("0.25", desc: "Share of shops that are locked")]
	float m_fLockShop;

	[Attribute("0.1", desc: "Share of houses (and farmhouses) that are locked")]
	float m_fLockHouse;

	[Attribute("crowbar,crow_bar,prybar,pry_bar,wreckingbar,wrecking_bar", desc: "Crowbar keywords (forces a lock)")]
	string m_sCrowbarWords;

	[Attribute("10", desc: "Seconds to force a door")]
	float m_fForceSeconds;

	[Attribute("25", desc: "Heat forcing a door makes")]
	float m_fForceHeat;

	[Attribute("lockpick,lock_pick,picklock", desc: "Lockpick keywords (picks a lock quietly; the bench's Lockpick recipe makes one)")]
	string m_sLockpickWords;

	[Attribute("14", desc: "Seconds to pick a lock")]
	float m_fPickSeconds;

	[Attribute("0.7", desc: "Chance a pick opens the lock; else the pick snaps (used up) and the lock holds")]
	float m_fPickChance;

	[Attribute("3", desc: "Heat picking a lock makes (forcing makes Heat forcing a door)")]
	float m_fPickHeat;

	[Attribute("180", desc: "Real minutes before an unlocked, forced or latched lock is back to its default (locked, doors shut)")]
	float m_fRelockMinutes;

	// ---------------------------------------------------------------- Keeping AI out (RZ_DoorBlockers.c, RZ_AIDoorHook.c)
	[Attribute("1", desc: "AI door hook: the AI's own door opening refuses a locked outside door (RZ_AIDoorHook.c, optional file). Off = the hook lets every door open")]
	bool m_bAiDoorHook;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Doorway blocker: an invisible, thin collision box that stops characters (not bullets) and counts for the navmesh. Stands in a locked outside doorway near players. Empty = no blockers (docs/WORKBENCH_SETUP.md)", "et")]
	ResourceName m_sDoorBlockerPrefab;

	[Attribute("1", desc: "Rebuild the navmesh around a doorway when its blocker comes or goes, so AI routes around a locked door. Off = blockers only (AI walk into them and stop)")]
	bool m_bNavmeshRebuild;

	[Attribute("150", desc: "Locked doorways this close (m) to a player get a blocker and can be banged on")]
	float m_fBlockerRange;

	[Attribute("20", desc: "Most blocked doorways on the server (the nearest to players win)")]
	int m_iBlockerCap;

	[Attribute("60", desc: "Seconds after a doorway's blocker came or went before it can come back (lock and unlock spam can't force rebuilds). Removing is always instant")]
	float m_fBlockerCooldown;

	[Attribute("4", desc: "Heat an infected pressing at a locked doorway makes (0 = silent)")]
	float m_fDoorBangHeat;

	[Attribute("15", desc: "Seconds between bangs at one doorway")]
	float m_fDoorBangSeconds;

	// ---------------------------------------------------------------- Readable things and bodies
	[Attribute("0.12", desc: "Chance a building's loot (or a wreck's boot) has something readable (ID card, letter, receipt, photo)")]
	float m_fReadableChance;

	[Attribute("0.3", desc: "Chance an infected body has something readable on it")]
	float m_fBodyReadableChance;

	[Attribute("0.08", desc: "Chance an infected body has a key on it")]
	float m_fBodyKeyChance;

	[Attribute("8", desc: "Most infected bodies a minute that get loot (killed within 60 m of a player): a horde shot down at once doesn't cover the ground")]
	int m_iBodyCap;
}

//------------------------------------------------------------------------------------------------
//! A readable thing (its text stays on the server). Its note prop exists only near players.
class RZ_Readable
{
	int m_iId;
	string m_sTitle;
	string m_sText;
	IEntity m_Body;
	float m_fAt;
	bool m_bLoose;		// lies in a building's loose loot (RZ_Loot keeps it), not left by a search
}

//------------------------------------------------------------------------------------------------
//! A stash someone wrote about: the building or wreck's boot nearest it holds it.
class RZ_Stash
{
	vector m_vPos;
	string m_sPlace;
	float m_fUntil;
}

//------------------------------------------------------------------------------------------------
class RZ_Search
{
	protected static const int NOTES_KEPT = 30;
	protected static const int LOOSE_NOTES_KEPT = 600;
	protected static const float NOTE_MINUTES = 30;
	protected static const float STASH_RADIUS = 30;
	protected static const float QUEST_CLEAR = 25;		// no lock within this (m) of an opening's or an errand's places
	protected static const float STASH_MINUTES = 90;
	protected static const int STASHES_KEPT = 6;
	protected static const float BODY_WITNESS = 60;
	protected static const float BODY_REACH = 3;	// a dead body this close to where the infected fell is theirs

	protected RZ_GameModeComponent m_GM;
	protected RZ_Interactions m_Hub;
	protected RZ_SearchSettings m_Settings;
	protected ref map<string, ref RZ_LootTable> m_mTables = new map<string, ref RZ_LootTable>();
	// Groups and mixes (RZ_LootGroups): the shared lists (food, drink, the low medicine), each kind's
	// own items by group, and each kind's everyday mix
	protected ref map<string, ref array<ResourceName>> m_mShared = new map<string, ref array<ResourceName>>();
	protected ref map<string, ref map<string, ref array<ResourceName>>> m_mOwn = new map<string, ref map<string, ref array<ResourceName>>>();
	protected ref map<string, ref array<string>> m_mMixGroups = new map<string, ref array<string>>();
	protected ref map<string, ref array<float>> m_mMixWeights = new map<string, ref array<float>>();
	protected ref map<string, float> m_mOpened = new map<string, float>();
	protected ref map<string, float> m_mForced = new map<string, float>();	// opened by force: broken until the reset
	protected ref array<ref RZ_Readable> m_aNotes = {};
	protected ref array<ref RZ_Stash> m_aStashes = {};
	protected ref array<string> m_aElectronicsWords = {};
	protected ref array<string> m_aStackWords;
	protected ref array<string> m_aNotBuildingWords = {};
	protected int m_iNextNote = 1;
	protected bool m_bKeyPrefabWarned;
	protected float m_fBodyMinuteAt = -100;
	protected int m_iBodiesThisMinute;
	protected int m_iExtras;

	//------------------------------------------------------------------------------------------------
	void RZ_Search(RZ_GameModeComponent gm, RZ_Interactions hub, RZ_SearchSettings settings)
	{
		m_GM = gm;
		m_Hub = hub;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled() { return m_Settings.m_bEnabled; }
	float GetReadableChance() { return m_Settings.m_fReadableChance; }
	float GetFindChance() { return m_Settings.m_fFindChance; }
	ResourceName GetKeyPrefab() { return m_Settings.m_sKeyPrefab; }

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	void SaveInto(RZ_InteractionSave data)
	{
		float now = m_Hub.GetClock();
		float relock = m_Settings.m_fRelockMinutes * 60;
		foreach (string okey, float oat : m_mOpened)
		{
			if (now - oat < relock)
			{
				data.openKey.Insert(okey);
				data.openAt.Insert(oat);
				if (m_mForced.Contains(okey))
					data.forcedKey.Insert(okey);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void RestoreFrom(RZ_InteractionSave data)
	{
		for (int j = 0; j < data.openKey.Count() && j < data.openAt.Count(); j++)
		{
			m_mOpened.Set(data.openKey[j], data.openAt[j]);
		}

		if (!data.forcedKey)
			return;

		foreach (string fkey : data.forcedKey)
		{
			float fat;
			if (m_mOpened.Find(fkey, fat))
				m_mForced.Set(fkey, fat);
		}
	}

	// ================================================================ Setup
	//------------------------------------------------------------------------------------------------
	void Setup()
	{
		RZ_ItemHelper.Words("radio,battery,batteries,flashlight,torch,electronic,gps,walkie,lamp_portable", m_aElectronicsWords);
		RZ_ItemHelper.Words(m_Settings.m_sNotBuildingWords, m_aNotBuildingWords);

		array<string> types = {};
		RZ_LootGroups.Kinds(types);
		foreach (string type : types)
		{
			BuildTable(type);
		}

		BuildGroups(types);
		int pool = 0;
		if (m_Settings.m_aLootPool)
			pool = m_Settings.m_aLootPool.Count();

		RZ_Log.Info(string.Format("Search: %1 items in the loot pool, %2 tables set by hand. Tables (common/rare/electronics): %3, %4, %5, %6, %7, %8, %9.",
			pool, CountConfigured(), TableSizes("house"), TableSizes("shop"), TableSizes("farm"), TableSizes("pharmacy"), TableSizes("garage"), TableSizes("police"), TableSizes("body")));
		RZ_Log.Info(string.Format("Search: the new kinds (common/rare/electronics): %1, %2, %3, %4, %5, %6, %7, %8, %9.",
			TableSizes("hospital"), TableSizes("firestation"), TableSizes("school"), TableSizes("office"), TableSizes("church"), TableSizes("industrial"), TableSizes("hunting"), TableSizes("coast"), TableSizes("army")));
		RZ_Log.Info(string.Format("Search: shared groups: %1 food, %2 drink, %3 low medicine; %4 kinds with a mix set by hand. Prison %5, downed drone %6, evacuation road %7.",
			SharedCount(RZ_LootGroups.FOOD), SharedCount(RZ_LootGroups.DRINK), SharedCount(RZ_LootGroups.MED_LOW), CountMixes(), TableSizes("prison"), TableSizes("drone"), TableSizes("evac")));

		if (m_iExtras > 0)
			RZ_Log.Info(string.Format("Search: %1 extra finds added to the tables (Search > Extra finds).", m_iExtras));

		string where = "off";
		if (m_Settings.m_bLocks)
			where = "on the outside doors";

		string keys = "off (no key prefab)";
		if (!m_Settings.m_sKeyPrefab.IsEmpty())
			keys = "key items";

		RZ_Log.Info(string.Format("Locks: %1; keys are %2.", where, keys));
		if (!m_Settings.m_bLocks)
			return;

		string blockers = "off (no blocker prefab)";
		if (!m_Settings.m_sDoorBlockerPrefab.IsEmpty() && m_Settings.m_bNavmeshRebuild)
			blockers = string.Format("on, navmesh rebuilt, at most %1 within %2 m", m_Settings.m_iBlockerCap, RZ_Util.RoundToInt(m_Settings.m_fBlockerRange));
		else if (!m_Settings.m_sDoorBlockerPrefab.IsEmpty())
			blockers = string.Format("on, no navmesh rebuild, at most %1 within %2 m", m_Settings.m_iBlockerCap, RZ_Util.RoundToInt(m_Settings.m_fBlockerRange));

		string hook = "off";
		if (m_Settings.m_bAiDoorHook)
			hook = "on";

		RZ_Log.Info(string.Format("Door blockers: %1; AI door hook %2.", blockers, hook));
	}

	//------------------------------------------------------------------------------------------------
	protected int CountConfigured()
	{
		if (!m_Settings.m_aTables)
			return 0;

		return m_Settings.m_aTables.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! A table per type: the one set by hand, with any empty list filled from the pool.
	protected void BuildTable(string type)
	{
		RZ_LootTable t = RZ_LootTable.Cast(RZ_Util.NewWithDefaults("RZ_LootTable"));
		t.m_sType = type;
		if (m_Settings.m_aTables)
		{
			foreach (RZ_LootTable table : m_Settings.m_aTables)
			{
				if (!table || table.m_sType != type)
					continue;

				AppendAll(table.m_aCommon, t.m_aCommon);
				AppendAll(table.m_aRare, t.m_aRare);
				AppendAll(table.m_aElectronics, t.m_aElectronics);
			}
		}

		// A list set by hand is kept as it is (topped up only if thin); one filled from the pool by
		// keywords always gets the built-in vanilla items for the type too, so a type's table is
		// wide (a small table repeats itself from house to house)
		bool handCommon = !t.m_aCommon.IsEmpty();
		bool handRare = !t.m_aRare.IsEmpty();
		bool handElectronics = !t.m_aElectronics.IsEmpty();
		array<string> common = {};
		array<string> rare = {};
		TypeWords(type, common, rare);
		if (!handCommon)
			FromPool(common, t.m_aCommon);

		if (!handRare)
			FromPool(rare, t.m_aRare);

		if (t.m_aElectronics.IsEmpty())
			FromPool(m_aElectronicsWords, t.m_aElectronics);

		array<ResourceName> builtCommon = {};
		array<ResourceName> builtRare = {};
		array<ResourceName> builtElectronics = {};
		RZ_LootDefaults.For(type, builtCommon, builtRare, builtElectronics);
		if (!handCommon || t.m_aCommon.Count() < RZ_LootDefaults.THIN_COMMON)
			AppendNew(builtCommon, t.m_aCommon);

		if (!handRare || t.m_aRare.Count() < RZ_LootDefaults.THIN_RARE)
			AppendNew(builtRare, t.m_aRare);

		if (t.m_aElectronics.IsEmpty())
			AppendNew(builtElectronics, t.m_aElectronics);

		// Extra finds: on top of the pool and the built-in items, never onto a list set by hand
		if (m_Settings.m_aExtras)
		{
			foreach (RZ_LootTable extra : m_Settings.m_aExtras)
			{
				if (!extra || extra.m_sType != type)
					continue;

				if (!handCommon)
					m_iExtras += AppendNew(extra.m_aCommon, t.m_aCommon);

				if (!handRare)
					m_iExtras += AppendNew(extra.m_aRare, t.m_aRare);

				if (!handElectronics)
					m_iExtras += AppendNew(extra.m_aElectronics, t.m_aElectronics);
			}
		}

		m_mTables.Set(type, t);
	}

	//------------------------------------------------------------------------------------------------
	//! After the tables: the shared lists (every kind's everyday items and the pool, by group: food,
	//! drink, the low medicine), each kind's own items by group, and each kind's mix (Loot mixes,
	//! else RZ_LootGroups.DefaultMix).
	protected void BuildGroups(array<string> kinds)
	{
		m_mShared.Clear();
		m_mOwn.Clear();
		m_mMixGroups.Clear();
		m_mMixWeights.Clear();
		array<ResourceName> everything = {};
		if (m_Settings.m_aLootPool)
			AppendNew(m_Settings.m_aLootPool, everything);

		foreach (string kind : kinds)
		{
			RZ_LootTable t;
			if (!m_mTables.Find(kind, t))
				continue;

			map<string, ref array<ResourceName>> own = new map<string, ref array<ResourceName>>();
			foreach (ResourceName r : t.m_aCommon)
			{
				string g = RZ_LootGroups.GroupOf(r);
				array<ResourceName> list;
				if (!own.Find(g, list))
				{
					list = {};
					own.Set(g, list);
				}

				if (!HasSame(list, r))
					list.Insert(r);

				// A kind's own food and drink feed everyone's (bodies and boots excepted: they aren't rooms)
				if (kind != "body" && kind != "car" && kind != "drone" && kind != "evac" && !HasSame(everything, r))
					everything.Insert(r);
			}

			m_mOwn.Set(kind, own);
			array<string> groups = {};
			array<float> weights = {};
			RZ_LootGroups.ParseMix(MixFor(kind), groups, weights);
			m_mMixGroups.Set(kind, groups);
			m_mMixWeights.Set(kind, weights);
		}

		foreach (ResourceName e : everything)
		{
			string sg = RZ_LootGroups.GroupOf(e);
			if (!RZ_LootGroups.IsShared(sg))
				continue;

			array<ResourceName> shared;
			if (!m_mShared.Find(sg, shared))
			{
				shared = {};
				m_mShared.Set(sg, shared);
			}

			shared.Insert(e);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A kind's mix: the one set by hand, else the built-in one.
	protected string MixFor(string kind)
	{
		if (m_Settings.m_aMixes)
		{
			foreach (RZ_LootMix m : m_Settings.m_aMixes)
			{
				if (m && m.m_sType == kind && !m.m_sMix.IsEmpty())
					return m.m_sMix;
			}
		}

		return RZ_LootGroups.DefaultMix(kind);
	}

	//------------------------------------------------------------------------------------------------
	protected int CountMixes()
	{
		if (!m_Settings.m_aMixes)
			return 0;

		return m_Settings.m_aMixes.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! How many items a shared group has (for the log and the test).
	int SharedCount(string group)
	{
		array<ResourceName> list;
		if (!m_mShared.Find(group, list))
			return 0;

		return list.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! A kind's everyday pick by its mix: a group by weight (among the groups it has anything of),
	//! then one of that group's items (a shared list for food, drink and the low medicine). "" if the
	//! kind has nothing.
	protected ResourceName PickEveryday(string kind, RZ_LootTable t)
	{
		array<string> groups;
		array<float> weights;
		map<string, ref array<ResourceName>> own;
		if (!m_mMixGroups.Find(kind, groups) || !m_mMixWeights.Find(kind, weights) || !m_mOwn.Find(kind, own))
		{
			if (t && !t.m_aCommon.IsEmpty())
				return t.m_aCommon.GetRandomElement();

			return "";
		}

		float total = 0;
		array<ref array<ResourceName>> lists = {};
		array<float> live = {};
		foreach (int i, string g : groups)
		{
			array<ResourceName> list;
			if (RZ_LootGroups.IsShared(g))
				m_mShared.Find(g, list);
			else
				own.Find(g, list);

			if (!list || list.IsEmpty())
				continue;

			lists.Insert(list);
			live.Insert(weights[i]);
			total += weights[i];
		}

		if (total <= 0)
		{
			if (t && !t.m_aCommon.IsEmpty())
				return t.m_aCommon.GetRandomElement();

			return "";
		}

		float roll = Math.RandomFloat(0, total);
		foreach (int n, float w : live)
		{
			roll -= w;
			if (roll <= 0)
				return lists[n].GetRandomElement();
		}

		return lists[lists.Count() - 1].GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! A resource's GUID ("{EB21D26B8D5A3232}"), or the whole name without one. Two spellings of
	//! one prefab (a mod's copy is "prefabs/...", the base game's "Prefabs/...") share it.
	static string GuidOf(ResourceName r)
	{
		string s = r;
		int close = s.IndexOf("}");
		if (s.StartsWith("{") && close > 0)
			return s.Substring(0, close + 1);

		s.ToLower();
		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! `into` already holds this prefab, by its GUID.
	protected static bool HasSame(notnull array<ResourceName> into, ResourceName r)
	{
		string g = GuidOf(r);
		foreach (ResourceName have : into)
		{
			if (GuidOf(have) == g)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The map counts it as a building but it's no room to loot (a pier, a bollard, rubble).
	bool IsNotBuilding(string lowerName)
	{
		// A cottage on a pier or a boat shed is a room, whatever the pier words say
		if (IsCoastName(lowerName))
			return false;

		return RZ_ItemHelper.NameHas(lowerName, m_aNotBuildingWords);
	}

	//------------------------------------------------------------------------------------------------
	protected void AppendAll(array<ResourceName> from, notnull array<ResourceName> into)
	{
		if (!from)
			return;

		foreach (ResourceName r : from)
		{
			if (!r.IsEmpty())
				into.Insert(r);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The ones not in `into` yet. Returns how many were added.
	protected int AppendNew(array<ResourceName> from, notnull array<ResourceName> into)
	{
		if (!from)
			return 0;

		int added = 0;
		foreach (ResourceName r : from)
		{
			if (!r.IsEmpty() && !HasSame(into, r))
			{
				into.Insert(r);
				added++;
			}
		}

		return added;
	}

	//------------------------------------------------------------------------------------------------
	//! The table's sizes for the log: "house 14/4/2".
	protected string TableSizes(string type)
	{
		RZ_LootTable t;
		if (!m_mTables.Find(type, t))
			return type + " 0";

		return string.Format("%1 %2/%3/%4", type, t.m_aCommon.Count(), t.m_aRare.Count(), t.m_aElectronics.Count());
	}

	//------------------------------------------------------------------------------------------------
	//! A loot pool item whose prefab name has one of the words (the Long Table's jar: a food
	//! item), "" if the pool has none.
	ResourceName PickFromPool(array<string> words)
	{
		array<ResourceName> found = {};
		FromPool(words, found);
		if (found.IsEmpty())
			return "";

		return found.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! Search > Crowbar keywords (what forces a lock), for missions that force something.
	string GetCrowbarWords()
	{
		return m_Settings.m_sCrowbarWords;
	}

	//------------------------------------------------------------------------------------------------
	protected void FromPool(array<string> words, notnull array<ResourceName> into)
	{
		if (!m_Settings.m_aLootPool)
			return;

		foreach (ResourceName r : m_Settings.m_aLootPool)
		{
			string name = r;
			name.ToLower();
			if (RZ_ItemHelper.NameHas(name, words) && !HasSame(into, r))
				into.Insert(r);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Keyword fallbacks per building type: what's usually there, and what's rare.
	protected void TypeWords(string type, notnull array<string> common, notnull array<string> rare)
	{
		// Torches are electronics (and a garage's, a police station's and a car's built-ins), not
		// every house's everyday find; "can" isn't a word here: it caught jerrycans and canteens
		string c = "food,tin,bottle,water,bandage,cloth,rag,knife,battery,pill,shirt,jacket,matches,lighter";
		string r = "pistol,medkit,radio,binocular,map";
		switch (type)
		{
			case "pharmacy":
			{
				c = "bandage,medkit,med,pill,tablet,saline,tourniquet,syringe,vitamin,painkiller,dressing,gauze,purification,iodine,bleach";
				r = "morphine,antibiotic,epinephrine,adrenaline,saline,medkit";
				break;
			}
			case "garage":
			{
				c = "jerrycan,jerry_can,fuel,canister,tool,wrench,spanner,screwdriver,battery,tyre,tire,spark,wire,rope,plank,nail,crowbar,rag,bottle";
				r = "battery,spark,tyre,tire,crowbar,toolbox";
				break;
			}
			case "farm":
			{
				c = "food,tin,bread,water,bottle,axe,hatchet,knife,rope,cloth,plank,nail,wire,jerrycan,matches";
				r = "shotgun,hunting,rifle,binocular";
				break;
			}
			case "police":
			{
				c = "ammo,magazine,mag_,baton,handcuff,radio,bandage,tourniquet,vest";
				r = "pistol,shotgun,vest,helmet,radio";
				break;
			}
			case "shop":
			{
				c = "food,tin,bottle,water,battery,cloth,matches,lighter,bleach,rope";
				r = "radio,binocular,medkit";
				break;
			}
			case "body":
			{
				c = "bandage,ammo,magazine,food,cloth,rag,knife,pill,lighter,matches";
				r = "pistol,medkit,radio";
				break;
			}
			case "car":
			{
				c = "jerrycan,jerry_can,fuel,tool,wrench,rope,bottle,water,food,battery,tyre,tire,rag,map";
				r = "battery,spark,toolbox,binocular";
				break;
			}
			case "hospital":
			{
				c = "bandage,dressing,gauze,tourniquet,saline,pill,tablet,painkiller,syringe,iodine,glove";
				r = "morphine,antibiotic,epinephrine,adrenaline,medkit,medicalkit,hazmat";
				break;
			}
			case "firestation":
			{
				c = "firefighter,helmet_civ,glove,overall,flashlight,rope,canteen";
				r = "fire axe,fire_axe,oxygen,firefighter_suit,gas_mask";
				break;
			}
			case "school":
			{
				c = "food,tin,water,bottle,writingpad,cloth,shirt,sneaker,map";
				r = "watch,binocular";
				break;
			}
			case "office":
			{
				c = "map,writingpad,watch,cash,creditcard,water,bottle";
				r = "goldbrick,radio";
				break;
			}
			case "church":
			{
				c = "food,water,bottle,candle";
				r = "writingpad";
				break;
			}
			case "industrial":
			{
				c = "wire,nail,scrap,jerrycan,plank,rope,overall,glove,hard";
				r = "fuse,toolbox,wrench,crowbar";
				break;
			}
			case "hunting":
			{
				c = "knife,binocular,compass,map,raincoat,boonie,canteen";
				r = "rifle,shotgun";
				break;
			}
			case "coast":
			{
				c = "raincoat,fisherman,rope,food,tin,water";
				r = "drybag,binocular";
				break;
			}
			case "prison":
			{
				c = "prisoner,food,tin";
				r = "";
				break;
			}
			case "army":
			{
				c = "magazine,mag_,ammo,bdu,m65,mre,canteen,smoke";
				r = "rifle,helmet,vest";
				break;
			}
			case "drone":
			{
				c = "cells,battery,wire,scrap,fuse";
				r = "radio";
				break;
			}
			case "evac":
			{
				c = "mre,cracker,food,canteen,water,dressing";
				r = "backpack,medkit,medicalkit";
				break;
			}
		}

		RZ_ItemHelper.Words(c, common);
		RZ_ItemHelper.Words(r, rare);
	}

	// ================================================================ Recognising things
	//------------------------------------------------------------------------------------------------
	//! The building kind from its lower-case prefab or mesh name (docs/DESIGN.md, "Building kinds").
	//! The order matters: a fisherman's shed is the coast's before it's a farm's shed, a medical
	//! barracks is a hospital before it's an army post.
	string BuildingType(string name)
	{
		// Vet clinics stock what farms do (the veterinary antiviral), not a pharmacy's shelves
		if (name.Contains("veterin") || name.Contains("vet_") || name.Contains("vetclinic"))
			return "farm";

		if (name.Contains("hospital") || name.Contains("clinic") || (name.Contains("barracks") && name.Contains("medical")))
			return "hospital";

		if (RZ_LandmarkFinder.Matches("pharmacy", name) || name.Contains("drugstore") || name.Contains("apothec"))
			return "pharmacy";

		if (name.Contains("firestation") || name.Contains("fire_station") || name.Contains("firehouse"))
			return "firestation";

		if (name.Contains("garage") || name.Contains("workshop") || name.Contains("carservice") || name.Contains("car_service") || name.Contains("autoservice") || name.Contains("repair") || name.Contains("mechanic"))
			return "garage";

		if (name.Contains("police") || name.Contains("gendarm") || name.Contains("sheriff"))
			return "police";

		if (name.Contains("prison") || name.Contains("jail"))
			return "prison";

		if (name.Contains("barrack") || name.Contains("bunker") || name.Contains("guardhouse") || name.Contains("radar") || name.Contains("controltower") || name.Contains("headquarters") || name.Contains("ammodump"))
			return "army";

		if (name.Contains("school") || name.Contains("kindergarten"))
			return "school";

		if (name.Contains("office") || name.Contains("townhall") || name.Contains("town_hall") || name.Contains("postoffice") || name.Contains("post_office") || name.Contains("bank_"))
			return "office";

		if (name.Contains("church") || name.Contains("chapel"))
			return "church";

		if (name.Contains("factory") || name.Contains("warehouse") || name.Contains("depot") || name.Contains("sawmill") || name.Contains("hangar") || name.Contains("concreteplant") || name.Contains("boilerhouse"))
			return "industrial";

		if (IsCoastName(name))
			return "coast";

		if (name.Contains("hunting") || name.Contains("hunterstand") || name.Contains("hunter_stand") || name.Contains("lodge"))
			return "hunting";

		if (name.Contains("farm") || name.Contains("barn") || name.Contains("stable") || name.Contains("shed") || name.Contains("silo") || name.Contains("cowshed"))
			return "farm";

		if (name.Contains("shop") || name.Contains("store") || name.Contains("market") || name.Contains("grocery") || name.Contains("kiosk") || name.Contains("fuelstation") || name.Contains("pubvillage"))
			return "shop";

		return "house";
	}

	//------------------------------------------------------------------------------------------------
	//! A boathouse, a fisherman's shed, a cottage on a pier: the coast's kind.
	static bool IsCoastName(string name)
	{
		return name.Contains("boathouse") || name.Contains("boatshed") || name.Contains("fisherman") || name.Contains("cottagepier") || name.Contains("pier_shed");
	}

	//------------------------------------------------------------------------------------------------
	static string TypeNoun(string type)
	{
		switch (type)
		{
			case "pharmacy": return "the pharmacy";
			case "garage": return "the garage";
			case "farm": return "the farmhouse";
			case "police": return "the police station";
			case "shop": return "the shop";
			case "body": return "the body";
			case "hospital": return "the hospital";
			case "firestation": return "the fire station";
			case "school": return "the school";
			case "office": return "the office";
			case "church": return "the church";
			case "industrial": return "the warehouse";
			case "hunting": return "the hunting lodge";
			case "coast": return "the boat shed";
			case "prison": return "the prison";
			case "army": return "the army post";
		}

		return "the house";
	}

	//------------------------------------------------------------------------------------------------
	//! What a key's tag says. Houses (and farmhouses) by the colour of their door.
	protected static string TypeTag(string type, string colour)
	{
		switch (type)
		{
			case "pharmacy": return "Pharmacy";
			case "garage": return "Garage";
			case "police": return "Police";
			case "shop": return "Shop, back door";
			case "hospital": return "Hospital";
			case "firestation": return "Fire station";
			case "school": return "School";
			case "office": return "Office";
			case "church": return "Church, vestry";
			case "industrial": return "Warehouse";
			case "hunting": return "Lodge";
			case "coast": return "Boat shed";
			case "prison": return "Prison";
			case "army": return "Army post";
		}

		if (colour.IsEmpty())
			return "House";

		return colour + " house";
	}

	// ================================================================ Locks
	//------------------------------------------------------------------------------------------------
	//! A building's own lock, seeded from its position: the same buildings have one after a restart.
	//! Never at a place people live or trade (safehouses, the lab, holdings, quartermasters): their
	//! doors stay open to the people who use them. Never within 25 m of a running mission's props
	//! (RZ_MissionManager.IsNearMissionProp) unless the lock is part of that mission, nor within 25 m
	//! of a running opening's places (the house you start in) or an errand's on offer or held. Only a
	//! building with outside doors is really locked (RZ_DoorLocks.HasDoorLock).
	bool HasLock(vector buildingPos, string type)
	{
		if (!HasOwnLock(buildingPos, type))
			return false;

		// Never where a job sends you: an opening's house (where you start) and places, an errand's
		// places (the author: no spawn or quest place behind a locked door)
		RZ_Guidance guidance = m_GM.GetGuidance();
		if (guidance && guidance.GetOpenings() && guidance.GetOpenings().IsNearPlace(buildingPos, QUEST_CLEAR))
			return false;

		RZ_Errands errands = m_GM.GetErrands();
		if (errands && errands.IsNearPlace(buildingPos, QUEST_CLEAR))
			return false;

		// Never next to a running mission's props (unless the lock is the mission's own)
		RZ_MissionManager missions = m_GM.GetMissions();
		return !missions || !missions.IsNearMissionProp(buildingPos);
	}

	//------------------------------------------------------------------------------------------------
	//! The building's seeded lock, whatever missions are running nearby (keys are cut for these).
	bool HasOwnLock(vector buildingPos, string type)
	{
		if (!m_Settings.m_bLocks || type == "body" || type == "car")
			return false;

		if (m_Hub.IsProtected(buildingPos))
			return false;

		float share = m_Settings.m_fLockHouse;
		switch (type)
		{
			case "pharmacy": share = m_Settings.m_fLockPharmacy; break;
			case "hospital": share = m_Settings.m_fLockPharmacy; break;
			case "police": share = m_Settings.m_fLockPolice; break;
			case "army": share = m_Settings.m_fLockPolice; break;
			case "prison": share = m_Settings.m_fLockPolice; break;
			case "garage": share = m_Settings.m_fLockGarage; break;
			case "firestation": share = m_Settings.m_fLockGarage; break;
			case "industrial": share = m_Settings.m_fLockGarage; break;
			case "shop": share = m_Settings.m_fLockShop; break;
			case "office": share = m_Settings.m_fLockShop; break;
			case "school": share = m_Settings.m_fLockShop; break;
		}

		int gx = Math.Floor(buildingPos[0] / 4);
		int gz = Math.Floor(buildingPos[2] / 4);
		RZ_Rng rng = new RZ_Rng((m_Hub.GetSeed() % 100000) + gx * 7919 + gz * 104729);
		return rng.Chance(share);
	}

	//------------------------------------------------------------------------------------------------
	//! Locked right now: it has a lock, and nobody unlocked, forced or latched it open lately.
	bool IsBuildingLocked(vector buildingPos, string buildingKey, string type)
	{
		float opened;
		if (m_mOpened.Find(buildingKey, opened) && m_Hub.GetClock() - opened < m_Settings.m_fRelockMinutes * 60)
			return false;

		return HasLock(buildingPos, type);
	}

	//------------------------------------------------------------------------------------------------
	//! Forced open and not reset yet: the lock is broken and can't be locked.
	bool IsForced(string buildingKey)
	{
		float at;
		if (!m_mForced.Find(buildingKey, at))
			return false;

		return m_Hub.GetClock() - at < m_Settings.m_fRelockMinutes * 60;
	}

	//------------------------------------------------------------------------------------------------
	//! A door spot's building is locked right now.
	bool IsLocked(RZ_Spot s)
	{
		if (s.m_iKind != RZ_Interactions.KIND_DOOR)
			return false;

		return IsBuildingLocked(s.m_vBuilding, s.m_sBuildingKey, s.m_sType);
	}

	//------------------------------------------------------------------------------------------------
	//! Unlocked (key, latch) or forced: open until the reset.
	void OpenBuilding(string buildingKey, bool forced)
	{
		m_mOpened.Set(buildingKey, m_Hub.GetClock());
		if (forced)
			m_mForced.Set(buildingKey, m_Hub.GetClock());
		else
			m_mForced.Remove(buildingKey);

		m_Hub.OnLockChanged(buildingKey);
	}

	//------------------------------------------------------------------------------------------------
	//! Locked again with its key: back to the default state.
	void LockBuilding(string buildingKey)
	{
		m_mOpened.Remove(buildingKey);
		m_mForced.Remove(buildingKey);
		m_Hub.OnLockChanged(buildingKey);
	}

	// ================================================================ Keys
	//------------------------------------------------------------------------------------------------
	//! Farmhouses take house keys.
	protected static string KeyType(string type)
	{
		if (type == "farm")
			return "house";

		return type;
	}

	//------------------------------------------------------------------------------------------------
	//! The colour of a house's door, seeded from its position. "" for other buildings.
	string DoorColour(vector buildingPos, string type)
	{
		if (KeyType(type) != "house")
			return "";

		int gx = Math.Floor(buildingPos[0] / 4);
		int gz = Math.Floor(buildingPos[2] / 4);
		RZ_Rng rng = new RZ_Rng(gx * 31337 + gz * 7907 + 17);
		array<string> colours = {"Blue", "Green", "Red", "White", "Yellow"};
		return colours[rng.Int(0, colours.Count())];
	}

	//------------------------------------------------------------------------------------------------
	//! "pharmacy@3", "house:Blue@3": opens that kind of building (that colour of house) in that town.
	protected static string KeyId(int town, string type, string colour)
	{
		if (colour.IsEmpty())
			return string.Format("%1@%2", KeyType(type), town);

		return string.Format("%1:%2@%3", KeyType(type), colour, town);
	}

	//------------------------------------------------------------------------------------------------
	//! The key a building's lock takes.
	string KeyForBuilding(vector buildingPos, string type)
	{
		int town = m_GM.FindNearestSettlement(buildingPos);
		return KeyId(town, type, DoorColour(buildingPos, type));
	}

	//------------------------------------------------------------------------------------------------
	//! What that key's tag says: "Pharmacy, Lindale", "Blue house, Morton".
	string KeyTagForBuilding(vector buildingPos, string type)
	{
		return KeyTag(m_GM.FindNearestSettlement(buildingPos), type, DoorColour(buildingPos, type));
	}

	//------------------------------------------------------------------------------------------------
	protected string KeyTag(int town, string type, string colour)
	{
		RZ_Settlement s = m_GM.GetSettlement(town);
		string place = "no town you know";
		if (s)
			place = s.m_sName;

		return string.Format("%1, %2", TypeTag(KeyType(type), colour), place);
	}

	//------------------------------------------------------------------------------------------------
	//! A key item in their inventory (at their feet if it doesn't fit): rewards, bodies, the
	//! Mechanic. `tag` is what it says. Nothing without a key prefab.
	void GiveKey(int playerId, string key, string tag = "")
	{
		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!user || RZ_KeyItemComponent.Give(m_Settings.m_sKeyPrefab, user, key, tag))
			return;

		if (!m_bKeyPrefabWarned)
		{
			m_bKeyPrefabWarned = true;
			RZ_Log.Warn("Key prefab is empty, has no RZ_KeyItemComponent or didn't spawn: no keys are given or placed.");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A key item they carry.
	bool HasKey(int playerId, string key)
	{
		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		return RZ_KeyItemComponent.Carries(user, key);
	}

	//------------------------------------------------------------------------------------------------
	//! A key for a real locked building in the town nearest pos (its own lock on outside doors,
	//! not burned down). False if that town has none: no key is cut. Needs a key prefab.
	bool PickRandomKey(vector pos, out string keyId, out string tag)
	{
		keyId = "";
		tag = "";
		int town = m_GM.FindNearestSettlement(pos);
		RZ_Settlement s = m_GM.GetSettlement(town);
		if (!s || !m_Settings.m_bLocks || m_Settings.m_sKeyPrefab.IsEmpty())
			return false;

		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(s.m_vCenter, s.m_fRadius + 50, buildings, 120);
		array<IEntity> locked = {};
		foreach (IEntity b : buildings)
		{
			vector bpos = b.GetOrigin();
			if (HasOwnLock(bpos, BuildingType(RZ_ItemHelper.LowerName(b))) && !m_Hub.GetTraps().IsBurned(bpos))
				locked.Insert(b);
		}

		// Only one whose outside doors are found (a lock that's really there): a few tries
		for (int tries = 0; tries < 6 && !locked.IsEmpty(); tries++)
		{
			int pick = Math.RandomInt(0, locked.Count());
			IEntity building = locked[pick];
			locked.Remove(pick);
			if (!m_Hub.GetDoors().HasDoorLock(building))
				continue;

			vector at = building.GetOrigin();
			string type = BuildingType(RZ_ItemHelper.LowerName(building));
			keyId = KeyForBuilding(at, type);
			tag = KeyTagForBuilding(at, type);
			return true;
		}

		return false;
	}

	// ================================================================ Bodies
	//------------------------------------------------------------------------------------------------
	//! Infected that die with a player close by (within 60 m) carry what a search used to find: a
	//! roll from the body table (sometimes two things), the treatment roll, sometimes a key and
	//! sometimes something to read. There's no prompt: the items go into the body's own inventory,
	//! looted with vanilla's inventory, or lie loose beside it when it has none (or none fits). A
	//! note is read where it lies; a key taped inside lies by it. Loose, they're cleaned up like
	//! any loose item; on the body, they go with it. At most Body cap bodies a minute get any.
	void OnZombiesKilled(vector pos, int count)
	{
		if (!m_Settings.m_bEnabled || count <= 0)
			return;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		if (RZ_Util.NearestDistance(pos, players) > BODY_WITNESS)
			return;

		float now = m_Hub.GetClock();
		if (now - m_fBodyMinuteAt >= 60)
		{
			m_fBodyMinuteAt = now;
			m_iBodiesThisMinute = 0;
		}

		array<IEntity> corpses = {};
		RZ_CorpseQuery.FindDead(pos, BODY_REACH, corpses);
		for (int i = 0; i < count && m_iBodiesThisMinute < m_Settings.m_iBodyCap; i++)
		{
			m_iBodiesThisMinute++;
			IEntity corpse;
			if (i < corpses.Count())
				corpse = corpses[i];

			vector at = pos;
			if (corpse)
				at = corpse.GetOrigin();

			LootBody(RZ_Util.GroundPos(at), corpse);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Rolls a body's things and puts them on it (or beside it).
	protected void LootBody(vector pos, IEntity corpse)
	{
		array<ResourceName> picks = {};
		RollBody(pos, picks);

		// Treatments (docs/DESIGN.md, "Treatments"): one roll
		RZ_Infection infection = m_GM.GetInfection();
		if (infection)
		{
			int med;
			ResourceName medPrefab = infection.PickMedFind("body", false, med);
			if (!medPrefab.IsEmpty())
				picks.Insert(medPrefab);
		}

		foreach (ResourceName prefab : picks)
		{
			PutOnBody(RZ_Util.SpawnPrefab(prefab, RZ_Util.RandomPointInRing(pos, 0.4, 1) + Vector(0, 0.3, 0), Math.RandomFloat(0, 360)), corpse);
		}

		// A key
		string keyId;
		string keyTag;
		if (Math.RandomFloat01() < m_Settings.m_fBodyKeyChance && PickRandomKey(pos, keyId, keyTag))
			PutOnBody(RZ_KeyItemComponent.Spawn(m_Settings.m_sKeyPrefab, RZ_Util.RandomPointInRing(pos, 0.4, 1) + Vector(0, 0.3, 0), Math.RandomFloat(0, 360), keyId, keyTag), corpse);

		// Something to read: beside the body, for anyone to read, with a key taped inside by it
		if (Math.RandomFloat01() >= m_Settings.m_fBodyReadableChance)
			return;

		string noteKey;
		string noteTag;
		string line;
		int note = CreateReadable(pos, true, false, noteKey, noteTag, line);
		if (note < 0)
			return;

		vector notePos = RZ_Util.RandomPointInRing(pos, 0.5, 1.2);
		SpawnNote(note, notePos + Vector(0, 0.2, 0), Math.RandomFloat(0, 360));
		if (!noteKey.IsEmpty())
			PutOnBody(RZ_KeyItemComponent.Spawn(m_Settings.m_sKeyPrefab, notePos + Vector(0.2, 0.3, 0), Math.RandomFloat(0, 360), noteKey, noteTag), null);
	}

	//------------------------------------------------------------------------------------------------
	//! A spawned item into the body's inventory if it has one with room, else left where it was
	//! spawned, beside it. Tracked as a loose item either way (cleaned up once it lies abandoned).
	//! VERIFY: a dead infected character from the zombie mod keeps an inventory that
	//! TryInsertItem fills and vanilla's inventory shows as the body's.
	protected void PutOnBody(IEntity item, IEntity corpse)
	{
		if (!item)
			return;

		if (corpse)
			RZ_Util.InsertIntoInventory(corpse, item);

		RZ_GameModeComponent.TrackLooseItem(item);
	}

	//------------------------------------------------------------------------------------------------
	//! What a body carries from the body table: one thing Find chance of the time, sometimes two.
	protected void RollBody(vector pos, notnull array<ResourceName> picks)
	{
		RZ_LootTable t = TableFor("body");
		if (!t || Math.RandomFloat01() >= Math.Clamp(m_Settings.m_fFindChance, 0, 0.95))
			return;

		int count = 1;
		if (Math.RandomFloat01() < 0.2)
			count = 2;

		float rare;
		float electronics;
		Odds("body", pos, false, true, rare, electronics);
		for (int i = 0; i < count; i++)
		{
			ResourceName pick = PickFresh(t, "body", false, rare, electronics, picks);
			if (!pick.IsEmpty())
				picks.Insert(pick);
		}
	}

	// ================================================================ Forcing a door
	//------------------------------------------------------------------------------------------------
	//! A forced door: open, loud, and broken until the reset.
	void CompleteForce(int playerId, RZ_Spot s, IEntity user)
	{
		if (!s)
			return;

		OpenBuilding(s.m_sBuildingKey, true);
		m_GM.GetHeat().AddHeat(s.m_vPos, m_Settings.m_fForceHeat);
		m_GM.NotifyPlayer(playerId, m_Hub.Title(s), "The frame splinters and the lock tears out of it. The door's open, and it won't lock again for hours. Everything for a mile heard that.");
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_LootTable TableFor(string type)
	{
		RZ_LootTable t;
		if (!m_mTables.Find(type, t))
			m_mTables.Find("house", t);

		return t;
	}

	//------------------------------------------------------------------------------------------------
	//! The rare and electronics chances for a find here.
	protected void Odds(string type, vector pos, bool locked, bool body, out float rare, out float electronics)
	{
		rare = m_Settings.m_fRareChance;
		if (locked)
			rare = rare * 2;

		// Season rules (Bastion's season, Military Collapse): more military kit at police
		// stations and on bodies
		if (body || type == "police")
			rare = Math.Min(0.5, rare * RZ_Seasons.Mods().m_fMilitaryLootMult);

		electronics = m_Settings.m_fElectronicsChance * RZ_PowerGrid.LootMult(pos, "electronics");

		// Near the line less was taken (people fled inward): rare finds are better (not on bodies)
		if (!body)
			rare = Math.Min(0.5, rare * NearLineMult(pos));
	}

	//------------------------------------------------------------------------------------------------
	//! The Perimeter is on and running (its Features switch and the system itself).
	protected RZ_Perimeter LivePerimeter()
	{
		if (!RZ_Features.On(RZ_EFeature.PERIMETER))
			return null;

		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		if (!perimeter || !perimeter.IsActive())
			return null;

		return perimeter;
	}

	//------------------------------------------------------------------------------------------------
	//! Near line (x the rare chance): Near line mult within Near line reach inside the Perimeter's
	//! edge, else 1. 1 with the Perimeter off.
	float NearLineMult(vector pos)
	{
		RZ_Perimeter perimeter = LivePerimeter();
		if (!perimeter || m_Settings.m_fNearLineMult <= 1)
			return 1;

		float edge = perimeter.EdgeDistance(pos);
		if (edge < 0 || edge > m_Settings.m_fNearLineReach)
			return 1;

		return m_Settings.m_fNearLineMult;
	}

	//------------------------------------------------------------------------------------------------
	//! A wreck here stands on the old evacuation road: within Evacuation road of a sealed road
	//! checkpoint, with the Perimeter on.
	bool IsEvacRoad(vector pos)
	{
		if (m_Settings.m_fEvacReach <= 0)
			return false;

		RZ_Perimeter perimeter = LivePerimeter();
		return perimeter && perimeter.GetCheckpointDistance(pos) <= m_Settings.m_fEvacReach;
	}

	//------------------------------------------------------------------------------------------------
	//! What lies in a building of this type, as prefabs: up to `slots` items, each there with Find
	//! chance x the town's loot level (+0.25 behind a lock); `sure`: every slot holds something.
	//! For a one-off find (a body's kit, a stranger's gift); loose loot uses PickHouseLootAt.
	void PickHouseLoot(string type, vector pos, bool locked, int slots, bool sure, notnull array<ResourceName> picks)
	{
		int sureSlots = 0;
		if (sure)
			sureSlots = slots;

		PickHouseLootAt(type, pos, locked, slots, sureSlots, m_Settings.m_fFindChance, picks);
	}

	//------------------------------------------------------------------------------------------------
	//! Loose loot (RZ_Loot): up to `slots` items of this type's table. The first `sure` always hold
	//! something; each other one holds something `chance` x the town's loot level of the time (+0.25
	//! behind a lock, at most 0.95).
	void PickHouseLootAt(string type, vector pos, bool locked, int slots, int sure, float chance, notnull array<ResourceName> picks)
	{
		RZ_LootTable t = TableFor(type);
		if (!t || slots <= 0)
			return;

		float odds = chance * RZ_TownMemory.GetLootLevel(pos);
		if (locked)
			odds += 0.25;

		float rare;
		float electronics;
		Odds(type, pos, locked, false, rare, electronics);
		for (int i = 0; i < slots; i++)
		{
			if (i >= sure && Math.RandomFloat01() >= Math.Clamp(odds, 0, 0.95))
				continue;

			ResourceName pick = PickFresh(t, type, locked, rare, electronics, picks);
			if (!pick.IsEmpty())
				picks.Insert(pick);
		}

		// Near the Long Table's farm: sometimes one of their jars (World/RZ_LongTableReach.c)
		RZ_LongTableReach.AddLoot(pos, picks);
	}

	//------------------------------------------------------------------------------------------------
	//! A pick that isn't in `picks` yet, unless it stacks (ammo, food, dressings: two of those in one
	//! place is normal), and may lie in this kind of place (the high medicine only at a hospital or a
	//! locked pharmacy: RZ_LootGroups.AllowedAt). A few tries, then "" (the slot stays empty).
	protected ResourceName PickFresh(RZ_LootTable t, string kind, bool locked, float rare, float electronics, array<ResourceName> picks)
	{
		for (int attempt = 0; attempt < 8; attempt++)
		{
			ResourceName pick = PickFrom(t, kind, rare, electronics);
			if (pick.IsEmpty())
				return pick;

			if (!RZ_LootGroups.AllowedAt(pick, kind, locked))
				continue;

			if (picks.Find(pick) < 0 || Stacks(pick))
				return pick;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! One everyday find for this kind (RZ_Loot's caps: in place of a capped rare one). "" if none.
	ResourceName PickEverydayFor(string kind, bool locked)
	{
		RZ_LootTable t = TableFor(kind);
		for (int attempt = 0; attempt < 6; attempt++)
		{
			ResourceName pick = PickEveryday(kind, t);
			if (pick.IsEmpty() || RZ_LootGroups.AllowedAt(pick, kind, locked))
				return pick;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The sizes of a kind's table, for the test: everyday, rare, electronics.
	vector TableCounts(string kind)
	{
		RZ_LootTable t;
		if (!m_mTables.Find(kind, t))
			return vector.Zero;

		return Vector(t.m_aCommon.Count(), t.m_aRare.Count(), t.m_aElectronics.Count());
	}

	//------------------------------------------------------------------------------------------------
	//! A kind's table has this prefab (any list), for the test.
	bool TableHas(string kind, ResourceName prefab)
	{
		RZ_LootTable t;
		if (!m_mTables.Find(kind, t))
			return false;

		return HasSame(t.m_aCommon, prefab) || HasSame(t.m_aRare, prefab) || HasSame(t.m_aElectronics, prefab);
	}

	//------------------------------------------------------------------------------------------------
	//! Things that come in twos: ammunition, food, dressings, tablets.
	protected bool Stacks(ResourceName prefab)
	{
		if (!m_aStackWords)
		{
			m_aStackWords = new array<string>();
			RZ_ItemHelper.Words("magazine,ammo,/food/,dressing,gauze,bandage,tablet", m_aStackWords);
		}

		string name = prefab;
		name.ToLower();
		return RZ_ItemHelper.NameHas(name, m_aStackWords);
	}

	//------------------------------------------------------------------------------------------------
	//! What a wreck's boot holds (RZ_WreckBoots), as prefabs: something Boot chance x the town's
	//! loot level of the time, then a second thing Boot two chance x the level. `stash` (a stash
	//! someone wrote about): +0.25, two half the time or more, and twice the rare chance. No prefab
	//! twice unless it stacks.
	void PickBootLoot(vector pos, bool stash, notnull array<ResourceName> picks)
	{
		RZ_LootTable t = TableFor("car");
		if (!t)
			return;

		float level = RZ_TownMemory.GetLootLevel(pos);
		float chance = m_Settings.m_fBootChance * level;
		// The old evacuation road: the last people who tried to drive out left fuller boots
		bool evac = IsEvacRoad(pos);
		if (stash || evac)
			chance += 0.25;

		if (Math.RandomFloat01() >= Math.Clamp(chance, 0, 0.95))
			return;

		int count = 1;
		float twoChance = m_Settings.m_fBootTwoChance * level;
		if (stash || evac)
			twoChance = Math.Max(0.5, twoChance);

		if (Math.RandomFloat01() < twoChance)
			count = 2;

		float rare;
		float electronics;
		Odds("car", pos, stash, false, rare, electronics);
		for (int i = 0; i < count; i++)
		{
			ResourceName pick = PickFresh(t, "car", false, rare, electronics, picks);
			if (!pick.IsEmpty())
				picks.Insert(pick);
		}

		// The evacuation road's boot: one more thing a family packed to leave (a bag, food for days,
		// a first-aid kit)
		RZ_LootTable evacTable = TableFor("evac");
		if (!evac || !evacTable)
			return;

		float evacRare;
		float evacElectronics;
		Odds("evac", pos, true, false, evacRare, evacElectronics);
		ResourceName extra = PickFresh(evacTable, "evac", false, evacRare, 0, picks);
		if (!extra.IsEmpty())
			picks.Insert(extra);
	}

	//------------------------------------------------------------------------------------------------
	//! Electronics, a rare find, or an everyday one by the kind's mix (PickEveryday).
	protected ResourceName PickFrom(RZ_LootTable t, string kind, float rare, float electronics)
	{
		float roll = Math.RandomFloat01();
		if (roll < electronics && !t.m_aElectronics.IsEmpty())
			return t.m_aElectronics.GetRandomElement();

		if (roll < electronics + rare && !t.m_aRare.IsEmpty())
			return t.m_aRare.GetRandomElement();

		ResourceName everyday = PickEveryday(kind, t);
		if (!everyday.IsEmpty())
			return everyday;

		if (m_Settings.m_aLootPool && !m_Settings.m_aLootPool.IsEmpty())
			return m_Settings.m_aLootPool.GetRandomElement();

		return "";
	}

	// ================================================================ Readable things
	//------------------------------------------------------------------------------------------------
	//! An ID card, a letter, a receipt or a photo, in the Zone's voice. Sometimes a key is taped
	//! inside (keyId and keyTag set: the caller places or gives the key), a stash is written on the
	//! back, or a town is circled. Registers it and returns its id (-1 if none); `line` is
	//! "Title: text" for a search message. loose = it lies in a building's loose loot; body = on an
	//! infected body (more often an ID card; a key taped inside lies by it, as in a building).
	int CreateReadable(vector pos, bool body, bool loose, out string keyId, out string keyTag, out string line)
	{
		keyId = "";
		keyTag = "";
		line = "";
		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		int town = m_GM.FindNearestSettlement(pos);
		RZ_Settlement here = m_GM.GetSettlement(town);
		string place = "the village";
		if (here)
			place = here.m_sName;

		int day = 11;
		if (m_GM.GetBroadcast())
			day = m_GM.GetBroadcast().GetDay();

		string name = RZ_NoteText.FullName(rng);
		string first = RZ_NoteText.FirstName(rng);
		int kind = rng.Int(0, 4);
		if (body && rng.Chance(0.5))
			kind = 0;

		string title;
		string text;
		switch (kind)
		{
			case 0:
			{
				title = "An ID card";
				text = string.Format("An identity card: %1, resident of %2. The photo is someone smiling at a camera in a different world. On the back, in biro: \"%3\"", name, place, RZ_NoteText.PersonalLine(rng, place));
				break;
			}
			case 1:
			{
				title = "A letter";
				text = string.Format("\"%1, if you get this we've gone. The radio said stay indoors, so we stayed indoors, and then the water stopped. %2\" %3", first, RZ_NoteText.PersonalLine(rng, place), RZ_NoteText.Signoff(rng, RZ_NoteText.FirstName(rng)));
				break;
			}
			case 2:
			{
				array<string> goods = {"tinned peaches", "batteries (all of them)", "bleach", "candles", "a tin opener", "dog food", "rice, 5 kg", "plasters"};
				rng.ShuffleStrings(goods);
				title = "A receipt";
				text = string.Format("A till receipt from %1, %2: %3, %4, %5. Paid in cash. Underneath, in pencil: \"enough for a week?\"", place, RZ_NoteText.Day(Math.Max(1, day - rng.Int(8, 11))), goods[0], goods[1], goods[2]);
				break;
			}
			default:
			{
				title = "A photo";
				text = string.Format("A photograph: two people squinting into the sun outside a house in %1. On the back: \"%2 and me, summer. Before.\"", place, first);
				break;
			}
		}

		// Sometimes it points somewhere
		float roll = rng.Float01();
		if (roll < 0.2)
		{
			if (PickRandomKey(pos, keyId, keyTag))
			{
				if (loose || body)
					text += string.Format(" A key was taped inside, with a cardboard tag: \"%1\". It's lying where it fell.", keyTag);
				else
					text += string.Format(" Taped inside: a key with a cardboard tag: \"%1\". You keep it.", keyTag);
			}
		}
		else if (roll < 0.35)
		{
			string stash = AddStash(pos, rng);
			if (!stash.IsEmpty())
				text += " " + stash;
		}
		else if (roll < 0.45)
		{
			string puzzle = PuzzleHint(pos);
			if (!puzzle.IsEmpty())
				text += " " + puzzle;
		}

		// The epilogue week: 3 in 10 are dated in it (never naming the farm's town)
		if (RZ_Epilogue.Get())
		{
			string dated = RZ_Epilogue.Get().DatedReadable(place, town);
			if (!dated.IsEmpty())
			{
				title = "A note";
				text = dated;
			}
		}

		line = string.Format("%1: %2", title, text);
		return AddNote(title, text, loose);
	}

	//------------------------------------------------------------------------------------------------
	//! A stash near a landmark in a town nearby. "" if none could be placed.
	protected string AddStash(vector from, RZ_Rng rng)
	{
		int idx = RZ_Mission.PickSettlementAway(m_GM, from, 0, 2500, 0, 85);
		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (!s)
			return "";

		array<string> kinds = {"church", "school", "station", "bank", "fuel", "watertower"};
		RZ_Landmark l = RZ_LandmarkFinder.FindNearest(s.m_vCenter, s.m_fRadius + 100, kinds);
		RZ_Stash stash = new RZ_Stash();
		string near = "the middle of " + s.m_sName;
		stash.m_vPos = RZ_Mission.LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.4);
		if (l)
		{
			stash.m_vPos = l.m_vPos;
			near = string.Format("%1 in %2", l.m_sName, s.m_sName);
		}

		stash.m_sPlace = near;
		stash.m_fUntil = m_Hub.GetClock() + STASH_MINUTES * 60;
		m_aStashes.Insert(stash);
		while (m_aStashes.Count() > STASHES_KEPT)
		{
			m_aStashes.RemoveOrdered(0);
		}

		return string.Format("On the back, in a hurry: \"Left the good things in the house nearest %1. Grid %2. Don't let Pavel have them.\"", near, RZ_SettlementScanner.GridReference(stash.m_vPos));
	}

	//------------------------------------------------------------------------------------------------
	//! A stash someone wrote about lies within reach of pos: it's used up and true (the building
	//! placed there, or a wreck's boot filled there, holds it).
	bool TakeStashNear(vector pos)
	{
		float now = m_Hub.GetClock();
		foreach (RZ_Stash st : m_aStashes)
		{
			if (now < st.m_fUntil && RZ_Util.DistXZ(st.m_vPos, pos) <= STASH_RADIUS)
			{
				m_aStashes.RemoveItem(st);
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A puzzle mission in progress, circled on a map. "" if none.
	protected string PuzzleHint(vector from)
	{
		foreach (RZ_Mission m : m_GM.GetMissions().GetActive())
		{
			if (!RZ_PuzzleMission.Cast(m) || m.m_bGlobal)
				continue;

			return string.Format("Folded into it, a road map with %1 circled twice, and \"ask the right questions\" in the margin.", m.GetLocationName(m_GM));
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Registers a readable thing (no prop yet). -1 without a note prefab.
	protected int AddNote(string title, string text, bool loose)
	{
		if (m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CLUE).IsEmpty())
			return -1;

		RZ_Readable r = new RZ_Readable();
		r.m_iId = m_iNextNote;
		m_iNextNote++;
		r.m_sTitle = title;
		r.m_sText = text;
		r.m_fAt = m_Hub.GetClock();
		r.m_bLoose = loose;
		m_aNotes.Insert(r);
		PruneNotes();
		return r.m_iId;
	}

	//------------------------------------------------------------------------------------------------
	//! The oldest left by searches beyond NOTES_KEPT go, and loose ones beyond LOOSE_NOTES_KEPT.
	protected void PruneNotes()
	{
		int searched = 0;
		int loose = 0;
		for (int i = m_aNotes.Count() - 1; i >= 0; i--)
		{
			RZ_Readable r = m_aNotes[i];
			if (r.m_bLoose)
				loose++;
			else
				searched++;

			if ((!r.m_bLoose && searched > NOTES_KEPT) || (r.m_bLoose && loose > LOOSE_NOTES_KEPT))
				RemoveNote(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Readable FindNote(int id)
	{
		foreach (RZ_Readable r : m_aNotes)
		{
			if (r.m_iId == id)
				return r;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The note prop for a readable thing, lying at pos. null if it's gone or didn't spawn.
	IEntity SpawnNote(int id, vector pos, float yaw)
	{
		RZ_Readable r = FindNote(id);
		if (!r)
			return null;

		if (r.m_Body)
			return r.m_Body;

		r.m_Body = RZ_Util.SpawnPrefab(m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CLUE), pos, yaw);
		if (!r.m_Body)
			return null;

		RZ_ClueComponent.Tag(r.m_Body, RZ_Interactions.TAG_NOTE, r.m_iId, "Read it: " + r.m_sTitle);
		return r.m_Body;
	}

	//------------------------------------------------------------------------------------------------
	//! Its prop goes (the building's loot was put away); the text stays.
	void DespawnNote(int id)
	{
		RZ_Readable r = FindNote(id);
		if (r && r.m_Body)
			SCR_EntityHelper.DeleteEntityAndChildren(r.m_Body);

		if (r)
			r.m_Body = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Gone for good (its building was rolled again or forgotten).
	void ReleaseNote(int id)
	{
		for (int i = m_aNotes.Count() - 1; i >= 0; i--)
		{
			if (m_aNotes[i].m_iId == id)
				RemoveNote(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveNote(int idx)
	{
		RZ_Readable r = m_aNotes[idx];
		if (r.m_Body)
			SCR_EntityHelper.DeleteEntityAndChildren(r.m_Body);

		m_aNotes.RemoveOrdered(idx); // oldest first stays oldest first
	}

	//------------------------------------------------------------------------------------------------
	void ReadNote(int playerId, int id)
	{
		RZ_Readable r = FindNote(id);
		if (r)
		{
			RZ_Reader.Show(m_GM, playerId, r.m_sTitle, "", r.m_sText);
			return;
		}

		m_GM.NotifyPlayer(playerId, "Nothing", "The ink has run. You can't make out a word.");
	}

	//------------------------------------------------------------------------------------------------
	//! Readable things a search left don't stay: gone after half an hour (no physical traces).
	//! Loose ones come and go with their building's loot (RZ_Loot).
	void Tick(float dt)
	{
		float now = m_Hub.GetClock();
		for (int i = m_aNotes.Count() - 1; i >= 0; i--)
		{
			RZ_Readable r = m_aNotes[i];
			if (!r.m_bLoose && (now - r.m_fAt >= NOTE_MINUTES * 60 || !r.m_Body))
				RemoveNote(i);
		}

		for (int j = m_aStashes.Count() - 1; j >= 0; j--)
		{
			if (now >= m_aStashes[j].m_fUntil)
				m_aStashes.RemoveOrdered(j);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! Dead characters near a point (the infected who just fell there), nearest first. Dynamic
//! entities only; one query per kill near a player.
class RZ_CorpseQuery
{
	protected ref array<IEntity> m_aFound = {};

	//------------------------------------------------------------------------------------------------
	static void FindDead(vector center, float radius, notnull array<IEntity> result)
	{
		RZ_CorpseQuery q = new RZ_CorpseQuery();
		GetGame().GetWorld().QueryEntitiesBySphere(center, radius, q.OnEntity, null, EQueryEntitiesFlags.DYNAMIC);
		while (!q.m_aFound.IsEmpty())
		{
			int best = 0;
			for (int i = 1; i < q.m_aFound.Count(); i++)
			{
				if (RZ_Util.DistXZ(q.m_aFound[i].GetOrigin(), center) < RZ_Util.DistXZ(q.m_aFound[best].GetOrigin(), center))
					best = i;
			}

			result.Insert(q.m_aFound[best]);
			q.m_aFound.Remove(best);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnEntity(IEntity e)
	{
		if (m_aFound.Count() >= 8)
			return false;

		if (ChimeraCharacter.Cast(e) && !RZ_Util.IsCharacterAlive(e) && GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(e) <= 0)
			m_aFound.Insert(e);

		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! The built-in vanilla items per building type: what a table gets when the loot pool (and the
//! tables set by hand) leave it thin, so a pharmacy has medicine and a garage tools even with an
//! empty pool. Base-game items only; one that doesn't spawn is skipped where it's rolled.
class RZ_LootDefaults
{
	static const int THIN_COMMON = 8;	// fewer everyday finds than this: the built-in ones are added
	static const int THIN_RARE = 3;

	// Food and water
	protected static const string MRE = "{EB21D26B8D5A3232}Prefabs/Items/Food/MRE.et";
	protected static const string CRACKERS = "{50A68300B537EDFA}Prefabs/Items/Food/ArmyCrackers_Soviet_01.et";
	protected static const string MEAT = "{C22A2F123CD609FC}Prefabs/Items/Food/Meat_01_Dry.et";
	protected static const string CANTEEN_SU = "{70BC751317551D9B}Prefabs/Items/Equipment/Canteens/Canteen_Soviet_01.et";
	protected static const string CANTEEN_US = "{630FB4AD4A735264}Prefabs/Items/Equipment/Canteens/Canteen_US_01.et";
	// Medicine
	protected static const string DRESSING_US = "{A81F501D3EF6F38E}Prefabs/Items/Medicine/FieldDressing_01/FieldDressing_US_01.et";
	protected static const string DRESSING_SU = "{C3F1FA1E2EC2B345}Prefabs/Items/Medicine/FieldDressing_01/FieldDressing_USSR_01.et";
	protected static const string GAUZE_1 = "{CDCC313B2FB76FE7}Prefabs/Items/Medicine/FieldDressing_01/Gauze_01.et";
	protected static const string GAUZE_2 = "{AF3411AAF94D18C2}Prefabs/Items/Medicine/Gauze_02/Gauze_02.et";
	protected static const string ASPIRIN = "{C98FCC6CA6C56069}Prefabs/Items/Medicine/AspirinBottle.et";
	protected static const string TOURNIQUET_US = "{D70216B1B2889129}Prefabs/Items/Medicine/Tourniquet_01/Tourniquet_US_01.et";
	protected static const string TOURNIQUET_SU = "{80E75A71C29190DB}Prefabs/Items/Medicine/Tourniquet_01/Tourniquet_USSR_01.et";
	protected static const string SALINE_US = "{00E36F41CA310E2A}Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_01.et";
	protected static const string SALINE_SU = "{527D7C5D2E476BDC}Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_01.et";
	protected static const string MORPHINE = "{0D9A5DCF89AE7AA9}Prefabs/Items/Medicine/MorphineInjection_01/MorphineInjection_01.et";
	protected static const string GLOVES_MED = "{D90A181625AD1F9A}Prefabs/Characters/Handwear/Gloves_White/Medical_Gloves_White_Base.et";
	protected static const string MED_BAG = "{79015191C0E77E48}Prefabs/Items/Equipment/Backpacks/Backpack_Medical_Soviet_02.et";
	// Tools and kit
	protected static const string JERRYCAN = "{12D5AD21E383B768}Prefabs/Items/Fuel/Jerrycan_01/Jerrycan_01_item.et";
	protected static const string WRENCH = "{33B2DFDCD0EBA3DB}Prefabs/Items/Equipment/Kits/RepairKit_01/RepairKit_01_wrench.et";
	protected static const string TORCH_SU = "{575EA58E67448C2A}Prefabs/Items/Equipment/Flashlights/Flashlight_Soviet_01/Flashlight_Soviet_01.et";
	protected static const string TORCH_US = "{3A421547BC29F679}Prefabs/Items/Equipment/Flashlights/Flashlight_MX991/Flashlight_MX991.et";
	protected static const string COMPASS = "{7CEF68E2BC68CE71}Prefabs/Items/Equipment/Compass/Compass_Adrianov.et";
	protected static const string COMPASS_US = "{61D4F80E49BF9B12}Prefabs/Items/Equipment/Compass/Compass_SY183.et";
	protected static const string BINOCULARS = "{243948B23D90BECB}Prefabs/Items/Equipment/Binoculars/Binoculars_B8/Binoculars_B8.et";
	protected static const string WATCH = "{61A705D76908160C}Prefabs/Items/Equipment/Watches/Watch_Orlik38/Watch_Orlik38.et";
	protected static const string WATCH_2 = "{6FD6C96121905202}Prefabs/Items/Equipment/Watches/Watch_Vostok.et";
	protected static const string MAP = "{13772C903CB5E4F7}Prefabs/Items/Equipment/Maps/Map_Paper_01/PaperMap_01_folded.et";
	protected static const string RADIO = "{73950FBA2D7DB5C5}Prefabs/Items/Equipment/Radios/Radio_ANPRC68.et";
	protected static const string BAG = "{41A9C55B61F375F0}Prefabs/Items/Equipment/Backpacks/Backpack_Kolobok.et";
	// Ammunition and police kit
	protected static const string MAG_PM = "{8B853CDD11BA916E}Prefabs/Weapons/Magazines/Magazine_9x18_PM_8rnd_Ball.et";
	protected static const string MAG_M9 = "{9C05543A503DB80E}Prefabs/Weapons/Magazines/Magazine_9x19_M9_15rnd_Ball.et";
	protected static const string MAG_AK = "{BBB50A815A2F916B}Prefabs/Weapons/Magazines/Magazine_545x39_AK_30rnd_Ball.et";
	protected static const string PISTOL = "{C0F7DD85A86B2900}Prefabs/Weapons/Handguns/PM/Handgun_PM.et";
	protected static const string VEST = "{4B57C11AA5161760}Prefabs/Characters/Vests/Vest_PASGT/Vest_PASGT.et";
	protected static const string HELMET = "{E503F84CD6E31B21}Prefabs/Characters/HeadGear/Helmet_PASGT_01/Helmet_PASGT_Black.et";
	protected static const string PATROL_CAP = "{7891C761B31741BD}Prefabs/Characters/HeadGear/Hat_Patrol_US_01/Hat_Patrol_US_dark.et";
	// Clothes
	protected static const string HAT_KNIT = "{BE19A15F1073E7BD}Prefabs/Characters/HeadGear/Hat_Knitted_01/Hat_Knitted_01_black.et";
	protected static const string HAT_FLAT = "{67662815EFF734DD}Prefabs/Characters/HeadGear/Hat_FlatCap_01/Hat_FlatCap_01_darkgrey.et";
	protected static const string HAT_HARD = "{8F5261C6B57B95C8}Prefabs/Characters/HeadGear/Hat_Hard_01/Hat_Hard_01_yellow.et";
	protected static const string GLOVES_WOOL = "{8266820FFDE17477}Prefabs/Characters/Handwear/Gloves_Wool_01/Gloves_Wool_01.et";
	protected static const string GLOVES_LEATHER = "{4F71E7468E78DDD2}Prefabs/Characters/Handwear/Gloves_Leather_01/Gloves_Leather_01.et";
	protected static const string SWEATSHIRT = "{3DA87A330B897C58}Prefabs/Characters/Uniforms/Sweatshirt/Sweatshirt_Red.et";
	protected static const string JEANS = "{C40CEB198DAABF5B}Prefabs/Characters/Uniforms/Pants_Jeans/Pants_Jeans_01.et";
	protected static const string SHIRT = "{CD87BF4D0F940AB3}Prefabs/Characters/Uniforms/Shirt/Shirt.et";
	protected static const string RAINCOAT = "{7F1451790F3D75BD}Prefabs/Characters/Uniforms/Jacket_Raincoat_01/Jacket_Raincoat_01_green.et";
	protected static const string FISHER_PANTS = "{0B1F0174869E0351}Prefabs/Characters/Uniforms/Pants_Fisherman_01/Pants_Fisherman_01_green.et";
	protected static const string OVERALLS = "{DFE230A386D20BFB}Prefabs/Characters/Uniforms/Overalls/Mechanic_uniform.et";
	protected static const string SNEAKERS = "{553F83578D29670C}Prefabs/Characters/Footwear/Sneakers/Sneakers_Black.et";
	// The new kinds (docs/DESIGN.md, "Building kinds"): base-game items and the mod's own crafting things
	protected static const string WATER = "{D5E10AA9B0808D76}prefabs/Items/Water/WaterBottle.et";
	protected static const string MEDKIT_US = "{AE578EEA4244D41F}Prefabs/Items/Equipment/Kits/MedicalKit_01/MedicalKit_01_US.et";
	protected static const string MEDKIT_SU = "{21EF98BFC1EB3793}Prefabs/Items/Equipment/Kits/MedicalKit_01/MedicalKit_01_USSR.et";
	protected static const string HAZMAT = "{BF54108D935086E5}Prefabs/Characters/Uniforms/Suit_Hazmat/Suit_Hazmat_Hood_HiVis.et";
	protected static const string GAS_MASK = "{FB66C1EC2E5DB34C}Prefabs/Clothing/Gas_Mask.et";
	protected static const string HELMET_FIRE = "{5C7E23A702959B1C}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_CIV_Firefighter.et";
	protected static const string FIRE_AXE = "{87FAD1AE2D462BD2}Prefabs/Weapons/Melee/Fire Axe.et";
	protected static const string OXYGEN = "{CAB1F18E563F8489}Prefabs/Items/Equipment/Backpacks/firefighter oxygen tank.et";
	protected static const string FIRE_SUIT = "{C347738A4C9E58EE}Prefabs/Characters/Uniforms/Suit_Pilot_US_01/firefighter_suit.et";
	protected static const string PAD = "{268E3FCBD96622AD}Prefabs/ReforgerZ/Crafting/RZ_WritingPad.et";
	protected static const string CASH = "{696CC1F6BC5BE3F7}PrefabsEditable/Auto/Props/Civilian/Suspicious Items/E_StackofCash.et";
	protected static const string CARDS = "{D0476EFD1BB2CCB3}PrefabsEditable/Auto/Props/Civilian/Suspicious Items/E_CreditCards.et";
	protected static const string GOLD = "{7B605E8E4B952837}PrefabsEditable/Auto/Props/Civilian/Suspicious Items/E_GoldBrick.et";
	protected static const string RADIO_PORTABLE = "{842711C53E9F890C}Prefabs/Items/Equipment/Misc/Portable_Radio.et";
	protected static const string WIRE = "{2E0FAA5F39CBA66C}Prefabs/ReforgerZ/Crafting/RZ_Wire.et";
	protected static const string NAILS = "{267D3FAF0F92499F}Prefabs/ReforgerZ/Crafting/RZ_Nails.et";
	protected static const string SCRAP = "{6CB732A4814E2F43}Prefabs/ReforgerZ/Crafting/RZ_Scrap.et";
	protected static const string FUSE = "{01C470CBEFA41D39}Prefabs/ReforgerZ/Crafting/RZ_Fuse.et";
	protected static const string CELLS = "{F51872C09AA3A89A}Prefabs/ReforgerZ/Crafting/RZ_Cells.et";
	protected static const string PLANKS = "{0A101873E8C9274C}Prefabs/ReforgerZ/Interactions/RZ_Planks.et";
	protected static const string WOOD = "{B5B3D0AFBB4DE757}Prefabs/Items/Core/FM_WoodStick.et";
	protected static const string ETOOL = "{0619993AE6CDBF85}Prefabs/Items/Equipment/Accessories/ETool_ALICE/ETool_ALICE.et";
	protected static const string KNIFE = "{3805C0B87B9D2AB0}prefabs/weapons/Melee/Knife.et";
	protected static const string BOONIE = "{1E8E0AF46736D299}Prefabs/Characters/HeadGear/Hat_Boonie_US_01/Hat_Boonie_US_dark.et";
	protected static const string RAINCOAT_US = "{7B8BBB818BD62015}Prefabs/Characters/Uniforms/Jacket_Raincoat_01/US RAINCOAT.et";
	protected static const string PRISONER = "{EF0516E165E01874}Prefabs/Characters/Uniforms/Overalls/Prisoner_uniform.et";
	protected static const string MAG_VZ58 = "{48720FC416263FC1}Prefabs/Weapons/Magazines/Vz58/Magazine_762x39_Vz58_30rnd_Ball.et";
	protected static const string MAG_STANAG = "{2EBF60EF24B108FC}Prefabs/Weapons/Magazines/Magazine_556x45_STANAG_30rnd_M855_Ball.et";
	protected static const string BDU = "{293F577C298061E3}Prefabs/Characters/Uniforms/Jacket_US_BDU_02.et";
	protected static const string M65 = "{86BB79B9A2233780}Prefabs/Characters/Uniforms/Pants_US_M65.et";
	protected static const string SMOKE_RU = "{77EAE5E07DC4678A}Prefabs/Weapons/Grenades/Smoke_RDG2.et";
	protected static const string SMOKE_US = "{D41D22DD1B8E921E}Prefabs/Weapons/Grenades/M18/Smoke_M18_Green.et";
	protected static const string RIFLE_M16 = "{3E413771E1834D2F}Prefabs/Weapons/Rifles/M16/Rifle_M16A2.et";
	protected static const string RIFLE_VZ58 = "{9C948630078D154D}Prefabs/Weapons/Rifles/VZ58/Rifle_VZ58P.et";
	protected static const string HELMET_ARMY = "{B74A4FF0DD8BB116}Prefabs/Characters/HeadGear/Helmet_PASGT_01/Helmet_PASGT_01.et";
	protected static const string VEST_6B3 = "{4CBDC206FEF9897C}Prefabs/Characters/Vests/Vest_6B3/Vest_6B3.et";
	protected static const string HIKING_BAG = "{A870260CF6ECB936}Prefabs/Items/Equipment/Backpacks/Backpack_IIFS_FieldPack_hiking.et";

	//------------------------------------------------------------------------------------------------
	static void For(string type, notnull array<ResourceName> common, notnull array<ResourceName> rare, notnull array<ResourceName> electronics)
	{
		electronics.Insert(TORCH_SU);
		electronics.Insert(TORCH_US);
		switch (type)
		{
			case "pharmacy":
			{
				array<string> common1 = {DRESSING_US, DRESSING_SU, GAUZE_1, GAUZE_2, ASPIRIN, ASPIRIN, TOURNIQUET_US, TOURNIQUET_SU, SALINE_SU, GLOVES_MED};
				Add(common, common1);
				array<string> rare2 = {MORPHINE, SALINE_US, MED_BAG};
				Add(rare, rare2);
				return;
			}
			case "garage":
			{
				array<string> common3 = {JERRYCAN, WRENCH, TORCH_SU, GLOVES_LEATHER, HAT_HARD, OVERALLS, CANTEEN_US, MAP};
				Add(common, common3);
				array<string> rare4 = {WRENCH, JERRYCAN, BAG};
				Add(rare, rare4);
				return;
			}
			case "farm":
			{
				array<string> common5 = {MEAT, CRACKERS, MRE, CANTEEN_SU, GLOVES_WOOL, HAT_KNIT, RAINCOAT, FISHER_PANTS, JERRYCAN};
				Add(common, common5);
				array<string> rare6 = {BINOCULARS, COMPASS_US, BAG};
				Add(rare, rare6);
				return;
			}
			case "police":
			{
				array<string> common7 = {MAG_PM, MAG_M9, TORCH_US, DRESSING_US, TOURNIQUET_US, PATROL_CAP, GLOVES_LEATHER, MAP};
				Add(common, common7);
				array<string> rare8 = {PISTOL, VEST, HELMET, RADIO};
				Add(rare, rare8);
				return;
			}
			case "shop":
			{
				array<string> common9 = {MRE, CRACKERS, MEAT, CANTEEN_US, CANTEEN_SU, WATCH, MAP, GLOVES_WOOL, HAT_KNIT};
				Add(common, common9);
				array<string> rare10 = {BINOCULARS, WATCH_2, COMPASS};
				Add(rare, rare10);
				return;
			}
			case "body":
			{
				array<string> common11 = {DRESSING_SU, GAUZE_2, CRACKERS, MAG_PM, MAG_AK, CANTEEN_SU, WATCH_2};
				Add(common, common11);
				array<string> rare12 = {MORPHINE, BINOCULARS, MAG_M9};
				Add(rare, rare12);
				return;
			}
			case "car":
			{
				array<string> common13 = {JERRYCAN, WRENCH, CANTEEN_US, MAP, TORCH_SU, CRACKERS};
				Add(common, common13);
				array<string> rare14 = {BINOCULARS, WRENCH};
				Add(rare, rare14);
				return;
			}
			case "hospital":
			{
				array<string> commonHosp = {DRESSING_US, DRESSING_SU, GAUZE_1, GAUZE_2, ASPIRIN, TOURNIQUET_US, TOURNIQUET_SU, SALINE_SU, SALINE_US, GLOVES_MED};
				Add(common, commonHosp);
				array<string> rareHosp = {MORPHINE, MEDKIT_US, MEDKIT_SU, MED_BAG, HAZMAT};
				Add(rare, rareHosp);
				return;
			}
			case "firestation":
			{
				array<string> commonFire = {HELMET_FIRE, GLOVES_LEATHER, OVERALLS, TORCH_SU, CANTEEN_US, HAT_HARD};
				Add(common, commonFire);
				array<string> rareFire = {FIRE_AXE, OXYGEN, FIRE_SUIT, GAS_MASK};
				Add(rare, rareFire);
				return;
			}
			case "school":
			{
				array<string> commonSchool = {CRACKERS, WATER, PAD, SNEAKERS, SWEATSHIRT, MAP};
				Add(common, commonSchool);
				array<string> rareSchool = {WATCH, BAG};
				Add(rare, rareSchool);
				return;
			}
			case "office":
			{
				array<string> commonOffice = {MAP, PAD, WATCH, WATCH_2, CASH, CARDS, WATER};
				Add(common, commonOffice);
				array<string> rareOffice = {GOLD, RADIO_PORTABLE};
				Add(rare, rareOffice);
				return;
			}
			case "church":
			{
				array<string> commonChurch = {CRACKERS, WATER, CANTEEN_SU};
				Add(common, commonChurch);
				array<string> rareChurch = {PAD};
				Add(rare, rareChurch);
				return;
			}
			case "industrial":
			{
				array<string> commonInd = {WIRE, NAILS, SCRAP, JERRYCAN, PLANKS, WOOD, OVERALLS, GLOVES_LEATHER, HAT_HARD};
				Add(common, commonInd);
				array<string> rareInd = {FUSE, WRENCH, ETOOL};
				Add(rare, rareInd);
				return;
			}
			case "hunting":
			{
				array<string> commonHunt = {KNIFE, BINOCULARS, COMPASS, MAP, RAINCOAT, BOONIE, CANTEEN_SU, MEAT};
				Add(common, commonHunt);
				array<string> rareHunt = {BINOCULARS, COMPASS_US, BAG};
				Add(rare, rareHunt);
				return;
			}
			case "coast":
			{
				array<string> commonCoast = {RAINCOAT, RAINCOAT_US, FISHER_PANTS, CRACKERS, WATER, CANTEEN_SU};
				Add(common, commonCoast);
				array<string> rareCoast = {BINOCULARS, BAG};
				Add(rare, rareCoast);
				return;
			}
			case "prison":
			{
				array<string> commonPrison = {PRISONER, CRACKERS};
				Add(common, commonPrison);
				return;
			}
			case "army":
			{
				array<string> commonArmy = {MAG_PM, MAG_AK, MAG_VZ58, MAG_STANAG, BDU, M65, MRE, CANTEEN_US, CANTEEN_SU, SMOKE_RU, SMOKE_US};
				Add(common, commonArmy);
				array<string> rareArmy = {RIFLE_M16, RIFLE_VZ58, HELMET_ARMY, VEST, VEST_6B3};
				Add(rare, rareArmy);
				return;
			}
			case "drone":
			{
				array<string> commonDrone = {CELLS, WIRE, SCRAP, FUSE, TORCH_SU};
				Add(common, commonDrone);
				array<string> rareDrone = {RADIO, RADIO_PORTABLE};
				Add(rare, rareDrone);
				return;
			}
			case "evac":
			{
				array<string> commonEvac = {MRE, CRACKERS, MEAT, CANTEEN_US, CANTEEN_SU, WATER, DRESSING_US};
				Add(common, commonEvac);
				array<string> rareEvac = {HIKING_BAG, BAG, MEDKIT_US};
				Add(rare, rareEvac);
				return;
			}
		}

		// A house: a bit of everything a family would leave
		array<string> common15 = {MRE, CRACKERS, MEAT, CANTEEN_SU, DRESSING_SU, GAUZE_1, ASPIRIN, WATCH_2, MAP, HAT_KNIT, HAT_FLAT, GLOVES_WOOL, SWEATSHIRT, JEANS, SHIRT, SNEAKERS};
		Add(common, common15);
		array<string> rare16 = {BINOCULARS, COMPASS, MAG_PM, BAG};
		Add(rare, rare16);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Add(notnull array<ResourceName> into, array<string> items)
	{
		foreach (string r : items)
		{
			into.Insert(r);
		}
	}
}
