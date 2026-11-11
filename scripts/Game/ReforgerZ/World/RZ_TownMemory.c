//! Town memory (docs/DESIGN.md, "Town memory"). Plain values per settlement, never objects in the
//! world: Looted, Barricaded, Powered and Burned, and how much loot a town still has (it comes
//! back over about two hours). Nothing here spawns anything; the world interactions
//! (Interactions/: search, barricades, fire) report into it through the static hooks, and read it back.
//!
//! Also the dead characters' journals: a few lines of data per life that ended, which surface
//! as a headline or a wanderer's rumour, never as an object.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > World > Town memory.
[BaseContainerProps()]
class RZ_TownMemorySettings
{
	[Attribute("1", desc: "Town memory on: Looted, Barricaded, Powered, Burned per town, loot that returns, journals of the dead (docs/DESIGN.md, Town memory)")]
	bool m_bEnabled;

	[Attribute("120", desc: "Real minutes for a fully searched town's loot to come all the way back")]
	float m_fLootReturnMinutes;

	[Attribute("0.5", desc: "A town counts as Looted while its loot level is under this (0..1)")]
	float m_fLootedBelow;

	[Attribute("0.2", desc: "Loot level a town never drops below, so nobody is locked out")]
	float m_fLootFloor;

	[Attribute("0.5", desc: "A town counts as Burned once this share of its buildings has burned")]
	float m_fBurnedShare;

	[Attribute("60", desc: "Real minutes for one burned building to stop counting (towns recover)")]
	float m_fBurnFadeMinutes;

	[Attribute("10", desc: "Journals: a life must last this many minutes to leave one")]
	float m_fJournalMinutes;
}

//------------------------------------------------------------------------------------------------
class RZ_TownSave
{
	string name;
	float loot = 1;
	bool barricaded;
	bool powered;
	int burned;
	int buildings;
}

//------------------------------------------------------------------------------------------------
class RZ_JournalSave
{
	string line;
	string place;
	float at;
}

//------------------------------------------------------------------------------------------------
class RZ_TownMemorySave
{
	ref array<ref RZ_TownSave> towns = {};
	ref array<ref RZ_JournalSave> journals = {};
}

//------------------------------------------------------------------------------------------------
//! One town's memory.
class RZ_TownState
{
	float m_fLoot = 1;			// 0..1, what's left to find
	bool m_bBarricaded;
	bool m_bPowered;
	int m_iBurned;				// buildings burned (reported by a fire system) that still count
	int m_iBuildings;			// 0 = estimate from the town's size
	ref array<float> m_aBurnAge = {};	// seconds since each counted burn, oldest first

	//------------------------------------------------------------------------------------------------
	//! One more burned building (the oldest stops counting if the town is full of them).
	void AddBurn(int cap)
	{
		m_aBurnAge.Insert(0);
		while (m_aBurnAge.Count() > Math.Max(0, cap))
		{
			m_aBurnAge.RemoveOrdered(0);
		}

		m_iBurned = m_aBurnAge.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Each burn stops counting `fadeAfter` seconds after it happened.
	void AgeBurns(float dt, float fadeAfter)
	{
		for (int i = m_aBurnAge.Count() - 1; i >= 0; i--)
		{
			m_aBurnAge[i] = m_aBurnAge[i] + dt;
			if (m_aBurnAge[i] >= fadeAfter)
				m_aBurnAge.RemoveOrdered(i);
		}

		m_iBurned = m_aBurnAge.Count();
	}
}

//------------------------------------------------------------------------------------------------
class RZ_TownMemory
{
	static const int FLAG_LOOTED = 1;
	static const int FLAG_BARRICADED = 2;
	static const int FLAG_POWERED = 4;
	static const int FLAG_BURNED = 8;

	protected static const int JOURNALS_KEPT = 6;
	protected static const float TOWN_MARGIN = 100;

	protected RZ_GameModeComponent m_GM;
	protected RZ_TownMemorySettings m_Settings;
	protected ref array<ref RZ_TownState> m_aTowns = {};
	protected ref array<ref RZ_JournalSave> m_aJournals = {};
	protected ref RZ_TownMemorySave m_Restored;
	protected float m_fClock;

