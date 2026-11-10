//! The Haven (Survivors) contracts. See docs/MISSIONS.md, "The Haven".

//------------------------------------------------------------------------------------------------
//! #1 Supply Cache: reach a spot in a town and hold it. Guarded by zombies.
//! Players race each other to it, so expect PvP.
class RZ_Mission_SupplyCache : RZ_Mission_Hold
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 10, 10);
		m_fHoldNeeded = 60;
		m_fHoldRadius = 25;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 3;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 15, 85);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.7);
		m_sTitle = "Supply cache";
		m_sBrief = string.Format("A Haven scout radioed about an untouched supply cache in %1. Secure it and hold the area for a minute.", s.m_sName);
		SpawnGuards(gm, m_vPos, 3, 15, 40);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.SpawnRewardCrate(m_vPos);
	}
}

//------------------------------------------------------------------------------------------------
//! #2 Survivors Trapped: reach a barricaded family, then hold out through three
//! waves until help arrives. With a survivor group prefab the survivors are real
//! AI, and the mission fails if they all die. Amends: works off Haven Infamy.
//! ★ Nurse: the survivors leave healthier, bonus Fame.
class RZ_Mission_SurvivorHoldout : RZ_Mission_Hold
{
	protected IEntity m_Survivors;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 20, 20);
		m_bAmends = true;
		m_Req.m_bAllowWatched = true;
		m_Req.Bonus("nurse");
		m_Req.Bonus("doctor");
		m_fBonusFameMult = 1.5;
		m_fHoldNeeded = 150;
		m_fHoldRadius = 40;
		m_fWaveInterval = 45;
		m_iMaxWaves = 3;
		m_fWaveTimer = 45; // first wave comes as soon as you arrive
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetDirector().GetStage() == RZ_EOutbreakStage.EARLY_OUTBREAK)
			return 1;

		return 2.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 30, 90);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.6);
		m_sTitle = "Survivors trapped";
		m_sBrief = string.Format("A family is barricaded inside a house in %1 and they're running out of time. Get to them and hold until help arrives.", s.m_sName);
		m_Survivors = RZ_Util.SpawnPrefab(gm.GetSurvivorGroupPrefab(), m_vPos);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_Survivors && RZ_Util.CountAlive(m_Survivors) == 0)
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		super.OnUpdate(gm, dt, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnHoldStarted(RZ_GameModeComponent gm)
	{
		m_fTimeLeft = Math.Max(m_fTimeLeft, m_fHoldNeeded + 300);
		NotifyParticipants(gm, "Hold the house", "They've heard the commotion. Hold on until help gets here!");
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.SpawnRewardCrate(m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		// The survivors leave with the rescue team (or they're gone)
		RZ_Util.DeleteUnit(m_Survivors);
	}
}

//------------------------------------------------------------------------------------------------
//! #3 Radio Relay: find high ground near a town and keep a beacon running.
//! It's loud, so it builds heat fast and pulls a horde. ★ Radio Op: half the hold time.
class RZ_Mission_RadioRelay : RZ_Mission_Hold
{
	protected bool m_bHordeSent;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 15, 15);
		m_Req.Bonus("radioham");
		m_fBonusHoldSpeed = 2;
		m_fHoldNeeded = 180;
		m_fHoldRadius = 30;
		m_fHeatPerSecond = 2;
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

		m_vPos = HighPointNear(s.m_vCenter, s.m_fRadius * 0.5, s.m_fRadius * 2);
		m_sTitle = "Radio relay";
		m_sBrief = string.Format("Get to the high ground near %1 and keep the Haven relay broadcasting for 3 minutes. Every infected nearby will hear it.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnHoldTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bHordeSent && m_fHeld >= 60)
		{
			m_bHordeSent = true;
			gm.GetSpawner().SpawnHorde(m_vPos, 350, m_iId);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.SpawnRewardCrate(m_vPos);
	}
}

//------------------------------------------------------------------------------------------------
//! #11 Burn the Nest: destroy a zombie hive in a heavily infested town. Needs a
//! destructible nest prefab (e.g. from the Zombie Hive mod).
class RZ_Mission_BurnTheNest : RZ_Mission_DestroyTarget
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.TRUSTED, 20, 25);
		m_fTimeLeft = 3600;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetNestPrefab().IsEmpty())
			return 0;

		return 1.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 60, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.5);
		if (!SpawnTarget(gm.GetNestPrefab(), m_vPos))
			return false;

		m_fParticipationRadius = s.m_fRadius + 100;
		m_sTitle = "Burn the nest";
		m_sBrief = string.Format("Something is breeding them in %1. Find the nest and destroy it.", s.m_sName);
		SpawnGuards(gm, m_vPos, 4, 10, 30);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
		{
			s.AddInfection(-30);
			gm.NotifyAll("Nest destroyed", s.m_sName + " is breathing easier.");
		}

		gm.SpawnSamples(m_vPos, 1);
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailed(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
			s.AddInfection(15);
	}
}
