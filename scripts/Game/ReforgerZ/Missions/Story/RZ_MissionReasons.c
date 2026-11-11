//! Which contracts answer which reasons (designs/MISSION_STYLE.md, "Reasons": the start of the
//! table). The reason lane (RZ_MissionManager.ReasonLane) starts one that answers a posted reason,
//! near its subject. One row per type; types not listed answer none and stay random picks only.
//! Server only.
class RZ_MissionReasons
{
	//------------------------------------------------------------------------------------------------
	//! True if contract type `t` answers reason `key`.
	static bool Answers(typename t, string key)
	{
		array<string> keys = {};
		Reasons(t, keys);
		foreach (string k : keys)
		{
			if (k == key || (k.EndsWith("*") && key.StartsWith(k.Substring(0, k.Length() - 1))))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The reasons a type answers ("cure.*" answers every cure milestone).
	static void Reasons(typename t, notnull array<string> result)
	{
		// A hungry house: food work for it, and the crew's collection
		if (t == RZ_Mission_HarvestGuard)
		{
			result.Insert("house.hungry");
			result.Insert("house.truck_lost");
		}

		if (t == RZ_Mission_ProtectionRacket)
			result.Insert("house.hungry");

		if (t == RZ_Mission_MedicineRun)
			result.Insert("wanderer.bitten");

		// A post under pressure: its lost truck
		if (t == RZ_Mission_CheckpointDuty || t == RZ_Mission_ConvoyEscort)
			result.Insert("post.truck_lost");

		// A town gone bad, or shelled
		if (t == RZ_Mission_ClearSettlement || t == RZ_Mission_TakeBackTown)
			result.Insert("town.infested");

		if (t == RZ_Mission_BurnTheBodies)
			result.Insert("town.shelled");

		// A leader dead: the new commander wants the numbers right
		if (t == RZ_Mission_StruckOff)
			result.Insert("leader.died");

		// A wanderer bitten on the road
		if (t == RZ_Mission_TrailOfClues)
			result.Insert("wanderer.bitten");

		// The lab's milestones
		if (t == RZ_Mission_ColdChain || t == RZ_Mission_PowerTheLab)
			result.Insert("cure.*");
	}
}
