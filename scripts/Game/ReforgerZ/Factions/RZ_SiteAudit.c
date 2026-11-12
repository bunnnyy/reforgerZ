//! The site audit (docs/SITES.md, "The audit"): every site laid out is measured once a run, as
//! data: its pieces' real footprints against each other, the buildings and what stands on the grid,
//! props floating or buried, the posts and what they see, the paths, the pieces dropped and the
//! patrol's loop, then a score out of 100 and one log line. The runtime test lays out every site on
//! the map this way (nothing spawned) and adds drawing lines for tools/camp_kit/render.py --placed.
//! Server only.

//------------------------------------------------------------------------------------------------
//! One site's audit, kept for the summary.
class RZ_SiteScore
{
	string m_sWhat;			// "Red Hand outpost at 041-077"
	bool m_bStronghold;
	int m_iScore;
	int m_iOverlaps;
	bool m_bFailed;			// a hard failure: an overlap, a tier 1 piece dropped, a stronghold with no gate watch, a piece outside the wire, the perimeter under its bar
	int m_iPerimeter = -1;	// percent of the wire line closed (-1: no wire)
	int m_iTierOne;			// tier 1 pieces dropped
	int m_iOutside;			// pieces of tier 2-4 outside the wire
	bool m_bWet;			// the building, the yard or a piece over water
	bool m_bGateWatch;		// a post sees the gate (a site with a gate)
	bool m_bNeedsGate;
	int m_iFaction = -1;
	int m_iStory = -1;		// the story its state picks (RZ_SiteStory)
	ref array<int> m_aStoryWhole = {};	// each of the faction's four: 1 if every element has its spot and prefab
	int m_iRoutePoints;		// its people's routes: points kept...
	int m_iRouteBad;		// ...and those failing the check
	int m_iRoutes;
}

//------------------------------------------------------------------------------------------------
//! What the audit found at one site, for its drawing.
class RZ_SiteMarks
{
	vector m_vGate;
	vector m_vDoor;
	ref array<vector> m_aPosts = {};
	ref array<string> m_aPostKind = {};
	ref array<float> m_aFacing = {};
	ref array<bool> m_aClear = {};
	ref array<vector> m_aRoute = {};
	ref array<bool> m_aRouteOk = {};
	ref array<string> m_aWalks = {};		// each walk out and back: "kind x z ok ..."
	ref array<vector> m_aOverlapAt = {};
	ref array<string> m_aOverlaps = {};
}

//------------------------------------------------------------------------------------------------
class RZ_SiteAudit
{
	static const float OVERLAP_AREA = 0.1;		// m2: less than this between two footprints isn't an overlap
	static const float FLOAT_SPREAD = 0.3;		// ground under one prop varying more than this: one edge floats or is buried
	static const float THREAT_WATCH = 3;		// a side this threatened wants a watch facing it
	static const int PERIMETER_STRONGHOLD = 80;	// a stronghold's wire closed this much at least (percent)
	static const int PERIMETER_OUTPOST = 60;	// an outpost's, a field station's, a safehouse's

	ref array<ref RZ_SiteScore> m_aScores = {};
	protected int m_iDrawn;

