//! Small lookups the newer contracts share (safehouses, fuel, messages near a spot).
class RZ_ContractHelp
{
	//------------------------------------------------------------------------------------------------
	static int CountSafehouses(RZ_GameModeComponent gm)
	{
		if (!gm.GetSafehouses())
			return 0;

		return gm.GetSafehouses().Count();
	}

	//------------------------------------------------------------------------------------------------
	static RZ_Safehouse Safehouse(RZ_GameModeComponent gm, int idx)
	{
		if (!gm.GetSafehouses())
			return null;

		return gm.GetSafehouses().Get(idx);
	}

	//------------------------------------------------------------------------------------------------
	//! A random safehouse between minDist and maxDist from `from`, -1 if none.
	static int PickSafehouse(RZ_GameModeComponent gm, vector from, float minDist, float maxDist)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses)
			return -1;

		array<int> options = {};
		for (int i = 0; i < houses.Count(); i++)
		{
			float d = RZ_Util.DistXZ(from, houses.Get(i).m_vPos);
			if (d >= minDist && d <= maxDist)
				options.Insert(i);
		}

		if (options.IsEmpty())
			return -1;

		return options.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! A random safehouse whose town no running mission is using, -1 if none.
	static int PickFreeSafehouse(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses)
			return -1;

		array<int> options = {};
		for (int i = 0; i < houses.Count(); i++)
		{
			if (!gm.GetMissions().IsSettlementBusy(houses.Get(i).m_iSettlement))
				options.Insert(i);
		}

		if (options.IsEmpty())
			return -1;

		return options.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! What marks a body or a corpse pile (Burn the Bodies, Memorial): Missions > Corpse pile, else
	//! Puzzles > Puzzle object. "" if neither is set (those missions then never appear).
	static ResourceName CorpsePilePrefab(RZ_GameModeComponent gm)
	{
		ResourceName prefab = gm.GetCorpsePilePrefab();
		if (prefab.IsEmpty())
			prefab = gm.GetPuzzlePrefab(RZ_EPuzzleProp.OBJECT);

		return prefab;
	}

	//------------------------------------------------------------------------------------------------
	//! Fuel can keywords (Interactions > Cars).
	static string FuelWords(RZ_GameModeComponent gm)
	{
		RZ_CarSettings cs = gm.GetCarSettings();
		if (cs)
			return cs.m_sFuelWords;

		return "jerrycan,jerry_can,fuelcan,fuel_can,canister";
	}

	//------------------------------------------------------------------------------------------------
	//! A message for everyone within radius of pos.
	static void NotifyNear(RZ_GameModeComponent gm, vector pos, float radius, string title, string text)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius)
				gm.NotifyPlayer(ids[i], title, text);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Adds the players within radius of pos to `list`, each once (participants).
	static void AddNear(array<int> ids, array<IEntity> players, vector pos, float radius, notnull array<int> list)
	{
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius && !list.Contains(ids[i]))
				list.Insert(ids[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	static bool AnyNear(array<IEntity> players, vector pos, float radius)
	{
		return RZ_Util.NearestDistance(pos, players) <= radius;
	}

	//------------------------------------------------------------------------------------------------
	//! A point partway from a to b, on land (for routes).
	static vector Between(vector a, vector b, float share)
	{
		vector p = a + (b - a) * share;
		return RZ_Mission.LandPointNear(RZ_Util.GroundPos(p), 0, 40);
	}

	//------------------------------------------------------------------------------------------------
	//! Is one of this mission's items in the player's inventory? Takes it if `take`.
	static bool CarriesItemOf(IEntity user, int missionId, bool take)
	{
		if (!user)
			return false;

		array<IEntity> items = {};
		RZ_MissionItemHelper.FindItems(user, items);
		foreach (IEntity item : items)
		{
			RZ_MissionItemComponent comp = RZ_MissionItemComponent.Cast(item.FindComponent(RZ_MissionItemComponent));
			if (!comp || comp.GetMissionId() != missionId)
				continue;

			if (take)
				RZ_MissionItemHelper.RemoveItem(user, item);

			return true;
		}

		return false;
	}
}

//------------------------------------------------------------------------------------------------
//! A spawned unit (a character or an AI group) a mission needs alive. An SCR_AIGroup deletes
//! itself when its last member dies, so a unit that was spawned and is now gone counts as dead.
//! Group members appear a moment after the group: it isn't judged until they've been seen alive
//! (or 15 s have passed). Call IsDead once per tick.
class RZ_UnitWatch
{
	IEntity m_Unit;
	protected bool m_bSpawned;
	protected bool m_bSeenAlive;
	protected float m_fAge;

	//------------------------------------------------------------------------------------------------
	//! Spawns it (once). False if the prefab is empty or invalid.
	bool Spawn(ResourceName prefab, vector pos, float yaw = 0)
	{
		m_Unit = RZ_Util.SpawnPrefab(prefab, pos, yaw);
		m_bSpawned = m_Unit != null;
		m_bSeenAlive = false;
		m_fAge = 0;
		return m_bSpawned;
	}

	//------------------------------------------------------------------------------------------------
	bool IsSpawned()
	{
		return m_bSpawned;
	}

	//------------------------------------------------------------------------------------------------
	//! True once everyone in it is dead (or it was deleted). False before it was spawned.
	bool IsDead(float dt)
	{
		if (!m_bSpawned)
			return false;

		if (!m_Unit)
			return true;

		m_fAge += dt;
		int alive = RZ_Util.CountAlive(m_Unit);
		if (alive > 0)
			m_bSeenAlive = true;

		return alive == 0 && (m_bSeenAlive || m_fAge > 15);
	}

	//------------------------------------------------------------------------------------------------
	vector GetPos(vector fallback)
	{
		if (!m_Unit)
			return fallback;

		return RZ_Util.GetUnitPosition(m_Unit);
	}

	//------------------------------------------------------------------------------------------------
	void Delete()
	{
		RZ_Util.DeleteUnit(m_Unit);
		m_Unit = null;
	}
}
