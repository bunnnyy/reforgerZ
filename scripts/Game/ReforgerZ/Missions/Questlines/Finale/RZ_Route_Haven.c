//! The Haven's route, The One Who Didn't Turn (docs/FINALE.md, "The Haven"), key "route_haven".
//! She was on the Day 2 bus, bitten in the church doorway on Day 4, kept back from the Day 6 hill,
//! and crossed off the List with the pencil the houses use for the dead. Nothing is drawn without
//! her yes. Nobody, the writing included, ever says why she didn't turn.
//!   I   The Crossed-Out Name (open): her name, her house, tonight's spot
//!   II  And my boy (sworn): her yes, and her boy's line in the book (the Red Hand's door) or she
//!       gives without it, one step harder; then Three Times: two bags, drawn at dusk, stored cold
//!   III Who It Was (open): live tissue (the controller's hunts)
//!   IV  Up the Hill (sworn, last): at dawn, the last draw fresh in the lab; "Two is enough" spares
//!       her the dawn for 2 real days of the lab's work
//! Simplified: tonight's spot is her house (the rota moves her every 3 real days); the walk up is
//! a choice, not an escort block; R1 (her Day 2 tube) isn't a separate item.
class RZ_Route_Haven : RZ_FinaleRoute
{
	protected static const float BAG_COLD = 900;	// a cool bag: 15 minutes out of the cold

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
			case S_ACT1: return "\"There's someone. We kept her from you. We kept her from everyone.\" A woman the houses crossed off the List to keep her alive. Find her name, and where she sleeps tonight.";
			case S_DOOR: return "\"Day 2 I was on the bus. Day 4 I was in the church door. Day 6 I was meant to walk up that hill.\" Ask {woman} for her yes. Her terms end with her boy's line in the Red Hand's book.";
			case S_PIECE: return "Two bags of {woman}'s blood, drawn at dusk at the edge of her town and carried cold to the lab.";
			case S_ACT4: return "At dawn, the hour the ten walked up on Day 6: the last draw, fresh, in the lab.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override protected string RemakeText()
	{
		return "The batch was lost. {woman} can give once more, if she agrees.";
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
			gm.GetFactionLife().OpenChannelLine(RZ_EFaction.SURVIVORS, "All houses: nobody says her name on the radio. She moves tonight.");
	}

	//------------------------------------------------------------------------------------------------
	//! Draws at dusk or after dark; the walk up at dawn (05:00-09:00) unless she was spared it.
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
			return "a bag of her blood";

