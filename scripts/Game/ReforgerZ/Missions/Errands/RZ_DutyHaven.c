//! The Haven's own duties (docs/ERRANDS.md, "The Haven's duties"): the school room, the garden, the
//! door round at dusk, the roll call and the swap table, as beats for RZ_DutyScenes. Three of them
//! are worked out and led by the player: you gather what you can see and hear, and the call is
//! yours. The garden's beds are kept per house between visits (the errands' saved counters).
//! Server only.
class RZ_DutyHaven
{
	//------------------------------------------------------------------------------------------------
	protected static string Ord(int i)
	{
		array<string> words = {"first", "second", "third", "fourth", "fifth"};
		return words[Math.ClampInt(i, 0, 4)];
	}

	// ================================================================ The school room (simple)
	//------------------------------------------------------------------------------------------------
	//! Count heads, find the one hiding, and decide what to do about why.
	static void School(RZ_Errand e, RZ_DutyScene scene)
	{
		RZ_Rng rng = RZ_DutyPosts.Seed(e, 13);
		string child = RZ_NoteText.FirstName(rng);
		RZ_DutyScript.Into(scene.m_aBeats);
		RZ_DutyScript.T("count", "", RZ_DutyBeat.AT_HERE);
		RZ_DutyScript.O("Count the heads at the benches", string.Format("Eleven. The slate says twelve. A small coat is still on its peg, and the name sewn in it is %1.", child), "hold:4");
		RZ_DutyBeat look = RZ_DutyScript.T("look", "Four places a child goes to not be found.");
		look.m_iPicks = 4;
		int hidden = Math.RandomInt(0, 4);
		array<string> places = {"Look behind the woodpile", "Look up in the loft", "Look under the stairs", "Look in the hen house"};
		array<string> empty = {"Nothing. A cat, offended.", "Dust, and a doll with one arm.", "A broom, a bucket, and the smell of onions.", "Hens. They've seen nothing, and say so."};
		foreach (int i, string place : places)
		{
			if (i == hidden)
				RZ_DutyScript.O(place, "There. Knees up, a book open, not reading it.", "out:9 ends flag:found");
			else
				RZ_DutyScript.O(place, empty[i], "out:9");
		}

		RZ_DutyScript.T("why", string.Format("%1 won't come out. It isn't the sums. The desk beside theirs has been empty since Tuesday.", child), RZ_DutyBeat.AT_SAME, "found");
		RZ_DutyScript.O("Sit down beside them a while", "You sit. After a time the book gets read aloud, badly, to both of you. Then you walk back together.", "hold:8 regard+ later", "", string.Format("%1 asks every morning whether you're taking the school room again. I've stopped pretending I don't know why.", child));
		RZ_DutyScript.O("Carry them back in", "Carried in under one arm, furious. Twelve at the benches. Nobody learns anything else today.");
		RZ_DutyScript.O("Leave them be, and tell the teacher", "Left with the book. The teacher nods, and moves the empty desk to the wall.", "regard+");
	}

	// ================================================================ The garden (worked out, when something's wrong)
	//------------------------------------------------------------------------------------------------
	protected static string Bed(RZ_Errand e, int bed, string what)
	{
		return string.Format("garden.%1.%2.%3", e.m_iHouse, bed, what);
	}

	//------------------------------------------------------------------------------------------------
	protected static string Crop(int crop)
	{
		if (crop == 1)
			return "potatoes";

		if (crop == 2)
			return "beans";

		return "onions";
	}

