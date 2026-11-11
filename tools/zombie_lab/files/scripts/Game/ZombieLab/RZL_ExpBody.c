// Test-only (not part of the mod): health and the silent kill, and server frame time.

//------------------------------------------------------------------------------------------------
//! Frame times, filled by the lab component's EOnFrame while measuring.
class RZL_Frames
{
	static ref array<float> s_aMs = {};
	static bool s_bOn;

	//------------------------------------------------------------------------------------------------
	static void Start()
	{
		s_aMs.Clear();
		s_bOn = true;
	}

	//------------------------------------------------------------------------------------------------
	static void Add(float ms)
	{
		if (s_bOn)
			s_aMs.Insert(ms);
	}

	//------------------------------------------------------------------------------------------------
	//! Stops and returns "frames=..|meanMs=..|p50=..|p95=..|p99=..|maxMs=..|fps=.."
	static string Stop()
	{
		s_bOn = false;
		int n = s_aMs.Count();
		if (n == 0)
			return "frames=0";

		float sum;
		foreach (float v : s_aMs)
			sum += v;
		s_aMs.Sort();
		float mean = sum / n;
		int i50 = n * 0.5;
		int i95 = n * 0.95;
		int i99 = n * 0.99;
		return string.Format("frames=%1|meanMs=%2|p50Ms=%3|p95Ms=%4|p99Ms=%5|maxMs=%6|fps=%7", n, RZL.F2(mean), RZL.F2(s_aMs[i50]), RZL.F2(s_aMs[i95]), RZL.F2(s_aMs[i99]), RZL.F2(s_aMs[n - 1]), RZL.F(1000 / mean));
	}
}

//------------------------------------------------------------------------------------------------
//! 6. Reading and setting health, regeneration, and four ways to remove an infected.
class RZL_ExpHealth : RZL_Exp
{
	protected ref array<string> m_aTypes = {RZL.CIV, RZL.MIL, RZL.RUNNER, RZL.HEAVY};
	protected ref array<string> m_aNames = {"civilian", "military", "exploder", "heavy"};
	protected ref array<IEntity> m_aBodies = {};
	protected ref array<string> m_aKills = {"civilian: damage manager Kill()", "civilian: SetHealthScaled(0)", "civilian: delete the entity", "exploder: Kill(), a civilian 3 m away", "exploder: delete the entity, a civilian 3 m away", "heavy: Kill()"};
	protected int m_iKill;
	protected IEntity m_Ch;
	protected IEntity m_Dummy;
	protected SCR_AIGroup m_G;
	protected int m_iItems0;
	protected ref RZL_ItemCounter m_Items = new RZL_ItemCounter();
	protected vector m_vAt;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpHealth()
	{
		m_sName = "health";
	}

	//------------------------------------------------------------------------------------------------
	protected static SCR_DamageManagerComponent Dmg(IEntity e)
	{
		ChimeraCharacter c = ChimeraCharacter.Cast(e);
		if (!c)
			return null;

		return c.GetDamageManager();
	}

