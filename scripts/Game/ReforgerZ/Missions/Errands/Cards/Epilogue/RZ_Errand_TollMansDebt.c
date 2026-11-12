//! The Toll Man's Last Debt (the epilogue deck): a Red Hand toll man wants the tins his crew took from
//! a Haven house carried back before the gates open (2 food reach it). Or sell them back to the crew
//! on the way: Red Hand Favor, and 5 Haven Infamy. Under the Red Hand's route, or with its book
//! balanced.
class RZ_Errand_TollMansDebt : RZ_EpiCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "epi_toll_debt";
		m_sTitle = "The Toll Man's Last Debt";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		Offer("We had three tins off {house} in the spring. The book says square by the end. Walk them back for me. I'm not welcome there.",
			"Last line in my page: three tins, {house}. I'd like it crossed out before the gates open. You carry, I cross.");
		Over("Forget it. Somebody from {house} came for them.");

		RZ_ErrandWay w = Way("give");
		w.m_sLabel = "Carry the tins to {house}";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 40;
		w.m_sStep = "Carry the toll man's three tins to {house} (at the gate), then come back to {giver}.";
		w.m_iFavor = 5;
		w.Done("Crossed out. First line I've ever crossed out going that way.");

		w = Way("press");
		w.m_sLabel = "Sell the tins back to the crew";
		w.m_iFavor = 4;
		w.Done("\"Sold them back to us? That's not what I asked.\" He writes it down anyway. \"It's what I'd have done.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool WhyCard(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Epilogue epi = Week();
		if (!epi || epi.Phase() >= 3 || !RedHandWay(gm))
			return false;

		int house = HouseNear(gm, e.m_vGiver, 3000, -1, 0);
		if (house < 0)
			return false;

		RZ_Safehouse h = gm.GetSafehouses().Get(house);
		e.m_iValue = house;
		e.m_vSubject = h.m_vPos;
		e.m_sSubject = "epi.toll:" + RZ_WorldFacts.Safehouse(house);
		e.SetText("house", h.m_sName);
		e.SetText("subject_place", h.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (w.m_iIndex == 0 && e.m_iValue >= 0)
		{
			gm.GetSafehouses().AddFood(e.m_iValue, 2);
			return;
		}

		gm.AddInfamy(playerId, RZ_EFaction.SURVIVORS, 5, "sold a house's tins back to the crew", 0, "sold a house's tins back to the Red Hand");
	}
}
