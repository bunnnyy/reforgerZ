// Test-only (not part of the mod): B15 the server frame time with the behaviour layer, and B12 LOD,
// the think-rate hook and sleep (/root/rz_tools/behaviour_addendum.md, section 14). Runs last, alone.

//------------------------------------------------------------------------------------------------
class RZL_ExpB15 : RZL_BExp
{
	static const float SETTLE = 20;
	static const float MEASURE = 50;

	protected ref array<string> m_aStages = {"0", "60 off", "100 off", "100 on", "lod 2", "lod 4", "lod 6", "wait 1.0", "sleep", "140 on", "after"};
	protected int m_iStage;
	protected ref array<SCR_AIGroup> m_aGs = {};
	protected ref array<ref array<IEntity>> m_aMem = {};
	protected ref array<vector> m_aA = {};
	protected ref array<vector> m_aB = {};
	protected ref array<bool> m_aWalk = {};
	protected ref array<int> m_aToB = {};
	protected ref array<AIWaypoint> m_aHeld = {};
	protected bool m_bLayer;
	protected int m_iSpawned;
	protected float m_fStageAt;
	protected bool m_bMeasuring;
	protected float m_fLastSlots;
	protected int m_iSlotMs;
	protected int m_iSlotRuns;
	protected int m_iLoops0;
	protected int m_iLoiter0;
	protected int m_iShooter0;
	protected int m_iClimb0;
	protected ref array<IEntity> m_aProbe = {};
	protected ref array<vector> m_aProbeSlot = {};
	protected ref array<float> m_aProbeMoved = {};
	protected float m_fProbeAt;
	protected ref array<IEntity> m_aSleep = {};
	protected ref array<int> m_aSleepLoops = {};
	protected float m_fWakeAt;
	protected ref array<float> m_aWakeLoop = {};
	protected ref array<float> m_aWakeStand = {};
	protected ref array<float> m_aWakeMove = {};
	protected ref array<vector> m_aWakePos = {};

