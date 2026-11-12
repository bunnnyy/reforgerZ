//! A site's wire line (docs/SITES.md, "Close it off"): the perimeter the solver traces round the
//! building and its yard (or round an open-ground camp), each side moved in or out a little to meet
//! the walls, fences, hedges, neighbouring buildings and water already on the site grid, which count
//! as closed. The faction's barrier fills the rest; the roads crossing it are its gates, kept open.
//! Stations every 0.5 m along the sides its doctrine closes say what closes each one. Data only:
//! RZ_SiteSolver lays the pieces. Server only.
class RZ_SiteWire
{
	static const float STATION = 0.5;
	static const float GATE_WIDTH = 5;		// a gate with no road through it: an opening this wide
	static const float GATE_MAX = 12;		// a road along the line further than this isn't a gate: its middle is, the rest stays open

	// What closes a station
	static const int ST_GAP = 0;			// nothing: an opening
	static const int ST_GROUND = 1;			// a building, a wall, a fence, a hedge, a tree, a car, water
	static const int ST_PIECE = 2;			// the faction's barrier
	static const int ST_GATE = 3;			// a gate: a road in, kept open, watched
	static const int ST_ROAD = 4;			// a road crossing (made a gate once the gates are chosen)

	protected static const float MIN_OFF = 4;	// a side never comes closer to the building than this
	protected static const float BANK_REACH = 3;	// a bank this far out of the line...
	protected static const float BANK_DROP = 1.5;	// ...this much lower closes it

	vector m_vCentre;		// the frame: the building's origin (or the plan's middle), on the ground
	vector m_vRight;		// its right and forward, unit, flat
	vector m_vFwd;
	vector m_vInMin;		// the building's box in the frame (x, z), or the plan's extent
	vector m_vInMax;
	ref array<float> m_aOff = {0, 0, 0, 0};	// each side's distance out from it: 0 = -z, 1 = +x, 2 = +z, 3 = -x
	float m_fX0;
	float m_fX1;
	float m_fZ0;
	float m_fZ1;
	int m_iClosing;			// the sides the doctrine closes (a bit each)
	int m_iGateFace = -1;
	vector m_vGate;			// the main gate's middle, on the line
	float m_fGateOut;		// the way out through it
	ref array<vector> m_aGates = {};	// every gate, the main one first
	ref array<vector> m_aAt = {};		// stations
	ref array<int> m_aState = {};
	ref array<int> m_aFace = {};
	ref array<bool> m_aHard = {};		// a gap nothing can fill (off the grid, other scenery)

	//------------------------------------------------------------------------------------------------
	//! The frame: centre and its flat right and forward, and the box round the building or the plan.
	void RZ_SiteWire(vector centre, vector right, vector fwd, vector inMin, vector inMax)
	{
		m_vCentre = centre;
		right[1] = 0;
		fwd[1] = 0;
		m_vRight = right.Normalized();
		m_vFwd = fwd.Normalized();
		m_vInMin = inMin;
		m_vInMax = inMax;
	}

	//------------------------------------------------------------------------------------------------
	//! A point x right and z forward in the frame, on the ground.
	vector Point(float x, float z)
	{
		vector pt = m_vCentre + m_vRight * x + m_vFwd * z;
		return RZ_Util.GroundPos(pt);
	}

	//------------------------------------------------------------------------------------------------
	//! A world point's (x, z) in the frame.
	vector Local(vector pos)
	{
		vector d = pos - m_vCentre;
		d[1] = 0;
		return Vector(vector.Dot(d, m_vRight), 0, vector.Dot(d, m_vFwd));
	}

	//------------------------------------------------------------------------------------------------
	//! The bearing out of side k (0 = -z, 1 = +x, 2 = +z, 3 = -x).
	float FaceOut(int k)
	{
		vector dir = -m_vFwd;
		if (k == 1)
			dir = m_vRight;
		else if (k == 2)
			dir = m_vFwd;
		else if (k == 3)
			dir = -m_vRight;

		return RZ_Compass.Bearing(vector.Zero, dir);
	}

