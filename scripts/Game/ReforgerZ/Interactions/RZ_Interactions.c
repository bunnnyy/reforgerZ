//! World interactions (docs/DESIGN.md, "World interactions"): searching, locks and keys,
//! readable things, fixable cars, crafting, water, barricades and traps, generators, fire,
//! bells and sirens, cracking secure radio channels and treating other players.
//!
//! How it works on any map, without touching the map's prefabs:
//! - Data first. Every few seconds the server looks around each player (a small sphere
//!   query of static map objects) and recognises things by prefab or mesh name: buildings by
//!   type (pharmacy, garage, farm, police, shop, house), furniture (cupboards, fridges,
//!   desks), wrecks, workbenches, wells and pumps, stoves, bells, sirens, radios, generators.
//! - Only near a player, an invisible **interaction spot** (one small prefab with the generic
//!   puzzle object's components) is put on each thing, with replicated labels ("Search the
//!   fridge"). The puzzle actions route here (TAG). Spots are removed when everyone leaves.
//! - What happens is decided on the server: loot is rolled when someone searches, and only
//!   then is anything spawned. What was searched, opened or burned is remembered as data
//!   (positions and times) and saved; town loot comes back through RZ_TownMemory.
//! - Longer actions (searching, forcing a lock, hotwiring, crafting, cracking a channel,
//!   treating someone) are timed on the server tick: stay put until they finish.
//!
//! The parts live in their own files: RZ_Search.c, RZ_DoorLocks.c (with RZ_DoorHooks.c and
//! RZ_KeyItem.c), RZ_Cars.c, RZ_Crafting.c (and water),
//! RZ_Traps.c (barricades, tripwires, spike traps, generators, molotovs and fire),
//! RZ_SecureRadio.c and RZ_Medical.c.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Interactions > General.
[BaseContainerProps()]
class RZ_InteractionSettings
{
	[Attribute("1", desc: "World interactions on: searching, locks, cars, crafting, water, traps, generators, fire, bells, secure radio, treating others (docs/DESIGN.md, World interactions)")]
	bool m_bEnabled;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Interaction spot: a small invisible prop (a little collision box, no mesh) with RplComponent, ActionsManagerComponent and the generic puzzle object's components (RZ_PuzzleObjectComponent, RZ_ClueComponent, RZ_ReadClueAction, 4 RZ_PuzzleUseAction values 0-3, 4 RZ_PuzzleChoiceAction options 0-3). Empty = Puzzles > Generic object (visible)", "et")]
	ResourceName m_sSpotPrefab;

	[Attribute("30", desc: "Things within this distance (m) of a player get an interaction spot")]
	float m_fScanRadius;

	[Attribute("10", desc: "Seconds between looks around each player (only after they moved 10 m)")]
	float m_fScanSeconds;

	[Attribute("10", desc: "Most interaction spots around one player (the nearest win)")]
	int m_iSpotsPerPlayer;

	[Attribute("70", desc: "Most interaction spots on the whole server (performance cap)")]
	int m_iSpotCap;

	[Attribute("3.5", desc: "A timed action (search, hotwire, craft...) stops if you move further than this (m)")]
	float m_fTaskReach;

	// ---------------------------------------------------------------- Bells, sirens and alarms
	[Attribute("45", desc: "Heat a rung church bell makes (it draws hordes)")]
	float m_fBellHeat;

	[Attribute("60", desc: "Heat a siren or alarm makes")]
	float m_fSirenHeat;

	[Attribute("15", desc: "Real minutes before the same bell or siren can be used again")]
	float m_fBellCooldownMinutes;

	[Attribute("bell,belfry", desc: "Bell prefab name keywords (churches get a bell spot too)")]
	string m_sBellWords;

	[Attribute("siren,alarm,klaxon", desc: "Siren and alarm prefab name keywords")]
	string m_sSirenWords;

