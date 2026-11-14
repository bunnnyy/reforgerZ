//! Door marks (docs/TRACES.md, "The door-mark grammar"): chalk, paint, stencils and scratches beside
//! the outside doors of a few buildings in a town, written by the people who were there. A mark is
//! data: a town, a door slot (a number, resolved from the map near players: RZ_DoorSlots), a few
//! layers (oldest first), each with its symbol, author, day and fields. Its words are always there
//! (RZ_TraceText); its look is a decal or a board when those are set up (RZ_TraceDraw).
//!
//! - Seeded marks (the first days, Days 1-10, RZ_FirstDays) are made again the same at every start
//!   and never saved. Live marks (from Day 11, RZ_TraceWriters) are saved, with their door's slot.
//! - Caps: 6 marked doors a town (8 in a stronghold's town), 3 layers a door, 2 live writes a town
//!   and 20 on the server a season day, 600 live layers on the server.
//! - Overpaint: the Red Hand paints over Bastion (and over Halcyon in its own towns), Bastion paints
//!   grey over the Red Hand, the Haven never paints over anything, nobody paints over a bear or
//!   residents' words. A fourth layer pushes the most weathered off.
//! - Never within 2 km of the Long Table's farm (residents' words from Days 1-3 aside), never on
//!   the first patient's clinic.
//! Server only.

//------------------------------------------------------------------------------------------------
//! The symbols. The first 16 are the cells of the door atlas, in order; the infected's three and
//! the thumbprint are cells 4-7 of the overlay atlas; residents' words have no cell (text only).
enum RZ_EDoorSymbol
{
	X,				// Bastion's search code
	SLASH,			// Bastion: went in, never marked it done
	POSTCODE,		// Bastion: a post's street, "B-2 · D14"
	CONTRIB,		// Bastion: "C · D9", contribution taken
	ATT,			// Bastion: attached civilians
	CLEARED,		// Bastion: culled, shelled or retaken
	BEAR,			// the Haven: children inside (ringed: moved on; ticked: everyone got out)
	PATCHED_BEAR,	// the Haven: a Haven house
	CANDLE,			// the Haven: someone from this house was lost
	HATCH,			// the Haven: a walk-out
	RING_DOT,		// the Haven: checked, nobody here
	ARROW,			// the Haven: gone to the house at...
	DROP,			// the Haven: the pump here works
	HAND,			// the Red Hand: paid (struck: lapsed)
	TALLY,			// the Red Hand: owed (PAID across it: cleared)
	BEAK,			// Halcyon: sampled here (ringed: please don't burn; barred: finished)
	SCRATCH_OUT,	// the infected: they wanted in
	SCRATCH_IN,		// the infected: someone turned in there
	SMEARS,			// the infected: the slow ones lay against it
	THUMB,			// the Red Hand: someone the Haven put out sleeps here (an exile, RZ_TraceWatch)
	WORDS,			// residents: whatever they needed to say
	PLATE			// the Long Table: a plate and a fork, scratched small; someone lone here was asked to supper (World/RZ_LongTableReach.c)
}

//------------------------------------------------------------------------------------------------
enum RZ_EDoorAuthor
{
	BASTION,
	HAVEN,
	REDHAND,
	HALCYON,
	INFECTED,
	RESIDENTS
}

//------------------------------------------------------------------------------------------------
//! What kind of building a write asks for (the first free slot of that kind; with none, a house).
enum RZ_EDoorWant
{
	ANY,
	HOUSE,
	SHOP,
	CLINIC,
	FARM,
	GARAGE,
	PUBLIC,		// a school, a church, a town hall
	NEAR		// the building nearest the write's point
}

//------------------------------------------------------------------------------------------------
//! One layer on a door.
class RZ_DoorMark
{
	static const int F_RING = 1;		// ringed (a bear, an X, a beak)
	static const int F_TICK = 2;		// a ticked bear
	static const int F_STRUCK = 4;		// a struck hand
	static const int F_PAID = 8;		// PAID across a tally
	static const int F_OVER = 16;		// painted over by someone else (shows through)
	static const int F_GREY = 32;		// Bastion painted grey first
	static const int F_SEEDED = 64;		// the first days: made again at every start, never saved
	static const int F_BAR = 128;		// a barred beak
	static const int F_SHOWN = 256;		// a candle's words show at a glance (a player's description)
	static const int F_TEMP = 512;		// errands' old mark: two hours, never saved
	static const int F_YOUNG = 1024;	// the young come home (L4): c is the pack's id + 1
	static const int F_ASKED = 2048;	// written for the epilogue: after the cure, past the daily caps

