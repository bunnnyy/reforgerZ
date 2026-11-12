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
	bool m_bFailed;			// a hard failure: an overlap, a tier 1 piece dropped, a stronghold with no gate watch
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
	ref array<vector> m_aOverlapAt = {};
	ref array<string> m_aOverlaps = {};
}

//------------------------------------------------------------------------------------------------
class RZ_SiteAudit
{
	static const float OVERLAP_AREA = 0.1;		// m2: less than this between two footprints isn't an overlap
	static const float FLOAT_SPREAD = 0.3;		// ground under one prop varying more than this: one edge floats or is buried
	static const float THREAT_WATCH = 3;		// a side this threatened wants a watch facing it

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
			if (entry.m_eAt == RZ_EDressAt.COUNTER)
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
				if (!seat[f] && Spread(grid, under) > FLOAT_SPREAD)
					floating++;
			}
		}

		// Posts: the watch pieces and the building's lookouts, the gate, and what they see
		vector gate = GatePoint(p);
		array<vector> posts = {};
		array<float> facing = {};
		array<string> postKind = {};
		foreach (RZ_SitePlaced w : result.m_aPlaced)
		{
			if (w.m_Entry.m_eUse == RZ_EDressUse.WATCH)
			{
				posts.Insert(w.m_vPos);
				facing.Insert(w.m_fYaw);
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

		bool gateWatch = false;
		int blindPosts = 0;
		array<bool> clear = {};
		foreach (int q, vector post : posts)
		{
			bool sees = !grid || grid.LineClear(post, gate);
			clear.Insert(sees);
			if (!sees)
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

		int blockedPaths = 0;
		string paths = "";
		if (grid && p.m_eKind != RZ_ECampPlace.ROAD_POST)
		{
			bool toGate = grid.PathClear(door, gate);
			paths = "door to gate " + OkText(toGate);
			if (!toGate)
				blockedPaths++;

			foreach (RZ_SitePlaced cook : result.m_aPlaced)
			{
				if (cook.m_Entry.m_eUse != RZ_EDressUse.COOK)
					continue;

				bool toKitchen = grid.PathClear(door, cook.m_vPos);
				paths += ", door to kitchen " + OkText(toKitchen);
				if (!toKitchen)
					blockedPaths++;

				break;
			}
		}

		// The patrol's loop: walkable, not in water, not over a drop
		array<vector> route = {};
		if (p.m_eKind != RZ_ECampPlace.ROAD_POST)
			RZ_CampLife.PatrolPoints(p, 0, route);

		array<bool> routeOk = {};
		int badRoute = 0;
		foreach (vector point : route)
		{
			bool ok = RouteOk(point);
			routeOk.Insert(ok);
			if (!ok)
				badRoute++;
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
		int scored = Math.ClampInt(RZ_Util.RoundToInt(score), 0, 100);

		RZ_SiteScore kept = new RZ_SiteScore();
		kept.m_sWhat = what;
		kept.m_bStronghold = stronghold;
		kept.m_iScore = scored;
		kept.m_iOverlaps = overlaps.Count();
		kept.m_bFailed = !overlaps.IsEmpty() || tierOneDropped > 0 || (stronghold && !gateWatch);
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

		RZ_Log.Info(string.Format("Sites: %1: score %2; %3 of %4 pieces; perimeter not built yet; %5dropped: %6; %7; %8 floating or buried; posts: %9", what, scored, result.m_aPlaced.Count(), planned, front, dropped, overlapText, floating, PostText(gateText, posts.Count(), blindPosts, watchedSides, threatSides))
			+ string.Format("; paths: %1; patrol %2 of %3 points walkable; read %4 ms in %5 steps (most %6 ms, the longest part %7 ms), laid out in %8 ms.", paths, route.Count() - badRoute, route.Count(), grid.m_iReadMs, grid.m_iSteps, grid.m_iMaxStepMs, grid.m_iMaxUnitMs, result.m_iSolveMs));

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
	protected static bool RouteOk(vector point)
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
	//! Importance by the piece's kind (docs/SITES.md, "Importance tiers"): 1 security (the counter, a
	//! watch, a barricade), 2 what the site is for, 3 life, 4 character, 5 clutter. Read from the
	//! piece's name until the pieces carry their own tier.
	static int TierOf(RZ_DressingPiece e, RZ_EFaction f)
	{
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
	//! How much the ground under a quad varies (from the grid's heights).
	protected static float Spread(RZ_SiteGrid grid, array<vector> quad)
	{
		array<int> cells = {};
		CellsIn(grid, quad, cells);
		float lo = float.MAX;
		float hi = -float.MAX;
		foreach (int c : cells)
		{
			float h = grid.HeightAt(grid.CellCentreX(c % grid.m_iN), grid.CellCentreZ(c / grid.m_iN));
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
			Print(head + PieceText("piece", placed, p.m_eFaction));
		}

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
		string text = string.Format("%1 %2 %3 %4 %5 %6 %7", kind, TierOf(placed.m_Entry, f), Num(placed.m_vPos[0]), Num(placed.m_vPos[2]), RZ_Util.RoundToInt(placed.m_fYaw), PieceName(placed.m_Entry), foot.m_iProps);
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
