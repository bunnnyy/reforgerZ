//! The site solver (docs/SITES.md, "The solver" and "Building in layers"): lays a site out from its
//! kit's rules against its ground (RZ_SiteGrid) and surroundings (RZ_SiteSurroundings), in the order
//! people would build it: close it off (the wire line, its gates, the barricades and the faction's
//! barrier), harden the building (side doors blocked, ground-floor windows boarded), keep the paths
//! clear, watch (the nest over the gate, a lookout toward each threatened side, the lights), then
//! work and life inside the wire, valuables indoors (as data). Each piece goes to the spot that
//! meets all its hard rules (zone, away from, the ground, the paths) with the best score for its
//! soft ones (near, covers, faces, the plan's own spot); at most Candidates a piece spots are tried,
//! and none = dropped, with the rule that failed. Every choice comes from the place's seed. Runs a
//! little each second (Site read ms), one site at a time, once a run; the result is data
//! (RZ_SiteResult) that camp life keeps and spawns. Server only.
class RZ_SiteSolver
{
	// Tasks, in order
	protected static const int T_PLAN = 0;		// one plan piece by its rules
	protected static const int T_LINE = 1;		// one plan piece on the wire line (a barricade)
	protected static const int T_FILL = 2;		// one side of the wire filled with the faction's barrier
	protected static const int T_HARDEN = 3;
	protected static const int T_PATH = 4;
	protected static const int T_LOOKOUTS = 5;
	protected static const int T_LIGHTS = 6;
	protected static const int T_STORES = 7;
	protected static const int T_CORNERS = 8;	// the building's lookouts: the corners that see the gate
	protected static const int T_GATEPOST = 9;	// the gate's post: a spot just inside it that sees it
	protected static const int T_STORY = 10;	// the story's spots (docs/SITES.md, "The story of each site")
	protected static const int T_PEOPLE = 11;	// the posts and the routes (docs/SITES.md, "The people")
	protected static const int T_SEAL = 12;		// every road in but the checkpoints closed (docs/SITES.md, "Checkpoints")
	protected static const int T_LEAF = 13;		// a chain-link gate's leaf, swung open at an end of a gate (docs/SITES.md, "Close it off")

	protected static const float THREAT_WATCH = 3;		// a side this threatened wants a watch (as the audit)
	protected static const float PLAN_WEIGHT = 0.6;		// score lost per metre from the plan's spot
	protected static const float WIRE_IN = 1.5;			// work and life keep this far inside the line
	protected static const float FLAT_SLOPE = 0.21;		// as camp life's fit: about 12 degrees...
	protected static const float FLAT_SLACK = 0.2;
	protected static const float RUN_SLOPE = 0.36;		// a barrier run tilts to follow ground at most this steep (20 degrees)...
	protected static const float RUN_OFF = 0.25;		// ...with no point of the ground under it more than this off its line
	protected static const int MAX_LOOKOUTS = 3;
	protected static const int MAX_WINDOWS = 14;
	protected static const int MAX_DOORS = 4;
	protected static const int TOP = 4;					// the best few spots are traced, in order
	protected static const float PROP_SPREAD = 0.28;	// the ground under any one prop varies at most this much
	protected static const float ROUGH_SLOPE = 0.36;	// a tier 1 piece's second pass, and a piece that may stand on a bank: 20 degrees...
	protected static const float ROUGH_SPREAD = 0.6;	// ...and a prop's ground up to this
	protected static const float ROW_GAP = 1.2;			// between two of a kind set out in a row
	protected static const float LEVEL_SLOPE = 0.05;	// a level barrier (crates, tyres, a wreck) stands on ground no steeper than this along it (3 degrees)
	protected static const float CLOSED_SHARE = 0.6;	// a road this much closed already by a wall, a fence or a gate needs no road block

	ref RZ_CampPlace m_Place;			// held: the job may hand its place over to a newer one (Survey) while this one is solved
	ref RZ_SiteResult m_Result;
	ref RZ_SiteWire m_Wire;				// null: no ring (a road post, or Layers off)
	int m_iMs;
	int m_iSteps;

	protected RZ_CampLife m_Camp;
	protected RZ_CampLifeSettings m_Settings;
	protected ref RZ_CampGround m_Ground;	// held, like the place: Survey gives the job a newer ground when the place comes back mid-solve
	protected RZ_SiteGrid m_Grid;
	protected RZ_SiteSurroundings m_Around;
	protected RZ_DressingKit m_Kit;
	protected ref RZ_CampFit m_Fit;
	protected ref RZ_CampLayout m_Lay;
	protected ref RZ_Rng m_Rng;
	protected bool m_bAudit;
	protected bool m_bStarted;
	protected bool m_bDone;
	protected ref array<int> m_aTask = {};
	protected ref array<RZ_DressingPiece> m_aTaskPiece = {};
	protected ref array<int> m_aTaskArg = {};
	protected int m_iNext;
	protected int m_iCap;
	protected int m_iLaid;
	protected int m_iLeaf;			// gate leaves hung so far (each takes the next end of a gate)
	protected int m_iLayerCap;
	protected int m_iLayerLaid;
	protected int m_iLife;				// work and life's pieces laid in the yard
	protected int m_iClose;
	protected int m_iHarden;
	protected vector m_vDoor;			// the one entrance (the yard's middle on open ground)
	protected vector m_vMid;			// the middle of the building or the plan
	protected vector m_vGate;
	protected float m_fGateOut;
	protected bool m_bGate;
	protected float m_fFront;			// the threat front's bearing
	protected int m_iYardFace = -1;		// the wire side the yard is on
	protected float m_fNeed;			// the yard's area its pieces need, m2 (docs/SITES.md, "Room for the yard")
	protected float m_fYardDepth;		// how far out from the building the yard reaches
	protected bool m_bVerge;			// a road post's piece: beside the road or not at all
	protected vector m_vKitchen;		// the kitchen's door
	protected ref array<RZ_DressingPiece> m_aWires = {};	// the faction's barrier runs, longest first
	protected ref array<RZ_DressingPiece> m_aBlocks = {};	// its road blocks, longest first
	protected ref map<string, int> m_mGround = new map<string, int>();	// what the ground under its soft-ground pieces was made of
	protected string m_sWhy;			// the last hard rule a spot failed
	protected bool m_bRough;			// a tier 1 piece's second pass: the ground's flatness eased
	protected bool m_bAnyZone;			// and its last: anywhere inside the wire
	protected ref RZ_SiteRules m_Plain = new RZ_SiteRules();	// no rules: a layer piece turned as it's given
	protected bool m_bStoryOut;			// a story element's spot: outside the wire

	//------------------------------------------------------------------------------------------------
	void RZ_SiteSolver(RZ_CampLife camp, RZ_CampPlace p, RZ_CampGround ground, RZ_SiteGrid grid, RZ_SiteSurroundings around, RZ_DressingKit kit, bool audit)
	{
		m_Camp = camp;
		m_Settings = camp.GetSettings();
		m_Place = p;
		m_Ground = ground;
		m_Grid = grid;
		m_Around = around;
		m_Kit = kit;
		m_bAudit = audit;
		m_Result = new RZ_SiteResult();
		m_Rng = new RZ_Rng(p.m_iSeed + 41);
	}

	//------------------------------------------------------------------------------------------------
	//! Works on for up to budgetMs; true once the site is laid out.
	bool Step(int budgetMs)
	{
		int t0 = System.GetTickCount();
		if (!m_bStarted)
		{
			m_bStarted = true;
			Start();
			m_Result.m_iSolveMaxUnitMs = System.GetTickCount() - t0;
		}

		while (!m_bDone && System.GetTickCount() - t0 < budgetMs)
		{
			int u0 = System.GetTickCount();
			StepOnce();
			m_Result.m_iSolveMaxUnitMs = Math.Max(m_Result.m_iSolveMaxUnitMs, System.GetTickCount() - u0);
		}

		int spent = System.GetTickCount() - t0;
		m_iMs += spent;
		m_iSteps++;
		m_Result.m_iSolveMs = m_iMs;
		m_Result.m_iSolveSteps = m_iSteps;
		m_Result.m_iSolveMaxMs = Math.Max(m_Result.m_iSolveMaxMs, spent);
		return m_bDone;
	}

