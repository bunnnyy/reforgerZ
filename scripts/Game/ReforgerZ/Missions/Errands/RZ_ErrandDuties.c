//! Duties (docs/ERRANDS.md, "Duties"): a place's own everyday work, taken over for a shift. A duty
//! is an errand that is always there to be asked for: it skips the routines' cooldowns for a short
//! duty gap, is asked when nothing truer is (or by the one whose own work it is), and pays through
//! the same small-pay ledger. Each has a trust rung with the giver's faction (RZ_Duties.Rung), and
//! each run is drawn from the world: the task (one of a few, per run) and the reason said for it
//! (the weather, the hour, who died, what the place is short of).
//! One class, one object per duty kind and faction (the same work in that faction's words).
//! Server only.

//------------------------------------------------------------------------------------------------
//! The trust ladder (docs/ERRANDS.md, "The trust ladder").
class RZ_Duties
{
	static const int RUNG_NONE = 0;		// Watched or worse: the amends work only
	static const int RUNG_CHORES = 1;	// anyone else: chores, watched
	static const int RUNG_FACE = 2;		// they know your face: three pieces of work for them this season, or Known
	static const int RUNG_ARMED = 3;	// Known: armed work with their people
	static const int RUNG_TRUST = 4;	// Trusted: the sensitive work

	//------------------------------------------------------------------------------------------------
	//! This player's rung with a faction.
	static int Rung(RZ_Errands dealer, int playerId, int faction)
	{
		if (!RZ_Factions.IsValid(faction))
			return RUNG_CHORES;

		int rank = RZ_Reputation.GetAccessRank(dealer.GetGM().GetTier(playerId, faction));
		if (rank < 0)
			return RUNG_NONE;

		if (rank >= 2)
			return RUNG_TRUST;

		if (rank == 1)
			return RUNG_ARMED;

		if (dealer.WorkedFor(playerId, faction) >= RZ_Errands.KNOWN_FACE)
			return RUNG_FACE;

		return RUNG_CHORES;
	}

	//------------------------------------------------------------------------------------------------
	//! Every duty, in order: each kind in each faction's words.
	static void Build(notnull array<ref RZ_ErrandDef> list)
	{
		array<string> kinds = {"kitchen", "wood", "wash", "line", "dead", "patrol"};
		foreach (string kind : kinds)
		{
			for (int f = 0; f < 4; f++)
			{
				list.Insert(new RZ_ErrandDuty(kind, f));
			}
		}

		// Not every place does these: the Haven and the Red Hand hunt, Halcyon has the bench
		list.Insert(new RZ_ErrandDuty("hunt", RZ_EFaction.SURVIVORS));
		list.Insert(new RZ_ErrandDuty("hunt", RZ_EFaction.BANDITS));
		list.Insert(new RZ_ErrandDuty("bench", RZ_EFaction.INSTITUTE));
		// The Haven's own: the school room, the garden, the door round, the roll call, the swap table
		array<string> haven = {"school", "garden", "doorround", "rollcall", "swap"};
		foreach (string own : haven)
		{
			list.Insert(new RZ_ErrandDuty(own, RZ_EFaction.SURVIVORS));
		}

		// Bastion's own: the loudspeaker tape, gate duty, kit inspection, the field telephone (stag at
		// the wire is its watch)
		array<string> bastion = {"tape", "gate", "kits", "telephone"};
		foreach (string post : bastion)
		{
			list.Insert(new RZ_ErrandDuty(post, RZ_EFaction.MILITARY));
		}

		// The Red Hand's own: stripping a wreck, the still, the count, the toll, spotter
		array<string> redhand = {"wreck", "still", "count", "toll", "spotter"};
		foreach (string camp : redhand)
		{
			list.Insert(new RZ_ErrandDuty(camp, RZ_EFaction.BANDITS));
		}

		// Halcyon's own: the sample round, the decon line, the incinerator, the ward, the cold chain
		array<string> halcyon = {"samples", "decon", "incinerator", "ward", "coldchain"};
		foreach (string lab : halcyon)
		{
			list.Insert(new RZ_ErrandDuty(lab, RZ_EFaction.INSTITUTE));
		}

		// A watch and the stores, everywhere
		for (int g = 0; g < 4; g++)
		{
			list.Insert(new RZ_ErrandDuty("watch", g));
			list.Insert(new RZ_ErrandDuty("stock", g));
		}

		// The Trusted rung: each where it belongs
		list.Insert(new RZ_ErrandDuty("lockup", RZ_EFaction.SURVIVORS));
		list.Insert(new RZ_ErrandDuty("lockup", RZ_EFaction.MILITARY));
		list.Insert(new RZ_ErrandDuty("lockup", RZ_EFaction.BANDITS));
		list.Insert(new RZ_ErrandDuty("names", RZ_EFaction.SURVIVORS));
		list.Insert(new RZ_ErrandDuty("names", RZ_EFaction.MILITARY));
		list.Insert(new RZ_ErrandDuty("list", RZ_EFaction.INSTITUTE));
		list.Insert(new RZ_ErrandDuty("cull", RZ_EFaction.SURVIVORS));
		list.Insert(new RZ_ErrandDuty("cull", RZ_EFaction.MILITARY));
		list.Insert(new RZ_ErrandDuty("lockup", RZ_EFaction.INSTITUTE));
		list.Insert(new RZ_ErrandDuty("names", RZ_EFaction.BANDITS));
		list.Insert(new RZ_ErrandDuty("names", RZ_EFaction.INSTITUTE));
		list.Insert(new RZ_ErrandDuty("dose", RZ_EFaction.SURVIVORS));
		list.Insert(new RZ_ErrandDuty("dose", RZ_EFaction.MILITARY));
		list.Insert(new RZ_ErrandDuty("dose", RZ_EFaction.BANDITS));
		list.Insert(new RZ_ErrandDuty("animals", RZ_EFaction.INSTITUTE));
	}

	//------------------------------------------------------------------------------------------------
	static string Short(int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "haven";
			case RZ_EFaction.MILITARY: return "bastion";
			case RZ_EFaction.BANDITS: return "redhand";
			case RZ_EFaction.INSTITUTE: return "halcyon";
		}

		return "any";
	}
}

//------------------------------------------------------------------------------------------------
class RZ_ErrandDuty : RZ_ErrandDef
{
	// Effects (RZ_ErrandWay.m_iEffect)
	static const int FX_NONE = 0;
	static const int FX_FED = 1;		// a house: +1 food, twice a season day at most (a holding: nothing)
	static const int FX_FED_MEAT = 2;	// meat counts double: +2 food, inside the same two a day

	protected int m_iTasks;				// its task ways ("t0", "t1"...): one is offered a run, drawn in Why

