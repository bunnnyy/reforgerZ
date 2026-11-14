//! What duties find (docs/ERRANDS.md, "Things that happen"): the beats a patrol meets at a point, a
//! hunter at the snares, and the small turns a chore takes. Each is drawn from the world as it is
//! at that spot (Traces' door marks, the rumours going round, how sick the town is), most of them not
//! a fight, with a choice where one fits. A patrol's find always has its thing really there (a real
//! door, a grave, a radio, remains, a box under a step) with its one prompt on it; a find whose
//! thing can't stand near the stop isn't used. Data for RZ_DutyScenes. Server only.
class RZ_DutyFinds
{
	protected static const float FIND_TIMEOUT = 150;	// a patrol waits this long at a find
	static const float FIND_REACH = 120;				// a find stands this far from the patrol's stop at most
	protected static const float DOORSTEP = 0.8;		// a door find's prompt stands this far out of the doorway

	// What stands at a find (tools/camp_kit/props.txt: base-game props, measured in footprints.json)
	protected static const ResourceName GRAVE = "{EAD2277399B3C658}Prefabs/Props/Military/Graves/Grave_USSR_02.et";
	protected static const ResourceName RADIO = "{2F2BBFDA3A2C73D4}Prefabs/Props/Civilian/RadioSmall_01_black.et";
	protected static const ResourceName BOX = "{2B26287DC5B7F9D0}Prefabs/Props/Industrial/ToolBox_01/ToolBox_01_red.et";
	protected static const ResourceName REMAINS = "{85EFAE5E2B52E5A1}Prefabs/Props/Remains/SkeletonRemains/SkeletonRemains_05_clothes.et";

	//------------------------------------------------------------------------------------------------
	//! A find at a real spot: its thing stands there and its one prompt is on it.
	protected static RZ_DutyBeat Find(string key, string line, vector at, ResourceName thing, string goal)
	{
		RZ_DutyBeat b = new RZ_DutyBeat(key, line, RZ_DutyBeat.AT_SET);
		b.m_vAt = at;
		b.m_sProp = thing;
		b.m_sGoal = goal;
		b.m_fTimeout = FIND_TIMEOUT;
		b.m_bFind = true;
		return b;
	}

	//------------------------------------------------------------------------------------------------
	//! A doorstep in front of a real outside door within FIND_REACH of `at` (`houses`: a house's).
	//! False when there's none: no door find there.
	protected static bool DoorStep(vector at, bool houses, out vector step)
	{
		IEntity door;
		vector centre;
		vector normal;
		step = vector.Zero;
		if (!RZ_DoorGeometry.NearestOuterDoor(at, 0, FIND_REACH, houses, door, centre, normal))
			return false;

		step = RZ_Util.GroundPos(centre + normal * DOORSTEP);
		return !RZ_Util.IsWater(step);
	}

