//! Where the power station is, and where its control stands (docs/DESIGN.md, "The power grid").
//!
//! The station is real infrastructure wherever the map has it: a power plant anywhere on the map
//! with towns in reach, else a power plant, a transformer station or a substation near a town
//! (biggest towns first), else a factory, sawmill or warehouse, else a real building on the edge
//! of the biggest town. The control is never left in
//! an open field: it is the map's own electrical cabinet at the station (an invisible interaction
//! spot on it), else the control prefab stood against the station's wall beside a door, else beside
//! the transformer the station is.
//!
//! The station's dressing (fuel drums and a warning sign) stands beside the control, along the same
//! wall, and its generator's sound plays at the map's own generator sets there when it has them
//! (World/RZ_PowerLooks.c).
//!
//! Server only, at setup: a few sphere queries over static objects, matched by prefab name.

//------------------------------------------------------------------------------------------------
//! Where the control goes.
class RZ_PowerPlacement
{
	vector m_vPos;
	float m_fYaw;
	bool m_bSpot;		// on a vanilla cabinet: an invisible spot, no prop of ours
	string m_sHow;		// for the log: "the cabinet", "by a door", "against a wall", "beside it", "as placed"
	bool m_bDress;		// the station's dressing has a free spot beside the control
	string m_sDressWhy;	// when it hasn't: what was in the way of each spot tried, for the log
	vector m_vDressPos;
	float m_fDressYaw;
}

//------------------------------------------------------------------------------------------------
class RZ_PowerSite
{
	static const int MODE_POWER = 0;		// power plants, transformer stations, substations
	static const int MODE_INDUSTRY = 1;		// factory halls, boiler houses, sawmills, warehouses
	static const int MODE_CABINET = 2;		// the map's own electrical cabinets and control boxes
	static const int MODE_GENERATOR = 3;	// the map's own generator sets (a power plant's)

	protected static const float WALL_GAP = 0.5;		// the control stands this far (m) off the wall
	protected static const float DOOR_SIDE = 2.2;		// and this far (m) to the side of the door (clear of a wide cabinet)
	protected static const float BESIDE = 3;			// beside a transformer or a yard's box (m)
	protected static const float SPOT_GAP = 0.25;		// a spot sits this far (m) out from a cabinet's face
	protected static const float DRESS_SIDE = 2.4;		// the dressing stands this far (m) along the wall from the control
	protected static const float DRESS_DOOR = 0.8;		// or this far (m) past the door's other side
	protected static const float DRESS_MORE = 2;		// a second try, this much further along
	protected static const float DRESS_OUT = 1.5;		// then both again this far out from the wall, and twice as far
	protected static const float DRESS_SMALL = 1.5;		// off the middle of a wall or fence piece (no box of its own)
	protected static const int MAX_GENERATORS = 2;

	protected int m_iMode;
	protected vector m_vCenter;
	protected ref array<IEntity> m_aAll;		// MODE_GENERATOR: every match, not only the best
	protected IEntity m_Best;
	protected int m_iBestScore;
	protected float m_fBestDist;

