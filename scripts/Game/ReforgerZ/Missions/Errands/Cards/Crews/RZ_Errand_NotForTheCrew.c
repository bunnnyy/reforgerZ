//! Not for the Crew (docs/ERRANDS.md, the crews): at a Red Hand camp, after a bite on the roads within
//! 1 km in the last played hour, a bitten lad knows the crew's cut dose is "for outsiders". Walk him
//! to the Halcyon station within 1.5 km, give him your own antidote, or tell the book-keeper. Before
//! the cure only.
class RZ_Errand_NotForTheCrew : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "not_for_crew";
		m_sTitle = "Not for the Crew";
		m_sFamily = "crews";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		Offer("The cut's for outsiders. And for lads worth less than a real one. Guess which I am. There's a station {where_place}. Walk me there?");
		Over("Forget it. The boss found out. It's sorted. Don't ask how.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Walk him to the station";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 40;
		w.m_iPayFaction = RZ_EFaction.INSTITUTE;
		w.m_sStep = "Walk the lad to the Halcyon station {where_place}, weapons slung (at its gate), then come back to {giver}.";
		w.m_iFavor = 4;
		w.Done("They took him in. Patched and sent on to the lab. He'll be a number now. A living one.");

		w = Way("give");
		w.m_sLabel = "Give him your antidote";
		w.m_eItem = RZ_EErrandItem.ANTIDOTE;
		w.m_iItems = 1;
		w.m_iFavor = 6;
		w.Done("A real one? From your own pocket? Mad. Don't tell the others. They'll want one.");

		w = Way("report");
		w.m_sLabel = "Tell the book-keeper";
		w.m_iFame = 2;
		w.Done("The book-keeper's seen to it. He's sent off before dark. The book's square.");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (RZ_WorldFacts.Get().IsCured() || !FreshSignal("bite", e.m_vGiver, 1000, 1, e.m_iAsker) || !gm.GetTerritory())
			return false;

		RZ_Holding station = gm.GetTerritory().FindHolding(RZ_EFaction.INSTITUTE, e.m_vGiver, 1500);
		if (!station)
			return false;

		e.m_vSubject = station.m_vPos;
		e.m_sSubject = "holding:" + e.m_iHolding.ToString() + ".lad";
		e.SetText("where_place", RZ_Util.DirectionText(e.m_vGiver, station.m_vPos));
		e.SetText("subject_place", station.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex == 0)
		{
			array<int> who = {playerId};
			RZ_WorldFacts.Get().Memory().Record("errand.not_for_crew", e.m_sSubject, "to the station", RZ_WorldMemory.SOURCE_ERRAND, false, who);
		}
	}
}