	//------------------------------------------------------------------------------------------------
	//! The house's four beds, kept between visits: stage 0 fallow, 1 dug, 2 planted, 3 watered;
	//! crop 1 potatoes (two season days, 2 food), 2 beans (one day, 1 food), 3 onions (two days, 1
	//! food and the house's morale). This visit works the bed that most needs it, and then one thing
	//! happens.
	static void Garden(RZ_Errands dealer, RZ_Errand e, RZ_DutyScene scene)
	{
		int today = RZ_SeasonClock.Day();
		int bed = 0;
		int best = -1;
		for (int i = 0; i < 4; i++)
		{
			int stage = dealer.Counter(Bed(e, i, "stage"));
			int crop = dealer.Counter(Bed(e, i, "crop"));
			int since = today - dealer.Counter(Bed(e, i, "day"));
			int need = 1;
			int grows = 2;
			if (crop == 2)
				grows = 1;

			// Ripe first, then dry, then to plant, then to dig
			if (stage == 3 && since >= grows)
				need = 5;
			else if (stage == 2)
				need = 4;
			else if (stage == 1)
				need = 3;
			else if (stage == 0)
				need = 2;

			if (need > best)
			{
				best = need;
				bed = i;
			}
		}

		string stageKey = Bed(e, bed, "stage");
		string cropKey = Bed(e, bed, "crop");
		string dayKey = Bed(e, bed, "day");
		int grown = dealer.Counter(cropKey);
		string name = string.Format("the %1 bed", Ord(bed));
		RZ_DutyScript.Into(scene.m_aBeats);
		if (best == 5)
		{
			RZ_DutyBeat ripe = RZ_DutyScript.T("lift", string.Format("The %1 in %2 are ready. You can tell by the tops.", Crop(grown), name), RZ_DutyBeat.AT_GATE);
			ripe.m_sThing = RZ_DutyThings.Props().m_sBedRipe;
			RZ_DutyScript.O("Lift what's ripe", "Up they come, with the soil still on. A sack, and a bit over.", "dig hold:8 set:" + stageKey + "=0");
			RZ_DutyScript.T("carry", "", RZ_DutyBeat.AT_HERE);
			if (grown == 1)
				RZ_DutyScript.O("Carry the sack to the kitchen", "Potatoes on the kitchen table. The cook counts them twice and says a number that isn't a word.", "feed2 regard+");
			else if (grown == 2)
				RZ_DutyScript.O("Carry the sack to the kitchen", "Beans, in a heap. Green on every plate tonight.", "feed regard+");
			else
				RZ_DutyScript.O("Carry the sack to the kitchen", "Onions, to be plaited and hung. The whole house will smell of supper for a month.", "feed morale+");
		}
		else if (best == 4)
		{
			RZ_DutyScript.T("well", string.Format("The %1 in %2 are dry to the knuckle.", Crop(grown), name), RZ_DutyBeat.AT_GATE);
			RZ_DutyScript.O("Fill the can at the well", "Full to the brim, and heavy.", "fill hold:5");
			RZ_DutyBeat dry = RZ_DutyScript.T("water", "", RZ_DutyBeat.AT_NEAR);
			dry.m_sThing = RZ_DutyThings.Props().m_sBedPlanted;
			RZ_DutyScript.O("Water the row", "Slowly, at the roots. The soil goes dark and stays dark.", "hold:5 set:" + stageKey + "=3");
		}
		else if (best == 3)
		{
			RZ_DutyBeat dug = RZ_DutyScript.T("seed", string.Format("The seed tin, for %1. Three packets, and enough of each for one bed.", name), RZ_DutyBeat.AT_GATE);
			dug.m_sThing = RZ_DutyThings.Props().m_sBedDug;
			string planted = string.Format(" set:%1=2 set:%2=%3 set:", stageKey, dayKey, today) + cropKey + "=";
			RZ_DutyScript.O("Plant potatoes", "Potatoes. Two days in the ground, and they fill a house.", "hold:6" + planted + "1");
			RZ_DutyScript.O("Plant beans", "Beans. Up by tomorrow, and gone by the day after.", "hold:6" + planted + "2");
			RZ_DutyScript.O("Plant onions", "Onions. Slow, and they keep, and a kitchen with onions in it is a kitchen.", "hold:6" + planted + "3");
		}
		else
		{
			RZ_DutyScript.T("dig", string.Format("Nothing in %1 but couch grass and a bent fork.", name), RZ_DutyBeat.AT_GATE);
			RZ_DutyScript.O("Dig the bed over", "Dug, turned and raked. It looks like somewhere a thing could grow.", "dig hold:9 set:" + stageKey + "=1");
		}

		Happens(e, scene, grown);
	}

