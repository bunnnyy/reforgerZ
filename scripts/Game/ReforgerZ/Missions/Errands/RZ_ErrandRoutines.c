//! The routines (docs/ERRANDS.md, "The routines"): the errands that repeat, on cooldowns (per
//! player per place a real day, per player anywhere 3 real hours, per place 20 minutes). One row
//! each: Define() switches on the key; Why(), Still() and Resolve() read the world the row names.
//! 24 at the four holding kinds, and the wanderers' asks. Each is true because the world says so:
//! a house short of food, a post's relief missing, a station's sensor dark.
//! Server only.

//------------------------------------------------------------------------------------------------
class RZ_ErrandRoutine : RZ_ErrandDef
{
	// Effects (RZ_ErrandWay.m_iEffect)
	static const int FX_NONE = 0;
	static const int FX_FOOD = 1;			// +2 food at the house
	static const int FX_FIX = 2;			// clears the routine's fault
	static const int FX_FIX_FOOD = 3;		// clears the fault and +2 food (a water run)
	static const int FX_MORALE = 4;			// the giver faction's morale +1 (contracts and errands lane)
	static const int FX_RECORD = 5;			// a world record about the giver (the count cooked, the name read)
	static const int FX_HUNTERS = 6;		// Wanted Poster: he sells where you are
	static const int FX_FAME_FIX = 7;		// clears the fault (Halcyon's freezers)

	protected int m_iFault;					// RZ_EFault the routine answers, 0 none

	//------------------------------------------------------------------------------------------------
	void RZ_ErrandRoutine(string key)
	{
		m_sKey = key;
	}

	//------------------------------------------------------------------------------------------------
	//! Every routine, in order.
	static void Build(notnull array<ref RZ_ErrandDef> list)
	{
		array<string> keys = {
			"pot", "lights", "water", "filter_parts", "gate_dusk", "burying", "round", "newcomer", "childrens_hour", "make_it_right",
			"shift_change", "the_set", "the_wire", "the_count", "the_bag", "fuel_post", "latrine",
			"fuel_lorry", "medicine_still", "short_change", "pit_count",
			"freezers", "night_readings", "labels",
			"before_dark", "wanted_poster"};

		foreach (string key : keys)
		{
			list.Insert(new RZ_ErrandRoutine(key));
		}
	}

	// ================================================================ Rows
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		switch (m_sKey)
		{
			case "pot": DefinePot(); break;
			case "lights": DefineLights(); break;
			case "water": DefineWater(); break;
			case "filter_parts": DefineFilterParts(); break;
			case "gate_dusk": DefineGateDusk(); break;
			case "burying": DefineBurying(); break;
			case "round": DefineRound(); break;
			case "newcomer": DefineNewcomer(); break;
			case "childrens_hour": DefineChildrensHour(); break;
			case "make_it_right": DefineMakeItRight(); break;
			case "shift_change": DefineShiftChange(); break;
			case "the_set": DefineTheSet(); break;
			case "the_wire": DefineTheWire(); break;
			case "the_count": DefineTheCount(); break;
			case "the_bag": DefineTheBag(); break;
			case "fuel_post": DefineFuelPost(); break;
			case "latrine": DefineLatrine(); break;
			case "fuel_lorry": DefineFuelLorry(); break;
			case "medicine_still": DefineMedicineStill(); break;
			case "short_change": DefineShortChange(); break;
			case "pit_count": DefinePitCount(); break;
			case "freezers": DefineFreezers(); break;
			case "night_readings": DefineNightReadings(); break;
			case "labels": DefineLabels(); break;
			case "before_dark": DefineBeforeDark(); break;
			case "wanted_poster": DefineWantedPoster(); break;
		}

