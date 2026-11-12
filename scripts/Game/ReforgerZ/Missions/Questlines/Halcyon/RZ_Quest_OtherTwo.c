//! The Other Two (Halcyon; HC2 in docs/QUESTLINES.md, built in docs/MISSIONS.md, "Questlines").
//! Eight in ten walk out of the trial dose, and the lab writes down what happens to the other two.
//! A Haven woman's brother volunteered and didn't come back. The answer is a back room at a field
//! station, where the other two are kept so someone with a clipboard can be there when they turn.
//! Its stages are the sister's (Haven) contracts, offered to anyone Known with the Haven or Halcyon.
//!
//! 1. His number (mission, at the lab): ask the trial clerk (Known with Halcyon; Known with the
//!    Haven once H3 opened the lab road), read the volunteers' ledger on the night shift unseen
//!    (seen: +8 Halcyon Infamy; a Lab Tech reads its codes) or pay a courier (20 Halcyon Favor).
//! 2. The back room (mission, at the field station): walk in with Halcyon's courier and his cold
//!    case (RZ_Escort), ask the field lead (Trusted, or Known as a Doctor), or slip in at night
//!    unseen (a Lab Tech who read the codes, any time).
//! 3. Her brother (mission): he has until the end of the Final stage. Sit with him, walk him home
//!    to his sister before he turns (RZ_Escort; a Nurse buys him longer), wait with the sample kit
//!    for the moment of turning (research +1) or end it (armed).
//! 4. The field lead (mission): ask her to close the room (Trusted, or Known as a Doctor), report
//!    her to the director, say nothing (25 Halcyon Favor) or sell one of the other two to the Red
//!    Hand (Trusted with them: 40 Red Hand Favor, and the questline ends there).
//! 5. Names (mission, at the sister's safehouse): the paper has the house's other volunteer as a
//!    number; the trial list by the door and the man who walked them up give the name (folded in
//!    from The Author List contract; an Elder reads the council's pencil). Then the names to the
//!    director (Trusted, or Known with the sister beside you after he came home), to the Haven's
//!    chair, or let them stay numbers.
//!
//! Endings: Named, Brought home, The work goes on, Sold. With H3's knows_other_two it starts at the
//! back room. Needs a safehouse with people, a Halcyon field station, the note and a civilian.
class RZ_Quest_OtherTwo : RZ_Questline
{
	static const string KEY = "other_two";
	static const string FLAG_NUMBERS = "other_two.numbers";		// set: the lab kept numbers (H3's headline gives numbers)
	static const string FLAG_NAMED = "other_two.named";			// set: the lab put names on its paper
	static const string FLAG_HAVEN_NAMES = "other_two.haven_names";	// set: the Haven read the names, the lab didn't
	static const string FLAG_WARD = "other_two.ward";				// set: the back room: 1 closed, 2 moved to the lab, 3 kept, 4 sold from, 5 gone
	static const string FLAG_SOLD = "other_two.sold";				// set: one of the other two went to the Red Hand's pit
	// Read from H3 (The Edge of Town, the Haven): the names H3's author publishes; see docs/QUESTLINES.md, "Crossings"
	static const string FLAG_H3_KNOWS = "walk.knows_other_two";	// starts at the back room
	static const string FLAG_H3_LAB_ROAD = "walk.lab_road";		// the clerk talks to the Haven's people

	static const int S_START = 0;
	static const int S_NUMBER = 1;
	static const int S_ROOM = 2;
	static const int S_BROTHER = 3;
	static const int S_LEAD = 4;
	static const int S_NAMES = 5;

	static const int OUT_LEARNED = 10;
	static const int OUT_IN = 11;
	static const int OUT_SAT = 12;
	static const int OUT_HOME = 13;
	static const int OUT_SAMPLE = 14;
	static const int OUT_ENDED = 15;
	static const int OUT_TURNED = 16;
	static const int OUT_CLOSED = 17;
	static const int OUT_REPORTED = 18;
	static const int OUT_KEPT = 19;
	static const int OUT_SOLD = 20;
	static const int OUT_GONE = 21;
	static const int OUT_NAMED = 22;
	static const int OUT_TO_HAVEN = 23;
	static const int OUT_NUMBERS = 24;

	// "brother": what happened to him
	static const int BRO_SAT = 1;
	static const int BRO_HOME = 2;
	static const int BRO_SAMPLE = 3;
	static const int BRO_ENDED = 4;
	static const int BRO_TURNED = 5;
	static const int BRO_UNKNOWN = 6;

	// "ward": the back room
	static const int WARD_CLOSED = 1;
	static const int WARD_LAB = 2;
	static const int WARD_KEPT = 3;
	static const int WARD_SOLD = 4;
	static const int WARD_GONE = 5;

	static const int E_NAMED = 0;
	static const int E_HOME = 1;
	static const int E_NUMBERS = 2;
	static const int E_SOLD = 3;

	static const float WALK_HOME_RANGE = 3500;		// the sister's house, from the field station, when one is this close

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		Info(KEY, "The Other Two", RZ_EFaction.INSTITUTE);

		RZ_QuestStage start = WaitStage(S_START, "His number", "{sister} at the safehouse in {town} has a brother, {brother}, who volunteered for the trial dose and didn't come back. The lab won't tell her more than a number.");
		start.Wait(0);

		RZ_QuestStage number = MissionStage(S_NUMBER, "His number", "{sister} at the safehouse in {town} has a brother, {brother}, who volunteered for the trial dose and didn't come back. The lab won't tell her more than a number. Find out his, at the lab.");
		number.GiveUp(4, "Nobody ever told {sister} what happened to {brother}.");
		number.Visits(2);

		RZ_QuestStage room = MissionStage(S_ROOM, "The back room", "{brother} is Volunteer {number}, {day}. The lab's code next to his number is F2: a back room at the field station in {lead_place}. Get in.");
		room.GiveUp(4, "Nobody got into the back room at {lead_place}. Whatever is in there stays a code in a ledger.");
		room.Visits(2);

		RZ_QuestStage brother = MissionStage(S_BROTHER, "Her brother", "Inside the back room at {lead_place}: three people in the Final stage, and a chair with a clipboard by each. {brother} is in the bed by the window. He hasn't long.");
		brother.GiveUp(2, "Nobody went back into the room for {brother}. The clipboard got its last line.");

		RZ_QuestStage lead = MissionStage(S_LEAD, "The field lead", "{lead} runs the back room at {lead_place}. Somebody has to talk to her about it, or about her.");
		lead.GiveUp(4, "Nobody said anything to {lead} about the back room. It's still there.");
		lead.Visits(2);

