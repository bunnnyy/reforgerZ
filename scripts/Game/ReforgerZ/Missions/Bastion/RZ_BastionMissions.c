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
		m_iSettlement = PickSettlement(gm, 50, 100);
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
			m_sBrief = string.Format("The Haven wants to move back into %1. Kill %2 infected in the town.", s.m_sName, m_iKillsNeeded);
			return;
		}

		m_sTitle = "Retake " + s.m_sName;
		m_sBrief = string.Format("Bastion command wants %1 cleared. Kill %2 infected in the town.", s.m_sName, m_iKillsNeeded);
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
		gm.NotifyAll("Town reclaimed", string.Format("%1 has been pushed back to %2.", s.m_sName, s.GetStateName()));
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
		m_iSettlement = PickSettlement(gm, 0, 70);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		// LZ on the edge of town where a helicopter could land
		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius, s.m_fRadius + 150);
		m_sTitle = "Last Helicopter";
		m_sBrief = string.Format("Bastion's last working helicopter is setting down at the LZ outside %1 on its run between bases. It lifts off in %2 minutes. Keep the LZ clear and be there when it goes.",
			s.m_sName, RZ_Util.RoundToInt(COUNTDOWN / 60));

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
			m_sBrief = "Infected corpses are piling up around the Halcyon lab. Burn the four piles before they draw more.";
			return true;
		}

		m_iSettlement = PickSettlement(gm, 30, 90);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = s.m_vCenter;
		m_fParticipationRadius = s.m_fRadius + 50;
		PlacePiles(gm, s.m_vCenter, 0, s.m_fRadius * 0.7, 5);
		m_sTitle = "Burn the Bodies";
		m_sDeed = "burnt the dead in {town}";
		m_sBrief = string.Format("Bastion wants the corpse piles in %1 burned before they spread the infection. There are five.", s.m_sName);
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
		m_iSettlement = PickSettlement(gm, 0, 100);
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
		m_sBrief = string.Format("Bastion needs eyes on the hills around %1. Reach three observation points and watch each for 15 seconds.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
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
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 25, 25);
		m_Req.m_fMinMilitary = 0.2;
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
		m_iSettlement = PickSettlement(gm, 55, 95);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.9, s.m_fRadius + 80);
		m_sTitle = "Checkpoint Duty";
		m_sDeed = "stood a shift at the checkpoint near {town}";
		m_sBrief = string.Format("Bastion is sealing off %1. Man the checkpoint at the edge of town for 10 minutes while the barriers go up. Let survivors through, stop everything else.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Quarantine Line's result: the town is sealed off and stops spreading for 30 minutes.
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return;

		s.m_fNoSpreadSeconds = 1800;
		gm.NotifyAll("Quarantine", s.m_sName + " has been sealed off. It won't spread the infection for a while.");
	}
}

//------------------------------------------------------------------------------------------------
//! #6 Missing Patrol: find a squad's last position and deal with what's left of them.
class RZ_Mission_MissingPatrol : RZ_Mission_KillTagged
{
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
		m_iSettlement = PickSettlement(gm, 20, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius + 200, s.m_fRadius + 600);
		m_sTitle = "Missing Patrol";
		m_sDeed = "found the patrol near {town}";
		m_sBrief = string.Format("A Bastion patrol went silent outside %1. Find their last position and put down whatever they've become.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! What's left of the patrol turns up when someone comes near.
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bSpawned && RZ_ContractHelp.AnyNear(players, m_vPos, 300))
		{
			m_bSpawned = true;
			SpawnGuards(gm, m_vPos, 4, 0, 20);
			SpawnGuards(gm, m_vPos, 1, 0, 10, true);
		}

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
	protected ref array<int> m_aRead = {};

	protected static const float SECURE_SECONDS = 60;
	protected static const int COCKPIT = 0;		// the cockpit's read (HandleTalk)

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

		int bastionTown = PickSettlementAway(gm, m_vPos, 800, 5000, 0, 60);
		int redHandTown = PickSettlementAway(gm, m_vPos, 800, 5000, 0, 100, bastionTown);
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

