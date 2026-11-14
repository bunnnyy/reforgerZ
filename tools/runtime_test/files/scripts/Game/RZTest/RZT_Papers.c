// Test-only (not part of the mod): the papers in the case's lid (Guidance/RZ_CasePapers.c,
// docs/DESIGN.md "The papers in the lid"), on the fake player, after the case's check:
// - The character gains all three papers, once; the case's read names them.
// - Whoever takes the case remarks on each paper.
// - At another faction's people each paper is offered as a job (carry it to its owner); taken, it's
//   tracked and held.
// - At its owner's people the hand-over reply ends the job, and the character is a participant of
//   its major (when it runs; else it waits and joins when it starts).
// - Given to a rival instead: the rival's line, the page in the open (majors.pages_open) once.
// Lines: "RZT papers OK|FAIL <what>" and "RZT papers: N checks, M failed".

class RZT_Papers
{
	static bool s_bDone;
	static int s_iChecks;
	static int s_iFails;

	//------------------------------------------------------------------------------------------------
	static void Check(bool ok, string what)
	{
		s_iChecks++;
		if (!ok)
			s_iFails++;

		string verdict = "OK";
		if (!ok)
			verdict = "FAIL";

		Print(string.Format("RZT papers %1 %2", verdict, what));
	}

	//------------------------------------------------------------------------------------------------
	static RZ_TalkSession Session(int faction, string name)
	{
		RZ_TalkSession s = new RZ_TalkSession();
		s.m_iPlayer = RZT_Fake.ID;
		s.m_iFaction = faction;
		s.m_sName = name;
		return s;
	}

