// Test-only (not part of the mod): what Bacon infected notice. Perception distances (P), target
// sharing, memory after losing sight, and gunfire.

//------------------------------------------------------------------------------------------------
class RZL_Trial
{
	int m_iStance; // 0 standing, 1 crouched, 2 prone, 3 running (standing), 4 standing behind a hill
	float m_fFactor;
	bool m_bWood;
	float m_fDist; // the infected's distance; 0 = the site's whole line

	//------------------------------------------------------------------------------------------------
	void RZL_Trial(int stance, float factor, bool wood, float dist)
	{
		m_iStance = stance;
		m_fFactor = factor;
		m_bWood = wood;
		m_fDist = dist;
	}

	//------------------------------------------------------------------------------------------------
	string StanceName()
	{
		if (m_iStance == 1)
			return "crouched";
		if (m_iStance == 2)
			return "prone";
		if (m_iStance == 3)
			return "running";
		if (m_iStance == 4)
			return "hidden (no line of sight)";
		return "standing";
	}
}

//------------------------------------------------------------------------------------------------
//! The perception trials, in light blocks (day, night, fog). Runners take trials of their terrain.
class RZL_PercPlan
{
	ref array<string> m_aBlocks = {"day", "night", "fog"};
	int m_iBlock = -1;
	ref array<ref RZL_Trial> m_aQueue = {};
	int m_iOut;
	bool m_bSwitching = true;
	bool m_bHasOpen;
	bool m_bHasWood;

	//------------------------------------------------------------------------------------------------
	string Block()
	{
		if (m_iBlock < 0 || m_iBlock >= m_aBlocks.Count())
			return "none";

		return m_aBlocks[m_iBlock];
	}

	//------------------------------------------------------------------------------------------------
	bool AllDone()
	{
		return m_iBlock >= m_aBlocks.Count();
	}

	//------------------------------------------------------------------------------------------------
	protected void Add(int stance, float factor, bool wood, float dist)
	{
		if (wood && !m_bHasWood)
			return;

		if (!wood && !m_bHasOpen)
			return;

		m_aQueue.Insert(new RZL_Trial(stance, factor, wood, dist));
	}

	//------------------------------------------------------------------------------------------------
	//! One trial at each rung of the ladder (25-200 m)
	protected void Ladder(int stance, float factor, bool wood)
	{
		array<float> rungs = {25, 50, 75, 100, 150, 200};
		if (RZL_LabComponent.s_bQuick)
		{
			rungs.Clear();
			rungs.Insert(50);
			rungs.Insert(150);
		}
		foreach (float d : rungs)
			Add(stance, factor, wood, d);
	}

	//------------------------------------------------------------------------------------------------
	void Fill()
	{
		m_aQueue.Clear();
		string b = Block();
		if (RZL_LabComponent.s_bQuick)
		{
			Ladder(0, 1, false);
			Ladder(0, 1, true);
			if (b == "day")
			{
				Ladder(3, 1, false);
				Add(4, 1, false, 0);
			}
			return;
		}

		if (b == "fog")
		{
			Ladder(0, 1, false);
			Ladder(3, 1, false);
			Ladder(0, 1, true);
			return;
		}

		// Stances in the open and at a wood edge, then the perception factor sweep in the open
		Ladder(0, 1, false);
		Ladder(0, 1, true);
		Ladder(2, 1, false);
		Ladder(2, 1, true);
		Ladder(3, 1, false);
		Ladder(3, 1, true);
		Ladder(1, 1, false);
		// the whole line (300-450 m): how far a standing man is seen
		Add(0, 1, false, 0);
		Add(0, 1, false, 0);
		array<float> factors = {0.25, 0.5, 2};
		foreach (float f : factors)
			Ladder(0, f, false);
		if (b == "day")
		{
			foreach (float f2 : factors)
				Ladder(2, f2, false);
		}
		// The control: a target behind a hill
		Add(4, 1, false, 0);
		Add(4, 1, false, 0);
	}

