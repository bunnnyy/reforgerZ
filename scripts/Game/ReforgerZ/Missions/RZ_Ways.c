//! The ways through a job (docs/DESIGN.md, "Checks and ways"): the approach helpers every job
//! shares, keyed by identity so a rejoin keeps everything. A job owns one RZ_Ways; questline
//! stage missions (RZ_QuestMission) hand their TryPersuade, TryBribe, Remember and the quiet
//! threat to theirs.
//!
//! - One try per person per job per identity, for every check (Trust, Fear, Trade).
//! - Who did what (the approach enum, RZ_EQuestApproach) per identity, for Did() and payment.
//! - Someone threatened or refused won't talk to that player for the rest of the job.
//! - The quiet threat: m_iMenaceDifficulty > 0 gives a built FORCE option a quiet success
//!   (Fear at that difficulty) before the fight.
//! - Stealth: m_Quiet, the built "not seen, no shots near" for one place (RZ_QuestStealth).
//! - PassAs (a claim in someone's kit) is disguise's and comes with disguise's phase 1.
//! Server only.
class RZ_Ways
{
	string m_sTitle;						// the job's title, for the lines a refusal sends
	int m_iMenaceDifficulty;				// > 0: built FORCE options have a quiet success at this Fear difficulty
	ref RZ_QuestStealth m_Quiet = new RZ_QuestStealth();

	protected ref map<string, int> m_mTried = new map<string, int>();	// "ident|person|check" -> RZ_ECheck
	protected ref map<string, int> m_mDid = new map<string, int>();		// ident -> bitmask of approaches
	protected ref array<string> m_aShut = {};							// "ident|person": won't talk to them this job

	//------------------------------------------------------------------------------------------------
	void RZ_Ways(string title = "")
	{
		m_sTitle = title;
	}

	//------------------------------------------------------------------------------------------------
	static string Ident(RZ_GameModeComponent gm, int playerId)
	{
		if (!gm || playerId <= 0)
			return "";

		string who = gm.GetProfile(playerId).m_sIdentity;
		if (who.IsEmpty())
			who = "id:" + playerId.ToString();

		return who;
	}

	//------------------------------------------------------------------------------------------------
	//! True if this player already tried `check` ("trust", "fear", "trade") at `person` in this job.
	bool Tried(RZ_GameModeComponent gm, int playerId, string person, string check)
	{
		return m_mTried.Contains(Key(gm, playerId, person, check));
	}

	//------------------------------------------------------------------------------------------------
	protected string Key(RZ_GameModeComponent gm, int playerId, string person, string check)
	{
		return string.Format("%1|%2|%3", Ident(gm, playerId), person, check);
	}

