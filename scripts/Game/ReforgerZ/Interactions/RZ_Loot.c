//! Loose loot (docs/DESIGN.md, "Loot in houses"): things lie in houses, on furniture and on the
//! floor, and are picked up with vanilla's Pick up. There are no search prompts in buildings.
//!
//! - Data first. Every building the look-around sees near a player (RZ_Interactions) gets a
//!   small record: its type, when it was rolled and emptied, and its items as data (prefab,
//!   anchor, key, note). Nothing is spawned until a player is within Spawn radius.
//! - Rolled on the first visit with the search rules (RZ_Search.PickHouseLootAt: the type's table,
//!   and the built-in vanilla items; Slot chance x the town's loot level, the first Sure items
//!   always there (one in a picked-over town); the rare and electronics chances; no prefab twice
//!   unless it stacks), plus one treatment roll
//!   (RZ_Infection.PickMedFind), sometimes the town's fixable car's keys (RZ_Cars), and sometimes
//!   a readable (a note, with a key beside it now and then). A locked place (a seeded lock on real
//!   outside doors) gets one more item and better finds. A stash someone wrote about adds two.
//!   A building emptied of items rolls again after Refill minutes (longer in a picked-over town);
//!   leftovers stay until three times that, then it rolls again.
//! - One record per building: a part the look-around sees (its doors, its furniture prefab or light
//!   probe, named under Prefabs/Structures/Houses/) counts as its building or as nothing
//!   (MainBuilding), and a plain house mesh inside a destructible building's box is a part of it
//!   (IsPart). So a building is rolled and put out once.
//! - Placed at anchors (RZ_LootAnchors): vanilla's own item spots, the tops of the building's
//!   furniture and the floor beside it, and floor points on a grid inside the building's box on
//!   each storey, each traced to the floor, under a ceiling and clear of walls; never in a smaller
//!   building's rooms. At most Place per tick buildings a tick, and at most Lying items on the
//!   server (the buildings nearest players win). A building is never rolled fresh with a player
//!   inside it (Fresh roll distance).
//! - Kept clear: none in a faction's own place (RZ_WorldFacts.IsNearLivedIn) or Clear radius past
//!   it, or near a quartermaster; the rest of a faction's town has loot at half the slot chance.
//! - The log (Loot log): a line per building put out (key, type, building, anchors, items) and a
//!   summary every 2 minutes.
//! - Taken: on the 5 s tick an item that went into an inventory (or was carried off) leaves the
//!   record and the town's loot drops (RZ_TownMemory.ReportSearched). From then on it's an
//!   ordinary item.
//! - Left: nobody within Despawn radius for a minute, and untaken items are deleted and kept as
//!   data. Only the rolled and emptied times are saved: after a restart a building rolls again
//!   once its refill time has passed. Nothing physical is saved.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Interactions > Loot.
[BaseContainerProps()]
class RZ_LootSettings
{
	[Attribute("1", desc: "Loose loot: things lie in houses (on furniture and the floor) and are picked up with vanilla's Pick up. Uses Search > Loot pool and tables, and built-in vanilla items where they are thin. Off = nothing lies in houses (a Car Keys job's keys still do)")]
	bool m_bEnabled;

	[Attribute("55", desc: "A building's loot is put out when a player is this close (m) to it (the look-around reaches this far for buildings), so it's there before you walk in")]
	float m_fSpawnRadius;

	[Attribute("80", desc: "Untaken loot is put away (deleted, kept as data) once no player is this close (m) for a minute")]
	float m_fDespawnRadius;

	[Attribute("20", desc: "Fresh roll distance: a building is never rolled fresh (refilled) while a player is this close (m); it waits until they've gone")]
	float m_fFreshDistance;

	[Attribute("45", desc: "Real minutes before an emptied building has loot again (divided by the town's loot level: longer in a picked-over town)")]
	float m_fRefillMinutes;

	[Attribute("500", desc: "Most loose items lying on the server at once (the buildings nearest players win; performance cap)")]
	int m_iMaxLying;

	[Attribute("6", desc: "Buildings put out per 5 s tick (the nearest to players first)")]
	int m_iPlacePerTick;

	[Attribute("6", desc: "Items per house: its slots (a locked one: one more)")]
	int m_iMaxHouse;

	[Attribute("9", desc: "Items per shop or farm: its slots (a locked one: one more)")]
	int m_iMaxShop;

	[Attribute("10", desc: "Items per pharmacy, police station or garage: its slots (a locked one: one more)")]
	int m_iMaxSpecial;

	[Attribute("0.75", desc: "Slot chance: at full town loot, the chance each slot holds something (x the town's loot level; +0.25 behind a lock)")]
	float m_fSlotChance;

	[Attribute("2", desc: "Sure items: this many of a building's slots always hold something while its town's loot level is 0.5 or more (a picked-over town gets one)")]
	int m_iSureItems;

	[Attribute("0.01", desc: "Town loot each item taken uses up (RZ_TownMemory.ReportSearched; it comes back over about two hours)")]
	float m_fTownLootPerItem;

	[Attribute("20", desc: "Kept clear (m): no loose loot in a faction's own place (a holding's camp as far as its building, yard and pieces reach, a Haven safehouse, a road post, the lab) or within this past it, nor within this + 20 m of a quartermaster. The rest of a faction's town has loot, at half the slot chance")]
	float m_fClearRadius;

	[Attribute("1", desc: "Loot log: a line per building put out (its key, type, anchors and items) and a summary every 2 minutes, for checking loot in play")]
	bool m_bLog;
}

//------------------------------------------------------------------------------------------------
//! One thing lying in a building. Data; m_Ent exists only while the building's loot is out.
class RZ_LootItem
{
	ResourceName m_sPrefab;
	vector m_vPos;
	float m_fYaw;
	string m_sKey;			// a key item: the lock it opens ("" for anything else)
	string m_sTag;
	int m_iNote = -1;		// a readable thing (RZ_Search's note), read where it lies
	int m_iCar = -1;		// a fixable car's keys (RZ_Cars): picking them up is "found"
	IEntity m_Ent;
}

//------------------------------------------------------------------------------------------------
//! One building's loot.
class RZ_LootHouse
{
	string m_sKey;
	vector m_vPos;
	string m_sType;
	IEntity m_Building;
	float m_fRolledAt = -1;
	float m_fEmptiedAt = -1;
	float m_fAwaySince = -1;
	float m_fSeen;
	bool m_bOut;			// its items are spawned
	bool m_bAnchored;		// its anchors were looked for
	int m_iFurniture;		// of its anchors, how many are vanilla item spots or on or by furniture
	bool m_bPlain;			// not a destructible building (a plain house mesh): it may be part of one
	int m_iPart = -1;		// a plain one: -1 not checked yet, 1 inside a destructible building's box (no loot of its own), 0 its own
	float m_fClearAt = -1000;	// when Kept clear was last checked
	bool m_bClear;			// kept clear of loose loot (by a faction's place)
	ref array<vector> m_aAnchors = {};
	ref array<ref RZ_LootItem> m_aItems = {};
}

