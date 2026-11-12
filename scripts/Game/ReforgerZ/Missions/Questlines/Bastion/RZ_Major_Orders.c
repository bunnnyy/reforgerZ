//! Await Relief: Bastion's major (docs/QUESTLINES.md, "Await Relief"). B1 grows into it and keeps
//! its key, "orders", its stage missions and its flags (orders.stove, .read, .mocked, .haven, .copy,
//! .tape): B1's five stages are acts I-III, and its four endings are act III's results
//! (AfterInnerEnding). The new acts are scenes at the radio by the CO's seat:
//!
//! I-III. Reply received, Who reads it, The reckoning (B1, as built).
//! IV. Effective Strength (Overrun, a session after act III): the CO's confession, then the true
//!     count from the three posts nearest the stronghold: ask the commander (Known; Independent:
//!     persuade 2), read the roster at night (sneak), or buy the quartermaster's copy (15 Favor).
//!     The true number is {count} (orders.count); every post not counted stays padded.
//! V. The muster (60%, a set piece at the nightly call, after settling day): keep the call (The
//!    Call Stands), read it to them and hold the stronghold's gate (All Stations), answer it
//!    yourself (Bastion, Command), or hold the relay for the Red Hand (Expended). The stance
//!    (orders.stance 1-4) is the ending.
//! VI. The last call and the last roll (the epilogue's coda): the CO's last call, then Sedlak walks
//!     the tags to the north road and reads the roll 150 m short of the barrier.
//!
//! Simplified (docs/QUESTLINES.md): the copies aren't collected before "keep the call", the padding
//! isn't reported or covered post by post, the disguise trick and the curfew walk aren't offered,
//! and Bastion's side doesn't fight for the relay.
class RZ_Major_Orders : RZ_Quest_AwaitRelief
{
	static const int S_COUNT_WAIT = 100;
	static const int S_COUNT = 101;
	static const int S_MUSTER_WAIT = 110;
	static const int S_MUSTER = 111;
	static const int S_CALL_WAIT = 130;
	static const int S_CALL = 131;

	static const int A_I = 1;
	static const int A_II = 2;
	static const int A_III = 3;
	static const int A_IV = 4;
	static const int A_V = 5;
	static const int A_VI = 6;

	static const int OUT_COUNTED = 30;
	static const int OUT_CALL = 40;			// stance 1
	static const int OUT_ALL = 41;			// 2
	static const int OUT_COMMAND = 42;		// 3
	static const int OUT_EXPENDED = 43;		// 4
	static const int OUT_ROLL = 50;

	static const int E_M_CALL = 10;			// + stance - 1
	static const int POSTS = 3;
	static const int TOLD = 38;				// what the CO tells Command every night

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		super.Define();
		Info(KEY, "Await Relief", RZ_EFaction.MILITARY);
		Major();

		RZ_QuestStage countWait = WaitStage(S_COUNT_WAIT, "Effective Strength", "The reckoning is over. At {stronghold} the CO has started reading the numbers to Command more slowly.");
		countWait.Wait(120);

		RZ_QuestStage count = MissionStage(S_COUNT, "Effective Strength", "{co} at the radio: \"Every night I tell Command thirty-eight. Before those gates open, I want to know what I'd be telling them if I told the truth.\" Get the true count from the posts.");
		count.GiveUp(4, "");

		RZ_QuestStage musterWait = WaitStage(S_MUSTER_WAIT, "The muster", "The count is in, true or padded. {co} is going to call every post onto the net.");
		musterWait.Wait(120);

		RZ_QuestStage muster = MissionStage(S_MUSTER, "The muster", "{co} on the net: \"Everyone on this net has heard a word by now. Tonight I read the numbers to Command, or I read them something else. Tell me which.\"");
		muster.GiveUp(4, "");

		RZ_QuestStage callWait = WaitStage(S_CALL_WAIT, "The last call", "The gates are opening. Bastion has one more call to make, and one more roll.");
		callWait.Wait(0);

		RZ_QuestStage call = MissionStage(S_CALL, "The last call", "The last call from {stronghold} at 20:30. At dawn Lieutenant Sedlak walks the box of tags to the north road and reads the roll.");
		call.GiveUp(4, "");

