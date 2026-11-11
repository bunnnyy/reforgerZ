//! Every Name: the Haven's major (docs/QUESTLINES.md, "Every Name"), under the new key
//! "every_name". H1 The List is its spine: H1's four investigation stages (and their stage
//! missions, texts and list.* flags) run as act III, and H1's built endings are applied by the
//! second sitting. H1 stays registered under "list" (saved runs load) and is never offered while the
//! majors are on.
//!
//! I. The Ones Who Can't Work (Early Outbreak, the season's first set piece): a family of four at
//!    the edge of town; the head will take the two who can work. Talk the head round, pay the
//!    stores, walk two to Bastion, or walk two back to the road.
//! II. The First Sitting (Collapse, a set piece at supper): the council writes the second rule
//!    down, by RZ_QuestVote: the shared pot, workers first, Bastion's cards or the crossroads'
//!    arrangement (the offers only when a player tables them). haven.rule until the second sitting.
//! III. The List (H1 stages 1-4, as built).
//! IV. The Hungry Week (Overrun): the rule's cost lands on one house. Feed it, hold it, move two
//!     to Bastion, or walk three to the far house; the heads who saw it lean that way.
//! V. The Second Sitting (Aftermath or 70%, after Bastion's muster; the climax): the vote again,
//!    and the List: read it out (and hold the gate), give the copies to the chair, or carry them to
//!    Bastion or a crew boss. The rule and the List make the ending.
//! VI. A Week On (after the sitting): the ending's first real cost.
//! VII. The Gate (the epilogue's first coda): the chair reads the List on the north road.
//!
//! Simplified (docs/QUESTLINES.md): no raid on the first sitting, no winter store, no gathering of
//! the other factions' pages, and A Week On is one scene with a choice per rule.
class RZ_Major_EveryName : RZ_Quest_TheList
{
	static const string KEY = "every_name";

	static const int S_FAMILY = 100;
	static const int S_SUMMONS = 110;
	static const int S_SITTING1 = 111;
	static const int S_HUNGRY_WAIT = 120;
	static const int S_HUNGRY = 121;
	static const int S_SITTING2_WAIT = 130;
	static const int S_SITTING2 = 131;
	static const int S_WEEK_WAIT = 140;
	static const int S_WEEK = 141;
	static const int S_GATE_WAIT = 150;
	static const int S_GATE = 151;

	static const int A_FAMILY = 1;
	static const int A_FIRST = 2;
	static const int A_LIST = 3;
	static const int A_HUNGRY = 4;
	static const int A_SECOND = 5;
	static const int A_WEEK = 6;
	static const int A_GATE = 7;

	// The rules (vote options; haven.rule is the option + 1)
	static const int R_POT = 0;
	static const int R_WORKERS = 1;
	static const int R_CARDS = 2;
	static const int R_ARRANGEMENT = 3;

	static const int OUT_ALL_IN = 30;
	static const int OUT_TO_BASTION = 31;
	static const int OUT_TURNED = 32;
	static const int OUT_RULE = 40;		// + the rule
	static const int OUT_FED = 50;
	static const int OUT_HELD = 51;
	static const int OUT_MOVED = 52;
	static const int OUT_GIVEN_UP = 53;
	static const int OUT_SAT = 60;		// + the rule (the second sitting's tally)
	static const int OUT_WEEK = 70;
	static const int OUT_WEEK_HELD = 71;
	static const int OUT_GATE = 80;

	static const int E_M_EVERY = 10;
	static const int E_M_CLOSED = 11;
	static const int E_M_FAR = 12;
	static const int E_M_COUNTED = 13;
	static const int E_M_PAID = 14;

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		super.Define();
		Info(KEY, "Every Name", RZ_EFaction.SURVIVORS);
		Major();

		RZ_QuestStage family = MissionStage(S_FAMILY, "The ones who can't work", "A family of four waits at the edge of {family_town} at dusk: two who can work, a grandfather in a wheelchair, and a girl of eight with a cough. {family_head} will take two.");
		family.GiveUp(4, "");

		RZ_QuestStage summons = WaitStage(S_SUMMONS, "The first sitting", "The houses are filling and the trucks come less often. Sooner or later {chair} will call the council to write the second rule down.");
		summons.Wait(0);

		RZ_QuestStage first = MissionStage(S_SITTING1, "The first sitting", "\"All houses. The council sits at {chair_place} at supper. Every head comes, or sends someone. Bring your count. Bring yourself either way.\"");
		first.GiveUp(4, "");

		RZ_QuestStage hungryWait = WaitStage(S_HUNGRY_WAIT, "The hungry week", "The rule is written. Now somebody finds out what it costs.");
		hungryWait.Wait(0);

		RZ_QuestStage hungry = MissionStage(S_HUNGRY, "The hungry week", "The house in {hungry_house} can't feed its mouths this week. Under the rule, somebody already has an answer.");
		hungry.GiveUp(4, "");

		RZ_QuestStage secondWait = WaitStage(S_SITTING2_WAIT, "The second sitting", "The hungry days are turning. {chair} will call the houses to keep the rule or change it.");
		secondWait.Wait(120);

		RZ_QuestStage second = MissionStage(S_SITTING2, "The second sitting", "The council sits again at {chair_place} at supper, to keep the rule or change it. If anyone found the truth about the List, the copies are on the table too.");
		second.GiveUp(4, "");

		RZ_QuestStage weekWait = WaitStage(S_WEEK_WAIT, "A week on", "The rule stands. Its first real cost is on its way.");
		weekWait.Wait(0);

		RZ_QuestStage week = MissionStage(S_WEEK, "A week on", "A week on, the rule's first real cost has turned up at {week_house}.");
		week.GiveUp(4, "");

		RZ_QuestStage gateWait = WaitStage(S_GATE_WAIT, "The gate", "When the line stands down, {chair} will walk the Haven to the north road and read the List towards the barrier.");
		gateWait.Wait(0);

		RZ_QuestStage gate = MissionStage(S_GATE, "The gate", "{chair} walks the Haven to the north road, where the Day 2 column was turned back, and reads the List at the last bend.");
		gate.GiveUp(4, "");

