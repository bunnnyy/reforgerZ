//! The season record and titles (docs/REPUTATION.md, "The season record"): the name a player made
//! this season, kept by identity across lives and wiped with the season. A face is forgotten when
//! a life ends; the name isn't. Server only.
//!
//! Per identity, sampled every 30 s for players online (nothing polls per frame):
//! - per faction: the best and worst cell reached (by access rank, then the cell's place on the
//!   grid), the life each was reached in, the deed phrase that stood when it was, the cell at the
//!   90% lock, kills, jobs and Infamy worked off across lives, and the last Infamy seen (square);
//! - the route at the lock and when it was taken, one war deed (NoteWar), the latest crew name.
//! Its title with a faction is the name of the best cell reached ("Necessary Evil").
//! Readers: dialogue's title, seasoned and lockdays conditions and {title:f}, and the epilogue
//! (SeasonPicture, SeasonDeed, SeasonCell, RouteAtLock, CrewName, TitleFor, IsSquare).
//! Saved in its own file, $profile:ReforgerZ/season_record.json, at the 2-minute save when changed.

//------------------------------------------------------------------------------------------------
class RZ_RepSeasonEntry
{
	string ident;
	string name;								// frozen at the first record (the names policy)
	int life;									// the life the per-life tallies below were last read in
	ref array<int> bestRank = {-9, -9, -9, -9};	// -9: never seen
	ref array<int> bestCell = {0, 0, 0, 0};		// RZ_ERep
	ref array<int> bestLife = {0, 0, 0, 0};
	ref array<int> worstRank = {9, 9, 9, 9};
	ref array<int> worstCell = {0, 0, 0, 0};
	ref array<string> bestDeed = {"", "", "", ""};
	ref array<string> worstDeed = {"", "", "", ""};
	ref array<int> lockCell = {-1, -1, -1, -1};
	ref array<int> killsBase = {0, 0, 0, 0};	// earlier lives' tallies
	ref array<int> killsNow = {0, 0, 0, 0};
	ref array<int> jobsBase = {0, 0, 0, 0};
	ref array<int> jobsNow = {0, 0, 0, 0};
	ref array<int> offBase = {0, 0, 0, 0};		// Infamy worked off (the forgiveness budget used)
	ref array<int> offNow = {0, 0, 0, 0};
	ref array<int> infamy = {0, 0, 0, 0};		// the last Infamy seen
	int route = -2;								// RZ_CureFinale.NONE
	int lockUnix;
	string war;
	string crew;
}

//------------------------------------------------------------------------------------------------
class RZ_RepSeasonSave
{
	int season;
	ref array<ref RZ_RepSeasonEntry> entries = {};
}

//------------------------------------------------------------------------------------------------
class RZ_RepSeason
{
	static const string PATH = "$profile:ReforgerZ/season_record.json";
	protected static const float SAMPLE = 30;
	protected static const int NEVER_BEST = -9;
	protected static const int NEVER_WORST = 9;

	protected static RZ_RepSeason s_Instance;

	protected RZ_GameModeComponent m_GM;
	protected ref map<string, ref RZ_RepSeasonEntry> m_mEntries = new map<string, ref RZ_RepSeasonEntry>();
	protected int m_iSeason;
	protected bool m_bDirty;
	protected float m_fTimer;

