// Test-only (not part of the mod): behaviour tests about navigation: B8 the climb gate, B9 stuck and
// the unstick ladder, B16 water, B17 the heavy's contact (/root/rz_tools/behaviour_addendum.md, 7).
// The walls are spawned props: the baked navmesh doesn't know them, so a body walks straight into
// one and stops (Bacon's climb test runs only then), unless the navmesh is rebuilt round them.

//------------------------------------------------------------------------------------------------
//! A body's place in a site's frame: along the site's yaw from T, and to its side.
class RZL_Frame
{
	//------------------------------------------------------------------------------------------------
	static float Along(RZL_Site s, vector p)
	{
		vector d = p - s.m_vT;
		vector f = vector.FromYaw(s.m_fYaw);
		return d[0] * f[0] + d[2] * f[2];
	}

	//------------------------------------------------------------------------------------------------
	static float Lateral(RZL_Site s, vector p)
	{
		vector d = p - s.m_vT;
		vector r = vector.FromYaw(s.m_fYaw + 90);
		return d[0] * r[0] + d[2] * r[2];
	}
}

//------------------------------------------------------------------------------------------------
//! B8. A course of walls about 0.8, 1.2, 1.4, 1.8 and 2.5 m high (2 pieces each), one body per wall
//! walking to a slot behind it: Bacon's climb (logged), then our gate at 1.4 m and at 1.2 m.
class RZL_ExpB8 : RZL_BExp
{
	static const float WALL_AT = 60;
	static const float LANE = 25;

	protected ref array<float> m_aWant = {0.8, 1.2, 1.4, 1.8, 2.5};
	protected ref array<int> m_aWall = {};
	protected ref array<float> m_aLimit = {0, 1.4, 1.2};
	protected int m_iMode;
	protected ref array<IEntity> m_aB = {};
	protected ref array<float> m_aCrossed = {};
	protected ref array<float> m_aCrossLat = {};
	protected ref array<bool> m_aClimbSeen = {};
	protected ref array<vector> m_aLast = {};
	protected float m_fT0;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpB8()
	{
		m_sName = "b8";
		if (RZL_LabComponent.s_bQuick)
			m_aLimit.Remove(2);
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		float s = Since(now);
		if (m_iStep == 0)
		{
			// The course, once
			for (int i = 0; i < m_aWant.Count(); i++)
			{
				int w = RZL_Walls.Nearest(m_aWant[i], false);
				m_aWall.Insert(w);
				float lat = (i - 2) * LANE;
				RZL_Walls.Build(w, m_Site.Side(WALL_AT, lat), m_Site.m_fYaw, 2, m_aProps);
			}
			m_iMode = 0;
			Next(now);
			return;
		}

		if (m_iStep == 1 && s >= 2)
		{
			if (m_iMode >= m_aLimit.Count())
			{
				Finish();
				return;
			}

			m_aB.Clear();
			m_aCrossed.Clear();
			m_aCrossLat.Clear();
			m_aClimbSeen.Clear();
			m_aLast.Clear();
			for (int j = 0; j < m_aWall.Count(); j++)
			{
				float latj = (j - 2) * LANE;
				vector p = m_Site.Side(WALL_AT - 12, latj);
				IEntity ch = RZL.SpawnSolo(RZL.CIV, p, m_Site.m_fYaw, m_aGroups);
				RZL_B.Steer(ch, m_Site.Side(WALL_AT + 12, latj));
				Bacon_622120A5448725E3_InfectedCharacter inf = RZL_B.Inf(ch);
				if (inf)
				{
					inf.m_iRZLClimb = 1;
					if (m_aLimit[m_iMode] > 0)
					{
						inf.m_iRZLClimb = 2;
						inf.m_fRZLClimbLimit = m_aLimit[m_iMode];
					}
				}
				m_aB.Insert(ch);
				m_aChars.Insert(ch);
				m_aCrossed.Insert(-1);
				m_aCrossLat.Insert(0);
				m_aClimbSeen.Insert(false);
				m_aLast.Insert(p);
			}
			m_fT0 = now;
			Next(now);
			return;
		}

		if (m_iStep == 2)
		{
			float t = now - m_fT0;
			if (t > 1 && t < 1.3)
			{
				foreach (IEntity b0 : m_aB)
					RZL_B.Factor(b0, 0);
			}
			for (int k = 0; k < m_aB.Count(); k++)
			{
				IEntity b = m_aB[k];
				if (!b)
					continue;

				CharacterControllerComponent c = RZL_B.Ctrl(b);
				if (c && c.IsClimbing())
					m_aClimbSeen[k] = true;
				if (m_aCrossed[k] < 0 && RZL_Frame.Along(m_Site, b.GetOrigin()) > WALL_AT + 1)
				{
					m_aCrossed[k] = t;
					m_aCrossLat[k] = RZL_Frame.Lateral(m_Site, b.GetOrigin()) - (k - 2) * LANE;
				}
				if (Math.Mod(t, 10) < 0.25)
					m_aLast[k] = b.GetOrigin();
			}
			if (t < 45)
				return;

			string mode = "Bacon's own (logged)";
			if (m_aLimit[m_iMode] > 0)
				mode = string.Format("our gate, limit %1 m", m_aLimit[m_iMode]);
			for (int r = 0; r < m_aB.Count(); r++)
			{
				IEntity br = m_aB[r];
				int wr = m_aWall[r];
				Bacon_622120A5448725E3_InfectedCharacter ir = RZL_B.Inf(br);
				string how = "stopped at it";
				if (m_aCrossed[r] >= 0)
				{
					how = "over it";
					if (Math.AbsFloat(m_aCrossLat[r]) > RZL_Walls.s_aLength[wr] - 0.5)
						how = "round its end";
				}
				else if (br && RZL.DistXZ(br.GetOrigin(), m_aLast[r]) > 3)
					how = "moving, not across";
				string wall = "-";
				float h = -1;
				if (wr >= 0)
				{
					wall = FilePath.StripPath(RZL_Walls.s_aPrefabs[wr]);
					h = RZL_Walls.s_aHeight[wr];
				}
				int ok = -1;
				int refused = -1;
				float grab = -1;
				if (ir)
				{
					ok = ir.m_iRZLClimbOk;
					refused = ir.m_iRZLClimbRefused;
					grab = ir.m_fRZLClimbGrab;
				}
				RZL.Row("b8", string.Format("gate=%1|wall=%2|wallHeight=%3|crossedAfter=%4|how=%5|climbingSeen=%6|climbTestsPassed=%7|refusedByGate=%8|lastGrabHeight=%9", mode, wall, RZL.F2(h), RZL.F(m_aCrossed[r]), how, m_aClimbSeen[r], ok, refused, RZL.F2(grab)));
			}
			RZL.DelAll(m_aChars, m_aGroups);
			m_iMode++;
			m_iStep = 1;
			m_fStepAt = now;
		}
	}
}

