//! Halcyon (Institute) contracts. See docs/MISSIONS.md, "Halcyon".

//------------------------------------------------------------------------------------------------
//! #1 Alpha Sighting: a special infected has been spotted. Kill it for research
//! samples. This is the main way the cure moves forward.
class RZ_Mission_SampleHunt : RZ_Mission
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.STRANGER, 15, 15);
		m_Req.m_bNotCured = true;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 2.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 40, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.8);
		m_fParticipationRadius = s.m_fRadius + 100;
		m_sTitle = "Alpha sighting";
		m_sBrief = string.Format("Halcyon tracked a mutated infected to %1. Kill it and bring its samples to the lab.", s.m_sName);

		RZ_TrackedZombie alpha = gm.GetSpawner().SpawnZombie(m_vPos, true, m_iSettlement, m_iId);
		return alpha != null;
	}

	//------------------------------------------------------------------------------------------------
	override void OnZombiesKilled(RZ_GameModeComponent gm, int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		if (missionId == m_iId && special)
		{
			m_vPos = pos;
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		// The kill already dropped one sample; an Alpha is worth more
		gm.SpawnSamples(m_vPos, 2);
	}
}

//------------------------------------------------------------------------------------------------
//! #2 Field Samples: kill infected in an infested town while Halcyon's people
//! take readings. Kills only count while someone eligible is in town. Amends:
//! works off Halcyon Infamy. ★ Lab Tech/Nurse: each kill counts double.
class RZ_Mission_FieldSamples : RZ_Mission
{
	protected int m_iTaken;
	protected bool m_bPresent;
	protected bool m_bBonusPresent;

	protected static const int NEEDED = 5;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.STRANGER, 12, 15);
		m_bAmends = true;
		m_Req.m_bAllowWatched = true;
		m_Req.m_bNotCured = true;
		m_Req.Bonus("labtech");
		m_Req.Bonus("nurse");
		m_Req.Bonus("doctor");
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 40, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = s.m_vCenter;
		m_fParticipationRadius = s.m_fRadius;
		m_sTitle = "Field samples";
		m_sBrief = string.Format("Halcyon needs tissue from regular infected. Kill %1 of them in %2 and the lab will take readings.", NEEDED, s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_bPresent = false;
		m_bBonusPresent = false;
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), m_vPos) > m_fParticipationRadius || !IsEligible(gm, ids[i]))
				continue;

			m_bPresent = true;
			if (HasBonus(gm, ids[i]))
				m_bBonusPresent = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnZombiesKilled(RZ_GameModeComponent gm, int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		if (settlementIdx != m_iSettlement || !m_bPresent || special)
			return;

		int worth = count;
		if (m_bBonusPresent)
			worth = count * 2;

		m_iTaken += worth;
		if (m_iTaken >= NEEDED)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.GetCure().AddResearch(1.5, "Field samples from " + GetLocationName(gm) + " came in.");
	}
}

//------------------------------------------------------------------------------------------------
//! #3 Water Survey: take water samples from wells in three towns (stand at each
//! for 10 s). Reveals the town most at risk. ★ Lab Tech: +50% Fame.
class RZ_Mission_WaterSurvey : RZ_Mission_VisitPoints
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.STRANGER, 15, 15);
		m_Req.Bonus("labtech");
		m_fBonusFameMult = 1.5;
		m_fVisitRadius = 12;
		m_fVisitSeconds = 10;
		m_sPointName = "Well";
		m_fTimeLeft = 3600;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetSettlements().Count() < 3)
			return 0;

		return 1.2;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 80);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = s.m_vCenter;
		m_fParticipationRadius = 0;
		AddPoint(LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.4));

		array<int> used = {m_iSettlement};
		int guard = 0;
		while (m_aPoints.Count() < 3 && guard < 20)
		{
			guard++;
			int idx = PickSettlementAway(gm, s.m_vCenter, 500, 3500, 0, 100);
			RZ_Settlement other = gm.GetSettlement(idx);
			if (!other || used.Contains(idx))
				continue;

			used.Insert(idx);
			AddPoint(LandPointNear(other.m_vCenter, 0, other.m_fRadius * 0.4));
		}

		if (m_aPoints.Count() < 3)
			return false;

		m_sTitle = "Water survey";
		m_sBrief = string.Format("Halcyon wants water samples from the wells in %1 and two towns nearby. Stand at each well for 10 seconds.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		// The town most likely to fall next: the most infected one that hasn't yet
		RZ_Settlement risk;
		foreach (RZ_Settlement s : gm.GetSettlements())
		{
			if (s.m_fInfection >= 50)
				continue;

			if (!risk || s.m_fInfection > risk.m_fInfection)
				risk = s;
		}

		if (risk)
			NotifyParticipants(gm, "Halcyon", "The samples say " + risk.m_sName + " is next. Warn anyone there.");
	}
}

