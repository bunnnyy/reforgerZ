//! The Red Hand (Bandits) contracts. See docs/MISSIONS.md, "The Red Hand".

//------------------------------------------------------------------------------------------------
//! #1 Dead Drop: the betrayal path. The Red Hand buys samples, paying in loot
//! and Favor. Every sample sold is one the lab never sees, and Halcyon finds out.
//! ★ Ex-Con: 25% better pay.
class RZ_Mission_BanditDeadDrop : RZ_Mission
{
	protected IEntity m_Drop;
	protected int m_iSold;

	protected static const int BANDIT_FAME_PER_SAMPLE = 8;
	protected static const int BANDIT_FAVOR_PER_SAMPLE = 15;
	protected static const int INSTITUTE_INFAMY_PER_SAMPLE = 6;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.STRANGER, 0, 0); // paid per sample instead
		m_Req.m_bNotCured = true;
		m_Req.Bonus("excon");
		m_Req.Bonus("raider");
		m_fTimeLeft = 1500;
		m_fParticipationRadius = 0; // only sellers count
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetDeadDropPrefab().IsEmpty())
			return 0;

		// The Red Hand gets bolder as order collapses
		return 0.5 + gm.GetDirector().GetStage() * 0.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 10, 70);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.3, s.m_fRadius * 0.9);
		m_Drop = RZ_Util.SpawnPrefab(gm.GetDeadDropPrefab(), m_vPos, Math.RandomFloat(0, 360));
		if (!m_Drop)
			return false;

		m_sTitle = "Red Hand dead drop";
		m_sBrief = string.Format("The Red Hand is paying well for infected samples. No questions asked. There's a drop near %1.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool IsDrop(IEntity e)
	{
		return e && e == m_Drop;
	}

	//------------------------------------------------------------------------------------------------
	void OnSold(RZ_GameModeComponent gm, int playerId, int value)
	{
		m_iSold += value;
		if (!m_aParticipants.Contains(playerId))
			m_aParticipants.Insert(playerId);

		float pay = 1;
		if (HasBonus(gm, playerId))
			pay = 1.25;

		int fame = RZ_Util.RoundToInt(BANDIT_FAME_PER_SAMPLE * value * pay);
		int favor = RZ_Util.RoundToInt(BANDIT_FAVOR_PER_SAMPLE * value * pay);
		gm.AddFame(playerId, RZ_EFaction.BANDITS, fame, false);
		gm.AddFavor(playerId, RZ_EFaction.BANDITS, favor);
		gm.AddInfamy(playerId, RZ_EFaction.INSTITUTE, INSTITUTE_INFAMY_PER_SAMPLE * value);
		gm.NotifyPlayer(playerId, "The Red Hand", string.Format("Pleasure doing business. +%1 Fame, +%2 Favor with the Red Hand. Halcyon won't like it.", fame, favor));
		if (gm.GetLeaders())
			gm.GetLeaders().OnSamplesSold(playerId);

		// One crate per sale, bigger sales get two
		gm.SpawnRewardCrate(RZ_Util.RandomPointInRing(m_vPos, 2, 4));
		if (value * pay >= 3)
			gm.SpawnRewardCrate(RZ_Util.RandomPointInRing(m_vPos, 2, 4));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		// The drop runs until the Red Hand packs up; selling doesn't end it early
		if (m_iSold > 0)
			m_eState = RZ_EMissionState.SUCCEEDED;
		else
			m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.NotifyAll("Halcyon", string.Format("Someone sold %1 samples to the Red Hand. That's research we'll never get back.", m_iSold));
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		RZ_Util.DeleteUnit(m_Drop);
	}
}

