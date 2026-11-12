//! Off the Report (docs/ERRANDS.md, Bastion): a private, for someone Known or better with Bastion
//! who carries some Bastion Infamy (they know you've bent a rule). He wants his name off a duty
//! report. The report's true. Take it off (a lie, found by events), tell him to own it, or (a Soldier)
//! tell him how to answer for it.
class RZ_Errand_OffTheReport : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "off_the_report";
		m_sTitle = "Off the Report";
		m_sFamily = "bastion";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		m_sRoles = "sentry,mechanic";
		Offer("There's a duty report on the sergeant's desk with my name on it. Asleep on watch. It's true. I'd still rather it wasn't there. You've bent a rule or two. Bend this one.",
			"One line on a report. Mine. It's accurate, which is the problem. Could it go missing?");
		Over("The report's gone up to the stronghold. Out of reach. Out of everyone's.");

		RZ_ErrandWay w = Way("lie");
		w.m_sLabel = "Lose the report for him";
		w.m_iFavor = 4;
		w.Done("Gone? Gone. I owe you. Don't tell anyone I owe you.");

		w = Way("talk");
		w.m_sLabel = "Tell him to own it";
		w.Done("\"Own it. Right. Easy for you.\" He goes to see the sergeant anyway.");

		w = Way("talk");
		w.m_sLabel = "Tell him how to answer for it";
		w.m_sBackground = "soldier";
		w.m_iFavor = 3;
		w.m_iFame = 1;
		w.Done("\"Stand up straight, say it first, ask for the extra watch yourself. That works?\" It works. It always worked.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		return RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.MILITARY), RZ_ETier.KNOWN) && gm.GetProfile(playerId).GetInfamy(RZ_EFaction.MILITARY) > 0;
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		e.m_sSubject = "holding:" + e.m_iHolding.ToString() + ".report";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex != 0)
			return;

		RZ_GameModeComponent gm = dealer.GetGM();
		gm.GetFacts().Memory().Lies().Tell(gm, playerId, "errand.report", e.m_sSubject, RZ_EFaction.MILITARY, e.m_sGiver, e.m_vGiver, RZ_ELieTest.RUMOUR, 3,
			"The sergeant found the missing report under the radio. He knows it didn't walk there.");
	}
}
