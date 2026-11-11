// Test-only (not part of the mod): the zombie lab. Placed on the game mode of the lab world
// (tools/zombie_lab/files/Worlds/ZLab/ZLab.ent). Finds sites on the map, runs the experiments at
// places far apart, prints "RZL R|table|..." rows for tools/zombie_lab/report.py, then closes.
// "$profile:zlab_quick.txt" makes a short debug run; "$profile:zlab_restart.txt" only reports the
// clock after a restart (run.sh's second boot).
class RZL_LabComponentClass : ScriptComponentClass
{
}

class RZL_LabComponent : ScriptComponent
{
	static bool s_bQuick;

	protected ref array<ref RZL_Exp> m_aExps = {};
	protected ref RZL_Sites m_Sites;
	protected ref RZL_PercPlan m_Plan;
	protected ref RZL_ExpClock m_Clock;
	protected ref RZL_ExpPerf m_Perf;
	protected float m_fT0 = -1;
	protected float m_fSwitchAt;
	protected bool m_bEnding;
	protected float m_fFrameSum;
	protected float m_fFrameMax;
	protected int m_iFrames;
	protected int m_iLastFrameLog;
	protected int m_iOpenRunners;
	protected int m_iWoodRunners;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;

		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;

		GetGame().GetCallqueue().CallLater(Start, 8000, false);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		float ms = timeSlice * 1000;
		RZL_Frames.Add(ms);
		m_fFrameSum += ms;
		m_fFrameMax = Math.Max(m_fFrameMax, ms);
		m_iFrames++;
	}

	//------------------------------------------------------------------------------------------------
	protected void Close()
	{
		RZL.Log("closing");
		GetGame().RequestClose();
	}

	//------------------------------------------------------------------------------------------------
	protected void Start()
	{
		if (FileIO.FileExists("$profile:zlab_restart.txt"))
		{
			RZL.Row("clock", RZL_ExpClock.State("after a restart"));
			RZL.Log("DONE");
			GetGame().GetCallqueue().CallLater(Close, 3000, false);
			return;
		}

		s_bQuick = FileIO.FileExists("$profile:zlab_quick.txt");
		RZL.Log(string.Format("zombie lab starts (quick=%1)", s_bQuick));
		LogFactions();

		AIWorld aiw = GetGame().GetAIWorld();
		if (aiw)
			RZL.Row("world", string.Format("aiLimit=%1|activeAIs=%2|maxLOD=%3", aiw.GetLimitOfActiveAIs(), aiw.GetCurrentNumOfActiveAIs(), AIAgent.GetMaxLOD()));
		else
			RZL.Row("world", "error=no AIWorld");

		m_Sites = new RZL_Sites();
		m_Sites.Find(3, 2, 8);
		Setup();
		m_fT0 = RZL.Now();
		GetGame().GetCallqueue().CallLater(Tick, 250, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void LogFactions()
	{
		FactionManager fm = GetGame().GetFactionManager();
		if (!fm)
		{
			RZL.Row("factions", "error=no FactionManager");
			return;
		}

		array<Faction> facs = {};
		fm.GetFactionsList(facs);
		Faction bacon = fm.GetFactionByKey("BACON_622120A5448725E3_FACTION");
		foreach (Faction f : facs)
		{
			string enemy = "-";
			if (bacon)
				enemy = bacon.IsFactionEnemy(f).ToString();
			RZL.Row("factions", string.Format("key=%1|baconHostileToIt=%2|itHostileToBacon=%3", f.GetFactionKey(), enemy, bacon && f.IsFactionEnemy(bacon)));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected RZL_Site TakeSite(bool wood, string forWhat)
	{
		RZL_Site s = m_Sites.TakeWork();
		if (s)
			return s;

		// Short of sites: reuse the first site found (the experiments may see each other)
		RZL.Log("SHORT OF SITES: " + forWhat + " shares a site");
		RZL_Site again = new RZL_Site();
		again.m_sName = "shared-" + forWhat;
		foreach (RZL_Exp e : m_aExps)
		{
			if (e.m_Site)
			{
				again.m_vT = e.m_Site.m_vT;
				again.m_fYaw = e.m_Site.m_fYaw;
				return again;
			}
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected void AddExp(RZL_Exp e, float startAt, bool wood)
	{
		e.m_fStartAt = startAt;
		e.m_Site = TakeSite(wood, e.m_sName);
		if (!e.m_Site)
		{
			RZL.Log("no site at all for " + e.m_sName + "; skipped");
			return;
		}

		m_aExps.Insert(e);
		// Navmesh round the site
		AIWorld aiw = GetGame().GetAIWorld();
		if (!aiw)
			return;

		for (int d = 0; d <= 300; d += 50)
			aiw.RequestNavmeshLoad(e.m_Site.Along(d));
	}

	//------------------------------------------------------------------------------------------------
	protected void Setup()
	{
		m_Plan = new RZL_PercPlan();
		m_Clock = new RZL_ExpClock();
		m_Clock.m_fStartAt = 0;
		m_aExps.Insert(m_Clock);

		// By priority, in case the map is short of sites
		AddExp(new RZL_ExpWaypoint(), 25, false);
		AddPerc(false);
		AddPerc(true);
		AddExp(new RZL_ExpShare(), 130, false);
		AddExp(new RZL_ExpSpeeds(), 35, false);
		AddExp(new RZL_ExpGroups(), 15, false);
		AddExp(new RZL_ExpLeak(), 20, false);
		AddExp(new RZL_ExpNoise(), 90, false);
		AddExp(new RZL_ExpHealth(), 60, false);
		AddPerc(false);
		AddPerc(false);
		AddPerc(true);
		m_Perf = new RZL_ExpPerf();
		m_Perf.m_fStartAt = 1000000;
		m_Perf.m_Site = TakeSite(false, "perf");
		m_Plan.m_bHasOpen = m_iOpenRunners > 0;
		m_Plan.m_bHasWood = m_iWoodRunners > 0;
		RZL.Log(string.Format("perception runners: %1 open, %2 wood edge", m_iOpenRunners, m_iWoodRunners));
	}

	//------------------------------------------------------------------------------------------------
	protected void AddPerc(bool wood)
	{
		RZL_Site s = m_Sites.Take(wood);
		if (!s)
		{
			RZL.Log(string.Format("no free site for a perception runner (wood=%1)", wood));
			return;
		}

		RZL_ExpPerc p = new RZL_ExpPerc();
		p.m_Plan = m_Plan;
		p.m_fStartAt = 0;
		p.m_Site = s;
		m_aExps.Insert(p);
		if (wood)
			m_iWoodRunners++;
		else
			m_iOpenRunners++;
		AIWorld aiw = GetGame().GetAIWorld();
		if (!aiw)
			return;

		for (int d = 0; d <= 300; d += 50)
			aiw.RequestNavmeshLoad(s.Along(d));
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyLight(string block)
	{
		TimeAndWeatherManagerEntity tm = RZL.Weather();
		if (!tm)
			return;

		tm.SetFogAmountOverride(false, 0);
		if (block == "night")
			tm.SetTimeOfTheDay(1, true);
		else
			tm.SetTimeOfTheDay(12, true);
		if (block == "fog")
			tm.SetFogAmountOverride(true, 1);
		RZL.Row("light", string.Format("block=%1|tod=%2|fog=%3|moonPhase=%4|autoAdvance=%5", block, RZL.F2(tm.GetTimeOfTheDay()), RZL.F2(tm.GetFogAmount()), RZL.F2(tm.GetMoonPhase(tm.GetTimeOfTheDay())), tm.GetIsDayAutoAdvanced()));
	}

	//------------------------------------------------------------------------------------------------
	protected void Tick()
	{
		float now = RZL.Now() - m_fT0;
		if (m_bEnding)
			return;

		// The perception blocks: day once the clock test froze noon, then night, then fog
		if (m_Clock.m_bDone)
		{
			if (m_Plan.m_iBlock < 0)
			{
				m_Plan.m_iBlock = 0;
				ApplyLight(m_Plan.Block());
				m_Plan.Fill();
				m_Plan.m_bSwitching = false;
			}
			else if (m_Plan.BlockFinished())
			{
				m_Plan.m_bSwitching = true;
				m_Plan.m_iBlock++;
				m_fSwitchAt = now;
				if (!m_Plan.AllDone())
					ApplyLight(m_Plan.Block());
				else
					RZL.Log("perception: all blocks done");
			}
			else if (m_Plan.m_bSwitching && !m_Plan.AllDone() && now - m_fSwitchAt > 5)
			{
				m_Plan.Fill();
				m_Plan.m_bSwitching = false;
			}
		}

		bool allDone = true;
		foreach (RZL_Exp e : m_aExps)
		{
			if (e.m_bDone)
				continue;

			allDone = false;
			if (!e.m_bStarted && now >= e.m_fStartAt)
				e.Begin(now);
			if (e.m_bStarted)
				e.Tick(now);
		}

		// Frame time over the last minute, for the whole run
		int minute = now / 60;
		if (minute > m_iLastFrameLog && m_iFrames > 0)
		{
			m_iLastFrameLog = minute;
			int active = -1;
			if (GetGame().GetAIWorld())
				active = GetGame().GetAIWorld().GetCurrentNumOfActiveAIs();
			RZL.Log(string.Format("minute %1: frames=%2 meanMs=%3 maxMs=%4 activeAIs=%5 tod=%6 perception=%7", minute, m_iFrames, RZL.F2(m_fFrameSum / m_iFrames), RZL.F2(m_fFrameMax), active, RZL.TimeOfDay(), m_Plan.Block()));
			m_fFrameSum = 0;
			m_fFrameMax = 0;
			m_iFrames = 0;
		}

		if (!allDone)
			return;

		// Server frame time last, with nothing else running
		if (m_Perf && m_Perf.m_Site && !m_Perf.m_bStarted)
		{
			m_Perf.m_fStartAt = now;
			m_aExps.Insert(m_Perf);
			return;
		}

		End();
	}

	//------------------------------------------------------------------------------------------------
	protected void End()
	{
		m_bEnding = true;
		TimeAndWeatherManagerEntity tm = RZL.Weather();
		if (tm)
		{
			tm.SetFogAmountOverride(false, 0);
			tm.SetIsDayAutoAdvanced(true);
			tm.SetTimeOfTheDay(3.2833, true);
			RZL.Row("clock", RZL_ExpClock.State("set to 03:17 before shutdown"));
		}
		RZL.Log("DONE");
		GetGame().GetCallqueue().CallLater(Close, 8000, false);
	}
}
