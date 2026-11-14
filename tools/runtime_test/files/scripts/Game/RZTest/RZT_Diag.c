// Test-only harness (not part of the mod).
modded class RZ_MissionManager
{
	protected int m_iRZTNext;
	protected int m_iRZTPhase;
	protected ref RZ_Mission m_RZTCur;
	protected int m_iRZTStarted;
	protected int m_iRZTWon;
	protected int m_iRZTWalks;
	protected bool m_bRZTChecked;

	// One mission at a time: start it, move the fake player there, then end it (alternately won
	// and failed) so the next one has the whole map. Returns false when all were tried.
	bool RZT_Step()
	{
		m_iRZTPhase++;
		if (m_iRZTPhase == 1)
		{
			// Clear anything still running (including missions the manager started itself)
			foreach (RZ_Mission old : m_aActive)
				old.m_eState = RZ_EMissionState.FAILED;
			return true;
		}
		if (m_iRZTPhase == 2)
		{
			if (m_iRZTNext >= m_aTypes.Count())
				return false;
			typename t = m_aTypes[m_iRZTNext];
			m_iRZTNext++;
			RZT_Prep.BeforeMission(m_GM, t);
			// A Setup that picks a random spot (dry ground, a free town) may miss; the manager would
			// just try again later, so the test has a few goes
			bool ok = false;
			int tries = 0;
			while (!ok && tries < RZT_Prep.TRIES)
			{
				if (tries > 0)
					RZT_Prep.BeforeMission(m_GM, t, tries);

				tries++;
				m_RZTCur = RZ_Mission.Cast(t.Spawn());
				if (!m_RZTCur)
					break;
				m_RZTCur.Configure(m_GM);
				// A pack's contract: the first go at the clock as it is, the others in its offer window
				if (tries > 1 && RZT_Prep.IntoOffer(m_RZTCur) && tries == 2)
					Print(string.Format("RZT forced: the clock into %1's offer window", t.ToString()));

				ok = StartMission(m_RZTCur);
			}
			if (!m_RZTCur)
			{
				Print("RZT mission " + t.ToString() + " SPAWN FAILED");
				m_iRZTPhase = 0;
				return true;
			}
			if (ok)
			{
				m_iRZTStarted++;
				// Jobs only by interaction: the radio only says who to ask, so the tester takes it the
				// way a giver offers it (RZ_Mission.Hire, what a leader's "Ask for work" calls); then
				// it's on his Quests tab and he's paid for it
				if (m_RZTCur.m_bContract)
				{
					HearOf(m_RZTCur, RZT_Fake.ID, RZT_Fake.s_Ent);
					RZT_TalkOffer(m_RZTCur);
					m_RZTCur.Hire(RZT_Fake.ID);
				}
				array<RZ_Mission> mine = {};
				ListFor(RZT_Fake.ID, mine);
				if (!mine.Contains(m_RZTCur) && m_RZTCur.m_bContract)
					Print(string.Format("RZT mission %1 offered but not on the tester's Quests tab", t.ToString()));
			}
			Print(string.Format("RZT mission %1 '%2' contract=%3 started=%4 tries=%5", t.ToString(), m_RZTCur.m_sTitle, m_RZTCur.m_bContract, ok, tries));
			if (!ok)
			{
				RZT_WhyNot(m_RZTCur);
				m_RZTCur = null;
				m_iRZTPhase = 0;
			}
			return true;
		}
		if (m_iRZTPhase == 3)
		{
			// The objective's place (RZT_ObjPlace.c): checked from where the tester is, then he goes
			// where the tracker points, not to the mission's centre
			m_iRZTWalks = 0;
			m_bRZTChecked = false;
			if (m_RZTCur && m_RZTCur.m_eState == RZ_EMissionState.ACTIVE && !m_RZTCur.m_bGlobal && m_RZTCur.m_vPos != vector.Zero)
			{
				RZT_ObjCheck(m_RZTCur, "start");
				RZT_Obj.Go(m_GM, m_RZTCur.m_iId, m_RZTCur.m_vPos);
			}
			return true;
		}
		if (m_iRZTPhase == 4)
		{
			if (!m_RZTCur || m_RZTCur.m_eState != RZ_EMissionState.ACTIVE || m_RZTCur.m_bGlobal)
				return true;

			// There: the place again (the step may have moved on), and a delivery's thing carried
			if (!m_bRZTChecked)
			{
				m_bRZTChecked = true;
				RZT_ObjCheck(m_RZTCur, "there");
				RZ_Mission_Delivery delivery = RZ_Mission_Delivery.Cast(m_RZTCur);
				if (delivery && delivery.RZT_Carry())
					RZT_ObjCheck(m_RZTCur, "carried");
			}

			// The fake player can't hold a prompt: the work sites he stands at are done for him (the
			// server's path); the others wait until the tracker sends him there
			if (RZ_WorkSites.Get() && RZT_Fake.s_Ent)
			{
				int worked = RZ_WorkSites.Get().CompleteAllFor(string.Format("mission.%1.", m_RZTCur.m_iId), RZT_Fake.ID, RZT_Fake.s_Ent.GetOrigin(), 25);
				if (worked > 0)
					Print(string.Format("RZT work sites: %1 done for '%2'", worked, m_RZTCur.m_sTitle));
			}

			// More steps done by being there (points to visit): on to each place the tracker gives,
			// phase 4 again 3 s later
			if (m_RZTCur.RZT_MoreSteps() && m_iRZTWalks < 24)
			{
				m_iRZTWalks++;
				RZT_Obj.Go(m_GM, m_RZTCur.m_iId, vector.Zero);
				m_iRZTPhase = 3;
			}
			return true;
		}
		if (m_iRZTPhase >= 5)
		{
			if (m_RZTCur && m_RZTCur.m_eState == RZ_EMissionState.ACTIVE)
			{
				if (m_iRZTNext % 2 == 0)
				{
					m_RZTCur.m_eState = RZ_EMissionState.SUCCEEDED;
					m_iRZTWon++;
				}
				else
					m_RZTCur.m_eState = RZ_EMissionState.FAILED;
			}
			else if (m_RZTCur)
				Print(string.Format("RZT mission '%1' ended by itself: state %2", m_RZTCur.m_sTitle, typename.EnumToString(RZ_EMissionState, m_RZTCur.m_eState)));
			m_RZTCur = null;
			m_iRZTPhase = 0;
		}
		return true;
	}

	//! The talk screen's offer path (docs/DIALOGUE.md, "Jobs on the first replies"): the giver's job
	//! rows on the first open list this contract, open for an eligible tester or locked with its need,
	//! and "I'll do it." takes it at once (RZ_TalkJobs.Take: hired, with the start card). A leader's
	//! own job is listed by the leader only, so it isn't looked for here.
	void RZT_TalkOffer(RZ_Mission m)
	{
		if (!m || !m.m_bHasGiver || m.m_bInPersonOnly || !RZT_Fake.s_Ent)
			return;

		array<int> ids = {};
		array<ref RZ_TalkReply> rows = {};
		RZ_TalkJobs.Build(m_GM, RZT_Fake.ID, RZT_Fake.s_Ent, m.m_Req.m_eGiver, null, ids, rows);
		int at = ids.Find(m.m_iId);
		if (at < 0)
		{
			Print(string.Format("RZT talk offer %1 '%2': NOT LISTED at its giver (%3 rows, eligible=%4, need '%5')", m.Type().ToString(), m.m_sTitle, rows.Count(), m.IsEligible(m_GM, RZT_Fake.ID), m.LockReason(m_GM, RZT_Fake.ID)));
			return;
		}

		RZ_TalkReply row = rows[at];
		if (!row.m_bEnabled)
		{
			Print(string.Format("RZT talk offer %1 '%2': locked %3 (%4)", m.Type().ToString(), m.m_sTitle, row.m_sTag, row.m_sNeed));
			return;
		}

		RZ_TalkJobs.Take(m_GM, RZT_Fake.ID, m.m_iId, true, "RZT giver", m.m_Req.m_eGiver, "RZT giver", RZT_Fake.s_Ent);
		Print(string.Format("RZT talk offer %1 '%2': %3 listed and taken in one conversation=%4", m.Type().ToString(), m.m_sTitle, row.m_sTag, m.IsOfferedTo(m_GM, RZT_Fake.ID)));
	}

	//! While a questline waits for its stage mission: every other mission (not a questline's stage)
	//! ends, so a contract the manager put on the board can't hold the stage's town for the hour it
	//! lasts (a real stage waits hours before it gives up; the test's waits are scaled down).
	int RZT_ClearOthers()
	{
		int count = 0;
		foreach (RZ_Mission m : m_aActive)
		{
			if (m.m_iQuestRun < 0 && m.m_eState == RZ_EMissionState.ACTIVE)
			{
				m.m_eState = RZ_EMissionState.FAILED;
				count++;
			}
		}
		return count;
	}

	//! Why a mission didn't start, as far as the test can tell from outside its Setup: what it
	//! claims and who holds it, what else runs, the towns a Setup picks from, the packs.
	void RZT_WhyNot(RZ_Mission m)
	{
		RZ_WorldFacts facts = m_GM.GetFacts();
		array<string> subjects = {};
		m.GetClaims(m_GM, subjects);
		string held = "";
		foreach (string subject : subjects)
		{
			if (facts.IsClaimed(subject))
				held += string.Format(" %1 (by %2)", subject, facts.ClaimedBy(subject));
		}

		if (held.IsEmpty())
			held = " none";

		int active = 0;
		foreach (RZ_Mission other : m_aActive)
		{
			if (other.m_eState == RZ_EMissionState.ACTIVE)
				active++;
		}

		int runs = 0;
		if (m_GM.GetQuestlines())
		{
			foreach (RZ_QuestRun run : m_GM.GetQuestlines().GetRuns())
			{
				if (!run.m_bDone)
					runs++;
			}
		}

		int heldTowns = 0;
		int busy = 0;
		int openTowns = 0;
		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			if (facts.TownOwner(i).IsHeld())
				heldTowns++;
			else if (IsSettlementBusy(i, m))
				busy++;
			else
				openTowns++;
		}

		int packs = -1;
		if (m_GM.GetTribes())
			packs = m_GM.GetTribes().GetTribeList().Count();

		Print(string.Format("RZT mission %1 why: claims held:%2; %3 other missions active, %4 questlines running; towns %5 held, %6 busy, %7 free; packs %8; weight %9",
			m.Type().ToString(), held, active, runs, heldTowns, busy, openTowns, packs, m.GetWeight(m_GM)));
	}

	//! The tracker's place for this mission's current objective against where its step is done.
	void RZT_ObjCheck(RZ_Mission m, string when)
	{
		array<vector> truths = {};
		float radius = m.RZT_Truths(m_GM, truths);
		RZT_Obj.Check(m_GM, m.m_iId, m.Type().ToString(), when, truths, radius);
	}

	string RZT_Summary()
	{
		int past = 0;
		RZ_PlayerProfile tester = m_GM.FindProfile(RZT_Fake.ID);
		if (tester && tester.m_aPastTitle)
			past = tester.m_aPastTitle.Count();

		return string.Format("%1 of %2 started, %3 marked won, %4 past jobs in the tester's Journal", m_iRZTStarted, m_aTypes.Count(), m_iRZTWon, past);
	}
	int RZT_TypeCount() { return m_aTypes.Count(); }
	int RZT_Active() { return m_aActive.Count(); }
}

