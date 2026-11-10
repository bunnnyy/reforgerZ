//! Task Force Bastion (Military) contracts. See docs/MISSIONS.md, "Task Force Bastion".

//------------------------------------------------------------------------------------------------
//! #1 Retake the Town: kill enough zombies inside an infested town. Only while
//! Bastion is still around. ★ Police/Soldier: +25% Fame.
class RZ_Mission_ClearSettlement : RZ_Mission
{
	protected int m_iKillsNeeded;
	protected int m_iKills;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.STRANGER, 15, 15);
		m_Req.m_fMinMilitary = 0.3;
		m_Req.Bonus("police");
		m_Req.Bonus("soldier");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 3600;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
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
		m_sTitle = "Retake " + s.m_sName;
		m_sBrief = string.Format("Bastion command wants %1 cleared. Kill %2 infected in the town.", s.m_sName, m_iKillsNeeded);
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
//! Haven #7 Take Back the Town: the same job once Bastion has pulled out (Known with the Haven).
class RZ_Mission_TakeBackTown : RZ_Mission_ClearSettlement
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 15, 15);
		m_Req.m_fMaxMilitary = 0.29;
		m_fTimeLeft = 3600;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 2;
	}

	//------------------------------------------------------------------------------------------------
	override protected void SetText(RZ_Settlement s)
	{
		m_sTitle = "Take back " + s.m_sName;
		m_sBrief = string.Format("The Haven wants to move back into %1. Kill %2 infected in the town.", s.m_sName, m_iKillsNeeded);
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

		return 3 * gm.GetDirector().GetStageSettings().m_fMilitaryPresence;
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
		m_sTitle = "Last helicopter";
		m_sBrief = string.Format("Bastion's last working helicopter is setting down at the LZ outside %1 on its run between bases. It lifts off in %2 minutes. Keep the LZ clear and be there when it goes.",
			s.m_sName, RZ_Util.RoundToInt(COUNTDOWN / 60));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bLanded && m_fTimeLeft < GROUND_TIME)
		{
			m_bLanded = true;
			NotifyInterested(gm, "Last helicopter", "The helicopter is down at the LZ outside " + GetLocationName(gm) + ". Three minutes on the ground.");
		}

		// The rotors are loud
		if (m_bLanded)
			gm.GetHeat().AddHeat(m_vPos, 4 * dt);

		if (!m_bWarned && m_fTimeLeft < 60)
		{
			m_bWarned = true;
			NotifyInterested(gm, "Last helicopter", "One minute until the helicopter lifts off from " + GetLocationName(gm) + ".");
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
//! The pile prefab is optional and only visual.
class RZ_Mission_BurnTheBodies : RZ_Mission_VisitPoints
{
	protected ref array<IEntity> m_aPiles = {};
	protected float m_fInfectionDrop = 10;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.STRANGER, 15, 15);
		m_bAmends = true;
		m_Req.m_bAllowWatched = true;
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
		return 1.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 30, 90);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = s.m_vCenter;
		m_fParticipationRadius = s.m_fRadius + 50;
		PlacePiles(gm, s.m_vCenter, 0, s.m_fRadius * 0.7, 5);
		m_sTitle = "Burn the bodies";
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
			m_aPiles.Insert(RZ_Util.SpawnPrefab(gm.GetCorpsePilePrefab(), p, Math.RandomFloat(0, 360)));
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
//! #5 Checkpoint Duty: hold a checkpoint at a town's edge for 10 minutes against
//! waves. The Red Hand notices. ★ Police: holds faster.
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
		m_iSettlement = PickSettlement(gm, 0, 60);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.9, s.m_fRadius + 80);
		m_sTitle = "Checkpoint duty";
		m_sBrief = string.Format("Man the Bastion checkpoint outside %1 for 10 minutes. Let survivors through, stop everything else.", s.m_sName);
		return true;
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
		SpawnGuards(gm, m_vPos, 4, 0, 20);
		SpawnGuards(gm, m_vPos, 1, 0, 10, true);
		m_bSpawned = true;
		m_sTitle = "Missing patrol";
		m_sBrief = string.Format("A Bastion patrol went silent outside %1. Find their last position and put down whatever they've become.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.SpawnRewardCrate(m_vPos);
	}
}

//------------------------------------------------------------------------------------------------
//! #7 Crash Site: secure a downed helicopter for 60 s, then take its black box.
//! Choice: hand it to Bastion, or sell it to the Red Hand.
//! ★ Mechanic: recovers the black box twice as fast.
class RZ_Mission_CrashSite : RZ_Mission_Choice
{
	protected IEntity m_Wreck;
	protected float m_fSecured;

	protected static const float SECURE_SECONDS = 60;

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
		m_iSettlement = PickSettlement(gm, 0, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius + 150, s.m_fRadius + 600);
		m_Wreck = RZ_Util.SpawnPrefab(gm.GetHeliWreckPrefab(), m_vPos, Math.RandomFloat(0, 360));
		if (!m_Wreck)
			return false;