//------------------------------------------------------------------------------------------------
//! #2 Fence the Goods: carry a crate of stolen goods to a Red Hand fence in another
//! town. Pays mostly Favor. Amends: works off Red Hand Infamy. ★ Ex-Con: +50% Fame, extra Favor.
class RZ_Mission_FenceTheGoods : RZ_Mission_Delivery
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.STRANGER, 8, 40);
		m_bAmends = true;
		m_Req.m_bAllowWatched = true;
		m_Req.Bonus("excon");
		m_Req.Bonus("raider");
		m_fBonusFameMult = 1.5;
		m_fTimeLeft = 2700;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!CanRun(gm, RZ_EFaction.BANDITS))
			return 0;

		return 1.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 80);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		int fenceIdx = PickSettlementAway(gm, s.m_vCenter, 1000, 4000, 0, 100);
		RZ_Settlement fence = gm.GetSettlement(fenceIdx);
		if (!fence)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.6);
		if (!SpawnItem(gm, m_vPos))
			return false;

		SpawnDrop(gm, RZ_EFaction.BANDITS, LandPointNear(fence.m_vCenter, fence.m_fRadius * 0.7, fence.m_fRadius + 50));
		m_sTitle = "Fence the goods";
		m_sBrief = string.Format("A Red Hand crew left a crate of stolen goods in %1. Get it to the fence in %2.", s.m_sName, fence.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
		if (HasBonus(gm, playerId))
			gm.AddFavor(playerId, RZ_EFaction.BANDITS, 20);
	}
}

//------------------------------------------------------------------------------------------------
//! #4 Pharmacy Heist: clear a pharmacy before the Haven gets there (hold 90 s).
class RZ_Mission_PharmacyHeist : RZ_Mission_Hold
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.STRANGER, 12, 20);
		SetRival(RZ_EFaction.SURVIVORS, 5);
		m_fHoldNeeded = 90;
		m_fHoldRadius = 25;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 20, 80);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.5);
		SpawnGuards(gm, m_vPos, 3, 15, 40);
		m_sTitle = "Pharmacy heist";
		m_sBrief = string.Format("The Haven is sending people to the pharmacy in %1. Get there first and clean it out.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.SpawnRewardCrate(m_vPos);
	}
}

//------------------------------------------------------------------------------------------------
//! #5 Fight Pit: survive waves of infected in a ring while the Red Hand bets on you.
class RZ_Mission_FightPit : RZ_Mission_Hold
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 15, 50);
		m_fHoldNeeded = 240;
		m_fHoldRadius = 25;
		m_fWaveInterval = 45;
		m_iMaxWaves = 5;
		m_fWaveDistance = 150;
		m_fWaveTimer = 40;
		m_fParticipationRadius = 40;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 70);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.9, s.m_fRadius + 100);
		m_sTitle = "Fight pit";
		m_sBrief = string.Format("The Red Hand runs a fight pit outside %1. Step into the ring and survive four minutes. They're betting on you.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnWave(RZ_GameModeComponent gm, int wave)
	{
		NotifyParticipants(gm, m_sTitle, string.Format("Round %1! The crowd roars.", wave));
	}
}

//------------------------------------------------------------------------------------------------
//! #6 Toll Road: set up a roadblock between two towns and hold it for 10 minutes.
class RZ_Mission_TollRoad : RZ_Mission_Hold
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 20, 30);
		SetRival(RZ_EFaction.SURVIVORS, 10);
		m_fHoldNeeded = 600;
		m_fHoldRadius = 30;
		m_fWaveInterval = 180;
		m_iMaxWaves = 2;
		m_fTimeLeft = 2400;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 70);
		RZ_Settlement a = gm.GetSettlement(m_iSettlement);
		if (!a)
			return false;

		int other = PickSettlementAway(gm, a.m_vCenter, 600, 2500, 0, 100);
		RZ_Settlement b = gm.GetSettlement(other);
		if (!b)
			return false;

		m_vPos = LandPointNear((a.m_vCenter + b.m_vCenter) * 0.5, 0, 40);
		m_sTitle = "Toll road";
		m_sBrief = string.Format("Set up a Red Hand roadblock on the road between %1 and %2 and hold it for 10 minutes. Everyone pays.", a.m_sName, b.m_sName);
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! #7 Smuggler's Run: carry a package across the map. Everyone else is told a
//! courier is out, with a rough location every few minutes. ★ Hunter: the
//! updates are less frequent (quiet route).
class RZ_Mission_SmugglersRun : RZ_Mission_Delivery
{
	protected float m_fReportTimer;