		RZ_QuestStage names = MissionStage(S_NAMES, "Names", "The lab's paper lists the volunteers by number: {count} of them who didn't walk out. {brother} is one. The safehouse in {town} knows who the others from there were.");
		names.GiveUp(4, "Nobody put names to the numbers. The paper stays the way the lab wrote it.");
		names.Visits(2);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		// Once the cure is out, nobody is volunteering for the trial dose any more
		if (!gm.HasLab() || gm.GetCure().IsCured() || !gm.GetLeaders() || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN) || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.OBJECT))
			return false;

		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses || houses.Count() == 0)
			return false;

		RZ_Leader lead = RZ_QM_HalcyonScene.PickFieldLead(gm, true);
		if (!lead)
			return false;

		// Her house: one with people, within a walk of the field station if there is one (he has to
		// be walked home from it before he turns), else the nearest
		array<int> options = {};
		int nearest = -1;
		float nearestDist = float.MAX;
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse cand = houses.Get(i);
			if (!cand || cand.m_iPopulation < 3 || cand.m_iSettlement < 0)
				continue;

			float dist = RZ_Util.DistXZ(cand.m_vPos, lead.m_vPos);
			if (dist <= WALK_HOME_RANGE)
				options.Insert(i);

			if (dist < nearestDist)
			{
				nearestDist = dist;
				nearest = i;
			}
		}

		if (options.IsEmpty() && nearest >= 0)
			options.Insert(nearest);

		if (options.IsEmpty())
			return false;

		int idx = options.GetRandomElement();
		RZ_Safehouse h = houses.Get(idx);
		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		string surname = RZ_NoteText.Surname(rng);
		array<string> firsts = {};
		RZ_NoteText.DistinctFirstNames(rng, 2, firsts);
		if (firsts.Count() < 2)
			return false;

		int today = 12;
		if (gm.GetBroadcast())
			today = Math.Max(8, gm.GetBroadcast().GetDay());

		run.SetFlag("house", idx);
		run.SetText("town", h.m_sName);
		run.SetText("sister", firsts[0] + " " + surname);
		run.SetText("brother", firsts[1]);
		run.SetText("brother_full", firsts[1] + " " + surname);
		int num = rng.Int(11, 90);
		int day = rng.Int(6, today);
		run.SetFlag("num", num);
		run.SetFlag("day", day);
		run.SetText("number", num.ToString());
		run.SetText("day", RZ_NoteText.Day(day));
		run.SetFlag("seed", rng.Int(1, 2000000000));
		run.SetText("count", CountWord(rng.Int(11, 18)));
		RZ_QM_HalcyonScene.StoreLeader(run, "lead", lead);
		RZ_QM_HalcyonScene.StorePos(run, "station", lead.m_vPos);
		run.SetText("director", RZ_QM_HalcyonScene.TopName(gm, RZ_EFaction.INSTITUTE, "the director"));
		run.SetPlace(h.m_vPos, h.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static string CountWord(int n)
	{
		array<string> words = {"eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen"};
		int i = Math.ClampInt(n - 11, 0, words.Count() - 1);
		return words[i];
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		Headline(gm, run.Fill("{sister} at the safehouse in {town} walks up to the lab every morning and asks for her brother by name. They give her a number."), false);
	}

	//------------------------------------------------------------------------------------------------
	//! The sister's house has to have people in it; the brother's stages need the field station.
	override bool IsReady(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (!RZ_QM_HalcyonScene.HouseLives(gm, run.Flag("house", -1)))
			return false;

		if (stage == S_ROOM || stage == S_BROTHER)
			return RZ_QM_HalcyonScene.SameLeader(gm, run, "lead") != null;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override RZ_Mission MakeMission(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		switch (stage)
		{
			case S_NUMBER: return new RZ_QM_OtherNumber();
			case S_ROOM: return new RZ_QM_OtherRoom();
			case S_BROTHER: return new RZ_QM_OtherBrother();
			case S_LEAD: return new RZ_QM_OtherLead();
			case S_NAMES: return new RZ_QM_OtherNames();
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
				// H3 found out about the other two already: straight to the back room
				if (gm.GetQuestlines().WorldFlag(FLAG_H3_KNOWS) == 1 && RZ_QM_HalcyonScene.SameLeader(gm, run, "lead"))
				{
					run.Note("Whoever looked into the walks at the edge of town already found the lab's numbers. {brother} is Volunteer {number}, {day}, and the code next to him is F2: the back room at {lead_place}.");
					return S_ROOM;
				}

				return S_NUMBER;
			}
			case S_NUMBER:
			{
				if (outcome == OUTCOME_FAILED)
				{
					run.Note("Nobody got {brother}'s number out of the lab.");
					return S_NUMBER;
				}

				run.Note("{brother} is Volunteer {number}, {day}. Next to the number, the lab's code: F2. The other two go to F2.");
				return ToRoom(gm, run);
			}
			case S_ROOM:
			{
				if (outcome == OUTCOME_FAILED)
				{
					run.Note("Nobody got into the back room.");
					return S_ROOM;
				}

				run.Note("Inside the back room at {lead_place}: three beds, three people in the Final stage, and a chair with a clipboard by each. {brother} is in the bed by the window.");
				run.SetPlace(RZ_QM_HalcyonScene.LoadPos(run, "station"), run.Text("lead_place"));
				return S_BROTHER;
			}
			case S_BROTHER:
				return AfterBrother(gm, run, outcome);
			case S_LEAD:
				return AfterLead(gm, run, outcome);
			case S_NAMES:
				return AfterNames(run, outcome);
		}

		return Fizzle(run, "");
	}

	//------------------------------------------------------------------------------------------------
	//! The back room is at the field station: if she's gone (dead, replaced, the station lost), so is
	//! the room, and he's a number on the lab's paper.
	protected int ToRoom(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (RZ_QM_HalcyonScene.SameLeader(gm, run, "lead"))
			return S_ROOM;

		run.SetFlag("brother", BRO_UNKNOWN);
		run.SetFlag("ward", WARD_GONE);
		run.Note("The field station at {lead_place} isn't {lead}'s any more, and its back room went with the field lead. {brother} is a number on the lab's paper, and nobody was there at the end.");
		run.SetPlace(GetHousePos(gm, run), run.Text("town"));
		return S_NAMES;
	}

	//------------------------------------------------------------------------------------------------
	protected static vector GetHousePos(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_Safehouse h = GetHouse(gm, run.Flag("house", -1));
		if (!h)
			return run.m_vPlace;

		return h.m_vPos;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterBrother(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		switch (outcome)
		{
			case OUT_SAT:
			{
				run.SetFlag("brother", BRO_SAT);
				run.Note("Somebody sat with {brother} until the end, and held his hand instead of the clipboard.");
				break;
			}
			case OUT_HOME:
			{
				run.SetFlag("brother", BRO_HOME);
				run.Note("{brother} made it home to {sister} before he turned. She was with him at the end.");
				break;
			}
			case OUT_SAMPLE:
			{
				run.SetFlag("brother", BRO_SAMPLE);
				run.Note("At the moment of turning, the sample was taken. The clipboard has its last line.");
				break;
			}
			case OUT_ENDED:
			{
				run.SetFlag("brother", BRO_ENDED);
				run.Note("Somebody ended it for {brother} before the clipboard could.");
				break;
			}
			default:
			{
				run.SetFlag("brother", BRO_TURNED);
				run.Note("{brother} turned in the back room with nobody beside him but the clipboard.");
				break;
			}
		}

		if (RZ_QM_HalcyonScene.SameLeader(gm, run, "lead"))
			return S_LEAD;

		run.SetFlag("ward", WARD_GONE);
		run.Note("{lead} isn't at {lead_place} any more, and the back room went too.");
		run.SetPlace(GetHousePos(gm, run), run.Text("town"));
		return S_NAMES;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterLead(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		switch (outcome)
		{
			case OUT_SOLD:
			{
				run.SetFlag("ward", WARD_SOLD);
				return Finish(run, E_SOLD, "Sold. One of the other two went to a Red Hand crew as a live one. The back room is one bed emptier, and the pit has something new in it.");
			}
			case OUT_REPORTED:
			{
				run.SetFlag("ward", WARD_LAB);
				run.Note("The director knows about the back room. It moves to the lab, under the director's eye, and {lead} loses the field station.");
				break;
			}
			case OUT_KEPT:
			{
				run.SetFlag("ward", WARD_KEPT);
				run.Note("Nobody says anything about the back room. {lead} is grateful, in Favor.");
				break;
			}
			case OUT_GONE:
			{
				run.SetFlag("ward", WARD_GONE);
				run.Note("{lead} is gone from {lead_place}, and so is the back room.");
				break;
			}
			case OUTCOME_FAILED:
			{
				run.Note("Nobody talked to {lead} about the back room.");
				return S_LEAD;
			}
			default:
			{
				run.SetFlag("ward", WARD_CLOSED);
				run.Note("{lead} closed the back room. The chairs went back to the waiting room.");
				break;
			}
		}

		run.SetPlace(GetHousePos(gm, run), run.Text("town"));
		return S_NAMES;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterNames(RZ_QuestRun run, int outcome)
	{
		if (outcome == OUT_TO_HAVEN)
			return Finish(run, E_HOME, "Brought home. The Haven has the names, and read every one at supper. The lab still has its numbers.");

		if (outcome == OUT_NUMBERS || outcome == OUTCOME_FAILED)
			return Finish(run, E_NUMBERS, "The work goes on. The volunteers stay numbers on the lab's paper, and the lab is grateful for the quiet.");

		return Finish(run, E_NAMED, "Named. The lab wrote {count} names over its numbers, in ink. {brother_full} is one of them.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
		RZ_Questlines q = gm.GetQuestlines();
		int ward = run.Flag("ward", WARD_CLOSED);
		q.SetWorldFlag(FLAG_WARD, ward);
		if (ward == WARD_LAB)
			MoveRoom(gm, run);

		switch (ending)
		{
			case E_NAMED:
			{
				q.SetWorldFlag(FLAG_NAMED, 1);
				ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 8, 0);
				ChangeStanding(gm, run, RZ_EFaction.INSTITUTE, 8, 0);
				NudgeMorale(gm, RZ_EFaction.SURVIVORS, 5);
				Headline(gm, run.Fill("The lab put {count} names on its paper last night. The Haven read them at supper."));
				if (gm.GetFactionLife())
				{
					gm.GetFactionLife().StartRelation(RZ_EFaction.SURVIVORS, RZ_EFaction.INSTITUTE, RZ_FactionLife.REL_TRUCE);
					if (gm.GetFactionLife().HasTruce(RZ_EFaction.SURVIVORS, RZ_EFaction.INSTITUTE))
						SpreadRumour(gm, "The Haven and the lab are talking again. A list of names will do that.", 6);
				}

				break;
			}
			case E_HOME:
			{
				q.SetWorldFlag(FLAG_HAVEN_NAMES, 1);
				ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 10, 0);
				NudgeMorale(gm, RZ_EFaction.INSTITUTE, -3);
				Headline(gm, run.Fill("The Haven read {count} names at supper last night, the lab's volunteers among them. " + RoomLine(ward)));
				break;
			}
			case E_NUMBERS:
			{
				q.SetWorldFlag(FLAG_NUMBERS, 1);
				ChangeStanding(gm, run, RZ_EFaction.INSTITUTE, 5, 0);
				NudgeMorale(gm, RZ_EFaction.SURVIVORS, -3);
				gm.GetCure().AddResearch(2, "The trial notes are complete. Every volunteer, by number.");
				Headline(gm, run.Fill("The lab's paper still lists its volunteers by number. {count} of them so far."), false);
				break;
			}
			case E_SOLD:
			{
				q.SetWorldFlag(FLAG_SOLD, 1);
				Headline(gm, "There's something new in the Red Hand's pit this week. The crew says it used to be a volunteer.");
				SoldFoundOut(gm, run);
				break;
			}
		}

		SpreadRumour(gm, "Eight in ten walk out. Ask the lab what the other two are for.", 8);
	}

	//------------------------------------------------------------------------------------------------
	//! Reported: the director replaces the field lead and takes the back room into the lab.
	protected void MoveRoom(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_Leader l = RZ_QM_HalcyonScene.SameLeader(gm, run, "lead");
		if (l)
			ReplaceLeader(gm, l.m_iId, run.Fill("{director} has replaced {lead} at the field station in {lead_place}. Its back room is at the lab now, where the director can watch it."));
	}

	//------------------------------------------------------------------------------------------------
	//! Sold: 35% the Haven finds out who sold them (Infamy for whoever took the Red Hand's side: the
	//! line ends at the sale, so nobody else has sided with anyone in it).
	protected void SoldFoundOut(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (Math.RandomFloat01() >= 0.35)
			return;

		ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 0, MAX_INFAMY, RZ_EQuestApproach.SIDE);
		TellParticipants(gm, run, "The Haven", run.Fill("{sister} heard who sold one of the other two to the Red Hand. So has every safehouse."));
	}

	//------------------------------------------------------------------------------------------------
	protected static string RoomLine(int ward)
	{
		if (ward == WARD_CLOSED)
			return "The back room they came from is closed.";

		if (ward == WARD_LAB)
			return "The back room they came from is at the lab now.";

		if (ward == WARD_GONE)
			return "The back room they came from is gone.";

		return "The back room they came from is still there.";
	}

	//------------------------------------------------------------------------------------------------
	override string JournalText(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (!RZ_QM_HalcyonScene.HouseLives(gm, run.Flag("house", -1)))
			return run.Fill("The safehouse in {town} is empty now. Nobody is waiting to hear about {brother}.");

		if ((run.m_iStage == S_ROOM || run.m_iStage == S_BROTHER) && !RZ_QM_HalcyonScene.SameLeader(gm, run, "lead"))
			return run.Fill("The field station at {lead_place} isn't {lead}'s any more. Whatever was in its back room has gone with her.");

		return super.JournalText(gm, run);
	}

	//------------------------------------------------------------------------------------------------
	override string Rumour(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.m_iStage >= S_ROOM && run.m_iStage <= S_LEAD)
			return "Halcyon couriers carry cold cases out to {lead_place} and come back empty-handed. Nothing lives out there that needs a cold case.";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The sister's council head has a word while it runs: about him until his end is known, then
	//! about the names.
	override string OnLeaderTalk(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, int leaderId)
	{
		RZ_Leader l = gm.GetLeaders().Get(leaderId);
		if (!l || l.m_iSafehouse != run.Flag("house", -1))
			return "";

		if (run.Flag("brother") == 0)
			return string.Format("%1 nods toward the door. \"{sister} still walks up the hill every morning. If you're going up there, find out what they did with {brother}.\"", l.m_sShort);

		return string.Format("%1 keeps their voice down. \"{sister} has the lab's paper now. She wants every number on it to have a name, not just his.\"", l.m_sShort);
	}
}

//------------------------------------------------------------------------------------------------
//! The Other Two's stage missions: the sister's (Haven) contracts, for anyone Known with the Haven
//! or with Halcyon and Hunted by neither.
class RZ_QM_OtherTwoScene : RZ_QM_HalcyonScene
{
	protected int m_iLead = -1;		// the field lead, held while the stage needs her stand-in

	//------------------------------------------------------------------------------------------------
	protected void SetSister(int fame, int favor, float seconds)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, fame, favor);
		m_fTimeLeft = seconds;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsEligible(RZ_GameModeComponent gm, int playerId)
	{
		if (!super.IsEligible(gm, playerId) || RZ_Reputation.IsHunted(gm.GetTier(playerId, RZ_EFaction.INSTITUTE)))
			return false;

		return Rank(gm, playerId, RZ_EFaction.SURVIVORS) >= 1 || Rank(gm, playerId, RZ_EFaction.INSTITUTE) >= 1;
	}

	//------------------------------------------------------------------------------------------------
	//! The field lead's stand-in at her station, held while the stage runs (-1 if she's gone).
	protected int HoldLead(RZ_GameModeComponent gm)
	{
		// Not while another mission has her (its own stand-in is out there)
		RZ_Leader l = SameLeader(gm, Run(gm), "lead");
		if (!l || l.m_iHeldBy >= 0 || !gm.GetLeaders())
			return -1;

		m_iLead = l.m_iId;
		gm.GetLeaders().Hold(m_iLead, m_iId);
		return m_iLead;
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
}

//------------------------------------------------------------------------------------------------
//! The Other Two, stage 1 (Haven contract): his number, at the lab. The trial clerk (talk), the
//! volunteers' ledger in the records room (sneak, night shift only) or a courier (bribe).
class RZ_QM_OtherNumber : RZ_QM_OtherTwoScene
{
	protected static const int OBJ_CLERK = 0;
	protected static const int OBJ_LEDGER = 1;
	protected static const int OBJ_COURIER = 2;
	protected static const int COURIER_FAVOR = 20;
	protected static const int SEEN_INFAMY = 8;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetSister(8, 10, 3600);
		m_Req.Bonus("labtech");
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		if (!Run(gm))
			return false;

		vector lab = gm.GetLabPosition();
		m_vPos = LandPointNear(lab, 6, 12);
		m_iSettlement = gm.GetLabSettlement();
		int clerk = AddClue("The trial clerk", QFill(gm, "The trial clerk keeps the volunteers' book on a chain. \"Names are confidential. Numbers are for staff, and for friends of the lab.\""));
		AddSpot(m_vPos, RZ_EPuzzleProp.CIVILIAN, OBJ_CLERK, clerk, "Talk to the trial clerk", QFill(gm, "Ask for {brother_full}'s number"), "the trial clerk at the lab");

		vector records = LandPointNear(lab, 15, 30);
		int ledger = AddClue("The records room", "A ledger on a shelf behind a door that's locked in the day and staffed until the lights go dim. At night the generator goes to the freezers.");
		AddSpot(records, RZ_EPuzzleProp.OBJECT, OBJ_LEDGER, ledger, "Look at the records room", "", "the records room at the lab", "Read the volunteers' ledger");
		m_Quiet.Watch(records, 80);

		int courier = AddClue("A courier", "A Halcyon courier with an empty steel case, smoking where he's not supposed to. \"I carry the books up and down. I don't read them. For a price I could.\"");
		AddSpot(LandPointNear(lab, 20, 35), RZ_EPuzzleProp.CIVILIAN, OBJ_COURIER, courier, "Talk to the courier", string.Format("Pay him to look (%1 Halcyon Favor)", COURIER_FAVOR), "a courier by the lab gate");

		m_sTitle = "The Other Two: his number";
		m_sBrief = QFill(gm, "My brother {brother} went up to the lab for the trial dose on {day} and didn't come back. They'll give me a number and nothing else. Find out his number. Then I'll know where to start.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (option != 0 || !MayAct(gm, playerId))
			return;

		if (objIdx == OBJ_CLERK)
		{
			// Staff and friends of the lab; with H3's lab road, the Haven's people too
			bool road = gm.GetQuestlines().WorldFlag(RZ_Quest_OtherTwo.FLAG_H3_LAB_ROAD) == 1 && Rank(gm, playerId, RZ_EFaction.SURVIVORS) >= 1;
			if (Rank(gm, playerId, RZ_EFaction.INSTITUTE) < 1 && !road)
			{
				Remember(gm, playerId, RZ_EQuestApproach.TALK, "asked the clerk and was refused");
				gm.NotifyPlayer(playerId, m_sTitle, "\"I'm sorry. Friends of the lab.\" The clerk turns the book face down.");
				return;
			}

			Learned(gm, playerId, "The clerk runs a finger down the column. \"Volunteer {number}, {day}.\" A pause. \"There's a code next to it. F2. I don't know what it means.\" The clerk knows exactly what it means.", RZ_EQuestApproach.TALK, "asked the trial clerk");
			return;
		}

		if (objIdx == OBJ_COURIER && TryBribe(gm, playerId, RZ_EFaction.INSTITUTE, COURIER_FAVOR))
			Learned(gm, playerId, "He comes back with it written on his cigarette packet. \"{number}, {day}. And F2. Don't say where you got it.\"", RZ_EQuestApproach.BRIBE, "paid a courier");
	}

	//------------------------------------------------------------------------------------------------
	//! The ledger: only on the night shift, only unseen. Seen: the lab hears.
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx != OBJ_LEDGER || !MayAct(gm, playerId))
			return;

		if (!RZ_GameTime.IsNight())
		{
			gm.NotifyPlayer(playerId, m_sTitle, "The records room is staffed in the day. Come back on the night shift, when the lights go dim.");
			return;
		}

		if (!m_Quiet.IsQuiet(gm, playerId))
		{
			string why = m_Quiet.Why(gm, playerId);
			m_Quiet.Spot(playerId);
			RZ_QuestApproach.SideWith(gm, playerId, -1, RZ_EFaction.INSTITUTE, 0, SEEN_INFAMY);
			Remember(gm, playerId, RZ_EQuestApproach.SNEAK, "was caught in the records room");
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 The night guard has a torch in your face before you've found the right page. (+%2 Halcyon Infamy)", why, SEEN_INFAMY));
			return;
		}

		string text = "Volunteer {number}, {day}: the dose, then the column for \"walked out\", empty. Next to it, in red: F2.";
		if (HasBonus(gm, playerId))
		{
			SetQFlag(gm, "knows_room", 1);
			text += " You know the lab's codes: F2 is Final-stage observation, at the field station in {lead_place}. And you know which door they use.";
		}

		Learned(gm, playerId, text, RZ_EQuestApproach.SNEAK, "read the ledger on the night shift");
	}

	//------------------------------------------------------------------------------------------------
	protected void Learned(RZ_GameModeComponent gm, int playerId, string text, int approach, string what)
	{
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, text));
		Resolve(gm, playerId, approach, RZ_Quest_OtherTwo.OUT_LEARNED, what);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Other Two: nobody got {brother}'s number.");
	}
}

