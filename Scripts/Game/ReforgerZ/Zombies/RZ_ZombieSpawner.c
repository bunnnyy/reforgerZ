//! Which zombie prefabs to use and how many. The prefabs come from whichever zombie
//! mod the server runs (Bacon Zombies by default), so swapping zombie mods means
//! changing this list, not the code. Group prefabs (SCR_AIGroup) are recommended.
[BaseContainerProps()]
class RZ_ZombieSettings
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Common zombies (group or character prefabs)", "et")]
	ref array<ResourceName> m_aCommonPrefabs = {};

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Special zombies. Killing one drops a research sample", "et")]
	ref array<ResourceName> m_aSpecialPrefabs = {};

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Horde groups (large groups for horde events)", "et")]
	ref array<ResourceName> m_aHordePrefabs = {};

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Move waypoint used to send hordes toward noise (e.g. vanilla AIWaypoint_Move)", "et")]
	ResourceName m_sMoveWaypointPrefab;

	[Attribute("4", desc: "Spawns per size class at 100% infection (size 3 city at 100% = 12)")]
	float m_fDensityPerSize = 4;

	[Attribute("600", desc: "Settlements within this distance of a player get populated")]
	float m_fActivationRadius = 600;

	[Attribute("1100", desc: "Zombies farther than this from every player are removed")]
	float m_fDespawnRadius = 1100;

	[Attribute("120", desc: "Never spawn closer than this to a player")]
	float m_fMinSpawnDistance = 120;

	[Attribute("60", desc: "Max tracked spawns on the whole server (performance cap)")]
	int m_iGlobalCap = 60;

	[Attribute("4", desc: "Max spawns per tick")]
	int m_iSpawnsPerTick = 4;

	[Attribute("2", desc: "Wandering zombies kept around each player in the countryside")]
	int m_iAmbientPerPlayer = 2;

	[Attribute("1.5", desc: "Settlement infection removed per zombie kill")]
	float m_fInfectionDropPerKill = 1.5;

	[Attribute("80", desc: "Heat needed in one area to trigger a horde")]
	float m_fHordeHeatThreshold = 80;

	[Attribute("600", desc: "Seconds between hordes")]
	float m_fHordeCooldown = 600;
}

//! Something the spawner created and is keeping an eye on.
class RZ_TrackedZombie
{
	IEntity m_Entity;
	int m_iSettlement = -1;
	int m_iMissionId = -1;
	bool m_bSpecial;
	int m_iLastAlive = 1;
	vector m_vLastPos;
}

