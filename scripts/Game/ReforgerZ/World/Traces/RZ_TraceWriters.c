//! Traces' writers (docs/TRACES.md, "Generation"): what happened, told to Traces by the systems that
//! own the fact, each event into the town's history and, through the grammar's table, maybe one
//! door. The same pattern as RZ_WorldWriters: one-line static hooks, most of them called from the
//! world memory's own writers, so no other system keeps Traces' rules.
//!
//! - Missions, by title: rescues (ticked bears), Walk Him Out (hatching), the crews' rackets and
//!   debts (hands, tallies, PAID), requisitions (C), culls (CLEARED), Halcyon's samples (beaks),
//!   bodies burned (ash, a player trace).
//! - Watched every 30 s (RZ_TraceWatch): holdings founded, taken and lost (hands, post codes, grey
//!   paint), a safehouse's dead going up (candles), a town going dark (scratches inside).
//! - Hooks: a town turned infested (scratches, smears), a safehouse emptied (its bears ringed),
//!   a player's death (graves and candles, RZ_PlayerTraces).
//! Server only.
class RZ_TraceWriters
{
	//------------------------------------------------------------------------------------------------
	protected static RZ_Traces Traces()
	{
		RZ_Traces t = RZ_Traces.Get();
		if (!t || !t.IsOn())
			return null;

		return t;
	}

	//------------------------------------------------------------------------------------------------
	//! The town a thing happened in: its own, else the nearest within 1.5 km. -1 if none.
	static int TownAt(vector pos, int town = -1)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return -1;

		if (town >= 0)
			return town;

		int nearest = gm.FindNearestSettlement(pos);
		RZ_Settlement s = gm.GetSettlement(nearest);
		if (!s || RZ_Util.DistXZ(s.m_vCenter, pos) > 1500)
			return -1;

