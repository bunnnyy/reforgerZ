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

	bool hasLab;
	float labX;
	float labZ;

	// Quartermaster positions per faction (x/z, set = 1 if valid)
	ref array<float> qmX = {};
	ref array<float> qmZ = {};
	ref array<int> qmSet = {};

	ref array<ref RZ_SettlementSave> settlements = {};
	ref array<ref RZ_PlayerProfile> profiles = {};
}

class RZ_Persistence
{
	static const string DIR = "$profile:ReforgerZ";
	static const string PATH = "$profile:ReforgerZ/state.json";

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
