//! In-game time of day, and how fast it runs on this server. ReforgerZ never skips time; it sets
//! the day's length at start (docs/DESIGN.md, The clock); night missions just wait. The speed is measured by
//! sampling the clock on the server tick, so it works with any time acceleration
//! the server config sets, without needing an API for it.
class RZ_GameTime
{
	static const float NIGHT_START = 20.5;	// hours; rough, depends on map latitude and date
	static const float NIGHT_END = 5.0;

	protected static float s_fRate;			// game hours per real second, 0 = not measured yet
	protected static float s_fLastHour = -1;
	protected static float s_fSampleSeconds;
	protected static float s_fSampleHours;

	protected static const float SAMPLE_WINDOW = 60;

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
	//! Called from the game mode's server tick.
	static void Tick(float dt)
	{
		float h = GetHour();
		if (h < 0)
			return;

		if (s_fLastHour >= 0)
		{
			float dh = h - s_fLastHour;
			if (dh < -12)
				dh += 24;	// past midnight

			if (dh >= 0 && dh < 6)
			{
				s_fSampleSeconds += dt;
				s_fSampleHours += dh;
			}
		}

		s_fLastHour = h;

		if (s_fSampleSeconds >= SAMPLE_WINDOW)
		{
			float measured = s_fSampleHours / s_fSampleSeconds;
			if (s_fRate <= 0)
				s_fRate = measured;
			else
				s_fRate = s_fRate * 0.7 + measured * 0.3;

			s_fSampleSeconds = 0;
			s_fSampleHours = 0;
		}
	}

	//------------------------------------------------------------------------------------------------
	static bool IsCalibrated()
	{
		return s_fRate > 0;
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
	//! Real seconds until the clock next shows `hour`. -1 if unknown (not measured yet,
	//! or time doesn't move on this server).
	static float RealSecondsUntil(float hour)
	{
		float now = GetHour();
		if (now < 0 || s_fRate <= 0)
			return -1;

		float ahead = hour - now;
		if (ahead < 0)
			ahead += 24;

		return ahead / s_fRate;
	}

	//------------------------------------------------------------------------------------------------
	//! Real seconds a span of game hours takes. -1 if unknown.
	static float RealSecondsFor(float hours)
	{
		if (s_fRate <= 0)
			return -1;

		return hours / s_fRate;
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
