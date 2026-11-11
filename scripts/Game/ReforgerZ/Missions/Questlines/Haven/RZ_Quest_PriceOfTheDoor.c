//! The Price of the Door (the Haven, or the Red Hand): questline H2 (docs/QUESTLINES.md, "H2. The
//! Price of the Door"). A hungry safehouse pays the Red Hand to be left alone. This month the crew
//! boss doesn't want tins: he wants a stranger, a man the house put out for stealing from the
//! stores, and where he sleeps now. Someone has to carry the envelope. Each side gives its own
//! version (run flag "side": 0 the house's council head asks, 1 the crew boss does).
//!
//! 1. Carry the envelope (mission): A Quiet Arrangement's walk, with its lookouts (★ Ex-Con: seen
//!    only at 25 m). Deliver it sealed, open it first (the grid and the name), or leave it.
//! 2. The man at the grid (mission, only if it was paid): warn him (he takes to the roads as a
//!    wanderer), bring him back to plead (escort), walk him to the crew (betray; seen by a lookout
//!    or a Haven wanderer: +15 Haven Infamy), or leave it (the crew finds him).
//! 3. The crew comes to collect (mission, only if it wasn't): hold the gate against a Red Hand raid
//!    (with the Red Hand group prefab), pay the boss off (40 Haven or 25 Red Hand Favor), take the
//!    debt on yourself (Trusted with the Red Hand or ★ Ex-Con), or fetch a Bastion patrol with
//!    the house's bread. Nobody does: the crew takes two people.
//! 4. A day of talk (wait): two hours of play. Then the vote (mission): speak for the council
//!    head (Known), against them (Trusted or ★ Elder), and, if he came home, for the exile.
//! 5. The house stands, or pays (mission): the barricades (Fortify the Safehouse; none needed
//!    with well.stood), next month's envelope of tins, or the first instalment of your debt in
//!    the crew's pit (House Rules) or 20 Red Hand Favor.
//!
//! Endings: Stands alone, Under Bastion, Paid in tins, The debt is yours.
class RZ_Quest_PriceOfTheDoor : RZ_Questline
{
	static const int S_ENVELOPE = 0;
	static const int S_GRID = 1;
	static const int S_COLLECT = 2;
	static const int S_TALK = 3;
	static const int S_VOTE = 4;
	static const int S_STAND = 5;

	static const int OUT_PAID = 10;			// 1: the envelope reached the crew
	static const int OUT_REFUSED = 11;		// 1: left where it was
	static const int OUT_WARNED = 12;		// 2: he was warned and moved on
	static const int OUT_HOME = 13;			// 2: brought back to plead
	static const int OUT_SOLD = 14;			// 2: walked to the crew
	static const int OUT_HELD = 15;			// 3: the gate held
	static const int OUT_BOUGHT = 16;		// 3: the boss was paid off
	static const int OUT_DEBT = 17;			// 3: the debt is someone's own now
	static const int OUT_BASTION = 18;		// 3: a Bastion patrol came
	static const int OUT_FOR = 19;			// 4: the council head keeps the chair
	static const int OUT_AGAINST = 20;		// 4: the house voted them out
	static const int OUT_TINS = 21;			// 5: next month's tins delivered
	static const int OUT_INSTALMENT = 22;	// 5: the first instalment paid

	// How the house came through the month (flag "path")
	static const int PATH_PAID = 0;			// the envelope went (or the crew took what it wanted)
	static const int PATH_HELD = 1;
	static const int PATH_BASTION = 2;
	static const int PATH_DEBT = 3;
	static const int PATH_BOUGHT = 4;

	static const int E_STANDS = 0;			// Stands alone
	static const int E_BASTION = 1;			// Under Bastion
	static const int E_TINS = 2;			// Paid in tins
	static const int E_DEBT = 3;			// The debt is yours

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		Info("door", "The Price of the Door", RZ_EFaction.SURVIVORS);

		RZ_QuestStage envelope = MissionStage(S_ENVELOPE, "Carry the envelope", "The safehouse in {house} pays the Red Hand to be left alone. This month {boss} doesn't want tins. The price is in an envelope under a loose board by the door, and somebody has to carry it to the crew's drop without the house's own lookouts seeing.");
		envelope.GiveUp(4, "{why}");

		RZ_QuestStage grid = MissionStage(S_GRID, "The man at the grid", "The envelope held a name and a grid: {exile}, put out of {house} for stealing from the stores, sleeping in a barn outside {barn}. The crew will come for him. Warn him, bring him home to plead, walk him to the crew yourself, or leave it.");
		grid.GiveUp(4, "{why}");

		RZ_QuestStage collect = MissionStage(S_COLLECT, "The crew comes to collect", "The price wasn't paid, so {boss} is coming to {house} in person. Hold the gate, pay him off, take the debt on yourself, or bring Bastion in.");
		collect.GiveUp(4, "{why}");

		RZ_QuestStage talk = WaitStage(S_TALK, "A day of talk", "Everyone in {house} knows what the month cost by now. The house will vote on its council head.");
		talk.Wait(120);
		talk.GiveUp(3, "{why}");

		RZ_QuestStage vote = MissionStage(S_VOTE, "The vote", "{house} votes tonight on whether {head} keeps the chair. Speak for them, or against them. {exile} may need someone to speak for him too.");
		vote.GiveUp(4, "{why}");

