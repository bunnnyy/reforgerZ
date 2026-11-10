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
//! ★ Nurse: the survivors leave healthier, bonus Fame (and one more of them makes it).
//! The family (3, or 4 with a ★) joins the nearest Haven safehouse.
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

		int rescued = 3;
		foreach (int playerId : m_aParticipants)
		{
			if (HasBonus(gm, playerId))
				rescued = 4;
		}

		if (gm.GetSafehouses())
			gm.GetSafehouses().AddRescued(m_vPos, rescued, m_aParticipants);
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

//------------------------------------------------------------------------------------------------
//! #4 Medicine Run: bring medical supplies from a pharmacy town to a Haven
//! safehouse. Amends: works off Haven Infamy. ★ Nurse/Doctor: bonus Fame and the
//! exact grid of the pharmacy.
class RZ_Mission_MedicineRun : RZ_Mission_Delivery
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 15, 20);
		m_bAmends = true;
		m_Req.m_bAllowWatched = true;
		m_Req.Bonus("nurse");
		m_Req.Bonus("doctor");
		m_fBonusFameMult = 1.5;
		m_fTimeLeft = 2700;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!CanRun(gm, RZ_EFaction.SURVIVORS))
			return 0;

		return 2;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		int safe = PickSafestSettlement(gm);
		RZ_Settlement home = gm.GetSettlement(safe);
		if (!home)
			return false;

		// The drop is at a real Haven safehouse when there are any
		vector dropPos = LandPointNear(home.m_vCenter, 0, home.m_fRadius * 0.4);
		string homeName = home.m_sName;
		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses && houses.Count() > 0)
		{
			RZ_Safehouse house = houses.Get(houses.PickRandom());
			dropPos = LandPointNear(house.m_vPos, 2, 6);
			homeName = house.m_sName;
		}

		m_iSettlement = PickSettlementAway(gm, dropPos, 800, 4000, 20, 90);
		RZ_Settlement pharmacy = gm.GetSettlement(m_iSettlement);
		if (!pharmacy)
			return false;

		m_vPos = LandPointNear(pharmacy.m_vCenter, 0, pharmacy.m_fRadius * 0.6);
		if (!SpawnItem(gm, m_vPos))
			return false;

		SpawnDrop(gm, RZ_EFaction.SURVIVORS, dropPos);
		SpawnGuards(gm, m_vPos, 3, 10, 35);
		m_sTitle = "Medicine run";
		m_sBrief = string.Format("The Haven safehouse in %1 is out of medicine. There's a stocked pharmacy in %2. Bring the supplies back.", homeName, pharmacy.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return super.GetWhereText(gm, from) + "\nPharmacy grid: " + RZ_SettlementScanner.GridReference(m_vPos);
	}
}

//------------------------------------------------------------------------------------------------
//! #5 The Bitten Friend: a Haven member is bitten. Bring an antidote before he
//! turns, or end it yourself (mercy, half the Fame). ★ Nurse: slows his infection.
class RZ_Mission_BittenFriend : RZ_Mission
{
	protected IEntity m_Friend;
	protected float m_fTurnIn = 900;
	protected bool m_bSlowed;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 20, 20);
		m_Req.Bonus("nurse");
		m_Req.Bonus("doctor");
		m_fTimeLeft = 1200;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetSurvivorGroupPrefab().IsEmpty() || gm.GetAntidotePrefab().IsEmpty())
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 60);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.5);
		m_Friend = RZ_Util.SpawnPrefab(gm.GetSurvivorGroupPrefab(), m_vPos);
		if (!m_Friend)
			return false;

		m_fParticipationRadius = 60;
		m_sTitle = "The bitten friend";
		m_sBrief = string.Format("One of ours was bitten in %1. Bring him an antidote before he turns, or do what has to be done.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		// Mercy: he was put down before he turned
		if (RZ_Util.CountAlive(m_Friend) == 0)
		{
			m_fRewardScale = 0.5;
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		vector at = RZ_Util.GetUnitPosition(m_Friend);
		if (!m_bSlowed && AnyBonusNear(gm, ids, players, at, 10))
		{
			m_bSlowed = true;
			m_fTurnIn = m_fTurnIn + 300;
			NotifyParticipants(gm, m_sTitle, "You clean and dress the bite. That buys him about five more minutes.");
		}

		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), at) > 4)
				continue;

			if (RZ_AntidoteHelper.TakeOne(p))
			{
				if (!m_aParticipants.Contains(ids[i]))
					m_aParticipants.Insert(ids[i]);

				m_eState = RZ_EMissionState.SUCCEEDED;
				return;
			}
		}

		m_fTurnIn -= dt;
		if (m_fTurnIn <= 0)
		{
			gm.GetSpawner().SpawnZombie(at, true, m_iSettlement, -1);
			NotifyParticipants(gm, m_sTitle, "Too late. He's one of them now.");
			m_eState = RZ_EMissionState.FAILED;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		RZ_Util.DeleteUnit(m_Friend);
	}
}

