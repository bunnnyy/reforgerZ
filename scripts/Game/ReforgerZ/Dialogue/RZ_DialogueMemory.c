//! What people remember of what you said to them (docs/DIALOGUE.md, "Memory"). Server only.
//!
//! Per player (identity): regard per speaker (-4..+4, one step toward 0 each real day), failed
//! checks, knowledge, and the day's talk Favor. Per character (this life; a new life is a new
//! face): faces, claims, seen nodes, passed checks, once/daily topics, character flags and each
//! speaker's notes. Saved in its own file, $profile:ReforgerZ/dialogue.json, at the 2-minute save
//! only when something changed, and dropped when the season changes. state.json doesn't grow.

//------------------------------------------------------------------------------------------------
//! One player's dialogue memory (also the save record).
class RZ_DMemPlayer
{
	string ident;
	int life;							// the life the per-character parts belong to
	ref array<string> regardKeys = {};
	ref array<int> regardVals = {};
	ref array<int> regardUnix = {};		// when the value was last set: it decays from there
	ref array<string> failKeys = {};	// "speaker|conv/@choice"
	ref array<int> failScores = {};
	ref array<string> knows = {};
	int favorDay;						// real day (unix / 86400) of favorToday
	ref array<int> favorToday = {0, 0, 0, 0};
	// Per character
	int faces;							// RZ_Factions.Bit of factions that know your face
	ref array<string> seen = {};		// "conv/node"
	ref array<string> passed = {};		// "speaker|conv/@choice"
	ref array<string> claims = {};		// "speaker|key"
	ref array<string> topicKeys = {};	// "speaker|conv": once and daily topics
	ref array<int> topicUnix = {};
	ref array<string> cflagKeys = {};
	ref array<int> cflagVals = {};
	ref array<string> notes = {};		// "speaker|key": this speaker's memory of this character
	ref array<string> deedSeats = {};	// local deeds: one phrase per seat (reputation's voice)
	ref array<string> deedTexts = {};
}

//------------------------------------------------------------------------------------------------
class RZ_DMemSave
{
	int season;
	ref array<ref RZ_DMemPlayer> players = {};
}

//------------------------------------------------------------------------------------------------
class RZ_DialogueMemory
{
	static const string PATH = "$profile:ReforgerZ/dialogue.json";
	static const int REGARD_MAX = 4;
	static const int DAY = 86400;
	static const int CLAIMS_PER_SPEAKER = 6;
	static const int KNOWS_MAX = 96;
	static const int SEEN_MAX = 512;

	protected RZ_GameModeComponent m_GM;
	protected ref map<string, ref RZ_DMemPlayer> m_mPlayers = new map<string, ref RZ_DMemPlayer>();
	protected bool m_bDirty;
	protected int m_iSeason;

	//------------------------------------------------------------------------------------------------
	void RZ_DialogueMemory(RZ_GameModeComponent gm)
	{
		m_GM = gm;
	}

	// ================================================================ Saving
	//------------------------------------------------------------------------------------------------
	//! At start: the saved file, unless it belongs to another season.
	void Load()
	{
		RZ_Seasons seasons = m_GM.GetSeasons();
		if (seasons)
			m_iSeason = seasons.GetSeason();

		if (!FileIO.FileExists(PATH))
			return;

		JsonLoadContext ctx = new JsonLoadContext();
		if (!ctx.LoadFromFile(PATH))
		{
			RZ_Log.Warn("Could not read " + PATH);
			return;
		}

		RZ_DMemSave data = new RZ_DMemSave();
		if (!ctx.ReadValue("dialogue", data) || !data.players)
			return;

		if (data.season != m_iSeason)
		{
			RZ_Log.Info(string.Format("Dialogue: memory from season %1 dropped (season %2)", data.season, m_iSeason));
			m_bDirty = true;
			return;
		}

		foreach (RZ_DMemPlayer p : data.players)
		{
			if (!p || p.ident.IsEmpty())
				continue;

			Repair(p);
			m_mPlayers.Set(p.ident, p);
		}

		RZ_Log.Info(string.Format("Dialogue: memory for %1 players", m_mPlayers.Count()));
	}

