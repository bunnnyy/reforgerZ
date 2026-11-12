//! How a job ended for a player (the notebook's Journal).
enum RZ_EJobResult
{
	DONE,
	FAILED,
	ABANDONED
}

//------------------------------------------------------------------------------------------------
//! Past jobs (docs/UI.md, "Notebook: Journal"): every job a player had that ended, done, failed or
//! abandoned: missions and contracts they were hired for or took part in, questlines they were part
//! of, errands they held (the openings' steps among them). One entry each: the job's icon, name,
//! giver, place, result, the season day and one line of how it ended. Kept on the player's profile
//! (saved with it), the newest MAX; sent to the owner as its own payload (RZ_UIFeed.PAYLOAD_HISTORY)
//! when the Journal opens. World events aren't jobs and are never here. Server only.
class RZ_JobHistory
{
	static const int MAX = 40;
	static const int LINE_MAX = 160;
	static const int ID_BASE = 500000;	// the rows' ids: -(base + n), n = 0 the newest

	//------------------------------------------------------------------------------------------------
	//! Older saves have none of the arrays, or (after a crash mid-write) uneven ones: made and trimmed
	//! to the shortest, so every index is one whole entry.
	static void Ensure(RZ_PlayerProfile p)
	{
		if (!p)
			return;

		if (!p.m_aPastTitle)
			p.m_aPastTitle = new array<string>();
		if (!p.m_aPastBy)
			p.m_aPastBy = new array<string>();
		if (!p.m_aPastWhere)
			p.m_aPastWhere = new array<string>();
		if (!p.m_aPastLine)
			p.m_aPastLine = new array<string>();
		if (!p.m_aPastKind)
			p.m_aPastKind = new array<int>();
		if (!p.m_aPastGiver)
			p.m_aPastGiver = new array<int>();
		if (!p.m_aPastResult)
			p.m_aPastResult = new array<int>();
		if (!p.m_aPastDay)
			p.m_aPastDay = new array<int>();
		if (!p.m_aPastSeason)
			p.m_aPastSeason = new array<int>();

		int n = p.m_aPastTitle.Count();
		n = Math.Min(n, p.m_aPastBy.Count());
		n = Math.Min(n, p.m_aPastWhere.Count());
		n = Math.Min(n, p.m_aPastLine.Count());
		n = Math.Min(n, p.m_aPastKind.Count());
		n = Math.Min(n, p.m_aPastGiver.Count());
		n = Math.Min(n, p.m_aPastResult.Count());
		n = Math.Min(n, p.m_aPastDay.Count());
		n = Math.Min(n, p.m_aPastSeason.Count());
		p.m_aPastTitle.Resize(n);
		p.m_aPastBy.Resize(n);
		p.m_aPastWhere.Resize(n);
		p.m_aPastLine.Resize(n);
		p.m_aPastKind.Resize(n);
		p.m_aPastGiver.Resize(n);
		p.m_aPastResult.Resize(n);
		p.m_aPastDay.Resize(n);
		p.m_aPastSeason.Resize(n);
	}

	//------------------------------------------------------------------------------------------------
	//! One ended job on this profile; the oldest goes past MAX.
	static void Add(RZ_PlayerProfile p, int kind, int giver, string title, string by, string where, int result, string line)
	{
		if (!p || title.IsEmpty())
			return;

		Ensure(p);
		string shortLine = line.Trim();
		if (shortLine.Length() > LINE_MAX)
			shortLine = shortLine.Substring(0, LINE_MAX - 3).Trim() + "...";

		int season = 0;
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm && gm.GetSeasons())
			season = gm.GetSeasons().GetSeason();

