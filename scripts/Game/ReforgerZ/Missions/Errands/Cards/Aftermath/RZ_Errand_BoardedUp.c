//! Boarded Up (docs/ERRANDS.md, players' traces): a player's barricade stands in a town within 1.5 km
//! that nobody holds, and the infected are thick there. A family has been sheltering behind it. Walk
//! them to the house, or leave them food. "Whoever boarded this: tell them it held." (a line to you only,
//! never the builder's name).
class RZ_Errand_BoardedUp : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "boarded_up";
		m_sTitle = "Boarded Up";
		m_sFamily = "aftermath";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("\"Somebody boarded up a house in {subject_place}. There's a family behind the boards, our lookout says. Bring them in, or bring them something.\"",
			"\"There are people behind somebody's barricade in {subject_place}. With all that out there. Do us a turn.\"");
		Over("\"They've come in on their own. Or they haven't. The lookout won't say which.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Walk them to the house";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 60;
		w.m_sStep = "Find the barricaded house in {subject_place} and bring the family out (10 s there), then come back to {giver}.";
		w.m_iFavor = 4;
		w.Done("\"In you come. Shoes off. Somebody find them a blanket.\"");

		w = Way("give");
		w.m_sLabel = "Leave them a tin";
		w.m_eItem = RZ_EErrandItem.FOOD;
		w.m_iItems = 1;
		w.m_bGiveThere = true;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 60;
		w.m_sStep = "Leave a tin at the barricade in {subject_place} (5 s there), then tell {giver}.";
		w.m_iFavor = 3;
		w.Done("\"A tin. It's not nothing. It's not much either.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_TownMemory towns = gm.GetTownMemory();
		RZ_WorldFacts facts = gm.GetFacts();
		if (!towns || !facts)
			return false;

		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			if (s.m_fInfection < 30 || RZ_Util.DistXZ(s.m_vCenter, e.m_vGiver) > 1500)
				continue;

			if (!towns.HasFlag(i, RZ_TownMemory.FLAG_BARRICADED) || facts.TownOwner(i).IsHeld())
				continue;

			Place(e, RZ_Mission.LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.5), s.m_sName);
			e.m_sSubject = RZ_WorldFacts.Town(i) + ".boarded";
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex == 0 && dealer.GetGM().GetSafehouses())
			dealer.GetGM().GetSafehouses().AddPopulationAt(e.m_iHouse, 1);

		said.Insert("The father says it to you on the way out: \"Whoever boarded this, tell them it held.\"");
	}
}
