//! World events about people (docs/MISSIONS.md, "World events"): Distress Call and its Red Hand
//! twin, Prison Break, The Sniper, Rival Survivors, Collapse Shelter. AI groups are data until a
//! player comes near (RZ_EventGroup); people you talk to are the puzzle framework's objects.

//------------------------------------------------------------------------------------------------
//! An AI group an event spawns only once a player is near, and watches until it's dead.
class RZ_EventGroup
{
	ResourceName m_sPrefab;
	vector m_vPos;				// where it spawns (and, before then, where it is as data)
	IEntity m_Unit;
	bool m_bSpawned;
	float m_fGrace = 15;		// groups spawn their members a frame later: don't count them dead yet

	//------------------------------------------------------------------------------------------------
	void RZ_EventGroup(ResourceName prefab, vector pos)
	{
		m_sPrefab = prefab;
		m_vPos = pos;
	}

	//------------------------------------------------------------------------------------------------
	bool Spawn()
	{
		if (m_bSpawned)
			return m_Unit != null;

		m_bSpawned = true;
		m_Unit = RZ_Util.SpawnPrefab(m_sPrefab, m_vPos, Math.RandomFloat(0, 360));
		return m_Unit != null;
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt)
	{
		if (m_bSpawned && m_fGrace > 0)
			m_fGrace -= dt;
	}

	//------------------------------------------------------------------------------------------------
	bool IsDead()
	{
		if (!m_bSpawned || m_fGrace > 0)
			return false;

		return !m_Unit || RZ_Util.CountAlive(m_Unit) <= 0;
	}

	//------------------------------------------------------------------------------------------------
	vector GetPos()
	{
		if (m_Unit && RZ_Util.CountAlive(m_Unit) > 0)
			return RZ_Util.GetUnitPosition(m_Unit);

		return m_vPos;
	}

	//------------------------------------------------------------------------------------------------
	void Delete()
	{
		RZ_Util.DeleteUnit(m_Unit);
		m_Unit = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Every group spawned and dead.
	static bool AllDead(array<ref RZ_EventGroup> groups)
	{
		if (groups.IsEmpty())
			return false;

		foreach (RZ_EventGroup g : groups)
		{
			if (!g.IsDead())
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static int CountAlive(array<ref RZ_EventGroup> groups)
	{
		int n = 0;
		foreach (RZ_EventGroup g : groups)
		{
			if (!g.m_bSpawned || !g.IsDead())
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	static void DeleteAll(array<ref RZ_EventGroup> groups)
	{
		foreach (RZ_EventGroup g : groups)
		{
			g.Delete();
		}
	}
}

//------------------------------------------------------------------------------------------------
//! What both distress calls share: the same title and the same voice, so nobody can tell them
//! apart from the announcement. A Radio Operator can: they hear whether it's live or a loop.
class RZ_Event_DistressBase : RZ_PuzzleMission
{
	protected vector m_vSpot;
	protected string m_sCaller;
	protected ref array<int> m_aWarned = {};

	protected static const string RADIO_OP = "radioham";
	protected static const float HINT_RADIUS = 1500;

	//------------------------------------------------------------------------------------------------
	protected bool PickSpot(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 20, 85);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vSpot = LandPointNear(s.m_vCenter, s.m_fRadius * 0.5, s.m_fRadius + 150);
		m_vPos = m_vSpot;
		m_sCaller = RZ_NoteText.FirstName(m_Rng);
		m_sTitle = "Distress Call";
		m_sBrief = string.Format("A voice on the open band, from just outside %1: \"This is %2. There's two of us, one's hurt, and they're all round the house. Please. Anyone.\" It comes again every few minutes.", s.m_sName, m_sCaller);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! What a Radio Operator hears in it. Override.
	protected string RadioOpHint()
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Radio Operators online now, and any who come within range later, get their hint once.
	protected void HintRadioOps(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];
			if (m_aWarned.Contains(playerId) || !RZ_Interactions.IsBackground(gm, playerId, RADIO_OP))
				continue;

			if (RZ_Util.DistXZ(p.GetOrigin(), m_vSpot) > HINT_RADIUS && m_fClock > 5)
				continue;

			m_aWarned.Insert(playerId);
			gm.NotifyPlayer(playerId, m_sTitle + " (Radio Operator)", RadioOpHint());
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The voice on the band near " + GetLocationName(gm) + " has stopped.";
	}
}

//------------------------------------------------------------------------------------------------
//! #5 Distress Call: two survivors are cornered in a house at the edge of a town. Get there,
//! clear the infected from around it (they come once someone is close) and talk them out with
//! nothing within 20 m. They join the nearest safehouse and leave you their stash. 25 minutes.
class RZ_Event_DistressCall : RZ_Event_DistressBase
{
	protected bool m_bStirred;

	protected static const float CLEAR_RADIUS = 20;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 1500;
		m_fParticipationRadius = 40;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN) || gm.GetMissions().IsTypeRunning(RZ_Event_FakeDistress))
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		if (!PickSpot(gm))
			return false;

		int talk = AddClue(m_sCaller, string.Format("%1, through a barricaded door, very quietly: \"Are they gone? Tell me when they're gone. She can't run.\"", m_sCaller));
		return SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vSpot, 0, "", "Tell them it's clear", talk, "Talk to " + m_sCaller) != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected string RadioOpHint()
	{
		return "The voice breaks in different places every time, and once there's banging behind it. It's live. Someone really is out there.";
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		HintRadioOps(gm, ids, players);
		if (m_bStirred || CountPlayersNear(players, m_vSpot, 300) == 0)
			return;

		// The infected round the house, and more coming at the noise
		m_bStirred = true;
		SpawnGuards(gm, m_vSpot, 4, 6, 20);
		gm.GetSpawner().SpawnHorde(m_vSpot, 250, m_iId);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (gm.GetSpawner().IsZombieNear(m_vSpot, CLEAR_RADIUS))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"Not yet! I can hear them!\" The door stays shut. Clear them away from the house first.");
			return;
		}