//------------------------------------------------------------------------------------------------
//! Where things can lie in a building: furniture tops and the floor beside furniture, and floor
//! points on a grid inside the building's box, on every storey. Every point is traced down to a
//! surface, has a ceiling over it (so it's indoors, not on the roof or in a yard) and is clear of
//! walls. Found once per building (the map doesn't change).
//!
//! Vanilla's buildings list their furniture as a child "furniture" prefab (House_..._furniture_01,
//! a mesh-less GenericEntity at the building's origin) whose children are the props (Table_02,
//! Cupboard_01_A, Dresser_01...). None of them has a Hierarchy component, so in the game they may
//! stand on their own rather than as the building's children: the children are walked a few
//! levels down, and a sphere query over every entity (props aren't flagged static) finds the rest.
//! One vanilla furniture prefab also holds SCR_PrefabSpawnPoint entities (the game's own item
//! spots): those are anchors as they are. Vanilla's furniture has no inventory storage, so nothing
//! is put inside it.
//!
//! Storeys: the furniture's own floors (two or more pieces standing at one height), else a guess at
//! the floor above. A point inside a smaller building's box (an add-on, a shed against the wall)
//! belongs to that building, so two buildings never put their loot in one room.
class RZ_LootAnchors
{
	static const int MAX_ANCHORS = 20;
	protected static const float TALL = 1.6;		// furniture taller than this (m): the floor beside it
	protected static const float FLOOR_START = 1.4;	// floor traces start this high above the storey's floor
	protected static const float FLOOR_BELOW = 0.8;	// and reach this far below it
	protected static const float FLOOR_ABOVE = 1.2;	// a point higher than this over the storey's floor is on a shelf
	protected static const float STOREY_GUESS = 3.4;	// no furniture upstairs: the floor above is about here (catches 2.6 to 4.6 m)
	protected static const float STOREY_MIN = 2.2;	// storeys at least this far apart (m)
	protected static const float STOREY_BAND = 0.4;	// furniture within this height of each other stands on one floor
	protected static const int STOREYS_MAX = 4;
	protected static const float GRID = 1.6;		// floor points this far apart (m)
	protected static const float CEILING = 5;		// a ceiling within this (m) over a point: it's indoors
	protected static const float CLEAR = 0.3;		// no wall within this (m) of a point
	protected static const float APART = 0.5;		// anchors at least this far apart (m)
	protected static const int FURNITURE_MAX = 40;
	protected static const int CHILD_DEPTH = 3;		// building > furniture prefab > prop > a prop's own parts
	protected static const int CHILDREN_MAX = 600;	// most child entities looked at in one building
	protected static const int GRID_MAX = 40;		// most floor points tried per storey
	protected static ref array<string> s_aFurnitureWords;

	protected IEntity m_Building;
	protected vector m_vMiddle;
	protected ref array<IEntity> m_aFound = {};
	protected ref array<vector> m_aSpawnPoints = {};
	protected ref array<IEntity> m_aSmaller = {};	// smaller buildings overlapping this one: their rooms are theirs
	protected int m_iChildren;
	protected int m_iFurniture;

	//------------------------------------------------------------------------------------------------
	//! Furniture by lower-case prefab or mesh name (cupboards, fridges, desks, worktops, shelves,
	//! beds, sofas, shop counters). Never a building or a building part (/structures/: "stable"
	//! has "table" in it).
	static bool IsFurnitureName(string lowerName)
	{
		if (!s_aFurnitureWords)
		{
			s_aFurnitureWords = new array<string>();
			RZ_ItemHelper.Words("cupboard,cabinet,wardrobe,fridge,refrigerator,freezer,desk,drawer,dresser,locker,bookcase,bookshelf,sideboard,nightstand,bedside,chest_,kitchen_0,kitchenunit,kitchen_unit,counter,shelf,shelves,medicine_cab,table,workbench,bench_work,crate,pallet,pantry,rack_0,bed_0,beddouble,bunkbed,sofa,armchair,cashier,coolingbox,washingmachine,sink,safe_0", s_aFurnitureWords);
		}

		if (lowerName.Contains("/structures/"))
			return false;

		return RZ_ItemHelper.NameHas(lowerName, s_aFurnitureWords);
	}

