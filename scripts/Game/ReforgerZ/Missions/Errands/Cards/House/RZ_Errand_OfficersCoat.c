//! The Officer's Coat (docs/ERRANDS.md, the house; a disguise card): you're in Bastion kit as a
//! disguise (below Known with Bastion) at a Haven house. A father asks after his son in the stockade.
//! Tell his you're not Bastion; play the soldier and take a tin "for the paperwork" (a lie, found
//! when he asks at a post); or go to the post yourself and ask: "He's on the roster as KIA, or
//! struck off."
class RZ_Errand_OfficersCoat : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "officers_coat";
		m_sTitle = "The Officer's Coat";
		m_sFamily = "house";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("You're army. You'll know. My son's in your stockade, they told me, three weeks now, for something on the hill road. Nobody at the post will tell me anything. Can you find out? I've a tin for whoever does the paperwork.",
			"Excuse me. You're with Bastion? My boy's with your lot, in the stockade, since the fight on the hill road. Is he eating? Is there a form? I'll fill in any form.");
		Over("Never mind. Someone from the post came by. I know now, and I wish I didn't.");

		RZ_ErrandWay w = Way("talk");
		w.m_sLabel = "Tell his you're not Bastion";
		w.Done("Oh. Not Bastion. Just the jacket, then. Well. Thank you for saying so. Most wouldn't.");

		w = Way("lie");
		w.m_sLabel = "Take the tin 'for the paperwork'";
		w.Done("Thank you. Thank you. I'll wait to hear. I'll be right here.");

		w = Way("do");
		w.m_sLabel = "Ask at the post yourself";
		w.m_iMinTier = RZ_ETier.KNOWN;
		w.m_iTierFaction = RZ_EFaction.MILITARY;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 25;
		w.m_sStep = "Go to the Bastion post {subject_place} out of uniform and ask after his son (10 s there), then come back to {giver}.";
		w.m_iFavor = 4;
		w.Done("On the roster as killed, or struck off? Which? They wouldn't say which? Then I'll sit down. I'll just sit down here a minute.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (!gm.GetHostility())
			return false;

		RZ_Disguise disguise = gm.GetHostility().GetDisguise();
		return disguise.IsOn() && disguise.IsDisguisedAs(playerId, RZ_EFaction.MILITARY);
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Checkpoints posts = dealer.GetGM().GetCheckpoints();
		RZ_Holding post = HoldingNear(dealer.GetGM(), RZ_EFaction.MILITARY, e.m_vGiver, 1500);
		if (post)
		{
			Place(e, post.m_vPos, post.m_sName);
		}
		else if (posts)
		{
			array<RZ_RoadPost> near = {};
			posts.PostsNear(RZ_EFaction.MILITARY, e.m_vGiver, 1500, near);
			if (near.IsEmpty())
				return false;

			Place(e, near[0].m_vPos, "");
		}
		else
		{
			return false;
		}

		e.m_sSubject = RZ_WorldFacts.Safehouse(e.m_iHouse) + ".son";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		if (w.m_iIndex == 0 && trees)
		{
			trees.GetMemory().AddRegard(playerId, e.m_sGiver, "kept");
			return;
		}

		if (w.m_iIndex != 1)
			return;

		// The tin was the house's; the lie comes out when he asks at a post himself
		if (gm.GetSafehouses())
			gm.GetSafehouses().AddFood(e.m_iHouse, -1);

		gm.GetFacts().Memory().Lies().Tell(gm, playerId, "errand.officers_coat", e.m_sSubject, RZ_EFaction.SURVIVORS, e.m_sGiver, e.m_vSubject, RZ_ELieTest.RUMOUR, 3,
			"He asked at the post himself, and there was no paperwork. {house} knows who took the tin.");
	}
}
