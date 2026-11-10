//! The government broadcast (docs/LORE.md, "The outside world: silent, watching").
//! The same official loop on every frequency, sent to everyone on a slow interval and
//! to each player on their first spawn of the session. Its words never change much,
//! but its tone follows the state of the Zone. Flavour only: nothing depends on it.
class RZ_Broadcast
{
	protected RZ_GameModeComponent m_GM;
	protected float m_fTimer;
	protected float m_fLastProgress = -1;
	protected float m_fStallSeconds;			// real seconds since cure research last moved
	protected int m_iLastMilestone = -1;
	protected float m_fSinceBreakthrough = -1;	// real seconds since the last breakthrough, -1 = none this session

	protected static const float STALL_SECONDS = 2700;			// 45 min without research: "stalled"
	protected static const float LONG_STALL_SECONDS = 10800;	// 3 h without research: "indefinitely"
	protected static const float BREAKTHROUGH_SECONDS = 5400;	// breakthrough news lasts 90 min
	protected static const int FIRST_DAY = 11;					// the Zone was sealed eleven days before the season starts

	static const string TITLE = "Government broadcast";

	//------------------------------------------------------------------------------------------------
	void RZ_Broadcast(RZ_GameModeComponent gm)
	{
		m_GM = gm;
	}

	//------------------------------------------------------------------------------------------------
	//! Server tick. intervalMinutes <= 0 turns the looping broadcast off (first-join still works).
	void Tick(float dt, float intervalMinutes)
	{
		RZ_CureResearch cure = m_GM.GetCure();
		if (m_fLastProgress < 0 || cure.m_fProgress > m_fLastProgress + 0.01)
			m_fStallSeconds = 0;
		else
			m_fStallSeconds += dt;

		m_fLastProgress = cure.m_fProgress;

		if (m_iLastMilestone >= 0 && cure.m_iMilestone > m_iLastMilestone)
			m_fSinceBreakthrough = 0;
		else if (m_fSinceBreakthrough >= 0)
			m_fSinceBreakthrough += dt;

		m_iLastMilestone = cure.m_iMilestone;

		if (intervalMinutes <= 0 || cure.IsCured())
			return;

		m_fTimer += dt;
		if (m_fTimer < intervalMinutes * 60)
			return;

		m_fTimer = 0;
		m_GM.NotifyAll(TITLE, GetText());
	}

	//------------------------------------------------------------------------------------------------
	//! "Day 14. This area is under federal quarantine. ..." with the tone for the Zone's state.
	string GetText()
	{
		int day = FIRST_DAY + Math.Floor(m_GM.GetDirector().GetTotalMinutes() / 60);
		return string.Format("\"%1. This area is under federal quarantine. Do not approach the perimeter. %2\"", RZ_NoteText.Day(day), GetToneLine());
	}

	//------------------------------------------------------------------------------------------------
	//! The table in docs/LORE.md. The first match wins.
	protected string GetToneLine()
	{
		if (m_fSinceBreakthrough >= 0 && m_fSinceBreakthrough < BREAKTHROUGH_SECONDS)
			return "Reports of research inside the zone are being evaluated.";

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
