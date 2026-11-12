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
		Offer("You want back in our good books after what you did? There's a burial pit out back that needs a shift on the shovel: lime, then earth, then lime. Nobody likes the one with the shovel, but everybody notices who held it.",
			"People here haven't forgotten you, and not kindly. The worst job going is yours if you want it: the pit behind the wire. People notice who does the worst job. Eventually.");
		Over("The pit shift's covered today. Come back another day.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Take the worst shift";
		w.m_ePlace = RZ_EErrandPlace.GATE;
		w.m_fSeconds = 60;
		w.m_fRadius = 30;
		w.m_bAmends = true;
		w.m_sStep = "Work a shift at the burial pit behind {here} (a minute there), then come back to {giver}.";
		w.Work(RZ_EWork.GRAVE, "Dig at the burial pit", 15);
		w.Done("Done? Wash first, properly, with the carbolic. Then we'll see about your name.");
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
