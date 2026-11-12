//! The base class for one questline (docs/DESIGN.md, "Questlines": the author's guide). A
//! questline holds no state of its own: everything that changes lives in the RZ_QuestRun it is
//! handed (saved by the engine, Director/RZ_Questlines.c). Write one subclass per questline in
//! its own file under Missions/Questlines/<Faction>/ and add it to RZ_QuestlineRegistry.

//------------------------------------------------------------------------------------------------
//! One stage of a questline: a mission stage (offers one mission) or a wait stage. Built in
//! Define() with RZ_Questline.MissionStage / WaitStage, then tuned by chaining:
//!   WaitStage(S_LATER, "A week later", "...").Wait(60, 24).GiveUp(4, "It went cold.");
class RZ_QuestStage
{
	int m_iId;
	string m_sTitle;			// the journal's "by" line ("The handover"); {keys} are filled in
	string m_sKnown;			// the journal brief while this stage is current; {keys} are filled in
	bool m_bMission;
	float m_fWaitMinutes;		// play minutes before the stage can happen (x the wait scale)
	float m_fWaitRealHours;		// real hours instead (or as well, with both)
	bool m_bWaitBoth;
	float m_fGiveUpHours = 3;	// play hours after the wait before the stage fizzles if it can't happen
	int m_iMaxVisits = 3;		// a stage entered more often than this fizzles (a retry loop)
	string m_sFizzle = "";