	//------------------------------------------------------------------------------------------------
	//! At the 2-minute save, only when something changed.
	void SaveIfChanged()
	{
		if (!m_bDirty)
			return;

		m_bDirty = false;
		RZ_DMemSave data = new RZ_DMemSave();
		data.season = m_iSeason;
		foreach (string ident, RZ_DMemPlayer p : m_mPlayers)
		{
			data.players.Insert(p);
		}

		FileIO.MakeDirectory(RZ_Persistence.DIR);
		JsonSaveContext ctx = new JsonSaveContext();
		ctx.WriteValue("dialogue", data);
		if (!ctx.SaveToFile(PATH))
			RZ_Log.Warn("Could not write " + PATH);
	}

	//------------------------------------------------------------------------------------------------
	//! Arrays a hand-edited or older file left out.
	protected void Repair(RZ_DMemPlayer p)
	{
		if (!p.regardKeys)
			p.regardKeys = new array<string>();
		if (!p.regardVals)
			p.regardVals = new array<int>();
		if (!p.regardUnix)
			p.regardUnix = new array<int>();
		if (!p.failKeys)
			p.failKeys = new array<string>();
		if (!p.failScores)
			p.failScores = new array<int>();
		if (!p.knows)
			p.knows = new array<string>();
		if (!p.favorToday)
			p.favorToday = new array<int>();
		while (p.favorToday.Count() < 4)
		{
			p.favorToday.Insert(0);
		}
		if (!p.seen)
			p.seen = new array<string>();
		if (!p.passed)
			p.passed = new array<string>();
		if (!p.claims)
			p.claims = new array<string>();
		if (!p.topicKeys)
			p.topicKeys = new array<string>();
		if (!p.topicUnix)
			p.topicUnix = new array<int>();
		if (!p.cflagKeys)
			p.cflagKeys = new array<string>();
		if (!p.cflagVals)
			p.cflagVals = new array<int>();
		if (!p.notes)
			p.notes = new array<string>();
		if (!p.deedSeats)
			p.deedSeats = new array<string>();
		if (!p.deedTexts)
			p.deedTexts = new array<string>();
	}

	//------------------------------------------------------------------------------------------------
	int Count()
	{
		return m_mPlayers.Count();
	}

	// ================================================================ The record
	//------------------------------------------------------------------------------------------------
	//! This player's record, with the per-character parts cleared when the life has changed. Null
	//! for a player with no identity.
	RZ_DMemPlayer For(int playerId)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		if (!profile || profile.m_sIdentity.IsEmpty())
			return null;

		RZ_DMemPlayer p;
		if (!m_mPlayers.Find(profile.m_sIdentity, p))
		{
			p = new RZ_DMemPlayer();
			p.ident = profile.m_sIdentity;
			p.life = profile.m_iLives;
			m_mPlayers.Set(p.ident, p);
		}

		if (p.life != profile.m_iLives)
		{
			p.life = profile.m_iLives;
			p.faces = 0;
			p.seen.Clear();
			p.passed.Clear();
			p.claims.Clear();
			p.topicKeys.Clear();
			p.topicUnix.Clear();
			p.cflagKeys.Clear();
			p.cflagVals.Clear();
			p.notes.Clear();
			p.deedSeats.Clear();
			p.deedTexts.Clear();
			m_bDirty = true;
		}