	//------------------------------------------------------------------------------------------------
	static int ReplyFor(notnull array<ref RZ_TalkReply> replies, int paper, int kind)
	{
		int id = RZ_CasePapers.REPLY_BASE + paper * 3 + kind;
		foreach (RZ_TalkReply r : replies)
		{
			if (r.m_iId == id)
				return id;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	static void Said(RZ_TalkSession s)
	{
		foreach (string line : s.m_aNew)
		{
			Print("RZT papers: they said: " + line);
		}

		s.m_aNew.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Once, after the case's check. True when done.
	static bool Tick(RZ_GameModeComponent gm)
	{
		if (s_bDone)
			return true;

		s_bDone = true;
		RZ_OpeningCase c = RZ_OpeningCase.Get();
		IEntity me = RZT_Fake.s_Ent;
		if (!c || !me || !gm.GetErrands() || !gm.GetErrands().IsOn())
		{
			Print("RZT papers: skipped (the case, the errands or the tester missing)");
			return true;
		}

		RZ_CasePapers papers = c.GetPapers();
		RZ_PlayerProfile profile = gm.GetProfile(RZT_Fake.ID);
		profile.m_iPapersLife = -1;
		papers.Gain(RZT_Fake.ID);
		bool all = true;
		for (int p = 0; p < RZ_CasePapers.COUNT; p++)
		{
			all = all && RZ_CasePapers.Carries(profile, p);
		}

		Check(all, "the character has the three papers from the lid");
		profile.m_aPapers[0] = RZ_CasePapers.NONE;
		papers.Gain(RZT_Fake.ID);
		Check(!RZ_CasePapers.Carries(profile, 0), "they're gained once a character, not again");
		profile.m_aPapers[0] = RZ_CasePapers.CARRIED;
		Check(RZ_CasePapers.ReadText().Contains("three papers"), "the case's read names them: " + RZ_CasePapers.ReadText());

		array<string> said = {};
		papers.Remark(RZT_Fake.ID, RZ_EFaction.INSTITUTE, said);
		Check(said.Count() == 3, "whoever takes the case has a word about each (" + said.Count().ToString() + ")");

		// At Halcyon's people: each paper is offered as a job
		RZ_TalkSession hal = Session(RZ_EFaction.INSTITUTE, "Test Halcyon");
		array<ref RZ_TalkReply> replies = {};
		papers.AddReplies(hal, RZ_EFaction.INSTITUTE, replies);
		int orders = ReplyFor(replies, RZ_CasePapers.ORDERS, RZ_CasePapers.R_JOB);
		int page = ReplyFor(replies, RZ_CasePapers.PAGE, RZ_CasePapers.R_JOB);
		Check(orders >= 0 || !papers.HasOwnerPlace(RZ_CasePapers.ORDERS), "at Halcyon's people the orders are offered as a job");
		Check(page >= 0 || !papers.HasOwnerPlace(RZ_CasePapers.PAGE), "and the page");

		// The orders: take the job, then hand them to Bastion
		if (orders >= 0)
		{
			papers.Pick(hal, RZ_EFaction.INSTITUTE, orders, me);
			Said(hal);
			Check(papers.JobOf(RZT_Fake.ID, RZ_CasePapers.ORDERS) >= 0, "the orders' job is held and tracked");
		}

		RZ_TalkSession bas = Session(RZ_EFaction.MILITARY, "Test Bastion");
		array<ref RZ_TalkReply> basReplies = {};
		papers.AddReplies(bas, RZ_EFaction.MILITARY, basReplies);
		int hand = ReplyFor(basReplies, RZ_CasePapers.ORDERS, RZ_CasePapers.R_HAND);
		Check(hand >= 0, "at Bastion's people the orders are handed over by a reply");
		papers.Pick(bas, RZ_EFaction.MILITARY, hand, me);
		Said(bas);
		int st = RZ_CasePapers.StateOf(profile, RZ_CasePapers.ORDERS);
		Check(st == RZ_CasePapers.OWNER || st == RZ_CasePapers.OWNER_WAIT, "the orders are Bastion's (state " + st.ToString() + ")");
		Check(papers.JobOf(RZT_Fake.ID, RZ_CasePapers.ORDERS) < 0, "the job ended with the hand-over");
		RZ_QuestRun run;
		if (gm.GetQuestlines())
			run = gm.GetQuestlines().FindRunByKey("orders");

		if (run)
			Check(RZ_OpeningCase.TookPart(gm, run, RZT_Fake.ID), "the character takes part in Await Relief");
		else
			Check(st == RZ_CasePapers.OWNER_WAIT, "Await Relief isn't running: the hand-over waits for it");

		// The page: the job, then given to the Haven instead
		if (page >= 0)
		{
			papers.Pick(hal, RZ_EFaction.INSTITUTE, page, me);
			Said(hal);
		}

		int open = RZ_MajorHelp.World(gm, "majors.pages_open");
		RZ_TalkSession hav = Session(RZ_EFaction.SURVIVORS, "Test Haven");
		array<ref RZ_TalkReply> havReplies = {};
		papers.AddReplies(hav, RZ_EFaction.SURVIVORS, havReplies);
		int rival = ReplyFor(havReplies, RZ_CasePapers.PAGE, RZ_CasePapers.R_RIVAL);
		if (page >= 0)
		{
			Check(rival >= 0, "with the page's job held, the Haven's people can be given it instead");
			papers.Pick(hav, RZ_EFaction.SURVIVORS, rival, me);
			Said(hav);
			Check(RZ_CasePapers.StateOf(profile, RZ_CasePapers.PAGE) == RZ_CasePapers.GIVEN + RZ_EFaction.SURVIVORS, "the page went to the Haven");
			bool once = RZ_MajorHelp.World(gm, "papers.book.open") == 1;
			Check(once && RZ_MajorHelp.World(gm, "majors.pages_open") >= open, "the page is in the open, once (pages_open " + open.ToString() + " -> " + RZ_MajorHelp.World(gm, "majors.pages_open").ToString() + ")");
		}

		// The list: at the Haven's own people, handed over
		havReplies.Clear();
		papers.AddReplies(hav, RZ_EFaction.SURVIVORS, havReplies);
		int list = ReplyFor(havReplies, RZ_CasePapers.LIST, RZ_CasePapers.R_HAND);
		Check(list >= 0, "the list is handed over at the Haven's own people");
		papers.Pick(hav, RZ_EFaction.SURVIVORS, list, me);
		Said(hav);
		st = RZ_CasePapers.StateOf(profile, RZ_CasePapers.LIST);
		Check(st == RZ_CasePapers.OWNER || st == RZ_CasePapers.OWNER_WAIT, "the list is the Haven's (state " + st.ToString() + ")");

		havReplies.Clear();
		papers.AddReplies(hav, RZ_EFaction.SURVIVORS, havReplies);
		if (page >= 0)
			Check(havReplies.IsEmpty(), "with every paper handed, nothing more is offered");
		Print(string.Format("RZT papers: %1 checks, %2 failed", s_iChecks, s_iFails));
		return true;
	}
}
