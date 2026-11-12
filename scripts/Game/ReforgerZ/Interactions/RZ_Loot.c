//! Loose loot (docs/DESIGN.md, "Loot in houses"): things lie in houses, on furniture and on the
//! floor, and are picked up with vanilla's Pick up. There are no search prompts in buildings.
//!
//! - Data first. Every building the look-around sees near a player (RZ_Interactions) gets a
//!   small record: its type, when it was rolled and emptied, and its items as data (prefab,
//!   anchor, key, note). Nothing is spawned until a player is within Spawn radius.
//! - Rolled on the first visit with the search rules (RZ_Search.PickHouseLootAt: the type's table,
//!   with the built-in vanilla items where it's thin; Slot chance x the town's loot level, the first
//!   Sure items always there in a town that isn't picked over; the rare and electronics chances),
//!   plus one treatment roll
//!   (RZ_Infection.PickMedFind), sometimes the town's fixable car's keys (RZ_Cars), and sometimes
//!   a readable (a note, with a key beside it now and then). A locked place (a seeded lock on real
//!   outside doors) gets one more item and better finds. A stash someone wrote about adds two.
//!   A building emptied of items rolls again after Refill minutes (longer in a picked-over town);
//!   leftovers stay until three times that, then it rolls again.
//! - Placed at anchors (RZ_LootAnchors): the tops of furniture in the building and the floor beside
//!   it, and floor points on a grid inside the building's box on each storey, each traced to the
//!   floor, under a ceiling and clear of walls. At most 4 buildings a tick, and at most Lying items
//!   on the server (the buildings nearest players win). A building is never rolled fresh with a
//!   player inside it (Fresh roll distance).
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

	[Attribute("45", desc: "A building's loot is put out when a player is this close (m) to it (the look-around reaches this far for buildings)")]
	float m_fSpawnRadius;

	[Attribute("70", desc: "Untaken loot is put away (deleted, kept as data) once no player is this close (m) for a minute")]
	float m_fDespawnRadius;

	[Attribute("20", desc: "Fresh roll distance: a building is never rolled fresh (refilled) while a player is this close (m); it waits until they've gone")]
	float m_fFreshDistance;

	[Attribute("60", desc: "Real minutes before an emptied building has loot again (divided by the town's loot level: longer in a picked-over town)")]
	float m_fRefillMinutes;

	[Attribute("350", desc: "Most loose items lying on the server at once (the buildings nearest players win; performance cap)")]
	int m_iMaxLying;

	[Attribute("4", desc: "Items per house: its slots (a locked one: one more)")]
	int m_iMaxHouse;

	[Attribute("5", desc: "Items per shop or farm: its slots (a locked one: one more)")]
	int m_iMaxShop;

	[Attribute("6", desc: "Items per pharmacy, police station or garage: its slots (a locked one: one more)")]
	int m_iMaxSpecial;

	[Attribute("0.7", desc: "Slot chance: at full town loot, the chance each slot holds something (x the town's loot level; +0.25 behind a lock)")]
	float m_fSlotChance;

	[Attribute("1", desc: "Sure items: this many of a building's slots always hold something while its town's loot level is 0.5 or more (a picked-over town gets none)")]
	int m_iSureItems;

	[Attribute("0.01", desc: "Town loot each item taken uses up (RZ_TownMemory.ReportSearched; it comes back over about two hours)")]
	float m_fTownLootPerItem;
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
	ref array<vector> m_aAnchors = {};
	ref array<ref RZ_LootItem> m_aItems = {};
}

//------------------------------------------------------------------------------------------------
//! Where things can lie in a building: furniture tops and the floor beside furniture, and floor
//! points on a grid inside the building's box, on the ground floor and the one above. Every point
//! is traced down to a surface, has a ceiling over it (so it's indoors, not on the roof or in a
//! yard) and is clear of walls. Found once per building (the map doesn't change).
class RZ_LootAnchors
{
	static const int MAX_ANCHORS = 16;
	protected static const float TALL = 1.6;		// furniture taller than this (m): the floor beside it
	protected static const float FLOOR_START = 1.4;	// floor traces start this high above the storey's floor
	protected static const float STOREY = 3;		// the next storey's floor is about this far up
	protected static const float GRID = 1.6;		// floor points this far apart (m)
	protected static const float CEILING = 5;		// a ceiling within this (m) over a point: it's indoors
	protected static const float CLEAR = 0.3;		// no wall within this (m) of a point
	protected static const float APART = 0.5;		// anchors at least this far apart (m)
	protected static const int FURNITURE_MAX = 24;
	protected static const int GRID_MAX = 40;		// most floor points tried per storey
	protected static ref array<string> s_aFurnitureWords;

	protected IEntity m_Building;
	protected vector m_vMiddle;
	protected ref array<IEntity> m_aFound = {};

