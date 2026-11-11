//! Last Letters (the epilogue deck): the Courier has the houses' letters for people already in the
//! queue. Carry the bag up the north road and hand it out. While the queue stands.
class RZ_Errand_LastLetters : RZ_EpiCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "epi_last_letters";
		m_sTitle = "Last Letters";
		m_eAnchor = RZ_EErrandAnchor.WANDERER;
		m_iWandererRole = RZ_EWandererRole.COURIER;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("\"Letters from the houses for people already in the queue. Last round I'll ever do. My feet say you should do it.\"",
			"\"A bag of letters for the north road. Names on the front. They'll be out of reach once the gates open.\"");
		Over("\"Never mind. I'll walk it myself. Slowly.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Carry the bag to the queue";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 20;
		w.m_fRadius = 40;
		w.m_sStep = "Walk the letters up to the queue on the north road and hand them out (20 s there), then find {giver} again.";
		w.m_iFavor = 4;
		w.m_iFame = 1;
		w.Done("\"All of them? Then the post's done. First time all season.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Epilogue epi = Week();
		vector q = Queue();
		if (!epi || epi.Phase() >= 3 || q == vector.Zero)
			return false;

		e.m_vSubject = q;
		e.m_sSubject = "epi.letters:" + RZ_SeasonClock.Day().ToString();
		e.SetText("subject_place", "the queue on the north road");
		return true;
	}
}
