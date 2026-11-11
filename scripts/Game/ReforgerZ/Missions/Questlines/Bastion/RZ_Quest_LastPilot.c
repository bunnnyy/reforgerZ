//! The Last Pilot (Task Force Bastion; B3 in docs/QUESTLINES.md). Of the helicopters Bastion came
//! in with, only one still flies, and only one pilot is left who can fly it. On Day 3 he was
//! wingman to the pilot who flew the wounded for the line and was shot down short of it. Lately he
//! asks people what the line looks like from the ground.
//!
//! It starts after any player finishes Last Helicopter (OnLzKept), and it
//! publishes what happens to the helicopter as world flags that the ordinary contracts read
//! (IsHelicopterDown): Last Helicopter and Rescue the Pilot stay off while
//! it's grounded or lost; Rescue the Pilot also waits while this questline runs.
//!
//! 1. Fuel (mission): two jerrycans to the pad (★ Mechanic: one), or the Red Hand's fuel: siphoned
//!    unseen from a depot by one of their seats (sneak; seen: +8 Red Hand Infamy) or bought (bribe,
//!    25 Red Hand Favor: the boss hears Bastion is buying, and a crew comes to look at the flight).
//! 2. Whose flight (mission, no storm): keep the LZ clear until lift-off, and choose the load at the
//!    pilot: Halcyon's samples, the Haven's wounded or Bastion's ammunition (pilot.flew_for: the
//!    faction it flew for, an RZ_EFaction).
//! 3. His friend's wreck (mission, needs the Perimeter): the Day 3 wreck in the warning zone, short
//!    of the kill zone. Secure it 60 s (★ Mechanic: twice as fast), carry the black box to him.
//!    ★ Radio Op: play it at the wreck; then he hears it too.
//! 4. He decides (mission): talk him down (Trusted, or ★ Soldier; Known if he's heard the box),
//!    report him at the stronghold, help him (two cans to a far LZ near the line), or sell the plan
//!    to a Red Hand fixer (Trusted with the Red Hand).
//! 5. The last of it (mission, by branch): one more run; walk him to the stronghold; hold the far LZ
//!    while he lifts off; or keep watch 90 s while a crew burns the helicopter on its pad.
//!
//! Endings: He stays, Grounded, He flew for the line, Burnt for the Red Hand.
class RZ_Quest_LastPilot : RZ_Questline
{
	static const string KEY = "pilot";

	static const int S_FUEL = 0;
	static const int S_FLIGHT = 1;
	static const int S_WRECK = 2;
	static const int S_DECIDE = 3;
	static const int S_LAST = 4;

	static const int OUT_FUELLED = 10;
	static const int OUT_FLOWN = 11;
	static const int OUT_BOX = 12;
	static const int OUT_STAYS = 13;
	static const int OUT_GROUNDED = 14;
	static const int OUT_LINE = 15;
	static const int OUT_REDHAND = 16;
	static const int OUT_RUN = 17;
	static const int OUT_WALKED = 18;
	static const int OUT_LIFTED = 19;
	static const int OUT_BURNT = 20;

	static const int BRANCH_STAYS = 0;
	static const int BRANCH_GROUNDED = 1;
	static const int BRANCH_LINE = 2;
	static const int BRANCH_REDHAND = 3;

	static const int E_STAYS = 0;
	static const int E_GROUNDED = 1;
	static const int E_LINE = 2;
	static const int E_BURNT = 3;

	static const int LOAD_SAMPLES = 1;
	static const int LOAD_WOUNDED = 2;
	static const int LOAD_AMMO = 3;

	//! Why the helicopter is down (pilot.down_reason): the brief says so when it flies again
	static const int DOWN_PILOT = 1;		// no pilot until a new one is found
	static const int DOWN_AIRFRAME = 2;		// lost: until the mechanics patch up a grounded airframe

	static const string WRECK_SPOTS = "The Last Pilot";
	static const float MIN_PRESENCE = 0.2;	// as Last Helicopter: Bastion still flies

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		Info(KEY, "The Last Pilot", RZ_EFaction.MILITARY);

		RZ_QuestStage fuel = MissionStage(S_FUEL, "Fuel", "Of the helicopters Bastion came in with, only one still flies, and only {pilot} is left who can fly it. It's dry. Fuel it at the pad by {stronghold}: two jerrycans, or the Red Hand's fuel, bought or not.");
		fuel.GiveUp(4, "Nobody found fuel for the helicopter. It sits on the pad by {stronghold}, and {pilot} sits in it.");
		fuel.Visits(2);

		RZ_QuestStage flight = MissionStage(S_FLIGHT, "Whose flight", "The helicopter flies from the pad by {stronghold} if the LZ stays clear. Everyone wants the load: Halcyon's samples, the Haven's wounded, Bastion's ammunition.");
		flight.GiveUp(6, "The helicopter never flew that run. {pilot} went back to asking people about the line.");
		flight.Visits(2);

		RZ_QuestStage wreck = MissionStage(S_WRECK, "His friend's wreck", "On Day 3 {pilot} was wingman to {friend}, who flew the wounded for the line and was shot down short of it. The wreck is still there, at the edge of the warning zone. He wants the black box.");
		wreck.GiveUp(4, "Nobody went to {friend}'s wreck. {pilot} stopped asking.");
		wreck.Visits(2);

		RZ_QuestStage decide = MissionStage(S_DECIDE, "He decides", "{pilot} wants to fly for the line. Talk him down, report him to {co}, help him fuel a far LZ, or sell the plan to the Red Hand.");
		decide.GiveUp(4, "{pilot} kept his plan to himself, and {co} kept him on the ground.");
		decide.Visits(2);

