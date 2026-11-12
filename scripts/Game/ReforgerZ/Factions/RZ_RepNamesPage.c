//! Reputation on the standing page and in the journal (docs/REPUTATION.md, "What people call you").
//!
//! - **The header** carries the whole picture (RZ_RepVoice.Picture), the `H` record's fifth field.
//! - **Each faction's leaders column** opens with two lines, the `W` record: the word as a sentence
//!   ("The crews call you Lucky.") and the nearest met leader's verdict.
//! - **A parley's job** reads "Amends: works off Infamy · finishes where the job says" in the
//!   journal's requirements.
//! Server only (the feed builds them; the notebook only draws them).
class RZ_RepNamesPage
{
	static const string AMENDS_LINE = "Amends: works off Infamy · finishes where the job says";

	//------------------------------------------------------------------------------------------------
	//! The whole picture for the header, "" with Reputation > Names off.
	static string Header(RZ_GameModeComponent gm, int playerId)
	{
		if (!gm || playerId <= 0 || !RZ_ReputationSettings.Get().m_bNames)
			return "";

		return RZ_RepVoice.Picture(gm, playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! The word as a sentence, in the faction's register: "The crews call you Lucky."
	static string WordLine(RZ_GameModeComponent gm, int playerId, int f)
	{
		if (!gm || playerId <= 0 || !RZ_Factions.IsValid(f))
			return "";

		string word = RZ_RepVoice.Word(gm, playerId, f);
		if (word.IsEmpty() || word == "you")
		{
			switch (f)
			{
				case RZ_EFaction.BANDITS: return "The crews don't call you anything yet.";
				case RZ_EFaction.MILITARY: return "Bastion hasn't got a word for you yet.";
				case RZ_EFaction.INSTITUTE: return "The lab hasn't filed you under anything yet.";
			}

			return "The houses haven't got a word for you yet.";
		}

		switch (f)
		{
			case RZ_EFaction.MILITARY: return string.Format("Bastion has you down as %1.", word);
			case RZ_EFaction.BANDITS: return string.Format("The crews call you %1.", word);
			case RZ_EFaction.INSTITUTE: return string.Format("The lab has you down as %1.", word);
		}

		return string.Format("The houses call you %1.", word);
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest leader of `f` this player has met, and what they make of you, "" if none.
	static string Verdict(RZ_GameModeComponent gm, int playerId, int f, vector from)
	{
		RZ_Leaders leaders = gm.GetLeaders();
		if (!leaders || playerId <= 0)
			return "";

		array<RZ_Leader> met = {};
		leaders.ListMet(playerId, met);
		RZ_Leader best;
		float bestDist = -1;
		foreach (RZ_Leader l : met)
		{
			if (l.m_eFaction != f)
				continue;

			float d = RZ_Util.DistXZ(l.m_vPos, from);
			if (!best || d < bestDist)
			{
				best = l;
				bestDist = d;
			}
		}

		if (!best)
			return "";

		return string.Format("%1: %2", best.m_sName, leaders.OpinionLine(best, playerId));
	}

	//------------------------------------------------------------------------------------------------
	//! The journal's requirements for a contract the parley reserved for this player, else "".
	static string JournalLine(RZ_GameModeComponent gm, RZ_Mission m, int playerId)
	{
		if (!gm || !m || !m.m_bAmends || m.m_sReservedFor.IsEmpty() || playerId <= 0)
			return "";

		if (gm.GetProfile(playerId).m_sIdentity != m.m_sReservedFor)
			return "";

		return AMENDS_LINE;
	}
}
