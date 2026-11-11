// Test-only (not part of the mod): behaviour tests about noticing, releasing and fighting:
// B1 blind spawn, B4 re-order mid-fight, B5 take-back, B6 release injection, B14 attack pieces,
// B18 hearing with the intake gate (/root/rz_tools/behaviour_addendum.md, section 15).

//------------------------------------------------------------------------------------------------
//! B1. A body spawned after its target is in reach, blind from its spawn frame (factor 0, intake
//! closed, holding its spawn spot): does it notice at 150/50/15/6 m within 30 s? At 6 m it is then
//! released (our contact rule) and must engage. Control: factor 1, not steered (phase 0's charge).
class RZL_ExpB1 : RZL_BExp
{
	static const float HOLD = 30;
	static const float RELEASE_AT = 20;

	protected ref array<float> m_aDist = {};
	protected ref array<bool> m_aControl = {};
	protected int m_iTrial;
	protected IEntity m_Target;
	protected IEntity m_Obs;
	protected ref RZL_Mover m_Mover;
	protected float m_fShownAt;
	protected float m_fLastFactor;
	protected string m_sPath;
	protected float m_fSeen;
	protected float m_fEnemy;
	protected float m_fBcast;
	protected float m_fCharge;
	protected float m_fMinDist;
	protected float m_fRelEngaged;
	protected float m_fRelContact;
	protected bool m_bReleased;

