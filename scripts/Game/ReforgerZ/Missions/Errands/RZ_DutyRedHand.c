//! The Red Hand's own duties (docs/ERRANDS.md, "The Red Hand's duties"): stripping a wreck, the
//! still, the count, the toll and spotter, as beats for RZ_DutyScenes. The toll draws five from
//! fifty passers-by; the count and spotter are worked out from what you see. Server only.
class RZ_DutyRedHand
{
	// ================================================================ Stripping a wreck (simple)
	//------------------------------------------------------------------------------------------------
	static void Wreck(RZ_Errand e, RZ_DutyScene scene)
	{
		RZ_DutyScript.Into(scene.m_aBeats);
		RZ_DutyScript.T("wheels", "A car in the ditch, doors open, picked at but not stripped.", RZ_DutyBeat.AT_OUT);
		RZ_DutyScript.O("Take off the wheels", "Four wheels, and the spare. The nuts fight you. You win.", "fix hold:8");
		RZ_DutyScript.T("battery", "");
		RZ_DutyScript.O("Lift out the battery", "Out, heavy, with acid crusted on the posts. It still shows a spark.", "fix hold:6");
		RZ_DutyScript.T("seats", "");
		RZ_DutyScript.O("Unbolt the seats", "The front seats come out whole. Something slides out from under the driver's as it tips.", "fix hold:7");
		int found = Math.RandomInt(0, 6);
		if (found == 0)
		{
			RZ_DutyScript.T("pistol", "Under the seat, a cloth bundle. A pistol, taken to pieces, every part oiled.");
			RZ_DutyScript.O("Put it on the cart with the rest", "On the cart, in its cloth. The boss likes a thing that goes bang.", "report +1");
			RZ_DutyScript.O("Put it in your own pack", "Into your pack, under the spare shirt.", "shady2 give");
		}
		else if (found == 1)
		{
			RZ_DutyScript.T("shoe", "Under the seat, a child's shoe. One. And a rattle.");
			RZ_DutyScript.O("Leave them on the dashboard", "Set on the dash, side by side, where someone coming back would see them.", "later", "", "Somebody left a child's shoe on the dash of that wreck. The lads won't strip it further. They say it's been asked not to.");
			RZ_DutyScript.O("Throw them in the ditch", "Into the ditch. It's a car. It's parts.");
		}
		else if (found == 2)
		{
			RZ_DutyScript.T("cash", "Under the seat, a canvas bag. Banknotes, in bank bands. A great deal.");
			RZ_DutyScript.O("Hand the bag to the camp", "The lads laugh at it, and then use it to light the stove, a band at a time.", "report morale+");
			RZ_DutyScript.O("Keep the bag", "Into your pack. It's worth nothing. You keep it anyway.", "shady1");
		}
		else if (found == 3)
		{
			RZ_DutyScript.T("photos", "Under the seat, a biscuit tin of photographs. A wedding. A dog. The same four people, getting older.");
			RZ_DutyScript.O("Take the tin to the Haven's board", "You'll pin them up at the next house you pass. Somebody might know a face.", "later", "", "Someone pinned a tin of photographs on the Haven's board. A woman stood at it for an hour. She took one, and left the rest for whoever comes next.");
			RZ_DutyScript.O("Leave it in the car", "Back under the seat. Not parts.");
		}
		else if (found == 4)
		{
			RZ_DutyScript.T("boot", "Something shifts in the boot when the seats come out. Then knocks, slowly, from the inside.");
			RZ_DutyScript.O("Open the boot, knife ready", "It was somebody's passenger. It comes out badly, and you put it down in the road.", "hold:6 infect- heat");
			RZ_DutyScript.O("Leave the boot shut", "You chalk a cross on the lid, and take what you have.", "write hold:3");
		}
		else
		{
			RZ_DutyScript.T("map", "Under the seat, a road map, folded to this road. Someone has marked every toll on it, with a price.");
			RZ_DutyScript.O("Take it to the boss", "The boss reads it for a long time. Somebody's been pricing our road.", "report +1", "", "", "That map out of the wreck had our tolls on it, priced. The boss has moved two of them. He asked whose eyes found it.");
			RZ_DutyScript.O("Burn it", "It burns well. Whoever drew it will draw another.", "burn hold:4");
		}

		RZ_DutyScript.T("load", "");
		RZ_DutyScript.O("Load the handcart", "Wheels, battery, seats, roped down. The car is a shell with a number plate.", "hold:6");
	}

	// ================================================================ The still (simple, by hand)
	//------------------------------------------------------------------------------------------------
	static void Still(RZ_Errand e, RZ_DutyScene scene)
	{
		array<string> mash = {"potato", "plum", "beet"};
		string batch = mash[Math.RandomInt(0, 3)];
		bool climbing = Math.RandomInt(0, 2) == 0;
		RZ_DutyScript.Into(scene.m_aBeats);
		RZ_DutyScript.T("mash", string.Format("The still, behind the cookhouse. Today's is a %1 mash, and it smells of it.", batch), RZ_DutyBeat.AT_GATE);
		RZ_DutyScript.O("Load the mash", "Tipped in by the bucket, and the lid screwed down on its rag.", "hold:6");
		RZ_DutyScript.T("fire", "");
		RZ_DutyScript.O("Light the fire and feed it", "Split wood, small, a piece at a time. The copper starts to tick.", "burn hold:6");
		string needle = "The needle is falling out of the band. The drip has stopped.";
		if (climbing)
			needle = "The needle is climbing past the band. The lid is beginning to sing.";

		string damp = "wrong flag:cold";
		string feed = "right";
		if (climbing)
		{
			damp = "right";
			feed = "wrong flag:boil";
		}

		RZ_DutyBeat gauge = RZ_DutyScript.T("gauge", needle);
		gauge.m_fWait = 15;
		RZ_DutyScript.O("Damp the fire", "A shovel of ash on the coals. The needle thinks about it.", damp);
		RZ_DutyScript.O("Feed the fire", "Two more billets. The copper answers.", feed);
		RZ_DutyScript.T("boil", "It boils over. Froth out of the cap, onto the fire, and the fire likes it.", RZ_DutyBeat.AT_SAME, "boil");
		RZ_DutyScript.O("Throw the sand bucket", "Sand over the lot. The batch is half gone, and the wall is black, and you have your eyebrows.", "hold:4 heat");
		RZ_DutyScript.T("cold", "The fire's gone to sulk, and the mash is cooling in the pot.", RZ_DutyBeat.AT_SAME, "cold");
		RZ_DutyScript.O("Start the fire again", "Kindling, breath, patience. An hour lost, and the batch thinner for it.", "burn hold:8");
		RZ_DutyBeat cut = RZ_DutyScript.T("cut", "The first of it is coming over. Clear, sharp, and it smells like paint.");
		cut.m_fWait = 10;
		RZ_DutyScript.O("Pour the first runnings away", "A cupful, onto the ground. The grass there won't thank you. The lads' eyes will.", "right hold:4");
		RZ_DutyScript.O("Keep it all", "Every drop into the jar. More to sell. The first cup of it is the kind that blinds.", "wrong morale- later", "", "Two of the lads couldn't see straight for three days after your batch. One still can't, of a morning. The boss asked who ran it.");
		Around(batch);
		array<ref RZ_DutyBeat> picked = {};
		RZ_DutyScript.Draw(1, picked);
		foreach (RZ_DutyBeat b : picked)
		{
			scene.m_aBeats.Insert(b);
		}

		RZ_DutyScript.Into(scene.m_aBeats);
		RZ_DutyScript.T("bottle", "");
		RZ_DutyScript.O("Bottle it", "Six bottles, corked with rag. One of them is yours, by custom.", "hold:6 vodka");
	}

