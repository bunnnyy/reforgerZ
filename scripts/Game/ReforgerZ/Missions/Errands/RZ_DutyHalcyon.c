//! Halcyon's own duties (docs/ERRANDS.md, "Halcyon's duties"): the sample round, the decon line,
//! the incinerator, the observation ward and the cold chain run, as beats for RZ_DutyScenes. All
//! five are worked out and led: readings, faults, bags, signs and a plan are read in person, and
//! the call is yours. Server only.
class RZ_DutyHalcyon
{
	// ================================================================ The sample round
	//------------------------------------------------------------------------------------------------
	//! Four marked spots round the town, a swab at each, and the strips say where it's coming from.
	static void Samples(RZ_GameModeComponent gm, RZ_Errand e, RZ_DutyScene scene)
	{
		array<string> spots = {"the pump handle", "the market trough", "the church hall door", "the mill race"};
		array<string> swab = {"Swab the pump handle", "Swab the market trough", "Swab the church hall door", "Swab the mill race"};
		int source = Math.RandomInt(0, 4);
		RZ_DutyScript.Into(scene.m_aBeats);
		RZ_DutyBeat round = RZ_DutyScript.T("round", "Four marked spots. The strip in each tube turns by how much is there: white, pink, red. Take all four before you say anything.", RZ_DutyBeat.AT_OUT);
		round.m_iPicks = 4;
		for (int i = 0; i < 4; i++)
		{
			string strip = "The strip stays white.";
			int away = Math.AbsInt(i - source);
			if (away == 0)
				strip = "The strip goes red before you've capped the tube.";
			else if (away == 1)
				strip = "The strip goes pink, slowly.";

			RZ_DutyScript.O(swab[i], strip, "out:14 hold:4");
		}

		RZ_DutyScript.T("source", "Where is it coming from? One mark on the station's map.");
		for (int s = 0; s < 4; s++)
		{
			if (s == source)
				RZ_DutyScript.O(string.Format("Mark %1", spots[s]), "Marked, in red pencil. The station will close it by morning.", "write right report infect- flag:found", "", "", string.Format("You were right about %1. It's been closed off, and the readings downstream are falling.", spots[s]));
			else
				RZ_DutyScript.O(string.Format("Mark %1", spots[s]), "Marked. The station will close it by morning, and wonder why nothing changes.", "write wrong", "", "", string.Format("We closed %1 on your mark. The readings haven't moved. It wasn't there.", spots[s]));
		}

		int seen = Math.RandomInt(0, 3);
		if (seen == 0)
		{
			RZ_DutyScript.T("child", "A child is at the spot you've just marked, with a cup.", RZ_DutyBeat.AT_SAME, "found");
			RZ_DutyScript.O("Take the cup out of her hand", "You take it. She howls. Her mother comes out to see who's laid hands on her, and you have to explain, in a suit.", "hold:3 local", "whose daughter was at the marked water", "A man in a white suit took the cup off my girl and I nearly brained him. Then he told me why. I've told the street.");
			RZ_DutyScript.O("Chalk a cross on it and go", "A cross, in chalk. She can't read a cross. She drinks.", "write hold:3 infect+");
		}
		else if (seen == 1)
		{
			RZ_DutyScript.T("window", "A face at an upstairs window, watching the suit. A man. He opens it and asks, quietly, what you've found.", RZ_DutyBeat.AT_SAME);
			RZ_DutyScript.O("Tell him what the strips say", "You tell him. He shuts the window. By dusk the street knows which water to leave alone.", "rumour local", "who asked from the window what the suit had found", "You told me what was in the water when you weren't meant to. Nobody on our street has gone down with it. I think about that.");
			RZ_DutyScript.O("Say it's routine", "Routine, you say. He says the last man in a suit said that, the week before they sealed the road.", "later", "", "The street heard there was a suit at the water and that he wouldn't say why. They've stopped drinking any of it. Three have gone down from thirst and bad cider instead.");
		}
		else
		{
			RZ_DutyScript.T("suit", "The suit has caught on the pump's ironwork. A tear at the elbow, the length of a finger. Skin under it.", RZ_DutyBeat.AT_SAME);
			RZ_DutyScript.O("Tape it and say so at the door", "Taped. You'll be an hour in the wash when you get back, and in the log.", "hold:4 report regard+");
			RZ_DutyScript.O("Tape it and say nothing", "Taped, and the tape rubbed dirty so it looks old.", "shady2");
		}
	}

