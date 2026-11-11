//! Clean Slate (docs/ERRANDS.md, the amends family): any faction's outpost, for someone it Watches.
//! The dirtiest job going (the burial pit, scrubbing the cages). Amends through Forgive(): 2 Infamy
//! off, at most 8 a life per faction from errands, never below 15. Nobody watches the man with the
//! shovel.
class RZ_Errand_CleanSlate : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "clean_slate";
		m_sTitle = "Clean Slate";
		m_sFamily = "amends";
		m_eAnchor = RZ_EErrandAnchor.ANY_HOLDING;
		Offer("\"You want back in their good books? There's a pit out back that needs a shift. Nobody watches the one with the shovel.\"",
			"\"The worst job going is yours if you want it. People notice who does the worst job. Eventually.\"");
		Over("\"Shift's covered. Come back another day.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Take the worst shift";
		w.m_ePlace = RZ_EErrandPlace.GATE;
		w.m_fSeconds = 60;
		w.m_fRadius = 30;
		w.m_bAmends = true;
		w.m_sStep = "Work the pit behind {here} (a minute there), then come back to {giver}.";
		w.Done("\"Done? Wash first. Then we'll see about your name.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		if (!RZ_Factions.IsValid(giver.m_iFaction))
			return false;

		return dealer.GetGM().GetTier(playerId, giver.m_iFaction) == RZ_ETier.WATCHED;
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		if (e.m_iHolding < 0)
			return false;

		e.m_sSubject = "holding:" + e.m_iHolding.ToString() + ".pit";
		return true;
	}
}
