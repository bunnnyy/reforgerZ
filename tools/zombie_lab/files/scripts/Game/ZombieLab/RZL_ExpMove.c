// Test-only (not part of the mod): one Bacon group on waypoints for 30 minutes, and speeds.

//------------------------------------------------------------------------------------------------
//! 2. One runtime group of 4 for 30 minutes: a long move order, idle without a waypoint, a fight on
//! the way to a second waypoint, then the held waypoint (one entity, moved and re-added each order).
class RZL_ExpWaypoint : RZL_Exp
{
	static const float LENGTH = 1800;

	protected SCR_AIGroup m_G;
	protected ref array<IEntity> m_aMembers = {};
	protected ref array<IEntity> m_aWps = {};
	protected AIWaypoint m_Wp;
	protected AIWaypoint m_Held;
	protected IEntity m_Target;
	protected vector m_vGoal;
	protected vector m_vGoal2;
	protected vector m_vStart;
	protected vector m_vRest;
	protected float m_fArrived = -1;
	protected float m_fCompleted = -1;
	protected float m_fTargetSeen = -1;
	protected float m_fTargetDied = -1;
	protected float m_fResumed = -1;
	protected float m_fMaxDrift;
	protected int m_iSamples;
	protected int m_iMovingSamples;
	protected int m_iOrder;
	protected float m_fLastLog;
	protected vector m_vLastCentroid;
	protected float m_fLastCentroidAt;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpWaypoint()
	{
		m_sName = "waypoint";
	}

	//------------------------------------------------------------------------------------------------
	protected vector Centroid()
	{
		vector sum;
		int n;
		foreach (IEntity m : m_aMembers)
		{
			if (m && RZL.Alive(m))
			{
				sum = sum + m.GetOrigin();
				n++;
			}
		}
		if (n == 0)
			return RZL.GroupPos(m_G);

		return sum * (1.0 / n);
	}

	//------------------------------------------------------------------------------------------------
	//! Centroid speed (m/s) since the last call, at most once a second.
	protected float CentroidSpeed(float now)
	{
		vector c = Centroid();
		float dt = now - m_fLastCentroidAt;
		if (dt < 1)
			return -1;

		float v = RZL.DistXZ(c, m_vLastCentroid) / dt;
		m_vLastCentroid = c;
		m_fLastCentroidAt = now;
		return v;
	}

