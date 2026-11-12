//! The mainland (docs/PLACES.md, "The mainland"): the map's biggest piece of land, connected by land
//! or a road (a bridge counts). Islands are close to impossible to reach on foot, so nothing is put on
//! them: no town is a settlement there on a fresh scan, and every place need checks it (MAINLAND).
//! Worked out once at the start from the terrain's heights and the road network, on squares of about
//! 100 m: a square is land if any of five samples is above sea level, or a road runs through it. A
//! flood fill over land squares (sides, not corners) gives each piece; the biggest is the mainland.
//! Works on any map; without terrain (no world) everything counts as mainland. Server only.
class RZ_Mainland
{
	protected static const float SEA_LEVEL = 0.05;		// above the sea: a beach or a spit you can walk across counts
	protected static const float MIN_CELL = 100;
	protected static const int MAX_SIDE = 320;			// squares a side at most (a bigger map gets bigger squares)
	protected static const float SAMPLE_OFF = 35;		// the four samples round each square's middle

	protected static ref RZ_Mainland s_Instance;

	protected vector m_vLow;
	protected float m_fCell = MIN_CELL;
	protected int m_iNX;
	protected int m_iNZ;
	protected ref array<int> m_aPiece = {};		// per square: -1 sea, else its piece
	protected ref array<int> m_aSize = {};		// squares per piece
	protected int m_iMain = -1;
	protected int m_iBuildMs;

	//------------------------------------------------------------------------------------------------
	static RZ_Mainland Get()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//! On the mainland, or its shore (a sea square beside it). True while there's no mainland worked
	//! out (no world, or before the start).
	static bool OnMainland(vector pos)
	{
		if (!s_Instance)
			return true;

		return s_Instance.IsMainland(pos);
	}

	//------------------------------------------------------------------------------------------------
	//! Reads the map once. Settlements on an island are named in the log.
	void Build()
	{
		s_Instance = this;
		float start = System.GetTickCount();
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		vector mins;
		vector maxs;
		world.GetBoundBox(mins, maxs);
		float side = Math.Max(maxs[0] - mins[0], maxs[2] - mins[2]);
		m_fCell = Math.Max(MIN_CELL, side / MAX_SIDE);
		m_vLow = mins;
		m_iNX = Math.Max(1, Math.Ceil((maxs[0] - mins[0]) / m_fCell));
		m_iNZ = Math.Max(1, Math.Ceil((maxs[2] - mins[2]) / m_fCell));
		int count = m_iNX * m_iNZ;
		array<bool> land = {};
		land.Resize(count);
		for (int z = 0; z < m_iNZ; z++)
		{
			for (int x = 0; x < m_iNX; x++)
			{
				land[z * m_iNX + x] = SquareIsLand(world, x, z);
			}
		}

		Roads(mins, maxs, land);
		Fill(land);
		m_iBuildMs = System.GetTickCount() - start;
	}

	//------------------------------------------------------------------------------------------------
	protected bool SquareIsLand(BaseWorld world, int x, int z)
	{
		float cx = m_vLow[0] + (x + 0.5) * m_fCell;
		float cz = m_vLow[2] + (z + 0.5) * m_fCell;
		if (world.GetSurfaceY(cx, cz) >= SEA_LEVEL)
			return true;

		if (world.GetSurfaceY(cx + SAMPLE_OFF, cz) >= SEA_LEVEL || world.GetSurfaceY(cx - SAMPLE_OFF, cz) >= SEA_LEVEL)
			return true;

		return world.GetSurfaceY(cx, cz + SAMPLE_OFF) >= SEA_LEVEL || world.GetSurfaceY(cx, cz - SAMPLE_OFF) >= SEA_LEVEL;
	}