	// ================================================================ The decon line
	//------------------------------------------------------------------------------------------------
	//! The faults: where each shows (0 the lamp over the suit, 1 the seals and the visor, 2 the
	//! case and the log), what's seen there, and the right call (1 run them through again, 2 the
	//! room, 3 report it).
	protected static void Faults(notnull array<int> station, notnull array<string> sign, notnull array<int> call)
	{
		array<string> all = {
			"0|1|A splash on the left boot, bright under the lamp, the size of a coin.",
			"0|1|Spray up the back of both legs. They walked through something and didn't look down.",
			"0|1|A handprint in the dust on the shoulder, glowing. Somebody touched them. Or something.",
			"0|1|The outer glove is torn across the palm. The inner one is whole.",
			"0|2|A tear in the suit at the knee, and the skin under it is scraped.",
			"0|2|The lamp shows through at the armpit: a seam has let go, a hand's width.",
			"0|1|Mud to the thigh, and the lamp can't see what's under the mud.",
			"0|2|The hood has been off and put back. There's hair caught in the neck seal, and it glows.",
			"1|1|The filter's hours card is two days past, and signed off in the wearer's own hand.",
			"1|2|The visor is cracked, corner to corner. It holds when you press it. It whistles when they breathe.",
			"1|1|The wrist seal turns on the arm. It was never taped.",
			"1|2|The neck seal comes away in your hand. It has been cut, and stuck back.",
			"1|1|The zip's storm flap is open its whole length.",
			"1|2|Behind the visor, sweat, and a flush climbing the throat. They're breathing through the mouth.",
			"1|2|The boot seal lifts. Inside the cuff, a strip of bandage, wet through, dark.",
			"2|3|The sample case is a tube short of its manifest. The foam has been pressed flat where it sat.",
			"2|1|The case is leaking at the hinge. Clear, and it smells of the fixative. Only the fixative.",
			"2|3|The log has the team at the well for an hour. Their boots have church lime on them.",
			"2|3|Under the manifest, a letter. Sealed, addressed to a house in the town, in the wearer's hand.",
			"2|3|Two vials in the case have no numbers. They aren't on any manifest. They're cold, and full.",
			"2|2|The log's last line is signed by someone who isn't on this team. Their suit is on the wearer.",
			"2|3|In the case, wrapped in a swab cloth, a kitten. Asleep. It has been in the hot town all day.",
			"2|1|The case's seal number doesn't match the log. One digit, transposed.",
			"0|2|A bite mark in the suit's forearm, pressed in but not through. The lamp shows what was on the teeth.",
			"1|3|The suit is somebody else's. The name on the collar has been inked over, and it's a size too big."};
		foreach (string f : all)
		{
			array<string> parts = {};
			f.Split("|", parts, false);
			station.Insert(parts[0].ToInt());
			call.Insert(parts[1].ToInt());
			sign.Insert(parts[2]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A shift on the decon line: five arrivals from fifteen, five faults from twenty-five shared out
	//! among them (one may carry two, one none). Two of the three stations each, then the call.
	static void Decon(RZ_GameModeComponent gm, RZ_Errand e, RZ_DutyScene scene)
	{
		array<string> who = {
			"A sampling team of three from a hot town, silent, the last one helping the middle one walk.",
			"A body-bag detail, two of them, grey in the face, wanting only to be let through to sit down.",
			"A Bastion officer in a borrowed suit, who won't hand over his sidearm and says so twice.",
			"A recruit on his first day out, who has put his own tape on, a great deal of it, everywhere.",
			"A girl of six in a suit cut down to fit, carried in by a nurse. She thinks it's a game. She's very good at it.",
			"The director. Alone. In a hurry. He holds his arms out for the lamp without being asked.",
			"The night technician, back from the well with a case, humming, an hour later than her log.",
			"A driver who never left his cab, he says, and so doesn't need any of this.",
			"Two porters with a trolley of clean linen, which isn't clean any more, and they know it.",
			"An old doctor who wrote this protocol, and will tell you which parts of it you're doing wrong.",
			"A man in a suit you don't recognise, with a pass from the lab, and no team.",
			"A sampling team of two, laughing, giddy, who can't stand still for the lamp.",
			"A stretcher, and the man on it is one of ours. His suit is intact. He isn't conscious.",
			"A cook from the canteen, who went out to the wire to buy eggs, in a suit, because rules are rules.",
			"A technician carrying her own visor in her hand, because it fogged, and she couldn't see the road."};
		array<int> station = {};
		array<string> sign = {};
		array<int> call = {};
		Faults(station, sign, call);
		// Five faults, shared among five arrivals
		array<int> order = {};
		for (int i = 0; i < sign.Count(); i++)
		{
			order.Insert(i);
		}

		array<ref array<int>> carried = {};
		array<int> arrivals = {};
		array<int> people = {};
		for (int p = 0; p < who.Count(); p++)
		{
			people.Insert(p);
		}

		for (int a = 0; a < 5; a++)
		{
			int pick = Math.RandomInt(0, people.Count());
			arrivals.Insert(people[pick]);
			people.Remove(pick);
			carried.Insert(new array<int>());
		}

		for (int f = 0; f < 5; f++)
		{
			int which = Math.RandomInt(0, order.Count());
			int on = Math.RandomInt(0, 5);
			if (carried[on].Count() >= 2)
				on = (on + 1) % 5;

			carried[on].Insert(order[which]);
			order.Remove(which);
		}

		RZ_DutyScript.Into(scene.m_aBeats);
		RZ_DutyScript.T("line", "", RZ_DutyBeat.AT_GATE);
		RZ_DutyScript.O("Open the decon line", "Spray, brush, rinse, the lamp, the seals, the log. Five are coming through. The tank on the roof holds four rinses and the drum six sprays: run anyone through twice and it's sooner. Two of the three checks on each is all the clock allows.", "hold:4 fill:water=4 fill:drum=6");
		foreach (int n, int person : arrivals)
		{
			array<string> shows = {"Nothing shows under the lamp.", "Every seal holds, and the visor is whole.", "The case matches its manifest, and the log reads true."};
			int right = 0;
			foreach (int fault : carried[n])
			{
				shows[station[fault]] = sign[fault];
				if (call[fault] > right)
					right = call[fault];
			}

			RZ_DutyBeat b = RZ_DutyScript.T("arrival" + n.ToString(), who[person], RZ_DutyBeat.AT_SAME);
			b.m_iPicks = 2;
			RZ_DutyScript.O("Go over the suit with the lamp", shows[0], "hold:3");
			RZ_DutyScript.O("Tug the seals and the visor", shows[1], "hold:3");
			RZ_DutyScript.O("Open the case and the log", shows[2], "hold:3");
			array<string> code = {"ends wrong infect+", "ends wrong", "ends wrong morale-", "ends wrong morale-"};
			if (right != 0)
				code[0] = "ends wrong infect+ infamy1";

			code[right] = "ends right";
			if (right == 0)
				code[0] = "ends right";

			if (right == 3)
				code[3] = "ends right report";

			RZ_DutyScript.O("Pass them through", "Through the inner door. The green lamp, and the bolt.", code[0] + " use:water use:drum", "", "", "");
			RZ_DutyScript.O("Run them through again", "Spray, brush, rinse, again, from the top. Twice the water, twice the drum, and ten minutes of their patience.", code[1] + " use:water use:water use:drum use:drum");
			RZ_DutyScript.O("Send them to the room", "The side door, and the room with the window in it. They look at you through the glass.", code[2] + " use:water");
			RZ_DutyScript.O("Report it to the director", "Written, signed, and sent up. Whatever it is, it isn't yours to settle at a door.", code[3]);
			// The tank and the drum run short by what's been used, not on a clock
			if (n < 4)
			{
				RZ_DutyScript.T("drum" + n.ToString(), "The spray coughs, and spits, and stops. The drum is empty.", RZ_DutyBeat.AT_SAME, "drumout");
				RZ_DutyScript.O("Mix a new drum from the store", "Powder, water, the paddle, and the smell that gets into your teeth. Full again.", "fill hold:8 fill:drum=6 +1");
				RZ_DutyScript.O("Brush only, no spray", "Brush and rinse, and no spray. It'll have to be enough.", "infect+ fill:drum=99");
				RZ_DutyScript.T("water" + n.ToString(), "The rinse runs brown, then stops. The tank on the roof is empty.", RZ_DutyBeat.AT_SAME, "waterout");
				RZ_DutyScript.O("Carry water up from the yard tank", "Six buckets, up the ladder. The rinse runs clear again.", "fill hold:10 fill:water=4 +1 regard+");
				RZ_DutyScript.O("Run the rest without the rinse", "Spray and brush, and no rinse. What the brush loosens stays on.", "infect+ later fill:water=99", "", "Two came through your line without a rinse. One of them carried it into the canteen on a sleeve. We caught it. That time.");
			}
		}

		RZ_DutyScript.T("close", "");
		RZ_DutyScript.O("Close the line and sign the log", "The lamp off, the drum capped, five lines in the log and your name under them.", "write hold:4");
	}

	// ================================================================ The incinerator
	//------------------------------------------------------------------------------------------------
	//! One bag. `right`: 0 burn it, 1 set it aside and report, 2 open it.
	protected static void Bag(string key, string line, string label, string weight, string feel, int right, string burned, string aside, string opened)
	{
		RZ_DutyScript.G(key);
		RZ_DutyScript.Case(key, line, RZ_DutyBeat.AT_GATE, 2,
			RZ_DutyScript.A("Read the label against the log", "Lift it for the weight", "Feel through the bag"),
			RZ_DutyScript.A(label, weight, feel),
			RZ_DutyScript.A("Put it in the fire", "Set it aside and report it", "Cut it open"),
			RZ_DutyScript.A(burned, aside, opened),
			RZ_DutyScript.A("burn", "report", "flag:open"), right);
	}

	//------------------------------------------------------------------------------------------------
	//! Four sealed bags to the burn pit, from twelve. The label, the weight and the feel don't
	//! always agree. Then the log against the count, and someone with one more.
	static int Incinerator(RZ_Errand e, RZ_DutyScene scene)
	{
		array<ref RZ_DutyBeat> picked = {};
		RZ_DutyScript.Pool();
		Bag("waste", "A yellow bag, tied twice.", "Clinical waste, ward two. In the log, same number.", "Light. Dressings.", "Soft all through.", 0, "It goes up yellow, then black.", "Set aside. The director asks why ward two's dressings are on his desk.", "Dressings, used. You've a faceful of the smell, and nothing else.");
		Bag("suits", "A grey bag, bulky, tied with tape.", "Suits, condemned, six, and a line in the log for them.", "Light for its size.", "Folded rubber, and the hard ring of a visor.", 0, "Rubber smoke, thick. Stand upwind.", "Set aside. Six condemned suits wait in a corridor for someone to wonder about them.", "Six suits. One has a name inked in the collar. Nothing more.");
		Bag("body", "A long bag, on the low trolley.", "Clinical waste, it says. The log has no line for this number.", "A man's weight. You need both arms.", "A shoulder. A hand. A wristband, under the plastic.", 1, "It burns for a long time. Whoever it was is nobody now, and nobody's question.", "Set aside, and reported. Someone died in this building and wasn't written down. The director goes very quiet.", "A man of forty, in a ward gown. A trial band on the wrist, and the number cut off it.");
		Bag("files", "A flat bag, square at the corners.", "Paper waste, the director's office. Logged this morning, in a hurry.", "Heavy, for paper. Dense.", "Folders. A great many. Metal clips.", 2, "It burns like paper. Whatever it said, it has stopped saying it.", "Set aside. The director's secretary comes for it herself, within the hour, and thanks you too carefully.", "Trial files. Names, doses, outcomes. Half the outcomes are one word.");
		Bag("live", "A small bag, cold to the touch.", "Expired cultures, it says. The log line has a question mark beside it.", "Heavy for its size, and it clinks.", "A rack of tubes. Cold. One is warm.", 1, "Glass cracks in the fire. A week of somebody's work, and one tube that wasn't dead.", "Set aside, and back to the cold room at a run. Somebody labelled a live rack for burning. Somebody will be asked.", "A rack of live cultures, sweating in the air. You've had them open in the yard.");
		Bag("moving", "A bag on the trolley's bottom shelf. It was at the other end a minute ago.", "Animal house, deceased, two, and the log agrees.", "It shifts against your arms.", "Fur. A leg. The leg pulls back.", 1, "It screams in the fire. Not for long. You'll hear it tonight.", "Set aside, at arm's length, and the animal house sent for. One of the two wasn't deceased. It's not in a mood to be handled.", "It's out before the knife is clear of the plastic. Small, fast, and through the fence toward the town.");
		Bag("stripped", "A bag that rattles.", "Sharps, it says. Logged, initialled by a porter.", "Heavier than needles.", "Rings. Watch cases. A set of teeth, on a plate.", 1, "Gold doesn't burn. It'll be in the ash for whoever rakes it, and he knows that.", "Set aside. Somebody is taking things off the dead, and sending the proof to the fire. The porter's initials are on it.", "A pound of wedding rings. You stand in the yard holding them.");
		Bag("ammo", "A canvas bag, not one of ours, taped shut.", "No label. A chalk mark. Not in the log.", "Very heavy. It sags.", "Small hard cylinders, hundreds, loose.", 1, "The first one goes off as you turn away. Then the rest. The pit wall is pocked, and you are lucky.", "Set aside, well away from the pit. Rifle rounds, hundreds. Somebody's guard has been disarming himself into the waste.", "Rounds. Hundreds. You nearly put this in a fire.");
		Bag("wet", "A bag that drips.", "Linen, soiled, ward three, and in the log.", "Heavy. Sodden.", "Cloth, wrung-wet, cold.", 0, "It hisses, steams, and puts half the fire out. It'll want fuel, and a second go.", "Set aside to dry. It's linen. The ward wonders why it's on a rail in the yard.", "Wet sheets. What's on them is why they're here.");
		Bag("pills", "A bag that rattles like a child's toy.", "Expired stock, pharmacy. Logged, a round number.", "Light, and loose.", "Bottles. Glass, and big ones, plastic. Sealed. Every one sealed.", 1, "They melt and flare. A month of the ward's sleep, gone, and none of it was out of date.", "Set aside. Sealed, in-date morphine and sleeping pills, written off as expired by somebody who meant to collect them from the ash pit.", "Sealed bottles, hundreds. In date. Somebody wrote these off on purpose.");
		Bag("uniform", "A kit bag, army, laced shut.", "Personal effects, unclaimed. Logged with no name.", "A man's kit, by the weight.", "Boots. A mess tin. Paper. A small hard square, like a card.", 2, "Boots take a long time. So does a man's name.", "Set aside. Nobody upstairs wants unclaimed effects on a desk. It comes back to you tomorrow.", "A soldier's kit. A photograph, a letter half written, and his card, with a number on it that somebody is looking for.");
		Bag("toys", "A small bag, tied with a ribbon somebody found.", "Ward four, contaminated effects. Logged, in a nurse's round hand.", "Almost nothing.", "A bear. A book with thick pages. One shoe.", 0, "It's gone very quickly. That was all there was of it.", "Set aside. The nurse who logged it sees it on the shelf, and has to sit down.", "A bear, a book, a shoe. You tie the ribbon again, badly.");
		int pool = RZ_DutyScript.Draw(4, picked);
		foreach (RZ_DutyBeat b : picked)
		{
			scene.m_aBeats.Insert(b);
		}

		RZ_DutyScript.Into(scene.m_aBeats);
		RZ_DutyScript.T("log", "The log says five bags today. You've had four. A porter is standing at the gate with a fifth, unlabelled, and a look. One more, he says. Off the book. It's nothing.", RZ_DutyBeat.AT_GATE);
		RZ_DutyScript.O("Burn it, off the book", "It goes in. He watches it catch, and then he leaves, without the look.", "burn shady2 later", "", "You burned one for me off the book. It was what I said it was. Mostly. If you ever need a door left unlocked, you know the porter.");
		RZ_DutyScript.O("Not without a line in the log", "Not without a line. He takes it away again. The log stays one short, and you write that down too.", "report regard+", "", "", "The bag you wouldn't burn off the book was opened upstairs. I can't tell you what was in it. I can tell you that you were right.");
		RZ_DutyScript.O("Open it first", "He lets you. Letters. Hundreds, from the wards, never posted. He was told to lose them.", "flag:letters");
		RZ_DutyScript.T("letters", "Every letter the wards have written since the road closed, in one bag. None of them went anywhere.", RZ_DutyBeat.AT_SAME, "letters");
		RZ_DutyScript.O("Burn them, as he was told", "They burn like any paper. The wards go on writing.", "burn");
		RZ_DutyScript.O("Take them to the Haven's board", "Into your coat, a bundle at a time. They'll be pinned up where somebody might know a name.", "shady2 rumour local", "who is a porter at the lab", "The letters from the wards are on the Haven's board. People are answering them. Nobody here knows how they got out. I know. I carried the bag, and you carried the rest.");
		return pool;
	}

	// ================================================================ The observation ward
	//------------------------------------------------------------------------------------------------
	//! Three beds, from eight conditions. Six signs at each, taken by hand, and time for four. Then
	//! what it is, then what's to be done. A turning missed is a turned subject on the ward.
	static void Ward(RZ_Errand e, RZ_DutyScene scene)
	{
		RZ_Rng rng = RZ_DutyPosts.Seed(e, 31);
		array<string> name = {"A plain fever", "The infection, early", "The infection, turning", "The dose is doing this", "Nothing wrong, and wants out", "Nothing wrong, and wants the dose", "Fear, and nothing else", "Hunger, and nothing else"};
		array<string> temp = {"38.9, and steady since morning.", "37.8, and it was 37.2 at noon.", "35.1. It was 36 an hour ago.", "39.6, since the injection.", "36.9.", "37.0. The glass is warm at the wrong end.", "37.2.", "35.9."};
		array<string> pulse = {"Fast, and regular as a clock.", "Fast, and it skips.", "Slow. You count twice to be sure.", "Racing. You lose count.", "Ordinary.", "Ordinary, once they forget you're counting.", "Racing, and it slows while you talk.", "Slow and steady."};
		array<string> eyes = {"Clear. They follow the torch, and complain about it.", "Slow to the torch. The left slower than the right.", "The torch does nothing. Both wide, and fixed on you.", "Pinpoints, clear, and they won't keep still.", "Clear.", "Clear. Watching your face, not the torch.", "Clear, and darting to the next bed.", "Clear, sunk in."};
		array<string> skin = {"No bite. A graze on the shin, clean.", "A bite on the forearm. The edge is pink, and hot to the back of your hand.", "The bite's edge has gone grey, and there are lines running up from it.", "A bite, a week old, healed white at the edge.", "No bite anywhere. You look twice.", "A scratch, opened again. There's skin under their own nail.", "A bite, three days. The edge is calm.", "No bite. Every rib."};
		array<string> speech = {"Clear, and cross at being woken.", "Clear, and then hunting for an ordinary word.", "Sounds. Not words. One sound, over.", "Fast, clear, and can't stop. They know it and can't stop.", "Fine, they say. Fine. Before you've asked.", "It burns, they say. Then: does it look bad. Then: bad enough.", "A whisper. Did the others turn, all of them, every one.", "Clear, slow, and very polite."};
		array<string> chart = {"Came in wet from the river two days ago. No bite recorded.", "Bitten three days ago. Nothing, until today.", "Bitten five days ago. Restless since last night. Pulled the drip out twice.", "Trial dose given yesterday at four.", "Admitted on suspicion. Has asked twice today when the gate opens.", "Asked the night nurse what the dose costs. Asked again.", "The next bed turned in the night. Hasn't slept since.", "Walked in from the hills. Hasn't eaten in six days."};
		array<string> care = {"Fluids, and cool them down", "Isolation, and call the doctor", "Restraints, now", "Call the doctor", "Tell them the truth", "Give them a meal"};
		array<int> best = {0, 1, 2, 3, 4, 4, 4, 5};
		array<string> done = {"Water, a wet cloth, the window open. By evening the glass reads lower.", "Wheeled to the end room, and the doctor sent for. The door has a window in it.", "Straps at the wrists and ankles, and the chest. They don't fight it, which is harder to watch.", "The doctor comes, reads the chart, and changes one number on it.", "You tell them what's so. They hear it. It's the first true thing anyone's said to them in here.", "A bowl, and then another. They eat as if it might be taken away."};
		array<int> beds = {};
		while (beds.Count() < 3)
		{
			int c = rng.Next() % 8;
			if (!beds.Contains(c))
				beds.Insert(c);
		}

		RZ_DutyScript.Into(scene.m_aBeats);
		RZ_DutyScript.T("ward", "", RZ_DutyBeat.AT_HERE);
		RZ_DutyScript.O("Take the ward round", "Three beds are yours. At each, six things you can take by hand, and time for four. Then say what it is, and then what's to be done.", "hold:3");
		foreach (int n, int cond : beds)
		{
			RZ_DutyBeat b = RZ_DutyScript.T("bed" + n.ToString(), string.Format("The %1 bed. They watch you come.", Ord(n)), RZ_DutyBeat.AT_NEAR);
			b.m_iPicks = 4;
			RZ_DutyScript.O("Take the temperature", temp[cond], "care hold:3");
			RZ_DutyScript.O("Take the pulse", pulse[cond], "care hold:3");
			RZ_DutyScript.O("Shine the torch in their eyes", eyes[cond], "care hold:2");
			RZ_DutyScript.O("Look at the skin, and any bite", skin[cond], "care hold:3");
			RZ_DutyScript.O("Ask them how they are", speech[cond], "care hold:2");
			RZ_DutyScript.O("Read the chart at the bed's foot", chart[cond], "hold:2");
			// What it is: the true one, and the two most like it
			int otherA = (cond + 1) % 8;
			int otherB = (cond + 5) % 8;
			if (cond == 1)
				otherB = 6;
			else if (cond == 2)
				otherB = 3;

			RZ_DutyScript.T("what" + n.ToString(), "What is it?", RZ_DutyBeat.AT_SAME);
			array<int> shown = {cond, otherA, otherB};
			int shift = rng.Next() % 3;
			for (int k = 0; k < 3; k++)
			{
				int d = shown[(k + shift) % 3];
				if (d == cond)
					RZ_DutyScript.O(name[d], "Written on the chart, in ink.", "write right");
				else if (cond == 2)
					RZ_DutyScript.O(name[d], "Written on the chart, in ink. The bed creaks behind you as you write.", "write wrong flag:missed");
				else
					RZ_DutyScript.O(name[d], "Written on the chart, in ink.", "write wrong");
			}

			RZ_DutyScript.T("care" + n.ToString(), "And what's to be done?", RZ_DutyBeat.AT_SAME);
			int good = best[cond];
			array<int> offered = {good, (good + 2) % 6, (good + 4) % 6};
			for (int m = 0; m < 3; m++)
			{
				int t = offered[(m + shift) % 3];
				if (t == good)
					RZ_DutyScript.O(care[t], done[t], "care right regard+");
				else if (cond == 2)
					RZ_DutyScript.O(care[t], done[t], "care wrong flag:missed");
				else
					RZ_DutyScript.O(care[t], done[t], "care wrong");
			}

			RZ_DutyScript.T("turned" + n.ToString(), "The bed behind you. A strap you didn't fasten. They are sitting up, and they aren't looking at you as a person looks.", RZ_DutyBeat.AT_SAME, "missed");
			RZ_DutyScript.O("Shout for the orderlies", "Three of them, with a blanket and a pole. It takes all three. The ward watches it happen.", "hold:3 morale- infect+", "", "", "A subject turned on the ward on your round. You'd written something else on the chart. Two orderlies are being stitched.");
		}

		RZ_DutyScript.T("chart", "The researcher, at the ward door, with the day's sheet. The trial needs the fevers written as responses, she says. It's only a word.", RZ_DutyBeat.AT_HERE);
		RZ_DutyScript.O("Write the charts as they are", "As they are. She reads them, and her mouth goes thin, and she takes them.", "write record regard+ research", "", "", "You wrote the ward's charts true. The trial's numbers are worse for it, and right for it. That's worth more than a good week.");
		RZ_DutyScript.O("Write them her way", "Her way. Three fevers become three responses. The trial has a good day, on paper.", "write shady2 +1");
	}

	//------------------------------------------------------------------------------------------------
	static string Ord(int i)
	{
		array<string> words = {"first", "second", "third"};
		return words[Math.ClampInt(i, 0, 2)];
	}

	// ================================================================ The cold chain run
	//------------------------------------------------------------------------------------------------
	//! A box from the station to the far point, with its own rule, and two things on the way that
	//! the rule decides. One of six cargoes, two of eight happenings, and the hand-over.
	static void ColdChain(RZ_Errand e, RZ_DutyScene scene)
	{
		int cargo = Math.RandomInt(0, 6);
		array<string> what = {"trial vials, twelve, in ice", "blood samples, in a rack", "a culture in a flask, half full", "doses, four, sealed", "sealed results, in a tin", "the heavy cold box"};
		array<string> rule = {"They're no good warm: don't stop for anything.", "They must stay in the dark: the lid stays shut.", "It must stay level: no climbing, no wading.", "They must arrive counted: four went out, four go in.", "The seal must be whole: nobody opens it, not even you.", "It's heavy, and it mustn't be put down on wet ground."};
		// What each cargo can bear: 0 a wait, 1 being opened, 2 rough ground
		array<bool> wait = {false, true, true, true, true, true};
		array<bool> open = {true, false, true, true, false, true};
		array<bool> rough = {true, true, false, true, true, false};
		RZ_DutyScript.Into(scene.m_aBeats);
		RZ_DutyScript.T("take", "", RZ_DutyBeat.AT_HERE);
		// The rules felt on the way: what can't wait warms after six minutes, what can't bear rough
		// ground rings at a sprint, a jump or a fall
		int carry = 0;
		if (!wait[cargo])
			carry += 1;

		if (!rough[cargo])
			carry += 2;

		RZ_DutyScript.O("Take the box from the cold room", string.Format("In your hands: %1. %2 You're carrying it alone, and what happens on the way is yours to decide.", what[cargo], rule[cargo]), "hold:4 carry:" + carry.ToString());
		RZ_DutyScript.Pool();
		RZ_DutyScript.G("checkpoint");
		RZ_DutyScript.T("checkpoint", "A Bastion corporal steps into the road. Open it, he says. Orders.", RZ_DutyBeat.AT_OUT);
		RZ_DutyScript.O("Open it for him", "The lid comes up. He looks, sees glass, and waves you on. The cold comes out. So does the dark.", Code(open[cargo]));
		RZ_DutyScript.O("Show him the lab's seal instead", "He reads the seal, slowly, moving his lips. Then he steps back. It's cost you four minutes.", Code(wait[cargo]));
		RZ_DutyScript.O("Go round by the field", "Over the gate and across the plough, with the box against your chest.", Code(rough[cargo]));
		RZ_DutyScript.G("toll");
		RZ_DutyScript.T("toll", "A bar across the road, and three lads who'd like the box. Whatever's in it must be worth something, since you're carrying it like that.", RZ_DutyBeat.AT_OUT);
		RZ_DutyScript.O("Open it, and let them see it's glass", "They look. Tubes. One of them spits. They lift the bar.", Code(open[cargo]));
		RZ_DutyScript.O("Stand and talk them down", "You tell them what it is and who it's for. It takes a while. The oldest one has a sister on a ward. The bar goes up.", Code(wait[cargo]));
		RZ_DutyScript.O("Leave the road, through the wood", "Down the bank, through bramble, over a stream on stones.", Code(rough[cargo]));
		RZ_DutyScript.G("begs");
		RZ_DutyScript.T("begs", "A woman, in the road, on her knees. She knows what the lab carries. One. For her boy. He's bitten, two days.", RZ_DutyBeat.AT_OUT);
		if (cargo == 3)
		{
			RZ_DutyScript.O("Give her one of the four", "You break the seal and put one in her hands. She can't speak. Three go on to the lab.", "wrong local", "whose boy was bitten, on the lab road", "You gave me one of the four. He's alive. I know what it cost you at the other end. I'd do the same to you, and I'm not sorry.");
			RZ_DutyScript.O("Tell her to bring him to the lab", "You tell her the door, and the name to ask for, and that there's a list. It's true. It isn't enough. She goes to fetch him.", "right local", "whose boy was bitten, on the lab road", "You wouldn't give me one, and you told me the door. He's on the list. He's third. I haven't decided about you yet.");
		}
		else
		{
			RZ_DutyScript.O("Tell her it isn't the dose", "You open your mouth to explain what's in the box. She's already seen your face. She gets up, and walks off the road.", Code(wait[cargo]));
			RZ_DutyScript.O("Tell her to bring him to the lab", "You tell her the door, and the name to ask for. She goes to fetch him, at a run.", "right local", "whose boy was bitten, on the lab road", "You told me which door. He's on the list. I didn't know there was a list.");
		}

		RZ_DutyScript.O("Walk round her", "You step off the road, round her, and on. She doesn't call after you.", "right later", "", "A woman has been at the lab gate every day since you carried the box past her. She doesn't ask for anything. She looks at each of us as we come out.");
		RZ_DutyScript.G("pack");
		RZ_DutyScript.T("pack", "The cold pack has split. Water is running out of the box's corner, and the glass inside is sweating.", RZ_DutyBeat.AT_OUT);
		RZ_DutyScript.O("Stand it in the stream to its lid", "Into the stream, to the lid, wedged with stones. A quarter hour, and it's cold through again.", Code(wait[cargo] && rough[cargo]));
		RZ_DutyScript.O("Find a cellar in the nearest house", "A cellar, a stone shelf, a quarter hour in the dark with someone's potatoes.", Code(wait[cargo]));
		RZ_DutyScript.O("Run for it", "You run, the box held out in front of you, and everything in it ringing.", Code(rough[cargo]));
		RZ_DutyScript.G("fall");
		RZ_DutyScript.T("fall", "The bridge is out. A plank, a foot wide, over brown water. Or the ford, a quarter mile down, to the knee.", RZ_DutyBeat.AT_OUT);
		RZ_DutyScript.O("Cross on the plank", "Sideways, the box against your chest. The plank bends. It holds.", "right hold:6");
		RZ_DutyScript.O("Wade the ford", "In to the knee, the box above your head, and the current pulling at your legs.", Code(rough[cargo]));
		RZ_DutyScript.O("Go round by the mill", "Round by the mill, on a road. It's a mile, and twenty minutes.", Code(wait[cargo]));
		RZ_DutyScript.G("dog");
		RZ_DutyScript.T("dog", "Something is following you, in the hedge, keeping pace. It stops when you stop.", RZ_DutyBeat.AT_OUT);
		RZ_DutyScript.O("Put the box down and face it", "You set the box on the verge and turn. A dog, thin, hopeful. It sits. You've lost five minutes to a dog.", Code(wait[cargo] && cargo != 5));
		RZ_DutyScript.O("Keep walking, the same pace", "You don't change step. At the crossroads it's gone, whatever it was.", "right");
		RZ_DutyScript.O("Run", "You run. It runs. It's a dog, and it thinks this is wonderful.", Code(rough[cargo]));
		RZ_DutyScript.G("rain");
		RZ_DutyScript.T("rain", "Rain, all at once, like a tap. The road's a stream in a minute. There's a barn.", RZ_DutyBeat.AT_OUT);
		RZ_DutyScript.O("Shelter in the barn till it passes", "A quarter hour under a roof, the box on a dry beam.", Code(wait[cargo]));
		RZ_DutyScript.O("Walk on through it", "Head down, the box under your coat, the road running over your boots.", Code(cargo != 5));
		RZ_DutyScript.O("Cut across the high field", "Up over the field, on grass, out of the water and into the wind.", Code(rough[cargo]));
		RZ_DutyScript.G("stranger");
		RZ_DutyScript.T("stranger", "A man in a good coat falls into step. He'd like to know what's in the box. He'd pay to look. Only to look.", RZ_DutyBeat.AT_OUT);
		RZ_DutyScript.O("Let him look, for the price", "The lid, for a moment. He looks a long time for a moment. He pays, and thanks you, and writes something down.", "wrong shady2 give");
		RZ_DutyScript.O("Tell him to ask at the lab", "Ask at the lab, you say. He smiles. He already has. He drops back.", "right report", "", "", "The man who walked beside you has been asking at three stations. You're the only one who sent him to the front door. We know his face now.");
		RZ_DutyScript.O("Lose him across the fields", "Over a gate while he's lighting a pipe, and away across the plough.", Code(rough[cargo]));
		array<ref RZ_DutyBeat> picked = {};
		RZ_DutyScript.Draw(2, picked);
		foreach (RZ_DutyBeat b : picked)
		{
			scene.m_aBeats.Insert(b);
		}

		RZ_DutyScript.Into(scene.m_aBeats);
		int end = Math.RandomInt(0, 3);
		if (end == 0)
		{
			RZ_DutyScript.T("dark", "The far door. No lamp over it. The power's out here, and the cold room behind it is warming.", RZ_DutyBeat.AT_FAR);
			RZ_DutyScript.O("Hand it in anyway", "Into a cold room that isn't. The technician looks at the box, and at the dead dial on the wall.", "wrong");
			RZ_DutyScript.O("Pack it in the well bucket", "Down the well in the bucket, on a marked rope, and the technician told the mark. It'll keep till the power's back.", "right hold:6 +1 regard+");
			RZ_DutyScript.O("Carry it back", "All the way back, with everything the road had on it the first time.", Code(wait[cargo]));
		}
		else if (end == 1)
		{
			RZ_DutyScript.T("nobody", "The far door. Shut. A note on it in pencil: back in an hour.", RZ_DutyBeat.AT_FAR);
			RZ_DutyScript.O("Wait by the door", "An hour on a doorstep, the box in the shade of your own body.", Code(wait[cargo]));
			RZ_DutyScript.O("Leave it on the step with a note", "On the step, with your note under a stone. Anyone could lift it. Somebody might.", "wrong");
			RZ_DutyScript.O("Find the cellar hatch", "Round the back, a hatch, a stone floor, the right shelf. Your note goes on the door over theirs.", "right hold:6 +1");
		}
		else
		{
			RZ_DutyScript.T("short", "The far door, and the technician with the manifest. She counts. She counts again. One short, she says.", RZ_DutyBeat.AT_FAR);
			RZ_DutyScript.O("It was short when you took it", "You say so. She looks at you for a while. Then she turns the manifest over, and there's the cold room's own note: one held back.", "right report");
			RZ_DutyScript.O("Offer to walk back and look", "You walk the road again, looking at the verge, for something that was never in the box.", "wrong hold:8");
			RZ_DutyScript.O("Say nothing", "You say nothing. She writes one short, and your name beside it.", "wrong regard-");
		}

		RZ_DutyScript.T("sign", "");
		RZ_DutyScript.O("Sign the manifest", string.Format("Signed. %1, delivered.", RZ_Util.Capitalize(what[cargo])), "write hold:3 carry:0");
	}

	//------------------------------------------------------------------------------------------------
	//! A choice the cargo can bear is right; one it can't is wrong.
	protected static string Code(bool bears)
	{
		if (bears)
			return "right";

		return "wrong";
	}
}
