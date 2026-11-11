//! Plain data saved to $profile:ReforgerZ/state.json on the server.
//! Covers the world state that belongs to this mod (director, settlements,
//! cure, player profiles). Player bodies, inventories, vehicles and bases are
//! left to the vanilla persistence system and the base-building mod.
class RZ_SettlementSave
{
	string name;
	float x;
	float y;
	float z;
	float radius;
	int size;
	float infection;
	bool lab;
	// The crawlers' kill ledger and stir (Zombies/RZ_Crawlers.c). Older saves have 0: none
	float ledger;
	float stir;
}

class RZ_SaveData
{
	int version = 1;
	string world;

	int stage;
	float stageMinutes;
	// Minutes played since the outbreak began (the season day, Core/RZ_Clocks.c). Older saves have 0
	// here: it's then counted from the stages' set lengths
	float playedMinutes;

	float cureProgress;
	int cureMilestone;
	bool cured;
	int labAntidotes;		// doses in the lab's stock
	int stolenAntidotes;	// doses on the Red Hand's black market
	int labStabilisers;		// stabilisers in Halcyon's stock
	// The lab's backlog: research delivered and not yet worked through, the real time the lab last
	// worked, whether a finale has opened the 90 percent gate, the real seconds spent at it, finale
	// research by faction, breakthrough news not heard yet, and the pace by play (player-hours on
	// the last real days, the real day of the first slot, the first whole day counted)
	// (Cure/RZ_CureResearch.c). Older saves have 0 here: an empty backlog, the clock from now.
	float cureBacklog;
	int labUnix;
	bool finaleGateOpen;
	float gateSeconds;
	ref array<float> finaleResearch = {};
	int cureNews;
	ref array<float> playHours = {};
	int playDay;
	int playFrom;

	// Season: Fame given to each faction by all players, and who got the cure (-1 = not yet)
	ref array<int> seasonFame = {};
	int seasonWinner = -1;

	// The season's number and clock, whose rules it started with, and a wipe that came due
	// (Core/RZ_Seasons.c). Older saves have 0 here: the season is then counted from now.
	int seasonNumber;
	int seasonStartUnix;
	int cureUnix;
	int rulesWinner = -1;
	bool wipeDue;

	bool hasLab;
	float labX;
	float labZ;

	// Quartermaster positions per faction (x/z, set = 1 if valid)
	ref array<float> qmX = {};
	ref array<float> qmZ = {};
	ref array<int> qmSet = {};

	// Haven safehouses (population, food, upkeep) and the wanderer roster (positions, memories)
	ref array<ref RZ_SafehouseSave> safehouses = {};
	ref RZ_WandererRosterSave wanderers;

	// Faction leaders (names, traits, loyalty, memories) and their recent headlines
	ref RZ_LeaderRosterSave leaders;

	// Territory (holdings, strength, pressure), supply (stock, shortages, departures) and
	// faction life (morale, truces and feuds, remembered moments)
	ref RZ_TerritorySave territory;
	ref RZ_SupplySave supply;
	ref RZ_FactionLifeSave factionLife;

	// Town memory (loot levels, flags) and the journals of the dead
	ref RZ_TownMemorySave towns;

	// The power station (where, how far restored, its condition) and the radio station
	ref RZ_PowerSave power;
	ref RZ_RadioStationSave radio;

	// Story arcs (the running one, when each last ran) and the session headlines' own facts
	ref RZ_StorySave story;
	ref RZ_HeadlineSave news;

	// Questlines (Director/RZ_Questlines.c): the running ones (stage, flags, notes, timers, who
	// did what) and the ones that ended this season
	ref RZ_QuestlinesSave questlines;

	// World facts (World/RZ_WorldFacts.c): the season's seed, flags, canon and gossip
	ref RZ_WorldFactsSave facts;

	// The shared engine (Core/RZ_Shared.c): waiting callbacks, the season budget and its lanes, the
	// set-piece calendar, the season's deeds by identity. Older saves have none: they start empty
	ref RZ_SharedSave shared;

	// World interactions: searched car boots, loose loot times, opened locks, burned buildings, stripped wrecks, fixable cars
	ref RZ_InteractionSave interactions;