		gm.NotifyPlayer(playerId, m_sTitle, string.Format("The barricade comes down a plank at a time. %1 comes out with a woman on his arm, grey and limping. \"There's a bag under the stairs. Take it. We won't need it where we're going.\"", m_sCaller));
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		SpawnRewardAt(gm, m_vSpot);
		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses && houses.Count() > 0)
			houses.AddRescued(m_vSpot, 2, m_aParticipants);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "Someone answered the distress call near " + GetLocationName(gm) + ". Two people walked out.";
	}
}

//------------------------------------------------------------------------------------------------
//! #19 Fake Distress Call: the same call, but it's a tape on a crate and the Red Hand is waiting
//! in the trees. Two Red Hand groups (Missions > Red Hand group) appear when a player is within
//! 400 m and close in when someone reaches the radio. They fight anyone near them except players
//! Known or better with the Red Hand. Kill them all for their kit. Radio Operators are warned.
class RZ_Event_FakeDistress : RZ_Event_DistressBase
{
	protected ref array<ref RZ_EventGroup> m_aGroups = {};
	protected bool m_bSprung;
	protected ref array<int> m_aSpared = {};

	protected static const float SPAWN_RADIUS = 400;
	protected static const float SPRING_RADIUS = 60;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 1500;
		m_fParticipationRadius = 80;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetRedHandGroupPrefab().IsEmpty() || !HasPrefab(gm, RZ_EPuzzleProp.OBJECT))
			return 0;

		if (gm.GetMissions().IsTypeRunning(RZ_Event_DistressCall))
			return 0;

		return 0.5;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		if (!PickSpot(gm))
			return false;

		int look = AddClue("A radio", string.Format("A radio on an upturned crate, a cassette player taped to the handset. \"This is %1. There's two of us...\" Click. Hiss. \"This is %1.\" Nobody's here. Nobody ever was.", m_sCaller));
		if (!SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vSpot, 0, "", "Switch it off", look, "Look at the radio"))
			return false;

		for (int i = 0; i < 2; i++)
		{
			m_aGroups.Insert(new RZ_EventGroup(gm.GetRedHandGroupPrefab(), LandPointNear(m_vSpot, 50, 90)));
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected string RadioOpHint()
	{
		return "Same breath in the same place, the same crackle after \"Anyone\", every time. That's a tape. The Red Hand baits people with recordings like this. Go in careful, or don't go.";
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		HintRadioOps(gm, ids, players);
		foreach (RZ_EventGroup g : m_aGroups)
		{
			g.Tick(dt);
			if (g.m_bSpawned || CountPlayersNear(players, m_vSpot, SPAWN_RADIUS) == 0)
				continue;

			if (g.Spawn())
				RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, g.m_Unit, 300, true);
		}

		if (!m_bSprung && CountPlayersNear(players, m_vSpot, SPRING_RADIUS) > 0)
			Spring(gm, ids, players);

		// Players the Red Hand counts as friends are waved off once
		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];
			if (m_aSpared.Contains(playerId) || RZ_Util.DistXZ(p.GetOrigin(), m_vSpot) > SPRING_RADIUS)
				continue;

			if (!RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.KNOWN))
				continue;

			m_aSpared.Insert(playerId);
			gm.NotifyPlayer(playerId, m_sTitle, "A whistle from the trees, then a voice you half know: \"Not you. Keep walking, and keep your mouth shut about this.\"");
		}

		if (RZ_EventGroup.AllDead(m_aGroups))
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	protected void Spring(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players)
	{
		m_bSprung = true;
		ResourceName wp = gm.GetSpawner().GetMoveWaypointPrefab();
		foreach (RZ_EventGroup g : m_aGroups)
		{
			if (g.m_Unit)
				RZ_Util.SendGroupTo(g.m_Unit, m_vSpot, wp);
		}

		RZ_ContractHelp.NotifyNear(gm, m_vSpot, 150, m_sTitle, "There's nobody here. Just a radio on a crate, playing the same words. Then a branch snaps in the treeline. It was never a call for help.");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		gm.NotifyPlayer(playerId, m_sTitle, "Click. The voice stops. The quiet after it is worse.");
		SetObjectLabels(0, "", "");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (RZ_EventGroup.AllDead(m_aGroups))
			SpawnRewardAt(gm, m_vSpot);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		RZ_EventGroup.DeleteAll(m_aGroups);
		super.Cleanup(gm);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		if (RZ_EventGroup.AllDead(m_aGroups))
			return "The distress call near " + GetLocationName(gm) + " was a Red Hand trap. The people who set it won't set another.";

		return "The distress call near " + GetLocationName(gm) + " has gone quiet. It was a Red Hand trap: a tape on a crate.";
	}
}