	//------------------------------------------------------------------------------------------------
	//! Side k's two ends (corner k, then corner k+1), going round.
	void FaceEnds(int k, out float ax, out float az, out float bx, out float bz)
	{
		array<float> xs = {m_fX0, m_fX1, m_fX1, m_fX0};
		array<float> zs = {m_fZ0, m_fZ0, m_fZ1, m_fZ1};
		ax = xs[k];
		az = zs[k];
		bx = xs[(k + 1) % 4];
		bz = zs[(k + 1) % 4];
	}

	//------------------------------------------------------------------------------------------------
	protected void Bounds()
	{
		m_fZ0 = m_vInMin[2] - m_aOff[0];
		m_fX1 = m_vInMax[0] + m_aOff[1];
		m_fZ1 = m_vInMax[2] + m_aOff[2];
		m_fX0 = m_vInMin[0] - m_aOff[3];
	}

	//------------------------------------------------------------------------------------------------
	//! Inside the line, at least `margin` in from it.
	bool Inside(vector pos, float margin)
	{
		vector l = Local(pos);
		return l[0] > m_fX0 + margin && l[0] < m_fX1 - margin && l[2] > m_fZ0 + margin && l[2] < m_fZ1 - margin;
	}

	//------------------------------------------------------------------------------------------------
	//! How far a point is from the line (inside or out).
	float DistToLine(vector pos)
	{
		vector l = Local(pos);
		float dx = Math.Min(Math.AbsFloat(l[0] - m_fX0), Math.AbsFloat(l[0] - m_fX1));
		float dz = Math.Min(Math.AbsFloat(l[2] - m_fZ0), Math.AbsFloat(l[2] - m_fZ1));
		bool inX = l[0] >= m_fX0 && l[0] <= m_fX1;
		bool inZ = l[2] >= m_fZ0 && l[2] <= m_fZ1;
		if (inX && inZ)
			return Math.Min(dx, dz);

		if (inX)
			return dz;

		if (inZ)
			return dx;

		return Math.Sqrt(dx * dx + dz * dz);
	}

	//------------------------------------------------------------------------------------------------
	//! The point on the line nearest the ray from the middle of the box toward `bearing`, and the
	//! side it's on.
	vector LinePointToward(float bearing, out int face)
	{
		vector mid = (m_vInMin + m_vInMax) * 0.5;
		vector dir = RZ_Compass.PointAt(vector.Zero, bearing, 1);
		float dx = vector.Dot(dir, m_vRight);
		float dz = vector.Dot(dir, m_vFwd);
		float t = 10000;
		face = 0;
		if (dx > 0.001 && (m_fX1 - mid[0]) / dx < t)
		{
			t = (m_fX1 - mid[0]) / dx;
			face = 1;
		}

		if (dx < -0.001 && (m_fX0 - mid[0]) / dx < t)
		{
			t = (m_fX0 - mid[0]) / dx;
			face = 3;
		}

		if (dz > 0.001 && (m_fZ1 - mid[2]) / dz < t)
		{
			t = (m_fZ1 - mid[2]) / dz;
			face = 2;
		}

		if (dz < -0.001 && (m_fZ0 - mid[2]) / dz < t)
		{
			t = (m_fZ0 - mid[2]) / dz;
			face = 0;
		}

		return Point(mid[0] + dx * t, mid[2] + dz * t);
	}

