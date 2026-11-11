// Test-only (not part of the mod): finds test sites on the map at start, far apart.
// A site is a point T and an approach yaw: the line from T out to m_fLine metres along the yaw is
// land, terrain line of sight is clear from the far end to T, and no trees stand near the line.
// Open: no trees within OPEN_CLEAR of T either. Wood edge: T sits just inside a tree line (trees
// within EDGE_NEAR of T, a wood behind it), with the same open approach.
class RZL_Site
{
	string m_sName;
	vector m_vT;
	float m_fYaw;
	bool m_bWood;
	bool m_bWork;
	int m_iTreesNear;
	float m_fLine;

	//------------------------------------------------------------------------------------------------
	vector Along(float dist)
	{
		return RZL.At(m_vT, m_fYaw, dist);
	}

	//------------------------------------------------------------------------------------------------
	vector Side(float along, float lateral)
	{
		vector p = m_vT + vector.FromYaw(m_fYaw) * along + vector.FromYaw(m_fYaw + 90) * lateral;
		return RZL.Ground(p);
	}
}

class RZL_Sites
{
	static const float MIN_APART = 1400;
	static const float LINE_CLEAR = 8;
	static const int LINE_TREES_MAX = 2;
	static const float OPEN_CLEAR = 25;
	static const float EDGE_NEAR = 12;

	ref array<ref RZL_Site> m_aSites = {};
	protected ref RZL_TreeCounter m_Trees = new RZL_TreeCounter();
	protected int m_iTreeQueries;
	protected int m_iNoLand;
	protected int m_iNoSight;
	protected int m_iTrees;
	protected int m_iNoEdge;
	protected bool m_bRelaxed;
	protected float m_fLine = 300;

	//------------------------------------------------------------------------------------------------
	//! Finds up to wantOpen open sites and wantWood wood-edge sites for the perception trials (strict:
	//! line of sight over 300 m), then wantWork work sites for the rest (land, walkable, few trees).
	void Find(int wantOpen, int wantWood, int wantWork)
	{
		float t0 = System.GetTickCount();
		// Perception sites: the longest clear lines first (a standing man is noticed from 300 m by day)
		m_bRelaxed = false;
		array<float> lengths = {600, 450, 300};
		foreach (float len : lengths)
		{
			m_fLine = len;
			Scan(wantOpen - CountKind(false, false), wantWood - CountKind(true, false), 0);
		}
		m_bRelaxed = true;
		m_fLine = 300;
		Scan(0, 0, wantWork);
		RZL.Log(string.Format("sites: %1 tree queries, %2 ms; lines rejected: %3 water or steep, %4 no line of sight, %5 trees on the line, %6 no wood edge behind", m_iTreeQueries, System.GetTickCount() - t0, m_iNoLand, m_iNoSight, m_iTrees, m_iNoEdge));
		foreach (RZL_Site s : m_aSites)
			RZL.Row("site", string.Format("name=%1|x=%2|z=%3|yaw=%4|wood=%5|work=%6|treesNear=%7|h=%8|line=%9", s.m_sName, RZL.F(s.m_vT[0]), RZL.F(s.m_vT[2]), s.m_fYaw, s.m_bWood, s.m_bWork, s.m_iTreesNear, RZL.F(s.m_vT[1]), s.m_fLine));
	}

