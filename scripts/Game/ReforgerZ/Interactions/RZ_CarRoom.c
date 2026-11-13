//! Room for a parked car (docs/DESIGN.md, "Cars"): a fixable car never stands in a building, in a
//! wall, under a roof or half up a step. RZ_Cars asks when it picks a car's spot and again the
//! first time the car is put out near a player, when everything around it is surely there; a car
//! with no room is moved to the nearest spot that has some (a save's cars too).
//!
//! The check is the car's box on the ground, a little larger than the car and long enough ahead
//! for the look-over spot at its grille: dry and level under its corners, nothing across its sides
//! and diagonals at bumper and window height (the flat traces camp life's pieces use), and nothing
//! overhead.

//------------------------------------------------------------------------------------------------
class RZ_CarRoom
{
	protected static const float HALF_WIDTH = 1.25;		// a saloon is about 1.7 m wide
	protected static const float BEHIND = 2.5;			// and about 4.2 m long
	protected static const float AHEAD = 3.3;			// with the look-over spot 0.8 m past the grille
	protected static const float STEP = 0.9;			// the ground under its corners differs by no more than this (m): a bank or a terrace edge is no place to park
	protected static const float ROOF = 7;				// something this close overhead is a roof, a porch or a bridge deck
	protected static const int HEADINGS = 4;			// a spot is tried at this many headings, 45 degrees apart

	//------------------------------------------------------------------------------------------------
	//! A car at pos facing yaw (degrees clockwise from north) stands free.
	static bool Fits(vector pos, float yaw)
	{
		array<vector> quad = {};
		RZ_CampLife.QuadOf(pos, yaw, Vector(-HALF_WIDTH, 0, -BEHIND), Vector(HALF_WIDTH, 0, AHEAD), 0, quad);
		array<vector> ground = {};
		float low = 100000;
		float high = -100000;
		foreach (vector corner : quad)
		{
			vector g = RZ_Util.GroundPos(corner);
			if (RZ_Util.IsWater(g))
				return false;

			ground.Insert(g);
			low = Math.Min(low, g[1]);
			high = Math.Max(high, g[1]);
		}

		if (high - low > STEP)
			return false;

		// Walls, fences, trees, steps, other cars: across its sides and both diagonals, twice
		for (int h = 0; h < 2; h++)
		{
			vector up = Vector(0, 0.45 + h * 0.75, 0);
			for (int i = 0; i < 4; i++)
			{
				if (Hits(ground[i] + up, ground[(i + 1) % 4] + up))
					return false;
			}

			if (Hits(ground[0] + up, ground[2] + up) || Hits(ground[1] + up, ground[3] + up))
				return false;
		}

		// Indoors, under a porch, a shelter or a bridge
		vector mid = RZ_Util.GroundPos(pos);
		return !Hits(mid + Vector(0, 1.8, 0), mid + Vector(0, ROOF, 0));
	}

	//------------------------------------------------------------------------------------------------
	//! A spot with room: `near` itself at its own heading first (when `keep`), then up to `tries`
	//! land points within maxR of it, each at a few headings. False if none was found: pos and yaw
	//! are left as they were.
	static bool Find(vector near, float maxR, int tries, bool keep, inout vector pos, inout float yaw)
	{
		if (keep && Fits(near, yaw))
		{
			pos = RZ_Util.GroundPos(near);
			return true;
		}

		for (int t = 0; t < tries; t++)
		{
			// Close by first, then wider
			float reach = Math.Max(6, maxR * (t + 1) / tries);
			vector spot = RZ_Mission.LandPointNear(near, 0, reach);
			if (RZ_Perimeter.IsNearLineAt(spot))
				continue;

			float turn = Math.RandomFloat(0, 360);
			for (int k = 0; k < HEADINGS; k++)
			{
				float heading = turn + k * 45;
				if (heading >= 360)
					heading -= 360;

				if (!Fits(spot, heading))
					continue;

				pos = RZ_Util.GroundPos(spot);
				yaw = heading;
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Something solid between the two points (the world's entities; the ground itself isn't asked).
	protected static bool Hits(vector from, vector to)
	{
		TraceParam param = new TraceParam();
		param.Start = from;
		param.End = to;
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		param.Flags = TraceFlags.ENTS;
		float frac = ChimeraCharacter.TraceMoveWithoutCharacters(GetGame().GetWorld(), param); // VERIFY: a trace that starts inside a building's wall or under its floor slab reports a hit (under 1), so a car's box inside a house is refused; and a trace straight up from 1.8 m stops on a roof, a porch and a bridge deck
		return frac < 0.999;
	}
}
