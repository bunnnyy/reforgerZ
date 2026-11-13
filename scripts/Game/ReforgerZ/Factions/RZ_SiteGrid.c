//! A site's ground on a grid (docs/SITES.md, "The site grid"): 0.5 m cells over the reach of a
//! camp, read once when the site is first dressed, a little each second, and freed once its layout
//! is worked out. Each cell carries flags (water, steep, a building, something solid, a road, a
//! piece...) so a piece's whole footprint is tested, not a few points. Server only.

//------------------------------------------------------------------------------------------------
//! A piece's footprint (docs/SITES.md, "Real footprints"): its props' boxes (up to 8 rectangles)
//! and a person round each seat, in the piece's own space, from RZ_DressingPiece.m_sFoot.
class RZ_SiteFoot
{
	static const float SEAT = 0.6;		// a seat's person: a 0.6 m square

	ref array<float> m_aRects = {};		// centre x, centre z, width, depth, turn (degrees): five per rectangle
	int m_iProps;						// the first m_iProps rectangles are props; the rest are seats

	//------------------------------------------------------------------------------------------------
	//! "x z w d yaw; ...; s x z" (written by tools/camp_kit/build.py). Empty: one box from fmin to fmax.
	static RZ_SiteFoot Parse(string text, vector fmin, vector fmax)
	{
		RZ_SiteFoot foot = new RZ_SiteFoot();
		array<string> items = {};
		text.Split(";", items, true);
		array<float> seats = {};
		foreach (string item : items)
		{
			item.Trim();
			array<string> bits = {};
			item.Split(" ", bits, true);
			if (bits.Count() == 3 && bits[0] == "s")
			{
				seats.Insert(bits[1].ToFloat());
				seats.Insert(bits[2].ToFloat());
				continue;
			}

			if (bits.Count() < 5)
				continue;

			for (int k = 0; k < 5; k++)
			{
				foot.m_aRects.Insert(bits[k].ToFloat());
			}

			foot.m_iProps++;
		}

		if (foot.m_iProps == 0)
		{
			foot.m_aRects.Insert((fmin[0] + fmax[0]) * 0.5);
			foot.m_aRects.Insert((fmin[2] + fmax[2]) * 0.5);
			foot.m_aRects.Insert(Math.Max(0.1, fmax[0] - fmin[0]));
			foot.m_aRects.Insert(Math.Max(0.1, fmax[2] - fmin[2]));
			foot.m_aRects.Insert(0);
			foot.m_iProps = 1;
		}

		for (int s = 0; s + 1 < seats.Count(); s += 2)
		{
			foot.m_aRects.Insert(seats[s]);
			foot.m_aRects.Insert(seats[s + 1]);
			foot.m_aRects.Insert(SEAT);
			foot.m_aRects.Insert(SEAT);
			foot.m_aRects.Insert(0);
		}

		return foot;
	}

	//------------------------------------------------------------------------------------------------
	int Count() { return m_aRects.Count() / 5; }
	bool IsSeat(int r) { return r >= m_iProps; }

	//------------------------------------------------------------------------------------------------
	//! Rectangle r of the piece standing at pos turned by yaw: its four corners on the ground plane.
	void Corners(int r, vector pos, float yaw, float grow, notnull array<vector> quad)
	{
		int i = r * 5;
		vector c = RZ_SiteGrid.Frame(pos, yaw, m_aRects[i], m_aRects[i + 1]);
		float turn = yaw + m_aRects[i + 4];
		float hw = m_aRects[i + 2] * 0.5 + grow;
		float hd = m_aRects[i + 3] * 0.5 + grow;
		quad.Insert(RZ_SiteGrid.Frame(c, turn, -hw, -hd));
		quad.Insert(RZ_SiteGrid.Frame(c, turn, hw, -hd));
		quad.Insert(RZ_SiteGrid.Frame(c, turn, hw, hd));
		quad.Insert(RZ_SiteGrid.Frame(c, turn, -hw, hd));
	}

	//------------------------------------------------------------------------------------------------
	//! "x z w d yaw" of rectangle r in the piece's own space, for the audit's drawing.
	string RectText(int r)
	{
		int i = r * 5;
		return string.Format("%1 %2 %3 %4 %5", Round2(m_aRects[i]), Round2(m_aRects[i + 1]), Round2(m_aRects[i + 2]), Round2(m_aRects[i + 3]), RZ_Util.RoundToInt(m_aRects[i + 4]));
	}

