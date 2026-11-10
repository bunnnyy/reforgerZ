//! The hall of fame (docs/DESIGN.md, "Seasons"): who delivered the most samples, held the final
//! stand, rescued the most people, lived longest, finished the most missions, and each faction's
//! Champion (the highest Fame anyone reached with it), per season and over all time. Saved to
//! $profile:ReforgerZ/hall_of_fame.json, which no wipe touches. Server only.

//! One player's record in a season or over all time.
class RZ_HallPlayer
{
	string ident;
	string name;
	int samples;
	int rescues;
	int missions;
	int stands;
	float longestLife;		// seconds
	ref array<int> peakFame = {0, 0, 0, 0};
}

//------------------------------------------------------------------------------------------------
//! A finished season, in a few lines.
class RZ_HallPast
{
	int season;
	string world;
	int winner = -1;
	string lines;
}

//------------------------------------------------------------------------------------------------
class RZ_HallData
{
	int version = 1;
	int season;
	string world;
	bool ended;
	ref array<ref RZ_HallPlayer> seasonPlayers = {};
	ref array<ref RZ_HallPlayer> allTime = {};
	ref array<string> standNames = {};		// who held this season's winning stand
	ref array<ref RZ_HallPast> past = {};
}

//------------------------------------------------------------------------------------------------
class RZ_HallOfFame
{
	static const string PATH = "$profile:ReforgerZ/hall_of_fame.json";

	//! Categories, in payload and text order. Champions follow as CHAMPION + faction.
	static const int CAT_SAMPLES = 0;
	static const int CAT_STAND = 1;
	static const int CAT_RESCUES = 2;
	static const int CAT_LIFE = 3;
	static const int CAT_MISSIONS = 4;
	static const int CAT_CHAMPION = 5;

	protected static const int PAST_KEEP = 10;
	protected static const int PLAYER_CAP = 500;
	protected static const float LIFE_CHECK = 60;

	protected RZ_GameModeComponent m_GM;
	protected bool m_bEnabled;
	protected ref RZ_HallData m_Data;
	protected bool m_bDirty;
	protected float m_fLifeTimer;

