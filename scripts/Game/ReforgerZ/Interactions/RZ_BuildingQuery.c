//! Buildings near a point, and the spots mission props must keep out of (docs/DESIGN.md,
//! "Locks and keys"). Mission items, props, people and crates are placed on random ground
//! points; a point inside a locked building would put them behind a door nobody can open
//! quietly. RZ_LockedSpots.IsBlocked() is checked by the shared spot pickers
//! (RZ_Mission.LandPointNear and HighPointNear) and by the landmark finder.

//------------------------------------------------------------------------------------------------
//! Building entities within a sphere. Runs once per call, over static entities only.
class RZ_BuildingQuery
{
	protected ref array<IEntity> m_aFound = {};
	protected int m_iMax = 64;

	//------------------------------------------------------------------------------------------------
	//! Buildings (destructible buildings, or anything under /buildings/) within radius of center.
	static void FindBuildings(vector center, float radius, notnull array<IEntity> result, int max = 64)
	{
		RZ_BuildingQuery q = new RZ_BuildingQuery();
		q.Run(center, radius, max);
		foreach (IEntity e : q.m_aFound)
		{
			result.Insert(e);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Run(vector center, float radius, int max)
	{
		m_iMax = max;
		GetGame().GetWorld().QueryEntitiesBySphere(center, radius, OnEntity, null, EQueryEntitiesFlags.STATIC);
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnEntity(IEntity e)
	{
		if (!e || m_aFound.Count() >= m_iMax)
			return true;

		if (SCR_DestructibleBuildingEntity.Cast(e))
		{
			m_aFound.Insert(e);
			return true;
		}

		string name = RZ_ItemHelper.LowerName(e);
		if (name.Contains("/buildings/"))
			m_aFound.Insert(e);

		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! Spots that are behind a lock: inside (or right against) a locked building, or by one of its
//! doorway blockers. Only while locks are on.
class RZ_LockedSpots
{
	protected static const float QUERY_RADIUS = 30;	// buildings whose origin is this close are checked
	protected static const float MARGIN = 1.5;			// metres around a locked building's box (its doorways)
	protected static const float BLOCKER_CLEAR = 2.5;	// metres from a standing doorway blocker
	protected static const float NO_BOX_CLEAR = 6;		// no usable box: keep this far from its origin

	//------------------------------------------------------------------------------------------------
	protected static RZ_Interactions Hub()
	{
		RZ_Interactions hub = RZ_Interactions.Get();
		if (!hub || !hub.GetDoors() || !hub.GetDoors().IsOn())
			return null;

		return hub;
	}

	//------------------------------------------------------------------------------------------------
	//! A building with a lock (seeded, not burned). Counted even while someone has it open: it
	//! locks again at the reset.
	static bool IsLockedBuilding(IEntity building)
	{
		RZ_Interactions hub = Hub();
		if (!hub || !building)
			return false;

		string name = RZ_ItemHelper.LowerName(building);
		if (!SCR_DestructibleBuildingEntity.Cast(building) && !name.Contains("/buildings/"))
			return false;

		vector pos = building.GetOrigin();
		RZ_Search search = hub.GetSearch();
		if (!search.HasLock(pos, search.BuildingType(name)))
			return false;

		return !hub.GetTraps().IsBurned(pos);
	}

	//------------------------------------------------------------------------------------------------
	//! In the building's box grown by `margin` (any height). Without a usable box, near its origin.
	static bool InFootprint(IEntity building, vector worldPos, float margin)
	{
		vector mins;
		vector maxs;
		if (!RZ_DoorGeometry.LocalBox(building, mins, maxs))
			return RZ_Util.DistXZ(building.GetOrigin(), worldPos) <= NO_BOX_CLEAR;

		vector p = RZ_DoorGeometry.ToLocal(building, worldPos);
		if (p[0] < mins[0] - margin || p[0] > maxs[0] + margin)
			return false;

		return p[2] >= mins[2] - margin && p[2] <= maxs[2] + margin;
	}

	//------------------------------------------------------------------------------------------------
	//! The locked building whose footprint holds pos, null if none.
	static IEntity BlockingBuilding(vector pos)
	{
		RZ_Interactions hub = Hub();
		if (!hub)
			return null;

		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(pos, QUERY_RADIUS, buildings);
		foreach (IEntity b : buildings)
		{
			if (IsLockedBuilding(b) && InFootprint(b, pos, MARGIN))
				return b;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! A spot a mission prop, item or person must not use: behind a lock, or by a doorway blocker.
	static bool IsBlocked(vector pos)
	{
		RZ_Interactions hub = Hub();
		if (!hub)
			return false;

		if (hub.GetDoors().IsNearBlocker(pos, BLOCKER_CLEAR))
			return true;

		return BlockingBuilding(pos) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! pos, moved out past the nearest wall of the locked building that holds it (on the ground).
	//! pos unchanged if it isn't blocked by a building.
	static vector MoveOut(vector pos)
	{
		IEntity b = BlockingBuilding(pos);
		if (!b)
			return pos;

		vector mins;
		vector maxs;
		if (!RZ_DoorGeometry.LocalBox(b, mins, maxs))
		{
			vector away = pos - b.GetOrigin();
			away[1] = 0;
			float len = away.Length();
			if (len < 0.1)
				away = Vector(1, 0, 0);
			else
				away = away * (1 / len);

			return RZ_Util.GroundPos(b.GetOrigin() + away * (NO_BOX_CLEAR + 1));
		}

		vector p = RZ_DoorGeometry.ToLocal(b, pos);
		float clear = MARGIN + 1;
		float toMinX = p[0] - mins[0];
		float toMaxX = maxs[0] - p[0];
		float toMinZ = p[2] - mins[2];
		float toMaxZ = maxs[2] - p[2];
		float best = Math.Min(Math.Min(toMinX, toMaxX), Math.Min(toMinZ, toMaxZ));
		if (best == toMinX)
			p[0] = mins[0] - clear;
		else if (best == toMaxX)
			p[0] = maxs[0] + clear;
		else if (best == toMinZ)
			p[2] = mins[2] - clear;
		else
			p[2] = maxs[2] + clear;

		return RZ_Util.GroundPos(RZ_DoorGeometry.ToWorld(b, p));
	}
}