		return nearest;
	}

	//------------------------------------------------------------------------------------------------
	protected static int Today()
	{
		return RZ_WorldFacts.Get().Day();
	}

	//------------------------------------------------------------------------------------------------
	//! The town of the safehouse nearest pos, + 1 (an arrow's field); 0 if none.
	protected static int HouseArrow(vector pos, int skipTown = -1)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetSafehouses())
			return 0;

		RZ_Safehouses houses = gm.GetSafehouses();
		int best = -1;
		float bestDist = float.MAX;
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			if (!h || h.m_iPopulation <= 0 || h.m_iSettlement == skipTown)
				continue;

			float d = RZ_Util.DistXZ(h.m_vPos, pos);
			if (d < bestDist)
			{
				bestDist = d;
				best = h.m_iSettlement;
			}
		}

		return best + 1;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool TitleIs(string title, array<string> names)
	{
		foreach (string n : names)
		{
			if (title.IndexOf(n) == 0)
				return true;
		}

		return false;
	}

	// ================================================================ Missions
	//------------------------------------------------------------------------------------------------
	//! A mission ended (RZ_WorldWriters.MissionEnded). Only what was done writes: the grammar's
	//! table by mission, read from its title.
	static void MissionEnded(RZ_Mission m, bool success)
	{
		RZ_Traces t = Traces();
		if (!t || !m || !success || m.m_bGlobal || m.m_vPos == vector.Zero)
			return;

		int town = TownAt(m.m_vPos, m.m_iSettlement);
		string title = m.m_sTitle;
		RZ_DoorMarks doors = t.GetDoors();
		int day = Today();
		if (TitleIs(title, {"Survivors Trapped", "Collapse Shelter", "Rescue the Kids", "Fetch the Family"}))
		{
			// P1: the bear ticked on the house they came out of, an arrow to the house they went to
			if (town < 0)
				return;

			t.History().Add(town, "rescue", RZ_EFaction.SURVIVORS, 1, m.m_vPos);
			doors.Write(town, RZ_EDoorWant.NEAR, m.m_vPos, RZ_DoorMark.Make(RZ_EDoorSymbol.BEAR, day, HouseArrow(m.m_vPos), 0, 0, RZ_DoorMark.F_TICK), title);
			doors.Write(town, RZ_EDoorWant.HOUSE, m.m_vPos, RZ_DoorMark.Make(RZ_EDoorSymbol.RING_DOT, day), title);
			return;
		}

		if (TitleIs(title, {"Walk Him Out"}))
		{
			if (town < 0)
				return;

			t.History().Add(town, "walkout", RZ_EFaction.SURVIVORS, 1, m.m_vPos);
			doors.Write(town, RZ_EDoorWant.HOUSE, m.m_vPos, RZ_DoorMark.Make(RZ_EDoorSymbol.HATCH, day), title);
			return;
		}

		if (TitleIs(title, {"Protection Racket", "A Quiet Arrangement", "Rent's Due"}))
		{
			if (town < 0)
				return;

			t.History().Add(town, "racket", RZ_EFaction.BANDITS, 1, m.m_vPos);
			// Under the Haven's rule 4, the crews' small hand goes on a safehouse's door too
			RZ_TownOwner o = RZ_WorldFacts.Get().TownOwner(town);
			if (o.m_eKind == RZ_ETownKind.SAFEHOUSE && RZ_WorldFacts.Get().Flag("haven.rule") == 4)
			{
				RZ_HouseFacts f = RZ_WorldFacts.Get().House(o.m_iRef);
				doors.Write(town, RZ_EDoorWant.NEAR, f.m_vPos, RZ_DoorMark.Make(RZ_EDoorSymbol.HAND, day, 1), title, RZ_FirstDays.HouseDoorKey(o.m_iRef));
				return;
			}

			doors.Write(town, RZ_EDoorWant.SHOP, m.m_vPos, RZ_DoorMark.Make(RZ_EDoorSymbol.HAND, day, Strokes(doors, town, RZ_EDoorWant.SHOP) + 1), title);
			return;
		}

		if (TitleIs(title, {"Debt Collection", "A Debt Gone Quiet"}))
		{
			if (town < 0)
				return;

			t.History().Add(town, "debt", RZ_EFaction.BANDITS, 1, m.m_vPos);
			if (title.IndexOf("Debt Collection") == 0 && HasTally(doors, town))
				doors.Write(town, RZ_EDoorWant.SHOP, vector.Zero, RZ_DoorMark.Make(RZ_EDoorSymbol.TALLY, day, 0, 0, 0, RZ_DoorMark.F_PAID), title);
			else
				doors.Write(town, RZ_EDoorWant.HOUSE, m.m_vPos, RZ_DoorMark.Make(RZ_EDoorSymbol.HAND, day, 0, 0, 0, RZ_DoorMark.F_STRUCK), title);
			return;
		}

		if (TitleIs(title, {"Requisition", "Off-Book Requisition"}))
		{
			if (town < 0)
				return;

			t.History().Add(town, "requisition", RZ_EFaction.MILITARY, 1, m.m_vPos);
			doors.Write(town, RZ_EDoorWant.FARM, m.m_vPos, RZ_DoorMark.Make(RZ_EDoorSymbol.CONTRIB, day), title);
			return;
		}

		if (TitleIs(title, {"Bastion Shelling", "Fire Mission", "Retake ", "Take Back ", "Cull"}))
		{
			if (town < 0)
				return;

			t.History().Add(town, "cull", RZ_EFaction.MILITARY, 1, m.m_vPos);
			doors.Write(town, RZ_EDoorWant.NEAR, m.m_vPos, RZ_DoorMark.Make(RZ_EDoorSymbol.CLEARED, day, ClockNow()), title);
			return;
		}

		if (TitleIs(title, {"Field Samples", "Specimen from the Nest", "Night Readings", "Live Capture"}))
		{
			if (town < 0)
				return;

			t.History().Add(town, "samples", RZ_EFaction.INSTITUTE, 1, m.m_vPos);
			int flags = 0;
			if (title.IndexOf("Specimen from the Nest") == 0)
				flags = RZ_DoorMark.F_RING;	// a nest Halcyon wants left

			int series = 20 + (town * 37 + day * 11) % 180;
			doors.Write(town, RZ_EDoorWant.FARM, m.m_vPos, RZ_DoorMark.Make(RZ_EDoorSymbol.BEAK, day, series, 1 + day % 3, 0, flags), title);
			return;
		}

		if (TitleIs(title, {"Burn the Bodies", "Burn the Nest", "Clear the Trail"}))
		{
			// P2: ash where the bodies were burned
			if (town >= 0)
				t.History().Add(town, "pyre", -1, 1, m.m_vPos);

			t.GetPlayers().Pyre(m.m_vPos, m.m_iId);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The game clock as hhmm (0 if unknown).
	protected static int ClockNow()
	{
		float h = RZ_GameTime.GetHour();
		if (h < 0)
			return 0;

		int hour = RZ_Util.RoundToInt(Math.Floor(h));
		int minute = RZ_Util.RoundToInt(Math.Floor((h - hour) * 60));
		return hour * 100 + minute;
	}

	//------------------------------------------------------------------------------------------------
	protected static int Strokes(RZ_DoorMarks doors, int town, int want)
	{
		array<RZ_MarkedDoor> here = {};
		doors.TownDoors(town, here);
		foreach (RZ_MarkedDoor d : here)
		{
			RZ_DoorMark hand = d.Find(RZ_EDoorSymbol.HAND);
			if (d.m_iWant == want && hand && !hand.Has(RZ_DoorMark.F_STRUCK) && !hand.Has(RZ_DoorMark.F_OVER))
				return hand.m_iA;
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool HasTally(RZ_DoorMarks doors, int town)
	{
		array<RZ_MarkedDoor> here = {};
		doors.TownDoors(town, here);
		foreach (RZ_MarkedDoor d : here)
		{
			RZ_DoorMark tally = d.Find(RZ_EDoorSymbol.TALLY);
			if (tally && !tally.Has(RZ_DoorMark.F_PAID))
				return true;
		}

		return false;
	}

	// ================================================================ Hooks
	//------------------------------------------------------------------------------------------------
	//! A town went infested (RZ_WorldWriters.TownState): scratches outside on a door or two, smears
	//! at a sill.
	static void TownInfested(int town)
	{
		RZ_Traces t = Traces();
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!t || !gm || RZ_WorldFacts.Get().IsLivedIn(town))
			return;

		RZ_Settlement s = gm.GetSettlement(town);
		if (!s)
			return;

		// Once: the watch tells it again after every restart
		array<RZ_MarkedDoor> here = {};
		t.GetDoors().TownDoors(town, here);
		foreach (RZ_MarkedDoor d : here)
		{
			RZ_DoorMark scratches = d.Find(RZ_EDoorSymbol.SCRATCH_OUT);
			if (scratches && !scratches.IsSeeded())
				return;
		}

		t.History().Add(town, "infested", -1, 1, s.m_vCenter);
		int day = Today();
		t.GetDoors().Write(town, RZ_EDoorWant.HOUSE, vector.Zero, RZ_DoorMark.Make(RZ_EDoorSymbol.SCRATCH_OUT, day, Math.RandomInt(2, 7), Math.RandomInt(0, 2)), "town infested");
		if (s.m_fInfection >= 65)
			t.GetDoors().Write(town, RZ_EDoorWant.HOUSE, vector.Zero, RZ_DoorMark.Make(RZ_EDoorSymbol.SMEARS, day), "town infested");
	}

	//------------------------------------------------------------------------------------------------
	//! A town went dark (85): scratches inside one door that has residents' words and no bear.
	static void TownDark(int town)
	{
		RZ_Traces t = Traces();
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!t || !gm || RZ_WorldFacts.Get().IsLivedIn(town))
			return;

		RZ_Settlement s = gm.GetSettlement(town);
		if (!s)
			return;

		t.History().Add(town, "dark", -1, 1, s.m_vCenter);
		array<RZ_MarkedDoor> here = {};
		t.GetDoors().TownDoors(town, here);
		foreach (RZ_MarkedDoor d : here)
		{
			if (!d.Find(RZ_EDoorSymbol.WORDS) || d.Find(RZ_EDoorSymbol.BEAR) || d.Find(RZ_EDoorSymbol.PATCHED_BEAR))
				continue;

			// Never beside residents' words about a child
			RZ_DoorMark words = d.Find(RZ_EDoorSymbol.WORDS);
			if (words.m_iA == 1)
				continue;

			t.GetDoors().Write(town, d.m_iWant, d.m_vAnchor, RZ_DoorMark.Make(RZ_EDoorSymbol.SCRATCH_IN, Today()), "town gone dark", d.m_sKey);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A safehouse emptied (RZ_WorldWriters.HouseEmptied): its bears ringed, an arrow to the nearest
	//! house that still has people.
	static void HouseEmptied(int town)
	{
		RZ_Traces t = Traces();
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!t || !gm || !gm.GetSafehouses())
			return;

		RZ_Safehouses houses = gm.GetSafehouses();
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			if (!h || h.m_iSettlement != town)
				continue;

			t.History().Add(town, "emptied", RZ_EFaction.SURVIVORS, 0, h.m_vPos);
			int arrow = HouseArrow(h.m_vPos, town);
			t.GetDoors().Write(town, RZ_EDoorWant.NEAR, h.m_vPos, RZ_DoorMark.Make(RZ_EDoorSymbol.PATCHED_BEAR, Today(), arrow, 0, 0, RZ_DoorMark.F_RING), "house emptied", RZ_FirstDays.HouseDoorKey(i));
			// One more bear in town, ringed the same way
			array<RZ_MarkedDoor> here = {};
			t.GetDoors().TownDoors(town, here);
			foreach (RZ_MarkedDoor d : here)
			{
				RZ_DoorMark bear = d.Find(RZ_EDoorSymbol.BEAR);
				if (bear && !bear.Has(RZ_DoorMark.F_RING) && !bear.Has(RZ_DoorMark.F_TICK))
				{
					t.GetDoors().Write(town, d.m_iWant, d.m_vAnchor, RZ_DoorMark.Make(RZ_EDoorSymbol.BEAR, Today(), arrow, 0, 0, RZ_DoorMark.F_RING), "house emptied", d.m_sKey);
					break;
				}
			}

			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A player's character died (RZ_WorldWriters.Death): graves, candles, and the doors read go.
	static void Death(int playerId, vector pos)
	{
		RZ_Traces t = Traces();
		if (t)
			t.OnDeath(playerId, pos);
	}
}

//------------------------------------------------------------------------------------------------
//! What Traces watches every 30 s, for facts no hook tells it: holdings changing hands, a
//! safehouse's dead going up, a town going dark. The first look only learns what's there.
class RZ_TraceWatch
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_Traces m_Traces;
	protected ref map<int, int> m_mHoldings = new map<int, int>();	// holding id -> faction
	protected ref map<int, int> m_mHoldingTown = new map<int, int>();
	protected ref map<int, int> m_mDied = new map<int, int>();		// safehouse -> its dead, last look
	protected ref map<int, bool> m_mDark = new map<int, bool>();		// town -> 85 or more, last look
	protected bool m_bLooked;

	//------------------------------------------------------------------------------------------------
	void RZ_TraceWatch(RZ_GameModeComponent gm, RZ_Traces traces)
	{
		m_GM = gm;
		m_Traces = traces;
	}

	//------------------------------------------------------------------------------------------------
	void Look()
	{
		bool first = !m_bLooked;
		m_bLooked = true;
		Holdings(first);
		Houses(first);
		Towns(first);
	}

	//------------------------------------------------------------------------------------------------
	protected void Holdings(bool first)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		if (!territory || !territory.IsActive())
			return;

		array<int> seen = {};
		foreach (RZ_Holding h : territory.GetHoldings())
		{
			seen.Insert(h.m_iId);
			int was;
			bool known = m_mHoldings.Find(h.m_iId, was);
			m_mHoldings.Set(h.m_iId, h.m_eFaction);
			m_mHoldingTown.Set(h.m_iId, h.m_iSettlement);
			if (first || h.m_iSettlement < 0)
				continue;

			if (!known)
				Gained(h, -1);
			else if (was != h.m_eFaction)
				Gained(h, was);
		}

		array<int> gone = {};
		foreach (int id, int faction : m_mHoldings)
		{
			if (!seen.Contains(id))
				gone.Insert(id);
		}

		foreach (int lost : gone)
		{
			int town;
			m_mHoldingTown.Find(lost, town);
			int loser;
			m_mHoldings.Find(lost, loser);
			m_mHoldings.Remove(lost);
			m_mHoldingTown.Remove(lost);
			if (!first && town >= 0)
				m_Traces.History().Add(town, "lost", loser, 0, vector.Zero);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A holding founded or taken: the taker's marks on the town's doors, over the loser's.
	protected void Gained(RZ_Holding h, int loser)
	{
		int town = h.m_iSettlement;
		string kind = "founded";
		if (loser >= 0)
			kind = "taken";

		m_Traces.History().Add(town, kind, h.m_eFaction, 0, h.m_vPos);
		int day = RZ_WorldFacts.Get().Day();
		RZ_DoorMarks doors = m_Traces.GetDoors();
		string cause = string.Format("%1 %2", RZ_Factions.GetName(h.m_eFaction), kind);
		if (h.m_eFaction == RZ_EFaction.BANDITS)
		{
			// The shop first, over Bastion's stencils; never over a bear
			doors.Write(town, RZ_EDoorWant.SHOP, vector.Zero, RZ_DoorMark.Make(RZ_EDoorSymbol.HAND, day, 1), cause);
			doors.Write(town, RZ_EDoorWant.HOUSE, vector.Zero, RZ_DoorMark.Make(RZ_EDoorSymbol.HAND, day, 1), cause);
		}
		else if (h.m_eFaction == RZ_EFaction.MILITARY)
		{
			// Grey paint over the hands, then the post code
			doors.Write(town, RZ_EDoorWant.SHOP, vector.Zero, RZ_DoorMark.Make(RZ_EDoorSymbol.POSTCODE, day, h.m_iId), cause);
			doors.Write(town, RZ_EDoorWant.HOUSE, vector.Zero, RZ_DoorMark.Make(RZ_EDoorSymbol.POSTCODE, day, h.m_iId), cause);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A safehouse's dead went up: a candle on a house near it, or its number goes up.
	protected void Houses(bool first)
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses)
			return;

		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_HouseFacts f = facts.House(i);
			if (!f.m_bValid)
				continue;

			int was;
			bool known = m_mDied.Find(i, was);
			m_mDied.Set(i, f.m_iDied);
			if (first || !known || f.m_iDied <= was || f.m_iSettlement < 0)
				continue;

			int more = f.m_iDied - was;
			m_Traces.History().Add(f.m_iSettlement, "dead", RZ_EFaction.SURVIVORS, more, f.m_vPos);
			RZ_DoorMarks doors = m_Traces.GetDoors();
			int count = more;
			array<RZ_MarkedDoor> here = {};
			doors.TownDoors(f.m_iSettlement, here);
			RZ_DoorMark old = null;
			foreach (RZ_MarkedDoor d : here)
			{
				RZ_DoorMark c = d.Find(RZ_EDoorSymbol.CANDLE);
				if (c && !c.Has(RZ_DoorMark.F_SHOWN))
				{
					old = c;
					break;
				}
			}

			RZ_DoorMark candle = RZ_DoorMark.Make(RZ_EDoorSymbol.CANDLE, facts.Day(), count);
			if (old)
			{
				candle.m_iA = old.m_iA + more;
				candle.m_sText = old.m_sText;
			}

			// The names under it: seeded, one per death (the Elder reads them)
			RZ_Rng rng = new RZ_Rng(facts.GetSeed() + i * 131 + candle.m_iA * 17);
			for (int k = 0; k < more; k++)
			{
				if (!candle.m_sText.IsEmpty())
					candle.m_sText += ", ";

				candle.m_sText += RZ_NoteText.FirstName(rng);
			}

			doors.Write(f.m_iSettlement, RZ_EDoorWant.HOUSE, f.m_vPos, candle, "a house's dead");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A town nobody holds crossed 85: it went dark.
	protected void Towns(bool first)
	{
		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			bool dark = s.m_fInfection >= 85;
			bool was;
			bool known = m_mDark.Find(i, was);
			m_mDark.Set(i, dark);
			if (!first && known && dark && !was)
				RZ_TraceWriters.TownDark(i);
		}
	}
}