//------------------------------------------------------------------------------------------------
//! The Other Two, stage 2 (Haven contract): the back room. Walk in with Halcyon's courier (he
//! follows you from the lab: RZ_Escort; without room for one, he hands you his case), ask the field
//! lead (persuade), or slip in at the back unseen at night (a Lab Tech who read the codes: any time).
class RZ_QM_OtherRoom : RZ_QM_OtherTwoScene
{
	protected ref RZ_Escort m_Courier = new RZ_Escort();
	protected bool m_bEscort;
	protected vector m_vStation;
	protected vector m_vDoor;
	protected int m_iCaseSpot = -1;

	protected static const int OBJ_LEAD = 0;
	protected static const int OBJ_DOOR = 1;
	protected static const int OBJ_COURIER = 2;
	protected static const int SEEN_INFAMY = 5;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetSister(10, 10, 3600);
		m_Req.Bonus("doctor");
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run || HoldLead(gm) < 0)
			return false;

		RZ_Leader l = gm.GetLeaders().Get(m_iLead);
		m_vStation = l.m_vPos;
		m_vPos = NearSeat(m_vStation);
		m_vDoor = LandPointNear(m_vStation, 15, 25);
		m_iSettlement = gm.FindNearestSettlement(m_vStation);
		int talk = AddClue(l.m_sName, string.Format("%1 doesn't pretend not to know what you want. \"F2 is a protocol, not a secret. It's also not a place for visitors.\"", l.m_sShort));
		AddSpot(m_vPos, RZ_EPuzzleProp.CIVILIAN, OBJ_LEAD, talk, "Talk to " + l.m_sShort, "Ask to see the back room", string.Format("%1 at the field station in %2", l.m_sShort, l.m_sPlace));
		int door = AddClue("The back door", "A side door with a new lock and an old step, worn down by the same few pairs of boots. A cold-case trolley is parked by it.");
		AddSpot(m_vDoor, RZ_EPuzzleProp.OBJECT, OBJ_DOOR, door, "Look at the side door", "", "the side door at the field station", "Slip inside");
		m_Quiet.Watch(m_vStation, 70);

		// Halcyon's courier, at the lab with a cold case for F2
		vector lab = LandPointNear(gm.GetLabPosition(), 8, 15);
		int courier = AddClue("The courier", "A Halcyon courier with a steel cold case for F2. \"Two handles. Nobody asks who's holding the other one.\"");
		m_Courier.m_sName = "the courier";
		m_Courier.m_sPrefab = RZ_Escort.PrefabFor(gm, 1);
		m_Courier.SetTalk(m_iId, OBJ_COURIER, "Take the other handle (he follows you)", courier, "Talk to the courier");
		m_bEscort = m_Courier.Begin(gm, lab);
		if (!m_bEscort)
			m_iCaseSpot = AddSpot(lab, RZ_EPuzzleProp.CIVILIAN, OBJ_COURIER, courier, "Talk to the courier", "Carry his cold case to F2", "the courier at the lab");

		m_sItemName = "the cold case";
		m_sTitle = "The Other Two: the back room";
		m_sBrief = QFill(gm, "F2 is a back room at the field station in {lead_place}. {brother} is in it, or was. Get in: carry a cold case in with the lab's courier, talk {lead_short} into it, or go in the side door at night when nobody's watching.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnScene(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bEscort)
			return;

		m_Courier.Update(gm, dt, ids, players);
		if (m_Courier.IsDead())
		{
			m_bEscort = false;
			NotifyInterested(gm, m_sTitle, "The courier didn't make it. His case is lying in the road. There are other ways in.");
			return;
		}

		if (m_Courier.GetFollowed() > 0 && m_Courier.IsNear(m_vStation, 30))
		{
			m_Courier.Arrive();
			In(gm, m_Courier.GetFollowed(), "You carry the case in together. Nobody at the station looks at the second pair of hands. The courier nods at a door at the back, and leaves you there.", RZ_EQuestApproach.TALK, "came in with the courier");
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (option != 0 || !MayAct(gm, playerId))
			return;

		if (objIdx == OBJ_COURIER)
		{
			if (m_bEscort)
			{
				m_Courier.Follow(playerId);
				m_Courier.Go();
				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "He hands you one side of the case. \"F2. Don't drop it, and don't open it.\" The station is " + RZ_Util.DirectionText(m_Courier.m_vPos, m_vStation) + "."));
				return;
			}

			if (HasHolder())
				return;

			SetHolder(playerId);
			SetSpotOn(m_iCaseSpot, false);
			gm.NotifyPlayer(playerId, m_sTitle, "\"You take it, then. F2, at the field station. Ask for the side door.\" The case is colder than the morning.");
			return;
		}

		if (objIdx != OBJ_LEAD)
			return;

		if (TryPersuade(gm, playerId, RZ_EFaction.INSTITUTE, 2, "doctor"))
		{
			In(gm, playerId, QFill(gm, "{lead_short} looks at you for a long time, then takes a key off a belt loop. \"You'll want to have seen it, then. Don't touch the clipboards.\""), RZ_EQuestApproach.PERSUADE, "talked the field lead into it");
			return;
		}

		Remember(gm, playerId, RZ_EQuestApproach.PERSUADE, "asked the field lead and was refused");
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "\"No.\" {lead_short} says it kindly. (She lets in people the lab trusts, and doctors it knows.)"));
	}

	//------------------------------------------------------------------------------------------------
	//! The side door: the case's carrier walks in; anyone else only at night, unseen (a Lab Tech who
	//! read the codes knows the door, any time).
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx != OBJ_DOOR || !MayAct(gm, playerId))
			return;

		if (IsHolder(playerId))
		{
			In(gm, playerId, "You knock with the cold case in both hands. The door opens before you've finished, and closes behind you.", RZ_EQuestApproach.TALK, "carried the cold case in");
			return;
		}

		if (!RZ_GameTime.IsNight() && QFlag(gm, "knows_room") != 1)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "Somebody is always on that step in the day. At night the station sleeps.");
			return;
		}

		if (!m_Quiet.IsQuiet(gm, playerId))
		{
			string why = m_Quiet.Why(gm, playerId);
			m_Quiet.Spot(playerId);
			RZ_QuestApproach.SideWith(gm, playerId, -1, RZ_EFaction.INSTITUTE, 0, SEEN_INFAMY);
			Remember(gm, playerId, RZ_EQuestApproach.SNEAK, "was seen at the side door");
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 A light comes on inside. Whoever is on watch knows your face now. (+%2 Halcyon Infamy)", why, SEEN_INFAMY));
			return;
		}

		In(gm, playerId, "The lock is new and the frame is old. You're inside before anyone stirs.", RZ_EQuestApproach.SNEAK, "slipped in at the side door");
	}

	//------------------------------------------------------------------------------------------------
	protected void In(RZ_GameModeComponent gm, int playerId, string text, int approach, string what)
	{
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, text + " Inside: three beds, three people in the Final stage, and a chair with a clipboard beside each one. {brother} is in the bed by the window."));
		Resolve(gm, playerId, approach, RZ_Quest_OtherTwo.OUT_IN, what);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnHolderLost(RZ_GameModeComponent gm)
	{
		SetSpotOn(m_iCaseSpot, true);
		super.OnHolderLost(gm);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = super.GetWhereText(gm, from);
		if (m_bEscort)
			text += "\nThe courier: " + RZ_Util.DirectionText(from, m_Courier.m_vPos);

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The Other Two: nobody got into the back room.";
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Courier.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! The Other Two, stage 3 (Haven contract): her brother, until the end of the Final stage (long
//! enough to walk him home, from when the first player reaches the room). Sit with him (90 s
//! beside him), walk him home (RZ_Escort to the sister's safehouse; a Nurse buys half as long again),
//! wait with the kit for the turning (90 s beside him: research +1, and he gets up) or end it.
class RZ_QM_OtherBrother : RZ_QM_OtherTwoScene
{
	protected ref RZ_Escort m_Walk = new RZ_Escort();
	protected bool m_bWalking;
	protected vector m_vBed;
	protected vector m_vHome;
	protected float m_fLeft = -1;
	protected float m_fBeside;
	protected int m_iSitter = -1;
	protected int m_iWatch;			// 1 sitting with him, 2 waiting with the kit
	protected int m_iBedSpot = -1;

	protected static const int OBJ_BROTHER = 0;
	protected static const int WATCH_SIT = 1;
	protected static const int WATCH_KIT = 2;
	protected static const float BESIDE_SECONDS = 90;
	protected static const float SAMPLE_RESEARCH = 1;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetSister(10, 15, 5400);
		m_Req.Bonus("nurse");
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		RZ_Safehouse h;
		if (run)
			h = RZ_Questline.GetHouse(gm, run.Flag("house", -1));

		if (!run || !h)
			return false;

		vector station = LoadPos(run, "station");
		m_vHome = h.m_vPos;
		m_vBed = LandPointNear(station, 12, 20);
		m_vPos = m_vBed;
		m_iSettlement = gm.FindNearestSettlement(station);
		int talk = AddClue(QText(gm, "brother"), QFill(gm, "{brother} knows his own name, and yours once you've said it. \"Is {sister} all right? Tell her it wasn't her fault. I volunteered.\" His hands are cold and his eyes keep going to the chair."));
		string choices = QFill(gm, "Sit with him|Walk him home to {sister}|Wait with the sample kit|End it");
		m_iBedSpot = AddSpot(m_vBed, RZ_EPuzzleProp.CIVILIAN, OBJ_BROTHER, talk, QFill(gm, "Talk to {brother}"), choices, QFill(gm, "{brother}, in the back room at {lead_place}"));
		int sheet = AddClue("The clipboard", QFill(gm, "Volunteer {number}. Temperature and pulse every fifteen minutes in a neat hand, and the times getting closer together. At the bottom, a line waiting: \"Moment of turning: ____\". The pen is tied to the chair with string."));
		AddSpot(LandPointNear(m_vBed, 1.5, 2.5), RZ_EPuzzleProp.CLUE, -1, sheet, "Read the clipboard", "", "");
		int second = AddClue("The second bed", "A woman in the second bed is counting the ceiling tiles out loud. She has got to forty-one, and she starts again.");
		AddSpot(LandPointNear(m_vBed, 3, 5), RZ_EPuzzleProp.CIVILIAN, -1, second, "Look at the second bed", "", "");
		int third = AddClue("The third bed", "The man in the third bed is asleep, or nearly. The chair beside him has a clipboard on it and nobody in it.");
		AddSpot(LandPointNear(m_vBed, 3, 5), RZ_EPuzzleProp.CIVILIAN, -1, third, "Look at the third bed", "", "");

		m_Walk.m_sName = QText(gm, "brother");
		m_Walk.m_sPrefab = RZ_Escort.PrefabFor(gm, 0);
		m_sTitle = "The Other Two: her brother";
		m_sBrief = QFill(gm, "{brother} is in the back room at {lead_place}, in the Final stage. He hasn't long. Whatever you do for him, do it before the clipboard does.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Long enough to walk home at a steady pace (an hour at most), from when the first player
	//! reaches the room.
	protected float TimeFor(RZ_GameModeComponent gm, int playerId)
	{
		float walk = RZ_Util.DistXZ(m_vBed, m_vHome) / 1.1;
		return Math.Clamp(walk * 1.3 + 240, 600, 3600);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnScene(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_fLeft < 0)
		{
			if (RZ_Util.NearestDistance(m_vBed, players) > 60)
				return;

			m_fLeft = TimeFor(gm, -1);
			NotifyInterested(gm, m_sTitle, QFill(gm, string.Format("The clipboard says {brother} has about %1 minutes.", RZ_Util.RoundToInt(m_fLeft / 60))));
		}

		m_fLeft -= dt;
		if (m_bWalking)
		{
			Walking(gm, dt, ids, players);
			return;
		}

		if (m_iWatch != 0)
			Watching(gm, dt, ids, players);

		if (!m_bResolved && m_fLeft <= 0)
			Turns(gm, m_vBed, ids, players, "{brother} turns in the bed by the window, with nobody beside him but the clipboard.");
	}

	//------------------------------------------------------------------------------------------------
	protected void Walking(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_Walk.Update(gm, dt, ids, players);
		m_vPos = m_Walk.m_vPos;
		if (m_Walk.IsDead())
		{
			int near = RZ_EscortHelp.PlayerBeside(ids, players, m_vPos, 300);
			NotifyInterested(gm, m_sTitle, QFill(gm, "{brother} went down on the road, and didn't get up again."));
			Resolve(gm, near, RZ_EQuestApproach.NONE, RZ_Quest_OtherTwo.OUT_TURNED, "was with him on the road", false);
			return;
		}

		RZ_ContractHelp.AddNear(ids, players, m_vPos, 100, m_aParticipants);
		if (m_Walk.IsNear(m_vHome, 25))
		{
			m_Walk.Arrive();
			int walker = m_Walk.GetFollowed();
			gm.NotifyPlayer(walker, m_sTitle, QFill(gm, "{sister} is at the gate before you are. She takes him inside and shuts the door, and the safehouse goes very quiet. It's a while before anyone comes out."));
			Resolve(gm, walker, RZ_EQuestApproach.TALK, RZ_Quest_OtherTwo.OUT_HOME, "walked him home to his sister");
			return;
		}

		if (m_fLeft <= 0)
			Turns(gm, m_vPos, ids, players, "{brother} stops on the road and sits down, and when he gets up it isn't him. He didn't make it home.");
	}

	//------------------------------------------------------------------------------------------------
	//! Sitting with him, or waiting with the kit: the one who chose stays beside him for 90 s.
	protected void Watching(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		// Gone (dead, off the server): someone else can choose
		IEntity sitter = PlayerEntity(m_iSitter);
		if (!sitter || !ids.Contains(m_iSitter))
		{
			m_iWatch = 0;
			m_iSitter = -1;
			m_fBeside = 0;
			return;
		}

		if (RZ_Util.DistXZ(sitter.GetOrigin(), m_vBed) > 5)
		{
			m_fBeside = 0;
			return;
		}

		m_fBeside += dt;
		if (m_fBeside < BESIDE_SECONDS)
			return;

		if (m_iWatch == WATCH_SIT)
		{
			gm.NotifyPlayer(m_iSitter, m_sTitle, QFill(gm, "He talks about {sister} for a while, and then about nothing, and then he stops. You're holding his hand, not the clipboard."));
			Resolve(gm, m_iSitter, RZ_EQuestApproach.TALK, RZ_Quest_OtherTwo.OUT_SAT, "sat with him to the end");
			return;
		}

		gm.GetCure().AddResearch(SAMPLE_RESEARCH, "A sample was taken at the moment of turning, as the protocol asks.");
		Remember(gm, m_iSitter, RZ_EQuestApproach.BETRAY, "took the sample at the moment of turning");
		gm.NotifyPlayer(m_iSitter, m_sTitle, "You fill in the line: the time, to the minute. Then you take the sample, the way the protocol says. Then he gets up.");
		Turns(gm, m_vBed, ids, players, "");
		Resolve(gm, m_iSitter, RZ_EQuestApproach.BETRAY, RZ_Quest_OtherTwo.OUT_SAMPLE, "took the sample at the moment of turning");
	}

	//------------------------------------------------------------------------------------------------
	//! He turns: an infected where he is (only with someone near to see it).
	protected void Turns(RZ_GameModeComponent gm, vector at, array<int> ids, array<IEntity> players, string text)
	{
		SetSpotOn(m_iBedSpot, false);
		m_Walk.Cleanup();
		if (RZ_Util.NearestDistance(at, players) <= 300)
			gm.GetSpawner().SpawnZombie(LandPointNear(at, 0.5, 1.5), false, m_iSettlement, -1);

		if (text.IsEmpty())
			return;

		NotifyInterested(gm, m_sTitle, QFill(gm, text));
		int near = RZ_EscortHelp.PlayerBeside(ids, players, at, 300);
		Resolve(gm, near, RZ_EQuestApproach.NONE, RZ_Quest_OtherTwo.OUT_TURNED, "was there when he turned", false);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_BROTHER || m_bWalking || m_iWatch != 0 || !MayAct(gm, playerId))
			return;

		if (m_fLeft < 0)
			m_fLeft = TimeFor(gm, playerId);

		switch (option)
		{
			case 0:
			{
				m_iWatch = WATCH_SIT;
				m_iSitter = playerId;
				gm.NotifyPlayer(playerId, m_sTitle, "You move the clipboard off the chair and sit down. Stay with him.");
				return;
			}
			case 1:
			{
				WalkHome(gm, playerId);
				return;
			}
			case 2:
			{
				m_iWatch = WATCH_KIT;
				m_iSitter = playerId;
				gm.NotifyPlayer(playerId, m_sTitle, "The kit is under the chair: tubes, labels, a pen on a string. Stay close. The protocol wants it at the moment of turning.");
				return;
			}
			case 3:
			{
				IEntity user = PlayerEntity(playerId);
				if (!user || !RZ_Util.IsArmed(user))
				{
					gm.NotifyPlayer(playerId, m_sTitle, "Not with your bare hands.");
					return;
				}

				SetSpotOn(m_iBedSpot, false);
				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "He sees what you're going to do and nods, and says {sister}'s name. Then it's done, and the clipboard's last line stays empty."));
				Resolve(gm, playerId, RZ_EQuestApproach.FORCE, RZ_Quest_OtherTwo.OUT_ENDED, "ended it before the clipboard could");
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void WalkHome(RZ_GameModeComponent gm, int playerId)
	{
		if (!m_Walk.Begin(gm, m_vBed))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "He tries to stand and can't. Not today. (Too many people are being walked about the Zone right now.)");
			return;
		}

		// A Nurse packs him properly for the road: half as long again
		if (RZ_QuestApproach.HasBackground(gm, playerId, "nurse"))
		{
			m_fLeft = m_fLeft * 1.5;
			gm.NotifyPlayer(playerId, m_sTitle, "You get fluids into him and his feet into his boots the way they taught you. He'll last longer on the road.");
		}

		SetSpotOn(m_iBedSpot, false);
		m_bWalking = true;
		m_Walk.Follow(playerId);
		m_Walk.Go();
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, string.Format("He gets up on your arm. Home is %1. He has about %2 minutes.", RZ_Util.DirectionText(m_vBed, m_vHome), RZ_Util.RoundToInt(m_fLeft / 60))));
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		if (!m_bWalking)
			return super.GetWhereText(gm, from);

		return QFill(gm, "{brother}: ") + RZ_Util.DirectionText(from, m_vPos) + QFill(gm, "\nHome, the safehouse in {town}: ") + RZ_Util.DirectionText(from, m_vHome);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Other Two: {brother} turned before anyone could do anything for him.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Walk.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! The Other Two, stage 4 (Haven contract): the field lead at her station (held while it runs) and
//! the duty officer at the lab. Close the room (persuade), report her, keep quiet (Halcyon Favor)
//! or sell one of the other two to the Red Hand (Trusted with them). She's gone: the room is too.
class RZ_QM_OtherLead : RZ_QM_OtherTwoScene
{
	protected static const int OBJ_LEAD = 0;
	protected static const int OBJ_DUTY = 1;
	protected static const int KEEP_FAVOR = 25;
	protected static const int SOLD_FAVOR = 40;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetSister(8, 10, 3600);
		m_Req.Bonus("doctor");
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		if (!Run(gm) || HoldLead(gm) < 0)
			return false;

		RZ_Leader l = gm.GetLeaders().Get(m_iLead);
		m_vPos = NearSeat(l.m_vPos);
		m_iSettlement = gm.FindNearestSettlement(l.m_vPos);
		int talk = AddClue(l.m_sName, string.Format("%1 has the three clipboards in a stack on her desk. \"Somebody has to be there at the moment of turning, or the other two die for nothing. I chose to be the somebody. What would you have me do?\"", l.m_sShort));
		string choices = string.Format("Ask her to close the room|Say nothing (%1 Halcyon Favor)|Sell one to the Red Hand (%2 Red Hand Favor)", KEEP_FAVOR, SOLD_FAVOR);
		AddSpot(m_vPos, RZ_EPuzzleProp.CIVILIAN, OBJ_LEAD, talk, "Talk to " + l.m_sShort, choices, string.Format("%1 at the field station in %2", l.m_sShort, l.m_sPlace));

		vector lab = gm.GetLabPosition();
		int duty = AddClue("The duty officer", "The duty officer at the lab writes down whatever you say, and the time you said it.");
		AddSpot(LandPointNear(lab, 6, 12), RZ_EPuzzleProp.CIVILIAN, OBJ_DUTY, duty, "Talk to the duty officer", "Report the back room to the director", "the duty officer at the lab");

		m_sTitle = "The Other Two: the field lead";
		m_sBrief = QFill(gm, "{lead} runs the back room at {lead_place}. Ask her to close it, tell the director about it, or keep it to yourself. Whatever you choose, {sister} will want to know.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnScene(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		// She's gone (killed, replaced, the station lost): the room went with her
		RZ_Leader l = gm.GetLeaders().Get(m_iLead);
		if (!l || !l.IsPresent())
			Resolve(gm, -1, RZ_EQuestApproach.NONE, RZ_Quest_OtherTwo.OUT_GONE, "", true);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (!MayAct(gm, playerId))
			return;

		RZ_QuestRun run = Run(gm);
		if (objIdx == OBJ_DUTY && option == 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "By evening {director} has read it. By morning there's a new name on the door at {lead_place}, and the back room's beds are in a van to the lab."));
			if (gm.GetLeaders() && m_iLead >= 0)
				gm.GetLeaders().Remember(m_iLead, playerId, RZ_Leaders.MEM_CROSSED);

			Resolve(gm, playerId, RZ_EQuestApproach.BETRAY, RZ_Quest_OtherTwo.OUT_REPORTED, "reported her to the director");
			return;
		}

		if (objIdx != OBJ_LEAD)
			return;

		switch (option)
		{
			case 0:
			{
				if (!TryPersuade(gm, playerId, RZ_EFaction.INSTITUTE, 2, "doctor"))
				{
					Remember(gm, playerId, RZ_EQuestApproach.PERSUADE, "asked her to close it and was refused");
					gm.NotifyPlayer(playerId, m_sTitle, "\"You don't know the work.\" She doesn't raise her voice. (She'd listen to someone the lab trusts, or to a doctor.)");
					return;
				}

				gm.NotifyPlayer(playerId, m_sTitle, "She looks at the clipboards for a long time. Then she takes the pen off its string and puts it in her pocket. \"All right. Sit with them, then. Somebody should.\"");
				Resolve(gm, playerId, RZ_EQuestApproach.PERSUADE, RZ_Quest_OtherTwo.OUT_CLOSED, "talked her into closing the room");
				return;
			}
			case 1:
			{
				if (run)
					run.Reward(gm, playerId, RZ_EFaction.INSTITUTE, 0, KEEP_FAVOR, "\"Thank you for understanding.\" She means it, which is the worst part.");

				Resolve(gm, playerId, RZ_EQuestApproach.BRIBE, RZ_Quest_OtherTwo.OUT_KEPT, "kept quiet about the room", false);
				return;
			}
			case 2:
			{
				if (Rank(gm, playerId, RZ_EFaction.BANDITS) < 2)
				{
					gm.NotifyPlayer(playerId, m_sTitle, "You'd need a crew that trusts you to take a live one off your hands. (Trusted with the Red Hand.)");
					return;
				}

				RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.BANDITS, -1, 10, 0);
				if (run)
					run.Reward(gm, playerId, RZ_EFaction.BANDITS, 5, SOLD_FAVOR, "A crew came for the one in the third bed at dusk, and paid well. \"For the pit. Nobody asks where they come from.\"");

				Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_OtherTwo.OUT_SOLD, "sold one of the other two to the Red Hand", false);
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "The Other Two: nobody said anything to {lead} about the back room.");
	}
}

