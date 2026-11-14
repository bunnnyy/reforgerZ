//! World interactions (docs/DESIGN.md, "World interactions"): loose loot in houses, wrecks' boots,
//! what bodies carry, locks and keys, readable things, fixable cars, crafting, water, barricades
//! and traps, generators, fire, bells and sirens, cracking secure radio channels and a medic
//! slowing a bite (treating each other is vanilla's).
//!
//! How it works on any map, without touching the map's prefabs:
//! - Data first. Every few seconds the server looks around each player (a small sphere
//!   query of static map objects) and recognises things by prefab or mesh name: buildings by
//!   type (pharmacy, garage, farm, police, shop, house), wrecks, workbenches, wells and pumps,
//!   stoves, bells, sirens, radios, generators. Buildings get a loot record (RZ_Loot.c).
//! - Only near a player, an invisible **interaction spot** (one small prefab with the generic
//!   puzzle object's components) is put on each thing, with replicated labels ("Ring the
//!   bell"). The puzzle actions route here (TAG). Spots are removed when everyone leaves. A
//!   locked door's spot sits on the door leaf (RZ_DoorGeometry.HandlePoint). A wreck's "spot" is
//!   its boot instead: an invisible storage opened with vanilla's inventory (RZ_WreckBoots.c). A
//!   thing whose prefab carries the set itself is its own spot (CanHost: vanilla's small house
//!   radio, overridden in the repo), so our prompts join its vanilla ones in one list.
//! - What happens is decided on the server. Loot lies in houses only near players and is
//!   kept as data otherwise; a wreck's boot is rolled when it's first put out, a body when it dies. What was emptied,
//!   opened or burned is remembered as data (positions and times) and saved; town loot comes
//!   back through RZ_TownMemory.
//! - Longer actions (forcing a lock, hotwiring, crafting, cracking a channel, cleaning a bite)
//!   are timed on the server tick: stay put until they finish.
//!
//! The parts live in their own files: RZ_Loot.c, RZ_Search.c, RZ_DoorLocks.c (with
//! RZ_DoorHooks.c and RZ_KeyItem.c), RZ_Cars.c, RZ_WreckBoots.c, RZ_Crafting.c (and water),
//! RZ_Traps.c (barricades, tripwires, spike traps, generators, molotovs and fire),
//! RZ_SecureRadio.c and RZ_Medical.c.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Interactions > General.
[BaseContainerProps()]
class RZ_InteractionSettings
{
	[Attribute("1", desc: "World interactions on: loose loot, wrecks' boots, what bodies carry, locks, cars, crafting, water, traps, generators, fire, bells, secure radio, a medic slowing a bite (docs/DESIGN.md, World interactions)")]
	bool m_bEnabled;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Interaction spot: a small invisible prop (a little collision box, no mesh) with RplComponent, ActionsManagerComponent and the generic puzzle object's components (RZ_PuzzleObjectComponent, RZ_ClueComponent, RZ_ReadClueAction, 4 RZ_PuzzleUseAction values 0-3, 4 RZ_PuzzleChoiceAction options 0-3). Empty = Puzzles > Generic object (visible)", "et")]
	ResourceName m_sSpotPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Door spot: the interaction spot for a locked door's Unlock, Lock and Force, on the door leaf at handle height. The same components as the Spot prefab with a tiny interaction box and a small action radius, so it doesn't cover vanilla's Open door at the handle. Empty = the Spot prefab", "et")]
	ResourceName m_sDoorSpotPrefab;

	[Attribute("30", desc: "Things within this distance (m) of a player get an interaction spot")]
	float m_fScanRadius;

	[Attribute("10", desc: "Seconds between looks around each player (only after they moved 10 m)")]
	float m_fScanSeconds;

	[Attribute("10", desc: "Most interaction spots around one player (the nearest win)")]
	int m_iSpotsPerPlayer;

	[Attribute("70", desc: "Most interaction spots on the whole server (performance cap)")]
	int m_iSpotCap;

	[Attribute("3.5", desc: "A timed action (a search, hotwiring, crafting...) stops if you move further than this (m)")]
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

	[Attribute("purification,purifying,watertablet,water_tablet,rz_tablets", desc: "Water purification tablet keywords (prefab name, lower case; rz_tablets is the repo's RZ_Tablets.et)")]
	string m_sTabletWords;

	[Attribute("0", desc: "Well spots: our own Fill a bottle at wells and pumps. Off by default: the game already lets you fill a bottle at a well, and both at once showed two actions")]
	bool m_bWellSpots;

	[Attribute("well,waterpump,water_pump,handpump,hand_pump,pump_water", desc: "Well and water pump prefab name keywords (with Well spots on)")]
	string m_sWellWords;

	[Attribute("stove,oven,fireplace,campfire,furnace,hearth", desc: "Stove and fireplace keywords (boil water there)")]
	string m_sStoveWords;

