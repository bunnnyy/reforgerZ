//! A recognisable building or prop in a town: a church, a water tower, a well.
class RZ_Landmark
{
	vector m_vPos;
	string m_sKind;		// "church", "watertower", "well"...
	string m_sName;		// "the church", "the water tower"
	bool m_bSpawned;	// true = a fallback prop the mission placed
}

//! Finds real landmark buildings near a point by prefab (or mesh) name keywords,
//! so puzzles can say "from the church, face the water tower" on any map.
//! Runs once per mission setup, over a small sphere of static entities. Locked buildings
//! (RZ_LockedSpots) are skipped, so nothing a mission places at a landmark is behind a lock.
class RZ_LandmarkFinder
{
	protected ref array<ref RZ_Landmark> m_aFound;
	protected ref array<string> m_aKinds;
	protected float m_fMinSpacing = 25;

	//------------------------------------------------------------------------------------------------
	//! Kinds: church, watertower, tower, castle, lighthouse, school, station, fuel, well, bank, monument,
	//! power, industrial, radio, pharmacy, police, military.
	static void Find(vector center, float radius, array<string> kinds, notnull array<ref RZ_Landmark> result)
	{
		RZ_LandmarkFinder finder = new RZ_LandmarkFinder();
		finder.Run(center, radius, kinds, result);
	}

