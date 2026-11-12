//! Openings' words and tables (docs/DESIGN.md, "Openings"): the kinds of place each background
//! starts at, what the spawn screen and the spawn card call them, why today and what the scene gives
//! you to read, who's outside and who they were to you, and the lines the chain's steps are said in. The contact's conversation (their
//! greeting and the story told on the way) is dialogue/openings/openings.rzd. Voice:
//! docs/WRITING.md.
class RZ_OpeningText
{
	//------------------------------------------------------------------------------------------------
	//! The kinds of place a background starts at, the likeliest first (RZ_OpeningRoll.KindOf).
	static void KindsFor(string bg, notnull array<string> result)
	{
		switch (bg)
		{
			case "nurse": result.Insert("clinic"); result.Insert("house"); return;
			case "doctor": result.Insert("clinic"); return;
			case "mechanic": result.Insert("garage"); result.Insert("fuel"); result.Insert("farm"); return;
			case "hunter": result.Insert("farm"); return;
			case "police": result.Insert("police"); result.Insert("public"); return;
			case "excon": result.Insert("garage"); result.Insert("farm"); return;
			case "labtech": result.Insert("school"); result.Insert("clinic"); return;
			case "radioham": result.Insert("radio"); result.Insert("house"); return;
			case "unemployed": result.Insert("house"); result.Insert("shop"); return;
			case "soldier": result.Insert("military"); result.Insert("public"); return;
			case "raider": result.Insert("house"); return;
			case "elder": result.Insert("house"); return;
		}

		// The Drifter, and any background not listed: anywhere at all
		result.Insert("house");
		result.Insert("shop");
		result.Insert("farm");
		result.Insert("garage");
		result.Insert("public");
	}

	//------------------------------------------------------------------------------------------------
	//! The spawn screen's Start line for a background: only the kind of place, never where.
	static string ScreenPhrase(string bg)
	{
		switch (bg)
		{
			case "nurse": return "A clinic, or a house near one";
			case "doctor": return "A back room at a clinic";
			case "mechanic": return "A garage, a pump or a farm shed";
			case "hunter": return "A farm at the edge of things";
			case "police": return "A police post or a town hall";
			case "excon": return "A lock-up or a shed";
			case "labtech": return "A school lab or a clinic";
			case "radioham": return "Somewhere under an aerial";
			case "unemployed": return "A house or a shop";
			case "soldier": return "An old barracks, or a hall";
			case "raider": return "A stash house";
			case "elder": return "A house on a square";
		}

		return "Anywhere at all";
	}

	//------------------------------------------------------------------------------------------------
	//! Where you hid, for the spawn card: "A cellar", "The pit under a garage".
	static string HidingPlace(string kind)
	{
		switch (kind)
		{
			case "clinic": return "A back room at the clinic";
			case "shop": return "A stockroom behind a shop";
			case "garage": return "The pit under a garage";
			case "farm": return "A farmhouse cellar";
			case "public": return "A store room off an empty hall";
			case "police": return "The cells at the police post";
			case "radio": return "A hut under the radio mast";
			case "fuel": return "The back office of a petrol station";
			case "school": return "The science room at the school";
			case "military": return "A store room at the old barracks";
		}

		return "A cellar";
	}

	//------------------------------------------------------------------------------------------------
	//! The spawn card's line, by why today: "A farmhouse cellar in Morton. Day 11. The last bottle is
	//! empty, so you open the door." Without a scene (no door found), only the town and the day.
	static string SpawnLine(int reason, string kind, bool scene, string town, int day)
	{
		if (!scene)
		{
			switch (reason)
			{
				case RZ_EOpeningReason.GROUP: return string.Format("Somewhere outside %1. Day %2. The others went on ahead three days ago.", town, day);
				case RZ_EOpeningReason.FEVER: return string.Format("Somewhere outside %1. Day %2. The fever has gone, and so has everyone who sat with you.", town, day);
			}

			return string.Format("Somewhere outside %1. Day %2. Somebody's calling your name.", town, day);
		}

		switch (reason)
		{
			case RZ_EOpeningReason.GROUP: return string.Format("%1 in %2. Day %3. Four bedrolls, one of them yours. They said back by dark, three days ago.", HidingPlace(kind), town, day);
			case RZ_EOpeningReason.FEVER: return string.Format("%1 in %2. Day %3. The fever broke in the night. The bite on your arm is only a scar.", HidingPlace(kind), town, day);
		}

		return string.Format("%1 in %2. Day %3. The last bottle is empty, so you open the door.", HidingPlace(kind), town, day);
	}

