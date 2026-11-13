//! Bastion's own duties (docs/ERRANDS.md, "Bastion's duties"): gate duty, kit inspection, the field
//! telephone and the loudspeaker tape, as beats for RZ_DutyScenes (stag at the wire is the watch,
//! RZ_DutyPosts). Three are worked out and led: papers read for forgery, kits read for what's wrong
//! and where it went, a break in the wire read for who made it. Server only.
class RZ_DutyBastion
{
	// ================================================================ Gate duty (worked out)
	//------------------------------------------------------------------------------------------------
	//! One who comes to the barrier. `right`: 0 pass them, 1 turn them back, 2 hold them for the sergeant.
	protected static void Papers(string key, string line, string stamp, string dates, string business, int right, string passed, string turned, string held, string told = "")
	{
		RZ_DutyScript.G(key);
		array<string> codes = {"", "", "report"};
		if (right != 0)
			codes[0] = "infamy1";

		RZ_DutyBeat b = RZ_DutyScript.Case(key, line, RZ_DutyBeat.AT_GATE, 2,
			RZ_DutyScript.A("Hold the stamp to the light", "Read the dates and the name", "Ask their business"),
			RZ_DutyScript.A(stamp, dates, business),
			RZ_DutyScript.A("Lift the barrier", "Turn them back", "Hold them for the sergeant"),
			RZ_DutyScript.A(passed, turned, held), codes, right, "", "", told);
		// They stand at the barrier to be talked to (dialogue/duties/gate.rzd): the asking is option 2;
		// their papers lie on the post's table in front
		b.m_sTalk = "gate:" + key;
		b.m_sTalkName = "Someone at the barrier";
		b.m_sThing = RZ_DutyThings.For("gate:" + key);
	}

