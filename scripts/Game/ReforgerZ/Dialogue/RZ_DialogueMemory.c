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
	ref array<string> heard = {};		// lies this player was told (heard_lie:<key>)
	// Promises (promise:<key>): kept by keep:<key>, or by talking to the same speaker for a .visit
	ref array<string> promKeys = {};
	ref array<string> promSpeaker = {};	// the regard key that was promised
	ref array<string> promText = {};	// what was promised, in a few words
	ref array<float> promLeft = {};		// seconds of the promiser's own play left
	ref array<int> promUnix = {};		// made, or settled (kept or broken)
	ref array<int> promState = {};		// 0 open, 1 kept, 2 broken
	// Callbacks (later:<hours>h:<key>): due:<key> once that much play has passed
	ref array<string> laterKeys = {};
	ref array<float> laterLeft = {};
	ref array<int> laterUnix = {};
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
	static const int HEARD_MAX = 64;
	static const int PROMISES_OPEN = 3;
	static const int PROMISE_REAL_DAYS = 3;		// a promise breaks after this many real days whatever the play
	static const int KEEP_DAYS = 3;				// settled promises and callbacks are kept this long (real days)
	static const int LATER_MAX = 10;
	static const int OPEN = 0;
	static const int KEPT = 1;
	static const int BROKEN = 2;

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
		if (!p.heard)
			p.heard = new array<string>();
		RepairPromises(p);
	}

	//------------------------------------------------------------------------------------------------
	//! Promise and callback arrays that don't line up (a hand-edited file) start empty.
	protected void RepairPromises(RZ_DMemPlayer p)
	{
		bool bad = !p.promKeys || !p.promSpeaker || !p.promText || !p.promLeft || !p.promUnix || !p.promState;
		if (!bad)
		{
			int n = p.promKeys.Count();
			bad = p.promSpeaker.Count() != n || p.promText.Count() != n || p.promLeft.Count() != n || p.promUnix.Count() != n || p.promState.Count() != n;
		}

		if (bad)
		{
			p.promKeys = new array<string>();
			p.promSpeaker = new array<string>();
			p.promText = new array<string>();
			p.promLeft = new array<float>();
			p.promUnix = new array<int>();
			p.promState = new array<int>();
		}

		bool badLater = !p.laterKeys || !p.laterLeft || !p.laterUnix;
		if (!badLater)
			badLater = p.laterLeft.Count() != p.laterKeys.Count() || p.laterUnix.Count() != p.laterKeys.Count();

		if (badLater)
		{
			p.laterKeys = new array<string>();
			p.laterLeft = new array<float>();
			p.laterUnix = new array<int>();
		}
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
			BreakOpen(playerId, p, "You died with it unkept.");
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

		// A lie caught or found out: the speaker's next greeting can say so (nmem:lied)
		if (word == "lied")
			Add(playerId, NOTES, speaker + "|lied");
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
	//! Any key in a per-character list starting with `prefix` (a claim already told this speaker).
	bool HasPrefix(int playerId, int which, string prefix)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p)
			return false;

		foreach (string key : ListOf(p, which))
		{
			if (key.StartsWith(prefix))
				return true;
		}

		return false;
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

	// ================================================================ Lies told to the player (per player)
	//------------------------------------------------------------------------------------------------
	bool HeardLie(int playerId, string key)
	{
		RZ_DMemPlayer p = For(playerId);
		return p && p.heard.Find(key) >= 0;
	}

	//------------------------------------------------------------------------------------------------
	void NoteHeardLie(int playerId, string key)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p || key.IsEmpty() || p.heard.Find(key) >= 0)
			return;

		if (p.heard.Count() >= HEARD_MAX)
			p.heard.Remove(0);

		p.heard.Insert(key);
		m_bDirty = true;
	}

	// ================================================================ Promises (per player)
	//------------------------------------------------------------------------------------------------
	//! A promise to `speaker`: `hours` of this player's own play to keep it, and at most 3 real days.
	//! "" if made (or already open), else why not (3 open already).
	string Promise(int playerId, string key, string speaker, string text, float hours)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p || key.IsEmpty())
			return "";

		int i = p.promKeys.Find(key);
		if (i >= 0 && p.promState[i] == OPEN)
			return "";

		if (CountOpen(p) >= PROMISES_OPEN)
			return "You've three promises open already. Keep one first.";

		if (i >= 0)
			RemovePromise(p, i);

		p.promKeys.Insert(key);
		p.promSpeaker.Insert(speaker);
		p.promText.Insert(text);
		p.promLeft.Insert(Math.Max(0.1, hours) * 3600);
		p.promUnix.Insert(RZ_SeasonClock.Now());
		p.promState.Insert(OPEN);
		m_bDirty = true;
		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected int CountOpen(RZ_DMemPlayer p)
	{
		int n = 0;
		foreach (int state : p.promState)
		{
			if (state == OPEN)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected void RemovePromise(RZ_DMemPlayer p, int i)
	{
		p.promKeys.Remove(i);
		p.promSpeaker.Remove(i);
		p.promText.Remove(i);
		p.promLeft.Remove(i);
		p.promUnix.Remove(i);
		p.promState.Remove(i);
	}

	//------------------------------------------------------------------------------------------------
	//! OPEN, KEPT or BROKEN; -1 if there's no such promise (or it settled more than 3 real days ago).
	int PromiseState(int playerId, string key)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p)
			return -1;

		int i = p.promKeys.Find(key);
		if (i < 0)
			return -1;

		return p.promState[i];
	}

	//------------------------------------------------------------------------------------------------
	//! keep:<key>: an open promise is kept (regard *kept* with whoever it was made to). True if one was.
	bool Keep(int playerId, string key)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p)
			return false;

		int i = p.promKeys.Find(key);
		if (i < 0 || p.promState[i] != OPEN)
			return false;

		Settle(playerId, p, i, KEPT, "");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A visit promise (a key ending ".visit") to this speaker is kept by talking to them.
	void KeepVisits(int playerId, string speaker)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p)
			return;

		for (int i = p.promKeys.Count() - 1; i >= 0; i--)
		{
			if (p.promState[i] == OPEN && p.promSpeaker[i] == speaker && p.promKeys[i].EndsWith(".visit"))
				Settle(playerId, p, i, KEPT, "");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! What's still open with this speaker, one line each; every open promise when `speaker` is empty.
	void OpenLines(int playerId, string speaker, notnull array<string> lines)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p)
			return;

		foreach (int i, string key : p.promKeys)
		{
			if (p.promState[i] != OPEN || (!speaker.IsEmpty() && p.promSpeaker[i] != speaker))
				continue;

			lines.Insert(string.Format("(You promised %1. %2 of play left.)", p.promText[i], PlayLeft(p.promLeft[i])));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static string PlayLeft(float secs)
	{
		int mins = Math.Max(1, RZ_Util.RoundToInt(secs / 60));
		if (mins < 60)
			return string.Format("%1 min", mins);

		return string.Format("%1 h %2 min", mins / 60, mins % 60);
	}

	//------------------------------------------------------------------------------------------------
	protected void Settle(int playerId, RZ_DMemPlayer p, int i, int state, string why)
	{
		p.promState[i] = state;
		p.promUnix[i] = RZ_SeasonClock.Now();
		m_bDirty = true;
		string speaker = p.promSpeaker[i];
		if (state == KEPT)
		{
			RegardRaw(p, speaker, 1);
			m_GM.NotifyPlayer(playerId, "A promise", string.Format("Kept: you promised %1.", p.promText[i]));
			RZ_Log.Info(string.Format("Dialogue: promise %1 kept by %2", p.promKeys[i], RZ_Util.PlayerName(playerId)));
			return;
		}

		RegardRaw(p, speaker, -1);
		m_GM.NotifyPlayer(playerId, "A promise", string.Format("Broken: you promised %1. %2", p.promText[i], why));
		RZ_Log.Info(string.Format("Dialogue: promise %1 broken by %2", p.promKeys[i], RZ_Util.PlayerName(playerId)));
	}

	//------------------------------------------------------------------------------------------------
	//! A new life breaks every open promise (regard *broke*).
	protected void BreakOpen(int playerId, RZ_DMemPlayer p, string why)
	{
		for (int i = 0; i < p.promKeys.Count(); i++)
		{
			if (p.promState[i] == OPEN)
				Settle(playerId, p, i, BROKEN, why);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Regard moved by `delta` on the record itself (kept +1, broke -1), with no life check.
	protected void RegardRaw(RZ_DMemPlayer p, string speaker, int delta)
	{
		if (speaker.IsEmpty())
			return;

		int i = p.regardKeys.Find(speaker);
		if (i < 0)
		{
			p.regardKeys.Insert(speaker);
			p.regardVals.Insert(Math.ClampInt(delta, -REGARD_MAX, REGARD_MAX));
			p.regardUnix.Insert(RZ_SeasonClock.Now());
			return;
		}

		p.regardVals[i] = Math.ClampInt(Decayed(p.regardVals[i], p.regardUnix[i]) + delta, -REGARD_MAX, REGARD_MAX);
		p.regardUnix[i] = RZ_SeasonClock.Now();
	}

	// ================================================================ Callbacks (per player)
	//------------------------------------------------------------------------------------------------
	//! later:<hours>h:<key>: due:<key> reads true once this player has played `hours` more. Words
	//! only (a reaction topic, a greet, a bark). At most 10 a player: the oldest goes.
	void Later(int playerId, string key, float hours)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p || key.IsEmpty())
			return;

		int i = p.laterKeys.Find(key);
		if (i >= 0)
			RemoveLater(p, i);

		if (p.laterKeys.Count() >= LATER_MAX)
			RemoveLater(p, 0);

		p.laterKeys.Insert(key);
		p.laterLeft.Insert(Math.Max(0, hours) * 3600);
		p.laterUnix.Insert(RZ_SeasonClock.Now());
		m_bDirty = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveLater(RZ_DMemPlayer p, int i)
	{
		p.laterKeys.Remove(i);
		p.laterLeft.Remove(i);
		p.laterUnix.Remove(i);
	}

	//------------------------------------------------------------------------------------------------
	bool Due(int playerId, string key)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p)
			return false;

		int i = p.laterKeys.Find(key);
		return i >= 0 && p.laterLeft[i] <= 0;
	}

	//------------------------------------------------------------------------------------------------
	//! On the server tick, for a player online: promise clocks and callbacks run on their own play;
	//! settled promises and callbacks go after 3 real days.
	void TickPlayer(int playerId, float dt)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		if (!profile || profile.m_sIdentity.IsEmpty())
			return;

		RZ_DMemPlayer known;
		if (!m_mPlayers.Find(profile.m_sIdentity, known) || (known.promKeys.IsEmpty() && known.laterKeys.IsEmpty()))
			return;

		RZ_DMemPlayer p = For(playerId);
		int now = RZ_SeasonClock.Now();
		for (int i = p.promKeys.Count() - 1; i >= 0; i--)
		{
			if (p.promState[i] == OPEN)
			{
				p.promLeft[i] = p.promLeft[i] - dt;
				if (p.promLeft[i] <= 0)
					Settle(playerId, p, i, BROKEN, "Time ran out.");
				else if (now - p.promUnix[i] >= PROMISE_REAL_DAYS * DAY)
					Settle(playerId, p, i, BROKEN, "Three days went by.");
			}
			else if (now - p.promUnix[i] >= KEEP_DAYS * DAY)
			{
				RemovePromise(p, i);
				m_bDirty = true;
			}
		}

		for (int k = p.laterKeys.Count() - 1; k >= 0; k--)
		{
			if (now - p.laterUnix[k] >= KEEP_DAYS * DAY)
			{
				RemoveLater(p, k);
				m_bDirty = true;
			}
			else if (p.laterLeft[k] > 0)
			{
				p.laterLeft[k] = Math.Max(0, p.laterLeft[k] - dt);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "/talkdebug": a short read-out of one player's memory.
	string Describe(int playerId)
	{
		RZ_DMemPlayer p = For(playerId);
		if (!p)
			return "no memory";

		return string.Format("regard at %1 speakers, %2 failed checks, %3 known, %4 seen nodes, %5 claims, faces %6, %7 promises, %8 callbacks, %9 lies heard",
			p.regardKeys.Count(), p.failKeys.Count(), p.knows.Count(), p.seen.Count(), p.claims.Count(), p.faces, p.promKeys.Count(), p.laterKeys.Count(), p.heard.Count());
	}
}
