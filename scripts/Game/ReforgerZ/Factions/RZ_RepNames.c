//! Reputations: the 16 names of the Fame x Infamy grid (docs/REPUTATION.md, "Reputations").
//! The tier stays the only rule word: a name opens nothing. It's what people call you, and the
//! card always shows what it counts as.
//!
//!   Fame \ Infamy   < 15        15-39              40-74                75+
//!   < 15            Stranger    Watched            Hunted               Kill on Sight
//!   15-39           Known       Mixed Bag          Bad News             Merciful Thug
//!   40-74           Trusted     Useful Trouble     Wildcard             Soft-Hearted Devil
//!   75+             Champion    Favourite Rascal   Necessary Evil       Wild Child
//!
//! Also here: the flags of AddInfamy and AddFame (RZ_RepFlag), the settings (Factions >
//! Reputation) and the debounced change notice (RZ_RepWatch).

//------------------------------------------------------------------------------------------------
//! Append only: saved as numbers in the profile (m_aRepCell).
enum RZ_ERep
{
	STRANGER,
	KNOWN,
	TRUSTED,
	CHAMPION,
	WATCHED,
	HUNTED,
	KILL_ON_SIGHT,
	MIXED_BAG,
	USEFUL_TROUBLE,
	FAVOURITE_RASCAL,
	BAD_NEWS,
	WILDCARD,
	NECESSARY_EVIL,
	MERCIFUL_THUG,
	SOFT_HEARTED_DEVIL,
	WILD_CHILD
}

//------------------------------------------------------------------------------------------------
//! Flags for RZ_GameModeComponent.AddInfamy and AddFame.
class RZ_RepFlag
{
	//! A rival share (the rival table, a mission's rival Infamy, a questline's standing): not
	//! something you did to them. Never direct Infamy, never doubled by a disguise.
	static const int SHARE = 1;
	//! Done in sight of the faction: may double while you're passing in its kit (RZ_Disguise).
	static const int ACT = 2;
	//! Small pay (errands): left out of season Fame, the Trusted goal and records.
	static const int SMALL = 4;
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Factions > Reputation.
[BaseContainerProps()]
class RZ_ReputationSettings
{
	[Attribute("1", desc: "Reputation names: the standing card, the change notices and Ask what they think show the 16 names (Mixed Bag, Wildcard, Merciful Thug...) with what each counts as. Off = the tier words only. A name never opens anything (docs/REPUTATION.md, Reputations)")]
	bool m_bNames;

	[Attribute("30", UIWidgets.Slider, "Forgiveness budget: Infamy each faction forgives per life, whatever the route (amends contracts, paying, surrender). Counted after the stronghold leader's temper. 0 = no budget", "0 100 1")]
	int m_iForgiveBudget;

	[Attribute("1", desc: "The credit runs out: while your Fame with a faction is at the line below or more, every Infamy gain with it also costs the same Fame")]
	bool m_bCreditRule;

	[Attribute("75", UIWidgets.Slider, "Fame from which the credit rule applies", "40 100 1")]
	int m_iCreditFrom;

	[Attribute("1", desc: "The Wildcard tier at posts: searched at Bastion checkpoints like a stranger (running that search costs the Infamy below), and the Red Hand toll times the multiplier below")]
	bool m_bWildcardPosts;

	[Attribute("6", UIWidgets.Slider, "Bastion Infamy for running a Wildcard's search", "0 20 1")]
	int m_iWildcardRunInfamy;

	[Attribute("2", UIWidgets.Slider, "Red Hand toll multiplier for the Wildcard tier", "1 4 0.5")]
	float m_fWildcardTollMult;

	[Attribute("3", UIWidgets.Slider, "Threats per life per faction (the Fear check); after that a threat is always refused and still costs its Infamy", "0 10 1")]
	int m_iThreatsPerLife;

	[Attribute("30", UIWidgets.Slider, "Change notice: seconds a new name waits before it's told (a quick swing back says nothing)", "0 120 5")]
	float m_fChangeHoldSeconds;

