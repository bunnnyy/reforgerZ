//! Task Force Bastion (Military) contracts. See docs/MISSIONS.md, "Task Force Bastion".

//------------------------------------------------------------------------------------------------
//! #1 Retake the Town (Take Back the Town merged in): kill enough zombies inside an infested town.
//! The giver depends on Bastion's presence: Bastion (anyone) while it's around, the Haven (Known)
//! after it pulls out. ★ Police/Soldier: +25% Fame. Its fallout line: "The town stays theirs."
class RZ_Mission_ClearSettlement : RZ_Mission
{
	protected int m_iKillsNeeded;
	protected int m_iKills;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		if (gm.GetDirector().GetStageSettings().m_fMilitaryPresence >= 0.3)
		{
			SetContract(RZ_EFaction.MILITARY, RZ_ETier.STRANGER, 15, 15);
			m_Req.m_fMinMilitary = 0.3;
			m_bPresenceWeighted = true;
			m_Req.Bonus("police");
			m_Req.Bonus("soldier");
			m_fBonusFameMult = 1.25;
		}
		else
		{
			SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 15, 15);
			m_Req.m_fMaxMilitary = 0.29;
		}

		m_fTimeLeft = 3600;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (m_eGiver == RZ_EFaction.SURVIVORS)
			return 2;

		return 2 * gm.GetDirector().GetStageSettings().m_fMilitaryPresence + 0.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		// A town to take back is one nobody lives in
		m_iSettlement = PickEmptySettlement(gm, 50, 100, this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = s.m_vCenter;
		m_fParticipationRadius = s.m_fRadius + 100;
		m_iKillsNeeded = 8 * s.m_iSize;
		SetText(s);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetText(RZ_Settlement s)
	{
		if (m_eGiver == RZ_EFaction.SURVIVORS)
		{
			m_sTitle = "Take Back " + s.m_sName;
			m_sBrief = string.Format("The families who lived in %1 have been sleeping on other people's floors since the first week, and they want to go home. The town is still full of infected. Put down %2 of them inside it, and we can start walking people back in.", s.m_sName, m_iKillsNeeded);
			m_sObjective = string.Format("Put down %1 infected inside %2.", m_iKillsNeeded, s.m_sName);
			return;
		}

		m_sTitle = "Retake " + s.m_sName;
		m_sBrief = string.Format("%1 has gone over to the infected, and every day it stays that way it spreads to the roads round it. The stronghold wants it cleared. Put down %2 of them inside the town. Count your rounds: we can't spare many.", s.m_sName, m_iKillsNeeded);
		m_sObjective = string.Format("Put down %1 infected inside %2.", m_iKillsNeeded, s.m_sName);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The town stays theirs.";
	}

	//------------------------------------------------------------------------------------------------
	override void OnZombiesKilled(RZ_GameModeComponent gm, int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		if (settlementIdx != m_iSettlement)
			return;

		m_iKills += count;
		if (m_iKills >= m_iKillsNeeded)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return;

		s.AddInfection(-35);
		gm.NotifyAll("Town reclaimed", string.Format("%1 has been pushed back to %2: fewer infected in its streets tonight, and less of it spreading to the roads round it.", s.m_sName, s.GetStateName()));
	}
}

//------------------------------------------------------------------------------------------------
//! #2 Last Helicopter: Bastion's one working helicopter shuttles people and stock
//! between its own bases inside the Zone. It sets down at an LZ outside a town and
//! lifts off at a fixed time. Everyone at the LZ when it leaves is paid for keeping it
//! clear (Fame and Favor only: nothing is thrown out of the helicopter). Everyone
//! converges on one spot, so expect PvP. Only while Bastion is still around.
class RZ_Mission_LastHelicopter : RZ_Mission
{
	protected bool m_bWarned;
	protected bool m_bLanded;

	//------------------------------------------------------------------------------------------------
	//! The helicopter is the run's (a claim, docs/DESIGN.md "World facts")
	override void GetClaims(RZ_GameModeComponent gm, notnull array<string> result)
	{
		result.Insert(RZ_WorldFacts.HELI);
	}

	protected static const float LZ_RADIUS = 40;
	protected static const float COUNTDOWN = 720;
	protected static const float GROUND_TIME = 180;	// it lands this long before it leaves

	//! The Last Pilot's ending: the helicopter runs twice as often until this mission-manager clock time
	protected static float s_fBoostUntil;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.STRANGER, 25, 20);
		m_Req.m_fMinMilitary = 0.2;
		m_Req.m_bNeedsHeli = true;
		m_bPresenceWeighted = true;
		m_fTimeLeft = COUNTDOWN;
		m_fParticipationRadius = 0; // decided at departure, see OnUpdate
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		// Storms ground it (docs/DESIGN.md, "Time and environment")
		if (gm.GetEnvironment() && gm.GetEnvironment().IsHelicopterGrounded())
			return 0;

		// Grounded or lost in The Last Pilot (or its pilot lost in Rescue the Pilot), or that
		// questline has the helicopter right now: no flights
		if (RZ_Quest_LastPilot.IsHelicopterDown(gm) || RZ_Quest_LastPilot.IsHelicopterBusy(gm))
			return 0;

		float w = 3 * gm.GetDirector().GetStageSettings().m_fMilitaryPresence;
		if (gm.GetMissions().GetClock() < s_fBoostUntil)
			w = w * 2;

		return w;
	}

	//------------------------------------------------------------------------------------------------
	//! More runs for a while (not saved).
	static void Boost(RZ_GameModeComponent gm, float seconds)
	{
		s_fBoostUntil = Math.Max(s_fBoostUntil, gm.GetMissions().GetClock() + seconds);
	}

	//------------------------------------------------------------------------------------------------
	//! A new mission manager (a new game on this server) starts with no boost.
	static void ResetBoost()
	{
		s_fBoostUntil = 0;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlementNotHeldBy(gm, 0, 70, RZ_Factions.Bit(RZ_EFaction.BANDITS), this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		// LZ on the edge of town where a helicopter could land
		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius, s.m_fRadius + 150);
		m_sTitle = "Last Helicopter";
		m_sBrief = string.Format("Our last working helicopter is setting down at the LZ outside %1 on its run between posts, with people and stock aboard. It lifts off in %2 minutes. The rotors bring in every infected for a kilometre, and anyone else who wants a look at what's aboard. Keep the LZ clear and be there when it goes.",
			s.m_sName, RZ_Util.RoundToInt(COUNTDOWN / 60));
		m_sObjective = string.Format("Keep the LZ outside %1 clear until the helicopter lifts off.", s.m_sName);

		// The first run after a grounding says what changed (a new pilot, a patched airframe)
		m_sBrief = RZ_Quest_LastPilot.BackLine(gm) + m_sBrief;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The last pilot notices who kept his LZ (The Last Pilot questline can start).
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Quest_LastPilot.OnLzKept(gm);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bLanded && m_fTimeLeft < GROUND_TIME)
		{
			m_bLanded = true;
			NotifyInterested(gm, "Last Helicopter", "The helicopter is down at the LZ outside " + GetLocationName(gm) + ". Three minutes on the ground.");
		}

		// The rotors are loud
		if (m_bLanded)
			gm.GetHeat().AddHeat(m_vPos, 4 * dt);

		if (!m_bWarned && m_fTimeLeft < 60)
		{
			m_bWarned = true;
			NotifyInterested(gm, "Last Helicopter", "One minute until the helicopter lifts off from " + GetLocationName(gm) + ".");
		}

		if (m_fTimeLeft > 0)
			return;

		m_aParticipants.Clear();
		GetPlayersNear(ids, players, m_vPos, LZ_RADIUS, m_aParticipants);

		if (m_aParticipants.IsEmpty())
			m_eState = RZ_EMissionState.FAILED;
		else
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The helicopter is up and gone from " + GetLocationName(gm) + ". Bastion noticed who kept the LZ clear.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The helicopter left " + GetLocationName(gm) + " with nobody holding the LZ.";
	}
}

//------------------------------------------------------------------------------------------------
//! #3 Burn the Bodies: burn corpse piles in a town to slow the infection. Stand at
//! each pile for 10 s. Amends: works off Bastion Infamy. ★ Nurse: faster.
//! The piles are Missions > Corpse pile, else Puzzles > Puzzle object (look only): without
//! either nobody could find them, so it never appears.
//! Decontamination merged in: the giver depends on the place. One time in three, while the lab
//! stands, it's Halcyon's (Trusted): four piles round the lab, the lab's town -15 infection.
//! The Authors questline's stage 5 forces the lab's version.
class RZ_Mission_BurnTheBodies : RZ_Mission_VisitPoints
{
	protected ref array<IEntity> m_aPiles = {};
	protected float m_fInfectionDrop = 10;
	protected bool m_bAtLab;
	protected bool m_bForceLab;