	//------------------------------------------------------------------------------------------------
	//! Four at the barrier, from fourteen. Two looks each, then the barrier is yours.
	static int Gate(RZ_Errand e, RZ_DutyScene scene)
	{
		array<ref RZ_DutyBeat> picked = {};
		RZ_DutyScript.Pool();
		Papers("farmer", "A farmer with a cart of turnips and a pass folded small from use.", "The stamp is worn, and the ink has bled into the fold. Old, and real.", "His name, last month's date, and this road written out in a clerk's hand.", "Turnips, for the post's own kitchen. He lifts the sack to show you.", 0,
			"The barrier goes up. He touches his cap, as he has every week since it was built.", "You turn him round. The post eats tinned tonight, and he has a cart of turnips going soft.", "You hold him. The sergeant comes, looks at the turnips, and looks at you.");
		Papers("clean", "A man on foot, well fed, with a pass so new it crackles.", "The stamp is the right crest. Under the light, the eagle has one wing longer than the other.", "Today's date. In the same ink as the stamp, and the same hand as the signature.", "Visiting his brother, he says. He doesn't know which company his brother is in.", 2,
			"Up goes the barrier. He walks into the post with a forged pass and a good look round.", "You send him back down the road. He goes easily. He'll try the east gate.", "Held. In his boot the sergeant finds a sketch of the wire, with the gaps marked.", "That pass you caught was drawn by hand. He'd have walked the whole wire. Well held.");
		Papers("nurse", "A woman with a nurse's bag and a pass a week out of date.", "The stamp is true. It was true when it was given.", "Expired Thursday. The name matches the name stitched on the bag.", "The field hospital sent for her on Wednesday. There's a boy with a leg, she says, and no time.", 0,
			"You lift it on your own word. She's running before it's fully up. The boy keeps the leg.", "Out of date is out of date. She stands at the barrier, looking past you at the hospital tent.", "You hold her for the sergeant, who is at supper. It takes forty minutes. It was the wrong forty.", "You let the nurse through on an expired pass. The surgeon wants to shake your hand. The adjutant doesn't.");
		Papers("soldier", "A soldier in our kit, alone, coming in. Tags, and a movement order.", "A real order form. The unit stamp is a unit that was wiped out at the bridge.", "Dated after the bridge. Signed by an officer who died on it.", "Rejoining, he says. He can't meet your eye, and his rifle is someone else's.", 2,
			"Up it goes. A man who walked away from the bridge walks back in, and nobody asks him anything.", "You turn one of our own away from our own wire. He sits down on the verge and doesn't move.", "Held. He tells the sergeant all of it, in one breath, as if he'd been holding it since the bridge.", "The man you held at the gate ran at the bridge. He's been given a shovel instead of a wall. That was your doing.");
		Papers("crew", "Two lads with a handcart of scrap and a trade pass, both grinning.", "The stamp is genuine. The pass is genuine. It was issued to a tinker, and they aren't one.", "A name neither of them answers to when you say it.", "Scrap for the motor pool, they say. Under the scrap, the cart rides heavy.", 1,
			"Barrier up. The motor pool gets no scrap. Somebody in the post gets a delivery.", "Not today, lads. They take it well. One of them tips an imaginary hat.", "Held, and the cart turned out. Under the scrap is nothing at all. They were only seeing who was on the gate.", "Those two with the cart were counting us. You turned them round before they'd counted much.");
		Papers("family", "A family of five with one pass between them, made out for one.", "The stamp is real, and so is the pass. One name on it.", "The father's name. The date is good.", "They walked from the coast. The children are asleep standing up.", 2,
			"You lift it for five on a pass for one. Five more mouths, and nobody has written them down.", "One may pass. He won't leave them. They all go back down the road, slowly.", "Held at the guard hut, with a blanket, till the sergeant can write four more names on a list.", "That family you held got papers by morning. All five. It took the sergeant ten minutes. Most would have turned them back.");
		Papers("officer", "A staff car, a driver, and a major you don't know, in a hurry.", "A proper warrant. The stamp is Command's, and Command hasn't answered a radio in weeks.", "Dated yesterday. From Command.", "Inspection, says the driver. The major says nothing, and looks at his watch.", 2,
			"You wave a major through, because he's a major. Nobody inside was expecting one.", "You refuse a major entry. He is astonished. He writes down your name, with a gold pen.", "You hold a major at the barrier and send for the sergeant. The major is furious. The sergeant is very interested.", "Your major had a warrant from a Command that doesn't answer. The captain is still talking to him. Nobody's told me what about.");
		Papers("priest", "An old priest with a bicycle and no papers at all. A letter from a dead bishop.", "No stamp. A wax seal, cracked across.", "No date. It commends the bearer to all Christian people.", "He's come to bury the men from the bridge. He has the names, written on his cuff.", 0,
			"You lift the barrier for a letter from a bishop. He blesses it on the way under.", "No papers, no entry. He nods, and begins to say the office at the barrier, loudly, in Latin.", "Held. The sergeant comes, crosses himself before he can stop, and walks the old man in.", "The priest you let in buried eleven of ours properly. The men haven't stopped talking about the gate that let him.");
		Papers("trader", "A trader with a mule, a good pass, and a second pass in his hat.", "Both stamps are real. One is ours. One is the Red Hand's toll mark.", "Both dated this week.", "He sells to whoever's buying. He says so. He seems to think that's a virtue.", 0,
			"Through he goes. He sells us lamp oil and tells the Red Hand how many lamps.", "Turned back. The post goes without lamp oil. He isn't troubled. There are other gates.", "Held, and searched, and found to be exactly what he said he was. He's owed an apology. He names a price for it.");
		Papers("girl", "A girl of about ten with a pass pinned to her coat like a label.", "A real stamp. A real pass. For an adult, male.", "Her father's name. Dated the day before the bridge.", "He told her to wait at the gate till he came out. That was eight days ago.", 2,
			"You lift the barrier and she walks in to look for a man who isn't there.", "You tell her to go home. She says this is where he said. She sits down again.", "You take her to the guard hut and send for the sergeant, who has a daughter, and a casualty list.", "That child at the gate. Her father's on the list from the bridge. The cook has her now. You did right not to send her off.");
		Papers("medic", "A man in a white coat with Halcyon's pass and a sealed steel case.", "Halcyon's stamp, and correct. The case has a seal with the same number.", "Good dates. The destination is our own medical tent.", "Samples, he says. He would rather you didn't open it. He would rather nobody did.", 0,
			"Through, case and all, unopened. It was what he said.", "Turned back. Our own medical officer had asked for him, and wants to know who sent him away.", "Held while the sergeant decides about the case. The samples warm up. The white coat watches the clock.");
		Papers("drunk", "One of ours, back from somewhere, cheerful, with his pass upside down.", "Our stamp. His name. It's even in date.", "A day pass. It ran out at dusk, and it's well past.", "He's been to see a girl at the Haven. He'd like you to know she's wonderful.", 0,
			"In you go, and straight to your bunk. He salutes the barrier.", "Turned back from his own post, for being late to it. He lies down on the verge to think.", "Held for the sergeant, who gives him a week of latrines. He bears you no ill will, and says so, at length.");
		Papers("list", "A thin woman with a forged pass, and she tells you it's forged before you ask.", "A potato stamp, and proud of it.", "A date, and a name. Both hers. Only the stamp is a lie.", "Her husband is in your stockade for selling boots. She wants to see him. She couldn't get papers, so she made some.", 2,
			"You pass a forgery because its owner was honest about it. She thanks you as if you'd lent her money.", "A forged pass is a forged pass. She folds it, and asks what time you come off.", "Held for the sergeant, forgery in hand. He reads it, and laughs, and gives her ten minutes at the stockade door.", "The woman with the potato stamp saw her husband for ten minutes. She's left the stamp with the sergeant. He's keeping it.");
		Papers("ident", "A man with a soldier's identity card and a civilian's shoes.", "The card is real. There's a dark stain along one edge, wiped but not gone.", "A name, a number, a photograph ten years younger than he is. Or than whoever it was.", "On leave, he says. From a unit he names a little too carefully.", 2,
			"A real card gets a real welcome. He's inside the wire wearing a dead man's name.", "You send him away. The card goes with him, and so does whatever happened to its owner.", "Held. The number on the card is on the missing list. He says he found it. He's asked where, for some time.", "That card you stopped was a missing man's. We know where to look for him now. That's a family that gets an answer.");
		int pool = RZ_DutyScript.Draw(4, picked);
		foreach (RZ_DutyBeat b : picked)
		{
			scene.m_aBeats.Insert(b);
		}

		return pool;
	}

