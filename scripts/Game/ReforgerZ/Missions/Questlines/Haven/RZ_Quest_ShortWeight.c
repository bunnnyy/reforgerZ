//! Short Weight (the Haven): the example questline (docs/MISSIONS.md, "Questlines"). Four stages,
//! two branches, two endings.
//!
//! 1. Where the food goes (mission): a safehouse's stores come up short. Ask its storekeeper,
//!    lean on him (persuade), cover the shortfall (bribe, Haven Favor) or read his ledger
//!    (sneak). He's trading tins to the Red Hand for antibiotics for a sick girl.
//! 2. The next handover (wait): 30 minutes of play or 12 real hours.
//! 3. The handover (mission): outside town, the storekeeper, a Red Hand runner and the crate.
//!    Turn him in (talk), talk him out of it (persuade), run the runner off (force, armed), pay
//!    the runner off (bribe, Red Hand Favor) or take the crate back unseen (sneak): the Haven's
//!    branch. Warn the runner (side with the Red Hand), or let nobody come: the Red Hand's branch.
//! 4. Word gets round (wait): 15 minutes of play or 6 real hours, then the ending.
//!
//! Endings: Full Weight (Haven morale and food up, the Red Hand's morale down) or Short Weight
//! (the other way round), with a headline and a rumour each.
class RZ_Quest_ShortWeight : RZ_Questline
{
	static const int S_ASK = 0;
	static const int S_WAIT = 1;
	static const int S_HANDOVER = 2;
	static const int S_AFTER = 3;

	static const int OUT_LEARNED = 10;		// stage 1: they know where the food goes
	static const int OUT_STOPPED = 11;		// stage 3: the deal is over
	static const int OUT_DEAL = 12;			// stage 3: the deal goes on

	static const int BRANCH_HAVEN = 0;
	static const int BRANCH_RED_HAND = 1;

	static const int E_FULL = 0;			// Full Weight
	static const int E_SHORT = 1;			// Short Weight

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		Info("haven_short_weight", "Short Weight", RZ_EFaction.SURVIVORS);

		RZ_QuestStage ask = MissionStage(S_ASK, "Where the food goes", "The stores at the safehouse in {town} come up short every week, and nobody's eaten more. {keeper} keeps them. Find out where the food goes.");
		ask.GiveUp(3, "Nobody asked {keeper} about the stores. {town} still comes up short, and nobody says why.");
		ask.Visits(2);

		RZ_QuestStage wait = WaitStage(S_WAIT, "The next handover", "{keeper} trades the house's tins to the Red Hand for antibiotics, for {girl}, who has a cough that won't stop. The next handover is outside {town}, in a day or so.");
		wait.Wait(30, 12);
		wait.GiveUp(2, "{keeper} never went out to meet anyone again. The stores in {town} are still short, but quietly.");

		RZ_QuestStage handover = MissionStage(S_HANDOVER, "The handover", "{keeper} is meeting a Red Hand runner, {runner}, outside {town} with a crate of the house's food. Stop it, take the crate back, or let it happen.");
		handover.GiveUp(3, "{keeper} stopped going out to meet anyone. The stores in {town} are still short, but quietly.");

