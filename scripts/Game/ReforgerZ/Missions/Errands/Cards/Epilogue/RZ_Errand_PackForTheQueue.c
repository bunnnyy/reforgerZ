//! Pack for the Queue (the epilogue deck): someone at a Haven house is going to the queue and wants
//! two things from the house they left behind, in an empty village nearby. Fetch both, or bring one
//! and say you couldn't find the other (less pay; the giver believes you). For players the Haven
//! knows, while the house has people.
class RZ_Errand_PackForTheQueue : RZ_EpiCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "epi_pack_queue";
		m_sTitle = "Pack for the Queue";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("We're going up to the processing queue, the whole family. Before we do, I need two things from the house we left on Day 2: the wedding photo, and my mother's coat. Nobody's lived there since. I can't go back in.",
			"I'm not walking up to that barrier without my father's watch and the good scissors. They're still in the old house, where we left everything on Day 2. Bring them, and I'll go.");
		Over("Leave it. We're going as we are.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Fetch both things";
		w.m_ePlace = RZ_EErrandPlace.EMPTY_VILLAGE;
		w.m_fSeconds = 30;
		w.m_fRadius = 40;
		w.m_sStep = "Search the family's old house in {place} for both things (30 s there), then bring them to {giver}.";
		w.Work(RZ_EWork.SEARCH, "Search the house");
		w.m_sInside = "house";
		w.m_eProp = RZ_EErrandProp.CRATE;
		w.m_iFavor = 4;
		w.m_iFame = 1;
		w.Done("Both. You found both. Look at us in that photo. Thank you.");

		w = Way("leave");
		w.m_sLabel = "Bring one, say the other was gone";
		w.m_ePlace = RZ_EErrandPlace.EMPTY_VILLAGE;
		w.m_fSeconds = 10;
		w.m_fRadius = 40;
		w.m_sStep = "Take one of the two things from the old house in {place}, then go back to {giver}.";
		w.m_eProp = RZ_EErrandProp.CRATE;
		w.m_iFavor = 2;
		w.Done("Only the one? People took things. It's all right. It's more than I had this morning.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return RZ_Reputation.IsAtLeast(dealer.GetGM().GetTier(playerId, RZ_EFaction.SURVIVORS), RZ_ETier.KNOWN);
	}

	//------------------------------------------------------------------------------------------------
	override bool WhyCard(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Epilogue epi = Week();
		if (!epi || epi.Phase() >= 3 || e.m_iHouse < 0 || Queue() == vector.Zero)
			return false;

		RZ_Safehouse h = dealer.GetGM().GetSafehouses().Get(e.m_iHouse);
		if (!h || h.m_iPopulation <= 0)
			return false;

		e.m_sSubject = "epi.pack:" + e.m_sGiver;
		return true;
	}
}