	//------------------------------------------------------------------------------------------------
	void RZ_TownMemory(RZ_GameModeComponent gm, RZ_TownMemorySettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled() { return m_Settings.m_bEnabled; }

	//------------------------------------------------------------------------------------------------
	protected static RZ_TownMemory Get()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return null;

		RZ_TownMemory mem = gm.GetTownMemory();
		if (!mem || !mem.IsEnabled())
			return null;

		return mem;
	}

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	void Restore(RZ_TownMemorySave saved)
	{
		m_Restored = saved;
	}

	//------------------------------------------------------------------------------------------------
	RZ_TownMemorySave Save()
	{
		RZ_TownMemorySave data = new RZ_TownMemorySave();
		foreach (int i, RZ_TownState t : m_aTowns)
		{
			RZ_TownSave ts = new RZ_TownSave();
			RZ_Settlement s = m_GM.GetSettlement(i);
			if (s)
				ts.name = s.m_sName;

			ts.loot = t.m_fLoot;
			ts.barricaded = t.m_bBarricaded;
			ts.powered = t.m_bPowered;
			ts.burned = t.m_iBurned;
			ts.buildings = t.m_iBuildings;
			data.towns.Insert(ts);
		}

		foreach (RZ_JournalSave j : m_aJournals)
		{
			data.journals.Insert(j);
		}

		return data;
	}

	//------------------------------------------------------------------------------------------------
	//! After settlements exist. Saved towns are matched by name and order.
	void Setup()
	{
		m_aTowns.Clear();
		array<ref RZ_Settlement> list = m_GM.GetSettlements();
		foreach (int i, RZ_Settlement s : list)
		{
			RZ_TownState t = new RZ_TownState();
			if (m_Restored && m_Restored.towns && i < m_Restored.towns.Count() && m_Restored.towns[i].name == s.m_sName)
			{
				RZ_TownSave ts = m_Restored.towns[i];
				t.m_fLoot = Math.Clamp(ts.loot, 0, 1);
				// Barricaded isn't restored: the barricades themselves aren't saved, so nothing would
				// ever clear it. Powered comes back from the power grid itself (RZ_PowerGrid).
				// Restored burns each count for a full fade time from the restart.
				t.m_iBuildings = Math.Max(0, ts.buildings);
				int cap = t.m_iBuildings;
				if (cap <= 0)
					cap = BuildingCount(i); // t isn't in the list yet: the guess by size

				for (int b = 0; b < ts.burned; b++)
				{
					t.AddBurn(cap);
				}
			}

			m_aTowns.Insert(t);
		}

		if (m_Restored && m_Restored.journals)
		{
			foreach (RZ_JournalSave j : m_Restored.journals)
			{
				if (j && !j.line.IsEmpty())
					m_aJournals.Insert(j);
			}
		}

		m_Restored = null;
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt)
	{
		if (!m_Settings.m_bEnabled)
			return;

		m_fClock += dt;
		float back = dt / Math.Max(1, m_Settings.m_fLootReturnMinutes * 60);
		float fadeEvery = Math.Max(1, m_Settings.m_fBurnFadeMinutes) * 60;
		foreach (RZ_TownState t : m_aTowns)
		{
			if (t.m_fLoot < 1)
				t.m_fLoot = Math.Min(1, t.m_fLoot + back);

			if (t.m_iBurned > 0)
				t.AgeBurns(dt, fadeEvery);
		}
	}

	// ================================================================ Towns
	//------------------------------------------------------------------------------------------------
	//! The settlement pos is in (within its radius plus a margin), -1 if none.
	int TownAt(vector pos)
	{
		int idx = m_GM.FindNearestSettlement(pos);
		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (!s || RZ_Util.DistXZ(pos, s.m_vCenter) > s.m_fRadius + TOWN_MARGIN)
			return -1;

		return idx;
	}

