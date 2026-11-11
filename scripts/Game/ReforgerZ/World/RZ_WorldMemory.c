//! World memory (docs/DESIGN.md, "World memory"): what the Zone remembers, in the world facts, on
//! four timescales. Missions, errands and the epilogue read it; the systems that own the facts
//! write it. Saved with the world facts, wiped with the season.
//!
//! - Reasons: live facts with a subject and a place ("house.hungry" at Levie), each for at most a
//!   season day, or until cleared. Why a job is offered now.
//! - The signal ring: recent events (a shot near a farm, a body on a road) with a quota per kind
//!   and a cause that is server-only (never shown).
//! - The town log: per settlement the best and the worst thing that happened there (a higher
//!   salience replaces; ties keep the older), with up to 2 names frozen at the deed; and up to 16
//!   identities who did something there this season.
//! - World records: one per story key and subject, 160 in all, evicted by importance (lowest, then
//!   oldest), each with an optional epilogue line and its carrier.
//! - The truth deck: what is really going on behind a story job or a card, per story key and
//!   subject, drawn once from the season's seed and kept 3 season days or until used up.
//! The systems that own the facts write them (docs/DESIGN.md, "World memory": the writers).
//! Server only.

//------------------------------------------------------------------------------------------------
class RZ_Reason
{
	string key;			// "house.hungry"
	string subject;		// "safehouse:2", "town:7"
	string text;		// the why-now line: "Levie's on half portions."
	float x;
	float z;
	int faction = -1;
	float madeDay;		// season days (RZ_SeasonClock.Days)
	float untilDay;
}

//------------------------------------------------------------------------------------------------
class RZ_Signal
{
	string kind;		// "shot", "body", "horde"
	string text;
	float x;
	float z;
	string cause;		// server-only: never shown to a player
	float day;
}

//------------------------------------------------------------------------------------------------
class RZ_TownEntry
{
	string kind;		// "rescue", "burned"
	string text;
	int salience;		// 1-5
	float day;
	ref array<string> idents = {};
	ref array<string> names = {};	// frozen at the deed; "" = written unnamed
}

//------------------------------------------------------------------------------------------------
class RZ_TownLog
{
	int settlement = -1;
	ref RZ_TownEntry best;
	ref RZ_TownEntry worst;
	ref array<string> visits = {};
}

//------------------------------------------------------------------------------------------------
class RZ_WorldRecord
{
	string key;			// "<story key>.<what>"
	string subject;
	string value;
	float day;
	int base;			// importance without the players: the source, a named person, first of its kind, an epilogue line
	int importance;		// base + 1 a player (3 at most)
	string epilogue;	// an epilogue line, "" for none
	string carrier;		// "board", "voice", "radio", "prop", "headline", "chalk"
	ref array<string> idents = {};
	ref array<string> names = {};
}

//------------------------------------------------------------------------------------------------
//! One drawn truth: what is really going on with `subject` in the story `key`.
class RZ_Truth
{
	string key;
	string subject;
	string value;		// the drawn truth ("bite", "flu", "as_said")
	float day;			// season day it was drawn
	ref array<string> found = {};	// how it has been found ("clue", "nurse", "talk"), for the journal
}

//------------------------------------------------------------------------------------------------
class RZ_WorldMemorySave
{
	ref array<ref RZ_Truth> truths = {};
	ref array<ref RZ_Reason> reasons = {};
	ref array<ref RZ_Signal> signals = {};
	ref array<ref RZ_TownLog> towns = {};
	ref array<ref RZ_WorldRecord> records = {};
	ref array<string> kindsSeen = {};
	ref array<ref RZ_Lie> lies = {};
}

//------------------------------------------------------------------------------------------------
class RZ_WorldMemory
{
	static const int MAX_REASONS = 32;
	static const float REASON_DAYS = 1;
	static const int RING = 48;
	static const int RING_PER_KIND = 8;
	static const int VISITS = 16;
	static const int NAMES_PER_ENTRY = 2;
	static const int NAMED_ENTRIES = 6;
	static const int MAX_RECORDS = 160;
	static const int RECORD_PLAYERS = 4;
	static const int MAX_TRUTHS = 64;
	static const float TRUTH_DAYS = 3;
	//! The truth "it's as they said" is never drawn below this share of the deck.
	static const float AS_SAID_FLOOR = 0.3;

