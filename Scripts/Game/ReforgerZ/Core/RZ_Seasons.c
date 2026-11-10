//! Seasons (docs/DESIGN.md, "Seasons"): the starting rules a fresh season takes from the last
//! winner (docs/LORE.md, "Endings"), the rule sets admins pick (Winter, Fog, Military Collapse,
//! No Vehicles, or their own), and the wipe cycle (on the cure after an epilogue, or after a
//! number of weeks). Everything here runs on the server.
//!
//! Effects are small nudges to systems that already exist, read through RZ_Seasons.Mods(). They are
//! capped when they are combined (RZ_SeasonMods.Cap), so no mix of winner and rule sets runs away.

//! When the server wipes the season (Season > Wipe).
enum RZ_EWipeMode
{
	OFF,			// only by hand (Wipe on start, or deleting state.json)
	ON_CURE,		// after the cure, once the epilogue is over
	AFTER_WEEKS,	// a fixed number of real weeks after the season started
	FIRST_OF_BOTH	// whichever comes first
}

//------------------------------------------------------------------------------------------------
//! One set of season rules: a rule set an admin picks by name, or what a winning faction leaves
//! behind. Every field is neutral by default; only what's set changes anything.
[BaseContainerProps()]
class RZ_SeasonRuleSet
{
	[Attribute("", desc: "Name it is picked by (Season > Rule sets). An entry with a built-in name (Winter, Fog, Military Collapse, No Vehicles) replaces it")]
	string m_sName;

	[Attribute("", desc: "One line for players, told on their first spawn of a session")]
	string m_sDescription;

	[Attribute("1", desc: "Fresh season only: every town's starting infection x this (0.6-1.3 after combining)")]
	float m_fStartInfectionMult = 1;

	[Attribute("0", desc: "Fresh season only: this many more of the quietest towns start almost clean (at most 3 after combining)")]
	int m_iSafeTowns = 0;

	[Attribute("1", desc: "Infection spread x this (0.75-1.25 after combining)")]
	float m_fSpreadMult = 1;

	[Attribute("1", desc: "Chance an infected is a special x this (0.8-1.4 after combining)")]
	float m_fSpecialMult = 1;

	[Attribute("1", desc: "At night: specials x this on top (1-1.3 after combining)")]
	float m_fNightSpecialMult = 1;

	[Attribute("1", desc: "At night: how far the infected hear (the heat spread to neighbouring squares) x this (1-2 after combining)")]
	float m_fNightHearingMult = 1;

	[Attribute("1", desc: "Food a safehouse eats at each upkeep x this (1-1.5 after combining)")]
	float m_fUpkeepMult = 1;

	[Attribute("1", desc: "Fog counts as fog from thinner fog: the fog threshold is divided by this (1-2 after combining)")]
	float m_fFogMult = 1;

	[Attribute("1", desc: "Fog Bank events come this many times as often (1-3 after combining)")]
	float m_fFogBankMult = 1;

	[Attribute("1", desc: "The closest distance infected spawn from players x this, all the time (shorter sight; 0.7-1 after combining)")]
	float m_fSightMult = 1;

	[Attribute("0", desc: "Fresh season only: this many more Haven safehouses (at most 2 more after combining)")]
	int m_iSafehouseBonus = 0;

	[Attribute("0", desc: "Road posts up at once: this many more (at most 2 more after combining, 6 in all)")]
	int m_iRoadPostBonus = 0;

	[Attribute("-1", desc: "Share of new road posts that are Bastion checkpoints rather than Red Hand roadblocks (0-1). -1 = unchanged (half)")]
	float m_fBastionPostShare = -1;

	[Attribute("0", desc: "Curfew: Bastion keeps its full guard at night, and its checkpoints go up first after dark")]
	bool m_bCurfew;

	[Attribute("1", desc: "Military loot: the rare-find chance at police stations and on bodies x this (1-2 after combining)")]
	float m_fMilitaryLootMult = 1;

	[Attribute("0", desc: "Black market everywhere: the Red Hand's fence opens at Known (not Trusted) and holds two more stolen doses")]
	bool m_bOpenBlackMarket;

	[Attribute("0", desc: "The Haven's strength: starts (fresh season) and settles this much above its middle (-20 to 20 after combining)")]
	int m_iHavenStrength = 0;

	[Attribute("0", desc: "Bastion's strength offset, as above")]
	int m_iBastionStrength = 0;

