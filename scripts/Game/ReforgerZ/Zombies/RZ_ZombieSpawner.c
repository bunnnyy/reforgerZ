//! Which zombie prefabs to use and how many. The prefabs come from whichever zombie
//! mod the server runs (Bacon Zombies by default), so swapping zombie mods means
//! changing this list, not the code. Group prefabs (SCR_AIGroup) are recommended.
[BaseContainerProps()]
class RZ_ZombieSettings
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Common zombies (group or character prefabs)", "et")]
	ref array<ResourceName> m_aCommonPrefabs;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Special zombies. Killing one drops a research sample", "et")]
	ref array<ResourceName> m_aSpecialPrefabs;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Horde groups (large groups for horde events)", "et")]
	ref array<ResourceName> m_aHordePrefabs;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Move waypoint used to send hordes toward noise (e.g. vanilla AIWaypoint_Move)", "et")]
	ResourceName m_sMoveWaypointPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Empty group (runtime groups): an AI group prefab with no members of its own (Bacon's Group_Zombies_Base). Crawlers and loose ones made from character prefabs are put in one, so they can walk somewhere. Empty: character prefabs spawn on their own (Bacon's then stay inactive)", "et")]
	ResourceName m_sEmptyGroupPrefab;

	[Attribute("4", desc: "Spawns per size class at 100% infection (size 3 city at 100% = 12)")]
	float m_fDensityPerSize;

	[Attribute("600", desc: "Settlements within this distance of a player get populated")]
	float m_fActivationRadius;

	[Attribute("1100", desc: "Zombies farther than this from every player are removed")]
	float m_fDespawnRadius;

	[Attribute("120", desc: "Never spawn closer than this to a player")]
	float m_fMinSpawnDistance;

	[Attribute("60", desc: "Max tracked spawns on the whole server (performance cap)")]
	int m_iGlobalCap;

	[Attribute("4", desc: "Max spawns per tick")]
	int m_iSpawnsPerTick;

	[Attribute("2", desc: "Wandering zombies kept around each player in the countryside")]
	int m_iAmbientPerPlayer;

	[Attribute("1.5", desc: "Settlement infection removed per zombie kill")]
	float m_fInfectionDropPerKill;

	[Attribute("80", desc: "Heat needed in one area to trigger a horde")]
	float m_fHordeHeatThreshold;

	[Attribute("600", desc: "Seconds between hordes")]
	float m_fHordeCooldown;
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
	bool m_bHorde;			// spawned as a horde (its own base senses, RZ_Stealth)
	bool m_bRelentless;		// must keep coming (a mission's horde): stealth never lowers its senses or makes it lose anyone
	vector m_vHome;			// where it spawned: where it goes back to when it gives up a search
	vector m_vGoal;			// where a horde was sent
	bool m_bHasGoal;
	ref RZ_SenseState m_Sense;	// stealth's view of it (Zombies/RZ_Stealth.c), once a player was near
	ref RZ_HeldWaypoint m_Held;	// a group's one move waypoint (Core/RZ_GroupOrders.c): every order moves it; deleted when the record goes

	// With Zombies > Crawlers or Tribes on (docs/INFECTED.md, "Bodies"): what it's for, the data it
	// writes back to, and each member, so the code knows who died where
	RZ_EInfectedRole m_eRole;	// NONE: as before (missions, specials, the old roamers)
	int m_iTribe = -1;			// the tribe it belongs to (Zombies/RZ_Tribes.c), -1 none
	bool m_bHunting;			// a tribe's hunt on its trail: stealth never makes it lose anyone
	ref RZ_GroupPlan m_Plan;	// a tribe group's plan in the behaviour layer (Zombies/Behaviour), or null
	RZ_InfectedPart m_Part;		// weak: the data part it writes back to, or null
	ResourceName m_sPrefab;
	bool m_bRuntimeGroup;		// a character prefab put in an empty group (RZ_InfectedBodies.WrapInGroup)
	float m_fBornAt;			// the spawner's clock
	int m_iAgents = -1;			// living members, -1 until read
	ref array<IEntity> m_aMembers;	// null: no per-member tracking (both switches off)
	ref array<vector> m_aMemberPos;
	ref array<int> m_aMemberIds;
	ref array<bool> m_aMemberDone;
	ref array<int> m_aSilentIds;	// members marked for a silent kill (no samples, infection, ledger or credit)
	// Crawlers: drift to noise, walking somewhere, a siege at a locked door
	int m_iHeardNoise = -1;
	bool m_bDrifting;
	float m_fDriftBackAt;
	bool m_bWalking;
	vector m_vWalkTo;
	int m_iWalkTown = -1;
	float m_fSiegeSince;
	float m_fSiegeLast;
	vector m_vSiegeAt;
	// Crawler life (Zombies/Behaviour/RZ_CrawlerLife.c)
	bool m_bAsleep;			// lying up with its AI off
	float m_fWokeAt;
	bool m_bHerdSlots;		// its members walk to herd slots
	bool m_bCapped;			// the crawler speed cap is on its members

	//------------------------------------------------------------------------------------------------
	//! Living agents for the budget: members once read, else an estimate.
	int Agents()
	{
		if (m_iAgents >= 0)
			return m_iAgents;

		return 4;
	}
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

	// World event modifiers (Every Ridge Calls, Siren Test). Reset by the event when it ends.
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
	protected static const float HORDE_PLAYER_RANGE = 600;	// heat hordes only go to squares this close to a living player

	// Crawlers and tribes (docs/INFECTED.md). With both off none of this runs
	protected ref RZ_Crawlers m_Crawlers;
	protected RZ_TribeSettings m_TribeSettings;
	protected ref RZ_InfectedBudget m_Budget = new RZ_InfectedBudget();
	protected ref RZ_BodyBudget m_Bodies;	// the one body budget: everything else's bodies, inside m_Budget
	protected ref map<string, int> m_mPrefabAgents = new map<string, int>();	// group prefabs: members seen
	protected float m_fClock;
	protected int m_iTickAgents;
	protected int m_iHordesFromTowns;
	protected int m_iLooseArrived;

	protected static const float LOOSE_GOAL_RANGE = 3000;	// loose ones walk to a town or a lair within this
	protected static const float HORDE_TOWN_RANGE = 1500;	// a heat horde comes from a town this close
	protected static const float SIEGE_GAP = 60;			// banging stopped this long: the siege is over
	protected static const float MEMBER_WAIT = 30;			// a group prefab may take this long to fill

	//------------------------------------------------------------------------------------------------
	void RZ_ZombieSpawner(RZ_GameModeComponent gm, RZ_ZombieSettings settings, RZ_CrawlerSettings crawlers = null, RZ_TribeSettings tribes = null)
	{
		m_GM = gm;
		m_Settings = settings;
		m_TribeSettings = tribes;
		m_Bodies = new RZ_BodyBudget(m_Budget);
		if (crawlers)
			m_Crawlers = new RZ_Crawlers(gm, crawlers);

		if (m_Settings.m_aCommonPrefabs.IsEmpty())
			RZ_Log.Error("No common zombie prefabs configured. Set them on RZ_GameModeComponent > Zombies.");
	}

	//------------------------------------------------------------------------------------------------
	bool CrawlersOn() { return m_Crawlers && m_Crawlers.IsEnabled(); }
	bool TribesOn() { return m_TribeSettings && m_TribeSettings.m_bEnabled; }
	//! Crawlers or tribes: the agent budget, per-member tracking, the sight rule. Both off: the
	//! spawner exactly as before.
	bool IsPlumbingOn() { return CrawlersOn() || TribesOn(); }
	RZ_Crawlers GetCrawlers() { return m_Crawlers; }
	RZ_ZombieSettings GetSettings() { return m_Settings; }
	RZ_InfectedBudget GetBudget() { return m_Budget; }
	RZ_BodyBudget GetBodies() { return m_Bodies; }
	float GetDensityPerSize() { return m_Settings.m_fDensityPerSize; }

	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<IEntity> players)
	{
		m_Bodies.Tick(players, IsPlumbingOn());
		if (IsPlumbingOn())
		{
			TickInfected(dt, players);
			return;
		}

		UpdateTracked(players);
		if (players.IsEmpty())
			return;

		RZ_StageSettings stage = m_GM.GetDirector().GetStageSettings();
		int budget = m_Settings.m_iSpawnsPerTick;
		budget -= PopulateSettlements(players, stage, budget);
		if (budget > 0)
			PopulateWilderness(players, stage, budget);

		m_fHordeCooldownLeft -= dt;
		TryHeatHorde(players);
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

		RZ_TrackedZombie z = Track(ent, pos, special, settlementIdx, missionId, prefab);
		z.m_bScreamer = screamer;
		return z;
	}

	//------------------------------------------------------------------------------------------------
	//! A new tracked record for a spawned unit.
	protected RZ_TrackedZombie Track(IEntity ent, vector pos, bool special, int settlementIdx, int missionId, ResourceName prefab)
	{
		RZ_TrackedZombie z = new RZ_TrackedZombie();
		z.m_Entity = ent;
		z.m_iSettlement = settlementIdx;
		z.m_iMissionId = missionId;
		z.m_bSpecial = special;
		z.m_vLastPos = pos;
		z.m_vHome = pos;
		z.m_sPrefab = prefab;
		z.m_fBornAt = m_fClock;
		// Its group's held waypoint (none spawned until the first order)
		z.m_Held = RZ_GroupOrders.Hold(ent);
		// Groups spawn their members a frame later, so read the real count on the next tick
		z.m_iLastAlive = -1;
		if (IsPlumbingOn())
		{
			z.m_aMembers = {};
			z.m_aMemberPos = {};
			z.m_aMemberIds = {};
			z.m_aMemberDone = {};
			z.m_iAgents = EstimateAgents(ent, prefab);
		}

		m_aTracked.Insert(z);
		return z;
	}

	//------------------------------------------------------------------------------------------------
	//! Agents a fresh unit will have: a character 1, a group prefab what it had last time (4 until
	//! seen once), a runtime group its members.
	protected int EstimateAgents(IEntity ent, ResourceName prefab)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(ent);
		if (!group)
			return 1;

		if (group.GetAgentsCount() > 0)
			return group.GetAgentsCount();

		int seen;
		if (m_mPrefabAgents.Find(prefab, seen) && seen > 0)
			return seen;

		return 4;
	}

	//------------------------------------------------------------------------------------------------
	//! One unit for the crawlers and loose ones: a group prefab as it is, or `size` characters of a
	//! character prefab in a runtime group (Zombies > Empty group). Empty prefab: Common.
	RZ_TrackedZombie SpawnUnit(vector pos, ResourceName prefab, int size, int settlementIdx, RZ_EInfectedRole role)
	{
		if (prefab.IsEmpty())
		{
			if (m_Settings.m_aCommonPrefabs.IsEmpty())
				return null;

			prefab = m_Settings.m_aCommonPrefabs.GetRandomElement();
		}

		float yaw = Math.RandomFloat(0, 360);
		IEntity first = RZ_Util.SpawnPrefab(prefab, pos, yaw);
		if (!first)
			return null;

		IEntity unit = first;
		bool runtime = false;
		if (!SCR_AIGroup.Cast(first) && ChimeraCharacter.Cast(first))
		{
			SCR_AIGroup group = RZ_InfectedBodies.WrapInGroup(first, m_Settings.m_sEmptyGroupPrefab);
			if (group)
			{
				unit = group;
				runtime = true;
				for (int i = 1; i < size; i++)
				{
					IEntity more = RZ_Util.SpawnPrefab(prefab, RZ_Util.GroundPos(RZ_Util.RandomPointInRing(pos, 1.5, 6)), yaw);
					RZ_InfectedBodies.AddToGroup(group, more);
				}
			}
		}

		RZ_TrackedZombie z = Track(unit, pos, false, settlementIdx, -1, prefab);
		z.m_eRole = role;
		z.m_bRuntimeGroup = runtime;
		m_Budget.Add(pos, z.Agents());
		m_iTickAgents += z.Agents();
		return z;
	}

	//------------------------------------------------------------------------------------------------
	//! A tribe's group (Zombies/RZ_TribeBodies.c): `count` characters, the first `young` of them from
	//! youngPrefabs, in a runtime group, each steered and blind from its spawn frame (the behaviour
	//! layer, lab B1) before and after it joins. A group prefab spawns as it is (not steered). Null
	//! if nothing spawned.
	RZ_TrackedZombie SpawnTribeUnit(vector pos, array<ResourceName> prefabs, array<ResourceName> youngPrefabs, int count, int young, RZ_EInfectedRole role, int tribe, RZ_EBodyKind kind)
	{
		if (!prefabs || prefabs.IsEmpty())
			return null;

		float yaw = Math.RandomFloat(0, 360);
		ResourceName firstPrefab = prefabs.GetRandomElement();
		RZ_EBodyKind firstKind = kind;
		if (young > 0 && youngPrefabs && !youngPrefabs.IsEmpty())
		{
			firstPrefab = youngPrefabs.GetRandomElement();
			firstKind = RZ_EBodyKind.YOUNG;
		}

		IEntity first = RZ_Util.SpawnPrefab(firstPrefab, pos, yaw);
		if (!first)
			return null;

		IEntity unit = first;
		bool runtime = false;
		if (!SCR_AIGroup.Cast(first) && ChimeraCharacter.Cast(first))
		{
			RZ_Behaviour.Blind(first, firstKind);
			SCR_AIGroup group = RZ_InfectedBodies.WrapInGroup(first, m_Settings.m_sEmptyGroupPrefab);
			RZ_Behaviour.Blind(first, firstKind);
			if (group)
			{
				unit = group;
				runtime = true;
				for (int i = 1; i < count; i++)
				{
					ResourceName prefab = prefabs.GetRandomElement();
					RZ_EBodyKind memberKind = kind;
					if (i < young && youngPrefabs && !youngPrefabs.IsEmpty())
					{
						prefab = youngPrefabs.GetRandomElement();
						memberKind = RZ_EBodyKind.YOUNG;
					}

					IEntity more = RZ_Util.SpawnPrefab(prefab, RZ_Util.GroundPos(RZ_Util.RandomPointInRing(pos, 3, 7)), yaw);
					RZ_Behaviour.Blind(more, memberKind);
					if (RZ_InfectedBodies.AddToGroup(group, more))
						RZ_Behaviour.Blind(more, memberKind);
				}
			}
		}

		RZ_TrackedZombie z = Track(unit, pos, false, -1, -1, firstPrefab);
		z.m_eRole = role;
		z.m_iTribe = tribe;
		z.m_bRuntimeGroup = runtime;
		if (runtime)
			z.m_iAgents = count;

		m_Budget.Add(pos, z.Agents());
		m_iTickAgents += z.Agents();
		return z;
	}

	//------------------------------------------------------------------------------------------------
	//! Agents that may spawn at pos this tick: the cluster's room, the server's, the tick's, and the
	//! tracked cap. 0 with both switches off.
	int SpawnRoom(vector pos)
	{
		if (!IsPlumbingOn() || m_aTracked.Count() >= m_Settings.m_iGlobalCap)
			return 0;

		return Math.Min(m_Budget.RoomAt(pos), TickRoom());
	}

	//------------------------------------------------------------------------------------------------
	//! The closest a body may spawn to a player here (x0.75 in fog, less in a Fog Bank).
	float MinSpawnDistance(vector pos)
	{
		float minMult = m_fEnvMinSpawnMult;
		if (m_bFogBank && RZ_Util.DistXZ(pos, m_vFogPos) <= m_fFogRadius)
			minMult = Math.Min(minMult, m_fFogSpawnMult);

		return m_Settings.m_fMinSpawnDistance * minMult;
	}

	//------------------------------------------------------------------------------------------------
	//! Stops tracking m_aTracked[i]: its group's held waypoint goes with the record.
	protected void Untrack(int i)
	{
		RZ_GroupOrders.Drop(m_aTracked[i].m_Held);
		m_aTracked.Remove(i);
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn a horde `distance` metres from target and send it there. False if nothing spawned.
	bool SpawnHorde(vector target, float distance, int missionId = -1)
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
			return false;

		return SpawnHordeAt(pos, target, missionId);
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn a horde at pos and send it to target. The spawned units go into spawnedOut; false if none.
	//! A mission's horde must keep coming: stealth never lowers its senses (docs/DESIGN.md, "Stealth").
	//! keepComing false: a horde where staying unseen is the point (Lead Them Away, The Clockwork
	//! Horde, The Tribe Moves). prefabs: a pack's own bodies (RZ_TribePeople.PackHordePrefabs), four of
	//! them, instead of the horde prefabs.
	bool SpawnHordeAt(vector pos, vector target, int missionId = -1, array<IEntity> spawnedOut = null, bool keepComing = true, array<ResourceName> prefabs = null)
	{
		array<RZ_TrackedZombie> spawned = {};
		if (prefabs && !prefabs.IsEmpty())
		{
			for (int k = 0; k < 4; k++)
			{
				RZ_TrackedZombie zp = SpawnZombie(RZ_Util.RandomPointInRing(pos, 0, 20), false, -1, missionId, prefabs.GetRandomElement());
				if (zp)
					spawned.Insert(zp);
			}
		}
		else if (!m_Settings.m_aHordePrefabs.IsEmpty())
		{
			RZ_TrackedZombie z = SpawnZombie(pos, false, -1, missionId, m_Settings.m_aHordePrefabs.GetRandomElement());
			if (z)
				spawned.Insert(z);
		}
		else
		{
			for (int j = 0; j < 4; j++)
			{
				RZ_TrackedZombie z2 = SpawnZombie(RZ_Util.RandomPointInRing(pos, 0, 20), false, -1, missionId);
				if (z2)
					spawned.Insert(z2);
			}
		}

		foreach (RZ_TrackedZombie t : spawned)
		{
			t.m_bHorde = true;
			t.m_vGoal = target;
			t.m_bHasGoal = true;
			t.m_bRelentless = missionId != -1 && keepComing;
			RZ_Util.SendGroupTo(t.m_Entity, target, m_Settings.m_sMoveWaypointPrefab);
			if (spawnedOut)
				spawnedOut.Insert(t.m_Entity);
		}

		return !spawned.IsEmpty();
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
			Untrack(i);
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

		// Tribe samples are budget-neutral: town specials scaled by the tribes' share (never below x0.6)
		if (m_GM.GetTribes() && m_GM.GetTribes().IsEnabled())
			chance = chance * m_GM.GetTribes().TownSpecialMult();

		return chance;
	}

	//------------------------------------------------------------------------------------------------
	//! Lets mission zombies despawn, and lose players (RZ_Stealth), normally again.
	void ReleaseMission(int missionId)
	{
		foreach (RZ_TrackedZombie z : m_aTracked)
		{
			if (z.m_iMissionId != missionId)
				continue;

			z.m_iMissionId = -1;
			z.m_bRelentless = false;
		}
	}

	//------------------------------------------------------------------------------------------------
	int GetTrackedCount()
	{
		return m_aTracked.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Everything the spawner tracks, for stealth (RZ_Stealth). Read it; don't add or remove.
	array<ref RZ_TrackedZombie> GetTracked()
	{
		return m_aTracked;
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
	//! Distance to the nearest living infected within `radius` of `pos`, or radius + 1 if none
	//! (the screen guard, UI/RZ_ScreenGuard.c).
	float NearestZombieDist(vector pos, float radius)
	{
		float best = radius + 1;
		array<AIAgent> agents = {};
		foreach (RZ_TrackedZombie z : m_aTracked)
		{
			if (!z.m_Entity || RZ_Util.DistXZ(z.m_vLastPos, pos) > radius + 60)
				continue;

			SCR_AIGroup group = SCR_AIGroup.Cast(z.m_Entity);
			if (!group)
			{
				float d = RZ_Util.DistXZ(z.m_Entity.GetOrigin(), pos);
				if (d < best && RZ_Util.IsCharacterAlive(z.m_Entity))
					best = d;

				continue;
			}

			agents.Clear();
			group.GetAgents(agents);
			foreach (AIAgent agent : agents)
			{
				IEntity member = agent.GetControlledEntity();
				if (!member)
					continue;

				float md = RZ_Util.DistXZ(member.GetOrigin(), pos);
				if (md < best && RZ_Util.IsCharacterAlive(member))
					best = md;
			}
		}

		return best;
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

				Untrack(i);
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
				Untrack(i);
				continue;
			}

			z.m_vLastPos = RZ_Util.GetUnitPosition(z.m_Entity);

			if (z.m_iMissionId == -1 && RZ_Util.NearestDistance(z.m_vLastPos, players) > m_Settings.m_fDespawnRadius)
			{
				RZ_Util.DeleteUnit(z.m_Entity);
				Untrack(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected int PopulateSettlements(array<IEntity> players, RZ_StageSettings stage, int budget)
	{
		int spawned = 0;
		float maxFromPlayer = Math.Min(m_Settings.m_fActivationRadius, m_Settings.m_fDespawnRadius * 0.9);
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
				// Never farther from every player than the despawn check allows, or it's deleted next tick
				vector pos;
				if (!FindSpawnPos(s.m_vCenter, 0, s.m_fRadius, players, pos, maxFromPlayer))
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
	protected void TryHeatHorde(array<IEntity> players)
	{
		if (m_fHordeCooldownLeft > 0)
			return;

		// Only where someone is: a powered station or a bell tower on an empty map draws nothing
		vector hotPos;
		float heat;
		if (!m_GM.GetHeat().GetHottestNear(players, HORDE_PLAYER_RANGE, hotPos, heat) || heat < m_Settings.m_fHordeHeatThreshold)
			return;

		if (!SpawnHorde(hotPos, 450))
			return;

		m_GM.GetHeat().AddHeat(hotPos, -heat * 0.5);
		m_fHordeCooldownLeft = m_Settings.m_fHordeCooldown;
		m_GM.NotifyHorde(hotPos);
	}

	//------------------------------------------------------------------------------------------------
	//! maxFromPlayer > 0: the point must also be within that distance of some player.
	protected bool FindSpawnPos(vector center, float minR, float maxR, array<IEntity> players, out vector pos, float maxFromPlayer = -1)
	{
		for (int i = 0; i < 6; i++)
		{
			pos = RZ_Util.RandomPointInRing(center, minR, maxR);
			if (RZ_Util.IsWater(pos))
				continue;

			float minMult = m_fEnvMinSpawnMult;
			if (m_bFogBank && RZ_Util.DistXZ(pos, m_vFogPos) <= m_fFogRadius)
				minMult = Math.Min(minMult, m_fFogSpawnMult);

			float nearest = RZ_Util.NearestDistance(pos, players);
			if (nearest < m_Settings.m_fMinSpawnDistance * minMult)
				continue;

			if (maxFromPlayer > 0 && nearest > maxFromPlayer)
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

	// ================================================================ Crawlers and tribes (docs/INFECTED.md)
	//------------------------------------------------------------------------------------------------
	//! The tick with Crawlers or Tribes on: bodies are counted per member and in agents, towns hold
	//! crawlers (or are populated as before with Crawlers off), the countryside's roamers are loose
	//! ones walking somewhere and heat hordes come from towns (or both as before with Tribes off).
	protected void TickInfected(float dt, array<IEntity> players)
	{
		m_fClock += dt;
		RZ_InfectedBodies.NewTick();
		UpdateTrackedBodies(players);

		int budget = 100;
		int share = 28;
		if (m_TribeSettings)
		{
			budget = m_TribeSettings.m_iAgentBudget;
			share = m_TribeSettings.m_iClusterShare;
		}

		m_Budget.Begin(players, m_aTracked, budget, share);
		m_Bodies.AddTo(m_Budget);
		m_iTickAgents = 0;
		if (m_Crawlers)
			m_Crawlers.Tick(dt, players);

		UpdateSieges(players);
		if (players.IsEmpty())
			return;

		RZ_StageSettings stage = m_GM.GetDirector().GetStageSettings();
		int entries = m_Settings.m_iSpawnsPerTick;
		if (CrawlersOn())
			PopulateCrawlers(players, stage);
		else
			entries -= PopulateSettlements(players, stage, entries);

		if (TribesOn())
			PopulateLoose(players, stage);
		else if (entries > 0)
			PopulateWilderness(players, stage, entries);

		if (CrawlersOn())
		{
			foreach (RZ_TrackedZombie z : m_aTracked)
			{
				if (z.m_Entity && z.m_eRole == RZ_EInfectedRole.CRAWLER && z.m_iMissionId == -1)
					m_Crawlers.UpdateDrift(z, players, m_Settings.m_sMoveWaypointPrefab);
			}
		}

		m_fHordeCooldownLeft -= dt;
		if (TribesOn())
			TryTownHorde(players);
		else
			TryHeatHorde(players);
	}

	//------------------------------------------------------------------------------------------------
	//! Agents a tick may still spawn (Zombies > Tribes > Agents per tick).
	protected int TickRoom()
	{
		int perTick = 8;
		if (m_TribeSettings)
			perTick = Math.Max(1, m_TribeSettings.m_iAgentsPerTick);

		return Math.Max(0, perTick - m_iTickAgents);
	}

	//------------------------------------------------------------------------------------------------
	//! Per-member tracking: who died where. A member missing from its group's living agents has
	//! died (phase 0: the count drops within a second; the group deletes itself when its last member
	//! dies, and the bodies stay). Each death is a kill at that member's spot, unless it was a
	//! silent kill. Parts get their positions and their dead written back.
	protected void UpdateTrackedBodies(array<IEntity> players)
	{
		for (int i = m_aTracked.Count() - 1; i >= 0; i--)
		{
			RZ_TrackedZombie z = m_aTracked[i];
			if (!z.m_aMembers)
			{
				// Spawned before the switches were read (not expected): treat it as tracked from now
				z.m_aMembers = {};
				z.m_aMemberPos = {};
				z.m_aMemberIds = {};
				z.m_aMemberDone = {};
			}

			int alive = UpdateMembers(z);
			if (!z.m_Entity)
			{
				EndPart(z, alive);
				Untrack(i);
				continue;
			}

			// A group prefab fills over a few frames: give it time before an empty one counts as dead
			if (z.m_iLastAlive < 0 && alive == 0 && m_fClock - z.m_fBornAt < MEMBER_WAIT)
				continue;

			if (z.m_iLastAlive < 0 && SCR_AIGroup.Cast(z.m_Entity) && !z.m_bRuntimeGroup && alive > 0)
				m_mPrefabAgents.Set(z.m_sPrefab, alive);

			z.m_iLastAlive = alive;
			z.m_iAgents = alive;
			if (alive <= 0)
			{
				EndPart(z, 0);
				Untrack(i);
				continue;
			}

			z.m_vLastPos = RZ_Util.GetUnitPosition(z.m_Entity);
			if (z.m_Part)
				z.m_Part.m_vPos = z.m_vLastPos;

			if (z.m_bWalking && RZ_Util.DistXZ(z.m_vLastPos, z.m_vWalkTo) < 40)
				Arrive(z);

			if (z.m_iMissionId != -1)
				continue;

			float nearest = RZ_Util.NearestDistance(z.m_vLastPos, players);
			// An alpha wounded in the last 10 minutes stays until every player is 1.5 km away
			if (z.m_iTribe >= 0 && m_GM.GetTribes() && m_GM.GetTribes().KeepBody(z, nearest))
				continue;

			// A part's bodies go back to data, with their survivors, once every player is beyond the
			// range they appear at (600 m) and a margin, so they don't flicker in and out
			if (z.m_Part && nearest > RZ_Crawlers.PART_SPAWN_RANGE + 100 && RZ_InfectedBodies.Unseen(z.m_vLastPos, players))
			{
				EndPart(z, alive);
				RZ_Util.DeleteUnit(z.m_Entity);
				Untrack(i);
				continue;
			}

			if (nearest > m_Settings.m_fDespawnRadius)
			{
				EndPart(z, alive);
				RZ_Util.DeleteUnit(z.m_Entity);
				Untrack(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! New members are learned, living ones remembered where they stand, the dead counted once.
	//! Returns the living.
	protected int UpdateMembers(RZ_TrackedZombie z)
	{
		array<IEntity> living = {};
		RZ_Util.GetUnitMembers(z.m_Entity, living);
		foreach (IEntity m : living)
		{
			if (z.m_aMembers.Find(m) >= 0)
				continue;

			z.m_aMembers.Insert(m);
			z.m_aMemberPos.Insert(m.GetOrigin());
			z.m_aMemberIds.Insert(RZ_InfectedBodies.NewMemberId());
			z.m_aMemberDone.Insert(false);
		}

		int alive = 0;
		for (int i = 0; i < z.m_aMembers.Count(); i++)
		{
			if (z.m_aMemberDone[i])
				continue;

			IEntity member = z.m_aMembers[i];
			if (member && RZ_Util.IsCharacterAlive(member))
			{
				// The unit is gone but this one stands (not expected): it goes quietly
				if (!z.m_Entity)
				{
					MarkSilent(z, z.m_aMemberIds[i]);
					SCR_EntityHelper.DeleteEntityAndChildren(member);
				}
				else
				{
					z.m_aMemberPos[i] = member.GetOrigin();
					alive++;
					continue;
				}
			}

			vector at = z.m_aMemberPos[i];
			if (member)
				at = member.GetOrigin();

			z.m_aMemberDone[i] = true;
			OnMemberGone(z, z.m_aMemberIds[i], at);
		}

		return alive;
	}

	//------------------------------------------------------------------------------------------------
	//! One member gone. A silent kill gives nothing; anything else is a kill at its spot (samples
	//! for a special drop there, the town's infection and ledger, bodies to search, mission credit).
	protected void OnMemberGone(RZ_TrackedZombie z, int memberId, vector at)
	{
		bool silent = z.m_aSilentIds && z.m_aSilentIds.Find(memberId) >= 0;
		if (z.m_Part)
		{
			z.m_Part.m_iCount = Math.Max(0, z.m_Part.m_iCount - 1);
			if (!silent)
				z.m_Part.m_iKilled = z.m_Part.m_iKilled + 1;
		}

		// A tribe's: its losses, the vigil, its samples (docs/INFECTED.md, Samples)
		if (z.m_iTribe >= 0 && m_GM.GetTribes())
			m_GM.GetTribes().OnMemberKilled(z, at, silent);

		if (silent)
			return;

		m_GM.OnZombiesKilled(z.m_iSettlement, z.m_iMissionId, z.m_bSpecial, at, 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void MarkSilent(RZ_TrackedZombie z, int memberId)
	{
		if (!z.m_aSilentIds)
			z.m_aSilentIds = {};

		if (z.m_aSilentIds.Find(memberId) < 0)
			z.m_aSilentIds.Insert(memberId);
	}

	//------------------------------------------------------------------------------------------------
	//! The bodies go: the part keeps its survivors where they were, as data.
	protected void EndPart(RZ_TrackedZombie z, int survivors)
	{
		RZ_InfectedPart part = z.m_Part;
		if (!part)
			return;

		RZ_TribePart tribePart = RZ_TribePart.Cast(part);
		if (tribePart && m_GM.GetTribes())
			m_GM.GetTribes().OnPartEnds(tribePart, z);

		part.m_iCount = Math.Max(0, survivors);
		part.m_vPos = z.m_vLastPos;
		part.m_Body = null;
		z.m_Part = null;
	}

	//------------------------------------------------------------------------------------------------
	//! A silent kill (docs/INFECTED.md, "Bodies"): no samples, no infection change, no ledger, no
	//! searchable body and no mission credit. DELETE: gone without a trace, only out of sight.
	//! KILL_AND_MARK: killed where it stands, the body stays (and the zombie mod's loose item, for
	//! the loose-item cleanup). e is a unit or one of its members. False if the spawner doesn't own it.
	bool KillSilently(IEntity e, RZ_ESilentKill mode)
	{
		if (!e)
			return false;

		for (int i = m_aTracked.Count() - 1; i >= 0; i--)
		{
			RZ_TrackedZombie z = m_aTracked[i];
			bool whole = z.m_Entity == e;
			int at = -1;
			if (!whole && z.m_aMembers)
				at = z.m_aMembers.Find(e);

			if (!whole && at < 0)
				continue;

			// No per-member tracking (both switches off): the unit goes, as a special caught alive
			if (!z.m_aMembers)
			{
				Untrack(i);
				RZ_Util.DeleteUnit(z.m_Entity);
				return true;
			}

			array<IEntity> victims = {};
			foreach (int k, IEntity member : z.m_aMembers)
			{
				if (z.m_aMemberDone[k] || (!whole && k != at))
					continue;

				MarkSilent(z, z.m_aMemberIds[k]);
				if (member)
					victims.Insert(member);
			}

			foreach (IEntity victim : victims)
			{
				if (mode == RZ_ESilentKill.KILL_AND_MARK)
					RZ_Util.KillCharacter(victim);
				else
					SCR_EntityHelper.DeleteEntityAndChildren(victim);
			}

			if (whole && mode == RZ_ESilentKill.DELETE)
				RZ_Util.DeleteUnit(z.m_Entity);

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A loose one got where it was going: at a town it becomes one of that town's crawlers.
	protected void Arrive(RZ_TrackedZombie z)
	{
		z.m_bWalking = false;
		z.m_vHome = z.m_vLastPos;
		if (z.m_iWalkTown < 0 || !CrawlersOn())
			return;

		z.m_iSettlement = z.m_iWalkTown;
		z.m_eRole = RZ_EInfectedRole.CRAWLER;
		m_iLooseArrived++;
	}

	//------------------------------------------------------------------------------------------------
	//! Living agents of a town's own (crawlers, its specials, its horde and scavengers out with bodies).
	int CountAgentsForSettlement(int idx)
	{
		int n = 0;
		foreach (RZ_TrackedZombie z : m_aTracked)
		{
			if (z.m_iSettlement == idx && z.m_Entity)
				n += z.Agents();
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Towns hold crawlers (Zombies > Crawlers): the old formula in spawns, times the average group,
	//! less the ledger and those out; placed by the time of day; once a town is full, what refills it
	//! comes from the buildings 200 m from the players and walks into the street. All inside the
	//! agent budget, a few agents a tick.
	protected void PopulateCrawlers(array<IEntity> players, RZ_StageSettings stage)
	{
		float maxFromPlayer = Math.Min(m_Settings.m_fActivationRadius, m_Settings.m_fDespawnRadius * 0.9);
		foreach (int idx, RZ_Settlement s : m_GM.GetSettlements())
		{
			if (TickRoom() <= 0 || m_aTracked.Count() >= m_Settings.m_iGlobalCap)
				break;

			if (RZ_Util.NearestDistance(s.m_vCenter, players) > m_Settings.m_fActivationRadius + s.m_fRadius)
			{
				// Gone back to data: the next visit fills it as a first visit
				if (m_Crawlers.IsFull(idx) && CountAgentsForSettlement(idx) == 0)
					m_Crawlers.SetFull(idx, false);

				continue;
			}

			if (m_aSuppressed.Contains(idx))
				continue;

			float entries = s.m_iSize * m_Settings.m_fDensityPerSize * (s.m_fInfection / 100) * stage.m_fDensityMult * m_fEventDensityMult;
			entries += m_GM.GetHeat().GetHeat(s.m_vCenter) / 25;
			if (m_iFocusSettlement >= 0)
			{
				if (idx == m_iFocusSettlement)
					entries = entries * 1.5;
				else
					entries = entries * 0.4;
			}

			entries = entries * GentleMult(s.m_vCenter, m_Settings.m_fActivationRadius + s.m_fRadius);
			float target = m_Crawlers.TargetAgents(idx, s, entries);
			int present = CountAgentsForSettlement(idx);
			if (present >= target - 0.5)
			{
				m_Crawlers.SetFull(idx, true);
				continue;
			}

			bool reinforce = m_Crawlers.IsFull(idx);
			int tries = 0;
			while (present < target - 0.5 && tries < 4 && TickRoom() > 0 && m_aTracked.Count() < m_Settings.m_iGlobalCap)
			{
				tries++;
				int room = Math.Min(m_Budget.RoomAt(s.m_vCenter), TickRoom());
				int size = Math.Min(m_Crawlers.RollGroupSize(), Math.Ceil(target - present));
				size = Math.Min(size, room);
				if (size <= 0)
					break;

				vector pos;
				RZ_ECrawlerSpot kind;
				if (!FindCrawlerSpot(idx, s, reinforce, players, maxFromPlayer, pos, kind))
					break;

				// Specials as before (Install Sensors doubles them)
				float specialChance = GetSpecialChance(stage);
				if (s.m_fSensorSeconds > 0)
					specialChance = Math.Min(0.5, specialChance * 2);

				RZ_TrackedZombie z;
				if (Math.RandomFloat01() < specialChance)
				{
					z = SpawnZombie(pos, true, idx, -1);
					if (z)
					{
						m_Budget.Add(pos, z.Agents());
						m_iTickAgents += z.Agents();
					}
				}
				else
				{
					z = SpawnUnit(pos, m_Crawlers.PickPrefab(s), size, idx, RZ_EInfectedRole.CRAWLER);
				}

				if (!z)
					break;

				present += z.Agents();
				// Reinforcements walk out of the buildings into the street
				if (reinforce)
				{
					vector street = RZ_Util.GroundPos(RZ_Util.RandomPointInRing(pos + (s.m_vCenter - pos) * 0.2, 10, 40));
					z.m_vHome = street;
					RZ_Util.SendGroupTo(z.m_Entity, street, m_Settings.m_sMoveWaypointPrefab);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A crawler spot that passes every check: dry, far enough (200 m for reinforcements), near
	//! enough a player not to be removed at once, not suppressed, scorched or by a held door, and out
	//! of every player's sight within 300 m (terrain, buildings and trees).
	protected bool FindCrawlerSpot(int idx, RZ_Settlement s, bool reinforce, array<IEntity> players, float maxFromPlayer, out vector pos, out RZ_ECrawlerSpot kind)
	{
		float minDist = m_Settings.m_fMinSpawnDistance * m_fEnvMinSpawnMult;
		if (reinforce)
			minDist = Math.Max(minDist, m_Crawlers.GetSettings().m_fReinforceDistance);

		RZ_Interactions hub = m_GM.GetInteractions();
		for (int i = 0; i < 6; i++)
		{
			pos = m_Crawlers.PickSpot(idx, s, reinforce, kind);
			if (RZ_Util.IsWater(pos))
				continue;

			float mult = 1;
			if (m_bFogBank && RZ_Util.DistXZ(pos, m_vFogPos) <= m_fFogRadius)
				mult = m_fFogSpawnMult;

			float nearest = RZ_Util.NearestDistance(pos, players);
			if (nearest < minDist * mult || nearest > maxFromPlayer)
				continue;

			if (!m_aSuppressed.IsEmpty() && IsSuppressedAt(pos))
				continue;

			if (hub && hub.IsActive() && hub.GetTraps().IsScorched(pos))
				continue;

			if (m_Crawlers.IsNearHeldDoor(pos))
				continue;

			if (RZ_InfectedBodies.InSight(pos, players, true))
				continue;

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Loose ones (Zombies > Tribes): the countryside's roamers, the same count, each spawned
	//! 250-300 m out behind terrain and walking somewhere, a town or a lair candidate within 3 km,
	//! so following one leads somewhere. By day they're crawlers drifted out of town; at night
	//! anything. With no town or lair within 3 km they spawn as before.
	protected void PopulateLoose(array<IEntity> players, RZ_StageSettings stage)
	{
		int wanted = RZ_Util.RoundToInt(m_Settings.m_iAmbientPerPlayer * stage.m_fDensityMult * m_fEventDensityMult);
		foreach (IEntity player : players)
		{
			if (TickRoom() <= 0 || m_aTracked.Count() >= m_Settings.m_iGlobalCap)
				return;

			vector p = player.GetOrigin();
			int wantedHere = RZ_Util.RoundToInt(wanted * GentleMult(p, 1));
			if (CountRoleNear(p, RZ_EInfectedRole.LOOSE, m_Settings.m_fDespawnRadius) >= wantedHere)
				continue;

			if (m_Budget.RoomAt(p) <= 0)
				continue;

			// Specials as the old roamers had them
			bool special = Math.RandomFloat01() < GetSpecialChance(stage) * 0.5;
			vector goal;
			int town;
			if (special || !PickLooseGoal(p, goal, town))
			{
				vector old;
				if (!FindSpawnPos(p, m_Settings.m_fMinSpawnDistance + 30, AMBIENT_RADIUS - 50, players, old))
					continue;

				RZ_TrackedZombie roamer = SpawnZombie(old, special, -1, -1);
				if (roamer)
				{
					roamer.m_eRole = RZ_EInfectedRole.LOOSE;
					m_Budget.Add(old, roamer.Agents());
					m_iTickAgents += roamer.Agents();
				}

				continue;
			}

			vector pos;
			if (!FindLooseSpot(p, players, pos))
				continue;

			ResourceName prefab = "";
			int size = 1;
			RZ_WorldFacts facts = m_GM.GetFacts();
			bool night = facts && facts.IsNight();
			if (CrawlersOn() && !night)
			{
				prefab = m_Crawlers.PickPrefab(m_GM.GetSettlement(town));
				size = Math.RandomIntInclusive(1, 2);
			}

			RZ_TrackedZombie z = SpawnUnit(pos, prefab, size, -1, RZ_EInfectedRole.LOOSE);
			if (!z)
				continue;

			z.m_bWalking = true;
			z.m_vWalkTo = goal;
			z.m_iWalkTown = town;
			z.m_vHome = goal;
			RZ_Util.SendGroupTo(z.m_Entity, goal, m_Settings.m_sMoveWaypointPrefab);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Where a loose one near p walks: by day a town (a crawler going home), at night a lair
	//! candidate when the land grid has one, else a town. Within 3 km; false if neither.
	protected bool PickLooseGoal(vector p, out vector goal, out int town)
	{
		town = -1;
		int idx = m_GM.FindNearestSettlement(p);
		RZ_Settlement s = m_GM.GetSettlement(idx);
		bool hasTown = s && RZ_Util.DistXZ(s.m_vCenter, p) <= LOOSE_GOAL_RANGE && RZ_Util.DistXZ(s.m_vCenter, p) > s.m_fRadius;

		vector lair;
		bool hasLair = false;
		RZ_Land land = m_GM.GetLand();
		if (land && land.IsReady())
			hasLair = land.NearestCellWith(p, RZ_Land.BIT_LAIR, 400, LOOSE_GOAL_RANGE, lair);

		RZ_WorldFacts facts = m_GM.GetFacts();
		bool night = facts && facts.IsNight();
		if (hasLair && (night || !hasTown))
		{
			goal = RZ_Util.GroundPos(RZ_Util.RandomPointInRing(lair, 0, 80));
			return true;
		}

		if (!hasTown)
			return false;

		town = idx;
		goal = RZ_Util.GroundPos(RZ_Util.RandomPointInRing(s.m_vCenter, 0, s.m_fRadius * 0.6));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! 250-300 m from the player and behind terrain from everyone within 300 m; else just beyond
	//! 300 m, where a body may appear in the open.
	protected bool FindLooseSpot(vector p, array<IEntity> players, out vector pos)
	{
		for (int i = 0; i < 10; i++)
		{
			float lo = 250;
			float hi = 300;
			if (i >= 6)
			{
				lo = 305;
				hi = 340;
			}

			pos = RZ_Util.GroundPos(RZ_Util.RandomPointInRing(p, lo, hi));
			if (RZ_Util.IsWater(pos) || RZ_Util.NearestDistance(pos, players) < m_Settings.m_fMinSpawnDistance)
				continue;

			if (!m_aSuppressed.IsEmpty() && IsSuppressedAt(pos))
				continue;

			if (RZ_InfectedBodies.InSight(pos, players, false))
				continue;

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected int CountRoleNear(vector pos, RZ_EInfectedRole role, float radius)
	{
		int n = 0;
		foreach (RZ_TrackedZombie z : m_aTracked)
		{
			if (z.m_Entity && z.m_eRole == role && RZ_Util.DistXZ(z.m_vLastPos, pos) <= radius)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Heat hordes come from somewhere (Zombies > Tribes): from the nearest town within 1.5 km, as
	//! that town's own crawlers spawned 300-450 m out on its side, so they come from a direction and
	//! count as lent out of the town. With no town that near, as before.
	protected void TryTownHorde(array<IEntity> players)
	{
		if (m_fHordeCooldownLeft > 0)
			return;

		vector hotPos;
		float heat;
		if (!m_GM.GetHeat().GetHottestNear(players, HORDE_PLAYER_RANGE, hotPos, heat) || heat < m_Settings.m_fHordeHeatThreshold)
			return;

		int idx = m_GM.FindNearestSettlement(hotPos);
		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (!s || RZ_Util.DistXZ(s.m_vCenter, hotPos) > s.m_fRadius + HORDE_TOWN_RANGE)
		{
			TryHeatHorde(players);
			return;
		}

		// Inside the budget: wait for room rather than spawn a thin horde
		if (m_Budget.RoomAt(hotPos) < 4)
			return;

		vector dir = s.m_vCenter - hotPos;
		dir[1] = 0;
		if (dir.Length() < 1)
			dir = Vector(1, 0, 0);
		dir.Normalize();
		float baseYaw = Math.Atan2(dir[0], dir[2]);

		vector from;
		bool found = false;
		for (int i = 0; i < 8; i++)
		{
			float yaw = baseYaw + Math.RandomFloat(-0.45, 0.45);
			from = RZ_Util.GroundPos(hotPos + Vector(Math.Sin(yaw), 0, Math.Cos(yaw)) * Math.RandomFloat(300, 450));
			if (RZ_Util.IsWater(from) || RZ_Util.NearestDistance(from, players) < 250)
				continue;

			if (RZ_InfectedBodies.InSight(from, players, false))
				continue;

			found = true;
			break;
		}

		if (!found)
			return;

		array<RZ_TrackedZombie> horde = {};
		if (CrawlersOn())
		{
			int left = Math.Min(m_Budget.RoomAt(hotPos), 12);
			for (int j = 0; j < 4 && left > 0; j++)
			{
				int size = Math.Min(m_Crawlers.RollGroupSize() + 1, left);
				RZ_TrackedZombie unit = SpawnUnit(RZ_Util.GroundPos(RZ_Util.RandomPointInRing(from, 0, 20)), m_Crawlers.PickPrefab(s), size, idx, RZ_EInfectedRole.CRAWLER);
				if (!unit)
					break;

				left -= unit.Agents();
				horde.Insert(unit);
			}
		}
		else if (!m_Settings.m_aHordePrefabs.IsEmpty())
		{
			RZ_TrackedZombie big = SpawnUnit(from, m_Settings.m_aHordePrefabs.GetRandomElement(), 1, idx, RZ_EInfectedRole.NONE);
			if (big)
				horde.Insert(big);
		}

		if (horde.IsEmpty())
			return;

		foreach (RZ_TrackedZombie t : horde)
		{
			t.m_bHorde = true;
			t.m_vGoal = hotPos;
			t.m_bHasGoal = true;
			RZ_Util.SendGroupTo(t.m_Entity, hotPos, m_Settings.m_sMoveWaypointPrefab);
		}

		m_GM.GetHeat().AddHeat(hotPos, -heat * 0.5);
		m_fHordeCooldownLeft = m_Settings.m_fHordeCooldown;
		m_iHordesFromTowns++;
		RZ_Log.Info(string.Format("Heat horde: %1 units out of %2, from its side of %3.", horde.Count(), s.m_sName, RZ_SettlementScanner.GridReference(hotPos)));
		m_GM.NotifyHorde(hotPos);
	}

	//------------------------------------------------------------------------------------------------
	//! A data part near a player gets bodies (RZ_Crawlers): at its spot if no player could see it
	//! there, else pushed back toward its town; inside the budget.
	void SpawnPart(RZ_InfectedPart part, array<IEntity> players)
	{
		if (part.HasBody() || part.m_iCount <= 0 || !m_Crawlers)
			return;

		int n = part.m_iCount;
		if (m_Budget.RoomAt(part.m_vPos) < n || TickRoom() < n || m_aTracked.Count() >= m_Settings.m_iGlobalCap)
			return;

		RZ_Settlement s = m_GM.GetSettlement(part.m_iTown);
		vector back = part.m_vPos;
		if (s)
			back = s.m_vCenter;

		vector pos = part.m_vPos;
		bool found = false;
		for (int i = 0; i < 6; i++)
		{
			float nearest = RZ_Util.NearestDistance(pos, players);
			if (!RZ_Util.IsWater(pos) && nearest >= m_Settings.m_fMinSpawnDistance && !RZ_InfectedBodies.InSight(pos, players, true))
			{
				found = true;
				break;
			}

			vector dir = back - pos;
			dir[1] = 0;
			if (dir.Length() < 40)
				break;

			dir.Normalize();
			pos = RZ_Util.GroundPos(pos + dir * 40);
		}

		if (!found)
			return;

		RZ_TrackedZombie z = SpawnUnit(pos, m_Crawlers.PickPrefab(s), n, part.m_iTown, part.m_eRole);
		if (!z)
			return;

		z.m_Part = part;
		part.m_Body = z;
		if (part.m_bHasTarget && RZ_Util.DistXZ(pos, part.m_vTarget) > 20)
			RZ_Util.SendGroupTo(z.m_Entity, RZ_Util.RandomPointInRing(part.m_vTarget, 2, 10), m_Settings.m_sMoveWaypointPrefab);
	}

	//------------------------------------------------------------------------------------------------
	//! Dawn: scavengers with bodies walk home and are the town's crawlers again.
	void SendPartHome(RZ_InfectedPart part)
	{
		RZ_TrackedZombie z = part.m_Body;
		part.m_Body = null;
		if (!z || !z.m_Entity)
			return;

		z.m_Part = null;
		z.m_eRole = RZ_EInfectedRole.CRAWLER;
		RZ_Settlement s = m_GM.GetSettlement(z.m_iSettlement);
		if (!s)
			return;

		z.m_vHome = RZ_Util.GroundPos(RZ_Util.RandomPointInRing(s.m_vCenter, 0, s.m_fRadius * 0.7));
		RZ_Util.SendGroupTo(z.m_Entity, z.m_vHome, m_Settings.m_sMoveWaypointPrefab);
	}

	// ================================================================ Sieges at locked doors
	//------------------------------------------------------------------------------------------------
	//! A locked door is being banged on (RZ_DoorBlockers): the infected at it are on the clock.
	void OnDoorBang(vector at)
	{
		if (!CrawlersOn())
			return;

		foreach (RZ_TrackedZombie z : m_aTracked)
		{
			if (!z.m_Entity || z.m_iMissionId != -1 || !z.m_aMemberPos || RZ_Util.DistXZ(z.m_vLastPos, at) > 80)
				continue;

			bool atDoor = false;
			foreach (vector mp : z.m_aMemberPos)
			{
				if (RZ_Util.DistXZ(mp, at) <= 4)
				{
					atDoor = true;
					break;
				}
			}

			if (!atDoor)
				continue;

			if (z.m_fSiegeSince <= 0)
			{
				z.m_fSiegeSince = m_fClock;
				z.m_vSiegeAt = at;
			}

			z.m_fSiegeLast = m_fClock;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! After Siege minutes at a door they give up and the door holds: out of every player's sight
	//! they go back to the town as data, else they're sent away. No crawler spawns by that door for
	//! as long again.
	protected void UpdateSieges(array<IEntity> players)
	{
		if (!CrawlersOn())
			return;

		for (int i = m_aTracked.Count() - 1; i >= 0; i--)
		{
			RZ_TrackedZombie z = m_aTracked[i];
			if (z.m_fSiegeSince <= 0)
				continue;

			if (m_fClock - z.m_fSiegeLast > SIEGE_GAP)
			{
				z.m_fSiegeSince = 0;
				continue;
			}

			if (m_fClock - z.m_fSiegeSince < m_Crawlers.SiegeSeconds())
				continue;

			z.m_fSiegeSince = 0;
			m_Crawlers.HoldDoor(z.m_vSiegeAt);
			foreach (IEntity p : players)
			{
				if (p && RZ_Util.DistXZ(p.GetOrigin(), z.m_vSiegeAt) <= 40)
					m_GM.NotifyPlayer(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(p), "Quiet", "The banging stops. Whatever it was has given up on the door, for now.");
			}

			if (!z.m_Entity)
				continue;

			if (!RZ_InfectedBodies.InSight(z.m_vLastPos, players, true))
			{
				EndPart(z, z.Agents());
				RZ_Util.DeleteUnit(z.m_Entity);
				Untrack(i);
				continue;
			}

			vector away = z.m_vSiegeAt - z.m_vHome;
			vector dest = z.m_vHome;
			if (away.Length() < 60)
				dest = RZ_Util.GroundPos(RZ_Util.RandomPointInRing(z.m_vSiegeAt, 80, 150));

			RZ_Util.RedirectGroup(z.m_Entity, dest, m_Settings.m_sMoveWaypointPrefab);
		}
	}

	// ================================================================ Death attribution and admin
	//------------------------------------------------------------------------------------------------
	//! The tracked unit with a living member nearest pos within radius (the bite detector's
	//! attribution), or null.
	RZ_TrackedZombie FindUnitNear(vector pos, float radius)
	{
		RZ_TrackedZombie best;
		float bestDist = radius;
		array<IEntity> members = {};
		foreach (RZ_TrackedZombie z : m_aTracked)
		{
			if (!z.m_Entity || RZ_Util.DistXZ(z.m_vLastPos, pos) > radius + 60)
				continue;

			members.Clear();
			RZ_Util.GetUnitMembers(z.m_Entity, members);
			foreach (IEntity m : members)
			{
				float d = RZ_Util.DistXZ(m.GetOrigin(), pos);
				if (d <= bestDist && RZ_Util.IsCharacterAlive(m))
				{
					bestDist = d;
					best = z;
				}
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! "infected 62/100 agents (19 entries), clusters 2 share 28 [28/28, 6/28], hordes from towns 1, loose arrived 3"
	string StatusText()
	{
		if (!IsPlumbingOn())
			return string.Format("infected: %1 entries (crawlers and tribes off)", m_aTracked.Count());

		int agents = 0;
		int crawlers = 0;
		int loose = 0;
		foreach (RZ_TrackedZombie z : m_aTracked)
		{
			if (!z.m_Entity)
				continue;

			agents += z.Agents();
			if (z.m_eRole == RZ_EInfectedRole.CRAWLER || z.m_eRole == RZ_EInfectedRole.SCAVENGER)
				crawlers += z.Agents();
			else if (z.m_eRole == RZ_EInfectedRole.LOOSE)
				loose += z.Agents();
		}

		string text = string.Format("infected %1/%2 agents (%3 entries; crawlers %4, loose %5), clusters %6 share %7 [%8]",
			agents, m_Budget.GetBudget(), m_aTracked.Count(), crawlers, loose, m_Budget.GetClusterCount(), m_Budget.GetShare(), m_Budget.DescribeClusters());
		text += string.Format(", other bodies %1 agents, hordes from towns %2, loose ones arrived %3", m_Bodies.Count(""), m_iHordesFromTowns, m_iLooseArrived);
		if (m_Crawlers)
			text += " | " + m_Crawlers.StatusText();

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! Every tracked unit, to the log (/tribes).
	void DumpTracked()
	{
		foreach (RZ_TrackedZombie z : m_aTracked)
		{
			if (!z.m_Entity)
				continue;

			int partId = -1;
			if (z.m_Part)
				partId = z.m_Part.m_iId;

			RZ_Log.Info(string.Format("Tribes dump | unit %1 at %2: town %3, mission %4, agents %5, part %6, drifting %7, walking %8, siege %9",
				typename.EnumToString(RZ_EInfectedRole, z.m_eRole), RZ_SettlementScanner.GridReference(z.m_vLastPos), z.m_iSettlement,
				z.m_iMissionId, z.Agents(), partId, z.m_bDrifting, z.m_bWalking, z.m_fSiegeSince > 0));
		}
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

			Untrack(i);
			RZ_Util.DeleteUnit(e);
			return true;
		}

		return false;
	}
}