	//! Importance: where a record comes from
	static const int SOURCE_STORY = 3;
	static const int SOURCE_CHOICE = 2;
	static const int SOURCE_ERRAND = 1;

	protected ref array<ref RZ_Reason> m_aReasons = {};
	protected ref array<ref RZ_Signal> m_aSignals = {};
	protected ref map<int, ref RZ_TownLog> m_mTowns = new map<int, ref RZ_TownLog>();
	protected ref array<ref RZ_WorldRecord> m_aRecords = {};
	protected ref array<ref RZ_Truth> m_aTruths = {};
	protected ref array<string> m_aKindsSeen = {};		// record keys written this season (first of its kind)
	protected ref map<string, int> m_mPins = new map<string, int>();	// "key|subject" -> readers holding it (runtime)
	protected ref RZ_Lies m_Lies = new RZ_Lies();

	//------------------------------------------------------------------------------------------------
	//! The one lie store (World/RZ_Lies.c).
	RZ_Lies Lies()
	{
		return m_Lies;
	}

	// ================================================================ Reasons
	//------------------------------------------------------------------------------------------------
	//! Post a reason: `key` about `subject` at `pos`, for `days` season days (at most one). A reason
	//! with the same key and subject is replaced. Past 32 the one that ends soonest goes.
	void PostReason(string key, string subject, string text, vector pos, int faction = -1, float days = 1)
	{
		if (key.IsEmpty())
			return;

		ClearReason(key, subject);
		RZ_Reason r = new RZ_Reason();
		r.key = key;
		r.subject = subject;
		r.text = text;
		r.x = pos[0];
		r.z = pos[2];
		r.faction = faction;
		r.madeDay = RZ_SeasonClock.Days();
		r.untilDay = r.madeDay + Math.Clamp(days, 0.01, REASON_DAYS);
		if (m_aReasons.Count() >= MAX_REASONS)
		{
			int soonest = 0;
			for (int i = 1; i < m_aReasons.Count(); i++)
			{
				if (m_aReasons[i].untilDay < m_aReasons[soonest].untilDay)
					soonest = i;
			}

			m_aReasons.Remove(soonest);
		}

		m_aReasons.Insert(r);
		RZ_Log.Info(string.Format("World memory: reason %1 (%2) %3", key, subject, text));
	}