	//------------------------------------------------------------------------------------------------
	//! At the lab it holds the lab (a claim, docs/DESIGN.md "World facts"), so no other story uses it
	override void GetClaims(RZ_GameModeComponent gm, notnull array<string> result)
	{
		if (m_bAtLab && IsStandalone())
			result.Insert(RZ_WorldFacts.LAB);
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		m_bAtLab = gm.HasLab() && (m_bForceLab || Math.RandomInt(0, 3) == 0);
		if (m_bAtLab)
		{
			SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.TRUSTED, 20, 20);
			m_fInfectionDrop = 15;
		}
		else
		{
			SetContract(RZ_EFaction.MILITARY, RZ_ETier.STRANGER, 15, 15);
			m_bAmends = true;
			m_Req.m_bAllowWatched = true;
		}

		m_Req.Bonus("nurse");
		m_Req.Bonus("doctor");
		m_fBonusVisitSpeed = 1.5;
		m_fVisitRadius = 5;
		m_fVisitSeconds = 10;
		m_bHandsOn = true;	// burning a pile you can see: at it, not near it
		m_sPointName = "Pile";
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (RZ_ContractHelp.CorpsePilePrefab(gm).IsEmpty())
			return 0;

		return 1.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		if (m_bAtLab)
		{
			m_iSettlement = gm.GetLabSettlement();
			m_vPos = gm.GetLabPosition();
			m_fParticipationRadius = 150;
			PlacePiles(gm, m_vPos, 20, 100, 4);
			m_sTitle = "Burn the Bodies";
			m_sDeed = "burnt the dead round the lab";
			m_sBrief = "The dead are stacking up round the Halcyon lab: four piles, from the nights its guards held the fence. Left there, they draw more in, and the lab can't afford a siege. Burn all four. The fire is seen for miles, so don't stand admiring it.";
			m_sObjective = "Burn the four piles of bodies round the lab in {lab_town}.";
			return true;
		}