		return p;
	}

	// ================================================================ Regard (per player)
	//------------------------------------------------------------------------------------------------
	//! -4..+4 after decay: one step toward 0 for each real day since it was last set.
	int Regard(int playerId, string speaker)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p || speaker.IsEmpty())
			return 0;

		int i = p.regardKeys.Find(speaker);
		if (i < 0)
			return 0;

		return Decayed(p.regardVals[i], p.regardUnix[i]);
	}

	//------------------------------------------------------------------------------------------------
	protected int Decayed(int value, int unix)
	{
		int days = (RZ_SeasonClock.Now() - unix) / DAY;
		if (days <= 0 || value == 0)
			return value;

		if (value > 0)
			return Math.Max(0, value - days);

		return Math.Min(0, value + days);
	}

	//------------------------------------------------------------------------------------------------
	//! The regard words (docs/DIALOGUE.md, "Regard"): thanks +1, kept +1, broke -1, threatened -1,
	//! slander -1, vouched_liar -1, lied -3 (a Merciful speaker -2, a Paranoid one -4).
	void AddRegard(int playerId, string speaker, string word, int lieWeight = 3)
	{
		int delta = 0;
		switch (word)
		{
			case "thanks": delta = 1; break;
			case "kept": delta = 1; break;
			case "broke": delta = -1; break;
			case "threatened": delta = -1; break;
			case "slander": delta = -1; break;
			case "vouched_liar": delta = -1; break;
			case "lied": delta = -lieWeight; break;
		}

		SetRegard(playerId, speaker, Regard(playerId, speaker) + delta);
	}

	//------------------------------------------------------------------------------------------------
	void SetRegard(int playerId, string speaker, int value)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p || speaker.IsEmpty())
			return;

		int v = Math.ClampInt(value, -REGARD_MAX, REGARD_MAX);
		int i = p.regardKeys.Find(speaker);
		if (i < 0)
		{
			p.regardKeys.Insert(speaker);
			p.regardVals.Insert(v);
			p.regardUnix.Insert(RZ_SeasonClock.Now());
		}
		else
		{
			p.regardVals[i] = v;
			p.regardUnix[i] = RZ_SeasonClock.Now();
		}

		m_bDirty = true;
	}

	//------------------------------------------------------------------------------------------------
	//! What regard does to a leader's or wanderer's attitude: regard / 2, toward zero (at most +-2).
	//! Only while Dialogue > Trees is on.
	static int AttitudeFrom(int playerId, string speaker)
	{
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		if (!trees || !trees.IsOn())
			return 0;

		int regard = trees.GetMemory().Regard(playerId, speaker);
		return regard / 2;
	}

	// ================================================================ Failed checks (per player)
	//------------------------------------------------------------------------------------------------
	//! The score a persuade failed at, -100 if it never failed.
	int FailedAt(int playerId, string key)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p)
			return -100;

		int i = p.failKeys.Find(key);
		if (i < 0)
			return -100;

		return p.failScores[i];
	}

	//------------------------------------------------------------------------------------------------
	void NoteFail(int playerId, string key, int score)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p)
			return;

		int i = p.failKeys.Find(key);
		if (i < 0)
		{
			p.failKeys.Insert(key);
			p.failScores.Insert(score);
		}
		else
		{
			p.failScores[i] = score;
		}

		m_bDirty = true;
	}

	// ================================================================ Knowledge (per player)
	//------------------------------------------------------------------------------------------------
	bool Knows(int playerId, string key)
	{
		RZ_DMemPlayer p = For(playerId);
		return p && p.knows.Find(key) >= 0;
	}

	//------------------------------------------------------------------------------------------------
	void Learn(int playerId, string key)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p || key.IsEmpty() || p.knows.Find(key) >= 0)
			return;

		if (p.knows.Count() >= KNOWS_MAX)
			p.knows.Remove(0);

		p.knows.Insert(key);
		m_bDirty = true;
	}

	// ================================================================ Talk Favor (per player)
	//------------------------------------------------------------------------------------------------
	//! How much of `amount` talk may still pay with `faction` today (Dialogue > Talk Favor a day),
	//! and books it.
	int TakeTalkFavor(int playerId, int faction, int amount, int perDay)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p || !RZ_Factions.IsValid(faction) || amount <= 0)
			return 0;

		int today = RZ_SeasonClock.Now() / DAY;
		if (p.favorDay != today)
		{
			p.favorDay = today;
			for (int f = 0; f < p.favorToday.Count(); f++)
			{
				p.favorToday[f] = 0;
			}
		}

		int room = Math.Max(0, perDay - p.favorToday[faction]);
		int paid = Math.Min(room, amount);
		p.favorToday[faction] = p.favorToday[faction] + paid;
		m_bDirty = true;
		return paid;
	}

	// ================================================================ Per character
	//------------------------------------------------------------------------------------------------
	bool HasFace(int playerId, int faction)
	{
		RZ_DMemPlayer p = For(playerId);
		return p && RZ_Factions.IsValid(faction) && (p.faces & RZ_Factions.Bit(faction)) != 0;
	}

	//------------------------------------------------------------------------------------------------
	void NoteFace(int playerId, int faction)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p || !RZ_Factions.IsValid(faction) || (p.faces & RZ_Factions.Bit(faction)) != 0)
			return;

		p.faces = p.faces | RZ_Factions.Bit(faction);
		m_bDirty = true;
	}

	//------------------------------------------------------------------------------------------------
	bool Has(int playerId, int which, string key)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p)
			return false;

		return ListOf(p, which).Find(key) >= 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Adds a key to a per-character list (SEEN, PASSED, CLAIMS, NOTES).
	void Add(int playerId, int which, string key)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p || key.IsEmpty())
			return;

		array<string> list = ListOf(p, which);
		if (list.Find(key) >= 0)
			return;

		if (which == CLAIMS)
			TrimClaims(list, key);
		else if (list.Count() >= SEEN_MAX)
			list.Remove(0);

		list.Insert(key);
		m_bDirty = true;
	}

	static const int SEEN = 0;
	static const int PASSED = 1;
	static const int CLAIMS = 2;
	static const int NOTES = 3;

	//------------------------------------------------------------------------------------------------
	protected array<string> ListOf(RZ_DMemPlayer p, int which)
	{
		switch (which)
		{
			case PASSED: return p.passed;
			case CLAIMS: return p.claims;
			case NOTES: return p.notes;
		}

		return p.seen;
	}

	//------------------------------------------------------------------------------------------------
	//! At most 6 claims a speaker: the oldest goes.
	protected void TrimClaims(array<string> list, string key)
	{
		int bar = key.IndexOf("|");
		if (bar < 0)
			return;

		string speaker = key.Substring(0, bar + 1);
		int count = 0;
		int oldest = -1;
		foreach (int i, string k : list)
		{
			if (!k.StartsWith(speaker))
				continue;

			count++;
			if (oldest < 0)
				oldest = i;
		}

		if (count >= CLAIMS_PER_SPEAKER && oldest >= 0)
			list.Remove(oldest);
	}

	//------------------------------------------------------------------------------------------------
	//! When a once or daily topic was last taken at this speaker (unix), 0 never.
	int TopicAt(int playerId, string key)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p)
			return 0;

		int i = p.topicKeys.Find(key);
		if (i < 0)
			return 0;

		return p.topicUnix[i];
	}

	//------------------------------------------------------------------------------------------------
	void NoteTopic(int playerId, string key)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p)
			return;

		int i = p.topicKeys.Find(key);
		if (i < 0)
		{
			p.topicKeys.Insert(key);
			p.topicUnix.Insert(RZ_SeasonClock.Now());
		}
		else
		{
			p.topicUnix[i] = RZ_SeasonClock.Now();
		}

		m_bDirty = true;
	}

	//------------------------------------------------------------------------------------------------
	int CharFlag(int playerId, string key)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p)
			return 0;

		int i = p.cflagKeys.Find(key);
		if (i < 0)
			return 0;

		return p.cflagVals[i];
	}

	//------------------------------------------------------------------------------------------------
	void SetCharFlag(int playerId, string key, int value)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p || key.IsEmpty())
			return;

		int i = p.cflagKeys.Find(key);
		if (i < 0)
		{
			p.cflagKeys.Insert(key);
			p.cflagVals.Insert(value);
		}
		else
		{
			p.cflagVals[i] = value;
		}

		m_bDirty = true;
	}

	//------------------------------------------------------------------------------------------------
	//! A local deed at a seat, this life ("fixed the pump"): the latest replaces the last.
	void SetDeed(int playerId, string seat, string phrase)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p)
			return;

		int i = p.deedSeats.Find(seat);
		if (i < 0)
		{
			p.deedSeats.Insert(seat);
			p.deedTexts.Insert(phrase);
		}
		else
		{
			p.deedTexts[i] = phrase;
		}

		m_bDirty = true;
	}

	//------------------------------------------------------------------------------------------------
	string GetDeed(int playerId, string seat)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p)
			return "";

		int i = p.deedSeats.Find(seat);
		if (i < 0 || i >= p.deedTexts.Count())
			return "";

		return p.deedTexts[i];
	}

	//------------------------------------------------------------------------------------------------
	//! "/talkdebug": a short read-out of one player's memory.
	string Describe(int playerId)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p)
			return "no memory";

		return string.Format("regard at %1 speakers, %2 failed checks, %3 known, %4 seen nodes, %5 claims, faces %6",
			p.regardKeys.Count(), p.failKeys.Count(), p.knows.Count(), p.seen.Count(), p.claims.Count(), p.faces);
	}
}