modded class RZ_Questlines
{
	//! The questlines this phase runs: not the majors-only ones (Every Name, Proof), which start only
	//! with the majors on; the majors' phase runs them (RZT_Majors.c).
	void RZT_Keys(notnull array<string> keys)
	{
		foreach (RZ_Questline line : m_aLines)
		{
			if (line.IsMajor())
			{
				Print(string.Format("RZT quest %1 left to the majors' phase (a major only)", line.m_sKey));
				continue;
			}

			keys.Insert(line.m_sKey);
		}
	}
}

modded class RZ_GameModeComponent
{
	protected int m_iRZTSec;
	protected ref array<vector> m_aRZTRoute = {};
	protected ref array<string> m_aRZTRouteName = {};
	protected int m_iRZTStep;
	protected bool m_bRZTMissionsDone;
	protected bool m_bRZTQuestsDone;
	protected bool m_bRZTMajorsDone;
	protected bool m_bRZTFinaleDone;
	protected bool m_bRZTEpilogueDone;
	protected ref array<string> m_aRZTQuestKeys;
	protected int m_iRZTQuest = -1;
	protected int m_iRZTQuestSec;
	protected int m_iRZTQuestStage = -2;
	protected int m_iRZTQuestMission = -1;
	protected int m_iRZTQuestMissionSec;
	protected int m_iRZTQuestWaitSec;
	protected int m_iRZTQuestEnded;
	protected int m_iRZTQuestStages;
	protected bool m_bRZTDeathTest;
	protected int m_iRZTDeadFor;
	protected int m_iRZTDeaths;
	protected int m_iRZTDoneAt;

	//! Why a questline's stage mission doesn't start: the stage isn't ready, the manager's check
	//! (requirements, claims), or else its Setup (one line: "RZT quest <key> stage <n> waits: ...").
	protected void RZT_WhyStageWaits(string qkey, RZ_QuestRun run)
	{
		RZ_QuestStage st = run.m_Line.GetStage(run.m_iStage);
		if (!st || !st.m_bMission)
			return;

		string why;
		if (!run.m_Line.IsReady(this, run, st.m_iId))
			why = "the stage isn't ready (IsReady)";
		else
		{
			RZ_Mission probe = run.m_Line.MakeMission(this, run, st.m_iId);
			if (!probe)
				why = "no mission made";
			else
			{
				probe.m_iQuestRun = run.m_iId;
				probe.m_sClaimParent = run.Owner();
				probe.Configure(this);
				why = GetMissions().WhyNot(probe, 1, false);
				if (why.IsEmpty())
					why = "its Setup said no";
			}
		}

		Print(string.Format("RZT quest %1 stage %2 waits: %3; claims held:%4", qkey, run.m_iStage, why, GetFacts().RZT_Claims()));
	}

	//! A stand-in for an optional prefab slot the test world leaves empty (test only). True if the
	//! slot was empty and now holds `prefab`.
	bool RZT_StandIn(string slot, ResourceName prefab)
	{
		if (slot == "keypad" && m_sKeypadPrefab.IsEmpty())
			m_sKeypadPrefab = prefab;
		else if (slot == "nest" && m_sNestPrefab.IsEmpty())
			m_sNestPrefab = prefab;
		else if (slot == "deaddrop" && m_sDeadDropPrefab.IsEmpty())
			m_sDeadDropPrefab = prefab;
		else if (slot == "heliwreck" && m_sHeliWreckPrefab.IsEmpty())
			m_sHeliWreckPrefab = prefab;
		else if (slot == "fueldepot" && m_sFuelDepotPrefab.IsEmpty())
			m_sFuelDepotPrefab = prefab;
		else if (slot == "relay" && m_sRelayPrefab.IsEmpty())
			m_sRelayPrefab = prefab;
		else if (slot == "trader" && m_sTraderPrefab.IsEmpty())
			m_sTraderPrefab = prefab;
		else if (slot == "landmark" && m_sLandmarkPrefab.IsEmpty())
			m_sLandmarkPrefab = prefab;
		else if (slot == "scale" && m_sScaleCratePrefab.IsEmpty())
			m_sScaleCratePrefab = prefab;
		else
			return false;

		return true;
	}

	override void NotifyPlayer(int playerId, string title, string text)
	{
		if (playerId == RZT_Fake.ID)
		{
			text.Replace("\n", " / ");
			Print(string.Format("RZT notify [%1] %2", title, text));
		}
		super.NotifyPlayer(playerId, title, text);
	}

	override void OnGameModeStart()
	{
		super.OnGameModeStart();
		RZT_Majors.Off(this);
		for (int i = 0; i < GetSettlements().Count(); i++)
		{
			if (i % 3 == 1)
				GetSettlements()[i].m_fInfection = 75;
			else if (i % 3 == 2)
				GetSettlements()[i].m_fInfection = 95;
		}
		// The route: every settlement, the safehouses, the lab, then the Perimeter
		foreach (RZ_Settlement st : GetSettlements())
		{
			m_aRZTRoute.Insert(st.m_vCenter);
			m_aRZTRouteName.Insert(st.m_sName);
		}
		for (int h = 0; h < GetSafehouses().Count(); h++)
		{
			m_aRZTRoute.Insert(GetSafehouses().Get(h).m_vPos);
			m_aRZTRouteName.Insert("safehouse " + h.ToString());
		}
		if (HasLab())
		{
			m_aRZTRoute.Insert(GetLabPosition());
			m_aRZTRouteName.Insert("lab");
		}
		vector mins, maxs;
		GetGame().GetWorld().GetBoundBox(mins, maxs);
		Print(string.Format("RZT start: %1 mission types, route of %2 stops", GetMissions().RZT_TypeCount(), m_aRZTRoute.Count()));
		GetGame().GetCallqueue().CallLater(RZT_Tick, 1000, true);
	}

	// Questlines, one at a time: force what its CanStart needs (RZT_Prep), begin it, fast-forward its
	// waits, go to each stage mission and end it as won (with the outcome that moves the story on,
	// where "done" alone would loop), until it ends, fizzles or is stuck for 15 minutes.
	void RZT_QuestTick()
	{
		RZ_Questlines q = GetQuestlines();
		if (!q || !q.IsEnabled())
		{
			Print("RZT questlines: off");
			m_bRZTQuestsDone = true;
			return;
		}
		if (!m_aRZTQuestKeys)
		{
			m_aRZTQuestKeys = {};
			q.RZT_Keys(m_aRZTQuestKeys);
			RZT_Prep.Order(m_aRZTQuestKeys);
			RZT_Prep.BeforeQuestlines(this);
			Print(string.Format("RZT questlines: %1 registered", m_aRZTQuestKeys.Count()));
		}
		if (m_iRZTQuest < 0 || m_iRZTQuestSec > 900)
		{
			if (m_iRZTQuest >= 0)
			{
				string prev = m_aRZTQuestKeys[m_iRZTQuest];
				if (m_iRZTQuestSec < 999)
				{
					Print(string.Format("RZT quest %1 STUCK at stage %2 after 15 min", prev, m_iRZTQuestStage));
					q.RZT_End(prev);
				}
				RZT_Prep.AfterQuest(this, prev);
			}
			m_iRZTQuest++;
			m_iRZTQuestSec = 0;
			m_iRZTQuestStage = -2;
			m_iRZTQuestMission = -1;
			if (m_iRZTQuest >= m_aRZTQuestKeys.Count())
			{
				Print(string.Format("RZT all questlines tried: %1 of %2 reached an ending, %3 stages entered", m_iRZTQuestEnded, m_aRZTQuestKeys.Count(), m_iRZTQuestStages));
				Print("RZT objective places (missions and questlines): " + RZT_Obj.Summary());
				Print("RZT person objectives: " + RZT_PersonObj.Summary());
				Print("RZT rules: " + RZT_Rules.Summary());
				m_bRZTQuestsDone = true;
				return;
			}
			string key = m_aRZTQuestKeys[m_iRZTQuest];
			RZT_Prep.BeforeQuest(this, key);
			bool began = q.Begin(key);
			RZT_Prep.AfterBegin(this, key);
			if (!began && !q.FindRunByKey(key))
			{
				Print(string.Format("RZT quest %1 did not start (CanStart false)", key));
				RZT_Prep.WhyNotQuest(this, key);
				m_iRZTQuestSec = 999;
				return;
			}
			Print(string.Format("RZT quest %1 started", key));
		}
		m_iRZTQuestSec++;
		string qkey = m_aRZTQuestKeys[m_iRZTQuest];
		RZ_QuestRun run = q.FindRunByKey(qkey);
		if (!run || run.m_bDone)
		{
			int ending = q.WorldFlag(qkey + ".ending", -1);
			if (ending >= 0)
			{
				m_iRZTQuestEnded++;
				Print(string.Format("RZT quest %1 ENDED: ending %2", qkey, ending));
			}
			else
			{
				string said = "";
				if (run)
					said = run.m_sEndText;

				Print(string.Format("RZT quest %1 fizzled or was dropped at stage %2: '%3'", qkey, m_iRZTQuestStage, said));
				Print(string.Format("RZT quest %1 claims held then:%2", qkey, GetFacts().RZT_Claims()));
			}
			m_iRZTQuestSec = 999;
			return;
		}
		RZT_Prep.Tick(this, qkey, run);
		if (run.m_iStage != m_iRZTQuestStage)
		{
			m_iRZTQuestStage = run.m_iStage;
			m_iRZTQuestStages++;
			Print(string.Format("RZT quest %1 stage %2", qkey, run.m_iStage));
		}
		if (run.m_iMissionId < 0)
		{
			// Nothing else on the board holds the stage's town (RZ_MissionManager.RZT_ClearOthers)
			int cleared = GetMissions().RZT_ClearOthers();
			if (cleared > 0)
				Print(string.Format("RZT forced: %1 other mission(s) ended while quest %2 waits for its stage", cleared, qkey));
			// Waiting: move the clocks back so the wait is over (test only)
			run.m_fStageAt = run.m_fStageAt - 1800;
			run.m_iStageUnix = run.m_iStageUnix - 43200;
			run.m_fNextTry = 0;
			m_iRZTQuestMission = -1;
			// Every minute it waits for a stage mission: why the stage's mission doesn't start
			m_iRZTQuestWaitSec++;
			if (m_iRZTQuestWaitSec % 60 == 0)
				RZT_WhyStageWaits(qkey, run);
			return;
		}
		m_iRZTQuestWaitSec = 0;
		RZ_Mission m = GetMissions().FindById(run.m_iMissionId);
		if (!m)
			return;
		if (m.m_iId != m_iRZTQuestMission)
		{
			m_iRZTQuestMission = m.m_iId;
			m_iRZTQuestMissionSec = 0;
			Print(string.Format("RZT quest %1 stage %2 mission '%3'", qkey, run.m_iStage, m.m_sTitle));
			// The stage is offered in person (its giver, or the story he's part of already)
			m.Hire(RZT_Fake.ID);
			// The questline's row points where the stage's current objective is done (RZT_ObjPlace.c)
			if (!m.m_bGlobal && m.m_vPos != vector.Zero)
			{
				array<vector> truths = {};
				float radius = m.RZT_Truths(this, truths);
				RZT_Obj.Check(this, -run.m_iId, qkey + " stage " + run.m_iStage.ToString(), "stage", truths, radius);
			}
		}
		m_iRZTQuestMissionSec++;
		if (!m.m_bGlobal && m.m_vPos != vector.Zero)
			RZT_Obj.Go(this, -run.m_iId, m.m_vPos);
		if (m_iRZTQuestMissionSec >= 10 && m.m_eState == RZ_EMissionState.ACTIVE)
		{
			int forced = RZT_Prep.Outcome(qkey, run.m_iStage);
			if (forced >= 0)
				run.SetOutcome(forced);
			m.m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	void RZT_VoiceCycle()
	{
		int since = m_iRZTSec - 120;
		if (since < 0 || since > 1800)
			return;

		int step = since / 300 + 1;
		if (since % 300 == 0)
		{
			if (step <= 5)
				RZ_VoiceSettings.Get().m_iForceMood = step;
			else
				RZ_VoiceSettings.Get().m_iForceMood = 0;
		}

		if (since % 300 != 290 || step > 5 || !GetFacts())
			return;

		vector at = vector.Zero;
		if (RZT_Fake.s_Ent)
			at = RZT_Fake.s_Ent.GetOrigin();

		for (int f = 0; f < 4; f++)
		{
			string radio = RZ_VoiceBank.PickShared(this, "radio_tail", f);
			string face = RZ_VoiceBank.PickFor(this, "opener", f, "warm", null, RZT_Fake.ID, at);
			Print(string.Format("RZT voice %1 faction %2: radio [%3] face [%4]", RZ_SeasonMood.Name(GetFacts().SeasonMood()), f, radio, face));
		}
	}

	void RZT_Tick()
	{
		m_iRZTSec++;
		if (m_iRZTSec > 5 && !m_bRZTDeathTest && RZT_Fake.s_Ent && !RZ_Util.IsCharacterAlive(RZT_Fake.s_Ent))
		{
			m_iRZTDeadFor++;
			if (m_iRZTDeadFor >= 5)
			{
				m_iRZTDeadFor = 0;
				m_iRZTDeaths++;
				vector at = RZT_Fake.s_Ent.GetOrigin();
				Print(string.Format("RZT fake player died at %1 (death %2); respawning", at, m_iRZTDeaths));
				IEntity again = RZT_Fake.Spawn(m_aRZTRoute[0]);
				HandlePlayerSpawned(RZT_Fake.ID, again);
			}
		}
		if (m_iRZTSec == 3)
		{
			IEntity e = RZT_Fake.Spawn(m_aRZTRoute[0]);
			Print("RZT spawned fake player at " + m_aRZTRouteName[0]);
			HandlePlayerSpawned(RZT_Fake.ID, e);
		}
		// Every conversation's nodes, once, for the fake player; a lie, a promise and a callback
		if (m_iRZTSec == 8 && RZ_DialogueTrees.Get())
			Print("RZT " + RZ_DialogueTrees.Get().RunSelfTest(RZT_Fake.ID));
		// Reputation's cells, the credit rule and the budget, on the fake player (put back after)
		if (m_iRZTSec == 9)
			Print("RZT " + RZ_RepGates.SelfTest(this, RZT_Fake.ID));
		// The power station's looks: the dressing, the generator's sound, the street lamps' list
		if (m_iRZTSec == 10 && GetPowerGrid() && GetPowerGrid().IsActive())
			Print("RZT " + GetPowerGrid().RZT_SelfTest());

		// The voice: Voice > Force mood cycles the five moods (a mood check every 5 minutes), and each
		// one's picks are printed before the next, so the in-game picker can be read against voice_sim
		RZT_VoiceCycle();
		// A fresh spawn (RZT_Fresh.c): the contact, step 1, and no job left from the last life
		if (m_iRZTSec >= 11)
			RZT_Fresh.Tick(this);

		if (m_iRZTSec >= 60 && m_iRZTSec % 20 == 0)
			RZT_Fresh.PartTick(this);

		// The openings' play-test faults (RZT_Openings.c), before the missions move the tester about
		if (m_iRZTSec >= 12 && !RZT_Openings.Tick(this, m_aRZTRoute[0]))
			return;

		// The opening's door and favour (RZT_Favour.c): who you were, the habit, the fetch, the
		// hand-over by reply, your background's way, for every background
		if (m_iRZTSec >= 13)
			RZT_Favour.Tick(this);

		// The runner's case (RZT_Case.c): found, carried, kept over a death, handed to Halcyon by reply
		if (m_iRZTSec >= 12 && !RZT_Case.Tick(this, m_aRZTRoute[0]))
			return;

		if (m_iRZTSec > 5 && m_iRZTSec % 3 == 0 && !m_bRZTMissionsDone)
		{
			if (!GetMissions().RZT_Step())
			{
				m_bRZTMissionsDone = true;
				Print("RZT all mission types tried: " + GetMissions().RZT_Summary());
				Print("RZT objective places (missions): " + RZT_Obj.Summary());
			}
		}
		if (!m_bRZTMissionsDone)
			return;
		if (!m_bRZTQuestsDone)
		{
			RZT_QuestTick();
			return;
		}
		// The majors' phase (RZT_Majors.c): their acts, then the fallback checks
		if (!m_bRZTMajorsDone)
		{
			m_bRZTMajorsDone = RZT_Majors.Tick(this);
			if (m_bRZTMajorsDone)
				m_iRZTDoneAt = m_iRZTSec;
			return;
		}
		int rel = m_iRZTSec - m_iRZTDoneAt;
		// Then every 20 s the next stop on the route
		if (rel > 0 && rel % 20 == 0 && m_iRZTStep < m_aRZTRoute.Count())
		{
			m_iRZTStep++;
			if (m_iRZTStep < m_aRZTRoute.Count())
			{
				RZT_Fake.MoveTo(m_aRZTRoute[m_iRZTStep]);
				Print("RZT moved to " + m_aRZTRouteName[m_iRZTStep]);
			}
			else
			{
				// The Perimeter: the warning zone, then the kill zone (west edge, 200 m then 50 m in from x 70)
				RZT_Fake.s_bLineTest = true;
				RZT_Fake.MoveTo(Vector(270, 0, 2000));
				Print("RZT moved to the Perimeter warning zone");
			}
		}
		if (rel == 20 * 17 + 60)
		{
			m_bRZTDeathTest = true;
			RZT_Fake.MoveTo(Vector(110, 0, 2000));
			Print("RZT moved into the Perimeter kill zone");
		}
		if (rel == 20 * 17 + 120)
		{
			if (RZ_Util.IsCharacterAlive(RZT_Fake.s_Ent))
			{
				Print("RZT killing the fake player");
				RZT_Fake.Kill();
			}
			else
				Print("RZT fake player already dead (the Perimeter got him)");
		}
		if (rel == 20 * 17 + 150)
		{
			IEntity e2 = RZT_Fake.Spawn(m_aRZTRoute[1]);
			Print("RZT respawned fake player (second life)");
			HandlePlayerSpawned(RZT_Fake.ID, e2);
		}
		// Last, the cure finale's phase (RZT_Finale.c): it ends with the cure, which stands the line down
		if (rel > 20 * 17 + 160 && !m_bRZTFinaleDone)
			m_bRZTFinaleDone = RZT_Finale.Tick(this);
		// Then the epilogue week at Test scale 0.02 (RZT_Epilogue.c): every closing item to its end
		else if (m_bRZTFinaleDone && !m_bRZTEpilogueDone)
			m_bRZTEpilogueDone = RZT_Epilogue.Tick(this);
		if (m_iRZTSec % 60 == 0)
			Print(string.Format("RZT t=%1 active missions=%2 alive=%3", m_iRZTSec, GetMissions().RZT_Active(), RZ_Util.IsCharacterAlive(RZT_Fake.s_Ent)));
	}
}

modded class RZ_WorldFacts
{
	//! Every claim held now, " subject (by owner)" each (the questlines' diagnostics).
	string RZT_Claims()
	{
		string all = "";
		foreach (string subject, string owner : m_mClaims)
		{
			all += string.Format(" %1 (by %2)", subject, owner);
		}

		if (all.IsEmpty())
			return " none";

		return all;
	}
}
