//! The Edge of Town (the Haven): questline H3 (docs/QUESTLINES.md, "H3. The Edge of Town"). At one
//! house, someone who declares a bite is walked to the edge of town at dusk, and the walker comes
//! back alone. A nurse at another house says eight in ten of them could have walked out of
//! Halcyon's lab instead. The walker says the lab writes down the other two. Both are right.
//! Starts after anyone has finished the contract Walk Him Out (world flag "walk.contract"), with
//! the lab there and no cure yet.
//!
//! 1. One more walk (mission, at dusk): Walk Him Out at the walker's house, plus a third choice:
//!    take him to the lab for the trial dose (he follows you; 8 in 10 he walks out).
//! 2. The walker's tally (mission): his graves at the edge of town (three) and his notebook. Ask
//!    him the names (Known), or read the notebook while he's on watch. ★ Elder: he asks you to
//!    walk the next one with him.
//! 3. The lab's numbers (mission, at the lab): the field lead (Trusted with Halcyon, or ★ Lab Tech
//!    or Doctor), the trial records on the night shift (seen: +8 Halcyon Infamy), or a courier
//!    (20 Halcyon Favor). Sets walk.knows_other_two for HC2.
//! 4. Two offers (two hours of play, then a mission at the nurse's house): the lab road (Known with
//!    Halcyon, and Trusted with the Haven, or Known after a man was saved at stage 1; Trusted
//!    again with weeks.public), refuse both, or the Red Hand's cut dose (Trusted with the Red Hand).
//! 5. Make it real (mission): Cold Chain to the nurse's clinic, the walker's last walk with you
//!    (at dusk), or the cut dose carried from a Red Hand seat past Halcyon's courier.
//! 6. A session later (wait, the cut dose only): the houses find out.
//!
//! Endings: The lab road, The walks stay, The cut dose.
class RZ_Quest_EdgeOfTown : RZ_Questline
{
	static const int S_WALK = 0;
	static const int S_TALLY = 1;
	static const int S_NUMBERS = 2;
	static const int S_OFFERS = 3;
	static const int S_REAL = 4;
	static const int S_LATER = 5;

	static const int OUT_WALKED = 10;		// 1: ended it, or let him walk on
	static const int OUT_LAB_SAVED = 11;	// 1: the trial dose took
	static const int OUT_LAB_LOST = 12;		// 1: it didn't
	static const int OUT_TALLY = 13;		// 2: the names
	static const int OUT_KNOWS = 14;		// 3: what happens to the other two
	static const int OUT_LAB_ROAD = 15;		// 4: broker the lab road
	static const int OUT_REFUSE = 16;		// 4: refuse both
	static const int OUT_CUT = 17;			// 4: the Red Hand's cut dose
	static const int OUT_DELIVERED = 18;	// 5: the cut dose delivered

	static const int BRANCH_LAB = 0;
	static const int BRANCH_WALKS = 1;
	static const int BRANCH_CUT = 2;

	static const int E_LAB_ROAD = 0;
	static const int E_WALKS = 1;
	static const int E_CUT = 2;

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		Info("walk", "The Edge of Town", RZ_EFaction.SURVIVORS);

		RZ_QuestStage walk = MissionStage(S_WALK, "One more walk", "In {walkhouse}, someone who declares a bite is walked to the edge of town at dusk by {walker}, who comes back alone. {nurse}, in {nursehouse}, says most of them could have walked out of Halcyon's lab instead. Tonight {man} is waiting at the edge of town.");
		walk.GiveUp(6, "{why}");
		walk.Visits(2);

		RZ_QuestStage tally = MissionStage(S_TALLY, "The walker's tally", "{walker} keeps a grave for everyone he's walked out, at the edge of {walkhouse}, and their names in a notebook. Count the graves, and get the names: ask him, or read the notebook while he's on watch.");
		tally.GiveUp(4, "{why}");
		tally.Visits(2);

		RZ_QuestStage numbers = MissionStage(S_NUMBERS, "The lab's numbers", "{walker} says the lab writes down the two in ten it can't save. Find out what that means: ask a field lead at the lab, read the trial records on the night shift, or pay a courier for a look.");
		numbers.GiveUp(4, "{why}");
		numbers.Visits(2);

		RZ_QuestStage offers = MissionStage(S_OFFERS, "Two offers", "The houses are talking. The lab will have something to say, and so will the Red Hand. {nurse} wants someone to settle it.");
		offers.Wait(120);
		offers.GiveUp(4, "{why}");

		RZ_QuestStage real = MissionStage(S_REAL, "Make it real", "{realtext}");
		real.GiveUp(6, "{why}");
		real.Visits(2);