	//------------------------------------------------------------------------------------------------
	static float Round2(float v)
	{
		return Math.Round(v * 100) / 100;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_SiteGrid
{
	static const float CELL = 0.5;

	// Cell flags (two cells to an int, 16 bits each)
	static const int WATER = 1;
	static const int SHORE = 2;		// within 4 m of water
	static const int STEEP = 4;		// over 12 degrees
	static const int STEP = 8;		// a jump over 0.3 m (a bank)
	static const int INSIDE = 16;	// in a building's box
	static const int WALL = 32;		// within 0.4 m of a building
	static const int DOOR = 64;		// by a building's outside door
	static const int WINDOW = 128;	// by a building's window
	static const int SOLID = 256;	// a wall, fence, hedge, tree trunk, rock, car, wreck, prop
	static const int SOFT = 512;	// a bush: a prop may stand on it, a path may not
	static const int ROAD = 1024;
	static const int VERGE = 2048;	// within 2 m of a road
	static const int KEPT = 4096;	// another system's scenery (Traces' scenes and boards)
	static const int TAKEN = 8192;	// one of the site's own pieces
	static const int OUTSIDE = 16384;	// off the grid (not a cell flag: what Blocks says)
	static const int CLEAR = 32768;	// kept clear by the solver: a path from the door to the gate or the kitchen

	// What a piece may not stand on: water, steep, a step, a building, solid, road, kept, taken
	static const int BLOCKS = 13597;
	// What a path may not cross: water, a building, solid, soft, kept, taken
	static const int NO_WALK = 13073;
	// What work and life may not stand on: BLOCKS, the shore (within 4 m of water) and the paths kept clear
	static const int BLOCKS_LIFE = 46367;

	protected static const float STEEP_RISE = 0.2126;	// tan 12 degrees, per metre
	protected static const float STEP_RISE = 0.3;
	protected static const float WALL_BAND = 0.4;
	protected static const float VERGE_BAND = 2;
	protected static const float DEFAULT_ROAD = 6;
	protected static const float SHORE_BAND = 4;
	protected static const float OVERHEAD = 2;			// an entity whose bottom is this far up is overhead (a wire, a lamp)
	protected static const float FLAT_THING = 0.12;		// one lower than this is flat (a decal, a kerb's top)
	protected static const float TOO_BIG = 60;
	protected static const float QUERY_TILE = 24;		// the entities are read a square this size at a time			// one wider than this isn't a prop (terrain, a whole field)
	protected static const float TRUNK = 0.5;			// a tree's trunk disc radius
	protected static const float ROCK_SHARE = 0.6;		// a big rock: an ellipse this share of its box's half-sizes

	// Reading stages
	protected static const int READ_HEIGHTS = 0;
	protected static const int READ_QUERY = 1;
	protected static const int READ_ENTITIES = 2;
	protected static const int READ_BUILDINGS = 3;
	protected static const int READ_ROADS = 4;
	protected static const int READ_WATER = 5;
	protected static const int READ_SLOPE = 6;
	protected static const int READ_SHORE = 7;
	protected static const int READ_DONE = 8;

	vector m_vCentre;
	vector m_vLow;				// the grid's low corner (x, z)
	float m_fHalf;
	int m_iN;					// cells a side
	int m_iL;					// height samples a side (1 m apart)
	ref array<int> m_aFlags = {};
	ref array<float> m_aHeight = {};
	ref array<int> m_aWet = {};		// per height sample: water there
	int m_iReadMs;
	int m_iSteps;
	int m_iMaxStepMs;
	int m_iMaxUnitMs;			// the longest single piece of reading (a row, a tile's query, 16 entities...)
	int m_iEntities;			// static and dynamic entities read
	int m_iRasterised;			// of those, drawn into the grid
	ref array<vector> m_aJetty = {};	// piers, jetties, docks and slipways on the grid (boats are a way in)
	ref array<vector> m_aDoors = {};	// every building's outside doors on the grid (the solver's harden layer)
	ref array<vector> m_aWindows = {};	// and their ground-floor windows
	ref array<string> m_aBig = {};		// the audit's drawing: the big solid things read (class, prefab, size)...
	ref array<int> m_aBigCount = {};	// ...and how many of each
	static bool s_bNameBig;				// the audit (the runtime test) names them

	protected int m_iStage;
	protected int m_iNext;
	protected ref array<IEntity> m_aEnts = {};
	protected ref set<IEntity> m_sSeen = new set<IEntity>();
	protected ref array<int> m_aDist = {};			// the shore's distances while they're worked out
	protected ref array<IEntity> m_aBuildings = {};
	protected ref array<vector> m_aRoad = {};
	protected ref array<float> m_aRoadWidth = {};
	protected int m_iWetStep;			// inland water sampled every this many metres (0 = not at all)

	//------------------------------------------------------------------------------------------------
	//! A grid `half` m each way round `centre`, to be read from the buildings and road segments given.
	void RZ_SiteGrid(vector centre, float half, array<IEntity> buildings, array<vector> road, array<float> roadWidth)
	{
		m_vCentre = centre;
		m_fHalf = half;
		m_iN = 4 * RZ_Util.RoundToInt(half);	// 0.5 m cells, an even count
		m_iL = m_iN / 2 + 1;
		m_vLow = Vector(centre[0] - m_iN * CELL * 0.5, 0, centre[2] - m_iN * CELL * 0.5);
		int words = (m_iN * m_iN + 1) / 2;
		m_aFlags.Resize(words);
		for (int w = 0; w < words; w++)
		{
			m_aFlags[w] = 0;
		}

		int samples = m_iL * m_iL;
		m_aHeight.Resize(samples);
		m_aWet.Resize(samples);
		for (int s = 0; s < samples; s++)
		{
			m_aHeight[s] = 0;
			m_aWet[s] = 0;
		}

		if (buildings)
			m_aBuildings.InsertAll(buildings);

		if (road)
			m_aRoad.InsertAll(road);

		if (roadWidth)
			m_aRoadWidth.InsertAll(roadWidth);
	}

	//------------------------------------------------------------------------------------------------
	bool IsRead() { return m_iStage >= READ_DONE; }
	array<IEntity> GetBuildings() { return m_aBuildings; }

	//------------------------------------------------------------------------------------------------
	//! Reads on for up to `budgetMs`; true once the whole grid is read.
	bool Step(int budgetMs)
	{
		int t0 = System.GetTickCount();
		while (m_iStage < READ_DONE && System.GetTickCount() - t0 < budgetMs)
		{
			int u0 = System.GetTickCount();
			StepOnce();
			m_iMaxUnitMs = Math.Max(m_iMaxUnitMs, System.GetTickCount() - u0);
		}

		int spent = System.GetTickCount() - t0;
		m_iReadMs += spent;
		m_iSteps++;
		if (spent > m_iMaxStepMs)
			m_iMaxStepMs = spent;

		return m_iStage >= READ_DONE;
	}

	//------------------------------------------------------------------------------------------------
	//! One small unit of reading: a row of heights, a few entities, one building...
	protected void StepOnce()
	{
		switch (m_iStage)
		{
			case READ_HEIGHTS:
			{
				ReadHeightRow(m_iNext);
				m_iNext++;
				if (m_iNext >= m_iL)
					NextStage();

				return;
			}
			case READ_QUERY:
			{
				// One 24 m tile a step, so no step holds the server long; a thing over two tiles is read once
				int tiles = Math.Ceil(m_iN * CELL / QUERY_TILE);
				int tx = m_iNext % tiles;
				int tz = m_iNext / tiles;
				vector lo = Vector(m_vLow[0] + tx * QUERY_TILE, -1000, m_vLow[2] + tz * QUERY_TILE);
				vector hi = Vector(lo[0] + QUERY_TILE, 10000, lo[2] + QUERY_TILE);
				GetGame().GetWorld().QueryEntitiesByAABB(lo, hi, OnQueryEntity, null, EQueryEntitiesFlags.ALL);
				m_iNext++;
				if (m_iNext >= tiles * tiles)
				{
					m_iEntities = m_aEnts.Count();
					m_sSeen.Clear();
					NextStage();
				}

				return;
			}
			case READ_ENTITIES:
			{
				for (int k = 0; k < 16 && m_iNext < m_aEnts.Count(); k++)
				{
					AddEntity(m_aEnts[m_iNext]);
					m_iNext++;
				}

				if (m_iNext >= m_aEnts.Count())
				{
					m_aEnts.Clear();
					NextStage();
				}

				return;
			}
			case READ_BUILDINGS:
			{
				if (m_iNext < m_aBuildings.Count())
					AddBuilding(m_aBuildings[m_iNext]);

				m_iNext++;
				if (m_iNext >= m_aBuildings.Count())
					NextStage();

				return;
			}
			case READ_ROADS:
			{
				for (int r = 0; r < 8; r++)
				{
					int seg = m_iNext * 2;
					if (seg + 1 >= m_aRoad.Count())
						break;

					float width = DEFAULT_ROAD;
					if (m_iNext < m_aRoadWidth.Count() && m_aRoadWidth[m_iNext] > 1)
						width = m_aRoadWidth[m_iNext];

					AddRoad(m_aRoad[seg], m_aRoad[seg + 1], width);
					m_iNext++;
				}

				if (m_iNext * 2 + 1 >= m_aRoad.Count())
					NextStage();

				return;
			}
			case READ_WATER:
			{
				ReadWaterRow(m_iNext);
				m_iNext++;
				if (m_iNext >= m_iL)
					NextStage();

				return;
			}
			case READ_SLOPE:
			{
				ReadSlopeRow(m_iNext);
				m_iNext++;
				if (m_iNext >= m_iL - 1)
					NextStage();

				return;
			}
			case READ_SHORE:
			{
				// Three passes of rows: down, up, then the cells
				if (!ShoreRow(m_iNext))
				{
					m_aDist.Clear();
					NextStage();
					return;
				}

				m_iNext++;
				return;
			}
		}

		m_iStage = READ_DONE;
	}

	//------------------------------------------------------------------------------------------------
	protected void NextStage()
	{
		m_iStage++;
		m_iNext = 0;
		if (m_iStage == READ_WATER)
			m_iWetStep = InlandStep();
	}

	// ================================================================ Cells
	//------------------------------------------------------------------------------------------------
	int Flags(int cx, int cz)
	{
		if (cx < 0 || cz < 0 || cx >= m_iN || cz >= m_iN)
			return OUTSIDE;

		int i = cz * m_iN + cx;
		int shift = (i & 1) * 16;
		return (m_aFlags[i >> 1] >> shift) & 65535;
	}

	//------------------------------------------------------------------------------------------------
	void SetFlag(int cx, int cz, int flag)
	{
		if (cx < 0 || cz < 0 || cx >= m_iN || cz >= m_iN)
			return;

		int i = cz * m_iN + cx;
		int shift = (i & 1) * 16;
		m_aFlags[i >> 1] = m_aFlags[i >> 1] | (flag << shift);
	}

	//------------------------------------------------------------------------------------------------
	int CellX(float x) { return Math.Floor((x - m_vLow[0]) / CELL); }
	int CellZ(float z) { return Math.Floor((z - m_vLow[2]) / CELL); }
	float CellCentreX(int cx) { return m_vLow[0] + (cx + 0.5) * CELL; }
	float CellCentreZ(int cz) { return m_vLow[2] + (cz + 0.5) * CELL; }

	//------------------------------------------------------------------------------------------------
	int FlagsAt(vector pos)
	{
		return Flags(CellX(pos[0]), CellZ(pos[2]));
	}

	//------------------------------------------------------------------------------------------------
	//! The ground's height at a point inside the grid, from the 1 m samples.
	float HeightAt(float x, float z)
	{
		float fx = Math.Clamp(x - m_vLow[0], 0, m_iL - 1.001);
		float fz = Math.Clamp(z - m_vLow[2], 0, m_iL - 1.001);
		int lx = Math.Floor(fx);
		int lz = Math.Floor(fz);
		float u = fx - lx;
		float v = fz - lz;
		int i = lz * m_iL + lx;
		float h0 = m_aHeight[i] * (1 - u) + m_aHeight[i + 1] * u;
		float h1 = m_aHeight[i + m_iL] * (1 - u) + m_aHeight[i + m_iL + 1] * u;
		return h0 * (1 - v) + h1 * v;
	}

	//------------------------------------------------------------------------------------------------
	//! A point `x` right and `z` ahead of `origin` facing `bearing` (as RZ_CampLife.FramePoint).
	static vector Frame(vector origin, float bearing, float x, float z)
	{
		float rad = bearing * Math.DEG2RAD;
		float s = Math.Sin(rad);
		float c = Math.Cos(rad);
		vector pt = origin;
		pt[0] = origin[0] + x * c + z * s;
		pt[2] = origin[2] - x * s + z * c;
		return pt;
	}

	// ================================================================ Footprints
	//------------------------------------------------------------------------------------------------
	//! The flags in `mask` found under a footprint (OUTSIDE if any of it is off the grid), and the
	//! ground's lowest and highest point under it. 0 = it may stand there.
	int Blocks(vector pos, float yaw, RZ_SiteFoot foot, int mask, out float lo, out float hi)
	{
		lo = float.MAX;
		hi = -float.MAX;
		int found = 0;
		for (int r = 0; r < foot.Count(); r++)
		{
			found = found | RectCells(pos, yaw, foot, r, 0.2, mask, 0, lo, hi);
		}

		return found;
	}

	//------------------------------------------------------------------------------------------------
	//! As Blocks, but stops at the first rectangle that meets a flag in `mask` (the solver tries many
	//! spots: most fail early). 0 = it may stand there, with the ground's range under it.
	int FirstBlock(vector pos, float yaw, RZ_SiteFoot foot, int mask, out float lo, out float hi)
	{
		lo = float.MAX;
		hi = -float.MAX;
		for (int r = 0; r < foot.Count(); r++)
		{
			int found = RectCells(pos, yaw, foot, r, 0.2, mask, 0, lo, hi);
			if (found != 0)
				return found;
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! How much the ground varies under one of a footprint's props (rectangle r, the cells whose
	//! middle is in it): the audit's floating test.
	float RectSpread(vector pos, float yaw, RZ_SiteFoot foot, int r)
	{
		float lo = float.MAX;
		float hi = -float.MAX;
		RectCells(pos, yaw, foot, r, 0, 0, 0, lo, hi);
		if (lo > hi)
			return 0;

		return hi - lo;
	}

	//------------------------------------------------------------------------------------------------
	//! Marks a placed footprint's cells (grown by a cell, so pieces keep a gap) with `flag`.
	void Paint(vector pos, float yaw, RZ_SiteFoot foot, int flag)
	{
		float lo;
		float hi;
		for (int r = 0; r < foot.Count(); r++)
		{
			RectCells(pos, yaw, foot, r, CELL, 0, flag, lo, hi);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Walks the cells under rectangle r (grown by `grow`): ORs together their flags in `mask` and
	//! sets `paint` on them; tracks the ground's height range there.
	protected int RectCells(vector pos, float yaw, RZ_SiteFoot foot, int r, float grow, int mask, int paint, inout float lo, inout float hi)
	{
		int i = r * 5;
		vector c = Frame(pos, yaw, foot.m_aRects[i], foot.m_aRects[i + 1]);
		float turn = (yaw + foot.m_aRects[i + 4]) * Math.DEG2RAD;
		float sn = Math.Sin(turn);
		float cs = Math.Cos(turn);
		float hw = foot.m_aRects[i + 2] * 0.5 + grow;
		float hd = foot.m_aRects[i + 3] * 0.5 + grow;
		float ex = Math.AbsFloat(cs) * hw + Math.AbsFloat(sn) * hd;
		float ez = Math.AbsFloat(sn) * hw + Math.AbsFloat(cs) * hd;
		int x0 = CellX(c[0] - ex);
		int x1 = CellX(c[0] + ex);
		int z0 = CellZ(c[2] - ez);
		int z1 = CellZ(c[2] + ez);
		int found = 0;
		for (int cz = z0; cz <= z1; cz++)
		{
			float dz = CellCentreZ(cz) - c[2];
			for (int cx = x0; cx <= x1; cx++)
			{
				float dx = CellCentreX(cx) - c[0];
				float lx = dx * cs - dz * sn;
				float lz = dx * sn + dz * cs;
				if (Math.AbsFloat(lx) > hw || Math.AbsFloat(lz) > hd)
					continue;

				if (paint != 0)
				{
					// A piece's gap round it never closes a path kept clear
					if (paint != TAKEN || (Flags(cx, cz) & CLEAR) == 0 || (Math.AbsFloat(lx) <= hw - grow && Math.AbsFloat(lz) <= hd - grow))
						SetFlag(cx, cz, paint);

					continue;
				}

				int f = Flags(cx, cz);
				found = found | (f & (mask | OUTSIDE));
				if (f == OUTSIDE)
					continue;

				float h = HeightAt(CellCentreX(cx), CellCentreZ(cz));
				lo = Math.Min(lo, h);
				hi = Math.Max(hi, h);
			}
		}

		return found;
	}

	//------------------------------------------------------------------------------------------------
	//! Flag names for a drop's reason and the log.
	static string FlagName(int flags)
	{
		if (flags & OUTSIDE)
			return "outside the ground read";

		if (flags & WATER)
			return "water";

		if (flags & INSIDE)
			return "a building";

		if (flags & ROAD)
			return "the road";

		if (flags & STEP)
			return "a bank";

		if (flags & STEEP)
			return "steep ground";

		if (flags & SOLID)
			return "something standing there";

		if (flags & KEPT)
			return "other scenery";

		if (flags & TAKEN)
			return "another piece";

		if (flags & CLEAR)
			return "a path kept clear";

		return "the ground";
	}

	//------------------------------------------------------------------------------------------------
	//! A paved surface at a spot on the ground (a road's or a yard's cobbles, asphalt, concrete,
	//! paving, tiles, brick, stone, metal): the material a downward trace hits, by its name. `seen`
	//! counts the names read (the solver's "Sites ground" log line).
	static bool PavedAt(vector at, map<string, int> seen = null)
	{
		TraceParam param = new TraceParam();
		param.Start = Vector(at[0], at[1] + 1.2, at[2]);
		param.End = Vector(at[0], at[1] - 1.5, at[2]);
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		float frac = GetGame().GetWorld().TraceMove(param, null);
		if (frac >= 1)
			return false;

		string name = param.TraceMaterial; // VERIFY: a downward trace's TraceMaterial names the surface hit (the terrain's layer under a road or a yard, a paving mesh's material), with a word that tells paved ground from soft; seen on Everon: grass_01 to grass_03, dirt_01, cobblestone_01_wave (.emat)
		if (name.IsEmpty())
			return false;

		name.ToLower();
		int slash = name.LastIndexOf("/");
		if (slash >= 0)
			name = name.Substring(slash + 1, name.Length() - slash - 1);

		if (seen)
			seen.Set(name, seen.Get(name) + 1);

		array<string> hard = {"asphalt", "concrete", "cobble", "pavement", "paving", "paved", "tile", "brick", "tarmac", "stone", "metal", "road"};
		foreach (string word : hard)
		{
			if (name.Contains(word))
				return true;
		}

		return false;
	}

	// ================================================================ Reading
	//------------------------------------------------------------------------------------------------
	protected void ReadHeightRow(int lz)
	{
		BaseWorld world = GetGame().GetWorld();
		float z = m_vLow[2] + lz;
		int row = lz * m_iL;
		for (int lx = 0; lx < m_iL; lx++)
		{
			m_aHeight[row + lx] = world.GetSurfaceY(m_vLow[0] + lx, z);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnQueryEntity(IEntity e)
	{
		if (e && !m_sSeen.Contains(e))
		{
			m_sSeen.Insert(e);
			m_aEnts.Insert(e);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Not part of the ground: people, terrain, roads, rivers, decals, lights, shapes, the site's own buildings.
	protected bool Skips(IEntity e)
	{
		if (ChimeraCharacter.Cast(e) || GenericTerrainEntity.Cast(e) || RoadEntity.Cast(e) || RiverEntity.Cast(e))
			return true;

		if (DecalEntity.Cast(e) || LightEntity.Cast(e) || ShapeEntity.Cast(e) || PowerlineEntity.Cast(e) || GeneratorBaseEntity.Cast(e))
			return true;

		return m_aBuildings.Contains(e);
	}

	//------------------------------------------------------------------------------------------------
	//! One entity on the grid: a tree's trunk, a bush as soft ground, other scenery as kept, the rest
	//! (walls, fences, hedges, rocks, cars, wrecks, props) as solid, by its turned box.
	protected void AddEntity(IEntity e)
	{
		if (!e || Skips(e))
			return;

		vector wmin;
		vector wmax;
		e.GetWorldBounds(wmin, wmax);
		if (wmax[0] - wmin[0] > TOO_BIG || wmax[2] - wmin[2] > TOO_BIG)
			return;

		if (wmax[0] - wmin[0] < 0.05 && wmax[2] - wmin[2] < 0.05)
			return;

		vector at = e.GetOrigin();
		if (at[0] < m_vLow[0] - 5 || at[2] < m_vLow[2] - 5 || at[0] > m_vLow[0] + m_iN * CELL + 5 || at[2] > m_vLow[2] + m_iN * CELL + 5)
		{
			// Its middle is off the grid: only a big thing reaches in
			if (wmax[0] - wmin[0] < 10 && wmax[2] - wmin[2] < 10)
				return;
		}

		// A pier or a jetty: a big thing named so
		if ((wmax[0] - wmin[0] > 4 || wmax[2] - wmin[2] > 4) && m_aJetty.Count() < 4)
		{
			string big = RZ_Util.GetPrefabName(e);
			big.ToLower();
			if (big.Contains("pier") || big.Contains("jetty") || big.Contains("dock") || big.Contains("slipway"))
				m_aJetty.Insert(at);
		}

		float ground = GetGame().GetWorld().GetSurfaceY(at[0], at[2]);
		if (wmin[1] - ground > OVERHEAD || wmax[1] - ground < FLAT_THING)
			return;

		vector mins;
		vector maxs;
		e.GetBounds(mins, maxs);
		if (maxs[1] - mins[1] < FLAT_THING)
			return;

		int flag = SOLID;
		if (RZ_CampLife.IsDressing(e))
			flag = KEPT;

		BaseTree tree = BaseTree.Cast(e);
		if (tree)
		{
			string name = RZ_Util.GetPrefabName(e);
			name.ToLower();
			if (name.Contains("/tree"))
			{
				Disc(at, TRUNK, SOLID);
				m_iRasterised++;
				return;
			}

			if (!name.Contains("hedge"))
				flag = SOFT;
		}

		// Paving is ground to stand on; a big rock is round and half buried in the ground (its box
		// isn't its shape): an ellipse inside its box, and the traces see the rest
		bool round = false;
		if ((maxs[0] - mins[0]) * (maxs[2] - mins[2]) > 4)
		{
			string kind = RZ_Util.GetPrefabName(e);
			kind.ToLower();
			if (kind.Contains("pavement") || kind.Contains("paving"))
				return;

			round = SCR_IndestructibleEnvironmentalEntity.Cast(e) != null || kind.Contains("granite") || kind.Contains("rock") || kind.Contains("boulder") || kind.Contains("stone") || kind.Contains("cliff");
		}

		vector mat[4];
		e.GetWorldTransform(mat);
		vector right = mat[0];
		vector fwd = mat[2];
		right[1] = 0;
		fwd[1] = 0;
		if (right.Length() < 0.01 || fwd.Length() < 0.01)
			return;

		vector c = mat[3] + mat[0] * ((mins[0] + maxs[0]) * 0.5) + mat[2] * ((mins[2] + maxs[2]) * 0.5);
		float yaw = RZ_Compass.Bearing(vector.Zero, fwd);
		if (s_bNameBig && flag == SOLID && (maxs[0] - mins[0]) * (maxs[2] - mins[2]) > 6)
		{
			string big = string.Format("%1 %2 %3x%4x%5", e.ClassName(), FilePath.StripPath(RZ_Util.GetPrefabName(e)), RZ_Util.RoundToInt(maxs[0] - mins[0]), RZ_Util.RoundToInt(maxs[2] - mins[2]), RZ_Util.RoundToInt(maxs[1] - mins[1]));
			int seen = m_aBig.Find(big);
			if (seen >= 0)
				m_aBigCount[seen] = m_aBigCount[seen] + 1;
			else if (m_aBig.Count() < 40)
			{
				m_aBig.Insert(big);
				m_aBigCount.Insert(1);
			}
		}

		float hw = (maxs[0] - mins[0]) * right.Length() * 0.5;
		float hd = (maxs[2] - mins[2]) * fwd.Length() * 0.5;
		if (round)
			Ellipse(c, yaw, hw * ROCK_SHARE, hd * ROCK_SHARE, flag);
		else
			Box(c, yaw, hw, hd, flag);

		m_iRasterised++;
	}

	//------------------------------------------------------------------------------------------------
	//! A building: inside its box, a wall band round it, its outside doors and its windows.
	protected void AddBuilding(IEntity b)
	{
		if (!b)
			return;

		vector mins;
		vector maxs;
		if (!RZ_DoorGeometry.LocalBox(b, mins, maxs))
		{
			Disc(b.GetOrigin(), 3, INSIDE);
			return;
		}

		vector mat[4];
		b.GetWorldTransform(mat);
		vector fwd = mat[2];
		fwd[1] = 0;
		float yaw = RZ_Compass.Bearing(vector.Zero, fwd);
		vector c = RZ_DoorGeometry.ToWorld(b, (mins + maxs) * 0.5);
		// A scaled building's box is scaled too (as the audit's boxes, which go through its transform)
		vector right = mat[0];
		right[1] = 0;
		float hw = (maxs[0] - mins[0]) * 0.5 * Math.Max(1, right.Length());
		float hd = (maxs[2] - mins[2]) * 0.5 * Math.Max(1, fwd.Length());
		// Every cell the box touches is inside (so no footprint reaches into it between cell middles)
		Box(c, yaw, hw + WALL_BAND + CELL * 0.5, hd + WALL_BAND + CELL * 0.5, WALL);
		Box(c, yaw, hw + CELL * 0.5, hd + CELL * 0.5, INSIDE);

		array<IEntity> doors = {};
		RZ_DoorGeometry.FindDoors(b, 0, doors);
		foreach (IEntity door : doors)
		{
			if (door && RZ_DoorGeometry.IsOuterDoor(b, door))
			{
				Disc(door.GetOrigin(), 1, DOOR);
				if (m_aDoors.Count() < 400)
					m_aDoors.Insert(door.GetOrigin());
			}
		}

		array<IEntity> windows = {};
		FindWindows(b, 0, windows);
		foreach (IEntity w : windows)
		{
			Disc(w.GetOrigin(), 0.6, WINDOW);
			if (m_aWindows.Count() < 600)
				m_aWindows.Insert(w.GetOrigin());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A building's window children, by prefab name, a few levels down.
	protected static void FindWindows(IEntity e, int depth, notnull array<IEntity> result)
	{
		if (!e || depth > 2 || result.Count() > 40)
			return;

		IEntity child = e.GetChildren();
		while (child)
		{
			string name = RZ_Util.GetPrefabName(child);
			name.ToLower();
			if (name.Contains("window")) // VERIFY: a building's ground-floor windows are children whose prefab names say "window"
			{
				if (child.GetOrigin()[1] - GetGame().GetWorld().GetSurfaceY(child.GetOrigin()[0], child.GetOrigin()[2]) < 3)
					result.Insert(child);
			}
			else
			{
				FindWindows(child, depth + 1, result);
			}

			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AddRoad(vector a, vector b, float width)
	{
		float half = width * 0.5;
		float reach = half + VERGE_BAND;
		int x0 = CellX(Math.Min(a[0], b[0]) - reach);
		int x1 = CellX(Math.Max(a[0], b[0]) + reach);
		int z0 = CellZ(Math.Min(a[2], b[2]) - reach);
		int z1 = CellZ(Math.Max(a[2], b[2]) + reach);
		x0 = Math.Max(x0, 0);
		z0 = Math.Max(z0, 0);
		x1 = Math.Min(x1, m_iN - 1);
		z1 = Math.Min(z1, m_iN - 1);
		for (int cz = z0; cz <= z1; cz++)
		{
			for (int cx = x0; cx <= x1; cx++)
			{
				vector p = Vector(CellCentreX(cx), 0, CellCentreZ(cz));
				float d = RZ_DressPlacer.DistToSegment(p, a, b);
				if (d <= half)
					SetFlag(cx, cz, ROAD);
				else if (d <= reach)
					SetFlag(cx, cz, VERGE);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! How often to look for inland water: every 2 m where the land grid says a river, a lake or a
	//! pond is in the square; never where it says there's none; every 4 m before it's read.
	protected int InlandStep()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		RZ_Land land;
		if (gm)
			land = gm.GetLand();

		if (!land || !land.IsReady())
			return 4;

		array<vector> corners = {m_vLow, Vector(m_vLow[0] + m_iN * CELL, 0, m_vLow[2]), Vector(m_vLow[0], 0, m_vLow[2] + m_iN * CELL), Vector(m_vLow[0] + m_iN * CELL, 0, m_vLow[2] + m_iN * CELL)};
		foreach (vector corner : corners)
		{
			int idx = land.CellAt(corner);
			if (idx < 0)
				continue;

			if (land.InlandSamples(idx) > 0 || land.HasBit(idx, RZ_Land.BIT_RIVER) || land.HasBit(idx, RZ_Land.BIT_LAKE))
				return 2;
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! One row of height samples: the sea (under 0.5 m), and inland water where it's looked for.
	protected void ReadWaterRow(int lz)
	{
		int row = lz * m_iL;
		for (int lx = 0; lx < m_iL; lx++)
		{
			if (m_aHeight[row + lx] < 0.5)
				m_aWet[row + lx] = 1;
		}

		if (m_iWetStep <= 0 || lz % m_iWetStep != 0)
			return;

		for (int sx = 0; sx < m_iL; sx += m_iWetStep)
		{
			vector ground = Vector(m_vLow[0] + sx, m_aHeight[row + sx], m_vLow[2] + lz);
			float reach;
			EWaterSurfaceType wet = RZ_Land.WaterAt(ground, reach);
			if (wet != EWaterSurfaceType.WST_POND && wet != EWaterSurfaceType.WST_RIVER) // the sea is read from the heights
				continue;

			for (int dz = 0; dz < m_iWetStep && lz + dz < m_iL; dz++)
			{
				for (int dx = 0; dx < m_iWetStep && sx + dx < m_iL; dx++)
				{
					m_aWet[(lz + dz) * m_iL + sx + dx] = 1;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! One row of 1 m squares: steep (over 12 degrees) or a step (a jump over 0.3 m), and water.
	protected void ReadSlopeRow(int lz)
	{
		for (int lx = 0; lx < m_iL - 1; lx++)
		{
			int i = lz * m_iL + lx;
			float h00 = m_aHeight[i];
			float h10 = m_aHeight[i + 1];
			float h01 = m_aHeight[i + m_iL];
			float h11 = m_aHeight[i + m_iL + 1];
			float rise = Math.Max(Math.Max(Math.AbsFloat(h10 - h00), Math.AbsFloat(h01 - h00)), Math.Max(Math.AbsFloat(h11 - h10), Math.AbsFloat(h11 - h01)));
			int flag = 0;
			if (rise > STEP_RISE)
				flag = STEP | STEEP;
			else if (rise > STEEP_RISE)
				flag = STEEP;

			int cx = lx * 2;
			int cz = lz * 2;
			for (int k = 0; k < 4; k++)
			{
				int kx = cx + (k % 2);
				int kz = cz + (k / 2);
				// A cell is wet if the sample nearest its middle is
				int sx = Math.Round((kx + 0.5) * CELL);
				int sz = Math.Round((kz + 0.5) * CELL);
				int f = flag;
				if (m_aWet[sz * m_iL + sx] != 0)
					f = f | WATER;

				if (f != 0)
					SetFlag(kx, kz, f);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every dry cell within 4 m of water: a two-pass distance over the 1 m samples, a row at a time
	//! (step k: rows down, then up, then the cells' rows). False when done (or no water at all).
	protected bool ShoreRow(int k)
	{
		int n = m_iL * m_iL;
		if (k == 0)
		{
			bool any = false;
			m_aDist.Resize(n);
			for (int i = 0; i < n; i++)
			{
				m_aDist[i] = 9999;
				if (m_aWet[i] != 0)
				{
					m_aDist[i] = 0;
					any = true;
				}
			}

			if (!any)
				return false;
		}

		if (k < m_iL)
		{
			int z = k;
			for (int x = 0; x < m_iL; x++)
			{
				int j = z * m_iL + x;
				if (x > 0)
					m_aDist[j] = Math.Min(m_aDist[j], m_aDist[j - 1] + 1);

				if (z > 0)
					m_aDist[j] = Math.Min(m_aDist[j], m_aDist[j - m_iL] + 1);
			}

			return true;
		}

		if (k < 2 * m_iL)
		{
			int bz = 2 * m_iL - 1 - k;
			for (int bx = m_iL - 1; bx >= 0; bx--)
			{
				int b = bz * m_iL + bx;
				if (bx < m_iL - 1)
					m_aDist[b] = Math.Min(m_aDist[b], m_aDist[b + 1] + 1);

				if (bz < m_iL - 1)
					m_aDist[b] = Math.Min(m_aDist[b], m_aDist[b + m_iL] + 1);
			}

			return true;
		}

		int cz = k - 2 * m_iL;
		if (cz >= m_iN)
			return false;

		for (int cx = 0; cx < m_iN; cx++)
		{
			int sx = Math.Round((cx + 0.5) * CELL);
			int sz = Math.Round((cz + 0.5) * CELL);
			int d = m_aDist[sz * m_iL + sx];
			if (d > 0 && d <= SHORE_BAND)
				SetFlag(cx, cz, SHORE);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A turned box on the ground: half-width hw (its right), half-depth hd (its front), facing yaw.
	void Box(vector c, float yaw, float hw, float hd, int flag)
	{
		float turn = yaw * Math.DEG2RAD;
		float sn = Math.Sin(turn);
		float cs = Math.Cos(turn);
		float ex = Math.AbsFloat(cs) * hw + Math.AbsFloat(sn) * hd;
		float ez = Math.AbsFloat(sn) * hw + Math.AbsFloat(cs) * hd;
		int x0 = Math.Max(0, CellX(c[0] - ex));
		int x1 = Math.Min(m_iN - 1, CellX(c[0] + ex));
		int z0 = Math.Max(0, CellZ(c[2] - ez));
		int z1 = Math.Min(m_iN - 1, CellZ(c[2] + ez));
		// A thin thing (a fence, a pole) still takes the cells it crosses
		float gw = Math.Max(hw, CELL * 0.5);
		float gd = Math.Max(hd, CELL * 0.5);
		for (int cz = z0; cz <= z1; cz++)
		{
			float dz = CellCentreZ(cz) - c[2];
			for (int cx = x0; cx <= x1; cx++)
			{
				float dx = CellCentreX(cx) - c[0];
				float lx = dx * cs - dz * sn;
				float lz = dx * sn + dz * cs;
				if (Math.AbsFloat(lx) <= gw && Math.AbsFloat(lz) <= gd)
					SetFlag(cx, cz, flag);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A turned ellipse on the ground, half-axes hw (its right) and hd (its front).
	void Ellipse(vector c, float yaw, float hw, float hd, int flag)
	{
		float turn = yaw * Math.DEG2RAD;
		float sn = Math.Sin(turn);
		float cs = Math.Cos(turn);
		float reach = Math.Max(hw, hd);
		int x0 = Math.Max(0, CellX(c[0] - reach));
		int x1 = Math.Min(m_iN - 1, CellX(c[0] + reach));
		int z0 = Math.Max(0, CellZ(c[2] - reach));
		int z1 = Math.Min(m_iN - 1, CellZ(c[2] + reach));
		float gw = Math.Max(hw, CELL * 0.5);
		float gd = Math.Max(hd, CELL * 0.5);
		for (int cz = z0; cz <= z1; cz++)
		{
			float dz = CellCentreZ(cz) - c[2];
			for (int cx = x0; cx <= x1; cx++)
			{
				float dx = CellCentreX(cx) - c[0];
				float lx = (dx * cs - dz * sn) / gw;
				float lz = (dx * sn + dz * cs) / gd;
				if (lx * lx + lz * lz <= 1)
					SetFlag(cx, cz, flag);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void Disc(vector c, float radius, int flag)
	{
		int x0 = Math.Max(0, CellX(c[0] - radius));
		int x1 = Math.Min(m_iN - 1, CellX(c[0] + radius));
		int z0 = Math.Max(0, CellZ(c[2] - radius));
		int z1 = Math.Min(m_iN - 1, CellZ(c[2] + radius));
		float r2 = (radius + CELL * 0.5) * (radius + CELL * 0.5);
		for (int cz = z0; cz <= z1; cz++)
		{
			float dz = CellCentreZ(cz) - c[2];
			for (int cx = x0; cx <= x1; cx++)
			{
				float dx = CellCentreX(cx) - c[0];
				if (dx * dx + dz * dz <= r2)
					SetFlag(cx, cz, flag);
			}
		}
	}

	// ================================================================ Lines and paths
	//------------------------------------------------------------------------------------------------
	//! Nothing solid or a building between two points (a 2 m-high line on the grid: the cells it
	//! crosses, inside the grid only). The first and last metre are left out (the post and its target).
	bool LineClear(vector a, vector b)
	{
		float len = RZ_Util.DistXZ(a, b);
		int steps = Math.Ceil(len / (CELL * 0.5));
		for (int i = 0; i <= steps; i++)
		{
			float t = i / Math.Max(1, steps);
			float along = t * len;
			if (along < 1 || along > len - 1)
				continue;

			float x = a[0] + (b[0] - a[0]) * t;
			float z = a[2] + (b[2] - a[2]) * t;
			int f = Flags(CellX(x), CellZ(z));
			if (f == OUTSIDE)
				return true;

			if (f & (SOLID | INSIDE))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A walkable way (1 m wide: 2 x 2 cells free of NO_WALK) from a to b, by a breadth-first search
	//! over the 1 m squares. True if b is reached, or either end is off the grid.
	bool PathClear(vector a, vector b)
	{
		return Path(a, b, null, null);
	}

	//------------------------------------------------------------------------------------------------
	//! PathClear, and the way found as points 1 m apart (from a to b) in `route` if given (empty
	//! when an end is off the grid or there's no way); with `within`, only inside that wire line
	//! (and a metre past it: its gates).
	bool Path(vector a, vector b, array<vector> route, RZ_SiteWire within)
	{
		int n = m_iL - 1;
		int ax = Math.Floor(a[0] - m_vLow[0]);
		int az = Math.Floor(a[2] - m_vLow[2]);
		int bx = Math.Floor(b[0] - m_vLow[0]);
		int bz = Math.Floor(b[2] - m_vLow[2]);
		if (ax < 0 || az < 0 || ax >= n || az >= n || bx < 0 || bz < 0 || bx >= n || bz >= n)
			return true;

		// 0 unseen; else the square it was reached from, plus one
		array<int> from = {};
		from.Resize(n * n);
		for (int i = 0; i < n * n; i++)
		{
			from[i] = 0;
		}

		int start = StartSquare(ax, az);
		array<int> queue = {start};
		from[start] = start + 1;
		int head = 0;
		while (head < queue.Count())
		{
			int cur = queue[head];
			head++;
			// Within 1.5 m of the goal is there (the goal itself may be a piece: the cook pot)
			int qx = cur % n;
			int qz = cur / n;
			if (Math.AbsInt(qx - bx) <= 1 && Math.AbsInt(qz - bz) <= 1)
			{
				if (route)
				{
					int back = cur;
					int guard = 0;
					while (guard < n * n)
					{
						int rx = back % n;
						int rz = back / n;
						route.InsertAt(Vector(m_vLow[0] + rx + 0.5, 0, m_vLow[2] + rz + 0.5), 0);
						if (back == start)
							break;

						back = from[back] - 1;
						guard++;
					}
				}

				return true;
			}

			for (int d = 0; d < 4; d++)
			{
				int nx = qx;
				int nz = qz;
				if (d == 0)
					nx++;
				else if (d == 1)
					nx--;
				else if (d == 2)
					nz++;
				else
					nz--;

				if (nx < 0 || nz < 0 || nx >= n || nz >= n)
					continue;

				int j = nz * n + nx;
				if (from[j] != 0)
					continue;

				from[j] = cur + 1;
				if (!Walkable(nx, nz) || !Within(within, nx, nz))
					continue;

				queue.Insert(j);
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The 1 m square a way starts from: (ax, az), or with a step or a porch beside a door the
	//! nearest square a body can stand on within 2 m.
	protected int StartSquare(int ax, int az)
	{
		int n = m_iL - 1;
		if (Walkable(ax, az))
			return az * n + ax;

		for (int r = 1; r <= 2; r++)
		{
			for (int dz = -r; dz <= r; dz++)
			{
				for (int dx = -r; dx <= r; dx++)
				{
					int sx = ax + dx;
					int sz = az + dz;
					if (sx >= 0 && sz >= 0 && sx < n && sz < n && Walkable(sx, sz))
						return sz * n + sx;
				}
			}
		}

		return az * n + ax;
	}

	//------------------------------------------------------------------------------------------------
	//! The middle of the nearest 1 m square a body can stand on within `reach` m of `at` (a door
	//! whose step the grid reads as in a building or a porch); `at` itself if none or off the grid.
	vector NearestWalkable(vector at, int reach)
	{
		int n = m_iL - 1;
		int ax = Math.Floor(at[0] - m_vLow[0]);
		int az = Math.Floor(at[2] - m_vLow[2]);
		if (ax < 0 || az < 0 || ax >= n || az >= n || Walkable(ax, az))
			return at;

		for (int r = 1; r <= reach; r++)
		{
			for (int dz = -r; dz <= r; dz++)
			{
				for (int dx = -r; dx <= r; dx++)
				{
					if (Math.AbsInt(dx) != r && Math.AbsInt(dz) != r)
						continue;

					int sx = ax + dx;
					int sz = az + dz;
					if (sx >= 0 && sz >= 0 && sx < n && sz < n && Walkable(sx, sz))
						return RZ_Util.GroundPos(Vector(m_vLow[0] + sx + 0.5, 0, m_vLow[2] + sz + 0.5));
				}
			}
		}

		return at;
	}

	//------------------------------------------------------------------------------------------------
	//! Every 1 m square reachable on foot from `a` (1 = reached; a breadth-first search as Path's).
	//! Empty if `a` is off the grid.
	array<int> Reach(vector a, RZ_SiteWire within)
	{
		int n = m_iL - 1;
		array<int> reached = {};
		int ax = Math.Floor(a[0] - m_vLow[0]);
		int az = Math.Floor(a[2] - m_vLow[2]);
		if (ax < 0 || az < 0 || ax >= n || az >= n)
			return reached;

		reached.Resize(n * n);
		for (int i = 0; i < n * n; i++)
		{
			reached[i] = 0;
		}

		int start = StartSquare(ax, az);
		array<int> queue = {start};
		reached[start] = 1;
		int head = 0;
		while (head < queue.Count())
		{
			int cur = queue[head];
			head++;
			int qx = cur % n;
			int qz = cur / n;
			for (int d = 0; d < 4; d++)
			{
				int nx = qx;
				int nz = qz;
				if (d == 0)
					nx++;
				else if (d == 1)
					nx--;
				else if (d == 2)
					nz++;
				else
					nz--;

				if (nx < 0 || nz < 0 || nx >= n || nz >= n)
					continue;

				int j = nz * n + nx;
				if (reached[j] != 0)
					continue;

				reached[j] = 2;
				if (!Walkable(nx, nz) || !Within(within, nx, nz))
					continue;

				reached[j] = 1;
				queue.Insert(j);
			}
		}

		return reached;
	}

	//------------------------------------------------------------------------------------------------
	//! A point is reached in `reached` (Reach's): its 1 m square or one beside it.
	bool Reached(array<int> reached, vector pt)
	{
		int n = m_iL - 1;
		if (reached.IsEmpty())
			return true;

		int px = Math.Floor(pt[0] - m_vLow[0]);
		int pz = Math.Floor(pt[2] - m_vLow[2]);
		for (int dz = -1; dz <= 1; dz++)
		{
			for (int dx = -1; dx <= 1; dx++)
			{
				int sx = px + dx;
				int sz = pz + dz;
				if (sx >= 0 && sz >= 0 && sx < n && sz < n && reached[sz * n + sx] == 1)
					return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A 1 m square inside a wire line, or within a metre outside it (none given: anywhere).
	protected bool Within(RZ_SiteWire within, int sx, int sz)
	{
		if (!within)
			return true;

		return within.Inside(Vector(m_vLow[0] + sx + 0.5, 0, m_vLow[2] + sz + 0.5), -1);
	}

	//------------------------------------------------------------------------------------------------
	protected bool Walkable(int sx, int sz)
	{
		int cx = sx * 2;
		int cz = sz * 2;
		int f = Flags(cx, cz) | Flags(cx + 1, cz) | Flags(cx, cz + 1) | Flags(cx + 1, cz + 1);
		return (f & NO_WALK) == 0;
	}

	// ================================================================ Drawing
	//------------------------------------------------------------------------------------------------
	//! One row of the grid at 1 m for the audit's drawing (render.py --placed): a letter a square,
	//! the strongest thing in it. '.' open, '~' water, 'b' building, '#' solid, '*' bush, '=' road,
	//! '^' steep, 'k' other scenery, 'p' a piece, ',' a path kept clear.
	string DrawRow(int sz)
	{
		string row = "";
		for (int sx = 0; sx < m_iL - 1; sx++)
		{
			int cx = sx * 2;
			int cz = sz * 2;
			int f = Flags(cx, cz) | Flags(cx + 1, cz) | Flags(cx, cz + 1) | Flags(cx + 1, cz + 1);
			row += Letter(f);
		}

		return row;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Letter(int f)
	{
		if (f & TAKEN)
			return "p";

		if (f & INSIDE)
			return "b";

		if (f & WATER)
			return "~";

		if (f & SOLID)
			return "#";

		if (f & KEPT)
			return "k";

		if (f & ROAD)
			return "=";

		if (f & (STEEP | STEP))
			return "^";

		if (f & SOFT)
			return "*";

		if (f & CLEAR)
			return ",";

		return ".";
	}
}