	//------------------------------------------------------------------------------------------------
	//! A road's squares are land: a bridge joins what it crosses.
	protected void Roads(vector mins, vector maxs, notnull array<bool> land)
	{
		RoadNetworkManager roads = RZ_Perimeter.GetRoadNetwork();
		if (!roads)
			return;

		array<BaseRoad> found = {};
		roads.GetRoadsInAABB(mins, maxs, found);
		foreach (BaseRoad road : found)
		{
			if (!road)
				continue;

			array<vector> points = {};
			road.GetPoints(points);
			for (int i = 0; i < points.Count(); i++)
			{
				int idx = Index(points[i]);
				if (idx >= 0)
					land[idx] = true;

				// Points along a long straight span can be far apart: the squares between count too
				if (i + 1 < points.Count())
				{
					float span = RZ_Util.DistXZ(points[i], points[i + 1]);
					int steps = Math.Floor(span / (m_fCell * 0.5));
					for (int k = 1; k <= steps; k++)
					{
						int mid = Index(points[i] + (points[i + 1] - points[i]) * (k / (steps + 1.0)));
						if (mid >= 0)
							land[mid] = true;
					}
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Each piece of land, by a flood fill over side neighbours; the biggest is the mainland.
	protected void Fill(notnull array<bool> land)
	{
		int count = m_iNX * m_iNZ;
		m_aPiece.Clear();
		m_aPiece.Resize(count);
		for (int i = 0; i < count; i++)
		{
			m_aPiece[i] = -1;
		}

		m_aSize.Clear();
		array<int> stack = {};
		for (int s = 0; s < count; s++)
		{
			if (!land[s] || m_aPiece[s] >= 0)
				continue;

			int piece = m_aSize.Count();
			int size = 0;
			m_aPiece[s] = piece;
			stack.Insert(s);
			while (!stack.IsEmpty())
			{
				int at = stack[stack.Count() - 1];
				stack.Remove(stack.Count() - 1);
				size++;
				int ax = at % m_iNX;
				int az = at / m_iNX;
				Push(land, ax - 1, az, piece, stack);
				Push(land, ax + 1, az, piece, stack);
				Push(land, ax, az - 1, piece, stack);
				Push(land, ax, az + 1, piece, stack);
			}

			m_aSize.Insert(size);
		}

		int best = 0;
		for (int p = 0; p < m_aSize.Count(); p++)
		{
			if (m_aSize[p] > best)
			{
				best = m_aSize[p];
				m_iMain = p;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Push(array<bool> land, int x, int z, int piece, array<int> stack)
	{
		if (x < 0 || z < 0 || x >= m_iNX || z >= m_iNZ)
			return;

		int idx = z * m_iNX + x;
		if (!land[idx] || m_aPiece[idx] >= 0)
			return;

		m_aPiece[idx] = piece;
		stack.Insert(idx);
	}

	//------------------------------------------------------------------------------------------------
	protected int Index(vector pos)
	{
		int x = Math.Floor((pos[0] - m_vLow[0]) / m_fCell);
		int z = Math.Floor((pos[2] - m_vLow[2]) / m_fCell);
		if (x < 0 || z < 0 || x >= m_iNX || z >= m_iNZ)
			return -1;

		return z * m_iNX + x;
	}

	//------------------------------------------------------------------------------------------------
	//! The piece of land at pos (a sea square: the piece of a side or corner neighbour, the mainland
	//! first); -1 for open sea or off the map.
	int PieceAt(vector pos)
	{
		int idx = Index(pos);
		if (idx < 0)
			return -1;

		if (m_aPiece[idx] >= 0)
			return m_aPiece[idx];

		int x = idx % m_iNX;
		int z = idx / m_iNX;
		int other = -1;
		for (int dz = -1; dz <= 1; dz++)
		{
			for (int dx = -1; dx <= 1; dx++)
			{
				int nx = x + dx;
				int nz = z + dz;
				if (nx < 0 || nz < 0 || nx >= m_iNX || nz >= m_iNZ)
					continue;

				int piece = m_aPiece[nz * m_iNX + nx];
				if (piece == m_iMain)
					return piece;

				if (piece >= 0)
					other = piece;
			}
		}

		return other;
	}

	//------------------------------------------------------------------------------------------------
	bool IsMainland(vector pos)
	{
		if (m_iMain < 0)
			return true;

		return PieceAt(pos) == m_iMain;
	}

	//------------------------------------------------------------------------------------------------
	//! Islands: pieces other than the mainland.
	int IslandCount()
	{
		if (m_iMain < 0)
			return 0;

		return m_aSize.Count() - 1;
	}

	//------------------------------------------------------------------------------------------------
	//! "mainland 140.2 km2 (12 ms); 3 islands: 0.8 km2 (Tyrone), 0.1 km2, ...", the islands biggest
	//! first, each with the towns whose middle is on it (at most 12 listed).
	string Describe(array<ref RZ_Settlement> towns)
	{
		if (m_iMain < 0)
			return "no land read";

		float km2 = m_fCell * m_fCell / 1000000;
		string text = string.Format("mainland %1 km2 (%2 m squares, %3 ms); %4 islands", Km(m_aSize[m_iMain] * km2), RZ_Util.RoundToInt(m_fCell), m_iBuildMs, IslandCount());
		array<int> order = {};
		for (int p = 0; p < m_aSize.Count(); p++)
		{
			if (p == m_iMain)
				continue;

			int at = 0;
			while (at < order.Count() && m_aSize[order[at]] >= m_aSize[p])
			{
				at++;
			}

			order.InsertAt(p, at);
		}

		for (int i = 0; i < order.Count() && i < 12; i++)
		{
			string names = "";
			foreach (RZ_Settlement s : towns)
			{
				if (PieceAt(s.m_vCenter) == order[i])
				{
					if (!names.IsEmpty())
						names += ", ";

					names += s.m_sName;
				}
			}

			string one = Km(m_aSize[order[i]] * km2) + " km2";
			if (!names.IsEmpty())
				one += " (" + names + ")";

			if (i == 0)
				text += ": " + one;
			else
				text += ", " + one;
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Km(float v)
	{
		int tenths = RZ_Util.RoundToInt(v * 10);
		return string.Format("%1.%2", tenths / 10, tenths % 10);
	}
}
