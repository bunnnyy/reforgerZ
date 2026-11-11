//! I'd Rather Be Wrong (docs/ERRANDS.md, the house): at a house big enough for a medic, the one who
//! does the medicine there is sure the lab's trial dose is poison. Someone with a Nurse's training
//! talks it through with the numbers, a Lab Technician reads them the labels; anyone can agree. Talk
//! only: a little Favor and the giver's regard, never the house's vote on anything.
class RZ_Errand_RatherBeWrong : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "rather_be_wrong";
		m_sTitle = "I'd Rather Be Wrong";
		m_sFamily = "house";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("\"The lab's trial dose. You know about these things. Tell me it isn't poison, because I've been telling people it is.\"");
		Over("\"Never mind. The medic from the lab came by. We shouted at each other. It helped.\"");

		RZ_ErrandWay w = Way("talk");
		w.m_sLabel = "Go through the numbers";
		w.m_sBackground = "doctor,nurse";
		w.m_iFavor = 3;
		w.Done("\"Eight in ten. Eight in ten is a number I can hate properly. Thank you.\"");

		w = Way("talk");
		w.m_sLabel = "Read them the labels";
		w.m_sBackground = "labtech";
		w.m_iFavor = 3;
		w.Done("\"Batch numbers and a cold chain. It's real, then. I'm not saying you're right. I'm saying I'd rather be wrong my way.\"");

		w = Way("talk");
		w.m_sLabel = "Agree with them";
		w.Done("\"I knew it. Well. I didn't know it. Nobody knows anything. Thank you for saying so.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_HouseFacts house = RZ_WorldFacts.Get().House(e.m_iHouse);
		if (!house.m_bValid || !house.m_bHasMedic || !gm.HasLab() || RZ_WorldFacts.Get().IsCured())
			return false;

		return RZ_QuestApproach.HasBackground(gm, e.m_iAsker, "doctor,nurse,labtech");
	}
}
