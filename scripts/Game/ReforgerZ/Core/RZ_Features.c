//! Features: one switch per feature, grouped by the stage it comes back in (docs/FEATURES.md).
//! The mod is cut back to its core and brought in again a stage at a time, each tested before the
//! next. Stage 0 (the core) has no switches: it's always on.
//!
//! How a switch works: a feature runs only if both its own setting and its switch here are on. For a
//! feature that has its own on/off setting, Apply turns that setting off at start when the switch is
//! off (so nothing else changes); a feature without one asks RZ_Features.On where it starts, ticks,
//! offers or shows a prompt. A switch whose needs are off is off too (Apply, "needs").
//!
//! The server profile can override the prefab: $profile:RZ_Features.txt, one word a line: "all"
//! (every switch on), "stage N" (stages 1 to N on), a switch's name ("Shops") to turn it on, or "-Shops"
//! to turn it off; lines run in order and "#" starts a comment. The runtime test writes "all".
//! The server sends the result to the clients, so their prompts and screens agree.

//! The switches, in stage order. COUNT last.
enum RZ_EFeature
{
	// Stage 1: People
	CAMP_SITES,
	CAMP_LIFE,
	SAFEHOUSES,
	LEADERS,
	SHOPS,
	ERRANDS,
	WORK_SITES,
	WANDERERS,
	FACTION_LIFE,
	CHECKPOINTS,
	// Stage 2: Start
	GUIDANCE,
	OPENING,
	HABITS,
	PERSONAL_GOALS,
	// Stage 3: Map
	MAP_FOG,
	ROAD_PATROLS,
	SUPPLY_TRUCKS,
	TONIGHT,
	// Stage 4: Work
	CONTRACTS,
	WORLD_EVENTS,
	PUZZLES,
	LOCKS,
	CARS,
	CRAFTING,
	RADIOS,
	// Stage 5: Threat
	HORDES,
	CRAWLERS,
	TRIBES,
	BEHAVIOUR,
	STEALTH,
	HUNTED,
	LONG_TABLE,
	WEATHER,
	PERIMETER,
	// Stage 6: Story
	STORY_ARCS,
	QUESTLINES,
	MAJORS,
	MEMORY,
	// Stage 7: World
	POWER,
	RADIO_STATION,
	TRACES,
	TOWN_MEMORY,
	WILDLIFE,
	SEASON_RULES,
	// Stage 8: Endgame
	RESEARCH,
	FINALE,
	EPILOGUE,
	COUNT
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class RZ_FeatureSettings
{
	// ---- Stage 1: People
	[Attribute("1", desc: "Stage 1. Camp sites: each holding's layout (the building it moves into, its scenery, wire, boards and lookouts; hand-built sites). Off: a holding is its people at the town's centre")]
	bool m_bCampSites;

	[Attribute("1", desc: "Stage 1. Camp life: people's routines at the places nearest players (sentry, patrol, sit, cook...) and the extra people. Off: they stand. Needs Camp sites")]
	bool m_bCampLife;

	[Attribute("1", desc: "Stage 1. Haven safehouses (Haven > Enabled)")]
	bool m_bSafehouses;

	[Attribute("1", desc: "Stage 1. Named leaders and their jobs (Leaders > Enabled)")]
	bool m_bLeaders;

	[Attribute("1", desc: "Stage 1. Shops and quartermasters: buying, selling, requisition, the Trade reply. Off: a quartermaster is a person with no shop")]
	bool m_bShops;

	[Attribute("1", desc: "Stage 1. Errands and duties: named people's small jobs, the dealer, duties at their place (Errands > Enabled)")]
	bool m_bErrands;

	[Attribute("1", desc: "Stage 1. Work sites and carrying: a step's work at its place, carrying for real (Shared > Work sites). Errands and the opening use them, so they come back with stage 1")]
	bool m_bWorkSites;

	[Attribute("1", desc: "Stage 1. Wanderers on the roads (Wanderers > Enabled)")]
	bool m_bWanderers;

	[Attribute("1", desc: "Stage 1. Faction life: needs, morale, chatter, truces (Faction life > Enabled)")]
	bool m_bFactionLife;

	[Attribute("1", desc: "Stage 1. Checkpoints: road posts and camps that warn and refuse (Checkpoints > Road posts, Camps)")]
	bool m_bCheckpoints;

	// ---- Stage 2: Start
	[Attribute("1", desc: "Stage 2. Guidance: levels, tips, the wind-up radio, its open channels and the first goal (Guidance > Enabled)")]
	bool m_bGuidance;

	[Attribute("1", desc: "Stage 2. The opening: the door, the favour, the line, the case, Ferda (Guidance > Openings). Needs Guidance, Errands and Work sites")]
	bool m_bOpening;

	[Attribute("1", desc: "Stage 2. Habits: the door's answer and what it does (a shorter hold, a quieter crouch...). Off: no habit is asked for or counts")]
	bool m_bHabits;

	[Attribute("1", desc: "Stage 2. Personal goals: one small goal for each character (reach a town, stay alive, earn a faction's trust...). Off: no goal is given, shown or counted. A goal whose feature is off is never picked")]
	bool m_bPersonalGoals;

	// ---- Stage 3: Map
	[Attribute("1", desc: "Stage 3. The map's fog (Map fog > On). Off: the whole map shows")]
	bool m_bMapFog;

	[Attribute("1", desc: "Stage 3. Road patrols (Territory > Road patrols > Enabled)")]
	bool m_bRoadPatrols;

	[Attribute("1", desc: "Stage 3. Supply trucks (Territory > Supply trucks > Enabled)")]
	bool m_bSupplyTrucks;

	[Attribute("1", desc: "Stage 3. Tonight: three true things near you (UI > Tonight)")]
	bool m_bTonight;

	// ---- Stage 4: Work
	[Attribute("1", desc: "Stage 4. Contracts: the director's own offers by faction. The mission engine stays: questlines, majors, leaders and the finale run their steps through it")]
	bool m_bContracts;

	[Attribute("1", desc: "Stage 4. World events: the director's events near players")]
	bool m_bWorldEvents;

	[Attribute("1", desc: "Stage 4. Puzzles: the puzzle contracts. Needs Contracts")]
	bool m_bPuzzles;

	[Attribute("1", desc: "Stage 4. Doors and locks: locked buildings, keys and picks. Off: no door is locked")]
	bool m_bLocks;

	[Attribute("1", desc: "Stage 4. Cars: fixable cars, wrecks, hotwiring (Interactions > Cars > Enabled)")]
	bool m_bCars;

	[Attribute("1", desc: "Stage 4. Crafting at workbenches and traps (Crafting > Enabled, Traps > Enabled). The stove is core: it cooks whatever this says")]
	bool m_bCrafting;

	[Attribute("1", desc: "Stage 4. Secure radio channels and house radios (Secure radio > Enabled)")]
	bool m_bRadios;

	// ---- Stage 5: Threat
	[Attribute("1", desc: "Stage 5. Heat hordes and adaptation: loose ones and hordes from towns, specials after breakthroughs (Adaptation > Enabled, Tribes' hordes)")]
	bool m_bHordes;

	[Attribute("1", desc: "Stage 5. Crawlers: the towns' slow infected (Crawlers > Enabled)")]
	bool m_bCrawlers;

	[Attribute("1", desc: "Stage 5. Tribes: the land grid's packs (Tribes > Enabled)")]
	bool m_bTribes;

	[Attribute("1", desc: "Stage 5. The behaviour layer (Behaviour > Enabled). Needs Tribes")]
	bool m_bBehaviour;

	[Attribute("1", desc: "Stage 5. Stealth: visibility and the noise meter (Stealth > Enabled)")]
	bool m_bStealth;

	[Attribute("1", desc: "Stage 5. Being hunted: hunter squads, bounties, the bounty board, disguises. Off: Hunted still means they fire, nobody comes looking")]
	bool m_bHunted;

	[Attribute("1", desc: "Stage 5. The Long Table and its reach (Long Table > Enabled)")]
	bool m_bLongTable;

	[Attribute("1", desc: "Stage 5. Time and weather's effects on the infected and patrols (Environment > Enabled)")]
	bool m_bWeather;

	[Attribute("1", desc: "Stage 5. The Perimeter: the sealed edge (Perimeter > Enabled)")]
	bool m_bPerimeter;

	// ---- Stage 6: Story
	[Attribute("1", desc: "Stage 6. Story arcs (Story arcs > Enabled)")]
	bool m_bStoryArcs;

	[Attribute("1", desc: "Stage 6. Questlines (Questlines > Enabled)")]
	bool m_bQuestlines;

	[Attribute("1", desc: "Stage 6. The majors and the case's three papers (Majors > Enabled). Off: the case comes without its papers. Needs Questlines")]
	bool m_bMajors;

	[Attribute("1", desc: "Stage 6. Memory's spoken parts: callbacks and reputation names. The store of what happened stays on")]
	bool m_bMemory;

	// ---- Stage 7: World
	[Attribute("1", desc: "Stage 7. Power and radio: the station, the grid, substations, masts, the loop relay (Power > Enabled, Masts > Enabled)")]
	bool m_bPower;

	[Attribute("1", desc: "Stage 7. The radio station (Radio station > Enabled)")]
	bool m_bRadioStation;

	[Attribute("1", desc: "Stage 7. Traces: door marks, scenes, sounds and light (Traces > Enabled)")]
	bool m_bTraces;

	[Attribute("1", desc: "Stage 7. Town memory: looted, barricaded, powered, burned (Town memory > Enabled)")]
	bool m_bTownMemory;

	[Attribute("1", desc: "Stage 7. Wildlife (Wildlife > Enabled)")]
	bool m_bWildlife;

	[Attribute("1", desc: "Stage 7. Season rules: rule sets and the last winner's starting rules. Off: every season starts plain")]
	bool m_bSeasonRules;

	// ---- Stage 8: Endgame
	[Attribute("1", desc: "Stage 8. The cure's research: samples and missions move it, the lab works its backlog. Off: the lab takes nothing and research stays where it is")]
	bool m_bResearch;

	[Attribute("1", desc: "Stage 8. The cure finale (Finale > Enabled). Needs Research and Majors")]
	bool m_bFinale;

	[Attribute("1", desc: "Stage 8. The epilogue week (Epilogue > Enabled). Needs Finale")]
	bool m_bEpilogue;
}

//------------------------------------------------------------------------------------------------
class RZ_Features
{
	static const string FILE = "$profile:RZ_Features.txt";