	//------------------------------------------------------------------------------------------------
	//! Furniture by lower-case prefab or mesh name (cupboards, fridges, desks, worktops, shelves).
	static bool IsFurnitureName(string lowerName)
	{
		if (!s_aFurnitureWords)
		{
			s_aFurnitureWords = new array<string>();
			RZ_ItemHelper.Words("cupboard,cabinet,wardrobe,fridge,refrigerator,freezer,desk,drawer,dresser,locker,bookcase,bookshelf,sideboard,nightstand,bedside,chest_of,kitchen_unit,kitchenunit,counter,shelf,shelves,medicine_cab,table,workbench,bench_work,crate,pallet", s_aFurnitureWords);
		}

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
	//! Up to `max` anchors in the building (world positions), furniture first, then the floors.
	static void Find(IEntity building, notnull array<vector> result, int max)
	{
		if (!building)
			return;

		RZ_LootAnchors q = new RZ_LootAnchors();
		q.Run(building, result, max);
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

		// Furniture fills at most half: the rest lies on the floors, so a room isn't all on one desk
		int furnitureMax = Math.Max(1, max / 2);
		GetGame().GetWorld().QueryEntitiesBySphere(m_vMiddle, radius, OnEntity, null, EQueryEntitiesFlags.STATIC); // VERIFY: furniture inside building prefabs is found as static entities (else only the floor points are used)
		foreach (IEntity f : m_aFound)
		{
			if (result.Count() >= furnitureMax)
				break;

			vector at;
			if (FurnitureAnchor(f, at) && Fits(at, result))
				result.Insert(at);
		}

		if (!box)
			return;

		// The ground floor (about the building's origin), then the storey above if it's tall enough
		FloorGrid(mins, maxs, 0, result, max);
		if (maxs[1] > STOREY + 2.2)
			FloorGrid(mins, maxs, STOREY, result, max);
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnEntity(IEntity e)
	{
		if (!e || e == m_Building || m_aFound.Count() >= FURNITURE_MAX || ChimeraCharacter.Cast(e))
			return true;

		string name = RZ_ItemHelper.LowerName(e);
		if (name.IsEmpty() || !IsFurnitureName(name))
			return true;

		if (RZ_LockedSpots.InFootprint(m_Building, e.GetOrigin(), 0))
			m_aFound.Insert(e);

		return true;
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
			if (!TraceDown(start, FLOOR_START + 0.8, null, at))
				continue;

			// It has to land on this storey (not on a high shelf; a raised floor or a low table is fine)
			float localY = RZ_DoorGeometry.ToLocal(m_Building, at)[1];
			if (localY > floorY + 1.2)
				continue;

			if (Inside(at) && Indoors(at) && Clear(at) && Fits(at, result))
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
	protected static const int PLACE_PER_TICK = 4;
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

		RZ_Log.Info(string.Format("Loose loot: on. Put out within %1 m, put away past %2 m, at most %3 items lying, refill %4 min; %5/%6/%7 items per house/shop/pharmacy.",
			RZ_Util.RoundToInt(m_Settings.m_fSpawnRadius), RZ_Util.RoundToInt(m_Settings.m_fDespawnRadius), m_Settings.m_iMaxLying, RZ_Util.RoundToInt(m_Settings.m_fRefillMinutes),
			m_Settings.m_iMaxHouse, m_Settings.m_iMaxShop, m_Settings.m_iMaxSpecial));
	}

	// ================================================================ From the look-around
	//------------------------------------------------------------------------------------------------
	//! A building near a player: its record, made now if it has none.
	void OnBuildingSeen(IEntity building, string lowerName)
	{
		if (!building)
			return;

		vector pos = building.GetOrigin();
		string key = RZ_Interactions.PosKey(pos, 4);
		RZ_LootHouse h;
		if (m_mHouses.Find(key, h))
		{
			if (!h.m_Building)
				h.m_Building = building;

			h.m_fSeen = m_Hub.GetClock();
			return;
		}

		h = new RZ_LootHouse();
		h.m_sKey = key;
		h.m_vPos = pos;
		h.m_sType = m_Hub.GetSearch().BuildingType(lowerName);
		h.m_Building = building;
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

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	//! On the 5 s tick: what was taken, what to put away, and the nearest few buildings to put out.
	void Tick(array<IEntity> players)
	{
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

		for (int n = 0; n < PLACE_PER_TICK && !wantKeys.IsEmpty(); n++)
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
		if (!h.m_Building)
			return false;

		return !m_Hub.GetTraps().IsBurned(h.m_vPos) && !m_Hub.IsProtected(h.m_vPos);
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
			RZ_LootAnchors.Find(h.m_Building, h.m_aAnchors, RZ_LootAnchors.MAX_ANCHORS);
			ShuffleAnchors(h);
		}

		if (NeedsRoll(h) && (h.m_fRolledAt < 0 || near > m_Settings.m_fFreshDistance))
			Roll(h);

		// A stash someone wrote about: two more good finds
		if (m_Settings.m_bEnabled && m_Hub.GetSearch().TakeStashNear(h.m_vPos))
		{
			array<ResourceName> stash = {};
			m_Hub.GetSearch().PickHouseLootAt(h.m_sType, h.m_vPos, true, 2, 2, 1, stash);
			foreach (ResourceName s : stash)
			{
				AddItem(h, s, "", "", -1, -1);
			}
		}

		AddMissionKeys(h);
		h.m_bOut = true;
		h.m_fAwaySince = -1;
		for (int i = h.m_aItems.Count() - 1; i >= 0; i--)
		{
			if (!SpawnItem(h.m_aItems[i]))
			{
				ReleaseItem(h.m_aItems[i]);
				h.m_aItems.Remove(i);
			}
		}
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

			// Searched-but-not-empty: the first Sure items are always there unless the town is
			// picked over; the rest each hold something Slot chance x the town's loot level
			int sure = 0;
			if (RZ_TownMemory.GetLootLevel(h.m_vPos) >= 0.5)
				sure = Math.Min(m_Settings.m_iSureItems, most);

			array<ResourceName> picks = {};
			search.PickHouseLootAt(h.m_sType, h.m_vPos, locked, most, sure, m_Settings.m_fSlotChance, picks);
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