		RZ_QuestStage last = MissionStage(S_LAST, "The last of it", "However it ends, it ends at an LZ.");
		last.GiveUp(4, "It never came to an end. {pilot} still talks about the line at the shift change.");
		last.Visits(2);
	}

	// ================================================================ The helicopter, for other content
	//------------------------------------------------------------------------------------------------
	//! Last Helicopter finished: the pilot notices who kept the LZ. Starts this
	//! questline now if the rules allow; otherwise it can start later.
	static void OnLzKept(RZ_GameModeComponent gm)
	{
		if (!gm.GetQuestlines())
			return;

		RZ_BastionQuest.SetWorld(gm, KEY + ".lz_kept", 1);
		gm.GetQuestlines().TryBegin(KEY);
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion's helicopter is grounded or lost right now (pilot.down_until, real time): no Last
	//! Helicopter, no Rescue the Pilot.
	static bool IsHelicopterDown(RZ_GameModeComponent gm)
	{
		int until = RZ_BastionQuest.World(gm, KEY + ".down_until");
		if (until <= 0)
			return false;

		int now = RZ_Guidance.UnixNow();
		return now > 0 && now < until;
	}

	//------------------------------------------------------------------------------------------------
	//! Rescue the Pilot waits while this questline runs (its pilot is busy with the story).
	static bool IsRunning(RZ_GameModeComponent gm)
	{
		return gm.GetQuestlines() && gm.GetQuestlines().FindRunByKey(KEY) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! The story has the helicopter right now (dry on the pad, flying its load, or its last run):
	//! no Last Helicopter run elsewhere at the same time.
	static bool IsHelicopterBusy(RZ_GameModeComponent gm)
	{
		if (!gm.GetQuestlines())
			return false;

		RZ_QuestRun run = gm.GetQuestlines().FindRunByKey(KEY);
		return run && (run.m_iStage == S_FUEL || run.m_iStage == S_FLIGHT || run.m_iStage == S_LAST);
	}

	//------------------------------------------------------------------------------------------------
	//! A Last Helicopter contract is flying (and not just finishing): this story's flights wait.
	static bool IsFlying(RZ_GameModeComponent gm)
	{
		return RZ_BastionQuest.IsActive(gm, RZ_Mission_LastHelicopter);
	}

	//------------------------------------------------------------------------------------------------
	//! Ground the helicopter for `hours` of real time (x the wait scale), and say why when it flies again.
	static void SetDown(RZ_GameModeComponent gm, float hours, int reason)
	{
		int now = RZ_Guidance.UnixNow();
		if (now <= 0)
			return;

		RZ_BastionQuest.SetWorld(gm, KEY + ".down_until", now + RZ_BastionQuest.RealSecondsFrom(gm, hours));
		RZ_BastionQuest.SetWorld(gm, KEY + ".down_reason", reason);
		RZ_BastionQuest.SetWorld(gm, KEY + ".new_pilot", Math.RandomInt(1, 2000000000));
		RZ_BastionQuest.SetWorld(gm, KEY + ".back_told", 0);
	}

	//------------------------------------------------------------------------------------------------
	//! Rescue the Pilot failed: the pilot is dead. Grounded for two real hours until a new pilot is
	//! found; if this questline hasn't happened yet, its pilot was that man, and it never will.
	static void OnPilotLost(RZ_GameModeComponent gm)
	{
		SetDown(gm, 2, DOWN_PILOT);
		if (RZ_BastionQuest.World(gm, KEY + ".ending", -1) < 0 && !IsRunning(gm))
			RZ_BastionQuest.SetWorld(gm, KEY + ".dead", 1);
	}

	//------------------------------------------------------------------------------------------------
	//! The new pilot found after a grounding (a named person: the name is seeded by the flag).
	static string NewPilotName(RZ_GameModeComponent gm)
	{
		RZ_Rng rng = new RZ_Rng(Math.MaxInt(1, RZ_BastionQuest.World(gm, KEY + ".new_pilot", 1)));
		return "Sergeant " + RZ_NoteText.Surname(rng);
	}

	//------------------------------------------------------------------------------------------------
	//! For Last Helicopter's brief, once, the first time it flies again after a grounding. "" if not.
	static string BackLine(RZ_GameModeComponent gm)
	{
		if (RZ_BastionQuest.World(gm, KEY + ".down_until") <= 0 || IsHelicopterDown(gm) || RZ_BastionQuest.World(gm, KEY + ".back_told") == 1)
			return "";

		RZ_BastionQuest.SetWorld(gm, KEY + ".back_told", 1);
		if (RZ_BastionQuest.World(gm, KEY + ".down_reason") == DOWN_AIRFRAME)
			return "Bastion's mechanics have patched up one of the grounded airframes, and it flies, more or less. ";

		return string.Format("Bastion has a pilot again: %1, who learned on the old airframes. ", NewPilotName(gm));
	}

	// ================================================================ Starting
	//------------------------------------------------------------------------------------------------
	//! pilot.flew_for, for other lines (HC3 reads it): the faction the flight was for.
	static int FlewFor(int load)
	{
		if (load == LOAD_SAMPLES)
			return RZ_EFaction.INSTITUTE;

		if (load == LOAD_WOUNDED)
			return RZ_EFaction.SURVIVORS;

		return RZ_EFaction.MILITARY;
	}

	//------------------------------------------------------------------------------------------------
	//! A run position kept in two int flags (metres; saved with the run).
	static void SetPos(RZ_QuestRun run, string key, vector pos)
	{
		run.SetFlag(key + "_x", RZ_Util.RoundToInt(pos[0]));
		run.SetFlag(key + "_z", RZ_Util.RoundToInt(pos[2]));
	}

	//------------------------------------------------------------------------------------------------
	static vector GetPos(RZ_QuestRun run, string key)
	{
		return RZ_Util.GroundPos(Vector(run.Flag(key + "_x"), 0, run.Flag(key + "_z")));
	}

	//------------------------------------------------------------------------------------------------
	static bool BastionFlies(RZ_GameModeComponent gm)
	{
		return gm.GetDirector().GetStageSettings().m_fMilitaryPresence >= MIN_PRESENCE;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsStorm(RZ_GameModeComponent gm)
	{
		return gm.GetEnvironment() && gm.GetEnvironment().IsHelicopterGrounded();
	}

	//------------------------------------------------------------------------------------------------
	//! Needs: someone kept an LZ this season, the helicopter flying and its pilot alive, Bastion's
	//! stronghold, the Perimeter (the wreck is at the line), a person to talk to and the mission item.
	override bool CanStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (RZ_BastionQuest.World(gm, KEY + ".lz_kept") <= 0 || RZ_BastionQuest.World(gm, KEY + ".dead") > 0 || IsHelicopterDown(gm))
			return false;

		// Not while the helicopter is out on a run, or its pilot is lying hurt in a field
		if (IsFlying(gm) || RZ_BastionQuest.IsActive(gm, RZ_Mission_RescueThePilot))
			return false;

		if (!RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN) || gm.GetMissionItemPrefab().IsEmpty())
			return false;

		if (gm.GetCure().IsCured() || !BastionFlies(gm))
			return false;

		RZ_Perimeter perimeter = RZ_PerimeterMissionHelper.Get(gm);
		if (!perimeter || !RZ_PerimeterMissionHelper.HasSpots(perimeter, WRECK_SPOTS, perimeter.GetKillDistance() + 35, 10, 500))
			return false;

		RZ_Leader co = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY);
		if (!co)
			return false;

		vector lz = RZ_Mission.LandPointNear(co.m_vPos, 120, 350);
		if (RZ_Util.IsWater(lz))
			return false;

		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		RZ_BastionQuest.Keep(run, "co", co);
		SetPos(run, "lz", lz);
		run.SetText("pilot", "Warrant Officer " + RZ_NoteText.Surname(rng));
		run.SetText("friend", "Lieutenant " + RZ_NoteText.Surname(rng));
		run.SetText("co", co.m_sShort);
		run.SetText("stronghold", co.m_sPlace);
		run.SetText("far", "a far LZ");
		run.SetPlace(lz, "the pad by " + co.m_sPlace);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		run.Note("{pilot} is the last man in Bastion who can fly the last helicopter. Lately he asks people what the line looks like from the ground.");
	}

	//------------------------------------------------------------------------------------------------
	//! Flights wait out storms; nothing happens once Bastion stops flying, or after the cure.
	override bool IsReady(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (gm.GetCure().IsCured() || !BastionFlies(gm) || !RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY))
			return false;

		if (stage == S_FLIGHT)
			return !IsStorm(gm) && !IsFlying(gm);

		if (stage == S_WRECK)
			return RZ_PerimeterMissionHelper.Get(gm) != null;

		if (stage == S_LAST && (run.m_iBranch == BRANCH_STAYS || run.m_iBranch == BRANCH_LINE))
			return !IsStorm(gm) && !IsFlying(gm);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override RZ_Mission MakeMission(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		switch (stage)
		{
			case S_FUEL: return new RZ_QM_PilotFuel();
			case S_FLIGHT: return new RZ_QM_PilotFlight();
			case S_WRECK: return new RZ_QM_PilotWreck();
			case S_DECIDE: return new RZ_QM_PilotDecide();
		}

		if (stage != S_LAST)
			return null;

		switch (run.m_iBranch)
		{
			case BRANCH_STAYS: return new RZ_QM_PilotRun();
			case BRANCH_LINE: return new RZ_QM_PilotRun();
			case BRANCH_GROUNDED: return new RZ_QM_PilotWalk();
			case BRANCH_REDHAND: return new RZ_QM_PilotBurn();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The commanding officer gives the stages (the pilot is his).
	static int Voice(RZ_GameModeComponent gm)
	{
		RZ_Leader co = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY);
		if (co)
			return co.m_iId;

		return -1;
	}

	// ================================================================ The story
	//------------------------------------------------------------------------------------------------
	override int Next(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		switch (stage)
		{
			case S_FUEL:
			{
				if (outcome == OUT_FUELLED)
				{
					if (run.Flag("bought") == 1)
						run.Note("The helicopter is fuelled with the Red Hand's diesel. The boss knows Bastion was buying.");
					else
						run.Note("The helicopter is fuelled. {pilot} runs the engine up for a minute, just to hear it.");

					return S_FLIGHT;
				}

				run.Note("The helicopter is still dry.");
				return S_FUEL;
			}
			case S_FLIGHT:
			{
				if (outcome == OUT_FLOWN)
				{
					run.Note(FlightNote(run.Flag("load")));
					return S_WRECK;
				}

				run.Note("It lifted off with nobody holding the LZ, and came back with nothing. {pilot} wants to try again.");
				return S_FLIGHT;
			}
			case S_WRECK:
			{
				if (outcome == OUT_BOX)
				{
					if (run.Flag("heard") == 1)
						run.Note("{pilot} has heard the black box: the line, firing on {friend}, and {friend} still talking to the wounded.");
					else
						run.Note("{pilot} has {friend}'s black box. He hasn't played it. It's in his locker.");

					return S_DECIDE;
				}

				run.Note("Nobody brought the black box back.");
				return S_WRECK;
			}
			case S_DECIDE:
				return AfterDecision(run, outcome);
			case S_LAST:
				return AfterLast(run, outcome);
		}

		return Fizzle(run, "");
	}

	//------------------------------------------------------------------------------------------------
	protected string FlightNote(int load)
	{
		if (load == LOAD_SAMPLES)
			return "It flew Halcyon's samples to the lab. Halcyon's people will remember who flew for them.";

		if (load == LOAD_WOUNDED)
			return "It flew the Haven's wounded to a safehouse. Two of them will walk again.";

		return "It flew Bastion's own ammunition between its bases, as ordered.";
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterDecision(RZ_QuestRun run, int outcome)
	{
		switch (outcome)
		{
			case OUT_STAYS:
			{
				run.SetBranch(BRANCH_STAYS);
				run.Note("{pilot} stays. He wants one more run, with the LZ kept clear, to prove he meant it.");
				return S_LAST;
			}
			case OUT_GROUNDED:
			{
				run.SetBranch(BRANCH_GROUNDED);
				run.Note("{co} knows. {pilot} is grounded: someone has to walk him to {stronghold}.");
				return S_LAST;
			}
			case OUT_LINE:
			{
				run.SetBranch(BRANCH_LINE);
				run.Note("There's fuel at {far}. {pilot} goes at the next chance. Someone has to keep that LZ clear while he lifts off.");
				return S_LAST;
			}
			case OUT_REDHAND:
			{
				run.SetBranch(BRANCH_REDHAND);
				run.Note("A Red Hand crew knows when the pad by {stronghold} is quiet. They want someone to keep watch while they take the helicopter.");
				return S_LAST;
			}
		}

		if (run.Visits(S_DECIDE) < 2)
		{
			run.Note("Nobody talked to {pilot}. He's still thinking about the line.");
			return S_DECIDE;
		}

		return Finish(run, E_GROUNDED, "Grounded. Nobody reported {pilot}, but {co} found out anyway, and the helicopter stays on the pad until someone else can fly it.");
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterLast(RZ_QuestRun run, int outcome)
	{
		switch (run.m_iBranch)
		{
			case BRANCH_STAYS:
			{
				if (outcome == OUT_RUN)
				{
					run.SetFlag("ran", 1);
					return Finish(run, E_STAYS, "He stays. {pilot} flew one more run with the LZ clear, and he's still on the roster.");
				}

				if (run.Visits(S_LAST) < 2)
					return S_LAST;

				return Finish(run, E_STAYS, "He stays. Nobody kept the LZ for his last run, but he flew it anyway, and came back.");
			}
			case BRANCH_GROUNDED:
				return Finish(run, E_GROUNDED, "Grounded. {pilot} walked into {stronghold} and handed in his helmet. The helicopter stays on the pad until someone else can fly it.");
			case BRANCH_LINE:
			{
				if (outcome == OUT_LIFTED)
					return Finish(run, E_LINE, "He flew for the line. The line shot him down short of it, like {friend} before him.");

				return Finish(run, E_GROUNDED, "Grounded. {pilot} couldn't get off the ground at {far}, and Bastion found him there.");
			}
			case BRANCH_REDHAND:
			{
				if (outcome == OUT_BURNT)
					return Finish(run, E_BURNT, "Burnt for the Red Hand. The crew couldn't fly it, so they burned it on the pad.");

				if (run.Visits(S_LAST) < 2)
					return S_LAST;

				return Fizzle(run, "The crew never came back for the helicopter. {pilot} still talks about the line at the shift change.");
			}
		}

		return Fizzle(run, "");
	}

	//------------------------------------------------------------------------------------------------
	override void OnEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
		switch (ending)
		{
			case E_STAYS:
			{
				ChangeStanding(gm, run, RZ_EFaction.MILITARY, 10, 0);
				NudgeMorale(gm, RZ_EFaction.MILITARY, 5);
				RZ_BastionQuest.SetWorld(gm, KEY + ".stays", 1);
				if (run.Flag("ran") == 1)
					RZ_Mission_LastHelicopter.Boost(gm, 7200);

				SpreadRumour(gm, run.Fill("\"Bastion's pilot, {pilot}. He was going to fly for the line. Somebody talked him out of it.\""), 8);
				break;
			}
			case E_GROUNDED:
			{
				ChangeStanding(gm, run, RZ_EFaction.MILITARY, 10, 0);
				RZ_BastionQuest.SetWorld(gm, KEY + ".grounded", 1);
				SetDown(gm, 2, DOWN_PILOT);
				SpreadRumour(gm, "\"Bastion's helicopter is on the ground. No pilot. They're teaching a sergeant on the old airframes.\"", 4);
				break;
			}
			case E_LINE:
			{
				RZ_BastionQuest.SetWorld(gm, KEY + ".line", 1);
				RZ_BastionQuest.SetWorld(gm, KEY + ".heli_lost", 1);
				RZ_BastionQuest.SetWorld(gm, KEY + ".dead", 1);
				SetDown(gm, 48, DOWN_AIRFRAME);
				NudgeMorale(gm, RZ_EFaction.MILITARY, -10);
				Headline(gm, run.Fill("Bastion's last pilot flew for the line " + RZ_BastionQuest.When() + ". The line shot him down short of it, like the one before."));
				StartWreck(gm, run);
				SpreadRumour(gm, "\"Bastion hasn't got a helicopter any more. The line saw to that. Second time.\"", 12);
				break;
			}
			case E_BURNT:
			{
				ChangeStanding(gm, run, RZ_EFaction.BANDITS, 10, 0, RZ_EQuestApproach.SIDE);
				ChangeStanding(gm, run, RZ_EFaction.MILITARY, 0, 15, RZ_EQuestApproach.SIDE);
				RZ_BastionQuest.SetWorld(gm, KEY + ".redhand", 1);
				RZ_BastionQuest.SetWorld(gm, KEY + ".heli_lost", 1);
				SetDown(gm, 48, DOWN_AIRFRAME);
				NudgeMorale(gm, RZ_EFaction.MILITARY, -10);
				NudgeMorale(gm, RZ_EFaction.BANDITS, 5);
				if (gm.GetFactionLife())
					gm.GetFactionLife().StartRelation(RZ_EFaction.MILITARY, RZ_EFaction.BANDITS, RZ_FactionLife.REL_FEUD);

				Headline(gm, run.Fill("Bastion's last helicopter burned on its pad by {stronghold} in the night. The Red Hand says it didn't know how to fly it anyway."));
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! He flew for the line: the wreck comes down at the edge of the warning zone near the far LZ,
	//! a Crashed Helicopter world event (with the wreck prefab).
	protected void StartWreck(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_Perimeter perimeter = RZ_PerimeterMissionHelper.Get(gm);
		if (!perimeter || gm.GetHeliWreckPrefab().IsEmpty())
			return;

		vector far = GetPos(run, "far");
		float depth = perimeter.GetKillDistance() + 40;
		array<int> spots = {};
		RZ_PerimeterMissionHelper.FindSpots(perimeter, depth, 10, 500, perimeter.GetWarnDistance() - 30, spots);
		RZ_ShorePoint best;
		float bestDist = float.MAX;
		foreach (int idx : spots)
		{
			RZ_ShorePoint p = perimeter.GetShorePoint(idx);
			float d = RZ_Util.DistXZ(p.m_vEdge, far);
			if (d < bestDist)
			{
				bestDist = d;
				best = p;
			}
		}

		if (!best)
			return;

		RZ_Event_LastPilotWreck evt = new RZ_Event_LastPilotWreck();
		evt.Configure(gm);
		evt.SetSpot(RZ_PerimeterMissionHelper.SpotAt(perimeter, best, depth, 10), run.Fill("{pilot}"));
		gm.GetMissions().StartMission(evt);
	}

	// ================================================================ Words
	//------------------------------------------------------------------------------------------------
	override string JournalText(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.m_iStage != S_LAST)
			return super.JournalText(gm, run);

		switch (run.m_iBranch)
		{
			case BRANCH_STAYS: return run.Fill("{pilot} stays. One more run from the pad by {stronghold}, with the LZ kept clear.");
			case BRANCH_GROUNDED: return run.Fill("{pilot} is grounded. Walk him to {stronghold}. He'll go quietly.");
			case BRANCH_LINE: return run.Fill("{pilot} goes for the line from {far}. Keep the LZ clear while he lifts off.");
			case BRANCH_REDHAND: return run.Fill("A Red Hand crew is coming for the helicopter on its pad by {stronghold}. Keep watch for them for 90 seconds.");
		}

		return super.JournalText(gm, run);
	}

	//------------------------------------------------------------------------------------------------
	override string Rumour(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.m_iStage <= S_WRECK)
			return "\"Bastion's last pilot asks people what the line looks like from the ground. Nobody's sure what answer he wants.\"";

		if (run.m_iStage == S_DECIDE)
			return "\"They say the pilot at {stronghold} has been working out fuel and distances. Not for anywhere inside.\"";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The commanding officer has heard the questions too.
	override string OnLeaderTalk(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, int leaderId)
	{
		if (run.m_iStage != S_WRECK && run.m_iStage != S_DECIDE)
			return "";

		RZ_Leader co = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY);
		if (!co || co.m_iId != leaderId)
			return "";

		return string.Format("%1 glances at the pad. \"My pilot has been asking questions about the line. If you hear anything, I want to know first.\"", co.m_sShort);
	}
}

//------------------------------------------------------------------------------------------------
//! The Last Pilot, stage 1 (Bastion contract, Known; ★ Mechanic): fuel. At the pad, the pilot and
//! the fuel point: pour in two jerrycans (a Mechanic finds the leak: one). Or the Red Hand's fuel,
//! at a depot by their nearest seat: siphon it (sneak: unseen; seen, +8 Red Hand Infamy; either way
//! you carry it back and pour it in), or buy it from the fence there (25 Red Hand Favor: the crew
//! drops it at the pad, and the boss hears Bastion is buying).
class RZ_QM_PilotFuel : RZ_QuestMission
{
	protected ref RZ_QuestCarry m_Fuel = new RZ_QuestCarry();
	protected vector m_vLZ;
	protected vector m_vPilot;
	protected vector m_vPoint;
	protected vector m_vDepot;
	protected vector m_vFence;
	protected bool m_bHasDepot;
	protected bool m_bDepotShown;
	protected int m_iCans;
	protected int m_iCansNeeded = 2;
	protected int m_iSiphoner = -1;
	protected bool m_bSeen;

	protected static const int OBJ_PILOT = 0;
	protected static const int OBJ_POINT = 1;
	protected static const int OBJ_DEPOT = 2;
	protected static const int OBJ_FENCE = 3;
	protected static const int BUY_FAVOR = 25;
	protected static const int SEEN_INFAMY = 8;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 10, 10);
		m_Req.Bonus("mechanic");
		m_fTimeLeft = 3000;
		m_fParticipationRadius = 0;
		m_iLeaderId = RZ_Quest_LastPilot.Voice(gm);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run)
			return false;

		m_vLZ = RZ_Quest_LastPilot.GetPos(run, "lz");
		m_vPilot = LandPointNear(m_vLZ, 4, 8);
		m_vPoint = LandPointNear(m_vLZ, 2, 5);
		m_iSettlement = gm.FindNearestSettlement(m_vLZ);

		RZ_Leader boss = RZ_BastionQuest.Nearest(gm, RZ_EFaction.BANDITS, m_vLZ, 5000);
		if (boss)
		{
			m_bHasDepot = true;
			m_vDepot = LandPointNear(boss.m_vPos, 25, 45);
		}
		else
		{
			string meetName;
			vector meet;
			m_bHasDepot = RZ_LeaderJob.MeetingPoint(gm, RZ_EFaction.BANDITS, m_vLZ, meet, meetName);
			m_vDepot = meet;
		}

		m_vFence = LandPointNear(m_vDepot, 5, 9);
		m_vPos = m_vLZ;
		m_sTitle = "The Last Pilot: fuel";
		m_sBrief = QFill(gm, "The last helicopter is dry. Two jerrycans to the pad by {stronghold}. Don't ask where the Red Hand's diesel comes from. Don't buy it either.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		int talk = AddClue(QText(gm, "pilot"), QFill(gm, "{pilot} is sitting in the open door with his helmet on his knee. \"Dry as a bone. Two cans gets her to the next base and back.\" He looks past you, toward the edge of the Zone. \"Further, if you were careful.\""));
		if (!SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPilot, OBJ_PILOT, "", "", talk, "Talk to " + QText(gm, "pilot")))
			return false;

		return SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vPoint, OBJ_POINT, "Pour in a jerrycan|Pour in the Red Hand's fuel", "", -1, "") != null;
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		result.Insert(m_vPilot);
		result.Insert(m_vPoint);
		if (m_bHasDepot)
		{
			result.Insert(m_vDepot);
			result.Insert(m_vFence);
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = QFill(gm, "The pad by {stronghold}: ") + RZ_Util.DirectionText(from, m_vLZ);
		if (m_bHasDepot)
			text += "\nA Red Hand fuel depot: " + RZ_Util.DirectionText(from, m_vDepot);

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved)
			return;

		if (m_bHasDepot && !m_bDepotShown && RZ_ContractHelp.AnyNear(players, m_vDepot, 300))
		{
			m_bDepotShown = true;
			int depotClue = AddClue("A Red Hand fuel depot", "Drums under a tarp, a hand pump and a padlock nobody bothered to lock. A painted hand on the nearest drum.");
			ResourceName depot = gm.GetFuelDepotPrefab();
			if (depot.IsEmpty())
			{
				SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vDepot, OBJ_DEPOT, "Siphon two cans' worth", "", depotClue, "Look at the drums");
			}
			else
			{
				TrackProp(RZ_Util.SpawnPrefab(depot, RZ_Util.GroundPos(m_vDepot), Math.RandomFloat(0, 360)));
				SpawnObject(gm, RZ_EPuzzleProp.OBJECT, RZ_Util.RandomPointInRing(m_vDepot, 3, 5), OBJ_DEPOT, "Siphon two cans' worth", "", depotClue, "Look at the drums");
			}

			int fenceClue = AddClue("A Red Hand fence", "\"Diesel? For Bastion?\" He laughs. \"Course. Everything's for sale. Even to them.\"");
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vFence, OBJ_FENCE, "", string.Format("Buy the fuel (%1 Red Hand Favor)", BUY_FAVOR), fenceClue, "Talk to the fence");
			m_Quiet.Watch(m_vDepot, 80);
		}

		if (!m_Fuel.Update(gm, this))
			m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx == OBJ_DEPOT)
		{
			Siphon(gm, playerId);
			return;
		}

		if (objIdx != OBJ_POINT || !user)
			return;

		if (value == 1)
		{
			if (!m_Fuel.TakeFrom(user))
			{
				gm.NotifyPlayer(playerId, m_sTitle, "You've none of the Red Hand's fuel on you.");
				return;
			}

			string how = "siphoned the Red Hand's fuel unseen";
			if (m_bSeen)
				how = "took the Red Hand's fuel, and was seen";

			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "Red Hand diesel, cut with something. {pilot} sniffs it and shrugs. \"She's not fussy.\""));
			Resolve(gm, playerId, RZ_EQuestApproach.SNEAK, RZ_Quest_LastPilot.OUT_FUELLED, how);
			return;
		}

		if (RZ_ItemHelper.Take(user, RZ_ContractHelp.FuelWords(gm), 1) <= 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You've no fuel on you. A full jerrycan, from a wreck or a petrol pump.");
			return;
		}

		AddParticipant(playerId);
		m_iCans++;
		if (HasBonus(gm, playerId) && m_iCansNeeded > 1)
		{
			m_iCansNeeded = 1;
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You find where she's been losing it: a weeping union by the pump. Tightened, one can is enough. {pilot} looks impressed."));
		}

		if (m_iCans < m_iCansNeeded)
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("Glug, glug. %1 more.", m_iCansNeeded - m_iCans));
			return;
		}

		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{pilot} runs her up for a minute, just to hear it. \"There she is.\""));
		Resolve(gm, playerId, RZ_EQuestApproach.NONE, RZ_Quest_LastPilot.OUT_FUELLED, "brought the fuel");
	}

	//------------------------------------------------------------------------------------------------
	protected void Siphon(RZ_GameModeComponent gm, int playerId)
	{
		if (m_Fuel.IsSpawned())
		{
			gm.NotifyPlayer(playerId, m_sTitle, "The cans are filled already. Get them to the pad.");
			return;
		}

		if (!m_Fuel.Spawn(gm, m_iId, RZ_Util.RandomPointInRing(m_vDepot, 1, 2)))
			return;

		m_iSiphoner = playerId;
		m_vPos = m_vLZ;
		if (!m_Quiet.IsQuiet(gm, playerId))
		{
			m_bSeen = true;
			string why = m_Quiet.Why(gm, playerId);
			RZ_QuestApproach.SideWith(gm, playerId, -1, RZ_EFaction.BANDITS, 0, SEEN_INFAMY);
			Remember(gm, playerId, RZ_EQuestApproach.SNEAK, "was seen at the Red Hand's fuel");
			gm.NotifyPlayer(playerId, m_sTitle, why + " A shout from the tarp: they've seen you at their drums. Two cans' worth, and a grudge to go with it. Get it to the pad.");
			return;
		}

		Remember(gm, playerId, RZ_EQuestApproach.SNEAK, "siphoned the Red Hand's fuel unseen");
		gm.NotifyPlayer(playerId, m_sTitle, "The pump is quiet if you go slow. Two cans' worth, and nobody looked up. Get it to the pad.");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_FENCE || option != 0)
			return;

		if (RZ_BastionQuest.Rank(gm, playerId, RZ_EFaction.BANDITS) < 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"Not to you.\" He doesn't even get up.");
			return;
		}

		if (!TryBribe(gm, playerId, RZ_EFaction.BANDITS, BUY_FAVOR))
			return;

		SetQFlag(gm, "bought", 1);
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "\"Pleasure.\" Two drums go on a cart toward the pad by {stronghold}. By tonight the boss will know Bastion was buying, and why."));
		Resolve(gm, playerId, RZ_EQuestApproach.BRIBE, RZ_Quest_LastPilot.OUT_FUELLED, "bought the Red Hand's fuel");
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The Last Pilot: the helicopter is fuelled.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Last Pilot: the helicopter is still dry on the pad by {stronghold}.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Fuel.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! A stage at an LZ (The Last Pilot): the helicopter sets down GROUND_TIME before it lifts off,
//! the rotors make heat, a horde comes when it lands, and whoever is within LZ_RADIUS at lift-off
//! kept it clear. Subclasses pick the LZ and say what lift-off means (OnLiftOff).
class RZ_QM_PilotLZ : RZ_QuestMission
{
	protected vector m_vLZ;
	protected vector m_vPilot;
	protected bool m_bLanded;
	protected bool m_bWarned;