//------------------------------------------------------------------------------------------------
//! #5 Lost Colleague: find a missing researcher. Half the time he's alive and
//! reaching him is enough; otherwise he has turned and has to be put down.
class RZ_Mission_LostColleague : RZ_Mission
{
	protected bool m_bReached;
	protected bool m_bTurned;
	protected bool m_bKilled;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.KNOWN, 20, 20);
		m_Req.m_bNotCured = true;
		m_bTurned = Math.RandomFloat01() < 0.5;
		m_fParticipationRadius = 80;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 20, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.5, s.m_fRadius + 300);
		m_sTitle = "Lost colleague";
		m_sBrief = string.Format("A Halcyon researcher stopped answering near %1. Find him. He may not be himself any more.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bReached || CountPlayersNear(players, m_vPos, 20) == 0)
			return;

		m_bReached = true;
		if (!m_bTurned)
		{
			NotifyParticipants(gm, m_sTitle, "He's alive, hiding with his notes. Halcyon will send someone for him.");
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		NotifyParticipants(gm, m_sTitle, "You're too late. He's turned. Put him down; his notes are still on him.");
		if (!gm.GetSpawner().SpawnZombie(RZ_Util.RandomPointInRing(m_vPos, 8, 15), true, m_iSettlement, m_iId))
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnZombiesKilled(RZ_GameModeComponent gm, int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		if (missionId == m_iId)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.GetCure().AddResearch(2, "A lost researcher's notes reached the lab.");
	}
}

//------------------------------------------------------------------------------------------------
//! #6 Research Data: recover a hard drive from an overrun hospital (hold 45 s).
//! Choice: Halcyon (research) or sell it to Bastion (Halcyon Infamy).
class RZ_Mission_ResearchData : RZ_Mission_Choice
{
	protected float m_fSearched;

	protected static const float SEARCH_SECONDS = 45;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.KNOWN, 20, 20);
		m_eChoiceOther = RZ_EFaction.MILITARY;
		m_iDoubleCrossInfamy = 15;
		m_Req.m_bNotCured = true;
		m_fTimeLeft = 3000;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!CanRunChoice(gm, RZ_EFaction.INSTITUTE, RZ_EFaction.MILITARY))
			return 0;

		return 1.2;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 55, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		// Halcyon's drop sits at the lab when there is one
		vector halcyonPos;
		if (gm.HasLab())
		{
			halcyonPos = LandPointNear(gm.GetLabPosition(), 8, 15);
		}
		else
		{
			RZ_Settlement h = gm.GetSettlement(PickSettlementAway(gm, s.m_vCenter, 800, 5000, 0, 70));
			if (!h)
				return false;

			halcyonPos = LandPointNear(h.m_vCenter, 0, h.m_fRadius * 0.5);
		}

		RZ_Settlement b = gm.GetSettlement(PickSettlementAway(gm, s.m_vCenter, 800, 5000, 0, 100));
		if (!b)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.5);
		SpawnDrop(gm, RZ_EFaction.INSTITUTE, halcyonPos);
		SpawnDrop(gm, RZ_EFaction.MILITARY, LandPointNear(b.m_vCenter, b.m_fRadius * 0.8, b.m_fRadius + 50));
		SpawnGuards(gm, m_vPos, 4, 10, 40);
		m_fParticipationRadius = 0;
		m_sTitle = "Research data";
		m_sBrief = string.Format("There's a hard drive with Halcyon's research in the hospital in %1. Search the building, then take it to the lab. Bastion's drop in %2 would pay for it too.", s.m_sName, b.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDeliveryTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bItemSpawned || CountPlayersNear(players, m_vPos, 20) == 0)
			return;

		m_fSearched += dt;
		if (m_fSearched >= SEARCH_SECONDS && SpawnItem(gm, m_vPos))
		{
			array<int> near = {};
			GetPlayersNear(ids, players, m_vPos, 40, near);
			foreach (int playerId : near)
			{
				gm.NotifyPlayer(playerId, m_sTitle, "Found it: the drive is on the floor of the records room.");
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoiceMade(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction)
	{
		if (faction == RZ_EFaction.INSTITUTE)
			gm.GetCure().AddResearch(3, "Recovered research data reached the lab.");
		else
			gm.NotifyAll("Halcyon", "Our research data was sold to Bastion. We know who did it.");
	}
}

//------------------------------------------------------------------------------------------------
//! #12 Hunt the Screamer: kill a Screamer before it alerts the town. After four
//! minutes it screams: a horde comes, but it can still be killed. ★ Hunter: +50% Fame.
class RZ_Mission_HuntScreamer : RZ_Mission
{
	protected float m_fScreamIn = 240;
	protected bool m_bScreamed;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.TRUSTED, 25, 25);
		m_Req.Bonus("hunter");
		m_fBonusFameMult = 1.5;
		m_fTimeLeft = 1500;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 30, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.7);
		m_fParticipationRadius = s.m_fRadius + 100;
		m_sTitle = "Hunt the screamer";
		m_sBrief = string.Format("A Screamer is in %1. Kill it before it calls every infected in town. Halcyon wants what's left.", s.m_sName);
		return gm.GetSpawner().SpawnZombie(m_vPos, true, m_iSettlement, m_iId) != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		// The clock only runs once someone is hunting it
		if (m_bScreamed || m_aParticipants.IsEmpty())
			return;

		m_fScreamIn -= dt;
		if (m_fScreamIn > 0)
			return;

		m_bScreamed = true;
		gm.GetHeat().AddHeat(m_vPos, 80);
		gm.GetSpawner().SpawnHorde(m_vPos, 300, m_iId);
		NotifyParticipants(gm, m_sTitle, "It screamed. Everything in town heard it.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnZombiesKilled(RZ_GameModeComponent gm, int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		if (missionId == m_iId && special)
		{
			m_vPos = pos;
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (!m_bScreamed)
			m_fRewardScale = 1.25;

		gm.SpawnSamples(m_vPos, 1);
	}
}