	//------------------------------------------------------------------------------------------------
	protected void Track(float now, string phase, vector goal)
	{
		if (now - m_fLastLog < 10)
			return;

		m_fLastLog = now;
		int alive;
		float spread;
		vector c = Centroid();
		foreach (IEntity m : m_aMembers)
		{
			if (m && RZL.Alive(m))
			{
				alive++;
				spread = Math.Max(spread, RZL.DistXZ(m.GetOrigin(), c));
			}
		}
		int agents = -1;
		if (m_G)
			agents = m_G.GetAgentsCount();
		RZL.Log(string.Format("W t=%1 phase=%2 distGoal=%3 alive=%4 agents=%5 spread=%6 wpList=%7 tod=%8", RZL.F(now - m_fBegin), phase, RZL.F(RZL.DistXZ(c, goal)), alive, agents, RZL.F(spread), RZL.WaypointCount(m_G), RZL.TimeOfDay()));
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
		float total = now - m_fBegin;
		if (!m_G && m_iStep > 0 && m_iStep < 9)
		{
			RZL.Row("waypoint", "phase=error|note=the group is gone");
			Finish();
			return;
		}

		if (m_iStep == 0)
		{
			m_G = RZL.SpawnEmptyGroup(m_Site.m_vT);
			m_aGroups.Insert(m_G);
			for (int i = 0; i < 4; i++)
			{
				IEntity ch = RZL.SpawnMember(m_G, RZL.CIV, m_Site.Side(2 * i, 2), m_Site.m_fYaw);
				m_aMembers.Insert(ch);
				m_aChars.Insert(ch);
			}
			Next(now);
			return;
		}

		// A. One move order 300 m away (the mod's SendGroupTo)
		if (m_iStep == 1 && s >= 3)
		{
			foreach (IEntity mm : m_aMembers)
				RZL.ForceLOD(mm, 0);
			m_vStart = Centroid();
			m_vGoal = m_Site.Along(300);
			RZ_Util.SendGroupTo(m_G, m_vGoal, RZL.WAYPOINT);
			m_Wp = LastWaypoint();
			m_aWps.Insert(m_Wp);
			Next(now);
			return;
		}

		if (m_iStep == 2)
		{
			Track(now, "A move 300 m", m_vGoal);
			if (m_fArrived < 0 && RZL.DistXZ(Centroid(), m_vGoal) < 10)
				m_fArrived = s;
			if (m_fCompleted < 0 && !RZL.HasWaypoint(m_G, m_Wp))
				m_fCompleted = s;
			if ((m_fCompleted >= 0 && s - m_fCompleted > 5) || s > 240)
			{
				float speed = -1;
				if (m_fArrived > 0)
					speed = (RZL.DistXZ(m_vStart, m_vGoal) - 10) / m_fArrived;
				RZL.Row("waypoint", string.Format("phase=A one move order 300 m|arrivedAt=%1|completedAt=%2|avgSpeed=%3|completedWaypointStillInWorld=%4|listAfter=%5", RZL.F(m_fArrived), RZL.F(m_fCompleted), RZL.F(speed), m_Wp != null, RZL.WaypointCount(m_G)));
				m_vRest = Centroid();
				m_vLastCentroid = m_vRest;
				m_fLastCentroidAt = now;
				Next(now);
			}
			return;
		}

		// B. Idle with no waypoint, 5 minutes
		if (m_iStep == 3)
		{
			Track(now, "B idle, no waypoint", m_vRest);
			foreach (IEntity m3 : m_aMembers)
			{
				if (m3)
					m_fMaxDrift = Math.Max(m_fMaxDrift, RZL.DistXZ(m3.GetOrigin(), m_vRest));
			}
			float v = CentroidSpeed(now);
			if (v >= 0)
			{
				m_iSamples++;
				if (v > 0.3)
					m_iMovingSamples++;
			}
			if (s >= 300 || (RZL_LabComponent.s_bQuick && s >= 60))
			{
				RZL.Row("waypoint", string.Format("phase=B idle 5 min, no waypoint|maxMemberDrift=%1|centroidDrift=%2|secondsCentroidMoving=%3 of %4", RZL.F(m_fMaxDrift), RZL.F(RZL.DistXZ(Centroid(), m_vRest)), m_iMovingSamples, m_iSamples));
				Next(now);
			}
			return;
		}

		// C. A fight on the way: order back to the start, a target standing 70 m along the path
		if (m_iStep == 4)
		{
			m_vGoal2 = m_Site.Along(10);
			RZ_Util.SendGroupTo(m_G, m_vGoal2, RZL.WAYPOINT);
			m_Wp = LastWaypoint();
			m_aWps.Insert(m_Wp);
			m_Target = RZL.Spawn(RZL.TARGET, m_Site.Along(220), m_Site.m_fYaw);
			m_aChars.Insert(m_Target);
			m_fCompleted = -1;
			m_vLastCentroid = Centroid();
			m_fLastCentroidAt = now;
			Next(now);
			return;
		}

		if (m_iStep == 5)
		{
			Track(now, "C fight on the way", m_vGoal2);
			if (m_fTargetSeen < 0)
			{
				foreach (IEntity m5 : m_aMembers)
				{
					if (RZL.Category(m5, m_Target) >= 0)
					{
						m_fTargetSeen = s;
						RZL.Log(string.Format("W target seen at %1 m", RZL.F(RZL.DistXZ(m5.GetOrigin(), m_Target.GetOrigin()))));
						break;
					}
				}
			}
			if (m_fTargetDied < 0 && m_Target && !RZL.Alive(m_Target))
			{
				m_fTargetDied = s;
				m_vLastCentroid = Centroid();
				m_fLastCentroidAt = now;
			}
			if (m_fCompleted < 0 && !RZL.HasWaypoint(m_G, m_Wp))
				m_fCompleted = s;
			if (m_fTargetDied >= 0 && m_fResumed < 0)
			{
				// resumed = the centroid closing on the waypoint at 1 m/s or more
				vector c0 = m_vLastCentroid;
				float before = RZL.DistXZ(c0, m_vGoal2);
				float sp = CentroidSpeed(now);
				if (sp > 1 && RZL.DistXZ(Centroid(), m_vGoal2) < before - 1)
					m_fResumed = s;
			}
			bool timeout = s > 480;
			if ((m_fTargetDied >= 0 && m_fCompleted >= 0) || timeout)
			{
				RZL.Row("waypoint", string.Format("phase=C fight on the way to a waypoint|targetSeenAt=%1|targetDiedAt=%2|resumedTowardWaypointAt=%3|waypointCompletedAt=%4|distToWaypointNow=%5|timedOut=%6", RZL.F(m_fTargetSeen), RZL.F(m_fTargetDied), RZL.F(m_fResumed), RZL.F(m_fCompleted), RZL.F(RZL.DistXZ(Centroid(), m_vGoal2)), timeout));
				RZL.Del(m_Target);
				Next(now);
			}
			return;
		}

		// E. The held waypoint: one entity, moved and re-added on each order, 8 orders
		if (m_iStep == 6)
		{
			if (!m_Held)
			{
				m_Held = RZL.NewWaypoint(m_Site.m_vT);
				m_aWps.Insert(m_Held);
			}

			if (m_iOrder > 0)
			{
				bool reached = RZL.DistXZ(Centroid(), m_vGoal) < 12;
				if (reached && m_fArrived < 0)
					m_fArrived = s;
				if (m_fCompleted < 0 && !RZL.HasWaypoint(m_G, m_Held))
					m_fCompleted = s;
				Track(now, "E held waypoint order " + m_iOrder.ToString(), m_vGoal);
				if (s < 75)
					return;

				RZL.Row("held", string.Format("order=%1|dist=%2|reachedAt=%3|completedAt=%4|stillOneEntity=%5|listCount=%6", m_iOrder, RZL.F(RZL.DistXZ(m_vStart, m_vGoal)), RZL.F(m_fArrived), RZL.F(m_fCompleted), m_Held != null, RZL.WaypointCount(m_G)));
			}

			if (m_iOrder >= 8 || (RZL_LabComponent.s_bQuick && m_iOrder >= 2))
			{
				m_iOrder = 8;
				Next(now);
				return;
			}

			m_iOrder++;
			float along = 100;
			float lateral = 0;
			if (m_iOrder % 2 == 0)
				along = 250;
			if (m_iOrder % 3 == 0)
				lateral = 50;
			m_vStart = Centroid();
			m_vGoal = m_Site.Side(along, lateral);
			RZL.MoveHeld(m_G, m_Held, m_vGoal);
			m_fArrived = -1;
			m_fCompleted = -1;
			m_fStepAt = now;
			return;
		}

		// E2. Move the current waypoint without removing and re-adding it
		if (m_iStep == 7)
		{
			if (m_iOrder == 8)
			{
				m_vGoal = m_Site.Along(20);
				RZL.MoveHeld(m_G, m_Held, m_vGoal);
				m_iOrder++;
				m_fStepAt = now;
				return;
			}

			if (m_iOrder == 9 && s >= 8)
			{
				m_vGoal2 = m_Site.Side(150, -60);
				m_Held.SetOrigin(m_vGoal2);
				m_Held.Update();
				m_iOrder++;
				m_fStepAt = now;
				return;
			}

			if (m_iOrder == 10 && s >= 60)
			{
				vector c7 = Centroid();
				RZL.Row("held", string.Format("order=move the current waypoint without re-adding|distToOldPos=%1|distToNewPos=%2|inList=%3", RZL.F(RZL.DistXZ(c7, m_vGoal)), RZL.F(RZL.DistXZ(c7, m_vGoal2)), RZL.HasWaypoint(m_G, m_Held)));
				Next(now);
			}
			return;
		}

		// Idle until 30 minutes are up, then the leak count and DeleteUnit with the held waypoint
		if (m_iStep == 8)
		{
			Track(now, "F idle until 30 min", m_vGoal2);
			if (total < LENGTH && !(RZL_LabComponent.s_bQuick && total > 600))
				return;

			RZL.Row("waypoint", string.Format("phase=end after 30 min|membersAlive=%1|groupExists=%2|waypointsCreated=%3|stillInWorld=%4|inList=%5", RZL.CountAlive(m_aMembers), m_G != null, m_aWps.Count(), RZL.CountAlive(m_aWps), RZL.WaypointCount(m_G)));
			RZ_Util.DeleteUnit(m_G);
			Next(now);
			return;
		}

		if (m_iStep == 9 && s >= 3)
		{
			RZL.Row("waypoint", string.Format("phase=after RZ_Util.DeleteUnit|groupExists=%1|heldWaypointStillInWorld=%2", m_G != null, m_Held != null));
			foreach (IEntity w : m_aWps)
				RZL.Del(w);
			Finish();
		}
	}
}