	protected static const float COUNTDOWN = 720;
	protected static const float GROUND_TIME = 180;
	protected static const float LZ_RADIUS = 40;

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		result.Insert(m_vLZ);
		result.Insert(m_vPilot);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved)
			return;

		if (!m_bLanded && m_fTimeLeft < GROUND_TIME)
		{
			m_bLanded = true;
			gm.GetSpawner().SpawnHorde(m_vLZ, 300, m_iId);
			NotifyInterested(gm, m_sTitle, "The helicopter is down at the LZ. Three minutes on the ground, and the noise is carrying.");
			OnLanded(gm);
		}

		if (m_bLanded)
			gm.GetHeat().AddHeat(m_vLZ, 4 * dt);

		if (!m_bWarned && m_fTimeLeft < 60)
		{
			m_bWarned = true;
			NotifyInterested(gm, m_sTitle, "One minute to lift-off.");
		}

		if (m_fTimeLeft > 0)
			return;

		array<int> at = {};
		GetPlayersNear(ids, players, m_vLZ, LZ_RADIUS, at);
		if (at.IsEmpty())
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		m_aParticipants.Clear();
		foreach (int playerId : at)
		{
			m_aParticipants.Insert(playerId);
		}

		OnLiftOff(gm, at);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLanded(RZ_GameModeComponent gm)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Players `at` the LZ when it lifted off. Resolve the stage here.
	protected void OnLiftOff(RZ_GameModeComponent gm, array<int> at)
	{
	}
}

