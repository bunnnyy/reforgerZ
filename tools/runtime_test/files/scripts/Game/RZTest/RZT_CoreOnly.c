//! Core only (run.sh core): the server boots with every Features switch off (docs/FEATURES.md), the
//! tester walks the first towns for a few minutes, and nothing a switched-off feature does happens:
//! no camp pieces, patrols, trucks, safehouses, wanderers, errands, jobs, opening, Tonight, habit,
//! research, tribes, Long Table, wildlife. Only this check runs (RZT_Diag hands it the tick).
//! "RZT core OK|FAIL ...", "RZT core: N checks, M failed", "RZT core done".
class RZT_CoreOnly
{
	static const int RUN_SECONDS = 420;
	static const int CHECK_EVERY = 30;

	protected static int s_iChecks;
	protected static int s_iFails;
	protected static int s_iStop;
	protected static bool s_bDone;
	protected static float s_fProgressAt = -1;

	//------------------------------------------------------------------------------------------------
	//! Every switch is off: this is the core run.
	static bool IsCore()
	{
		for (int i = 0; i < RZ_EFeature.COUNT; i++)
		{
			if (RZ_Features.On(i))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Check(bool ok, string what)
	{
		s_iChecks++;
		if (ok)
		{
			Print("RZT core OK " + what);
			return;
		}

		s_iFails++;
		Print("RZT core FAIL " + what);
	}

	//------------------------------------------------------------------------------------------------
	//! From RZT_Diag's one-second tick, instead of every other check.
	static void Tick(RZ_GameModeComponent gm, int sec, notnull array<vector> route)
	{
		if (s_bDone)
			return;

		if (sec == 4)
		{
			Print("RZT core: " + RZ_Features.Describe());
			if (gm.GetCure())
				s_fProgressAt = gm.GetCure().m_fProgress;
		}

		// The tester walks the route's towns, a stop every 60 s, so the core spawns near him
		if (sec > 5 && sec % 60 == 0 && RZT_Fake.s_Ent && RZ_Util.IsCharacterAlive(RZT_Fake.s_Ent) && !route.IsEmpty())
		{
			s_iStop = (s_iStop + 1) % route.Count();
			RZT_Fake.s_Ent.SetOrigin(route[s_iStop] + Vector(0, 1, 0));
		}

		if (sec > 10 && sec % CHECK_EVERY == 0)
			Look(gm, sec);

		if (sec >= RUN_SECONDS)
		{
			Last(gm);
			s_bDone = true;
			Print(string.Format("RZT core: %1 checks, %2 failed", s_iChecks, s_iFails));
			Print("RZT core done");
			GetGame().RequestClose();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! What a switched-off feature would have put in the world by now: none of it.
	protected static void Look(RZ_GameModeComponent gm, int sec)
	{
		string at = string.Format(" (%1 s)", sec);
		if (gm.GetCampLife())
			Check(gm.GetCampLife().CountPieces() == 0, "no camp pieces: " + gm.GetCampLife().CountPieces().ToString() + at);

		if (gm.GetRoadPatrols())
			Check(gm.GetRoadPatrols().Count() == 0, "no road patrols: " + gm.GetRoadPatrols().Count().ToString() + at);

		if (gm.GetSafehouses())
			Check(gm.GetSafehouses().Count() == 0, "no safehouses: " + gm.GetSafehouses().Count().ToString() + at);

		if (gm.GetSupply())
			Check(!gm.GetSupply().IsActive(), "no supply trucks" + at);

		if (gm.GetErrands())
			Check(!gm.GetErrands().IsOn(), "no errands" + at);

		if (gm.GetLeaders())
			Check(!gm.GetLeaders().IsEnabled(), "no leaders" + at);

		if (gm.GetTribes())
			Check(!gm.GetTribes().IsEnabled(), "no tribes" + at);

		if (gm.GetLongTable())
			Check(!gm.GetLongTable().IsEnabled(), "no Long Table" + at);

		if (gm.GetWildlife())
			Check(gm.GetWildlife().CountHerds() == 0, "no wildlife herds" + at);

		// No job of any kind: the director's contracts and events are off, and nothing that starts its
		// steps as missions (questlines, majors, leaders, arcs, the opening) runs
		array<ref RZ_Mission> active = gm.GetMissions().GetActive();
		string titles = "";
		foreach (RZ_Mission m : active)
		{
			titles = titles + " [" + m.m_sTitle + "]";
		}

		Check(active.IsEmpty(), "no jobs running: " + active.Count().ToString() + titles + at);

		// The tester: no opening, no Tonight line, no habit
		if (gm.GetGuidance() && gm.GetGuidance().GetOpenings())
			Check(gm.GetGuidance().GetOpenings().OpeningIdOf(RZT_Fake.ID) < 0, "the tester has no opening" + at);

		RZ_UIFeed feed = gm.GetUIFeed();
		if (feed && feed.GetTonight() && RZT_Fake.s_Ent)
		{
			array<RZ_TonightLine> lines = {};
			feed.GetTonight().LinesFor(RZT_Fake.ID, RZT_Fake.s_Ent, lines);
			Check(lines.IsEmpty(), "no Tonight lines: " + lines.Count().ToString() + at);
		}

		Check(RZ_Habits.OfPlayer(RZT_Fake.ID) == RZ_EPlayerHabit.NONE, "the tester has no habit" + at);
	}

	//------------------------------------------------------------------------------------------------
	//! At the end: the core itself ran (towns, the infected, the tester alive or come round), and the
	//! lab didn't move.
	protected static void Last(RZ_GameModeComponent gm)
	{
		Check(!gm.GetSettlements().IsEmpty(), "the towns were found: " + gm.GetSettlements().Count().ToString());
		Check(RZT_Fake.s_Ent != null, "the tester is in the world");
		if (gm.GetCure() && s_fProgressAt >= 0)
			Check(gm.GetCure().m_fProgress == s_fProgressAt, string.Format("research didn't move: %1 then %2", s_fProgressAt, gm.GetCure().m_fProgress));

		Check(!RZ_Features.On(RZ_EFeature.SHOPS), "shops are off (no Trade, no shop actions)");
		Check(gm.GetFeaturesOff().Length() > 2, "the clients are told what's off: " + gm.GetFeaturesOff());
	}
}
