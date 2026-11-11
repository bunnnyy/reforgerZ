//! The government broadcast (docs/LORE.md, "The outside world: silent, watching"; its lines:
//! docs/WRITING.md, "The government broadcast").
//! The same official loop on every frequency, sent to everyone on a slow interval and
//! to each player on their first spawn of the session. Its words never change much,
//! but its tone follows the state of the Zone. Flavour only: nothing depends on it.
class RZ_Broadcast
{
	protected RZ_GameModeComponent m_GM;
	protected float m_fTimer;
	protected float m_fLastProgress = -1;
	protected float m_fStallSeconds;			// real seconds since cure research last moved
	protected float m_fSinceBreakthrough = -1;	// seconds online since players heard the last breakthrough, -1 = none this session

	static const float STALL_SECONDS = 2700;			// 45 min without research: "stalled"
	static const float LONG_STALL_SECONDS = 10800;		// 3 h without research: "indefinitely"
	protected static const float BREAKTHROUGH_SECONDS = 5400;	// breakthrough news lasts 90 min
	protected static const int FIRST_DAY = 11;					// the Zone was sealed eleven days before the season starts
	protected static const float PLAYED_MINUTES_PER_DAY = 480;	// fallback without a real clock: 8 hours played a day

	static const string TITLE = "Government broadcast";

	//------------------------------------------------------------------------------------------------
	void RZ_Broadcast(RZ_GameModeComponent gm)
	{
		m_GM = gm;
	}

	//------------------------------------------------------------------------------------------------
	//! The cure's breakthrough news was just told (RZ_CureResearch.FlushNews, once someone is in the
	//! world to hear it). For the next 90 minutes online the broadcast evaluates "reports of research".
	void OnBreakthrough()
	{
		m_fSinceBreakthrough = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The breakthrough window is open (the season mood's Hope).
	bool InBreakthroughWindow()
	{
		return m_fSinceBreakthrough >= 0 && m_fSinceBreakthrough < BREAKTHROUGH_SECONDS;
	}

	//------------------------------------------------------------------------------------------------
	//! Real seconds since research last moved.
	float StallSeconds()
	{
		return m_fStallSeconds;
	}

	//------------------------------------------------------------------------------------------------
	//! Server tick. intervalMinutes <= 0 turns the looping broadcast off (first-join still works).
	//! online: a player is in the world (the breakthrough window only runs then).
	void Tick(float dt, float intervalMinutes, bool online)
	{
		// The lab works research through a little at a time (RZ_CureResearch.Work), so "moved" is
		// measured from where it stood when it last moved, not from the last tick
		RZ_CureResearch cure = m_GM.GetCure();
		if (m_fLastProgress < 0 || cure.m_fProgress > m_fLastProgress + 0.01 || cure.m_fProgress < m_fLastProgress)
		{
			m_fStallSeconds = 0;
			m_fLastProgress = cure.m_fProgress;
		}
		else
		{
			m_fStallSeconds += dt;
		}

		if (m_fSinceBreakthrough >= 0 && online)
			m_fSinceBreakthrough += dt;

		if (intervalMinutes <= 0 || cure.IsCured())
			return;

		m_fTimer += dt;
		if (m_fTimer < intervalMinutes * 60)
			return;

		m_fTimer = 0;
		m_GM.NotifyAll(TITLE, GetText());
	}

	//------------------------------------------------------------------------------------------------
	//! Days since the gates closed, as the broadcast counts them: real days since the season
	//! started (docs/DESIGN.md, "The government broadcast"). Without the server's clock, a day
	//! is 8 hours played (the season pacing's hours with someone online a day).
	int GetDay()
	{
		RZ_Seasons seasons = m_GM.GetSeasons();
		if (seasons)
		{
			int days = seasons.GetRealDays();
			if (days >= 0)
				return FIRST_DAY + days;
		}

		return FIRST_DAY + Math.Floor(m_GM.GetDirector().GetTotalMinutes() / PLAYED_MINUTES_PER_DAY);
	}

	//------------------------------------------------------------------------------------------------
	//! "Day 14. This area is under federal quarantine. ..." with the tone for the Zone's state.
	//! After the cure, the new order (the loop itself stops; first spawns still hear this).
	string GetText()
	{
		int day = GetDay();
		if (m_GM.GetFacts() && m_GM.GetFacts().IsCured())
			return string.Format("\"%1. The quarantine order for this area has been lifted. Remain where you are. Processing will begin at the road checkpoints.\"", RZ_NoteText.Day(day));

		return string.Format("\"%1. This area is under federal quarantine. Do not approach the perimeter. %2\"", RZ_NoteText.Day(day), GetToneLine());
	}

	//------------------------------------------------------------------------------------------------
	//! The table in docs/WRITING.md, "The government broadcast": the season mood's ladder (the same
	//! one every voice reads), with the gate after Hope. The first match wins.
	protected string GetToneLine()
	{
		if (InBreakthroughWindow())
			return "Reports of research inside the zone are being evaluated.";

		RZ_WorldFacts facts = m_GM.GetFacts();
		if (facts && facts.AtFinaleGate())
			return "The status of this area is under review. Remain where you are.";

		if (facts && facts.SeasonMoodOn())
		{
			RZ_EMood mood = facts.SeasonMood();
			if (mood == RZ_EMood.BLEAK)
			{
				if (m_fStallSeconds >= LONG_STALL_SECONDS)
					return "The perimeter will be maintained indefinitely.";

				if (m_GM.GetDirector().GetStage() >= RZ_EOutbreakStage.AFTERMATH)
					return "Residents are thanked for remaining indoors. Residents are thanked for remaining.";

				return "The perimeter will be maintained.";
			}

			if (mood == RZ_EMood.GRIM)
				return "Residents are reminded that the perimeter is for your protection.";

			return "Remain indoors. Aid is on its way.";
		}

		if (m_fStallSeconds >= LONG_STALL_SECONDS)
			return "The perimeter will be maintained indefinitely.";

		RZ_EOutbreakStage stage = m_GM.GetDirector().GetStage();
		float progress = m_GM.GetCure().m_fProgress;
		if (stage >= RZ_EOutbreakStage.OVERRUN && progress < 50)
			return "The perimeter will be maintained.";

		if (stage >= RZ_EOutbreakStage.COLLAPSE || m_GM.GetDirector().GetAverageInfection() >= 45 || m_fStallSeconds >= STALL_SECONDS)
			return "Residents are reminded that the perimeter is for your protection.";

		return "Remain indoors. Aid is on its way.";
	}
}