	//------------------------------------------------------------------------------------------------
	void RZ_HallOfFame(RZ_GameModeComponent gm, bool enabled)
	{
		m_GM = gm;
		m_bEnabled = enabled;
		m_Data = Load();
		if (!m_Data)
			m_Data = new RZ_HallData();
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled() { return m_bEnabled; }

	//------------------------------------------------------------------------------------------------
	//! The season the hall last recorded, 0 if none.
	int GetSeason()
	{
		return m_Data.season;
	}

	// ================================================================ File
	//------------------------------------------------------------------------------------------------
	protected static RZ_HallData Load()
	{
		if (!FileIO.FileExists(PATH))
			return null;

		JsonLoadContext ctx = new JsonLoadContext();
		if (!ctx.LoadFromFile(PATH))
			return null;

		RZ_HallData data = new RZ_HallData();
		if (!ctx.ReadValue("hall", data))
			return null;

		if (!data.seasonPlayers)
			data.seasonPlayers = new array<ref RZ_HallPlayer>();
		if (!data.allTime)
			data.allTime = new array<ref RZ_HallPlayer>();
		if (!data.standNames)
			data.standNames = new array<string>();
		if (!data.past)
			data.past = new array<ref RZ_HallPast>();

		return data;
	}

	//------------------------------------------------------------------------------------------------
	void SaveIfChanged()
	{
		if (!m_bEnabled || !m_bDirty)
			return;

		m_bDirty = false;
		FileIO.MakeDirectory(RZ_Persistence.DIR);
		JsonSaveContext ctx = new JsonSaveContext();
		ctx.WriteValue("hall", m_Data);
		if (!ctx.SaveToFile(PATH))
			RZ_Log.Warn("Could not write " + PATH);
	}

	// ================================================================ Seasons
	//------------------------------------------------------------------------------------------------
	//! A season starts or continues. A new number archives the old season first (if it was never
	//! closed by a wipe, e.g. a wipe by hand).
	void BeginSeason(int season, string world)
	{
		if (!m_bEnabled || season == m_Data.season)
			return;

		if (m_Data.season > 0 && !m_Data.ended)
			Archive(-1);

		m_Data.season = season;
		m_Data.world = world;
		m_Data.ended = false;
		m_Data.seasonPlayers.Clear();
		m_Data.standNames.Clear();
		m_bDirty = true;
		SaveIfChanged();
	}

	//------------------------------------------------------------------------------------------------
	//! The final stand was won: the players at the lab are recorded by AddStand before this.
	void OnSeasonWon()
	{
		m_bDirty = true;
		SaveIfChanged();
	}

	//------------------------------------------------------------------------------------------------
	//! The wipe came due: archive this season now.
	void EndSeason(int season, int winner)
	{
		if (!m_bEnabled || m_Data.ended)
			return;

		Archive(winner);
		m_Data.ended = true;
		m_bDirty = true;
		SaveIfChanged();
	}

	//------------------------------------------------------------------------------------------------
	protected void Archive(int winner)
	{
		// A season closed by hand: last_season.json may still know who won it
		if (winner < 0)
		{
			RZ_SeasonRecord record = RZ_Persistence.LoadSeason();
			if (record && record.season == m_Data.season)
				winner = record.winner;
		}

		RZ_HallPast p = new RZ_HallPast();
		p.season = m_Data.season;
		p.world = m_Data.world;
		p.winner = winner;
		p.lines = BuildLines(m_Data.seasonPlayers, m_Data.standNames);
		m_Data.past.InsertAt(p, 0);
		while (m_Data.past.Count() > PAST_KEEP)
		{
			m_Data.past.Remove(m_Data.past.Count() - 1);
		}
	}

	// ================================================================ Records
	//------------------------------------------------------------------------------------------------
	protected RZ_HallPlayer Find(array<ref RZ_HallPlayer> list, string ident, string name)
	{
		foreach (RZ_HallPlayer p : list)
		{
			if (p.ident == ident)
			{
				if (!name.IsEmpty())
					p.name = name;

				return p;
			}
		}

		// Past the cap, the record with the least to show for itself makes room
		if (list.Count() >= PLAYER_CAP)
			list.Remove(Weakest(list));

		RZ_HallPlayer fresh = new RZ_HallPlayer();
		fresh.ident = ident;
		fresh.name = name;
		list.Insert(fresh);
		return fresh;
	}

	//------------------------------------------------------------------------------------------------
	protected int Weakest(array<ref RZ_HallPlayer> list)
	{
		int best = 0;
		float bestScore = float.MAX;
		foreach (int i, RZ_HallPlayer p : list)
		{
			float score = p.samples + p.rescues + p.missions + p.stands * 10 + p.longestLife / 600;
			if (score < bestScore)
			{
				bestScore = score;
				best = i;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! This season's and the all-time record of a player, or false (unknown, or the hall is off).
	protected bool Records(int playerId, notnull array<RZ_HallPlayer> result)
	{
		result.Clear();
		if (!m_bEnabled || playerId <= 0)
			return false;

		RZ_PlayerProfile profile = m_GM.FindProfile(playerId);
		if (!profile || profile.m_sIdentity.IsEmpty())
			return false;

		string name = RZ_Util.PlayerName(playerId);
		result.Insert(Find(m_Data.seasonPlayers, profile.m_sIdentity, name));
		result.Insert(Find(m_Data.allTime, profile.m_sIdentity, name));
		m_bDirty = true;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void AddSamples(int playerId, int count)
	{
		array<RZ_HallPlayer> recs = {};
		if (count <= 0 || !Records(playerId, recs))
			return;

		foreach (RZ_HallPlayer r : recs)
		{
			r.samples = r.samples + count;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! People rescued (to a safehouse), credited to each rescuer.
	void AddRescues(array<int> rescuers, int count)
	{
		if (!rescuers || count <= 0)
			return;

		foreach (int playerId : rescuers)
		{
			array<RZ_HallPlayer> recs = {};
			if (!Records(playerId, recs))
				continue;

			foreach (RZ_HallPlayer r : recs)
			{
				r.rescues = r.rescues + count;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void AddMission(int playerId)
	{
		array<RZ_HallPlayer> recs = {};
		if (!Records(playerId, recs))
			return;

		foreach (RZ_HallPlayer r : recs)
		{
			r.missions = r.missions + 1;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! At the lab when the final stand was won.
	void AddStand(int playerId)
	{
		array<RZ_HallPlayer> recs = {};
		if (!Records(playerId, recs))
			return;

		foreach (RZ_HallPlayer r : recs)
		{
			r.stands = r.stands + 1;
		}

		string name = RZ_Util.PlayerName(playerId);
		if (!m_Data.standNames.Contains(name))
			m_Data.standNames.Insert(name);
	}

	//------------------------------------------------------------------------------------------------
	//! Fame with a faction reached in a life: the season's Champion is whoever reached the most.
	void NoteFame(int playerId, int faction, int fame)
	{
		if (!m_bEnabled || faction < 0 || faction >= RZ_Factions.COUNT || fame <= 0)
			return;

		RZ_PlayerProfile profile = m_GM.FindProfile(playerId);
		if (!profile || profile.m_sIdentity.IsEmpty())
			return;

		// Cheap check first: most Fame changes don't set a record
		RZ_HallPlayer seasonRec = Find(m_Data.seasonPlayers, profile.m_sIdentity, "");
		if (fame <= seasonRec.peakFame[faction])
			return;

		array<RZ_HallPlayer> recs = {};
		if (!Records(playerId, recs))
			return;

		foreach (RZ_HallPlayer r : recs)
		{
			if (fame > r.peakFame[faction])
				r.peakFame[faction] = fame;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Once a minute: the longest lives so far (the life clock in the profile).
	void Tick(float dt, array<int> ids)
	{
		if (!m_bEnabled)
			return;

		m_fLifeTimer += dt;
		if (m_fLifeTimer < LIFE_CHECK)
			return;

		m_fLifeTimer = 0;
		foreach (int playerId : ids)
		{
			RZ_PlayerProfile profile = m_GM.FindProfile(playerId);
			if (!profile || profile.m_sIdentity.IsEmpty() || profile.m_fLifeSeconds < 600)
				continue;

			RZ_HallPlayer seasonRec = Find(m_Data.seasonPlayers, profile.m_sIdentity, "");
			if (profile.m_fLifeSeconds <= seasonRec.longestLife)
				continue;

			array<RZ_HallPlayer> recs = {};
			if (!Records(playerId, recs))
				continue;

			foreach (RZ_HallPlayer r : recs)
			{
				r.longestLife = Math.Max(r.longestLife, profile.m_fLifeSeconds);
			}
		}
	}

	// ================================================================ Reading
	//------------------------------------------------------------------------------------------------
	//! The best record in a category, or null if nobody has one. value: the number to show.
	protected RZ_HallPlayer Best(array<ref RZ_HallPlayer> list, int cat, out float value)
	{
		RZ_HallPlayer best;
		value = 0;
		foreach (RZ_HallPlayer p : list)
		{
			float v = ValueOf(p, cat);
			if (v > value)
			{
				value = v;
				best = p;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	protected static float ValueOf(RZ_HallPlayer p, int cat)
	{
		switch (cat)
		{
			case CAT_SAMPLES: return p.samples;
			case CAT_STAND: return p.stands;
			case CAT_RESCUES: return p.rescues;
			case CAT_LIFE: return p.longestLife;
			case CAT_MISSIONS: return p.missions;
		}

		int f = cat - CAT_CHAMPION;
		if (f >= 0 && f < p.peakFame.Count())
			return p.peakFame[f];

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	static string CategoryName(int cat)
	{
		switch (cat)
		{
			case CAT_SAMPLES: return "Most samples delivered";
			case CAT_STAND: return "Held the final stand";
			case CAT_RESCUES: return "Most people rescued";
			case CAT_LIFE: return "Longest life";
			case CAT_MISSIONS: return "Most missions";
		}

		return "Champion of " + RZ_Factions.GetNameInText(cat - CAT_CHAMPION);
	}

	//------------------------------------------------------------------------------------------------
	protected static string ValueText(int cat, float value)
	{
		if (cat == CAT_LIFE)
		{
			int minutes = RZ_Util.RoundToInt(value / 60);
			return string.Format("%1 h %2 min", minutes / 60, minutes % 60);
		}

		if (cat == CAT_STAND)
			return string.Format("%1 time(s)", RZ_Util.RoundToInt(value));

		if (cat >= CAT_CHAMPION)
			return string.Format("%1 Fame", RZ_Util.RoundToInt(value));

		return RZ_Util.RoundToInt(value).ToString();
	}

	//------------------------------------------------------------------------------------------------
	static int CategoryCount()
	{
		return CAT_CHAMPION + RZ_Factions.COUNT;
	}

	//------------------------------------------------------------------------------------------------
	//! "Most samples delivered: Mara (41)" lines for a list. The stand lists everyone who held it.
	protected string BuildLines(array<ref RZ_HallPlayer> list, array<string> standNames)
	{
		string text = "";
		for (int cat = 0; cat < CategoryCount(); cat++)
		{
			if (cat == CAT_STAND && standNames && !standNames.IsEmpty())
			{
				text += CategoryName(cat) + ": " + JoinNames(standNames) + "\n";
				continue;
			}

			float value;
			RZ_HallPlayer p = Best(list, cat, value);
			if (p)
				text += string.Format("%1: %2 (%3)\n", CategoryName(cat), p.name, ValueText(cat, value));
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	protected static string JoinNames(array<string> names)
	{
		string text = "";
		foreach (int i, string n : names)
		{
			if (i >= 8)
			{
				text += string.Format(" and %1 more", names.Count() - 8);
				break;
			}

			if (i > 0)
				text += ", ";

			text += n;
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! The hall as one notice (first spawn of a session, and without the menu).
	string BuildText()
	{
		if (!m_bEnabled)
			return "";

		string text = "";
		string seasonLines = BuildLines(m_Data.seasonPlayers, m_Data.standNames);
		if (!seasonLines.IsEmpty())
			text += string.Format("Season %1:\n%2", m_Data.season, seasonLines);

		string allLines = BuildLines(m_Data.allTime, null);
		if (!allLines.IsEmpty())
			text += "\nAll time:\n" + allLines;

		if (!m_Data.past.IsEmpty())
		{
			RZ_HallPast last = m_Data.past[0];
			text += string.Format("\nSeason %1: the cure went to %2.", last.season, WinnerText(last.winner));
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	protected static string WinnerText(int winner)
	{
		if (winner < 0)
			return "nobody";

		return RZ_Factions.GetNameInText(winner);
	}

	//------------------------------------------------------------------------------------------------
	//! For the hall of fame menu (RZ_UIFeed, PAYLOAD_HALL):
	//! H|season
	//! S|category|category name|name|value text        this season (the stand: every name)
	//! A|category|category name|name|value text        all time
	//! P|season|winner text|lines (separated by "; ")
	string BuildPayload()
	{
		RZ_UILine head = new RZ_UILine("H");
		string s = head.AddInt(m_Data.season).End();
		if (!m_bEnabled)
			return s;

		for (int cat = 0; cat < CategoryCount(); cat++)
		{
			if (cat == CAT_STAND && !m_Data.standNames.IsEmpty())
			{
				RZ_UILine lnStand = new RZ_UILine("S");
				s += lnStand.AddInt(cat).Add(CategoryName(cat)).Add(JoinNames(m_Data.standNames)).Add("").End();
			}
			else
			{
				float v;
				RZ_HallPlayer p = Best(m_Data.seasonPlayers, cat, v);
				if (p)
				{
					RZ_UILine lnS = new RZ_UILine("S");
					s += lnS.AddInt(cat).Add(CategoryName(cat)).Add(p.name).Add(ValueText(cat, v)).End();
				}
			}

			float va;
			RZ_HallPlayer pa = Best(m_Data.allTime, cat, va);
			if (pa)
			{
				RZ_UILine lnA = new RZ_UILine("A");
				s += lnA.AddInt(cat).Add(CategoryName(cat)).Add(pa.name).Add(ValueText(cat, va)).End();
			}
		}

		foreach (RZ_HallPast past : m_Data.past)
		{
			string lines = past.lines;
			lines.Replace("\n", "; ");
			RZ_UILine lnP = new RZ_UILine("P");
			s += lnP.AddInt(past.season).Add(WinnerText(past.winner)).Add(lines).End();
		}

		return s;
	}
}