	[Attribute("5", UIWidgets.Slider, "Change notice: minutes between name notices per faction (a tier change is always told)", "0 30 1")]
	float m_fChangeGapMinutes;

	[Attribute("1", desc: "Red Hand crew names: the first thing the crews hold on you this life (Known or better) becomes what they call you: Lucky, Snack, Legs, Skint, Pockets, Sorry (docs/REPUTATION.md, The word for you)")]
	bool m_bCrewNames;

	[Attribute("1", desc: "Life-stamped memory: leaders and wanderers count what they remember of an earlier life at half (rounded toward zero), and met doesn't carry. Off = memory by identity, as before")]
	bool m_bLifeMemory;

	protected static ref RZ_ReputationSettings s_Defaults;

	//------------------------------------------------------------------------------------------------
	//! The game mode's settings, else the defaults (always non-null).
	static RZ_ReputationSettings Get()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm && gm.GetReputationSettings())
			return gm.GetReputationSettings();

		if (!s_Defaults)
			s_Defaults = RZ_ReputationSettings.Cast(RZ_Util.NewWithDefaults("RZ_ReputationSettings"));

		return s_Defaults;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_RepNames
{
	//------------------------------------------------------------------------------------------------
	//! The cell of the grid for these values.
	static RZ_ERep GetRep(int fame, int infamy)
	{
		return CellAt(RZ_Reputation.Band(fame), RZ_Reputation.Band(infamy));
	}

	//------------------------------------------------------------------------------------------------
	//! The cell at Fame band `f` and Infamy band `i` (0 low, 1 some, 2 high, 3 extreme).
	static RZ_ERep CellAt(int f, int i)
	{
		if (f <= 0)
		{
			if (i <= 0) return RZ_ERep.STRANGER;
			if (i == 1) return RZ_ERep.WATCHED;
			if (i == 2) return RZ_ERep.HUNTED;
			return RZ_ERep.KILL_ON_SIGHT;
		}

		if (f == 1)
		{
			if (i <= 0) return RZ_ERep.KNOWN;
			if (i == 1) return RZ_ERep.MIXED_BAG;
			if (i == 2) return RZ_ERep.BAD_NEWS;
			return RZ_ERep.MERCIFUL_THUG;
		}

		if (f == 2)
		{
			if (i <= 0) return RZ_ERep.TRUSTED;
			if (i == 1) return RZ_ERep.USEFUL_TROUBLE;
			if (i == 2) return RZ_ERep.WILDCARD;
			return RZ_ERep.SOFT_HEARTED_DEVIL;
		}

		if (i <= 0) return RZ_ERep.CHAMPION;
		if (i == 1) return RZ_ERep.FAVOURITE_RASCAL;
		if (i == 2) return RZ_ERep.NECESSARY_EVIL;
		return RZ_ERep.WILD_CHILD;
	}

	//------------------------------------------------------------------------------------------------
	static string GetName(RZ_ERep rep)
	{
		switch (rep)
		{
			case RZ_ERep.STRANGER: return "Stranger";
			case RZ_ERep.KNOWN: return "Known";
			case RZ_ERep.TRUSTED: return "Trusted";
			case RZ_ERep.CHAMPION: return "Champion";
			case RZ_ERep.WATCHED: return "Watched";
			case RZ_ERep.HUNTED: return "Hunted";
			case RZ_ERep.KILL_ON_SIGHT: return "Kill on Sight";
			case RZ_ERep.MIXED_BAG: return "Mixed Bag";
			case RZ_ERep.USEFUL_TROUBLE: return "Useful Trouble";
			case RZ_ERep.FAVOURITE_RASCAL: return "Favourite Rascal";
			case RZ_ERep.BAD_NEWS: return "Bad News";
			case RZ_ERep.WILDCARD: return "Wildcard";
			case RZ_ERep.NECESSARY_EVIL: return "Necessary Evil";
			case RZ_ERep.MERCIFUL_THUG: return "Merciful Thug";
			case RZ_ERep.SOFT_HEARTED_DEVIL: return "Soft-Hearted Devil";
			case RZ_ERep.WILD_CHILD: return "Wild Child";
		}

		return "Stranger";
	}

	//------------------------------------------------------------------------------------------------
	//! A mixed cell: both Fame and Infamy past the low band, and not a plain tier cell.
	static bool IsMixed(RZ_ERep rep)
	{
		return rep >= RZ_ERep.MIXED_BAG;
	}

	//------------------------------------------------------------------------------------------------
	//! The name players see: the name with Reputation > Names on, else the tier word.
	static string Shown(int fame, int infamy)
	{
		if (!RZ_ReputationSettings.Get().m_bNames)
			return RZ_Reputation.GetTierName(RZ_Reputation.GetTier(fame, infamy));

		return GetName(GetRep(fame, infamy));
	}

	//------------------------------------------------------------------------------------------------
	//! What a name counts as, in rule words: "Known" for Useful Trouble, "Hunted" for Bad News.
	//! The tier word itself for a plain cell.
	static string CountsAs(int fame, int infamy)
	{
		RZ_ETier tier = RZ_Reputation.GetTier(fame, infamy);
		if (RZ_Reputation.IsHunted(tier) || RZ_Reputation.IsWatchedOrWorse(tier))
			return RZ_Reputation.GetTierName(tier);

		switch (RZ_Reputation.GetAccessRank(tier))
		{
			case 0: return "Stranger";
			case 1: return "Known";
			case 2: return "Trusted";
			case 3: return "Champion";
		}

		return RZ_Reputation.GetTierName(tier);
	}

	//------------------------------------------------------------------------------------------------
	//! "Useful Trouble - counts as Known", "Soft-Hearted Devil - Hunted", or the plain tier word.
	static string NameWithRule(int fame, int infamy)
	{
		string shown = Shown(fame, infamy);
		string rule = CountsAs(fame, infamy);
		if (shown == rule)
			return shown;

		RZ_ETier tier = RZ_Reputation.GetTier(fame, infamy);
		if (RZ_Reputation.IsWatchedOrWorse(tier))
			return string.Format("%1 - %2", shown, rule);

		return string.Format("%1 - counts as %2", shown, rule);
	}

	//------------------------------------------------------------------------------------------------
	//! The card's note, upper case and at most 42 characters: "COUNTS AS KNOWN - SEARCHED - TOLL 20"
	//! for the Wildcard tier, "COUNTS AS KNOWN" for a mixed cell, the built tier note otherwise.
	static string Note(int fame, int infamy)
	{
		RZ_ETier tier = RZ_Reputation.GetTier(fame, infamy);
		RZ_ERep rep = GetRep(fame, infamy);
		string text = "";
		if (tier == RZ_ETier.WILDCARD)
		{
			RZ_ReputationSettings settings = RZ_ReputationSettings.Get();
			text = "COUNTS AS KNOWN";
			if (settings.m_bWildcardPosts)
			{
				RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
				int toll = 10;
				if (gm && gm.GetCheckpointSettings())
					toll = gm.GetCheckpointSettings().m_iToll;

				text = string.Format("COUNTS AS KNOWN - SEARCHED - TOLL %1", RZ_Util.RoundToInt(toll * Math.Max(1, settings.m_fWildcardTollMult)));
			}
		}
		else if (IsMixed(rep) && !RZ_Reputation.IsHunted(tier))
		{
			text = "COUNTS AS " + CountsAs(fame, infamy);
			text.ToUpper();
		}

		if (text.Length() > 42)
			text = text.Substring(0, 42);

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! How well their people know your name, 0-3: the higher of the Fame band and the Infamy band.
	static int Notoriety(int fame, int infamy)
	{
		return Math.Max(RZ_Reputation.Band(fame), RZ_Reputation.Band(infamy));
	}

	//------------------------------------------------------------------------------------------------
	//! A cell's place from best to worst, for the season record's best and worst cell: the Fame band
	//! less the Infamy band, then Fame.
	static int Goodness(RZ_ERep rep)
	{
		for (int f = 0; f < 4; f++)
		{
			for (int i = 0; i < 4; i++)
			{
				if (CellAt(f, i) == rep)
					return (f - i) * 4 + f;
			}
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Initials for the grid: "H" the Haven, "B" Bastion, "R" the Red Hand, "C" Halcyon.
	static string Initial(int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "H";
			case RZ_EFaction.MILITARY: return "B";
			case RZ_EFaction.BANDITS: return "R";
			case RZ_EFaction.INSTITUTE: return "C";
		}

		return "";
	}
}

//------------------------------------------------------------------------------------------------
//! The debounced change notice (server): a new name is told only once it has held for the hold
//! seconds, at most once per faction per gap, with the tier change always told at once by
//! CheckTierChange. A log line on every cell change: "Reputation: <player> <faction> <name> (<tier>)".
class RZ_RepWatch
{
	protected RZ_GameModeComponent m_GM;
	protected ref map<string, float> m_mPendingAt = new map<string, float>();	// "playerId:f" -> clock when the new cell appeared
	protected ref map<string, float> m_mToldAt = new map<string, float>();		// "playerId:f" -> clock of the last name notice
	protected float m_fClock;

	//------------------------------------------------------------------------------------------------
	void RZ_RepWatch(RZ_GameModeComponent gm)
	{
		m_GM = gm;
	}

	//------------------------------------------------------------------------------------------------
	//! After every standing change: notes a new cell (the profile keeps the last told one).
	void OnStanding(int playerId, int faction, bool tierChanged)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		p.EnsureArrays();
		int cell = RZ_RepNames.GetRep(p.GetFame(faction), p.GetInfamy(faction));
		// A profile from before the names: take today's cell as told, silently
		if (p.m_aRepSeen[faction] < 0)
			p.m_aRepSeen[faction] = cell;

		if (cell == p.m_aRepSeen[faction])
		{
			m_mPendingAt.Remove(Key(playerId, faction));
			return;
		}

		RZ_Log.Info(string.Format("Reputation: %1 %2 %3 (%4)", RZ_Util.PlayerName(playerId), RZ_Factions.GetName(faction),
			RZ_RepNames.GetName(cell), RZ_Reputation.GetTierName(p.GetTier(faction))));

		RZ_Deeds.NoteCell(m_GM, playerId, faction, cell);
		if (tierChanged)
		{
			// The tier notice leads with the name (CheckTierChange); this cell has been told
			p.m_aRepSeen[faction] = cell;
			m_mPendingAt.Remove(Key(playerId, faction));
			m_mToldAt.Set(Key(playerId, faction), m_fClock);
			return;
		}

		if (!m_mPendingAt.Contains(Key(playerId, faction)))
			m_mPendingAt.Set(Key(playerId, faction), m_fClock);
	}

	//------------------------------------------------------------------------------------------------
	protected static string Key(int playerId, int faction)
	{
		return string.Format("%1:%2", playerId, faction);
	}

	//------------------------------------------------------------------------------------------------
	//! Server tick: tells the names that have held long enough.
	void Tick(float dt)
	{
		m_fClock += dt;
		if (m_mPendingAt.IsEmpty())
			return;

		RZ_ReputationSettings settings = RZ_ReputationSettings.Get();
		array<string> done = {};
		foreach (string key, float since : m_mPendingAt)
		{
			if (m_fClock - since < settings.m_fChangeHoldSeconds)
				continue;

			float told;
			if (m_mToldAt.Find(key, told) && m_fClock - told < settings.m_fChangeGapMinutes * 60)
				continue;

			done.Insert(key);
			array<string> parts = {};
			key.Split(":", parts, false);
			if (parts.Count() < 2)
				continue;

			int playerId = parts[0].ToInt();
			int faction = parts[1].ToInt();
			Tell(playerId, faction);
			m_mToldAt.Set(key, m_fClock);
		}

		foreach (string k : done)
		{
			m_mPendingAt.Remove(k);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Tell(int playerId, int faction)
	{
		RZ_PlayerProfile p = m_GM.FindProfile(playerId);
		if (!p)
			return;

		p.EnsureArrays();
		int fame = p.GetFame(faction);
		int infamy = p.GetInfamy(faction);
		int cell = RZ_RepNames.GetRep(fame, infamy);
		int before = p.m_aRepSeen[faction];
		p.m_aRepSeen[faction] = cell;
		if (cell == before || !RZ_ReputationSettings.Get().m_bNames)
			return;

		string text = string.Format("They call you %1 now.", RZ_RepNames.NameWithRule(fame, infamy));
		if (RZ_RepNames.IsMixed(cell))
			text += " People can like you and fear you at once.";

		m_GM.NotifyPlayerAs(playerId, RZ_Factions.GetName(faction), text, RZ_ENoticeTag.HIGH);
		RZ_Guidance guide = m_GM.GetGuidance();
		if (guide && RZ_RepNames.IsMixed(cell))
			guide.Fire(playerId, "mixed", RZ_Factions.GetName(faction), RZ_RepNames.GetName(cell));
	}

	//------------------------------------------------------------------------------------------------
	void Forget(int playerId)
	{
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			m_mPendingAt.Remove(Key(playerId, f));
		}
	}

	// ================================================================ The admin read-out
	//------------------------------------------------------------------------------------------------
	//! "/rep <name>" (server admins): a player's cells, Earned Fame and Feared, the budget left,
	//! deeds and tallies, to the admin only and to the log. An empty name reads the admin.
	void Readout(int adminId, string name)
	{
		if (!RZ_NoticeBoard.IsAdmin(adminId))
		{
			m_GM.NotifyPlayer(adminId, "Reputation", "Only a server admin can read someone's standing.");
			return;
		}

		int target = FindPlayer(name);
		if (name.Trim().IsEmpty())
			target = adminId;

		if (target <= 0)
		{
			m_GM.NotifyPlayer(adminId, "Reputation", "Nobody online by that name: " + name);
			return;
		}

		RZ_PlayerProfile p = m_GM.GetProfile(target);
		p.EnsureArrays();
		string text = string.Format("%1, life %2:\n", RZ_Util.PlayerName(target), p.m_iLives);
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			int fame = p.GetFame(f);
			int infamy = p.GetInfamy(f);
			text += string.Format("%1: %2 (%3), Fame %4 earned %5, Infamy %6 feared %7, %8 left to forgive\n", RZ_Factions.GetName(f),
				RZ_RepNames.GetName(RZ_RepNames.GetRep(fame, infamy)), RZ_Reputation.GetTierName(p.GetTier(f)), fame, p.EarnedFame(f),
				infamy, p.Feared(f), p.ForgiveLeft(f));
			text += string.Format("   best: %1. worst: %2. kills %3, jobs %4, threats %5\n", Or(p.m_aBestDeed[f]), Or(RZ_Deeds.WorstText(p, f)),
				p.m_aKills[f], p.m_aJobs[f], p.m_aThreats[f]);
		}

		RZ_Log.Info("Reputation read-out: " + text);
		m_GM.NotifyPlayer(adminId, "Reputation", text);
	}

	//------------------------------------------------------------------------------------------------
	protected static string Or(string text)
	{
		if (text.IsEmpty())
			return "-";

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! The online player whose name matches `name` (whole, any case; else the first that contains it).
	protected int FindPlayer(string name)
	{
		string want = name.Trim();
		want.ToLower();
		if (want.IsEmpty())
			return 0;

		array<int> ids = {};
		GetGame().GetPlayerManager().GetPlayers(ids);
		int partial = 0;
		foreach (int id : ids)
		{
			string have = RZ_Util.PlayerName(id);
			have.ToLower();
			if (have == want)
				return id;

			if (partial == 0 && have.Contains(want))
				partial = id;
		}

		return partial;
	}
}