	//------------------------------------------------------------------------------------------------
	void RZ_ErrandDuty(string kind, int faction)
	{
		m_sDuty = kind;
		m_iFaction = faction;
		m_sKey = "duty_" + kind + "_" + RZ_Duties.Short(faction);
	}

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_bDuty = true;
		m_sFamily = "duty";
		m_eAnchor = RZ_EErrandAnchor.ANY_HOLDING;
		m_fWeight = 0.8;
		switch (m_sDuty)
		{
			case "kitchen": DefineKitchen(); break;
			case "wood": DefineWood(); break;
			case "wash": DefineWash(); break;
			case "line": DefineLine(); break;
			case "dead": DefineDead(); break;
			case "patrol": DefinePatrol(); break;
			case "hunt": DefineHunt(); break;
			case "bench": DefineBench(); break;
			case "watch": DefineWatch(); break;
			case "stock": DefineStock(); break;
			case "lockup": DefineLockUp(); break;
			case "names": DefineNames(); break;
			case "list": DefineList(); break;
			case "cull": DefineCull(); break;
			case "dose": DefineDose(); break;
			case "animals": DefineAnimals(); break;
			default:
			{
				if (m_iFaction == RZ_EFaction.MILITARY)
					DefineBastion();
				else if (m_iFaction == RZ_EFaction.BANDITS)
					DefineRedHand();
				else if (m_iFaction == RZ_EFaction.INSTITUTE)
					DefineHalcyon();
				else
					DefineHaven();

				break;
			}
		}
	}

	// What stands at a hands task's work site: the thing its words name (HANDS has no look of its own)
	protected static const ResourceName LOOK_POT = "{2CA928D7FADD1FBA}Prefabs/ReforgerZ/Dressing/Common/RZ_Dress_CookPot.et";		// the camp kit's cook pot on its fire
	protected static const ResourceName LOOK_WOOD = "{0AA08FC6D5A6FF79}Prefabs/ReforgerZ/Dressing/Common/RZ_Dress_Firewood.et";	// the woodpile and the chopping block
	protected static const ResourceName LOOK_LINE = "{136E35552D91BCE4}Prefabs/ReforgerZ/Dressing/Common/RZ_Dress_DryingLine.et";	// the washing line
	protected static const ResourceName LOOK_BUCKET = "{BCE07A58672955B9}Prefabs/Props/Civilian/Bucket_01.et";	// the base game's bucket (footprints.json 0.43 x 0.42 m)
	// The kitchen's own looks (tools/camp_kit/work_looks.py), where the place has no cook pot of its own
	protected static const ResourceName LOOK_STOVE = "{0DFBDF240FC11A1A}Prefabs/ReforgerZ/Work/RZ_Work_Stove.et";		// the pot on a canteen stove
	protected static const ResourceName LOOK_KITCHEN_TABLE = "{87F5BB277D49A412}Prefabs/ReforgerZ/Work/RZ_Work_KitchenTable.et";	// the kitchen table: the pot, bowls, a sack of potatoes
	protected static const ResourceName LOOK_SERVE_TABLE = "{E7365B2986211B5C}Prefabs/ReforgerZ/Work/RZ_Work_ServeTable.et";	// the pot on a table, bowls set out
	// Plainer things of the same kind, where the look above doesn't fit (docs/ERRANDS.md, "Work sites")
	protected static const ResourceName PLAIN_CHOP = "{6105032F794BA64B}Prefabs/Props/Civilian/LogChopping_01.et";	// the chopping block on its own (0.48 x 0.49 m)
	protected static const ResourceName PLAIN_LINE = "{5444982FED78FECE}Prefabs/Props/Furniture/BasketLaundry_01.et";	// a laundry basket
	// One crate to carry: the base game's small wooden box (a hand's load, 3 kg)
	protected static const ResourceName CARRY_CRATE = "{2B1D0128ED660BE9}Prefabs/Props/Crates/BoxWooden_01.et";

	//------------------------------------------------------------------------------------------------
	//! A task way: work with your hands at the place (`gate`: behind it), then back to the giver.
	protected RZ_ErrandWay Task(string label, RZ_EWork work, string prompt, float hold, bool gate, string step, int favor)
	{
		RZ_ErrandWay w = Way("t" + m_iTasks.ToString());
		m_iTasks++;
		w.m_sLabel = label;
		w.m_ePlace = RZ_EErrandPlace.HERE;
		if (gate)
			w.m_ePlace = RZ_EErrandPlace.GATE;

		w.m_fSeconds = 30;
		w.m_fRadius = 30;
		w.m_sStep = step;
		w.Work(work, prompt, hold);
		w.m_iFavor = favor;
		return w;
	}

	//------------------------------------------------------------------------------------------------
	//! The same lines on every way that has none of its own.
	protected void Said(string take, string doneA, string doneB)
	{
		foreach (RZ_ErrandWay w : m_aWays)
		{
			if (w.m_aTake.IsEmpty() && w.HasStep())
				w.Take(take);

			if (w.m_aDone.IsEmpty())
				w.Done(doneA, doneB);
		}
	}

	// ================================================================ Rung 1: chores, watched
	//------------------------------------------------------------------------------------------------
	protected void DefineKitchen()
	{
		m_iRung = RZ_Duties.RUNG_CHORES;
		m_sRoles = "cook,storekeeper,farmer";
		m_iNeed = RZ_ENeed.FOOD;
		m_sThread = "took a turn in the kitchen";
		// At the camp's own cook pot when it has one; else at a kitchen table (or a stove) with the pot on
		// it, and where none fits, the pot itself (docs/ERRANDS.md, "Work sites")
		Task("Peel and chop for the pot", RZ_EWork.BENCH, "Peel and chop", 18, false, "Peel and chop at the kitchen table at {here}, then tell {giver}.", 3).Look(LOOK_KITCHEN_TABLE).AtKitchen();
		Task("Mind the pot", RZ_EWork.HANDS, "Stir the pot", 20, false, "Stir the pot in the kitchen at {here}, then tell {giver}.", 3).Look(LOOK_STOVE).Fallback(LOOK_KITCHEN_TABLE).AtKitchen();
		Task("Serve the line", RZ_EWork.HANDS, "Ladle supper out", 16, false, "Ladle supper out to the line at {here}, then tell {giver}.", 3).Look(LOOK_SERVE_TABLE).AtKitchen();
		RZ_ErrandWay give = Way("give");
		give.m_sLabel = "Put two tins in the pot";
		give.m_eItem = RZ_EErrandItem.FOOD;
		give.m_iItems = 2;
		give.m_iFavor = 4;
		give.m_iEffect = FX_FED;
		RZ_ErrandWay meat = Way("meat");
		meat.m_sLabel = "Put a leg of raw meat in the pot";
		meat.m_eItem = RZ_EErrandItem.MEAT;
		meat.m_iItems = 1;
		meat.m_iFavor = 4;
		meat.m_iEffect = FX_FED_MEAT;
		switch (m_iFaction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				m_sTitle = "Kitchen Duty";
				Offer("The kitchen wants a pair of hands. {why} Nobody's asking you to cook. Peel, stir, serve, and keep your thumbs out of it.",
					"You're standing still, which here means you're volunteering. {why} There's an apron on the nail. Or put something in the pot and I'll call it even.");
				Over("Never mind. Someone's children did it for a biscuit.");
				Said("Wash your hands first. I'll know.", "That's a house fed. Sit down, you get a bowl like everyone else.", "You peel like you've never met a potato. It all went in. Thank you.");
				break;
			}

			case RZ_EFaction.MILITARY:
			{
				m_sTitle = "Mess Duty";
				Offer("Mess duty. {why} You draw a knife from the cook, you return the knife to the cook. That is the whole briefing.",
					"The field kitchen is a man short and the rota doesn't care who fills it. {why} Report to the stove, or put rations in the pot and sign for them.");
				Over("Stand down. The rota caught up.");
				Said("Sleeves up. The cook outranks you in there.", "Mess duty logged against your name. That's a first entry. Keep them clean.", "Fed and on time. The sergeant won't say it, so I will: good.");
				break;
			}

			case RZ_EFaction.BANDITS:
			{
				m_sTitle = "Feeding the Crew";
				Offer("You want in? Everyone starts at the pot. {why} Stir it, serve it, and if the boss asks, it was always that colour.",
					"Kitchen's yours for an hour, new face. {why} Feed the crew and the crew remembers. Or drop two tins in and skip the onions!");
				Over("Forget it. Somebody hungrier got there first.");
				Said("Don't touch the still. The pot. Only the pot.", "Nobody's sick and nobody's complaining. Round here that's a feast!", "The crew ate. You're less of a stranger than you were this morning.");
				break;
			}

			case RZ_EFaction.INSTITUTE:
			{
				m_sTitle = "Canteen";
				Offer("The canteen needs a volunteer, and you're the only one here without a doctorate. {why} Forty trays. The portions are weighed, please don't improvise.",
					"We have a staffing gap in the canteen. {why} It isn't research, but nobody here researches well on an empty stomach.");
				Over("No longer required. Someone found the rota.");
				Said("Gloves are by the door. Please use them.", "Forty trays, forty people fed. That's the most reproducible result we've had this week.", "Thank you. It's been noted, in the good column.");
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineWood()
	{
		m_iRung = RZ_Duties.RUNG_CHORES;
		m_sRoles = "storekeeper,farmer,mechanic,technician";
		m_sThread = "kept the stove in wood and water";
		Task("Split the wood", RZ_EWork.HANDS, "Split the wood", 18, true, "Split the wood stacked behind {here}, then tell {giver}.", 3).Look(LOOK_WOOD).Fallback(PLAIN_CHOP);
		Task("Fill the water cans", RZ_EWork.FILL, "Fill the cans", 15, true, "Fill the water cans at the butt behind {here}, then tell {giver}.", 3);
		// Carried for real: three crates from the road, one by one, stacked indoors (Missions/Blocks/RZ_Carry.c)
		RZ_ErrandWay crates = Task("Carry the crates in", RZ_EWork.HANDS, "Carry and stack the crates", 16, false, "Carry the three crates in from outside and stack them indoors at {here}, then tell {giver}.", 3);
		crates.Carry("crate", "crates", CARRY_CRATE, 3, true, true);
		crates.m_sInside = "any";
		switch (m_iFaction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				m_sTitle = "Wood and Water";
				Offer("There's wood to split and water to carry, and there always will be. {why} Do one of them and you've earned your place by the stove.",
					"If you want to be useful, be useful with your back. {why} It isn't glamorous. Nothing that keeps a house alive is.");
				Over("Never mind. It's done, and badly, but it's done.");
				Said("The axe is behind the door. Mind the handle, it's older than you.", "That'll see us through the night. You're allowed to sit by it now.", "Good. Tomorrow it all wants doing again, but that's tomorrow.");
				break;
			}

			case RZ_EFaction.MILITARY:
			{
				m_sTitle = "Fatigues";
				Offer("Fatigue party of one, and that's you. {why} Wood, water or crates. Pick up, carry, put down. Repeat until told otherwise.",
					"There's labour outstanding on this post. {why} It's the kind that needs no clearance, which is the only kind you've got.");
				Over("Stand down. Fatigues are covered.");
				Said("Tools are signed out to you. They come back.", "Done and returned. You'd be surprised how rare that is.", "Logged. A post runs on this more than it runs on rifles.");
				break;
			}

			case RZ_EFaction.BANDITS:
			{
				m_sTitle = "Earn Your Keep";
				Offer("Nobody eats here for free, and you're nobody yet. {why} Split it, fill it or carry it. Then we'll see about a name for you.",
					"Got a job that needs a strong back and no questions! {why} You've a back. I haven't heard any questions.");
				Over("Too slow. One of the lads did it to get out of something worse.");
				Said("Round the back. Don't wander.", "Fair work. You can stop looking over your shoulder, for today.", "Done? Done. I'll tell the boss you sweat like one of us.");
				break;
			}

			case RZ_EFaction.INSTITUTE:
			{
				m_sTitle = "The Boiler";
				Offer("The boiler needs feeding, and the autoclave runs off the boiler. {why} It's manual work, I'm afraid. We've rather run out of manual people.",
					"Could I ask something unscientific of you? {why} Water, fuel wood, a few crates. Everything else here depends on it.");
				Over("It's been attended to. Thank you for offering.");
				Said("Round the back, by the tank. Thank you.", "The pressure's back up. You've kept the sterile room sterile, in a roundabout way.", "Thank you. People forget the building is part of the experiment.");
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineWash()
	{
		m_iRung = RZ_Duties.RUNG_CHORES;
		m_sRoles = "nurse,medic,lab assistant,clerk";
		m_sThread = "scrubbed the place down";
		Task("Scrub the tables down", RZ_EWork.BENCH, "Scrub the tables", 18, false, "Scrub the tables and boil the cloths at {here}, then tell {giver}.", 3);
		Task("Wash the bedding", RZ_EWork.HANDS, "Wash and hang the bedding", 16, true, "Wash the bedding and hang it on the line behind {here}, then tell {giver}.", 3).Look(LOOK_LINE).Fallback(PLAIN_LINE);
		Task("Sluice the yard", RZ_EWork.HANDS, "Sluice the yard", 18, true, "Sluice the yard and the step at {here} with lime water, then tell {giver}.", 3).Look(LOOK_BUCKET);
		switch (m_iFaction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				m_sTitle = "Washing Down";
				Offer("The place wants washing down. {why} Hot water, lime, and elbow. It's the only medicine we've plenty of.",
					"If you've an hour, there's scrubbing. {why} We lost a house up the road to dirty hands, not to the dead.");
				Over("Never mind, it's been done. You can still wash your own hands.");
				Said("Bucket's by the pump. Boil it first.", "Smells like lime and nothing else. That's how a house should smell now.", "Clean. I'd shake your hand, but look at the state of it.");
				break;
			}

			case RZ_EFaction.MILITARY:
			{
				m_sTitle = "Hygiene Detail";
				Offer("Hygiene detail. {why} Standing orders say the post is washed down daily, and standing orders don't wash anything themselves.",
					"You're on hygiene detail if you want it. {why} A post that gets sick is a post that gets overrun. Scrub accordingly.");
				Over("Detail's covered. Find something else to stand next to.");
				Said("Lime's in the store. Sign for the bucket.", "Inspected and passed. Don't look so surprised.", "That's one thing on this post that's up to standard.");
				break;
			}

			case RZ_EFaction.BANDITS:
			{
				m_sTitle = "The Scrub";
				Offer("Camp stinks, and stink is how people get found. {why} There's a bucket with your name on it. I wrote it on myself!",
					"Scrubbing. I know. {why} The last crew that laughed at soap is in a pit up the road.");
				Over("Forget it. Lost a bet, did it myself.");
				Said("Bucket, brush, off you go.", "Look at that. You can nearly see what colour it was.", "Clean hands, clean camp. Still a dirty business, mind.");
				break;
			}

			case RZ_EFaction.INSTITUTE:
			{
				m_sTitle = "Decontamination, Minor";
				Offer("We need the common areas washed down to protocol. {why} It's the minor protocol, the one that only takes a bucket.",
					"Would you mind a decontamination task? Minor grade. {why} The major grade needs a suit, and we're not there yet, you and I.");
				Over("That's been done on the last shift. My mistake.");
				Said("The solution is premixed. Please don't add to it.", "Swabs come back clean. I did check, that isn't personal.", "Thank you. Clean surfaces are the cheapest result we produce.");
				break;
			}
		}
	}

	// ================================================================ Rung 2: they know your face
	//------------------------------------------------------------------------------------------------
	//! A sorting way: its row is laid out at the place (RZ_DutyGame) and worked by hand; the last
	//! subject put down is the step done.
	protected RZ_ErrandWay TableWay(int kind, bool gate, string label, string step, int favor, int fame)
	{
		RZ_ErrandWay w = Way("table");
		w.m_sLabel = label;
		w.m_ePlace = RZ_EErrandPlace.HERE;
		if (gate)
			w.m_ePlace = RZ_EErrandPlace.GATE;

		w.m_fSeconds = 0;
		w.m_fRadius = 30;
		w.m_sStep = step;
		w.m_iGame = kind;
		w.m_iFavor = favor;
		w.m_iFame = fame;
		return w;
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineLine()
	{
		m_iRung = RZ_Duties.RUNG_FACE;
		m_sRoles = "nurse,medic,lookout,sentry,guard,toll man";
		m_sThread = "stood the line at the gate";
		TableWay(RZ_DutyGame.KIND_LINE, true, "Look the line over",
			"Go to each of the people waiting at the gate of {here}, look them over, and take each by the arm to the gate, the wash or the holding room. Then tell {giver}.", 6, 1);
		switch (m_iFaction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				m_sTitle = "The Line at the Door";
				Offer("There are people at the door, and somebody has to look at them before they're let in. {why} Arms, eyes, a hand on the brow. You've been about long enough to be trusted with a door.",
					"You've a steady face, and that's most of the job. {why} Look them over at the door. If you're not sure of one, hold them. Nobody ever died of waiting in a porch.");
				Over("Never mind. They've been seen to, and one of them wasn't pleased about it.");
				Said("Be kind and be thorough. If it has to be one, be thorough.", "That's the door done. Come in and tell me how it went.", "The porch is empty, then. Tell me who you sent where.");
				break;
			}

			case RZ_EFaction.MILITARY:
			{
				m_sTitle = "The Wash-Down";
				Offer("Arrivals at the wire need screening, and my screener has been on his feet for a day. {why} Sleeves up, eyes to the lamp, back of the hand to the forehead. Anything doubtful goes in the holding pen.",
					"Screening at the wire. {why} You've done fatigues here without a mark against you, so you get a lamp and a list. Bites are held. Fevers are washed. No discussion at the gate.");
				Over("Stand down. Screening is covered.");
				Said("The lamp's on the table. Use it on every one of them.", "Screening complete. Report what you held and what you passed.", "The wire's clear. Let's hear the numbers.");
				break;
			}

			case RZ_EFaction.BANDITS:
			{
				m_sTitle = "The Look-Over";
				Offer("Faces at the gate, and I don't know half of them. {why} Give them the look-over: sleeves, eyes, sweat. A bitten one inside costs more than any toll they'd pay!",
					"You've done enough round here that the lads know your name. So, the gate. {why} Look them over. If one's wrong, he sits in the shed till the medic's sober.");
				Over("Forget it. I looked at them myself and I didn't like any of them.");
				Said("Don't let them talk you round. They all talk.", "Done? Right. Who's in, who's in the shed?", "Gate's clear. Go on, then, tell me.");
				break;
			}

			case RZ_EFaction.INSTITUTE:
			{
				m_sTitle = "Intake";
				Offer("We have people waiting at intake and nobody free to examine them. {why} It's three observations each: forearms, pupils, temperature. I'd rather a careful amateur than a tired expert.",
					"Would you take intake for this shift? {why} You've been reliable, and the checklist is short. Anything you can't rule out goes to isolation, not past you.");
				Over("Intake has been covered. Thank you.");
				Said("The checklist is on the clipboard. Please follow its order.", "Intake is clear. I'll need your dispositions for the log.", "Thank you. Tell me what you decided for each.");
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineDead()
	{
		m_iRung = RZ_Duties.RUNG_FACE;
		m_sRoles = "nurse,medic,clerk,cook,lab assistant";
		m_sThread = "saw to the dead";
		TableWay(RZ_DutyGame.KIND_DEAD, false, "See to the dead",
			"Go to each of the dead laid out at {here}, look them over, and carry each to the pyre, the holding table or the grave. Then tell {giver}.", 6, 1);
		switch (m_iFaction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				m_sTitle = "Laying Out";
				Offer("There are dead to lay out, and it wants someone we know. {why} Check the arms and the neck, and check the name against the tag. The bitten are burned. A wrong name is held till someone's told.",
					"I wouldn't ask a stranger, and you aren't one now. {why} They're in the back room. Be gentle with them, and be certain. A bitten one in the ground doesn't stay in it.");
				Over("Never mind. It's been done, and I'd rather not say by whom.");
				Said("There's a sheet for each. Use it.", "That's them seen to. Sit a minute, then tell me.", "Thank you. Nobody thanks you for this one, so I am.");
				break;
			}

			case RZ_EFaction.MILITARY:
			{
				m_sTitle = "Burial Detail";
				Offer("Burial detail. {why} Each one is checked before it goes in the ground: forearms, neck, tags against papers. Bitten are burned, by order. Mismatched tags are held for the casualty return.",
					"I need the dead processed and I need it done right. {why} You've kept your nose clean here. Check, tag, sort. If the name's wrong, a family gets the wrong letter.");
				Over("Stand down. The detail's been found.");
				Said("Gloves on. Tags are read twice.", "Detail complete. Report your dispositions.", "Processed. Give me the count: buried, burned, held.");
				break;
			}

			case RZ_EFaction.BANDITS:
			{
				m_sTitle = "The Pit's Leavings";
				Offer("There's bodies wants sorting, and the lads won't touch them. {why} Arms, neck, and whatever's in the pockets that says who they were. Bitten ones burn. If the name's wrong, hold it, someone's owed.",
					"You've earned a job nobody wants. That's a promotion round here! {why} Sort the dead. Get it wrong and one of them sits up in the night, and then it's your job twice.");
				Over("Forget it. Lost at cards, did it myself.");
				Said("Don't go through the pockets twice. I count what comes back.", "Done? You look like it's done. Tell me.", "Right. Who burns, who's planted, who waits?");
				break;
			}

			case RZ_EFaction.INSTITUTE:
			{
				m_sTitle = "The Cold Room";
				Offer("The cold room needs clearing and the pathologist is one person. {why} Each subject is checked at the forearms and neck, and the paperwork against the tag. Bitten tissue is incinerated. Discrepancies are held.",
					"Might I ask you to take the cold room? {why} You've shown you follow a procedure. This one matters: an error leaves that room on its own feet.");
				Over("The cold room has been cleared. Thank you all the same.");
				Said("The forms are on the trolley. One per subject.", "Thank you. I'll take your dispositions for the register.", "The room is clear. Tell me what you recorded.");
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DefinePatrol()
	{
		m_iRung = RZ_Duties.RUNG_FACE;
		m_sRoles = "lookout,sentry,guard,toll man";
		m_sThread = "walked the round with their people";
		RZ_ErrandWay leg = Way("leg");
		leg.m_sLabel = "Walk the short leg with them";
		leg.m_ePlace = RZ_EErrandPlace.GATE;
		leg.m_fSeconds = 0;
		leg.m_fRadius = 30;
		leg.m_bSquad = true;
		leg.m_sStep = "Walk the short round with {squad} from the gate of {here}: three points, staying within 150 m of them, and back in with them. Then tell {giver}.";
		leg.m_iFavor = 5;
		leg.m_iFame = 1;
		RZ_ErrandWay far = Way("round");
		far.m_sLabel = "Take the long round, armed";
		far.m_ePlace = RZ_EErrandPlace.GATE;
		far.m_fSeconds = 0;
		far.m_fRadius = 30;
		far.m_bSquad = true;
		far.m_iMinTier = RZ_ETier.KNOWN;
		far.m_sStep = "Walk the long round with {squad} from the gate of {here}: four points out past 200 m, staying within 150 m of them, and back in with them. Then tell {giver}.";
		far.m_iFavor = 8;
		far.m_iFame = 1;
		switch (m_iFaction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				m_sTitle = "The Walk Round";
				Offer("Two of ours are about to walk the fence and beyond, and a third pair of eyes is welcome. {why} Go with them. You don't lead, you don't wander, and you come back when they do.",
					"The walk round's going out. {why} You've done right by this house, so you can go with them. Keep close. They'll stop if they lose sight of you.");
				Over("Never mind. They've gone without you, and come back.");
				Said("They're at the gate. Don't keep them standing.", "All back, then. That's the only report I care about.", "Everyone home? Good. Now tell me what was out there.");
				break;
			}

			case RZ_EFaction.MILITARY:
			{
				m_sTitle = "Patrol Leg";
				Offer("A patrol's stepping off and it's a rifle short. {why} You'll walk the leg with them: out, round the markers, back. You are under the corporal's orders from the gate to the gate.",
					"Patrol leg, if you want it. {why} You've a clean sheet here. Stay within sight of the section. If you drop back, they halt, and the corporal hates halting.");
				Over("Stand down. The patrol's out and back.");
				Said("They form up at the gate, and they won't form up twice.", "Patrol in. All accounted for. That's a good patrol.", "Section's back. Make your report.");
				break;
			}

			case RZ_EFaction.BANDITS:
			{
				m_sTitle = "The Round";
				Offer("The lads are doing the round and they could use a body at the back. {why} Walk it with them. See what they see. Say nothing clever!",
					"You've been useful, so here's a walk. {why} Round the markers with the lads and back. Fall behind and they'll wait, and they'll remember waiting.");
				Over("Too late. They went, and nobody got eaten.");
				Said("They're by the gate, scratching. Go on.", "All back with the same number of legs. Lovely.", "Back? Good. What's out there?");
				break;
			}

			case RZ_EFaction.INSTITUTE:
			{
				m_sTitle = "The Sample Walk";
				Offer("The survey pair are going round the collection points, and protocol wants a third. {why} You'd walk with them and observe. They carry the instruments. You carry the benefit of the doubt.",
					"Would you accompany the sample walk? {why} You've been dependable. Please stay close to them: they stop when they can't see you, and the samples don't keep.");
				Over("The walk has been completed without you. No harm done.");
				Said("They're waiting at the gate with the cases.", "Everyone returned. That is the result we most wanted.", "You're back, and so are they. What did you see?");
				break;
			}
		}
	}

	// ================================================================ Rung 3: armed work with their people
	//------------------------------------------------------------------------------------------------
	protected void DefineHunt()
	{
		m_iRung = RZ_Duties.RUNG_ARMED;
		m_sRoles = "farmer,cook,lookout,toll man";
		m_iNeed = RZ_ENeed.FOOD;
		m_sThread = "went out with the hunter";
		RZ_ErrandWay snares = Way("snares");
		snares.m_sLabel = "Walk the snare line with the hunter";
		snares.m_ePlace = RZ_EErrandPlace.OUT;
		snares.m_fSeconds = 30;
		snares.m_fRadius = 40;
		snares.m_bSquad = true;
		snares.m_sStep = "Go out with {squad} to the snare line {place}, check the snares there with the hunter beside you, then bring what's caught to {giver}.";
		snares.Work(RZ_EWork.SEARCH, "Check the snares", 15);
		snares.m_iFavor = 7;
		snares.m_iFame = 1;
		snares.m_iEffect = FX_FED;
		RZ_ErrandWay meat = Way("meat");
		meat.m_sLabel = "Hand over a leg of raw meat";
		meat.m_eItem = RZ_EErrandItem.MEAT;
		meat.m_iItems = 1;
		meat.m_iFavor = 8;
		meat.m_iFame = 1;
		meat.m_iEffect = FX_FED_MEAT;
		switch (m_iFaction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				m_sTitle = "For the Pot";
				Offer("Our hunter's going out and wants someone who can shoot and can keep quiet. {why} Walk the snare line with them. And if you've a deer's leg on you already, I'll take that and kiss you.",
					"You're known here now, so you can carry a gun past the gate with one of ours. {why} Go out with the hunter and come back with something that used to run.");
				Over("Never mind. They went alone and came back with a pigeon.");
				Said("They're waiting by the gate. They don't talk much. Don't make them.", "That's meat. Real meat. I'd forgotten the smell.", "Into the pot with it. You've fed a house today.");
				break;
			}

			case RZ_EFaction.BANDITS:
			{
				m_sTitle = "The Meat Run";
				Offer("The crew wants meat and the hunter wants a second gun. {why} Out to the snares, back with something dead that isn't walking. Or hand me a leg now and save your boots!",
					"You're crew enough to hunt with us. {why} Go with the hunter, mind the snares, and don't shoot at anything that shoots back.");
				Over("Forget it. The hunter went out drunk and came back fed.");
				Said("Hunter's at the gate. Keep up.", "Now that's a proper dinner. The boss'll hear whose it was.", "Meat! You can stay.");
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineBench()
	{
		m_iRung = RZ_Duties.RUNG_ARMED;
		m_sRoles = "lab assistant,technician";
		m_sThread = "worked the sample bench";
		TableWay(RZ_DutyGame.KIND_BENCH, false, "Work the sample bench",
			"Go to each sample tray set out at {here}, check the tray, and carry each to the rack, the bath or the bin. Then tell {giver}.", 8, 1);
		m_sTitle = "Bench Work";
		Offer("I can put you on the bench, now that we know you. {why} Each tray is checked three ways: the seal, the colour, the label against the log. A turned sample that gets filed is a wrong number in the count.",
			"We don't let many people near the trays. You're one of them now. {why} File what's sound, re-run what's mislabelled, destroy what's turned. If you can't tell, destroy it. We can draw another.");
		Over("The bench has been cleared by the last shift. I'll ask you again.");
		Said("Gloves and a mask, please. The trays are numbered.", "Thank you. I'll enter your dispositions against the log.", "The bench is clear. Tell me what you filed.");
	}

	// ================================================================ The Haven's own
	//------------------------------------------------------------------------------------------------
	protected void DefineHaven()
	{
		switch (m_sDuty)
		{
			case "school":
			{
				m_iRung = RZ_Duties.RUNG_CHORES;
				m_sRoles = "clerk,nurse";
				m_sThread = "minded the school room";
				m_sTitle = "The School Room";
				SceneWay("Mind the school room", false, "Count the heads in the school room at {here}, and find whoever is missing from the benches. Then tell {giver}.", 4, 0);
				Offer("The school room wants minding for an hour, and the teacher has a tooth. {why} Count them when you go in. Count them again when you come out. The numbers are meant to match.");
				Over("Never mind. The teacher's tooth came out and he's back.");
				Said("They'll try it on. They try it on with everyone.", "Twelve in, twelve out. You'd be surprised how often it isn't.", "Thank you. They liked you, which they'll deny.");
				break;
			}

			case "garden":
			{
				m_iRung = RZ_Duties.RUNG_CHORES;
				m_sRoles = "farmer,cook,storekeeper";
				m_iNeed = RZ_ENeed.FOOD;
				m_sThread = "worked the garden";
				m_sTitle = "The Garden";
				SceneWay("Work the garden", true, "Work the bed that needs it in the garden behind {here}, and see to whatever else the garden shows you. Then tell {giver}.", 4, 0);
				Offer("The garden's four beds and it's half of what we eat. {why} Go and see what it needs today. It'll be digging, or planting, or water, or lifting. And it'll be something else as well, it always is.");
				Over("Never mind. The children did it, and trod on half of it.");
				Said("The fork's in the first bed. Mind the string.", "Good. Whatever you did today, we eat it in a week.", "You've soil to the elbows. That's the right amount.");
				break;
			}

			case "doorround":
			{
				m_iRung = RZ_Duties.RUNG_FACE;
				m_sRoles = "lookout,clerk";
				m_sThread = "walked the door round";
				m_sTitle = "The Door Round";
				SceneWay("Walk the door round", true, "Knock and listen at each of the four doors on the round from {here}, and chalk the door where somebody is living unannounced. Then tell {giver}.", 6, 1);
				Offer("Four doors on the round tonight, and one of them has somebody behind it who hasn't told us. {why} Knock, listen, and use your head. Chalk the one. Chalk the wrong one and I send two people to a door they shouldn't open.");
				Over("Never mind. The round's been walked.");
				Said("Take the chalk. Knock like a neighbour, not like a patrol.", "Back. Which door, and why that one?", "Right. I'll send them in the morning, with bread.");
				break;
			}

			case "rollcall":
			{
				m_iRung = RZ_Duties.RUNG_FACE;
				m_sRoles = "clerk,cook,storekeeper";
				m_sThread = "took the roll";
				m_sTitle = "The Roll Call";
				SceneWay("Take the roll", false, "Find out who of {here} is on the list and wasn't fed, from what's in the house, and write why against the name. Then tell {giver}.", 6, 1);
				Offer("The list says twenty and the kitchen fed nineteen. {why} I could ask, and be told what people want me to hear. You go and look. Pegs, beds, the gate book. Then tell me who, and what's become of them.");
				Over("Never mind. They walked in at supper, asking what the fuss was.");
				Said("Look before you ask anyone. People tidy things, when asked.", "Well? Who, and where?", "Written. I'll do what has to be done about it.");
				break;
			}

			case "swap":
			{
				m_iRung = RZ_Duties.RUNG_ARMED;
				m_sRoles = "storekeeper,clerk,cook";
				m_sThread = "kept the swap table";
				m_sTitle = "The Swap Table";
				SceneWay("Keep the swap table", false, "Keep the swap table at {here}: handle what each of the four brings, ask about it, and call each trade yourself. Then tell {giver}.", 8, 1);
				Offer("It's market day and I need someone on the swap table who can't be talked round. {why} Four will come. Look at what they put down. Ask where it's from and who it's for. Then it's your call, each one. Fair isn't always the trade they ask for.");
				Over("Never mind. I took the table myself, and gave too much away.");
				Said("The box is under the cloth. Nothing goes in it you wouldn't explain to me.", "Table's cleared. Tell me what we took in, and what you sent away.", "Good. I'll hear about it from all four by supper, so tell me first.");
				break;
			}
		}
	}

	// ================================================================ Bastion's own
	//------------------------------------------------------------------------------------------------
	protected void DefineBastion()
	{
		switch (m_sDuty)
		{
			case "tape":
			{
				m_iRung = RZ_Duties.RUNG_CHORES;
				m_sRoles = "radio operator,mechanic";
				m_sThread = "changed the loudspeaker tape";
				m_sTitle = "The Loudspeaker Tape";
				SceneWay("Change the tape", true, "Go out to the loudspeaker on its pole beyond {here}, change the tape and test the speaker. Then tell {giver}.", 4, 0);
				Offer("The tape in the east speaker has stretched. It's telling people to turn back in a voice like a drowning man. {why} Take the new loop out, fit it, test it. It's loud. Don't hang about after.");
				Over("Stand down. Signals changed it.");
				Said("The new loop's in the tin. Keep your fingers off the oxide.", "Heard it from here. Good and clear.", "Changed and tested. That's one thing on this post that speaks plainly.");
				break;
			}

			case "gate":
			{
				m_iRung = RZ_Duties.RUNG_FACE;
				m_sRoles = "sentry,medic";
				m_sThread = "stood gate duty";
				m_sTitle = "Gate Duty";
				SceneWay("Take the barrier", true, "Take the barrier at the gate of {here}: look over the papers of each of the four who come, and pass, turn back or hold each one. Then tell {giver}.", 6, 1);
				Offer("Gate duty, one relief. {why} Four will come to the barrier. Hold the stamp to the light, read the dates, ask their business. You've time for two of the three. Some papers are forged, and some true papers are in the wrong hands. The barrier's yours.");
				Over("Stand down. The gate's manned.");
				Said("The lamp's on the hook. A forged stamp shows against it.", "Relieved. Who did you pass, and who's in my guard hut?", "Report. All four, in order.");
				break;
			}

			case "kits":
			{
				m_iRung = RZ_Duties.RUNG_FACE;
				m_sRoles = "sentry,mechanic,radio operator";
				m_sThread = "inspected the section's kit";
				m_sTitle = "Kit Inspection";
				SceneWay("Inspect the section's kit", false, "Inspect each of the three kits laid out at {here}, and pass, write up or deal with each one. Then clean the spare rifle and tell {giver}.", 6, 1);
				Offer("Kit inspection, and I want it done by someone the section can't get round. {why} Three kits. Count it, look down the barrel, go through the pack. You'll have time for two of those on each. If something's gone, find out where. That's worth more to me than the write-up.");
				Over("Stand down. The corporal inspected them.");
				Said("The list is on each blanket. They'll be standing by their bunks.", "Inspection complete. What's missing, and who's lying?", "Right. Let's have it, kit by kit.");
				break;
			}

			case "telephone":
			{
				m_iRung = RZ_Duties.RUNG_ARMED;
				m_sRoles = "radio operator,mechanic";
				m_sThread = "walked the telephone line";
				m_sTitle = "The Field Telephone";
				SceneWay("Walk the line", true, "Test the field telephone at {here}, walk the wire out to its break, work out what broke the wire, splice it and ring through. Then tell {giver}.", 8, 1);
				Offer("The line to the far post is dead. {why} Walk it out till you find the break. Then look at it properly before you touch it. Broken, cut and tapped are three different reports, and I act on each of them differently. Then splice it and ring through.");
				Over("Stand down. The line came back by itself, which worries me more.");
				Said("Handset, tape, cutters. And your rifle.", "Line's up. What was it, and what answered?", "Good. Now tell me what you saw at the break.");
				break;
			}
		}
	}

	// ================================================================ The Red Hand's own
	//------------------------------------------------------------------------------------------------
	protected void DefineRedHand()
	{
		switch (m_sDuty)
		{
			case "wreck":
			{
				m_iRung = RZ_Duties.RUNG_CHORES;
				m_sRoles = "mechanic,toll man";
				m_sThread = "stripped a wreck for the camp";
				m_sTitle = "Stripping a Wreck";
				SceneWay("Strip the wreck", true, "Go out to the wreck in the ditch beyond {here}, strip the wreck by hand and load the handcart. Then tell {giver}.", 4, 0);
				Offer("There's a car in the ditch up the road that nobody's had the wheels off yet. {why} Wheels, battery, seats. Bring it back on the handcart. And if there's anything under the seats, it's the camp's, mostly.");
				Over("Too slow. The lads had it down to the chassis by noon.");
				Said("Spanner's on the cart. Bring that back too.", "That's a week of spares. You can strip a car, then.", "Good. What was under the seats? There's always something.");
				break;
			}

			case "still":
			{
				m_iRung = RZ_Duties.RUNG_FACE;
				m_sRoles = "cook,mechanic";
				m_sThread = "ran a batch at the still";
				m_sTitle = "The Still";
				SceneWay("Run a batch", true, "Run a batch at the still behind {here}: load the still, fire it, keep the needle in its band, pour off the first cup and bottle the rest. Then tell {giver}.", 6, 1);
				Offer("The still wants running and the last one who ran it can't see straight. {why} Load it, light it, watch the needle. Pour the first cup on the ground, whatever anyone tells you. One bottle's yours, when it's done right.");
				Over("Forget it. The boss ran it himself. Nobody's saying how it came out.");
				Said("Don't smoke near it. I shouldn't have to say that.", "Six bottles and nobody blind. That's a batch.", "Done? Then pour me one, and we'll see what you made.");
				break;
			}

			case "count":
			{
				m_iRung = RZ_Duties.RUNG_FACE;
				m_sRoles = "toll man,lookout";
				m_sThread = "counted the haul";
				m_sTitle = "The Count";
				SceneWay("Count the haul", false, "Sort each of the four things off the tarp at {here} to the crew, the fence or the boss, then name whoever shorted the tally. Then tell {giver}.", 6, 1);
				Offer("There's a haul on the tarp wants counting, by someone who wasn't on the job. {why} What the crew can use goes to the crew. What only a fence wants goes to the fence. What's marked is the boss's. And read the tally as you go. Someone's been at it.");
				Over("Forget it. The boss counted it, and he was in a mood.");
				Said("Tarp's in the shed. The book's on the nail.", "Counted. Now, who's short?", "Right. A name. I'll take it to him myself.");
				break;
			}

			case "toll":
			{
				m_iRung = RZ_Duties.RUNG_FACE;
				m_sRoles = "toll man,lookout";
				m_sThread = "worked the toll";
				m_sTitle = "The Toll";
				SceneWay("Work the bar", true, "Work the toll bar on the road by {here}: look over each of the five who come up the road, and take the toll, wave them through or turn each back. Then tell {giver}.", 6, 1);
				Offer("The bar's yours till dusk. {why} Five will come up before then. Look in what they carry, look at their hands, ask where they're bound. You've time for two of those. Then you take the toll, or you wave them on, or you turn them round. The boss counts the box at the end, and I count it after him.");
				Over("Forget it. I took the bar back. You were too long about it.");
				Said("The box is under the bar. The stick's beside it. You won't need the stick, mostly.", "Shift's done. Who came, and what's in the box?", "Bar's down. Tell it, all five.");
				break;
			}

			case "spotter":
			{
				m_iRung = RZ_Duties.RUNG_ARMED;
				m_sRoles = "lookout";
				m_sThread = "lay up on the hill with the glass";
				m_sTitle = "Spotter";
				SceneWay("Take the glass up the hill", true, "Go up the hill beyond {here}, lie up with the glass, watch three times, and mark on the slate the one thing the crew can use. Then tell {giver}.", 8, 1);
				Offer("I want a pair of eyes on the hill, and the boss said yours. {why} Lie up, watch, and watch again. Don't count everything. Find the one thing we can use, and put that on the slate. One line. The crew goes out on what you write.");
				Over("Forget it. The hill's been watched. You'd only have counted sheep.");
				Said("Take the glass. Keep the sun off it.", "Down in one piece. What's on the slate?", "Right. One line. Let's see if you can see.");
				break;
			}
		}
	}

	// ================================================================ Halcyon's own
	//------------------------------------------------------------------------------------------------
	protected void DefineHalcyon()
	{
		switch (m_sDuty)
		{
			case "samples":
			{
				m_iRung = RZ_Duties.RUNG_FACE;
				m_sRoles = "lab assistant,technician";
				m_sThread = "walked the sample round";
				m_sTitle = "The Sample Round";
				SceneWay("Walk the sample round", true, "Take a swab at each of the four marked spots out from {here}, and mark on the station's map where the infection is coming from. Then tell {giver}.", 6, 1);
				Offer("The sample round is due and my field team is in the wash. {why} Four marked spots. A swab at each, and the strip tells you how much is there. Don't just bring me four tubes. Tell me where it's coming from. That's the result.");
				Over("The round has been walked. Thank you for offering.");
				Said("The suit's on the hook. The strips are in the left pocket.", "You're back. One mark on the map, please, and your reasons.", "Thank you. Now, where?");
				break;
			}

			case "decon":
			{
				m_iRung = RZ_Duties.RUNG_FACE;
				m_sRoles = "lab assistant,guard,technician";
				m_sThread = "kept the decon line";
				m_sTitle = "The Decon Line";
				SceneWay("Keep the decon line", true, "Keep the decon line at the door of {here}: check each of the five who come through, and pass, re-run, hold or report each one. Then close the line and tell {giver}.", 6, 1);
				Offer("I need the decon line kept for a shift, by someone who'll actually look. {why} Five are due through. The lamp over the suit, the seals and the visor, the case and the log. You've time and disinfectant for two of those on each. Pass what's clean. Don't pass what isn't. And don't hold a clean one because you're frightened.");
				Over("The line has been covered. Thank you.");
				Said("The lamp's by the door. The drum is a third full, so think before you re-run.", "The line's closed. I'll read your log now, with you standing there.", "Five through. Tell me about each.");
				break;
			}

			case "incinerator":
			{
				m_iRung = RZ_Duties.RUNG_FACE;
				m_sRoles = "guard,technician";
				m_sThread = "fed the incinerator";
				m_sTitle = "The Incinerator";
				SceneWay("Take the bags to the pit", true, "Take each of the four sealed bags to the burn pit behind {here}, and burn, report or open each one. Then settle the log and tell {giver}.", 6, 1);
				Offer("There are bags for the pit, and the porter who does it has been reassigned. {why} Each has a label and a line in the log. Read the label, lift the bag, feel it. If those three agree, burn it. If they don't, that's the interesting part, and I'd like to hear about it before it's ash.");
				Over("The pit's been fed. No need now.");
				Said("The log's on the nail by the pit. Don't breathe the smoke.", "Done. Did the log and the bags agree?", "You smell of it. Go on, what was in them?");
				break;
			}

			case "ward":
			{
				m_iRung = RZ_Duties.RUNG_ARMED;
				m_sRoles = "doctor,lab assistant,technician";
				m_sThread = "took the observation ward's round";
				m_sTitle = "The Observation Ward";
				SceneWay("Take the ward round", false, "Take the round of the three beds on the observation ward at {here}: read the signs at each bed, say what it is and what's to be done. Then tell {giver}.", 8, 1);
				Offer("The observation ward needs its round, and I trust your eyes more than a tired nurse's. {why} Three beds. Temperature, pulse, the eyes, the skin, how they speak, the chart. You won't have time for all six on each. Then you write what it is, and you say what's done about it. If one of them is turning and you write fever, that is how a ward is lost.");
				Over("The round has been done. The night nurse took it.");
				Said("The torch is on the trolley. Wash between beds.", "Three beds. What did you write, and what did you do?", "Thank you. Sit down first. Then tell me.");
				break;
			}

			case "coldchain":
			{
				m_iRung = RZ_Duties.RUNG_ARMED;
				m_sRoles = "technician,guard";
				m_sThread = "carried the cold box";
				m_sTitle = "The Cold Chain";
				SceneWay("Carry the cold box", false, "Take the cold box from {here} and carry the box to the far door, keeping to its rule whatever the road does. Then sign the manifest and tell {giver}.", 8, 1);
				Offer("There's a box to go to the far door, and it can't go by truck. {why} I'll tell you what's in it and the one thing it can't bear. After that it's your road. Things will happen on it. I shan't be there to ask. Decide by the rule.");
				Over("The box has gone with a courier. Thank you.");
				Said("It's in the cold room, on the second shelf. Read the tag before you lift it.", "It's there? Then tell me what the road did, and what you did.", "Signed for. Good. Now the truth of how it went.");
				break;
			}
		}
	}

	// ================================================================ A watch, the stores, and the Trusted rung
	//------------------------------------------------------------------------------------------------
	//! A way played as a scene (RZ_DutyScenes): its beats are the step.
	protected RZ_ErrandWay SceneWay(string label, bool gate, string step, int favor, int fame)
	{
		RZ_ErrandWay w = Way("scene");
		w.m_sLabel = label;
		w.m_ePlace = RZ_EErrandPlace.HERE;
		if (gate)
			w.m_ePlace = RZ_EErrandPlace.GATE;

		w.m_fSeconds = 0;
		w.m_fRadius = 40;
		w.m_sStep = step;
		w.m_iFavor = favor;
		w.m_iFame = fame;
		return w;
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineWatch()
	{
		m_iRung = RZ_Duties.RUNG_FACE;
		m_sRoles = "lookout,sentry,guard,toll man";
		m_sThread = "stood a watch";
		SceneWay("Take the post", true, "Take the post at the gate of {here} and stand the watch: three things will come by. Stand the gate to for a threat, send someone out to a person, let the rest pass. Then tell {giver}.", 6, 1);
		switch (m_iFaction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				m_sTitle = "The Roof at Dusk";
				Offer("Someone has to sit the roof while the light goes, and I'd rather it was someone who thinks before they shout. {why} You'll see things. Most of them are nothing. Wake the house for the ones that aren't.");
				Over("Never mind. The roof's been sat.");
				Said("The ladder's round the side. Take a blanket.", "Down you come. Tell me what you saw, and what you made of it.", "A quiet roof is a good roof. Tell me anyway.");
				break;
			}

			case RZ_EFaction.MILITARY:
			{
				m_sTitle = "Stag";
				Offer("Stag at the wire, one relief. {why} You report what moves and you decide what it is. Stand the gate to for nothing and the section will hate you. Miss one, and there won't be a section.");
				Over("Stand down. Stag is covered.");
				Said("You're relieved when you're relieved, and not a minute sooner.", "Stag complete. Let's have it.", "Relieved. What did you have?");
				break;
			}

			case RZ_EFaction.BANDITS:
			{
				m_sTitle = "The Quiet Hour";
				Offer("Sit the gate for the quiet hour. {why} It's quiet because someone's watching. Shout for the right things. The lads don't thank you for the wrong ones!");
				Over("Forget it. Someone couldn't sleep and took it.");
				Said("There's a crate to sit on. Don't get comfortable.", "Still here, and so are we. What came by?", "Right. Tell it.");
				break;
			}

			case RZ_EFaction.INSTITUTE:
			{
				m_sTitle = "The Gate Log";
				Offer("The gate log wants an observer for the evening interval. {why} You record what approaches and classify it. A false alarm costs the night shift an hour. A missed one costs rather more.");
				Over("The interval has been covered. Thank you.");
				Said("The log is on the hook. Times in the left column.", "Thank you. I'll read your entries now.", "The interval is closed. What did you record?");
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineStock()
	{
		m_iRung = RZ_Duties.RUNG_ARMED;
		m_sRoles = "storekeeper,clerk,mechanic,technician,toll man";
		m_sThread = "took stock of the stores";
		TableWay(RZ_DutyGame.KIND_STOCK, false, "Take stock of the stores",
			"Go to each crate set out at {here}, check the crate against the book, and carry each to the counted stack, the written-off pile or the query table. Then tell {giver}.", 8, 1);
		switch (m_iFaction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				m_sTitle = "The Larder Book";
				Offer("The larder wants counting against the book, and it wants someone who isn't from here to do it. {why} If a crate's short and the seal's been cut, that goes on the table and someone answers. If it's only my handwriting, write it off.");
				Over("Never mind. I counted it myself, twice, and cried once.");
				Said("The book's on the shelf. My sevens look like ones.", "Counted. Sit down and tell me who I have to talk to.", "That's the larder. Now tell me the worst of it.");
				break;
			}

			case RZ_EFaction.MILITARY:
			{
				m_sTitle = "The Armoury Count";
				Offer("Armoury count. {why} Every crate is checked against the ledger: contents, seal, chit. A cut seal and a short count is a charge. A wrong chit is a clerk. Know the difference before you ruin a man.");
				Over("Stand down. The count's been done.");
				Said("Ledger's chained to the desk. It stays chained.", "Count complete. Report discrepancies.", "Signed off. Now, who do I see?");
				break;
			}

			case RZ_EFaction.BANDITS:
			{
				m_sTitle = "Counted Twice";
				Offer("The boss wants the stores counted by someone with no friends in them. That's you. {why} If it's short and the seal's cut, somebody's been at it, and I want the crate on my table. If it's just the tally wrong, lose it quietly.");
				Over("Too late. The boss counted it himself. Somebody's limping.");
				Said("Count it like it's yours, and remember whose it is.", "Done? Who's been at it, then?", "Right. Names. Or crates, and I'll find the names.");
				break;
			}

			case RZ_EFaction.INSTITUTE:
			{
				m_sTitle = "The Cold Store";
				Offer("The cold store is due its audit, and it should be someone outside the department. {why} Contents against the register, the seal, the requisition slip. A broken seal with a short count is a matter for the director. A clerical slip is not.");
				Over("The audit has been completed. Another quarter, perhaps.");
				Said("The register is by the door. Please initial each line.", "Thank you. I'll take the discrepancies first.", "The store is audited. Tell me what you found.");
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineLockUp()
	{
		m_iRung = RZ_Duties.RUNG_TRUST;
		m_sRoles = "sentry,guard,lookout,toll man,medic";
		m_sThread = "kept the door on the lock-up";
		SceneWay("Take the night watch on the door", true, "Take the watch at the lock-up door by the gate of {here}, and keep the door until the one inside has had their say. Then tell {giver}.", 10, 2);
		switch (m_iFaction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				m_sTitle = "The One at the Gate";
				Offer("There's someone in the gate room for the night, and we don't know yet what they are. {why} I need the door kept by someone I'd trust with my own. They'll talk. People do, in the dark. What you do about it is yours to carry.");
				Over("Never mind. They were let in at supper. Nothing wrong with them but fear.");
				Said("The bar's on the outside. That's all there is to it.", "Morning. Tell me how the night went.", "You look like you've been spoken to. Go on.");
				break;
			}

			case RZ_EFaction.MILITARY:
			{
				m_sTitle = "The Stockade Watch";
				Offer("Stockade watch, the night relief. One prisoner, awaiting the captain. {why} You don't speak to him and you don't open that door. He'll speak to you. Whether it goes in the watch book is your decision, and your name goes beside it.");
				Over("Stand down. The prisoner's been moved.");
				Said("Keys stay on the hook. You have the bar.", "Watch relieved. Anything for the book?", "Report, and leave nothing out.");
				break;
			}

			case RZ_EFaction.BANDITS:
			{
				m_sTitle = "The Debtor in the Shed";
				Offer("There's a man in the shed who owes, and the boss wants him there in the morning. {why} Sit on the door. He'll tell you a story. They've all got a story! You're trusted, so you get to decide if it's a good one.");
				Over("Forget it. He paid. Don't ask what with.");
				Said("Sit on the crate. Keep the bar down.", "Still in there? Good. What did he say?", "Morning. Well?");
				break;
			}

			case RZ_EFaction.INSTITUTE:
			{
				m_sTitle = "The Isolation Room";
				Offer("One of the subjects is in the isolation room overnight, on observation. {why} The protocol says the door stays shut until the morning reading. They will talk to you through the hatch. What you write down goes in their file, and what you do with the door is on yours.");
				Over("Stand down. The morning reading came early. They've been moved.");
				Said("The hatch opens from your side only. Don't open the door.", "The night's over. What did they say?", "Thank you. Put it in the file, if it belongs there.");
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineNames()
	{
		m_iRung = RZ_Duties.RUNG_TRUST;
		m_sThread = "carried the names";
		if (m_iFaction == RZ_EFaction.MILITARY)
		{
			m_sRoles = "radio operator,medic,sentry";
			m_sTitle = "The Casualty Return";
			SceneWay("Make up the casualty return", false, "Go to each of the three bunks at {here}, read what was left at the bunk, and return each man killed, missing or gone. Then sign the return and tell {giver}.", 10, 2);
			Offer("The casualty return is three lines short and it goes up tonight. {why} Go to each man's bunk and look at what he left. Killed, missing, or walked. The return says which, and that word follows him and his family. Get it right.");
			Over("Stand down. The adjutant made up the return.");
			Said("Their bunks haven't been touched. Keep it that way.", "Return received. Three lines. I'll read them now.", "Signed. I hope you looked properly.");
			return;
		}

		if (m_iFaction == RZ_EFaction.BANDITS)
		{
			m_sRoles = "toll man,mechanic,cook";
			m_sTitle = "The Debt Book";
			m_sThread = "kept the boss's book";
			SceneWay("Go round the boss's book", false, "Go to the door of each of the three in the boss's debt book near {here}, see what's there, and mark each owed, written off or paid. Then press your thumb under the lines and tell {giver}.", 10, 2);
			Offer("The boss wants the book squared before the week turns. {why} Three names, three doors. Go and look. Some are hiding it, some haven't got it, and some paid and the book never heard. Whatever you write, the crew acts on it.");
			Over("Forget it. The boss did the round himself. Nobody enjoyed it.");
			Said("Look before you write. That's all he asks.", "The book. Let's see your lines.", "Squared. He'll read it tonight.");
			return;
		}

		if (m_iFaction == RZ_EFaction.INSTITUTE)
		{
			m_sRoles = "lab assistant,technician";
			m_sTitle = "Whose Work It Was";
			m_sThread = "put the names on the week's work";
			SceneWay("Read the desks for the week's report", false, "Go to each of the three desks at {here}, read what's on it, and place each name on the week's report: first, thanked, or left off. Then initial it and tell {giver}.", 10, 2);
			Offer("The week's bench report goes in the record tonight, and the names on it are wrong more often than they're right. {why} Read the desks. The notebooks don't lie, even when the people do. Whatever you place stays on their record.");
			Over("Never mind. The director placed the names himself.");
			Said("The notebooks are on the desks. Read the hands, not the covers.", "The report. Show me where you put them.", "Initialled. Thank you. It's the first one in a month I'd sign.");
			return;
		}

		m_sRoles = "clerk,cook,nurse";
		m_sTitle = "The Book of Names";
		SceneWay("Carry the names to their doors", false, "Go to the door of each of the dead of {here} and tell whoever is inside, in your own way. Then write the names in the book by the stove and tell {giver}.", 10, 2);
		Offer("We lost people this week, and their own don't all know yet. {why} I can't send a stranger to those doors, and I can't go to all of them myself. You're one of us now. Tell them however you think is right. Then write the names in the book.");
		Over("Never mind. I went myself. I shouldn't have asked.");
		Said("Take your time at each door. That's all the kindness there is.", "It's done, then. Sit. You don't have to tell me how each one took it.", "Thank you. Nobody should have to, and somebody always does.");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineList()
	{
		m_iRung = RZ_Duties.RUNG_TRUST;
		m_sRoles = "doctor,lab assistant,technician";
		m_sThread = "wrote a name on the trial list";
		m_sTitle = "The Trial List";
		SceneWay("Choose who goes on the trial list", false, "Read the chart at each of the three beds at {here}, then write one name on the trial list by the door. Then tell {giver}.", 10, 2);
		Offer("There is one place on the trial list this week and three people it could go to. {why} The staff are too close to choose, and the director would choose by the numbers alone. You're trusted, and you're not one of us. Read the charts. Write one name.");
		Over("The place has been filled. The director decided.");
		Said("The charts are at the foot of each bed. Take as long as you need.", "It's written. I'll tell the other two myself. That part isn't yours.", "Thank you. Whatever you chose, somebody had to.");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineDose()
	{
		m_iRung = RZ_Duties.RUNG_TRUST;
		m_sThread = "wrote a name on the dose";
		switch (m_iFaction)
		{
			case RZ_EFaction.MILITARY:
			{
				m_sRoles = "medic,sentry";
				m_sTitle = "The Sergeant's List";
				SceneWay("Read the three beds and name one", false, "Read the chart at each of the three beds at {here}, then write one name on the card in the dose case. Then tell {giver}.", 10, 2);
				Offer("The lab sent one dose for the post. There are three on the sergeant's list. {why} He won't choose, and he says I can't. You can. Read the beds. Whoever you write, the post will know it was you.");
				Over("Stand down. The sergeant chose in the end.");
				Said("The case stays shut until there's a name on the card.", "It's done. Who?", "Done. I'll tell the other two. That part's mine.");
				break;
			}

			case RZ_EFaction.BANDITS:
			{
				m_sRoles = "toll man,cook";
				m_sTitle = "Who the Crew Pays For";
				SceneWay("Read the three beds and name one", false, "Read the three bitten at {here}, then write one name on the card on the fence's dose. Then tell {giver}.", 10, 2);
				Offer("The fence has one dose on his shelf, and the boss will pay for it, for one of ours. {why} Three are bitten. He says you choose, so nobody can say he chose. Read them. Write a name, or put it back on the shelf.");
				Over("Forget it. The fence sold it while we were talking.");
				Said("The card's on the case. One name.", "Well? Who's it for?", "Done. The boss paid. Don't expect thanks from the other two.");
				break;
			}

			default:
			{
				m_sRoles = "nurse,clerk";
				m_sTitle = "The House's One Dose";
				SceneWay("Read the three beds and name one", false, "Read the chart at each of the three beds at {here}, then write one name on the card in the dose case. Then tell {giver}.", 10, 2);
				Offer("The lab gave the house one dose. There are three in the back room who need it. {why} We can't choose. We know them. You know them a little, which is better. Read them, and write a name, or send it back.");
				Over("Never mind. The three of them decided, between themselves.");
				Said("Take your time with each. They're awake.", "It's done, then. Sit down a minute.", "Thank you. I couldn't have.");
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineAnimals()
	{
		m_iRung = RZ_Duties.RUNG_TRUST;
		m_sRoles = "lab assistant,technician";
		m_sThread = "went through the animal house";
		m_sTitle = "The Animal House";
		SceneWay("Go through the animal house", false, "Go through the three cages at {here}: read each animal and put it down, keep it for the trial, or move it to the clean room. Then sign the log and tell {giver}.", 10, 2);
		Offer("The animal house needs going through tonight, and the keeper won't do it any more. {why} Three cages. Some are turning, some are getting better, and some cards are on the wrong cage. A turning one kept is one that gets out.");
		Over("Never mind. The director had them all put down. Every one.");
		Said("Gloves, and don't put your hand through the bars.", "The log. What did you do with them?", "Signed. The keeper will want to know which ones.");
	}

	//------------------------------------------------------------------------------------------------
	protected void DefineCull()
	{
		m_iRung = RZ_Duties.RUNG_TRUST;
		m_sRoles = "lookout,sentry,guard,farmer";
		m_sThread = "cleared the strays with their people";
		RZ_ErrandWay w = Way("cull");
		w.m_sLabel = "Go out with them and clear it";
		w.m_ePlace = RZ_EErrandPlace.GATE;
		w.m_fSeconds = 0;
		w.m_fRadius = 30;
		w.m_bSquad = true;
		w.m_sStep = "Go out armed with {squad} from the gate of {here} to where the strays gather, stand with {squad} until it's clear, and come back in with them. Then tell {giver}.";
		w.m_iFavor = 10;
		w.m_iFame = 2;
		if (m_iFaction == RZ_EFaction.MILITARY)
		{
			m_sTitle = "Clearing the Wire";
			Offer("There are strays gathering off the wire, a handful, and a handful becomes a hundred. {why} A section's going out to clear them and I want you on it. You've earned a place in the line. Hold it.");
			Over("Stand down. The section cleared it without you.");
			Said("They're at the gate. Check your magazine twice.", "All back. Clear. That's the whole report.", "Section's in. Good work out there.");
			return;
		}

		m_sTitle = "The Strays";
		Offer("There are strays at the bottom field again. A few, off some pack, and they bring the rest in time. {why} Two of ours are going down to see to it and they asked for you by name. That's not nothing, here.");
		Over("Never mind. They've gone down and come back, and nobody's saying how it went.");
		Said("They're waiting at the gate. Don't let them go first.", "Everyone home. That's the only count I want.", "It's done. Wash before you come in the kitchen.");
	}

	//------------------------------------------------------------------------------------------------
	//! Done once at a place on a season day (the Trusted rung's: nothing farmable).
	protected string DayKey(RZ_Errand e)
	{
		int place = e.m_iHouse;
		if (e.m_iHolding >= 0)
			place = 1000 + e.m_iHolding;

		return string.Format("duty.%1.%2.%3", m_sDuty, place, RZ_SeasonClock.Day());
	}

	//------------------------------------------------------------------------------------------------
	//! What's so for the duties that aren't always there: a watch at dusk or night, the lock-up at
	//! night, the names when the house has dead, the return from the Collapse on, the list once the
	//! cure is 40% made, a cull by a town 30+ infected.
	protected bool TrueNow(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_WorldFacts facts = gm.GetFacts();
		bool dark = facts && (facts.IsDusk() || facts.IsNight());
		if (m_iRung >= RZ_Duties.RUNG_TRUST && dealer.Counter(DayKey(e)) > 0)
			return false;

		switch (m_sDuty)
		{
			case "watch": return dark;
			case "lockup": return dark;
			case "doorround": return dark;
			case "school": return !dark;
			case "garden": return !dark && e.m_iHouse >= 0;
			case "swap": return !dark;
			case "gate": return !dark;
			case "samples": return !dark;
			case "ward": return facts && !facts.IsCured();
			case "toll": return !dark;
			case "wreck": return !dark;
			case "spotter": return !dark;
			case "names":
			{
				if (m_iFaction == RZ_EFaction.MILITARY)
					return facts && facts.Stage() >= RZ_EOutbreakStage.COLLAPSE;

				if (m_iFaction == RZ_EFaction.BANDITS)
					return !dark;

				if (m_iFaction == RZ_EFaction.INSTITUTE)
					return facts && facts.CurePercent() >= 20 && !facts.IsCured();

				if (e.m_iHouse < 0 || !facts)
					return false;

				RZ_HouseFacts h = facts.House(e.m_iHouse);
				return h && h.m_iDied > 0;
			}

			case "list": return facts && facts.CurePercent() >= 40 && !facts.IsCured();
			case "animals": return facts && !facts.IsCured();
			case "dose":
			{
				RZ_CureResearch cure = gm.GetCure();
				if (!cure || cure.IsCured())
					return false;

				if (m_iFaction == RZ_EFaction.BANDITS)
					return cure.GetStolenAntidotes() > 0;

				return cure.GetLabAntidotes() > 0;
			}
			case "cull":
			{
				RZ_Settlement town = gm.GetSettlement(e.m_iSettlement);
				return town && town.m_fInfection >= 30 && RZ_Escort.HasRoom(gm);
			}
		}

		return true;
	}

	// ================================================================ Who, why, still
	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return RZ_Duties.Rung(dealer, playerId, m_iFaction) >= m_iRung;
	}

	//------------------------------------------------------------------------------------------------
	//! Always true while the place stands: the run's task and the reason said for it are drawn here.
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (e.m_iHouse >= 0 && gm.GetSafehouses() && gm.GetSafehouses().IsExiled(e.m_iAsker))
			return false;

		if (!TrueNow(dealer, e))
			return false;

		if (m_iTasks > 0)
			e.m_iValue = Math.RandomInt(0, m_iTasks);

		e.SetText("why", Reason(dealer, e));
		e.SetText("squad", SquadName(null));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Still(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (e.m_iHouse >= 0)
		{
			RZ_HouseFacts h = gm.GetFacts().House(e.m_iHouse);
			return h && h.m_bValid && !h.m_bEmpty;
		}

		if (e.m_iHolding >= 0 && gm.GetTerritory())
			return gm.GetTerritory().GetHolding(e.m_iHolding) != null;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! One task a run: the one drawn for it. Every other way shows.
	override bool Offers(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w)
	{
		if (w.m_iGame <= 0 && w.m_sWay.StartsWith("t"))
			return w.m_sWay == "t" + e.m_iValue.ToString();

		// Going out with their people needs room for one more escort on the server
		if (w.m_bSquad && e.m_eState != RZ_EErrandState.HELD)
			return RZ_Escort.HasRoom(dealer.GetGM());

		return true;
	}

	// ================================================================ The reason, from the world
	//------------------------------------------------------------------------------------------------
	//! Why this duty, now: one sentence from what's so (the house's last meal, a death, the rain, the
	//! hour, the outbreak's stage, what the faction is short of), else one of the everyday ones.
	protected string Reason(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_WorldFacts facts = gm.GetFacts();
		array<string> so = {};
		RZ_HouseFacts h;
		if (e.m_iHouse >= 0 && facts)
			h = facts.House(e.m_iHouse);

		bool rain = gm.GetEnvironment() && gm.GetEnvironment().IsRaining();
		bool dusk = facts && facts.IsDusk();
		bool night = facts && facts.IsNight();
		bool late = facts && facts.Stage() >= RZ_EOutbreakStage.COLLAPSE;
		bool died = h && h.m_iDied > 0;
		bool sick = false;
		RZ_Settlement town = gm.GetSettlement(e.m_iSettlement);
		if (town && town.m_fInfection >= 50)
			sick = true;

		RZ_FactionLife life = gm.GetFactionLife();
		bool hungry = life && life.GetNeed(m_iFaction) == RZ_ENeed.FOOD;
		switch (m_sDuty)
		{
			case "kitchen":
			{
				if (h && h.m_bShort)
					so.Insert("The last meal ran short, so this one has to stretch.");

				if (died)
					so.Insert("There was a burying this week, and people eat after a burying.");

				if (rain)
					so.Insert("The rain's driven everyone indoors, and indoors they get hungry.");

				if (dusk)
					so.Insert("Supper's due before dark and the pot isn't even on.");

				if (night)
					so.Insert("The night watch eats at midnight and nothing's ready for them.");

				if (hungry)
					so.Insert("The stores are low, so every scrap has to go in and none of it wasted.");

				if (so.IsEmpty())
				{
					so.Insert("It's the same mouths as yesterday and two fewer hands.");
					so.Insert("The one who does it is on the gate today.");
					so.Insert("People work on what they've eaten, and they've work to do.");
				}

				break;
			}

			case "wood":
			{
				if (rain)
					so.Insert("It's wet, and wet wood won't catch unless it's split thin.");

				if (night || dusk)
					so.Insert("It gets cold after dark and the stack is down to the bark.");

				if (late)
					so.Insert("Nothing's coming up the road any more, so what we burn is what we cut.");

				if (died)
					so.Insert("The one who used to do it isn't here to do it.");

				if (so.IsEmpty())
				{
					so.Insert("The stove eats a stack a day and gives nothing back.");
					so.Insert("Nobody's carried a drop since this morning.");
					so.Insert("It was done yesterday, which is the trouble with it.");
				}

				break;
			}

			case "wash":
			{
				if (sick && town)
					so.Insert(string.Format("There's sickness in %1, and it comes in on people's boots.", town.m_sName));

				if (died)
					so.Insert("Someone died in there this week, and nobody's had the heart to scrub it.");

				if (rain)
					so.Insert("The rain's brought the mud in, and half the yard with it.");

				if (late)
					so.Insert("There's no doctor coming now, so nobody here can afford a fever.");

				if (so.IsEmpty())
				{
					so.Insert("It's been three days, and it shows.");
					so.Insert("A dozen people came through today and every one of them touched something.");
					so.Insert("You can't see what kills you, so we wash everything.");
				}

				break;
			}

			case "line":
			{
				if (sick && town)
					so.Insert(string.Format("They're coming in from %1 now, and %1 is bad.", town.m_sName));

				if (rain)
					so.Insert("It's raining, so they won't queue long before they start to push.");

				if (late)
					so.Insert("The post that used to check them up the road is gone.");

				if (night)
					so.Insert("They've come in after dark, which is never a good sign.");

				if (so.IsEmpty())
				{
					so.Insert("Six since breakfast, and more on the road.");
					so.Insert("The one who does it has been at it since yesterday.");
				}

				break;
			}

			case "dead":
			{
				if (died)
					so.Insert("We lost our own this week, and they're laid out with the rest.");

				if (sick && town)
					so.Insert(string.Format("%1 sent us theirs. They've nobody left to do it.", town.m_sName));

				if (late)
					so.Insert("There are more of them every day now, and fewer of us.");

				if (so.IsEmpty())
				{
					so.Insert("They've waited long enough.");
					so.Insert("Nobody wants it, which is why it's come to you.");
				}

				break;
			}

			case "patrol":
			{
				if (night)
					so.Insert("It's dark, and the dark is when the fence gets tested.");

				if (rain)
					so.Insert("Rain covers noise, theirs and ours.");

				if (sick && town)
					so.Insert(string.Format("Something has stirred up %1, and it drifts this way.", town.m_sName));

				if (dusk)
					so.Insert("Last light. Whatever's out there starts moving now.");

				if (so.IsEmpty())
				{
					so.Insert("It's walked twice a day whether anything's out there or not.");
					so.Insert("The last walk found a door open that had been shut.");
				}

				break;
			}

			case "hunt":
			{
				if (hungry)
					so.Insert("The stores are down, and the pot knows it.");

				if (late)
					so.Insert("No truck is coming, so it's what we catch.");

				if (dusk)
					so.Insert("They come down to the field's edge at dusk.");

				if (rain)
					so.Insert("Wet ground holds a track. Today we can follow one.");

				if (so.IsEmpty())
				{
					so.Insert("The snares want checking before the crows do it for us.");
					so.Insert("Nobody here has tasted meat since last week.");
				}

				break;
			}

			case "samples":
			{
				if (sick && town)
					so.Insert(string.Format("The count in %1 doubled in a week, and it's coming from somewhere.", town.m_sName));

				if (rain)
					so.Insert("It's rained, so whatever's in the ground is in the water now.");

				if (so.IsEmpty())
				{
					so.Insert("The last round's strips didn't agree with each other, and I want to know why.");
					so.Insert("Something's feeding it out there, and it isn't where we've been looking.");
				}

				break;
			}

			case "decon":
			{
				if (sick && town)
					so.Insert(string.Format("They've been in %1 all day, and it's the worst it has been.", town.m_sName));

				if (late)
					so.Insert("We've no spare suits now, so people are mending their own, badly.");

				if (so.IsEmpty())
				{
					so.Insert("The last shift passed everyone in under a minute each.");
					so.Insert("Something came through that door last week, and I still don't know on whom.");
				}

				break;
			}

			case "incinerator":
			{
				if (late)
					so.Insert("People have started using the pit to lose things, now nobody checks.");

				if (rain)
					so.Insert("Half of it's wet through, and wet doesn't burn clean.");

				if (so.IsEmpty())
				{
					so.Insert("The log had nine bags yesterday, and the ash was eleven bags deep.");
					so.Insert("One of last week's bags came back out of the ash, part of it. I'd rather not say which part.");
				}

				break;
			}

			case "ward":
			{
				if (facts && facts.CurePercent() >= 50)
					so.Insert("The trial is past halfway, and every bed on that ward is a number in it.");

				if (night)
					so.Insert("They turn at night, the ones who turn. It's nearly always night.");

				if (so.IsEmpty())
				{
					so.Insert("One of them is lying to us, and one of them is worse than they look.");
					so.Insert("The last round wrote fever on all three, and I don't believe it.");
				}

				break;
			}

			case "coldchain":
			{
				if (rain)
					so.Insert("The road's a river, so no truck will take it.");

				if (late)
					so.Insert("There's no fuel for the truck, and the box won't wait for fuel.");

				if (so.IsEmpty())
				{
					so.Insert("The truck was stopped twice last week, and opened once.");
					so.Insert("A person on foot is the one thing nobody thinks to search.");
				}

				break;
			}

			case "wreck":
			{
				if (late)
					so.Insert("Nobody's making parts any more, so every wreck's a shop.");

				if (rain)
					so.Insert("Another week of this rain and it'll be rust.");

				if (so.IsEmpty())
				{
					so.Insert("The lorry wants a wheel and the boss wants the lorry.");
					so.Insert("Somebody else has seen it by now, and they've a spanner too.");
				}

				break;
			}

			case "still":
			{
				if (hungry)
					so.Insert("We've nothing left to trade but drink, so the drink had better be good.");

				if (rain)
					so.Insert("It's a wet day, and the smoke won't show.");

				if (so.IsEmpty())
				{
					so.Insert("The camp's dry, and a dry camp starts thinking.");
					so.Insert("There's a mash been sitting three days and it won't sit a fourth.");
				}

				break;
			}

			case "count":
			{
				if (late)
					so.Insert("There's less coming in every week, so every bit of it gets counted.");

				if (so.IsEmpty())
				{
					so.Insert("The last count came up short and nobody could say where.");
					so.Insert("The lads who brought it in each say the others had the tarp.");
				}

				break;
			}

			case "toll":
			{
				if (sick && town)
					so.Insert(string.Format("Half of %1 is on the road, and some of them are bringing it with them.", town.m_sName));

				if (rain)
					so.Insert("It's raining, so only the ones who have to are walking.");

				if (late)
					so.Insert("Bastion's lot are deserting, and they walk our way.");

				if (so.IsEmpty())
				{
					so.Insert("The man who had it yesterday let his cousin through. And his cousin's cart.");
					so.Insert("The road's busy. That's money, or it's trouble.");
				}

				break;
			}

			case "spotter":
			{
				if (hungry)
					so.Insert("We're short, and the boss wants to know what's moving before he sends anyone.");

				if (late)
					so.Insert("Everyone's pulling in, so whoever's still on a road has something worth guarding.");

				if (so.IsEmpty())
				{
					so.Insert("We've been guessing, and guessing got two of ours shot.");
					so.Insert("There's something regular about it down there. Find out what.");
				}

				break;
			}

			case "tape":
			{
				if (rain)
					so.Insert("The damp's got into the housing and the voice has gone to mud.");

				if (late)
					so.Insert("It's the only thing left on that road telling anyone anything.");

				if (so.IsEmpty())
				{
					so.Insert("It's been round ten thousand times and it sounds it.");
					so.Insert("People have stopped hearing it, and a new voice makes them listen again.");
				}

				break;
			}

			case "gate":
			{
				if (sick && town)
					so.Insert(string.Format("Half of %1 is on the road, and they all have a piece of paper.", town.m_sName));

				if (late)
					so.Insert("Command's stamps have been turning up in hands Command never knew.");

				if (so.IsEmpty())
				{
					so.Insert("We turned a forger back yesterday. He'll have told his friends which gate.");
					so.Insert("The man who had it this morning passed everyone, to be liked.");
				}

				break;
			}

			case "kits":
			{
				if (late)
					so.Insert("We can't replace anything now, so I need to know where everything is.");

				if (hungry)
					so.Insert("The men are trading kit for food over the wire, and I want to know who.");

				if (so.IsEmpty())
				{
					so.Insert("Something's missing from that section, and nobody's reported it.");
					so.Insert("They've been in contact, and men get careless after contact.");
				}

				break;
			}

			case "telephone":
			{
				if (rain)
					so.Insert("It went in the storm, which may mean the storm did it. Or that somebody used the storm.");

				if (night || dusk)
					so.Insert("It went quiet at last light, and I don't like what goes quiet at last light.");

				if (so.IsEmpty())
				{
					so.Insert("It was working at stand-to, and dead an hour after.");
					so.Insert("That's twice this week, and the first time I called it weather.");
				}

				break;
			}

			case "school":
			{
				if (rain)
					so.Insert("It's raining, so they've been in all day and it shows.");

				if (died)
					so.Insert("One of them lost someone this week, and the others know.");

				if (so.IsEmpty())
				{
					so.Insert("Twelve of them, and one of me.");
					so.Insert("They've had the same lesson three days running and they've noticed.");
				}

				break;
			}

			case "garden":
			{
				if (hungry)
					so.Insert("The stores are low, so the beds matter more than they did.");

				if (rain)
					so.Insert("It's rained, so the weeds are up and the slugs are out.");

				if (late)
					so.Insert("There's no truck now. What's in those beds is the winter.");

				if (so.IsEmpty())
				{
					so.Insert("A garden left a day is a garden lost a week.");
					so.Insert("Nobody's been down there since the day before yesterday.");
				}

				break;
			}

			case "doorround":
			{
				if (sick && town)
					so.Insert(string.Format("People have been coming out of %1 for days, and not all of them came to us.", town.m_sName));

				if (rain)
					so.Insert("Nobody sleeps rough in this. They're behind a door somewhere.");

				if (so.IsEmpty())
				{
					so.Insert("There was a light where there shouldn't be, two nights ago.");
					so.Insert("Somebody's been taking from the gardens, carefully, like someone ashamed.");
				}

				break;
			}

			case "rollcall":
			{
				if (died)
					so.Insert("We've buried one this week already, and I won't be surprised twice.");

				if (late)
					so.Insert("People walk, these days. They don't always say.");

				if (so.IsEmpty())
				{
					so.Insert("It was nineteen at breakfast too, and nobody said.");
					so.Insert("Somebody knows, and has decided it isn't my business.");
				}

				break;
			}

			case "swap":
			{
				if (hungry)
					so.Insert("People are hungry, and hungry people sell what they shouldn't.");

				if (late)
					so.Insert("Money's paper now, so everything else has become money.");

				if (so.IsEmpty())
				{
					so.Insert("Half of them will be honest, and you won't know which half.");
					so.Insert("Last market someone sold us our own spoons.");
				}

				break;
			}

			case "watch":
			{
				if (rain)
					so.Insert("The rain hides everything out there, and makes everything sound like feet.");

				if (sick && town)
					so.Insert(string.Format("%1 has been loud all day, and loud travels.", town.m_sName));

				if (late)
					so.Insert("There's nobody further out than us now to see it first.");

				if (so.IsEmpty())
				{
					so.Insert("The light's going, and that's when things come to look.");
					so.Insert("The one who had it last night saw something and won't say what.");
				}

				break;
			}

			case "stock":
			{
				if (hungry)
					so.Insert("We're short, and I need to know if it's short or if it's stolen.");

				if (late)
					so.Insert("Nothing's coming in any more, so what's on the shelf is all there is.");

				if (so.IsEmpty())
				{
					so.Insert("The book and the shelf haven't agreed since the last truck.");
					so.Insert("Something's gone that shouldn't be, and nobody will say.");
				}

				break;
			}

			case "lockup":
			{
				if (rain)
					so.Insert("It's a foul night, and nobody else will sit out in it.");

				if (so.IsEmpty())
				{
					so.Insert("Everyone else on the rota knows them, and that's the trouble.");
					so.Insert("It has to be someone who can hear a hard thing and stay sat.");
				}

				break;
			}

			case "names":
			{
				if (died)
					so.Insert("Some of them are still laid out, and the families have only heard there was trouble.");

				if (so.IsEmpty())
				{
					so.Insert("It's been left two days already, which is two days too long.");
					so.Insert("The longer it waits, the more of it they hear from the wrong mouths.");
				}

				break;
			}

			case "dose":
			{
				if (sick)
					so.Insert("There are more bitten every day, and only the one dose.");

				if (so.IsEmpty())
				{
					so.Insert("It won't keep, and neither will they.");
					so.Insert("All three have been told there's a dose. None was told there's one.");
				}

				break;
			}

			case "animals":
			{
				if (night)
					so.Insert("They're louder at night. Some of them are louder than they should be.");

				if (so.IsEmpty())
				{
					so.Insert("Two of the cards were swapped last week, and nobody noticed for three days.");
					so.Insert("The keeper found one at the bars this morning, and it wasn't on the turning list.");
				}

				break;
			}

			case "list":
			{
				if (facts && facts.CurePercent() >= 70)
					so.Insert("We're close enough now that the place on the list is very likely a life.");

				if (so.IsEmpty())
				{
					so.Insert("The batch is due, and a dose without a name on it is wasted.");
					so.Insert("All three were told there was a list. None was told there was one place.");
				}

				break;
			}

			case "cull":
			{
				if (sick && town)
					so.Insert(string.Format("They're coming off %1 in ones and twos, and they don't go back.", town.m_sName));

				if (dusk || night)
					so.Insert("They're bolder with the light going.");

				if (so.IsEmpty())
				{
					so.Insert("Leave five for a week and it's fifty.");
					so.Insert("They were at the fence at first light, and they'll be back at last.");
				}

				break;
			}

			case "bench":
			{
				if (facts && facts.CurePercent() >= 50)
					so.Insert("The count is past halfway, and every tray matters now.");

				if (sick && town)
					so.Insert(string.Format("The field teams are sending them in warm from %1.", town.m_sName));

				if (night)
					so.Insert("The day shift left them stacked and went to bed.");

				if (so.IsEmpty())
				{
					so.Insert("Forty came in overnight and nobody has looked at one.");
					so.Insert("The backlog is older than some of the staff.");
				}

				break;
			}
		}

		if (so.IsEmpty())
			return "";

		return so[Math.RandomInt(0, so.Count())];
	}

	// ================================================================ Effects
	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iGame > 0)
			ResolveTable(dealer, e, w, playerId, said);

		ResolveScene(dealer, e, w, said);

		if (m_iRung >= RZ_Duties.RUNG_TRUST)
			dealer.SetCounter(DayKey(e), 1);

		if (w.m_bSquad && ResolveLost(dealer, e, said))
			return;

		if (w.m_sWay == "cull")
		{
			RZ_Settlement cleared = dealer.GetGM().GetSettlement(e.m_iSettlement);
			if (cleared)
				cleared.AddInfection(-2);
		}

		switch (w.m_iEffect)
		{
			case FX_FED:
			{
				Feed(dealer, e, 1);
				break;
			}

			case FX_FED_MEAT:
			{
				Feed(dealer, e, 2);
				break;
			}
		}
	}

	// ================================================================ Tables and squads
	//------------------------------------------------------------------------------------------------
	//! The sickest town within 2.5 km of the giver: the one arrivals, the dead and samples come from.
	protected static RZ_Settlement SickTown(RZ_GameModeComponent gm, vector from)
	{
		RZ_Settlement worst;
		foreach (RZ_Settlement s : gm.GetSettlements())
		{
			if (s.m_bIsland || RZ_Util.DistXZ(s.m_vCenter, from) > 2500)
				continue;

			if (!worst || s.m_fInfection > worst.m_fInfection)
				worst = s;
		}

		return worst;
	}

	//------------------------------------------------------------------------------------------------
	//! Who goes out with you, in the faction's words.
	protected string SquadName(RZ_ErrandWay w)
	{
		if (m_sDuty == "hunt")
			return "the hunter";

		switch (m_iFaction)
		{
			case RZ_EFaction.SURVIVORS: return "the two from the house";
			case RZ_EFaction.MILITARY: return "the patrol";
			case RZ_EFaction.BANDITS: return "the lads";
			case RZ_EFaction.INSTITUTE: return "the survey pair";
		}

		return "them";
	}

	//------------------------------------------------------------------------------------------------
	//! A way was taken: a sorting way's row is dealt from the world, a squad way's people set off.
	override void Taken(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_WorldFacts facts = gm.GetFacts();
		if (w.m_iGame > 0)
		{
			RZ_Settlement town = SickTown(gm, e.m_vGiver);
			string from = "";
			float infection = 30;
			if (town)
			{
				from = town.m_sName;
				infection = town.m_fInfection;
			}

			int stage = 0;
			if (facts)
				stage = facts.Stage();

			bool rain = gm.GetEnvironment() && gm.GetEnvironment().IsRaining();
			dealer.GetGames().Deal(e, w.m_iGame, e.Fill(m_sTitle), e.GetText("here"), from, infection, stage, rain);
			return;
		}

		if (w.m_sWay == "scene")
		{
			RZ_DutyScene played = dealer.GetScenes().Ensure(e, e.Fill(m_sTitle), e.m_vGiver, false);
			played.m_vLast = e.m_vPlace;
			switch (m_sDuty)
			{
				case "school": RZ_DutyHaven.School(e, played); break;
				case "garden": RZ_DutyHaven.Garden(dealer, e, played); break;
				case "doorround": RZ_DutyHaven.DoorRound(e, played); break;
				case "rollcall": RZ_DutyHaven.RollCall(e, played); break;
				case "swap": RZ_DutyHaven.SwapTable(e, played); break;
				case "samples": RZ_DutyHalcyon.Samples(gm, e, played); break;
				case "decon": RZ_DutyHalcyon.Decon(gm, e, played); break;
				case "incinerator": RZ_DutyHalcyon.Incinerator(e, played); break;
				case "ward": RZ_DutyHalcyon.Ward(e, played); break;
				case "coldchain": RZ_DutyHalcyon.ColdChain(e, played); break;
				case "wreck": RZ_DutyRedHand.Wreck(e, played); break;
				case "still": RZ_DutyRedHand.Still(e, played); break;
				case "count": RZ_DutyRedHand.Count(e, played); break;
				case "toll": RZ_DutyRedHand.Toll(dealer, gm, e, played); break;
				case "spotter": RZ_DutyRedHand.Spotter(dealer, e, played); break;
				case "tape": RZ_DutyBastion.Tape(e, played); break;
				case "gate": RZ_DutyBastion.Gate(e, played); break;
				case "kits": RZ_DutyBastion.Kits(e, played); break;
				case "telephone": RZ_DutyBastion.Telephone(e, played); break;
				case "watch": RZ_DutyPosts.Watch(gm, e, played); break;
				case "lockup": RZ_DutyPosts.LockUp(gm, e, played); break;
				case "list": RZ_DutyPosts.TrialList(gm, e, played); break;
				case "dose": RZ_DutyTrusted.Dose(e, played); break;
				case "animals": RZ_DutyTrusted.Animals(e, played); break;
				case "names":
				{
					if (m_iFaction == RZ_EFaction.MILITARY)
					{
						RZ_DutyPosts.Return(gm, e, played);
						break;
					}

					if (m_iFaction == RZ_EFaction.BANDITS)
					{
						RZ_DutyTrusted.DebtBook(e, played);
						break;
					}

					if (m_iFaction == RZ_EFaction.INSTITUTE)
					{
						RZ_DutyTrusted.Names(e, played);
						break;
					}

					int dead = 1;
					if (facts && e.m_iHouse >= 0)
						dead = Math.ClampInt(facts.House(e.m_iHouse).m_iDied, 1, 3);

					RZ_DutyPosts.Names(gm, e, played, dead);
					break;
				}
			}

			// Its steps, one objective each, for the tracker
			RZ_DutyGoals.Set(m_sDuty, m_iFaction, played);
			return;
		}

		if (!w.m_bSquad)
			return;

		array<vector> points = {};
		vector home = e.m_vPlace;
		if (w.m_sWay == "cull")
			RZ_DutySquads.Round(home, 70, 120, 1, points);
		else if (w.m_sWay == "leg")
			RZ_DutySquads.Round(home, 70, 140, 3, points);
		else if (w.m_sWay == "round")
			RZ_DutySquads.Round(home, 180, 300, 4, points);
		else
			home = e.m_vGiver;

		bool round = w.m_sWay == "leg" || w.m_sWay == "round" || w.m_sWay == "cull";
		if ((round && points.IsEmpty()) || !dealer.GetSquads().Start(e, e.Fill(m_sTitle), SquadName(w), home, points))
			dealer.Notify(e, "Nobody is free to go with you after all. Do it alone, and come back.");
		else if (w.m_sWay == "cull")
			dealer.GetSquads().SetCull(e.m_iId);

		// A hunt: something at the snare line, where its work is
		if (m_sDuty == "hunt")
		{
			RZ_DutyScene scene = dealer.GetScenes().Ensure(e, e.Fill(m_sTitle), e.m_vGiver, false);
			scene.m_vLast = e.m_vPlace;
			RZ_DutyFinds.Hunt(gm, e, scene.m_aBeats);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A sorting way's step is its verdicts; a round's is its people home with you beside them; a
	//! follower's is the work done with them at your side. People lost: back to say so.
	override bool StepCounts(RZ_Errands dealer, RZ_Errand e, IEntity me)
	{
		RZ_ErrandWay w = e.GetWay();
		if (!w)
			return true;

		if (w.m_iGame > 0)
			return !dealer.GetGames().Find(e.m_iId) || dealer.GetGames().IsJudged(e.m_iId);

		// A chore's small turn: once its work is done, four times in ten, something comes up there
		if (w.m_sWay.StartsWith("t") && !w.m_bSquad)
		{
			RZ_WorkSites sites = RZ_WorkSites.Get();
			if (!sites || !sites.IsDone(RZ_Errands.WorkKey(e)))
				return true;

			if (e.GetText("turn").IsEmpty())
			{
				e.SetText("turn", "1");
				array<ref RZ_DutyBeat> turn = {};
				if (Math.RandomFloat01() < 0.4)
					RZ_DutyFinds.Turn(dealer.GetGM(), e, m_sDuty, turn);

				if (!turn.IsEmpty())
				{
					RZ_DutyScene turned = dealer.GetScenes().Ensure(e, e.Fill(m_sTitle), e.m_vGiver, false);
					turned.m_vLast = e.m_vPlace;
					foreach (RZ_DutyBeat tb : turn)
					{
						turned.m_aBeats.Insert(tb);
					}
				}
			}

			return dealer.GetScenes().IsDone(e.m_iId);
		}

		if (!w.m_bSquad)
			return dealer.GetScenes().IsDone(e.m_iId);

		RZ_DutySquads squads = dealer.GetSquads();
		if (!squads.Find(e.m_iId) || squads.IsLost(e.m_iId))
			return true;

		if (w.m_sWay == "leg" || w.m_sWay == "round" || w.m_sWay == "cull")
			return squads.IsHome(e.m_iId, me);

		return squads.IsWith(e.m_iId, me) && dealer.GetScenes().IsDone(e.m_iId);
	}

	//------------------------------------------------------------------------------------------------
	//! A dangerous miss at a table comes out whether or not it's reported.
	override void Ended(RZ_Errands dealer, RZ_Errand e, string why)
	{
		RZ_DutyGame t = dealer.GetGames().Find(e.m_iId);
		if (t && t.m_bJudged && t.m_bDanger && !t.m_bPaidFor)
			Danger(dealer, e, t, e.m_iAsker);
	}

	//------------------------------------------------------------------------------------------------
	//! The dangerous miss: 2 Infamy with the giver's faction, and something small and real for the
	//! place (the dead and the line: the town 2 more infected; the bench: its faction's morale down 1).
	protected void Danger(RZ_Errands dealer, RZ_Errand e, RZ_DutyGame t, int playerId)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		t.m_bPaidFor = true;
		// Only someone who is here to answer for it (a held errand can end while its player is away)
		if (RZ_Factions.IsValid(e.m_iFaction) && gm.FindProfile(playerId))
			gm.AddInfamy(playerId, e.m_iFaction, 2, "a duty done carelessly: " + e.Fill(m_sTitle));

		if (t.m_iKind == RZ_DutyGame.KIND_BENCH || t.m_iKind == RZ_DutyGame.KIND_STOCK)
		{
			dealer.Morale(e.m_iFaction, -1, m_sKey);
			return;
		}

		RZ_Settlement town = gm.GetSettlement(e.m_iSettlement);
		if (town)
			town.AddInfection(2);
	}

	//------------------------------------------------------------------------------------------------
	//! A judged table decides the pay: the share got right, no Fame unless every one was, nothing
	//! after a dangerous miss. Lines for the giver go into `said`.
	protected void ResolveTable(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_DutyGame t = dealer.GetGames().Find(e.m_iId);
		if (!t || !t.m_bJudged)
			return;

		e.m_iFameCap = 0;
		if (t.m_bDanger)
		{
			e.m_iCap = 0;
			if (!t.m_bPaidFor)
				Danger(dealer, e, t, playerId);

			said.Insert("One of those was bad, and you passed it. It's come out already. That's on your name here, not mine.");
			return;
		}

		e.m_iCap = Math.Max(1, RZ_Util.RoundToInt(w.m_iFavor * t.m_fScore));
		if (t.m_fScore >= 0.99)
		{
			e.m_iFameCap = -1;
			array<int> who = {playerId};
			RZ_WorldFacts.Get().Memory().Record("duty." + m_sDuty, e.m_sGiver, e.m_sGiverName, RZ_WorldMemory.SOURCE_ERRAND, true, who);
			said.Insert("Every one of them right. I checked, and I didn't need to. I'll remember that.");
			return;
		}

		if (t.m_fScore >= 0.7)
			said.Insert("Mostly right, and nothing dangerous. Where you weren't sure you were careful, and careful I can live with.");
		else
			said.Insert("You got through it, and nothing bad got past you. But that was a lot of care and not much certainty.");
	}

	//------------------------------------------------------------------------------------------------
	//! What was chosen on the way decides part of the pay: the choices' Favor (3 more at most), and
	//! for a scored scene the share got right (no Fame unless every call was). What the giver has
	//! heard of it goes into `said`.
	protected void ResolveScene(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, notnull array<string> said)
	{
		RZ_DutyScene scene = dealer.GetScenes().Find(e.m_iId);
		if (!scene)
			return;

		e.m_iBonus = scene.m_iFavor;
		foreach (string told : scene.m_aTold)
		{
			said.Insert(told);
		}

		if (scene.m_iScored <= 0)
			return;

		float score = scene.Score();
		if (score < 0.99)
		{
			e.m_iFameCap = 0;
			e.m_iCap = Math.Max(1, RZ_Util.RoundToInt((w.m_iFavor + Math.Min(scene.m_iFavor, 3)) * score));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Their people didn't come back: no pay for it.
	protected bool ResolveLost(RZ_Errands dealer, RZ_Errand e, notnull array<string> said)
	{
		if (!dealer.GetSquads().IsLost(e.m_iId))
			return false;

		e.m_iCap = 0;
		e.m_iFameCap = 0;
		dealer.Morale(e.m_iFaction, -1, m_sKey);
		said.Insert("You came back and they didn't. I'm not paying for that, and I'm not blaming you for it. Not out loud.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A house fed from its kitchen: `amount` food, on two duties a season day at most.
	protected void Feed(RZ_Errands dealer, RZ_Errand e, int amount)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (e.m_iHouse < 0 || !gm.GetSafehouses())
			return;

		string key = string.Format("duty.fed.%1.%2", e.m_iHouse, RZ_SeasonClock.Now() / 86400);
		int fed = dealer.Counter(key);
		if (fed >= 2)
			return;

		dealer.SetCounter(key, fed + 1);
		gm.GetSafehouses().AddFood(e.m_iHouse, amount);
	}
}