	//------------------------------------------------------------------------------------------------
	void RZL_ExpB15()
	{
		m_sName = "b15";
		if (RZL_LabComponent.s_bQuick)
		{
			m_aStages.Clear();
			m_aStages.Insert("0");
			m_aStages.Insert("20 on");
			m_aStages.Insert("lod 4");
			m_aStages.Insert("sleep");
			m_aStages.Insert("after");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnGroup(bool walk)
	{
		int gi = m_aGs.Count();
		int col = gi % 7;
		int row = gi / 7;
		vector a = m_Site.Side(20 + col * 40, -90 + row * 45);
		vector b = m_Site.Side(20 + col * 40, 90 - row * 45);
		SCR_AIGroup g = RZL.SpawnEmptyGroup(a);
		array<IEntity> mem = {};
		for (int i = 0; i < 5; i++)
		{
			IEntity ch = RZL.SpawnMember(g, RZL.CIV, a + Vector(i * 2, 0, 0), 0);
			RZL.ForceLOD(ch, 0);
			mem.Insert(ch);
			m_aChars.Insert(ch);
			m_iSpawned++;
		}
		m_aGs.Insert(g);
		m_aGroups.Insert(g);
		m_aMem.Insert(mem);
		m_aA.Insert(a);
		m_aB.Insert(b);
		m_aWalk.Insert(walk);
		m_aToB.Insert(1);
		AIWaypoint held;
		if (walk && !m_bLayer)
		{
			held = RZL.NewWaypoint(b);
			g.AddWaypoint(held);
		}
		if (walk && m_bLayer)
		{
			foreach (IEntity m : mem)
				RZL_B.Steer(m, m.GetOrigin());
		}
		m_aHeld.Insert(held);
	}

	//------------------------------------------------------------------------------------------------
	//! Layer off: walking groups go back and forth on their held waypoint (as phase 0).
	protected void KeepWalkingWaypoints()
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
	//! Layer on: walking groups in a line formation, slots 18 m ahead of their centre (walk-stop).
	protected void WriteSlots()
	{
		int t0 = System.GetTickCount();
		for (int i = 0; i < m_aGs.Count(); i++)
		{
			if (!m_aWalk[i])
				continue;

			array<IEntity> mem = m_aMem[i];
			vector sum = vector.Zero;
			int n;
			foreach (IEntity m : mem)
			{
				if (!m)
					continue;

				sum = sum + m.GetOrigin();
				n++;
			}
			if (n == 0)
				continue;

			float fn = n;
			vector centre = sum * (1 / fn);
			vector goal = m_aB[i];
			if (m_aToB[i] == 0)
				goal = m_aA[i];
			if (RZL.DistXZ(centre, goal) < 8)
			{
				m_aToB[i] = 1 - m_aToB[i];
				continue;
			}

			vector dir = goal - centre;
			dir[1] = 0;
			float len = dir.Length();
			dir = dir * (1 / len);
			vector anchor = centre + dir * Math.Min(18, len);
			float yaw = RZL.YawTo(centre, goal);
			for (int k = 0; k < mem.Count(); k++)
			{
				if (mem[k])
					RZL_B.Slot(mem[k], RZL_Form.At(anchor, yaw, RZL_Form.Offset("line", k, mem.Count(), 4)));
			}
		}
		m_iSlotMs += System.GetTickCount() - t0;
		m_iSlotRuns++;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetLayer(bool on)
	{
		m_bLayer = on;
		for (int i = 0; i < m_aGs.Count(); i++)
		{
			if (!m_aWalk[i])
				continue;

			if (on)
			{
				if (m_aHeld[i] && RZL.HasWaypoint(m_aGs[i], m_aHeld[i]))
					m_aGs[i].RemoveWaypoint(m_aHeld[i]);
				foreach (IEntity m : m_aMem[i])
					RZL_B.Steer(m, m.GetOrigin());
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ForAll(int lod, float wait)
	{
		foreach (IEntity c : m_aChars)
		{
			RZL.ForceLOD(c, lod);
			Bacon_622120A5448725E3_InfectedCharacter inf = RZL_B.Inf(c);
			if (inf)
				inf.m_fRZLWait = wait;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Five steered bodies get a slot 15 m to the side: the order latency at this think rate.
	protected void StartProbe(float now)
	{
		m_aProbe.Clear();
		m_aProbeSlot.Clear();
		m_aProbeMoved.Clear();
		for (int i = 0; i < m_aGs.Count() && m_aProbe.Count() < 5; i++)
		{
			if (!m_aWalk[i] || !m_aMem[i][0])
				continue;

			IEntity b = m_aMem[i][0];
			m_aWalk[i] = false;
			vector slot = b.GetOrigin() + vector.FromYaw(m_Site.m_fYaw + 90) * 15;
			RZL_B.Slot(b, slot);
			m_aProbe.Insert(b);
			m_aProbeSlot.Insert(b.GetOrigin());
			m_aProbeMoved.Insert(-1);
		}
		m_fProbeAt = now;
	}

	//------------------------------------------------------------------------------------------------
	protected string ProbeResult()
	{
		string r = "";
		for (int i = 0; i < m_aProbe.Count(); i++)
		{
			r += RZL.F(m_aProbeMoved[i]) + " ";
			for (int g = 0; g < m_aGs.Count(); g++)
			{
				if (m_aMem[g].Contains(m_aProbe[i]))
					m_aWalk[g] = true;
			}
		}
		return r;
	}

	//------------------------------------------------------------------------------------------------
	protected string Lods()
	{
		map<int, int> hist = new map<int, int>();
		foreach (IEntity c : m_aChars)
		{
			int lod = RZL.LOD(c);
			hist.Set(lod, hist.Get(lod) + 1);
		}
		string s = "";
		for (int h = 0; h < hist.Count(); h++)
			s += string.Format("LOD%1 x%2 ", hist.GetKey(h), hist.GetElement(h));
		return s;
	}

	//------------------------------------------------------------------------------------------------
	protected void BeginStage(float now)
	{
		string st = m_aStages[m_iStage];
		m_fStageAt = now;
		m_bMeasuring = false;
		if (st == "lod 2" || st == "lod 4" || st == "lod 6")
		{
			int lod = st.Substring(4, 1).ToInt();
			ForAll(lod, 0);
		}
		if (st == "wait 1.0")
			ForAll(0, 1.0);
		if (st == "sleep")
		{
			ForAll(0, 0);
			m_aSleep.Clear();
			m_aSleepLoops.Clear();
			for (int i = 0; i < m_aGs.Count() && m_aSleep.Count() < 20; i++)
			{
				if (m_aWalk[i])
					continue;

				foreach (IEntity m : m_aMem[i])
				{
					if (!m || m_aSleep.Count() >= 20)
						continue;

					CharacterControllerComponent c = RZL_B.Ctrl(m);
					if (c)
						c.SetStanceChange(ECharacterStanceChange.STANCECHANGE_TOPRONE);
					m_aSleep.Insert(m);
				}
			}
		}
		if (st == "after")
		{
			foreach (AIWaypoint w : m_aHeld)
				RZL.Del(w);
			RZL.DelAll(m_aChars, m_aGroups);
			m_aGs.Clear();
			m_aMem.Clear();
			m_aHeld.Clear();
			m_aWalk.Clear();
			m_iSpawned = 0;
		}
		if (st == "100 on")
			SetLayer(true);
	}

	//------------------------------------------------------------------------------------------------
	//! The target agent count of a stage (spawning stages), else -1.
	protected int Want(string st)
	{
		if (st == "20 on")
			return 20;
		if (st == "60 off")
			return 60;
		if (st == "100 off")
			return 100;
		if (st == "140 on")
			return 140;
		return -1;
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		float s = Since(now);
		AIWorld aiw = GetGame().GetAIWorld();
		if (m_iStep == 0)
		{
			if (aiw.GetLimitOfActiveAIs() < 250)
				aiw.SetLimitOfActiveAIs(250);
			m_iStage = 0;
			BeginStage(now);
			Next(now);
			return;
		}

		if (m_iStage >= m_aStages.Count())
		{
			Finish();
			return;
		}

		string st = m_aStages[m_iStage];
		if (!m_bLayer)
			KeepWalkingWaypoints();
		if (m_bLayer && now - m_fLastSlots >= 1)
		{
			m_fLastSlots = now;
			WriteSlots();
		}

		// Spawning stages: one group every 0.5 s up to the count
		int want = Want(st);
		if (want >= 0 && m_iSpawned < want)
		{
			if (st.Contains(" on"))
				m_bLayer = true;
			if (s >= 0.5)
			{
				SpawnGroup(m_aGs.Count() % 2 == 1);
				m_fStepAt = now;
				m_fStageAt = now;
			}
			return;
		}

		float t = now - m_fStageAt;
		// Sleep: AI off once they are down
		if (st == "sleep" && !m_bMeasuring && t >= 4 && m_aSleepLoops.IsEmpty())
		{
			foreach (IEntity zs : m_aSleep)
			{
				AIAgent ag = RZL.Agent(zs);
				if (ag)
					ag.DeactivateAI();
				Bacon_622120A5448725E3_InfectedCharacter inf = RZL_B.Inf(zs);
				int loops = -1;
				if (inf)
					loops = inf.m_iRZLLoops;
				m_aSleepLoops.Insert(loops);
			}
		}

		if (!m_bMeasuring && t >= SETTLE)
		{
			m_bMeasuring = true;
			m_iLoops0 = RZL_Hooks.s_iLodWait;
			m_iLoiter0 = RZL_Hooks.s_iLoiter;
			m_iShooter0 = RZL_Hooks.s_iShooter;
			m_iClimb0 = RZL_Hooks.s_iClimb;
			m_iSlotMs = 0;
			m_iSlotRuns = 0;
			if (st.Contains("lod") || st.Contains("wait") || st.Contains(" on"))
				StartProbe(now);
			RZL_Frames.Start();
			return;
		}

		// The probe: when each probed body starts moving to its new slot
		for (int i = 0; i < m_aProbe.Count(); i++)
		{
			IEntity pb = m_aProbe[i];
			if (pb && m_aProbeMoved[i] < 0 && RZL.DistXZ(pb.GetOrigin(), m_aProbeSlot[i]) > 1)
				m_aProbeMoved[i] = now - m_fProbeAt;
		}

		if (!m_bMeasuring || t < SETTLE + MEASURE)
			return;

		string stats = RZL_Frames.Stop();
		float secs = MEASURE;
		int agents = 0;
		foreach (IEntity c : m_aChars)
		{
			if (c)
				agents++;
		}
		float loopsPerAgent = -1;
		if (agents > 0)
			loopsPerAgent = (RZL_Hooks.s_iLodWait - m_iLoops0) / (secs * agents);
		string hooks = string.Format("loopsPerAgentPerS=%1|loiterHookPerS=%2|shooterHookPerS=%3|climbHookPerS=%4|slotWritesMsPerRun=%5", RZL.F2(loopsPerAgent), RZL.F((RZL_Hooks.s_iLoiter - m_iLoiter0) / secs), RZL.F((RZL_Hooks.s_iShooter - m_iShooter0) / secs), RZL.F((RZL_Hooks.s_iClimb - m_iClimb0) / secs), RZL.F2(m_iSlotMs / Math.Max(1, m_iSlotRuns)));
		RZL.Row("b15", string.Format("stage=%1|agents=%2|layer=%3|activeAIs=%4|lods=%5|", st, agents, m_bLayer, aiw.GetCurrentNumOfActiveAIs(), Lods()) + stats + "|" + hooks);
		if (!m_aProbe.IsEmpty())
		{
			RZL.Row("b12", string.Format("stage=%1|lods=%2|orderLatencyS=%3|loopsPerAgentPerS=%4|", st, Lods(), ProbeResult(), RZL.F2(loopsPerAgent)) + stats);
			m_aProbe.Clear();
		}
		if (st == "sleep")
		{
			int stillLooping;
			for (int k = 0; k < m_aSleep.Count(); k++)
			{
				Bacon_622120A5448725E3_InfectedCharacter infk = RZL_B.Inf(m_aSleep[k]);
				if (infk && k < m_aSleepLoops.Count() && infk.m_iRZLLoops > m_aSleepLoops[k] + 2)
					stillLooping++;
			}
			int prone;
			foreach (IEntity zp : m_aSleep)
			{
				CharacterControllerComponent cp = RZL_B.Ctrl(zp);
				if (cp && cp.GetStance() == ECharacterStance.PRONE)
					prone++;
			}
			RZL.Row("b12", string.Format("stage=sleep (%1 of %2 bodies told prone, then DeactivateAI)|proneWhileOff=%3|treeStillLoopingWhileOff=%4 of %1|", m_aSleep.Count(), agents, prone, stillLooping) + stats);
			// Wake them: AI on, a slot 10 m away
			m_aWakeLoop.Clear();
			m_aWakeStand.Clear();
			m_aWakeMove.Clear();
			m_aWakePos.Clear();
			for (int w = 0; w < m_aSleep.Count(); w++)
			{
				IEntity zw = m_aSleep[w];
				AIAgent aw = RZL.Agent(zw);
				if (aw)
					aw.ActivateAI();
				RZL.ForceLOD(zw, 0);
				Bacon_622120A5448725E3_InfectedCharacter infw = RZL_B.Inf(zw);
				if (infw)
					m_aSleepLoops[w] = infw.m_iRZLLoops;
				vector wp = vector.Zero;
				if (zw)
				{
					wp = zw.GetOrigin();
					RZL_B.Steer(zw, wp + vector.FromYaw(m_Site.m_fYaw) * 10);
				}
				m_aWakePos.Insert(wp);
				m_aWakeLoop.Insert(-1);
				m_aWakeStand.Insert(-1);
				m_aWakeMove.Insert(-1);
			}
			m_fWakeAt = now;
			m_iStep = 5;
			m_fStepAt = now;
			m_iStage++;
			return;
		}
		m_iStage++;
		if (m_iStage < m_aStages.Count())
			BeginStage(now);
	}

	//------------------------------------------------------------------------------------------------
	//! After sleep: how long until each body's tree loops, it stands, and it moves.
	protected void WatchWake(float now)
	{
		float t = now - m_fWakeAt;
		for (int i = 0; i < m_aSleep.Count(); i++)
		{
			IEntity z = m_aSleep[i];
			if (!z)
				continue;

			Bacon_622120A5448725E3_InfectedCharacter inf = RZL_B.Inf(z);
			if (m_aWakeLoop[i] < 0 && inf && inf.m_iRZLLoops > m_aSleepLoops[i])
				m_aWakeLoop[i] = t;
			CharacterControllerComponent c = RZL_B.Ctrl(z);
			if (m_aWakeStand[i] < 0 && c && c.GetStance() == ECharacterStance.STAND)
				m_aWakeStand[i] = t;
			if (m_aWakeMove[i] < 0 && RZL.DistXZ(z.GetOrigin(), m_aWakePos[i]) > 1)
				m_aWakeMove[i] = t;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static string Median(array<float> a)
	{
		array<float> v = {};
		int never;
		foreach (float x : a)
		{
			if (x >= 0)
				v.Insert(x);
			else
				never++;
		}
		if (v.IsEmpty())
			return "never";

		v.Sort();
		return string.Format("%1 (max %2, never %3)", RZL.F(v[v.Count() / 2]), RZL.F(v[v.Count() - 1]), never);
	}

	//------------------------------------------------------------------------------------------------
	override protected void Finish()
	{
		foreach (AIWaypoint w : m_aHeld)
			RZL.Del(w);
		super.Finish();
	}

	//------------------------------------------------------------------------------------------------
	//! Step 5 is the wake watch (20 s), then the stages go on.
	void TickWake(float now)
	{
		WatchWake(now);
		if (now - m_fWakeAt < 20)
			return;

		RZL.Row("b12", string.Format("stage=wake (ActivateAI, slot 10 m away)|treeLoopsAgainAfter=%1|standingAfter=%2|movingAfter=%3", Median(m_aWakeLoop), Median(m_aWakeStand), Median(m_aWakeMove)));
		foreach (IEntity z : m_aSleep)
		{
			// back to idle
			RZL_B.Mode(z, 0);
			RZL_B.ClearSlot(z);
		}
		m_aSleep.Clear();
		m_iStep = 1;
		if (m_iStage < m_aStages.Count())
			BeginStage(now);
	}
}

//------------------------------------------------------------------------------------------------
//! B15 with the wake watch hooked in (step 5).
class RZL_ExpB15Run : RZL_ExpB15
{
	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		if (m_iStep == 5)
		{
			TickWake(now);
			return;
		}

		super.Tick(now);
	}
}