	//------------------------------------------------------------------------------------------------
	//! One thing that happens in a garden, drawn from eight.
	protected static void Happens(RZ_Errand e, RZ_DutyScene scene, int crop)
	{
		array<ref RZ_DutyBeat> picked = {};
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		RZ_WorldFacts facts = gm.GetFacts();
		bool dusk = facts && facts.IsDusk();
		RZ_DutyScript.Pool();
		RZ_DutyScript.G("crows");
		RZ_DutyScript.T("crows", "Crows on the far bed, six of them, working along the row like men with a job.", RZ_DutyBeat.AT_NEAR);
		RZ_DutyScript.O("Run at them, shouting", "They go up, and sit on the fence, and wait for you to leave.", "heat");
		RZ_DutyScript.O("Hang a tin on a string", "A tin lid on a string, turning. They hate it. It will work until they stop hating it.", "hold:6 regard+");
		RZ_DutyScript.G("rabbits");
		RZ_DutyScript.T("rabbits", "Rabbits have been in. The young tops are bitten off clean, at a slant.", RZ_DutyBeat.AT_NEAR);
		RZ_DutyScript.O("Set a snare in the run", "A snare in the gap under the fence. By morning it holds something.", "hold:7 meat");
		RZ_DutyScript.O("Mend the fence at the gap", "Wired shut. The rabbits will have to find somebody else's supper.", "fix hold:7 regard+");
		RZ_DutyScript.G("blight");
		RZ_DutyBeat rows = RZ_DutyScript.T("blight", "One row is dying and the next is not. Something is doing it. Look before you pull anything, then decide.", RZ_DutyBeat.AT_NEAR);
		rows.m_iPicks = 3;
		int cause = Math.RandomInt(0, 3);
		array<string> leaf = {"Brown blotches with a pale edge, and grey fur under the leaf.", "The leaves are whole, only limp and yellowing from the bottom.", "Ragged holes in every leaf, and a silver trail on the soil."};
		array<string> root = {"The roots are sound, white and firm.", "The root is tunnelled through, and a thin orange grub drops out.", "The roots are sound. Something has been at the stems, above ground."};
		array<string> soil = {"Wet. It has stood wet for days.", "Dry, and crumbling from the fork.", "Damp, and under the board at the bed's end, a dozen slugs."};
		RZ_DutyScript.O("Turn a leaf over", leaf[cause], "out:3");
		RZ_DutyScript.O("Pull one plant and look at the root", root[cause], "out:3 dig");
		RZ_DutyScript.O("Push a finger into the soil", soil[cause], "out:3");
		RZ_DutyScript.T("cure", "What is killing the row?");
		array<string> right = {"right regard+", "right regard+", "right regard+"};
		array<string> codes = {"wrong", "wrong", "wrong"};
		codes[cause] = right[cause];
		RZ_DutyScript.O("It's blight. Pull the row and burn it", "The whole row out and on the fire, before it jumps.", "burn hold:8 " + codes[0], "", "", "You pulled the right row. The bed next to it is still standing because of it.");
		RZ_DutyScript.O("It's wireworm. Lift it, turn the soil", "Lifted, and the bed turned for the birds to pick over.", "dig hold:8 " + codes[1]);
		RZ_DutyScript.O("It's slugs. Lay boards, pick them off", "Boards down, and a tin of them carried to the hens.", "hold:8 " + codes[2]);
		RZ_DutyScript.G("dugup");
		int found = Math.RandomInt(0, 4);
		if (found == 0)
		{
			RZ_DutyScript.T("box", "The fork rings on something. A tin box, wrapped in oilcloth, not long buried.", RZ_DutyBeat.AT_NEAR);
			RZ_DutyScript.O("Hand it in unopened", "Carried in as it is. It's somebody's, and they'll be asked.", "report regard+", "", "", "The box from the garden was the old man's. Photographs. He hasn't put them down since.");
			RZ_DutyScript.O("Open it and keep what's in it", "A watch, and paper money, and a ring too small for you.", "shady1 watch");
		}
		else if (found == 1)
		{
			RZ_DutyScript.T("bone", "The fork turns up a bone. Then another. Too small for a cow, and laid straight.", RZ_DutyBeat.AT_NEAR);
			RZ_DutyScript.O("Cover it, and move the bed over", "Covered again, and a stone set on it. The bed starts a yard to the left now.", "hold:6 later", "", "Somebody set a stone in the garden. Nobody's asked what's under it. Everybody walks round it.");
			RZ_DutyScript.O("Tell the house what's under the bed", "You say it, at the door. Nobody is surprised. Somebody goes for the book.", "report record");
		}
		else if (found == 2)
		{
			RZ_DutyScript.T("rifle", "Something long, in a sack, under the bean row. A rifle, greased, and forty rounds in a jar.", RZ_DutyBeat.AT_NEAR);
			RZ_DutyScript.O("Carry it in to the storekeeper", "Laid on the store's table. The storekeeper looks at it, and then at who isn't in the room.", "report morale+", "", "", "That rifle from the garden is on the wall now. Whoever buried it hasn't said a word. They know we know.");
			RZ_DutyScript.O("Bury it again, deeper", "Back in the sack, a spit deeper. Whoever it belongs to will find it gone from where they left it.", "shady1 hold:6 later", "", "I went for what I'd buried under the beans and it was lower than I left it. I know it was you. I'm not sure what I owe you.");
		}
		else
		{
			RZ_DutyScript.T("toys", "A biscuit tin, a hand's depth down. A lead soldier, three marbles, and a drawing of a house with too many windows.", RZ_DutyBeat.AT_NEAR);
			RZ_DutyScript.O("Take it in to the children", "The tin goes round the benches. Nobody claims it. Everybody wants the soldier.", "morale+ regard+");
			RZ_DutyScript.O("Put it back where it was", "Back in the ground, and the soil pressed down. It was put there to be found by one person.", "later", "", "Someone dug up a tin in the garden and put it back. A child told me. He won't say how he knows what was in it.");
		}

		RZ_DutyScript.G("frost", dusk);
		RZ_DutyScript.T("frost", "The sky has gone hard and clear, and the breath shows. It will freeze tonight.", RZ_DutyBeat.AT_NEAR);
		RZ_DutyScript.O("Cover the beds with sacking", "Every bed under sacking, pegged at the corners. They'll see the morning.", "hold:10 +1 regard+");
		RZ_DutyScript.O("Leave them to it", "You leave them. The tender ones won't thank you.", "later regard-", "", "The frost had the young beans the night you left the beds open. We'd sacks for it. They were by the door.");
		RZ_DutyScript.G("thief");
		RZ_DutyScript.T("thief", "Someone is crouched in the carrot row with a full shirt front. Not one of the house. They haven't seen you.", RZ_DutyBeat.AT_NEAR);
		RZ_DutyScript.O("Let them go with what they have", "You cough. They run, holding the shirt together with both hands.", "local", "who took carrots from the garden", "You coughed before you came round the shed. I've thought about that cough a good deal.");
		RZ_DutyScript.O("Bring them in to the house", "By the collar, to the door. They're given soup before anyone asks a question.", "report local", "who was caught in the carrot row", "You brought me in by the collar. They gave me soup. I'd have brought myself in, if I'd known about the soup.");
		RZ_DutyScript.O("Give them a tin as well", "A tin on top of the carrots. They look at it as if it might go off.", "food1 local", "who took carrots from the garden", "Carrots, and then a tin on top. I didn't know what to do with my face. I still don't.");
		RZ_DutyScript.G("quarrel");
		RZ_DutyScript.T("quarrel", "Two of the house at the end bed, each with a hand on the same fork. One dug it. The other's mother planted it, before.", RZ_DutyBeat.AT_NEAR);
		RZ_DutyScript.O("It belongs to who works it", "The one who dug it keeps the fork. The other goes in without a word.", "later", "", "You gave my mother's bed to the one who digs it. You were right. I'm not speaking to you until I've finished minding.");
		RZ_DutyScript.O("Split the bed down the middle", "A string down the middle. They'll each grow the same thing, to spite each other. The house eats either way.", "hold:5 feed");
		RZ_DutyScript.G("welllow");
		RZ_DutyScript.T("welllow", "The bucket comes up a third full, and what's in it is brown.", RZ_DutyBeat.AT_NEAR);
		RZ_DutyScript.O("Water only the beds that need it", "Half a can each, to the ones that are flagging. The rest will have to hope for rain.", "hold:5");
		RZ_DutyScript.O("Carry it from the stream instead", "Four trips, with the yoke. Your shoulders will have opinions tomorrow.", "fill hold:12 +1 regard+");
		RZ_DutyScript.Draw(1, picked);
		foreach (RZ_DutyBeat b : picked)
		{
			scene.m_aBeats.Insert(b);
		}
	}