	//------------------------------------------------------------------------------------------------
	//! A spot in the open 10-45 m from `at` (the errands' "out" need: dry, mainland, not steep).
	protected static bool OpenSpot(vector at, out vector spot)
	{
		spot = RZ_PlaceFind.Near(at, 10, 45, RZ_PlaceNeeds.Get("errand.out"));
		return spot != vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	//! What a patrol stops for near its point `at`: one find (and what it leads to) from those that
	//! are true there AND have their thing really standing there (docs/MISSION_RULES.md, rules 2
	//! and 3), into `beats`, its first beat set at the thing; what the lead calls out is `call`.
	//! False when none fits (logged): the patrol walks on.
	static bool Patrol(RZ_GameModeComponent gm, RZ_Errand e, vector at, string lead, notnull array<ref RZ_DutyBeat> beats, out string call)
	{
		call = "";
		RZ_WorldFacts facts = gm.GetFacts();
		RZ_Settlement town = gm.GetSettlement(gm.FindNearestSettlement(at));
		bool sick = town && town.m_fInfection >= 40;
		array<string> kinds = {"body", "cache", "house"};
		string markKey = RZ_Traces.OldMarkKey(at);
		if (!markKey.IsEmpty())
		{
			kinds.Insert("door");
			kinds.Insert("door");
		}

		vector sceneAt;
		string sceneKey;
		if (sick || RZ_ErrandCard.OnScene("L1", at, 400, sceneAt, sceneKey))
			kinds.Insert("grave");

		array<string> rumours = {};
		if (facts)
			facts.GetRumours(rumours);

		if (!rumours.IsEmpty())
			kinds.Insert("radio");

		// In a random order, the first whose thing can stand there
		for (int k = kinds.Count() - 1; k > 0; k--)
		{
			int swap = Math.RandomInt(0, k + 1);
			string held = kinds[k];
			kinds[k] = kinds[swap];
			kinds[swap] = held;
		}

		RZ_DutyBeat b;
		RZ_DutyOption o;
		vector spot;
		foreach (string kind : kinds)
		{
			switch (kind)
			{
				case "door":
				{
					vector markPos;
					vector markOut;
					if (!RZ_Traces.MarkedDoorAt(markKey, markPos, markOut))
						break;

					vector markStep = RZ_Util.GroundPos(markPos + markOut * DOORSTEP);
					if (RZ_Util.DistXZ(markStep, at) > FIND_REACH)
						break;

					spot = markStep;
					string mark = RZ_Traces.OldMarkLine(markKey, e.m_iAsker);
					if (mark.IsEmpty())
						mark = "It wasn't there yesterday.";

					call = "Hold up. That door's been marked. Go and read it to me, I'll wait.";
					b = Find("door", "A door off the path, and a mark beside it. " + mark, spot, "", string.Format("Read the mark on the door, %1 waiting", lead));
					o = b.Opt("Copy the mark into the patrol book", "Good. Someone at the gate will know what it means.");
					o.Fx("report,record").Told("That mark you copied down was read last night. It mattered.");
					b.Opt("Leave the door alone", "Fair. Some marks aren't meant for patrols.");
					break;
				}

				case "grave":
				{
					vector graveAt;
					if (!OpenSpot(at, graveAt))
						break;

					spot = graveAt;
					call = "Grave at the field edge. Fresh. Read me the name.";
					b = Find("grave", "A fresh grave. A stick cross, a name in pencil, already running in the damp.", spot, GRAVE, "Read the name on the fresh grave");
					o = b.Opt("Read the name for the report", "Say it again. Right. Somebody will want to know where.");
					o.Fx("report,record");
					o = b.Opt("Set the cross straight and go over the name", "Leave it like that. Nobody needs to know who.");
					o.Work(RZ_EWork.WRITE, 5).Fx("later").Later("", "Someone set the cross straight on that grave at the field edge. The family asked who. I said I didn't know.");
					break;
				}

				case "radio":
				{
					vector room;
					if (!RZ_WorkSites.Indoors(at, FIND_REACH, "house", room))
						break;

					spot = room;
					string heard = rumours[Math.RandomInt(0, rumours.Count())];
					call = "Hear that? A radio, in that house. Go and see what it's saying.";
					b = Find("radio", "A radio still talking in an empty house. The battery can't have long.", spot, RADIO, "Find the radio talking in the empty house");
					o = b.Opt("Listen, and write down what it says", "Write it as you heard it. " + heard);
					o.Fx("report");
					b.Opt("Switch it off", "Good. One less voice calling things to the house.");
					break;
				}

				case "house":
				{
					vector houseStep;
					if (!DoorStep(at, true, houseStep))
						break;

					spot = houseStep;
					call = "That house. Path trodden to the door, curtains drawn. Somebody's in. Go and knock, softly.";
					b = Find("house", "A trodden path to the door. The curtains are drawn tight, and the step's been swept.", spot, "", "Knock at the house with the trodden path");
					o = b.Opt("Knock", "Something moved in there. Wait.");
					o.Flag("door").Next("hiding");
					o = b.Opt("Mark the house for the report", "Marked. Someone else can knock another day.");
					o.Fx("report,hook");
					beats.Insert(b);
					b = new RZ_DutyBeat("hiding", "A voice through the door, low: \"Don't write us down. They'll move us on. Please.\"", RZ_DutyBeat.AT_SAME);
					b.m_sIf = "door";
					b.m_fTimeout = FIND_TIMEOUT;
					b.m_bFind = true;
					b.m_sGoal = "Answer the voice behind the door";
					o = b.Opt("Put them in the report", "In the book, then. They'll be fetched in, or moved on. That isn't ours to choose.");
					o.Fx("report").Told("The ones you found behind that door have been brought in. They aren't grateful yet.");
					o = b.Opt("Say nothing, and walk on", "Nothing written. I didn't see it either.");
					o.Shady(1).Fx("local").Later("who was behind the door of the house with the trodden path", "You didn't write us down. I know what that could have cost you.");
					o = b.Opt("Leave a tin on the step", "A tin on the step. Go on, before the door opens.");
					o.Needs(RZ_EErrandItem.FOOD, 1).Fx("local").Later("who was behind the door of the house with the trodden path", "There was a tin on our step. I never saw your face, but I asked about you.");
					break;
				}

				case "body":
				{
					vector bodyAt;
					if (!OpenSpot(at, bodyAt))
						break;

					spot = bodyAt;
					ResourceName remains = RZ_ErrandSettings.Get().Props().m_sDutyBody;
					if (remains.IsEmpty())
						remains = REMAINS;

					call = "Something in the grass there. Go on, I've got you covered.";
					b = Find("body", "What's left of someone, still in his coat. Papers in the pocket. A watch on the wrist, still going.", spot, remains, "Look at the remains in the grass");
					o = b.Opt("Take the papers for the report", "A name? Someone at the gate will know it.");
					o.Fx("report,record").Told("The papers you brought in had a name on them. Someone here knew it.");
					o = b.Opt("Take the watch", "I didn't see that.");
					o.Shady(1).Fx("watch");
					break;
				}

				case "cache":
				{
					vector cacheStep;
					if (!DoorStep(at, true, cacheStep))
						break;

					spot = cacheStep;
					call = "Loose step on that porch. Have a look underneath.";
					b = Find("cache", "A metal box under the loose step. The dust on it is fresh.", spot, BOX, "Look under the loose step");
					o = b.Opt("Hand it in unopened", "Unopened. Good. It goes to the gate as it is.");
					o.Fx("report").Told("The box you handed in was somebody's savings. Paper money. Worthless, and they cried anyway.");
					o = b.Opt("Open it and take what's in it", "I'll not ask what was in it.");
					o.Shady(1).Fx("give");
					break;
				}
			}

			if (b)
			{
				beats.Insert(b);
				RZ_Log.Info(string.Format("Duty finds: %1 at %2, %3 m from the patrol's stop at %4 (errand %5).", kind, RZ_SettlementScanner.GridReference(spot), RZ_Util.RoundToInt(RZ_Util.DistXZ(spot, at)), RZ_SettlementScanner.GridReference(at), e.m_iId));
				return true;
			}

			RZ_Log.Info(string.Format("Duty finds: no %1 by the patrol's stop at %2: its thing can't stand there, left out.", kind, RZ_SettlementScanner.GridReference(at)));
		}

		beats.Clear();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! What a hunt finds at the snare line.
	static void Hunt(RZ_GameModeComponent gm, RZ_Errand e, notnull array<ref RZ_DutyBeat> beats)
	{
		RZ_DutyBeat b;
		RZ_DutyOption o;
		int roll = Math.RandomInt(0, 3);
		if (roll == 0)
		{
			b = new RZ_DutyBeat("cat", "One snare holds a cat, alive and furious, somebody's once.", RZ_DutyBeat.AT_SAME);
			o = b.Opt("Cut it loose", "It's gone before the wire hits the ground. The hunter says nothing, and nearly smiles.");
			o.Fx("later").Later("", "There's a cat been coming to the kitchen door since your hunt. It won't come in. It looks like it knows you.");
			o = b.Opt("Take it for the pot", "It's meat. Nobody at the table will ask what kind.");
			o.Fx("feed");
			beats.Insert(b);
			return;
		}

		if (roll == 1)
		{
			b = new RZ_DutyBeat("deerpath", "The hunter stops and points with two fingers. A deer path, used this morning, crossing the line.", RZ_DutyBeat.AT_SAME);
			o = b.Opt("Set a snare across it", "Set low, across the path. If it holds, that's a week of meat.");
			o.Work(RZ_EWork.INSTALL, 8).Fx("feed");
			b.Opt("Leave the path alone", "Left. The hunter shrugs. There'll be deer next year, perhaps, because of it.");
			beats.Insert(b);
			return;
		}

		b = new RZ_DutyBeat("thief", "Two snares sprung and empty, and a boot print beside them. A small boot.", RZ_DutyBeat.AT_SAME);
		o = b.Opt("Wait in the hedge for them", "You wait. The hunter waits better. After a while, someone comes.");
		o.Work(RZ_EWork.HANDS, 8).Flag("wait").Next("girl");
		o = b.Opt("Reset them and go on", "Reset, and one moved a field over.");
		o.Next("end");
		beats.Insert(b);
		b = new RZ_DutyBeat("girl", "A boy, nine or ten, with a rabbit by the ears. He sees you and doesn't run. He just holds it tighter.", RZ_DutyBeat.AT_SAME);
		b.m_sIf = "wait";
		o = b.Opt("Let him keep it", "He goes backwards for the first ten steps. The hunter looks at the sky.");
		o.Fx("local").Later("who took rabbits from the snare line", "You let me keep the rabbit. My brother ate. I've been setting my own since, away from yours.");
		o = b.Opt("Take the rabbit back", "He lets go of it. He doesn't cry until he's past the gate.");
		o.Fx("feed");
		o = b.Opt("Bring him in to the house", "He comes, because there's nowhere else. There's another mouth at the table tonight.");
		o.Fx("report,local").Later("who was brought in off the snare line", "You brought me in. I was angry. I'm not now, mostly.");
		beats.Insert(b);
	}

	//------------------------------------------------------------------------------------------------
	//! The small turn a chore can take once its work is done: one beat, where you worked. Nothing
	//! when this chore has none.
	static void Turn(RZ_GameModeComponent gm, RZ_Errand e, string duty, notnull array<ref RZ_DutyBeat> beats)
	{
		RZ_DutyBeat b;
		RZ_DutyOption o;
		switch (duty)
		{
			case "kitchen":
			{
				b = new RZ_DutyBeat("short", "The pot comes up a ladle short, with three still in the line.");
				o = b.Opt("Water it down", "Thinner, and enough. Nobody says anything. Somebody looks.");
				o = b.Opt("Put in a tin of your own", "Your tin, into the pot. The last three eat what the first three did.");
				o.Needs(RZ_EErrandItem.FOOD, 1).Pay(1).Fx("feed,regard+");
				o = b.Opt("Give up your own bowl", "You go without. It was noticed, by the one who counts.");
				o.Fx("regard+");
				break;
			}

			case "wood":
			{
				if (Math.RandomInt(0, 2) == 0)
				{
					b = new RZ_DutyBeat("rope", "The well rope has frayed through to a thread at the bucket.");
					o = b.Opt("Splice the rope", "Spliced, and whipped with twine. It'll outlast the bucket.");
					o.Work(RZ_EWork.REPAIR, 8).Pay(1).Fx("regard+");
					b.Opt("Knot it and say so", "A knot, and a word to whoever keeps the well. It'll do for a day.");
				}
				else
				{
					b = new RZ_DutyBeat("axe", "The axe head has worked loose on its handle.");
					o = b.Opt("Wedge the head", "A sliver of oak and a soak in the bucket. Tight.");
					o.Work(RZ_EWork.REPAIR, 6).Pay(1).Fx("regard+");
					o = b.Opt("Leave it for the next one", "Left leaning on the block. It's the next one's problem.");
					o.Fx("later").Later("", "The axe head came off the day after you had it. It missed a child by the width of a hand. I'm only saying.");
				}

				break;
			}

			case "wash":
			{
				b = new RZ_DutyBeat("grey", "The wash water has gone grey before you're half done.");
				o = b.Opt("Fetch clean and do it again", "Clean water, boiled, and the whole of it done over. It smells of lime and nothing else.");
				o.Work(RZ_EWork.FILL, 10).Pay(1).Fx("regard+");
				o = b.Opt("Call it done", "Done, near enough, by the look of it.");
				o.Fx("later,regard-").Later("", "Two down with their stomachs since you did the washing. Nobody's saying it was the water. Nobody's saying it wasn't.");
				break;
			}
		}

		if (b)
			beats.Insert(b);
	}
}
