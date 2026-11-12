//! One placer for scenery (docs/TRACES.md, "Where scenes stand"; docs/DESIGN.md, Camp life): Traces'
//! scenes' ground rules, so a scene can be several pieces round its centre; camp life shares its
//! ground read and fits whole footprints by its own rules. A spot is dry, off the line, flat (under about 12 degrees over 3 m), out
//! of every building's footprint (2 m), 6 m off a road's centre line, apart from the pieces already
//! placed and clear of the spots that must stay free (a quartermaster, the medic, a barrier).
//! Read once a place: the buildings within 70 m and the road segments within 160 m (camp life reads
//! further for a place that moves into a building). Server only.

//------------------------------------------------------------------------------------------------
//! The ground near a place, read once (ReadGround).
class RZ_DressGround
{
	ref array<IEntity> m_aBuildings = {};
	ref array<vector> m_aRoad = {};		// road segments, two points each
	ref array<int> m_aRoadId = {};		// which road each segment belongs to
}

//------------------------------------------------------------------------------------------------
class RZ_DressPlacer
{
	static const float FLAT_STEP = 0.64;		// height change over 3 m: about 12 degrees
	static const float ROAD_CLEAR = 6;			// from a road's centre line
	static const float BUILDING_MARGIN = 2;
	static const float BUILDINGS_NEAR = 70;
	static const float ROADS_NEAR = 160;

	//------------------------------------------------------------------------------------------------
	//! The buildings within 70 m of pos and the road segments within 160 m.
	static void ReadGround(vector pos, notnull RZ_DressGround ground)
	{
		ReadGroundWithin(pos, ground, BUILDINGS_NEAR, ROADS_NEAR);
	}

	//------------------------------------------------------------------------------------------------
	//! The buildings within buildingsR of pos and the road segments within roadsR (camp life reads
	//! further when a place moves into a building), with which road each segment belongs to.
	static void ReadGroundWithin(vector pos, notnull RZ_DressGround ground, float buildingsR, float roadsR)
	{
		RZ_BuildingQuery.FindBuildings(pos, buildingsR, ground.m_aBuildings);
		RoadNetworkManager roads = RZ_Perimeter.GetRoadNetwork();
		if (!roads)
			return;

		vector mins = Vector(pos[0] - roadsR, -1000, pos[2] - roadsR);
		vector maxs = Vector(pos[0] + roadsR, 10000, pos[2] + roadsR);
		array<BaseRoad> found = {};
		roads.GetRoadsInAABB(mins, maxs, found); // VERIFY: the road network's points follow every drivable road, so pieces keep off them (else a piece can stand on a road)
		foreach (int id, BaseRoad road : found)
		{
			if (!road)
				continue;

			array<vector> points = {};
			road.GetPoints(points);
			for (int i = 1; i < points.Count(); i++)
			{
				ground.m_aRoad.Insert(points[i - 1]);
				ground.m_aRoad.Insert(points[i]);
				ground.m_aRoadId.Insert(id);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Slope under about 12 degrees, from four ground heights 1.5 m around the spot.
	static bool IsFlat(vector spot)
	{
		BaseWorld world = GetGame().GetWorld();
		float east = world.GetSurfaceY(spot[0] + 1.5, spot[2]);
		float west = world.GetSurfaceY(spot[0] - 1.5, spot[2]);
		float north = world.GetSurfaceY(spot[0], spot[2] + 1.5);
		float south = world.GetSurfaceY(spot[0], spot[2] - 1.5);
		return Math.AbsFloat(east - west) < FLAT_STEP && Math.AbsFloat(north - south) < FLAT_STEP;
	}

	//------------------------------------------------------------------------------------------------
	static float DistToSegment(vector pt, vector a, vector b)
	{
		float dx = b[0] - a[0];
		float dz = b[2] - a[2];
		float len2 = dx * dx + dz * dz;
		if (len2 < 0.01)
			return RZ_Util.DistXZ(pt, a);

		float t = ((pt[0] - a[0]) * dx + (pt[2] - a[2]) * dz) / len2;
		t = Math.Clamp(t, 0, 1);
		vector proj = a;
		proj[0] = a[0] + dx * t;
		proj[2] = a[2] + dz * t;
		return RZ_Util.DistXZ(pt, proj);
	}

	//------------------------------------------------------------------------------------------------
	//! The shared rules: dry, off the line, flat, `gap` from every spot in `taken`, `keepR` from
	//! every spot in `keepClear`, 6 m off the roads read, out of every building's footprint (2 m).
	static bool Good(vector spot, notnull RZ_DressGround ground, array<vector> keepClear, float keepR, array<vector> taken, float gap)
	{
		if (RZ_Util.IsWater(spot) || RZ_Perimeter.IsNearLineAt(spot) || !IsFlat(spot))
			return false;

		if (keepClear)
		{
			foreach (vector k : keepClear)
			{
				if (RZ_Util.DistXZ(k, spot) < keepR)
					return false;
			}
		}

		if (taken)
		{
			foreach (vector other : taken)
			{
				if (RZ_Util.DistXZ(other, spot) < gap)
					return false;
			}
		}

		for (int i = 0; i + 1 < ground.m_aRoad.Count(); i += 2)
		{
			if (DistToSegment(spot, ground.m_aRoad[i], ground.m_aRoad[i + 1]) < ROAD_CLEAR)
				return false;
		}

		foreach (IEntity b : ground.m_aBuildings)
		{
			if (b && RZ_LockedSpots.InFootprint(b, spot, BUILDING_MARGIN))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A spot in a ring round `centre` by the shared rules, the same draws for the same seed (so a
	//! scene's pieces stand in the same places after a restart). False if none of ten fits.
	static bool Around(vector centre, float minR, float maxR, int seed, notnull RZ_DressGround ground, array<vector> taken, float gap, out vector spot)
	{
		RZ_Rng rng = new RZ_Rng(seed);
		for (int i = 0; i < 10; i++)
		{
			float bearing = rng.Range(0, 360);
			float r = minR + rng.Float01() * (maxR - minR);
			vector cand = RZ_Util.GroundPos(RZ_Compass.PointAt(centre, bearing, r));
			if (Good(cand, ground, null, 0, taken, gap))
			{
				spot = cand;
				return true;
			}
		}

		return false;
	}
}