		p.m_aPastTitle.Insert(title);
		p.m_aPastBy.Insert(by);
		p.m_aPastWhere.Insert(where);
		p.m_aPastLine.Insert(RZ_Util.Capitalize(shortLine));
		p.m_aPastKind.Insert(kind);
		p.m_aPastGiver.Insert(giver);
		p.m_aPastResult.Insert(result);
		p.m_aPastDay.Insert(RZ_SeasonClock.Day());
		p.m_aPastSeason.Insert(season);
		while (p.m_aPastTitle.Count() > MAX)
		{
			p.m_aPastTitle.RemoveOrdered(0);
			p.m_aPastBy.RemoveOrdered(0);
			p.m_aPastWhere.RemoveOrdered(0);
			p.m_aPastLine.RemoveOrdered(0);
			p.m_aPastKind.RemoveOrdered(0);
			p.m_aPastGiver.RemoveOrdered(0);
			p.m_aPastResult.RemoveOrdered(0);
			p.m_aPastDay.RemoveOrdered(0);
			p.m_aPastSeason.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! For a player id (online, or a profile kept for them).
	static void AddFor(RZ_GameModeComponent gm, int playerId, int kind, int giver, string title, string by, string where, int result, string line)
	{
		if (!gm || playerId <= 0)
			return;

		Add(gm.FindProfile(playerId), kind, giver, title, by, where, result, line);
	}

	//------------------------------------------------------------------------------------------------
	static string ResultWord(int result)
	{
		switch (result)
		{
			case RZ_EJobResult.DONE: return "DONE";
			case RZ_EJobResult.FAILED: return "FAILED";
		}

		return "ABANDONED";
	}

	//------------------------------------------------------------------------------------------------
	//! When, for the row: "Day 4" this season, "Season 2, day 9" for an older one.
	static string WhenText(int day, int season, int now)
	{
		if (season == now || season <= 0)
			return string.Format("Day %1", day);

		return string.Format("Season %1, day %2", season, day);
	}

	// ================================================================ From the systems
	//------------------------------------------------------------------------------------------------
	//! A mission ended (RZ_MissionManager.Finish, before rewards). `before`: its participants before
	//! anyone who stepped out was dropped. Its people: a contract's hired players who took part, an
	//! event's only if it was theirs (a puzzle or arc taken by its clue), and anyone a leader or a
	//! parley offered it to alone. A plain world event is news, not a job: nobody's.
	static void MissionEnded(RZ_GameModeComponent gm, RZ_Mission m, bool success, array<int> before)
	{
		if (!gm || !m)
			return;

		int giver = -1;
		if (m.m_bHasGiver)
			giver = m.m_eGiver;

		string by = "";
		if (m.m_iLeaderId >= 0 && gm.GetLeaders())
		{
			RZ_Leader l = gm.GetLeaders().Get(m.m_iLeaderId);
			if (l)
				by = l.m_sName;
		}

		if (by.IsEmpty() && giver >= 0)
			by = RZ_Factions.GetName(giver);

		int kind = RZ_UIFeed.MissionKind(m);
		string where = m.GetLocationName(gm);
		string doneLine;
		if (m.m_bHasGiver)
			doneLine = m.DeedPhrase(gm) + ".";
		else
			doneLine = m.GetSuccessText(gm);

		array<int> seen = {};
		foreach (int p : before)
		{
			if (seen.Contains(p))
				continue;

			seen.Insert(p);
			bool mine = m.IsOfferedTo(gm, p) || (m.m_bContract && m.IsHired(gm, p));
			if (!mine)
				continue;

			int result = RZ_EJobResult.FAILED;
			string line = m.GetFailText(gm);
			if (m.m_bWithdrawn)
			{
				result = RZ_EJobResult.ABANDONED;
				line = m.m_sWithdrawText;
			}
			else if (!m.m_aParticipants.Contains(p))
			{
				result = RZ_EJobResult.ABANDONED;
				line = "You walked away from it.";
			}
			else if (success)
			{
				result = RZ_EJobResult.DONE;
				line = doneLine;
				if (m.m_bContract && !m.IsEligible(gm, p))
					line = string.Format("Done, but %1 didn't pay you for it.", RZ_Factions.GetNameInText(m.m_eGiver));
			}

			AddFor(gm, p, kind, giver, m.m_sTitle, by, where, result, line);
		}

		// Offered to them alone (a leader's own job, a parley's), and they never went
		if (!m.m_bInPersonOnly && m.m_sReservedFor.IsEmpty())
			return;

		foreach (int offered : m.m_aOfferedTo)
		{
			if (seen.Contains(offered))
				continue;

			seen.Insert(offered);
			int res = RZ_EJobResult.FAILED;
			string why = m.GetFailText(gm);
			if (m.m_bWithdrawn)
			{
				res = RZ_EJobResult.ABANDONED;
				why = m.m_sWithdrawText;
			}
			else if (success)
			{
				res = RZ_EJobResult.ABANDONED;
				why = "Someone else saw to it.";
			}

			AddFor(gm, offered, kind, giver, m.m_sTitle, by, where, res, why);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A questline ended (RZ_Questlines.EndRun): for everyone who took part, online or not. An ending
	//! is done; one that went quiet is abandoned.
	static void QuestEnded(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (!gm || !run || !run.m_Line)
			return;

		int result = RZ_EJobResult.ABANDONED;
		if (run.m_iEnding >= 0)
			result = RZ_EJobResult.DONE;

		string line = run.m_sEndText;
		if (line.IsEmpty() && result == RZ_EJobResult.DONE)
			line = "It came to an end.";
		else if (line.IsEmpty())
			line = "It went quiet.";

		int giver = run.m_Line.m_iFaction;
		string by = "";
		if (RZ_Factions.IsValid(giver))
			by = RZ_Factions.GetName(giver);

		array<string> idents = {};
		run.GetIdents(idents);
		foreach (string ident : idents)
		{
			Add(gm.FindProfileByIdentity(ident), RZ_EMissionKindUI.QUESTLINE, giver, run.m_Line.m_sTitle, by, run.m_sPlaceName, result, line);
		}
	}

	// ================================================================ The payload
	//------------------------------------------------------------------------------------------------
	//! The Journal's past jobs, newest first, as journal rows (M records, RZ_UIFeed.BuildJournal's
	//! fields): the result word as the kind label and the season day as "now", the result last.
	static string Build(RZ_GameModeComponent gm, int playerId)
	{
		RZ_PlayerProfile p = gm.FindProfile(playerId);
		if (!p)
			return "";

		Ensure(p);
		int now = 0;
		if (gm.GetSeasons())
			now = gm.GetSeasons().GetSeason();

		string s = "";
		int n = 0;
		for (int i = p.m_aPastTitle.Count() - 1; i >= 0; i--)
		{
			RZ_UILine ln = new RZ_UILine("M");
			ln.AddInt(-(ID_BASE + n)).AddInt(p.m_aPastKind[i]).AddInt(p.m_aPastGiver[i]).Add(p.m_aPastTitle[i]).Add(p.m_aPastBy[i]).Add(p.m_aPastWhere[i]).Add("");
			ln.AddInt(-1).Add("").AddInt(0).AddInt(0).Add(p.m_aPastLine[i]).Add("0");
			ln.Add(ResultWord(p.m_aPastResult[i])).Add(WhenText(p.m_aPastDay[i], p.m_aPastSeason[i], now)).AddInt(p.m_aPastResult[i]);
			s += ln.End();
			n++;
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! A row id from Build (client and server).
	static bool IsRow(int id)
	{
		return id <= -ID_BASE && id > -(ID_BASE + 100000);
	}
}
