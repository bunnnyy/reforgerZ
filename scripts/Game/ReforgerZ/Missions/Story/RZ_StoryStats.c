//! The story jobs' hourly summary (docs/DESIGN.md, "Story blocks"; for tuning from the logs): Fame and
//! Favor per player-hour paid by the story jobs (by job), how often a result was switched to another
//! side or given to nobody, food spent on trades, fallout errands started and the Favor they paid.
//! One log line every real hour, then the counts start again. Server only; nothing saved.
class RZ_StoryStats
{
	protected static const float PERIOD = 3600;

	protected static float s_fClock;
	protected static float s_fPlayerSeconds;
	protected static int s_iFame;
	protected static int s_iFavor;
	protected static int s_iFood;
	protected static int s_iFallouts;
	protected static int s_iFalloutFavor;
	protected static ref map<string, int> s_mByJob;		// key -> Fame + Favor
	protected static ref map<string, int> s_mSwitch;		// key -> switches
	protected static ref map<string, int> s_mNobody;		// key -> "nobody"

	//------------------------------------------------------------------------------------------------
	protected static void Ensure()
	{
		if (!s_mByJob)
			s_mByJob = new map<string, int>();

		if (!s_mSwitch)
			s_mSwitch = new map<string, int>();

		if (!s_mNobody)
			s_mNobody = new map<string, int>();
	}

	//------------------------------------------------------------------------------------------------
	//! On the mission manager's tick: player-seconds, and the line once an hour.
	static void Tick(float dt, int online)
	{
		Ensure();
		s_fClock += dt;
		s_fPlayerSeconds += dt * online;
		if (s_fClock < PERIOD)
			return;

		float hours = Math.Max(0.01, s_fPlayerSeconds / 3600);
		string jobs = "";
		foreach (string key, int paid : s_mByJob)
		{
			jobs += string.Format(" %1 %2", key, paid);
		}

		string picks = "";
		foreach (string sw, int n : s_mSwitch)
		{
			picks += string.Format(" %1 switched %2", sw, n);
		}

		foreach (string nb, int none : s_mNobody)
		{
			picks += string.Format(" %1 nobody %2", nb, none);
		}

		RZ_Log.Info(string.Format("Story | per player-hour: Fame %1, Favor %2 | by job:%3 | picks:%4 | food on trades %5 | fallout: started %6, paid %7 Favor",
			RZ_Util.RoundToInt(s_iFame / hours), RZ_Util.RoundToInt(s_iFavor / hours), jobs, picks, s_iFood, s_iFallouts, s_iFalloutFavor));

		s_fClock = 0;
		s_fPlayerSeconds = 0;
		s_iFame = 0;
		s_iFavor = 0;
		s_iFood = 0;
		s_iFallouts = 0;
		s_iFalloutFavor = 0;
		s_mByJob.Clear();
		s_mSwitch.Clear();
		s_mNobody.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! A story job paid a player (RZ_MissionManager.RewardPlayer).
	static void Paid(RZ_Mission m, int fame, int favor)
	{
		if (!m || !m.m_Story)
			return;

		Ensure();
		s_iFame += fame;
		s_iFavor += favor;
		int sum;
		s_mByJob.Find(m.m_Story.m_sKey, sum);
		s_mByJob.Set(m.m_Story.m_sKey, sum + fame + favor);
	}

	//------------------------------------------------------------------------------------------------
	//! Who benefits: a result went to another side, or to nobody.
	static void Picked(string key, bool switched, bool nobody)
	{
		Ensure();
		int n;
		if (switched)
		{
			s_mSwitch.Find(key, n);
			s_mSwitch.Set(key, n + 1);
		}

		if (nobody)
		{
			int none;
			s_mNobody.Find(key, none);
			s_mNobody.Set(key, none + 1);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Food handed over in a story job's Trade way, counted; the same as RZ_HavenQuest.TakeFood.
	static bool TakeFood(RZ_GameModeComponent gm, IEntity character, int count)
	{
		if (!RZ_HavenQuest.TakeFood(gm, character, count))
			return false;

		s_iFood += count;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static void FalloutStarted()
	{
		s_iFallouts++;
	}

	//------------------------------------------------------------------------------------------------
	static void FalloutPaid(int favor)
	{
		s_iFalloutFavor += favor;
	}
}