		RZ_QuestStage stand = MissionStage(S_STAND, "The house stands, or pays", "{standtext}");
		stand.GiveUp(4, "{why}");
		stand.Visits(2);
	}

	//------------------------------------------------------------------------------------------------
	//! Needs a safehouse under pressure (hungry, or it has lost people lately; not the chair's, not at
	//! the Preacher's prayers) with a Red Hand crew within 6 km, the note, the object and the
	//! civilian. The Haven's version needs its council head to be Independent or Rogue; the Red
	//! Hand's comes from the crew boss.
	override bool CanStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (!RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN) || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.OBJECT))
			return false;

		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses || !gm.GetLeaders())
			return false;

		int chairHouse = RZ_HavenQuest.ChairHouse(gm);
		array<int> havenSide = {};
		array<int> redHandSide = {};
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, i);
			if (!h || i == chairHouse || h.m_iPopulation < 3 || !RZ_HavenQuest.IsPressed(h) || RZ_HavenQuest.IsPraying(gm, i))
				continue;

			RZ_Leader head = RZ_HavenQuest.CouncilHead(gm, i);
			if (!head || head.IsStronghold() || !RZ_HavenQuest.NearestSeat(gm, RZ_EFaction.BANDITS, h.m_vPos, 6000))
				continue;

			redHandSide.Insert(i);
			if (head.m_eLoyalty != RZ_ELoyalty.LOYAL)
				havenSide.Insert(i);
		}

		if (redHandSide.IsEmpty())
			return false;

		int side = 1;
		int house = redHandSide.GetRandomElement();
		if (!havenSide.IsEmpty() && Math.RandomFloat01() < 0.5)
		{
			side = 0;
			house = havenSide.GetRandomElement();
		}

		RZ_Safehouse home = houses.Get(house);
		int barnTown = RZ_Mission.PickSettlementAway(gm, home.m_vPos, 800, 3000, 0, 80, home.m_iSettlement);
		RZ_Settlement barn = gm.GetSettlement(barnTown);
		if (!barn)
			return false;

		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		run.SetFlag("side", side);
		run.SetFlag("house", house);
		run.SetText("house", home.m_sName);
		RZ_HavenQuest.KeepLeader(run, "head", RZ_HavenQuest.CouncilHead(gm, house));
		RZ_HavenQuest.KeepLeader(run, "boss", RZ_HavenQuest.NearestSeat(gm, RZ_EFaction.BANDITS, home.m_vPos, 6000));
		run.SetFlag("barn", barnTown);
		run.SetText("barn", barn.m_sName);
		run.SetText("exile", RZ_HavenQuest.ManName(rng));
		run.SetFlag("path", PATH_PAID);
		run.SetText("standtext", "Next month comes round.");
		run.SetText("why", run.Fill("Nobody carried anything for {house} in the end. The Red Hand will ask again next month."));
		run.SetPlace(home.m_vPos, home.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		Headline(gm, run.Fill("The safehouse in {house} has gone to half portions. The Red Hand hasn't been by, which is strange, given."), false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStageBegin(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		CheckBoss(gm, run);
		RZ_Settlement barn = gm.GetSettlement(run.Flag("barn", -1));
		RZ_Safehouse home = RZ_HavenQuest.LiveHouse(gm, run.Flag("house", -1));
		if (stage == S_GRID && barn)
			run.SetPlace(barn.m_vCenter, barn.m_sName);
		else if (home)
			run.SetPlace(home.m_vPos, home.m_sName);

		if (stage == S_ENVELOPE)
		{
			if (run.Flag("side") == 1)
				run.Note("{boss} wants the envelope from {house} brought to the crew's drop. \"Don't open it. It's not tins this month.\"");
			else
				run.Note("{head} would rather not know what's in the envelope. \"Carry it, and don't let our lookouts see you. The house mustn't know what it costs.\"");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The crew boss who set the price left his seat: whoever has it now wants the same. If the crew
	//! is gone from round here altogether, there's nobody to pay.
	protected void CheckBoss(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (RZ_HavenQuest.KeptLeader(gm, run, "boss"))
			return;

		RZ_Leader next = RZ_HavenQuest.SeatHolder(gm, run.Flag("boss", -1));
		RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, run.Flag("house", -1));
		if (!next && h)
			next = RZ_HavenQuest.NearestSeat(gm, RZ_EFaction.BANDITS, h.m_vPos, 6000);

		if (!next)
			return;

		run.Note(string.Format("{boss} is gone. %1, %2, has taken over the arrangement with {house}, and wants the same price.", next.m_sName, next.m_sTitle));
		RZ_HavenQuest.KeepLeader(run, "boss", next);
	}

	//------------------------------------------------------------------------------------------------
	//! The house must still have people, and some crew must still be in reach.
	override bool IsReady(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, run.Flag("house", -1));
		if (!h)
		{
			run.SetText("why", run.Fill("The safehouse in {house} stands empty now. There's nobody left there to pay anyone."));
			return false;
		}

		if (stage == S_VOTE || stage == S_TALK)
			return true;

		if (!RZ_HavenQuest.KeptLeader(gm, run, "boss"))
		{
			CheckBoss(gm, run);
			if (!RZ_HavenQuest.KeptLeader(gm, run, "boss"))
			{
				run.SetText("why", run.Fill("The crew that wanted paying is gone from {boss_place}. Nobody has come to {house} for the price since."));
				return false;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override RZ_Mission MakeMission(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		switch (stage)
		{
			case S_ENVELOPE: return new RZ_QM_DoorEnvelope();
			case S_GRID: return new RZ_QM_DoorExile();
			case S_COLLECT: return new RZ_QM_DoorCollect();
			case S_VOTE: return new RZ_QM_DoorVote();
			case S_STAND: return StandMission(run);
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Mission StandMission(RZ_QuestRun run)
	{
		int path = run.Flag("path");
		if (path == PATH_HELD || path == PATH_BASTION)
			return new RZ_QM_DoorFortify();

		if (path == PATH_DEBT)
			return new RZ_QM_DoorInstalment();

		RZ_QM_DoorEnvelope tins = new RZ_QM_DoorEnvelope();
		tins.m_bTins = true;
		return tins;
	}

	//------------------------------------------------------------------------------------------------
	override int Next(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		switch (stage)
		{
			case S_ENVELOPE:
			{
				if (outcome == OUT_PAID || outcome == OUTCOME_DONE)
				{
					gm.GetQuestlines().SetWorldFlag("door.paid", 1);
					run.Note("The envelope reached the crew. Inside it was a man: {exile}, put out of {house} for stealing from the stores, and the barn outside {barn} where he sleeps now.");
					return S_GRID;
				}

				if (outcome == OUT_REFUSED)
					run.Note("Nobody would carry it. The envelope is still under the board, and the price is still owed.");
				else
					run.Note("The envelope never reached the crew. The price is still owed.");

				return S_COLLECT;
			}
			case S_GRID: return AfterGrid(gm, run, outcome);
			case S_COLLECT: return AfterCollect(gm, run, outcome);
			case S_TALK:
			{
				if (RZ_HavenQuest.KeptLeader(gm, run, "head"))
					return S_VOTE;

				run.Note("{head} isn't council head in {house} any more. There's nothing left to vote on.");
				return ToStand(gm, run);
			}
			case S_VOTE:
			{
				if (outcome == OUT_AGAINST)
				{
					run.SetFlag("against", 1);
					run.Note("The house voted. {head} is out of the chair, and the cook will hold it until they choose properly.");
				}
				else
				{
					run.Note("The house voted, and {head} keeps the chair. Not everyone clapped.");
				}

				return ToStand(gm, run);
			}
			case S_STAND: return AfterStand(run, outcome);
		}

		return Fizzle(run, "");
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterGrid(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		RZ_Questlines q = gm.GetQuestlines();
		if (outcome == OUT_WARNED)
		{
			run.SetFlag("exile_warned", 1);
			q.SetWorldFlag("door.exile_warned", 1);
			run.Note("{exile} was warned. He's on the roads now, somewhere the crew isn't.");
		}
		else if (outcome == OUT_HOME)
		{
			run.SetFlag("exile_home", 1);
			q.SetWorldFlag("door.exile_home", 1);
			run.Note("{exile} came back to {house} to plead his case. Someone will have to speak for him.");
		}
		else if (outcome == OUT_SOLD)
		{
			run.SetFlag("exile_sold", 1);
			q.SetWorldFlag("door.exile_sold", 1);
			run.Note("{exile} was walked to the crew. The house's price is paid in full.");
		}
		else
		{
			run.SetFlag("exile_taken", 1);
			run.Note("Nobody went to the barn outside {barn}. The crew did, a day later.");
		}

		return S_TALK;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterCollect(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		if (outcome == OUT_HELD || outcome == OUTCOME_DONE)
		{
			run.SetFlag("path", PATH_HELD);
			run.Note("The gate held. {boss}'s crew went back down the road with nothing.");
		}
		else if (outcome == OUT_BOUGHT)
		{
			run.SetFlag("path", PATH_BOUGHT);
			run.Note("{boss} was paid off this time. Next month he'll want paying again.");
		}
		else if (outcome == OUT_DEBT)
		{
			run.SetFlag("path", PATH_DEBT);
			gm.GetQuestlines().SetWorldFlag("door.debt_yours", 1);
			run.Note("A debt is a debt, {boss} says, but it can be someone else's. It's yours now, not the house's.");
		}
		else if (outcome == OUT_BASTION)
		{
			run.SetFlag("path", PATH_BASTION);
			run.Note("A Bastion patrol walked up the road to {house} with the bread in its truck. {boss}'s crew didn't stay to argue.");
		}
		else
		{
			// Nobody stopped them: they took what they came for
			run.SetFlag("path", PATH_PAID);
			if (RZ_HavenQuest.TakePeople(gm, run.Flag("house", -1), 2) > 0)
				run.Note("Nobody stood at the gate. The crew took people from {house} as the price, and left.");
			else
				run.Note("Nobody stood at the gate. The crew took what food {house} had left, as the price.");
		}

		return S_TALK;
	}

	//------------------------------------------------------------------------------------------------
	//! The last stage for the way the month went, or straight to the ending when the house already
	//! stands (well.stood, from R3: no barricades needed).
	protected int ToStand(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		int path = run.Flag("path");
		if (path == PATH_HELD && gm.GetQuestlines().WorldFlag("well.stood") == 1)
			return Finish(run, E_STANDS, "Stands alone. {house} stood up to the crew, and the houses round it stood with it.");

		if (path == PATH_HELD || path == PATH_BASTION)
			run.SetText("standtext", run.Fill("{house} held this month. Put the barricades up before the next."));
		else if (path == PATH_DEBT)
			run.SetText("standtext", run.Fill("The debt is yours now. The first instalment is due: in {boss}'s pit, or in Favor."));
		else
			run.SetText("standtext", run.Fill("Next month's envelope is ready in {house}. It's tins again, thank God. Somebody has to carry it."));

		return S_STAND;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterStand(RZ_QuestRun run, int outcome)
	{
		int path = run.Flag("path");
		bool done = outcome == OUTCOME_DONE || outcome == OUT_TINS || outcome == OUT_INSTALMENT;
		if (done && path == PATH_HELD)
			return Finish(run, E_STANDS, "Stands alone. {house} is boarded up tight, and it isn't paying anybody.");

		if (done && path == PATH_BASTION)
			return Finish(run, E_BASTION, "Under Bastion. {house} is boarded up, and Bastion has written it into its census.");

		if (done && path == PATH_DEBT)
			return Finish(run, E_DEBT, "The debt is yours. Nobody was sold, and {house} owes the crew nothing.");

		if (done)
			return Finish(run, E_TINS, "Paid in tins. {house} pays the crew every month, and the crew leaves it alone.");

		if (run.Visits(S_STAND) < 2)
			return S_STAND;

		if (path == PATH_DEBT)
			return Finish(run, E_TINS, "Paid in tins. The debt was never paid, so it went back to {house}, and the house pays it in tins.");

		return Finish(run, E_TINS, "Paid in tins. Nothing was done in time, so {house} went back to paying the crew, as before.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
		RZ_Questlines q = gm.GetQuestlines();
		int house = run.Flag("house", -1);
		if (ending == E_STANDS || ending == E_BASTION)
			ReplaceIfVoted(gm, run);

		switch (ending)
		{
			case E_STANDS:
			{
				q.SetWorldFlag("door.shut", 1);
				ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 10, 0);
				ChangeStanding(gm, run, RZ_EFaction.BANDITS, 0, 10);
				RZ_HavenQuest.LeaderRemembers(gm, run, RZ_HavenQuest.KeptLeader(gm, run, "boss"), RZ_Leaders.MEM_CROSSED);
				NudgeMorale(gm, RZ_EFaction.SURVIVORS, 4);
				Headline(gm, run.Fill("The safehouse in {house} has stopped paying the Red Hand. It has boards on its windows instead."));
				SpreadRumour(gm, run.Fill("\"{house} told the Red Hand no. To their faces. I'd have liked to see that.\""), 6);
				return;
			}
			case E_BASTION:
			{
				q.SetWorldFlag("door.bastion", 1);
				q.SetWorldFlag("door.shut", 1);
				ChangeStanding(gm, run, RZ_EFaction.MILITARY, 10, 0);
				ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 8, 0);
				Headline(gm, run.Fill("Bastion patrols the road to {house} now, and has the house on its census."));
				SpreadRumour(gm, run.Fill("\"There's a Bastion truck on the road to {house} most nights. They get bread. The Red Hand gets nothing.\""), 6);
				return;
			}
			case E_TINS:
			{
				q.SetWorldFlag("door.tins", 1);
				ChangeStanding(gm, run, RZ_EFaction.BANDITS, 5, 0);
				ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 5, 0);
				ChangeFood(gm, house, -4);
				Headline(gm, run.Fill("The safehouse in {house} eats a little less every month. The Red Hand never bothers it."), false);
				SpreadRumour(gm, run.Fill("\"{house} pays. Everybody knows. Nobody says.\""), 6);
				return;
			}
			case E_DEBT:
			{
				ChangeStanding(gm, run, RZ_EFaction.BANDITS, 10, 0);
				ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 10, 0);
				Headline(gm, run.Fill("The safehouse in {house} owes the Red Hand nothing. Somebody else is paying its debt."), false);
				SpreadRumour(gm, run.Fill("\"Somebody took {house}'s debt to the Red Hand on themselves. Mad, or kind. Same thing these days.\""), 6);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The vote went against the council head: they leave the chair, if they're still in it.
	protected void ReplaceIfVoted(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.Flag("against") != 1)
			return;

		RZ_Leader head = RZ_HavenQuest.KeptLeader(gm, run, "head");
		if (head && ReplaceLeader(gm, head.m_iId, string.Format("%1 has stepped down as council head in %2. The house voted.", head.m_sName, head.m_sPlace)))
			gm.GetQuestlines().SetWorldFlag("door.head_replaced", 1);
	}

	//------------------------------------------------------------------------------------------------
	override string Rumour(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.m_iStage == S_ENVELOPE || run.m_iStage == S_GRID)
			return "\"The safehouse in {house} pays the Red Hand to be left alone. This month it isn't tins, they say.\"";

		if (run.m_iStage == S_TALK || run.m_iStage == S_VOTE)
			return "\"{house} is voting on its council head. About time, some say. Others say the opposite, louder.\"";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override string OnLeaderTalk(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, int leaderId)
	{
		if (!run.PlayerTookPart(gm, playerId))
			return "";

		if (leaderId == run.Flag("head", -1) && (run.m_iStage == S_TALK || run.m_iStage == S_VOTE) && RZ_HavenQuest.KeptLeader(gm, run, "head"))
			return "\"They're voting on me tonight. Say what you think. I'll still be here in the morning, or I won't.\"";

		if (leaderId == run.Flag("boss", -1) && run.m_iStage == S_COLLECT && RZ_HavenQuest.KeptLeader(gm, run, "boss"))
			return "\"{house} owes. I'm going to collect. You can be there or not.\"";

		return "";
	}
}

//------------------------------------------------------------------------------------------------
//! The Price of the Door, stage 1 (Haven or Red Hand contract, Known): carry the envelope. A Quiet
//! Arrangement's walk: the envelope under a loose board by the door, the crew's drop 150-250 m
//! outside the crew boss's seat, two of the house's lookouts along the way (RZ_HavenLookouts; within
//! 35 m, 25 m for ★ Ex-Con, or a Haven wanderer: +12 Haven Infamy, once). Take it sealed, open it
//! first (the grid and the name), or leave it (refused). Only the carrier hands it over. Stage 5's
//! "next month's envelope" is the same walk with tins in it (m_bTins).
class RZ_QM_DoorEnvelope : RZ_HavenQM
{
	bool m_bTins;
	protected int m_iHouse = -1;
	protected vector m_vBoard;
	protected vector m_vDrop;
	protected bool m_bDropUp;
	protected bool m_bSeen;
	protected int m_iBoardClue = -1;
	protected ref RZ_HavenLookouts m_Lookouts = new RZ_HavenLookouts();

	protected static const int OBJ_ENVELOPE = 0;
	protected static const int OBJ_DROP = 1;
	protected static const float SEEN_RADIUS = 35;
	protected static const float SEEN_RADIUS_BONUS = 25;
	protected static const int SEEN_INFAMY = 12;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		RZ_EFaction giver = RZ_EFaction.SURVIVORS;
		if (QFlag(gm, "side") == 1)
			giver = RZ_EFaction.BANDITS;

		SetContract(giver, RZ_ETier.KNOWN, 8, 15);
		m_Req.Bonus("excon");
		m_fTimeLeft = 2700;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iHouse = QFlag(gm, "house", -1);
		RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, m_iHouse);
		RZ_Leader boss = RZ_HavenQuest.KeptLeader(gm, Run(gm), "boss");
		if (!h || !boss)
			return false;

		m_iSettlement = h.m_iSettlement;
		m_vBoard = LandPointNear(h.m_vPos, 2, 5);
		m_vPos = m_vBoard;
		m_vDrop = LandPointNear(boss.m_vPos, 150, 250);
		if (m_bTins)
		{
			m_iBoardClue = AddClue("Under the loose board", "A flour sack of tins, the house's best, counted twice and tied with string. Next month's price.");
			m_sTitle = "The Price of the Door: the tins";
		}
		else
		{
			m_iBoardClue = AddClue("Under the loose board", "An envelope, sealed with candle wax. It's too thin for money.");
			m_sTitle = "The Price of the Door: the envelope";
		}

		if (QFlag(gm, "side") == 1)
		{
			VoiceBy(gm, "boss");
			m_sBrief = QFill(gm, "The safehouse in {house} pays us to keep the door on its hinges. Their keeper can't be seen walking it over. You can. Under the loose board by the door. Our drop's outside {boss_place}. Don't open it.");
			if (m_bTins)
				m_sBrief = QFill(gm, "It's tins again this month, so {house} has remembered its manners. Same board, same drop outside {boss_place}. Same lookouts, too, so mind them.");
		}
		else
		{
			VoiceBy(gm, "head");
			m_sBrief = QFill(gm, "There's an envelope under the loose board by our door. It goes to the Red Hand's drop outside {boss_place}. I'd rather not know what's in it, and nobody here must see you carry it. Not even our own lookouts.");
			if (m_bTins)
				m_sBrief = QFill(gm, "Next month's price is under the board. Tins, this time. Take it to the drop outside {boss_place}, and keep away from our lookouts. The house still mustn't see it go.");
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		string choices = "Take it, sealed|Open it, then take it|Leave it where it is";
		if (m_bTins)
			choices = "Take the tins|Leave them where they are";

		if (!SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vBoard, OBJ_ENVELOPE, "", choices, m_iBoardClue, "Lift the loose board"))
			return false;

		m_Lookouts.SpawnAlong(gm, m_vBoard, m_vDrop, m_iHouse, 2);
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
			NotifyParticipants(gm, m_sTitle, QFill(gm, "Whoever had it is gone. It's back under the board in {house}."));
			return;
		}

		IEntity ent = PlayerEnt(carrier);
		if (!m_bDropUp && RZ_Util.DistXZ(ent.GetOrigin(), m_vDrop) <= 300)
		{
			m_bDropUp = true;
			string label = "Hand over the envelope";
			if (m_bTins)
				label = "Hand over the tins";

			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vDrop, OBJ_DROP, "", label, -1, "");
		}

		// Leaving the house with it is fine: it was handed over in private
		if (m_bSeen || RZ_Util.DistXZ(ent.GetOrigin(), m_vBoard) < 40)
			return;

		float radius = SEEN_RADIUS;
		if (HasBonus(gm, carrier))
			radius = SEEN_RADIUS_BONUS;

		string who = m_Lookouts.SeenBy(gm, ent.GetOrigin(), radius, true);
		if (who.IsEmpty())
			return;

		m_bSeen = true;
		SetQFlag(gm, "seen", 1);
		m_Quiet.Spot(carrier);
		gm.AddInfamy(carrier, RZ_EFaction.SURVIVORS, SEEN_INFAMY);
		gm.NotifyPlayer(carrier, m_sTitle, string.Format("%1 sees what you're carrying and knows exactly what it is. By tonight, so will the safehouse in %2. (+%3 Haven Infamy)", who, QText(gm, "house"), SEEN_INFAMY));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx == OBJ_DROP)
		{
			Deliver(gm, playerId, option);
			return;
		}

		if (objIdx != OBJ_ENVELOPE)
			return;

		if (m_iCarrier >= 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "The board is empty. Somebody already has it.");
			return;
		}

		if (!MayDecide(gm, playerId, "Whoever keeps the board"))
			return;

		int leaveOption = 2;
		if (m_bTins)
			leaveOption = 1;

		if (option == leaveOption)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You put the board back over it. Somebody else can carry the house's price, or nobody can.");
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_PriceOfTheDoor.OUT_REFUSED, "wouldn't carry it", false);
			return;
		}

		if (option == 1 && !m_bTins)
		{
			SetQFlag(gm, "knows", 1);
			gm.GetQuestlines().SetWorldFlag("door.knows", 1);
			Remember(gm, playerId, RZ_EQuestApproach.SNEAK, "opened the envelope");
			RZ_Settlement barn = gm.GetSettlement(QFlag(gm, "barn", -1));
			string grid = "";
			if (barn)
				grid = RZ_HavenQuest.Grid(barn.m_vCenter);

			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, string.Format("You lift the wax with a warm knife. A page from a notebook, in careful pencil: \"{exile}. Put out for the stores. Sleeps in the barn outside {barn}, grid %1.\" You seal it again. It almost looks untouched.", grid)));
		}
		else if (option != 0)
		{
			return;
		}

		GiveTo(playerId);
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You have it. The crew's drop is outside {boss_place}. The house's lookouts are out on the road."));
	}

	//------------------------------------------------------------------------------------------------
	protected void Deliver(RZ_GameModeComponent gm, int playerId, int option)
	{
		if (option != 0 || m_bResolved)
			return;

		if (playerId != m_iCarrier)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"Not from you, friend. Whoever's carrying it brings it.\"");
			return;
		}

		int outcome = RZ_Quest_PriceOfTheDoor.OUT_PAID;
		string text = "The runner weighs the envelope in his hand and grins. \"Lighter than tins. Worth more.\"";
		if (m_bTins)
		{
			outcome = RZ_Quest_PriceOfTheDoor.OUT_TINS;
			text = QFill(gm, "The runner counts the tins twice. \"Tell {house} we'll see them next month.\"");
		}

		gm.NotifyPlayer(playerId, m_sTitle, text);
		Resolve(gm, playerId, RZ_EQuestApproach.TALK, outcome, "carried the house's price");
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return super.GetWhereText(gm, from) + "\nThe crew's drop: grid " + RZ_HavenQuest.Grid(m_vDrop);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Price of the Door: {house}'s price reached the crew.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Price of the Door: {house}'s price never reached the crew.");
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		result.Insert(m_vDrop);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Lookouts.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! The Price of the Door, stage 2 (Haven contract; the Red Hand's in its version, Known): the man
//! at the grid. {exile} sleeps in a barn outside {barn}; he is the escort (RZ_Escort, a figure or
//! the survivor group), talked to on his body. Warn him (he takes to the roads as a wanderer, if
//! the roster has room), bring him back to plead (he follows you to the house), or walk him to the
//! crew's drop (two lookouts on the way; seen, or a Haven wanderer near him: +15 Haven Infamy). Time
//! up: the crew found him. Which of these pays depends on who gave the job.
class RZ_QM_DoorExile : RZ_HavenQM
{
	protected ref RZ_Escort m_Escort = new RZ_Escort();
	protected ref RZ_HavenLookouts m_Lookouts = new RZ_HavenLookouts();
	protected int m_iHouse = -1;
	protected vector m_vBarn;
	protected vector m_vHouse;
	protected vector m_vDrop;
	protected int m_iTalk = -1;
	protected int m_iTo = -1;				// TO_HOME or TO_CREW once he's walking
	protected bool m_bSeen;
	protected bool m_bRedHand;				// the Red Hand gave the job

	protected static const int OBJ_EXILE = 0;
	protected static const int TO_HOME = 0;
	protected static const int TO_CREW = 1;
	protected static const float SEEN_RADIUS = 35;
	protected static const int SEEN_INFAMY = 15;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		m_bRedHand = QFlag(gm, "side") == 1;
		RZ_EFaction giver = RZ_EFaction.SURVIVORS;
		if (m_bRedHand)
			giver = RZ_EFaction.BANDITS;

		SetContract(giver, RZ_ETier.KNOWN, 12, 12);
		m_Req.Bonus("elder");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iHouse = QFlag(gm, "house", -1);
		RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, m_iHouse);
		RZ_Settlement barn = gm.GetSettlement(QFlag(gm, "barn", -1));
		RZ_Leader boss = RZ_HavenQuest.KeptLeader(gm, Run(gm), "boss");
		if (!h || !barn || !boss)
			return false;

		m_vHouse = h.m_vPos;
		m_vBarn = LandPointNear(barn.m_vCenter, barn.m_fRadius * 0.8, barn.m_fRadius + 80);
		m_vDrop = LandPointNear(boss.m_vPos, 150, 250);
		m_vPos = m_vBarn;
		m_iSettlement = QFlag(gm, "barn", -1);
		m_iTalk = AddClue(QText(gm, "exile"), QFill(gm, "A man asleep in the straw with his boots on, who's awake before you've taken a step. \"I took bread. Two loaves, when there was nothing else. They put me out that night, no gun, no coat.\" He looks at your hands. \"Who sent you?\""));
		m_Escort.m_sName = QText(gm, "exile");
		m_Escort.m_sPrefab = RZ_Escort.PrefabFor(gm, 0);
		m_Escort.SetTalk(m_iId, OBJ_EXILE, "Warn him: they know where you sleep|Bring him back to plead his case|Walk him to the crew", m_iTalk, "Talk to " + QText(gm, "exile"));
		if (!m_Escort.Begin(gm, m_vBarn))
			return false;

		m_sTitle = "The Price of the Door: the man in the barn";
		if (m_bRedHand)
		{
			VoiceBy(gm, "boss");
			m_sBrief = QFill(gm, "The man in the envelope sleeps in a barn outside {barn}. Walk him to our drop outside {boss_place} and save my lads the trip. He'll come quietly if you're friendly about it.");
		}
		else
		{
			VoiceBy(gm, "head");
			m_sBrief = QFill(gm, "Somebody sold where {exile} sleeps: a barn outside {barn}. He was ours once. The crew will come for him. Get there first, and do what you think is right. I can't.");
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_Escort.Update(gm, dt, ids, players);
		m_vPos = m_Escort.m_vPos;
		if (m_bResolved)
			return;

		if (m_Escort.IsDead())
		{
			SetQFlag(gm, "exile_dead", 1);
			NotifyParticipants(gm, m_sTitle, QFill(gm, "{exile} is dead. The infected got there before anyone could stop it."));
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (m_iTo < 0)
			return;

		RZ_ContractHelp.AddNear(ids, players, m_vPos, 100, m_aParticipants);
		if (m_iTo == TO_HOME)
		{
			if (!m_Escort.IsNear(m_vHouse, 30))
				return;

			m_Escort.Arrive();
			int home = m_Escort.GetFollowed();
			NotifyParticipants(gm, m_sTitle, QFill(gm, "{exile} stops at the gate of {house} with his cap in his hands. \"I'll say it to their faces. That's all I want.\""));
			Resolve(gm, home, RZ_EQuestApproach.TALK, RZ_Quest_PriceOfTheDoor.OUT_HOME, "brought him back to plead", !m_bRedHand);
			return;
		}

		// Walking him to the crew: the house's lookouts and Haven wanderers on the road see it
		int leader = m_Escort.GetFollowed();
		if (!m_bSeen)
		{
			string who = m_Lookouts.SeenBy(gm, m_vPos, SEEN_RADIUS, true);
			if (!who.IsEmpty() && leader > 0)
			{
				m_bSeen = true;
				gm.AddInfamy(leader, RZ_EFaction.SURVIVORS, SEEN_INFAMY);
				gm.NotifyPlayer(leader, "The Haven", string.Format("%1 sees you walking %2 toward the Red Hand. Every house will hear who sold him. (+%3 Haven Infamy)", who, QText(gm, "exile"), SEEN_INFAMY));
			}
		}

		if (!m_Escort.IsNear(m_vDrop, 25))
			return;

		m_Escort.Arrive();
		NotifyParticipants(gm, m_sTitle, QFill(gm, "The crew's men take {exile} by the arms, not roughly. He doesn't look back at you. He doesn't have to."));
		if (!m_bRedHand && leader > 0)
			RZ_QuestApproach.SideWith(gm, leader, RZ_EFaction.BANDITS, -1, 5, 0);

		Resolve(gm, leader, RZ_EQuestApproach.BETRAY, RZ_Quest_PriceOfTheDoor.OUT_SOLD, "walked him to the crew", m_bRedHand);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_EXILE || m_iTo >= 0 || m_bResolved)
			return;

		if (!MayDecide(gm, playerId, QText(gm, "exile")))
			return;

		if (option == 0)
		{
			string gone = QFill(gm, "{exile} is up and gone before you've finished, over the back of the barn and away across the fields. \"Tell nobody which way.\"");
			if (gm.GetWanderers() && !gm.GetWanderers().AddFromLeaver(LandPointNear(m_vBarn, 800, 1500)).IsEmpty())
				gone += " You'll see him on the roads, maybe. Under another name.";

			gm.NotifyPlayer(playerId, m_sTitle, gone);
			if (m_bRedHand)
				RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.SURVIVORS, RZ_EFaction.BANDITS, 3, 5);

			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_PriceOfTheDoor.OUT_WARNED, "warned him", !m_bRedHand);
			return;
		}

		if (option == 1)
		{
			m_iTo = TO_HOME;
			m_Escort.Follow(playerId);
			m_Escort.Go();
			Remember(gm, playerId, RZ_EQuestApproach.TALK, "is bringing him home");
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "He thinks about it for a long time. \"They'll want someone to speak for me. Someone they trust.\" He follows you. {house}: " + RZ_Util.DirectionText(m_vBarn, m_vHouse) + "."));
			return;
		}

		if (option == 2)
		{
			m_iTo = TO_CREW;
			m_Escort.Follow(playerId);
			m_Escort.Go();
			m_Lookouts.SpawnAlong(gm, m_vBarn, m_vDrop, m_iHouse, 2);
			Remember(gm, playerId, RZ_EQuestApproach.BETRAY, "is walking him to the crew");
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "\"Somewhere safer,\" you tell him, and he believes you, because he wants to. The crew's drop outside {boss_place}: grid " + RZ_HavenQuest.Grid(m_vDrop) + "."));
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = super.GetWhereText(gm, from);
		if (m_iTo == TO_HOME)
			text += QFill(gm, "\n{house}: ") + RZ_Util.DirectionText(from, m_vHouse);
		else if (m_iTo == TO_CREW)
			text += "\nThe crew's drop: grid " + RZ_HavenQuest.Grid(m_vDrop);

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (QFlag(gm, "exile_dead") == 1)
			return QFill(gm, "The Price of the Door: {exile} didn't make it.");

		return QFill(gm, "The Price of the Door: nobody went to the barn outside {barn}. The crew did.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Escort.Cleanup();
		m_Lookouts.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! The Price of the Door, stage 3 (Haven contract, Known): the crew comes to collect. {boss} at the
//! gate on the road in (70-90 m out, beyond the house's no-fire rule). Pay him off (40 Haven or 25
//! Red Hand Favor), take the debt on yourself (persuade the Red Hand, difficulty 2; ★ Ex-Con +1),
//! or tell him no: with the Red Hand group prefab, a raid comes a minute later and the gate must be
//! held 3 minutes (or the raiders killed). The house's bread can go to the nearest Bastion seat
//! for a patrol (carried by one player; the house's stores lose 2 food). Time up: nobody stood up.
class RZ_QM_DoorCollect : RZ_HavenQM
{
	protected int m_iHouse = -1;
	protected vector m_vGate;
	protected vector m_vBread;
	protected vector m_vPost;
	protected string m_sPost;
	protected bool m_bPost;
	protected bool m_bPostUp;
	protected bool m_bRaid;
	protected bool m_bRaidSent;
	protected float m_fRaidIn = 60;
	protected int m_iBossTalk = -1;
	protected ref RZ_UnitWatch m_Raiders = new RZ_UnitWatch();

	protected static const int OBJ_BOSS = 0;
	protected static const int OBJ_BREAD = 1;
	protected static const int OBJ_POST = 2;
	protected static const int HAVEN_PRICE = 40;
	protected static const int REDHAND_PRICE = 25;
	protected static const int BREAD_FOOD = 2;
	protected static const float HOLD_SECONDS = 180;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 15, 15);
		m_Req.Bonus("excon");
		m_fTimeLeft = 3000;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iHouse = QFlag(gm, "house", -1);
		RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, m_iHouse);
		if (!h || !RZ_HavenQuest.KeptLeader(gm, Run(gm), "boss"))
			return false;

		m_iSettlement = h.m_iSettlement;
		m_vGate = LandPointNear(h.m_vPos, 70, 90);
		m_vBread = LandPointNear(h.m_vPos, 3, 7);
		m_vPos = m_vGate;
		RZ_Leader post = RZ_HavenQuest.NearestSeat(gm, RZ_EFaction.MILITARY, h.m_vPos, 6000);
		if (post)
		{
			m_bPost = true;
			m_vPost = LandPointNear(post.m_vPos, 6, 12);
			m_sPost = string.Format("%1, %2", post.m_sShort, post.m_sPlace);
		}

		m_iBossTalk = AddClue(QText(gm, "boss"), QFill(gm, "{boss} leans on the gate like he built it. \"The house owes me a month. I'll take it in tins, or in people, or in whatever you've got. I'm not fussy. I'm only owed.\""));
		VoiceBy(gm, "head");
		m_sTitle = "The Price of the Door: the crew comes to collect";
		m_sBrief = QFill(gm, "The price wasn't paid, and {boss} is coming to {house} for it in person. We can't fight the Red Hand. Can you? Or pay them, or talk to them, or find someone who'll stand with us. Before dark.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		string refuse = "";
		if (!gm.GetRedHandGroupPrefab().IsEmpty())
			refuse = "Tell him no";

		string choices = string.Format("Pay him off (%1 Haven Favor)|Pay him off (%2 Red Hand Favor)|Make the debt yours|%3", HAVEN_PRICE, REDHAND_PRICE, refuse);
		if (!SpawnPerson(gm, m_vGate, OBJ_BOSS, m_iBossTalk, "Talk to " + QText(gm, "boss"), choices))
			return false;

		if (m_bPost)
			SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vBread, OBJ_BREAD, "", "Take the house's bread to Bastion", -1, "");

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved)
			return;

		// The bread for Bastion: the post's people turn up when the carrier is near
		int carrier = Carrier();
		if (carrier > 0 && !m_bPostUp && IsNear(carrier, m_vPost, 300))
		{
			m_bPostUp = true;
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPost, OBJ_POST, "", "Hand over the bread: we need a patrol", -1, "");
		}

		if (!m_bRaid)
			return;

		if (!m_bRaidSent)
		{
			if (CountPlayersNear(players, m_vGate, 120) == 0)
				return;

			m_fRaidIn -= dt;
			if (m_fRaidIn > 0)
				return;

			m_bRaidSent = true;
			if (!m_Raiders.Spawn(gm.GetRedHandGroupPrefab(), LandPointNear(m_vGate, 250, 350)))
				return;

			RZ_Util.SendGroupTo(m_Raiders.m_Unit, m_vGate, gm.GetSpawner().GetMoveWaypointPrefab());
			RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, m_Raiders.m_Unit);
			NotifyParticipants(gm, m_sTitle, "Here they come up the road. Hold the gate.");
			return;
		}

		bool beaten = m_Raiders.IsDead(dt);
		if (!beaten && !TickHold(gm, dt, ids, players, m_vGate, 30, HOLD_SECONDS, 0, 0))
			return;

		array<int> near = {};
		GetPlayersNear(ids, players, m_vGate, 150, near);
		int by = -1;
		if (!near.IsEmpty())
			by = near[0];

		foreach (int playerId : near)
		{
			Remember(gm, playerId, RZ_EQuestApproach.FORCE, "held the gate");
		}

		NotifyParticipants(gm, m_sTitle, QFill(gm, "The crew pulls back down the road. {boss} shouts something about next month, and nobody at the gate answers."));
		Resolve(gm, by, RZ_EQuestApproach.FORCE, RZ_Quest_PriceOfTheDoor.OUT_HELD, "held the gate");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx == OBJ_POST)
		{
			HandBread(gm, playerId, option);
			return;
		}

		if (m_bRaid)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "It's too late for talking. Hold the gate.");
			return;
		}

		if (!MayDecide(gm, playerId, QText(gm, "boss")))
			return;

		if (objIdx == OBJ_BREAD)
		{
			if (option != 0 || m_iCarrier >= 0)
				return;

			GiveTo(playerId);
			Remember(gm, playerId, RZ_EQuestApproach.SIDE, "went for Bastion");
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("Two sacks of the house's bread. Bastion's post: %1, grid %2. %3 will wait at the gate, smoking, for a while.", m_sPost, RZ_HavenQuest.Grid(m_vPost), QText(gm, "boss")));
			return;
		}

		if (objIdx != OBJ_BOSS)
			return;

		if (option == 0 && TryBribe(gm, playerId, RZ_EFaction.SURVIVORS, HAVEN_PRICE))
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{boss} takes it, counts it, and tips his hat to the house. \"Pleasure. Same time next month.\""));
			Resolve(gm, playerId, RZ_EQuestApproach.BRIBE, RZ_Quest_PriceOfTheDoor.OUT_BOUGHT, "paid the crew off");
			return;
		}

		if (option == 1 && TryBribe(gm, playerId, RZ_EFaction.BANDITS, REDHAND_PRICE))
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{boss} laughs. \"Paying me with my own people's favours. I like you.\" He goes."));
			Resolve(gm, playerId, RZ_EQuestApproach.BRIBE, RZ_Quest_PriceOfTheDoor.OUT_BOUGHT, "paid the crew off");
			return;
		}

		if (option == 2)
		{
			if (!TryPersuade(gm, playerId, RZ_EFaction.BANDITS, 2, "excon"))
			{
				Remember(gm, playerId, RZ_EQuestApproach.PERSUADE, "offered to take the debt");
				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{boss} looks you up and down. \"Yours? I don't know you well enough to own you.\""));
				return;
			}

			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{boss} thinks about it, then spits on his palm and holds it out. \"A debt's a debt. Doesn't much matter whose. It's yours now. First payment soon.\""));
			Resolve(gm, playerId, RZ_EQuestApproach.PERSUADE, RZ_Quest_PriceOfTheDoor.OUT_DEBT, "took the house's debt on");
			return;
		}

		if (option == 3 && !gm.GetRedHandGroupPrefab().IsEmpty())
		{
			m_bRaid = true;
			Remember(gm, playerId, RZ_EQuestApproach.FORCE, "told the crew no");
			NotifyParticipants(gm, m_sTitle, QFill(gm, "\"No,\" you say. {boss} nods as if he expected it, and walks back down the road without hurrying. He'll be back in a minute, with friends."));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! At the Bastion seat: the carrier hands the bread over and a patrol comes.
	protected void HandBread(RZ_GameModeComponent gm, int playerId, int option)
	{
		if (option != 0 || m_bResolved)
			return;

		if (playerId != m_iCarrier)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"Whose bread is this? Bring the one who's carrying it.\"");
			return;
		}

		RZ_Questline.ChangeFood(gm, m_iHouse, -BREAD_FOOD);
		RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.MILITARY, -1, 4, 0);
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "A sergeant tears a loaf in half and eats it standing. \"A patrol, for the night. We'll park on the road to {house} and see who comes.\" Nobody comes."));
		Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_PriceOfTheDoor.OUT_BASTION, "brought a Bastion patrol");
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = super.GetWhereText(gm, from);
		if (m_iCarrier > 0 && m_bPost)
			text += string.Format("\nBastion's post: %1, grid %2", m_sPost, RZ_HavenQuest.Grid(m_vPost));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Price of the Door: nobody stood at the gate of {house}.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Raiders.Delete();
	}
}