	//------------------------------------------------------------------------------------------------
	//! One thing that happens round a still, from eight. Into the script's pool.
	protected static void Around(string batch)
	{
		RZ_DutyScript.Pool();
		RZ_DutyScript.G("thief");
		RZ_DutyScript.T("thief", "A hand comes round the door post and feels along the shelf for a bottle.");
		RZ_DutyScript.O("Take hold of the wrist", "A boy of fourteen on the end of it, wanting it for his dad's leg, he says. To clean it.", "flag:boy");
		RZ_DutyScript.O("Let the hand find one", "It finds one, and goes. You'll be a bottle short at the count.", "-1");
		RZ_DutyScript.T("boy", "He's telling the truth about the leg. You can smell it on his clothes.", RZ_DutyBeat.AT_SAME, "boy");
		RZ_DutyScript.O("Give him a cupful, for the leg", "A cupful in a jar, and how to use it. He runs.", "local", "whose father has the bad leg", "My dad kept the leg. He says the stuff you gave me was the worst thing he ever tasted, and he only washed with half of it.");
		RZ_DutyScript.O("Send him off with a clip", "Off he goes, holding his ear. The leg isn't your business.");
		RZ_DutyScript.G("smoke");
		RZ_DutyScript.T("smoke", "The smoke is going straight up, white, in still air. You can see it from the road. So can anything else.");
		RZ_DutyScript.O("Bank the fire and wait it out", "Banked down to a glow. An hour lost. Nothing comes.", "hold:8");
		RZ_DutyScript.O("Run it hot and be done", "Hot and fast. Something on the road has turned its head this way.", "heat");
		RZ_DutyScript.G("stronger");
		RZ_DutyScript.T("stronger", "The boss puts his head in. Stronger, he says. Last lot wouldn't light.");
		RZ_DutyScript.O("Run it twice through", "Through the copper a second time. Half as much, and it'd strip a door.", "hold:8 +1");
		RZ_DutyScript.O("Tell him it's as strong as it's safe", "He looks at you. Then he laughs, and tells the lads you've a spine.", "regard+");
		RZ_DutyScript.G("buyer");
		RZ_DutyScript.T("buyer", "A woman from the Haven is at the fence with a basket. Not for drinking, she says. For wounds, and she has eggs to pay with.");
		RZ_DutyScript.O("Sell her a bottle for the eggs", "A bottle for a dozen eggs. The crew eats omelette and asks no questions.", "+1 local", "who bought spirit for wounds at the camp fence", "Your spirit cleaned eleven dressings before it ran out. Nobody here asks where I got it. They know. They say thank you by not asking.");
		RZ_DutyScript.O("Give her one for nothing", "You hand it through the wire and wave the eggs away. The boss would have your hide.", "shady1 local", "who was given spirit for wounds at the camp fence", "You wouldn't take the eggs. I've told the house there's one of them at that camp who isn't all the way gone.");
		RZ_DutyScript.O("Send her away", "No trade with the Haven, the boss says. She goes, with her eggs.");
		RZ_DutyScript.G("coil");
		RZ_DutyScript.T("coil", "A hiss, and steam where steam shouldn't be. The coil has cracked at the bend.");
		RZ_DutyScript.O("Bind it with rag and clay", "Rag, clay, wire. It holds, sweating. It'll want doing properly.", "fix hold:7");
		RZ_DutyScript.O("Cut the crack out and rejoin it", "Sawn out, sleeved, crimped. Better than it was.", "fix hold:12 +1 regard+");
		RZ_DutyScript.G("rival");
		RZ_DutyScript.T("rival", "A man you don't know is leaning on the fence, friendly. What's the mash, he asks. What's the cut. His crew would pay.");
		RZ_DutyScript.O("Tell him nothing", "Nothing. He shrugs, and counts the bottles on the shelf from where he stands.", "regard+");
		RZ_DutyScript.O("Tell him wrong", "You tell him it's all in keeping the first cup. He thanks you warmly. His crew will have a bad week.", "rumour", "", "A crew down the valley has gone half blind on its own drink. They say somebody at the Red Hand's still gave them the method.");
		RZ_DutyScript.O("Sell him the method", "He pays in cartridges, on the spot. The boss's drink is everyone's drink now.", "shady2 give");
		RZ_DutyScript.G("wet");
		RZ_DutyScript.T("wet", "The woodpile is wet through. It hisses and won't take.");
		RZ_DutyScript.O("Split it down to the dry heart", "Every billet split to its middle. Slow, and it burns.", "hold:9");
		RZ_DutyScript.O("Pour a cup of the last batch on it", "It takes at once, blue. That's a cup nobody will drink.", "burn hold:3 -1");
		RZ_DutyScript.G("taster");
		RZ_DutyScript.T("taster", string.Format("One of the lads wants to taste the %1 before it's bottled. He's on the gate in an hour.", batch));
		RZ_DutyScript.O("Give him a thimble", "A thimble. He pulls a face and says it's the best yet. He walks to the gate in a straight line.", "regard+");
		RZ_DutyScript.O("Give him a cup", "A cup. He's asleep at the gate by dusk, with the bar up.", "morale- later", "", "The gate was open and the gateman asleep, the night of your batch. Nothing came through. That's luck, not management.");
		RZ_DutyScript.O("Not before the gate", "No. He calls you his mother. He goes to the gate sober.");
	}

	// ================================================================ The count (worked out)
	//------------------------------------------------------------------------------------------------
	//! One thing on the tarp. `right`: 0 the crew's pile, 1 to the fence, 2 the boss's.
	protected static void Haul(string key, string line, string turned, string weighed, int right, string remark)
	{
		RZ_DutyScript.G(key);
		RZ_DutyBeat b = RZ_DutyScript.Case(key, line, RZ_DutyBeat.AT_HERE, 2,
			RZ_DutyScript.A("Turn it over", "Weigh it in your hand", "Check it against the tally"),
			RZ_DutyScript.A(turned, weighed, remark),
			RZ_DutyScript.A("Put it on the crew's pile", "Put it aside for the fence", "Put it on the boss's pile"),
			RZ_DutyScript.A("On the crew's pile, to be shared out.", "Aside, for the fence, and whatever he'll give.", "On the boss's pile. Nobody touches that pile."),
			RZ_DutyScript.A("", "", ""), right);
		b.m_sThing = RZ_DutyThings.For("count:" + key);
	}

