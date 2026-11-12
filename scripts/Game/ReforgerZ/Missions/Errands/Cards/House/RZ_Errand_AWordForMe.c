//! A Word for Me (docs/ERRANDS.md, the house): a Champion of the Haven is asked to speak to the
//! house's head for a family that wants the last room. It changes no vote and moves no leader: the
//! answer is a world record, and a line.
class RZ_Errand_AWordForMe : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "a_word_for_me";
		m_sTitle = "A Word for Me";
		m_sFamily = "house";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("Two families want the last room in the house, the one with the window that shuts. The council decides, and {head} listens to you. Say a word for the Novaks? They've the little one, and she coughs at night.");
		Over("It's settled. The room went to the Novaks without you.");

		RZ_ErrandWay w = Way("talk");
		w.m_sLabel = "Speak to {head}";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 15;
		w.m_sStep = "Find {head} at {house} and say a word for the Novaks (10 s beside them), then come back to {giver}.";
		w.Done("{head} said yes because it was you asking. Don't let it go to your head. It's a small room.");

		w = Way("talk");
		w.m_sLabel = "Say you're no one to ask";
		w.Done("That's what the last one said. Fine. We'll draw lots for it, like animals.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return dealer.GetGM().GetTier(playerId, RZ_EFaction.SURVIVORS) == RZ_ETier.CHAMPION;
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Leaders leaders = dealer.GetGM().GetLeaders();
		if (!leaders)
			return false;

		RZ_Leader head = leaders.FindAt(RZ_EFaction.SURVIVORS, e.m_vGiver);
		if (!head || !head.m_bAlive)
			return false;

		e.m_vSubject = head.m_vPos;
		e.SetText("head", head.m_sName);
		e.SetText("subject_place", e.GetText("house"));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex != 0)
			return;

		array<int> who = {playerId};
		RZ_WorldFacts.Get().Memory().Record("errand.a_word_for_me", RZ_WorldFacts.Safehouse(e.m_iHouse), "the room", RZ_WorldMemory.SOURCE_ERRAND, true, who);
	}
}