//------------------------------------------------------------------------------------------------
//! #13 Quarantine Breach: infection has started inside a safe town. Kill the
//! infected there within 10 minutes. Pays Haven Fame too.
class RZ_Mission_QuarantineBreach : RZ_Mission_KillTagged
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.TRUSTED, 25, 25);
		m_fTimeLeft = 600;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 25);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = s.m_vCenter;
		m_fParticipationRadius = s.m_fRadius + 50;
		s.AddInfection(10);
		SpawnGuards(gm, s.m_vCenter, 4 + s.m_iSize, 0, s.m_fRadius * 0.7);
		m_bSpawned = true;
		m_sTitle = "Quarantine breach";
		m_sBrief = string.Format("Infection has broken out inside %1. Halcyon needs it contained in 10 minutes: kill every infected that broke in.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
		gm.AddFame(playerId, RZ_EFaction.SURVIVORS, 10);
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
			s.AddInfection(-15);
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailed(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
		{
			s.AddInfection(25);
			gm.NotifyAll("Quarantine breach", s.m_sName + " couldn't be contained.");
		}
	}
}

//------------------------------------------------------------------------------------------------
//! #14 Decontamination: burn the infected corpses around the lab (four piles).
//! ★ Nurse: faster. The lab's town gets less infected.
class RZ_Mission_Decontamination : RZ_Mission_BurnTheBodies
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.TRUSTED, 20, 20);
		m_Req.Bonus("nurse");
		m_Req.Bonus("doctor");
		m_fBonusVisitSpeed = 1.5;
		m_fVisitRadius = 5;
		m_fVisitSeconds = 10;
		m_sPointName = "Pile";
		m_fInfectionDrop = 15;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!gm.HasLab())
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = gm.GetLabSettlement();
		m_vPos = gm.GetLabPosition();
		m_fParticipationRadius = 150;
		PlacePiles(gm, m_vPos, 20, 100, 4);
		m_sTitle = "Decontamination";
		m_sBrief = "Infected corpses are piling up around the Halcyon lab. Burn the four piles before they draw more.";
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! #15 Lab Defence: repel a raid on the lab (hold 4 minutes against waves). If a Red
//! Hand group prefab is set, raiders join the attack and the Red Hand remembers.
class RZ_Mission_LabDefence : RZ_Mission_Hold
{
	protected IEntity m_Raiders;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.TRUSTED, 30, 30);
		m_Req.m_bNotCured = true;
		m_fHoldNeeded = 240;
		m_fHoldRadius = 60;
		m_fWaveInterval = 60;
		m_iMaxWaves = 4;
		m_fWaveDistance = 300;
		m_fWaveTimer = 55;
		m_fTimeLeft = 1800;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!gm.HasLab() || gm.GetCure().IsFinalStand())
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = gm.GetLabSettlement();
		m_vPos = gm.GetLabPosition();
		m_fParticipationRadius = 100;
		m_sTitle = "Lab defence";
		m_sBrief = "Something is coming for the Halcyon lab. Get there and hold it for four minutes.";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnHoldStarted(RZ_GameModeComponent gm)
	{
		if (gm.GetRedHandGroupPrefab().IsEmpty())
			return;

		m_Raiders = RZ_Util.SpawnPrefab(gm.GetRedHandGroupPrefab(), LandPointNear(m_vPos, 250, 350));
		if (!m_Raiders)
			return;

		RZ_Util.SendGroupTo(m_Raiders, m_vPos, gm.GetSpawner().GetMoveWaypointPrefab());
		SetRival(RZ_EFaction.BANDITS, 10);
		NotifyParticipants(gm, m_sTitle, "Red Hand raiders are using the infected as cover!");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		RZ_Util.DeleteUnit(m_Raiders);
	}
}
