//! Escort block (docs/DESIGN.md, "Mission building blocks"). People who walk with players:
//! along a route (Refugee Column, Bust Out) or behind one player (Rescue the Pilot, Escort the
//! Scientist). A mission owns one RZ_Escort, starts it, ticks it from its OnUpdate and reads
//! its state.
//!
//! - **Data first.** The escorted people are a position on the server. They get a body only
//!   while a player is within the body radius (Contracts > Escort body radius) and lose it when
//!   everyone leaves, like the wanderers.
//! - **With an AI group** the body walks on its own to move waypoints; its position is synced
//!   back. If every member dies, the escort is DEAD and the mission fails.
//! - **Without one** (no prefab, or a character or prop) the people move as data while they're
//!   moving, and stand as a figure (Puzzles > Civilian) when they stop or arrive. As data they
//!   can't be killed: the infected only reach a body.
//! - **Stop and resume.** They only move while a player is within the leash (150 m), while no
//!   infected is within 25 m of them (they hide), and while the mission hasn't stopped them.
//! - **Capped.** At most Contracts > Max escorts run at once, server-wide (HasRoom / Begin).
//! - **Handed over.** If the player they follow leaves or dies, they follow the nearest player
//!   within 20 m instead (or wait for one).
//! - **Talk to them.** With SetTalk(), the body (or the group's leader) carries the generic
//!   object's actions for the owning puzzle mission (patch up the pilot, arrest the deserter).
enum RZ_EEscortState
{
	WAITING,	// not started, or waiting for the mission
	MOVING,
	STOPPED,	// held: nobody near, infected close, or stopped by the mission
	ARRIVED,
	DEAD
}

class RZ_Escort
{
	protected static int s_iRunning;
	protected static ref map<string, bool> s_mGroupPrefabs;	// prefab -> spawns an AI group

	string m_sName = "them";				// "the refugees", for messages
	ResourceName m_sPrefab;					// an AI group walks; anything else stands as a figure
	vector m_vPos;
	RZ_EEscortState m_eState = RZ_EEscortState.WAITING;
	float m_fArriveRadius = 20;
	float m_fFollowGap = 6;					// follow mode: stop this close behind the player
	bool m_bHide = true;					// wait while infected are within 25 m

	protected ref array<vector> m_aRoute = {};
	protected int m_iLeg;
	protected int m_iFollow = -1;			// follow this player instead of the route
	protected bool m_bHeld;					// stopped by the mission
	protected bool m_bStarted;
	protected bool m_bCounted;

	protected IEntity m_Body;
	protected bool m_bHasBody;
	protected bool m_bWalks;
	protected bool m_bTagged;
	protected bool m_bSeenAlive;
	protected float m_fBodyAge;
	protected vector m_vOrdered;
	protected bool m_bOrdered;

	// Talk-to tags for the owning puzzle mission
	protected int m_iTagMission = -1;
	protected int m_iTagObject = -1;
	protected string m_sChoiceLabels;
	protected int m_iTagClue = -1;
	protected string m_sReadLabel;

	protected static const float HIDE_RADIUS = 25;
	protected static const float REORDER_DIST = 15;
	protected static const float DESPAWN_MARGIN = 100;