	int m_iSymbol;
	int m_iAuthor;
	int m_iDay;			// the day as the broadcast counts it (Days 1-10 are the first days)
	int m_iA;			// fields, by symbol (RZ_TraceText)
	int m_iB;
	int m_iC;
	int m_iFlags;
	float m_fMade = -1;	// season days (RZ_SeasonClock.Days) when written; -1 for seeded marks
	float m_fRain;		// the rain counter when written (chalk washes twice as fast in rain)
	int m_iUntil;		// F_TEMP: real time it goes
	string m_sText;		// a candle's names or description

	//------------------------------------------------------------------------------------------------
	static RZ_DoorMark Make(int symbol, int day, int a = 0, int b = 0, int c = 0, int flags = 0)
	{
		RZ_DoorMark m = new RZ_DoorMark();
		m.m_iSymbol = symbol;
		m.m_iAuthor = AuthorOf(symbol);
		m.m_iDay = day;
		m.m_iA = a;
		m.m_iB = b;
		m.m_iC = c;
		m.m_iFlags = flags;
		return m;
	}

	//------------------------------------------------------------------------------------------------
	static int AuthorOf(int symbol)
	{
		if (symbol <= RZ_EDoorSymbol.CLEARED)
			return RZ_EDoorAuthor.BASTION;

		if (symbol <= RZ_EDoorSymbol.DROP)
			return RZ_EDoorAuthor.HAVEN;

		if (symbol == RZ_EDoorSymbol.HAND || symbol == RZ_EDoorSymbol.TALLY || symbol == RZ_EDoorSymbol.THUMB)
			return RZ_EDoorAuthor.REDHAND;

		if (symbol == RZ_EDoorSymbol.BEAK)
			return RZ_EDoorAuthor.HALCYON;

		if (symbol == RZ_EDoorSymbol.WORDS || symbol == RZ_EDoorSymbol.PLATE)
			return RZ_EDoorAuthor.RESIDENTS;

		return RZ_EDoorAuthor.INFECTED;
	}

	//------------------------------------------------------------------------------------------------
	//! Haven chalk washes; paint, stencils and the bear don't; scratches grey.
	bool IsChalk()
	{
		return m_iSymbol == RZ_EDoorSymbol.CANDLE || m_iSymbol == RZ_EDoorSymbol.HATCH || m_iSymbol == RZ_EDoorSymbol.RING_DOT
			|| m_iSymbol == RZ_EDoorSymbol.ARROW || m_iSymbol == RZ_EDoorSymbol.DROP;
	}

	//------------------------------------------------------------------------------------------------
	bool Has(int flag)
	{
		return (m_iFlags & flag) != 0;
	}

	//------------------------------------------------------------------------------------------------
	bool IsSeeded()
	{
		return Has(F_SEEDED);
	}

	//------------------------------------------------------------------------------------------------
	bool IsBear()
	{
		return m_iSymbol == RZ_EDoorSymbol.BEAR || m_iSymbol == RZ_EDoorSymbol.PATCHED_BEAR;
	}

	//------------------------------------------------------------------------------------------------
	//! How much a reader would miss it: what the eviction keeps longest.
	int Salience()
	{
		switch (m_iSymbol)
		{
			case RZ_EDoorSymbol.CANDLE: return 4;
			case RZ_EDoorSymbol.BEAR: return 3;
			case RZ_EDoorSymbol.PATCHED_BEAR: return 4;
			case RZ_EDoorSymbol.HATCH: return 3;
			case RZ_EDoorSymbol.WORDS: return 3;
			case RZ_EDoorSymbol.X: return 2;
			case RZ_EDoorSymbol.HAND: return 2;
			case RZ_EDoorSymbol.BEAK: return 2;
			case RZ_EDoorSymbol.CLEARED: return 2;
		}

		return 1;
	}
}

//------------------------------------------------------------------------------------------------
//! A door that carries marks. m_iSlot -1 until its town's doors are resolved near a player.
class RZ_MarkedDoor
{
	string m_sKey;			// "fd.<town>.<n>" for a first-days door, "lv.<n>" for a live one
	int m_iTown = -1;
	int m_iSlot = -1;
	int m_iWant;			// RZ_EDoorWant
	vector m_vAnchor;		// the point the write was about (zero: anywhere in town)
	ref array<ref RZ_DoorMark> m_aLayers = {};	// oldest first

	// Runtime, once bound (RZ_DoorSlots)
	bool m_bPlaced;			// its slot is resolved on the map this run
	vector m_vPos;			// the mark, beside the door on its wall
	vector m_vNormal;		// straight out of the wall
	bool m_bLocked;			// the lock system's building (the X says SEAL)
	int m_iKind;			// the slot's building kind (RZ_EDoorWant)

	//------------------------------------------------------------------------------------------------
	bool IsBound()
	{
		return m_bPlaced;
	}

	//------------------------------------------------------------------------------------------------
	RZ_DoorMark Top()
	{
		if (m_aLayers.IsEmpty())
			return null;

		return m_aLayers[m_aLayers.Count() - 1];
	}

