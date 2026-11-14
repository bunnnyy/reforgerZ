//! Story groups (docs/DESIGN.md, "Story blocks"): the AI groups a story job adds on top of its base
//! mission's own (Crash Site's recovery team, a keeper's people, carriers). At most 4 on the server
//! and 2 a job, spawned only near a player, entered in the one body budget as story bodies, and
//! deleted when the job ends (RZ_MissionManager). Every caller has a written fallback for when the
//! cap is full or the prefab is missing: Spawn returns null and the job goes on in words.
//! Server only.

//------------------------------------------------------------------------------------------------
class RZ_StoryGroup
{
	IEntity m_Unit;
	int m_iMission;
	string m_sWhat;
}

//------------------------------------------------------------------------------------------------
class RZ_StoryGroups
{
	static const int MAX_SERVER = 4;
	static const int MAX_JOB = 2;
	static const float NEAR_PLAYER = 400;

	protected static ref array<ref RZ_StoryGroup> s_aGroups;

	//------------------------------------------------------------------------------------------------
	protected static array<ref RZ_StoryGroup> Groups()
	{
		if (!s_aGroups)
			s_aGroups = new array<ref RZ_StoryGroup>();

		return s_aGroups;
	}

	//------------------------------------------------------------------------------------------------
	//! Drop the groups that are gone or dead.
	protected static void Prune()
	{
		array<ref RZ_StoryGroup> groups = Groups();
		for (int i = groups.Count() - 1; i >= 0; i--)
		{
			if (!groups[i].m_Unit || RZ_Util.CountAlive(groups[i].m_Unit) <= 0)
				groups.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Live story groups for one job (missionId), or on the server (-1).
	static int Count(int missionId)
	{
		Prune();
		int n = 0;
		foreach (RZ_StoryGroup g : Groups())
		{
			if (missionId < 0 || g.m_iMission == missionId)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Room for one more for this job, and a player within 400 m of pos.
	static bool HasRoom(int missionId, vector pos)
	{
		if (Count(-1) >= MAX_SERVER || Count(missionId) >= MAX_JOB)
			return false;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		return !players.IsEmpty() && RZ_Util.NearestDistance(pos, players) <= NEAR_PLAYER;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn `prefab` (an AI group) at pos for the job and, with moveTo set, send it there. Null when
	//! there's no prefab, no room or no player near: the caller's fallback runs instead.
	static IEntity Spawn(RZ_GameModeComponent gm, RZ_Mission m, ResourceName prefab, vector pos, vector moveTo, string what)
	{
		if (!gm || !m || prefab.IsEmpty() || !HasRoom(m.m_iId, pos))
			return null;

		RZ_Shared shared = gm.GetShared();
		if (shared && !shared.CanSpawnBody(RZ_BodyBudget.STORY, pos))
			return null;

		IEntity unit = RZ_Util.SpawnPrefab(prefab, RZ_Util.GroundPos(pos), Math.RandomFloat(0, 360));
		if (!unit)
			return null;

		if (shared)
			shared.AddBody(RZ_BodyBudget.STORY, unit);

		if (moveTo != vector.Zero && gm.GetSpawner())
			RZ_Util.RedirectGroup(unit, moveTo, gm.GetSpawner().GetMoveWaypointPrefab());

		RZ_StoryGroup g = new RZ_StoryGroup();
		g.m_Unit = unit;
		g.m_iMission = m.m_iId;
		g.m_sWhat = what;
		Groups().Insert(g);
		RZ_Log.Info(string.Format("Story: %1 group '%2' at %3 (%4 on the server)", m.m_sTitle, what, RZ_SettlementScanner.GridReference(pos), Count(-1)));
		return unit;
	}

	//------------------------------------------------------------------------------------------------
	//! The job ended: its groups go.
	static void Release(int missionId)
	{
		array<ref RZ_StoryGroup> groups = Groups();
		for (int i = groups.Count() - 1; i >= 0; i--)
		{
			if (groups[i].m_iMission != missionId)
				continue;

			RZ_Leaving.Remove(groups[i].m_Unit);
			groups.Remove(i);
		}
	}
}
