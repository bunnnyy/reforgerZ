//! Laundry Day (docs/ERRANDS.md, help across lines): a Haven and Bastion truce, a Bastion post within
//! 1.5 km of the house, and a Red Hand roadblock within 1.5 km too. The house washes Bastion's
//! uniforms for bread. Get the sack to the post: round the roadblock (never within 150 m of it, else
//! the crew takes the sack and it's over), or pay the toll twice (20 Red Hand Favor). Delivered: +2
//! Haven Fame, +1 Bastion Fame, +2 food at the house. Or sell the sack to the fence (15 Red Hand
//! Favor): the house loses its bread (-2 food), and 35% it's found out (+3 Haven Infamy). The sack is
//! the card's, not an item: wearing a uniform from it is just Bastion colours.
class RZ_Errand_LaundryDay : RZ_ErrandCard
{
	static const float ROADBLOCK_RADIUS = 150;

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "laundry_day";
		m_sTitle = "Laundry Day";
		m_sFamily = "across";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("We've a truce with the Bastion post {subject_place}: we wash their uniforms, they pay in bread. This week's sack is ready, and there's a Red Hand roadblock on the road in between. The crews don't love anyone who does the army's washing. Don't wear any of it. Please.",
			"The sack of laundry for the soldiers {subject_place} is ready. They pay in bread, and the house needs it. The crews at the roadblock on that road pay in trouble.");
		Over("The truce with the post is off. They'll have to wash their own shirts, and we'll do without the bread.");

		RZ_ErrandWay w = Way("sneak");
		w.m_sLabel = "Carry the sack round the roadblock";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 30;
		w.m_sStep = "Carry the laundry to the Bastion post {subject_place} the long way, never within 150 m of the Red Hand roadblock (10 s at the post). Then come back to {giver}.";
		w.Done("Clean shirts delivered and no trouble on the road. The bread will come tomorrow, and the children will have it first.");

		w = Way("pay");
		w.m_sLabel = "Pay the toll twice";
		w.m_iFavorCost = 20;
		w.m_iPayFaction = RZ_EFaction.BANDITS;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 30;
		w.m_sStep = "Carry the laundry to the Bastion post {subject_place}, paying the roadblock's toll with your Red Hand Favor (10 s at the post). Then come back to {giver}.";
		w.Done("They made you pay twice? For shirts? Thank you. Don't tell the soldiers what it cost. They'd only feel bad, and then they'd stop sending the bread.");

		w = Way("betray");
		w.m_sLabel = "Sell the sack to the fence";
		w.m_iMinTier = RZ_ETier.KNOWN;
		w.m_iTierFaction = RZ_EFaction.BANDITS;
		w.m_iPayFaction = RZ_EFaction.BANDITS;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT2;
		w.m_fSeconds = 10;
		w.m_fRadius = 40;
		w.m_sStep = "Take the sack to the Red Hand roadblock and sell it (10 s there). Then tell {giver} the crew took it.";
		w.m_iFavor = 15;
		w.Done("The crew took it? All of it? Then there's no bread this week, and a post full of soldiers with no clean shirts.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return RZ_Reputation.IsAtLeast(dealer.GetGM().GetTier(playerId, RZ_EFaction.SURVIVORS), RZ_ETier.KNOWN);
	}

	//------------------------------------------------------------------------------------------------
	//! A Haven and Bastion truce, a Bastion post and a Red Hand roadblock both within 1.5 km.
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_FactionLife life = gm.GetFactionLife();
		RZ_Checkpoints posts = gm.GetCheckpoints();
		if (!life || !posts || !life.HasTruce(RZ_EFaction.SURVIVORS, RZ_EFaction.MILITARY))
			return false;

		array<RZ_RoadPost> red = {};
		posts.PostsNear(RZ_EFaction.BANDITS, e.m_vGiver, 1500, red);
		if (red.IsEmpty())
			return false;

		vector green;
		RZ_Holding h = HoldingNear(gm, RZ_EFaction.MILITARY, e.m_vGiver, 1500);
		if (h)
		{
			green = h.m_vPos;
		}
		else
		{
			array<RZ_RoadPost> near = {};
			posts.PostsNear(RZ_EFaction.MILITARY, e.m_vGiver, 1500, near);
			if (near.IsEmpty())
				return false;

			green = near[0].m_vPos;
		}

		Place(e, green, "");
		e.m_vSubject2 = red[0].m_vPos;
		e.m_iValue = red[0].m_iId;
		e.m_sSubject = RZ_WorldFacts.Safehouse(e.m_iHouse) + ".laundry";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The truce holds; going round, never within 150 m of the roadblock while it stands.
	override bool Still(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_FactionLife life = gm.GetFactionLife();
		if (!life || !life.HasTruce(RZ_EFaction.SURVIVORS, RZ_EFaction.MILITARY))
			return false;

		if (e.m_eState != RZ_EErrandState.HELD || e.m_bStepDone || e.m_iWay != 0 || !gm.GetCheckpoints() || !gm.GetCheckpoints().FindPost(e.m_iValue))
			return true;

		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(e.m_iAsker);
		if (!me || RZ_Util.DistXZ(me.GetOrigin(), e.m_vSubject2) > ROADBLOCK_RADIUS)
			return true;

		gm.NotifyPlayer(e.m_iAsker, "Errand: Laundry Day", "You came too close to the roadblock. The crew opened the sack, found Bastion's shirts and kept the lot. The house gets no bread for this week's washing.");
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Safehouses houses = gm.GetSafehouses();
		array<int> who = {playerId};
		if (w.m_iIndex == 2)
		{
			if (houses)
				houses.AddFood(e.m_iHouse, -2);

			if (Math.RandomFloat01() < 0.35)
			{
				gm.AddInfamy(playerId, RZ_EFaction.SURVIVORS, 3, "sold the house's laundry", RZ_RepFlag.ACT);
				dealer.Notify(e, "The crew wore Bastion's shirts to the next toll, and somebody from {house} saw them. +3 Haven Infamy.");
			}

			RZ_WorldFacts.Get().Memory().Record("errand.laundry_day", e.m_sSubject, "sold", RZ_WorldMemory.SOURCE_ERRAND, false, who);
			return;
		}

		dealer.SmallFame(playerId, RZ_EFaction.SURVIVORS, 2);
		dealer.SmallFame(playerId, RZ_EFaction.MILITARY, 1);
		if (houses)
			houses.AddFood(e.m_iHouse, 2);

		RZ_WorldFacts.Get().Memory().Record("errand.laundry_day", e.m_sSubject, "delivered", RZ_WorldMemory.SOURCE_ERRAND, false, who);
	}
}
