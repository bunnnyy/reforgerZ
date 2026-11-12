//! The people of a site (docs/SITES.md, "The people"): where its people stand and walk, worked out
//! from the site as the solver leaves it. Posts (the gate, the nest over it, a lookout toward each
//! threatened side, the door), each a spot and a facing a watch job uses; routes (inside the wire,
//! out along the road in, toward the threat, toward the friendly side, the run to the faction's
//! nearest road post); and the spots along the wire on the threat side where people with a weapon
//! go when the site is attacked. Built once when the site is solved (RZ_SiteSolver), kept with its
//! layout as data; camp life's jobs read it (RZ_CampLife: Watch, Patrol, the errands, the alert).
//! Nothing here spawns or is saved. Server only.

//------------------------------------------------------------------------------------------------
//! One post: a spot and a facing a watch job uses.
class RZ_SitePost
{
	static const int GATE = 0;		// the barrier: just inside the main gate, beside the way in
	static const int NEST = 1;		// the watch piece over the gate (a nest, the guard post, a lookout's chair)
	static const int LOOKOUT = 2;	// toward a threatened side: a lookout piece, a watch piece or the building's lookout
	static const int DOOR = 3;		// the entrance

	int m_iKind;
	vector m_vPos;
	float m_fFacing;
	float m_fThreat;		// the threat of the side it watches
	bool m_bCovered;		// that side is covered (bushes, trees, buildings): a lookout there first at night
	bool m_bSeesGate;		// a clear line to the gate within 80 m
	bool m_bPiece;			// a watch piece stands here: its own watch point is used

	//------------------------------------------------------------------------------------------------
	static string KindName(int kind)
	{
		switch (kind)
		{
			case GATE: return "gate";
			case NEST: return "nest";
			case LOOKOUT: return "lookout";
			case DOOR: return "door";
		}

		return "post";
	}
}

//------------------------------------------------------------------------------------------------
//! One walk: points in order from the gate (the wire's loop from its first corner).
class RZ_SiteRoute
{
	static const int WIRE = 0;		// the inside of the wire line, 2 m in; corners stand a while
	static const int APPROACH = 1;	// out of the gate along the road in, 80-150 m, and back
	static const int THREAT = 2;	// out of the gate toward the threat front, 60-100 m, and back
	static const int FRIENDLY = 3;	// out toward the friendly side (Halcyon's courier), 60-90 m, and back
	static const int ROAD_POST = 4;	// to the faction's nearest road post within 1.5 km, and back
	static const int KINDS = 5;

	int m_iKind;
	ref array<vector> m_aPoints = {};
	ref array<bool> m_aStop = {};	// stand a while here (the wire's corners, a walk's far end)
	int m_iBad;						// points that failed the check and were cut (water, a drop, not walkable)
	float m_fOut;					// how far out it goes from the gate (the wire: its length)
	string m_sTo;					// a road post run: the post's title

	//------------------------------------------------------------------------------------------------
	static string KindName(int kind)
	{
		switch (kind)
		{
			case WIRE: return "the wire";
			case APPROACH: return "the approach";
			case THREAT: return "the threat side";
			case FRIENDLY: return "the friendly side";
			case ROAD_POST: return "the road post run";
		}

		return "a walk";
	}

	//------------------------------------------------------------------------------------------------
	static string ShortName(int kind)
	{
		switch (kind)
		{
			case WIRE: return "wire";
			case APPROACH: return "approach";
			case THREAT: return "threat";
			case FRIENDLY: return "friendly";
			case ROAD_POST: return "road post";
		}

		return "walk";
	}
}

//------------------------------------------------------------------------------------------------
//! A site's posts and routes, as data.
class RZ_SitePeople
{
	static const int ALERT_CALM = 0;
	static const int ALERT_WARY = 1;		// a hostile base or heat within 300 m: every lookout manned, nobody outside the wire
	static const int ALERT_ALERT = 2;		// contact: everyone with a weapon to the wire on the threat side
	static const int ALERT_HIT = 3;			// recently hit (the story): the gate doubled, the threat-side walk every shift

