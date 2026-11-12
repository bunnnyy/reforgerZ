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
		// The drop is outside a Red Hand town or one nobody holds
		m_iSettlement = PickSettlementNotHeldBy(gm, 10, 70, OthersBits(RZ_EFaction.BANDITS), this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.3, s.m_fRadius * 0.9);
		m_Drop = RZ_Util.SpawnPrefab(gm.GetDeadDropPrefab(), m_vPos, Math.RandomFloat(0, 360));
		if (!m_Drop)
			return false;

		m_sTitle = "Dead Drop";
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
		// The crate was left in a town nobody lives in; the fence is in a Red Hand town when one is in reach
		m_iSettlement = PickEmptySettlement(gm, 0, 80, this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		int fenceIdx = PickHeldSettlementAway(gm, RZ_EFaction.BANDITS, s.m_vCenter, 1000, 4000, m_iSettlement, this);
		if (fenceIdx < 0)
			fenceIdx = PickSettlementAwayNotHeldBy(gm, s.m_vCenter, 1000, 4000, 0, 100, OthersBits(RZ_EFaction.BANDITS), m_iSettlement, this);
		RZ_Settlement fence = gm.GetSettlement(fenceIdx);
		if (!fence)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.6);
		if (!SpawnItem(gm, m_vPos))
			return false;

		SpawnDrop(gm, RZ_EFaction.BANDITS, LandPointNear(fence.m_vCenter, fence.m_fRadius * 0.7, fence.m_fRadius + 50));
		m_sTitle = "Fence the Goods";
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
		m_iSettlement = PickEmptySettlement(gm, 20, 80, this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		// A real pharmacy (or clinic) when the town has one; otherwise the brief doesn't name a building
		array<string> kinds = {"pharmacy"};
		RZ_Landmark shop = RZ_LandmarkFinder.FindNearest(s.m_vCenter, s.m_fRadius, kinds);
		if (shop)
			m_vPos = RZ_Util.RandomPointInRing(shop.m_vPos, 2, 5);
		else
			m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.5);

		SpawnGuards(gm, m_vPos, 3, 15, 40);
		m_sTitle = "Pharmacy Heist";
		if (shop)
			m_sBrief = string.Format("The Haven is sending people to %1 in %2. Get there first and clean it out.", shop.m_sName, s.m_sName);
		else
			m_sBrief = string.Format("The Haven is sending people to pick %1 clean of medicine. Get there first, hold the middle of town and clean it out.", s.m_sName);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.SpawnRewardCrate(m_vPos);
	}
}