//------------------------------------------------------------------------------------------------
//! B17. The heavy (Zombie_Heavy_Tank) run into destructible wooden fences: does its contact break
//! them (m_bDestroysThings)? A civilian at the same fence as the control.
class RZL_ExpB17 : RZL_BExp
{
	protected ref array<IEntity> m_aB = {};
	protected ref array<int> m_aWall = {};
	protected ref array<ref array<IEntity>> m_aPieces = {};
	protected ref array<float> m_aHealth0 = {};
	protected ref array<float> m_aMaxSpeed = {};
	protected ref array<ref RZL_Mover> m_aMv = {};
	protected float m_fT0;
	protected int m_iContacts0;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpB17()
	{
		m_sName = "b17";
	}

	//------------------------------------------------------------------------------------------------
	protected float PiecesHealth(int k, out int left)
	{
		float sum = 0;
		left = 0;
		foreach (IEntity e : m_aPieces[k])
		{
			if (!e)
				continue;

			left++;
			DestructibleEntity de = DestructibleEntity.Cast(e);
			if (de)
				sum += de.GetCurrentHealth();
		}
		return sum;
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		float s = Since(now);
		if (m_iStep == 0)
		{
			int wa = RZL_Walls.Nearest(1.2, true);
			int wb = RZL_Walls.Nearest(1.8, true);
			if (wa < 0)
			{
				RZL.Row("b17", "note=no destructible wall prefab among the candidates");
				Finish();
				return;
			}
			if (wb < 0)
				wb = wa;
			array<string> prefabs = {RZL.HEAVY, RZL.HEAVY, RZL.CIV};
			array<int> walls = {wa, wb, wa};
			m_iContacts0 = RZL_Hooks.s_iHeavyContacts;
			for (int i = 0; i < 3; i++)
			{
				float lat = (i - 1) * 25;
				array<IEntity> pieces = {};
				RZL_Walls.Build(walls[i], m_Site.Side(60, lat), m_Site.m_fYaw, 2, pieces);
				foreach (IEntity pc : pieces)
					m_aProps.Insert(pc);
				m_aPieces.Insert(pieces);
				m_aWall.Insert(walls[i]);
				vector p = m_Site.Side(30, lat);
				IEntity ch = RZL.SpawnSolo(prefabs[i], p, m_Site.m_fYaw, m_aGroups);
				RZL_B.Steer(ch, m_Site.Side(85, lat));
				m_aB.Insert(ch);
				m_aChars.Insert(ch);
				m_aMv.Insert(new RZL_Mover(ch));
				m_aMaxSpeed.Insert(0);
				int left0;
				m_aHealth0.Insert(PiecesHealth(i, left0));
			}
			m_fT0 = now;
			Next(now);
			return;
		}

		if (m_iStep == 1)
		{
			float t = now - m_fT0;
			if (t > 1 && t < 1.3)
			{
				foreach (IEntity b0 : m_aB)
					RZL_B.Factor(b0, 0);
			}
			for (int k = 0; k < m_aB.Count(); k++)
			{
				m_aMv[k].Sample();
				m_aMaxSpeed[k] = Math.Max(m_aMaxSpeed[k], m_aMv[k].m_fSpeed);
			}
			if (t < 40)
				return;

			for (int r = 0; r < m_aB.Count(); r++)
			{
				int left;
				float hp = PiecesHealth(r, left);
				string kind = "civilian (control)";
				string destroys = "-";
				Bacon_622120A5448725E3_InfectedCharacterHeavy hv = Bacon_622120A5448725E3_InfectedCharacterHeavy.Cast(m_aB[r]);
				if (hv)
				{
					kind = "heavy";
					destroys = hv.RZL_DestroysThings().ToString();
				}
				float along = -1;
				if (m_aB[r])
					along = RZL_Frame.Along(m_Site, m_aB[r].GetOrigin());
				RZL.Row("b17", string.Format("body=%1|m_bDestroysThings=%2|wall=%3|piecesLeft=%4 of 2|wallHealth=%5 -> %6|maxSpeed=%7|passedTheWall=%8|heavyContactsWithDestructibles=%9", kind, destroys, FilePath.StripPath(RZL_Walls.s_aPrefabs[m_aWall[r]]), left, RZL.F(m_aHealth0[r]), RZL.F(hp), RZL.F2(m_aMaxSpeed[r]), along > 61, RZL_Hooks.s_iHeavyContacts - m_iContacts0));
			}
			Finish();
		}
	}
}

