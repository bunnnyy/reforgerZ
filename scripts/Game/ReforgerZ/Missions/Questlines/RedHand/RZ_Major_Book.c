//! The Book Has to Balance: the Red Hand's major (docs/QUESTLINES.md, "The Book Has to Balance").
//! R1 The Book grows into it and keeps its key, "book", its stage missions and every book.* flag:
//! R1's stages 1-2 are act 2 and stages 3-6 act 3, and R1's four endings are act 3's results
//! (AfterInnerEnding). The new acts are scenes (RZ_QM_MajorScene) at Rudi's crossroads and the
//! boss's seat:
//!
//! 1. Send him off (Collapse's second evening, a set piece): the crews send Rudi off loud at his
//!    shutter, and the back page is gone in front of every crew. Ask a crew boss to come, pay a greedy
//!    one, or hold the crossroads through the send-off.
//! 2-3. The torn page and who gets it (R1, as built).
//! 4. Settling day (50%, the first climax, a set piece): the whole book read out; the rival asks who
//!    should hold it. Laugh him off, name him, speak for him, stand with him, tell Bastion where the
//!    crews will be, or offer his crew a Haven door. The coup always fails.
//! 5. The last page (80%, after Halcyon's reading): what the Red Hand is owed when the gates open.
//!    The bill to the lab, stand for nothing, sell it by the page, send it off loud, lift it for
//!    Bastion, or warn the director. Its answer is the major's ending.
//! 6. At the lab road: how the Red Hand turns up at the final stand (book.stand), read as data;
//!    "Sent off loud" holds the fire at the crossroads first.
//! 7. The last line (the epilogue's coda): the boss closes the book at Rudi's crossroads.
//!
//! Simplified (docs/QUESTLINES.md): no forgery, no sale of the page to the rival, the bill and the
//! sale by the page are one choice each (no courier run), and the crews at the crossroads are words.
class RZ_Major_Book : RZ_Quest_TheBook
{
	static const int S_SEND = 100;
	static const int S_SETTLE_WAIT = 110;
	static const int S_SETTLE = 111;
	static const int S_LAST_WAIT = 120;
	static const int S_LAST = 121;
	static const int S_FIRE = 122;
	static const int S_STAND = 130;
	static const int S_CODA_WAIT = 140;
	static const int S_CODA = 141;

	static const int A_SEND = 1;
	static const int A_TORN = 2;
	static const int A_PAGE = 3;
	static const int A_SETTLE = 4;
	static const int A_LAST = 5;
	static const int A_ROAD = 6;
	static const int A_LINE = 7;

	static const int OUT_SENT = 30;			// the send-off went ahead, held or talked up
	static const int OUT_SENT_LOST = 31;	// the hold broke: the crews went home early
	static const int OUT_LAUGHED = 40;		// book.settled 1
	static const int OUT_PURGED = 41;		// 2
	static const int OUT_PULLED = 42;		// 3
	static const int OUT_HAVEN = 43;		// 4
	static const int OUT_RAID = 44;			// 5
	static const int OUT_STOOD = 45;		// 6
	static const int OUT_BROKE_UP = 46;		// the hold was lost: purged, morale -3
	static const int OUT_BILL = 50;			// book.last 1
	static const int OUT_NOTHING = 51;		// 2
	static const int OUT_SOLD = 52;			// 3
	static const int OUT_LOUD = 53;			// 4
	static const int OUT_FILE = 54;			// 5
	static const int OUT_TAKES = 55;		// 6
	static const int OUT_FIRE = 60;
	static const int OUT_LINE = 70;

	// The page after act 3 (run flag "page")
	static const int P_IN_BOOK = 1;
	static const int P_KEPT = 2;
	static const int P_BASTION = 3;
	static const int P_BURNT = 4;
	static const int P_OUT = 5;

	// The major's endings (10 and up: R1's own are 0-3)
	static const int E_M_BALANCES = 10;
	static const int E_M_SOLD = 11;
	static const int E_M_LOUD = 12;
	static const int E_M_FILE = 13;
	static const int E_M_BROKEN = 14;

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		super.Define();
		Info("book", "The Book Has to Balance", RZ_EFaction.BANDITS);
		Major();

		RZ_QuestStage send = MissionStage(S_SEND, "Send him off", "{boss} calls every crew to Rudi's shutter at {crossroads} to send him off loud: horns, music, shots in the air. Then the book is read over him. Bring the crew bosses. Hold the road if the noise brings company.");
		send.GiveUp(4, "");

		RZ_QuestStage settleWait = WaitStage(S_SETTLE_WAIT, "Settling day", "The page is settled, one way or another. {boss} says the whole book gets read out before the cure, at {crossroads}, and it has to balance.");
		settleWait.Wait(120);

		RZ_QuestStage settle = MissionStage(S_SETTLE, "Settling day", "Settling day at {crossroads}: the whole book read out loud at dusk. {rival} means to use the reading to ask who should hold the book.");
		settle.GiveUp(4, "");

		RZ_QuestStage lastWait = WaitStage(S_LAST_WAIT, "The last page", "Settling day is done. {boss} is counting the book again, and waiting for the lab to say what the cure is worth.");
		lastWait.Wait(120);

		RZ_QuestStage last = MissionStage(S_LAST, "The last page", "{boss} opens the book at Rudi's tally and asks what the last page should say: what the Red Hand is owed when the gates open.");
		last.GiveUp(4, "");

		RZ_QuestStage fire = MissionStage(S_FIRE, "Sent off loud", "The book goes on a fire at {crossroads} at dusk, the way the Red Hand sends off its dead. Hold the road while it burns.");
		fire.GiveUp(4, "");

		RZ_QuestStage stand = WaitStage(S_STAND, "At the lab road", "The last page is written. The Red Hand meets the night the cure is made the way it says.");
		stand.Wait(0);

		RZ_QuestStage codaWait = WaitStage(S_CODA_WAIT, "The last line", "The gates are opening. {boss} will close the book at Rudi's crossroads, paid or not.");
		codaWait.Wait(0);