	//------------------------------------------------------------------------------------------------
	//! The calendar on the step: the days crossed off since the gates closed.
	static string Calendar(int day)
	{
		return string.Format("This month's page. Every day up to Day %1 is crossed off in pencil, one stroke a morning. Day 2 has a ring round it and two words: ROADS SHUT. The last few strokes shake.", day);
	}

	//------------------------------------------------------------------------------------------------
	//! What the scene gives you to read, by why today: its title (also the giver when nobody's outside).
	static string ReadTitle(int reason)
	{
		switch (reason)
		{
			case RZ_EOpeningReason.GROUP: return "The note";
			case RZ_EOpeningReason.FEVER: return "The door";
		}

		return "The calendar";
	}

	//------------------------------------------------------------------------------------------------
	static string ReadLabel(int reason)
	{
		switch (reason)
		{
			case RZ_EOpeningReason.GROUP: return "Read the note";
			case RZ_EOpeningReason.FEVER: return "Read the door";
		}

		return "Read the calendar";
	}

	//------------------------------------------------------------------------------------------------
	//! The read itself: the calendar, the group's note with the town they went to circled, the words
	//! on the door. `brief`: a veteran's one line.
	static string Read(int reason, int day, string circled, bool brief)
	{
		switch (reason)
		{
			case RZ_EOpeningReason.GROUP:
			{
				if (brief)
					return "A note under a stone: \"Back by dark.\" That was three days ago.";

				return string.Format("A page torn from a school exercise book, under a stone: \"Gone to see about %1. Back by dark. Don't eat the peaches.\" Under it, a map of the district, %1 circled twice. Three days of dark since.", circled);
			}

			case RZ_EOpeningReason.FEVER:
			{
				if (brief)
					return "Chalk on the door: IF YOU'RE READING THIS, YOU'RE YOU.";

				return "Chalk on the inside of the door, in big careful capitals: IF YOU'RE READING THIS, YOU'RE YOU. COME FIND US. Under it, four strokes, one for each morning somebody checked.";
			}
		}

		if (brief)
			return string.Format("The calendar, crossed off up to Day %1.", day);

		return Calendar(day);
	}

	//------------------------------------------------------------------------------------------------
	//! Who the contact was to you, by your background (the design's someone from before): a few
	//! words after their name in their title ("Vera Holm, a patient's mother").
	static string Who(string bg)
	{
		switch (bg)
		{
			case "nurse": return "a patient's mother";
			case "doctor": return "a nurse from your ward";
			case "mechanic": return "a regular at the garage";
			case "hunter": return "the farmer whose land you hunted";
			case "police": return "a shopkeeper on your beat";
			case "excon": return "your old cellmate";
			case "labtech": return "a junior from the institute";
			case "radioham": return "a voice from the night band";
			case "unemployed": return "the neighbour who lent you money";
			case "soldier": return "a private from your unit";
			case "raider": return "someone you robbed";
			case "elder": return "a mother from your street";
		}

		return "a face you half know";
	}

	//------------------------------------------------------------------------------------------------
	//! A contact's name from the season's names (none of LORE's people).
	static string Name(RZ_Rng rng)
	{
		array<string> first = {"Vera", "Pavel", "Henri", "Jana", "Lucie", "Emil", "Ruth", "Anton", "Irena", "Elise",
			"Karel", "Nadia", "Oskar", "Milan", "Sabine", "Jakub", "Ilse", "Bruno", "Dana", "Leon", "Agata", "Petra"};
		array<string> last = {"Holm", "Novak", "Brandt", "Kowal", "Duval", "Mercer", "Horak", "Petit", "Vogel", "Sorel",
			"Kral", "Weber", "Dvorak", "Benes", "Fournier", "Hale", "Moreau", "Lindqvist"};
		return rng.Pick(first) + " " + rng.Pick(last);
	}

	//------------------------------------------------------------------------------------------------
	//! A stranger, by why today: who heard the door, who saw them go, who wrote on it.
	static string StrangerWho(int reason)
	{
		switch (reason)
		{
			case RZ_EOpeningReason.GROUP: return "someone who saw them go";
			case RZ_EOpeningReason.FEVER: return "the stranger who wrote on the door";
		}

		return "a stranger who heard the door";
	}

	//------------------------------------------------------------------------------------------------
	//! A wanderer who met an earlier life of yours: "a trader who thinks they know you".
	static string EarlierWho(string role)
	{
		return "a " + role + " who thinks they know you";
	}

