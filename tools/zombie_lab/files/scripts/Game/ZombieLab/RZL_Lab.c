// Test-only (not part of the mod): the zombie lab. Placed on the game mode of the lab world
// (tools/zombie_lab/files/Worlds/ZLab/ZLab.ent). Finds sites on the map, runs the experiments at
// places far apart, prints "RZL R|table|..." rows for tools/zombie_lab/report.py, then closes.
// "$profile:zlab_quick.txt" makes a short debug run; "$profile:zlab_only.txt" (a comma-separated list
// of experiment names) runs only those; "$profile:zlab_restart.txt" only reports the clock after a
// restart (run.sh's second boot). Naming "behaviour" (or any of b1-b20) runs the behaviour tests of
// the infected behaviour addendum instead (RZL_B*.c, with the hooks in RZL_Hooks.c).
class RZL_LabComponentClass : ScriptComponentClass
{
}

class RZL_LabComponent : ScriptComponent
{
	static bool s_bQuick;
	static string s_sOnly;

	protected ref array<ref RZL_Exp> m_aExps = {};
	protected ref RZL_Sites m_Sites;
	protected ref RZL_PercPlan m_Plan;
	protected ref RZL_ExpClock m_Clock;
	protected ref RZL_Exp m_Perf;
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
		s_sOnly = "";
		if (FileIO.FileExists("$profile:zlab_only.txt"))
		{
			FileHandle f = FileIO.OpenFile("$profile:zlab_only.txt", FileMode.READ);
			if (f)
			{
				f.ReadLine(s_sOnly);
				f.Close();
			}
			s_sOnly.TrimInPlace();
		}
		RZL.Log(string.Format("zombie lab starts (quick=%1, only='%2')", s_bQuick, s_sOnly));
		LogFactions();

		AIWorld aiw = GetGame().GetAIWorld();
		if (aiw)
			RZL.Row("world", string.Format("aiLimit=%1|activeAIs=%2|maxLOD=%3", aiw.GetLimitOfActiveAIs(), aiw.GetCurrentNumOfActiveAIs(), AIAgent.GetMaxLOD()));
		else
			RZL.Row("world", "error=no AIWorld");

		m_Sites = new RZL_Sites();
		if (IsBehaviour())
		{
			SetupBehaviour();
		}
		else
		{
			m_Sites.Find(3, 2, 8);
			Setup();
		}
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
	//! Is this experiment in the run? (all of them unless zlab_only.txt names some)
	static bool Wanted(string name)
	{
		if (s_sOnly == "")
			return true;

		string list = "," + s_sOnly + ",";
		if (list.Contains("," + name + ","))
			return true;

		return IsBName(name) && list.Contains(",behaviour,");
	}