//------------------------------------------------------------------------------------------------
//! #9 Missing Scout: search an area for a scout's body and bring his notes home.
//! The notes point to a stash, which is revealed to whoever hands them in.
class RZ_Mission_MissingScout : RZ_Mission_Delivery
{
	protected vector m_vSearchCenter;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 15, 15);
		m_fTimeLeft = 2700;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!CanRun(gm, RZ_EFaction.SURVIVORS))
			return 0;

		return 1.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 10, 80);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		// The body is somewhere in a 250 m area outside town; only the area is given
		m_vSearchCenter = LandPointNear(s.m_vCenter, s.m_fRadius, s.m_fRadius + 400);
		m_vPos = LandPointNear(m_vSearchCenter, 0, 200);
		if (!SpawnItem(gm, m_vPos))
			return false;

		int home = PickSafestSettlement(gm, m_iSettlement);
		RZ_Settlement homeTown = gm.GetSettlement(home);
		if (!homeTown)
			return false;

		SpawnDrop(gm, RZ_EFaction.SURVIVORS, LandPointNear(homeTown.m_vCenter, 0, homeTown.m_fRadius * 0.4));
		SpawnGuards(gm, m_vPos, 2, 10, 30);
		m_sTitle = "Missing scout";
		m_sBrief = string.Format("A Haven scout never came back from %1. Search the area, find his notes and bring them to the safehouse in %2.", s.m_sName, homeTown.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return string.Format("search area %1, %2", RZ_SettlementScanner.GridReference(m_vSearchCenter), RZ_Util.DirectionText(from, m_vSearchCenter));
	}

	//------------------------------------------------------------------------------------------------
	override void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
		if (m_aParticipants.IsEmpty() || m_aParticipants[0] != playerId)
			return;

		// His notes mention a stash
		int idx = PickSettlement(gm, 0, 100);
		RZ_Settlement s = gm.GetSettlement(idx);
		if (!s)
			return;

		vector stash = LandPointNear(s.m_vCenter, s.m_fRadius * 0.5, s.m_fRadius);
		gm.SpawnRewardCrate(stash);
		gm.NotifyPlayer(playerId, "The scout's notes", string.Format("He marked a stash near %1, grid %2.", s.m_sName, RZ_SettlementScanner.GridReference(stash)));
	}
}

