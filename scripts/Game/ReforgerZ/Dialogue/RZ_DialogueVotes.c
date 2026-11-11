//! The majors' sittings in conversations (docs/DIALOGUE.md, "Several players"): a vote a major
//! seats (RZ_QuestVote, docs/DESIGN.md "Votes") is read and swayed by its own rules, one sway per
//! player per sitting. Keys are `<major>.<vote>` (every_name.first) and `<major>.<vote>.<word>`
//! (every_name.first.pot), the words being the ones the major gave its options. Server only.
class RZ_DialogueVotes
{
	//------------------------------------------------------------------------------------------------
	//! The major's run and the vote a key names, or null. `word` gets the third part ("" none).
	protected static RZ_QuestVote VoteFor(RZ_GameModeComponent gm, string key, out string word)
	{
		word = "";
		array<string> parts = {};
		key.Split(".", parts, true);
		if (parts.Count() < 2 || !gm || !gm.GetQuestlines())
			return null;

		RZ_QuestRun run = gm.GetQuestlines().FindRunByKey(parts[0]);
		if (!run)
			return null;

		if (parts.Count() >= 3)
			word = parts[2];

		RZ_QuestVote vote = new RZ_QuestVote(run, parts[1]);
		if (vote.Count() <= 0)
			return null;

		return vote;
	}

	//------------------------------------------------------------------------------------------------
	//! The option a word names on this vote (the major's own words), -1 none.
	protected static int OptionOf(RZ_GameModeComponent gm, string key, string word)
	{
		array<string> parts = {};
		key.Split(".", parts, true);
		RZ_QuestRun run = gm.GetQuestlines().FindRunByKey(parts[0]);
		for (int o = 0; o < RZ_QuestVote.MAX_OPTIONS; o++)
		{
			if (run.Text("vote." + parts[1] + ".w" + o.ToString()) == word)
				return o;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! sitting:every_name.first: the vote is seated and not tallied yet.
	static bool Sitting(RZ_GameModeComponent gm, string key)
	{
		string word;
		RZ_QuestVote vote = VoteFor(gm, key, word);
		return vote && vote.Result() < 0;
	}

	//------------------------------------------------------------------------------------------------
	//! tabled:every_name.first.cards: that option is on the table at an open sitting.
	static bool Tabled(RZ_GameModeComponent gm, string key)
	{
		string word;
		RZ_QuestVote vote = VoteFor(gm, key, word);
		if (!vote || vote.Result() >= 0 || word.IsEmpty())
			return false;

		int o = OptionOf(gm, key, word);
		return o >= 0 && vote.IsTabled(o);
	}

	//------------------------------------------------------------------------------------------------
	//! 1 swayed, 0 not, -1 the key isn't a seated vote (a scene's own tally then).
	static int Swayed(RZ_GameModeComponent gm, int playerId, string key)
	{
		string word;
		RZ_QuestVote vote = VoteFor(gm, key, word);
		if (!vote)
			return -1;

		if (vote.HasSwayed(gm, playerId))
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! sway:every_name.first.pot from a conversation. True if the key is a seated vote (then this
	//! handled it: `said` gets the line); false for a scene's own tally. The voter swung is the
	//! speaker, when they vote; else the first voter who doesn't lean that way yet.
	static bool Sway(RZ_GameModeComponent gm, int playerId, string key, RZ_Leader speaker, out string said)
	{
		said = "";
		string word;
		RZ_QuestVote vote = VoteFor(gm, key, word);
		if (!vote)
			return false;

		int option = OptionOf(gm, key, word);
		if (vote.Result() >= 0 || option < 0 || !vote.IsTabled(option))
		{
			said = "\"It's not on the table. Not at this sitting.\"";
			return true;
		}

		if (vote.HasSwayed(gm, playerId))
		{
			said = "(You've had your say at this sitting.)";
			return true;
		}

		int voter = -1;
		for (int i = 0; i < vote.Count(); i++)
		{
			if (speaker && vote.VoterLeader(i) == speaker.m_iId)
				voter = i;
		}

		for (int k = 0; k < vote.Count() && voter < 0; k++)
		{
			if (vote.Lean(k) != option)
				voter = k;
		}

		if (voter < 0)
		{
			said = "(Everyone at the table leans that way already.)";
			return true;
		}

		if (vote.Sway(gm, playerId, voter, option))
		{
			said = string.Format("(%1 leans that way now.)", vote.VoterName(voter));
			RZ_Log.Info(string.Format("Dialogue: sway %1 by %2 (voter %3)", key, RZ_Util.PlayerName(playerId), voter));
		}

		return true;
	}
}
