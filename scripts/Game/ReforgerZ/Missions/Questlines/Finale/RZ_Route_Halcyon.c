//! Halcyon's route, The Lower Store (docs/FINALE.md, "Halcyon"), key "route_halcyon". The lab's batch
//! works on what's in the Zone now; it can't show it works on what got out first. The reference
//! stock is in a dewar in the closed site's lower store (origin.site.lower), behind the chain the
//! cordon detail put on at 04:10 on Day 1. Two keys were cut and signed for. "Proof" stays the
//! major's; the route never re-decides the paper's first page.
//!   I   Reference: None (open): what's missing, where, who holds the way in
//!   II  Item 114 (sworn): the key to Bastion's chain (Bastion's door), or the sergeant's second key
//!       or cutting the chain (the other way); then the vials out in something cold
//!   III The Index (open): live tissue
//!   IV  Samples First (sworn, last): the stock in the lab's freezers, the comparison (a real day and
//!       4 hours of play, quicker with Wolff's method and samples), then a 4-minute hold while the
//!       centrifuge runs
//! Simplified: the canisters' riddle, the family at dusk and the rival contracts on the way down
//! aren't built; the comparison is real time only.
class RZ_Route_Halcyon : RZ_FinaleRoute
{
	protected static const float CASE_COLD = 1800;		// the courier's case
	protected static const float TANK_COLD = 86400;		// the vet's tank

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		Route(RZ_CureFinale.HALCYON);
	}

	//------------------------------------------------------------------------------------------------
	override protected string StageTitle(int stage)
	{
		switch (stage)
		{
			case S_ACT1: return "Reference: None";
			case S_DOOR: return "Item 114";
			case S_PIECE: return "Something Cold";
			case S_ACT4: return "Samples First";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override protected string StageKnown(int stage)
	{
		switch (stage)
		{
			case S_ACT1: return "\"Batch at ninety. Reference: none.\" The lab needs the first isolate to compare against. Find out where it is.";
			case S_DOOR: return "\"By order: lower store to be secured and not opened. Day 1, 04:10. Lt H. Sedlak.\" Two keys. One is item 114 in Bastion's keeping.";
			case S_PIECE: return "The dewar in the lower store at {site}. The vials go out in something cold.";
			case S_ACT4: return "The reference stock in the lab's freezers, the comparison, then the centrifuge.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override protected string RemakeText()
	{
		return "The batch was lost. Half a comparison, with one vial less.";
	}

	override protected float RemakeMinutes() { return 240; }
	override protected float RemakeHours() { return 12; }

	//------------------------------------------------------------------------------------------------
	override protected bool Places(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		vector lower = RZ_WorldFacts.Get().CanonPos("origin.site.lower");
		if (lower == vector.Zero)
			return false;

		RZ_Settlement near = gm.GetSettlement(gm.FindNearestSettlement(lower));
		string where = "the closed site";
		if (near)
			where = near.m_sName;

		run.SetText("site", where);
		run.SetText("sergeant", Finale().GetPeople().Name("sergeant"));
		return RZ_MajorHelp.Top(gm, RZ_EFaction.INSTITUTE) != null;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (gm.GetFactionLife())
			gm.GetFactionLife().OpenChannelLine(RZ_EFaction.INSTITUTE, "Batch at ninety. Reference: none.");
	}

	//------------------------------------------------------------------------------------------------
	override protected string ItemName(string kind)
	{
		return "the reference vials";
	}

	//------------------------------------------------------------------------------------------------
	override protected bool RouteScene(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene)
	{
		vector lab = gm.GetLabPosition();
		vector lower = RZ_Mission.LandPointNear(RZ_WorldFacts.Get().CanonPos("origin.site.lower"), 3, 8);
		scene.m_sTitle = StageTitle(stage);
		scene.m_sBrief = run.Fill(StageKnown(stage));
		switch (stage)
		{
			case S_ACT1:
			{
				scene.m_vPos = lab;
				int duty = DutyOfficer(gm, scene, "\"Batch at ninety. Reference: none. You can read the rest yourself.\"");
				scene.Choice(duty, "Ask what's missing").Persuade(RZ_EFaction.INSTITUTE, 2, "labtech,doctor").Decides(OUT_ACT1).Says(run.Fill("\"The first isolate. It's in a dewar at {site}, behind Bastion's chain.\""), "had the duty officer say where").Refused("\"Reference: none. That's all I'm allowed.\"");
				if (Major(gm, "proof.first_page") == 2)
					scene.Choice(duty, "Ask the director").Decides(OUT_ACT1).Says("The director has nothing left to hide. \"The lower store. We've known since Day 3.\"", "asked the director outright");

				int stand = scene.Person(RZ_MajorHelp.NearSeat(lab), "the director's stand-in", "Talk to the stand-in", "\"We've known where it was since Day 3.\"");
				int step = 3;
				if (Major(gm, "weeks.record") != 0)
					step = 2;

				scene.Choice(stand, "Ask for the vial map").Persuade(RZ_EFaction.INSTITUTE, step, "labtech").Sets("map").Decides(OUT_ACT1).Says("A sketch of the dewar: four canisters, a surname and a date on the right one.", "had the vial map").Refused("\"Not before the paper is.\"");
				int sergeant = scene.Person(PersonAt("sergeant", lower), run.Fill("{sergeant}"), "Talk to the sergeant", "\"We chained a door up there on Day 1. Two keys. I've still got mine. Nobody ever asked.\"");
				scene.Choice(sergeant, "Ask about the door").Decides(OUT_ACT1).Says("\"The lower store. A dewar. I never looked in it.\"", "heard about the lower store from the sergeant");
				scene.Choice(duty, "Read the records room at night [unseen]").Sneak().Decides(OUT_ACT1).Says("The order on the chained door, filed. And a note: reference stock still wanted.", "read the records room at night");
				return true;
			}
			case S_DOOR:
			{
				scene.m_vPos = Seat(gm, RZ_EFaction.MILITARY);
				int adj = scene.Person(scene.m_vPos, "the CO's adjutant", "Talk to the adjutant", "\"Item one-one-four stays in the safe. CO's orders.\"");
				scene.Choice(adj, "Ask for item 114, signed out").Persuade(RZ_EFaction.MILITARY, 2, "soldier,police").Decides(OUT_DOOR).Says("Signed out to you by name. Bastion's condition goes in the terms: the result is read on its call first.", "had item 114 signed out").Refused("\"It's been signed for since Day 1. It stays signed for.\"");
				scene.Choice(adj, "Bribe the quartermaster [40 Favor]").Bribe(RZ_EFaction.MILITARY, 40).Decides(OUT_DOOR).Says("Twenty-four hours. Bring it back, or it's your name in the book.", "bought item 114 for a day");
				scene.Choice(adj, "Lift it off the key board [unseen]").Sneak().Decides(OUT_DOOR).Says("20:30, the call is being read. The board is in reach.", "took item 114 off the board");
				if (Finale().PactOf(m_iRoute) == RZ_CureFinale.BASTION || Major(gm, "proof.first_page") == 4)
					scene.Choice(adj, "Under the pact, the key").Decides(OUT_DOOR).Says("The key comes in an envelope. Bastion's condition is in the terms.", "had item 114 by the pact");

				int keyholder = scene.Person(PersonAt("sergeant", lower), run.Fill("{sergeant} at dusk"), "Talk to the sergeant", "\"Nobody's to know where I sleep.\"");
				scene.Choice(keyholder, "Ask for the second key [Haven: Known]").Needs(RZ_EFaction.SURVIVORS, 1).Decides(OUT_OTHER).Says("He hands it over without a word.", "had the sergeant's second key");
				scene.Choice(keyholder, "Cut the chain instead [loud]").Side(-1, RZ_EFaction.MILITARY, 0, 10).HoldAt(lower, 180, OUT_OTHER, -1).Says("Three minutes on the chain with a crowbar. Bastion's seal breaks, and the noise carries.", "cut Bastion's chain");
				return true;
			}
			case S_PIECE:
			{
				scene.m_vPos = lower;
				int vet = scene.Person(PersonAt("vet", lower), "the vet with the tank", "Talk to the vet", "\"The tank holds six. The first ampoules go to my house.\"");
				scene.Choice(vet, "Borrow the tank [Haven: Known]").Needs(RZ_EFaction.SURVIVORS, 1).Sets("tank").Says("A promise: her house first. The tank keeps a real day.", "borrowed the vet's tank");
				scene.Choice(vet, "Pay for the tank [25 Haven Favor]").Bribe(RZ_EFaction.SURVIVORS, 25).Sets("tank").Says("The tank, and a look that says she'll remember.", "paid for the vet's tank");
				int dewar = scene.Person(lower, "the dewar in the lower store", "Look at the dewar", "Four canisters. A clipboard by the door: \"Day 3. Chained. Soldiers gone. Four of us staying.\"");
				float hold = 240;
				if (run.Flag("map") != 0)
					hold = 160;

				scene.Choice(dewar, "Take the reference vials").Sets("do:vials").HoldAt(lower, hold, OUT_PIECE, -1).Says("A vial a minute while the cold smoke pours out. The noise carries.", "took the reference vials");
				return true;
			}
			case S_ACT4:
			{
				scene.m_vPos = lab;
				int officer = DutyOfficer(gm, scene, "\"The reference? Into the freezers. Then we compare.\"");
				scene.Choice(officer, "Hand in the reference stock").Sets("do:compare").Says("The comparison starts. A real day, more or less, while the lab has power.", "handed in the reference stock");
				scene.Choice(officer, "Hold the lab while it spins [4 min]").Only("compare").Sets("do:wait:compare_until").HoldAt(lab, 240, OUT_LAST, -1).Says("The centrifuge winds up. Everything for a mile hears it.", "held the lab through the centrifuge");
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override string CanDo(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, string code)
	{
		if (code == "compare" && Carries(gm, playerId, "vials") <= 0)
			return "You haven't got the vials with you.";

		return super.CanDo(gm, run, playerId, code);
	}

	//------------------------------------------------------------------------------------------------
	override int DoIt(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, string code)
	{
		if (code == "vials")
		{
			float cold = CASE_COLD;
			if (run.Flag("tank") != 0)
				cold = TANK_COLD;

			GiveItem(gm, playerId, "vials", ItemName("vials"), RZ_WorldFacts.Get().CanonPos("origin.site.lower"), ColdFor(gm, playerId, cold));
			return -1;
		}

		if (code == "compare")
		{
			HandIn(gm, playerId, "vials", 1);
			run.SetFlag("compare", 1);
			// A real day and 4 hours (x the wait scale); half without Wolff's method twice as long
			float hours = 28;
			if (Major(gm, "weeks.record") == 0 && Major(gm, "weeks.public") == 0 && Major(gm, "weeks.bastion") == 0)
				hours = 56;

			run.SetFlag("compare_until", RZ_SeasonClock.Now() + RZ_Util.RoundToInt(hours * 3600 * gm.GetQuestlines().WaitScale()));
			return -1;
		}

		return super.DoIt(gm, run, playerId, code);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool ReleaseScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene)
	{
		scene.m_sTitle = "The release";
		scene.m_sBrief = "The duty officer brings the release form to the clean room. Halcyon's people choose the order of the doses.";
		scene.m_vPos = gm.GetLabPosition();
		int form = DutyOfficer(gm, scene, "\"The order of the doses. Write it in. It goes on the paper.\"");
		scene.Choice(form, "The authors first").Decides(OUT_TERMS + 1).Says("Halcyon's own first, and the two Bastion men at the lab since Day 5.", "chose the authors first");
		scene.Choice(form, "The volunteers first").Decides(OUT_TERMS + 2).Says("The other two and the Day 6 cohort first, by name where the names are known.", "chose the volunteers first");
		scene.Choice(form, "By the numbers").Decides(OUT_TERMS + 3).Says("The newly bitten first. They're likeliest to walk out.", "chose by the numbers");
		scene.Choice(form, "Published").Decides(OUT_TERMS + 4).Says("The method on the open channel at six. Home batches anywhere, at the trial's odds.", "had the method published");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override int DefaultTerms(RZ_GameModeComponent gm)
	{
		if (RZ_MajorHelp.TopHas(gm, RZ_EFaction.INSTITUTE, RZ_ELeaderTrait.IDEALIST))
			return 4;

		return 3;
	}

	override protected string ControlledLine(int terms) { return "The canister's label is the paper's first table. Never its first page."; }
	override protected string LostLine() { return "The reference came down the hill. Somebody else wrote the label."; }
	override protected string NeverLine() { return "Reference stock: wanted."; }
}