	//------------------------------------------------------------------------------------------------
	bool HasLive()
	{
		foreach (RZ_DoorMark m : m_aLayers)
		{
			if (!m.IsSeeded() && !m.Has(RZ_DoorMark.F_TEMP))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	RZ_DoorMark Find(int symbol)
	{
		for (int i = m_aLayers.Count() - 1; i >= 0; i--)
		{
			if (m_aLayers[i].m_iSymbol == symbol)
				return m_aLayers[i];
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	int Salience()
	{
		int best = 0;
		foreach (RZ_DoorMark m : m_aLayers)
		{
			best = Math.Max(best, m.Salience());
		}

		return best;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_DoorMarkSave
{
	int symbol;
	int day;
	int a;
	int b;
	int c;
	int flags;
	float made;
	float rain;
	string text;
}

//------------------------------------------------------------------------------------------------
//! A door with live layers (seeded layers are made again at start and never saved).
class RZ_MarkedDoorSave
{
	string key;
	int town;
	int slot;
	int want;
	float x;
	float z;
	ref array<ref RZ_DoorMarkSave> layers = {};
}

//------------------------------------------------------------------------------------------------
class RZ_DoorMarks
{
	static const int LIVE_CAP = 600;
	protected static const float OLD_MARK_RADIUS = 60;
	protected static const int OLD_MARK_SECONDS = 7200;

	protected RZ_GameModeComponent m_GM;
	protected RZ_Traces m_Traces;
	protected ref array<ref RZ_MarkedDoor> m_aDoors = {};
	protected ref map<string, int> m_mWrites = new map<string, int>();	// "town|day" -> live writes, "*|day" the server
	protected int m_iNextKey = 1;

	//------------------------------------------------------------------------------------------------
	void RZ_DoorMarks(RZ_GameModeComponent gm, RZ_Traces traces)
	{
		m_GM = gm;
		m_Traces = traces;
	}

	//------------------------------------------------------------------------------------------------
	array<ref RZ_MarkedDoor> GetDoors()
	{
		return m_aDoors;
	}

	//------------------------------------------------------------------------------------------------
	RZ_MarkedDoor FindByKey(string key)
	{
		foreach (RZ_MarkedDoor d : m_aDoors)
		{
			if (d.m_sKey == key)
				return d;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	void TownDoors(int town, notnull array<RZ_MarkedDoor> result)
	{
		foreach (RZ_MarkedDoor d : m_aDoors)
		{
			if (d.m_iTown == town && !d.m_aLayers.IsEmpty())
				result.Insert(d);
		}
	}

	//------------------------------------------------------------------------------------------------
	int LiveCount()
	{
		int n = 0;
		foreach (RZ_MarkedDoor d : m_aDoors)
		{
			foreach (RZ_DoorMark m : d.m_aLayers)
			{
				if (!m.IsSeeded() && !m.Has(RZ_DoorMark.F_TEMP))
					n++;
			}
		}

		return n;
	}

	// ================================================================ Age
	//------------------------------------------------------------------------------------------------
	//! Season days since it was written. Chalk counts the rain twice. Worked out when asked:
	//! nothing ticks.
	float AgeDays(RZ_DoorMark m)
	{
		float age;
		if (m.m_fMade >= 0)
			age = RZ_SeasonClock.DaysSince(m.m_fMade);
		else
			age = Math.Max(0, RZ_WorldFacts.Get().Day() - m.m_iDay) + 0.5;

		if (m.IsChalk() && m.m_fMade >= 0 && m_Traces)
			age += Math.Max(0, m_Traces.RainDays() - m.m_fRain);

		return age;
	}

	//------------------------------------------------------------------------------------------------
	//! 0 fresh, 1 weathered, 2 old (docs/TRACES.md, How things age).
	int Stage(RZ_DoorMark m)
	{
		float age = AgeDays(m);
		if (age < 1)
			return 0;

		float oldAt = 5;
		if (m.IsChalk())
			oldAt = 3;

		if (age <= oldAt)
			return 1;

		return 2;
	}

	//------------------------------------------------------------------------------------------------
	//! Live chalk at 8 days with no rewrite is gone; a temp mark at its time. Seeded marks are
	//! history: they only get old.
	protected bool IsGone(RZ_DoorMark m)
	{
		if (m.Has(RZ_DoorMark.F_TEMP))
			return RZ_SeasonClock.Now() >= m.m_iUntil;

		return m.IsChalk() && m.m_fMade >= 0 && AgeDays(m) >= 8;
	}

	// ================================================================ Writing
	//------------------------------------------------------------------------------------------------
	//! A town may take a mark: not the Long Table's ground (residents' words from Days 1-3 aside).
	protected bool TownAllows(int town, RZ_DoorMark m)
	{
		RZ_Settlement s = m_GM.GetSettlement(town);
		if (!s)
			return false;

		RZ_LongTable lt = m_GM.GetLongTable();
		if (lt && lt.Exists() && lt.IsNearHouse(s.m_vCenter, 2000))
			return m.m_iSymbol == RZ_EDoorSymbol.WORDS && m.m_iDay <= 3;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A live write may happen: 2 a town and 20 on the server a season day.
	protected bool MayWriteLive(int town)
	{
		int day = RZ_SeasonClock.Day();
		string townKey = string.Format("%1|%2", town, day);
		string allKey = string.Format("*|%1", day);
		int n;
		m_mWrites.Find(townKey, n);
		int all;
		m_mWrites.Find(allKey, all);
		if (n >= 2 || all >= m_Traces.GetSettings().m_iLiveWritesPerDay)
			return false;

		m_mWrites.Set(townKey, n + 1);
		m_mWrites.Set(allKey, all + 1);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Writes `m` on a door in `town`: the door `doorKey` if given, else one already marked with
	//! the same want near `anchor`, else a new one (a full town gives up its least salient, most
	//! weathered door). Seeded layers (F_SEEDED) skip the daily caps. Null if nothing was written.
	RZ_MarkedDoor Write(int town, int want, vector anchor, RZ_DoorMark m, string cause, string doorKey = "")
	{
		if (!m || !m_Traces.DoorMarksOn() || !TownAllows(town, m))
			return null;

		if (want == RZ_EDoorWant.CLINIC && town == RZ_WorldFacts.Get().CanonSettlement("origin.clinic"))
			return null;

		bool seeded = m.IsSeeded();
		if (!seeded && !m.Has(RZ_DoorMark.F_TEMP))
		{
			bool asked = m.Has(RZ_DoorMark.F_ASKED);
			if (RZ_WorldFacts.Get().IsCured() && !asked)
				return null;	// after the cure only what the epilogue asks for

			if (!asked && Fading(town, m))
				return null;

			if (!asked && !MayWriteLive(town))
				return null;

			m.m_fMade = RZ_SeasonClock.Days();
			m.m_fRain = m_Traces.RainDays();
		}

		RZ_MarkedDoor d = null;
		if (!doorKey.IsEmpty())
			d = FindByKey(doorKey);

		if (!d && doorKey.IsEmpty())
			d = FindDoor(town, want, anchor, m);

		if (!d)
			d = NewDoor(town, want, anchor, doorKey, seeded);

		if (!d)
			return null;

		AddLayer(d, m);
		if (!seeded)
		{
			EnforceLiveCap();
			RZ_Log.Info(string.Format("Traces: door %1#%2 %3 %4 D%5 (%6)", TownName(town), d.m_iSlot, AuthorKey(m.m_iAuthor), RZ_TraceText.SymbolKey(m.m_iSymbol), m.m_iDay, cause));
		}

		if (!d.IsBound() && m_Traces.GetSlots().IsResolved(town))
			Bind(town);

		m_Traces.DoorChanged(d);
		return d;
	}

	//------------------------------------------------------------------------------------------------
	//! The epilogue's asks (GONE HOME, a wanderer's name): through the grammar and its door caps,
	//! after the cure and past the daily caps (RZ_Traces.ChalkName, GoneHome).
	RZ_MarkedDoor WriteForEpilogue(int town, int want, vector anchor, RZ_DoorMark m, string cause, string doorKey)
	{
		m.m_iFlags = m.m_iFlags | RZ_DoorMark.F_ASKED;
		return Write(town, want, anchor, m, cause, doorKey);
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion fading (L6): with its presence on the roads under 0.25, no new Bastion mark outside
	//! the towns it holds (with World > Traces > Live traces on).
	protected bool Fading(int town, RZ_DoorMark m)
	{
		if (m.m_iAuthor != RZ_EDoorAuthor.BASTION || !m_Traces.GetSettings().m_bLiveTraces)
			return false;

		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		return facts.BastionPresence() < 0.25 && !facts.TownOwner(town).IsHeldBy(RZ_EFaction.MILITARY);
	}

	//------------------------------------------------------------------------------------------------
	//! A door already marked in the town that this write belongs on: the same want, near the
	//! anchor (40 m), and one whose top layer the writer may sit beside or over.
	protected RZ_MarkedDoor FindDoor(int town, int want, vector anchor, RZ_DoorMark m)
	{
		RZ_MarkedDoor best = null;
		float bestDist = 40;
		RZ_MarkedDoor over = null;
		foreach (RZ_MarkedDoor d : m_aDoors)
		{
			if (d.m_iTown != town || d.m_iWant != want)
				continue;

			// A rewrite of the same kind of mark always goes on the door that has it
			RZ_DoorMark same = d.Find(m.m_iSymbol);
			if (same && same.m_iAuthor == m.m_iAuthor && (anchor == vector.Zero || RZ_Util.DistXZ(d.m_vAnchor, anchor) < 60))
				return d;

			// A writer who paints over someone goes where their marks are (the crews' hand on the
			// shop over Bastion's X)
			if (!over && PaintsOver(d, m))
				over = d;

			if (anchor == vector.Zero)
				continue;

			float dist = RZ_Util.DistXZ(d.m_vAnchor, anchor);
			if (dist < bestDist)
			{
				bestDist = dist;
				best = d;
			}
		}

		if (best)
			return best;

		return over;
	}

	//------------------------------------------------------------------------------------------------
	//! `m` would paint over a mark on `d` (Overpaint's rules).
	protected bool PaintsOver(RZ_MarkedDoor d, RZ_DoorMark m)
	{
		foreach (RZ_DoorMark a : d.m_aLayers)
		{
			if (a.Has(RZ_DoorMark.F_OVER))
				continue;

			if (m.m_iSymbol == RZ_EDoorSymbol.HAND && a.m_iAuthor == RZ_EDoorAuthor.BASTION)
				return true;

			if ((m.m_iSymbol == RZ_EDoorSymbol.POSTCODE || m.m_iSymbol == RZ_EDoorSymbol.CLEARED) && a.m_iAuthor == RZ_EDoorAuthor.REDHAND)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_MarkedDoor NewDoor(int town, int want, vector anchor, string key, bool seeded)
	{
		array<RZ_MarkedDoor> here = {};
		TownDoors(town, here);
		int cap = m_Traces.DoorsPerTown(town);
		if (here.Count() >= cap)
		{
			RZ_MarkedDoor drop = null;
			foreach (RZ_MarkedDoor d : here)
			{
				if (!drop || d.Salience() < drop.Salience() || (d.Salience() == drop.Salience() && WeatherScore(d) > WeatherScore(drop)))
					drop = d;
			}

			// A seeded write never pushes out a live door: it's history, and history waits
			if (!drop || (seeded && drop.HasLive()))
				return null;

			RemoveDoor(drop);
		}

		RZ_MarkedDoor nd = new RZ_MarkedDoor();
		nd.m_sKey = key;
		if (nd.m_sKey.IsEmpty())
		{
			nd.m_sKey = "lv." + m_iNextKey.ToString();
			m_iNextKey++;
		}

		nd.m_iTown = town;
		nd.m_iWant = want;
		nd.m_vAnchor = anchor;
		m_aDoors.Insert(nd);
		return nd;
	}

	//------------------------------------------------------------------------------------------------
	protected float WeatherScore(RZ_MarkedDoor d)
	{
		float worst = 0;
		foreach (RZ_DoorMark m : d.m_aLayers)
		{
			worst = Math.Max(worst, AgeDays(m));
		}

		return worst;
	}

	//------------------------------------------------------------------------------------------------
	void RemoveDoor(RZ_MarkedDoor d)
	{
		m_Traces.DoorRemoved(d);
		m_aDoors.RemoveItemOrdered(d);
	}

	//------------------------------------------------------------------------------------------------
	//! A rewrite (the same symbol by the same writer: a candle's number, a hand's strokes, the drop
	//! re-chalked) updates the layer and resets its age; anything else is a new layer, with the
	//! overpaint rules, and a fourth layer pushes the most weathered off.
	protected void AddLayer(RZ_MarkedDoor d, RZ_DoorMark m)
	{
		RZ_DoorMark same = d.Find(m.m_iSymbol);
		if (same && same.m_iAuthor == m.m_iAuthor && !same.Has(RZ_DoorMark.F_OVER) && same.IsSeeded() == m.IsSeeded() && IsRewrite(m))
		{
			same.m_iDay = m.m_iDay;
			same.m_iA = m.m_iA;
			same.m_iB = m.m_iB;
			same.m_iC = m.m_iC;
			same.m_iFlags = m.m_iFlags;
			same.m_fMade = m.m_fMade;
			same.m_fRain = m.m_fRain;
			same.m_iUntil = m.m_iUntil;
			if (!m.m_sText.IsEmpty())
				same.m_sText = m.m_sText;

			// Back on top: it's the freshest thing on the door
			d.m_aLayers.RemoveItemOrdered(same);
			d.m_aLayers.Insert(same);
			return;
		}

		Overpaint(d, m);
		d.m_aLayers.Insert(m);
		int cap = Math.Max(1, m_Traces.GetSettings().m_iLayersPerDoor);
		while (d.m_aLayers.Count() > cap)
		{
			RZ_DoorMark drop = null;
			foreach (RZ_DoorMark old : d.m_aLayers)
			{
				if (old == m)
					continue;

				if (!drop || (old.Has(RZ_DoorMark.F_OVER) && !drop.Has(RZ_DoorMark.F_OVER)) || (old.Has(RZ_DoorMark.F_OVER) == drop.Has(RZ_DoorMark.F_OVER) && AgeDays(old) > AgeDays(drop)))
					drop = old;
			}

			if (!drop)
				break;

			d.m_aLayers.RemoveItemOrdered(drop);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsRewrite(RZ_DoorMark m)
	{
		switch (m.m_iSymbol)
		{
			case RZ_EDoorSymbol.CANDLE: return true;
			case RZ_EDoorSymbol.DROP: return true;
			case RZ_EDoorSymbol.HAND: return true;
			case RZ_EDoorSymbol.TALLY: return true;
			case RZ_EDoorSymbol.BEAR: return true;
			case RZ_EDoorSymbol.BEAK: return true;
			case RZ_EDoorSymbol.SCRATCH_OUT: return true;
			case RZ_EDoorSymbol.POSTCODE: return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Who paints over whom. Nobody paints over a bear or residents' words; the Haven paints over
	//! nothing; the infected's marks aren't paint.
	protected void Overpaint(RZ_MarkedDoor d, RZ_DoorMark m)
	{
		bool over = false;
		if (m.m_iAuthor == RZ_EDoorAuthor.REDHAND && m.m_iSymbol == RZ_EDoorSymbol.HAND)
		{
			bool crewTown = RZ_WorldFacts.Get().TownOwner(d.m_iTown).IsHeldBy(RZ_EFaction.BANDITS);
			foreach (RZ_DoorMark a : d.m_aLayers)
			{
				if (a.Has(RZ_DoorMark.F_OVER))
					continue;

				if (a.m_iAuthor == RZ_EDoorAuthor.BASTION || (crewTown && a.m_iAuthor == RZ_EDoorAuthor.HALCYON))
				{
					a.m_iFlags = a.m_iFlags | RZ_DoorMark.F_OVER;
					over = true;
				}
			}
		}
		else if (m.m_iAuthor == RZ_EDoorAuthor.BASTION && (m.m_iSymbol == RZ_EDoorSymbol.POSTCODE || m.m_iSymbol == RZ_EDoorSymbol.CLEARED))
		{
			foreach (RZ_DoorMark b : d.m_aLayers)
			{
				if (b.m_iAuthor == RZ_EDoorAuthor.REDHAND && !b.Has(RZ_DoorMark.F_OVER))
				{
					b.m_iFlags = b.m_iFlags | RZ_DoorMark.F_OVER;
					m.m_iFlags = m.m_iFlags | RZ_DoorMark.F_GREY;
					over = true;
				}
			}
		}

		if (over && !m.IsSeeded())
			RZ_Log.Info(string.Format("Traces: overpaint %1#%2 by %3", TownName(d.m_iTown), d.m_iSlot, AuthorKey(m.m_iAuthor)));
	}

	//------------------------------------------------------------------------------------------------
	//! Over 600 live layers on the server: the oldest live layer anywhere goes.
	protected void EnforceLiveCap()
	{
		while (LiveCount() > LIVE_CAP)
		{
			RZ_MarkedDoor dropDoor = null;
			RZ_DoorMark drop = null;
			foreach (RZ_MarkedDoor d : m_aDoors)
			{
				foreach (RZ_DoorMark m : d.m_aLayers)
				{
					if (m.IsSeeded() || m.Has(RZ_DoorMark.F_TEMP))
						continue;

					if (!drop || m.m_fMade < drop.m_fMade)
					{
						drop = m;
						dropDoor = d;
					}
				}
			}

			if (!drop)
				return;

			dropDoor.m_aLayers.RemoveItemOrdered(drop);
			if (dropDoor.m_aLayers.IsEmpty())
				RemoveDoor(dropDoor);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every minute: gone chalk and old marks' time up come off; empty doors go.
	void Prune()
	{
		for (int i = m_aDoors.Count() - 1; i >= 0; i--)
		{
			RZ_MarkedDoor d = m_aDoors[i];
			bool changed = false;
			for (int j = d.m_aLayers.Count() - 1; j >= 0; j--)
			{
				if (IsGone(d.m_aLayers[j]))
				{
					d.m_aLayers.RemoveOrdered(j);
					changed = true;
				}
			}

			if (d.m_aLayers.IsEmpty())
				RemoveDoor(d);
			else if (changed)
				m_Traces.DoorChanged(d);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every mark in a town comes off (the admin command).
	int ClearTown(int town)
	{
		int n = 0;
		for (int i = m_aDoors.Count() - 1; i >= 0; i--)
		{
			if (m_aDoors[i].m_iTown != town)
				continue;

			RemoveDoor(m_aDoors[i]);
			n++;
		}

		return n;
	}

	// ================================================================ Binding to the map
	//------------------------------------------------------------------------------------------------
	//! The town's doors are resolved (RZ_DoorSlots): each marked door takes a slot. Doors that kept
	//! a slot from the save keep it; the rest take the first free slot of their kind, nearest their
	//! point, else a house, else any. A door with no slot left stays words only.
	void Bind(int town)
	{
		RZ_TownSlots slots = m_Traces.GetSlots().Get(town);
		if (!slots)
			return;

		array<int> used = {};
		array<RZ_MarkedDoor> here = {};
		TownDoors(town, here);
		foreach (RZ_MarkedDoor kept : here)
		{
			if (kept.m_iSlot >= 0 && kept.m_iSlot < slots.m_aSlots.Count() && !used.Contains(kept.m_iSlot))
			{
				used.Insert(kept.m_iSlot);
				Place(kept, slots.m_aSlots[kept.m_iSlot]);
			}
			else
			{
				kept.m_iSlot = -1;
			}
		}

		bool clinicTown = town == RZ_WorldFacts.Get().CanonSettlement("origin.clinic");
		foreach (RZ_MarkedDoor d : here)
		{
			if (d.m_iSlot >= 0)
				continue;

			int pick = PickSlot(slots, d, used, clinicTown);
			if (pick < 0)
				continue;

			used.Insert(pick);
			d.m_iSlot = pick;
			Place(d, slots.m_aSlots[pick]);
			m_Traces.DoorChanged(d);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Place(RZ_MarkedDoor d, RZ_DoorSlot s)
	{
		d.m_bPlaced = true;
		d.m_vPos = s.m_vPos;
		d.m_vNormal = s.m_vNormal;
		d.m_bLocked = s.m_bLocked;
		d.m_iKind = s.m_iKind;
	}

	//------------------------------------------------------------------------------------------------
	protected int PickSlot(RZ_TownSlots slots, RZ_MarkedDoor d, array<int> used, bool clinicTown)
	{
		int best = -1;
		float bestScore = float.MAX;
		foreach (int i, RZ_DoorSlot s : slots.m_aSlots)
		{
			if (used.Contains(i) || s.m_bOff)
				continue;

			// The first patient's clinic carries nothing: no door there can be read as a clue
			if (clinicTown && s.m_iKind == RZ_EDoorWant.CLINIC)
				continue;

			float score = i;
			if (d.m_vAnchor != vector.Zero)
				score = RZ_Util.DistXZ(s.m_vPos, d.m_vAnchor);

			if (d.m_iWant == RZ_EDoorWant.NEAR)
			{
				if (d.m_vAnchor != vector.Zero && score > 60)
					score += 10000;
			}
			else if (d.m_iWant != RZ_EDoorWant.ANY && s.m_iKind != d.m_iWant)
			{
				// Not the kind asked for: a house next, anything else last
				if (s.m_iKind == RZ_EDoorWant.HOUSE)
					score += 5000;
				else
					score += 20000;
			}

			if (score < bestScore)
			{
				bestScore = score;
				best = i;
			}
		}

		return best;
	}

	// ================================================================ Errands' old mark
	//------------------------------------------------------------------------------------------------
	//! Errands' found hook "an old mark" (decision 15): a marked door within 60 m of `at`, else a
	//! two-hour chalk or stencil written in the grammar by whoever runs near there, on the nearest
	//! door if the town's doors are known, else standing alone. Its key for OldMarkLine; "" if none.
	string OldMarkKey(vector at)
	{
		foreach (RZ_MarkedDoor d : m_aDoors)
		{
			if (d.IsBound() && !d.m_aLayers.IsEmpty() && RZ_Util.DistXZ(d.m_vPos, at) <= OLD_MARK_RADIUS)
				return d.m_sKey;
		}

		int town = m_GM.FindNearestSettlement(at);
		RZ_Settlement s = m_GM.GetSettlement(town);
		if (!s || RZ_Util.DistXZ(s.m_vCenter, at) > s.m_fRadius + 150)
			return "";

		RZ_DoorMark m = OldMarkLayer(at);
		m.m_iFlags = m.m_iFlags | RZ_DoorMark.F_TEMP;
		m.m_iUntil = RZ_SeasonClock.Now() + OLD_MARK_SECONDS;
		RZ_MarkedDoor d2 = Write(town, RZ_EDoorWant.NEAR, at, m, "an old mark");
		if (!d2)
			return "";

		return d2.m_sKey;
	}

	//------------------------------------------------------------------------------------------------
	//! A few days old, by whoever holds the nearest ground within 3 km (the Haven otherwise).
	protected RZ_DoorMark OldMarkLayer(vector at)
	{
		int day = Math.Max(11, RZ_WorldFacts.Get().Day() - Math.RandomInt(2, 9));
		int faction = RZ_EFaction.SURVIVORS;
		float bestDist = 3000;
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			foreach (RZ_Holding h : territory.GetHoldings())
			{
				float dist = RZ_Util.DistXZ(h.m_vPos, at);
				if (dist < bestDist)
				{
					bestDist = dist;
					faction = h.m_eFaction;
				}
			}
		}

		switch (faction)
		{
			case RZ_EFaction.MILITARY: return RZ_DoorMark.Make(RZ_EDoorSymbol.CLEARED, day, Math.RandomInt(0, 24) * 100);
			case RZ_EFaction.BANDITS: return RZ_DoorMark.Make(RZ_EDoorSymbol.HAND, day, Math.RandomInt(1, 6), 0, 0, RZ_DoorMark.F_STRUCK);
			case RZ_EFaction.INSTITUTE: return RZ_DoorMark.Make(RZ_EDoorSymbol.BEAK, day, Math.RandomInt(20, 140), Math.RandomInt(1, 4));
		}

		return RZ_DoorMark.Make(RZ_EDoorSymbol.RING_DOT, day);
	}

	// ================================================================ Save
	//------------------------------------------------------------------------------------------------
	void Save(notnull array<ref RZ_MarkedDoorSave> result)
	{
		foreach (RZ_MarkedDoor d : m_aDoors)
		{
			if (!d.HasLive())
				continue;

			RZ_MarkedDoorSave ds = new RZ_MarkedDoorSave();
			ds.key = d.m_sKey;
			ds.town = d.m_iTown;
			ds.slot = d.m_iSlot;
			ds.want = d.m_iWant;
			ds.x = d.m_vAnchor[0];
			ds.z = d.m_vAnchor[2];
			foreach (RZ_DoorMark m : d.m_aLayers)
			{
				if (m.IsSeeded() || m.Has(RZ_DoorMark.F_TEMP))
					continue;

				RZ_DoorMarkSave ms = new RZ_DoorMarkSave();
				ms.symbol = m.m_iSymbol;
				ms.day = m.m_iDay;
				ms.a = m.m_iA;
				ms.b = m.m_iB;
				ms.c = m.m_iC;
				ms.flags = m.m_iFlags;
				ms.made = m.m_fMade;
				ms.rain = m.m_fRain;
				ms.text = m.m_sText;
				ds.layers.Insert(ms);
			}

			result.Insert(ds);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Before the first days are seeded: live doors come back with their slots, and the seeded
	//! layers go under them when the first days find the same door key.
	void Restore(array<ref RZ_MarkedDoorSave> saved, int nextKey)
	{
		m_iNextKey = Math.Max(1, nextKey);
		if (!saved)
			return;

		foreach (RZ_MarkedDoorSave ds : saved)
		{
			if (!ds || !m_GM.GetSettlement(ds.town))
				continue;

			RZ_MarkedDoor d = new RZ_MarkedDoor();
			d.m_sKey = ds.key;
			d.m_iTown = ds.town;
			d.m_iSlot = ds.slot;
			d.m_iWant = ds.want;
			if (ds.x != 0 || ds.z != 0)
				d.m_vAnchor = Vector(ds.x, 0, ds.z);

			foreach (RZ_DoorMarkSave ms : ds.layers)
			{
				RZ_DoorMark m = RZ_DoorMark.Make(ms.symbol, ms.day, ms.a, ms.b, ms.c, ms.flags);
				m.m_fMade = ms.made;
				m.m_fRain = ms.rain;
				m.m_sText = ms.text;
				d.m_aLayers.Insert(m);
			}

			if (!d.m_aLayers.IsEmpty())
				m_aDoors.Insert(d);
		}
	}

	//------------------------------------------------------------------------------------------------
	int NextKey()
	{
		return m_iNextKey;
	}

	//------------------------------------------------------------------------------------------------
	//! A seeded layer for the first-days door `key`: under any live layers already on it.
	void Seed(int town, int want, vector anchor, RZ_DoorMark m, string key)
	{
		m.m_iFlags = m.m_iFlags | RZ_DoorMark.F_SEEDED;
		RZ_MarkedDoor d = FindByKey(key);
		if (d)
		{
			if (!TownAllows(town, m))
				return;

			// History goes underneath; the cap keeps the newest
			d.m_aLayers.InsertAt(m, 0);
			int cap = Math.Max(1, m_Traces.GetSettings().m_iLayersPerDoor);
			while (d.m_aLayers.Count() > cap)
			{
				d.m_aLayers.RemoveOrdered(0);
			}

			return;
		}

		Write(town, want, anchor, m, "first days", key);
	}

	// ================================================================ Names for the log
	//------------------------------------------------------------------------------------------------
	string TownName(int town)
	{
		RZ_Settlement s = m_GM.GetSettlement(town);
		if (!s)
			return "?";

		return s.m_sName;
	}

	//------------------------------------------------------------------------------------------------
	static string AuthorKey(int author)
	{
		switch (author)
		{
			case RZ_EDoorAuthor.BASTION: return "bastion";
			case RZ_EDoorAuthor.HAVEN: return "haven";
			case RZ_EDoorAuthor.REDHAND: return "redhand";
			case RZ_EDoorAuthor.HALCYON: return "halcyon";
			case RZ_EDoorAuthor.INFECTED: return "infected";
		}

		return "residents";
	}
}