	//------------------------------------------------------------------------------------------------
	RZ_TownState GetTown(int idx)
	{
		if (idx < 0 || idx >= m_aTowns.Count())
			return null;

		return m_aTowns[idx];
	}

	//------------------------------------------------------------------------------------------------
	//! Buildings in a town: the count a fire or building system reported, else a guess by size.
	int BuildingCount(int idx)
	{
		RZ_TownState t = GetTown(idx);
		if (t && t.m_iBuildings > 0)
			return t.m_iBuildings;

		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (!s)
			return 20;

		if (s.m_iSize >= 3)
			return 60;

		if (s.m_iSize == 2)
			return 30;

		return 12;
	}

	//------------------------------------------------------------------------------------------------
	//! FLAG_* bits for a town.
	int GetFlags(int idx)
	{
		RZ_TownState t = GetTown(idx);
		if (!t)
			return 0;

		int flags = 0;
		if (t.m_fLoot < m_Settings.m_fLootedBelow)
			flags = flags | FLAG_LOOTED;

		if (t.m_bBarricaded)
			flags = flags | FLAG_BARRICADED;

		if (t.m_bPowered)
			flags = flags | FLAG_POWERED;

		if (t.m_iBurned >= Math.Max(1, BuildingCount(idx) * m_Settings.m_fBurnedShare))
			flags = flags | FLAG_BURNED;

		return flags;
	}

	//------------------------------------------------------------------------------------------------
	bool HasFlag(int idx, int flag)
	{
		return (GetFlags(idx) & flag) != 0;
	}

