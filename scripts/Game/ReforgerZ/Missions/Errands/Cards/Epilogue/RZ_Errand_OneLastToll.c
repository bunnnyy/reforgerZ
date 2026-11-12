//! One Last Toll (the epilogue deck): a crew boss wants a toll stood on the road to the queue, one
//! tin a head, for two minutes. Stand it (Red Hand Favor, 3 Haven Infamy), or talk the crew out of
//! it (a Trust check with the Red Hand; the Haven pays a little). Under the Red Hand's route or with
//! its book balanced, before processing.
class RZ_Errand_OneLastToll : RZ_EpiCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "epi_last_toll";
		m_sTitle = "One Last Toll";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		Offer("\"Everybody's walking up the north road with a bag. A tin a head at the bend. Last toll of the season. Stand it for us.\"",
			"\"The queue road's the only road anyone uses now. A crew ought to be on it, one last time. You'll do.\"");
		Over("\"Forget the toll. The boss called it off.\"");

		RZ_ErrandWay w = Way("press");
		w.m_sLabel = "Stand the toll at the bend";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 120;
		w.m_fRadius = 40;
		w.m_sStep = "Stand the toll at the bend on the road to the queue for two minutes, then bring the tins to {giver}.";
		w.m_iFavor = 6;
		w.Done("\"Tins. Good. The book will miss this road.\"");

		w = Way("talk");
		w.m_sLabel = "Talk the crew out of it";
		w.m_iTrust = 2;
		w.m_iFavor = 3;
		w.m_iPayFaction = RZ_EFaction.SURVIVORS;
		w.Done("\"Fine. Let them walk.\" The boss looks at the road a long time. \"Let them all walk.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool WhyCard(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Epilogue epi = Week();
		if (!epi || epi.Phase() >= 3 || !RedHandWay(gm))
			return false;

		vector bend = UpTheRoad(e.m_vGiver, 400);
		if (bend == vector.Zero)
			return false;

		e.m_vSubject = bend;
		e.m_sSubject = "epi.toll.road:" + RZ_SeasonClock.Day().ToString();
		e.SetText("subject_place", "the bend on the queue road");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex == 0)
			dealer.GetGM().AddInfamy(playerId, RZ_EFaction.SURVIVORS, 3, "stood a toll on the queue road", 0, "tolled people on their way to the queue");
	}
}