	// ================================================================ Tracing
	//------------------------------------------------------------------------------------------------
	//! What closes the ground at a station (its cell and the cells 0.5 m either side of the line): a
	//! building, something standing, water, or a bank falling 1.5 m away outside it.
	static int GroundState(RZ_SiteGrid grid, vector at, vector across, out bool hard)
	{
		hard = false;
		int f = 0;
		for (int k = -1; k <= 1; k++)
		{
			int g = grid.FlagsAt(at + across * (k * RZ_SiteGrid.CELL));
			if (g == RZ_SiteGrid.OUTSIDE)
			{
				hard = true;
				return ST_GAP;
			}

			f = f | g;
		}

		if (f & (RZ_SiteGrid.WATER | RZ_SiteGrid.INSIDE | RZ_SiteGrid.SOLID | RZ_SiteGrid.TAKEN))
			return ST_GROUND;

		// A bank falling away outside the line: 1.5 m down within 3 m out
		vector beyond = at + across * BANK_REACH;
		if (grid.FlagsAt(beyond) != RZ_SiteGrid.OUTSIDE && grid.HeightAt(at[0], at[2]) - grid.HeightAt(beyond[0], beyond[2]) >= BANK_DROP)
			return ST_GROUND;

		if (f & RZ_SiteGrid.ROAD)
			return ST_ROAD;

		if (f & RZ_SiteGrid.KEPT)
			hard = true;

		return ST_GAP;
	}