	// ================================================================ The door round at dusk (worked out)
	//------------------------------------------------------------------------------------------------
	//! Four doors: knock and listen at each, then chalk the one where somebody is living who
	//! hasn't said so. One is hiding, one is the dead's, the others are empty or honest.
	static void DoorRound(RZ_Errand e, RZ_DutyScene scene)
	{
		array<string> hiding = {"No answer. But a kettle has just been lifted off the heat: the lid is still ticking.", "No answer. A curtain upstairs is an inch further across than it was a moment ago.", "No answer. Wet footprints on the step, small ones, going in and not coming out."};
		array<string> empty = {"The knock echoes. Nothing in there but the echo.", "Nobody. A calendar on the wall inside, three weeks behind.", "Nothing. Leaves have blown up against the door and not been disturbed."};
		array<string> honest = {"A voice, at once: who is it? It's the round. Goodnight, then.", "The bolt is drawn before you've finished knocking. They were expecting the round."};
		array<string> dead = {"Something answers the knock by walking into the door. And again. And again.", "A slow scrape along the inside of the door, at about the height of a hand."};
		int hide = Math.RandomInt(0, 4);
		int walker = (hide + 1 + Math.RandomInt(0, 3)) % 4;
		RZ_DutyScript.Into(scene.m_aBeats);
		RZ_DutyBeat round = RZ_DutyScript.T("round", "Four doors on the round. Knock and listen at each one. Then you'll be asked which to chalk.", RZ_DutyBeat.AT_GATE);
		round.m_iPicks = 4;
		for (int i = 0; i < 4; i++)
		{
			string heard = empty[Math.RandomInt(0, empty.Count())];
			if (i == hide)
				heard = hiding[Math.RandomInt(0, hiding.Count())];
			else if (i == walker)
				heard = dead[Math.RandomInt(0, dead.Count())];
			else if (Math.RandomInt(0, 2) == 0)
				heard = honest[Math.RandomInt(0, honest.Count())];

			RZ_DutyScript.O(string.Format("Knock at the %1 door", Ord(i)), heard, "out:18 hold:4");
		}

		RZ_DutyScript.T("chalk", "Which door has somebody living behind it who hasn't said so? Chalk that one for the house.");
		for (int d = 0; d < 4; d++)
		{
			string code = "wrong";
			string told = "";
			if (d == hide)
			{
				code = "right flag:found report";
				told = "The door you chalked had a family behind it. They'd been afraid to knock on ours. They've knocked now.";
			}
			else if (d == walker)
			{
				told = "You chalked a door for the living that had the dead behind it. The two who went to fetch them in are still shaking.";
				code = "wrong morale-";
			}

			RZ_DutyScript.O(string.Format("Chalk the %1 door", Ord(d)), "A cross and the date, in chalk, at eye height.", code + " write", "", "", told);
		}

		RZ_DutyScript.T("face", "As you turn to go, a face at the glass of the door you chalked. A man. He shakes his head, slowly. Not yet.", RZ_DutyBeat.AT_SAME, "found");
		RZ_DutyScript.O("Leave the chalk, and tell the house", "The chalk stays. The house will come tomorrow, with bread, and without rifles.", "local", "who was behind the chalked door", "You chalked us. I was angry for a day. Then they came with bread.");
		RZ_DutyScript.O("Rub it out, and say nothing", "Your sleeve takes the cross off the door. He puts a hand flat on the glass, and is gone.", "shady1 local later", "who was behind the door you didn't chalk", "You rubbed the chalk off. We'll come in when we can stand to. I'll know you when we do.");
	}

