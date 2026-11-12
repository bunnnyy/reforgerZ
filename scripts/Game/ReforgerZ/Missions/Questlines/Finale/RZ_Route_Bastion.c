//! Bastion's route, Held for Collection (docs/FINALE.md, "Bastion"), key "route_bastion". On Day 2
//! the battalion's medical officer took a tube from everyone in Sedlak's column at a tent short of
//! the north road barrier. "13:10. Results to Command by radio: 4 R / {count}. Ack." At 14:00 the
//! barrier didn't lift. He wrote "Held for collection" and took the freezers to the cold store in
//! the hills (held.store), and he's still there. He opens to "collection", only the CO can say it,
//! and a tube goes only with its slip.
//!   I   Request Instructions (open): the store's grid
//!   II  Names for the tubes (sworn): the slips on the bus driver's clipboard (the Haven's door), or
//!       the CO signs a manifest from the four ledgers (the other way); then Collection: the store
//!       opened and the box out (45 minutes cold, 60 with a Doctor or Nurse carrying)
//!   III Contact, Captured (open): live tissue
//!   IV  Signed For (sworn, last): the box signed in at the lab; a short box (fewer than three good
//!       tubes) waits 2 real days a missing tube (the long way). Bringing Twice home is Bastion's own
//!       way to a fourth tube.
//! Branch: Processing Again (the captain gone or the freezers drained): donors and Twice, boxed short.
//! Simplified: the fuel clock is the refuel way only; the stage road posts, truck and squad aren't
//! built; the four ledgers are one manifest choice.
class RZ_Route_Bastion : RZ_FinaleRoute
{
	protected static const float BOX_COLD = 2700;

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		Route(RZ_CureFinale.BASTION);
	}

	//------------------------------------------------------------------------------------------------
	override protected string StageTitle(int stage)
	{
		switch (stage)
		{
			case S_ACT1: return "Request Instructions";
			case S_DOOR: return "Names for the tubes";
			case S_PIECE: return "Collection";
			case S_ACT4: return "Signed For";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override protected string StageKnown(int stage)
	{
		switch (stage)
		{
			case S_ACT1: return "\"Stores, detached. Held for collection, {held_count}. Request instructions. Over.\" Nobody has replied since Day 2. Find the store's grid.";
			case S_DOOR: return "The captain releases a tube only with its slip. The slips are on the bus driver's clipboard, at a Haven house.";
			case S_PIECE: return "The cold store near {store}. Only the CO can say collection. Bring the box out cold.";
			case S_ACT4: return "The box to the lab, signed in against a tray. The receipt is read on the 20:30 call.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override protected string RemakeText()
	{
		return "The batch was lost. One session of the lab's work on what it kept.";
	}

	override protected float RemakeMinutes() { return 120; }
	override protected float RemakeHours() { return 12; }

	//------------------------------------------------------------------------------------------------
	override protected string SubjectKey()
	{
		return "captain";
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Places(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_FinalePeople people = Finale().GetPeople();
		vector store = people.Pos("captain");
		if (store == vector.Zero)
			return false;

		RZ_Settlement near = gm.GetSettlement(gm.FindNearestSettlement(store));
		string where = "the hills";
		if (near)
			where = near.m_sName;

		run.SetText("store", where);
		run.SetText("captain", people.Name("captain"));
		run.Claim(RZ_FinalePeople.Subject("captain"));
		run.SetText("held_count", RZ_WorldFacts.Get().CanonInt("held.count", 280, 340).ToString());
		run.SetFlag("tubes", 3);
		return RZ_MajorHelp.Top(gm, RZ_EFaction.MILITARY) != null;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (!gm.GetFactionLife())
			return;

		if (Major(gm, "orders.stance") == 3)
			gm.GetFactionLife().OpenChannelLine(RZ_EFaction.MILITARY, "Stores, Command. Hold for collection. Command out.");
		else
			gm.GetFactionLife().OpenChannelLine(RZ_EFaction.MILITARY, run.Fill("...Stores, detached. Held for collection, {held_count}. Fuel, low. Request instructions. Over. No reply."));
	}

	//------------------------------------------------------------------------------------------------
	override protected string ItemName(string kind)
	{
		return "the captain's box";
	}

	//------------------------------------------------------------------------------------------------
	//! The CO's release: persuade 3 on Bastion rank (Soldier or Police +1); one easier under
	//! Bastion, Command (stance 3), one harder under Expended (stance 4).
	protected int ReleaseStep(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		int d = 3;
		int stance = Major(gm, "orders.stance");
		if (stance == 3)
			d--;

		if (stance == 4)
			d++;

		d -= RZ_MajorHelp.PageStep(gm, run);
		return Math.ClampInt(d, 1, 4);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool RouteScene(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene)
	{
		vector seat = Seat(gm, RZ_EFaction.MILITARY);
		vector store = PersonAt("captain", seat);
		RZ_FinalePeople people = Finale().GetPeople();
		scene.m_sTitle = StageTitle(stage);
		scene.m_sBrief = run.Fill(StageKnown(stage));
		switch (stage)
		{
			case S_ACT1:
			{
				scene.m_vPos = seat;
				int sedlak = scene.Person(seat, "Sedlak at the roster board", "Talk to Sedlak", "\"Only the CO can say collection.\" The roster says: Detached, stores: 1.");
				scene.Choice(sedlak, "Ask where the stores are [Trusted]").Needs(RZ_EFaction.MILITARY, 2).Decides(OUT_ACT1).Says(run.Fill("Up the hill road past {store}. He logs at 20:30. Nobody answers."), "had the store's grid from Sedlak");
				scene.Choice(sedlak, "Bribe the quartermaster [20 Favor]").Bribe(RZ_EFaction.MILITARY, 20).Decides(OUT_ACT1).Says("\"Everything's signed for. Even the man nobody signs out.\" A grid on a fuel chit.", "bought the grid off the quartermaster");
				scene.Choice(sedlak, "Read the fuel log in a Bastion coat").Trick(RZ_EFaction.MILITARY, 2).Decides(OUT_ACT1).Says("Twenty litres a week to \"Stores, detached\", and a grid. Nobody looks up.", "read the fuel log in borrowed kit").Refused("Name and unit?");
				int driver = scene.Person(Seat(gm, RZ_EFaction.SURVIVORS), "the bus driver", "Talk to the driver", "I drove them up on Day 2. I drove them back.");
				scene.Choice(driver, "Ask about the cold boxes [Haven: Known]").Needs(RZ_EFaction.SURVIVORS, 1).Decides(OUT_ACT1).Says("The medics' truck took the cold boxes up the hill road. I watched it go.", "had the hill road from the bus driver");
				int fence = scene.Person(Seat(gm, RZ_EFaction.BANDITS), "the fence with the book", "Talk to the fence", "Diesel, twenty litres, every week. Stores, it says. Funny sort of stores.");
				scene.Choice(fence, "Read the diesel line [15 Favor]").Bribe(RZ_EFaction.BANDITS, 15).Sets("world:held.rh_knows").Decides(OUT_ACT1).Says("The diesel line has a grid beside it. The boss knows it now too.", "read the diesel line in the book");
				scene.Choice(fence, "Lean on the fence").Threaten(RZ_EFaction.BANDITS, 2).Decides(OUT_ACT1).Says("He reads the grid out without opening the book. He'll tell the boss you asked.", "leaned on the fence").Refused("Read your own book.");
				return true;
			}
			case S_DOOR:
			{
				scene.m_vPos = Seat(gm, RZ_EFaction.SURVIVORS);
				int keeper = scene.Person(scene.m_vPos, "the bus driver", "Talk to the driver", "The clipboard is in the house office. \"They were my passengers.\"");
				scene.Choice(keeper, "Ask for the slips [Haven: Known]").Persuade(RZ_EFaction.SURVIVORS, 2, "police,elder").Decides(OUT_DOOR).Says("\"They were my passengers. Give them their names back.\" The slips come off the clipboard.", "had the slips from the driver").Refused("Not to Bastion. Not after the barrier.");
				scene.Choice(keeper, "Take the clipboard at night [unseen]").Sneak().Decides(OUT_DOOR).Says("The clipboard goes under your coat. The slips are still stapled to it.", "took the clipboard at night");
				scene.Choice(keeper, "Lean on the driver").Threaten(RZ_EFaction.SURVIVORS, 2).Decides(OUT_DOOR).Says("He hands over the slips with shaking hands. The house will hear how you asked.", "leaned on the bus driver").Refused("I drove through worse than you.");
				if (Major(gm, "haven.rule") == 3 || Finale().PactOf(m_iRoute) == RZ_CureFinale.HAVEN)
					scene.Choice(keeper, "The census stands in for the slips").Decides(OUT_DOOR).Says("The census has every name on the bus. Counted, after all.", "used the census for the slips");

				int co = scene.Person(RZ_MajorHelp.NearSeat(seat), "the CO's adjutant", "Talk to the adjutant", "Four ledgers. Any three name the tubes. The CO signs if they agree.");
				scene.Choice(co, "Put the four ledgers together [Trusted]").Needs(RZ_EFaction.MILITARY, 2).Sets("world:held.named").Decides(OUT_OTHER).Says("The CO signs a manifest: \"Collection. List to follow.\"", "had the CO sign a manifest");
				scene.Choice(co, "Trade the slips' copies [2 food]").Trade(RZ_EErrandItem.FOOD, 2).Decides(OUT_OTHER).Says("The ration clerk keeps copies of everything. Two tins buys a look at Day 2.", "traded for the ration clerk's copies");
				return true;
			}
			case S_PIECE:
			{
				scene.m_vPos = store;
				if (!people.IsAlive("captain") || IsDrained())
					return AgainScene(gm, run, scene, seat);

				int captain = scene.Person(store, run.Fill("{captain} at the cold store"), "Talk to the captain", "Two chest freezers, a generator and a log. \"Held for collection. Are you collection?\"");
				RZ_QM_RouteScene.Cast(scene).TrackSubject(captain, "captain");
				scene.Choice(captain, "Say collection, on the CO's word").Persuade(RZ_EFaction.MILITARY, ReleaseStep(gm, run), "soldier,police").Sets("do:box").Decides(OUT_PIECE).Says("\"Stores held for collection released to bearer. Over.\" He packs the box himself.", "had the CO release the stores").Refused("Only the CO says that.");
				scene.Choice(captain, "A signal form saying bearer [40 Favor]").Bribe(RZ_EFaction.MILITARY, 40).Sets("do:box_dirty").Decides(OUT_PIECE).Says("He opens for the form. The CO will find out at the receipt.", "bought a signal form");
				scene.Choice(captain, "Take the box while he sleeps [unseen]").Sneak().Sets("do:box_dirty").Decides(OUT_PIECE).Says("He logs \"Stores missing\" at 20:30. You're well down the hill.", "took the box while he slept");
				scene.Choice(captain, "Cut the door [loud]").Side(-1, RZ_EFaction.MILITARY, 0, 15).Sets("do:box_cut").Decides(OUT_PIECE).Says("He pulls a plug before he goes down. One tube spoils.", "cut the cold store's door");
				int shed = scene.Person(RZ_Mission.LandPointNear(store, 6, 12), "the generator shed", "Look at the generator", "The gauge is on the last line. A tube spoils every eight dry hours.");
				scene.Choice(shed, "Refuel it and ask him [1 fuel]").Trade(RZ_EErrandItem.FUEL, 1).Persuade(RZ_EFaction.MILITARY, Math.MaxInt(1, ReleaseStep(gm, run) - 1), "soldier,police,mechanic").Sets("do:box").Decides(OUT_PIECE).Says("The generator coughs and holds. \"Collection. Thank you.\" He means the diesel.", "refuelled the cold store and had the release").Refused("Diesel's diesel. Orders are orders.");
				scene.Choice(shed, "Read collection off a signals pad").Trick(RZ_EFaction.MILITARY, 2).Sets("do:box_dirty").Decides(OUT_PIECE).Says("A signals runner, a pad, the right words. He packs the box and logs your face.", "passed as a signals runner").Refused("Signals don't walk up here.");
				int twice = scene.Person(PersonAt("twice", Seat(gm, RZ_EFaction.BANDITS)), "a man the crews call Twice", "Talk to Twice", "\"Killed in action once. Bitten once. Neither took.\" His nail on the board is empty.");
				RZ_QM_RouteScene.Cast(scene).TrackSubject(twice, "twice");
				scene.Choice(twice, "Walk him home to a post").Persuade(RZ_EFaction.BANDITS, 3, "soldier,excon").Sets("do:twice").Says("At the roll he answers \"Here\" to his own name. The captain releases his tube to its own person.", "brought Twice home").Refused("Bastion's looking for me. You're Bastion.");
				scene.Choice(twice, "Buy him out of his crew [40 Red Hand Favor]").Bribe(RZ_EFaction.BANDITS, 40).Sets("do:twice").Says("The crew strikes his line. He walks to the post on his own.", "bought Twice out of his crew");
				FreeChoice(gm, run, scene, shed, "captain");
				RivalSpot(gm, run, scene, store, "captain", "the captain", "the freezers");
				return true;
			}
			case S_ACT4:
			{
				scene.m_vPos = gm.GetLabPosition();
				int duty = DutyOfficer(gm, scene, "Bastion's box? Against a tray, an hour. Then the receipt goes on your call.");
				if (run.Flag("short_until") > 0)
					scene.Choice(duty, "Ask how the long way's going").Sets("do:wait:short_until").Decides(OUT_LAST).Says("The receipt goes out short. Each missing tube cost two days.", "waited out the long way");
				else
					scene.Choice(duty, "Sign the box in").Sets("do:signin").Says("Signed in, against a tray.", "signed the box in at the lab");

				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Processing Again (the branch: the captain is gone, or the freezers spoiled): tubes from the
	//! living. Twice (his draw, by his crew's term or 40 Red Hand Favor), donors from the column at a
	//! post ("Give blood"), then the lab boxes what there is. Fewer than three is the long way.
	protected bool AgainScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene, vector seat)
	{
		scene.m_sBrief = "The freezers are gone. Processing again, from the living: whoever was in the column, and whoever gives.";
		scene.m_vPos = seat;
		int post = scene.Person(seat, "the medic at the post", "Talk to the medic", "Processing again. Names, a tube each, the same as Day 2. This time we keep the slips.");
		scene.Choice(post, "Give blood").Sets("do:donor").Says("A tube with your name on it, in a rack marked D.", "gave blood for the column");
		scene.Choice(post, "Box what we have").Sets("do:box_again").Decides(OUT_PIECE).Says("The medic packs the rack in a cool box. \"Short. It'll do the long way.\"", "boxed the column's tubes");
		int twice = scene.Person(PersonAt("twice", Seat(gm, RZ_EFaction.BANDITS)), "a man the crews call Twice", "Talk to Twice", "Killed in action once. Bitten once. Neither took.");
		RZ_QM_RouteScene.Cast(scene).TrackSubject(twice, "twice");
		scene.Choice(twice, "Walk him home to a post").Persuade(RZ_EFaction.BANDITS, 3, "soldier,excon").Sets("do:twice").Says("At the roll he answers \"Here\". His tube goes in the rack first.", "brought Twice home").Refused("Bastion's looking for me. You're Bastion.");
		scene.Choice(twice, "Buy him out of his crew [40 Red Hand Favor]").Bribe(RZ_EFaction.BANDITS, 40).Sets("do:twice").Says("The crew strikes his line. He gives a tube at the post.", "bought Twice out of his crew");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string CanDo(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, string code)
	{
		if (code == "signin" && Carries(gm, playerId, "box") <= 0)
			return "You haven't got the box with you.";

		if (code == "twice")
		{
			if (!Finale().GetPeople().IsAlive("twice"))
				return "Nobody here answers to that.";

			int to = Finale().GetPeople().YesTo("twice");
			if (to >= 0 && to != m_iRoute)
				return "I've given to someone already.";

			if (run.Flag("twice_home") != 0)
				return "Here. I said it already.";
		}

		if (code == "donor" && run.Flag("donor." + IdentOf(gm, playerId).Hash().ToString()) != 0)
			return "One each. That's the rule this time.";

		if (code == "box_again" && run.Flag("again") <= 0)
			return "An empty rack is an empty rack.";

		return super.CanDo(gm, run, playerId, code);
	}

	//------------------------------------------------------------------------------------------------
	override int DoIt(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, string code)
	{
		if (code == "box" || code == "box_dirty" || code == "box_cut")
		{
			if (code != "box")
				run.SetFlag("dirty", 1);

			if (code == "box_cut")
				run.SetFlag("tubes", run.Flag("tubes") - 1);

			GiveItem(gm, playerId, "box", ItemName("box"), PersonAt("captain", PlayerPos(playerId)), ColdFor(gm, playerId, BOX_COLD));
			return -1;
		}

		if (code == "twice")
		{
			Finale().GetPeople().Yes("twice", m_iRoute);
			run.SetFlag("twice_home", 1);
			run.SetFlag("tubes", run.Flag("tubes") + 1);
			run.SetFlag("again", run.Flag("again") + 1);
			RZ_WorldFacts.Get().SetFlag("held.processed", 1);
			return -1;
		}

		if (code == "donor")
		{
			run.SetFlag("donor." + IdentOf(gm, playerId).Hash().ToString(), 1);
			run.SetFlag("again", run.Flag("again") + 1);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("Tubes in the rack: %1.", run.Flag("again")));
			return -1;
		}

		if (code == "box_again")
		{
			run.SetFlag("dirty", 1);
			run.SetFlag("tubes", run.Flag("again"));
			RZ_WorldFacts.Get().SetFlag("held.processed", 1);
			GiveItem(gm, playerId, "box", ItemName("box"), PlayerPos(playerId), ColdFor(gm, playerId, BOX_COLD));
			return -1;
		}

		if (code == "signin")
		{
			HandIn(gm, playerId, "box", 1);
			int missing = Math.MaxInt(0, 3 - run.Flag("tubes"));
			if (missing <= 0)
				return OUT_LAST;

			// The long way: the receipt goes out short, 2 real days a missing tube
			run.SetFlag("short_until", RZ_SeasonClock.Now() + RZ_Util.RoundToInt(missing * 172800 * gm.GetQuestlines().WaitScale()));
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("The box is %1 short of three good tubes. The lab will need longer.", missing));
			return -1;
		}

		return super.DoIt(gm, run, playerId, code);
	}

	//------------------------------------------------------------------------------------------------
	override protected void AfterStep(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		if (stage == S_ACT1)
			run.Note(run.Fill("The cold store is in the hills near {store}."));

		if (stage == S_ACT4 && gm.GetFactionLife())
			gm.GetFactionLife().OpenChannelLine(RZ_EFaction.MILITARY, "Command, Bastion. Stores held for collection: collected. Over. Bastion out.");
	}

	//------------------------------------------------------------------------------------------------
	override protected bool ReleaseScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene)
	{
		scene.m_sTitle = "The release";
		scene.m_sBrief = "The duty officer brings the release form to the clean room. Bastion's people choose the order.";
		scene.m_vPos = gm.GetLabPosition();
		int form = DutyOfficer(gm, scene, "Who goes first? Write it in. I'll read it out.");
		if (run.Flag("dirty") == 0)
			scene.Choice(form, "The roster").Decides(OUT_TERMS + 1).Says("Soldiers first, then civilians attached. Anyone can sign on at a post.", "chose the roster");

		scene.Choice(form, "The column").Decides(OUT_TERMS + 2).Says("The living of the Day 2 column first, by their slips.", "chose the column");
		scene.Choice(form, "By need [Trusted]").Needs(RZ_EFaction.MILITARY, 2).Decides(OUT_TERMS + 3).Says("The sick first. Whoever.", "chose by need");
		if (Major(gm, "orders.stance") == 3)
			scene.Choice(form, "The census").Decides(OUT_TERMS + 4).Says("Signed for everyone, Command's way: names taken at the table.", "chose the census");

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override int DefaultTerms(RZ_GameModeComponent gm)
	{
		if (Major(gm, "orders.stance") == 3)
			return 4;

		if (RZ_MajorHelp.TopHas(gm, RZ_EFaction.MILITARY, RZ_ELeaderTrait.MERCIFUL))
			return 3;

		return 2;
	}

	override protected string ControlledLine(int terms) { return "Command, Bastion. Stores held for collection: collected. Over."; }
	override protected string LostLine() { return "Stores held for collection: signed for. Not ours."; }
	override protected string NeverLine() { return "Stores held for collection, {held_count}. Over. No reply."; }
}