//------------------------------------------------------------------------------------------------
//! The Last Pilot, stage 2 (Bastion contract, Known, no storm): whose flight. The LZ by the
//! stronghold, lift-off in 12 minutes. Choose the load at the pilot (the first choice stands):
//! Halcyon's samples (Halcyon Fame +6 for you, research +1), the Haven's wounded (Haven Fame +6,
//! +2 people at the nearest safehouse) or Bastion's ammunition (Bastion Fame +6, Bastion strength
//! +2). Nobody chose: ammunition, as ordered. With the fuel bought from the Red Hand, a crew comes
//! to look (with the Red Hand group prefab).
class RZ_QM_PilotFlight : RZ_QM_PilotLZ
{
	protected ref RZ_UnitWatch m_Crew = new RZ_UnitWatch();
	protected int m_iLoad;
	protected int m_iChooser = -1;
	protected int m_iHouse = -1;

	protected static const int OBJ_PILOT = 0;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 15, 15);
		m_Req.m_fMinMilitary = RZ_Quest_LastPilot.MIN_PRESENCE;
		m_fTimeLeft = COUNTDOWN;
		m_fParticipationRadius = 0;
		m_iLeaderId = RZ_Quest_LastPilot.Voice(gm);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run)
			return false;

		m_vLZ = RZ_Quest_LastPilot.GetPos(run, "lz");
		m_vPilot = LandPointNear(m_vLZ, 4, 8);
		m_iSettlement = gm.FindNearestSettlement(m_vLZ);
		if (gm.GetSafehouses())
			m_iHouse = gm.GetSafehouses().FindNearest(m_vLZ, 8000);

		m_vPos = m_vLZ;
		m_sTitle = "The Last Pilot: whose flight";
		m_sBrief = QFill(gm, "The helicopter flies from the pad by {stronghold} in 12 minutes. Keep the LZ clear. Everyone wants the load: tell {pilot} whose it is.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		int talk = AddClue(QText(gm, "pilot"), QFill(gm, "{pilot} has three manifests on his knee. \"Halcyon wants its samples run to the lab. The Haven has two people who won't walk again unless a doctor sees them. And {co} wants the ammunition moved. One trip.\""));
		string labels = "Load Halcyon's samples|Load the Haven's wounded|Load Bastion's ammunition";
		return SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPilot, OBJ_PILOT, "", labels, talk, "Talk to " + QText(gm, "pilot")) != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnLanded(RZ_GameModeComponent gm)
	{
		if (QFlag(gm, "bought") != 1)
			return;

		// The boss heard Bastion was buying: a crew comes to see what for
		ResourceName crew = gm.GetRedHandGroupPrefab();
		if (crew.IsEmpty() || !m_Crew.Spawn(crew, LandPointNear(m_vLZ, 220, 300)))
			return;

		TrackProp(m_Crew.m_Unit);
		RZ_Util.SendGroupTo(m_Crew.m_Unit, m_vLZ, gm.GetSpawner().GetMoveWaypointPrefab());
		RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, m_Crew.m_Unit, -1, true);
		NotifyInterested(gm, m_sTitle, "A Red Hand crew is coming up the road to the pad. Somebody told them where Bastion's fuel went.");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_PILOT || option < 0 || option > 2)
			return;

		if (m_iLoad > 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "It's loaded. He's not unloading it again.");
			return;
		}

		m_iLoad = option + 1;
		m_iChooser = playerId;
		string what = "chose Bastion's ammunition";
		string text = "Ammunition, as ordered. He doesn't argue.";
		if (m_iLoad == RZ_Quest_LastPilot.LOAD_SAMPLES)
		{
			what = "chose Halcyon's samples";
			text = "Cold boxes with the lab's mark on them go in behind his seat. \"The lab, then.\"";
		}
		else if (m_iLoad == RZ_Quest_LastPilot.LOAD_WOUNDED)
		{
			what = "chose the Haven's wounded";
			text = "Two stretchers go in, and a Haven nurse who won't let go of either. He nods. \"Good.\"";
		}

		Remember(gm, playerId, RZ_EQuestApproach.TALK, what);
		gm.NotifyPlayer(playerId, m_sTitle, text + " Keep the LZ clear until it lifts off.");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnLiftOff(RZ_GameModeComponent gm, array<int> at)
	{
		RZ_QuestRun run = Run(gm);
		if (!run)
			return;

		if (m_iLoad <= 0)
			m_iLoad = RZ_Quest_LastPilot.LOAD_AMMO;

		run.SetFlag("load", m_iLoad);
		RZ_BastionQuest.SetWorld(gm, RZ_Quest_LastPilot.KEY + ".flew_for", RZ_Quest_LastPilot.FlewFor(m_iLoad));
		string text = "It's up and gone, low over the trees, with Bastion's ammunition.";
		switch (m_iLoad)
		{
			case RZ_Quest_LastPilot.LOAD_SAMPLES:
			{
				run.Reward(gm, m_iChooser, RZ_EFaction.INSTITUTE, 6, 0, "Halcyon's people will remember who flew for them.");
				gm.GetCure().AddResearch(1, "Samples flown in on Bastion's last helicopter.");
				text = "It's up and gone, low over the trees, toward the lab.";
				break;
			}
			case RZ_Quest_LastPilot.LOAD_WOUNDED:
			{
				run.Reward(gm, m_iChooser, RZ_EFaction.SURVIVORS, 6, 0, "The Haven heard who got its wounded flown.");
				RZ_Questline.ChangePopulation(gm, m_iHouse, 2);
				text = "It's up and gone, low over the trees, with two stretchers aboard.";
				break;
			}
			default:
			{
				run.Reward(gm, m_iChooser, RZ_EFaction.MILITARY, 6, 0, "Everything is signed for.");
				RZ_Questline.NudgeStrength(gm, RZ_EFaction.MILITARY, 2);
				break;
			}
		}

		NotifyParticipants(gm, m_sTitle, text);
		int who = m_iChooser;
		if (who <= 0)
			who = at[0];

		Resolve(gm, who, RZ_EQuestApproach.TALK, RZ_Quest_LastPilot.OUT_FLOWN, "kept the LZ clear for the flight");
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The Last Pilot: the helicopter flew.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The Last Pilot: it lifted off with nobody holding the LZ, and came back empty.";
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Crew.Delete();
	}
}

