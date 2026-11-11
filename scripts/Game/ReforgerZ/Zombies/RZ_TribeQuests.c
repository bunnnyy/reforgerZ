//! The packs' questline hooks (docs/INFECTED.md, "Questline hooks"). Small, read-only helpers the
//! questlines call, and the seeds the packs leave for questlines not written yet (world flags):
//! - B1 Await Relief: the Day 3 runner's road crosses the column's range, and his body lies at an
//!   old kill site there when one is near (RunnerBody);
//! - R1 The Book: while the page is being read, a rumour of a name on it that isn't a person: the
//!   crew in the woods the Red Hand feeds (the moat) (Daily);
//! - HC3 Authors: the tagged leaders numbered in the paper's appendix (Appendix);
//! - seeds: packs.home, packs.contact, packs.moat and packs.index (Daily).
//! Player text never says alpha, tribe or crawler. Server only.

//------------------------------------------------------------------------------------------------
class RZ_TribeQuests
{
	static const string SEED_HOME = "packs.home";			// a named one was brought home (count)
	static const string SEED_CONTACT = "packs.contact";		// a pack founded at a lost Bastion post (1)
	static const string SEED_MOAT = "packs.moat";			// the Red Hand feeds a pack (1 while it does)
	static const string SEED_INDEX = "packs.index";			// Halcyon has tagged a leader (count tagged)
	static const string BOOK_TOLD = "packs.book_told";		// R1's rumour was told for the run that started then (unix)

	protected static const float RUNNER_REACH = 2500;		// m: the column this close to the runner's last town
	protected static const float KILL_SITE_REACH = 800;		// m: an old kill site this close to his road

	//------------------------------------------------------------------------------------------------
	protected static RZ_Tribes Tribes(RZ_GameModeComponent gm)
	{
		if (!gm || !gm.GetTribes() || !gm.GetTribes().IsEnabled())
			return null;

		return gm.GetTribes();
	}

	//------------------------------------------------------------------------------------------------
	//! B1's runner: where his body lies when his road (from the second town to the last) crosses the
	//! column's range: at an old kill site within 800 m of the road inside the range, else on the
	//! road where it passes nearest the range's middle. False: no column near, or his road misses it
	//! (the questline keeps its own spot).
	static bool RunnerBody(RZ_GameModeComponent gm, vector from, vector to, out vector spot)
	{
		RZ_Tribes tribes = Tribes(gm);
		if (!tribes)
			return false;

		RZ_Tribe column;
		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			if (t.m_bColumn && !t.IsRover())
			{
				column = t;
				break;
			}
		}

		if (!column || RZ_Util.DistXZ(column.RangeCentre(), to) > RUNNER_REACH)
			return false;

		vector c = column.RangeCentre();
		vector road = NearestOnSegment(from, to, c);
		if (RZ_Util.DistXZ(road, c) > column.m_fRadius)
			return false;

		RZ_Crawlers crawlers = gm.GetSpawner().GetCrawlers();
		if (crawlers)
		{
			foreach (RZ_KillSite site : crawlers.GetKillSites())
			{
				if (RZ_Util.DistXZ(site.m_vPos, c) > column.m_fRadius)
					continue;

				if (RZ_Util.DistXZ(NearestOnSegment(from, to, site.m_vPos), site.m_vPos) > KILL_SITE_REACH)
					continue;

				spot = RZ_Util.GroundPos(site.m_vPos);
				return !RZ_Util.IsWater(spot);
			}
		}

		spot = RZ_Util.GroundPos(road);
		return !RZ_Util.IsWater(spot);
	}

	//------------------------------------------------------------------------------------------------
	protected static vector NearestOnSegment(vector a, vector b, vector p)
	{
		vector ab = b - a;
		ab[1] = 0;
		float len2 = ab[0] * ab[0] + ab[2] * ab[2];
		if (len2 < 1)
			return a;

		vector ap = p - a;
		float k = Math.Clamp((ap[0] * ab[0] + ap[2] * ab[2]) / len2, 0, 1);
		return a + ab * k;
	}

	//------------------------------------------------------------------------------------------------
	//! HC3's paper: its appendix numbers the leaders Halcyon has tagged (" Appendix C: index I-1, the
	//! quarry by Morton; I-2, the woods above Levie."), "" when none is. In tag order, four at most.
	static string Appendix(RZ_GameModeComponent gm)
	{
		RZ_Tribes tribes = Tribes(gm);
		if (!tribes)
			return "";

		array<RZ_Tribe> tagged = {};
		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			if (t.m_iTagDay < 0)
				continue;

			int at = 0;
			while (at < tagged.Count() && tagged[at].m_iTagDay <= t.m_iTagDay)
			{
				at++;
			}

			tagged.InsertAt(t, at);
		}

		if (tagged.IsEmpty())
			return "";

		string text = " Appendix C lists the index by number:";
		int shown = Math.MinInt(tagged.Count(), 4);
		for (int i = 0; i < shown; i++)
		{
			string sep = ";";
			if (i == shown - 1)
				sep = ".";

			text += string.Format(" I-%1, %2%3", i + 1, tagged[i].m_sPlace, sep);
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! Once a season day (RZ_TribePeople's day): R1's rumour, and the seeds' world flags.
	static void Daily(RZ_GameModeComponent gm)
	{
		RZ_Tribes tribes = Tribes(gm);
		RZ_Questlines q = gm.GetQuestlines();
		if (!tribes || !q)
			return;

		RZ_Tribe moat;
		int tagged = 0;
		bool contact = false;
		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			if (t.m_bMoat)
				moat = t;

			if (t.m_iTagDay >= 0)
				tagged++;

			if (t.m_fMilitaryShare >= 0.5)
				contact = true;
		}

		// Seeds for the questlines not written yet (docs/QUESTLINES.md, world flags)
		int moatFlag = 0;
		if (moat)
			moatFlag = 1;

		q.SetWorldFlag(SEED_MOAT, moatFlag);
		q.SetWorldFlag(SEED_INDEX, tagged);
		if (contact)
			q.SetWorldFlag(SEED_CONTACT, 1);

		// R1: the page has a line on it that isn't a person
		RZ_QuestRun book = q.FindRunByKey("book");
		if (!moat || !book || book.m_iStage < RZ_Quest_TheBook.S_READ || q.WorldFlag(BOOK_TOLD) == book.m_iStartUnix)
			return;

		q.SetWorldFlag(BOOK_TOLD, book.m_iStartUnix);
		RZ_WorldFacts.Get().AddRumour(string.Format("They say there's a line on the Red Hand's missing page that isn't a person at all: \"the crew in the woods\", by %1. What the Red Hand pays them in, nobody writes down.", moat.m_sPlace), 24);
		RZ_Log.Info("Tribes: R1's rumour of the crew in the woods (the moat at " + moat.m_sPlace + ").");
	}

	//------------------------------------------------------------------------------------------------
	//! A named one brought home (Bring Them Home): the seed counts it.
	static void BroughtHome(RZ_GameModeComponent gm)
	{
		RZ_Questlines q;
		if (gm)
			q = gm.GetQuestlines();

		if (q)
			q.SetWorldFlag(SEED_HOME, q.WorldFlag(SEED_HOME) + 1);
	}
}