//------------------------------------------------------------------------------------------------
//! B9. Stuck and the unstick ladder (section 7.2-7.3) against a tall wall the navmesh doesn't know:
//! a yard open toward the body's back, the same yard after a local navmesh rebuild, and a long wall.
//! Also the cost of a navmesh snap (L18).
class RZL_ExpB9 : RZL_BExp
{
	protected ref array<string> m_aCases = {"yard, navmesh not rebuilt", "yard, navmesh rebuilt", "long wall, navmesh not rebuilt", "closed pen, navmesh not rebuilt"};
	protected int m_iCase;
	protected IEntity m_B;
	protected vector m_vGoal;
	protected float m_fT0;
	protected ref array<vector> m_aHist = {};
	protected int m_iLadder;
	protected float m_fStepEnds;
	protected float m_fReached;
	protected string m_sSteps;
	protected int m_iWall;
	protected float m_fLastCheck;
	protected int m_iSide = 1;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpB9()
	{
		m_sName = "b9";
		if (RZL_LabComponent.s_bQuick)
		{
			m_aCases.Remove(2);
			m_aCases.Remove(1);
			m_aCases.Remove(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SnapCost()
	{
		IEntity holder = null;
		if (!m_aGroups.IsEmpty())
			holder = m_aGroups[0];
		vector probe = m_Site.Along(30);
		vector snapped;
		bool any = RZL_B.Snap(holder, probe, 10, snapped);
		string on = "group";
		if (!any && m_B)
		{
			holder = m_B;
			any = RZL_B.Snap(holder, probe, 10, snapped);
			on = "character";
		}
		if (!any && m_B)
		{
			holder = RZL.Agent(m_B);
			any = RZL_B.Snap(holder, probe, 10, snapped);
			on = "agent";
		}
		if (!any)
		{
			RZL.Row("b9snap", "note=no AIPathfindingComponent answered on the group or the character");
			return;
		}

		int ok;
		int t0 = System.GetTickCount();
		for (int i = 0; i < 1000; i++)
		{
			vector q = probe + Vector(Math.RandomFloat(-30, 30), 0, Math.RandomFloat(-30, 30));
			vector snapQ;
			if (RZL_B.Snap(holder, q, 10, snapQ))
				ok++;
		}
		int ms = System.GetTickCount() - t0;
		RZL.Row("b9snap", string.Format("on=%1|calls=1000|succeeded=%2|totalMs=%3|msPerSnap=%4|snapMovedProbeBy=%5", on, ok, ms, RZL.F2(ms / 1000.0), RZL.F2(vector.Distance(probe, snapped))));
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildCase(string kind)
	{
		m_iWall = RZL_Walls.Nearest(2.6, false);
		if (m_iWall < 0)
			return;

		float len = RZL_Walls.s_aLength[m_iWall];
		if (kind.Contains("long wall"))
		{
			RZL_Walls.Build(m_iWall, m_Site.Side(60, 0), m_Site.m_fYaw, 3, m_aProps);
			return;
		}

		// A closed pen: one piece on each side round the body's spot (52 m out)
		if (kind.Contains("pen"))
		{
			float h = len * 0.5;
			RZL_Walls.Build(m_iWall, m_Site.Side(52 + h, 0), m_Site.m_fYaw, 1, m_aProps);
			RZL_Walls.Build(m_iWall, m_Site.Side(52 - h, 0), m_Site.m_fYaw, 1, m_aProps);
			RZL_Walls.Build(m_iWall, m_Site.Side(52, -h), m_Site.m_fYaw + 90, 1, m_aProps);
			RZL_Walls.Build(m_iWall, m_Site.Side(52, h), m_Site.m_fYaw + 90, 1, m_aProps);
			return;
		}

		// The back wall across, two side walls along the approach, open toward T
		RZL_Walls.Build(m_iWall, m_Site.Side(60, 0), m_Site.m_fYaw, 2, m_aProps);
		RZL_Walls.Build(m_iWall, m_Site.Side(60 - len * 0.5, -len), m_Site.m_fYaw + 90, 1, m_aProps);
		RZL_Walls.Build(m_iWall, m_Site.Side(60 - len * 0.5, len), m_Site.m_fYaw + 90, 1, m_aProps);
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
			BuildCase(kind);
			if (m_iWall < 0)
			{
				RZL.Row("b9", "note=no wall prefab spawned");
				Finish();
				return;
			}
			if (kind.Contains("rebuilt"))
			{
				bool asked = RZ_NavRebuild.RequestArea(m_Site.Side(55, 0), 25);
				RZL.Log("b9: navmesh rebuild requested=" + asked.ToString());
			}
			Next(now);
			return;
		}

		// Give a rebuild time
		if (m_iStep == 1 && s >= 10)
		{
			vector p = m_Site.Along(52);
			m_B = RZL.SpawnSolo(RZL.CIV, p, m_Site.m_fYaw, m_aGroups);
			m_aChars.Insert(m_B);
			m_vGoal = m_Site.Along(78);
			RZL_B.Steer(m_B, m_vGoal);
			if (m_iCase == 0 || m_aCases.Count() == 1)
				SnapCost();
			m_aHist.Clear();
			m_iLadder = 0;
			m_fStepEnds = -1;
			m_fReached = -1;
			m_sSteps = "";
			m_fT0 = now;
			m_fLastCheck = now;
			Next(now);
			return;
		}

		if (m_iStep == 2)
		{
			float t = now - m_fT0;
			if (!m_B)
			{
				Next(now);
				return;
			}

			if (t > 1 && t < 1.3)
				RZL_B.Factor(m_B, 0);
			vector pos = m_B.GetOrigin();
			if (m_fReached < 0 && RZL.DistXZ(pos, m_vGoal) < 4)
				m_fReached = t;
			if (now - m_fLastCheck >= 1)
			{
				m_fLastCheck = now;
				m_aHist.Insert(pos);
				// A temporary ladder slot ran out: back to the goal
				if (m_fStepEnds > 0 && t >= m_fStepEnds)
				{
					RZL_B.Slot(m_B, m_vGoal);
					m_fStepEnds = -1;
					m_aHist.Clear();
				}
				// Stuck: under 1.5 m in 10 s with the goal more than 8 m away
				int n = m_aHist.Count();
				if (m_fReached < 0 && m_fStepEnds < 0 && n >= 10 && RZL.DistXZ(pos, m_aHist[n - 10]) < 1.5 && RZL.DistXZ(pos, m_vGoal) > 8)
				{
					m_iLadder++;
					vector dir = m_vGoal - pos;
					dir[1] = 0;
					dir.Normalize();
					vector side = Vector(dir[2], 0, -dir[0]);
					if (m_iLadder == 1)
					{
						RZL_B.Slot(m_B, pos + side * 4);
						m_fStepEnds = t + 6;
					}
					else if (m_iLadder == 2)
					{
						RZL_B.Slot(m_B, pos - dir * 15);
						m_fStepEnds = t + 10;
					}
					else if (m_iLadder == 3)
					{
						vector off = dir * 0.7071 + side * 0.7071;
						RZL_B.Slot(m_B, pos + off * RZL.DistXZ(pos, m_vGoal));
						m_fStepEnds = t + 20;
					}
					else if (m_iLadder == 4)
					{
						// Out of sight (no player in the lab): the nearest navmesh point 15 m ahead
						vector ahead = pos + dir * 15;
						vector snapped;
						if (!m_aGroups.IsEmpty() && RZL_B.Snap(m_aGroups[0], ahead, 5, snapped))
							ahead = snapped;
						RZL.Teleport(m_B, ahead, RZL.YawTo(ahead, m_vGoal));
					}
					m_sSteps += string.Format("%1@%2s ", m_iLadder, RZL.F(t));
					m_aHist.Clear();
				}
			}
			if (m_fReached < 0 && t < 120 && m_iLadder < 5)
				return;

			RZL.Row("b9", string.Format("case=%1|wall=%2|wallHeight=%3|reachedGoalAfter=%4|ladderSteps=%5|endDistToGoal=%6", m_aCases[m_iCase], FilePath.StripPath(RZL_Walls.s_aPrefabs[m_iWall]), RZL.F2(RZL_Walls.s_aHeight[m_iWall]), RZL.F(m_fReached), m_sSteps, RZL.F(RZL.DistXZ(pos, m_vGoal))));
			Next(now);
			return;
		}

		if (m_iStep == 3)
		{
			RZL.DelAll(m_aChars, m_aGroups);
			ClearProps();
			if (m_aCases[m_iCase].Contains("rebuilt"))
				RZ_NavRebuild.RequestArea(m_Site.Side(55, 0), 25);
			m_iCase++;
			Next(now);
			return;
		}

		if (m_iStep == 4 && s >= 3)
		{
			m_iStep = 0;
			m_fStepAt = now;
		}
	}
}

//------------------------------------------------------------------------------------------------
//! Where B16 tests water: a river point with dry banks, and a pond's edge if the map has one.
class RZL_Water
{
	static bool s_bRiver;
	static vector s_vC;			// a point on the river's centre line
	static vector s_vAlong;		// the river's direction
	static float s_fEdge;		// metres from the centre to the dry bank
	static bool s_bPond;
	static vector s_vPondEdge;	// the first dry point
	static vector s_vPondOut;	// from the water outward

	//------------------------------------------------------------------------------------------------
	static float EdgeFrom(vector c, vector dir)
	{
		for (float d = 1; d <= 40; d += 1)
		{
			if (RZL_B.WaterDepth(c + dir * d) <= 0)
				return d;
		}
		return -1;
	}

	//------------------------------------------------------------------------------------------------
	static bool FarFrom(vector p, array<ref RZL_Site> used, float minDist)
	{
		foreach (RZL_Site s : used)
		{
			if (RZL.DistXZ(s.m_vT, p) < minDist)
				return false;
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static void Find(array<ref RZL_Site> used)
	{
		array<RiverEntity> all = {};
		RiverEntity.GetExistingInstances(all);
		int rivers;
		foreach (RiverEntity river : all)
		{
			if (!river || river.GetWorld() != GetGame().GetWorld())
				continue;

			rivers++;
			array<vector> pts = {};
			array<float> widths = {};
			if (!river.GetCentralPolyline(pts, widths, 5))
				continue;

			for (int i = 2; i < pts.Count() - 2 && !s_bRiver; i += 3)
			{
				if (widths[i] < 8 || widths[i] > 30)
					continue;

				vector c = RZL.Ground(pts[i]);
				if (RZL_B.WaterDepth(c) < 0.2 || !FarFrom(c, used, 900))
					continue;

				vector along = pts[i + 1] - pts[i - 1];
				along[1] = 0;
				along.Normalize();
				vector side = Vector(along[2], 0, -along[0]);
				float ea = EdgeFrom(c, side);
				float eb = EdgeFrom(c, side * -1);
				if (ea < 0 || eb < 0)
					continue;

				vector bankA = RZL.Ground(c + side * (ea + 12));
				vector bankB = RZL.Ground(c - side * (eb + 12));
				if (Math.AbsFloat(bankA[1] - bankB[1]) > 5 || RZL_B.WaterDepth(bankA) > 0 || RZL_B.WaterDepth(bankB) > 0)
					continue;

				s_bRiver = true;
				s_vC = c;
				s_vAlong = along;
				s_fEdge = Math.Max(ea, eb);
				RZL.Row("water", string.Format("what=river|x=%1|z=%2|width=%3|edgeA=%4|edgeB=%5|depthAtCentre=%6", RZL.F(c[0]), RZL.F(c[2]), RZL.F(widths[i]), ea, eb, RZL.F2(RZL_B.WaterDepth(c))));
			}
		}
		if (!s_bRiver)
			RZL.Row("water", string.Format("what=river|note=no river point with dry banks found (%1 rivers)", rivers));

		// A pond: grid samples, then out to the first dry point
		vector mins, maxs;
		GetGame().GetWorld().GetBoundBox(mins, maxs);
		for (float x = mins[0] + 500; x < maxs[0] - 500 && !s_bPond; x += 150)
		{
			for (float z = mins[2] + 500; z < maxs[2] - 500 && !s_bPond; z += 150)
			{
				vector q = RZL.Ground(Vector(x, 0, z));
				if (RZL_B.WaterType(q) != EWaterSurfaceType.WST_POND)
					continue;

				for (int k = 0; k < 8 && !s_bPond; k++)
				{
					vector dir = vector.FromYaw(k * 45);
					for (float d = 5; d <= 400; d += 5)
					{
						vector e = RZL.Ground(q + dir * d);
						if (RZL_B.WaterDepth(e) > 0)
							continue;

						vector inland = RZL.Ground(e + dir * 15);
						if (Math.AbsFloat(inland[1] - e[1]) < 4 && RZL_B.WaterDepth(inland) <= 0 && FarFrom(e, used, 900))
						{
							s_bPond = true;
							s_vPondEdge = e;
							s_vPondOut = dir;
							RZL.Row("water", string.Format("what=pond edge|x=%1|z=%2", RZL.F(e[0]), RZL.F(e[2])));
						}
						break;
					}
				}
			}
		}
		if (!s_bPond)
			RZL.Row("water", "what=pond edge|note=none found");
	}
}

//------------------------------------------------------------------------------------------------
//! B16. Slots across a river and at the water's edge (raw, as Bacon would walk), our checked slot
//! (refused across water, kept on the bank), and a pond's edge. Does any body enter the water?
class RZL_ExpB16 : RZL_BExp
{
	protected ref array<IEntity> m_aB = {};
	protected ref array<string> m_aKind = {};
	protected ref array<vector> m_aStart = {};
	protected ref array<vector> m_aGoal = {};
	protected ref array<float> m_aMaxDepth = {};
	protected ref array<float> m_aWet = {};
	protected ref array<float> m_aReached = {};
	protected float m_fT0;
	protected float m_fLast;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpB16()
	{
		m_sName = "b16";
	}

	//------------------------------------------------------------------------------------------------
	protected void AddBody(string kind, vector start, vector goal)
	{
		start = RZL.Ground(start);
		goal = RZL.Ground(goal);
		if (RZL_B.WaterDepth(start) > 0)
		{
			RZL.Row("b16", string.Format("body=%1|note=start point is wet, skipped", kind));
			return;
		}

		IEntity ch = RZL.SpawnSolo(RZL.CIV, start, RZL.YawTo(start, goal), m_aGroups);
		RZL_B.Steer(ch, goal);
		m_aB.Insert(ch);
		m_aChars.Insert(ch);
		m_aKind.Insert(kind);
		m_aStart.Insert(start);
		m_aGoal.Insert(goal);
		m_aMaxDepth.Insert(0);
		m_aWet.Insert(0);
		m_aReached.Insert(-1);
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		float s = Since(now);
		if (m_iStep == 0)
		{
			if (RZL_Water.s_bRiver)
			{
				vector c = RZL_Water.s_vC;
				vector al = RZL_Water.s_vAlong;
				vector side = Vector(al[2], 0, -al[0]);
				float e = RZL_Water.s_fEdge;
				AddBody("river, raw slot across", c + side * (e + 12) - al * 25, c - side * (e + 12) - al * 25);
				AddBody("river, raw slot across", c + side * (e + 12) + al * 25, c - side * (e + 12) + al * 25);
				vector edgeStart = c + side * (e + 14) + al * 50;
				AddBody("river, raw slot 1.5 m from the water", edgeStart, c + side * (RZL_Water.EdgeFrom(c + al * 50, side) + 1.5) + al * 50);
				// Our check: a slot across water is refused; the body is kept on its bank
				vector want = c - side * (e + 12) - al * 50;
				vector fromP = c + side * (e + 12) - al * 50;
				vector chosen = want;
				string kind = "river, checked slot (accepted)";
				if (RZL_B.CrossesWater(fromP, want))
				{
					chosen = fromP + al * 10;
					kind = "river, checked slot (refused across: kept on the bank)";
				}
				AddBody(kind, fromP, chosen);
			}
			if (RZL_Water.s_bPond)
			{
				vector pe = RZL_Water.s_vPondEdge;
				vector po = RZL_Water.s_vPondOut;
				AddBody("pond, raw slot 1.5 m from the water", pe + po * 20, pe + po * 1.5);
				AddBody("pond, raw slot 1.5 m from the water", pe + po * 20 + Vector(po[2], 0, -po[0]) * 15, pe + po * 1.5 + Vector(po[2], 0, -po[0]) * 15);
			}
			if (m_aB.IsEmpty())
			{
				RZL.Row("b16", "note=no water site found");
				Finish();
				return;
			}
			m_fT0 = now;
			m_fLast = now;
			Next(now);
			return;
		}

		if (m_iStep == 1)
		{
			float t = now - m_fT0;
			if (t > 1 && t < 1.3)
			{
				foreach (IEntity b0 : m_aB)
					RZL_B.Factor(b0, 0);
			}
			if (now - m_fLast >= 0.5)
			{
				m_fLast = now;
				for (int k = 0; k < m_aB.Count(); k++)
				{
					IEntity b = m_aB[k];
					if (!b)
						continue;

					float dep = RZL_B.WaterDepth(b.GetOrigin());
					m_aMaxDepth[k] = Math.Max(m_aMaxDepth[k], dep);
					if (dep > 0.2)
						m_aWet[k] = m_aWet[k] + 0.5;
					if (m_aReached[k] < 0 && RZL.DistXZ(b.GetOrigin(), m_aGoal[k]) < 3)
						m_aReached[k] = t;
				}
			}
			if (t < 90)
				return;

			for (int r = 0; r < m_aB.Count(); r++)
			{
				float endDist = -1;
				if (m_aB[r])
					endDist = RZL.DistXZ(m_aB[r].GetOrigin(), m_aGoal[r]);
				RZL.Row("b16", string.Format("body=%1|goalWet=%2|maxWaterDepthAtBody=%3|secondsInWaterOver20cm=%4|reachedGoalAfter=%5|endDistToGoal=%6|straightLineCrossesWater=%7", m_aKind[r], RZL.F2(RZL_B.WaterDepth(m_aGoal[r])), RZL.F2(m_aMaxDepth[r]), RZL.F(m_aWet[r]), RZL.F(m_aReached[r]), RZL.F(endDist), RZL_B.CrossesWater(m_aStart[r], m_aGoal[r])));
			}
			Finish();
		}
	}
}