		Act(A_I, "Reply received").Stage(S_TAPE).Stage(S_RUNNER).AnchorCure(60).Need(2).LastCall("A stronghold patrol walks the runner's road tomorrow. Whatever he was carrying goes in the stove.");
		Act(A_II, "Who reads it").Stage(S_WHO).AnchorCure(60).Need(1).LastCall("The page goes in the stove tomorrow, read or not.");
		Act(A_III, "The reckoning").Stage(S_WAIT).Stage(S_RECKON).AnchorCure(60).Need(2).LastCall("Whatever the page started settles itself tomorrow, one way or another.");
		Act(A_IV, "Effective Strength").Stage(S_COUNT_WAIT).Stage(S_COUNT).AnchorCure(60).Need(3).Rank(2).LastCall("The CO reads the count at tomorrow's call, with or without the posts' numbers.");
		Act(A_V, "The muster").Stage(S_MUSTER_WAIT).Stage(S_MUSTER).Opens(60).Need(1).PatienceEvenings(2).SetPiece(19, 22.5).Climax().LastCall("The CO decides at tomorrow's call what Bastion is. Alone, if nobody comes.");
		Act(A_VI, "The last call and the last roll").Stage(S_CALL_WAIT).Stage(S_CALL).AnchorWipe().Need(0).Rank(0);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsMajor()
	{
		return m_bMajor && RZ_Majors.IsOn();
	}

	//------------------------------------------------------------------------------------------------
	//! Collapse, after a nightly call; with the majors off, B1 as built.
	override bool CanStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (!RZ_Majors.IsOn())
			return super.CanStart(gm, run);

		if (!gm.GetDirector() || gm.GetDirector().GetStage() < RZ_EOutbreakStage.COLLAPSE || !RZ_BastionQuest.IsAfterCall())
			return false;

		if (!super.CanStart(gm, run))
			return false;

		RZ_WorldFacts.Get().CanonText("bastion.orders");
		Headline(gm, "Three Bastion posts logged a reply after the call last night. The stronghold's order on the secure net: log nothing.", false);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! B1's commander, else (in the major) any outpost commander: someone gives "reply received".
	override protected RZ_Leader PickCommander(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_Leader l = super.PickCommander(gm, run);
		if (l || !IsMajor() || !gm.GetLeaders())
			return l;

		array<RZ_Leader> list = {};
		gm.GetLeaders().GetFactionLeaders(RZ_EFaction.MILITARY, list);
		foreach (RZ_Leader c : list)
		{
			if (!c.IsStronghold() && c.IsPresent() && c.m_iHeldBy < 0)
				return c;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsReady(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (!IsMajor())
			return super.IsReady(gm, run, stage);

		switch (stage)
		{
			case S_COUNT_WAIT:
				return gm.GetDirector().GetStage() >= RZ_EOutbreakStage.OVERRUN && RZ_MajorHelp.PlayedSince(run, 2);
			case S_COUNT: return RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY) != null;
			case S_MUSTER_WAIT:
				return RZ_MajorHelp.PlayedSince(run, 2) && RZ_Majors.ClimaxDone("book") && RZ_Majors.Reached(run, 60);
			case S_MUSTER: return Free(gm, run, S_MUSTER);
			case S_CALL_WAIT: return RZ_MajorHelp.CodaDue(gm, "every_name");
			case S_CALL: return RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY) != null;
		}

		return super.IsReady(gm, run, stage);
	}

	//------------------------------------------------------------------------------------------------
	protected bool Free(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		RZ_SetPieceCalendar cal = RZ_SetPieceCalendar.Get();
		RZ_MajorAct act = ActOf(stage);
		RZ_Leader co = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY);
		if (!cal || !act || !co || !RZ_Majors.InWindow(act))
			return false;

