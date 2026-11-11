// Test-only (not part of the mod): behaviour tests at houses: B10 shut doors, B11 stairs, B19 seeing
// through walls, and B13 the cost of the sight checks (/root/rz_tools/behaviour_addendum.md, 5.5, 7).

//------------------------------------------------------------------------------------------------
//! A map building with doors, and points inside it.
class RZL_House
{
	IEntity m_Building;
	vector m_vMins;
	vector m_vMaxs;
	ref array<IEntity> m_aDoors = {};		// every door of the building
	ref array<IEntity> m_aOuter = {};		// its outside doors
	vector m_vInside;						// a floor point in the middle of the ground floor
	bool m_bUpper;
	vector m_vUpper;						// a floor point on an upper floor
	string m_sName;

	//------------------------------------------------------------------------------------------------
	bool IsInside(vector world, float margin)
	{
		vector l = RZ_DoorGeometry.ToLocal(m_Building, world);
		return l[0] > m_vMins[0] + margin && l[0] < m_vMaxs[0] - margin && l[2] > m_vMins[2] + margin && l[2] < m_vMaxs[2] - margin;
	}

	//------------------------------------------------------------------------------------------------
	//! From the building's middle out through an outside door (horizontal, unit).
	vector OutThrough(IEntity door)
	{
		vector d = door.GetOrigin() - m_vInside;
		d[1] = 0;
		d.Normalize();
		return d;
	}

	//------------------------------------------------------------------------------------------------
	void SetDoors(float value)
	{
		foreach (IEntity d : m_aDoors)
		{
			DoorComponent dc = RZ_DoorGeometry.GetDoor(d);
			if (dc)
				dc.SetControlValue(value, RplId.Invalid());
		}
	}

	//------------------------------------------------------------------------------------------------
	int OpenCount()
	{
		int n;
		foreach (IEntity d : m_aDoors)
		{
			if (RZ_DoorGeometry.IsOpen(d))
				n++;
		}
		return n;
	}
}

//------------------------------------------------------------------------------------------------
class RZL_BuildingCollector
{
	ref array<IEntity> m_aFound = {};

	//------------------------------------------------------------------------------------------------
	void Collect(vector c, float r)
	{
		m_aFound.Clear();
		GetGame().GetWorld().QueryEntitiesBySphere(c, r, OnEntity, null, EQueryEntitiesFlags.STATIC);
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnEntity(IEntity e)
	{
		if (SCR_DestructibleBuildingEntity.Cast(e))
			m_aFound.Insert(e);
		return true;
	}
}

//------------------------------------------------------------------------------------------------
class RZL_Houses
{
	static ref array<ref RZL_House> s_aHouses = {};