	//------------------------------------------------------------------------------------------------
	//! A new game (the mission manager is made): no escort runs yet. Statics outlive a session.
	static void ResetCount()
	{
		s_iRunning = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Room for another escort right now (Contracts > Max escorts).
	static bool HasRoom(RZ_GameModeComponent gm)
	{
		RZ_ContractSettings cs = gm.GetContractSettings();
		int cap = 3;
		if (cs)
			cap = cs.m_iMaxEscorts;

		return s_iRunning < cap;
	}

	//------------------------------------------------------------------------------------------------
	//! Takes a slot. False if the cap is reached (the mission shouldn't run then).
	bool Begin(RZ_GameModeComponent gm, vector start)
	{
		if (m_bCounted)
			return true;

		if (!HasRoom(gm))
			return false;

		s_iRunning++;
		m_bCounted = true;
		m_vPos = RZ_Util.GroundPos(start);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Walk these points in order. Clears an earlier route.
	void SetRoute(array<vector> points)
	{
		m_aRoute.Clear();
		foreach (vector p : points)
		{
			m_aRoute.Insert(p);
		}

		m_iLeg = 0;
		m_iFollow = -1;
		m_bOrdered = false;
	}

	//------------------------------------------------------------------------------------------------
	//! Walk straight to one point.
	void SetDestination(vector p)
	{
		array<vector> one = {p};
		SetRoute(one);
	}

	//------------------------------------------------------------------------------------------------
	//! Follow one player (-1: stop following and wait).
	void Follow(int playerId)
	{
		m_iFollow = playerId;
		m_bOrdered = false;
	}

	//------------------------------------------------------------------------------------------------
	int GetFollowed()
	{
		return m_iFollow;
	}

	//------------------------------------------------------------------------------------------------
	//! Start walking (the route, or behind the followed player).
	void Go()
	{
		m_bStarted = true;
		m_bHeld = false;
		m_bOrdered = false;
	}

	//------------------------------------------------------------------------------------------------
	//! The mission stops them where they are (a breakdown, a talk, a choice).
	void Stop()
	{
		m_bHeld = true;
		m_bOrdered = false;
	}

	//------------------------------------------------------------------------------------------------
	void Resume()
	{
		m_bHeld = false;
		m_bOrdered = false;
	}

	//------------------------------------------------------------------------------------------------
	//! Make the body talkable for a puzzle mission: its choice slots and a "Talk to" read action.
	void SetTalk(int missionId, int objIdx, string choiceLabels, int clueIdx, string readLabel)
	{
		m_iTagMission = missionId;
		m_iTagObject = objIdx;
		m_sChoiceLabels = choiceLabels;
		m_iTagClue = clueIdx;
		m_sReadLabel = readLabel;
		m_bTagged = false;
	}

	//------------------------------------------------------------------------------------------------
	bool IsArrived()
	{
		return m_eState == RZ_EEscortState.ARRIVED;
	}

	//------------------------------------------------------------------------------------------------
	bool IsDead()
	{
		return m_eState == RZ_EEscortState.DEAD;
	}

	//------------------------------------------------------------------------------------------------
	bool HasBody()
	{
		return m_bHasBody && m_Body;
	}

	//------------------------------------------------------------------------------------------------
	bool IsNear(vector p, float radius)
	{
		return RZ_Util.DistXZ(m_vPos, p) <= radius;
	}

	//------------------------------------------------------------------------------------------------
	//! Mark them arrived (follow mode: the mission decides where "there" is).
	void Arrive()
	{
		m_eState = RZ_EEscortState.ARRIVED;
		OrderStop();
	}

	//------------------------------------------------------------------------------------------------
	//! Once per mission tick.
	void Update(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_eState == RZ_EEscortState.DEAD)
			return;

		UpdateBody(gm, dt, players);
		if (m_eState == RZ_EEscortState.DEAD || m_eState == RZ_EEscortState.ARRIVED || !m_bStarted)
			return;

		if (m_iFollow > 0)
			CheckFollowed(gm, ids, players);

		vector target;
		if (!GetTarget(target))
		{
			m_eState = RZ_EEscortState.STOPPED;
			OrderStop();
			return;
		}

		if (!CanMove(gm, players))
		{
			if (m_eState != RZ_EEscortState.STOPPED)
				OrderStop();

			m_eState = RZ_EEscortState.STOPPED;
			return;
		}

		m_eState = RZ_EEscortState.MOVING;
		float stopAt = m_fArriveRadius * 0.5;
		if (m_iFollow > 0)
			stopAt = m_fFollowGap;

		if (m_bWalks && HasBody())
			OrderMove(gm, target);
		else
			StepData(gm, dt, target, stopAt);

		// Route legs: on to the next point, or there
		if (m_iFollow > 0 || RZ_Util.DistXZ(m_vPos, target) > m_fArriveRadius)
			return;

		m_iLeg++;
		m_bOrdered = false;
		if (m_iLeg >= m_aRoute.Count())
			Arrive();
	}

	//------------------------------------------------------------------------------------------------
	//! The player they follow left the server or died: they fall in behind whoever is closest
	//! (within 20 m), so the job doesn't stall. Nobody close: they wait where they are.
	protected void CheckFollowed(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players)
	{
		IEntity followed = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iFollow);
		if (followed && RZ_Util.IsCharacterAlive(followed))
			return;

		int best = -1;
		float bestDist = 20;
		foreach (int i, IEntity p : players)
		{
			float d = RZ_Util.DistXZ(p.GetOrigin(), m_vPos);
			if (d <= bestDist)
			{
				bestDist = d;
				best = ids[i];
			}
		}

		if (best <= 0)
			return;

		m_iFollow = best;
		m_bOrdered = false;
		gm.NotifyPlayer(best, "Escort", string.Format("Whoever %1 was following is gone. Now they follow you.", m_sName));
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetTarget(out vector target)
	{
		if (m_iFollow > 0)
		{
			IEntity p = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iFollow);
			if (!p || !RZ_Util.IsCharacterAlive(p))
				return false;

			target = p.GetOrigin();
			return true;
		}

		if (m_iLeg < 0 || m_iLeg >= m_aRoute.Count())
			return false;

		target = m_aRoute[m_iLeg];
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanMove(RZ_GameModeComponent gm, array<IEntity> players)
	{
		if (m_bHeld)
			return false;

		float leash = 150;
		RZ_ContractSettings cs = gm.GetContractSettings();
		if (cs)
			leash = cs.m_fEscortLeash;

		if (m_iFollow > 0)
		{
			IEntity p = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iFollow);
			if (!p || RZ_Util.DistXZ(p.GetOrigin(), m_vPos) > leash)
				return false;
		}
		else if (RZ_Util.NearestDistance(m_vPos, players) > leash)
		{
			return false;
		}

		if (m_bHide && gm.GetSpawner().IsZombieNear(m_vPos, HIDE_RADIUS))
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! As data: a straight line toward the target, no closer than `stopAt`.
	protected void StepData(RZ_GameModeComponent gm, float dt, vector target, float stopAt)
	{
		float speed = 1.3;
		RZ_ContractSettings cs = gm.GetContractSettings();
		if (cs)
			speed = cs.m_fEscortSpeed;

		float dist = RZ_Util.DistXZ(m_vPos, target);
		if (dist <= stopAt)
			return;

		float step = Math.Min(speed * dt, dist - stopAt);
		vector p = m_vPos;
		p[0] = p[0] + (target[0] - p[0]) * step / dist;
		p[2] = p[2] + (target[2] - p[2]) * step / dist;
		m_vPos = RZ_Util.GroundPos(p);
	}

	//------------------------------------------------------------------------------------------------
	//! An AI body: a move waypoint to the target, renewed when the target has moved on.
	protected void OrderMove(RZ_GameModeComponent gm, vector target)
	{
		if (m_bOrdered && RZ_Util.DistXZ(m_vOrdered, target) < REORDER_DIST)
			return;

		vector at = target;
		if (m_iFollow > 0)
		{
			// Stop a few metres short of the player, on their side
			float dist = RZ_Util.DistXZ(m_vPos, target);
			if (dist > m_fFollowGap)
			{
				at[0] = target[0] + (m_vPos[0] - target[0]) * m_fFollowGap / dist;
				at[2] = target[2] + (m_vPos[2] - target[2]) * m_fFollowGap / dist;
				at = RZ_Util.GroundPos(at);
			}
		}

		RZ_Util.RedirectGroup(m_Body, at, gm.GetSpawner().GetMoveWaypointPrefab());
		m_vOrdered = target;
		m_bOrdered = true;
	}

	//------------------------------------------------------------------------------------------------
	//! An AI body stands still: a move waypoint where it is.
	protected void OrderStop()
	{
		m_bOrdered = false;
		if (!HasBody() || !m_bWalks)
			return;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm)
			RZ_Util.RedirectGroup(m_Body, m_vPos, gm.GetSpawner().GetMoveWaypointPrefab());
	}