	[Attribute("0", desc: "The Red Hand's strength offset, as above")]
	int m_iRedHandStrength = 0;

	[Attribute("0", desc: "Halcyon's strength offset, as above")]
	int m_iHalcyonStrength = 0;

	[Attribute("0", desc: "No vehicles: no fixable cars, no supply trucks (so no shortages), no contracts that need a vehicle")]
	bool m_bNoVehicles;

	//------------------------------------------------------------------------------------------------
	protected static RZ_SeasonRuleSet Make(string name, string description)
	{
		RZ_SeasonRuleSet r = new RZ_SeasonRuleSet();
		r.m_sName = name;
		r.m_sDescription = description;
		return r;
	}

	//------------------------------------------------------------------------------------------------
	//! The four built-in rule sets.
	static void CreateDefaults(notnull array<ref RZ_SeasonRuleSet> list)
	{
		// Winter: nights matter more. Real cold is a weather or temperature mod's job.
		RZ_SeasonRuleSet winter = Make("Winter", "Winter. The nights are long and hungry: they hear further in the dark, more of the strange ones come out, and the safehouses burn through their food.");
		winter.m_fNightSpecialMult = 1.2;
		winter.m_fNightHearingMult = 1.5;
		winter.m_fUpkeepMult = 1.3;
		list.Insert(winter);

		RZ_SeasonRuleSet fog = Make("Fog", "Fog season. It rolls in off the low ground most days. You won't see them until they're close.");
		fog.m_fFogMult = 1.6;
		fog.m_fFogBankMult = 2.5;
		fog.m_fSightMult = 0.85;
		list.Insert(fog);

		RZ_SeasonRuleSet collapse = Make("Military Collapse", "Bastion is coming apart. Fewer checkpoints, more Red Hand on the roads, and military kit lying where soldiers dropped it.");
		collapse.m_iBastionStrength = -15;
		collapse.m_iRedHandStrength = 10;
		collapse.m_fBastionPostShare = 0.25;
		collapse.m_fMilitaryLootMult = 1.5;
		list.Insert(collapse);

		RZ_SeasonRuleSet walk = Make("No Vehicles", "No vehicles. The fuel's gone, the cars are dead, and even the factions walk their supplies now.");
		walk.m_bNoVehicles = true;
		list.Insert(walk);
	}

	//------------------------------------------------------------------------------------------------
	//! What last season's winner leaves behind (docs/LORE.md, "Endings"). Modest by design.
	static RZ_SeasonRuleSet ForWinner(int faction)
	{
		RZ_SeasonRuleSet r;
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				r = Make("The Haven's season", "Last season the Haven had the cure. It spread hand to hand, and some towns never quite lost it: more safe towns, one more safehouse.");
				r.m_iSafeTowns = 2;
				r.m_iSafehouseBonus = 1;
				r.m_iHavenStrength = 5;
				break;
			}
			case RZ_EFaction.MILITARY:
			{
				r = Make("Bastion's season", "Last season Bastion had the cure, and kept it for those who complied. Stronger checkpoints, a curfew after dark, more military kit about.");
				r.m_iRoadPostBonus = 1;
				r.m_fBastionPostShare = 0.7;
				r.m_bCurfew = true;
				r.m_fMilitaryLootMult = 1.3;
				r.m_iBastionStrength = 5;
				break;
			}
			case RZ_EFaction.BANDITS:
			{
				r = Make("The Red Hand's season", "Last season the Red Hand sold the cure. The roads are theirs: more roadblocks, and the black market open to anyone they know.");
				r.m_iRoadPostBonus = 1;
				r.m_fBastionPostShare = 0.3;
				r.m_bOpenBlackMarket = true;
				r.m_iRedHandStrength = 5;
				break;
			}
			case RZ_EFaction.INSTITUTE:
			{
				r = Make("Halcyon's season", "Last season Halcyon published the cure. Infection is lower everywhere, but the infected that are left have learned: more specials.");
				r.m_fStartInfectionMult = 0.8;
				r.m_fSpreadMult = 0.9;
				r.m_fSpecialMult = 1.15;
				r.m_iHalcyonStrength = 5;
				break;
			}
		}

		return r;
	}
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Season.
[BaseContainerProps()]
class RZ_SeasonSettings
{
	[Attribute("1", desc: "A fresh season starts with the last winner's rules (last_season.json; docs/LORE.md, Endings)")]
	bool m_bWinnerRules = true;

