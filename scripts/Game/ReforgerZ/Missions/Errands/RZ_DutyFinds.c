//! What duties find (docs/ERRANDS.md, "Things that happen"): the beats a patrol meets at a point, a
//! hunter at the snares, and the small turns a chore takes. Each is drawn from the world as it is
//! at that spot (Traces' door marks and scenes, the rumours going round, the weather, the hour, the
//! stage, how sick the town is), most of them not a fight, with a choice where one fits. Data for
//! RZ_DutyScenes. Server only.
class RZ_DutyFinds
{
	protected static const float FIND_TIMEOUT = 150;	// a patrol waits this long at a find

	//------------------------------------------------------------------------------------------------
	protected static RZ_DutyBeat Find(string key, string line)
	{
		RZ_DutyBeat b = new RZ_DutyBeat(key, line, RZ_DutyBeat.AT_NEAR);
		b.m_fTimeout = FIND_TIMEOUT;
		return b;
	}

	//------------------------------------------------------------------------------------------------
	//! What stands off a patrol's point: one find (and what it leads to) from those that are true
	//! there, into `beats`. Nothing when none is.
	static void Patrol(RZ_GameModeComponent gm, RZ_Errand e, vector at, notnull array<ref RZ_DutyBeat> beats)
	{
		RZ_WorldFacts facts = gm.GetFacts();
		RZ_Settlement town = gm.GetSettlement(gm.FindNearestSettlement(at));
		bool rain = gm.GetEnvironment() && gm.GetEnvironment().IsRaining();
		bool dusk = facts && (facts.IsDusk() || facts.IsNight());
		bool late = facts && facts.Stage() >= RZ_EOutbreakStage.COLLAPSE;
		bool sick = town && town.m_fInfection >= 40;
		array<string> kinds = {"drawing", "dog", "cache", "body"};
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

		if (rain || late)
			kinds.Insert("tracks");

		if (dusk || (town && facts && !facts.IsLivedIn(gm.FindNearestSettlement(at))))
			kinds.Insert("smoke");

		if (sick)
			kinds.Insert("pit");

		if (late)
			kinds.Insert("truck");

		kinds.Insert("others");
		string kind = kinds[Math.RandomInt(0, kinds.Count())];
		RZ_DutyBeat b;
		RZ_DutyOption o;
		switch (kind)
		{
			case "door":
			{
				string mark = RZ_Traces.OldMarkLine(markKey, e.m_iAsker);
				if (mark.IsEmpty())
					mark = "It wasn't there yesterday.";

				b = Find("door", "A door off the path, and a mark on it. " + mark);
				o = b.Opt("Copy the mark into the patrol book", "Copied down as it stands. Someone at the gate will know what it means.");
				o.Fx("report,record").Told("That mark you copied down was read last night. It mattered.");
				b.Opt("Leave the door alone", "Left as it was. Some marks aren't meant for patrols.");
				break;
			}

			case "grave":
			{
				b = Find("grave", "A fresh grave by the hedge. A stick cross, a name in pencil, already running in the damp.");
				o = b.Opt("Read the name for the report", "The name goes in the book. Somebody will want to know where.");
				o.Fx("report,record");
				o = b.Opt("Set the cross straight and go over the name", "Straightened, and the name gone over in pencil. Nobody saw who did it.");
				o.Work(RZ_EWork.WRITE, 5).Fx("later").Later("", "Someone set the cross straight on that grave by the hedge. The family asked who. I said I didn't know.");
				break;
			}

			case "radio":
			{
				string heard = rumours[Math.RandomInt(0, rumours.Count())];
				b = Find("radio", "A radio still talking in an empty kitchen. The battery can't have long.");
				o = b.Opt("Listen, and write down what it says", "It says this, twice over, then hiss. " + heard);
				o.Fx("report");
				o = b.Opt("Switch it off", "Quiet. One less voice calling things to the house.");
				break;
			}

			case "drawing":
			{
				b = Find("drawing", "A child's drawing pinned inside a gate. Four people and a dog, and a sun with a face.");
				o = b.Opt("Take it back for the board", "Folded into your coat, carefully. It'll go on the board.");
				o.Fx("later,regard+").Later("", "That drawing you brought in is up on the board. Three people have stood in front of it today, not saying anything.");
				b.Opt("Leave it where it's pinned", "Left for whoever comes back for it. If anyone does.");
				break;
			}

			case "tracks":
			{
				b = Find("tracks", "Tracks in the mud, a great many, barefoot, all going the one way.");
				o = b.Opt("Count them and mark the way they went", "Thirty, near enough, and going towards the low ground. Marked.");
				o.Fx("report,record").Told("They moved the night post because of the tracks you counted. Nothing came. That's how you know it worked.");
				b.Opt("Scuff them out", "Gone. Nobody else will be frightened by them.");
				break;
			}

			case "smoke":
			{
				b = Find("smoke", "Smoke from a chimney that ought to be cold.");
				o = b.Opt("Go to the door", "You knock. Something moves inside, and stops.");
				o.Flag("door").Next("hiding");
				o = b.Opt("Mark the house for the report", "Marked. Someone else will knock, another day.");
				o.Fx("report,hook");
				beats.Insert(b);
				b = Find("hiding", "A voice through the door, low. Don't write us down. They'll move us on. Please.");
				b.m_iWhere = RZ_DutyBeat.AT_SAME;
				b.m_sIf = "door";
				o = b.Opt("Put them in the report", "In the book. They'll be fetched in, or moved on. That isn't yours to choose.");
				o.Fx("report").Told("The ones you found behind that door have been brought in. They aren't grateful yet.");
				o = b.Opt("Say nothing, and walk on", "Nothing written. The door stays shut behind you.");
				o.Shady(1).Fx("local").Later("who was behind the door with the cold chimney", "You didn't write us down. I know what that could have cost you.");
				o = b.Opt("Leave a tin on the step", "A tin on the step. The door opens a hand's width after you've gone.");
				o.Needs(RZ_EErrandItem.FOOD, 1).Fx("local").Later("who was behind the door with the cold chimney", "There was a tin on our step. I never saw your face, but I asked about you.");
				break;
			}

			case "pit":
			{
				b = Find("pit", "One of the infected in a drainage pit, up to its chest, reaching. It can't climb out. It hasn't made a sound yet.");
				o = b.Opt("Finish it with the knife", "Done, and quietly. One fewer for the town to send.");
				o.Work(RZ_EWork.HANDS, 6).Fx("infect-");
				o = b.Opt("Mark the pit for the report", "Marked. Someone will come with a pole and a sack.");
				o.Fx("report");
				break;
			}

			case "body":
			{
				b = Find("body", "A body in the ditch, a week old. Papers in the coat. A watch on the wrist, still going.");
				o = b.Opt("Take the papers for the report", "The papers, and nothing else. There's a name on them.");
				o.Fx("report,record").Told("The papers you brought in had a name on them. Someone here knew it.");
				o = b.Opt("Take the watch", "The strap is stiff. It comes off.");
				o.Shady(1).Fx("watch");
				break;
			}

			case "truck":
			{
				b = Find("truck", "A truck off the road, nose down, its load still roped. Nobody has found it yet.");
				o = b.Opt("Mark it for the quartermaster", "Marked on the map, with what's on it. That's a week of someone's stores.");
				o.Fx("report,morale+").Told("They brought that truck's load in this morning. That was yours, and it's been said.");
				o = b.Opt("Take a share before anyone counts", "One thing off the back, into your pack. The ropes go back as they were.");
				o.Shady(2).Fx("give");
				break;
			}

			case "dog":
			{
				b = Find("dog", "A dog in a gateway, thin, watching. A collar with the name worn off.");
				o = b.Opt("Give it something to eat", "It takes it from the ground, not from your hand. Then it follows, at a distance.");
				o.Needs(RZ_EErrandItem.FOOD, 1).Fx("later,morale+").Later("", "That dog followed your patrol in. It sleeps by the gate now. The children have given it four names.");
				b.Opt("Send it off", "It goes, looking back twice.");
				break;
			}

			case "cache":
			{
				b = Find("cache", "A tin box under a loose step. The dust on it is fresh.");
				o = b.Opt("Hand it in unopened", "Unopened, into the pack, for the gate.");
				o.Fx("report").Told("The box you handed in was somebody's savings. Paper money. Worthless, and they cried anyway.");
				o = b.Opt("Open it and take what's in it", "Open. Most of it is paper. One thing isn't.");
				o.Shady(1).Fx("give");
				break;
			}

			default:
			{
				b = Find("others", "Another patrol across the field, not ours, stopped as you are stopped. Nobody has raised anything yet.");
				o = b.Opt("Raise an open hand", "A hand goes up over there too. Then they turn and go.");
				o.Fx("record,morale+");
				b.Opt("Stand until they go", "They go first. Somebody will call that a win.");
				break;
			}
		}

		beats.Insert(b);
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
		b = new RZ_DutyBeat("girl", "A girl, nine or ten, with a rabbit by the ears. She sees you and doesn't run. She just holds it tighter.", RZ_DutyBeat.AT_SAME);
		b.m_sIf = "wait";
		o = b.Opt("Let her keep it", "She goes backwards for the first ten steps. The hunter looks at the sky.");
		o.Fx("local").Later("who took rabbits from the snare line", "You let me keep the rabbit. My brother ate. I've been setting my own since, away from yours.");
		o = b.Opt("Take the rabbit back", "She lets go of it. She doesn't cry until she's past the gate.");
		o.Fx("feed");
		o = b.Opt("Bring her in to the house", "She comes, because there's nowhere else. There's another mouth at the table tonight.");
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
