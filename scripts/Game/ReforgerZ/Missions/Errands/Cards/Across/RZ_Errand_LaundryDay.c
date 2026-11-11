//! Laundry Day (docs/ERRANDS.md, help across lines): Known with the Haven; a Haven-Bastion truce, a
//! Bastion post and a Red Hand road post both within 1.5 km of the house. The house washes Bastion's
//! uniforms for bread. Get the sack to the post, the long way round the roadblock, or sell it to the
//! fence (the house loses its bread; a lie its next count finds).
class RZ_Errand_LaundryDay : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "laundry";
		m_sTitle = "Laundry Day";
		m_sFamily = "across";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("\"The soldiers' washing's done. It goes to the post {subject_place}, and the bread comes back. There's a toll on the road. Go round it. Don't wear anything in that sack.\"",
			"\"Laundry day. A sack of Bastion's green, clean, for a sack of bread. The crews sit on the road between. Do us a turn.\"");
		Over("\"The sack's gone already, and the bread's back on the shelf.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Carry it round the roadblock";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 30;
		w.m_sStep = "Carry the sack to Bastion's post {subject_place}, well clear of the crews' roadblock, then come back to {giver} with the bread.";
		w.m_iFavor = 4;
		w.m_iFame = 2;
		w.Done("\"Bread. Proper bread. They even counted it right this week.\"");

		w = Way("lie");
		w.m_sLabel = "Sell the sack to the fence";
		w.m_iPayFaction = RZ_EFaction.BANDITS;
		w.m_iFavor = 6;
		w.Done("\"Lost on the road? To the crews? Well. There's no bread, then.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return RZ_Reputation.IsAtLeast(dealer.GetGM().GetTier(playerId, RZ_EFaction.SURVIVORS), RZ_ETier.KNOWN);
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_FactionLife life = gm.GetFactionLife();
		if (!life || !life.HasTruce(RZ_EFaction.SURVIVORS, RZ_EFaction.MILITARY))
			return false;

		RZ_Holding post = HoldingNear(gm, RZ_EFaction.MILITARY, e.m_vGiver, 1500);
		RZ_Checkpoints roads = gm.GetCheckpoints();
		if (!post || !roads)
			return false;

		array<RZ_RoadPost> tolls = {};
		roads.PostsNear(RZ_EFaction.BANDITS, e.m_vGiver, 1500, tolls);
		if (tolls.IsEmpty())
			return false;

		Place(e, post.m_vPos, post.m_sName);
		e.m_sSubject = RZ_WorldFacts.Safehouse(e.m_iHouse) + ".laundry";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Safehouses houses = gm.GetSafehouses();
		if (w.m_iIndex == 0)
		{
			if (houses)
				houses.AddFood(e.m_iHouse, 2);

			gm.AddFame(playerId, RZ_EFaction.MILITARY, 1, false, RZ_RepFlag.SMALL);
			return;
		}

		RZ_Safehouse h = houses.Get(e.m_iHouse);
		if (h)
			h.m_iFood = Math.Max(0, h.m_iFood - 2);

		gm.GetFacts().Memory().Lies().Tell(gm, playerId, "errand.laundry", e.m_sSubject, RZ_EFaction.SURVIVORS, e.m_sGiver, e.m_vGiver, RZ_ELieTest.TICK, 3,
			"The post sent word: no laundry this week, so no bread. Then a crew lad turned up at the gate in a very clean shirt.");
	}
}