//------------------------------------------------------------------------------------------------
//! The Last Pilot, stage 3 (Bastion contract, Known; ★ Mechanic, ★ Radio Op): his friend's wreck,
//! in the warning zone short of the kill zone (the Perimeter's shore points, 500 m from any road
//! checkpoint). Stay within 25 m for 60 s (★ Mechanic: twice as fast) and the black box comes out.
//! ★ Radio Op: play it there; then the pilot hears it when it's handed to him at the pad.
class RZ_QM_PilotWreck : RZ_QuestMission
{
	protected ref RZ_QuestCarry m_Box = new RZ_QuestCarry();
	protected vector m_vWreck;
	protected vector m_vLZ;
	protected vector m_vPilot;
	protected bool m_bWreckShown;
	protected bool m_bPilotShown;
	protected bool m_bPlayed;
	protected float m_fSecured;

	protected static const int OBJ_WRECK = 0;
	protected static const int OBJ_PILOT = 1;
	protected static const float SECURE_SECONDS = 60;
	protected static const float SECURE_RADIUS = 25;
	protected static const int MAX_TRIES = 40;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 15, 15);
		m_Req.Bonus("mechanic");
		m_Req.Bonus("radioham");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
		m_iLeaderId = RZ_Quest_LastPilot.Voice(gm);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		RZ_Perimeter perimeter = RZ_PerimeterMissionHelper.Get(gm);
		if (!run || !perimeter)
			return false;

		// In the warning zone, short of the kill zone
		float depth = perimeter.GetKillDistance() + m_Rng.Range(30, 60);
		if (depth > perimeter.GetWarnDistance() - 30)
			depth = (perimeter.GetKillDistance() + perimeter.GetWarnDistance()) * 0.5;

		array<int> spots = {};
		RZ_PerimeterMissionHelper.FindSpots(perimeter, depth, 10, 500, perimeter.GetWarnDistance() - 30, spots);
		bool found = false;
		foreach (int i, int idx : spots)
		{
			if (i >= MAX_TRIES)
				break;

			RZ_ShorePoint p = perimeter.GetShorePoint(idx);
			m_vWreck = RZ_PerimeterMissionHelper.SpotAt(perimeter, p, depth, 10);
			if (!RZ_Util.IsWater(m_vWreck))
			{
				found = true;
				break;
			}
		}

		if (!found)
			return false;

		m_vLZ = RZ_Quest_LastPilot.GetPos(run, "lz");
		m_vPilot = LandPointNear(m_vLZ, 4, 8);
		m_iSettlement = gm.FindNearestSettlement(m_vWreck);
		m_vPos = m_vWreck;
		m_sTitle = "The Last Pilot: his friend's wreck";
		m_sBrief = QFill(gm, string.Format("{friend}'s helicopter came down short of the line on Day 3, at grid %1, in the warning zone. Bring {pilot} the black box. Stay out of the kill zone: it starts just past the wreck.", RZ_SettlementScanner.GridReference(m_vWreck)));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		result.Insert(m_vWreck);
		result.Insert(m_vPilot);
		if (m_Box.IsOnGround())
			result.Insert(m_Box.GetPos());
	}

	//------------------------------------------------------------------------------------------------
	override string GetLocationName(RZ_GameModeComponent gm)
	{
		return "the quarantine line, grid " + RZ_SettlementScanner.GridReference(m_vWreck);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return string.Format("The wreck, grid %1: %2", RZ_SettlementScanner.GridReference(m_vWreck), RZ_Util.DirectionText(from, m_vWreck)) + QFill(gm, "\n{pilot}, at the pad by {stronghold}: ") + RZ_Util.DirectionText(from, m_vPilot);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved)
			return;

		if (!m_bWreckShown && RZ_ContractHelp.AnyNear(players, m_vWreck, 400))
		{
			m_bWreckShown = true;
			int clue = AddClue("The wreck", QFill(gm, "Burnt aluminium, a rotor blade folded back on itself, and a red cross on the door still readable. {friend}'s name is stencilled under the window. Somewhere in the tail is the black box."));
			ResourceName wreck = gm.GetHeliWreckPrefab();
			IEntity e = RZ_Util.SpawnPrefab(wreck, RZ_Util.GroundPos(m_vWreck), m_Rng.Range(0, 360));
			TrackProp(e);
			if (!e || !TagClue(e, clue, "Look at the wreck"))
				SpawnObject(gm, RZ_EPuzzleProp.OBJECT, RZ_Util.RandomPointInRing(m_vWreck, 2, 3), OBJ_WRECK, "", "", clue, "Look at the wreck");

			SpawnGuards(gm, m_vWreck, 3, 15, 40);
		}

		if (!m_bPilotShown && RZ_ContractHelp.AnyNear(players, m_vPilot, 300))
		{
			m_bPilotShown = true;
			int pilotClue = AddClue(QText(gm, "pilot"), QFill(gm, "\"Did you find it?\" {pilot} doesn't get up. \"He was talking to them all the way down. The wounded. I heard him. I want to hear the rest.\""));
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPilot, OBJ_PILOT, "Give him the black box", "", pilotClue, "Talk to " + QText(gm, "pilot"));
		}

		if (!m_Box.Update(gm, this))
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (m_Box.IsSpawned())
		{
			if (!m_Box.IsOnGround())
				m_vPos = m_vPilot;

			return;
		}

		if (CountPlayersNear(players, m_vWreck, SECURE_RADIUS) == 0)
			return;

		RZ_ContractHelp.AddNear(ids, players, m_vWreck, SECURE_RADIUS, m_aParticipants);
		float rate = 1;
		array<int> near = {};
		GetPlayersNear(ids, players, m_vWreck, SECURE_RADIUS, near);
		foreach (int playerId : near)
		{
			if (RZ_QuestApproach.HasBackground(gm, playerId, "mechanic"))
				rate = 2;
		}

		m_fSecured += dt * rate;
		if (m_fSecured < SECURE_SECONDS)
			return;

		if (!m_Box.Spawn(gm, m_iId, RZ_Util.RandomPointInRing(m_vWreck, 2, 4)))
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		string text = QFill(gm, "You get the tail panel off and the black box out. It's lighter than you'd think. Take it to {pilot}.");
		foreach (int listener : near)
		{
			if (RZ_Radio.IsRadioOperator(gm, listener))
			{
				m_bPlayed = true;
				SetQFlag(gm, "played", 1);
				gm.NotifyPlayer(listener, m_sTitle, QFill(gm, "You patch the box into your set and play the last minute. Tracer, close. A voice on the line's frequency: \"Turn back. Turn back.\" Then {friend}, very calm, talking to the wounded behind him the whole way down. Then nothing."));
				Remember(gm, listener, RZ_EQuestApproach.TALK, "played the black box at the wreck");
			}
		}

		RZ_ContractHelp.NotifyNear(gm, m_vWreck, 60, m_sTitle, text);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx != OBJ_PILOT)
			return;

		if (!m_Box.TakeFrom(user))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You haven't got the black box.");
			return;
		}

		if (m_bPlayed)
		{
			SetQFlag(gm, "heard", 1);
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You play it for him on the operator's set, the last minute only. He listens to all of it with his eyes shut: the line telling {friend} to turn back, and firing anyway. \"They knew it was a medevac,\" he says. \"They knew.\""));
		}
		else
		{
			gm.NotifyPlayer(playerId, m_sTitle, "He turns it over in his hands, and doesn't play it. It goes in his locker. \"Later.\"");
		}

		Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_LastPilot.OUT_BOX, "brought him the black box");
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Last Pilot: {pilot} has the black box.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Last Pilot: {friend}'s black box is still in the wreck.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Box.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! The Last Pilot, stage 4 (Bastion contract, Known): he decides. At the pad: talk him down
//! (persuade: Trusted, or ★ Soldier; Known if he's heard the black box), or help him (two cans to a
//! far LZ near the line, outside the warning zone: betray; Bastion pays nothing). At the stronghold:
//! report him to the duty officer. At the nearest Red Hand seat: sell the plan to a fixer (Trusted
//! with the Red Hand: +5 Red Hand Fame, 40 Red Hand Favor, +10 Bastion Infamy; no pay).
class RZ_QM_PilotDecide : RZ_QuestMission
{
	protected vector m_vLZ;
	protected vector m_vPilot;
	protected vector m_vOfficer;
	protected vector m_vFixer;
	protected vector m_vFar;
	protected bool m_bHasFixer;
	protected bool m_bHasFar;
	protected bool m_bPadShown;
	protected bool m_bOfficerShown;
	protected bool m_bFixerShown;
	protected bool m_bFarShown;
	protected bool m_bHelping;
	protected int m_iCans;
	protected int m_iCansNeeded = 2;
	protected int m_iChosen = -1;
	protected string m_sFixer;

