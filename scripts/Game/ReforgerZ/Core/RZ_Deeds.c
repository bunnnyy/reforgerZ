//! Deeds (docs/DESIGN.md, "Deeds"): one store, two scopes.
//!
//! - Life (the character, on RZ_PlayerProfile, cleared with a new life): per faction the best deed
//!   (the phrase of the biggest single Fame gain) and the worst (the biggest single direct Infamy
//!   gain), each with its season day; the tallies (that faction's people killed, contracts done for
//!   it); and up to 32 life records ("pressed", "hid_bite"...), bad ones pinned for 3 season days.
//! - Season (by identity, saved with the season, wiped with it): the ledger (what you did, for
//!   whom, against whom), the best and worst reputation cell per faction, the biggest deeds, the
//!   tallies and the Infamy forgiven.
//! A bigger deed replaces a smaller one; an equal one keeps the first. Phrases are lowercase and past
//! tense ("brought medicine to Levie"). Who writes phrases (each contract's m_sDeed, questline
//! endings) comes with reputation's phase 2; the tallies and the cells are written now.
//! Server only.
class RZ_Deeds
{
	static const int LIFE_RECORDS = 32;
	static const float PIN_DAYS = 3;

	// ================================================================ Life scope
	//------------------------------------------------------------------------------------------------
	//! A Fame gain: the life's best deed with `f` if it's the biggest yet (and has a phrase).
	static void NoteBest(RZ_PlayerProfile p, int f, int amount, string phrase)
	{
		if (!p || amount <= 0 || !RZ_Factions.IsValid(f))
			return;

		p.EnsureArrays();
		if (!phrase.IsEmpty() && amount > p.m_aBestAmount[f])
		{
			p.m_aBestDeed[f] = phrase;
			p.m_aBestAmount[f] = amount;
			p.m_aBestDay[f] = RZ_SeasonClock.Day();
		}

		RZ_SeasonDeeds season = Season();
		if (season)
			season.NoteDeed(p.m_sIdentity, f, amount, phrase, true);
	}

	//------------------------------------------------------------------------------------------------
	//! A direct Infamy gain: the life's worst deed with `f` if it's the biggest yet (and has a phrase).
	static void NoteWorst(RZ_PlayerProfile p, int f, int amount, string phrase)
	{
		if (!p || amount <= 0 || !RZ_Factions.IsValid(f))
			return;

		p.EnsureArrays();
		if (!phrase.IsEmpty() && amount > p.m_aWorstAmount[f])
		{
			p.m_aWorstDeed[f] = phrase;
			p.m_aWorstAmount[f] = amount;
			p.m_aWorstDay[f] = RZ_SeasonClock.Day();
		}

		RZ_SeasonDeeds season = Season();
		if (season)
			season.NoteDeed(p.m_sIdentity, f, amount, phrase, false);
	}

	//------------------------------------------------------------------------------------------------
	//! One of `f`'s people killed (retaliation kills, leaders).
	static void AddKill(RZ_GameModeComponent gm, int playerId, int f)
	{
		if (!gm || playerId <= 0 || !RZ_Factions.IsValid(f))
			return;

		RZ_PlayerProfile p = gm.GetProfile(playerId);
		p.EnsureArrays();
		p.m_aKills[f] = p.m_aKills[f] + 1;
		RZ_SeasonDeeds season = Season();
		if (season)
			season.NoteTally(p.m_sIdentity, f, true);
	}

	//------------------------------------------------------------------------------------------------
	//! A contract done for `f`: the tally, and a ledger line (for whom, against whom).
	static void AddJob(RZ_GameModeComponent gm, int playerId, int f, int against, string what)
	{
		if (!gm || playerId <= 0 || !RZ_Factions.IsValid(f))
			return;

		RZ_PlayerProfile p = gm.GetProfile(playerId);
		p.EnsureArrays();
		p.m_aJobs[f] = p.m_aJobs[f] + 1;
		RZ_SeasonDeeds season = Season();
		if (!season)
			return;

		season.NoteTally(p.m_sIdentity, f, false);
		season.Ledger(p.m_sIdentity, RZ_Util.PlayerName(playerId), what, f, against);
	}

	//------------------------------------------------------------------------------------------------
	//! The worst deed with `f` as people would say it: the kill tally ("put 9 of ours down") when
	//! three kills outweigh the biggest single deed, else its phrase, else "".
	static string WorstText(RZ_PlayerProfile p, int f)
	{
		if (!p || !RZ_Factions.IsValid(f))
			return "";

		p.EnsureArrays();
		int kills = p.m_aKills[f];
		if (kills > 1 && 3 * kills > p.m_aWorstAmount[f])
			return string.Format("put %1 of ours down", kills);

		return p.m_aWorstDeed[f];
	}

