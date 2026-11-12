//! Take a Name to the Board (the epilogue deck): a resident knows a name from the season's dead that
//! nobody has written up. Carry it to the Names board on the north road; the wall lists it as home,
//! with who brought it (Bring Them Home's list). Nothing to carry back, nothing grey about it.
class RZ_Errand_NameToTheBoard : RZ_EpiCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "epi_name_board";
		m_sTitle = "Take a Name to the Board";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("{dead} died {lies}, and nobody's written it up. There's a board on the north road now, by the queue, with the names of everyone who didn't make it to the gates. People read it on the way out. Would you put {dead} on it?",
			"Somebody should put {dead} on the Names board on the north road, so the people going out read it. I can't walk that far any more.");
		Over("Somebody already wrote {dead} up on the board. Good.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Write {dead} on the Names board";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 25;
		w.m_sStep = "Go to the Names board on the north road and write {dead} up (10 s there), then tell {giver}.";
		w.Work(RZ_EWork.WRITE, "Write the name up", 8);
		w.m_iFavor = 3;
		w.m_iFame = 1;
		w.Done("On the board. Good. Everyone in the queue walks past it every day until the gates open.");
	}

	//------------------------------------------------------------------------------------------------
	override bool WhyCard(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Epilogue epi = Week();
		if (!epi)
			return false;

		vector board = epi.GetSpots().NamesPos();
		if (board == vector.Zero)
			return false;

		RZ_EpiName n = LyingNear(-2, e.m_vGiver, 4000);
		if (!n)
			return false;

		e.m_vSubject = board;
		e.m_sSubject = "epi.name:" + n.name;
		e.m_sValue = n.name;
		e.SetText("dead", n.name);
		e.SetText("lies", n.lies);
		e.SetText("subject_place", "the Names board on the north road");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Still(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_EpiName n = NameCalled(e.m_sValue);
		return n && n.state == 0;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		NameHome(NameCalled(e.m_sValue), "the Names board", playerId);
	}
}