	//------------------------------------------------------------------------------------------------
	//! The best match of `mode` within `radius` of `center` (highest score, then nearest); null if none.
	static IEntity FindBest(vector center, float radius, int mode, out int score)
	{
		RZ_PowerSite q = new RZ_PowerSite();
		q.m_iMode = mode;
		q.m_vCenter = center;
		q.m_fBestDist = float.MAX;
		GetGame().GetWorld().QueryEntitiesBySphere(center, radius, q.OnEntity, null, EQueryEntitiesFlags.STATIC);
		score = q.m_iBestScore;
		return q.m_Best;
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnEntity(IEntity e)
	{
		if (!e || RZ_CampLife.IsDressing(e))
			return true;

		string name = RZ_Util.GetPrefabName(e);
		if (name.IsEmpty())
			return true;

		name.ToLower();
		int s = Score(m_iMode, name);
		if (s <= 0)
			return true;

		// A real building beats one of its parts (a vent, the stairs) of the same kind
		if (m_iMode != MODE_CABINET && SCR_DestructibleBuildingEntity.Cast(e))
			s += 1;

		// A cabinet behind a locked door is no use to anyone
		if (m_iMode == MODE_CABINET && RZ_LockedSpots.IsBlocked(e.GetOrigin()))
			return true;

		if (m_aAll && !m_aAll.Contains(e))
			m_aAll.Insert(e);

		float d = RZ_Util.DistXZ(e.GetOrigin(), m_vCenter);
		if (s > m_iBestScore || (s == m_iBestScore && d < m_fBestDist))
		{
			m_Best = e;
			m_iBestScore = s;
			m_fBestDist = d;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Every power plant building on the map, wherever it stands: one pass over the map's static
	//! entities (about a second on Everon, as the settlement scan's), at setup.
	static void FindPlants(notnull array<IEntity> result)
	{
		RZ_PowerSite q = new RZ_PowerSite();
		q.m_aAll = result;
		vector mins;
		vector maxs;
		BaseWorld world = GetGame().GetWorld();
		world.GetBoundBox(mins, maxs);
		world.QueryEntitiesByAABB(mins, maxs, q.OnPlant, null, EQueryEntitiesFlags.STATIC);
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnPlant(IEntity e)
	{
		// The building itself, not its furniture, its rooftop sign or a wreck of it
		if (!e || !SCR_DestructibleBuildingEntity.Cast(e) || RZ_CampLife.IsDressing(e))
			return true;

		string name = RZ_Util.GetPrefabName(e);
		name.ToLower();
		if (Score(MODE_POWER, name) >= 40)
			m_aAll.Insert(e);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Wrecks, ruins, debris and building probes never count.
	protected static bool Skip(string name)
	{
		return name.Contains("/dst/") || name.Contains("_dst") || name.Contains("ruin") || name.Contains("debris") || name.Contains("probe") || name.Contains("ladder") || name.Contains("/signs/");
	}

	//------------------------------------------------------------------------------------------------
	//! 0 = no match; higher is better. Scores are in steps of 10 so a building's +1 never jumps a kind.
	//! Vanilla names (Everon and Arland): Infrastructure/Power/DieselPowerPlant_01,
	//! TransformerStation_E_01/E_02, TransformerSubstation_01, TransformerStation110kV_E_01 (a yard,
	//! with ControlBox110kV_E_01), TransformerStation40kV_E_01; Industrial/Houses/FactoryHall_E_01,
	//! BoilerHouse_E_02, Sawmill_E_01, ConcretePlant_E_01, Warehouse_01, Workshop_01/02; the cabinets
	//! Infrastructure/Power/Utility/ElectricCabinet_01, UtilityCabinet_02 and BuildingParts/Eletrical/
	//! CabinetElectricalLargeSet_01, BreakerBox_0x, UtilityBoxExt_02, ElectricityBox_0x.
	static int Score(int mode, string name)
	{
		if (Skip(name))
			return 0;

		if (mode == MODE_POWER)
		{
			if (name.Contains("powerplant") || name.Contains("power_plant") || name.Contains("powerstation") || name.Contains("power_station"))
				return 40;

			if (name.Contains("transformerstation_") || name.Contains("substation"))
			{
				// The 110 kV and 40 kV kinds are open yards; the others are buildings with a door
				if (name.Contains("110kv") || name.Contains("40kv"))
					return 20;

				return 30;
			}

			if (name.Contains("controlbox110kv") || name.Contains("transformer"))
				return 10;

			return 0;
		}

		if (mode == MODE_INDUSTRY)
		{
			// Buildings only: a barrel or a gas tank under Props/Industrial is not a factory
			if (!name.Contains("/structures/") || name.Contains("buildingparts") || name.Contains("furniture"))
				return 0;

			if (name.Contains("factoryhall") || name.Contains("boilerhouse") || name.Contains("factory"))
				return 30;

			if (name.Contains("sawmill") || name.Contains("concreteplant"))
				return 20;

			if (name.Contains("warehouse") || name.Contains("workshop_0"))
				return 10;

			return 0;
		}

		if (mode == MODE_GENERATOR)
		{
			// Structures/BuildingParts/Eletrical/DieselGenerator_01 (two stand in DieselPowerPlant_01)
			if (name.Contains("dieselgenerator"))
				return 10;

			return 0;
		}

		// MODE_CABINET: the map's own electrical boxes, outside ones first (an interior box's face is a guess)
		if (name.Contains("interior") || name.Contains("utilityboxint") || name.Contains("furniture"))
			return 0;

		if (name.Contains("controlbox110kv"))
			return 60;

		if (name.Contains("electriccabinet_0"))
			return 50;

		if (name.Contains("cabinetelectricallarge") || name.Contains("utilitycabinet"))
			return 40;

		if (name.Contains("breakerbox") || name.Contains("utilityboxext"))
			return 30;

		if (name.Contains("electricitybox") || name.Contains("controlpanel_0"))
			return 20;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! What the station is called, by what it is: "the power station outside Morton".
	static string NameFor(int mode, int score, string town, bool inTown)
	{
		string where = " outside ";
		if (inTown)
			where = " in ";

		string what = "the power station";
		if (mode == MODE_POWER && score >= 30 && score < 40)
			what = "the substation";
		else if (mode == MODE_POWER && score >= 20 && score < 30)
			what = "the transformer yard";
		else if (mode == MODE_POWER && score < 20)
			what = "the transformer";
		else if (mode == MODE_INDUSTRY && score >= 30)
			what = "the factory's switch room";
		else if (mode == MODE_INDUSTRY && score >= 20)
			what = "the sawmill's switch room";
		else if (mode == MODE_INDUSTRY)
			what = "the warehouse switch room";

		return what + where + town;
	}

	//------------------------------------------------------------------------------------------------
	//! A real building on the edge of a town (the farthest from its middle within its radius) with
	//! a usable box, not locked, preferably with an outside door. null if the town has none.
	static IEntity EdgeBuilding(RZ_Settlement s)
	{
		array<IEntity> found = {};
		RZ_BuildingQuery.FindBuildings(s.m_vCenter, s.m_fRadius + 60, found, 200);
		IEntity best;
		float bestScore = -1;
		foreach (IEntity b : found)
		{
			vector mins;
			vector maxs;
			if (!b || !RZ_DoorGeometry.LocalBox(b, mins, maxs) || RZ_LockedSpots.IsLockedBuilding(b))
				continue;

			float score = RZ_Util.DistXZ(b.GetOrigin(), s.m_vCenter);
			if (OuterDoor(b, s.m_vCenter))
				score += 1000;

			if (score > bestScore)
			{
				bestScore = score;
				best = b;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! The outside door of `building` nearest `toward`, or null.
	static IEntity OuterDoor(IEntity building, vector toward)
	{
		array<IEntity> doors = {};
		RZ_DoorGeometry.FindDoors(building, 0, doors);
		IEntity best;
		float bestDist = float.MAX;
		foreach (IEntity d : doors)
		{
			if (!RZ_DoorGeometry.IsOuterDoor(building, d))
				continue;

			float dist = RZ_Util.DistXZ(d.GetOrigin(), toward);
			if (dist < bestDist)
			{
				bestDist = dist;
				best = d;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! A station's anchor near `pos` (a marker or a saved site): power, then industry, then the
	//! nearest building with a box. null if there's nothing within `radius`.
	static IEntity AnchorNear(vector pos, float radius)
	{
		int score;
		IEntity e = FindBest(pos, radius, MODE_POWER, score);
		if (e)
			return e;

		e = FindBest(pos, radius, MODE_INDUSTRY, score);
		if (e)
			return e;

		array<IEntity> found = {};
		RZ_BuildingQuery.FindBuildings(pos, radius, found, 64);
		IEntity best;
		float bestDist = float.MAX;
		foreach (IEntity b : found)
		{
			vector mins;
			vector maxs;
			if (!b || !RZ_DoorGeometry.LocalBox(b, mins, maxs))
				continue;

			float d = RZ_Util.DistXZ(b.GetOrigin(), pos);
			if (d < bestDist)
			{
				bestDist = d;
				best = b;
			}
		}

		return best;
	}

	// ================================================================ Placing the control
	//------------------------------------------------------------------------------------------------
	//! Where the control goes at a station anchored on `anchor` (may be null) at `site`, facing
	//! people coming from `toward` (the town). `cabinets`: a vanilla electrical cabinet within
	//! `reach` m may carry it as an invisible spot.
	static void Place(IEntity anchor, vector site, vector toward, bool cabinets, float reach, RZ_PowerPlacement result)
	{
		result.m_bSpot = false;
		result.m_bDress = false;
		if (cabinets)
		{
			int score;
			IEntity box = FindBest(site, reach, MODE_CABINET, score);
			if (box)
			{
				result.m_vPos = FrontOf(box, toward);
				result.m_fYaw = 0;
				result.m_bSpot = true;
				result.m_sHow = "on the cabinet there";

				// The dressing in line with the cabinet, clear of its width and of what it stands against,
				// facing the way its face does
				vector boxMins;
				vector boxMaxs;
				box.GetBounds(boxMins, boxMaxs);
				float clear = Math.Max(boxMaxs[0] - boxMins[0], boxMaxs[2] - boxMins[2]) * 0.5 + DRESS_SIDE - 0.5;
				vector face = Flat(result.m_vPos - box.GetOrigin(), toward - box.GetOrigin());
				vector along = Vector(face[2], 0, -face[0]);
				DressAlong(box.GetOrigin() + face * WALL_GAP, along, face, clear, clear, anchor, result);

				return;
			}
		}

		vector mins;
		vector maxs;
		if (anchor && RZ_DoorGeometry.LocalBox(anchor, mins, maxs))
		{
			IEntity door = OuterDoor(anchor, toward);
			if (door)
			{
				// Against the wall, a step to the side of the door, its front facing out
				vector n = RZ_DoorGeometry.WallNormal(anchor, door);
				vector side = Vector(n[2], 0, -n[0]);
				vector byDoor = RZ_DoorGeometry.OutsidePoint(anchor, door, WALL_GAP);
				vector p = byDoor + side * DOOR_SIDE;
				if (RZ_DoorGeometry.InBuilding(anchor, p))
				{
					side = side * -1;
					p = byDoor + side * DOOR_SIDE;
				}

				result.m_vPos = RZ_Util.GroundPos(p);
				result.m_fYaw = YawAlong(n);
				result.m_sHow = "by a door";

				// The dressing further along the wall, past the control; else past the door's other side
				DressAlong(byDoor, side, n, DOOR_SIDE + DRESS_SIDE, DOOR_SIDE + DRESS_DOOR, anchor, result);

				return;
			}

			// No door to stand by: against the wall nearest the town
			vector wall = RZ_DoorGeometry.SidePoint(anchor, toward, WALL_GAP);
			vector outward = wall - anchor.GetOrigin();
			result.m_vPos = RZ_Util.GroundPos(wall);
			result.m_fYaw = YawAlong(outward);
			result.m_sHow = "against a wall";
			DressBeside(wall, Flat(outward, toward - anchor.GetOrigin()), anchor, result);
			return;
		}

		if (anchor)
		{
			// A transformer or a yard's box on its own: the control beside it, toward the town
			vector away = toward - anchor.GetOrigin();
			away[1] = 0;
			float len = away.Length();
			if (len < 0.1)
				away = Vector(0, 0, 1);
			else
				away = away * (1 / len);

			result.m_vPos = RZ_Util.GroundPos(anchor.GetOrigin() + away * BESIDE);
			result.m_fYaw = YawAlong(away);
			result.m_sHow = "beside it";
			DressBeside(result.m_vPos, away, anchor, result);
			return;
		}

		result.m_vPos = RZ_Util.GroundPos(site);
		result.m_fYaw = RZ_Compass.Bearing(site, toward);
		result.m_sHow = "as placed";
		DressBeside(result.m_vPos, Flat(toward - site, Vector(0, 0, 1)), null, result);
	}

	// ================================================================ The dressing and the generators
	//------------------------------------------------------------------------------------------------
	//! `dir` flattened to length 1; `fallback` (then north) when it's too short to tell.
	static vector Flat(vector dir, vector fallback)
	{
		dir[1] = 0;
		float len = dir.Length();
		if (len < 0.1)
		{
			dir = fallback;
			dir[1] = 0;
			len = dir.Length();
		}

		if (len < 0.1)
			return Vector(0, 0, 1);

		return dir * (1 / len);
	}

	//------------------------------------------------------------------------------------------------
	//! The dressing DRESS_SIDE m to one side of `from` (the control) or the other, facing `face`.
	static void DressBeside(vector from, vector face, IEntity anchor, RZ_PowerPlacement result)
	{
		DressAlong(from, Vector(face[2], 0, -face[0]), face, DRESS_SIDE, DRESS_SIDE, anchor, result);
	}

	//------------------------------------------------------------------------------------------------
	//! The first spot that will do: `first` m along the wall from `from`, else `second` m the other
	//! way, else each DRESS_MORE further; then the same four a step out from the wall, and two steps.
	//! With a building in the way of all twelve (a control in a yard between houses): the nearest
	//! clear spot of three rings round the control, facing it.
	protected static void DressAlong(vector from, vector along, vector face, float first, float second, IEntity anchor, RZ_PowerPlacement result)
	{
		for (int step = 0; step < 3; step++)
		{
			vector line = from + face * (step * DRESS_OUT);
			for (int more = 0; more < 2; more++)
			{
				float further = more * DRESS_MORE;
				if (TryDress(line + along * (first + further), face, anchor, result) || TryDress(line - along * (second + further), face, anchor, result))
					return;
			}
		}

		for (int ring = 1; ring <= 3; ring++)
		{
			for (int k = 0; k < 8; k++)
			{
				vector spot = RZ_Compass.PointAt(from, k * 45, DRESS_SIDE + ring * DRESS_MORE);
				if (TryDress(spot, Flat(from - spot, face), anchor, result))
					return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The dressing at `spot` if it's dry, flat and outside every building near it (and off the middle
	//! of a wall or fence piece, which has no box to stand outside of); its front along `face`.
	protected static bool TryDress(vector spot, vector face, IEntity anchor, RZ_PowerPlacement result)
	{
		vector at = RZ_Util.GroundPos(spot);
		if (RZ_Util.IsWater(at))
			return NoDress(result, "water");

		if (!RZ_DressPlacer.IsFlat(at))
			return NoDress(result, "a slope");

		if (anchor && RZ_DoorGeometry.InBuilding(anchor, at))
			return NoDress(result, "the station itself");

		array<IEntity> near = {};
		RZ_BuildingQuery.FindBuildings(at, 30, near, 32);
		foreach (IEntity b : near)
		{
			if (!b)
				continue;

			vector mins;
			vector maxs;
			if (RZ_DoorGeometry.LocalBox(b, mins, maxs))
			{
				if (RZ_LockedSpots.InFootprint(b, at, 0.1))
					return NoDress(result, RZ_Util.PrettyPrefabName(RZ_Util.GetPrefabName(b)));
			}
			else if (RZ_Util.DistXZ(b.GetOrigin(), at) < DRESS_SMALL)
			{
				return NoDress(result, RZ_Util.PrettyPrefabName(RZ_Util.GetPrefabName(b)));
			}
		}

		result.m_bDress = true;
		result.m_vDressPos = at;
		result.m_fDressYaw = YawAlong(face);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A spot that won't do: what was in the way goes on the list for the log.
	protected static bool NoDress(RZ_PowerPlacement result, string why)
	{
		if (result.m_sDressWhy.Contains(why))
			return false;

		if (!result.m_sDressWhy.IsEmpty())
			result.m_sDressWhy += ", ";

		result.m_sDressWhy += why;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The map's own generator sets within `reach` m of the station (two at most): where each stands
	//! and how it's turned, for the running station's sound. None: the caller plays it at the control.
	static void Generators(vector site, float reach, notnull array<vector> spots, notnull array<float> yaws)
	{
		RZ_PowerSite q = new RZ_PowerSite();
		q.m_iMode = MODE_GENERATOR;
		q.m_vCenter = site;
		q.m_fBestDist = float.MAX;
		q.m_aAll = {};
		GetGame().GetWorld().QueryEntitiesBySphere(site, reach, q.OnEntity, null, EQueryEntitiesFlags.STATIC);
		foreach (IEntity e : q.m_aAll)
		{
			if (!e || spots.Count() >= MAX_GENERATORS)
				continue;

			vector mat[4];
			e.GetWorldTransform(mat);
			spots.Insert(e.GetOrigin());
			yaws.Insert(YawAlong(mat[2]));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The yaw (degrees, as RZ_Util.SpawnPrefab takes it) that turns a prefab's front along `dir`.
	//! VERIFY: a spawned prefab's front (local +Z) points along compass bearing `yaw` (clockwise from
	//! north); World > Power grid > Control yaw turns it if the control prefab's front is another side.
	static float YawAlong(vector dir)
	{
		return RZ_Compass.Bearing(vector.Zero, Vector(dir[0], 0, dir[2]));
	}

	//------------------------------------------------------------------------------------------------
	//! A point just out from the face of a cabinet that looks toward `toward`, at chest height (or
	//! two thirds up a short box). From the box's own transform and bounds.
	static vector FrontOf(IEntity box, vector toward)
	{
		vector mins;
		vector maxs;
		box.GetBounds(mins, maxs); // VERIFY: a vanilla cabinet's local box, and the spot's prompt shows 0.25 m off its face
		vector c = (mins + maxs) * 0.5;
		vector t = RZ_DoorGeometry.ToLocal(box, toward);
		float dx = t[0] - c[0];
		float dz = t[2] - c[2];
		float height = Math.Min(1.2, (maxs[1] - mins[1]) * 0.66);
		vector local = Vector(c[0], mins[1] + height, maxs[2] + SPOT_GAP);
		if (Math.AbsFloat(dx) > Math.AbsFloat(dz))
		{
			if (dx > 0)
				local = Vector(maxs[0] + SPOT_GAP, mins[1] + height, c[2]);
			else
				local = Vector(mins[0] - SPOT_GAP, mins[1] + height, c[2]);
		}
		else if (dz < 0)
		{
			local = Vector(c[0], mins[1] + height, mins[2] - SPOT_GAP);
		}

		return RZ_DoorGeometry.ToWorld(box, local);
	}
}