	//------------------------------------------------------------------------------------------------
	//! Nearest landmark of any of the kinds, or null.
	static RZ_Landmark FindNearest(vector center, float radius, array<string> kinds)
	{
		array<ref RZ_Landmark> found = {};
		Find(center, radius, kinds, found);
		RZ_Landmark best;
		float bestDist = float.MAX;
		foreach (RZ_Landmark l : found)
		{
			float d = RZ_Util.DistXZ(center, l.m_vPos);
			if (d < bestDist)
			{
				bestDist = d;
				best = l;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	protected void Run(vector center, float radius, array<string> kinds, notnull array<ref RZ_Landmark> result)
	{
		m_aFound = result;
		m_aKinds = kinds;
		BaseWorld world = GetGame().GetWorld();
		world.QueryEntitiesBySphere(center, radius, OnEntity, null, EQueryEntitiesFlags.STATIC);
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnEntity(IEntity e)
	{
		// A camp's scenery (its generator, antenna, water barrel) is never a mission's landmark
		string name = RZ_Util.GetPrefabName(e);
		if (name.IsEmpty() || RZ_CampLife.IsDressing(e))
			return true;

		name.ToLower();
		foreach (string kind : m_aKinds)
		{
			if (!Matches(kind, name))
				continue;

			// Never a locked building: whatever a mission puts there would be behind its door
			if (RZ_LockedSpots.IsLockedBuilding(e))
				return true;

			vector pos = e.GetOrigin();
			foreach (RZ_Landmark existing : m_aFound)
			{
				if (existing.m_sKind == kind && RZ_Util.DistXZ(existing.m_vPos, pos) < m_fMinSpacing)
					return true;
			}

			RZ_Landmark l = new RZ_Landmark();
			l.m_vPos = pos;
			l.m_sKind = kind;
			l.m_sName = DisplayName(kind, name);
			m_aFound.Insert(l);
			return true;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Keyword rules per kind. Lower-case prefab or mesh path.
	static bool Matches(string kind, string name)
	{
		switch (kind)
		{
			case "church": return name.Contains("church") || name.Contains("chapel");
			case "watertower": return name.Contains("watertower") || name.Contains("water_tower");
			case "tower": return name.Contains("tower") && !name.Contains("watertower") && !name.Contains("water_tower") && !name.Contains("power");
			case "castle": return name.Contains("castle");
			case "lighthouse": return name.Contains("lighthouse");
			case "school": return name.Contains("school");
			case "station": return name.Contains("station") && !name.Contains("gasstation") && !name.Contains("fuelstation");
			case "fuel": return name.Contains("gasstation") || name.Contains("fuelstation") || name.Contains("petrol");
			case "well": return name.Contains("waterwell") || name.Contains("/well") || name.Contains("_well") || name.Contains("well_0");
			case "bank": return (name.Contains("/bank") || name.Contains("bank_")) && !name.Contains("embank");
			case "monument": return name.Contains("monument") || name.Contains("memorial") || name.Contains("statue");
			case "power": return name.Contains("powerplant") || name.Contains("power_plant") || name.Contains("powerstation") || name.Contains("power_station") || name.Contains("substation") || name.Contains("transformer");
			case "industrial": return name.Contains("factory") || name.Contains("industrial") || name.Contains("warehouse") || name.Contains("sawmill");
			case "radio": return name.Contains("radio") || name.Contains("antenna") || name.Contains("transmitter") || name.Contains("telecom");
			case "pharmacy": return name.Contains("pharmacy") || name.Contains("hospital") || name.Contains("clinic") || name.Contains("medical");
			case "police": return name.Contains("police") || name.Contains("gendarm") || name.Contains("sheriff");
			case "military": return name.Contains("military") || name.Contains("barrack") || name.Contains("bunker") || name.Contains("armory") || name.Contains("armoury");
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	static string DisplayName(string kind, string name)
	{
		if (kind == "church" && name.Contains("chapel"))
			return "the chapel";

		switch (kind)
		{
			case "church": return "the church";
			case "watertower": return "the water tower";
			case "tower": return "the tower";
			case "castle": return "the castle";
			case "lighthouse": return "the lighthouse";
			case "school": return "the school";
			case "station": return "the station";
			case "fuel": return "the petrol station";
			case "well": return "the well";
			case "bank": return "the bank";
			case "monument": return "the monument";
			case "power": return "the power station";
			case "industrial": return "the factory";
			case "radio": return "the radio mast";
			case "pharmacy": return "the pharmacy";
			case "police": return "the police station";
			case "military": return "the barracks";
		}

		return "the " + kind;
	}
}

//! Compass helpers. Enfusion: +X is east, +Z is north. Bearings are degrees
//! clockwise from north, like a real compass.
class RZ_Compass
{
	static const float METRES_PER_PACE = 0.75;

	//------------------------------------------------------------------------------------------------
	static float Normalize(float deg)
	{
		while (deg < 0)
		{
			deg += 360;
		}

		while (deg >= 360)
		{
			deg -= 360;
		}

		return deg;
	}

	//------------------------------------------------------------------------------------------------
	static float Bearing(vector from, vector to)
	{
		float dx = to[0] - from[0];
		float dz = to[2] - from[2];
		return Normalize(Math.Atan2(dx, dz) * Math.RAD2DEG);
	}

	//------------------------------------------------------------------------------------------------
	static vector PointAt(vector from, float bearingDeg, float dist)
	{
		float rad = bearingDeg * Math.DEG2RAD;
		vector p = from;
		p[0] = p[0] + Math.Sin(rad) * dist;
		p[2] = p[2] + Math.Cos(rad) * dist;
		return RZ_Util.GroundPos(p);
	}

	//------------------------------------------------------------------------------------------------
	static int Paces(float metres)
	{
		return RZ_Util.RoundToInt(metres / METRES_PER_PACE);
	}

	//------------------------------------------------------------------------------------------------
	static float PacesToMetres(int paces)
	{
		return paces * METRES_PER_PACE;
	}

	//------------------------------------------------------------------------------------------------
	//! "290°" (whole degrees, 360 shown as 0).
	static string BearingText(float deg)
	{
		int d = RZ_Util.RoundToInt(Normalize(deg)) % 360;
		return RZ_NoteText.Pad(d, 3) + "°";
	}

	//------------------------------------------------------------------------------------------------
	//! "north", "south-west"... for notes.
	static string DirectionWord(float deg)
	{
		array<string> names = {"north", "north-east", "east", "south-east", "south", "south-west", "west", "north-west"};
		int idx = RZ_Util.RoundToInt(Normalize(deg) / 45) % 8;
		return names[idx];
	}
}
