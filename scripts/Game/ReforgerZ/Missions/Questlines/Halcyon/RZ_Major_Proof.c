//! Proof: Halcyon's major (docs/QUESTLINES.md, "Proof"), key "proof". It runs two of Halcyon's
//! questlines as inner lines under their own keys, saves and stage missions: HC1 The Three Weeks
//! ("weeks") as act II and HC3 Authors ("authors") as act III and the stand. Neither starts on its
//! own while the majors run.
//!
//! I. Day One (Early Outbreak to 25%): the clinic's file disagrees with the paper's first page.
//!    Hand it in sealed, ask the director to read the date, copy the margin first, or carry it to
//!    Bastion, the Haven or the Red Hand (proof.day_one 1-4). It finds the clinic records
//!    (weeks.records), so HC1 can start.
//! II. The Three Weeks (HC1, as built): proof.weeks and proof.pages from its ending.
//! III. Authors (HC3 stages 1-4, as built).
//! IV. The reading (75%, the last climax, a set piece at the lab's change of shift): the first page
//!     (proof.first_page 1 Day 1, 2 the whole of it, 4 Bastion's countersignature). Then Before the
//!     stand, until the stand begins: sell the director's pages to the boss's man (proof.pages 5),
//!     clear that line (pay it, take the debt, lift them unseen), or swap the first page in the
//!     records room unseen (proof.first_page 3). Settled as the stand begins (OnFinalStandStart):
//!     a line nobody cleared is read on the Red Hand's channel, Two Tins a Copy.
//! V. The final stand (the finale owns it): read as data when the cure is out.
//! VI. The Paper (the epilogue's coda): the case carried to a table on the north road, just outside
//!     the kill zone.
//!
//! Simplified (docs/QUESTLINES.md): Day One is a scene (the file and four takers), not Patient
//! Zero's Ledger; Before the stand's swap is any hour, unseen, with no disguise of its own; Unsigned is
//! the default when no cure comes.
class RZ_Major_Proof : RZ_Questline
{
	static const string KEY = "proof";

	static const int S_DAYONE = 100;
	static const int S_WEEKS = 110;
	static const int S_AUTHORS = 120;
	static const int S_READING_WAIT = 130;
	static const int S_READING = 131;
	static const int S_BEFORE = 135;
	static const int S_STAND = 140;
	static const int S_PAPER_WAIT = 150;
	static const int S_PAPER = 151;

	static const int A_DAYONE = 1;
	static const int A_WEEKS = 2;
	static const int A_AUTHORS = 3;
	static const int A_READING = 4;
	static const int A_STAND = 5;
	static const int A_PAPER = 6;

	static const int OUT_SEALED = 30;		// proof.day_one 1
	static const int OUT_READ_DATE = 31;	// 2
	static const int OUT_TO_BASTION = 32;	// 3
	static const int OUT_PUBLIC = 33;		// 4
	static const int OUT_DAY1 = 40;			// proof.first_page 1
	static const int OUT_WHOLE = 41;		// 2
	static const int OUT_STOVED = 42;		// the pages in the stove for Favor: Day 1
	static const int OUT_SEAL = 43;			// 4
	static const int OUT_CARRIED = 50;

	static const int E_M_WHOLE = 10;
	static const int E_M_SEAL = 11;
	static const int E_M_DAY1 = 12;
	static const int E_M_UNSIGNED = 13;
	static const int E_M_TWOTINS = 14;

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		Info(KEY, "Proof", RZ_EFaction.INSTITUTE);
		Major();

		RZ_QuestStage dayOne = MissionStage(S_DAYONE, "Day One", "\"Day 11. Overnight numbers. The paper has a first page. It says the outbreak began on Day 1.\" The clinic in {clinic} has a file that disagrees.");
		dayOne.GiveUp(4, "");

		RZ_QuestStage weeks = WaitStage(S_WEEKS, "The Three Weeks", "\"Field antiviral. Chapter two. We'd like to thank the couriers.\" Somewhere between the clinic and the hill is what the lab did with three weeks.");
		weeks.Wait(0);

		RZ_QuestStage authors = WaitStage(S_AUTHORS, "Authors", "Bite treatment. The method exists now. So does the question of who owns it.");
		authors.Wait(0);

		RZ_QuestStage readingWait = WaitStage(S_READING_WAIT, "The reading", "The methods chapter is nearly written. The first page is read to the authors at the change of shift once it's done.");
		readingWait.Wait(0);

		RZ_QuestStage reading = MissionStage(S_READING, "The reading", "Immunity serum. The methods chapter is eight in ten; we write down the other two. The first page is read to the authors at the change of shift.");
		reading.GiveUp(4, "");

		RZ_QuestStage before = MissionStage(S_BEFORE, "Before the stand", "The first page is written. Until the batch is made, the director's three pages are still loose somewhere, and so is the draft in the records room.");
		before.GiveUp(4, "");

		RZ_QuestStage stand = WaitStage(S_STAND, "The final stand", "We're making the batch. Whoever is at the lab in the next fifteen minutes is on the paper.");
		stand.Wait(0);

		RZ_QuestStage paperWait = WaitStage(S_PAPER_WAIT, "The paper", "The paper is finished. The director will read it on the open channel before it goes into the case.");
		paperWait.Wait(0);

		RZ_QuestStage paper = MissionStage(S_PAPER, "The paper", "The director reads the paper on the open channel, then seals it in a steel case. Somebody carries it to a table on the north road, just outside the line.");
		paper.GiveUp(4, "");

