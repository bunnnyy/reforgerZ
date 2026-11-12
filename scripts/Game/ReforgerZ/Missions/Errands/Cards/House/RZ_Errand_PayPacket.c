//! Pay Packet (docs/ERRANDS.md, backgrounds: the Unemployed). A Haven clerk has a job going: hand
//! the payroll of tins out at the house, six people, a while each. 4 Favor; the Unemployed's Fame
//! bonus applies within the ceiling.
class RZ_Errand_PayPacket : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "pay_packet";
		m_sTitle = "Pay Packet";
		m_sFamily = "background";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "clerk";
		Offer("I've a job going, if you want one. Everyone in this house works, and the six on this week's rota get a tin each for it: watch, water, burying. Hand them out and get a signature for each. It's payroll. Sort of.",
			"Six names on the rota, six tins, and a signature for each. Someone has to do it who won't eat the seventh. I'm told you were out of work before all this. Well, now you're not.");
		Over("Never mind. The rota's paid. Someone else signed for it.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Hand out the payroll";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 180;
		w.m_fRadius = 20;
		w.m_sStep = "Hand a tin to each of the six on the rota at {here} and get a signature for each (3 minutes).";
		w.m_iFavor = 4;
		w.m_iFame = 1;
		w.Done("First pay packet since before the gates closed, is it? Don't spend it all at once. There's nothing to spend it on.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return RZ_QuestApproach.HasBackground(dealer.GetGM(), playerId, "unemployed");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_HouseFacts house = RZ_WorldFacts.Get().House(e.m_iHouse);
		if (!house.m_bValid || house.m_iPeople < 6 || house.m_bShort)
			return false;

		e.m_sSubject = RZ_WorldFacts.Safehouse(e.m_iHouse) + ".rota";
		return true;
	}
}