	[Attribute("", desc: "Rule sets for this season, by name, comma-separated (Winter, Fog, Military Collapse, No Vehicles, or your own below). Empty = none")]
	string m_sRuleSets;

	[Attribute(desc: "Your own rule sets, or replacements for the built-in ones (same name). Empty = the built-in four")]
	ref array<ref RZ_SeasonRuleSet> m_aRuleSets;

	[Attribute("1", UIWidgets.ComboBox, "Wipe: Off (by hand), On cure (after the epilogue), After weeks, or whichever comes first. A wipe backs up state.json and starts fresh on the next restart", "", ParamEnumArray.FromEnum(RZ_EWipeMode))]
	RZ_EWipeMode m_eWipeMode = RZ_EWipeMode.ON_CURE;

	[Attribute("48", desc: "Epilogue: real hours the world stays open after the cure before the wipe is due")]
	float m_fEpilogueHours = 48;

	[Attribute("4", desc: "After weeks: real weeks from the season's start until the wipe is due")]
	int m_iWipeWeeks = 4;

	[Attribute("24", desc: "Players are warned this many real hours before the wipe is due (every hour, and on first spawn)")]
	float m_fWarnHours = 24;

	[Attribute("1", desc: "Hall of fame: samples, the final stand, rescues, longest life, missions and each faction's Champion, per season and all time (hall_of_fame.json, never wiped)")]
	bool m_bHallOfFame = true;
}

//------------------------------------------------------------------------------------------------
//! The effective season rules: the winner's and the picked rule sets, combined and capped.
class RZ_SeasonMods
{
	float m_fStartInfectionMult = 1;
	int m_iSafeTowns;
	float m_fSpreadMult = 1;
	float m_fSpecialMult = 1;
	float m_fNightSpecialMult = 1;
	float m_fNightHearingMult = 1;
	float m_fUpkeepMult = 1;
	float m_fFogMult = 1;
	float m_fFogBankMult = 1;
	float m_fSightMult = 1;
	int m_iSafehouseBonus;
	int m_iRoadPostBonus;
	float m_fBastionPostShare = 0.5;
	bool m_bCurfew;
	float m_fMilitaryLootMult = 1;
	bool m_bOpenBlackMarket;
	ref array<int> m_aStrength = {0, 0, 0, 0};
	bool m_bNoVehicles;
	ref array<string> m_aLines = {};	// one line per rule set, for players
	ref array<string> m_aNames = {};

	protected bool m_bShareSet;

	//------------------------------------------------------------------------------------------------
	void Add(RZ_SeasonRuleSet r)
	{
		if (!r)
			return;

		m_fStartInfectionMult = m_fStartInfectionMult * r.m_fStartInfectionMult;
		m_iSafeTowns += r.m_iSafeTowns;
		m_fSpreadMult = m_fSpreadMult * r.m_fSpreadMult;
		m_fSpecialMult = m_fSpecialMult * r.m_fSpecialMult;
		m_fNightSpecialMult = m_fNightSpecialMult * r.m_fNightSpecialMult;
		m_fNightHearingMult = m_fNightHearingMult * r.m_fNightHearingMult;
		m_fUpkeepMult = m_fUpkeepMult * r.m_fUpkeepMult;
		m_fFogMult = m_fFogMult * r.m_fFogMult;
		m_fFogBankMult = m_fFogBankMult * r.m_fFogBankMult;
		m_fSightMult = m_fSightMult * r.m_fSightMult;
		m_iSafehouseBonus += r.m_iSafehouseBonus;
		m_iRoadPostBonus += r.m_iRoadPostBonus;
		if (r.m_fBastionPostShare >= 0)
		{
			// Two rule sets that both set it meet in the middle
			if (m_bShareSet)
				m_fBastionPostShare = (m_fBastionPostShare + r.m_fBastionPostShare) * 0.5;
			else
				m_fBastionPostShare = r.m_fBastionPostShare;

			m_bShareSet = true;
		}

		m_bCurfew = m_bCurfew || r.m_bCurfew;
		m_fMilitaryLootMult = m_fMilitaryLootMult * r.m_fMilitaryLootMult;
		m_bOpenBlackMarket = m_bOpenBlackMarket || r.m_bOpenBlackMarket;
		m_aStrength[RZ_EFaction.SURVIVORS] = m_aStrength[RZ_EFaction.SURVIVORS] + r.m_iHavenStrength;
		m_aStrength[RZ_EFaction.MILITARY] = m_aStrength[RZ_EFaction.MILITARY] + r.m_iBastionStrength;
		m_aStrength[RZ_EFaction.BANDITS] = m_aStrength[RZ_EFaction.BANDITS] + r.m_iRedHandStrength;
		m_aStrength[RZ_EFaction.INSTITUTE] = m_aStrength[RZ_EFaction.INSTITUTE] + r.m_iHalcyonStrength;
		m_bNoVehicles = m_bNoVehicles || r.m_bNoVehicles;
		m_aNames.Insert(r.m_sName);
		if (!r.m_sDescription.IsEmpty())
			m_aLines.Insert(r.m_sDescription);
	}

