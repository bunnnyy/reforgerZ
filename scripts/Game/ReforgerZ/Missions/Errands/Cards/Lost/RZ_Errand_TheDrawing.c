//! The Drawing (docs/ERRANDS.md, the Long Table; horror): the Long Table was broken up and the boy got
//! out, and you're Known with the Haven. The house's clerk has the drawing he left: a long table, one
//! more chair than people, a row of boots in too many sizes. Nobody says more. Take it to the clerk's
//! drawer, keep it, or burn it on the stove. Pays nothing. The epilogue reads which.
class RZ_Errand_TheDrawing : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "drawing";
		m_sTitle = "The Drawing";
		m_sFamily = "lost";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "clerk";
		Offer("\"The boy left this. I don't want it in the house. I don't want to throw it away either. You decide.\"",
			"\"He drew it before he left. A table. Count the chairs. Then count the people. Then tell me what to do with it.\"");
		Over("\"Never mind. Somebody took it. I didn't ask who.\"");

		RZ_ErrandWay w = Way("give");
		w.m_sLabel = "Put it in the drawer";
		w.Done("\"In the drawer. With the other things we don't talk about.\"");

		w = Way("do");
		w.m_sLabel = "Keep it";
		w.Done("\"Keep it, then. Don't show it to anyone. Don't look at it at night.\"");

		w = Way("do");
		w.m_sLabel = "Burn it on the stove";
		w.Done("It takes a long time to catch. Nobody at the stove says anything.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return RZ_Reputation.IsAtLeast(dealer.GetGM().GetTier(playerId, RZ_EFaction.SURVIVORS), RZ_ETier.KNOWN);
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Questlines q = dealer.GetGM().GetQuestlines();
		if (!q || q.WorldFlag("table.broken") <= 0 || q.WorldFlag("table.boy_lost") > 0)
			return false;

		e.m_sSubject = "table.drawing";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_WorldFacts facts = dealer.GetGM().GetFacts();
		if (facts)
			facts.SetFlag("errand.drawing", w.m_iIndex + 1);

		array<int> who = {playerId};
		string value = "drawer";
		if (w.m_iIndex == 1)
			value = "kept";
		else if (w.m_iIndex == 2)
			value = "burnt";

		RZ_WorldFacts.Get().Memory().Record("errand.drawing", "table", value, RZ_WorldMemory.SOURCE_ERRAND, false, who);
	}
}