	protected static const int OBJ_PILOT = 0;
	protected static const int OBJ_OFFICER = 1;
	protected static const int OBJ_FIXER = 2;
	protected static const int OBJ_FAR = 3;
	protected static const int SELL_FAVOR = 40;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 10, 10);
		m_Req.Bonus("soldier");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
		m_iLeaderId = RZ_Quest_LastPilot.Voice(gm);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		RZ_Leader co = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY);
		if (!run || !co)
			return false;

		m_vLZ = RZ_Quest_LastPilot.GetPos(run, "lz");
		m_vPilot = LandPointNear(m_vLZ, 4, 8);
		m_vOfficer = LandPointNear(co.m_vPos, 6, 12);
		m_iSettlement = gm.FindNearestSettlement(m_vLZ);

		RZ_Leader boss = RZ_BastionQuest.Nearest(gm, RZ_EFaction.BANDITS, m_vLZ, 6000);
		if (boss)
		{
			m_bHasFixer = true;
			m_vFixer = LandPointNear(boss.m_vPos, 6, 12);
			m_sFixer = boss.m_sShort + "'s fixer";
		}
		else
		{
			string meetName;
			vector meet;
			m_bHasFixer = RZ_LeaderJob.MeetingPoint(gm, RZ_EFaction.BANDITS, m_vLZ, meet, meetName);
			m_vFixer = meet;
			m_sFixer = "a Red Hand fixer";
		}

		// The far LZ: near the line, but outside the warning zone, as far from the pad as can be found
		RZ_Perimeter perimeter = RZ_PerimeterMissionHelper.Get(gm);
		if (perimeter)
		{
			float depth = perimeter.GetWarnDistance() + 200;
			array<int> spots = {};
			RZ_PerimeterMissionHelper.FindSpots(perimeter, depth, 0, 600, depth + 200, spots);
			float bestDist = 1200;
			foreach (int i, int idx : spots)
			{
				if (i >= 60)
					break;

				vector spot = RZ_PerimeterMissionHelper.SpotAt(perimeter, perimeter.GetShorePoint(idx), depth, 0);
				float d = RZ_Util.DistXZ(spot, m_vLZ);
				if (d > bestDist && !RZ_Util.IsWater(spot) && !perimeter.IsNearLine(spot))
				{
					bestDist = d;
					m_vFar = spot;
					m_bHasFar = true;
				}
			}
		}

		m_vPos = m_vPilot;
		m_sTitle = "The Last Pilot: he decides";
		m_sBrief = QFill(gm, "{pilot} has been working out fuel and distances, and none of them are to anywhere inside the Zone. Go and talk to him. I want to know what he means to do.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		result.Insert(m_vPilot);
		result.Insert(m_vOfficer);
		if (m_bHasFixer)
			result.Insert(m_vFixer);

		if (m_bHelping)
			result.Insert(m_vFar);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = QFill(gm, "{pilot}, at the pad by {stronghold}: ") + RZ_Util.DirectionText(from, m_vPilot);
		text += "\nThe duty officer, for the commanding officer: " + RZ_Util.DirectionText(from, m_vOfficer);
		if (m_bHasFixer)
			text += "\n" + RZ_Util.Capitalize(m_sFixer) + " (the Red Hand): " + RZ_Util.DirectionText(from, m_vFixer);

		if (m_bHelping)
			text += string.Format("\nThe far LZ (grid %1): %2", RZ_SettlementScanner.GridReference(m_vFar), RZ_Util.DirectionText(from, m_vFar));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved)
			return;

		if (!m_bPadShown && RZ_ContractHelp.AnyNear(players, m_vPilot, 300))
		{
			m_bPadShown = true;
			string talk = "{pilot} has a map on his knee, the edge of the Zone in pencil. \"Fuel for an hour, from a field near the line. I'd go low. {friend} went high.\" He folds it. \"Somebody has to try again. Or somebody has to stop me.\"";
			if (QFlag(gm, "heard") == 1)
				talk = "{pilot} has a map on his knee, and the black box beside it. \"You heard them. Turn back, and then they fired. On a medevac.\" He folds the map. \"Somebody has to try again. Or somebody has to stop me.\"";

			int clue = AddClue(QText(gm, "pilot"), QFill(gm, talk));
			string labels = "Talk him down";
			if (m_bHasFar)
				labels += "|Help him: fuel for a far LZ";

			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPilot, OBJ_PILOT, "", labels, clue, "Talk to " + QText(gm, "pilot"));
		}

		if (!m_bOfficerShown && RZ_ContractHelp.AnyNear(players, m_vOfficer, 300))
		{
			m_bOfficerShown = true;
			int officerClue = AddClue("The duty officer", QFill(gm, "\"If it's about {pilot}, {co} wants to hear it. Today.\""));
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vOfficer, OBJ_OFFICER, "", "Report the pilot's plan", officerClue, "Talk to the duty officer");
		}

		if (m_bHasFixer && !m_bFixerShown && RZ_ContractHelp.AnyNear(players, m_vFixer, 300))
		{
			m_bFixerShown = true;
			int fixerClue = AddClue(RZ_Util.Capitalize(m_sFixer), "\"A helicopter? Can't fly one. Don't need to.\" He grins. \"Things that are worth a lot to Bastion are worth something to us. Even on fire.\"");
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vFixer, OBJ_FIXER, "", string.Format("Sell the helicopter's plan (Trusted with the Red Hand; %1 Favor)", SELL_FAVOR), fixerClue, "Talk to " + m_sFixer);
		}

		if (m_bHelping && !m_bFarShown && RZ_ContractHelp.AnyNear(players, m_vFar, 300))
		{
			m_bFarShown = true;
			int farClue = AddClue("A field near the line", "Flat, firm, and out of sight of the drones. A good place to fuel a helicopter nobody's supposed to know about.");
			SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vFar, OBJ_FAR, "Leave a jerrycan here", "", farClue, "Look around");
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (m_bHelping && objIdx != OBJ_FAR)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "It's decided. The fuel is going to the far LZ.");
			return;
		}

		if (objIdx == OBJ_PILOT)
		{
			if (option == 0)
				TalkDown(gm, playerId);
			else if (option == 1 && m_bHasFar)
				Help(gm, playerId);

			return;
		}

		if (objIdx == OBJ_OFFICER && option == 0)
		{
			m_iChosen = OBJ_OFFICER;
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "The duty officer writes it down word for word and takes it straight in to {co}. Within the hour, the board by {pilot}'s name says \"Grounded\"."));
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_LastPilot.OUT_GROUNDED, "reported him to the commanding officer");
			return;
		}

		if (objIdx != OBJ_FIXER || option != 0)
			return;

		if (RZ_BastionQuest.Rank(gm, playerId, RZ_EFaction.BANDITS) < 2)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"You? Sell me something about Bastion? I'd have to trust you first.\"");
			return;
		}

		m_iChosen = OBJ_FIXER;
		RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.BANDITS, RZ_EFaction.MILITARY, 5, 10);
		RZ_QuestRun run = Run(gm);
		if (run)
			run.Reward(gm, playerId, RZ_EFaction.BANDITS, 0, SELL_FAVOR, "The crew pays well for a quiet night at Bastion's pad.");

		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You tell him when the pad by {stronghold} is quiet, and who's on the gate. He writes nothing down. \"We'll want you there. Keep watch for us.\""));
		Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_LastPilot.OUT_REDHAND, "sold the helicopter's plan to the Red Hand", false);
	}

	//------------------------------------------------------------------------------------------------
	protected void TalkDown(RZ_GameModeComponent gm, int playerId)
	{
		int difficulty = 2;
		if (QFlag(gm, "heard") == 1)
			difficulty = 1;

		if (!TryPersuade(gm, playerId, RZ_EFaction.MILITARY, difficulty, "soldier"))
		{
			Remember(gm, playerId, RZ_EQuestApproach.PERSUADE, "tried to talk him down");
			gm.NotifyPlayer(playerId, m_sTitle, "He hears you out. \"You weren't there on Day 3.\" He goes back to the map.");
			return;
		}

		m_iChosen = OBJ_PILOT;
		string text = "You talk for a long time. About the people on the roster, about who flies the wounded next time. In the end he puts the map in the stove himself.";
		if (QFlag(gm, "heard") == 1)
			text = "You say it plainly: they told {friend} to turn back, and fired anyway, and they'll do it again. He's quiet for a long time. Then he puts the map in the stove himself.";

		RZ_QuestRun run = Run(gm);
		if (run)
			run.Reward(gm, playerId, RZ_EFaction.MILITARY, 10, 30, "Bastion keeps its last pilot.");

		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, text + " \"One more run, then. With the LZ clear. So they know I meant it.\""));
		Resolve(gm, playerId, RZ_EQuestApproach.PERSUADE, RZ_Quest_LastPilot.OUT_STAYS, "talked him down");
	}

	//------------------------------------------------------------------------------------------------
	protected void Help(RZ_GameModeComponent gm, int playerId)
	{
		m_bHelping = true;
		m_vPos = m_vFar;
		RZ_QuestRun run = Run(gm);
		if (run)
		{
			RZ_Quest_LastPilot.SetPos(run, "far", m_vFar);
			run.SetText("far", "the field at grid " + RZ_SettlementScanner.GridReference(m_vFar));
		}

		Remember(gm, playerId, RZ_EQuestApproach.BETRAY, "agreed to fuel his flight for the line");
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("He marks a field on your map, near the line, out of sight of the drones: grid %1. \"Two cans. Leave them there. I'll do the rest.\"", RZ_SettlementScanner.GridReference(m_vFar)));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx != OBJ_FAR || !m_bHelping || !user)
			return;

		if (RZ_ItemHelper.Take(user, RZ_ContractHelp.FuelWords(gm), 1) <= 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You've no fuel on you. A full jerrycan, from a wreck or a petrol pump.");
			return;
		}

		AddParticipant(playerId);
		m_iCans++;
		if (RZ_QuestApproach.HasBackground(gm, playerId, "mechanic"))
			m_iCansNeeded = 1;

		if (m_iCans < m_iCansNeeded)
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("One can under the hedge. %1 more.", m_iCansNeeded - m_iCans));
			return;
		}

		m_iChosen = OBJ_FAR;
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "The cans are hidden under the hedge. {pilot} will come for them at the next chance. Somebody will have to keep this field clear while he lifts off."));
		Resolve(gm, playerId, RZ_EQuestApproach.BETRAY, RZ_Quest_LastPilot.OUT_LINE, "fuelled his flight for the line", false);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Last Pilot: {pilot} has decided.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (m_iChosen == OBJ_FIXER)
			return "The Last Pilot: the Red Hand knows when Bastion's pad is quiet. Bastion won't thank you.";

		if (m_iChosen == OBJ_FAR)
			return QFill(gm, "The Last Pilot: {pilot} has his fuel. Bastion won't thank you.");

		return QFill(gm, "The Last Pilot: nobody talked to {pilot}.");
	}
}

