//! Role spots in a hand-built site (docs/SITES.md, "Role spots"): where its people sit, stand, work,
//! keep watch, cook, wash, chop and sleep, where the canteen, the medic's tent, the food tent and
//! the latrine are, where the quartermaster and the leader stand, and the points of its walks. The
//! author puts them down with the site builder; camp life's daily routine runs on them as it runs on
//! its own pieces' smart actions (RZ_CampLife.LayHandBuilt). A site's needs (the checklist) are worked
//! out from its faction and kind (Needs), the same as the routine table in docs/DESIGN.md asks for.
//!
//! A save's spot line: spot <type> x y z yaw [seats].

//------------------------------------------------------------------------------------------------
//! One spot, as data.
class RZ_HandSpot
{
	string m_sType;
	vector m_vPos;
	float m_fYaw;
	int m_iSeats = 1;
}

//------------------------------------------------------------------------------------------------
//! One line of a site's checklist.
class RZ_HandNeed
{
	string m_sType;
	int m_iWant;
	int m_iHave;
	bool m_bRequired;
}

//------------------------------------------------------------------------------------------------
class RZ_HandSpots
{
	static const int MEAL_SEATS = 8;			// the canteen table's prefab seats eight round it
	static const float TOO_CLOSE = 0.8;			// two spots nearer than this are on top of each other
	static const float GATE_LANE = 1.5;			// a spot this near the line from the gate to the middle is in the way in
	protected static const float WALL_PROBE = 0.35;	// a spot is in a wall or a prop when this close round it is solid

	//------------------------------------------------------------------------------------------------
	//! The types in the order the builder cycles through them.
	static void Types(notnull array<string> result)
	{
		result.Insert("sit");
		result.Insert("meal");
		result.Insert("lean");
		result.Insert("work");
		result.Insert("water");
		result.Insert("watch");
		result.Insert("gate");
		result.Insert("cook");
		result.Insert("wash");
		result.Insert("chop");
		result.Insert("sleep");
		result.Insert("medic");
		result.Insert("stores");
		result.Insert("latrine");
		result.Insert("qm");
		result.Insert("leader");
		result.Insert("route");
	}

	//------------------------------------------------------------------------------------------------
	static bool IsKnown(string type)
	{
		array<string> all = {};
		Types(all);
		return all.Contains(type);
	}

	//------------------------------------------------------------------------------------------------
	//! The type's name in the menu and the checklist.
	static string Label(string type)
	{
		switch (type)
		{
			case "sit": return "Seat";
			case "meal": return "Canteen table (8 seats)";
			case "lean": return "Lean spot";
			case "work": return "Work spot";
			case "water": return "Water";
			case "watch": return "Watch post";
			case "gate": return "Gate";
			case "cook": return "Cook spot";
			case "wash": return "Wash spot";
			case "chop": return "Chopping block";
			case "sleep": return "Sleeping place";
			case "medic": return "Medic's tent";
			case "stores": return "Food tent / stores door";
			case "latrine": return "Latrine";
			case "qm": return "Quartermaster's place";
			case "leader": return "Leader's place";
			case "route": return "Walk point";
		}

		return type;
	}

