// Test-only (not part of the mod): the majors' phase of the runtime test (docs/WORKBENCH_SETUP.md,
// the runtime test). The majors are off through the mission and questline phases
// (so The Book and Await Relief run there as their built questlines, and nothing starts by itself),
// then this phase switches them on and:
// 1. starts each major and walks every act through its default (data), printing each act and the
//    ending; The Book's send-off also runs live once (its scene mission, ended as won);
// 2. the fallback checks: three empty days fire nothing; a squeeze sends a last call, then a
//    default after the grace;
// 3. what the endings leave: the story rules' cap, the epilogue lines by controller, the season
//    tab's lines and the stand's groups by stance.
// Lines: "RZT major <key> act <n> ...", "RZT major <key> ENDED: ending <n>",
// "RZT majors check: <name>: OK|FAILED (...)", "RZT majors done: ...".

modded class RZ_Majors
{
	//! Majors on or off for the test (the settings object is the server's own).
	void RZT_Set(bool on)
	{
		m_Settings.m_bEnabled = on;
	}

	//! A research history: `days` real days with play, research gained `perDay` a day up to now,
	//! and `hours` played on each.
	void RZT_History(int days, float perDay, float hours)
	{
		m_aResDays.Clear();
		m_aResAt.Clear();
		m_aDayKeys.Clear();
		m_aDayHours.Clear();
		int today = Today();
		float now = Progress();
		for (int i = days; i >= 1; i--)
		{
			m_aResDays.Insert(today - i);
			m_aResAt.Insert(now - perDay * i);
			m_aDayKeys.Insert(today - i);
			m_aDayHours.Insert(hours);
		}

		m_aDayKeys.Insert(today);
		m_aDayHours.Insert(0);
	}

	//! One clock tick for one run (as Tick does for every major).
	void RZT_TickRun(RZ_QuestRun run, array<int> ids)
	{
		TickRun(run, 10, ids);
	}

	void RZT_Default(RZ_QuestRun run)
	{
		RZ_MajorAct act = run.m_Line.ActOf(run.m_iStage);
		if (act)
			FireDefault(run, act, "runtime test", 1);
	}
}

modded class RZ_Director
{
	void RZT_StageMinutes(float minutes)
	{
		m_fStageMinutes = minutes;
	}
}

modded class RZ_Questlines
{
	//! The majors' lane doesn't start anything while the test hasn't reached the majors' phase.
	static bool s_bRZTMajors;

	override protected void TryStartMajors()
	{
		if (s_bRZTMajors)
			super.TryStartMajors();
	}

	void RZT_Forget(string key)
	{
		m_aDone.RemoveItem(key);
	}
}

class RZT_Majors
{
	protected static int s_iStep;
	protected static int s_iMajor;
	protected static int s_iSec;
	protected static int s_iAct;
	protected static int s_iOk;
	protected static int s_iFailed;
	protected static float s_fCureBefore = -1;
	protected static bool s_bLiveDone;
	protected static int s_iLiveAt;
	protected static ref array<string> s_aKeys = {"book", "orders", "every_name", "proof"};

	//------------------------------------------------------------------------------------------------
	//! Before the mission phase: the majors are off until their own phase.
	static void Off(RZ_GameModeComponent gm)
	{
		RZ_Majors m = RZ_Majors.Get();
		if (m)
			m.RZT_Set(false);

		Print("RZT forced: the majors are off until the majors' phase");
	}