//------------------------------------------------------------------------------------------------
//! The Other Two, stage 5 (Haven contract): names. The paper has the house's other volunteer as a
//! number. At the sister's safehouse, the trial list by the door says who went up that day and the
//! man who walked them up says who came back (The Author List's deduction; an Elder reads the
//! council's pencil). The sister takes the name (wrong: five minutes; twice: numbers, then). Then
//! the duty officer at the lab (for the director: Trusted, or Known with the sister there after he
//! came home), the Haven's council clerk, or let them stay numbers.
class RZ_QM_OtherNames : RZ_QM_OtherTwoScene
{
	protected string m_sAnswer;
	protected int m_iOther;
	protected ref array<string> m_aOptions = {};
	protected int m_iListClue = -1;
	protected int m_iHelperClue = -1;
	protected bool m_bListRead;
	protected bool m_bHelperHeard;
	protected bool m_bNamed;
	protected int m_iSisterSpot = -1;
	protected int m_iLabSpot = -1;
	protected int m_iHavenSpot = -1;
	protected int m_iSisterLabSpot = -1;

	protected static const int OBJ_SISTER = 0;
	protected static const int OBJ_DUTY = 1;
	protected static const int OBJ_CLERK = 2;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetSister(12, 15, 4200);
		m_Req.Bonus("elder");
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		RZ_Safehouse h;
		if (run)
			h = RZ_Questline.GetHouse(gm, run.Flag("house", -1));