	protected static const float WIRE_IN = 2;		// the wire's walk keeps this far inside the line
	protected static const float STEP_THREAT = 5;	// the wire's walk: a point this often on a threatened side...
	protected static const float STEP_QUIET = 10;	// ...and this often elsewhere
	protected static const float GATE_SEE = 80;		// a post sees the gate within this
	protected static const float WALK_STEP = 20;	// the walks outside: a point this often
	protected static const float ROAD_SNAP = 12;	// a walk's point moves onto a road this near
	protected static const float ROAD_POST_REACH = 1500;
	protected static const float THREAT_WATCH = 3;	// a side this threatened (as the audit)

	ref array<ref RZ_SitePost> m_aPosts = {};		// in order of importance: the gate's, the lookouts by threat, the door
	ref array<ref RZ_SiteRoute> m_aRoutes = {};
	ref array<vector> m_aThreatLine = {};			// spots 2 m inside the wire facing the threat (the alert)
	vector m_vGate;
	float m_fGateOut;
	vector m_vDoor;
	vector m_vMid;
	float m_fFront;					// the threat front's bearing
	float m_fHostileDist = -1;		// the nearest hostile base (-1: none within 3 km)
	int m_iHostileFaction = -1;
	int m_iFrontKind = RZ_SiteFacts.OUT_ROAD;	// what the threat front is (the story: RZ_SiteFacts.OUT_*)
	bool m_bThrough;				// a road passes through