	// ================================================================ The body
	//------------------------------------------------------------------------------------------------
	protected void UpdateBody(RZ_GameModeComponent gm, float dt, array<IEntity> players)
	{
		float radius = 300;
		RZ_ContractSettings cs = gm.GetContractSettings();
		if (cs)
			radius = cs.m_fEscortBodyRadius;

		if (m_bHasBody)
		{
			if (!m_Body)
			{
				m_bHasBody = false;
				// Deleted by someone else after its members were seen: it died
				if (m_bSeenAlive && m_bWalks)
					m_eState = RZ_EEscortState.DEAD;

				return;
			}

			m_fBodyAge += dt;
			int alive = RZ_Util.CountAlive(m_Body);
			if (alive > 0)
				m_bSeenAlive = true;

			// Group members exist a moment after the group: only judge once they've been seen
			if (alive == 0 && (m_bSeenAlive || m_fBodyAge > 15))
			{
				m_eState = RZ_EEscortState.DEAD;
				return;
			}

			if (!m_bTagged)
				TryTag();

			if (m_bWalks)
				m_vPos = RZ_Util.GetUnitPosition(m_Body);

			// A figure that can't walk goes while they move (they move as data, unseen)
			bool moving = m_eState == RZ_EEscortState.MOVING;
			if (RZ_Util.NearestDistance(m_vPos, players) > radius + DESPAWN_MARGIN || (!m_bWalks && moving))
				Despawn();

			return;
		}

		if (RZ_Util.NearestDistance(m_vPos, players) > radius)
			return;

		// Without an AI group: only shown while standing
		bool walking = m_eState == RZ_EEscortState.MOVING;
		ResourceName prefab = m_sPrefab;
		bool group = false;
		if (!prefab.IsEmpty())
			group = IsGroupPrefab(prefab);

		if (!group)
		{
			if (walking)
				return;

			if (prefab.IsEmpty())
				prefab = gm.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);
		}