		RZ_QuestStage later = WaitStage(S_LATER, "A session later", "The cut dose is in the houses now. Sooner or later they'll count who walked out.");
		later.Wait(120);
		later.GiveUp(3, "{why}");
	}

	//------------------------------------------------------------------------------------------------
	//! Needs Walk Him Out done this season, the lab, no cure yet, a safehouse of four or more (the
	//! walker's) and a second one for the nurse (one with a medic if there is one), and the note, the
	//! object and the civilian.
	override bool CanStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (!RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN) || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.OBJECT))
			return false;

		if (!gm.HasLab() || gm.GetCure().IsCured() || !gm.GetQuestlines() || gm.GetQuestlines().WorldFlag("walk.contract") != 1)
			return false;

		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses || houses.Count() == 0)
			return false;

		array<int> walkers = {};
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, i);
			if (h && h.m_iPopulation >= 4)
				walkers.Insert(i);
		}

		if (walkers.IsEmpty())
			return false;

		int walkHouse = walkers.GetRandomElement();
		int nurseHouse = PickNurseHouse(gm, walkHouse);
		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		string walker = RZ_HavenQuest.ManName(rng);
		string man = RZ_HavenQuest.ManName(rng);
		for (int k = 0; k < 6 && man == walker; k++)
		{
			man = RZ_HavenQuest.ManName(rng);
		}

		run.SetFlag("walkhouse", walkHouse);
		run.SetFlag("nursehouse", nurseHouse);
		run.SetText("walkhouse", houses.Get(walkHouse).m_sName);
		run.SetText("nursehouse", houses.Get(nurseHouse).m_sName);
		run.SetText("walker", walker);
		run.SetText("nurse", RZ_HavenQuest.WomanName(rng));
		run.SetText("man", man);
		run.SetText("next", RZ_HavenQuest.ManName(rng));
		run.SetText("lab", LabName(gm));
		if (gm.GetLeaders())
			RZ_HavenQuest.KeepLeader(run, "director", gm.GetLeaders().GetStronghold(RZ_EFaction.INSTITUTE));

		if (!run.HasFlag("director"))
			run.SetText("director", "Halcyon's research director");

		run.SetText("realtext", "Make it real.");
		run.SetText("count", "Eleven");
		run.SetText("why", run.Fill("Nobody settled it. In {walkhouse} they still walk people to the edge of town at dusk."));
		run.SetPlace(houses.Get(walkHouse).m_vPos, houses.Get(walkHouse).m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The nurse's house: another one with a medic, else any other one with people, else the same.
	protected int PickNurseHouse(RZ_GameModeComponent gm, int walkHouse)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		array<int> medics = {};
		array<int> others = {};
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, i);
			if (!h || i == walkHouse)
				continue;

			others.Insert(i);
			if (h.HasMedic())
				medics.Insert(i);
		}

		if (!medics.IsEmpty())
			return medics.GetRandomElement();

		if (!others.IsEmpty())
			return others.GetRandomElement();

		return walkHouse;
	}

	//------------------------------------------------------------------------------------------------
	static string LabName(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(gm.GetLabSettlement());
		if (s)
			return "the lab at " + s.m_sName;

		return "the lab";
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		Headline(gm, run.Fill("Someone in {walkhouse} declared a bite at supper. The house went quiet, the way it does."), false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStageBegin(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		CheckNurse(gm, run);
		RZ_Safehouse nurseHouse = RZ_HavenQuest.LiveHouse(gm, run.Flag("nursehouse", -1));
		RZ_Safehouse walkHouse = RZ_HavenQuest.LiveHouse(gm, run.Flag("walkhouse", -1));
		if (stage == S_NUMBERS && gm.HasLab())
			run.SetPlace(gm.GetLabPosition(), run.Text("lab"));
		else if ((stage == S_OFFERS || stage == S_LATER) && nurseHouse)
			run.SetPlace(nurseHouse.m_vPos, nurseHouse.m_sName);
		else if (walkHouse)
			run.SetPlace(walkHouse.m_vPos, walkHouse.m_sName);

		if (stage == S_WALK && run.Visits(S_WALK) <= 1)
		{
			run.Note("{walker} has walked eleven people out of {walkhouse}. He comes back alone, and doesn't talk about it.");
			run.Note("{nurse}, in {nursehouse}: \"Eight in ten of them could have walked out of the lab instead. Eight in ten.\"");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The nurse's house emptied: she's gone to the walker's house, and the stages meet her there.
	protected void CheckNurse(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		int nurseHouse = run.Flag("nursehouse", -1);
		int walkHouse = run.Flag("walkhouse", -1);
		if (nurseHouse == walkHouse || RZ_HavenQuest.LiveHouse(gm, nurseHouse) || !RZ_HavenQuest.LiveHouse(gm, walkHouse))
			return;

		run.Note("{nursehouse} stands empty now. {nurse} has moved to {walkhouse}, and brought her bag.");
		run.SetFlag("nursehouse", walkHouse);
		run.SetText("nursehouse", run.Text("walkhouse"));
	}

	//------------------------------------------------------------------------------------------------
	//! The walker's house must have people, the cure mustn't be out (then nobody is walked out any
	//! more), and the walks happen at dusk.
	override bool IsReady(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (gm.GetCure().IsCured())
		{
			run.SetText("why", "The cure is out. Nobody walks anyone to the edge of town any more.");
			return false;
		}

		if (!RZ_HavenQuest.LiveHouse(gm, run.Flag("walkhouse", -1)))
		{
			run.SetText("why", run.Fill("{walkhouse} stands empty now. There's nobody left there to walk anyone out, or to be walked."));
			return false;
		}

		if (stage == S_WALK || (stage == S_REAL && run.m_iBranch == BRANCH_WALKS))
			return RZ_HavenQuest.IsDusk();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override RZ_Mission MakeMission(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		switch (stage)
		{
			case S_WALK: return new RZ_QM_EdgeWalk();
			case S_TALLY: return new RZ_QM_EdgeTally();
			case S_NUMBERS: return new RZ_QM_EdgeNumbers();
			case S_OFFERS: return new RZ_QM_EdgeOffers();
			case S_REAL: return RealMission(run);
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Mission RealMission(RZ_QuestRun run)
	{
		if (run.m_iBranch == BRANCH_LAB)
			return new RZ_QM_EdgeColdChain();

		if (run.m_iBranch == BRANCH_CUT)
			return new RZ_QM_EdgeCutDose();

		RZ_QM_EdgeWalk last = new RZ_QM_EdgeWalk();
		last.m_bLastWalk = true;
		return last;
	}

	//------------------------------------------------------------------------------------------------
	override int Next(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		switch (stage)
		{
			case S_WALK:
			{
				if (outcome == OUT_LAB_SAVED)
				{
					run.SetFlag("lab_saved", 1);
					gm.GetQuestlines().SetWorldFlag("walk.lab_saved", 1);
					run.Note("{man} walked out of {lab} on his own feet, with a dressing on his arm and a number on his file. He's back in {walkhouse}.");
				}
				else if (outcome == OUT_LAB_LOST)
				{
					run.SetFlag("lab_lost", 1);
					run.Note("The trial dose didn't take for {man}. Halcyon wrote him down. {walker} says that's what they do with the other two.");
				}
				else if (outcome == OUT_WALKED || outcome == OUTCOME_DONE)
				{
					run.SetText("count", "Twelve");
					run.Note("{man} was walked out at dusk, the way {walkhouse} does it. Twelve, now.");
				}
				else
				{
					run.Note("Nobody walked {man} out. He waited at the edge of town as long as he could.");
					if (run.Visits(S_WALK) < 2)
					{
						// Another dusk, another bite declared
						run.SetText("man", RZ_HavenQuest.ManName(new RZ_Rng(Math.RandomInt(1, 2000000000))));
						return S_WALK;
					}
				}

				return S_TALLY;
			}
			case S_TALLY:
			{
				if (outcome == OUT_TALLY || outcome == OUTCOME_DONE)
				{
					run.Note("{count} graves at the edge of {walkhouse}, and the same names in {walker}'s notebook, with the day and what they asked for last.");
					if (run.Flag("elder_asked") == 1)
						run.Note("{walker} has asked someone old enough to know how it's done to walk the next one with him.");

					return S_NUMBERS;
				}

				if (run.Visits(S_TALLY) < 2)
					return S_TALLY;

				run.Note("Nobody got {walker}'s tally. The graves are still there, if anyone wants to count them.");
				return S_NUMBERS;
			}
			case S_NUMBERS:
			{
				if (outcome == OUT_KNOWS || outcome == OUTCOME_DONE)
				{
					gm.GetQuestlines().SetWorldFlag("walk.knows_other_two", 1);
					run.Note("The other two in ten: the lab keeps them, watches them turn, and writes it all down. Names on the first page, numbers after.");
					return S_OFFERS;
				}

				if (run.Visits(S_NUMBERS) < 2)
					return S_NUMBERS;

				run.Note("The lab kept its numbers to itself. The houses will have to decide without them.");
				return S_OFFERS;
			}
			case S_OFFERS: return AfterOffers(run, outcome);
			case S_REAL: return AfterReal(run, outcome);
			case S_LATER:
				return Finish(run, E_CUT, "The cut dose. Half of them walked out, and the Red Hand calls that a good rate.");
		}

		return Fizzle(run, "");
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterOffers(RZ_QuestRun run, int outcome)
	{
		if (outcome == OUT_LAB_ROAD)
		{
			run.SetBranch(BRANCH_LAB);
			run.SetText("realtext", run.Fill("The lab road is agreed. Now it has to be real: a batch of antidote from {lab} to the clinic in {nursehouse}, before it spoils."));
			run.Note("The lab road: the lab takes every bitten Haven member who'll come, and sells doses to the Haven's clinics at fair prices.");
			return S_REAL;
		}

		if (outcome == OUT_CUT)
		{
			run.SetBranch(BRANCH_CUT);
			run.SetText("realtext", run.Fill("The Red Hand's cut dose is cheap, and it's for every house. A crate of it has to get from a Red Hand drop to {nursehouse} without Halcyon's courier seeing."));
			run.Note("The cut dose: cheap, for every house, and nobody says how much it's been cut.");
			return S_REAL;
		}

		run.SetBranch(BRANCH_WALKS);
		run.SetText("realtext", run.Fill("The walks stay. {walker} walks one more out of {walkhouse} at dusk, and he's asked you to walk with him."));
		run.Note("Both offers refused. In {walkhouse}, somebody will still hold your hand at the edge of town.");
		return S_REAL;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterReal(RZ_QuestRun run, int outcome)
	{
		bool done = outcome == OUTCOME_DONE || outcome == OUT_DELIVERED;
		if (done && run.m_iBranch == BRANCH_LAB)
			return Finish(run, E_LAB_ROAD, "The lab road. The first batch reached the clinic in {nursehouse}, and the bitten go to the lab now.");

		if (done && run.m_iBranch == BRANCH_CUT)
		{
			run.Note("The crate reached {nursehouse}. The houses are giving it out.");
			return S_LATER;
		}

		if (done)
			return Finish(run, E_WALKS, "The walks stay. {walker} came back alone again, and this time he talked about it.");

		if (run.Visits(S_REAL) < 2)
			return S_REAL;

		if (run.m_iBranch == BRANCH_LAB)
			return Finish(run, E_WALKS, "The walks stay. The lab road never got going, so {walkhouse} kept walking people to the edge of town.");

		if (run.m_iBranch == BRANCH_CUT)
			return Finish(run, E_WALKS, "The walks stay. The Red Hand's crate never came, and the walks went on.");

		return Finish(run, E_WALKS, "The walks stay. {walker} walked the next one out on his own, as he always has.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
		RZ_Questlines q = gm.GetQuestlines();
		switch (ending)
		{
			case E_LAB_ROAD:
			{
				q.SetWorldFlag("walk.lab_road", 1);
				ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 5, 0);
				ChangeStanding(gm, run, RZ_EFaction.INSTITUTE, 5, 0);
				gm.GetCure().AddResearch(2, run.Fill("The first of the Haven's bitten came to the lab of their own accord."));
				NudgeMorale(gm, RZ_EFaction.SURVIVORS, -4);
				if (q.WorldFlag("other_two.numbers") == 1)
					Headline(gm, "Halcyon has written up its first trial patients from the Haven. Two in ten, numbered, not named. The houses read the numbers at supper anyway.");
				else
					Headline(gm, run.Fill("Halcyon has written up the first of its Haven patients who didn't walk out. {walkhouse} read their names at supper."));

				// A truce between the Haven and Halcyon, if the usual rules let one start
				RZ_FactionLife life = gm.GetFactionLife();
				if (life && life.IsActive())
				{
					life.StartRelation(RZ_EFaction.SURVIVORS, RZ_EFaction.INSTITUTE, RZ_FactionLife.REL_TRUCE);
					if (life.HasTruce(RZ_EFaction.SURVIVORS, RZ_EFaction.INSTITUTE))
						q.SetWorldFlag("walk.truce", 1);
				}

				SpreadRumour(gm, "\"The Haven sends its bitten to the lab now. Most of them come back. The ones who don't get a number.\"", 8);
				return;
			}
			case E_WALKS:
			{
				q.SetWorldFlag("walk.walks_stay", 1);
				ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 10, 0);
				NudgeMorale(gm, RZ_EFaction.INSTITUTE, -3);
				Headline(gm, run.Fill("{walker} still walks people out of {walkhouse} at dusk. The house says his name now, at supper, with the others."), false);
				SpreadRumour(gm, run.Fill("\"In {walkhouse} they still walk you out, if it comes to it. {walker} holds your hand. Better than a number.\""), 8);
				return;
			}
			case E_CUT:
			{
				q.SetWorldFlag("walk.cut_dose", 1);
				ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 0, 10, RZ_EQuestApproach.SIDE);
				RZ_HavenQuest.TakePeople(gm, run.Flag("walkhouse", -1), 2);
				Headline(gm, "Half of them walked out. The Red Hand calls that a good rate.");
				SpreadRumour(gm, "\"The Red Hand's dose works half the time. The houses owe them for every one of it.\"", 8);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override string Rumour(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.m_iStage == S_TALLY || run.m_iStage == S_NUMBERS)
			return "\"In {walkhouse} they walk you to the edge of town at dusk, if you're bitten. Somebody comes back alone. Every time.\"";

		if (run.m_iStage == S_OFFERS)
			return "\"The lab wants the Haven's bitten. The Red Hand wants to sell the Haven a cheap cure. Guess which one the houses can afford.\"";

		return "";
	}
}

//------------------------------------------------------------------------------------------------
//! The Edge of Town, stages 1 and 5 ("the walks stay"): the contract Walk Him Out
//! (Missions/Haven/RZ_HavenHouseMissions.c) at the walker's house. Stage 1 adds a third choice:
//! take him to the lab for the trial dose (he follows you as an escort; at the lab 8 in 10 he walks
//! out and goes home, else Halcyon writes him down). Stage 5 (m_bLastWalk) is the walker's last
//! walk, with you.
class RZ_QM_EdgeWalk : RZ_Mission_WalkHimOut
{
	bool m_bLastWalk;
	protected ref RZ_Escort m_Escort = new RZ_Escort();
	protected bool m_bToLab;
	protected int m_iLed = -1;

	protected static const float LAB_RADIUS = 40;
	protected static const float SAVED_CHANCE = 0.8;

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_QuestRun Run(RZ_GameModeComponent gm)
	{
		if (!gm.GetQuestlines())
			return null;

		return gm.GetQuestlines().FindRun(m_iQuestRun);
	}

	//------------------------------------------------------------------------------------------------
	override protected int PickHouse(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run)
			return -1;

		return run.Flag("walkhouse", -1);
	}

	//------------------------------------------------------------------------------------------------
	override protected string PickName(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run)
			return super.PickName(gm);

		if (m_bLastWalk)
			return run.Text("next");

		return run.Text("man");
	}

	//------------------------------------------------------------------------------------------------
	override protected string ChoiceLabels(RZ_GameModeComponent gm)
	{
		if (m_bLastWalk || !gm.HasLab())
			return super.ChoiceLabels(gm);

		return "End it here|Let him walk on|Take him to the lab for the trial dose";
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run || !super.SetupPuzzle(gm))
			return false;

		if (m_bLastWalk)
		{
			m_sTitle = "The Edge of Town: the last walk";
			m_sBrief = run.Fill(string.Format("%1 told the council about his bite himself. {walker} walks him out past the edge of {walkhouse} at dusk, and has asked for you to walk with them. At the edge, %1 will ask for one last thing. {walker} will let you give it.", m_sMan));
			return true;
		}

		m_sTitle = "The Edge of Town: one more walk";
		m_sBrief += run.Fill(" {nurse} says there's another way: the lab's trial dose, if he gets there before he turns.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0 || m_bToLab)
			return;

		if (option == 2 && !m_bLastWalk && gm.HasLab())
		{
			ToLab(gm, playerId);
			return;
		}

		super.OnChoice(gm, playerId, objIdx, option, source);
		if (m_eState != RZ_EMissionState.SUCCEEDED)
			return;

		RZ_QuestRun run = Run(gm);
		if (!run)
			return;

		string what = "ended it at the edge of town";
		if (option == 1)
			what = "let him walk on";

		run.Did(gm, playerId, RZ_EQuestApproach.TALK, what);
		run.SetOutcome(RZ_Quest_EdgeOfTown.OUT_WALKED);
	}

	//------------------------------------------------------------------------------------------------
	//! He follows the player to the lab: the figure at the edge of town becomes an escort.
	protected void ToLab(RZ_GameModeComponent gm, int playerId)
	{
		if (!IsEligible(gm, playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 looks at you and shakes his head. \"The council didn't send you. I'll wait for the one they did.\"", m_sMan));
			return;
		}

		if (!RZ_Escort.HasRoom(gm))
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 looks at the road and the dark coming down. \"Not tonight. Not that far.\" (Too many people on the roads under escort right now.)", m_sMan));
			return;
		}

		RZ_PuzzleObjectComponent comp;
		if (m_mObjects.Find(0, comp) && comp)
		{
			RZ_Util.DeleteUnit(comp.GetOwner());
			m_mObjects.Remove(0);
		}

		m_Escort.m_sName = m_sMan;
		m_Escort.m_sPrefab = RZ_Escort.PrefabFor(gm, 0);
		m_Escort.SetTalk(m_iId, 0, "", m_iTalk, "Talk to " + m_sMan);
		if (!m_Escort.Begin(gm, m_vPos))
			return;

		m_bToLab = true;
		m_iLed = playerId;
		m_Escort.Follow(playerId);
		m_Escort.Go();
		m_aParticipants.Clear();
		m_aParticipants.Insert(playerId);
		RZ_QuestRun run = Run(gm);
		if (run)
			run.Did(gm, playerId, RZ_EQuestApproach.TALK, "took him to the lab");

		gm.NotifyPlayer(playerId, m_sTitle, string.Format("\"The lab?\" %1 thinks about it for as long as he can afford to. \"All right. Walk fast.\" %2: %3.", m_sMan, RZ_Quest_EdgeOfTown.LabName(gm), RZ_Util.DirectionText(m_vPos, gm.GetLabPosition())));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		super.OnUpdate(gm, dt, ids, players);
		if (!m_bToLab || m_eState != RZ_EMissionState.ACTIVE)
			return;

		m_Escort.Update(gm, dt, ids, players);
		m_vPos = m_Escort.m_vPos;
		if (m_Escort.IsDead())
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (!m_Escort.IsNear(gm.GetLabPosition(), LAB_RADIUS))
			return;

		m_Escort.Arrive();
		RZ_QuestRun run = Run(gm);
		int outcome = RZ_Quest_EdgeOfTown.OUT_LAB_LOST;
		string text = string.Format("Halcyon's people take %1 in without a fuss. An hour later a researcher comes out with a clipboard and doesn't look at you. \"It didn't take. We'll take care of him from here. He'll be in the report.\"", m_sMan);
		if (Math.RandomFloat01() < SAVED_CHANCE)
		{
			outcome = RZ_Quest_EdgeOfTown.OUT_LAB_SAVED;
			text = string.Format("Halcyon's people take %1 in without a fuss. Two hours later he walks out on his own, grey, with a dressing on his arm and a number on his file. \"I'm going home,\" he says, as if it's a surprise.", m_sMan);
			if (run)
				RZ_Questline.ChangePopulation(gm, run.Flag("walkhouse", -1), 1);
		}
		else
		{
			m_fRewardScale = 0.5;
		}

		NotifyParticipants(gm, m_sTitle, text);
		if (run)
			run.SetOutcome(outcome);

		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		if (m_bToLab)
			return string.Format("%1, %2\n%3: %4", m_sMan, RZ_Util.DirectionText(from, m_vPos), RZ_Util.Capitalize(RZ_Quest_EdgeOfTown.LabName(gm)), RZ_Util.DirectionText(from, gm.GetLabPosition()));

		return super.GetWhereText(gm, from);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		if (m_bToLab)
			return string.Format("The Edge of Town: %1 reached the lab.", m_sMan);

		return string.Format("The Edge of Town: %1 was walked out.", m_sMan);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (m_bToLab)
			return string.Format("The Edge of Town: %1 didn't reach the lab in time.", m_sMan);

		return super.GetFailText(gm);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Escort.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! The Edge of Town, stage 2 (Haven contract, Known, ★ Elder): the walker's tally, at the edge of the
//! walker's town. Three graves to stand at (5 s each) and the tally itself: ask {walker} for the
//! names, or read his notebook by the graves while he's on watch (more than 15 m from him, nothing
//! has noticed you; seen, he takes it back and won't talk to you). Both the graves and the tally
//! end the stage. ★ Elder: he asks you to walk the next one with him.
class RZ_QM_EdgeTally : RZ_HavenQM
{
	protected vector m_vEdge;
	protected vector m_vWalker;
	protected vector m_vBook;
	protected ref array<vector> m_aGraves = {};
	protected ref array<float> m_aStood = {};
	protected int m_iGravesDone;
	protected bool m_bTally;
	protected int m_iTallyBy = -1;
	protected int m_iTallyHow = RZ_EQuestApproach.TALK;
	protected ref array<int> m_aCaught = {};
	protected int m_iWalkerTalk = -1;
	protected int m_iBook = -1;

	protected static const int OBJ_WALKER = 0;
	protected static const float GRAVE_SECONDS = 5;
	protected static const float SEEN_RADIUS = 15;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 10, 10);
		m_Req.Bonus("elder");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, QFlag(gm, "walkhouse", -1));
		RZ_Settlement s;
		if (h)
			s = gm.GetSettlement(h.m_iSettlement);

		if (!h || !s)
			return false;

		float dir = m_Rng.Int(0, 8) * 45;
		m_vEdge = LandPointNear(RZ_Compass.PointAt(s.m_vCenter, dir, s.m_fRadius + 60), 0, 15);
		m_vWalker = LandPointNear(RZ_Compass.PointAt(s.m_vCenter, dir, s.m_fRadius + 10), 0, 8);
		m_vBook = LandPointNear(m_vEdge, 3, 6);
		for (int i = 0; i < 3; i++)
		{
			m_aGraves.Insert(LandPointNear(m_vEdge, 6, 18));
			m_aStood.Insert(0);
		}

		m_iSettlement = h.m_iSettlement;
		m_vPos = m_vEdge;
		m_iWalkerTalk = AddClue(QText(gm, "walker"), QFill(gm, "{walker} is on watch at the end of the road, where it turns to track. \"{count}. I don't do it because I like it. I do it because somebody has to hold their hand, and the lab doesn't.\""));
		m_iBook = AddClue("His notebook", QFill(gm, "A school exercise book, wrapped in oilcloth and tucked under a stone. The names, one to a page, each with a day and a line: what they asked for last. \"A cigarette.\" \"To see the sea.\" \"For you to look away.\" On the inside cover, in another hand, a nurse's: \"Eight in ten, {walker}. Eight.\""));
		m_sTitle = "The Edge of Town: the walker's tally";
		m_sBrief = QFill(gm, "{walker} keeps a grave for everyone he's walked out of {walkhouse}, at the edge of town, and their names in a notebook. I want the tally. Count the graves, and get the names, one way or another.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		if (!SpawnPerson(gm, m_vWalker, OBJ_WALKER, m_iWalkerTalk, "Talk to " + QText(gm, "walker"), "Ask him to say their names"))
			return false;

		foreach (vector g : m_aGraves)
		{
			SpawnProp(gm, RZ_EPuzzleProp.OBJECT, g);
		}

		m_Quiet.Watch(m_vBook, 80);
		return SpawnClue(gm, RZ_EPuzzleProp.CLUE, m_vBook, m_iBook, "Read the notebook") != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bSceneUp || m_bResolved)
			return;

		foreach (int i, vector g : m_aGraves)
		{
			if (m_aStood[i] >= GRAVE_SECONDS || CountPlayersNear(players, g, 5) == 0)
				continue;

			m_aStood[i] = m_aStood[i] + dt;
			if (m_aStood[i] < GRAVE_SECONDS)
				continue;

			m_iGravesDone++;
			RZ_ContractHelp.AddNear(ids, players, g, 5, m_aParticipants);
			RZ_ContractHelp.NotifyNear(gm, g, 20, m_sTitle, string.Format("A grave, a stick for a cross, a name cut into it with a knife. %1 of 3.", m_iGravesDone));
		}

		CheckDone(gm);
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckDone(RZ_GameModeComponent gm)
	{
		if (!m_bTally || m_iGravesDone < m_aGraves.Count())
			return;

		Resolve(gm, m_iTallyBy, m_iTallyHow, RZ_Quest_EdgeOfTown.OUT_TALLY, "got the walker's tally");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_WALKER || option != 0 || !MayDecide(gm, playerId, QText(gm, "walker")))
			return;

		if (m_aCaught.Contains(playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{walker} doesn't look at you. \"You've read them already. Go and count the graves if you want more.\""));
			return;
		}

		m_bTally = true;
		m_iTallyBy = playerId;
		m_iTallyHow = RZ_EQuestApproach.TALK;
		Remember(gm, playerId, RZ_EQuestApproach.TALK, "asked him for the names");
		string text = QFill(gm, "He says them all, in order, without stopping, the way you'd say a prayer you learned as a child. Then he lights a cigarette. \"{count}.\"");
		if (RZ_QuestApproach.HasBackground(gm, playerId, "elder"))
		{
			SetQFlag(gm, "elder_asked", 1);
			text += QFill(gm, "\n\"You're old enough to know how it's done.\" He looks at you properly. \"Walk the next one with me. They should see somebody's face who isn't afraid.\"");
		}

		gm.NotifyPlayer(playerId, m_sTitle, text);
		if (m_iGravesDone < m_aGraves.Count())
			gm.NotifyPlayer(playerId, m_sTitle, "The graves are a little way past him, at the edge of town. Stand at each a moment.");

		CheckDone(gm);
	}

	//------------------------------------------------------------------------------------------------
	//! The notebook, unseen: well away from him, with nothing noticing you.
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		if (idx != m_iBook || m_bTally)
			return;

		bool seen = !m_Quiet.IsQuiet(gm, playerId) || IsNear(playerId, m_vWalker, SEEN_RADIUS);
		if (seen)
		{
			if (!m_aCaught.Contains(playerId))
				m_aCaught.Insert(playerId);

			Remember(gm, playerId, RZ_EQuestApproach.SNEAK, "was seen at his notebook");
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{walker} is beside you before you've turned a page. He takes the book out of your hands, wraps it again and puts it in his coat. He doesn't say anything at all."));
			return;
		}

		m_bTally = true;
		m_iTallyBy = playerId;
		m_iTallyHow = RZ_EQuestApproach.SNEAK;
		Remember(gm, playerId, RZ_EQuestApproach.SNEAK, "read his notebook while he was on watch");
		if (m_iGravesDone < m_aGraves.Count())
			gm.NotifyPlayer(playerId, m_sTitle, "You put it back under the stone. The graves are all around you. Stand at each a moment.");

		CheckDone(gm);
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		foreach (vector g : m_aGraves)
		{
			result.Insert(g);
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Edge of Town: nobody got {walker}'s tally.");
	}
}