	//------------------------------------------------------------------------------------------------
	//! One sentence on what the spot is for (the build bar says it when the mouse is on its button).
	static string Help(string type)
	{
		switch (type)
		{
			case "sit": return "Where someone sits to rest, smoke or talk.";
			case "meal": return "The canteen table: eight people eat here at mealtimes.";
			case "lean": return "Where someone leans on a wall and passes the time.";
			case "work": return "Where someone works with their hands: mending, sorting, cleaning kit.";
			case "water": return "Where people fill canteens and drink.";
			case "watch": return "A lookout who watches outward, toward the threats.";
			case "gate": return "The guard who stands at the way in and stops people.";
			case "cook": return "Where the cook works the stove or the pot.";
			case "wash": return "Where people wash hands, faces and clothes.";
			case "chop": return "Where someone splits wood for the fires.";
			case "sleep": return "Where someone sleeps off-shift.";
			case "medic": return "The medic's tent: the hurt and the sick go here.";
			case "stores": return "The food tent or stores door, where supplies go in and out.";
			case "latrine": return "Where people go to relieve themselves.";
			case "qm": return "Where the quartermaster stands to trade with you.";
			case "leader": return "Where the camp's leader spends the day.";
			case "route": return "A point on a patrol: the guards walk the points in order.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! What camp life's routine does there (RZ_EDressUse): the use its piece takes.
	static RZ_EDressUse UseOf(string type)
	{
		switch (type)
		{
			case "sit": return RZ_EDressUse.SIT;
			case "meal": return RZ_EDressUse.SIT;
			case "lean": return RZ_EDressUse.LEAN;
			case "work": return RZ_EDressUse.WORK;
			case "water": return RZ_EDressUse.WORK;
			case "watch": return RZ_EDressUse.WATCH;
			case "gate": return RZ_EDressUse.WATCH;
			case "cook": return RZ_EDressUse.COOK;
			case "wash": return RZ_EDressUse.WASH;
			case "chop": return RZ_EDressUse.CHOP;
			case "sleep": return RZ_EDressUse.SLEEP;
		}

		return RZ_EDressUse.NONE;
	}

	//------------------------------------------------------------------------------------------------
	//! Whether the spot stands in the world as a piece (an invisible one carrying its smart action, or
	//! a point the routine finds by name). The quartermaster's, the leader's, the medic's, the stores'
	//! and the walks' spots are read as positions only.
	static bool IsPiece(string type)
	{
		return UseOf(type) != RZ_EDressUse.NONE || type == "latrine";
	}

	//------------------------------------------------------------------------------------------------
	//! The piece's name: the routine finds the water and the latrine by it (RZ_CampLife.AtNamed).
	static string NameOf(string type)
	{
		if (type == "latrine")
			return "HandSpot_Latrine";

		return "HandSpot_" + type;
	}

	//------------------------------------------------------------------------------------------------
	static int SeatsOf(RZ_HandSpot s)
	{
		if (s.m_sType == "meal")
			return Math.ClampInt(s.m_iSeats, 1, MEAL_SEATS);

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! How many of a type the spots give: seats count the canteen's too.
	static int Count(notnull array<ref RZ_HandSpot> spots, string type)
	{
		int n = 0;
		foreach (RZ_HandSpot s : spots)
		{
			if (s.m_sType == type)
				n++;
			else if (type == "sit" && s.m_sType == "meal")
				n += SeatsOf(s);
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! The first spot of a type, or null.
	static RZ_HandSpot First(array<ref RZ_HandSpot> spots, string type)
	{
		if (!spots)
			return null;

		foreach (RZ_HandSpot s : spots)
		{
			if (s.m_sType == type)
				return s;
		}

		return null;
	}

	// ================================================================ The checklist
	//------------------------------------------------------------------------------------------------
	//! What a site of this faction and kind needs (docs/SITES.md, "What a site needs"): the routine's
	//! table in docs/DESIGN.md turned into spots. Required ones count as missing; the rest are nice.
	static void Needs(RZ_EFaction f, RZ_ECampPlace kind, notnull array<ref RZ_HandNeed> result)
	{
		if (kind == RZ_ECampPlace.ROAD_POST)
		{
			Need(result, "gate", 1, true);
			Need(result, "lean", 2, false);
			Need(result, "sit", 2, false);
			return;
		}

		bool hold = kind == RZ_ECampPlace.STRONGHOLD;
		bool safe = kind == RZ_ECampPlace.SAFEHOUSE;
		int seats = 3;
		int beds = 1;
		if (hold)
		{
			seats = 4;
			beds = 2;
		}

		bool haven = f == RZ_EFaction.SURVIVORS;
		bool bastion = f == RZ_EFaction.MILITARY;
		bool red = f == RZ_EFaction.BANDITS;
		bool halcyon = f == RZ_EFaction.INSTITUTE;

		Need(result, "gate", 1, true);
		Need(result, "watch", 1, bastion || (hold && !haven));
		Need(result, "sit", seats, true);
		int work = 1;
		if (halcyon)
			work = 2;

		Need(result, "work", work, true);
		Need(result, "sleep", beds, true);
		Need(result, "chop", 1, bastion || red || haven);
		Need(result, "cook", 1, haven);
		Need(result, "meal", 1, haven);
		Need(result, "water", 1, haven);
		Need(result, "wash", 1, halcyon);
		Need(result, "route", 2, red || halcyon);
		Need(result, "lean", 1, red);
		Need(result, "qm", 1, hold);
		Need(result, "leader", 1, kind == RZ_ECampPlace.OUTPOST);
		Need(result, "medic", 1, false);
		Need(result, "stores", 1, false);
		Need(result, "latrine", 1, false);
		if (safe)
		{
			// A safehouse's lookout is the gate's chair or the door: its watch post is the gate
			foreach (RZ_HandNeed n : result)
			{
				if (n.m_sType == "watch")
					n.m_bRequired = false;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void Need(notnull array<ref RZ_HandNeed> result, string type, int want, bool required)
	{
		RZ_HandNeed n = new RZ_HandNeed();
		n.m_sType = type;
		n.m_iWant = want;
		n.m_bRequired = required;
		result.Insert(n);
	}

	//------------------------------------------------------------------------------------------------
	//! The checklist with what's there filled in; `missing`: required needs not met.
	static void Check(RZ_EFaction f, RZ_ECampPlace kind, array<ref RZ_HandSpot> spots, notnull array<ref RZ_HandNeed> result, out int missing)
	{
		missing = 0;
		Needs(f, kind, result);
		foreach (RZ_HandNeed n : result)
		{
			if (spots)
				n.m_iHave = Count(spots, n.m_sType);

			if (n.m_bRequired && n.m_iHave < n.m_iWant)
				missing++;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The checklist as text: required lines, then the nice ones. One line each, for the menu.
	static string Text(RZ_EFaction f, RZ_ECampPlace kind, array<ref RZ_HandSpot> spots, out int missing)
	{
		array<ref RZ_HandNeed> needs = {};
		Check(f, kind, spots, needs, missing);
		string req = "";
		string nice = "";
		foreach (RZ_HandNeed n : needs)
		{
			string state = "OK";
			if (n.m_iHave < n.m_iWant)
			{
				state = "MISSING";
				if (!n.m_bRequired)
					state = "none yet";
			}

			string line = string.Format("%1 %2 of %3 %4", Label(n.m_sType), Math.Min(n.m_iHave, 99), n.m_iWant, state);
			if (n.m_bRequired)
			{
				if (!req.IsEmpty())
					req += " · ";

				req += line;
			}
			else
			{
				if (!nice.IsEmpty())
					nice += " · ";

				nice += line;
			}
		}

		string text = "NEEDS: " + req;
		if (!nice.IsEmpty())
			text += "\nNICE TO HAVE: " + nice;

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! The checklist in one short line for the log and the list: "complete" or "3 missing (Gate, ...)".
	static string Short(RZ_EFaction f, RZ_ECampPlace kind, array<ref RZ_HandSpot> spots, out int missing)
	{
		array<ref RZ_HandNeed> needs = {};
		Check(f, kind, spots, needs, missing);
		if (missing == 0)
			return "complete";

		string names = "";
		foreach (RZ_HandNeed n : needs)
		{
			if (!n.m_bRequired || n.m_iHave >= n.m_iWant)
				continue;

			if (!names.IsEmpty())
				names += ", ";

			names += Label(n.m_sType);
		}

		return string.Format("%1 missing (%2)", missing, names);
	}

	// ================================================================ Is a spot right where it is
	//------------------------------------------------------------------------------------------------
	//! What's wrong with the spot where it stands ("" nothing): outside the site, in a wall or a prop,
	//! on top of another spot, or in the way in from the gate. `self` is left out of the wall check
	//! (its marker).
	static string Problem(RZ_HandSpot s, vector centre, float radius, array<ref RZ_HandSpot> all, IEntity self)
	{
		if (RZ_Util.DistXZ(s.m_vPos, centre) > radius)
			return "outside the site";

		if (s.m_sType != "route" && Blocked(s.m_vPos, self))
			return "in a wall or a prop";

		RZ_HandSpot gate = First(all, "gate");
		foreach (RZ_HandSpot o : all)
		{
			if (o == s || o.m_sType == "route" || s.m_sType == "route")
				continue;

			if (RZ_Util.DistXZ(o.m_vPos, s.m_vPos) < TOO_CLOSE)
				return "on top of another spot";
		}

		if (gate && gate != s && s.m_sType != "gate" && s.m_sType != "route" && s.m_sType != "watch" && s.m_sType != "lean")
		{
			if (DistToSegment(s.m_vPos, gate.m_vPos, centre) < GATE_LANE)
				return "in the way in from the gate";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Something solid across the spot at knee and chest height (a wall, a crate, a tent's side).
	protected static bool Blocked(vector at, IEntity self)
	{
		vector g = OnFloor(at);
		for (int h = 0; h < 2; h++)
		{
			vector up = Vector(0, 0.5 + h * 0.7, 0);
			if (Hits(g + up + Vector(-WALL_PROBE, 0, 0), g + up + Vector(WALL_PROBE, 0, 0), self))
				return true;

			if (Hits(g + up + Vector(0, 0, -WALL_PROBE), g + up + Vector(0, 0, WALL_PROBE), self))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The spot's own height when it stands on a floor (indoors, a porch), else the ground's.
	static vector OnFloor(vector at)
	{
		vector g = RZ_Util.GroundPos(at);
		if (at[1] > g[1])
			return at;

		return g;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Hits(vector from, vector to, IEntity self)
	{
		TraceParam param = new TraceParam();
		param.Start = from;
		param.End = to;
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		param.Flags = TraceFlags.ENTS;
		param.Exclude = self;
		float frac = ChimeraCharacter.TraceMoveWithoutCharacters(GetGame().GetWorld(), param); // VERIFY: a short flat trace through a spot inside a wall, a crate or a tent's side reports a hit, and the excluded marker cone doesn't
		return frac < 0.999;
	}

	//------------------------------------------------------------------------------------------------
	protected static float DistToSegment(vector p, vector a, vector b)
	{
		float ax = b[0] - a[0];
		float az = b[2] - a[2];
		float len = ax * ax + az * az;
		if (len < 0.01)
			return RZ_Util.DistXZ(p, a);

		float t = ((p[0] - a[0]) * ax + (p[2] - a[2]) * az) / len;
		t = Math.Clamp(t, 0, 1);
		vector on = Vector(a[0] + ax * t, p[1], a[2] + az * t);
		return RZ_Util.DistXZ(p, on);
	}

	// ================================================================ From the automated layout
	//------------------------------------------------------------------------------------------------
	//! The spot type a piece of camp life's own layout stands for ("" none): the builder suggests a
	//! missing spot where the automated layout had one.
	static string TypeOfPiece(RZ_EDressUse use, string name)
	{
		if (name.Contains("Latrine"))
			return "latrine";

		if (name.Contains("Water"))
			return "water";

		switch (use)
		{
			case RZ_EDressUse.SIT: return "sit";
			case RZ_EDressUse.WORK: return "work";
			case RZ_EDressUse.WATCH: return "watch";
			case RZ_EDressUse.LEAN: return "lean";
			case RZ_EDressUse.COOK: return "cook";
			case RZ_EDressUse.WASH: return "wash";
			case RZ_EDressUse.CHOP: return "chop";
			case RZ_EDressUse.SLEEP: return "sleep";
		}

		return "";
	}
}