	//------------------------------------------------------------------------------------------------
	//! Audits one laid-out site and logs it; `draw` adds the drawing lines (the runtime test).
	void Site(RZ_CampLife camp, RZ_CampPlace p, RZ_CampGround ground, RZ_SiteGrid grid, RZ_SiteSurroundings around, RZ_SiteResult result, bool draw)
	{
		string what = string.Format("%1 %2 at %3", RZ_Factions.GetName(p.m_eFaction), KindName(p.m_eKind), RZ_SettlementScanner.GridReference(p.m_vPos));
		bool stronghold = p.m_eKind == RZ_ECampPlace.STRONGHOLD;
		int id = m_iDrawn;
		m_iDrawn++;

		// Every placed footprint's rectangles, in the world
		array<ref array<vector>> rects = {};
		array<int> owner = {};
		array<bool> seat = {};
		foreach (int pi, RZ_SitePlaced placed : result.m_aPlaced)
		{
			if (placed.m_bIndoors)
				continue;

			RZ_SiteFoot foot = placed.m_Entry.Foot();
			for (int r = 0; r < foot.Count(); r++)
			{
				array<vector> quad = {};
				foot.Corners(r, placed.m_vPos, placed.m_fYaw, 0, quad);
				rects.Insert(quad);
				owner.Insert(pi);
				seat.Insert(foot.IsSeat(r));
			}
		}

		// Overlaps: between pieces, with buildings, with what stands on the grid
		array<string> overlaps = {};
		array<vector> overlapAt = {};
		for (int a = 0; a < rects.Count(); a++)
		{
			for (int b = a + 1; b < rects.Count(); b++)
			{
				if (owner[a] == owner[b] || (seat[a] && seat[b]))
					continue;

				float both = OverlapArea(rects[a], rects[b]);
				if (both > OVERLAP_AREA)
					AddOverlap(overlaps, overlapAt, PieceName(result.m_aPlaced[owner[a]].m_Entry) + " and " + PieceName(result.m_aPlaced[owner[b]].m_Entry), Middle(rects[a]));
			}
		}

		array<ref array<vector>> boxes = {};
		BuildingBoxes(grid, boxes);
		foreach (int k, array<vector> rect : rects)
		{
			RZ_DressingPiece entry = result.m_aPlaced[owner[k]].m_Entry;
			if (entry.m_eAt == RZ_EDressAt.COUNTER || result.m_aPlaced[owner[k]].m_bWall)
				continue;

			foreach (array<vector> box : boxes)
			{
				if (OverlapArea(rect, box) > OVERLAP_AREA)
				{
					AddOverlap(overlaps, overlapAt, PieceName(entry) + " and a building", Middle(rect));
					break;
				}
			}

			if (!seat[k] && grid && SolidArea(grid, rect) > OVERLAP_AREA)
				AddOverlap(overlaps, overlapAt, PieceName(entry) + " and something standing there", Middle(rect));
		}

		// Floating or buried: the ground under one prop varies over 0.3 m
		int floating = 0;
		if (grid)
		{
			foreach (int f, array<vector> under : rects)
			{
				RZ_SitePlaced on = result.m_aPlaced[owner[f]];
				if (!seat[f] && !on.m_bWall && Spread(grid, under, on.m_vPos, on.m_fYaw, on.m_fSlope) > FLOAT_SPREAD)
					floating++;
			}
		}

		// Posts: the watch pieces and the building's lookouts, the gate, and what they see
		vector gate = GatePoint(p);
		if (result.m_bGate)
			gate = result.m_vGate;

		array<vector> posts = {};
		array<float> facing = {};
		array<string> postKind = {};
		foreach (RZ_SitePlaced w : result.m_aPlaced)
		{
			if (w.m_Entry.m_eUse == RZ_EDressUse.WATCH && !w.m_bIndoors)
			{
				posts.Insert(w.m_vPos);
				facing.Insert(w.m_fYaw);
				if (w.m_iLayer == RZ_ESiteLayer.LOOKOUT)
					postKind.Insert("side");
				else
					postKind.Insert("nest");
			}
		}

		if (p.m_Site)
		{
			int lookouts = 2;
			if (p.m_eKind == RZ_ECampPlace.SAFEHOUSE)
				lookouts = 1;

			for (int l = 0; l < lookouts; l++)
			{
				posts.Insert(camp.LookoutSpot(p, l));
				facing.Insert(p.m_fApproach);
				postKind.Insert("lookout");
			}
		}

		// The lookouts' posts toward threatened sides (they watch a side, not the gate)
		foreach (int sp, vector side : result.m_aSidePosts)
		{
			posts.Insert(side);
			facing.Insert(result.m_aSideFacing[sp]);
			postKind.Insert("side");
		}

		// The gate's post (the watch layer's spot just inside the gate)
		if (result.m_bGatePost)
		{
			posts.Insert(result.m_vGatePost);
			facing.Insert(RZ_Compass.Bearing(result.m_vGatePost, gate));
			postKind.Insert("gate");
		}

		bool gateWatch = false;
		int blindPosts = 0;
		array<bool> clear = {};
		foreach (int q, vector post : posts)
		{
			bool sees = !grid || grid.LineClear(post, gate);
			clear.Insert(sees);
			// A lookout toward a threatened side watches that side, not the gate
			if (!sees && postKind[q] != "side")
				blindPosts++;

			if (sees && RZ_Util.DistXZ(post, gate) < 80)
				gateWatch = true;
		}

		int threatSides = 0;
		int watchedSides = 0;
		if (around)
		{
			for (int sector = 0; sector < RZ_SiteSurroundings.SECTORS; sector++)
			{
				if (around.m_aThreat[sector] < THREAT_WATCH && sector != around.m_iFront)
					continue;

				threatSides++;
				foreach (float face : facing)
				{
					float off = Math.AbsFloat(RZ_Compass.Normalize(face - sector * 45 + 180) - 180);
					if (off <= 67.5)
					{
						watchedSides++;
						break;
					}
				}
			}
		}

		// Paths: door to gate, door to kitchen
		vector door = p.m_vHome;
		if (p.m_Site)
			door = camp.DoorSpot(p);

		if (result.m_bDoor)
			door = result.m_vDoor;

		int blockedPaths = 0;
		string paths = "";
		if (grid && p.m_eKind != RZ_ECampPlace.ROAD_POST)
		{
			bool toGate = grid.PathClear(door, gate);
			paths = "door to gate " + OkText(toGate);
			if (result.m_iBlockedPaths > 0)
				paths += " (no way to keep clear)";
			if (!toGate)
				blockedPaths++;

			foreach (RZ_SitePlaced cook : result.m_aPlaced)
			{
				if (cook.m_Entry.m_eUse != RZ_EDressUse.COOK)
					continue;

				bool toKitchen = grid.PathClear(door, FrontOf(cook.m_Entry, cook.m_vPos, cook.m_fYaw));
				paths += ", door to kitchen " + OkText(toKitchen);
				if (!toKitchen)
					blockedPaths++;

				break;
			}
		}

		// The people's routes (the wire's walk, the walks out): each point walkable, not in water, not
		// over a drop; without them (Solver off) the old patrol's loop
		array<vector> route = {};
		array<bool> routeOk = {};
		array<string> walks = {};
		int badRoute = 0;
		int routePoints = 0;
		string routeText = "";
		if (result.m_People && p.m_eKind != RZ_ECampPlace.ROAD_POST)
		{
			foreach (RZ_SiteRoute walk : result.m_People.m_aRoutes)
			{
				string walkText = RZ_SiteRoute.ShortName(walk.m_iKind);
				int walkBad = 0;
				foreach (vector wp : walk.m_aPoints)
				{
					bool walkOk = RouteOk(wp) && (!grid || (grid.FlagsAt(wp) & (RZ_SiteGrid.WATER | RZ_SiteGrid.INSIDE | RZ_SiteGrid.SOLID)) == 0);
					routePoints++;
					if (!walkOk)
					{
						badRoute++;
						walkBad++;
					}

					if (walk.m_iKind == RZ_SiteRoute.WIRE)
					{
						route.Insert(wp);
						routeOk.Insert(walkOk);
					}
					else
					{
						walkText += string.Format(" %1 %2 %3", Num(wp[0]), Num(wp[2]), YesNo(walkOk));
					}
				}

				if (walk.m_iKind != RZ_SiteRoute.WIRE)
					walks.Insert(walkText);

				if (!routeText.IsEmpty())
					routeText += ", ";

				routeText += string.Format("%1 %2 points, %3 m", RZ_SiteRoute.ShortName(walk.m_iKind), walk.m_aPoints.Count(), RZ_Util.RoundToInt(walk.m_fOut));
				if (walkBad > 0)
					routeText += string.Format(" (%1 bad)", walkBad);
			}
		}
		else if (p.m_eKind != RZ_ECampPlace.ROAD_POST)
		{
			RZ_CampLife.PatrolPoints(p, 0, route);
			foreach (vector point : route)
			{
				bool ok = RouteOk(point);
				routeOk.Insert(ok);
				routePoints++;
				if (!ok)
					badRoute++;
			}

			routeText = "the old loop";
		}

		if (routeText.IsEmpty())
			routeText = "none";

		// Over water: the building, the yard or any piece standing over the sea, a river or a pond
		string wet = "";
		if (p.m_Site && RZ_CampLife.WetAt(p.m_Site.m_vCentre))
			wet = "the building";

		if (RZ_CampLife.WetAt(p.m_vHome))
			wet = AddItem(wet, "the yard");

		foreach (RZ_SitePlaced dry : result.m_aPlaced)
		{
			if (!dry.m_bIndoors && !dry.m_bWall && dry.m_Entry.m_eAt != RZ_EDressAt.COUNTER && RZ_CampLife.WetAt(dry.m_vPos))
				wet = AddItem(wet, PieceName(dry.m_Entry));
		}

		// The perimeter: the wire line's share closed, and its widest gap
		int perimeter = -1;
		string perimeterText = "no wire";
		bool perimeterLow = false;
		int perimeterBar = PERIMETER_OUTPOST;
		if (stronghold)
			perimeterBar = PERIMETER_STRONGHOLD;

		if (result.m_Wire)
		{
			perimeter = result.m_Wire.Share();
			perimeterText = "perimeter " + perimeter.ToString() + "%";
			float gapWidth;
			float gapBearing;
			if (result.m_Wire.WidestGap(gapWidth, gapBearing))
				perimeterText += string.Format(" (gap %1 m at %2)", Math.Round(gapWidth * 10) / 10, RZ_Util.RoundToInt(gapBearing));

			perimeterText += string.Format(", %1 gates", result.m_Wire.m_aGates.Count());
			perimeterLow = perimeter < perimeterBar;
		}

		// Outside the wire: a piece of tier 2-4 of the plan (not a dump or the dead's) out past the line
		int outside = 0;
		string outsideText = "";
		if (result.m_Wire)
		{
			foreach (RZ_SitePlaced o : result.m_aPlaced)
			{
				if (o.m_bIndoors || o.m_iLayer != RZ_ESiteLayer.PLAN || o.m_Entry.m_eZone == RZ_ESiteZone.DUMP || o.m_Entry.m_eZone == RZ_ESiteZone.DEAD || o.m_Entry.m_eZone == RZ_ESiteZone.GATE || o.m_Entry.m_eZone == RZ_ESiteZone.WIRE)
					continue;

				int ot = TierOf(o.m_Entry, p.m_eFaction);
				if (ot < 2 || ot > 4 || result.m_Wire.Inside(o.m_vPos, 0))
					continue;

				outside++;
				if (!outsideText.IsEmpty())
					outsideText += ", ";

				outsideText += PieceName(o.m_Entry);
			}
		}

		// Dropped, by tier
		int tierOneDropped = 0;
		float dropCost = 0;
		string dropped = "";
		foreach (RZ_SitePlaced drop : result.m_aDropped)
		{
			int tier = TierOf(drop.m_Entry, p.m_eFaction);
			if (tier == 1)
				tierOneDropped++;

			dropCost += DropCost(tier);
			if (!dropped.IsEmpty())
				dropped += ", ";

			dropped += string.Format("%1 (%2)", PieceName(drop.m_Entry), drop.m_sWhy);
		}

		if (dropped.IsEmpty())
			dropped = "none";

		// The score
		float score = 100;
		score -= Math.Min(45, 15 * overlaps.Count());
		score -= Math.Min(20, 2 * floating);
		bool needsGate = p.m_eKind != RZ_ECampPlace.ROAD_POST;
		if (needsGate && !gateWatch)
		{
			if (stronghold)
				score -= 25;
			else
				score -= 10;
		}

		score -= Math.Min(10, 5 * blindPosts);
		score -= Math.Min(15, 5 * (threatSides - watchedSides));
		score -= 10 * blockedPaths;
		score -= dropCost;
		score -= Math.Min(16, 4 * badRoute);
		if (perimeterLow)
			score -= Math.Min(15, (perimeterBar - perimeter) * 0.5);

		score -= Math.Min(12, 3 * outside);
		int scored = Math.ClampInt(RZ_Util.RoundToInt(score), 0, 100);

		RZ_SiteScore kept = new RZ_SiteScore();
		kept.m_sWhat = what;
		kept.m_bStronghold = stronghold;
		kept.m_iScore = scored;
		kept.m_iOverlaps = overlaps.Count();
		kept.m_bFailed = !overlaps.IsEmpty() || tierOneDropped > 0 || (stronghold && !gateWatch) || outside > 0 || perimeterLow || !wet.IsEmpty();
		kept.m_bWet = !wet.IsEmpty();
		kept.m_iPerimeter = perimeter;
		kept.m_iTierOne = tierOneDropped;
		kept.m_iOutside = outside;
		kept.m_bGateWatch = gateWatch;
		kept.m_bNeedsGate = needsGate;
		kept.m_iFaction = p.m_eFaction;
		kept.m_iRoutePoints = routePoints;
		kept.m_iRouteBad = badRoute;
		if (result.m_People)
			kept.m_iRoutes = result.m_People.m_aRoutes.Count();

		m_aScores.Insert(kept);

		int planned = result.m_aPlaced.Count() + result.m_aDropped.Count();
		string front = "";
		if (around)
			front = around.Describe() + "; ";

		string gateText = "no gate watch";
		if (gateWatch)
			gateText = "gate watched";

		string overlapText = "0 overlaps";
		if (!overlaps.IsEmpty())
			overlapText = string.Format("%1 overlaps (%2)", overlaps.Count(), Join(overlaps));

		// The plan laid down whole (Solver off) takes one step
		int solveSteps = result.m_iSolveSteps;
		int solveMost = result.m_iSolveMaxMs;
		int solvePart = result.m_iSolveMaxUnitMs;
		if (solveSteps == 0)
		{
			solveSteps = 1;
			solveMost = result.m_iSolveMs;
			solvePart = result.m_iSolveMs;
		}

		string layersText = string.Format("%1 indoors, %2 doors blocked, %3 windows boarded", result.m_iIndoors, result.m_iDoorsBlocked, result.m_iWindowsBoarded);
		if (outside > 0)
			layersText += string.Format(", %1 outside the wire (%2)", outside, outsideText);

		if (!wet.IsEmpty())
			layersText += ", over water: " + wet;

		RZ_Log.Info(string.Format("Sites: %1: score %2; %3 of %4 pieces; %5; %6; %7dropped: %8; %9", what, scored, result.m_aPlaced.Count(), planned, perimeterText, layersText, front, dropped, overlapText)
			+ string.Format("; %1 floating or buried; posts: %2", floating, PostText(gateText, posts.Count(), blindPosts, watchedSides, threatSides))
			+ string.Format("; paths: %1; routes %2 of %3 points walkable (%4); read %5 ms in %6 steps (most %7 ms, the longest part %8 ms)", paths, routePoints - badRoute, routePoints, routeText, grid.m_iReadMs, grid.m_iSteps, grid.m_iMaxStepMs, grid.m_iMaxUnitMs)
			+ string.Format(", laid out in %1 ms in %2 steps (most %3 ms, the longest part %4 ms).", result.m_iSolveMs, solveSteps, solveMost, solvePart));

		People(camp, p, what, result);
		Story(p, what, result, kept);
		if (!draw)
			return;

		RZ_SiteMarks marks = new RZ_SiteMarks();
		marks.m_vGate = gate;
		marks.m_vDoor = door;
		marks.m_aPosts = posts;
		marks.m_aPostKind = postKind;
		marks.m_aFacing = facing;
		marks.m_aClear = clear;
		marks.m_aRoute = route;
		marks.m_aRouteOk = routeOk;
		marks.m_aWalks = walks;
		marks.m_aOverlapAt = overlapAt;
		marks.m_aOverlaps = overlaps;
		Draw(id, p, grid, around, result, scored, marks);
	}