	//------------------------------------------------------------------------------------------------
	//! The building whose origin is nearest pos within radius, null if none.
	static IEntity BuildingAt(vector pos, float radius)
	{
		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(pos, radius, buildings, 16);
		IEntity best;
		float bestDist = float.MAX;
		foreach (IEntity b : buildings)
		{
			float d = RZ_Util.DistXZ(b.GetOrigin(), pos);
			if (d < bestDist)
			{
				bestDist = d;
				best = b;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Up to `max` anchors in the building (world positions): vanilla's own item spots and furniture
	//! first, then the floors. Returns how many are on or by furniture (for the log).
	static int Find(IEntity building, notnull array<vector> result, int max)
	{
		if (!building)
			return 0;

		RZ_LootAnchors q = new RZ_LootAnchors();
		q.Run(building, result, max);
		return q.m_iFurniture;
	}

	//------------------------------------------------------------------------------------------------
	//! One anchor for a mission item. False if the building has none.
	static bool PickOne(IEntity building, out vector at)
	{
		array<vector> anchors = {};
		Find(building, anchors, MAX_ANCHORS);
		if (anchors.IsEmpty())
			return false;

		at = anchors.GetRandomElement();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Run(IEntity building, notnull array<vector> result, int max)
	{
		m_Building = building;
		m_vMiddle = building.GetOrigin();
		vector mins;
		vector maxs;
		bool box = RZ_DoorGeometry.LocalBox(building, mins, maxs);
		float radius = 12;
		if (box)
		{
			m_vMiddle = RZ_DoorGeometry.ToWorld(building, (mins + maxs) * 0.5);
			vector size = maxs - mins;
			radius = Math.Max(4, size.Length() * 0.5 + 1);
		}

		// Smaller buildings overlapping this one (before the furniture: their rooms are theirs)
		if (box)
			FindSmaller(mins, maxs, radius);

		// The building's own furniture (its children), then any standing on its own in it. Every
		// entity, not only static ones: vanilla's props aren't flagged static
		WalkChildren(building, 1);
		GetGame().GetWorld().QueryEntitiesBySphere(m_vMiddle, radius, OnEntity, null, EQueryEntitiesFlags.ALL);

		// Vanilla's item spots and furniture fill at most two thirds: the rest lies on the floors,
		// so a room isn't all on one desk. The furniture in a random order, so every storey has some
		int furnitureMax = Math.Max(1, (max * 2) / 3);
		for (int k = m_aFound.Count() - 1; k > 0; k--)
		{
			m_aFound.SwapItems(k, Math.RandomInt(0, k + 1));
		}

		foreach (vector sp : m_aSpawnPoints)
		{
			if (result.Count() >= furnitureMax)
				break;

			vector spAt;
			if (TraceDown(sp + Vector(0, 0.3, 0), 0.8, null, spAt) && Inside(spAt) && Keep(spAt, result))
				result.Insert(spAt);
		}

		foreach (IEntity f : m_aFound)
		{
			if (result.Count() >= furnitureMax)
				break;

			vector at;
			if (FurnitureAnchor(f, at) && Keep(at, result))
				result.Insert(at);
		}

		m_iFurniture = result.Count();
		if (!box)
			return;

		// The floors, each storey its share of what's left (one that finds fewer leaves the rest to
		// the storeys above)
		array<float> storeys = {};
		Storeys(maxs, storeys);
		foreach (int i, float floorY : storeys)
		{
			if (result.Count() >= max)
				break;

			int share = (max - result.Count()) / (storeys.Count() - i);
			if (share < 1)
				share = 1;

			FloorGrid(mins, maxs, floorY, result, result.Count() + share);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The storeys' floors (heights over the building's origin): the ground floor, then each height
	//! where two or more pieces of furniture stand (the props' origins are at their feet), else one
	//! guess at the floor above in a building tall enough.
	protected void Storeys(vector maxs, notnull array<float> floors)
	{
		floors.Insert(0);
		array<float> heights = {};
		foreach (IEntity f : m_aFound)
		{
			heights.Insert(RZ_DoorGeometry.ToLocal(m_Building, f.GetOrigin())[1]);
		}

		heights.Sort();
		for (int i = 0; i < heights.Count() && floors.Count() < STOREYS_MAX; i++)
		{
			float y = heights[i];
			if (y < floors[floors.Count() - 1] + STOREY_MIN || y > maxs[1] - STOREY_MIN)
				continue;

			int standing = 0;
			for (int j = i; j < heights.Count() && heights[j] <= y + STOREY_BAND; j++)
			{
				standing++;
			}

			if (standing >= 2)
				floors.Insert(y);
		}

		if (floors.Count() == 1 && maxs[1] > STOREY_GUESS + STOREY_MIN)
			floors.Insert(STOREY_GUESS);
	}

	//------------------------------------------------------------------------------------------------
	//! Destructible buildings near this one whose box is smaller and overlaps it.
	protected void FindSmaller(vector mins, vector maxs, float radius)
	{
		float area = (maxs[0] - mins[0]) * (maxs[2] - mins[2]);
		array<IEntity> near = {};
		RZ_BuildingQuery.FindBuildings(m_vMiddle, radius + 10, near, 16);
		foreach (IEntity b : near)
		{
			if (b == m_Building || !SCR_DestructibleBuildingEntity.Cast(b))
				continue;

			vector bMins;
			vector bMaxs;
			if (!RZ_DoorGeometry.LocalBox(b, bMins, bMaxs))
				continue;

			float bArea = (bMaxs[0] - bMins[0]) * (bMaxs[2] - bMins[2]);
			if (bArea < area && RZ_LockedSpots.InFootprint(m_Building, b.GetOrigin(), Math.Max(bMaxs[0] - bMins[0], bMaxs[2] - bMins[2])))
				m_aSmaller.Insert(b);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A point to keep: far enough from the others, and not in a smaller building's rooms.
	protected bool Keep(vector at, array<vector> found)
	{
		if (!Fits(at, found))
			return false;

		foreach (IEntity b : m_aSmaller)
		{
			if (b && RZ_DoorGeometry.InBuilding(b, at))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Furniture and vanilla's item spots among the building's children, a few levels down.
	protected void WalkChildren(IEntity e, int depth)
	{
		if (!e || depth > CHILD_DEPTH)
			return;

		IEntity child = e.GetChildren(); // VERIFY: a building's furniture props are found, as its children (GetChildren/GetSibling) or by the sphere query over every entity in its box
		while (child && m_iChildren < CHILDREN_MAX)
		{
			m_iChildren++;
			if (SCR_PrefabSpawnPoint.Cast(child))
				m_aSpawnPoints.Insert(child.GetOrigin());
			else
				TakeFurniture(child, false);

			WalkChildren(child, depth + 1);
			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnEntity(IEntity e)
	{
		if (!e || e == m_Building || ChimeraCharacter.Cast(e))
			return true;

		// Vanilla's own item spot, standing on its own in the building
		if (SCR_PrefabSpawnPoint.Cast(e))
		{
			vector sp = e.GetOrigin();
			if (m_aSpawnPoints.Find(sp) < 0 && RZ_LockedSpots.InFootprint(m_Building, sp, 0))
				m_aSpawnPoints.Insert(sp);

			return true;
		}

		TakeFurniture(e, true);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Keeps e if it's furniture (once). `check`: it must stand in the building's footprint (one the
	//! sphere found; the building's own children are in it).
	protected void TakeFurniture(IEntity e, bool check)
	{
		if (m_aFound.Count() >= FURNITURE_MAX || m_aFound.Find(e) >= 0)
			return;

		string name = RZ_ItemHelper.LowerName(e);
		if (name.IsEmpty() || !IsFurnitureName(name))
			return;

		if (check && !RZ_LockedSpots.InFootprint(m_Building, e.GetOrigin(), 0))
			return;

		m_aFound.Insert(e);
	}

	//------------------------------------------------------------------------------------------------
	//! In the building: inside its box (0.5 m in from the sides), or near its origin without a box.
	protected bool Inside(vector pos)
	{
		vector mins;
		vector maxs;
		if (RZ_DoorGeometry.LocalBox(m_Building, mins, maxs))
			return RZ_DoorGeometry.InBuilding(m_Building, pos);

		return RZ_LockedSpots.InFootprint(m_Building, pos, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! Far enough from every anchor already found.
	protected static bool Fits(vector at, array<vector> found)
	{
		foreach (vector o : found)
		{
			if (vector.Distance(o, at) < APART)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The top of a low piece of furniture (a trace down at its middle from just above its box),
	//! else the floor beside it on the side toward the middle of the building.
	protected bool FurnitureAnchor(IEntity f, out vector at)
	{
		vector mins;
		vector maxs;
		f.GetBounds(mins, maxs);
		float height = maxs[1] - mins[1];
		vector mid = (mins + maxs) * 0.5;
		if (height > 0.3 && height <= TALL)
		{
			vector top = RZ_DoorGeometry.ToWorld(f, Vector(mid[0], maxs[1] + 0.3, mid[2]));
			if (TraceDown(top, height + 0.6, f, at) && Inside(at) && Indoors(at))
				return true;
		}

		vector base = RZ_DoorGeometry.ToWorld(f, Vector(mid[0], mins[1], mid[2]));
		vector toMiddle = m_vMiddle - base;
		toMiddle[1] = 0;
		float len = toMiddle.Length();
		if (len < 0.1)
			toMiddle = Vector(1, 0, 0);
		else
			toMiddle = toMiddle * (1 / len);

		float reach = Math.Max(maxs[0] - mins[0], maxs[2] - mins[2]) * 0.5 + 0.35;
		vector start = base + toMiddle * reach + Vector(0, 1, 0);
		return TraceDown(start, 2, null, at) && Inside(at) && Indoors(at) && Clear(at);
	}

	//------------------------------------------------------------------------------------------------
	//! Floor points on a grid over the box at one storey (`floorY`: its floor's rough height over the
	//! building's origin), in a spread-out order: each traced down from FLOOR_START above it, kept if
	//! it lands near that floor, inside the box, under a ceiling and clear of walls.
	protected void FloorGrid(vector mins, vector maxs, float floorY, notnull array<vector> result, int max)
	{
		float sx = maxs[0] - mins[0] - 1;
		float sz = maxs[2] - mins[2] - 1;
		if (sx <= 0 || sz <= 0)
			return;

		int nx = Math.Max(1, Math.Floor(sx / GRID) + 1);
		int nz = Math.Max(1, Math.Floor(sz / GRID) + 1);
		int total = nx * nz;
		if (total > GRID_MAX)
			total = GRID_MAX;

		// A stride through the cells that's coprime with their count spreads the points out
		int stride = 7;
		while (total > 1 && stride < total && GreatestDivisor(stride, total) != 1)
		{
			stride++;
		}

		int cell = 0;
		for (int n = 0; n < total && result.Count() < max; n++)
		{
			cell = (cell + stride) % total;
			int ix = cell % nx;
			int iz = cell / nx;
			float x = mins[0] + 0.5 + sx * 0.5;
			if (nx > 1)
				x = mins[0] + 0.5 + sx * ix / (nx - 1);

			float z = mins[2] + 0.5 + sz * 0.5;
			if (nz > 1)
				z = mins[2] + 0.5 + sz * iz / (nz - 1);

			vector start = RZ_DoorGeometry.ToWorld(m_Building, Vector(x, floorY + FLOOR_START, z));
			vector at;
			if (!TraceDown(start, FLOOR_START + FLOOR_BELOW, null, at))
				continue;

			// It has to land on this storey (not on a high shelf; a raised floor or a low table is fine)
			float localY = RZ_DoorGeometry.ToLocal(m_Building, at)[1];
			if (localY > floorY + FLOOR_ABOVE)
				continue;

			if (Inside(at) && Keep(at, result) && Indoors(at) && Clear(at))
				result.Insert(at);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static int GreatestDivisor(int a, int b)
	{
		while (b != 0)
		{
			int t = a % b;
			a = b;
			b = t;
		}

		return a;
	}

	//------------------------------------------------------------------------------------------------
	//! A ceiling over the point (a trace up hits something within CEILING): indoors, not on a roof
	//! or in an open yard inside the box.
	protected static bool Indoors(vector at)
	{
		TraceParam param = new TraceParam();
		param.Start = at + Vector(0, 0.3, 0);
		param.End = at + Vector(0, CEILING, 0);
		param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		return ChimeraCharacter.TraceMoveWithoutCharacters(GetGame().GetWorld(), param) < 1;
	}

	//------------------------------------------------------------------------------------------------
	//! No wall within CLEAR of the point at knee height, in any of four directions (an item there
	//! won't sit in a wall or behind one).
	protected static bool Clear(vector at)
	{
		vector from = at + Vector(0, 0.25, 0);
		array<vector> dirs = {Vector(1, 0, 0), Vector(-1, 0, 0), Vector(0, 0, 1), Vector(0, 0, -1)};
		foreach (vector d : dirs)
		{
			TraceParam param = new TraceParam();
			param.Start = from;
			param.End = from + d * CLEAR;
			param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
			param.LayerMask = EPhysicsLayerPresets.Projectile;
			if (ChimeraCharacter.TraceMoveWithoutCharacters(GetGame().GetWorld(), param) < 1)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Straight down from `from`: against `only` if given, else the world and every entity but
	//! characters. False if nothing was hit within depth, or it started inside something.
	protected static bool TraceDown(vector from, float depth, IEntity only, out vector hit)
	{
		TraceParam param = new TraceParam();
		param.Start = from;
		param.End = from - Vector(0, depth, 0);
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		float frac = 1;
		if (only)
		{
			param.Flags = TraceFlags.ENTS;
			param.Include = only;
			frac = GetGame().GetWorld().TraceMove(param, null);
		}
		else
		{
			param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
			frac = ChimeraCharacter.TraceMoveWithoutCharacters(GetGame().GetWorld(), param);
		}

		// A hit at the very start is the inside of a wall or a piece of furniture
		if (frac >= 1 || frac <= 0.01)
			return false;

		hit = from + (param.End - from) * frac;
		return true;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_Loot
{
	protected static const float AWAY_SECONDS = 60;
	protected static const float SUMMARY_SECONDS = 120;
	protected static const float FACTION_TOWN = 150;	// a holding this close (m): a faction's town, half the slot chance
	protected static const float QM_REACH = 20;		// a quartermaster keeps Clear radius and this much more free of loot
	protected static const float CLEAR_RECHECK = 60;	// seconds between checks of a building's Kept clear
	protected static const float PART_REACH = 40;	// a plain house's origin within this of a destructible building's: maybe its part
	protected static const int RECORDS_KEPT = 500;
	protected static const int TIMES_KEPT = 2000;
	protected static const float STALE_MULT = 3;	// leftovers roll again after this many refill times
	protected static const float CARRIED_OFF = 1.5;	// an item this far (m) from where it lay was taken
	protected static const float NOTE_KEY_OFFSET = 0.15;

	protected RZ_GameModeComponent m_GM;
	protected RZ_Interactions m_Hub;
	protected RZ_LootSettings m_Settings;
	protected ref map<string, ref RZ_LootHouse> m_mHouses = new map<string, ref RZ_LootHouse>();
	protected ref map<string, vector> m_mTimes = new map<string, vector>();	// building key: (rolled at, emptied at), with no record (saved, or forgotten)
	protected int m_iLying;
	// For the log's summary: since the last one
	protected float m_fSummaryAt;
	protected int m_iPutOut;
	protected int m_iPlaced;
	protected int m_iTaken;
	protected int m_iNoAnchors;
	protected int m_iParts;

	//------------------------------------------------------------------------------------------------
	void RZ_Loot(RZ_GameModeComponent gm, RZ_Interactions hub, RZ_LootSettings settings)
	{
		m_GM = gm;
		m_Hub = hub;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	bool IsOn() { return m_Settings.m_bEnabled; }
	float GetTownLootPerItem() { return m_Settings.m_fTownLootPerItem; }
	float GetSpawnRadius() { return m_Settings.m_fSpawnRadius; }
	int CountLying() { return m_iLying; }

	// ================================================================ Save / load (times only)
	//------------------------------------------------------------------------------------------------
	void SaveInto(RZ_InteractionSave data)
	{
		float now = m_Hub.GetClock();
		float keep = m_Settings.m_fRefillMinutes * 60 * STALE_MULT * 4;
		foreach (string key, RZ_LootHouse h : m_mHouses)
		{
			if (h.m_fRolledAt >= 0 && now - h.m_fRolledAt < keep && data.lootKey.Count() < TIMES_KEPT)
			{
				data.lootKey.Insert(key);
				data.lootRolled.Insert(h.m_fRolledAt);
				data.lootEmptied.Insert(EmptiedForSave(h));
			}
		}

		foreach (string tkey, vector t : m_mTimes)
		{
			if (now - t[0] < keep && data.lootKey.Count() < TIMES_KEPT)
			{
				data.lootKey.Insert(tkey);
				data.lootRolled.Insert(t[0]);
				data.lootEmptied.Insert(t[1]);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Items aren't saved: after a restart the building counts as emptied when it was rolled
	//! (or emptied), so it rolls again once its refill time has passed.
	protected float EmptiedForSave(RZ_LootHouse h)
	{
		if (h.m_fEmptiedAt >= 0)
			return h.m_fEmptiedAt;

		return h.m_fRolledAt;
	}

	//------------------------------------------------------------------------------------------------
	void RestoreFrom(RZ_InteractionSave data)
	{
		if (!data.lootKey || !data.lootRolled || !data.lootEmptied)
			return;

		for (int i = 0; i < data.lootKey.Count() && i < data.lootRolled.Count() && i < data.lootEmptied.Count(); i++)
		{
			m_mTimes.Set(data.lootKey[i], Vector(data.lootRolled[i], data.lootEmptied[i], 0));
		}
	}

	//------------------------------------------------------------------------------------------------
	void Setup()
	{
		if (!m_Settings.m_bEnabled)
		{
			RZ_Log.Info("Loose loot: off.");
			return;
		}

		RZ_Log.Info(string.Format("Loose loot: on. Put out within %1 m (%2 buildings a tick), put away past %3 m, at most %4 items lying, refill %5 min; %6/%7/%8 slots per house/shop/pharmacy, slot chance %9.",
			RZ_Util.RoundToInt(m_Settings.m_fSpawnRadius), m_Settings.m_iPlacePerTick, RZ_Util.RoundToInt(m_Settings.m_fDespawnRadius), m_Settings.m_iMaxLying, RZ_Util.RoundToInt(m_Settings.m_fRefillMinutes),
			m_Settings.m_iMaxHouse, m_Settings.m_iMaxShop, m_Settings.m_iMaxSpecial, m_Settings.m_fSlotChance));
		string logOn = "off";
		if (m_Settings.m_bLog)
			logOn = "on";

		RZ_Log.Info(string.Format("Loose loot: %1 sure items a building (one in a picked-over town), kept clear %2 m past faction places, loot log %3.",
			m_Settings.m_iSureItems, RZ_Util.RoundToInt(m_Settings.m_fClearRadius), logOn));
	}

	// ================================================================ From the look-around
	//------------------------------------------------------------------------------------------------
	//! A building near a player: its record, made now if it has none. The look-around also sees a
	//! building's parts whose prefab names say "house" (its furniture prefab, its light probe, its
	//! doors, all under Prefabs/Structures/Houses/...): each counts as the building it belongs to, or
	//! as nothing, so one building is one record and its loot is put out once (MainBuilding).
	void OnBuildingSeen(IEntity seen, string lowerName)
	{
		IEntity building = MainBuilding(seen);
		if (!building)
		{
			m_iParts++;
			return;
		}

		if (building != seen)
		{
			m_iParts++;
			lowerName = RZ_ItemHelper.LowerName(building);
		}

		bool plain = !SCR_DestructibleBuildingEntity.Cast(building);
		vector pos = building.GetOrigin();
		string key = RZ_Interactions.PosKey(pos, 4);
		RZ_LootHouse h;
		if (m_mHouses.Find(key, h))
		{
			// The building itself wins over anything else that landed on its key
			if (!h.m_Building || (h.m_Building != building && !plain && h.m_bPlain))
			{
				h.m_Building = building;
				h.m_bPlain = plain;
				h.m_iPart = -1;
				h.m_sType = m_Hub.GetSearch().BuildingType(lowerName);
				if (!h.m_bOut && h.m_aItems.IsEmpty())
				{
					h.m_bAnchored = false;
					h.m_aAnchors.Clear();
				}
			}

			h.m_fSeen = m_Hub.GetClock();
			return;
		}

		h = new RZ_LootHouse();
		h.m_sKey = key;
		h.m_vPos = pos;
		h.m_sType = m_Hub.GetSearch().BuildingType(lowerName);
		h.m_Building = building;
		h.m_bPlain = plain;
		h.m_fSeen = m_Hub.GetClock();
		vector times;
		if (m_mTimes.Find(key, times))
		{
			h.m_fRolledAt = times[0];
			h.m_fEmptiedAt = times[1];
			m_mTimes.Remove(key);
		}

		m_mHouses.Set(key, h);
		if (m_mHouses.Count() > RECORDS_KEPT)
			Prune();
	}

	//------------------------------------------------------------------------------------------------
	//! The building an entity the look-around saw belongs to: itself if it's a destructible
	//! building, else the nearest destructible building above it. Without one, itself only if it
	//! has a mesh and a box a room fits in (a plain house); never a mesh-less part (a building's
	//! light probe or its furniture prefab, which vanilla doesn't attach to the building: they'd
	//! give the building a second roll in its own rooms). null for those.
	static IEntity MainBuilding(IEntity e)
	{
		if (!e)
			return null;

		if (SCR_DestructibleBuildingEntity.Cast(e))
			return e;

		IEntity p = e.GetParent();
		while (p)
		{
			if (SCR_DestructibleBuildingEntity.Cast(p))
				return p;

			p = p.GetParent();
		}

		vector mins;
		vector maxs;
		if (!e.GetVObject() || !RZ_DoorGeometry.LocalBox(e, mins, maxs))
			return null;

		return e;
	}

	//------------------------------------------------------------------------------------------------
	//! A plain house (not a destructible building) that stands inside a destructible building's box
	//! is a part of it (an add-on mesh, a porch): it has no loot of its own. Checked once.
	protected bool IsPart(RZ_LootHouse h)
	{
		if (!h.m_bPlain)
			return false;

		if (h.m_iPart >= 0)
			return h.m_iPart == 1;

		h.m_iPart = 0;
		foreach (string key, RZ_LootHouse o : m_mHouses)
		{
			if (o == h || o.m_bPlain || !o.m_Building || RZ_Util.DistXZ(o.m_vPos, h.m_vPos) > PART_REACH)
				continue;

			if (RZ_LockedSpots.InFootprint(o.m_Building, h.m_vPos, 0.5))
			{
				h.m_iPart = 1;
				break;
			}
		}

		return h.m_iPart == 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Kept clear of loose loot: a faction's own place (RZ_WorldFacts.IsNearLivedIn: a holding's
	//! camp as far as its building, yard and pieces reach, a Haven safehouse, a road post, the lab)
	//! or a quartermaster, within Clear radius. Narrower than RZ_Interactions.IsProtected (what
	//! can't burn or be locked), so the rest of a faction's town has loot in its empty houses.
	protected bool IsKeptClear(vector pos)
	{
		float clear = Math.Max(0, m_Settings.m_fClearRadius);

		RZ_WorldFacts facts = m_GM.GetFacts();
		if (facts && facts.IsNearLivedIn(pos, clear))
			return true;

		RZ_FactionPerks perks = m_GM.GetPerks();
		if (perks)
		{
			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				vector qm;
				if (perks.GetQuartermasterPos(f, qm) && RZ_Util.DistXZ(qm, pos) <= clear + QM_REACH)
					return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! In a faction's town (a holding within 150 m): its empty houses have loot, at half the chance.
	protected bool InFactionTown(vector pos)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		return territory && territory.IsActive() && territory.FindHolding(-1, pos, FACTION_TOWN);
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	//! On the 5 s tick: what was taken, what to put away, and the nearest few buildings to put out.
	void Tick(array<IEntity> players)
	{
		MaybeSummary();
		if (m_mHouses.IsEmpty())
			return;

		bool missionKeys = m_Hub.GetCars().HasUnplacedMissionKeys();
		array<string> wantKeys = {};
		array<float> wantDist = {};
		foreach (string key, RZ_LootHouse h : m_mHouses)
		{
			float near = RZ_Util.NearestDistance(h.m_vPos, players);
			if (h.m_bOut)
			{
				UpdateOut(h, near, missionKeys);
				continue;
			}

			if (near <= m_Settings.m_fSpawnRadius && CanHaveLoot(h))
			{
				wantKeys.Insert(key);
				wantDist.Insert(near);
			}
		}

		for (int n = 0; n < m_Settings.m_iPlacePerTick && !wantKeys.IsEmpty(); n++)
		{
			int best = 0;
			foreach (int i, float d : wantDist)
			{
				if (d < wantDist[best])
					best = i;
			}

			RZ_LootHouse next = m_mHouses.Get(wantKeys[best]);
			float nextDist = wantDist[best];
			wantKeys.Remove(best);
			wantDist.Remove(best);
			if (!MakeRoom(next, nextDist, players))
				break;

			PutOut(next, nextDist);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanHaveLoot(RZ_LootHouse h)
	{
		if (!h.m_Building || IsPart(h) || m_Hub.GetTraps().IsBurned(h.m_vPos))
			return false;

		// Faction places move rarely: checked again once a minute
		float now = m_Hub.GetClock();
		if (now - h.m_fClearAt >= CLEAR_RECHECK)
		{
			h.m_fClearAt = now;
			h.m_bClear = IsKeptClear(h.m_vPos);
		}

		return !h.m_bClear;
	}

	//------------------------------------------------------------------------------------------------
	//! The log's summary every 2 minutes, while anything was out or put out since the last one.
	protected void MaybeSummary()
	{
		float now = m_Hub.GetClock();
		if (now - m_fSummaryAt < SUMMARY_SECONDS)
			return;

		m_fSummaryAt = now;
		int outCount = 0;
		int clearCount = 0;
		int partCount = 0;
		foreach (string key, RZ_LootHouse h : m_mHouses)
		{
			if (h.m_bOut)
				outCount++;

			if (h.m_bClear)
				clearCount++;

			if (h.m_iPart == 1)
				partCount++;
		}

		if (m_Settings.m_bLog && (outCount > 0 || m_iPutOut > 0))
		{
			string known = string.Format("Loot: %1 items lying in %2 buildings out; %3 known, %4 kept clear by faction places, %5 parts of another building.",
				m_iLying, outCount, m_mHouses.Count(), clearCount, partCount);
			RZ_Log.Info(known + string.Format(" Last 2 min: %1 buildings put out, %2 items placed, %3 taken, %4 with no anchors, %5 sightings of a building's parts.",
				m_iPutOut, m_iPlaced, m_iTaken, m_iNoAnchors, m_iParts));
		}

		m_iPutOut = 0;
		m_iPlaced = 0;
		m_iTaken = 0;
		m_iNoAnchors = 0;
		m_iParts = 0;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateOut(RZ_LootHouse h, float near, bool missionKeys)
	{
		if (m_Hub.GetTraps().IsBurned(h.m_vPos))
		{
			// Whatever lay there burned with it
			PutAway(h);
			ClearItems(h);
			h.m_fEmptiedAt = m_Hub.GetClock();
			return;
		}

		CheckTaken(h);
		if (missionKeys)
			AddMissionKeys(h);

		if (near <= m_Settings.m_fDespawnRadius)
		{
			h.m_fAwaySince = -1;
			h.m_fSeen = m_Hub.GetClock();
			return;
		}

		if (h.m_fAwaySince < 0)
		{
			h.m_fAwaySince = m_Hub.GetClock();
			return;
		}

		if (m_Hub.GetClock() - h.m_fAwaySince >= AWAY_SECONDS)
			PutAway(h);
	}

	//------------------------------------------------------------------------------------------------
	//! Items that went into an inventory (their parent is a storage), or were carried off, are taken.
	protected void CheckTaken(RZ_LootHouse h)
	{
		for (int i = h.m_aItems.Count() - 1; i >= 0; i--)
		{
			RZ_LootItem it = h.m_aItems[i];
			if (it.m_iNote >= 0)
				continue;

			if (!it.m_Ent)
			{
				// Deleted by something else: gone, and nobody has it
				m_iLying = Math.Max(0, m_iLying - 1);
				ReleaseItem(it);
				h.m_aItems.Remove(i);
				continue;
			}

			if (!it.m_Ent.GetParent() && RZ_Util.DistXZ(it.m_Ent.GetOrigin(), it.m_vPos) <= CARRIED_OFF)
				continue;

			Taken(h, it);
			h.m_aItems.Remove(i);
		}

		if (!HasItems(h) && h.m_fEmptiedAt < 0)
			h.m_fEmptiedAt = m_Hub.GetClock();
	}

	//------------------------------------------------------------------------------------------------
	protected void Taken(RZ_LootHouse h, RZ_LootItem it)
	{
		m_iLying = Math.Max(0, m_iLying - 1);
		m_iTaken++;
		RZ_TownMemory.ReportSearched(h.m_vPos, m_Settings.m_fTownLootPerItem);
		RZ_GameModeComponent.TrackLooseItem(it.m_Ent); // dropped and forgotten later: cleaned up in time
		if (it.m_iCar >= 0)
			m_Hub.GetCars().OnKeysTaken(it.m_iCar);
	}

	//------------------------------------------------------------------------------------------------
	//! Anything but notes left in it.
	protected bool HasItems(RZ_LootHouse h)
	{
		foreach (RZ_LootItem it : h.m_aItems)
		{
			if (it.m_iNote < 0)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Untaken items are deleted and kept as data.
	protected void PutAway(RZ_LootHouse h)
	{
		CheckTaken(h);
		foreach (RZ_LootItem it : h.m_aItems)
		{
			if (it.m_iNote >= 0)
				m_Hub.GetSearch().DespawnNote(it.m_iNote);
			else if (it.m_Ent)
				SCR_EntityHelper.DeleteEntityAndChildren(it.m_Ent);

			if (it.m_Ent)
				m_iLying = Math.Max(0, m_iLying - 1);

			it.m_Ent = null;
		}

		h.m_bOut = false;
		h.m_fAwaySince = -1;
	}

	//------------------------------------------------------------------------------------------------
	//! A car's keys and a note left in a record are let go (the car's keys can lie elsewhere).
	protected void ReleaseItem(RZ_LootItem it)
	{
		if (it.m_iCar >= 0)
			m_Hub.GetCars().ReleaseKeys(it.m_iCar);

		if (it.m_iNote >= 0)
			m_Hub.GetSearch().ReleaseNote(it.m_iNote);
	}

	//------------------------------------------------------------------------------------------------
	//! Every item goes (a put-away record only: nothing is spawned).
	protected void ClearItems(RZ_LootHouse h)
	{
		foreach (RZ_LootItem it : h.m_aItems)
		{
			ReleaseItem(it);
		}

		h.m_aItems.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Room under Lying items for this building: the farthest put-out building goes if it's
	//! farther from players than this one. False if there's no room.
	protected bool MakeRoom(RZ_LootHouse h, float dist, array<IEntity> players)
	{
		int need = h.m_aItems.Count();
		if (NeedsRoll(h))
			need = MostItems(h.m_sType) + 3;

		for (int guard = 0; guard < 8 && m_iLying + need > m_Settings.m_iMaxLying; guard++)
		{
			RZ_LootHouse far;
			float farDist = -1;
			foreach (string key, RZ_LootHouse o : m_mHouses)
			{
				if (!o.m_bOut)
					continue;

				float d = RZ_Util.NearestDistance(o.m_vPos, players);
				if (d > farDist)
				{
					farDist = d;
					far = o;
				}
			}

			if (!far || farDist <= dist)
				return false;

			PutAway(far);
		}

		return m_iLying + need <= m_Settings.m_iMaxLying;
	}

	// ================================================================ Rolling and putting out
	//------------------------------------------------------------------------------------------------
	protected int MostItems(string type)
	{
		switch (type)
		{
			case "pharmacy": return m_Settings.m_iMaxSpecial;
			case "police": return m_Settings.m_iMaxSpecial;
			case "garage": return m_Settings.m_iMaxSpecial;
			case "shop": return m_Settings.m_iMaxShop;
			case "farm": return m_Settings.m_iMaxShop;
		}

		return m_Settings.m_iMaxHouse;
	}

	//------------------------------------------------------------------------------------------------
	//! Refill minutes, divided by the town's loot level (at most four times as long).
	protected float RefillSeconds(RZ_LootHouse h)
	{
		float level = Math.Max(0.25, RZ_TownMemory.GetLootLevel(h.m_vPos));
		return m_Settings.m_fRefillMinutes * 60 / level;
	}

	//------------------------------------------------------------------------------------------------
	protected bool NeedsRoll(RZ_LootHouse h)
	{
		if (h.m_fRolledAt < 0)
			return true;

		float now = m_Hub.GetClock();
		float refill = RefillSeconds(h);
		if (now - h.m_fRolledAt >= refill * STALE_MULT)
			return true;

		return !HasItems(h) && h.m_fEmptiedAt >= 0 && now - h.m_fEmptiedAt >= refill;
	}

	//------------------------------------------------------------------------------------------------
	//! `near`: the nearest player's distance. A building that was rolled before is only rolled fresh
	//! with nobody within Fresh roll distance: nothing new appears in a room someone stands in (what
	//! was left there is put out as it was).
	protected void PutOut(RZ_LootHouse h, float near)
	{
		if (!h.m_bAnchored)
		{
			h.m_bAnchored = true;
			h.m_iFurniture = RZ_LootAnchors.Find(h.m_Building, h.m_aAnchors, RZ_LootAnchors.MAX_ANCHORS);
			ShuffleAnchors(h);
		}

		bool rolled = false;
		if (NeedsRoll(h) && (h.m_fRolledAt < 0 || near > m_Settings.m_fFreshDistance))
		{
			Roll(h);
			rolled = true;
		}

		// A stash someone wrote about: two more good finds (not what's already here)
		if (m_Settings.m_bEnabled && m_Hub.GetSearch().TakeStashNear(h.m_vPos))
		{
			array<ResourceName> stash = {};
			PrefabsHere(h, stash);
			int had = stash.Count();
			m_Hub.GetSearch().PickHouseLootAt(h.m_sType, h.m_vPos, true, 2, 2, 1, stash);
			for (int s = had; s < stash.Count(); s++)
			{
				AddItem(h, stash[s], "", "", -1, -1);
			}
		}

		AddMissionKeys(h);
		h.m_bOut = true;
		h.m_fAwaySince = -1;
		int placed = 0;
		for (int i = h.m_aItems.Count() - 1; i >= 0; i--)
		{
			if (h.m_aItems[i].m_Ent)
				continue;

			if (SpawnItem(h.m_aItems[i]))
			{
				placed++;
				continue;
			}

			ReleaseItem(h.m_aItems[i]);
			h.m_aItems.Remove(i);
		}

		m_iPutOut++;
		m_iPlaced += placed;
		if (h.m_aAnchors.IsEmpty())
			m_iNoAnchors++;

		LogPutOut(h, rolled, placed);
	}

	//------------------------------------------------------------------------------------------------
	//! The prefabs lying in the building now (so a new find isn't one of them).
	protected void PrefabsHere(RZ_LootHouse h, notnull array<ResourceName> into)
	{
		foreach (RZ_LootItem it : h.m_aItems)
		{
			if (!it.m_sPrefab.IsEmpty() && it.m_sKey.IsEmpty())
				into.Insert(it.m_sPrefab);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! One line per building put out: its key (so two records for one building show up), type,
	//! anchors (on furniture / all) and the items' prefab names.
	protected void LogPutOut(RZ_LootHouse h, bool rolled, int placed)
	{
		if (!m_Settings.m_bLog)
			return;

		string items = "";
		foreach (RZ_LootItem it : h.m_aItems)
		{
			string name = "note";
			if (it.m_iNote < 0)
				name = ShortName(it.m_sPrefab);

			if (!it.m_sKey.IsEmpty())
				name = "key";

			if (!items.IsEmpty())
				items += ", ";

			items += name;
		}

		if (items.IsEmpty())
			items = "nothing";

		string how = "kept";
		if (rolled)
			how = "rolled";

		string at = string.Format("%1 %2", RZ_Util.RoundToInt(h.m_vPos[0]), RZ_Util.RoundToInt(h.m_vPos[2]));
		RZ_Log.Info(string.Format("Loot: put out %1 (%2, %3) at %4: %5 anchors (%6 on furniture), %7 %8 items, %9 spawned: " + items,
			h.m_sKey, h.m_sType, ShortName(RZ_Util.GetPrefabName(h.m_Building)), at, h.m_aAnchors.Count(), h.m_iFurniture, how, h.m_aItems.Count(), placed));
	}

	//------------------------------------------------------------------------------------------------
	//! A prefab's file name without folder or extension ("Flashlight_Soviet_01").
	protected static string ShortName(string prefab)
	{
		int slash = prefab.LastIndexOf("/");
		string name = prefab;
		if (slash >= 0)
			name = prefab.Substring(slash + 1, prefab.Length() - slash - 1);

		if (name.EndsWith(".et"))
			name = name.Substring(0, name.Length() - 3);

		return name;
	}

	//------------------------------------------------------------------------------------------------
	protected void ShuffleAnchors(RZ_LootHouse h)
	{
		for (int i = h.m_aAnchors.Count() - 1; i > 0; i--)
		{
			int j = Math.RandomInt(0, i + 1);
			vector a = h.m_aAnchors[i];
			h.m_aAnchors[i] = h.m_aAnchors[j];
			h.m_aAnchors[j] = a;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! What lies here now: the search rules, a treatment, sometimes a car's keys and a readable.
	protected void Roll(RZ_LootHouse h)
	{
		ClearItems(h);
		h.m_fRolledAt = m_Hub.GetClock();
		h.m_fEmptiedAt = -1;

		bool locked = m_Hub.GetDoors().HasDoorLock(h.m_Building);
		RZ_Search search = m_Hub.GetSearch();
		if (m_Settings.m_bEnabled)
		{
			int most = MostItems(h.m_sType);
			if (locked)
				most++;

			// Searched-but-not-empty: the first Sure items are always there (one in a picked-over
			// town); the rest each hold something Slot chance x the town's loot level (half that in
			// a faction's town). No prefab twice unless it stacks (RZ_Search.PickHouseLootAt).
			int sure = Math.Min(1, m_Settings.m_iSureItems);
			if (RZ_TownMemory.GetLootLevel(h.m_vPos) >= 0.5)
				sure = m_Settings.m_iSureItems;

			sure = Math.Min(sure, most);
			float chance = m_Settings.m_fSlotChance;
			if (InFactionTown(h.m_vPos))
				chance *= 0.5;

			array<ResourceName> picks = {};
			search.PickHouseLootAt(h.m_sType, h.m_vPos, locked, most, sure, chance, picks);
			foreach (ResourceName pick : picks)
			{
				AddItem(h, pick, "", "", -1, -1);
			}

			// Treatments (docs/DESIGN.md, "Treatments"): one roll, better behind a lock
			RZ_Infection infection = m_GM.GetInfection();
			if (infection)
			{
				int med;
				ResourceName treatment = infection.PickMedFind(h.m_sType, locked, med);
				if (!treatment.IsEmpty())
					AddItem(h, treatment, "", "", -1, -1);
			}

			// The keys of this town's fixable car, sometimes
			AddCarKeys(h, m_Hub.GetCars().ClaimTownKeysAt(h.m_vPos), false);

			if (Math.RandomFloat01() < search.GetReadableChance())
				AddReadable(h);
		}

		if (!HasItems(h))
			h.m_fEmptiedAt = h.m_fRolledAt;
	}

	//------------------------------------------------------------------------------------------------
	//! A letter (or an ID card, a receipt, a photo) lying here, with the key that was taped inside
	//! next to it now and then.
	protected void AddReadable(RZ_LootHouse h)
	{
		string keyId;
		string keyTag;
		string line;
		int note = m_Hub.GetSearch().CreateReadable(h.m_vPos, false, true, keyId, keyTag, line);
		if (note < 0)
			return;

		RZ_LootItem paper = AddItem(h, "", "", "", note, -1);
		if (!paper)
		{
			m_Hub.GetSearch().ReleaseNote(note);
			return;
		}

		if (keyId.IsEmpty())
			return;

		RZ_LootItem key = new RZ_LootItem();
		key.m_sPrefab = m_Hub.GetSearch().GetKeyPrefab();
		key.m_sKey = keyId;
		key.m_sTag = keyTag;
		key.m_vPos = paper.m_vPos + Vector(NOTE_KEY_OFFSET, 0, NOTE_KEY_OFFSET);
		key.m_fYaw = Math.RandomFloat(0, 360);
		h.m_aItems.Insert(key);
	}

	//------------------------------------------------------------------------------------------------
	//! A mission car's keys whose house this is (Car Keys): always here, by the door if the house
	//! has no anchor.
	protected void AddMissionKeys(RZ_LootHouse h)
	{
		int car = m_Hub.GetCars().ClaimMissionKeysAt(h.m_vPos);
		if (car < 0)
			return;

		RZ_LootItem it = AddCarKeys(h, car, true);
		if (it && h.m_bOut && !SpawnItem(it))
		{
			ReleaseItem(it);
			h.m_aItems.RemoveItem(it);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A car's key item in this building. null (and the keys let go) if there's no room or no prefab.
	protected RZ_LootItem AddCarKeys(RZ_LootHouse h, int car, bool always)
	{
		if (car < 0)
			return null;

		string keyId;
		string tag;
		ResourceName prefab = m_Hub.GetSearch().GetKeyPrefab();
		if (prefab.IsEmpty() || !m_Hub.GetCars().KeyFor(car, keyId, tag))
		{
			m_Hub.GetCars().ReleaseKeys(car);
			return null;
		}

		RZ_LootItem it = AddItem(h, prefab, keyId, tag, -1, car);
		if (it || !always)
		{
			if (!it)
				m_Hub.GetCars().ReleaseKeys(car);

			return it;
		}

		// No anchor left: just outside the wall nearest its middle, where someone dropped them
		it = new RZ_LootItem();
		it.m_sPrefab = prefab;
		it.m_sKey = keyId;
		it.m_sTag = tag;
		it.m_iCar = car;
		it.m_vPos = RZ_Util.GroundPos(RZ_DoorGeometry.SidePoint(h.m_Building, h.m_vPos, 0.4));
		it.m_fYaw = Math.RandomFloat(0, 360);
		h.m_aItems.Insert(it);
		return it;
	}

	//------------------------------------------------------------------------------------------------
	//! An item at the next free anchor. null if the building has no free anchor (it isn't placed).
	protected RZ_LootItem AddItem(RZ_LootHouse h, ResourceName prefab, string keyId, string tag, int note, int car)
	{
		vector at;
		if (!FreeAnchor(h, at))
			return null;

		RZ_LootItem it = new RZ_LootItem();
		it.m_sPrefab = prefab;
		it.m_sKey = keyId;
		it.m_sTag = tag;
		it.m_iNote = note;
		it.m_iCar = car;
		it.m_vPos = at;
		it.m_fYaw = Math.RandomFloat(0, 360);
		h.m_aItems.Insert(it);
		return it;
	}

	//------------------------------------------------------------------------------------------------
	protected bool FreeAnchor(RZ_LootHouse h, out vector at)
	{
		foreach (vector a : h.m_aAnchors)
		{
			bool used = false;
			foreach (RZ_LootItem it : h.m_aItems)
			{
				if (vector.Distance(it.m_vPos, a) < 0.3)
				{
					used = true;
					break;
				}
			}

			if (!used)
			{
				at = a;
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns one item where it lies. False if it couldn't be.
	protected bool SpawnItem(RZ_LootItem it)
	{
		if (it.m_Ent)
			return true;

		vector pos = it.m_vPos + Vector(0, 0.02, 0);
		if (it.m_iNote >= 0)
			it.m_Ent = m_Hub.GetSearch().SpawnNote(it.m_iNote, pos, it.m_fYaw);
		else if (!it.m_sKey.IsEmpty())
			it.m_Ent = RZ_KeyItemComponent.Spawn(it.m_sPrefab, pos, it.m_fYaw, it.m_sKey, it.m_sTag);
		else
			it.m_Ent = SpawnLoose(it.m_sPrefab, pos, it.m_fYaw);

		if (!it.m_Ent)
			return false;

		m_iLying++;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static IEntity SpawnLoose(ResourceName prefab, vector pos, float yaw)
	{
		// VERIFY: a spawned item lies still where it's put (on furniture, not through the floor or rolling off)
		IEntity e = RZ_Util.SpawnPrefab(prefab, pos, yaw);
		return e; // VERIFY: vanilla persistence doesn't save items lying in the world (else loose loot leaves traces after a restart)
	}

	//------------------------------------------------------------------------------------------------
	//! A mission car is gone (Car Keys ended): its keys, wherever they lie, go too.
	void DropCarKeys(int carId)
	{
		foreach (string key, RZ_LootHouse h : m_mHouses)
		{
			for (int i = h.m_aItems.Count() - 1; i >= 0; i--)
			{
				RZ_LootItem it = h.m_aItems[i];
				if (it.m_iCar != carId)
					continue;

				if (it.m_Ent)
				{
					SCR_EntityHelper.DeleteEntityAndChildren(it.m_Ent);
					m_iLying = Math.Max(0, m_iLying - 1);
				}

				h.m_aItems.Remove(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Too many records: the ones not seen for longest (and not out) go; their times are kept.
	protected void Prune()
	{
		array<string> keys = {};
		array<float> seen = {};
		foreach (string key, RZ_LootHouse h : m_mHouses)
		{
			if (h.m_bOut)
				continue;

			keys.Insert(key);
			seen.Insert(h.m_fSeen);
		}

		int drop = m_mHouses.Count() - (RECORDS_KEPT - 50);
		for (int n = 0; n < drop && !keys.IsEmpty(); n++)
		{
			int oldest = 0;
			foreach (int i, float s : seen)
			{
				if (s < seen[oldest])
					oldest = i;
			}

			RZ_LootHouse gone = m_mHouses.Get(keys[oldest]);
			if (gone.m_fRolledAt >= 0)
				m_mTimes.Set(gone.m_sKey, Vector(gone.m_fRolledAt, EmptiedForSave(gone), 0));

			ClearItems(gone);
			m_mHouses.Remove(keys[oldest]);
			keys.Remove(oldest);
			seen.Remove(oldest);
		}

		if (m_mTimes.Count() > TIMES_KEPT)
			m_mTimes.Clear();
	}
}