	// ================================================================ Kit inspection (worked out, with a follow-on)
	//------------------------------------------------------------------------------------------------
	//! One man's kit. `shows`: 0 the count shows it, 1 the barrel, 2 the pack. `right`: 0 pass, 1
	//! write up, 2 deal with it quietly. `missing`: writing it up or dealing with it opens the search.
	protected static void Kit(string key, string line, int shows, string found, int right, string passed, string written, string quiet, bool missing, string who = "", string later = "")
	{
		RZ_DutyScript.G(key);
		array<string> seen = {"All there, by the list.", "Clean. You could eat off it.", "Socks, a razor, a letter. Nothing a man shouldn't have."};
		seen[shows] = found;
		array<string> codes = {"", "report", ""};
		if (right != 0)
			codes[0] = "morale-";

		if (missing)
		{
			codes[1] = "report flag:missing";
			codes[2] = "flag:missing";
		}

		if (right == 2 && !who.IsEmpty())
			codes[2] = codes[2] + " local";

		RZ_DutyBeat b = RZ_DutyScript.Case(key, line, RZ_DutyBeat.AT_HERE, 2,
			RZ_DutyScript.A("Count it against the list", "Look down the barrel", "Go through the pack"), seen,
			RZ_DutyScript.A("Pass the kit", "Write him up", "Deal with it quietly"),
			RZ_DutyScript.A(passed, written, quiet), codes, right, who, later);
		b.m_sThing = RZ_DutyThings.For("kit:" + key);
	}

