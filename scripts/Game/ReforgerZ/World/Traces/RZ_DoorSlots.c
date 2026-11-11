//! A town's door slots (docs/TRACES.md, "Placement on any map"): its buildings with an outside door,
//! one slot each, sorted by a hash of the building's position and the season seed, so a slot number
//! names the same door after every restart and nothing about a building is saved. Resolved once a
//! run per town (one sphere query, at most 64 buildings and their door children), when a player
//! first comes within 400 m, one town per tick. Runtime only.
//!
//! Left out: buildings inside a lived-in place's ring (camp life owns those), except the building
//! a safehouse stands in (its patched bear), and buildings within 25 m of a running mission.
//! Server only.

//------------------------------------------------------------------------------------------------
class RZ_DoorSlot
{
	vector m_vPos;			// the mark: on the door's wall, beside the leaf, at chest height
	vector m_vNormal;		// straight out of the wall
	vector m_vBuilding;		// the building's origin
	int m_iKind;			// RZ_EDoorWant: HOUSE, SHOP, CLINIC, FARM, GARAGE, PUBLIC
	bool m_bLocked;
	int m_iHash;
}

//------------------------------------------------------------------------------------------------
class RZ_TownSlots
{
	int m_iTown = -1;
	ref array<ref RZ_DoorSlot> m_aSlots = {};
}

//------------------------------------------------------------------------------------------------
class RZ_DoorSlots
{
	protected static const float RESOLVE_RADIUS = 400;	// a player this far from a town's edge
	protected static const int MAX_BUILDINGS = 64;
	protected static const float BESIDE_LEAF = 0.8;		// metres along the wall from the doorway
	protected static const float MARK_HEIGHT = 1.5;

	protected RZ_GameModeComponent m_GM;
	protected ref map<int, ref RZ_TownSlots> m_mTowns = new map<int, ref RZ_TownSlots>();

	//------------------------------------------------------------------------------------------------
	void RZ_DoorSlots(RZ_GameModeComponent gm)
	{
		m_GM = gm;
	}

	//------------------------------------------------------------------------------------------------
	bool IsResolved(int town)
	{
		return m_mTowns.Contains(town);
	}

	//------------------------------------------------------------------------------------------------
	RZ_TownSlots Get(int town)
	{
		RZ_TownSlots t;
		m_mTowns.Find(town, t);
		return t;
	}

