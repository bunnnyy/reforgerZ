//! What the world says to new players: spawn scenario notes, the open radio channels,
//! Bastion's loudspeakers, Red Hand taunts, Halcyon's flyers and wanderer gossip.
//! Written to the voices in docs/LORE.md: Bastion clipped and tired, the Haven warm and
//! worried, the Red Hand mocking, Halcyon precise and a little guilty.
class RZ_GuidanceText
{
	// ================================================================ Spawn scenarios
	//------------------------------------------------------------------------------------------------
	static string ScenarioTitle(int scenario)
	{
		switch (scenario)
		{
			case RZ_EGuideScenario.CAR: return "A back road";
			case RZ_EGuideScenario.BASEMENT: return "A basement";
			case RZ_EGuideScenario.CHURCH: return "Church notice";
			case RZ_EGuideScenario.FARMHOUSE: return "A farmhouse kitchen";
			case RZ_EGuideScenario.ATTIC: return "An attic";
			case RZ_EGuideScenario.BARRICADE: return "Behind the barricade";
		}

		return "Where you woke";
	}

	//------------------------------------------------------------------------------------------------
	//! Where you wake. `brief` = a veteran: one line.
	static string Scene(int scenario, string town, string extra, bool brief)
	{
		switch (scenario)
		{
			case RZ_EGuideScenario.CAR:
				if (brief)
					return string.Format("A car in the ditch outside %1. A note behind the wiper.", town);

				return string.Format("You come to on the verge of a back road outside %1, next to a car with its nose in the ditch. The doors are open. Someone left a note on the dashboard.", town);
			case RZ_EGuideScenario.BASEMENT:
				if (brief)
					return string.Format("A basement in %1. Scratches on the back of the door.", town);

				return string.Format("You wake on the floor of a cold basement in %1. Light comes in under the door. Something has been scratched into the back of it.", town);
			case RZ_EGuideScenario.CHURCH:
				if (!extra.IsEmpty())
				{
					if (brief)
						return string.Format("%1. A flyer on the church door, %2.", town, extra);

					return string.Format("You come to in %1 to the sound of a church door banging in the wind, %2. Something white is nailed to it, flapping.", town, extra);
				}

				if (brief)
					return string.Format("%1. A flyer pinned to a post.", town);

				return string.Format("You come to in %1. Someone has pinned a flyer to the post beside you, over an old notice for a church fete.", town);
			case RZ_EGuideScenario.FARMHOUSE:
				if (brief)
					return string.Format("A farmhouse kitchen in %1. A wind-up radio on the table, still talking.", town);

				return string.Format("A farmhouse kitchen on the edge of %1. Plates still on the table, a chair on its back. Next to the plates, a wind-up radio is talking to itself:\n%2", town, extra);
			case RZ_EGuideScenario.ATTIC:
				if (brief)
					return string.Format("An attic in %1. A dead man by the skylight.", town);

				return string.Format("You wake in an attic in %1, under a skylight thick with dust. You're not alone up here. He's been dead a while.", town);
			case RZ_EGuideScenario.BARRICADE:
				if (brief)
					return string.Format("Behind a Bastion barricade in %1. A dead soldier.", town);

				return string.Format("You come to behind a barricade of sandbags and an overturned car at the edge of %1. A soldier sits against it, very still, a folded paper in his breast pocket.", town);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Crashed car: the dashboard note points at the nearest Haven safehouse.
	static string CarNote(RZ_Rng rng, string safehouseTown, string direction)
	{
		string writer = RZ_NoteText.FirstName(rng);
		string waiting = RZ_NoteText.FirstName(rng);
		return string.Format("\"Gone to the safehouse in %1. They're sharing food. Didn't want to wait for the car, didn't want to wait for anything. %2, if it's you: the keys are in it, the battery isn't. - %3\"\nOn the back, a rough arrow pointing %4.", safehouseTown, waiting, writer, direction);
	}

	//------------------------------------------------------------------------------------------------
	//! Basement door: the roads out are closed, the line shoots. `roadDir` is the nearest
	//! checkpoint's direction ("" if none), `edgeDir` the nearest edge's.
	static string DoorText(RZ_Rng rng, int day, string roadDir, string edgeDir)
	{
		string name = RZ_NoteText.FirstName(rng);
		string text = "Letters gouged with a nail, deep, gone over twice:\nTHE ROADS OUT END IN CONCRETE. THEY DON'T TALK. THEY SHOOT.";
		if (!roadDir.IsEmpty())
			text += string.Format("\nWE TRIED THE %1 ROAD. ONLY I CAME BACK.", roadDir);
		else if (!edgeDir.IsEmpty())
			text += string.Format("\nWE TRIED GOING %1. ONLY I CAME BACK.", edgeDir);

		text += "\nIF YOU HEAR A DRONE YOU'RE TOO CLOSE. NOBODY GETS OUT.";
		string dayText = RZ_NoteText.Day(day);
		dayText.ToUpper();	// in place, like ToLower() in RZ_LandmarkFinder
		name.ToUpper();
		text += string.Format("\n%1. %2", dayText, name);
		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! Halcyon's flyer. Precise, a little cold, and sorry. `labTown` "" = no lab.
	static string FlyerText(string labTown, bool inSafehouse)
	{
		string text = "HALCYON FIELD RESEARCH\nInfected samples wanted. Reward.\n";
		text += string.Format("Tissue from the changed ones (the large ones, the fast ones, the loud ones) is needed for research toward a cure. Deliver samples to the Halcyon lab in %1.\n", labTown);
		if (inSafehouse)
			text += "Bites are treated at the Haven clinic in this house if they are declared. Declare them. Bastion curfew is in force on all roads from dusk.\nSomeone has written underneath, in pencil: \"Reward my foot. Still. Go.\"";
		else
			text += "Do not treat bites yourself: report them to a Haven clinic. Observe Bastion curfew.\nWe are sorry for what this has cost you.";

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! The dead raider's list of toll roads, pointing at the Red Hand boss.
	static string RaiderText(string tollA, string tollB, string bossPlace, string boss)
	{
		return string.Format("Good boots, a better jacket. Somebody has painted a red hand across the back of it. In the inside pocket, a list in pencil:\n\"TOLLS. Road past %1: two tins. Crossroads at %2: whatever they've got. Bastion trucks: everything.\nBoss is in %3. Ask for %4. Don't ask twice.\"", tollA, tollB, bossPlace, boss);
	}

	//------------------------------------------------------------------------------------------------
	//! The dead soldier's orders: hold the line, await relief. Nobody came. Day 2's order had no
	//! "Task Force Bastion" on it: the name came on the radio the night of Day 3 (docs/LORE.md).
	static string OrdersText(RZ_Rng rng, int day, string fellBackTo)
	{
		string name = RZ_NoteText.Surname(rng);
		return string.Format("CORDON DETAIL. Day 2, evening. To all units inside the line: Hold the line. Await relief.\nUnderneath, in pencil, the writing smaller each time:\n\"Day 5. Holding.\" \"Day 8. Holding. Radio quiet.\" \"%1. No relief. Nobody is coming. The others fell back to %2. Tell them %3 held.\"", RZ_NoteText.Day(day), fellBackTo, name);
	}

	// ================================================================ Open radio channels
	//------------------------------------------------------------------------------------------------
	static string HavenChannel(RZ_Rng rng, string town)
	{
		array<string> lines = {
			"\"If you can hear this, %1 is open. Bring water if you can. Bring yourself either way.\"",
			"\"This is the Haven, on the open channel. %1 has beds and soup. Nobody shoots in the house. If you're bitten, say so at the door, and we'll still help.\"",
			"\"Haven, open channel. Still here, still in %1. If you're out there on your own: you don't have to be.\""
		};

		return string.Format(rng.Pick(lines), town);
	}

	//------------------------------------------------------------------------------------------------
	static string BastionChatter(RZ_Rng rng, string havenTown)
	{
		array<string> lines = {
			"\"All stations, Bastion. Curfew at dusk remains in effect. Civilians to the Haven safehouse in %1. Armed groups on the roads. Bastion out.\"",
			"\"Bastion to Command. Requesting status of relief. Over.\" A long hiss. \"Bastion to Command. Over.\" Nothing. \"...Bastion out.\"",
			"\"Patrol two, Bastion. Report. ...Patrol two, Bastion.\" Someone holds the key down for a long moment without saying anything."
		};

		return string.Format(rng.Pick(lines), havenTown);
	}

	//------------------------------------------------------------------------------------------------
	static string RedHandChatter(RZ_Rng rng, string boss, string place)
	{
		array<string> lines = {
			"\"Bastion, Bastion, come in. Still waiting for relief? We'll keep the kettle on.\" Laughter, and the channel goes dead.",
			"\"Toll's due on every road out of %2. Tins, fuel or something worth knowing. Mostly tins.\""
		};

		// Nobody buys samples once the cure is out
		if (!RZ_WorldFacts.Get().IsCured())
			lines.Insert("\"Anyone with samples from the big ones: we pay faster than Halcyon. Ask for %1 in %2. No questions. Well. Some questions.\"");

		return string.Format(rng.Pick(lines), boss, place);
	}

	// ================================================================ Heard near faction seats
	//------------------------------------------------------------------------------------------------
	static string BastionLoudspeaker(RZ_Rng rng, string havenTown)
	{
		array<string> lines = {
			"From somewhere ahead, a loudspeaker: \"Attention. This is a Task Force Bastion position. Curfew is in force from dusk. Civilians requiring shelter: the Haven safehouse in %1. Armed groups are active on the roads. Approach on foot, weapons slung.\"",
			"A loudspeaker crackles from the Bastion post: \"Civilians are reminded that relief is on its way. Report infected sightings. Do not approach the quarantine line. Shelter and food at the Haven safehouse in %1.\" The man reading it has stopped believing it."
		};

		return string.Format(rng.Pick(lines), havenTown);
	}

	//------------------------------------------------------------------------------------------------
	static string RedHandShout(RZ_Rng rng, string boss)
	{
		array<string> lines = {
			"A whistle from a window above you. \"New face! Tolls are fair, %1 is fairer, and Bastion's on a radio nobody answers. Come and say hello.\""
		};

		if (RZ_WorldFacts.Get().IsCured())
			lines.Insert("Someone on a rooftop calls down, laughing: \"Cure's out and you're still here? Gates won't open till they're ready, friend. Ask for %1 in the meantime.\"");
		else
			lines.Insert("Someone on a rooftop calls down, laughing: \"Lost, are we? Bastion only looks after its own, you know. We look after anyone who pays. Got samples? We pay faster than Halcyon. Ask for %1.\"");

		return string.Format(rng.Pick(lines), boss);
	}

	// ================================================================ Wanderer gossip
	//------------------------------------------------------------------------------------------------
	//! One line about a faction and where it sits. The Deserter has his own view of Bastion.
	static string Gossip(RZ_Rng rng, int role, RZ_EFaction faction, string place)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS:
				return string.Format("\"The Haven's holding out in %1. Soup, a medic, a bed if you're lucky. Don't fire a gun near the door.\"", place);
			case RZ_EFaction.MILITARY:
				if (role == RZ_EWandererRole.DESERTER)
					return RZ_WorldFacts.Get().Fill(string.Format("\"Bastion's dug in at %1. Every night they read the roster out loud. When they get to my name, nobody answers. Don't tell them where you saw me.\"", place));

				return string.Format("\"Bastion holds %1. Follow their rules and they'll leave you alone. Their rules change.\"", place);
			case RZ_EFaction.BANDITS:
				return string.Format("\"The Red Hand runs %1 these days. They'll sell you anything, including your own boots back.\"", place);
			case RZ_EFaction.INSTITUTE:
			{
				if (RZ_WorldFacts.Get().IsCured())
					return string.Format("\"The scientists in %1 got their cure out. Now they wait for the gates like the rest of us.\"", place);

				return string.Format("\"The scientists are in %1. They want samples from the big ones. Say it's for a cure. I hope they're right.\"", place);
			}
		}

		return "";
	}
}