	protected static ref array<bool> s_aOn;
	protected static bool s_bServer;

	//------------------------------------------------------------------------------------------------
	//! Is this feature on? On the server, what Apply settled; on a client, what the server sent.
	//! Before either (a client that hasn't heard yet), on.
	static bool On(RZ_EFeature f)
	{
		if (s_bServer && s_aOn && f < s_aOn.Count())
			return s_aOn[f];

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm)
		{
			string off = gm.GetFeaturesOff();
			int fi = f;
			if (!off.IsEmpty())
				return !off.Contains(";" + fi.ToString() + ";");
		}

		if (s_aOn && f < s_aOn.Count())
			return s_aOn[f];

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Can anyone offer a job at all? Contracts, a leader's own job, or a questline's or story arc's
	//! step. With none of them on (stage 0), the talk screen shows no job rows and nobody makes one.
	static bool AnyJobs()
	{
		return On(RZ_EFeature.CONTRACTS) || On(RZ_EFeature.LEADERS) || On(RZ_EFeature.QUESTLINES) || On(RZ_EFeature.STORY_ARCS);
	}

	//------------------------------------------------------------------------------------------------
	//! ";3;7;": the switches that are off, for the clients.
	static string OffList()
	{
		string s = ";";
		if (!s_aOn)
			return s;

		for (int i = 0; i < s_aOn.Count(); i++)
		{
			if (!s_aOn[i])
				s = s + i.ToString() + ";";
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! A switch's name, as the profile file and the log write it.
	static string Name(int f)
	{
		switch (f)
		{
			case RZ_EFeature.CAMP_SITES: return "CampSites";
			case RZ_EFeature.CAMP_LIFE: return "CampLife";
			case RZ_EFeature.SAFEHOUSES: return "Safehouses";
			case RZ_EFeature.LEADERS: return "Leaders";
			case RZ_EFeature.SHOPS: return "Shops";
			case RZ_EFeature.ERRANDS: return "Errands";
			case RZ_EFeature.WORK_SITES: return "WorkSites";
			case RZ_EFeature.WANDERERS: return "Wanderers";
			case RZ_EFeature.FACTION_LIFE: return "FactionLife";
			case RZ_EFeature.CHECKPOINTS: return "Checkpoints";
			case RZ_EFeature.GUIDANCE: return "Guidance";
			case RZ_EFeature.OPENING: return "Opening";
			case RZ_EFeature.HABITS: return "Habits";
			case RZ_EFeature.PERSONAL_GOALS: return "PersonalGoals";
			case RZ_EFeature.MAP_FOG: return "MapFog";
			case RZ_EFeature.ROAD_PATROLS: return "RoadPatrols";
			case RZ_EFeature.SUPPLY_TRUCKS: return "SupplyTrucks";
			case RZ_EFeature.TONIGHT: return "Tonight";
			case RZ_EFeature.CONTRACTS: return "Contracts";
			case RZ_EFeature.WORLD_EVENTS: return "WorldEvents";
			case RZ_EFeature.PUZZLES: return "Puzzles";
			case RZ_EFeature.LOCKS: return "Locks";
			case RZ_EFeature.CARS: return "Cars";
			case RZ_EFeature.CRAFTING: return "Crafting";
			case RZ_EFeature.RADIOS: return "Radios";
			case RZ_EFeature.HORDES: return "Hordes";
			case RZ_EFeature.CRAWLERS: return "Crawlers";
			case RZ_EFeature.TRIBES: return "Tribes";
			case RZ_EFeature.BEHAVIOUR: return "Behaviour";
			case RZ_EFeature.STEALTH: return "Stealth";
			case RZ_EFeature.HUNTED: return "Hunted";
			case RZ_EFeature.LONG_TABLE: return "LongTable";
			case RZ_EFeature.WEATHER: return "Weather";
			case RZ_EFeature.PERIMETER: return "Perimeter";
			case RZ_EFeature.STORY_ARCS: return "StoryArcs";
			case RZ_EFeature.QUESTLINES: return "Questlines";
			case RZ_EFeature.MAJORS: return "Majors";
			case RZ_EFeature.MEMORY: return "Memory";
			case RZ_EFeature.POWER: return "Power";
			case RZ_EFeature.RADIO_STATION: return "RadioStation";
			case RZ_EFeature.TRACES: return "Traces";
			case RZ_EFeature.TOWN_MEMORY: return "TownMemory";
			case RZ_EFeature.WILDLIFE: return "Wildlife";
			case RZ_EFeature.SEASON_RULES: return "SeasonRules";
			case RZ_EFeature.RESEARCH: return "Research";
			case RZ_EFeature.FINALE: return "Finale";
			case RZ_EFeature.EPILOGUE: return "Epilogue";
		}

		return "?";
	}

	//------------------------------------------------------------------------------------------------
	//! The stage a switch comes back in (1 to 8).
	static int Stage(int f)
	{
		if (f <= RZ_EFeature.CHECKPOINTS)
			return 1;

		if (f <= RZ_EFeature.PERSONAL_GOALS)
			return 2;

		if (f <= RZ_EFeature.TONIGHT)
			return 3;

		if (f <= RZ_EFeature.RADIOS)
			return 4;

		if (f <= RZ_EFeature.PERIMETER)
			return 5;

		if (f <= RZ_EFeature.MEMORY)
			return 6;

		if (f <= RZ_EFeature.SEASON_RULES)
			return 7;

		return 8;
	}

	//------------------------------------------------------------------------------------------------
	//! Read the switches (the prefab's, then the profile file's), settle the needs. Server or client.
	static void Read(RZ_FeatureSettings s, bool server)
	{
		s_bServer = server;
		s_aOn = {};
		s_aOn.Resize(RZ_EFeature.COUNT);
		// No Features object on the game mode: every switch at its default, on
		if (!s)
		{
			for (int d = 0; d < s_aOn.Count(); d++)
			{
				s_aOn[d] = true;
			}

			Settle(server);
			return;
		}

		s_aOn[RZ_EFeature.CAMP_SITES] = s.m_bCampSites;
		s_aOn[RZ_EFeature.CAMP_LIFE] = s.m_bCampLife;
		s_aOn[RZ_EFeature.SAFEHOUSES] = s.m_bSafehouses;
		s_aOn[RZ_EFeature.LEADERS] = s.m_bLeaders;
		s_aOn[RZ_EFeature.SHOPS] = s.m_bShops;
		s_aOn[RZ_EFeature.ERRANDS] = s.m_bErrands;
		s_aOn[RZ_EFeature.WORK_SITES] = s.m_bWorkSites;
		s_aOn[RZ_EFeature.WANDERERS] = s.m_bWanderers;
		s_aOn[RZ_EFeature.FACTION_LIFE] = s.m_bFactionLife;
		s_aOn[RZ_EFeature.CHECKPOINTS] = s.m_bCheckpoints;
		s_aOn[RZ_EFeature.GUIDANCE] = s.m_bGuidance;
		s_aOn[RZ_EFeature.OPENING] = s.m_bOpening;
		s_aOn[RZ_EFeature.HABITS] = s.m_bHabits;
		s_aOn[RZ_EFeature.PERSONAL_GOALS] = s.m_bPersonalGoals;
		s_aOn[RZ_EFeature.MAP_FOG] = s.m_bMapFog;
		s_aOn[RZ_EFeature.ROAD_PATROLS] = s.m_bRoadPatrols;
		s_aOn[RZ_EFeature.SUPPLY_TRUCKS] = s.m_bSupplyTrucks;
		s_aOn[RZ_EFeature.TONIGHT] = s.m_bTonight;
		s_aOn[RZ_EFeature.CONTRACTS] = s.m_bContracts;
		s_aOn[RZ_EFeature.WORLD_EVENTS] = s.m_bWorldEvents;
		s_aOn[RZ_EFeature.PUZZLES] = s.m_bPuzzles;
		s_aOn[RZ_EFeature.LOCKS] = s.m_bLocks;
		s_aOn[RZ_EFeature.CARS] = s.m_bCars;
		s_aOn[RZ_EFeature.CRAFTING] = s.m_bCrafting;
		s_aOn[RZ_EFeature.RADIOS] = s.m_bRadios;
		s_aOn[RZ_EFeature.HORDES] = s.m_bHordes;
		s_aOn[RZ_EFeature.CRAWLERS] = s.m_bCrawlers;
		s_aOn[RZ_EFeature.TRIBES] = s.m_bTribes;
		s_aOn[RZ_EFeature.BEHAVIOUR] = s.m_bBehaviour;
		s_aOn[RZ_EFeature.STEALTH] = s.m_bStealth;
		s_aOn[RZ_EFeature.HUNTED] = s.m_bHunted;
		s_aOn[RZ_EFeature.LONG_TABLE] = s.m_bLongTable;
		s_aOn[RZ_EFeature.WEATHER] = s.m_bWeather;
		s_aOn[RZ_EFeature.PERIMETER] = s.m_bPerimeter;
		s_aOn[RZ_EFeature.STORY_ARCS] = s.m_bStoryArcs;
		s_aOn[RZ_EFeature.QUESTLINES] = s.m_bQuestlines;
		s_aOn[RZ_EFeature.MAJORS] = s.m_bMajors;
		s_aOn[RZ_EFeature.MEMORY] = s.m_bMemory;
		s_aOn[RZ_EFeature.POWER] = s.m_bPower;
		s_aOn[RZ_EFeature.RADIO_STATION] = s.m_bRadioStation;
		s_aOn[RZ_EFeature.TRACES] = s.m_bTraces;
		s_aOn[RZ_EFeature.TOWN_MEMORY] = s.m_bTownMemory;
		s_aOn[RZ_EFeature.WILDLIFE] = s.m_bWildlife;
		s_aOn[RZ_EFeature.SEASON_RULES] = s.m_bSeasonRules;
		s_aOn[RZ_EFeature.RESEARCH] = s.m_bResearch;
		s_aOn[RZ_EFeature.FINALE] = s.m_bFinale;
		s_aOn[RZ_EFeature.EPILOGUE] = s.m_bEpilogue;
		Settle(server);
	}

	//------------------------------------------------------------------------------------------------
	//! The profile file on the server, then the needs.
	protected static void Settle(bool server)
	{
		if (server)
			ReadFile();

		// Needs: a switch whose needs are off is off too
		Need(RZ_EFeature.CAMP_LIFE, RZ_EFeature.CAMP_SITES);
		Need(RZ_EFeature.OPENING, RZ_EFeature.GUIDANCE);
		Need(RZ_EFeature.OPENING, RZ_EFeature.ERRANDS);
		Need(RZ_EFeature.OPENING, RZ_EFeature.WORK_SITES);
		Need(RZ_EFeature.PUZZLES, RZ_EFeature.CONTRACTS);
		Need(RZ_EFeature.BEHAVIOUR, RZ_EFeature.TRIBES);
		Need(RZ_EFeature.MAJORS, RZ_EFeature.QUESTLINES);
		Need(RZ_EFeature.FINALE, RZ_EFeature.RESEARCH);
		Need(RZ_EFeature.FINALE, RZ_EFeature.MAJORS);
		Need(RZ_EFeature.EPILOGUE, RZ_EFeature.FINALE);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Need(int f, int need)
	{
		if (!s_aOn[need])
			s_aOn[f] = false;
	}

	//------------------------------------------------------------------------------------------------
	//! $profile:RZ_Features.txt, when it's there (the header says what it holds).
	protected static void ReadFile()
	{
		if (!FileIO.FileExists(FILE))
			return;

		FileHandle file = FileIO.OpenFile(FILE, FileMode.READ);
		if (!file)
			return;

		string line;
		while (file.ReadLine(line) >= 0)
		{
			string word = line.Trim();
			int hash = word.IndexOf("#");
			if (hash >= 0)
				word = word.Substring(0, hash).Trim();

			if (word.IsEmpty())
				continue;

			Word(word);
		}

		file.Close();
		RZ_Log.Info("Features: " + FILE + " read");
	}

	//------------------------------------------------------------------------------------------------
	protected static void Word(string word)
	{
		string low = word;
		low.ToLower();
		if (low == "all")
		{
			for (int a = 0; a < s_aOn.Count(); a++)
			{
				s_aOn[a] = true;
			}

			return;
		}

		if (low == "core")
		{
			for (int c = 0; c < s_aOn.Count(); c++)
			{
				s_aOn[c] = false;
			}

			return;
		}

		if (low.IndexOf("stage") == 0)
		{
			int upTo = low.Substring(5, low.Length() - 5).Trim().ToInt();
			for (int st = 0; st < s_aOn.Count(); st++)
			{
				s_aOn[st] = Stage(st) <= upTo;
			}

			return;
		}

		bool want = true;
		string name = low;
		if (name.IndexOf("-") == 0)
		{
			want = false;
			name = name.Substring(1, name.Length() - 1);
		}

		for (int n = 0; n < s_aOn.Count(); n++)
		{
			string known = Name(n);
			known.ToLower();
			if (known == name)
			{
				s_aOn[n] = want;
				return;
			}
		}

		RZ_Log.Warn("Features: " + FILE + " names no switch \"" + word + "\"");
	}

	//------------------------------------------------------------------------------------------------
	//! The log's line: which stages are whole, and every switch that's on outside them.
	static string Describe()
	{
		string on = "";
		string off = "";
		int count = 0;
		for (int i = 0; i < s_aOn.Count(); i++)
		{
			if (s_aOn[i])
			{
				count++;
				on = on + " " + Name(i);
			}
			else
			{
				off = off + " " + Name(i);
			}
		}

		if (on.IsEmpty())
			on = " none (only the core)";

		if (off.IsEmpty())
			off = " none";

		return string.Format("%1 of %2 on:%3. Off:%4", count, s_aOn.Count(), on, off);
	}
}
