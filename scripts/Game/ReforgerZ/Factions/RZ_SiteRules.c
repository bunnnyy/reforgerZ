//! Where things belong at a site (docs/SITES.md, "The rules"): each camp-kit piece's zone, its tier
//! and its rules, written by tools/camp_kit/build.py from pieces.py onto the kit (RZ_DressingPiece:
//! Tier, Zone, Rules) and read here once. The solver (RZ_SiteSolver) places every piece at the
//! spot that meets its hard rules with the best score for its soft ones. Server only.

//------------------------------------------------------------------------------------------------
//! The parts of a site a piece belongs to (docs/SITES.md, "Zones").
enum RZ_ESiteZone
{
	YARD,		// anywhere inside the wire (the default)
	WIRE,		// on the wire line: barricades
	GATE,		// at the gate: the barrier, the sign, the toll
	KILL,		// 10-40 m outside the wire on the threat sides: nothing stands there
	COURT,		// the inner court: the yard nearest the main door
	STORES,		// against the building's wall, furthest from the wire
	WORK,		// the yard within 15 m of the building
	KITCHEN,	// by the building's back or side door
	SLEEP,		// the yard's quietest corner: furthest from the gate
	SICK,		// the far corner, 8 m or more from the gate
	DUMP,		// outside the wire on the quietest side, away from the front
	DEAD		// outside the wire on a quiet side
}

//------------------------------------------------------------------------------------------------
//! Which layer a kit's extra piece belongs to (docs/SITES.md, "Building in layers"). The plan's own
//! pieces are PLAN; the rest are the faction's doctrine, on the kit's Layers list.
enum RZ_ESiteLayer
{
	PLAN,		// a piece of the plan: the gate, the barricades, the watch, work and life
	WIRE,		// a run of the faction's barrier along the wire line (close it off)
	WINDOW,		// a board over a ground-floor window (harden)
	DOOR,		// a side door blocked (harden)
	LOOKOUT,	// a watch toward a threatened side (watch)
	LIGHT,		// a light on the gate and the corners (watch)
	STORES,		// the stores board by the entrance, once valuables went indoors (work and life)
	STORY,		// a spot kept for a story element (docs/SITES.md, "The story of each site"): data until shown
	BLOCK		// a road block: the faction's barrier across a road in that isn't its one gate (close it off)
}

//------------------------------------------------------------------------------------------------
//! How much of the wire line a kind of site closes (each faction's doctrine).
enum RZ_ESiteClose
{
	NONE,		// no wire (a road post: its plan is its barrier)
	ROAD,		// the side the road comes in on
	TWO,		// that side and the most threatened other one
	ALL			// all round
}

//------------------------------------------------------------------------------------------------
//! When a dense kit's extra stands (docs/SITES.md, "What the state changes in the yard"): always, or
//! only while the site's state (RZ_SiteFacts, the story's own) says so. Worked out with the story,
//! each season day.
enum RZ_ESiteWhen
{
	ALWAYS,
	SUPPLIED,	// the faction is thriving or holding on: its stores, fuel and firewood are there
	THRIVING,	// only while it thrives: what it has more of than it needs
	HIT			// attacked, burned or mourning in the last days, or under pressure: defences going up
}

//------------------------------------------------------------------------------------------------
//! How hard a site's building is made.
enum RZ_ESiteHarden
{
	NONE,
	DOORS,		// one entrance: every other outside door blocked
	THREAT,		// and the ground-floor windows on the threatened sides boarded
	ALL			// and every ground-floor window boarded
}

//------------------------------------------------------------------------------------------------
//! One rule of a piece.
class RZ_SiteRule
{
	int m_iType;
	string m_sWhat;		// a piece's name (or several, "A|B"), "use:Cook", a door's kind, a building's kind, a target
	float m_fDist;
}

//------------------------------------------------------------------------------------------------
//! A piece's rules, read from its Rules text ("zone" and "tier" are their own fields). Rules, ";"
//! between them:
//!   near <what> <m>       soft: within m of the nearest placed piece of that kind
//!   away <what> <m>       hard: at least m from it (also: water, gate, wire, door, road)
//!   door <front|side|back|any> <m>   soft: within m of the building's door of that kind
//!   wall <m>              soft: its back within m of the building's wall
//!   window <m>, corner <m> soft: within m of a ground-floor window, a corner of the building
//!   kind <building kind> <m>          soft: within m of a building of that kind (garage, farm, church, well...)
//!   over <what>           soft: over that piece (within 2 m of it)
//!   covers <gate|road|threat|out>     soft: a clear 2 m-high line to it
//!   behind <threat|building>          soft: on the far side of it from the threat
//!   faces <front|out|in|threat|gate|road|wall|door|<what>>   which way it turns
//!   guarded, high, bank, soil, jitter, noline, indoors, flat <degrees>   (docs/SITES.md, "Rule types")
class RZ_SiteRules
{
	static const int R_NEAR = 1;
	static const int R_AWAY = 2;
	static const int R_DOOR = 3;
	static const int R_WALL = 4;
	static const int R_WINDOW = 5;
	static const int R_CORNER = 6;
	static const int R_KIND = 7;
	static const int R_OVER = 8;
	static const int R_COVERS = 9;
	static const int R_BEHIND = 10;
	static const int R_BY = 11;		// stands by: within reach of what it serves, or it isn't there at all

