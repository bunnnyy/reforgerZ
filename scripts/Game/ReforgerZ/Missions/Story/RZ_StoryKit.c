//! Helpers the story jobs share (docs/DESIGN.md, "Story blocks"): who may decide a scene's option
//! (eligible and took part, else "It's not your call."), a background read told to that player
//! alone and kept in the journal, a world record with its epilogue line, a fallout errand at the
//! subject, and the mission's people as stand-ins. Server only.
class RZ_StoryKit
{
	//------------------------------------------------------------------------------------------------
	//! Options that change the scene or end the job belong to someone the job was offered to who took
	//! part (a step, or 60 s within 150 m). False, and they're told, if not.
	static bool MayDecide(RZ_GameModeComponent gm, RZ_Mission m, int playerId)
	{
		if (!m.m_Story)
			return m.IsEligible(gm, playerId);

		m.m_Story.m_Deciding.Step(gm, playerId);
		if (m.m_Story.m_Deciding.MayDecide(gm, m, playerId))
			return true;

		gm.NotifyPlayer(playerId, m.m_sTitle, "\"It's not your call. You weren't hired for this.\"");
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A background read: one line to that player only, and the journal's "What you've found".
	static bool Read(RZ_GameModeComponent gm, RZ_Mission m, int playerId, string backgrounds, string line)
	{
		if (!RZ_QuestApproach.HasBackground(gm, playerId, backgrounds))
			return false;

		gm.NotifyPlayer(playerId, m.m_sTitle, line);
		if (m.m_Story && !m.m_Story.m_aFound.Contains(line))
			m.m_Story.m_aFound.Insert(line);

		RZ_Log.Info(string.Format("Story: %1 read by %2 (%3)", m.m_sTitle, RZ_Util.PlayerName(playerId), backgrounds));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A world record about this job's subject, with who did it and an epilogue line (and its carrier).
	static void Record(RZ_Mission m, string what, string subject, string value, array<int> players, string epilogue = "", string carrier = "board")
	{
		string key = what;
		if (m.m_Story)
			key = m.m_Story.m_sKey + "." + what;

		RZ_WorldFacts.Get().Memory().Record(key, subject, value, RZ_WorldMemory.SOURCE_STORY, false, players, epilogue, carrier);
		RZ_Log.Info(string.Format("Story: %1 record %2 (%3) = %4", m.m_sTitle, key, subject, value));
	}

	//------------------------------------------------------------------------------------------------
	//! A failed story job's fallout: one errand at its subject, dealt to its participants first for 20
	//! minutes, then to anyone, paying at most 30% of the job's base (RZ_Errands.Fallout).
	static void Fallout(RZ_GameModeComponent gm, RZ_Mission m, string card, vector at, string who)
	{
		RZ_Errands errands = gm.GetErrands();
		if (!errands || m.m_bWithdrawn)
			return;

		int cap = RZ_Util.RoundToInt(m.m_iFavorReward * 0.3);
		errands.Fallout(card, at, m.m_aParticipants, cap, m.m_sTitle, who);
	}
}