	[Attribute("30", desc: "Seconds to boil water")]
	float m_fBoilSeconds;

	// ---------------------------------------------------------------- Medical items (the bite) and treating each other
	[Attribute("medkit,firstaid,first_aid,ifak", desc: "First-aid kit keywords (cleaning a fresh bite with Disinfect and bandage it)")]
	string m_sMedkitWords;

	[Attribute("bandage,dressing,gauze", desc: "Bandage keywords (cleaning a fresh bite with Disinfect and bandage it)")]
	string m_sBandageWords;

	[Attribute("1.15", desc: "Medic slows a bite: a Nurse or Doctor treating a bitten player with vanilla's medical items (bandage, tourniquet, morphine, saline used on them) multiplies their time left by this, once per life. 1 = off")]
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
	vector m_vBuilding;		// the building it belongs to (locks, fire, building type)
	string m_sBuildingKey;
	string m_sType;			// building type: pharmacy, garage, farm, police, shop, house; or car
	string m_sNoun;			// "the blue door", "the wreck"
	int m_iRef = -1;		// car project id...
	float m_fSeen;			// hub clock when a player last saw it
	float m_fExpires = -1;	// a bite's spot: gone after this
	float m_fLift = 0.6;	// its prop stands this high (m) above m_vPos (a map object's: about counter height; a door's: 0, it's on the leaf)
	IEntity m_Body;
	RZ_PuzzleObjectComponent m_Comp;
	IEntity m_Host;			// the map object carries the prompts itself (CanHost): it's the body, never spawned or deleted
	bool m_bHosted;			// it had a host (gone: the thing was destroyed, so no spot)
	IEntity m_Fire;			// a fireplace (a campfire a player built, or a map's): cooks only while lit
	bool m_bWasFire;		// it was a fireplace (m_Fire gone: taken apart)
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
	float m_fTotal;			// how long it takes (the object menu's countdown bar)
	float m_fHealth = -1;
	bool m_bStill;			// moving at all (2 m) stops it; noise and damage too (cracking a channel)
	string m_sLabel;		// "the search"
	IEntity m_Target;		// cleaning another player's bite
}

//------------------------------------------------------------------------------------------------
//! A candidate found by the look-around.
class RZ_SpotCandidate
{
	int m_iKind;
	vector m_vPos;
	string m_sName;		// lower-case prefab or mesh name
	float m_fDist;
	IEntity m_Entity;	// the map object (buildings: their doors are looked at, RZ_DoorLocks; props: where the spot stands; wrecks: where their boot is)
}

//------------------------------------------------------------------------------------------------
//! Where a map object's interaction spot stands: the middle of its world bounds (many props have
//! their origin at a back corner), at about counter height, pulled a little toward its open side
//! (away from the wall it stands against). Worked out once, when the spot is made.
class RZ_SpotPlacer
{
	protected static const float MIN_LIFT = 0.6;
	protected static const float MAX_LIFT = 1.0;	// counter height
	protected static const float PULL = 0.35;		// at most this far (m) toward the open side
	protected static const float PROBE = 1.5;		// free space looked for beyond each side (m)
	protected static const float MAX_SIZE = 12;		// bigger bounds than this (m): the origin is used

	//------------------------------------------------------------------------------------------------
	//! The spot's ground point (its prop stands `lift` above it). The origin and 0.6 m without bounds.
	static vector Place(IEntity e, out float lift)
	{
		lift = MIN_LIFT;
		if (!e)
			return vector.Zero;

		vector origin = e.GetOrigin();
		vector mins;
		vector maxs;
		e.GetBounds(mins, maxs); // VERIFY: a static map prop's local bounds are its mesh's box (a stove's front is inside them), not zero or its whole composition
		vector size = maxs - mins;
		if (size[0] <= 0.05 || size[2] <= 0.05 || size[0] > MAX_SIZE || size[2] > MAX_SIZE)
			return origin;

		vector mat[4];
		e.GetWorldTransform(mat);
		lift = Math.Clamp(size[1], MIN_LIFT, MAX_LIFT);
		vector localMid = Vector((mins[0] + maxs[0]) * 0.5, mins[1], (mins[2] + maxs[2]) * 0.5);
		vector mid = mat[3] + mat[0] * localMid[0] + mat[1] * localMid[1] + mat[2] * localMid[2];
		vector probeAt = mid + Vector(0, lift, 0);

		// The prop's own four sides: the one with the most free space beyond it is the open side
		vector right = Flat(mat[0]);
		vector ahead = Flat(mat[2]);
		array<vector> dirs = {right, right * -1, ahead, ahead * -1};
		array<float> halves = {size[0] * 0.5, size[0] * 0.5, size[2] * 0.5, size[2] * 0.5};
		int best = -1;
		float bestFree = 0.3;
		foreach (int i, vector d : dirs)
		{
			float free = FreeBeyond(e, probeAt, d, halves[i]);
			if (free > bestFree)
			{
				bestFree = free;
				best = i;
			}
		}

		if (best < 0)
			return mid;

		return mid + dirs[best] * Math.Min(PULL, halves[best]);
	}