		Act(A_FAMILY, "The ones who can't work").Stage(S_FAMILY).AnchorStage(RZ_EOutbreakStage.COLLAPSE).Need(1).PatienceEvenings(3).SetPiece(17, 21).Rank(0).LastCall("{family_head} says they'll be walked back to the road tomorrow at dusk.");
		Act(A_FIRST, "The first sitting").Stage(S_SUMMONS).Stage(S_SITTING1).AnchorStage(RZ_EOutbreakStage.OVERRUN).Need(1).PatienceEvenings(1).SetPiece(18.5, 20.5).Rank(0).LastCall("The council sits tomorrow at supper, whoever comes.");
		Act(A_LIST, "The List").Stage(S_HUSBAND).Stage(S_COPIES).Stage(S_PATTERN).Stage(S_FARM).AnchorCure(70).Need(4);
		Act(A_HUNGRY, "The hungry week").Stage(S_HUNGRY_WAIT).Stage(S_HUNGRY).AnchorCure(70).Need(2).Patience(3).CostsMorale().LastCall("{hungry_head} says the house will do what the rule says, tomorrow at dusk.");
		Act(A_SECOND, "The second sitting").Stage(S_SITTING2_WAIT).Stage(S_SITTING2).Opens(70).Need(1).PatienceEvenings(1).SetPiece(18.5, 20.5).Climax().Rank(0).LastCall("The council sits again tomorrow at supper. If nobody speaks, the rule stands.");
		Act(A_WEEK, "A week on").Stage(S_WEEK_WAIT).Stage(S_WEEK).PastGate().Need(1).Patience(2);
		Act(A_GATE, "The gate").Stage(S_GATE_WAIT).Stage(S_GATE).AnchorWipe().Need(0).SetPiece(17, 22).Rank(0);
	}

	//------------------------------------------------------------------------------------------------
	override int FirstStage()
	{
		return S_FAMILY;
	}

	//------------------------------------------------------------------------------------------------
	//! The season's first evening: a house near the chair's (or the chair's own) for the family.
	override bool CanStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_Leader chair = RZ_MajorHelp.Top(gm, RZ_EFaction.SURVIVORS);
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!chair || !houses || houses.Count() == 0 || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return false;

		int chairHouse = RZ_HavenQuest.ChairHouse(gm);
		int house = chairHouse;
		float best = float.MAX;
		for (int i = 0; i < houses.Count(); i++)
		{
			if (i == chairHouse || !RZ_HavenQuest.LiveHouse(gm, i))
				continue;

			float d = RZ_Util.DistXZ(houses.Get(i).m_vPos, chair.m_vPos);
			if (d < best)
			{
				best = d;
				house = i;
			}
		}

		if (house < 0)
			house = 0;

		RZ_Safehouse h = houses.Get(house);
		RZ_Leader head = RZ_HavenQuest.CouncilHead(gm, house);
		if (!head)
			head = chair;

		RZ_HavenQuest.KeepLeader(run, "chair", chair);
		run.SetFlag("family_house", house);
		run.SetText("family_town", h.m_sName);
		run.SetText("family_head", head.m_sName);
		run.SetFlag("family_head_id", head.m_iId);
		run.SetFlag("chairhouse", chairHouse);
		run.SetPlace(h.m_vPos, h.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		Headline(gm, run.Fill("A family of four is waiting at the edge of {family_town}. The house there has room for two."), false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStageBegin(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (stage < S_FAMILY)
		{
			super.OnStageBegin(gm, run, stage);
			return;
		}

		RZ_Leader chair = RZ_MajorHelp.Top(gm, RZ_EFaction.SURVIVORS);
		if ((stage == S_SITTING1 || stage == S_SITTING2) && chair)
			run.SetPlace(chair.m_vPos, chair.m_sPlace);

		if (stage == S_SITTING1 && run.Flag("summoned") == 0)
		{
			run.SetFlag("summoned", 1);
			if (gm.GetFactionLife())
				gm.GetFactionLife().OpenChannelLine(RZ_EFaction.SURVIVORS, run.Fill("All houses. The council sits at {chair_place} at supper. Every head comes, or sends someone. Bring your count. Bring yourself either way."));
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool IsReady(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		switch (stage)
		{
			case S_FAMILY: return Free(gm, run, S_FAMILY);
			case S_SUMMONS:
			{
				int st = gm.GetDirector().GetStage();
				if (st < RZ_EOutbreakStage.COLLAPSE)
					return false;

				return st >= RZ_EOutbreakStage.OVERRUN || RZ_HavenQuest.AnyHungry(gm) || (gm.GetFactionLife() && gm.GetFactionLife().GetNeed(RZ_EFaction.SURVIVORS) == RZ_ENeed.FOOD);
			}
			case S_SITTING1: return RZ_MajorHelp.PlayedSince(run, 2) && Free(gm, run, S_SITTING1);
			case S_HUSBAND:
			{
				// Act III opens once a house has lost people or The List contract was done (H1's own
				// start), and picks its people then
				if (run.Flag("list_setup") == 1)
					return super.IsReady(gm, run, stage);

				if (!super.CanStart(gm, run))
					return false;

				run.SetFlag("list_setup", 1);
				return true;
			}
			case S_HUNGRY_WAIT:
			{
				bool overrun = gm.GetDirector().GetStage() >= RZ_EOutbreakStage.OVERRUN;
				return overrun || RZ_Questline.GetMorale(gm, RZ_EFaction.SURVIVORS) < 45;
			}
			case S_HUNGRY: return PickHungry(gm, run);
			case S_SITTING2_WAIT:
			{
				bool due = gm.GetDirector().GetStage() >= RZ_EOutbreakStage.AFTERMATH || RZ_Majors.Reached(run, 70);
				return due && RZ_Majors.ClimaxDone("orders");
			}
			case S_SITTING2: return Free(gm, run, S_SITTING2);
			case S_WEEK_WAIT:
				return RZ_HavenQuest.AnyHungry(gm) || RZ_Majors.BeatGapHours(run) >= 4;
			case S_WEEK: return PickWeek(gm, run);
			case S_GATE_WAIT: return RZ_MajorHelp.CodaDue(gm, "");
			case S_GATE: return Free(gm, run, S_GATE);
		}

		return super.IsReady(gm, run, stage);
	}

	//------------------------------------------------------------------------------------------------
	protected bool Free(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		RZ_SetPieceCalendar cal = RZ_SetPieceCalendar.Get();
		RZ_MajorAct act = ActOf(stage);
		if (!cal || !act || !RZ_Majors.InWindow(act))
			return false;

		return RZ_MajorHelp.HoldEvening(run, string.Format("%1.%2", KEY, stage), run.m_vPlace);
	}

	//------------------------------------------------------------------------------------------------
	protected void ReleaseEvening(int stage)
	{
		RZ_SetPieceCalendar cal = RZ_SetPieceCalendar.Get();
		if (cal)
			cal.Release(string.Format("%1.%2", KEY, stage));
	}

	//------------------------------------------------------------------------------------------------
	//! Act IV's house: the family's if they came in, else the hungriest one.
	protected bool PickHungry(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.HasFlag("hungry_house"))
			return true;

		int house = -1;
		if (RZ_MajorHelp.World(gm, KEY + ".family") == 1)
			house = run.Flag("family_house", -1);

		RZ_Safehouses houses = gm.GetSafehouses();
		int least = 100000;
		for (int i = 0; i < houses.Count() && house < 0; i++)
		{
			RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, i);
			if (h && h.m_iFood < least)
			{
				least = h.m_iFood;
				house = i;
			}
		}

		RZ_Safehouse pick = RZ_HavenQuest.LiveHouse(gm, house);
		if (!pick)
			return false;

		run.SetFlag("hungry_house", house);
		run.SetText("hungry_house", pick.m_sName);
		RZ_Leader head = RZ_HavenQuest.CouncilHead(gm, house);
		string name = "the council head";
		if (head)
			name = head.m_sName;

		run.SetText("hungry_head", name);
		ChangeFood(gm, house, -2);
		run.SetPlace(pick.m_vPos, pick.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool PickWeek(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.HasFlag("week_house"))
			return true;

		int house = run.Flag("hungry_house", 0);
		if (RZ_MajorHelp.World(gm, "haven.rule") == 2)
			house = FarHouse(gm);

		RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, house);
		if (!h)
			return false;

		run.SetFlag("week_house", house);
		run.SetText("week_house", h.m_sName);
		run.SetPlace(h.m_vPos, h.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The far house: the smallest safehouse farthest from the stronghold.
	static int FarHouse(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		RZ_Leader chair = RZ_MajorHelp.Top(gm, RZ_EFaction.SURVIVORS);
		if (!houses || !chair)
			return -1;

		int best = -1;
		float bestScore = -1;
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, i);
			if (!h)
				continue;

			float score = RZ_Util.DistXZ(h.m_vPos, chair.m_vPos) - h.m_iPopulation * 50;
			if (score > bestScore)
			{
				bestScore = score;
				best = i;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	override RZ_Mission MakeMission(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (stage >= S_FAMILY)
			return new RZ_QM_MajorScene();

		return super.MakeMission(gm, run, stage);
	}

	// ================================================================ The votes
	//------------------------------------------------------------------------------------------------
	//! A head's lean: trait first, then the house's food, then what the house did in act I.
	protected int LeanOf(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_Leader head, int chairLean)
	{
		if (head.HasTrait(RZ_ELeaderTrait.MERCIFUL) || head.HasTrait(RZ_ELeaderTrait.IDEALIST))
			return R_POT;

		if (head.HasTrait(RZ_ELeaderTrait.RUTHLESS) || head.HasTrait(RZ_ELeaderTrait.PARANOID))
			return R_WORKERS;

		if (head.HasTrait(RZ_ELeaderTrait.PRAGMATIST))
			return R_CARDS;

		if (head.HasTrait(RZ_ELeaderTrait.GREEDY))
			return R_ARRANGEMENT;

		if (head.m_eLoyalty == RZ_ELoyalty.LOYAL)
			return chairLean;

		RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, head.m_iSafehouse);
		if (h && RZ_HavenQuest.IsHungry(h))
			return R_WORKERS;

		return R_POT;
	}

	//------------------------------------------------------------------------------------------------
	protected int ChairLean(RZ_GameModeComponent gm)
	{
		RZ_Leader chair = RZ_MajorHelp.Top(gm, RZ_EFaction.SURVIVORS);
		if (chair && (chair.HasTrait(RZ_ELeaderTrait.RUTHLESS) || chair.HasTrait(RZ_ELeaderTrait.PARANOID)))
			return R_WORKERS;

		return R_POT;
	}

	//------------------------------------------------------------------------------------------------
	//! The voters: every council head, topped up to three with the stronghold's named people. The
	//! second sitting starts from the first's leans, moved by the hungry week.
	protected void SeatVoters(RZ_GameModeComponent gm, RZ_QuestRun run, string name)
	{
		RZ_QuestVote vote = new RZ_QuestVote(run, name);
		if (vote.Count() > 0)
			return;

		int chairLean = ChairLean(gm);
		RZ_QuestVote first = new RZ_QuestVote(run, "first");
		RZ_Safehouses houses = gm.GetSafehouses();
		int chairHouse = run.Flag("chairhouse", -1);
		for (int i = 0; i < houses.Count() && vote.Count() < 5; i++)
		{
			if (i == chairHouse)
				continue;

			RZ_Leader head = RZ_HavenQuest.CouncilHead(gm, i);
			if (!head || head.IsStronghold())
				continue;

			int lean = LeanOf(gm, run, head, chairLean);
			if (name != "first")
				lean = LeanFromFirst(first, head.m_iId, lean);
			else if (i == run.Flag("family_house", -1))
				lean = FamilyLean(gm, lean);

			vote.AddVoter(head.m_sName, lean, head.m_iId);
		}

		array<string> named = {"the stronghold's cook", "the stores clerk", "the night watch"};
		foreach (string person : named)
		{
			if (vote.Count() >= 3)
				break;

			vote.AddVoter(person, chairLean);
		}

		array<string> words = {"pot", "workers", "cards", "tins"};
		vote.Words(words);
		vote.Table(R_POT);
		vote.Table(R_WORKERS);
		if (CardsAllowed(gm) && (RZ_MajorHelp.World(gm, KEY + ".family") == 2 || RZ_MajorHelp.World(gm, "door.bastion") != 0 || (name != "first" && first.Result() == R_CARDS)))
			vote.Table(R_CARDS);

		if (RZ_MajorHelp.World(gm, "door.tins") != 0 || RZ_MajorHelp.World(gm, "door.paid") != 0 || RZ_MajorHelp.World(gm, "walk.cut_dose") != 0 || (name != "first" && first.Result() == R_ARRANGEMENT))
			vote.Table(R_ARRANGEMENT);

		// The hungry week: the heads who saw the rule work, or fail, lean the way it went
		int week = run.Flag("week_lean", -1);
		if (name != "first" && week >= 0)
			vote.Swing(-1, week, 2);
	}

	//------------------------------------------------------------------------------------------------
	protected static int LeanFromFirst(RZ_QuestVote first, int leaderId, int fallback)
	{
		for (int i = 0; i < first.Count(); i++)
		{
			if (first.VoterLeader(i) == leaderId)
				return first.Lean(i);
		}

		return fallback;
	}

	//------------------------------------------------------------------------------------------------
	protected int FamilyLean(RZ_GameModeComponent gm, int lean)
	{
		switch (RZ_MajorHelp.World(gm, KEY + ".family"))
		{
			case 1: return R_POT;
			case 2: return R_CARDS;
			case 3: return R_WORKERS;
		}

		return lean;
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion's cards are never on the table after Expended: Bastion has nothing left to count for.
	protected bool CardsAllowed(RZ_GameModeComponent gm)
	{
		return RZ_MajorHelp.World(gm, "orders.stance") != 4;
	}

	//------------------------------------------------------------------------------------------------
	static string RuleName(int rule)
	{
		switch (rule)
		{
			case R_POT: return "the shared pot";
			case R_WORKERS: return "workers first";
			case R_CARDS: return "Bastion's cards";
			case R_ARRANGEMENT: return "the arrangement with the crossroads";
		}

		return "nothing yet";
	}

	// ================================================================ Scenes
	//------------------------------------------------------------------------------------------------
	override bool BuildScene(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene)
	{
		RZ_Leader chair = RZ_MajorHelp.Top(gm, RZ_EFaction.SURVIVORS);
		if (!chair)
			return false;

		scene.m_iGiver = RZ_EFaction.SURVIVORS;
		switch (stage)
		{
			case S_FAMILY: return FamilyScene(gm, run, scene);
			case S_SITTING1: return SittingScene(gm, run, scene, chair, "first");
			case S_HUNGRY: return HungryScene(gm, run, scene);
			case S_SITTING2: return SittingScene(gm, run, scene, chair, "second");
			case S_WEEK: return WeekScene(gm, run, scene);
			case S_GATE: return GateScene(gm, run, scene, chair);
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool FamilyScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene)
	{
		RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, run.Flag("family_house", -1));
		if (!h)
			return false;

		scene.m_sTitle = "The ones who can't work";
		scene.m_sBrief = run.Fill("A family at the edge of {family_town}: two who can work, a grandfather in a wheelchair, a girl of eight with a cough. {family_head}: \"We can take two. The two who can work. Not with that cough.\"");
		scene.m_eTier = RZ_ETier.STRANGER;
		scene.m_iFame = 5;
		scene.m_iFavor = 10;
		scene.m_vPos = RZ_Mission.LandPointNear(h.m_vPos, 80, 140);
		scene.m_iTimeout = -1;
		int easier = 2;
		RZ_Leader head = RZ_BastionQuest.Leader(gm, run.Flag("family_head_id", -1));
		if (head && head.HasTrait(RZ_ELeaderTrait.MERCIFUL))
			easier = 1;

		int spot = scene.Person(scene.m_vPos, run.Fill("{family_head} at the edge of town"), "Talk to the council head", "\"It's the first hungry week again, in small. I can feed two who work. I can't feed four who don't.\"");
		scene.Choice(spot, "Talk the head round").Persuade(RZ_EFaction.SURVIVORS, easier, "elder,nurse,doctor").Decides(OUT_ALL_IN).Says("All four come in. The girl's cough is a chest cold, and the grandfather asks where he can be useful. The house is two meals shorter.", "talked the head into taking all four").Refused("\"Kind of you. Feed them yourself, then.\"");
		scene.Choice(spot, "Pay into the stores (20 Favor)").Bribe(RZ_EFaction.SURVIVORS, 20).Decides(OUT_ALL_IN).Says("The stores take your Favor in tins. All four come in, and the head writes four names on the house's List.", "paid the stores for the family");
		scene.Choice(spot, "Walk two to a Bastion post").Needs(RZ_EFaction.MILITARY, 1).Side(RZ_EFaction.MILITARY, -1, 5, 0).Decides(OUT_TO_BASTION).Says("The grandfather and the girl are signed for at the nearest post, as civilians sheltered. The two who can work go in.", "walked two of the family to Bastion");
		scene.Choice(spot, "Walk two back to the road").Way(RZ_EQuestApproach.LEAVE).Decides(OUT_TURNED).Says("At dusk you walk the grandfather and the girl back to the road. The head pays you, and writes two names on the List with a line through them.", "walked two of the family back to the road");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool SittingScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene, RZ_Leader chair, string voteName)
	{
		SeatVoters(gm, run, voteName);
		RZ_QuestVote vote = new RZ_QuestVote(run, voteName);
		bool second = voteName != "first";
		scene.m_sTitle = "The first sitting";
		scene.m_sBrief = run.Fill("Supper at {chair_place}. On the table, in the Haven's own words: everyone eats, or workers first. Bastion's cards and the crossroads' tins only if somebody carries the offer.");
		if (second)
		{
			scene.m_sTitle = "The second sitting";
			scene.m_sBrief = run.Fill("The council sits again at {chair_place}, to keep the rule or change it. If the List's truth came out, the copies are on the table too.");
		}

		scene.m_eTier = RZ_ETier.STRANGER;
		scene.m_iFame = 5;
		scene.m_iFavor = 10;
		scene.m_vPos = chair.m_vPos;
		scene.m_fWindow = 5400;
		int base = OUT_RULE;
		if (second)
			base = OUT_SAT;

		array<int> outcomes = {};
		for (int o = R_POT; o <= R_ARRANGEMENT; o++)
		{
			outcomes.Insert(base + o);
		}

		scene.SetVote(voteName, outcomes, ChairLean(gm));
		run.SetFlag("t_cards", RZ_Majors.Bit(vote.IsTabled(R_CARDS)));
		run.SetFlag("t_arr", RZ_Majors.Bit(vote.IsTabled(R_ARRANGEMENT)));
		run.SetFlag("cards_ok", RZ_Majors.Bit(CardsAllowed(gm)));

		for (int i = 0; i < vote.Count(); i++)
		{
			vector at = RZ_MajorHelp.NearSeat(chair.m_vPos);
			int spot = scene.Person(at, vote.VoterName(i), "Talk to " + vote.VoterName(i), string.Format("\"I lean to %1. Tell me why I shouldn't.\"", RuleName(vote.Lean(i))), i);
			scene.Choice(spot, "Speak for the shared pot [Known]").Needs(RZ_EFaction.SURVIVORS, 1).Sway(R_POT).Says("\"Everyone eats.\" The head writes it down and underlines it.", "spoke for the shared pot");
			scene.Choice(spot, "Speak for workers first [Known]").Needs(RZ_EFaction.SURVIVORS, 1).Sway(R_WORKERS).Says("\"Houses that work, eat.\" The head doesn't look up, and nods.", "spoke for workers first");
			scene.Choice(spot, "Speak for Bastion's cards [Known]").Needs(RZ_EFaction.SURVIVORS, 1).Only("t_cards").Sway(R_CARDS).Says("\"Names for bread.\" The head turns the card over and reads the back.", "spoke for Bastion's cards").Refused("Nobody has put Bastion's cards on the table.");
			scene.Choice(spot, "Speak for the arrangement [Known]").Needs(RZ_EFaction.SURVIVORS, 1).Only("t_arr").Sway(R_ARRANGEMENT).Says("\"Tins until the trucks come back. Nobody asks the price.\" The head asks the price.", "spoke for the arrangement").Refused("Nobody has put the crossroads' tins on the table.");
		}

		int clerk = scene.Person(RZ_MajorHelp.NearSeat(chair.m_vPos), "the chair's clerk", "Talk to the clerk", ClerkLine(gm, run, second));
		if (!second)
		{
			scene.Choice(clerk, "Table Bastion's cards [Bastion: Known]").Needs(RZ_EFaction.MILITARY, 1).Only("cards_ok").Tables(R_CARDS).Side(RZ_EFaction.MILITARY, -1, 5, 0).Says("Ration cards for every house on the census, a curfew, and a contribution when Bastion is short. The clerk puts them on the table.", "tabled Bastion's cards");
			scene.Choice(clerk, "Table the crossroads' tins (15 Favor)").Bribe(RZ_EFaction.BANDITS, 15).Tables(R_ARRANGEMENT).Says("Tins until the trucks come back. The clerk writes \"arrangement\" and doesn't write who with.", "tabled the Red Hand's arrangement");
			scene.Choice(clerk, "Read the best-fed house's stores").Sneak().Swings(R_POT, 1).Says("Behind the lookouts, a storeroom with more than its share. The head of that house goes quiet at supper, then votes for the pot.", "found a house keeping back food");
			return true;
		}

		run.SetFlag("truth", RZ_Majors.Bit(RZ_MajorHelp.World(gm, "list.truth") > 0));
		scene.Choice(clerk, "Read it out").Persuade(RZ_EFaction.SURVIVORS, ReadStep(gm, run), "elder").Only("truth").HoldAt(chair.m_vPos, 180, OUT_SAT + 10, OUT_SAT + 11).Says("You read the first crossed name. {chair} answers once: \"We'd all be dead. I decided, and I'd decide it again. Read the next one.\" The bells bring them.", "read the List out at supper").Refused("\"Read what? Sit down and eat.\"");
		scene.Choice(clerk, "Give the copies to the chair").Sets("buried", 1).Swings(ChairLean(gm), 1).Says("{chair} puts the copies in the drawer and locks it. The loyal heads vote with the chair.", "gave the copies to the chair");
		scene.Choice(clerk, "Carry the copies to Bastion").Sneak().Needs(RZ_EFaction.MILITARY, 1).Only("cards_ok").Sets("to_bastion", 1).Tables(R_CARDS).Swings(R_CARDS, 2).Side(RZ_EFaction.MILITARY, RZ_EFaction.SURVIVORS, 10, 15).Says("The copies go to a Bastion seat before the vote. Two heads come back from supper talking about the census.", "carried the List to Bastion");
		scene.Choice(clerk, "Sell the copies to a crew boss").Sneak().Sets("to_redhand", 1).Tables(R_ARRANGEMENT).Swings(R_ARRANGEMENT, 2).Says("The copies go to the crossroads. Two heads come back from supper talking about tins.", "sold the List to the Red Hand");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected string ClerkLine(RZ_GameModeComponent gm, RZ_QuestRun run, bool second)
	{
		if (!second)
			return "\"Two rules on the table. Bring an offer and there's four. The vote is at half eight.\"";

		if (RZ_MajorHelp.World(gm, "list.truth") == 1)
			return "\"The copies are here. So is the radio log, Day 7, in the chair's hand. Somebody has to decide whether supper hears it.\"";

		return "\"The copies are here. They point at a farm. That's all they point at.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! "Read it out": persuade 2 (Trusted); one easier for each proof (two at most), and once if
	//! another stronghold's page is out; one harder with only half the truth.
	protected int ReadStep(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		int proofs = 0;
		array<string> keys = {"list.husband_home", KEY + ".boy", KEY + ".witness", "cargo.freed", "door.exile_home", "table.walked_out"};
		foreach (string k : keys)
		{
			if (RZ_MajorHelp.World(gm, k) != 0)
				proofs++;
		}

		int d = 2 - Math.MinInt(proofs, 2) - RZ_MajorHelp.PageStep(gm, run);
		if (RZ_MajorHelp.World(gm, "list.truth") == 2)
			d++;

		return Math.ClampInt(d, 1, 4);
	}

	//------------------------------------------------------------------------------------------------
	protected bool HungryScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene)
	{
		RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, run.Flag("hungry_house", -1));
		if (!h)
			return false;

		scene.m_sTitle = "The hungry week";
		scene.m_sBrief = run.Fill("The house in {hungry_house} can't feed its mouths this week. Under {rule}, somebody already has an answer. Every answer is open, whatever the rule says.");
		scene.m_eTier = RZ_ETier.KNOWN;
		scene.m_iFame = 6;
		scene.m_iFavor = 15;
		scene.m_vPos = h.m_vPos;
		scene.m_fWindow = 5400;
		scene.m_iTimeout = -1;
		scene.SetHold(h.m_vPos, 240, OUT_HELD, OUT_GIVEN_UP);
		int spot = scene.Person(RZ_MajorHelp.NearSeat(h.m_vPos), run.Fill("{hungry_head} at the stores"), "Talk to the council head", "\"Four days of food for six days. I've done the sums three times.\"");
		scene.Choice(spot, "Pay for their food (30 Favor)").Bribe(RZ_EFaction.SURVIVORS, 30).Decides(OUT_FED).Says("Your Favor turns into four days of tins from the best-fed house. Nobody's walked anywhere tonight.", "fed the hungry house");
		scene.Choice(spot, "Hold the house tonight").Hold().Says("The doors are barred. Three times before dawn something tries them.", "held the hungry house");
		scene.Choice(spot, "Walk two to a Bastion post").Needs(RZ_EFaction.MILITARY, 1).Only("cards_ok").Side(RZ_EFaction.MILITARY, -1, 6, 0).Decides(OUT_MOVED).Says("Two mouths signed for at a Bastion post as civilians sheltered. The head writes \"moved\" next to their names.", "walked two mouths to Bastion").Refused("Bastion's posts take nobody in now.");
		scene.Choice(spot, "Walk three to the far house").Way(RZ_EQuestApproach.LEAVE).Decides(OUT_GIVEN_UP).Says("At dusk you walk three of them to the far house, where there's room. Nobody says the word for them.", "walked mouths to the far house");
		run.SetFlag("cards_ok", RZ_Majors.Bit(CardsAllowed(gm)));
		run.SetText("rule", RuleName(RZ_MajorHelp.World(gm, "haven.rule", 1) - 1));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool WeekScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene)
	{
		RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, run.Flag("week_house", -1));
		if (!h)
			return false;

		int rule = RZ_MajorHelp.World(gm, "haven.rule", 1);
		scene.m_sTitle = "A week on";
		scene.m_sBrief = WeekBrief(rule);
		scene.m_eTier = RZ_ETier.KNOWN;
		scene.m_iFame = 5;
		scene.m_iFavor = 10;
		scene.m_vPos = h.m_vPos;
		scene.m_fWindow = 5400;
		scene.m_iTimeout = OUT_WEEK;
		run.SetFlag("rule_pot", RZ_Majors.Bit(rule == 1));
		run.SetFlag("rule_far", RZ_Majors.Bit(rule == 2));
		run.SetFlag("rule_paid", RZ_Majors.Bit(rule == 4));
		int spot = scene.Person(RZ_MajorHelp.NearSeat(h.m_vPos), run.Fill("the head at {week_house}"), "Talk to the council head", "\"A week on, and the rule has a cost now. It has names, too.\"");
		scene.Choice(spot, "Hear both houses and split it [Known]").Needs(RZ_EFaction.SURVIVORS, 1).Only("rule_pot").Decides(OUT_WEEK_HELD).Says("Both houses talk, then both eat. The pot holds, for now.", "split the borrowed food between houses").Refused("That isn't what this house is arguing about.");
		scene.Choice(spot, "Carry food to the far house (20 Favor)").Bribe(RZ_EFaction.SURVIVORS, 20).Only("rule_far").Sets("world:every_name.far_fed").Decides(OUT_WEEK_HELD).Says("The far house eats this week. Its head tells you who left, by name.", "fed the far house").Refused("The far house isn't the one asking.");
		scene.Choice(spot, "Pay the collection (25 Favor)").Bribe(RZ_EFaction.BANDITS, 25).Only("rule_paid").Decides(OUT_WEEK_HELD).Says("The crew's man wanted a name. He takes Favor instead, and writes the house's line paid for the week.", "paid the crossroads' collection in Favor").Refused("Nobody's come collecting here.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static string WeekBrief(int rule)
	{
		switch (rule)
		{
			case 2: return "The far house has stopped answering the radio.";
			case 3: return "The first curfew night in a Haven town. Bastion's cards come with a patrol.";
			case 4: return "The crossroads' first collection wants a name, not tins.";
		}

		return "A short house \"borrowed\" from the best-fed one in the night.";
	}

	//------------------------------------------------------------------------------------------------
	protected bool GateScene(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_QM_MajorScene scene, RZ_Leader chair)
	{
		vector bend = RZ_MajorHelp.NorthRoad(gm, 150);
		if (bend == vector.Zero)
			bend = chair.m_vPos;

		scene.m_sTitle = "The gate";
		scene.m_sBrief = run.Fill("{chair} walks the Haven to the north road, where the Day 2 column was turned back. At the last bend, 150 metres short of the barrier, the List is read. Nobody behind the concrete answers.");
		scene.m_bLate = true;
		scene.m_eTier = RZ_ETier.STRANGER;
		scene.m_iFame = 0;
		scene.m_iFavor = 0;
		scene.m_vPos = bend;
		scene.m_fWindow = 5400;
		scene.m_iTimeout = OUT_GATE;
		scene.SetHold(bend, 150, OUT_GATE, OUT_GATE);
		run.SetFlag("may_read", RZ_Majors.Bit(RZ_MajorHelp.World(gm, "list.truth") == 1 && RZ_MajorHelp.World(gm, KEY + ".ending", -1) != 0));
		int spot = scene.Person(RZ_MajorHelp.NearSeat(bend), run.Fill("{chair} at the bend"), "Stand with the chair", GateLine(gm));
		scene.Choice(spot, "Stand with the chair").Decides(OUT_GATE).Says("{chair} reads the names towards the barrier. Nobody behind it answers, just as on Day 2.", "stood with the chair at the gate");
		scene.Choice(spot, "Read the crossed names yourself").Persuade(RZ_EFaction.SURVIVORS, 2, "elder").Only("may_read").Sets("world:every_name.crossed_read").Says("You read the crossed names after the chair stops. Nobody stops you.", "read the crossed names at the barrier").Refused("\"Not here. Not today.\"");
		scene.Choice(spot, "Hold the road while it's read").Hold().Says("Behind the column, the infected come down the road. The reading doesn't stop.", "held the road at the gate");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected string GateLine(RZ_GameModeComponent gm)
	{
		switch (RZ_MajorHelp.World(gm, KEY + ".ending", -1))
		{
			case 0: return "\"Every name. The crossed ones too.\"";
			case 2: return "\"The names of the houses that worked. The far house reads its own.\"";
			case 3: return "A Bastion officer stands beside the chair with the Haven's number on a card.";
			case 4: return "A Red Hand lorry is parked on the verge. Nobody looks at it.";
		}

		return "\"The names. Not the crossed ones.\"";
	}

	// ================================================================ How it goes on
	//------------------------------------------------------------------------------------------------
	override int Next(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		switch (stage)
		{
			case S_FAMILY: return AfterFamily(gm, run, outcome, false);
			case S_SUMMONS: return S_SITTING1;
			case S_SITTING1: return AfterFirst(gm, run, outcome, false);
			case S_FARM:
			{
				super.Next(gm, run, stage, outcome);
				return EndList(gm, run);
			}
			case S_HUNGRY_WAIT: return S_HUNGRY;
			case S_HUNGRY: return AfterHungry(gm, run, outcome, false);
			case S_SITTING2_WAIT: return S_SITTING2;
			case S_SITTING2: return AfterSecond(gm, run, outcome, false);
			case S_WEEK_WAIT: return S_WEEK;
			case S_WEEK: return AfterWeek(gm, run, outcome);
			case S_GATE_WAIT: return S_GATE;
			case S_GATE: return AfterGate(gm, run, false);
		}

		return super.Next(gm, run, stage, outcome);
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterFamily(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome, bool fallback)
	{
		ReleaseEvening(S_FAMILY);
		int house = run.Flag("family_house", -1);
		int family = 3;
		string line = "Two of the family went in. The grandfather and the girl were walked back to the road at dusk, and their names were crossed off. The season's first.";
		if (outcome == OUT_ALL_IN)
		{
			family = 1;
			ChangePopulation(gm, house, 3);
			ChangeFood(gm, house, -2);
			line = "All four came in at {family_town}. The girl's cough was a chest cold. Somebody painted a bear on the door.";
		}
		else if (outcome == OUT_TO_BASTION)
		{
			family = 2;
			ChangePopulation(gm, house, 2);
			line = "Two went in at {family_town}. The grandfather and the girl are civilians sheltered at a Bastion post now.";
		}
		else if (outcome == OUT_TURNED || fallback)
		{
			ChangePopulation(gm, house, 2);
		}
		else
		{
			return S_FAMILY;
		}

		RZ_MajorHelp.SetWorld(gm, KEY + ".family", family);
		if (fallback)
			RZ_Majors.Get().TellDefault(run.Fill(line));

		ActEnd(gm, run, A_FAMILY, family, line);
		return S_SUMMONS;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterFirst(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome, bool fallback)
	{
		ReleaseEvening(S_SITTING1);
		SeatVoters(gm, run, "first");
		RZ_QuestVote vote = new RZ_QuestVote(run, "first");
		int rule = outcome - OUT_RULE;
		if (fallback || rule < R_POT || rule > R_ARRANGEMENT)
			rule = vote.Tally(ChairLean(gm));

		RZ_MajorHelp.SetWorld(gm, "haven.rule", rule + 1);
		string line = string.Format("The council sat at supper and wrote the second rule down: %1.", RuleName(rule));
		if (rule == R_ARRANGEMENT)
			RZ_MajorHelp.SetWorld(gm, "haven.paying", 1);

		if (rule == R_CARDS)
			RZ_MajorHelp.SetWorld(gm, "haven.counted", 1);

		Headline(gm, string.Format("The Haven's council sat at supper last night. The houses go by %1 now.", RuleName(rule)));
		if (fallback)
			RZ_Majors.Get().TellDefault(line);

		ActEnd(gm, run, A_FIRST, rule + 1, line);
		return S_HUSBAND;
	}

	//------------------------------------------------------------------------------------------------
	protected int EndList(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		int truth = 1;
		if (run.Flag("half_truth") == 1)
			truth = 2;

		RZ_MajorHelp.SetWorld(gm, "list.truth", truth);
		ActEnd(gm, run, A_LIST, truth, "");
		return S_HUNGRY_WAIT;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterHungry(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome, bool fallback)
	{
		int rule = RZ_MajorHelp.World(gm, "haven.rule", 1) - 1;
		if (fallback)
		{
			switch (rule)
			{
				case R_WORKERS: outcome = OUT_GIVEN_UP; break;
				case R_CARDS: outcome = OUT_MOVED; break;
				default: outcome = OUT_FED; break;
			}
		}

		int house = run.Flag("hungry_house", -1);
		int lean = R_POT;
		string line = "The house in {hungry_house} ate this week.";
		switch (outcome)
		{
			case OUT_HELD: line = "The house in {hungry_house} held its doors and kept its people."; break;
			case OUT_MOVED:
			{
				lean = R_CARDS;
				ChangePopulation(gm, house, -2);
				line = "Two of the mouths at {hungry_house} are civilians sheltered at a Bastion post now.";
				break;
			}
			case OUT_GIVEN_UP:
			{
				lean = R_WORKERS;
				int farIdx = FarHouse(gm);
				int moved = RZ_HavenQuest.TakePeople(gm, house, 3);
				if (farIdx >= 0 && farIdx != house && moved > 0)
					ChangePopulation(gm, farIdx, moved);

				line = "Three of the mouths at {hungry_house} were walked to the far house at dusk, where there's room.";
				break;
			}
			case OUT_FED:
			{
				ChangeFood(gm, house, 4);
				break;
			}
			default: return S_HUNGRY;
		}

		run.SetFlag("week_lean", lean);
		RZ_MajorHelp.SetWorld(gm, KEY + ".week", outcome - OUT_FED + 1);
		if (fallback)
			RZ_Majors.Get().TellDefault(run.Fill(line));

		ActEnd(gm, run, A_HUNGRY, outcome - OUT_FED + 1, line);
		return S_SITTING2_WAIT;
	}

	//------------------------------------------------------------------------------------------------
	//! The second sitting: the rule, the List's fate, and the major's ending.
	protected int AfterSecond(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome, bool fallback)
	{
		ReleaseEvening(S_SITTING2);
		SeatVoters(gm, run, "second");
		RZ_QuestVote vote = new RZ_QuestVote(run, "second");
		bool read = outcome == OUT_SAT + 10;
		if (read)
		{
			// Every head who isn't Ruthless swings to the pot
			for (int i = 0; i < vote.Count(); i++)
			{
				RZ_Leader head = RZ_BastionQuest.Leader(gm, vote.VoterLeader(i));
				if (!head || !head.HasTrait(RZ_ELeaderTrait.RUTHLESS))
					vote.SetLean(i, R_POT);
			}
		}

		// The default: the List is kept, unless the chair knows the truth and is Merciful or Idealist
		if (fallback && RZ_MajorHelp.World(gm, "list.truth") == 1 && (RZ_MajorHelp.TopHas(gm, RZ_EFaction.SURVIVORS, RZ_ELeaderTrait.MERCIFUL) || RZ_MajorHelp.TopHas(gm, RZ_EFaction.SURVIVORS, RZ_ELeaderTrait.IDEALIST)))
			read = true;

		int rule = vote.Tally(ChairLean(gm));
		if (rule < 0)
			rule = RZ_MajorHelp.World(gm, "haven.rule", 1) - 1;

		RZ_MajorHelp.SetWorld(gm, "haven.rule", rule + 1);
		RZ_MajorHelp.SetWorld(gm, "haven.counted", RZ_Majors.Bit(rule == R_CARDS));
		RZ_MajorHelp.SetWorld(gm, "haven.paying", RZ_Majors.Bit(rule == R_ARRANGEMENT));

		// The List's fate, through H1's own endings (list.read, list.buried, list.to_bastion, list.to_redhand)
		int listEnding = E_BURIED;
		if (read)
			listEnding = E_READ;
		else if (run.Flag("to_bastion") == 1)
			listEnding = E_CENSUS;
		else if (run.Flag("to_redhand") == 1)
			listEnding = E_SOLD;

		super.OnEnding(gm, run, listEnding);
		int ending = E_M_CLOSED;
		switch (rule)
		{
			case R_WORKERS: ending = E_M_FAR; break;
			case R_CARDS: ending = E_M_COUNTED; break;
			case R_ARRANGEMENT: ending = E_M_PAID; break;
			default:
			{
				if (read)
					ending = E_M_EVERY;

				break;
			}
		}

		run.SetFlag("ending", ending);
		RZ_MajorHelp.SetWorld(gm, KEY + ".ending", ending - E_M_EVERY);
		ApplyEnding(gm, run, ending, read, fallback);
		ActEnd(gm, run, A_SECOND, ending - E_M_EVERY, EndingLine(ending));
		return S_WEEK_WAIT;
	}

	//------------------------------------------------------------------------------------------------
	protected static string EndingLine(int ending)
	{
		switch (ending)
		{
			case E_M_EVERY: return "Every Name. The houses read the List, crossed names and all, and share the pot.";
			case E_M_CLOSED: return "The Closed Book. The houses share the pot. Nobody read the List.";
			case E_M_FAR: return "The Far House. Every house that works will eat. The rest go where there's room.";
			case E_M_COUNTED: return "Counted. Every Haven house is on Bastion's census.";
		}

		return "Paid at the Door. The Haven eats the crossroads' tins.";
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending, bool read, bool fallback)
	{
		array<string> deciders = {};
		RZ_MajorHelp.Deciders(run, S_SITTING2, -1, deciders);
		RZ_MajorHelp.Deciders(run, S_SITTING1, -1, deciders);
		RZ_MajorBudget budget = RZ_MajorBudget.Get();
		string story = "";
		switch (ending)
		{
			case E_M_EVERY:
			{
				RZ_MajorHelp.PayIdents(gm, deciders, RZ_EFaction.SURVIVORS, 15, 0);
				RZ_MajorHelp.OpenPage(gm, "the Haven's List, read at supper");
				Headline(gm, run.Fill("Every Haven house read the List at supper, crossed names and all. {chair} read the first one."));
				story = "The Haven read every name on its List this season, the crossed ones too, and shared the pot until the gates opened.";
				break;
			}
			case E_M_CLOSED:
			{
				NudgeMorale(gm, RZ_EFaction.SURVIVORS, 3);
				RZ_MajorHelp.PayIdents(gm, deciders, RZ_EFaction.SURVIVORS, 10, 0);
				story = "Every Haven house shared the pot this season. Nobody read the List.";
				break;
			}
			case E_M_FAR:
			{
				if (read)
					NudgeMorale(gm, RZ_EFaction.SURVIVORS, -8);
				else
					NudgeMorale(gm, RZ_EFaction.SURVIVORS, 5);

				NudgeStrength(gm, RZ_EFaction.SURVIVORS, 3);
				RZ_MajorHelp.PayIdents(gm, deciders, RZ_EFaction.SURVIVORS, 10, 0);
				int farHouse = FarHouse(gm);
				RZ_Safehouse fh = RZ_HavenQuest.LiveHouse(gm, farHouse);
				string farTown = "the far house";
				if (fh)
					farTown = fh.m_sName;

				Headline(gm, string.Format("Every house that works will eat. The rest are going to %1, where there's room.", farTown));
				story = string.Format("The far house in %1 was last on every list this season.", farTown);
				break;
			}
			case E_M_COUNTED:
			{
				NudgeMorale(gm, RZ_EFaction.SURVIVORS, -5);
				NudgeMorale(gm, RZ_EFaction.MILITARY, 5);
				NudgeStrength(gm, RZ_EFaction.MILITARY, 3);
				RZ_MajorHelp.PayIdents(gm, deciders, RZ_EFaction.MILITARY, 15, 0);
				if (budget)
					budget.SetContractWeight("RZ_Mission_Requisition", 0.8);

				Headline(gm, "Bastion read the Haven's names to Command last night. No reply.");
				story = "Bastion's call reads \"Civilians sheltered. List to follow. Over.\"";
				break;
			}
			default:
			{
				NudgeMorale(gm, RZ_EFaction.SURVIVORS, -5);
				NudgeMorale(gm, RZ_EFaction.BANDITS, 5);
				NudgeStrength(gm, RZ_EFaction.BANDITS, 3);
				RZ_MajorHelp.PayIdents(gm, deciders, RZ_EFaction.BANDITS, 15, 0);
				if (budget)
					budget.SetContractWeight("RZ_Mission_QuietArrangement", 1.2);

				Headline(gm, "The Haven's houses ate Red Hand tins this week. Nobody at supper asked what they cost.");
				story = "The Haven ate Red Hand tins this season. Everyone knows what they cost now.";
				break;
			}
		}

		RZ_Majors.SeasonStory(RZ_EFaction.SURVIVORS, KEY, ending, RZ_MajorHelp.World(gm, "haven.rule"), story, deciders, fallback);
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterWeek(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		if (outcome == OUT_WEEK_HELD)
		{
			NudgeMorale(gm, RZ_EFaction.SURVIVORS, 5);
			RZ_MajorHelp.SetWorld(gm, KEY + ".held", 1);
			ActEnd(gm, run, A_WEEK, 1, "A week on, it held.");
		}
		else
		{
			ActEnd(gm, run, A_WEEK, 0, "A week on, the numbers stand.");
		}

		return S_GATE_WAIT;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterGate(RZ_GameModeComponent gm, RZ_QuestRun run, bool fallback)
	{
		ReleaseEvening(S_GATE);
		RZ_MajorHelp.SetWorld(gm, KEY + ".coda", 1);
		string line = "{chair} read the List at the last bend of the north road. Nobody behind the concrete answered.";
		if (fallback)
			line = "{chair} read the List at the north road with the houses' people and no one else. It's pinned on every house board now.";

		return Finish(run, run.Flag("ending", E_M_CLOSED), line);
	}

	//------------------------------------------------------------------------------------------------
	//! The major's endings were applied at the second sitting; H1's endings come through it.
	override void OnEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
		if (ending >= E_M_EVERY)
			return;

		super.OnEnding(gm, run, ending);
	}

	// ================================================================ Defaults
	//------------------------------------------------------------------------------------------------
	override int ActDefault(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_MajorAct act)
	{
		switch (act.m_iId)
		{
			case A_FAMILY:
			{
				// A Merciful or Idealist head takes all four; any other turns two away
				RZ_Leader head = RZ_BastionQuest.Leader(gm, run.Flag("family_head_id", -1));
				int outcome = OUT_TURNED;
				if (head && (head.HasTrait(RZ_ELeaderTrait.MERCIFUL) || head.HasTrait(RZ_ELeaderTrait.IDEALIST)))
					outcome = OUT_ALL_IN;

				return AfterFamily(gm, run, outcome, true);
			}
			case A_FIRST: return AfterFirst(gm, run, -1, true);
			case A_LIST:
			{
				// Knowledge only: the open stages close
				RZ_MajorHelp.SetWorld(gm, "list.truth", 0);
				ActEnd(gm, run, A_LIST, 0, "");
				return S_HUNGRY_WAIT;
			}
			case A_HUNGRY: return AfterHungry(gm, run, -1, true);
			case A_SECOND: return AfterSecond(gm, run, -1, true);
			case A_WEEK: return AfterWeek(gm, run, OUT_WEEK);
			case A_GATE: return AfterGate(gm, run, true);
		}

		return S_GATE_WAIT;
	}

	// ================================================================ Voice
	//------------------------------------------------------------------------------------------------
	override string BetweenLine(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		return "\"Every house is counting its pantry twice. The council will sit again before long.\"";
	}

	//------------------------------------------------------------------------------------------------
	override string SeasonState(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		int rule = RZ_MajorHelp.World(gm, "haven.rule");
		string text = "";
		if (rule > 0)
			text = "The rule: " + RuleName(rule - 1) + ".";

		RZ_QuestVote vote = new RZ_QuestVote(run, "second");
		if (vote.Count() == 0)
			vote = new RZ_QuestVote(run, "first");

		if (vote.Count() > 0 && vote.Result() < 0)
		{
			array<string> names = {RuleName(R_POT), RuleName(R_WORKERS), RuleName(R_CARDS), RuleName(R_ARRANGEMENT)};
			text += "\n" + vote.LeanLines(names);
		}

		return text;
	}
}