	//------------------------------------------------------------------------------------------------
	void RZ_RepSeason(RZ_GameModeComponent gm)
	{
		m_GM = gm;
		s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	void ~RZ_RepSeason()
	{
		if (s_Instance == this)
			s_Instance = null;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_RepSeason Get()
	{
		return s_Instance;
	}

	// ================================================================ Saving
	//------------------------------------------------------------------------------------------------
	//! At start: the saved record, unless it belongs to another season.
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

		RZ_RepSeasonSave data = new RZ_RepSeasonSave();
		if (!ctx.ReadValue("record", data) || !data.entries)
			return;

		if (data.season != m_iSeason)
		{
			RZ_Log.Info(string.Format("Reputation: season record from season %1 dropped (season %2)", data.season, m_iSeason));
			m_bDirty = true;
			return;
		}

		foreach (RZ_RepSeasonEntry e : data.entries)
		{
			if (e && !e.ident.IsEmpty() && Whole(e))
				m_mEntries.Set(e.ident, e);
		}

		RZ_Log.Info(string.Format("Reputation: season record for %1 players", m_mEntries.Count()));
	}

	//------------------------------------------------------------------------------------------------
	//! A saved entry with every per-faction array in place (a hand-edited file may lack some).
	protected static bool Whole(RZ_RepSeasonEntry e)
	{
		array<ref array<int>> ints = {e.bestRank, e.bestCell, e.bestLife, e.worstRank, e.worstCell, e.lockCell, e.killsBase, e.killsNow,
			e.jobsBase, e.jobsNow, e.offBase, e.offNow, e.infamy};
		foreach (array<int> a : ints)
		{
			if (!a || a.Count() < RZ_Factions.COUNT)
				return false;
		}

		return e.bestDeed && e.worstDeed && e.bestDeed.Count() >= RZ_Factions.COUNT && e.worstDeed.Count() >= RZ_Factions.COUNT;
	}

	//------------------------------------------------------------------------------------------------
	void SaveIfChanged()
	{
		if (!m_bDirty)
			return;

		m_bDirty = false;
		RZ_RepSeasonSave data = new RZ_RepSeasonSave();
		data.season = m_iSeason;
		foreach (string ident, RZ_RepSeasonEntry e : m_mEntries)
		{
			data.entries.Insert(e);
		}

		FileIO.MakeDirectory(RZ_Persistence.DIR);
		JsonSaveContext ctx = new JsonSaveContext();
		ctx.WriteValue("record", data);
		if (!ctx.SaveToFile(PATH))
			RZ_Log.Warn("Could not write " + PATH);
	}

	// ================================================================ Sampling
	//------------------------------------------------------------------------------------------------
	//! On the server tick: every 30 s, every player online.
	void Tick(float dt)
	{
		m_fTimer += dt;
		if (m_fTimer < SAMPLE)
			return;

		m_fTimer = 0;
		array<int> ids = {};
		GetGame().GetPlayerManager().GetPlayers(ids);
		foreach (int id : ids)
		{
			Note(id);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_RepSeasonEntry EntryFor(string ident, int playerId)
	{
		RZ_RepSeasonEntry e;
		if (m_mEntries.Find(ident, e))
			return e;

		e = new RZ_RepSeasonEntry();
		e.ident = ident;
		e.name = RZ_Names.Public(playerId);
		e.life = m_GM.GetProfile(playerId).m_iCharacter;
		m_mEntries.Set(ident, e);
		m_bDirty = true;
		return e;
	}

	//------------------------------------------------------------------------------------------------
	//! Access rank as the record ranks cells: Kill on Sight -3, Hunted -2, Watched -1, else as built.
	protected static int Rank(RZ_ETier tier)
	{
		if (tier == RZ_ETier.KILL_ON_SIGHT)
			return -3;

		if (tier == RZ_ETier.HUNTED)
			return -2;

		if (tier == RZ_ETier.WATCHED)
			return -1;

		return RZ_Reputation.GetAccessRank(tier);
	}

	//------------------------------------------------------------------------------------------------
	//! One player's standing now, into their season entry.
	void Note(int playerId)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		if (!p || p.m_sIdentity.IsEmpty())
			return;

		p.EnsureArrays();
		RZ_RepSeasonEntry e = EntryFor(p.m_sIdentity, playerId);
		if (e.life != p.m_iCharacter)
		{
			// A new life: what the last one tallied stays in the base
			for (int b = 0; b < RZ_Factions.COUNT; b++)
			{
				e.killsBase[b] = e.killsBase[b] + e.killsNow[b];
				e.jobsBase[b] = e.jobsBase[b] + e.jobsNow[b];
				e.offBase[b] = e.offBase[b] + e.offNow[b];
				e.killsNow[b] = 0;
				e.jobsNow[b] = 0;
				e.offNow[b] = 0;
			}

			e.life = p.m_iCharacter;
			m_bDirty = true;
		}

		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			NoteFaction(e, p, playerId, f);
		}

		if (!p.m_sCrewName.IsEmpty() && p.m_sCrewName != e.crew)
		{
			e.crew = p.m_sCrewName;
			m_bDirty = true;
		}

		NoteLock(e, playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void NoteFaction(RZ_RepSeasonEntry e, RZ_PlayerProfile p, int playerId, int f)
	{
		int fame = p.GetFame(f);
		int inf = p.GetInfamy(f);
		RZ_ERep cell = RZ_RepNames.GetRep(fame, inf);
		int rank = Rank(m_GM.GetTier(playerId, f));
		int good = RZ_RepNames.Goodness(cell);
		if (rank > e.bestRank[f] || (rank == e.bestRank[f] && good > RZ_RepNames.Goodness(e.bestCell[f])))
		{
			e.bestRank[f] = rank;
			e.bestCell[f] = cell;
			e.bestLife[f] = p.m_iCharacter;
			m_bDirty = true;
		}

		if (rank < e.worstRank[f] || (rank == e.worstRank[f] && good < RZ_RepNames.Goodness(e.worstCell[f])))
		{
			e.worstRank[f] = rank;
			e.worstCell[f] = cell;
			m_bDirty = true;
		}

		// The deeds that stood at the best and the worst: the latest of the life that reached them
		if (e.bestLife[f] == p.m_iCharacter && !p.m_aBestDeed[f].IsEmpty() && p.m_aBestDeed[f] != e.bestDeed[f])
		{
			e.bestDeed[f] = p.m_aBestDeed[f];
			m_bDirty = true;
		}

		string worst = RZ_Deeds.WorstText(p, f);
		if (rank <= e.worstRank[f] && !worst.IsEmpty() && worst != e.worstDeed[f])
		{
			e.worstDeed[f] = worst;
			m_bDirty = true;
		}

		if (p.m_aKills[f] != e.killsNow[f] || p.m_aJobs[f] != e.jobsNow[f] || p.m_aForgiven[f] != e.offNow[f] || inf != e.infamy[f])
		{
			e.killsNow[f] = p.m_aKills[f];
			e.jobsNow[f] = p.m_aJobs[f];
			e.offNow[f] = p.m_aForgiven[f];
			e.infamy[f] = inf;
			m_bDirty = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! At the 90% lock: the route, when (as first seen) and the cell with each faction then.
	protected void NoteLock(RZ_RepSeasonEntry e, int playerId)
	{
		RZ_CureFinale fin = m_GM.GetFinale();
		if (!fin || e.route != RZ_CureFinale.NONE)
			return;

		int r = fin.RouteOfPlayer(playerId);
		if (r == RZ_CureFinale.NONE)
			return;

		e.route = r;
		e.lockUnix = RZ_SeasonClock.Now();
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			e.lockCell[f] = RZ_RepNames.GetRep(p.GetFame(f), p.GetInfamy(f));
		}

		m_bDirty = true;
		RZ_Log.Info(string.Format("Reputation: season record: %1 locked to %2", RZ_Util.PlayerName(playerId), RZ_CureFinale.RouteName(r)));
	}

	//------------------------------------------------------------------------------------------------
	//! One war deed (a kill between locked players, said plainly), the latest kept. For the finale.
	void NoteWar(string ident, string text)
	{
		RZ_RepSeasonEntry e;
		if (text.IsEmpty() || !m_mEntries.Find(ident, e))
			return;

		e.war = text;
		m_bDirty = true;
	}

	// ================================================================ What the ending reads
	//------------------------------------------------------------------------------------------------
	protected RZ_RepSeasonEntry Find(string ident)
	{
		RZ_RepSeasonEntry e;
		m_mEntries.Find(ident, e);
		return e;
	}

	//------------------------------------------------------------------------------------------------
	//! The name of the best cell reached with `f` this season ("Necessary Evil"), "" if never seen.
	string TitleFor(string ident, int f)
	{
		RZ_RepSeasonEntry e = Find(ident);
		if (!e || !RZ_Factions.IsValid(f) || e.bestRank[f] == NEVER_BEST)
			return "";

		return RZ_RepNames.GetName(e.bestCell[f]);
	}

	//------------------------------------------------------------------------------------------------
	//! The best cell (RZ_ERep) reached with `f`; `worst` for the worst. -1 if never seen.
	int SeasonCell(string ident, int f, bool worst = false)
	{
		RZ_RepSeasonEntry e = Find(ident);
		if (!e || !RZ_Factions.IsValid(f) || e.bestRank[f] == NEVER_BEST)
			return -1;

		if (worst)
			return e.worstCell[f];

		return e.bestCell[f];
	}

	//------------------------------------------------------------------------------------------------
	//! The deed that stood at the best cell with `f` ("held the lab"); `worst` for the worst.
	string SeasonDeed(string ident, int f, bool worst = false)
	{
		RZ_RepSeasonEntry e = Find(ident);
		if (!e || !RZ_Factions.IsValid(f))
			return "";

		if (worst)
			return e.worstDeed[f];

		return e.bestDeed[f];
	}

	//------------------------------------------------------------------------------------------------
	//! Kills of `f`'s people, jobs for `f` and Infamy worked off with `f`, across every life.
	int Kills(string ident, int f)
	{
		RZ_RepSeasonEntry e = Find(ident);
		if (!e || !RZ_Factions.IsValid(f))
			return 0;

		return e.killsBase[f] + e.killsNow[f];
	}

	//------------------------------------------------------------------------------------------------
	int Jobs(string ident, int f)
	{
		RZ_RepSeasonEntry e = Find(ident);
		if (!e || !RZ_Factions.IsValid(f))
			return 0;

		return e.jobsBase[f] + e.jobsNow[f];
	}

	//------------------------------------------------------------------------------------------------
	int WorkedOff(string ident, int f)
	{
		RZ_RepSeasonEntry e = Find(ident);
		if (!e || !RZ_Factions.IsValid(f))
			return 0;

		return e.offBase[f] + e.offNow[f];
	}

	//------------------------------------------------------------------------------------------------
	//! The route at the lock (RZ_CureFinale's route numbers), NONE if none.
	int RouteAtLock(string ident)
	{
		RZ_RepSeasonEntry e = Find(ident);
		if (!e)
			return RZ_CureFinale.NONE;

		return e.route;
	}

	//------------------------------------------------------------------------------------------------
	//! Real days since the lock, -1 if not locked.
	int LockDays(string ident)
	{
		RZ_RepSeasonEntry e = Find(ident);
		if (!e || e.route == RZ_CureFinale.NONE)
			return -1;

		return (RZ_SeasonClock.Now() - e.lockUnix) / 86400;
	}

	//------------------------------------------------------------------------------------------------
	//! The cell with `f` at the lock (RZ_ERep), -1 if not locked.
	int LockCell(string ident, int f)
	{
		RZ_RepSeasonEntry e = Find(ident);
		if (!e || !RZ_Factions.IsValid(f))
			return -1;

		return e.lockCell[f];
	}

	//------------------------------------------------------------------------------------------------
	string CrewName(string ident)
	{
		RZ_RepSeasonEntry e = Find(ident);
		if (!e)
			return "";

		return e.crew;
	}

	//------------------------------------------------------------------------------------------------
	string WarDeed(string ident)
	{
		RZ_RepSeasonEntry e = Find(ident);
		if (!e)
			return "";

		return e.war;
	}

	//------------------------------------------------------------------------------------------------
	//! The name as it was first recorded this season (names policy: the caller filters it).
	string NameOf(string ident)
	{
		RZ_RepSeasonEntry e = Find(ident);
		if (!e)
			return "";

		return e.name;
	}

	//------------------------------------------------------------------------------------------------
	//! Square with `f`: the last Infamy seen with them is under 15. (The Red Hand's book lines are
	//! the finale's and the epilogue's to add.)
	bool IsSquare(string ident, int f)
	{
		RZ_RepSeasonEntry e = Find(ident);
		if (!e || !RZ_Factions.IsValid(f) || e.infamy[f] >= 15)
			return false;

		// With the Red Hand, square also means no unpaid line in the book (a dose on credit, the
		// finale's price door), until the epilogue's Pay into the book marks it paid
		if (f == RZ_EFaction.BANDITS)
			return !UnpaidLine(ident);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A line in the Red Hand's book this identity hasn't paid.
	bool UnpaidLine(string ident)
	{
		RZ_Epilogue epi = RZ_Epilogue.Get();
		bool paid = epi && epi.Data().bookPaid.Contains(ident);
		if (paid)
			return false;

		if (epi && epi.Data().owed.Contains(ident))
			return true;

		RZ_CureFinale finale = RZ_CureFinale.Get();
		int pid = m_GM.FindPlayerByIdentity(ident);
		return finale && pid > 0 && finale.PaidPrice(pid, RZ_CureFinale.REDHAND);
	}

	//------------------------------------------------------------------------------------------------
	//! The biggest deed with `f` across every life this season (the season's deeds store), else the one
	//! that stood at the best (or worst) cell.
	string BiggestDeed(string ident, int f, bool worst = false)
	{
		RZ_SeasonDeeds season = RZ_Deeds.Season();
		if (season && RZ_Factions.IsValid(f))
		{
			RZ_SeasonDeed d = season.Find(ident, false);
			if (d && worst && !d.worstDeed[f].IsEmpty())
				return d.worstDeed[f];

			if (d && !worst && !d.bestDeed[f].IsEmpty())
				return d.bestDeed[f];
		}

		return SeasonDeed(ident, f, worst);
	}

	//------------------------------------------------------------------------------------------------
	//! They know the name, not the face: an earlier life this season stood out with `f` (Known or
	//! better at its best, or Watched or worse at its worst).
	bool KnownByName(string ident, int f, int life)
	{
		RZ_RepSeasonEntry e = Find(ident);
		if (!e || !RZ_Factions.IsValid(f) || e.bestRank[f] == NEVER_BEST)
			return false;

		bool earlier = e.bestLife[f] < life;
		return (earlier && e.bestRank[f] >= 1) || (e.worstRank[f] <= -1 && life > 0);
	}

	//------------------------------------------------------------------------------------------------
	//! The season in a few words: the title with the faction the player stood highest with
	//! ("Necessary Evil to Bastion"), "" if nothing stood out.
	string SeasonPicture(string ident)
	{
		RZ_RepSeasonEntry e = Find(ident);
		if (!e)
			return "";

		int best = -1;
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (e.bestRank[f] == NEVER_BEST)
				continue;

			if (best < 0 || e.bestRank[f] > e.bestRank[best] || (e.bestRank[f] == e.bestRank[best] && RZ_RepNames.Goodness(e.bestCell[f]) > RZ_RepNames.Goodness(e.bestCell[best])))
				best = f;
		}

		if (best < 0 || e.bestRank[best] < 1)
			return "";

		return string.Format("%1 to %2", RZ_RepNames.GetName(e.bestCell[best]), RZ_Factions.GetName(best));
	}

	//------------------------------------------------------------------------------------------------
	//! "/rep" and the admin read-out: one line for a player.
	string Describe(string ident)
	{
		RZ_RepSeasonEntry e = Find(ident);
		if (!e)
			return "no season record";

		string text = "Season:";
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (e.bestRank[f] == NEVER_BEST)
				continue;

			text += string.Format(" %1 best %2, worst %3;", RZ_RepNames.Initial(f), RZ_RepNames.GetName(e.bestCell[f]), RZ_RepNames.GetName(e.worstCell[f]));
		}

		if (e.route != RZ_CureFinale.NONE)
			text += " route " + RZ_CureFinale.RouteName(e.route) + ".";

		return text;
	}
}