		int bastionTown = PickSettlementAway(gm, m_vPos, 800, 5000, 0, 60);
		int redHandTown = PickSettlementAway(gm, m_vPos, 800, 5000, 0, 100);
		RZ_Settlement b = gm.GetSettlement(bastionTown);
		RZ_Settlement r = gm.GetSettlement(redHandTown);
		if (!b || !r)
			return false;

		SpawnDrop(gm, RZ_EFaction.MILITARY, LandPointNear(b.m_vCenter, b.m_fRadius * 0.8, b.m_fRadius + 50));
		SpawnDrop(gm, RZ_EFaction.BANDITS, LandPointNear(r.m_vCenter, r.m_fRadius * 0.8, r.m_fRadius + 50));
		SpawnGuards(gm, m_vPos, 3, 15, 40);
		gm.GetHeat().AddHeat(m_vPos, 40);

		m_fParticipationRadius = 0;
		m_sTitle = "Crash site";
		m_sBrief = string.Format("The wreck of a Bastion helicopter lost in the first week lies near %1. Secure it and recover the black box. Bastion's drop is at %2. The Red Hand would pay for it at %3.", s.m_sName, b.m_sName, r.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDeliveryTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bItemSpawned || CountPlayersNear(players, m_vPos, 25) == 0)
			return;

		float rate = 1;
		if (AnyBonusNear(gm, ids, players, m_vPos, 25))
			rate = 2;

		m_fSecured += dt * rate;
		if (m_fSecured < SECURE_SECONDS)
			return;

		if (SpawnItem(gm, RZ_Util.RandomPointInRing(m_vPos, 2, 4)))
			NotifyParticipantsNear(gm, players, ids, "You pull the black box out of the wreck. Now decide who gets it.");
	}

	//------------------------------------------------------------------------------------------------
	protected void NotifyParticipantsNear(RZ_GameModeComponent gm, array<IEntity> players, array<int> ids, string text)
	{
		array<int> near = {};
		GetPlayersNear(ids, players, m_vPos, 60, near);
		foreach (int playerId : near)
		{
			gm.NotifyPlayer(playerId, m_sTitle, text);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		RZ_Util.DeleteUnit(m_Wreck);
	}
}

//------------------------------------------------------------------------------------------------
//! #13 Quarantine Line: hold the edge of an infested town for 10 minutes while
//! it's sealed off. That town stops spreading infection for 30 minutes.
class RZ_Mission_QuarantineLine : RZ_Mission_Hold
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.TRUSTED, 30, 30);
		m_fHoldNeeded = 600;
		m_fHoldRadius = 40;
		m_fWaveInterval = 150;
		m_iMaxWaves = 3;
		m_fTimeLeft = 2400;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1.2;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 55, 95);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius, s.m_fRadius + 60);
		m_sTitle = "Quarantine line";
		m_sBrief = string.Format("Bastion is sealing off %1. Hold the line at the edge of town for 10 minutes while the barriers go up.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
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
//! #15 Raid the Red Hand: assault a Red Hand camp. Needs the Red Hand group prefab.
//! Not for Ex-Cons or Raiders unless they're Champions of Bastion.
class RZ_Mission_RaidRedHand : RZ_Mission
{
	protected ref array<IEntity> m_aGroups = {};
	protected float m_fGrace = 15;

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
		for (int i = 0; i < 2; i++)
		{
			IEntity g = RZ_Util.SpawnPrefab(gm.GetRedHandGroupPrefab(), LandPointNear(m_vPos, 0, 25));
			if (g)
			{
				m_aGroups.Insert(g);
				RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, g); // the camp shoots back
			}
		}

		if (m_aGroups.IsEmpty())
			return false;

		m_fParticipationRadius = 250;
		m_sTitle = "Raid the Red Hand";
		m_sBrief = string.Format("The Red Hand has a camp outside %1. Bastion wants it gone.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_fGrace -= dt;
		if (m_fGrace > 0)
			return;

		foreach (IEntity g : m_aGroups)
		{
			if (RZ_Util.CountAlive(g) > 0)
				return;
		}

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
		foreach (IEntity g : m_aGroups)
		{
			RZ_Util.DeleteUnit(g);
		}
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

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSafestSettlement(gm);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s || gm.GetMissions().IsSettlementBusy(m_iSettlement))
			return false;

		int bastionTown = PickSettlementAway(gm, s.m_vCenter, 800, 5000, 0, 100);
		RZ_Settlement b = gm.GetSettlement(bastionTown);
		if (!b)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.4);
		if (!SpawnItem(gm, m_vPos))
			return false;

		SpawnDrop(gm, RZ_EFaction.SURVIVORS, LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.4));
		SpawnDrop(gm, RZ_EFaction.MILITARY, LandPointNear(b.m_vCenter, b.m_fRadius * 0.8, b.m_fRadius + 50));
		m_fParticipationRadius = 0;
		m_sTitle = "Requisition";
		m_sBrief = string.Format("Bastion is 'requisitioning' the Haven's supplies in %1. Take them to the Bastion drop at %2. Or hand them back to the Haven. Your call.", s.m_sName, b.m_sName);
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