	//------------------------------------------------------------------------------------------------
	//! Every second of the majors' phase. True when it's done.
	static bool Tick(RZ_GameModeComponent gm)
	{
		RZ_Majors majors = RZ_Majors.Get();
		RZ_Questlines q = gm.GetQuestlines();
		if (!majors || !q || !q.IsEnabled())
		{
			Print("RZT majors: questlines off, skipped");
			return true;
		}

		RZT_Prep.HoldHour();
		s_iSec++;
		if (s_iStep == 0)
		{
			q.RZT_EndAll();
			majors.RZT_Set(true);
			RZ_Questlines.s_bRZTMajors = false;
			s_fCureBefore = gm.GetCure().m_fProgress;
			// The book's send-off wants Collapse's second evening
			if (gm.GetDirector().GetStage() < RZ_EOutbreakStage.COLLAPSE)
				gm.GetDirector().SetStage(RZ_EOutbreakStage.COLLAPSE, false);

			gm.GetDirector().RZT_StageMinutes(RZ_SeasonClock.DayMinutes() + 10);
			Print("RZT majors: on; phase starts");
			s_iStep = 1;
			return false;
		}

		if (s_iStep == 1)
			return Acts(gm, q, majors);

		if (s_iStep == 2)
			return EmptyDays(gm, q, majors);

		if (s_iStep == 3)
			return Squeeze(gm, q, majors);

		if (s_iStep == 4)
			return Story(gm, majors);

		if (s_iStep == 5)
			return Rest(gm);

		q.RZT_EndAll();
		RZ_Questlines.s_bRZTMajors = true;
		gm.GetCure().RZT_SetProgress(s_fCureBefore);
		Print(string.Format("RZT majors done: %1 checks OK, %2 FAILED", s_iOk, s_iFailed));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Each major in turn: begin, the book's send-off live once, then every act's default.
	protected static bool Acts(RZ_GameModeComponent gm, RZ_Questlines q, RZ_Majors majors)
	{
		if (s_iMajor >= s_aKeys.Count())
		{
			s_iStep = 2;
			return false;
		}

		string key = s_aKeys[s_iMajor];
		RZ_QuestRun run = q.FindRunByKey(key);
		if (!run && s_iAct == 0)
		{
			q.RZT_Forget(key);
			if (!q.Begin(key))
			{
				Print(string.Format("RZT major %1 did not start (CanStart false)", key));
				s_iMajor++;
				return false;
			}

			Print(string.Format("RZT major %1 started", key));
			s_iAct = 1;
			s_iSec = 0;
			return false;
		}

		if (!run)
		{
			int ending = q.WorldFlag(key + ".ending", -1);
			Print(string.Format("RZT major %1 ENDED: ending %2", key, ending));
			Check(ending >= 0, "major " + key + " reaches an ending", "ending " + ending.ToString());
			s_iMajor++;
			s_iAct = 0;
			return false;
		}

		// The Book's send-off, live: its scene mission comes up and is ended as won (held)
		if (key == "book" && !s_bLiveDone && run.m_iStage == RZ_Major_Book.S_SEND)
		{
			if (run.m_iMissionId < 0)
			{
				run.m_fNextTry = 0;
				if (s_iSec < 90)
					return false;

				Print("RZT major book: the send-off's scene didn't come up in 90 s; its default instead");
				s_bLiveDone = true;
				s_iAct = 2;
				return false;
			}

			RZ_Mission m = gm.GetMissions().FindById(run.m_iMissionId);
			if (m && m.m_eState == RZ_EMissionState.ACTIVE)
			{
				RZT_Fake.MoveTo(m.m_vPos);
				run.SetOutcome(RZ_Major_Book.OUT_SENT);
				m.m_eState = RZ_EMissionState.SUCCEEDED;
				Print(string.Format("RZT major book: the send-off's scene '%1' ended as won", m.m_sTitle));
			}

			s_bLiveDone = true;
			s_iLiveAt = s_iSec;
			return false;
		}

		// The mission manager hands the won scene to the questline within a few seconds
		if (key == "book" && s_bLiveDone && s_iAct == 1 && run.m_iStage == RZ_Major_Book.S_SEND && s_iSec - s_iLiveAt < 30)
			return false;

		if (key == "book" && s_bLiveDone && s_iAct == 1)
		{
			Check(q.HasWorldFlag("book.act1"), "the send-off settles act 1 live", "book.act1 " + q.WorldFlag("book.act1", -1).ToString());
			s_iAct = 2;
		}

		RZ_MajorAct act = run.m_Line.ActOf(run.m_iStage);
		if (!act)
		{
			Print(string.Format("RZT major %1 stage %2 belongs to no act: ended", key, run.m_iStage));
			q.RZT_End(key);
			return false;
		}

		if (s_iSec > 400)
		{
			Print(string.Format("RZT major %1 STUCK at act %2", key, act.m_iId));
			q.RZT_End(key);
			return false;
		}

		Print(string.Format("RZT major %1 act %2 (%3), stage %4: its default", key, act.m_iId, act.m_sName, run.m_iStage));
		majors.RZT_Default(run);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Three real days with nobody online: the questlines' tick returns at once, so no clock moves
	//! and no default fires.
	protected static bool EmptyDays(RZ_GameModeComponent gm, RZ_Questlines q, RZ_Majors majors)
	{
		q.RZT_Forget("book");
		if (!q.FindRunByKey("book") && !q.Begin("book"))
		{
			Check(false, "three empty days fire nothing", "the book didn't start");
			s_iStep = 3;
			return false;
		}

		RZ_QuestRun run = q.FindRunByKey("book");
		int stage = run.m_iStage;
		int defaults = run.Flag("mc.defaults");
		float clock = q.GetClock();
		array<int> none = {};
		array<IEntity> nobody = {};
		// 3 days x 24 hours x 360 ticks of 10 s
		for (int i = 0; i < 3 * 24 * 360; i++)
		{
			q.Tick(10, none, nobody);
		}

		bool same = run.m_iStage == stage && run.Flag("mc.defaults") == defaults && q.GetClock() == clock;
		Check(same, "three empty days fire nothing", string.Format("stage %1 -> %2, defaults %3 -> %4", stage, run.m_iStage, defaults, run.Flag("mc.defaults")));
		q.RZT_End("book");
		s_iStep = 3;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Proof's Day One with research just short of 25% and a fast pace: squeezed. The first tick
	//! sends the last call; once a session's grace has passed, the default fires.
	protected static bool Squeeze(RZ_GameModeComponent gm, RZ_Questlines q, RZ_Majors majors)
	{
		q.RZT_Forget("proof");
		if (!q.FindRunByKey("proof") && !q.Begin("proof"))
		{
			Check(false, "a squeeze sends a last call, then a default", "proof didn't start");
			s_iStep = 4;
			return false;
		}

		RZ_QuestRun run = q.FindRunByKey("proof");
		gm.GetCure().RZT_SetProgress(24.9);
		majors.RZT_History(3, 6, 8);
		run.m_mFlags.Remove("mc.touch");
		run.m_fReadyAt = q.GetClock();
		array<int> ids = {};
		array<IEntity> ents = {};
		RZ_Util.GetAlivePlayers(ids, ents);
		majors.RZT_TickRun(run, ids);
		bool called = run.Flag("mc.call") == 1;
		Check(called, "a squeeze sends a last call", "mc.call " + run.Flag("mc.call").ToString() + ", squeezed " + run.Flag("mc.squeezed").ToString());

		// The grace passes (a session of offered time), and nothing holds the act
		int stage = run.m_iStage;
		run.SetFlag("mc.callOffered", run.Flag("mc.offered") - 7300);
		run.m_mFlags.Remove("mc.touch");
		q.Withdraw(run);
		majors.RZT_TickRun(run, ids);
		bool moved = run.m_bDone || run.m_iStage != stage;
		Check(called && moved, "then a default after the grace", string.Format("stage %1 -> %2, defaults %3", stage, run.m_iStage, run.Flag("mc.defaults")));
		q.RZT_End("proof");
		s_iStep = 4;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! 4. What the endings leave: next season's story rules (strength capped at 2 a faction), the
	//! epilogue lines by controller, the season tab's lines, the stand's groups by stance.
	protected static bool Story(RZ_GameModeComponent gm, RZ_Majors majors)
	{
		array<string> keys = {"book:10", "book:13", "orders:12", "proof:11", "proof:14"};
		RZ_StoryRules rules = RZ_StoryRules.Build(keys);
		RZ_SeasonRuleSet ruleSet = rules.RuleSet();
		bool capped = ruleSet && ruleSet.m_iBastionStrength == 2 && ruleSet.m_iRedHandStrength == 2;
		string detail = "no rule set";
		if (ruleSet)
			detail = string.Format("Bastion %1, Red Hand %2", ruleSet.m_iBastionStrength, ruleSet.m_iRedHandStrength);

		Check(capped, "story rules cap strength at 2 a faction", detail);
		Check(rules.m_iStolenDoses == 1 && rules.m_aLegacy.Contains("legacy.terms"), "Two Tins' dose and Under Seal's legacy flag", string.Format("doses %1", rules.m_iStolenDoses));
		RZ_StoryRules.Build(null);
		Check(RZ_StoryRules.Get().RuleSet() == null, "no story rules without a record", "");

		bool variants = !RZ_Majors.Variant("orders", 10, true).IsEmpty() && !RZ_Majors.Variant("book", 10, false).IsEmpty() && RZ_Majors.Variant("book", 99, true).IsEmpty();
		Check(variants, "epilogue lines by controller", "");
		array<string> lines = {};
		majors.SeasonLines(lines);
		Check(!lines.IsEmpty(), "the season tab's lines", string.Format("%1 lines", lines.Count()));
		foreach (string line : lines)
		{
			if (!line.IsEmpty())
				Print("RZT majors: season tab: " + line);
		}

		int stance = RZ_MajorHelp.World(gm, "orders.stance");
		RZ_MajorHelp.SetWorld(gm, "orders.stance", 3);
		bool road = RZ_Majors.SendsStandGroup(gm, RZ_EFaction.MILITARY) && RZ_Majors.OnLabRoad(gm, RZ_EFaction.MILITARY);
		RZ_MajorHelp.SetWorld(gm, "orders.stance", 4);
		bool none = !RZ_Majors.SendsStandGroup(gm, RZ_EFaction.MILITARY);
		RZ_MajorHelp.SetWorld(gm, "orders.stance", stance);
		Check(road && none, "the stand's groups by stance", "");
		s_iStep = 5;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The engine's rest (Director/RZ_MajorRest.c) and the story rules' last pieces.
	protected static bool Rest(RZ_GameModeComponent gm)
	{
		Check(!RZ_MajorRest.Muted(RZ_EFaction.BANDITS), "no faction muted by default", "");
		RZ_MajorRest.Mute(RZ_EFaction.BANDITS);
		bool muted = RZ_MajorRest.Muted(RZ_EFaction.BANDITS);
		RZ_WorldFacts.Get().SetFlag("mute." + RZ_EFaction.BANDITS.ToString(), 0);
		Check(muted && !RZ_MajorRest.Muted(RZ_EFaction.BANDITS), "a one-night mute", "");
		RZ_MajorRest.SecureLine(RZ_EFaction.MILITARY, "RZT secure line", 1);
		array<string> options = {};
		RZ_MajorRest.SecureLines(RZ_EFaction.MILITARY, options);
		Check(options.Contains("RZT secure line"), "a secure line for Radio Operators", string.Format("%1 lines", options.Count()));
		Check(!RZ_StoryRules.CountedRoad(gm, vector.Zero), "Counted's road post off without its rule", "");
		array<string> keys = {"crew:rzt|Bailiff"};
		RZ_StoryRules.Build(keys);
		Check(RZ_StoryRules.Get().m_aCrews.Contains("rzt|Bailiff"), "the book's crew-name greeting kept", "");
		RZ_StoryRules.Build(null);
		s_iStep = 6;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Check(bool ok, string name, string detail)
	{
		string verdict = "FAILED";
		if (ok)
		{
			verdict = "OK";
			s_iOk++;
		}
		else
		{
			s_iFailed++;
		}

		Print(string.Format("RZT majors check: %1: %2 (%3)", name, verdict, detail));
	}
}
