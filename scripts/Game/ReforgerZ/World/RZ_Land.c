//! The land grid (docs/DESIGN.md, "The land grid"): what each 250 m square of the map is like
//! (water, height, trees, buildings, roads, bridges) and which squares are wild, read from the
//! engine once and cached in the profile. The infected tribes (planned) will live in the wild it
//! finds. Off unless Zombies > Tribes > Land survey is on. Server only; nothing is spawned.
//!
//! The squares are the heat map's: square (gx, gz) covers x from gx * 250 and z from gz * 250.
//! Reading a square: five terrain samples (height, sea, river or pond), one query of the square's
//! static entities (trees, buildings, lair landmarks), and, eight squares at a time, the road
//! segments whose middle lies in them (roads, and bridges inferred where a segment runs 3 m or more
//! above the ground between its points). Rivers come from the map's river entities. Then, from the
//! grid: ridges, valley floors, forests, fields, lair candidates and the wild flag.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Zombies > Tribes. The tribes themselves are planned; for now
//! this holds only what the land grid needs.
[BaseContainerProps()]
class RZ_TribeSettings
{
	[Attribute("0", desc: "Land survey: read the map into the land grid at the first start (water, height, trees, buildings, roads, bridges, wild land), cache it in the profile and log what it found (docs/DESIGN.md, The land grid). Off by default: nothing uses it yet (the tribes are planned)")]
	bool m_bLandSurvey;

	[Attribute("20", UIWidgets.Slider, "Land survey: land squares (250 m) read a second while the grid is first built. Sea squares cost almost nothing, and a second's reading stops after 8 ms whatever this says", "1 100 1")]
	int m_iLandCellsPerSecond;
}

//------------------------------------------------------------------------------------------------
//! What the land grid caches: only what was read from the engine. Everything worked out from it
//! (ridges, valleys, forests, fields, lairs, wild land) is worked out again on load.
class RZ_LandSave
{
	int version;
	string world;
	int hash;
	int x0;
	int z0;
	int nx;
	int nz;
	int readMs;
	ref array<int> height = {};		// mean of the five samples, decimetres
	ref array<int> range = {};		// highest minus lowest sample, metres
	ref array<int> bits = {};		// RZ_Land's read bits (READ_MASK)
	ref array<int> trees = {};
	ref array<int> buildings = {};
	ref array<int> road = {};		// widest road, metres (0 = none)
	ref array<int> river = {};		// widest river, metres (0 = none)
	ref array<float> crossX = {};
	ref array<float> crossZ = {};
	ref array<int> crossKind = {};	// RZ_Land.CROSS_*
	int rivers;
	float riverKm;
	int ponds;
	int roads;
}

//------------------------------------------------------------------------------------------------
class RZ_Land
{
	static const float CELL = 250;
	static const int SAVE_VERSION = 1;

	// What a square holds (m_aBits). Read from the map, cached:
	static const int SEA_MASK = 7;			// bits 0-2: samples in the sea (0-5)
	static const int INLAND_SHIFT = 3;		// bits 3-5: samples in a river or pond (0-5)
	static const int BIT_LAKE = 64;			// a sample in inland water over about 100 m across
	static const int BIT_RIVER = 128;		// a river runs through it (the map's river entities)
	static const int BIT_BRIDGE = 256;		// an inferred bridge
	static const int BIT_FORD = 512;		// a road through a river with no deck
	static const int MARK_SHIFT = 10;		// bits 10-13: a lair landmark, 1 + its index in the lair kinds
	static const int READ_MASK = 16383;
	// Worked out from the grid, never cached:
	static const int BIT_RIDGE = 65536;		// a ridge: 25 m above its neighbours, or the highest within 1 km
	static const int BIT_VALLEY = 131072;	// a valley floor: 15 m below both neighbours across it
	static const int BIT_FOREST = 262144;	// forest, in a forest of 3 squares or more
	static const int BIT_FIELD = 524288;	// open, flat, within 800 m of a settlement
	static const int BIT_LAIR = 1048576;	// a lair candidate (forest or landmark, far from people)
	static const int BIT_WILD = 2097152;

	static const int TREES_NONE = 0;
	static const int TREES_SPARSE = 1;
	static const int TREES_FOREST = 2;

	static const int CROSS_DIP = 1;		// a bridge over a dip
	static const int CROSS_WATER = 2;	// a bridge over a river or pond
	static const int CROSS_FORD = 3;	// a road through a river or pond, no deck

	// Trees in a square (6.25 ha; trees only, not bushes). Under 300 it's open land (hedgerows and
	// orchards); 600 or more, about one tree per 100 m2, is forest: about 45% of Arland's and
	// Everon's land squares (tools/land_survey). Worked out from the cached count, so changing these
	// needs no new reading.
	protected static const int TREES_PER_SPARSE = 300;
	protected static const int TREES_PER_FOREST = 600;
	protected static const float FIELD_RANGE = 20;		// a field: its samples within 20 m of each other
	protected static const float FIELD_RELIEF = 15;		// and within 15 m of its neighbours' mean
	protected static const float DECK_HEIGHT = 3;		// a road this far above the ground is a deck
	protected static const float ROAD_STEP = 4;			// metres between checks along a road segment
	protected static const float CROSS_MERGE = 60;		// crossings closer than this are one
	protected static const float LAKE_EXTENT = 50;		// inland water reaching this far is a lake
	protected static const float RIDGE_RELIEF = 25;
	protected static const float VALLEY_DEPTH = 15;
	protected static const int VALLEY_REACH = 2;		// squares across a valley floor to compare with
	protected static const int ROAD_BLOCK = 8;			// squares of a row whose roads are read at once
	protected static const int RIDGE_REACH = 4;			// squares: "the highest within 1 km"
	protected static const int STEP_MS = 8;				// a second's reading stops after this
	protected static const float SMALL_MAP_KM2 = 25;

	protected RZ_GameModeComponent m_GM;
	protected RZ_TribeSettings m_Settings;
	protected bool m_bReady;
	protected bool m_bBuilding;

	// The grid
	protected int m_iX0;
	protected int m_iZ0;
	protected int m_iNX;
	protected int m_iNZ;
	protected ref array<float> m_aHeight = {};
	protected ref array<int> m_aRange = {};
	protected ref array<int> m_aBits = {};
	protected ref array<int> m_aTrees = {};
	protected ref array<int> m_aBuildings = {};
	protected ref array<int> m_aRoad = {};
	protected ref array<int> m_aRiver = {};
	protected ref array<vector> m_aCross = {};
	protected ref array<int> m_aCrossKind = {};
	protected int m_iRivers;
	protected float m_fRiverKm;
	protected int m_iPonds;
	protected int m_iRoads;
	protected int m_iHash;
	protected string m_sWorld;

