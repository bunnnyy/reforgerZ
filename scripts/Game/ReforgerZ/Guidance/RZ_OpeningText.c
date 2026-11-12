//! Openings' words and tables (docs/DESIGN.md, "Openings"): the kinds of place each background
//! starts at, what the spawn screen and the spawn card call them, who the contact was to you, the
//! calendar, and the lines the chain's steps are said in. The contact's conversation (their
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
	//! The spawn card's line: "A cellar in Morton. Day 11. The last bottle is empty, so you open the
	//! door." Without a scene (no door found), only the town and the day.
	static string SpawnLine(string kind, bool scene, string town, int day)
	{
		if (!scene)
			return string.Format("Somewhere outside %1. Day %2. Somebody's calling your name.", town, day);

		return string.Format("%1 in %2. Day %3. The last bottle is empty, so you open the door.", HidingPlace(kind), town, day);
	}

	//------------------------------------------------------------------------------------------------
	//! The calendar on the step: the days crossed off since the gates closed.
	static string Calendar(int day)
	{
		return string.Format("This month's page. Every day up to Day %1 is crossed off in pencil, one stroke a morning. Day 2 has a ring round it and two words: ROADS SHUT. The last few strokes shake.", day);
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
	//! The contact's line when the talk screen has no tree for them (Dialogue > Trees off).
	static string GreetPlain()
	{
		return "\"Christ. You're alive. I'd stopped knocking on that door.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! Anyone else who talks to the contact.
	static string NotYou()
	{
		return "\"Sorry. I'm waiting for someone. Give us a minute.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! "What was it you needed?": the step again, short.
	static string AskAgain(int step, bool water)
	{
		if (step == 1)
			return "\"The house over the way. Look, and come back. Walk, don't run.\"";

		if (step == 2 && water)
			return "\"Water. The well. Whatever you can carry.\"";

		if (step == 2)
			return "\"My hand. Anything clean you've got.\"";

		return "\"Go on. Don't wait for me. I'm slower than I look.\"";
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
