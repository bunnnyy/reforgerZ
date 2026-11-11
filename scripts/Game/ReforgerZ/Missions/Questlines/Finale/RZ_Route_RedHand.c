//! The Red Hand's route, The Cook (docs/FINALE.md, "The Red Hand"), key "route_redhand". The cook
//! makes the dusk meal at the Red Hand's stronghold and, after supper, the cut dose on an old farm
//! still (redhand.still). On a tray of the lab's 90 percent batch, she says, it would hold every
//! time and keep in a pocket. Only the lab's word opens the gates, so for once the Red Hand walks
//! in through the front door.
//!   I   The Kitchen Book (open): find her, hear her four needs and her own term
//!   II  A Tray and a Night (sworn): a tray of the batch (Halcyon's door) or four stolen doses (the
//!       other way, a harder base); then the pan cut on the still at night
//!   III Fresh Stock (open): live tissue
//!   IV  The Lab's Word (sworn, last, by day): the proof on someone bitten who asked (60 percent,
//!       +15 a step, a harder base -15; the third pan always holds), then the case to the lab
//! Simplified: Who Came Back, the rival crew boss, sabotage and the still's fight are folded into
//! the proof's odds; the still's hold is one choice.
class RZ_Route_RedHand : RZ_FinaleRoute
{
	protected static const float TRAY_COLD = 900;

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		Route(RZ_CureFinale.REDHAND);
	}

	//------------------------------------------------------------------------------------------------
	override protected string StageTitle(int stage)
	{
		switch (stage)
		{
			case S_ACT1: return "The Kitchen Book";
			case S_DOOR: return "A Tray";
			case S_PIECE: return "A Night on the Still";
			case S_ACT4: return "The Lab's Word";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override protected string StageKnown(int stage)
	{
		switch (stage)
		{
			case S_ACT1: return "\"My cook says she can finish it. Find out if she's lying, and what she wants.\" She cooks the dusk meal at the stronghold.";
			case S_DOOR: return "A tray of the lab's 90 percent batch, or four stolen doses for a harder base. The tray keeps 15 minutes out of the cold.";
			case S_PIECE: return "\"Not in daylight. The smoke shows.\" The still near {still}, on the lab's night shift.";
			case S_ACT4: return "A dose to someone bitten who asks for it, told plainly. Then the case to the lab gate, by day.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override protected string RemakeText()
	{
		return "The batch was lost. The cook kept enough back for one more night on the still.";
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Places(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_FinalePeople people = Finale().GetPeople();
		if (!people.Get("cook"))
			return false;

		vector still = RZ_WorldFacts.Get().CanonPos("redhand.still");
		RZ_Settlement near = gm.GetSettlement(gm.FindNearestSettlement(still));
		string where = "the hills";
		if (near)
			where = near.m_sName;

		run.SetText("still", where);
		run.SetText("cook", people.Name("cook"));
		run.Claim(RZ_FinalePeople.Subject("cook"));
		return RZ_MajorHelp.Top(gm, RZ_EFaction.BANDITS) != null;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (gm.GetFactionLife())
			gm.GetFactionLife().OpenChannelLine(RZ_EFaction.BANDITS, "Ninety percent and it won't leave the fridge. Ours has been riding round in a lorry since Day 9. Just saying.");
	}

	//------------------------------------------------------------------------------------------------
	//! The still on the lab's night shift (20:30-05:00); the proof by day (05:30-18:30).
	override protected bool Window(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (stage == S_PIECE)
			return Between(20.5, 5);

		if (stage == S_ACT4)
			return Between(5.5, 18.5);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected string ItemName(string kind)
	{
		if (kind == "tray")
			return "a tray of the lab's batch";

		return "the cook's case";
	}

	//------------------------------------------------------------------------------------------------
	override protected bool RouteScene(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene)
	{
		vector rh = Seat(gm, RZ_EFaction.BANDITS);
		vector still = RZ_Mission.LandPointNear(RZ_WorldFacts.Get().CanonPos("redhand.still"), 3, 10);
		scene.m_sTitle = StageTitle(stage);
		scene.m_sBrief = run.Fill(StageKnown(stage));
		switch (stage)
		{
			case S_ACT1:
			{
				scene.m_vPos = rh;
				int cook = scene.Person(PersonAt("cook", rh), run.Fill("{cook} at the dusk meal"), "Talk to the cook", "\"He let people pay later. That's what he died of.\" Soup, as always.");
				scene.Choice(cook, "Ask what she needs [Red Hand: Known]").Needs(RZ_EFaction.BANDITS, 1).Sets("world:cook.heard").Decides(OUT_ACT1).Says("\"A tray of theirs. A night on the still. The lab's word. And mark my line paid the way you marked yours.\"", "heard the cook's terms");
				scene.Choice(cook, "Win her round").Persuade(RZ_EFaction.BANDITS, 2, "excon,labtech,doctor,nurse").Sets("world:cook.heard").Decides(OUT_ACT1).Says("\"Fine. A tray. A night. The lab's word. And my line.\"", "talked the cook round").Refused("\"Soup. It keeps.\"");
				scene.Choice(cook, "Follow her after supper [unseen]").Sneak().Sets("world:cook.still_known").Decides(OUT_ACT1).Says(run.Fill("A barn past {still}, smoke at night. The still."), "followed the cook to the still");
				int sister = scene.Person(PersonAt("sister", Seat(gm, RZ_EFaction.SURVIVORS)), "the cook's sister", "Talk to the sister", "\"She was a dinner lady. Now she's a chemist. Don't tell her I said so.\"");
				scene.Choice(sister, "Ask where the still is [3 food]").Sets("do:food3").Decides(OUT_ACT1).Says(run.Fill("\"A barn past {still}. Nobody lives near.\""), "asked the cook's sister");
				return true;
			}
			case S_DOOR:
			{
				scene.m_vPos = gm.GetLabPosition();
				int director = scene.Person(RZ_MajorHelp.NearSeat(gm.GetLabPosition()), "the director", "Talk to the director", "\"I want to see it keep. That's the only reason I'd do this.\"");
				scene.Choice(director, "Ask for a tray [Halcyon: Trusted]").Needs(RZ_EFaction.INSTITUTE, 2).Sets("do:tray").Decides(OUT_DOOR).Says("A tray, signed out \"for observation\".", "had a tray from the director");
				if (Finale().PactOf(m_iRoute) == RZ_CureFinale.HALCYON || Major(gm, "proof.redhand") != 0)
					scene.Choice(director, "A page of her tally for a tray").Sets("do:tray").Decides(OUT_DOOR).Says("The lab keeps the page. The method goes on the paper.", "traded a page of the tally for a tray");

				int lead = scene.Person(RZ_Mission.LandPointNear(gm.GetLabPosition(), 30, 60), "a field lead off the books", "Talk to the field lead", "\"For the field annex. Nobody counts them at night.\"");
				scene.Choice(lead, "Buy a tray [40 Favor]").Bribe(RZ_EFaction.BANDITS, 40).Sets("do:tray").Decides(OUT_DOOR).Says("A tray walks out at night \"for the field annex\".", "bought a tray off the books");
				scene.Choice(lead, "Steal one from the night store [unseen]").Sneak().Sets("do:tray_stolen").Decides(OUT_DOOR).Says("Ninety seconds at the night store. Halcyon will notice in the morning.", "stole a tray from the night store");
				int boss = scene.Person(rh, "the quartermaster", "Talk to the quartermaster", "\"Four doses, stolen fair and square. A harder base. Two nights instead of one.\"");
				scene.Choice(boss, "Cook on four stolen doses [30 Favor]").Bribe(RZ_EFaction.BANDITS, 30).Sets("harder").Decides(OUT_OTHER).Says("Four doses off the black market go in her crate.", "took four stolen doses for the base");
				return true;
			}
			case S_PIECE:
			{
				scene.m_vPos = still;
				int cooker = scene.Person(still, run.Fill("{cook} at the still"), "Talk to the cook", "\"Rudi used to say I'd poison somebody.\" Two jerrycans, six boxes of veterinary antiviral, the coil leaking.");
				scene.Choice(cooker, "Cut the pan on the tray").Sets("do:base").HoldAt(still, 480, OUT_PIECE, -1).Says("Keep the still running while she cuts the batch. The smoke will bring them.", "kept the still running");
				scene.Choice(cooker, "Run it cold [unseen]").Sneak().Sets("do:base_clean").Decides(OUT_PIECE).Says("No light, no shots. A clean run.", "ran the still cold");
				return true;
			}
			case S_ACT4:
			{
				scene.m_vPos = gm.GetLabPosition();
				int vol = scene.Person(RZ_Mission.LandPointNear(rh, 15, 40), "someone bitten who asked for it", "Talk to the volunteer", "\"It's yours? Never tried on a tray of theirs? Give it here anyway.\"");
				scene.Choice(vol, "Give the dose, told plainly").Sets("do:proof").Says("Five minutes, watched. It holds.", "gave the proof dose");
				int duty = DutyOfficer(gm, scene, "\"A case of cut doses, at the front door. The director checks it in an hour.\"");
				scene.Choice(duty, "Bring the case in").Only("proved").Sets(string.Format("do:store:case:1:%1", OUT_LAST)).Says("The director opens the case and doesn't say anything for a while.", "brought the cook's case to the lab");
				int bossman = scene.Person(rh, "the boss", "Talk to the boss", "\"Everyone inside knows. Outside it's a rumour. You'd make it a fact?\"");
				scene.Choice(bossman, "Mark her line paid").Persuade(RZ_EFaction.BANDITS, 3, "excon").Sets("world:cook.line_paid").Says("The boss marks her line on the first page: Paid. Same hand as the boss's own.", "had the cook's line marked paid").Refused("\"A debt is a debt.\"");
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override string CanDo(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, string code)
	{
		RZ_CureFinale f = Finale();
		if (code == "tray" || code == "tray_stolen")
		{
			if (!f.TakeTray(m_iRoute))
				return "\"No tray for you. Not while you've got one out, or we've none in.\"";

			return "";
		}

		if (code == "base" || code == "base_clean")
		{
			if (run.Flag("harder") == 0 && Carries(gm, playerId, "tray") <= 0)
				return "\"No tray, no batch. Bring me the tray.\"";

			return "";
		}

		if (code == "proof")
		{
			if (run.Flag("proved") != 0)
				return "\"It's proved. Take the case in.\"";

			// 60 percent, +15 a step (a clean run, Halcyon watching, the formula out), -15 for the
			// harder base; each failed pan is a step higher, and the third always holds
			float chance = 0.6 + 0.15 * run.Flag("clean") + 0.2 * run.Flag("pans");
			if (Major(gm, "authors.formula_sold") != 0)
				chance += 0.15;

			if (run.Flag("harder") != 0)
				chance -= 0.15;

			chance = Math.Clamp(chance, 0.4, 1);
			if (run.Flag("pans") >= 2 || Math.RandomFloat01() < chance)
				return "";

			run.SetFlag("pans", run.Flag("pans") + 1);
			return "It doesn't hold. The volunteer keeps the cut dose's usual luck, and the cook cooks again, a step higher.";
		}

		return super.CanDo(gm, run, playerId, code);
	}

	//------------------------------------------------------------------------------------------------
	override int DoIt(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, string code)
	{
		if (code == "tray" || code == "tray_stolen")
		{
			if (code == "tray_stolen")
				NudgeMorale(gm, RZ_EFaction.INSTITUTE, -5);

			GiveItem(gm, playerId, "tray", ItemName("tray"), gm.GetLabPosition(), ColdFor(gm, playerId, TRAY_COLD));
			return -1;
		}

		if (code == "base" || code == "base_clean")
		{
			HandIn(gm, playerId, "tray", 1);
			if (code == "base_clean")
				run.SetFlag("clean", 1);

			return -1;
		}

		if (code == "proof")
		{
			run.SetFlag("proved", 1);
			return -1;
		}

		return super.DoIt(gm, run, playerId, code);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool ReleaseScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene)
	{
		scene.m_sTitle = "The release";
		scene.m_sBrief = "The duty officer brings the release form to the clean room. The Red Hand's people set the price.";
		scene.m_vPos = gm.GetLabPosition();
		int form = DutyOfficer(gm, scene, "\"Price, then. Write it in. The director signs under protest.\"");
		scene.Choice(form, "The boss's price [Trusted]").Needs(RZ_EFaction.BANDITS, 2).Decides(OUT_TERMS + 1).Says("Two tins a dose at the crossroads. Lab stickers extra.", "set the boss's price");
		scene.Choice(form, "Pay when you can").Only("world:cook.heard").Decides(OUT_TERMS + 2).Says("One tin now, one when you come back. The unpaid lines are read out and marked paid.", "chose pay when you can");
		scene.Choice(form, "The first pan to the houses [Haven: Known]").Needs(RZ_EFaction.SURVIVORS, 1).Decides(OUT_TERMS + 3).Says("Through her sister's house, for nothing. The boss writes that house a line.", "sent the first pan to the houses");
		scene.Choice(form, "The book first").Decides(OUT_TERMS + 4).Says("Anyone with a line in the book is dosed against it. Their line grows.", "chose the book first");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override int DefaultTerms(RZ_GameModeComponent gm)
	{
		if (RZ_MajorHelp.TopHas(gm, RZ_EFaction.BANDITS, RZ_ELeaderTrait.GREEDY))
			return 1;

		return 4;
	}

	override protected string ControlledLine(int terms) { return "Every dose at the crossroads has a sticker on it now. The lab's."; }
	override protected string LostLine() { return "Our cook's pan is in their batch. Write that down."; }
	override protected string NeverLine() { return "Soup. It keeps."; }

	//------------------------------------------------------------------------------------------------
	//! The pan is cut: the case goes to whoever decided it (it keeps a week: no cold clock).
	override protected void AfterStep(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		if (stage != S_PIECE)
			return;

		string decider = run.Text("decider");
		vector at = RZ_WorldFacts.Get().CanonPos("redhand.still");
		Finale().GetItems().Give("case", ItemName("case"), m_iRoute, decider, at, -1);
	}
}
