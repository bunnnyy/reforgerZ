//! Authors (Halcyon; HC3 in docs/QUESTLINES.md, built in docs/MISSIONS.md, "Questlines"). At half
//! research the cure becomes something a person can own. The director would trade what the lab
//! knows for Bastion's guns at the final stand; a field lead would give the method to the Haven's
//! clinics for nothing; the Red Hand would buy the batch. Where the cure goes is still the season's
//! Fame: this line is about who is standing at the lab when it's made. It adds no research, never
//! moves the stand, and pays its endings only after the stand is over (so after the winner is
//! chosen, when the stand is won).
//!
//! 1. The director's letter (mission): carry her terms from the lab to Bastion's commanding
//!    officer. Read it first, and leave a copy on the field lead's desk unseen. Skipped when HC1
//!    ended in Bastion's file: Bastion already has terms.
//! 2. The field lead's plan (mission): the method, packed cold, to a Haven clinic before it's
//!    missed (20 minutes; Nurse or Doctor 30), report her to the director, or sell it to the Red
//!    Hand (Sell the Formula). Skipped if she's no longer in her chair.
//! 3. The CO's answer (wait one session, then a mission at Bastion's stronghold): agree for the
//!    director, bargain (Trusted with both: two guards at the gate, no key), or refuse (+5 Bastion
//!    Infamy).
//! 4. The Red Hand's offer (mission): turn the boss's man down (the director refuses), take the
//!    money (Red Hand Favor; Take the Lab is offered during the stand), or tell the CO (a Bastion
//!    group on the lab's road during the stand).
//! 5. Ready the lab (mission, from 75% research): Power the Lab, Decontamination or Lab Defence,
//!    picked by the director from the flags.
//! 6. The final stand (wait for it, then a mission that lasts as long as the stand): the Bastion
//!    group at the gate, the road group, and with B3's samples flight one more Halcyon guard.
//!
//! Endings: Under Bastion's guard, Open doors, The Red Hand's batch, The lab alone. Starts at 50%
//! research or more, never during or after the final stand; every stage waits out a stand and
//! none runs after the cure.
class RZ_Quest_Authors : RZ_Questline
{
	static const string KEY = "halcyon_authors";
	static const string FLAG_BASTION_KEY = "halcyon_authors.bastion_key";	// set: Bastion holds a key to the clean room
	static const string FLAG_GUARD = "halcyon_authors.guard";				// set: a Bastion group held the lab gate through the stand
	static const string FLAG_OPEN = "halcyon_authors.open";				// set: the method went to the Haven's clinics for nothing
	static const string FLAG_LEAD_FIRST = "halcyon_authors.lead_first";	// set: the field lead's name is first on the paper
	static const string FLAG_BATCH_SOLD = "halcyon_authors.batch_sold";	// set: the boss paid for the batch
	static const string FLAG_FORMULA_SOLD = "halcyon_authors.formula_sold";	// set: the method was sold to the Red Hand (stage 2)
	// Read from B3 (The Last Pilot, Bastion): the faction the pilot flew for, Halcyon = the samples
	static const string FLAG_PILOT = "bastion_last_pilot.flew_for";

	static const int S_START = 0;
	static const int S_LETTER = 1;
	static const int S_PLAN = 2;
	static const int S_ANSWER_WAIT = 3;
	static const int S_ANSWER = 4;
	static const int S_OFFER = 5;
	static const int S_READY = 6;
	static const int S_STAND_WAIT = 7;
	static const int S_STAND = 8;

	static const int OUT_DELIVERED = 10;
	static const int OUT_HAVEN_COPY = 11;
	static const int OUT_REPORTED = 12;
	static const int OUT_SOLD = 13;
	static const int OUT_MISSED = 14;
	static const int OUT_AGREE = 15;
	static const int OUT_BARGAIN = 16;
	static const int OUT_REFUSE = 17;
	static const int OUT_DECLINED = 18;
	static const int OUT_BETRAYED = 19;
	static const int OUT_TOLD_CO = 20;
	static const int OUT_WON = 21;
	static const int OUT_LOST = 22;
	static const int OUT_STOLEN = 23;

	static const int TERMS_AGREE = 1;
	static const int TERMS_BARGAIN = 2;
	static const int TERMS_REFUSED = 3;

	static const int STAND_WON = 1;
	static const int STAND_LOST = 2;
	static const int STAND_STOLEN = 3;

	static const int TASK_POWER = 1;
	static const int TASK_DECON = 2;
	static const int TASK_DEFENCE = 3;

	static const int E_BASTION = 0;
	static const int E_OPEN = 1;
	static const int E_REDHAND = 2;
	static const int E_ALONE = 3;

	static const float CURE_START = 50;
	static const float CURE_READY = 75;
	static const float CURE_LATE = 90;		// too close to the stand to start: it would only fizzle

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		Info(KEY, "Authors", RZ_EFaction.INSTITUTE);

		RZ_QuestStage start = WaitStage(S_START, "The director's letter", "Research is past half. {director} would trade what the lab knows for Bastion's guns at the final stand.");
		start.Wait(0);

		RZ_QuestStage letter = MissionStage(S_LETTER, "The director's letter", "Research is past half. {director} would trade what the lab knows for Bastion's guns at the final stand. Carry her terms to {co}.");
		letter.GiveUp(4, "Nobody carried the director's letter. Whatever her terms were, Bastion never read them.");
		letter.Visits(2);

		RZ_QuestStage plan = MissionStage(S_PLAN, "The field lead's plan", "{lead} at {lead_place} wants the method at a Haven clinic first, before Bastion locks it in a crate. She has it packed cold for the clinic at the safehouse in {clinic}.");
		plan.GiveUp(4, "{lead}'s cold case stayed on her desk until it was warm.");

		RZ_QuestStage wait = WaitStage(S_ANSWER_WAIT, "The CO's answer", "{co} has the director's terms. Bastion takes its time.");
		wait.Wait(120, 24);

		RZ_QuestStage answer = MissionStage(S_ANSWER, "The CO's answer", "{co} wants a locked crate, a key, and a guard on the clean room. Somebody has to answer him for the lab.");
		answer.GiveUp(4, "Nobody answered {co} for the lab. Bastion took the silence as no.");

		RZ_QuestStage offer = MissionStage(S_OFFER, "The Red Hand's offer", "{boss} has heard what the lab is cooking. He'll buy the batch outright, and leave the lab alone during the final stand.");
		offer.GiveUp(4, "Nobody went to hear the Red Hand's offer. It stands, they say, the way their offers do.");

		RZ_QuestStage ready = MissionStage(S_READY, "Ready the lab", "Past three quarters, the final push is close. {director} wants the lab ready for it.");
		ready.GiveUp(96, "The lab got ready for the final stand without anyone's help.");

		RZ_QuestStage standWait = WaitStage(S_STAND_WAIT, "The final stand", "The lab is ready. When research reaches 100 percent the final stand begins, and whoever stands at the lab then is who the paper remembers.");
		standWait.GiveUp(96, "The final stand didn't come while anyone was waiting for it.");

		RZ_QuestStage stand = MissionStage(S_STAND, "The final stand", "The final stand. Hold the lab while the cure is made.");
		stand.Visits(6);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_CureResearch cure = gm.GetCure();
		if (!gm.HasLab() || !cure || cure.m_fProgress < CURE_START || cure.m_fProgress >= CURE_LATE || cure.IsCured() || cure.IsFinalStand())
			return false;

