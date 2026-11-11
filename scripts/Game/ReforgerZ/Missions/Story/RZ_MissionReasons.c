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

		// The one-choice jobs (each also has its one choice: Missions/Blocks/RZ_OneChoice.c)
		if (t == RZ_Mission_SurvivorHoldout || t == RZ_Mission_RescueTheKids || t == RZ_Mission_LeadThemAway)
			result.Insert("horde.passed");

		if (t == RZ_Mission_RadioRelay || t == RZ_Mission_BurnTheNest || t == RZ_Mission_RescueTheKids || t == RZ_Mission_CheckpointDuty || t == RZ_Mission_WaterSurvey)
			result.Insert("town.infested");

		if (t == RZ_Mission_ZombieBait || t == RZ_Mission_KillTheOfficer)
			result.Insert("town.shelled");

		if (t == RZ_Mission_StealAntidotes)
			result.Insert("house.bitten");

		if (t == RZ_Mission_LabDefence)
			result.Insert("cure.*");

		// The reasons read from other systems' facts (RZ_ReasonPosts)
		if (t == RZ_Mission_CheckpointDuty || t == RZ_Mission_ConvoyEscort)
			result.Insert("post.pressure");

		if (t == RZ_Mission_WaterSurvey)
			result.Insert("house.water");

		if (t == RZ_Mission_ReinIn)
		{
			result.Insert("leader.rogue");
			result.Insert("crew.skimmed");
		}

		if (t == RZ_Mission_LostColleague || t == RZ_Mission_ColdChain)
			result.Insert("lab.short");

		if (t == RZ_Mission_PowerTheLab || t == RZ_Mission_ColdChain)
			result.Insert("lab.power");

		if (t == RZ_Mission_NotTonight || t == RZ_Mission_LeadThemAway)
			result.Insert("tribe.hungry");

		if (t == RZ_Mission_TheLedger)
		{
			result.Insert("house.feud");
			result.Insert("racket.ran");
		}

		// The work rows (RZ_WorkRows.c)
		RZ_WorkRows.Reasons(t, result);
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

		// The leader jobs and Rein In (the rest of the Tier 0 audit)
		if (t == RZ_Job_ShutTheDoor)
			return "The door stays open a while longer. The council will ask someone else, and sooner.";

		if (t == RZ_Job_RealOrders)
			return "The orders stay in the safe. The post goes on reading the ones it was given.";

		if (t == RZ_Job_LocalLaw)
			return "The town settles it the old way. Nobody writes down how.";

		if (t == RZ_Job_OffBookRequisition)
			return "The stores stay on the book, and the post stays short. Someone signs for it anyway.";

		if (t == RZ_Job_DesertionRoute)
			return "The men stay where they were put. One of them stops talking to anyone.";

		if (t == RZ_Job_HonourAmongThieves)
			return "The crews settle it with each other. The book has one name fewer in it.";

		if (t == RZ_Job_DebtCollection)
			return "The debt stays where it was, and grows. The book doesn't forget a name.";

		if (t == RZ_Job_PatientsNotSamples)
			return "The trial goes on as written. The patients are numbers again by morning.";

		if (t == RZ_Job_TheNotebook)
			return "The notebook stays where it fell. Somebody else's handwriting goes into the record instead.";

		if (t == RZ_Job_PrivateCollection)
			return "The collection stays private. The director never asks about it, which is an answer.";

		if (t == RZ_Mission_ReinIn)
			return "The outpost carries on as it likes. The stronghold notes it, and waits.";

		// The one-choice jobs
		if (t == RZ_Mission_RadioRelay)
			return "The relay stays dark. The house listens to static at night and calls it quiet.";

		if (t == RZ_Mission_HarvestGuard)
			return "The field went to the infected. The farmer counts what they left, which is nothing.";

		if (t == RZ_Mission_BurnTheNest)
			return "The nest is still there, and fuller. The town breathes worse for it.";

		if (t == RZ_Mission_RescueTheKids)
			return "The school's quiet now. Nobody says why out loud.";

		if (t == RZ_Mission_CheckpointDuty)
			return "The barrier never went up. The town spreads as it likes.";

		if (t == RZ_Mission_ConvoyEscort)
			return "The post goes short this week. The truck's on the call, by number.";

		if (t == RZ_Mission_BlowTheBridge || t == RZ_Mission_LeadThemAway)
			return "They got where they were going. The house there will count heads in the morning.";

		if (t == RZ_Mission_ZombieBait)
			return "Bastion's post sleeps well tonight. The crews don't.";

		if (t == RZ_Mission_FightPit)
			return "The pit had a short night. The book didn't.";

		if (t == RZ_Mission_HijackTheConvoy)
			return "The truck got through. Bastion's call reads the delivery out loud, slowly.";

		if (t == RZ_Mission_KillTheOfficer)
			return "He's still running the post, and running it well.";

		if (t == RZ_Mission_StealAntidotes)
			return "The store room stays locked. The crews sell something else.";

		if (t == RZ_Mission_LabDefence)
			return "The lab lost a room tonight. The night shift sleeps in the corridor.";

		if (t == RZ_Mission_WaterSurvey)
			return "Nobody knows which town is next. Somebody finds out.";

		// The work rows (RZ_WorkRows.c)
		return RZ_WorkRows.Fallout(t);
	}

	//------------------------------------------------------------------------------------------------
	//! The Tier 0 audit's record key for a job that was already there (docs/DESIGN.md, "Story
	//! blocks"): each writes `t0.<key>` at its place when it ends (done or failed), and the next one
	//! offered there reads it in its brief. "" if `t` isn't one of the 27.
	static string Tier0Key(typename t)
	{
		if (t == RZ_Mission_WalkHimOut) return "walk_him_out";
		if (t == RZ_Mission_BittenFriend) return "bitten_friend";
		if (t == RZ_Mission_TheList) return "the_list";
		if (t == RZ_Mission_TheLedger) return "the_ledger";
		if (t == RZ_Mission_QuietArrangement) return "quiet_arrangement";
		if (t == RZ_Mission_Requisition) return "requisition";
		if (t == RZ_Mission_SecureTheShipment) return "secure_shipment";
		if (t == RZ_Mission_NoReply) return "no_reply";
		if (t == RZ_Mission_LearningTheDrones) return "learning_drones";
		if (t == RZ_Mission_PitsDebtor) return "pits_debtor";
		if (t == RZ_Mission_TiedDown) return "tied_down";
		if (t == RZ_Mission_SerumTrial) return "serum_trial";
		if (t == RZ_Event_RivalSurvivors) return "rival_survivors";
		if (t == RZ_Mission_TheRat) return "the_rat";
		if (t == RZ_Mission_SilentVillage) return "silent_village";
		if (t == RZ_Mission_TheWitness) return "the_witness";
		if (t == RZ_Job_ShutTheDoor) return "shut_the_door";
		if (t == RZ_Job_RealOrders) return "real_orders";
		if (t == RZ_Job_LocalLaw) return "local_law";
		if (t == RZ_Job_OffBookRequisition) return "off_book";
		if (t == RZ_Job_DesertionRoute) return "desertion_route";
		if (t == RZ_Job_HonourAmongThieves) return "honour_thieves";
		if (t == RZ_Job_DebtCollection) return "debt_collection";
		if (t == RZ_Job_PatientsNotSamples) return "patients_samples";
		if (t == RZ_Job_TheNotebook) return "the_notebook";
		if (t == RZ_Job_PrivateCollection) return "private_collection";
		if (t == RZ_Mission_ReinIn) return "rein_in";
		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Where a Tier 0 job's record is kept: its town, else its grid square.
	static string Tier0Subject(RZ_Mission m)
	{
		if (m.m_iSettlement >= 0)
			return RZ_WorldFacts.Town(m.m_iSettlement);

		return "grid:" + RZ_SettlementScanner.GridReference(m.m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	//! A Tier 0 job ended: `t0.<key>` at its place, done or failed, with who was there.
	static void Tier0Ended(RZ_Mission m, bool success, array<int> players)
	{
		string key = Tier0Key(m.Type());
		if (key.IsEmpty() || m.m_bWithdrawn)
			return;

		string value = "failed";
		if (success)
			value = "done";

		RZ_WorldFacts.Get().Memory().Record("t0." + key, Tier0Subject(m), value, RZ_WorldMemory.SOURCE_STORY, false, players);
	}

	//------------------------------------------------------------------------------------------------
	//! A Tier 0 job offered where the last one ended: its brief says how that went.
	static void Tier0Read(RZ_Mission m)
	{
		string key = Tier0Key(m.Type());
		if (key.IsEmpty())
			return;

		RZ_WorldRecord r = RZ_WorldFacts.Get().Memory().FindRecord("t0." + key, Tier0Subject(m));
		if (!r)
			return;

		if (r.value == "done")
			m.m_sBrief += " The last time this came up here, someone saw it through. People remember that.";
		else
			m.m_sBrief += " The last time this came up here, nobody did. People remember that too.";
	}
}