	// Reading
	protected int m_iNext;				// next square to read
	protected int m_iRoadAt = -1;		// the square at which a block of roads was last read
	protected int m_iMaxCellMs;			// the longest square
	protected int m_iMaxRoadMs;			// the longest block of roads
	protected int m_iReadMs;			// ms spent reading
	protected int m_iMaxStepMs;
	protected int m_iSteps;
	protected int m_iStartedAt;
	protected bool m_bFromCache;
	protected int m_iLoadMs;			// loading the cache
	protected int m_iWorkMs;			// working out ridges, valleys, forests, wild land
	protected ref array<float> m_aOffX = {0, -62.5, 62.5, -62.5, 62.5};
	protected ref array<float> m_aOffZ = {0, -62.5, -62.5, 62.5, 62.5};
	protected ref array<string> m_aLairKinds = {"barn", "mill", "silo", "quarry", "culvert", "shed", "ruin"};
	protected ref map<string, int> m_mMarkByPrefab = new map<string, int>();
	protected ref map<string, bool> m_mTreeByPrefab = new map<string, bool>();
	protected int m_iCellTrees;
	protected int m_iCellBuildings;
	protected int m_iCellMark;

	// Worked out
	protected float m_fLandKm2;
	protected float m_fWildKm2;
	protected int m_iRidges;
	protected int m_iValleyLines;
	protected int m_iValleyRivers;	// with no river entities: valley lines through two or more bridges
	protected int m_iForests;
	protected int m_iFields;
	protected int m_iLakes;
	protected int m_iLairs;
	protected ref array<int> m_aLairByKind = {};	// [0] forest, then one per LAIR_KINDS