	//------------------------------------------------------------------------------------------------
	//! The fact stopped holding: the reason goes. subject "" clears every subject of `key`.
	void ClearReason(string key, string subject)
	{
		for (int i = m_aReasons.Count() - 1; i >= 0; i--)
		{
			RZ_Reason r = m_aReasons[i];
			if (r.key == key && (subject.IsEmpty() || r.subject == subject))
				m_aReasons.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	RZ_Reason FindReason(string key, string subject = "")
	{
		foreach (RZ_Reason r : m_aReasons)
		{
			if (r.key == key && (subject.IsEmpty() || r.subject == subject))
				return r;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Reasons within `radius` of `pos` (key "" = any).
	void ReasonsNear(vector pos, float radius, notnull array<RZ_Reason> result, string key = "")
	{
		foreach (RZ_Reason r : m_aReasons)
		{
			if (!key.IsEmpty() && r.key != key)
				continue;

			if (RZ_Util.DistXZ(pos, Vector(r.x, 0, r.z)) <= radius)
				result.Insert(r);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every live reason, oldest first.
	void AllReasons(notnull array<RZ_Reason> result)
	{
		foreach (RZ_Reason r : m_aReasons)
		{
			result.Insert(r);
		}
	}

	//------------------------------------------------------------------------------------------------
	int ReasonCount()
	{
		return m_aReasons.Count();
	}

	// ================================================================ The signal ring
	//------------------------------------------------------------------------------------------------
	//! A recent event. At most 8 of one kind (its oldest goes), 48 in all (the oldest goes).
	//! `cause` stays on the server: who or what did it, never shown.
	void Signal(string kind, vector pos, string text, string cause = "")
	{
		if (kind.IsEmpty())
			return;

		int ofKind = 0;
		int oldestOfKind = -1;
		foreach (int i, RZ_Signal s : m_aSignals)
		{
			if (s.kind != kind)
				continue;

			ofKind++;
			if (oldestOfKind < 0)
				oldestOfKind = i;
		}

		if (ofKind >= RING_PER_KIND && oldestOfKind >= 0)
			m_aSignals.RemoveOrdered(oldestOfKind);
		else if (m_aSignals.Count() >= RING)
			m_aSignals.RemoveOrdered(0);

		RZ_Signal sig = new RZ_Signal();
		sig.kind = kind;
		sig.text = text;
		sig.x = pos[0];
		sig.z = pos[2];
		sig.cause = cause;
		sig.day = RZ_SeasonClock.Days();
		m_aSignals.Insert(sig);
	}

	//------------------------------------------------------------------------------------------------
	//! Signals of `kind` ("" any) within `radius` of `pos` and at most `maxDays` season days old,
	//! newest first.
	void RecentSignals(string kind, vector pos, float radius, float maxDays, notnull array<RZ_Signal> result)
	{
		float now = RZ_SeasonClock.Days();
		for (int i = m_aSignals.Count() - 1; i >= 0; i--)
		{
			RZ_Signal s = m_aSignals[i];
			if (!kind.IsEmpty() && s.kind != kind)
				continue;

			if (now - s.day > maxDays || RZ_Util.DistXZ(pos, Vector(s.x, 0, s.z)) > radius)
				continue;

			result.Insert(s);
		}
	}

	// ================================================================ The town log
	//------------------------------------------------------------------------------------------------
	protected RZ_TownLog Town(int settlement, bool create)
	{
		RZ_TownLog t;
		if (m_mTowns.Find(settlement, t) || !create)
			return t;

		t = new RZ_TownLog();
		t.settlement = settlement;
		m_mTowns.Set(settlement, t);
		return t;
	}

	//------------------------------------------------------------------------------------------------
	//! Something happened in town `settlement`: the best slot or the worst. Salience is the kind's
	//! base (1-5) plus 1 when three or more took part (5 at most). It replaces the slot only when
	//! strictly higher; ties keep the older. Up to 2 players are named, each frozen under the name
	//! they play under now; an identity is named in at most one entry per town and 6 in all, after
	//! which it's written unnamed. True if the entry was kept.
	bool LogTown(int settlement, bool best, string kind, int salience, string text, array<int> players = null)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (settlement < 0 || !gm)
			return false;

		int count = 0;
		if (players)
			count = players.Count();

		int score = Math.ClampInt(salience, 1, 5);
		if (count >= 3)
			score = Math.Min(5, score + 1);

		RZ_TownLog t = Town(settlement, true);
		RZ_TownEntry old = t.worst;
		if (best)
			old = t.best;

		if (old && old.salience >= score)
			return false;

		RZ_TownEntry e = new RZ_TownEntry();
		e.kind = kind;
		e.text = text;
		e.salience = score;
		e.day = RZ_SeasonClock.Days();
		if (players)
		{
			foreach (int playerId : players)
			{
				string ident = gm.GetProfile(playerId).m_sIdentity;
				if (ident.IsEmpty() || e.idents.Contains(ident))
					continue;

				e.idents.Insert(ident);
				string name = "";
				if (e.names.Count() < NAMES_PER_ENTRY && MayName(t, ident))
					name = RZ_Util.PlayerName(playerId);

				e.names.Insert(name);
			}
		}

		if (best)
			t.best = e;
		else
			t.worst = e;

		RZ_Log.Info(string.Format("World memory: town %1 %2 %3 (salience %4)", settlement, kind, text, score));
		PlaceSeasonMark(settlement, best, e);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The town log's season mark (World/RZ_Marks.c, the season pool): an entry of salience 3 or
	//! more marks its town, on the verge of the road into it, with the entry's line. The prop is
	//! Shared > Season mark prefab (optional; without it the mark is text only, an approach line).
	//! Marks aren't saved: PlaceSeasonMarks puts them back after a restart.
	protected void PlaceSeasonMark(int settlement, bool best, RZ_TownEntry e)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		RZ_Marks marks = RZ_Marks.Get();
		if (!gm || !marks || !e)
			return;

		string key = string.Format("town.%1.%2", settlement, best);
		if (e.salience < 3)
		{
			marks.Remove(key);
			return;
		}

		RZ_Settlement s = gm.GetSettlement(settlement);
		if (!s)
			return;

		vector at = RZ_Util.GroundPos(s.m_vCenter + Vector(s.m_fRadius * 0.6, 0, 0));
		marks.Place(RZ_EMarkPool.SEASON, key, at, RZ_SharedSettings.Get().m_sSeasonMarkPrefab, e.text, e.salience, -1, e.salience >= 5);
	}

	//------------------------------------------------------------------------------------------------
	//! After a restart (marks aren't saved): every kept town entry puts its season mark back.
	void PlaceSeasonMarks()
	{
		foreach (int idx, RZ_TownLog t : m_mTowns)
		{
			if (t.best)
				PlaceSeasonMark(idx, true, t.best);

			if (t.worst)
				PlaceSeasonMark(idx, false, t.worst);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The naming rule: not already named in this town, and named in fewer than 6 entries in all.
	protected bool MayName(RZ_TownLog here, string ident)
	{
		int named = 0;
		foreach (int idx, RZ_TownLog t : m_mTowns)
		{
			int inTown = NamedIn(t.best, ident) + NamedIn(t.worst, ident);
			if (t == here && inTown > 0)
				return false;

			named += inTown;
		}

		return named < NAMED_ENTRIES;
	}

	//------------------------------------------------------------------------------------------------
	protected static int NamedIn(RZ_TownEntry e, string ident)
	{
		if (!e)
			return 0;

		int i = e.idents.Find(ident);
		if (i < 0 || i >= e.names.Count() || e.names[i].IsEmpty())
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! `ident` did something in town `settlement` (searched, fought, handed something in, finished a
	//! mission step, talked to someone). Up to 16 per town; standing about doesn't count.
	void Visit(int settlement, string ident)
	{
		if (settlement < 0 || ident.IsEmpty())
			return;

		RZ_TownLog t = Town(settlement, true);
		if (t.visits.Contains(ident) || t.visits.Count() >= VISITS)
			return;

		t.visits.Insert(ident);
	}

	//------------------------------------------------------------------------------------------------
	RZ_TownEntry TownBest(int settlement)
	{
		RZ_TownLog t = Town(settlement, false);
		if (!t)
			return null;

		return t.best;
	}

	//------------------------------------------------------------------------------------------------
	RZ_TownEntry TownWorst(int settlement)
	{
		RZ_TownLog t = Town(settlement, false);
		if (!t)
			return null;

		return t.worst;
	}

	//------------------------------------------------------------------------------------------------
	bool Visited(int settlement, string ident)
	{
		RZ_TownLog t = Town(settlement, false);
		if (!t)
			return false;

		return t.visits.Contains(ident);
	}

	// ================================================================ World records
	//------------------------------------------------------------------------------------------------
	//! One record per key and subject: a repeat overwrites the value and the day and adds players
	//! (4 at most). Importance: the source (RZ_WorldMemory.SOURCE_*), +2 for a named person, +2 for
	//! the first of its kind this season, +1 per player (3 at most), +1 for an epilogue line. Past
	//! 160 the lowest importance goes, then the oldest, never one a reader holds (Pin).
	RZ_WorldRecord Record(string key, string subject, string value, int source, bool namedPerson, array<int> players = null, string epilogue = "", string carrier = "")
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (key.IsEmpty() || !gm)
			return null;

		RZ_WorldRecord rec = FindRecord(key, subject);
		bool fresh = !rec;
		if (fresh)
		{
			rec = new RZ_WorldRecord();
			rec.key = key;
			rec.subject = subject;
		}

		rec.value = value;
		rec.day = RZ_SeasonClock.Days();
		if (!epilogue.IsEmpty())
		{
			if (!fresh && rec.epilogue.IsEmpty())
				rec.base = rec.base + 1;

			rec.epilogue = epilogue;
			rec.carrier = carrier;
		}

		if (players)
		{
			foreach (int playerId : players)
			{
				string ident = gm.GetProfile(playerId).m_sIdentity;
				if (ident.IsEmpty() || rec.idents.Contains(ident) || rec.idents.Count() >= RECORD_PLAYERS)
					continue;

				rec.idents.Insert(ident);
				rec.names.Insert(RZ_Util.PlayerName(playerId));
			}
		}

		if (fresh)
		{
			int importance = Math.ClampInt(source, SOURCE_ERRAND, SOURCE_STORY);
			if (namedPerson)
				importance += 2;

			if (!m_aKindsSeen.Contains(key))
			{
				importance += 2;
				m_aKindsSeen.Insert(key);
			}

			if (!rec.epilogue.IsEmpty())
				importance++;

			rec.base = importance;
			rec.importance = importance;
			if (m_aRecords.Count() >= MAX_RECORDS && !Evict())
				return null;

			m_aRecords.Insert(rec);
		}

		rec.importance = rec.base + Math.Min(3, rec.idents.Count());
		return rec;
	}

	//------------------------------------------------------------------------------------------------
	//! Frees one slot: the lowest importance, then the oldest, not pinned. False if all are pinned.
	protected bool Evict()
	{
		int drop = -1;
		foreach (int i, RZ_WorldRecord r : m_aRecords)
		{
			if (m_mPins.Contains(r.key + "|" + r.subject))
				continue;

			if (drop < 0 || r.importance < m_aRecords[drop].importance || (r.importance == m_aRecords[drop].importance && r.day < m_aRecords[drop].day))
				drop = i;
		}

		if (drop < 0)
			return false;

		m_aRecords.Remove(drop);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	RZ_WorldRecord FindRecord(string key, string subject)
	{
		foreach (RZ_WorldRecord r : m_aRecords)
		{
			if (r.key == key && r.subject == subject)
				return r;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! A reader (a story job's check) holds a record so it's never evicted while it reads it.
	void Pin(string key, string subject, bool hold)
	{
		string k = key + "|" + subject;
		int n;
		m_mPins.Find(k, n);
		if (hold)
			n++;
		else
			n--;

		if (n <= 0)
			m_mPins.Remove(k);
		else
			m_mPins.Set(k, n);
	}

	//------------------------------------------------------------------------------------------------
	int RecordCount()
	{
		return m_aRecords.Count();
	}

	// ================================================================ The truth deck
	//------------------------------------------------------------------------------------------------
	//! The truth behind `key` at `subject`: drawn once, weighted, from the season's seed, the key and
	//! the subject, so a job offered again at the same subject (after a restart or a time-out) gets the
	//! same truth. values[0] is "it's as they said" and never weighs under 30% of the deck. Kept 3
	//! season days or until UseTruth. Gates are the caller's: pass only the truths whose gate holds.
	string DrawTruth(string key, string subject, notnull array<string> values, notnull array<float> weights)
	{
		RZ_Truth kept = FindTruth(key, subject);
		if (kept)
		{
			RZ_Log.Info(string.Format("Story: truth %1.%2 = %3 (kept)", key, subject, kept.value));
			return kept.value;
		}

		if (values.IsEmpty())
			return "";

		float total = 0;
		for (int i = 0; i < values.Count(); i++)
		{
			if (i < weights.Count())
				total += Math.Max(0, weights[i]);
		}

		float first = 0;
		if (!weights.IsEmpty())
			first = Math.Max(0, weights[0]);

		// The floor for "as they said": raise its weight until it's 30% of the deck
		float rest = total - first;
		if (rest > 0 && first < AS_SAID_FLOOR * (first + rest))
			first = rest * AS_SAID_FLOOR / (1 - AS_SAID_FLOOR);

		total = first + rest;
		RZ_Rng rng = new RZ_Rng(RZ_WorldFacts.Get().GetSeed() + key.Hash() * 31 + subject.Hash());
		float roll = rng.Float01() * total;
		string pick = values[values.Count() - 1];
		for (int v = 0; v < values.Count(); v++)
		{
			float w = first;
			if (v > 0 && v < weights.Count())
				w = Math.Max(0, weights[v]);
			else if (v > 0)
				w = 0;

			roll -= w;
			if (roll <= 0)
			{
				pick = values[v];
				break;
			}
		}

		RZ_Truth t = new RZ_Truth();
		t.key = key;
		t.subject = subject;
		t.value = pick;
		t.day = RZ_SeasonClock.Days();
		if (m_aTruths.Count() >= MAX_TRUTHS)
			m_aTruths.RemoveOrdered(0);

		m_aTruths.Insert(t);
		RZ_Log.Info(string.Format("Story: truth %1.%2 = %3 (new)", key, subject, pick));
		return pick;
	}

	//------------------------------------------------------------------------------------------------
	RZ_Truth FindTruth(string key, string subject)
	{
		foreach (RZ_Truth t : m_aTruths)
		{
			if (t.key == key && t.subject == subject)
				return t;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The truth was found (a clue, a background read, a talk check, a third party): noted once per
	//! way, for the journal's "What you've found". Logged.
	void FoundTruth(string key, string subject, string how)
	{
		RZ_Truth t = FindTruth(key, subject);
		if (!t)
			return;

		if (!t.found.Contains(how))
			t.found.Insert(how);

		RZ_Log.Info(string.Format("Story: truth %1.%2 = %3; found by %4", key, subject, t.value, how));
	}

	//------------------------------------------------------------------------------------------------
	//! The job used it up, or its subject is gone: the next job there draws again.
	void UseTruth(string key, string subject)
	{
		for (int i = m_aTruths.Count() - 1; i >= 0; i--)
		{
			if (m_aTruths[i].key == key && m_aTruths[i].subject == subject)
				m_aTruths.RemoveOrdered(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	int TruthCount()
	{
		return m_aTruths.Count();
	}

	// ================================================================ Tick and save
	//------------------------------------------------------------------------------------------------
	//! Reasons past their season day go.
	void Tick()
	{
		float now = RZ_SeasonClock.Days();
		for (int i = m_aReasons.Count() - 1; i >= 0; i--)
		{
			if (m_aReasons[i].untilDay <= now)
				m_aReasons.Remove(i);
		}

		for (int t = m_aTruths.Count() - 1; t >= 0; t--)
		{
			if (now - m_aTruths[t].day > TRUTH_DAYS)
				m_aTruths.RemoveOrdered(t);
		}
	}

	//------------------------------------------------------------------------------------------------
	RZ_WorldMemorySave Save()
	{
		RZ_WorldMemorySave data = new RZ_WorldMemorySave();
		foreach (RZ_Reason r : m_aReasons)
		{
			data.reasons.Insert(r);
		}

		foreach (RZ_Signal s : m_aSignals)
		{
			data.signals.Insert(s);
		}

		foreach (int idx, RZ_TownLog t : m_mTowns)
		{
			data.towns.Insert(t);
		}

		foreach (RZ_WorldRecord rec : m_aRecords)
		{
			data.records.Insert(rec);
		}

		foreach (RZ_Truth truth : m_aTruths)
		{
			data.truths.Insert(truth);
		}

		data.kindsSeen.Copy(m_aKindsSeen);
		m_Lies.Save(data.lies);
		return data;
	}

	//------------------------------------------------------------------------------------------------
	void Restore(RZ_WorldMemorySave data)
	{
		if (!data)
			return;

		m_Lies.Restore(data.lies);

		if (data.reasons)
		{
			foreach (RZ_Reason r : data.reasons)
			{
				if (r)
					m_aReasons.Insert(r);
			}
		}

		if (data.signals)
		{
			foreach (RZ_Signal s : data.signals)
			{
				if (s)
					m_aSignals.Insert(s);
			}
		}

		if (data.towns)
		{
			foreach (RZ_TownLog t : data.towns)
			{
				if (!t || t.settlement < 0)
					continue;

				if (!t.visits)
					t.visits = new array<string>();

				m_mTowns.Set(t.settlement, t);
			}
		}

		if (data.records)
		{
			foreach (RZ_WorldRecord rec : data.records)
			{
				if (!rec)
					continue;

				if (!rec.idents)
					rec.idents = new array<string>();

				if (!rec.names)
					rec.names = new array<string>();

				if (rec.base <= 0)
					rec.base = Math.Max(1, rec.importance - Math.Min(3, rec.idents.Count()));

				m_aRecords.Insert(rec);
			}
		}

		if (data.kindsSeen)
			m_aKindsSeen.Copy(data.kindsSeen);

		if (data.truths)
		{
			foreach (RZ_Truth truth : data.truths)
			{
				if (!truth)
					continue;

				if (!truth.found)
					truth.found = new array<string>();

				m_aTruths.Insert(truth);
			}
		}

		RZ_Log.Info(string.Format("World memory: %1 reasons, %2 signals, %3 town logs, %4 world records.", m_aReasons.Count(), m_aSignals.Count(), m_mTowns.Count(), m_aRecords.Count()));
	}
}
