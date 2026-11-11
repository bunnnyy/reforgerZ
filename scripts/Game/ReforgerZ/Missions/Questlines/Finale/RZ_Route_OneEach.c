//! The Independent route, One Each (docs/FINALE.md, "Independent"), key "route_one_each". On Day 6
//! ten bitten Haven people walked up the hill for the first trial dose, and eight walked out.
//! Whatever the trial dose taught their blood, they've carried it longest. Nobody owns them. The lab
//! has read its numbers on the open channel since; a retired schoolteacher with a long-wire aerial
//! (the radio ham, ham.house) has copied every one into exercise books.
//!   I   Eight Walked Out (open): who the eight are and where they are now
//!   II  Asking (sworn): each of the eight by their own yes and their keeper's leave (a council
//!       head, Sedlak, a crew boss, a field lead: every faction is a door); fewer than three alive
//!       or willing: donors at a Haven medic (the other way); then three bags in the lab's cold store
//!   III Nobody's Index (open): live tissue
//!   IV  For Everyone (sworn, last): a tray off the books, then the batch sheet: "Released to: -"
//! One unit from another route's subject counts as a bag; Twice and the Scavenger stand in when
//! fewer than three of the eight are alive. Simplified: the on-air carry isn't built.
class RZ_Route_OneEach : RZ_FinaleRoute
{
	protected static const float BAG_COLD = 900;

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		Route(RZ_CureFinale.INDEP);
	}

	//------------------------------------------------------------------------------------------------
	override protected string StageTitle(int stage)
	{
		switch (stage)
		{
			case S_ACT1: return "Eight Walked Out";
			case S_DOOR: return "Asking";
			case S_PIECE: return "Three bags";
			case S_ACT4: return "For Everyone";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override protected string StageKnown(int stage)
	{
		switch (stage)
		{
			case S_ACT1: return "Eight walked out of the Day 6 trial. Find out who they are and where they live now.";
			case S_DOOR: return "Ask the eight, one by one, at dusk at the edge of their towns. Each gives by their own yes, with their keeper's leave.";
			case S_PIECE: return "Three bags in the lab's cold store. The lab never refuses blood: the director logs it \"for everyone\".";
			case S_ACT4: return "A tray off the books, then the batch sheet: Released to: nobody.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override protected string RemakeText()
	{
		return "The batch was lost. One more bag from the eight, by consent.";
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Places(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_FinalePeople people = Finale().GetPeople();
		if (!people.Get("ham"))
			return false;

		run.SetText("ham", people.Name("ham"));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (!gm.GetFactionLife())
			return;

		gm.GetFactionLife().OpenChannelLine(RZ_EFaction.INSTITUTE, "Ninety. Ninety. Got it.");
		gm.GetFactionLife().OpenChannelLine(RZ_EFaction.SURVIVORS, "Four lots of people are about to tell you the cure is theirs. It isn't anyone's yet. If you'd rather it went to everyone, any radio will do.");
	}

	//------------------------------------------------------------------------------------------------
	//! The asking at dusk; the batch sheet at a change of shift.
	override protected bool Window(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (stage == S_DOOR)
			return Between(16.5, 22);

		if (stage == S_ACT4)
			return Between(5, 7) || Between(17, 19);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected string ItemName(string kind)
	{
		return "a cool bag from one of the eight";
	}

	//------------------------------------------------------------------------------------------------
	override protected bool RouteScene(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene)
	{
		vector lab = gm.GetLabPosition();
		scene.m_sTitle = StageTitle(stage);
		scene.m_sBrief = run.Fill(StageKnown(stage));
		switch (stage)
		{
			case S_ACT1:
			{
				vector ham = PersonAt("ham", lab);
				scene.m_vPos = ham;
				int books = scene.Person(ham, run.Fill("{ham} with the exercise books"), "Talk to the ham", "A long-wire aerial between two trees. \"In case. Somebody should.\"");
				scene.Choice(books, "Read the exercise books").Decides(OUT_ACT1).Says("Every number the lab ever read, in a schoolteacher's hand. Day 6: eight discharged, by number and date.", "read the ham's exercise books");
				int clerk = scene.Person(RZ_MajorHelp.NearSeat(lab), "the lab's clerk", "Talk to the clerk", "The Day 6 ledger, the same one the Haven reads.");
				scene.Choice(clerk, "Read the Day 6 ledger [Halcyon: Known]").Needs(RZ_EFaction.INSTITUTE, 1).Decides(OUT_ACT1).Says("Eight names beside eight numbers. A house medic matches them to faces.", "read the Day 6 ledger");
				scene.Choice(clerk, "Pay a courier for a look [20 Favor]").Bribe(RZ_EFaction.INSTITUTE, 20).Decides(OUT_ACT1).Says("A courier copies the eight lines for you.", "bought a look at the ledger");
				int doctor = scene.Person(LabGate(gm), "the Doctor at the lab gate", "Talk to the Doctor", "\"I sat with two of them that night.\"");
				scene.Choice(doctor, "Ask about the eight").Persuade(RZ_EFaction.SURVIVORS, 1, "doctor,nurse").Decides(OUT_ACT1).Says("\"Most went back to the houses. One's on a Bastion roster. One's in a crew's book. One stayed at a field station.\"", "heard about the eight from the Doctor").Refused("\"Ask me when you know why you're asking.\"");
				return true;
			}
			case S_DOOR:
			{
				array<string> eight = {};
				Finale().GetPeople().LivingEight(eight);
				scene.m_vPos = LabGate(gm);
				int asked = 0;
				foreach (string key : eight)
				{
					if (asked >= 4)
						break;

					RZ_FinalePersonSave p = Finale().GetPeople().Get(key);
					if (!p || p.yes >= 0)
						continue;

					vector at = PersonAt(key, lab);
					int who = scene.Person(at, p.name + ", one of the eight", "Talk to them", "\"Day 6. I walked up the hill and I walked back down. What do you want with that?\"");
					scene.Choice(who, AskLabel(p.faction)).Sets("do:ask:" + key).Says("A yes, and a bag, at the edge of the town at dusk.", "asked one of the eight");
					asked++;
				}

				if (eight.Count() < 3)
					run.SetFlag("short", 1);

				int gate = scene.Person(scene.m_vPos, "the Doctor at the lab gate", "Talk to the Doctor", "\"If the eight can't, the houses' donors can. It's slower.\"");
				scene.Choice(gate, "Donors at a Haven medic instead").Only("short").Decides(OUT_OTHER).Says("Donors at a Haven medic, one by one. Nobody's, all of them.", "asked the houses' donors");
				if (run.Flag("short") != 0 && Finale().GetPeople().IsAlive("twice"))
				{
					// The eight's fallback: Twice, who has the same luck, and the Scavenger
					int twice = scene.Person(PersonAt("twice", lab), "a man the crews call Twice", "Talk to Twice", "\"Killed in action once. Bitten once. Neither took. For everyone? Fine.\"");
					RZ_QM_RouteScene.Cast(scene).TrackSubject(twice, "twice");
					scene.Choice(twice, "Pay his debt and ask [30 Favor]").Sets("do:ask:twice").Says("A yes, and a bag, at the edge of the town at dusk.", "asked Twice");
				}

				if (run.Flag("short") != 0)
				{
					int scav = scene.Person(RZ_Mission.LandPointNear(scene.m_vPos, 30, 60), "the Scavenger at the lab gate", "Talk to the Scavenger", "\"Bitten twice on the road. Ate well both times. Want some?\"");
					scene.Choice(scav, "Share food and ask [2 food]").Trade(RZ_EErrandItem.FOOD, 2).Sets("do:scav").Says("A bag, given over a tin of peaches. \"For everyone. Even them.\"", "asked the Scavenger");
				}

				return true;
			}
			case S_PIECE:
			{
				scene.m_vPos = lab;
				int duty = DutyOfficer(gm, scene, "\"Blood? The lab never refuses blood. For whom?\" \"For everyone.\" \"Right.\"");
				scene.Choice(duty, "Store a bag, for everyone").Sets(string.Format("do:store:bag:3:%1", OUT_PIECE)).Says("Logged: for everyone.", "stored a bag for everyone");
				scene.Choice(duty, "Store another route's unit, for everyone").Way(RZ_EQuestApproach.BETRAY).Sets("do:store_any").Says("A bag, a vial, a dose that was meant for someone's route. Logged: for everyone.", "stored another route's unit for everyone");
				if (run.Flag("door") == OUT_OTHER)
					scene.Choice(duty, "The donors' bags, for everyone").Decides(OUT_PIECE).Says("Three donors' bags in the cold store. Logged: for everyone.", "stored the donors' bags");

				return true;
			}
			case S_ACT4:
			{
				scene.m_vPos = lab;
				int lead = scene.Person(RZ_Mission.LandPointNear(lab, 30, 60), "a field lead", "Talk to the field lead", "\"Some of us would rather give it away.\"");
				int step = 2;
				if (Major(gm, "authors.open") != 0)
					step = 1;

				scene.Choice(lead, "A tray off the books").Persuade(RZ_EFaction.INSTITUTE, step, "labtech").Sets("do:tray").Says("A tray set aside, \"spoiled\".", "had a tray set aside").Refused("\"Not one of ours. Not under nobody's name.\"");
				int clerk2 = DutyOfficer(gm, scene, "\"A tray under nobody's name. The director won't like the sheet.\"");
				scene.Choice(clerk2, "A spoiled tray [30 Favor]").Bribe(RZ_EFaction.INSTITUTE, 30).Sets("do:tray").Says("The duty officer marks a tray spoiled.", "bought a spoiled tray");
				int director = scene.Person(RZ_MajorHelp.NearSeat(lab), "the director", "Talk to the director", "The batch sheet: \"Released to: ...\"");
				scene.Choice(director, "Released to: nobody").Only("tray").Persuade(RZ_EFaction.INSTITUTE, SheetStep(gm), "labtech").Decides(OUT_LAST).Says("\"Everyone. Fine.\" The director writes a dash.", "had the director write the sheet").Refused("\"Somebody has to be on the label.\"");
				scene.Choice(clerk2, "Fill in the sheet [40 Favor]").Only("tray").Bribe(RZ_EFaction.INSTITUTE, 40).Decides(OUT_LAST).Says("The duty officer fills it in: Released to: -", "bought the batch sheet");
				scene.Choice(lead, "Fill it in yourself [unseen]").Only("tray").Sneak().Decides(OUT_LAST).Says("The records room at night. A dash where a name goes.", "filled in the batch sheet at night");
				int still = scene.Person(RZ_Mission.LandPointNear(RZ_WorldFacts.Get().CanonPos("redhand.still"), 8, 20), "the still's crew", "Talk to the crew", "\"Trays? We've one we're not using. It's yours for a price.\"");
				scene.Choice(still, "Buy a tray at the still [30 Red Hand Favor]").Bribe(RZ_EFaction.BANDITS, 30).Sets("do:tray").Says("A tray in a cold box that smells of diesel. A deal's a deal.", "bought a tray at the still");
				int store = scene.Person(RZ_Mission.LandPointNear(lab, 20, 40), "the lab's night store", "Look at the night store", "A cold room with a padlock and a rota. Nobody's on the rota at the change of shift.");
				scene.Choice(store, "Steal a tray [unseen]").Sneak().Sets("do:tray").Says("Ninety seconds. A tray under your coat, and a gap on the shelf.", "stole a tray for nobody");
				int sit = scene.Person(LabGate(gm), "the lab gate at the change of shift", "Look at the gate", "\"Released to: -\". It needs saying where the shift can see it.");
				scene.Choice(sit, "Sit in at the gate, unarmed [10 min]").Only("tray").Sets("do:sitin").HoldAt(LabGate(gm), 600, OUT_LAST, -1).Says("You sit down in the gate with nothing in your hands. The shift has to step round you.", "sat in at the lab gate");
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Their keeper's leave, by who holds them.
	protected string AskLabel(int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.MILITARY: return "Ask, with Sedlak's leave [Bastion: Known]";
			case RZ_EFaction.BANDITS: return "Pay their debt and ask [30 Favor]";
			case RZ_EFaction.INSTITUTE: return "Ask, with the field lead's leave";
		}

		return "Ask, with the council head's leave";
	}

	//------------------------------------------------------------------------------------------------
	//! The batch sheet: persuade 3; easier under The Whole of It, with an Idealist or Merciful director.
	protected int SheetStep(RZ_GameModeComponent gm)
	{
		int d = 3;
		if (Major(gm, "proof.first_page") == 2)
			d--;

		if (RZ_MajorHelp.TopHas(gm, RZ_EFaction.INSTITUTE, RZ_ELeaderTrait.IDEALIST) || RZ_MajorHelp.TopHas(gm, RZ_EFaction.INSTITUTE, RZ_ELeaderTrait.MERCIFUL))
			d--;

		return Math.ClampInt(d, 1, 4);
	}

	//------------------------------------------------------------------------------------------------
	override string CanDo(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, string code)
	{
		if (code.StartsWith("ask:"))
		{
			string key = code.Substring(4, code.Length() - 4);
			RZ_FinalePersonSave p = Finale().GetPeople().Get(key);
			if (!p || !p.alive)
				return "Nobody here answers to that.";

			if (p.yes >= 0 && p.yes != m_iRoute)
				return "\"I've given to someone already. One each.\"";

			switch (p.faction)
			{
				case RZ_EFaction.MILITARY:
				{
					if (RZ_Reputation.GetAccessRank(gm.GetTier(playerId, RZ_EFaction.MILITARY)) < 1)
						return "\"Sedlak says no. Come back when Bastion knows you.\"";

					break;
				}
				case RZ_EFaction.BANDITS:
				{
					if (!gm.SpendFavor(playerId, RZ_EFaction.BANDITS, 30))
						return "\"The crew boss owns the debt. Thirty Red Hand Favor, or it stays.\"";

					break;
				}
				case RZ_EFaction.INSTITUTE:
				{
					if (RZ_Reputation.GetAccessRank(gm.GetTier(playerId, RZ_EFaction.INSTITUTE)) < 0)
						return "\"The field lead wants her blood for the paper. Not from you.\"";

					break;
				}
				default:
				{
					if (!RZ_QuestApproach.Persuade(gm, playerId, RZ_EFaction.SURVIVORS, 2, "nurse"))
						return "\"The council head lets them choose. They choose not to, for now.\"";

					break;
				}
			}

			return "";
		}

		if (code == "tray" && !Finale().TakeTray(m_iRoute))
			return "\"No tray. Not while you've one out, or we've none in.\"";

		if (code == "scav" && run.Flag("scav") != 0)
			return "\"I gave. Once is generous.\"";

		if (code == "store_any" && Finale().GetItems().CountAny(IdentOf(gm, playerId), RZ_CureFinale.INDEP) <= 0)
			return "You haven't got anything of anyone's with you.";

		if (code == "sitin")
		{
			IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (me && RZ_Util.IsArmed(me))
				return "\"Not with that. A sit-in is hands empty.\"";

			if (!Between(17, 18) && !Between(5, 6))
				return "\"At the change of shift. Half five, morning or evening.\"";
		}

		return super.CanDo(gm, run, playerId, code);
	}

	//------------------------------------------------------------------------------------------------
	override int DoIt(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, string code)
	{
		if (code.StartsWith("ask:"))
		{
			string key = code.Substring(4, code.Length() - 4);
			RZ_FinalePeople people = Finale().GetPeople();
			people.Yes(key, m_iRoute);
			people.Draw(key, m_iRoute, 1);
			GiveItem(gm, playerId, "bag", ItemName("bag"), PlayerPos(playerId), ColdFor(gm, playerId, BAG_COLD));
			int yeses = run.Flag("yeses") + 1;
			run.SetFlag("yeses", yeses);
			if (yeses >= 3)
				return OUT_DOOR;

			return -1;
		}

		if (code == "tray")
		{
			run.SetFlag("tray", 1);
			return -1;
		}

		if (code == "scav")
		{
			run.SetFlag("scav", 1);
			GiveItem(gm, playerId, "bag", "a bag of the Scavenger's blood", PlayerPos(playerId), ColdFor(gm, playerId, BAG_COLD));
			return -1;
		}

		if (code == "store_any")
		{
			// One unit from another route's subject: her bag, a tube, a dose, a vial
			int handed = Finale().GetItems().HandInAny(IdentOf(gm, playerId), RZ_CureFinale.INDEP);
			if (handed <= 0)
				return -1;

			int stored = run.Flag("stored.bag") + 1;
			run.SetFlag("stored.bag", stored);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("The lab signs it into the cold store: %1 of 3.", stored));
			if (stored >= 3)
				return OUT_PIECE;

			return -1;
		}

		return super.DoIt(gm, run, playerId, code);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool ReleaseScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene)
	{
		scene.m_sTitle = "The release";
		scene.m_sBrief = "The duty officer brings the release form to the clean room. Nobody's people choose who it goes to.";
		scene.m_vPos = gm.GetLabPosition();
		int form = DutyOfficer(gm, scene, "\"Nobody's batch. So who's it for?\"");
		scene.Choice(form, "Open: one each").Decides(OUT_TERMS + 1).Says("The ONE EACH crate at the queue. Free, once each, first come first served.", "chose one each");
		scene.Choice(form, "Sold").Decides(OUT_TERMS + 3).Says("The same crate, with a price chalked on it. The payment goes nowhere.", "had it sold");
		scene.Choice(form, "Kept back").Decides(OUT_TERMS + 4).Says("A padlock that isn't the lab's. BACK SOON.", "kept it back");
		int chalk = scene.Person(LabGate(gm), "the crate at the lab gate", "Look at the crate", "Chalk, and a space for a condition.");
		scene.Choice(chalk, "One side: the Haven").Sets("condition", RZ_EFaction.SURVIVORS + 1).Decides(OUT_TERMS + 2).Says("All of them, exiles too.", "gave it to the Haven");
		scene.Choice(chalk, "One side: Bastion").Sets("condition", RZ_EFaction.MILITARY + 1).Decides(OUT_TERMS + 2).Says("No names taken.", "gave it to Bastion");
		scene.Choice(chalk, "One side: the Red Hand").Sets("condition", RZ_EFaction.BANDITS + 1).Decides(OUT_TERMS + 2).Says("One tin, not two.", "gave it to the Red Hand");
		scene.Choice(chalk, "One side: Halcyon").Sets("condition", RZ_EFaction.INSTITUTE + 1).Decides(OUT_TERMS + 2).Says("A home batch at every house. The method free.", "gave it to Halcyon");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Nobody chose: open.
	override int DefaultTerms(RZ_GameModeComponent gm)
	{
		return 1;
	}

	override protected string ControlledLine(int terms) { return "Eight walked out on Day 6. Their blood went to everyone."; }
	override protected string LostLine() { return "Eight walked out on Day 6. Three gave blood. Somebody else wrote the label."; }
	override protected string NeverLine() { return "Chalk on the lab gate: WE TRIED."; }
}