	// ================================================================ The roll call (worked out)
	//------------------------------------------------------------------------------------------------
	//! The list says one more than the kitchen fed. Five places to look, three looks, and then who
	//! is missing and why: out and not back, dead in the night and not written, or gone with their
	//! things.
	static void RollCall(RZ_Errand e, RZ_DutyScene scene)
	{
		RZ_Rng rng = RZ_DutyPosts.Seed(e, 19);
		string who = RZ_DutyPosts.Name(rng);
		int truth = Math.RandomInt(0, 3);
		array<string> pegs = {"A peg with no coat on it, and no boots under.", "A coat on every peg. One hasn't moved since yesterday: dust on the shoulders.", "An empty peg, and under it a clean square on the floor where a pack stood."};
		array<string> gate = {"The gate book has two out for wood at first light, and one line signed back in.", "Nobody out since yesterday. The bar hasn't been lifted.", "The gate book shows nobody out. But the back fence wire has been unhooked and hooked again, wrong."};
		array<string> beds = {"A bed made, tight, as it's made every morning.", "A bed slept in and cold. The blanket is drawn up over the pillow.", "A bed stripped to the ticking. Even the blanket's gone."};
		array<string> bowls = {"Nineteen bowls washed. The list has twenty names.", "Nineteen bowls washed. One was filled, and carried out, and came back full.", "Nineteen bowls. And the bread bin is short a loaf nobody signed for."};
		array<string> talk = {"The children say someone promised to bring back a bird's egg.", "The children have been told to play outside today, and not to ask.", "The children say somebody said goodbye to the dog."};
		RZ_DutyScript.Into(scene.m_aBeats);
		RZ_DutyBeat look = RZ_DutyScript.T("looks", string.Format("The list on the door has twenty names. The kitchen fed nineteen. Find out who, and why. Five places, and time for three of them.", who), RZ_DutyBeat.AT_HERE);
		look.m_iPicks = 3;
		RZ_DutyScript.O("Look along the coat pegs", pegs[truth], "out:6");
		RZ_DutyScript.O("Read the gate book", gate[truth], "out:12");
		RZ_DutyScript.O("Go round the beds", beds[truth], "out:7 care");
		RZ_DutyScript.O("Count the bowls in the kitchen", bowls[truth], "out:5");
		RZ_DutyScript.O("Ask the children", talk[truth], "out:9");
		RZ_DutyScript.T("who", string.Format("It's %1 who isn't on the benches. What do you write against the name?", who), RZ_DutyBeat.AT_HERE);
		array<string> code = {"wrong", "wrong", "wrong"};
		code[truth] = "right record";
		RZ_DutyScript.O("Out, and not back", "Written. Two will go and look along the wood road before dark.", code[0] + " write flag:out", "", "", "");
		RZ_DutyScript.O("Dead in the night, not written", "Written. Somebody goes to the room, and somebody else for the clerk.", code[1] + " write flag:dead");
		RZ_DutyScript.O("Gone, with their things", "Written. The name is struck from the kitchen's count, and the store is checked.", code[2] + " write flag:gone");
		// Out and not back, written: two go to look along the wood road, and you can go with them
		array<string> found = {
			"In the ditch past the second gate: %1, a turned ankle and a day's fright, too ashamed to shout.",
			"In the loft of the old barn: %1, who hid from something on the road at dusk and hasn't dared come down.",
			"At the wood's edge: %1, cold, and bitten, and quite still. The wood they went for is stacked beside them."};
		int fate = Math.RandomInt(0, 3);
		RZ_DutyScript.T("after", string.Format("Two are going out along the wood road to look for %1, before the light goes.", who), RZ_DutyBeat.AT_GATE, "out");
		RZ_DutyScript.O("Go with them", "The three of you, along the wood road, calling the name.", "flag:look");
		RZ_DutyScript.O("Let them go", "They go. You hear what they found at supper.", "later", "", string.Format("They found %1 on the wood road. Somebody should have gone with them.", who));
		string what = string.Format(found[fate], who);
		if (truth != 0)
			what = string.Format("Nothing on the wood road, all the way to the trees. Wherever %1 is, it isn't out here.", who);

		RZ_DutyScript.T("found", "", RZ_DutyBeat.AT_OUT, "look");
		if (truth == 0 && fate == 0)
			RZ_DutyScript.O("Look along the ditches", what, "search hold:6 +2 morale+ local", "who was found in the ditch on the wood road", "You were one of the three who came for me. I'd have lain there all night out of shame. I owe you a supper, at least.");
		else if (truth == 0 && fate == 1)
			RZ_DutyScript.O("Look in the barns", what, "search hold:6 +2 regard+ local", "who was talked down out of a barn loft", "You talked me down off that loft. I'd heard it on the road, and I couldn't move. I still hear it.");
		else if (truth == 0)
			RZ_DutyScript.O("Look along the wood's edge", what, "search hold:6 morale- record", "", "", "We buried them by the wood they went for. It was good of you to go and look.");
		else
			RZ_DutyScript.O("Look all the way to the trees", what, "search hold:6 wrong", "", "", "You had them written out and not back. They weren't. Think on that, next time you read a house.");

		if (truth == 1)
		{
			RZ_DutyBeat card = RZ_DutyScript.T("card", string.Format("On the blanket, an identity card. %1, and a face twenty years younger. It would pass for nearly anyone, now.", who), RZ_DutyBeat.AT_NEAR, "dead");
			card.m_sThing = RZ_DutyThings.For("rollcall:card");
			RZ_DutyScript.O("Hand the card to the clerk", "Into the clerk's hand, for the book. A name with a face is worth keeping.", "report regard+");
			RZ_DutyScript.O("Keep the card", "Into your coat. A dead name is a door, in some places.", "shady1 later", "", string.Format("The clerk went to write %1 in the book and there was no card to copy from. There's only your word for the spelling.", who));
		}
	}