//------------------------------------------------------------------------------------------------
//! The Last Pilot, stage 5 for "he stays" and "the line" (Bastion contract, Known, no storm): one
//! more run from the pad (paid), or the far LZ near the line while he lifts off for it (no pay).
class RZ_QM_PilotRun : RZ_QM_PilotLZ
{
	protected bool m_bLine;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 15, 15);
		m_Req.m_fMinMilitary = RZ_Quest_LastPilot.MIN_PRESENCE;
		m_fTimeLeft = COUNTDOWN;
		m_fParticipationRadius = 0;
		m_iLeaderId = RZ_Quest_LastPilot.Voice(gm);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run)
			return false;

		m_bLine = run.m_iBranch == RZ_Quest_LastPilot.BRANCH_LINE;
		m_vLZ = RZ_Quest_LastPilot.GetPos(run, "lz");
		if (m_bLine)
			m_vLZ = RZ_Quest_LastPilot.GetPos(run, "far");

		m_vPilot = LandPointNear(m_vLZ, 4, 8);
		m_iSettlement = gm.FindNearestSettlement(m_vLZ);
		m_vPos = m_vLZ;
		if (m_bLine)
		{
			m_sTitle = "The Last Pilot: the far LZ";
			m_sBrief = QFill(gm, "{pilot} is coming to {far} for the fuel you left him, and then he's going. Keep the field clear while he lifts off. Nobody at {stronghold} knows.");
			return true;
		}

		m_sTitle = "The Last Pilot: one more run";
		m_sBrief = QFill(gm, "{pilot} flies one more run from the pad by {stronghold}, to show he meant it. Keep the LZ clear until he lifts off.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnLiftOff(RZ_GameModeComponent gm, array<int> at)
	{
		if (m_bLine)
		{
			NotifyParticipants(gm, m_sTitle, QFill(gm, "He lifts off low and turns toward the edge of the Zone without looking back. You hear the drones change their note, and then the guns."));
			Resolve(gm, at[0], RZ_EQuestApproach.BETRAY, RZ_Quest_LastPilot.OUT_LIFTED, "kept the field clear while he flew for the line", false);
			return;
		}

		NotifyParticipants(gm, m_sTitle, QFill(gm, "He lifts off on time and comes back on time. {pilot} climbs out, hands the logbook to the duty officer, and goes to eat."));
		Resolve(gm, at[0], RZ_EQuestApproach.TALK, RZ_Quest_LastPilot.OUT_RUN, "kept the LZ clear for his last run");
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Last Pilot: {pilot} flew his run.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (m_bLine && m_bResolved)
			return QFill(gm, "The Last Pilot: {pilot} flew for the line. Bastion paid nothing for that.");

		if (m_bLine)
			return QFill(gm, "The Last Pilot: nobody kept {far} clear, and {pilot} couldn't get off the ground.");

		return "The Last Pilot: nobody kept the LZ for his last run.";
	}
}