//------------------------------------------------------------------------------------------------
//! 4. Speeds on a move waypoint and in a chase, per prefab type.
class RZL_ExpSpeeds : RZL_Exp
{
	protected ref array<string> m_aTypes = {RZL.CIV, RZL.CIV_BASE, RZL.MIL, RZL.RUNNER, RZL.HEAVY, RZL.BOMBER};
	protected ref array<string> m_aNames = {"civilian (Variant_CIV_Regular1)", "civilian (Zombie_CIV)", "military (Variant_US_Rifleman)", "exploder (Variant_Exploder1)", "heavy (Zombie_Heavy_Tank)", "bomber (Character_Bomber)"};
	protected int m_iType;
	protected IEntity m_Ch;
	protected SCR_AIGroup m_G;
	protected AIWaypoint m_Wp;
	protected IEntity m_Target;
	protected ref RZL_Mover m_Mover;
	protected vector m_vGoal;
	protected float m_fPath;
	protected float m_fPathT;
	protected bool m_bIn;
	protected float m_fMax;
	protected float m_fWpSpeed;
	protected float m_fWpMax;
	protected float m_fDetect;
	protected float m_fChasePath;
	protected float m_fChaseT;
	protected float m_fChaseMax;
	protected float m_fContact;
	protected vector m_vPrev;
	protected float m_fLastTick;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpSpeeds()
	{
		m_sName = "speeds";
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		float s = Since(now);
		if (m_iStep == 0)
		{
			if (m_iType >= m_aTypes.Count())
			{
				Finish();
				return;
			}

			m_Ch = RZL.SpawnSolo(m_aTypes[m_iType], m_Site.m_vT, m_Site.m_fYaw, m_aGroups);
			m_G = SCR_AIGroup.Cast(m_aGroups[m_aGroups.Count() - 1]);
			m_aChars.Insert(m_Ch);
			m_fPath = 0;
			m_fPathT = 0;
			m_bIn = false;
			m_fMax = 0;
			m_fDetect = -1;
			m_fChasePath = 0;
			m_fChaseT = 0;
			m_fChaseMax = 0;
			m_fContact = -1;
			m_fWpSpeed = -1;
			m_fWpMax = 0;
			Next(now);
			return;
		}

		if (m_iStep == 1 && s >= 3)
		{
			m_vGoal = m_Site.Along(160);
			m_Wp = RZL.NewWaypoint(m_vGoal);
			if (m_G)
				m_G.AddWaypoint(m_Wp);
			m_Mover = new RZL_Mover(m_Ch);
			m_vPrev = m_Ch.GetOrigin();
			m_fLastTick = now;
			Next(now);
			return;
		}

		if (m_iStep == 2)
		{
			if (!m_Ch)
			{
				Next(now);
				return;
			}

			float tickDt2 = now - m_fLastTick;
			m_fLastTick = now;
			m_Mover.Sample();
			m_fMax = Math.Max(m_fMax, m_Mover.m_fSpeed);
			float d = RZL.DistXZ(m_Ch.GetOrigin(), m_vGoal);
			float step = RZL.DistXZ(m_Ch.GetOrigin(), m_vPrev);
			m_vPrev = m_Ch.GetOrigin();
			if (d < 140 && d > 25)
			{
				m_bIn = true;
				m_fPath += step;
				m_fPathT += tickDt2;
				m_fWpMax = Math.Max(m_fWpMax, m_Mover.m_fSpeed);
			}
			if ((m_bIn && d <= 25) || s > 90)
			{
				if (m_fPathT > 0)
					m_fWpSpeed = m_fPath / m_fPathT;
				// Chase: a target 50 m on
				m_Target = RZL.Spawn(RZL.TARGET, RZL.At(m_Ch.GetOrigin(), m_Site.m_fYaw, 50), m_Site.m_fYaw);
				m_aChars.Insert(m_Target);
				m_vPrev = m_Ch.GetOrigin();
				Next(now);
			}
			return;
		}

		if (m_iStep == 3)
		{
			if (!m_Ch || !m_Target)
			{
				Next(now);
				return;
			}

			float tickDt3 = now - m_fLastTick;
			m_fLastTick = now;
			m_Mover.Sample();
			float dt = RZL.DistXZ(m_Ch.GetOrigin(), m_Target.GetOrigin());
			float step3 = RZL.DistXZ(m_Ch.GetOrigin(), m_vPrev);
			m_vPrev = m_Ch.GetOrigin();
			if (m_fDetect < 0 && RZL.Category(m_Ch, m_Target) >= 0)
				m_fDetect = dt;
			if (m_fDetect >= 0 && dt < 45 && dt > 4 && m_Mover.Toward(m_Target.GetOrigin(), 1.5))
			{
				m_fChasePath += step3;
				m_fChaseT += tickDt3;
				m_fChaseMax = Math.Max(m_fChaseMax, m_Mover.m_fSpeed);
			}
			if (m_fContact < 0 && dt < 3)
				m_fContact = s;
			if (m_fContact >= 0 || s > 45)
			{
				float chase = -1;
				if (m_fChaseT > 0)
					chase = m_fChasePath / m_fChaseT;
				RZL.Row("speeds", string.Format("type=%1|waypointSpeed=%2|waypointMax=%3|chaseSpeed=%4|chaseMax=%5|seenAt=%6|contactAfter=%7", m_aNames[m_iType], RZL.F(m_fWpSpeed), RZL.F(m_fWpMax), RZL.F(chase), RZL.F(m_fChaseMax), RZL.F(m_fDetect), RZL.F(m_fContact)));
				Next(now);
			}
			return;
		}

		if (m_iStep == 4)
		{
			RZL.Del(m_Wp);
			RZL.DelAll(m_aChars, m_aGroups);
			m_iType++;
			m_iStep = 5;
			m_fStepAt = now;
			return;
		}

		if (m_iStep == 5 && s >= 3)
		{
			m_iStep = 0;
			m_fStepAt = now;
		}
	}
}
