//! Come In (the epilogue deck, errands' amends): a faction that Watches or Hunts you, in the last week,
//! asks you to walk up to its gate and wait a minute. Amends through Forgive (2 Infamy off, within
//! errands' amends caps). Before processing; any holding of that faction.
class RZ_Errand_ComeIn : RZ_EpiCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "epi_come_in";
		m_sTitle = "Come In";
		m_eAnchor = RZ_EErrandAnchor.ANY_HOLDING;
		m_sFamily = "amends";
		Offer("It's the last week. Walk up to the gate slowly, and wait. We'll talk after.",
			"Gates open soon. Nobody wants to carry this out with them. Come to the gate and stand there a minute.");
		Over("Not today. Come back when it's quieter.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Walk up to the gate and wait";
		w.m_ePlace = RZ_EErrandPlace.GATE;
		w.m_fSeconds = 60;
		w.m_fRadius = 15;
		w.m_bAmends = true;
		w.m_sStep = "Walk up to {here}'s gate and wait there a minute, then come back to {giver}.";
		w.Done("You came. That's most of it.", "Stood there the whole minute. All right. We'll start again from there.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		if (!RZ_Factions.IsValid(giver.m_iFaction))
			return false;

		RZ_ETier tier = dealer.GetGM().GetTier(playerId, giver.m_iFaction);
		return tier == RZ_ETier.WATCHED || tier == RZ_ETier.HUNTED;
	}

	//------------------------------------------------------------------------------------------------
	override bool WhyCard(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Epilogue epi = Week();
		if (!epi || epi.Phase() >= 3 || e.m_iHolding < 0)
			return false;

		e.m_sSubject = "epi.comein:" + e.m_iHolding.ToString();
		return true;
	}
}