//------------------------------------------------------------------------------------------------
//! #10 Clear the Road: wrecks and infected block the road between two Haven
//! towns. Kill everything on it. ★ Mechanic: bonus Fame (tows wrecks away).
class RZ_Mission_ClearTheRoad : RZ_Mission_KillTagged
{
	protected int m_iOther = -1;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 15, 15);
		m_Req.Bonus("mechanic");
		m_fBonusFameMult = 1.5;
		m_fTimeLeft = 2400;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 50);
		RZ_Settlement a = gm.GetSettlement(m_iSettlement);
		if (!a)
			return false;

		m_iOther = PickSettlementAway(gm, a.m_vCenter, 700, 2500, 0, 60);
		RZ_Settlement b = gm.GetSettlement(m_iOther);
		if (!b)
			return false;

		// Spread blockers along the straight line between the two towns
		vector from = a.m_vCenter;
		vector to = b.m_vCenter;
		m_vPos = RZ_Util.GroundPos((from + to) * 0.5);
		m_fParticipationRadius = RZ_Util.DistXZ(from, to) * 0.5 + 100;
		for (int i = 1; i <= 4; i++)
		{
			vector p = RZ_Util.GroundPos(from + (to - from) * (i / 5.0));
			SpawnGuards(gm, p, 2, 0, 25);
		}

		m_bSpawned = true;
		m_sTitle = "Clear the road";
		m_sBrief = string.Format("The road from %1 to %2 is blocked by wrecks and infected. Clear every one of them.", a.m_sName, b.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Settlement a = gm.GetSettlement(m_iSettlement);
		RZ_Settlement b = gm.GetSettlement(m_iOther);
		if (a)
			a.AddInfection(-5);
		if (b)
			b.AddInfection(-5);
	}
}

//------------------------------------------------------------------------------------------------
//! #13 Signal Fires: light beacons on three hills within two minutes of each
//! other. Needs at least three players online. Nearby towns' infection drops.
class RZ_Mission_SignalFires : RZ_Mission
{
	protected ref array<vector> m_aHills = {};
	protected ref array<float> m_aLitAt = {};
	protected float m_fClock;

	protected static const float WINDOW = 120;
	protected static const float LIGHT_RADIUS = 10;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.TRUSTED, 25, 25);
		m_Req.m_iMinPlayers = 3;
		m_fTimeLeft = 2700;
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

		m_vPos = s.m_vCenter;
		m_fParticipationRadius = s.m_fRadius + 900;
		for (int i = 0; i < 3; i++)
		{
			// One hill per third of the circle around town
			vector center = s.m_vCenter;
			float angle = i * Math.PI2 / 3;
			center[0] = center[0] + Math.Cos(angle) * (s.m_fRadius + 500);
			center[2] = center[2] + Math.Sin(angle) * (s.m_fRadius + 500);
			m_aHills.Insert(HighPointNear(center, 0, 250));
			m_aLitAt.Insert(-1000);
		}

		m_sTitle = "Signal fires";
		m_sBrief = string.Format("Light the three beacons on the hills around %1 within two minutes of each other, so the Haven's people know it's safe to move. One person can't do this alone.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string grids = "";
		foreach (int i, vector h : m_aHills)
		{
			if (i > 0)
				grids += ", ";

			grids += RZ_SettlementScanner.GridReference(h);
		}

		return super.GetWhereText(gm, from) + "\nBeacons (grid): " + grids;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_fClock += dt;
		int lit = 0;
		foreach (int i, vector hill : m_aHills)
		{
			if (CountPlayersNear(players, hill, LIGHT_RADIUS) > 0)
			{
				if (m_fClock - m_aLitAt[i] > WINDOW)
					NotifyParticipants(gm, m_sTitle, "A beacon is burning.");

				m_aLitAt[i] = m_fClock;
			}

			if (m_fClock - m_aLitAt[i] <= WINDOW)
				lit++;
		}

		if (lit >= m_aHills.Count())
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		foreach (RZ_Settlement other : gm.GetSettlements())
		{
			if (s && RZ_Util.DistXZ(other.m_vCenter, s.m_vCenter) <= 2000)
				other.AddInfection(-10);
		}

		gm.NotifyAll("Signal fires", "Three beacons are burning. The towns nearby take heart.");
	}
}

