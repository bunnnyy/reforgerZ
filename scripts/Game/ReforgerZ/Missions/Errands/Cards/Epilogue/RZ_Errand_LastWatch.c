//! The Last Watch (the epilogue deck): an outpost that was still its owner's at the cure wants its
//! gate held for ten minutes, one last time, by someone it knows. Leaving early is the step not
//! finishing. The outpost doesn't change hands in the week (the world settles).
class RZ_Errand_LastWatch : RZ_EpiCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "epi_last_watch";
		m_sTitle = "The Last Watch";
		m_eAnchor = RZ_EErrandAnchor.ANY_HOLDING;
		Offer("\"We held this gate all season. Somebody should stand the last proper watch on it. Ten minutes. I'd like it to be someone we know.\"",
			"\"One more watch on the gate, before it doesn't matter. Ten minutes. Then I'll write it down as kept.\"");
		Over("\"Never mind. The lads are taking it themselves.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Stand the gate for ten minutes";
		w.m_ePlace = RZ_EErrandPlace.GATE;
		w.m_fSeconds = 600;
		w.m_fRadius = 30;
		w.m_sStep = "Stand at {here}'s gate for ten minutes, then report to {giver}.";
		w.m_iFavor = 8;
		w.m_iFame = 2;
		w.Done("\"Watch kept. Last one in the book.\" The pen stops. \"Thank you.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		if (!RZ_Factions.IsValid(giver.m_iFaction))
			return false;

		return RZ_Reputation.IsAtLeast(dealer.GetGM().GetTier(playerId, giver.m_iFaction), RZ_ETier.KNOWN);
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Epilogue epi = Week();
		if (!epi || e.m_iHolding < 0)
			return false;

		// Held at the cure by the faction that holds it now
		RZ_EpilogueSave s = epi.Data();
		int i = s.holdIds.Find(e.m_iHolding);
		if (i < 0 || i >= s.holdOwner.Count() || s.holdOwner[i] != e.m_iFaction)
			return false;

		e.m_sSubject = "epi.watch:" + e.m_iHolding.ToString();
		return true;
	}
}