	//------------------------------------------------------------------------------------------------
	protected int CountKind(bool wood, bool work)
	{
		int n;
		foreach (RZL_Site s : m_aSites)
		{
			if (s.m_bWood == wood && s.m_bWork == work)
				n++;
		}
		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected void Scan(int wantOpen, int wantWood, int wantWork)
	{
		vector mins, maxs;
		GetGame().GetWorld().GetBoundBox(mins, maxs);
		float x0 = mins[0] + 800;
		float z0 = mins[2] + 800;
		float x1 = maxs[0] - 800;
		float z1 = maxs[2] - 800;
		float step = 230;
		int nx = (x1 - x0) / step;
		int nz = (z1 - z0) / step;
		int total = nx * nz;
		int open;
		int wood;
		int work;
		// A stride walk over the grid spreads the picks over the map
		int stride = 7919;
		for (int k = 0; k < total; k++)
		{
			if (open >= wantOpen && wood >= wantWood && work >= wantWork)
				break;

			int idx = (k * stride) % total;
			int ix = idx % nx;
			int iz = idx / nx;
			vector c = RZL.Ground(Vector(x0 + ix * step, 0, z0 + iz * step));
			if (c[1] < 3 || !FarFromAll(c))
				continue;

			for (int d = 0; d < 8; d++)
			{
				float yaw = d * 45;
				if (!LandAndSight(c, yaw))
					continue;

				if (!LineClear(c, yaw, 30))
					continue;

				if (work < wantWork)
				{
					Add(c, yaw, false, 0);
					m_aSites[m_aSites.Count() - 1].m_bWork = true;
					m_aSites[m_aSites.Count() - 1].m_sName = "K" + (m_aSites.Count() - 1).ToString();
					work++;
					break;
				}

				if (open < wantOpen && m_Trees.Count(c, OPEN_CLEAR, false) == 0)
				{
					m_iTreeQueries++;
					Add(c, yaw, false, 0);
					open++;
					break;
				}

				if (wood < wantWood)
				{
					vector edge;
					int near;
					if (FindEdgeBehind(c, yaw, edge, near) && LandAndSight(edge, yaw) && LineClear(edge, yaw, 25))
					{
						Add(edge, yaw, true, near);
						wood++;
						break;
					}
				}
			}
		}

		RZL.Log(string.Format("sites scan (relaxed=%1): %2 open, %3 wood edge, %4 work", m_bRelaxed, open, wood, work));
	}

	//------------------------------------------------------------------------------------------------
	protected void Add(vector t, float yaw, bool wood, int near)
	{
		RZL_Site s = new RZL_Site();
		s.m_vT = t;
		s.m_fYaw = yaw;
		s.m_bWood = wood;
		s.m_iTreesNear = near;
		s.m_fLine = m_fLine;
		if (wood)
			s.m_sName = "W" + m_aSites.Count().ToString();
		else
			s.m_sName = "O" + m_aSites.Count().ToString();
		m_aSites.Insert(s);
	}

	//------------------------------------------------------------------------------------------------
	protected bool FarFromAll(vector c)
	{
		foreach (RZL_Site s : m_aSites)
		{
			if (RZL.DistXZ(s.m_vT, c) < MIN_APART)
				return false;
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Land all along the line, gentle, and terrain line of sight from 300, 200, 100 m to T.
	protected bool LandAndSight(vector t, float yaw)
	{
		float hmin = 10000;
		float hmax = -10000;
		int steps = m_fLine / 10;
		for (int i = 0; i <= steps; i++)
		{
			vector p = RZL.At(t, yaw, i * 10);
			if (p[1] < 2)
			{
				m_iNoLand++;
				return false;
			}

			hmin = Math.Min(hmin, p[1]);
			hmax = Math.Max(hmax, p[1]);
		}

		float steep = 45 * m_fLine / 300;
		if (m_bRelaxed)
			steep = 70;
		if (hmax - hmin > steep)
		{
			m_iNoLand++;
			return false;
		}

		if (m_bRelaxed)
			return true;

		bool seen = true;
		for (float sd = m_fLine; sd >= 100; sd -= 100)
		{
			if (!Sight(t, yaw, sd))
			{
				seen = false;
				break;
			}
		}
		if (seen)
			return true;

		m_iNoSight++;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool Sight(vector t, float yaw, float dist)
	{
		vector eye = RZL.At(t, yaw, dist);
		float eyeY = eye[1] + 1.6;
		float tY = t[1] + 1.0;
		int n = dist / 10;
		for (int i = 1; i < n; i++)
		{
			float fi = i;
			float f = fi / n;
			vector p = RZL.At(t, yaw, dist * f);
			float lineY = tY + (eyeY - tY) * f;
			if (p[1] > lineY - 0.1)
				return false;
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! No trees within LINE_CLEAR of the line from 'from' metres out to LINE.
	protected bool LineClear(vector t, float yaw, float from)
	{
		int n;
		for (float d = from; d <= m_fLine; d += 15)
		{
			m_iTreeQueries++;
			n += m_Trees.Count(RZL.At(t, yaw, d), LINE_CLEAR, false);
			int most = LINE_TREES_MAX;
			if (m_bRelaxed)
				most = 15;
			if (n > most)
			{
				m_iTrees++;
				return false;
			}
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Walks back from an open point (against the approach) to the first spot with trees round it
	//! and a wood behind it.
	protected bool FindEdgeBehind(vector c, float yaw, out vector edge, out int near)
	{
		for (int back = 5; back <= 200; back += 5)
		{
			vector p = RZL.At(c, yaw + 180, back);
			if (p[1] < 2)
			{
				m_iNoEdge++;
				return false;
			}

			m_iTreeQueries++;
			int n = m_Trees.Count(p, EDGE_NEAR, false);
			if (n < 3)
				continue;

			// a wood behind: trees round a point 30 m further back
			m_iTreeQueries++;
			int behind = m_Trees.Count(RZL.At(p, yaw + 180, 30), 20, false);
			if (behind < 8)
			{
				m_iNoEdge++;
				return false;
			}

			edge = p;
			near = n;
			return true;
		}
		m_iNoEdge++;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A perception site (open or wood edge)
	RZL_Site Take(bool wood)
	{
		for (int i = 0; i < m_aSites.Count(); i++)
		{
			if (m_aSites[i].m_bWood == wood && !m_aSites[i].m_bWork)
			{
				RZL_Site s = m_aSites[i];
				m_aSites.Remove(i);
				return s;
			}
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! A work site, or a spare open perception site
	RZL_Site TakeWork()
	{
		for (int i = 0; i < m_aSites.Count(); i++)
		{
			if (m_aSites[i].m_bWork)
			{
				RZL_Site s = m_aSites[i];
				m_aSites.Remove(i);
				return s;
			}
		}
		return Take(false);
	}
}
