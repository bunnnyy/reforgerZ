//! The Well Story (the Red Hand; R3 in docs/QUESTLINES.md, built as in docs/MISSIONS.md,
//! "Questlines"). Nobody in the Red Hand says what happens to a place that doesn't pay. They tell
//! the story about the well. A crew boss with a village that has stopped paying wants the story
//! told, and if that doesn't work, he wants it to stop being a story. The boss says the story
//! works best untold: you don't kill the cow you milk.
//!
//! The village: the safehouse H2 left standing alone (door.shut) or on Bastion's census
//! (door.bastion), named by door.house, else the Haven safehouse nearest the crew boss, else a quiet village under 30
//! infection. Its well: a real one if the town has one, else the well prop (Puzzles > Well).
//!
//! 1. Collect (mission): the keeper won't pay. Ask, or lean on them (+5 Haven Infamy; their people
//!    fight, with the survivor group prefab).
//! 2. Why they stopped (mission): ask the council head (Known with the Haven): they're short, or
//!    they pay Bastion now. Or pay their dues yourself (20 Red Hand Favor): it ends at The story
//!    worked.
//! 3. Tell the story (mission, dusk): at the well, to whoever's there. 60% they pay (★ Ex-Con:
//!    always; 40% after leaning on them). Or warn them instead: two people on the well, +5 Haven Fame.
//! 4. The crew boss's order (mission): put something in the well (after dark; seen while warned,
//!    noticed or with shots fired: +15 Haven Infamy at once); fake it (★ Lab Tech/Nurse know what's
//!    harmless, others get it wrong a third of the time: +5 infection); take a water sample to
//!    Halcyon (it tests the water and seals the well); or stand guard when the crew comes to do it
//!    themselves (the Red Hand group prefab). Nobody does anything: the crew does it.
//! 5. A session later (wait), then 6. The boss hears (mission, Trusted): own it, blame the crew boss
//!    (he goes in the pit), or say nothing.
//!
//! Endings: The story worked, The well was done, Faked, Stood up.
class RZ_Quest_WellStory : RZ_Questline
{
	static const int S_COLLECT = 0;
	static const int S_WHY = 1;
	static const int S_STORY = 2;
	static const int S_ORDER = 3;
	static const int S_LATER = 4;
	static const int S_BOSS = 5;

	static const int OUT_REFUSED = 10;		// stage 1
	static const int OUT_REASON = 11;		// stage 2: asked why
	static const int OUT_DUES = 12;			// stage 2: paid their dues
	static const int OUT_PAID = 13;			// stage 3: the story worked
	static const int OUT_UNPAID = 14;		// stage 3: told, and they still didn't pay
	static const int OUT_WARNED = 15;		// stage 3: warned them instead
	static const int OUT_DONE = 16;			// stage 4
	static const int OUT_FAKED = 17;
	static const int OUT_TESTED = 18;
	static const int OUT_HELD = 19;
	static const int OUT_OWN = 20;			// stage 6
	static const int OUT_BLAME = 21;
	static const int OUT_QUIET = 22;

	static const int E_WORKED = 0;
	static const int E_DONE = 1;
	static const int E_FAKED = 2;
	static const int E_STOOD = 3;

	static const float OVERRUN = 75;		// a village this infected has nobody left to tell
	static const float FOUND_OUT = 0.25;	// a fake comes out to the crew boss

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		Info("well", "The Well Story", RZ_EFaction.BANDITS);

		RZ_QuestStage collect = MissionStage(S_COLLECT, "Collect", "{village} has stopped paying {crew_short}'s crew in {crew_place}. {crew_short} wants it collected.{shelter_line}");
		collect.GiveUp(4, "Nobody went to collect from {village}. {crew_short} has stopped asking, which is worse.");
		collect.Visits(2);

		RZ_QuestStage why = MissionStage(S_WHY, "Why they stopped", "The keeper in {village} won't pay. Find out why from the council head, or pay their dues yourself and let the story end there.");
		why.GiveUp(4, "Nobody asked {village} why it stopped paying. The story will be told anyway.");

		RZ_QuestStage story = MissionStage(S_STORY, "Tell the story", "Tell the story about the well, at the well in {village}, at dusk, to whoever's there. Or warn them instead.");
		story.GiveUp(12, "Nobody told {village} the story about the well. {crew_short} has stopped waiting for someone to.");

		RZ_QuestStage order = MissionStage(S_ORDER, "The crew boss's order", "{village} didn't pay. {crew_short} wants something in the well. Do it after dark, unseen; fake it; get Halcyon to test the water and seal it; or stand guard when the crew comes to do it themselves.");
		order.GiveUp(4, "{crew_short}'s order went cold, and so did the story about {village}.");

		RZ_QuestStage later = WaitStage(S_LATER, "A session later", "Whatever happened at the well in {village}, the boss will hear about it.");
		later.Wait(120);