		RZ_QuestStage after = WaitStage(S_AFTER, "Word gets round", "Whatever happened at the handover, {town} will have heard about it by tomorrow.");
		after.Wait(15, 6);
	}

	//------------------------------------------------------------------------------------------------
	//! Needs a safehouse with people in it, the puzzle note and a person to talk to.
	override bool CanStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (!RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return false;

		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses || houses.Count() == 0)
			return false;

		array<int> options = {};
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse cand = houses.Get(i);
			if (cand && cand.m_iPopulation >= 3 && cand.m_iSettlement >= 0)
				options.Insert(i);
		}

		if (options.IsEmpty())
			return false;

		int idx = options.GetRandomElement();
		RZ_Safehouse h = houses.Get(idx);
		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		run.SetFlag("house", idx);
		run.SetText("town", h.m_sName);
		run.SetText("keeper", RZ_NoteText.FullName(rng));
		run.SetText("runner", RZ_NoteText.FirstName(rng));
		run.SetText("girl", RZ_NoteText.FirstName(rng));
		run.SetPlace(h.m_vPos, h.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		Headline(gm, run.Fill("The safehouse in {town} weighed its stores twice this week. Short both times."), false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStageBegin(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (stage == S_ASK)
			run.Note("The stores in {town} come up short every week. {keeper} keeps them, and says it's mice.");
	}

	//------------------------------------------------------------------------------------------------
	//! The safehouse has to still be there, with people in it.
	override bool IsReady(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		RZ_Safehouse h = GetHouse(gm, run.Flag("house", -1));
		return h && h.m_iPopulation > 0;
	}

	//------------------------------------------------------------------------------------------------
	override RZ_Mission MakeMission(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (stage == S_ASK)
			return new RZ_QM_ShortWeightAsk();

		if (stage == S_HANDOVER)
			return new RZ_QM_ShortWeightHandover();

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override int Next(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		switch (stage)
		{
			case S_ASK:
			{
				if (outcome == OUT_LEARNED)
				{
					run.Note("{keeper} trades tins to the Red Hand for antibiotics. They're for {girl}, who coughs all night. Every few days, outside town.");
					return S_WAIT;
				}

				run.Note("Nobody got anywhere with {keeper}. The council will ask someone else.");
				return S_ASK;
			}
			case S_WAIT:
				return S_HANDOVER;
			case S_HANDOVER:
			{
				if (outcome == OUT_STOPPED)
				{
					run.SetBranch(BRANCH_HAVEN);
					run.Note("The crate didn't go to the Red Hand. " + HowStopped(run));
					return S_AFTER;
				}

				run.SetBranch(BRANCH_RED_HAND);
				if (outcome == OUT_DEAL)
				{
					run.Note("Someone warned {runner} that the Haven knows. The deal moves somewhere nobody will look.");
				}
				else
				{
					run.SetFlag("ignored", 1);
					run.Note("Nobody came to the handover. {runner} took the crate, and {keeper} walked home with the pills.");
				}

				return S_AFTER;
			}
			case S_AFTER:
			{
				if (run.m_iBranch == BRANCH_HAVEN)
					return Finish(run, E_FULL, "Full Weight. The stores in {town} weigh right again, and the Red Hand will have to find its tins somewhere else.");

				return Finish(run, E_SHORT, "Short Weight. The stores in {town} stay a little light. {girl} has stopped coughing.");
			}
		}

		return Fizzle(run, "");
	}

	//------------------------------------------------------------------------------------------------
	protected string HowStopped(RZ_QuestRun run)
	{
		if (run.Flag("turned_in") == 1)
			return "{keeper} was walked back to the council.";

		if (run.Flag("mended") == 1)
			return "{keeper} was talked out of it, and carried it home himself.";

		if (run.Flag("ran_off") == 1)
			return "{runner} was run off at gunpoint.";

		if (run.Flag("paid") == 1)
			return "{runner} was paid to walk away. He left the pills anyway.";

		return "It went back into the stores, and nobody saw who carried it.";
	}

	//------------------------------------------------------------------------------------------------
	override void OnEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
		int house = run.Flag("house", -1);
		if (ending == E_FULL)
		{
			NudgeMorale(gm, RZ_EFaction.SURVIVORS, 5);
			NudgeMorale(gm, RZ_EFaction.BANDITS, -3);
			ChangeFood(gm, house, 4);
			if (run.Flag("ran_off") == 1)
				NudgeStrength(gm, RZ_EFaction.BANDITS, -2);

			// The council's justice: he's put out (the house is one smaller)
			if (run.Flag("turned_in") == 1)
			{
				ChangePopulation(gm, house, -1);
				Headline(gm, run.Fill("The council in {town} put {keeper} out at dawn with a day's food. The stores weigh right again."));
			}
			else
			{
				Headline(gm, run.Fill("{keeper} still keeps the stores in {town}. He weighs everything twice now, and the Red Hand doesn't come by."));
			}

			ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 4, 0);
			SpreadRumour(gm, run.Fill("\"The safehouse in {town} eats properly again. Somebody caught the mouse, they say. A big one.\""), 6);
			return;
		}

		NudgeMorale(gm, RZ_EFaction.BANDITS, 4);
		NudgeMorale(gm, RZ_EFaction.SURVIVORS, -3);
		ChangeFood(gm, house, -3);

		// Whoever warned the runner is remembered by the Red Hand
		ChangeStanding(gm, run, RZ_EFaction.BANDITS, 3, 0, RZ_EQuestApproach.SIDE);
		Headline(gm, run.Fill("The safehouse in {town} eats a little less and coughs a little less. Nobody asks why."));
		SpreadRumour(gm, "\"The Red Hand's eating tinned peaches. Haven tins. Make of that what you like.\"", 6);
	}

	//------------------------------------------------------------------------------------------------
	override string Rumour(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.m_iStage == S_WAIT || run.m_iStage == S_HANDOVER)
			return "\"The safehouse in {town} comes up short every week. Someone says mice. Mice don't eat tins.\"";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The house's council head has something to say while the handover is coming.
	override string OnLeaderTalk(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, int leaderId)
	{
		if (run.m_iStage != S_WAIT && run.m_iStage != S_HANDOVER)
			return "";

		RZ_Leader l = gm.GetLeaders().Get(leaderId);
		if (!l || l.m_iSafehouse != run.Flag("house", -1))
			return "";

		return string.Format("%1 lowers their voice. \"About {keeper}. I've known him twenty years. Bring the food back. Bring him back too, if you can.\"", l.m_sShort);
	}
}