		Spawn(gm, prefab);
	}

	//------------------------------------------------------------------------------------------------
	//! Whether a prefab is an AI group: spawned once to find out, the answer kept per prefab.
	protected static bool IsGroupPrefab(ResourceName prefab)
	{
		if (!s_mGroupPrefabs)
			s_mGroupPrefabs = new map<string, bool>();

		bool known;
		if (s_mGroupPrefabs.Find(prefab, known))
			return known;

		// Unknown: assume a group; Spawn() records the truth
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Spawn(RZ_GameModeComponent gm, ResourceName prefab)
	{
		if (prefab.IsEmpty())
			return;

		vector at = m_vPos;
		if (RZ_Util.IsWater(at))
			at = RZ_Mission.LandPointNear(at, 3, 30);

		// The one body budget: a mission's people are always allowed, and counted so the infected
		// leave room for them
		RZ_Shared shared = gm.GetShared();
		if (shared && !shared.CanSpawnBody(RZ_BodyBudget.STORY, at))
			return;

		IEntity e = RZ_Util.SpawnPrefab(prefab, at, Math.RandomFloat(0, 360));
		if (!e)
			return;

		if (shared)
			shared.AddBody(RZ_BodyBudget.STORY, e);

		m_Body = e;
		m_bHasBody = true;
		m_bWalks = SCR_AIGroup.Cast(e) != null;
		m_bTagged = false;
		m_bSeenAlive = false;
		m_fBodyAge = 0;
		m_bOrdered = false;
		if (!s_mGroupPrefabs)
			s_mGroupPrefabs = new map<string, bool>();

		s_mGroupPrefabs.Set(prefab, m_bWalks);
		if (!m_bWalks)
			m_bSeenAlive = true;

		TryTag();
	}

	//------------------------------------------------------------------------------------------------
	//! The talk actions go on the body, or on a group's leader once the members exist.
	protected void TryTag()
	{
		if (m_iTagMission < 0 || !m_Body)
		{
			m_bTagged = true;
			return;
		}

		IEntity target = m_Body;
		SCR_AIGroup group = SCR_AIGroup.Cast(m_Body);
		if (group)
		{
			AIAgent leader = group.GetLeaderAgent();
			if (!leader || !leader.GetControlledEntity())
				return;

			target = leader.GetControlledEntity();
		}

		if (m_iTagObject >= 0)
			RZ_PuzzleObjectComponent.Tag(target, m_iTagMission, m_iTagObject, "", m_sChoiceLabels);

		if (m_iTagClue >= 0)
			RZ_ClueComponent.Tag(target, m_iTagMission, m_iTagClue, m_sReadLabel);

		m_bTagged = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Despawn()
	{
		if (m_Body)
			RZ_Util.DeleteUnit(m_Body);

		m_Body = null;
		m_bHasBody = false;
		m_bOrdered = false;
	}

	//------------------------------------------------------------------------------------------------
	//! "the refugees (300 m NE)" for messages.
	string Where(vector from)
	{
		return string.Format("%1 (%2)", m_sName, RZ_Util.DirectionText(from, m_vPos));
	}

	//------------------------------------------------------------------------------------------------
	//! Deletes the body and frees the slot. Call from the mission's Cleanup().
	void Cleanup()
	{
		Despawn();
		if (m_bCounted)
		{
			s_iRunning = Math.Max(0, s_iRunning - 1);
			m_bCounted = false;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Escort prefab for a kind: 0 civilians, 1 scientist, 2 soldier, with the fallbacks in
	//! RZ_ContractSettings. "" = data with a civilian figure.
	static ResourceName PrefabFor(RZ_GameModeComponent gm, int kind)
	{
		RZ_ContractSettings cs = gm.GetContractSettings();
		ResourceName prefab;
		if (cs)
		{
			if (kind == 1)
				prefab = cs.m_sEscortScientist;
			else if (kind == 2)
				prefab = cs.m_sEscortSoldier;

			if (prefab.IsEmpty())
				prefab = cs.m_sEscortCivilians;
		}

		if (prefab.IsEmpty())
			prefab = gm.GetSurvivorGroupPrefab();

		return prefab;
	}
}

//------------------------------------------------------------------------------------------------
//! Shared by escort missions.
class RZ_EscortHelp
{
	//------------------------------------------------------------------------------------------------
	//! A player within radius of pos (the first found), -1 if none: someone standing beside the
	//! escorted person, so a mission can go on even if the body can't be talked to.
	static int PlayerBeside(array<int> ids, array<IEntity> players, vector pos, float radius)
	{
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius)
				return ids[i];
		}

		return -1;
	}
}
