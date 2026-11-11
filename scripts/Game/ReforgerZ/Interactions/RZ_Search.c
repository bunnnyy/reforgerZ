//! Loot rules, locks and keys, readable things, and the searches that are left: car boots
//! (RZ_Cars) and the bodies of infected killed near players (docs/DESIGN.md, "World
//! interactions"). Loot in houses lies loose (RZ_Loot.c) and is rolled here (PickHouseLoot).
//!
//! - Loot suits the building: pharmacy, garage, farm, police, shop or house, from the prefab
//!   name of the building. Tables are configurable per type; anything left empty is filled from
//!   one flat loot pool by keywords (bandage words for pharmacies, tool and fuel words for
//!   garages...). Scaled by the town's loot level (RZ_TownMemory.GetLootLevel); electronics get
//!   the power grid's multiplier (RZ_PowerGrid.LootMult).
//! - A car boot or a body is searched (a few seconds), rolled then, and remembered as searched
//!   for an hour.
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
	[Attribute("house", desc: "Building type: pharmacy, garage, farm, police, shop, house, body (infected bodies) or car (car boots)")]
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
	[Attribute("1", desc: "Searching car boots and infected bodies, locks and keys, readable things (loot in houses: Interactions > Loot)")]
	bool m_bEnabled;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Loot pool: every item that can lie in a house or turn up in a car boot or on a body. Tables left empty are filled from it by keywords in the prefab names (see docs/WORKBENCH_SETUP.md). Each must be a real item with vanilla's pick-up action", "et")]
	ref array<ResourceName> m_aLootPool;

	[Attribute(desc: "Loot tables per building type (optional; empty lists use the pool)")]
	ref array<ref RZ_LootTable> m_aTables;

	[Attribute("0.6", desc: "At full town loot (scaled by what the town has left): the chance each of a building's item slots holds something (loose loot), and that a car boot or body search finds anything")]
	float m_fFindChance;

	[Attribute("0.1", desc: "Chance a find is rare (doubled behind a lock)")]
	float m_fRareChance;

	[Attribute("0.1", desc: "Chance a find is electronics (x the power grid's electronics multiplier)")]
	float m_fElectronicsChance;

	[Attribute("60", desc: "Real minutes before a searched car boot has anything again")]
	float m_fSpotReturnMinutes;

	[Attribute("4", desc: "Seconds a car boot or body search takes (stay close)")]
	float m_fSearchSeconds;

	[Attribute("1", desc: "Heat a car boot or body search makes")]
	float m_fSearchHeat;

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
	[Attribute("0.12", desc: "Chance a building's loot (or a car boot) has something readable (ID card, letter, receipt, photo)")]
	float m_fReadableChance;

	[Attribute("0.3", desc: "Chance an infected body has something readable on it")]
	float m_fBodyReadableChance;

	[Attribute("0.08", desc: "Chance an infected body has a key on it")]
	float m_fBodyKeyChance;

	[Attribute("4", desc: "Real minutes an infected body can be searched (players within 60 m when it died)")]
	float m_fBodyMinutes;

	[Attribute("8", desc: "Most searchable bodies at once")]
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
//! A stash someone wrote about: the building or car boot nearest it holds it.
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
	protected static const float STASH_MINUTES = 90;
	protected static const int STASHES_KEPT = 6;
	protected static const float BODY_WITNESS = 60;

	protected RZ_GameModeComponent m_GM;
	protected RZ_Interactions m_Hub;
	protected RZ_SearchSettings m_Settings;
	protected ref map<string, ref RZ_LootTable> m_mTables = new map<string, ref RZ_LootTable>();
	protected ref map<string, float> m_mSearched = new map<string, float>();	// car boots
	protected ref map<string, float> m_mOpened = new map<string, float>();
	protected ref map<string, float> m_mForced = new map<string, float>();	// opened by force: broken until the reset
	protected ref array<ref RZ_Readable> m_aNotes = {};
	protected ref array<ref RZ_Stash> m_aStashes = {};
	protected ref array<string> m_aElectronicsWords = {};
	protected int m_iNextNote = 1;
	protected bool m_bKeyPrefabWarned;

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
		float keep = m_Settings.m_fSpotReturnMinutes * 60;
		foreach (string key, float at : m_mSearched)
		{
			if (now - at < keep)
			{
				data.searchedKey.Insert(key);
				data.searchedAt.Insert(at);
			}
		}

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
		for (int i = 0; i < data.searchedKey.Count() && i < data.searchedAt.Count(); i++)
		{
			m_mSearched.Set(data.searchedKey[i], data.searchedAt[i]);
		}

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

		array<string> types = {"pharmacy", "garage", "farm", "police", "shop", "house", "body", "car"};
		foreach (string type : types)
		{
			BuildTable(type);
		}

		int pool = 0;
		if (m_Settings.m_aLootPool)
			pool = m_Settings.m_aLootPool.Count();

		RZ_Log.Info(string.Format("Search: %1 items in the loot pool, %2 tables set by hand.", pool, CountConfigured()));

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

		array<string> common = {};
		array<string> rare = {};
		TypeWords(type, common, rare);
		if (t.m_aCommon.IsEmpty())
			FromPool(common, t.m_aCommon);

		if (t.m_aRare.IsEmpty())
			FromPool(rare, t.m_aRare);

		if (t.m_aElectronics.IsEmpty())
			FromPool(m_aElectronicsWords, t.m_aElectronics);

		m_mTables.Set(type, t);
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
			if (RZ_ItemHelper.NameHas(name, words))
				into.Insert(r);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Keyword fallbacks per building type: what's usually there, and what's rare.
	protected void TypeWords(string type, notnull array<string> common, notnull array<string> rare)
	{
		string c = "food,can,tin,bottle,water,bandage,cloth,rag,knife,flashlight,battery,pill,shirt,jacket,matches,lighter";
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
				c = "food,can,tin,bread,water,bottle,axe,hatchet,knife,rope,cloth,plank,nail,wire,jerrycan,matches";
				r = "shotgun,hunting,rifle,binocular";
				break;
			}
			case "police":
			{
				c = "ammo,magazine,mag_,baton,handcuff,flashlight,radio,bandage,tourniquet,vest";
				r = "pistol,shotgun,vest,helmet,radio";
				break;
			}
			case "shop":
			{
				c = "food,can,tin,bottle,water,battery,flashlight,cloth,matches,lighter,bleach,rope";
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
		}

		RZ_ItemHelper.Words(c, common);
		RZ_ItemHelper.Words(r, rare);
	}

	// ================================================================ Recognising things
	//------------------------------------------------------------------------------------------------
	//! The building type from its lower-case prefab or mesh name.
	string BuildingType(string name)
	{
		// Vet clinics stock what farms do (the veterinary antiviral), not a pharmacy's shelves
		if (name.Contains("veterin") || name.Contains("vet_") || name.Contains("vetclinic"))
			return "farm";

		if (RZ_LandmarkFinder.Matches("pharmacy", name) || name.Contains("drugstore") || name.Contains("apothec"))
			return "pharmacy";

		if (name.Contains("garage") || name.Contains("workshop") || name.Contains("carservice") || name.Contains("car_service") || name.Contains("autoservice") || name.Contains("repair") || name.Contains("mechanic"))
			return "garage";

		if (name.Contains("police") || name.Contains("gendarm") || name.Contains("sheriff"))
			return "police";

		if (name.Contains("farm") || name.Contains("barn") || name.Contains("stable") || name.Contains("shed") || name.Contains("silo") || name.Contains("cowshed"))
			return "farm";

		if (name.Contains("shop") || name.Contains("store") || name.Contains("market") || name.Contains("grocery") || name.Contains("kiosk"))
			return "shop";

		return "house";
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
	//! (RZ_MissionManager.IsNearMissionProp) unless the lock is part of that mission. Only a
	//! building with outside doors is really locked (RZ_DoorLocks.HasDoorLock).
	bool HasLock(vector buildingPos, string type)
	{
		if (!HasOwnLock(buildingPos, type))
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
			case "police": share = m_Settings.m_fLockPolice; break;
			case "garage": share = m_Settings.m_fLockGarage; break;
			case "shop": share = m_Settings.m_fLockShop; break;
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

	//------------------------------------------------------------------------------------------------
	//! A key (see PickRandomKey) straight into their inventory: a body. Returns a description, ""
	//! if none.
	string GiveRandomKey(int playerId, vector pos)
	{
		string keyId;
		string tag;
		if (!PickRandomKey(pos, keyId, tag))
			return "";

		GiveKey(playerId, keyId, tag);
		return string.Format("a key with a cardboard tag: \"%1\"", tag);
	}

	// ================================================================ Bodies
	//------------------------------------------------------------------------------------------------
	//! Infected that die with a player close by leave a body to search for a few minutes.
	void OnZombiesKilled(vector pos, int count)
	{
		if (!m_Settings.m_bEnabled || count <= 0)
			return;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		if (RZ_Util.NearestDistance(pos, players) > BODY_WITNESS)
			return;

		if (m_Hub.CountKind(RZ_Interactions.KIND_BODY) >= m_Settings.m_iBodyCap)
			return;

		m_Hub.AddSpot(RZ_Interactions.KIND_BODY, RZ_Util.GroundPos(pos), "body", "the body", -1, m_Hub.GetClock() + m_Settings.m_fBodyMinutes * 60);
	}

	//------------------------------------------------------------------------------------------------
	//! A body's spot: Look at the body, Search the body.
	void Labels(RZ_Spot s, out string read, out string choiceLabels)
	{
		read = "Look at the body";
		choiceLabels = "Search the body";
	}

	//------------------------------------------------------------------------------------------------
	void Describe(int playerId, RZ_Spot s)
	{
		m_GM.NotifyPlayer(playerId, m_Hub.Title(s), "It was someone, once. Pockets, a bag, a wallet maybe. Be quick: they don't lie still for long out here.");
	}

	//------------------------------------------------------------------------------------------------
	//! A body's one choice: search it.
	void HandleChoice(int playerId, RZ_Spot s, int option, IEntity user)
	{
		if (m_Settings.m_bEnabled && option == 0)
			StartSearch(playerId, s, user);
	}

	// ================================================================ Searching (car boots, bodies)
	//------------------------------------------------------------------------------------------------
	protected bool WasSearched(string key)
	{
		float at;
		return m_mSearched.Find(key, at) && m_Hub.GetClock() - at < m_Settings.m_fSpotReturnMinutes * 60;
	}

	//------------------------------------------------------------------------------------------------
	//! A search of a car boot or a body: a few seconds, then the roll.
	void StartSearch(int playerId, RZ_Spot s, IEntity user)
	{
		if (WasSearched(s.m_sKey))
		{
			m_GM.NotifyPlayer(playerId, m_Hub.Title(s), "You've been through it, or someone has. Nothing new. Give it time, or try somewhere else.");
			return;
		}

		m_Hub.StartTask(playerId, user, RZ_Interactions.TASK_SEARCH, s.m_iId, 0, m_Settings.m_fSearchSeconds, "the search", false);
	}

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
	void CompleteSearch(int playerId, RZ_Spot s, IEntity user)
	{
		if (!s)
			return;

		string title = m_Hub.Title(s);
		bool body = s.m_iKind == RZ_Interactions.KIND_BODY;
		m_mSearched.Set(s.m_sKey, m_Hub.GetClock());
		PruneSearched();
		m_GM.GetHeat().AddHeat(s.m_vPos, m_Settings.m_fSearchHeat);

		array<string> found = {};
		int items = RollLoot(s.m_sType, s.m_vPos, false, body, user, s.m_vPos, found);

		// A stash someone wrote about
		if (!body && TakeStashNear(s.m_vPos))
		{
			for (int i = 0; i < 2; i++)
			{
				items += RollLoot(s.m_sType, s.m_vPos, true, false, user, s.m_vPos, found);
			}

			found.Insert("the stash from the note, wrapped in a bin bag");
		}

		if (!body && items > 0)
			RZ_TownMemory.ReportSearched(s.m_vPos, m_Hub.GetLoot().GetTownLootPerItem() * items);

		// Treatments (docs/DESIGN.md, "Treatments"): one roll
		RZ_Infection infection = m_GM.GetInfection();
		if (infection)
		{
			string med = infection.RollMedFind(s.m_sType, false, user, s.m_vPos);
			if (!med.IsEmpty())
				found.Insert(med);
		}

		// A key on a body
		if (body && Math.RandomFloat01() < m_Settings.m_fBodyKeyChance)
		{
			string key = GiveRandomKey(playerId, s.m_vPos);
			if (!key.IsEmpty())
				found.Insert(key);
		}

		string text = "";
		if (found.IsEmpty())
			text = EmptyText(body, RZ_TownMemory.GetLootLevel(s.m_vPos));
		else
			text = "You find " + ListText(found) + ".";

		float readChance = m_Settings.m_fReadableChance;
		if (body)
			readChance = m_Settings.m_fBodyReadableChance;

		if (Math.RandomFloat01() < readChance)
		{
			string keyId;
			string keyTag;
			string line;
			int note = CreateReadable(s.m_vPos, body, false, keyId, keyTag, line);
			if (note >= 0)
			{
				// A search leaves the readable thing where it was, for others to read too
				SpawnNote(note, RZ_Util.RandomPointInRing(s.m_vPos, 0.4, 1) + Vector(0, 0.2, 0), Math.RandomFloat(0, 360));
				if (!keyId.IsEmpty())
					GiveKey(playerId, keyId, keyTag);

				text += "\n" + line;
			}
		}

		m_GM.NotifyPlayer(playerId, title, text);
		if (body)
			m_Hub.RemoveSpot(s.m_iId);
	}

	//------------------------------------------------------------------------------------------------
	protected void PruneSearched()
	{
		if (m_mSearched.Count() < 800)
			return;

		float now = m_Hub.GetClock();
		float keep = m_Settings.m_fSpotReturnMinutes * 60;
		array<string> old = {};
		foreach (string key, float at : m_mSearched)
		{
			if (now - at >= keep)
				old.Insert(key);
		}

		foreach (string k : old)
		{
			m_mSearched.Remove(k);
		}
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
	}

	//------------------------------------------------------------------------------------------------
	//! Loose loot (RZ_Loot): what lies in a building of this type, as prefabs. Up to `slots`
	//! items, each there with Find chance x the town's loot level (+0.25 behind a lock); `sure`:
	//! every slot holds something (a stash).
	void PickHouseLoot(string type, vector pos, bool locked, int slots, bool sure, notnull array<ResourceName> picks)
	{
		RZ_LootTable t = TableFor(type);
		if (!t || slots <= 0)
			return;

		float chance = m_Settings.m_fFindChance * RZ_TownMemory.GetLootLevel(pos);
		if (locked)
			chance += 0.25;

		float rare;
		float electronics;
		Odds(type, pos, locked, false, rare, electronics);
		for (int i = 0; i < slots; i++)
		{
			if (!sure && Math.RandomFloat01() >= Math.Clamp(chance, 0, 0.95))
				continue;

			ResourceName pick = PickFrom(t, rare, electronics);
			if (!pick.IsEmpty())
				picks.Insert(pick);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Rolls what a car boot or body search finds and gives it. Returns how many items. Names go
	//! into `found`.
	protected int RollLoot(string type, vector pos, bool locked, bool body, IEntity user, vector at, notnull array<string> found)
	{
		RZ_LootTable t = TableFor(type);
		if (!t)
			return 0;

		float level = 1;
		if (!body)
			level = RZ_TownMemory.GetLootLevel(pos);

		float chance = m_Settings.m_fFindChance * level;
		if (locked)
			chance += 0.25;

		if (Math.RandomFloat01() >= Math.Clamp(chance, 0, 0.95))
			return 0;

		int count = 1;
		float twoChance = 0.2 * level;
		if (locked)
			twoChance = 0.5;

		if (Math.RandomFloat01() < twoChance)
			count = 2;

		float rare;
		float electronics;
		Odds(type, pos, locked, body, rare, electronics);
		int given = 0;
		for (int i = 0; i < count; i++)
		{
			ResourceName pick = PickFrom(t, rare, electronics);
			if (pick.IsEmpty())
				continue;

			if (RZ_ItemHelper.Give(pick, user, at))
			{
				found.Insert(RZ_ItemHelper.DisplayName(pick));
				given++;
			}
		}

		return given;
	}

	//------------------------------------------------------------------------------------------------
	protected ResourceName PickFrom(RZ_LootTable t, float rare, float electronics)
	{
		float roll = Math.RandomFloat01();
		if (roll < electronics && !t.m_aElectronics.IsEmpty())
			return t.m_aElectronics.GetRandomElement();

		if (roll < electronics + rare && !t.m_aRare.IsEmpty())
			return t.m_aRare.GetRandomElement();

		if (!t.m_aCommon.IsEmpty())
			return t.m_aCommon.GetRandomElement();

		if (m_Settings.m_aLootPool && !m_Settings.m_aLootPool.IsEmpty())
			return m_Settings.m_aLootPool.GetRandomElement();

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected static string ListText(array<string> parts)
	{
		string text = "";
		foreach (int i, string p : parts)
		{
			if (i > 0 && i == parts.Count() - 1)
				text += " and ";
			else if (i > 0)
				text += ", ";

			text += p;
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	protected static string EmptyText(bool body, float level)
	{
		if (body)
			return "Nothing but a bus ticket and a photo too stained to make out.";

		if (level < 0.4)
			return "Empty. Somebody beat you to it, and somebody beat them.";

		return "A spare wheel with no air in it and an empty oil can.";
	}

	// ================================================================ Readable things
	//------------------------------------------------------------------------------------------------
	//! An ID card, a letter, a receipt or a photo, in the Zone's voice. Sometimes a key is taped
	//! inside (keyId and keyTag set: the caller places or gives the key), a stash is written on the
	//! back, or a town is circled. Registers it and returns its id (-1 if none); `line` is
	//! "Title: text" for a search message. loose = it lies in a building's loose loot.
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
				if (loose)
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
	//! placed there, or the car boot searched there, holds it).
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