//------------------------------------------------------------------------------------------------
//! The Edge of Town, stage 3 (Haven contract, Known): the lab's numbers, at the lab. A night-shift
//! field lead (persuade Halcyon, difficulty 2; ★ Lab Tech/Doctor +1), the trial records (read after
//! dark, unseen: nothing noticed you there, no shots; seen: +8 Halcyon Infamy and no look), or a
//! courier at the gate (20 Halcyon Favor). Any of them: walk.knows_other_two.
class RZ_QM_EdgeNumbers : RZ_HavenQM
{
	protected vector m_vLead;
	protected vector m_vRecords;
	protected vector m_vCourier;
	protected int m_iLeadTalk = -1;
	protected int m_iRecords = -1;
	protected int m_iCourierTalk = -1;
	protected ref array<int> m_aSeen = {};

	protected static const int OBJ_LEAD = 0;
	protected static const int OBJ_COURIER = 1;
	protected static const int COURIER_FAVOR = 20;
	protected static const int SEEN_INFAMY = 8;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 12, 10);
		m_Req.Bonus("labtech");
		m_Req.Bonus("doctor");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		if (!gm.HasLab())
			return false;

		vector lab = gm.GetLabPosition();
		m_iSettlement = gm.GetLabSettlement();
		m_vLead = LandPointNear(lab, 10, 18);
		m_vRecords = LandPointNear(lab, 4, 9);
		m_vCourier = LandPointNear(lab, 40, 60);
		m_vPos = m_vLead;
		m_iLeadTalk = AddClue("The field lead", "A tired woman in a lab coat over two jumpers, on her third coffee of the night shift. \"The Haven? You want to know about the trial.\" She doesn't say yes or no.");
		m_iRecords = AddClue("The trial records", "Ring binders on a trolley under a dim bulb. The first pages of each file have a name. After the first week, the files have numbers. Every file has two columns: walked out, and observed. The observed column has times in it, every quarter of an hour, until it stops.");
		m_iCourierTalk = AddClue("A courier", "A Halcyon courier with a satchel she's not supposed to open, waiting for the gate. \"Files? I carry them. I don't read them.\" A pause. \"Mostly.\"");
		m_sTitle = "The Edge of Town: the lab's numbers";
		m_sBrief = QFill(gm, "{walker} says the lab writes down the other two in ten. I want to know what that means before I tell another house to send its bitten to {lab}. Find out. Nicely, if you can.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		if (!SpawnPerson(gm, m_vLead, OBJ_LEAD, m_iLeadTalk, "Talk to the field lead", "Ask about the other two in ten"))
			return false;

		SpawnPerson(gm, m_vCourier, OBJ_COURIER, m_iCourierTalk, "Talk to the courier", string.Format("Pay for a look at the file (%1 Halcyon Favor)", COURIER_FAVOR));
		m_Quiet.Watch(m_vRecords, 60);
		return SpawnClue(gm, RZ_EPuzzleProp.CLUE, m_vRecords, m_iRecords, "Read the trial records") != null;
	}

	//------------------------------------------------------------------------------------------------
	protected void Learned(RZ_GameModeComponent gm, int playerId, int approach, string what)
	{
		NotifyParticipants(gm, m_sTitle, "The other two in ten stay at the lab. They're watched, and written up every quarter of an hour, until there's nothing left to write. Names on the first page. Numbers after the first week.");
		Resolve(gm, playerId, approach, RZ_Quest_EdgeOfTown.OUT_KNOWS, what);
	}

	//------------------------------------------------------------------------------------------------
	override string GetClueText(RZ_GameModeComponent gm, int playerId, int idx)
	{
		if (idx == m_iRecords && !RZ_HavenQuest.IsDark())
			return "Ring binders on a trolley, and a researcher sitting right next to them, wide awake. On the night shift, when the lights go down, nobody sits here.";

		return super.GetClueText(gm, playerId, idx);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		if (idx != m_iRecords || !RZ_HavenQuest.IsDark() || m_aSeen.Contains(playerId))
			return;

		if (!m_Quiet.IsQuiet(gm, playerId))
		{
			m_aSeen.Insert(playerId);
			gm.AddInfamy(playerId, RZ_EFaction.INSTITUTE, SEEN_INFAMY);
			Remember(gm, playerId, RZ_EQuestApproach.SNEAK, "was caught at the records");
			gm.NotifyPlayer(playerId, "Halcyon", string.Format("%1 The binders are snapped shut and taken away. (+%2 Halcyon Infamy)", m_Quiet.Why(gm, playerId), SEEN_INFAMY));
			return;
		}

		Learned(gm, playerId, RZ_EQuestApproach.SNEAK, "read the trial records on the night shift");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (option != 0 || !MayDecide(gm, playerId, "The night shift"))
			return;

		if (objIdx == OBJ_LEAD)
		{
			if (!TryPersuade(gm, playerId, RZ_EFaction.INSTITUTE, 2, "labtech,doctor"))
			{
				Remember(gm, playerId, RZ_EQuestApproach.PERSUADE, "asked the field lead, and was told nothing");
				gm.NotifyPlayer(playerId, m_sTitle, "\"I can't discuss trial outcomes with the public.\" She means it kindly. She also means it.");
				return;
			}

			gm.NotifyPlayer(playerId, m_sTitle, "She looks at her coffee. \"We keep them. We watch. We write it down, every quarter of an hour, because it's the only thing that makes the next dose better. Two in ten. I know their names. I'm not supposed to.\"");
			Learned(gm, playerId, RZ_EQuestApproach.PERSUADE, "got it from a field lead");
			return;
		}

		if (objIdx == OBJ_COURIER && TryBribe(gm, playerId, RZ_EFaction.INSTITUTE, COURIER_FAVOR))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "The courier opens the satchel a hand's width and lets you read the top sheet in the light from the gate. \"You didn't see that. I didn't show you.\"");
			Learned(gm, playerId, RZ_EQuestApproach.BRIBE, "paid a courier for a look");
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The Edge of Town: the lab kept its numbers to itself.";
	}
}

