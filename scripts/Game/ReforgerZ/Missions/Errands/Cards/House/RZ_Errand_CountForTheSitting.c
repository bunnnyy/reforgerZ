//! Count for the Sitting (docs/ERRANDS.md, margin card of Every Name): the council is summoned (the
//! major waits for a sitting), it's before supper, and the house's head stays home on watch. The clerk
//! can't leave either. Carry the house's count to the next house before supper. Read on the way, it's
//! hungrier than it says: carry it as written, or say so too. It changes no vote.
class RZ_Errand_CountForTheSitting : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "count_sitting";
		m_sTitle = "Count for the Sitting";
		m_sFamily = "house";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "clerk";
		m_bTimed = true;
		Offer("The council of the houses sits soon, and every house sends its count: people, mouths, food. Our head stays home on watch, and I can't leave the book. Take our count to {subject_place} before supper. It's sealed. Ish.",
			"Our count for the council's sitting, how many we are and what we eat. It has to be at {subject_place} before supper. Don't read it. Everybody reads it.");
		Over("Never mind. Somebody carried it. I hope they didn't read it.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Carry it as written";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 30;
		w.m_fBefore = 18.5;
		w.m_sStep = "Carry the count to the clerk at {subject_place} before half past six (5 s at the door), then come back to {giver}.";
		w.m_iFavor = 3;
		w.Done("Delivered, as written. Thank you.");

		w = Way("talk");
		w.m_sLabel = "Carry it, and say what it doesn't";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 30;
		w.m_fBefore = 18.5;
		w.m_sStep = "Carry the count to the clerk at {subject_place} before half past six, and tell them the house is hungrier than the count says (5 s at the door).";
		w.m_iFavor = 3;
		w.Done("You told them we're hungrier than it says. I'd have told them myself, if I'd been allowed to leave the book.");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Questlines q = gm.GetQuestlines();
		if (!q)
			return false;

		RZ_QuestRun run = q.FindRunByKey("every_name");
		if (!run || (run.m_iStage != RZ_Major_EveryName.S_SUMMONS && run.m_iStage != RZ_Major_EveryName.S_SITTING2_WAIT))
			return false;

		// Before supper, never on the sitting's evening
		if (RZ_GameTime.GetHour() >= 17)
			return false;

		int other = HouseNear(gm, e.m_vGiver, 1500, e.m_iHouse, 0);
		if (other < 0)
			return false;

		RZ_Safehouse h = gm.GetSafehouses().Get(other);
		Place(e, h.m_vPos, h.m_sName);
		e.m_sSubject = RZ_WorldFacts.Safehouse(e.m_iHouse) + ".count";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex != 1)
			return;

		array<int> who = {playerId};
		RZ_WorldFacts.Get().Memory().Record("errand.count_sitting", e.m_sSubject, "told", RZ_WorldMemory.SOURCE_ERRAND, true, who);
	}
}
