//! The clue registry (docs/TRACES.md, "Where scenes stand"): where each running questline's story is
//! now, so a dealt scene of the same faction keeps 1 km off it and never reads as a clue to the
//! wrong story. Read-only, from what the questlines already keep: a run's place (the journal's
//! place and direction) and its stage mission's spot. Nothing is written or saved. Server only.

//------------------------------------------------------------------------------------------------
class RZ_TraceClues
{
	static const float KEEP_OFF = 1000;

	//------------------------------------------------------------------------------------------------
	//! The faction a dealt scene speaks for, -1 for none (no clue check).
	static int KindFaction(string kind)
	{
		switch (kind)
		{
			case "V2": return RZ_EFaction.SURVIVORS;
			case "V3": return RZ_EFaction.SURVIVORS;
			case "V4": return RZ_EFaction.SURVIVORS;
			case "V8": return RZ_EFaction.SURVIVORS;
			case "V5": return RZ_EFaction.INSTITUTE;
			case "V7": return RZ_EFaction.MILITARY;
			case "L6": return RZ_EFaction.MILITARY;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Every active clue of `faction`: each running questline's place and its stage mission's spot.
	static void Active(RZ_GameModeComponent gm, int faction, notnull array<vector> result)
	{
		RZ_Questlines lines = gm.GetQuestlines();
		if (!lines || faction < 0)
			return;

		RZ_MissionManager missions = gm.GetMissions();
		foreach (RZ_QuestRun run : lines.GetRuns())
		{
			if (!run || run.m_bDone || !run.m_Line || run.m_Line.m_iFaction != faction)
				continue;

			if (run.m_bHasPlace)
				result.Insert(run.m_vPlace);

			if (!missions || run.m_iMissionId < 0)
				continue;

			RZ_Mission m = missions.FindById(run.m_iMissionId);
			if (m && !m.m_bGlobal && m.m_vPos != vector.Zero)
				result.Insert(m.m_vPos);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! An active clue of the faction kind `kind` speaks for within 1 km of at.
	static bool Near(RZ_GameModeComponent gm, vector at, string kind)
	{
		array<vector> clues = {};
		Active(gm, KindFaction(kind), clues);
		foreach (vector c : clues)
		{
			if (RZ_Util.DistXZ(c, at) < KEEP_OFF)
				return true;
		}

		return false;
	}
}
