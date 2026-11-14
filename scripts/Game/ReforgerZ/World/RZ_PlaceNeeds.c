//! Task places and things (docs/PLACES.md): what a job's place, and the things its words name, must
//! be for its words to be true, as data. Every picker that puts a task somewhere and names it in
//! words ("the houses over the way", "the well in Tisy", "what's left of a big one by the road") has
//! a need: place flags with their ranges, and things (a prop, a person, an item, remains, or a prefab
//! slot they're made from). The picker tries its candidates against the need and takes the first that
//! fits, or offers nothing; the place it settles on is checked again (RZ_PlaceNeeds.Verify), the
//! things once they're spawned (VerifyThings), and a failure is logged as a warning, live in play. The
//! headless audit (RZ_PlaceAudit, tools/runtime_test/run.sh tasks) asks the same pickers for places
//! across a map and checks each with the same code. Server only.

//------------------------------------------------------------------------------------------------
//! What a place must have. Bits: a need holds any mix.
enum RZ_EPlaceFlag
{
	DRY = 1,				// not in the sea, a lake or a river
	NOT_AT_LINE = 2,		// not at the quarantine line
	NOT_STEEP = 4,			// the ground under it less steep than m_fMaxSlope degrees
	HOUSES = 8,				// at least m_iHouses houses within m_fHouseRange
	BUILDING = 16,			// a building or landmark of kind m_sBuilding within m_fBuildingRange
	ROAD = 32,				// a road within m_fRoadRange
	WATER_SOURCE = 64,		// a well, a hand pump or a water tower within m_fWaterRange
	OPEN_GROUND = 128,		// no building and at most m_iMaxTrees trees within m_fOpenRange
	TREES = 256,			// at least m_iTrees trees within m_fTreeRange
	IN_TOWN = 512,			// within the town its words name (else the nearest), its radius + m_fTownReach
	OUTSIDE_TOWN = 1024,	// outside every town's radius
	NEAR_FACTION = 2048,	// a holding or road post of m_iFaction (-1 any) within m_fFactionRange
	AWAY_FROM_FACTION = 4096,	// none within m_fFactionRange
	NOT_LOCKED = 8192,		// not behind a lock (RZ_LockedSpots)
	HIGH_GROUND = 16384,	// at least m_fRise above the ground 100 m round it (a ridge, a hilltop)
	MAINLAND = 32768		// on the map's main landmass, not an island (RZ_Mainland)
}

//------------------------------------------------------------------------------------------------
//! What kind of thing a task's words name.
enum RZ_EThingKind
{
	SLOT,		// a prefab slot that must be set (what's made from it is checked elsewhere)
	ITEM,		// an item made from the slot, lying within range of the place once the step starts
	PROP,		// a prop made from the slot, within range once the step starts
	NPC,		// a living person within range once the step starts
	REMAINS		// a dead body within range once the step starts
}

//------------------------------------------------------------------------------------------------
//! One thing a task's words name: what it is, the prefab slot it's made from, and what happens
//! when that slot is empty (never a silent missing thing).
class RZ_PlaceThing
{
	int m_eKind;
	string m_sName;			// "the sample"
	string m_sSlot;			// RZ_PlaceThings.Slot's key: sample, remains, contact, key, prop.parts...
	float m_fRange;
	string m_sFallback;		// what the game does without it ("the step isn't offered"); empty: a failure

	//------------------------------------------------------------------------------------------------
	string Text()
	{
		string text = string.Format("%1 %2", typename.EnumToString(RZ_EThingKind, m_eKind), m_sName);
		if (!m_sSlot.IsEmpty())
			text += " [" + RZ_PlaceThings.SlotLabel(m_sSlot) + "]";

		if (m_fRange > 0)
			text += string.Format(" within %1 m", RZ_Util.RoundToInt(m_fRange));

		return text;
	}
}

//------------------------------------------------------------------------------------------------
//! One kind of task's need. Made in RZ_PlaceNeeds.Build; read-only after.
class RZ_PlaceNeed
{
	string m_sKey;				// "opening.favour"
	string m_sWords;			// what the player reads about the place
	int m_iFlags = RZ_EPlaceFlag.DRY | RZ_EPlaceFlag.NOT_AT_LINE | RZ_EPlaceFlag.MAINLAND;
	float m_fMaxSlope = 30;
	int m_iHouses;
	float m_fHouseRange;
	string m_sBuilding;			// a kind (RZ_PlaceCheck.KindNear): any, house, clinic, fuel, well, pumphouse...
	float m_fBuildingRange;
	float m_fRoadRange;
	float m_fWaterRange;
	float m_fOpenRange;
	int m_iMaxTrees;
	int m_iTrees;
	float m_fTreeRange;
	float m_fTownReach;
	int m_iFaction = -1;
	float m_fFactionRange;
	float m_fRise;
	ref array<ref RZ_PlaceThing> m_aThings = {};

	//------------------------------------------------------------------------------------------------
	void RZ_PlaceNeed(string key, string words)
	{
		m_sKey = key;
		m_sWords = words;
	}

	//------------------------------------------------------------------------------------------------
	bool Has(int flag)
	{
		return (m_iFlags & flag) != 0;
	}

	//------------------------------------------------------------------------------------------------
	void Steep(float maxDegrees)
	{
		m_iFlags = m_iFlags | RZ_EPlaceFlag.NOT_STEEP;
		m_fMaxSlope = maxDegrees;
	}

	//------------------------------------------------------------------------------------------------
	void Houses(int count, float range)
	{
		m_iFlags = m_iFlags | RZ_EPlaceFlag.HOUSES;
		m_iHouses = count;
		m_fHouseRange = range;
	}

	//------------------------------------------------------------------------------------------------
	void Building(string kind, float range)
	{
		m_iFlags = m_iFlags | RZ_EPlaceFlag.BUILDING;
		m_sBuilding = kind;
		m_fBuildingRange = range;
	}

	//------------------------------------------------------------------------------------------------
	void Road(float range)
	{
		m_iFlags = m_iFlags | RZ_EPlaceFlag.ROAD;
		m_fRoadRange = range;
	}

	//------------------------------------------------------------------------------------------------
	void Water(float range)
	{
		m_iFlags = m_iFlags | RZ_EPlaceFlag.WATER_SOURCE;
		m_fWaterRange = range;
	}

	//------------------------------------------------------------------------------------------------
	void Open(float range, int maxTrees)
	{
		m_iFlags = m_iFlags | RZ_EPlaceFlag.OPEN_GROUND;
		m_fOpenRange = range;
		m_iMaxTrees = maxTrees;
	}

	//------------------------------------------------------------------------------------------------
	void Trees(int count, float range)
	{
		m_iFlags = m_iFlags | RZ_EPlaceFlag.TREES;
		m_iTrees = count;
		m_fTreeRange = range;
	}

	//------------------------------------------------------------------------------------------------
	void InTown(float reach)
	{
		m_iFlags = m_iFlags | RZ_EPlaceFlag.IN_TOWN;
		m_fTownReach = reach;
	}

	//------------------------------------------------------------------------------------------------
	void OutsideTown()
	{
		m_iFlags = m_iFlags | RZ_EPlaceFlag.OUTSIDE_TOWN;
	}

