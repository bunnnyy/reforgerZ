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
		array<string> kinds = {"kitchen", "wood", "wash"};
		foreach (string kind : kinds)
		{
			for (int f = 0; f < 4; f++)
			{
				list.Insert(new RZ_ErrandDuty(kind, f));
			}
		}
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
		if (w.m_sWay.StartsWith("t"))
			return w.m_sWay == "t" + e.m_iValue.ToString();

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
		}

		if (so.IsEmpty())
			return "";

		return so[Math.RandomInt(0, so.Count())];
	}

	// ================================================================ Effects
	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		switch (w.m_iEffect)
		{
			case FX_FED:
			{
				Feed(dealer, e, 1);
				break;
			}
		}
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