	//------------------------------------------------------------------------------------------------
	//! How good side k is at `off` from the building: the share of its stations closed already or
	//! where a barrier can stand, less a little for every metre from where the doctrine wants it.
	protected float FaceScore(RZ_SiteGrid grid, int k, float off, float want)
	{
		float keep = m_aOff[k];
		m_aOff[k] = off;
		Bounds();
		float ax;
		float az;
		float bx;
		float bz;
		FaceEnds(k, ax, az, bx, bz);
		m_aOff[k] = keep;
		Bounds();

		float len = Math.Sqrt((bx - ax) * (bx - ax) + (bz - az) * (bz - az));
		int n = Math.Max(1, Math.Floor(len / 1.0));
		float good = 0;
		vector across = RZ_Compass.PointAt(vector.Zero, FaceOut(k), 1);
		for (int i = 0; i < n; i++)
		{
			float t = (i + 0.5) / n;
			vector at = m_vCentre + m_vRight * (ax + (bx - ax) * t) + m_vFwd * (az + (bz - az) * t);
			bool hard;
			int st = GroundState(grid, at, across, hard);
			if (hard)
				good -= 2;
			else if (st == ST_GROUND)
				good += 1.2;
			else if (st == ST_ROAD)
				good += 0.3;
			else if ((grid.FlagsAt(at) & (RZ_SiteGrid.STEP | RZ_SiteGrid.KEPT)) == 0)
				good += 1;
		}

		return 100 * good / n - 3 * Math.AbsFloat(off - want);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets each side's distance (want[k], or within -3..+6 m of it where more of the side is closed
	//! already or can be), then the stations along the sides in `closing` (a bit each).
	void Trace(RZ_SiteGrid grid, array<float> want, int closing)
	{
		m_iClosing = closing;
		for (int k = 0; k < 4; k++)
		{
			m_aOff[k] = Math.Max(MIN_OFF, want[k]);
		}

		Bounds();
		for (int f = 0; f < 4; f++)
		{
			float best = -float.MAX;
			float bestOff = m_aOff[f];
			for (int step = -3; step <= 6; step++)
			{
				float off = want[f] + step;
				if (off < MIN_OFF)
					continue;

				float score = FaceScore(grid, f, off, want[f]);
				if (score > best)
				{
					best = score;
					bestOff = off;
				}
			}

			m_aOff[f] = bestOff;
			Bounds();
		}

		m_aAt.Clear();
		m_aState.Clear();
		m_aFace.Clear();
		m_aHard.Clear();
		for (int face = 0; face < 4; face++)
		{
			if ((closing & (1 << face)) == 0)
				continue;

			float ax;
			float az;
			float bx;
			float bz;
			FaceEnds(face, ax, az, bx, bz);
			float len = Math.Sqrt((bx - ax) * (bx - ax) + (bz - az) * (bz - az));
			int n = Math.Max(1, Math.Floor(len / STATION));
			vector across = RZ_Compass.PointAt(vector.Zero, FaceOut(face), 1);
			for (int i = 0; i < n; i++)
			{
				float t = (i + 0.5) / n;
				vector at = m_vCentre + m_vRight * (ax + (bx - ax) * t) + m_vFwd * (az + (bz - az) * t);
				bool hard;
				int st = GroundState(grid, at, across, hard);
				m_aAt.Insert(RZ_Util.GroundPos(at));
				m_aState.Insert(st);
				m_aFace.Insert(face);
				m_aHard.Insert(hard);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The gates: every road crossing the line is a gate, kept open (a road running along the line
	//! is one only for GATE_MAX in its middle); the main one is one the entrance (`door`) can walk to
	//! inside the line (`reached`), the nearest it and toward `mainAt` (where the road in from the
	//! next town meets the site). With no road through the line, an opening GATE_WIDTH wide on the
	//! side toward the road in, where nothing stands.
	void ChooseGates(vector mainAt, vector door, RZ_SiteGrid grid, array<int> reached)
	{
		m_aGates.Clear();
		m_iGateFace = -1;
		float mainBearing = RZ_Compass.Bearing(m_vCentre, mainAt);
		float best = float.MAX;
		bool bestReached = false;
		int i = 0;
		while (i < m_aAt.Count())
		{
			if (m_aState[i] != ST_ROAD)
			{
				i++;
				continue;
			}

			int j = i;
			while (j + 1 < m_aAt.Count() && m_aState[j + 1] == ST_ROAD && m_aFace[j + 1] == m_aFace[i])
			{
				j++;
			}

			// The road and a step either side of it are the gate; a long run along the line, its middle
			int lo = Math.Max(0, i - 2);
			int hi = Math.Min(m_aAt.Count() - 1, j + 2);
			int half = RZ_Util.RoundToInt(GATE_MAX * 0.5 / STATION);
			if (hi - lo > 2 * half)
			{
				int middle = (i + j) / 2;
				lo = middle - half;
				hi = middle + half;
			}

			for (int k = lo; k <= hi; k++)
			{
				if (m_aFace[k] == m_aFace[i] && m_aState[k] != ST_GROUND)
					m_aState[k] = ST_GATE;
			}

			vector mid = (m_aAt[lo] + m_aAt[hi]) * 0.5;
			m_aGates.Insert(mid);
			// The main gate: one the entrance can walk to, near the entrance and toward the road in
			// from the next town
			bool reach = grid.Reached(reached, mid);
			float d = RZ_Util.DistXZ(mid, door) + 0.5 * RZ_Util.DistXZ(mid, mainAt);
			if ((reach && !bestReached) || (reach == bestReached && d < best))
			{
				best = d;
				bestReached = reach;
				m_vGate = mid;
				m_iGateFace = m_aFace[i];
			}

			i = j + 1;
		}

		if (m_iGateFace >= 0 && (bestReached || !HasReachable(grid, reached)))
		{
			m_aGates.RemoveItem(m_vGate);
			m_aGates.InsertAt(m_vGate, 0);
			m_fGateOut = FaceOut(m_iGateFace);
			return;
		}

		// No road through (or none the entrance can walk to): an opening toward the road in, on the
		// open station nearest it that the entrance can walk to
		int face;
		vector toward = LinePointToward(mainBearing, face);
		int pick = -1;
		float near = float.MAX;
		foreach (int s, vector at : m_aAt)
		{
			if (m_aState[s] != ST_GAP || m_aHard[s] || !grid.Reached(reached, at))
				continue;

			float ds = RZ_Util.DistXZ(at, toward);
			if (ds < near)
			{
				near = ds;
				pick = s;
			}
		}

		if (pick < 0)
		{
			// Nothing open on a closing side: the gate is where the road would come in (it stays closed ground)
			m_vGate = toward;
			m_iGateFace = face;
			m_fGateOut = FaceOut(face);
			m_aGates.Insert(m_vGate);
			return;
		}

		m_vGate = m_aAt[pick];
		m_iGateFace = m_aFace[pick];
		m_fGateOut = FaceOut(m_iGateFace);
		m_aGates.Insert(m_vGate);
		foreach (int g, vector pt : m_aAt)
		{
			if (m_aFace[g] == m_iGateFace && m_aState[g] == ST_GAP && RZ_Util.DistXZ(pt, m_vGate) <= GATE_WIDTH * 0.5)
				m_aState[g] = ST_GATE;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Keeps only the stations on the sides in `closing` (a partial doctrine: the road's side...).
	void KeepSides(int closing)
	{
		m_iClosing = closing;
		for (int i = m_aAt.Count() - 1; i >= 0; i--)
		{
			if (closing & (1 << m_aFace[i]))
				continue;

			m_aAt.RemoveOrdered(i);
			m_aState.RemoveOrdered(i);
			m_aFace.RemoveOrdered(i);
			m_aHard.RemoveOrdered(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Some open station on the line can be walked to from the entrance.
	protected bool HasReachable(RZ_SiteGrid grid, array<int> reached)
	{
		foreach (int s, vector at : m_aAt)
		{
			if ((m_aState[s] == ST_GAP || m_aState[s] == ST_ROAD) && grid.Reached(reached, at))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Marks the stations a placed barrier covers on side `face`: those within `half` of `mid` along it.
	void Cover(int face, vector mid, float half)
	{
		foreach (int i, vector at : m_aAt)
		{
			if (m_aFace[i] == face && m_aState[i] == ST_GAP && RZ_Util.DistXZ(at, mid) <= half + 0.05)
				m_aState[i] = ST_PIECE;
		}
	}

	// ================================================================ The share closed
	//------------------------------------------------------------------------------------------------
	//! Percent of the line closed (by the ground, a barrier or a gate). 100 with no line.
	int Share()
	{
		if (m_aAt.IsEmpty())
			return 100;

		int closed = 0;
		foreach (int st : m_aState)
		{
			if (st != ST_GAP && st != ST_ROAD)
				closed++;
		}

		float share = closed;
		return RZ_Util.RoundToInt(100 * share / m_aAt.Count());
	}

	//------------------------------------------------------------------------------------------------
	//! The widest gap: its width (m) and the bearing to it from the middle. False if none.
	bool WidestGap(out float width, out float bearing)
	{
		width = 0;
		bearing = 0;
		int i = 0;
		while (i < m_aAt.Count())
		{
			if (m_aState[i] != ST_GAP)
			{
				i++;
				continue;
			}

			int j = i;
			while (j + 1 < m_aAt.Count() && m_aState[j + 1] == ST_GAP && m_aFace[j + 1] == m_aFace[i])
			{
				j++;
			}

			float w = (j - i + 1) * STATION;
			if (w > width)
			{
				width = w;
				bearing = RZ_Compass.Bearing(m_vCentre, (m_aAt[i] + m_aAt[j]) * 0.5);
			}

			i = j + 1;
		}

		return width > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! "Sites draw" lines: the line as runs of one state ("wire <state> x1 z1 x2 z2").
	void DrawLines(string head)
	{
		int i = 0;
		while (i < m_aAt.Count())
		{
			int j = i;
			while (j + 1 < m_aAt.Count() && m_aState[j + 1] == m_aState[i] && m_aFace[j + 1] == m_aFace[i])
			{
				j++;
			}

			vector a = m_aAt[i];
			vector b = m_aAt[j];
			Print(head + string.Format("wire %1 %2 %3 %4 %5", m_aState[i], Math.Round(a[0] * 10) / 10, Math.Round(a[2] * 10) / 10, Math.Round(b[0] * 10) / 10, Math.Round(b[2] * 10) / 10));
			i = j + 1;
		}

		string ring = "";
		for (int k = 0; k < 4; k++)
		{
			float ax;
			float az;
			float bx;
			float bz;
			FaceEnds(k, ax, az, bx, bz);
			vector c = Point(ax, az);
			ring += string.Format(" %1 %2", Math.Round(c[0] * 10) / 10, Math.Round(c[2] * 10) / 10);
		}

		Print(head + "ring" + ring);
	}
}