	//------------------------------------------------------------------------------------------------
	//! Four things off the tarp, from fourteen: each to the crew, the fence or the boss. Then the
	//! tally is one short, and the tally's own hand says who.
	static int Count(RZ_Errand e, RZ_DutyScene scene)
	{
		RZ_Rng rng = RZ_DutyPosts.Seed(e, 29);
		array<string> crew = {RZ_NoteText.FirstName(rng), RZ_NoteText.FirstName(rng), RZ_NoteText.FirstName(rng)};
		int skimmer = Math.RandomInt(0, 3);
		array<string> remarks = {};
		for (int i = 0; i < 14; i++)
		{
			int by = Math.RandomInt(0, 3);
			if (by == skimmer && Math.RandomInt(0, 2) == 0)
				remarks.Insert(string.Format("Entered by %1, in pencil. The line under it has been rubbed out, and something written over.", crew[by]));
			else if (by == skimmer)
				remarks.Insert(string.Format("Entered by %1. The count beside it was a five, and is now a four.", crew[by]));
			else
				remarks.Insert(string.Format("Entered by %1, in ink, and initialled.", crew[by]));
		}

		array<ref RZ_DutyBeat> picked = {};
		RZ_DutyScript.Pool();
		Haul("tins", "A sack of tins, dented, most with their labels.", "Meat, beans, one of peaches. In date, near enough.", "Heavy. Full, every one.", 0, remarks[0]);
		Haul("boots", "Three pairs of boots, tied by the laces.", "Good soles. One pair has a name inked inside, crossed through.", "Worn in, not worn out.", 0, remarks[1]);
		Haul("gold", "A bar of gold the size of a soap, wrapped in a sock.", "A bank's stamp, and a number.", "Heavier than it has any right to be.", 2, remarks[2]);
		Haul("pearls", "A string of pearls, in a jewel case.", "Knotted between each one. A jeweller's mark on the clasp.", "Cool, and they warm in the hand. Real.", 1, remarks[3]);
		Haul("cards", "A wallet fat with bank cards, in six different names.", "Six names, one signature. Somebody's whole trade, before.", "Nothing. Plastic.", 1, remarks[4]);
		Haul("cash", "Banknotes, in a pillowcase.", "Real, and crisp, and no use to anyone.", "A pillow's weight of paper.", 1, remarks[5]);
		Haul("medicine", "A box of dressings and brown glass bottles.", "Iodine, morphine, sulfa. The seals are whole.", "Full bottles, by the slosh.", 0, remarks[6]);
		Haul("rifle", "A hunting rifle with a carved stock.", "Initials cut under the grip, and a red thumbprint in wax beside them. The boss's mark.", "Balanced. Somebody loved this.", 2, remarks[7]);
		Haul("radio", "A field radio, olive, with a cracked case.", "Bastion's stencil on the back. Still set to a frequency.", "The battery's in it. It's warm.", 2, remarks[8]);
		Haul("watch", "A gold watch on a chain.", "Engraved inside the lid, to a station master, for forty years.", "Still wound. Still right.", 1, remarks[9]);
		Haul("coat", "A child's winter coat, nearly new.", "A name tape in the collar. Mittens on a string through the sleeves.", "Light. A pocket has a boiled sweet in it.", 0, remarks[10]);
		Haul("tools", "A mechanic's roll of spanners, every size.", "Oiled, and each in its pocket.", "A proper weight. The real thing.", 0, remarks[11]);
		Haul("papers", "A folder of passes, blank, stamped ready.", "Bastion's stamp, on forty forms, and no names filled in.", "Paper. Worth a great deal more than paper.", 2, remarks[12]);
		Haul("ring", "A man's signet ring.", "A red thumbprint pressed into the wax in the hollow. Marked for the boss before it reached the tarp.", "Heavy. Old gold.", 2, remarks[13]);
		int pool = RZ_DutyScript.Draw(4, picked);
		foreach (RZ_DutyBeat b : picked)
		{
			scene.m_aBeats.Insert(b);
		}

		RZ_DutyScript.Into(scene.m_aBeats);
		RZ_DutyScript.T("short", "The tarp's empty, and the tally says there was one thing more. Somebody had the book before you. Who do you name to the boss?", RZ_DutyBeat.AT_HERE);
		for (int c = 0; c < 3; c++)
		{
			if (c == skimmer)
				RZ_DutyScript.O(string.Format("Name %1", crew[c]), "You say the name. The boss nods slowly, as if you'd told him the time.", "right report", "", "", string.Format("%1 had it in his boot. The boss let him keep the boot. You read that book right, and he'll remember who did.", crew[c]));
			else
				RZ_DutyScript.O(string.Format("Name %1", crew[c]), "You say the name. The boss looks at you a moment longer than you'd like.", "wrong morale-", "", "", string.Format("You named %1, and it wasn't him. He took a beating for it before the right man owned up. He knows whose word it was.", crew[c]));
		}

		return pool;
	}

	// ================================================================ The toll (worked out)
	//------------------------------------------------------------------------------------------------
	//! One who comes up the road. `right`: 0 take the toll, 1 wave them through, 2 turn them back.
	protected static void Pass(string key, bool when, string line, string pack, string them, string bound, int right, string took, string waved, string turned)
	{
		RZ_DutyScript.G(key, when);
		RZ_DutyBeat b = RZ_DutyScript.Case(key, line, RZ_DutyBeat.AT_ENTRY, 2,
			RZ_DutyScript.A("Look in what they carry", "Look at their hands and boots", "Ask where they're bound"),
			RZ_DutyScript.A(pack, them, bound),
			RZ_DutyScript.A("Take the toll", "Wave them through", "Turn them back"),
			RZ_DutyScript.A(took, waved, turned),
			RZ_DutyScript.A("+1", "", ""), right);
		// They stand at the pole to be talked to (dialogue/duties/toll.rzd): the asking is option 2
		b.m_sTalk = "toll:" + key;
		b.m_sTalkName = "Someone at the pole";
		b.m_sThing = RZ_DutyThings.For("toll:" + key);
	}