	// ================================================================ The swap table (worked out)
	//------------------------------------------------------------------------------------------------
	//! Four come to the table, one at a time, each with something laid on it and something they
	//! want. Handle it and ask about it (two of the three), then call it. Drawn from sixteen.
	static int SwapTable(RZ_Errand e, RZ_DutyScene scene)
	{
		array<ref RZ_DutyBeat> picked = {};
		RZ_DutyScript.Pool();
		Swap("ring", "A man lays a wedding ring on the cloth. Two tins, he says. He doesn't look at it.",
			"Worn thin on the palm side. Twenty years of wearing, at least.", "It's mine to sell, he says. He'd have said sell it. He said it about everything.", "They're for the boy next door to him, the one whose father can't get up.",
			2, "The ring for two tins. He takes them without counting.", "You push it back to him. He closes his hand on it and stands a while.", "Two tins, and he keeps the ring on a promise. He says nothing. He'll keep the promise.",
			"who sold his ring at the swap table", "You gave me the tins and left me the ring. I wear it on a string, where nobody asks.");
		Swap("watch", "A child, seven or eight, puts a man's watch on the cloth with both hands. He wants bread.",
			"Heavy, good, still wound. A name scratched inside the back. Not a child's name.", "It was my dad's, he says. Then, quieter: it was on the man by the bridge.", "It's for him and his brother. He has to be told twice that you asked.",
			2, "Bread for a dead man's watch. He runs before you can change your mind.", "You keep the watch back. He stares at the bread, and goes.", "Bread for nothing, and the watch to the clerk for the book. He doesn't understand it and takes it anyway.",
			"who brought a watch to the swap table", "You gave me the bread and I didn't have to give the watch. My brother says you're soft. He ate half.");
		Swap("coat", "A man takes his coat off and lays it on the cloth. It's freezing. A tin of meat, he says.",
			"A good coat. Still warm from him. His shirt under it is two shirts.", "I've another at home, he says. He hasn't.", "For himself. He hasn't eaten since the day before yesterday, and says so as if it were someone else.",
			2, "The coat for the tin. He eats it standing at the table, shaking.", "You push the coat back. He puts it on, and it doesn't make him less hungry.", "The tin for an hour at the woodpile instead. He keeps the coat, and his pride mostly.",
			"who tried to sell his coat in the cold", "I've still the coat. I split wood for an hour and ate like a lord. Don't tell anyone I cried.");
		Swap("aspirin", "A neat man with a neat packet. Antibiotics, he says, from the hospital. Six tins.",
			"The foil is right. The pills are the wrong size, and the stamp on them is a headache brand.", "Straight off the dispensary shelf, he says. Which dispensary, he can't say.", "For the house, he says, out of public spirit. And six tins.",
			1, "Six tins for chalk and aspirin. Somebody will take them for a fever that needs more.", "You push the packet back across. He's surprised, and then not there.", "You call the nurse over. He breaks one, tastes it, and looks at him until he leaves.",
			"", "");
		Swap("stolen", "A lad puts a silver spoon on the cloth. Across the room, an old man has stopped talking.",
			"A crest on the handle. The same crest is on the brooch the old man is wearing.", "Found it, he says. On a windowsill. Inside a window.", "For his mates. There are three of them at the door, not looking at you.",
			1, "A tin for his own spoon. He watches it go into the box. He says nothing, loudly.", "Back across the cloth. He pockets it. He watches that as well.", "You call him over. He takes the spoon, and then, after a while, gives him a tin for bringing it back.",
			"who had his spoon brought back at the swap table", "He brought my spoon back because you made him, and I gave him a tin because you made me. We're both better for it, I suppose.");
		Swap("hoard", "A man with a sack wants every candle on the table. He'll pay in tins. He has the tins.",
			"His sack already clinks. He has been round the other tables.", "Winter's coming, he says. A man who has light has friends.", "For himself. And then for whoever wants light in January, at his price.",
			1, "Every candle for a fair heap of tins. The house eats tonight, and sits in the dark at Christmas.", "No. He calls you a fool, pleasantly, and tries the next table.", "Four candles, like anyone else. He takes them. He'll send his husband for four more.",
			"", "");
		Swap("proxy", "A boy with a list in someone else's handwriting. Lamp oil, salt, a needle. He has eggs.",
			"Six eggs, clean, still warm. The list is written very large, by a hand that shakes.", "It's my grandad's list, he says. He can't do the stairs now.", "For the grandad. The boy hasn't asked for a thing for himself, and keeps looking at the boiled sweets.",
			0, "Oil, salt and a needle for six eggs. And a sweet, which wasn't on the list.", "You say the old man must come himself. The boy folds the list very small.", "The list filled for four eggs. He carries two back up the stairs, carefully.",
			"who traded for his grandad at the swap table", "Grandad says whoever had the table that day can count. From him that's a blessing. I got a sweet. I've not told him about the sweet.");
		Swap("crew", "A man in a clean jacket wants medicine. Any. He pays in cartridges, laid in a row.",
			"Good cartridges. There's a red smear on the jacket cuff he has tried to wash out.", "For my camp, he says. We've a lad with a leg. That's all I'll say about my camp.", "For the lad with the leg. He means it. He's also counting the door and the windows.",
			2, "Dressings for cartridges. He nods, once, and the house has a debt owed by the Red Hand.", "Not to you. He gathers the cartridges one at a time. He remembers faces.", "One dressing, for the lad, and no cartridges taken. He doesn't know what to do with that. Good.",
			"who came to the Haven's table from the camp", "You gave me the dressing and wouldn't take the rounds. The lad kept the leg. The boss asked whose table it was. I said I didn't see.");
		Swap("cash", "A man in a good suit gone at the knees lays out banknotes, fanned. Thousands. A loaf.",
			"Real notes, crisp, a bank's band still round some. Worth exactly what paper is worth.", "It's all there, he says. You can count it. Please count it.", "For himself. He was somebody. He's waiting to be told he still is.",
			2, "A loaf for a fortune. He thanks you as if you'd done well out of it.", "You shake your head. He squares the notes up, carefully, and puts them away.", "A loaf for an afternoon's work on the house's sums. He was a bookkeeper. He nearly smiles.",
			"who offered banknotes for a loaf", "I keep the house's books now. It's twelve lines. I've never been so careful with twelve lines.");
		Swap("pearls", "A string of pearls, laid out straight. The man wants a place on the truck list for his son.",
			"Real, and old, and knotted between each one. The clasp has a jeweller's mark.", "They were my father's, he says. I'd rather he lived than they stayed in a drawer.", "For the son. He's fourteen, and outside, and doesn't know he's in here.",
			2, "The pearls go in the box. You can't sell him a place on a list. You've taken them anyway.", "You tell him the list isn't for sale. He nods as if he knew, and had to try.", "You send him to the clerk, with the pearls in his pocket. The list is by need. His name was already on it.",
			"who tried to buy his son a place with pearls", "His name was on the list already. You could have had the pearls and I'd never have known. I know now.");
		Swap("papers", "A thin man with a folder. Travel papers, stamped, three sets. A week's food for the three.",
			"The stamp is the right shape. The ink is this month's, and the date on it is last spring's.", "They'll get you past the line, he says. I've seen it done. He hasn't said he's seen it work.", "For whoever wants to believe the line can be crossed. He sells that, mostly. The papers come free.",
			1, "A week's food for three sheets of hope. Somebody will walk to the line with them.", "No. He shrugs. There's always another house.", "You call the lookout. The folder is taken, and burned in the stove, and he's walked to the gate.",
			"", "");
		Swap("tools", "A carpenter with a full roll of chisels. He wants them kept safe, not sold. Two tins a week.",
			"Beautiful tools, oiled, each in its pocket. His hands are bandaged, both of them.", "I can't hold them this month, he says. Next month. I'll want them back next month.", "For him. And for whoever needs a door hung after next month.",
			2, "The roll for two tins, outright. He lets go of it one finger at a time.", "You've no shelf for a pawnbroker's. He rolls them up again, slowly.", "Kept on the top shelf with his name on. Two tins now against a week's work when his hands mend.",
			"who left his chisels at the swap table", "My chisels were where you said they'd be, and nobody had used them. I hung your kitchen door for nothing. Don't argue.");
		Swap("boots", "Two boys with one pair of boots between them, wanting two pairs of anything.",
			"Good boots, a man's size, and wet inside. There's a name inked under the tongue.", "Our brother's, they say together. He doesn't need them.", "For themselves. They're both barefoot, on a stone floor, pretending not to mind.",
			2, "The boots for two pairs of clogs. They put them on at the table and clatter out.", "You ask where the brother is. They take the boots and go, fast.", "Two pairs of clogs, and the boots kept with the name on, for the book. They agree to that, after looking at each other.",
			"", "");
		Swap("salt", "A man with a sack of salt, coarse and grey. A third of it for flour, weight for weight.",
			"It's salt. And it's damp at the bottom, and the damp part is grit.", "From the pans at the coast, he says. He hasn't a mark on him from walking to any coast.", "For his house, two streets off, the one that keeps to itself.",
			1, "Flour for the sack, weight for weight. A quarter of what you've bought is sand.", "No trade. He hoists the sack and tries the next house.", "Flour for the top half only, weighed dry. He laughs, and takes it. You'll get on.",
			"", "");
		Swap("letter", "An old man with a sealed letter and a tin. The tin is for whoever carries the letter north.",
			"The address is a street in a town that burned in the first week.", "It's to my daughter, he says. Someone will be going that way.", "For her. He's been bringing it every market day. Everyone has always said: next time.",
			0, "You take the tin and the letter. It will sit in the box. He goes home lighter.", "You tell him what happened to that street. He hears it. He takes the letter back.", "You take the letter, and not the tin, and say it'll go when someone goes. He knows. He thanks you for it.",
			"who brings a letter to every market", "You took the letter and left me the tin. I know where the street is. I knew. Thank you for not making me say so.");
		Swap("knife", "A boy of sixteen with a kitchen knife. He wants to swap it for a bigger one.",
			"A paring knife, sharpened until there's hardly any blade. Tape on the handle, worn to his hand.", "For peeling, he says. He isn't looking at the potatoes.", "For himself. He walks to the well alone at dusk, and has done since his father.",
			2, "A bigger knife for his. He tests it on his thumb, and nods like a tradesman.", "No knives to children. He says he isn't one. He's right, and you've still said no.", "You keep both knives and send him to the lookout, who needs a pair of eyes at dusk and will teach him the rest.",
			"who wanted a bigger knife", "The lookout has me on the roof at dusk now. I don't walk to the well alone. I've still got the little knife. Just so you know.");
		int pool = RZ_DutyScript.Draw(4, picked);
		foreach (RZ_DutyBeat b : picked)
		{
			scene.m_aBeats.Insert(b);
		}

		return pool;
	}