	//------------------------------------------------------------------------------------------------
	RZL_Trial Take(bool wood)
	{
		if (m_bSwitching || m_iBlock < 0 || AllDone())
			return null;

		for (int i = 0; i < m_aQueue.Count(); i++)
		{
			if (m_aQueue[i].m_bWood == wood)
			{
				RZL_Trial t = m_aQueue[i];
				m_aQueue.Remove(i);
				m_iOut++;
				return t;
			}
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	void Finished()
	{
		m_iOut--;
	}

	//------------------------------------------------------------------------------------------------
	bool BlockFinished()
	{
		return !m_bSwitching && m_iBlock >= 0 && !AllDone() && m_aQueue.IsEmpty() && m_iOut <= 0;
	}
}

//------------------------------------------------------------------------------------------------
//! 3. Perception: one infected is spawned first, at a set distance from the site's point (a rung
//! of the ladder); the target is spawned 1.5 km away, takes its stance there, and is then put at the
//! site. Then 30 s: when the infected first sees it (in its perception and seen within the last
//! second), counts it an enemy, and charges it. The infected loiters as it likes meanwhile.
//! Stance 4 is the control: a standing target behind a hill (no terrain line of sight).
class RZL_ExpPerc : RZL_Exp
{
	static const float HOLD = 30;

	RZL_PercPlan m_Plan;
	protected ref RZL_Trial m_Trial;
	protected IEntity m_Target;
	protected IEntity m_Obs;
	protected ref RZL_Mover m_ObsMover;
	protected ref RZL_Mover m_TgtMover;
	protected vector m_vObsAt;
	protected float m_fDist;
	protected float m_fSeen;
	protected float m_fEnemy;
	protected float m_fCharge;
	protected float m_fChargeDist;
	protected float m_fShownAt;
	protected string m_sFactorPath;
	protected string m_sRunMode;
	protected float m_fRunSpeedSum;
	protected int m_iRunSpeedN;
	protected float m_fFlipAt;
	protected int m_iRunDir = 1;
	protected int m_iLod;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpPerc()
	{
		m_sName = "perception";
	}

	//------------------------------------------------------------------------------------------------
	//! A spot far from everything for the target to take its stance.
	protected vector Park()
	{
		array<float> sides = {90, -90, 180};
		foreach (float side : sides)
		{
			vector p = RZL.At(m_Site.m_vT, m_Site.m_fYaw + side, 1500);
			if (p[1] > 2)
				return p;
		}
		return RZL.At(m_Site.m_vT, m_Site.m_fYaw + 90, 1500);
	}

	//------------------------------------------------------------------------------------------------
	//! A point 100-250 m from T with a hill in between (terrain 2 m or more above the sight line).
	protected bool FindHidden(out vector spot)
	{
		vector t = m_Site.m_vT;
		for (float yaw = 0; yaw < 360; yaw += 15)
		{
			for (float d = 100; d <= 250; d += 50)
			{
				vector o = RZL.At(t, yaw, d);
				if (o[1] > 2 && RZL.LosBlock(o, 1.7, t, 1.6) >= 2)
				{
					spot = o;
					return true;
				}
			}
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void DriveRunner(float now)
	{
		if (!m_Target)
			return;

		ChimeraCharacter c = ChimeraCharacter.Cast(m_Target);
		CharacterControllerComponent ctrl = c.GetCharacterController();
		if (now - m_fFlipAt > 4)
		{
			m_iRunDir = -m_iRunDir;
			m_fFlipAt = now;
		}
		float yaw = m_Site.m_fYaw + 90 * m_iRunDir;
		if (RZL.DistXZ(m_Target.GetOrigin(), m_Site.m_vT) > 12)
		{
			RZL.Teleport(m_Target, m_Site.m_vT, yaw);
			m_TgtMover.Reset();
		}

		if (m_sRunMode == "scripted")
		{
			// fallback: moved by script at 4.5 m/s (no animation, no velocity)
			vector p = m_Target.GetOrigin() + vector.FromYaw(yaw) * (4.5 * 0.25);
			m_Target.SetOrigin(RZL.Ground(p));
			m_Target.Update();
			return;
		}

		if (ctrl)
		{
			ctrl.SetHeadingAngle(yaw * Math.DEG2RAD, false);
			ctrl.SetMovement(2, vector.Forward);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		float s = Since(now);
		if (m_iStep == 0)
		{
			if (m_Plan.AllDone())
			{
				Finish();
				return;
			}

			m_Trial = m_Plan.Take(m_Site.m_bWood);
			if (!m_Trial)
				return;

			m_fSeen = -1;
			m_fEnemy = -1;
			m_fCharge = -1;
			m_fChargeDist = -1;
			m_sRunMode = "-";
			m_fRunSpeedSum = 0;
			m_iRunSpeedN = 0;
			m_fDist = m_Trial.m_fDist;
			if (m_fDist <= 0 || m_fDist > m_Site.m_fLine)
				m_fDist = m_Site.m_fLine;
			m_vObsAt = m_Site.Along(m_fDist);
			if (m_Trial.m_iStance == 4)
			{
				vector hidden;
				if (!FindHidden(hidden))
				{
					RZL.Row("perc", string.Format("site=%1|light=%2|stance=hidden (no line of sight)|note=no hidden spot near this site", m_Site.m_sName, m_Plan.Block()));
					m_Plan.Finished();
					return;
				}
				m_vObsAt = hidden;
				m_fDist = RZL.DistXZ(hidden, m_Site.m_vT);
			}

			// The infected first, then the target from far away (an infected spawned after its target
			// starts with it already in its perception)
			m_Obs = RZL.SpawnSolo(RZL.CIV, m_vObsAt, RZL.YawTo(m_vObsAt, m_Site.m_vT), m_aGroups);
			m_aChars.Insert(m_Obs);
			m_sFactorPath = RZL.SetFactor(m_Obs, m_Trial.m_fFactor);
			m_Target = RZL.Spawn(RZL.TARGET, Park(), 0);
			m_aChars.Insert(m_Target);
			m_TgtMover = new RZL_Mover(m_Target);
			Next(now);
			return;
		}

		if (m_iStep == 1 && s >= 1)
		{
			m_sFactorPath = RZL.SetFactor(m_Obs, m_Trial.m_fFactor);
			ChimeraCharacter tc = ChimeraCharacter.Cast(m_Target);
			if (tc && m_Trial.m_iStance == 1)
				tc.GetCharacterController().SetStanceChange(ECharacterStanceChange.STANCECHANGE_TOCROUCH);
			if (tc && m_Trial.m_iStance == 2)
				tc.GetCharacterController().SetStanceChange(ECharacterStanceChange.STANCECHANGE_TOPRONE);
			Next(now);
			return;
		}

		if (m_iStep == 2 && s >= 4)
		{
			if (!m_Obs || !m_Target)
			{
				RZL.Log("perception: observer or target vanished before the start");
				m_iStep = 4;
				m_fStepAt = now;
				return;
			}

			RZL.Teleport(m_Target, m_Site.m_vT, RZL.YawTo(m_Site.m_vT, m_vObsAt));
			m_TgtMover.Reset();
			if (m_Trial.m_iStance == 3)
			{
				m_sRunMode = "controller";
				m_fFlipAt = now;
			}
			m_ObsMover = new RZL_Mover(m_Obs);
			m_iLod = RZL.LOD(m_Obs);
			m_fShownAt = now;
			Next(now);
			return;
		}

		if (m_iStep == 3)
		{
			if (!m_Obs || !m_Target)
			{
				RZL.Log("perception: observer or target vanished");
				Next(now);
				return;
			}

			float t = now - m_fShownAt;
			if (m_Trial.m_iStance == 3)
			{
				DriveRunner(now);
				m_TgtMover.Sample();
				if (t > 1.5)
				{
					m_fRunSpeedSum += m_TgtMover.m_fSpeed;
					m_iRunSpeedN++;
				}
				if (t > 2.5 && t < 2.8 && m_sRunMode == "controller" && m_iRunSpeedN > 0 && m_fRunSpeedSum / m_iRunSpeedN < 1)
				{
					m_sRunMode = "scripted";
					RZL.Log("perception: SetMovement didn't move the target; running by script instead");
				}
			}

			// the first half second after the teleport doesn't count (the target may still be arriving)
			if (t < 0.5)
				return;

			m_ObsMover.Sample();
			if (m_fSeen < 0 && RZL.Seen(m_Obs, m_Target))
				m_fSeen = t;
			if (m_fEnemy < 0 && RZL.Category(m_Obs, m_Target) == ETargetCategory.ENEMY)
				m_fEnemy = t;
			if (m_fCharge < 0 && m_fSeen >= 0 && m_ObsMover.Toward(m_Target.GetOrigin(), 2.5))
			{
				m_fCharge = t;
				m_fChargeDist = RZL.DistXZ(m_Obs.GetOrigin(), m_Target.GetOrigin());
			}

			bool over = t >= HOLD || m_fCharge >= 0 || (m_fEnemy >= 0 && t - m_fEnemy > 3);
			if (!over)
				return;

			string result = "nothing";
			if (RZL.Category(m_Obs, m_Target) >= 0)
				result = "in perception";
			if (m_fSeen >= 0)
				result = "seen";
			if (m_fEnemy >= 0)
				result = "enemy";
			if (m_fCharge >= 0)
				result = "charged";
			float runSpeed = -1;
			if (m_iRunSpeedN > 0)
				runSpeed = m_fRunSpeedSum / m_iRunSpeedN;
			string terrain = "open";
			if (m_Site.m_bWood)
				terrain = "wood edge";
			float fog = -1;
			TimeAndWeatherManagerEntity tm = RZL.Weather();
			if (tm)
				fog = tm.GetFogAmount();
			RZL.Row("perc", string.Format("site=%1|terrain=%2|light=%3|stance=%4|factor=%5|dist=%6|seenAfter=%7|enemyAfter=%8|chargedAfter=%9", m_Site.m_sName, terrain, m_Plan.Block(), m_Trial.StanceName(), RZL.F2(m_Trial.m_fFactor), RZL.F(m_fDist), RZL.F(m_fSeen), RZL.F(m_fEnemy), RZL.F(m_fCharge)) + string.Format("|chargeFrom=%1|result=%2|runMode=%3|runSpeed=%4|lod=%5|factorPath=%6|tod=%7|fog=%8", RZL.F(m_fChargeDist), result, m_sRunMode, RZL.F(runSpeed), m_iLod, m_sFactorPath, RZL.TimeOfDay(), RZL.F2(fog)));
			Next(now);
			return;
		}

		if (m_iStep == 4)
		{
			RZL.DelAll(m_aChars, m_aGroups);
			m_Plan.Finished();
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
//! 3b. Target sharing and memory. Sharing: a "seer" stands just before a crest with the target in
//! sight; sighted listeners stand beyond the crest, where terrain hides the target from them. If they
//! come over the crest (close 25 m or more on the target), Bacon's shared target reached them.
//! Memory: a seer sees the target 40 m away in the open; when it is close, the target vanishes
//! (moved 1.5 km away); how long it keeps going, where it stops, when its perception forgets.
class RZL_ExpShare : RZL_Exp
{
	protected ref array<string> m_aCases = {"crest", "crest", "crest", "memory", "memory", "memory"};
	protected int m_iCase;
	protected IEntity m_Seer;
	protected IEntity m_Target;
	protected ref RZL_Mover m_SeerMover;
	protected ref array<IEntity> m_aListeners = {};
	protected ref array<float> m_aDist = {};
	protected ref array<float> m_aStart = {};
	protected ref array<float> m_aCharged = {};
	protected float m_fT0;
	protected vector m_vTargetAt;
	protected vector m_vLastSeen;
	protected vector m_vSeerSpawn;
	protected float m_fHidden;
	protected float m_fStopped;
	protected float m_fForgot;
	protected float m_fSlowSince;
	protected float m_fD30;
	protected float m_fD60;
	protected float m_fDStop;
	protected bool m_bPlaced;
	protected float m_fPlacedAt;
	protected float m_fToHidden0;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpShare()
	{
		m_sName = "share";
		if (RZL_LabComponent.s_bQuick)
		{
			m_aCases.Clear();
			m_aCases.Insert("crest");
			m_aCases.Insert("memory");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Along some direction from T: the last spot (20-120 m out) that sees T, then spots beyond the
	//! crest that don't. Returns false if this site has no such crest.
	protected bool FindCrest(vector t, out vector seer, notnull array<vector> beyond, notnull array<float> fromSeer)
	{
		for (float yaw = 0; yaw < 360; yaw += 15)
		{
			float lastSeen = -1;
			float hiddenAt = -1;
			for (float d = 20; d <= 160; d += 5)
			{
				vector p = RZL.At(t, yaw, d);
				if (p[1] < 2)
					break;

				if (RZL.LosBlock(p, 1.7, t, 1.6) < 0)
				{
					lastSeen = d;
					continue;
				}

				if (lastSeen > 0 && RZL.LosBlock(p, 1.7, t, 1.6) > 1)
				{
					hiddenAt = d;
					break;
				}
			}
			if (lastSeen < 20 || hiddenAt < 0)
				continue;

			seer = RZL.At(t, yaw, lastSeen);
			beyond.Clear();
			fromSeer.Clear();
			array<float> steps = {5, 15, 30, 45, 60, 80, 110, 150};
			foreach (float k : steps)
			{
				vector q = RZL.At(t, yaw, hiddenAt + k);
				if (q[1] < 2 || RZL.LosBlock(q, 1.7, t, 1.6) < 0.5)
					continue;

				beyond.Insert(q);
				fromSeer.Insert(RZL.DistXZ(q, seer));
			}
			if (beyond.Count() >= 3)
				return true;
		}
		return false;
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

			string kind = m_aCases[m_iCase];
			m_aListeners.Clear();
			m_aDist.Clear();
			m_aStart.Clear();
			m_aCharged.Clear();
			m_vTargetAt = m_Site.Along(290);
			m_vSeerSpawn = m_Site.Along(250);
			if (kind == "crest")
			{
				array<vector> beyond = {};
				array<float> fromSeer = {};
				if (!FindCrest(m_Site.m_vT, m_vSeerSpawn, beyond, fromSeer))
				{
					RZL.Row("share", string.Format("case=crest|run=%1|note=no crest near site %2", m_iCase, m_Site.m_sName));
					m_iCase++;
					return;
				}
				m_vTargetAt = m_Site.m_vT;
				for (int i = 0; i < beyond.Count(); i++)
				{
					IEntity l = RZL.SpawnSolo(RZL.CIV, beyond[i], RZL.YawTo(beyond[i], m_vTargetAt), m_aGroups);
					m_aListeners.Insert(l);
					m_aChars.Insert(l);
					m_aDist.Insert(fromSeer[i]);
					m_aStart.Insert(RZL.DistXZ(beyond[i], m_vTargetAt));
					m_aCharged.Insert(-1);
				}
			}

			m_Seer = RZL.SpawnSolo(RZL.CIV, m_vSeerSpawn, RZL.YawTo(m_vSeerSpawn, m_vTargetAt), m_aGroups);
			m_aChars.Insert(m_Seer);
			m_SeerMover = new RZL_Mover(m_Seer);
			// The target waits 1.5 km away until the infected are in place (an infected spawned
			// after its target starts with it in its perception)
			m_Target = RZL.Spawn(RZL.TARGET, m_Site.Side(150, 1500), 0);
			m_aChars.Insert(m_Target);
			m_bPlaced = false;
			m_fT0 = -1;
			m_fHidden = -1;
			m_fStopped = -1;
			m_fForgot = -1;
			m_fSlowSince = -1;
			m_fD30 = -1;
			m_fD60 = -1;
			m_fDStop = -1;
			Next(now);
			return;
		}

		string kind2 = m_aCases[m_iCase];
		if (m_iStep == 1)
		{
			if (!m_Seer || !m_Target)
			{
				Next(now);
				return;
			}

			if (!m_bPlaced)
			{
				if (s >= 3)
				{
					RZL.Teleport(m_Target, m_vTargetAt, RZL.YawTo(m_vTargetAt, m_vSeerSpawn));
					m_bPlaced = true;
					m_fPlacedAt = now;
				}
				return;
			}

			m_SeerMover.Sample();
			if (m_fT0 < 0)
			{
				if (RZL.Seen(m_Seer, m_Target))
				{
					m_fT0 = now;
					RZL.Log(string.Format("share %1: seer saw the target at %2 m, %3 s after it appeared", kind2, RZL.F(RZL.DistXZ(m_Seer.GetOrigin(), m_Target.GetOrigin())), RZL.F(now - m_fPlacedAt)));
				}
				else if (now - m_fPlacedAt > 40)
				{
					RZL.Row("share", string.Format("case=%1|run=%2|note=the seer never saw the target in 40 s (%3 m)", kind2, m_iCase, RZL.F(RZL.DistXZ(m_Seer.GetOrigin(), m_Target.GetOrigin()))));
					Next(now);
				}
				return;
			}

			float since = now - m_fT0;
			if (kind2 != "memory")
			{
				for (int j = 0; j < m_aListeners.Count(); j++)
				{
					IEntity lj = m_aListeners[j];
					if (lj && m_aCharged[j] < 0 && m_aStart[j] - RZL.DistXZ(lj.GetOrigin(), m_vTargetAt) >= 25)
						m_aCharged[j] = since;
				}

				if (since >= 40)
				{
					for (int k = 0; k < m_aListeners.Count(); k++)
					{
						float closed = -1;
						if (m_aListeners[k])
							closed = m_aStart[k] - RZL.DistXZ(m_aListeners[k].GetOrigin(), m_vTargetAt);
						RZL.Row("share", string.Format("case=%1|run=%2|seerToTarget=%3|listenerFromSeer=%4|listenerToTarget=%5|cameAfter=%6|closedIn40s=%7|listenerHadTarget=%8", kind2, m_iCase, RZL.F(RZL.DistXZ(m_vSeerSpawn, m_vTargetAt)), RZL.F(m_aDist[k]), RZL.F(m_aStart[k]), RZL.F(m_aCharged[k]), RZL.F(closed), RZL.Category(m_aListeners[k], m_Target)));
					}
					Next(now);
				}
				return;
			}

			// Memory: hide the target once the seer is charging and close
			if (m_fHidden < 0)
			{
				if (RZL.DistXZ(m_Seer.GetOrigin(), m_Target.GetOrigin()) < 18 || since > 12)
				{
					m_vLastSeen = m_Target.GetOrigin();
					RZL.Teleport(m_Target, m_Site.Side(150, 1500), 0);
					m_fHidden = now;
					m_fToHidden0 = RZL.DistXZ(m_Seer.GetOrigin(), m_Site.Side(150, 1500));
				}
				return;
			}

			float h = now - m_fHidden;
			float sp = m_SeerMover.m_fSpeed;
			if (m_fStopped < 0 && h > 0.5)
			{
				if (sp < 0.8)
				{
					if (m_fSlowSince < 0)
						m_fSlowSince = h;
					if (h - m_fSlowSince >= 2)
					{
						m_fStopped = m_fSlowSince;
						m_fDStop = RZL.DistXZ(m_Seer.GetOrigin(), m_vLastSeen);
					}
				}
				else
				{
					m_fSlowSince = -1;
				}
			}
			if (m_fForgot < 0 && RZL.Category(m_Seer, m_Target) < 0)
				m_fForgot = h;
			if (m_fD30 < 0 && h >= 30)
				m_fD30 = RZL.DistXZ(m_Seer.GetOrigin(), m_vLastSeen);
			if (m_fD60 < 0 && h >= 60)
				m_fD60 = RZL.DistXZ(m_Seer.GetOrigin(), m_vLastSeen);
			if (h >= 120 || (RZL_LabComponent.s_bQuick && h >= 40))
			{
				RZL.Row("memory", string.Format("run=%1|stoppedRunningAfter=%2|distToLastSeenWhenStopped=%3|distToLastSeen30s=%4|60s=%5|end=%6|perceptionForgotAfter=%7|sinceSeenAtEnd=%8|distFromSpawnAtEnd=%9", m_iCase, RZL.F(m_fStopped), RZL.F(m_fDStop), RZL.F(m_fD30), RZL.F(m_fD60), RZL.F(RZL.DistXZ(m_Seer.GetOrigin(), m_vLastSeen)), RZL.F(m_fForgot), RZL.F(RZL.SinceSeen(m_Seer, m_Target)), RZL.F(RZL.DistXZ(m_Seer.GetOrigin(), m_vSeerSpawn))) + string.Format("|toHiddenTargetAtHide=%1|toHiddenTargetAtEnd=%2|window=%3", RZL.F(m_fToHidden0), RZL.F(RZL.DistXZ(m_Seer.GetOrigin(), m_Target.GetOrigin())), RZL.F(h)));
				Next(now);
			}
			return;
		}

		if (m_iStep == 2)
		{
			RZL.DelAll(m_aChars, m_aGroups);
			m_iCase++;
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

//------------------------------------------------------------------------------------------------
//! 3c. Gunfire: blind listeners round a shooter at 40-550 m. First a weapon-fire danger event raised
//! from script, then (fresh listeners) a real shot if the shooter can be made to fire.
class RZL_ExpNoise : RZL_Exp
{
	protected ref array<string> m_aCases = {"scripted event", "real shot"};
	protected int m_iCase;
	protected IEntity m_Shooter;
	protected vector m_vShooter;
	protected ref array<IEntity> m_aListeners = {};
	protected ref array<ref RZL_Mover> m_aMovers = {};
	protected ref array<float> m_aDist = {};
	protected ref array<float> m_aCharged = {};
	protected int m_iShots;
	protected float m_fFirst;
	protected int m_iAmmoBefore = -1;
	protected string m_sFired;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpNoise()
	{
		m_sName = "noise";
	}

	//------------------------------------------------------------------------------------------------
	protected int Ammo()
	{
		ChimeraCharacter c = ChimeraCharacter.Cast(m_Shooter);
		if (!c || !c.GetWeaponManager())
			return -1;

		BaseWeaponComponent w = c.GetWeaponManager().GetCurrentWeapon();
		if (!w || !w.GetCurrentMagazine())
			return -1;

		return w.GetCurrentMagazine().GetAmmoCount();
	}

	//------------------------------------------------------------------------------------------------
	protected bool RaiseShot()
	{
		AIDangerEventWeaponFire ev = new AIDangerEventWeaponFire();
		if (!ev)
			return false;

		ev.SetDangerType(EAIDangerEventType.Danger_WeaponFire);
		ev.SetObject(m_Shooter);
		ev.SetVictim(m_Shooter);
		ev.SetPosition(m_Shooter.GetOrigin());
		GetGame().GetAIWorld().RequestBroadcastDangerEvent(ev);
		return true;
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

			m_vShooter = m_Site.Along(150);
			// The shooter waits 1.5 km away until the listeners are in place
			m_Shooter = RZL.Spawn(RZL.SHOOTER, m_Site.Side(150, 1500), m_Site.m_fYaw);
			m_aChars.Insert(m_Shooter);
			m_aListeners.Clear();
			m_aMovers.Clear();
			m_aDist.Clear();
			m_aCharged.Clear();
			array<float> rs = {40, 80, 120, 170, 230, 300, 400, 550};
			for (int i = 0; i < rs.Count(); i++)
			{
				vector p = RZL.Ground(m_vShooter + vector.FromYaw(m_Site.m_fYaw + i * 45) * rs[i]);
				if (p[1] < 2)
				{
					RZL.Log(string.Format("noise: listener at %1 m would be in water, skipped", rs[i]));
					continue;
				}

				IEntity l = RZL.SpawnSolo(RZL.CIV, p, m_Site.m_fYaw + i * 45, m_aGroups);
				RZL.SetFactor(l, 0);
				m_aListeners.Insert(l);
				m_aChars.Insert(l);
				m_aMovers.Insert(new RZL_Mover(l));
				m_aDist.Insert(rs[i]);
				m_aCharged.Insert(-1);
			}
			m_iShots = 0;
			m_fFirst = -1;
			m_sFired = "-";
			Next(now);
			return;
		}

		string kind = m_aCases[m_iCase];
		if (m_iStep == 1)
		{
			if (s >= 1 && s < 1.3)
			{
				foreach (IEntity l1 : m_aListeners)
					RZL.SetFactor(l1, 0);
			}

			foreach (RZL_Mover mv : m_aMovers)
				mv.Sample();

			if (s >= 2.5 && s < 2.8 && m_Shooter)
				RZL.Teleport(m_Shooter, m_vShooter, m_Site.m_fYaw);

			ChimeraCharacter sc = ChimeraCharacter.Cast(m_Shooter);
			CharacterControllerComponent ctrl;
			if (sc)
				ctrl = sc.GetCharacterController();

			if (kind == "scripted event" && s >= 4 && m_iShots < 3 && s >= 4 + m_iShots)
			{
				bool ok = RaiseShot();
				m_iShots++;
				m_sFired = "event raised=" + ok.ToString();
				if (m_fFirst < 0)
					m_fFirst = now;
			}

			if (kind == "real shot" && ctrl)
			{
				if (s >= 4 && m_iShots == 0)
				{
					m_iAmmoBefore = Ammo();
					ctrl.SetWeaponRaised(true);
					m_iShots = 1;
				}
				if (s >= 5.5 && m_iShots == 1)
				{
					ctrl.SetFireWeaponWanted(true);
					m_iShots = 2;
					m_fFirst = now;
				}
				if (s >= 7 && m_iShots == 2)
				{
					ctrl.SetFireWeaponWanted(false);
					m_iShots = 3;
					IEntity inHands = ctrl.GetCurrentItemInHands();
					m_sFired = string.Format("ammo %1 -> %2, item in hands=%3", m_iAmmoBefore, Ammo(), inHands != null);
				}
			}

			if (m_fFirst < 0)
				return;

			float since = now - m_fFirst;
			for (int j = 0; j < m_aListeners.Count(); j++)
			{
				if (m_aListeners[j] && m_aCharged[j] < 0 && m_aDist[j] - RZL.DistXZ(m_aListeners[j].GetOrigin(), m_vShooter) >= 30)
					m_aCharged[j] = since;
			}

			if (since >= 30)
			{
				for (int k = 0; k < m_aListeners.Count(); k++)
				{
					float left = -1;
					if (m_aListeners[k])
						left = RZL.DistXZ(m_aListeners[k].GetOrigin(), m_vShooter);
					RZL.Row("noise", string.Format("case=%1|fired=%2|listenerDist=%3|cameAfter=%4|distAfter30s=%5", kind, m_sFired, RZL.F(m_aDist[k]), RZL.F(m_aCharged[k]), RZL.F(left)));
				}
				Next(now);
			}
			return;
		}

		if (m_iStep == 2)
		{
			RZL.DelAll(m_aChars, m_aGroups);
			m_iCase++;
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