	//------------------------------------------------------------------------------------------------
	//! Nothing overpowered: every combined effect stays in a modest band.
	void Cap()
	{
		m_fStartInfectionMult = Math.Clamp(m_fStartInfectionMult, 0.6, 1.3);
		m_iSafeTowns = Math.ClampInt(m_iSafeTowns, 0, 3);
		m_fSpreadMult = Math.Clamp(m_fSpreadMult, 0.75, 1.25);
		m_fSpecialMult = Math.Clamp(m_fSpecialMult, 0.8, 1.4);
		m_fNightSpecialMult = Math.Clamp(m_fNightSpecialMult, 1, 1.3);
		m_fNightHearingMult = Math.Clamp(m_fNightHearingMult, 1, 2);
		m_fUpkeepMult = Math.Clamp(m_fUpkeepMult, 1, 1.5);
		m_fFogMult = Math.Clamp(m_fFogMult, 1, 2);
		m_fFogBankMult = Math.Clamp(m_fFogBankMult, 1, 3);
		m_fSightMult = Math.Clamp(m_fSightMult, 0.7, 1);
		m_iSafehouseBonus = Math.ClampInt(m_iSafehouseBonus, 0, 2);
		m_iRoadPostBonus = Math.ClampInt(m_iRoadPostBonus, 0, 2);
		m_fBastionPostShare = Math.Clamp(m_fBastionPostShare, 0, 1);
		m_fMilitaryLootMult = Math.Clamp(m_fMilitaryLootMult, 1, 2);
		for (int f = 0; f < m_aStrength.Count(); f++)
		{
			m_aStrength[f] = Math.ClampInt(m_aStrength[f], -20, 20);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A faction's strength offset (territory), -20 to 20.
	int StrengthOffset(int faction)
	{
		if (faction < 0 || faction >= m_aStrength.Count())
			return 0;

		return m_aStrength[faction];
	}

	//------------------------------------------------------------------------------------------------
	bool IsNeutral()
	{
		return m_aNames.IsEmpty();
	}
}

//------------------------------------------------------------------------------------------------
//! The wipe marker: written when a wipe is due, read (and closed) on the next start.
class RZ_WipeMarker
{
	bool pending;
	int season;
	string reason;
}

//------------------------------------------------------------------------------------------------
//! The season's clock, its rules and the wipe cycle. Owned by RZ_GameModeComponent.
class RZ_Seasons
{
	protected static const string WIPE_PATH = "$profile:ReforgerZ/wipe.json";
	protected static const float CHECK_INTERVAL = 60;
	protected static const int HOUR = 3600;
	protected static const int DAY = 86400;

	protected static ref RZ_SeasonMods s_Mods;

	protected RZ_GameModeComponent m_GM;
	protected RZ_SeasonSettings m_Settings;
	protected ref array<ref RZ_SeasonRuleSet> m_aRuleSets = {};
	protected ref RZ_HallOfFame m_Hall;

	protected int m_iSeason = 1;
	protected int m_iStartUnix;
	protected int m_iCureUnix;
	protected int m_iRulesWinner = -1;		// whose rules this season started with, -1 none
	protected bool m_bFresh;
	protected bool m_bWipeDue;
	protected string m_sWipeReason;
	protected float m_fCheckTimer;
	protected int m_iLastWarnHours = -1;

	//------------------------------------------------------------------------------------------------
	void RZ_Seasons(RZ_GameModeComponent gm, RZ_SeasonSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		RZ_SeasonRuleSet.CreateDefaults(m_aRuleSets);
		if (m_Settings.m_aRuleSets)
		{
			foreach (RZ_SeasonRuleSet own : m_Settings.m_aRuleSets)
			{
				if (!own || own.m_sName.IsEmpty())
					continue;

				int idx = FindRuleSet(own.m_sName);
				if (idx >= 0)
					m_aRuleSets.RemoveOrdered(idx);

				m_aRuleSets.Insert(own);
			}
		}

		m_Hall = new RZ_HallOfFame(gm, m_Settings.m_bHallOfFame);
		s_Mods = new RZ_SeasonMods();
	}

	//------------------------------------------------------------------------------------------------
	//! The season's effective rules (never null; neutral until the season begins).
	static RZ_SeasonMods Mods()
	{
		if (!s_Mods)
			s_Mods = new RZ_SeasonMods();

		return s_Mods;
	}

	//------------------------------------------------------------------------------------------------
	int GetSeason() { return m_iSeason; }
	bool IsFresh() { return m_bFresh; }
	bool IsWipeDue() { return m_bWipeDue; }
	RZ_HallOfFame GetHall() { return m_Hall; }

	//------------------------------------------------------------------------------------------------
	//! By name, ignoring case and spaces ("military collapse" finds Military Collapse).
	protected int FindRuleSet(string name)
	{
		string want = Norm(name);
		foreach (int i, RZ_SeasonRuleSet r : m_aRuleSets)
		{
			if (Norm(r.m_sName) == want)
				return i;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Norm(string name)
	{
		array<string> words = {};
		RZ_ItemHelper.Words(name, words);
		if (words.IsEmpty())
			return "";

		return words[0];
	}

	// ================================================================ Start
	//------------------------------------------------------------------------------------------------
	//! Before the state loads: a wipe that came due last time (wipe.json) is carried out now. The
	//! old state.json is copied to state_season<N>.json first. True = start fresh.
	static bool TakeWipeMarker()
	{
		RZ_WipeMarker marker = LoadMarker();
		if (!marker || !marker.pending)
			return false;

		RZ_SaveData old = RZ_Persistence.Load();
		if (old)
		{
			string backup = RZ_Persistence.BackupPath(Math.Max(marker.season, old.seasonNumber));
			RZ_Persistence.SaveTo(old, backup);
			RZ_Log.Info("Season wipe: the old state was copied to " + backup + ".");
		}

		marker.pending = false;
		SaveMarker(marker);
		RZ_Log.Info(string.Format("Season wipe carried out (%1). A fresh outbreak starts.", marker.reason));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_WipeMarker LoadMarker()
	{
		if (!FileIO.FileExists(WIPE_PATH))
			return null;

		SCR_JsonLoadContext ctx = new SCR_JsonLoadContext();
		if (!ctx.LoadFromFile(WIPE_PATH))
			return null;

		RZ_WipeMarker marker = new RZ_WipeMarker();
		if (!ctx.ReadValue("wipe", marker))
			return null;

		return marker;
	}

	//------------------------------------------------------------------------------------------------
	protected static void SaveMarker(RZ_WipeMarker marker)
	{
		FileIO.MakeDirectory(RZ_Persistence.DIR);
		SCR_JsonSaveContext ctx = new SCR_JsonSaveContext();
		ctx.WriteValue("wipe", marker);
		if (!ctx.SaveToFile(WIPE_PATH))
			RZ_Log.Warn("Could not write " + WIPE_PATH);
	}

	//------------------------------------------------------------------------------------------------
	//! From the save (same season after a restart).
	void Restore(RZ_SaveData data)
	{
		m_iSeason = data.seasonNumber;
		m_iStartUnix = data.seasonStartUnix;
		m_iCureUnix = data.cureUnix;
		m_iRulesWinner = data.rulesWinner;
		m_bWipeDue = data.wipeDue;
	}

	//------------------------------------------------------------------------------------------------
	void Save(RZ_SaveData data)
	{
		data.seasonNumber = m_iSeason;
		data.seasonStartUnix = m_iStartUnix;
		data.cureUnix = m_iCureUnix;
		data.rulesWinner = m_iRulesWinner;
		data.wipeDue = m_bWipeDue;
		if (m_Hall)
			m_Hall.SaveIfChanged();
	}

	//------------------------------------------------------------------------------------------------
	//! After the state loaded (or didn't). A fresh season numbers itself after the last one and
	//! takes the last winner's rules; a loaded one keeps its own. Then the rule sets are combined.
	void Begin(bool loaded, RZ_SeasonRecord last)
	{
		m_bFresh = !loaded;
		int now = RZ_Guidance.UnixNow();
		if (m_bFresh)
		{
			int previous = m_Hall.GetSeason();
			if (last)
				previous = Math.Max(previous, last.season);

			m_iSeason = previous + 1;
			m_iStartUnix = now;
			m_iCureUnix = 0;
			m_bWipeDue = false;
			m_iRulesWinner = -1;
			if (last && m_Settings.m_bWinnerRules)
				m_iRulesWinner = last.winner;
		}
		else
		{
			if (m_iSeason <= 0)
				m_iSeason = Math.Max(1, m_Hall.GetSeason());

			// Saves from before the season clock: count the weeks from now
			if (m_iStartUnix <= 0)
				m_iStartUnix = now;
		}

		m_Hall.BeginSeason(m_iSeason, m_GM.GetWorldName());
		s_Mods = BuildMods();
		string names = "none";
		if (!s_Mods.IsNeutral())
		{
			names = "";
			foreach (int n, string ruleName : s_Mods.m_aNames)
			{
				if (n > 0)
					names += ", ";

				names += ruleName;
			}
		}

		RZ_Log.Info(string.Format("Season %1 (%2). Rules: %3. Wipe: %4.", m_iSeason, BoolText(m_bFresh, "fresh", "continued"), names, DescribeWipe()));
	}

	//------------------------------------------------------------------------------------------------
	protected static string BoolText(bool b, string yes, string no)
	{
		if (b)
			return yes;

		return no;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_SeasonMods BuildMods()
	{
		RZ_SeasonMods mods = new RZ_SeasonMods();
		if (m_iRulesWinner >= 0)
			mods.Add(RZ_SeasonRuleSet.ForWinner(m_iRulesWinner));

		// Names lower-cased with spaces dropped, as FindRuleSet compares them
		array<string> picked = {};
		RZ_ItemHelper.Words(m_Settings.m_sRuleSets, picked);
		foreach (string name : picked)
		{
			int idx = FindRuleSet(name);
			if (idx < 0)
			{
				RZ_Log.Warn("Season rule set '" + name + "' doesn't exist; ignored.");
				continue;
			}

			mods.Add(m_aRuleSets[idx]);
		}

		mods.Cap();
		return mods;
	}

	//------------------------------------------------------------------------------------------------
	//! Fresh season only, once settlements exist: starting infection and the safe towns.
	void ApplyStartRules()
	{
		if (!m_bFresh)
			return;

		RZ_SeasonMods mods = Mods();
		array<ref RZ_Settlement> list = m_GM.GetSettlements();
		foreach (RZ_Settlement s : list)
		{
			s.m_fInfection = Math.Clamp(s.m_fInfection * mods.m_fStartInfectionMult, 0, 100);
		}

		// Safe towns: the quietest towns start almost clean
		for (int n = 0; n < mods.m_iSafeTowns; n++)
		{
			RZ_Settlement best;
			foreach (RZ_Settlement c : list)
			{
				if (c.m_fInfection <= SAFE_TOWN_INFECTION || c.m_bHasLab)
					continue;

				if (!best || c.m_fInfection < best.m_fInfection)
					best = c;
			}

			if (!best)
				break;

			best.m_fInfection = SAFE_TOWN_INFECTION;
			RZ_Log.Info("Season rules: " + best.m_sName + " starts as a safe town.");
		}
	}

	protected static const float SAFE_TOWN_INFECTION = 5;

	// ================================================================ The wipe cycle
	//------------------------------------------------------------------------------------------------
	void OnCureCompleted()
	{
		if (m_iCureUnix <= 0)
			m_iCureUnix = RZ_Guidance.UnixNow();

		if (m_Hall)
			m_Hall.OnSeasonWon();
	}

	//------------------------------------------------------------------------------------------------
	//! Real-world time the wipe comes due, 0 if never.
	int WipeAt()
	{
		RZ_EWipeMode mode = m_Settings.m_eWipeMode;
		int at = 0;
		if ((mode == RZ_EWipeMode.ON_CURE || mode == RZ_EWipeMode.FIRST_OF_BOTH) && m_iCureUnix > 0)
			at = m_iCureUnix + RZ_Util.RoundToInt(Math.Max(0, m_Settings.m_fEpilogueHours) * HOUR);

		if ((mode == RZ_EWipeMode.AFTER_WEEKS || mode == RZ_EWipeMode.FIRST_OF_BOTH) && m_Settings.m_iWipeWeeks > 0 && m_iStartUnix > 0)
		{
			int weeks = m_iStartUnix + m_Settings.m_iWipeWeeks * 7 * DAY;
			if (at <= 0 || weeks < at)
				at = weeks;
		}

		return at;
	}

	//------------------------------------------------------------------------------------------------
	protected string DescribeWipe()
	{
		switch (m_Settings.m_eWipeMode)
		{
			case RZ_EWipeMode.ON_CURE: return string.Format("%1 h after the cure", RZ_Util.RoundToInt(m_Settings.m_fEpilogueHours));
			case RZ_EWipeMode.AFTER_WEEKS: return string.Format("after %1 weeks", m_Settings.m_iWipeWeeks);
			case RZ_EWipeMode.FIRST_OF_BOTH: return string.Format("%1 h after the cure or after %2 weeks", RZ_Util.RoundToInt(m_Settings.m_fEpilogueHours), m_Settings.m_iWipeWeeks);
		}

		return "by hand";
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids)
	{
		if (m_Hall)
			m_Hall.Tick(dt, ids);

		m_fCheckTimer += dt;
		if (m_fCheckTimer < CHECK_INTERVAL)
			return;

		m_fCheckTimer = 0;
		if (m_bWipeDue)
			return;

		int at = WipeAt();
		int now = RZ_Guidance.UnixNow();
		if (at <= 0 || now <= 0)
			return;

		if (now >= at)
		{
			MarkWipeDue();
			return;
		}

		// Warnings: every hour inside the warning window
		int hoursLeft = (at - now + HOUR - 1) / HOUR;
		if (hoursLeft <= m_Settings.m_fWarnHours && hoursLeft != m_iLastWarnHours && !ids.IsEmpty())
		{
			m_iLastWarnHours = hoursLeft;
			m_GM.NotifyAll(RZ_Broadcast.TITLE, WarningText(hoursLeft));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string WarningText(int hoursLeft)
	{
		return string.Format("Residents are advised that the zone will be reprocessed in about %1 hour(s). Remain indoors.\n(This season ends then: the server starts a new outbreak at its next restart. The hall of fame is kept.)", hoursLeft);
	}

	//------------------------------------------------------------------------------------------------
	//! For a player's first spawn of a session: the season, its rules, and a coming wipe. "" if nothing.
	string DescribeForWelcome()
	{
		string text = string.Format("Season %1.", m_iSeason);
		RZ_SeasonMods mods = Mods();
		foreach (string line : mods.m_aLines)
		{
			text += "\n" + line;
		}

		if (m_bWipeDue)
		{
			text += "\nThis season is over. The Zone resets at the next restart.";
			return text;
		}

		int at = WipeAt();
		int now = RZ_Guidance.UnixNow();
		if (at > 0 && now > 0 && at - now <= m_Settings.m_fWarnHours * HOUR)
			text += string.Format("\nThe Zone resets in about %1 hour(s).", Math.Max(1, (at - now) / HOUR));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! The wipe came due: the season record (no winner if the cure never came), the hall's
	//! archive, the marker for the next start, and word to everyone.
	protected void MarkWipeDue()
	{
		m_bWipeDue = true;
		m_sWipeReason = "the cure and its epilogue";
		if (m_iCureUnix <= 0)
		{
			m_sWipeReason = string.Format("%1 weeks without a cure", m_Settings.m_iWipeWeeks);
			RZ_SeasonRecord record = new RZ_SeasonRecord();
			record.world = m_GM.GetWorldName();
			record.season = m_iSeason;
			record.winner = -1;
			record.winnerName = "nobody";
			record.endedBy = "time";
			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				record.fame.Insert(m_GM.GetSeasonFame(f));
			}

			RZ_Persistence.SaveSeason(record);
		}

		if (m_Hall)
			m_Hall.EndSeason(m_iSeason, m_GM.GetSeasonWinner());

		RZ_WipeMarker marker = new RZ_WipeMarker();
		marker.pending = true;
		marker.season = m_iSeason;
		marker.reason = m_sWipeReason;
		SaveMarker(marker);
		RZ_Log.Info(string.Format("Season %1 is over (%2). The wipe happens at the next restart.", m_iSeason, m_sWipeReason));
		m_GM.NotifyAll(RZ_Broadcast.TITLE, "The perimeter will be maintained.\n(This season is over. The Zone resets at the next restart; the hall of fame is kept.)");
	}
}