		RZ_QuestStage boss = MissionStage(S_BOSS, "The boss hears", "The boss has sent for you. The boss says the story works best untold: you don't kill the cow you milk. Own what happened at {village}, blame {crew_short}, or say nothing.");
		boss.GiveUp(6, "The boss stopped waiting for an answer about {village}, and decided alone.");
	}

	//------------------------------------------------------------------------------------------------
	//! A crew boss (Ruthless or Greedy ones twice as likely) and the village that won't pay him.
	override bool CanStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (!RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.OBJECT) || !gm.GetLeaders())
			return false;

		RZ_Leader boss = gm.GetLeaders().GetStronghold(RZ_EFaction.BANDITS);
		int traits = (1 << RZ_ELeaderTrait.RUTHLESS) | (1 << RZ_ELeaderTrait.GREEDY);
		RZ_Leader crew = RZ_RedHandQuest.PickAny(gm, RZ_EFaction.BANDITS, traits);
		if (!boss || !crew)
			return false;

		if (!PickVillage(gm, run, crew))
			return false;

		RZ_RedHandQuest.Keep(run, "boss", boss);
		RZ_RedHandQuest.Keep(run, "crew", crew);
		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		run.SetText("keeper", RZ_NoteText.FullName(rng));
		RZ_RedHandQuest.SetWorld(gm, "well.town", run.Flag("town") + 1);
		RZ_RedHandQuest.SetWorld(gm, "well.house", run.Flag("house", -1) + 1);
		run.SetPlace(RZ_RedHandQuest.PosOf(run, "well"), run.Text("village"));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! H2's house (door.house, when door.shut or door.bastion is set), else the Haven safehouse
	//! nearest the crew boss, else a quiet village under 30 infection 0.6-4 km away.
	protected bool PickVillage(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_Leader crew)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		int house = -1;
		string shelter = "";
		if (houses)
		{
			// H2's house (door.house: its safehouse index + 1) when it stood alone or went to Bastion
			int door = RZ_RedHandQuest.World(gm, "door.house") - 1;
			if (houses.Get(door) && RZ_RedHandQuest.HasWorld(gm, "door.bastion"))
			{
				house = door;
				shelter = " The house there is on Bastion's census now, and pays Bastion instead.";
			}
			else if (houses.Get(door) && RZ_RedHandQuest.HasWorld(gm, "door.shut"))
			{
				house = door;
				shelter = " The house there stood up to the Red Hand once already, and put its barricades up.";
			}
			else
			{
				house = RZ_LeaderJob.SafehouseNear(gm, crew.m_vPos, 0, 6000, -1);
			}
		}

		RZ_Safehouse h = RZ_Questline.GetHouse(gm, house);
		int town = -1;
		vector center;
		if (h && h.m_iSettlement >= 0)
		{
			town = h.m_iSettlement;
			center = h.m_vPos;
		}
		else
		{
			house = -1;
			town = RZ_Mission.PickSettlementAway(gm, crew.m_vPos, 600, 4000, 0, 30);
		}

		RZ_Settlement s = gm.GetSettlement(town);
		if (!s || s.m_fInfection >= OVERRUN)
			return false;

		if (house < 0)
			center = s.m_vCenter;

		run.SetFlag("house", house);
		run.SetFlag("town", town);
		run.SetText("village", s.m_sName);
		run.SetText("shelter_line", shelter);

		// The well: a real one in the town if there is one
		array<string> kinds = {"well"};
		RZ_Landmark real = RZ_LandmarkFinder.FindNearest(s.m_vCenter, s.m_fRadius, kinds);
		vector well = RZ_Mission.LandPointNear(center, 10, Math.Max(20, s.m_fRadius * 0.3));
		if (real)
		{
			well = RZ_Util.RandomPointInRing(real.m_vPos, 1.5, 2.5);
			run.SetFlag("real_well", 1);
		}

		RZ_RedHandQuest.KeepPos(run, "well", well);
		RZ_RedHandQuest.KeepPos(run, "door", RZ_Mission.LandPointNear(center, 5, 12));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_Safehouse House(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		return RZ_Questline.GetHouse(gm, run.Flag("house", -1));
	}

	//------------------------------------------------------------------------------------------------
	//! The crew boss: the same one, or whoever has the chair now (the grudge goes with the seat).
	//! False if the crew's camp itself is gone.
	static bool CheckCrew(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (!RZ_RedHandQuest.Seat(gm, run, "crew"))
			return false;

		string old = run.Text("crew");
		RZ_Leader fresh = RZ_RedHandQuest.Successor(gm, run, "crew");
		if (fresh)
			run.Note(string.Format("%1 is out of the chair in %2. %3 has it now, and the same grudge against {village}.", old, fresh.m_sPlace, fresh.m_sName));

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Still somebody at the well to tell a story to.
	static bool VillageStands(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_Settlement s = gm.GetSettlement(run.Flag("town", -1));
		return s && s.m_fInfection < OVERRUN;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		run.Note("{village} has stopped paying {crew_short}'s crew. Nobody in the Red Hand says what happens to a place that doesn't pay. They tell the story about the well.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnStageBegin(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		CheckCrew(gm, run);
		if (stage == S_BOSS)
		{
			RZ_Leader boss = RZ_RedHandQuest.Seat(gm, run, "boss");
			if (boss)
				run.SetPlace(boss.m_vPos, boss.m_sPlace);

			return;
		}

		run.SetPlace(RZ_RedHandQuest.PosOf(run, "well"), run.Text("village"));
	}

	//------------------------------------------------------------------------------------------------
	override bool IsReady(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		// Once the order is carried out (or not), the story is decided: only the boss is left
		if (stage > S_ORDER)
			return true;

		return CheckCrew(gm, run) && VillageStands(gm, run);
	}

	//------------------------------------------------------------------------------------------------
	override RZ_Mission MakeMission(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		switch (stage)
		{
			case S_COLLECT: return new RZ_QM_WellCollect();
			case S_WHY: return new RZ_QM_WellWhy();
			case S_STORY:
			{
				// The story is told at dusk
				if (!RZ_RedHandQuest.IsDusk())
					return null;

				return new RZ_QM_WellTell();
			}
			case S_ORDER: return new RZ_QM_WellOrder();
			case S_BOSS: return new RZ_QM_WellBoss();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override int Next(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		if (stage < S_ORDER && !CheckCrew(gm, run))
			return Fizzle(run, "The crew in {crew_place} is gone, and nobody else is asking {village} for anything.");

		if (stage < S_ORDER && !VillageStands(gm, run))
			return Fizzle(run, "Nobody's left at the well in {village} to tell a story to.");

		switch (stage)
		{
			case S_COLLECT:
			{
				if (outcome == OUT_REFUSED)
				{
					run.Note("The keeper won't pay. Not this month.");
					return S_WHY;
				}

				run.Note("Nobody went to collect. {crew_short} will send someone else.");
				return S_COLLECT;
			}
			case S_WHY:
			{
				if (outcome == OUT_DUES)
				{
					run.SetFlag("dues", 1);
					run.Note("Somebody paid {village}'s dues out of their own pocket. {crew_short} doesn't care who paid.");
					return Finish(run, E_WORKED, "The story worked, without being told. {village} is paid up, and nobody in the Red Hand asks who paid.");
				}

				if (outcome == OUT_REASON)
					run.Note(ReasonLine(gm, run));
				else
					run.Note("Nobody found out why {village} stopped paying. The story will be told anyway.");

				return S_STORY;
			}
			case S_STORY:
			{
				if (outcome == OUT_PAID)
				{
					run.Note("They heard the story at the well, and they paid.");
					return Finish(run, E_WORKED, "The story worked. {village} paid, and nobody was hurt. The well is just a story again.");
				}

				if (outcome == OUT_WARNED)
				{
					run.SetFlag("warned", 1);
					run.Note("Somebody warned {village} instead of telling the story. There are two people on the well now, day and night.");
				}
				else
				{
					run.Note("{village} heard the story and still didn't pay.");
				}

				return S_ORDER;
			}
			case S_ORDER:
			{
				int branch = outcome;
				if (outcome != OUT_DONE && outcome != OUT_FAKED && outcome != OUT_TESTED && outcome != OUT_HELD)
				{
					// Nobody stood in the way: the crew did it themselves
					branch = OUT_DONE;
					run.SetFlag("crew_did_it", 1);
					RZ_RedHandQuest.SetWorld(gm, "well.done", 1);
					run.Note("Nobody stood in the way, so {crew_short}'s crew went to the well in {village} themselves, after dark.");
				}

				run.SetBranch(branch);
				run.Note(BranchLine(run));
				return S_LATER;
			}
			case S_LATER:
				return S_BOSS;
			case S_BOSS:
			{
				if (outcome == OUT_OWN)
				{
					run.SetFlag("owned", 1);
					run.Note("Somebody owned what happened at {village}, to the boss's face.");
				}
				else if (outcome == OUT_BLAME)
				{
					run.SetFlag("blamed", 1);
					run.Note("Somebody told the boss it was {crew_short}'s doing. The boss's pit is on tonight.");
				}
				else
				{
					run.Note("Nobody said anything to the boss. The boss decided alone.");
				}

				return Ending(run);
			}
		}

		return Fizzle(run, "");
	}

	//------------------------------------------------------------------------------------------------
	protected int Ending(RZ_QuestRun run)
	{
		switch (run.m_iBranch)
		{
			case OUT_DONE: return Finish(run, E_DONE, "The well was done. Nobody in {village} is drinking from it, and the Red Hand has one story fewer to tell.");
			case OUT_FAKED: return Finish(run, E_FAKED, "Faked. Something dead in a bucket, a story that spread anyway, and nobody hurt. Probably.");
		}

		return Finish(run, E_STOOD, "Stood up. {village} kept its well, and the Red Hand knows who stood beside it.");
	}

	//------------------------------------------------------------------------------------------------
	//! Why they stopped: short of food, or paying Bastion now.
	static string ReasonLine(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.Flag("reason") == 2)
			return "They pay Bastion now. A Bastion patrol comes by twice a week and doesn't ask for tins, only names.";

		return "They're short. The house can feed its own people or {crew_short}'s crew, not both.";
	}

	//------------------------------------------------------------------------------------------------
	static string BranchLine(RZ_QuestRun run)
	{
		switch (run.m_iBranch)
		{
			case OUT_DONE: return "Something went in the well at {village}.";
			case OUT_FAKED: return "Something dead went in a bucket at the well in {village}, and the story went round the houses.";
			case OUT_TESTED: return "Halcyon tested the water at {village} and sealed the well. It's clean, and it's theirs.";
		}

		return "When {crew_short}'s crew came to do it themselves, somebody was standing at the well.";
	}

	//------------------------------------------------------------------------------------------------
	override void OnEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
		int house = run.Flag("house", -1);
		int town = run.Flag("town", -1);
		bool blamed = run.Flag("blamed") == 1;
		RZ_Leader crew = RZ_RedHandQuest.Seat(gm, run, "crew");
		RZ_Leader boss = RZ_RedHandQuest.Seat(gm, run, "boss");
		if (ending == E_WORKED)
		{
			ChangeStanding(gm, run, RZ_EFaction.BANDITS, 10, 0);
			RZ_RedHandQuest.RemembersAll(boss, run, RZ_Leaders.MEM_HELPED);
			if (run.Flag("dues") != 1)
				ChangeFood(gm, house, -2);

			SpreadRumour(gm, run.Fill("\"Ask the Red Hand what happened to the well in the village that didn't pay. Go on. They'll tell you a story.\""), 8);
			return;
		}

		if (blamed && crew && crew.IsPresent())
			ReplaceLeader(gm, crew.m_iId, run.Fill(string.Format("%1 went in the boss's pit last night, for the business at {village}. The crew outside %2 has a new boss.", crew.m_sName, crew.m_sPlace)));

		if (ending == E_DONE)
		{
			NudgeInfection(gm, town, 15);
			RZ_Safehouse h = House(gm, run);
			if (h)
			{
				int loss = Math.MinInt(3, h.m_iPopulation - RZ_Safehouses.UPKEEP_FLOOR);
				if (loss > 0)
					ChangePopulation(gm, house, -loss);
			}

			// A bad deal: the boss takes it out on the crew boss (unless someone else already put him in the pit)
			if (!blamed && crew && crew.IsPresent())
				ReplaceLeader(gm, crew.m_iId, string.Format("The boss has taken the crew outside %1 off %2. A bad deal, the boss says, without saying which.", crew.m_sPlace, crew.m_sName));

			if (run.Flag("crew_did_it") != 1)
			{
				int infamy = 10;
				if (blamed)
					infamy = 0;

				ChangeStanding(gm, run, RZ_EFaction.BANDITS, 5, infamy, RZ_EQuestApproach.SNEAK);
				if (run.Flag("seen") == 1)
					ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 0, 15, RZ_EQuestApproach.SNEAK);
			}

			Headline(gm, run.Fill("Nobody in {village} is drinking from the well."));
			SpreadRumour(gm, run.Fill("\"They say the Red Hand put something in the well at {village}. They say it like a joke. It isn't.\""), 12);
			return;
		}

		if (ending == E_FAKED)
		{
			ChangeFood(gm, house, -1);
			ChangeStanding(gm, run, RZ_EFaction.BANDITS, 10, 0, RZ_EQuestApproach.BETRAY);
			if (!blamed && run.Flag("owned") != 1 && Math.RandomFloat01() < FOUND_OUT)
			{
				ChangeStanding(gm, run, RZ_EFaction.BANDITS, 0, 10, RZ_EQuestApproach.BETRAY);
				Headline(gm, run.Fill("{crew_short} found out what really went in the bucket at {village}, and is telling everyone who did it."), false);
			}

			SpreadRumour(gm, run.Fill("\"Somebody found a dead dog in the well bucket at {village}. Red Hand, they say. The story spreads itself.\""), 8);
			return;
		}

		// Stood up
		ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 10, 0);
		ChangeStanding(gm, run, RZ_EFaction.BANDITS, 0, 10, RZ_EQuestApproach.SIDE);
		if (run.m_iBranch == OUT_TESTED)
			NudgeInfection(gm, town, -10);

		if (!blamed)
			RZ_RedHandQuest.SwingLoyalty(gm, RZ_RedHandQuest.Same(gm, run, "crew"), -RZ_RedHandQuest.SWING);

		Headline(gm, run.Fill("{village} kept its well. The Red Hand crew that wanted it hasn't been back."));
		SpreadRumour(gm, run.Fill("\"Somebody stood at the well in {village} when the Red Hand came. The crew went home.\""), 8);
	}

	//------------------------------------------------------------------------------------------------
	override string JournalText(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		string text = super.JournalText(gm, run);
		if (run.m_iStage == S_ORDER && run.Flag("warned") == 1)
			text += run.Fill(" There are two of {village}'s people on the well now.");

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override string Rumour(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.m_iStage >= S_WHY && run.m_iStage <= S_ORDER)
			return "\"Ask the Red Hand about the well in {village}. Go on. They'll tell you a story.\"";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The village's council head talks about it; after a fake, someone who took part can tell them it
	//! was harmless (+8 Haven Fame, once).
	override string OnLeaderTalk(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, int leaderId)
	{
		RZ_Leader l = gm.GetLeaders().Get(leaderId);
		if (!l || l.m_eRole != RZ_ELeaderRole.SAFEHOUSE || l.m_iSafehouse != run.Flag("house", -1))
			return "";

		if (run.m_iBranch == OUT_FAKED && run.m_iStage >= S_LATER && run.Flag("told") != 1 && run.PlayerTookPart(gm, playerId))
		{
			run.SetFlag("told", 1);
			run.Reward(gm, playerId, RZ_EFaction.SURVIVORS, 8, 0, string.Format("%1 hears it was only something dead in a bucket, and sits down very suddenly.", l.m_sShort));
			return string.Format("You tell %1 what really went in the bucket. \"We've been boiling it for days.\" A long breath. \"Thank you for telling us.\"", l.m_sShort);
		}

		if (run.m_iStage <= S_STORY)
			return string.Format("%1 glances at the well. \"The Red Hand's been telling stories again. We've heard that one.\"", l.m_sShort);

		return "";
	}
}