		if (!run || !h)
			return false;

		// The same names, days and numbers every time this stage is offered (a restart included)
		RZ_Rng rng = new RZ_Rng(run.Flag("seed", 1));
		array<string> names = {};
		PickVolunteers(rng, names);
		m_sAnswer = names[0];
		string walked = names[1];
		string other = names[2];
		string helper = names[3];
		int num = run.Flag("num", 40);
		int day = run.Flag("day", 9);
		int today = 12;
		if (gm.GetBroadcast())
			today = Math.Max(8, gm.GetBroadcast().GetDay());

		// The other volunteer went up on another day before today; the lab numbers them in order
		int otherDay = day;
		for (int g = 0; g < 20 && otherDay == day; g++)
		{
			otherDay = rng.Int(6, today);
		}

		if (otherDay == day)
			otherDay = Math.Max(6, day - 1);

		m_iOther = num + rng.Int(3, 9);
		if (otherDay < day)
			m_iOther = Math.Max(1, num - rng.Int(3, 9));

		// The list by the door: his day (somebody else went up with him and came back), and the
		// other volunteer's day (two went up, one came back)
		string sameDay = string.Format("%1. %2.", m_sAnswer, walked);
		if (rng.Chance(0.5))
			sameDay = string.Format("%1. %2.", walked, m_sAnswer);

