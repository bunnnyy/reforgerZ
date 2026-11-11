//! Professional Courtesy (docs/ERRANDS.md, the crews; backgrounds: the Raider). A rival crew's lad
//! wants your opinion on his ambush spot. Give it straight; give it wrong so his crew walks into the
//! infected's trail at dusk (a lie, found by events); or tell Bastion where it is (a crew body seeing
//! it costs you with the Red Hand).
class RZ_Errand_Courtesy : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "courtesy";
		m_sTitle = "Professional Courtesy";
		m_sFamily = "background";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		Offer("\"You've done roads. Everyone can tell. Different crew, no hard feelings: is the bend below {subject_place} a good spot for a stop, or am I an idiot?\"",
			"\"Professional question, one raider to another. The bend by {subject_place}. Would you hit a lorry there?\"");
		Over("\"Never mind. The boss picked somewhere else. He always does.\"");

		RZ_ErrandWay w = Way("talk");
		w.m_sLabel = "Give it to him straight";
		w.m_iFavor = 4;
		w.m_iFame = 1;
		w.Done("\"Huh. The ditch. Never thought of the ditch. Cheers.\"");

		w = Way("lie");
		w.m_sLabel = "Steer him wrong";
		w.m_iFavor = 2;
		w.Done("\"Right on the trail? Brave. All right. If you say so.\"");

		w = Way("report");
		w.m_sLabel = "Tell Bastion where it is";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT2;
		w.m_fSeconds = 5;
		w.m_fRadius = 30;
		w.m_sStep = "Tell Bastion's people {post} about the bend (5 s there).";
		w.m_iPayFaction = RZ_EFaction.MILITARY;
		w.Done("\"Noted. We'll have someone sit on that bend.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return RZ_QuestApproach.HasBackground(dealer.GetGM(), playerId, "raider");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		int town = gm.FindNearestSettlement(e.m_vGiver);
		RZ_Settlement s = gm.GetSettlement(town);
		if (!s)
			return false;

		Place(e, s.m_vCenter, s.m_sName);
		e.m_sSubject = "ambush:" + s.m_sName;
		RZ_Holding post = HoldingNear(gm, RZ_EFaction.MILITARY, e.m_vGiver, 1500);
		if (post)
		{
			e.m_vSubject2 = post.m_vPos;
			e.SetText("post", "at " + post.m_sName);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (w.m_iIndex == 1)
		{
			gm.GetFacts().Memory().Lies().Tell(gm, playerId, "errand.courtesy", e.m_sSubject, RZ_EFaction.BANDITS, e.m_sGiver, e.m_vSubject, RZ_ELieTest.RUMOUR, 3,
				"The rival crew sat on the bend you picked, right across the trail at dusk. Two of them came back. They know who picked it.");
			return;
		}

		if (w.m_iIndex == 2)
		{
			dealer.SmallFame(playerId, RZ_EFaction.MILITARY, 2);
			if (RZ_Witness.Seen(gm, playerId, e.m_vSubject2, RZ_EFaction.BANDITS))
				gm.AddInfamy(playerId, RZ_EFaction.BANDITS, 6, "told Bastion about a crew's ambush", 0, "sold a crew's ambush to the army");
		}
	}
}