//------------------------------------------------------------------------------------------------
//! The Well Story, stage 1 (Red Hand contract, Known, not Elders; voiced by the crew boss): the keeper
//! won't pay. Ask, or lean on them: +5 Haven Infamy, and with the survivor group prefab their
//! people come out to fight (they still won't pay once it's over).
class RZ_QM_WellCollect : RZ_QuestMission
{
	protected ref RZ_UnitWatch m_Locals = new RZ_UnitWatch();
	protected int m_iLeaner = -1;
	protected int m_iTalk = -1;

	protected static const int LEAN_INFAMY = 5;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 8, 10);
		m_Req.Deny("elder");
		m_fTimeLeft = 2400;
		m_fParticipationRadius = 0;
		RZ_Leader crew = RZ_RedHandQuest.Same(gm, Run(gm), "crew");
		if (crew)
			m_iLeaderId = crew.m_iId;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run || !RZ_RedHandQuest.HasPos(run, "door"))
			return false;

		m_vPos = RZ_RedHandQuest.PosOf(run, "door");
		m_iSettlement = run.Flag("town", -1);
		m_iTalk = AddClue(QText(gm, "keeper"), QFill(gm, "{keeper} doesn't get up from the step. \"I know who sent you, and I know what he wants. We haven't got it. Tell him that.\""));
		m_sTitle = "The Well Story: collect";
		m_sBrief = QFill(gm, "{village} hasn't paid this month. Go and remind {keeper} what my protection is worth. Politely, the first time.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		return SpawnPerson(gm, m_vPos, 0, m_iTalk, "Talk to " + QText(gm, "keeper"), "Ask for what's owed|Lean on them") != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0 || m_bResolved || m_iLeaner > 0)
			return;

		if (option == 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "\"No.\" {keeper} folds their arms. Faces in the doorways. \"Not this month. Tell him that.\""));
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_WellStory.OUT_REFUSED, "asked for what's owed");
			return;
		}

		if (option != 1)
			return;

		m_iLeaner = playerId;
		RZ_QuestApproach.SideWith(gm, playerId, -1, RZ_EFaction.SURVIVORS, 0, LEAN_INFAMY);
		SetQFlag(gm, "leaned", 1);
		ResourceName prefab = gm.GetSurvivorGroupPrefab();
		if (!prefab.IsEmpty() && m_Locals.Spawn(prefab, LandPointNear(m_vPos, 8, 15)))
		{
			TrackProp(m_Locals.m_Unit);
			RZ_Hostility.MarkFight(RZ_EFaction.SURVIVORS, m_Locals.m_Unit, 120);
			Remember(gm, playerId, RZ_EQuestApproach.FORCE, "leaned on the keeper");
			RZ_ContractHelp.NotifyNear(gm, m_vPos, 120, m_sTitle, string.Format("The house's people come out with whatever they could grab. They're not paying. Not while they're standing. (+%1 Haven Infamy)", LEAN_INFAMY));
			return;
		}

		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, string.Format("You lean in close and say it quietly. {keeper} goes white, and still says no. \"You can break the door. You can't make us have what we haven't got.\" (+%1 Haven Infamy)", LEAN_INFAMY)));
		Resolve(gm, playerId, RZ_EQuestApproach.FORCE, RZ_Quest_WellStory.OUT_REFUSED, "leaned on the keeper");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved || m_iLeaner <= 0 || !m_Locals.IsDead(dt))
			return;

		NotifyParticipants(gm, m_sTitle, QFill(gm, "It's over. {keeper} wipes blood off their lip and still says no. \"Tell him. Tell him all of it.\""));
		Resolve(gm, m_iLeaner, RZ_EQuestApproach.FORCE, RZ_Quest_WellStory.OUT_REFUSED, "leaned on the keeper");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Well Story: nobody went to collect from {village}.");
	}
}