//------------------------------------------------------------------------------------------------
//! The Edge of Town, stage 4 (Haven contract, Known; the stage waits two hours of play first): the
//! two offers, at the nurse's house. Radio Operators hear the director's offer on the secure
//! channel when the stage opens; everyone hears both from {nurse}. Broker the lab road (Known with
//! Halcyon, and persuade the Haven: difficulty 2, 1 after a man was saved at stage 1, 2 again with
//! weeks.public; ★ Nurse/Doctor +1; needs the mission item and the Haven drop point for stage 5),
//! refuse both, or take the cut dose (Trusted with the Red Hand; the Haven pays nothing for it).
//! Time up: nobody chose, and the walks stay.
class RZ_QM_EdgeOffers : RZ_HavenQM
{
	protected int m_iTalk = -1;

	protected static const int OBJ_NURSE = 0;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 10, 10);
		m_Req.Bonus("nurse");
		m_Req.Bonus("doctor");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, QFlag(gm, "nursehouse", -1));
		if (!h)
			return false;

		m_iSettlement = h.m_iSettlement;
		m_vPos = LandPointNear(h.m_vPos, 4, 10);
		m_iTalk = AddClue(QText(gm, "nurse"), QFill(gm, "{nurse} has both offers written on the back of a ration card. \"{director} says the lab will take every bitten Haven member who'll come, and sell doses to our clinics at fair prices. The Red Hand says it has a cheaper dose, for every house. Cut, they don't say how much.\" She turns the card over. \"Or we keep walking them out. You've seen both ends now. You choose.\""));
		m_sTitle = "The Edge of Town: two offers";
		m_sBrief = QFill(gm, "The lab has made an offer, and so has the Red Hand. The houses can't agree, so they've agreed to let whoever's seen it all decide. Come to {nursehouse}.");

		// The director's offer goes out on the secure channel: Radio Operators who are part of it hear it
		RZ_QuestRun run = Run(gm);
		if (!run)
			return true;

		array<string> idents = {};
		run.GetIdents(idents);
		foreach (string ident : idents)
		{
			int playerId = gm.GetQuestlines().OnlineId(ident);
			if (playerId > 0 && RZ_QuestApproach.HasBackground(gm, playerId, "radioham"))
				gm.NotifyPlayer(playerId, "Secure channel: Halcyon", QFill(gm, "\"{director} to all Haven houses. The lab will accept every bitten member of the Haven who presents in time. Doses for your clinics at Known prices. Eight in ten. We will be honest about the other two.\""));
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		string lab = "";
		if (RZ_Mission_Delivery.CanRun(gm, RZ_EFaction.SURVIVORS))
			lab = "Broker the lab road";

		string choices = string.Format("%1|Refuse both: the walks stay|Take the Red Hand's cut dose", lab);
		return SpawnPerson(gm, m_vPos, OBJ_NURSE, m_iTalk, "Talk to " + QText(gm, "nurse"), choices) != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_NURSE || !MayDecide(gm, playerId, QText(gm, "nurse")))
			return;

		if (option == 0 && RZ_Mission_Delivery.CanRun(gm, RZ_EFaction.SURVIVORS))
		{
			BrokerLabRoad(gm, playerId);
			return;
		}

		if (option == 1)
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{nurse} nods slowly, as if she knew. \"Then {walker} keeps walking them. And somebody had better walk with him.\""));
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_EdgeOfTown.OUT_REFUSE, "refused both offers");
			return;
		}

		if (option != 2)
			return;

		if (!RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.TRUSTED))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "The Red Hand's offer is only good through someone it trusts. That isn't you, yet.");
			return;
		}

		RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.BANDITS, -1, 5, 0);
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{nurse} looks at you for a long time. \"Half price for half a cure.\" She writes it down anyway."));
		Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_EdgeOfTown.OUT_CUT, "took the Red Hand's offer", false);
	}

	//------------------------------------------------------------------------------------------------
	protected void BrokerLabRoad(RZ_GameModeComponent gm, int playerId)
	{
		if (!RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.INSTITUTE), RZ_ETier.KNOWN))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "The lab won't make a deal through somebody it doesn't know.");
			return;
		}

		// A man who walked out of the lab is proof; the three weeks made public (HC1) undo it
		int difficulty = 2;
		if (QFlag(gm, "lab_saved") == 1)
			difficulty = 1;

		if (gm.GetQuestlines().WorldFlag("weeks.public") == 1)
			difficulty = 2;

		if (!RZ_QuestApproach.Persuade(gm, playerId, RZ_EFaction.SURVIVORS, difficulty, "nurse,doctor"))
		{
			Remember(gm, playerId, RZ_EQuestApproach.PERSUADE, "argued for the lab road, and lost");
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "\"The houses won't send their own to {lab} on your word.\" {nurse} isn't unkind about it. \"They'd need to trust you more, or see someone come back.\""));
			return;
		}

		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{nurse} writes \"lab\" on the ration card and underlines it twice. \"I'll tell the houses. You tell {lab}. The first batch has to come soon, or nobody will believe it.\""));
		Resolve(gm, playerId, RZ_EQuestApproach.PERSUADE, RZ_Quest_EdgeOfTown.OUT_LAB_ROAD, "brokered the lab road");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Edge of Town: nobody chose, so the walks stay in {walkhouse}.");
	}
}