	protected static const float REPORT_SECONDS = 180;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 15, 80);
		m_Req.Bonus("hunter");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!CanRun(gm, RZ_EFaction.BANDITS))
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

		int destIdx = PickSettlementAway(gm, s.m_vCenter, 2000, 10000, 0, 100);
		if (destIdx < 0)
			destIdx = PickSettlementAway(gm, s.m_vCenter, 1000, 10000, 0, 100);

		RZ_Settlement dest = gm.GetSettlement(destIdx);
		if (!dest)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.5);
		if (!SpawnItem(gm, m_vPos))
			return false;

		SpawnDrop(gm, RZ_EFaction.BANDITS, LandPointNear(dest.m_vCenter, dest.m_fRadius * 0.7, dest.m_fRadius + 50));
		m_sTitle = "Smuggler's run";
		m_sBrief = string.Format("Pick up a Red Hand package in %1 and get it to %2. Word will get out that a courier is on the road.", s.m_sName, dest.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnCarrierChanged(RZ_GameModeComponent gm, int previous, int carrier)
	{
		super.OnCarrierChanged(gm, previous, carrier);
		if (carrier > 0 && previous <= 0)
		{
			m_fReportTimer = 0;
			ReportCourier(gm);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDeliveryTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_iCarrier <= 0)
			return;

		float interval = REPORT_SECONDS;
		if (HasBonus(gm, m_iCarrier))
			interval = REPORT_SECONDS * 2;

		m_fReportTimer += dt;
		if (m_fReportTimer >= interval)
		{
			m_fReportTimer = 0;
			ReportCourier(gm);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ReportCourier(RZ_GameModeComponent gm)
	{
		IEntity courier = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iCarrier);
		if (!courier)
			return;

		RZ_Settlement near = gm.GetSettlement(gm.FindNearestSettlement(courier.GetOrigin()));
		string where = RZ_SettlementScanner.GridReference(courier.GetOrigin());
		if (near)
			where = "near " + near.m_sName;

		array<int> all = {};
		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int playerId : all)
		{
			if (playerId != m_iCarrier)
				gm.NotifyPlayer(playerId, "Radio chatter", "A Red Hand courier was seen " + where + ". Whatever they're carrying is worth a lot.");
		}
	}
}

//------------------------------------------------------------------------------------------------
//! #8 Raid the Cache: beat the Haven to a supply cache. Only offered to players
//! the Haven already watches.
class RZ_Mission_RaidTheCache : RZ_Mission_Hold
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 15, 25);
		m_Req.Need(RZ_EFaction.SURVIVORS, RZ_EStandingCheck.AT_LEAST_WATCHED);
		SetRival(RZ_EFaction.SURVIVORS, 10);
		m_fHoldNeeded = 60;
		m_fHoldRadius = 25;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 60);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.6);
		SpawnGuards(gm, m_vPos, 2, 15, 40);
		m_sTitle = "Raid the cache";
		m_sBrief = string.Format("The Haven stashed supplies in %1. Get there before they do and hold it for a minute.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.SpawnRewardCrate(m_vPos);
	}
}

//------------------------------------------------------------------------------------------------
//! #12 Zombie Bait: make enough noise at a Bastion checkpoint that a horde comes
//! for it. ★ Radio Op: uses a radio to lure, twice the noise.
class RZ_Mission_ZombieBait : RZ_Mission
{
	protected static const float HEAT_NEEDED = 60;
	protected static const float LURE_RADIUS = 60;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.TRUSTED, 25, 35);
		m_Req.Bonus("radioham");
		m_Req.m_fMinMilitary = 0.1;
		SetRival(RZ_EFaction.MILITARY, 15);
		m_fParticipationRadius = LURE_RADIUS;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 70);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.9, s.m_fRadius + 60);
		m_sTitle = "Zombie bait";
		m_sBrief = string.Format("Bastion has a checkpoint outside %1. Make enough noise there that every infected around comes for it. Then get out.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		int near = CountPlayersNear(players, m_vPos, LURE_RADIUS);
		if (near > 0)
		{
			float rate = 1;
			if (AnyBonusNear(gm, ids, players, m_vPos, LURE_RADIUS))
				rate = 2;

			gm.GetHeat().AddHeat(m_vPos, rate * dt);
		}

		if (gm.GetHeat().GetHeat(m_vPos) < HEAT_NEEDED)
			return;

		gm.GetSpawner().SpawnHorde(m_vPos, 400);
		NotifyParticipants(gm, m_sTitle, "They heard it. A horde is coming. Get clear!");
		m_eState = RZ_EMissionState.SUCCEEDED;
	}
}

