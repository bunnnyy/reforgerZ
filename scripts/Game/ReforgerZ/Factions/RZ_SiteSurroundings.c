//! What lies round a site (docs/SITES.md, "The surroundings"): its roads, the sea, high ground, open
//! and covered sides, hostile and friendly bases, the infected and the town's history, read once from
//! data the game already has, and the threat each of its eight sides carries. Kept as a small
//! record on the site so later rules can ask "where is the threat?" without a query. Server only.

//------------------------------------------------------------------------------------------------
class RZ_SiteSurroundings
{
	static const int SECTORS = 8;

	protected static const float SEA_REACH = 80;		// a side with water this near is the sea side
	protected static const float BASE_REACH = 3000;	// hostile and friendly bases this near count
	protected static const float WILD_REACH = 2500;	// a lair or wild land this near counts
	protected static const float RIDGE_REACH = 200;
	protected static const float RIDGE_RISE = 10;		// ground this much above the yard overlooks it
	protected static const float CLIFF_DROP = 8;		// ground this much lower within 30 m is a drop
	protected static const float OPEN_COVER = 0.08;	// a side with less cover than this is open
	protected static const float COVERED = 0.25;		// and with more, covered
	protected static const int HISTORY_DAYS = 3;

	vector m_vMiddle;
	float m_fYardY;

	// Roads
	float m_fMainRoad = -1;			// the bearing of the main road in (-1 none)
	ref array<float> m_aEntries = {};	// each road entry's bearing
	int m_iThrough;					// roads that pass and leave again

	// Water and high ground
	int m_iSeaMask;					// bit per 22.5 degrees with water within 80 m
	int m_iCliffMask;				// bit per 22.5 degrees with a drop within 30 m
	bool m_bJetty;
	float m_fJetty;
	vector m_vHigh;					// the highest ground on the site's grid
	float m_fHighRise;				// how far above the yard
	float m_fRidge = -1;			// the bearing of the ground overlooking the site (-1 none)
	float m_fRidgeRise;

	// Cover: the share of each side's outer ground that is buildings, trees, walls and bushes
	ref array<float> m_aCover = {};

	// Bases
	float m_fHostile = -1;			// bearing to the nearest hostile holding or road post (-1 none)
	float m_fHostileDist;
	int m_iHostileFaction = -1;
	bool m_bFeud;
	float m_fFriendly = -1;
	float m_fFriendlyDist;

	// The infected
	float m_fWild = -1;				// bearing to the nearest lair candidate (else wild land)
	float m_fWildDist;
	bool m_bLair;
	int m_iInfection = -1;			// the town's infection, 0-100 (-1 no town)

	// History
	int m_iRecent;					// the town's events in the last three days that hurt it
	bool m_bTaken;					// the town changed hands at some point

	// Threat
	ref array<float> m_aThreat = {};
	int m_iFront;

	//------------------------------------------------------------------------------------------------
	static int SectorOf(float bearing)
	{
		int s = Math.Floor(RZ_Compass.Normalize(bearing + 22.5) / 45);
		return s % SECTORS;
	}

	//------------------------------------------------------------------------------------------------
	static string SectorName(int s)
	{
		array<string> names = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
		return names[s % SECTORS];
	}

	//------------------------------------------------------------------------------------------------
	//! Reads everything for a site whose middle is `middle` and yard `yard`, of faction f.
	void Read(RZ_GameModeComponent gm, RZ_EFaction f, vector middle, vector yard, RZ_SiteGrid grid, array<float> entries, float mainRoad, int through)
	{
		m_vMiddle = middle;
		m_fYardY = RZ_Util.GroundPos(yard)[1];
		m_fMainRoad = mainRoad;
		m_iThrough = through;
		if (entries)
			m_aEntries.InsertAll(entries);

		ReadWater();
		ReadHigh(grid);
		ReadCover(grid);
		ReadBases(gm, f);
		ReadInfected(gm);
		ReadHistory(gm);
		WorkOutThreat();
	}

