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
	bool m_bScreamer;		// Aftermath trait (RZ_Adaptation): shrieks when it sees a player
	float m_fShriekAt;		// adaptation clock time of the next allowed shriek
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
	protected float m_fEnvSpecialMult = 1;		// time and weather (RZ_Environment): night
	protected float m_fEnvMinSpawnMult = 1;		// fog: infected may spawn a little closer
	protected bool m_bFogBank;					// the Fog Bank event: closer still inside it
	protected vector m_vFogPos;
	protected float m_fFogRadius;
	protected float m_fFogSpawnMult = 1;
	protected int m_iFocusSettlement = -1;	// gets 1.5x density, every other town 0.4x
	protected ref array<int> m_aSuppressed = {};	// settlements with no spawns at all (The Silent Village)

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
		bool screamer = false;
		if (prefab.IsEmpty())
		{
			// Infection adapts a little (docs/DESIGN.md, "Infection adapts"): one trait per stage
			RZ_Adaptation adapt = m_GM.GetAdaptation();
			if (special && adapt && adapt.RollScreamer())
			{
				screamer = true;
				prefab = adapt.ScreamerPrefab();
			}
			else if (!special && adapt)
			{
				prefab = adapt.PickTraitPrefab();
			}

			if (prefab.IsEmpty() && special && !m_Settings.m_aSpecialPrefabs.IsEmpty())
				prefab = m_Settings.m_aSpecialPrefabs.GetRandomElement();
			else if (prefab.IsEmpty() && !m_Settings.m_aCommonPrefabs.IsEmpty())
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
		z.m_bScreamer = screamer;
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
	//! Time and weather (RZ_Environment): specials at night, closer spawns in fog.
	void SetEnvironmentModifiers(float specialMult, float minSpawnMult)
	{
		m_fEnvSpecialMult = specialMult;
		m_fEnvMinSpawnMult = Math.Clamp(minSpawnMult, 0.3, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! The Fog Bank event (RZ_Environment): inside it, the closest spawn distance is multiplied by mult.
	void SetFogBank(bool on, vector pos, float radius, float mult)
	{
		m_bFogBank = on;
		m_vFogPos = pos;
		m_fFogRadius = radius;
		m_fFogSpawnMult = Math.Clamp(mult, 0.3, 1);
	}

	//------------------------------------------------------------------------------------------------
	void SetFocusSettlement(int idx)
	{
		m_iFocusSettlement = idx;
	}

	//------------------------------------------------------------------------------------------------
	//! No zombies spawn in this settlement while suppressed. Turning it on also removes
	//! the ones already there (not mission zombies).
	void SetSuppressed(int idx, bool suppressed)
	{
		if (idx < 0)
			return;

		if (!suppressed)
		{
			m_aSuppressed.RemoveItem(idx);
			return;
		}

		if (!m_aSuppressed.Contains(idx))
			m_aSuppressed.Insert(idx);

		RZ_Settlement s = m_GM.GetSettlement(idx);
		for (int i = m_aTracked.Count() - 1; i >= 0; i--)
		{
			RZ_TrackedZombie z = m_aTracked[i];
			if (z.m_iMissionId != -1)
				continue;

			bool inside = z.m_iSettlement == idx;
			if (!inside && s && RZ_Util.DistXZ(z.m_vLastPos, s.m_vCenter) <= s.m_fRadius)
				inside = true;

			if (!inside)
				continue;

			RZ_Util.DeleteUnit(z.m_Entity);
			m_aTracked.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	bool IsSuppressedAt(vector pos)
	{
		foreach (int idx : m_aSuppressed)
		{
			RZ_Settlement s = m_GM.GetSettlement(idx);
			if (s && RZ_Util.DistXZ(pos, s.m_vCenter) <= s.m_fRadius + 50)
				return true;
		}

		return false;
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
	//! Living specials (individuals) that belong to a mission (Quiet Count).
	int CountSpecialsForMission(int missionId)
	{
		int n = 0;
		foreach (RZ_TrackedZombie z : m_aTracked)
		{
			if (z.m_iMissionId == missionId && z.m_bSpecial && z.m_Entity)
				n += Math.Max(0, RZ_Util.CountAlive(z.m_Entity));
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! A living special of this mission within radius of pos (last known position).
	bool IsMissionSpecialNear(int missionId, vector pos, float radius)
	{
		foreach (RZ_TrackedZombie z : m_aTracked)
		{
			if (z.m_iMissionId == missionId && z.m_bSpecial && z.m_Entity && RZ_Util.DistXZ(z.m_vLastPos, pos) <= radius)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The units (zombies or groups) a mission owns, for missions that steer them.
	void GetMissionUnits(int missionId, notnull array<IEntity> result)
	{
		foreach (RZ_TrackedZombie z : m_aTracked)
		{
			if (z.m_iMissionId == missionId && z.m_Entity)
				result.Insert(z.m_Entity);
		}
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
	//! Living screamers (Aftermath trait, RZ_Adaptation).
	void GetScreamers(notnull array<RZ_TrackedZombie> result)
	{
		foreach (RZ_TrackedZombie z : m_aTracked)
		{
			if (z.m_bScreamer && z.m_Entity)
				result.Insert(z);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Chance that a spawn is a special: the stage's, times world events, the night
	//! (RZ_Environment), cure breakthroughs (RZ_Adaptation, +15% at most) and the season's rules.
	float GetSpecialChance(RZ_StageSettings stage)
	{
		float chance = stage.m_fSpecialChance * m_fEventSpecialMult * m_fEnvSpecialMult * RZ_Seasons.Mods().m_fSpecialMult;
		if (m_GM.GetAdaptation())
			chance = chance * m_GM.GetAdaptation().SpecialMult();

		return chance;
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
	//! How many living infected the spawner owns are within radius of pos, counting group members.
	//! Stops counting at stopAt (the bite detector only needs to know "3 or more").
	int CountZombiesNear(vector pos, float radius, int stopAt)
	{
		int n = 0;
		array<AIAgent> crowd = {};
		foreach (RZ_TrackedZombie z : m_aTracked)
		{
			if (!z.m_Entity || RZ_Util.DistXZ(z.m_vLastPos, pos) > radius + 60)
				continue;

			SCR_AIGroup group = SCR_AIGroup.Cast(z.m_Entity);
			if (!group)
			{
				if (RZ_Util.DistXZ(z.m_Entity.GetOrigin(), pos) <= radius && RZ_Util.IsCharacterAlive(z.m_Entity))
					n++;
			}
			else
			{
				crowd.Clear();
				group.GetAgents(crowd);
				foreach (AIAgent agent : crowd)
				{
					IEntity member = agent.GetControlledEntity();
					if (member && RZ_Util.DistXZ(member.GetOrigin(), pos) <= radius && RZ_Util.IsCharacterAlive(member))
						n++;
				}
			}

			if (n >= stopAt)
				return n;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Kills up to `cap` living infected the spawner owns within radius of pos (an artillery strike,
	//! RZ_Artillery). Returns how many. Their deaths are counted by the normal tick.
	int KillZombiesNear(vector pos, float radius, int cap)
	{
		int killed = 0;
		array<AIAgent> herd = {};
		foreach (RZ_TrackedZombie z : m_aTracked)
		{
			if (killed >= cap)
				break;

			if (!z.m_Entity || RZ_Util.DistXZ(z.m_vLastPos, pos) > radius + 60)
				continue;

			SCR_AIGroup group = SCR_AIGroup.Cast(z.m_Entity);
			if (!group)
			{
				if (RZ_Util.DistXZ(z.m_Entity.GetOrigin(), pos) <= radius && RZ_Util.IsCharacterAlive(z.m_Entity))
				{
					RZ_Util.KillCharacter(z.m_Entity);
					killed++;
				}

				continue;
			}

			herd.Clear();
			group.GetAgents(herd);
			foreach (AIAgent agent : herd)
			{
				IEntity member = agent.GetControlledEntity();
				if (killed < cap && member && RZ_Util.DistXZ(member.GetOrigin(), pos) <= radius && RZ_Util.IsCharacterAlive(member))
				{
					RZ_Util.KillCharacter(member);
					killed++;
				}
			}
		}

		return killed;
	}

	//------------------------------------------------------------------------------------------------
	//! A living infected character the spawner owns within radius of pos (a group's member), or
	//! null. Spike traps and molotovs use it.
	IEntity FindZombieNear(vector pos, float radius)
	{
		array<AIAgent> members = {};
		foreach (RZ_TrackedZombie z : m_aTracked)
		{
			if (!z.m_Entity || RZ_Util.DistXZ(z.m_vLastPos, pos) > radius + 60)
				continue;

			SCR_AIGroup group = SCR_AIGroup.Cast(z.m_Entity);
			if (!group)
			{
				if (RZ_Util.DistXZ(z.m_Entity.GetOrigin(), pos) <= radius && RZ_Util.IsCharacterAlive(z.m_Entity))
					return z.m_Entity;

				continue;
			}

			members.Clear();
			group.GetAgents(members);
			foreach (AIAgent agent : members)
			{
				IEntity member = agent.GetControlledEntity();
				if (member && RZ_Util.DistXZ(member.GetOrigin(), pos) <= radius && RZ_Util.IsCharacterAlive(member))
					return member;
			}
		}

		return null;
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

			if (m_aSuppressed.Contains(idx))
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

			// A brand-new player nearby gets a gentler first few minutes (RZ_Guidance)
			desired = desired * GentleMult(s.m_vCenter, m_Settings.m_fActivationRadius + s.m_fRadius);

			int deficit = RZ_Util.RoundToInt(desired) - CountForSettlement(idx);
			while (deficit > 0 && spawned < budget && m_aTracked.Count() < m_Settings.m_iGlobalCap)
			{
				vector pos;
				if (!FindSpawnPos(s.m_vCenter, 0, s.m_fRadius, players, pos))
					break;

				// Halcyon's sensors (Install Sensors) draw specials to a town for a while
				float specialChance = GetSpecialChance(stage);
				if (s.m_fSensorSeconds > 0)
					specialChance = Math.Min(0.5, specialChance * 2);

				bool special = Math.RandomFloat01() < specialChance;
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
			int wantedHere = RZ_Util.RoundToInt(wanted * GentleMult(p, 1));
			if (CountNear(p, AMBIENT_RADIUS) >= wantedHere)
				continue;

			vector pos;
			if (!FindSpawnPos(p, m_Settings.m_fMinSpawnDistance + 30, AMBIENT_RADIUS - 50, players, pos))
				continue;

			SpawnZombie(pos, Math.RandomFloat01() < GetSpecialChance(stage) * 0.5, -1, -1);
			spawned++;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Density multiplier from new-player guidance (1 = normal).
	protected float GentleMult(vector pos, float radius)
	{
		if (!m_GM.GetGuidance())
			return 1;

		return m_GM.GetGuidance().GetDensityMult(pos, radius);
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

			float minMult = m_fEnvMinSpawnMult;
			if (m_bFogBank && RZ_Util.DistXZ(pos, m_vFogPos) <= m_fFogRadius)
				minMult = Math.Min(minMult, m_fFogSpawnMult);

			if (RZ_Util.NearestDistance(pos, players) < m_Settings.m_fMinSpawnDistance * minMult)
				continue;

			if (!m_aSuppressed.IsEmpty() && IsSuppressedAt(pos))
				continue;

			// Nothing crawls out of a building that's still smouldering (world interactions, fire)
			RZ_Interactions hub = m_GM.GetInteractions();
			if (hub && hub.IsActive() && hub.GetTraps().IsScorched(pos))
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

	// ================================================================ Mission building blocks
	//------------------------------------------------------------------------------------------------
	//! The first living special a mission owns, or null (live capture).
	RZ_TrackedZombie FindMissionSpecial(int missionId)
	{
		foreach (RZ_TrackedZombie z : m_aTracked)
		{
			if (z.m_iMissionId == missionId && z.m_bSpecial && z.m_Entity && z.m_iLastAlive != 0)
				return z;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Takes a tracked unit out of the world without counting it as killed (no kill event, no
	//! sample): a special caught alive. False if the spawner doesn't own it.
	bool RemoveSilently(IEntity e)
	{
		if (!e)
			return false;

		for (int i = m_aTracked.Count() - 1; i >= 0; i--)
		{
			if (m_aTracked[i].m_Entity != e)
				continue;

			m_aTracked.Remove(i);
			RZ_Util.DeleteUnit(e);
			return true;
		}

		return false;
	}
}
