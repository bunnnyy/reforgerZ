//! Clear the Way (the epilogue deck): the Haven asks for the road to the queue walked and kept clear
//! for a minute at a bend 300-500 m up from it, where people on their way have turned back. Whatever
//! is on the road comes to the noise as usual. Before processing, while the queue stands.
class RZ_Errand_ClearTheWay : RZ_EpiCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "epi_clear_way";
		m_sTitle = "Clear the Way";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("\"Two families turned back on the road to the queue yesterday. Something at the bend. Walk it, and stay a while so they'll walk it too.\"",
			"\"People won't go up the north road past the bend. Clear it, stand there a minute, and the word gets round.\"");
		Over("\"The road's clear. Somebody from Bastion walked it this morning.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Hold the bend on the queue road";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 60;
		w.m_fRadius = 60;
		w.m_sStep = "Walk up to the bend on the road to the queue and keep it clear for a minute, then come back to {giver}.";
		w.m_iFavor = 5;
		w.m_iFame = 1;
		w.Done("\"Clear? Then they'll go. Thank you.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Epilogue epi = Week();
		if (!epi || epi.Phase() >= 3)
			return false;

		vector bend = UpTheRoad(e.m_vGiver, 400);
		if (bend == vector.Zero || RZ_Util.DistXZ(bend, e.m_vGiver) > 4000)
			return false;

		e.m_vSubject = bend;
		e.m_sSubject = "epi.clear:" + RZ_SeasonClock.Day().ToString();
		e.SetText("subject_place", "the bend on the queue road");
		return true;
	}
}