	//------------------------------------------------------------------------------------------------
	//! Runner k of n takes every n-th trial.
	void RZL_ExpB1(int k, int n)
	{
		m_sName = "b1";
		array<float> cells = {150, 50, 15, 6};
		int per = 6;
		int controls = 2;
		if (RZL_LabComponent.s_bQuick)
		{
			per = 1;
			controls = 1;
		}
		int idx;
		for (int rep = 0; rep < per; rep++)
		{
			foreach (float d : cells)
			{
				if (idx % n == k)
				{
					m_aDist.Insert(d);
					m_aControl.Insert(false);
				}
				idx++;
			}
		}
		for (int c = 0; c < controls; c++)
		{
			if (idx % n == k)
			{
				m_aDist.Insert(50);
				m_aControl.Insert(true);
			}
			idx++;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		float s = Since(now);
		if (m_iStep == 0)
		{
			if (m_iTrial >= m_aDist.Count())
			{
				Finish();
				return;
			}

			// The target first, so the body is spawned with the target already in reach
			m_Target = RZL_B.Target(m_Site.m_vT, m_Site.m_fYaw, true);
			m_aChars.Insert(m_Target);
			Next(now);
			return;
		}

		if (m_iStep == 1 && s >= 3)
		{
			float d = m_aDist[m_iTrial];
			vector at = m_Site.Along(d);
			float yaw = RZL.YawTo(at, m_Site.m_vT);
			if (m_aControl[m_iTrial])
			{
				m_Obs = RZL.SpawnSolo(RZL.CIV, at, yaw, m_aGroups);
				m_sPath = "control (factor 1)";
			}
			else
			{
				string path;
				m_Obs = RZL_B.SpawnSteered(RZL.CIV, at, yaw, at, m_aGroups, path);
				m_sPath = path;
			}
			m_aChars.Insert(m_Obs);
			m_Mover = new RZL_Mover(m_Obs);
			m_fShownAt = now;
			m_fLastFactor = now;
			m_fSeen = -1;
			m_fEnemy = -1;
			m_fBcast = -1;
			m_fCharge = -1;
			m_fMinDist = d;
			m_fRelEngaged = -1;
			m_fRelContact = -1;
			m_bReleased = false;
			Next(now);
			return;
		}

		if (m_iStep == 2)
		{
			float t = now - m_fShownAt;
			bool control = m_aControl[m_iTrial];
			float dist = m_aDist[m_iTrial];
			if (!m_Obs || !m_Target)
			{
				Next(now);
				return;
			}

			if (!control && !m_bReleased && now - m_fLastFactor >= 1)
			{
				RZL_B.Factor(m_Obs, 0);
				m_fLastFactor = now;
			}

			m_Mover.Sample();
			float dNow = RZL.DistXZ(m_Obs.GetOrigin(), m_Target.GetOrigin());
			m_fMinDist = Math.Min(m_fMinDist, dNow);
			if (!m_bReleased)
			{
				if (m_fSeen < 0 && RZL.Seen(m_Obs, m_Target))
					m_fSeen = t;
				if (m_fEnemy < 0 && RZL.Category(m_Obs, m_Target) == ETargetCategory.ENEMY)
					m_fEnemy = t;
				Bacon_622120A5448725E3_InfectedCharacter inf = RZL_B.Inf(m_Obs);
				if (m_fBcast < 0 && inf && inf.m_RZLBroadcastTarget == m_Target && inf.m_fRZLBroadcastAt >= RZL_Hooks.Now() - t)
					m_fBcast = t;
				if (m_fCharge < 0 && t > 0.5 && m_Mover.Toward(m_Target.GetOrigin(), 2.5))
					m_fCharge = t;
			}

			// Our contact release at 6 m
			if (!control && dist < 8 && !m_bReleased && t >= RELEASE_AT)
			{
				RZL_B.Release(m_Obs, m_Target, true);
				m_bReleased = true;
			}
			if (m_bReleased)
			{
				float tr = t - RELEASE_AT;
				if (m_fRelEngaged < 0 && RZL_B.Engaged(m_Obs, m_Target))
					m_fRelEngaged = tr;
				if (m_fRelContact < 0 && dNow < 1.6)
					m_fRelContact = tr;
			}

			float hold = HOLD;
			if (control)
				hold = 20;
			if (t < hold)
				return;

			string cell = string.Format("%1 m", dist);
			if (control)
				cell = "50 m control";
			RZL.Row("b1", string.Format("site=%1|cell=%2|factorPath=%3|seenAfter=%4|enemyAfter=%5|baconTargetAfter=%6|chargedAfter=%7|minDist=%8|releasedEngagedAfter=%9", m_Site.m_sName, cell, m_sPath, RZL.F(m_fSeen), RZL.F(m_fEnemy), RZL.F(m_fBcast), RZL.F(m_fCharge), RZL.F(m_fMinDist), RZL.F(m_fRelEngaged)) + string.Format("|releasedContactAfter=%1|lod=%2", RZL.F(m_fRelContact), RZL.LOD(m_Obs)));
			Next(now);
			return;
		}

		if (m_iStep == 3)
		{
			RZL.DelAll(m_aChars, m_aGroups);
			m_iTrial++;
			Next(now);
			return;
		}

		if (m_iStep == 4 && s >= 2)
		{
			m_iStep = 0;
			m_fStepAt = now;
		}
	}
}

//------------------------------------------------------------------------------------------------
//! B6. A steered body 30 m from a target (spawned before it), released after 5 s blind: factor back
//! plus our injected share event, against the factor back alone. Time to engage.
class RZL_ExpB6 : RZL_BExp
{
	protected int m_iTrial;
	protected int m_iTrials = 6;
	protected ref array<float> m_aRelDist = {30, 30, 15, 15, 10, 10};
	protected IEntity m_Obs;
	protected IEntity m_Target;
	protected ref RZL_Mover m_Mover;
	protected float m_fRelAt;
	protected float m_fBlindEngaged;
	protected float m_fEngaged;
	protected float m_fCharge;
	protected float m_fContact;
	protected float m_fMinDist;
	protected float m_fSpeedSum;
	protected int m_iSpeedN;
	protected string m_sPath;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpB6()
	{
		m_sName = "b6";
		if (RZL_LabComponent.s_bQuick)
			m_iTrials = 2;
		if (m_iTrials > m_aRelDist.Count())
			m_iTrials = m_aRelDist.Count();
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		float s = Since(now);
		bool inject = m_iTrial % 2 == 0;
		if (m_iStep == 0)
		{
			if (m_iTrial >= m_iTrials)
			{
				Finish();
				return;
			}

			vector at = m_Site.Along(m_aRelDist[m_iTrial]);
			string path;
			m_Obs = RZL_B.SpawnSteered(RZL.CIV, at, RZL.YawTo(at, m_Site.m_vT), at, m_aGroups, path);
			m_sPath = path;
			m_aChars.Insert(m_Obs);
			m_Target = RZL_B.Target(m_Site.Side(150, 1500), 0, true);
			m_aChars.Insert(m_Target);
			Next(now);
			return;
		}

		if (m_iStep == 1 && s >= 3)
		{
			RZL.Teleport(m_Target, m_Site.m_vT, m_Site.m_fYaw);
			m_fBlindEngaged = -1;
			m_fEngaged = -1;
			m_fCharge = -1;
			m_fContact = -1;
			m_Mover = new RZL_Mover(m_Obs);
			Next(now);
			return;
		}

		if (m_iStep == 2)
		{
			if (!m_Obs || !m_Target)
			{
				m_iStep = 4;
				return;
			}

			if (m_fBlindEngaged < 0 && RZL_B.Engaged(m_Obs, m_Target))
				m_fBlindEngaged = s;
			if (s >= 5)
			{
				RZL_B.Release(m_Obs, m_Target, inject);
				m_fRelAt = now;
				m_fMinDist = 1000;
				m_fSpeedSum = 0;
				m_iSpeedN = 0;
				Next(now);
			}
			return;
		}

		if (m_iStep == 3)
		{
			float t = now - m_fRelAt;
			m_Mover.Sample();
			if (!m_Obs || !m_Target)
			{
				Next(now);
				return;
			}

			if (m_fEngaged < 0 && RZL_B.Engaged(m_Obs, m_Target))
				m_fEngaged = t;
			if (m_fCharge < 0 && m_Mover.Toward(m_Target.GetOrigin(), 2.5))
				m_fCharge = t;
			float dt6 = RZL.DistXZ(m_Obs.GetOrigin(), m_Target.GetOrigin());
			m_fMinDist = Math.Min(m_fMinDist, dt6);
			m_fSpeedSum += m_Mover.m_fSpeed;
			m_iSpeedN++;
			if (m_fContact < 0 && dt6 < 2)
				m_fContact = t;
			if (t < 20 && m_fContact < 0)
				return;

			string how = "factor back only";
			if (inject)
				how = "factor back + injected share";
			float meanV = -1;
			if (m_iSpeedN > 0)
				meanV = m_fSpeedSum / m_iSpeedN;
			RZL.Row("b6", string.Format("dist=" + m_aRelDist[m_iTrial].ToString() + "|release=%1|engagedAfter=%2|chargedAfter=%3|contactAfter=%4|engagedWhileBlind=%5|minDistIn20s=%6|meanSpeed=%7|factorPath=%8", how, RZL.F(m_fEngaged), RZL.F(m_fCharge), RZL.F(m_fContact), RZL.F(m_fBlindEngaged), RZL.F(m_fMinDist), RZL.F2(meanV), m_sPath));
			Next(now);
			return;
		}

		if (m_iStep == 4)
		{
			RZL.DelAll(m_aChars, m_aGroups);
			m_iTrial++;
			Next(now);
			return;
		}

		if (m_iStep == 5 && s >= 2)
		{
			m_iStep = 0;
			m_fStepAt = now;
		}
	}
}

//------------------------------------------------------------------------------------------------
//! B4. A group of 3 fighting a target (that can't be hurt) gets a new waypoint 150 m away: does it
//! leave the fight? Then the target dies: how soon do they take the waypoint?
class RZL_ExpB4 : RZL_BExp
{
	protected int m_iTrial;
	protected int m_iTrials = 3;
	protected SCR_AIGroup m_G;
	protected ref array<IEntity> m_aM = {};
	protected IEntity m_Target;
	protected AIWaypoint m_Wp;
	protected vector m_vWp;
	protected float m_fEngagedAt;
	protected float m_fOrderAt;
	protected float m_fKillAt;
	protected float m_fMaxFromTarget;
	protected int m_iLeft;
	protected ref array<float> m_aToWp0 = {};
	protected ref array<float> m_aTook = {};
	protected float m_fCompleted;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpB4()
	{
		m_sName = "b4";
		if (RZL_LabComponent.s_bQuick)
			m_iTrials = 1;
	}

