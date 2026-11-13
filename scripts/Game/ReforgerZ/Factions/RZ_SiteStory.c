//! Each site's story (docs/SITES.md, "The story of each site"): a few facts worked out from what the
//! game already knows (how the place was settled, how its faction is doing, what happened there in
//! the last three days, what's out there, the town's infection), one of four stories per faction
//! picked from them (the seed breaks ties), and the story's elements shown through Traces' own
//! machinery: each element is a scene kind (C1-C18, docs/TRACES.md "Sites' stories") whose prefab
//! comes from World > Traces > Kit, aged by thirds as scenes are (its faded twin when old, Kit >
//! Pieces that age), standing on a spot the site's solver kept for it. Worked out again each season
//! day from the same seed and the new state; nothing is saved. Its lines (one when you come in, a
//! quiet headline when a site is hit or mourning) are in the faction's voice (docs/WRITING.md).
//! Server only.

//------------------------------------------------------------------------------------------------
//! A site's state, from real data.
class RZ_SiteFacts
{
	static const int HELD = 0;		// held from the start
	static const int TAKEN = 1;		// taken from another faction
	static const int SETTLED = 2;	// settled an empty town
	static const int RETAKEN = 3;	// lost, then taken back

	static const int THRIVING = 0;
	static const int HOLDING = 1;	// holding on
	static const int SHORT = 2;		// a need
	static const int PRESSURE = 3;	// under pressure

	static const int QUIET = 0;
	static const int HIT = 1;		// attacked: by the infected or by people
	static const int BURNED = 2;	// a building burned at it
	static const int MOURNING = 3;	// its people died

	static const int OUT_HOSTILE = 0;	// the threat front: a hostile neighbour
	static const int OUT_INFECTED = 1;
	static const int OUT_ROAD = 2;
	static const int OUT_SEA = 3;

	static const int CLEAN = 0;		// the town under 30 infection
	static const int FIGHTING = 1;	// fighting it
	static const int LOSING = 2;	// over 60

	int m_iSettled;
	int m_iGoing = HOLDING;
	int m_iLately;
	bool m_bByPeople;
	bool m_bByInfected;
	int m_iOutside = OUT_ROAD;
	int m_iTown;
	int m_iTownIndex = -1;		// the town's index (-1: none)
	int m_iOldOwner = -1;		// taken from (a faction)
	int m_iTakenDay = -1;		// the broadcast day it changed hands
	int m_iLatelyDay = -1;		// the broadcast day of the latest hurt
	int m_iDeaths;				// in the last three days
	bool m_bHostileRedHand;		// a hostile Red Hand holding within 3 km
	bool m_bThrough;			// a road passes through
	bool m_bChurch;
	bool m_bFarm;
	bool m_bLab;
	bool m_bStudy;				// a nest under study within 1.5 km (Specimen from the Nest running)
	string m_sPlace;			// "the school"
	string m_sTown;				// "Levie"

	//------------------------------------------------------------------------------------------------
	string Describe()
	{
		array<string> settled = {"held from the start", "taken from someone", "settled an empty town", "retaken"};
		array<string> going = {"thriving", "holding on", "short", "under pressure"};
		array<string> lately = {"quiet", "hit", "burned", "mourning"};
		array<string> outside = {"a hostile neighbour", "the infected", "the road", "the sea"};
		array<string> town = {"clean", "fighting it", "losing it"};
		string s = settled[m_iSettled];
		if (m_iSettled == TAKEN && m_iOldOwner >= 0)
			s = "taken from " + RZ_Factions.GetNameInText(m_iOldOwner);

		string l = lately[m_iLately];
		if (m_iLately == HIT && m_bByInfected)
			l = "hit by the infected";
		else if (m_iLately == HIT && m_bByPeople)
			l = "hit by people";

		if (m_iDeaths > 0)
			l += string.Format(", %1 dead", m_iDeaths);

		return string.Format("%1, %2, %3, %4 out there, the town %5", s, going[m_iGoing], l, outside[m_iOutside], town[m_iTown]);
	}
}

//------------------------------------------------------------------------------------------------
class RZ_SiteStory
{
	// The library: four stories a faction
	static const int B_HELD = 0;		// Bastion: the post that held
	static const int B_DUG_IN = 1;		// dug in against the crews
	static const int B_SHORT = 2;		// short of everything
	static const int B_CLOSE = 3;		// after the night it came close
	static const int R_TOOK = 4;		// the Red Hand: took it off Bastion
	static const int R_TOLL = 5;		// the toll road
	static const int R_FAT = 6;			// fat and loud
	static const int R_GOT_IN = 7;		// something got in
	static const int H_CHURCH = 8;		// the Haven: the church that opened its doors
	static const int H_FARM = 9;		// the farm that feeds three towns
	static const int H_BURYING = 10;	// burying our own
	static const int H_NIGHT = 11;		// the night watch
	static const int I_CLEAN = 12;		// Halcyon: the clean room
	static const int I_COURIER = 13;	// the station that lost its courier
	static const int I_BURNING = 14;	// burning the samples
	static const int I_STUDY = 15;		// under study
	static const int STORIES = 16;

