// Test-only (not part of the mod): the experiment base, the game clock, runtime groups and the
// waypoint leak.
class RZL_Exp
{
	string m_sName;
	float m_fStartAt;
	bool m_bStarted;
	bool m_bDone;
	ref RZL_Site m_Site;
	ref array<IEntity> m_aChars = {};
	ref array<IEntity> m_aGroups = {};
	protected float m_fBegin;
	protected int m_iStep;
	protected float m_fStepAt;

	//------------------------------------------------------------------------------------------------
	void Begin(float now)
	{
		m_bStarted = true;
		m_fBegin = now;
		m_fStepAt = now;
		string where = "global";
		if (m_Site)
			where = string.Format("%1 (%2, %3)", m_Site.m_sName, RZL.F(m_Site.m_vT[0]), RZL.F(m_Site.m_vT[2]));
		RZL.Log(string.Format("%1 begins at t=%2 s, site %3", m_sName, RZL.F(now), where));
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float now)
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void Next(float now)
	{
		m_iStep++;
		m_fStepAt = now;
	}

	//------------------------------------------------------------------------------------------------
	protected float Since(float now)
	{
		return now - m_fStepAt;
	}

	//------------------------------------------------------------------------------------------------
	protected void Finish()
	{
		RZL.DelAll(m_aChars, m_aGroups);
		m_bDone = true;
		RZL.Log(m_sName + " done");
	}
}

//------------------------------------------------------------------------------------------------
//! 7. The game clock: its rate, acceleration by SetDayDuration, and (with run.sh's second boot)
//! whether it survives a restart. Leaves the clock frozen at noon for the perception tests.
class RZL_ExpClock : RZL_Exp
{
	protected float m_fTod0;
	protected int m_iReal0;
	protected float m_fDur0;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpClock()
	{
		m_sName = "clock";
	}