//------------------------------------------------------------------------------------------------
//! The Well Story, stage 2 (Red Hand contract, Known, not Elders): the council head (held while a
//! stand-in speaks for them; the house's keeper without one). Ask why (Known with the Haven): they're
//! short, or they pay Bastion now. Or pay their dues yourself (20 Red Hand Favor).
class RZ_QM_WellWhy : RZ_RedHandQuestMission
{
	protected int m_iTalk = -1;
	protected string m_sWho;

	protected static const int DUES_FAVOR = 20;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 8, 10);
		m_Req.Deny("elder");
		m_fTimeLeft = 2400;
		m_fParticipationRadius = 0;
		RZ_Leader crew = RZ_RedHandQuest.Same(gm, Run(gm), "crew");
		if (crew)
			m_iLeaderId = crew.m_iId;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run || !RZ_RedHandQuest.HasPos(run, "door"))
			return false;

		// Why: short of food, or paying Bastion now (H2's door.bastion, or a Bastion seat close by)
		RZ_Safehouse h = RZ_Quest_WellStory.House(gm, run);
		int reason = 1;
		if (h && RZ_RedHandQuest.HasWorld(gm, "door.bastion") && RZ_RedHandQuest.World(gm, "door.house") - 1 == run.Flag("house", -1))
			reason = 2;
		else if (h && h.m_bShort)
			reason = 1;
		else if (gm.GetLeaders() && gm.GetLeaders().FindNearest(RZ_EFaction.MILITARY, RZ_RedHandQuest.PosOf(run, "door"), 1500))
			reason = 2;

		run.SetFlag("reason", reason);
		m_sWho = QText(gm, "keeper");
		if (h)
		{
			int council = RZ_Questline.LeaderNear(gm, RZ_EFaction.SURVIVORS, h.m_vPos);
			RZ_Leader l = gm.GetLeaders().Get(council);
			if (l && l.IsPresent())
			{
				m_sWho = l.m_sShort;
				if (l.m_iHeldBy < 0)
					HoldLeader(gm, l.m_iId);
			}
		}

		AddSpot(RZ_RedHandQuest.PosOf(run, "door"), m_sWho + " in " + QText(gm, "village"));
		m_iTalk = AddClue(m_sWho, QFill(gm, "\"The keeper told you no, and the keeper was right.\" They don't sit down, and they don't ask you to."));
		m_vPos = m_aSpots[0];
		m_iSettlement = run.Flag("town", -1);
		m_sTitle = "The Well Story: why they stopped";
		m_sBrief = QFill(gm, "{village} says no. Nobody says no to me without a reason. Find out what it is. Or pay what they owe yourself, if you're feeling soft. I don't care who pays.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void SpawnSpot(RZ_GameModeComponent gm, int spot)
	{
		string choices = string.Format("Ask why they stopped paying|Pay their dues yourself (%1 Red Hand Favor)", DUES_FAVOR);
		SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_aSpots[spot], 0, "", choices, m_iTalk, "Talk to " + m_sWho);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		TickSpots(gm, players);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		RZ_QuestRun run = Run(gm);
		if (!run || objIdx != 0 || m_bResolved)
			return;

		if (option == 0)
		{
			if (!RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.SURVIVORS), RZ_ETier.KNOWN))
			{
				Remember(gm, playerId, RZ_EQuestApproach.TALK, "asked why");
				gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 looks at you the way the Haven looks at strangers who come with the Red Hand's messages. \"That's the house's business.\"", m_sWho));
				return;
			}

			gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 tells you, because you're not a stranger here. ", m_sWho) + QFill(gm, RZ_Quest_WellStory.ReasonLine(gm, run)));
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_WellStory.OUT_REASON, "asked why they stopped");
			return;
		}

		if (option == 1 && TryBribe(gm, playerId, RZ_EFaction.BANDITS, DUES_FAVOR))
		{
			RZ_RedHandQuest.SetWorld(gm, "well.paid", 1);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("You count it out yourself. %1 stares at it, then at you. \"Why?\" You don't have a good answer. The crew boss won't ask for one.", m_sWho));
			Resolve(gm, playerId, RZ_EQuestApproach.BRIBE, RZ_Quest_WellStory.OUT_DUES, "paid their dues");
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Well Story: nobody found out why {village} stopped paying.");
	}
}

