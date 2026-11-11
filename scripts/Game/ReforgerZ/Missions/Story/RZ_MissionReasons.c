//! Which contracts answer which reasons (docs/DESIGN.md, "Story blocks"). The reason lane
//! (RZ_MissionManager.ReasonLane) starts one that answers a posted reason, near its subject. One row
//! per type; types not listed answer none and stay random picks only. Also the fallout lines: what
//! the world says when a job with no fallout errand fails (the Tier 0 audit). Server only.
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
		if (t == RZ_Mission_ClearSettlement)
			result.Insert("town.infested");

		if (t == RZ_Mission_BurnTheBodies)
			result.Insert("town.shelled");

		// A leader dead: the new commander wants the numbers right
		if (t == RZ_Mission_StruckOff)
			result.Insert("leader.died");

		// A wanderer bitten on the road
		if (t == RZ_Mission_TrailOfClues)
			result.Insert("wanderer.bitten");

		// A hungry house: someone counts its stores
		if (t == RZ_Mission_TheLedger)
			result.Insert("house.hungry");

		// The lab's milestones
		if (t == RZ_Mission_ColdChain || t == RZ_Mission_PowerTheLab)
			result.Insert("cure.*");
	}

	//------------------------------------------------------------------------------------------------
	//! What the world says when a job of type `t` fails, told to those it was offered to after the fail
	//! text; "" none. Story jobs with a fallout errand say theirs through the errand instead.
	static string FalloutLine(typename t)
	{
		if (t == RZ_Mission_WalkHimOut)
			return "Nobody came. He turned in the end, and the house says his name at supper.";

		if (t == RZ_Mission_BittenFriend)
			return "The house found out the way houses do. They're quieter at the door now.";

		if (t == RZ_Mission_TheList)
			return "The list goes round without you. Somebody else decides whose names are on it.";

		if (t == RZ_Mission_TheLedger)
			return "The stores stay a mystery. The council counts the tins twice a day now.";

		if (t == RZ_Mission_QuietArrangement)
			return "The arrangement stays as it was, and so does the price.";

		if (t == RZ_Mission_Requisition)
			return "The post goes short this week. The sergeant says so on the call, by name of the house.";

		if (t == RZ_Mission_SecureTheShipment)
			return "The case went where it was going, without you.";

		if (t == RZ_Mission_NoReply)
			return "The log says what it always says.";

		if (t == RZ_Mission_LearningTheDrones)
			return "The drones fly the same lines. Nobody wrote them down.";

		if (t == RZ_Mission_PitsDebtor)
			return "The debt stays on the book. The pit doesn't forget a name.";

		if (t == RZ_Mission_TiedDown)
			return "The truck went on to wherever the cargo goes.";

		if (t == RZ_Mission_SerumTrial)
			return "The trial goes on with somebody else's arm.";

		if (t == RZ_Event_RivalSurvivors)
			return "The two crews settled it between themselves. One of them is smaller now.";

		if (t == RZ_Mission_TheRat)
			return "The rat is still in the walls, and still talking.";

		if (t == RZ_Mission_SilentVillage)
			return "The village stays silent. Nobody goes there twice.";

		if (t == RZ_Mission_TheWitness)
			return "What she saw goes with her.";

		return "";
	}
}
