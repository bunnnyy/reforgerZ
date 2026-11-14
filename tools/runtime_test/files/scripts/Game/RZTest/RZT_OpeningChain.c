// Test-only (not part of the mod): the whole opening chain by guidance level (docs/DESIGN.md,
// "Openings", the chain by level), once, on the fake player in a town's middle, a Nurse:
// - Level 1 (learning): the door asks who you were; the favour, its hand-over by reply, then the
//   line (or its short form) and the runner; Ferda comes for a visit in the street.
// - Level 2 (familiar, with a habit to carry): the door asks nothing; the favour and its hand-over;
//   the line's short form (straight to the runner); Ferda as one scene where you are.
// - Level 3 (veteran, with a habit): the door asks nothing; no favour; the runner first; Ferda's note.
// Each level ends with the case delivered to Halcyon by the hand-over reply (the case went to
// Halcyon; Proof counts the tester when Proof runs).
// Lines: "RZT chain OK|FAIL <what>" and "RZT chain: N checks, M failed".

class RZT_OpeningChain
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

		Print(string.Format("RZT chain %1 %2", verdict, what));
	}

	//------------------------------------------------------------------------------------------------
	//! Once, from RZT_Tick. Always true.
	static bool Tick(RZ_GameModeComponent gm)
	{
		if (s_bDone)
			return true;

		s_bDone = true;
		IEntity me = RZT_Fake.s_Ent;
		int id = RZT_Fake.ID;
		RZ_OpeningCase c = RZ_OpeningCase.Get();
		if (!me || !c || !gm.GetGuidance() || !gm.GetGuidance().GetOpenings() || !gm.GetErrands() || !gm.GetErrands().IsOn())
		{
			Print("RZT chain: skipped (no fake player, no openings, no case or errands off)");
			return true;
		}

		if (RZ_OpeningCase.CasePrefab(gm).IsEmpty())
		{
			Print("RZT chain: skipped (Guidance > Cold case is empty)");
			return true;
		}

		RZ_PlayerProfile profile = gm.GetProfile(id);
		int guideLives = profile.m_iGuideLives;
		int refresh = profile.m_iGuideRefreshLeft;
		string bgBefore = profile.m_sBackgroundId;
		int habitBefore = profile.m_iHabit;
		int habitLifeBefore = profile.m_iHabitLife;
		int caseBefore = profile.m_iCaseState;
		vector from = me.GetOrigin();
		RZT_Fake.MoveTo(RZT_Favour.Door(gm, me));

		for (int level = 1; level <= 3; level++)
		{
			Level(gm, id, me, profile, c, level);
		}

		RZT_Fake.MoveTo(from);
		profile.m_iGuideLives = guideLives;
		profile.m_iGuideRefreshLeft = refresh;
		profile.m_sBackgroundId = bgBefore;
		profile.m_iHabit = habitBefore;
		profile.m_iHabitLife = habitLifeBefore;
		profile.m_iCaseState = caseBefore;
		RZ_ColdCaseComponent.s_TestCarried = null;
		RZ_ColdCaseComponent.s_TestHolder = null;
		Print(string.Format("RZT chain: %1 checks, %2 failed", s_iChecks, s_iFails));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The profile's counted lives set so the guidance level is `level`. False if it can't be.
	protected static bool SetLevel(RZ_GameModeComponent gm, RZ_PlayerProfile profile, int level)
	{
		profile.m_iGuideRefreshLeft = 0;
		for (int lives = 0; lives < 60; lives++)
		{
			profile.m_iGuideLives = lives;
			if (gm.GetGuidance().GetLevel(profile) == level)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! One fresh character at this level, a Nurse, through the whole chain to the case at Halcyon.
	protected static void Level(RZ_GameModeComponent gm, int id, IEntity me, RZ_PlayerProfile profile, RZ_OpeningCase c, int level)
	{
		RZ_Openings openings = gm.GetGuidance().GetOpenings();
		RZ_Errands errands = gm.GetErrands();
		string tag = string.Format("level %1", level);
		if (!SetLevel(gm, profile, level))
		{
			Check(false, tag + ": the profile reads that level");
			return;
		}

		profile.m_iCaseState = 0;
		profile.m_iHabit = 0;
		profile.m_iHabitLife = -1;
		// A familiar or veteran player has a last character's habit to carry
		if (level >= 2)
			RZ_Habits.Set(profile, RZ_EPlayerHabit.LIGHT);

		RZ_OpeningFixer fixer = RZ_OpeningFixer.Get();
		if (fixer)
			fixer.Forget(id);

		array<string> lines = {};
		int oid = RZT_FavourStart(gm, id, me, profile, "nurse", lines);
		if (oid < 0)
		{
			Check(false, tag + ": the opening began");
			return;
		}

		bool asked = false;
		string question = RZ_OpeningFavours.Question("nurse");
		foreach (string line : lines)
		{
			if (line == question)
				asked = true;
		}

		if (level == 1)
		{
			Check(asked, tag + ": the door asks who you were");
			if (openings.HabitReplies(id, oid))
			{
				array<string> said = {};
				openings.TakeHabit(id, oid, RZ_EPlayerHabit.STEADY, said);
			}
		}
		else
		{
			Check(!asked && RZ_Habits.Of(profile) == RZ_EPlayerHabit.LIGHT, tag + ": the door asks nothing, the last habit carries");
		}

		int stage = openings.StageOf(id);
		if (level == 3)
		{
			Check(stage == RZ_EOpeningStage.CASE, string.Format("%1: no favour, the runner first (stage %2)", tag, stage));
		}
		else if (stage == RZ_EOpeningStage.FAVOUR)
		{
			RZ_Errand fetch = openings.StepErrand(id);
			Check(fetch && errands.FinishNow(fetch, me), tag + ": the favour's fetch is done there");
			openings.NextNow(id, me);
			if (openings.StageOf(id) == RZ_EOpeningStage.GIVE)
			{
				array<string> thanks = {};
				openings.HandOver(id, oid, me, thanks);
				openings.NextNow(id, me);
			}

			stage = openings.StageOf(id);
			if (level == 2)
				Check(stage == RZ_EOpeningStage.CASE, string.Format("%1: the line's short form, straight to the runner (stage %2)", tag, stage));
			else
				Check(stage == RZ_EOpeningStage.LINE || stage == RZ_EOpeningStage.CASE, string.Format("%1: the line, or its short form, then the runner (stage %2)", tag, stage));
		}
		else
		{
			Print(string.Format("RZT chain: %1: no place for the favour here (stage %2), on to the runner", tag, stage));
		}

		// The line, if it runs, is checked by RZT_Line: here it's moved past to the runner
		if (openings.StageOf(id) == RZ_EOpeningStage.LINE)
		{
			openings.SkipLineForTest(id);
			openings.NextNow(id, me);
		}

		Check(openings.StageOf(id) == RZ_EOpeningStage.CASE, string.Format("%1: the runner's step (stage %2)", tag, openings.StageOf(id)));
		RZ_Errand caseStep = openings.StepErrand(id);
		if (!caseStep || openings.StageOf(id) != RZ_EOpeningStage.CASE)
		{
			Done(gm, id);
			return;
		}

		// The case in the tester's hands (held through the test stand-in where the pack refuses it)
		IEntity item = RZT_Case.NewCase(gm, c, me);
		if (!RZ_OpeningCase.Carries(me) && item)
			RZT_Case.Hold(me, item, false);

		Check(RZ_OpeningCase.Carries(me), tag + ": the case is in the tester's hands");
		Check(errands.FinishNow(caseStep, me), tag + ": the runner's step is done with the case in hand");
		// The step's end hands the lead to the player's own character; the fake tester isn't a player's
		// (GetPlayerControlledEntity is null for him), so it's handed to him here, as RZT_Case does
		if (c.LeadOf(id) < 0 && !GetGame().GetPlayerManager().GetPlayerControlledEntity(id))
			c.Begin(id, me, "Test Contact", "Test Contact, a neighbour", me.GetOrigin());

		Check(c.LeadOf(id) >= 0, tag + ": the lead to Halcyon's station is handed");

		if (fixer)
		{
			RZ_FixerVisit v = fixer.Of(id);
			if (level == 3)
				Check(!v, tag + ": no visit from Ferda, his note instead");
			else if (level == 2)
				Check(v && v.m_bShort, tag + ": Ferda as one scene where you are");
			else
				Check(v && !v.m_bShort, tag + ": Ferda's visit to the street");

			fixer.Forget(id);
		}

		RZ_TalkSession s = new RZ_TalkSession();
		s.m_iPlayer = id;
		s.m_iFaction = RZ_EFaction.INSTITUTE;
		s.m_sName = "Test Halcyon";
		array<ref RZ_TalkReply> replies = {};
		c.AddReplies(s, me, replies);
		Check(!replies.IsEmpty() && replies[0].m_iId == RZ_OpeningCase.REPLY_HALCYON, tag + ": at Halcyon the hand-over reply is first");
		c.Pick(s, RZ_OpeningCase.REPLY_HALCYON, me);
		Check(profile.m_iCaseState == RZ_OpeningCase.STATE_HALCYON && !RZ_OpeningCase.Carries(me), tag + ": the case is delivered to Halcyon");
		RZ_Questlines quests = gm.GetQuestlines();
		RZ_QuestRun proof;
		if (quests)
			proof = quests.FindRunByKey("proof");

		if (proof)
			Check(RZ_OpeningCase.TookPart(gm, proof, id), tag + ": Proof counts the tester as taking part");
		else
			Print("RZT chain: " + tag + ": Proof isn't running, its part not checked");

		RZ_ColdCaseComponent.s_TestCarried = null;
		RZ_ColdCaseComponent.s_TestHolder = null;
		Done(gm, id);
	}

	//------------------------------------------------------------------------------------------------
	//! An opening for this background, started as if you talked to him (RZT_Favour's way).
	protected static int RZT_FavourStart(RZ_GameModeComponent gm, int id, IEntity me, RZ_PlayerProfile profile, string bgId, notnull array<string> lines)
	{
		RZ_Openings openings = gm.GetGuidance().GetOpenings();
		RZ_Background bg = gm.GetBackground(bgId);
		if (!bg)
			return -1;

		profile.m_sBackgroundId = bgId;
		RZ_OpeningPlace place = new RZ_OpeningPlace();
		place.m_vPos = me.GetOrigin();
		place.m_iTown = gm.FindNearestSettlement(place.m_vPos);
		openings.Begin(id, me, profile, bg, place);
		int oid = openings.OpeningIdOf(id);
		if (oid < 0)
			return -1;

		openings.TalkLines(id, oid, false, lines, me);
		return oid;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Done(RZ_GameModeComponent gm, int id)
	{
		RZ_Openings openings = gm.GetGuidance().GetOpenings();
		openings.Close(id, "the chain check is done");
		RZ_OpeningFixer fixer = RZ_OpeningFixer.Get();
		if (fixer)
			fixer.Forget(id);
	}
}