		string hisDay = string.Format("%1. %2.", QText(gm, "brother_full"), other);
		string list = string.Format("Day %1: %2\nDay %3: %4", day, hisDay, otherDay, sameDay);
		if (otherDay < day)
			list = string.Format("Day %1: %2\nDay %3: %4", otherDay, sameDay, day, hisDay);
		m_iListClue = AddClue("The trial list", string.Format("Pinned inside the door, in pencil, in three different hands: \"Walked up to the lab for the trial, by %1:\"\n%2\nUnderneath, a row of little candles drawn in the same pencil, and no names next to them.", helper, list));
		m_iHelperClue = AddClue(helper, string.Format("\"I walked them up. All of them, every time. You hold their hand up the hill, and then you wait outside, because they won't let you in.\" %1 looks at the door, not at you. \"%2 came back down. So did %3. I carried %3's bag.\"", helper, walked, other));
		int sister = AddClue(QText(gm, "sister"), QFill(gm, "{sister} has a copy of the lab's paper, folded small. \"{brother} is Volunteer {number}. There's another from this house: Volunteer " + m_iOther.ToString() + ", Day " + otherDay.ToString() + ". Somebody from here, and nobody will say who.\""));

		m_aOptions.Insert(m_sAnswer);
		m_aOptions.Insert(walked);
		m_aOptions.Insert(other);
		rng.ShuffleStrings(m_aOptions);