	//------------------------------------------------------------------------------------------------
	protected static string PostText(string gateText, int posts, int blind, int watched, int threats)
	{
		return string.Format("%1, %2 posts (%3 can't see the gate), %4 of %5 threatened sides watched", gateText, posts, blind, watched, threats);
	}

	//------------------------------------------------------------------------------------------------
	protected static string OkText(bool ok)
	{
		if (ok)
			return "clear";

		return "blocked";
	}

	//------------------------------------------------------------------------------------------------
	protected static void AddOverlap(notnull array<string> list, notnull array<vector> at, string text, vector where)
	{
		if (list.Count() < 12)
		{
			list.Insert(text);
			at.Insert(where);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static string AddItem(string list, string item)
	{
		if (list.IsEmpty())
			return item;

		return list + ", " + item;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Join(array<string> items)
	{
		string text = "";
		foreach (int i, string item : items)
		{
			if (i > 0)
				text += ", ";

			text += item;
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! Just in front of a piece (where its cook or worker stands): 0.8 m past its footprint's front.
	static vector FrontOf(RZ_DressingPiece e, vector pos, float yaw)
	{
		return RZ_SiteGrid.Frame(pos, yaw, 0, e.m_vFootMax[2] + 0.8);
	}

	//------------------------------------------------------------------------------------------------
	//! Where the gate is: the road entry left open, else 25 m out toward the approach.
	static vector GatePoint(RZ_CampPlace p)
	{
		if (p.m_Site && p.m_Site.m_iMain >= 0)
			return p.m_Site.m_aEntries[p.m_Site.m_iMain];

		vector from = p.m_vPos;
		if (p.m_Site)
			from = p.m_Site.m_vCentre;

		float ahead = 25;
		if (p.m_eKind == RZ_ECampPlace.ROAD_POST)
			ahead = 12;

		return RZ_Compass.PointAt(from, p.m_fApproach, ahead);
	}

	//------------------------------------------------------------------------------------------------
	//! A patrol point a body can stand at: dry, and no drop of 4 m within 6 m of it.
	static bool RouteOk(vector point)
	{
		if (RZ_Util.IsWater(point))
			return false;

		float reach;
		EWaterSurfaceType type = RZ_Land.WaterAt(point, reach);
		if (type == EWaterSurfaceType.WST_POND || type == EWaterSurfaceType.WST_RIVER)
			return false;

		BaseWorld world = GetGame().GetWorld();
		for (int k = 0; k < 8; k++)
		{
			vector near = RZ_Compass.PointAt(point, k * 45, 6);
			if (Math.AbsFloat(near[1] - point[1]) > 4)
				return false;
		}

		return world.GetSurfaceY(point[0], point[2]) > 0.5;
	}

	//------------------------------------------------------------------------------------------------
	//! Importance (docs/SITES.md, "Importance tiers"): 1 security (the counter, a watch, a barricade),
	//! 2 what the site is for, 3 life, 4 character, 5 clutter: the piece's own Tier (pieces.py), else
	//! read from its name.
	static int TierOf(RZ_DressingPiece e, RZ_EFaction f)
	{
		if (e.m_iTier > 0)
			return e.m_iTier;

		if (e.m_eAt == RZ_EDressAt.COUNTER || e.m_eUse == RZ_EDressUse.WATCH)
			return 1;

		string name = PieceName(e);
		if (name.Contains("Barricade"))
			return 1;

		if (name == "Clutter")
			return 5;

		array<string> purpose = {"RosterBoard", "DutyBoard", "TollBarrier", "Memorial", "SampleTable", "DeconShower", "ColdBox"};
		if (purpose.Contains(name) || (name == "Table" && f == RZ_EFaction.SURVIVORS))
			return 2;

		array<string> character = {"TrophyWall", "Kennel", "ChildrensCorner", "Garden", "VegStall", "FlagPole", "RedHandSheet", "PlagueBoard", "BearDoor", "DryingLine", "WashingLine", "Antenna", "Duckboards"};
		if (character.Contains(name))
			return 4;

		return 3;
	}

	//------------------------------------------------------------------------------------------------
	protected static float DropCost(int tier)
	{
		switch (tier)
		{
			case 1: return 20;
			case 2: return 6;
			case 3: return 3;
			case 4: return 1.5;
		}

		return 0.5;
	}

	//------------------------------------------------------------------------------------------------
	//! "SentryNest_Bastion" from the piece's prefab (.../RZ_Dress_SentryNest_Bastion.et).
	static string PieceName(RZ_DressingPiece e)
	{
		string path = e.m_sPrefab.GetPath();
		int at = path.LastIndexOf("RZ_Dress_");
		if (at < 0)
			return path;

		string name = path.Substring(at + 9, path.Length() - at - 9);
		name.Replace(".et", "");
		return name;
	}

	//------------------------------------------------------------------------------------------------
	static string KindName(RZ_ECampPlace kind)
	{
		switch (kind)
		{
			case RZ_ECampPlace.STRONGHOLD: return "stronghold";
			case RZ_ECampPlace.OUTPOST: return "outpost";
			case RZ_ECampPlace.SAFEHOUSE: return "safehouse";
			case RZ_ECampPlace.ROAD_POST: return "road post";
		}

		return "place";
	}

	// ================================================================ Geometry
	//------------------------------------------------------------------------------------------------
	protected static vector Middle(array<vector> quad)
	{
		return (quad[0] + quad[2]) * 0.5;
	}

	//------------------------------------------------------------------------------------------------
	//! The area two convex quads share (square metres, on the ground): one clipped by the other.
	static float OverlapArea(array<vector> a, array<vector> b)
	{
		// Far apart: no clipping
		float ra = RZ_Util.DistXZ(a[0], a[2]) * 0.5;
		float rb = RZ_Util.DistXZ(b[0], b[2]) * 0.5;
		if (RZ_Util.DistXZ(Middle(a), Middle(b)) > ra + rb)
			return 0;

		array<vector> poly = {};
		poly.InsertAll(a);
		float sign = 1;
		if (SignedArea(b) < 0)
			sign = -1;

		for (int e = 0; e < 4 && poly.Count() > 0; e++)
		{
			vector p0 = b[e];
			vector p1 = b[(e + 1) % 4];
			array<vector> next = {};
			for (int i = 0; i < poly.Count(); i++)
			{
				vector cur = poly[i];
				vector prev = poly[(i + poly.Count() - 1) % poly.Count()];
				float dc = Side(p0, p1, cur) * sign;
				float dp = Side(p0, p1, prev) * sign;
				if (dc >= 0)
				{
					if (dp < 0)
						next.Insert(Cross(prev, cur, dp, dc));

					next.Insert(cur);
				}
				else if (dp >= 0)
				{
					next.Insert(Cross(prev, cur, dp, dc));
				}
			}

			poly = next;
		}

		if (poly.Count() < 3)
			return 0;

		return Math.AbsFloat(SignedArea(poly));
	}

	//------------------------------------------------------------------------------------------------
	protected static float Side(vector p0, vector p1, vector pt)
	{
		return (p1[0] - p0[0]) * (pt[2] - p0[2]) - (p1[2] - p0[2]) * (pt[0] - p0[0]);
	}

	//------------------------------------------------------------------------------------------------
	protected static vector Cross(vector a, vector b, float da, float db)
	{
		float t = da / (da - db);
		return a + (b - a) * t;
	}

	//------------------------------------------------------------------------------------------------
	protected static float SignedArea(array<vector> poly)
	{
		float sum = 0;
		for (int i = 0; i < poly.Count(); i++)
		{
			vector u = poly[i];
			vector v = poly[(i + 1) % poly.Count()];
			sum += u[0] * v[2] - v[0] * u[2];
		}

		return sum * 0.5;
	}

	//------------------------------------------------------------------------------------------------
	//! The grid's buildings' boxes near the site, as quads.
	protected static void BuildingBoxes(RZ_SiteGrid grid, notnull array<ref array<vector>> boxes)
	{
		if (!grid)
			return;

		foreach (IEntity b : grid.GetBuildings())
		{
			vector mins;
			vector maxs;
			if (!b || !RZ_DoorGeometry.LocalBox(b, mins, maxs))
				continue;

			array<vector> quad = {};
			quad.Insert(RZ_DoorGeometry.ToWorld(b, Vector(mins[0], 0, mins[2])));
			quad.Insert(RZ_DoorGeometry.ToWorld(b, Vector(maxs[0], 0, mins[2])));
			quad.Insert(RZ_DoorGeometry.ToWorld(b, Vector(maxs[0], 0, maxs[2])));
			quad.Insert(RZ_DoorGeometry.ToWorld(b, Vector(mins[0], 0, maxs[2])));
			boxes.Insert(quad);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Square metres of solid cells (walls, trees, rocks, cars) whose middle lies in the quad.
	protected static float SolidArea(RZ_SiteGrid grid, array<vector> quad)
	{
		int n = 0;
		array<int> cells = {};
		CellsIn(grid, quad, cells);
		foreach (int c : cells)
		{
			if (grid.Flags(c % grid.m_iN, c / grid.m_iN) & RZ_SiteGrid.SOLID)
				n++;
		}

		return n * RZ_SiteGrid.CELL * RZ_SiteGrid.CELL;
	}

	//------------------------------------------------------------------------------------------------
	//! How much the ground under a quad varies (from the grid's heights), less the slope a tilted
	//! barrier run follows (`slope` a metre along the x of a piece at pos turned by yaw).
	protected static float Spread(RZ_SiteGrid grid, array<vector> quad, vector pos, float yaw, float slope)
	{
		array<int> cells = {};
		CellsIn(grid, quad, cells);
		float lo = float.MAX;
		float hi = -float.MAX;
		vector right = RZ_SiteGrid.Frame(vector.Zero, yaw, 1, 0);
		foreach (int c : cells)
		{
			float cx = grid.CellCentreX(c % grid.m_iN);
			float cz = grid.CellCentreZ(c / grid.m_iN);
			float along = (cx - pos[0]) * right[0] + (cz - pos[2]) * right[2];
			float h = grid.HeightAt(cx, cz) - slope * along;
			lo = Math.Min(lo, h);
			hi = Math.Max(hi, h);
		}

		if (cells.IsEmpty())
			return 0;

		return hi - lo;
	}

	//------------------------------------------------------------------------------------------------
	//! The grid cells whose middle lies in a convex quad.
	protected static void CellsIn(RZ_SiteGrid grid, array<vector> quad, notnull array<int> cells)
	{
		float x0 = float.MAX;
		float x1 = -float.MAX;
		float z0 = float.MAX;
		float z1 = -float.MAX;
		foreach (vector c : quad)
		{
			x0 = Math.Min(x0, c[0]);
			x1 = Math.Max(x1, c[0]);
			z0 = Math.Min(z0, c[2]);
			z1 = Math.Max(z1, c[2]);
		}

		float sign = 1;
		if (SignedArea(quad) < 0)
			sign = -1;

		for (int cz = Math.Max(0, grid.CellZ(z0)); cz <= Math.Min(grid.m_iN - 1, grid.CellZ(z1)); cz++)
		{
			for (int cx = Math.Max(0, grid.CellX(x0)); cx <= Math.Min(grid.m_iN - 1, grid.CellX(x1)); cx++)
			{
				vector pt = Vector(grid.CellCentreX(cx), 0, grid.CellCentreZ(cz));
				bool inside = true;
				for (int e = 0; e < 4; e++)
				{
					if (Side(quad[e], quad[(e + 1) % 4], pt) * sign < 0)
					{
						inside = false;
						break;
					}
				}

				if (inside)
					cells.Insert(cz * grid.m_iN + cx);
			}
		}
	}

	// ================================================================ The summary and the drawing
	//------------------------------------------------------------------------------------------------
	//! Its people (docs/SITES.md, "The people"): the posts and the routes, and the routine's dry run
	//! for the people it has in play (headless: who would man which post and walk which route).
	protected void People(RZ_CampLife camp, RZ_CampPlace p, string what, RZ_SiteResult result)
	{
		if (!result.m_People || p.m_eKind == RZ_ECampPlace.ROAD_POST)
			return;

		p.m_People = result.m_People;
		RZ_Log.Info(string.Format("Sites people: %1: %2; dry run, %3.", what, result.m_People.Describe(), camp.PeopleDryRun(p)));
	}

	//------------------------------------------------------------------------------------------------
	//! Its story (docs/SITES.md, "The story of each site"): the state now, the story it picks and
	//! which of its elements have their spot and a prefab; then each of the faction's four as if
	//! picked (the probe), so every story is placed somewhere in the run.
	protected void Story(RZ_CampPlace p, string what, RZ_SiteResult result, RZ_SiteScore kept)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int first = RZ_SiteStory.First(p.m_eFaction);
		if (!gm || first < 0 || p.m_eKind == RZ_ECampPlace.ROAD_POST || !gm.GetCampLife() || !gm.GetCampLife().GetSettings().m_bStory)
			return;

		RZ_SiteFacts facts = RZ_SiteStory.Read(gm, p);
		int story = RZ_SiteStory.Pick(p.m_eFaction, facts, p.m_iSeed);
		kept.m_iStory = story;
		string own = "";
		string four = "";
		for (int s = first; s < first + 4; s++)
		{
			array<int> kinds = {};
			array<int> counts = {};
			RZ_SiteStory.Elements(s, facts, kinds, counts);
			int stand = 0;
			string missing = "";
			foreach (int kind : kinds)
			{
				bool spot = RZ_CampLife.StorySpotOf(result.m_aPlaced, kind) != null;
				bool prefab = !RZ_SiteStory.Prefab(kind, p.m_eFaction, facts, 0).IsEmpty();
				if (spot && prefab)
				{
					stand++;
					continue;
				}

				string why = "no spot";
				if (spot)
					why = "no prefab";

				missing = AddItem(missing, RZ_SiteStory.ElementName(kind) + ": " + why);
			}

			int whole = 0;
			if (stand == kinds.Count())
				whole = 1;

			kept.m_aStoryWhole.Insert(whole);
			string line = string.Format("%1 %2 of %3", RZ_SiteStory.Name(s), stand, kinds.Count());
			if (!missing.IsEmpty())
				line += " (" + missing + ")";

			if (s == story)
				own = line;

			if (!four.IsEmpty())
				four += "; ";

			four += line;
		}

		int spots = 0;
		foreach (RZ_SitePlaced placed : result.m_aPlaced)
		{
			if (placed.m_iLayer == RZ_ESiteLayer.STORY)
				spots++;
		}

		array<int> all = {};
		RZ_SiteStory.FactionElements(p.m_eFaction, all);
		RZ_Log.Info(string.Format("Sites story: %1: %2 (%3); %4 of %5 spots kept; its elements on their spots: %6; the four: %7.", what, RZ_SiteStory.Name(story), facts.Describe(), spots, all.Count(), own, four));
	}

	//------------------------------------------------------------------------------------------------
	//! "Sites audit: 31 sites, mean 82, worst 64 (Red Hand outpost at 041-077), 0 overlaps." and the
	//! sites under the pass mark. How many strongholds are under it.
	int Summary(int threshold)
	{
		if (m_aScores.IsEmpty())
		{
			RZ_Log.Info("Sites audit: no sites.");
			return 0;
		}

		int total = 0;
		int overlaps = 0;
		RZ_SiteScore worst;
		array<string> under = {};
		int strongholdsUnder = 0;
		int failed = 0;
		foreach (RZ_SiteScore s : m_aScores)
		{
			total += s.m_iScore;
			overlaps += s.m_iOverlaps;
			if (!worst || s.m_iScore < worst.m_iScore)
				worst = s;

			if (s.m_bFailed)
				failed++;

			if (s.m_iScore < threshold)
			{
				under.Insert(string.Format("%1 (%2)", s.m_sWhat, s.m_iScore));
				if (s.m_bStronghold)
					strongholdsUnder++;
			}
		}

		float sum = total;
		int mean = RZ_Util.RoundToInt(sum / m_aScores.Count());
		RZ_Log.Info(string.Format("Sites audit: %1 sites, mean %2, worst %3 (%4), %5 overlaps; %6 with a hard failure; %7 under %8%9", m_aScores.Count(), mean, worst.m_iScore, worst.m_sWhat, overlaps, failed, under.Count(), threshold, UnderText(under)));

		// The strongholds' perimeters and what was dropped that mustn't be
		string walls = "";
		int tierOne = 0;
		int outside = 0;
		int wetSites = 0;
		foreach (RZ_SiteScore w : m_aScores)
		{
			tierOne += w.m_iTierOne;
			outside += w.m_iOutside;
			if (w.m_bWet)
				wetSites++;

			if (!w.m_bStronghold)
				continue;

			if (!walls.IsEmpty())
				walls += ", ";

			if (w.m_iPerimeter < 0)
				walls += w.m_sWhat + " no wire";
			else
				walls += w.m_sWhat + " " + w.m_iPerimeter.ToString() + "%";
		}

		RZ_Log.Info(string.Format("Sites audit perimeters: %1; %2 tier 1 pieces dropped, %3 pieces outside the wire, %4 sites with something over water.", walls, tierOne, outside, wetSites));

		// The people: every stronghold's gate watched, the routes walkable
		int strongholds = 0;
		int watched = 0;
		int gates = 0;
		int gatesWatched = 0;
		int points = 0;
		int bad = 0;
		int routes = 0;
		string blind = "";
		foreach (RZ_SiteScore g : m_aScores)
		{
			points += g.m_iRoutePoints;
			bad += g.m_iRouteBad;
			routes += g.m_iRoutes;
			if (!g.m_bNeedsGate)
				continue;

			gates++;
			if (g.m_bGateWatch)
				gatesWatched++;
			else
				blind = AddItem(blind, g.m_sWhat);

			if (!g.m_bStronghold)
				continue;

			strongholds++;
			if (g.m_bGateWatch)
				watched++;
		}

		if (blind.IsEmpty())
			blind = "none";

		RZ_Log.Info(string.Format("Sites audit people: %1 of %2 strongholds and %3 of %4 sites with a gate have a post that sees it (blind: %5); %6 routes, %7 of %8 points walkable.", watched, strongholds, gatesWatched, gates, blind, routes, points - bad, points));

		// The stories: which each faction's sites pick, and where each of the four stands whole
		array<int> factions = {RZ_EFaction.MILITARY, RZ_EFaction.BANDITS, RZ_EFaction.SURVIVORS, RZ_EFaction.INSTITUTE};
		string stories = "";
		foreach (int f : factions)
		{
			int first = RZ_SiteStory.First(f);
			int sites = 0;
			foreach (RZ_SiteScore c : m_aScores)
			{
				if (c.m_iFaction == f && c.m_iStory >= 0)
					sites++;
			}

			if (sites == 0)
				continue;

			string text = "";
			for (int st = first; st < first + 4; st++)
			{
				int picked = 0;
				int whole = 0;
				foreach (RZ_SiteScore d : m_aScores)
				{
					if (d.m_iFaction != f || d.m_iStory < 0)
						continue;

					if (d.m_iStory == st)
						picked++;

					if (st - first < d.m_aStoryWhole.Count() && d.m_aStoryWhole[st - first] == 1)
						whole++;
				}

				text = AddItem(text, string.Format("%1 picked at %2, whole at %3 of %4", RZ_SiteStory.Name(st), picked, whole, sites));
			}

			if (!stories.IsEmpty())
				stories += "; ";

			stories += RZ_Factions.GetName(f) + ": " + text;
		}

		if (stories.IsEmpty())
			stories = "none";

		RZ_Log.Info("Sites audit stories: " + stories + ".");
		return strongholdsUnder;
	}

	//------------------------------------------------------------------------------------------------
	protected static string UnderText(array<string> under)
	{
		if (under.IsEmpty())
			return ".";

		return ": " + Join(under) + ".";
	}

	//------------------------------------------------------------------------------------------------
	//! The lines tools/camp_kit/render.py --placed draws from: "Sites draw <id> <what> ...".
	protected void Draw(int id, RZ_CampPlace p, RZ_SiteGrid grid, RZ_SiteSurroundings around, RZ_SiteResult result, int score, RZ_SiteMarks marks)
	{
		vector gate = marks.m_vGate;
		vector door = marks.m_vDoor;
		string head = string.Format("Sites draw %1 ", id);
		Print(head + string.Format("site %1 %2 %3 %4 %5 %6 %7 %8", p.m_eFaction, p.m_eKind, RZ_SettlementScanner.GridReference(p.m_vPos), Num(grid.m_vCentre[0]), Num(grid.m_vCentre[2]), Num(grid.m_fHalf), score, p.m_sKey) + " " + p.m_sTitle);
		Print(head + string.Format("grid %1 %2 %3", Num(grid.m_vLow[0]), Num(grid.m_vLow[2]), grid.m_iL - 1));
		for (int row = 0; row < grid.m_iL - 1; row++)
		{
			Print(head + "row " + row.ToString() + " " + grid.DrawRow(row));
		}

		if (p.m_Site)
		{
			vector mins = p.m_Site.m_vMins;
			vector maxs = p.m_Site.m_vMaxs;
			array<vector> corners = {Vector(mins[0], 0, mins[2]), Vector(maxs[0], 0, mins[2]), Vector(maxs[0], 0, maxs[2]), Vector(mins[0], 0, maxs[2])};
			string bld = "";
			foreach (vector local : corners)
			{
				vector w = p.m_Site.m_vOrigin + p.m_Site.m_vRight * local[0] + p.m_Site.m_vFwd * local[2];
				bld += " " + Num(w[0]) + " " + Num(w[2]);
			}

			Print(head + "building " + p.m_Site.m_sWhat + " |" + bld);
			foreach (int k, vector entry : p.m_Site.m_aEntries)
			{
				string kind = "closed";
				if (k == p.m_Site.m_iMain)
					kind = "main";

				Print(head + string.Format("entry %1 %2 %3", Num(entry[0]), Num(entry[2]), kind));
			}
		}

		Print(head + string.Format("gate %1 %2", Num(gate[0]), Num(gate[2])));
		Print(head + string.Format("door %1 %2", Num(door[0]), Num(door[2])));
		if (around)
			Print(head + "around " + around.DrawText());

		foreach (RZ_SitePlaced placed : result.m_aPlaced)
		{
			if (placed.m_bIndoors)
				Print(head + string.Format("indoors %1 %2", TierOf(placed.m_Entry, p.m_eFaction), PieceName(placed.m_Entry)));
			else
				Print(head + PieceText("piece", placed, p.m_eFaction));
		}

		if (result.m_Wire)
			result.m_Wire.DrawLines(head);

		foreach (RZ_SitePlaced dropped : result.m_aDropped)
		{
			Print(head + PieceText("drop", dropped, p.m_eFaction) + " | " + dropped.m_sWhy);
		}

		foreach (int q, vector post : marks.m_aPosts)
		{
			Print(head + string.Format("post %1 %2 %3 %4 %5", marks.m_aPostKind[q], Num(post[0]), Num(post[2]), RZ_Util.RoundToInt(marks.m_aFacing[q]), YesNo(marks.m_aClear[q])));
		}

		string loop = "";
		foreach (int r, vector point : marks.m_aRoute)
		{
			loop += string.Format(" %1 %2 %3", Num(point[0]), Num(point[2]), YesNo(marks.m_aRouteOk[r]));
		}

		if (!loop.IsEmpty())
			Print(head + "route" + loop);

		foreach (string walkLine : marks.m_aWalks)
		{
			Print(head + "walk " + walkLine);
		}

		foreach (int o, vector at : marks.m_aOverlapAt)
		{
			Print(head + string.Format("overlap %1 %2 ", Num(at[0]), Num(at[2])) + marks.m_aOverlaps[o]);
		}

		foreach (int b, string big : grid.m_aBig)
		{
			Print(head + string.Format("solid %1 %2", grid.m_aBigCount[b], big));
		}

		vector look = gate;
		Print(head + string.Format("goto %1 %2 %3 %4", Num(grid.m_vCentre[0]), Num(grid.m_vCentre[2]), Num(look[0]), Num(look[2])));
	}

	//------------------------------------------------------------------------------------------------
	//! "piece <tier> <x> <z> <yaw> <name> <props> <rect x z w d yaw> ...": the footprint as data.
	protected static string PieceText(string kind, RZ_SitePlaced placed, RZ_EFaction f)
	{
		RZ_SiteFoot foot = placed.m_Entry.Foot();
		string name = PieceName(placed.m_Entry);
		if (placed.m_iStory >= 0)
			name = "Story_" + RZ_SiteStory.SceneKind(placed.m_iStory);

		string text = string.Format("%1 %2 %3 %4 %5 %6 %7", kind, TierOf(placed.m_Entry, f), Num(placed.m_vPos[0]), Num(placed.m_vPos[2]), RZ_Util.RoundToInt(placed.m_fYaw), name, foot.m_iProps);
		for (int r = 0; r < foot.Count(); r++)
		{
			text += " " + foot.RectText(r);
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	protected static string YesNo(bool yes)
	{
		if (yes)
			return "1";

		return "0";
	}

	//------------------------------------------------------------------------------------------------
	protected static string Num(float v)
	{
		return (Math.Round(v * 10) / 10).ToString();
	}
}