	//------------------------------------------------------------------------------------------------
	//! A shift on the bar: five come up the road, drawn from fifty by what's so (the stage, the
	//! hour, the weather, how sick the town is). Then the boss comes to count the take. How many
	//! were in the pool today.
	static int Toll(RZ_Errands dealer, RZ_GameModeComponent gm, RZ_Errand e, RZ_DutyScene scene)
	{
		RZ_WorldFacts facts = gm.GetFacts();
		// Who comes up the road follows who's at odds and who holds it: a feud keeps a faction's
		// people off it, a truce or a Bastion post near brings Bastion's up it
		RZ_FactionLife life = gm.GetFactionLife();
		bool feudBastion = life && life.HasFeud(RZ_EFaction.BANDITS, RZ_EFaction.MILITARY);
		bool truceBastion = life && life.HasTruce(RZ_EFaction.BANDITS, RZ_EFaction.MILITARY);
		bool feudHaven = life && life.HasFeud(RZ_EFaction.BANDITS, RZ_EFaction.SURVIVORS);
		bool feudHalcyon = life && life.HasFeud(RZ_EFaction.BANDITS, RZ_EFaction.INSTITUTE);
		array<RZ_RoadPost> posts = {};
		if (gm.GetCheckpoints())
			gm.GetCheckpoints().PostsNear(RZ_EFaction.MILITARY, e.m_vGiver, 1500, posts);

		bool bastionRoad = !posts.IsEmpty();
		RZ_Settlement town = gm.GetSettlement(e.m_iSettlement);
		bool late = facts && facts.Stage() >= RZ_EOutbreakStage.COLLAPSE;
		bool early = !late;
		bool dark = facts && (facts.IsDusk() || facts.IsNight());
		bool day = !dark;
		bool rain = gm.GetEnvironment() && gm.GetEnvironment().IsRaining();
		bool sick = town && town.m_fInfection >= 40;
		array<ref RZ_DutyBeat> picked = {};
		RZ_DutyScript.Pool();
		Pass("handcart", true, "A family with a handcart, two children asleep on the bedding.", "Bedding, a clock, a birdcage with no bird. A sack of flour under it all.", "The father's hands are a clerk's. The boots are split.", "Anywhere that has a roof, he says.", 1, "A scoop of their flour into the box. The mother watches it go.", "Through, flour and all. They don't believe it until they're past the bend.", "Back the way they came. There's nothing the way they came.");
		Pass("courier", !feudHaven, "A woman on a bicycle with a satchel, in a hurry, with the Haven's ribbon on the bars.", "Dressings and sulfa, wrapped against the wet.", "Ink on her fingers, mud to the knee.", "A house with a fever in it, three miles on.", 1, "A third of the dressings into the box. That's a third of someone's chance.", "Through. She's gone before the bar's fully up.", "Turned. She says the name of the house, twice, and then goes.");
		Pass("deserter", true, "A man alone, in a Bastion greatcoat with the badges cut off.", "A rifle with no bolt, wrapped in sacking. Nothing to eat.", "The boots are army, and good. He hasn't slept.", "Away, he says. Just away.", 0, "The coat for his passage. He gives it up like a skin.", "Through, coat and all. A Bastion patrol will ask who passed him.", "Back toward the post he ran from. He stands in the road a long time.");
		Pass("tech", !feudHalcyon, "A man in a white coat under a mac, with a sealed steel case chained to his wrist.", "Cold, through the steel. A seal with a number.", "Soft hands, good shoes ruined.", "The lab. He's late. Please.", 1, "You take his watch, since the case won't open. He hardly notices.", "Through. Whatever's in the case gets where it's going, cold.", "Turned. The case warms in the road while he argues.");
		Pass("child", true, "A child of about nine, alone, walking on the crown of the road as if it were hers.", "A doll, a crust, a kitchen knife.", "No shoes. Feet like leather.", "To her auntie's. She can't say where that is.", 1, "You take the knife. A child shouldn't. She looks at you as if you'd taken her arm.", "Through. She doesn't look back. She hasn't looked back at anything in a while.", "You send a child back down an empty road.");
		Pass("oldman", true, "An old man with a stick and nothing else, and a great deal to say.", "Nothing. Not a crumb. He turns his pockets out to show you.", "Clean hands. He's been fed somewhere this morning.", "The next toll, and the one after. He walks the road and carries the news.", 0, "He pays in news: the bridge is down, and Bastion's truck is three days late. That's worth more than tins.", "Through, with his news untold.", "Turned. He tells the news to the hedge, loudly, as he goes.");
		Pass("bitten", sick, "A man with his hand in his coat, sweating, with a full pack.", "Food for a week, and a child's drawing.", "He won't take the hand out. There's a smell, sweet.", "Home, he says. He's nearly home.", 2, "You take half the pack. He's past caring. He's inside the camp's ground now.", "Through. He's on our road, among our people, with that hand.", "Turned, at a pole's length. He nods. He knew.");
		Pass("runner", true, "A lad at a trot, with a blue rag round his arm. Not our colour.", "A folded note, sealed with candle wax. Nothing else at all.", "Light shoes. He's run this before.", "He won't say. He's breathing through his nose and watching your hands.", 2, "You take the note. He lets you. It's blank. He was sent to see what we'd do.", "Through. Another crew's message crosses our ground free.", "Turned, with a word for whoever sent him. He grins. That was the message.");
		Pass("armed", true, "A big man with a shotgun over his arm, broken open, and a look.", "Cartridges. A brace of rabbits.", "A keeper's hands. A keeper's patience.", "Through. He's said it once.", 1, "You name a price. He closes the gun. You un-name it.", "Through, with a nod between men who've each decided something.", "You tell him no. He considers you. It is a long consideration.");
		Pass("deadcart", sick, "A flat cart, a tarpaulin, and a man leading the horse with a rag over his face.", "Six, under the tarp. Wrapped. One of the wrappings moves with the cart, only with the cart.", "Lime to the elbows.", "The pit, beyond the camp. There's nowhere else.", 1, "You take the horse's nosebag of oats. He lets you. He's past caring about oats.", "Through, quickly. Everybody steps back.", "Turned. He'll tip them in the ditch, a hundred yards down. Upwind.");
		Pass("forged", late || bastionRoad, "A man in a suit with Bastion papers he presents before he's asked.", "A change of collar. A second set of papers, different name.", "A gentleman's hands. A burglar's eyes.", "On Command's business. He says Command a great deal.", 0, "Both sets of papers into the box. The boss will know what they're worth. He walks on, lighter.", "Through, on forged papers. He'll use them at Bastion's gate and say he came through us.", "Turned. He'll find a toll with a duller man on it.");
		Pass("workoff", true, "A young man with nothing, and he says so before you ask.", "A spare shirt. That is the whole of it.", "Hands like a navvy's.", "Wherever there's work. He'll do an hour's for his passage, now.", 0, "An hour on the woodpile for the camp, and he's through, with soup in him.", "Through for nothing. He looks at you as if you'd made a mistake.", "Turned. There was an hour of work in him, and you've sent it down the road.");
		Pass("protected", true, "A woman with a red thread round her wrist, who says the boss knows her.", "Tobacco, a great deal of it.", "Rings on every finger. None of them wedding.", "The camp. Where else.", 1, "You take a twist of tobacco. She smiles, and says she'll mention you.", "Through. She says she'll mention you. This time it's a promise.", "Turned. She laughs, out loud, and sits down on the verge to wait for the boss.");
		Pass("priest", true, "A priest with a bicycle and a tin box of the Host.", "Wafers. Wine in a medicine bottle. A book.", "Chilblains. A patched cassock.", "A dying man, the far side. He's been sent for.", 1, "You take the wine. He blesses you anyway, which is worse.", "Through. He rings the bell on his bicycle as he goes under.", "Turned. A man dies without him, the far side.");
		Pass("pedlar", true, "A pedlar with a tray of needles, thread, combs and one mirror.", "Exactly that. And a bottle in the lining.", "Quick fingers. He's noticed where your box is.", "Every camp and every house. He's welcome in all of them.", 0, "A paper of needles for the box. The cook will kiss you.", "Through, with his tray. He'd have paid.", "Turned. The camp goes without needles till he comes round again.");
		Pass("doctor", sick, "A tired woman with a black bag, walking, because the car ran dry.", "Instruments, boiled and wrapped. Very little else left.", "Scrubbed raw to the wrist.", "Whoever needs her. Today that's the far side.", 1, "You take the bag's last morphine. She gives you a look that will stay.", "Through. She's back before dark, and stops to look at the cook's hand, for nothing.", "Turned. Somewhere the far side, someone waits for a knock.");
		Pass("soldiers", (late || bastionRoad || truceBastion) && !feudBastion, "Two Bastion soldiers, rifles slung, walking the wrong way.", "Rations for three days. A map with a cross on it.", "They keep their hands away from the slings, with care.", "They were told to walk till they found someone. They've found you.", 2, "You take the rations. They let you. They've decided you're what they were sent to find.", "Through. Two of Bastion's walk into our ground and write down what they see.", "Turned, politely. They go, and they count the men on the bar as they leave.");
		Pass("mother", true, "A woman carrying a boy of five who is too big to be carried.", "A feeding bottle, a blanket, a tin of condensed milk.", "She's swaying. The boy is very hot.", "The Haven's nurse. Two miles.", 1, "You take the milk. It's all she had for him.", "Through. One of the lads walks her the first half mile, and doesn't say why.", "Turned. She sits down in the road with him. She isn't going anywhere else.");
		Pass("twins", true, "Two brothers who answer together, with one pack between them.", "Tools, all good, and all with different initials.", "Four hands that have been in other people's sheds.", "Trading. They say it at the same moment.", 0, "A brace and bit for the box. They part with it without grief. It wasn't theirs for long.", "Through, with a shed's worth of other people's tools.", "Turned. They go round by the field. You'd have had a brace and bit.");
		Pass("singer", day, "A girl with a guitar on her back and no pack.", "Strings, a tuning fork, an apple.", "A player's nails. Nothing else to her name.", "Wherever they'll feed her for a song.", 0, "A song for the bar. The lads go quiet. One of them goes round the back of the hut.", "Through. You never heard her play.", "Turned, with her guitar. The camp stays as it was.");
		Pass("butcher", true, "A man with a goat on a rope and a knife in his belt.", "The goat. A whetstone.", "Blood under the nails, old.", "The camp, to sell the goat, if he gets that far with it.", 0, "A leg, when it's killed, for the bar. He agrees. He kills it there and then.", "Through, with all four legs.", "Turned. The goat doesn't mind either way.");
		Pass("nun", true, "Three nuns in a row, the last one leading a blind man.", "Bandages, a rosary, a ledger of names.", "Three pairs of sensible shoes, worn through.", "They're walking the names in the ledger. Every house, till they've found each one or its grave.", 1, "You take the bandages. The eldest thanks you for leaving the ledger.", "Through. The eldest writes something in the ledger. Your face, probably.", "Turned. They kneel in the road and start on the rosary. It's a long rosary.");
		Pass("thief", dark, "A man walking fast in the dark, with a sack that clinks.", "Candlesticks. A clock. A crucifix, silver.", "Plaster dust on his sleeves.", "Away from the church at the crossroads, by the look.", 0, "The sack, for the box. All of it. He goes on with his hands empty and his skin whole.", "Through, with a church in a sack.", "Turned, back toward the people he took it from.");
		Pass("lovers", dark, "A boy and a girl, hand in hand, from different sides of something.", "Bread, a blanket. A red rag in her pocket, a Haven ribbon in his.", "Neither is wearing the boots for a long walk.", "They don't know. Not back.", 1, "You take the bread. They share what's left of nothing.", "Through. One of the lads says good luck, and gets looked at.", "Turned. They go back, a yard apart.");
		Pass("midwife", true, "A stout woman at a trot with a bag, who doesn't slow down.", "Towels, scissors, string, a bottle of spirit.", "Sleeves already rolled.", "The house with the green door, and it won't wait for your bar.", 1, "You stop her to take the spirit. She tells you what you are, with details.", "The bar goes up before she reaches it. She doesn't thank you. She hasn't time.", "Turned. Nobody has ever turned her before. She goes through the hedge instead.");
		Pass("gambler", true, "A neat man with a pack of cards, who offers to cut you for the toll.", "Cards, three packs, all the same back. Dice.", "Very clean nails.", "The camp. He's heard the lads have tins and no sense.", 2, "You cut him for it. You lose your own tobacco. So that's how that works.", "Through. By morning the lads have no tins, and he has no enemies, somehow.", "Turned. He takes it as a compliment.");
		Pass("teacher", early, "A schoolmaster with a satchel of exercise books and chalk in his pocket.", "Thirty exercise books, marked. A register.", "Chalk dust. A cane he has never used.", "He's looking for his class. He's found nine of the thirty.", 1, "You take the chalk. He has to ask for a piece back, to write with.", "Through. He asks if you've any children at the camp. You have. He writes their names.", "Turned, with twenty-one left to find.");
		Pass("vet", true, "A woman with a horse, leading it, though it could carry her.", "Horse medicine. Enough to stop a heart, or start one.", "A farmer's forearms.", "A farm with a sick cow, and the whole valley's milk in it.", 1, "You take a bottle. You don't know what it's for. She tells you, and you give it back.", "Through. The valley has milk next week.", "Turned. The cow dies. So does the milk.");
		Pass("fugitive", late, "A man in pyjamas and an overcoat, with a hospital band on his wrist.", "Nothing. Not even shoes.", "A needle mark in the crook of each arm. Halcyon's band.", "Away from the lab. He says they were going to try something on him tomorrow.", 1, "You take the overcoat. It's all there is. He's in pyjamas, on a road.", "Through. Halcyon will ask the road who saw him.", "Turned, back toward the lab. He sits down. He'd rather be shot, he says, and means it kindly.");
		Pass("hunter", true, "A hunter with two deer legs over his shoulder, still dripping.", "Meat. Real meat, today's.", "Blood to the elbow, and a good knife.", "The Haven. He's promised it.", 0, "One leg for the bar. He'd expected to lose both.", "Through, with both legs. The lads watch them go the way dogs do.", "Turned. The meat goes off in the road while he thinks about it.");
		Pass("clerk", early, "A bank clerk with a ledger chained to his wrist.", "The ledger. Every account in the valley, and who owed what.", "A callus on one finger, from the pen.", "He's keeping it safe. For afterwards.", 0, "You take a page from it: the page with the boss's name. The boss owes nothing, now.", "Through, with every debt in the valley intact.", "Turned. He goes off to keep it safe somewhere else.");
		Pass("beekeeper", day, "An old woman with a skep on a barrow, humming.", "Bees. A great many, and they know she's there.", "Stung so often it doesn't show.", "Higher up, where the heather is.", 1, "You reach for the honey jar. The barrow has opinions about that.", "Through, at arm's length. The humming goes with her.", "Turned. She sets the skep down, in the road, by the bar, to rest. For as long as it takes.");
		Pass("smuggler", true, "A cheerful man with a false bottom in his cart, which he shows you.", "Under the false bottom: salt, soap, lamp oil.", "A horse trader's hands.", "The far towns. He comes back this way, every week, and pays every week.", 0, "The usual, into the box. He's paid it for a year. It's how the road works.", "Through for nothing. He's worried now. Free is worse than dear.", "Turned. He won't come this way again, and the camp goes without soap.");
		Pass("pilgrim", true, "A barefoot man with a cross on his back, a real one, timber.", "Nothing. He's carrying it.", "Shoulders rubbed raw. Feet worse.", "The line. He's going to stand at the line and pray till they open it.", 1, "There's nothing to take. You take nothing, and write it down as nothing.", "Through. Somebody gives him a pair of socks. He carries them.", "Turned. He turns, with the cross, in a wide circle, and tries the field.");
		Pass("widow", true, "A woman in black with a handbag, quite composed.", "A man's razor, shaving brush, pipe. Kept clean.", "A ring worn outside her glove.", "To throw them in the river. She's decided.", 1, "You take the pipe. It was a good one. She watches where you put it.", "Through. She comes back an hour later with an empty bag and thanks you.", "Turned. She stands there. Then she hands you the bag, and goes home.");
		Pass("recruit", true, "A boy of sixteen who wants to join, and has brought his own knife.", "A kitchen knife. A loaf. A note from his mother, which he hasn't read.", "No calluses yet.", "Here. He wants to be one of you.", 2, "You take the loaf and tell him to wait for the boss. He stands up very straight.", "Through, into the camp. By the week's end he has a red rag and a split lip.", "You send him home. He calls you a coward from forty yards.");
		Pass("leper", sick, "Someone wrapped from crown to foot, ringing a handbell.", "Under the wrappings, a tin plate and a spoon.", "You don't look. Nobody looks.", "The pit. Or anywhere that will have them.", 1, "You take nothing, with a long pole.", "Through, wide. The bell goes on down the road.", "Turned. The bell stops in the road, a hundred yards back. It's still there at dusk.");
		Pass("surveyor", early, "A man with a tripod and a notebook, measuring the road.", "A level, a chain, a notebook of figures.", "Good boots, government issue.", "He's finishing the survey. Nobody told him to stop.", 0, "You take the notebook: every track and ford in the valley, measured. The boss will want it.", "Through, measuring as he goes. He writes down the bar.", "Turned. He measures the distance back.");
		Pass("carter", rain, "A carter with a wagon stuck to its axles, forty yards short of the bar.", "Seed potatoes. A whole spring's planting for somewhere.", "He's been pushing since noon.", "The Haven's farm. If the wagon ever moves.", 0, "The lads push him out, for a sack. He'd have paid two.", "The lads push him out for nothing. He can't make it out, and keeps looking behind him.", "You leave him there. By dark the potatoes are somebody else's.");
		Pass("drowned", rain, "A man wet to the skin, without a pack, who came through the river.", "Nothing. The river has it.", "Shaking too hard to stand still.", "The bridge is down. He says so three times.", 1, "There's nothing on him. You take his news, and it's worth having.", "Through, to the fire. He tells the camp about the bridge, when he can talk.", "Turned. He goes back toward a river.");
		Pass("sleeper", dark, "A man walking with his eyes shut, in a nightshirt.", "Nothing. A door key, in his fist.", "Bare feet, cut. He's walked a mile at least.", "He's asleep. He's going home, to a house he had before.", 1, "You take the key. He doesn't wake. He stands at the bar until someone leads him off.", "One of the lads turns him by the shoulders and walks him back to his bed.", "You turn him round. He walks back the way he came, into the dark, asleep.");
		Pass("patrolman", late, "A man in half a police uniform, who still asks to see your authority.", "Handcuffs, a whistle, a notebook with every toll's takings in it.", "A truncheon, worn smooth.", "He's making a case. For afterwards. He has all your names.", 2, "You take the notebook. He says he has it by heart. You believe him.", "Through. He notes the time, and your face.", "Turned. He goes, and writes that down as well.");
		Pass("burner", sick, "Two men in aprons with cans of paraffin and faces like doors.", "Paraffin. Rags. A list of houses.", "Scorched to the elbow.", "The next house on the list, since somebody has to.", 1, "You take a can. It's the camp's now. The list is a house short of paraffin.", "Through. There's smoke the far side by evening.", "Turned. The house on the list stays as it is, with what's in it.");
		Pass("bride", day, "A girl in a wedding dress, hem black to the knee, with a suitcase.", "A trousseau. Linen, initialled, never used.", "One ring, on the wrong hand.", "The church. It was to have been this month. She's going anyway.", 1, "You take a tablecloth. She says keep it. She says it very brightly.", "Through. The lads take their caps off. Nobody knows why.", "Turned. She sets the suitcase down and sits on it, facing the way she was going.");
		Pass("spy", late, "A woman selling eggs, who knows the name of every man on the bar.", "Eggs. Under the eggs, straw. Under the straw, a pencil stub.", "A countrywoman's hands, and a town haircut.", "The camp, to sell eggs. She was here last week. And the week before.", 2, "You take a dozen eggs. She counts the men again while you count the eggs.", "Through, as every week. She's very good with names.", "Turned. She's surprised, and for half a second it shows.");
		Pass("grave", true, "A man with a spade over his shoulder and clay to the knee.", "A spade. A bottle. A list of names, most crossed through.", "Blistered, and they've burst.", "The next one on the list. He digs them for whoever asks.", 1, "You take the bottle. He needed that more than anyone on this road.", "Through. He nods at the pit behind the camp as he passes, a tradesman's nod.", "Turned. The next one on the list stays above ground.");
		Pass("poacher", true, "A wiry man with a bulge under his coat that kicks.", "A hare, alive, and two that aren't.", "A snare wire round his wrist like a bracelet.", "He doesn't say. He's been taking them off our own ground.", 0, "Both dead hares for the bar, and he keeps the live one, and his fingers.", "Through, with our hares.", "Turned. He'll be back in the wood by dark, on our ground, with his wires.");
		Pass("crier", bastionRoad || truceBastion, "A man with a handbell and a paper, reading it aloud as he walks.", "The paper. It's a notice, printed, with Bastion's crest.", "A big voice. Sore feet.", "Every crossroads. He reads it at each one.", 2, "You take the paper. It says the road is Bastion's. He's been reading that at every crossroads to here.", "Through. He reads it at our bar, loudly, on his way under.", "Turned, with his paper. Our road hears no more of it.");
		Pass("orphans", late, "Eleven children in a line, holding a rope, with one grown woman at the front.", "A handcart of blankets. No food at all.", "The woman is asleep on her feet.", "Anywhere with a door that shuts.", 1, "There's nothing to take but blankets. You don't.", "Through. The cook comes out with the big pot before anyone tells her to.", "Turned. Twelve people turn round on a rope.");
		Pass("gravedigger", sick, "A cart with a coffin on it, new wood, and nobody walking behind.", "A coffin. Nailed down. Heavy for its size.", "The carter won't meet your eye.", "The churchyard, he says. Quickly.", 2, "You have it opened. It's tins. A coffin of tins, going out of the valley.", "Through, with respect. A hundredweight of food leaves the valley in a box.", "Turned. He turns the cart so fast the coffin slides.");
		Pass("returner", true, "One of ours, back, a month late, with a different coat and a story.", "Another crew's tobacco. Another crew's knife.", "Our rag, tied on fresh this morning.", "Home, he says. He got lost. For a month.", 2, "You take the knife and send him to the boss. He goes like a man walking to a dentist.", "Through, as if he'd never been away. He goes straight to the cookhouse and listens.", "You don't lift the bar for him. He stands outside it, one of ours, where everyone can see.");
		int pool = RZ_DutyScript.Draw(5, picked);
		foreach (RZ_DutyBeat b : picked)
		{
			scene.m_aBeats.Insert(b);
		}

		int force = -1;
		if (feudBastion)
			force = 3;

		Extension(scene, force);
		RZ_DutyScript.Into(scene.m_aBeats);
		Marked(dealer, e);
		RZ_DutyScript.T("boss", "The boss, come down to count the take. He looks in the box, and then at you.", RZ_DutyBeat.AT_ENTRY);
		RZ_DutyScript.O("Hand him the box", "He counts it with one finger. He grunts. From him, that's a speech.", "regard+");
		RZ_DutyScript.O("Hand it over, one thing short", "One thing stays in your coat. He counts with one finger, and stops, and starts again.", "shady2 give");
		return pool;
	}