	//------------------------------------------------------------------------------------------------
	//! b1 ... b20
	static bool IsBName(string name)
	{
		return name.Length() >= 2 && name.Length() <= 3 && name.IndexOf("b") == 0 && name.Substring(1, name.Length() - 1).ToInt() > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! A behaviour run: zlab_only.txt names "behaviour" or any of b1-b20.
	static bool IsBehaviour()
	{
		array<string> names = {};
		s_sOnly.Split(",", names, true);
		foreach (string n : names)
		{
			n.TrimInPlace();
			if (n == "behaviour" || IsBName(n))
				return true;
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void LoadNavmesh(RZL_Site s)
	{
		AIWorld aiw = GetGame().GetAIWorld();
		if (!aiw || !s)
			return;

		for (int d = -100; d <= 300; d += 50)
			aiw.RequestNavmeshLoad(s.Along(d));
	}

	//------------------------------------------------------------------------------------------------
	protected RZL_Lane AddLane(string name, RZL_Site site)
	{
		RZL_Lane lane = new RZL_Lane(name);
		lane.m_fStartAt = 3;
		lane.m_Site = site;
		LoadNavmesh(site);
		return lane;
	}

	//------------------------------------------------------------------------------------------------
	protected void Keep(RZL_Lane lane)
	{
		if (!lane || lane.m_aSeq.IsEmpty() || !lane.m_Site)
			return;

		string names = "";
		foreach (RZL_Exp e : lane.m_aSeq)
			names += e.m_sName + " ";
		RZL.Log(string.Format("lane %1 at %2: %3", lane.m_sName, lane.m_Site.m_sName, names));
		m_aExps.Insert(lane);
	}

	//------------------------------------------------------------------------------------------------
	//! An open site with a clear line (the perception kind), else a work site.
	protected RZL_Site OpenSite()
	{
		RZL_Site s = m_Sites.Take(false);
		if (!s)
			s = m_Sites.TakeWork();
		return s;
	}

	//------------------------------------------------------------------------------------------------
	protected static RZL_Site NewSite(string name, vector t)
	{
		RZL_Site s = new RZL_Site();
		s.m_sName = name;
		s.m_vT = t;
		s.m_fYaw = 0;
		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! The behaviour tests B1-B20, in lanes at places far apart; B15 and B12 last, alone.
	protected void SetupBehaviour()
	{
		TimeAndWeatherManagerEntity tm = RZL.Weather();
		if (tm)
		{
			tm.SetTimeOfTheDay(12, true);
			tm.SetIsDayAutoAdvanced(false);
		}

		m_Sites.m_fApart = 1100;
		m_Sites.Find(4, 0, 9);
		array<ref RZL_Site> used = {};
		foreach (RZL_Site s0 : m_Sites.m_aSites)
			used.Insert(s0);
		RZL_Walls.Measure(RZL.Ground(m_Sites.m_aSites[0].m_vT + Vector(0, 0, 600)));
		RZL_Water.Find(used);
		RZL_Houses.Find(used, 2);

		// B13: the sight checks' cost, right away (no agents yet)
		RZL_House houseA = null;
		if (!RZL_Houses.s_aHouses.IsEmpty())
			houseA = RZL_Houses.s_aHouses[0];
		RZL_House houseU = RZL_Houses.Upper();
		if (Wanted("b13") && !m_Sites.m_aSites.IsEmpty())
		{
			RZL_ExpB13 b13 = new RZL_ExpB13(houseA);
			b13.m_Site = m_Sites.m_aSites[0];
			b13.m_fStartAt = 1;
			m_aExps.Insert(b13);
		}

		RZL_Lane l1 = AddLane("L1", OpenSite());
		l1.Add(new RZL_ExpB1(0, 3));
		l1.Add(new RZL_ExpB6());
		l1.Add(new RZL_ExpB14());
		Keep(l1);
		RZL_Lane l2 = AddLane("L2", OpenSite());
		l2.Add(new RZL_ExpB1(1, 3));
		l2.Add(new RZL_ExpB4());
		Keep(l2);
		RZL_Lane l3 = AddLane("L3", OpenSite());
		l3.Add(new RZL_ExpB1(2, 3));
		l3.Add(new RZL_ExpB5());
		Keep(l3);
		RZL_Lane l4 = AddLane("L4", OpenSite());
		l4.Add(new RZL_ExpB18());
		Keep(l4);
		RZL_Lane l5 = AddLane("L5", m_Sites.TakeWork());
		l5.Add(new RZL_ExpB3(true));
		Keep(l5);
		RZL_Lane l6 = AddLane("L6", m_Sites.TakeWork());
		l6.Add(new RZL_ExpB3(false));
		Keep(l6);
		RZL_Lane l7 = AddLane("L7", m_Sites.TakeWork());
		l7.Add(new RZL_ExpB20());
		Keep(l7);
		RZL_Lane l8 = AddLane("L8", m_Sites.TakeWork());
		l8.Add(new RZL_ExpB7());
		Keep(l8);
		RZL_Lane l9 = AddLane("L9", m_Sites.TakeWork());
		l9.Add(new RZL_ExpB8());
		l9.Add(new RZL_ExpB17());
		Keep(l9);
		RZL_Lane l10 = AddLane("L10", m_Sites.TakeWork());
		l10.Add(new RZL_ExpB9());
		Keep(l10);
		if (RZL_Water.s_bRiver || RZL_Water.s_bPond)
		{
			vector wt = RZL_Water.s_vPondEdge;
			if (RZL_Water.s_bRiver)
				wt = RZL_Water.s_vC;
			RZL_Lane l11 = AddLane("L11", NewSite("water", wt));
			l11.Add(new RZL_ExpB16());
			Keep(l11);
		}
		else if (Wanted("b16"))
		{
			RZL.Row("b16", "note=no river or pond found");
		}
		if (houseA)
		{
			RZL_Lane l12 = AddLane("L12", NewSite(houseA.m_sName, houseA.m_vInside));
			l12.Add(new RZL_ExpB19(houseA));
			l12.Add(new RZL_ExpB10(houseA, false));
			if (houseU == houseA)
				l12.Add(new RZL_ExpB10(houseU, true));
			Keep(l12);
		}
		else
		{
			RZL.Row("b19", "note=no house found");
		}
		if (houseU && houseU != houseA)
		{
			RZL_Lane l13 = AddLane("L13", NewSite(houseU.m_sName, houseU.m_vInside));
			l13.Add(new RZL_ExpB10(houseU, true));
			l13.Add(new RZL_ExpB10(houseU, false));
			Keep(l13);
		}
		if (Wanted("b15") || Wanted("b12"))
		{
			RZL_ExpB15Run perf = new RZL_ExpB15Run();
			perf.m_fStartAt = 1000000;
			perf.m_Site = m_Sites.TakeWork();
			if (!perf.m_Site)
				perf.m_Site = m_Sites.m_aSites[0];
			LoadNavmesh(perf.m_Site);
			m_Perf = perf;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AddExp(RZL_Exp e, float startAt, bool wood)
	{
		if (!Wanted(e.m_sName))
			return;

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
		if (Wanted("clock") || Wanted("perception") || Wanted("perception-core"))
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
		if (Wanted("perf"))
		{
			m_Perf = new RZL_ExpPerf();
			m_Perf.m_fStartAt = 1000000;
			m_Perf.m_Site = TakeSite(false, "perf");
		}
		m_Plan.m_bHasOpen = m_iOpenRunners > 0;
		m_Plan.m_bHasWood = m_iWoodRunners > 0;
		RZL.Log(string.Format("perception runners: %1 open, %2 wood edge", m_iOpenRunners, m_iWoodRunners));
	}

	//------------------------------------------------------------------------------------------------
	protected void AddPerc(bool wood)
	{
		if (!Wanted("perception") && !Wanted("perception-core"))
			return;

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
		if (m_Clock && m_Plan && m_Clock.m_bDone)
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
			string block = "-";
			if (m_Plan)
				block = m_Plan.Block();
			RZL.Log(string.Format("minute %1: frames=%2 meanMs=%3 maxMs=%4 activeAIs=%5 tod=%6 perception=%7", minute, m_iFrames, RZL.F2(m_fFrameSum / m_iFrames), RZL.F2(m_fFrameMax), active, RZL.TimeOfDay(), block));
			RZL.Row("b2", string.Format("minute=%1|activeAIs=%2|", minute, active) + RZL_Hooks.Line());
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
		RZL.Row("b2", "minute=end|activeAIs=-1|" + RZL_Hooks.Line());
		RZL.Log("DONE");
		GetGame().GetCallqueue().CallLater(Close, 8000, false);
	}
}