//------------------------------------------------------------------------------------------------
//! The Well Story, stage 3 (Red Hand contract, Known, not Elders; only at dusk): at the well, to
//! whoever's there. Tell the story: TELL_CHANCE they pay (★ Ex-Con: always; LEANED_CHANCE after
//! someone leaned on them). Or warn them: two people on the well, +5 Haven Fame, no pay.
class RZ_QM_WellTell : RZ_QuestMission
{
	protected vector m_vWell;
	protected int m_iTalk = -1;

	protected static const float TELL_CHANCE = 0.6;
	protected static const float LEANED_CHANCE = 0.4;
	protected static const int STORY_FAVOR = 15;
	protected static const int WARN_FAME = 5;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 10, 15);
		m_Req.Deny("elder");
		m_Req.Bonus("excon");
		m_fTimeLeft = 2700;
		m_fParticipationRadius = 0;
		RZ_Leader crew = RZ_RedHandQuest.Same(gm, Run(gm), "crew");
		if (crew)
			m_iLeaderId = crew.m_iId;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run || !RZ_RedHandQuest.HasPos(run, "well"))
			return false;

		m_vWell = RZ_RedHandQuest.PosOf(run, "well");
		m_vPos = RZ_Util.RandomPointInRing(m_vWell, 2, 4);
		m_iSettlement = run.Flag("town", -1);
		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		string woman = RZ_RedHandQuest.WomanName(rng);
		run.SetText("drawer", woman);
		m_iTalk = AddClue(woman, QFill(gm, "{drawer} is hauling up a bucket, and doesn't stop. \"You're the Red Hand's. Go on, then. Say it.\""));
		m_sTitle = "The Well Story: tell the story";
		m_sBrief = QFill(gm, "Go to the well in {village} at dusk, when they come for water. Tell them the story. You know the one. Let them finish it themselves.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (run && run.Flag("real_well") != 1)
			SpawnProp(gm, RZ_EPuzzleProp.WELL, m_vWell);

		return SpawnPerson(gm, m_vPos, 0, m_iTalk, "Talk to " + QText(gm, "drawer"), "Tell them the story about the well|Warn them instead") != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		RZ_QuestRun run = Run(gm);
		if (!run || objIdx != 0 || m_bResolved)
			return;

		if (option == 0)
		{
			float chance = TELL_CHANCE;
			if (run.Flag("leaned") == 1)
				chance = LEANED_CHANCE;

			if (HasBonus(gm, playerId))
				chance = 1;

			string told = "You tell it the way it was told to you: a village, a well, a crew that wasn't paid. You stop before the end. You let them finish it.";
			if (Math.RandomFloat01() < chance)
			{
				RZ_RedHandQuest.SetWorld(gm, "well.paid", 1);
				run.Reward(gm, playerId, RZ_EFaction.BANDITS, 0, STORY_FAVOR, "The crew boss counts what came from the village. All of it.");
				gm.NotifyPlayer(playerId, m_sTitle, told + QFill(gm, " {drawer} puts the bucket down very carefully. In the morning there's a sack at the crew's door."));
				Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_WellStory.OUT_PAID, "told the story, and they paid");
				return;
			}

			gm.NotifyPlayer(playerId, m_sTitle, told + QFill(gm, " {drawer} hears you out and goes back to hauling. In the morning there's nothing at the crew's door."));
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_WellStory.OUT_UNPAID, "told the story", false);
			return;
		}

		if (option == 1)
		{
			RZ_RedHandQuest.SetWorld(gm, "well.warned", 1);
			run.Reward(gm, playerId, RZ_EFaction.SURVIVORS, WARN_FAME, 0, QFill(gm, "{village} heard who warned them."));
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You don't tell the story. You tell them the end of it, plainly, and who's asking. {drawer} sends for two of the men, and they don't go home that night."));
			Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_WellStory.OUT_WARNED, "warned them instead", false);
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Well Story: {village} didn't pay.");
	}
}