		if (!RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN) || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.OBJECT))
			return false;

		RZ_Leader director = RZ_QM_HalcyonScene.Top(gm, RZ_EFaction.INSTITUTE);
		RZ_Leader co = RZ_QM_HalcyonScene.Top(gm, RZ_EFaction.MILITARY);
		RZ_Leader lead = RZ_QM_HalcyonScene.PickFieldLead(gm, true);
		int clinic = PickClinic(gm);
		if (!director || !co || !lead || clinic < 0)
			return false;

		RZ_QM_HalcyonScene.StoreLeader(run, "lead", lead);
		RZ_QM_HalcyonScene.StorePos(run, "station", lead.m_vPos);
		run.SetText("director", director.m_sName);
		run.SetText("director_short", director.m_sShort);
		run.SetText("co", co.m_sName);
		run.SetText("boss", RZ_QM_HalcyonScene.TopName(gm, RZ_EFaction.BANDITS, "The Red Hand's boss"));
		run.SetText("chair", RZ_QM_HalcyonScene.TopName(gm, RZ_EFaction.SURVIVORS, "the chair of the Haven council"));
		run.SetFlag("clinic", clinic);
		run.SetText("clinic", gm.GetSafehouses().Get(clinic).m_sName);
		run.SetPlace(gm.GetLabPosition(), "the lab");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A Haven safehouse with a medic for the clinic, else any with people in it. -1 if none.
	static int PickClinic(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses)
			return -1;

		int any = -1;
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			if (!h || h.m_iPopulation <= 0)
				continue;

			if (h.HasMedic())
				return i;

			if (any < 0)
				any = i;
		}

		return any;
	}

	//------------------------------------------------------------------------------------------------
	//! The director voices her stages; the field lead hers, while she's in her chair.
	static void Voice(RZ_GameModeComponent gm, RZ_Mission m, bool byLead)
	{
		if (!gm.GetQuestlines())
			return;

		RZ_Leader l = RZ_QM_HalcyonScene.Top(gm, RZ_EFaction.INSTITUTE);
		if (byLead)
			l = RZ_QM_HalcyonScene.SameLeader(gm, gm.GetQuestlines().FindRun(m.m_iQuestRun), "lead");

		if (l)
			m.m_iLeaderId = l.m_iId;
	}

	//------------------------------------------------------------------------------------------------
	//! Stages before the stand: not during one, never after the cure.
	static bool BeforeStand(RZ_GameModeComponent gm)
	{
		return !gm.GetCure().IsFinalStand() && !gm.GetCure().IsCured();
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		Headline(gm, run.Fill("{director} has started signing her letters \"for the authors\". Nobody at the lab will say who the authors are."), false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStageBegin(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (stage == S_STAND && run.Flag("betrayed") == 1)
			OfferTakeTheLab(gm);
	}

	//------------------------------------------------------------------------------------------------
	//! The boss was paid: his crew is offered Take the Lab during this stand (unless one runs already).
	protected void OfferTakeTheLab(RZ_GameModeComponent gm)
	{
		RZ_MissionManager missions = gm.GetMissions();
		if (missions.IsTypeRunning(RZ_Mission_TakeTheLab) || missions.IsTypeRunning(RZ_QM_AuthorsTakeTheLab))
			return;

		RZ_QM_AuthorsTakeTheLab take = new RZ_QM_AuthorsTakeTheLab();
		take.Configure(gm);
		missions.StartMission(take);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsReady(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		RZ_CureResearch cure = gm.GetCure();
		switch (stage)
		{
			case S_LETTER:
				return BeforeStand(gm);
			case S_OFFER:
				return BeforeStand(gm);
			case S_PLAN:
				return BeforeStand(gm) && RZ_QM_HalcyonScene.SameLeader(gm, run, "lead") != null;
			case S_ANSWER:
				return BeforeStand(gm) && RZ_QM_HalcyonScene.Top(gm, RZ_EFaction.MILITARY) != null;
			case S_READY:
				return BeforeStand(gm) && cure.m_fProgress >= CURE_READY;
			case S_STAND_WAIT:
				return cure.IsFinalStand() || cure.IsCured();
			case S_STAND:
				return cure.IsFinalStand();
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override RZ_Mission MakeMission(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		switch (stage)
		{
			case S_LETTER: return new RZ_QM_AuthorsLetter();
			case S_PLAN: return new RZ_QM_AuthorsPlan();
			case S_ANSWER: return new RZ_QM_AuthorsAnswer();
			case S_OFFER: return new RZ_QM_AuthorsOffer();
			case S_READY: return MakeReady(gm, run);
			case S_STAND: return new RZ_QM_AuthorsStand();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The director picks from the flags: clean power for a Bastion deal, a clean yard for the
	//! Haven's people, and otherwise a lab that can hold on its own. One that can't run, or is
	//! already running as an ordinary contract, gives way to the next.
	protected RZ_Mission MakeReady(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		array<int> order = {};
		int terms = run.Flag("terms");
		if (terms == TERMS_AGREE || terms == TERMS_BARGAIN)
			order.Insert(TASK_POWER);
		else if (run.Flag("haven_copy") == 1)
			order.Insert(TASK_DECON);

		order.Insert(TASK_DEFENCE);
		order.Insert(TASK_POWER);
		order.Insert(TASK_DECON);

		RZ_MissionManager missions = gm.GetMissions();
		foreach (int task : order)
		{
			if (task == TASK_POWER && !gm.GetMissionItemPrefab().IsEmpty() && RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.OBJECT) && !missions.IsTypeRunning(RZ_Mission_PowerTheLab))
			{
				run.SetFlag("task", TASK_POWER);
				return new RZ_QM_AuthorsPower();
			}

			if (task == TASK_DECON && !RZ_ContractHelp.CorpsePilePrefab(gm).IsEmpty() && !missions.IsTypeRunning(RZ_Mission_Decontamination))
			{
				run.SetFlag("task", TASK_DECON);
				return new RZ_QM_AuthorsDecon();
			}

			if (task == TASK_DEFENCE && !missions.IsTypeRunning(RZ_Mission_LabDefence))
			{
				run.SetFlag("task", TASK_DEFENCE);
				return new RZ_QM_AuthorsDefence();
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override int Next(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		switch (stage)
		{
			case S_START:
			{
				// HC1 put the three weeks in Bastion's file: Bastion already has terms
				if (gm.GetQuestlines().WorldFlag(RZ_Quest_ThreeWeeks.FLAG_BASTION) == 1)
				{
					run.SetFlag("early_terms", 1);
					run.Note("{co} has Dr Wolff's notebook in Bastion's file, and named his price before {director} could write a letter.");
					return ToPlan(gm, run);
				}

				return S_LETTER;
			}
			case S_LETTER:
			{
				if (outcome == OUTCOME_FAILED)
				{
					run.Note("The letter is still at the lab.");
					return S_LETTER;
				}

				run.Note("{co} has the director's terms: the method for Bastion alone, in return for guards through the final stand.");
				if (run.Flag("copy") == 1)
					run.Note("{lead} has a copy of the letter. She knows what the director is selling.");

				return ToPlan(gm, run);
			}
			case S_PLAN:
				return AfterPlan(run, outcome);
			case S_ANSWER_WAIT:
			{
				if (RZ_QM_HalcyonScene.Top(gm, RZ_EFaction.MILITARY))
					return S_ANSWER;

				return ToOffer(gm, run);
			}
			case S_ANSWER:
				return AfterAnswer(gm, run, outcome);
			case S_OFFER:
				return AfterOffer(run, outcome);
			case S_READY:
			{
				if (outcome == OUTCOME_FAILED)
				{
					run.Note("The lab isn't as ready as the director wanted. It will have to do.");
				}
				else
				{
					run.SetFlag("ready", 1);
					run.Note("The lab is ready for the final push.");
				}

				return S_STAND_WAIT;
			}
			case S_STAND_WAIT:
			{
				if (gm.GetCure().IsFinalStand())
					return S_STAND;

				// The stand came and went before anyone was standing for the story (or the cure is out)
				return FinishAuthors(gm, run, STAND_WON);
			}
			case S_STAND:
			{
				if (outcome == OUT_WON)
					return FinishAuthors(gm, run, STAND_WON);

				if (outcome == OUT_STOLEN)
					return FinishAuthors(gm, run, STAND_STOLEN);

				if (outcome == OUT_LOST)
					return FinishAuthors(gm, run, STAND_LOST);

				// The stage mission timed out while the stand still runs: back to the stand
				if (gm.GetCure().IsFinalStand())
					return S_STAND;

				if (gm.GetCure().IsCured())
					return FinishAuthors(gm, run, STAND_WON);

				return FinishAuthors(gm, run, STAND_LOST);
			}
		}

		return Fizzle(run, "");
	}

	//------------------------------------------------------------------------------------------------
	protected int ToPlan(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (RZ_QM_HalcyonScene.SameLeader(gm, run, "lead"))
			return S_PLAN;

		run.Note("{lead} isn't at {lead_place} any more. Her plan for the Haven's clinics went with her.");
		return S_ANSWER_WAIT;
	}

	//------------------------------------------------------------------------------------------------
	protected int ToOffer(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (RZ_QM_HalcyonScene.Top(gm, RZ_EFaction.BANDITS))
			return S_OFFER;

		return S_READY;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterPlan(RZ_QuestRun run, int outcome)
	{
		switch (outcome)
		{
			case OUT_HAVEN_COPY:
			{
				run.SetFlag("haven_copy", 1);
				run.Note("The method is at the clinic in {clinic}, written out, for nothing. Bastion's crate will have a copy missing.");
				break;
			}
			case OUT_REPORTED:
			{
				run.SetFlag("reported", 1);
				run.Note("{director} knows about {lead}'s plan. The cold case is back in the lab's cold room.");
				break;
			}
			case OUT_SOLD:
			{
				run.SetFlag("formula_sold", 1);
				run.Note("The method went to the Red Hand instead. Somebody will be selling the cure before there is one.");
				break;
			}
			case OUT_MISSED:
			{
				run.Note("The cold case went warm on the road, and the lab noticed it was gone.");
				break;
			}
			case OUTCOME_FAILED:
			{
				run.Note("Nobody carried {lead}'s cold case anywhere.");
				break;
			}
			default:
			{
				run.SetFlag("haven_copy", 1);
				run.Note("The method is at the clinic in {clinic}, written out, for nothing.");
				break;
			}
		}

		return S_ANSWER_WAIT;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterAnswer(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		switch (outcome)
		{
			case OUT_BARGAIN:
			{
				run.SetFlag("terms", TERMS_BARGAIN);
				run.Note("{co} settled for two guards at the lab gate through the stand, and no key. The lab keeps its own clean room.");
				break;
			}
			case OUT_REFUSE:
			{
				run.SetFlag("terms", TERMS_REFUSED);
				run.Note("The lab said no to {co}. Bastion's guns will be somewhere else when the stand comes.");
				break;
			}
			case OUTCOME_FAILED:
			{
				run.Note("Nobody answered {co}. Bastion took the silence as no.");
				break;
			}
			default:
			{
				run.SetFlag("terms", TERMS_AGREE);
				run.Note("The lab agreed: a locked crate, a key in {co}'s pocket, and a Bastion guard on the clean room through the stand.");
				break;
			}
		}

		return ToOffer(gm, run);
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterOffer(RZ_QuestRun run, int outcome)
	{
		if (outcome == OUT_BETRAYED)
		{
			run.SetFlag("betrayed", 1);
			run.Note("Somebody took the Red Hand's money. When the stand comes, a crew will want the clean room.");
		}
		else if (outcome == OUT_TOLD_CO)
		{
			run.SetFlag("road_post", 1);
			run.Note("{co} heard about the Red Hand's offer. Bastion will have people on the lab's road during the stand.");
		}
		else if (outcome == OUTCOME_FAILED)
		{
			run.Note("Nobody went to hear the Red Hand out.");
		}
		else
		{
			run.Note("The Red Hand's offer went to {director}. She refused it without looking up.");
		}

		return S_READY;
	}

	//------------------------------------------------------------------------------------------------
	//! The ending from the flags, after the stand: the Red Hand's deal, then Bastion's terms, then the
	//! Haven's copy, else the lab alone.
	protected int FinishAuthors(RZ_GameModeComponent gm, RZ_QuestRun run, int standResult)
	{
		run.SetFlag("stand", standResult);
		run.SetText("authors", Authors(run));
		int terms = run.Flag("terms");
		if (run.Flag("betrayed") == 1)
		{
			run.SetBranch(E_REDHAND);
			return Finish(run, E_REDHAND, "The Red Hand's batch. The boss paid for it. Whether he got it is written in the stand's last minutes." + PaperLine(run));
		}

		if (terms == TERMS_AGREE || terms == TERMS_BARGAIN)
		{
			run.SetBranch(E_BASTION);
			return Finish(run, E_BASTION, "Under Bastion's guard. {co} has his terms, and Bastion's name is on the lab's door." + PaperLine(run));
		}

		if (run.Flag("haven_copy") == 1 && run.Flag("reported") != 1)
		{
			run.SetBranch(E_OPEN);
			return Finish(run, E_OPEN, "Open doors. The method went to the Haven's clinics for nothing, and {lead}'s name is first on the paper." + PaperLine(run));
		}

		run.SetBranch(E_ALONE);
		return Finish(run, E_ALONE, "The lab alone. No terms, no guards, no buyers: the lab stood on its own." + PaperLine(run));
	}

	//------------------------------------------------------------------------------------------------
	//! "Mara, Jonas and Pavel": up to four who stood at the lab for the story, "" if nobody did.
	protected static string Authors(RZ_QuestRun run)
	{
		array<string> names = {};
		foreach (RZ_QuestPart p : run.m_aParts)
		{
			if (p.m_iStage == S_STAND && !p.m_sName.IsEmpty() && !names.Contains(p.m_sName))
				names.Insert(p.m_sName);
		}

		if (names.IsEmpty())
			return "";

		string text = names[0];
		int shown = Math.MinInt(names.Count(), 4);
		for (int i = 1; i < shown; i++)
		{
			if (i == shown - 1)
				text += " and " + names[i];
			else
				text += ", " + names[i];
		}

		if (names.Count() > shown)
			text += string.Format(" and %1 more", names.Count() - shown);

		return text;
	}

	//------------------------------------------------------------------------------------------------
	protected static string PaperLine(RZ_QuestRun run)
	{
		if (run.Text("authors").IsEmpty())
			return "";

		return " Standing at the lab when it mattered: {authors}.";
	}

	//------------------------------------------------------------------------------------------------
	override void OnEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
		RZ_Questlines q = gm.GetQuestlines();
		int stand = run.Flag("stand", STAND_WON);
		if (run.Flag("formula_sold") == 1)
			q.SetWorldFlag(FLAG_FORMULA_SOLD, 1);

		switch (ending)
		{
			case E_BASTION:
			{
				bool agreed = run.Flag("terms") == TERMS_AGREE;
				if (agreed)
					q.SetWorldFlag(FLAG_BASTION_KEY, 1);

				if (run.Flag("guard_up") == 1)
					q.SetWorldFlag(FLAG_GUARD, 1);

				ChangeStanding(gm, run, RZ_EFaction.MILITARY, 10, 0);
				ChangeStanding(gm, run, RZ_EFaction.INSTITUTE, 10, 0);
				NudgeStrength(gm, RZ_EFaction.MILITARY, 3);
				NudgeMorale(gm, RZ_EFaction.SURVIVORS, -3);
				Headline(gm, run.Fill(BastionHeadline(run, agreed, stand)));
				break;
			}
			case E_OPEN:
			{
				q.SetWorldFlag(FLAG_OPEN, 1);
				q.SetWorldFlag(FLAG_LEAD_FIRST, 1);
				ChangeStanding(gm, run, RZ_EFaction.INSTITUTE, 10, 0);
				ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 10, 0);
				OpenClinics(gm);
				string who = "{lead}'s name is first on the paper, wherever she is now.";
				if (RZ_QM_HalcyonScene.SameLeader(gm, run, "lead"))
					who = "On the lab's paper, {lead}'s name is first.";

				Headline(gm, run.Fill("The cure's method went to every Haven clinic for nothing, written out by hand. " + who));
				break;
			}
			case E_REDHAND:
			{
				// Only whoever took the money (the line's one betray approach)
				q.SetWorldFlag(FLAG_BATCH_SOLD, 1);
				ChangeStanding(gm, run, RZ_EFaction.BANDITS, 10, 0, RZ_EQuestApproach.BETRAY);
				ChangeStanding(gm, run, RZ_EFaction.INSTITUTE, 0, MAX_INFAMY, RZ_EQuestApproach.BETRAY);
				NudgeMorale(gm, RZ_EFaction.INSTITUTE, -10);
				Headline(gm, RedHandHeadline(stand));
				break;
			}
			case E_ALONE:
			{
				ChangeStanding(gm, run, RZ_EFaction.INSTITUTE, 10, 0);
				Headline(gm, AloneHeadline(stand));
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string BastionHeadline(RZ_QuestRun run, bool agreed, int stand)
	{
		string text = "Bastion signed the director's terms. {co} has a key to the lab's clean room.";
		if (!agreed)
			text = "Bastion settled for two guards at the lab gate and no key. The lab kept its own clean room.";

		if (run.Flag("guard_up") == 1)
			text += " A Bastion squad held the gate through the final stand.";

		if (stand != STAND_WON)
			text += " The stand was lost anyway, this time.";

		return text;
	}

	//------------------------------------------------------------------------------------------------
	protected static string RedHandHeadline(int stand)
	{
		if (stand == STAND_STOLEN)
			return "The Red Hand walked out of the lab's clean room with the batch during the final stand. Somebody had let them know when.";

		if (stand == STAND_WON)
			return "The Red Hand paid for a batch it never got. The lab held, and the cure went out anyway.";

		return "The Red Hand paid for a batch nobody got: the infected reached the clean room first.";
	}

	//------------------------------------------------------------------------------------------------
	protected static string AloneHeadline(int stand)
	{
		if (stand == STAND_WON)
			return "The lab stood alone at the final stand, with no terms and no guards but its own, and it held.";

		if (stand == STAND_STOLEN)
			return "The lab stood alone at the final stand, and the Red Hand walked off with the batch. It will try again.";

		return "The lab stood alone at the final stand, and lost the batch. It will try again.";
	}

	//------------------------------------------------------------------------------------------------
	//! Open doors: infection -3 in every safehouse town, once each (as Leak the Data).
	protected void OpenClinics(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses)
			return;

		array<int> towns = {};
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			if (!h || h.m_iSettlement < 0 || towns.Contains(h.m_iSettlement))
				continue;

			towns.Insert(h.m_iSettlement);
			NudgeInfection(gm, h.m_iSettlement, -3);
		}
	}

	//------------------------------------------------------------------------------------------------
	override string JournalText(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_CureResearch cure = gm.GetCure();
		int pct = RZ_Util.RoundToInt(cure.m_fProgress);
		if (cure.IsCured() && run.m_iStage < S_STAND_WAIT)
			return run.Fill("The cure went out before anyone settled whose names go on the paper. The lab stood alone.");

		if (cure.IsFinalStand() && run.m_iStage < S_STAND_WAIT)
			return run.Fill("The final stand has begun. Whatever was being settled can wait, if the lab survives it.");

		if (run.m_iStage == S_PLAN && !RZ_QM_HalcyonScene.SameLeader(gm, run, "lead"))
			return run.Fill("{lead} isn't at {lead_place} any more. Her plan went with her.");

		if (run.m_iStage == S_READY && cure.m_fProgress < CURE_READY)
			return run.Fill(string.Format("{director} will want the lab ready for the final push once research is past three quarters. It's at %1 percent.", pct));

		if (run.m_iStage == S_STAND_WAIT)
			return run.Fill(string.Format("The lab is ready. When research reaches 100 percent the final stand begins (it's at %1 percent), and whoever stands at the lab then is who the paper remembers.", pct));

		return super.JournalText(gm, run);
	}

	//------------------------------------------------------------------------------------------------
	override string Rumour(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.m_iStage <= S_OFFER)
			return "\"The cure's nearly done, and everybody wants their name on it. Bastion wants a key. The Red Hand wants the whole thing.\"";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override string OnLeaderTalk(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, int leaderId)
	{
		if (run.m_iStage >= S_STAND)
			return "";

		RZ_Leader director = RZ_QM_HalcyonScene.Top(gm, RZ_EFaction.INSTITUTE);
		if (director && director.m_iId == leaderId)
			return string.Format("%1 signs something without reading it. \"Bastion's guns, or the Haven's goodwill. One of them keeps this building standing. I know which, and I'm sorry.\"", director.m_sShort);

		RZ_Leader co = RZ_QM_HalcyonScene.Top(gm, RZ_EFaction.MILITARY);
		if (co && co.m_iId == leaderId)
			return string.Format("%1 taps the desk. \"The lab's terms are interesting. Mine are simpler: a locked crate, and a key in my pocket.\"", co.m_sShort);

		RZ_Leader boss = RZ_QM_HalcyonScene.Top(gm, RZ_EFaction.BANDITS);
		if (boss && boss.m_iId == leaderId)
			return string.Format("%1 grins. \"Everything's for sale. A cure more than most things.\"", boss.m_sShort);

		return "";
	}
}

//------------------------------------------------------------------------------------------------
//! Authors, stage 1 (Halcyon contract, Trusted, cure 50%+): the director's letter. The duty officer
//! at the lab hands it over (sealed, or read on the way); Bastion's adjutant takes it from whoever
//! carries it. Read, it can be copied onto the field lead's desk at her station, unseen.
class RZ_QM_AuthorsLetter : RZ_QM_HalcyonScene
{
	protected int m_iDutySpot = -1;
	protected int m_iCoSpot = -1;
	protected int m_iDeskSpot = -1;
	protected int m_iLetterClue = -1;

	protected static const int OBJ_DUTY = 0;
	protected static const int OBJ_CO = 1;
	protected static const int OBJ_DESK = 2;
	protected static const int SEEN_INFAMY = 5;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.TRUSTED, 15, 20);
		m_Req.m_fMinCure = RZ_Quest_Authors.CURE_START;
		m_Req.m_bNotCured = true;
		m_Req.Bonus("soldier");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
		RZ_Quest_Authors.Voice(gm, this, false);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		RZ_Leader co = Top(gm, RZ_EFaction.MILITARY);
		if (!run || !co)
			return false;

		vector lab = gm.GetLabPosition();
		m_vPos = LandPointNear(lab, 6, 12);
		m_iSettlement = gm.GetLabSettlement();
		int duty = AddClue("The duty officer", QFill(gm, "The duty officer has an envelope with {director}'s own tape across the flap. \"For {co}, by hand. She said you'd know better than to open it.\""));
		m_iDutySpot = AddSpot(m_vPos, RZ_EPuzzleProp.CIVILIAN, OBJ_DUTY, duty, "Talk to the duty officer", "Take the sealed letter|Take it, and read it on the way", "the duty officer at the lab");
		m_iLetterClue = AddClue("The director's letter", QFill(gm, "{director} to {co}. \"The method, written out, for Bastion alone. In return: Bastion's guns on the lab through the final stand, and nobody else's near it. Nobody else needs to know this letter exists.\""));

		int adjutant = AddClue("The adjutant", QFill(gm, "{co}'s adjutant holds out his hand for whatever you've brought. \"The CO reads everything the lab sends. Twice.\""));
		m_iCoSpot = AddSpot(NearSeat(co.m_vPos), RZ_EPuzzleProp.CIVILIAN, OBJ_CO, adjutant, "Talk to the adjutant", "Hand over the director's letter", "the commanding officer's adjutant");

		RZ_Leader lead = SameLeader(gm, run, "lead");
		if (lead)
		{
			vector desk = LandPointNear(lead.m_vPos, 8, 14);
			int deskClue = AddClue("A desk", QFill(gm, "{lead_short}'s desk at the field station: three notebooks, a cold cup, and a list of Haven clinics with ticks next to them."));
			m_iDeskSpot = AddSpot(desk, RZ_EPuzzleProp.OBJECT, OBJ_DESK, deskClue, "Look at the desk", "", QFill(gm, "{lead_short}'s desk at {lead_place}"), "Leave her a copy of the letter");
			m_Quiet.Watch(lead.m_vPos, 70);
		}

		m_sItemName = "the director's letter";
		m_sTitle = "Authors: the director's letter";
		m_sBrief = QFill(gm, "Take my letter to {co}, by hand. It's the lab's terms for the final stand. Don't open it. If you open it, don't tell me.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (!MayAct(gm, playerId))
			return;

		if (objIdx == OBJ_DUTY)
		{
			if (HasHolder() || option < 0 || option > 1)
				return;

			SetHolder(playerId);
			SetSpotLabels(m_iDutySpot, "");
			if (option == 1)
				ReadIt(gm, playerId);
			else
				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "The envelope is light, and the tape is the director's. {co}'s seat is " + RZ_Util.DirectionText(m_vPos, SpotPos(m_iCoSpot)) + "."));

			return;
		}

		if (objIdx != OBJ_CO || option != 0)
			return;

		if (!IsHolder(playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"You don't have it. Whoever does should be here.\"");
			return;
		}

		string text = "The adjutant takes it in to the CO. He comes back without it. \"The CO will answer the lab. In his own time.\"";
		if (HasBonus(gm, playerId))
			text = "The adjutant sees a soldier and takes you straight in. The CO reads it twice, the second time slowly. \"Tell your director she'll have an answer.\"";

		gm.NotifyPlayer(playerId, m_sTitle, text);
		Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_Authors.OUT_DELIVERED, "carried the director's letter to Bastion");
	}

	//------------------------------------------------------------------------------------------------
	//! Read it first: the player knows the terms (their own business), and can copy it.
	protected void ReadIt(RZ_GameModeComponent gm, int playerId)
	{
		SetQFlag(gm, "read", 1);
		Remember(gm, playerId, RZ_EQuestApproach.SNEAK, "read the director's letter");
		gm.NotifyPlayer(playerId, "The director's letter", GetClueText(gm, playerId, m_iLetterClue));
	}

	//------------------------------------------------------------------------------------------------
	//! The copy on the field lead's desk: only by whoever carries the letter, and only unseen.
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx != OBJ_DESK || !MayAct(gm, playerId))
			return;

		if (!IsHolder(playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You've nothing to leave her. The letter is at the lab, or with whoever took it.");
			return;
		}

		if (QFlag(gm, "read") != 1)
			ReadIt(gm, playerId);

		SetSpotOn(m_iDeskSpot, false);
		if (!m_Quiet.IsQuiet(gm, playerId))
		{
			string why = m_Quiet.Why(gm, playerId);
			SetQFlag(gm, "copy_seen", 1);
			RZ_QuestApproach.SideWith(gm, playerId, -1, RZ_EFaction.INSTITUTE, 0, SEEN_INFAMY);
			Remember(gm, playerId, RZ_EQuestApproach.SNEAK, "was seen copying the director's letter");
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 One of the station's people takes the copy off the desk and puts it in an envelope for the lab. The director will know who wrote it. (+%2 Halcyon Infamy)", why, SEEN_INFAMY));
			return;
		}

		SetQFlag(gm, "copy", 1);
		Remember(gm, playerId, RZ_EQuestApproach.SNEAK, "left the field lead a copy of the letter");
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You copy it out under her lamp and leave it in her top notebook. {lead_short} will find it in the morning."));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnHolderLost(RZ_GameModeComponent gm)
	{
		SetSpotLabels(m_iDutySpot, "Take the sealed letter|Take it, and read it on the way");
		super.OnHolderLost(gm);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Authors: the director's letter never reached Bastion.";
	}
}

//------------------------------------------------------------------------------------------------
//! Authors, stage 2 (Halcyon contract from the field lead, Trusted, cure 50%+): her cold case with
//! the method. Carry it to the medic at the clinic before it warms (20 minutes from pickup; Nurse
//! or Doctor 30), report her at the lab, or sell it to a Red Hand buyer (Sell the Formula).
class RZ_QM_AuthorsPlan : RZ_QM_HalcyonScene
{
	protected int m_iLead = -1;
	protected int m_iLeadSpot = -1;
	protected float m_fCold = -1;
	protected bool m_bWarned;

	protected static const int OBJ_LEAD = 0;
	protected static const int OBJ_CLINIC = 1;
	protected static const int OBJ_DUTY = 2;
	protected static const int OBJ_BUYER = 3;
	protected static const float COLD_SECONDS = 1200;
	protected static const float COLD_BONUS_SECONDS = 1800;
	protected static const int SOLD_FAVOR = 30;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.TRUSTED, 15, 15);
		m_Req.m_fMinCure = RZ_Quest_Authors.CURE_START;
		m_Req.m_bNotCured = true;
		m_Req.Bonus("nurse");
		m_Req.Bonus("doctor");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
		RZ_Quest_Authors.Voice(gm, this, true);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		RZ_Leader l = SameLeader(gm, run, "lead");
		RZ_Safehouse clinic;
		if (run)
			clinic = RZ_Questline.GetHouse(gm, run.Flag("clinic", -1));

		// Not while another mission has her (its own stand-in is out there)
		if (!l || l.m_iHeldBy >= 0 || !clinic || !gm.GetLeaders())
			return false;

		m_iLead = l.m_iId;
		gm.GetLeaders().Hold(m_iLead, m_iId);
		m_vPos = NearSeat(l.m_vPos);
		m_iSettlement = gm.FindNearestSettlement(l.m_vPos);
		int talk = AddClue(l.m_sName, QFill(gm, "{lead_short} has a cold case on the desk, sealed with her own initials. \"The method, every step. If the Haven's clinics have it first, nobody can lock it in a crate. It won't stay cold long, and the lab will miss it by tonight.\""));
		m_iLeadSpot = AddSpot(m_vPos, RZ_EPuzzleProp.CIVILIAN, OBJ_LEAD, talk, "Talk to " + l.m_sShort, "Take the cold case", string.Format("%1 at the field station in %2", l.m_sShort, l.m_sPlace));

		int medic = AddClue("The clinic's medic", "The medic at the safehouse clinic has cleared a shelf in the cold box and is pretending she hasn't.");
		AddSpot(LandPointNear(clinic.m_vPos, 4, 8), RZ_EPuzzleProp.CIVILIAN, OBJ_CLINIC, medic, "Talk to the medic", "Hand over the method", "the clinic at the safehouse in " + clinic.m_sName);

		int duty = AddClue("The duty officer", "The duty officer at the lab writes down everything you say, and the time.");
		AddSpot(LandPointNear(gm.GetLabPosition(), 6, 12), RZ_EPuzzleProp.CIVILIAN, OBJ_DUTY, duty, "Talk to the duty officer", QFill(gm, "Report {lead_short} to the director"), "the duty officer at the lab");

		vector meet;
		string meetName;
		if (RZ_LeaderJob.MeetingPoint(gm, RZ_EFaction.BANDITS, l.m_vPos, meet, meetName))
		{
			int buyer = AddClue("A Red Hand buyer", "A Red Hand buyer who can't read the label and doesn't pretend to. \"Someone will.\"");
			AddSpot(meet, RZ_EPuzzleProp.CIVILIAN, OBJ_BUYER, buyer, "Talk to the buyer", string.Format("Sell it to the Red Hand (%1 Red Hand Favor)", SOLD_FAVOR), meetName);
		}

		m_sItemName = "the cold case";
		m_sTitle = "Authors: the field lead's plan";
		m_sBrief = QFill(gm, "I want the method at a Haven clinic before Bastion locks it in a crate. It's packed cold for the clinic at the safehouse in {clinic}. Carry it before it's missed. Or don't, and tell the director. I'll know either way.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnScene(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!HasHolder() || m_fCold < 0)
			return;

		m_fCold -= dt;
		if (!m_bWarned && m_fCold <= 180)
		{
			m_bWarned = true;
			gm.NotifyPlayer(m_iHolder, m_sTitle, "The case is sweating. Three minutes, maybe.");
		}

		if (m_fCold > 0)
			return;

		NotifyInterested(gm, m_sTitle, "The case is warm. Whatever was in it is no use now, and the lab has noticed it's gone.");
		Resolve(gm, m_iHolder, RZ_EQuestApproach.NONE, RZ_Quest_Authors.OUT_MISSED, "let the method go warm", false);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (option != 0 || !MayAct(gm, playerId))
			return;

		RZ_QuestRun run = Run(gm);
		switch (objIdx)
		{
			case OBJ_LEAD:
			{
				if (HasHolder())
					return;

				SetHolder(playerId);
				SetSpotLabels(m_iLeadSpot, "");
				m_fCold = COLD_SECONDS;
				string text = "The clock starts when the seal breaks: twenty minutes, she says, before it's no use to anyone.";
				if (HasBonus(gm, playerId))
				{
					m_fCold = COLD_BONUS_SECONDS;
					text = "You pack it the way the clinics do, ice and newspaper. Thirty minutes.";
				}

				gm.NotifyPlayer(playerId, m_sTitle, text);
				return;
			}
			case OBJ_CLINIC:
			{
				if (!IsHolder(playerId))
				{
					gm.NotifyPlayer(playerId, m_sTitle, "\"I was told someone would bring it. It wasn't you.\"");
					return;
				}

				gm.NotifyPlayer(playerId, m_sTitle, "The medic reads the first page in the doorway and the rest at the table. By morning every Haven clinic with a radio has the method.");
				if (run)
					run.Reward(gm, playerId, RZ_EFaction.SURVIVORS, 5, 0, "The Haven's clinics have the method, for nothing.");

				Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_Authors.OUT_HAVEN_COPY, "carried the method to a Haven clinic");
				return;
			}
			case OBJ_DUTY:
			{
				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "By evening {director} has the case back in the lab's cold room, and {lead_short} has been told what the director thinks of her plan."));
				if (gm.GetLeaders() && m_iLead >= 0)
					gm.GetLeaders().Remember(m_iLead, playerId, RZ_Leaders.MEM_CROSSED);

				Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_Authors.OUT_REPORTED, "reported the field lead to the director");
				return;
			}
			case OBJ_BUYER:
			{
				if (!IsHolder(playerId))
				{
					gm.NotifyPlayer(playerId, m_sTitle, "\"Bring me the case, then we'll talk.\"");
					return;
				}

				RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.BANDITS, RZ_EFaction.INSTITUTE, 10, 15);
				if (run)
					run.Reward(gm, playerId, RZ_EFaction.BANDITS, 0, SOLD_FAVOR, "He pays in fuel coupons and a promise. The lab will hear who sold it. (+15 Halcyon Infamy)");

				Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_Authors.OUT_SOLD, "sold the method to the Red Hand", false);
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnHolderLost(RZ_GameModeComponent gm)
	{
		m_fCold = -1;
		m_bWarned = false;
		SetSpotLabels(m_iLeadSpot, "Take the cold case");
		super.OnHolderLost(gm);
	}

	//------------------------------------------------------------------------------------------------
	protected void ReleaseLead(RZ_GameModeComponent gm)
	{
		if (m_iLead >= 0 && gm.GetLeaders())
			gm.GetLeaders().Release(m_iLead);

		m_iLead = -1;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		ReleaseLead(gm);
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailed(RZ_GameModeComponent gm)
	{
		ReleaseLead(gm);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		ReleaseLead(gm);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Authors: the field lead's cold case went nowhere.";
	}
}

//------------------------------------------------------------------------------------------------
//! Authors, stage 3 (Halcyon contract, Trusted, cure 50%+): the CO's answer, at Bastion's adjutant.
//! Agree (talk), bargain (Trusted with Halcyon and Bastion) or refuse (+5 Bastion Infamy).
class RZ_QM_AuthorsAnswer : RZ_QM_HalcyonScene
{
	protected static const int OBJ_ADJUTANT = 0;
	protected static const int REFUSE_INFAMY = 5;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.TRUSTED, 10, 15);
		m_Req.m_fMinCure = RZ_Quest_Authors.CURE_START;
		m_Req.m_bNotCured = true;
		m_Req.Bonus("soldier");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
		RZ_Quest_Authors.Voice(gm, this, false);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Leader co = Top(gm, RZ_EFaction.MILITARY);
		if (!Run(gm) || !co)
			return false;

		m_vPos = NearSeat(co.m_vPos);
		m_iSettlement = gm.FindNearestSettlement(co.m_vPos);
		string demand = "\"A locked crate for the batch. A key, in my pocket. A Bastion guard on the clean room through the stand.\" The adjutant reads it out flat, the way he reads everything. \"The CO considers that generous.\"";
		if (QFlag(gm, "early_terms") == 1)
			demand = "\"The CO has read the dead woman's notebook. He knows what the lab sat on for three weeks. His terms: a locked crate, a key in his pocket, and a Bastion guard on the clean room.\"";

		int talk = AddClue("The adjutant", demand);
		AddSpot(m_vPos, RZ_EPuzzleProp.CIVILIAN, OBJ_ADJUTANT, talk, "Talk to the adjutant", "Agree, for the director|Bargain: two guards at the gate, no key|Refuse, for the lab", "the commanding officer's adjutant");
		m_sTitle = "Authors: the CO's answer";
		m_sBrief = QFill(gm, "{co} has answered my letter, and he wants more than I offered. Go and hear it, and answer him for the lab. You know what we can live with.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_ADJUTANT || !MayAct(gm, playerId))
			return;

		if (option == 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "The adjutant writes \"agreed\" and the time. \"The CO will send the crate. And the guard.\""));
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_Authors.OUT_AGREE, "agreed to Bastion's terms for the lab");
			return;
		}

		if (option == 1)
		{
			if (Rank(gm, playerId, RZ_EFaction.INSTITUTE) < 2 || Rank(gm, playerId, RZ_EFaction.MILITARY) < 2)
			{
				Remember(gm, playerId, RZ_EQuestApproach.PERSUADE, "tried to bargain and wasn't heard");
				gm.NotifyPlayer(playerId, m_sTitle, "\"You can carry messages. You can't change them.\" (Bargaining takes someone both the lab and Bastion trust.)");
				return;
			}

			gm.NotifyPlayer(playerId, m_sTitle, "The adjutant takes it in and comes back quickly. \"Two guards at the gate. No key. The CO says you drive a hard bargain for a courier.\"");
			Resolve(gm, playerId, RZ_EQuestApproach.PERSUADE, RZ_Quest_Authors.OUT_BARGAIN, "bargained Bastion down to two guards");
			return;
		}

		if (option == 2)
		{
			RZ_QuestApproach.SideWith(gm, playerId, -1, RZ_EFaction.MILITARY, 0, REFUSE_INFAMY);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("\"Refused.\" The adjutant writes it down, and your name next to it. \"The CO will remember who said it.\" (+%1 Bastion Infamy)", REFUSE_INFAMY));
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_Authors.OUT_REFUSE, "refused Bastion's terms for the lab");
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Authors: nobody answered Bastion for the lab.";
	}
}