//------------------------------------------------------------------------------------------------
//! #14 Arson: burn a Bastion fuel depot. Needs a destructible fuel depot prefab.
class RZ_Mission_Arson : RZ_Mission_DestroyTarget
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.TRUSTED, 25, 35);
		SetRival(RZ_EFaction.MILITARY, 15);
		m_fTimeLeft = 2700;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetFuelDepotPrefab().IsEmpty())
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 70);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.8, s.m_fRadius + 150);
		if (!SpawnTarget(gm.GetFuelDepotPrefab(), m_vPos))
			return false;

		m_fParticipationRadius = 150;
		m_sTitle = "Arson";
		m_sBrief = string.Format("Bastion keeps a fuel depot outside %1. Burn it.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.GetHeat().AddHeat(m_vPos, 60);
		gm.NotifyAll("Explosion", "A fuel depot went up outside " + GetLocationName(gm) + ". Bastion is furious.");
	}
}

//------------------------------------------------------------------------------------------------
//! #15 Sabotage the Relay: destroy a Haven radio relay. Needs a destructible relay prefab.
class RZ_Mission_SabotageRelay : RZ_Mission_DestroyTarget
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.TRUSTED, 20, 30);
		m_Req.Need(RZ_EFaction.SURVIVORS, RZ_EStandingCheck.NOT_AT_LEAST_TRUSTED);
		SetRival(RZ_EFaction.SURVIVORS, 15);
		m_fTimeLeft = 2700;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetRelayPrefab().IsEmpty())
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

		m_vPos = HighPointNear(s.m_vCenter, s.m_fRadius * 0.5, s.m_fRadius * 1.5);
		if (!SpawnTarget(gm.GetRelayPrefab(), m_vPos))
			return false;

		m_fParticipationRadius = 150;
		m_sTitle = "Sabotage the relay";
		m_sBrief = string.Format("The Haven talks to its safehouses through a relay near %1. Take it down.", s.m_sName);
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! #16 Steal the Antidotes: get into the lab and hold its store room for 90 s.
//! Only once the lab is making antidotes. ★ Lab Tech: knows where they're kept (twice as fast).
class RZ_Mission_StealAntidotes : RZ_Mission_Hold
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.TRUSTED, 35, 40);
		m_Req.m_fMinCure = 50;
		m_Req.m_bNotCured = true;
		m_Req.Bonus("labtech");
		m_fBonusHoldSpeed = 2;
		SetRival(RZ_EFaction.INSTITUTE, 30);
		m_fHoldNeeded = 90;
		m_fHoldRadius = 20;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!gm.HasLab() || gm.GetAntidotePrefab().IsEmpty())
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = gm.GetLabSettlement();
		m_vPos = gm.GetLabPosition();
		m_fParticipationRadius = 40;
		m_sTitle = "Steal the antidotes";
		m_sBrief = "Halcyon is sitting on a batch of antidotes. Get into the lab's store room and hold it for 90 seconds while the crew loads up.";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.SpawnAntidoteAt(RZ_Util.RandomPointInRing(m_vPos, 2, 4));
		gm.SpawnAntidoteAt(RZ_Util.RandomPointInRing(m_vPos, 2, 4));
		gm.NotifyAll("Halcyon", "Someone robbed the lab's antidote store. The Red Hand will be selling them soon.");
	}
}