	// The elements: each a scene kind C1-C18 (kind + 1)
	static const int E_PYRE = 0;		// burnt infected piles at the kill zone's edge
	static const int E_CASINGS = 1;		// casings and a broken crate by the nest
	static const int E_SIGN = 2;		// the faction's mark on the approach (two under pressure)
	static const int E_WRECK = 3;		// cars pushed off the road onto the verge
	static const int E_GRAVES = 4;		// graves on the dead zone, one a death (cap 7)
	static const int E_ROSTER = 5;		// a board by the entrance: the roster with a gap, the count crossed out
	static const int E_SCORCH = 6;		// scorch by the entrance
	static const int E_GATEMARK = 7;	// the mark at the gate (the old owner's beside it while taken)
	static const int E_SANDBAGS = 8;	// sandbags and a HALT sign on its side, toward the hostile side
	static const int E_BOARDED = 9;		// planks and furniture against the wire on the threat side
	static const int E_CHAIR = 10;		// a chair under a sheet
	static const int E_JARS = 11;		// candle jars
	static const int E_CHURCHDOOR = 12;	// the board, a chair and a blanket by the door
	static const int E_BICYCLE = 13;	// the courier's bicycle on its side, a sample case taped shut
	static const int E_BURN = 14;		// ash by the back: the samples burned
	static const int E_TAPE = 15;		// tape between posts at the gate
	static const int E_SPOILS = 16;		// bags and cases stacked by the door
	static const int E_BUCKETS = 17;	// buckets by the kitchen
	static const int ELEMENTS = 18;

	static const int MAX_ELEMENTS = 6;	// at one site at once
	static const int MAX_GRAVES = 7;	// as N1's crosses
	static const int HISTORY_DAYS = 3;	// what happened lately
	static const int TAKEN_DAYS = 3;	// the old owner's mark stays this long

	// Where each element's spot is (RZ_SiteSolver.StorySpots)
	static const int AT_KILL = 0;		// just outside the wire toward the threat front
	static const int AT_HOSTILE = 1;	// just outside the wire toward the hostile base
	static const int AT_APPROACH = 2;	// on the verge outside the gate
	static const int AT_FAR_VERGE = 3;	// further out on the verge
	static const int AT_DEAD = 4;		// the dead zone
	static const int AT_DUMP = 5;		// the dump
	static const int AT_COURT = 6;		// the court by the entrance
	static const int AT_ENTRANCE = 7;	// against the building by the entrance
	static const int AT_GATE = 8;		// just inside the gate
	static const int AT_NEST = 9;		// by the nest or the gate's post
	static const int AT_THREAT_IN = 10;	// inside the wire against the threat side
	static const int AT_KITCHEN = 11;

	// ================================================================ The library
	//------------------------------------------------------------------------------------------------
	static string Name(int story)
	{
		switch (story)
		{
			case B_HELD: return "The post that held";
			case B_DUG_IN: return "Dug in against the crews";
			case B_SHORT: return "Short of everything";
			case B_CLOSE: return "After the night it came close";
			case R_TOOK: return "Took it off Bastion";
			case R_TOLL: return "The toll road";
			case R_FAT: return "Fat and loud";
			case R_GOT_IN: return "Something got in";
			case H_CHURCH: return "The church that opened its doors";
			case H_FARM: return "The farm that feeds three towns";
			case H_BURYING: return "Burying our own";
			case H_NIGHT: return "The night watch";
			case I_CLEAN: return "The clean room";
			case I_COURIER: return "The station that lost its courier";
			case I_BURNING: return "Burning the samples";
			case I_STUDY: return "Under study";
		}

		return "none";
	}