//! Spawns zombies only near players and removes them when nobody is around.
//! This keeps the AI count low enough for Reforger servers while the world still
//! feels full. Kills are counted by watching tracked units, so this works
//! with any zombie mod and needs no damage hooks.
class RZ_ZombieSpawner
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_ZombieSettings m_Settings;
	protected ref array<ref RZ_TrackedZombie> m_aTracked = {};
	protected float m_fHordeCooldownLeft = 300;

	// World event modifiers (Night of the Dead, Siren Test). Reset by the event when it ends.
	protected float m_fEventDensityMult = 1;
	protected float m_fEventSpecialMult = 1;
	protected int m_iFocusSettlement = -1;	// gets 1.5x density, every other town 0.4x

	protected static const float AMBIENT_RADIUS = 300;

	//------------------------------------------------------------------------------------------------
	void RZ_ZombieSpawner(RZ_GameModeComponent gm, RZ_ZombieSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;

		if (m_Settings.m_aCommonPrefabs.IsEmpty())
			RZ_Log.Error("No common zombie prefabs configured. Set them on RZ_GameModeComponent > Zombies.");
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<IEntity> players)
	{
		UpdateTracked(players);
		if (players.IsEmpty())
			return;

		RZ_StageSettings stage = m_GM.GetDirector().GetStageSettings();
		int budget = m_Settings.m_iSpawnsPerTick;
		budget -= PopulateSettlements(players, stage, budget);
		if (budget > 0)
			PopulateWilderness(players, stage, budget);

		m_fHordeCooldownLeft -= dt;
		TryHeatHorde();
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn one zombie (or zombie group). Mission-tagged zombies are never despawned
	//! until the mission releases them.
	RZ_TrackedZombie SpawnZombie(vector pos, bool special, int settlementIdx, int missionId, ResourceName prefabOverride = "")
	{
		ResourceName prefab = prefabOverride;
		if (prefab.IsEmpty())
		{
			if (special && !m_Settings.m_aSpecialPrefabs.IsEmpty())
				prefab = m_Settings.m_aSpecialPrefabs.GetRandomElement();
			else if (!m_Settings.m_aCommonPrefabs.IsEmpty())
				prefab = m_Settings.m_aCommonPrefabs.GetRandomElement();
		}

		IEntity ent = RZ_Util.SpawnPrefab(prefab, pos, Math.RandomFloat(0, 360));
		if (!ent)
			return null;

		RZ_TrackedZombie z = new RZ_TrackedZombie();
		z.m_Entity = ent;
		z.m_iSettlement = settlementIdx;
		z.m_iMissionId = missionId;
		z.m_bSpecial = special;
		z.m_vLastPos = pos;
		// Groups spawn their members a frame later, so read the real count on the next tick
		z.m_iLastAlive = -1;
		m_aTracked.Insert(z);
		return z;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn a horde `distance` metres from target and send it there.
	void SpawnHorde(vector target, float distance, int missionId = -1)
	{
		vector pos;
		bool found = false;
		for (int i = 0; i < 8; i++)
		{
			pos = RZ_Util.RandomPointInRing(target, distance * 0.8, distance * 1.2);
			if (!RZ_Util.IsWater(pos))
			{
				found = true;
				break;
			}
		}

		if (!found)
			return;

		SpawnHordeAt(pos, target, missionId);
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn a horde at pos and send it to target. Returns the spawned units.
	void SpawnHordeAt(vector pos, vector target, int missionId = -1, array<IEntity> spawnedOut = null)
	{
		array<IEntity> spawned = {};
		if (!m_Settings.m_aHordePrefabs.IsEmpty())
		{
			RZ_TrackedZombie z = SpawnZombie(pos, false, -1, missionId, m_Settings.m_aHordePrefabs.GetRandomElement());
			if (z)
				spawned.Insert(z.m_Entity);
		}
		else
		{
			for (int j = 0; j < 4; j++)
			{
				RZ_TrackedZombie z2 = SpawnZombie(RZ_Util.RandomPointInRing(pos, 0, 20), false, -1, missionId);
				if (z2)
					spawned.Insert(z2.m_Entity);
			}
		}

		foreach (IEntity e : spawned)
		{
			RZ_Util.SendGroupTo(e, target, m_Settings.m_sMoveWaypointPrefab);
			if (spawnedOut)
				spawnedOut.Insert(e);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetEventModifiers(float densityMult, float specialMult)
	{
		m_fEventDensityMult = densityMult;
		m_fEventSpecialMult = specialMult;
	}

	//------------------------------------------------------------------------------------------------
	void SetFocusSettlement(int idx)
	{
		m_iFocusSettlement = idx;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetMoveWaypointPrefab()
	{
		return m_Settings.m_sMoveWaypointPrefab;
	}

	//------------------------------------------------------------------------------------------------
	//! Tracked units (zombies or groups) still alive that belong to a mission.
	int CountForMission(int missionId)
	{
		int n = 0;
		foreach (RZ_TrackedZombie z : m_aTracked)
		{
			if (z.m_iMissionId == missionId && z.m_Entity)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Positions of living special infected within radius (for the Halcyon sample scanner).
	void GetSpecialsNear(vector pos, float radius, notnull array<vector> result)
	{
		foreach (RZ_TrackedZombie z : m_aTracked)
		{
			if (z.m_bSpecial && z.m_Entity && RZ_Util.DistXZ(z.m_vLastPos, pos) <= radius)
				result.Insert(z.m_vLastPos);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Lets mission zombies despawn normally again.
	void ReleaseMission(int missionId)
	{
		foreach (RZ_TrackedZombie z : m_aTracked)
		{
			if (z.m_iMissionId == missionId)
				z.m_iMissionId = -1;
		}
	}

	//------------------------------------------------------------------------------------------------
	int GetTrackedCount()
	{
		return m_aTracked.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! True if any living zombie the spawner owns is within radius of pos.
	//! Checks every member of a group, not just the leader.
	bool IsZombieNear(vector pos, float radius)
	{
		array<AIAgent> agents = {};
		foreach (RZ_TrackedZombie z : m_aTracked)
		{
			if (!z.m_Entity)
				continue;

			// Cheap reject: a group never spreads out more than ~60 m
			if (RZ_Util.DistXZ(z.m_vLastPos, pos) > radius + 60)
				continue;

			SCR_AIGroup group = SCR_AIGroup.Cast(z.m_Entity);
			if (!group)
			{
				if (RZ_Util.DistXZ(z.m_Entity.GetOrigin(), pos) <= radius && RZ_Util.IsCharacterAlive(z.m_Entity))
					return true;

				continue;
			}

			agents.Clear();
			group.GetAgents(agents);
			foreach (AIAgent agent : agents)
			{
				IEntity member = agent.GetControlledEntity();
				if (member && RZ_Util.DistXZ(member.GetOrigin(), pos) <= radius && RZ_Util.IsCharacterAlive(member))
					return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateTracked(array<IEntity> players)
	{
		for (int i = m_aTracked.Count() - 1; i >= 0; i--)
		{
			RZ_TrackedZombie z = m_aTracked[i];

			// Entity is gone. Empty groups delete themselves when the last member dies,
			// so count whatever was still alive as killed.
			if (!z.m_Entity)
			{
				if (z.m_iLastAlive > 0)
					m_GM.OnZombiesKilled(z.m_iSettlement, z.m_iMissionId, z.m_bSpecial, z.m_vLastPos, z.m_iLastAlive);

				m_aTracked.Remove(i);
				continue;
			}

			int alive = RZ_Util.CountAlive(z.m_Entity);
			if (z.m_iLastAlive < 0)
				z.m_iLastAlive = alive;

			if (alive < z.m_iLastAlive)
			{
				m_GM.OnZombiesKilled(z.m_iSettlement, z.m_iMissionId, z.m_bSpecial, z.m_vLastPos, z.m_iLastAlive - alive);
				z.m_iLastAlive = alive;
			}

			if (alive <= 0)
			{
				m_aTracked.Remove(i);
				continue;
			}

			z.m_vLastPos = RZ_Util.GetUnitPosition(z.m_Entity);

			if (z.m_iMissionId == -1 && RZ_Util.NearestDistance(z.m_vLastPos, players) > m_Settings.m_fDespawnRadius)
			{
				RZ_Util.DeleteUnit(z.m_Entity);
				m_aTracked.Remove(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected int PopulateSettlements(array<IEntity> players, RZ_StageSettings stage, int budget)
	{
		int spawned = 0;
		array<ref RZ_Settlement> settlements = m_GM.GetSettlements();
		foreach (int idx, RZ_Settlement s : settlements)
		{
			if (spawned >= budget || m_aTracked.Count() >= m_Settings.m_iGlobalCap)
				break;

			if (RZ_Util.NearestDistance(s.m_vCenter, players) > m_Settings.m_fActivationRadius + s.m_fRadius)
				continue;

			float desired = s.m_iSize * m_Settings.m_fDensityPerSize * (s.m_fInfection / 100) * stage.m_fDensityMult * m_fEventDensityMult;
			desired += m_GM.GetHeat().GetHeat(s.m_vCenter) / 25;
			if (m_iFocusSettlement >= 0)
			{
				if (idx == m_iFocusSettlement)
					desired = desired * 1.5;
				else
					desired = desired * 0.4;
			}

			int deficit = RZ_Util.RoundToInt(desired) - CountForSettlement(idx);
			while (deficit > 0 && spawned < budget && m_aTracked.Count() < m_Settings.m_iGlobalCap)
			{
				vector pos;
				if (!FindSpawnPos(s.m_vCenter, 0, s.m_fRadius, players, pos))
					break;

				bool special = Math.RandomFloat01() < stage.m_fSpecialChance * m_fEventSpecialMult;
				SpawnZombie(pos, special, idx, -1);
				deficit--;
				spawned++;
			}
		}

		return spawned;
	}

	//------------------------------------------------------------------------------------------------
	protected void PopulateWilderness(array<IEntity> players, RZ_StageSettings stage, int budget)
	{
		int wanted = RZ_Util.RoundToInt(m_Settings.m_iAmbientPerPlayer * stage.m_fDensityMult * m_fEventDensityMult);
		int spawned = 0;
		foreach (IEntity player : players)
		{
			if (spawned >= budget || m_aTracked.Count() >= m_Settings.m_iGlobalCap)
				return;

			vector p = player.GetOrigin();
			if (CountNear(p, AMBIENT_RADIUS) >= wanted)
				continue;

			vector pos;
			if (!FindSpawnPos(p, m_Settings.m_fMinSpawnDistance + 30, AMBIENT_RADIUS - 50, players, pos))
				continue;

			SpawnZombie(pos, Math.RandomFloat01() < stage.m_fSpecialChance * m_fEventSpecialMult * 0.5, -1, -1);
			spawned++;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void TryHeatHorde()
	{
		if (m_fHordeCooldownLeft > 0)
			return;

		vector hotPos;
		float heat;
		if (!m_GM.GetHeat().GetHottest(hotPos, heat) || heat < m_Settings.m_fHordeHeatThreshold)
			return;

		SpawnHorde(hotPos, 450);
		m_GM.GetHeat().AddHeat(hotPos, -heat * 0.5);
		m_fHordeCooldownLeft = m_Settings.m_fHordeCooldown;
		m_GM.NotifyHorde(hotPos);
	}

	//------------------------------------------------------------------------------------------------
	protected bool FindSpawnPos(vector center, float minR, float maxR, array<IEntity> players, out vector pos)
	{
		for (int i = 0; i < 6; i++)
		{
			pos = RZ_Util.RandomPointInRing(center, minR, maxR);
			if (RZ_Util.IsWater(pos))
				continue;

			if (RZ_Util.NearestDistance(pos, players) < m_Settings.m_fMinSpawnDistance)
				continue;

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected int CountForSettlement(int idx)
	{
		int n = 0;
		foreach (RZ_TrackedZombie z : m_aTracked)
		{
			if (z.m_iSettlement == idx)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected int CountNear(vector pos, float radius)
	{
		int n = 0;
		foreach (RZ_TrackedZombie z : m_aTracked)
		{
			if (RZ_Util.DistXZ(z.m_vLastPos, pos) <= radius)
				n++;
		}

		return n;
	}
}