//------------------------------------------------------------------------------------------------
//! #11 Prison Break: convicts broke out of the holding cells in a town and are walking to the
//! Red Hand's nearest camp. Two groups (Missions > Events > Convicts, else Missions > Red Hand
//! group), data until a player is within 600 m. They fight anyone near them except players Known
//! or better with the Red Hand. Stop them all: what they took is where the last one fell. If any
//! reach the camp (or the time runs out), the Red Hand takes them in: a little stronger, and its
//! friends who walked in with them get a little Fame.
class RZ_Event_PrisonBreak : RZ_Mission
{
	protected ref array<ref RZ_EventGroup> m_aGroups = {};
	protected vector m_vTarget;
	protected string m_sTarget;
	protected bool m_bToRedHand;
	protected bool m_bRecruited;
	protected vector m_vLast;

	protected static const float BODY_RADIUS = 600;
	protected static const float ARRIVE_RADIUS = 150;
	protected static const float DATA_SPEED = 1.2;

	//------------------------------------------------------------------------------------------------
	static ResourceName Prefab(RZ_GameModeComponent gm)
	{
		ResourceName prefab = RZ_EventHelp.Settings(gm).m_sConvictGroupPrefab;
		if (prefab.IsEmpty())
			prefab = gm.GetRedHandGroupPrefab();

		return prefab;
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 1800;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (Prefab(gm).IsEmpty())
			return 0;

		return 0.6;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 90);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		array<string> kinds = {"police"};
		vector start = s.m_vCenter;
		RZ_Landmark cells = RZ_LandmarkFinder.FindNearest(s.m_vCenter, s.m_fRadius + 100, kinds);
		if (cells)
			start = cells.m_vPos;

		start = LandPointNear(start, 8, 30);