	//------------------------------------------------------------------------------------------------
	//! A direction without its height, unit length (x if it has none).
	protected static vector Flat(vector d)
	{
		vector f = Vector(d[0], 0, d[2]);
		float len = f.Length();
		if (len < 0.01)
			return Vector(1, 0, 0);

		return f * (1 / len);
	}

	//------------------------------------------------------------------------------------------------
	//! How much room (m) there is past the prop's side in `dir`, up to PROBE: a flat trace from the
	//! middle that ignores the prop itself.
	protected static float FreeBeyond(IEntity e, vector from, vector dir, float half)
	{
		float len = half + PROBE;
		TraceParam param = new TraceParam();
		param.Start = from;
		param.End = from + dir * len;
		param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		param.Exclude = e;
		float frac = ChimeraCharacter.TraceMoveWithoutCharacters(GetGame().GetWorld(), param);
		return frac * len - half;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_InteractionSave
{
	int seed;
	float clock;
	ref array<string> searchedKey = {};	// unused since car boots aren't searched (RZ_WreckBoots); kept so older saves load
	ref array<float> searchedAt = {};
	ref array<string> openKey = {};
	ref array<float> openAt = {};
	ref array<string> forcedKey = {};
	ref array<float> burnX = {};
	ref array<float> burnZ = {};
	ref array<float> burnAt = {};
	ref array<string> wreckKey = {};		// "pump:<key>" (RZ_Cars), "boot:<key>" (RZ_WreckBoots)
	ref array<float> wreckAt = {};
	ref array<string> lootKey = {};		// loose loot: each building's rolled-at and emptied-at times (no items)
	ref array<float> lootRolled = {};
	ref array<float> lootEmptied = {};
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
	//! The first carried item that matches the words (see Matches), null if none.
	static IEntity FirstMatch(IEntity character, array<string> words)
	{
		if (!character || !words || words.IsEmpty())
			return null;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return null;

		array<IEntity> items = {};
		inv.GetItems(items);
		foreach (IEntity item : items)
		{
			if (Matches(item, words))
				return item;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! What the character holds in its hands: the item or gadget in the right hand (a fuel can),
	//! else the held gadget, else the current weapon or item. null if nothing. Any machine.
	static IEntity HeldItem(IEntity character)
	{
		ChimeraCharacter cc = ChimeraCharacter.Cast(character);
		if (!cc)
			return null;

		CharacterControllerComponent ctrl = cc.GetCharacterController();
		if (ctrl)
		{
			IEntity right = ctrl.GetRightHandItem(); // VERIFY: a fuel can held in the hands is seen on the server (here, as the held gadget, or as the item in hands)
			if (right)
				return right;
		}

		SCR_GadgetManagerComponent gadgets = SCR_GadgetManagerComponent.GetGadgetManager(character);
		if (gadgets && gadgets.GetHeldGadget())
			return gadgets.GetHeldGadget();

		if (ctrl)
			return ctrl.GetCurrentItemInHands();

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Server: deletes one item the character carries or holds.
	static void DeleteCarried(IEntity character, IEntity item)
	{
		if (!item)
			return;

		SCR_InventoryStorageManagerComponent inv;
		if (character)
			inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));

		if (inv && inv.TryDeleteItem(item))
			return;

		SCR_EntityHelper.DeleteEntityAndChildren(item);
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

			if (inv.TryDeleteItem(item))
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
//! Spot kinds (the same values as RZ_Interactions.KIND_*), for switch statements. Buildings and
//! bodies have none: loot lies loose in buildings (RZ_Loot) and bodies use vanilla's inventory.
enum RZ_ESpotKind
{
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
	WOUND,
	BURN
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
	BURN,
	FIT,
	CLEAN,
	BREW,
	COOK
}

//------------------------------------------------------------------------------------------------
class RZ_Interactions
{
	static const int TAG = -9;				// RZ_PuzzleObjectComponent / RZ_ClueComponent mission id for spots
	static const int TAG_NOTE = -10;		// readable things (lying in a house, or left by a search)

	static const int KIND_WRECK = 0;
	static const int KIND_WORKBENCH = 1;
	static const int KIND_WELL = 2;
	static const int KIND_FUELPUMP = 3;
	static const int KIND_STOVE = 4;
	static const int KIND_BELL = 5;
	static const int KIND_SIREN = 6;
	static const int KIND_RADIO = 7;
	static const int KIND_GENERATOR = 8;
	static const int KIND_CAR = 9;
	static const int KIND_DOOR = 10;		// a locked place's outside door (RZ_DoorLocks)
	static const int KIND_WOUND = 11;		// a fresh bite at a player's feet (RZ_Infection): Clean the bite
	static const int KIND_BURN = 12;		// a building's doorway near someone holding fuel (RZ_Traps): Douse the doorway
	static const int CAND_BUILDING = 100;	// a look-around candidate only (doors, loose loot, church bells): never a spot, not in RZ_ESpotKind

	static const int TASK_SEARCH = 0;		// unused (car boots aren't searched: RZ_WreckBoots); kept for the enum's order
	static const int TASK_FORCE = 1;
	static const int TASK_HOTWIRE = 2;
	static const int TASK_CRAFT = 3;
	static const int TASK_SIPHON = 4;		// unused (wrecks have no fuel); kept for the enum's order
	static const int TASK_PUMP = 5;
	static const int TASK_STRIP = 6;		// unused (a wreck's part is in its boot); kept for the enum's order
	static const int TASK_BOIL = 7;
	static const int TASK_CRACK = 8;
	static const int TASK_BURN = 9;
	static const int TASK_FIT = 10;
	static const int TASK_CLEAN = 11;		// cleaning a fresh bite (RZ_Infection)
	static const int TASK_BREW = 12;		// brewing a home remedy on a stove
	static const int TASK_COOK = 13;		// a stove recipe (RZ_Crafting): its arg is its place in the stove's list

	protected static const float DESPAWN_MARGIN = 25;
	protected static const float RESCAN_MOVE = 10;
	protected static const int SEARCHED_KEPT = 800;

	protected RZ_GameModeComponent m_GM;
	protected RZ_InteractionSettings m_Settings;
	protected ref RZ_Search m_Search;
	protected ref RZ_Loot m_Loot;
	protected ref RZ_DoorLocks m_Doors;
	protected ref RZ_Cars m_Cars;
	protected ref RZ_WreckBoots m_Boots;
	protected ref RZ_FuelPumps m_Pumps;
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
	void RZ_Interactions(RZ_GameModeComponent gm, RZ_InteractionSettings settings, RZ_SearchSettings search, RZ_LootSettings loot,
		RZ_CarSettings cars, RZ_CraftSettings craft, RZ_TrapSettings traps, RZ_RadioCrackSettings radio)
	{
		m_GM = gm;
		m_Settings = settings;
		m_Search = new RZ_Search(gm, this, search);
		m_Loot = new RZ_Loot(gm, this, loot);
		m_Doors = new RZ_DoorLocks(gm, this, m_Search, search);
		m_Cars = new RZ_Cars(gm, this, cars);
		m_Boots = new RZ_WreckBoots(gm, this, cars);
		m_Pumps = new RZ_FuelPumps(gm, this, cars);
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
	RZ_Loot GetLoot() { return m_Loot; }
	RZ_DoorLocks GetDoors() { return m_Doors; }
	RZ_Cars GetCars() { return m_Cars; }
	RZ_WreckBoots GetBoots() { return m_Boots; }
	RZ_FuelPumps GetPumps() { return m_Pumps; }
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
		m_Loot.SaveInto(data);
		m_Traps.SaveInto(data);
		m_Cars.SaveInto(data);
		m_Boots.SaveInto(data);
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
			m_Loot.RestoreFrom(m_Restored);
			m_Traps.RestoreFrom(m_Restored);
			m_Cars.RestoreWrecks(m_Restored);
			m_Boots.RestoreFrom(m_Restored);
		}

		m_bActive = true;
		m_Search.Setup();
		m_Loot.Setup();
		m_Crafting.Setup();
		m_Traps.Setup();
		if (m_Restored)
			m_Cars.Setup(m_Restored.cars);
		else
			m_Cars.Setup(null);

		m_Restored = null;
		RZ_Log.Info(string.Format("World interactions: on. %1 fixable cars on the map.", m_Cars.Count()));
		if (!m_Boots.IsOn())
			RZ_Log.Info("Wreck boots: off (Interactions > Cars > Wreck boot prefab is empty, or cars are off). Wrecks hold nothing.");
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
		m_Loot.Tick(players);
		m_Doors.Tick(players);
		m_Doors.TickBlockers(ids, players);
		TickTasks(dt);
		m_Traps.Tick(dt, ids, players);
		m_Radio.Tick(dt, ids, players);
		m_Search.Tick(dt);
		m_Boots.Tick();
		m_Pumps.Tick();
	}

	//------------------------------------------------------------------------------------------------
	//! A new life: timed actions and listening don't carry over.
	void OnPlayerSpawned(int playerId)
	{
		if (!m_bActive)
			return;

		CancelTask(playerId, "");
		m_Radio.OnPlayerSpawned(playerId);
		m_mScanAt.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! From the game mode: infected died here. Near a player, the body carries some loot.
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
		float reach = Math.Max(m_Settings.m_fScanRadius, m_Loot.GetSpawnRadius());
		world.QueryEntitiesBySphere(center, reach, OnScanEntity, null, EQueryEntitiesFlags.STATIC);
		// A fireplace a player built is no map object: found among what moves, as a stove
		world.QueryEntitiesBySphere(center, m_Settings.m_fScanRadius, OnScanFire, null, EQueryEntitiesFlags.DYNAMIC); // VERIFY: a fireplace spawned at run time (the campfire mod's) is found by the dynamic query, not the static one

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

			// A locked place's outside doors get a spot each (Unlock, Lock, Force); the building
			// itself never gets one (its loot lies loose: RZ_Loot)
			if (c.m_iKind == CAND_BUILDING)
			{
				added += m_Doors.OnBuildingSeen(c.m_Entity, c.m_sName, m_Settings.m_iSpotsPerPlayer - added);
				continue;
			}

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
	//! Among what moves only a fireplace counts (a campfire a player built): a stove like the map's,
	//! by its name.
	protected bool OnScanFire(IEntity e)
	{
		if (!e || ChimeraCharacter.Cast(e) || !RZ_Fires.IsFireplace(e))
			return true;

		return OnScanEntity(e);
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

		// The cheap name filter first: only what it keeps is looked up among the scenery
		int kind = Classify(name, building);
		if (kind < 0 || RZ_CampLife.IsDressing(e))
			return true;

		vector origin = e.GetOrigin();
		float dist = RZ_Util.DistXZ(origin, m_vScanCenter);
		// The sphere reaches out to the loot's spawn radius: past the spot radius only buildings
		// count, and only for their loot. Any destructible building has loot, whatever spot its
		// name gives it (a fuel station's building is a fuel spot, a workshop a workbench)
		if (kind == CAND_BUILDING || building)
			m_Loot.OnBuildingSeen(e, name);

		if (dist > m_Settings.m_fScanRadius)
			return true;

		RZ_SpotCandidate c = new RZ_SpotCandidate();
		c.m_iKind = kind;
		c.m_vPos = origin;
		c.m_sName = name;
		c.m_fDist = dist;
		c.m_Entity = e;

		m_aCand.Insert(c);

		// A church gets a bell
		if (kind == CAND_BUILDING && RZ_LandmarkFinder.Matches("church", name))
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

		if (m_Settings.m_bWellSpots && !building && (RZ_LandmarkFinder.Matches("well", name) || RZ_ItemHelper.NameHas(name, m_aWellWords)))
			return KIND_WELL;

		if (!building && RZ_ItemHelper.NameHas(name, m_aStoveWords))
			return KIND_STOVE;

		if (!building && m_Radio.IsHouseRadioName(name))
			return KIND_RADIO;

		// Furniture is where loose loot lies (RZ_LootAnchors), not a thing of its own
		if (!building && RZ_LootAnchors.IsFurnitureName(name))
			return -1;

		if (building || name.Contains("/buildings/") || name.Contains("house"))
			return CAND_BUILDING;

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
		if (c.m_Entity)
		{
			float lift;
			s.m_vPos = RZ_SpotPlacer.Place(c.m_Entity, lift);
			s.m_fLift = lift;
		}

		// A map object with our set on it (vanilla's house radio, overridden in the repo) is its own
		// spot: our prompts join its vanilla ones (Turn on) in one list
		if (CanHost(c.m_Entity, c.m_iKind))
		{
			s.m_Host = c.m_Entity;
			s.m_bHosted = true;
		}

		// A fireplace cooks only while it burns
		if (c.m_iKind == KIND_STOVE && RZ_Fires.IsFireplace(c.m_Entity))
		{
			s.m_Fire = c.m_Entity;
			s.m_bWasFire = true;
		}

		if (c.m_iKind == KIND_RADIO || c.m_iKind == KIND_STOVE || c.m_iKind == KIND_WORKBENCH)
		{
			RZ_SpotCandidate b = NearestBuilding(c.m_vPos);
			if (b)
			{
				s.m_vBuilding = b.m_vPos;
				s.m_sType = m_Search.BuildingType(b.m_sName);
			}
		}

		// A wreck's boot stands at its rear (the key stays the wreck's own position)
		if (c.m_iKind == KIND_WRECK)
		{
			s.m_sType = "car";
			if (c.m_Entity)
				s.m_vPos = RZ_WreckBoots.BootPoint(c.m_Entity);
		}

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
			if (c.m_iKind != CAND_BUILDING)
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
	//! A spot other systems ask for (a bite, car projects). Returns its id, -1 at the cap.
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
	//! A spot with a stable key (a burn spot at a doorway): the existing one is kept (and moved to
	//! pos if it moved), else a new one is added. null at the cap.
	RZ_Spot EnsureSpot(int kind, string key, vector pos, vector buildingPos, string type, string noun)
	{
		RZ_Spot s = FindSpotByKey(key);
		if (s)
		{
			s.m_fSeen = m_fClock;
			if (RZ_Util.DistXZ(s.m_vPos, pos) > 0.5)
			{
				DespawnSpot(s);
				s.m_vPos = pos;
			}

			return s;
		}

		if (m_aSpots.Count() >= m_Settings.m_iSpotCap)
			return null;

		s = new RZ_Spot();
		s.m_iId = m_iNextSpot;
		m_iNextSpot++;
		s.m_iKind = kind;
		s.m_sKey = key;
		s.m_vPos = pos;
		s.m_vBuilding = buildingPos;
		s.m_sBuildingKey = PosKey(buildingPos, 4);
		s.m_sType = type;
		s.m_sNoun = noun;
		s.m_fSeen = m_fClock;
		m_aSpots.Insert(s);
		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! Every spot of this kind whose key isn't in `keep` goes (with its prop).
	void RemoveSpotsOfKind(int kind, array<string> keep)
	{
		for (int i = m_aSpots.Count() - 1; i >= 0; i--)
		{
			RZ_Spot s = m_aSpots[i];
			if (s.m_iKind != kind || keep.Contains(s.m_sKey))
				continue;

			DespawnSpot(s);
			m_aSpots.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A spot on a locked place's door (RZ_DoorLocks), its prop exactly at pos (on the leaf). False
	//! if it exists already (a double door shares one) or at the cap. The door's own spot moves to
	//! pos when `canMove` (the door is shut) and pos is more than 0.3 m from where it is.
	bool AddDoorSpot(vector pos, vector buildingPos, string buildingKey, string type, string noun, int doorIndex, bool canMove)
	{
		string key = "door_" + PosKey(pos, 1);
		foreach (RZ_Spot o : m_aSpots)
		{
			if (o.m_iKind != KIND_DOOR || o.m_sBuildingKey != buildingKey)
			{
				if (o.m_sKey == key)
				{
					o.m_fSeen = m_fClock;
					return false;
				}

				continue;
			}

			if (o.m_iRef == doorIndex)
			{
				o.m_fSeen = m_fClock;
				if (canMove && vector.Distance(o.m_vPos, pos) > 0.3)
				{
					DespawnSpot(o);
					o.m_vPos = pos;
				}

				return false;
			}

			if (RZ_Util.DistXZ(o.m_vPos, pos) < 1.8)
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
		s.m_fLift = 0;
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
	//! The nearest spot of `kind` within `radius` of pos (strangers' wrecks), null if none.
	RZ_Spot NearestSpot(int kind, vector pos, float radius)
	{
		RZ_Spot best;
		float bestDist = radius;
		foreach (RZ_Spot s : m_aSpots)
		{
			if (s.m_iKind != kind)
				continue;

			float d = RZ_Util.DistXZ(s.m_vPos, pos);
			if (d <= bestDist)
			{
				best = s;
				bestDist = d;
			}
		}

		return best;
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
	//! A locked door's spot: the door spot prefab, else the interaction spot.
	protected ResourceName DoorSpotPrefab()
	{
		if (!m_Settings.m_sDoorSpotPrefab.IsEmpty())
			return m_Settings.m_sDoorSpotPrefab;

		return SpotPrefab();
	}

	//------------------------------------------------------------------------------------------------
	//! The interaction spot prefab (else the generic puzzle object), for systems that place their own.
	ResourceName GetSpotPrefab()
	{
		return SpotPrefab();
	}

	//------------------------------------------------------------------------------------------------
	//! The door spot prefab (an invisible readable box), for systems that put something to read on a
	//! real door (an opening's scratched door).
	ResourceName GetDoorSpotPrefab()
	{
		return DoorSpotPrefab();
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
			// A hosted spot whose thing is gone (destroyed): no spot
			bool lost = s.m_bHosted && !s.m_Host;
			if (expired || far || lost)
			{
				DespawnSpot(s);
				m_aSpots.Remove(i);
				continue;
			}

			if (!s.m_Body)
				SpawnSpot(s);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnSpot(RZ_Spot s)
	{
		// A wreck: its boot, a storage vanilla's inventory opens (no labels, no puzzle actions)
		if (s.m_iKind == KIND_WRECK)
		{
			s.m_Body = m_Boots.PutOut(s);
			return;
		}

		// A petrol station's pump with Cars > Fuel pump station: vanilla's fuel point (no prompts of ours)
		if (s.m_iKind == KIND_FUELPUMP && m_Pumps.IsOn())
		{
			s.m_Body = m_Pumps.PutOut(s);
			return;
		}

		// The thing carries our set itself: tag it, nothing is spawned
		if (s.m_bHosted)
		{
			if (s.m_Host)
				AttachLabels(s, s.m_Host);

			return;
		}

		ResourceName prefab = SpotPrefab();
		if (s.m_iKind == KIND_DOOR)
			prefab = DoorSpotPrefab();

		s.m_Body = RZ_Util.SpawnPrefab(prefab, s.m_vPos + Vector(0, s.m_fLift, 0), 0);
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
		if (s.m_Comp)
			s.m_Comp.SetMenu(SpotUsesMenu(s));

		RZ_ClueComponent.Tag(body, TAG, s.m_iId, read);
	}

	//------------------------------------------------------------------------------------------------
	//! Kinds whose prompts go into the object menu (docs/UI.md, "Every interaction, by kind"): a
	//! stove, a generator, a house radio, a well and a fuel pump have several options or something
	//! to read. Bells, sirens, doors, a bite and a doorway to burn stay quick prompts; a car and a
	//! workbench have screens of their own.
	static bool UsesMenu(int kind)
	{
		return kind == KIND_STOVE || kind == KIND_GENERATOR || kind == KIND_RADIO || kind == KIND_WELL || kind == KIND_FUELPUMP;
	}

	//------------------------------------------------------------------------------------------------
	//! A map object that carries the generic set itself (the puzzle object and clue components and
	//! their actions, hidden until tagged): it is its spot, so our prompts sit in its own prompt list
	//! beside its vanilla ones. Today vanilla's small house radio, which the repo overrides
	//! (docs/WORKBENCH_SETUP.md, "Add to prefabs you already have"); a wreck has its boot instead.
	static bool CanHost(IEntity e, int kind)
	{
		if (!e || kind == KIND_WRECK || kind == KIND_DOOR)
			return false;

		// VERIFY: the repo's same-GUID copy of vanilla's RadioSmall_01_Base.et (our set added) replaces vanilla's on every map's radios
		return e.FindComponent(RZ_PuzzleObjectComponent) != null && e.FindComponent(RZ_ClueComponent) != null && e.FindComponent(RplComponent) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! This spot's prompts go into the object menu: by kind, except a stove while the cooking screen
	//! is used (its read opens that screen and its slots hide behind it, as a workbench's do).
	protected bool SpotUsesMenu(RZ_Spot s)
	{
		if (s.m_iKind == KIND_STOVE && m_Crafting.StoveScreenOn())
			return false;

		return UsesMenu(s.m_iKind);
	}

	//------------------------------------------------------------------------------------------------
	protected void DespawnSpot(RZ_Spot s)
	{
		// A wreck's boot: what's left in it is kept as data, then it goes
		if (s.m_iKind == KIND_WRECK)
			m_Boots.PutAway(s);

		if (s.m_iKind == KIND_FUELPUMP && m_Pumps.IsOn())
			m_Pumps.PutAway(s);

		// The map's own thing stays: untagged, our prompts on it hide again (hidden until tagged)
		if (s.m_bHosted)
		{
			if (s.m_Body)
			{
				RZ_PuzzleObjectComponent.Tag(s.m_Body, -1, -1, "", "");
				RZ_ClueComponent.Tag(s.m_Body, -1, -1, "");
			}

			s.m_Body = null;
			s.m_Comp = null;
			return;
		}

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
		s.m_Comp.SetMenu(SpotUsesMenu(s));
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
		switch (s.m_iKind)
		{
			case RZ_ESpotKind.DOOR:
			{
				m_Doors.Labels(s, read, choiceLabels);
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
				// With the cooking screen the read opens it, and the slots hide behind it on the client
				if (m_Crafting.StoveScreenOn())
					read = RZ_Crafting.STOVE_LABEL;

				choiceLabels = m_Crafting.StoveLabels();
				break;
			}
			case RZ_ESpotKind.WOUND:
			{
				if (m_GM.GetInfection())
					m_GM.GetInfection().WoundLabels(read, choiceLabels);
				break;
			}
			case RZ_ESpotKind.BURN:
			{
				// Only the choice, and only for someone whose hands hold fuel (RZ_PuzzleChoiceAction)
				read = RZ_ClueComponent.NO_READ;
				choiceLabels = RZ_Traps.BURN_LABEL;
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
				choiceLabels = "Tune through the open channels|Work on a scrambled channel|";
				if (RZ_Dialogue.IsOn() && RZ_DialogueSettings.Get().m_bTrees)
					choiceLabels += "Call on the open channel";

				// The radio screen: every channel, its latest line and the log (option 3)
				if (RZ_RadioScreen.Available())
					choiceLabels += "|" + RZ_RadioScreen.VIEW_LABEL;
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

		switch (s.m_iKind)
		{
			case RZ_ESpotKind.DOOR: m_Doors.Describe(playerId, s); return;
			case RZ_ESpotKind.CAR: m_Cars.Inspect(playerId, s, user); return;
			case RZ_ESpotKind.WORKBENCH: m_Crafting.Look(playerId, s, user); return;
			case RZ_ESpotKind.STOVE:
			{
				if (m_Crafting.UseStove(playerId, s, user))
					return;
				break;
			}
			case RZ_ESpotKind.RADIO: m_Radio.ListenHouseRadio(playerId, s, user); return;
			case RZ_ESpotKind.GENERATOR: m_Traps.DescribeFoundGenerator(playerId, s); return;
			case RZ_ESpotKind.BURN: return;
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
			return "Here";

		string first = t.Substring(0, 1);
		first.ToUpper(); // in place, like ToLower()
		return first + t.Substring(1, t.Length() - 1);
	}

	//------------------------------------------------------------------------------------------------
	//! A spot's title by its id (the object menu's header), "Here" when it's gone.
	string TitleOf(int spotId)
	{
		RZ_Spot s = FindSpot(spotId);
		if (!s)
			return "Here";

		return Title(s);
	}

	//------------------------------------------------------------------------------------------------
	//! RZ_PuzzleChoiceAction on a spot.
	void HandleChoice(int playerId, int id, int option, IEntity owner, IEntity user)
	{
		RZ_Spot s = FindSpot(id);
		if (!s || !user)
			return;

		// A house radio's View radio only shows, so it's fine mid-task (cracking a channel)
		bool viewing = s.m_iKind == KIND_RADIO && option == 3;
		if (HasTask(playerId) && !viewing)
		{
			m_GM.NotifyPlayer(playerId, Title(s), "You're already busy with something. Finish it first, or step away to stop.");
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
			case RZ_ESpotKind.BURN: m_Traps.StartBurn(playerId, s, user); return;
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
	//! The player's timed action, or null.
	RZ_TimedTask GetTask(int playerId)
	{
		foreach (RZ_TimedTask t : m_aTasks)
		{
			if (t.m_iPlayerId == playerId)
				return t;
		}

		return null;
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
		t.m_fTotal = t.m_fLeft;
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
			m_Cars.OnTaskStopped(playerId);
			m_Crafting.OnTaskStopped(playerId);
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
				int stopped = t.m_iPlayerId;
				RZ_ObjectScreen.WrapBegin(stopped);
				m_GM.NotifyPlayer(stopped, "Stopped", string.Format("%1 You leave %2 unfinished.", why, t.m_sLabel));
				m_aTasks.Remove(i);
				m_Cars.OnTaskStopped(stopped);
				m_Crafting.OnTaskStopped(stopped);
				RZ_ObjectScreen.WrapEnd(stopped);
				continue;
			}

			t.m_fLeft = t.m_fLeft - dt;
			if (t.m_fLeft > 0)
				continue;

			// Done: what it says goes into the player's object menu if one is open
			int done = t.m_iPlayerId;
			m_aTasks.Remove(i);
			RZ_ObjectScreen.WrapBegin(done);
			Complete(t, user);
			RZ_ObjectScreen.WrapEnd(done);
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

		// Cleaning someone's bite: the patient has to stay too
		if (t.m_iKind == TASK_CLEAN && (!t.m_Target || !RZ_Util.IsCharacterAlive(t.m_Target) || RZ_Util.DistXZ(t.m_Target.GetOrigin(), user.GetOrigin()) > reach))
			return "They're not next to you any more.";

		if (!t.m_bStill)
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
			case RZ_ETaskKind.FORCE:
			{
				if (t.m_iArg == RZ_DoorLocks.PICK_ARG)
					m_Doors.CompletePick(t.m_iPlayerId, s, user);
				else
					m_Search.CompleteForce(t.m_iPlayerId, s, user);

				return;
			}
			case RZ_ETaskKind.BURN: m_Traps.CompleteBurn(t.m_iPlayerId, s, user); return;
			case RZ_ETaskKind.HOTWIRE: m_Cars.CompleteHotwire(t.m_iPlayerId, s, t.m_iArg); return;
			case RZ_ETaskKind.FIT: m_Cars.CompleteFit(t.m_iPlayerId, s, t.m_iArg, user); return;
			case RZ_ETaskKind.PUMP: m_Cars.CompletePump(t.m_iPlayerId, s, user); return;
			case RZ_ETaskKind.CRAFT: m_Crafting.CompleteRecipe(t.m_iPlayerId, s, t.m_iArg, user); return;
			case RZ_ETaskKind.BOIL: m_Crafting.CompleteBoil(t.m_iPlayerId, s, user); return;
			case RZ_ETaskKind.CRACK: m_Radio.CompleteCrack(t.m_iPlayerId, t.m_iArg, user, t.m_iSpot); return;
			case RZ_ETaskKind.BREW: m_Crafting.CompleteBrew(t.m_iPlayerId, s, user); return;
			case RZ_ETaskKind.COOK: m_Crafting.CompleteCook(t.m_iPlayerId, s, t.m_iArg, user); return;
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
