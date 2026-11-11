//! The clocks (docs/DESIGN.md, "Clocks"): one helper per clock, so every system reads the same one.
//!
//! - Season days: world rhythms. One season day is the first three stages' set lengths / 30 of
//!   minutes played (480 with the built-in stages, about one real day at the 45-day pacing).
//!   Counted only while someone plays, so an empty server's season doesn't move.
//! - Played time: personal pacing, the life's m_fLifeSeconds on the profile.
//! - Real time: anti-abuse limits and the epilogue week (System.GetUnixTime, through Now()).
//! - The game clock: RZ_GameTime (Puzzles/RZ_GameTime.c) turns game-clock windows into real
//!   minutes.
//! Server only.
class RZ_SeasonClock
{
	//------------------------------------------------------------------------------------------------
	//! Minutes played since the outbreak began. 0 without a game mode.
	static float PlayedMinutes()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetDirector())
			return 0;

		return gm.GetDirector().GetPlayedMinutes();
	}

	//------------------------------------------------------------------------------------------------
	//! Played minutes in one season day (480 with the built-in stages and season speed 1).
	static float DayMinutes()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetDirector())
			return 480;

		return gm.GetDirector().GetSeasonDayMinutes();
	}

	//------------------------------------------------------------------------------------------------
	//! The season day as a fraction: 0 at the start, 1.5 halfway through the second day.
	static float Days()
	{
		return PlayedMinutes() / DayMinutes();
	}

	//------------------------------------------------------------------------------------------------
	//! The season day as a number, starting at 1.
	static int Day()
	{
		int whole = Math.Floor(Days());
		return whole + 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Season days since `days` (a value Days() returned earlier). Never negative.
	static float DaysSince(float days)
	{
		return Math.Max(0, Days() - days);
	}

	//------------------------------------------------------------------------------------------------
	//! Real-world time in seconds (anti-abuse limits, the epilogue week).
	static int Now()
	{
		return System.GetUnixTime();
	}

	//------------------------------------------------------------------------------------------------
	//! True while the epilogue runs (the cure is out, or the season ran out and waits for its wipe).
	static bool IsEpilogue()
	{
		return RZ_WorldFacts.Get().IsEpilogue();
	}
}
