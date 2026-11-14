//! The Haven's route, The One Who Didn't Turn (docs/FINALE.md, "The Haven"), key "route_haven".
//! He was on the Day 2 bus, bitten in the church doorway on Day 4, kept back from the Day 6 hill,
//! and crossed off the List with the pencil the houses use for the dead. Nothing is drawn without
//! his yes. Nobody, the writing included, ever says why he didn't turn.
//!   I   The Crossed-Out Name (open): his name, his house, tonight's spot
//!   II  And my boy (sworn): his yes, and his boy's line in the book (the Red Hand's door) or he
//!       gives without it, one step harder; then Three Times: two bags, drawn at dusk, stored cold
//!   III Who It Was (open): live tissue (the controller's hunts)
//!   IV  Up the Hill (sworn, last): at dawn, the last draw fresh in the lab; "Two is enough" spares
//!       him the dawn for 2 real days of the lab's work
//! Branch: What He Left (his notebook for the door) and Twice (up to six bags) when he's gone.
//! Simplified: tonight's spot is his house (the rota moves him every 3 real days); the walk up is
//! a choice, not an escort block; R1 (his Day 2 tube) is one bag.
class RZ_Route_Haven : RZ_FinaleRoute
{
	protected static const float BAG_COLD = 900;	// a cool bag: 15 minutes out of the cold
	protected static const int TWICE_DRAWS = 6;		// Twice gives six at most (the branch)

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		Route(RZ_CureFinale.HAVEN);
	}

	//------------------------------------------------------------------------------------------------
	override protected string StageTitle(int stage)
	{
		switch (stage)
		{
			case S_ACT1: return "The Crossed-Out Name";
			case S_DOOR: return "And my boy";
			case S_PIECE: return "Three Times";
			case S_ACT4: return "Up the Hill";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override protected string StageKnown(int stage)
	{
		switch (stage)
		{
			case S_ACT1: return "\"There's someone. We kept him from you. We kept him from everyone.\" A man the houses crossed off the List to keep him alive. Find his name, and where he sleeps tonight.";
			case S_DOOR: return "\"Day 2 I was on the bus. Day 4 I was in the church door. Day 6 I was meant to walk up that hill.\" Ask {woman} for his yes. His terms end with his boy's line in the Red Hand's book.";
			case S_PIECE: return "Two bags of {woman}'s blood, drawn at dusk at the edge of his town and carried cold to the lab.";
			case S_ACT4: return "At dawn, the hour the ten walked up on Day 6: the last draw, fresh, in the lab.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override protected string StageObjective(int stage)
	{
		switch (stage)
		{
			case S_ACT1: return "Find the man the houses crossed off the List, starting with {chair}.";
			case S_DOOR: return "Ask {woman} for a yes, and meet {woman}'s terms.";
			case S_PIECE: return "Carry two bags of {woman}'s blood, drawn at dusk, cold to the lab in {lab_town}.";
			case S_ACT4: return "Bring {woman} to the lab in {lab_town} at dawn for the last draw.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override protected string RemakeText()
	{
		return "The batch was lost. {woman} can give once more, if he agrees.";
	}

	//------------------------------------------------------------------------------------------------
	override protected string SubjectKey()
	{
		return "woman";
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Places(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_FinalePeople people = Finale().GetPeople();
		if (!people.Get("woman"))
			return false;

		run.SetText("woman", people.Name("woman"));
		run.Claim(RZ_FinalePeople.Subject("woman"));
		run.SetText("son", people.Name("son"));
		RZ_Leader chair = RZ_MajorHelp.Top(gm, RZ_EFaction.SURVIVORS);
		if (!chair)
			return false;

		run.SetText("chair", chair.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (gm.GetFactionLife())
			gm.GetFactionLife().OpenChannelLine(RZ_EFaction.SURVIVORS, "All houses: nobody says his name on the radio. He moves tonight.");
	}

	//------------------------------------------------------------------------------------------------
	//! Draws at dusk or after dark; the walk up at dawn (05:00-09:00) unless he was spared it.
	override protected bool Window(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (stage == S_PIECE)
			return Between(17, 5);

		if (stage == S_ACT4 && run.Flag("spared") == 0)
			return Between(5, 9);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected string ItemName(string kind)
	{
		if (kind == "bag")
			return "a bag of his blood";

		return "the last draw";
	}

	//------------------------------------------------------------------------------------------------
	//! His yes: persuade 3 on Haven rank (Nurse, Doctor or Elder +1); one easier with his name read
	//! at supper (Every Name), one harder if he was sent to the far house as a mouth.
	protected int YesStep(RZ_GameModeComponent gm)
	{
		int d = 3;
		if (Major(gm, "list.read") != 0)
			d--;

		if (Major(gm, "haven.rule") == 2)
			d++;

		return Math.ClampInt(d, 1, 4);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool RouteScene(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene)
	{
		RZ_FinalePeople people = Finale().GetPeople();
		vector house = PersonAt("woman", Seat(gm, RZ_EFaction.SURVIVORS));
		bool here = people.IsAlive("woman");
		scene.m_sTitle = StageTitle(stage);
		scene.m_sBrief = run.Fill(StageKnown(stage));
		scene.m_sObjective = run.Fill(StageObjective(stage));
		if (!here && stage >= S_DOOR)
		{
			scene.m_sBrief = run.Fill("{woman} is gone. What he left is a notebook, and a man the crews call Twice has the same luck.");
			scene.m_sObjective = run.Fill("Find the man the crews call Twice, who has {woman}'s luck.");
		}

		switch (stage)
		{
			case S_ACT1:
			{
				scene.m_vPos = Seat(gm, RZ_EFaction.SURVIVORS);
				int chair = scene.Person(scene.m_vPos, run.Fill("{chair}"), "Talk to the chair", "There's someone. We kept him from you. We kept him from everyone.");
				scene.Choice(chair, "Ask who he is [Haven: Known]").Needs(RZ_EFaction.SURVIVORS, 1).Decides(OUT_ACT1).Says(run.Fill("\"His name is {woman}. Don't say it on the radio.\" The chair tells you which house, and that it changes."), "heard his name from the chair");
				scene.Choice(chair, "Read his name off the List").Only("world:list.read").Decides(OUT_ACT1).Says(run.Fill("His name was read with the rest at supper: {woman}. Uncrossed now."), "found his name on the List");
				int head = scene.Person(HeadSeat(gm, house), "a council head near his house", "Talk to the council head", "Which house? They all look the same at dusk. That's the point.");
				scene.Choice(head, "Ask the council head [Haven: Known]").Needs(RZ_EFaction.SURVIVORS, 1).Sets("do:head").Decides(OUT_ACT1).Says("Tonight it's the house with the second bowl. Tomorrow it isn't.", "had his house from a council head");
				scene.Choice(head, "Ask the Preacher who rings for him").Persuade(RZ_EFaction.SURVIVORS, 2, "elder").Decides(OUT_ACT1).Says("\"I ring for the ones the List forgot. One of them still answers.\" A house, and a name.", "heard it from the Preacher").Refused("The bells are for the dead. Ask the living.");
				int lookout = scene.Person(house, "a house's back door at dusk", "Look at the back door", "Two bowls go out by the back door at dusk. The house has one cat. It hasn't got a cat.");
				scene.Choice(lookout, "Watch for the second bowl [unseen]").Sneak().Decides(OUT_ACT1).Says("The second bowl goes in through the side window. Someone eats on half portions in there.", "watched a house at dusk");
				scene.Choice(lookout, "Feed the hungry house [3 food]").Sets("do:food3").Decides(OUT_ACT1).Says("The council head takes the food and answers one question. It's the right one.", "fed his house");
				scene.Choice(lookout, "Tail the walker at dusk [unseen]").Only("world:walk.walks_stay").Sneak().Decides(OUT_ACT1).Says("The walker goes the long way round and knocks twice at one door.", "tailed the walker");
				scene.Choice(lookout, "Knock as a house runner [Haven kit]").Trick(RZ_EFaction.SURVIVORS, 2).Decides(OUT_ACT1).Says("\"You're late with the bread.\" The door opens wider than it should.", "passed as a house runner").Refused("You're not from any house I know.");
				int clerk = scene.Person(RZ_MajorHelp.NearSeat(gm.GetLabPosition()), "the lab's clerk", "Talk to the clerk", "The Day 6 ledger: eleven referred, ten treated, eight discharged. One did not present.");
				scene.Choice(clerk, "Read the Day 6 ledger [Halcyon: Known]").Needs(RZ_EFaction.INSTITUTE, 1).Decides(OUT_ACT1).Says("Line eleven: did not present. A house's name in the margin.", "read the Day 6 ledger");
				scene.Choice(clerk, "Pay a courier for a look [20 Favor]").Bribe(RZ_EFaction.INSTITUTE, 20).Decides(OUT_ACT1).Says("A courier copies line eleven for you, and the margin.", "bought a look at the ledger");
				int list = scene.Person(Seat(gm, RZ_EFaction.MILITARY), "Bastion's clerk with the Day 2 list", "Talk to the clerk", "Day 2 processing. Every name off the bus, and where it went.");
				scene.Choice(list, "Read the processing list [Bastion: Trusted]").Needs(RZ_EFaction.MILITARY, 2).Decides(OUT_ACT1).Says("One name off the bus with no line after it. A Haven house's name in pencil.", "read Bastion's Day 2 list");
				scene.Choice(list, "Buy a look at the list [15 Favor]").Bribe(RZ_EFaction.MILITARY, 15).Decides(OUT_ACT1).Says("\"Ten minutes. Don't copy it.\" You copy it.", "bought a look at the Day 2 list");
				int strays = scene.Person(Seat(gm, RZ_EFaction.BANDITS), "the crew that runs the strays", "Talk to the crew", "We know every house that feeds a stray. And every house that feeds a person.");
				scene.Choice(strays, "Pay the crew for the house [20 Favor]").Bribe(RZ_EFaction.BANDITS, 20).Decides(OUT_ACT1).Says("\"The one with the cat it hasn't got.\" They laugh. It's right.", "paid the strays crew");
				scene.Choice(strays, "Lean on the crew").Threaten(RZ_EFaction.BANDITS, 2).Decides(OUT_ACT1).Says("\"Fine. The house with two bowls.\" They'll remember your face.", "leaned on the strays crew").Refused("Try that again with more friends.");
				return true;
			}
			case S_DOOR:
			{
				scene.m_vPos = house;
				if (!here)
				{
					int notebook = scene.Person(Seat(gm, RZ_EFaction.SURVIVORS), run.Fill("{chair}"), "Talk to the chair", run.Fill("He kept a notebook. Every bite, every fever, every day he didn't turn."));
					scene.Choice(notebook, "Take what he left to the lab").Decides(OUT_OTHER).Says("The lab reads his notebook, slowly. It's not blood, but it's two years of him.", "took his notebook to the lab");
					return true;
				}

				int her = scene.Person(house, run.Fill("{woman}"), "Talk to him", "\"They crossed me off to keep me safe. Nobody asked me either.\" His terms: not up that hill, nobody first because of guns, his name said out loud, and his boy.");
				RZ_QM_RouteScene.Cast(scene).TrackSubject(her, "woman");
				scene.Choice(her, "Ask for his yes").Persuade(RZ_EFaction.SURVIVORS, YesStep(gm), "nurse,doctor,elder").Sets("do:yes").Says("\"Then say it. Out loud.\" You do. He nods once.", "had his yes").Refused("Ask me again when you mean it.");
				scene.Choice(her, "Ask him to give without it").Only("yes").Persuade(RZ_EFaction.SURVIVORS, Math.MinInt(4, YesStep(gm) + 1), "nurse,doctor,elder").Decides(OUT_OTHER).Says("\"He's a grown man. He chose his crew.\" He rolls up his sleeve.", "had him give without his boy's line").Refused("And my boy.");
				FreeChoice(gm, run, scene, her, "woman");
				int boss = scene.Person(PersonAt("son", Seat(gm, RZ_EFaction.BANDITS)), "the crew boss who holds his boy's line", "Talk to the crew boss", run.Fill("{son}? Owes. Everyone owes. A debt is a debt."));
				int price = 30;
				if (Major(gm, "haven.rule") == 4)
					price = 15;

				scene.Choice(boss, string.Format("Pay his line [%1 Red Hand Favor]", price)).Only("yes").Needs(RZ_EFaction.BANDITS, 2).Bribe(RZ_EFaction.BANDITS, price).Decides(OUT_DOOR).Says("\"A debt is a debt. It was paid, by him.\" The boss marks the line paid.", "paid his boy's line");
				scene.Choice(boss, "Walk him out at night [unseen]").Only("yes").Sneak().Decides(OUT_DOOR).Says("He comes because his father asked. His line moves to his house: leaving doesn't pay a debt.", "walked his boy out of the camp");
				scene.Choice(boss, "Lean on the crew boss").Only("yes").Threaten(RZ_EFaction.BANDITS, 3, 5).Decides(OUT_DOOR).Says("\"Take him, then. The book keeps the line.\" The boss won't forget it.", "leaned on his boy's crew boss").Refused("In my own camp? Brave.");
				if (Major(gm, "book.last") == 4 || Finale().PactOf(m_iRoute) == RZ_CureFinale.REDHAND)
					scene.Choice(boss, "His line's already settled").Only("yes").Decides(OUT_DOOR).Says("\"Every debt went with the book.\" Or with the pact. Either way his line is clean.", "found his boy's line already clear");

				int desk = scene.Person(RZ_Mission.LandPointNear(Seat(gm, RZ_EFaction.BANDITS), 10, 25), "the fence's desk", "Look at the desk", "The fence keeps a copy of every crew's page. Nobody checks the copies.");
				scene.Choice(desk, "Mark his line paid [Red Hand kit]").Only("yes").Trick(RZ_EFaction.BANDITS, 2).Decides(OUT_DOOR).Says("Paid, in the fence's own hand, near enough. Nobody checks the copies.", "marked his boy's line paid at the fence's desk").Refused("That's not your page.");
				RivalSpot(gm, run, scene, house, "woman", "her", "");
				return true;
			}
			case S_PIECE:
			{
				scene.m_vPos = RZ_Mission.LandPointNear(house, 60, 120);
				int duty = DutyOfficer(gm, scene, "Cold, please. It goes straight in the store.");
				scene.Choice(duty, "Store a bag of blood").Sets(string.Format("do:store:bag:2:%1", OUT_PIECE)).Says("Signed into the cold store in the Haven's name.", "stored a bag at the lab");
				if (!here)
				{
					scene.m_vPos = PersonAt("twice", Seat(gm, RZ_EFaction.BANDITS));
					int twice = scene.Person(scene.m_vPos, "a man the crews call Twice", "Talk to Twice", "\"Killed in action once. Bitten once. Neither took.\" He's heard what the lodger was.");
					RZ_QM_RouteScene.Cast(scene).TrackSubject(twice, "twice");
					scene.Choice(twice, "Ask him for a bag").Persuade(RZ_EFaction.SURVIVORS, TwiceStep(gm), "nurse,doctor").Sets("do:twice_yes").Says("Six. Not seven.", "had Twice's yes").Refused("Ask me when you've eaten with me.");
					scene.Choice(twice, "Ask him as one of the crews").Persuade(RZ_EFaction.BANDITS, TwiceStep(gm), "excon,raider").Sets("do:twice_yes").Says("For a crew? Six. Not seven.", "had Twice's yes as a crew").Refused("You're not crew enough.");
					scene.Choice(twice, "Share bread and ask [1 food]").Trade(RZ_EErrandItem.FOOD, 1).Persuade(RZ_EFaction.SURVIVORS, Math.MaxInt(1, TwiceStep(gm) - 1), "nurse,doctor").Sets("do:twice_yes").Says("He eats first. Then he rolls up a sleeve.", "shared bread with Twice").Refused("Bread's bread. A yes is a yes.");
					scene.Choice(twice, "Draw a bag from Twice").Sets("do:twice_draw").Says("He watches the bag fill. \"Write Twice on it. They'll know.\"", "drew a bag from Twice");
					return true;
				}

				int draw = scene.Person(scene.m_vPos, "the edge of his town at dusk", "Look at the draw point", "Where the houses used to walk the bitten out at dusk. A table, a lamp, a cool bag.");
				RZ_QM_RouteScene.Cast(scene).TrackSubject(draw, "woman");
				scene.Choice(draw, "Draw a bag [his yes]").Sets("do:draw").Says("He looks away while it fills. \"Twice more. Then I'm done.\"", "drew a bag of his blood");
				scene.Choice(draw, "Keep a bag back for yourself").Way(RZ_EQuestApproach.BETRAY).Sets("do:divert").Says("A bag goes in your coat instead of the cool box. He saw. He doesn't say anything.", "kept a bag of his blood back");
				scene.Choice(duty, "Two is enough [Haven: Trusted]").Needs(RZ_EFaction.SURVIVORS, 2).Sets("spared").Says("Then he doesn't walk up at dawn. The lab will make do, slower.", "spared him the dawn");
				int tube = scene.Person(Seat(gm, RZ_EFaction.MILITARY), "Bastion's medical store", "Look at the store", "Day 2 tubes, one a passenger, in a rack marked R. One of them is his.");
				scene.Choice(tube, "Trade for his Day 2 tube [40 Favor]").Bribe(RZ_EFaction.MILITARY, 40).Sets("do:r1").Says("\"R1. Signed out to the bearer.\" Cold, in a bag, as good as a draw.", "traded for his Day 2 tube");
				scene.Choice(tube, "Take his Day 2 tube [unseen]").Sneak().Sets("do:r1").Says("R1 goes in your coat. The rack has a gap now.", "took his Day 2 tube");
				RivalSpot(gm, run, scene, scene.m_vPos, "woman", "her", "");
				return true;
			}
			case S_ACT4:
			{
				scene.m_vPos = gm.GetLabPosition();
				int gate = DutyOfficer(gm, scene, "Dawn. The hour the ten walked up on Day 6.");
				if (run.Flag("spared") != 0 || !here)
				{
					run.SetFlag("spare_until", Math.MaxInt(run.Flag("spare_until"), RZ_SeasonClock.Now() + RZ_Util.RoundToInt(172800 * gm.GetQuestlines().WaitScale())));
					scene.Choice(gate, "Ask how the work's going").Sets("do:wait:spare_until").Decides(OUT_LAST).Says("What he gave was enough. Slower, and kinder.", "waited out the lab's work");
					return true;
				}

				int walker = scene.Person(PersonAt("woman", house), run.Fill("{woman}"), "Talk to him", "I'll walk. Not carried.");
				RZ_QM_RouteScene.Cast(scene).TrackSubject(walker, "woman");
				scene.Choice(walker, "Walk up the hill with him [his yes]").Sets("walked").Says("Houses on his road light candles. He keeps his own pace.", "walked him up the hill");
				scene.Choice(gate, "Bring him in at the gate").Only("walked").Sets("do:last").Decides(OUT_LAST).Says("The last draw, fresh, in the clean room.", "brought him up the hill");
				scene.Choice(walker, "Draw at first light and run it up").Sets("do:give:last:720").Says("Twelve minutes in a cool bag. Run.", "ran the last draw up");
				scene.Choice(gate, "Hand in the last draw").Sets(string.Format("do:store:last:1:%1", OUT_LAST)).Says("The duty officer takes it straight through.", "handed in the last draw");
				int road = scene.Person(RZ_Mission.LandPointNear(house, 40, 80), "the Preacher at the bells", "Talk to the Preacher", "I'll ring him up the hill. They'll hear it and they'll come. Better them than him.");
				scene.Choice(road, "Ring him up the hill").Sets("walked").Says("The bells go, and the road clears ahead of him for a while.", "had the Preacher ring him up");
				scene.Choice(road, "Take the courier road [unseen]").Sneak().Sets("walked").Says("Back lanes, no lights. Nobody sees him go.", "took him up the courier road");
				scene.Choice(road, "Pay the roadblocks [30 Red Hand Favor]").Bribe(RZ_EFaction.BANDITS, 30).Sets("walked").Says("The poles go up and stay up. Somebody writes it in a book.", "paid his way through the roadblocks");
				scene.Choice(road, "Pay Halcyon's gate [20 Favor]").Bribe(RZ_EFaction.INSTITUTE, 20).Sets("walked").Says("The gate opens before he reaches it. Nobody asks his name.", "paid his way in at the gate");
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A council head near his house, read by their leader: Loyal heads send you to the chair, a
	//! Paranoid head gives a decoy below Trusted, an Independent head wants a job (3 food) first;
	//! a Merciful head tells anyone Known.
	protected vector HeadSeat(RZ_GameModeComponent gm, vector house)
	{
		RZ_Leader l = CouncilHead(gm, house);
		if (l)
			return RZ_MajorHelp.NearSeat(l.m_vPos);

		return RZ_Mission.LandPointNear(house, 15, 30);
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Leader CouncilHead(RZ_GameModeComponent gm, vector house)
	{
		RZ_Leaders leaders = gm.GetLeaders();
		RZ_Leader top = RZ_MajorHelp.Top(gm, RZ_EFaction.SURVIVORS);
		if (!leaders)
			return null;

		int topId = -1;
		if (top)
			topId = top.m_iId;

		return leaders.FindNearest(RZ_EFaction.SURVIVORS, house, 5000, false, topId);
	}

	//------------------------------------------------------------------------------------------------
	//! Twice's yes (the branch): 2 on rank; one harder passing in Bastion's uniform.
	protected int TwiceStep(RZ_GameModeComponent gm)
	{
		return 2;
	}

	//------------------------------------------------------------------------------------------------
	override string CanDo(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, string code)
	{
		RZ_FinalePeople people = Finale().GetPeople();
		if (code == "yes")
		{
			RZ_FinalePersonSave p = people.Get("woman");
			int to = people.YesTo("woman");
			if (!people.IsAlive("woman"))
				return "He's gone.";

			if (p && p.holder >= 0 && p.holder != m_iRoute)
				return "Not while they're holding me.";

			if (to >= 0 && to != m_iRoute)
				return "I've said yes to someone already. One at a time.";

			return "";
		}

		if (code == "head")
		{
			RZ_Leader head = CouncilHead(gm, PlayerPos(playerId));
			if (!head)
				return "";

			int rank = RZ_QM_HalcyonScene.Rank(gm, playerId, RZ_EFaction.SURVIVORS);
			if (head.m_eLoyalty == RZ_ELoyalty.LOYAL && !head.HasTrait(RZ_ELeaderTrait.MERCIFUL))
				return run.Fill("Ask the chair. {chair} decides who knows.");

			if (head.HasTrait(RZ_ELeaderTrait.PARANOID) && rank < 2)
				return "\"The far house. He's at the far house.\" He isn't. You can tell by how fast it came.";

			if (head.m_eLoyalty == RZ_ELoyalty.INDEPENDENT)
			{
				IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
				if (!me || !RZ_HavenQuest.TakeFood(gm, me, 3))
					return "Do something for this house first. Three for the pot.";
			}

			return "";
		}

		if (code == "twice_yes")
		{
			int twiceTo = people.YesTo("twice");
			if (!people.IsAlive("twice"))
				return "Nobody here answers to that.";

			if (twiceTo >= 0 && twiceTo != m_iRoute)
				return "I've said yes to someone already.";

			return "";
		}

		if (code == "twice_draw")
		{
			if (people.YesTo("twice") != m_iRoute)
				return "Ask first.";

			if (people.Draws("twice") >= TWICE_DRAWS)
				return "Six. I said six.";

			return "";
		}

		if (code == "r1")
		{
			if (RZ_WorldFacts.Get().Flag("finale.r1") != 0)
				return "The rack has a gap where R1 was.";

			return "";
		}

		if (code == "draw" || code == "last" || code == "divert" || code.StartsWith("give:last"))
		{
			if (run.Flag("yes") == 0 && people.YesTo("woman") != m_iRoute)
				return "Not without my yes.";

			if (people.Draws("woman") >= RZ_FinalePeople.MAX_DRAWS)
				return "Five. I said five. Never a sixth.";

			int next = run.Flag("draw_unix") + RZ_Util.RoundToInt(43200 * gm.GetQuestlines().WaitScale());
			if ((code == "draw" || code == "divert") && RZ_SeasonClock.Now() < next && run.Flag("draws") > 0)
				return "Twelve hours between. I'm not a tap.";

			return "";
		}

		return super.CanDo(gm, run, playerId, code);
	}

	//------------------------------------------------------------------------------------------------
	override int DoIt(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, string code)
	{
		RZ_FinalePeople people = Finale().GetPeople();
		if (code == "yes")
		{
			people.Yes("woman", m_iRoute);
			run.SetFlag("yes", 1);
			return -1;
		}

		if (code == "draw" || code == "divert")
		{
			people.Draw("woman", m_iRoute, RZ_FinalePeople.MAX_DRAWS);
			run.SetFlag("draws", run.Flag("draws") + 1);
			run.SetFlag("draw_unix", RZ_SeasonClock.Now());
			if (code == "draw")
			{
				GiveItem(gm, playerId, "bag", ItemName("bag"), PlayerPos(playerId), ColdFor(gm, playerId, BAG_COLD));
				return -1;
			}

			// Betraying a bag: it's nobody's now (the Independent's one unit), +15 Haven Infamy
			Finale().GetItems().Give("bag", ItemName("bag"), RZ_CureFinale.INDEP, IdentOf(gm, playerId), PlayerPos(playerId), ColdFor(gm, playerId, BAG_COLD));
			people.Betray("woman");
			Finale().PayIdent(IdentOf(gm, playerId), RZ_EFaction.SURVIVORS, 0, 0, 15, "kept a bag of his blood back");
			return -1;
		}

		if (code == "twice_yes")
		{
			people.Yes("twice", m_iRoute);
			return -1;
		}

		if (code == "twice_draw")
		{
			people.Draw("twice", m_iRoute, TWICE_DRAWS);
			GiveItem(gm, playerId, "bag", "a bag of Twice's blood", PlayerPos(playerId), ColdFor(gm, playerId, BAG_COLD));
			return -1;
		}

		if (code == "r1")
		{
			RZ_WorldFacts.Get().SetFlag("finale.r1", 1);
			GiveItem(gm, playerId, "bag", "his Day 2 tube (R1)", PlayerPos(playerId), ColdFor(gm, playerId, BAG_COLD));
			return -1;
		}

		if (code == "last")
		{
			people.Draw("woman", m_iRoute, RZ_FinalePeople.MAX_DRAWS);
			return -1;
		}

		if (code.StartsWith("give:last"))
			people.Draw("woman", m_iRoute, RZ_FinalePeople.MAX_DRAWS);

		return super.DoIt(gm, run, playerId, code);
	}

	//------------------------------------------------------------------------------------------------
	override protected void AfterStep(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		if (stage == S_ACT1)
			run.Note(run.Fill("His name is {woman}. The houses move him at dusk."));
		else if (stage == S_DOOR && outcome == OUT_DOOR)
			run.Note(run.Fill("{son}'s line is marked paid."));
	}

	//------------------------------------------------------------------------------------------------
	override protected bool ReleaseScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene)
	{
		scene.m_sTitle = "The release";
		scene.m_sBrief = "The duty officer brings the release form to the clean room. The Haven's people choose the order.";
		scene.m_sObjective = run.Fill(ReleaseObjective());
		scene.m_vPos = gm.GetLabPosition();
		int form = DutyOfficer(gm, scene, "Who goes first? Write it in. I'll read it out.");
		scene.Choice(form, "His house, and his boy").Decides(OUT_TERMS + 1).Says("The first box goes to his house. His boy carries it.", "chose his house first");
		if (Major(gm, "haven.rule") == 2)
			scene.Choice(form, "The far house first").Decides(OUT_TERMS + 2).Says("Somebody finally drives out to the far house.", "chose the far house first");

		scene.Choice(form, "By the bite book").Decides(OUT_TERMS + 3).Says("The sick first, whoever they are. The houses read the bite book, not the List.", "chose the bite book");
		scene.Choice(form, "The crossed-out first").Only("world:list.read").Decides(OUT_TERMS + 4).Says("They go down the List from the bottom.", "chose the crossed-out first");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Nobody chose: a Merciful or Idealist chair reads the bite book; else his house first.
	override int DefaultTerms(RZ_GameModeComponent gm)
	{
		if (RZ_MajorHelp.TopHas(gm, RZ_EFaction.SURVIVORS, RZ_ELeaderTrait.MERCIFUL) || RZ_MajorHelp.TopHas(gm, RZ_EFaction.SURVIVORS, RZ_ELeaderTrait.IDEALIST))
			return 3;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected string ControlledLine(int terms)
	{
		return "If you can hear this: his name is {woman}. He was bitten on Day 4, and he's still here. Bring water if you can.";
	}

	override protected string LostLine() { return "He gave three times. It went out on somebody else's terms. The Haven read his name anyway."; }
	override protected string NeverLine() { return "He was never needed. The Haven uncrossed his name anyway."; }
}