	//------------------------------------------------------------------------------------------------
	//! The faction's first story (its four are this and the next three).
	static int First(RZ_EFaction f)
	{
		switch (f)
		{
			case RZ_EFaction.MILITARY: return B_HELD;
			case RZ_EFaction.BANDITS: return R_TOOK;
			case RZ_EFaction.SURVIVORS: return H_CHURCH;
			case RZ_EFaction.INSTITUTE: return I_CLEAN;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! The story the state picks of the faction's four: the best fit, the seed breaking ties.
	//! Whether an extra that follows the site's state stands today (docs/SITES.md, "What the state
	//! changes in the yard").
	static bool Holds(RZ_ESiteWhen when, RZ_SiteFacts facts)
	{
		switch (when)
		{
			case RZ_ESiteWhen.SUPPLIED: return facts.m_iGoing == RZ_SiteFacts.THRIVING || facts.m_iGoing == RZ_SiteFacts.HOLDING;
			case RZ_ESiteWhen.THRIVING: return facts.m_iGoing == RZ_SiteFacts.THRIVING;
			case RZ_ESiteWhen.HIT: return facts.m_iLately != RZ_SiteFacts.QUIET || facts.m_iGoing == RZ_SiteFacts.PRESSURE;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static int Pick(RZ_EFaction f, RZ_SiteFacts facts, int seed)
	{
		int first = First(f);
		if (first < 0)
			return -1;

		RZ_Rng rng = new RZ_Rng(seed + 211);
		int best = first;
		float bestScore = -1;
		for (int s = first; s < first + 4; s++)
		{
			float score = Fit(s, facts) + rng.Range(0, 0.2);
			if (score > bestScore)
			{
				bestScore = score;
				best = s;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! How well a story fits the state (docs/SITES.md, "The library": its When).
	static float Fit(int story, RZ_SiteFacts f)
	{
		bool hit = f.m_iLately == RZ_SiteFacts.HIT || f.m_iLately == RZ_SiteFacts.BURNED;
		bool mourning = f.m_iLately == RZ_SiteFacts.MOURNING;
		bool thriving = f.m_iGoing == RZ_SiteFacts.THRIVING;
		bool shortOf = f.m_iGoing == RZ_SiteFacts.SHORT || f.m_iGoing == RZ_SiteFacts.PRESSURE;
		switch (story)
		{
			case B_HELD:
			{
				if (f.m_iSettled == RZ_SiteFacts.HELD && thriving)
					return 3;

				return 1;
			}
			case B_DUG_IN:
			{
				if (!f.m_bHostileRedHand)
					return 0;

				if (f.m_iOutside == RZ_SiteFacts.OUT_HOSTILE)
					return 3.5;

				return 2.5;
			}
			case B_SHORT:
			{
				if (f.m_iGoing == RZ_SiteFacts.SHORT)
					return 3;

				if (f.m_iGoing == RZ_SiteFacts.PRESSURE)
					return 2.5;

				return 0;
			}
			case B_CLOSE:
			{
				if (hit && f.m_bByInfected)
					return 4;

				if (hit || mourning)
					return 2;

				return 0;
			}
			case R_TOOK:
			{
				if (f.m_iSettled == RZ_SiteFacts.TAKEN && f.m_iOldOwner == RZ_EFaction.MILITARY)
					return 4;

				if (f.m_iSettled == RZ_SiteFacts.TAKEN || f.m_iSettled == RZ_SiteFacts.RETAKEN)
					return 2;

				return 0;
			}
			case R_TOLL:
			{
				if (f.m_bThrough || f.m_iOutside == RZ_SiteFacts.OUT_ROAD)
					return 2.5;

				return 1;
			}
			case R_FAT:
			{
				if (thriving)
					return 3;

				return 0.8;
			}
			case R_GOT_IN:
			{
				if (hit || mourning)
					return 4;

				return 0;
			}
			case H_CHURCH:
			{
				if (f.m_bChurch)
					return 3;

				return 1;
			}
			case H_FARM:
			{
				if (f.m_bFarm && thriving)
					return 3.5;

				if (f.m_bFarm || thriving)
					return 1.5;

				return 0.5;
			}
			case H_BURYING:
			{
				if (mourning)
					return 4;

				return 0;
			}
			case H_NIGHT:
			{
				if (f.m_iOutside == RZ_SiteFacts.OUT_INFECTED && f.m_iTown == RZ_SiteFacts.LOSING)
					return 4;

				if (f.m_iTown == RZ_SiteFacts.LOSING || (hit && f.m_bByInfected))
					return 2.5;

				if (f.m_iOutside == RZ_SiteFacts.OUT_INFECTED)
					return 1.2;

				return 0;
			}
			case I_CLEAN:
			{
				if (f.m_bLab && thriving)
					return 3;

				if (f.m_bLab)
					return 2;

				return 1;
			}
			case I_COURIER:
			{
				if (hit && f.m_bByPeople && shortOf)
					return 4;

				if ((hit && f.m_bByPeople) || shortOf)
					return 2.2;

				return 0;
			}
			case I_BURNING:
			{
				if (f.m_iTown == RZ_SiteFacts.LOSING)
					return 3.5;

				return 0;
			}
			case I_STUDY:
			{
				if (f.m_bStudy)
					return 4;

				return 0;
			}
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! A story's own elements, then what the state adds (docs/SITES.md, "What it puts in the
	//! world"), at most MAX_ELEMENTS; `counts`: how many of each (signs, wrecks, graves).
	static void Elements(int story, RZ_SiteFacts f, notnull array<int> kinds, notnull array<int> counts)
	{
		switch (story)
		{
			case B_HELD: Add(kinds, counts, E_SIGN, 1); break;
			case B_DUG_IN: Add(kinds, counts, E_SANDBAGS, 1); Add(kinds, counts, E_SIGN, 2); break;
			case B_SHORT: Add(kinds, counts, E_ROSTER, 1); break;
			case B_CLOSE: Add(kinds, counts, E_PYRE, 1); Add(kinds, counts, E_BOARDED, 1); break;
			case R_TOOK: Add(kinds, counts, E_GATEMARK, 1); Add(kinds, counts, E_SANDBAGS, 1); break;
			case R_TOLL: Add(kinds, counts, E_WRECK, 2); Add(kinds, counts, E_SIGN, 1); break;
			case R_FAT: Add(kinds, counts, E_SPOILS, 1); Add(kinds, counts, E_SIGN, 1); break;
			case R_GOT_IN: Add(kinds, counts, E_BOARDED, 1); Add(kinds, counts, E_CHAIR, 1); break;
			case H_CHURCH: Add(kinds, counts, E_CHURCHDOOR, 1); Add(kinds, counts, E_SIGN, 1); break;
			case H_FARM: Add(kinds, counts, E_BUCKETS, 1); Add(kinds, counts, E_SIGN, 1); break;
			case H_BURYING: Add(kinds, counts, E_GRAVES, Math.Max(1, f.m_iDeaths)); Add(kinds, counts, E_JARS, 1); break;
			case H_NIGHT: Add(kinds, counts, E_BOARDED, 1); Add(kinds, counts, E_SIGN, 1); break;
			case I_CLEAN: Add(kinds, counts, E_TAPE, 1); break;
			case I_COURIER: Add(kinds, counts, E_BICYCLE, 1); break;
			case I_BURNING: Add(kinds, counts, E_BURN, 1); Add(kinds, counts, E_ROSTER, 1); break;
			case I_STUDY: Add(kinds, counts, E_GATEMARK, 1); break;
		}

		// What the state adds, whatever the story
		if (f.m_iSettled == RZ_SiteFacts.TAKEN && f.m_iOldOwner >= 0 && Recent(f.m_iTakenDay, TAKEN_DAYS))
			Add(kinds, counts, E_GATEMARK, 1);

		if (f.m_iLately == RZ_SiteFacts.BURNED)
			Add(kinds, counts, E_SCORCH, 1);

		if (f.m_iLately == RZ_SiteFacts.MOURNING || f.m_iDeaths > 0)
			Add(kinds, counts, E_GRAVES, Math.Max(1, f.m_iDeaths));

		if (f.m_bByInfected || f.m_iOutside == RZ_SiteFacts.OUT_INFECTED)
			Add(kinds, counts, E_PYRE, 1);

		if (f.m_bByPeople || f.m_iOutside == RZ_SiteFacts.OUT_HOSTILE)
			Add(kinds, counts, E_CASINGS, 1);

		int signs = 1;
		if (f.m_iGoing == RZ_SiteFacts.PRESSURE)
			signs = 2;

		Add(kinds, counts, E_SIGN, signs);
		if ((f.m_iSettled == RZ_SiteFacts.TAKEN || f.m_iSettled == RZ_SiteFacts.SETTLED) && f.m_bThrough)
			Add(kinds, counts, E_WRECK, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Once each, at most MAX_ELEMENTS; a second Add of a kind raises its count.
	protected static void Add(notnull array<int> kinds, notnull array<int> counts, int kind, int count)
	{
		int at = kinds.Find(kind);
		if (at >= 0)
		{
			counts[at] = Math.Max(counts[at], count);
			return;
		}

		if (kinds.Count() >= MAX_ELEMENTS)
			return;

		kinds.Insert(kind);
		counts.Insert(count);
	}

	//------------------------------------------------------------------------------------------------
	//! Every element a faction's stories (and the state) can show: the solver keeps a spot for each.
	static void FactionElements(RZ_EFaction f, notnull array<int> kinds)
	{
		kinds.Insert(E_SIGN);
		kinds.Insert(E_GATEMARK);
		kinds.Insert(E_CASINGS);
		kinds.Insert(E_PYRE);
		kinds.Insert(E_GRAVES);
		kinds.Insert(E_SCORCH);
		kinds.Insert(E_WRECK);
		switch (f)
		{
			case RZ_EFaction.MILITARY: kinds.Insert(E_SANDBAGS); kinds.Insert(E_ROSTER); kinds.Insert(E_BOARDED); break;
			case RZ_EFaction.BANDITS: kinds.Insert(E_SANDBAGS); kinds.Insert(E_BOARDED); kinds.Insert(E_CHAIR); kinds.Insert(E_SPOILS); break;
			case RZ_EFaction.SURVIVORS: kinds.Insert(E_CHURCHDOOR); kinds.Insert(E_BUCKETS); kinds.Insert(E_JARS); kinds.Insert(E_BOARDED); break;
			case RZ_EFaction.INSTITUTE: kinds.Insert(E_BICYCLE); kinds.Insert(E_BURN); kinds.Insert(E_TAPE); kinds.Insert(E_ROSTER); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	static string ElementName(int kind)
	{
		switch (kind)
		{
			case E_PYRE: return "the pyre";
			case E_CASINGS: return "casings by the nest";
			case E_SIGN: return "the sign on the approach";
			case E_WRECK: return "cars on the verge";
			case E_GRAVES: return "graves";
			case E_ROSTER: return "the board with a gap";
			case E_SCORCH: return "scorch";
			case E_GATEMARK: return "the mark at the gate";
			case E_SANDBAGS: return "sandbags and a HALT sign";
			case E_BOARDED: return "planks against the wire";
			case E_CHAIR: return "a chair under a sheet";
			case E_JARS: return "candle jars";
			case E_CHURCHDOOR: return "the church door";
			case E_BICYCLE: return "the courier's bicycle";
			case E_BURN: return "ash by the back";
			case E_TAPE: return "tape at the gate";
			case E_SPOILS: return "bags stacked by the door";
			case E_BUCKETS: return "buckets by the kitchen";
		}

		return "an element";
	}

	//------------------------------------------------------------------------------------------------
	//! The element's scene kind in Traces (Kit > Pieces that age: its faded twin): C1-C18.
	static string SceneKind(int kind)
	{
		return "C" + (kind + 1).ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! Where its spot is kept (AT_*).
	static int Where(int kind)
	{
		switch (kind)
		{
			case E_PYRE: return AT_KILL;
			case E_CASINGS: return AT_NEST;
			case E_SIGN: return AT_APPROACH;
			case E_WRECK: return AT_FAR_VERGE;
			case E_GRAVES: return AT_DEAD;
			case E_ROSTER: return AT_COURT;
			case E_SCORCH: return AT_ENTRANCE;
			case E_GATEMARK: return AT_GATE;
			case E_SANDBAGS: return AT_HOSTILE;
			case E_BOARDED: return AT_THREAT_IN;
			case E_CHAIR: return AT_COURT;
			case E_JARS: return AT_DEAD;
			case E_CHURCHDOOR: return AT_ENTRANCE;
			case E_BICYCLE: return AT_GATE;
			case E_BURN: return AT_DUMP;
			case E_TAPE: return AT_GATE;
			case E_SPOILS: return AT_COURT;
			case E_BUCKETS: return AT_KITCHEN;
		}

		return AT_COURT;
	}

	//------------------------------------------------------------------------------------------------
	//! Its footprint's half-size (x, z) as it stands: the spot kept for it (not measured: the kit's
	//! compositions are sized by hand here). Graves: a row of MAX_GRAVES, 1.6 m apart.
	static vector HalfSize(int kind)
	{
		switch (kind)
		{
			case E_PYRE: return Vector(1.5, 0, 1.5);
			case E_CASINGS: return Vector(1, 0, 0.75);
			case E_SIGN: return Vector(0.6, 0, 0.3);
			case E_WRECK: return Vector(1.1, 0, 2.4);
			case E_GRAVES: return Vector(MAX_GRAVES * 0.8, 0, 0.7);
			case E_ROSTER: return Vector(0.7, 0, 0.3);
			case E_SCORCH: return Vector(0.8, 0, 0.8);
			case E_GATEMARK: return Vector(1.3, 0, 0.3);
			case E_SANDBAGS: return Vector(1.5, 0, 1.2);
			case E_BOARDED: return Vector(1, 0, 0.75);
			case E_CHAIR: return Vector(0.6, 0, 0.6);
			case E_JARS: return Vector(0.4, 0, 0.4);
			case E_CHURCHDOOR: return Vector(1, 0, 0.75);
			case E_BICYCLE: return Vector(1, 0, 0.75);
			case E_BURN: return Vector(0.8, 0, 0.8);
			case E_TAPE: return Vector(1.5, 0, 0.3);
			case E_SPOILS: return Vector(0.8, 0, 0.6);
			case E_BUCKETS: return Vector(0.8, 0, 0.6);
		}

		return Vector(0.6, 0, 0.6);
	}

	//------------------------------------------------------------------------------------------------
	//! The element's i-th prefab from World > Traces > Kit ("" = text only). The mark at the gate:
	//! the old owner's door board first while the taking is recent, then the faction's own.
	static ResourceName Prefab(int kind, RZ_EFaction f, RZ_SiteFacts facts, int i)
	{
		RZ_TraceKit kit = RZ_TraceSettings.Get().Kit();
		if (!kit)
			return "";

		switch (kind)
		{
			case E_PYRE: return kit.m_sPyre;
			case E_CASINGS: return kit.m_sHeldOut;
			case E_SIGN: return MarkBoard(kit, f);
			case E_WRECK:
			{
				if (i % 2 == 1 && !kit.m_sQueueCarB.IsEmpty())
					return kit.m_sQueueCarB;

				return kit.m_sQueueCarA;
			}
			case E_GRAVES: return kit.m_sGrave;
			case E_ROSTER: return kit.m_sSeasonBoard;
			case E_SCORCH: return kit.m_sScorch;
			case E_GATEMARK:
			{
				if (i == 0 && facts && facts.m_iOldOwner >= 0 && Recent(facts.m_iTakenDay, TAKEN_DAYS))
					return Board(kit, facts.m_iOldOwner);

				return MarkBoard(kit, f);
			}
			case E_SANDBAGS: return kit.m_sRoadPostRelic;
			case E_BOARDED: return kit.m_sHeldOut;
			case E_CHAIR: return kit.m_sEdgeChair;
			case E_JARS: return kit.m_sCandleJars;
			case E_CHURCHDOOR: return kit.m_sChurchDoor;
			case E_BICYCLE: return kit.m_sDroppedRun;
			case E_BURN: return kit.m_sScorch;
			case E_TAPE: return kit.m_sSeasonTape;
			case E_SPOILS: return kit.m_sSuitcases;
			case E_BUCKETS: return kit.m_sWaterQueue;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! How many of an element stand (the mark at the gate: two while the old owner's is up).
	static int Count(int kind, RZ_SiteFacts facts, int want)
	{
		if (kind == E_GATEMARK)
		{
			if (facts && facts.m_iOldOwner >= 0 && Recent(facts.m_iTakenDay, TAKEN_DAYS))
				return 2;

			return 1;
		}

		if (kind == E_GRAVES)
			return Math.Clamp(want, 1, MAX_GRAVES);

		return Math.Clamp(want, 1, 2);
	}

	//------------------------------------------------------------------------------------------------
	//! The faction's own mark: its door board, else the kit's stencilled board (Season mark, taken).
	protected static ResourceName MarkBoard(RZ_TraceKit kit, int f)
	{
		ResourceName board = Board(kit, f);
		if (board.IsEmpty())
			board = kit.m_sSeasonStencil;

		return board;
	}

	//------------------------------------------------------------------------------------------------
	protected static ResourceName Board(RZ_TraceKit kit, int f)
	{
		switch (f)
		{
			case RZ_EFaction.MILITARY: return kit.m_sBoardBastion;
			case RZ_EFaction.SURVIVORS: return kit.m_sBoardHaven;
			case RZ_EFaction.BANDITS: return kit.m_sBoardRedHand;
			case RZ_EFaction.INSTITUTE: return kit.m_sBoardHalcyon;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The element's prefab as its age has it (docs/TRACES.md, How scenes age): Traces' own Stage on
	//! a scene of its kind made when its cause happened (an event's day, else the season's start, which
	//! ages by days), and its faded twin (Kit > Pieces that age, scene C1-C18 with no piece) in its last
	//! third. Its age (0 fresh, 1 weathered, 2 old) in `stage`.
	static ResourceName Aged(int kind, ResourceName own, int madeDay, out int stage)
	{
		stage = 0;
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetTraces() || !gm.GetTraces().GetScenes())
			return own;

		RZ_Scene s = new RZ_Scene();
		s.kind = SceneKind(kind);
		float today = RZ_SeasonClock.Days();
		if (madeDay >= 0)
		{
			s.made = today - Math.Max(0, RZ_WorldFacts.Get().Day() - madeDay);
			s.until = s.made + HISTORY_DAYS;
		}
		else
		{
			s.made = 0;
			s.until = -1;
		}

		stage = gm.GetTraces().GetScenes().Stage(s);
		if (stage < 2)
			return own;

		RZ_TraceKit kit = RZ_TraceSettings.Get().Kit();
		if (!kit || !kit.m_aPieces)
			return own;

		foreach (RZ_TracePiece piece : kit.m_aPieces)
		{
			if (piece && piece.m_sScene == s.kind && piece.m_sPrefab.IsEmpty() && !piece.m_sFaded.IsEmpty())
				return piece.m_sFaded;
		}

		return own;
	}

	//------------------------------------------------------------------------------------------------
	//! The broadcast day an element's cause happened (-1: always there, aged by days).
	static int MadeDay(int kind, RZ_SiteFacts f)
	{
		switch (kind)
		{
			case E_GATEMARK:
			case E_SANDBAGS:
			{
				if (f.m_iTakenDay >= 0 && Recent(f.m_iTakenDay, TAKEN_DAYS))
					return f.m_iTakenDay;

				return -1;
			}
			case E_PYRE:
			case E_CASINGS:
			case E_GRAVES:
			case E_SCORCH:
			case E_BOARDED:
			case E_CHAIR:
			case E_JARS:
			case E_BICYCLE:
				return f.m_iLatelyDay;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	static bool Recent(int day, int days)
	{
		if (day < 0)
			return false;

		return RZ_WorldFacts.Get().Day() - day <= days;
	}

	// ================================================================ The state
	//------------------------------------------------------------------------------------------------
	//! A site's facts now: its holding's history in the town (Traces' town history), its faction's
	//! strength, morale, need and pressure, a fire at it (Traces' scorch, L2), the town's infection,
	//! and what its people's plan kept of the surroundings (the threat front, the hostile neighbour).
	static RZ_SiteFacts Read(RZ_GameModeComponent gm, RZ_CampPlace p)
	{
		RZ_SiteFacts f = new RZ_SiteFacts();
		vector mid = p.m_vPos;
		if (p.m_Site)
		{
			mid = p.m_Site.m_vCentre;
			string what = p.m_Site.m_sWhat;
			f.m_bChurch = what.Contains("church");
			f.m_bFarm = what.Contains("farm");
			f.m_sPlace = PlaceName(what);
		}
		else
		{
			f.m_sPlace = "the camp";
		}

		f.m_bLab = p.m_eFaction == RZ_EFaction.INSTITUTE && p.m_eKind == RZ_ECampPlace.STRONGHOLD;
		if (!gm)
			return f;

		// The town
		int town = -1;
		RZ_Holding holding = HoldingOf(gm, p);
		if (holding)
			town = holding.m_iSettlement;

		if (town < 0)
			town = NearestTown(gm, mid);

		f.m_iTownIndex = town;
		if (town >= 0)
		{
			RZ_Settlement s = gm.GetSettlement(town);
			if (s)
			{
				f.m_sTown = s.m_sName;
				if (s.m_fInfection > 60)
					f.m_iTown = RZ_SiteFacts.LOSING;
				else if (s.m_fInfection >= 30)
					f.m_iTown = RZ_SiteFacts.FIGHTING;
			}
		}

		// How it was settled and what happened lately: the town's history
		RZ_Traces traces = gm.GetTraces();
		int today = RZ_WorldFacts.Get().Day();
		bool lost = false;
		if (traces && town >= 0)
		{
			array<ref RZ_TownEvent> events = traces.History().Get(town);
			if (events)
			{
				foreach (RZ_TownEvent e : events)
				{
					if (e.kind == "lost" && e.faction == p.m_eFaction)
						lost = true;

					if ((e.kind == "taken" || e.kind == "founded") && e.faction == p.m_eFaction)
					{
						f.m_iSettled = RZ_SiteFacts.SETTLED;
						if (e.kind == "taken")
						{
							f.m_iSettled = RZ_SiteFacts.TAKEN;
							// A taking's count is the old owner plus one (Traces' writers)
							if (e.count > 0)
								f.m_iOldOwner = e.count - 1;

							f.m_iTakenDay = e.day;
						}

						if (lost)
							f.m_iSettled = RZ_SiteFacts.RETAKEN;
					}

					if (today - e.day > HISTORY_DAYS)
						continue;

					bool infected = e.kind == "pyre" || e.kind == "infested";
					bool people = (e.kind == "taken" && e.faction != p.m_eFaction) || (e.kind == "lost" && e.faction == p.m_eFaction);
					bool dead = (e.kind == "dead" && e.faction == p.m_eFaction) || e.kind == "death";
					if (!infected && !people && !dead)
						continue;

					f.m_iLatelyDay = Math.Max(f.m_iLatelyDay, e.day);
					if (infected)
						f.m_bByInfected = true;

					if (people)
						f.m_bByPeople = true;

					if (dead)
						f.m_iDeaths += Math.Max(1, e.count);
				}
			}
		}

		if (f.m_iDeaths > 0)
			f.m_iLately = RZ_SiteFacts.MOURNING;
		else if (f.m_bByInfected || f.m_bByPeople)
			f.m_iLately = RZ_SiteFacts.HIT;

		// A building burned at it (Traces' scorch, L2) these days
		vector burnt;
		string burntKey;
		if (traces && traces.GetScenes() && traces.GetScenes().SceneNear("L2", mid, 60, burnt, burntKey) && f.m_iLately == RZ_SiteFacts.QUIET)
		{
			f.m_iLately = RZ_SiteFacts.BURNED;
			f.m_iLatelyDay = today;
		}

		// How it's going: its faction's strength, morale, need and pressure
		RZ_Territory territory = gm.GetTerritory();
		RZ_FactionLife life = gm.GetFactionLife();
		float strength = 50;
		float morale = 60;
		int need = RZ_ENeed.NONE;
		bool pressure = false;
		if (territory)
		{
			strength = territory.GetStrength(p.m_eFaction);
			pressure = territory.IsUnderPressure(p.m_eFaction);
		}

		if (life)
		{
			morale = life.GetMorale(p.m_eFaction);
			need = life.GetNeed(p.m_eFaction);
		}

		if (pressure)
			f.m_iGoing = RZ_SiteFacts.PRESSURE;
		else if (need != RZ_ENeed.NONE)
			f.m_iGoing = RZ_SiteFacts.SHORT;
		else if (strength >= 50 && morale >= 55)
			f.m_iGoing = RZ_SiteFacts.THRIVING;

		// What's out there: kept from the surroundings with its people's plan
		RZ_SitePeople people = p.m_People;
		if (people)
		{
			f.m_iOutside = people.m_iFrontKind;
			f.m_bThrough = people.m_bThrough;
			f.m_bHostileRedHand = people.m_iHostileFaction == RZ_EFaction.BANDITS && people.m_fHostileDist >= 0;
		}

		// A nest under study near (Halcyon's Specimen from the Nest running)
		RZ_MissionManager missions = gm.GetMissions();
		if (missions && p.m_eFaction == RZ_EFaction.INSTITUTE)
		{
			foreach (RZ_Mission m : missions.GetActive())
			{
				if (m && m.m_eState == RZ_EMissionState.ACTIVE && m.m_sTitle.IndexOf("Specimen from the Nest") == 0 && RZ_Util.DistXZ(m.m_vPos, mid) < 1500)
					f.m_bStudy = true;
			}
		}

		return f;
	}

	//------------------------------------------------------------------------------------------------
	//! The holding a camp place stands for (its key is "h" and the holding's id), or null.
	protected static RZ_Holding HoldingOf(RZ_GameModeComponent gm, RZ_CampPlace p)
	{
		if (!p.m_sKey.StartsWith("h") || !gm.GetTerritory())
			return null;

		int id = p.m_sKey.Substring(1, p.m_sKey.Length() - 1).ToInt();
		foreach (RZ_Holding h : gm.GetTerritory().GetHoldings())
		{
			if (h.m_iId == id)
				return h;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected static int NearestTown(RZ_GameModeComponent gm, vector at)
	{
		int best = -1;
		float bestDist = float.MAX;
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			float d = RZ_Util.DistXZ(s.m_vCenter, at);
			if (d < Math.Max(s.m_fRadius, 150) && d < bestDist)
			{
				bestDist = d;
				best = i;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! The place's name from the building it moved into: "a school" -> "the school".
	static string PlaceName(string what)
	{
		if (what.StartsWith("a "))
			return "the " + what.Substring(2, what.Length() - 2);

		if (what.StartsWith("an "))
			return "the " + what.Substring(3, what.Length() - 3);

		if (what.IsEmpty())
			return "the camp";

		return what;
	}

	// ================================================================ Words
	//------------------------------------------------------------------------------------------------
	//! Who speaks when you come in.
	static string Speaker(RZ_EFaction f)
	{
		switch (f)
		{
			case RZ_EFaction.MILITARY: return "The sentry";
			case RZ_EFaction.BANDITS: return "The toll man";
			case RZ_EFaction.SURVIVORS: return "The lookout";
			case RZ_EFaction.INSTITUTE: return "The guard";
		}

		return "Someone at the gate";
	}

	//------------------------------------------------------------------------------------------------
	//! The line its people say when you come in, by its state (docs/WRITING.md: the faction's voice,
	//! plain and short; never "you're new").
	static string Line(RZ_EFaction f, RZ_SiteFacts facts)
	{
		bool hit = facts.m_iLately == RZ_SiteFacts.HIT || facts.m_iLately == RZ_SiteFacts.BURNED;
		bool mourning = facts.m_iLately == RZ_SiteFacts.MOURNING;
		bool shortOf = facts.m_iGoing == RZ_SiteFacts.SHORT || facts.m_iGoing == RZ_SiteFacts.PRESSURE;
		switch (f)
		{
			case RZ_EFaction.MILITARY:
			{
				if (mourning)
					return "New tags on the nails this week. Sign in, keep it short.";

				if (hit)
					return "Two on the wire tonight. Don't argue.";

				if (shortOf)
					return "Stores are thin. Don't ask, and nobody has to tell you no.";

				return "Sign in at the board. Keep your hands where we can see them.";
			}
			case RZ_EFaction.BANDITS:
			{
				if (mourning || hit)
					return "Something came through the back. We're not talking about it.";

				if (shortOf)
					return "Prices went up this morning. Take it up with the road.";

				return "Toll's on the sign. Read it.";
			}
			case RZ_EFaction.SURVIVORS:
			{
				if (mourning)
					return "We buried our own this week. Sit with us a while.";

				if (hit)
					return "It came close last night. Sit down, you look worse than we do.";

				if (shortOf)
					return "Half portions this week. There's still a bowl for you.";

				return "There's soup. There's always soup.";
			}
			case RZ_EFaction.INSTITUTE:
			{
				if (mourning || hit)
					return "Nobody goes out past the lights.";

				if (shortOf)
					return "We're short of everything that matters. Don't touch the cold store.";

				return "Through the shower first. All of you.";
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A quiet headline when a site is hit or mourning (the news lines: stored, never broadcast):
	//! one plain sentence and one detail. "" when it's neither.
	static string Mention(RZ_EFaction f, RZ_SiteFacts facts)
	{
		if (facts.m_sTown.IsEmpty())
			return "";

		string at = facts.m_sPlace + " in " + facts.m_sTown;
		if (facts.m_iLately == RZ_SiteFacts.MOURNING)
		{
			switch (f)
			{
				case RZ_EFaction.MILITARY: return "Bastion hung new tags on the nails at " + at + ". The roll is shorter.";
				case RZ_EFaction.BANDITS: return "The Red Hand boarded up a door at " + at + " and won't say why.";
				case RZ_EFaction.SURVIVORS: return "The Haven buried its own by " + at + ". The candle jars were lit at dusk.";
				case RZ_EFaction.INSTITUTE: return "Halcyon lost people at " + at + ". The guard on the gate has been doubled.";
			}
		}

		if (facts.m_iLately == RZ_SiteFacts.HIT || facts.m_iLately == RZ_SiteFacts.BURNED)
		{
			switch (f)
			{
				case RZ_EFaction.MILITARY: return "Something came at the wire at " + at + " in the night. Bastion has two on it now.";
				case RZ_EFaction.BANDITS: return "Somebody hit the Red Hand at " + at + ". The toll went up the next morning.";
				case RZ_EFaction.SURVIVORS: return "It came close at " + at + ". The Haven has doubled the fence with furniture.";
				case RZ_EFaction.INSTITUTE: return "Halcyon burned something behind " + at + ". Nobody goes out past the lights.";
			}
		}

		return "";
	}
}