	//------------------------------------------------------------------------------------------------
	//! Their first line when the talk screen has no tree for them (Dialogue > Trees off), by why
	//! today and who they are.
	static string GreetPlain(int reason, int who)
	{
		if (who == RZ_EOpeningWho.EARLIER)
			return "\"You look like someone I knew. Same walk. Never mind. Come here, let me look at you.\"";

		if (who == RZ_EOpeningWho.STRANGER)
		{
			switch (reason)
			{
				case RZ_EOpeningReason.GROUP: return "\"You were with the four who went out. I saw them go. I never saw them come home.\"";
				case RZ_EOpeningReason.FEVER: return "\"I'm the one who wrote on your door. I didn't know your name. I sat out here anyway.\"";
			}

			return "\"I heard the door. Sorry. I don't know you, but you look like you could use someone.\"";
		}

		switch (reason)
		{
			case RZ_EOpeningReason.GROUP: return "\"You're still here. I came back for you. The others didn't come back at all.\"";
			case RZ_EOpeningReason.FEVER: return "\"It's you. It's really you. I wrote that on the door and then I couldn't bear to look.\"";
		}

		return "\"Look at you. Alive. I'd stopped knocking on that door.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! The story told on the way (the design's beats), one piece as each step starts, in the
	//! contact's voice: step 1 the seal; step 2 the broadcast, and nobody coming; step 3 the
	//! Perimeter, and the cure. With Dialogue > Trees on, the topics in
	//! dialogue/openings/openings.rzd tell more of it.
	static void Beat(int step, int day, notnull array<string> lines)
	{
		if (step <= 1)
		{
			lines.Insert(string.Format("\"Day %1. They shut the roads on the second, and nobody's opened one since. I counted. Somebody had to.\"", day));
			return;
		}

		if (step == 2)
		{
			lines.Insert("\"Listen. The radio's still on the same tape: aid is on its way. Every hour, the same voice. I used to wait for it.\"");
			lines.Insert("\"Nobody's coming. Say it out loud once. It helps, I promise.\"");
			return;
		}

		lines.Insert("\"My brother tried the river road. The loudspeaker said his name. Then it didn't.\"");
		lines.Insert("\"The lab people say there's a way out. A cure. Proof, they call it. It won't come to us in here, so we go to it.\"");
	}

	//------------------------------------------------------------------------------------------------
	//! Anyone else who talks to the contact.
	static string NotYou()
	{
		return "\"Sorry. I'm waiting for someone. Give us a minute.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! "What was it you needed?": the step again, short. `search`: step 1's place.
	static string AskAgain(int step, int task, string search)
	{
		if (step == 1)
		{
			if (search.IsEmpty())
				search = "the house over the way";

			return string.Format("\"%1. Look, and come back. Walk, don't run.\"", Capital(search));
		}

		if (step == 2)
		{
			switch (task)
			{
				case RZ_EOpeningTask.WATER: return "\"Water. The well. Whatever you can carry.\"";
				case RZ_EOpeningTask.CUT: return "\"My hand. Anything clean you've got.\"";
				case RZ_EOpeningTask.FEVER: return "\"The boy. Whatever's in your bag.\"";
				case RZ_EOpeningTask.BOILED: return "\"Water for the fever. I've got the fire going.\"";
				case RZ_EOpeningTask.PUMP: return "\"A can of fuel from the pump. Just the one.\"";
				case RZ_EOpeningTask.DOWNWIND: return "\"Watch where they go about. Don't let them see you.\"";
				case RZ_EOpeningTask.KEYS: return "\"The locked door. You've got the key now.\"";
				case RZ_EOpeningTask.TOLL: return "\"The toll. Talk, look, come back. No shooting.\"";
				case RZ_EOpeningTask.SAMPLE: return "\"The sample. By the remains, down the road.\"";
				case RZ_EOpeningTask.CHANNEL: return "\"Call on the open channel. Somebody's asking.\"";
				case RZ_EOpeningTask.SWAP: return "\"Catch the one on the road. See what they'll swap.\"";
				case RZ_EOpeningTask.GATE: return "\"The gate. One minute.\"";
				case RZ_EOpeningTask.CLOCK: return "\"The antidote. Now, while it still works.\"";
				case RZ_EOpeningTask.CLOCK_MED: return "\"Keep him going. A dressing, anything.\"";
				case RZ_EOpeningTask.DEBT: return "\"The house that owes. Ask them.\"";
				case RZ_EOpeningTask.SETTLE: return "\"The two families. Sit with me and hear it.\"";
			}
		}

		return "\"Go on. Don't wait for me. I'm slower than I look.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! The Drifter's next ask, said as it starts (asks 2 and 3).
	static string NextAsk(int asksDone)
	{
		if (asksDone == 1)
			return "\"That's one. The woman at the end house heard you were about. She's got a thing too.\"";

		return "\"Last one, I promise. The man with the cart asked. Everyone asks, once they've seen you'll do it.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! The clock ran out.
	static string ClockLost()
	{
		return "\"It's taken him. I stayed till it did. Don't come back this way: I'll see to it.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! The two families settled only halfway (the check came up short).
	static string SettledHalf()
	{
		return "\"Half a peace. They'll take turns at the well and glare the whole time. Better than knives.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! Step 1's building kinds by background (RZ_OpeningRoll.KindOf), when one is near the contact.
	static void SearchKinds(string bg, notnull array<string> result)
	{
		switch (bg)
		{
			case "nurse": result.Insert("clinic"); return;
			case "doctor": result.Insert("clinic"); return;
			case "mechanic": result.Insert("garage"); result.Insert("fuel"); return;
			case "hunter": result.Insert("farm"); return;
			case "police": result.Insert("police"); result.Insert("public"); return;
			case "labtech": result.Insert("school"); result.Insert("clinic"); return;
			case "radioham": result.Insert("radio"); return;
			case "soldier": result.Insert("military"); result.Insert("public"); return;
			case "unemployed": result.Insert("shop"); return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! What step 1 calls its building: "the chemist's over the way".
	static string SearchPlace(string kind)
	{
		switch (kind)
		{
			case "clinic": return "the chemist's over the way";
			case "garage": return "the garage over the way";
			case "fuel": return "the petrol station over the way";
			case "farm": return "the farm over the way";
			case "police": return "the police post over the way";
			case "public": return "the hall over the way";
			case "school": return "the school over the way";
			case "radio": return "the hut under the mast";
			case "military": return "the old barracks over the way";
			case "shop": return "the shop over the way";
		}

		return "the house over the way";
	}

	//------------------------------------------------------------------------------------------------
	protected static string Capital(string text)
	{
		return RZ_Util.Capitalize(text);
	}

	//------------------------------------------------------------------------------------------------
	//! Whoever finds you after the contact is killed.
	static string ContactDied(string town)
	{
		return string.Format("\"Your friend's gone. I saw. Don't stand there. There's a safehouse in %1. They take people in.\"", town);
	}

	//------------------------------------------------------------------------------------------------
	//! The last step's title by the faction it ends with.
	static string WalkTitle(int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.MILITARY: return "To Bastion";
			case RZ_EFaction.BANDITS: return "To the Red Hand";
			case RZ_EFaction.INSTITUTE: return "To Halcyon";
		}

		return "To the Haven";
	}

	//------------------------------------------------------------------------------------------------
	//! The place the last step names: "the safehouse in Morton".
	static string EndPlace(int faction, string town, bool lab)
	{
		switch (faction)
		{
			case RZ_EFaction.MILITARY: return "the Bastion post at " + town;
			case RZ_EFaction.BANDITS: return "the Red Hand's camp at " + town;
			case RZ_EFaction.INSTITUTE:
			{
				if (lab)
					return "Halcyon's lab at " + town;

				return "Halcyon's field station at " + town;
			}
		}

		return "the safehouse in " + town;
	}

	//------------------------------------------------------------------------------------------------
	//! The contact sends you on: the faction, in a sentence (the design's story beat).
	static string SendOff(int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.MILITARY: return "\"Go to {place}. The soldiers count you at the door. Let them. It's how they know you're real.\"";
			case RZ_EFaction.BANDITS: return "\"Try {place}. They'll want to know what you're worth. Tell them, but not all of it.\"";
			case RZ_EFaction.INSTITUTE: return "\"Go to {place}. The lab people say there's a way out. A cure. They'll want hands like yours.\"";
		}

		return "\"Get yourself to {place}. They feed you if you work. Go on. I'll follow when it's dark.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! The lead's title (levels 2-3: the nearest place with work).
	static string LeadTitle()
	{
		return "Where the Work Is";
	}

	//------------------------------------------------------------------------------------------------
	//! What you find at the place with work, by whose it is.
	static string LeadArrived(int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.MILITARY: return "A corporal runs a finger down a clipboard. \"Work detail at dawn. Be on time, or be somewhere else.\"";
			case RZ_EFaction.BANDITS: return "Someone counting tins nods at you without stopping. \"Work? Always. It pays in tins.\"";
			case RZ_EFaction.INSTITUTE: return "A woman in a mask points you at a board of chores. \"Pick one. Wash first.\"";
		}

		return "Someone with a list looks up. \"Hands? Good. There's always something that needs carrying.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! What you find when you get there, plain.
	static string Arrived(int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.MILITARY: return "A soldier at the wire takes your name and writes it down, slowly, in capitals. \"Next.\"";
			case RZ_EFaction.BANDITS: return "Someone on a crate looks you up and down. \"New face. The book likes a new face.\"";
			case RZ_EFaction.INSTITUTE: return "A woman in a mask looks in your eyes with a torch, then nods you through. \"Wash your hands.\"";
		}

		return "Someone at the gate looks you over and steps aside. \"You'll want soup. Everyone wants soup.\"";
	}
}