//------------------------------------------------------------------------------------------------
//! The Price of the Door, stage 4 (Haven contract, Known): the vote, at the house. Speak for the
//! council head (Known), or against them (persuade the Haven, difficulty 2, ★ Elder +1): either
//! decides it. If {exile} came home, someone may speak for him first (the same check): he's let
//! back in, one more person. Time up: nobody spoke, and the council head keeps the chair.
class RZ_QM_DoorVote : RZ_HavenQM
{
	protected int m_iHouse = -1;
	protected int m_iTalk = -1;
	protected int m_iExileTalk = -1;
	protected bool m_bExile;

	protected static const int OBJ_MEETING = 0;
	protected static const int OBJ_EXILE = 1;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 8, 8);
		m_Req.Bonus("elder");
		m_fTimeLeft = 2700;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iHouse = QFlag(gm, "house", -1);
		RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, m_iHouse);
		if (!h || !RZ_HavenQuest.KeptLeader(gm, Run(gm), "head"))
			return false;

		m_iSettlement = h.m_iSettlement;
		m_vPos = LandPointNear(h.m_vPos, 4, 9);
		m_bExile = QFlag(gm, "exile_home") == 1;
		m_iTalk = AddClue("The meeting", QFill(gm, "Everyone who sleeps in {house} is in the one room, standing, because there aren't enough chairs. {head} sits at the front and doesn't say anything. The cook has her arms folded. Somebody says the word \"price\", and the room goes quiet."));
		if (m_bExile)
			m_iExileTalk = AddClue(QText(gm, "exile"), QFill(gm, "{exile} stands at the back by the door, where he can leave fast. \"I took bread. I'd take it again. Ask them who they'd have fed.\""));

		m_sTitle = "The Price of the Door: the vote";
		m_sBrief = QFill(gm, "{house} votes tonight on whether {head} keeps the chair. Everyone gets a say, even you. Say it carefully.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		string head = QText(gm, "head");
		if (!SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vPos, OBJ_MEETING, "", string.Format("Speak for %1|Speak against %1", head), m_iTalk, "Listen to the meeting"))
			return false;

		if (m_bExile)
			SpawnPerson(gm, LandPointNear(m_vPos, 3, 6), OBJ_EXILE, m_iExileTalk, "Talk to " + QText(gm, "exile"), "Speak for him");

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (!MayDecide(gm, playerId, "The room"))
			return;

		if (objIdx == OBJ_EXILE && option == 0 && m_bExile)
		{
			if (QFlag(gm, "exile_in") == 1)
				return;

			if (!RZ_QuestApproach.Persuade(gm, playerId, RZ_EFaction.SURVIVORS, 2, "elder"))
			{
				Remember(gm, playerId, RZ_EQuestApproach.PERSUADE, "spoke for him, and wasn't heard");
				gm.NotifyPlayer(playerId, m_sTitle, "They hear you out, politely, and don't change their minds. They'd need to hear it from someone they trust.");
				return;
			}

			SetQFlag(gm, "exile_in", 1);
			RZ_Questline.ChangePopulation(gm, m_iHouse, 1);
			Remember(gm, playerId, RZ_EQuestApproach.PERSUADE, "spoke for him");
			NotifyParticipants(gm, m_sTitle, QFill(gm, "Hands go up, slowly, then most of them. {exile} is let back in. Somebody gives him a coat."));
			return;
		}

		if (objIdx != OBJ_MEETING)
			return;

		if (option == 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You say what {head} kept the house through. It's enough. {head} keeps the chair, and looks as if they'd rather not."));
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_PriceOfTheDoor.OUT_FOR, "spoke for the council head");
			return;
		}

		if (option != 1)
			return;

		if (!RZ_QuestApproach.Persuade(gm, playerId, RZ_EFaction.SURVIVORS, 2, "elder"))
		{
			Remember(gm, playerId, RZ_EQuestApproach.PERSUADE, "spoke against the council head, and wasn't heard");
			gm.NotifyPlayer(playerId, m_sTitle, "\"Who asked you?\" somebody says, and that's the end of it. They'd need to hear it from someone they trust.");
			return;
		}

		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You say what the price was. The cook stands up. Then the rest of them. {head} goes out without a word, and the cook takes the chair for now."));
		Resolve(gm, playerId, RZ_EQuestApproach.PERSUADE, RZ_Quest_PriceOfTheDoor.OUT_AGAINST, "spoke against the council head");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Price of the Door: nobody spoke at the vote. {head} keeps the chair.");
	}
}

