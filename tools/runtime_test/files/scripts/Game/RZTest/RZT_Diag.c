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
			m_RZTCur = RZ_Mission.Cast(t.Spawn());
			if (!m_RZTCur)
			{
				Print("RZT mission " + t.ToString() + " SPAWN FAILED");
				m_iRZTPhase = 0;
				return true;
			}
			m_RZTCur.Configure(m_GM);
			bool ok = StartMission(m_RZTCur);
			if (ok)
				m_iRZTStarted++;
			Print(string.Format("RZT mission %1 '%2' contract=%3 started=%4", t.ToString(), m_RZTCur.m_sTitle, m_RZTCur.m_bContract, ok));
			if (!ok)
			{
				m_RZTCur = null;
				m_iRZTPhase = 0;
			}
			return true;
		}
		if (m_iRZTPhase == 3)
		{
			if (m_RZTCur && m_RZTCur.m_eState == RZ_EMissionState.ACTIVE)
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

	string RZT_Summary() { return string.Format("%1 of %2 started, %3 marked won", m_iRZTStarted, m_aTypes.Count(), m_iRZTWon); }
	int RZT_TypeCount() { return m_aTypes.Count(); }
	int RZT_Active() { return m_aActive.Count(); }
}

modded class RZ_GameModeComponent
{
	protected int m_iRZTSec;
	protected ref array<vector> m_aRZTRoute = {};
	protected ref array<string> m_aRZTRouteName = {};
	protected int m_iRZTStep;
	protected bool m_bRZTMissionsDone;
	protected int m_iRZTDoneAt;

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

	void RZT_Tick()
	{
		m_iRZTSec++;
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
				m_iRZTDoneAt = m_iRZTSec;
				Print("RZT all mission types tried: " + GetMissions().RZT_Summary());
			}
		}
		if (!m_bRZTMissionsDone)
			return;
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
