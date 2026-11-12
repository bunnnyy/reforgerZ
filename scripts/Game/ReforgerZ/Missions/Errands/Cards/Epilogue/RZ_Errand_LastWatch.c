//! The Last Watch (the epilogue deck): an outpost that was still its owner's at the cure wants its
//! gate held for ten minutes at the evening change of watch (17:00-21:00), one last time, by
//! someone it knows. Leaving early is the step not finishing. The outpost doesn't change hands.
class RZ_Errand_LastWatch : RZ_EpiCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "epi_last_watch";
		m_sTitle = "The Last Watch";
		m_eAnchor = RZ_EErrandAnchor.ANY_HOLDING;
		Offer("We held this gate all season, every night, with whoever we had. The gates open at the end of the week and then it's nobody's. Somebody should stand the last proper watch on it. Ten minutes. I'd like it to be someone we know.",
			"One more watch on our gate, before it stops mattering. Ten minutes. Then I'll write it in the book as kept, the last line on the last page.");
		Over("Never mind. The regulars are standing it themselves.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Stand the gate for ten minutes";
		w.m_ePlace = RZ_EErrandPlace.GATE;
		w.m_fSeconds = 600;
		w.m_fRadius = 30;
		w.m_fBefore = 21.5;
		w.m_sStep = "Stand watch at the gate of {here} for ten minutes, then report to {giver}.";
		w.m_iFavor = 8;
		w.m_iFame = 2;
		w.Done("Watch kept. That's the last one in the book. I'm putting the pen down now. Thank you.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		if (!RZ_Factions.IsValid(giver.m_iFaction))
			return false;

		return RZ_Reputation.IsAtLeast(dealer.GetGM().GetTier(playerId, giver.m_iFaction), RZ_ETier.KNOWN);
	}

	//------------------------------------------------------------------------------------------------
	override bool WhyCard(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Epilogue epi = Week();
		if (!epi || e.m_iHolding < 0)
			return false;

		// The evening change of watch (17:00-21:00), done before half past nine
		if (!RZ_GameTime.IsCalibrated() || RZ_GameTime.GetHour() < 17 || RZ_GameTime.GetHour() >= 21)
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