//------------------------------------------------------------------------------------------------
//! The Price of the Door, stage 5 when the house held (or Bastion came): the barricades. This is the
//! contract Fortify the Safehouse (Missions/Haven/RZ_HavenContracts.c) at the questline's house: hold
//! it 3 minutes through 3 waves, ★ Mechanic faster, and the town counts as Barricaded. Known is
//! enough here. Its success is the stage's.
class RZ_QM_DoorFortify : RZ_Mission_FortifySafehouse
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		super.Configure(gm);
		m_Req.m_eMinTier = RZ_ETier.KNOWN;
		m_iFameReward = 15;
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
		if (!run)
			return false;

		RZ_Safehouse h = RZ_HavenQuest.LiveHouse(gm, run.Flag("house", -1));
		if (!h)
			return false;

		RZ_Leader head = RZ_HavenQuest.CouncilHead(gm, run.Flag("house", -1));
		if (head && !head.IsStronghold())
			m_iLeaderId = head.m_iId;

		m_vPos = h.m_vPos;
		m_iSettlement = h.m_iSettlement;
		m_fParticipationRadius = 80;
		m_sTitle = "The Price of the Door: the barricades";
		m_sBrief = run.Fill("{house} held this month. Next month it has to hold without anybody's help. Stand with the people boarding up the windows. The hammering will bring them.");
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! The Price of the Door, stage 5 when the debt is someone's own (Red Hand contract, anyone not
//! Hunted): the first instalment, near the crew boss's seat. The pit boss offers the crew's pit
//! (House Rules: hold the ring 200 s through 4 waves) or 20 Red Hand Favor.
class RZ_QM_DoorInstalment : RZ_HavenQM
{
	protected vector m_vPit;
	protected bool m_bFighting;
	protected int m_iFighter = -1;
	protected int m_iTalk = -1;

