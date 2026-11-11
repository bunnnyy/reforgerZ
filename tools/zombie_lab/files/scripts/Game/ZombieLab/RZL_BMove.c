// Test-only (not part of the mod): behaviour tests about moving: B3 per-member slots, B7 gait levers,
// B20 a fan and a pincer written as tracks (/root/rz_tools/behaviour_addendum.md, sections 4 and 6).

//------------------------------------------------------------------------------------------------
//! Formation slot offsets (x to the side, z forward), section 6.2.
class RZL_Form
{
	//------------------------------------------------------------------------------------------------
	static vector Offset(string form, int i, int n, float w)
	{
		float c = (n - 1) * 0.5;
		float fi = i;
		if (form == "file")
			return Vector(0, 0, -fi * 5);

		if (form == "wedge")
		{
			int k = (i + 1) / 2;
			float fk = k;
			float side = 1;
			if (i % 2 == 1)
				side = -1;
			return Vector(side * fk * w, 0, -fk * 4);
		}

		return Vector((fi - c) * w, 0, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! A point from an anchor, a heading yaw and an offset.
	static vector At(vector anchor, float yaw, vector off)
	{
		return RZL.Ground(anchor + vector.FromYaw(yaw + 90) * off[0] + vector.FromYaw(yaw) * off[2]);
	}
}

//------------------------------------------------------------------------------------------------
//! B3. A group of 4 in a file, a line and a wedge, steered by per-member slots (L2), with and without
//! a held waypoint. Two anchors 40 m apart, 50 s each: how many are within 4 m of their slot.
//! Bodies 0 and 1 are muted (L15) to check the voice hook.
class RZL_ExpB3 : RZL_BExp
{
	static const float PHASE = 50;

	protected bool m_bWp;
	protected ref array<string> m_aForms = {"file", "line", "wedge"};
	protected int m_iForm;
	protected int m_iPhase;
	protected SCR_AIGroup m_G;
	protected AIWaypoint m_Wp;
	protected ref array<IEntity> m_aM = {};
	protected vector m_vAnchor;
	protected float m_fPhaseAt;
	protected float m_fSettled;
	protected float m_fLastCheck;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpB3(bool withWaypoint)
	{
		m_sName = "b3";
		m_bWp = withWaypoint;
		if (RZL_LabComponent.s_bQuick)
		{
			m_aForms.Clear();
			m_aForms.Insert("line");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void WriteSlots()
	{
		string form = m_aForms[m_iForm];
		for (int i = 0; i < m_aM.Count(); i++)
			RZL_B.Slot(m_aM[i], RZL_Form.At(m_vAnchor, m_Site.m_fYaw, RZL_Form.Offset(form, i, m_aM.Count(), 6)));
	}

	//------------------------------------------------------------------------------------------------
	protected int Within(float r, out float mean, out float most)
	{
		int n;
		mean = 0;
		most = 0;
		int counted;
		foreach (IEntity m : m_aM)
		{
			if (!m)
				continue;

			float d = RZL.DistXZ(m.GetOrigin(), RZL_B.SlotOf(m));
			mean += d;
			most = Math.Max(most, d);
			counted++;
			if (d <= r)
				n++;
		}
		if (counted > 0)
			mean = mean / counted;
		return n;
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		float s = Since(now);
		if (m_iStep == 0)
		{
			if (m_iForm >= m_aForms.Count())
			{
				Finish();
				return;
			}

			vector a0 = m_Site.Along(20);
			m_G = RZL.SpawnEmptyGroup(a0);
			m_aGroups.Insert(m_G);
			m_aM.Clear();
			for (int i = 0; i < 4; i++)
			{
				IEntity ch = RZL.SpawnMember(m_G, RZL.CIV, m_Site.Side(20, (i - 1.5) * 2), m_Site.m_fYaw);
				RZL.ForceLOD(ch, 0);
				RZL_B.Steer(ch, ch.GetOrigin());
				Bacon_622120A5448725E3_InfectedCharacter inf = RZL_B.Inf(ch);
				if (inf && i < 2)
					inf.m_bRZLMute = true;
				m_aM.Insert(ch);
				m_aChars.Insert(ch);
			}
			if (m_bWp)
				m_Wp = RZL.NewWaypoint(a0);
			m_iPhase = 0;
			Next(now);
			return;
		}

		if (m_iStep == 1 && s >= 3)
		{
			float along = 60 + m_iPhase * 40;
			m_vAnchor = m_Site.Along(along);
			WriteSlots();
			if (m_bWp)
				RZL.MoveHeld(m_G, m_Wp, m_vAnchor);
			m_fPhaseAt = now;
			m_fSettled = -1;
			m_fLastCheck = now;
			Next(now);
			return;
		}

		if (m_iStep == 2)
		{
			float t = now - m_fPhaseAt;
			float mean;
			float most;
			if (now - m_fLastCheck >= 2)
			{
				m_fLastCheck = now;
				if (m_iPhase == 0 && t > 3 && t < 4)
				{
					foreach (IEntity m0 : m_aM)
						RZL_B.Factor(m0, 0);
				}
				int inNow = Within(4, mean, most);
				if (m_fSettled < 0 && inNow == m_aM.Count())
					m_fSettled = t;
			}
			if (t < PHASE)
				return;

			int within = Within(4, mean, most);
			int loiter;
			int soundsMuted;
			int soundsOn;
			for (int j = 0; j < m_aM.Count(); j++)
			{
				Bacon_622120A5448725E3_InfectedCharacter infj = RZL_B.Inf(m_aM[j]);
				if (!infj)
					continue;

				loiter += infj.m_iRZLLoiterN;
				if (j < 2)
					soundsMuted += infj.m_iRZLSounds;
				else
					soundsOn += infj.m_iRZLSounds;
			}
			RZL.Row("b3", string.Format("formation=%1|heldWaypoint=%2|anchor=%3|withinFourM=%4 of %5|meanFromSlot=%6|maxFromSlot=%7|allWithinFourMAfter=%8|waypointStillInGroup=%9", m_aForms[m_iForm], m_bWp, m_iPhase + 1, within, m_aM.Count(), RZL.F(mean), RZL.F(most), RZL.F(m_fSettled), RZL.HasWaypoint(m_G, m_Wp)) + string.Format("|loiterHookCalls=%1|soundsMutedPair=%2|soundsOtherPair=%3", loiter, soundsMuted, soundsOn));
			m_iPhase++;
			if (m_iPhase < 2)
			{
				m_iStep = 1;
				m_fStepAt = now - 3;
				return;
			}
			Next(now);
			return;
		}

		if (m_iStep == 3)
		{
			RZL.Del(m_Wp);
			RZL.DelAll(m_aChars, m_aGroups);
			m_iForm++;
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
//! B20. A fan of 5 closing on a target at walk-stop (section 6.3), then a pincer of two pairs
//! (section 6.4, scaled: flank radius 60 m, start 120 m out). Positions every second as track rows.
class RZL_ExpB20 : RZL_BExp
{
	protected ref array<string> m_aCases = {"fan", "pincer"};
	protected int m_iCase;
	protected IEntity m_Target;
	protected ref array<IEntity> m_aM = {};
	protected float m_fT0;
	protected float m_fLastTrack;
	protected float m_fFanAt;
	protected float m_fMinSpacing;
	protected float m_fMaxWidth;
	protected ref array<int> m_aLeg = {0, 0};
	protected ref array<float> m_aArrive = {-1, -1};
	protected vector m_vW1a;
	protected vector m_vW1b;
	protected vector m_vFa;
	protected vector m_vFb;
	protected float m_fReleaseAt;
	protected int m_iStrikeContacts;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpB20()
	{
		m_sName = "b20";
	}

	//------------------------------------------------------------------------------------------------
	protected vector Centre(int from, int to)
	{
		vector sum = vector.Zero;
		int n;
		for (int i = from; i < to && i < m_aM.Count(); i++)
		{
			if (!m_aM[i])
				continue;

			sum = sum + m_aM[i].GetOrigin();
			n++;
		}
		if (n == 0)
			return vector.Zero;

		float fn = n;
		return sum * (1 / fn);
	}

	//------------------------------------------------------------------------------------------------
	protected void Track(float t)
	{
		for (int i = 0; i < m_aM.Count(); i++)
		{
			if (!m_aM[i])
				continue;

			vector p = m_aM[i].GetOrigin();
			vector sl = RZL_B.SlotOf(m_aM[i]);
			RZL.Row("b20t", string.Format("case=%1|t=%2|i=%3|x=%4|z=%5|slotX=%6|slotZ=%7", m_aCases[m_iCase], RZL.F(t), i, RZL.F(p[0]), RZL.F(p[2]), RZL.F(sl[0]), RZL.F(sl[2])));
		}
		float spacing = 1000;
		for (int a = 0; a < m_aM.Count(); a++)
		{
			for (int b = a + 1; b < m_aM.Count(); b++)
			{
				if (m_aM[a] && m_aM[b])
					spacing = Math.Min(spacing, RZL.DistXZ(m_aM[a].GetOrigin(), m_aM[b].GetOrigin()));
			}
		}
		if (t > 20)
			m_fMinSpacing = Math.Min(m_fMinSpacing, spacing);
	}

	//------------------------------------------------------------------------------------------------
	//! The fan's slots from the group's distance to the target.
	protected void FanSlots(float t)
	{
		vector T = m_Target.GetOrigin();
		vector g = Centre(0, m_aM.Count());
		float gd = RZL.DistXZ(g, T);
		float D = Math.Max(gd - 18, 35);
		float w = Math.Clamp(4 + 0.1 * (200 - gd), 4, 20);
		int n = m_aM.Count();
		float c = (n - 1) * 0.5;
		float backYaw = m_Site.m_fYaw;
		bool fan = w * (n - 1) > 30;
		if (fan && m_fFanAt < 0)
			m_fFanAt = t;
		m_fMaxWidth = Math.Max(m_fMaxWidth, w * (n - 1));
		for (int i = 0; i < n; i++)
		{
			float fi = i;
			vector slot;
			if (fan)
			{
				float theta = (fi - c) * w / D;
				slot = T + vector.FromYaw(backYaw + theta * Math.RAD2DEG) * D;
			}
			else
			{
				vector anchor = T + vector.FromYaw(backYaw) * D;
				slot = anchor + vector.FromYaw(backYaw + 90) * ((fi - c) * w);
			}
			RZL_B.Slot(m_aM[i], slot);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! One pair of the pincer: leg 1 to W1, then leg 2 to F, at walk-stop (slots 18 m ahead).
	protected void PairSlots(int pair, float t)
	{
		int i0 = pair * 2;
		vector pc = Centre(i0, i0 + 2);
		vector w1 = m_vW1a;
		vector f = m_vFa;
		if (pair == 1)
		{
			w1 = m_vW1b;
			f = m_vFb;
		}
		if (m_aLeg[pair] == 0 && RZL.DistXZ(pc, w1) < 8)
			m_aLeg[pair] = 1;
		vector goal = w1;
		if (m_aLeg[pair] == 1)
			goal = f;
		if (m_aLeg[pair] == 1 && m_aArrive[pair] < 0 && RZL.DistXZ(pc, f) < 8)
			m_aArrive[pair] = t;
		vector dir = goal - pc;
		dir[1] = 0;
		float len = dir.Length();
		if (len > 0.1)
			dir = dir * (1 / len);
		vector lead = pc + dir * Math.Min(18, len);
		if (m_aArrive[pair] >= 0)
			lead = f;
		vector side = Vector(dir[2], 0, -dir[0]);
		RZL_B.Slot(m_aM[i0], lead);
		if (i0 + 1 < m_aM.Count())
			RZL_B.Slot(m_aM[i0 + 1], lead + side * 2 - dir * 4);
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
			int n = 5;
			float start = 200;
			if (kind == "pincer")
			{
				n = 4;
				start = 120;
			}
			m_aM.Clear();
			for (int i = 0; i < n; i++)
			{
				vector p = m_Site.Side(start, (i - (n - 1) * 0.5) * 3);
				IEntity ch = RZL.SpawnSolo(RZL.CIV, p, m_Site.m_fYaw + 180, m_aGroups);
				RZL_B.Steer(ch, p);
				m_aM.Insert(ch);
				m_aChars.Insert(ch);
			}
			m_Target = RZL_B.Target(m_Site.Side(150, 1500), 0, true);
			m_aChars.Insert(m_Target);
			m_fFanAt = -1;
			m_fMinSpacing = 1000;
			m_fMaxWidth = 0;
			m_aLeg[0] = 0;
			m_aLeg[1] = 0;
			m_aArrive[0] = -1;
			m_aArrive[1] = -1;
			m_fReleaseAt = -1;
			Next(now);
			return;
		}

		if (m_iStep == 1 && s >= 3)
		{
			foreach (IEntity m1 : m_aM)
				RZL_B.Factor(m1, 0);
			RZL.Teleport(m_Target, m_Site.m_vT, m_Site.m_fYaw);
			vector T = m_Site.m_vT;
			RZL.Row("b20t", string.Format("case=%1|t=0|i=target|x=%2|z=%3", m_aCases[m_iCase], RZL.F(T[0]), RZL.F(T[2])));
			if (m_aCases[m_iCase] == "pincer")
			{
				// Flank points: R_f 60 m, alpha 70 degrees
				vector g = Centre(0, m_aM.Count());
				vector u = T - g;
				u[1] = 0;
				u.Normalize();
				vector pv = Vector(u[2], 0, -u[0]);
				float rf = 60;
				float sa = Math.Sin(70 * Math.DEG2RAD);
				float ca = Math.Cos(70 * Math.DEG2RAD);
				m_vW1a = RZL.Ground(g - pv * (rf * sa));
				m_vW1b = RZL.Ground(g + pv * (rf * sa));
				m_vFa = RZL.Ground(T - u * (rf * ca) - pv * (rf * sa));
				m_vFb = RZL.Ground(T - u * (rf * ca) + pv * (rf * sa));
				RZL.Row("b20t", string.Format("case=pincer|t=0|i=flank|x=%1|z=%2|slotX=%3|slotZ=%4", RZL.F(m_vFa[0]), RZL.F(m_vFa[2]), RZL.F(m_vFb[0]), RZL.F(m_vFb[2])));
			}
			m_fT0 = now;
			m_fLastTrack = -1;
			Next(now);
			return;
		}

		if (m_iStep == 2)
		{
			float t = now - m_fT0;
			string kind2 = m_aCases[m_iCase];
			if (t - m_fLastTrack < 1)
				return;

			m_fLastTrack = t;
			if (kind2 == "fan" && m_fReleaseAt < 0)
				FanSlots(t);
			if (kind2 == "pincer" && m_fReleaseAt < 0)
			{
				PairSlots(0, t);
				PairSlots(1, t);
			}
			Track(t);
			vector T2 = m_Target.GetOrigin();
			bool release = false;
			string why = "";
			if (kind2 == "fan" && RZL.DistXZ(Centre(0, m_aM.Count()), T2) <= 38)
			{
				release = true;
				why = "within 38 m";
			}
			if (kind2 == "pincer")
			{
				if (m_aArrive[0] >= 0 && m_aArrive[1] >= 0)
				{
					release = true;
					why = "both in position";
				}
				else if ((m_aArrive[0] >= 0 && t - m_aArrive[0] >= 60) || (m_aArrive[1] >= 0 && t - m_aArrive[1] >= 60))
				{
					release = true;
					why = "60 s after the first";
				}
			}
			if (t > 300)
			{
				release = true;
				why = "timeout 300 s";
			}
			if (m_fReleaseAt < 0 && release)
			{
				m_fReleaseAt = t;
				foreach (IEntity mr : m_aM)
					RZL_B.Release(mr, m_Target, true);
				RZL.Row("b20", string.Format("case=%1|releasedAt=%2|why=%3|fanFromSecond=%4|maxFrontWidth=%5|minSpacingAfter20s=%6|pairArrived=%7 / %8", kind2, RZL.F(t), why, RZL.F(m_fFanAt), RZL.F(m_fMaxWidth), RZL.F(m_fMinSpacing), RZL.F(m_aArrive[0]), RZL.F(m_aArrive[1])));
			}
			if (m_fReleaseAt >= 0 && t - m_fReleaseAt >= 15)
			{
				int close;
				foreach (IEntity mc : m_aM)
				{
					if (mc && RZL.DistXZ(mc.GetOrigin(), T2) < 3)
						close++;
				}
				RZL.Row("b20", string.Format("case=%1|strike=within 3 m of the target 15 s after the release: %2 of %3", kind2, close, m_aM.Count()));
				Next(now);
			}
			return;
		}

		if (m_iStep == 3)
		{
			RZL.DelAll(m_aChars, m_aGroups);
			m_iCase++;
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
//! B7. Gait levers: (a) speed caps on a RUN order and (b) in a chase, per prefab; (c) SPRINT wanted
//! in a chase; (d) Roam's walk-stop with slots 10/20/24/30 m ahead, with dynamic stances 0.6/0.7.
class RZL_ExpB7 : RZL_BExp
{
	static const float WIN = 24;

	protected ref array<float> m_aFrac = {1, 0.8, 0.6, 0.4, 0.2};
	protected ref array<string> m_aPrefab = {RZL.CIV, RZL.MIL, RZL.HEAVY};
	protected ref array<string> m_aName = {"civilian", "military", "heavy"};
	protected ref array<IEntity> m_aB = {};
	protected ref array<SCR_AIGroup> m_aG = {};
	protected ref array<AIWaypoint> m_aW = {};
	protected ref array<ref RZL_Mover> m_aMv = {};
	protected ref array<float> m_aSum = {};
	protected ref array<int> m_aN = {};
	protected ref array<float> m_aMax = {};
	protected ref array<float> m_aAnim = {};
	protected ref array<float> m_aStance = {};
	protected ref array<int> m_aMoving = {};
	protected ref array<int> m_aWantSprint = {};
	protected int m_iJ;
	protected int m_iPart;
	protected float m_fT0;
	protected IEntity m_Target;
	protected int m_iChaser;
	protected int m_iSprintMode;
	protected ref array<float> m_aStep = {10, 20, 24, 30, 20, 20, 20};
	protected ref array<float> m_aStanceWanted = {0, 0, 0, 0, 0.6, 0.7, 0.9};
	protected ref array<int> m_aDir = {};
	protected ref array<int> m_aHops = {};
	protected float m_fLastInject;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpB7()
	{
		m_sName = "b7";
		if (RZL_LabComponent.s_bQuick)
		{
			m_aFrac.Clear();
			m_aFrac.Insert(1);
			m_aFrac.Insert(0.4);
			m_aPrefab.Remove(1);
			m_aName.Remove(1);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearSamples(int n)
	{
		m_aSum.Clear();
		m_aN.Clear();
		m_aMax.Clear();
		m_aAnim.Clear();
		m_aStance.Clear();
		m_aMoving.Clear();
		m_aWantSprint.Clear();
		for (int i = 0; i < n; i++)
		{
			m_aSum.Insert(0);
			m_aN.Insert(0);
			m_aMax.Insert(0);
			m_aAnim.Insert(0);
			m_aStance.Insert(0);
			m_aMoving.Insert(0);
			m_aWantSprint.Insert(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! One sample per body; `onlyMoving` keeps samples above 0.3 m/s for the speed.
	protected void SampleAll(bool onlyMoving)
	{
		for (int i = 0; i < m_aB.Count(); i++)
		{
			if (!m_aB[i])
				continue;

			m_aMv[i].Sample();
			float v = m_aMv[i].m_fSpeed;
			CharacterControllerComponent c = RZL_B.Ctrl(m_aB[i]);
			if (v > 0.3)
				m_aMoving[i] = m_aMoving[i] + 1;
			if (RZL_B.MoveWanted(m_aB[i]) == EMovementType.SPRINT)
				m_aWantSprint[i] = m_aWantSprint[i] + 1;
			if (onlyMoving && v <= 0.3)
				continue;

			m_aSum[i] = m_aSum[i] + v;
			m_aN[i] = m_aN[i] + 1;
			m_aMax[i] = Math.Max(m_aMax[i], v);
			if (c)
			{
				m_aAnim[i] = m_aAnim[i] + c.GetMovementSpeed();
				m_aStance[i] = m_aStance[i] + c.GetDynamicStance();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected float Mean(array<float> a, int i)
	{
		if (m_aN[i] <= 0)
			return -1;

		return a[i] / m_aN[i];
	}

	//------------------------------------------------------------------------------------------------
	protected void Clean()
	{
		foreach (AIWaypoint w : m_aW)
			RZL.Del(w);
		m_aW.Clear();
		RZL.DelAll(m_aChars, m_aGroups);
		m_aB.Clear();
		m_aG.Clear();
		m_aMv.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! The chase rig: one body, a target that can't be hurt kept 35 m ahead (it jumps on when the body
	//! is within 10 m), named to the body by our share event.
	protected void ChaseTick(float now, float yaw)
	{
		IEntity b = m_aB[0];
		if (!b || !m_Target)
			return;

		if (RZL.DistXZ(b.GetOrigin(), m_Target.GetOrigin()) < 10)
		{
			RZL.Teleport(m_Target, b.GetOrigin() + vector.FromYaw(yaw) * 35, yaw);
			RZL_B.Inject(b, m_Target);
			m_fLastInject = now;
		}
		if (now - m_fLastInject >= 2)
		{
			RZL_B.Inject(b, m_Target);
			m_fLastInject = now;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		float s = Since(now);
		// Part 0: caps on a RUN order, three prefabs side by side
		if (m_iPart == 0)
		{
			if (m_iStep == 0)
			{
				for (int i = 0; i < m_aPrefab.Count(); i++)
				{
					vector p = m_Site.Side(150, (i - 1) * 15);
					IEntity ch = RZL.SpawnSolo(m_aPrefab[i], p, m_Site.m_fYaw, m_aGroups);
					m_aB.Insert(ch);
					m_aChars.Insert(ch);
					m_aG.Insert(SCR_AIGroup.Cast(m_aGroups[m_aGroups.Count() - 1]));
					m_aW.Insert(RZL.NewWaypoint(p));
					m_aMv.Insert(new RZL_Mover(ch));
				}
				m_iJ = 0;
				Next(now);
				return;
			}

			if (m_iStep == 1 && s >= 3)
			{
				if (m_iJ >= m_aFrac.Count())
				{
					Clean();
					m_iPart = 1;
					m_iJ = 0;
					m_iChaser = 0;
					m_iStep = 0;
					m_fStepAt = now;
					return;
				}

				float yaw = m_Site.m_fYaw;
				if (m_iJ % 2 == 1)
					yaw += 180;
				for (int k = 0; k < m_aB.Count(); k++)
				{
					RZL_B.Cap(m_aB[k], m_aFrac[m_iJ]);
					if (m_aB[k])
						RZL.MoveHeld(m_aG[k], m_aW[k], m_aB[k].GetOrigin() + vector.FromYaw(yaw) * 140);
					m_aMv[k].Reset();
				}
				ClearSamples(m_aB.Count());
				m_fT0 = now;
				Next(now);
				return;
			}

			if (m_iStep == 2)
			{
				float t = now - m_fT0;
				if (t >= 4)
					SampleAll(true);
				if (t < WIN)
					return;

				for (int r = 0; r < m_aB.Count(); r++)
				{
					float mvN = m_aMoving[r];
					RZL.Row("b7cap", string.Format("prefab=%1|context=RUN order (held waypoint)|fraction=%2|speed=%3|maxSpeed=%4|animMovementSpeed=%5|movingShare=%6", m_aName[r], RZL.F2(m_aFrac[m_iJ]), RZL.F2(Mean(m_aSum, r)), RZL.F2(m_aMax[r]), RZL.F2(Mean(m_aAnim, r)), RZL.F2(mvN / ((WIN - 4) * 4))));
				}
				m_iJ++;
				m_iStep = 1;
				m_fStepAt = now;
				return;
			}
			return;
		}

		// Part 1: caps in a chase (each prefab in turn, every fraction); part 2: SPRINT wanted in a chase
		if (m_iPart == 1 || m_iPart == 2)
		{
			int runs = m_aFrac.Count();
			if (m_iPart == 2)
				runs = 2;
			if (m_iStep == 0)
			{
				if (m_iChaser >= m_aPrefab.Count())
				{
					m_iPart++;
					m_iChaser = 0;
					m_iJ = 0;
					m_fStepAt = now;
					return;
				}

				vector p1 = m_Site.Along(150);
				IEntity c1 = RZL.SpawnSolo(m_aPrefab[m_iChaser], p1, m_Site.m_fYaw, m_aGroups);
				m_aB.Insert(c1);
				m_aChars.Insert(c1);
				m_aMv.Insert(new RZL_Mover(c1));
				m_Target = RZL_B.Target(m_Site.Side(150, 1500), 0, true);
				m_aChars.Insert(m_Target);
				m_iJ = 0;
				Next(now);
				return;
			}

			if (m_iStep == 1 && s >= 3)
			{
				if (m_iJ >= runs)
				{
					Clean();
					m_iChaser++;
					m_iStep = 0;
					m_fStepAt = now;
					return;
				}

				float yaw1 = m_Site.m_fYaw;
				if (m_iJ % 2 == 1)
					yaw1 += 180;
				float frac = 1;
				if (m_iPart == 1)
					frac = m_aFrac[m_iJ];
				RZL_B.Cap(m_aB[0], frac);
				m_iSprintMode = 0;
				if (m_iPart == 2)
					m_iSprintMode = m_iJ + 1;
				RZL.Teleport(m_Target, m_aB[0].GetOrigin() + vector.FromYaw(yaw1) * 35, yaw1);
				RZL_B.Inject(m_aB[0], m_Target);
				ClearSamples(1);
				m_aMv[0].Reset();
				m_fT0 = now;
				Next(now);
				return;
			}

			if (m_iStep == 2)
			{
				float t2 = now - m_fT0;
				float yaw2 = m_Site.m_fYaw;
				if (m_iJ % 2 == 1)
					yaw2 += 180;
				ChaseTick(now, yaw2);
				if ((m_iSprintMode == 1 && t2 >= 2 && t2 < 2.3) || (m_iSprintMode == 2 && t2 >= 2))
					RZL_B.WantMove(m_aB[0], EMovementType.SPRINT);
				if (t2 >= 6 && m_aB[0] && m_Target && m_aMv[0].Toward(m_Target.GetOrigin(), 0.5))
					SampleAll(false);
				else if (t2 >= 6)
					m_aMv[0].Sample();
				if (t2 < WIN)
					return;

				string ctx = string.Format("chase, cap %1", RZL.F2(m_aFrac[m_iJ]));
				float fr = m_aFrac[m_iJ];
				if (m_iPart == 2)
				{
					fr = 1;
					ctx = "chase, SPRINT wanted once at 2 s";
					if (m_iSprintMode == 2)
						ctx = "chase, SPRINT wanted every 0.25 s";
				}
				RZL.Row("b7cap", string.Format("prefab=%1|context=%2|fraction=%3|speed=%4|maxSpeed=%5|animMovementSpeed=%6|samplesTowardTarget=%7|sprintWantedSamples=%8|moveWantedAtEnd=%9", m_aName[m_iChaser], ctx, RZL.F2(fr), RZL.F2(Mean(m_aSum, 0)), RZL.F2(m_aMax[0]), RZL.F2(Mean(m_aAnim, 0)), m_aN[0], m_aWantSprint[0], RZL_B.MoveWanted(m_aB[0])));
				m_iJ++;
				m_iStep = 1;
				m_fStepAt = now - 3;
				return;
			}
			return;
		}

		// Part 3: Roam's walk-stop with slots N m ahead, and dynamic stances
		if (m_iPart == 3)
		{
			if (m_iStep == 0)
			{
				m_aDir.Clear();
				m_aHops.Clear();
				for (int i3 = 0; i3 < m_aStep.Count(); i3++)
				{
					vector p3 = m_Site.Side(120, (i3 - 3) * 12);
					IEntity c3 = RZL.SpawnSolo(RZL.CIV, p3, m_Site.m_fYaw, m_aGroups);
					RZL_B.Steer(c3, p3);
					Bacon_622120A5448725E3_InfectedCharacter inf = RZL_B.Inf(c3);
					if (inf)
						inf.m_fRZLStance = m_aStanceWanted[i3];
					m_aB.Insert(c3);
					m_aChars.Insert(c3);
					m_aMv.Insert(new RZL_Mover(c3));
					m_aDir.Insert(1);
					m_aHops.Insert(0);
				}
				Next(now);
				return;
			}

			if (m_iStep == 1 && s >= 3)
			{
				foreach (IEntity cb : m_aB)
					RZL_B.Factor(cb, 0);
				ClearSamples(m_aB.Count());
				m_fT0 = now;
				Next(now);
				return;
			}

			if (m_iStep == 2)
			{
				float t3 = now - m_fT0;
				for (int k3 = 0; k3 < m_aB.Count(); k3++)
				{
					IEntity bk = m_aB[k3];
					if (!bk)
						continue;

					vector sl = RZL_B.SlotOf(bk);
					if (m_aHops[k3] == 0 || RZL.DistXZ(bk.GetOrigin(), sl) < 3)
					{
						m_aHops[k3] = m_aHops[k3] + 1;
						if (m_aHops[k3] % 4 == 0)
							m_aDir[k3] = -m_aDir[k3];
						float yawk = m_Site.m_fYaw;
						if (m_aDir[k3] < 0)
							yawk += 180;
						RZL_B.Slot(bk, bk.GetOrigin() + vector.FromYaw(yawk) * m_aStep[k3]);
					}
				}
				if (t3 >= 2)
					SampleAll(false);
				if (t3 < 75)
					return;

				for (int r3 = 0; r3 < m_aB.Count(); r3++)
				{
					float nMoving = m_aMoving[r3];
					float nAll = m_aN[r3];
					float movingShare = 0;
					if (nAll > 0)
						movingShare = nMoving / nAll;
					float movingSpeed = -1;
					if (nMoving > 0)
						movingSpeed = m_aSum[r3] / nMoving;
					RZL.Row("b7walk", string.Format("slotAhead=%1 m|stanceWanted=%2|meanSpeedWithStops=%3|speedWhileMoving=%4|movingShare=%5|dynamicStanceRead=%6|slotsReached=%7|maxSpeed=%8", m_aStep[r3], RZL.F2(m_aStanceWanted[r3]), RZL.F2(Mean(m_aSum, r3)), RZL.F2(movingSpeed), RZL.F2(movingShare), RZL.F2(Mean(m_aStance, r3)), m_aHops[r3] - 1, RZL.F2(m_aMax[r3])));
				}
				Finish();
			}
		}
	}
}