//------------------------------------------------------------------------------------------------
//! The Last Pilot, stage 5 for "grounded" (Bastion contract, Known): walk him from the pad to the
//! stronghold. He goes quietly (an escort following whoever talks to him first).
class RZ_QM_PilotWalk : RZ_QuestMission
{
	protected ref RZ_Escort m_Him = new RZ_Escort();
	protected vector m_vLZ;
	protected vector m_vDest;
	protected bool m_bWalking;

	protected static const int OBJ_HIM = 0;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 10, 10);
		m_fTimeLeft = 2400;
		m_fParticipationRadius = 0;
		m_iLeaderId = RZ_Quest_LastPilot.Voice(gm);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		RZ_Leader co = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY);
		if (!run || !co || !RZ_Escort.HasRoom(gm))
			return false;

		m_vLZ = RZ_Quest_LastPilot.GetPos(run, "lz");
		m_vDest = co.m_vPos;
		m_iSettlement = gm.FindNearestSettlement(m_vLZ);
		int talk = AddClue(QText(gm, "pilot"), QFill(gm, "{pilot} has his helmet in his hand and nothing else. \"I know. {co} told me.\" He stands up. \"Let's get it done.\""));
		m_Him.m_sName = QText(gm, "pilot");
		m_Him.m_sPrefab = RZ_Escort.PrefabFor(gm, 2);
		m_Him.m_bHide = false;
		m_Him.SetTalk(m_iId, OBJ_HIM, QFill(gm, "Walk with him to {stronghold}"), talk, "Talk to " + QText(gm, "pilot"));
		m_vPos = m_vLZ;
		m_sTitle = "The Last Pilot: grounded";
		m_sBrief = QFill(gm, "{pilot} is grounded. Walk him from the pad to {stronghold}. He'll go quietly. Make sure he gets there.");
		return m_Him.Begin(gm, LandPointNear(m_vLZ, 4, 8));
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		result.Insert(m_vLZ);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved)
			return;

		m_Him.Update(gm, dt, ids, players);
		m_vPos = m_Him.m_vPos;

		// Nobody can talk to his body: stand beside him and he comes anyway
		if (!m_bWalking)
		{
			int beside = RZ_EscortHelp.PlayerBeside(ids, players, m_Him.m_vPos, 4);
			if (beside > 0)
				OnChoice(gm, beside, OBJ_HIM, 0, null);

			return;
		}

		if (m_Him.IsDead())
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		RZ_ContractHelp.AddNear(ids, players, m_vPos, 100, m_aParticipants);
		if (m_Him.IsNear(m_vDest, 30))
		{
			m_Him.Arrive();
			NotifyParticipants(gm, m_sTitle, QFill(gm, "At the gate he hands his helmet to the duty officer, salutes nobody in particular, and goes in."));
			Resolve(gm, m_Him.GetFollowed(), RZ_EQuestApproach.TALK, RZ_Quest_LastPilot.OUT_WALKED, "walked him to the stronghold");
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_HIM || option != 0 || m_bWalking)
			return;

		m_bWalking = true;
		m_Him.Follow(playerId);
		m_Him.Go();
		AddParticipant(playerId);
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "He falls in beside you. Walk him to {stronghold}."));
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Last Pilot: {pilot} walked to {stronghold} on his own.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Him.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! The Last Pilot, stage 5 for the Red Hand (Red Hand contract, Known): keep watch at the pad 90 s
//! while a crew takes the helicopter. With the Red Hand group prefab the crew is real (it walks in
//! from 200-300 m and spares Red Hand friends). They can't fly it, so they burn it.
class RZ_QM_PilotBurn : RZ_QuestMission
{
	protected ref RZ_UnitWatch m_Crew = new RZ_UnitWatch();
	protected vector m_vLZ;
	protected bool m_bShown;
	protected float m_fWatched;
	protected int m_iWatcher = -1;

	protected static const int OBJ_PAD = 0;
	protected static const float WATCH_SECONDS = 90;
	protected static const float WATCH_RADIUS = 25;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 10, 10);
		m_fTimeLeft = 2400;
		m_fParticipationRadius = 0;
		RZ_Leader boss = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.BANDITS);
		if (boss)
			m_iLeaderId = boss.m_iId;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run)
			return false;

		m_vLZ = RZ_Quest_LastPilot.GetPos(run, "lz");
		m_iSettlement = gm.FindNearestSettlement(m_vLZ);
		m_vPos = m_vLZ;
		m_sTitle = "The Last Pilot: the pad";
		m_sBrief = QFill(gm, "Bastion's helicopter sits on its pad by {stronghold}, and tonight the gate is thin. Keep watch while the boys do the rest.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		int clue = AddClue("The pad", "Bastion's last helicopter, tied down, with a tarp over the engine. Nobody on the gate is looking this way.");
		if (!SpawnObject(gm, RZ_EPuzzleProp.OBJECT, LandPointNear(m_vLZ, 2, 5), OBJ_PAD, "", "Keep watch for the crew", clue, "Look at the pad"))
			return false;

		ResourceName crew = gm.GetRedHandGroupPrefab();
		if (!crew.IsEmpty() && m_Crew.Spawn(crew, LandPointNear(m_vLZ, 200, 300)))
		{
			TrackProp(m_Crew.m_Unit);
			RZ_Util.SendGroupTo(m_Crew.m_Unit, m_vLZ, gm.GetSpawner().GetMoveWaypointPrefab());
			RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, m_Crew.m_Unit, -1, true);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved || m_iWatcher <= 0)
			return;

		// A crew that's dead can't burn anything
		if (m_Crew.IsDead(dt))
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		IEntity watcher = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iWatcher);
		if (watcher && RZ_Util.DistXZ(watcher.GetOrigin(), m_vLZ) <= WATCH_RADIUS)
			m_fWatched += dt;

		if (m_fWatched < WATCH_SECONDS)
			return;

		ResourceName fire = gm.GetExplosionPrefab();
		if (!fire.IsEmpty())
			RZ_Util.SpawnPrefab(fire, RZ_Util.GroundPos(m_vLZ));

		gm.GetHeat().AddHeat(m_vLZ, 60);
		NotifyParticipants(gm, m_sTitle, "They pull the tarp off and stand around the cockpit for a while. Nobody can fly it. In the end somebody pours fuel on the seats, and the whole crew runs.");
		Resolve(gm, m_iWatcher, RZ_EQuestApproach.SIDE, RZ_Quest_LastPilot.OUT_BURNT, "kept watch while the Red Hand burned the helicopter");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_PAD || option != 0 || m_iWatcher > 0)
			return;

		if (RZ_BastionQuest.Rank(gm, playerId, RZ_EFaction.BANDITS) < 1)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "The crew doesn't know you. If you stand at the pad, you'll be the first thing they shoot.");
			return;
		}

		m_iWatcher = playerId;
		Remember(gm, playerId, RZ_EQuestApproach.SIDE, "kept watch for the Red Hand at the pad");
		gm.NotifyPlayer(playerId, m_sTitle, "You take the corner with the best view of the gate. Stay by the pad while they work.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The Last Pilot: Bastion's helicopter is burning on its pad.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The Last Pilot: the crew never got to the helicopter.";
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Crew.Delete();
	}
}

//------------------------------------------------------------------------------------------------
//! The Last Pilot's "He flew for the line": the Crashed Helicopter event at the edge of the warning
//! zone, near where he went. Never picked at random (not registered); the questline starts it. As
//! Crashed Helicopter: the wreck and a crate (spawned once a player is within 400 m), infected
//! round it, a horde when someone reaches it.
class RZ_Event_LastPilotWreck : RZ_Mission
{
	protected vector m_vWreck;
	protected string m_sPilot;
	protected bool m_bShown;

	//------------------------------------------------------------------------------------------------
	void SetSpot(vector pos, string pilot)
	{
		m_vWreck = pos;
		m_sPilot = pilot;
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 1800;
		m_fParticipationRadius = 40;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		if (m_vWreck == vector.Zero || gm.GetHeliWreckPrefab().IsEmpty())
			return false;

		m_vPos = m_vWreck;
		m_iSettlement = gm.FindNearestSettlement(m_vWreck);
		m_sTitle = "Crashed Helicopter";
		m_sBrief = string.Format("Bastion's last helicopter came down short of the line at grid %1. %2 was flying it. Whatever was aboard is still there. The infected got there first.", RZ_SettlementScanner.GridReference(m_vWreck), m_sPilot);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetLocationName(RZ_GameModeComponent gm)
	{
		return "the quarantine line, grid " + RZ_SettlementScanner.GridReference(m_vWreck);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bShown && RZ_ContractHelp.AnyNear(players, m_vWreck, 400))
		{
			m_bShown = true;
			IEntity wreck = RZ_Util.SpawnPrefab(gm.GetHeliWreckPrefab(), RZ_Util.GroundPos(m_vWreck), Math.RandomFloat(0, 360));

			// No lasting traces: once nobody has been near it for a while, it's cleared (RZ_LooseItems)
			RZ_GameModeComponent.TrackLooseItem(wreck);
			gm.SpawnRewardCrate(RZ_Util.RandomPointInRing(m_vWreck, 6, 10));
			SpawnGuards(gm, m_vWreck, 4, 15, 45);
			gm.GetHeat().AddHeat(m_vWreck, 50);
		}

		if (!m_bShown || CountPlayersNear(players, m_vWreck, 30) == 0)
			return;

		gm.GetSpawner().SpawnHorde(m_vWreck, 300);
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "Someone reached the last helicopter's wreck at the line. So did the infected.";
	}
}