		Act(A_DAYONE, "Day One").Stage(S_DAYONE).AnchorCure(25).Need(2).LastCall("If nobody brings the clinic's file in by the change of shift, the field lead goes to the clinic in person.");
		Act(A_WEEKS, "The Three Weeks").Stage(S_WEEKS).AnchorCure(50).Need(4).Watch().LastCall("Nobody has asked about the first three weeks. The lab will take that as an answer.");
		Act(A_AUTHORS, "Authors").Stage(S_AUTHORS).AnchorCure(75).Need(3).Watch();
		Act(A_READING, "The reading").Stage(S_READING_WAIT).Stage(S_READING).Opens(75).Need(1).PatienceEvenings(2).SetPiece(17.5, 20.5).Climax().LastCall("The director reads the first page at tomorrow's change of shift. The draft stands if nobody asks.");
		Act(A_STAND, "The final stand").Stage(S_BEFORE).Stage(S_STAND).PastGate();
		Act(A_PAPER, "The paper").Stage(S_PAPER_WAIT).Stage(S_PAPER).AnchorWipe().Need(0).Rank(0);
	}

	//------------------------------------------------------------------------------------------------
	//! Early Outbreak: the lab, the director and the clinic town.
	override bool CanStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_Leader director = RZ_MajorHelp.Top(gm, RZ_EFaction.INSTITUTE);
		if (!gm.HasLab() || !director || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return false;

		int clinic = RZ_WorldFacts.Get().CanonSettlement("origin.clinic");
		RZ_Settlement town = gm.GetSettlement(clinic);
		if (!town)
			return false;

		run.SetText("director", director.m_sName);
		run.SetText("clinic", town.m_sName);
		run.SetFlag("clinic", clinic);
		run.SetPlace(gm.GetLabPosition(), "the lab");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (gm.GetFactionLife())
			gm.GetFactionLife().OpenChannelLine(RZ_EFaction.INSTITUTE, "Day 11. Overnight numbers follow. The paper has a first page. It says the outbreak began on Day 1.");
	}

	//------------------------------------------------------------------------------------------------
	override bool IsReady(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		switch (stage)
		{
			case S_DAYONE: return true;
			case S_WEEKS: return InnerDone(gm, run, "weeks", RZ_Majors.Progress() >= 25 || run.Flag("copied") == 1 || RZ_MajorHelp.PlayedSince(run, 2));
			case S_AUTHORS: return InnerDone(gm, run, "authors", RZ_Majors.Progress() >= 50);
			case S_READING_WAIT:
			{
				if (RZ_Majors.Progress() >= 90)
					return true;

				return RZ_Majors.ClimaxDone("every_name") && RZ_Majors.Reached(run, 75);
			}
			case S_READING: return Free(gm, run, S_READING);
			case S_BEFORE: return gm.GetCure() && !gm.GetCure().IsFinalStand() && !gm.GetCure().IsCured();
			case S_STAND: return gm.GetCure() && gm.GetCure().IsCured();
			case S_PAPER_WAIT: return RZ_MajorHelp.CodaDue(gm, "orders");
			case S_PAPER: return RZ_GameTime.GetHour() < 0 || RZ_GameTime.GetHour() >= 17.5 || !RZ_GameTime.IsCalibrated();
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! An inner line: begun once `due` (Begin, tried every few minutes), and done once it isn't
	//! running any more (an ending, or it gave up). Authors counts as done for act III once it has
	//! reached Ready the lab, its stage 5.
	protected bool InnerDone(RZ_GameModeComponent gm, RZ_QuestRun run, string key, bool due)
	{
		RZ_Questlines q = gm.GetQuestlines();
		string begun = key + "_begun";
		if (run.Flag(begun) == 0)
		{
			if (!due || q.GetClock() < run.Flag(key + "_try"))
				return false;

			run.SetFlag(key + "_try", RZ_Util.RoundToInt(q.GetClock() + 300));
			if (q.IsRunning(key) || q.HasEnded(key) || q.Begin(key))
			{
				run.SetFlag(begun, 1);
				RZ_Majors.MarkBeat(run);
			}

			return false;
		}

		RZ_QuestRun inner = q.FindRunByKey(key);
		if (!inner)
			return true;

		return key == "authors" && inner.m_iStage >= RZ_Quest_Authors.S_READY;
	}

	//------------------------------------------------------------------------------------------------
	protected bool Free(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		RZ_SetPieceCalendar cal = RZ_SetPieceCalendar.Get();
		RZ_MajorAct act = ActOf(stage);
		if (!cal || !act || !RZ_Majors.InWindow(act) || (gm.GetCure() && gm.GetCure().IsFinalStand()))
			return false;

		return RZ_MajorHelp.HoldEvening(run, string.Format("%1.%2", KEY, stage), gm.GetLabPosition());
	}

	//------------------------------------------------------------------------------------------------
	protected void ReleaseEvening(int stage)
	{
		RZ_SetPieceCalendar cal = RZ_SetPieceCalendar.Get();
		if (cal)
			cal.Release(string.Format("%1.%2", KEY, stage));
	}

	//------------------------------------------------------------------------------------------------
	override RZ_Mission MakeMission(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		return new RZ_QM_MajorScene();
	}

	// ================================================================ Scenes
	//------------------------------------------------------------------------------------------------
	override bool BuildScene(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene)
	{
		RZ_Leader director = RZ_MajorHelp.Top(gm, RZ_EFaction.INSTITUTE);
		if (!director || !gm.HasLab())
			return false;

		scene.m_iGiver = RZ_EFaction.INSTITUTE;
		switch (stage)
		{
			case S_DAYONE: return DayOneScene(gm, run, scene, director);
			case S_READING: return ReadingScene(gm, run, scene, director);
			case S_BEFORE: return BeforeScene(gm, run, scene, director);
			case S_PAPER: return PaperScene(gm, run, scene, director);
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool DayOneScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene, RZ_Leader director)
	{
		RZ_Settlement clinic = gm.GetSettlement(run.Flag("clinic", -1));
		if (!clinic)
			return false;

		scene.m_sTitle = "Day One";
		scene.m_sBrief = run.Fill("The clinic in {clinic} kept a file on its first case. \"Notify the Institute\" is underlined twice. The date on it is three weeks before the gates closed. Bring it to the lab, or to whoever you think should read it.");
		scene.m_sObjective = run.Fill("Take the first case's file from the clinic in {clinic} to a reader you choose.");
		scene.m_eTier = RZ_ETier.KNOWN;
		scene.m_iFame = 10;
		scene.m_iFavor = 15;
		scene.m_vPos = RZ_Mission.LandPointNear(clinic.m_vCenter, 10, clinic.m_fRadius);
		scene.m_fWindow = 5400;
		int file = scene.Person(scene.m_vPos, "the clinic's records room", "Read the file", "A patient file, a ward ledger and three hospital bracelets. In the margin, in a hurry: notify the Institute. Underlined twice.");
		scene.Choice(file, "Copy the margin first").Sneak().Sets("copied", 1).Sets("world:proof.copy").Says("You copy the margin onto the back of your hand, then onto paper. The copy is yours.", "copied the clinic file's margin");

		int duty = scene.Person(RZ_MajorHelp.NearSeat(gm.GetLabPosition()), "the lab's duty officer", "Talk to the duty officer", "For the director? Sealed, please. The director reads things in order.");
		scene.Choice(duty, "Hand it in sealed").Decides(OUT_SEALED).Says("The duty officer signs for it and doesn't look inside. The first page still says Day 1.", "handed in the clinic file sealed");
		scene.Choice(duty, "Ask the director to read the date").Persuade(RZ_EFaction.INSTITUTE, 2, "doctor,labtech").Decides(OUT_READ_DATE).Says("At the change of shift the director reads one date on the open channel, and nothing after it.", "had the director read the date").Refused("The director will read it when the director reads it.");

		RZ_Leader co = RZ_MajorHelp.Top(gm, RZ_EFaction.MILITARY);
		if (co)
		{
			int adj = scene.Person(RZ_MajorHelp.NearSeat(co.m_vPos), "the CO's adjutant", "Talk to the adjutant", "If it's proof the lab knew, Bastion files it. Bastion files everything.");
			scene.Choice(adj, "Give it to Bastion [Bastion: Known]").Needs(RZ_EFaction.MILITARY, 1).Side(RZ_EFaction.MILITARY, RZ_EFaction.INSTITUTE, 10, 10).Decides(OUT_TO_BASTION, false).Says("The adjutant files it under the lab's name. Now the CO has proof the lab knew.", "gave the clinic file to Bastion");
		}

		RZ_Leader chair = RZ_MajorHelp.Top(gm, RZ_EFaction.SURVIVORS);
		if (chair)
		{
			int clerk = scene.Person(RZ_MajorHelp.NearSeat(chair.m_vPos), "the chair's clerk", "Talk to the clerk", "The chair would like to know when it started. So would everyone at supper.");
			scene.Choice(clerk, "Give it to the chair's clerk [Haven: Known]").Needs(RZ_EFaction.SURVIVORS, 1).Side(RZ_EFaction.SURVIVORS, RZ_EFaction.INSTITUTE, 10, 5).Decides(OUT_PUBLIC, false).Says("The clerk reads it at supper. By morning every house knows the date.", "took the clinic file to the Haven");
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool ReadingScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene, RZ_Leader director)
	{
		scene.m_sTitle = "The reading";
		scene.m_sBrief = run.Fill("{director} reads the first page to the authors around a table at the change of shift: \"A paper that starts with that word won't be believed, and then nothing we did in here counts.\"");
		scene.m_sObjective = run.Fill("Help {director} settle the paper's first page at the lab in {lab_town}.");
		scene.m_eTier = RZ_ETier.KNOWN;
		scene.m_iFame = 10;
		scene.m_iFavor = 20;
		scene.m_vPos = gm.GetLabPosition();
		scene.m_fWindow = 5400;
		scene.m_iTimeout = -1;
		vector lab = gm.GetLabPosition();
		int whole = WholeStep(gm, run);
		int table = scene.Person(RZ_MajorHelp.NearSeat(lab), run.Fill("{director} at the table"), "Talk to the director", "Day 1. That's where the paper starts. Tell me why it shouldn't.");
		scene.Choice(table, "Ask for the whole of it").Persuade(RZ_EFaction.INSTITUTE, whole, "labtech,doctor").Side(RZ_EFaction.SURVIVORS, -1, 10, 0).Decides(OUT_WHOLE).Says("The director crosses out Day 1 and writes a date three weeks earlier, then one word: Continue. The director's name goes last.", "had the director write the whole of it").Refused("Then nothing we did in here counts. No.");
		scene.Choice(table, "Stand with the director [Trusted]").Needs(RZ_EFaction.INSTITUTE, 2).Decides(OUT_DAY1).Says("The first page stays as it is. The director doesn't thank you, and writes your name among the authors.", "stood with the director at the reading").Refused("I don't need a stranger to stand with me.");
		run.SetFlag("pages_held", RZ_Majors.Bit(RZ_MajorHelp.World(gm, "proof.pages") == 2));
		bool reads = RZ_MajorHelp.TopHas(gm, RZ_EFaction.INSTITUTE, RZ_ELeaderTrait.IDEALIST) || RZ_MajorHelp.TopHas(gm, RZ_EFaction.INSTITUTE, RZ_ELeaderTrait.MERCIFUL) || ZoneKnows(gm);
		int leverage = OUT_STOVED;
		if (reads)
			leverage = OUT_WHOLE;

		scene.Choice(table, "Put the pages on the table").Way(RZ_EQuestApproach.BETRAY).Only("pages_held").Decides(leverage).Says("Three pages from the site log, in the director's own hand. The room goes very quiet.", "put the director's pages on the table").Refused("You'd need the pages for that.");

		RZ_Leader chair = RZ_MajorHelp.Top(gm, RZ_EFaction.SURVIVORS);
		if (chair && ClerkSits(gm))
		{
			int clerk = scene.Person(RZ_MajorHelp.NearSeat(lab), "the chair's clerk", "Talk to the clerk", "The chair would like to know when it started. That's the only question I was sent with.");
			scene.Choice(clerk, "Take up the chair's question [Haven: Known]").Needs(RZ_EFaction.SURVIVORS, 1).Persuade(RZ_EFaction.INSTITUTE, Math.MaxInt(1, whole - 1), "labtech,doctor").Side(RZ_EFaction.SURVIVORS, RZ_EFaction.INSTITUTE, 5, 5).Decides(OUT_WHOLE).Says("You ask the chair's question out loud. The director answers it, all of it.", "took up the chair's question at the reading").Refused("The director answers the question with a date: Day 1.");
		}

		int stance = RZ_MajorHelp.World(gm, "orders.stance");
		RZ_Leader co = RZ_MajorHelp.Top(gm, RZ_EFaction.MILITARY);
		run.SetFlag("terms", RZ_Majors.Bit(RZ_MajorHelp.World(gm, "authors.terms") == 1 || RZ_MajorHelp.World(gm, "authors.terms") == 2 || RZ_MajorHelp.World(gm, "weeks.bastion") != 0 || RZ_MajorHelp.World(gm, "proof.day_one") == 3 || RZ_MajorHelp.World(gm, "legacy.terms") != 0));
		if (co && stance != 4)
		{
			int adj = scene.Person(RZ_MajorHelp.NearSeat(lab), "the CO's adjutant", "Talk to the adjutant", "Bastion secured this lab on Day 4. The paper could say so, over a signature.");
			scene.Choice(adj, "Have Bastion countersign [Bastion: Known]").Needs(RZ_EFaction.MILITARY, 1).Only("terms").Side(RZ_EFaction.MILITARY, RZ_EFaction.INSTITUTE, 10, 10).Decides(OUT_SEAL, false).Says("The adjutant signs under the director's name. The first page now says Task Force Bastion secured the lab on Day 4.", "had Bastion countersign the paper").Refused("Bastion signs for what Bastion has terms on. Not this.");
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The whole of it: persuade 3; one easier for each of the Zone knowing, the pages read out, the
	//! volunteers named, another stronghold's page out (once), an Idealist or Merciful director; one
	//! harder with a Paranoid or Ruthless director, or if nobody asked about the three weeks.
	protected int WholeStep(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		int d = 3;
		if (ZoneKnows(gm))
			d--;

		if (RZ_MajorHelp.World(gm, "proof.pages") == 3)
			d--;

		if (RZ_MajorHelp.World(gm, "other_two.named") != 0)
			d--;

		d -= RZ_MajorHelp.PageStep(gm, run);
		if (RZ_MajorHelp.TopHas(gm, RZ_EFaction.INSTITUTE, RZ_ELeaderTrait.IDEALIST) || RZ_MajorHelp.TopHas(gm, RZ_EFaction.INSTITUTE, RZ_ELeaderTrait.MERCIFUL))
			d--;

		if (RZ_MajorHelp.TopHas(gm, RZ_EFaction.INSTITUTE, RZ_ELeaderTrait.PARANOID) || RZ_MajorHelp.TopHas(gm, RZ_EFaction.INSTITUTE, RZ_ELeaderTrait.RUTHLESS))
			d++;

		if (RZ_MajorHelp.World(gm, "proof.weeks") == 0)
			d++;

		return Math.ClampInt(d, 1, 4);
	}

	//------------------------------------------------------------------------------------------------
	protected bool ZoneKnows(RZ_GameModeComponent gm)
	{
		return RZ_MajorHelp.World(gm, "proof.day_one") == 4 || RZ_MajorHelp.World(gm, "weeks.public") != 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The chair's clerk sits at the reading if the Haven heard the date or the notebook, HC2 named
	//! the volunteers, H3's lab road, or the Haven read any list this season.
	protected bool ClerkSits(RZ_GameModeComponent gm)
	{
		if (RZ_MajorHelp.World(gm, "proof.day_one") == 4 || RZ_MajorHelp.World(gm, "weeks.public") != 0 || RZ_MajorHelp.World(gm, "other_two.named") != 0)
			return true;

		return RZ_MajorHelp.World(gm, "walk.lab_road") != 0 || RZ_MajorHelp.World(gm, "list.read") != 0;
	}

	//------------------------------------------------------------------------------------------------
	protected bool PaperScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene, RZ_Leader director)
	{
		vector table = RZ_MajorHelp.NorthRoad(gm, 110);
		if (table == vector.Zero)
			table = gm.GetLabPosition();

		scene.m_sTitle = "The paper";
		scene.m_sBrief = run.Fill("{director} reads the paper on the open channel for two minutes, the way the lab speaks for its dead, then seals it in a steel case. The case goes to a table on the north road, just outside the line.");
		scene.m_sObjective = run.Fill("Walk the paper's steel case from the lab to the table on the north road.");
		scene.m_bLate = true;
		scene.m_eTier = RZ_ETier.STRANGER;
		scene.m_iFame = 0;
		scene.m_iFavor = 0;
		scene.m_vPos = table;
		scene.m_fWindow = 5400;
		scene.m_iTimeout = OUT_CARRIED;
		int courier = scene.Person(RZ_MajorHelp.NearSeat(gm.GetLabPosition()), "the courier at the lab gate", "Talk to the courier", "A respirator, a hood, the steel case on a strap. \"I walk it down. You can walk beside me.\"");
		scene.Choice(courier, "Walk the case to the table").Says("The courier sets off down the road with the case. The table is waiting, just outside the line.", "walked with the paper's case");
		int spot = scene.Person(table, "the table on the north road", "Look at the table", "A folding table, a steel case, nobody behind the concrete.");
		scene.Choice(spot, "Set the case on the table").Decides(OUT_CARRIED).Says("The case goes on the table. Nobody takes it. It stays on the Zone's side until the gates open.", "set the paper on the table at the barrier");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Before the stand: the boss's man at the Red Hand's seat (sell the pages; pay, take or lift the
	//! line back) and the records room at the lab (the night swap). Nothing here decides the stage:
	//! the stand's start settles it (SettleBefore).
	protected bool BeforeScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene, RZ_Leader director)
	{
		scene.m_sTitle = "Before the stand";
		scene.m_sBrief = run.Fill("The first page is written, and the batch is close. {director}'s three pages from the site log are still loose, and so is the draft in the records room. Whatever happens to them happens before the stand.");
		scene.m_sObjective = run.Fill("Settle {director}'s loose pages and the draft in the lab's records room before the stand.");
		scene.m_eTier = RZ_ETier.STRANGER;
		scene.m_iFame = 0;
		scene.m_iFavor = 0;
		scene.m_vPos = gm.GetLabPosition();
		scene.m_fWindow = 5400;
		scene.m_iTimeout = -1;
		run.SetFlag("pages_held", RZ_Majors.Bit(RZ_MajorHelp.World(gm, "proof.pages") == 2));
		bool canSwap = RZ_MajorHelp.World(gm, "proof.pages") == 2 || RZ_MajorHelp.World(gm, "weeks.public") != 0 || RZ_MajorHelp.World(gm, "proof.weeks") != 0;
		run.SetFlag("can_swap", RZ_Majors.Bit(canSwap && RZ_MajorHelp.World(gm, "proof.first_page") != 2));

		RZ_Leader boss = RZ_MajorHelp.Top(gm, RZ_EFaction.BANDITS);
		if (boss)
		{
			int man = scene.Person(RZ_MajorHelp.NearSeat(boss.m_vPos), "the boss's man", "Talk to the boss's man", "Paper's paper. The boss buys paper. Two tins a copy, once it's printed.");
			scene.Choice(man, "Sell him the director's pages [Red Hand: Known]").Needs(RZ_EFaction.BANDITS, 1).Only("pages_held").Side(RZ_EFaction.BANDITS, RZ_EFaction.INSTITUTE, 10, 15).Sets("sold").Says("He counts the pages, then counts them again, and writes the lab into the book.", "sold the director's pages to the Red Hand").Refused("You'd need the pages for that.");
			scene.Choice(man, "Pay the lab's line off (40 Favor)").Bribe(RZ_EFaction.BANDITS, 40).Only("sold").Sets("cleared").Says("He crosses the line out and hands the pages back. \"Paid is paid.\"", "paid the lab's line in the book").Refused("There's no line to pay. Yet.");
			scene.Choice(man, "Put the debt in your own name [Red Hand: Trusted]").Needs(RZ_EFaction.BANDITS, 2).Only("sold").Sets("debt_yours").Says("He writes your name where the lab's was. The pages come back across the table.", "took the lab's debt in their own name").Refused("Your name's not good for that much.");
			scene.Choice(man, "Lift the pages from his satchel").Sneak().Way(RZ_EQuestApproach.SNEAK).Only("sold").Sets("lifted").Says("Two lookouts, one satchel, three pages. Nobody saw you, this time.", "lifted the pages from the boss's man").Refused("There's nothing in the satchel worth taking yet.");
		}

		int room = scene.Person(RZ_MajorHelp.NearSeat(gm.GetLabPosition()), "the records room", "Look in the records room", "Three filing cabinets, a draft in a folder marked FIRST PAGE, and the duty officer's chair, empty for now.");
		scene.Choice(room, "Swap the first page in the draft").Sneak().Way(RZ_EQuestApproach.SNEAK).Only("can_swap").Sets("swapped").Says("The new first page says three weeks before Day 1, in a hand close enough. The old one goes in your pocket.", "swapped the first page in the records room").Refused("You'd need to know what the true page says first.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The stand begins (or the act's hard stop): what was done before it is settled. A sold line
	//! nobody cleared, with the book not sent off loud and the paper not already the whole of it, is
	//! read on the Red Hand's channel: Two Tins a Copy. A swap the director finds stands under an
	//! Idealist or Merciful director (the whole of it), else the draft goes back.
	protected void SettleBefore(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.Flag("before_done") != 0)
			return;

		run.SetFlag("before_done", 1);
		bool sold = run.Flag("sold") != 0;
		bool cleared = run.Flag("cleared") != 0 || run.Flag("debt_yours") != 0 || run.Flag("lifted") != 0;
		if (run.Flag("debt_yours") != 0)
			RZ_MajorHelp.SetWorld(gm, "proof.debt_yours", 1);

		int ending = run.Flag("ending", E_M_DAY1);
		if (sold && !cleared)
		{
			RZ_MajorHelp.SetWorld(gm, "proof.pages", 5);
			if (RZ_MajorHelp.World(gm, "book.last") != 4 && ending != E_M_WHOLE)
			{
				TwoTins(gm, run);
				return;
			}
		}
		else if (sold)
		{
			RZ_MajorHelp.SetWorld(gm, "proof.pages", 2);
		}

		if (run.Flag("swapped") == 0 || ending == E_M_WHOLE)
			return;

		RZ_MajorHelp.SetWorld(gm, "proof.first_page", 3);
		bool keeps = RZ_MajorHelp.TopHas(gm, RZ_EFaction.INSTITUTE, RZ_ELeaderTrait.IDEALIST) || RZ_MajorHelp.TopHas(gm, RZ_EFaction.INSTITUTE, RZ_ELeaderTrait.MERCIFUL);
		if (!keeps)
		{
			run.Note("The director found the swapped first page as the stand began, and put the draft back.");
			return;
		}

		run.Note("The director found the swapped first page as the stand began, read it twice, and let it stand.");
		run.SetFlag("ending", E_M_WHOLE);
		ApplyEnding(gm, run, E_M_WHOLE, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Two Tins a Copy: the boss's man reads the pages on the Red Hand's channel as the stand begins.
	protected void TwoTins(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		array<string> sellers = {};
		RZ_MajorHelp.Deciders(run, S_BEFORE, RZ_EQuestApproach.SIDE, sellers);
		RZ_MajorHelp.PayIdents(gm, sellers, RZ_EFaction.BANDITS, 5, 0);
		NudgeMorale(gm, RZ_EFaction.INSTITUTE, -5);
		NudgeMorale(gm, RZ_EFaction.BANDITS, 5);
		SpreadRumour(gm, "Halcyon made this. It's in their own hand. Two tins a copy at any Red Hand camp.", 48);
		Headline(gm, "The Red Hand's market has the lab's first page for sale, two tins a copy. The stickers are still on.");
		if (gm.GetFactionLife())
			gm.GetFactionLife().OpenChannelLine(RZ_EFaction.BANDITS, "Page one of the lab's own log, read out for anyone with a radio. The rest is two tins a copy.");

		run.SetFlag("ending", E_M_TWOTINS);
		RZ_Majors.SeasonStory(RZ_EFaction.INSTITUTE, KEY, E_M_TWOTINS, 3, "There's a copy of the lab's first page in every Red Hand camp.", sellers, false);
		run.Note("The boss's man read the director's pages on the Red Hand's channel as the stand began.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnFinalStandStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.m_iStage != S_BEFORE)
			return;

		SettleBefore(gm, run);
		gm.GetQuestlines().JumpTo(run, S_STAND);
	}

	//------------------------------------------------------------------------------------------------
	//! A restart in the middle of the stand starts it before the questlines load: the end settles it.
	override void OnFinalStandEnd(RZ_GameModeComponent gm, RZ_QuestRun run, bool won)
	{
		OnFinalStandStart(gm, run);
	}

	// ================================================================ How it goes on
	//------------------------------------------------------------------------------------------------
	override int Next(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		switch (stage)
		{
			case S_DAYONE: return AfterDayOne(gm, run, outcome, false);
			case S_WEEKS: return AfterWeeks(gm, run, false);
			case S_AUTHORS: return AfterAuthors(gm, run);
			case S_READING_WAIT: return S_READING;
			case S_READING: return AfterReading(gm, run, outcome, false);
			case S_BEFORE: return S_BEFORE;
			case S_STAND: return AfterStand(gm, run);
			case S_PAPER_WAIT: return S_PAPER;
			case S_PAPER: return AfterPaper(gm, run, false);
		}

		return END;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterDayOne(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome, bool fallback)
	{
		int dayOne = 1;
		string line = "The clinic's file went to the lab sealed. The paper's first page still says Day 1.";
		switch (outcome)
		{
			case OUT_READ_DATE:
			{
				dayOne = 2;
				NudgeMorale(gm, RZ_EFaction.INSTITUTE, -3);
				line = "The director read one date on the open channel at the change of shift. Three weeks before the gates closed. Nothing after it.";
				break;
			}
			case OUT_TO_BASTION:
			{
				dayOne = 3;
				line = "Bastion's adjutant has the clinic's file. The CO has proof the lab knew.";
				break;
			}
			case OUT_PUBLIC:
			{
				dayOne = 4;
				NudgeMorale(gm, RZ_EFaction.INSTITUTE, -3);
				SpreadRumour(gm, "Halcyon made this. Ask the clinic. They had a file on it three weeks before the gates.", 8);
				line = "The clinic's date was read at supper in every Haven house. The Zone knows when it started.";
				break;
			}
			case OUT_SEALED: break;
			default:
			{
				if (!fallback)
					return S_DAYONE;

				// Last season's paper started three weeks early (legacy.records): the lab reads the date itself
				if (RZ_MajorHelp.World(gm, "legacy.records") != 0)
				{
					dayOne = 2;
					line = "The director read the clinic's date on the open channel without being asked. Last season's paper said the same.";
					break;
				}

				line = "The field lead went to the clinic in person and brought the file in. Day 1 stays the lab's.";
				break;
			}
		}

		RZ_MajorHelp.SetWorld(gm, "proof.day_one", dayOne);
		RZ_MajorHelp.SetWorld(gm, "weeks.records", 1);
		if (fallback)
			RZ_Majors.Get().TellDefault(line);

		ActEnd(gm, run, A_DAYONE, dayOne, line);
		return S_WEEKS;
	}

	//------------------------------------------------------------------------------------------------
	//! HC1's ending, copied to proof.weeks (1 in the record, 2 public, 3 Bastion's file; 0 nobody asked)
	//! and proof.pages (1 the stove, 3 read out, 4 Bastion's file).
	protected int AfterWeeks(RZ_GameModeComponent gm, RZ_QuestRun run, bool fallback)
	{
		int weeks = 0;
		int pages = 0;
		if (RZ_MajorHelp.World(gm, "weeks.bastion") != 0)
		{
			weeks = 3;
			pages = 4;
		}
		else if (RZ_MajorHelp.World(gm, "weeks.public") != 0)
		{
			weeks = 2;
			pages = 3;
		}
		else if (gm.GetQuestlines().HasEnded("weeks"))
		{
			weeks = 1;
			pages = 1;
		}

		RZ_MajorHelp.SetWorld(gm, "proof.weeks", weeks);
		if (pages > 0)
			RZ_MajorHelp.SetWorld(gm, "proof.pages", pages);

		string line = "";
		if (fallback && weeks == 0)
			line = "Nobody asked the lab about the first three weeks. The lab took that as an answer.";

		ActEnd(gm, run, A_WEEKS, weeks, line);
		return S_AUTHORS;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterAuthors(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		ActEnd(gm, run, A_AUTHORS, RZ_MajorHelp.World(gm, "authors.terms"), "");
		return S_READING_WAIT;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterReading(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome, bool fallback)
	{
		ReleaseEvening(S_READING);
		int first = 1;
		switch (outcome)
		{
			case OUT_WHOLE: first = 2; break;
			case OUT_SEAL: first = 4; break;
			case OUT_STOVED:
			{
				RZ_MajorHelp.SetWorld(gm, "proof.pages", 1);
				break;
			}
			case OUT_DAY1: break;
			default:
			{
				if (!fallback)
					return S_READING;

				break;
			}
		}

		RZ_MajorHelp.SetWorld(gm, "proof.first_page", first);
		int ending = E_M_DAY1;
		if (first == 2)
			ending = E_M_WHOLE;
		else if (first == 4)
			ending = E_M_SEAL;

		run.SetFlag("ending", ending);
		ApplyEnding(gm, run, ending, fallback);
		ActEnd(gm, run, A_READING, first, EndingLine(ending));
		return S_BEFORE;
	}

	//------------------------------------------------------------------------------------------------
	protected static string EndingLine(int ending)
	{
		switch (ending)
		{
			case E_M_WHOLE: return "The Whole of It. The paper starts three weeks before Day 1, with one word: Continue.";
			case E_M_SEAL: return "Under Seal. The paper carries Bastion's signature.";
			case E_M_TWOTINS: return "Two Tins a Copy. The Red Hand read the lab's first page before the lab did.";
		}

		return "Day 1. The lab's paper starts where the lab says it does.";
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending, bool fallback)
	{
		array<string> deciders = {};
		RZ_MajorHelp.Deciders(run, S_READING, -1, deciders);
		string story = "";
		switch (ending)
		{
			case E_M_WHOLE:
			{
				int hit = -8;
				if (RZ_MajorHelp.World(gm, "weeks.public") != 0)
					hit = -3;

				NudgeMorale(gm, RZ_EFaction.INSTITUTE, hit);
				NudgeMorale(gm, RZ_EFaction.SURVIVORS, 5);
				RZ_MajorHelp.PayIdents(gm, deciders, RZ_EFaction.INSTITUTE, 10, 0);
				RZ_MajorHelp.Relation(gm, RZ_EFaction.SURVIVORS, RZ_EFaction.INSTITUTE, RZ_FactionLife.REL_TRUCE);
				RZ_MajorHelp.OpenPage(gm, "the lab's first page, the whole of it");
				RZ_MajorHelp.SetWorld(gm, "proof.haven_gate", 1);
				if (gm.GetFactionLife())
					gm.GetFactionLife().OpenChannelLine(RZ_EFaction.INSTITUTE, run.Fill("Page one says the first case came to the clinic in {clinic} three weeks before the gates closed, and that the Institute was told. Page two says what it told us. My name is last."));

				story = "The lab's paper is on the table at the barrier. It starts three weeks before Day 1, with one word in the director's hand: Continue. The director's name is last.";
				break;
			}
			case E_M_SEAL:
			{
				NudgeStrength(gm, RZ_EFaction.MILITARY, 3);
				NudgeMorale(gm, RZ_EFaction.INSTITUTE, -5);
				NudgeMorale(gm, RZ_EFaction.SURVIVORS, -3);
				RZ_MajorHelp.PayIdents(gm, deciders, RZ_EFaction.MILITARY, 10, 0);
				story = "The lab's paper is in a Bastion crate at the barrier, signed for. Its first page says the lab was secured on Day 4.";
				break;
			}
			default:
			{
				if (ZoneKnows(gm))
				{
					NudgeMorale(gm, RZ_EFaction.SURVIVORS, -3);
					SpreadRumour(gm, "The lab's paper starts on Day 1. Under 'Origin' it says: under investigation.", 24);
					story = "The lab's paper is on the table at the barrier. Under 'Origin' it says: under investigation.";
				}
				else
				{
					NudgeMorale(gm, RZ_EFaction.INSTITUTE, 5);
					NudgeStrength(gm, RZ_EFaction.INSTITUTE, 2);
					story = "The lab's paper is on the table at the barrier. Nobody in it was sick before Day 1.";
				}

				RZ_MajorHelp.PayIdents(gm, deciders, RZ_EFaction.INSTITUTE, 10, 0);
				break;
			}
		}

		RZ_Majors.SeasonStory(RZ_EFaction.INSTITUTE, KEY, ending, RZ_MajorHelp.World(gm, "proof.first_page"), story, deciders, fallback);
	}

	//------------------------------------------------------------------------------------------------
	//! The stand is the finale's: HC3 reads it; the major only notes how it went.
	protected int AfterStand(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		ActEnd(gm, run, A_STAND, RZ_MajorHelp.World(gm, "authors.ending", -1), "The batch is made. Whoever stood at the lab is on the paper.");
		return S_PAPER_WAIT;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterPaper(RZ_GameModeComponent gm, RZ_QuestRun run, bool fallback)
	{
		RZ_MajorHelp.SetWorld(gm, KEY + ".coda", 1);
		// The hall's "On the paper": whoever set the case down first, then everyone who walked with it
		array<string> first = {};
		RZ_MajorHelp.Deciders(run, S_PAPER, -1, first);
		string firstAuthor = "";
		if (!first.IsEmpty())
			firstAuthor = first[0];

		RZ_MajorHelp.HallNames(run, S_PAPER, "On the paper", firstAuthor);
		string line = "The paper is sealed in its case on a table on the north road, just outside the line. Somewhere a radio ham writes the last number down.";
		if (fallback)
			line = "Nobody carried the case, so the duty officer walked it down alone at dawn. It's on the table at the north road.";

		return Finish(run, run.Flag("ending", E_M_DAY1), line);
	}

	// ================================================================ Defaults
	//------------------------------------------------------------------------------------------------
	override int ActDefault(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_MajorAct act)
	{
		switch (act.m_iId)
		{
			case A_DAYONE: return AfterDayOne(gm, run, -1, true);
			case A_WEEKS: return AfterWeeks(gm, run, true);
			case A_AUTHORS: return AfterAuthors(gm, run);
			case A_READING: return AfterReading(gm, run, -1, true);
			case A_STAND:
			{
				SettleBefore(gm, run);
				// No cure by the wipe: the paper is in the lab's safe, signed and never sent
				if (!gm.GetCure() || !gm.GetCure().IsCured())
				{
					RZ_Majors.SeasonStory(RZ_EFaction.INSTITUTE, KEY, E_M_UNSIGNED, -1, "", null, true);
					return Finish(run, E_M_UNSIGNED, "The paper is in the lab's safe, signed and never sent.");
				}

				return AfterStand(gm, run);
			}
			case A_PAPER: return AfterPaper(gm, run, true);
		}

		return S_PAPER_WAIT;
	}

	// ================================================================ Voice
	//------------------------------------------------------------------------------------------------
	override string BetweenLine(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		return "The director has been redrafting the first page. Three drafts this week. All of them say Day 1.";
	}

	//------------------------------------------------------------------------------------------------
	override string SeasonState(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.Flag("ending") == E_M_TWOTINS)
			return "The first page: two tins a copy at the Red Hand's market.";

		if (RZ_MajorHelp.World(gm, "proof.pages") == 5)
			return "The lab is a line in the Red Hand's book.";

		switch (RZ_MajorHelp.World(gm, "proof.first_page"))
		{
			case 1: return "The first page: Day 1.";
			case 2: return "The first page: the whole of it.";
			case 3: return "The first page: swapped in the records room.";
			case 4: return "The first page: Bastion's countersignature.";
		}

		switch (RZ_MajorHelp.World(gm, "proof.day_one"))
		{
			case 1: return "The lab has the clinic's file. The paper says Day 1.";
			case 2: return "The director read the date on the open channel.";
			case 3: return "Bastion has the clinic's file.";
			case 4: return "The Zone knows the date.";
		}

		return "";
	}
}