		m_vPos = h.m_vPos;
		m_iSettlement = h.m_iSettlement;
		AddSpot(LandPointNear(h.m_vPos, 2, 4), RZ_EPuzzleProp.CLUE, -1, m_iListClue, "Read the list by the door", "", "");
		AddSpot(LandPointNear(h.m_vPos, 4, 8), RZ_EPuzzleProp.CIVILIAN, -1, m_iHelperClue, "Talk to " + helper, "", "");
		m_iSisterSpot = AddSpot(LandPointNear(h.m_vPos, 4, 8), RZ_EPuzzleProp.CIVILIAN, OBJ_SISTER, sister, QFill(gm, "Talk to {sister}"), "", QFill(gm, "{sister} at the safehouse in {town}"));

		// Where the names can go, once she has them
		int duty = AddClue("The duty officer", QFill(gm, "The duty officer at the lab. \"Names for {director}? She'll see you if she knows you.\""));
		m_iLabSpot = AddSpot(LandPointNear(gm.GetLabPosition(), 6, 12), RZ_EPuzzleProp.CIVILIAN, OBJ_DUTY, duty, "Talk to the duty officer", "Bring the names to the director", "the duty officer at the lab");
		SetSpotOn(m_iLabSpot, false);
		RZ_Leader chair = Top(gm, RZ_EFaction.SURVIVORS);
		if (chair)
		{
			int clerk = AddClue("The council clerk", "The Haven council's clerk. \"The chair reads names at supper. Every one, slowly.\"");
			m_iHavenSpot = AddSpot(NearSeat(chair.m_vPos), RZ_EPuzzleProp.CIVILIAN, OBJ_CLERK, clerk, "Talk to the council clerk", "Give the names to the chair", "the council clerk at the Haven");
			SetSpotOn(m_iHavenSpot, false);
		}