	//------------------------------------------------------------------------------------------------
	//! "looted, powered" ("" if nothing).
	string DescribeFlags(int idx)
	{
		int flags = GetFlags(idx);
		array<string> words = {};
		if ((flags & FLAG_LOOTED) != 0)
			words.Insert("picked over");

		if ((flags & FLAG_BARRICADED) != 0)
			words.Insert("barricaded");

		if ((flags & FLAG_POWERED) != 0)
			words.Insert("has power");

		if ((flags & FLAG_BURNED) != 0)
			words.Insert("burned out");

		string text = "";
		foreach (int i, string w : words)
		{
			if (i > 0)
				text += ", ";

			text += w;
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! The power grid (RZ_PowerGrid) marks the towns in its region.
	void SetPowered(int idx, bool on)
	{
		RZ_TownState t = GetTown(idx);
		if (t)
			t.m_bPowered = on;
	}

	// ================================================================ Hooks for later systems
	//------------------------------------------------------------------------------------------------
	//! Hook for the search system: how much loot the town at pos still has (0..1, never under
	//! the floor). 1 outside towns. Scale what a search finds by it.
	static float GetLootLevel(vector pos)
	{
		RZ_TownMemory mem = Get();
		if (!mem)
			return 1;

		RZ_TownState t = mem.GetTown(mem.TownAt(pos));
		if (!t)
			return 1;

		return Math.Max(mem.m_Settings.m_fLootFloor, t.m_fLoot);
	}

	//------------------------------------------------------------------------------------------------
	//! Hook for the search system: a search in the town at pos took `share` of its loot
	//! (e.g. 0.02 per container). It comes back on its own over about two hours.
	static void ReportSearched(vector pos, float share)
	{
		RZ_TownMemory mem = Get();
		if (!mem)
			return;

		RZ_TownState t = mem.GetTown(mem.TownAt(pos));
		if (t)
			t.m_fLoot = Math.Clamp(t.m_fLoot - share, mem.m_Settings.m_fLootFloor, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Hook for building or fortification mods: the town at pos is barricaded (or not any more).
	static void ReportBarricaded(vector pos, bool on)
	{
		RZ_TownMemory mem = Get();
		if (!mem)
			return;

		RZ_TownState t = mem.GetTown(mem.TownAt(pos));
		if (t)
			t.m_bBarricaded = on;
	}

	//------------------------------------------------------------------------------------------------
	//! Hook for the fire system: one building burned at pos. The town counts as Burned once
	//! most of its buildings have (Burned share); each one stops counting after a while.
	static void ReportBuildingBurned(vector pos)
	{
		RZ_WorldWriters.Fire(pos);
		RZ_TownMemory mem = Get();
		if (!mem)
			return;

		int idx = mem.TownAt(pos);
		RZ_TownState t = mem.GetTown(idx);
		if (!t)
			return;

		bool wasBurned = mem.HasFlag(idx, FLAG_BURNED);
		t.AddBurn(mem.BuildingCount(idx));
		if (!wasBurned && mem.HasFlag(idx, FLAG_BURNED))
			mem.OnTownBurned(idx);
	}

	//------------------------------------------------------------------------------------------------
	//! Hook for the fire or building system: the real number of buildings in the town at pos
	//! (replaces the guess by size).
	static void ReportBuildingCount(vector pos, int count)
	{
		RZ_TownMemory mem = Get();
		if (!mem)
			return;

		RZ_TownState t = mem.GetTown(mem.TownAt(pos));
		if (t)
			t.m_iBuildings = Math.Max(0, count);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTownBurned(int idx)
	{
		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (!s)
			return;

		if (m_GM.GetLeaders())
			m_GM.GetLeaders().AddHeadline(string.Format("%1 has burned. Smoke over the roofs since morning, and nobody went to put it out.", s.m_sName), false);

		RZ_WorldWriters.TownBurned(idx, s.m_sName);
	}

	// ================================================================ Journals of the dead
	//------------------------------------------------------------------------------------------------
	//! A player's life ended at pos (died or turned). A life that lasted long enough leaves a
	//! journal: one line of data, later a headline or a wanderer's rumour. Never an object.
	void RecordJournal(int playerId, vector pos)
	{
		if (!m_Settings.m_bEnabled || playerId <= 0)
			return;

		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		if (!profile || profile.m_fLifeSeconds < m_Settings.m_fJournalMinutes * 60)
			return;

		RZ_Background bg = m_GM.GetPlayerBackground(playerId);
		string place = RZ_SettlementScanner.GridReference(pos);
		int idx = m_GM.FindNearestSettlement(pos);
		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (s)
			place = s.m_sName;

		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		int day = 11;
		if (m_GM.GetBroadcast())
			day = m_GM.GetBroadcast().GetDay();

		string who = "someone";
		if (bg)
			who = bg.m_sName;

		string goal = RZ_Objectives.Describe(m_GM, profile);
		string body = RZ_NoteText.PersonalLine(rng, place);
		if (!goal.IsEmpty() && rng.Chance(0.5))
			body = "Still trying to do what I came out for. " + body;

		string lasted = string.Format("%1 hours in", RZ_Util.RoundToInt(profile.m_fLifeSeconds / 3600));
		if (profile.m_fLifeSeconds < 5400)
			lasted = string.Format("%1 minutes in", RZ_Util.RoundToInt(profile.m_fLifeSeconds / 60));

		RZ_JournalSave j = new RZ_JournalSave();
		j.place = place;
		j.at = m_fClock;
		j.line = string.Format("Found near %1: a journal (%2, %3). Last page, %4: \"%5\"",
			place, who, lasted, RZ_NoteText.Day(day), body);
		m_aJournals.Insert(j);
		while (m_aJournals.Count() > JOURNALS_KEPT)
		{
			m_aJournals.RemoveOrdered(0); // the oldest goes; the newest stays last (LatestJournal)
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The newest journal line for the headlines, "" if none.
	string LatestJournal()
	{
		if (m_aJournals.IsEmpty())
			return "";

		return m_aJournals[m_aJournals.Count() - 1].line;
	}

	//------------------------------------------------------------------------------------------------
	//! A wanderer's rumour about a journal they found, "" if none.
	string JournalRumour()
	{
		if (m_aJournals.IsEmpty())
			return "";

		RZ_JournalSave j = m_aJournals.GetRandomElement();
		return string.Format("\"Found a notebook near %1. Somebody's last days. I read it twice and then I couldn't any more.\"", j.place);
	}
}
