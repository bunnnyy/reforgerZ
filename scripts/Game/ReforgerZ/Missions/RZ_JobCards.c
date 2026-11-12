//! A job's start and end cards (docs/UI.md, "Notices"): one clear card to that player only when a
//! job of theirs starts ("Quest started: The Other Two", "Job started: Recover the Bus", "Errand
//! started: The Pump") with who gave it and the first thing to do, and one when it ends ("Job done:
//! ...", "Errand failed: ...", "Errand abandoned: ..."). The title's kind word picks the card's icon
//! (RZ_NoticeStyle.JobIcon). Start cards are tagged START: high, a card even in the quiet start, and
//! never said into the talk screen; end cards are MISSION. Both go into the notebook as usual. Server.
class RZ_JobCards
{
	//------------------------------------------------------------------------------------------------
	//! "<Kind> started: <title>", then "From <giver>." and the first objective.
	static void Started(RZ_GameModeComponent gm, int playerId, string kind, string title, string giver, string objective)
	{
		if (!gm || playerId <= 0 || title.IsEmpty())
			return;

		string text = "";
		if (!giver.IsEmpty())
			text = string.Format("From %1.", giver);

		if (!objective.IsEmpty())
		{
			if (!text.IsEmpty())
				text += "\n";

			text += objective;
		}

		gm.NotifyPlayerAs(playerId, string.Format("%1 started: %2", kind, title), text, RZ_ENoticeTag.START);
		RZ_Log.Info(string.Format("Job card: %1 started '%2' for %3", kind, title, RZ_Util.PlayerName(playerId)));
	}

	//------------------------------------------------------------------------------------------------
	//! A mission or contract became this player's (taken in talk, a questline's new stage): its first
	//! objective from where they stand.
	static void MissionStarted(RZ_GameModeComponent gm, int playerId, RZ_Mission m, string giver, IEntity user, string kindOverride = "")
	{
		if (!gm || !m)
			return;

		vector from = vector.Zero;
		if (user)
			from = user.GetOrigin();

		RZ_ObjList list = new RZ_ObjList();
		m.GetObjectives(gm, playerId, from, list);
		string first = list.First(RZ_EObjState.CURRENT);
		string kind = m.KindWord();
		if (!kindOverride.IsEmpty())
			kind = kindOverride;

		Started(gm, playerId, kind, m.m_sTitle, giver, first);
	}

	//------------------------------------------------------------------------------------------------
	//! "<Kind> <how>: <title>" (how: done, failed, abandoned), with what happened.
	static void Ended(RZ_GameModeComponent gm, int playerId, string kind, string how, string title, string text)
	{
		if (!gm || playerId <= 0)
			return;

		gm.NotifyPlayerAs(playerId, string.Format("%1 %2: %3", kind, how, title), text, RZ_ENoticeTag.MISSION);
	}

	//------------------------------------------------------------------------------------------------
	//! The giver as a card names them: "Sergeant Hale at Fort Kell".
	static string LeaderGiver(RZ_Leader l)
	{
		if (!l)
			return "";

		if (l.m_sPlace.IsEmpty())
			return l.m_sName;

		return string.Format("%1 at %2", l.m_sName, l.m_sPlace);
	}
}