//------------------------------------------------------------------------------------------------
//! Authors, stage 4 (Halcyon contract, Trusted, cure 50%+): the boss's man at a meeting place near
//! the Red Hand's seat. Turn him down (the director refuses), take the money (betray), or, once
//! someone has heard him out, tell Bastion's adjutant (side with Bastion).
class RZ_QM_AuthorsOffer : RZ_QM_HalcyonScene
{
	protected int m_iOfferClue = -1;
	protected int m_iCoSpot = -1;

	protected static const int OBJ_MAN = 0;
	protected static const int OBJ_ADJUTANT = 1;
	protected static const int MONEY_FAVOR = 40;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.TRUSTED, 10, 10);
		m_Req.m_fMinCure = RZ_Quest_Authors.CURE_START;
		m_Req.m_bNotCured = true;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
		RZ_Quest_Authors.Voice(gm, this, false);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Leader boss = Top(gm, RZ_EFaction.BANDITS);
		if (!Run(gm) || !boss)
			return false;

		vector meet;
		string meetName;
		if (!RZ_LeaderJob.MeetingPoint(gm, RZ_EFaction.BANDITS, gm.GetLabPosition(), meet, meetName))
		{
			meet = NearSeat(boss.m_vPos);
			meetName = "the boss's man at the Red Hand's camp";
		}

		m_vPos = meet;
		m_iSettlement = gm.FindNearestSettlement(meet);
		m_iOfferClue = AddClue("The boss's man", QFill(gm, "\"{boss} wants the batch. All of it, the moment it's made. He'll pay like nobody's ever paid for anything in here, and his crews will leave the lab alone while the dead come for it. Or they won't.\" He smiles. \"Up to you.\""));
		AddSpot(meet, RZ_EPuzzleProp.CIVILIAN, OBJ_MAN, m_iOfferClue, "Talk to the boss's man", string.Format("Turn him down and tell the director|Take the money (%1 Red Hand Favor)", MONEY_FAVOR), meetName);

		RZ_Leader co = Top(gm, RZ_EFaction.MILITARY);
		if (co)
		{
			int adjutant = AddClue("The adjutant", "The adjutant listens with his pen ready. \"The Red Hand, near the lab, during the stand. The CO will want to know that.\"");
			m_iCoSpot = AddSpot(NearSeat(co.m_vPos), RZ_EPuzzleProp.CIVILIAN, OBJ_ADJUTANT, adjutant, "Talk to the adjutant", "Tell the CO about the Red Hand's offer", "the commanding officer's adjutant");
			SetSpotOn(m_iCoSpot, false);
		}

		m_sTitle = "Authors: the Red Hand's offer";
		m_sBrief = QFill(gm, "The Red Hand wants to buy my batch. They've sent someone to make the offer properly. Hear him out, and bring me his terms. I will say no. I'd like to know what I'm saying no to.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Once someone has heard the offer, Bastion can be told about it.
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		if (idx == m_iOfferClue)
			SetSpotOn(m_iCoSpot, true);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (!MayAct(gm, playerId))
			return;

		if (objIdx == OBJ_ADJUTANT && option == 0)
		{
			RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.MILITARY, -1, 6, 0);
			gm.NotifyPlayer(playerId, m_sTitle, "\"Noted.\" The adjutant is already reaching for the radio. \"Bastion will have people on the lab's road when the stand comes.\"");
			Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_Authors.OUT_TOLD_CO, "told Bastion about the Red Hand's offer");
			return;
		}

		if (objIdx != OBJ_MAN)
			return;

		if (option == 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "He shrugs as if he expected it. \"The offer stands.\" At the lab, {director} hears it out and says no without looking up."));
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_Authors.OUT_DECLINED, "turned the Red Hand down for the lab");
			return;
		}

		if (option != 1)
			return;

		RZ_QuestRun run = Run(gm);
		if (run)
			run.Reward(gm, playerId, RZ_EFaction.BANDITS, 0, MONEY_FAVOR, "\"Pleasure doing business.\" When the stand comes, a crew will want the clean room, and you'll know when.");

		Resolve(gm, playerId, RZ_EQuestApproach.BETRAY, RZ_Quest_Authors.OUT_BETRAYED, "took the Red Hand's money for the batch", false);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Authors: nobody heard the Red Hand's offer.";
	}
}