		m_iSettlement = PickEmptySettlement(gm, 30, 90, this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = s.m_vCenter;
		m_fParticipationRadius = s.m_fRadius + 50;
		PlacePiles(gm, s.m_vCenter, 0, s.m_fRadius * 0.7, 5);
		m_sTitle = "Burn the Bodies";
		m_sDeed = "burnt the dead in {town}";
		m_sBrief = string.Format("We put down a lot of infected in %1 and had no time to bury them. There are five piles in the streets now, and they spread it to anything that feeds there. Burn all five. Fire draws attention: expect company while it burns.", s.m_sName);
		m_sObjective = string.Format("Burn the five piles of bodies in %1.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void PlacePiles(RZ_GameModeComponent gm, vector center, float minR, float maxR, int count)
	{
		for (int i = 0; i < count; i++)
		{
			vector p = LandPointNear(center, minR, maxR);
			AddPoint(p);
			m_aPiles.Insert(RZ_Util.SpawnPrefab(RZ_ContractHelp.CorpsePilePrefab(gm), RZ_Util.GroundPos(p), Math.RandomFloat(0, 360)));
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnPointDone(RZ_GameModeComponent gm, int idx)
	{
		gm.GetHeat().AddHeat(m_aPoints[idx], 10); // fire draws attention
		if (idx < m_aPiles.Count())
			RZ_Util.DeleteUnit(m_aPiles[idx]);
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
			s.AddInfection(-m_fInfectionDrop);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		foreach (IEntity pile : m_aPiles)
		{
			RZ_Util.DeleteUnit(pile);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! #4 Recon: visit three observation points on high ground and report. Amends:
//! works off Bastion Infamy. ★ Hunter/Radio Op: reports count double (double Fame).
//! On success, Bastion shares where the next horde is forming.
class RZ_Mission_Recon : RZ_Mission_VisitPoints
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.STRANGER, 15, 15);
		m_bAmends = true;
		m_Req.m_bAllowWatched = true;
		m_Req.Bonus("hunter");
		m_Req.Bonus("radioham");
		m_fBonusFameMult = 2;
		m_fVisitRadius = 15;
		m_fVisitSeconds = 15;
		m_sPointName = "Observation point";
		m_fTimeLeft = 3000;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		// Eyes on someone else's hills, never Bastion's own
		m_iSettlement = PickSettlementNotHeldBy(gm, 0, 100, RZ_Factions.Bit(RZ_EFaction.MILITARY), this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = s.m_vCenter;
		m_fParticipationRadius = 0; // only those who reach a point
		for (int i = 0; i < 3; i++)
		{
			AddPoint(HighPointNear(s.m_vCenter, s.m_fRadius + 200, s.m_fRadius + 900));
		}

		m_sTitle = "Recon";
		m_sDeed = "scouted {town} for us";
		m_sBrief = string.Format("We have no eyes on the hills round %1, and the next horde could be forming there for all we know. Get up to three observation points on the high ground and watch from each for 15 seconds. What you see tells the stronghold where to send the guns.", s.m_sName);
		m_sObjective = string.Format("Watch from three observation points on the hills round %1.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		// With the packs: the hungriest of them, in Bastion's word, with its place
		if (gm.GetTribes())
		{
			string pack = gm.GetTribes().GetPeople().Hungriest();
			if (!pack.IsEmpty())
			{
				NotifyParticipants(gm, "Bastion intel", pack);
				return;
			}
		}

		vector hot;
		float heat;
		if (!gm.GetHeat().GetHottest(hot, heat))
		{
			NotifyParticipants(gm, "Bastion intel", "All quiet out there. No hordes forming.");
			return;
		}

		RZ_Settlement s = gm.GetSettlement(gm.FindNearestSettlement(hot));
		string where = RZ_SettlementScanner.GridReference(hot);
		if (s)
			where = s.m_sName + " (" + where + ")";

		NotifyParticipants(gm, "Bastion intel", "Your reports point to infected massing near " + where + ".");
	}
}

//------------------------------------------------------------------------------------------------
//! #5 Checkpoint Duty (Quarantine Line merged in): hold a checkpoint at an infested town's edge for
//! 10 minutes against waves; the town stops spreading for 30 minutes. The Red Hand notices.
//! ★ Police: holds faster.
class RZ_Mission_CheckpointDuty : RZ_Mission_Hold
{
	protected int m_iWhere;		// the one choice: 0 turned back, 1 let through

	//------------------------------------------------------------------------------------------------
	void RZ_Mission_CheckpointDuty()
	{
		m_Story = new RZ_MissionStory("barrier");
		m_Choice = new RZ_OneChoice();
		m_Choice.Set("The line held. Now a dozen people stand at the barrier with bags: \"We're not sick. Look at us.\" The sergeant says turn them back. He also says it's your call, since you're the one with the rifle on it.",
			"Turn them back|Let them through", "decide who passes the barrier", 0);
		m_Choice.Asset(1, RZ_CureFinale.INDEP);
	}

	//------------------------------------------------------------------------------------------------
	override void OnChosen(RZ_GameModeComponent gm, int decider, int option)
	{
		m_iWhere = option;
		string value = "turned back";
		if (option == 1)
		{
			value = "let through";
			m_bNoLeaderCredit = true;
		}

		RZ_StoryKit.Record(this, "barrier", RZ_WorldFacts.Town(m_iSettlement), value, m_aParticipants);
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 25, 25);
		m_Req.m_fMinMilitary = 0.2;
		m_bPresenceWeighted = true;
		m_Req.Bonus("police");
		m_Req.Bonus("soldier");
		SetRival(RZ_EFaction.BANDITS, 5);
		m_fBonusHoldSpeed = 1.25;
		m_fHoldNeeded = 600;
		m_fHoldRadius = 30;
		m_fWaveInterval = 120;
		m_iMaxWaves = 4;
		m_fTimeLeft = 2400;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1.5 * gm.GetDirector().GetStageSettings().m_fMilitaryPresence + 0.3;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		// Sealing off a town nobody lives in any more
		m_iSettlement = PickEmptySettlement(gm, 55, 95, this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.9, s.m_fRadius + 80);
		m_sTitle = "Checkpoint Duty";
		m_sDeed = "stood a shift at the checkpoint near {town}";
		m_sBrief = string.Format("%1 is infested, and we're sealing it off before it spreads down the roads: barriers across every way in and out. Hold the checkpoint at the edge of town for 10 minutes while they go up. Survivors come through and get searched. Everything else gets stopped.", s.m_sName);
		m_sObjective = string.Format("Hold the checkpoint at the edge of %1 for 10 minutes.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Quarantine Line's result: the town is sealed off and stops spreading for 30 minutes.
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return;

		if (m_iWhere == 1)
		{
			// Let through: two reach a house; the seal is leakier for it
			s.m_fNoSpreadSeconds = 900;
			if (gm.GetSafehouses())
				gm.GetSafehouses().AddRescued(m_vPos, 2, m_aParticipants);

			gm.NotifyAll("Quarantine", s.m_sName + " has been sealed off, mostly. Some people came through first.");
			return;
		}

		s.m_fNoSpreadSeconds = 1800;
		gm.NotifyAll("Quarantine", s.m_sName + " has been sealed off. It won't spread the infection for a while.");
	}
}

//------------------------------------------------------------------------------------------------
//! #6 Missing Patrol: find a squad's last position and deal with what's left of them. With a military
//! pack within 3 km, they run with it now: its young, in its look.
class RZ_Mission_MissingPatrol : RZ_Mission_KillTagged
{
	protected ResourceName m_sTurned;	// a military pack's young ("": the usual infected)
	protected string m_sTruth;			// turned (as they said), ambushed (a crew took their rifles), orders (sent to the line)
	protected string m_sCall;			// "Patrol Two-Six"
	protected string m_sCrew;			// ambushed: the crew's holding
	protected ref array<int> m_aRead = {};

	protected static const int OPT_NAIL = 0;
	protected static const int OPT_BURY = 1;
	protected static const int OPT_THIRD = 2;	// ambushed: tell the post; orders: the open channel
	protected static const int OPT_SELL = 3;	// ambushed: a quiet word to the crew

	//------------------------------------------------------------------------------------------------
	void RZ_Mission_MissingPatrol()
	{
		m_Story = new RZ_MissionStory("patrol");
		m_Choice = new RZ_OneChoice();
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 20, 20);
		m_fTimeLeft = 2400;
		m_fParticipationRadius = 150;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1.2;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickEmptySettlement(gm, 20, 100, this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius + 200, s.m_fRadius + 600);
		array<string> calls = {"Two-Six", "Four-One", "Three-Three", "One-Nine", "Five-Two"};
		m_sCall = "Patrol " + calls[RZ_Util.PickIndex(RZ_WorldFacts.Get().GetSeed() + m_iSettlement * 7, calls.Count())];
		Truth(gm, s);
		m_sTitle = "Missing Patrol";
		m_sDeed = "found the patrol near {town}";
		m_sBrief = string.Format("One of our patrols went silent outside %1. The names are still on the roster, and the roster stands: we look for our own, one way or the other. Find their last position and put down whatever they've become. If they've turned, it's the last thing anyone can do for them.", s.m_sName);
		m_sObjective = string.Format("Find the missing patrol's last position outside %1.", s.m_sName);

		// A military pack near: they're its new contacts now (docs/INFECTED.md, People)
		RZ_Tribes tribes = RZ_TribeContract.Tribes(gm);
		if (tribes)
		{
			RZ_Tribe pack = tribes.GetPeople().NearMilitary(m_vPos, 3000);
			if (pack)
			{
				m_sTurned = tribes.GetPeople().YoungPrefab(pack);
				if (!m_sTurned.IsEmpty())
					m_sBrief += string.Format(" Last contact puts them with %1.", tribes.GetPeople().NameAt(pack, RZ_EFaction.MILITARY));
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The truth (story key `patrol`, kept per patrol for 3 season days): they turned, as the post says;
	//! a crew took their rifles (a Red Hand holding within 5 km); or they were sent to the line. Then
	//! the question the post's runner asks when the work's done.
	protected void Truth(RZ_GameModeComponent gm, RZ_Settlement s)
	{
		array<string> values = {"turned", "ambushed", "orders"};
		array<float> weights = {0.45, 0, 0.25};
		RZ_Holding crew = RZ_ErrandCard.HoldingNear(gm, RZ_EFaction.BANDITS, m_vPos, 5000);
		if (crew)
		{
			weights[1] = 0.3;
			m_sCrew = crew.m_sName;
		}

		m_sTruth = m_Story.Truth(m_sCall, values, weights, this);
		if (m_sTruth == "ambushed" && !crew)
			m_sTruth = "turned";

		string prompt = string.Format("A private from the post has come out on a bike. He looks at what's left of %1, not at you. \"Tags to the nail? Or leave them with their lads?\"", m_sCall);
		string labels = "Their tags to the nail|Bury them here, tags and all";
		if (m_sTruth == "ambushed")
		{
			prompt = string.Format("A private from the post has come out on a bike. Their rifles are gone, every one. A toll chit in the sergeant's pocket, from %1. \"That's a red hand on it,\" he says, and waits.", m_sCrew);
			labels += "|Tell the post a crew did it|Sell the crew a quiet word";
		}
		else if (m_sTruth == "orders")
		{
			prompt = string.Format("A private from the post has come out on a bike. The sergeant's log is open on his knee. Last order: proceed to the line, grid %1, weather permitting. Signed by the CO. \"Weather,\" the private says.", RZ_SettlementScanner.GridReference(m_vPos));
			labels += "|Read their last order on the open channel";
		}

		m_Choice.Set(prompt, labels, "decide what's done with the patrol", OPT_NAIL);
		m_Choice.Asset(OPT_NAIL, RZ_EFaction.MILITARY);
		m_Choice.Asset(OPT_BURY, RZ_CureFinale.INDEP);
		if (m_sTruth == "ambushed")
			m_Choice.Asset(OPT_SELL, RZ_EFaction.BANDITS);
		else if (m_sTruth == "orders")
			m_Choice.Asset(OPT_THIRD, RZ_CureFinale.INDEP);

		RZ_Log.Info(string.Format("Story: patrol %1 is %2", m_sCall, m_sTruth));
	}

	//------------------------------------------------------------------------------------------------
	//! Reads at the last position: a Soldier the unit codes, an Ex-Con or Raider the chit, a Hunter the
	//! ground. Each once a player; the truth is also in the runner's question, so nobody needs one.
	protected void Reads(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity p : players)
		{
			if (m_aRead.Contains(ids[i]) || RZ_Util.DistXZ(p.GetOrigin(), m_vPos) > 30)
				continue;

			m_aRead.Insert(ids[i]);
			if (m_sTruth == "orders")
				RZ_StoryKit.Read(gm, this, ids[i], "soldier", "The unit code on their kit's for the line sector, not this one. Somebody sent them the long way round, and wrote it down.");
			else if (m_sTruth == "ambushed")
				RZ_StoryKit.Read(gm, this, ids[i], "excon,raider", string.Format("Clean work: rifles, not tins. That's %1's way. They don't take what they can't sell.", m_sCrew));

			RZ_StoryKit.Read(gm, this, ids[i], "hunter", "Boot prints in, and only some of them out. The rest went on as something else.");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The runner's question, answered (the decider rule and the 5 s hold ran in RZ_OneChoice).
	override void OnChosen(RZ_GameModeComponent gm, int decider, int option)
	{
		array<int> who = {decider};
		if (option == OPT_BURY)
		{
			m_bHasGiver = false;
			RZ_StoryKit.Record(this, "fate", m_sCall, "buried", who, string.Format("Four helmets on four sticks outside %1. Nobody from the post has been out to them.", GetLocationName(gm)), "board");
			NotifyParticipants(gm, m_sTitle, "You bury them where they fell, tags and all. The post's nail list stays a name short. Nobody pays for that.");
			return;
		}

		if (option == OPT_THIRD && m_sTruth == "ambushed")
		{
			m_Choice.BlameWith(gm, this, RZ_EFaction.BANDITS, 4, "named a crew to Bastion");
			RZ_OneChoice.Morale(gm, RZ_EFaction.MILITARY, -1, "a patrol robbed by a crew");
			RZ_StoryKit.Record(this, "fate", m_sCall, "crew", who);
			NotifyParticipants(gm, m_sTitle, string.Format("The post logs it: %1, rifles taken, %2. The Red Hand hears who told them.", m_sCall, m_sCrew));
			return;
		}

		if (option == OPT_SELL && m_sTruth == "ambushed")
		{
			m_bHasGiver = false;
			m_Choice.PayWith(gm, this, RZ_EFaction.BANDITS, 10);
			gm.GetFacts().Memory().Lies().Tell(gm, decider, "patrol.crew", m_sCall, RZ_EFaction.MILITARY, "", m_vPos, RZ_ELieTest.ROAD, 5,
				string.Format("%1 didn't turn. A crew had their rifles, and someone told the crew we knew.", m_sCall));
			RZ_StoryKit.Record(this, "fate", m_sCall, "sold", who);
			NotifyParticipants(gm, m_sTitle, "The crew pays for a quiet word: the post will hear they turned, and nothing about rifles. Bastion pays nothing for what it doesn't know.");
			return;
		}

		if (option == OPT_THIRD && m_sTruth == "orders")
		{
			m_bHasGiver = false;
			m_Choice.BlameWith(gm, this, RZ_EFaction.MILITARY, 5, "read a patrol's orders on the air");
			RZ_OneChoice.Morale(gm, RZ_EFaction.MILITARY, -2, "a patrol's orders read on the air");
			RZ_StoryKit.Record(this, "fate", m_sCall, "aired", who, string.Format("Somebody chalked a grid reference on %1's bus shelter, and 'weather permitting' under it.", GetLocationName(gm)), "chalk");
			NotifyParticipants(gm, m_sTitle, string.Format("You read it on the open channel: %1, sent to the line, weather permitting. Nobody pays you. Everybody hears it.", m_sCall));
			return;
		}

		RZ_StoryKit.Record(this, "fate", m_sCall, "home", who, string.Format("%1's tags on the nail at the post, four in a row. The paint under them says weather.", m_sCall), "board");
		NotifyParticipants(gm, m_sTitle, "Their tags go on the nail at the post. The sergeant reads the names on the call.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return string.Format("%1 (%2): nobody went. Their tags stay out there.", m_sTitle, GetLocationName(gm));
	}

	//------------------------------------------------------------------------------------------------
	//! What's left of the patrol turns up when someone comes near.
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bSpawned && RZ_ContractHelp.AnyNear(players, m_vPos, 300))
		{
			m_bSpawned = true;
			if (m_sTurned.IsEmpty())
			{
				SpawnGuards(gm, m_vPos, 4, 0, 20);
			}
			else
			{
				for (int i = 0; i < 4; i++)
				{
					gm.GetSpawner().SpawnZombie(LandPointNear(m_vPos, 0, 20), false, m_iSettlement, m_iId, m_sTurned);
				}
			}

			SpawnGuards(gm, m_vPos, 1, 0, 10, true);
		}

		if (m_bSpawned)
			Reads(gm, ids, players);

		super.OnUpdate(gm, dt, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.SpawnRewardCrate(m_vPos);
	}
}

//------------------------------------------------------------------------------------------------
//! #7 Crash Site (story key `crash`): secure a downed helicopter for 60 s, then take its black box
//! to Bastion's drop or the Red Hand's. ★ Mechanic: recovers it twice as fast and reads it. The truth,
//! kept per wreck: weather (icing and a stall), fell off the inventory (the manifest in the cockpit
//! lists rifles routed to a Red Hand crossroads; the box says the engines were fine), or already
//! taken (cut straps and drag marks: two Red Hand carriers have it at their fire; kill them for it).
//! The Last Pilot's "He flew for the line" starts it at his wreck at the line (the line shot him down).
//! Inventory to the Red Hand: the manifest is read on the open channel (Bastion morale -3). Time up
//! with the box not handed in: Buy It Back, for the participants first.
class RZ_Mission_CrashSite : RZ_Mission_Choice
{
	protected IEntity m_Wreck;
	protected float m_fSecured;
	protected bool m_bGuarded;
	protected bool m_bShown;
	protected string m_sTruth;
	protected vector m_vAt;
	protected string m_sPilot;
	protected vector m_vCamp;
	protected ref RZ_EventGroup m_Carriers;
	protected IEntity m_Cockpit;
	protected IEntity m_Fire;					// Already taken: the carriers' fire, to talk at
	protected ref array<int> m_aRead = {};
	protected ref RZ_Ways m_Ways = new RZ_Ways("Crash Site");
	protected float m_fAge;
	protected float m_fFirstNear = -1;			// when someone first came within 150 m (the sneak lift's 90 s)
	protected bool m_bTeam;						// the recovery team came at minute 15
	protected bool m_bLifted;
	protected float m_fAiring;					// the open channel: seconds the carrier has stood at the radio station
	protected bool m_bAired;

	protected static const float SECURE_SECONDS = 60;
	protected static const int COCKPIT = 0;		// the cockpit's read (HandleTalk)
	protected static const int OBJ_FIRE = 1;	// the carriers' fire (HandleObjectChoice)
	protected static const float TEAM_SECONDS = 900;
	protected static const float LIFT_WINDOW = 90;
	protected static const float AIR_SECONDS = 10;
	protected static const int BRIBE_FAVOR = 25;
	protected static const int BRIBE_FOOD = 5;

	//------------------------------------------------------------------------------------------------
	void RZ_Mission_CrashSite()
	{
		m_Story = new RZ_MissionStory("crash");
	}

	//------------------------------------------------------------------------------------------------
	//! The Last Pilot: his wreck at the line, before StartMission.
	void SetWreck(vector pos, string pilot)
	{
		m_vAt = pos;
		m_sPilot = pilot;
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 25, 25);
		m_eChoiceOther = RZ_EFaction.BANDITS;
		m_iDoubleCrossInfamy = 20;
		m_Req.Bonus("mechanic");
		m_fTimeLeft = 3000;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetHeliWreckPrefab().IsEmpty() || !CanRunChoice(gm, RZ_EFaction.MILITARY, RZ_EFaction.BANDITS))
			return 0;

		return 1.2;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		if (m_vAt != vector.Zero)
		{
			m_vPos = m_vAt;
			m_iSettlement = gm.FindNearestSettlement(m_vAt);
		}
		else
		{
			m_iSettlement = PickSettlement(gm, 0, 100);
			RZ_Settlement s = gm.GetSettlement(m_iSettlement);
			if (!s)
				return false;

			m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius + 150, s.m_fRadius + 600);
		}

		// Each drop where its faction really is, when it holds a town in reach
		int bastionTown = PickHeldSettlementAway(gm, RZ_EFaction.MILITARY, m_vPos, 800, 5000, -1, this);
		if (bastionTown < 0)
			bastionTown = PickSettlementAway(gm, m_vPos, 800, 5000, 0, 60, -1, this);

		int redHandTown = PickHeldSettlementAway(gm, RZ_EFaction.BANDITS, m_vPos, 800, 5000, bastionTown, this);
		if (redHandTown < 0)
			redHandTown = PickSettlementAway(gm, m_vPos, 800, 5000, 0, 100, bastionTown, this);
		RZ_Settlement b = gm.GetSettlement(bastionTown);
		RZ_Settlement r = gm.GetSettlement(redHandTown);
		if (!b || !r)
			return false;

		// The truth, kept per wreck; his wreck at the line is the line's doing
		string wreckKey = "wreck:" + RZ_SettlementScanner.GridReference(m_vPos);
		if (!m_sPilot.IsEmpty())
		{
			m_sTruth = "line";
		}
		else
		{
			array<string> values = {"weather", "inventory", "taken"};
			array<float> weights = {0.45, 0.35, 0};
			if (!gm.GetRedHandGroupPrefab().IsEmpty())
				weights[2] = 0.2;

			m_sTruth = m_Story.Truth(wreckKey, values, weights, this);
		}

		m_vCamp = LandPointNear(m_vPos, 300, 600);
		if (m_sTruth == "taken")
			m_Carriers = new RZ_EventGroup(gm.GetRedHandGroupPrefab(), m_vCamp);

		SpawnDrop(gm, RZ_EFaction.MILITARY, LandPointNear(b.m_vCenter, b.m_fRadius * 0.8, b.m_fRadius + 50));
		SpawnDrop(gm, RZ_EFaction.BANDITS, LandPointNear(r.m_vCenter, r.m_fRadius * 0.8, r.m_fRadius + 50));

		m_fParticipationRadius = 0;
		m_sTitle = "Crash Site";
		m_sDeed = "went to the crash near {town}";
		m_sObjective = string.Format("Recover the helicopter's black box from the wreck at grid %1.", RZ_SettlementScanner.GridReference(m_vPos));
		if (!m_sPilot.IsEmpty())
			m_sBrief = string.Format("Bastion's last helicopter came down short of the line at grid %1. %2 was flying it. The box light's still blinking. Bastion's drop is at %3. The Red Hand would pay for it at %4.", RZ_SettlementScanner.GridReference(m_vPos), m_sPilot, b.m_sName, r.m_sName);
		else
			m_sBrief = string.Format("The helicopter flew over it yesterday: a Bastion wreck near %1, and the box light's still blinking. Secure it and recover the black box. Bastion's drop is at %2. The Red Hand would pay for it at %3.", GetLocationName(gm), b.m_sName, r.m_sName);

		RZ_Log.Info(string.Format("Story: crash at %1 is %2", RZ_SettlementScanner.GridReference(m_vPos), m_sTruth));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected string CockpitText()
	{
		if (m_sTruth == "inventory")
			return "A manifest clipped to the seat: twelve rifles signed out by an outpost on Day 9, routed via a Red Hand crossroads, \"for diesel\". The flight plan ends there, not at the post.";

		if (m_sTruth == "taken")
			return "The box's bay is open and the straps are cut, not torn. Drag marks in the mud, two sets of boots, heading east.";

		if (m_sTruth == "line")
			return string.Format("Holes along the tail, all from the same side. The line. %1's seat is empty, the harness cut.", m_sPilot);

		return "Frost still on the inside of the canopy. A grease-pencil note on the knee board: \"Icing. Again.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! What a Mechanic hears on the box.
	protected string BoxRead()
	{
		if (m_sTruth == "inventory")
			return "The box says the engines were fine to the last second. He wasn't falling. He was landing, somewhere he shouldn't.";

		if (m_sTruth == "line")
			return "The box has the line's warning on the radio, twice, and the pilot answering it calmly. Then the rounds.";

		return "Ice warnings, the pilot calm, then a stall he almost caught. Then nothing.";
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDeliveryTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		// The wreck, the cockpit and the infected around it turn up when someone comes near
		if (!m_bShown && RZ_ContractHelp.AnyNear(players, m_vPos, 400))
		{
			m_bShown = true;
			m_Wreck = RZ_Util.SpawnPrefab(gm.GetHeliWreckPrefab(), RZ_Util.GroundPos(m_vPos), Math.RandomFloat(0, 360));
			if (HasPuzzleObject(gm))
				SpawnCockpit(gm);

			gm.GetHeat().AddHeat(m_vPos, 40);
		}

		if (!m_bGuarded && RZ_ContractHelp.AnyNear(players, m_vPos, 300))
		{
			m_bGuarded = true;
			SpawnGuards(gm, m_vPos, 3, 15, 40);
		}

		if (m_sTruth == "taken")
		{
			UpdateCarriers(gm, dt, players);
			return;
		}

		m_fAge += dt;
		if (!m_bItemSpawned)
		{
			Sneak(gm, ids, players);
			RecoveryTeam(gm, ids, players);
			if (m_eState != RZ_EMissionState.ACTIVE)
				return;
		}

		if (m_bItemSpawned)
		{
			OpenChannel(gm, dt);
			return;
		}

		if (CountPlayersNear(players, m_vPos, 25) == 0 && m_fSecured < SECURE_SECONDS)
			return;

		float rate = 1;
		if (AnyBonusNear(gm, ids, players, m_vPos, 25))
			rate = 2;

		m_fSecured += dt * rate;
		if (m_fSecured < SECURE_SECONDS)
			return;

		if (!SpawnItem(gm, RZ_Util.RandomPointInRing(m_vPos, 2, 4)))
			return;

		array<int> near = {};
		GetPlayersNear(ids, players, m_vPos, 60, near);
		foreach (int playerId : near)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You pull the black box out of the wreck. Now decide who gets it.");
			RZ_StoryKit.Read(gm, this, playerId, "mechanic", BoxRead());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The sneak lift: in the 90 s after someone first comes within 150 m, a player hidden within 8 m
	//! of the wreck lifts the box before anything notices.
	protected void Sneak(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players)
	{
		if (m_fFirstNear < 0 && RZ_ContractHelp.AnyNear(players, m_vPos, 150))
			m_fFirstNear = m_fAge;

		if (m_bLifted || m_fFirstNear < 0 || m_fAge - m_fFirstNear > LIFT_WINDOW)
			return;

		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), m_vPos) > 8 || !RZ_Stealth.IsHidden(ids[i]))
				continue;

			m_bLifted = true;
			m_fSecured = SECURE_SECONDS;
			m_Story.m_Deciding.Step(gm, ids[i]);
			gm.NotifyPlayer(ids[i], m_sTitle, "You work the box out of its bay with your knife, slowly, and nothing turns its head. It's yours to carry.");
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Minute 15: Bastion's recovery truck stops at the road and shouts for the box. Trusted with
	//! Bastion: they leave it to you. Only Known: they take it, and the job's paid at half. Watched or
	//! worse: they fire over your heads (+5 Bastion Infamy). The team is a real squad (Contracts >
	//! Bastion squad, else the hunter squad or a truck crew) as a story group, walking from the road
	//! to the wreck; with no prefab or no room it's words only, as before.
	protected void RecoveryTeam(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players)
	{
		if (m_bTeam || m_fAge < TEAM_SECONDS || m_sTruth == "line")
			return;

		array<int> near = {};
		GetPlayersNear(ids, players, m_vPos, 150, near);
		if (near.IsEmpty())
			return;

		m_bTeam = true;
		vector road = LandPointNear(m_vPos, 70, 110);
		ResourceName team = gm.PickLook(RZ_Mission_BustOut.SquadPrefab(gm), RZ_Looks.SeedAt(road));
		if (RZ_StoryGroups.Spawn(gm, this, team, road, LandPointNear(m_vPos, 8, 15), "recovery team"))
			RZ_ContractHelp.NotifyNear(gm, m_vPos, 150, m_sTitle, string.Format("A truck pulls up on the road, %1, and a section climbs down and walks toward the wreck.", RZ_Util.DirectionText(m_vPos, road)));

		bool trusted = false;
		bool known = false;
		foreach (int playerId : near)
		{
			RZ_ETier tier = gm.GetTier(playerId, RZ_EFaction.MILITARY);
			if (RZ_Reputation.IsAtLeast(tier, RZ_ETier.TRUSTED))
				trusted = true;
			else if (RZ_Reputation.IsAtLeast(tier, RZ_ETier.KNOWN))
				known = true;
			else if (tier == RZ_ETier.WATCHED || RZ_Reputation.IsHunted(tier))
			{
				gm.AddInfamy(playerId, RZ_EFaction.MILITARY, 5, "at a crash site Bastion's team came for");
				gm.NotifyPlayer(playerId, m_sTitle, "A truck stops on the road and somebody fires over your head. \"Away from the wreck. Now.\"");
			}
		}

		if (trusted)
		{
			m_fSecured = SECURE_SECONDS;
			RZ_ContractHelp.NotifyNear(gm, m_vPos, 150, m_sTitle, "Bastion's recovery truck stops on the road. A sergeant waves: \"Carry on. It's yours to bring in.\" They keep the infected off you while you pull it.");
			return;
		}

		if (!known)
			return;

		// They take the box themselves: done for Bastion, at half pay
		m_fRewardScale = 0.5;
		m_bDelivered = true;
		m_iDeliveredTo = RZ_EFaction.MILITARY;
		foreach (int who : near)
		{
			if (!m_aParticipants.Contains(who))
				m_aParticipants.Insert(who);
		}

		RZ_StoryKit.Record(this, "box", "wreck:" + RZ_SettlementScanner.GridReference(m_vPos), "team:" + m_sTruth, near);
		RZ_ContractHelp.NotifyNear(gm, m_vPos, 150, m_sTitle, "Bastion's recovery team takes the box off the wreck themselves. \"Thanks for keeping it warm.\" You'll get half.");
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	//! The open channel (Inventory): the box's carrier stands at the radio station for 10 s and reads
	//! the manifest out. Nobody pays; Bastion morale -3, +10 Bastion Infamy, the commander remembers.
	protected void OpenChannel(RZ_GameModeComponent gm, float dt)
	{
		if (m_bAired || m_bDelivered || m_sTruth != "inventory" || m_iCarrier <= 0 || !gm.GetRadioStation())
			return;

		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iCarrier);
		if (!me || RZ_Util.DistXZ(me.GetOrigin(), gm.GetRadioStation().GetPosition()) > 20)
		{
			m_fAiring = 0;
			return;
		}

		if (!RZ_RouteAssets.Refuse(gm, m_iCarrier, RZ_CureFinale.INDEP).IsEmpty())
		{
			gm.NotifyPlayer(m_iCarrier, m_sTitle, RZ_RouteAssets.Refuse(gm, m_iCarrier, RZ_CureFinale.INDEP));
			m_fAiring = -600;
			return;
		}

		m_fAiring += dt;
		if (m_fAiring < AIR_SECONDS)
			return;

		m_bAired = true;
		m_bDelivered = true;
		m_iFameReward = 0;
		m_iFavorReward = 0;
		m_bNoLeaderCredit = true;
		RZ_OneChoice.Morale(gm, RZ_EFaction.MILITARY, -3, "the manifest read on the open channel");
		gm.AddInfamy(m_iCarrier, RZ_EFaction.MILITARY, 10, "read Bastion's manifest on the open channel");
		RZ_StoryKit.Remember(gm, RZ_EFaction.MILITARY, m_vPos, 5000, m_iCarrier, RZ_Leaders.MEM_CROSSED);
		gm.NotifyAllAs("Open channel", "A voice nobody knows, reading slowly from a clipboard: twelve rifles, signed out on Day 9, routed via a Red Hand crossroads, for diesel. Then the date, and the signature.", RZ_ENoticeTag.EVENT);
		array<int> who = {m_iCarrier};
		RZ_Deeds.AddLifeRecord(gm.GetProfile(m_iCarrier), "aired", true);
		RZ_StoryKit.Record(this, "box", "wreck:" + RZ_SettlementScanner.GridReference(m_vPos), "aired:" + m_sTruth, who);
		m_aParticipants.Clear();
		m_aParticipants.Insert(m_iCarrier);
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	//! After the lock: a player locked to another route can't hand the box to this side.
	override bool OnItemDelivered(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction, IEntity drop)
	{
		if (!m_bDelivered && m_sTruth == "inventory")
		{
			string refusal = RZ_RouteAssets.Refuse(gm, playerId, faction);
			if (!refusal.IsEmpty())
			{
				gm.NotifyPlayer(playerId, m_sTitle, refusal);
				return false;
			}
		}

		return super.OnItemDelivered(gm, playerId, faction, drop);
	}

	//------------------------------------------------------------------------------------------------
	//! Already taken: two carriers at their fire; the box is there once they're dead.
	protected void UpdateCarriers(RZ_GameModeComponent gm, float dt, array<IEntity> players)
	{
		if (!m_Carriers || m_bItemSpawned)
			return;

		m_Carriers.Tick(dt);
		if (!m_Carriers.m_bSpawned && RZ_ContractHelp.AnyNear(players, m_vCamp, 400))
		{
			if (m_Carriers.Spawn())
				RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, m_Carriers.m_Unit, 200, true);

			// Their fire: talk, pay, or a claim in Bastion kit with the flight order
			ResourceName fire = gm.GetPuzzlePrefab(RZ_EPuzzleProp.OBJECT);
			if (!fire.IsEmpty())
			{
				m_Fire = RZ_Util.SpawnPrefab(fire, RZ_Util.GroundPos(RZ_Util.RandomPointInRing(m_vCamp, 2, 4)));
				RZ_PuzzleObjectComponent.Tag(m_Fire, m_iId, OBJ_FIRE, "", "[Known · Red Hand] \"Bastion pays twice what the fence does\"|Pay them [25 Red Hand Favor | 5 food]|[Bastion kit] \"That's ours, lads\"");
			}
		}

		if (m_Carriers.IsDead() && SpawnItem(gm, RZ_Util.RandomPointInRing(m_vCamp, 1, 3)))
			RZ_ContractHelp.NotifyNear(gm, m_vCamp, 150, m_sTitle, "The box is by their fire, still in its cut straps. Now decide who gets it.");
	}

	//------------------------------------------------------------------------------------------------
	//! The carriers' fire: the ways other than a fight. Each that works sends them off without the box.
	override void HandleObjectChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option)
	{
		super.HandleObjectChoice(gm, playerId, objIdx, option);
		if (objIdx != OBJ_FIRE || m_bItemSpawned || !m_Carriers || m_Carriers.IsDead())
			return;

		m_Story.m_Deciding.Step(gm, playerId);
		bool gone = false;
		if (option == 0)
		{
			if (!RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.KNOWN))
			{
				gm.NotifyPlayer(playerId, m_sTitle, "\"Who are you, then?\" Nobody here knows your face.");
				return;
			}

			gone = m_Ways.TryTrust(gm, playerId, "carriers", RZ_EFaction.BANDITS, 1, "") == RZ_ECheck.PASS;
			if (!gone)
				gm.NotifyPlayer(playerId, m_sTitle, "The fence pays now. Bastion pays one day. Walk on.");
		}
		else if (option == 1)
		{
			IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (me && RZ_HavenQuest.CountFood(gm, me) >= BRIBE_FOOD && RZ_StoryStats.TakeFood(gm, me, BRIBE_FOOD))
				gone = true;
			else
				gone = m_Ways.TryBribe(gm, playerId, RZ_EFaction.BANDITS, BRIBE_FAVOR);
		}
		else if (option == 2)
		{
			gone = Trick(gm, playerId);
		}

		if (!gone)
			return;

		m_Carriers.Delete();
		RZ_Util.DeleteUnit(m_Fire);
		m_Fire = null;
		if (SpawnItem(gm, RZ_Util.RandomPointInRing(m_vCamp, 1, 3)))
			RZ_ContractHelp.NotifyNear(gm, m_vCamp, 150, m_sTitle, "They kick dirt over the fire and walk off east without it. The box sits by the ashes in its cut straps. Now decide who gets it.");
	}

	//------------------------------------------------------------------------------------------------
	//! "That's ours, lads": a claim judged by the look, with the crew's flight order from the cockpit
	//! as the cover detail (read it first). Caught: the catch, and the carriers walk off to the fence.
	protected bool Trick(RZ_GameModeComponent gm, int playerId)
	{
		RZ_StoryKit.Dirty(gm, this, playerId);
		RZ_Disguise disguise = null;
		if (gm.GetHostility())
			disguise = gm.GetHostility().GetDisguise();

		if (!disguise || !disguise.IsOn())
		{
			gm.NotifyPlayer(playerId, m_sTitle, "They look you up and down and laugh.");
			return false;
		}

		string cannot = disguise.CannotClaim(playerId, RZ_EFaction.MILITARY);
		if (!cannot.IsEmpty())
		{
			gm.NotifyPlayer(playerId, m_sTitle, cannot);
			return false;
		}

		string paper = "";
		if (m_aRead.Contains(playerId))
			paper = "The flight order";

		if (disguise.Look(playerId, RZ_EFaction.MILITARY, 1, "crash:" + RZ_SettlementScanner.GridReference(m_vCamp), paper, 300))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You read them the tail number off the flight order. They look at each other, and decide it isn't worth it.");
			return true;
		}

		// Caught: they run for the fence with it
		m_Carriers.Delete();
		RZ_Util.DeleteUnit(m_Fire);
		m_Fire = null;
		m_eState = RZ_EMissionState.FAILED;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasPuzzleObject(RZ_GameModeComponent gm)
	{
		return !gm.GetPuzzlePrefab(RZ_EPuzzleProp.CLUE).IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnCockpit(RZ_GameModeComponent gm)
	{
		m_Cockpit = RZ_Util.SpawnPrefab(gm.GetPuzzlePrefab(RZ_EPuzzleProp.CLUE), RZ_Util.GroundPos(RZ_Util.RandomPointInRing(m_vPos, 2, 3)) + Vector(0, 0.3, 0));
		if (m_Cockpit)
			RZ_ClueComponent.Tag(m_Cockpit, m_iId, COCKPIT, "Look in the cockpit");
	}

	//------------------------------------------------------------------------------------------------
	//! The cockpit read (an ordinary mission's tagged clue): what's there, and the truth for the room.
	override bool HandleTalk(RZ_GameModeComponent gm, int playerId, int clueIdx)
	{
		if (clueIdx != COCKPIT)
			return false;

		gm.NotifyPlayer(playerId, "The cockpit", CockpitText());
		if (!m_aRead.Contains(playerId))
		{
			m_aRead.Insert(playerId);
			m_Story.m_Deciding.Step(gm, playerId);
			m_Story.Reveal(gm, this, "wreck:" + RZ_SettlementScanner.GridReference(m_vPos), playerId, "cockpit", TruthLine());
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected string TruthLine()
	{
		if (m_sTruth == "inventory")
			return "The rifles on that manifest never reached the post. They fell off the inventory, and the helicopter went with them.";

		if (m_sTruth == "taken")
			return string.Format("Somebody got here first. The box went east, toward grid %1. Bastion's brief didn't say.", RZ_SettlementScanner.GridReference(m_vCamp));

		if (m_sTruth == "line")
			return "The line shot it down. Everyone at the post knows it, and nobody's going to write it.";

		return "Weather. Just the weather. It still killed them.";
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoiceMade(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction)
	{
		string went = "bastion";
		if (faction == RZ_EFaction.BANDITS)
			went = "redhand";

		string epilogue = "";
		// The majors' flags: Bastion's pay for the box under the muster's stance (Expended: command has
		// gone quiet, x0.75), and the stove's silence is worth less once a stronghold's page is out (x0.8)
		if (faction == RZ_EFaction.MILITARY && RZ_MajorHelp.World(gm, "orders.stance") == 4)
			m_fRewardScale *= 0.75;

		if (faction == RZ_EFaction.MILITARY && m_sTruth == "inventory")
		{
			gm.NotifyPlayer(playerId, m_sTitle, "The commander thanks you, takes the box into his office, and you hear the stove door open. \"Weather,\" he says when he comes out. \"Write it down.\"");
			went = "stove";
			if (RZ_MajorHelp.World(gm, "majors.pages_open") >= 1)
				m_fRewardScale *= 0.8;

			// The commander, quietly: his loyalty up, Bastion's standing for the one who brought it, and
			// a silence that comes out if the manifest ever turns up another way
			RZ_Leaders leaders = gm.GetLeaders();
			if (leaders)
				RZ_StoryKit.Loyalty(gm, leaders.FindNearest(RZ_EFaction.MILITARY, m_vPos, 5000), 5);

			RZ_WorldFacts.Get().Memory().Lies().Tell(gm, playerId, "crash.stove", "wreck:" + RZ_SettlementScanner.GridReference(m_vPos), RZ_EFaction.MILITARY, "", m_vPos, RZ_ELieTest.RUMOUR, 10,
				"A copy of a manifest is going round the posts: twelve rifles, Day 9, for diesel. Somebody remembers who carried a box into the commander's office.");
		}
		else if (faction == RZ_EFaction.BANDITS && m_sTruth == "inventory")
		{
			RZ_Errands errands = gm.GetErrands();
			if (errands)
				errands.Morale(RZ_EFaction.MILITARY, -3, "the manifest on the open channel");

			gm.NotifyAllAs("Open channel", "A Red Hand voice, reading slowly: \"Twelve rifles, signed correct, every one. Turns out they were signed to us.\" Laughter, then static.", RZ_ENoticeTag.EVENT);
			RZ_Leaders crossed = gm.GetLeaders();
			if (crossed)
				RZ_StoryKit.Loyalty(gm, crossed.FindNearest(RZ_EFaction.MILITARY, m_vPos, 5000), -5);
		}
		else if (faction == RZ_EFaction.MILITARY)
		{
			epilogue = string.Format("Behind the radio at the post nearest %1, more tags on the nail. Under them, in grease pencil, the weather.", GetLocationName(gm));
			// The nail (Weather): the crew's tags go up, morale +2, the names on tonight's call
			if (m_sTruth == "weather")
			{
				went = "nail";
				RZ_OneChoice.Morale(gm, RZ_EFaction.MILITARY, 2, "the crew's tags on the nail");
				RZ_Shared shared = gm.GetShared();
				if (shared)
					shared.GetCallbacks().Queue(playerId, "Bastion's nightly call", "The operator reads three names off the nail, slowly, and adds: \"Recovered by a civilian. Weather.\"", m_vPos);
			}
		}

		array<int> who = {playerId};
		RZ_StoryKit.Record(this, "box", "wreck:" + RZ_SettlementScanner.GridReference(m_vPos), went + ":" + m_sTruth, who, epilogue, "voice");
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailed(RZ_GameModeComponent gm)
	{
		super.OnFailed(gm);
		if (!m_bDelivered)
			RZ_StoryKit.Fallout(gm, this, "buy_back", m_vPos, "the black box from the wreck");

		// Time up with the box still out there: whoever comes next finds it already taken
		if (!m_bDelivered && !m_bWithdrawn && m_sTruth != "line" && m_Story && !gm.GetRedHandGroupPrefab().IsEmpty())
		{
			RZ_Truth kept = RZ_WorldFacts.Get().Memory().FindTruth(m_Story.m_sKey, "wreck:" + RZ_SettlementScanner.GridReference(m_vPos));
			if (kept)
				kept.value = "taken";
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		RZ_Util.DeleteUnit(m_Wreck);
		RZ_Util.DeleteUnit(m_Cockpit);
		RZ_Util.DeleteUnit(m_Fire);
		if (m_Carriers)
			m_Carriers.Delete();
	}
}

//------------------------------------------------------------------------------------------------
//! #15 Raid the Red Hand: assault a Red Hand camp. Needs the Red Hand group prefab.
//! Not for Ex-Cons or Raiders unless they're Champions of Bastion.
//! A story job (key `raid`, kept per town): a camp, as they said; people held there (a house within 5
//! km); or Bastion's own deserters in red paint (a Bastion holding on the map). The lookout on its
//! edge (the talk scene): [Threaten] tell them to clear out (Fear 2: the camp is gone without a shot),
//! or three tins and he leaves the gate (one group fewer). Done, the question: the camp burned
//! (default); held: walk them to the house (10 Haven Favor, the house +2) or Bastion's truck
//! (sheltered); deserters: their tags to the post (Bastion morale -1) or burn the paint and say
//! nothing (nobody pays, an Independent asset); a camp: leave its stores to the scavengers (no crate).
class RZ_Mission_RaidRedHand : RZ_Mission
{
	protected ref array<ref RZ_UnitWatch> m_aGroups = {};
	protected bool m_bSpawned;
	protected bool m_bFewer;
	protected bool m_bGone;
	protected bool m_bCrate = true;
	protected int m_iHouse = -1;

	protected static const int GROUPS = 2;
	protected static const int WAY_GO = 0;
	protected static const int WAY_GATE = 1;
	protected static const int OPT_BURN = 0;
	protected static const int OPT_OTHER = 1;

	//------------------------------------------------------------------------------------------------
	void RZ_Mission_RaidRedHand()
	{
		m_Story = new RZ_MissionStory("raid");
		m_Choice = new RZ_OneChoice();
		m_Job = new RZ_StoryJob();
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.TRUSTED, 35, 40);
		m_Req.Need(RZ_EFaction.BANDITS, RZ_EStandingCheck.NOT_AT_LEAST_TRUSTED);
		m_Req.Deny("excon");
		m_Req.Deny("raider");
		m_Req.m_bDeniedUnlessChampion = true;
		SetRival(RZ_EFaction.BANDITS, 30);
		m_fTimeLeft = 3000;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetRedHandGroupPrefab().IsEmpty())
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		// The camp is outside a town nobody lives in (a crew's own holdings never fall)
		m_iSettlement = PickEmptySettlement(gm, 0, 80, this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.8, s.m_fRadius + 250);
		m_fParticipationRadius = 250;
		m_sTitle = "Raid the Red Hand";
		m_sBrief = string.Format("A Red Hand crew has set up camp outside %1 and is tolling the road there: tins, fuel, sometimes people. Since the hill road on Day 8, we don't leave a crew camped near our roads. Take the camp. Their lookout sits on the edge of it, if you'd rather talk first.", s.m_sName);
		m_sObjective = string.Format("Take the Red Hand camp outside %1.", s.m_sName);
		Truth(gm);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Truth(RZ_GameModeComponent gm)
	{
		array<string> values = {"camp", "held", "deserters"};
		array<float> weights = {0.5, 0, 0};
		if (gm.GetSafehouses())
			m_iHouse = gm.GetSafehouses().FindNearest(m_vPos, 5000);

		if (m_iHouse >= 0)
			weights[1] = 0.3;

		if (RZ_ErrandCard.HoldingNear(gm, RZ_EFaction.MILITARY, m_vPos, 100000))
			weights[2] = 0.2;

		string truth = m_Job.Draw(this, RZ_WorldFacts.Town(m_iSettlement), values, weights);
		m_Job.m_vReadAt = m_vPos;
		m_Job.m_fReadRange = 60;
		m_Job.Read("deserters", "soldier", "Bastion boots under the red paint, laced the Bastion way. These were ours.");
		m_Job.Read("held", "elder", "A child's shoe by the fire, and a name scratched on a post. Somebody's people are held here.");
		m_Job.Read("held", "hunter", "Two sets of tracks in from the road with short steps. Feet tied together.");
		m_Job.Read("camp", "excon,raider", "Lookouts on a rota and a latrine dug properly. This crew's been here a while and got comfortable.");

		string reveal = "Lads. Rifles. A pot. Mostly the pot.";
		if (truth == "held")
			reveal = "Some people from a house. They're for trading. Don't look at me like that, I only look.";
		else if (truth == "deserters")
			reveal = "Half of them were Bastion last month. They still salute when they're drunk.";

		RZ_TalkScene sc = m_Job.Scene("The lookout", "raid.lookout", LandPointNear(m_vPos, 60, 90));
		int n0 = sc.Node("A lad with a rifle too big for him. \"I'm the lookout. I looked. I'd like to stop looking now.\"");
		int n1 = sc.Node("They'll kill me if you get in. You'll kill me if you don't. It's not a great job.");
		sc.Ask(n0, "Who's in there?", n1, reveal, true);
		sc.Way(n1, "Tell them to clear out", RZ_TalkScene.FEAR, RZ_EFaction.BANDITS, 2, WAY_GO, "He runs. A minute later the camp is packing up, fast and badly.", "He laughs, and shouts for the others.");
		sc.Way(n1, "Three tins and you never saw me", RZ_TalkScene.FOOD, -1, 3, WAY_GATE, "He takes the tins and walks off into the trees. One fire fewer in the camp tonight.");

		string labels = "Burn the camp";
		if (truth == "held")
		{
			labels = "Bastion's truck takes them|Walk them to the house";
			m_Choice.Asset(OPT_OTHER, RZ_EFaction.SURVIVORS);
		}
		else if (truth == "deserters")
		{
			labels = "Their tags to the post|Burn the paint and say nothing";
			m_Choice.Asset(OPT_OTHER, RZ_CureFinale.INDEP);
		}
		else
		{
			labels += "|Leave the stores to scavengers";
		}

		m_Choice.Set("The camp's quiet now. Bastion's corporal walks up the track with a clipboard. \"Well?\"", labels, "decide what's done with the camp", OPT_BURN);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStoryWay(RZ_GameModeComponent gm, int playerId, int way)
	{
		if (way == WAY_GATE)
		{
			m_bFewer = true;
			if (m_aGroups.Count() > 1)
			{
				m_aGroups[0].Delete();
				m_aGroups.Remove(0);
			}

			return;
		}

		m_bGone = true;
		foreach (RZ_UnitWatch watch : m_aGroups)
		{
			watch.Delete();
		}

		m_aGroups.Clear();
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnChosen(RZ_GameModeComponent gm, int decider, int option)
	{
		array<int> who = {decider};
		string town = RZ_WorldFacts.Town(m_iSettlement);
		string truth = m_Job.m_sTruth;
		if (truth == "held" && option == OPT_OTHER)
		{
			m_Choice.PayWith(gm, this, RZ_EFaction.SURVIVORS, 10);
			if (m_iHouse >= 0)
				gm.GetSafehouses().AddPopulationAt(m_iHouse, 2);

			RZ_StoryKit.Record(this, "fate", town, "walked", who);
			RZ_StoryKit.LaterWith(gm, this, "At the house", "A man with rope marks on his wrists is splitting wood by the door. He stops, and nods, and goes back to it.", m_vPos, true);
			return;
		}

		if (truth == "held")
		{
			RZ_StoryKit.Sheltered(gm, this, 2);
			RZ_StoryKit.Record(this, "fate", town, "truck", who);
			return;
		}

		if (truth == "deserters" && option == OPT_OTHER)
		{
			m_bHasGiver = false;
			RZ_StoryKit.Record(this, "fate", town, "burned", who, "A burned patch outside the town where the red paint ran into the grass. Nobody from the post asked whose boots.", "board");
			NotifyParticipants(gm, m_sTitle, "The paint goes in the fire with the jackets. The corporal writes 'Red Hand camp, cleared' and nothing else. Bastion pays for none of it.");
			return;
		}

		if (truth == "deserters")
		{
			RZ_OneChoice.Morale(gm, RZ_EFaction.MILITARY, -1, "deserters in red paint");
			RZ_StoryKit.Record(this, "fate", town, "tags", who);
			return;
		}

		if (option == OPT_OTHER)
		{
			m_bCrate = false;
			RZ_StoryKit.Record(this, "fate", town, "left", who);
			return;
		}

		RZ_StoryKit.Record(this, "fate", town, "burned", who);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		// The camp is manned when someone comes near
		if (!m_bSpawned)
		{
			if (!RZ_ContractHelp.AnyNear(players, m_vPos, 300))
				return;

			m_bSpawned = true;
			int groups = GROUPS;
			if (m_bFewer)
				groups = GROUPS - 1;

			if (m_bGone)
				groups = 0;

			for (int i = 0; i < groups; i++)
			{
				RZ_UnitWatch g = new RZ_UnitWatch();
				if (!g.Spawn(gm.GetRedHandGroupPrefab(), LandPointNear(m_vPos, 0, 25)))
					continue;

				m_aGroups.Insert(g);
				RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, g.m_Unit); // the camp shoots back
			}

			if (m_aGroups.IsEmpty() && !m_bGone)
				m_eState = RZ_EMissionState.FAILED;

			return;
		}

		if (m_bGone)
		{
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		// Every group has to be dead (a group deletes itself when its last member dies)
		bool allDead = true;
		foreach (RZ_UnitWatch watch : m_aGroups)
		{
			if (!watch.IsDead(dt))
				allDead = false;
		}

		if (allDead)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (m_bGone)
		{
			gm.NotifyAll("Red Hand", "A Red Hand camp near " + GetLocationName(gm) + " packed up and left in a hurry. Somebody frightened it.");
			return;
		}

		if (m_bCrate)
			gm.SpawnRewardCrate(m_vPos);

		gm.NotifyAll("Red Hand", "Someone wiped out a Red Hand camp near " + GetLocationName(gm) + ". The Red Hand is out for blood.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		foreach (RZ_UnitWatch watch : m_aGroups)
		{
			watch.Delete();
		}

		m_aGroups.Clear();
	}
}

//------------------------------------------------------------------------------------------------
//! #17 Requisition: "collect" supplies from a Haven safehouse for Bastion. A nasty
//! choice: deliver them to Bastion (Haven Infamy) or give them back to the Haven
//! (Bastion Infamy). Not for Elders.
class RZ_Mission_Requisition : RZ_Mission_Choice
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.TRUSTED, 20, 35);
		m_eChoiceOther = RZ_EFaction.SURVIVORS;
		m_iDoubleCrossInfamy = 20;
		m_Req.Deny("elder");
		m_fTimeLeft = 2400;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!CanRunChoice(gm, RZ_EFaction.MILITARY, RZ_EFaction.SURVIVORS))
			return 0;

		// A refused census sends it back more often (The Census, Missions/Questlines/Bastion/)
		return 0.8 * RZ_Quest_Census.RequisitionWeight(gm);
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		// The safest town, but never a house Bastion's questlines spared, and refused houses first
		m_iSettlement = RZ_Quest_Census.RequisitionTown(gm);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s || IsTownBusy(gm, m_iSettlement))
			return false;

		int bastionTown = PickHeldSettlementAway(gm, RZ_EFaction.MILITARY, s.m_vCenter, 800, 5000, m_iSettlement, this);
		if (bastionTown < 0)
			bastionTown = PickSettlementAway(gm, s.m_vCenter, 800, 5000, 0, 100, m_iSettlement, this);
		RZ_Settlement b = gm.GetSettlement(bastionTown);
		if (!b)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.4);

		// Handing them back means carrying them to another Haven safehouse, not dropping them by the door
		vector havenPos;
		string havenName;
		int house = RZ_ContractHelp.PickSafehouse(gm, m_vPos, 800, 5000);
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, house);
		if (h && h.m_iSettlement != bastionTown)
		{
			havenPos = LandPointNear(h.m_vPos, 4, 10);
			havenName = "the safehouse in " + h.m_sName;
		}
		else
		{
			RZ_Settlement other = gm.GetSettlement(PickSettlementAway(gm, s.m_vCenter, 800, 5000, 0, 40, bastionTown));
			if (!other)
				return false;

			havenPos = LandPointNear(other.m_vCenter, 0, other.m_fRadius * 0.4);
			havenName = other.m_sName;
		}

		if (!SpawnItem(gm, m_vPos))
			return false;

		if (!SpawnDrop(gm, RZ_EFaction.SURVIVORS, havenPos))
			return false;

		if (!SpawnDrop(gm, RZ_EFaction.MILITARY, LandPointNear(b.m_vCenter, b.m_fRadius * 0.8, b.m_fRadius + 50)))
			return false;

		m_fParticipationRadius = 0;
		m_sTitle = "Requisition";
		m_sDeed = "brought in what the stores were short of";
		m_sBrief = string.Format("Every name on our roster has to eat, and the Haven's stores in %1 hold more than they're letting on. Collect a contribution and bring it to our drop at %2. It's for their protection. Their people in %3 will call it theft and ask for it back, and it's for whoever carries it to decide if they get it.", s.m_sName, b.m_sName, havenName);
		m_sObjective = string.Format("Collect a contribution from the Haven stores in %1 and bring it to the drop at %2.", s.m_sName, b.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The supplies are real: taken to Bastion, the nearest Haven safehouse loses 4 food.
	override protected void OnChoiceMade(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction)
	{
		if (faction != RZ_EFaction.MILITARY || !gm.GetSafehouses())
			return;

		int idx = gm.GetSafehouses().TakeFood(m_vPos, REQUISITION_FOOD);
		RZ_Safehouse h = gm.GetSafehouses().Get(idx);
		if (h)
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("The safehouse in %1 has %2 food left in its stores.", h.m_sName, h.m_iFood));
	}

	protected static const int REQUISITION_FOOD = 4;
}
