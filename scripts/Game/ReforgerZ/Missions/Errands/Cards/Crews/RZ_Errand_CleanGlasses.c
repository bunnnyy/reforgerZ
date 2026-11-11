//! Clean Glasses (docs/ERRANDS.md, margin card of The Book Has to Balance): the crews meet soon (the
//! major is waiting for its settling day or its last page), and it's not the set piece's evening. The
//! boss wants six unbroken glasses. "He wants it to look civilised. Then he'll read out who owes."
class RZ_Errand_CleanGlasses : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "clean_glasses";
		m_sTitle = "Clean Glasses";
		m_sFamily = "crews";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		m_bTimed = true;
		Offer("\"The crews sit down soon. The boss wants glasses. Six. Not cracked. He wants it to look civilised. Then he'll read out who owes.\"",
			"\"Six glasses, unbroken, from {place}. For the boss's table. Don't ask. Nobody asks.\"");
		Over("\"Never mind the glasses. He's drinking from the bottle. That's a bad sign for somebody.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Find six unbroken glasses";
		w.m_ePlace = RZ_EErrandPlace.EMPTY_VILLAGE;
		w.m_fSeconds = 30;
		w.m_fRadius = 80;
		w.m_sStep = "Search the houses at {place} for six glasses without a crack (30 s there), then bring them to {giver}.";
		w.m_eProp = RZ_EErrandProp.CRATE;
		w.m_iFavor = 5;
		w.Done("\"Six. Clean. He'll hold one up to the light before he reads the names. Old habit.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Questlines q = dealer.GetGM().GetQuestlines();
		if (!q)
			return false;

		RZ_QuestRun run = q.FindRunByKey("book");
		if (!run || (run.m_iStage != RZ_Major_Book.S_SETTLE_WAIT && run.m_iStage != RZ_Major_Book.S_LAST_WAIT))
			return false;

		// Never on a set piece's evening
		float hour = RZ_GameTime.GetHour();
		if (hour >= 16 && hour <= 21)
			return false;

		e.m_sSubject = "book.glasses";
		return true;
	}
}