//------------------------------------------------------------------------------------------------
//! #16 Trader Caravan: defend a Haven trader at a crossroads from a Red Hand raid.
//! Needs the survivor group and Red Hand group prefabs.
class RZ_Mission_TraderCaravan : RZ_Mission
{
	protected IEntity m_Trader;
	protected IEntity m_Raiders;
	protected float m_fRaidIn = 90;
	protected bool m_bRaidSent;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.TRUSTED, 25, 30);
		m_Req.Need(RZ_EFaction.BANDITS, RZ_EStandingCheck.NOT_AT_LEAST_TRUSTED);
		SetRival(RZ_EFaction.BANDITS, 15);
		m_fTimeLeft = 1800;
		m_fParticipationRadius = 120;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetSurvivorGroupPrefab().IsEmpty() || gm.GetRedHandGroupPrefab().IsEmpty())
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 60);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius, s.m_fRadius + 150);
		m_Trader = RZ_Util.SpawnPrefab(gm.GetSurvivorGroupPrefab(), m_vPos);
		if (!m_Trader)
			return false;

		m_sTitle = "Trader caravan";
		m_sBrief = string.Format("A Haven trader stopped at the crossroads outside %1. The Red Hand knows. Get there and keep him alive.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (RZ_Util.CountAlive(m_Trader) == 0)
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (!m_bRaidSent)
		{
			// The raid starts once someone is there to see it
			if (CountPlayersNear(players, m_vPos, 120) == 0)
				return;

			m_fRaidIn -= dt;
			if (m_fRaidIn > 0)
				return;

			m_bRaidSent = true;
			vector from = LandPointNear(m_vPos, 250, 350);
			m_Raiders = RZ_Util.SpawnPrefab(gm.GetRedHandGroupPrefab(), from);
			RZ_Util.SendGroupTo(m_Raiders, m_vPos, gm.GetSpawner().GetMoveWaypointPrefab());
			// The raiders fight the players near them and go for the trader, who stays friendly to players
			RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, m_Raiders);
			if (gm.GetHostility())
				gm.GetHostility().MakeUnitHostileTo(m_Trader, RZ_EFaction.BANDITS);
			NotifyParticipants(gm, m_sTitle, "Red Hand raiders incoming!");
			return;
		}

		if (RZ_Util.CountAlive(m_Raiders) == 0)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.SpawnRewardCrate(RZ_Util.RandomPointInRing(m_vPos, 3, 6));
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		RZ_Util.DeleteUnit(m_Trader);
		RZ_Util.DeleteUnit(m_Raiders);
	}
}

//------------------------------------------------------------------------------------------------
//! #17 Rescue the Kids: get children out of a school without making noise. Hold
//! the school for 90 s; if the area gets too loud, a horde comes.
//! ★ Hunter: quieter approach (hold time runs a third faster).
class RZ_Mission_RescueTheKids : RZ_Mission_Hold
{
	protected bool m_bHordeSent;

	protected static const float LOUD_HEAT = 40;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.TRUSTED, 30, 30);
		m_Req.Bonus("hunter");
		m_fBonusHoldSpeed = 1.33;
		m_fHoldNeeded = 90;
		m_fHoldRadius = 25;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 30, 85);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.5);
		SpawnGuards(gm, m_vPos, 2, 20, 50);
		m_sTitle = "Rescue the kids";
		m_sBrief = string.Format("Children are hiding in the school in %1. Get them out quietly. If it gets loud, everything in town will come.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnHoldTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bHordeSent || gm.GetHeat().GetHeat(m_vPos) < LOUD_HEAT)
			return;

		m_bHordeSent = true;
		gm.GetSpawner().SpawnHorde(m_vPos, 250, m_iId);
		NotifyParticipants(gm, m_sTitle, "Too loud! They're coming. Hold on and get the kids out.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (!m_bHordeSent)
			m_fRewardScale = 1.25; // nobody noticed a thing

		// Three children join the nearest Haven safehouse
		if (gm.GetSafehouses())
			gm.GetSafehouses().AddRescued(m_vPos, 3, m_aParticipants);
	}
}