		return RZ_MajorHelp.HoldEvening(run, string.Format("orders.%1", stage), co.m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	protected void ReleaseEvening(int stage)
	{
		RZ_SetPieceCalendar cal = RZ_SetPieceCalendar.Get();
		if (cal)
			cal.Release(string.Format("orders.%1", stage));
	}

	//------------------------------------------------------------------------------------------------
	override RZ_Mission MakeMission(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (stage >= S_COUNT_WAIT)
			return new RZ_QM_MajorScene();

		return super.MakeMission(gm, run, stage);
	}

	// ================================================================ Scenes
	//------------------------------------------------------------------------------------------------
	override bool BuildScene(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene)
	{
		RZ_Leader co = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY);
		if (!co)
			return false;

		scene.m_iGiver = RZ_EFaction.MILITARY;
		scene.m_vPos = co.m_vPos;
		switch (stage)
		{
			case S_COUNT: return CountScene(gm, run, scene, co);
			case S_MUSTER: return MusterScene(gm, run, scene, co);
			case S_CALL: return CallScene(gm, run, scene, co);
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CountScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene, RZ_Leader co)
	{
		scene.m_sTitle = "Effective Strength";
		scene.m_sBrief = run.Fill("{co} at the radio, before the call: \"That wasn't the first copy to reach my stove. They could carry 'nobody is coming'. Tell them nobody ever meant to come, and they'll stop asking anyone for anything.\" Then the ask: the true count, from the posts.");
		scene.m_eTier = RZ_ETier.KNOWN;
		scene.m_iFame = 6;
		scene.m_iFavor = 15;
		scene.m_fWindow = 5400;
		scene.m_iTimeout = OUT_COUNTED;
		int radio = scene.Person(RZ_MajorHelp.NearSeat(co.m_vPos), "the radio by the CO's seat", "Listen at the radio", "\"Priority remains Institute site and personnel.\" The CO reads it off the page from memory. \"That's the line the runner's page lost. Now you know why Command wanted the lab.\"");
		scene.Choice(radio, "Hand the CO the count").Decides(OUT_COUNTED).Says("The CO reads your numbers twice and writes a smaller one under thirty-eight.", "handed the CO the true count");

		array<RZ_Leader> posts = {};
		NearestPosts(gm, co, posts);
		foreach (int i, RZ_Leader p : posts)
		{
			string key = "post" + i.ToString();
			run.SetFlag(key + "_id", p.m_iId);
			int persuade = 1;
			if (p.m_eLoyalty != RZ_ELoyalty.LOYAL)
				persuade = 2;

			int spot = scene.Person(RZ_MajorHelp.NearSeat(p.m_vPos), "the board at " + p.m_sPlace, "Talk to the duty sergeant", string.Format("The duty board at %1. A roster, a stores ledger, and a number at the bottom that's the same as last week's.", p.m_sPlace));
			scene.Choice(spot, "Ask for the true count").Persuade(RZ_EFaction.MILITARY, persuade, "police,soldier").Sets(key, 1).Says("The commander shows you the board. The struck-off names are under a strip of tape. The number underneath is smaller.", "got a post's true count").Refused("\"The count is the count. Ask the stronghold.\"");
			scene.Choice(spot, "Read the roster at night").Sneak().Sets(key, 1).Says("The roster and the ledger, by torchlight. Two rifles off the inventory, one man struck off who isn't dead.", "read a post's roster at night");
			scene.Choice(spot, "Buy the quartermaster's copy (15 Favor)").Bribe(RZ_EFaction.MILITARY, 15).Sets(key, 1).Says("The quartermaster's own copy, folded small. He doesn't say which number is his.", "bought a post's real ledger");
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The posts nearest the stronghold, up to three.
	protected void NearestPosts(RZ_GameModeComponent gm, RZ_Leader co, notnull array<RZ_Leader> result)
	{
		array<RZ_Leader> all = {};
		gm.GetLeaders().GetFactionLeaders(RZ_EFaction.MILITARY, all);
		for (int pick = 0; pick < POSTS; pick++)
		{
			RZ_Leader best;
			float bestDist = float.MAX;
			foreach (RZ_Leader l : all)
			{
				if (l.IsStronghold() || !l.IsPresent() || result.Contains(l))
					continue;

				float d = RZ_Util.DistXZ(l.m_vPos, co.m_vPos);
				if (d < bestDist)
				{
					bestDist = d;
					best = l;
				}
			}

			if (!best)
				return;

			result.Insert(best);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool MusterScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene, RZ_Leader co)
	{
		scene.m_sTitle = "The muster";
		scene.m_sBrief = run.Fill("Every post on the secure net at the nightly call. {co}: \"Tonight I read the numbers to Command, or I read them something else. Tell me which.\" Command never answers, in any version.");
		scene.m_eTier = RZ_ETier.KNOWN;
		scene.m_iFame = 10;
		scene.m_iFavor = 20;
		scene.m_fWindow = 5400;
		scene.m_iTimeout = -1;
		int radio = scene.Person(RZ_MajorHelp.NearSeat(co.m_vPos), "the radio by the CO's seat", "Talk to the CO", run.Fill("\"Effective strength {count}. That's the true number now. What I read after it is up to the people in this room.\""));
		scene.Choice(radio, "Say keep the call [Known]").Needs(RZ_EFaction.MILITARY, 1).Decides(OUT_CALL).Says("\"Command, Bastion. Effective strength {count}. Requesting relief. Over.\" The hiss after it sounds like every other night's.", "told the CO to keep the call").Refused("\"You'd want to be known on this net first.\"");
		scene.Choice(radio, "Say read it to them").Persuade(RZ_EFaction.MILITARY, ReadStep(gm, run), "soldier").HoldAt(co.m_vPos, 180, OUT_ALL, OUT_CALL).Says("The CO picks up the handset. \"All stations. This is Bastion. I have something to read you.\" The loudspeakers carry it, and the noise carries further.", "talked the CO into reading the orders").Refused("\"Read them what? That nobody meant to come? Not tonight.\"");
		scene.Choice(radio, "Say answer it yourself").Persuade(RZ_EFaction.MILITARY, AnswerStep(gm, run), "soldier").Decides(OUT_COMMAND).Says("\"Bastion, Command. Hold the line.\" The CO says it to the net in Command's voice, and every post answers.", "had the stronghold answer as Command").Refused("\"Command doesn't take advice from the queue.\"");

		vector relay = RelayPos(gm, run, co);
		int crew = scene.Person(RZ_MajorHelp.NearSeat(relay), "the crew's radio man at the relay", "Talk to the radio man", "\"At half eight every post hears a word over the CO's call. You keep them off me for three minutes, the word's free.\"");
		// Bastion's side: break the relay hold (+10 Bastion Fame), the call goes out as the CO says it
		scene.Choice(crew, "Take the relay back for Bastion [Bastion: Known]").Way(RZ_EQuestApproach.FORCE).Needs(RZ_EFaction.MILITARY, 1).Only("!relay_taken").Side(RZ_EFaction.MILITARY, RZ_EFaction.BANDITS, 10, 5).Sets("relay_taken").Says("You pull the crew's lead out of the relay and sit on it. Whatever the CO says tonight, nobody talks over it.", "took the relay back for Bastion").Refused("The relay's Bastion's already.");

		// The rations bribe: the quartermaster's tins go round before the call (All Stations' morale hit halved)
		int qm = scene.Person(RZ_MajorHelp.NearSeat(co.m_vPos), "the quartermaster", "Talk to the quartermaster", "\"If the CO reads them something hard tonight, they'd take it better on a full stomach. I've tins. You've Favor.\"");
		scene.Choice(qm, "Pay for rations before the call (30 Favor)").Bribe(RZ_EFaction.MILITARY, 30).Only("!rations").Sets("rations").Says("The tins go round the posts before half eight. Whatever comes over the net, they'll hear it fed.", "paid for the rations before the muster").Refused("\"They've eaten. Twice would be suspicious.\"");

		scene.Choice(crew, "Hold the relay for the crew (+25 Infamy)").Way(RZ_EQuestApproach.BETRAY).Needs(RZ_EFaction.BANDITS, RedHandRank(gm)).Only("!relay_taken").Side(RZ_EFaction.BANDITS, RZ_EFaction.MILITARY, 10, 15).HoldAt(relay, 180, OUT_EXPENDED, OUT_CALL).Says("The radio man keys up. Behind you, something on the hill hears the generator.", "held the relay for the Red Hand").Refused("\"The boss says you're not one of ours. Not for this.\"");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The relay on the hill from act I if B1 kept it, else a rise near the stronghold.
	protected vector RelayPos(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_Leader co)
	{
		if (run.HasFlag("relay_x"))
			return RZ_Util.GroundPos(Vector(run.Flag("relay_x"), 0, run.Flag("relay_z")));

		return RZ_Mission.LandPointNear(co.m_vPos, 700, 1500);
	}

	//------------------------------------------------------------------------------------------------
	//! The Red Hand's betray: Trusted with the Red Hand, or Known with orders.rh_knows, orders.mocked
	//! or pilot.redhand; always Trusted after Bastion raided the rival's camp (book.settled 5).
	protected int RedHandRank(RZ_GameModeComponent gm)
	{
		if (RZ_MajorHelp.World(gm, "book.settled") == 5)
			return 2;

		if (RZ_MajorHelp.World(gm, KEY + ".rh_knows") != 0 || RZ_MajorHelp.World(gm, KEY + ".mocked") != 0 || RZ_MajorHelp.World(gm, "pilot.redhand") != 0)
			return 1;

		return 2;
	}

	//------------------------------------------------------------------------------------------------
	//! "Read it to them": persuade 3; one easier for each of the true count, proof (orders.copy, B3's
	//! pilot.line), a Merciful or Idealist CO, HC1's weeks.public and, once, another page in the open;
	//! one harder with a Paranoid CO. Never easier than 2.
	protected int ReadStep(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		int d = 3;
		if (RZ_MajorHelp.World(gm, KEY + ".padded", POSTS) < POSTS)
			d--;

		if (RZ_MajorHelp.World(gm, KEY + ".copy") != 0 || RZ_MajorHelp.World(gm, "pilot.line") != 0)
			d--;

		if (RZ_MajorHelp.TopHas(gm, RZ_EFaction.MILITARY, RZ_ELeaderTrait.MERCIFUL) || RZ_MajorHelp.TopHas(gm, RZ_EFaction.MILITARY, RZ_ELeaderTrait.IDEALIST))
			d--;

		if (RZ_MajorHelp.World(gm, "weeks.public") != 0)
			d--;

		d -= RZ_MajorHelp.PageStep(gm, run);
		if (RZ_MajorHelp.TopHas(gm, RZ_EFaction.MILITARY, RZ_ELeaderTrait.PARANOID))
			d++;

		return Math.ClampInt(d, 2, 4);
	}

	//------------------------------------------------------------------------------------------------
	//! "Answer it yourself": persuade 2; Known is enough after padded posts were reported, with the
	//! Haven counted (haven.rule 3, B2's Counted), proof the lab knew, or a Ruthless or Paranoid CO.
	protected int AnswerStep(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		bool easy = RZ_MajorHelp.World(gm, "haven.rule") == 3 || RZ_MajorHelp.World(gm, "census.counted") != 0;
		easy = easy || RZ_MajorHelp.World(gm, "proof.day_one") == 3 || RZ_MajorHelp.World(gm, "weeks.bastion") != 0;
		easy = easy || RZ_MajorHelp.TopHas(gm, RZ_EFaction.MILITARY, RZ_ELeaderTrait.RUTHLESS) || RZ_MajorHelp.TopHas(gm, RZ_EFaction.MILITARY, RZ_ELeaderTrait.PARANOID);
		if (easy)
			return 1;

		return 2;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CallScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene, RZ_Leader co)
	{
		scene.m_sTitle = "The last call";
		scene.m_sBrief = run.Fill("The last nightly call of the season from {stronghold}, then the walk. Lieutenant Sedlak keeps the roster board, and leads the walk with the box of tags. The CO stays at the radio: \"Somebody has to be on the net.\"");
		scene.m_bLate = true;
		scene.m_eTier = RZ_ETier.STRANGER;
		scene.m_iFame = 0;
		scene.m_iFavor = 0;
		scene.m_fWindow = 7200;
		scene.m_iTimeout = OUT_ROLL;
		int radio = scene.Person(RZ_MajorHelp.NearSeat(co.m_vPos), "the radio by the CO's seat", "Talk to the operator", "\"Effective strength, civilians sheltered, the new names on the nail. Then the stance's line. Read it if you want it read.\"");
		scene.Choice(radio, "Read the last call").Says(LastCallLine(gm), "read Bastion's last call");
		vector halt = RZ_MajorHelp.NorthRoad(gm, 150);
		if (halt != vector.Zero)
		{
			int sedlak = scene.Person(halt, "Lieutenant Sedlak at the halt", "Talk to Sedlak", "\"We stop here. Any closer and the voice turns us back. I read the roll from here.\"");
			// Sedlak's box: the tags brought home to the nail in the week (Bring Them Home)
			string box = "Sedlak reads the roster out loud. The living answer. For the dead, everyone at the halt answers together. For a struck-off man, nobody does.";
			int tags = SedlakBox();
			if (tags > 0)
				box += string.Format(" %1 sets of tags in the box, brought home this week. Those names get a Here too.", tags);

			scene.Choice(sedlak, "Stand at the halt for the roll").Decides(OUT_ROLL).Says(box, "stood at the barrier for the last roll");
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion's names brought home to the nail in the epilogue week (Sedlak's box at the roll).
	protected int SedlakBox()
	{
		RZ_Epilogue epi = RZ_Epilogue.Get();
		if (!epi || !epi.IsBuilt())
			return 0;

		int n = 0;
		foreach (RZ_EpiName name : epi.Data().names)
		{
			if (name.state == 2 && name.faction == RZ_EFaction.MILITARY)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected string LastCallLine(RZ_GameModeComponent gm)
	{
		switch (RZ_MajorHelp.World(gm, KEY + ".stance"))
		{
			case 2: return "\"All stations, all houses, this is Bastion. Civilians first. Bastion out.\"";
			case 3: return "\"Bastion, Command. Hold the line.\"";
			case 4: return "\"Effective strength {count}. Bastion out.\"";
		}

		// The finale's clause: under Bastion's route, the captain and the four R numbers
		if (gm.GetFinale() && gm.GetFinale().Controller() == RZ_CureFinale.BASTION)
			return "\"Command, Bastion. Stores held for collection, R1 to R4, released. The captain signed. Requesting relief. Over.\"";

		return "\"Command, Bastion. Holding at the barrier. Requesting relief. Over.\"";
	}

	// ================================================================ How it goes on
	//------------------------------------------------------------------------------------------------
	override int Next(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		if (!IsMajor())
			return super.Next(gm, run, stage, outcome);

		switch (stage)
		{
			case S_COUNT_WAIT: return S_COUNT;
			case S_COUNT: return Counted(gm, run, false);
			case S_MUSTER_WAIT: return S_MUSTER;
			case S_MUSTER:
			{
				ReleaseEvening(S_MUSTER);
				if (outcome < OUT_CALL || outcome > OUT_EXPENDED)
					return S_MUSTER;

				return Muster(gm, run, outcome - OUT_CALL + 1, false);
			}
			case S_CALL_WAIT: return S_CALL;
			case S_CALL: return LastRoll(gm, run, false);
		}

		return super.Next(gm, run, stage, outcome);
	}

	//------------------------------------------------------------------------------------------------
	//! B1's endings are act III's results (orders.reckoning 1-4), and the line goes on.
	override int AfterInnerEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
		if (!IsMajor() || ending >= E_M_CALL)
			return END;

		OnEnding(gm, run, ending);
		RZ_MajorHelp.SetWorld(gm, KEY + ".reckoning", ending + 1);
		ActEnd(gm, run, A_III, ending + 1, run.m_sEndText);
		return S_COUNT_WAIT;
	}

	//------------------------------------------------------------------------------------------------
	//! The count: every post counted lowers the number the CO reads; the rest stay padded.
	protected int Counted(RZ_GameModeComponent gm, RZ_QuestRun run, bool fallback)
	{
		int counted = 0;
		for (int i = 0; i < POSTS; i++)
		{
			if (run.Flag("post" + i.ToString()) != 0)
				counted++;
		}

		int padded = POSTS - counted;
		int count = TOLD - 2 * counted;
		if (RZ_MajorHelp.World(gm, "book.bastion") != 0 && counted < POSTS)
			count -= 2;

		RZ_MajorHelp.SetWorld(gm, KEY + ".count", count);
		RZ_MajorHelp.SetWorld(gm, KEY + ".padded", padded);
		run.SetText("count", count.ToString());
		string line = string.Format("The count is in. %1 posts gave the true number. The CO wrote %2 under thirty-eight.", counted, count);
		if (fallback)
		{
			line = "The count stayed padded. The CO reads thirty-eight to Command, as every night.";
			if (RZ_MajorHelp.TopHas(gm, RZ_EFaction.MILITARY, RZ_ELeaderTrait.PARANOID) || RZ_MajorHelp.TopHas(gm, RZ_EFaction.MILITARY, RZ_ELeaderTrait.RUTHLESS))
				line += " The worst post has been written up.";

			RZ_Majors.Get().TellDefault(line);
		}

		ActEnd(gm, run, A_IV, counted, line);
		return S_MUSTER_WAIT;
	}

	//------------------------------------------------------------------------------------------------
	//! The muster's stance (orders.stance 1-4) is the major's ending: its world part now.
	protected int Muster(RZ_GameModeComponent gm, RZ_QuestRun run, int stance, bool fallback)
	{
		// Expended needs the Red Hand's relay held; under a fallback the CO decides alone
		RZ_MajorHelp.SetWorld(gm, KEY + ".stance", stance);
		if (run.Text("count").IsEmpty())
			run.SetText("count", RZ_MajorHelp.World(gm, KEY + ".count", TOLD).ToString());

		array<string> deciders = {};
		RZ_MajorHelp.Deciders(run, S_MUSTER, -1, deciders);
		RZ_MajorBudget budget = RZ_MajorBudget.Get();
		string story = "";
		string line = "";
		switch (stance)
		{
			case 1:
			{
				NudgeMorale(gm, RZ_EFaction.MILITARY, 6);
				NudgeStrength(gm, RZ_EFaction.MILITARY, 3);
				RZ_MajorHelp.PayIdents(gm, deciders, RZ_EFaction.MILITARY, 20, 0);
				line = run.Fill("Bastion read the numbers to Command last night, as every night. Effective strength {count}. No reply.");
				story = "At the north road, Bastion formed up at the barrier in its best order and read its roster out loud. Nobody came to relieve it.";
				break;
			}
			case 2:
			{
				// The rations bribe halves the hit
				int hit = -8;
				if (run.Flag("rations") != 0)
					hit = -4;

				NudgeMorale(gm, RZ_EFaction.MILITARY, hit);
				NudgeMorale(gm, RZ_EFaction.BANDITS, -3);
				NudgeMorale(gm, RZ_EFaction.INSTITUTE, -3);
				RZ_MajorHelp.PayIdents(gm, deciders, RZ_EFaction.MILITARY, 20, 0);
				RZ_MajorHelp.PayIdents(gm, deciders, RZ_EFaction.SURVIVORS, 10, 0);
				RZ_MajorHelp.Relation(gm, RZ_EFaction.SURVIVORS, RZ_EFaction.MILITARY, RZ_FactionLife.REL_TRUCE);
				RZ_MajorHelp.OpenPage(gm, "Bastion's Day 3 orders, read on the open channel");
				if (budget)
				{
					budget.SetContractWeight("RZ_Mission_Requisition", 0.8);
					budget.SetContractWeight("RZ_Job_HeartsAndMinds", 1.2);
				}

				line = "Bastion's commanding officer read the Day 3 orders on the open channel last night, all of them. Then the numbers, to everyone inside the line.";
				story = "When the gates opened, Bastion was at the north road, behind the column it walked there on Day 2.";
				break;
			}
			case 3:
			{
				NudgeMorale(gm, RZ_EFaction.MILITARY, 8);
				NudgeStrength(gm, RZ_EFaction.MILITARY, 5);
				NudgeMorale(gm, RZ_EFaction.SURVIVORS, -8);
				RZ_MajorHelp.PayIdents(gm, deciders, RZ_EFaction.MILITARY, 25, 0);
				if (budget)
				{
					budget.SetContractWeight("RZ_Mission_Requisition", 1.2);
					budget.SetContractWeight("RZ_Mission_CheckpointDuty", 1.2);
				}

				line = "Bastion called Command at 20:30 last night. For the first time, someone answered. It was the stronghold.";
				story = "At the north road, Bastion searched everyone who walked to the barrier, and wrote down every name.";
				break;
			}
			default:
			{
				NudgeMorale(gm, RZ_EFaction.MILITARY, -10);
				NudgeStrength(gm, RZ_EFaction.MILITARY, -5);
				NudgeMorale(gm, RZ_EFaction.BANDITS, 5);
				RZ_MajorHelp.Relation(gm, RZ_EFaction.MILITARY, RZ_EFaction.BANDITS, RZ_FactionLife.REL_FEUD);
				if (budget)
				{
					budget.SetContractWeight("RZ_Mission_StruckOff", 1.2);
					budget.SetContractWeight("RZ_Mission_Requisition", 1.2);
				}

				line = "At half eight every Bastion post heard one word read over the CO's call: \"expended\". Bastion's channel stayed silent the rest of the night.";
				story = "Each night the CO is alone on the net: \"Effective strength " + run.Text("count") + ". Bastion out.\"";
				break;
			}
		}

		Headline(gm, line);
		if (fallback)
			RZ_Majors.Get().TellDefault(line);

		run.SetFlag("ending", E_M_CALL + stance - 1);
		RZ_Majors.SeasonStory(RZ_EFaction.MILITARY, KEY, E_M_CALL + stance - 1, stance, story, deciders, fallback);
		ActEnd(gm, run, A_V, stance, "");
		return S_CALL_WAIT;
	}

	//------------------------------------------------------------------------------------------------
	protected int LastRoll(RZ_GameModeComponent gm, RZ_QuestRun run, bool fallback)
	{
		RZ_MajorHelp.SetWorld(gm, KEY + ".coda", 1);
		RZ_MajorHelp.SetWorld(gm, KEY + ".last_line", 1);
		RZ_MajorHelp.HallNames(run, S_CALL, "At the barrier", "");
		string line = "At dawn Sedlak walked the tags to the north road and read the last roll, 150 metres short of the barrier. The loudspeaker read its processing order. It didn't say Bastion.";
		if (fallback)
			line = "The CO read the last call alone. At dawn Sedlak walked whoever was left to the north road and read the roll. The box of tags is back on the nail.";

		// Struck Off's men: walked home this season, or still out (Missions/Bastion/RZ_BastionLoreMissions.c)
		string roll = RZ_Mission_StruckOff.LastRoll();
		if (!roll.IsEmpty())
			line += " " + roll;

		return Finish(run, run.Flag("ending", E_M_CALL), line);
	}

	//------------------------------------------------------------------------------------------------
	override void OnEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
		if (ending >= E_M_CALL)
			return;

		super.OnEnding(gm, run, ending);
	}

	// ================================================================ Defaults
	//------------------------------------------------------------------------------------------------
	override int ActDefault(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_MajorAct act)
	{
		switch (act.m_iId)
		{
			case A_I:
			case A_II:
			{
				// A stronghold patrol walks the runner's road; the page goes in the stove
				RZ_BastionQuest.SetWorld(gm, KEY + ".stove", 1);
				run.SetBranch(BRANCH_STOVE);
				RZ_Majors.Get().TellDefault("A Bastion patrol walked a runner's road and came back with a page for the stove. The log says \"No reply\" again.");
				ActEnd(gm, run, act.m_iId, 0, "");
				return S_WAIT;
			}
			case A_III:
			{
				// The branch settles itself, at half effect
				int ending = E_STOVE;
				switch (run.m_iBranch)
				{
					case BRANCH_READ: ending = E_READ; break;
					case BRANCH_MOCKED: ending = E_MOCKED; break;
					case BRANCH_HAVEN: ending = E_HAVEN; break;
				}

				RZ_Majors.Enter(KEY, true, 0.5);
				OnEnding(gm, run, ending);
				RZ_Majors.Enter(KEY, true);
				RZ_MajorHelp.SetWorld(gm, KEY + ".reckoning", ending + 1);
				ActEnd(gm, run, A_III, ending + 1, "");
				return S_COUNT_WAIT;
			}
			case A_IV: return Counted(gm, run, true);
			case A_V:
			{
				ReleaseEvening(S_MUSTER);
				return Muster(gm, run, CoDecides(gm), true);
			}
			case A_VI: return LastRoll(gm, run, true);
		}

		return S_CALL_WAIT;
	}

	//------------------------------------------------------------------------------------------------
	//! The muster's default: the CO decides alone, by act III's result and the CO's traits.
	protected int CoDecides(RZ_GameModeComponent gm)
	{
		int reckoning = RZ_MajorHelp.World(gm, KEY + ".reckoning");
		if (RZ_MajorHelp.TopHas(gm, RZ_EFaction.MILITARY, RZ_ELeaderTrait.MERCIFUL) || RZ_MajorHelp.TopHas(gm, RZ_EFaction.MILITARY, RZ_ELeaderTrait.IDEALIST))
		{
			if (reckoning == E_READ + 1)
				return 2;
		}

		if (RZ_MajorHelp.TopHas(gm, RZ_EFaction.MILITARY, RZ_ELeaderTrait.RUTHLESS) || RZ_MajorHelp.TopHas(gm, RZ_EFaction.MILITARY, RZ_ELeaderTrait.PARANOID))
			return 3;

		return 1;
	}

	// ================================================================ Voice
	//------------------------------------------------------------------------------------------------
	override string BetweenLine(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		return "\"The CO's been reading the numbers slower. Like there's more after them.\"";
	}

	//------------------------------------------------------------------------------------------------
	override string SeasonState(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		switch (RZ_MajorHelp.World(gm, KEY + ".stance"))
		{
			case 1: return "The stance: the call stands.";
			case 2: return "The stance: all stations.";
			case 3: return "The stance: Bastion, Command.";
			case 4: return "The stance: expended.";
		}

		if (RZ_MajorHelp.World(gm, KEY + ".count") > 0)
			return string.Format("Effective strength %1.", RZ_MajorHelp.World(gm, KEY + ".count"));

		return "";
	}
}