	protected static const int OBJ_PIT_BOSS = 0;
	protected static const int INSTALMENT_FAVOR = 20;
	protected static const float PIT_SECONDS = 200;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.STRANGER, 6, 10);
		m_Req.Bonus("excon");
		m_fTimeLeft = 2700;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Leader boss = RZ_HavenQuest.KeptLeader(gm, Run(gm), "boss");
		if (!boss)
			return false;

		m_vPit = LandPointNear(boss.m_vPos, 60, 150);
		m_vPos = m_vPit;
		m_iSettlement = gm.FindNearestSettlement(m_vPit);
		m_iTalk = AddClue("The pit boss", QFill(gm, "A big man with a ledger and a cashbox. \"You're {house}'s debt now. First instalment. In the ring, or in Favor. The ring's more fun for the rest of us.\""));
		VoiceBy(gm, "boss");
		m_sTitle = "The Price of the Door: the first instalment";
		m_sBrief = QFill(gm, "You took {house}'s debt on. Very noble. First instalment's due. Come to the pit behind our place in {boss_place} and pay it, one way or the other.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		string choices = string.Format("Fight in the pit|Pay %1 Red Hand Favor", INSTALMENT_FAVOR);
		return SpawnPerson(gm, LandPointNear(m_vPit, 6, 10), OBJ_PIT_BOSS, m_iTalk, "Talk to the pit boss", choices) != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bFighting || m_bResolved)
			return;

		if (!TickHold(gm, dt, ids, players, m_vPit, 25, PIT_SECONDS, 45, 4))
			return;

		NotifyParticipants(gm, m_sTitle, "The bell. The crew pays out on you, grudgingly. First instalment: paid.");
		Resolve(gm, m_iFighter, RZ_EQuestApproach.FORCE, RZ_Quest_PriceOfTheDoor.OUT_INSTALMENT, "paid in the pit");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_PIT_BOSS || m_bFighting || m_bResolved || !MayDecide(gm, playerId, "The pit boss"))
			return;

		if (option == 0)
		{
			m_bFighting = true;
			m_iFighter = playerId;
			Remember(gm, playerId, RZ_EQuestApproach.FORCE, "went into the pit");
			NotifyParticipants(gm, m_sTitle, "Into the ring. They open the pens. Stay standing.");
			return;
		}

		if (option == 1 && TryBribe(gm, playerId, RZ_EFaction.BANDITS, INSTALMENT_FAVOR))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "The pit boss writes it in the ledger with a stub of pencil. \"Paid. Boring, but paid.\"");
			Resolve(gm, playerId, RZ_EQuestApproach.BRIBE, RZ_Quest_PriceOfTheDoor.OUT_INSTALMENT, "paid the instalment");
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Price of the Door: the instalment wasn't paid. The debt goes back to {house}.");
	}
}