	//------------------------------------------------------------------------------------------------
	protected static string Zones(IEntity e)
	{
		SCR_DamageManagerComponent dmg = Dmg(e);
		if (!dmg)
			return "-";

		array<HitZone> zones = {};
		dmg.GetAllHitZones(zones);
		string list = "";
		foreach (HitZone z : zones)
			list += string.Format("%1:%2 ", z.GetName(), RZL.F(z.GetMaxHealth()));
		return list;
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		float s = Since(now);
		if (m_iStep == 0)
		{
			for (int i = 0; i < m_aTypes.Count(); i++)
			{
				IEntity ch = RZL.SpawnSolo(m_aTypes[i], m_Site.Side(20 + 25 * i, 0), m_Site.m_fYaw, m_aGroups);
				m_aBodies.Insert(ch);
				m_aChars.Insert(ch);
			}
			Next(now);
			return;
		}

		if (m_iStep == 1 && s >= 3)
		{
			for (int j = 0; j < m_aBodies.Count(); j++)
			{
				SCR_DamageManagerComponent d1 = Dmg(m_aBodies[j]);
				if (!d1)
					continue;

				float h0 = d1.GetHealth();
				float max0 = d1.GetMaxHealth();
				bool setOk = d1.SetHealthScaled(0.5);
				RZL.Row("health", string.Format("type=%1|health=%2|maxHealth=%3|scaled=%4|SetHealthScaled(0.5)=%5|readBack=%6|zones=%7", m_aNames[j], RZL.F(h0), RZL.F(max0), RZL.F2(d1.GetHealthScaled()), setOk, RZL.F2(d1.GetHealthScaled()), Zones(m_aBodies[j])));
			}
			Next(now);
			return;
		}

		if (m_iStep == 2 && s >= 15)
		{
			for (int k = 0; k < m_aBodies.Count(); k++)
			{
				SCR_DamageManagerComponent d2 = Dmg(m_aBodies[k]);
				if (d2)
					RZL.Row("healthRegen", string.Format("type=%1|scaledAfter15s=%2|state=%3", m_aNames[k], RZL.F2(d2.GetHealthScaled()), typename.EnumToString(EDamageState, d2.GetState())));
			}
			RZL.DelAll(m_aChars, m_aGroups);
			Next(now);
			return;
		}

		// The removal cases, one at a time
		if (m_iStep == 3 && s >= 2)
		{
			if (m_iKill >= m_aKills.Count())
			{
				Finish();
				return;
			}

			string prefab = RZL.CIV;
			if (m_iKill == 3 || m_iKill == 4)
				prefab = RZL.RUNNER;
			if (m_iKill == 5)
				prefab = RZL.HEAVY;
			m_vAt = m_Site.Side(120, 0);
			m_Ch = RZL.SpawnSolo(prefab, m_vAt, m_Site.m_fYaw, m_aGroups);
			m_G = SCR_AIGroup.Cast(m_aGroups[m_aGroups.Count() - 1]);
			m_aChars.Insert(m_Ch);
			Next(now);
			return;
		}

		if (m_iStep == 4 && s >= 3)
		{
			if (!m_Ch || !Dmg(m_Ch))
			{
				RZL.Row("kill", "case=" + m_aKills[m_iKill] + "|note=spawn failed");
				m_iKill++;
				m_iStep = 3;
				m_fStepAt = now;
				return;
			}

			m_vAt = m_Ch.GetOrigin();
			m_iItems0 = m_Items.Count(m_vAt, 4);
			if (m_iKill == 3 || m_iKill == 4)
			{
				m_Dummy = RZL.Spawn(RZL.TARGET, RZL.At(m_vAt, m_Site.m_fYaw, 3), 0);
				m_aChars.Insert(m_Dummy);
			}

			if (m_iKill == 0 || m_iKill == 3 || m_iKill == 5)
				RZL.Kill(m_Ch);
			if (m_iKill == 1)
				Dmg(m_Ch).SetHealthScaled(0);
			if (m_iKill == 2 || m_iKill == 4)
				RZL.Del(m_Ch);
			Next(now);
			return;
		}

		if (m_iStep == 5 && s >= 5)
		{
			int agents = -1;
			if (m_G)
				agents = m_G.GetAgentsCount();
			string state = "gone";
			if (m_Ch && Dmg(m_Ch))
				state = typename.EnumToString(EDamageState, Dmg(m_Ch).GetState());
			float dummy = -1;
			if (m_Dummy && Dmg(m_Dummy))
				dummy = Dmg(m_Dummy).GetHealthScaled();
			RZL.Row("kill", string.Format("case=%1|bodyExists=%2|state=%3|groupExists=%4|groupAgents=%5|looseItemsBefore=%6|looseItemsAfter=%7|dummyHealth=%8", m_aKills[m_iKill], m_Ch != null, state, m_G != null, agents, m_iItems0, m_Items.Count(m_vAt, 4), RZL.F2(dummy)));
			RZL.DelAll(m_aChars, m_aGroups);
			m_iKill++;
			m_iStep = 3;
			m_fStepAt = now;
		}
	}
}