//------------------------------------------------------------------------------------------------
//! The Well Story, stage 4 (Red Hand contract, Known, not Elders; voiced by the crew boss): the order.
//! At the well (Puzzles > Well, else the puzzle object, or beside a real well): put it in (after
//! dark; seen if the village was warned, someone noticed you or shots were fired near: +15 Haven
//! Infamy at once), fake it (★ Lab Tech/Nurse know what's harmless; others get it wrong
//! WRONG_FAKE of the time: +5 infection), take a water sample (SAMPLE_SECONDS, ★ at once) to the
//! nearest Halcyon seat, or stand guard (the Red Hand group prefab: the crew comes CREW_DELAY later
//! and has to be driven off). Time up: the crew does it themselves.
class RZ_QM_WellOrder : RZ_RedHandQuestMission
{
	protected ref RZ_UnitWatch m_Crew = new RZ_UnitWatch();
	protected vector m_vWell;
	protected vector m_vLab;
	protected string m_sLab;
	protected bool m_bHasLab;
	protected int m_iWellSpot = -1;
	protected int m_iLabSpot = -1;
	protected int m_iSampler = -1;
	protected float m_fSample;
	protected bool m_bSample;
	protected int m_iGuard = -1;
	protected float m_fCrewIn = -1;
	protected int m_iTalk = -1;

