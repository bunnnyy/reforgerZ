//! Unarmed missions (docs/MISSIONS.md, "Unarmed missions"):
//! low-level contracts about stealth, noise and thinking for new characters and unarmed
//! backgrounds. They use the noisemakers (any loud noise: RZ_HeatMap.FindRecentNoise) and
//! the "being noticed" check (RZ_NoticeTracker), and pay small Fame. This file has the ones where
//! something is carried; RZ_UnarmedTasks.c has the rest.

//------------------------------------------------------------------------------------------------
//! Shared bits for the unarmed missions.
class RZ_UnarmedHelper
{
	static const float LURE_RADIUS = 120;	// a noise this close to the infected a mission steers turns them
	static const float LURE_AGE = 12;		// ...if it's this fresh (s)
	static const float LURE_HOLD = 90;		// they stay at the noise this long before drifting back

	//------------------------------------------------------------------------------------------------
	//! A place to hand something to the Haven near `from`: the nearest safehouse within 4 km,
	//! else the least infected town. False if neither.
	static bool HavenDrop(RZ_GameModeComponent gm, vector from, out vector pos, out string name, out int house)
	{
		house = -1;
		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses)
		{
			int idx = houses.FindNearest(from, 4000);
			RZ_Safehouse h = houses.Get(idx);
			if (h)
			{
				pos = RZ_Mission.LandPointNear(h.m_vPos, 2, 6);
				name = "the safehouse in " + h.m_sName;
				house = idx;
				return true;
			}
		}

		RZ_Settlement s = gm.GetSettlement(RZ_Mission.PickSafestSettlement(gm));
		if (!s)
			return false;

		pos = RZ_Mission.LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.3);
		name = s.m_sName;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A building of one of `kinds` in the town, else a point near its centre.
	static vector BuildingIn(RZ_Settlement s, array<string> kinds, out string what)
	{
		RZ_Landmark l = RZ_LandmarkFinder.FindNearest(s.m_vCenter, s.m_fRadius, kinds);
		if (l)
		{
			what = l.m_sName;
			return RZ_Util.RandomPointInRing(l.m_vPos, 2, 5);
		}

		what = "the house with the red door";
		return RZ_Mission.LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.3);
	}

	//------------------------------------------------------------------------------------------------
	//! Send every unit in `units` to target (drops the dead ones). Adds each group's held waypoint
	//! to `waypoints` once, for the mission to delete later (DeleteUnit also deletes it with the group).
	static void SteerUnits(RZ_GameModeComponent gm, array<IEntity> units, vector target, array<IEntity> waypoints)
	{
		ResourceName wp = gm.GetSpawner().GetMoveWaypointPrefab();
		for (int i = units.Count() - 1; i >= 0; i--)
		{
			IEntity unit = units[i];
			if (!unit)
			{
				units.Remove(i);
				continue;
			}

			IEntity made = RZ_Util.RedirectGroup(unit, target, wp);
			if (made && waypoints && !waypoints.Contains(made))
				waypoints.Insert(made);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Position of the first living unit, false if all are gone.
	static bool UnitsPosition(array<IEntity> units, out vector pos)
	{
		foreach (IEntity unit : units)
		{
			if (unit && RZ_Util.CountAlive(unit) > 0)
			{
				pos = RZ_Util.GetUnitPosition(unit);
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	static void DeleteAll(array<IEntity> list)
	{
		foreach (IEntity e : list)
		{
			RZ_Util.DeleteUnit(e);
		}

		list.Clear();
	}
}