		// He came home: she'll walk up the hill with you, and stand beside you when the names go in
		if (run.Flag("brother") == RZ_Quest_OtherTwo.BRO_HOME)
		{
			int sisterLab = AddClue(QText(gm, "sister"), QFill(gm, "{sister} came up the hill with the names. She's wearing his coat."));
			m_iSisterLabSpot = AddSpot(LandPointNear(gm.GetLabPosition(), 6, 12), RZ_EPuzzleProp.CIVILIAN, -1, sisterLab, QFill(gm, "Talk to {sister}"), "", "");
			SetSpotOn(m_iSisterLabSpot, false);
		}

		m_sTitle = "The Other Two: names";
		m_sBrief = QFill(gm, "The lab's paper lists its volunteers by number. {brother} is Volunteer {number}. Another number on it came from the safehouse in {town}. Find out who it was. Then decide who gets the names: the director, the chair of the Haven, or nobody.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Four full names; the first three (the volunteers) with different initials.
	protected static void PickVolunteers(RZ_Rng rng, notnull array<string> result)
	{
		array<string> firsts = {};
		RZ_NoteText.DistinctFirstNames(rng, 4, firsts);
		while (firsts.Count() < 4)
		{
			firsts.Insert("Dana");
		}

		array<string> initials = {};
		for (int i = 0; i < 4; i++)
		{
			string full = firsts[i] + " " + RZ_NoteText.Surname(rng);
			for (int tries = 0; tries < 10 && i < 3 && initials.Contains(Initials(full)); tries++)
			{
				full = firsts[i] + " " + RZ_NoteText.Surname(rng);
			}

			initials.Insert(Initials(full));
			result.Insert(full);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static string Initials(string full)
	{
		int gap = full.IndexOf(" ");
		if (gap < 0 || gap + 1 >= full.Length())
			return full.Substring(0, 1);

		return full.Substring(0, 1) + full.Substring(gap + 1, 1);
	}

	//------------------------------------------------------------------------------------------------
	protected bool HouseDone()
	{
		return m_bListRead && m_bHelperHeard;
	}

	//------------------------------------------------------------------------------------------------
	protected string SisterLabels()
	{
		if (!HouseDone() || m_bNamed)
			return "";

		string labels = "";
		foreach (int i, string n : m_aOptions)
		{
			labels += string.Format("Volunteer %1 was %2|", m_iOther, n);
		}

		return labels + "Let them stay numbers";
	}

	//------------------------------------------------------------------------------------------------
	override string GetClueText(RZ_GameModeComponent gm, int playerId, int idx)
	{
		string text = super.GetClueText(gm, playerId, idx);
		if (idx == m_iListClue && HasBonus(gm, playerId))
			text += string.Format("\n(You know how a council keeps a list. The pencil has gone over %1 twice, the way they mark someone who didn't come home.)", m_sAnswer);

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		bool before = HouseDone();
		if (idx == m_iListClue)
			m_bListRead = true;

		if (idx == m_iHelperClue)
			m_bHelperHeard = true;

		if (before || !HouseDone())
			return;

		SetSpotLabels(m_iSisterSpot, SisterLabels());
		NotifyParticipants(gm, m_sTitle, QFill(gm, "You know who went up on which day, and who came back down. Tell {sister} who Volunteer " + m_iOther.ToString() + " was."));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (!MayAct(gm, playerId))
			return;

		if (objIdx == OBJ_SISTER)
		{
			SisterChoice(gm, playerId, option);
			return;
		}

		if (!m_bNamed || option != 0)
			return;

		RZ_QuestRun run = Run(gm);
		if (objIdx == OBJ_CLERK)
		{
			RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.SURVIVORS, RZ_EFaction.INSTITUTE, 10, 5);
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "The chair reads them at supper, every one, slowly. When she gets to {brother_full} somebody at the back says \"Here,\" the way they used to at school."));
			Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_OtherTwo.OUT_TO_HAVEN, "gave the names to the Haven", true);
			return;
		}

		if (objIdx != OBJ_DUTY)
			return;

		// Trusted with the lab; or Known, with the sister beside you once he came home
		bool sisterThere = run && run.Flag("brother") == RZ_Quest_OtherTwo.BRO_HOME && Rank(gm, playerId, RZ_EFaction.INSTITUTE) >= 1;
		if (Rank(gm, playerId, RZ_EFaction.INSTITUTE) < 2 && !sisterThere)
		{
			Remember(gm, playerId, RZ_EQuestApproach.TALK, "was turned away at the lab");
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "\"{director} doesn't see people she doesn't know.\" (She'll see someone the lab trusts, or someone it knows with the family beside them.)"));
			return;
		}

		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{director} reads the names herself. Then she takes the paper, and a pen, and writes them over the numbers, in ink, one at a time. It takes a long time. Nobody leaves."));
		Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_OtherTwo.OUT_NAMED, "brought the names to the director");
	}

	//------------------------------------------------------------------------------------------------
	protected void SisterChoice(RZ_GameModeComponent gm, int playerId, int option)
	{
		if (!HouseDone() || m_bNamed)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You don't know enough yet. The list by the door does, and so does the man who walked them up.");
			return;
		}

		if (option == m_aOptions.Count())
		{
			RZ_QuestRun run = Run(gm);
			if (run)
				run.Reward(gm, playerId, RZ_EFaction.INSTITUTE, 10, 25, "The lab hears that nobody is coming with names. It's grateful, quietly.");

			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{sister} folds the paper back up. \"Numbers, then. At least I know his.\""));
			Resolve(gm, playerId, RZ_EQuestApproach.BETRAY, RZ_Quest_OtherTwo.OUT_NUMBERS, "let them stay numbers", false);
			return;
		}

		if (option < 0 || option >= m_aOptions.Count())
			return;

		if (m_aOptions[option] == m_sAnswer)
		{
			m_bNamed = true;
			SetSpotLabels(m_iSisterSpot, "");
			SetSpotOn(m_iLabSpot, true);
			SetSpotOn(m_iHavenSpot, true);
			SetSpotOn(m_iSisterLabSpot, true);
			Remember(gm, playerId, RZ_EQuestApproach.TALK, "put a name to the number");
			NotifyParticipants(gm, m_sTitle, QFill(gm, string.Format("{sister} writes it in, in pencil, next to the number: %1. \"Now the rest of them.\" The houses have their lists. The names can go to {director} at the lab, or to the chair of the Haven. Or nowhere.", m_sAnswer)));
			return;
		}

		m_iWrong++;
		if (m_iWrong >= 2)
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{sister} shakes her head. \"I won't write a guess next to his number. Numbers, then.\""));
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_OtherTwo.OUT_NUMBERS, "couldn't put a name to the number", false);
			return;
		}

		LockOut(300);
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{sister} checks it against the list and shakes her head. \"That one came back. Ask again. I'll be here.\""));
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The Other Two: the numbers stay numbers.";
	}
}