	//------------------------------------------------------------------------------------------------
	static string State(string what)
	{
		TimeAndWeatherManagerEntity tm = RZL.Weather();
		if (!tm)
			return "what=" + what + "|error=no TimeAndWeatherManager";

		int y;
		int mo;
		int dd;
		tm.GetDate(y, mo, dd);
		string handler = "no";
		SCR_TimeAndWeatherHandlerComponent h = SCR_TimeAndWeatherHandlerComponent.GetInstance();
		if (h)
			handler = "yes";
		string header = "?";
		SCR_MissionHeader mh = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
		if (mh)
			header = string.Format("override=%1 day=%2 night=%3", mh.m_bOverrideScenarioTimeAndWeather, RZL.F2(mh.m_fDayTimeAcceleration), RZL.F2(mh.m_fNightTimeAcceleration));
		return string.Format("what=%1|tod=%2|dayDuration=%3|autoAdvance=%4|date=%5-%6-%7|handlerComponent=%8|header=%9", what, RZL.F2(tm.GetTimeOfTheDay()), RZL.F(tm.GetDayDuration()), tm.GetIsDayAutoAdvanced(), y, mo, dd, handler, header);
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		TimeAndWeatherManagerEntity tm = RZL.Weather();
		if (!tm)
		{
			RZL.Row("clock", State("start"));
			m_bDone = true;
			return;
		}

		if (m_iStep == 0)
		{
			RZL.Row("clock", State("at start"));
			m_fDur0 = tm.GetDayDuration();
			tm.SetTimeOfTheDay(12, true);
			m_fTod0 = tm.GetTimeOfTheDay();
			m_iReal0 = System.GetTickCount();
			Next(now);
			return;
		}

		int realMs = System.GetTickCount() - m_iReal0;
		if ((m_iStep == 1 || m_iStep == 2) && realMs >= 60000)
		{
			float tod1 = tm.GetTimeOfTheDay();
			float realSec = realMs * 0.001;
			float gameSec = (tod1 - m_fTod0) * 3600;
			string what = "default rate";
			if (m_iStep == 2)
				what = "after SetDayDuration(7200)";
			RZL.Row("clock", string.Format("what=%1|realSec=%2|gameSec=%3|accel=%4|dayDuration=%5", what, RZL.F(realSec), RZL.F(gameSec), RZL.F2(gameSec / realSec), RZL.F(tm.GetDayDuration())));
			if (m_iStep == 1)
			{
				bool ok = tm.SetDayDuration(7200);
				RZL.Log("clock: SetDayDuration(7200) returned " + ok.ToString());
			}
			else
			{
				tm.SetDayDuration(m_fDur0);
				tm.SetTimeOfTheDay(12, true);
				tm.SetIsDayAutoAdvanced(false);
				RZL.Row("clock", State("frozen at noon for the tests"));
				m_bDone = true;
			}
			m_fTod0 = tm.GetTimeOfTheDay();
			m_iReal0 = System.GetTickCount();
			Next(now);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! 1. Runtime groups (design VERIFY 1): an empty Bacon group filled with AddAIEntityToGroup, a
//! one-member group, waypoints, survival, deletion when empty; and a Bacon group prefab for comparison.
class RZL_ExpGroups : RZL_Exp
{
	protected SCR_AIGroup m_G1;
	protected SCR_AIGroup m_G2;
	protected SCR_AIGroup m_G3;
	protected ref array<IEntity> m_aG1 = {};
	protected IEntity m_Solo;
	protected AIWaypoint m_Wp1;
	protected AIWaypoint m_Wp2;
	protected vector m_vWp1;
	protected vector m_vWp2;
	protected vector m_vG1Start;
	protected vector m_vG2Start;
	protected float m_fArrive1 = -1;
	protected float m_fDone1 = -1;
	protected float m_fArrive2 = -1;
	protected float m_fDone2 = -1;
	protected int m_iKilled;
	protected float m_fKillAt;
	protected bool m_bKillLogged;
	protected bool m_bSoloLogged;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpGroups()
	{
		m_sName = "groups";
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		float s = Since(now);
		if (m_iStep == 0)
		{
			m_G1 = RZL.SpawnEmptyGroup(m_Site.m_vT);
			m_G2 = RZL.SpawnEmptyGroup(m_Site.Side(0, 120));
			m_aGroups.Insert(m_G1);
			m_aGroups.Insert(m_G2);
			RZL.Log(string.Format("groups: empty groups spawned: %1 %2 agents=%3", m_G1 != null, m_G2 != null, m_G1.GetAgentsCount()));
			Next(now);
			return;
		}

		if (m_iStep == 1 && s >= 20)
		{
			// Does an empty group delete itself before anything is added?
			int a20 = -1;
			if (m_G1)
				a20 = m_G1.GetAgentsCount();
			RZL.Row("groups", string.Format("case=empty group, 20 s after spawn|exists=%1|agents=%2", m_G1 != null, a20));
			if (!m_G1 || !m_G2)
			{
				m_bDone = true;
				return;
			}

			array<string> prefabs = {RZL.CIV, RZL.CIV, RZL.MIL, RZL.HEAVY};
			for (int i = 0; i < prefabs.Count(); i++)
			{
				IEntity ch = RZL.Spawn(prefabs[i], m_Site.Side(3 * i, 2), m_Site.m_fYaw);
				bool before = RZL.AIActive(ch);
				bool ok = m_G1.AddAIEntityToGroup(ch);
				RZL.Row("groupsAdd", string.Format("prefab=%1|added=%2|aiActiveBefore=%3|aiActiveAfter=%4|faction=%5|lodNatural=%6", i, ok, before, RZL.AIActive(ch), RZL.FactionKey(ch), RZL.LOD(ch)));
				m_aG1.Insert(ch);
				m_aChars.Insert(ch);
			}

			m_Solo = RZL.Spawn(RZL.CIV, m_Site.Side(0, 120), m_Site.m_fYaw);
			bool okSolo = m_G2.AddAIEntityToGroup(m_Solo);
			m_aChars.Insert(m_Solo);
			RZL.Log("groups: one-member group added=" + okSolo.ToString());
			Next(now);
			return;
		}

		if (m_iStep == 2 && s >= 3)
		{
			IEntity leader = m_G1.GetLeaderEntity();
			string lods = "";
			foreach (IEntity m : m_aG1)
			{
				lods += RZL.LOD(m).ToString() + " ";
				RZL.ForceLOD(m, 0);
			}
			RZL.ForceLOD(m_Solo, 0);
			RZL.Row("groups", string.Format("case=filled runtime group|agents=%1|leader=%2|groupFaction=%3|naturalLOD=%4|soloAgents=%5", m_G1.GetAgentsCount(), leader != null, m_G1.GetFactionName(), lods, m_G2.GetAgentsCount()));
			m_vWp1 = m_Site.Along(150);
			m_vWp2 = m_Site.Side(100, 120);
			m_vG1Start = RZL.GroupPos(m_G1);
			m_vG2Start = RZL.GroupPos(m_G2);
			m_Wp1 = RZL.NewWaypoint(m_vWp1);
			m_Wp2 = RZL.NewWaypoint(m_vWp2);
			m_G1.AddWaypoint(m_Wp1);
			m_G2.AddWaypoint(m_Wp2);
			Next(now);
			return;
		}

		if (m_iStep == 3)
		{
			if (m_fArrive1 < 0 && RZL.DistXZ(RZL.GroupPos(m_G1), m_vWp1) < 10)
				m_fArrive1 = s;
			if (m_fDone1 < 0 && !RZL.HasWaypoint(m_G1, m_Wp1))
				m_fDone1 = s;
			if (m_fArrive2 < 0 && RZL.DistXZ(RZL.GroupPos(m_G2), m_vWp2) < 10)
				m_fArrive2 = s;
			if (m_fDone2 < 0 && !RZL.HasWaypoint(m_G2, m_Wp2))
				m_fDone2 = s;
			if (s < 120)
				return;

			RZL.Row("groups", string.Format("case=4-member runtime group, move 150 m|moved=%1|arrivedAt=%2|completedAt=%3|waypointEntityAfter=%4|agents=%5", RZL.F(RZL.DistXZ(RZL.GroupPos(m_G1), m_vG1Start)), RZL.F(m_fArrive1), RZL.F(m_fDone1), m_Wp1 != null, m_G1.GetAgentsCount()));
			RZL.Row("groups", string.Format("case=1-member runtime group, move 100 m|moved=%1|arrivedAt=%2|completedAt=%3|waypointEntityAfter=%4|agents=%5", RZL.F(RZL.DistXZ(RZL.GroupPos(m_G2), m_vG2Start)), RZL.F(m_fArrive2), RZL.F(m_fDone2), m_Wp2 != null, m_G2.GetAgentsCount()));
			RZL.Del(m_Wp1);
			RZL.Del(m_Wp2);
			Next(now);
			return;
		}

		if (m_iStep == 4)
		{
			// Survival, then kill the members one by one
			int a4 = -1;
			int a1 = -1;
			if (m_G1)
				a4 = m_G1.GetAgentsCount();
			if (m_G2)
				a1 = m_G2.GetAgentsCount();
			RZL.Row("groups", string.Format("case=alive after 2.5 min|group4Exists=%1|group4Agents=%2|soloExists=%3|soloAgents=%4", m_G1 != null, a4, m_G2 != null, a1));
			RZL.Del(m_Solo);
			Next(now);
			return;
		}

		if (m_iStep == 5)
		{
			if (s >= 2 && !m_bSoloLogged)
			{
				m_bSoloLogged = true;
				RZL.Row("groups", string.Format("case=1-member group, member deleted, 2 s later|groupExists=%1", m_G2 != null));
			}

			if (m_iKilled < m_aG1.Count() && s >= 4 + m_iKilled * 4)
			{
				RZL.Kill(m_aG1[m_iKilled]);
				m_iKilled++;
				m_fKillAt = now;
				m_bKillLogged = false;
				int count = -1;
				if (m_G1)
					count = m_G1.GetAgentsCount();
				RZL.Log(string.Format("groups: killed member %1, agents right after: %2", m_iKilled, count));
			}

			if (m_iKilled > 0 && !m_bKillLogged && now - m_fKillAt >= 1)
			{
				m_bKillLogged = true;
				int c1 = -1;
				if (m_G1)
					c1 = m_G1.GetAgentsCount();
				RZL.Row("groupsKill", string.Format("killed=%1|agents1sLater=%2|groupExists=%3|bodyExists=%4", m_iKilled, c1, m_G1 != null, m_aG1[m_iKilled - 1] != null));
			}

			if (s >= 4 + m_aG1.Count() * 4 + 6)
			{
				RZL.Row("groups", string.Format("case=4-member group, all killed, 6 s later|groupExists=%1", m_G1 != null));
				Next(now);
			}
			return;
		}

		if (m_iStep == 6)
		{
			m_G3 = SCR_AIGroup.Cast(RZL.Spawn(RZL.GROUP_CIV, m_Site.Side(60, -80), 0));
			m_aGroups.Insert(m_G3);
			Next(now);
			return;
		}

		if (m_iStep == 7 && s >= 15)
		{
			int c15 = -1;
			if (m_G3)
				c15 = m_G3.GetAgentsCount();
			RZL.Row("groups", string.Format("case=Bacon group prefab (7 slots), 15 s after spawn|agents=%1", c15));
			Next(now);
			return;
		}

		if (m_iStep == 8 && s >= 30)
		{
			int c45 = -1;
			array<AIAgent> agents = {};
			if (m_G3)
			{
				c45 = m_G3.GetAgentsCount();
				m_G3.GetAgents(agents);
			}
			string lodsG3 = "";
			foreach (AIAgent ag : agents)
			{
				lodsG3 += ag.GetLOD().ToString() + " ";
				if (ag.GetControlledEntity())
					m_aChars.Insert(ag.GetControlledEntity());
			}
			RZL.Row("groups", string.Format("case=Bacon group prefab, 45 s after spawn|agents=%1|naturalLOD=%2", c45, lodsG3));
			ZLab_DeleteUnit(m_G3);
			Next(now);
			return;
		}

		if (m_iStep == 9 && s >= 3)
		{
			int left;
			foreach (IEntity e : m_aChars)
			{
				if (e)
					left++;
			}
			RZL.Row("groups", string.Format("case=RZ_Util.DeleteUnit on the prefab group, 3 s later|groupExists=%1|charactersLeft=%2", m_G3 != null, left));
			Finish();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ZLab_DeleteUnit(IEntity e)
	{
		RZ_Util.DeleteUnit(e);
	}
}

//------------------------------------------------------------------------------------------------
//! 2b. The waypoint leak (design VERIFY 2): RZ_Util.SendGroupTo, RedirectGroup and DeleteUnit.
class RZL_ExpLeak : RZL_Exp
{
	protected SCR_AIGroup m_G;
	protected IEntity m_Ch;
	protected ref array<IEntity> m_aWps = {};
	protected AIWaypoint m_Cur;
	protected AIWaypoint m_Old;
	protected int m_iOrders;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpLeak()
	{
		m_sName = "leak";
	}

	//------------------------------------------------------------------------------------------------
	protected AIWaypoint LastWaypoint()
	{
		array<AIWaypoint> wps = {};
		m_G.GetWaypoints(wps);
		if (wps.IsEmpty())
			return null;

		return wps[wps.Count() - 1];
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		float s = Since(now);
		if (m_iStep == 0)
		{
			m_Ch = RZL.SpawnSolo(RZL.CIV, m_Site.m_vT, m_Site.m_fYaw, m_aGroups);
			m_aChars.Insert(m_Ch);
			m_G = SCR_AIGroup.Cast(m_aGroups[0]);
			Next(now);
			return;
		}

		// Three orders in turn with the mod's SendGroupTo, each after the last is complete
		if (m_iStep == 1 && s >= 2)
		{
			if (m_Cur && RZL.HasWaypoint(m_G, m_Cur) && s < 60)
				return;

			if (m_Cur)
				RZL.Log(string.Format("leak: order %1 done after %2 s, in list=%3", m_iOrders, RZL.F(s), RZL.HasWaypoint(m_G, m_Cur)));
			if (m_iOrders >= 3)
			{
				RZL.Row("leak", string.Format("case=3 SendGroupTo orders, each completed|created=3|stillInWorld=%1|inGroupList=%2", RZL.CountAlive(m_aWps), RZL.WaypointCount(m_G)));
				Next(now);
				return;
			}

			float dist = 40;
			if (m_iOrders == 1)
				dist = 80;
			RZ_Util.SendGroupTo(m_G, m_Site.Along(dist), RZL.WAYPOINT);
			m_Cur = LastWaypoint();
			m_aWps.Insert(m_Cur);
			m_iOrders++;
			m_fStepAt = now - 2;
			return;
		}

		// SendGroupTo far, then RedirectGroup before it arrives
		if (m_iStep == 2)
		{
			RZ_Util.SendGroupTo(m_G, m_Site.Along(250), RZL.WAYPOINT);
			m_Old = LastWaypoint();
			Next(now);
			return;
		}

		if (m_iStep == 3 && s >= 3)
		{
			IEntity nw = RZ_Util.RedirectGroup(m_G, m_Site.Along(200), RZL.WAYPOINT);
			m_Cur = AIWaypoint.Cast(nw);
			RZL.Row("leak", string.Format("case=RedirectGroup over a pending waypoint|oldStillInWorld=%1|newInList=%2|listCount=%3|waypointHasRplComponent=%4", m_Old != null, RZL.HasWaypoint(m_G, m_Cur), RZL.WaypointCount(m_G), m_Cur && m_Cur.FindComponent(RplComponent)));
			Next(now);
			return;
		}

		// DeleteUnit with a waypoint still pending
		if (m_iStep == 4 && s >= 3)
		{
			RZ_Util.DeleteUnit(m_G);
			Next(now);
			return;
		}

		if (m_iStep == 5 && s >= 3)
		{
			RZL.Row("leak", string.Format("case=DeleteUnit with a waypoint pending|groupExists=%1|characterExists=%2|pendingWaypointStillInWorld=%3", m_G != null, m_Ch != null, m_Cur != null));
			RZL.Del(m_Cur);
			foreach (IEntity w : m_aWps)
				RZL.Del(w);
			Finish();
		}
	}
}