	//------------------------------------------------------------------------------------------------
	RZ_SiteRoute Route(int kind)
	{
		foreach (RZ_SiteRoute r : m_aRoutes)
		{
			if (r.m_iKind == kind)
				return r;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! How many posts of a kind.
	int CountPosts(int kind)
	{
		int n = 0;
		foreach (RZ_SitePost post : m_aPosts)
		{
			if (post.m_iKind == kind)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! The posts in the order watches fill them: the gate's two (the nest over it first, then the
	//! barrier: a faction with one watch sits in its nest), the door next for Halcyon (its guard pair)
	//! and the Haven (its lookout), then the lookouts by threat (at night the covered sides first),
	//! then the door.
	void Order(RZ_EFaction f, bool night, notnull array<int> result)
	{
		array<int> gates = {};
		array<int> doors = {};
		array<int> looks = {};
		array<float> worth = {};
		foreach (int i, RZ_SitePost post : m_aPosts)
		{
			if (post.m_iKind == RZ_SitePost.NEST && post.m_bSeesGate)
				gates.InsertAt(i, 0);
			else if (post.m_iKind == RZ_SitePost.GATE || post.m_iKind == RZ_SitePost.NEST)
				gates.Insert(i);
			else if (post.m_iKind == RZ_SitePost.DOOR)
				doors.Insert(i);
			else
			{
				float value = post.m_fThreat;
				if (night && post.m_bCovered)
					value += 100;

				int at = 0;
				while (at < worth.Count() && worth[at] >= value)
				{
					at++;
				}

				looks.InsertAt(i, at);
				worth.InsertAt(value, at);
			}
		}

		// The first gate post always; the door before the second for Halcyon and the Haven
		bool doorEarly = f == RZ_EFaction.INSTITUTE || f == RZ_EFaction.SURVIVORS;
		if (!gates.IsEmpty())
			result.Insert(gates[0]);

		if (doorEarly)
			result.InsertAll(doors);

		// At night the covered side's lookout before the second gate post
		if (night && !looks.IsEmpty())
		{
			result.Insert(looks[0]);
			looks.Remove(0);
		}

		for (int g = 1; g < gates.Count(); g++)
		{
			result.Insert(gates[g]);
		}

		result.InsertAll(looks);
		if (!doorEarly)
			result.InsertAll(doors);
	}

	// ================================================================ Building it
	//------------------------------------------------------------------------------------------------
	//! Works out a solved site's posts and routes. `gatePost`: the gate's post the watch layer chose
	//! (RZ_SiteSolver.GatePost); `roads`: the road segments near (two points each).
	static RZ_SitePeople Build(RZ_CampPlace p, RZ_SiteGrid grid, RZ_SiteWire wire, RZ_SiteSurroundings around, RZ_SiteResult result, vector door, vector gate, float gateOut, vector mid, array<vector> roads, int seed)
	{
		RZ_SitePeople people = new RZ_SitePeople();
		people.m_vGate = gate;
		people.m_fGateOut = gateOut;
		people.m_vDoor = door;
		people.m_vMid = mid;
		people.m_fFront = gateOut;
		if (around)
		{
			people.m_fFront = around.m_iFront * 45;
			people.m_bThrough = around.m_iThrough > 0;
			if (around.m_fHostile >= 0)
			{
				people.m_fHostileDist = around.m_fHostileDist;
				people.m_iHostileFaction = around.m_iHostileFaction;
			}

			people.m_iFrontKind = FrontKind(around);
		}

		if (p.m_eKind == RZ_ECampPlace.ROAD_POST)
			return people;

		people.Posts(p, grid, around, result);
		RZ_Rng rng = new RZ_Rng(seed + 59);
		if (wire)
		{
			people.WireWalk(grid, wire, around);
			people.ThreatLine(grid, wire);
		}

		people.Walk(RZ_SiteRoute.APPROACH, gateOut, rng.Range(80, 150), grid, wire, roads, true);
		people.Walk(RZ_SiteRoute.THREAT, people.m_fFront, rng.Range(60, 100), grid, wire, roads, false);
		if (p.m_eFaction == RZ_EFaction.INSTITUTE)
		{
			float friendly = RZ_Compass.Normalize(people.m_fFront + 180);
			if (around && around.m_fFriendly >= 0)
				friendly = around.m_fFriendly;

			people.Walk(RZ_SiteRoute.FRIENDLY, friendly, rng.Range(60, 90), grid, wire, roads, false);
		}

		if (p.m_eFaction == RZ_EFaction.MILITARY || p.m_eFaction == RZ_EFaction.BANDITS)
			people.RoadPostRun(p, grid);

		return people;
	}

	//------------------------------------------------------------------------------------------------
	//! What the threat front is: a hostile base that way, the infected's land, the road in, the sea;
	//! with none of them on the front, the nearest that is there.
	protected static int FrontKind(RZ_SiteSurroundings around)
	{
		int front = around.m_iFront;
		if (around.m_fHostile >= 0 && RZ_SiteSurroundings.SectorOf(around.m_fHostile) == front)
			return RZ_SiteFacts.OUT_HOSTILE;

		if (around.m_fWild >= 0 && RZ_SiteSurroundings.SectorOf(around.m_fWild) == front)
			return RZ_SiteFacts.OUT_INFECTED;

		if (around.m_fMainRoad >= 0 && RZ_SiteSurroundings.SectorOf(around.m_fMainRoad) == front)
			return RZ_SiteFacts.OUT_ROAD;

		if (around.m_iSeaMask != 0)
			return RZ_SiteFacts.OUT_SEA;

		if (around.m_fHostile >= 0)
			return RZ_SiteFacts.OUT_HOSTILE;

		if (around.m_fWild >= 0)
			return RZ_SiteFacts.OUT_INFECTED;

		return RZ_SiteFacts.OUT_ROAD;
	}

	//------------------------------------------------------------------------------------------------
	//! The posts: the gate's (the watch layer's spot), each watch piece (the nearest the gate is its
	//! nest), the building's lookouts, the entrance.
	protected void Posts(RZ_CampPlace p, RZ_SiteGrid grid, RZ_SiteSurroundings around, RZ_SiteResult result)
	{
		if (result.m_bGatePost)
			AddPost(RZ_SitePost.GATE, result.m_vGatePost, m_fGateOut, false, grid, around);

		// The watch pieces: the nearest the gate that sees it is the nest; the rest are lookouts
		int nest = -1;
		float nearest = float.MAX;
		foreach (int i, RZ_SitePlaced w : result.m_aPlaced)
		{
			if (w.m_Entry.m_eUse != RZ_EDressUse.WATCH || w.m_bIndoors)
				continue;

			float d = RZ_Util.DistXZ(w.m_vPos, m_vGate);
			if (d < nearest && w.m_iLayer == RZ_ESiteLayer.PLAN)
			{
				nearest = d;
				nest = i;
			}
		}

		foreach (int j, RZ_SitePlaced piece : result.m_aPlaced)
		{
			if (piece.m_Entry.m_eUse != RZ_EDressUse.WATCH || piece.m_bIndoors)
				continue;

			int kind = RZ_SitePost.LOOKOUT;
			if (j == nest)
				kind = RZ_SitePost.NEST;

			AddPost(kind, piece.m_vPos, piece.m_fYaw, true, grid, around);
		}

		// The lookouts' posts toward threatened sides nothing faced
		foreach (int k, vector side : result.m_aSidePosts)
		{
			AddPost(RZ_SitePost.LOOKOUT, side, result.m_aSideFacing[k], false, grid, around);
		}

		// The building's lookouts: where its sentries stand round it
		if (p.m_Site)
		{
			foreach (vector look : p.m_Site.m_aLookouts)
			{
				AddPost(RZ_SitePost.LOOKOUT, look, RZ_Compass.Bearing(m_vMid, look), false, grid, around);
			}

			AddPost(RZ_SitePost.DOOR, m_vDoor, RZ_Compass.Bearing(m_vMid, m_vDoor), false, grid, around);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AddPost(int kind, vector at, float facing, bool piece, RZ_SiteGrid grid, RZ_SiteSurroundings around)
	{
		RZ_SitePost post = new RZ_SitePost();
		post.m_iKind = kind;
		post.m_vPos = at;
		post.m_fFacing = facing;
		post.m_bPiece = piece;
		post.m_bSeesGate = RZ_Util.DistXZ(at, m_vGate) < GATE_SEE && (!grid || grid.LineClear(at, m_vGate));
		if (around)
		{
			int sector = RZ_SiteSurroundings.SectorOf(facing);
			post.m_fThreat = around.m_aThreat[sector];
			post.m_bCovered = around.IsCovered(sector);
		}

		m_aPosts.Insert(post);
	}

	//------------------------------------------------------------------------------------------------
	//! The wire's walk: round the line 2 m in, from the corner nearest the gate; a point every 5 m
	//! on a threatened side (the heaviest side gets the longest stretch) and every 10 m elsewhere,
	//! each on ground a body can stand on (cut where none is near); the corners stand a while.
	protected void WireWalk(RZ_SiteGrid grid, RZ_SiteWire wire, RZ_SiteSurroundings around)
	{
		RZ_SiteRoute route = new RZ_SiteRoute();
		route.m_iKind = RZ_SiteRoute.WIRE;
		int first = 0;
		float best = float.MAX;
		for (int c = 0; c < 4; c++)
		{
			float cx;
			float cz;
			float dx;
			float dz;
			wire.FaceEnds(c, cx, cz, dx, dz);
			float d = RZ_Util.DistXZ(wire.Point(cx, cz), m_vGate);
			if (d < best)
			{
				best = d;
				first = c;
			}
		}

		for (int s = 0; s < 4; s++)
		{
			int k = (first + s) % 4;
			float ax;
			float az;
			float bx;
			float bz;
			wire.FaceEnds(k, ax, az, bx, bz);
			// 2 m in from both sides that meet at each corner
			float inAX = ax + WIRE_IN * SignToward(ax, wire.m_fX0, wire.m_fX1);
			float inAZ = az + WIRE_IN * SignToward(az, wire.m_fZ0, wire.m_fZ1);
			float inBX = bx + WIRE_IN * SignToward(bx, wire.m_fX0, wire.m_fX1);
			float inBZ = bz + WIRE_IN * SignToward(bz, wire.m_fZ0, wire.m_fZ1);
			float len = Math.Sqrt((inBX - inAX) * (inBX - inAX) + (inBZ - inAZ) * (inBZ - inAZ));
			route.m_fOut = route.m_fOut + len;
			float step = STEP_QUIET;
			if (around && Threatened(around, wire.FaceOut(k)))
				step = STEP_THREAT;

			int n = Math.Max(1, Math.Ceil(len / step));
			for (int i = 0; i < n; i++)
			{
				float t = i;
				t = t / n;
				vector pt = wire.Point(inAX + (inBX - inAX) * t, inAZ + (inBZ - inAZ) * t);
				AddPoint(route, pt, i == 0, grid);
			}
		}

		if (route.m_aPoints.Count() >= 3)
			m_aRoutes.Insert(route);
	}

	//------------------------------------------------------------------------------------------------
	//! +1 toward the inside of [lo, hi] from v (v on one of its ends).
	protected static float SignToward(float v, float lo, float hi)
	{
		if (Math.AbsFloat(v - lo) < Math.AbsFloat(v - hi))
			return 1;

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Threatened(RZ_SiteSurroundings around, float bearing)
	{
		for (int s = 0; s < RZ_SiteSurroundings.SECTORS; s++)
		{
			if (around.m_aThreat[s] < THREAT_WATCH && s != around.m_iFront)
				continue;

			if (Math.AbsFloat(RZ_Compass.Normalize(bearing - s * 45 + 180) - 180) <= 67.5)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The alert's spots: along the wire's side facing the threat front, 2 m in, every 4 m, on
	//! ground a body can stand on (at most 10).
	protected void ThreatLine(RZ_SiteGrid grid, RZ_SiteWire wire)
	{
		int face;
		vector mid = wire.LinePointToward(m_fFront, face);
		float outB = wire.FaceOut(face);
		for (int i = 0; i < 10; i++)
		{
			float along = ((i + 1) / 2) * 4;
			if (i % 2 == 1)
				along = -along;

			vector pt = RZ_Compass.PointAt(RZ_Compass.PointAt(mid, outB + 90, along), outB + 180, WIRE_IN);
			if (!wire.Inside(pt, 1))
				continue;

			vector stand = pt;
			if (grid)
			{
				stand = grid.NearestWalkable(pt, 2);
				if ((grid.FlagsAt(stand) & RZ_SiteGrid.NO_WALK) != 0)
					continue;
			}

			m_aThreatLine.Insert(RZ_Util.GroundPos(stand));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A walk out of the gate: first along the way out, then turning toward `bearing` (the road in
	//! itself when `road`: each point onto the road where one is near), a point every 20 m out to
	//! `reach` m; it stops where the next point is wet, over a drop, or not walkable.
	protected void Walk(int kind, float bearing, float reach, RZ_SiteGrid grid, RZ_SiteWire wire, array<vector> roads, bool road)
	{
		RZ_SiteRoute route = new RZ_SiteRoute();
		route.m_iKind = kind;
		vector at = RZ_Util.GroundPos(RZ_Compass.PointAt(m_vGate, m_fGateOut, 3));
		AddPoint(route, at, false, grid);
		float heading = m_fGateOut;
		int steps = Math.Ceil(reach / WALK_STEP);
		for (int i = 0; i < steps; i++)
		{
			// Turn toward the bearing a little a step (along the road in: follow the road)
			float off = RZ_Compass.Normalize(bearing - heading + 180) - 180;
			heading = RZ_Compass.Normalize(heading + Math.Clamp(off, -45, 45));
			vector next = RZ_Compass.PointAt(at, heading, WALK_STEP);
			if (road && roads)
				next = OntoRoad(next, roads);

			// Never back in through the wire
			if (wire && wire.Inside(next, -2))
				next = RZ_Compass.PointAt(at, RZ_Compass.Bearing(m_vMid, at), WALK_STEP);

			bool ok = PointOk(next, grid);
			if (!ok)
			{
				// A little either way, else it stops here
				for (int k = 0; k < 4 && !ok; k++)
				{
					float turn = 25;
					if (k % 2 == 1)
						turn = -25;

					turn = turn * ((k / 2) + 1);
					next = RZ_Compass.PointAt(at, heading + turn, WALK_STEP);
					ok = PointOk(next, grid);
				}
			}

			if (!ok)
			{
				route.m_iBad++;
				break;
			}

			heading = RZ_Compass.Bearing(at, next);
			at = RZ_Util.GroundPos(next);
			AddPoint(route, at, i == steps - 1, grid);
		}

		route.m_fOut = RZ_Util.DistXZ(m_vGate, at);
		if (route.m_aPoints.Count() >= 3)
		{
			route.m_aStop[route.m_aStop.Count() - 1] = true;
			m_aRoutes.Insert(route);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The road post run: to the faction's nearest road post within 1.5 km, out of the gate and
	//! along the way (a point every 150 m; the AI finds its own way between them), and back.
	protected void RoadPostRun(RZ_CampPlace p, RZ_SiteGrid grid)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetCheckpoints())
			return;

		array<RZ_RoadPost> posts = {};
		gm.GetCheckpoints().PostsNear(p.m_eFaction, m_vGate, ROAD_POST_REACH, posts);
		RZ_RoadPost best;
		float near = float.MAX;
		foreach (RZ_RoadPost post : posts)
		{
			float d = RZ_Util.DistXZ(post.m_vPos, m_vGate);
			if (d < near && d > 60)
			{
				near = d;
				best = post;
			}
		}

		if (!best)
			return;

		RZ_SiteRoute route = new RZ_SiteRoute();
		route.m_iKind = RZ_SiteRoute.ROAD_POST;
		route.m_sTo = best.GetTitle();
		vector at = RZ_Util.GroundPos(RZ_Compass.PointAt(m_vGate, m_fGateOut, 3));
		AddPoint(route, at, false, grid);
		int legs = Math.Max(1, Math.Ceil(near / 150));
		for (int i = 1; i <= legs; i++)
		{
			float t = i;
			t = t / legs;
			vector pt = RZ_Util.GroundPos(m_vGate + (best.m_vPos - m_vGate) * t);
			if (i == legs)
				pt = RZ_Util.GroundPos(RZ_Compass.PointAt(best.m_vPos, RZ_Compass.Bearing(best.m_vPos, m_vGate), 8));

			if (!PointOk(pt, null))
			{
				route.m_iBad++;
				continue;
			}

			AddPoint(route, pt, i == legs, grid);
		}

		route.m_fOut = near;
		if (route.m_aPoints.Count() >= 2 && route.m_aStop[route.m_aStop.Count() - 1])
			m_aRoutes.Insert(route);
	}

	//------------------------------------------------------------------------------------------------
	//! A point onto the nearest road segment within ROAD_SNAP m (it stays where it is if none).
	protected static vector OntoRoad(vector pt, array<vector> roads)
	{
		vector best = pt;
		float near = ROAD_SNAP;
		for (int i = 0; i + 1 < roads.Count(); i += 2)
		{
			vector on = ClosestOnSegment(pt, roads[i], roads[i + 1]);
			float d = RZ_Util.DistXZ(on, pt);
			if (d < near)
			{
				near = d;
				best = on;
			}
		}

		return RZ_Util.GroundPos(best);
	}

	//------------------------------------------------------------------------------------------------
	protected static vector ClosestOnSegment(vector pt, vector a, vector b)
	{
		vector ab = b - a;
		ab[1] = 0;
		float len2 = ab[0] * ab[0] + ab[2] * ab[2];
		if (len2 < 0.0001)
			return a;

		float t = Math.Clamp(((pt[0] - a[0]) * ab[0] + (pt[2] - a[2]) * ab[2]) / len2, 0, 1);
		return a + ab * t;
	}

	//------------------------------------------------------------------------------------------------
	//! A point on a route: on the grid, the nearest square a body can stand on within 2 m; it must
	//! pass the audit's check (PointOk), else it's cut (counted). `stop`: stand a while there.
	protected bool AddPoint(RZ_SiteRoute route, vector pt, bool stop, RZ_SiteGrid grid)
	{
		vector at = pt;
		if (grid && (grid.FlagsAt(pt) & RZ_SiteGrid.OUTSIDE) == 0)
		{
			at = grid.NearestWalkable(pt, 2);
			if ((grid.FlagsAt(at) & (RZ_SiteGrid.NO_WALK | RZ_SiteGrid.WATER)) != 0)
			{
				route.m_iBad++;
				return false;
			}
		}

		at = RZ_Util.GroundPos(at);
		if (!PointOk(at, grid))
		{
			route.m_iBad++;
			return false;
		}

		route.m_aPoints.Insert(at);
		route.m_aStop.Insert(stop);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A walk's point a body can stand at: dry, no drop of 4 m within 6 m (the audit's check), not
	//! steep or in the water on the grid.
	static bool PointOk(vector pt, RZ_SiteGrid grid)
	{
		if (!RZ_SiteAudit.RouteOk(RZ_Util.GroundPos(pt)))
			return false;

		if (!grid)
			return true;

		int flags = grid.FlagsAt(pt);
		if ((flags & RZ_SiteGrid.OUTSIDE) != 0)
			return true;

		return (flags & (RZ_SiteGrid.WATER | RZ_SiteGrid.INSIDE | RZ_SiteGrid.SOLID)) == 0;
	}

	// ================================================================ The log
	//------------------------------------------------------------------------------------------------
	//! "posts gate 1, nest 1, lookout 2 (2 see the gate), door 1; routes the wire 14 points (86 m),
	//! the approach 6 points (110 m), ..."
	string Describe()
	{
		array<int> kinds = {RZ_SitePost.GATE, RZ_SitePost.NEST, RZ_SitePost.LOOKOUT, RZ_SitePost.DOOR};
		string posts = "";
		int see = 0;
		foreach (RZ_SitePost post : m_aPosts)
		{
			if (post.m_bSeesGate)
				see++;
		}

		foreach (int kind : kinds)
		{
			int n = CountPosts(kind);
			if (n == 0)
				continue;

			if (!posts.IsEmpty())
				posts += ", ";

			posts += string.Format("%1 %2", RZ_SitePost.KindName(kind), n);
		}

		if (posts.IsEmpty())
			posts = "none";

		string routes = "";
		foreach (RZ_SiteRoute r : m_aRoutes)
		{
			if (!routes.IsEmpty())
				routes += ", ";

			routes += string.Format("%1 %2 points (%3 m", RZ_SiteRoute.KindName(r.m_iKind), r.m_aPoints.Count(), RZ_Util.RoundToInt(r.m_fOut));
			if (!r.m_sTo.IsEmpty())
				routes += " to " + r.m_sTo;

			if (r.m_iBad > 0)
				routes += string.Format(", %1 cut", r.m_iBad);

			routes += ")";
		}

		if (routes.IsEmpty())
			routes = "none";

		return string.Format("posts %1 (%2 see the gate); routes %3; %4 spots on the wire facing the threat", posts, see, routes, m_aThreatLine.Count());
	}

	//------------------------------------------------------------------------------------------------
	static string AlertName(int alert)
	{
		switch (alert)
		{
			case ALERT_WARY: return "wary";
			case ALERT_ALERT: return "alert";
			case ALERT_HIT: return "recently hit";
		}

		return "calm";
	}
}
