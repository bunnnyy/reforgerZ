// Test-only (not part of the mod): the opening's door and favour (docs/DESIGN.md, "Openings", beats
// 1 and 2), once, early, on the fake player, for every background:
// - The door: the first talk asks who you were (the background's line), the habit answers show, and
//   one picked is this character's habit; each habit's effect (a hold's speed, crouched visibility,
//   one persuade a talk easier) and the Standing page's line.
// - The favour: its fetch is handed at a place its words are true of (or there's no place here, said);
//   done, the hand-over waits for its reply (standing by him never counts), and the reply finishes it.
// - Your background's way: the tagged reply shows for every background but the Unemployed and the
//   Drifter; picked, a reply way settles the favour there and then, a way of its own hands its step,
//   done where it's done; either way no hand-over follows.
// Lines: "RZT favour OK|FAIL <what>" and "RZT favour: N checks, M failed".

class RZT_Favour
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

		Print(string.Format("RZT favour %1 %2", verdict, what));
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
		if (!me || !gm.GetGuidance() || !gm.GetGuidance().GetOpenings() || !gm.GetErrands() || !gm.GetErrands().IsOn())
		{
			Print("RZT favour: skipped (no fake player, no openings or errands off)");
			return true;
		}

		RZ_PlayerProfile profile = gm.GetProfile(id);
		int guideLives = profile.m_iGuideLives;
		string bgBefore = profile.m_sBackgroundId;
		int habitBefore = profile.m_iHabit;
		int habitLifeBefore = profile.m_iHabitLife;
		profile.m_iGuideLives = 0;

		// The openings stand where you come out of the door: the tester stands in a town's middle for
		// the check (the contact is spawned by them), and goes back after
		vector from = me.GetOrigin();
		RZT_Fake.MoveTo(Door(gm, me));

		Habits(gm, id, profile);

		array<string> bgs = {"nurse", "doctor", "mechanic", "hunter", "police", "excon", "labtech", "radioham", "soldier", "raider", "elder", "unemployed", "drifter"};
		int placed = 0;
		foreach (int i, string bgId : bgs)
		{
			if (Normal(gm, id, me, profile, bgId, i))
				placed++;

			if (RZ_OpeningFavours.HasWay(bgId))
				Way(gm, id, me, profile, bgId);
		}

		RZ_Settlement townAt = Town(gm, me);
		string townName = "no town";
		if (townAt)
			townName = townAt.m_sName;

		Print(string.Format("RZT favour: a place for the favour found for %1 of %2 backgrounds, from the middle of %3", placed, bgs.Count(), townName));
		RZT_Fake.MoveTo(from);
		profile.m_iGuideLives = guideLives;
		profile.m_sBackgroundId = bgBefore;
		profile.m_iHabit = habitBefore;
		profile.m_iHabitLife = habitLifeBefore;
		Print(string.Format("RZT favour: %1 checks, %2 failed", s_iChecks, s_iFails));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Each habit's effect on this character.
	protected static void Habits(RZ_GameModeComponent gm, int id, RZ_PlayerProfile profile)
	{
		RZ_Habits.Set(profile, RZ_EPlayerHabit.STEADY);
		Check(RZ_Habits.HoldSpeed(id) > 1.05, string.Format("Steady hands: a hold is shorter (speed %1)", RZ_Habits.HoldSpeed(id)));
		Check(!RZ_Habits.StandingLine(profile).IsEmpty(), "the Standing page shows the habit: " + RZ_Habits.StandingLine(profile));
		RZ_Habits.Set(profile, RZ_EPlayerHabit.LIGHT);
		Check(RZ_Habits.CrouchMult(id) < 0.95, string.Format("Light feet: quieter crouched (x%1)", RZ_Habits.CrouchMult(id)));
		Check(RZ_Habits.HoldSpeed(id) == 1, "Light feet doesn't shorten holds");
		RZ_Habits.Set(profile, RZ_EPlayerHabit.LIAR);
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		if (trees)
		{
			RZ_DCtx ctx = new RZ_DCtx();
			ctx.m_GM = gm;
			ctx.m_iPlayer = id;
			RZ_DChoice a = new RZ_DChoice();
			a.m_sId = "rzt.a";
			a.m_sCheck = "persuade";
			a.m_iDifficulty = 3;
			RZ_DChoice b = new RZ_DChoice();
			b.m_sId = "rzt.b";
			b.m_sCheck = "persuade";
			b.m_iDifficulty = 3;
			int da = trees.Difficulty(ctx, a);
			int db = trees.Difficulty(ctx, b);
			int daAgain = trees.Difficulty(ctx, a);
			Check(da == 2 && daAgain == 2 && db == 3, string.Format("A good liar: the first persuade of a talk is one easier, only that one (%1, %2, %3)", da, db, daAgain));
		}

		// An earlier character's habit isn't this one's
		profile.m_iHabitLife = profile.m_iHabitLife - 1;
		Check(RZ_Habits.Of(profile) == RZ_EPlayerHabit.NONE, "an earlier character's habit isn't this one's");
	}

	//------------------------------------------------------------------------------------------------
	//! An opening for this background, started as if you talked to him.
	protected static int Start(RZ_GameModeComponent gm, int id, IEntity me, RZ_PlayerProfile profile, string bgId, notnull array<string> lines)
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
	//! The door, the fetch and the hand-over by reply. True: a place was found for the favour here.
	protected static bool Normal(RZ_GameModeComponent gm, int id, IEntity me, RZ_PlayerProfile profile, string bgId, int index)
	{
		RZ_Openings openings = gm.GetGuidance().GetOpenings();
		RZ_Errands errands = gm.GetErrands();
		array<string> lines = {};
		int oid = Start(gm, id, me, profile, bgId, lines);
		if (oid < 0)
		{
			Check(false, bgId + ": the opening began");
			return false;
		}

		bool asked = false;
		string question = RZ_OpeningFavours.Question(bgId);
		foreach (string line : lines)
		{
			if (line == question)
				asked = true;
		}

		Check(asked, bgId + ": the door asks who you were: " + question);
		Check(openings.HabitReplies(id, oid), bgId + ": the habit answers show");
		int habit = RZ_EPlayerHabit.STEADY + index % 3;
		array<string> said = {};
		openings.TakeHabit(id, oid, habit, said);
		Check(RZ_Habits.Of(profile) == habit && !said.IsEmpty(), bgId + ": the answer is this character's habit, " + RZ_Habits.Name(habit));
		Check(!openings.HabitReplies(id, oid), bgId + ": the habit answers go once one is picked");

		if (openings.StageOf(id) != RZ_EOpeningStage.FAVOUR)
		{
			Print(string.Format("RZT favour: %1: no place for the favour here (stage %2), on to the runner", bgId, openings.StageOf(id)));
			Done(gm, id);
			return false;
		}

		RZ_Errand e = openings.StepErrand(id);
		Check(e && e.m_Def.m_sKey.IndexOf(RZ_OpeningFavours.KEY + ".") == 0, bgId + ": the favour's fetch is handed (" + KeyOf(e) + ")");
		if (!e)
		{
			Done(gm, id);
			return true;
		}

		float far = RZ_Util.DistXZ(e.m_vPlace, me.GetOrigin());
		Check(far > 20 && far < 600, string.Format("%1: its place is a walk away (%2 m), %3", bgId, RZ_Util.RoundToInt(far), e.Fill("{place}")));
		Check(errands.FinishNow(e, me), bgId + ": the fetch is done there");
		openings.NextNow(id, me);
		Check(openings.StageOf(id) == RZ_EOpeningStage.GIVE, string.Format("%1: the hand-over is next (stage %2)", bgId, openings.StageOf(id)));
		RZ_Errand give = openings.StepErrand(id);
		if (give && openings.StageOf(id) == RZ_EOpeningStage.GIVE)
		{
			Check(!give.m_Def.StepCounts(errands, give, me), bgId + ": standing by him doesn't hand it over");
			string label;
			string need;
			Check(openings.HandOverReply(id, oid, me, label, need) && need.IsEmpty(), bgId + ": the hand-over reply shows: " + label);
			array<string> thanks = {};
			openings.HandOver(id, oid, me, thanks);
			Check(give.m_eState == RZ_EErrandState.DONE && !thanks.IsEmpty(), bgId + ": the reply hands it over there and then");
			openings.NextNow(id, me);
			int after = openings.StageOf(id);
			Check(after != RZ_EOpeningStage.GIVE && after != RZ_EOpeningStage.FAVOUR, string.Format("%1: then on to the runner (stage %2)", bgId, after));
		}

		Done(gm, id);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Your background's way through the favour.
	protected static void Way(RZ_GameModeComponent gm, int id, IEntity me, RZ_PlayerProfile profile, string bgId)
	{
		RZ_Openings openings = gm.GetGuidance().GetOpenings();
		RZ_Errands errands = gm.GetErrands();
		array<string> lines = {};
		int oid = Start(gm, id, me, profile, bgId, lines);
		if (oid < 0 || openings.StageOf(id) != RZ_EOpeningStage.FAVOUR)
		{
			Done(gm, id);
			return;
		}

		if (openings.FavourOf(id) != RZ_OpeningFavours.For(bgId))
		{
			Print(string.Format("RZT favour: %1: a plain favour stood in for its own here, its way not tried", bgId));
			Done(gm, id);
			return;
		}

		if (bgId == "nurse" || bgId == "doctor")
			RZT_Openings.GiveDressing();

		string label;
		string tag;
		string need;
		bool shows = openings.WayReply(id, oid, me, label, tag, need);
		Check(shows && tag.Contains(RZ_DialogueTrees.BackgroundName(bgId)), string.Format("%1: the way's reply shows, %2 %3", bgId, tag, label));
		if (!shows || !need.IsEmpty())
		{
			if (shows)
				Print(string.Format("RZT favour: %1: the way needs %2 here, not tried", bgId, need));

			Done(gm, id);
			return;
		}

		RZ_Errand fetch = openings.StepErrand(id);
		array<string> said = {};
		openings.TakeWay(id, oid, me, said);
		if (RZ_OpeningFavours.WayIsReply(bgId))
		{
			Check(!said.IsEmpty() && fetch && fetch.m_eState == RZ_EErrandState.DONE, bgId + ": the reply way settles it there and then");
		}
		else
		{
			RZ_Errand way = openings.StepErrand(id);
			Check(way && way != fetch && fetch.m_eState == RZ_EErrandState.DONE, string.Format("%1: the way hands a step of its own (%2) instead of the fetch", bgId, KeyOf(way)));
			if (way)
			{
				float off = RZ_Util.DistXZ(way.m_vPlace, me.GetOrigin());
				Check(off < 600, string.Format("%1: the way's place is %2 m off", bgId, RZ_Util.RoundToInt(off)));
				Check(errands.FinishNow(way, me), bgId + ": the way's step is done there");
			}
		}

		openings.NextNow(id, me);
		int after = openings.StageOf(id);
		Check(after != RZ_EOpeningStage.GIVE && after != RZ_EOpeningStage.FAVOUR, string.Format("%1: no hand-over after the way (stage %2)", bgId, after));
		Done(gm, id);
	}

	//------------------------------------------------------------------------------------------------
	//! Where the check's openings start: the middle of the nearest town (size 2 or more, not on an
	//! island: an opening's start is always among houses, RZ_OpeningRoll), on the ground; the
	//! nearest settlement if there's no town; the tester's own spot if there's none.
	static vector Door(RZ_GameModeComponent gm, IEntity me)
	{
		RZ_Settlement best = Town(gm, me);
		if (!best)
			return me.GetOrigin();

		return RZ_Util.GroundPos(best.m_vCenter);
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_Settlement Town(RZ_GameModeComponent gm, IEntity me)
	{
		RZ_Settlement best;
		float bestDist = 1000000;
		foreach (RZ_Settlement t : gm.GetSettlements())
		{
			if (!t || t.m_iSize < 2 || t.m_bIsland)
				continue;

			float d = RZ_Util.DistXZ(t.m_vCenter, me.GetOrigin());
			if (d < bestDist)
			{
				bestDist = d;
				best = t;
			}
		}

		if (!best)
			best = gm.GetSettlement(gm.FindNearestSettlement(me.GetOrigin()));

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! This check's opening ends, and its contact goes now (the next one needs the story budget).
	protected static void Done(RZ_GameModeComponent gm, int id)
	{
		RZ_Openings openings = gm.GetGuidance().GetOpenings();
		int first;
		IEntity contact = openings.ContactOf(id, first);
		openings.Close(id, "the favour check is done");
		if (contact)
			RZ_Util.DeleteUnit(contact);
	}

	//------------------------------------------------------------------------------------------------
	protected static string KeyOf(RZ_Errand e)
	{
		if (!e || !e.m_Def)
			return "none";

		return e.m_Def.m_sKey;
	}
}