	protected static const int OBJ_WELL = 0;
	protected static const int OBJ_LAB = 1;
	protected static const float WRONG_FAKE = 0.35;
	protected static const float FAKE_INFECTION = 5;
	protected static const int SEEN_INFAMY = 15;
	protected static const float SAMPLE_SECONDS = 20;
	protected static const float CREW_DELAY = 60;
	protected static const int HALCYON_FAME = 8;
	protected static const int HAVEN_FAME = 5;
	protected static const int HELD_INFAMY = 10;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 10, 15);
		m_Req.Deny("elder");
		m_Req.Bonus("labtech");
		m_Req.Bonus("nurse");
		m_fTimeLeft = 5400;
		m_fParticipationRadius = 0;
		RZ_Leader crew = RZ_RedHandQuest.Same(gm, Run(gm), "crew");
		if (crew)
			m_iLeaderId = crew.m_iId;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run || !RZ_RedHandQuest.HasPos(run, "well"))
			return false;

		m_vWell = RZ_RedHandQuest.PosOf(run, "well");
		m_iWellSpot = AddSpot(m_vWell, "The well in " + run.Text("village"));
		m_bHasLab = RZ_RedHandQuest.FactionSeat(gm, RZ_EFaction.INSTITUTE, m_vWell, m_vLab, m_sLab);
		m_Quiet.Watch(m_vWell, 80);
		m_iTalk = AddClue("The well", "Stone, waist high, a bucket on a chain. Whatever goes in here, everyone in the village drinks.");
		m_vPos = m_vWell;
		m_iSettlement = run.Flag("town", -1);
		m_sTitle = "The Well Story: the order";
		m_sBrief = QFill(gm, "{village} still hasn't paid. So now it stops being a story. Put something in their well, after dark, and don't let anyone see you do it.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool IsSpotOpen(int spot)
	{
		if (spot == m_iLabSpot)
			return m_bSample;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void SpawnSpot(RZ_GameModeComponent gm, int spot)
	{
		if (spot == m_iLabSpot)
		{
			int lab = AddClue("Halcyon", "\"A water sample? From where?\" The field lead's interest is very polite and very quick.");
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_aSpots[spot], OBJ_LAB, "Hand over the water sample", "", lab, "Talk to Halcyon's people");
			return;
		}

		RZ_QuestRun run = Run(gm);
		string uses = "Put it in the well (after dark)|Fake it: something dead in the bucket";
		if (m_bHasLab)
			uses += "|Take a water sample";
		else
			uses += "|";

		if (!gm.GetRedHandGroupPrefab().IsEmpty())
			uses += "|Stand guard at the well";

		if (run && run.Flag("real_well") == 1)
			SpawnObject(gm, RZ_EPuzzleProp.OBJECT, RZ_Util.RandomPointInRing(m_vWell, 1, 2), OBJ_WELL, uses, "", m_iTalk, "Look at the well");
		else
			SpawnObject(gm, RZ_EPuzzleProp.WELL, m_vWell, OBJ_WELL, uses, "", m_iTalk, "Look at the well");

		// Warned: two of the village's people on the well
		if (run && run.Flag("warned") == 1)
		{
			int guards = AddClue("On the well", "Two men with sticks and a lamp, taking turns to sleep. \"Evening. Thirsty?\"");
			SpawnClue(gm, RZ_EPuzzleProp.CIVILIAN, RZ_Util.RandomPointInRing(m_vWell, 3, 5), guards, "Talk to the men on the well");
			SpawnClue(gm, RZ_EPuzzleProp.CIVILIAN, RZ_Util.RandomPointInRing(m_vWell, 3, 5), guards, "Talk to the men on the well");
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved)
			return;

		TickSpots(gm, players);
		UpdateSample(gm, dt);
		UpdateGuard(gm, dt, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		RZ_QuestRun run = Run(gm);
		if (!run || m_bResolved)
			return;

		if (objIdx == OBJ_LAB)
		{
			if (value == 0 && m_bSample)
				Tested(gm, run, playerId);

			return;
		}

		if (objIdx != OBJ_WELL)
			return;

		if (value == 0)
		{
			DoIt(gm, run, playerId);
			return;
		}

		if (value == 1)
		{
			Fake(gm, run, playerId);
			return;
		}

		if (value == 2 && m_bHasLab && !m_bSample)
		{
			if (HasBonus(gm, playerId))
			{
				m_iSampler = playerId;
				Sampled(gm);
				return;
			}

			m_iSampler = playerId;
			m_fSample = 0;
			gm.NotifyPlayer(playerId, m_sTitle, "You fill a bottle slowly, the way Halcyon's people do it, and keep it out of the light. Stay at the well.");
			return;
		}

		if (value == 3 && m_iGuard < 0 && !gm.GetRedHandGroupPrefab().IsEmpty())
		{
			m_iGuard = playerId;
			m_fCrewIn = CREW_DELAY;
			Remember(gm, playerId, RZ_EQuestApproach.FORCE, "stood guard at the well");
			RZ_ContractHelp.NotifyNear(gm, m_vWell, 150, m_sTitle, QFill(gm, "You sit down on the well's edge. If {crew_short} wants it done, the crew will have to come and do it themselves. They will."));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! After dark only. Seen while the village was warned, once noticed, or with shots fired near.
	protected void DoIt(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId)
	{
		if (!RZ_RedHandQuest.IsDark())
		{
			gm.NotifyPlayer(playerId, m_sTitle, "Not in daylight, with the whole village coming and going. After dark.");
			return;
		}

		bool seen = run.Flag("warned") == 1 || !m_Quiet.IsQuiet(gm, playerId);
		RZ_RedHandQuest.SetWorld(gm, "well.done", 1);
		string text = "It goes in without a sound. Nobody saw. In the morning, everybody drinks.";
		if (seen)
		{
			run.SetFlag("seen", 1);
			RZ_QuestApproach.SideWith(gm, playerId, -1, RZ_EFaction.SURVIVORS, 0, SEEN_INFAMY);
			text = string.Format("Somebody sees you at the well. There's shouting before you're over the wall, and every safehouse will know your face by morning. (+%1 Haven Infamy)", SEEN_INFAMY);
		}

		gm.NotifyPlayer(playerId, m_sTitle, text);
		Resolve(gm, playerId, RZ_EQuestApproach.SNEAK, RZ_Quest_WellStory.OUT_DONE, "put something in the well");
	}

	//------------------------------------------------------------------------------------------------
	//! ★ Lab Tech and Nurse know what's harmless; anyone else gets it wrong some of the time.
	protected void Fake(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId)
	{
		RZ_RedHandQuest.SetWorld(gm, "well.faked", 1);
		string text = "Something long dead from the ditch, in the bucket, where it'll be found. Harmless, if you know which dead things are. You do.";
		if (!HasBonus(gm, playerId))
		{
			text = "Something dead from the ditch, in the bucket, where it'll be found. You hope it's harmless.";
			if (Math.RandomFloat01() < WRONG_FAKE)
			{
				RZ_Questline.NudgeInfection(gm, run.Flag("town", -1), FAKE_INFECTION);
				text += " It wasn't, quite: two families have fevers by the weekend.";
			}
		}

		gm.NotifyPlayer(playerId, m_sTitle, text);
		Resolve(gm, playerId, RZ_EQuestApproach.BETRAY, RZ_Quest_WellStory.OUT_FAKED, "faked it");
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateSample(RZ_GameModeComponent gm, float dt)
	{
		if (m_bSample || m_iSampler <= 0)
			return;

		IEntity sampler = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iSampler);
		if (!sampler || RZ_Util.DistXZ(sampler.GetOrigin(), m_vWell) > 6)
		{
			m_iSampler = -1;
			m_fSample = 0;
			return;
		}

		m_fSample += dt;
		if (m_fSample >= SAMPLE_SECONDS)
			Sampled(gm);
	}

	//------------------------------------------------------------------------------------------------
	protected void Sampled(RZ_GameModeComponent gm)
	{
		m_bSample = true;
		Remember(gm, m_iSampler, RZ_EQuestApproach.SIDE, "took a water sample for Halcyon");
		m_iLabSpot = AddSpot(LandPointNear(m_vLab, 6, 12), "Halcyon at " + m_sLab);
		gm.NotifyPlayer(m_iSampler, m_sTitle, string.Format("A clean bottle of the village's water. Take it to Halcyon at %1: they'll test it, and seal the well if they have to.", m_sLab));
	}

	//------------------------------------------------------------------------------------------------
	protected void Tested(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId)
	{
		RZ_RedHandQuest.SetWorld(gm, "well.tested", 1);
		RZ_RedHandQuest.SetWorld(gm, "well.stood", 1);
		RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.INSTITUTE, -1, HALCYON_FAME, 0);
		run.Reward(gm, playerId, RZ_EFaction.SURVIVORS, HAVEN_FAME, 0, QFill(gm, "{village} heard who brought Halcyon to its well."));
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "Halcyon sends two people in masks to {village} with a test kit and a padlock. The water's clean, and now it's watched. The crew boss will hear who brought them."));
		Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_WellStory.OUT_TESTED, "got Halcyon to seal the well", false);
	}

	//------------------------------------------------------------------------------------------------
	//! Standing guard: the crew comes after CREW_DELAY, and has to be driven off.
	protected void UpdateGuard(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_iGuard <= 0)
			return;

		if (m_fCrewIn > 0)
		{
			m_fCrewIn -= dt;
			if (m_fCrewIn > 0)
				return;

			if (!m_Crew.Spawn(gm.GetRedHandGroupPrefab(), LandPointNear(m_vWell, 80, 120)))
				return;

			TrackProp(m_Crew.m_Unit);
			if (gm.GetHostility())
				gm.GetHostility().WatchUnit(m_Crew.m_Unit, RZ_EFaction.BANDITS);

			RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, m_Crew.m_Unit, 200);
			RZ_Util.SendGroupTo(m_Crew.m_Unit, m_vWell, gm.GetSpawner().GetMoveWaypointPrefab());
			RZ_ContractHelp.NotifyNear(gm, m_vWell, 300, m_sTitle, QFill(gm, "Headlights on the road from {crew_place}. {crew_short}'s crew is coming to do it themselves."));
			return;
		}

		RZ_ContractHelp.AddNear(ids, players, m_vWell, 150, m_aParticipants);
		if (!m_Crew.IsDead(dt))
			return;

		int who = RZ_EscortHelp.PlayerBeside(ids, players, m_vWell, 150);
		if (who <= 0)
			return;

		RZ_RedHandQuest.SetWorld(gm, "well.stood", 1);
		RZ_QuestRun run = Run(gm);
		if (run)
			run.Reward(gm, m_iGuard, RZ_EFaction.SURVIVORS, HAVEN_FAME, 0, QFill(gm, "{village} saw who stood at its well."));

		// The crew won't forget who was standing there (the ones who sided are counted at the ending)
		RZ_QuestApproach.SideWith(gm, m_iGuard, -1, RZ_EFaction.BANDITS, 0, HELD_INFAMY);
		if (who != m_iGuard)
			RZ_QuestApproach.SideWith(gm, who, -1, RZ_EFaction.BANDITS, 0, HELD_INFAMY);

		NotifyParticipants(gm, m_sTitle, QFill(gm, "The crew is down or gone. {village} comes out with lamps and stands round the well with you until it's light."));
		Resolve(gm, who, RZ_EQuestApproach.FORCE, RZ_Quest_WellStory.OUT_HELD, "drove the crew off the well", false);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Well Story: the crew boss's order is carried out at {village}.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Well Story: {crew_short}'s order wasn't carried out the way {crew_short} wanted.");
	}
}