	//------------------------------------------------------------------------------------------------
	//! False (and the player is told) if they can't try this at `person`: tried before, or that
	//! person won't talk to them this job.
	protected bool MayTry(RZ_GameModeComponent gm, int playerId, string person, string check)
	{
		string who = Ident(gm, playerId);
		if (m_aShut.Contains(who + "|" + person))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "They won't talk to you. Not after that.");
			return false;
		}

		if (m_mTried.Contains(Key(gm, playerId, person, check)))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You've said your piece. Saying it again won't help.");
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A Trust check (RZ_Checks.TrustScore) at `person`, once per player in this job. Refused shuts
	//! that person to that player for the rest of the job.
	RZ_ECheck TryTrust(RZ_GameModeComponent gm, int playerId, string person, int faction, int difficulty, string favoured, int dialogueBonus = 0)
	{
		if (!MayTry(gm, playerId, person, "trust"))
			return RZ_ECheck.REFUSED;

		RZ_ECheck result = RZ_Checks.Result(RZ_Checks.TrustScore(gm, playerId, faction, favoured, dialogueBonus), difficulty);
		Note(gm, playerId, person, "trust", result);
		Remember(gm, playerId, RZ_EQuestApproach.PERSUADE);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! The built persuade check kept as it was (RZ_QuestApproach.Persuade: pass or fail), once per
	//! player per person in this job. Questline stages use it.
	bool TryPersuade(RZ_GameModeComponent gm, int playerId, int faction, int difficulty, string favoured, string person = "")
	{
		if (!MayTry(gm, playerId, person, "persuade"))
			return false;

		bool ok = RZ_QuestApproach.Persuade(gm, playerId, faction, difficulty, favoured);
		RZ_ECheck result = RZ_ECheck.REFUSED;
		if (ok)
			result = RZ_ECheck.PASS;

		m_mTried.Set(Key(gm, playerId, person, "persuade"), result);
		return ok;
	}

	//------------------------------------------------------------------------------------------------
	//! A threat (RZ_Checks.Threaten) at `person`, once per player in this job. Refused or not, that
	//! person won't talk to this player again this job.
	RZ_ECheck TryFear(RZ_GameModeComponent gm, int playerId, string person, int listener, int giver, int difficulty, bool stronghold, int infamy = 3, string favoured = "")
	{
		if (!MayTry(gm, playerId, person, "fear"))
			return RZ_ECheck.REFUSED;

		RZ_ECheck result = RZ_Checks.Threaten(gm, playerId, person, listener, giver, difficulty, stronghold, infamy, favoured);
		Note(gm, playerId, person, "fear", result);
		Shut(gm, playerId, person);
		Remember(gm, playerId, RZ_EQuestApproach.PRESS);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! The quiet threat on a built FORCE option: Fear at m_iMenaceDifficulty. False without one.
	bool TryMenace(RZ_GameModeComponent gm, int playerId, string person, int listener, int giver, bool stronghold)
	{
		if (m_iMenaceDifficulty <= 0)
			return false;

		return TryFear(gm, playerId, person, listener, giver, m_iMenaceDifficulty, stronghold) == RZ_ECheck.PASS;
	}

	//------------------------------------------------------------------------------------------------
	//! Pay `cost` Favor with `faction`. False (and the player is told) if they haven't got it.
	bool TryBribe(RZ_GameModeComponent gm, int playerId, int faction, int cost)
	{
		bool ok = RZ_QuestApproach.Bribe(gm, playerId, faction, cost, m_sTitle);
		if (ok)
			Remember(gm, playerId, RZ_EQuestApproach.BRIBE);

		return ok;
	}

	//------------------------------------------------------------------------------------------------
	//! Record that this player took `approach` (by identity) for Did() and payment.
	void Remember(RZ_GameModeComponent gm, int playerId, int approach)
	{
		if (approach <= RZ_EQuestApproach.NONE)
			return;

		string who = Ident(gm, playerId);
		if (who.IsEmpty())
			return;

		int mask;
		m_mDid.Find(who, mask);
		m_mDid.Set(who, mask | (1 << approach));
	}

	//------------------------------------------------------------------------------------------------
	//! True if this player took `approach` in this job.
	bool Did(RZ_GameModeComponent gm, int playerId, int approach)
	{
		int mask;
		if (!m_mDid.Find(Ident(gm, playerId), mask))
			return false;

		return (mask & (1 << approach)) != 0;
	}

	//------------------------------------------------------------------------------------------------
	//! `person` won't talk to this player for the rest of the job.
	void Shut(RZ_GameModeComponent gm, int playerId, string person)
	{
		string key = Ident(gm, playerId) + "|" + person;
		if (!m_aShut.Contains(key))
			m_aShut.Insert(key);
	}

	//------------------------------------------------------------------------------------------------
	protected void Note(RZ_GameModeComponent gm, int playerId, string person, string check, RZ_ECheck result)
	{
		m_mTried.Set(Key(gm, playerId, person, check), result);
		if (result == RZ_ECheck.REFUSED)
			Shut(gm, playerId, person);
	}
}