	//------------------------------------------------------------------------------------------------
	void RZ_Land(RZ_GameModeComponent gm, RZ_TribeSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled() { return m_Settings && m_Settings.m_bLandSurvey; }
	//! The grid is read (or loaded) and worked out. Nothing about the land is known before.
	bool IsReady() { return m_bReady; }
	bool IsBuilding() { return m_bBuilding; }
	bool WasCached() { return m_bFromCache; }
	int GetCellCount() { return m_iNX * m_iNZ; }
	int GetWidth() { return m_iNX; }
	int GetDepth() { return m_iNZ; }
	float GetLandKm2() { return m_fLandKm2; }
	float GetWildKm2() { return m_fWildKm2; }
	int GetRiverCount() { return m_iRivers; }
	float GetRiverKm() { return m_fRiverKm; }
	int GetLakeCount() { return m_iLakes; }
	int GetPondSamples() { return m_iPonds; }
	int GetRoadCount() { return m_iRoads; }
	int GetRidgeCount() { return m_iRidges; }
	int GetValleyLineCount() { return m_iValleyLines; }
	int GetValleyRiverCount() { return m_iValleyRivers; }
	int GetForestCount() { return m_iForests; }
	int GetFieldCount() { return m_iFields; }
	int GetLairCount() { return m_iLairs; }
	int GetReadMs() { return m_iReadMs; }
	int GetMaxStepMs() { return m_iMaxStepMs; }
	int GetSteps() { return m_iSteps; }
	int GetLoadMs() { return m_iLoadMs; }
	int GetMaxCellMs() { return m_iMaxCellMs; }
	int GetMaxRoadMs() { return m_iMaxRoadMs; }
	int GetWorkMs() { return m_iWorkMs; }
	string GetCachePath() { return CachePath(); }

	//------------------------------------------------------------------------------------------------
	//! Lair candidates of one kind: 0 forest, then 1 + the index in barn, mill, silo, quarry, culvert,
	//! shed, ruin.
	int GetLairCountByKind(int kind)
	{
		if (kind < 0 || kind >= m_aLairByKind.Count())
			return 0;

		return m_aLairByKind[kind];
	}

	//------------------------------------------------------------------------------------------------
	string GetLairKindName(int kind)
	{
		if (kind == 0)
			return "forest";

		if (kind < 1 || kind > m_aLairKinds.Count())
			return "";

		return m_aLairKinds[kind - 1];
	}

	//------------------------------------------------------------------------------------------------
	int GetLairKindCount() { return m_aLairKinds.Count() + 1; }

	//------------------------------------------------------------------------------------------------
	//! Bridges and fords: kind CROSS_DIP, CROSS_WATER or CROSS_FORD.
	int GetCrossingCount() { return m_aCross.Count(); }
	vector GetCrossing(int i) { return m_aCross[i]; }
	int GetCrossingKind(int i) { return m_aCrossKind[i]; }

	//------------------------------------------------------------------------------------------------
	//! Count of crossings of one kind.
	int CountCrossings(int kind)
	{
		int n = 0;
		foreach (int k : m_aCrossKind)
		{
			if (k == kind)
				n++;
		}

		return n;
	}

	// ================================================================ Squares
	//------------------------------------------------------------------------------------------------
	//! The square at pos, -1 outside the grid.
	int CellAt(vector pos)
	{
		int gx = Math.Floor(pos[0] / CELL) - m_iX0;
		int gz = Math.Floor(pos[2] / CELL) - m_iZ0;
		return CellIndex(gx, gz);
	}

	//------------------------------------------------------------------------------------------------
	//! The square at grid column gx and row gz (from the grid's corner), -1 outside.
	int CellIndex(int gx, int gz)
	{
		if (gx < 0 || gz < 0 || gx >= m_iNX || gz >= m_iNZ)
			return -1;

		return gz * m_iNX + gx;
	}

	//------------------------------------------------------------------------------------------------
	//! The square's centre on the ground.
	vector CellCenter(int idx)
	{
		int gx = idx % m_iNX;
		int gz = idx / m_iNX;
		float x = (m_iX0 + gx + 0.5) * CELL;
		float z = (m_iZ0 + gz + 0.5) * CELL;
		return Vector(x, m_aHeight[idx], z);
	}

	int GetBits(int idx) { return m_aBits[idx]; }
	bool HasBit(int idx, int bit) { return (m_aBits[idx] & bit) != 0; }
	float GetHeight(int idx) { return m_aHeight[idx]; }
	int GetTrees(int idx) { return m_aTrees[idx]; }
	int GetBuildings(int idx) { return m_aBuildings[idx]; }
	int GetRoadWidth(int idx) { return m_aRoad[idx]; }
	int GetRiverWidth(int idx) { return m_aRiver[idx]; }
	int GetMark(int idx) { return (m_aBits[idx] >> MARK_SHIFT) & 15; }

	//------------------------------------------------------------------------------------------------
	//! TREES_NONE (open), TREES_SPARSE or TREES_FOREST, from the square's tree count.
	int GetTreeClass(int idx)
	{
		if (m_aTrees[idx] >= TREES_PER_FOREST)
			return TREES_FOREST;

		if (m_aTrees[idx] >= TREES_PER_SPARSE)
			return TREES_SPARSE;

		return TREES_NONE;
	}

	//------------------------------------------------------------------------------------------------
	//! Samples (of five) in the sea, and in a river or pond.
	int SeaSamples(int idx) { return m_aBits[idx] & SEA_MASK; }
	int InlandSamples(int idx) { return (m_aBits[idx] >> INLAND_SHIFT) & 7; }

	//------------------------------------------------------------------------------------------------
	//! Share of the square that's dry land, from its five samples.
	float LandShare(int idx)
	{
		int wet = SeaSamples(idx) + InlandSamples(idx);
		float share = 5 - wet;
		return Math.Clamp(share / 5, 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Dry enough to live on: at most two of five samples in water, and no lake.
	bool IsDry(int idx)
	{
		if (HasBit(idx, BIT_LAKE))
			return false;

		return SeaSamples(idx) + InlandSamples(idx) <= 2;
	}

	//------------------------------------------------------------------------------------------------
	//! Wild land at pos: false before the grid is ready.
	bool IsWild(vector pos)
	{
		if (!m_bReady)
			return false;

		int idx = CellAt(pos);
		return idx >= 0 && HasBit(idx, BIT_WILD);
	}

	//------------------------------------------------------------------------------------------------
	//! Height above the mean of the eight neighbours that are mostly land (a sea square reads the
	//! sea bed, -256 m on Cain, which would make every coast a ridge).
	float Relief(int idx)
	{
		int gx = idx % m_iNX;
		int gz = idx / m_iNX;
		float sum = 0;
		int n = 0;
		for (int dz = -1; dz <= 1; dz++)
		{
			for (int dx = -1; dx <= 1; dx++)
			{
				if (dx == 0 && dz == 0)
					continue;

				int j = CellIndex(gx + dx, gz + dz);
				if (j < 0 || SeaSamples(j) >= 3)
					continue;

				sum += m_aHeight[j];
				n++;
			}
		}

		if (n == 0)
			return 0;

		return m_aHeight[idx] - sum / n;
	}

	// ================================================================ Tribe slots
	//------------------------------------------------------------------------------------------------
	//! Tribes the wild land holds at one per `divisor` km2 (the design's 6), rounded, 1 to 15. None
	//! under 1.5 km2 of wild land or with no lair candidate.
	int SlotsAt(float divisor)
	{
		if (!m_bReady || divisor <= 0 || m_fWildKm2 < 1.5 || m_iLairs <= 0)
			return 0;

		return Math.ClampInt(RZ_Util.RoundToInt(m_fWildKm2 / divisor), 1, 15);
	}

	//------------------------------------------------------------------------------------------------
	//! Of the slots, those kept for a tribe that splits.
	static int KeptForSplits(int slots)
	{
		if (slots >= 13)
			return 3;

		if (slots >= 9)
			return 2;

		if (slots >= 3)
			return 1;

		return 0;
	}

	// ================================================================ Start
	//------------------------------------------------------------------------------------------------
	//! At start, after the settlements, the Perimeter, safehouses, territory and the Long Table: loads
	//! the cached grid for this map or starts reading it (a few squares a second, from Step).
	void Setup()
	{
		if (!IsEnabled())
			return;

		vector mins, maxs;
		if (!GetWorldBounds(mins, maxs))
		{
			RZ_Log.Warn("Land survey: couldn't read the map's size; the land grid is off.");
			return;
		}

		m_iX0 = Math.Floor(mins[0] / CELL);
		m_iZ0 = Math.Floor(mins[2] / CELL);
		int x1 = Math.Floor(maxs[0] / CELL);
		int z1 = Math.Floor(maxs[2] / CELL);
		m_iNX = x1 - m_iX0 + 1;
		m_iNZ = z1 - m_iZ0 + 1;
		m_sWorld = WorldToken();
		m_iHash = TerrainHash(mins, maxs);

		int loadStart = System.GetTickCount();
		if (LoadCache())
		{
			m_iLoadMs = System.GetTickCount() - loadStart;
			m_bFromCache = true;
			Finish();
			return;
		}

		int cells = m_iNX * m_iNZ;
		m_aHeight.Resize(cells);
		m_aRange.Resize(cells);
		m_aBits.Resize(cells);
		m_aTrees.Resize(cells);
		m_aBuildings.Resize(cells);
		m_aRoad.Resize(cells);
		m_aRiver.Resize(cells);
		for (int i = 0; i < cells; i++)
		{
			m_aHeight[i] = 0;
			m_aRange[i] = 0;
			m_aBits[i] = 0;
			m_aTrees[i] = 0;
			m_aBuildings[i] = 0;
			m_aRoad[i] = 0;
			m_aRiver[i] = 0;
		}

		m_iStartedAt = System.GetTickCount();
		m_bBuilding = true;
		m_iNext = 0;
		m_iRoadAt = -1;
		int t0 = System.GetTickCount();
		ReadRivers();
		AddStepTime(System.GetTickCount() - t0);
		RZ_Log.Info(string.Format("Land survey: reading the map, %1 squares of 250 m (%2 x %3). Tribes stay dormant until it's done.",
			cells, m_iNX, m_iNZ));
	}

	//------------------------------------------------------------------------------------------------
	//! Every second (the bite check's tick) while reading: the next few squares.
	void Step()
	{
		if (!m_bBuilding)
			return;

		int t0 = System.GetTickCount();
		int budget = Math.Max(1, m_Settings.m_iLandCellsPerSecond);
		int cells = m_iNX * m_iNZ;
		int landRead = 0;
		while (m_iNext < cells && landRead < budget)
		{
			int gx = m_iNext % m_iNX;
			int gz = m_iNext / m_iNX;
			if (gx % ROAD_BLOCK == 0 && m_iRoadAt < m_iNext)
			{
				// The roads of the next eight squares of the row first; if that took half the
				// second's budget, the squares wait for the next second
				int r0 = System.GetTickCount();
				ReadRoadBlock(gx, gz);
				m_iRoadAt = m_iNext;
				int roadMs = System.GetTickCount() - r0;
				if (roadMs > m_iMaxRoadMs)
					m_iMaxRoadMs = roadMs;

				if (System.GetTickCount() - t0 >= STEP_MS / 2)
					break;
			}

			int c0 = System.GetTickCount();
			if (ReadCell(m_iNext))
				landRead++;

			int cellMs = System.GetTickCount() - c0;
			if (cellMs > m_iMaxCellMs)
				m_iMaxCellMs = cellMs;

			m_iNext++;
			if (System.GetTickCount() - t0 >= STEP_MS)
				break;
		}

		AddStepTime(System.GetTickCount() - t0);
		if (m_iNext < cells)
			return;

		m_bBuilding = false;
		int t1 = System.GetTickCount();
		SaveCache();
		AddStepTime(System.GetTickCount() - t1);
		Finish();
	}

	//------------------------------------------------------------------------------------------------
	protected void AddStepTime(int ms)
	{
		m_iReadMs += ms;
		m_iSteps++;
		if (ms > m_iMaxStepMs)
			m_iMaxStepMs = ms;
	}

	//------------------------------------------------------------------------------------------------
	//! Works out everything from the read grid, the wild land, and logs it.
	protected void Finish()
	{
		int t0 = System.GetTickCount();
		WorkOutTerrain();
		RefreshWild();
		m_bReady = true;
		m_iWorkMs = System.GetTickCount() - t0;
		LogSummary();
	}

	// ================================================================ Reading the map
	//------------------------------------------------------------------------------------------------
	//! One square: five samples, then its static entities. False for a square that is all sea
	//! (nearly free: no entity query).
	protected bool ReadCell(int idx)
	{
		int gx = idx % m_iNX;
		int gz = idx / m_iNX;
		float cx = (m_iX0 + gx + 0.5) * CELL;
		float cz = (m_iZ0 + gz + 0.5) * CELL;

		float sum = 0;
		float lo = float.MAX;
		float hi = -float.MAX;
		int sea = 0;
		int inland = 0;
		bool lake = false;
		for (int k = 0; k < 5; k++)
		{
			float x = cx + m_aOffX[k];
			float z = cz + m_aOffZ[k];
			float y = TerrainY(x, z);
			sum += y;
			lo = Math.Min(lo, y);
			hi = Math.Max(hi, y);

			float reach;
			EWaterSurfaceType wt = WaterAt(Vector(x, y, z), reach);
			if (wt == EWaterSurfaceType.WST_OCEAN || RZ_Util.IsWater(Vector(x, y, z)))
				sea++;
			else if (wt == EWaterSurfaceType.WST_POND || wt == EWaterSurfaceType.WST_RIVER)
			{
				inland++;
				if (wt == EWaterSurfaceType.WST_POND)
				{
					if (reach >= LAKE_EXTENT)
						lake = true;
					else
						m_iPonds++;
				}
			}
		}

		m_aHeight[idx] = sum / 5;
		m_aRange[idx] = RZ_Util.RoundToInt(hi - lo);
		int bits = m_aBits[idx] & (BIT_RIVER | BIT_BRIDGE | BIT_FORD);
		bits = bits | sea | (inland << INLAND_SHIFT);
		if (lake)
			bits = bits | BIT_LAKE;

		if (sea >= 5)
		{
			m_aBits[idx] = bits;
			return false;
		}

		m_iCellTrees = 0;
		m_iCellBuildings = 0;
		m_iCellMark = 0;
		vector mins = Vector(cx - CELL * 0.5, -1000, cz - CELL * 0.5);
		vector maxs = Vector(cx + CELL * 0.5, 10000, cz + CELL * 0.5);
		QueryStatic(mins, maxs);

		bits = bits | (m_iCellMark << MARK_SHIFT);
		m_aBits[idx] = bits;
		m_aTrees[idx] = m_iCellTrees;
		m_aBuildings[idx] = m_iCellBuildings;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Called for every static entity in the square being read.
	protected bool OnCellEntity(IEntity e)
	{
		if (IsVegetation(e))
		{
			if (IsTreePrefab(e))
				m_iCellTrees++;

			return true;
		}

		if (IsBuilding(e))
			m_iCellBuildings++;

		if (m_iCellMark == 0)
			m_iCellMark = LairMark(e);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A tree, not a bush or a plant. The base game's trees are prefabs under .../Vegetation/Tree/
	//! (Arland) or .../Vegetation/Trees/ (Everon's prefab library), its bushes and plants (also
	//! BaseTree: a third of them on Arland and Everon) under Bush, Bushes, Plant and Plants. Cached
	//! by prefab name.
	protected bool IsTreePrefab(IEntity e)
	{
		string name = RZ_Util.GetPrefabName(e);
		bool tree;
		if (m_mTreeByPrefab.Find(name, tree))
			return tree;

		string lower = name;
		lower.ToLower();
		tree = lower.Contains("/tree");
		m_mTreeByPrefab.Set(name, tree);
		return tree;
	}

	//------------------------------------------------------------------------------------------------
	//! 1 + the lair kind a landmark is (barn, mill...), 0 for anything else. Cached by prefab name.
	protected int LairMark(IEntity e)
	{
		string name = RZ_Util.GetPrefabName(e);
		if (name.IsEmpty())
			return 0;

		int mark;
		if (m_mMarkByPrefab.Find(name, mark))
			return mark;

		string lower = name;
		lower.ToLower();
		mark = 0;
		foreach (int i, string kind : m_aLairKinds)
		{
			if (RZ_LandmarkFinder.Matches(kind, lower))
			{
				mark = i + 1;
				break;
			}
		}

		m_mMarkByPrefab.Set(name, mark);
		return mark;
	}

	//------------------------------------------------------------------------------------------------
	//! The road segments whose middle lies in the eight squares of row gz from column gx0: the widest
	//! road in each square they cross, and crossings: a deck (the straight line between two road
	//! points 3 m or more above the ground) or a ford (the road runs through a river or pond with no
	//! deck). Road points themselves sit on the ground, even at bridges: only the line between them
	//! shows the deck (on Arland no road point is even 2 m up). Eight squares at a time, because a
	//! whole row of Cain's roads took up to 26 ms.
	protected void ReadRoadBlock(int gx0, int gz)
	{
		float zLo = (m_iZ0 + gz) * CELL;
		float zHi = zLo + CELL;
		float xLo = (m_iX0 + gx0) * CELL;
		float xHi = xLo + ROAD_BLOCK * CELL;
		vector mins = Vector(xLo, -1000, zLo);
		vector maxs = Vector(xHi, 10000, zHi);
		array<BaseRoad> roads = {};
		GetRoads(mins, maxs, roads);
		foreach (BaseRoad road : roads)
		{
			if (!road)
				continue;

			array<vector> points = {};
			road.GetPoints(points);
			int width = RZ_Util.RoundToInt(road.GetWidth());
			if (width < 1)
				width = 1;

			for (int k = 1; k < points.Count(); k++)
			{
				vector a = points[k - 1];
				vector b = points[k];
				float midX = (a[0] + b[0]) * 0.5;
				float midZ = (a[2] + b[2]) * 0.5;
				if (midZ < zLo || midZ >= zHi || midX < xLo || midX >= xHi)
					continue;

				// Counted once: its first segment lies in one block only
				if (k == 1)
					m_iRoads++;

				ReadRoadSegment(a, b, width);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ReadRoadSegment(vector a, vector b, int width)
	{
		float len = RZ_Util.DistXZ(a, b);
		int steps = RZ_Util.RoundToInt(Math.Max(1, Math.Floor(len / ROAD_STEP)));
		float deck = 0;
		vector deckAt = a;
		bool wet = false;
		vector wetAt = a;
		for (int s = 0; s <= steps; s++)
		{
			float t = s;
			t = t / steps;
			vector p = a + (b - a) * t;
			int idx = CellAt(p);
			if (idx >= 0 && m_aRoad[idx] < width)
				m_aRoad[idx] = width;

			if (s == 0 || s == steps)
				continue;

			float ground = TerrainY(p[0], p[2]);
			float above = p[1] - ground;
			if (above > deck)
			{
				deck = above;
				deckAt = p;
			}

			if (!wet)
			{
				float reach;
				EWaterSurfaceType wt = WaterAt(Vector(p[0], ground, p[2]), reach);
				if (wt == EWaterSurfaceType.WST_RIVER || wt == EWaterSurfaceType.WST_POND)
				{
					wet = true;
					wetAt = p;
				}
			}
		}

		if (deck >= DECK_HEIGHT)
		{
			int kind = CROSS_DIP;
			if (wet)
				kind = CROSS_WATER;

			AddCrossing(deckAt, kind);
		}
		else if (wet)
			AddCrossing(wetAt, CROSS_FORD);
	}

	//------------------------------------------------------------------------------------------------
	//! A crossing, merged with one within CROSS_MERGE (a bridge's two lanes, its segments). A deck
	//! over water beats a deck over a dip, which beats a ford.
	protected void AddCrossing(vector pos, int kind)
	{
		foreach (int i, vector c : m_aCross)
		{
			if (RZ_Util.DistXZ(c, pos) >= CROSS_MERGE)
				continue;

			if (Rank(kind) > Rank(m_aCrossKind[i]))
			{
				m_aCrossKind[i] = kind;
				m_aCross[i] = pos;
			}

			MarkCrossing(pos, kind);
			return;
		}

		m_aCross.Insert(pos);
		m_aCrossKind.Insert(kind);
		MarkCrossing(pos, kind);
	}

	//------------------------------------------------------------------------------------------------
	protected static int Rank(int kind)
	{
		if (kind == CROSS_WATER)
			return 3;

		if (kind == CROSS_DIP)
			return 2;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	protected void MarkCrossing(vector pos, int kind)
	{
		int idx = CellAt(pos);
		if (idx < 0)
			return;

		if (kind == CROSS_FORD)
			m_aBits[idx] = m_aBits[idx] | BIT_FORD;
		else
			m_aBits[idx] = m_aBits[idx] | BIT_BRIDGE;
	}

	//------------------------------------------------------------------------------------------------
	//! The map's rivers: every square a river's centre line passes through, with its widest width.
	protected void ReadRivers()
	{
		array<RiverEntity> rivers = {};
		GetRiverEntities(rivers);
		foreach (RiverEntity river : rivers)
		{
			array<vector> points = {};
			array<float> widths = {};
			if (!RiverLine(river, points, widths) || points.Count() < 2)
				continue;

			m_iRivers++;
			float len = 0;
			for (int k = 1; k < points.Count(); k++)
			{
				vector a = points[k - 1];
				vector b = points[k];
				float seg = RZ_Util.DistXZ(a, b);
				len += seg;
				int steps = RZ_Util.RoundToInt(Math.Max(1, Math.Ceil(seg / 25)));
				for (int s = 0; s <= steps; s++)
				{
					float t = s;
					t = t / steps;
					vector p = a + (b - a) * t;
					int idx = CellAt(p);
					if (idx < 0)
						continue;

					m_aBits[idx] = m_aBits[idx] | BIT_RIVER;
					int w = RZ_Util.RoundToInt(Math.Max(widths[k - 1], widths[k]));
					if (w > m_aRiver[idx])
						m_aRiver[idx] = w;
				}
			}

			m_fRiverKm += len / 1000;
		}
	}

	// ================================================================ Engine calls
	//------------------------------------------------------------------------------------------------
	protected static bool GetWorldBounds(out vector mins, out vector maxs)
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return false;

		world.GetBoundBox(mins, maxs);
		return maxs[0] > mins[0] && maxs[2] > mins[2];
	}

	//------------------------------------------------------------------------------------------------
	protected static float TerrainY(float x, float z)
	{
		return GetGame().GetWorld().GetSurfaceY(x, z);
	}

	//------------------------------------------------------------------------------------------------
	//! The water at a point on the ground: WST_NONE (dry), WST_OCEAN, WST_POND or WST_RIVER; `reach`
	//! is the water body's larger horizontal extent (metres). Measured on Arland, Everon and Cain:
	//! rivers read WST_RIVER, Everon's reservoir WST_POND with an extent of 442 (tools/land_survey).
	protected static EWaterSurfaceType WaterAt(vector ground, out float reach)
	{
		reach = 0;
		vector probe = Vector(ground[0], ground[1] + 0.2, ground[2]);
		vector surface;
		EWaterSurfaceType type;
		vector transform[4];
		vector extents;
		if (!ChimeraWorldUtils.TryGetWaterSurface(GetGame().GetWorld(), probe, surface, type, transform, extents))
			return EWaterSurfaceType.WST_NONE;

		reach = Math.Max(extents[0], extents[2]);
		return type;
	}

	//------------------------------------------------------------------------------------------------
	protected void QueryStatic(vector mins, vector maxs)
	{
		GetGame().GetWorld().QueryEntitiesByAABB(mins, maxs, OnCellEntity, null, EQueryEntitiesFlags.STATIC);
	}

	//------------------------------------------------------------------------------------------------
	//! Trees, bushes and plants are all BaseTree (Tree, SCR_DestructibleTreeV2).
	protected static bool IsVegetation(IEntity e)
	{
		return BaseTree.Cast(e) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! The settlement scanner's test: map buildings are SCR_DestructibleBuildingEntity.
	protected static bool IsBuilding(IEntity e)
	{
		return SCR_DestructibleBuildingEntity.Cast(e) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Roads from the AI world's road network (empty without an SCR_AIWorld in the world).
	protected static void GetRoads(vector mins, vector maxs, notnull array<BaseRoad> roads)
	{
		RoadNetworkManager network = RZ_Perimeter.GetRoadNetwork();
		if (network)
			network.GetRoadsInAABB(mins, maxs, roads);
	}

	//------------------------------------------------------------------------------------------------
	//! The map's rivers. The engine lists them across every loaded world (in Workbench that can be the
	//! edited world too), so only the game world's are kept.
	protected static void GetRiverEntities(notnull array<RiverEntity> rivers)
	{
		array<RiverEntity> all = {};
		RiverEntity.GetExistingInstances(all);
		BaseWorld world = GetGame().GetWorld();
		foreach (RiverEntity river : all)
		{
			if (river && river.GetWorld() == world)
				rivers.Insert(river);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A river's centre line and its width at each point (5 m precision is plenty for 250 m squares).
	protected static bool RiverLine(RiverEntity river, notnull array<vector> points, notnull array<float> widths)
	{
		if (!river)
			return false;

		return river.GetCentralPolyline(points, widths, 5);
	}

	// ================================================================ Working it out
	//------------------------------------------------------------------------------------------------
	//! Ridges, valley floors, forests, fields and lakes from the read grid. Clears what was worked out
	//! before. Rivers with no river entities on the map: a valley line through two or more bridges.
	protected void WorkOutTerrain()
	{
		int cells = m_aBits.Count();
		for (int i = 0; i < cells; i++)
		{
			m_aBits[i] = m_aBits[i] & READ_MASK;
		}

		m_iValleyRivers = 0;
		MarkRidges();
		MarkValleys();
		m_iForests = MarkClusters(BIT_FOREST);
		m_iLakes = CountLakes();
		m_fLandKm2 = 0;
		for (int j = 0; j < cells; j++)
		{
			m_fLandKm2 += LandShare(j) * CELL * CELL / 1000000;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A ridge: dry, and 25 m above its neighbours or the highest square within 1 km (a square
	//! window of 4 squares each way, taken as a row pass then a column pass).
	protected void MarkRidges()
	{
		int cells = m_aBits.Count();
		array<float> rowMax = {};
		rowMax.Resize(cells);
		for (int gz = 0; gz < m_iNZ; gz++)
		{
			for (int gx = 0; gx < m_iNX; gx++)
			{
				float best = -float.MAX;
				for (int d = -RIDGE_REACH; d <= RIDGE_REACH; d++)
				{
					int j = CellIndex(gx + d, gz);
					if (j >= 0 && m_aHeight[j] > best)
						best = m_aHeight[j];
				}

				rowMax[gz * m_iNX + gx] = best;
			}
		}

		m_iRidges = 0;
		for (int i = 0; i < cells; i++)
		{
			if (!IsDry(i))
				continue;

			int cx = i % m_iNX;
			int cz = i / m_iNX;
			float top = -float.MAX;
			for (int e = -RIDGE_REACH; e <= RIDGE_REACH; e++)
			{
				int k = CellIndex(cx, cz + e);
				if (k >= 0 && rowMax[k] > top)
					top = rowMax[k];
			}

			if (m_aHeight[i] >= top || Relief(i) >= RIDGE_RELIEF)
			{
				m_aBits[i] = m_aBits[i] | BIT_RIDGE;
				m_iRidges++;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A valley floor: dry, and 15 m below both dry squares across it, one or two squares away (250
	//! or 500 m), in one of four directions. A valley line is three or more such squares touching.
	//! Measured: one square away finds no valley line on Cain or Arland; two find 6 and 1 (and 10 on
	//! Everon, against 4).
	protected void MarkValleys()
	{
		array<int> dxs = {1, 0, 1, 1};
		array<int> dzs = {0, 1, 1, -1};
		int cells = m_aBits.Count();
		for (int i = 0; i < cells; i++)
		{
			if (!IsDry(i))
				continue;

			int gx = i % m_iNX;
			int gz = i / m_iNX;
			bool floor = false;
			for (int d = 0; d < 4 && !floor; d++)
			{
				for (int reach = 1; reach <= VALLEY_REACH && !floor; reach++)
				{
					int a = CellIndex(gx - dxs[d] * reach, gz - dzs[d] * reach);
					int b = CellIndex(gx + dxs[d] * reach, gz + dzs[d] * reach);
					if (a < 0 || b < 0 || !IsDry(a) || !IsDry(b))
						continue;

					if (m_aHeight[i] + VALLEY_DEPTH <= m_aHeight[a] && m_aHeight[i] + VALLEY_DEPTH <= m_aHeight[b])
						floor = true;
				}
			}

			if (floor)
				m_aBits[i] = m_aBits[i] | BIT_VALLEY;
		}

		m_iValleyLines = MarkClusters(BIT_VALLEY);
	}

	//------------------------------------------------------------------------------------------------
	//! Groups of touching squares (8 ways). For BIT_FOREST: forest squares in groups of three or
	//! more get the bit. For BIT_VALLEY: squares in groups under three lose it. Returns the groups
	//! of three or more. With no river entities, a valley line through two or more bridges is a river.
	protected int MarkClusters(int bit)
	{
		int cells = m_aBits.Count();
		array<int> label = {};
		label.Resize(cells);
		for (int i = 0; i < cells; i++)
		{
			label[i] = -1;
		}

		int groups = 0;
		array<int> stack = {};
		array<int> members = {};
		for (int start = 0; start < cells; start++)
		{
			if (label[start] >= 0 || !InCluster(start, bit))
				continue;

			members.Clear();
			stack.Clear();
			stack.Insert(start);
			label[start] = start;
			while (!stack.IsEmpty())
			{
				int cur = stack[stack.Count() - 1];
				stack.Remove(stack.Count() - 1);
				members.Insert(cur);
				int gx = cur % m_iNX;
				int gz = cur / m_iNX;
				for (int dz = -1; dz <= 1; dz++)
				{
					for (int dx = -1; dx <= 1; dx++)
					{
						int n = CellIndex(gx + dx, gz + dz);
						if (n < 0 || label[n] >= 0 || !InCluster(n, bit))
							continue;

						label[n] = start;
						stack.Insert(n);
					}
				}
			}

			bool big = members.Count() >= 3;
			if (big)
				groups++;

			int bridges = 0;
			foreach (int m : members)
			{
				if (bit == BIT_FOREST && big)
					m_aBits[m] = m_aBits[m] | BIT_FOREST;
				else if (bit == BIT_VALLEY && !big)
					m_aBits[m] = m_aBits[m] & ~BIT_VALLEY;

				if (HasBit(m, BIT_BRIDGE))
					bridges++;
			}

			if (bit == BIT_VALLEY && big && m_iRivers == 0 && bridges >= 2)
			{
				m_iValleyRivers++;
				foreach (int r : members)
				{
					m_aBits[r] = m_aBits[r] | BIT_RIVER;
				}
			}
		}

		return groups;
	}

	//------------------------------------------------------------------------------------------------
	protected bool InCluster(int idx, int bit)
	{
		if (bit == BIT_FOREST)
			return GetTreeClass(idx) == TREES_FOREST && IsDry(idx);

		return HasBit(idx, bit);
	}

	//------------------------------------------------------------------------------------------------
	//! Lakes: groups of touching squares with a lake sample.
	protected int CountLakes()
	{
		int cells = m_aBits.Count();
		array<bool> seen = {};
		seen.Resize(cells);
		for (int i = 0; i < cells; i++)
		{
			seen[i] = false;
		}

		int lakes = 0;
		array<int> stack = {};
		for (int start = 0; start < cells; start++)
		{
			if (seen[start] || !HasBit(start, BIT_LAKE))
				continue;

			lakes++;
			stack.Clear();
			stack.Insert(start);
			seen[start] = true;
			while (!stack.IsEmpty())
			{
				int cur = stack[stack.Count() - 1];
				stack.Remove(stack.Count() - 1);
				int gx = cur % m_iNX;
				int gz = cur / m_iNX;
				for (int dz = -1; dz <= 1; dz++)
				{
					for (int dx = -1; dx <= 1; dx++)
					{
						int n = CellIndex(gx + dx, gz + dz);
						if (n < 0 || seen[n] || !HasBit(n, BIT_LAKE))
							continue;

						seen[n] = true;
						stack.Insert(n);
					}
				}
			}
		}

		return lakes;
	}

	// ================================================================ Wild land
	//------------------------------------------------------------------------------------------------
	//! Which squares are wild, which are fields and which hold a lair candidate, from where people
	//! are now. Wild: dry, and outside every settlement's radius + 100 m, the 300 m no man's land
	//! round every lived-in place (lived-in towns, holdings, safehouses, the lab), 700 m round the
	//! Long Table's farm, and the Perimeter's warning zone + 200 m. Cheap: circles are stamped on the
	//! squares they cover. Holdings and safehouses change, so later systems call it again.
	void RefreshWild()
	{
		int cells = m_aBits.Count();
		if (cells == 0)
			return;

		array<int> near = {};
		near.Resize(cells);
		for (int i = 0; i < cells; i++)
		{
			near[i] = 0;
			m_aBits[i] = m_aBits[i] & ~(BIT_WILD | BIT_FIELD | BIT_LAIR);
		}

		bool small = m_fLandKm2 < SMALL_MAP_KM2;
		float townGap = 600;
		float peopleGap = 800;
		if (small)
		{
			townGap = 400;
			peopleGap = 600;
		}

		// near: 1 not wild, 2 within 800 m of a settlement (fields), 4 too close for a lair
		RZ_WorldFacts facts = m_GM.GetFacts();
		foreach (int si, RZ_Settlement s : m_GM.GetSettlements())
		{
			bool livedIn = facts && facts.IsLivedIn(si);
			float keep = s.m_fRadius + 100;
			if (livedIn)
				keep = s.m_fRadius + 300;

			Stamp(near, s.m_vCenter, keep, 1);
			Stamp(near, s.m_vCenter, s.m_fRadius + 800, 2);
			Stamp(near, s.m_vCenter, townGap, 4);
			if (livedIn)
				Stamp(near, s.m_vCenter, Math.Max(peopleGap, s.m_fRadius + 300), 4);
		}

		array<vector> people = {};
		LivedInPlaces(people);
		foreach (vector p : people)
		{
			Stamp(near, p, 400, 1);
			Stamp(near, p, peopleGap, 4);
		}

		RZ_LongTable table = m_GM.GetLongTable();
		if (table)
		{
			RZ_ETableState state = table.GetState();
			if (state == RZ_ETableState.ACTIVE || state == RZ_ETableState.QUIET || state == RZ_ETableState.EXPOSED)
				Stamp(near, table.GetHousePos(), 700, 1 | 4);
		}

		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		m_fWildKm2 = 0;
		m_iFields = 0;
		m_iLairs = 0;
		m_aLairByKind.Clear();
		for (int k = 0; k < GetLairKindCount(); k++)
		{
			m_aLairByKind.Insert(0);
		}

		for (int c = 0; c < cells; c++)
		{
			if (!IsDry(c))
				continue;

			vector center = CellCenter(c);
			bool atLine = perimeter && perimeter.IsNearLineWithin(center, 200);
			if ((near[c] & 2) != 0 && (near[c] & 1) == 0 && GetTreeClass(c) == TREES_NONE && m_aRange[c] <= FIELD_RANGE && Math.AbsFloat(Relief(c)) < FIELD_RELIEF)
			{
				m_aBits[c] = m_aBits[c] | BIT_FIELD;
				m_iFields++;
			}

			if ((near[c] & 1) != 0 || atLine)
				continue;

			m_aBits[c] = m_aBits[c] | BIT_WILD;
			m_fWildKm2 += LandShare(c) * CELL * CELL / 1000000;
			if ((near[c] & 4) != 0)
				continue;

			int kind = -1;
			if (HasBit(c, BIT_FOREST))
				kind = 0;
			else if (GetMark(c) > 0)
				kind = GetMark(c);

			if (kind < 0)
				continue;

			m_aBits[c] = m_aBits[c] | BIT_LAIR;
			m_iLairs++;
			m_aLairByKind[kind] = m_aLairByKind[kind] + 1;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Holdings, safehouses and the lab: where people live (their towns are stamped as settlements).
	protected void LivedInPlaces(notnull array<vector> result)
	{
		if (m_GM.HasLab())
			result.Insert(m_GM.GetLabPosition());

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
		{
			for (int i = 0; i < houses.Count(); i++)
			{
				RZ_Safehouse h = houses.Get(i);
				if (h)
					result.Insert(h.m_vPos);
			}
		}

		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			foreach (RZ_Holding holding : territory.GetHoldings())
			{
				result.Insert(holding.m_vPos);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! ORs `bit` into every square whose centre is within radius of center.
	protected void Stamp(notnull array<int> near, vector center, float radius, int bit)
	{
		int gx0 = Math.Floor((center[0] - radius) / CELL) - m_iX0;
		int gx1 = Math.Floor((center[0] + radius) / CELL) - m_iX0;
		int gz0 = Math.Floor((center[2] - radius) / CELL) - m_iZ0;
		int gz1 = Math.Floor((center[2] + radius) / CELL) - m_iZ0;
		for (int gz = gz0; gz <= gz1; gz++)
		{
			for (int gx = gx0; gx <= gx1; gx++)
			{
				int idx = CellIndex(gx, gz);
				if (idx < 0)
					continue;

				float x = (m_iX0 + gx + 0.5) * CELL;
				float z = (m_iZ0 + gz + 0.5) * CELL;
				float dx = x - center[0];
				float dz = z - center[2];
				if (dx * dx + dz * dz <= radius * radius)
					near[idx] = near[idx] | bit;
			}
		}
	}

	// ================================================================ Cache
	//------------------------------------------------------------------------------------------------
	//! The map's file name without folders or extension ("Eden" for worlds/Eden/Eden.ent).
	protected static string WorldToken()
	{
		string ext;
		string file = FilePath.StripExtension(FilePath.StripPath(GetGame().GetWorldFile()), ext);
		if (file.IsEmpty())
			return "world";

		return file;
	}

	//------------------------------------------------------------------------------------------------
	//! A fingerprint of the terrain: 64 heights on an 8 x 8 lattice over the map, in decimetres. A
	//! changed terrain (a map update) gives another file name, so the grid is read again.
	protected int TerrainHash(vector mins, vector maxs)
	{
		int hash = 17;
		for (int iz = 0; iz < 8; iz++)
		{
			for (int ix = 0; ix < 8; ix++)
			{
				float x = mins[0] + (maxs[0] - mins[0]) * (ix + 0.5) / 8;
				float z = mins[2] + (maxs[2] - mins[2]) * (iz + 0.5) / 8;
				int h = RZ_Util.RoundToInt(TerrainY(x, z) * 10);
				hash = (hash * 31 + h) % 1000003;
			}
		}

		hash = (hash * 31 + m_iNX * 1000 + m_iNZ) % 1000003;
		return Math.AbsInt(hash);
	}

	//------------------------------------------------------------------------------------------------
	protected string CachePath()
	{
		return string.Format("%1/land_%2_%3.json", RZ_Persistence.DIR, m_sWorld, m_iHash);
	}

	//------------------------------------------------------------------------------------------------
	protected bool LoadCache()
	{
		string path = CachePath();
		if (!FileIO.FileExists(path))
			return false;

		JsonLoadContext ctx = new JsonLoadContext();
		RZ_LandSave data = new RZ_LandSave();
		if (!ctx.LoadFromFile(path) || !ctx.ReadValue("land", data))
		{
			RZ_Log.Warn("Land survey: the cached grid " + path + " doesn't read; reading the map again.");
			return false;
		}

		int cells = m_iNX * m_iNZ;
		if (data.version != SAVE_VERSION || data.hash != m_iHash || data.nx != m_iNX || data.nz != m_iNZ || data.x0 != m_iX0 || data.z0 != m_iZ0)
			return false;

		if (!data.height || data.height.Count() != cells || !data.bits || data.bits.Count() != cells || !data.range || data.range.Count() != cells
			|| !data.trees || data.trees.Count() != cells || !data.buildings || data.buildings.Count() != cells
			|| !data.road || data.road.Count() != cells || !data.river || data.river.Count() != cells)
			return false;

		m_aHeight.Resize(cells);
		for (int i = 0; i < cells; i++)
		{
			m_aHeight[i] = data.height[i] * 0.1;
		}

		m_aRange.Copy(data.range);
		m_aBits.Copy(data.bits);
		m_aTrees.Copy(data.trees);
		m_aBuildings.Copy(data.buildings);
		m_aRoad.Copy(data.road);
		m_aRiver.Copy(data.river);
		m_aCross.Clear();
		m_aCrossKind.Clear();
		if (data.crossX && data.crossZ && data.crossKind)
		{
			for (int c = 0; c < data.crossKind.Count() && c < data.crossX.Count() && c < data.crossZ.Count(); c++)
			{
				float x = data.crossX[c];
				float z = data.crossZ[c];
				m_aCross.Insert(Vector(x, TerrainY(x, z), z));
				m_aCrossKind.Insert(data.crossKind[c]);
			}
		}

		m_iRivers = data.rivers;
		m_fRiverKm = data.riverKm;
		m_iPonds = data.ponds;
		m_iRoads = data.roads;
		m_iReadMs = data.readMs;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void SaveCache()
	{
		RZ_LandSave data = new RZ_LandSave();
		data.version = SAVE_VERSION;
		data.world = m_sWorld;
		data.hash = m_iHash;
		data.x0 = m_iX0;
		data.z0 = m_iZ0;
		data.nx = m_iNX;
		data.nz = m_iNZ;
		data.readMs = m_iReadMs;
		foreach (int i, float h : m_aHeight)
		{
			data.height.Insert(RZ_Util.RoundToInt(h * 10));
			data.bits.Insert(m_aBits[i] & READ_MASK);
		}

		data.range.Copy(m_aRange);
		data.trees.Copy(m_aTrees);
		data.buildings.Copy(m_aBuildings);
		data.road.Copy(m_aRoad);
		data.river.Copy(m_aRiver);
		foreach (int c, vector pos : m_aCross)
		{
			data.crossX.Insert(pos[0]);
			data.crossZ.Insert(pos[2]);
			data.crossKind.Insert(m_aCrossKind[c]);
		}

		data.rivers = m_iRivers;
		data.riverKm = m_fRiverKm;
		data.ponds = m_iPonds;
		data.roads = m_iRoads;

		FileIO.MakeDirectory(RZ_Persistence.DIR);
		JsonSaveContext ctx = new JsonSaveContext();
		ctx.WriteValue("land", data);
		if (!ctx.SaveToFile(CachePath()))
			RZ_Log.Warn("Land survey: could not write " + CachePath() + "; the map is read again at the next start.");
	}

	// ================================================================ Log
	//------------------------------------------------------------------------------------------------
	protected void LogSummary()
	{
		string how = string.Format("read in %1 s (%2 ms of work, at most %3 ms a second)",
			RZ_Util.RoundToInt((System.GetTickCount() - m_iStartedAt) / 1000.0), m_iReadMs, m_iMaxStepMs);
		if (m_bFromCache)
			how = string.Format("loaded from the cache in %1 ms", m_iLoadMs);

		RZ_Log.Info(string.Format("Land: %1 squares of 250 m (%2 x %3), %4 km2 of land, %5 km2 wild; %6, %7 ms to work out. File %8.",
			GetCellCount(), m_iNX, m_iNZ, Tenths(m_fLandKm2), Tenths(m_fWildKm2), how, m_iWorkMs, CachePath()));

		string rivers = "rivers: none found (the sea only; borders follow ridges and valley floors)";
		if (m_iRivers > 0)
			rivers = string.Format("%1 rivers (%2 km)", m_iRivers, Tenths(m_fRiverKm));
		else if (m_iValleyRivers > 0)
			rivers = string.Format("rivers: %1 inferred (valleys through two or more bridges)", m_iValleyRivers);

		string roads = string.Format("%1 roads, %2 bridges inferred (%3 over water), %4 fords", m_iRoads,
			CountCrossings(CROSS_DIP) + CountCrossings(CROSS_WATER), CountCrossings(CROSS_WATER), CountCrossings(CROSS_FORD));
		if (m_iRoads == 0)
			roads = "roads: none found (no road network; no bridges)";

		RZ_Log.Info(string.Format("Land: the sea, %1, %2 lakes. %3. Ridge posts %4, valley lines %5, forests %6, fields %7.",
			rivers, m_iLakes, roads, m_iRidges, m_iValleyLines, m_iForests, m_iFields));

		string kinds = "";
		for (int k = 0; k < GetLairKindCount(); k++)
		{
			int n = GetLairCountByKind(k);
			if (n <= 0)
				continue;

			if (!kinds.IsEmpty())
				kinds += ", ";

			kinds += string.Format("%1 %2", GetLairKindName(k), n);
		}

		if (kinds.IsEmpty())
			kinds = "none";

		int slots = SlotsAt(6);
		RZ_Log.Info(string.Format("Land: lair candidates %1 (%2). Tribe slots at 1 per 6 km2 of wild: %3 (%4 kept for a split).",
			m_iLairs, kinds, slots, KeptForSplits(slots)));
	}

	//------------------------------------------------------------------------------------------------
	//! "31.2"
	protected static string Tenths(float value)
	{
		int t = RZ_Util.RoundToInt(value * 10);
		return string.Format("%1.%2", t / 10, Math.AbsInt(t % 10));
	}
}
