// Test-only harness (not part of the mod).
modded class RZ_MissionManager
{
	protected int m_iRZTNext;
	protected int m_iRZTPhase;
	protected ref RZ_Mission m_RZTCur;
	protected int m_iRZTStarted;
	protected int m_iRZTWon;

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
				tries++;
				m_RZTCur = RZ_Mission.Cast(t.Spawn());
				if (!m_RZTCur)
					break;
				m_RZTCur.Configure(m_GM);
				ok = StartMission(m_RZTCur);
			}
			if (!m_RZTCur)
			{
				Print("RZT mission " + t.ToString() + " SPAWN FAILED");
				m_iRZTPhase = 0;
				return true;
			}
			if (ok)
				m_iRZTStarted++;
			Print(string.Format("RZT mission %1 '%2' contract=%3 started=%4 tries=%5", t.ToString(), m_RZTCur.m_sTitle, m_RZTCur.m_bContract, ok, tries));
			if (!ok)
			{
				m_RZTCur = null;
				m_iRZTPhase = 0;
			}
			return true;
		}
		if (m_iRZTPhase == 3)
		{
			if (m_RZTCur && m_RZTCur.m_eState == RZ_EMissionState.ACTIVE && !m_RZTCur.m_bGlobal && m_RZTCur.m_vPos != vector.Zero)
				RZT_Fake.MoveTo(m_RZTCur.m_vPos);
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

	string RZT_Summary() { return string.Format("%1 of %2 started, %3 marked won", m_iRZTStarted, m_aTypes.Count(), m_iRZTWon); }
	int RZT_TypeCount() { return m_aTypes.Count(); }
	int RZT_Active() { return m_aActive.Count(); }
}

modded class RZ_Questlines
{
	void RZT_Keys(notnull array<string> keys)
	{
		foreach (RZ_Questline line : m_aLines)
			keys.Insert(line.m_sKey);
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
	protected ref array<string> m_aRZTQuestKeys;
	protected int m_iRZTQuest = -1;
	protected int m_iRZTQuestSec;
	protected int m_iRZTQuestStage = -2;
	protected int m_iRZTQuestMission = -1;
	protected int m_iRZTQuestMissionSec;
	protected int m_iRZTQuestEnded;
	protected int m_iRZTQuestStages;
	protected bool m_bRZTDeathTest;
	protected int m_iRZTDeadFor;
	protected int m_iRZTDeaths;
	protected int m_iRZTDoneAt;

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
				Print(string.Format("RZT quest %1 fizzled or was dropped at stage %2", qkey, m_iRZTQuestStage));
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
			return;
		}
		RZ_Mission m = GetMissions().FindById(run.m_iMissionId);
		if (!m)
			return;
		if (m.m_iId != m_iRZTQuestMission)
		{
			m_iRZTQuestMission = m.m_iId;
			m_iRZTQuestMissionSec = 0;
			Print(string.Format("RZT quest %1 stage %2 mission '%3'", qkey, run.m_iStage, m.m_sTitle));
		}
		m_iRZTQuestMissionSec++;
		if (!m.m_bGlobal && m.m_vPos != vector.Zero)
			RZT_Fake.MoveTo(m.m_vPos);
		if (m_iRZTQuestMissionSec >= 10 && m.m_eState == RZ_EMissionState.ACTIVE)
		{
			int forced = RZT_Prep.Outcome(qkey, run.m_iStage);
			if (forced >= 0)
				run.SetOutcome(forced);
			m.m_eState = RZ_EMissionState.SUCCEEDED;
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
		if (m_iRZTSec > 5 && m_iRZTSec % 3 == 0 && !m_bRZTMissionsDone)
		{
			if (!GetMissions().RZT_Step())
			{
				m_bRZTMissionsDone = true;
				Print("RZT all mission types tried: " + GetMissions().RZT_Summary());
			}
		}
		if (!m_bRZTMissionsDone)
			return;
		if (!m_bRZTQuestsDone)
		{
			RZT_QuestTick();
			if (m_bRZTQuestsDone)
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
		if (m_iRZTSec % 60 == 0)
			Print(string.Format("RZT t=%1 active missions=%2 alive=%3", m_iRZTSec, GetMissions().RZT_Active(), RZ_Util.IsCharacterAlive(RZT_Fake.s_Ent)));
	}
}