	//------------------------------------------------------------------------------------------------
	void NearFaction(int faction, float range)
	{
		m_iFlags = m_iFlags | RZ_EPlaceFlag.NEAR_FACTION;
		m_iFaction = faction;
		m_fFactionRange = range;
	}

	//------------------------------------------------------------------------------------------------
	void AwayFromFaction(int faction, float range)
	{
		m_iFlags = m_iFlags | RZ_EPlaceFlag.AWAY_FROM_FACTION;
		m_iFaction = faction;
		m_fFactionRange = range;
	}

	//------------------------------------------------------------------------------------------------
	void NotLocked()
	{
		m_iFlags = m_iFlags | RZ_EPlaceFlag.NOT_LOCKED;
	}

	//------------------------------------------------------------------------------------------------
	void High(float rise)
	{
		m_iFlags = m_iFlags | RZ_EPlaceFlag.HIGH_GROUND;
		m_fRise = rise;
	}

	//------------------------------------------------------------------------------------------------
	//! A thing its words name (RZ_EThingKind), the slot it's made from, and the fallback without it.
	void Thing(int kind, string name, string slot, float range, string fallback)
	{
		RZ_PlaceThing t = new RZ_PlaceThing();
		t.m_eKind = kind;
		t.m_sName = name;
		t.m_sSlot = slot;
		t.m_fRange = range;
		t.m_sFallback = fallback;
		m_aThings.Insert(t);
	}