	//------------------------------------------------------------------------------------------------
	//! Who a trader is, for the talk screen.
	protected static string SwapName(string key)
	{
		switch (key)
		{
			case "ring": return "Ivan Varga";
			case "watch": return "A small boy";
			case "coat": return "Pavel, in his shirt";
			case "aspirin": return "A neat man";
			case "stolen": return "A lad with a spoon";
			case "hoard": return "Bruno Hask";
			case "proxy": return "Milos, for his grandad";
			case "crew": return "A man in a clean jacket";
			case "cash": return "Mr Lindqvist";
			case "pearls": return "Augustin Roux";
			case "papers": return "A thin man with a folder";
			case "tools": return "Tomas the carpenter";
			case "boots": return "Two brothers";
			case "salt": return "A man with a sack";
			case "letter": return "Old Josef";
			case "knife": return "Nikola";
		}

		return "Someone at the table";
	}

	//------------------------------------------------------------------------------------------------
	//! One who comes to the table. `fair`: which call is right (0 take it, 1 refuse, 2 the third way).
	protected static void Swap(string key, string line, string handled, string asked, string forWhom, int fair, string took, string refused, string third, string who, string later)
	{
		RZ_DutyScript.G(key);
		RZ_DutyBeat b = RZ_DutyScript.T(key, line, RZ_DutyBeat.AT_HERE);
		b.m_iPicks = 2;
		// They stand at the table to be talked to (dialogue/duties/swap.rzd), and what they brought
		// lies on the cloth in front (RZ_DutyThings)
		b.m_sTalk = "swap:" + key;
		b.m_sTalkName = SwapName(key);
		b.m_sThing = RZ_DutyThings.For("swap:" + key);
		RZ_DutyScript.O("Turn it over in your hands", handled, "hold:2");
		RZ_DutyScript.O("Ask where it came from", asked, "hold:2");
		RZ_DutyScript.O("Ask who it's for", forWhom, "hold:2");
		array<string> code = {"wrong", "wrong", "wrong"};
		code[fair] = "right";
		string made = "";
		if (!who.IsEmpty() && fair == 2)
			made = " local";

		RZ_DutyScript.O("Take the trade as offered", took, "ends feed " + code[0]);
		RZ_DutyScript.O("Refuse it", refused, "ends " + code[1]);
		RZ_DutyScript.O("Find another way", third, "ends " + code[2] + made, who, later);
	}
}
