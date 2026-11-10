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
}

class RZ_SaveData
{
	int version = 1;
	string world;

	int stage;
	float stageMinutes;

	float cureProgress;
	int cureMilestone;
	bool cured;

	// Season: Fame given to each faction by all players, and who got the cure (-1 = not yet)
	ref array<int> seasonFame = {};
	int seasonWinner = -1;

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

	ref array<ref RZ_SettlementSave> settlements = {};
	ref array<ref RZ_PlayerProfile> profiles = {};
}

//! The result of the last finished season. Kept in its own file so a season
//! wipe (m_bWipeOnStart, or deleting state.json) doesn't lose it.
class RZ_SeasonRecord
{
	string world;
	int winner = -1;
	string winnerName;
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

		SCR_JsonLoadContext ctx = new SCR_JsonLoadContext();
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
		SCR_JsonSaveContext ctx = new SCR_JsonSaveContext();
		ctx.WriteValue("season", record);
		if (!ctx.SaveToFile(SEASON_PATH))
			RZ_Log.Warn("Could not write " + SEASON_PATH);
	}

	//------------------------------------------------------------------------------------------------
	// VERIFY: SCR_JsonLoadContext/SCR_JsonSaveContext automatically serialize plain class members
	static RZ_SaveData Load()
	{
		if (!FileIO.FileExists(PATH))
			return null;

		SCR_JsonLoadContext ctx = new SCR_JsonLoadContext();
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
		FileIO.MakeDirectory(DIR);
		SCR_JsonSaveContext ctx = new SCR_JsonSaveContext();
		ctx.WriteValue("state", data);
		if (!ctx.SaveToFile(PATH))
			RZ_Log.Warn("Could not write " + PATH);
	}
}