	//------------------------------------------------------------------------------------------------
	//! A thing that is checked once the step starts (not a bare slot).
	bool HasPresentThings()
	{
		foreach (RZ_PlaceThing t : m_aThings)
		{
			if (t.m_eKind != RZ_EThingKind.SLOT)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! "DRY NOT_AT_LINE HOUSES(1/15 m)": the need as the audit and the docs print it.
	string FlagsText()
	{
		string text = "";
		if (Has(RZ_EPlaceFlag.DRY))
			text += " DRY";
		if (Has(RZ_EPlaceFlag.NOT_AT_LINE))
			text += " NOT_AT_LINE";
		if (Has(RZ_EPlaceFlag.MAINLAND))
			text += " MAINLAND";
		if (Has(RZ_EPlaceFlag.NOT_STEEP))
			text += string.Format(" NOT_STEEP(%1)", RZ_Util.RoundToInt(m_fMaxSlope));
		if (Has(RZ_EPlaceFlag.HOUSES))
			text += string.Format(" HOUSES(%1/%2 m)", m_iHouses, RZ_Util.RoundToInt(m_fHouseRange));
		if (Has(RZ_EPlaceFlag.BUILDING))
			text += string.Format(" BUILDING(%1/%2 m)", m_sBuilding, RZ_Util.RoundToInt(m_fBuildingRange));
		if (Has(RZ_EPlaceFlag.ROAD))
			text += string.Format(" ROAD(%1 m)", RZ_Util.RoundToInt(m_fRoadRange));
		if (Has(RZ_EPlaceFlag.WATER_SOURCE))
			text += string.Format(" WATER_SOURCE(%1 m)", RZ_Util.RoundToInt(m_fWaterRange));
		if (Has(RZ_EPlaceFlag.OPEN_GROUND))
			text += string.Format(" OPEN_GROUND(%1 m)", RZ_Util.RoundToInt(m_fOpenRange));
		if (Has(RZ_EPlaceFlag.TREES))
			text += string.Format(" TREES(%1/%2 m)", m_iTrees, RZ_Util.RoundToInt(m_fTreeRange));
		if (Has(RZ_EPlaceFlag.IN_TOWN))
			text += string.Format(" IN_TOWN(+%1 m)", RZ_Util.RoundToInt(m_fTownReach));
		if (Has(RZ_EPlaceFlag.OUTSIDE_TOWN))
			text += " OUTSIDE_TOWN";
		if (Has(RZ_EPlaceFlag.NEAR_FACTION))
			text += string.Format(" NEAR_FACTION(%1/%2 m)", m_iFaction, RZ_Util.RoundToInt(m_fFactionRange));
		if (Has(RZ_EPlaceFlag.AWAY_FROM_FACTION))
			text += string.Format(" AWAY_FROM_FACTION(%1/%2 m)", m_iFaction, RZ_Util.RoundToInt(m_fFactionRange));
		if (Has(RZ_EPlaceFlag.NOT_LOCKED))
			text += " NOT_LOCKED";
		if (Has(RZ_EPlaceFlag.HIGH_GROUND))
			text += string.Format(" HIGH_GROUND(+%1 m)", RZ_Util.RoundToInt(m_fRise));

		foreach (RZ_PlaceThing t : m_aThings)
		{
			text += "; " + t.Text();
		}

		return text.Trim();
	}
}

//------------------------------------------------------------------------------------------------
//! The place checks themselves, one engine query each. The pickers and the audit call the same ones.
class RZ_PlaceCheck
{
	protected static const float SLOPE_STEP = 3;		// the slope is read this far each side
	protected static const float HOUSE_SIZE = 8;		// a house's box this much further still counts
	protected static const int QUERY_MAX = 160;
	static const int ORCHARD_TREES = 3;				// fruit trees that make an orchard

	// A building whose prefab says one of these isn't a house, whatever its folder
	protected static ref array<string> s_aNotHouse;

	//------------------------------------------------------------------------------------------------
	//! Does `at` meet `need`'s place flags? `town`: the town its words name (-1: the nearest);
	//! `building`: a kind that replaces the need's (a picker that names its building, "the chemist's").
	//! `why` lists every failure ("0 of 1 houses within 15 m; slope 34 deg"). Things: RZ_PlaceThings.
	static bool Check(vector at, RZ_PlaceNeed need, int town, string building, out string why)
	{
		why = "";
		if (!need)
			return true;

		vector ground = RZ_Util.GroundPos(at);
		if (need.Has(RZ_EPlaceFlag.DRY) && IsWet(ground))
			why = Add(why, "in water");

		if (need.Has(RZ_EPlaceFlag.NOT_AT_LINE) && RZ_Perimeter.IsNearLineAt(ground))
			why = Add(why, "at the line");

		if (need.Has(RZ_EPlaceFlag.MAINLAND) && !RZ_Mainland.OnMainland(ground))
			why = Add(why, "on an island");

		if (need.Has(RZ_EPlaceFlag.NOT_STEEP))
		{
			float slope = Slope(ground);
			if (slope > need.m_fMaxSlope)
				why = Add(why, string.Format("slope %1 deg", RZ_Util.RoundToInt(slope)));
		}

		if (need.Has(RZ_EPlaceFlag.HOUSES))
		{
			int houses = CountHouses(ground, need.m_fHouseRange, need.m_iHouses);
			if (houses < need.m_iHouses)
				why = Add(why, string.Format("%1 of %2 houses within %3 m", houses, need.m_iHouses, RZ_Util.RoundToInt(need.m_fHouseRange)));
		}

		if (need.Has(RZ_EPlaceFlag.BUILDING))
		{
			string kind = need.m_sBuilding;
			if (!building.IsEmpty())
				kind = building;

			if (!KindNear(ground, kind, need.m_fBuildingRange))
				why = Add(why, string.Format("no %1 within %2 m", kind, RZ_Util.RoundToInt(need.m_fBuildingRange)));
		}

		if (need.Has(RZ_EPlaceFlag.ROAD) && RoadDistance(ground, need.m_fRoadRange) > need.m_fRoadRange)
			why = Add(why, string.Format("no road within %1 m", RZ_Util.RoundToInt(need.m_fRoadRange)));

		if (need.Has(RZ_EPlaceFlag.WATER_SOURCE) && !WaterSourceNear(ground, need.m_fWaterRange))
			why = Add(why, string.Format("no well, pump or water tower within %1 m", RZ_Util.RoundToInt(need.m_fWaterRange)));

		if (need.Has(RZ_EPlaceFlag.OPEN_GROUND))
		{
			if (AnyBuilding(ground, need.m_fOpenRange))
				why = Add(why, string.Format("a building within %1 m", RZ_Util.RoundToInt(need.m_fOpenRange)));

			int openTrees = CountTrees(ground, need.m_fOpenRange, need.m_iMaxTrees + 1);
			if (openTrees > need.m_iMaxTrees)
				why = Add(why, string.Format("%1 trees within %2 m", openTrees, RZ_Util.RoundToInt(need.m_fOpenRange)));
		}

		if (need.Has(RZ_EPlaceFlag.TREES))
		{
			int trees = CountTrees(ground, need.m_fTreeRange, need.m_iTrees);
			if (trees < need.m_iTrees)
				why = Add(why, string.Format("%1 of %2 trees within %3 m", trees, need.m_iTrees, RZ_Util.RoundToInt(need.m_fTreeRange)));
		}

		if (need.Has(RZ_EPlaceFlag.IN_TOWN) || need.Has(RZ_EPlaceFlag.OUTSIDE_TOWN))
			why = Add(why, TownWhy(ground, need, town));

		if (need.Has(RZ_EPlaceFlag.NEAR_FACTION) && !FactionNear(ground, need.m_iFaction, need.m_fFactionRange))
			why = Add(why, string.Format("no holding or post of faction %1 within %2 m", need.m_iFaction, RZ_Util.RoundToInt(need.m_fFactionRange)));

		if (need.Has(RZ_EPlaceFlag.AWAY_FROM_FACTION) && FactionNear(ground, need.m_iFaction, need.m_fFactionRange))
			why = Add(why, string.Format("a holding or post of faction %1 within %2 m", need.m_iFaction, RZ_Util.RoundToInt(need.m_fFactionRange)));

		if (need.Has(RZ_EPlaceFlag.NOT_LOCKED) && RZ_LockedSpots.IsBlocked(ground))
			why = Add(why, "behind a lock");

		if (need.Has(RZ_EPlaceFlag.HIGH_GROUND))
		{
			float rise = Rise(ground);
			if (rise < need.m_fRise)
				why = Add(why, string.Format("only %1 m above the ground round it", RZ_Util.RoundToInt(rise)));
		}

		return why.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! Check with no town or building given.
	static bool Fits(vector at, RZ_PlaceNeed need)
	{
		string why;
		return Check(at, need, -1, "", why);
	}

	//------------------------------------------------------------------------------------------------
	static string Add(string list, string item)
	{
		if (item.IsEmpty())
			return list;

		if (list.IsEmpty())
			return item;

		return list + "; " + item;
	}

	//------------------------------------------------------------------------------------------------
	//! The sea (by height), a lake, a pond or a river (the engine's water surfaces).
	static bool IsWet(vector ground)
	{
		if (RZ_Util.IsWater(ground))
			return true;

		float reach;
		EWaterSurfaceType wet = RZ_Land.WaterAt(ground, reach);
		return wet == EWaterSurfaceType.WST_POND || wet == EWaterSurfaceType.WST_RIVER;
	}

	//------------------------------------------------------------------------------------------------
	//! The steeper of the two slopes across the point, in degrees.
	static float Slope(vector ground)
	{
		BaseWorld world = GetGame().GetWorld();
		float x = ground[0];
		float z = ground[2];
		float dx = world.GetSurfaceY(x + SLOPE_STEP, z) - world.GetSurfaceY(x - SLOPE_STEP, z);
		float dz = world.GetSurfaceY(x, z + SLOPE_STEP) - world.GetSurfaceY(x, z - SLOPE_STEP);
		float rise = Math.Max(Math.AbsFloat(dx), Math.AbsFloat(dz));
		return Math.Atan2(rise, SLOPE_STEP * 2) * Math.RAD2DEG;
	}

	//------------------------------------------------------------------------------------------------
	//! How far the point stands above the mean of eight points 100 m round it.
	static float Rise(vector ground)
	{
		BaseWorld world = GetGame().GetWorld();
		float sum = 0;
		for (int i = 0; i < 8; i++)
		{
			vector p = RZ_Compass.PointAt(ground, i * 45, 100);
			sum += world.GetSurfaceY(p[0], p[2]);
		}

		return ground[1] - sum / 8;
	}

	//------------------------------------------------------------------------------------------------
	//! A building (RZ_BuildingQuery's) that people lived or worked in: its prefab names a house, a
	//! villa, a cottage, a flat, a pub, a shop, a hotel or an office, and it isn't a shed, a garage,
	//! a ruin or a military or industrial block.
	static bool IsHouse(IEntity b)
	{
		if (!b)
			return false;

		string name = RZ_ItemHelper.LowerName(b);
		if (!SCR_DestructibleBuildingEntity.Cast(b) && !name.Contains("/buildings/"))
			return false;

		if (!s_aNotHouse)
			s_aNotHouse = {"/military/", "/industrial/", "ruin", "shed", "garage", "barn", "probe", "doghouse", "henhouse", "greenhouse", "cowshed", "booth", "latrine", "bunker", "hangar", "church", "chapel", "tower"};

		foreach (string word : s_aNotHouse)
		{
			if (name.Contains(word))
				return false;
		}

		return name.Contains("house") || name.Contains("villa") || name.Contains("cottage") || name.Contains("apartment") || name.Contains("pub") || name.Contains("shop") || name.Contains("hotel") || name.Contains("office");
	}

	//------------------------------------------------------------------------------------------------
	//! Houses within `range` of `at` (their box reaching it); stops at `enough` (0: counts them all).
	static int CountHouses(vector at, float range, int enough)
	{
		RZ_HouseQuery q = new RZ_HouseQuery();
		q.m_iMax = enough;
		GetGame().GetWorld().QueryEntitiesBySphere(at, range, q.OnEntity, null, EQueryEntitiesFlags.STATIC);
		return q.m_aFound.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! The houses whose origin is `minDist` to `range` from `at`. The houses themselves are queried,
	//! so a street of walls, sheds and fences doesn't fill the list first.
	static void Houses(vector at, float minDist, float range, notnull array<IEntity> result)
	{
		RZ_HouseQuery q = new RZ_HouseQuery();
		q.m_iMax = QUERY_MAX;
		GetGame().GetWorld().QueryEntitiesBySphere(at, range, q.OnEntity, null, EQueryEntitiesFlags.STATIC);
		foreach (IEntity b : q.m_aFound)
		{
			float d = RZ_Util.DistXZ(at, b.GetOrigin());
			if (d >= minDist && d <= range)
				result.Insert(b);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Any building at all (not a ruin) within range.
	static bool AnyBuilding(vector at, float range)
	{
		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(at, range, buildings, QUERY_MAX);
		foreach (IEntity b : buildings)
		{
			string name = RZ_ItemHelper.LowerName(b);
			if (!name.Contains("ruin"))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The door slots' kinds, read from a building's prefab (RZ_DoorSlots.KindOf).
	static bool IsDoorKind(string kind)
	{
		return kind == "clinic" || kind == "aid" || kind == "shop" || kind == "garage" || kind == "farm" || kind == "public";
	}

	//------------------------------------------------------------------------------------------------
	//! The words are true of this building (docs/PLACES.md, "A place is what its words say"): the one
	//! rule by prefab name for a kind a task's words name. "clinic" (the chemist's) is a civilian
	//! pharmacy, surgery, clinic or hospital, never an army hut or a tent; "aid" (the old aid post) is
	//! the army's medical building; "farm" a farmhouse, barn or stable, not a shed; "public" (the hall)
	//! a town hall, a school, a church or an office, not a fuel or rail station; "house" a house
	//! (IsHouse); the rest by the landmark finder's words. The pickers and the place audit both ask it.
	static bool WordsTrue(string kind, IEntity b)
	{
		if (!b)
			return false;

		if (kind.IsEmpty() || kind == "any")
			return true;

		if (kind == "house")
			return IsHouse(b);

		string name = RZ_ItemHelper.LowerName(b);
		switch (kind)
		{
			case "clinic":
				return HasWord(name, {"pharmacy", "drugstore", "apothec", "clinic", "hospital", "surgery", "medical"}) && !HasWord(name, {"/military/", "tent", "barracks", "camp", "shed", "veterin"});
			case "aid":
				return HasWord(name, {"medical", "hospital"}) && name.Contains("/military/") && !HasWord(name, {"tent", "/camps/"});
			case "farm":
				return HasWord(name, {"farm", "barn", "stable", "cowshed", "pigsty"}) && !name.Contains("shed_");
			case "garage":
				return HasWord(name, {"garage", "workshop", "autoservice", "carservice", "car_service", "mechanic", "repair"});
			case "shop":
				return HasWord(name, {"shop", "store", "market", "kiosk", "grocery", "pub", "bar_", "cafe", "restaurant"}) && !HasWord(name, {"/military/", "workshop", "/industrial/"});
			case "public":
				return HasWord(name, {"townhall", "town_hall", "school", "church", "chapel", "office", "hall", "culture"}) && !HasWord(name, {"fuel", "gas", "/military/"});
		}

		return RZ_LandmarkFinder.Matches(kind, name);
	}

	//------------------------------------------------------------------------------------------------
	//! A building within range that the words naming `kind` are true of (WordsTrue). False: `seen` is
	//! the nearest building there, by name (the place audit's fail line says what stands there).
	static bool WordsTrueNear(vector at, string kind, float range, out string seen)
	{
		seen = "";
		if (kind.IsEmpty() || kind == "any" || kind == "orchard" || kind == "well" || kind == "watertower")
			return true;

		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(at, range + HOUSE_SIZE, buildings, QUERY_MAX);
		float best = float.MAX;
		foreach (IEntity b : buildings)
		{
			if (WordsTrue(kind, b))
				return true;

			float d = RZ_Util.DistXZ(at, b.GetOrigin());
			if (d < best)
			{
				best = d;
				seen = RZ_Util.PrettyPrefabName(RZ_Util.GetPrefabName(b));
			}
		}

		// A landmark kind the finder knows by other things than buildings (a fuel pump, a radio mast)
		if (!IsDoorKind(kind) && kind != "house")
		{
			array<string> kinds = {kind};
			if (RZ_LandmarkFinder.FindNearest(at, range, kinds) != null)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool HasWord(string name, array<string> words)
	{
		foreach (string w : words)
		{
			if (name.Contains(w))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A building or landmark of `kind` within range: "any" any building, "house" a house, "orchard"
	//! three fruit trees; the door slots' kinds (clinic, shop, garage, farm, public) by building;
	//! anything else by the landmark finder's keywords (fuel, well, watertower, pumphouse, church,
	//! police, radio, school, military...).
	static bool KindNear(vector at, string kind, float range)
	{
		if (kind.IsEmpty() || kind == "any")
			return AnyBuilding(at, range);

		if (kind == "house")
			return CountHouses(at, range, 1) > 0;

		if (kind == "orchard")
			return CountFruitTrees(at, range, ORCHARD_TREES, null) >= ORCHARD_TREES;

		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(at, range + HOUSE_SIZE, buildings, QUERY_MAX);
		array<string> kinds = {kind};
		foreach (IEntity b : buildings)
		{
			if (!RZ_OpeningRoll.KindOf(b, kinds).IsEmpty())
				return true;
		}

		if (IsDoorKind(kind))
			return false;

		return RZ_LandmarkFinder.FindNearest(at, range, kinds) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! A well, a hand pump or a water tower within range.
	static bool WaterSourceNear(vector at, float range)
	{
		array<string> kinds = {"well", "watertower"};
		return RZ_LandmarkFinder.FindNearest(at, range, kinds) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! The distance to the nearest road's centre line, if one is within `range` (else more).
	static float RoadDistance(vector at, float range)
	{
		float best = range + 1;
		RoadNetworkManager roads = RZ_Perimeter.GetRoadNetwork();
		if (!roads)
			return best;

		vector mins = Vector(at[0] - range, -1000, at[2] - range);
		vector maxs = Vector(at[0] + range, 10000, at[2] + range);
		array<BaseRoad> found = {};
		roads.GetRoadsInAABB(mins, maxs, found);
		foreach (BaseRoad road : found)
		{
			if (!road)
				continue;

			array<vector> points = {};
			road.GetPoints(points);
			for (int i = 0; i + 1 < points.Count(); i++)
			{
				best = Math.Min(best, RZ_Util.DistToSegmentXZ(at, points[i], points[i + 1]));
			}

			if (points.Count() == 1)
				best = Math.Min(best, RZ_Util.DistXZ(at, points[0]));
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Trees (not bushes) within range; stops at `enough` (0: counts them all).
	static int CountTrees(vector at, float range, int enough)
	{
		RZ_TreeCount q = new RZ_TreeCount();
		q.m_iEnough = enough;
		GetGame().GetWorld().QueryEntitiesBySphere(at, range, q.OnEntity, null, EQueryEntitiesFlags.STATIC);
		return q.m_iCount;
	}

	//------------------------------------------------------------------------------------------------
	//! Fruit trees (apple, plum, pear) within range; stops at `enough`; `where` gets where they stand.
	static int CountFruitTrees(vector at, float range, int enough, array<vector> where)
	{
		RZ_TreeCount q = new RZ_TreeCount();
		q.m_iEnough = enough;
		q.m_bFruit = true;
		q.m_aAt = where;
		GetGame().GetWorld().QueryEntitiesBySphere(at, range, q.OnEntity, null, EQueryEntitiesFlags.STATIC);
		return q.m_iCount;
	}

	//------------------------------------------------------------------------------------------------
	protected static string TownWhy(vector at, RZ_PlaceNeed need, int town)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return "";

		if (need.Has(RZ_EPlaceFlag.OUTSIDE_TOWN))
		{
			foreach (RZ_Settlement s : gm.GetSettlements())
			{
				if (RZ_Util.DistXZ(s.m_vCenter, at) < s.m_fRadius)
					return "in " + s.m_sName;
			}
		}

		if (!need.Has(RZ_EPlaceFlag.IN_TOWN))
			return "";

		int idx = town;
		if (idx < 0)
			idx = gm.FindNearestSettlement(at);

		RZ_Settlement named = gm.GetSettlement(idx);
		if (!named)
			return "no town";

		float d = RZ_Util.DistXZ(named.m_vCenter, at);
		if (d > named.m_fRadius + need.m_fTownReach)
			return string.Format("%1 m from %2 (radius %3)", RZ_Util.RoundToInt(d), named.m_sName, RZ_Util.RoundToInt(named.m_fRadius));

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected static bool FactionNear(vector at, int faction, float range)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return false;

		RZ_Territory territory = gm.GetTerritory();
		if (territory && territory.FindHolding(faction, at, range))
			return true;

		RZ_Checkpoints posts = gm.GetCheckpoints();
		if (!posts)
			return false;

		array<RZ_RoadPost> list = {};
		posts.PostsNear(faction, at, range, list);
		return !list.IsEmpty();
	}
}

//------------------------------------------------------------------------------------------------
//! A sphere query that keeps houses only (RZ_PlaceCheck.IsHouse), up to m_iMax (0: all).
class RZ_HouseQuery
{
	int m_iMax;
	ref array<IEntity> m_aFound = {};

	//------------------------------------------------------------------------------------------------
	bool OnEntity(IEntity e)
	{
		if (m_iMax > 0 && m_aFound.Count() >= m_iMax)
			return false;

		if (RZ_PlaceCheck.IsHouse(e))
			m_aFound.Insert(e);

		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! A sphere query that counts trees (BaseTree named /tree; bushes and plants are BaseTree too).
class RZ_TreeCount
{
	int m_iCount;
	int m_iEnough;
	bool m_bFruit;					// only fruit trees (an orchard)
	ref array<vector> m_aAt;		// where each counted tree stands (null: not kept)

	//------------------------------------------------------------------------------------------------
	bool OnEntity(IEntity e)
	{
		if (m_iEnough > 0 && m_iCount >= m_iEnough)
			return false;

		if (!BaseTree.Cast(e))
			return true;

		string name = RZ_Util.GetPrefabName(e);
		name.ToLower();
		if (!name.Contains("/tree"))
			return true;

		if (m_bFruit && !name.Contains("malus") && !name.Contains("prunus") && !name.Contains("pyrus"))
			return true;

		m_iCount++;
		if (m_aAt)
			m_aAt.Insert(e.GetOrigin());

		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! The things a task's words name: their prefab slots, and whether they stand at the place.
class RZ_PlaceThings
{
	//------------------------------------------------------------------------------------------------
	//! The prefab a thing is made from, by its slot key; "" when the slot is empty.
	static ResourceName Slot(string key)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return "";

		switch (key)
		{
			case "sample": return gm.GetSamplePrefab();
			case "antidote": return gm.GetAntidotePrefab();
			case "remains": return RZ_OpeningTasks.RemainsPrefab(gm);
			case "runner": return gm.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);
			case "case": return RZ_OpeningCase.CasePrefab(gm);
			case "spawnpoint": return gm.GetSpawnPointPrefab();
			case "contact": return ContactPrefab(gm);
			case "key":
			{
				RZ_Interactions hub = RZ_Interactions.Get();
				if (hub && hub.GetSearch())
					return hub.GetSearch().GetKeyPrefab();

				return "";
			}
			case "prop.effects": return RZ_ErrandSettings.Get().Props().For(RZ_EErrandProp.EFFECTS);
			case "prop.crate": return RZ_ErrandSettings.Get().Props().For(RZ_EErrandProp.CRATE);
			case "prop.papers": return RZ_ErrandSettings.Get().Props().For(RZ_EErrandProp.PAPERS);
			case "prop.parts": return RZ_ErrandSettings.Get().Props().For(RZ_EErrandProp.PARTS);
			case "prop.car": return RZ_ErrandSettings.Get().Props().For(RZ_EErrandProp.CAR);
			case "work.site": return RZ_SharedSettings.Get().WorkSites().m_sSitePrefab;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Where the slot is set in Workbench.
	static string SlotLabel(string key)
	{
		switch (key)
		{
			case "sample": return "Cure > Sample prefab";
			case "antidote": return "Cure > Antidote prefab";
			case "remains": return "Zombies > Special prefabs (one that doesn't burst)";
			case "runner": return "Puzzles > Civilian";
			case "case": return "Guidance > Cold case prefab";
			case "spawnpoint": return "World > Spawn point prefab";
			case "contact": return "Guidance > Contact prefab, else Puzzles > Civilian";
			case "key": return "Interactions > Search > Key prefab";
			case "prop.effects": return "Missions > Errands > Props > Effects";
			case "prop.crate": return "Missions > Errands > Props > Crate";
			case "prop.papers": return "Missions > Errands > Props > Papers";
			case "prop.parts": return "Missions > Errands > Props > Parts";
			case "prop.car": return "Guidance > Car wreck prefab";
			case "work.site": return "Shared > Work sites > Site prefab";
		}

		return key;
	}

	//------------------------------------------------------------------------------------------------
	//! The errand prop slot for a way's prop.
	static string PropSlot(int prop)
	{
		switch (prop)
		{
			case RZ_EErrandProp.EFFECTS: return "prop.effects";
			case RZ_EErrandProp.CRATE: return "prop.crate";
			case RZ_EErrandProp.PAPERS: return "prop.papers";
			case RZ_EErrandProp.PARTS: return "prop.parts";
			case RZ_EErrandProp.CAR: return "prop.car";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	static ResourceName ContactPrefab(RZ_GameModeComponent gm)
	{
		RZ_Guidance guide = gm.GetGuidance();
		if (guide && guide.GetSettings() && !guide.GetSettings().m_sContactPrefab.IsEmpty())
			return guide.GetSettings().m_sContactPrefab;

		return gm.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);
	}

	//------------------------------------------------------------------------------------------------
	//! Are the slots its things are made from set? `why`: the missing ones with no fallback (a
	//! failure); `fallbacks`: the missing ones whose fallback the game takes instead.
	static bool SlotsSet(RZ_PlaceNeed need, out string why, out string fallbacks)
	{
		why = "";
		fallbacks = "";
		if (!need)
			return true;

		foreach (RZ_PlaceThing t : need.m_aThings)
		{
			if (t.m_sSlot.IsEmpty() || !Slot(t.m_sSlot).IsEmpty())
				continue;

			string missing = string.Format("%1 (%2 is empty)", t.m_sName, SlotLabel(t.m_sSlot));
			if (t.m_sFallback.IsEmpty())
				why = RZ_PlaceCheck.Add(why, "missing " + missing);
			else
				fallbacks = RZ_PlaceCheck.Add(fallbacks, missing + ": " + t.m_sFallback);
		}

		return why.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! Do its things stand at the place now (once the step has started)? `why` names the missing.
	static bool Present(vector at, RZ_PlaceNeed need, out string why)
	{
		why = "";
		if (!need)
			return true;

		foreach (RZ_PlaceThing t : need.m_aThings)
		{
			if (t.m_eKind != RZ_EThingKind.SLOT && !Near(at, t))
				why = RZ_PlaceCheck.Add(why, string.Format("missing %1 (none within %2 m)", t.m_sName, RZ_Util.RoundToInt(t.m_fRange)));
		}

		return why.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! This thing within its range of `at`: an item or prop made from its slot, a living person
	//! (not a player), or a dead body.
	static bool Near(vector at, RZ_PlaceThing t)
	{
		RZ_ThingQuery q = new RZ_ThingQuery();
		q.m_eKind = t.m_eKind;
		if (t.m_eKind == RZ_EThingKind.ITEM || t.m_eKind == RZ_EThingKind.PROP)
		{
			ResourceName prefab = Slot(t.m_sSlot);
			if (prefab.IsEmpty())
				return false;

			string path = prefab.GetPath();
			path.ToLower();
			q.m_sPath = path;
		}

		GetGame().GetWorld().QueryEntitiesBySphere(at, Math.Max(1, t.m_fRange), q.OnEntity, null, EQueryEntitiesFlags.DYNAMIC | EQueryEntitiesFlags.STATIC);
		return q.m_bFound;
	}
}

//------------------------------------------------------------------------------------------------
//! A sphere query for one thing (RZ_PlaceThings.Near).
class RZ_ThingQuery
{
	int m_eKind;
	string m_sPath;		// ITEM, PROP: the slot's prefab path, lower case
	bool m_bFound;

	//------------------------------------------------------------------------------------------------
	bool OnEntity(IEntity e)
	{
		if (m_bFound || !e)
			return false;

		if (m_eKind == RZ_EThingKind.NPC || m_eKind == RZ_EThingKind.REMAINS)
		{
			if (!ChimeraCharacter.Cast(e) || GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(e) > 0)
				return true;

			bool alive = RZ_Util.IsCharacterAlive(e);
			if ((m_eKind == RZ_EThingKind.NPC && alive) || (m_eKind == RZ_EThingKind.REMAINS && !alive))
				m_bFound = true;

			return !m_bFound;
		}

		string name = RZ_Util.GetPrefabName(e);
		name.ToLower();
		if (!m_sPath.IsEmpty() && name.Contains(m_sPath))
			m_bFound = true;

		return !m_bFound;
	}
}

//------------------------------------------------------------------------------------------------
//! Places that meet a need, drawn from what the need asks for: its building, a house, a well, a
//! road, an orchard, high ground, else open ground. The pickers use it to try their candidates.
class RZ_PlaceFind
{
	protected static const int RANDOM_TRIES = 8;
	protected static const int MAX_CANDIDATES = 14;
	protected static const float CACHE_SECONDS = 600;	// a found place is kept this long (by need and where it was asked)
	protected static const int CACHE_MAX = 200;
	protected static const float SIDE_OUT = 3;			// a building's spot: this far out of its side facing the asker
	protected static ref map<string, vector> s_mCache;
	protected static ref map<string, float> s_mCacheAt;
	protected static string s_sLastWhy;

	//------------------------------------------------------------------------------------------------
	//! A place `minR`-`maxR` from `center` that meets `need` (`town`, `building` as RZ_PlaceCheck.Check);
	//! zero if no candidate does.
	static vector Near(vector center, float minR, float maxR, RZ_PlaceNeed need, int town = -1, string building = "")
	{
		s_sLastWhy = "";
		if (!need)
			return RZ_Mission.LandPointNear(center, minR, maxR);

		array<vector> candidates = {};
		Candidates(center, minR, maxR, need, building, candidates);
		map<string, int> whys = new map<string, int>();
		foreach (vector c : candidates)
		{
			string why;
			if (RZ_PlaceCheck.Check(c, need, town, building, why))
				return c;

			int n;
			whys.Find(why, n);
			whys.Set(why, n + 1);
		}

		s_sLastWhy = string.Format("%1 candidates", candidates.Count());
		foreach (string w, int k : whys)
		{
			s_sLastWhy += string.Format("; %1 x%2", w, k);
		}

		return vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	//! Why the last Near found nothing: its candidates and why each failed (the audit prints it).
	static string LastWhy()
	{
		return s_sLastWhy;
	}

	//------------------------------------------------------------------------------------------------
	//! Near, kept for CACHE_SECONDS by need, ring and where it was asked (to 25 m): the errands ask
	//! the same giver's places every time a talk menu is built.
	static vector NearKept(vector center, float minR, float maxR, RZ_PlaceNeed need, int town = -1, string building = "")
	{
		if (!need)
			return Near(center, minR, maxR, need, town, building);

		if (!s_mCache)
		{
			s_mCache = new map<string, vector>();
			s_mCacheAt = new map<string, float>();
		}

		string key = string.Format("%1|%2|%3|%4|%5|%6|%7", need.m_sKey, Math.Floor(center[0] / 25), Math.Floor(center[2] / 25), minR, maxR, town, building);
		float now = GetGame().GetWorld().GetWorldTime() / 1000;
		float at;
		if (s_mCacheAt.Find(key, at) && now - at < CACHE_SECONDS)
			return s_mCache.Get(key);

		if (s_mCache.Count() > CACHE_MAX)
		{
			s_mCache.Clear();
			s_mCacheAt.Clear();
		}

		vector found = Near(center, minR, maxR, need, town, building);
		s_mCache.Set(key, found);
		s_mCacheAt.Set(key, now);
		return found;
	}

	//------------------------------------------------------------------------------------------------
	//! The candidates, the need's own kind first (shuffled), then random dry points in the ring.
	static void Candidates(vector center, float minR, float maxR, RZ_PlaceNeed need, string building, notnull array<vector> result)
	{
		string kind = need.m_sBuilding;
		if (!building.IsEmpty())
			kind = building;

		array<vector> own = {};
		if (need.Has(RZ_EPlaceFlag.BUILDING) && !kind.IsEmpty() && kind != "any")
			KindSpots(center, minR, maxR, kind, own);
		else if (need.Has(RZ_EPlaceFlag.WATER_SOURCE))
			LandmarkSpots(center, minR, maxR, {"well", "watertower"}, own);
		else if (need.Has(RZ_EPlaceFlag.HOUSES))
			KindSpots(center, minR, maxR, "house", own);
		else if (need.Has(RZ_EPlaceFlag.ROAD))
			RoadSpots(center, minR, maxR, own);
		else if (need.Has(RZ_EPlaceFlag.HIGH_GROUND))
		{
			for (int h = 0; h < 4; h++)
			{
				own.Insert(RZ_Mission.HighPointNear(center, minR, maxR, 24));
			}
		}

		Shuffle(own);
		for (int i = 0; i < own.Count() && result.Count() < MAX_CANDIDATES; i++)
		{
			result.Insert(own[i]);
		}

		// What the need asks for is the place itself: a random point won't have it
		if (need.Has(RZ_EPlaceFlag.BUILDING) || need.Has(RZ_EPlaceFlag.WATER_SOURCE) || need.Has(RZ_EPlaceFlag.HOUSES) || need.Has(RZ_EPlaceFlag.ROAD))
			return;

		for (int r = 0; r < RANDOM_TRIES; r++)
		{
			result.Insert(RZ_Mission.LandPointNear(center, minR, maxR));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Spots by buildings or landmarks of `kind` in the ring.
	static void KindSpots(vector center, float minR, float maxR, string kind, notnull array<vector> result)
	{
		if (kind == "orchard")
		{
			array<vector> trees = {};
			RZ_PlaceCheck.CountFruitTrees(center, maxR, 0, trees);
			foreach (vector t : trees)
			{
				if (InRing(center, t, minR, maxR))
					result.Insert(RZ_Mission.LandPointNear(t, 1, 4));
			}

			return;
		}

		array<IEntity> buildings = {};
		if (kind == "house")
			RZ_PlaceCheck.Houses(center, minR, maxR, buildings);
		else
			RZ_BuildingQuery.FindBuildings(center, maxR, buildings, 400);

		array<string> kinds = {kind};
		foreach (IEntity b : buildings)
		{
			if (!b || !InRing(center, b.GetOrigin(), minR, maxR) || RZ_LockedSpots.IsLockedBuilding(b))
				continue;

			if (kind == "house" || !RZ_OpeningRoll.KindOf(b, kinds).IsEmpty())
				result.Insert(RZ_Util.GroundPos(RZ_DoorGeometry.SidePoint(b, center, SIDE_OUT)));
		}

		if (kind == "house" || RZ_PlaceCheck.IsDoorKind(kind))
			return;

		LandmarkSpots(center, minR, maxR, kinds, result);
	}

	//------------------------------------------------------------------------------------------------
	static void LandmarkSpots(vector center, float minR, float maxR, array<string> kinds, notnull array<vector> result)
	{
		array<ref RZ_Landmark> found = {};
		RZ_LandmarkFinder.Find(center, maxR, kinds, found);
		foreach (RZ_Landmark l : found)
		{
			if (InRing(center, l.m_vPos, minR, maxR))
				result.Insert(RZ_Mission.LandPointNear(l.m_vPos, 1, 4));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Spots on a road's verge in the ring: 2-4 m off a point on its centre line.
	static void RoadSpots(vector center, float minR, float maxR, notnull array<vector> result)
	{
		array<vector> points = {};
		RZ_Perimeter.RoadPointsNear(center, maxR, points);
		foreach (vector p : points)
		{
			if (InRing(center, p, minR, maxR))
				result.Insert(RZ_Util.GroundPos(RZ_Util.RandomPointInRing(p, 2, 4)));
		}
	}

	//------------------------------------------------------------------------------------------------
	static bool InRing(vector center, vector p, float minR, float maxR)
	{
		float d = RZ_Util.DistXZ(center, p);
		return d >= minR && d <= maxR;
	}

	//------------------------------------------------------------------------------------------------
	static void Shuffle(notnull array<vector> list)
	{
		for (int i = list.Count() - 1; i > 0; i--)
		{
			int j = Math.RandomInt(0, i + 1);
			vector tmp = list[i];
			list[i] = list[j];
			list[j] = tmp;
		}
	}
}

//------------------------------------------------------------------------------------------------
//! Every kind of task's need (the table in docs/PLACES.md), and the live checks.
class RZ_PlaceNeeds
{
	protected static ref map<string, ref RZ_PlaceNeed> s_mNeeds;
	protected static ref array<string> s_aKeys;
	protected static ref map<string, int> s_mChecked;
	protected static ref map<string, int> s_mFailed;

	static const string NOT_OFFERED = "the step isn't offered";

	//------------------------------------------------------------------------------------------------
	protected static RZ_PlaceNeed Add(string key, string words)
	{
		RZ_PlaceNeed n = new RZ_PlaceNeed(key, words);
		s_mNeeds.Set(key, n);
		s_aKeys.Insert(key);
		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Build()
	{
		s_mNeeds = new map<string, ref RZ_PlaceNeed>();
		s_aKeys = {};
		s_mChecked = new map<string, int>();
		s_mFailed = new map<string, int>();

		// Openings (Guidance/): where a life starts, and its chain's steps
		// Houses down the road are the roll's preference (its rank), not the start's words: step 1 needs them
		RZ_PlaceNeed n = Add("opening.start", "outside the door you came out of, someone waiting");
		n.Building("any", 12);
		n.Steep(30);
		n.Thing(RZ_EThingKind.NPC, "the contact", "contact", 15, "nobody stands there: the scene's note points at the nearest work");

		n = Add("opening.favour", "where the contact's favour sends you: the chemist's, the garage, his shed, the school, the shop, the church, the house next door");
		n.Building("house", 15);
		n.Steep(30);

		n = Add("opening.favour.door", "the neighbour's door: a real outside door, its doorstep");
		n.Building("house", 6);
		n.Steep(30);

		n = Add("opening.water", "the well");
		n.Water(8);

		n = Add("opening.pump", "the petrol station");
		n.Building("fuel", 20);

		n = Add("opening.downwind", "the pack's ground");
		n.Steep(35);

		n = Add("opening.treeline", "the tree line where they cross");
		n.Trees(4, 40);
		n.Steep(35);

		n = Add("opening.keys", "the locked house over the way, and its key");
		n.Building("any", 6);
		n.Thing(RZ_EThingKind.SLOT, "the key", "key", 0, NOT_OFFERED);

		n = Add("opening.sample", "what's left of a big one by the road, and the sample by it");
		n.Road(12);
		n.Steep(25);
		n.NotLocked();
		n.Thing(RZ_EThingKind.REMAINS, "the remains", "remains", 4, NOT_OFFERED);
		n.Thing(RZ_EThingKind.ITEM, "the sample", "sample", 4, NOT_OFFERED);

		n = Add("opening.runner", "the runner, fallen by the road, and the case he carried");
		n.Road(15);
		n.Steep(25);
		n.NotLocked();
		n.Thing(RZ_EThingKind.REMAINS, "the runner", "runner", 4, NOT_OFFERED);
		n.Thing(RZ_EThingKind.ITEM, "the case", "case", 4, NOT_OFFERED);

		n = Add("opening.line", "the road above the line, where you watch it with him");
		n.Road(10);
		n.Steep(20);

		n = Add("opening.line.open", "the rise above the line, where you watch it with him (a world with no road network)");
		n.Steep(20);

		n = Add("opening.runner.yard", "the runner, fallen in a yard on the edge of town, and the case he carried");
		n.Houses(1, 25);
		n.Steep(25);
		n.NotLocked();
		n.Thing(RZ_EThingKind.REMAINS, "the runner", "runner", 4, NOT_OFFERED);
		n.Thing(RZ_EThingKind.ITEM, "the case", "case", 4, NOT_OFFERED);

		n = Add("opening.sample.yard", "what's left of a big one in a yard over the way, and the sample by it");
		n.Houses(1, 20);
		n.Steep(25);
		n.NotLocked();
		n.Thing(RZ_EThingKind.REMAINS, "the remains", "remains", 4, NOT_OFFERED);
		n.Thing(RZ_EThingKind.ITEM, "the sample", "sample", 4, NOT_OFFERED);

		n = Add("opening.gate", "the gate");
		n.Steep(30);

		n = Add("opening.debt", "the house with the owing in it");
		n.Houses(1, 15);

		n = Add("opening.toll", "the Red Hand's toll or gate");
		n.NearFaction(RZ_EFaction.BANDITS, 120);

		n = Add("opening.swap", "the wanderer on the road");

		n = Add("spawn.point", "a spawn point named after its town");
		n.InTown(0);
		n.Houses(1, 60);
		n.Steep(30);
		n.Thing(RZ_EThingKind.SLOT, "the spawn point", "spawnpoint", 0, "none spawn: the map's own spawn points");

		// Errands (Missions/Errands/): a way's step (RZ_ErrandWay.m_sNeed; else by its place kind)
		n = Add("errand.here", "{here} or {house}: the giver's place");

		n = Add("errand.out", "a direction and a distance out from the giver");

		n = Add("errand.subject", "the card's own place");

		n = Add("errand.town", "in {place}, a town by name");
		n.InTown(0);
		n.Houses(2, 60);

		n = Add("errand.gardens", "the gardens and sheds at {place}");
		n.InTown(100);
		n.Houses(2, 60);

		n = Add("errand.well", "the well in {place}");
		n.InTown(150);
		n.Water(10);

		n = Add("errand.pumphouse", "the pump house at {place}");
		n.InTown(300);
		n.Building("pumphouse", 20);

		n = Add("errand.car", "a car out on the road, {place}");
		n.Road(25);

		n = Add("errand.sensor", "a sensor out in the open, {place}");
		n.Steep(30);

		n = Add("errand.ridge", "up {place}: the ridge, for the weather");
		n.High(8);

		n = Add("errand.farmhouse", "the farmhouse {place}");
		n.Building("farm", 25);

		n = Add("errand.orchard", "the orchard at {place}");
		n.InTown(300);
		n.Building("orchard", 40);

		n = Add("errand.police", "the police station in {place}");
		n.InTown(300);
		n.Building("police", 30);

		n = Add("errand.school", "the school in {place}");
		n.InTown(300);
		n.Building("school", 30);

		n = Add("errand.hall", "the hall in {place}");
		n.InTown(300);
		n.Building("public", 30);

		n = Add("errand.shop", "the shops in {place}");
		n.InTown(300);
		n.Building("shop", 30);

		// A way's prop (Missions > Errands > Props): the thing its words name at the place
		n = Add("errand.prop.effects", "a dead man's things at the place");
		n.Thing(RZ_EThingKind.PROP, "the effects", "prop.effects", 6, "words only");

		n = Add("errand.prop.crate", "a crate or a box at the place");
		n.Thing(RZ_EThingKind.PROP, "the crate", "prop.crate", 6, "words only");

		n = Add("errand.prop.papers", "papers or a book at the place");
		n.Thing(RZ_EThingKind.PROP, "the papers", "prop.papers", 6, "words only");

		n = Add("errand.prop.parts", "a toolbox or a machine part at the place");
		n.Thing(RZ_EThingKind.PROP, "the parts", "prop.parts", 6, "words only");

		n = Add("errand.prop.car", "the car at the verge");
		n.Thing(RZ_EThingKind.PROP, "the car", "prop.car", 8, "words only");

		// What stands all season (the audit lists any on an island): holdings, safehouses, the lab,
		// the Long Table's farm, quartermasters, wanderers and the towns themselves
		n = Add("world.place", "a holding, a safehouse, the lab, the Long Table, a wanderer or a town");
		n.m_iFlags = RZ_EPlaceFlag.DRY | RZ_EPlaceFlag.MAINLAND;	// a town by the edge is by the line: that's the map

		// Work sites (Missions/Blocks/RZ_WorkSites.c): the spot beside the thing a step says you work at
		n = Add("work.site", "beside the thing you work at: its before look and its prompt");
		n.Steep(25);
		n.NotLocked();
		n.Thing(RZ_EThingKind.SLOT, "the work site's prompt", "work.site", 0, "the step is done by staying there");

		// Contracts and world events, questline stages and arcs (Missions/): checked when one starts
		n = Add("mission", "at {town}: the town its brief names");
		n.InTown(1500);

		// The perimeter's own jobs (The Blind Spot, The Crossing, Shot at the Line, the Last Pilot's
		// wreck): at the line by design (docs/MISSION_RULES.md, rule 8), still dry and on the mainland
		n = Add("mission.line", "at the line, by design: the perimeter's own jobs");
		n.m_iFlags = RZ_EPlaceFlag.DRY | RZ_EPlaceFlag.MAINLAND;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_PlaceNeed Get(string key)
	{
		if (!s_mNeeds)
			Build();

		RZ_PlaceNeed n;
		s_mNeeds.Find(key, n);
		return n;
	}

	//------------------------------------------------------------------------------------------------
	static void Keys(notnull array<string> result)
	{
		if (!s_mNeeds)
			Build();

		result.InsertAll(s_aKeys);
	}

	//------------------------------------------------------------------------------------------------
	//! The live check of a place a picker settled on: its place flags and its things' slots. A
	//! failure is logged as a warning ("Place check: ..."). `text`: what the player reads; `town`,
	//! `building`: as RZ_PlaceCheck.Check.
	static bool Verify(string key, vector at, string text, int town = -1, string building = "")
	{
		RZ_PlaceNeed n = Get(key);
		if (!n)
		{
			RZ_Log.Warn("Place check: no need called " + key);
			return true;
		}

		string why;
		RZ_PlaceCheck.Check(at, n, town, building, why);
		string slots;
		string fallbacks;
		RZ_PlaceThings.SlotsSet(n, slots, fallbacks);
		return Count(key, at, text, RZ_PlaceCheck.Add(why, slots));
	}

	//------------------------------------------------------------------------------------------------
	//! The live check once a step has started: the things its words name stand at the place.
	static bool VerifyThings(string key, vector at, string text)
	{
		RZ_PlaceNeed n = Get(key);
		if (!n)
			return true;

		string why;
		RZ_PlaceThings.Present(at, n, why);
		return Count(key + " things", at, text, why);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Count(string key, vector at, string text, string why)
	{
		if (!s_mChecked)
			Build();

		int checked;
		s_mChecked.Find(key, checked);
		s_mChecked.Set(key, checked + 1);
		if (why.IsEmpty())
			return true;

		int failed;
		s_mFailed.Find(key, failed);
		s_mFailed.Set(key, failed + 1);
		RZ_Log.Warn(string.Format("Place check: %1 at %2 (\"%3\") fails: %4", key, RZ_SettlementScanner.GridReference(at), text, why));
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! "opening.favour 12/12, mission 3/4": the live checks so far (passed / checked, by need).
	static string LiveText()
	{
		if (!s_mChecked || s_mChecked.IsEmpty())
			return "no places checked yet";

		string text = "";
		foreach (string key, int checked : s_mChecked)
		{
			int failed;
			s_mFailed.Find(key, failed);
			if (!text.IsEmpty())
				text += ", ";

			text += string.Format("%1 %2/%3", key, checked - failed, checked);
		}

		return text;
	}
}