	//------------------------------------------------------------------------------------------------
	//! Wait `playMinutes` of play or `realHours` of real time, whichever comes first (both = true:
	//! both must pass). 0 = no wait of that kind.
	RZ_QuestStage Wait(float playMinutes, float realHours = 0, bool both = false)
	{
		m_fWaitMinutes = Math.Max(0, playMinutes);
		m_fWaitRealHours = Math.Max(0, realHours);
		m_bWaitBoth = both;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	//! After the wait, how many play hours the stage may wait for its world conditions (IsReady)
	//! or for its mission to find a place, before the questline fizzles with `text`.
	RZ_QuestStage GiveUp(float playHours, string text)
	{
		m_fGiveUpHours = Math.Max(0.1, playHours);
		m_sFizzle = text;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	//! How often the story may come back to this stage (retries after a failure).
	RZ_QuestStage Visits(int count)
	{
		m_iMaxVisits = Math.MaxInt(1, count);
		return this;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_Questline
{
	static const int END = -1;				// Next(): the questline is over (after Finish or Fizzle)
	static const int OUTCOME_FAILED = 0;	// the stage mission failed or ran out of time, and set no outcome
	static const int OUTCOME_DONE = 1;		// it succeeded and set no outcome, or a wait stage is over
	// Authors' own outcomes and endings: any number, 10 and up for outcomes by convention

	// Consequence caps (one call; nothing overpowered, everything recovers)
	static const float MAX_MORALE = 10;
	static const float MAX_STRENGTH = 5;
	static const float MAX_INFECTION = 15;
	static const int MAX_FOOD = 6;
	static const int MAX_PEOPLE = 3;
	static const int MAX_FAME = 10;
	static const int MAX_INFAMY = 15;

	string m_sKey;					// stable id, saved: never change it once players have it
	string m_sTitle;
	int m_iFaction = -1;			// the faction it belongs to (journal accent, one per faction); -1 none
	protected string m_sSlot;		// what "one at a time" counts by; default the faction
	protected ref array<ref RZ_QuestStage> m_aStages = {};

	// The majors (docs/QUESTLINES.md, "The majors"; the engine in Director/RZ_Majors.c)
	protected bool m_bMajor;
	protected ref array<ref RZ_MajorAct> m_aActs = {};

	// ================================================================ The definition
	//------------------------------------------------------------------------------------------------
	//! Set the key, title and faction with Info(), then add the stages. The first stage added is
	//! where the questline starts.
	void Define()
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void Info(string key, string title, int faction)
	{
		m_sKey = key;
		m_sTitle = title;
		m_iFaction = faction;
	}

	//------------------------------------------------------------------------------------------------
	//! Questlines with the same slot never run together. Default: the faction ("f0".."f3"; "f-1"
	//! for none). A group that isn't an RZ_EFaction (e.g. a cannibal band) sets its own.
	protected void Slot(string slot)
	{
		m_sSlot = slot;
	}

	//------------------------------------------------------------------------------------------------
	string GetSlot()
	{
		if (!m_sSlot.IsEmpty())
			return m_sSlot;

		return "f" + m_iFaction.ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! A cure finale route (planned; docs/DESIGN.md, "Cure research"): while any registered questline
	//! says yes, research stops at 90 percent until a finale takes it further with
	//! RZ_CureResearch.AddFinaleResearch. None does yet, so research runs to 100 as before.
	bool IsCureFinale()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A stage that offers one mission (MakeMission) to eligible players.
	protected RZ_QuestStage MissionStage(int id, string title, string known)
	{
		return AddStage(id, title, known, true);
	}

	//------------------------------------------------------------------------------------------------
	//! A stage that only waits: time (Wait) and the world (IsReady).
	protected RZ_QuestStage WaitStage(int id, string title, string known)
	{
		return AddStage(id, title, known, false);
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_QuestStage AddStage(int id, string title, string known, bool mission)
	{
		RZ_QuestStage st = new RZ_QuestStage();
		st.m_iId = id;
		st.m_sTitle = title;
		st.m_sKnown = known;
		st.m_bMission = mission;
		st.m_sFizzle = string.Format("%1 went quiet. Nobody picked it up again.", m_sTitle);
		m_aStages.Insert(st);
		return st;
	}

	//------------------------------------------------------------------------------------------------
	RZ_QuestStage GetStage(int id)
	{
		foreach (RZ_QuestStage st : m_aStages)
		{
			if (st.m_iId == id)
				return st;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	int StageCount()
	{
		return m_aStages.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Where a run begins: the first stage added, unless a questline says otherwise.
	int FirstStage()
	{
		if (m_aStages.IsEmpty())
			return END;

		return m_aStages[0].m_iId;
	}

	// ================================================================ The majors
	//------------------------------------------------------------------------------------------------
	//! In Define(): this questline is its faction's major. It runs in the majors' lane (outside Max
	//! active and the one-per-faction rule), starts with priority once CanStart is true, runs once
	//! a season and never fizzles: a stage that can't happen waits, and the act's default (when the
	//! clock says so) moves it on. Its stages belong to acts (Act()).
	protected void Major()
	{
		m_bMajor = true;
	}

	//------------------------------------------------------------------------------------------------
	bool IsMajor()
	{
		return m_bMajor;
	}

	//------------------------------------------------------------------------------------------------
	//! False: never offered by the half-hourly roll (an inner line a major starts with Begin, or a
	//! questline a major absorbed under a new key). Begin() still starts it.
	bool StartsOnItsOwn()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A major's act: its stages, anchor, need, patience and last call (RZ_MajorAct's setters).
	protected RZ_MajorAct Act(int id, string name)
	{
		RZ_MajorAct act = new RZ_MajorAct();
		act.m_iId = id;
		act.m_sName = name;
		m_aActs.Insert(act);
		return act;
	}

	//------------------------------------------------------------------------------------------------
	RZ_MajorAct GetAct(int id)
	{
		foreach (RZ_MajorAct act : m_aActs)
		{
			if (act.m_iId == id)
				return act;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The act a stage belongs to, null if none.
	RZ_MajorAct ActOf(int stage)
	{
		foreach (RZ_MajorAct act : m_aActs)
		{
			if (act.Has(stage))
				return act;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	int ActCount()
	{
		return m_aActs.Count();
	}

	//------------------------------------------------------------------------------------------------
	RZ_MajorAct ActAt(int idx)
	{
		if (idx < 0 || idx >= m_aActs.Count())
			return null;

		return m_aActs[idx];
	}

	//------------------------------------------------------------------------------------------------
	//! Could this player decide one of the act's approaches now? Default: the act's access rank
	//! with the major's faction (Known unless the act says otherwise).
	bool ActQualifies(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_MajorAct act, int playerId)
	{
		if (!gm || playerId <= 0 || !RZ_Factions.IsValid(m_iFaction))
			return false;

		return RZ_Reputation.GetAccessRank(gm.GetTier(playerId, m_iFaction)) >= act.m_iRank;
	}

	//------------------------------------------------------------------------------------------------
	//! The act's default (docs/QUESTLINES.md, "Dynamic fallbacks"): what happens when no player
	//! decides it. It pays nobody (the engine turns pay off around it). Apply its world part, set its
	//! flags and return the next stage, or END through Finish().
	int ActDefault(RZ_GameModeComponent gm, RZ_QuestRun run, RZ_MajorAct act)
	{
		return END;
	}

	//------------------------------------------------------------------------------------------------
	//! An absorbed questline reached one of its own endings inside the major (B1, R1, H1): apply
	//! it as an act's result (ActEnd) and return the stage the major goes on to. END: the major
	//! itself ends there.
	int AfterInnerEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
		return END;
	}

	//------------------------------------------------------------------------------------------------
	//! One line for "between beats" (one of the major's people saying what's coming), "" if none.
	string BetweenLine(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The major's line for the journal's season view: the rule or stance in force, the leans. ""
	//! if none.
	string SeasonState(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! For a major's stage missions (RZ_QM_MajorScene): fill in the scene for `stage`. False: it
	//! can't now (tried again in a few minutes).
	bool BuildScene(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! For Next() and ActDefault(): an act's result inside a major, without ending it. Sets
	//! "<key>.act<act>" (and "<key>.climax" for the climax) to `outcome`, notes `line`, tells the
	//! participants and marks a beat. `line` "" = no line.
	protected void ActEnd(RZ_GameModeComponent gm, RZ_QuestRun run, int act, int outcome, string line)
	{
		RZ_Majors.ActEnd(gm, run, act, outcome, line);
	}

	// ================================================================ What the author writes
	//------------------------------------------------------------------------------------------------
	//! Can it begin now? Pick its places and people here (run.SetFlag / SetText / SetPlace).
	//! False = not now (it's tried again later). Spawn nothing here.
	bool CanStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! It began: an opening headline, a first note.
	void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! A stage began (also the first one).
	void OnStageBegin(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! World gates on top of the stage's wait: a leader alive, a town held, morale, a flag. While
	//! false the stage waits (and fizzles after its give-up time).
	bool IsReady(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Mission stages: a new stage mission (usually an RZ_QuestMission subclass). The engine sets
	//! its m_iQuestRun, runs Configure and starts it through the mission manager; it reads its
	//! data from the run in Setup. Null = can't now (tried again in a few minutes).
	RZ_Mission MakeMission(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The stage `stage` came out as `outcome` (yours, set by the mission's approach, or
	//! OUTCOME_DONE / OUTCOME_FAILED). Return the next stage's id, or END through Finish() or
	//! Fizzle(). Add notes and set flags here.
	int Next(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		return Fizzle(run, "");
	}

	//------------------------------------------------------------------------------------------------
	//! The ending's world consequences, through the helpers below.
	void OnEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! The final stand (re)starts (RZ_Questlines.OnFinalStandStart): a running line may settle what
	//! must be settled before it. A major's pay and effects go through its caps.
	void OnFinalStandStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! The final stand ended: won (the cure), or lost (the batch ruined, research back to 90).
	void OnFinalStandEnd(RZ_GameModeComponent gm, RZ_QuestRun run, bool won)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! The clean room changed holder during the stand (RZ_Questlines.OnStandHolder): from and to are
	//! finale routes (RZ_CureFinale.HAVEN ... INDEP, NONE for nobody).
	void OnStandHolder(RZ_GameModeComponent gm, RZ_QuestRun run, int from, int to)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! The journal's brief for the current stage. Default: the stage's text, filled in.
	string JournalText(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_QuestStage st = GetStage(run.m_iStage);
		if (!st)
			return "";

		return run.Fill(st.m_sKnown);
	}

	//------------------------------------------------------------------------------------------------
	//! Gossip while it runs ({keys} filled in), "" if none.
	string Rumour(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A line when a wanderer is talked to ({keys} filled in), "" if none. Can set flags.
	string OnWandererTalk(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, int wandererId)
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A line when a leader is talked to ({keys} filled in), "" if none. Can set flags.
	string OnLeaderTalk(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, int leaderId)
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! For Next(): the questline reaches ending `ending` (your number, 0 and up). `text` is told
	//! to everyone who took part; OnEnding applies the consequences.
	protected int Finish(RZ_QuestRun run, int ending, string text)
	{
		run.m_iEnding = Math.MaxInt(0, ending);
		run.m_sEndText = run.Fill(text);
		return END;
	}

	//------------------------------------------------------------------------------------------------
	//! For Next(): the questline fizzles out with one line ("" = a plain one). It may come back later.
	protected int Fizzle(RZ_QuestRun run, string text)
	{
		run.m_iEnding = -1;
		run.m_sEndText = run.Fill(text);
		return END;
	}

	// ================================================================ World state
	//------------------------------------------------------------------------------------------------
	static bool IsLeaderAlive(RZ_GameModeComponent gm, int leaderId)
	{
		if (!gm.GetLeaders())
			return false;

		RZ_Leader l = gm.GetLeaders().Get(leaderId);
		return l && l.IsPresent();
	}

	//------------------------------------------------------------------------------------------------
	//! A faction's leader for a safehouse or holding seat near pos (within 300 m), -1 if none.
	static int LeaderNear(RZ_GameModeComponent gm, int faction, vector pos)
	{
		if (!gm.GetLeaders())
			return -1;

		RZ_Leader l = gm.GetLeaders().FindNearest(faction, pos, 300);
		if (!l)
			return -1;

		return l.m_iId;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsTownHeldBy(RZ_GameModeComponent gm, int settlement, int faction)
	{
		RZ_Territory territory = gm.GetTerritory();
		if (!territory || !territory.IsActive())
			return false;

		RZ_Holding h = territory.FindInTown(settlement);
		return h && h.m_eFaction == faction;
	}

	//------------------------------------------------------------------------------------------------
	//! A faction's morale (0-100, 60 at rest); 60 with faction life off.
	static float GetMorale(RZ_GameModeComponent gm, int faction)
	{
		if (!gm.GetFactionLife() || !gm.GetFactionLife().IsActive())
			return 60;

		return gm.GetFactionLife().GetMorale(faction);
	}

	//------------------------------------------------------------------------------------------------
	static RZ_Safehouse GetHouse(RZ_GameModeComponent gm, int idx)
	{
		if (!gm.GetSafehouses())
			return null;

		return gm.GetSafehouses().Get(idx);
	}

	// ================================================================ Consequences (capped)
	//------------------------------------------------------------------------------------------------
	//! Faction morale, at most 10 points either way (it drifts back by itself).
	static void NudgeMorale(RZ_GameModeComponent gm, int faction, float delta)
	{
		delta = RZ_Majors.MoraleThroughBudget(faction, Math.Clamp(delta, -MAX_MORALE, MAX_MORALE));
		if (gm.GetFactionLife() && delta != 0)
			gm.GetFactionLife().AddMorale(faction, delta);
	}

	//------------------------------------------------------------------------------------------------
	//! Faction strength (territory), at most 5 points either way (it drifts back too).
	static void NudgeStrength(RZ_GameModeComponent gm, int faction, float delta)
	{
		delta = RZ_Majors.StrengthThroughBudget(faction, Math.Clamp(delta, -MAX_STRENGTH, MAX_STRENGTH));
		if (gm.GetTerritory() && delta != 0)
			gm.GetTerritory().AddStrength(faction, delta);
	}

	//------------------------------------------------------------------------------------------------
	//! A town's infection, at most 15 points either way.
	static void NudgeInfection(RZ_GameModeComponent gm, int settlement, float delta)
	{
		RZ_Settlement s = gm.GetSettlement(settlement);
		if (s)
			s.AddInfection(Math.Clamp(delta, -MAX_INFECTION, MAX_INFECTION));
	}

	//------------------------------------------------------------------------------------------------
	//! A safehouse's food, at most 6 either way.
	static void ChangeFood(RZ_GameModeComponent gm, int house, int delta)
	{
		RZ_Safehouse h = GetHouse(gm, house);
		if (!h)
			return;

		delta = Math.ClampInt(delta, -MAX_FOOD, MAX_FOOD);
		if (delta > 0)
			gm.GetSafehouses().AddFood(house, delta);
		else
			h.m_iFood = Math.Max(0, h.m_iFood + delta);
	}

	//------------------------------------------------------------------------------------------------
	//! People in a safehouse, at most 3 either way (those who leave count as lost). Never below the
	//! upkeep floor of 2 people (docs/QUESTLINES.md, "Limits every questline keeps").
	static void ChangePopulation(RZ_GameModeComponent gm, int house, int delta)
	{
		RZ_Safehouse h = GetHouse(gm, house);
		if (!h)
			return;

		delta = Math.ClampInt(delta, -MAX_PEOPLE, MAX_PEOPLE);
		if (delta < 0)
			delta = Math.Min(0, Math.Max(delta, RZ_Safehouses.UPKEEP_FLOOR - h.m_iPopulation));

		if (delta > 0)
			gm.GetSafehouses().AddPopulationAt(house, delta);
		else if (delta < 0)
			gm.GetSafehouses().RemovePopulation(house, -delta);
	}

	//------------------------------------------------------------------------------------------------
	//! A leader leaves their seat and is replaced like after a death, with `headline`. Never a
	//! stronghold leader, nor one another story has claimed (a leader `run` claimed itself may go).
	//! False if it can't happen (then say nothing about it).
	static bool ReplaceLeader(RZ_GameModeComponent gm, int leaderId, string headline, RZ_QuestRun run = null)
	{
		if (!gm.GetLeaders())
			return false;

		string owner = "";
		if (run)
			owner = run.Owner();

		return gm.GetLeaders().RemoveFromSeat(leaderId, headline, owner);
	}

	//------------------------------------------------------------------------------------------------
	//! An outpost goes to `taker` (RZ_Territory.HandOver: never a stronghold, never the owner's
	//! last, not during a truce, not while a player watches). False if it can't happen now.
	static bool HandOutpost(RZ_GameModeComponent gm, int holdingId, int taker)
	{
		if (!gm.GetTerritory())
			return false;

		return gm.GetTerritory().HandOver(holdingId, taker);
	}

	//------------------------------------------------------------------------------------------------
	//! A headline (kept for "Word on the roads"; announce = also told to everyone online now).
	static void Headline(RZ_GameModeComponent gm, string text, bool announce = true)
	{
		if (text.IsEmpty())
			return;

		if (gm.GetLeaders())
			gm.GetLeaders().AddHeadline(text, announce);
		else if (announce)
			gm.NotifyAll("Word on the roads", text);
	}

	//------------------------------------------------------------------------------------------------
	//! Gossip wanderers and leaders repeat for `hours` of play.
	static void SpreadRumour(RZ_GameModeComponent gm, string text, float hours)
	{
		if (gm.GetQuestlines())
			gm.GetQuestlines().AddRumour(text, Math.Clamp(hours, 0, 48));
	}

	//------------------------------------------------------------------------------------------------
	//! A wanderer remembers everyone who took part (RZ_Wanderers.MEM_*: helped, wronged, saved).
	static void RememberWanderer(RZ_GameModeComponent gm, RZ_QuestRun run, int wandererId, int flag)
	{
		if (!gm.GetWanderers())
			return;

		array<string> idents = {};
		run.GetIdents(idents);
		foreach (string ident : idents)
		{
			gm.GetWanderers().RememberIdentity(wandererId, ident, flag);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Standing with `faction` for everyone who took part (online or not): Fame at most 10 either
	//! way, Infamy 0 to 15. onlyApproach >= 0: only those who took that approach at some stage.
	static void ChangeStanding(RZ_GameModeComponent gm, RZ_QuestRun run, int faction, int fame, int infamy, int onlyApproach = -1)
	{
		if (!RZ_Factions.IsValid(faction))
			return;

		fame = Math.ClampInt(fame, -MAX_FAME, MAX_FAME);
		infamy = Math.ClampInt(infamy, 0, MAX_INFAMY);
		if (RZ_Majors.IsNoPay())
			return;

		// The ending's own words as the deed (RZ_RepDeeds), when the run has reached one
		string deed = "";
		if (run.m_iEnding >= 0)
			deed = RZ_RepDeeds.ForEnding(run.m_sKey, run.m_iEnding);

		array<string> idents = {};
		run.GetIdents(idents);
		foreach (string ident : idents)
		{
			if (onlyApproach >= 0 && !run.IdentDid(ident, onlyApproach))
				continue;

			ChangeStandingFor(gm, ident, faction, fame, infamy, deed);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Standing for one identity (online or not; offline straight onto the saved profile, with no
	//! tier message), inside a major's caps while a major pays (RZ_Majors.ClipPay).
	static void ChangeStandingFor(RZ_GameModeComponent gm, string ident, int faction, int fame, int infamy, string deed = "")
	{
		if (RZ_Majors.IsNoPay() || ident.IsEmpty() || !RZ_Factions.IsValid(faction))
			return;

		fame = RZ_Majors.ClipPay(ident, faction, "fame", fame);
		infamy = RZ_Majors.ClipPay(ident, faction, "infamy", infamy);
		if (fame == 0 && infamy <= 0)
			return;

		int playerId = gm.GetQuestlines().OnlineId(ident);
		if (playerId > 0)
		{
			gm.AddFame(playerId, faction, fame, false, 0, deed);
			gm.AddInfamy(playerId, faction, infamy, "questline", RZ_RepFlag.SHARE);
			return;
		}

		RZ_PlayerProfile profile = gm.FindProfileByIdentity(ident);
		if (!profile)
			return;

		RZ_EFaction ef = faction;
		if (fame != 0)
			profile.AddFame(ef, fame);

		if (fame > 0)
			RZ_Deeds.NoteBest(profile, faction, fame, deed);

		if (infamy > 0)
			profile.AddInfamy(ef, infamy, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Tell everyone online who took part.
	static void TellParticipants(RZ_GameModeComponent gm, RZ_QuestRun run, string title, string text)
	{
		if (!gm.GetQuestlines() || text.IsEmpty())
			return;

		array<string> idents = {};
		run.GetIdents(idents);
		foreach (string ident : idents)
		{
			int playerId = gm.GetQuestlines().OnlineId(ident);
			if (playerId > 0)
				gm.NotifyPlayer(playerId, title, text);
		}
	}
}