//------------------------------------------------------------------------------------------------
//! Short Weight, stage 1 (Haven contract): find out where the food goes. The storekeeper at the
//! safehouse (talk, persuade, bribe) and his ledger nearby (sneak: read it unseen).
class RZ_QM_ShortWeightAsk : RZ_QuestMission
{
	protected int m_iHouse = -1;
	protected vector m_vKeeper;
	protected vector m_vLedger;
	protected int m_iTalk = -1;
	protected int m_iLedger = -1;

	protected static const int OBJ_KEEPER = 0;
	protected static const int SEEN_RADIUS = 12;
	protected static const int COVER_FAVOR = 15;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 8, 10);
		m_Req.Bonus("police");
		m_fTimeLeft = 2400;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iHouse = QFlag(gm, "house", -1);
		RZ_Safehouse h = RZ_Questline.GetHouse(gm, m_iHouse);
		if (!h || IsTownBusy(gm, h.m_iSettlement))
			return false;

		m_iSettlement = h.m_iSettlement;
		m_vPos = h.m_vPos;
		m_vKeeper = LandPointNear(h.m_vPos, 6, 18);
		m_vLedger = LandPointNear(h.m_vPos, 20, 35);
		m_iTalk = AddClue(QText(gm, "keeper"), QFill(gm, "{keeper} is counting tins with his lips moving. \"Short? Mice. Damp. People with long arms.\" He doesn't look at the shelves. He keeps touching his coat pocket."));
		m_iLedger = AddClue("The stores ledger", QFill(gm, "Tins, rice and flour in a careful hand. Every fourth day the same line with no name: '6 tins, 2 rice - R.' Under it, smaller: 'amoxicillin x10, for {girl}.' The next one says 'the old bus shelter, outside town'."));
		m_Quiet.Watch(h.m_vPos, 60);
		m_sTitle = "Short Weight";
		m_sBrief = QFill(gm, "The council in {town} says the stores come up short every week, and nobody's eaten more. {keeper} keeps them. Find out where the food goes. Ask nicely, or don't.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		string choices = string.Format("Ask about the stores|Lean on him|Cover the shortfall (%1 Favor)", COVER_FAVOR);
		if (!SpawnPerson(gm, m_vKeeper, OBJ_KEEPER, m_iTalk, "Talk to " + QText(gm, "keeper"), choices))
			return false;

		return SpawnClue(gm, RZ_EPuzzleProp.CLUE, m_vLedger, m_iLedger, "Read the stores ledger") != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_KEEPER)
			return;

		if (option == 0)
		{
			Remember(gm, playerId, RZ_EQuestApproach.TALK, "asked about the stores");
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "\"I count them, I write them down, I lock the door. If the numbers don't add up, ask the numbers.\" There's a ledger somewhere close. He keeps glancing at the shed."));
			return;
		}

		if (option == 1)
		{
			if (TryPersuade(gm, playerId, RZ_EFaction.SURVIVORS, 2, "police,excon,elder"))
			{
				SetQFlag(gm, "confessed", 1);
				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "He sits down on a sack of rice. \"It's {girl}. The cough. The Red Hand have antibiotics, and they want tins. Every few days, outside town. I'd do it again.\""));
				Resolve(gm, playerId, RZ_EQuestApproach.PERSUADE, RZ_Quest_ShortWeight.OUT_LEARNED, "got him to admit it");
				return;
			}

			Remember(gm, playerId, RZ_EQuestApproach.PERSUADE, "leaned on him and got nowhere");
			gm.NotifyPlayer(playerId, m_sTitle, "He folds his arms. \"I've got shelves to count.\" He won't say more to you.");
			return;
		}

		if (option == 2 && TryBribe(gm, playerId, RZ_EFaction.SURVIVORS, COVER_FAVOR))
		{
			SetQFlag(gm, "covered", 1);
			RZ_Questline.ChangeFood(gm, m_iHouse, 2);
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "He looks at what you've put on the shelf for a long time. \"That covers this week.\" Then, quietly: \"It's for {girl}. The Red Hand have antibiotics, and they want tins.\""));
			Resolve(gm, playerId, RZ_EQuestApproach.BRIBE, RZ_Quest_ShortWeight.OUT_LEARNED, "covered the shortfall");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Reading the ledger: unseen if he's not right there and nothing has noticed you.
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		if (idx != m_iLedger)
			return;

		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		bool seen = !m_Quiet.IsQuiet(gm, playerId) || (ent && RZ_Util.DistXZ(ent.GetOrigin(), m_vKeeper) <= RZ_Stealth.SightRadius(playerId, SEEN_RADIUS));
		if (seen)
		{
			SetQFlag(gm, "caught", 1);
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{keeper} is in the doorway before you've turned two pages. \"That's mine.\" He knows you know. He'll be careful now."));
			Resolve(gm, playerId, RZ_EQuestApproach.SNEAK, RZ_Quest_ShortWeight.OUT_LEARNED, "was caught reading his ledger");
			return;
		}

		Resolve(gm, playerId, RZ_EQuestApproach.SNEAK, RZ_Quest_ShortWeight.OUT_LEARNED, "read his ledger unseen");
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "Short Weight: you know where the food in {town} is going.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "Short Weight: nobody found out where the food in {town} goes. The council will ask again.");
	}
}