	//------------------------------------------------------------------------------------------------
	protected void StepOnce()
	{
		if (m_iNext >= m_aTask.Count())
		{
			m_bDone = true;
			if (!m_mGround.IsEmpty())
			{
				string seen = "";
				foreach (string material, int times : m_mGround)
				{
					if (!seen.IsEmpty())
						seen += ", ";

					seen += string.Format("%1 x%2", material, times);
				}

				RZ_Log.Info(string.Format("Sites ground: %1 at %2: %3", RZ_Factions.GetName(m_Place.m_eFaction), RZ_SettlementScanner.GridReference(m_Place.m_vPos), seen));
			}

			return;
		}

		int task = m_aTask[m_iNext];
		RZ_DressingPiece e = m_aTaskPiece[m_iNext];
		int arg = m_aTaskArg[m_iNext];
		m_iNext++;
		switch (task)
		{
			case T_PLAN: PlacePlan(e); break;
			case T_LINE: PlaceOnLine(e); break;
			case T_FILL: FillSide(arg); break;
			case T_HARDEN: Harden(); break;
			case T_PATH: Paths(); break;
			case T_LOOKOUTS: Lookouts(); break;
			case T_LIGHTS: Lights(); break;
			case T_STORES: StoresBoard(); break;
			case T_CORNERS: BuildingLookouts(); break;
			case T_GATEPOST: GatePost(); SidePosts(); break;
			case T_STORY: StorySpots(); break;
			case T_PEOPLE: People(); break;
			case T_SEAL: SealRoads(); break;
			case T_LEAF: GateLeaf(e); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AddTask(int task, RZ_DressingPiece e, int arg)
	{
		m_aTask.Insert(task);
		m_aTaskPiece.Insert(e);
		m_aTaskArg.Insert(arg);
	}

	// ================================================================ Setting out
	//------------------------------------------------------------------------------------------------
	//! The counters, the plan's frame, the doctrine, the wire line and its gates, then the tasks.
	protected void Start()
	{
		RZ_CampPlace p = m_Place;
		m_Fit = m_Camp.ReadFitFor(p, m_Ground);
		m_Fit.m_Grid = m_Grid;
		m_iCap = m_Camp.PieceCapOf(p);
		m_iLayerCap = m_Settings.m_iLayerPieces;
		m_iLaid = m_Camp.PlaceCountersFor(p, m_Fit, m_Kit, m_iCap, m_Result, m_bAudit);

		// The plan: in a building they sleep indoors (no tents)
		array<RZ_DressingPiece> plan = {};
		foreach (RZ_DressingPiece e : m_Kit.m_aPieces)
		{
			if (!e || e.m_sPrefab.IsEmpty() || e.m_eAt == RZ_EDressAt.COUNTER)
				continue;

			// (a dense kit's extra tents stand in the yard all the same: they're the look of the camp)
			if (p.m_Site && e.m_eUse == RZ_EDressUse.SLEEP && !e.m_bExtra)
				continue;

			plan.Insert(e);
		}

		// No plan: a kit says what the faction brings; each piece finds its spot by its zone and rules.
		// The yard is sized by what its pieces need (their measured footprints, at least 12 m2 each)
		m_fNeed = NeedOf(plan);
		m_Lay = new RZ_CampLayout();
		array<RZ_DressingPiece> noPlan = {};
		m_Camp.ChooseFrame(p, m_Ground, m_Fit, noPlan, m_iCap, m_Lay);
		if (p.m_Site)
			ChooseRoom();

		m_Result.m_sWhere = m_Lay.m_sWhere;

		m_iClose = m_Kit.m_eClose;
		m_iHarden = m_Kit.m_eHarden;
		if (!m_Settings.m_bLayers || p.m_eKind == RZ_ECampPlace.ROAD_POST)
		{
			m_iClose = RZ_ESiteClose.NONE;
			m_iHarden = RZ_ESiteHarden.NONE;
		}

		m_fFront = p.m_fApproach;
		if (m_Around)
			m_fFront = m_Around.m_iFront * 45;

		m_vMid = p.m_vPos;
		m_vDoor = m_Lay.m_vOrigin;
		if (p.m_Site)
		{
			m_vMid = p.m_Site.m_vCentre;
			m_vDoor = m_Camp.DoorSpot(p);
		}

		// The entrance's step: where a body can stand, within 6 m of the door
		m_vDoor = m_Grid.NearestWalkable(m_vDoor, 6);
		m_Result.m_vDoor = m_vDoor;
		m_Result.m_bDoor = true;

		m_vGate = RZ_SiteAudit.GatePoint(p);
		m_fGateOut = RZ_Compass.Bearing(m_vMid, m_vGate);
		if (m_iClose != RZ_ESiteClose.NONE)
			TraceWire(plan);

		if (m_Wire)
		{
			m_vGate = m_Wire.m_vGate;
			m_fGateOut = m_Wire.m_fGateOut;
			m_bGate = true;
		}

		m_vKitchen = KitchenDoor();
		m_Result.m_Wire = m_Wire;
		m_Result.m_vGate = m_vGate;
		m_Result.m_bGate = m_bGate;

		foreach (RZ_DressingPiece w : m_Kit.m_aLayers)
		{
			if (!w || w.m_eLayer != RZ_ESiteLayer.WIRE || w.m_sPrefab.IsEmpty())
				continue;

			int at = 0;
			while (at < m_aWires.Count() && Length(m_aWires[at]) >= Length(w))
			{
				at++;
			}

			m_aWires.InsertAt(w, at);
		}

		foreach (RZ_DressingPiece block : m_Kit.m_aLayers)
		{
			if (!block || block.m_eLayer != RZ_ESiteLayer.BLOCK || block.m_sPrefab.IsEmpty())
				continue;

			int before = 0;
			while (before < m_aBlocks.Count() && Length(m_aBlocks[before]) >= Length(block))
			{
				before++;
			}

			m_aBlocks.InsertAt(block, before);
		}

		// The tasks: the path, the other roads closed, the gate, the wire, harden, the watch, then work
		// and life by tier
		AddTask(T_PATH, null, 0);
		if (m_Wire && m_Settings.m_iCheckpoints > 0 && m_iClose == RZ_ESiteClose.ALL && !m_aBlocks.IsEmpty())
			AddTask(T_SEAL, null, 0);

		foreach (RZ_DressingPiece g : plan)
		{
			if (g.m_eZone == RZ_ESiteZone.GATE && !g.m_sPrefab.Contains("GateLeaf"))
				AddTask(T_PLAN, g, 0);
		}

		foreach (RZ_DressingPiece b : plan)
		{
			if (b.m_eZone == RZ_ESiteZone.WIRE)
				AddTask(T_LINE, b, 0);
		}

		if (m_Wire)
		{
			for (int k = 0; k < 4; k++)
			{
				if (m_Wire.m_iClosing & (1 << k))
					AddTask(T_FILL, null, k);
			}

			// The gate's leaves once the line is laid, so their room doesn't cut it short
			foreach (RZ_DressingPiece leaf : plan)
			{
				if (leaf.m_eZone == RZ_ESiteZone.GATE && leaf.m_sPrefab.Contains("GateLeaf"))
					AddTask(T_LEAF, leaf, 0);
			}
		}

		AddTask(T_HARDEN, null, 0);
		foreach (RZ_DressingPiece n : plan)
		{
			if (n.m_eUse == RZ_EDressUse.WATCH && n.m_eZone != RZ_ESiteZone.GATE && n.m_eZone != RZ_ESiteZone.WIRE)
				AddTask(T_PLAN, n, 0);
		}

		AddTask(T_LOOKOUTS, null, 0);
		AddTask(T_LIGHTS, null, 0);
		AddTask(T_CORNERS, null, 0);
		AddTask(T_GATEPOST, null, 0);
		for (int tier = 1; tier <= 5; tier++)
		{
			foreach (RZ_DressingPiece l : plan)
			{
				if (l.m_eZone == RZ_ESiteZone.GATE || l.m_eZone == RZ_ESiteZone.WIRE || l.m_eUse == RZ_EDressUse.WATCH)
					continue;

				if (RZ_SiteAudit.TierOf(l, p.m_eFaction) == tier)
					AddTask(T_PLAN, l, 0);
			}
		}

		AddTask(T_STORES, null, 0);
		AddTask(T_STORY, null, 0);
		AddTask(T_PEOPLE, null, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! The yard's area a kit's pieces need: each yard piece's measured footprint with its half-metre
	//! gap all round, at least the 12 m2 the spacing rule gives a piece (docs/SITES.md, "Room for the
	//! yard"). Pieces by the door, the front wall, the gate or indoors don't count.
	protected float NeedOf(array<RZ_DressingPiece> plan)
	{
		float need = 0;
		foreach (RZ_DressingPiece e : plan)
		{
			if (e.m_bIndoors || e.m_eZone == RZ_ESiteZone.GATE || e.m_eZone == RZ_ESiteZone.WIRE)
				continue;

			RZ_EDressAt at = RZ_CampLife.AtOfPiece(e);
			if (m_Place.m_Site && at != RZ_EDressAt.YARD)
				continue;

			vector size = e.m_vFootMax - e.m_vFootMin;
			need += Math.Max((size[0] + 2 * RZ_SiteGrid.CELL) * (size[2] + 2 * RZ_SiteGrid.CELL), 12);
		}

		return need;
	}

	//------------------------------------------------------------------------------------------------
	//! By a building: the side of it the yard goes on. Each side is read for free ground (the grid:
	//! nothing a piece may not stand on) over as deep a strip as the pieces need along its length and
	//! a ring either end; the faction's own side (behind the building; the Red Hand's in front)
	//! scores a little extra, so it's kept unless another side has clearly more room.
	protected void ChooseRoom()
	{
		RZ_CampSite site = m_Place.m_Site;
		array<int> faces = {site.m_iYardFace};
		array<int> others = {RZ_CampLife.FACE_BACK, RZ_CampLife.FACE_LEFT, RZ_CampLife.FACE_RIGHT, RZ_CampLife.FACE_FRONT};
		foreach (int o : others)
		{
			if (!faces.Contains(o))
				faces.Insert(o);
		}

		float best = -1;
		foreach (int face : faces)
		{
			vector wall = m_Camp.FacePointOf(site, face, 0, 0);
			float outB = RZ_Compass.Bearing(wall, m_Camp.FacePointOf(site, face, 0, 10));
			float length = RZ_Util.DistXZ(m_Camp.FacePointOf(site, face, -1, 0), m_Camp.FacePointOf(site, face, 1, 0)) + 2 * m_Kit.m_fRing;
			float depth = Math.Clamp(m_fNeed / Math.Max(length, 8) + 2.5, m_Kit.m_fYard, 32);
			float free = 0;
			float read = 0;
			for (float z = 2.5; z <= depth; z += 2)
			{
				for (float x = -length * 0.5; x <= length * 0.5; x += 2)
				{
					vector spot = RZ_Compass.PointAt(RZ_Compass.PointAt(wall, outB, z), outB + 90, x);
					read++;
					if ((m_Grid.FlagsAt(spot) & (RZ_SiteGrid.BLOCKS_LIFE | RZ_SiteGrid.OUTSIDE)) == 0)
						free++;
				}
			}

			float score = 0;
			if (read > 0)
				score = free / read;

			if (face == site.m_iYardFace)
				score += 0.1;

			if (score > best)
			{
				best = score;
				m_fYardDepth = depth;
				m_Lay.m_iFace = face;
				m_Lay.m_vOrigin = RZ_Util.GroundPos(RZ_Compass.PointAt(wall, outB, depth * 0.5));
				m_Lay.m_fBearing = RZ_Compass.Normalize(outB + 180);
				m_Lay.m_sWhere = RZ_CampLife.YardWhere(face, face == RZ_CampLife.FACE_FRONT && site.m_iYardFace == RZ_CampLife.FACE_FRONT, site);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Where a yard piece's search starts: the middle of the yard, halfway between the building's yard
	//! side and the wire's line on that side (with no wire, the side's room as read; on open ground
	//! the camp's middle).
	protected vector YardMiddle()
	{
		if (!m_Wire || m_iYardFace < 0)
			return m_Lay.m_vOrigin;

		float x = (m_Wire.m_fX0 + m_Wire.m_fX1) * 0.5;
		float z = (m_Wire.m_fZ0 + m_Wire.m_fZ1) * 0.5;
		if (m_Place.m_Site)
		{
			switch (m_iYardFace)
			{
				case 0: z = (m_Wire.m_vInMin[2] + m_Wire.m_fZ0) * 0.5; break;
				case 1: x = (m_Wire.m_vInMax[0] + m_Wire.m_fX1) * 0.5; break;
				case 2: z = (m_Wire.m_vInMax[2] + m_Wire.m_fZ1) * 0.5; break;
				case 3: x = (m_Wire.m_vInMin[0] + m_Wire.m_fX0) * 0.5; break;
			}
		}

		return m_Wire.Point(x, z);
	}

	//------------------------------------------------------------------------------------------------
	//! The ring round the building (or the plan on open ground), its sides set to meet what closes
	//! already, the gates on the roads in, and the sides the doctrine closes.
	protected void TraceWire(array<RZ_DressingPiece> plan)
	{
		RZ_CampPlace p = m_Place;
		array<float> want = {m_Kit.m_fRing, m_Kit.m_fRing, m_Kit.m_fRing, m_Kit.m_fRing};
		if (p.m_Site)
		{
			RZ_CampSite site = p.m_Site;
			m_Wire = new RZ_SiteWire(RZ_Util.GroundPos(site.m_vOrigin), site.m_vRight, site.m_vFwd, site.m_vMins, site.m_vMaxs);
			m_iYardFace = WireFace(site, m_Lay.m_iFace);
			// The yard's side is as deep as its pieces need (ChooseRoom)
			if (m_iYardFace >= 0)
				want[m_iYardFace] = Math.Clamp(m_fYardDepth, m_Kit.m_fYard, 32);
		}
		else
		{
			// Open ground: a square as big as the pieces need is the "building", a few metres round it
			vector bearing = RZ_Compass.PointAt(vector.Zero, m_Lay.m_fBearing, 1);
			vector right = Vector(bearing[2], 0, -bearing[0]);
			float half = Math.Clamp(Math.Sqrt(m_fNeed), 8, 40) * 0.5;
			m_Wire = new RZ_SiteWire(m_Lay.m_vOrigin, right, bearing, Vector(-half, 0, -half), Vector(half, 0, half));
			for (int k = 0; k < 4; k++)
			{
				want[k] = 4;
			}
		}

		m_Wire.Trace(m_Grid, want, 15, m_iYardFace);
		vector mainAt = RZ_Compass.PointAt(m_vMid, p.m_fApproach, 50);
		if (p.m_Site && p.m_Site.m_iMain >= 0)
			mainAt = p.m_Site.m_aEntries[p.m_Site.m_iMain];

		m_Wire.ChooseGates(mainAt, m_vDoor, m_Grid, m_Grid.Reach(m_vDoor, m_Wire));
		int closing = 15;
		if (m_iClose == RZ_ESiteClose.ROAD)
		{
			closing = 1 << m_Wire.m_iGateFace;
		}
		else if (m_iClose == RZ_ESiteClose.TWO)
		{
			// The road's side and the most threatened other side
			int other = -1;
			float worst = -float.MAX;
			for (int f = 0; f < 4; f++)
			{
				if (f == m_Wire.m_iGateFace)
					continue;

				float threat = 0;
				if (m_Around)
					threat = m_Around.m_aThreat[RZ_SiteSurroundings.SectorOf(m_Wire.FaceOut(f))];

				if (threat > worst)
				{
					worst = threat;
					other = f;
				}
			}

			closing = (1 << m_Wire.m_iGateFace) | (1 << other);
		}

		if (closing != 15)
			m_Wire.KeepSides(closing);
	}

	//------------------------------------------------------------------------------------------------
	//! The wire's side (0 = -z, 1 = +x, 2 = +z, 3 = -x of the building) of one of camp life's faces.
	static int WireFace(RZ_CampSite site, int face)
	{
		int axis = site.m_iAxis;
		bool positive = site.m_fSign > 0;
		if (face == RZ_CampLife.FACE_BACK)
			positive = !positive;

		if (face == RZ_CampLife.FACE_LEFT || face == RZ_CampLife.FACE_RIGHT)
		{
			axis = 2 - axis;
			positive = face == RZ_CampLife.FACE_RIGHT;
		}

		if (axis == 2)
		{
			if (positive)
				return 2;

			return 0;
		}

		if (positive)
			return 1;

		return 3;
	}

	//------------------------------------------------------------------------------------------------
	//! A barrier run's length along the line (its footprint across its own x).
	protected static float Length(RZ_DressingPiece e)
	{
		return e.m_vFootMax[0] - e.m_vFootMin[0];
	}

	// ================================================================ Close it off
	//------------------------------------------------------------------------------------------------
	//! A plan barricade on the line: beside a gate that isn't the main one, else over the widest gap
	//! on the threat front, else any gap; with no line, at the road entries as the plan has it.
	protected void PlaceOnLine(RZ_DressingPiece e)
	{
		if (m_iLaid >= m_iCap)
			return;

		if (!m_Wire)
		{
			vector planPos;
			float planYaw;
			m_Camp.PlanSpot(m_Place, m_Lay, e, planPos, planYaw);
			array<vector> one = {planPos};
			array<float> oneYaw = {planYaw};
			PlaceBest(e, e.Rules(), one, oneYaw, true, planPos, RZ_SiteGrid.BLOCKS, RZ_ESiteLayer.PLAN, 0);
			return;
		}

		float half = Length(e) * 0.5;
		array<vector> spots = {};
		array<int> faces = {};
		// Beside the other gates
		for (int g = 1; g < m_Wire.m_aGates.Count(); g++)
		{
			LineSpotsBeside(m_Wire.m_aGates[g], half, spots, faces);
		}

		// The gaps, the front's widest first
		GapSpots(half * 2, spots, faces);
		array<float> yaws = {};
		foreach (int i, vector at : spots)
		{
			yaws.Insert(m_Wire.FaceOut(faces[i]));
		}

		array<float> slides = {0, 1, -1, 2, -2, 0.6};
		for (int s = 0; s < spots.Count(); s++)
		{
			foreach (int si, float slide : slides)
			{
				// Along the line a little, the last try half a metre in
				vector slid = RZ_Compass.PointAt(spots[s], yaws[s] + 90, slide);
				if (si == slides.Count() - 1)
					slid = RZ_Compass.PointAt(spots[s], yaws[s] + 180, slide);

				vector pos = LinePose(e, slid, yaws[s]);
				m_sWhy = Fails(e, e.Rules(), pos, yaws[s], WireMask(), FLAT_SLACK);
				if (!m_sWhy.IsEmpty())
					continue;

				if (!m_Camp.Traced(pos, yaws[s], e))
				{
					m_sWhy = "something in the way";
					continue;
				}

				Take(e, pos, yaws[s], RZ_ESiteLayer.PLAN, false);
				m_iLaid++;
				m_Wire.Cover(faces[s], slid, half);
				return;
			}
		}

		// Nothing to close: no gate to stand beside and no gap it fits. A barricade stands where it
		// closes something or not at all (one once stood in the open on the Haven's car park); it
		// isn't counted as dropped
	}

	//------------------------------------------------------------------------------------------------
	//! A chain-link gate standing open (docs/SITES.md, "Close it off"): each leaf hangs at the next end
	//! of a gate, the main gate's two ends first, hinged where the line starts again and swung out
	//! along the road's edge, so the way through stays clear. Not on a road, a building, water or
	//! another piece; with no end to hang on it isn't counted as dropped.
	protected void GateLeaf(RZ_DressingPiece e)
	{
		vector size = e.m_vFootMax - e.m_vFootMin;
		float half = Math.Max(size[0], size[2]) * 0.5;
		int mask = RZ_SiteGrid.INSIDE | RZ_SiteGrid.WATER | RZ_SiteGrid.ROAD | RZ_SiteGrid.KEPT | RZ_SiteGrid.TAKEN;
		int end = 0;
		foreach (vector gate : m_Wire.m_aGates)
		{
			array<vector> spots = {};
			array<int> faces = {};
			LineSpotsBeside(gate, 0, spots, faces);
			foreach (int s, vector at : spots)
			{
				if (end++ < m_iLeaf)
					continue;

				m_iLeaf++;
				float outB = m_Wire.FaceOut(faces[s]);
				vector pos = RZ_Util.GroundPos(RZ_Compass.PointAt(at, outB, half + 0.15));
				float lo;
				float hi;
				if (m_Grid.FirstBlock(pos, outB, e.Foot(), mask, lo, hi) != 0)
					continue;

				Take(e, pos, outB, RZ_ESiteLayer.PLAN, false);
				m_iLaid++;
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Points on the line just beside a gate (both sides), for a piece half as long as `half`.
	protected void LineSpotsBeside(vector gate, float half, notnull array<vector> spots, notnull array<int> faces)
	{
		int best = -1;
		float near = float.MAX;
		foreach (int i, vector at : m_Wire.m_aAt)
		{
			float d = RZ_Util.DistXZ(at, gate);
			if (d < near)
			{
				near = d;
				best = i;
			}
		}

		if (best < 0)
			return;

		int face = m_Wire.m_aFace[best];
		for (int side = -1; side <= 1; side += 2)
		{
			// Walk out of the gate to the first open station, then half the piece on
			int walk = best;
			while (walk >= 0 && walk < m_Wire.m_aAt.Count() && m_Wire.m_aFace[walk] == face && m_Wire.m_aState[walk] == RZ_SiteWire.ST_GATE)
			{
				walk += side;
			}

			int j = walk + side * RZ_Util.RoundToInt(half / RZ_SiteWire.STATION);
			if (j < 0 || j >= m_Wire.m_aAt.Count() || m_Wire.m_aFace[j] != face)
				continue;

			spots.Insert(m_Wire.m_aAt[j]);
			faces.Insert(face);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The middles of the line's gaps long enough for a piece `len` long: the threat front's first,
	//! then the widest.
	protected void GapSpots(float len, notnull array<vector> spots, notnull array<int> faces)
	{
		array<vector> mids = {};
		array<int> midFace = {};
		array<float> worth = {};
		int i = 0;
		int n = m_Wire.m_aAt.Count();
		while (i < n)
		{
			if (m_Wire.m_aState[i] != RZ_SiteWire.ST_GAP || m_Wire.m_aHard[i])
			{
				i++;
				continue;
			}

			int j = i;
			while (j + 1 < n && m_Wire.m_aState[j + 1] == RZ_SiteWire.ST_GAP && !m_Wire.m_aHard[j + 1] && m_Wire.m_aFace[j + 1] == m_Wire.m_aFace[i])
			{
				j++;
			}

			float w = (j - i + 1) * RZ_SiteWire.STATION;
			if (w + 0.25 >= len)
			{
				float off = Math.AbsFloat(RZ_Compass.Normalize(m_Wire.FaceOut(m_Wire.m_aFace[i]) - m_fFront + 180) - 180);
				float value = w;
				if (off < 50)
					value += 100;

				int k = 0;
				while (k < worth.Count() && worth[k] >= value)
				{
					k++;
				}

				// Its start (a piece laid from the gap's start leaves room for the next)
				int first = i + RZ_Util.RoundToInt(len * 0.5 / RZ_SiteWire.STATION) - 1;
				mids.InsertAt(m_Wire.m_aAt[Math.Min(first, j)], k);
				midFace.InsertAt(m_Wire.m_aFace[i], k);
				worth.InsertAt(value, k);
			}

			i = j + 1;
		}

		spots.InsertAll(mids);
		faces.InsertAll(midFace);
	}

	//------------------------------------------------------------------------------------------------
	//! Where a line piece stands so its footprint's middle is on `at`, its front out.
	protected static vector LinePose(RZ_DressingPiece e, vector at, float yaw)
	{
		float cx = (e.m_vFootMin[0] + e.m_vFootMax[0]) * 0.5;
		float cz = (e.m_vFootMin[2] + e.m_vFootMax[2]) * 0.5;
		vector pos = RZ_SiteGrid.Frame(at, yaw, -cx, -cz);
		return RZ_Util.GroundPos(pos);
	}

	//------------------------------------------------------------------------------------------------
	//! A barrier may stand on steep ground and banks (it's tilted to follow them, RunFails) but not on
	//! water, a building, something solid, a road, other scenery, a piece or a path kept clear.
	//! Runs are tested this far inside their measured box: two runs whose end faces meet don't block
	//! each other
	static const float RUN_TOUCH = 0.05;
	static const float FENCE_OVERHANG = 0.6;	// how far a fence's last section may reach past its gap into what closes it
	//! A barrier piece shorter than this is a filler (FillSide)
	static const float FILLER = 1.5;

	//------------------------------------------------------------------------------------------------
	//! A point along a run of stations, `dist` metres from the run's first edge (half a station before
	//! station `first`), and the run's direction. Runs are laid end to end from there by their measured
	//! lengths, so each one's end face meets the next one's (docs/SITES.md, "Spacing").
	protected vector RunPoint(int first, int last, float dist, out vector dir)
	{
		dir = vector.Zero;
		if (last > first)
		{
			dir = m_Wire.m_aAt[last] - m_Wire.m_aAt[first];
			dir[1] = 0;
			dir.Normalize();
		}

		vector at = m_Wire.m_aAt[first] + dir * (dist - RZ_SiteWire.STATION * 0.5);
		return at;
	}

	//------------------------------------------------------------------------------------------------
	//! A run's own length axis (its x at `yaw`), pointing the way `dir` goes: the next run is laid on
	//! along it, so their end faces meet square even where the line's stations run a few degrees off.
	protected static vector RunAxis(float yaw, vector dir)
	{
		vector axis = RZ_SiteGrid.Frame(vector.Zero, yaw, 1, 0);
		axis[1] = 0;
		axis.Normalize();
		if (axis[0] * dir[0] + axis[2] * dir[2] < 0)
			axis = -axis;

		return axis;
	}

	//------------------------------------------------------------------------------------------------
	//! The turn that lays a run's length along `dir` (the line's real direction, which can run a few
	//! degrees off its side's bearing), of the two such turns the one nearer `yaw` (its face out).
	protected static float AlongLine(float yaw, vector dir)
	{
		if (dir[0] == 0 && dir[2] == 0)
			return yaw;

		// A run's x at turn t is (cos t, -sin t)
		float t = Math.Atan2(-dir[2], dir[0]) * Math.RAD2DEG;
		float off = Math.Repeat(t - yaw + 180, 360) - 180;
		if (Math.AbsFloat(off) > 90)
			t += 180;

		return Math.Repeat(t, 360);
	}

	//------------------------------------------------------------------------------------------------
	protected static int WireMask()
	{
		return (RZ_SiteGrid.BLOCKS | RZ_SiteGrid.CLEAR) & ~(RZ_SiteGrid.STEEP | RZ_SiteGrid.STEP);
	}

	//------------------------------------------------------------------------------------------------
	//! A barrier run where it would stand: its cells free (the wire's mask), and the ground along it
	//! close to one straight slope (at most 20 degrees; no point more than 0.25 m off it), so the run
	//! is tilted to follow it (RZ_SitePlaced.m_fSlope) rather than float or sink. Its height is the
	//! slope's at its origin. "" if it fits.
	protected string RunFails(RZ_DressingPiece e, inout vector pos, float yaw, out float slope, bool onRoad = false, float overhang = 0)
	{
		slope = 0;
		float lo;
		float hi;
		int wireMask = WireMask();
		if (onRoad)
			wireMask = wireMask & ~(RZ_SiteGrid.ROAD | RZ_SiteGrid.CLEAR);

		// A fence's last section may reach past its gap into what closes it (a wall, a building), so the
		// line ends against it with no slit (docs/SITES.md, "Close it off")
		int blocks = m_Grid.FirstBlock(pos, yaw, e.Foot(), wireMask, lo, hi, -RUN_TOUCH - overhang, e.Rules().m_bFence);
		if (blocks != 0)
			return RZ_SiteGrid.FlagName(blocks);

		// Anything but a fence keeps off what stands there to its very ends: the shortfall above lets
		// it meet the next run, not reach into a car or a rock (the audit counts that as an overlap)
		if (!e.Rules().m_bFence && m_Grid.FirstBlock(pos, yaw, e.Foot(), RZ_SiteGrid.SOLID, lo, hi, RZ_SiteGrid.CELL * 0.5) != 0)
			return RZ_SiteGrid.FlagName(RZ_SiteGrid.SOLID);

		float x0 = e.m_vFootMin[0];
		float x1 = e.m_vFootMax[0];
		float cz = (e.m_vFootMin[2] + e.m_vFootMax[2]) * 0.5;
		float half = Math.Min(0.6, (e.m_vFootMax[2] - e.m_vFootMin[2]) * 0.5);
		// A road block may be deep (a truck across the road): the ground is read under all of it, so no
		// wheel hangs over a kerb
		if (onRoad)
			half = (e.m_vFootMax[2] - e.m_vFootMin[2]) * 0.5;

		int steps = Math.Max(2, Math.Ceil(x1 - x0));
		array<float> xs = {};
		array<float> hs = {};
		float sx = 0;
		float sh = 0;
		for (int i = 0; i <= steps; i++)
		{
			float x = x0 + (x1 - x0) * i / steps;
			for (int k = -1; k <= 1; k++)
			{
				vector at = RZ_SiteGrid.Frame(pos, yaw, x, cz + k * half);
				float h = m_Grid.HeightAt(at[0], at[2]);
				xs.Insert(x);
				hs.Insert(h);
				sx += x;
				sh += h;
			}
		}

		float mx = sx / xs.Count();
		float mh = sh / hs.Count();
		float num = 0;
		float den = 0;
		foreach (int j, float xj : xs)
		{
			num += (xj - mx) * (hs[j] - mh);
			den += (xj - mx) * (xj - mx);
		}

		if (den > 0.0001)
			slope = num / den;

		if (Math.AbsFloat(slope) > RUN_SLOPE)
			return "steep ground";

		foreach (int m, float xm : xs)
		{
			if (Math.AbsFloat(hs[m] - (mh + slope * (xm - mx))) > RUN_OFF)
				return "not flat";
		}

		pos[1] = mh + slope * (0 - mx);
		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! One side of the line: its gaps filled with the faction's barrier, the longest run that fits
	//! first, nudged in or out half a metre where the line itself is blocked.
	protected void FillSide(int face)
	{
		if (m_aWires.IsEmpty())
			return;

		// A filler (under 1.5 m: a basket of earth, a crate, a sign) closes only a last gap shorter than
		// the shortest real run: never a row of signs where the runs didn't fit
		float shortest = 1000;
		foreach (RZ_DressingPiece r : m_aWires)
		{
			if (Length(r) >= FILLER)
				shortest = Math.Min(shortest, Length(r));
		}

		int n = m_Wire.m_aAt.Count();
		int i = 0;
		while (i < n)
		{
			if (m_iLayerLaid >= m_iLayerCap)
				return;

			if (m_Wire.m_aFace[i] != face || m_Wire.m_aState[i] != RZ_SiteWire.ST_GAP || m_Wire.m_aHard[i])
			{
				i++;
				continue;
			}

			int j = i;
			while (j + 1 < n && m_Wire.m_aFace[j + 1] == face && m_Wire.m_aState[j + 1] == RZ_SiteWire.ST_GAP && !m_Wire.m_aHard[j + 1])
			{
				j++;
			}

			// The gap filled end to end from its first edge: each run starts where the last one's
			// measured end face is, the longest that fits first; where none fits, half a station on
			float run = (j - i + 1) * RZ_SiteWire.STATION;
			float used = 0;
			vector lineDir;
			RunPoint(i, j, 0, lineDir);
			float yaw = AlongLine(m_Wire.FaceOut(face), lineDir);
			// Where the last run's end face is, while runs follow on one from another
			vector chainEnd;
			bool chained = false;
			while (run - used > 0.3)
			{
				if (m_iLayerLaid >= m_iLayerCap)
					return;

				bool placed = false;
				foreach (RZ_DressingPiece w : m_aWires)
				{
					float len = Length(w);
					// A fence closes its gap to the end: its last section may overhang into what closes
					// it at its far end, up to FENCE_OVERHANG
					float overhang = 0;
					if (len > run - used + 0.3)
					{
						if (!w.Rules().m_bFence || len > run - used + FENCE_OVERHANG)
							continue;

						overhang = len - (run - used) + 0.05;
					}

					if (len < FILLER && run - used > shortest + 0.3)
						continue;

					vector dir;
					vector mid = RunPoint(i, j, used + len * 0.5, dir);
					vector axis = RunAxis(yaw, dir);
					if (j == i)
						mid = m_Wire.m_aAt[i];
					else if (chained)
						mid = chainEnd + axis * (len * 0.5);

					array<float> nudges = {0, -0.5, 0.5, -1.0, 1.0, -1.5};
					foreach (float nudge : nudges)
					{
						vector at = RZ_Compass.PointAt(mid, yaw, nudge);
						vector pos = LinePose(w, at, yaw);
						float slope;
						if (!RunFails(w, pos, yaw, slope, false, overhang).IsEmpty())
							continue;

						// Crates, tyres and wrecks sit level or not at all: only wire and fences tilt
						if (w.Rules().m_bLevel)
						{
							if (Math.AbsFloat(slope) > LEVEL_SLOPE)
								continue;

							slope = 0;
						}

						Take(w, pos, yaw, RZ_ESiteLayer.WIRE, false);
						m_Result.m_aPlaced[m_Result.m_aPlaced.Count() - 1].m_fSlope = slope;
						m_iLayerLaid++;
						m_Wire.Cover(face, mid, len * 0.5);
						placed = true;
						used += len;
						chained = nudge == 0;
						chainEnd = mid + axis * (len * 0.5);
						break;
					}

					if (placed)
						break;
				}

				if (!placed)
				{
					used += RZ_SiteWire.STATION;
					chained = false;
				}
			}

			i = j + 1;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Checkpoints (docs/SITES.md): a site that closes every side keeps a few road entrances open
	//! (setting Checkpoints, 2): its main gate and the road crossings furthest from it, so a road
	//! through comes in at one and leaves at the other and a truck can pass. Each gets a sentry's
	//! post and a nest (GatePost, Lookouts). Every other road through its line (and a road running
	//! along it) is closed with the faction's road blocks, the longest that fits first, side by side
	//! across it, each squared to the line; a road closed from side to side is no gate any more.
	//! Nothing else stands on a road: inside the line the roads stay clear.
	protected void SealRoads()
	{
		// The road crossings that aren't the main gate, as runs of stations
		int n = m_Wire.m_aAt.Count();
		array<int> runFrom = {};
		array<int> runTo = {};
		array<float> away = {};
		int i = 0;
		while (i < n)
		{
			int st = m_Wire.m_aState[i];
			if (st != RZ_SiteWire.ST_GATE && st != RZ_SiteWire.ST_ROAD)
			{
				i++;
				continue;
			}

			int face = m_Wire.m_aFace[i];
			int j = i;
			bool main = RZ_Util.DistXZ(m_Wire.m_aAt[i], m_vGate) < 1.5;
			while (j + 1 < n && m_Wire.m_aFace[j + 1] == face && (m_Wire.m_aState[j + 1] == RZ_SiteWire.ST_GATE || m_Wire.m_aState[j + 1] == RZ_SiteWire.ST_ROAD))
			{
				j++;
				if (RZ_Util.DistXZ(m_Wire.m_aAt[j], m_vGate) < 1.5)
					main = true;
			}

			if (!main)
			{
				runFrom.Insert(i);
				runTo.Insert(j);
				away.Insert(RZ_Util.DistXZ((m_Wire.m_aAt[i] + m_Wire.m_aAt[j]) * 0.5, m_vGate));
			}

			i = j + 1;
		}

		// The furthest of them are kept as checkpoints (a gate only: a road along the line doesn't count)
		int keep = m_Settings.m_iCheckpoints - 1;
		array<bool> stays = {};
		for (int u = 0; u < runFrom.Count(); u++)
		{
			stays.Insert(false);
		}

		for (int c = 0; c < keep; c++)
		{
			int best = -1;
			foreach (int r, float d : away)
			{
				if (stays[r] || m_Wire.m_aState[runFrom[r]] != RZ_SiteWire.ST_GATE)
					continue;

				if (best < 0 || d > away[best])
					best = r;
			}

			if (best < 0)
				break;

			stays[best] = true;
		}

		foreach (int run, int start : runFrom)
		{
			if (stays[run])
				continue;

			int stop = runTo[run];
			// A way something closes already (a track that ends at a garden gate, a lane walled off)
			// gets no road block in front of it
			int shut = 0;
			for (int q = start; q <= stop; q++)
			{
				if (m_Wire.Shadowed(m_Grid, q))
					shut++;
			}

			if (shut >= (stop - start + 1) * CLOSED_SHARE)
			{
				m_Wire.Seal(start, stop);
				m_Wire.DropGate((m_Wire.m_aAt[start] + m_Wire.m_aAt[stop]) * 0.5, RZ_SiteWire.GATE_MAX);
				continue;
			}

			// Side by side across the road from its first edge, each block's end face against the
			// last one's (measured), the longest that fits first
			vector roadDir;
			RunPoint(start, stop, 0, roadDir);
			float yaw = AlongLine(m_Wire.FaceOut(m_Wire.m_aFace[start]), roadDir);
			float width = (stop - start + 1) * RZ_SiteWire.STATION;
			float done = 0;
			float left = 0;
			vector blockEnd;
			bool joined = false;
			while (width - done > 0.25)
			{
				bool laid = false;
				foreach (RZ_DressingPiece w : m_aBlocks)
				{
					if (m_iLayerLaid >= m_iLayerCap)
						break;

					float blockLen = Length(w);
					if (blockLen > width - done + 0.6)
						continue;

					vector across;
					vector centre = RunPoint(start, stop, done + blockLen * 0.5, across);
					vector blockAxis = RunAxis(yaw, across);
					if (stop == start)
						centre = m_Wire.m_aAt[start];
					else if (joined)
						centre = blockEnd + blockAxis * (blockLen * 0.5);

					array<float> shifts = {0, -0.5, 0.5, -1.0, 1.0};
					foreach (float shift : shifts)
					{
						vector spot = LinePose(w, RZ_Compass.PointAt(centre, yaw, shift), yaw);
						float tilt;
						if (!RunFails(w, spot, yaw, tilt, true).IsEmpty())
							continue;

						if (w.Rules().m_bLevel)
						{
							if (Math.AbsFloat(tilt) > LEVEL_SLOPE)
								continue;

							tilt = 0;
						}

						Take(w, spot, yaw, RZ_ESiteLayer.BLOCK, false);
						m_Result.m_aPlaced[m_Result.m_aPlaced.Count() - 1].m_fSlope = tilt;
						m_iLayerLaid++;
						int sealFrom = Math.Min(stop, start + RZ_Util.RoundToInt(Math.Floor(done / RZ_SiteWire.STATION)));
						int sealTo = Math.Min(stop, start + RZ_Util.RoundToInt(Math.Ceil((done + blockLen) / RZ_SiteWire.STATION)) - 1);
						m_Wire.Seal(sealFrom, sealTo);
						laid = true;
						done += blockLen;
						joined = shift == 0;
						blockEnd = centre + blockAxis * (blockLen * 0.5);
						break;
					}

					if (laid)
						break;
				}

				if (!laid)
				{
					left += RZ_SiteWire.STATION;
					done += RZ_SiteWire.STATION;
					joined = false;
				}
			}

			// A slit a body can pass (under 1.5 m) still counts as closed: nothing drives through it
			if (left < 1.5)
				m_Wire.DropGate((m_Wire.m_aAt[start] + m_Wire.m_aAt[stop]) * 0.5, RZ_SiteWire.GATE_MAX);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The bearing out of a gate: its side of the line's.
	protected float GateOut(vector gate)
	{
		int nearest = -1;
		float d = float.MAX;
		foreach (int s, vector at : m_Wire.m_aAt)
		{
			float ds = RZ_Util.DistXZ(at, gate);
			if (ds < d)
			{
				d = ds;
				nearest = s;
			}
		}

		if (nearest < 0)
			return m_fGateOut;

		return m_Wire.FaceOut(m_Wire.m_aFace[nearest]);
	}

	// ================================================================ Harden the building
	//------------------------------------------------------------------------------------------------
	//! One entrance: every other outside door of the building blocked; ground-floor windows boarded
	//! (all, or those on the threatened sides); the faction's Traces door board by the entrance.
	protected void Harden()
	{
		RZ_CampSite site = m_Place.m_Site;
		if (!site || m_iHarden == RZ_ESiteHarden.NONE)
			return;

		RZ_DressingPiece doorBoard = LayerPiece(RZ_ESiteLayer.DOOR);
		RZ_DressingPiece windowBoard = LayerPiece(RZ_ESiteLayer.WINDOW);

		// The entrance: the outside door nearest the front door's spot
		array<vector> doors = {};
		foreach (vector d : m_Grid.m_aDoors)
		{
			if (RZ_CampLife.BoxDistanceOf(site, d) <= 1.5)
				doors.Insert(d);
		}

		vector entrance = m_vDoor;
		float near = float.MAX;
		foreach (vector c : doors)
		{
			float dd = RZ_Util.DistXZ(c, m_vDoor);
			if (dd < near)
			{
				near = dd;
				entrance = c;
			}
		}

		int blocked = 0;
		if (doorBoard)
		{
			foreach (vector side : doors)
			{
				if (blocked >= MAX_DOORS || m_iLayerLaid >= m_iLayerCap)
					break;

				if (RZ_Util.DistXZ(side, entrance) < 2.5)
					continue;

				if (OnWall(doorBoard, site, side, false, RZ_ESiteLayer.DOOR))
					blocked++;
			}
		}

		bool boards = m_Settings.m_bBoardWindows && (m_iHarden == RZ_ESiteHarden.THREAT || m_iHarden == RZ_ESiteHarden.ALL);
		int boarded = 0;
		if (windowBoard && boards)
		{
			foreach (vector w : m_Grid.m_aWindows)
			{
				if (boarded >= MAX_WINDOWS || m_iLayerLaid >= m_iLayerCap)
					break;

				if (!OnOutline(site, w))
					continue;

				bool byDoor = false;
				foreach (vector dw : doors)
				{
					if (RZ_Util.DistXZ(dw, w) < 1.6)
						byDoor = true;
				}

				if (byDoor)
					continue;

				if (m_iHarden == RZ_ESiteHarden.THREAT && !Threatened(RZ_Compass.Bearing(m_vMid, w)))
					continue;

				if (OnWall(windowBoard, site, w, true, RZ_ESiteLayer.WINDOW))
					boarded++;
			}
		}

		m_Result.m_iDoorsBlocked = blocked;
		m_Result.m_iWindowsBoarded = boarded;

		// Who holds it: the faction's Traces door board leaned by the entrance, if the kit has one
		ResourceName mark = TraceBoard();
		if (mark.IsEmpty())
			return;

		RZ_DressingPiece markPiece = RZ_DressingPiece.Cast(RZ_Util.NewWithDefaults("RZ_DressingPiece"));
		if (!markPiece)
			return;

		markPiece.m_sPrefab = mark;
		markPiece.m_vFootMin = Vector(-0.4, 0, -0.2);
		markPiece.m_vFootMax = Vector(0.4, 0, 0.2);
		m_Result.m_aKeepAlive.Insert(markPiece);
		float outB = WallOut(site, entrance);
		vector at = RZ_SiteGrid.Frame(entrance, outB, 1.3, 0.4);
		at = RZ_Util.GroundPos(at);
		array<vector> spot = {at};
		array<float> spotYaw = {outB};
		PlaceBest(markPiece, markPiece.Rules(), spot, spotYaw, false, at, RZ_SiteGrid.BLOCKS_LIFE, RZ_ESiteLayer.DOOR, 0.3);
	}

	//------------------------------------------------------------------------------------------------
	//! The faction's door board from World > Traces > Kit (empty if none).
	protected ResourceName TraceBoard()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetTraces() || !gm.GetTraces().GetSettings())
			return "";

		RZ_TraceKit kit = gm.GetTraces().GetSettings().Kit();
		if (!kit)
			return "";

		switch (m_Place.m_eFaction)
		{
			case RZ_EFaction.MILITARY: return kit.m_sBoardBastion;
			case RZ_EFaction.SURVIVORS: return kit.m_sBoardHaven;
			case RZ_EFaction.BANDITS: return kit.m_sBoardRedHand;
			case RZ_EFaction.INSTITUTE: return kit.m_sBoardHalcyon;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A side faces a threatened sector (threat 3 or more, or the front).
	protected bool Threatened(float bearing)
	{
		if (!m_Around)
			return true;

		for (int s = 0; s < RZ_SiteSurroundings.SECTORS; s++)
		{
			if (m_Around.m_aThreat[s] < THREAT_WATCH && s != m_Around.m_iFront)
				continue;

			if (Math.AbsFloat(RZ_Compass.Normalize(bearing - s * 45 + 180) - 180) <= 67.5)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! On the building's outline: within 2.5 m of one of its box's sides (a window of an inner wall
	//! is further in; the box may stand a little proud of the walls).
	protected static bool OnOutline(RZ_CampSite site, vector at)
	{
		vector d = at - site.m_vOrigin;
		vector right = site.m_vRight;
		vector fwd = site.m_vFwd;
		right[1] = 0;
		fwd[1] = 0;
		right.Normalize();
		fwd.Normalize();
		float lx = vector.Dot(d, right);
		float lz = vector.Dot(d, fwd);
		if (lx < site.m_vMins[0] - 1.5 || lx > site.m_vMaxs[0] + 1.5 || lz < site.m_vMins[2] - 1.5 || lz > site.m_vMaxs[2] + 1.5)
			return false;

		float toX = Math.Min(Math.AbsFloat(lx - site.m_vMins[0]), Math.AbsFloat(lx - site.m_vMaxs[0]));
		float toZ = Math.Min(Math.AbsFloat(lz - site.m_vMins[2]), Math.AbsFloat(lz - site.m_vMaxs[2]));
		return Math.Min(toX, toZ) <= 2.5;
	}

	//------------------------------------------------------------------------------------------------
	//! The bearing out of the building's side nearest `at`.
	protected static float WallOut(RZ_CampSite site, vector at)
	{
		vector d = at - site.m_vOrigin;
		vector right = site.m_vRight;
		vector fwd = site.m_vFwd;
		right[1] = 0;
		fwd[1] = 0;
		right.Normalize();
		fwd.Normalize();
		float lx = vector.Dot(d, right);
		float lz = vector.Dot(d, fwd);
		float toX0 = Math.AbsFloat(lx - site.m_vMins[0]);
		float toX1 = Math.AbsFloat(lx - site.m_vMaxs[0]);
		float toZ0 = Math.AbsFloat(lz - site.m_vMins[2]);
		float toZ1 = Math.AbsFloat(lz - site.m_vMaxs[2]);
		float least = Math.Min(Math.Min(toX0, toX1), Math.Min(toZ0, toZ1));
		vector dir = fwd;
		if (least == toX0)
			dir = -right;
		else if (least == toX1)
			dir = right;
		else if (least == toZ0)
			dir = -fwd;

		return RZ_Compass.Bearing(vector.Zero, dir);
	}

	//------------------------------------------------------------------------------------------------
	//! A board against the building's wall over a door or a window: on the wall's face, its front
	//! out; a window's at the window's own height. Stands on the wall, so the grid isn't asked.
	protected bool OnWall(RZ_DressingPiece e, RZ_CampSite site, vector at, bool keepHeight, RZ_ESiteLayer layer)
	{
		float outB = WallOut(site, at);
		// Onto the wall's plane: out of the box by the band, along the wall where the opening is
		float off = RZ_CampLife.BoxDistanceOf(site, at);
		vector pos = RZ_Compass.PointAt(at, outB, 0.15 - Math.Min(off, 0.5));
		float h = at[1];
		pos = RZ_Util.GroundPos(pos);
		if (keepHeight)
			pos[1] = h; // VERIFY: a ground-floor window child's origin is at the middle of its opening, so a board spawned at its height covers it (a door's origin on its sill, not its hinge, so a door board stands across it)

		foreach (RZ_SitePlaced placed : m_Result.m_aPlaced)
		{
			if (placed.m_iLayer == layer && RZ_Util.DistXZ(placed.m_vPos, pos) < 1.0)
				return false;
		}

		// Never across the way kept clear from the entrance, nor on another board
		float lo;
		float hi;
		if (m_Grid.FirstBlock(pos, outB, e.Foot(), RZ_SiteGrid.CLEAR | RZ_SiteGrid.TAKEN, lo, hi) != 0)
			return false;

		Take(e, pos, outB, layer, true);
		m_iLayerLaid++;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The kit's first layer piece of a kind (null: none).
	protected RZ_DressingPiece LayerPiece(RZ_ESiteLayer layer)
	{
		foreach (RZ_DressingPiece e : m_Kit.m_aLayers)
		{
			if (e && e.m_eLayer == layer && !e.m_sPrefab.IsEmpty())
				return e;
		}

		return null;
	}

	// ================================================================ Clear
	//------------------------------------------------------------------------------------------------
	//! The way from the entrance to the gate kept clear; and if there's no way from the entrance to
	//! the kitchen's door (round the building), the kitchen is by the entrance instead.
	protected void Paths()
	{
		KeepPathClear(m_vGate);
		if (m_Place.m_Site && m_Place.m_eKind != RZ_ECampPlace.ROAD_POST && !m_Grid.Path(m_vDoor, m_vKitchen, null, m_Wire))
		{
			float outB = WallOut(m_Place.m_Site, m_vDoor);
			m_vKitchen = RZ_Util.GroundPos(RZ_Compass.PointAt(RZ_Compass.PointAt(m_vDoor, outB + 90, 6), outB, 2));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A way 1.5 m wide from the entrance to `to`, kept clear of work and life.
	protected void KeepPathClear(vector to)
	{
		if (m_Place.m_eKind == RZ_ECampPlace.ROAD_POST)
			return;

		array<vector> route = {};
		if (!m_Grid.Path(m_vDoor, to, route, m_Wire))
		{
			m_Result.m_iBlockedPaths++;
			return;
		}

		foreach (vector pt : route)
		{
			m_Grid.Disc(pt, 0.5, RZ_SiteGrid.CLEAR);
		}
	}

	// ================================================================ Watch
	//------------------------------------------------------------------------------------------------
	//! A lookout toward each threatened side no watch faces yet (the most threatened first, at most
	//! three), just inside the line that way, facing out.
	protected void Lookouts()
	{
		if (!m_Around || m_Place.m_eKind == RZ_ECampPlace.ROAD_POST)
			return;

		array<RZ_DressingPiece> looks = {};
		foreach (RZ_DressingPiece e : m_Kit.m_aLayers)
		{
			if (e && e.m_eLayer == RZ_ESiteLayer.LOOKOUT && !e.m_sPrefab.IsEmpty())
				looks.Insert(e);
		}

		if (looks.IsEmpty())
			return;

		array<int> sectors = {};
		array<float> threats = {};
		for (int s = 0; s < RZ_SiteSurroundings.SECTORS; s++)
		{
			float t = m_Around.m_aThreat[s];
			if (t < THREAT_WATCH && s != m_Around.m_iFront)
				continue;

			int at = 0;
			while (at < threats.Count() && threats[at] >= t)
			{
				at++;
			}

			sectors.InsertAt(s, at);
			threats.InsertAt(t, at);
		}

		int placed = 0;
		// A nest at each other road entrance kept open (docs/SITES.md, "Checkpoints"): just inside it,
		// beside the road, facing out along it
		if (m_Wire && m_Settings.m_iCheckpoints > 1 && m_iClose == RZ_ESiteClose.ALL)
		{
			for (int g = 1; g < m_Wire.m_aGates.Count(); g++)
			{
				if (placed >= MAX_LOOKOUTS || m_iLayerLaid >= m_iLayerCap)
					return;

				vector check = m_Wire.m_aGates[g];
				float checkOut = GateOut(check);
				array<vector> beside = {};
				array<float> besideYaw = {};
				array<float> offs = {5, -5, 6.5, -6.5, 8, -8};
				foreach (float off : offs)
				{
					for (int deep = 0; deep < 2; deep++)
					{
						vector by = RZ_Compass.PointAt(RZ_Compass.PointAt(check, checkOut + 180, 3 + deep * 2.5), checkOut + 90, off);
						beside.Insert(RZ_Util.GroundPos(by));
						besideYaw.Insert(checkOut);
					}
				}

				if (PlaceBest(looks[placed % looks.Count()], m_Plain, beside, besideYaw, false, check, RZ_SiteGrid.BLOCKS_LIFE & ~(RZ_SiteGrid.STEP | RZ_SiteGrid.STEEP), RZ_ESiteLayer.LOOKOUT, 0.5))
					placed++;
			}
		}

		foreach (int sector : sectors)
		{
			if (placed >= MAX_LOOKOUTS || m_iLayerLaid >= m_iLayerCap)
				return;

			float bearing = sector * 45;
			if (Watched(bearing))
				continue;

			RZ_DressingPiece look = looks[placed % looks.Count()];
			array<vector> spots = {};
			array<float> yaws = {};
			vector edge = RZ_Compass.PointAt(m_vMid, bearing, 14);
			if (m_Wire)
			{
				int face;
				edge = m_Wire.LinePointToward(bearing, face);
				edge = RZ_Compass.PointAt(edge, bearing + 180, 3);
			}

			for (int k = 0; k < 9; k++)
			{
				float along = ((k + 1) / 2) * 3;
				if (k % 2 == 1)
					along = -along;

				float back = (k / 3) * 1.5;
				vector spot = RZ_Compass.PointAt(RZ_Compass.PointAt(edge, bearing + 90, along), bearing + 180, back);
				spots.Insert(RZ_Util.GroundPos(spot));
				yaws.Insert(bearing);
			}

			if (PlaceBest(look, m_Plain, spots, yaws, false, edge, RZ_SiteGrid.BLOCKS_LIFE & ~(RZ_SiteGrid.STEP | RZ_SiteGrid.STEEP), RZ_ESiteLayer.LOOKOUT, 0.5))
				placed++;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The building's lookouts (where its sentries stand, RZ_CampLife.LookoutSpot): of five spots
	//! along each side, 2 and 4 m out, the two (a safehouse one) that see the gate (a clear 2 m line
	//! on the grid) and stand nearest it, on ground a body can stand on; one if none sees it.
	protected void BuildingLookouts()
	{
		RZ_CampSite site = m_Place.m_Site;
		if (!site || !m_bGate)
			return;

		array<vector> spots = {};
		array<int> faces = {RZ_CampLife.FACE_FRONT, RZ_CampLife.FACE_BACK, RZ_CampLife.FACE_LEFT, RZ_CampLife.FACE_RIGHT};
		foreach (int f : faces)
		{
			for (int a = -2; a <= 2; a++)
			{
				spots.Insert(m_Camp.FacePointOf(site, f, a * 0.5, 2));
				spots.Insert(m_Camp.FacePointOf(site, f, a * 0.5, 4));
			}
		}

		array<vector> chosen = {};
		array<float> worth = {};
		foreach (vector spot : spots)
		{
			if ((m_Grid.FlagsAt(spot) & (RZ_SiteGrid.INSIDE | RZ_SiteGrid.SOLID | RZ_SiteGrid.WATER | RZ_SiteGrid.TAKEN)) != 0)
				continue;

			float value = -RZ_Util.DistXZ(spot, m_vGate);
			if (m_Grid.LineClear(spot, m_vGate))
				value += 100;

			int at = 0;
			while (at < worth.Count() && worth[at] >= value)
			{
				at++;
			}

			chosen.InsertAt(spot, at);
			worth.InsertAt(value, at);
		}

		int want = 2;
		if (m_Place.m_eKind == RZ_ECampPlace.SAFEHOUSE)
			want = 1;

		site.m_aLookouts.Clear();
		for (int i = 0; i < chosen.Count() && i < want; i++)
		{
			// Only those that see the gate (the best one if none does); a second not on top of the first
			if (i > 0 && (worth[i] < 0 || RZ_Util.DistXZ(chosen[i], site.m_aLookouts[0]) < 4))
			{
				want++;
				continue;
			}

			site.m_aLookouts.Insert(chosen[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A watch piece placed so far faces within 67.5 degrees of `bearing`.
	protected bool Watched(float bearing)
	{
		foreach (RZ_SitePlaced w : m_Result.m_aPlaced)
		{
			if (w.m_Entry.m_eUse != RZ_EDressUse.WATCH || w.m_bIndoors)
				continue;

			if (Math.AbsFloat(RZ_Compass.Normalize(w.m_fYaw - bearing + 180) - 180) <= 67.5)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The lights: one each side of the main gate, then one in each corner of the wire, facing out.
	protected void Lights()
	{
		RZ_DressingPiece light = LayerPiece(RZ_ESiteLayer.LIGHT); // VERIFY: GeneratorFloodlight_US_01 in a composition is lit when it spawns (LIT_ON_SPAWN) and RZ_DressingGuard leaves it (no fuel, storage or ReforgerZ part; its switch action stays)
		if (!light || !m_Wire)
			return;

		array<vector> spots = {};
		array<float> yaws = {};
		vector inGate = RZ_Compass.PointAt(m_vGate, m_fGateOut + 180, 2.5);
		spots.Insert(RZ_Compass.PointAt(inGate, m_fGateOut + 90, 6));
		spots.Insert(RZ_Compass.PointAt(inGate, m_fGateOut - 90, 6));
		yaws.Insert(m_fGateOut);
		yaws.Insert(m_fGateOut);
		for (int k = 0; k < 4; k++)
		{
			float ax;
			float az;
			float bx;
			float bz;
			m_Wire.FaceEnds(k, ax, az, bx, bz);
			vector corner = m_Wire.Point(ax, az);
			float outB = RZ_Compass.Bearing(m_vMid, corner);
			spots.Insert(RZ_Compass.PointAt(corner, outB + 180, 2.5));
			yaws.Insert(outB);
		}

		foreach (int i, vector spot : spots)
		{
			if (m_iLayerLaid >= m_iLayerCap)
				return;

			// Each light tries its own spot and a few round it
			array<vector> near = {RZ_Util.GroundPos(spot)};
			array<float> nearYaw = {yaws[i]};
			for (int r = 0; r < 6; r++)
			{
				near.Insert(RZ_Util.GroundPos(RZ_Compass.PointAt(spot, r * 60, 2)));
				nearYaw.Insert(yaws[i]);
			}

			PlaceBest(light, light.Rules(), near, nearYaw, false, spot, RZ_SiteGrid.BLOCKS_LIFE, RZ_ESiteLayer.LIGHT, 0.5);
		}
	}

	// ================================================================ Work and life
	//------------------------------------------------------------------------------------------------
	//! One plan piece by its rules: indoors as data if it's valuable and there's a building; else at
	//! the best of its candidate spots.
	protected void PlacePlan(RZ_DressingPiece e)
	{
		if (m_iLaid >= m_iCap)
			return;

		RZ_SiteRules rules = e.Rules();
		if (e.m_bIndoors && m_Place.m_Site && m_Settings.m_bLayers)
		{
			RZ_SitePlaced indoors = Record(e, m_vDoor, 0, RZ_ESiteLayer.PLAN);
			indoors.m_bIndoors = true;
			m_Result.m_iIndoors++;
			m_iLaid++;
			return;
		}

		// Spacing: at most one piece of work and life per 12 m2 of the yard inside the wire
		if (m_Wire && e.m_eZone != RZ_ESiteZone.GATE && e.m_eZone != RZ_ESiteZone.WIRE && m_iLife >= YardRoom())
		{
			Drop(e, m_vMid, 0, "Spacing: the yard is full");
			return;
		}

		// The plan's spot: a yard piece's in the frame; a door or front piece's by the building; with
		// no wire, the gate and the closed entries as the plan has them
		// No piece has a spot drawn for it: one by the front door or along the front wall stands where
		// that rule puts it (CampLife.Slot: beside the door, its back to the wall), as does one at the
		// road entries with no wire; every other piece's zone and rules point to its spots, the search
		// starting from the yard's middle (docs/SITES.md, "The solver")
		vector planPos = YardMiddle();
		float planYaw = -1;
		RZ_EDressAt at = RZ_CampLife.AtOfPiece(e);
		bool hasPlan = false;
		if (m_Place.m_Site && (at == RZ_EDressAt.DOOR || at == RZ_EDressAt.FRONT || (!m_Wire && (at == RZ_EDressAt.GATE || at == RZ_EDressAt.CLOSED))))
		{
			m_Camp.PlanSpot(m_Place, m_Lay, e, planPos, planYaw);
			hasPlan = true;
		}

		array<vector> spots = {};
		array<float> yaws = {};
		if (hasPlan)
		{
			spots.Insert(planPos);
			yaws.Insert(planYaw);
		}

		// A road post: on the verges either side of the road, squared to it
		if (m_Place.m_eKind == RZ_ECampPlace.ROAD_POST)
			Verges(spots, yaws);

		Anchors(e, rules, spots, yaws);
		// The rings round the plan's spot if it's in its zone, else round the zone's own middle
		vector anchor = planPos;
		vector zoneAt;
		if (!(hasPlan && InZone(e.m_eZone, planPos, -1)) && ZoneAnchor(e.m_eZone, zoneAt))
			anchor = zoneAt;
		else if (!hasPlan && !spots.IsEmpty())
			anchor = spots[0];

		Around(anchor, spots, yaws);
		int mask = RZ_SiteGrid.BLOCKS_LIFE;
		if (rules.m_bBank)
			mask = mask & ~(RZ_SiteGrid.STEP | RZ_SiteGrid.STEEP);

		if (rules.m_bSoil)
			mask = mask | RZ_SiteGrid.VERGE | RZ_SiteGrid.WALL;

		foreach (RZ_SiteRule rule : rules.m_aRules)
		{
			if (rule.m_iType == RZ_SiteRules.R_AWAY && rule.m_sWhat == "water")
				mask = mask | RZ_SiteGrid.SHORE;
		}

		// Security goes up even on rough ground: a tier 1 piece that fits nowhere is tried again with
		// the ground's flatness eased (up to 20 degrees, a prop's ground up to 0.6 m)
		bool notSecurity = RZ_SiteAudit.TierOf(e, m_Place.m_eFaction) > 1;
		m_bVerge = m_Place.m_eKind == RZ_ECampPlace.ROAD_POST;
		if (m_bVerge)
			mask = mask & ~RZ_SiteGrid.VERGE;

		bool done = PlaceBest(e, rules, spots, yaws, hasPlan, planPos, mask, RZ_ESiteLayer.PLAN, -1, notSecurity);
		m_bVerge = false;
		if (done || notSecurity)
			return;

		m_bRough = true;
		bool placed = PlaceBest(e, rules, spots, yaws, hasPlan, planPos, mask & ~(RZ_SiteGrid.STEP | RZ_SiteGrid.STEEP), RZ_ESiteLayer.PLAN, -1, !m_Wire);
		m_bRough = false;
		if (!placed && m_Wire)
			Anywhere(e, rules, mask);
	}

	//------------------------------------------------------------------------------------------------
	//! A road post's spots: along the road both ways from the post, on each verge, 3 to 7 m off the
	//! road's line, squared to the road (facing along it, each verge's piece the other way).
	protected void Verges(notnull array<vector> spots, notnull array<float> yaws)
	{
		float axis = m_Lay.m_fBearing;
		array<float> along = {0, 3, -3, 6, -6, 9, -9, 12, -12};
		array<float> off = {4, 5.5, 3, 7};
		foreach (float a : along)
		{
			foreach (float o : off)
			{
				for (int side = -1; side <= 1; side += 2)
				{
					vector spot = RZ_Compass.PointAt(RZ_Compass.PointAt(m_Lay.m_vOrigin, axis, a), axis + 90, side * o);
					spots.Insert(RZ_Util.GroundPos(spot));
					float yaw = axis;
					if (side < 0)
						yaw = axis + 180;

					yaws.Insert(RZ_Compass.Normalize(yaw));
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A tier 1 piece's last pass: anywhere inside the wire (its zone eased), on rough ground, the
	//! spots nearest its zone's middle first. Dropped (a hard failure) only if it fits nowhere.
	protected void Anywhere(RZ_DressingPiece e, RZ_SiteRules rules, int mask)
	{
		vector anchor = m_vGate;
		vector zoneAt;
		if (ZoneAnchor(e.m_eZone, zoneAt))
			anchor = zoneAt;

		array<vector> spots = {};
		array<float> yaws = {};
		Around(anchor, spots, yaws);
		m_bRough = true;
		m_bAnyZone = true;
		PlaceBest(e, rules, spots, yaws, false, anchor, mask & ~(RZ_SiteGrid.STEP | RZ_SiteGrid.STEEP), RZ_ESiteLayer.PLAN, -1, true);
		m_bRough = false;
		m_bAnyZone = false;
	}

	//------------------------------------------------------------------------------------------------
	//! The spots a piece's zone and rules point to: the gate's sides, the court by the door, the
	//! kitchen door, the far corners, along the building's walls, round the piece it should be near,
	//! the high ground, a building of the kind it wants.
	protected void Anchors(RZ_DressingPiece e, RZ_SiteRules rules, notnull array<vector> spots, notnull array<float> yaws)
	{
		RZ_CampSite site = m_Place.m_Site;
		float none = -1;
		// A row: beside each of its kind already down, either side, turned as it is
		if (rules.m_bRow)
		{
			float step = e.m_vFootMax[0] - e.m_vFootMin[0] + ROW_GAP;
			foreach (RZ_SitePlaced mate : m_Result.m_aPlaced)
			{
				if (mate.m_Entry != e || mate.m_bIndoors)
					continue;

				spots.Insert(RZ_Util.GroundPos(RZ_SiteGrid.Frame(mate.m_vPos, mate.m_fYaw, step, 0)));
				yaws.Insert(mate.m_fYaw);
				spots.Insert(RZ_Util.GroundPos(RZ_SiteGrid.Frame(mate.m_vPos, mate.m_fYaw, -step, 0)));
				yaws.Insert(mate.m_fYaw);
			}
		}

		switch (e.m_eZone)
		{
			case RZ_ESiteZone.GATE:
			{
				Fan(m_vGate, m_fGateOut + 180, 1, 11, 1.5, 8, spots, yaws);
				break;
			}
			case RZ_ESiteZone.COURT:
			{
				Ring(m_vDoor, 2 + HalfSize(e), 8, spots, yaws);
				Ring(m_vDoor, 5 + HalfSize(e), 8, spots, yaws);
				break;
			}
			case RZ_ESiteZone.KITCHEN:
			{
				vector kitchen = m_vKitchen;
				Ring(kitchen, 2 + HalfSize(e), 8, spots, yaws);
				Ring(kitchen, 5 + HalfSize(e), 8, spots, yaws);
				break;
			}
			case RZ_ESiteZone.STORES:
			{
				if (site && m_iYardFace >= 0)
				{
					for (int s = 0; s < 4; s++)
					{
						spots.Insert(m_Camp.FacePointOf(site, ToCampFace(m_iYardFace), -0.6 + s * 0.4, 1.8));
						yaws.Insert(none);
					}
				}

				break;
			}
			case RZ_ESiteZone.SLEEP:
			case RZ_ESiteZone.SICK:
			{
				FarCorners(spots, yaws);
				break;
			}
			case RZ_ESiteZone.DUMP:
			case RZ_ESiteZone.DEAD:
			{
				if (m_Wire)
				{
					float quiet = QuietBearing();
					int face;
					vector beyond = m_Wire.LinePointToward(quiet, face);
					Ring(RZ_Compass.PointAt(beyond, quiet, 6), 3, 6, spots, yaws);
				}

				break;
			}
		}

		foreach (RZ_SiteRule rule : rules.m_aRules)
		{
			switch (rule.m_iType)
			{
				case RZ_SiteRules.R_NEAR:
				case RZ_SiteRules.R_BY:
				case RZ_SiteRules.R_OVER:
				{
					RZ_SitePlaced target = Nearest(rule.m_sWhat, m_vMid);
					if (target)
					{
						Ring(target.m_vPos, Math.Max(1.5, rule.m_fDist * 0.5) + HalfSize(e), 8, spots, yaws);
						Ring(target.m_vPos, Math.Max(1.5, rule.m_fDist * 0.5) + HalfSize(e) + 2, 8, spots, yaws);
					}

					break;
				}
				case RZ_SiteRules.R_DOOR:
				{
					// Close by the door, then a little further out: the wall's own margin takes the nearest spots
					vector door = DoorOfKind(rule.m_sWhat);
					Ring(door, 1 + HalfSize(e), 8, spots, yaws);
					Ring(door, 3 + HalfSize(e), 8, spots, yaws);
					Ring(door, 6 + HalfSize(e), 8, spots, yaws);
					break;
				}
				case RZ_SiteRules.R_WALL:
				{
					if (site)
					{
						array<int> faces = {RZ_CampLife.FACE_BACK, RZ_CampLife.FACE_LEFT, RZ_CampLife.FACE_RIGHT};
						if (m_iYardFace >= 0)
							faces.InsertAt(ToCampFace(m_iYardFace), 0);

						foreach (int f : faces)
						{
							spots.Insert(m_Camp.FacePointOf(site, f, m_Rng.Range(-0.7, 0.7), 1.2 + (e.m_vFootMax[2] - e.m_vFootMin[2]) * 0.5));
							yaws.Insert(none);
						}

						// An extra has no spot of its own to start from: every side is tried along its length,
						// against the wall and a step off it
						if (e.m_bExtra)
						{
							array<float> along = {-0.6, -0.3, 0, 0.3, 0.6};
							foreach (int side : faces)
							{
								foreach (float t : along)
								{
									spots.Insert(m_Camp.FacePointOf(site, side, t, 1.2 + (e.m_vFootMax[2] - e.m_vFootMin[2]) * 0.5));
									yaws.Insert(none);
									spots.Insert(m_Camp.FacePointOf(site, side, t, 3.5 + (e.m_vFootMax[2] - e.m_vFootMin[2]) * 0.5));
									yaws.Insert(none);
								}
							}
						}
					}

					break;
				}
				case RZ_SiteRules.R_COVERS:
				{
					Fan(m_vGate, m_fGateOut + 180, 3, 12, 2, 8, spots, yaws);
					break;
				}
				case RZ_SiteRules.R_KIND:
				{
					vector kind;
					if (BuildingOfKind(rule.m_sWhat, rule.m_fDist + 20, kind))
						Ring(kind, Math.Max(3, rule.m_fDist * 0.6), 4, spots, yaws);

					break;
				}
			}
		}

		if (rules.m_bHigh && m_Around)
		{
			spots.Insert(RZ_Util.GroundPos(m_Around.m_vHigh));
			yaws.Insert(none);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! `count` spots on a circle round `c` (the seed turns where it starts).
	protected void Ring(vector c, float radius, int count, notnull array<vector> spots, notnull array<float> yaws)
	{
		float start = m_Rng.Range(0, 360);
		for (int i = 0; i < count; i++)
		{
			spots.Insert(RZ_Util.GroundPos(RZ_Compass.PointAt(c, start + i * 360 / count, radius)));
			yaws.Insert(-1);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Rings round the anchor (1.5 to 13 m), then spots spread over the yard by the seed. Spots
	//! whose middle is blocked or out of the zone are passed over cheaply; Candidates a piece of the
	//! rest are scored (PlaceBest).
	protected void Around(vector anchor, notnull array<vector> spots, notnull array<float> yaws)
	{
		array<float> radii = {1.5, 3, 4.5, 6, 8, 10, 13};
		foreach (float r : radii)
		{
			Ring(anchor, r, 8, spots, yaws);
		}

		for (int k = 0; k < 24; k++)
		{
			vector spot;
			if (m_Wire)
			{
				float x = m_Rng.Range(m_Wire.m_fX0 + WIRE_IN, m_Wire.m_fX1 - WIRE_IN);
				float z = m_Rng.Range(m_Wire.m_fZ0 + WIRE_IN, m_Wire.m_fZ1 - WIRE_IN);
				spot = m_Wire.Point(x, z);
			}
			else
			{
				spot = RZ_Util.GroundPos(RZ_Compass.PointAt(anchor, m_Rng.Range(0, 360), m_Rng.Range(2, 16)));
			}

			spots.Insert(spot);
			yaws.Insert(-1);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Spots in a fan from `c` toward `bearing`: `depth` steps in, out to `reach` either side.
	protected void Fan(vector c, float bearing, float firstIn, float reach, float step, int count, notnull array<vector> spots, notnull array<float> yaws)
	{
		for (int d = 0; d < 3; d++)
		{
			vector row = RZ_Compass.PointAt(c, bearing, firstIn + d * step);
			for (int a = 0; a <= count; a++)
			{
				float along = -reach + 2 * reach * a / Math.Max(1, count);
				if (Math.AbsFloat(along) < 3.5)
					continue;

				spots.Insert(RZ_Util.GroundPos(RZ_Compass.PointAt(row, bearing + 90, along)));
				yaws.Insert(-1);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! How many pieces of work and life the yard holds: one per 12 m2 inside the wire, less the building.
	protected int YardRoom()
	{
		float area = (m_Wire.m_fX1 - m_Wire.m_fX0) * (m_Wire.m_fZ1 - m_Wire.m_fZ0);
		if (m_Place.m_Site)
		{
			vector size = m_Place.m_Site.m_vMaxs - m_Place.m_Site.m_vMins;
			area -= size[0] * size[2];
		}

		int room = RZ_Util.RoundToInt(Math.Floor(area / 12));
		if (room < 4)
			return 4;

		return room;
	}

	//------------------------------------------------------------------------------------------------
	//! The middle of a zone, where its spots are sought when the plan's spot isn't in it.
	protected bool ZoneAnchor(RZ_ESiteZone zone, out vector at)
	{
		switch (zone)
		{
			case RZ_ESiteZone.GATE: at = RZ_Compass.PointAt(m_vGate, m_fGateOut + 180, 4); return true;
			case RZ_ESiteZone.YARD: at = YardMiddle(); return true;
			case RZ_ESiteZone.COURT: at = m_vDoor; return true;
			case RZ_ESiteZone.KITCHEN: at = m_vKitchen; return true;
			case RZ_ESiteZone.STORES:
			case RZ_ESiteZone.WORK:
			{
				if (!m_Place.m_Site)
					return false;

				int yardFace = m_iYardFace;
				if (yardFace < 0)
					yardFace = 0;

				at = m_Camp.FacePointOf(m_Place.m_Site, ToCampFace(yardFace), 0, 3);
				return true;
			}
			case RZ_ESiteZone.SLEEP:
			case RZ_ESiteZone.SICK:
			{
				array<vector> far = {};
				array<float> farYaw = {};
				FarCorners(far, farYaw);
				if (far.IsEmpty())
					return false;

				at = far[0];
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Half a piece's footprint's longer side.
	protected static float HalfSize(RZ_DressingPiece e)
	{
		vector size = e.m_vFootMax - e.m_vFootMin;
		return Math.Max(size[0], size[2]) * 0.5;
	}

	//------------------------------------------------------------------------------------------------
	//! Places a piece at the best of `spots`: each must be in its zone and meet every hard rule and
	//! the ground; then the best few by score are traced in turn. `margin`: how far inside the wire
	//! (-1: work and life's own). False, and dropped (a plan piece), if none fits.
	protected bool PlaceBest(RZ_DressingPiece e, RZ_SiteRules rules, array<vector> spots, array<float> yaws, bool hasPlan, vector planPos, int mask, RZ_ESiteLayer layer, float margin, bool canDrop = true)
	{
		// Candidates a piece are scored; twice as many for security and what the site is for
		int limit = Math.Max(4, m_Settings.m_iCandidates);
		if (RZ_SiteAudit.TierOf(e, m_Place.m_eFaction) <= 2)
			limit *= 2;

		int scored = 0;
		array<vector> bestPos = {};
		array<float> bestYaw = {};
		array<float> bestScore = {};
		string first = "";
		map<string, int> reasons = new map<string, int>();
		foreach (int i, vector spot : spots)
		{
			if (scored >= limit)
				break;

			// Cheap first: its middle in its zone and free
			if (!InZone(e.m_eZone, spot, margin))
			{
				Count(reasons, "outside its zone (" + typename.EnumToString(RZ_ESiteZone, e.m_eZone) + ")");
				continue;
			}

			int mid = m_Grid.FlagsAt(spot) & mask;
			if (mid != 0)
			{
				Count(reasons, RZ_SiteGrid.FlagName(mid));
				continue;
			}

			scored++;
			float yaw = FacesYaw(rules, spot, yaws[i], hasPlan && i == 0);
			vector pos = spot;
			if (rules.m_bJitter)
			{
				yaw = RZ_Compass.Normalize(yaw + m_Rng.Range(-8, 8));
				pos = RZ_Util.GroundPos(RZ_Compass.PointAt(pos, m_Rng.Range(0, 360), m_Rng.Range(0, 0.5)));
			}

			string why = HardFails(e, rules, pos, yaw, mask, margin);
			if (!why.IsEmpty())
			{
				Count(reasons, why);
				continue;
			}

			float score = Score(e, rules, pos, hasPlan, planPos);
			int at = 0;
			while (at < bestScore.Count() && bestScore[at] >= score)
			{
				at++;
			}

			if (at >= TOP)
				continue;

			bestPos.InsertAt(pos, at);
			bestYaw.InsertAt(yaw, at);
			bestScore.InsertAt(score, at);
			if (bestPos.Count() > TOP)
			{
				bestPos.Remove(TOP);
				bestYaw.Remove(TOP);
				bestScore.Remove(TOP);
			}
		}

		bool wall = layer == RZ_ESiteLayer.DOOR;
		foreach (int k, vector chosen : bestPos)
		{
			if (!m_Camp.Traced(chosen, bestYaw[k], e))
			{
				first = "something in the way";
				continue;
			}

			// The cook pot: people must reach it from the door (and the way is kept clear)
			array<vector> cookRoute = {};
			if (e.m_eUse == RZ_EDressUse.COOK && m_Place.m_eKind != RZ_ECampPlace.ROAD_POST && !m_Grid.Path(m_vDoor, RZ_SiteAudit.FrontOf(e, chosen, bestYaw[k]), cookRoute, m_Wire))
			{
				first = "no way to it from the door";
				continue;
			}

			foreach (vector step : cookRoute)
			{
				m_Grid.Disc(step, 0.5, RZ_SiteGrid.CLEAR);
			}

			Take(e, chosen, bestYaw[k], layer, wall);
			if (layer == RZ_ESiteLayer.PLAN)
			{
				m_iLaid++;
				if (e.m_eZone != RZ_ESiteZone.GATE && e.m_eZone != RZ_ESiteZone.WIRE)
					m_iLife++;
			}
			else
			{
				m_iLayerLaid++;
			}

			return true;
		}

		if (layer != RZ_ESiteLayer.PLAN || !canDrop)
			return false;

		// Why: something in the way of the best spots, else the commonest reason (a zone only if
		// nothing else was met)
		string dropWhy = first;
		if (dropWhy.IsEmpty())
		{
			int most = 0;
			foreach (string reason, int count : reasons)
			{
				int weight = count * 4;
				if (reason.IndexOf("outside its zone") == 0)
					weight = count;

				if (weight > most)
				{
					most = weight;
					dropWhy = reason;
				}
			}
		}

		if (dropWhy.IsEmpty())
			dropWhy = "no room";

		float dropYaw = 0;
		if (!yaws.IsEmpty())
			dropYaw = Math.Max(0, yaws[0]);

		Drop(e, planPos, dropYaw, dropWhy);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Count(map<string, int> reasons, string why)
	{
		int seen = 0;
		reasons.Find(why, seen);
		reasons.Set(why, seen + 1);
	}

	//------------------------------------------------------------------------------------------------
	//! The first hard rule a spot fails ("" = none): its zone, inside the wire, away from, the ground.
	protected string HardFails(RZ_DressingPiece e, RZ_SiteRules rules, vector pos, float yaw, int mask, float margin)
	{
		if (!InZone(e.m_eZone, pos, margin))
			return "outside its zone (" + typename.EnumToString(RZ_ESiteZone, e.m_eZone) + ")";

		// A road post's pieces stand beside the road (within 2 m of it), not out in the field
		if (m_bVerge && (m_Grid.FlagsAt(pos) & RZ_SiteGrid.VERGE) == 0)
			return "not beside the road";

		foreach (RZ_SiteRule rule : rules.m_aRules)
		{
			if (rule.m_iType == RZ_SiteRules.R_BY)
			{
				// It serves something: no such thing here, or too far from it, and it isn't put down
				RZ_SitePlaced served = Nearest(rule.m_sWhat, pos);
				if (!served || RZ_Util.DistXZ(served.m_vPos, pos) > rule.m_fDist + HalfSize(e) + HalfSize(served.m_Entry))
					return RZ_SiteRules.Text(rule);

				continue;
			}

			if (rule.m_iType != RZ_SiteRules.R_AWAY)
				continue;

			float d = AwayDistance(rule.m_sWhat, pos);
			if (d < rule.m_fDist)
				return RZ_SiteRules.Text(rule);
		}

		return Fails(e, rules, pos, yaw, mask, FLAT_SLACK);
	}

	//------------------------------------------------------------------------------------------------
	//! The ground under the whole footprint (the grid's cells, flat enough), off the line and clear
	//! of the spots kept free ("" = it may stand there).
	protected string Fails(RZ_DressingPiece e, RZ_SiteRules rules, vector pos, float yaw, int mask, float slack)
	{
		RZ_SiteFoot foot = e.Foot();
		float lo;
		float hi;
		int blocks = m_Grid.FirstBlock(pos, yaw, foot, mask, lo, hi);
		if (blocks != 0)
			return RZ_SiteGrid.FlagName(blocks);

		if (rules.m_bSoil && Paved(e, pos, yaw))
			return "paved ground";

		float slope = FLAT_SLOPE;
		if (rules.m_fFlat >= 0)
			slope = Math.Tan(rules.m_fFlat * Math.DEG2RAD);

		float propSpread = PROP_SPREAD;
		if (m_bRough || rules.m_bBank)
		{
			slope = ROUGH_SLOPE;
			propSpread = ROUGH_SPREAD;
		}

		vector size = e.m_vFootMax - e.m_vFootMin;
		float diag = Math.Sqrt(size[0] * size[0] + size[2] * size[2]);
		if (hi - lo > slope * diag + slack)
			return "not flat";

		// No prop of it floats or sinks: the ground under each varies at most 0.3 m (the audit's test)
		for (int r = 0; r < foot.m_iProps; r++)
		{
			if (m_Grid.RectSpread(pos, yaw, foot, r) > propSpread)
				return "not flat";
		}

		array<vector> quad = {};
		RZ_CampLife.QuadOf(pos, yaw, e.m_vFootMin, e.m_vFootMax, 0, quad);
		vector mid = (quad[0] + quad[2]) * 0.5;
		if (RZ_Perimeter.IsNearLineAt(RZ_Util.GroundPos(mid)))
			return "the line";

		foreach (int n, vector keep : m_Fit.m_aKeep)
		{
			if (RZ_CampLife.QuadDistanceOf(keep, quad) < m_Fit.m_aKeepR[n])
				return "a spot kept clear";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Soft ground only (rule "soil": a tent, a bed, a garden, a line): true if the middle or a corner
	//! of the footprint is on a paved surface. The grid knows roads and their verges; a yard's
	//! cobbles or a forecourt's asphalt it doesn't, so the ground's own material is read here.
	protected bool Paved(RZ_DressingPiece e, vector pos, float yaw)
	{
		array<vector> quad = {};
		RZ_CampLife.QuadOf(pos, yaw, e.m_vFootMin, e.m_vFootMax, 0, quad);
		quad.Insert((quad[0] + quad[2]) * 0.5);
		foreach (vector corner : quad)
		{
			if (RZ_SiteGrid.PavedAt(RZ_Util.GroundPos(corner), m_mGround))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! In its zone (docs/SITES.md, "Zones"). With no wire, every zone is anywhere.
	protected bool InZone(RZ_ESiteZone zone, vector pos, float margin)
	{
		if (!m_Wire)
			return true;

		// A story element outside the wire: past the line, not far
		if (m_bStoryOut)
			return !m_Wire.Inside(pos, -1.5) && m_Wire.DistToLine(pos) <= 45;

		float m = WIRE_IN;
		if (margin >= 0)
			m = margin;

		bool inside = m_Wire.Inside(pos, m);
		if (m_bAnyZone)
			return m_Wire.Inside(pos, 0.5);

		switch (zone)
		{
			case RZ_ESiteZone.WIRE: return m_Wire.DistToLine(pos) <= 1.5;
			case RZ_ESiteZone.GATE: return RZ_Util.DistXZ(pos, m_vGate) <= 16 && (m_Wire.Inside(pos, 0.3) || m_Wire.DistToLine(pos) <= 1.5);
			case RZ_ESiteZone.KILL: return false;
			case RZ_ESiteZone.COURT: return inside && RZ_Util.DistXZ(pos, m_vDoor) <= 18;
			case RZ_ESiteZone.STORES: return inside && m_Place.m_Site && RZ_CampLife.BoxDistanceOf(m_Place.m_Site, pos) <= 4;
			case RZ_ESiteZone.WORK: return inside && (!m_Place.m_Site || RZ_CampLife.BoxDistanceOf(m_Place.m_Site, pos) <= 15);
			case RZ_ESiteZone.KITCHEN: return inside && RZ_Util.DistXZ(pos, m_vKitchen) <= 14;
			case RZ_ESiteZone.SLEEP: return inside && RZ_Util.DistXZ(pos, m_vGate) >= 12;
			case RZ_ESiteZone.SICK: return inside && RZ_Util.DistXZ(pos, m_vGate) >= 8;
			case RZ_ESiteZone.DUMP:
			case RZ_ESiteZone.DEAD:
			{
				if (m_Wire.Inside(pos, -3) || m_Wire.DistToLine(pos) > 20)
					return false;

				return !Threatened(RZ_Compass.Bearing(m_vMid, pos));
			}
		}

		return inside;
	}

	//------------------------------------------------------------------------------------------------
	//! How far a spot is from what an Away rule names: a piece, water, the gate, the wire, the door, a road.
	protected float AwayDistance(string what, vector pos)
	{
		switch (what)
		{
			case "water": return 1000;	// the mask's shore cells (4 m) say it
			case "gate": return RZ_Util.DistXZ(pos, m_vGate);
			case "door": return RZ_Util.DistXZ(pos, m_vDoor);
			case "wire":
			{
				if (!m_Wire)
					return 1000;

				return m_Wire.DistToLine(pos);
			}
			case "road":
			{
				float best = 1000;
				for (int i = 0; i + 1 < m_Fit.m_aRoad.Count(); i += 2)
				{
					best = Math.Min(best, RZ_DressPlacer.DistToSegment(pos, m_Fit.m_aRoad[i], m_Fit.m_aRoad[i + 1]));
				}

				return best;
			}
		}

		RZ_SitePlaced near = Nearest(what, pos);
		if (!near)
			return 1000;

		return RZ_Util.DistXZ(near.m_vPos, pos);
	}

	//------------------------------------------------------------------------------------------------
	//! The soft rules' score at a spot (higher is better).
	protected float Score(RZ_DressingPiece e, RZ_SiteRules rules, vector pos, bool hasPlan, vector planPos)
	{
		float score = 0;
		if (hasPlan)
			score -= PLAN_WEIGHT * RZ_Util.DistXZ(pos, planPos);

		RZ_CampSite site = m_Place.m_Site;
		foreach (RZ_SiteRule rule : rules.m_aRules)
		{
			switch (rule.m_iType)
			{
				case RZ_SiteRules.R_NEAR:
				case RZ_SiteRules.R_BY:
				{
					RZ_SitePlaced near = Nearest(rule.m_sWhat, pos);
					if (near)
						score += Within(RZ_Util.DistXZ(near.m_vPos, pos), rule.m_fDist, 10);

					break;
				}
				case RZ_SiteRules.R_OVER:
				{
					RZ_SitePlaced over = Nearest(rule.m_sWhat, pos);
					if (over)
						score += Within(RZ_Util.DistXZ(over.m_vPos, pos), 2, 12);

					break;
				}
				case RZ_SiteRules.R_DOOR:
				{
					score += Within(RZ_Util.DistXZ(DoorOfKind(rule.m_sWhat), pos), rule.m_fDist, 8);
					break;
				}
				case RZ_SiteRules.R_WALL:
				{
					if (site)
						score += Within(RZ_CampLife.BoxDistanceOf(site, pos), rule.m_fDist + (e.m_vFootMax[2] - e.m_vFootMin[2]) * 0.5, 8);

					break;
				}
				case RZ_SiteRules.R_WINDOW:
				{
					score += Within(NearestOf(m_Grid.m_aWindows, pos), rule.m_fDist, 6);
					break;
				}
				case RZ_SiteRules.R_CORNER:
				{
					if (site)
						score += Within(CornerDistance(site, pos), rule.m_fDist, 6);

					break;
				}
				case RZ_SiteRules.R_KIND:
				{
					vector kind;
					if (BuildingOfKind(rule.m_sWhat, rule.m_fDist + 20, kind))
						score += Within(RZ_Util.DistXZ(kind, pos), rule.m_fDist, 6);

					break;
				}
				case RZ_SiteRules.R_COVERS:
				{
					vector eye = pos;
					if (m_Grid.LineClear(eye, CoverTarget(rule.m_sWhat, pos)))
						score += 15;
					else
						score -= 15;

					break;
				}
				case RZ_SiteRules.R_BEHIND:
				{
					vector away = pos - m_vMid;
					vector front = RZ_Compass.PointAt(vector.Zero, m_fFront, 1);
					if (vector.Dot(away, front) < 0)
						score += 6;
					else
						score -= 3;

					break;
				}
			}
		}

		if (rules.m_bGuarded)
		{
			if (m_Wire)
				score += Math.Min(12, m_Wire.DistToLine(pos)) * 0.4;

			score -= RZ_Util.DistXZ(pos, m_vDoor) * 0.3;
			if (site && RZ_CampLife.BoxDistanceOf(site, pos) < 3)
				score += 3;
		}

		if (rules.m_bHigh)
		{
			float rise = m_Grid.HeightAt(pos[0], pos[2]) - m_Grid.HeightAt(m_vMid[0], m_vMid[2]);
			score += Math.Clamp(rise * 2, -4, 12);
		}

		// A row: right beside one of its kind
		if (rules.m_bRow)
		{
			float beside = e.m_vFootMax[0] - e.m_vFootMin[0] + ROW_GAP + 0.6;
			foreach (RZ_SitePlaced mate : m_Result.m_aPlaced)
			{
				if (mate.m_Entry == e && !mate.m_bIndoors && RZ_Util.DistXZ(mate.m_vPos, pos) <= beside)
				{
					score += 14;
					break;
				}
			}
		}

		if (rules.m_bNoLine && m_Wire)
		{
			vector l = m_Wire.Local(pos);
			foreach (RZ_SitePlaced other : m_Result.m_aPlaced)
			{
				if (other.m_Entry != e)
					continue;

				vector ol = m_Wire.Local(other.m_vPos);
				if (Math.AbsFloat(ol[0] - l[0]) < 0.4 || Math.AbsFloat(ol[2] - l[2]) < 0.4)
					score -= 4;
			}
		}

		return score;
	}

	//------------------------------------------------------------------------------------------------
	//! A soft rule's worth: full within `reach`, falling 2 a metre past it, at least -worth.
	protected static float Within(float d, float reach, float worth)
	{
		if (d <= reach)
			return worth;

		return Math.Max(-worth, worth - 2 * (d - reach));
	}

	//------------------------------------------------------------------------------------------------
	//! Which way a piece turns: its Faces rule, else the plan's own turn, else toward the yard's middle.
	protected float FacesYaw(RZ_SiteRules rules, vector pos, float given, bool planSpot)
	{
		string faces = rules.m_sFaces;
		if (faces.IsEmpty())
		{
			if (given >= 0)
				return given;

			// The first of a row is squared to the building (to the road in on open ground)
			if (rules.m_bRow)
			{
				if (m_Place.m_Site)
					return WallOut(m_Place.m_Site, pos);

				return m_fGateOut;
			}

			return RZ_Compass.Bearing(pos, m_vMid);
		}

		switch (faces)
		{
			case "front": return m_fGateOut;
			case "road": return m_fGateOut;
			case "threat": return m_fFront;
			case "out": return RZ_Compass.Bearing(m_vMid, pos);
			case "in": return RZ_Compass.Bearing(pos, m_vMid);
			case "gate": return RZ_Compass.Bearing(pos, m_vGate);
			case "door": return RZ_Compass.Bearing(pos, m_vDoor);
			case "wall":
			{
				if (m_Place.m_Site)
					return WallOut(m_Place.m_Site, pos);

				return RZ_Compass.Bearing(m_vMid, pos);
			}
		}

		RZ_SitePlaced target = Nearest(faces, pos);
		if (target)
			return RZ_Compass.Bearing(pos, target.m_vPos);

		if (given >= 0)
			return given;

		return RZ_Compass.Bearing(pos, m_vMid);
	}

	//------------------------------------------------------------------------------------------------
	//! What a Covers rule must see from `pos`: the gate, 30 m out along the road, 40 m toward the
	//! threat, or 30 m straight out.
	protected vector CoverTarget(string what, vector pos)
	{
		switch (what)
		{
			case "gate": return m_vGate;
			case "road": return RZ_Compass.PointAt(m_vGate, m_fGateOut, 30);
			case "threat": return RZ_Compass.PointAt(pos, m_fFront, 40);
		}

		return RZ_Compass.PointAt(pos, RZ_Compass.Bearing(m_vMid, pos), 30);
	}

	// ================================================================ The stores board
	//------------------------------------------------------------------------------------------------
	//! Once valuables went indoors: the stores board by the entrance, facing out.
	protected void StoresBoard()
	{
		if (m_Result.m_iIndoors <= 0)
			return;

		RZ_DressingPiece board = LayerPiece(RZ_ESiteLayer.STORES);
		if (!board)
			return;

		array<vector> spots = {};
		array<float> yaws = {};
		float outB = RZ_Compass.Bearing(m_vMid, m_vDoor);
		if (m_Place.m_Site)
			outB = WallOut(m_Place.m_Site, m_vDoor);

		for (int k = 0; k < 8; k++)
		{
			float along = 2 + (k / 2) * 1.2;
			if (k % 2 == 1)
				along = -along;

			spots.Insert(RZ_Util.GroundPos(RZ_Compass.PointAt(RZ_Compass.PointAt(m_vDoor, outB + 90, along), outB, 0.5 + (k / 4) * 1.5)));
			yaws.Insert(outB);
		}

		PlaceBest(board, board.Rules(), spots, yaws, false, m_vDoor, RZ_SiteGrid.BLOCKS_LIFE, RZ_ESiteLayer.STORES, 0.5);
	}

	// ================================================================ Helpers
	//------------------------------------------------------------------------------------------------
	//! A piece placed: as data, its cells taken; `wall`: it stands on the building's wall.
	protected void Take(RZ_DressingPiece e, vector pos, float yaw, RZ_ESiteLayer layer, bool wall)
	{
		// A barrier run or a road block takes no more than its measured box past its ends: the next run
		// meets its end face (docs/SITES.md, "Spacing")
		bool line = layer == RZ_ESiteLayer.WIRE || layer == RZ_ESiteLayer.BLOCK;
		m_Grid.Paint(pos, yaw, e.Foot(), RZ_SiteGrid.TAKEN, line);
		RZ_SitePlaced placed = Record(e, pos, yaw, layer);
		placed.m_bWall = wall;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_SitePlaced Record(RZ_DressingPiece e, vector pos, float yaw, RZ_ESiteLayer layer)
	{
		RZ_SitePlaced placed = new RZ_SitePlaced();
		placed.m_Entry = e;
		placed.m_vPos = pos;
		placed.m_fYaw = yaw;
		placed.m_iLayer = layer;
		m_Result.m_aPlaced.Insert(placed);
		return placed;
	}

	//------------------------------------------------------------------------------------------------
	protected void Drop(RZ_DressingPiece e, vector pos, float yaw, string why)
	{
		RZ_SitePlaced dropped = new RZ_SitePlaced();
		dropped.m_Entry = e;
		dropped.m_vPos = pos;
		dropped.m_fYaw = yaw;
		dropped.m_sWhy = why;
		m_Result.m_aDropped.Insert(dropped);
	}

	//------------------------------------------------------------------------------------------------
	//! The placed piece `what` names nearest `pos` (null: none placed yet).
	protected RZ_SitePlaced Nearest(string what, vector pos)
	{
		RZ_SitePlaced best;
		float near = float.MAX;
		foreach (RZ_SitePlaced placed : m_Result.m_aPlaced)
		{
			if (placed.m_bIndoors || !RZ_SiteRules.Names(what, RZ_SiteAudit.PieceName(placed.m_Entry), placed.m_Entry.m_eUse))
				continue;

			float d = RZ_Util.DistXZ(placed.m_vPos, pos);
			if (d < near)
			{
				near = d;
				best = placed;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	protected static float NearestOf(array<vector> points, vector pos)
	{
		float best = 1000;
		foreach (vector pt : points)
		{
			best = Math.Min(best, RZ_Util.DistXZ(pt, pos));
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! How far a spot is from the nearest corner of the building.
	protected static float CornerDistance(RZ_CampSite site, vector pos)
	{
		float best = float.MAX;
		array<float> xs = {site.m_vMins[0], site.m_vMaxs[0]};
		array<float> zs = {site.m_vMins[2], site.m_vMaxs[2]};
		foreach (float x : xs)
		{
			foreach (float z : zs)
			{
				vector c = site.m_vOrigin + site.m_vRight * x + site.m_vFwd * z;
				best = Math.Min(best, RZ_Util.DistXZ(c, pos));
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! The door of a kind: front (the entrance), side (the nearest other outside door to the yard),
	//! back (the one furthest from the front), any (the nearest to the yard).
	protected vector DoorOfKind(string kind)
	{
		if (kind == "front" || !m_Place.m_Site)
			return m_vDoor;

		if (kind == "back" || kind == "side")
			return m_vKitchen;

		return m_vDoor;
	}

	//------------------------------------------------------------------------------------------------
	//! The kitchen's door: the building's outside door furthest from the entrance (its back or side
	//! door), else the middle of its yard side, else the yard's middle.
	protected vector KitchenDoor()
	{
		RZ_CampSite site = m_Place.m_Site;
		if (!site)
			return m_Lay.m_vOrigin;

		vector best = m_Camp.FacePointOf(site, RZ_CampLife.FACE_BACK, 0, 1.5);
		if (m_iYardFace >= 0)
			best = m_Camp.FacePointOf(site, ToCampFace(m_iYardFace), 0, 1.5);

		float far = 3;
		foreach (vector d : m_Grid.m_aDoors)
		{
			if (RZ_CampLife.BoxDistanceOf(site, d) > 1.5)
				continue;

			float dd = RZ_Util.DistXZ(d, m_vDoor);
			if (dd > far)
			{
				far = dd;
				best = RZ_Util.GroundPos(RZ_Compass.PointAt(d, WallOut(site, d), 1.5));
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Camp life's face (FACE_*) of a wire side.
	protected int ToCampFace(int wireFace)
	{
		RZ_CampSite site = m_Place.m_Site;
		array<int> faces = {RZ_CampLife.FACE_FRONT, RZ_CampLife.FACE_BACK, RZ_CampLife.FACE_LEFT, RZ_CampLife.FACE_RIGHT};
		foreach (int f : faces)
		{
			if (WireFace(site, f) == wireFace)
				return f;
		}

		return RZ_CampLife.FACE_BACK;
	}

	//------------------------------------------------------------------------------------------------
	//! The yard's two corners furthest from the gate, pulled 4 m in.
	protected void FarCorners(notnull array<vector> spots, notnull array<float> yaws)
	{
		if (!m_Wire)
			return;

		array<vector> corners = {};
		array<float> far = {};
		for (int k = 0; k < 4; k++)
		{
			float ax;
			float az;
			float bx;
			float bz;
			m_Wire.FaceEnds(k, ax, az, bx, bz);
			vector c = m_Wire.Point(ax, az);
			c = RZ_Util.GroundPos(RZ_Compass.PointAt(c, RZ_Compass.Bearing(c, m_vMid), 4));
			float d = RZ_Util.DistXZ(c, m_vGate);
			int at = 0;
			while (at < far.Count() && far[at] >= d)
			{
				at++;
			}

			corners.InsertAt(c, at);
			far.InsertAt(d, at);
		}

		for (int i = 0; i < 2; i++)
		{
			spots.Insert(corners[i]);
			yaws.Insert(-1);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The quietest way out: the side with the least threat, not the front.
	protected float QuietBearing()
	{
		if (!m_Around)
			return RZ_Compass.Normalize(m_fGateOut + 180);

		int best = 0;
		float least = float.MAX;
		for (int s = 0; s < RZ_SiteSurroundings.SECTORS; s++)
		{
			if (s == m_Around.m_iFront)
				continue;

			if (m_Around.m_aThreat[s] < least)
			{
				least = m_Around.m_aThreat[s];
				best = s;
			}
		}

		return best * 45;
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest building of a kind (garage, farm, church, shop, clinic, house, public, or a word in
	//! its prefab's name: well, pump) within reach of the middle.
	protected bool BuildingOfKind(string kind, float reach, out vector where)
	{
		float best = reach + 30;
		bool found = false;
		foreach (IEntity b : m_Grid.GetBuildings())
		{
			if (!b)
				continue;

			float d = RZ_Util.DistXZ(b.GetOrigin(), m_vMid);
			if (d > best)
				continue;

			string name = RZ_Util.GetPrefabName(b);
			name.ToLower();
			string want = typename.EnumToString(RZ_EDoorWant, RZ_DoorSlots.KindOf(b));
			want.ToLower();
			if (want != kind && !name.Contains(kind))
				continue;

			best = d;
			where = b.GetOrigin();
			found = true;
		}

		return found;
	}

	// ================================================================ The gate's post
	//------------------------------------------------------------------------------------------------
	//! The gate's post (docs/SITES.md, "Watch"): a spot just inside the main gate, beside the way in,
	//! on ground a body can stand on, with a clear line to the gate; the first of 3-7 m in and 2.5-6 m
	//! to either side (off the road and the way kept clear if it can), else the nearest square a body
	//! can stand on by the gate. Every site with a gate has one, so its gate is always watched: a
	//! sentry stands there whatever the nest can see.
	protected void GatePost()
	{
		if (!m_bGate || m_Place.m_eKind == RZ_ECampPlace.ROAD_POST)
			return;

		m_Result.m_vGatePost = GatePostAt(m_vGate, m_fGateOut);
		m_Result.m_bGatePost = true;

		// The other road entrances kept open: a checkpoint's post at each
		if (!m_Wire || m_Settings.m_iCheckpoints < 2 || m_iClose != RZ_ESiteClose.ALL)
			return;

		for (int g = 1; g < m_Wire.m_aGates.Count(); g++)
		{
			float gateOut = GateOut(m_Wire.m_aGates[g]);
			m_Result.m_aCheckposts.Insert(GatePostAt(m_Wire.m_aGates[g], gateOut));
			m_Result.m_aCheckOut.Insert(gateOut);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A sentry's spot at a gate: just inside it, 3-7 m in and 2.5-6 m to the side of the way in, on
	//! ground a body can stand on (off the road and the way kept clear if it can), with a clear line
	//! to the gate; else any square a body can stand on within 8 m that sees it; else the gate itself.
	protected vector GatePostAt(vector gate, float gateOut)
	{
		array<float> ins = {3, 5, 7};
		array<float> sides = {2.5, -2.5, 4, -4, 6, -6};
		vector best;
		bool found = false;
		for (int pass = 0; pass < 2 && !found; pass++)
		{
			int avoid = RZ_SiteGrid.NO_WALK;
			if (pass == 0)
				avoid = avoid | RZ_SiteGrid.ROAD | RZ_SiteGrid.CLEAR | RZ_SiteGrid.STEEP | RZ_SiteGrid.STEP;

			foreach (float d : ins)
			{
				if (found)
					break;

				foreach (float side : sides)
				{
					vector spot = RZ_Util.GroundPos(RZ_Compass.PointAt(RZ_Compass.PointAt(gate, gateOut + 180, d), gateOut + 90, side));
					if (m_Wire && !m_Wire.Inside(spot, 0.5))
						continue;

					int flags = m_Grid.FlagsAt(spot);
					if ((flags & RZ_SiteGrid.OUTSIDE) == 0 && (flags & avoid) != 0)
						continue;

					if (!m_Grid.LineClear(spot, gate))
						continue;

					best = spot;
					found = true;
					break;
				}
			}
		}

		// Else any square a body can stand on round the gate (inside the wire first) that sees it,
		// else the gate itself
		for (int pass2 = 0; pass2 < 2 && !found; pass2++)
		{
			for (int r = 1; r <= 8 && !found; r++)
			{
				for (int k = 0; k < 16; k++)
				{
					vector near = RZ_Util.GroundPos(RZ_Compass.PointAt(gate, k * 22.5, r));
					if (pass2 == 0 && m_Wire && !m_Wire.Inside(near, 0.3))
						continue;

					int nearFlags = m_Grid.FlagsAt(near);
					if ((nearFlags & RZ_SiteGrid.OUTSIDE) == 0 && (nearFlags & RZ_SiteGrid.NO_WALK) != 0)
						continue;

					if (!m_Grid.LineClear(near, gate))
						continue;

					best = near;
					found = true;
					break;
				}
			}
		}

		if (!found)
			best = gate;

		return RZ_Util.GroundPos(best);
	}

	//------------------------------------------------------------------------------------------------
	//! A lookout's post toward each threatened side (threat 3 or more, or the front) that nothing
	//! faces yet (no watch piece, the building's lookouts, the gate's post), at most three: a spot a
	//! body can stand on 3-4.5 m inside the wire that way (off the road and the ways kept clear),
	//! facing out. A post, not a piece: a sentry stands there when the watches reach it.
	protected void SidePosts()
	{
		if (!m_Around || m_Place.m_eKind == RZ_ECampPlace.ROAD_POST)
			return;

		array<float> facing = {};
		foreach (RZ_SitePlaced w : m_Result.m_aPlaced)
		{
			if (w.m_Entry.m_eUse == RZ_EDressUse.WATCH && !w.m_bIndoors)
				facing.Insert(w.m_fYaw);
		}

		if (m_Place.m_Site && !m_Place.m_Site.m_aLookouts.IsEmpty())
			facing.Insert(m_Place.m_fApproach);

		if (m_Result.m_bGatePost)
			facing.Insert(RZ_Compass.Bearing(m_Result.m_vGatePost, m_vGate));

		array<float> alongs = {0, 3, -3, 6, -6, 9, -9};
		array<float> backs = {3, 4.5};
		for (int s = 0; s < RZ_SiteSurroundings.SECTORS; s++)
		{
			if (m_Result.m_aSidePosts.Count() >= MAX_LOOKOUTS)
				return;

			if (m_Around.m_aThreat[s] < THREAT_WATCH && s != m_Around.m_iFront)
				continue;

			float bearing = s * 45;
			bool seen = false;
			foreach (float f : facing)
			{
				if (Math.AbsFloat(RZ_Compass.Normalize(f - bearing + 180) - 180) <= 67.5)
					seen = true;
			}

			if (seen)
				continue;

			vector edge = RZ_Compass.PointAt(m_vMid, bearing, 12);
			if (m_Wire)
			{
				int face;
				edge = m_Wire.LinePointToward(bearing, face);
			}

			bool found = false;
			foreach (float back : backs)
			{
				if (found)
					break;

				foreach (float along : alongs)
				{
					vector spot = RZ_Util.GroundPos(RZ_Compass.PointAt(RZ_Compass.PointAt(edge, bearing + 90, along), bearing + 180, back));
					if (m_Wire && !m_Wire.Inside(spot, 1))
						continue;

					int flags = m_Grid.FlagsAt(spot);
					if ((flags & RZ_SiteGrid.OUTSIDE) == 0 && (flags & (RZ_SiteGrid.NO_WALK | RZ_SiteGrid.ROAD | RZ_SiteGrid.CLEAR | RZ_SiteGrid.STEEP)) != 0)
						continue;

					m_Result.m_aSidePosts.Insert(spot);
					m_Result.m_aSideFacing.Insert(bearing);
					facing.Insert(bearing);
					found = true;
					break;
				}
			}
		}
	}

	// ================================================================ The people and the story
	//------------------------------------------------------------------------------------------------
	//! The posts and routes its people use (RZ_SitePeople), from the site as laid out.
	protected void People()
	{
		m_Result.m_People = RZ_SitePeople.Build(m_Place, m_Grid, m_Wire, m_Around, m_Result, m_vDoor, m_vGate, m_fGateOut, m_vMid, m_Fit.m_aRoad, m_Place.m_iSeed);
	}

	//------------------------------------------------------------------------------------------------
	//! A spot kept for each element the faction's stories can show (RZ_SiteStory.FactionElements):
	//! last of all, so they take only what work and life left; each by the same rules (the ground,
	//! nothing standing, off the roads and the ways kept clear), on its own spot (outside the wire
	//! toward the threat or a hostile base, on the verge outside the gate, the dead zone, the dump,
	//! the court, by the entrance, inside the gate, by the nest, against the wire, the kitchen). Data
	//! only: camp life spawns what the site's story shows (RZ_CampLife.SpawnStory).
	protected void StorySpots()
	{
		if (!m_Settings.m_bStory || m_Place.m_eKind == RZ_ECampPlace.ROAD_POST)
			return;

		array<int> kinds = {};
		RZ_SiteStory.FactionElements(m_Place.m_eFaction, kinds);
		foreach (int kind : kinds)
		{
			int rows = 1;
			if (kind == RZ_SiteStory.E_GRAVES)
				rows = RZ_SiteStory.MAX_GRAVES;

			// Graves: a row of seven, else four, else two
			while (rows >= 1)
			{
				if (StorySpot(kind, rows))
					break;

				if (rows <= 2)
					break;

				rows = Math.Max(2, rows - 3);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool StorySpot(int kind, int rows)
	{
		RZ_DressingPiece e = RZ_DressingPiece.Cast(RZ_Util.NewWithDefaults("RZ_DressingPiece"));
		if (!e)
			return false;

		vector half = RZ_SiteStory.HalfSize(kind);
		if (kind == RZ_SiteStory.E_GRAVES)
			half[0] = rows * 0.8;

		e.m_vFootMin = Vector(-half[0], 0, -half[2]);
		e.m_vFootMax = Vector(half[0], 0, half[2]);
		e.m_iTier = 5;
		e.m_sPrefab = "Story_" + RZ_SiteStory.SceneKind(kind);	// its name in the audit (never spawned as a piece)
		m_Result.m_aKeepAlive.Insert(e);

		// Its own spots, then where it may also stand: the dead and the dump round the other quiet
		// sides, the entrance's and the kitchen's in the court, then the yard
		int where = RZ_SiteStory.Where(kind);
		array<int> tries = {where};
		if (where == RZ_SiteStory.AT_DEAD || where == RZ_SiteStory.AT_DUMP || where == RZ_SiteStory.AT_KILL || where == RZ_SiteStory.AT_HOSTILE)
		{
			tries.Insert(where + 100);
			tries.Insert(where + 200);
		}
		else if (where == RZ_SiteStory.AT_GATE)
		{
			tries.Insert(where + 100);
			tries.Insert(RZ_SiteStory.AT_COURT);
		}
		else if (where == RZ_SiteStory.AT_ENTRANCE || where == RZ_SiteStory.AT_KITCHEN)
		{
			tries.Insert(RZ_SiteStory.AT_COURT);
			tries.Insert(-1);
		}
		else if (where == RZ_SiteStory.AT_COURT || where == RZ_SiteStory.AT_NEST || where == RZ_SiteStory.AT_THREAT_IN)
		{
			tries.Insert(-1);
		}

		bool placed = false;
		foreach (int at : tries)
		{
			array<vector> spots = {};
			array<float> yaws = {};
			bool outside = StoryCandidates(at, spots, yaws, e);
			if (spots.IsEmpty())
				continue;

			m_bStoryOut = outside && m_Wire != null;
			placed = PlaceBest(e, m_Plain, spots, yaws, false, spots[0], RZ_SiteGrid.BLOCKS_LIFE, RZ_ESiteLayer.STORY, 0.5, false);
			m_bStoryOut = false;
			if (placed)
				break;
		}

		if (!placed)
			return false;

		RZ_SitePlaced kept = m_Result.m_aPlaced[m_Result.m_aPlaced.Count() - 1];
		kept.m_iStory = kind;
		kept.m_iStoryRows = rows;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The spots an element tries (RZ_SiteStory.AT_*), each turned as it stands; sets the piece's
	//! zone. True if they're outside the wire.
	protected bool StoryCandidates(int where, notnull array<vector> spots, notnull array<float> yaws, RZ_DressingPiece e)
	{
		e.m_eZone = RZ_ESiteZone.YARD;
		// The dead and the dump on another quiet side (+100: 60 degrees one way, +200 the other)
		float turnQuiet = 0;
		if (where >= 200)
		{
			turnQuiet = -60;
			where -= 200;
		}
		else if (where >= 100)
		{
			turnQuiet = 60;
			where -= 100;
		}

		// Anywhere in the yard: rings round the middle of the wire (or the plan)
		if (where < 0)
		{
			vector middle = m_vMid;
			if (m_Wire)
				middle = m_Wire.Point((m_Wire.m_fX0 + m_Wire.m_fX1) * 0.5, (m_Wire.m_fZ0 + m_Wire.m_fZ1) * 0.5);

			array<float> yRings = {6, 9, 12, 15, 18};
			foreach (float yr : yRings)
			{
				for (int y = 0; y < 8; y++)
				{
					spots.Insert(RZ_Util.GroundPos(RZ_Compass.PointAt(middle, y * 45 + yr * 7, yr)));
					yaws.Insert(y * 45);
				}
			}

			return false;
		}

		switch (where)
		{
			case RZ_SiteStory.AT_KILL:
			case RZ_SiteStory.AT_HOSTILE:
			{
				float b = m_fFront;
				if (where == RZ_SiteStory.AT_HOSTILE && m_Around && m_Around.m_fHostile >= 0)
					b = m_Around.m_fHostile;

				b = RZ_Compass.Normalize(b + turnQuiet * 0.6);

				vector edge = RZ_Compass.PointAt(m_vMid, b, 16);
				if (m_Wire)
				{
					int face;
					edge = m_Wire.LinePointToward(b, face);
				}

				array<float> outs = {4, 6, 8, 11};
				array<float> alongs = {0, 3, -3, 6, -6};
				foreach (float o : outs)
				{
					foreach (float a : alongs)
					{
						spots.Insert(RZ_Util.GroundPos(RZ_Compass.PointAt(RZ_Compass.PointAt(edge, b + 90, a), b, o)));
						yaws.Insert(b);
					}
				}

				return true;
			}
			case RZ_SiteStory.AT_APPROACH:
			case RZ_SiteStory.AT_FAR_VERGE:
			{
				array<float> nearDs = {8, 12, 16, 20};
				array<float> nearOffs = {6, -6, 8, -8};
				array<float> farDs = {14, 18, 22, 26, 30};
				array<float> farOffs = {7, -7, 9, -9, 11, -11};
				array<float> ds = nearDs;
				array<float> offs = nearOffs;
				if (where == RZ_SiteStory.AT_FAR_VERGE)
				{
					ds = farDs;
					offs = farOffs;
				}

				foreach (float d : ds)
				{
					foreach (float off : offs)
					{
						spots.Insert(RZ_Util.GroundPos(RZ_Compass.PointAt(RZ_Compass.PointAt(m_vGate, m_fGateOut, d), m_fGateOut + 90, off)));
						float yaw = m_fGateOut;
						if (where == RZ_SiteStory.AT_FAR_VERGE)
							yaw = RZ_Compass.Normalize(m_fGateOut + 12);

						yaws.Insert(yaw);
					}
				}

				return true;
			}
			case RZ_SiteStory.AT_DEAD:
			case RZ_SiteStory.AT_DUMP:
			{
				e.m_eZone = RZ_ESiteZone.DEAD;
				float q = RZ_Compass.Normalize(QuietBearing() + turnQuiet);
				if (where == RZ_SiteStory.AT_DUMP)
				{
					e.m_eZone = RZ_ESiteZone.DUMP;
					q = RZ_Compass.Normalize(q + 40);
				}

				vector qEdge = RZ_Compass.PointAt(m_vMid, q, 16);
				if (m_Wire)
				{
					int qFace;
					qEdge = m_Wire.LinePointToward(q, qFace);
				}

				array<float> qOuts = {5, 8, 12, 16};
				array<float> qAlongs = {0, 5, -5, 10, -10};
				foreach (float qo : qOuts)
				{
					foreach (float qa : qAlongs)
					{
						spots.Insert(RZ_Util.GroundPos(RZ_Compass.PointAt(RZ_Compass.PointAt(qEdge, q + 90, qa), q, qo)));
						yaws.Insert(RZ_Compass.Normalize(q + 90));
					}
				}

				return false;
			}
			case RZ_SiteStory.AT_COURT:
			case RZ_SiteStory.AT_ENTRANCE:
			{
				e.m_eZone = RZ_ESiteZone.COURT;
				float outB = RZ_Compass.Bearing(m_vMid, m_vDoor);
				if (m_Place.m_Site)
					outB = WallOut(m_Place.m_Site, m_vDoor);

				if (where == RZ_SiteStory.AT_ENTRANCE)
				{
					array<float> walls = {2.5, -2.5, 3.5, -3.5, 5, -5};
					array<float> wallOut = {1.6, 2.4};
					foreach (float wo : wallOut)
					{
						foreach (float w : walls)
						{
							spots.Insert(RZ_Util.GroundPos(RZ_Compass.PointAt(RZ_Compass.PointAt(m_vDoor, outB + 90, w), outB, wo)));
							yaws.Insert(outB);
						}
					}
				}

				array<float> rings = {3, 5, 7};
				foreach (float r : rings)
				{
					for (int k = 0; k < 8; k++)
					{
						float turn = outB + k * 45;
						spots.Insert(RZ_Util.GroundPos(RZ_Compass.PointAt(m_vDoor, turn, r)));
						yaws.Insert(RZ_Compass.Normalize(turn));
					}
				}

				return false;
			}
			case RZ_SiteStory.AT_GATE:
			{
				e.m_eZone = RZ_ESiteZone.GATE;
				array<float> nearIns = {2.5, 4, 6};
				array<float> nearSides = {3, -3, 4.5, -4.5, 6, -6};
				array<float> farIns = {8, 10, 12};
				array<float> farSides = {2, -2, 5, -5, 8, -8, 11, -11};
				array<float> gIns = nearIns;
				array<float> gSides = nearSides;
				// Again: further in and out to the sides
				if (turnQuiet != 0)
				{
					gIns = farIns;
					gSides = farSides;
				}

				foreach (float gi : gIns)
				{
					foreach (float gs : gSides)
					{
						spots.Insert(RZ_Util.GroundPos(RZ_Compass.PointAt(RZ_Compass.PointAt(m_vGate, m_fGateOut + 180, gi), m_fGateOut + 90, gs)));
						yaws.Insert(m_fGateOut);
					}
				}

				return false;
			}
			case RZ_SiteStory.AT_NEST:
			{
				vector nest = m_vGate;
				if (m_Result.m_bGatePost)
					nest = m_Result.m_vGatePost;

				float near = float.MAX;
				foreach (RZ_SitePlaced w : m_Result.m_aPlaced)
				{
					float dn = RZ_Util.DistXZ(w.m_vPos, m_vGate);
					if (w.m_Entry.m_eUse == RZ_EDressUse.WATCH && !w.m_bIndoors && dn < near)
					{
						near = dn;
						nest = w.m_vPos;
					}
				}

				array<float> nRings = {2.5, 3.5, 5};
				foreach (float nr : nRings)
				{
					for (int n = 0; n < 8; n++)
					{
						spots.Insert(RZ_Util.GroundPos(RZ_Compass.PointAt(nest, n * 45, nr)));
						yaws.Insert(n * 45);
					}
				}

				return false;
			}
			case RZ_SiteStory.AT_THREAT_IN:
			{
				vector tEdge = RZ_Compass.PointAt(m_vMid, m_fFront, 12);
				if (m_Wire)
				{
					int tFace;
					tEdge = m_Wire.LinePointToward(m_fFront, tFace);
				}

				array<float> tIns = {2.5, 4, 6};
				array<float> tAlongs = {0, 3, -3, 6, -6, 9, -9, 12, -12};
				foreach (float ti : tIns)
				{
					foreach (float ta : tAlongs)
					{
						spots.Insert(RZ_Util.GroundPos(RZ_Compass.PointAt(RZ_Compass.PointAt(tEdge, m_fFront + 90, ta), m_fFront + 180, ti)));
						yaws.Insert(m_fFront);
					}
				}

				return false;
			}
			case RZ_SiteStory.AT_KITCHEN:
			{
				e.m_eZone = RZ_ESiteZone.KITCHEN;
				array<float> kRings = {2, 3.5, 5, 7, 9};
				foreach (float kr : kRings)
				{
					for (int kk = 0; kk < 8; kk++)
					{
						spots.Insert(RZ_Util.GroundPos(RZ_Compass.PointAt(m_vKitchen, kk * 45, kr)));
						yaws.Insert(kk * 45);
					}
				}

				return false;
			}
		}

		return false;
	}
}
