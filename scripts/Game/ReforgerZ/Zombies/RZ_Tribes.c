//! The infected tribes (docs/INFECTED.md). Phase 1 builds what they stand on: the land grid
//! (World/RZ_Land.c), the agent budget, the engine's active-AI limit, loose ones that walk
//! somewhere, heat hordes that come from towns, and admin visibility. The tribes themselves (lairs,
//! day parties, hunts, the life cycle) are planned for phase 2 and later.

//------------------------------------------------------------------------------------------------
//! Zombies > Tribes. Off: no land grid, and the countryside's roamers and heat hordes are as before.
//! The agent budget here also governs the crawlers (Zombies > Crawlers).
[BaseContainerProps()]
class RZ_TribeSettings
{
	[Attribute("1", desc: "Tribes: read the land grid (wild land, lair candidates; about 5 minutes the first time on a big map, then cached), raise the engine's active-AI limit, send loose ones walking somewhere, bring heat hordes from the nearest town. The tribes themselves are planned (docs/INFECTED.md). Off: none of this; the countryside's roamers and heat hordes as before")]
	bool m_bEnabled;

	[Attribute("0", desc: "Slots: tribes on this map whatever its wild land, 1 to 15 (the runtime test uses 2 on Arland). 0 = from the land, at Wild km2 per slot")]
	int m_iSlots;

	[Attribute("5", desc: "Wild km2 per slot: one tribe per this much wild land (phase 0 measured Everon at 24 km2: 5 slots; Cain 20 km2: 4; Arland under the floor: none)")]
	float m_fWildKm2PerSlot;

	[Attribute("100", desc: "Agent budget: living infected on the server, counting every group member (with Crawlers or Tribes on). Phase 0 measured 100 agents at a p99 frame of 11.6 ms. Mission infected count toward it but are never removed for it")]
	int m_iAgentBudget;

	[Attribute("28", desc: "Cluster share: agents near one player cluster (players within 600 m share one), or the budget divided among the clusters if that's less")]
	int m_iClusterShare;

	[Attribute("8", desc: "Agents spawned a tick at most (not entries), so a full town or a horde arrives over a few ticks")]
	int m_iAgentsPerTick;

	[Attribute("160", desc: "Active-AI limit: the engine's limit on active AI, for all AI (faction AI, wanderers and trucks share it with the infected). Raised to this at start when it's lower (default 128). 0 = leave the engine's")]
	int m_iActiveAILimit;

	[Attribute("20", UIWidgets.Slider, "Land grid: land squares (250 m) read a second while the grid is first built. Sea squares cost almost nothing, and a second's reading stops after 8 ms whatever this says", "1 100 1")]
	int m_iLandCellsPerSecond;
}