//------------------------------------------------------------------------------------------------
//! #5 Fight Pit: survive waves of infected in a ring while the Red Hand bets on you. The ring is
//! the season's one pit (the canon spot redhand.pit, RZ_RedHandEdges.FindPit), and it holds it.
class RZ_Mission_FightPit : RZ_Mission_Hold
{
	//------------------------------------------------------------------------------------------------
	void RZ_Mission_FightPit()
	{
		m_Story = new RZ_MissionStory("pit");
		m_Choice = new RZ_OneChoice();
		m_Choice.Set("Between rounds a man you don't know grabs the rope: \"I've my whole debt on you going down in this round. Go down and I'm out of the book. You'll get half. Please.\" Answer him at the rope.",
			"Fight it out|Go down this round", "decide how the round ends", 0);
		m_Choice.Asset(1, RZ_CureFinale.INDEP);
		m_Choice.m_bManual = true;
		m_Choice.m_bEnds = false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnChosen(RZ_GameModeComponent gm, int decider, int option)
	{
		if (option != 1)
		{
			NotifyParticipants(gm, m_sTitle, "You fight on. The man at the rope stops shouting. Somebody in the book just got a little longer.");
			return;
		}

		m_fRewardScale = 0.5;
		NotifyParticipants(gm, m_sTitle, "You go down at the bell and stay down. The crowd boos. One man in it doesn't. He walks out of the book tonight.");
		RZ_StoryKit.Record(this, "debtor", RZ_WorldFacts.PIT, "walked free", m_aParticipants);
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void GetClaims(RZ_GameModeComponent gm, notnull array<string> result)
	{
		result.Insert(RZ_WorldFacts.PIT);
	}

	protected string m_sStockCrew;			// Stock the Pit: the caged young's crew ("" none)
	protected ResourceName m_sStockPrefab;

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
		string place;
		if (!RZ_RedHandEdges.FindPit(gm, m_vPos, place))
			return false;

		m_iSettlement = gm.FindNearestSettlement(m_vPos);
		if (IsTownBusy(gm, m_iSettlement))
			return false;

		m_sTitle = "Fight Pit";
		m_sBrief = string.Format("The Red Hand's pit is in %1. Step into the ring and survive four minutes. They're betting on you.", place);

		// Stock the Pit: two caged young come out in the last round (docs/MISSIONS.md)
		RZ_Tribes tribes = gm.GetTribes();
		if (tribes && tribes.GetPeople().IsOn() && tribes.GetPeople().TakePitStock(m_sStockCrew, m_sStockPrefab))
			m_sBrief += string.Format(" \"Fresh hires tonight, from %1.\"", m_sStockCrew);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnWave(RZ_GameModeComponent gm, int wave)
	{
		NotifyParticipants(gm, m_sTitle, string.Format("Round %1! The crowd roars.", wave));
		if (wave == 3)
			m_Choice.Ask(gm, this, m_vPos);

		if (wave < m_iMaxWaves || m_sStockCrew.IsEmpty())
			return;

		// The caged ones, let out into the ring
		for (int i = 0; i < 2; i++)
		{
			gm.GetSpawner().SpawnZombie(LandPointNear(m_vPos, 15, 22), false, m_iSettlement, m_iId, m_sStockPrefab);
		}

		NotifyParticipants(gm, m_sTitle, string.Format("They drag the cages up to the ring and kick the doors open. \"Fresh hires, from %1!\"", m_sStockCrew));
		m_sStockCrew = "";
	}
}

//------------------------------------------------------------------------------------------------
//! #7 Smuggler's Run: carry a package across the map. Everyone else is told a
//! courier is out, with a rough location every few minutes. ★ Hunter: the
//! updates are less frequent (quiet route).
class RZ_Mission_SmugglersRun : RZ_Mission_Delivery
{
	protected float m_fReportTimer;
	protected string m_sTruth;		// goods (as they said), doses (a Haven house's antidotes), pages (Bastion's roster pages)
	protected int m_iHouse = -1;	// doses: the house they came from
	protected vector m_vDrop;
	protected bool m_bLabelRead;

	protected static const float REPORT_SECONDS = 180;
	protected static const int OPT_HAND = 0;
	protected static const int OPT_SECOND = 1;

	//------------------------------------------------------------------------------------------------
	void RZ_Mission_SmugglersRun()
	{
		m_Story = new RZ_MissionStory("smuggle");
		m_Choice = new RZ_OneChoice();
	}

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
		// Never from or to a Bastion town: that's the one place a courier doesn't walk into
		int bastion = RZ_Factions.Bit(RZ_EFaction.MILITARY);
		m_iSettlement = PickSettlementNotHeldBy(gm, 0, 80, bastion, this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		int destIdx = PickSettlementAwayNotHeldBy(gm, s.m_vCenter, 2000, 10000, 0, 100, bastion, m_iSettlement, this);
		if (destIdx < 0)
			destIdx = PickSettlementAwayNotHeldBy(gm, s.m_vCenter, 1000, 10000, 0, 100, bastion, m_iSettlement, this);

		RZ_Settlement dest = gm.GetSettlement(destIdx);
		if (!dest)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.5);
		if (!SpawnItem(gm, m_vPos))
			return false;

		m_vDrop = LandPointNear(dest.m_vCenter, dest.m_fRadius * 0.7, dest.m_fRadius + 50);
		SpawnDrop(gm, RZ_EFaction.BANDITS, m_vDrop);
		Truth(gm, s);
		m_sTitle = "Smuggler's Run";
		m_sBrief = string.Format("Pick up a Red Hand package in %1 and get it to %2. Word will get out that a courier is on the road.", s.m_sName, dest.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! The truth (story key `smuggle`, kept per town): the package is what they said (cigarettes and
	//! tins); a Haven house's antidotes (a house within 4 km); or pages off Bastion's roster (a Bastion
	//! holding on the map). Whoever picks it up feels the weight and reads the tape on it; the crew's
	//! runner at the drop asks for it.
	protected void Truth(RZ_GameModeComponent gm, RZ_Settlement s)
	{
		array<string> values = {"goods", "doses", "pages"};
		array<float> weights = {0.5, 0, 0};
		if (gm.GetSafehouses())
			m_iHouse = gm.GetSafehouses().FindNearest(m_vPos, 4000);

		if (m_iHouse >= 0)
			weights[1] = 0.3;

		bool bastion = RZ_ErrandCard.HoldingNear(gm, RZ_EFaction.MILITARY, m_vPos, 100000) != null;
		if (bastion)
			weights[2] = 0.2;

		m_sTruth = m_Story.Truth(RZ_WorldFacts.Town(m_iSettlement), values, weights, this);
		if ((m_sTruth == "doses" && m_iHouse < 0) || (m_sTruth == "pages" && !bastion))
			m_sTruth = "goods";

		string prompt = "The crew's runner holds out both hands. \"All of it. Unopened, I'd hope.\"";
		string labels = "Hand it over";
		if (m_sTruth == "doses")
		{
			labels += "|Take the doses back to the house they came from";
			m_Choice.Asset(OPT_SECOND, RZ_EFaction.SURVIVORS);
		}
		else if (m_sTruth == "pages")
		{
			labels += "|Take the pages to Bastion's post";
			m_Choice.Asset(OPT_SECOND, RZ_EFaction.MILITARY);
		}

		m_Choice.Set(prompt, labels, "decide where the package goes", OPT_HAND);
		RZ_Log.Info(string.Format("Story: smuggle at %1 is %2", s.m_sName, m_sTruth));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnCarrierChanged(RZ_GameModeComponent gm, int previous, int carrier)
	{
		super.OnCarrierChanged(gm, previous, carrier);
		if (carrier > 0 && previous <= 0)
		{
			m_fReportTimer = 0;
			ReportCourier(gm);
			ReadLabel(gm, carrier);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The carrier feels the weight and reads the tape: the truth, to them and the room.
	protected void ReadLabel(RZ_GameModeComponent gm, int carrier)
	{
		if (m_bLabelRead)
			return;

		m_bLabelRead = true;
		m_Story.m_Deciding.Step(gm, carrier);
		string line = "Cigarettes and tins, by the weight. The tape says nothing at all, which is what tape's for.";
		if (m_sTruth == "doses" && gm.GetSafehouses() && gm.GetSafehouses().Get(m_iHouse))
			line = string.Format("It clinks. Under the tape, a Haven sticker: %1's medicine store. Somebody's antidotes.", gm.GetSafehouses().Get(m_iHouse).m_sName);
		else if (m_sTruth == "pages")
			line = "Paper, a lot of it. Typed names with lines through them and KIA beside the lines. Bastion's roster, or part of it.";

		m_Story.Reveal(gm, this, RZ_WorldFacts.Town(m_iSettlement), carrier, "label", line);
	}

	//------------------------------------------------------------------------------------------------
	override vector ChoiceAt()
	{
		return m_vDrop;
	}

	//------------------------------------------------------------------------------------------------
	//! The runner's ask: hand it over (the crew pays), or take it to the side it was taken from (that
	//! side pays 10 Favor to those with the decider, +5 Red Hand Infamy, and the crew pays nothing).
	override void OnChosen(RZ_GameModeComponent gm, int decider, int option)
	{
		array<int> who = {decider};
		if (option != OPT_SECOND || m_sTruth == "goods")
		{
			RZ_StoryKit.Record(this, "package", RZ_WorldFacts.Town(m_iSettlement), "delivered", who);
			return;
		}

		m_bHasGiver = false;
		m_Choice.BlameWith(gm, this, RZ_EFaction.BANDITS, 5, "took a crew's package to its owners");
		if (m_sTruth == "doses")
		{
			m_Choice.PayWith(gm, this, RZ_EFaction.SURVIVORS, 10);
			RZ_StoryKit.Record(this, "package", RZ_WorldFacts.Town(m_iSettlement), "doses_home", who, "A box of antidotes on a Haven shelf with a red thumbprint on the tape. Nobody's scrubbed it off.", "board");
			NotifyParticipants(gm, m_sTitle, "The doses go back on the house's shelf. The crew hears where its package went.");
			return;
		}

		m_Choice.PayWith(gm, this, RZ_EFaction.MILITARY, 10);
		RZ_StoryKit.Record(this, "package", RZ_WorldFacts.Town(m_iSettlement), "pages_home", who);
		NotifyParticipants(gm, m_sTitle, "The pages go back to Bastion's post. Someone there reads every struck-out name before he says thank you.");
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
//! #12 Zombie Bait: make enough noise at a Bastion post that a horde comes for it. With no
//! Bastion outpost on the map (territory off), the edge of a town Bastion passes through.
//! ★ Radio Op: uses a radio to lure, twice the noise.
class RZ_Mission_ZombieBait : RZ_Mission
{
	//------------------------------------------------------------------------------------------------
	void RZ_Mission_ZombieBait()
	{
		m_Story = new RZ_MissionStory("bait");
		m_Choice = new RZ_OneChoice();
		RZ_HordeWhere.Set(m_Choice, RZ_HordeWhere.BASTION);
	}

	//------------------------------------------------------------------------------------------------
	override void OnChosen(RZ_GameModeComponent gm, int decider, int option)
	{
		// The crews pay for Bastion's long night; anything else is your own idea
		int went = RZ_HordeWhere.Apply(gm, this, option);
		if (went == RZ_HordeWhere.BASTION)
			return;

		m_bHasRival = false;
		if (went == RZ_HordeWhere.EMPTY)
		{
			m_fRewardScale = 0.5;
			NotifyParticipants(gm, m_sTitle, "\"Into an empty town. Lovely. Very kind. Half.\"");
			return;
		}

		m_fRewardScale = 0;
		NotifyParticipants(gm, m_sTitle, "\"Past our own camp. Our own. You're not getting paid for that, and you know why.\"");
	}

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
		m_sTitle = "Zombie Bait";
		RZ_Holding post = PickBastionPost(gm);
		if (post)
		{
			m_iSettlement = post.m_iSettlement;
			if (m_iSettlement < 0)
				m_iSettlement = gm.FindNearestSettlement(post.m_vPos);

			m_vPos = LandPointNear(post.m_vPos, 25, 50);
			m_sBrief = string.Format("Bastion has a post at %1. Make enough noise right outside it that every infected around comes for it. Then get out.", post.m_sName);
			return true;
		}

		// A town on Bastion's way that nobody lives in: the crews bait Bastion, not the Haven's houses
		m_iSettlement = PickEmptySettlement(gm, 0, 70, this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.9, s.m_fRadius + 60);
		m_sBrief = string.Format("We want %1 crawling the next time Bastion comes through. Make enough noise at the edge of town that every infected around comes for it. Then get out.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A random Bastion outpost (never the stronghold) in a town no other mission holds, or null.
	protected RZ_Holding PickBastionPost(RZ_GameModeComponent gm)
	{
		RZ_Territory territory = gm.GetTerritory();
		if (!territory || !territory.IsActive())
			return null;

		array<RZ_Holding> posts = {};
		territory.GetOutposts(RZ_EFaction.MILITARY, posts);
		array<RZ_Holding> free = {};
		foreach (RZ_Holding h : posts)
		{
			if (h.m_iSettlement < 0 || !gm.GetMissions().IsSettlementBusy(h.m_iSettlement))
				free.Insert(h);
		}

		if (free.IsEmpty())
			return null;

		return free.GetRandomElement();
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

		// Within 2 km of a pack's ground, its hunters come (by day its walkers); else the town's own
		bool pack = gm.GetTribes() && gm.GetTribes().GetPeople().PackComes(m_vPos, 2000);
		if (!pack)
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
		// Outside a town with a Bastion outpost, else one nobody holds (never a stronghold: they never fall)
		RZ_TownFilter outposts = new RZ_TownFilter();
		outposts.m_iHeldBy = RZ_EFaction.MILITARY;
		outposts.m_iKinds = RZ_WorldFacts.KindBit(RZ_ETownKind.OUTPOST);
		outposts.m_fMaxInfection = 70;
		m_iSettlement = gm.GetFacts().PickTown(outposts, this);
		if (m_iSettlement < 0)
			m_iSettlement = PickEmptySettlement(gm, 0, 70, this);

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
		// Near a town with a Haven safehouse, else one neither Bastion nor the Red Hand holds
		RZ_TownFilter houses = new RZ_TownFilter();
		houses.m_iHeldBy = RZ_EFaction.SURVIVORS;
		houses.m_fMaxInfection = 60;
		m_iSettlement = gm.GetFacts().PickTown(houses, this);
		if (m_iSettlement < 0)
			m_iSettlement = PickSettlementNotHeldBy(gm, 0, 60, RZ_Factions.Bit(RZ_EFaction.MILITARY) | RZ_Factions.Bit(RZ_EFaction.BANDITS), this);

		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = HighPointNear(s.m_vCenter, s.m_fRadius * 0.5, s.m_fRadius * 1.5);
		if (!SpawnTarget(gm.GetRelayPrefab(), m_vPos))
			return false;

		m_fParticipationRadius = 150;
		m_sTitle = "Sabotage the Relay";
		m_sBrief = string.Format("The Haven talks to its safehouses through a relay near %1. Take it down.", s.m_sName);
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! #16 Steal the Antidotes: get into the lab and hold its store room for 90 s.
//! Only while the lab has doses in stock; up to two go to the Red Hand's black market (the
//! Red Hand quartermaster's antidote, if one is set up, sells only those). ★ Lab Tech: knows
//! where they're kept (twice as fast).
class RZ_Mission_StealAntidotes : RZ_Mission_Hold
{
	protected int m_iWhere;		// the one choice: 0 the black market, 1 a Haven medic

	//------------------------------------------------------------------------------------------------
	void RZ_Mission_StealAntidotes()
	{
		m_Story = new RZ_MissionStory("antidotes");
		m_Choice = new RZ_OneChoice();
		m_Choice.Set("You've the two doses. On the way out a Haven medic is waiting by the wire: \"There's a house with three sick and nothing. The crews will sell them for more than you'll ever see.\" Decide here.",
			"To the black market|To the Haven medic", "decide where the doses go", 0);
		m_Choice.Asset(1, RZ_EFaction.SURVIVORS);
	}

	//------------------------------------------------------------------------------------------------
	override void OnChosen(RZ_GameModeComponent gm, int decider, int option)
	{
		m_iWhere = option;
		if (option != 1)
			return;

		// No Red Hand pay: the doses go to a house's sick
		m_fRewardScale = 0;
		m_iFameReward = 0;
		m_bNoLeaderCredit = true;
		RZ_OneChoice.Morale(gm, RZ_EFaction.SURVIVORS, 1, "doses for a house's sick");
		NotifyParticipants(gm, m_sTitle, "The medic takes them without a word and runs. The crews pay you nothing, and say so, at length.");
	}

	static const int STOLEN_DOSES = 2;

	//------------------------------------------------------------------------------------------------
	//! It holds the lab (a claim, docs/DESIGN.md "World facts"): never while another story is there
	override void GetClaims(RZ_GameModeComponent gm, notnull array<string> result)
	{
		if (IsStandalone())
			result.Insert(RZ_WorldFacts.LAB);
	}

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
		// Only while the lab has doses to steal (they go to the Red Hand's black market)
		if (!gm.HasLab() || gm.GetAntidotePrefab().IsEmpty() || !gm.GetCure() || gm.GetCure().GetLabAntidotes() <= 0)
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = gm.GetLabSettlement();
		m_vPos = gm.GetLabPosition();
		m_fParticipationRadius = 40;
		m_sTitle = "Steal the Antidotes";
		m_sBrief = "Halcyon is sitting on a batch of antidotes and selling them only to its friends. Get into the lab's store room and hold it for 90 seconds while the crew loads up. They'll be on our shelves by tonight.";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		// The crew takes the doses to the black market; the job pays the usual Fame and Favor
		int taken = 0;
		if (gm.GetCure())
			taken = gm.GetCure().StealLabAntidotes(STOLEN_DOSES);

		if (taken > 0 && m_iWhere == 1)
			gm.NotifyAll("Halcyon", string.Format("Someone robbed the lab's antidote store: %1 doses gone. They turned up at a Haven house, in three arms, by morning.", taken));
		else if (taken > 0)
			gm.NotifyAll("Halcyon", string.Format("Someone robbed the lab's antidote store: %1 doses gone. The Red Hand will be selling them.", taken));
		else
			gm.NotifyAll("Halcyon", "Someone broke into the lab's store room. The shelves were already empty.");
	}
}