//------------------------------------------------------------------------------------------------
//! The Well Story, stage 6 (Red Hand contract, Trusted): the boss sends for you. His second at the
//! stronghold takes your answer: own it, blame the crew boss (he goes in the pit) or say nothing.
class RZ_QM_WellBoss : RZ_QuestMission
{
	protected int m_iTalk = -1;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.TRUSTED, 8, 15);
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		RZ_Leader boss = RZ_RedHandQuest.Seat(gm, run, "boss");
		if (!run || !boss)
			return false;

		m_vPos = LandPointNear(boss.m_vPos, 8, 16);
		m_iSettlement = gm.FindNearestSettlement(boss.m_vPos);
		m_iTalk = AddClue("The boss's second", QFill(gm, "\"The boss wants to know about {village}. The boss says the story works best untold: you don't kill the cow you milk. So. What happened?\""));
		m_sTitle = "The Well Story: the boss hears";
		m_sBrief = QFill(gm, "The boss has heard about {village}, and wants it from you, at {boss_place}. Not angry. Yet.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		string choices = QFill(gm, "Own it|Blame {crew_short}|Say nothing");
		return SpawnPerson(gm, m_vPos, 0, m_iTalk, "Talk to the boss's second", choices) != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0 || m_bResolved)
			return;

		if (!IsEligible(gm, playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"The boss sent for people the boss trusts.\" The door stays shut.");
			return;
		}

		if (option == 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You tell it straight, what you did and why. The second writes nothing down. \"The boss will remember that you came and said it.\"");
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_WellStory.OUT_OWN, "owned it");
			return;
		}

		if (option == 1)
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "\"{crew_short}'s order, was it.\" The second nods slowly. \"The pit's on tonight. {crew_short} will be in it.\""));
			Resolve(gm, playerId, RZ_EQuestApproach.PERSUADE, RZ_Quest_WellStory.OUT_BLAME, "blamed the crew boss");
			return;
		}

		if (option == 2)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You say nothing. The second waits, then shrugs. \"Then the boss decides what happened. The boss usually does.\"");
			Resolve(gm, playerId, RZ_EQuestApproach.NONE, RZ_Quest_WellStory.OUT_QUIET, "said nothing");
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The Well Story: nobody answered the boss.";
	}
}