		m_sThread = ThreadFor(m_sKey);
	}

	//------------------------------------------------------------------------------------------------
	//! What another of the place's people says you did, as a thread ("" none).
	protected static string ThreadFor(string key)
	{
		switch (key)
		{
			case "pot": return "brought something for the pot";
			case "lights": return "got the lights back on";
			case "water": return "carried the water in";
			case "filter_parts": return "fixed the filter";
			case "gate_dusk": return "kept the gate at dusk";
			case "burying": return "dug one of ours a grave";
			case "round": return "sat with the sick";
			case "newcomer": return "saw to the new one";
			case "childrens_hour": return "watched the children";
			case "the_wire": return "stood the wire with us";
			case "fuel_post": return "brought the post its fuel";
			case "freezers": return "kept the freezers cold";
			case "short_change": return "found where the short change went";
		}

		return "";
	}

	// ---------------------------------------------------------------- A Haven safehouse
	//------------------------------------------------------------------------------------------------
	protected void DefinePot()
	{
		m_sTitle = "The Pot";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "cook";
		m_iNeed = RZ_ENeed.FOOD;
		Offer("\"The pot's thin. Two tins would make it soup again. Or tell me there's somewhere left to pick.\"",
			"\"We're on half ladles at {house}. If you've food on you, I'll take it. If not, the farms out east might have something.\"",
			"\"Supper's water with ideas in it. Anything you can spare, or anywhere you know that isn't picked clean.\"");
		Over("\"Never mind. The truck came in after all.\"", "\"Forget it. Someone else filled the pot. Don't ask me who, I was busy crying into it.\"");

		RZ_ErrandWay w = Way("give");
		w.m_sLabel = "Hand over two tins";
		w.m_eItem = RZ_EErrandItem.FOOD;
		w.m_iItems = 2;
		w.m_iFavor = 4;
		w.m_iFame = 1;
		w.m_iEffect = FX_FOOD;
		w.Done("\"That's supper. The ham bone gets another night off.\"", "\"Two tins. I'll make it look like four.\"");

		w = Way("do");
		w.m_sLabel = "Ask where's left to pick";
		w.m_ePlace = RZ_EErrandPlace.EMPTY_VILLAGE;
		w.m_fSeconds = 30;
		w.m_fRadius = 60;
		w.m_sStep = "Search the gardens and sheds at {place} for anything that grows (30 s there), then bring it to {giver}.";
		w.m_iFavor = 5;
		w.m_iEffect = FX_FOOD;
		w.Take("\"Try {place}. Nobody's been out there since the frost. Bring a bag.\"");
		w.Done("\"Potatoes. Small ones, but potatoes. Who wants seconds? Nobody gets seconds.\"", "\"Carrots with the soil still on. The children will complain. Good.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineLights()
	{
		m_sTitle = "Lights";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "storekeeper";
		m_iFault = RZ_EFault.FUEL;
		m_bTimed = true;
		Offer("\"Generator's dry. One can, before the lamps. Otherwise it's candles and ghost stories.\"",
			"\"The lamps go out at dusk unless someone finds fuel. I've written it on the board twice.\"");
		Over("\"The lamps are on. Someone beat you to it.\"");

		RZ_ErrandWay w = Way("give");
		w.m_sLabel = "Pour in a can of fuel";
		w.m_eItem = RZ_EErrandItem.FUEL;
		w.m_iItems = 1;
		w.m_iFavor = 5;
		w.m_iEffect = FX_FIX;
		w.Done("\"Hear that? That's the sound of nobody tripping over the bucket tonight.\"");

		w = Way("give");
		w.m_sLabel = "Wire in a car battery";
		w.m_sBackground = "mechanic";
		w.m_eItem = RZ_EErrandItem.BATTERY;
		w.m_iItems = 1;
		w.m_iFavor = 5;
		w.m_iEffect = FX_FIX;
		w.Done("\"A battery? Huh. I'd have kept looking for a can until Tuesday.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineWater()
	{
		m_sTitle = "Water for {house}";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "lookout";
		m_iFault = RZ_EFault.WATER;
		Offer("\"Barrels are at the sludge. Two bottles from the well at {next_town} would carry us to the next run.\"",
			"\"We're down to washing in tea. The well at {next_town} still runs, if you go quiet.\"");
		Over("\"Somebody's filled the barrels. You're off the hook.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Fetch water from {next_town}";
		w.m_ePlace = RZ_EErrandPlace.NEXT_TOWN;
		w.m_fSeconds = 20;
		w.m_fRadius = 60;
		w.m_bQuiet = true;
		w.m_fQuietMult = 1.3;
		w.m_sStep = "Fill two bottles at the well in {place} (20 s there) and bring them to {giver}. Quieter is better.";
		w.m_iFavor = 5;
		w.m_iEffect = FX_FIX_FOOD;
		w.Take("\"The short way runs through the bad street. Don't take the short way.\"");
		w.Done("\"Clean enough to boil. We'll have tea that tastes of tea.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineFilterParts()
	{
		m_sTitle = "Filter Parts";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "storekeeper";
		m_iFault = RZ_EFault.WATER;
		Offer("\"The filter's choking. There's a pump house at {next_town} with the part we need, if nobody's had it.\"");
		Over("\"The filter's running again. Someone found the part.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Strip the part at {next_town}";
		w.m_ePlace = RZ_EErrandPlace.NEXT_TOWN;
		w.m_fSeconds = 25;
		w.m_fRadius = 60;
		w.m_sStep = "Find the pump house at {place} and take the filter housing (25 s there), then bring it to {giver}.";
		w.m_eProp = RZ_EErrandProp.PARTS;
		w.m_iFavor = 8;
		w.m_iEffect = FX_FIX;
		w.Done("\"That's the one. I'll tell everyone it was me.\"");

		w = Way("do");
		w.m_sLabel = "Take only the good part";
		w.m_sBackground = "mechanic,labtech";
		w.m_ePlace = RZ_EErrandPlace.NEXT_TOWN;
		w.m_fSeconds = 12;
		w.m_fRadius = 60;
		w.m_sStep = "At the pump house in {place}, take the one part worth taking (12 s there), then bring it to {giver}.";
		w.m_eProp = RZ_EErrandProp.PARTS;
		w.m_iFavor = 8;
		w.m_iEffect = FX_FIX;
		w.Done("\"One part, not two. You know what you're doing. That's rare.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineGateDusk()
	{
		m_sTitle = "The Gate at Dusk";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "lookout";
		m_bTimed = true;
		Offer("\"It's me on the gate tonight, and only me. {near_town} has been loud. Stand with me a while?\"",
			"\"Dusk soon. One on the gate, and {near_town} is restless. I'd rather not be the only one listening.\"");
		Over("\"The others are back. Go on, I'm fine.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Stand at the gate with them";
		w.m_ePlace = RZ_EErrandPlace.GATE;
		w.m_fSeconds = 600;
		w.m_fRadius = 25;
		w.m_sStep = "Stand at the gate at {house} through dusk (10 real minutes there), then tell {giver} it's done.";
		w.m_iFavor = 5;
		w.Done("\"Quiet night. I'll remember who stood with me.\"", "\"Nothing came. That's the best sort of night. Thank you.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineBurying()
	{
		m_sTitle = "The Burying";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "clerk";
		Offer("\"We lost someone today. The ground's hard at the edge of town, and I can't dig on my own.\"",
			"\"There's a grave to dig. We read the name at supper. Dig it deep.\"");
		Over("\"It's dug. The others did it while you were away.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Dig at the edge of town";
		w.m_ePlace = RZ_EErrandPlace.GATE;
		w.m_fSeconds = 30;
		w.m_fRadius = 30;
		w.m_sStep = "Dig the grave at the edge of {house} (30 s there), then tell {giver}.";
		w.m_iFavor = 3;
		w.m_iEffect = FX_RECORD;
		w.Done("\"Deep enough. We'll read the name at supper. Come, if you like.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineRound()
	{
		m_sTitle = "The Round";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "clerk";
		Offer("\"No medic here, and three coughing in the back room. You've the hands for it. A minute at each bed?\"");
		Over("\"The medic from the next house came after all.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "See to the sick";
		w.m_sBackground = "nurse,doctor";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 120;
		w.m_fRadius = 20;
		w.m_sStep = "See to the sick at {house}, a minute at each bed (2 real minutes there).";
		w.m_iFavor = 4;
		w.Done("\"They'll sleep tonight. So will I, which is the real gift.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineNewcomer()
	{
		m_sTitle = "The Newcomer";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "clerk";
		Offer("\"We took someone in today. No coat, no shoes worth the name. There's a clothes shop at {next_town} nobody's emptied.\"");
		Over("\"Someone gave him a coat already. It is too big. He loves it.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Find a coat at {next_town}";
		w.m_ePlace = RZ_EErrandPlace.NEXT_TOWN;
		w.m_fSeconds = 20;
		w.m_fRadius = 60;
		w.m_sStep = "Find a coat in {place} (20 s there) and bring it to {giver}.";
		w.m_iFavor = 4;
		w.Done("\"It fits. He's asked for a job already. We gave him the bucket.\"");

		w = Way("do");
		w.m_sLabel = "Check him over";
		w.m_sBackground = "nurse,doctor";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 30;
		w.m_fRadius = 20;
		w.m_sStep = "Look the newcomer over at {house} (30 s there).";
		w.m_iFavor = 4;
		w.Done("\"No bites, no fever. Just hungry. We can fix hungry, given a week.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineChildrensHour()
	{
		m_sTitle = "The Children's Hour";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_bTimed = true;
		Offer("\"The gate's open for a delivery and the little ones won't stay in the yard. Five minutes, keep them in?\"");
		Over("\"Gate's shut. They've all gone in for lunch.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Watch the children in the yard";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 300;
		w.m_fRadius = 20;
		w.m_bQuiet = true;
		w.m_sStep = "Keep the children in the yard at {house} (5 real minutes there). No shooting near them.";
		w.m_iFavor = 4;
		w.Done("\"Count them in, count them out. Same number, ideally. Same number.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineMakeItRight()
	{
		m_sTitle = "Make It Right";
		m_sFamily = "amends";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "lookout";
		Offer("\"People here know your face, and not kindly. Keep the gate a while, or carry the night water. It's a start.\"");
		Over("\"Not now. Something's happening at the house.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Keep the gate at dusk";
		w.m_ePlace = RZ_EErrandPlace.GATE;
		w.m_fSeconds = 180;
		w.m_fRadius = 25;
		w.m_sStep = "Keep the gate at {house} (3 real minutes there), then tell {giver}.";
		w.m_bAmends = true;
		w.Done("\"You stood there. People saw you stand there. That's how it starts.\"");

		w = Way("do");
		w.m_sLabel = "Carry the night water";
		w.m_ePlace = RZ_EErrandPlace.NEXT_TOWN;
		w.m_fSeconds = 15;
		w.m_fRadius = 60;
		w.m_sStep = "Fill the house's cans at {place} (15 s there) and bring them to {giver}.";
		w.m_bAmends = true;
		w.Done("\"Full cans. Nobody will say thank you. That's how you know it's counted.\"");
	}

	// ---------------------------------------------------------------- A Bastion post
	//------------------------------------------------------------------------------------------------
	protected void DefineShiftChange()
	{
		m_sTitle = "Shift Change";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		m_iFault = RZ_EFault.RELIEF;
		Offer("\"Relief hasn't shown. Last seen walking to a car out on the road with a blanket. Wake him, or stand his post.\"",
			"\"One on the gate and the relief missing. I know where he is. I'd rather not know.\"");
		Over("\"Relief's turned up. Carry on.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Find and wake the relief";
		w.m_ePlace = RZ_EErrandPlace.OUT;
		w.m_fSeconds = 10;
		w.m_fRadius = 40;
		w.m_sStep = "Find the private asleep in a car {place} and wake him (10 s there), then report to {giver}.";
		w.m_iFavor = 4;
		w.m_iEffect = FX_FIX;
		w.Done("\"He'll be on the gate in five. He owes you. He'll pretend he doesn't.\"");

		w = Way("report");
		w.m_sLabel = "Report him asleep";
		w.m_iFame = 1;
		w.Done("\"Charge him with what? Sleeping through the end of the world? I'll write it up anyway.\"");

		w = Way("do");
		w.m_sLabel = "Stand the post yourself";
		w.m_ePlace = RZ_EErrandPlace.GATE;
		w.m_fSeconds = 180;
		w.m_fRadius = 20;
		w.m_sStep = "Stand at the gate at {here} (3 real minutes there), then report to {giver}.";
		w.m_iFavor = 6;
		w.m_iEffect = FX_FIX;
		w.Done("\"Post held. Logged as a volunteer. The form has no box for that, so I drew one.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineTheSet()
	{
		m_sTitle = "The Set";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		m_sRoles = "radio operator";
		m_iFault = RZ_EFault.SET;
		m_bTimed = true;
		Offer("\"Set's flat. Tonight's numbers go out as a hiss unless someone has a battery. Command won't notice. Command doesn't notice.\"");
		Over("\"A spare turned up in a drawer, under a sandwich.\"");

		RZ_ErrandWay w = Way("give");
		w.m_sLabel = "Hand over a battery";
		w.m_eItem = RZ_EErrandItem.BATTERY;
		w.m_iItems = 1;
		w.m_iFavor = 5;
		w.m_iEffect = FX_FIX;
		w.Done("\"Signal's clean. The numbers go out on time, to nobody, as usual.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineTheWire()
	{
		m_sTitle = "The Wire";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		Offer("\"Contact on the north road tonight, and the wire's thin. Five minutes on the wire with us.\"",
			"\"Pressure on the post since the truck went. Another pair of eyes on the wire would be logged gratefully.\"");
		Over("\"Stand down. The pressure's moved off.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Man the wire";
		w.m_ePlace = RZ_EErrandPlace.GATE;
		w.m_fSeconds = 300;
		w.m_fRadius = 25;
		w.m_sStep = "Stand on the wire at {here} (5 real minutes there), then report to {giver}.";
		w.m_iFavor = 6;
		w.Done("\"Wire held. Logged. You weren't here, officially, which is a compliment.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineTheCount()
	{
		m_sTitle = "The Count";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		Offer("\"Stores count is due and the numbers don't add. Three stacks. Count them, and write what's there.\"");
		Over("\"Count's done. Someone signed it already.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Count it and write what's there";
		w.m_iMinTier = RZ_ETier.KNOWN;
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 30;
		w.m_fRadius = 20;
		w.m_sStep = "Count the three stacks at {here} (30 s there), then hand the sheet to {giver}.";
		w.m_iFavor = 6;
		w.Done("\"That's short. That's very short. I'll file it, and nobody will read it.\"");

		w = Way("do");
		w.m_sLabel = "Count it and write what they want";
		w.m_iMinTier = RZ_ETier.KNOWN;
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 30;
		w.m_fRadius = 20;
		w.m_sStep = "Count the three stacks at {here} (30 s there), then hand the sheet to {giver}.";
		w.m_iFavor = 10;
		w.m_iEffect = FX_RECORD;
		w.Done("\"Signed correct. Every one. Don't look at me like that.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineTheBag()
	{
		m_sTitle = "The Bag";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		m_bTimed = true;
		Offer("\"Put this in the mail bag by the radio? I'd do it, but I'm not allowed to want things.\"");
		Over("\"Never mind. Corporal took it.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Put his letter in the mail bag";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 5;
		w.m_fRadius = 20;
		w.m_sStep = "Put the letter in the mail bag by the radio at {here} (5 s there).";
		w.m_iEffect = FX_MORALE;
		w.Done("\"The bag's full. Nobody has collected it since Day 3. It goes in anyway.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineFuelPost()
	{
		m_sTitle = "Fuel for the Post";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		m_iNeed = RZ_ENeed.SUPPLIES;
		Offer("\"Truck didn't make it, and the generator's on fumes. A can from your own pack would be logged.\"");
		Over("\"Resupply's in. Stand down.\"");

		RZ_ErrandWay w = Way("give");
		w.m_sLabel = "Hand over a can of fuel";
		w.m_eItem = RZ_EErrandItem.FUEL;
		w.m_iItems = 1;
		w.m_iFavor = 6;
		w.Done("\"Signed for. You'll get a receipt when the gates open.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineLatrine()
	{
		m_sTitle = "The Latrine";
		m_sFamily = "amends";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		Offer("\"There's a hole to dig behind the post. Nobody volunteers. You look like a volunteer.\"");
		Over("\"Dug already. Lucky you.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Dig the latrine";
		w.m_ePlace = RZ_EErrandPlace.GATE;
		w.m_fSeconds = 30;
		w.m_fRadius = 30;
		w.m_sStep = "Dig the new latrine behind {here} (30 s there), then report to {giver}.";
		w.m_iFavor = 3;
		w.m_bAmends = true;
		w.Done("\"Everything is signed for. Including this.\"");
	}

	// ---------------------------------------------------------------- A Red Hand camp
	//------------------------------------------------------------------------------------------------
	protected void DefineFuelLorry()
	{
		m_sTitle = "Fuel for the Lorry";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		m_sRoles = "toll man,mechanic";
		m_iNeed = RZ_ENeed.SUPPLIES;
		Offer("\"Lorry's dry and the boss is in a mood. A can from your pack and I'll forget I saw your face. In a good way.\"");
		Over("\"Somebody found a can. You're too slow, friend.\"");

		RZ_ErrandWay w = Way("give");
		w.m_sLabel = "Hand over a can of fuel";
		w.m_eItem = RZ_EErrandItem.FUEL;
		w.m_iItems = 1;
		w.m_iFavor = 6;
		w.Done("\"There she goes. The book says you're alright. The book's never wrong, except when it is.\"");

		w = Way("talk");
		w.m_sLabel = "Say there's no fuel left";
		w.Done("\"Then we're all walking. Good. Walking pays tolls too.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineMedicineStill()
	{
		m_sTitle = "Medicine";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		m_sRoles = "cook";
		Offer("\"The still wants feeding. Two tins of fruit. It's for the medicine. The medicine is also the brandy.\"");
		Over("\"Still's fed. Come back when it's thirsty.\"");

		RZ_ErrandWay w = Way("give");
		w.m_sLabel = "Hand over two tins";
		w.m_eItem = RZ_EErrandItem.FOOD;
		w.m_iItems = 2;
		w.m_iFavor = 6;
		w.Done("\"Lovely. In a week it'll strip paint and cure coughs. Same bottle.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineShortChange()
	{
		m_sTitle = "Short Change";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		m_sRoles = "lookout";
		Offer("\"Two collectors on the toll, and the take's light. One of them is pocketing. Watch them and tell me which.\"");
		Over("\"Forget it. The boss sorted it himself, loudly.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Watch them, then name him";
		w.m_iMinTier = RZ_ETier.KNOWN;
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 60;
		w.m_fRadius = 25;
		w.m_sStep = "Watch the two collectors at {here} (a minute there), then tell {giver} which one.";
		w.m_iFavor = 8;
		w.Done("\"Him? Course it's him. He's got new boots. Nobody gets new boots.\"");

		w = Way("do");
		w.m_sLabel = "Pick him out";
		w.m_sBackground = "police,excon";
		w.m_iMinTier = RZ_ETier.KNOWN;
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 20;
		w.m_fRadius = 25;
		w.m_sStep = "Watch the collectors at {here} (20 s there): you know what to look for. Then tell {giver}.";
		w.m_iFavor = 8;
		w.Done("\"Twenty seconds. You've done this before, haven't you. Don't answer.\"");

		w = Way("do");
		w.m_sLabel = "Watch them, then warn him";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 60;
		w.m_fRadius = 25;
		w.m_sStep = "Watch the two collectors at {here} (a minute there), then come back to {giver}.";
		w.Done("\"Couldn't tell, eh? Funny. He's suddenly very honest. Funny, that.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefinePitCount()
	{
		m_sTitle = "The Pit Count";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		Offer("\"There's a fight on tonight and nobody here can count past their fingers. Count the bets into the book?\"");
		Over("\"Fight's off. Someone's knuckles gave out early.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Count the bets into the book";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 120;
		w.m_fRadius = 25;
		w.m_sStep = "Count the bets into the book at {here} (2 real minutes there), then hand it to {giver}.";
		w.m_iFavor = 5;
		w.Done("\"Balanced to the tin. You'd make a terrible gambler and a lovely bookie.\"");
	}

	// ---------------------------------------------------------------- A Halcyon station or the lab
	//------------------------------------------------------------------------------------------------
	protected void DefineFreezers()
	{
		m_sTitle = "The Freezers";
		m_eAnchor = RZ_EErrandAnchor.HALCYON_STATION;
		m_iFaction = RZ_EFaction.INSTITUTE;
		m_sRoles = "technician,lab assistant";
		m_iFault = RZ_EFault.FUEL;
		Offer("\"The freezers have fuel for one night, perhaps less. A can, or a battery for the backup. Samples first. We apologise to the samples.\"");
		Over("\"Fuel arrived. The samples will survive. So will we, incidentally.\"");

		RZ_ErrandWay w = Way("give");
		w.m_sLabel = "Hand over a can of fuel";
		w.m_eItem = RZ_EErrandItem.FUEL;
		w.m_iItems = 1;
		w.m_iFavor = 6;
		w.m_iFame = 1;
		w.m_iEffect = FX_FIX;
		w.Done("\"Temperature holding. I'll note who supplied it. Notes are what we have.\"");

		w = Way("give");
		w.m_sLabel = "Wire in a battery";
		w.m_eItem = RZ_EErrandItem.BATTERY;
		w.m_iItems = 1;
		w.m_iFavor = 6;
		w.m_iFame = 1;
		w.m_iEffect = FX_FIX;
		w.Done("\"The backup's live. Not ideal. Ideal ended some weeks ago.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineNightReadings()
	{
		m_sTitle = "Night Readings";
		m_eAnchor = RZ_EErrandAnchor.HALCYON_STATION;
		m_iFaction = RZ_EFaction.INSTITUTE;
		m_iFault = RZ_EFault.SENSOR;
		Offer("\"Sensor four has gone dark, {place}. A fresh battery, or a reset if you can do it unseen. We'd like the readings.\"",
			"\"No data from sensor four since the shift change. Possibly the cable. Possibly not a rat. We'd like it to be a rat.\"");
		Over("\"Sensor four is back. Perhaps it was the rat.\"");

		RZ_ErrandWay w = Way("give");
		w.m_sLabel = "Put a battery in the sensor";
		w.m_eItem = RZ_EErrandItem.BATTERY;
		w.m_iItems = 1;
		w.m_bGiveThere = true;
		w.m_ePlace = RZ_EErrandPlace.FAR;
		w.m_fSeconds = 5;
		w.m_fRadius = 30;
		w.m_sStep = "Fit a battery to sensor four, {place} (at the sensor), then tell {giver}.";
		w.m_iFavor = 5;
		w.m_iEffect = FX_FIX;
		w.Done("\"Data flowing. Nothing of note. Nothing of note is the best data.\"");

		w = Way("sneak");
		w.m_sLabel = "Reset it unseen";
		w.m_ePlace = RZ_EErrandPlace.FAR;
		w.m_fSeconds = 15;
		w.m_fRadius = 25;
		w.m_bUnseen = true;
		w.m_sStep = "Reset sensor four, {place}, without being seen (15 s there, hidden), then tell {giver}.";
		w.m_iFavor = 5;
		w.m_iEffect = FX_FIX;
		w.Done("\"Reset clean. Whatever chewed the cable didn't see you. Excellent. For you.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineLabels()
	{
		m_sTitle = "Labels";
		m_eAnchor = RZ_EErrandAnchor.HALCYON_STATION;
		m_iFaction = RZ_EFaction.INSTITUTE;
		m_bTimed = true;
		Offer("\"Night shift. The trays need relabelling before the morning count. Nobody reads them. That's not the point.\"");
		Over("\"Morning shift will do it. Thank you anyway.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Relabel the trays";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 120;
		w.m_fRadius = 20;
		w.m_sStep = "Relabel the trays at the bench at {here} (2 real minutes there).";
		w.m_iFavor = 4;
		w.Done("\"Legible. Correct. I will sleep for an hour, if the freezers allow it.\"");

		w = Way("do");
		w.m_sLabel = "Relabel them properly";
		w.m_sBackground = "labtech";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 60;
		w.m_fRadius = 20;
		w.m_sStep = "Relabel the trays at the bench at {here} (a minute there): you know the codes.";
		w.m_iFavor = 4;
		w.Done("\"You used the old codes. Nobody uses the old codes any more. Thank you.\"");
	}

	// ---------------------------------------------------------------- The wanderers' asks
	//------------------------------------------------------------------------------------------------
	protected void DefineBeforeDark()
	{
		m_sTitle = "Before Dark";
		m_sFamily = "wanderer";
		m_eAnchor = RZ_EErrandAnchor.WANDERER;
		m_iWandererRole = RZ_EWandererRole.DOCTOR;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_bTimed = true;
		Offer("\"There's someone in {next_town} with a fresh bite and a clean rag. Something from a medical kit before half eight, and they'll see the morning.\"");
		Over("\"Too late, or too early. Either way, not you. Not today.\"");

		RZ_ErrandWay w = Way("give");
		w.m_sLabel = "Take them something medical";
		w.m_eItem = RZ_EErrandItem.MEDICAL;
		w.m_iItems = 1;
		w.m_bGiveThere = true;
		w.m_ePlace = RZ_EErrandPlace.NEXT_TOWN;
		w.m_fSeconds = 5;
		w.m_fRadius = 60;
		w.m_fBefore = 20.5;
		w.m_sStep = "Take something medical to the bitten survivor in {place} and leave it with them, then find {giver} again.";
		w.m_iFavor = 6;
		w.Done("\"They'll see the morning. After that, it's the lab or prayer, and I've seen how both go.\"");

		w = Way("do");
		w.m_sLabel = "Dress the bite yourself";
		w.m_sBackground = "nurse,doctor";
		w.m_ePlace = RZ_EErrandPlace.NEXT_TOWN;
		w.m_fSeconds = 45;
		w.m_fRadius = 60;
		w.m_fBefore = 20.5;
		w.m_sStep = "Dress the bite of the survivor in {place} (45 s there), then find {giver} again.";
		w.m_iFavor = 6;
		w.Done("\"Clean dressing? Good. You'd have made a decent doctor. Shame about the world.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineWantedPoster()
	{
		m_sTitle = "Wanted Poster";
		m_sFamily = "wanderer";
		m_eAnchor = RZ_EErrandAnchor.WANDERER;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("\"I've seen your face on a notice. Somebody would pay to know where you sleep. You could pay more.\"");
		Over("\"Forget it. Your face is old news now.\"");

		RZ_ErrandWay w = Way("pay");
		w.m_sLabel = "Pay him to forget your face";
		w.m_iFavorCost = 8;
		w.Done("\"Forgotten. Faces are hard to remember. Yours especially, now.\"");

		w = Way("press");
		w.m_sLabel = "Lean on him";
		w.m_bThreat = true;
		w.Done("\"Alright. Alright. I never saw you. I never see anyone.\"");

		w = Way("leave");
		w.m_sLabel = "Let him sell it";
		w.m_iEffect = FX_HUNTERS;
		w.Done("\"No hard feelings. Business. Sleep somewhere else tonight.\"");
	}

	// ================================================================ Why
	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		int f = m_iFaction;
		if (!RZ_Factions.IsValid(f))
			f = RZ_EFaction.SURVIVORS;

		RZ_ETier tier = gm.GetTier(playerId, f);
		switch (m_sKey)
		{
			case "make_it_right": return tier == RZ_ETier.WATCHED;
			case "latrine": return tier == RZ_ETier.WATCHED || tier == RZ_ETier.STRANGER;
			case "filter_parts": return RZ_Reputation.IsAtLeast(tier, RZ_ETier.KNOWN);
			case "the_count": return RZ_Reputation.IsAtLeast(tier, RZ_ETier.KNOWN);
			case "short_change": return RZ_Reputation.IsAtLeast(tier, RZ_ETier.KNOWN);
			case "the_wire": return !RZ_Reputation.IsWatchedOrWorse(tier);
			case "wanted_poster": return gm.GetTier(playerId, RZ_EFaction.MILITARY) == RZ_ETier.KILL_ON_SIGHT || gm.GetTier(playerId, RZ_EFaction.BANDITS) == RZ_ETier.KILL_ON_SIGHT;
			case "round": return RZ_QuestApproach.HasBackground(gm, playerId, "nurse,doctor");
		}

		// The amends family is for the Watched; everyone else gets the ordinary chores
		if (tier == RZ_ETier.WATCHED && m_sFamily != "amends")
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		vector next = dealer.PlaceFor(e, RZ_EErrandPlace.NEXT_TOWN);
		if (next != vector.Zero)
		{
			RZ_Settlement s = gm.GetSettlement(gm.FindNearestSettlement(next));
			if (s)
				e.SetText("next_town", s.m_sName);
		}

		if (m_iFault != 0 && !HasFault(dealer, e))
			return false;

		if (e.m_iHouse >= 0)
		{
			e.m_sSubject = RZ_WorldFacts.Safehouse(e.m_iHouse);
			if (gm.GetSafehouses() && gm.GetSafehouses().IsExiled(e.m_iAsker))
				return false;
		}
		else if (e.m_iHolding >= 0)
		{
			e.m_sSubject = "holding:" + e.m_iHolding.ToString();
		}

		return True(dealer, e);
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasFault(RZ_Errands dealer, RZ_Errand e)
	{
		string place = RZ_Faults.HoldingPlace(e.m_iHolding);
		if (e.m_iHouse >= 0)
			place = RZ_Faults.HousePlace(e.m_iHouse);

		return dealer.GetFaults().Has(place, m_iFault);
	}

	//------------------------------------------------------------------------------------------------
	//! Each row's own live facts.
	protected bool True(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_WorldFacts facts = gm.GetFacts();
		float hour = RZ_GameTime.GetHour();
		switch (m_sKey)
		{
			case "pot": return HouseHungry(gm, e.m_iHouse) && !TruckDue(gm, e.m_iHouse);
			case "lights": return DuskSoon(hour);
			case "water": return true;
			case "filter_parts": return HouseFacts(e).m_iPeople >= 8;
			case "gate_dusk": return DuskSoon(hour) && HouseGuards(gm, e.m_iHouse) <= 1 && LoudTownNear(dealer, e, 60) && !gm.GetMissions().IsTypeRunning(RZ_Event_EveryRidgeCalls);
			case "burying": return HouseFacts(e).m_iDied > 0;
			case "round": return !HouseFacts(e).m_bHasMedic && HouseFacts(e).m_iPeople >= 4;
			case "newcomer": return dealer.TookSomeoneIn(e.m_iHouse);
			case "childrens_hour": return HouseFacts(e).m_iPeople >= 5 && hour >= 8 && hour < 17;
			case "make_it_right": return true;
			case "shift_change": return true;
			case "the_set": return hour >= 17.5 && hour < 20.5;
			case "the_wire": return TruckLostNear(facts, e.m_vGiver, RZ_EFaction.MILITARY) || LoudTownNear(dealer, e, 50);
			case "the_count": return TruckLostNear(facts, e.m_vGiver, RZ_EFaction.MILITARY) || PostShort(gm, e);
			case "the_bag": return (hour >= 20.5 || hour < 5.5) && Morale(gm, RZ_EFaction.MILITARY) < 40;
			case "fuel_post": return PostShort(gm, e);
			case "latrine": return true;
			case "fuel_lorry": return gm.GetFactionLife() && gm.GetFactionLife().GetNeed(RZ_EFaction.BANDITS) == RZ_ENeed.SUPPLIES;
			case "medicine_still": return !IsStronghold(gm, e);
			case "short_change": return Morale(gm, RZ_EFaction.BANDITS) < 55;
			case "pit_count": return gm.GetMissions().IsTypeRunning(RZ_Mission_FightPit);
			case "freezers": return hour >= 17.5 || hour < 5.5;
			case "night_readings": SetSensor(dealer, e); return true;
			case "labels": return hour >= 20.5 || hour < 5.5;
			case "before_dark": return hour >= 15 && hour < 20 && InfectedTownNear(dealer, e, 30);
			case "wanted_poster": return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool Still(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (m_iFault != 0 && !HasFault(dealer, e))
			return false;

		switch (m_sKey)
		{
			case "pot": return HouseHungry(gm, e.m_iHouse);
			case "fuel_post": return PostShort(gm, e);
			case "pit_count": return gm.GetMissions().IsTypeRunning(RZ_Mission_FightPit);
		}

		if (e.m_iHouse >= 0)
			return HouseFacts(e).m_bValid && !HouseFacts(e).m_bEmpty;

		if (e.m_iHolding >= 0 && gm.GetTerritory())
			return gm.GetTerritory().GetHolding(e.m_iHolding) != null;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		array<int> who = {playerId};
		switch (w.m_iEffect)
		{
			case FX_FOOD:
			{
				if (gm.GetSafehouses() && e.m_iHouse >= 0)
					gm.GetSafehouses().AddFood(e.m_iHouse, 2);

				RZ_WorldFacts.Get().Memory().Record("errand.fed", RZ_WorldFacts.Safehouse(e.m_iHouse), "1", RZ_WorldMemory.SOURCE_ERRAND, false, who);
				break;
			}

			case FX_FIX:
			{
				ClearFault(dealer, e);
				break;
			}

			case FX_FIX_FOOD:
			{
				ClearFault(dealer, e);
				if (gm.GetSafehouses() && e.m_iHouse >= 0)
					gm.GetSafehouses().AddFood(e.m_iHouse, 2);

				break;
			}

			case FX_MORALE:
			{
				dealer.Morale(e.m_iFaction, 1, m_sKey);
				break;
			}

			case FX_RECORD:
			{
				string subject = e.m_sSubject;
				RZ_WorldFacts.Get().Memory().Record("errand." + m_sKey, subject, e.m_sGiverName, RZ_WorldMemory.SOURCE_ERRAND, true, who);
				break;
			}

			case FX_HUNTERS:
			{
				if (gm.GetPerks())
					gm.GetPerks().SendHuntersNow(playerId);

				break;
			}
		}

		// The lights: a house lit at dusk is a record the epilogue may show
		if (m_sKey == "lights")
			RZ_WorldFacts.Get().Memory().Record("errand.lit", e.m_sSubject, "1", RZ_WorldMemory.SOURCE_ERRAND, false, who);
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearFault(RZ_Errands dealer, RZ_Errand e)
	{
		if (m_iFault == 0)
			return;

		string place = RZ_Faults.HoldingPlace(e.m_iHolding);
		if (e.m_iHouse >= 0)
			place = RZ_Faults.HousePlace(e.m_iHouse);

		dealer.GetFaults().Clear(place, m_iFault);
	}

	// ================================================================ The facts the rows read
	//------------------------------------------------------------------------------------------------
	protected static RZ_HouseFacts HouseFacts(RZ_Errand e)
	{
		return RZ_WorldFacts.Get().House(e.m_iHouse);
	}

	//------------------------------------------------------------------------------------------------
	//! A safehouse with food under one upkeep (the cards read it too).
	static bool HouseShort(RZ_GameModeComponent gm, int idx)
	{
		return HouseHungry(gm, idx);
	}

	//------------------------------------------------------------------------------------------------
	//! Food under one upkeep.
	protected static bool HouseHungry(RZ_GameModeComponent gm, int idx)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses || idx < 0)
			return false;

		RZ_Safehouse h = houses.Get(idx);
		if (!h)
			return false;

		return h.m_iFood < h.FoodNeed(houses.PeoplePerFood());
	}

	//------------------------------------------------------------------------------------------------
	//! A Haven truck is on its way to this house.
	protected static bool TruckDue(RZ_GameModeComponent gm, int idx)
	{
		RZ_SupplyLines supply = gm.GetSupply();
		if (!supply)
			return false;

		RZ_SupplyTruck t = supply.GetActiveTruck(RZ_EFaction.SURVIVORS);
		return t && !t.m_bDone && !t.m_bFailed && t.m_iDestKind == RZ_SupplyLines.KIND_SAFEHOUSE && t.m_iDestIndex == idx;
	}

	//------------------------------------------------------------------------------------------------
	protected static int HouseGuards(RZ_GameModeComponent gm, int idx)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses)
			return 0;

		RZ_Safehouse h = houses.Get(idx);
		if (!h)
			return 0;

		return h.GuardCount();
	}

	//------------------------------------------------------------------------------------------------
	//! Dusk within about 20 real minutes, or already dusk.
	protected static bool DuskSoon(float hour)
	{
		if (hour < 0)
			return false;

		if (hour >= RZ_WorldFacts.DUSK_START && hour < 20.5)
			return true;

		return hour < RZ_WorldFacts.DUSK_START && RZ_GameTime.RealMinutesUntil(RZ_WorldFacts.DUSK_START) <= 20;
	}

	//------------------------------------------------------------------------------------------------
	//! A town within 1.5 km of the giver at `infection` or more ({near_town}).
	protected bool LoudTownNear(RZ_Errands dealer, RZ_Errand e, float infection)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			if (i == e.m_iSettlement || s.m_fInfection < infection || RZ_Util.DistXZ(s.m_vCenter, e.m_vGiver) > 1500)
				continue;

			e.SetText("near_town", s.m_sName);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Before Dark: a town within 1.5 km at `infection` or more; it becomes the place ({next_town}).
	protected bool InfectedTownNear(RZ_Errands dealer, RZ_Errand e, float infection)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			if (s.m_fInfection < infection || RZ_Util.DistXZ(s.m_vCenter, e.m_vGiver) > 1500)
				continue;

			e.SetText("next_town", s.m_sName);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A truck of `faction` lost within 1.5 km in the last season day (the signal ring).
	protected static bool TruckLostNear(RZ_WorldFacts facts, vector pos, int faction)
	{
		if (!facts)
			return false;

		array<RZ_Signal> found = {};
		facts.Memory().RecentSignals("truck." + faction.ToString(), pos, 1500, 1, found);
		return !found.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! The holding is short after a lost truck (its supply state).
	protected static bool PostShort(RZ_GameModeComponent gm, RZ_Errand e)
	{
		RZ_SupplyLines supply = gm.GetSupply();
		if (!supply || !supply.IsActive())
			return false;

		RZ_SupplyState st = supply.FindShopState(e.m_iFaction, e.m_vGiver);
		return st && st.m_bShortage;
	}

	//------------------------------------------------------------------------------------------------
	protected static float Morale(RZ_GameModeComponent gm, int f)
	{
		RZ_FactionLife life = gm.GetFactionLife();
		if (!life || !life.IsActive())
			return 60;

		return life.GetMorale(f);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IsStronghold(RZ_GameModeComponent gm, RZ_Errand e)
	{
		if (!gm.GetTerritory())
			return false;

		RZ_Holding h = gm.GetTerritory().GetHolding(e.m_iHolding);
		return h && h.m_bStronghold;
	}

	//------------------------------------------------------------------------------------------------
	//! Night Readings: the sensor stands 300-500 m out; its place is named once.
	protected void SetSensor(RZ_Errands dealer, RZ_Errand e)
	{
		vector at = dealer.PlaceFor(e, RZ_EErrandPlace.FAR);
		e.SetText("place", RZ_Util.DirectionText(e.m_vGiver, at));
	}

	// ================================================================ Barter Chain
	//------------------------------------------------------------------------------------------------
	//! Barter Chain, as built (Missions/Unarmed/RZ_UnarmedTasks.c), now a wanderer's ask: started from
	//! one of its wanderers, offered to this player alone, paid by errands' rules. True if it started.
	static bool StartBarterChain(RZ_GameModeComponent gm, int playerId, RZ_Wanderer w, IEntity user)
	{
		RZ_MissionManager missions = gm.GetMissions();
		if (!missions || missions.IsTypeRunning(RZ_Mission_BarterChain))
			return false;

		RZ_Mission_BarterChain job = new RZ_Mission_BarterChain();
		job.Configure(gm);
		job.m_bInPersonOnly = true;
		job.m_iFameReward = 2;
		job.m_iFavorReward = 8;
		if (job.GetWeight(gm) <= 0 || !missions.StartIfCan(job, GetGame().GetPlayerManager().GetPlayerCount()))
			return false;

		job.m_aOfferedTo.Insert(playerId);
		gm.NotifyPlayerAs(playerId, w.GetTitle(), string.Format("%1\n%2\n(%3)", job.m_sTitle, job.GetBrief(gm), job.GetWhereTextFor(gm, user.GetOrigin(), playerId)), RZ_ENoticeTag.MISSION);
		return true;
	}
}