	//------------------------------------------------------------------------------------------------
	//! A life record ("pressed", "hid_bite", "lied.0"). One per key: a repeat moves its day. A bad one
	//! is pinned for 3 season days, so cheap jobs can't push it off. At most 32: the oldest unpinned
	//! goes first; with every one pinned, the new one isn't kept.
	static void AddLifeRecord(RZ_PlayerProfile p, string key, bool bad)
	{
		if (!p || key.IsEmpty())
			return;

		p.EnsureArrays();
		float day = RZ_SeasonClock.Days();
		float pin = 0;
		if (bad)
			pin = day + PIN_DAYS;

		int idx = p.m_aRecKeys.Find(key);
		if (idx >= 0)
		{
			p.m_aRecDays[idx] = day;
			p.m_aRecPinned[idx] = Math.Max(p.m_aRecPinned[idx], pin);
			return;
		}

		if (p.m_aRecKeys.Count() >= LIFE_RECORDS)
		{
			int drop = -1;
			for (int i = 0; i < p.m_aRecKeys.Count(); i++)
			{
				if (p.m_aRecPinned[i] > day)
					continue;

				if (drop < 0 || p.m_aRecDays[i] < p.m_aRecDays[drop])
					drop = i;
			}

			if (drop < 0)
				return;

			p.m_aRecKeys.RemoveOrdered(drop);
			p.m_aRecDays.RemoveOrdered(drop);
			p.m_aRecPinned.RemoveOrdered(drop);
		}

		p.m_aRecKeys.Insert(key);
		p.m_aRecDays.Insert(day);
		p.m_aRecPinned.Insert(pin);
	}

	//------------------------------------------------------------------------------------------------
	static bool HasLifeRecord(RZ_PlayerProfile p, string key)
	{
		if (!p || !p.m_aRecKeys)
			return false;

		return p.m_aRecKeys.Contains(key);
	}

	// ================================================================ Season scope
	//------------------------------------------------------------------------------------------------
	//! A new reputation cell with `f`: the season record's best and worst cell.
	static void NoteCell(RZ_GameModeComponent gm, int playerId, int f, int cell)
	{
		RZ_SeasonDeeds season = Season();
		if (!gm || !season)
			return;

		season.NoteCell(gm.GetProfile(playerId).m_sIdentity, RZ_Util.PlayerName(playerId), f, cell);
	}

	//------------------------------------------------------------------------------------------------
	static RZ_SeasonDeeds Season()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetShared())
			return null;

		return gm.GetShared().GetSeasonDeeds();
	}
}

//------------------------------------------------------------------------------------------------
//! One identity's season record. Plain fields: saved as they are.
class RZ_SeasonDeed
{
	string ident;
	string name;								// the name they last played under
	ref array<int> bestCell = {-1, -1, -1, -1};		// RZ_ERep, -1 = none yet
	ref array<int> worstCell = {-1, -1, -1, -1};
	ref array<string> bestDeed = {"", "", "", ""};
	ref array<int> bestAmount = {0, 0, 0, 0};
	ref array<string> worstDeed = {"", "", "", ""};
	ref array<int> worstAmount = {0, 0, 0, 0};
	ref array<int> kills = {0, 0, 0, 0};
	ref array<int> jobs = {0, 0, 0, 0};
	ref array<int> forgiven = {0, 0, 0, 0};
	ref array<string> ledger = {};				// "day|what|for|against", newest last

	//------------------------------------------------------------------------------------------------
	void Pad()
	{
		if (!bestCell) bestCell = new array<int>();
		if (!worstCell) worstCell = new array<int>();
		if (!bestDeed) bestDeed = new array<string>();
		if (!bestAmount) bestAmount = new array<int>();
		if (!worstDeed) worstDeed = new array<string>();
		if (!worstAmount) worstAmount = new array<int>();
		if (!kills) kills = new array<int>();
		if (!jobs) jobs = new array<int>();
		if (!forgiven) forgiven = new array<int>();
		if (!ledger) ledger = new array<string>();
		while (bestCell.Count() < 4) { bestCell.Insert(-1); }
		while (worstCell.Count() < 4) { worstCell.Insert(-1); }
		while (bestDeed.Count() < 4) { bestDeed.Insert(""); }
		while (bestAmount.Count() < 4) { bestAmount.Insert(0); }
		while (worstDeed.Count() < 4) { worstDeed.Insert(""); }
		while (worstAmount.Count() < 4) { worstAmount.Insert(0); }
		while (kills.Count() < 4) { kills.Insert(0); }
		while (jobs.Count() < 4) { jobs.Insert(0); }
		while (forgiven.Count() < 4) { forgiven.Insert(0); }
	}
}