//------------------------------------------------------------------------------------------------
//! 5. Server frame time with 0, 60, 100 and 140 Bacon agents (groups of 5, half of them walking
//! between two held waypoints), all at LOD 0; then the same 140 left to their natural LOD.
class RZL_ExpPerf : RZL_Exp
{
	protected ref array<int> m_aLevels = {0, 60, 100, 140};
	protected int m_iLevel;
	protected ref array<SCR_AIGroup> m_aGs = {};
	protected ref array<AIWaypoint> m_aHeld = {};
	protected ref array<vector> m_aA = {};
	protected ref array<vector> m_aB = {};
	protected int m_iSpawned;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpPerf()
	{
		m_sName = "perf";
		if (RZL_LabComponent.s_bQuick)
		{
			m_aLevels.Clear();
			m_aLevels.Insert(0);
			m_aLevels.Insert(20);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnGroup()
	{
		int gi = m_aGs.Count();
		int col = gi % 7;
		int row = gi / 7;
		float along = 20 + col * 40;
		float lateral = -90 + row * 60;
		vector a = m_Site.Side(along, lateral);
		vector b = m_Site.Side(along, -lateral);
		SCR_AIGroup g = RZL.SpawnEmptyGroup(a);
		for (int i = 0; i < 5; i++)
		{
			IEntity ch = RZL.SpawnMember(g, RZL.CIV, a + Vector(i * 1.5, 0, 0), 0);
			RZL.ForceLOD(ch, 0);
			m_aChars.Insert(ch);
			m_iSpawned++;
		}
		m_aGs.Insert(g);
		m_aGroups.Insert(g);
		m_aA.Insert(a);
		m_aB.Insert(b);
		AIWaypoint held;
		if (gi % 2 == 1)
		{
			held = RZL.NewWaypoint(b);
			g.AddWaypoint(held);
		}
		m_aHeld.Insert(held);
	}

	//------------------------------------------------------------------------------------------------
	//! Walking groups go back and forth: when the held waypoint is done, move it to the other end.
	protected void KeepWalking()
	{
		for (int i = 0; i < m_aGs.Count(); i++)
		{
			SCR_AIGroup g = m_aGs[i];
			AIWaypoint w = m_aHeld[i];
			if (!g || !w || RZL.HasWaypoint(g, w))
				continue;

			vector next = m_aA[i];
			if (RZL.DistXZ(w.GetOrigin(), m_aA[i]) < 1)
				next = m_aB[i];
			RZL.MoveHeld(g, w, next);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		float s = Since(now);
		KeepWalking();
		AIWorld aiw = GetGame().GetAIWorld();
		if (m_iStep == 0)
		{
			int limit = aiw.GetLimitOfActiveAIs();
			RZL.Row("perfInfo", string.Format("aiLimitAtStart=%1|activeAIs=%2", limit, aiw.GetCurrentNumOfActiveAIs()));
			if (limit < 250)
			{
				aiw.SetLimitOfActiveAIs(250);
				RZL.Log("perf: AI limit raised to " + aiw.GetLimitOfActiveAIs().ToString());
			}
			Next(now);
			return;
		}

		// Spawn up to the level, one group every 0.5 s
		if (m_iStep == 1)
		{
			if (m_iLevel >= m_aLevels.Count())
			{
				m_iStep = 4;
				m_fStepAt = now;
				return;
			}

			if (m_iSpawned < m_aLevels[m_iLevel])
			{
				if (s >= 0.5)
				{
					SpawnGroup();
					m_fStepAt = now;
				}
				return;
			}

			m_iStep = 2;
			m_fStepAt = now;
			return;
		}

		if (m_iStep == 2 && s >= 30)
		{
			RZL_Frames.Start();
			m_iStep = 3;
			m_fStepAt = now;
			return;
		}

		if (m_iStep == 3 && s >= 60)
		{
			string stats = RZL_Frames.Stop();
			RZL.Row("perf", string.Format("agents=%1|lod=forced 0|activeAIs=%2|", m_iSpawned, aiw.GetCurrentNumOfActiveAIs()) + stats);
			m_iLevel++;
			m_iStep = 1;
			m_fStepAt = now;
			return;
		}

		// Natural LOD (no player anywhere near)
		if (m_iStep == 4)
		{
			foreach (IEntity c4 : m_aChars)
			{
				AIAgent a4 = RZL.Agent(c4);
				if (a4)
					a4.SetPermanentLOD(-1);
			}
			Next(now);
			return;
		}

		if (m_iStep == 5 && s >= 20)
		{
			RZL_Frames.Start();
			Next(now);
			return;
		}

		if (m_iStep == 6 && s >= 60)
		{
			string stats6 = RZL_Frames.Stop();
			map<int, int> hist = new map<int, int>();
			foreach (IEntity c6 : m_aChars)
			{
				int lod = RZL.LOD(c6);
				hist.Set(lod, hist.Get(lod) + 1);
			}
			string lods = "";
			for (int h = 0; h < hist.Count(); h++)
				lods += string.Format("LOD%1 x%2 ", hist.GetKey(h), hist.GetElement(h));
			RZL.Row("perf", string.Format("agents=%1|lod=natural (%2)|activeAIs=%3|", m_iSpawned, lods, aiw.GetCurrentNumOfActiveAIs()) + stats6);
			foreach (AIWaypoint w : m_aHeld)
				RZL.Del(w);
			RZL.DelAll(m_aChars, m_aGroups);
			m_aGs.Clear();
			m_aHeld.Clear();
			Next(now);
			return;
		}

		if (m_iStep == 7 && s >= 15)
		{
			RZL_Frames.Start();
			Next(now);
			return;
		}

		if (m_iStep == 8 && s >= 30)
		{
			RZL.Row("perf", string.Format("agents=0|lod=after deleting all|activeAIs=%1|", aiw.GetCurrentNumOfActiveAIs()) + RZL_Frames.Stop());
			Finish();
		}
	}
}