	//------------------------------------------------------------------------------------------------
	//! The floor under a point: a physics line down from `top` for `depth` metres. False if none.
	static bool FloorUnder(vector top, float depth, out vector floor)
	{
		floor = top;
		TraceParam tp = new TraceParam();
		tp.Start = top;
		tp.End = top - Vector(0, depth, 0);
		tp.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
		tp.LayerMask = EPhysicsLayerPresets.Projectile;
		float f = GetGame().GetWorld().TraceMove(tp, null);
		if (f >= 1)
			return false;

		floor = top - Vector(0, depth * f, 0);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Walls round a point: how many of 4 horizontal lines (12 m) hit something.
	static int WallsRound(vector p)
	{
		int n;
		for (int k = 0; k < 4; k++)
		{
			vector a = p + Vector(0, 1, 0);
			if (!RZL_B.TraceClear(a, a + vector.FromYaw(k * 90) * 12, null, null))
				n++;
		}
		return n;
	}

	//------------------------------------------------------------------------------------------------
	static RZL_House Make(IEntity b)
	{
		vector mins, maxs;
		if (!RZ_DoorGeometry.LocalBox(b, mins, maxs))
			return null;

		float sx = maxs[0] - mins[0];
		float sz = maxs[2] - mins[2];
		if (sx < 6 || sz < 6 || sx > 25 || sz > 25)
			return null;

		RZL_House h = new RZL_House();
		h.m_Building = b;
		h.m_vMins = mins;
		h.m_vMaxs = maxs;
		RZ_DoorGeometry.FindDoors(b, 0, h.m_aDoors);
		if (h.m_aDoors.IsEmpty())
			return null;

		float lowest = 100000;
		foreach (IEntity d : h.m_aDoors)
		{
			vector l = RZ_DoorGeometry.ToLocal(b, d.GetOrigin());
			bool edge = l[0] < mins[0] + RZ_DoorGeometry.EDGE || l[0] > maxs[0] - RZ_DoorGeometry.EDGE || l[2] < mins[2] + RZ_DoorGeometry.EDGE || l[2] > maxs[2] - RZ_DoorGeometry.EDGE;
			if (edge)
				h.m_aOuter.Insert(d);
			lowest = Math.Min(lowest, d.GetOrigin()[1]);
		}
		if (h.m_aOuter.IsEmpty())
			return null;

		vector centre = b.CoordToParent((mins + maxs) * 0.5);
		vector floor;
		if (!FloorUnder(Vector(centre[0], lowest + 1.5, centre[2]), 3, floor))
			return null;

		if (WallsRound(floor) < 3)
			return null;

		h.m_vInside = floor + Vector(0, 0.05, 0);
		// An upper floor: a door 2.3 m or more above the lowest, a floor 1.5 m in from it
		foreach (IEntity du : h.m_aDoors)
		{
			if (du.GetOrigin()[1] < lowest + 2.3)
				continue;

			vector toMid = h.m_vInside - du.GetOrigin();
			toMid[1] = 0;
			toMid.Normalize();
			vector up;
			for (int side = -1; side <= 1; side += 2)
			{
				vector cand = du.GetOrigin() + toMid * (1.5 * side);
				if (FloorUnder(cand + Vector(0, 1, 0), 2, up) && up[1] > lowest + 2)
				{
					h.m_bUpper = true;
					h.m_vUpper = up + Vector(0, 0.05, 0);
					break;
				}
			}
			if (h.m_bUpper)
				break;
		}
		return h;
	}

	//------------------------------------------------------------------------------------------------
	//! Up to `want` houses (and at least one with an upper floor if any), each 900 m from `used`.
	static void Find(array<ref RZL_Site> used, int want)
	{
		vector mins, maxs;
		GetGame().GetWorld().GetBoundBox(mins, maxs);
		int t0 = System.GetTickCount();
		bool haveUpper = false;
		int looked;
		RZL_BuildingCollector col = new RZL_BuildingCollector();
		for (float x = mins[0] + 600; x < maxs[0] - 600; x += 700)
		{
			for (float z = mins[2] + 600; z < maxs[2] - 600; z += 700)
			{
				if (s_aHouses.Count() >= want && haveUpper)
					break;

				vector c = RZL.Ground(Vector(x, 0, z));
				if (c[1] < 2 || !RZL_Water.FarFrom(c, used, 900))
					continue;

				bool near = false;
				foreach (RZL_House hh : s_aHouses)
				{
					if (RZL.DistXZ(hh.m_vInside, c) < 900)
						near = true;
				}
				if (near)
					continue;

				col.Collect(c, 250);
				foreach (IEntity b : col.m_aFound)
				{
					looked++;
					RZL_House h = Make(b);
					if (!h)
						continue;

					if (s_aHouses.Count() >= want && (!h.m_bUpper || haveUpper))
						continue;

					h.m_sName = "H" + s_aHouses.Count().ToString();
					s_aHouses.Insert(h);
					if (h.m_bUpper)
						haveUpper = true;
					RZL.Row("house", string.Format("name=%1|x=%2|z=%3|size=%4 x %5|doors=%6|outsideDoors=%7|upperFloor=%8|upperAbove=%9", h.m_sName, RZL.F(h.m_vInside[0]), RZL.F(h.m_vInside[2]), RZL.F(h.m_vMaxs[0] - h.m_vMins[0]), RZL.F(h.m_vMaxs[2] - h.m_vMins[2]), h.m_aDoors.Count(), h.m_aOuter.Count(), h.m_bUpper, RZL.F(h.m_vUpper[1] - h.m_vInside[1])));
					break;
				}
			}
		}
		RZL.Log(string.Format("houses: %1 found, %2 buildings looked at, %3 ms", s_aHouses.Count(), looked, System.GetTickCount() - t0));
	}

	//------------------------------------------------------------------------------------------------
	static RZL_House Upper()
	{
		foreach (RZL_House h : s_aHouses)
		{
			if (h.m_bUpper)
				return h;
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	static RZL_House Plain(RZL_House notThis)
	{
		foreach (RZL_House h : s_aHouses)
		{
			if (h != notThis)
				return h;
		}
		return null;
	}
}

//------------------------------------------------------------------------------------------------
//! Puts a character at an exact point (inside a building the ground is the wrong height).
class RZL_Place
{
	//------------------------------------------------------------------------------------------------
	static void Exact(IEntity e, vector pos, float yaw)
	{
		if (!e)
			return;

		vector mat[4];
		Math3D.AnglesToMatrix(Vector(yaw, 0, 0), mat);
		mat[3] = pos;
		BaseGameEntity bge = BaseGameEntity.Cast(e);
		if (bge)
		{
			bge.Teleport(mat);
			return;
		}

		e.SetWorldTransform(mat);
		e.Update();
	}
}

//------------------------------------------------------------------------------------------------
//! B19. A target inside a house, an infected outside at 10, 20 and 30 m on the door side and the far
//! side (factor 1, Bacon's own sight): does it see the target through the walls?
class RZL_ExpB19 : RZL_BExp
{
	RZL_House m_House;
	protected int m_iTrial;
	protected ref array<float> m_aD = {10, 20, 30, 10, 20, 30};
	protected IEntity m_Obs;
	protected IEntity m_Target;
	protected vector m_vObs;
	protected float m_fShown;
	protected float m_fSeen;
	protected float m_fEnemy;
	protected bool m_bTrace;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpB19(RZL_House h)
	{
		m_sName = "b19";
		m_House = h;
		if (RZL_LabComponent.s_bQuick)
		{
			m_aD.Clear();
			m_aD.Insert(10);
			m_aD.Insert(30);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		float s = Since(now);
		if (!m_House)
		{
			RZL.Row("b19", "note=no house found");
			Finish();
			return;
		}

		if (m_iStep == 0)
		{
			if (m_iTrial >= m_aD.Count())
			{
				Finish();
				return;
			}

			vector out1 = m_House.OutThrough(m_House.m_aOuter[0]);
			if (m_iTrial >= m_aD.Count() / 2)
				out1 = out1 * -1;
			m_vObs = RZL.Ground(m_House.m_vInside + out1 * m_aD[m_iTrial]);
			if (m_House.IsInside(m_vObs, -0.5) || RZL_B.WaterDepth(m_vObs) > 0)
			{
				RZL.Row("b19", string.Format("dist=%1|note=no spot outside on this side", m_aD[m_iTrial]));
				m_iTrial++;
				return;
			}

			m_Obs = RZL.SpawnSolo(RZL.CIV, m_vObs, RZL.YawTo(m_vObs, m_House.m_vInside), m_aGroups);
			m_aChars.Insert(m_Obs);
			m_Target = RZL_B.Target(m_Site.Side(150, 1500), 0, true);
			m_aChars.Insert(m_Target);
			Next(now);
			return;
		}

		if (m_iStep == 1 && s >= 3)
		{
			RZL_Place.Exact(m_Target, m_House.m_vInside, 0);
			m_fShown = now;
			m_fSeen = -1;
			m_fEnemy = -1;
			m_bTrace = RZL_B.TraceClear(m_vObs + Vector(0, 1.7, 0), m_House.m_vInside + Vector(0, 1.6, 0), m_Obs, m_Target);
			Next(now);
			return;
		}

		if (m_iStep == 2)
		{
			float t = now - m_fShown;
			if (!m_Obs || !m_Target)
			{
				Next(now);
				return;
			}

			if (t > 0.5 && m_fSeen < 0 && RZL.Seen(m_Obs, m_Target))
				m_fSeen = t;
			if (t > 0.5 && m_fEnemy < 0 && RZL.Category(m_Obs, m_Target) == ETargetCategory.ENEMY)
				m_fEnemy = t;
			if (t < 30)
				return;

			string side = "door side";
			if (m_iTrial >= m_aD.Count() / 2)
				side = "far side";
			RZL.Row("b19", string.Format("house=%1|side=%2|dist=%3|lineTraceClear=%4|seenAfter=%5|enemyAfter=%6|targetHeightAboveTerrain=%7|targetStillInside=%8", m_House.m_sName, side, m_aD[m_iTrial], m_bTrace, RZL.F(m_fSeen), RZL.F(m_fEnemy), RZL.F2(RZL_B.Height(m_Target)), m_House.IsInside(m_Target.GetOrigin(), 0)));
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
//! B10 (shut, unlocked doors, then open) and B11 (a target upstairs): the infected is told where the
//! target is (our share event) and comes from 20 m outside an outside door.
class RZL_ExpB10 : RZL_BExp
{
	RZL_House m_House;
	protected bool m_bStairs;
	protected IEntity m_Body;
	protected IEntity m_Target;
	protected vector m_vGoal;
	protected float m_fT0;
	protected int m_iPhase;
	protected float m_fMinDist;
	protected float m_fMaxHeight;
	protected bool m_bAtDoor;
	protected bool m_bInside;
	protected float m_fEntered;
	protected float m_fReached;
	protected int m_iMaxOpen;
	protected int m_iMelee;
	protected float m_fLastInject;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpB10(RZL_House h, bool stairs)
	{
		m_sName = "b10";
		if (stairs)
			m_sName = "b11";
		m_House = h;
		m_bStairs = stairs;
	}

	//------------------------------------------------------------------------------------------------
	protected void ResetPhase(float now)
	{
		m_fT0 = now;
		m_fMinDist = 1000;
		m_fMaxHeight = 0;
		m_bAtDoor = false;
		m_bInside = false;
		m_fEntered = -1;
		m_fReached = -1;
		m_iMaxOpen = 0;
		m_iMelee = 0;
		m_fLastInject = now;
	}

	//------------------------------------------------------------------------------------------------
	protected void Watch(float now)
	{
		if (!m_Body || !m_Target)
			return;

		float t = now - m_fT0;
		vector p = m_Body.GetOrigin();
		float d3 = vector.Distance(p, m_vGoal);
		m_fMinDist = Math.Min(m_fMinDist, d3);
		m_fMaxHeight = Math.Max(m_fMaxHeight, RZL_B.Height(m_Body));
		m_iMaxOpen = Math.Max(m_iMaxOpen, m_House.OpenCount());
		foreach (IEntity d : m_House.m_aOuter)
		{
			if (RZL.DistXZ(p, d.GetOrigin()) < 2)
				m_bAtDoor = true;
		}
		if (m_House.IsInside(p, 0.5))
		{
			m_bInside = true;
			if (m_fEntered < 0)
				m_fEntered = t;
		}
		if (m_fReached < 0 && d3 < 2.5)
			m_fReached = t;
		CharacterControllerComponent c = RZL_B.Ctrl(m_Body);
		if (c && c.IsMeleeAttack())
			m_iMelee++;
		if (now - m_fLastInject >= 8)
		{
			RZL_B.Inject(m_Body, m_Target);
			m_fLastInject = now;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		float s = Since(now);
		if (!m_House || (m_bStairs && !m_House.m_bUpper))
		{
			RZL.Row(m_sName, "note=no suitable house found");
			Finish();
			return;
		}

		if (m_iStep == 0)
		{
			m_vGoal = m_House.m_vInside;
			if (m_bStairs)
			{
				m_vGoal = m_House.m_vUpper;
				m_House.SetDoors(1);
			}
			else
			{
				m_House.SetDoors(0);
			}
			IEntity door = m_House.m_aOuter[0];
			vector start = RZL.Ground(door.GetOrigin() + m_House.OutThrough(door) * 20);
			m_Body = RZL.SpawnSolo(RZL.CIV, start, RZL.YawTo(start, door.GetOrigin()), m_aGroups);
			m_aChars.Insert(m_Body);
			m_Target = RZL_B.Target(m_Site.Side(150, 1500), 0, true);
			m_aChars.Insert(m_Target);
			Next(now);
			return;
		}

		if (m_iStep == 1 && s >= 3)
		{
			RZL_Place.Exact(m_Target, m_vGoal, 0);
			Next(now);
			return;
		}

		if (m_iStep == 2 && s >= 2)
		{
			// The target has to stay where it was put
			float drop = m_vGoal[1] - m_Target.GetOrigin()[1];
			if (drop > 1)
				RZL.Row(m_sName, string.Format("note=the target fell %1 m from where it was put", RZL.F(drop)));
			RZL_B.Inject(m_Body, m_Target);
			ResetPhase(now);
			m_iPhase = 0;
			Next(now);
			return;
		}

		if (m_iStep == 3)
		{
			Watch(now);
			float t = now - m_fT0;
			float limit = 45;
			if (m_bStairs)
				limit = 70;
			if (t < limit && m_fReached < 0)
				return;

			string what = "doors shut (unlocked)";
			if (m_iPhase == 1)
				what = "doors opened by script";
			if (m_bStairs)
				what = string.Format("target upstairs (%1 m up), doors open", RZL.F(m_vGoal[1] - m_House.m_vInside[1]));
			RZL.Row(m_sName, string.Format("house=%1|case=%2|doorsOpenAtMost=%3 of %4|reachedADoor=%5|wentInside=%6|insideAfter=%7|reachedTargetAfter=%8|closest3D=%9", m_House.m_sName, what, m_iMaxOpen, m_House.m_aDoors.Count(), m_bAtDoor, m_bInside, RZL.F(m_fEntered), RZL.F(m_fReached), RZL.F(m_fMinDist)) + string.Format("|maxHeightAboveTerrain=%1|meleeSamples=%2", RZL.F2(m_fMaxHeight), m_iMelee));
			if (!m_bStairs && m_iPhase == 0)
			{
				m_House.SetDoors(1);
				RZL_B.Inject(m_Body, m_Target);
				ResetPhase(now);
				m_iPhase = 1;
				return;
			}
			m_House.SetDoors(0);
			Finish();
		}
	}
}

//------------------------------------------------------------------------------------------------
//! B13. The cost of the sight checks (section 5.5): 1,000 terrain checks (a GetSurfaceY every 10 m
//! over 50-250 m) and 1,000 physics traces of 80 m among houses.
class RZL_ExpB13 : RZL_BExp
{
	RZL_House m_House;

	//------------------------------------------------------------------------------------------------
	void RZL_ExpB13(RZL_House h)
	{
		m_sName = "b13";
		m_House = h;
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		vector c = m_Site.m_vT;
		if (m_House)
			c = m_House.m_vInside;
		array<vector> a = {};
		array<vector> b = {};
		for (int i = 0; i < 1000; i++)
		{
			vector p = RZL.Ground(c + Vector(Math.RandomFloat(-150, 150), 0, Math.RandomFloat(-150, 150)));
			a.Insert(p);
			b.Insert(RZL.Ground(p + vector.FromYaw(Math.RandomFloat(0, 360)) * Math.RandomFloat(50, 250)));
		}
		int clear;
		int t0 = System.GetTickCount();
		for (int j = 0; j < 1000; j++)
		{
			if (RZL_B.TerrainClear(a[j], 1.7, b[j], 1.6, 10))
				clear++;
		}
		int msTerrain = System.GetTickCount() - t0;
		RZL.Row("b13", string.Format("check=terrain line (GetSurfaceY every 10 m, 50-250 m)|calls=1000|totalMs=%1|msEach=%2|clear=%3", msTerrain, RZL.F2(msTerrain / 1000.0), clear));

		array<vector> a2 = {};
		array<vector> b2 = {};
		for (int k = 0; k < 1000; k++)
		{
			vector p2 = RZL.Ground(c + Vector(Math.RandomFloat(-60, 60), 0, Math.RandomFloat(-60, 60)));
			a2.Insert(p2 + Vector(0, 1.7, 0));
			b2.Insert(RZL.Ground(p2 + vector.FromYaw(Math.RandomFloat(0, 360)) * 80) + Vector(0, 1.6, 0));
		}
		int clear2;
		int t1 = System.GetTickCount();
		for (int m = 0; m < 1000; m++)
		{
			if (RZL_B.TraceClear(a2[m], b2[m], null, null))
				clear2++;
		}
		int msTrace = System.GetTickCount() - t1;
		string where = "open site";
		if (m_House)
			where = "among houses (" + m_House.m_sName + ")";
		RZL.Row("b13", string.Format("check=physics trace 80 m, %1|calls=1000|totalMs=%2|msEach=%3|clear=%4", where, msTrace, RZL.F2(msTrace / 1000.0), clear2));
		Finish();
	}
}