	ref array<ref RZ_SiteRule> m_aRules = {};
	string m_sFaces;			// "" = the plan's own turn
	bool m_bGuarded;			// the most guarded spot: far from the wire, near the main door
	bool m_bHigh;				// the highest free ground with a view
	bool m_bBank;				// may stand on a bank or steep ground (a nest)
	bool m_bSoil;				// soft ground: off the verges and the building's paving, never on a paved surface
	bool m_bRow;				// set out in a row with the others of its kind, squared to the building
	bool m_bLevel;				// a barrier of stacked or standing things (crates, tyres, a wreck): it sits level or isn't put down, never tilted to a slope
	bool m_bJitter;				// clutter: turned 0-8 degrees and moved 0-0.5 m by the seed
	bool m_bNoLine;				// two of a kind never in a straight line
	bool m_bIndoors;			// valuable or delicate: indoors (as data) at a site with a building
	float m_fFlat = -1;			// the steepest the ground under it may be, degrees (-1 the default, about 12)

	//------------------------------------------------------------------------------------------------
	static RZ_SiteRules Parse(string text)
	{
		RZ_SiteRules rules = new RZ_SiteRules();
		array<string> items = {};
		text.Split(";", items, true);
		foreach (string item : items)
		{
			item.Trim();
			array<string> bits = {};
			item.Split(" ", bits, true);
			if (bits.IsEmpty())
				continue;

			string word = bits[0];
			string what = "";
			float dist = 0;
			if (bits.Count() > 1)
				what = bits[1];

			if (bits.Count() > 2)
				dist = bits[2].ToFloat();
			else if (bits.Count() > 1)
				dist = bits[1].ToFloat();

			switch (word)
			{
				case "near": rules.Add(R_NEAR, what, dist); break;
				case "away": rules.Add(R_AWAY, what, dist); break;
				case "door": rules.Add(R_DOOR, what, dist); break;
				case "wall": rules.Add(R_WALL, "", dist); break;
				case "window": rules.Add(R_WINDOW, "", dist); break;
				case "corner": rules.Add(R_CORNER, "", dist); break;
				case "kind": rules.Add(R_KIND, what, dist); break;
				case "over": rules.Add(R_OVER, what, 2); break;
				case "covers": rules.Add(R_COVERS, what, 0); break;
				case "behind": rules.Add(R_BEHIND, what, 0); break;
				case "faces": rules.m_sFaces = what; break;
				case "guarded": rules.m_bGuarded = true; break;
				case "high": rules.m_bHigh = true; break;
				case "bank": rules.m_bBank = true; break;
				case "soil": rules.m_bSoil = true; break;
				case "by": rules.Add(R_BY, what, dist); break;
				case "row": rules.m_bRow = true; break;
				case "level": rules.m_bLevel = true; break;
				case "jitter": rules.m_bJitter = true; break;
				case "noline": rules.m_bNoLine = true; break;
				case "indoors": rules.m_bIndoors = true; break;
				case "flat": rules.m_fFlat = dist; break;
			}
		}

		return rules;
	}

	//------------------------------------------------------------------------------------------------
	protected void Add(int type, string what, float dist)
	{
		RZ_SiteRule rule = new RZ_SiteRule();
		rule.m_iType = type;
		rule.m_sWhat = what;
		rule.m_fDist = dist;
		m_aRules.Insert(rule);
	}

	//------------------------------------------------------------------------------------------------
	//! Has a rule of this type (covers: with this target, "" any).
	bool Has(int type, string what = "")
	{
		foreach (RZ_SiteRule rule : m_aRules)
		{
			if (rule.m_iType == type && (what.IsEmpty() || rule.m_sWhat == what))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! "Latrine", "use:Cook" or "FirePit|FireBarrel" names this piece.
	static bool Names(string what, string name, RZ_EDressUse use)
	{
		if (what.IsEmpty())
			return false;

		if (what.IndexOf("use:") == 0)
		{
			string want = what.Substring(4, what.Length() - 4);
			want.ToUpper();
			return typename.EnumToString(RZ_EDressUse, use) == want;
		}

		array<string> names = {};
		what.Split("|", names, true);
		return names.Contains(name);
	}

	//------------------------------------------------------------------------------------------------
	//! A rule as the log gives it: "Away from: Latrine 15 m".
	static string Text(RZ_SiteRule rule)
	{
		string what = rule.m_sWhat;
		what.Replace("|", " or ");
		what.Replace("use:", "");
		switch (rule.m_iType)
		{
			case R_AWAY: return string.Format("Away from: %1 %2 m", what, rule.m_fDist);
			case R_NEAR: return string.Format("Near: %1 %2 m", what, rule.m_fDist);
			case R_BY: return string.Format("Stands by: %1 %2 m", what, rule.m_fDist);
		}

		return what;
	}
}