	// Errands (Missions/Errands): each player's errand ledger for the season, the safehouses' named
	// people, the houses' turns done. Older saves have none: everything starts empty
	ref RZ_ErrandsSave errands;

	// The Long Table (World/RZ_LongTable.c): this season's roll, the farm, its people, hunger,
	// suspicion, what it took, its tells on the road and what it remembers of each character
	ref RZ_LongTableSave longTable;

	// The crawlers' kill sites (Zombies/RZ_Crawlers.c), and the time of day so a restart carries on
	// where the day stopped (Core/RZ_GameModeComponent.c, Keep time of day). Older saves have none
	ref RZ_CrawlerSave crawlers;
	// The tribes (Zombies/RZ_Tribes.c). Older saves have none: the season's seeds are made on load
	ref RZ_TribesSave tribes;
	bool hasTimeOfDay;
	float timeOfDay;

	ref array<ref RZ_SettlementSave> settlements = {};
	ref array<ref RZ_PlayerProfile> profiles = {};
}

//! The result of the last finished season. Kept in its own file so a season
//! wipe (the wipe cycle, Wipe on start, or deleting state.json) doesn't lose it. The next fresh
//! season reads it for its starting rules (Core/RZ_Seasons.c).
class RZ_SeasonRecord
{
	string world;
	int season;
	int winner = -1;		// -1: the season ran out of time without a cure
	string winnerName;
	string endedBy = "cure";	// "cure" or "time"
	ref array<int> fame = {};
}

class RZ_Persistence
{
	static const string DIR = "$profile:ReforgerZ";
	static const string PATH = "$profile:ReforgerZ/state.json";
	static const string SEASON_PATH = "$profile:ReforgerZ/last_season.json";

	//------------------------------------------------------------------------------------------------
	static RZ_SeasonRecord LoadSeason()
	{
		if (!FileIO.FileExists(SEASON_PATH))
			return null;

		JsonLoadContext ctx = new JsonLoadContext();
		if (!ctx.LoadFromFile(SEASON_PATH))
			return null;

		RZ_SeasonRecord record = new RZ_SeasonRecord();
		if (!ctx.ReadValue("season", record))
			return null;

		return record;
	}

	//------------------------------------------------------------------------------------------------
	static void SaveSeason(RZ_SeasonRecord record)
	{
		FileIO.MakeDirectory(DIR);
		JsonSaveContext ctx = new JsonSaveContext();
		ctx.WriteValue("season", record);
		if (!ctx.SaveToFile(SEASON_PATH))
			RZ_Log.Warn("Could not write " + SEASON_PATH);
	}

	//------------------------------------------------------------------------------------------------
	// VERIFY: JsonSaveContext and JsonLoadContext write and read plain class members and arrays of them (check a save file after a restart)
	static RZ_SaveData Load()
	{
		if (!FileIO.FileExists(PATH))
			return null;

		JsonLoadContext ctx = new JsonLoadContext();
		if (!ctx.LoadFromFile(PATH))
		{
			RZ_Log.Warn("Could not read " + PATH);
			return null;
		}

		RZ_SaveData data = new RZ_SaveData();
		if (!ctx.ReadValue("state", data))
		{
			RZ_Log.Warn("Save file is corrupt or from an incompatible version, ignoring.");
			return null;
		}

		return data;
	}

	//------------------------------------------------------------------------------------------------
	static void Save(RZ_SaveData data)
	{
		SaveTo(data, PATH);
	}

	//------------------------------------------------------------------------------------------------
	//! The state written somewhere else (a season wipe's backup).
	static void SaveTo(RZ_SaveData data, string path)
	{
		FileIO.MakeDirectory(DIR);
		JsonSaveContext ctx = new JsonSaveContext();
		ctx.WriteValue("state", data);
		if (!ctx.SaveToFile(path))
			RZ_Log.Warn("Could not write " + path);
	}

	//------------------------------------------------------------------------------------------------
	//! Where a wiped season's state is kept: state_season3.json.
	static string BackupPath(int season)
	{
		return string.Format("%1/state_season%2.json", DIR, Math.Max(0, season));
	}
}