	//------------------------------------------------------------------------------------------------
	//! Where a missing thing truly went, this time: one of five, after the kit it was missing from.
	protected static void Search(string what)
	{
		int where = Math.RandomInt(0, 5);
		RZ_DutyScript.T("ask", string.Format("The %1 has to be somewhere. He says he doesn't know. The man in the next bunk is studying the ceiling.", what), RZ_DutyBeat.AT_SAME, "missing");
		RZ_DutyScript.O("Go and find it", "You'll look. The whole hut pretends not to watch you go.", "flag:search");
		RZ_DutyScript.O("Leave it at a line in the book", "A line in the book, and the stores will issue another, eventually.");
		if (where == 0)
		{
			RZ_DutyScript.T("bunk", "", RZ_DutyBeat.AT_NEAR, "search");
			RZ_DutyScript.O("Look under the end bunk", string.Format("The %1, in a boot, under the end bunk. Hidden by someone with a grudge. The man studying the ceiling goes red.", what), "search hold:5 +2 regard+");
		}
		else if (where == 1)
		{
			RZ_DutyScript.T("contact", "The section had a contact on yesterday's patrol, out past the wire. He went to ground there, hard.", RZ_DutyBeat.AT_OUT, "search");
			RZ_DutyScript.O("Search where he went to ground", string.Format("In the long grass where a man threw himself flat: the %1, and eleven spent cases.", what), "search hold:8 +2 regard+");
		}
		else if (where == 2)
		{
			RZ_DutyScript.T("sold", string.Format("The man in the next bunk talks, in the end. The %1 went over the wire for cigarettes. To a toll man.", what), RZ_DutyBeat.AT_SAME, "search");
			RZ_DutyScript.O("Put the toll in the report", "In the report: what, to whom, for how many cigarettes. Somebody will be sent to buy it back.", "report +1 write", "", "", "The thing your inspection lost to a toll has been bought back. The private's paying for it out of his tobacco, for a year.");
			RZ_DutyScript.O("Let it stay sold", "You close the book on it. Two men owe you, and one of them knows a toll man.", "shady1 later", "", "You didn't write up where it went. I haven't forgotten. If you ever need something off the far side of the wire, ask me.");
		}
		else if (where == 3)
		{
			RZ_DutyScript.T("traded", string.Format("He says it, finally. The %1 went to a house down the lane, for bread, for a woman with three children.", what), RZ_DutyBeat.AT_OUT, "search");
			RZ_DutyScript.O("Ask at the house for it back", string.Format("She has it wrapped in a cloth on the dresser. She gives the %1 back before you finish asking, and asks if he's in trouble.", what), "hold:4 +2 later", "", "The woman down the lane sent bread up to the gate, for the one who came for the army's things and said the private wasn't in trouble.");
			RZ_DutyScript.O("Leave it where it's doing good", "You turn back at the gate. The book will say lost on patrol.", "shady1 local", "who has three children down the lane from the post", "A soldier came to my gate and went away again without knocking. I know what he came for. Tell him thank you.");
		}
		else
		{
			RZ_DutyScript.T("walked", string.Format("It isn't the %1 that's missing. It's the man who had it last. His bunk is the one made too neatly.", what), RZ_DutyBeat.AT_OUT, "search");
			RZ_DutyScript.O("Talk him back", "He's sitting on a milestone a quarter mile out, with his boots off. He lets you talk. He puts the boots back on.", "hold:10 +2 local", "who sat on a milestone outside the post", "You talked me back in. I'd have been shot or eaten by now. I'm on latrines for a month and I've never been so grateful for a shovel.");
			RZ_DutyScript.O("Let him go, and say you found nothing", "You look at him. He looks at you. You walk back the way you came.", "shady2 later", "", "You saw me on the milestone and you turned round. I'm a long way off now. I think of you when I hear a gate.");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Three kits laid out, from fourteen. Two looks at each of three places.
	static int Kits(RZ_Errand e, RZ_DutyScene scene)
	{
		array<ref RZ_DutyBeat> picked = {};
		RZ_DutyScript.Pool();
		Kit("clean", "A kit laid out square on the blanket, every strap rolled.", 0, "All there, by the list.", 0, "Passed. He lets out a breath he's held since reveille.", "You write up a clean kit. The section hears of it before you've left the hut.", "There's nothing to be quiet about. He looks at you, puzzled.", false);
		Kit("clean2", "A kit laid out by someone who has done it a hundred times.", 1, "Clean. You could eat off it.", 0, "Passed. He nods, once.", "Written up for nothing. He takes it without a word, which is worse.", "You wink at a man with nothing to hide. He doesn't wink back.", false);
		Kit("magazine", "A kit laid out a little too wide, to fill the blanket.", 0, "One magazine short of the list. The gap has been closed up.", 1, "Passed, a magazine short. It's somewhere, in somebody's hands.", "Written up, one magazine. He stares straight ahead.", "You say nothing in the book and tell him to find it by Friday.", true);
		Search("magazine");
		Kit("battery", "The section radio man's kit, with the set beside it.", 0, "The spare battery isn't there. The list says two.", 1, "Passed. The set will die on the first long patrol.", "Written up. He starts to say something, and stops.", "You tell him you'll look for it yourself, off the book.", true);
		Search("radio battery");
		Kit("map", "The corporal's kit. He is watching your hands.", 0, "The section's map case is here. The map isn't in it.", 1, "Passed. A section map is out there, with our posts marked on it.", "Written up, a corporal. The hut goes very quiet.", "You close the case and tell him he has till dark.", true);
		Search("section map");
		Kit("syrette", "The medic's kit, every pouch open.", 0, "One morphine syrette short. The count's been altered, in pencil, by a careful hand.", 1, "Passed. Someone is carrying morphine who shouldn't be, or needed it more than the book.", "Written up. He says quietly who he gave it to, and why, and it's a good reason, and you've written it up.", "You change the pencil back and tell him to tell you first, next time.", true);
		Search("syrette");
		Kit("tags", "A young private's kit. His hand keeps going to his collar.", 2, "His letters are gone from the pack, and his spare tags with them. He packed them himself, to go.", 2, "Passed. He'll be over the wire by the end of the week.", "Written up for intent. He's nineteen. The stockade has a draught.", "You put the tags back in his hand and tell him to write to her instead. He nods. He can't speak.", false, "who nearly went over the wire", "I had my letters packed. You gave me the tags back and didn't say anything in the book. I'm still here. I wrote to her.");
		Kit("barrel", "A kit laid out fast, by someone who was late.", 1, "The barrel is fouled grey. He hasn't pulled it through since the last contact.", 1, "Passed. That rifle will stop when he needs it.", "Written up, and he cleans it in front of you, properly, twice.", "You hand him the pull-through and stand there while he uses it. Nothing in the book.", false);
		Kit("round", "A kit laid out, the rifle on top, the bolt closed.", 1, "There's a round in the chamber. In a hut with thirty men in it.", 1, "Passed, loaded. It will go off one night, when someone knocks it off a peg.", "Written up. He goes white when you eject it onto the blanket.", "You clear it yourself and say what you think of him, quietly, for some time.", false);
		Kit("serial", "A kit with a rifle rather better kept than its owner.", 0, "The serial isn't his. It's on the lost list, from a man posted missing at the bridge.", 1, "Passed. A missing man's rifle stays in a hut, and his family stays without an answer.", "Written up. He says where he found it. A patrol goes out to look where he says.", "You tell him to swap it back before anyone else reads the number. A missing man stays missing.", false, "", "");
		Kit("vodka", "A kit with a pack that sloshes.", 2, "A bottle, wrapped in a shirt. Not army issue.", 2, "Passed, bottle and all. He'll be asleep on stag.", "Written up for a bottle. The section thinks less of you, and he does a week of nights.", "The bottle goes into the stove bucket, and he watches every drop, and you both say nothing.", false);
		Kit("smokes", "A kit with a pack packed very square.", 2, "Forty packets of cigarettes, in a brick. He doesn't smoke.", 2, "Passed. He's the post's tobacconist, at four times the price.", "Written up. The whole post is out of cigarettes by morning, and knows why.", "You tell him the price is halved from today, or the book hears of it. He agrees it's fair.", false);
		Kit("pass", "A kit with a pack he'd rather you didn't open.", 2, "A pass, stamped, made out in another name. And a civilian shirt.", 1, "Passed. He walks out the gate one evening as someone else.", "Written up, with the pass pinned to the page. He sits down on the bunk.", "You burn the pass in the stove and leave the shirt. He hasn't decided whether to thank you.", false);
		Kit("armband", "A kit laid out perfectly. Too perfectly.", 2, "At the bottom of the pack, folded small: a strip of red cloth, the Red Hand's.", 1, "Passed. One of the men on your wire answers to somebody else.", "Written up. He's gone from the hut before supper, between two corporals.", "You hold it up. He says it was his brother's. You put it back. You've chosen to believe him.", false);
		int pool = RZ_DutyScript.Draw(3, picked);
		foreach (RZ_DutyBeat b : picked)
		{
			scene.m_aBeats.Insert(b);
		}

		RZ_DutyScript.Into(scene.m_aBeats);
		RZ_DutyScript.T("rifle", "", RZ_DutyBeat.AT_HERE);
		RZ_DutyScript.O("Clean the section's spare rifle", "Stripped, pulled through, oiled and put back together with no parts left over.", "fix hold:10");
		return pool;
	}

	// ================================================================ The field telephone (worked out)
	//------------------------------------------------------------------------------------------------
	//! A break in the line. `how`: 0 broken (weather or beast), 1 cut (someone), 2 tapped.
	protected static void Break(string key, string line, string ends, string ground, string along, int how, string told = "")
	{
		RZ_DutyScript.G(key);
		RZ_DutyScript.Case(key, line, RZ_DutyBeat.AT_OUT, 2,
			RZ_DutyScript.A("Look at the wire's ends", "Look at the ground", "Follow the wire a few yards"),
			RZ_DutyScript.A(ends, ground, along),
			RZ_DutyScript.A("It broke, by weather or a beast", "It was cut, by someone", "It was tapped"),
			RZ_DutyScript.A("In the book as a break. A splice and it's done.", "In the book as cut. The post will double the night watch.", "In the book as tapped. Everything said on this line is to be thought of as heard."),
			RZ_DutyScript.A("", "report", "report"), how, "", "", told);
	}

	//------------------------------------------------------------------------------------------------
	//! Walk the wire to its break (one of ten), read it, splice it, ring through, and answer for
	//! what answers.
	static int Telephone(RZ_Errand e, RZ_DutyScene scene)
	{
		array<ref RZ_DutyBeat> picked = {};
		RZ_DutyScript.Into(scene.m_aBeats);
		RZ_DutyScript.T("start", "", RZ_DutyBeat.AT_GATE);
		RZ_DutyScript.O("Test the line at the post", "Dead. Not even a hum. The break is somewhere out along it.", "hold:3");
		RZ_DutyScript.Pool();
		Break("tree", "A pine is down across the line, and the wire goes under it.", "Stretched thin and snapped, the strands pulled to points.", "Torn roots, and the hole they came out of. Nothing else.", "The wire is whole on both sides of the tree, and taut.", 0);
		Break("chewed", "The wire lies in two pieces in the leaves.", "Flattened and frayed, strand by strand. There are tooth marks in the casing.", "Droppings. Something small lives here and didn't like the taste.", "Chewed again, ten yards on, and given up.", 0);
		Break("clean", "The wire hangs in two pieces from a fence post.", "Both ends square, bright, and exactly the same. Cutters.", "One boot print, deep at the toe. Someone stood on tiptoe to reach.", "No more damage. They cut once, here, and left.", 1, "The cut you read on the line was the first of three that week. We were ready for the other two because of your report.");
		Break("snares", "Thirty yards of wire are simply gone.", "Cut at both ends, not neatly. A knife, sawed.", "A rabbit run through the hedge, and a peg driven beside it.", "Further on, a loop of our wire on a peg, set across the run. Someone's hungry.", 1, "Whoever took the wire for snares has been found. A boy of twelve. He's been given real snares, and a clip round the ear.");
		Break("box", "The line goes into a junction box at the bottom of a ditch. The ditch is full.", "No break in the wire at all. The casing is sound right up to the box.", "The ditch has flooded since the rain. The box is a foot under.", "Sound beyond the box too. It's the box, and the water in it.", 0);
		Break("body", "The wire is pulled down off three posts and wound round something in the hedge.", "Not cut. Dragged, and stretched till it parted.", "A man, a week dead. He walked into it in the dark and took it with him as he fell.", "The wire leads into the hedge and round him, twice. He fought it.", 0);
		Break("snagged", "The wire is jerking. Something on the end of it is still moving.", "Not cut. Dragged down, and tight as a bowstring.", "Scuffed in a half circle, the width of its reach.", "One of the infected, wound up in forty yards of line, walking against it. It has been for days.", 0);
		Break("tap", "The wire looks whole. It tests dead from here to the post, and live beyond.", "No break. But here the casing has been slit and bound again, carefully, with the wrong tape.", "Nothing. Somebody swept their own tracks with a branch.", "A second wire, thin, grey, twisted onto ours and run off under the leaves towards the wood.", 2, "The tap you found had been on that line for a fortnight. We've been reading them our weakness every night. Not any more.");
		Break("house", "The line runs in at one window of an empty cottage and out at another.", "Cut inside the room, and the two ends twisted to a third wire.", "Swept floor. A cup, rinsed. A blanket folded very small behind the door.", "The third wire goes to an earpiece hung on a nail by the hearth. Someone sits here and listens.", 2, "Somebody was living in that cottage with an ear on our line. Not a spy. A woman whose husband is in the far post. She's been told he's alive.");
		Break("frost", "The wire has come away from four insulators and lies along the ground.", "Not cut. Brittle, and cracked through where it lay on the stone.", "Frost heave. The posts have all leaned the same way.", "Cracked again further on, the same way. It's the cold, and cheap wire.", 0);
		int pool = RZ_DutyScript.Draw(1, picked);
		foreach (RZ_DutyBeat b : picked)
		{
			scene.m_aBeats.Insert(b);
		}

		RZ_DutyScript.Into(scene.m_aBeats);
		RZ_DutyScript.T("splice", "");
		RZ_DutyScript.O("Splice the line", "Stripped, twisted, taped and hung clear of the ground.", "fix hold:9");
		int answer = Math.RandomInt(0, 4);
		RZ_DutyScript.T("ring", "The handset, clipped to the line above the splice.");
		if (answer == 0)
		{
			RZ_DutyScript.O("Crank the handle and ring through", "The far post, at once, bored and loud. They ask if you've any cigarettes.", "hold:4");
		}
		else if (answer == 1)
		{
			RZ_DutyScript.O("Crank the handle and ring through", "It rings. It goes on ringing. The far post has a man on that set day and night.", "hold:4 flag:quiet");
			RZ_DutyScript.T("quiet", "Nobody is answering at the far post.", RZ_DutyBeat.AT_SAME, "quiet");
			RZ_DutyScript.O("Report the far post silent", "Reported. A section goes out to them within the hour.", "report", "", "", "The far post didn't answer because the man on the set was asleep. He isn't, now. It could have been worse, and they went because you said.");
			RZ_DutyScript.O("Give it another minute", "On the ninth ring, a voice, out of breath. He'd been at the latrine. He begs you not to say.", "hold:6");
		}
		else if (answer == 2)
		{
			RZ_DutyScript.O("Crank the handle and ring through", "A voice answers. Not ours. It says the far post's name, slowly, as if reading it.", "hold:4 flag:voice");
			RZ_DutyScript.T("voice", "Whoever it is, they're waiting for you to speak.", RZ_DutyBeat.AT_SAME, "voice");
			RZ_DutyScript.O("Hang up and report it", "The handset goes down. The post stands to within the minute.", "report +1", "", "", "The voice on the line wasn't ours. The far post had been walked into an hour before. They were got out, most of them, because you hung up and ran.");
			RZ_DutyScript.O("Keep them talking", "You ask about the weather there. They tell you. They ask how many of you there are. You say plenty.", "hold:8 record", "", "", "You kept a stranger talking on our line for three minutes. The signals officer wants every word. He thinks he knows the accent.");
		}
		else
		{
			RZ_DutyScript.O("Crank the handle and ring through", "The line opens. Nobody speaks. Someone is breathing into it, close, wet, and regular.", "hold:4 report", "", "", "What you heard on the line was in the far post's signal hut. The set was off its hook. They've cleared the hut. You'll not want the details.");
		}

		RZ_DutyScript.T("more", "There's half a drum of wire left, and the further post has never had a line.");
		RZ_DutyScript.O("Run new wire to the further post", "Out along the hedge, post by post, to a hut that has only ever had runners. They can't believe it rings.", "hold:12 +2 morale+", "", "", "The further post has a telephone now. They rang last night to say goodnight. The sergeant pretended to be cross.");
		RZ_DutyScript.O("Coil it and go back", "Coiled, and carried home. Someone else's job, another day.");
		return pool;
	}

	// ================================================================ The loudspeaker tape (simple)
	//------------------------------------------------------------------------------------------------
	//! Out to the speaker on its pole, the old tape out, the new one in, and a test. And one of five
	//! things about the old tape, or the pole.
	static void Tape(RZ_Errand e, RZ_DutyScene scene)
	{
		RZ_DutyScript.Into(scene.m_aBeats);
		RZ_DutyScript.T("pole", "The speaker on its pole, facing the road. It has been saying the same thing for a month.", RZ_DutyBeat.AT_OUT);
		RZ_DutyScript.O("Open the housing", "Four wing nuts, rusted. The lid comes off with a noise like a gate.", "fix hold:5");
		RZ_DutyScript.T("swap", "");
		RZ_DutyScript.O("Take out the old tape, fit the new", "The old loop out, stretched and grey. The new one threaded and snapped shut.", "hold:6");
		int what = Math.RandomInt(0, 5);
		if (what == 0)
		{
			RZ_DutyScript.T("voice", "Under the official voice, on the old tape, there's another. Someone recorded over the end of the loop. A soldier, saying names.", RZ_DutyBeat.AT_SAME);
			RZ_DutyScript.O("Hand the old tape in", "Into your pack, for the signals hut. Somebody's last roll call.", "report record", "", "", "The names on that old tape were a section we'd written off. One of them said where they were going. We've sent someone to look.");
			RZ_DutyScript.O("Splice it into the new loop", "Thirty seconds of names, spliced in after the warning. The road will hear them every ten minutes.", "fix hold:8 shady1 rumour", "", "The speaker on the east road says names now, after the warning. Nobody will say who put them there. Nobody has taken them out.");
		}
		else if (what == 1)
		{
			RZ_DutyScript.T("nest", "There is a nest in the horn. Three eggs, and a wren very sure of her rights.", RZ_DutyBeat.AT_SAME);
			RZ_DutyScript.O("Move the nest to the hedge", "Carried over in your hat, eggs and all. She follows, shouting.", "hold:6 later", "", "The wren you moved has hatched three in the hedge by the east speaker. The sentry's named them after the officers.");
			RZ_DutyScript.O("Leave her there", "The speaker will sound like a speaker through a wren. The road can put up with it.");
		}
		else if (what == 2)
		{
			RZ_DutyScript.T("cutwire", "The lead to the battery has been cut, and the battery is gone. The speaker has been silent a while and nobody noticed.", RZ_DutyBeat.AT_SAME);
			RZ_DutyScript.O("Put the missing battery in the report", "In the report. A speaker with no battery warns nobody.", "report", "", "", "The battery from the east speaker turned up running a lamp in a cottage. They've been given a lamp of their own, and a talking to.");
			RZ_DutyScript.O("Wire it to the spare cell", "The spare cell from your pack, twisted on. It'll last a week.", "fix hold:6 +1");
		}
		else if (what == 3)
		{
			RZ_DutyScript.T("listener", "Someone is standing under the pole, face turned up to the horn, as they must have stood every day. They don't turn round. They can't hear you. They aren't breathing.", RZ_DutyBeat.AT_SAME);
			RZ_DutyScript.O("Put them down, quietly", "With the knife, from behind. They fall still facing the speaker.", "hold:6 infect-");
			RZ_DutyScript.O("Leave them listening", "You back away. The new tape starts, and they tilt their head to it.");
		}
		else
		{
			RZ_DutyScript.T("chalked", "Someone has chalked on the pole, under the speaker, in a child's hand: we heard you, we are in the mill.", RZ_DutyBeat.AT_SAME);
			RZ_DutyScript.O("Copy it down for the post", "Copied, with the date. The mill is two miles off.", "report hook", "", "", "There were nine people in that mill. They'd been answering our speaker in chalk for a fortnight. Nobody had read the pole.");
			RZ_DutyScript.O("Chalk an answer under it", "You write: stay there, someone will come. Then you'll have to make it true.", "write hold:5 later", "", "Somebody chalked an answer on the east pole. The people in the mill have been reading it every morning. They want to know when.");
		}

		RZ_DutyScript.T("test", "");
		RZ_DutyScript.O("Run the test", "It booms out over the road. Turn back, this zone is closed, in a voice that was a person once. Every bird for a mile goes up.", "hold:4 heat");
	}
}