//------------------------------------------------------------------------------------------------
//! Authors, stage 5: Power the Lab, picked by the director (Trusted; the contract's own rules).
class RZ_QM_AuthorsPower : RZ_Mission_PowerTheLab
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		super.Configure(gm);
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.TRUSTED, 15, 20);
		RZ_Quest_Authors.Voice(gm, this, false);
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		if (!super.SetupPuzzle(gm))
			return false;

		m_sTitle = "Authors: ready the lab";
		m_sBrief = string.Format("If Bastion's guards are going to stand at my door, the synthesis had better not stall for want of power. The generator needs a regulator from %1, and fuel. Clean power for the final push.", m_sPartPlace);
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! Authors, stage 5: Decontamination, picked by the director (the Haven's people will come).
class RZ_QM_AuthorsDecon : RZ_Mission_Decontamination
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		super.Configure(gm);
		RZ_Quest_Authors.Voice(gm, this, false);
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		if (!super.Setup(gm))
			return false;

		m_sTitle = "Authors: ready the lab";
		m_sBrief = "The Haven's clinics have the method now, and their people will come up the hill for the stand. I won't have them walk through our dead. Burn the four piles around the lab.";
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! Authors, stage 5: Lab Defence, picked by the director (the lab holds on its own).
class RZ_QM_AuthorsDefence : RZ_Mission_LabDefence
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		super.Configure(gm);
		RZ_Quest_Authors.Voice(gm, this, false);
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		if (!super.Setup(gm))
			return false;

		m_sTitle = "Authors: ready the lab";
		m_sBrief = "When the stand comes, nobody else's guns will be at my door. We hold the lab ourselves, so we had better know how. Hold it for four minutes now, while it's only practice.";
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! Authors, stage 6 (Halcyon contract, Trusted): the final stand itself, from when it starts until
//! it ends. Whoever is at the lab (150 m) takes part. Near players: a Bastion group at the gate
//! (Bastion's terms; Territory > Bastion guard), one on the road toward the Red Hand (if Bastion was
//! told of the offer) and one more Halcyon guard (B3's pilot flew for the samples; Territory >
//! Halcyon guard). Ends when the stand does: won (after the winner is chosen), lost, or stolen.
class RZ_QM_AuthorsStand : RZ_QM_HalcyonScene
{
	protected bool m_bSides;
	protected int m_iLostAtStart;
	protected int m_iStolenAtStart;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.TRUSTED, 5, 10);
		m_fTimeLeft = 7200;
		m_fParticipationRadius = RZ_CureResearch.LAB_DEFENCE_RADIUS;
		RZ_Quest_Authors.Voice(gm, this, false);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run || !gm.GetCure().IsFinalStand())
			return false;

		m_vPos = gm.GetLabPosition();
		m_iSettlement = gm.GetLabSettlement();
		m_iLostAtStart = gm.GetCure().GetBatchesLost();
		m_iStolenAtStart = gm.GetCure().GetBatchesStolen();
		m_sTitle = "Authors: the final stand";
		m_sBrief = "The cure is being made. Stand at the lab until it's done. Whoever is standing here when it's made is who the paper remembers.";
		int terms = run.Flag("terms");
		if (terms == RZ_Quest_Authors.TERMS_AGREE || terms == RZ_Quest_Authors.TERMS_BARGAIN)
			m_sBrief += " Bastion's people are at the gate, as agreed.";

		if (PilotFlew(gm))
			m_sBrief += " Halcyon's people remember who flew for the samples. One more of them will stand at the gate.";

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! B3 (The Last Pilot): the pilot's flight was for Halcyon's samples.
	static bool PilotFlew(RZ_GameModeComponent gm)
	{
		return gm.GetQuestlines() && gm.GetQuestlines().WorldFlag(RZ_Quest_Authors.FLAG_PILOT, -1) == RZ_EFaction.INSTITUTE;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnScene(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bSides && RZ_Util.NearestDistance(m_vPos, players) <= 400)
		{
			m_bSides = true;
			PlaceSides(gm);
		}

		RZ_CureResearch cure = gm.GetCure();
		if (cure.IsFinalStand())
			return;

		// The stand is over: the cure is out (the winner was chosen first), or the batch is gone
		int near = RZ_EscortHelp.PlayerBeside(ids, players, m_vPos, RZ_CureResearch.LAB_DEFENCE_RADIUS);
		if (cure.IsCured())
		{
			Resolve(gm, near, RZ_EQuestApproach.NONE, RZ_Quest_Authors.OUT_WON, "stood at the lab", true);
			return;
		}

		if (cure.GetBatchesStolen() > m_iStolenAtStart)
		{
			Resolve(gm, near, RZ_EQuestApproach.NONE, RZ_Quest_Authors.OUT_STOLEN, "stood at the lab", false);
			return;
		}

		Resolve(gm, near, RZ_EQuestApproach.NONE, RZ_Quest_Authors.OUT_LOST, "stood at the lab", false);
	}

	//------------------------------------------------------------------------------------------------
	//! The groups the story put at the lab, only with players near, gone with the stage.
	protected void PlaceSides(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		RZ_Territory territory = gm.GetTerritory();
		if (!run || !territory)
			return;

		vector lab = m_vPos;
		int terms = run.Flag("terms");
		if ((terms == RZ_Quest_Authors.TERMS_AGREE || terms == RZ_Quest_Authors.TERMS_BARGAIN) && Place(territory.GuardPrefab(RZ_EFaction.MILITARY), LandPointNear(lab, 25, 40)))
		{
			SetQFlag(gm, "guard_up", 1);
			RZ_ContractHelp.NotifyNear(gm, lab, 400, m_sTitle, "A Bastion squad is at the lab gate, as agreed. They're facing outwards.");
		}

		RZ_Leader boss = Top(gm, RZ_EFaction.BANDITS);
		if (run.Flag("road_post") == 1 && boss)
		{
			vector road = LandPointNear(RZ_Compass.PointAt(lab, RZ_Compass.Bearing(lab, boss.m_vPos), 250), 0, 20);
			if (Place(territory.GuardPrefab(RZ_EFaction.MILITARY), road))
			{
				SetQFlag(gm, "post_up", 1);
				RZ_ContractHelp.NotifyNear(gm, lab, 400, m_sTitle, "Bastion has people on the road from the Red Hand's side, as the CO promised.");
			}
		}

		if (PilotFlew(gm) && Place(territory.GuardPrefab(RZ_EFaction.INSTITUTE), LandPointNear(lab, 20, 30)))
			SetQFlag(gm, "pilot_guard", 1);
	}

	//------------------------------------------------------------------------------------------------
	protected bool Place(ResourceName prefab, vector pos)
	{
		if (prefab.IsEmpty())
			return false;

		IEntity e = RZ_Util.SpawnPrefab(prefab, RZ_Util.GroundPos(pos), Math.RandomFloat(0, 360));
		if (!e)
			return false;

		TrackProp(e);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Authors: the final stand ended without a cure.";
	}
}

//------------------------------------------------------------------------------------------------
//! Authors: the boss was paid, so his crews are offered Take the Lab during the stand (Known with
//! the Red Hand, not Champion). Its usual result: 90 seconds in the clean room and the batch is gone.
class RZ_QM_AuthorsTakeTheLab : RZ_Mission_TakeTheLab
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		super.Configure(gm);
		m_Req.m_eMinTier = RZ_ETier.KNOWN;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		if (!super.Setup(gm))
			return false;

		m_sBrief = "The boss paid for that batch, and somebody on the inside took the money. While Halcyon is busy with the dead, walk into the clean room and hold it for ninety seconds. We'll carry it out ourselves.";
		return true;
	}
}