		return "the last draw";
	}

	//------------------------------------------------------------------------------------------------
	//! Her yes: persuade 3 on Haven rank (Nurse, Doctor or Elder +1); one easier with her name read
	//! at supper (Every Name), one harder if she was sent to the far house as a mouth.
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
		vector house = PersonAt("woman", Seat(gm, RZ_EFaction.SURVIVORS));
		scene.m_sTitle = StageTitle(stage);
		scene.m_sBrief = run.Fill(StageKnown(stage));
		switch (stage)
		{
			case S_ACT1:
			{
				scene.m_vPos = Seat(gm, RZ_EFaction.SURVIVORS);
				int chair = scene.Person(scene.m_vPos, run.Fill("{chair}"), "Talk to the chair", "\"There's someone. We kept her from you. We kept her from everyone.\"");
				scene.Choice(chair, "Ask who she is [Haven: Known]").Needs(RZ_EFaction.SURVIVORS, 1).Decides(OUT_ACT1).Says(run.Fill("\"Her name is {woman}. Don't say it on the radio.\" The chair tells you which house, and that it changes."), "heard her name from the chair");
				scene.Choice(chair, "Read her name off the List").Only("world:list.read").Decides(OUT_ACT1).Says(run.Fill("Her name was read with the rest at supper: {woman}. Uncrossed now."), "found her name on the List");
				int lookout = scene.Person(house, "a house's back door at dusk", "Look at the back door", "Two bowls go out by the back door at dusk. The house has one cat. It hasn't got a cat.");
				scene.Choice(lookout, "Watch for the second bowl [unseen]").Sneak().Decides(OUT_ACT1).Says("The second bowl goes in through the side window. Someone eats on half portions in there.", "watched a house at dusk");
				scene.Choice(lookout, "Feed the hungry house [3 food]").Sets("do:food3").Decides(OUT_ACT1).Says("The council head takes the food and answers one question. It's the right one.", "fed her house");
				int clerk = scene.Person(RZ_MajorHelp.NearSeat(gm.GetLabPosition()), "the lab's clerk", "Talk to the clerk", "The Day 6 ledger: eleven referred, ten treated, eight discharged. One did not present.");
				scene.Choice(clerk, "Read the Day 6 ledger [Halcyon: Known]").Needs(RZ_EFaction.INSTITUTE, 1).Decides(OUT_ACT1).Says("Line eleven: did not present. A house's name in the margin.", "read the Day 6 ledger");
				scene.Choice(clerk, "Pay a courier for a look [20 Favor]").Bribe(RZ_EFaction.INSTITUTE, 20).Decides(OUT_ACT1).Says("A courier copies line eleven for you, and the margin.", "bought a look at the ledger");
				return true;
			}
			case S_DOOR:
			{
				scene.m_vPos = house;
				int her = scene.Person(house, run.Fill("{woman}"), "Talk to her", "\"They crossed me off to keep me safe. Nobody asked me either.\" Her terms: not up that hill, nobody first because of guns, her name said out loud, and her boy.");
				scene.Choice(her, "Ask for her yes").Persuade(RZ_EFaction.SURVIVORS, YesStep(gm), "nurse,doctor,elder").Sets("do:yes").Says("\"Then say it. Out loud.\" You do. She nods once.", "had her yes").Refused("\"Ask me again when you mean it.\"");
				scene.Choice(her, "Ask her to give without it").Only("yes").Persuade(RZ_EFaction.SURVIVORS, Math.MinInt(4, YesStep(gm) + 1), "nurse,doctor,elder").Decides(OUT_OTHER).Says("\"He's a grown man. He chose his crew.\" She rolls up her sleeve.", "had her give without her boy's line").Refused("\"And my boy.\"");
				int boss = scene.Person(PersonAt("son", Seat(gm, RZ_EFaction.BANDITS)), "the crew boss who holds her boy's line", "Talk to the crew boss", run.Fill("\"{son}? Owes. Everyone owes. A debt is a debt.\""));
				int price = 30;
				if (Major(gm, "haven.rule") == 4)
					price = 15;

				scene.Choice(boss, string.Format("Pay his line [%1 Red Hand Favor]", price)).Only("yes").Needs(RZ_EFaction.BANDITS, 2).Bribe(RZ_EFaction.BANDITS, price).Decides(OUT_DOOR).Says("\"A debt is a debt. It was paid, by her.\" The boss marks the line paid.", "paid her boy's line");
				scene.Choice(boss, "Walk him out at night [unseen]").Only("yes").Sneak().Decides(OUT_DOOR).Says("He comes because she asked. His line moves to her house: leaving doesn't pay a debt.", "walked her boy out of the camp");
				if (Major(gm, "book.last") == 4 || Finale().PactOf(m_iRoute) == RZ_CureFinale.REDHAND)
					scene.Choice(boss, "His line's already settled").Only("yes").Decides(OUT_DOOR).Says("\"Every debt went with the book.\" Or with the pact. Either way his line is clean.", "found her boy's line already clear");

				return true;
			}
			case S_PIECE:
			{
				scene.m_vPos = RZ_Mission.LandPointNear(house, 60, 120);
				int draw = scene.Person(scene.m_vPos, "the edge of her town at dusk", "Look at the draw point", "Where the houses used to walk the bitten out at dusk. A table, a lamp, a cool bag.");
				scene.Choice(draw, "Draw a bag [her yes]").Sets("do:draw").Says("She looks away while it fills. \"Twice more. Then I'm done.\"", "drew a bag of her blood");
				int duty = DutyOfficer(gm, scene, "\"Cold, please. It goes straight in the store.\"");
				scene.Choice(duty, "Store a bag of her blood").Sets(string.Format("do:store:bag:2:%1", OUT_PIECE)).Says("Signed into the cold store in the Haven's name.", "stored her blood at the lab");
				scene.Choice(duty, "Two is enough [Haven: Trusted]").Needs(RZ_EFaction.SURVIVORS, 2).Sets("spared").Says("\"Then she doesn't walk up at dawn. The lab will make do, slower.\"", "spared her the dawn");
				return true;
			}
			case S_ACT4:
			{
				scene.m_vPos = gm.GetLabPosition();
				int gate = DutyOfficer(gm, scene, "\"Dawn. The hour the ten walked up on Day 6.\"");
				if (run.Flag("spared") != 0)
				{
					run.SetFlag("spare_until", Math.MaxInt(run.Flag("spare_until"), RZ_SeasonClock.Now() + RZ_Util.RoundToInt(172800 * gm.GetQuestlines().WaitScale())));
					scene.Choice(gate, "Ask how the work's going").Sets("do:wait:spare_until").Decides(OUT_LAST).Says("Two bags were enough. Slower, and kinder.", "waited out the lab's work");
					return true;
				}

				int walker = scene.Person(PersonAt("woman", house), run.Fill("{woman}"), "Talk to her", "\"I'll walk. Not carried.\"");
				scene.Choice(walker, "Walk up the hill with her [her yes]").Sets("walked").Says("Houses on her road light candles. She keeps her own pace.", "walked her up the hill");
				scene.Choice(gate, "Bring her in at the gate").Only("walked").Sets("do:last").Decides(OUT_LAST).Says("The last draw, fresh, in the clean room.", "brought her up the hill");
				scene.Choice(walker, "Draw at first light and run it up").Sets("do:give:last:720").Says("Twelve minutes in a cool bag. Run.", "ran the last draw up");
				scene.Choice(gate, "Hand in the last draw").Sets(string.Format("do:store:last:1:%1", OUT_LAST)).Says("The duty officer takes it straight through.", "handed in the last draw");
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override string CanDo(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, string code)
	{
		RZ_FinalePeople people = Finale().GetPeople();
		if (code == "yes")
		{
			int to = people.YesTo("woman");
			if (!people.IsAlive("woman"))
				return "She's gone.";

			if (to >= 0 && to != m_iRoute)
				return "\"I've said yes to someone already. One at a time.\"";

			return "";
		}

		if (code == "draw" || code == "last" || code.StartsWith("give:last"))
		{
			if (run.Flag("yes") == 0 && people.YesTo("woman") != m_iRoute)
				return "\"Not without my yes.\"";

			if (people.Draws("woman") >= RZ_FinalePeople.MAX_DRAWS)
				return "\"Five. I said five. Never a sixth.\"";

			int next = run.Flag("draw_unix") + RZ_Util.RoundToInt(43200 * gm.GetQuestlines().WaitScale());
			if (code == "draw" && RZ_SeasonClock.Now() < next && run.Flag("draws") > 0)
				return "\"Twelve hours between. I'm not a tap.\"";

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

		if (code == "draw")
		{
			people.Draw("woman", m_iRoute, RZ_FinalePeople.MAX_DRAWS);
			run.SetFlag("draws", run.Flag("draws") + 1);
			run.SetFlag("draw_unix", RZ_SeasonClock.Now());
			GiveItem(gm, playerId, "bag", ItemName("bag"), PlayerPos(playerId), ColdFor(gm, playerId, BAG_COLD));
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
			run.Note(run.Fill("Her name is {woman}. The houses move her at dusk."));
		else if (stage == S_DOOR && outcome == OUT_DOOR)
			run.Note(run.Fill("{son}'s line is marked paid."));
	}

	//------------------------------------------------------------------------------------------------
	override protected bool ReleaseScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene)
	{
		scene.m_sTitle = "The release";
		scene.m_sBrief = "The duty officer brings the release form to the clean room. The Haven's people choose the order.";
		scene.m_vPos = gm.GetLabPosition();
		int form = DutyOfficer(gm, scene, "\"Who goes first? Write it in. I'll read it out.\"");
		scene.Choice(form, "Her house, and her boy").Decides(OUT_TERMS + 1).Says("The first box goes to her house. Her boy carries it.", "chose her house first");
		if (Major(gm, "haven.rule") == 2)
			scene.Choice(form, "The far house first").Decides(OUT_TERMS + 2).Says("Somebody finally drives out to the far house.", "chose the far house first");

		scene.Choice(form, "By the bite book").Decides(OUT_TERMS + 3).Says("The sick first, whoever they are. The houses read the bite book, not the List.", "chose the bite book");
		scene.Choice(form, "The crossed-out first").Only("world:list.read").Decides(OUT_TERMS + 4).Says("They go down the List from the bottom.", "chose the crossed-out first");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Nobody chose: a Merciful or Idealist chair reads the bite book; else her house first.
	override int DefaultTerms(RZ_GameModeComponent gm)
	{
		if (RZ_MajorHelp.TopHas(gm, RZ_EFaction.SURVIVORS, RZ_ELeaderTrait.MERCIFUL) || RZ_MajorHelp.TopHas(gm, RZ_EFaction.SURVIVORS, RZ_ELeaderTrait.IDEALIST))
			return 3;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected string ControlledLine(int terms)
	{
		return "If you can hear this: her name is {woman}. She was bitten on Day 4, and she's still here. Bring water if you can.";
	}

	override protected string LostLine() { return "She gave three times. It went out on somebody else's terms. The Haven read her name anyway."; }
	override protected string NeverLine() { return "She was never needed. The Haven uncrossed her name anyway."; }
}
