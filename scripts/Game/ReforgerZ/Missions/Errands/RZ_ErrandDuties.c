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
		}
	}

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
		Task("Peel and chop for the pot", RZ_EWork.BENCH, "Peel and chop", 18, false, "Peel and chop at the kitchen table at {here}, then tell {giver}.", 3);
		Task("Mind the pot", RZ_EWork.HANDS, "Stir the pot", 20, false, "Stir the pot on the stove at {here}, then tell {giver}.", 3);
		Task("Serve the line", RZ_EWork.HANDS, "Ladle supper out", 16, false, "Ladle supper out to the line at {here}, then tell {giver}.", 3);
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
				m_sTitle = "The Pot and the Still";
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
		Task("Split the wood", RZ_EWork.HANDS, "Split the wood", 18, true, "Split the wood stacked behind {here}, then tell {giver}.", 3);
		Task("Fill the water cans", RZ_EWork.FILL, "Fill the cans", 15, true, "Fill the water cans at the butt behind {here}, then tell {giver}.", 3);
		Task("Carry the crates in", RZ_EWork.HANDS, "Carry and stack the crates", 16, false, "Carry the crates in and stack them at {here}, then tell {giver}.", 3);
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
		Task("Wash the bedding", RZ_EWork.HANDS, "Wash and hang the bedding", 16, true, "Wash the bedding and hang it on the line behind {here}, then tell {giver}.", 3);
		Task("Sluice the yard", RZ_EWork.HANDS, "Sluice the yard", 18, true, "Sluice the yard and the step at {here} with lime water, then tell {giver}.", 3);
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
		snares.m_sStep = "Go out with {squad} to the snare line {place}, check the snares there with the hunter beside you, and bring what's caught back to {giver}.";
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

		if (w.m_bSquad && ResolveLost(dealer, e, said))
			return;

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

		if (!w.m_bSquad)
			return;

		array<vector> points = {};
		vector home = e.m_vPlace;
		if (w.m_sWay == "leg")
			RZ_DutySquads.Round(home, 70, 140, 3, points);
		else if (w.m_sWay == "round")
			RZ_DutySquads.Round(home, 180, 300, 4, points);
		else
			home = e.m_vGiver;

		bool round = w.m_sWay == "leg" || w.m_sWay == "round";
		if ((round && points.IsEmpty()) || !dealer.GetSquads().Start(e, e.Fill(m_sTitle), SquadName(w), home, points))
			dealer.Notify(e, "Nobody is free to go with you after all. Do it alone, and come back.");

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

		if (w.m_sWay == "leg" || w.m_sWay == "round")
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

		if (t.m_iKind == RZ_DutyGame.KIND_BENCH)
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
