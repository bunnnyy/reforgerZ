//! Paint (docs/ERRANDS.md, the crews; Name Day's Brush merged in): at night, with a Bastion road post
//! within 2.5 km of a Red Hand camp. New faces paint a red hand on. Paint one on the barrier unseen (20 s
//! hidden at the post): the crew names you for it and +2 Red Hand Fame. Or say you did (a lie).
class RZ_Errand_Paint : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "paint";
		m_sTitle = "Paint";
		m_sFamily = "crews";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		Offer("\"New faces paint a red hand on. Bastion's barrier {subject_place}. Tonight. Don't get searched.\"",
			"\"Here's a tin and a brush. There's a barrier {subject_place} that's very clean. It shouldn't be.\"");
		Over("\"Somebody beat you to it. It's dripping. Lovely.\"");

		RZ_ErrandWay w = Way("sneak");
		w.m_sLabel = "Paint it on the barrier, unseen";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 20;
		w.m_fRadius = 15;
		w.m_bUnseen = true;
		w.m_sStep = "Paint a red hand on Bastion's barrier {subject_place}, unseen (20 s hidden there), then come back to {giver}.";
		w.m_iFavor = 5;
		w.m_iFame = 2;
		w.Done("\"There it is. Dripping. They'll have it down by breakfast. Doesn't matter. Everyone saw.\"");

		w = Way("lie");
		w.m_sLabel = "Say you did it";
		w.m_iFavor = 2;
		w.Done("\"Did you. Good. I'll look in the morning.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return RZ_Reputation.IsAtLeast(dealer.GetGM().GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.KNOWN);
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		if (!RZ_GameTime.IsNight())
			return false;

		RZ_Checkpoints posts = dealer.GetGM().GetCheckpoints();
		if (!posts)
			return false;

		array<RZ_RoadPost> near = {};
		posts.PostsNear(RZ_EFaction.MILITARY, e.m_vGiver, 2500, near);
		if (near.IsEmpty())
			return false;

		Place(e, near[0].m_vPos, "");
		e.m_sSubject = "post:" + RZ_SettlementScanner.GridReference(near[0].m_vPos) + ".paint";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Still(RZ_Errands dealer, RZ_Errand e)
	{
		return RZ_GameTime.IsNight() || e.m_eState == RZ_EErrandState.HELD;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (w.m_iIndex == 1)
		{
			gm.GetFacts().Memory().Lies().Tell(gm, playerId, "errand.paint", e.m_sSubject, RZ_EFaction.BANDITS, e.m_sGiver, e.m_vSubject, RZ_ELieTest.RUMOUR, 3,
				"The barrier was clean in the morning. Clean all night, the lookout says. The crew remembers who said otherwise.");
			return;
		}

		RZ_Deeds.AddLifeRecord(gm.GetProfile(playerId), "painted", false);
		said.Insert(string.Format("The post's incident sheet, the next day: \"Civilian with a paint tin. Day %1.\"", RZ_SeasonClock.Day()));
	}
}