		RZ_QuestStage coda = MissionStage(S_CODA, "The last line", "At {crossroads} at dusk {boss} closes the book: the last line, then every line still open, read out with its name.");
		coda.GiveUp(4, "");

		Act(A_SEND, "Send him off").Stage(S_SEND).AnchorCure(50).Need(1).PatienceEvenings(2).SetPiece(16, 21);
		Act(A_TORN, "The torn page").Stage(S_WHO).Stage(S_FIND).AnchorCure(50).Need(1).LastCall("Somebody's selling a page at a barn. Not for long.");
		Act(A_PAGE, "Who gets the page").Stage(S_READ).Stage(S_PAGE).Stage(S_THIEF).Stage(S_HEARS).AnchorCure(50).Need(3).LastCall("The boss's men are going to the pit tonight, page or no page.");
		Act(A_SETTLE, "Settling day").Stage(S_SETTLE_WAIT).Stage(S_SETTLE).Opens(50).AnchorCure(60).Need(1).PatienceEvenings(2).SetPiece(16, 21).Climax().CostsMorale().LastCall("{boss} reads the book at dusk tomorrow, with or without anyone standing up for it.");
		Act(A_LAST, "The last page").Stage(S_LAST_WAIT).Stage(S_LAST).Stage(S_FIRE).Opens(80).Need(1).PatienceEvenings(2).SetPiece(16, 21).Rank(2).LastCall("{boss} writes the last page tomorrow at dusk. Whoever isn't there doesn't get a say.");
		Act(A_ROAD, "At the lab road").Stage(S_STAND).PastGate();
		Act(A_LINE, "The last line").Stage(S_CODA_WAIT).Stage(S_CODA).AnchorWipe().Need(0).SetPiece(17, 22).Rank(0);
	}

	//------------------------------------------------------------------------------------------------
	override int FirstStage()
	{
		if (!RZ_Majors.IsOn())
			return S_WHO;

		return S_SEND;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsMajor()
	{
		return m_bMajor && RZ_Majors.IsOn();
	}

	//------------------------------------------------------------------------------------------------
	//! Collapse's second evening at the earliest, with the boss in the chair and a crossroads; picks
	//! the rival. With the majors off, R1 as built.
	override bool CanStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (!RZ_Majors.IsOn())
			return super.CanStart(gm, run);

		if (!RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN) || !gm.GetDirector() || !gm.GetLeaders())
			return false;

		if (gm.GetDirector().GetStage() < RZ_EOutbreakStage.COLLAPSE)
			return false;

		// Collapse's second evening: an evening has passed since Collapse began
		if (gm.GetDirector().GetStage() == RZ_EOutbreakStage.COLLAPSE && gm.GetDirector().GetStageMinutes() < RZ_SeasonClock.DayMinutes())
			return false;

		RZ_Leader boss = RZ_MajorHelp.Top(gm, RZ_EFaction.BANDITS);
		vector cross = RZ_MajorHelp.Crossroads();
		if (!boss || cross == vector.Zero)
			return false;

		RZ_RedHandQuest.Keep(run, "boss", boss);
		RZ_RedHandQuest.KeepPos(run, "cross", cross);
		run.SetText("crossroads", "Rudi's crossroads outside " + boss.m_sPlace);
		PickRival(gm, run);
		run.SetPlace(cross, run.Text("crossroads"));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The rival: the Rogue crew boss, else the least loyal Independent one, else the boss's second.
	protected void PickRival(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		array<RZ_Leader> crews = {};
		gm.GetLeaders().GetFactionLeaders(RZ_EFaction.BANDITS, crews);
		RZ_Leader best;
		foreach (RZ_Leader l : crews)
		{
			if (l.IsStronghold() || !l.IsPresent() || l.m_eLoyalty == RZ_ELoyalty.LOYAL)
				continue;

			if (!best || l.m_iScore < best.m_iScore)
				best = l;
		}

		if (best)
		{
			RZ_RedHandQuest.Keep(run, "rival", best);
			RZ_MajorHelp.SetWorld(gm, "book.rival", best.m_iId + 1);
			return;
		}

		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		array<string> names = {};
		RZ_RedHandQuest.MenNames(rng, 1, names);
		string second = "the boss's second";
		if (!names.IsEmpty())
			second = names[0] + " " + RZ_NoteText.Surname(rng);

		run.SetText("rival", second);
		run.SetText("rival_short", second);
		run.SetText("rival_place", run.Text("boss_place"));
		RZ_MajorHelp.SetWorld(gm, "book.rival", 0);
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (!IsMajor())
		{
			super.OnBegin(gm, run);
			return;
		}

		Headline(gm, run.Fill("The Red Hand is sending Rudi off at {crossroads}. Every crew has been told to come, and to bring something loud."), false);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsReady(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (!IsMajor())
			return super.IsReady(gm, run, stage);

		switch (stage)
		{
			case S_SEND: return Free(gm, run, S_SEND);
			case S_WHO:
			{
				// Act 2 picks the page's names and the thief (R1's setup) the first time it's ready
				if (run.Flag("page_setup") == 1)
					return true;

				if (!super.CanStart(gm, run))
					return false;

				run.SetFlag("page_setup", 1);
				return true;
			}
			case S_SETTLE_WAIT: return RZ_Majors.Reached(run, 50) && RZ_MajorHelp.PlayedSince(run, 2);
			case S_SETTLE: return Free(gm, run, S_SETTLE);
			case S_LAST_WAIT:
				return RZ_MajorHelp.PlayedSince(run, 2) && RZ_Majors.ClimaxDone("proof") && RZ_Majors.Reached(run, 80);
			case S_LAST: return Free(gm, run, S_LAST);
			case S_FIRE: return Free(gm, run, S_FIRE);
			case S_STAND:
			{
				RZ_CureResearch cure = gm.GetCure();
				return cure && (cure.IsFinalStand() || cure.IsCured());
			}
			case S_CODA_WAIT: return RZ_MajorHelp.CodaDue(gm, "proof");
			case S_CODA: return Free(gm, run, S_CODA);
		}

		return super.IsReady(gm, run, stage);
	}

	//------------------------------------------------------------------------------------------------
	//! A set piece holds the evening (the set-piece calendar); one that has waited twice runs anyway.
	protected bool Free(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		RZ_SetPieceCalendar cal = RZ_SetPieceCalendar.Get();
		RZ_MajorAct act = ActOf(stage);
		if (!cal || !act || !RZ_Majors.InWindow(act))
			return false;

		return RZ_MajorHelp.HoldEvening(run, string.Format("book.%1", stage), RZ_RedHandQuest.PosOf(run, "cross"));
	}

	//------------------------------------------------------------------------------------------------
	override RZ_Mission MakeMission(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (stage >= S_SEND)
			return new RZ_QM_MajorScene();

		return super.MakeMission(gm, run, stage);
	}

	// ================================================================ Scenes
	//------------------------------------------------------------------------------------------------
	override bool BuildScene(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene)
	{
		RZ_Leader boss = RZ_MajorHelp.Top(gm, RZ_EFaction.BANDITS);
		vector cross = RZ_RedHandQuest.PosOf(run, "cross");
		if (!boss || cross == vector.Zero)
			return false;

		scene.m_iGiver = RZ_EFaction.BANDITS;
		scene.m_vPos = cross;
		switch (stage)
		{
			case S_SEND: return SendScene(gm, run, scene, cross);
			case S_SETTLE: return SettleScene(gm, run, scene, cross);
			case S_LAST: return LastScene(gm, run, scene, boss);
			case S_FIRE: return FireScene(gm, run, scene, cross);
			case S_CODA: return LineScene(gm, run, scene, cross);
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool SendScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene, vector cross)
	{
		scene.m_sTitle = "Send him off";
		scene.m_sBrief = run.Fill("Rudi was never sent off. Tonight he is, at his shutter at {crossroads}: horns, music, shots in the air, and the book read over him. Bring the crew bosses. The noise will bring company.");
		scene.m_eTier = RZ_ETier.STRANGER;
		scene.m_iTimeout = OUT_SENT;
		scene.SetHold(cross, 240, OUT_SENT, OUT_SENT_LOST);
		int keeper = scene.Person(RZ_MajorHelp.NearSeat(cross), "the book-keeper at the shutter", "Talk to the book-keeper", run.Fill("\"Horns at dusk, then the book. Every crew boss is supposed to be here. {rival_short} sent word: too much on this week.\""));
		scene.Choice(keeper, "Ask a crew boss to come [Known]").Persuade(RZ_EFaction.BANDITS, 1, "excon").Sets("crews", 1).Says("A crew boss comes, with two lorries and a horn that only plays one note. The book-keeper writes the name down.", "brought a crew boss to the send-off").Refused("\"They'll come when the boss asks them. You're not the boss.\"");
		scene.Choice(keeper, "Pay a greedy one to come (15 Favor)").Bribe(RZ_EFaction.BANDITS, 15).Sets("crews_paid", 1).Says("The money goes in an inside pocket. A crew boss who wasn't coming turns up early and stands at the front.", "paid a crew boss to come");
		scene.Choice(keeper, "Hold the crossroads through it").Hold().Says("The horns start. Somewhere past the hedges, things that were lying still get up.", "held the crossroads for the send-off");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool SettleScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene, vector cross)
	{
		scene.m_sTitle = "Settling day";
		scene.m_sBrief = run.Fill("The whole book read out at {crossroads} before the cure, loud on purpose. {rival} will use the reading to ask who should hold it. It fails either way. How it fails is up to whoever is standing there.");
		scene.m_eTier = RZ_ETier.KNOWN;
		scene.m_iFame = 8;
		scene.m_iFavor = 15;
		scene.m_iTimeout = -1;
		scene.SetHold(cross, 240, OUT_LAUGHED, OUT_BROKE_UP);
		int page = run.Flag("page");
		if (page == P_IN_BOOK)
			run.SetFlag("page_in", 1);

		int keeper = scene.Person(RZ_MajorHelp.NearSeat(cross), "the book-keeper at the lorry", "Talk to the book-keeper", run.Fill("\"Every line, front to back, then the back page. {boss} reads the first one. Then {rival_short} gets to talk, and everyone gets to watch.\""));
		scene.Choice(keeper, "Hold the crossroads while it's read").Hold().Says("The reading starts. The horns go first, and the hedges answer.", "held the crossroads for the reading");
		scene.Choice(keeper, "Laugh it off with the crews [Known]").Needs(RZ_EFaction.BANDITS, 1).Decides(OUT_LAUGHED).Says("You laugh first, and the crews laugh after you. {rival_short} stops talking halfway through a word.", "laughed the rival off").Refused("\"Laugh at a crew boss? You'd want to be known here first.\"");
		int rival = scene.Person(RZ_MajorHelp.NearSeat(cross), run.Fill("{rival_short} by the lorries"), "Talk to the rival", run.Fill("\"A book is only worth what the one holding it can keep. Ask the boss where the back page went.\""));
		scene.Choice(rival, "Name the rival to the crews [Trusted]").Persuade(RZ_EFaction.BANDITS, NameStep(gm, run), "excon").Only("page_in").Decides(OUT_PURGED).Says("You say who paid the thief, and how much. Nobody laughs. The boss's men walk {rival_short} off the road.", "named the rival").Refused("\"Name me? With what? The page isn't even in the book.\"");
		scene.Choice(rival, "Speak for the rival [Trusted]").Persuade(RZ_EFaction.BANDITS, NameStep(gm, run), "excon").Decides(OUT_PULLED).Says("You say {rival_short} only asked out loud what everyone was thinking. The boss writes that name under yours, with a debt beside it.", "spoke for the rival").Refused("The crews don't hear you over the horns.");
		scene.Choice(rival, "Stand with the rival (+15 Infamy)").Way(RZ_EQuestApproach.BETRAY).Side(-1, RZ_EFaction.BANDITS, 0, 15).Decides(OUT_STOOD, false).Says("You stand next to {rival_short}. The boss's crew comes up the road for both of you. It was never going to work. It still costs.", "stood with the rival");
		scene.Choice(rival, "Offer the rival's crew a Haven door [Haven: Known]").Needs(RZ_EFaction.SURVIVORS, 1).Side(RZ_EFaction.SURVIVORS, RZ_EFaction.BANDITS, 10, 10).Decides(OUT_HAVEN, false).Says("{rival_short} looks at the lorries, then at the hands painted on them. By morning three of that crew are painting them out.", "walked the rival's crew to a Haven door");
		RZ_Leader co = RZ_MajorHelp.Top(gm, RZ_EFaction.MILITARY);
		if (co)
		{
			int duty = scene.Person(RZ_MajorHelp.NearSeat(co.m_vPos), "Bastion's duty officer at " + co.m_sPlace, "Talk to the duty officer", "\"If you've got something to report, report it. If you've got something to sell, there's a queue.\"");
			scene.Choice(duty, "Say where the crews will be [Known]").Needs(RZ_EFaction.MILITARY, NameRaidRank(gm)).Side(RZ_EFaction.MILITARY, RZ_EFaction.BANDITS, 10, 15).Decides(OUT_RAID, false).Says("The duty officer writes the grid down and the time. That night Bastion hits a camp with half its crew away at a reading.", "told Bastion where the crews would be");
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Speaking up at settling day: Trusted (persuade 2); one easier with two crew bosses at the
	//! send-off, a crew name, R3's well.stood or R2's cargo.purge; one harder after R3's well.done.
	protected int NameStep(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		int d = 2;
		if (RZ_MajorHelp.World(gm, "book.sendoff") >= 2 || RZ_MajorHelp.World(gm, "well.stood") != 0 || RZ_MajorHelp.World(gm, "cargo.purge") != 0 || !run.Text("crewname").IsEmpty())
			d--;

		if (RZ_MajorHelp.World(gm, "well.done") != 0)
			d++;

		return Math.ClampInt(d, 1, 4);
	}

	//------------------------------------------------------------------------------------------------
	//! Telling Bastion where the crews will be: Known with Bastion.
	protected int NameRaidRank(RZ_GameModeComponent gm)
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	protected bool LastScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene, RZ_Leader boss)
	{
		scene.m_sTitle = "The last page";
		scene.m_sBrief = run.Fill("{boss} opens the book at Rudi's tally. \"When the gates open, a toll is a lorry parked across a road. So what are we owed?\"");
		scene.m_eTier = RZ_ETier.KNOWN;
		scene.m_iFame = 10;
		scene.m_iFavor = 20;
		scene.m_vPos = boss.m_vPos;
		bool whole = run.Flag("whole") == 1;
		if (whole)
			run.SetFlag("is_whole", 1);

		int loud = 2;
		if (boss.HasTrait(RZ_ELeaderTrait.PARANOID) || boss.HasTrait(RZ_ELeaderTrait.PRAGMATIST) || !whole || RZ_MajorHelp.World(gm, "book.thief") == T_PLACE || RZ_MajorHelp.World(gm, "book.thief") == T_PAID || RZ_MajorHelp.World(gm, "book.thief") == T_FREED)
			loud = 1;

		int seat = scene.Person(RZ_MajorHelp.NearSeat(boss.m_vPos), run.Fill("{boss} at the book"), "Talk to the boss", run.Fill("\"First page, ten litres, on tick. Paid. Every line after it, paid or owed. The last page is what the cure owes us.\""));
		scene.Choice(seat, "Stand for nothing [Trusted]").Persuade(RZ_EFaction.BANDITS, 2, "excon").Decides(OUT_NOTHING).Says("\"A debt's a debt. We hold the road, and the cure owes us.\" The boss writes it on the last page in capitals.", "talked the boss into standing for nothing").Refused("\"Nothing? You'd want to be trusted here before you sell me nothing.\"");
		scene.Choice(seat, "Sell it by the page").Way(RZ_EQuestApproach.TRADE).Only("is_whole").Decides(OUT_SOLD).Says("Three lines, three couriers, three strongholds buying their own names back. The boss counts it twice.", "sold the book by the page").Refused("\"Sell what? Half of it's ash or in somebody's pocket.\"");
		scene.Choice(seat, "Say send it off loud").Persuade(RZ_EFaction.BANDITS, loud, "excon").Decides(OUT_LOUD).Says("\"When the gates open, that book is a confession, and its first page is your own.\" The boss looks at the first page for a long time.", "talked the boss into burning the book").Refused("\"Burn the book? Then what am I?\"");
		scene.Choice(seat, "Ask to keep the first page out [Trusted]").Needs(RZ_EFaction.BANDITS, 2).Sets("world:book.first_page").Says("The boss closes a hand over the first page. \"That one stays mine.\"", "asked the boss to keep the first page out");

		int table = scene.Person(RZ_MajorHelp.NearSeat(boss.m_vPos), "the boss's table at supper", "Look at the table", "The book on the table, a mug on the book. The crews eat at the other end, with their backs to it.");
		scene.Choice(table, "Lift the book for Bastion (+25 Infamy)").Sneak().Needs(RZ_EFaction.MILITARY, 1).Side(RZ_EFaction.MILITARY, RZ_EFaction.BANDITS, 10, 15).Decides(OUT_FILE, false).Says("The book goes under your coat. By the next call it's in a cupboard at Bastion's stronghold.", "took the Red Hand's book to Bastion");

		RZ_Leader director = RZ_MajorHelp.Top(gm, RZ_EFaction.INSTITUTE);
		if (director)
		{
			int lab = scene.Person(RZ_MajorHelp.NearSeat(director.m_vPos), "the lab's duty officer", "Talk to the duty officer", "\"The director doesn't meet couriers. Leave it with me.\"");
			scene.Choice(lab, "Hand over the boss's bill").Needs(RZ_EFaction.INSTITUTE, 1).Only("is_whole").Decides(OUT_BILL).Says("The bill forgives the live ones and the cases, for two doses and the lab road at the stand. The duty officer signs for it.", "carried the bill to the lab").Refused("\"The director won't take a bill from a book with pages missing.\"");
			scene.Choice(lab, "Warn the director (+10 Infamy)").Side(RZ_EFaction.INSTITUTE, RZ_EFaction.BANDITS, 10, 10).Decides(OUT_TAKES, false).Says("You tell the duty officer what's coming. One more guard on the gate tonight. The boss will take what's owed some other way.", "warned the director about the bill");
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool FireScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene, vector cross)
	{
		scene.m_sTitle = "Sent off loud";
		scene.m_sBrief = run.Fill("The book goes on a fire at {crossroads} at dusk, the way the Red Hand sends off its dead. Hold the road while it burns.");
		scene.m_eTier = RZ_ETier.STRANGER;
		scene.m_iTimeout = OUT_FIRE;
		scene.SetHold(cross, 180, OUT_FIRE, OUT_FIRE);
		int fire = scene.Person(RZ_MajorHelp.NearSeat(cross), "the fire at the crossroads", "Look at the fire", "Diesel on old pallets. The book on top, open at the first page. The horns are already going.");
		scene.Choice(fire, "Hold the road while it burns").Hold().Says("The pages go up one at a time. The noise carries a long way.", "held the fire at the crossroads");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool LineScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene, vector cross)
	{
		scene.m_sTitle = "The last line";
		scene.m_sBrief = run.Fill("At {crossroads} {boss} closes the book: the last line, then every line still open, paid or not. Nobody is ever out of the book, and tonight everyone hears it.");
		scene.m_bLate = true;
		scene.m_eTier = RZ_ETier.STRANGER;
		scene.m_iFame = 0;
		scene.m_iFavor = 0;
		scene.m_iTimeout = OUT_LINE;
		scene.SetHold(cross, 150, OUT_LINE, OUT_LINE);
		int lorry = scene.Person(RZ_MajorHelp.NearSeat(cross), run.Fill("{boss} at the lorry"), "Talk to the boss", LastLineText(gm, run));
		scene.Choice(lorry, "Stand at the lorry for your line").Decides(OUT_LINE).Says("The boss finds your line with one finger, reads it out with your name, and writes the word beside it.", "stood at the lorry for their own line");
		scene.Choice(lorry, "Ask about the first line [Trusted]").Needs(RZ_EFaction.BANDITS, 2).Says(FirstLineText(gm), "asked about the first line").Refused("The boss doesn't look up.");
		scene.Choice(lorry, "Hold the crossroads for the dead").Hold().Says("The horns go for the season's dead. Their names are read before the book is closed.", "held the crossroads for the dead");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected string LastLineText(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		switch (RZ_MajorHelp.World(gm, "book.last"))
		{
			case 4: return "The lorry is empty. The horns go anyway. \"Nothing written down.\"";
			case 5: return "\"The book's in a cupboard at Bastion's. I'll read you the open ones from memory. I've got a good memory.\"";
		}

		if (run.Flag("whole") != 1)
			return "The boss reads up to the torn page, and stops.";

		return "The book is nailed open at the last page on the side of the lorry. The boss reads the last line first.";
	}

	//------------------------------------------------------------------------------------------------
	protected string FirstLineText(RZ_GameModeComponent gm)
	{
		if (RZ_MajorHelp.World(gm, "book.first_page") != 0)
			return "\"No.\" The boss closes the book.";

		if (RZ_MajorHelp.World(gm, "cook.line_paid") != 0)
			return "The cook stands at the lorry. The boss reads her line on the first page. \"Paid. On the first page, again.\"";

		return "\"Day 2. Ten litres, on tick.\" Beside it: Paid. \"He let people pay later. You can't run a road like that.\"";
	}

	// ================================================================ How it goes on
	//------------------------------------------------------------------------------------------------
	override int Next(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		if (!IsMajor())
			return super.Next(gm, run, stage, outcome);

		switch (stage)
		{
			case S_SEND: return AfterSend(gm, run, outcome);
			case S_SETTLE_WAIT: return S_SETTLE;
			case S_SETTLE: return AfterSettle(gm, run, outcome);
			case S_LAST_WAIT: return S_LAST;
			case S_LAST: return AfterLast(gm, run, outcome);
			case S_FIRE: return AfterFire(gm, run, outcome);
			case S_STAND: return AfterStand(gm, run);
			case S_CODA_WAIT: return S_CODA;
			case S_CODA: return LastLine(gm, run, false);
		}

		return super.Next(gm, run, stage, outcome);
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterSend(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		ReleaseEvening(S_SEND);
		if (outcome != OUT_SENT && outcome != OUT_SENT_LOST)
			return S_SEND;

		int crews = run.Flag("crews") + run.Flag("crews_paid");
		if (run.Flag("crews") == 0 && RZ_MajorHelp.TopHas(gm, RZ_EFaction.BANDITS, RZ_ELeaderTrait.VETERAN))
			crews++;

		if (outcome == OUT_SENT && run.AnyDid(RZ_EQuestApproach.FORCE, S_SEND))
		{
			if (crews >= 2)
				NudgeMorale(gm, RZ_EFaction.BANDITS, 3);

			crews++;
		}

		RZ_MajorHelp.SetWorld(gm, "book.sendoff", Math.ClampInt(crews, 0, 3));
		string line = "Rudi went off loud at his shutter. Then the book-keeper turned to the back page in front of every crew, and it wasn't there. {rival_short} laughed.";
		if (outcome == OUT_SENT_LOST)
			line = "The send-off broke up when the hedges came alive, and the crews went home early. In the morning the back page of the book was gone. {rival_short} laughed about that too.";

		ActEnd(gm, run, A_SEND, crews, line);
		return S_WHO;
	}

	//------------------------------------------------------------------------------------------------
	//! R1's endings are act 3's results: the page's place decides what settling day can be.
	override int AfterInnerEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
		if (!IsMajor() || ending >= E_M_BALANCES)
			return END;

		OnEnding(gm, run, ending);
		int page = P_BURNT;
		switch (ending)
		{
			case E_BALANCES: page = P_IN_BOOK; break;
			case E_HELD: page = P_KEPT; break;
			case E_BASTION: page = P_BASTION; break;
		}

		run.SetFlag("page", page);
		if (page == P_IN_BOOK)
			run.SetFlag("whole", 1);

		ActEnd(gm, run, A_PAGE, ending, run.m_sEndText);
		return S_SETTLE_WAIT;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterSettle(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		ReleaseEvening(S_SETTLE);
		if (outcome < OUT_LAUGHED || outcome > OUT_BROKE_UP)
			return S_SETTLE;

		return Settled(gm, run, outcome, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Settling day's result (book.settled), live or as its default.
	protected int Settled(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome, bool fallback)
	{
		int settled = 1;
		string line = "{rival_short} asked who should hold the book, and the crossroads laughed. The boss read the next line.";
		RZ_Leader rival = RZ_RedHandQuest.Same(gm, run, "rival");
		switch (outcome)
		{
			case OUT_PURGED:
			case OUT_BROKE_UP:
			{
				settled = 2;
				line = "The boss's men walked {rival_short} off the road before the reading was done. The crew boss's chair went to somebody quieter.";
				if (outcome == OUT_BROKE_UP)
					NudgeMorale(gm, RZ_EFaction.BANDITS, -3);

				if (rival && RZ_MajorBudget.Get() && RZ_MajorBudget.Get().TakeSeat("book", RZ_EFaction.BANDITS, false))
					ReplaceLeader(gm, rival.m_iId, run.Fill("The Red Hand has a new crew boss at {rival_place}. Nobody asks where the last one went."), run);

				break;
			}
			case OUT_PULLED:
			{
				settled = 3;
				line = "Somebody spoke for {rival_short} at the crossroads, and the boss let it stand. The chair stays filled, and the debt is written down.";
				if (rival)
					RZ_RedHandQuest.SwingLoyalty(gm, rival, RZ_RedHandQuest.SWING);

				break;
			}
			case OUT_HAVEN:
			{
				settled = 4;
				line = "{rival_short} and three of the crew painted the hands off their lorries and walked to a Haven door. A new crew boss sat down the same night.";
				int house = -1;
				if (gm.GetSafehouses() && rival)
					house = gm.GetSafehouses().FindNearest(rival.m_vPos);

				if (house >= 0)
					ChangePopulation(gm, house, 3);

				break;
			}
			case OUT_RAID:
			{
				settled = 5;
				line = "While every crew boss was at the crossroads, Bastion hit {rival_short}'s half-empty camp. Bastion never shells a gathering. It doesn't have to.";
				NudgeStrength(gm, RZ_EFaction.BANDITS, -3);
				break;
			}
			case OUT_STOOD:
			{
				settled = 6;
				line = "Somebody stood with {rival_short} at the crossroads. The boss's crew came up the road. The coup failed, the way they do.";
				if (rival)
					RZ_RedHandQuest.SwingLoyalty(gm, rival, -RZ_RedHandQuest.SWING);

				break;
			}
		}

		RZ_MajorHelp.SetWorld(gm, "book.settled", settled);
		if (run.Flag("whole") == 1)
		{
			RZ_MajorHelp.OpenPage(gm, "the Red Hand's book, read whole at settling day");
			Headline(gm, "The Red Hand read its whole book out at Rudi's crossroads, back page and all.");
		}

		if (fallback)
			RZ_Majors.Get().TellDefault(run.Fill(line));

		ActEnd(gm, run, A_SETTLE, settled, line);
		return S_LAST_WAIT;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterLast(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		ReleaseEvening(S_LAST);
		if (outcome < OUT_BILL || outcome > OUT_TAKES)
			return S_LAST;

		return LastPage(gm, run, outcome - OUT_BILL + 1, false);
	}

	//------------------------------------------------------------------------------------------------
	//! The last page's answer (book.last 1-6) is the major's ending: its world part now; the stand and
	//! the coda follow.
	protected int LastPage(RZ_GameModeComponent gm, RZ_QuestRun run, int last, bool fallback)
	{
		// A broken book can only burn, or the boss takes what's owed
		if (run.Flag("whole") != 1 && last != 4 && last != 6 && last != 5)
			last = 6;

		// The director refuses the bill while Bastion guards the lab
		if (last == 1 && (RZ_MajorHelp.World(gm, "orders.stance") == 1 || RZ_MajorHelp.World(gm, "orders.stance") == 3))
			last = 2;

		RZ_MajorHelp.SetWorld(gm, "book.last", last);
		int ending = EndingFor(last);
		run.SetFlag("ending", ending);
		ApplyEnding(gm, run, ending, last, fallback);
		ActEnd(gm, run, A_LAST, last, EndingLine(ending));
		if (last == 4)
			return S_FIRE;

		return S_STAND;
	}

	//------------------------------------------------------------------------------------------------
	protected static int EndingFor(int last)
	{
		switch (last)
		{
			case 1:
			case 2: return E_M_BALANCES;
			case 3: return E_M_SOLD;
			case 4: return E_M_LOUD;
			case 5: return E_M_FILE;
		}

		return E_M_BROKEN;
	}

	//------------------------------------------------------------------------------------------------
	protected static string EndingLine(int ending)
	{
		switch (ending)
		{
			case E_M_BALANCES: return "The book balances. The last line is the lab's.";
			case E_M_SOLD: return "Sold by the page. Three strongholds bought their own lines back.";
			case E_M_LOUD: return "Sent off loud. The book goes on a fire at the crossroads, and every debt in it with it.";
			case E_M_FILE: return "Bastion's file. The Red Hand's book is in a cupboard at Bastion's stronghold.";
		}

		return "The book doesn't balance. The boss takes what's owed.";
	}

	//------------------------------------------------------------------------------------------------
	//! The ending's world part, inside the season budget, and the season's story. Pays only the
	//! deciders (never on a default).
	protected void ApplyEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending, int last, bool fallback)
	{
		array<string> deciders = {};
		RZ_MajorHelp.Deciders(run, S_LAST, -1, deciders);
		RZ_MajorHelp.Deciders(run, S_SETTLE, -1, deciders);
		RZ_MajorBudget budget = RZ_MajorBudget.Get();
		string story = "";
		switch (ending)
		{
			case E_M_BALANCES:
			{
				NudgeMorale(gm, RZ_EFaction.BANDITS, 7);
				NudgeStrength(gm, RZ_EFaction.BANDITS, 3);
				RZ_MajorHelp.PayIdents(gm, deciders, RZ_EFaction.BANDITS, 15, 0);
				if (budget)
					budget.SetContractWeight("RZ_Job_DebtCollection", 1.2);

				if (last == 2)
					RZ_MajorHelp.Relation(gm, RZ_EFaction.INSTITUTE, RZ_EFaction.BANDITS, RZ_FactionLife.REL_TRUCE);

				Headline(gm, "The Red Hand read its book out at Rudi's crossroads. It balanced. The last line is the lab's.");
				story = "When the gates opened, the boss of the Red Hand walked out with a book under one arm, and everyone in it watched.";
				break;
			}
			case E_M_SOLD:
			{
				NudgeMorale(gm, RZ_EFaction.BANDITS, 3);
				NudgeStrength(gm, RZ_EFaction.BANDITS, 3);
				RZ_MajorHelp.PayIdents(gm, deciders, RZ_EFaction.BANDITS, 15, 0);
				Headline(gm, "Three strongholds bought their own lines back from the Red Hand this week. Nobody asked what the Red Hand kept.");
				story = "When the gates opened, everyone in the Zone had bought their own page back from the Red Hand.";
				break;
			}
			case E_M_LOUD:
			{
				NudgeMorale(gm, RZ_EFaction.BANDITS, -5);
				RZ_MajorHelp.SetWorld(gm, "book.debts_dead", 1);
				RZ_MajorHelp.OpenPage(gm, "the Red Hand's book, sent off loud");
				RZ_MajorHelp.Relation(gm, RZ_EFaction.SURVIVORS, RZ_EFaction.BANDITS, RZ_FactionLife.REL_TRUCE);
				if (budget)
				{
					budget.SetContractWeight("RZ_Mission_ProtectionRacket", 0.8);
					budget.SetContractWeight("RZ_Mission_QuietArrangement", 0.8);
				}

				if (gm.GetFactionLife())
					gm.GetFactionLife().RecordMoment("the night the book burned");

				story = "When the gates opened, the Red Hand walked out with nothing written down. Some of them went home.";
				if (RZ_MajorHelp.World(gm, "book.first_page") != 0)
					story += " All but the first page. The boss kept that.";

				break;
			}
			case E_M_FILE:
			{
				NudgeMorale(gm, RZ_EFaction.BANDITS, -7);
				NudgeStrength(gm, RZ_EFaction.BANDITS, -2);
				NudgeMorale(gm, RZ_EFaction.MILITARY, 5);
				RZ_MajorHelp.Relation(gm, RZ_EFaction.MILITARY, RZ_EFaction.BANDITS, RZ_FactionLife.REL_FEUD);
				if (budget)
					budget.SetContractWeight("RZ_Mission_BustOut", 1.2);

				Headline(gm, "Command, Bastion. Attached, the Red Hand's book. Over. A long hiss. Bastion out.");
				story = "When the gates opened, Bastion handed over a ledger with half the Zone in it. The Red Hand's names were on top.";
				break;
			}
			default:
			{
				NudgeMorale(gm, RZ_EFaction.BANDITS, -5);
				if (budget)
					budget.SetContractWeight("RZ_Mission_TheRat", 1.2);

				Headline(gm, "The Red Hand's book doesn't balance. The boss is asking who has the page.");
				story = "When the gates opened, the Red Hand's book still didn't balance. Somebody had the page.";
				break;
			}
		}

		RZ_Majors.SeasonStory(RZ_EFaction.BANDITS, "book", ending, last, story, deciders, fallback);
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterFire(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		ReleaseEvening(S_FIRE);
		array<string> holders = {};
		RZ_MajorHelp.Deciders(run, S_FIRE, RZ_EQuestApproach.FORCE, holders);
		RZ_MajorHelp.PayIdents(gm, holders, RZ_EFaction.BANDITS, 15, 0);
		RZ_MajorHelp.PayIdents(gm, holders, RZ_EFaction.SURVIVORS, 10, 0);
		NudgeMorale(gm, RZ_EFaction.BANDITS, 5);
		Headline(gm, "The Red Hand sent its book off loud at Rudi's crossroads last night. Half the infected in the valley came to see. Nobody owes anybody now.");
		run.Note("The book burned at the crossroads, first page last.");
		return S_STAND;
	}

	//------------------------------------------------------------------------------------------------
	//! The final stand (the finale owns it): how the Red Hand turned up, from the last page.
	protected int AfterStand(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		int last = RZ_MajorHelp.World(gm, "book.last");
		int stand = 4;
		if (last == 1 || last == 2)
		{
			stand = 1;
			if (RZ_MajorHelp.World(gm, "orders.stance") == 3)
				run.Note("Bastion's guns were on the lab road. The boss wouldn't stand beside them, and the crews stayed home.");
			else
				run.Note("The crews held the lab road through the stand, as the last page said.");
		}
		else if (last == 5 || last == 6)
		{
			// With no finale, Take the Lab runs as built: a stolen batch counts as the room taken
			stand = 3;
			if (gm.GetCure() && gm.GetCure().GetBatchesStolen() > run.Flag("stolen_before"))
				stand = 2;
		}

		RZ_MajorHelp.SetWorld(gm, "book.stand", stand);
		ActEnd(gm, run, A_ROAD, stand, "");
		return S_CODA_WAIT;
	}

	//------------------------------------------------------------------------------------------------
	override void OnStageBegin(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		super.OnStageBegin(gm, run, stage);
		if (stage == S_STAND && gm.GetCure())
			run.SetFlag("stolen_before", gm.GetCure().GetBatchesStolen());

		if (stage >= S_SEND && RZ_RedHandQuest.HasPos(run, "cross"))
			run.SetPlace(RZ_RedHandQuest.PosOf(run, "cross"), run.Text("crossroads"));
	}

	//------------------------------------------------------------------------------------------------
	//! The coda closes the major on the ending the last page set.
	protected int LastLine(RZ_GameModeComponent gm, RZ_QuestRun run, bool fallback)
	{
		ReleaseEvening(S_CODA);
		RZ_MajorHelp.SetWorld(gm, "book.coda", 1);
		int ending = run.Flag("ending", E_M_BROKEN);
		string line = "The boss closed the book at Rudi's crossroads, paid or not.";
		if (fallback)
			line = "The boss closed the book at Rudi's crossroads with nobody there to hear it. The lad on the kitchen chair by the lorry reads you your line if you ask.";

		return Finish(run, ending, line);
	}

	//------------------------------------------------------------------------------------------------
	protected void ReleaseEvening(int stage)
	{
		RZ_SetPieceCalendar cal = RZ_SetPieceCalendar.Get();
		if (cal)
			cal.Release(string.Format("book.%1", stage));
	}

	//------------------------------------------------------------------------------------------------
	//! The major's endings were applied at the last page; R1's own endings (inside act 3) as built.
	override void OnEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
		if (ending >= E_M_BALANCES)
			return;

		super.OnEnding(gm, run, ending);
	}

	// ================================================================ Defaults
	//------------------------------------------------------------------------------------------------
	override int ActDefault(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_MajorAct act)
	{
		switch (act.m_iId)
		{
			case A_SEND:
			{
				ReleaseEvening(S_SEND);
				RZ_MajorHelp.SetWorld(gm, "book.sendoff", 0);
				RZ_Majors.Get().TellDefault(run.Fill("The crews sent Rudi off at his shutter without anybody's help. At the reading the back page of the book was missing."));
				ActEnd(gm, run, A_SEND, 0, "");
				return S_WHO;
			}
			case A_TORN:
			{
				RZ_MajorHelp.SetWorld(gm, "book.burnt", 1);
				RZ_MajorHelp.SetWorld(gm, "book.thief", T_RAN);
				run.SetFlag("page", P_BURNT);
				RZ_Majors.Get().TellDefault("Whoever tore the page out of the Red Hand's book burned it himself and ran.");
				ActEnd(gm, run, A_TORN, 0, "");
				ActEnd(gm, run, A_PAGE, E_BURNT, "");
				return S_SETTLE_WAIT;
			}
			case A_PAGE:
			{
				run.SetFlag("page", P_OUT);
				RZ_MajorHelp.SetWorld(gm, "book.thief", T_PIT);
				RZ_Majors.Get().TellDefault("The Red Hand's page is still out. The boss's men put the thief in the pit anyway.");
				ActEnd(gm, run, A_PAGE, -1, "");
				return S_SETTLE_WAIT;
			}
			case A_SETTLE:
			{
				ReleaseEvening(S_SETTLE);
				int outcome = OUT_LAUGHED;
				if (run.Flag("page") != P_IN_BOOK && Math.RandomFloat01() < 0.5)
					outcome = OUT_PURGED;

				return Settled(gm, run, outcome, true);
			}
			case A_LAST:
			{
				ReleaseEvening(S_LAST);
				ReleaseEvening(S_FIRE);
				if (run.m_iStage == S_FIRE)
					return S_STAND;

				return LastPage(gm, run, BossDecides(gm, run), true);
			}
			case A_ROAD:
				return AfterStand(gm, run);
			case A_LINE:
				return LastLine(gm, run, true);
		}

		return S_STAND;
	}

	//------------------------------------------------------------------------------------------------
	//! The last page's default: the boss's traits (read only; the boss never changes).
	protected int BossDecides(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		bool whole = run.Flag("whole") == 1;
		RZ_Leader boss = RZ_MajorHelp.Top(gm, RZ_EFaction.BANDITS);
		if (!boss)
			return 6;

		if (boss.HasTrait(RZ_ELeaderTrait.PARANOID) || boss.HasTrait(RZ_ELeaderTrait.PRAGMATIST))
			return 4;

		if (boss.HasTrait(RZ_ELeaderTrait.GREEDY) && whole)
			return 3;

		if (boss.HasTrait(RZ_ELeaderTrait.RUTHLESS) || boss.HasTrait(RZ_ELeaderTrait.GREEDY) || !whole)
			return 6;

		return 1;
	}

	// ================================================================ Voice
	//------------------------------------------------------------------------------------------------
	override string BetweenLine(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.m_iStage == S_LAST_WAIT)
			return "\"The boss has been counting the book again. Every night now.\"";

		return "\"Settling day's coming. Every crew boss is going through their own line twice.\"";
	}

	//------------------------------------------------------------------------------------------------
	override string SeasonState(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		switch (run.Flag("page"))
		{
			case P_IN_BOOK: return "The page is back in the book.";
			case P_KEPT: return "The page is in somebody's pocket.";
			case P_BASTION: return "The page is on Bastion's desk.";
			case P_BURNT: return "The page is ash.";
			case P_OUT: return "The page is still out.";
		}

		return "";
	}
}
