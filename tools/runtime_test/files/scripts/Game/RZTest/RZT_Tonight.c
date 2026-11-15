// Test-only (not part of the mod): Tonight (UI/RZ_Tonight.c; docs/UI.md, "Tonight"). Once, on the
// fake player, after the openings' checks, with everything put back after:
// - The card: nothing while an opening runs; otherwise "Tonight:" and a line each.
// - The lines: at most one of each kind, each within reach and true; a town you've never seen is
//   one (the map's fog cleared for the check); the journal's rows are the lines, of kind Tonight.
// - Tracking: a Tonight row tracks its own line (or the job it points at), its one objective has a
//   place, and reaching the place ends the line.
// - Off (UI > Tonight): no card, no rows.
// Lines: "RZT tonight OK|FAIL <what>", "RZT tonight line <kind>: <line>" and "RZT tonight: N checks, M failed".

class RZT_Tonight
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

		Print(string.Format("RZT tonight %1 %2", verdict, what));
	}

	//------------------------------------------------------------------------------------------------
	static void Tick(RZ_GameModeComponent gm)
	{
		if (s_bDone || !RZT_Fake.s_Ent)
			return;

		s_bDone = true;
		int id = RZT_Fake.ID;
		IEntity me = RZT_Fake.s_Ent;
		RZ_PlayerProfile p = gm.GetProfile(id);
		RZ_UIFeed feed = gm.GetUIFeed();
		RZ_Tonight tonight;
		if (feed)
			tonight = feed.GetTonight();

		Check(tonight != null, "the UI feed has Tonight");
		RZ_UISettings settings = RZ_UISettings.Get();
		if (!tonight || !settings)
		{
			Done();
			return;
		}

		// What's put back after
		vector home = me.GetOrigin();
		int trackedBefore = p.m_iTrackedId;
		string trackedTitle = p.m_sTrackedTitle;
		bool trackOff = p.m_bTrackOff;
		array<int> fogBefore = {};
		fogBefore.Copy(p.m_aFog);
		bool wasOn = settings.m_bTonight;
		settings.m_bTonight = true;

		// 1. The card and the opening
		string card = tonight.CardFor(id, me);
		if (RZ_Openings.IsRunningFor(id))
			Check(card.IsEmpty(), "no Tonight while an opening runs");
		else
			Check(card.IsEmpty() || card.IndexOf("Tonight:") == 0, "the card starts with Tonight: (" + card + ")");

		// 2. A town never seen: the fog cleared for the check, then the lines picked
		RZ_MapFog fog = gm.GetMapFog();
		if (fog && fog.IsOn())
			fog.SetAll(id, false);

		tonight.TestRepick(id, me);
		int n = tonight.TestCount(id);
		Check(n > 0 && n <= 3, string.Format("one to three lines (%1)", n));
		array<int> kinds = {};
		bool allNear = true;
		bool dup = false;
		RZ_TonightLine own;
		for (int i = 0; i < n; i++)
		{
			RZ_TonightLine l = tonight.TestLine(id, i);
			Print(string.Format("RZT tonight line %1: %2 (%3)", RZ_Tonight.KindWord(l.m_iKind), l.m_sLine, l.m_sSource));
			if (kinds.Contains(l.m_iKind))
				dup = true;

			kinds.Insert(l.m_iKind);
			if (l.m_vAt != vector.Zero && RZ_Util.DistXZ(l.m_vAt, me.GetOrigin()) > settings.m_fTonightReach + 50)
				allNear = false;

			if (l.m_iJob == 0 && !own)
				own = l;
		}

		Check(!dup, "at most one line of each kind");
		Check(allNear, "every line within reach");
		if (fog && fog.IsOn())
			Check(kinds.Contains(RZ_ETonightKind.FIND), "a town you've never seen is something to find");

		// 3. The journal's rows: the lines, of kind Tonight
		string rows = tonight.Rows(id, me, me.GetOrigin());
		int count = 0;
		int at = rows.IndexOf("M|");
		while (at >= 0)
		{
			count++;
			int next = rows.IndexOfFrom(at + 2, "M|");
			at = next;
		}

		// While an opening runs the rows wait for it, as the card does
		int wantRows = tonight.TestCount(id);
		if (RZ_Openings.IsRunningFor(id))
			wantRows = 0;

		Check(count == wantRows, string.Format("a row per line (%1 rows, %2 wanted)", count, wantRows));
		string kindField = string.Format("|%1|", RZ_EMissionKindUI.TONIGHT);
		Check(count == 0 || rows.Contains(kindField), "the rows are of kind Tonight");

		// 4. Tracking a line of its own: tracked, one objective with a place, ended where it points
		if (own)
		{
			RZ_QuestTracker tracker = feed.GetTracker();
			tracker.Toggle(id, own.OwnRow());
			Check(p.m_iTrackedId == own.OwnRow(), string.Format("picking a Tonight row tracks it (%1, wanted %2)", p.m_iTrackedId, own.OwnRow()));
			RZ_ObjList list = new RZ_ObjList();
			tracker.Objectives(id, own.OwnRow(), me.GetOrigin(), list);
			RZ_ObjPlace now = list.Now();
			Check(!list.First(RZ_EObjState.CURRENT).IsEmpty() && now != null, "its one objective has a place (" + list.First(RZ_EObjState.CURRENT) + ")");
			if (now)
			{
				RZT_Fake.MoveTo(now.m_vPos);
				Check(tonight.CheckReached(id, own.OwnRow(), me), "reaching its place ends the line");
				array<ref RZ_QuestJob> jobs = {};
				tracker.Jobs(id, me, jobs);
				bool still = false;
				foreach (RZ_QuestJob j : jobs)
				{
					if (j.m_iId == own.OwnRow())
						still = true;
				}

				Check(!still, "a reached line is no longer a job to follow");
			}
		}
		else
		{
			Check(false, "a line of its own to track (none: only real jobs near)");
		}

		// 5. A line that is a real job tracks the job
		for (int k = 0; k < tonight.TestCount(id); k++)
		{
			RZ_TonightLine jobLine = tonight.TestLine(id, k);
			if (jobLine && jobLine.m_iJob != 0)
				Check(tonight.TrackId(id, jobLine.OwnRow()) == jobLine.m_iJob, "a Tonight row that is a job tracks the job");
		}

		// 6. Off: no card, no rows
		settings.m_bTonight = false;
		Check(tonight.CardFor(id, me).IsEmpty(), "off: no card");
		Check(tonight.Rows(id, me, me.GetOrigin()).IsEmpty(), "off: no rows");

		// Put back
		settings.m_bTonight = wasOn;
		p.m_aFog.Copy(fogBefore);
		p.m_iTrackedId = trackedBefore;
		p.m_sTrackedTitle = trackedTitle;
		p.m_bTrackOff = trackOff;
		RZT_Fake.MoveTo(home);
		tonight.Forget(id);
		Done();
	}

	//------------------------------------------------------------------------------------------------
	protected static void Done()
	{
		Print(string.Format("RZT tonight: %1 checks, %2 failed", s_iChecks, s_iFails));
	}
}
