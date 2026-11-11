// Test-only land survey (tools/land_survey/run.sh). Not part of the mod.
// Waits for the land grid, prints what it found as "RZL ..." lines (the numbers, a check of the
// inferred bridges against the bridge prefabs on the map, the lair prefabs it matched, a tree
// histogram and a picture of the grid), then closes the server.

//------------------------------------------------------------------------------------------------
//! The bridge prefabs on the map (the survey's ground truth), from one pass over every static
//! entity. Test only: the mod never does this.
class RZL_BridgeFinder
{
	ref array<vector> m_aFound = {};
	ref array<string> m_aNames = {};

	bool OnEntity(IEntity e)
	{
		string name = RZ_Util.GetPrefabName(e);
		if (name.IsEmpty())
			return true;

		// Road and castle bridges; not the training ground's obstacle "bridges"
		name.ToLower();
		if (!name.Contains("bridge") || name.Contains("obstacle"))
			return true;

		vector pos = e.GetOrigin();
		foreach (vector f : m_aFound)
		{
			if (RZ_Util.DistXZ(f, pos) < 80)
				return true;
		}

		m_aFound.Insert(pos);
		m_aNames.Insert(FilePath.StripPath(name));
		return true;
	}
}

modded class RZ_Land
{
	//------------------------------------------------------------------------------------------------
	//! How much of the dry land each of the wild-land rules takes, applied in order.
	void RZL_WildBreakdown()
	{
		int cells = GetCellCount();
		array<int> near = {};
		near.Resize(cells);
		for (int i = 0; i < cells; i++)
			near[i] = 0;

		RZ_WorldFacts facts = m_GM.GetFacts();
		foreach (int si, RZ_Settlement s : m_GM.GetSettlements())
		{
			Stamp(near, s.m_vCenter, s.m_fRadius + 100, 1);
			if (facts && facts.IsLivedIn(si))
				Stamp(near, s.m_vCenter, s.m_fRadius + 300, 2);
		}

		array<vector> people = {};
		LivedInPlaces(people);
		foreach (vector p : people)
			Stamp(near, p, 400, 2);

		RZ_LongTable table = m_GM.GetLongTable();
		bool farm = false;
		if (table)
		{
			RZ_ETableState state = table.GetState();
			farm = state == RZ_ETableState.ACTIVE || state == RZ_ETableState.QUIET || state == RZ_ETableState.EXPOSED;
			if (farm)
				Stamp(near, table.GetHousePos(), 700, 4);
		}

		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		float area = CELL * CELL / 1000000;
		float dry = 0;
		float towns = 0;
		float lived = 0;
		float farmLeft = 0;
		float lineLeft = 0;
		float noLine = 0;
		for (int c = 0; c < cells; c++)
		{
			if (!IsDry(c))
				continue;

			float share = LandShare(c) * area;
			bool atLine = perimeter && perimeter.IsNearLineWithin(CellCenter(c), 200);
			dry += share;
			if ((near[c] & 1) == 0)
				towns += share;
			if ((near[c] & 3) == 0)
				lived += share;
			if ((near[c] & 7) == 0)
				farmLeft += share;
			if ((near[c] & 7) == 0 && !atLine)
				lineLeft += share;
			if (!atLine)
				noLine += share;
		}

		Print(string.Format("RZL wild rules: dry land %1 km2; outside settlements + 100 m %2; and outside lived-in places + 300 m %3; and the Long Table's 700 m (farm this season %4) %5; and the line's warning zone + 200 m %6 (= wild). The line alone leaves %7 of the dry land",
			Tenths(dry), Tenths(towns), Tenths(lived), farm, Tenths(farmLeft), Tenths(lineLeft), Tenths(noLine)));
	}

	//------------------------------------------------------------------------------------------------
	//! A road segment of the road network passes within reach of pos.
	bool RZL_NearRoad(vector pos, float reach)
	{
		array<BaseRoad> roads = {};
		GetRoads(Vector(pos[0] - 100, -1000, pos[2] - 100), Vector(pos[0] + 100, 10000, pos[2] + 100), roads);
		foreach (BaseRoad road : roads)
		{
			if (!road)
				continue;

			array<vector> points = {};
			road.GetPoints(points);
			for (int k = 1; k < points.Count(); k++)
			{
				vector a = points[k - 1];
				vector b = points[k];
				for (int s = 0; s <= 10; s++)
				{
					float t = s;
					t = t / 10;
					if (RZ_Util.DistXZ(a + (b - a) * t, pos) <= reach)
						return true;
				}
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The lair prefabs the reading matched, one line per kind.
	void RZL_DumpMarks()
	{
		foreach (int k, string kind : m_aLairKinds)
		{
			string names = "";
			int n = 0;
			foreach (string prefab, int mark : m_mMarkByPrefab)
			{
				if (mark != k + 1)
					continue;

				n++;
				if (n <= 6)
					names += " " + FilePath.StripPath(prefab);
			}

			Print(string.Format("RZL lair prefabs %1: %2 kinds of prefab:%3", kind, n, names));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! One character per square, north at the top.
	string RZL_Char(int idx)
	{
		if (SeaSamples(idx) >= 3)
			return "~";
		if (HasBit(idx, BIT_LAKE))
			return "o";
		if (HasBit(idx, BIT_BRIDGE))
			return "B";
		if (HasBit(idx, BIT_FORD))
			return "F";
		if (m_aBuildings[idx] >= 8)
			return "H";
		if (HasBit(idx, BIT_RIVER))
			return "=";
		if (HasBit(idx, BIT_LAIR))
			return "L";
		if (HasBit(idx, BIT_RIDGE) && HasBit(idx, BIT_WILD))
			return "^";
		if (HasBit(idx, BIT_VALLEY) && HasBit(idx, BIT_WILD))
			return "v";
		if (HasBit(idx, BIT_FOREST) && HasBit(idx, BIT_WILD))
			return "#";
		if (HasBit(idx, BIT_FOREST))
			return "+";
		if (HasBit(idx, BIT_WILD))
			return ".";
		return ",";
	}

	//------------------------------------------------------------------------------------------------
	void RZL_Report()
	{
		int cells = GetCellCount();
		Print(string.Format("RZL grid: cell 250 m, %1 x %2 = %3 squares, origin square %4 %5, cached %6",
			m_iNX, m_iNZ, cells, m_iX0, m_iZ0, m_bFromCache));
		Print(string.Format("RZL time: read %1 ms of work in %2 steps, longest step %3 ms (longest square %4 ms, longest block of roads %5 ms); worked out in %6 ms; cache load %7 ms",
			m_iReadMs, m_iSteps, m_iMaxStepMs, m_iMaxCellMs, m_iMaxRoadMs, m_iWorkMs, m_iLoadMs));
		// 7 int-sized arrays a square, plus the crossings (a vector and an int each)
		int bytes = cells * 7 * 4 + m_aCross.Count() * 16;
		Print(string.Format("RZL memory: grid arrays about %1 KB (%2 squares x 28 bytes + %3 crossings)", bytes / 1024, cells, m_aCross.Count()));

		int landCells = 0;
		int wildCells = 0;
		int ridgeWild = 0;
		int valleyCells = 0;
		int valleyWild = 0;
		int forestCells = 0;
		int riverCells = 0;
		int lakeCells = 0;
		for (int i = 0; i < cells; i++)
		{
			if (SeaSamples(i) < 5)
				landCells++;
			if (HasBit(i, BIT_WILD))
				wildCells++;
			if (HasBit(i, BIT_RIDGE) && HasBit(i, BIT_WILD))
				ridgeWild++;
			if (HasBit(i, BIT_VALLEY))
				valleyCells++;
			if (HasBit(i, BIT_VALLEY) && HasBit(i, BIT_WILD))
				valleyWild++;
			if (HasBit(i, BIT_FOREST))
				forestCells++;
			if (HasBit(i, BIT_RIVER))
				riverCells++;
			if (HasBit(i, BIT_LAKE))
				lakeCells++;
		}

		Print(string.Format("RZL land: %1 km2 of land (%2 squares with land), %3 km2 wild (%4 wild squares)",
			Tenths(m_fLandKm2), landCells, Tenths(m_fWildKm2), wildCells));
		RZL_WildBreakdown();
		for (int d = 4; d <= 9; d++)
		{
			if (d != 4 && d != 6 && d != 9)
				continue;

			int slots = SlotsAt(d);
			Print(string.Format("RZL slots at 1 per %1 km2: %2 (%3 kept for a split)", d, slots, KeptForSplits(slots)));
		}

		Print(string.Format("RZL water: %1 rivers (%2 km, %3 squares), %4 lakes (%5 squares), %6 small-pond samples, %7 rivers inferred from valleys",
			m_iRivers, Tenths(m_fRiverKm), riverCells, m_iLakes, lakeCells, m_iPonds, m_iValleyRivers));
		Print(string.Format("RZL roads: %1 roads; crossings %2: bridges over a dip %3, over water %4, fords %5",
			m_iRoads, m_aCross.Count(), CountCrossings(CROSS_DIP), CountCrossings(CROSS_WATER), CountCrossings(CROSS_FORD)));
		Print(string.Format("RZL relief: ridge posts %1 (%2 in wild land), valley squares %3 (%4 wild) in %5 valley lines",
			m_iRidges, ridgeWild, valleyCells, valleyWild, m_iValleyLines));
		Print(string.Format("RZL woods: %1 forests (%2 forest squares), %3 fields, %4 lair candidates",
			m_iForests, forestCells, m_iFields, m_iLairs));
		for (int k = 0; k < GetLairKindCount(); k++)
			Print(string.Format("RZL lairs %1: %2", GetLairKindName(k), GetLairCountByKind(k)));

		RZL_DumpMarks();

		// Trees per square (land squares only)
		array<int> edges = {1, 50, 150, 300, 600, 1000, 1500, 100000};
		array<int> counts = {0, 0, 0, 0, 0, 0, 0, 0};
		for (int j = 0; j < cells; j++)
		{
			if (SeaSamples(j) >= 5)
				continue;
			int t = m_aTrees[j];
			for (int b = 0; b < edges.Count(); b++)
			{
				if (t < edges[b])
				{
					counts[b] = counts[b] + 1;
					break;
				}
			}
		}
		Print(string.Format("RZL trees per square: 0: %1, 1-49: %2, 50-149: %3, 150-299: %4, 300-599: %5, 600-999: %6, 1000-1499: %7, 1500+: %8",
			counts[0], counts[1], counts[2], counts[3], counts[4], counts[5], counts[6], counts[7]));

		// Crossings, each with the nearest bridge prefab
		RZL_BridgeFinder finder = new RZL_BridgeFinder();
		vector mins, maxs;
		GetGame().GetWorld().GetBoundBox(mins, maxs);
		GetGame().GetWorld().QueryEntitiesByAABB(mins, maxs, finder.OnEntity, null, EQueryEntitiesFlags.STATIC);
		int matched = 0;
		foreach (int c, vector cp : m_aCross)
		{
			float best = 100000;
			string bestName = "";
			foreach (int f, vector fp : finder.m_aFound)
			{
				float dist = RZ_Util.DistXZ(cp, fp);
				if (dist < best)
				{
					best = dist;
					bestName = finder.m_aNames[f];
				}
			}

			int cell = CellAt(cp);
			int width = 0;
			if (cell >= 0)
				width = m_aRoad[cell];

			string kindName = "ford";
			if (m_aCrossKind[c] == CROSS_DIP)
				kindName = "bridge over a dip";
			else if (m_aCrossKind[c] == CROSS_WATER)
				kindName = "bridge over water";

			if (best <= 100 && m_aCrossKind[c] != CROSS_FORD)
				matched++;

			Print(string.Format("RZL crossing %1: %2 at %3 (road up to %4 m in its square); nearest bridge prefab %5 m: %6",
				c, kindName, cp, width, RZ_Util.RoundToInt(best), bestName));
		}

		int found = 0;
		int onRoad = 0;
		int onRoadFound = 0;
		foreach (int g, vector gp : finder.m_aFound)
		{
			bool hit = false;
			foreach (int h, vector hp : m_aCross)
			{
				if (m_aCrossKind[h] != CROSS_FORD && RZ_Util.DistXZ(gp, hp) <= 100)
					hit = true;
			}

			bool road = RZL_NearRoad(gp, 25);
			if (road)
				onRoad++;

			if (hit)
			{
				found++;
				if (road)
					onRoadFound++;
			}
			else
				Print(string.Format("RZL bridge prefab not inferred: %1 at %2 (a road within 25 m: %3)", finder.m_aNames[g], gp, road));
		}

		Print(string.Format("RZL bridges check: %1 inferred bridges, %2 of them within 100 m of a bridge prefab; %3 bridge prefab groups on the map (%4 with a road over them), %5 of them found (%6 of those with a road)",
			CountCrossings(CROSS_DIP) + CountCrossings(CROSS_WATER), matched, finder.m_aFound.Count(), onRoad, found, onRoadFound));

		// The picture, north at the top
		Print("RZL map legend: ~ sea, o lake, B bridge, F ford, H 8+ buildings, = river, L lair candidate, ^ ridge, v valley, # forest (wild), + forest (not wild), . wild, , not wild");
		for (int gz = m_iNZ - 1; gz >= 0; gz--)
		{
			string row = "";
			for (int gx = 0; gx < m_iNX; gx++)
				row += RZL_Char(CellIndex(gx, gz));

			Print("RZL map |" + row + "|");
		}
	}
}

modded class RZ_GameModeComponent
{
	protected int m_iRZLSec;
	protected bool m_bRZLDone;

	override void OnGameModeStart()
	{
		super.OnGameModeStart();
		Print(string.Format("RZL start: world %1, land survey %2", GetGame().GetWorldFile(), GetLand() && GetLand().IsEnabled()));
		GetGame().GetCallqueue().CallLater(RZL_Tick, 1000, true);
	}

	void RZL_Tick()
	{
		m_iRZLSec++;
		RZ_Land land = GetLand();
		if (m_bRZLDone || !land)
			return;

		if (!land.IsReady())
		{
			if (m_iRZLSec % 30 == 0)
				Print(string.Format("RZL t=%1 s: still reading (%2 steps so far)", m_iRZLSec, land.GetSteps()));
			return;
		}

		m_bRZLDone = true;
		Print(string.Format("RZL ready after %1 s", m_iRZLSec));
		land.RZL_Report();
		Print("RZL done");
		GetGame().GetCallqueue().CallLater(RZL_Close, 2000, false);
	}

	void RZL_Close()
	{
		GetGame().RequestClose();
	}
}