			m_sTruth = m_Story.Truth(wreckKey, values, weights);
		}

		m_vCamp = LandPointNear(m_vPos, 300, 600);
		if (m_sTruth == "taken")
			m_Carriers = new RZ_EventGroup(gm.GetRedHandGroupPrefab(), m_vCamp);

		SpawnDrop(gm, RZ_EFaction.MILITARY, LandPointNear(b.m_vCenter, b.m_fRadius * 0.8, b.m_fRadius + 50));
		SpawnDrop(gm, RZ_EFaction.BANDITS, LandPointNear(r.m_vCenter, r.m_fRadius * 0.8, r.m_fRadius + 50));

		m_fParticipationRadius = 0;
		m_sTitle = "Crash Site";
		m_sDeed = "went to the crash near {town}";
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

		if (m_bItemSpawned || CountPlayersNear(players, m_vPos, 25) == 0)
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
		}

		if (m_Carriers.IsDead() && SpawnItem(gm, RZ_Util.RandomPointInRing(m_vCamp, 1, 3)))
			RZ_ContractHelp.NotifyNear(gm, m_vCamp, 150, m_sTitle, "The box is by their fire, still in its cut straps. Now decide who gets it.");
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
		if (faction == RZ_EFaction.MILITARY && m_sTruth == "inventory")
		{
			gm.NotifyPlayer(playerId, m_sTitle, "The commander thanks you, takes the box into his office, and you hear the stove door open. \"Weather,\" he says when he comes out. \"Write it down.\"");
			went = "stove";
		}
		else if (faction == RZ_EFaction.BANDITS && m_sTruth == "inventory")
		{
			RZ_Errands errands = gm.GetErrands();
			if (errands)
				errands.Morale(RZ_EFaction.MILITARY, -3, "the manifest on the open channel");

			gm.NotifyAllAs("Open channel", "A Red Hand voice, reading slowly: \"Twelve rifles, signed correct, every one. Turns out they were signed to us.\" Laughter, then static.", RZ_ENoticeTag.EVENT);
		}
		else if (faction == RZ_EFaction.MILITARY)
		{
			epilogue = string.Format("Behind the radio at the post nearest %1, more tags on the nail. Under them, in grease pencil, the weather.", GetLocationName(gm));
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
		if (m_Carriers)
			m_Carriers.Delete();
	}
}

//------------------------------------------------------------------------------------------------
//! #15 Raid the Red Hand: assault a Red Hand camp. Needs the Red Hand group prefab.
//! Not for Ex-Cons or Raiders unless they're Champions of Bastion.
class RZ_Mission_RaidRedHand : RZ_Mission
{
	protected ref array<ref RZ_UnitWatch> m_aGroups = {};
	protected bool m_bSpawned;

	protected static const int GROUPS = 2;

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
		m_iSettlement = PickSettlement(gm, 0, 80);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.8, s.m_fRadius + 250);
		m_fParticipationRadius = 250;
		m_sTitle = "Raid the Red Hand";
		m_sBrief = string.Format("The Red Hand has a camp outside %1. Bastion wants it gone.", s.m_sName);
		return true;
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
			for (int i = 0; i < GROUPS; i++)
			{
				RZ_UnitWatch g = new RZ_UnitWatch();
				if (!g.Spawn(gm.GetRedHandGroupPrefab(), LandPointNear(m_vPos, 0, 25)))
					continue;

				m_aGroups.Insert(g);
				RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, g.m_Unit); // the camp shoots back
			}

			if (m_aGroups.IsEmpty())
				m_eState = RZ_EMissionState.FAILED;

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

		int bastionTown = PickSettlementAway(gm, s.m_vCenter, 800, 5000, 0, 100);
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
		m_sBrief = string.Format("Bastion is 'requisitioning' the Haven's supplies in %1. Take them to the Bastion drop at %2. Or carry them to the Haven in %3 and hand them back. Your call.", s.m_sName, b.m_sName, havenName);
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