	//------------------------------------------------------------------------------------------------
	protected int CloseCount(float r)
	{
		int n;
		foreach (IEntity m : m_aM)
		{
			if (m && m_Target && RZL.DistXZ(m.GetOrigin(), m_Target.GetOrigin()) < r)
				n++;
		}
		return n;
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		float s = Since(now);
		if (m_iStep == 0)
		{
			if (m_iTrial >= m_iTrials)
			{
				Finish();
				return;
			}

			vector at = m_Site.Along(30);
			m_G = RZL.SpawnEmptyGroup(at);
			m_aGroups.Insert(m_G);
			m_aM.Clear();
			for (int i = 0; i < 3; i++)
			{
				IEntity ch = RZL.SpawnMember(m_G, RZL.CIV, m_Site.Side(30, (i - 1) * 2.5), m_Site.m_fYaw + 180);
				RZL.ForceLOD(ch, 0);
				m_aM.Insert(ch);
				m_aChars.Insert(ch);
			}
			m_Target = RZL_B.Target(m_Site.Side(150, 1500), 0, true);
			m_aChars.Insert(m_Target);
			Next(now);
			return;
		}

		if (m_iStep == 1 && s >= 3)
		{
			RZL.Teleport(m_Target, m_Site.m_vT, m_Site.m_fYaw);
			m_fEngagedAt = -1;
			Next(now);
			return;
		}

		// Wait for the fight: two or more within 3 m
		if (m_iStep == 2)
		{
			if (m_fEngagedAt < 0 && CloseCount(3) >= 2)
				m_fEngagedAt = s;
			if (m_fEngagedAt >= 0 && s - m_fEngagedAt >= 8)
			{
				m_vWp = m_Site.Side(0, 150);
				m_Wp = RZL.NewWaypoint(m_vWp);
				m_G.AddWaypoint(m_Wp);
				m_fOrderAt = now;
				m_fMaxFromTarget = 0;
				m_iLeft = 0;
				Next(now);
				return;
			}
			if (s > 45)
			{
				RZL.Row("b4", string.Format("trial=%1|note=no fight within 45 s (%2 within 3 m)", m_iTrial, CloseCount(3)));
				m_iStep = 5;
			}
			return;
		}

		// 30 s with the new waypoint: does anyone leave?
		if (m_iStep == 3)
		{
			foreach (IEntity m3 : m_aM)
			{
				if (m3 && m_Target)
					m_fMaxFromTarget = Math.Max(m_fMaxFromTarget, RZL.DistXZ(m3.GetOrigin(), m_Target.GetOrigin()));
			}
			if (now - m_fOrderAt < 30)
				return;

			m_iLeft = 3 - CloseCount(15);
			m_aToWp0.Clear();
			m_aTook.Clear();
			foreach (IEntity m4 : m_aM)
			{
				float dw = -1;
				if (m4)
					dw = RZL.DistXZ(m4.GetOrigin(), m_vWp);
				m_aToWp0.Insert(dw);
				m_aTook.Insert(-1);
			}
			RZL_B.Invulnerable(m_Target, false);
			RZL.Kill(m_Target);
			m_fKillAt = now;
			m_fCompleted = -1;
			Next(now);
			return;
		}

		// After the kill: when does each member head for the waypoint (10 m closer)?
		if (m_iStep == 4)
		{
			float t = now - m_fKillAt;
			for (int j = 0; j < m_aM.Count(); j++)
			{
				IEntity mj = m_aM[j];
				if (mj && m_aTook[j] < 0 && m_aToWp0[j] - RZL.DistXZ(mj.GetOrigin(), m_vWp) >= 10)
					m_aTook[j] = t;
			}
			if (m_fCompleted < 0 && !RZL.HasWaypoint(m_G, m_Wp))
				m_fCompleted = t;
			if (t < 40)
				return;

			string took = "";
			foreach (float tk : m_aTook)
				took += RZL.F(tk) + " ";
			RZL.Row("b4", string.Format("trial=%1|fightAfter=%2|orderAt=8 s into the fight|leftTheFightIn30s=%3 of 3|maxDistFromTargetIn30s=%4|tookWaypointAfterKill=%5|waypointCompletedAfterKill=%6", m_iTrial, RZL.F(m_fEngagedAt), m_iLeft, RZL.F(m_fMaxFromTarget), took, RZL.F(m_fCompleted)));
			Next(now);
			return;
		}

		if (m_iStep == 5)
		{
			RZL.Del(m_Wp);
			RZL.DelAll(m_aChars, m_aGroups);
			m_iTrial++;
			Next(now);
			return;
		}

		if (m_iStep == 6 && s >= 2)
		{
			m_iStep = 0;
			m_fStepAt = now;
		}
	}
}

//------------------------------------------------------------------------------------------------
//! B5. Phase 0's memory test with the hooks watching: the seer sees the target, the target vanishes.
//! Control: left alone. Take-back: 10 s after it vanished the seer is steered (factor 0, intake and
//! sharing closed, slot at the last-seen spot). Every 2 s: what Bacon's tree is doing.
class RZL_ExpB5 : RZL_BExp
{
	protected ref array<string> m_aCases = {"control", "take-back", "control", "take-back"};
	protected int m_iCase;
	protected IEntity m_Seer;
	protected IEntity m_Target;
	protected ref RZL_Mover m_Mover;
	protected float m_fPlacedAt;
	protected float m_fSawAt;
	protected float m_fHideAt;
	protected float m_fTakeAt;
	protected vector m_vLastSeen;
	protected vector m_vSpawn;
	protected float m_fLastLog;
	protected float m_fTargetGone;
	protected float m_fSuspectGone;
	protected float m_fRoamBack;
	protected float m_fOnSlot;
	protected float m_fLastTargetSeen;
	protected float m_fLastSuspect;
	protected float m_fEngagedAt;
	protected bool m_bNamed;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpB5()
	{
		m_sName = "b5";
		if (RZL_LabComponent.s_bQuick)
		{
			m_aCases.Clear();
			m_aCases.Insert("control");
			m_aCases.Insert("take-back");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static string Age(float stamp, float now)
	{
		if (stamp <= 0)
			return "never";

		return RZL.F(now - stamp);
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		float s = Since(now);
		if (m_iStep == 0)
		{
			if (m_iCase >= m_aCases.Count())
			{
				Finish();
				return;
			}

			m_vSpawn = m_Site.Along(20);
			m_Seer = RZL.SpawnSolo(RZL.CIV, m_vSpawn, RZL.YawTo(m_vSpawn, m_Site.m_vT), m_aGroups);
			m_aChars.Insert(m_Seer);
			m_Target = RZL_B.Target(m_Site.Side(150, 1500), 0, true);
			m_aChars.Insert(m_Target);
			m_Mover = new RZL_Mover(m_Seer);
			Next(now);
			return;
		}

		if (m_iStep == 1 && s >= 3)
		{
			RZL.Teleport(m_Target, m_Site.m_vT, RZL.YawTo(m_Site.m_vT, m_vSpawn));
			m_fPlacedAt = now;
			m_fSawAt = -1;
			m_fEngagedAt = -1;
			// Named to the seer at once (our share event), so every run has a chase
			RZL_B.Inject(m_Seer, m_Target);
			m_bNamed = true;
			Next(now);
			return;
		}

		string kind = m_aCases[m_iCase];
		if (m_iStep == 2)
		{
			if (!m_Seer || !m_Target)
			{
				m_iStep = 4;
				return;
			}

			if (m_fSawAt < 0 && (RZL.Seen(m_Seer, m_Target) || RZL_B.Engaged(m_Seer, m_Target)))
				m_fSawAt = now;
			if (m_fSawAt < 0 && now - m_fPlacedAt > 30)
			{
				RZL.Row("b5", string.Format("case=%1|run=%2|note=the seer neither saw nor took the target in 30 s", kind, m_iCase));
				m_iStep = 4;
				return;
			}
			// Wait for a real chase: Bacon picked the target (named to it once if it hasn't by 10 s)
			if (m_fSawAt >= 0 && m_fEngagedAt < 0 && RZL_B.Engaged(m_Seer, m_Target))
				m_fEngagedAt = now;
			if (m_fSawAt >= 0 && m_fEngagedAt < 0 && !m_bNamed && now - m_fSawAt > 10)
			{
				RZL_B.Inject(m_Seer, m_Target);
				m_bNamed = true;
			}
			if (m_fSawAt >= 0 && m_fEngagedAt < 0 && now - m_fSawAt > 30)
			{
				RZL.Row("b5", string.Format("case=%1|run=%2|note=the seer never took the target as its target in 30 s", kind, m_iCase));
				m_iStep = 4;
				return;
			}
			if (m_fEngagedAt >= 0 && (RZL.DistXZ(m_Seer.GetOrigin(), m_Target.GetOrigin()) < 8 || now - m_fEngagedAt > 8))
			{
				m_vLastSeen = m_Target.GetOrigin();
				RZL.Teleport(m_Target, m_Site.Side(150, 1500), 0);
				m_fHideAt = now;
				m_fTakeAt = -1;
				m_fLastLog = -10;
				m_fTargetGone = -1;
				m_fSuspectGone = -1;
				m_fRoamBack = -1;
				m_fOnSlot = -1;
				m_fLastTargetSeen = now;
				m_fLastSuspect = now;
				Next(now);
			}
			return;
		}

		if (m_iStep == 3)
		{
			float h = now - m_fHideAt;
			if (!m_Seer || !m_Target)
			{
				Next(now);
				return;
			}

			m_Mover.Sample();
			float hn = RZL_Hooks.Now();
			Bacon_622120A5448725E3_InfectedCharacter inf = RZL_B.Inf(m_Seer);
			if (kind == "take-back" && m_fTakeAt < 0 && h >= 10)
			{
				RZL_B.Steer(m_Seer, m_vLastSeen);
				m_fTakeAt = now;
			}
			if (m_fTakeAt > 0 && now - m_fTakeAt >= 1 && now - m_fTakeAt < 1.3)
				RZL_B.Factor(m_Seer, 0);

			// Bacon's state from the hooks: target (broadcast), suspect (AIGetShooter succeeded or
			// Roam updating the loiter point), roam (our loiter hook)
			if (inf)
			{
				if (hn - inf.m_fRZLBroadcastAt < 1)
					m_fLastTargetSeen = now;
				if (hn - inf.m_fRZLShooterOkAt < 1 || (hn - inf.m_fRZLUpdLoiterAt < 1 && hn - inf.m_fRZLLoiterAt > 3))
					m_fLastSuspect = now;
				if (m_fTargetGone < 0 && now - m_fLastTargetSeen >= 3)
					m_fTargetGone = m_fLastTargetSeen - m_fHideAt;
				if (m_fSuspectGone < 0 && now - m_fLastSuspect >= 3)
					m_fSuspectGone = m_fLastSuspect - m_fHideAt;
				// the hooks stamp world time (RZL_Hooks.Now), the lab's `now` counts from its start
				float since = hn - h;
				if (m_fTakeAt > 0)
					since = hn - (now - m_fTakeAt);
				if (m_fRoamBack < 0 && inf.m_fRZLLoiterAt > since)
					m_fRoamBack = inf.m_fRZLLoiterAt - since;
				if (m_fTakeAt > 0 && m_fOnSlot < 0 && RZL.DistXZ(m_Seer.GetOrigin(), m_vLastSeen) < 5)
					m_fOnSlot = now - m_fTakeAt;
			}

			if (h - m_fLastLog >= 2 && inf)
			{
				m_fLastLog = h;
				RZL.Row("b5t", string.Format("case=%1|run=%2|t=%3|category=%4|sinceSeen=%5|baconTarget=%6|suspectOk=%7|ourLoiterHook=%8|loiterSet=%9", kind, m_iCase, RZL.F(h), RZL.Category(m_Seer, m_Target), RZL.F(RZL.SinceSeen(m_Seer, m_Target)), Age(inf.m_fRZLBroadcastAt, hn), Age(inf.m_fRZLShooterOkAt, hn), Age(inf.m_fRZLLoiterAt, hn), Age(inf.m_fRZLUpdLoiterAt, hn)) + string.Format("|speed=%1|toLastSeen=%2|towardReal=%3|mode=%4", RZL.F(m_Mover.m_fSpeed), RZL.F(RZL.DistXZ(m_Seer.GetOrigin(), m_vLastSeen)), m_Mover.Toward(m_Target.GetOrigin(), 1), inf.m_iRZLMode));
			}

			if (h < 60)
				return;

			RZL.Row("b5", string.Format("case=%1|run=%2|baconTargetLastAt=%3|suspectLastAt=%4|roamBackAfter=%5|onSlotAfterTakeBack=%6|distFromLastSeenAt60s=%7|distFromSpawnAt60s=%8|categoryAt60s=%9", kind, m_iCase, RZL.F(m_fTargetGone), RZL.F(m_fSuspectGone), RZL.F(m_fRoamBack), RZL.F(m_fOnSlot), RZL.F(RZL.DistXZ(m_Seer.GetOrigin(), m_vLastSeen)), RZL.F(RZL.DistXZ(m_Seer.GetOrigin(), m_vSpawn)), RZL.Category(m_Seer, m_Target)) + string.Format("|namedByUs=%1|chaseBeforeHide=%2", m_bNamed, RZL.F(m_fHideAt - m_fEngagedAt)));
			Next(now);
			return;
		}

		if (m_iStep == 4)
		{
			RZL.DelAll(m_aChars, m_aGroups);
			m_iCase++;
			Next(now);
			return;
		}

		if (m_iStep == 5 && s >= 2)
		{
			m_iStep = 0;
			m_fStepAt = now;
		}
	}
}

//------------------------------------------------------------------------------------------------
//! B14. The pieces of the special moves: SetJump(1) and SetMeleeAttack(true) on an AI body,
//! IsMeleeAttack() read on the server during Bacon's own fight (and whether hits follow it), the
//! melee gate (L12), and a sprint-and-jump leap.
class RZL_ExpB14 : RZL_BExp
{
	protected IEntity m_Body;
	protected IEntity m_Target;
	protected float m_fT0;
	protected float m_fMaxH;
	protected bool m_bFalling;
	protected int m_iMeleeTrue;
	protected int m_iSamples;
	protected float m_fLastHealth;
	protected int m_iDrops;
	protected int m_iDropsAfterMelee;
	protected float m_fLastMeleeAt;
	protected vector m_vStart;
	protected int m_iPhase;
	protected int m_iInput;
	protected int m_iCan0;
	protected float m_fHp0;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpB14()
	{
		m_sName = "b14";
	}

	//------------------------------------------------------------------------------------------------
	protected void ResetCounts(float now)
	{
		m_fT0 = now;
		m_fMaxH = 0;
		m_bFalling = false;
		m_iMeleeTrue = 0;
		m_iSamples = 0;
		m_iDrops = 0;
		m_iDropsAfterMelee = 0;
		m_fLastMeleeAt = -10;
		m_iInput = 0;
		Bacon_622120A5448725E3_InfectedCharacter inf0 = RZL_B.Inf(m_Body);
		if (inf0)
			m_iCan0 = inf0.m_iRZLCanMelee;
		m_fHp0 = AbsHealth();
		if (m_Body)
			m_vStart = m_Body.GetOrigin();
		if (m_Target)
			m_fLastHealth = RZL_B.Health(m_Target);
	}

	//------------------------------------------------------------------------------------------------
	//! The target's health in points (not scaled), -1 without one.
	protected float AbsHealth()
	{
		ChimeraCharacter tc = ChimeraCharacter.Cast(m_Target);
		if (!tc || !tc.GetDamageManager())
			return -1;

		return tc.GetDamageManager().GetHealth();
	}

	//------------------------------------------------------------------------------------------------
	protected string Extra()
	{
		int can = -1;
		Bacon_622120A5448725E3_InfectedCharacter inf = RZL_B.Inf(m_Body);
		if (inf)
			can = inf.m_iRZLCanMelee - m_iCan0;
		return string.Format("|canMeleeYes=%1|meleeInputSamples=%2|targetHealth=%3 -> %4", can, m_iInput, RZL.F(m_fHp0), RZL.F(AbsHealth()));
	}

	//------------------------------------------------------------------------------------------------
	protected void Sample(float now)
	{
		CharacterControllerComponent c = RZL_B.Ctrl(m_Body);
		if (!c)
			return;

		m_iSamples++;
		m_fMaxH = Math.Max(m_fMaxH, RZL_B.Height(m_Body));
		if (c.IsFalling())
			m_bFalling = true;
		if (c.GetMeleeAttackInput())
			m_iInput++;
		if (c.IsMeleeAttack())
		{
			m_iMeleeTrue++;
			m_fLastMeleeAt = now;
		}
		if (m_Target)
		{
			float hp = RZL_B.Health(m_Target);
			if (hp >= 0 && hp < m_fLastHealth - 0.001)
			{
				m_iDrops++;
				if (now - m_fLastMeleeAt <= 1.0)
					m_iDropsAfterMelee++;
			}
			if (hp >= 0 && hp < 0.6)
			{
				ChimeraCharacter tc = ChimeraCharacter.Cast(m_Target);
				tc.GetDamageManager().SetHealthScaled(1);
				hp = RZL_B.Health(m_Target);
			}
			m_fLastHealth = hp;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		float s = Since(now);
		if (m_iStep == 0)
		{
			m_Body = RZL.SpawnSolo(RZL.CIV, m_Site.m_vT, m_Site.m_fYaw, m_aGroups);
			m_aChars.Insert(m_Body);
			Next(now);
			return;
		}

		// (a) SetJump(1) on an idle AI body
		if (m_iStep == 1 && s >= 4)
		{
			ResetCounts(now);
			CharacterControllerComponent c1 = RZL_B.Ctrl(m_Body);
			if (c1)
				c1.SetJump(1);
			Next(now);
			return;
		}

		if (m_iStep == 2)
		{
			Sample(now);
			if (s < 2)
				return;

			RZL.Row("b14", string.Format("piece=SetJump(1) on an idle AI body|maxHeightAboveGround=%1|fallingSeen=%2|moved=%3", RZL.F2(m_fMaxH), m_bFalling, RZL.F2(RZL.DistXZ(m_Body.GetOrigin(), m_vStart))));
			ResetCounts(now);
			CharacterControllerComponent c2 = RZL_B.Ctrl(m_Body);
			if (c2)
				c2.SetMeleeAttack(true);
			Next(now);
			return;
		}

		// (b) SetMeleeAttack(true) on an idle AI body (no target)
		if (m_iStep == 3)
		{
			Sample(now);
			CharacterControllerComponent c3 = RZL_B.Ctrl(m_Body);
			if (c3)
				c3.SetMeleeAttack(s < 1);
			if (s < 2)
				return;

			RZL.Row("b14", string.Format("piece=SetMeleeAttack(true) held 1 s on an idle AI body|isMeleeAttackTrueSamples=%1 of %2", m_iMeleeTrue, m_iSamples));
			// A target that can be hurt, 10 m in front, named to the body
			m_Target = RZL.Spawn(RZL.TARGET, m_Site.Along(10), m_Site.m_fYaw + 180);
			m_aChars.Insert(m_Target);
			RZL_B.Inject(m_Body, m_Target);
			Next(now);
			return;
		}

		// (c) Bacon's own fight: IsMeleeAttack on the server, and hits that follow it
		if (m_iStep == 4)
		{
			if (Math.Mod(s, 2) < 0.25)
				RZL_B.Inject(m_Body, m_Target);
			if (s < 25 && RZL.DistXZ(m_Body.GetOrigin(), m_Target.GetOrigin()) > 1.8)
				return;

			ResetCounts(now);
			m_iPhase = 0;
			Next(now);
			return;
		}

		if (m_iStep == 5)
		{
			Sample(now);
			Bacon_622120A5448725E3_InfectedCharacter inf = RZL_B.Inf(m_Body);
			if (m_iPhase == 0 && s >= 15)
			{
				RZL.Row("b14", string.Format("piece=IsMeleeAttack() during Bacon's fight (15 s)|isMeleeAttackTrueSamples=%1 of %2|healthDrops=%3|dropsWithin1sOfMelee=%4|distToTarget=%5", m_iMeleeTrue, m_iSamples, m_iDrops, m_iDropsAfterMelee, RZL.F(RZL.DistXZ(m_Body.GetOrigin(), m_Target.GetOrigin()))) + Extra());
				ResetCounts(now);
				if (inf)
					inf.m_bRZLMeleeHold = true;
				m_iPhase = 1;
				return;
			}
			if (m_iPhase == 1 && now - m_fT0 >= 10)
			{
				RZL.Row("b14", string.Format("piece=melee gate on (L12), 10 s|isMeleeAttackTrueSamples=%1 of %2|healthDrops=%3|meleeHeldCalls=%4", m_iMeleeTrue, m_iSamples, m_iDrops, RZL_Hooks.s_iMeleeHeld) + Extra());
				ResetCounts(now);
				if (inf)
					inf.m_bRZLMeleeHold = false;
				m_iPhase = 2;
				return;
			}
			if (m_iPhase == 2 && now - m_fT0 >= 10)
			{
				RZL.Row("b14", string.Format("piece=melee gate off again, 10 s|isMeleeAttackTrueSamples=%1 of %2|healthDrops=%3", m_iMeleeTrue, m_iSamples, m_iDrops) + Extra());
				Next(now);
			}
			return;
		}

		// (d) The leap: the target 6 m ahead, SPRINT wanted and SetJump(1) together
		if (m_iStep == 6)
		{
			RZL_B.Invulnerable(m_Target, true);
			vector ahead = m_Body.GetOrigin() + vector.FromYaw(m_Site.m_fYaw) * 6;
			RZL.Teleport(m_Target, ahead, m_Site.m_fYaw + 180);
			Next(now);
			return;
		}

		if (m_iStep == 7 && s >= 1)
		{
			ResetCounts(now);
			RZL_B.WantMove(m_Body, EMovementType.SPRINT);
			CharacterControllerComponent c7 = RZL_B.Ctrl(m_Body);
			if (c7)
				c7.SetJump(1);
			Next(now);
			return;
		}

		if (m_iStep == 8)
		{
			Sample(now);
			if (s < 1.5)
				return;

			RZL.Row("b14", string.Format("piece=leap (SPRINT + SetJump(1), target 6 m ahead)|maxHeightAboveGround=%1|fallingSeen=%2|movedIn1.5s=%3|toTargetAfter=%4|moveWanted=%5", RZL.F2(m_fMaxH), m_bFalling, RZL.F2(RZL.DistXZ(m_Body.GetOrigin(), m_vStart)), RZL.F2(RZL.DistXZ(m_Body.GetOrigin(), m_Target.GetOrigin())), RZL_B.MoveWanted(m_Body)));
			Finish();
		}
	}
}

//------------------------------------------------------------------------------------------------
//! B18. A scripted shot heard by blind bodies at 100/170/230 m: steered ones (intake closed; our
//! hearing table gives those within 170 m a lead) against engaged ones (Bacon's own hearing).
class RZL_ExpB18 : RZL_BExp
{
	protected int m_iRound;
	protected int m_iRounds = 2;
	protected IEntity m_Shooter;
	protected vector m_vShooter;
	protected ref array<IEntity> m_aL = {};
	protected ref array<float> m_aDist = {};
	protected ref array<bool> m_aSteer = {};
	protected ref array<float> m_aCame = {};
	protected int m_iShots;
	protected float m_fFirst;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpB18()
	{
		m_sName = "b18";
		if (RZL_LabComponent.s_bQuick)
			m_iRounds = 1;
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		float s = Since(now);
		if (m_iStep == 0)
		{
			if (m_iRound >= m_iRounds)
			{
				Finish();
				return;
			}

			m_vShooter = m_Site.Along(150);
			m_Shooter = RZL.Spawn(RZL.SHOOTER, m_Site.Side(150, 1500), m_Site.m_fYaw);
			m_aChars.Insert(m_Shooter);
			m_aL.Clear();
			m_aDist.Clear();
			m_aSteer.Clear();
			m_aCame.Clear();
			array<float> rs = {100, 170, 230};
			for (int i = 0; i < 6; i++)
			{
				float d = rs[i % 3];
				bool steer = i < 3;
				float yaw = m_Site.m_fYaw + i * 60 + m_iRound * 30;
				vector p = RZL.Ground(m_vShooter + vector.FromYaw(yaw) * d);
				if (p[1] < 2)
					continue;

				IEntity l = RZL.SpawnSolo(RZL.CIV, p, yaw + 180, m_aGroups);
				if (steer)
					RZL_B.Steer(l, p);
				else
					RZL_B.Factor(l, 0);
				m_aL.Insert(l);
				m_aChars.Insert(l);
				m_aDist.Insert(d);
				m_aSteer.Insert(steer);
				m_aCame.Insert(-1);
			}
			m_iShots = 0;
			m_fFirst = -1;
			Next(now);
			return;
		}

		if (m_iStep == 1)
		{
			if (s >= 1 && s < 1.3)
			{
				foreach (IEntity l1 : m_aL)
					RZL_B.Factor(l1, 0);
			}
			if (s >= 2.5 && s < 2.8 && m_Shooter)
				RZL.Teleport(m_Shooter, m_vShooter, m_Site.m_fYaw);
			if (s >= 4 && m_iShots < 3 && s >= 4 + m_iShots && m_Shooter)
			{
				AIDangerEventWeaponFire ev = new AIDangerEventWeaponFire();
				ev.SetDangerType(EAIDangerEventType.Danger_WeaponFire);
				ev.SetObject(m_Shooter);
				ev.SetVictim(m_Shooter);
				ev.SetPosition(m_Shooter.GetOrigin());
				GetGame().GetAIWorld().RequestBroadcastDangerEvent(ev);
				m_iShots++;
				if (m_fFirst < 0)
				{
					m_fFirst = now;
					// Our hearing table: a steered body within 170 m gets a lead (15% error)
					for (int k = 0; k < m_aL.Count(); k++)
					{
						if (!m_aSteer[k] || m_aDist[k] > 170 || !m_aL[k])
							continue;

						float err = m_aDist[k] * 0.15;
						vector lead = m_vShooter + Vector(Math.RandomFloat(-err, err), 0, Math.RandomFloat(-err, err));
						RZL_B.Slot(m_aL[k], lead);
					}
				}
			}
			if (m_fFirst < 0)
				return;

			float since = now - m_fFirst;
			for (int j = 0; j < m_aL.Count(); j++)
			{
				if (m_aL[j] && m_aCame[j] < 0 && m_aDist[j] - RZL.DistXZ(m_aL[j].GetOrigin(), m_vShooter) >= 30)
					m_aCame[j] = since;
			}
			if (since < 40)
				return;

			for (int r = 0; r < m_aL.Count(); r++)
			{
				string kind = "engaged (Bacon's hearing, intake open)";
				if (m_aSteer[r])
					kind = "steered (intake closed, our table)";
				int blocked = -1;
				Bacon_622120A5448725E3_InfectedCharacter inf = RZL_B.Inf(m_aL[r]);
				if (inf)
					blocked = inf.m_iRZLEventsBlocked;
				float closed = -1;
				if (m_aL[r])
					closed = m_aDist[r] - RZL.DistXZ(m_aL[r].GetOrigin(), m_vShooter);
				RZL.Row("b18", string.Format("round=%1|body=%2|dist=%3|cameAfter=%4|closedIn40s=%5|eventsBlockedByGate=%6", m_iRound, kind, m_aDist[r], RZL.F(m_aCame[r]), RZL.F(closed), blocked));
			}
			Next(now);
			return;
		}

		if (m_iStep == 2)
		{
			RZL.DelAll(m_aChars, m_aGroups);
			m_iRound++;
			Next(now);
			return;
		}

		if (m_iStep == 3 && s >= 3)
		{
			m_iStep = 0;
			m_fStepAt = now;
		}
	}
}