//------------------------------------------------------------------------------------------------
//! The Edge of Town, stage 5 on the lab road: the contract Cold Chain
//! (Missions/Halcyon/RZ_HalcyonContracts.c) to the nurse's clinic, with no cure threshold. Its
//! success is the stage's. Needs the mission item and the Haven drop point.
class RZ_QM_EdgeColdChain : RZ_Mission_ColdChain
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		super.Configure(gm);
		m_Req.m_fMinCure = 0;
		m_iFameReward = 12;
		m_iFavorReward = 15;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = gm.GetQuestlines().FindRun(m_iQuestRun);
		if (!run || !gm.HasLab() || !CanRun(gm, RZ_EFaction.SURVIVORS))
			return false;

		RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, run.Flag("nursehouse", -1));
		if (!h)
			return false;

		vector lab = gm.GetLabPosition();
		m_vPos = LandPointNear(lab, 5, 10);
		m_iSettlement = gm.GetLabSettlement();
		if (!SpawnItem(gm, m_vPos) || !SpawnDrop(gm, RZ_EFaction.SURVIVORS, LandPointNear(h.m_vPos, 4, 10)))
			return false;

		RZ_Leader director = RZ_HavenQuest.KeptLeader(gm, run, "director");
		if (director)
			m_iLeaderId = director.m_iId;

		m_sTitle = "The Edge of Town: the lab road";
		m_sBrief = run.Fill("The first batch for the Haven's clinics, as agreed: antidote for {nurse}'s clinic in {nursehouse}. Fifteen minutes out of the cold box. If it spoils, nobody will believe in the lab road. Run.");
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! The Edge of Town, stage 5 on the cut dose (Red Hand contract, Known): the crate at the nearest
//! Red Hand seat, carried by one player to the nurse's house, past Halcyon's courier on the road
//! (within 35 m, 25 m for ★ Ex-Con: +8 Halcyon Infamy, and the lab knows). The carrier hands it over.
class RZ_QM_EdgeCutDose : RZ_HavenQM
{
	protected vector m_vCrate;
	protected vector m_vClinic;
	protected bool m_bClinicUp;
	protected bool m_bSeen;
	protected ref RZ_HavenLookouts m_Courier = new RZ_HavenLookouts();