//------------------------------------------------------------------------------------------------
class RZ_Tribes
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_TribeSettings m_Settings;
	protected int m_iLimitBefore = -1;

	//------------------------------------------------------------------------------------------------
	void RZ_Tribes(RZ_GameModeComponent gm, RZ_TribeSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled() { return m_Settings && m_Settings.m_bEnabled; }
	RZ_TribeSettings GetSettings() { return m_Settings; }

	//------------------------------------------------------------------------------------------------
	//! At start: the engine's active-AI limit covers every AI on the server, so it's raised when
	//! it's lower than the setting (phase 0: 128 by default).
	void Setup()
	{
		if (!IsEnabled() || m_Settings.m_iActiveAILimit <= 0)
			return;

		AIWorld aiw = GetGame().GetAIWorld();
		if (!aiw)
			return;

		m_iLimitBefore = aiw.GetLimitOfActiveAIs();
		if (m_iLimitBefore >= m_Settings.m_iActiveAILimit)
			return;

		aiw.SetLimitOfActiveAIs(m_Settings.m_iActiveAILimit);
		RZ_Log.Info(string.Format("Tribes: the engine's active-AI limit raised from %1 to %2 (it covers faction AI, wanderers and trucks too).",
			m_iLimitBefore, aiw.GetLimitOfActiveAIs()));
	}

	//------------------------------------------------------------------------------------------------
	//! "active AI 74/160"
	static string ActiveAIText()
	{
		AIWorld aiw = GetGame().GetAIWorld();
		if (!aiw)
			return "active AI unknown";

		return string.Format("active AI %1/%2", aiw.GetCurrentNumOfActiveAIs(), aiw.GetLimitOfActiveAIs());
	}

	//------------------------------------------------------------------------------------------------
	//! The land and the slots, for the status line.
	string LandText()
	{
		RZ_Land land = m_GM.GetLand();
		if (!IsEnabled() || !land)
			return "tribes off";

		if (land.IsBuilding())
			return "land: reading the map (tribes dormant)";

		if (!land.IsReady())
			return "land: not read";

		return string.Format("land: %1 km2 wild, %2 slots, no tribes yet (planned)", RZ_Util.RoundToInt(land.GetWildKm2()), land.Slots());
	}

	// ================================================================ Admin visibility
	//------------------------------------------------------------------------------------------------
	//! /tribes from an admin: no word dumps everything; "stir" stirs the nearest town (+60) and
	//! "site" leaves a kill site at the admin's feet, to play-test the crawlers before tribes exist.
	void Command(int playerId, string text)
	{
		if (!RZ_NoticeBoard.IsAdmin(playerId))
		{
			m_GM.NotifyPlayer(playerId, "Not allowed", "Only a server admin can use /tribes.");
			return;
		}

		string word = text;
		word.ToLower();
		word = word.Trim();
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		RZ_Crawlers crawlers = m_GM.GetSpawner().GetCrawlers();
		if (me && crawlers && crawlers.IsEnabled() && word == "stir")
		{
			int idx = crawlers.AddStir(me.GetOrigin(), 60);
			RZ_Settlement s = m_GM.GetSettlement(idx);
			if (s)
				m_GM.NotifyPlayer(playerId, "Infected", string.Format("%1 is stirred (%2).", s.m_sName, RZ_Util.RoundToInt(s.m_fStir)));
			else
				m_GM.NotifyPlayer(playerId, "Infected", "No town within 1 km of you to stir.");

			return;
		}

		if (me && crawlers && crawlers.IsEnabled() && word == "site")
		{
			crawlers.AddKillSite(me.GetOrigin(), "a test by an admin");
			m_GM.NotifyPlayer(playerId, "Infected", "A kill site where you stand. The nearest town's slow ones come out to it at nightfall.");
			return;
		}

		Dump(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! /tribes (admins): everything the infected are doing, written to the server log, and a short
	//! notice to the admin. The status log prints the summary every few minutes anyway, so nothing
	//! here needs typing (docs/UI.md, Controllers).
	void Dump(int playerId)
	{
		if (playerId > 0 && !RZ_NoticeBoard.IsAdmin(playerId))
		{
			m_GM.NotifyPlayer(playerId, "Not allowed", "Only a server admin can use /tribes.");
			return;
		}

		RZ_Log.Info("Tribes dump | " + m_GM.GetSpawner().StatusText() + " | " + ActiveAIText() + " | " + LandText());
		RZ_Land land = m_GM.GetLand();
		if (IsEnabled() && land && land.IsReady())
			land.LogSlots();

		RZ_Crawlers crawlers = m_GM.GetSpawner().GetCrawlers();
		if (crawlers)
			crawlers.Dump();

		m_GM.GetSpawner().DumpTracked();
		m_GM.GetBiteDetector().DumpHits();
		RZ_Log.Info("Tribes dump | tribes: none (phase 2 builds them)");

		if (playerId > 0)
			m_GM.NotifyPlayer(playerId, "Infected", "Written to the server log: towns, parts, every tracked unit and the budget.");
	}
}