	//------------------------------------------------------------------------------------------------
	//! One chance to do more than stand at the bar, for more pay: from four.
	//------------------------------------------------------------------------------------------------
	//! The spotter's slate read: a mark from the hill in the last two season days, and the crew went
	//! out on it. The boss says what came of it before he counts the box, and the mark is used up.
	protected static void Marked(RZ_Errands dealer, RZ_Errand e)
	{
		int today = RZ_SeasonClock.Day();
		array<string> came = {
			"The crew took the three carts after the crossroads, on the slate's mark. Nobody hurt, and the camp eats tonight.",
			"The crew walked a cart past Bastion's barrier at the bell, on the slate's mark, and nobody stopped it.",
			"The crew stopped the Haven's truck on the slow climb past the ford, on the slate's mark. They took a third and let it go.",
			"The crew had the handcart off Halcyon's people at the well, on the slate's mark. Sealed bottles. The boss is pleased and doesn't know why.",
			"The crew went for the hut with the lamp at dusk, on the slate's mark. The other crew are a hut short, and asking who drew the map."};
		for (int t = 0; t < 5; t++)
		{
			string key = string.Format("spotter.%1.%2", e.m_iHolding, t);
			int day = dealer.Counter(key);
			if (day <= 0 || today - day > 2)
				continue;

			RZ_DutyScript.T("marked", came[t], RZ_DutyBeat.AT_ENTRY);
			RZ_DutyScript.O("Hear him out", "He tells it twice, the second time with more in it.", string.Format("morale+ rumour set:%1=0", key), "", came[t]);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! `force` >= 0: that one (a feud with Bastion brings its patrol up the road).
	protected static void Extension(RZ_DutyScene scene, int force)
	{
		int which = Math.RandomInt(0, 4);
		if (force >= 0)
			which = force;

		RZ_DutyScript.Into(scene.m_aBeats);
		if (which == 0)
		{
			RZ_DutyScript.T("escort", "A merchant with a mule and a strongbox. He'll pay double for a man to walk him through to the far marker.", RZ_DutyBeat.AT_ENTRY);
			RZ_DutyScript.O("Walk him through yourself", "You'll walk him. The lads keep the bar. He falls in beside you with the mule.", "flag:walk walker");
			RZ_DutyScript.O("He pays the toll like anyone", "The toll, and no more. He goes on alone, looking at the hedges.");
			RZ_DutyScript.T("marker", "", RZ_DutyBeat.AT_FAR, "walk");
			RZ_DutyScript.O("See him past the far marker", "Past the marker, and nobody stepped out of a hedge. He pays double, and says so at every inn after.", "hold:4 +2 fame1 walker_end");
		}
		else if (which == 1)
		{
			RZ_DutyBeat bolt = RZ_DutyScript.T("bolt", "One of them drops his pack and runs, across the field, toward the trees. Catch him before the trees, alive.", RZ_DutyBeat.AT_OUT);
			bolt.m_fTimeout = 50;
			bolt.m_sPassed = "He's in the trees. Whatever he ran for, he's kept.";
			RZ_DutyScript.O("Bring him down, unhurt", "You have him by the collar, winded, at the wood's edge. In his shirt: a letter for Bastion, about the camp.", "hold:3 +2 fame1 report", "", "", "The one you ran down had a letter on him about us, for Bastion. You could have let him run. You didn't, and you didn't kill him. The boss liked both halves.");
		}
		else if (which == 2)
		{
			RZ_DutyScript.T("follow", "One who paid without a word turns off the road forty yards on, into the wood, where there's no path.", RZ_DutyBeat.AT_ENTRY);
			RZ_DutyScript.O("Follow, and see where they go", "You leave the bar to the lads and go after, quietly.", "flag:follow");
			RZ_DutyScript.O("They paid. Let them go", "Paid is paid. The wood closes behind them.");
			RZ_DutyScript.T("cache", "A hollow under a fallen beech. They've been coming here a while. Tins, in rows, and a list of who gets them.", RZ_DutyBeat.AT_OUT, "follow");
			RZ_DutyScript.O("Tell the boss where it is", "The boss sends four men with sacks. The list is burned.", "report +2 fame1", "", "", "That cache in the beech wood fed the camp for a week. It was feeding eleven families before. I thought you should know both.");
			RZ_DutyScript.O("Leave it, and say they went home", "You pull the branches back over it. You never saw a beech.", "shady2 local", "who keeps the tins under the beech", "Someone found the beech and covered it again. Eleven families ate this week. I've left a tin on your side of the bar. Don't look for me.");
		}
		else
		{
			RZ_DutyScript.T("patrol", "Bastion, coming up the road. Six, in step, a corporal in front. They've seen the bar.", RZ_DutyBeat.AT_ENTRY);
			RZ_DutyScript.O("Lift the bar and stand aside", "They go through without paying, without looking. The lads spit when they've gone.", "morale-");
			RZ_DutyScript.O("Hold the bar, and name the toll", "You name it. The corporal looks at you, then at the lads on the bank he hadn't counted. He pays. In cigarettes.", "+2 fame1 morale+");
			RZ_DutyScript.O("Tell him the road's sick ahead", "You tell him there's fever at the camp. He looks at your face a while. They turn back.", "+1", "", "", "Bastion think we've fever. They've kept off the road for a week. That was your lie, and a good one, until one of us needs their doctor.");
		}
	}

	// ================================================================ Spotter (worked out)
	//------------------------------------------------------------------------------------------------
	//! A hill, a glass, three things seen a while apart, and the pattern you were sent to learn.
	//! One of five things to watch, and one of five things that happens up there.
	static void Spotter(RZ_Errands dealer, RZ_Errand e, RZ_DutyScene scene)
	{
		int target = Math.RandomInt(0, 5);
		array<string> what = {"the road below", "Bastion's post", "the Haven's truck road", "Halcyon's people at the well", "the other crew's camp"};
		array<string> first = {"Carts go by in ones, an hour apart. At noon, three together, close, with a rider ahead.", "Two on the gate. At the bell, four come out of the hut and two go in.", "A truck, slow, loaded. It stops at the ford, and the driver gets down to look at the water.", "Three in white, with cases. They take water at the well, then at the trough, then the ditch.", "Eleven men, six fires. The lookout sits with his back to this hill."};
		array<string> second = {"The three carts have sheets over them, roped. The lone carts don't.", "The two who come off walk to the cookhouse without looking back.", "The far side of the ford it climbs in bottom gear, at a walk, for two hundred yards.", "One stands apart, watching the road, with a rifle he holds like a broom.", "At dusk nine of the eleven go to one hut, and a lamp is lit."};
		array<string> third = {"The rider turns back at the crossroads. After that the three carts are alone.", "For as long as it takes to fill a pipe, there's nobody at the barrier at all.", "Nobody rides guard on the back. The mate sits in the cab with the driver.", "When they've done, the cases go on a handcart, and the man with the rifle pulls it.", "The two left out are at the store hut. One sleeps. The other is looking at the store's door."};
		array<string> right = {"Mark the three carts after the crossroads", "Mark the gate bare at the bell", "Mark the slow climb past the ford", "Mark the handcart, when it leaves", "Mark the hut with the lamp, at dusk"};
		array<string> wrongA = {"Mark the lone carts, every hour", "Mark the cookhouse", "Mark the ford itself", "Mark the well", "Mark the six fires"};
		array<string> wrongB = {"Mark the rider", "Mark the hut at night", "Mark the truck's cab", "Mark the one with the rifle", "Mark the lookout"};
		RZ_DutyScript.Into(scene.m_aBeats);
		RZ_DutyScript.T("climb", string.Format("The hill, and a hollow under gorse with a view of %1.", what[target]), RZ_DutyBeat.AT_FAR);
		RZ_DutyScript.O("Lie up with the glass", first[target], "hold:8");
		Up();
		array<ref RZ_DutyBeat> picked = {};
		RZ_DutyScript.Draw(1, picked);
		foreach (RZ_DutyBeat b : picked)
		{
			scene.m_aBeats.Insert(b);
		}

		RZ_DutyScript.Into(scene.m_aBeats);
		RZ_DutyBeat two = RZ_DutyScript.T("second", "");
		two.m_fWait = 20;
		RZ_DutyScript.O("Watch again", second[target], "hold:8");
		RZ_DutyBeat three = RZ_DutyScript.T("third", "");
		three.m_fWait = 20;
		RZ_DutyScript.O("Watch a third time", third[target], "hold:8");
		string key = string.Format("spotter.%1.%2", e.m_iHolding, target);
		RZ_DutyScript.T("board", "The slate. One line is all the boss reads. What's the thing the crew can use?");
		RZ_DutyScript.O(right[target], "Marked. Down the hill, and onto the camp's board, in chalk.", "write right record rumour set:" + key + "=" + RZ_SeasonClock.Day().ToString(), "", string.Format("The Red Hand have had a man on the hill above %1. They know when to come.", what[target]), "Your line is on the board. The crew goes out on it tomorrow. If it's right, there'll be meat. If it's wrong, I know whose chalk it was.");
		RZ_DutyScript.O(wrongA[target], "Marked. It's true, as far as it goes. It isn't what they can use.", "write wrong");
		RZ_DutyScript.O(wrongB[target], "Marked. The boss will read it, and ask what he's meant to do with it.", "write wrong");
	}

	//------------------------------------------------------------------------------------------------
	//! One thing that happens up on the hill, from five. Into the script's pool.
	protected static void Up()
	{
		RZ_DutyScript.Pool();
		RZ_DutyScript.G("glint");
		RZ_DutyBeat still = RZ_DutyScript.T("glint", "The sun's come round. Your glass has flashed. Down there, a head turns, and a hand points. Lie still, and they'll doubt it.");
		still.m_fTimeout = 20;
		still.m_sPassed = "You don't move. A minute, by your own breathing. The hand comes down. They go back to what they were doing.";
		RZ_DutyScript.O("Get up and run", "You break cover, and now they're sure. Two of them start up the hill.", "wrong heat -1");
		RZ_DutyScript.G("weather");
		RZ_DutyScript.T("weather", "Cloud is coming down the valley like a lid. In a quarter hour you won't see your own hand.");
		RZ_DutyScript.O("Stay, and watch what you can", "You stay. Shapes, in grey. You'll have to guess at some of it.", "hold:6");
		RZ_DutyScript.O("Move down to the hedge, closer", "A hundred yards nearer, flat in the hedge bottom. You can hear them talk.", "hold:10 +1");
		RZ_DutyScript.G("other");
		RZ_DutyScript.T("other", "Somebody else is coming up your hill, bent low, with a glass of their own. A blue rag. Not ours.");
		RZ_DutyScript.O("Let them see you first", "A long look, across thirty yards of gorse. A nod. You take one side of the hollow each, like men fishing.", "local", "who had the other side of the hollow on the hill", "We lay in the same gorse for an hour and never said a word. I marked my slate wrong on purpose, for mine. I hope you marked yours right.");
		RZ_DutyScript.O("See them off the hill", "A stone, thrown short. They go down faster than they came up. Their crew will know the hill's taken.", "heat");
		RZ_DutyScript.G("shouldnt");
		RZ_DutyScript.T("shouldnt", "Something you weren't sent to see. One of ours, down there, handing a package to one of theirs. You know his walk.");
		RZ_DutyScript.O("Put it on the slate", "On the slate, with the hour. The boss will know the walk too.", "report +1", "", "", "The man you saw from the hill has been asked about the package. He's not been seen since. You'll be wondering if you should have. So am I.");
		RZ_DutyScript.O("Keep it off the slate", "It isn't what you were sent for. You watch him walk back to our side.", "shady1 later", "", "You saw me from the hill. I saw the glass. It was my sister's medicine in that package. I'm still here because of what you didn't write.");
		RZ_DutyScript.G("known");
		RZ_DutyScript.T("known", "Somebody you know is walking down there, straight toward what the crew has planned. They gave you soup once.");
		RZ_DutyScript.O("Stand up and wave them off", "You stand, against the sky, and wave both arms. They stop, and turn. So does everyone else down there.", "shady2 local heat", "who was waved off from the hill", "A man stood up on the hill and waved me back, and I went, and an hour later I heard what happened on that road. I know it was you.");
		RZ_DutyScript.O("Stay down", "You stay down. It isn't planned for today. You tell yourself that.", "later", "", "I walked the low road the day you were on the hill. Nothing happened to me. I've been told since how near it was. You were up there the whole time.");
	}
}
