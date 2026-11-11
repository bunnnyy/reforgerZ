//! In-game time of day, and how fast it runs on this server. ReforgerZ never skips time; it sets
//! the day's length at start from the Day length setting (docs/DESIGN.md, The clock); night
//! missions just wait. The rate is read from the time manager's day duration, as the base game's
//! own clock does, so it follows Day length, the scenario's day and night acceleration and an
//! admin's change alike.
//! Shared clock helper (docs/DESIGN.md, "Clocks"): game-clock windows in real minutes go through
//! RealMinutesFor and RealMinutesUntil, at the rate the clock runs now.
class RZ_GameTime
{
	static const float NIGHT_START = 20.5;	// hours; rough, depends on map latitude and date
	static const float NIGHT_END = 5.0;

	//------------------------------------------------------------------------------------------------
	//! Hour of the day, 0..24. -1 if the world has no time manager.
	static float GetHour()
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return -1;

		TimeAndWeatherManagerEntity tm = world.GetTimeAndWeatherManager();
		if (!tm)
			return -1;

		return tm.GetTimeOfTheDay(); // hours as a float, e.g. 16.5
	}

	//------------------------------------------------------------------------------------------------
	//! Game hours per real second the clock runs at now: 24 / the time manager's day duration.
	//! 0 if the world has no time manager or its day doesn't advance.
	static float ClockRate()
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return 0;

		TimeAndWeatherManagerEntity tm = world.GetTimeAndWeatherManager();
		if (!tm || !tm.GetIsDayAutoAdvanced())
			return 0;

		float seconds = tm.GetDayDuration();
		if (seconds <= 0)
			return 0;

		return 24 / seconds;
	}

	//------------------------------------------------------------------------------------------------
	//! Game hours per real second for turning windows into real time: the clock's rate, or the
	//! Day length setting's (default 4 real hours a game day) when the clock can't be read.
	static float Rate()
	{
		float rate = ClockRate();
		if (rate > 0)
			return rate;

		float hours = 4;
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm && gm.GetDayLengthHours() > 0)
			hours = gm.GetDayLengthHours();

		return 24 / (Math.Clamp(hours, 2, 24) * 3600);
	}

	//------------------------------------------------------------------------------------------------
	//! Real minutes a span of `hours` game hours takes.
	static float RealMinutesFor(float hours)
	{
		return Math.Max(0, hours) / Rate() / 60;
	}

	//------------------------------------------------------------------------------------------------
	//! Real minutes until the clock next shows `hour`. -1 without a clock.
	static float RealMinutesUntil(float hour)
	{
		float now = GetHour();
		if (now < 0)
			return -1;

		float ahead = hour - now;
		if (ahead < 0)
			ahead += 24;

		return RealMinutesFor(ahead);
	}

	//------------------------------------------------------------------------------------------------
	//! True when the clock exists and runs, so its windows can be timed.
	static bool IsCalibrated()
	{
		return ClockRate() > 0;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsNightHour(float h)
	{
		return h >= NIGHT_START || h < NIGHT_END;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsNight()
	{
		float h = GetHour();
		if (h < 0)
			return false;

		return IsNightHour(h);
	}

	//------------------------------------------------------------------------------------------------
	//! True if `h` lies in [from, from + length) hours, wrapping past midnight.
	static bool IsWithin(float h, float from, float length)
	{
		float d = h - from;
		if (d < 0)
			d += 24;

		return d < length;
	}

	//------------------------------------------------------------------------------------------------
	//! Real seconds until the clock next shows `hour`. -1 if unknown (no clock, or time doesn't
	//! move on this server).
	static float RealSecondsUntil(float hour)
	{
		float now = GetHour();
		float rate = ClockRate();
		if (now < 0 || rate <= 0)
			return -1;

		float ahead = hour - now;
		if (ahead < 0)
			ahead += 24;

		return ahead / rate;
	}

	//------------------------------------------------------------------------------------------------
	//! Real seconds a span of game hours takes. -1 if unknown.
	static float RealSecondsFor(float hours)
	{
		float rate = ClockRate();
		if (rate <= 0)
			return -1;

		return hours / rate;
	}

	//------------------------------------------------------------------------------------------------
	//! "03:07"
	static string Clock(float h)
	{
		if (h < 0)
			return "--:--";

		int hours = Math.Floor(h);
		int minutes = Math.Floor((h - hours) * 60);
		return RZ_NoteText.Pad(hours % 24, 2) + ":" + RZ_NoteText.Pad(minutes, 2);
	}
}