//------------------------------------------------------------------------------------------------
//! Short Weight, stage 3 (Haven contract): the handover outside town. The storekeeper (turn him
//! in, talk him out of it), the Red Hand runner (warn him, run him off, pay him off), the crate
//! (take it back unseen) and, with the Red Hand group prefab set, the runner's lookouts (kill
//! them all: force). Time up: nobody came, and the deal goes on.
class RZ_QM_ShortWeightHandover : RZ_QuestMission
{
	protected int m_iHouse = -1;
	protected vector m_vKeeper;
	protected vector m_vRunner;
	protected vector m_vCrate;
	protected int m_iKeeperTalk = -1;
	protected int m_iRunnerTalk = -1;
	protected ref RZ_UnitWatch m_Lookouts = new RZ_UnitWatch();
	protected bool m_bSided;

	protected static const int OBJ_KEEPER = 0;
	protected static const int OBJ_RUNNER = 1;
	protected static const int OBJ_CRATE = 2;
	protected static const int PAY_FAVOR = 15;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 12, 12);
		m_Req.Bonus("nurse");
		m_fTimeLeft = 2700;
		m_fParticipationRadius = 0;
		m_fSceneRadius = 350;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iHouse = QFlag(gm, "house", -1);
		RZ_Safehouse h = RZ_Questline.GetHouse(gm, m_iHouse);
		if (!h)
			return false;

		m_vPos = LandPointNear(h.m_vPos, 350, 600);
		if (RZ_Util.IsWater(m_vPos))
			return false;

		m_iSettlement = h.m_iSettlement;
		m_vKeeper = RZ_Util.RandomPointInRing(m_vPos, 1, 3);
		m_vRunner = RZ_Util.RandomPointInRing(m_vPos, 3, 5);
		m_vCrate = LandPointNear(m_vPos, 20, 28);
		m_iKeeperTalk = AddClue(QText(gm, "keeper"), QFill(gm, "{keeper} won't meet your eyes. \"Don't. Please. She's nine, and it's in her chest now.\""));
		m_iRunnerTalk = AddClue(QText(gm, "runner"), QFill(gm, "{runner} grins. \"Haven's out for a walk. Tins for pills, friend. Everybody wins, except the mice.\""));
		m_sTitle = "Short Weight: the handover";
		m_sBrief = QFill(gm, "{keeper} is meeting a Red Hand runner, {runner}, near the old bus shelter outside {town}, with a crate of the house's food. Bring the food back. How is up to you.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		if (!SpawnPerson(gm, m_vKeeper, OBJ_KEEPER, m_iKeeperTalk, "Talk to " + QText(gm, "keeper"), "Turn him in|Talk him out of it"))
			return false;

		string runnerChoices = string.Format("Warn him: the Haven knows|Run him off|Pay him off (%1 Red Hand Favor)", PAY_FAVOR);
		if (!SpawnPerson(gm, m_vRunner, OBJ_RUNNER, m_iRunnerTalk, "Talk to " + QText(gm, "runner"), runnerChoices))
			return false;

		if (!SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vCrate, OBJ_CRATE, "Take the food back", ""))
			return false;

		m_Quiet.Watch(m_vPos, 150);

		// The runner's lookouts, if the Red Hand group prefab is set: killing them all is force.
		// Tracked as a prop, so Cleanup removes them with the rest
		ResourceName group = gm.GetRedHandGroupPrefab();
		if (!group.IsEmpty() && m_Lookouts.Spawn(group, LandPointNear(m_vPos, 30, 45)))
			TrackProp(m_Lookouts.m_Unit);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved || !m_Lookouts.IsDead(dt))
			return;

		// The lookouts are dead: whoever was close did it
		array<int> near = {};
		GetPlayersNear(ids, players, m_vPos, 150, near);
		if (near.IsEmpty())
			return;

		foreach (int playerId : near)
		{
			Remember(gm, playerId, RZ_EQuestApproach.FORCE, "fought the Red Hand's lookouts");
		}

		SetQFlag(gm, "ran_off", 1);
		RZ_QuestApproach.SideWith(gm, near[0], -1, RZ_EFaction.BANDITS, 0, 5);
		NotifyParticipants(gm, m_sTitle, QFill(gm, "{runner} doesn't wait to see the rest. He's gone over the field, and the crate is still on the road."));
		Resolve(gm, near[0], RZ_EQuestApproach.FORCE, RZ_Quest_ShortWeight.OUT_STOPPED, "fought the Red Hand's lookouts");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx == OBJ_KEEPER)
		{
			KeeperChoice(gm, playerId, option);
			return;
		}

		if (objIdx == OBJ_RUNNER)
			RunnerChoice(gm, playerId, option);
	}

	//------------------------------------------------------------------------------------------------
	protected void KeeperChoice(RZ_GameModeComponent gm, int playerId, int option)
	{
		if (option == 0)
		{
			SetQFlag(gm, "turned_in", 1);
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "He doesn't argue. He picks up the crate himself and walks back toward {town} ahead of you. {runner} spits and leaves."));
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_ShortWeight.OUT_STOPPED, "turned him in");
			return;
		}

		if (option != 1)
			return;

		// Harder if he caught someone reading his ledger
		int difficulty = 2 + QFlag(gm, "caught");
		if (TryPersuade(gm, playerId, RZ_EFaction.SURVIVORS, difficulty, "nurse,doctor,elder"))
		{
			SetQFlag(gm, "mended", 1);
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You tell him what Halcyon's medic told you about coughs like that. He listens. Then he picks up the crate. \"We'll find another way. You'll help.\""));

			// A personal reward on top of the contract's pay: the council hears who brought him home
			RZ_QuestRun run = Run(gm);
			if (run)
				run.Reward(gm, playerId, RZ_EFaction.SURVIVORS, 3, 5, QFill(gm, "The council in {town} heard who brought {keeper} home."));

			Resolve(gm, playerId, RZ_EQuestApproach.PERSUADE, RZ_Quest_ShortWeight.OUT_STOPPED, "talked him out of it");
			return;
		}

		Remember(gm, playerId, RZ_EQuestApproach.PERSUADE, "tried to talk him out of it");
		gm.NotifyPlayer(playerId, m_sTitle, "\"You don't know her. You don't know anything.\" He turns his back on you.");
	}

	//------------------------------------------------------------------------------------------------
	protected void RunnerChoice(RZ_GameModeComponent gm, int playerId, int option)
	{
		if (option == 0)
		{
			// Siding with the Red Hand: the Haven pays nothing for this, and will hear of it
			m_bSided = true;
			RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.BANDITS, RZ_EFaction.SURVIVORS, 6, 8);
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{runner} looks at you for a long moment, then laughs. \"Decent of you.\" He takes the crate. \"We'll meet somewhere else next time. You're welcome at the camp.\""));
			Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_ShortWeight.OUT_DEAL, "warned the runner", false);
			return;
		}

		if (option == 1)
		{
			IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (!user || !RZ_Util.IsArmed(user))
			{
				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{runner} looks at your empty hands and smiles. \"Run me off with what?\""));
				return;
			}

			SetQFlag(gm, "ran_off", 1);
			gm.GetHeat().AddHeat(m_vPos, 15);
			RZ_QuestApproach.SideWith(gm, playerId, -1, RZ_EFaction.BANDITS, 0, 5);
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{runner} backs off with his hands up. \"Fine. Keep your tins. We'll remember the face.\""));
			Resolve(gm, playerId, RZ_EQuestApproach.FORCE, RZ_Quest_ShortWeight.OUT_STOPPED, "ran the runner off");
			return;
		}

		if (option == 2 && TryBribe(gm, playerId, RZ_EFaction.BANDITS, PAY_FAVOR))
		{
			SetQFlag(gm, "paid", 1);
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{runner} pockets it and leaves the crate. Then he tosses the box of pills to {keeper}. \"On the house. Don't tell anyone I'm soft.\""));
			Resolve(gm, playerId, RZ_EQuestApproach.BRIBE, RZ_Quest_ShortWeight.OUT_STOPPED, "paid the runner off");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The crate: taken back unseen (no shots near, not noticed), or the runner sees you.
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx != OBJ_CRATE)
			return;

		if (!m_Quiet.IsQuiet(gm, playerId))
		{
			string why = m_Quiet.Why(gm, playerId);
			m_Quiet.Spot(playerId);
			Remember(gm, playerId, RZ_EQuestApproach.SNEAK, "was seen at the crate");
			RZ_QuestApproach.SideWith(gm, playerId, -1, RZ_EFaction.BANDITS, 0, 4);
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, why + " {runner} turns round. \"Hands off, Haven.\" You won't get near it quietly now."));
			return;
		}

		SetQFlag(gm, "quiet", 1);
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "The crate is lighter than it should be, but it's most of a week. By the time {runner} looks round, it's gone."));
		Resolve(gm, playerId, RZ_EQuestApproach.SNEAK, RZ_Quest_ShortWeight.OUT_STOPPED, "took the crate back unseen");
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "Short Weight: the crate is going back into the stores in {town}.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (m_bSided)
			return QFill(gm, "Short Weight: {runner} left with the crate, and a warning.");

		return QFill(gm, "Short Weight: nobody stopped the handover. The crate went with {runner}.");
	}

}