	protected static const int OBJ_CRATE = 0;
	protected static const int OBJ_CLINIC = 1;
	protected static const float SEEN_RADIUS = 35;
	protected static const float SEEN_RADIUS_BONUS = 25;
	protected static const int SEEN_INFAMY = 8;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 6, 10);
		m_Req.Bonus("excon");
		m_fTimeLeft = 3000;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, QFlag(gm, "nursehouse", -1));
		if (!h)
			return false;

		RZ_Leader boss = RZ_HavenQuest.NearestSeat(gm, RZ_EFaction.BANDITS, h.m_vPos, 8000);
		if (!boss)
			return false;

		m_iLeaderId = boss.m_iId;
		m_vCrate = LandPointNear(boss.m_vPos, 120, 220);
		m_vClinic = LandPointNear(h.m_vPos, 4, 10);
		m_vPos = m_vCrate;
		m_iSettlement = gm.FindNearestSettlement(m_vCrate);
		m_Courier.m_sWho = "Halcyon's courier, on the road with her satchel,";
		m_sTitle = "The Edge of Town: the cut dose";
		m_sBrief = QFill(gm, "The Haven's houses want our dose. Cheap, and for everyone. The crate's at our drop outside " + boss.m_sPlace + ". Get it to {nursehouse}, and keep away from Halcyon's courier on the road. The lab would take it personally.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		if (!SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vCrate, OBJ_CRATE, "", "Take the crate", -1, ""))
			return false;

		m_Courier.SpawnAlong(gm, m_vCrate, m_vClinic, -1, 1);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_iCarrier < 0 || m_bResolved)
			return;

		int carrier = Carrier();
		if (carrier < 0)
		{
			m_iCarrier = -1;
			NotifyParticipants(gm, m_sTitle, "Whoever had the crate is gone. It's back at the drop.");
			return;
		}

		IEntity ent = PlayerEnt(carrier);
		if (!m_bClinicUp && RZ_Util.DistXZ(ent.GetOrigin(), m_vClinic) <= 300)
		{
			m_bClinicUp = true;
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vClinic, OBJ_CLINIC, "", "Hand over the crate", -1, "");
		}

		if (m_bSeen)
			return;

		float radius = SEEN_RADIUS;
		if (HasBonus(gm, carrier))
			radius = SEEN_RADIUS_BONUS;

		string who = m_Courier.SeenBy(gm, ent.GetOrigin(), radius, false);
		if (who.IsEmpty())
			return;

		m_bSeen = true;
		SetQFlag(gm, "lab_saw", 1);
		gm.AddInfamy(carrier, RZ_EFaction.INSTITUTE, SEEN_INFAMY);
		gm.NotifyPlayer(carrier, "Halcyon", string.Format("%1 sees the Red Hand's crate on your shoulder and writes something down. The lab will know by tonight. (+%2 Halcyon Infamy)", who, SEEN_INFAMY));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (option != 0 || m_bResolved)
			return;

		if (objIdx == OBJ_CRATE)
		{
			if (m_iCarrier >= 0 || !MayDecide(gm, playerId, "The crew's man"))
				return;

			GiveTo(playerId);
			Remember(gm, playerId, RZ_EQuestApproach.SIDE, "carried the Red Hand's crate");
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "Heavy for what's in it. {nursehouse}: grid " + RZ_HavenQuest.Grid(m_vClinic) + ". Halcyon's courier uses the same roads."));
			return;
		}

		if (objIdx != OBJ_CLINIC)
			return;

		if (playerId != m_iCarrier)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"Whoever's carrying it brings it in.\"");
			return;
		}

		RZ_QuestRun run = Run(gm);
		if (run)
			run.Reward(gm, playerId, RZ_EFaction.BANDITS, 6, 20, "The Red Hand pays its carriers. The houses will pay the Red Hand.");

		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{nurse} counts the vials and doesn't say anything about the labels. \"Half price,\" she says, to nobody."));
		Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_EdgeOfTown.OUT_DELIVERED, "brought the cut dose to the houses");
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return super.GetWhereText(gm, from) + QFill(gm, "\n{nursehouse}: grid ") + RZ_HavenQuest.Grid(m_vClinic);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Edge of Town: the Red Hand's crate never reached {nursehouse}.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Courier.Cleanup();
	}
}