	// ---------------------------------------------------------------- Water
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Untreated water: a bottle from a well or pump (the hunger and thirst mod's unsafe water if it has one). Empty = no filling", "et")]
	ResourceName m_sDirtyWaterPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Safe water: boiled or purified (the hunger and thirst mod's drinkable water bottle)", "et")]
	ResourceName m_sCleanWaterPrefab;

	[Attribute("bottle_empty,empty_bottle,emptybottle,canteen_empty,empty_canteen", desc: "Empty container keywords (one is filled at a well)")]
	string m_sContainerWords;

	[Attribute("water_dirty,dirty_water,untreated,unsafe_water,water_unsafe", desc: "Untreated water keywords (boiled or purified into safe water)")]
	string m_sDirtyWaterWords;

	[Attribute("purification,purifying,watertablet,water_tablet", desc: "Water purification tablet keywords")]
	string m_sTabletWords;

	[Attribute("well,waterpump,water_pump,handpump,hand_pump,pump_water", desc: "Well and water pump prefab name keywords")]
	string m_sWellWords;

	[Attribute("stove,oven,fireplace,campfire,furnace,hearth", desc: "Stove and fireplace keywords (boil water there)")]
	string m_sStoveWords;

	[Attribute("30", desc: "Seconds to boil water")]
	float m_fBoilSeconds;

	// ---------------------------------------------------------------- Treating other players
	[Attribute("1", desc: "Treat another player (RZ_TreatOtherAction on the player character prefab)")]
	bool m_bTreatOthers;

	[Attribute("20", desc: "Seconds to treat someone (anyone with a first-aid kit)")]
	float m_fTreatSeconds;

	[Attribute("0.4", desc: "Nurse: treating time multiplier (works with just a bandage)")]
	float m_fNurseMult;

	[Attribute("0.3", desc: "Doctor: treating time multiplier (works with just a bandage)")]
	float m_fDoctorMult;

	[Attribute("medkit,firstaid,first_aid,ifak", desc: "First-aid kit keywords (anyone can treat with one)")]
	string m_sMedkitWords;

	[Attribute("bandage,dressing,gauze", desc: "Bandage keywords (Nurses and Doctors treat with one of these too)")]
	string m_sBandageWords;

	[Attribute("5", desc: "Real minutes before the same player can be treated again")]
	float m_fTreatCooldownMinutes;

	[Attribute("1.15", desc: "A Nurse or Doctor treating a bitten player slows the infection this much (x the time left, once per life)")]
	float m_fTreatSlowsInfection;
}

//------------------------------------------------------------------------------------------------
//! One interaction spot. Data; m_Body exists only while a player is near.
class RZ_Spot
{
	int m_iId;
	int m_iKind;
	string m_sKey;			// position key, stable for map objects
	vector m_vPos;
	vector m_vBuilding;		// the building it belongs to (locks, fire, loot type)
	string m_sBuildingKey;
	string m_sType;			// building type: pharmacy, garage, farm, police, shop, house; or body, car
	string m_sNoun;			// "the fridge", "the pharmacy", "the wreck"
	int m_iRef = -1;		// car project id, body index...
	float m_fSeen;			// hub clock when a player last saw it
	float m_fExpires = -1;	// bodies: gone after this
	IEntity m_Body;
	RZ_PuzzleObjectComponent m_Comp;
}

//------------------------------------------------------------------------------------------------
//! A timed action: the player stays put until it finishes.
class RZ_TimedTask
{
	int m_iPlayerId;
	int m_iKind;
	int m_iSpot = -1;
	int m_iArg;
	vector m_vStart;
	float m_fLeft;
	float m_fHealth = -1;
	bool m_bStill;			// moving at all (2 m) stops it; noise and damage too (cracking a channel)
	string m_sLabel;		// "the search"
	IEntity m_Target;		// treating another player
}

//------------------------------------------------------------------------------------------------
//! A candidate found by the look-around.
class RZ_SpotCandidate
{
	int m_iKind;
	vector m_vPos;
	string m_sName;		// lower-case prefab or mesh name
	float m_fDist;
	IEntity m_Entity;	// buildings: their doors are looked at (RZ_DoorLocks)
}

//------------------------------------------------------------------------------------------------
class RZ_InteractionSave
{
	int seed;
	float clock;
	ref array<string> searchedKey = {};
	ref array<float> searchedAt = {};
	ref array<string> openKey = {};
	ref array<float> openAt = {};
	ref array<string> forcedKey = {};
	ref array<float> burnX = {};
	ref array<float> burnZ = {};
	ref array<float> burnAt = {};
	ref array<string> wreckKey = {};
	ref array<float> wreckAt = {};
	ref RZ_CarRosterSave cars;
}

//------------------------------------------------------------------------------------------------
//! Inventory helpers by keyword (prefab names), shared by every interaction.
class RZ_ItemHelper
{
	static const string NOT_EMPTY = ",-empty";	// appended to fuel keywords: an empty can is no fuel

	//------------------------------------------------------------------------------------------------
	//! "a,b, c" into lower-case words.
	static void Words(string text, notnull array<string> result)
	{
		result.Clear();
		string part = "";
		int len = text.Length();
		for (int i = 0; i < len; i++)
		{
			string ch = text.Substring(i, 1);
			if (ch == ",")
			{
				AddWord(part, result);
				part = "";
			}
			else if (ch != " ")
			{
				part += ch;
			}
		}

		AddWord(part, result);
	}

	//------------------------------------------------------------------------------------------------
	protected static void AddWord(string word, notnull array<string> result)
	{
		if (word.IsEmpty())
			return;

		word.ToLower();
		result.Insert(word);
	}

	//------------------------------------------------------------------------------------------------
	static bool NameHas(string lowerName, array<string> words)
	{
		foreach (string w : words)
		{
			if (!w.IsEmpty() && lowerName.Contains(w))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	static string LowerName(IEntity e)
	{
		string name = RZ_Util.GetPrefabName(e);
		name.ToLower();
		return name;
	}

	//------------------------------------------------------------------------------------------------
	//! Whether an item's prefab name has any of the words and none of the "-word" exclusions
	//! (fuel lists end in NOT_EMPTY: a full can, never an empty one).
	static bool Matches(IEntity item, array<string> words)
	{
		if (!item)
			return false;

		string name = LowerName(item);
		if (name.IsEmpty())
			return false;

		bool hit = false;
		foreach (string w : words)
		{
			if (w.Length() > 1 && w.Substring(0, 1) == "-")
			{
				if (name.Contains(w.Substring(1, w.Length() - 1)))
					return false;
			}
			else if (!w.IsEmpty() && name.Contains(w))
			{
				hit = true;
			}
		}

		return hit;
	}

	//------------------------------------------------------------------------------------------------
	//! How many carried items match the words (see Matches).
	static int Count(IEntity character, array<string> words)
	{
		if (!character || words.IsEmpty())
			return 0;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return 0;

		array<IEntity> items = {};
		inv.GetItems(items);
		int n = 0;
		foreach (IEntity item : items)
		{
			if (Matches(item, words))
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	static bool Has(IEntity character, string wordList)
	{
		array<string> words = {};
		Words(wordList, words);
		return Count(character, words) > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Takes up to n matching items (see Matches). Returns how many were taken.
	static int Take(IEntity character, string wordList, int n)
	{
		array<string> words = {};
		Words(wordList, words);
		if (words.IsEmpty() || !character || n <= 0)
			return 0;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return 0;

		array<IEntity> items = {};
		inv.GetItems(items);
		int taken = 0;
		foreach (IEntity item : items)
		{
			if (taken >= n)
				break;

			if (!Matches(item, words))
				continue;

			inv.TryDeleteItem(item);
			taken++;
		}

		return taken;
	}

	//------------------------------------------------------------------------------------------------
	//! "bandage" from "{GUID}Prefabs/Items/Medicine/Bandage_01.et": the file name, without digits.
	static string DisplayName(ResourceName prefab)
	{
		string path = prefab;
		int len = path.Length();
		int start = 0;
		int end = len;
		for (int i = 0; i < len; i++)
		{
			string ch = path.Substring(i, 1);
			if (ch == "/" || ch == "}")
			{
				start = i + 1;
				end = len;
			}
			else if (ch == ".")
			{
				end = i;
			}
		}

		if (end <= start)
			return "something";

		string text = "";
		string digits = "0123456789";
		for (int j = start; j < end; j++)
		{
			string c = path.Substring(j, 1);
			if (c == "_" || c == "-")
				text += " ";
			else if (!digits.Contains(c))
				text += c;
		}

		text.ToLower();
		while (text.Length() > 0 && text.Substring(text.Length() - 1, 1) == " ")
		{
			text = text.Substring(0, text.Length() - 1);
		}

		if (text.IsEmpty())
			return "something";

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns the item and puts it in the player's inventory; on the ground at `at` if it doesn't fit.
	static bool Give(ResourceName prefab, IEntity user, vector at)
	{
		if (prefab.IsEmpty())
			return false;

		vector pos = RZ_Util.RandomPointInRing(at, 0.3, 0.9) + Vector(0, 0.3, 0);
		IEntity item = RZ_Util.SpawnPrefab(prefab, pos, Math.RandomFloat(0, 360));
		if (!item)
			return false;

		if (user)
			RZ_Util.InsertIntoInventory(user, item);

		// Dropped later, or left on the ground because it didn't fit: cleaned up in time
		RZ_GameModeComponent.TrackLooseItem(item);
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! Spot kinds (the same values as RZ_Interactions.KIND_*), for switch statements.
enum RZ_ESpotKind
{
	FURNITURE,
	BUILDING,
	BODY,
	WRECK,
	WORKBENCH,
	WELL,
	FUELPUMP,
	STOVE,
	BELL,
	SIREN,
	RADIO,
	GENERATOR,
	CAR,
	DOOR,
	WOUND
}

//------------------------------------------------------------------------------------------------
//! Timed action kinds (the same values as RZ_Interactions.TASK_*), for switch statements.
enum RZ_ETaskKind
{
	SEARCH,
	FORCE,
	HOTWIRE,
	CRAFT,
	SIPHON,
	PUMP,
	STRIP,
	BOIL,
	CRACK,
	TREAT,
	BURN,
	FIT,
	CLEAN,
	BREW
}

//------------------------------------------------------------------------------------------------
class RZ_Interactions
{
	static const int TAG = -9;				// RZ_PuzzleObjectComponent / RZ_ClueComponent mission id for spots
	static const int TAG_NOTE = -10;		// readable things a search turned up

	static const int KIND_FURNITURE = 0;
	static const int KIND_BUILDING = 1;
	static const int KIND_BODY = 2;
	static const int KIND_WRECK = 3;
	static const int KIND_WORKBENCH = 4;
	static const int KIND_WELL = 5;
	static const int KIND_FUELPUMP = 6;
	static const int KIND_STOVE = 7;
	static const int KIND_BELL = 8;
	static const int KIND_SIREN = 9;
	static const int KIND_RADIO = 10;
	static const int KIND_GENERATOR = 11;
	static const int KIND_CAR = 12;
	static const int KIND_DOOR = 13;		// a locked place's outside door (RZ_DoorLocks)
	static const int KIND_WOUND = 14;		// a fresh bite at a player's feet (RZ_Infection): Clean the bite

	static const int TASK_SEARCH = 0;
	static const int TASK_FORCE = 1;
	static const int TASK_HOTWIRE = 2;
	static const int TASK_CRAFT = 3;
	static const int TASK_SIPHON = 4;
	static const int TASK_PUMP = 5;
	static const int TASK_STRIP = 6;
	static const int TASK_BOIL = 7;
	static const int TASK_CRACK = 8;
	static const int TASK_TREAT = 9;
	static const int TASK_BURN = 10;
	static const int TASK_FIT = 11;
	static const int TASK_CLEAN = 12;		// cleaning a fresh bite (RZ_Infection)
	static const int TASK_BREW = 13;		// brewing a home remedy on a stove

	protected static const float DESPAWN_MARGIN = 25;
	protected static const float RESCAN_MOVE = 10;
	protected static const int SEARCHED_KEPT = 800;

	protected RZ_GameModeComponent m_GM;
	protected RZ_InteractionSettings m_Settings;
	protected ref RZ_Search m_Search;
	protected ref RZ_DoorLocks m_Doors;
	protected ref RZ_Cars m_Cars;
	protected ref RZ_Crafting m_Crafting;
	protected ref RZ_Traps m_Traps;
	protected ref RZ_SecureRadio m_Radio;
	protected ref RZ_Medical m_Medical;
	protected ref RZ_InteractionSave m_Restored;

	protected bool m_bActive;
	protected int m_iSeed;
	protected float m_fClock;
	protected int m_iNextSpot = 1;
	protected ref array<ref RZ_Spot> m_aSpots = {};
	protected ref array<ref RZ_TimedTask> m_aTasks = {};
	protected ref map<int, vector> m_mScanPos = new map<int, vector>();
	protected ref map<int, float> m_mScanAt = new map<int, float>();
	protected ref map<string, float> m_mUsedAt = new map<string, float>();	// bells and sirens
	protected ref array<string> m_aBellWords = {};
	protected ref array<string> m_aSirenWords = {};
	protected ref array<string> m_aWellWords = {};
	protected ref array<string> m_aStoveWords = {};

	// The look-around in progress
	protected ref array<ref RZ_SpotCandidate> m_aCand = {};
	protected vector m_vScanCenter;

	//------------------------------------------------------------------------------------------------
	void RZ_Interactions(RZ_GameModeComponent gm, RZ_InteractionSettings settings, RZ_SearchSettings search, RZ_CarSettings cars,
		RZ_CraftSettings craft, RZ_TrapSettings traps, RZ_RadioCrackSettings radio)
	{
		m_GM = gm;
		m_Settings = settings;
		m_Search = new RZ_Search(gm, this, search);
		m_Doors = new RZ_DoorLocks(gm, this, m_Search, search);
		m_Cars = new RZ_Cars(gm, this, cars);
		m_Crafting = new RZ_Crafting(gm, this, craft, settings);
		m_Traps = new RZ_Traps(gm, this, traps);
		m_Radio = new RZ_SecureRadio(gm, this, radio);
		m_Medical = new RZ_Medical(gm, this, settings);
	}

	//------------------------------------------------------------------------------------------------
	bool IsActive() { return m_bActive; }
	float GetClock() { return m_fClock; }
	int GetSeed() { return m_iSeed; }
	RZ_InteractionSettings GetSettings() { return m_Settings; }
	RZ_Search GetSearch() { return m_Search; }
	RZ_DoorLocks GetDoors() { return m_Doors; }
	RZ_Cars GetCars() { return m_Cars; }
	RZ_Crafting GetCrafting() { return m_Crafting; }
	RZ_Traps GetTraps() { return m_Traps; }
	RZ_SecureRadio GetRadio() { return m_Radio; }
	RZ_Medical GetMedical() { return m_Medical; }

	//------------------------------------------------------------------------------------------------
	static RZ_Interactions Get()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return null;

		RZ_Interactions hub = gm.GetInteractions();
		if (!hub || !hub.IsActive())
			return null;

		return hub;
	}

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	void Restore(RZ_InteractionSave saved)
	{
		m_Restored = saved;
	}

	//------------------------------------------------------------------------------------------------
	RZ_InteractionSave Save()
	{
		RZ_InteractionSave data = new RZ_InteractionSave();
		data.seed = m_iSeed;
		data.clock = m_fClock;
		m_Search.SaveInto(data);
		m_Traps.SaveInto(data);
		m_Cars.SaveInto(data);
		return data;
	}

	// ================================================================ Setup
	//------------------------------------------------------------------------------------------------
	//! After settlements, safehouses, territory and the power grid exist.
	void Setup()
	{
		if (!m_Settings.m_bEnabled || m_GM.GetSettlements().IsEmpty())
		{
			RZ_Log.Info("World interactions are off.");
			return;
		}

		RZ_ItemHelper.Words(m_Settings.m_sBellWords, m_aBellWords);
		RZ_ItemHelper.Words(m_Settings.m_sSirenWords, m_aSirenWords);
		RZ_ItemHelper.Words(m_Settings.m_sWellWords, m_aWellWords);
		RZ_ItemHelper.Words(m_Settings.m_sStoveWords, m_aStoveWords);

		m_iSeed = Math.RandomInt(1, 2000000000);
		if (m_Restored)
		{
			if (m_Restored.seed > 0)
				m_iSeed = m_Restored.seed;

			m_fClock = Math.Max(0, m_Restored.clock);
			m_Search.RestoreFrom(m_Restored);
			m_Traps.RestoreFrom(m_Restored);
			m_Cars.RestoreWrecks(m_Restored);
		}

		m_bActive = true;
		m_Search.Setup();
		m_Crafting.Setup();
		m_Traps.Setup();
		if (m_Restored)
			m_Cars.Setup(m_Restored.cars);
		else
			m_Cars.Setup(null);

		m_Restored = null;
		RZ_Log.Info(string.Format("World interactions: on. %1 fixable cars on the map.", m_Cars.Count()));
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bActive)
			return;

		m_fClock += dt;
		foreach (int i, IEntity p : players)
		{
			MaybeScan(ids[i], p);
		}

		m_Cars.Tick(dt, players);
		UpdateSpots(players);
		m_Doors.Tick(players);
		m_Doors.TickBlockers(ids, players);
		TickTasks(dt);
		m_Traps.Tick(dt, ids, players);
		m_Radio.Tick(dt, ids, players);
		m_Search.Tick(dt);
	}

	//------------------------------------------------------------------------------------------------
	//! A new life: keys, timed actions and listening don't carry over.
	void OnPlayerSpawned(int playerId)
	{
		if (!m_bActive)
			return;

		CancelTask(playerId, "");
		m_Search.OnPlayerSpawned(playerId);
		m_Radio.OnPlayerSpawned(playerId);
		m_Medical.OnPlayerSpawned(playerId);
		m_mScanAt.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! From the game mode: infected died here. A body to search for a few minutes, near players.
	void OnZombiesKilled(vector pos, int count)
	{
		if (m_bActive)
			m_Search.OnZombiesKilled(pos, count);
	}

	// ================================================================ The look-around
	//------------------------------------------------------------------------------------------------
	protected void MaybeScan(int playerId, IEntity p)
	{
		vector pos = p.GetOrigin();
		float last;
		vector lastPos;
		if (m_mScanAt.Find(playerId, last))
		{
			if (m_fClock - last < m_Settings.m_fScanSeconds)
				return;

			if (m_mScanPos.Find(playerId, lastPos) && RZ_Util.DistXZ(lastPos, pos) < RESCAN_MOVE)
			{
				m_mScanAt.Set(playerId, m_fClock);
				TouchSpotsNear(pos);
				return;
			}
		}

		m_mScanAt.Set(playerId, m_fClock);
		m_mScanPos.Set(playerId, pos);
		if (RZ_Util.IsInVehicle(p))
			return;

		Scan(pos);
	}

	//------------------------------------------------------------------------------------------------
	protected void TouchSpotsNear(vector pos)
	{
		foreach (RZ_Spot s : m_aSpots)
		{
			if (RZ_Util.DistXZ(s.m_vPos, pos) <= m_Settings.m_fScanRadius + DESPAWN_MARGIN)
				s.m_fSeen = m_fClock;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Scan(vector center)
	{
		m_aCand.Clear();
		m_vScanCenter = center;
		BaseWorld world = GetGame().GetWorld();
		world.QueryEntitiesBySphere(center, m_Settings.m_fScanRadius, OnScanEntity, null, EQueryEntitiesFlags.STATIC); // VERIFY: furniture inside building prefabs is found as static entities (else only whole buildings get a spot)

		// Nearest first (an index list, so the candidates themselves never move)
		array<int> order = {};
		int wanted = Math.Min(m_aCand.Count(), 40);
		while (order.Count() < wanted)
		{
			int best = -1;
			for (int i = 0; i < m_aCand.Count(); i++)
			{
				if (order.Find(i) >= 0)
					continue;

				if (best < 0 || m_aCand[i].m_fDist < m_aCand[best].m_fDist)
					best = i;
			}

			if (best < 0)
				break;

			order.Insert(best);
		}

		int added = CountSpotsNear(center, m_Settings.m_fScanRadius);
		foreach (int idx : order)
		{
			RZ_SpotCandidate c = m_aCand[idx];
			if (added >= m_Settings.m_iSpotsPerPlayer || m_aSpots.Count() >= m_Settings.m_iSpotCap)
				break;

			// A locked place's outside doors get a spot each (Unlock, Lock, Force)
			if (c.m_iKind == KIND_BUILDING)
			{
				added += m_Doors.OnBuildingSeen(c.m_Entity, c.m_sName, m_Settings.m_iSpotsPerPlayer - added);
				if (added >= m_Settings.m_iSpotsPerPlayer)
					break;
			}

			if (c.m_iKind == KIND_BUILDING && HasFurnitureNear(c.m_vPos))
				continue;

			string key = PosKey(c.m_vPos, 1);
			RZ_Spot existing = FindSpotByKey(key);
			if (existing)
			{
				existing.m_fSeen = m_fClock;
				continue;
			}

			if (AddCandidate(c, key))
				added++;
		}

		m_aCand.Clear();
		TouchSpotsNear(center);
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasFurnitureNear(vector buildingPos)
	{
		foreach (RZ_SpotCandidate c : m_aCand)
		{
			if (c.m_iKind == KIND_FURNITURE && RZ_Util.DistXZ(c.m_vPos, buildingPos) <= 12)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnScanEntity(IEntity e)
	{
		if (!e || ChimeraCharacter.Cast(e) || SCR_AIGroup.Cast(e))
			return true;

		string name = RZ_ItemHelper.LowerName(e);
		bool building = SCR_DestructibleBuildingEntity.Cast(e) != null; // as in RZ_SettlementScanner
		if (name.IsEmpty() && !building)
			return true;

		int kind = Classify(name, building);
		if (kind < 0)
			return true;

		RZ_SpotCandidate c = new RZ_SpotCandidate();
		c.m_iKind = kind;
		c.m_vPos = e.GetOrigin();
		c.m_sName = name;
		c.m_fDist = RZ_Util.DistXZ(c.m_vPos, m_vScanCenter);
		if (kind == KIND_BUILDING)
			c.m_Entity = e;

		m_aCand.Insert(c);

		// A church gets a bell
		if (kind == KIND_BUILDING && RZ_LandmarkFinder.Matches("church", name))
		{
			RZ_SpotCandidate bell = new RZ_SpotCandidate();
			bell.m_iKind = KIND_BELL;
			bell.m_vPos = c.m_vPos + Vector(1.5, 0, 1.5);
			bell.m_sName = "church bell";
			bell.m_fDist = c.m_fDist;
			m_aCand.Insert(bell);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! What a static map object is for us, -1 if nothing. Lower-case prefab or mesh name.
	protected int Classify(string name, bool building)
	{
		if (m_Cars.IsWreckName(name))
			return KIND_WRECK;

		if (m_Crafting.IsWorkbenchName(name))
			return KIND_WORKBENCH;

		if (m_Traps.IsGeneratorName(name))
			return KIND_GENERATOR;

		if (RZ_ItemHelper.NameHas(name, m_aSirenWords))
			return KIND_SIREN;

		if (RZ_ItemHelper.NameHas(name, m_aBellWords) && !building)
			return KIND_BELL;

		if (RZ_LandmarkFinder.Matches("fuel", name) || name.Contains("fuelpump") || name.Contains("fuel_pump") || name.Contains("gaspump") || name.Contains("petrolpump"))
			return KIND_FUELPUMP;

		if (!building && (RZ_LandmarkFinder.Matches("well", name) || RZ_ItemHelper.NameHas(name, m_aWellWords)))
			return KIND_WELL;

		if (!building && RZ_ItemHelper.NameHas(name, m_aStoveWords))
			return KIND_STOVE;

		if (!building && m_Radio.IsHouseRadioName(name))
			return KIND_RADIO;

		if (!building && m_Search.IsFurnitureName(name))
			return KIND_FURNITURE;

		if (building || name.Contains("/buildings/") || name.Contains("house"))
			return KIND_BUILDING;

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	protected bool AddCandidate(RZ_SpotCandidate c, string key)
	{
		RZ_Spot s = new RZ_Spot();
		s.m_iId = m_iNextSpot;
		m_iNextSpot++;
		s.m_iKind = c.m_iKind;
		s.m_sKey = key;
		s.m_vPos = c.m_vPos;
		s.m_vBuilding = c.m_vPos;
		s.m_sType = "house";
		s.m_fSeen = m_fClock;

		if (c.m_iKind == KIND_FURNITURE || c.m_iKind == KIND_BUILDING || c.m_iKind == KIND_RADIO || c.m_iKind == KIND_STOVE || c.m_iKind == KIND_WORKBENCH)
		{
			RZ_SpotCandidate b = NearestBuilding(c.m_vPos);
			if (b)
			{
				s.m_vBuilding = b.m_vPos;
				s.m_sType = m_Search.BuildingType(b.m_sName);
			}
			else if (c.m_iKind == KIND_BUILDING)
			{
				s.m_sType = m_Search.BuildingType(c.m_sName);
			}
		}

		if (c.m_iKind == KIND_WRECK)
			s.m_sType = "car";

		s.m_sBuildingKey = PosKey(s.m_vBuilding, 4);
		s.m_sNoun = Noun(s, c.m_sName);
		m_aSpots.Insert(s);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_SpotCandidate NearestBuilding(vector pos)
	{
		RZ_SpotCandidate best;
		float bestDist = 15;
		foreach (RZ_SpotCandidate c : m_aCand)
		{
			if (c.m_iKind != KIND_BUILDING)
				continue;

			float d = RZ_Util.DistXZ(c.m_vPos, pos);
			if (d < bestDist)
			{
				bestDist = d;
				best = c;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	protected string Noun(RZ_Spot s, string name)
	{
		switch (s.m_iKind)
		{
			case RZ_ESpotKind.FURNITURE: return m_Search.FurnitureNoun(name);
			case RZ_ESpotKind.BUILDING: return RZ_Search.TypeNoun(s.m_sType);
			case RZ_ESpotKind.WRECK: return "the wreck";
			case RZ_ESpotKind.WORKBENCH: return "the workbench";
			case RZ_ESpotKind.WELL: return "the well";
			case RZ_ESpotKind.FUELPUMP: return "the fuel pump";
			case RZ_ESpotKind.STOVE: return "the stove";
			case RZ_ESpotKind.BELL: return "the bell";
			case RZ_ESpotKind.SIREN: return "the siren";
			case RZ_ESpotKind.RADIO: return "the radio";
			case RZ_ESpotKind.GENERATOR: return "the generator";
		}

		return "it";
	}

	//------------------------------------------------------------------------------------------------
	//! A spot other systems ask for (bodies, car projects). Returns its id, -1 at the cap.
	int AddSpot(int kind, vector pos, string type, string noun, int refId, float expires)
	{
		// A bite spot is never refused: one per bitten player, and missing it costs a life
		if (kind != KIND_WOUND && m_aSpots.Count() >= m_Settings.m_iSpotCap)
			return -1;

		RZ_Spot s = new RZ_Spot();
		s.m_iId = m_iNextSpot;
		m_iNextSpot++;
		s.m_iKind = kind;
		s.m_vPos = pos;
		s.m_vBuilding = pos;
		s.m_sKey = string.Format("%1:%2", kind, s.m_iId);
		s.m_sBuildingKey = PosKey(pos, 4);
		s.m_sType = type;
		s.m_sNoun = noun;
		s.m_iRef = refId;
		s.m_fSeen = m_fClock;
		s.m_fExpires = expires;
		m_aSpots.Insert(s);
		return s.m_iId;
	}

	//------------------------------------------------------------------------------------------------
	//! A spot outside a locked place's door (RZ_DoorLocks). False if it exists already (a double
	//! door shares one) or at the cap.
	bool AddDoorSpot(vector pos, vector buildingPos, string buildingKey, string type, string noun, int doorIndex)
	{
		string key = "door_" + PosKey(pos, 1);
		foreach (RZ_Spot o : m_aSpots)
		{
			if (o.m_sKey == key || (o.m_iKind == KIND_DOOR && o.m_sBuildingKey == buildingKey && RZ_Util.DistXZ(o.m_vPos, pos) < 1.8))
			{
				o.m_fSeen = m_fClock;
				return false;
			}
		}

		if (m_aSpots.Count() >= m_Settings.m_iSpotCap)
			return false;

		RZ_Spot s = new RZ_Spot();
		s.m_iId = m_iNextSpot;
		m_iNextSpot++;
		s.m_iKind = KIND_DOOR;
		s.m_sKey = key;
		s.m_vPos = pos;
		s.m_vBuilding = buildingPos;
		s.m_sBuildingKey = buildingKey;
		s.m_sType = type;
		s.m_sNoun = noun;
		s.m_iRef = doorIndex;
		s.m_fSeen = m_fClock;
		m_aSpots.Insert(s);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void RemoveSpot(int id)
	{
		for (int i = m_aSpots.Count() - 1; i >= 0; i--)
		{
			if (m_aSpots[i].m_iId != id)
				continue;

			DespawnSpot(m_aSpots[i]);
			m_aSpots.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	int CountKind(int kind)
	{
		int n = 0;
		foreach (RZ_Spot s : m_aSpots)
		{
			if (s.m_iKind == kind)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected int CountSpotsNear(vector pos, float radius)
	{
		int n = 0;
		foreach (RZ_Spot s : m_aSpots)
		{
			if (RZ_Util.DistXZ(s.m_vPos, pos) <= radius)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	RZ_Spot FindSpot(int id)
	{
		foreach (RZ_Spot s : m_aSpots)
		{
			if (s.m_iId == id)
				return s;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Spot FindSpotByKey(string key)
	{
		foreach (RZ_Spot s : m_aSpots)
		{
			if (s.m_sKey == key)
				return s;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! "x_z" on a grid of `grid` metres. Stable for map objects, so it can be saved.
	static string PosKey(vector pos, float grid)
	{
		int gx = Math.Floor(pos[0] / grid);
		int gz = Math.Floor(pos[2] / grid);
		return string.Format("%1_%2", gx, gz);
	}

	// ================================================================ Spot bodies near players
	//------------------------------------------------------------------------------------------------
	protected ResourceName SpotPrefab()
	{
		if (!m_Settings.m_sSpotPrefab.IsEmpty())
			return m_Settings.m_sSpotPrefab;

		return m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.OBJECT);
	}

	//------------------------------------------------------------------------------------------------
	//! The interaction spot prefab (else the generic puzzle object), for systems that place their own.
	ResourceName GetSpotPrefab()
	{
		return SpotPrefab();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateSpots(array<IEntity> players)
	{
		for (int i = m_aSpots.Count() - 1; i >= 0; i--)
		{
			RZ_Spot s = m_aSpots[i];
			// A car project's prop is spawned and removed by RZ_Cars
			if (s.m_iKind == KIND_CAR)
				continue;

			bool expired = s.m_fExpires >= 0 && m_fClock >= s.m_fExpires;
			float nearest = RZ_Util.NearestDistance(s.m_vPos, players);
			bool far = nearest > m_Settings.m_fScanRadius + DESPAWN_MARGIN;
			if (expired || (far && s.m_iKind != KIND_BODY))
			{
				DespawnSpot(s);
				m_aSpots.Remove(i);
				continue;
			}

			if (far)
			{
				DespawnSpot(s);
				continue;
			}

			if (!s.m_Body)
				SpawnSpot(s);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnSpot(RZ_Spot s)
	{
		s.m_Body = RZ_Util.SpawnPrefab(SpotPrefab(), s.m_vPos + Vector(0, 0.6, 0), 0);
		if (!s.m_Body)
			return;

		AttachLabels(s, s.m_Body);
	}

	//------------------------------------------------------------------------------------------------
	//! Tag an entity (a spot, or a car project's own prop) so its puzzle actions route here.
	void AttachLabels(RZ_Spot s, IEntity body)
	{
		s.m_Body = body;
		string read;
		string useLabels;
		string choiceLabels;
		Labels(s, read, useLabels, choiceLabels);
		s.m_Comp = RZ_PuzzleObjectComponent.Tag(body, TAG, s.m_iId, useLabels, choiceLabels);
		RZ_ClueComponent.Tag(body, TAG, s.m_iId, read);
	}

	//------------------------------------------------------------------------------------------------
	protected void DespawnSpot(RZ_Spot s)
	{
		if (s.m_Body)
			RZ_Util.DeleteUnit(s.m_Body);

		s.m_Body = null;
		s.m_Comp = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Labels change (a lock opened, a generator started): relabel every spot of that building.
	void Relabel(RZ_Spot s)
	{
		if (!s || !s.m_Comp)
			return;

		string read;
		string useLabels;
		string choiceLabels;
		Labels(s, read, useLabels, choiceLabels);
		s.m_Comp.SetLabels(useLabels, choiceLabels);
	}

	//------------------------------------------------------------------------------------------------
	void RelabelBuilding(string buildingKey)
	{
		foreach (RZ_Spot s : m_aSpots)
		{
			if (s.m_sBuildingKey == buildingKey)
				Relabel(s);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A building was unlocked, locked or forced: its spots' labels and the door list follow.
	void OnLockChanged(string buildingKey)
	{
		RelabelBuilding(buildingKey);
		m_Doors.OnLockChanged(buildingKey);
	}

	//------------------------------------------------------------------------------------------------
	//! Read label, use slots (0-3) and choice slots (0-3) for a spot, "|" separated.
	protected void Labels(RZ_Spot s, out string read, out string useLabels, out string choiceLabels)
	{
		read = "Look at " + s.m_sNoun;
		useLabels = "";
		choiceLabels = "";
		if (s.m_iKind == KIND_FURNITURE || s.m_iKind == KIND_BUILDING || s.m_iKind == KIND_BODY)
		{
			m_Search.Labels(s, read, choiceLabels);
			return;
		}

		switch (s.m_iKind)
		{
			case RZ_ESpotKind.DOOR:
			{
				m_Doors.Labels(s, read, choiceLabels);
				break;
			}
			case RZ_ESpotKind.WRECK:
			{
				m_Cars.WreckLabels(s, read, choiceLabels);
				break;
			}
			case RZ_ESpotKind.FUELPUMP:
			{
				choiceLabels = "Work the hand pump (fuel)";
				break;
			}
			case RZ_ESpotKind.CAR:
			{
				m_Cars.ProjectLabels(s, read, choiceLabels);
				break;
			}
			case RZ_ESpotKind.WORKBENCH:
			{
				m_Crafting.WorkbenchLabels(s, read, useLabels, choiceLabels);
				break;
			}
			case RZ_ESpotKind.WELL:
			{
				choiceLabels = "Fill a bottle|Fill a bottle and purify it";
				break;
			}
			case RZ_ESpotKind.STOVE:
			{
				choiceLabels = m_Crafting.StoveLabels();
				break;
			}
			case RZ_ESpotKind.WOUND:
			{
				if (m_GM.GetInfection())
					m_GM.GetInfection().WoundLabels(read, choiceLabels);
				break;
			}
			case RZ_ESpotKind.BELL:
			{
				choiceLabels = "Ring the bell";
				break;
			}
			case RZ_ESpotKind.SIREN:
			{
				choiceLabels = "Sound the siren";
				break;
			}
			case RZ_ESpotKind.RADIO:
			{
				read = "Listen to the radio";
				choiceLabels = "Tune through the open channels|Work on a scrambled channel";
				break;
			}
			case RZ_ESpotKind.GENERATOR:
			{
				m_Traps.FoundGeneratorLabels(s, read, choiceLabels);
				break;
			}
		}
	}

	// ================================================================ Routing from the puzzle actions
	//------------------------------------------------------------------------------------------------
	//! RZ_ReadClueAction on a spot or a readable note.
	void HandleRead(int playerId, int tag, int id, IEntity owner)
	{
		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!user)
			return;

		if (tag == TAG_NOTE)
		{
			m_Search.ReadNote(playerId, id);
			return;
		}

		RZ_Spot s = FindSpot(id);
		if (!s)
		{
			m_GM.NotifyPlayer(playerId, "Nothing", "Nothing here you hadn't seen.");
			return;
		}

		if (s.m_iKind == KIND_FURNITURE || s.m_iKind == KIND_BUILDING || s.m_iKind == KIND_BODY)
		{
			m_Search.Describe(playerId, s);
			return;
		}

		switch (s.m_iKind)
		{
			case RZ_ESpotKind.DOOR: m_Doors.Describe(playerId, s); return;
			case RZ_ESpotKind.WRECK: m_Cars.DescribeWreck(playerId, s); return;
			case RZ_ESpotKind.CAR: m_Cars.Inspect(playerId, s, user); return;
			case RZ_ESpotKind.WORKBENCH: m_Crafting.DescribeBench(playerId); return;
			case RZ_ESpotKind.RADIO: m_Radio.ListenHouseRadio(playerId, s, user); return;
			case RZ_ESpotKind.GENERATOR: m_Traps.DescribeFoundGenerator(playerId, s); return;
			case RZ_ESpotKind.WOUND:
			{
				if (m_GM.GetInfection())
					m_GM.GetInfection().DescribeWound(playerId, s);
				return;
			}
		}

		m_GM.NotifyPlayer(playerId, Title(s), DescribeSimple(s));
	}

	//------------------------------------------------------------------------------------------------
	protected string DescribeSimple(RZ_Spot s)
	{
		switch (s.m_iKind)
		{
			case RZ_ESpotKind.WELL: return "A well. The water looks clear enough, which means nothing. Boil it or purify it before you drink.";
			case RZ_ESpotKind.FUELPUMP: return "The electric pumps are dead, but there's a hand pump on the side for when the power goes. There's still something in the tank, by the sound of it.";
			case RZ_ESpotKind.STOVE: return "Cold, but it would light. Boiling water takes a while and smokes a little.";
			case RZ_ESpotKind.BELL: return "A heavy bell on a rope. Everything for a mile would hear it, and come.";
			case RZ_ESpotKind.SIREN: return "A civil defence siren with a hand crank. It still turns.";
		}

		return "Nothing special.";
	}

	//------------------------------------------------------------------------------------------------
	string Title(RZ_Spot s)
	{
		string t = s.m_sNoun;
		if (t.Length() > 4 && t.Substring(0, 4) == "the ")
			t = t.Substring(4, t.Length() - 4);

		if (t.IsEmpty())
			return "Search";

		string first = t.Substring(0, 1);
		first.ToUpper(); // in place, like ToLower()
		return first + t.Substring(1, t.Length() - 1);
	}

	//------------------------------------------------------------------------------------------------
	//! RZ_PuzzleChoiceAction on a spot.
	void HandleChoice(int playerId, int id, int option, IEntity owner, IEntity user)
	{
		RZ_Spot s = FindSpot(id);
		if (!s || !user)
			return;

		if (HasTask(playerId))
		{
			m_GM.NotifyPlayer(playerId, Title(s), "You're already busy with something. Finish it first, or step away to stop.");
			return;
		}

		if (s.m_iKind == KIND_FURNITURE || s.m_iKind == KIND_BUILDING || s.m_iKind == KIND_BODY)
		{
			m_Search.HandleChoice(playerId, s, option, user);
			return;
		}

		if (s.m_iKind == KIND_BELL || s.m_iKind == KIND_SIREN)
		{
			Sound(playerId, s);
			return;
		}

		switch (s.m_iKind)
		{
			case RZ_ESpotKind.DOOR: m_Doors.HandleChoice(playerId, s, option, user); return;
			case RZ_ESpotKind.WRECK: m_Cars.HandleWreck(playerId, s, option, user); return;
			case RZ_ESpotKind.CAR: m_Cars.HandleProject(playerId, s, option, user); return;
			case RZ_ESpotKind.FUELPUMP: m_Cars.HandlePump(playerId, s, user); return;
			case RZ_ESpotKind.WORKBENCH: m_Crafting.HandleRecipe(playerId, s, option, user); return;
			case RZ_ESpotKind.WELL: m_Crafting.HandleWell(playerId, s, option, user); return;
			case RZ_ESpotKind.STOVE: m_Crafting.HandleStove(playerId, s, option, user); return;
			case RZ_ESpotKind.WOUND:
			{
				if (m_GM.GetInfection())
					m_GM.GetInfection().StartClean(playerId, s, option, user);
				return;
			}
			case RZ_ESpotKind.RADIO: m_Radio.HandleHouseRadio(playerId, s, option, user); return;
			case RZ_ESpotKind.GENERATOR: m_Traps.HandleFoundGenerator(playerId, s, option, user); return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! RZ_PuzzleUseAction on a spot: the workbench's last four recipes.
	void HandleUse(int playerId, int id, int value, IEntity owner, IEntity user)
	{
		RZ_Spot s = FindSpot(id);
		if (!s || !user)
			return;

		if (HasTask(playerId))
		{
			m_GM.NotifyPlayer(playerId, Title(s), "You're already busy with something. Finish it first, or step away to stop.");
			return;
		}

		if (s.m_iKind == KIND_WORKBENCH)
			m_Crafting.HandleRecipe(playerId, s, value + 4, user);
	}

	// ================================================================ Bells and sirens
	//------------------------------------------------------------------------------------------------
	//! Anyone can ring a bell or sound a siren: a lot of heat right there, to lure hordes.
	protected void Sound(int playerId, RZ_Spot s)
	{
		float last;
		float cooldown = m_Settings.m_fBellCooldownMinutes * 60;
		if (m_mUsedAt.Find(s.m_sKey, last) && m_fClock - last < cooldown)
		{
			m_GM.NotifyPlayer(playerId, Title(s), string.Format("It's still ringing in your ears. Give it about %1 minutes.", RZ_Util.RoundToInt((cooldown - (m_fClock - last)) / 60) + 1));
			return;
		}

		m_mUsedAt.Set(s.m_sKey, m_fClock);
		float heat = m_Settings.m_fBellHeat;
		string text = "The bell swings, and the sound rolls out over the roofs. Somewhere, something turns its head. Then lots of somethings.";
		string heard = "A church bell is ringing near %1.";
		if (s.m_iKind == KIND_SIREN)
		{
			heat = m_Settings.m_fSirenHeat;
			text = "You crank the handle and the siren winds up into a howl that doesn't stop when you do. They'll come from every direction. Make sure you're not here.";
			heard = "A siren is wailing near %1.";
		}

		m_GM.GetHeat().AddHeat(s.m_vPos, heat);
		m_GM.NotifyPlayer(playerId, Title(s), text);

		string where = RZ_SettlementScanner.GridReference(s.m_vPos);
		RZ_Settlement town = m_GM.GetSettlement(m_GM.FindNearestSettlement(s.m_vPos));
		if (town)
			where = town.m_sName;

		NotifyNear(s.m_vPos, 1200, playerId, "Far off", string.Format(heard, where));
	}

	//------------------------------------------------------------------------------------------------
	//! Everyone within radius of pos except `except`.
	void NotifyNear(vector pos, float radius, int except, string title, string text)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (ids[i] != except && RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius)
				m_GM.NotifyPlayer(ids[i], title, text);
		}
	}

	// ================================================================ Timed actions
	//------------------------------------------------------------------------------------------------
	bool HasTask(int playerId)
	{
		foreach (RZ_TimedTask t : m_aTasks)
		{
			if (t.m_iPlayerId == playerId)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Starts a timed action. Stay within reach (or still, for `still`) until it finishes.
	RZ_TimedTask StartTask(int playerId, IEntity user, int kind, int spotId, int arg, float seconds, string label, bool still)
	{
		RZ_TimedTask t = new RZ_TimedTask();
		t.m_iPlayerId = playerId;
		t.m_iKind = kind;
		t.m_iSpot = spotId;
		t.m_iArg = arg;
		t.m_vStart = user.GetOrigin();
		t.m_fLeft = Math.Max(1, seconds);
		t.m_bStill = still;
		t.m_sLabel = label;
		t.m_fHealth = RZ_Util.GetCharacterHealth(user);
		m_aTasks.Insert(t);
		return t;
	}

	//------------------------------------------------------------------------------------------------
	void CancelTask(int playerId, string why)
	{
		for (int i = m_aTasks.Count() - 1; i >= 0; i--)
		{
			RZ_TimedTask t = m_aTasks[i];
			if (t.m_iPlayerId != playerId)
				continue;

			if (!why.IsEmpty())
				m_GM.NotifyPlayer(playerId, "Stopped", string.Format("%1 You leave %2 unfinished.", why, t.m_sLabel));

			m_aTasks.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void TickTasks(float dt)
	{
		PlayerManager pm = GetGame().GetPlayerManager();
		for (int i = m_aTasks.Count() - 1; i >= 0; i--)
		{
			RZ_TimedTask t = m_aTasks[i];
			IEntity user = pm.GetPlayerControlledEntity(t.m_iPlayerId);
			if (!user || !RZ_Util.IsCharacterAlive(user))
			{
				m_aTasks.Remove(i);
				continue;
			}

			string why = Interrupted(t, user);
			if (!why.IsEmpty())
			{
				m_GM.NotifyPlayer(t.m_iPlayerId, "Stopped", string.Format("%1 You leave %2 unfinished.", why, t.m_sLabel));
				m_aTasks.Remove(i);
				continue;
			}

			t.m_fLeft = t.m_fLeft - dt;
			if (t.m_fLeft > 0)
				continue;

			m_aTasks.Remove(i);
			Complete(t, user);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Why a timed action stops, "" if it goes on.
	protected string Interrupted(RZ_TimedTask t, IEntity user)
	{
		float reach = m_Settings.m_fTaskReach;
		if (t.m_bStill)
			reach = 2;

		if (RZ_Util.DistXZ(user.GetOrigin(), t.m_vStart) > reach)
			return "You moved away.";

		// Treating someone or cleaning their bite: the patient has to stay too
		if ((t.m_iKind == TASK_TREAT || t.m_iKind == TASK_CLEAN) && (!t.m_Target || !RZ_Util.IsCharacterAlive(t.m_Target) || RZ_Util.DistXZ(t.m_Target.GetOrigin(), user.GetOrigin()) > reach))
			return "They're not next to you any more.";

		if (!t.m_bStill && t.m_iKind != TASK_TREAT)
			return "";

		float health = RZ_Util.GetCharacterHealth(user);
		if (t.m_fHealth >= 0 && health >= 0 && health < t.m_fHealth - 0.01)
			return "You were hurt.";

		if (t.m_bStill && m_Radio.IsTooNoisy(user.GetOrigin()))
			return "Too much noise around you to hear anything.";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected void Complete(RZ_TimedTask t, IEntity user)
	{
		RZ_Spot s = FindSpot(t.m_iSpot);
		switch (t.m_iKind)
		{
			case RZ_ETaskKind.SEARCH: m_Search.CompleteSearch(t.m_iPlayerId, s, user); return;
			case RZ_ETaskKind.FORCE: m_Search.CompleteForce(t.m_iPlayerId, s, user); return;
			case RZ_ETaskKind.BURN: m_Traps.CompleteBurn(t.m_iPlayerId, s, user); return;
			case RZ_ETaskKind.HOTWIRE: m_Cars.CompleteHotwire(t.m_iPlayerId, s, t.m_iArg); return;
			case RZ_ETaskKind.FIT: m_Cars.CompleteFit(t.m_iPlayerId, s, t.m_iArg, user); return;
			case RZ_ETaskKind.SIPHON: m_Cars.CompleteSiphon(t.m_iPlayerId, s, user); return;
			case RZ_ETaskKind.STRIP: m_Cars.CompleteStrip(t.m_iPlayerId, s, user); return;
			case RZ_ETaskKind.PUMP: m_Cars.CompletePump(t.m_iPlayerId, s, user); return;
			case RZ_ETaskKind.CRAFT: m_Crafting.CompleteRecipe(t.m_iPlayerId, s, t.m_iArg, user); return;
			case RZ_ETaskKind.BOIL: m_Crafting.CompleteBoil(t.m_iPlayerId, s, user); return;
			case RZ_ETaskKind.CRACK: m_Radio.CompleteCrack(t.m_iPlayerId, t.m_iArg, user, t.m_iSpot); return;
			case RZ_ETaskKind.TREAT: m_Medical.CompleteTreat(t.m_iPlayerId, user, t.m_Target, t.m_iArg); return;
			case RZ_ETaskKind.BREW: m_Crafting.CompleteBrew(t.m_iPlayerId, s, user); return;
			case RZ_ETaskKind.CLEAN:
			{
				if (m_GM.GetInfection())
					m_GM.GetInfection().CompleteClean(t.m_iPlayerId, t.m_iArg, user);
				return;
			}
		}
	}

	// ================================================================ Shared rules
	//------------------------------------------------------------------------------------------------
	//! Strongholds, safehouses, the lab and faction outposts: they can't burn, and their buildings
	//! are never locked.
	bool IsProtected(vector pos)
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses && houses.FindNearest(pos, 80) >= 0)
			return true;

		if (m_GM.HasLab() && RZ_Util.DistXZ(pos, m_GM.GetLabPosition()) <= 80)
			return true;

		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			RZ_Holding h = territory.FindHolding(-1, pos, 150);
			if (h && (h.m_bStronghold || RZ_Util.DistXZ(h.m_vPos, pos) <= 80))
				return true;
		}

		RZ_FactionPerks perks = m_GM.GetPerks();
		if (perks)
		{
			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				vector qm;
				if (perks.GetQuartermasterPos(f, qm) && RZ_Util.DistXZ(qm, pos) <= 100)
					return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsBackground(RZ_GameModeComponent gm, int playerId, string id)
	{
		RZ_Background bg = gm.GetPlayerBackground(playerId);
		return bg && bg.m_sId == id;
	}
}