//------------------------------------------------------------------------------------------------
class RZ_SeasonDeedsSave
{
	ref array<ref RZ_SeasonDeed> list = {};
}

//------------------------------------------------------------------------------------------------
//! The season scope of the deeds store, by identity. Saved with the season, wiped with it.
class RZ_SeasonDeeds
{
	static const int LEDGER_PER_PLAYER = 24;
	static const int MAX_PLAYERS = 400;

	protected ref map<string, ref RZ_SeasonDeed> m_mDeeds = new map<string, ref RZ_SeasonDeed>();

	//------------------------------------------------------------------------------------------------
	//! The record for `ident`, made when `create` (null for an empty identity or a full store).
	RZ_SeasonDeed Find(string ident, bool create = true)
	{
		if (ident.IsEmpty())
			return null;

		RZ_SeasonDeed d;
		if (m_mDeeds.Find(ident, d))
			return d;

		if (!create || m_mDeeds.Count() >= MAX_PLAYERS)
			return null;

		d = new RZ_SeasonDeed();
		d.ident = ident;
		d.Pad();
		m_mDeeds.Set(ident, d);
		return d;
	}

	//------------------------------------------------------------------------------------------------
	void NoteCell(string ident, string name, int f, int cell)
	{
		RZ_SeasonDeed d = Find(ident);
		if (!d || !RZ_Factions.IsValid(f))
			return;

		d.name = name;
		int good = RZ_RepNames.Goodness(cell);
		if (d.bestCell[f] < 0 || good > RZ_RepNames.Goodness(d.bestCell[f]))
			d.bestCell[f] = cell;

		if (d.worstCell[f] < 0 || good < RZ_RepNames.Goodness(d.worstCell[f]))
			d.worstCell[f] = cell;
	}

	//------------------------------------------------------------------------------------------------
	void NoteDeed(string ident, int f, int amount, string phrase, bool best)
	{
		if (phrase.IsEmpty() || !RZ_Factions.IsValid(f))
			return;

		RZ_SeasonDeed d = Find(ident);
		if (!d)
			return;

		if (best && amount > d.bestAmount[f])
		{
			d.bestAmount[f] = amount;
			d.bestDeed[f] = phrase;
		}
		else if (!best && amount > d.worstAmount[f])
		{
			d.worstAmount[f] = amount;
			d.worstDeed[f] = phrase;
		}
	}

	//------------------------------------------------------------------------------------------------
	void NoteTally(string ident, int f, bool kill)
	{
		RZ_SeasonDeed d = Find(ident);
		if (!d || !RZ_Factions.IsValid(f))
			return;

		if (kill)
			d.kills[f] = d.kills[f] + 1;
		else
			d.jobs[f] = d.jobs[f] + 1;
	}

	//------------------------------------------------------------------------------------------------
	void NoteForgiven(string ident, int f, int amount)
	{
		RZ_SeasonDeed d = Find(ident);
		if (!d || !RZ_Factions.IsValid(f) || amount <= 0)
			return;

		d.forgiven[f] = d.forgiven[f] + amount;
	}

	//------------------------------------------------------------------------------------------------
	//! A ledger line: what you did, for whom (-1 nobody), against whom (-1 nobody). The newest 24.
	void Ledger(string ident, string name, string what, int forF, int against)
	{
		RZ_SeasonDeed d = Find(ident);
		if (!d || what.IsEmpty())
			return;

		d.name = name;
		d.ledger.Insert(string.Format("%1|%2|%3|%4", RZ_SeasonClock.Day(), RZ_UIText.Clean(what, 120), forF, against));
		while (d.ledger.Count() > LEDGER_PER_PLAYER)
		{
			d.ledger.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	int Count()
	{
		return m_mDeeds.Count();
	}

	//------------------------------------------------------------------------------------------------
	RZ_SeasonDeedsSave Save()
	{
		RZ_SeasonDeedsSave data = new RZ_SeasonDeedsSave();
		foreach (string ident, RZ_SeasonDeed d : m_mDeeds)
		{
			data.list.Insert(d);
		}

		return data;
	}

	//------------------------------------------------------------------------------------------------
	void Restore(RZ_SeasonDeedsSave data)
	{
		m_mDeeds.Clear();
		if (!data || !data.list)
			return;

		foreach (RZ_SeasonDeed d : data.list)
		{
			if (!d || d.ident.IsEmpty())
				continue;

			d.Pad();
			m_mDeeds.Set(d.ident, d);
		}
	}
}