	//------------------------------------------------------------------------------------------------
	int ResolvedCount()
	{
		return m_mTowns.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! One town per call: the first unresolved town a player is within 400 m of. Its index, or -1.
	int Step(array<IEntity> players)
	{
		if (players.IsEmpty())
			return -1;

		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			if (m_mTowns.Contains(i))
				continue;

			if (RZ_Util.NearestDistance(s.m_vCenter, players) > s.m_fRadius + RESOLVE_RADIUS)
				continue;

			Resolve(i);
			return i;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! The town's slots, worked out now.
	RZ_TownSlots Resolve(int town)
	{
		RZ_TownSlots t = new RZ_TownSlots();
		t.m_iTown = town;
		m_mTowns.Set(town, t);
		RZ_Settlement s = m_GM.GetSettlement(town);
		if (!s)
			return t;

		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(s.m_vCenter, s.m_fRadius, buildings, MAX_BUILDINGS);
		int seed = RZ_WorldFacts.Get().GetSeed();
		array<vector> busy = {};
		RZ_MissionManager missions = m_GM.GetMissions();
		if (missions)
		{
			foreach (RZ_Mission m : missions.GetActive())
			{
				if (m && !m.m_bGlobal)
					busy.Insert(m.m_vPos);
			}
		}

		foreach (IEntity b : buildings)
		{
			if (!b || InRing(b))
				continue;

			array<IEntity> doors = {};
			RZ_DoorGeometry.FindDoors(b, 0, doors);
			IEntity door = null;
			foreach (IEntity d : doors)
			{
				if (RZ_DoorGeometry.IsOuterDoor(b, d))
				{
					door = d;
					break;
				}
			}

			if (!door)
				continue;

			RZ_DoorSlot slot = MakeSlot(b, door, seed);
			bool nearJob = false;
			foreach (vector job : busy)
			{
				if (RZ_Util.DistXZ(job, slot.m_vPos) < 25)
					nearJob = true;
			}

			if (!nearJob)
				t.m_aSlots.Insert(slot);
		}

		SortByHash(t.m_aSlots);
		return t;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_DoorSlot MakeSlot(IEntity b, IEntity door, int seed)
	{
		RZ_DoorSlot slot = new RZ_DoorSlot();
		vector normal = RZ_DoorGeometry.WallNormal(b, door);
		vector centre = RZ_DoorGeometry.DoorwayCentre(door);
		// Beside the leaf, never on it or in its swing: along the wall, a little off its face
		vector along = Vector(-normal[2], 0, normal[0]);
		vector pos = centre + along * BESIDE_LEAF + normal * 0.05;
		if (normal == vector.Zero)
			pos = RZ_DoorGeometry.OutsidePoint(b, door, 0.3);

		pos[1] = RZ_Util.GroundPos(pos)[1] + MARK_HEIGHT;
		slot.m_vPos = pos;
		slot.m_vNormal = normal;
		slot.m_vBuilding = b.GetOrigin();
		slot.m_iKind = KindOf(b);
		slot.m_bLocked = RZ_LockedSpots.IsLockedBuilding(b);
		vector o = b.GetOrigin();
		slot.m_iHash = (RZ_Util.RoundToInt(o[0]) * 73856093) ^ (RZ_Util.RoundToInt(o[2]) * 19349663) ^ seed;
		return slot;
	}

	//------------------------------------------------------------------------------------------------
	protected void SortByHash(array<ref RZ_DoorSlot> slots)
	{
		for (int i = 1; i < slots.Count(); i++)
		{
			int j = i;
			while (j > 0 && slots[j - 1].m_iHash > slots[j].m_iHash)
			{
				slots.SwapItems(j - 1, j);
				j--;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Inside a lived-in place's ring: within 50 m of a holding, 80 m of the lab, or 40 m of a
	//! safehouse unless it's the building the safehouse is in (the nearest one, within 30 m).
	protected bool InRing(IEntity b)
	{
		vector o = b.GetOrigin();
		if (m_GM.HasLab() && RZ_Util.DistXZ(o, m_GM.GetLabPosition()) < 80)
			return true;

		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			foreach (RZ_Holding h : territory.GetHoldings())
			{
				if (RZ_Util.DistXZ(h.m_vPos, o) < 50)
					return true;
			}
		}

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses)
			return false;

		int idx = houses.FindNearest(o, 40);
		if (idx < 0)
			return false;

		return RZ_Util.DistXZ(houses.Get(idx).m_vPos, o) > 30;
	}

	//------------------------------------------------------------------------------------------------
	//! What kind of building, from its prefab or mesh name.
	static int KindOf(IEntity b)
	{
		string name = RZ_Util.GetPrefabName(b);
		name.ToLower();
		if (Has(name, {"clinic", "hospital", "pharmacy", "medical", "surgery"}))
			return RZ_EDoorWant.CLINIC;

		if (Has(name, {"shop", "store", "market", "kiosk", "pub", "bar_", "cafe", "restaurant", "grocery"}))
			return RZ_EDoorWant.SHOP;

		if (Has(name, {"garage", "workshop", "autoservice", "carservice", "mechanic"}))
			return RZ_EDoorWant.GARAGE;

		if (Has(name, {"farm", "barn", "stable", "cowshed", "pigsty", "silo", "shed"}))
			return RZ_EDoorWant.FARM;

		if (Has(name, {"school", "church", "chapel", "townhall", "town_hall", "office", "station"}))
			return RZ_EDoorWant.PUBLIC;

		return RZ_EDoorWant.HOUSE;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Has(string name, array<string> words)
	{
		foreach (string w : words)
		{
			if (name.Contains(w))
				return true;
		}

		return false;
	}
}