		// The Red Hand's nearest camp, else into the hills toward another town
		RZ_Territory territory = gm.GetTerritory();
		RZ_Holding camp;
		if (territory && territory.IsActive())
			camp = territory.FindHolding(RZ_EFaction.BANDITS, start, 8000);

		if (camp)
		{
			m_vTarget = camp.m_vPos;
			m_sTarget = camp.GetTitle();
			m_bToRedHand = true;
		}
		else
		{
			RZ_Settlement t = gm.GetSettlement(PickSettlementAway(gm, start, 1500, 4000, 0, 100));
			if (!t)
				return false;

			m_vTarget = t.m_vCenter;
			m_sTarget = t.m_sName;
		}

		for (int i = 0; i < 2; i++)
		{
			m_aGroups.Insert(new RZ_EventGroup(Prefab(gm), LandPointNear(start, 0, 15)));
		}

		m_vPos = start;
		m_vLast = start;
		m_sTitle = "Prison Break";
		m_sBrief = string.Format("The holding cells in %1 didn't hold. A dozen convicts are out and armed with whatever they found, walking toward %2. The Red Hand will take in anyone who gets there. Stop them on the road, or stay out of their way.", s.m_sName, m_sTarget);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		bool anySpawned = false;
		foreach (RZ_EventGroup g : m_aGroups)
		{
			g.Tick(dt);
			if (g.m_bSpawned)
			{
				anySpawned = true;
				if (!g.IsDead())
					m_vLast = g.GetPos();

				continue;
			}

			// As data they walk on toward the camp
			vector dir = m_vTarget - g.m_vPos;
			dir[1] = 0;
			float len = dir.Length();
			if (len > 1)
				g.m_vPos = RZ_Util.GroundPos(g.m_vPos + dir * (Math.Min(len, DATA_SPEED * dt) / len));

			m_vLast = g.m_vPos;
			if (RZ_Util.NearestDistance(g.m_vPos, players) > BODY_RADIUS)
				continue;

			if (g.Spawn())
			{
				RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, g.m_Unit, 250, true);
				RZ_Util.SendGroupTo(g.m_Unit, m_vTarget, gm.GetSpawner().GetMoveWaypointPrefab());
			}
		}

		m_vPos = m_vLast;
		if (RZ_EventGroup.AllDead(m_aGroups))
		{
			RZ_ContractHelp.AddNear(ids, players, m_vLast, 250, m_aParticipants);
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (RZ_Util.DistXZ(m_vLast, m_vTarget) <= ARRIVE_RADIUS)
			Recruit(gm, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	protected void Recruit(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players)
	{
		m_bRecruited = true;
		m_eState = RZ_EMissionState.SUCCEEDED;
		if (!m_bToRedHand)
			return;

		int alive = RZ_EventGroup.CountAlive(m_aGroups);
		RZ_Territory territory = gm.GetTerritory();
		if (territory)
			territory.AddStrength(RZ_EFaction.BANDITS, 1.5 * alive);

		if (gm.GetLeaders())
			gm.GetLeaders().AddHeadline(string.Format("The Red Hand has new faces at %1: convicts who walked out of the cells in %2.", m_sTarget, GetSettlementName(gm)), false);

		// The Red Hand's friends who walked in with them
		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];
			if (RZ_Util.DistXZ(p.GetOrigin(), m_vLast) > 250 || !RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.KNOWN))
				continue;

			gm.AddFame(playerId, RZ_EFaction.BANDITS, 4);
			gm.NotifyPlayer(playerId, "The Red Hand", "You walked them in. The Red Hand notices who brings it new hands: +4 Fame.");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string GetSettlementName(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
			return s.m_sName;

		return "town";
	}

	//------------------------------------------------------------------------------------------------
	override string GetLocationName(RZ_GameModeComponent gm)
	{
		return RZ_SettlementScanner.GridReference(m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		// They made it off the roads: the Red Hand finds them sooner or later
		array<int> noIds = {};
		array<IEntity> noPlayers = {};
		Recruit(gm, noIds, noPlayers);
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (!m_bRecruited)
			SpawnRewardCrateAt(gm, m_vLast);
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnRewardCrateAt(RZ_GameModeComponent gm, vector pos)
	{
		gm.SpawnRewardCrate(LandPointNear(pos, 1, 4));
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		RZ_EventGroup.DeleteAll(m_aGroups);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		if (!m_bRecruited)
			return "The convicts from " + GetSettlementName(gm) + " never made it. What they took is lying where the last one fell.";

		if (m_bToRedHand)
			return "The convicts from " + GetSettlementName(gm) + " reached the Red Hand. It has more guns tonight.";

		return "The convicts from " + GetSettlementName(gm) + " have scattered into the hills.";
	}
}

//------------------------------------------------------------------------------------------------
//! #14 The Sniper: someone has gone mad on the high ground above a town and shoots at anything
//! that moves. One armed AI (Missions > Events > Sniper, else Contracts > Bounty target), there
//! only while a player is within 900 m; he fights everyone near him, Red Hand friends included.
//! His shots draw infected. Take him down: his stash is up there. After 30 minutes he moves on.
class RZ_Event_TheSniper : RZ_Mission
{
	protected ref RZ_EventGroup m_Sniper;

	protected static const float BODY_RADIUS = 900;
	protected static const float FIGHT_RADIUS = 500;

	//------------------------------------------------------------------------------------------------
	static ResourceName Prefab(RZ_GameModeComponent gm)
	{
		ResourceName prefab = RZ_EventHelp.Settings(gm).m_sSniperPrefab;
		if (prefab.IsEmpty() && gm.GetContractSettings())
			prefab = gm.GetContractSettings().m_sBountyTargetPrefab;

		return prefab;
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 1800;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (Prefab(gm).IsEmpty())
			return 0;

		return 0.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		if (Prefab(gm).IsEmpty())
			return false;

		m_iSettlement = RZ_EventHelp.PickEmptyTown(gm, 0, 90);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = HighPointNear(s.m_vCenter, s.m_fRadius + 80, s.m_fRadius + 450);
		m_Sniper = new RZ_EventGroup(Prefab(gm), m_vPos);
		m_sTitle = "The Sniper";
		m_sBrief = string.Format("Someone's been shooting from the high ground above %1 since dawn. Not infected, not any faction's. Whoever it is has a stash up there and nothing left to lose. Come at him from cover.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_Sniper.Tick(dt);
		if (!m_Sniper.m_bSpawned)
		{
			if (CountPlayersNear(players, m_vPos, BODY_RADIUS) == 0)
				return;

			if (!m_Sniper.Spawn())
			{
				m_eState = RZ_EMissionState.FAILED;
				return;
			}

			// The sniper prefab is a Red Hand (BAND) group; the bounty target it falls back to is on
			// SURV, as Collect the Bounty needs. Mark the fight for the faction he's really on.
			RZ_EFaction side = RZ_EFaction.BANDITS;
			if (RZ_EventHelp.Settings(gm).m_sSniperPrefab.IsEmpty())
				side = RZ_EFaction.SURVIVORS;

			RZ_Hostility.MarkFight(side, m_Sniper.m_Unit, FIGHT_RADIUS, false);
			return;
		}

		if (m_Sniper.IsDead())
		{
			RZ_ContractHelp.AddNear(ids, players, m_vPos, 300, m_aParticipants);
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		// His shots carry
		if (CountPlayersNear(players, m_vPos, FIGHT_RADIUS) > 0)
			gm.GetHeat().AddHeat(m_vPos, dt);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (m_Sniper.IsDead())
			gm.SpawnRewardCrate(LandPointNear(m_vPos, 1, 4));
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		if (m_Sniper)
			m_Sniper.Delete();
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		if (m_Sniper.IsDead())
			return "The shooting above " + GetLocationName(gm) + " has stopped for good. His stash is still up there.";

		return "The shooting above " + GetLocationName(gm) + " has stopped. He's moved on.";
	}
}

//------------------------------------------------------------------------------------------------
//! #22 Rival Survivors: an unaligned group camps outside a town. How they treat you depends on
//! your reputation overall: Hunted by anyone, or Watched by two factions, and they fight (only
//! you); Trusted by someone and Watched by nobody, and they trade for 2 food; anyone else pays
//! 3. Two trades in all, one per player, each a crate of their spare kit. Gun them down when
//! they hadn't turned on anyone and the Haven hears of it (+6 Infamy); if they had, their kit is
//! yours. Group: Missions > Events > Rival group, else Missions > Survivor group.
class RZ_Event_RivalSurvivors : RZ_PuzzleMission
{
	protected ref RZ_EventGroup m_Group;
	protected vector m_vCamp;
	protected bool m_bProvoked;
	protected int m_iTrades;
	protected ref array<int> m_aTraded = {};
	protected ref array<int> m_aMarked = {};

	protected static const float BODY_RADIUS = 500;
	protected static const float NOTICE_RADIUS = 120;
	protected static const int MAX_TRADES = 2;
	protected static const int MURDER_INFAMY = 6;

	//------------------------------------------------------------------------------------------------
	static ResourceName Prefab(RZ_GameModeComponent gm)
	{
		ResourceName prefab = RZ_EventHelp.Settings(gm).m_sRivalGroupPrefab;
		if (prefab.IsEmpty())
			prefab = gm.GetSurvivorGroupPrefab();

		return prefab;
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 1800;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (Prefab(gm).IsEmpty() || !HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return 0;

		return 0.7;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		if (Prefab(gm).IsEmpty())
			return false;

		m_iSettlement = RZ_EventHelp.PickEmptyTown(gm, 0, 70);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vCamp = LandPointNear(s.m_vCenter, s.m_fRadius + 50, s.m_fRadius + 300);
		m_vPos = m_vCamp;
		string leader = RZ_NoteText.FirstName(m_Rng);
		int look = AddClue(leader, string.Format("%1 does the talking for them: five or six people, nobody's colours, rifles held low. They watch your hands the whole time. \"We've heard of most people. Depends what we've heard.\"", leader));
		if (!SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, LandPointNear(m_vCamp, 3, 8), 0, "", "Trade (food for some of their kit)|Ask what they've seen", look, "Talk to " + leader))
			return false;

		m_Group = new RZ_EventGroup(Prefab(gm), m_vCamp);
		m_sTitle = "Rival Survivors";
		m_sBrief = string.Format("A group nobody's heard from has made camp outside %1. They answer to no faction. They'll trade with people they've heard good things about. The others, they shoot.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! -1 they fight you, 0 wary (trade dearer), 1 they like what they've heard.
	protected int Attitude(RZ_GameModeComponent gm, int playerId)
	{
		int watched = 0;
		bool trusted = false;
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			RZ_ETier tier = gm.GetTier(playerId, f);
			if (RZ_Reputation.IsHunted(tier))
				return -1;

			if (RZ_Reputation.IsWatchedOrWorse(tier))
				watched++;
			else if (RZ_Reputation.IsAtLeast(tier, RZ_ETier.TRUSTED))
				trusted = true;
		}

		if (watched >= 2)
			return -1;

		if (trusted && watched == 0)
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_Group.Tick(dt);
		if (!m_Group.m_bSpawned)
		{
			if (CountPlayersNear(players, m_vCamp, BODY_RADIUS) > 0)
				m_Group.Spawn();

			return;
		}

		if (m_Group.IsDead())
		{
			OnGroupDead(gm, ids, players);
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		// People they've heard bad things about: they fight them, and only them
		RZ_Hostility hostility = gm.GetHostility();
		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];
			if (m_aMarked.Contains(playerId) || RZ_Util.DistXZ(p.GetOrigin(), m_vCamp) > NOTICE_RADIUS)
				continue;

			if (Attitude(gm, playerId) >= 0)
				continue;

			m_aMarked.Insert(playerId);
			m_bProvoked = true;
			if (hostility && m_Group.m_Unit)
				hostility.MarkHostileAround(RZ_EFaction.SURVIVORS, m_Group.m_Unit, 200, playerId);

			gm.NotifyPlayer(playerId, m_sTitle, "Someone in the camp says your name, and every rifle comes up at once. They've heard of you.");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGroupDead(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players)
	{
		array<int> near = {};
		GetPlayersNear(ids, players, m_vCamp, 250, near);
		foreach (int playerId : near)
		{
			if (!m_aParticipants.Contains(playerId))
				m_aParticipants.Insert(playerId);

			if (m_bProvoked)
				continue;

			gm.AddInfamy(playerId, RZ_EFaction.SURVIVORS, MURDER_INFAMY);
			gm.NotifyPlayer(playerId, "The Haven", string.Format("They came to trade, and somebody gunned them down. Word gets around: +%1 Infamy with the Haven.", MURDER_INFAMY));
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		int attitude = Attitude(gm, playerId);
		if (attitude < 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"We know who you are. Walk away while we're letting you.\"");
			return;
		}

		if (option == 1)
		{
			gm.NotifyPlayer(playerId, m_sTitle, SeenText(gm));
			return;
		}

		if (m_iTrades >= MAX_TRADES || m_aTraded.Contains(playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"That's all we can spare. We've a long way to go.\"");
			return;
		}

		int cost = 3;
		if (attitude > 0)
			cost = 2;

		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		array<string> keywords = {};
		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses)
			keywords = houses.GetFoodKeywords();
		else
			RZ_FoodHelper.DefaultKeywords(keywords);

		if (!user || RZ_ItemHelper.Count(user, keywords) < cost)
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("\"%1 tins, or whatever you've got that's food. Then we talk.\"", cost));
			return;
		}

		RZ_FoodHelper.TakeFood(user, keywords, cost);
		m_iTrades++;
		m_aTraded.Insert(playerId);
		AddParticipant(playerId);
		gm.SpawnRewardCrate(LandPointNear(m_vCamp, 2, 5));
		gm.NotifyPlayer(playerId, m_sTitle, "They count the food twice and push a crate across with their boots. \"Fair. Don't follow us.\"");
	}

	//------------------------------------------------------------------------------------------------
	//! The most infected town within 3 km: where not to go.
	protected string SeenText(RZ_GameModeComponent gm)
	{
		RZ_Settlement worst;
		foreach (RZ_Settlement t : gm.GetSettlements())
		{
			if (RZ_Util.DistXZ(t.m_vCenter, m_vCamp) > 3000)
				continue;

			if (!worst || t.m_fInfection > worst.m_fInfection)
				worst = t;
		}

		if (!worst)
			return "\"Nothing worth telling. It's quiet. Too quiet, if you ask me.\"";

		return string.Format("\"Stay out of %1. We went in for water and left two behind. It's crawling.\"", worst.m_sName);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (m_Group.IsDead() && m_bProvoked)
			gm.SpawnRewardCrate(LandPointNear(m_vCamp, 1, 4));
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		if (m_Group)
			m_Group.Delete();

		super.Cleanup(gm);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		if (m_Group.IsDead())
			return "The camp outside " + GetLocationName(gm) + " is empty now. Nobody from it walked away.";

		return "The group outside " + GetLocationName(gm) + " has packed up and moved on.";
	}
}

//------------------------------------------------------------------------------------------------
//! #28 Collapse Shelter: a town's shelter (its school or church) is overrun and four survivors
//! have scattered round the town, each hiding with a couple of infected close by. Find them
//! and tell them the way to the safehouse (nothing within 15 m of them) before they turn: the
//! first in 15 minutes, the others a little later. Each one saved joins the nearest safehouse.
//! The Haven pays its Fame and Favor to the rescuers, scaled by how many were saved.
class RZ_Event_CollapseShelter : RZ_PuzzleMission
{
	protected vector m_vShelter;
	protected ref array<vector> m_aPeople = {};
	protected ref array<IEntity> m_aBodies = {};
	protected ref array<float> m_aLeft = {};
	protected ref array<int> m_aState = {};		// 0 hiding, 1 saved, 2 turned
	protected ref array<bool> m_aGuarded = {};
	protected bool m_bStirred;
	protected int m_iSaved;
	protected int m_iLost;

	protected static const int PEOPLE = 4;
	protected static const float CLEAR_RADIUS = 15;
	protected static const float GUARD_RADIUS = 300;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(true, RZ_EFaction.SURVIVORS, 12, 8);
		m_fTimeLeft = 1800;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses || houses.Count() == 0 || !HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = RZ_EventHelp.PickEmptyTown(gm, 30, 90);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		array<string> kinds = {"school", "church"};
		RZ_Landmark l = RZ_LandmarkFinder.FindNearest(s.m_vCenter, s.m_fRadius, kinds);
		string shelter = "the town hall";
		m_vShelter = s.m_vCenter;
		if (l)
		{
			m_vShelter = l.m_vPos;
			shelter = l.m_sName;
		}

		for (int i = 0; i < PEOPLE; i++)
		{
			vector pos = LandPointNear(m_vShelter, 80, 350);
			string name = RZ_NoteText.FirstName(m_Rng);
			int talk = AddClue(name, string.Format("%1 is pressed into a doorway, shaking. \"They came through the shelter in the night. I ran. I don't know where the others went. Where do I go?\"", name));
			IEntity body = SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, pos, i, "", "Tell them the way to the safehouse", talk, "Talk to " + name);
			if (!body)
				return false;

			m_aPeople.Insert(pos);
			m_aBodies.Insert(body);
			m_aLeft.Insert(900 + i * 150);
			m_aState.Insert(0);
			m_aGuarded.Insert(false);
		}

		m_vPos = m_vShelter;
		m_sTitle = "Collapse Shelter";
		m_sBrief = string.Format("The shelter in %1 (%2) was overrun in the night. Four people got out and scattered round the town. They're hiding, and some of them are bitten. Find them before they turn and point them to a safehouse.", s.m_sName, shelter);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string where = "";
		foreach (int i, vector p : m_aPeople)
		{
			if (m_aState[i] != 0)
				continue;

			if (!where.IsEmpty())
				where += ", ";

			where += RZ_SettlementScanner.GridReference(p);
		}

		return super.GetWhereText(gm, from) + "\nLast seen (grid): " + where;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bStirred && CountPlayersNear(players, m_vShelter, 400) > 0)
		{
			m_bStirred = true;
			SpawnGuards(gm, m_vShelter, 5, 5, 25);
		}

		foreach (int i, vector p : m_aPeople)
		{
			if (m_aState[i] != 0)
				continue;

			if (!m_aGuarded[i] && CountPlayersNear(players, p, GUARD_RADIUS) > 0)
			{
				m_aGuarded[i] = true;
				SpawnGuards(gm, p, 2, 12, 25);
			}

			m_aLeft[i] = m_aLeft[i] - dt;
			if (m_aLeft[i] <= 0)
				Turn(gm, i);
		}

		if (m_iSaved + m_iLost >= PEOPLE)
			Finish();
	}

	//------------------------------------------------------------------------------------------------
	protected void Turn(RZ_GameModeComponent gm, int i)
	{
		m_aState[i] = 2;
		m_iLost++;
		RZ_Util.DeleteUnit(m_aBodies[i]);
		gm.GetSpawner().SpawnZombie(m_aPeople[i], false, m_iSettlement, m_iId);
		RZ_ContractHelp.NotifyNear(gm, m_aPeople[i], 600, m_sTitle, "Somewhere close a scream turns into something else. One of the people from the shelter didn't make it.");
	}

	//------------------------------------------------------------------------------------------------
	protected void Finish()
	{
		if (m_iSaved > 0)
			m_eState = RZ_EMissionState.SUCCEEDED;
		else
			m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx < 0 || objIdx >= m_aPeople.Count() || m_aState[objIdx] != 0)
			return;

		vector p = m_aPeople[objIdx];
		if (gm.GetSpawner().IsZombieNear(p, CLEAR_RADIUS))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "They won't move with those things that close. Clear them off first.");
			return;
		}

		m_aState[objIdx] = 1;
		m_iSaved++;
		AddParticipant(playerId);
		RZ_Util.DeleteUnit(m_aBodies[objIdx]);
		array<int> rescuer = {playerId};
		gm.GetSafehouses().AddRescued(p, 1, rescuer);
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("They nod, and go, keeping low. %1 of %2 found.", m_iSaved + m_iLost, PEOPLE));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		foreach (int i, int state : m_aState)
		{
			if (state == 0)
				Turn(gm, i);
		}

		Finish();
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		m_fRewardScale = m_iSaved;
		m_fRewardScale = m_fRewardScale / PEOPLE;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return string.Format("%1 of the %2 people from the shelter in %3 reached safety.", m_iSaved, PEOPLE, GetLocationName(gm));
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Nobody from the shelter in " + GetLocationName(gm) + " made it.";
	}
}