	//------------------------------------------------------------------------------------------------
	//! Each of sixteen ways: water within 80 m (the sea, a river, a lake) and a drop within 30 m.
	protected void ReadWater()
	{
		BaseWorld world = GetGame().GetWorld();
		float startY = world.GetSurfaceY(m_vMiddle[0], m_vMiddle[2]);
		for (int k = 0; k < 16; k++)
		{
			float bearing = k * 22.5;
			for (int step = 1; step * 10 <= SEA_REACH; step++)
			{
				vector pt = RZ_Compass.PointAt(m_vMiddle, bearing, step * 10);
				bool wet = RZ_Util.IsWater(pt);
				if (!wet && step % 2 == 0)
				{
					float reach;
					EWaterSurfaceType type = RZ_Land.WaterAt(pt, reach);
					wet = type == EWaterSurfaceType.WST_POND || type == EWaterSurfaceType.WST_RIVER;
				}

				if (wet)
				{
					m_iSeaMask = m_iSeaMask | (1 << k);
					break;
				}

				if (step <= 3 && startY - pt[1] > CLIFF_DROP)
					m_iCliffMask = m_iCliffMask | (1 << k);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The highest ground on the grid, and ground 10 m above the yard within 200 m (a ridge).
	protected void ReadHigh(RZ_SiteGrid grid)
	{
		m_vHigh = m_vMiddle;
		m_fHighRise = 0;
		if (grid)
		{
			for (int lz = 0; lz < grid.m_iL; lz += 2)
			{
				for (int lx = 0; lx < grid.m_iL; lx += 2)
				{
					float h = grid.m_aHeight[lz * grid.m_iL + lx];
					if (h - m_fYardY > m_fHighRise)
					{
						m_fHighRise = h - m_fYardY;
						m_vHigh = Vector(grid.m_vLow[0] + lx, h, grid.m_vLow[2] + lz);
					}
				}
			}

			if (grid.m_aJetty.Count() > 0)
			{
				m_bJetty = true;
				m_fJetty = RZ_Compass.Bearing(m_vMiddle, grid.m_aJetty[0]);
			}
		}

		for (int k = 0; k < 16; k++)
		{
			for (int ring = 1; ring <= 4; ring++)
			{
				vector pt = RZ_Compass.PointAt(m_vMiddle, k * 22.5, ring * RIDGE_REACH / 4);
				float rise = pt[1] - m_fYardY;
				if (rise >= RIDGE_RISE && rise > m_fRidgeRise)
				{
					m_fRidgeRise = rise;
					m_fRidge = k * 22.5;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Each side's cover: buildings, walls, trees and bushes on the grid's outer ring.
	protected void ReadCover(RZ_SiteGrid grid)
	{
		array<int> covered = {};
		array<int> all = {};
		for (int s = 0; s < SECTORS; s++)
		{
			covered.Insert(0);
			all.Insert(0);
			m_aCover.Insert(0);
		}

		if (!grid)
			return;

		float inner = grid.m_fHalf * 0.4;
		for (int cz = 0; cz < grid.m_iN; cz += 2)
		{
			for (int cx = 0; cx < grid.m_iN; cx += 2)
			{
				vector c = Vector(grid.CellCentreX(cx), 0, grid.CellCentreZ(cz));
				float d = RZ_Util.DistXZ(c, m_vMiddle);
				if (d < inner || d > grid.m_fHalf)
					continue;

				int sector = SectorOf(RZ_Compass.Bearing(m_vMiddle, c));
				all[sector] = all[sector] + 1;
				if (grid.Flags(cx, cz) & (RZ_SiteGrid.SOLID | RZ_SiteGrid.SOFT | RZ_SiteGrid.INSIDE))
					covered[sector] = covered[sector] + 1;
			}
		}

		for (int i = 0; i < SECTORS; i++)
		{
			float share = covered[i];
			if (all[i] > 0)
				m_aCover[i] = share / all[i];
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Two factions' people shoot at each other: a feud, else not under a truce and enemies by the
	//! faction manager's relations, else rivals by Fame.
	static bool Hostile(RZ_GameModeComponent gm, int a, int b)
	{
		if (a < 0 || b < 0 || a == b)
			return false;

		RZ_FactionLife life;
		if (gm)
			life = gm.GetFactionLife();

		if (life && life.HasFeud(a, b))
			return true;

		if (life && life.HasTruce(a, b))
			return false;

		FactionManager fm = GetGame().GetFactionManager();
		if (fm)
		{
			Faction fa = fm.GetFactionByKey(RZ_Factions.GetAIKey(a));
			Faction fb = fm.GetFactionByKey(RZ_Factions.GetAIKey(b));
			if (fa && fb && fa != fb)
				return fa.IsFactionEnemy(fb);
		}

		return RZ_Reputation.GetRivalShare(a, b) > 0 || RZ_Reputation.GetRivalShare(b, a) > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest hostile and friendly holding, road post or Haven safehouse within 3 km.
	protected void ReadBases(RZ_GameModeComponent gm, RZ_EFaction f)
	{
		if (!gm)
			return;

		array<vector> spots = {};
		array<int> owners = {};
		RZ_Territory territory = gm.GetTerritory();
		if (territory)
		{
			foreach (RZ_Holding h : territory.GetHoldings())
			{
				spots.Insert(h.m_vPos);
				owners.Insert(h.m_eFaction);
			}
		}

		RZ_Checkpoints posts = gm.GetCheckpoints();
		if (posts)
		{
			array<RZ_RoadPost> near = {};
			posts.PostsNear(-1, m_vMiddle, BASE_REACH, near);
			foreach (RZ_RoadPost post : near)
			{
				spots.Insert(post.m_vPos);
				owners.Insert(post.m_eFaction);
			}
		}

		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses)
		{
			for (int i = 0; i < houses.Count(); i++)
			{
				RZ_Safehouse house = houses.Get(i);
				if (house)
				{
					spots.Insert(house.m_vPos);
					owners.Insert(RZ_EFaction.SURVIVORS);
				}
			}
		}

		float bestHostile = BASE_REACH;
		float bestFriendly = BASE_REACH;
		foreach (int k, vector spot : spots)
		{
			float d = RZ_Util.DistXZ(spot, m_vMiddle);
			if (d < 60 || d > BASE_REACH)
				continue;

			int owner = owners[k];
			if (Hostile(gm, f, owner))
			{
				if (d < bestHostile)
				{
					bestHostile = d;
					m_fHostile = RZ_Compass.Bearing(m_vMiddle, spot);
					m_fHostileDist = d;
					m_iHostileFaction = owner;
				}
			}
			else if (d < bestFriendly)
			{
				bestFriendly = d;
				m_fFriendly = RZ_Compass.Bearing(m_vMiddle, spot);
				m_fFriendlyDist = d;
			}
		}

		RZ_FactionLife life = gm.GetFactionLife();
		if (life && m_iHostileFaction >= 0)
			m_bFeud = life.HasFeud(f, m_iHostileFaction);
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest lair candidate (else wild land) on the land grid, and the town's infection.
	protected void ReadInfected(RZ_GameModeComponent gm)
	{
		if (!gm)
			return;

		RZ_Land land = gm.GetLand();
		vector found;
		if (land && land.NearestCellWith(m_vMiddle, RZ_Land.BIT_LAIR, 0, WILD_REACH, found))
		{
			m_bLair = true;
			m_fWild = RZ_Compass.Bearing(m_vMiddle, found);
			m_fWildDist = RZ_Util.DistXZ(m_vMiddle, found);
		}
		else if (land && land.NearestCellWith(m_vMiddle, RZ_Land.BIT_WILD, 0, WILD_REACH, found))
		{
			m_fWild = RZ_Compass.Bearing(m_vMiddle, found);
			m_fWildDist = RZ_Util.DistXZ(m_vMiddle, found);
		}

		int town = NearestTown(gm);
		if (town >= 0)
			m_iInfection = RZ_Util.RoundToInt(gm.GetSettlements()[town].m_fInfection);
	}

	//------------------------------------------------------------------------------------------------
	protected int NearestTown(RZ_GameModeComponent gm)
	{
		int best = -1;
		float bestDist = float.MAX;
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			float d = RZ_Util.DistXZ(s.m_vCenter, m_vMiddle);
			if (d < Math.Max(s.m_fRadius, 150) && d < bestDist)
			{
				bestDist = d;
				best = i;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! The town's history (Traces): the deaths, fights, fires and takings of the last three days.
	protected void ReadHistory(RZ_GameModeComponent gm)
	{
		if (!gm || !gm.GetTraces())
			return;

		int town = NearestTown(gm);
		if (town < 0)
			return;

		array<ref RZ_TownEvent> events = gm.GetTraces().History().Get(town);
		if (!events)
			return;

		int today = RZ_WorldFacts.Get().Day();
		foreach (RZ_TownEvent e : events)
		{
			if (e.kind == "taken")
				m_bTaken = true;

			bool hurt = e.kind == "dead" || e.kind == "death" || e.kind == "pyre" || e.kind == "infested" || e.kind == "taken" || e.kind == "lost";
			if (hurt && today - e.day <= HISTORY_DAYS)
				m_iRecent++;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Each side's threat: a hostile base that way, the road in, open ground, the infected's land,
	//! a ridge over it; less for the sea, a friendly base or a drop. The highest side is the front.
	protected void WorkOutThreat()
	{
		m_aThreat.Clear();
		for (int s = 0; s < SECTORS; s++)
		{
			m_aThreat.Insert(0);
		}

		if (m_fHostile >= 0)
		{
			int hs = SectorOf(m_fHostile);
			float hostile = 3;
			if (m_fHostileDist < 1000)
				hostile += 1;

			if (m_bFeud)
				hostile += 1;

			m_aThreat[hs] = m_aThreat[hs] + hostile;
		}

		if (m_fMainRoad >= 0)
		{
			int ms = SectorOf(m_fMainRoad);
			m_aThreat[ms] = m_aThreat[ms] + 2;
		}

		foreach (float entry : m_aEntries)
		{
			if (m_fMainRoad >= 0 && SectorOf(entry) == SectorOf(m_fMainRoad))
				continue;

			int es = SectorOf(entry);
			m_aThreat[es] = m_aThreat[es] + 1;
		}

		if (m_fWild >= 0)
		{
			int ws = SectorOf(m_fWild);
			float wild = 2;
			if (m_iInfection > 60)
				wild += 1;

			m_aThreat[ws] = m_aThreat[ws] + wild;
		}

		if (m_fRidge >= 0)
		{
			int rs = SectorOf(m_fRidge);
			m_aThreat[rs] = m_aThreat[rs] + 1;
		}

		if (m_fFriendly >= 0)
		{
			int fs = SectorOf(m_fFriendly);
			m_aThreat[fs] = m_aThreat[fs] - 1;
		}

		for (int i = 0; i < SECTORS; i++)
		{
			if (m_aCover[i] < OPEN_COVER)
				m_aThreat[i] = m_aThreat[i] + 1;

			// Two of the sixteen ways fall in each side
			int bits = (1 << (i * 2)) | (1 << ((i * 2 + 15) % 16));
			if (m_iSeaMask & bits)
				m_aThreat[i] = m_aThreat[i] - 3;
			else if (m_iCliffMask & bits)
				m_aThreat[i] = m_aThreat[i] - 2;
		}

		m_iFront = 0;
		if (m_fMainRoad >= 0)
			m_iFront = SectorOf(m_fMainRoad);

		for (int j = 0; j < SECTORS; j++)
		{
			if (m_aThreat[j] > m_aThreat[m_iFront])
				m_iFront = j;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A side open (little cover) or covered (trees, buildings close).
	bool IsOpen(int s) { return m_aCover[s] < OPEN_COVER; }
	bool IsCovered(int s) { return m_aCover[s] > COVERED; }

	//------------------------------------------------------------------------------------------------
	//! For the log: "front N (road in, Red Hand 1.2 km)".
	string Describe()
	{
		string why = "";
		if (m_fMainRoad >= 0 && SectorOf(m_fMainRoad) == m_iFront)
			why = AddWhy(why, "road in");

		if (m_fHostile >= 0 && SectorOf(m_fHostile) == m_iFront)
			why = AddWhy(why, string.Format("%1 %2 km", RZ_Factions.GetName(m_iHostileFaction), Math.Round(m_fHostileDist / 100) / 10));

		if (m_fWild >= 0 && SectorOf(m_fWild) == m_iFront)
		{
			string wild = "wild land";
			if (m_bLair)
				wild = "a lair";

			why = AddWhy(why, wild);
		}

		if (m_fRidge >= 0 && SectorOf(m_fRidge) == m_iFront)
			why = AddWhy(why, "a ridge");

		if (IsOpen(m_iFront))
			why = AddWhy(why, "open");

		string text = "front " + SectorName(m_iFront);
		if (!why.IsEmpty())
			text += " (" + why + ")";

		string sea = "";
		for (int s = 0; s < SECTORS; s++)
		{
			int bits = (1 << (s * 2)) | (1 << ((s * 2 + 15) % 16));
			if (m_iSeaMask & bits)
				sea = AddWhy(sea, SectorName(s));
		}

		if (!sea.IsEmpty())
			text += ", water " + sea;

		return text;
	}

	//------------------------------------------------------------------------------------------------
	protected static string AddWhy(string list, string item)
	{
		if (list.IsEmpty())
			return item;

		return list + ", " + item;
	}

	//------------------------------------------------------------------------------------------------
	//! For the audit's drawing: the eight threat scores and the record's bearings.
	string DrawText()
	{
		string threat = "";
		foreach (int i, float t : m_aThreat)
		{
			if (i > 0)
				threat += ",";

			threat += RZ_Util.RoundToInt(t).ToString();
		}

		string cover = "";
		foreach (int c, float part : m_aCover)
		{
			if (c > 0)
				cover += ",";

			cover += RZ_Util.RoundToInt(part * 100).ToString();
		}

		return string.Format("threat %1 front %2 cover %3 sea %4 hostile %5 %6 friendly %7 wild %8 ridge %9", threat, m_iFront, cover, m_iSeaMask, RZ_Util.RoundToInt(m_fHostile), RZ_Util.RoundToInt(m_fHostileDist), RZ_Util.RoundToInt(m_fFriendly), RZ_Util.RoundToInt(m_fWild), RZ_Util.RoundToInt(m_fRidge)) + string.Format(" high %1 %2 %3 infection %4 recent %5 through %6", RZ_Util.RoundToInt(m_vHigh[0]), RZ_Util.RoundToInt(m_vHigh[2]), RZ_Util.RoundToInt(m_fHighRise), m_iInfection, m_iRecent, m_iThrough);
	}
}
