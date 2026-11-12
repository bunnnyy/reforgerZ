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
//!   a player's death (graves and candles, RZ_PlayerTraces; a cross at the line, N1), a building
//!   burned (scorch, L2), a supply truck lost (its wreck, L5; a Halcyon run, V5).
//! - Scenes from missions: a failed rescue (V2), a failed Halcyon run (V5), a walk-out (V3).
//! - Watched: a Bastion outpost lost (V7), red hands spreading (L7), the young come home (L4), the
//!   thumbprint where an exile sleeps.
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
		if (!t || !m || m.m_bGlobal || m.m_vPos == vector.Zero)
			return;

		int town = TownAt(m.m_vPos, m.m_iSettlement);
		string title = m.m_sTitle;
		if (!success)
		{
			Failed(t, m, town);
			return;
		}

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
			// V3: the chair at the edge of the village he was walked out of, only when he walked on
			// (Walk Him Out's record walk.out: "walked on", not "ended")
			RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
			RZ_WorldRecord walk = RZ_WorldFacts.Get().Memory().NewestRecord("walk.out", 0.1);
			if (gm && gm.GetSafehouses() && walk && walk.value == "walked on")
			{
				int house = gm.GetSafehouses().FindNearest(m.m_vPos, 3000);
				if (house >= 0)
					t.GetScenes().Chair(house, "a walk-out");
			}

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
	//! A job that failed: a rescue that didn't get them out leaves a house that held out (V2); a
	//! Halcyon run lost on the road leaves its case in the ditch (V5).
	protected static void Failed(RZ_Traces t, RZ_Mission m, int town)
	{
		string title = m.m_sTitle;
		if (TitleIs(title, {"Survivors Trapped", "Collapse Shelter"}))
		{
			t.GetScenes().HeldOut(town, m.m_vPos, title + " failed");
			return;
		}

		if (TitleIs(title, {"Field Samples", "Night Readings", "Live Capture", "Specimen from the Nest", "Cold Chain", "Escort the Scientist"}))
			t.GetScenes().DroppedRun(m.m_vPos, title + " failed");
	}

	//------------------------------------------------------------------------------------------------
	//! A building burned (RZ_WorldWriters.Fire): scorch at its doorway (L2).
	static void Fire(vector pos)
	{
		RZ_Traces t = Traces();
		if (t)
			t.GetScenes().Fire(pos);
	}

	//------------------------------------------------------------------------------------------------
	//! A supply truck was lost (RZ_SupplyLines.Fail): its wreck on the verge (L5); a Halcyon truck
	//! is also a dropped run (V5). why: "robbed", "taken" (a pack), "destroyed", "late".
	static void TruckLost(int faction, vector pos, string why, string dest)
	{
		RZ_Traces t = Traces();
		if (!t || pos == vector.Zero || why == "late")
			return;

		int how = 0;
		if (why == "robbed")
			how = 1;
		else if (why == "taken")
			how = 2;

		t.GetScenes().Truck(faction, pos, how, dest);
		if (faction == RZ_EFaction.INSTITUTE)
			t.GetScenes().DroppedRun(pos, "a Halcyon truck lost");
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
		// V2: a barricade a player put up is still standing in a town that fell
		RZ_TownMemory memory = gm.GetTownMemory();
		if (memory && memory.IsEnabled() && memory.HasFlag(town, RZ_TownMemory.FLAG_BARRICADED))
			t.GetScenes().HeldOut(town, s.m_vCenter, "town infested, a barricade standing");

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
		t.GetScenes().TownDark(town);
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
		if (!t)
			return;

		t.GetScenes().LineDeath(pos);
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
	protected ref map<int, vector> m_mHoldingPos = new map<int, vector>();
	protected ref map<int, int> m_mDied = new map<int, int>();		// safehouse -> its dead, last look
	protected ref map<int, bool> m_mDark = new map<int, bool>();		// town -> 85 or more, last look
	protected bool m_bLooked;
	protected int m_iSpreadDay = -1;		// L7: the season day red hands last spread
	protected ref set<string> m_aYoungDone = new set<string>();	// "tribe|day": the young came home tonight
	protected ref map<string, float> m_mExileAt = new map<string, float>();	// "ident|town" -> seconds seen
	protected ref map<string, vector> m_mExilePos = new map<string, vector>();
	protected ref map<string, int> m_mExileDue = new map<string, int>();	// "ident|town" -> real time to write
	protected ref set<string> m_aExileDone = new set<string>();	// "ident|day"
	protected ref set<string> m_aStudied = new set<string>();		// "study|mission": L8 at a nest under study

	protected static const int THUMB_SECONDS = 1200;	// an exile in a town nobody holds this long: they sleep there
	protected static const int THUMB_DELAY = 7200;		// written 2 real hours later, never where they are now

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
		if (first || RZ_WorldFacts.Get().IsCured() || !m_Traces.GetSettings().m_bLiveTraces)
			return;

		Spread();
		YoungHome();
		Exiles();
		NestsUnderStudy();
	}

	//------------------------------------------------------------------------------------------------
	//! L8 at a nest under study: while Halcyon's Specimen from the Nest runs (its study list is the
	//! running jobs), a ringed beak goes on the nearest door to the nest, once a job: a nest Halcyon
	//! wants left.
	protected void NestsUnderStudy()
	{
		RZ_MissionManager missions = m_GM.GetMissions();
		if (!missions)
			return;

		foreach (RZ_Mission m : missions.GetActive())
		{
			if (!m || m.m_eState != RZ_EMissionState.ACTIVE || m.m_bGlobal || m.m_sTitle.IndexOf("Specimen from the Nest") != 0)
				continue;

			string key = string.Format("study|%1", m.m_iId);
			if (m_aStudied.Contains(key))
				continue;

			m_aStudied.Insert(key);
			int town = RZ_TraceWriters.TownAt(m.m_vPos, m.m_iSettlement);
			if (town < 0)
				continue;

			int day = RZ_WorldFacts.Get().Day();
			int series = 20 + (town * 37 + day * 11) % 180;
			m_Traces.GetDoors().Write(town, RZ_EDoorWant.NEAR, m.m_vPos, RZ_DoorMark.Make(RZ_EDoorSymbol.BEAK, day, series, 1 + day % 3, 0, RZ_DoorMark.F_RING), "a nest under study");
		}
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
			m_mHoldingPos.Set(h.m_iId, h.m_vPos);
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
			vector where;
			m_mHoldingPos.Find(lost, where);
			m_mHoldings.Remove(lost);
			m_mHoldingTown.Remove(lost);
			m_mHoldingPos.Remove(lost);
			if (!first && town >= 0)
				m_Traces.History().Add(town, "lost", loser, 0, vector.Zero);

			// V7: the roll at a fallen Bastion post
			if (!first && loser == RZ_EFaction.MILITARY && where != vector.Zero)
				m_Traces.GetScenes().FallenPost(where, town);
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
		if (loser == RZ_EFaction.MILITARY && !h.m_bStronghold)
			m_Traces.GetScenes().FallenPost(h.m_vPos, town);

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
	//! L7, once a season day: with the Red Hand's strength 50 or more, a hand on one more door (shops
	//! and garages first) in a town within 1.5 km of a crew's holding or road post; under 35, a hand
	//! struck through (nobody's collecting).
	protected void Spread()
	{
		int today = RZ_SeasonClock.Day();
		RZ_Territory territory = m_GM.GetTerritory();
		if (today == m_iSpreadDay || !territory || !territory.IsActive())
			return;

		m_iSpreadDay = today;
		float strength = territory.GetStrength(RZ_EFaction.BANDITS);
		RZ_DoorMarks doors = m_Traces.GetDoors();
		int day = RZ_WorldFacts.Get().Day();
		if (strength < 35)
		{
			foreach (RZ_MarkedDoor d : doors.GetDoors())
			{
				RZ_DoorMark hand = d.Find(RZ_EDoorSymbol.HAND);
				if (hand && !hand.Has(RZ_DoorMark.F_STRUCK) && !hand.Has(RZ_DoorMark.F_OVER) && !RZ_WorldFacts.Get().IsLivedIn(d.m_iTown))
				{
					doors.Write(d.m_iTown, d.m_iWant, d.m_vAnchor, RZ_DoorMark.Make(RZ_EDoorSymbol.HAND, day, hand.m_iA, 0, 0, RZ_DoorMark.F_STRUCK), "the crews stopped collecting", d.m_sKey);
					return;
				}
			}

			return;
		}

		if (strength < 50)
			return;

		array<vector> crews = {};
		foreach (RZ_Holding h : territory.GetHoldings())
		{
			if (h.m_eFaction == RZ_EFaction.BANDITS)
				crews.Insert(h.m_vPos);
		}

		RZ_Checkpoints checkpoints = m_GM.GetCheckpoints();
		if (checkpoints)
		{
			array<RZ_RoadPost> posts = {};
			checkpoints.PostsNear(RZ_EFaction.BANDITS, vector.Zero, float.MAX, posts);
			foreach (RZ_RoadPost p : posts)
			{
				crews.Insert(p.m_vPos);
			}
		}

		RZ_Rng rng = new RZ_Rng(RZ_WorldFacts.Get().GetSeed() + today * 271);
		array<int> towns = {};
		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			if (RZ_WorldFacts.Get().IsLivedIn(i))
				continue;

			foreach (vector c : crews)
			{
				if (RZ_Util.DistXZ(s.m_vCenter, c) <= 1500 + s.m_fRadius)
				{
					towns.Insert(i);
					break;
				}
			}
		}

		if (towns.IsEmpty())
			return;

		int town = towns[rng.Int(0, towns.Count())];
		int want = RZ_EDoorWant.SHOP;
		if (rng.Chance(0.4))
			want = RZ_EDoorWant.GARAGE;

		doors.Write(town, want, vector.Zero, RZ_DoorMark.Make(RZ_EDoorSymbol.HAND, day, rng.Int(1, 4)), "red hands spreading");
	}

	//------------------------------------------------------------------------------------------------
	//! L4, at dusk, 3 evenings in 10 a pack: one of its named young from a town nobody lives in, its
	//! range within 1.5 km, leaves fresh scratches outside a ringed bear there (a season day, never
	//! saved). Reading them makes the pack Heard of.
	protected void YoungHome()
	{
		RZ_Tribes tribes = m_GM.GetTribes();
		if (!tribes || !RZ_WorldFacts.Get().IsDusk())
			return;

		int today = RZ_SeasonClock.Day();
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			if (!t || t.IsRover() || t.m_aNamed.IsEmpty())
				continue;

			string key = string.Format("%1|%2", t.m_iId, today);
			if (m_aYoungDone.Contains(key))
				continue;

			m_aYoungDone.Insert(key);
			RZ_Rng rng = new RZ_Rng(facts.GetSeed() + t.m_iId * 977 + today * 31);
			if (!rng.Chance(0.3))
				continue;

			RZ_NamedYoung y = t.m_aNamed[rng.Int(0, t.m_aNamed.Count())];
			int town = m_GM.FindNearestSettlement(y.m_vAt);
			RZ_Settlement s = m_GM.GetSettlement(town);
			if (!s || facts.IsLivedIn(town) || RZ_Util.DistXZ(s.m_vCenter, t.m_vLair) > t.m_fRadius + 1500 + s.m_fRadius)
				continue;

			array<RZ_MarkedDoor> here = {};
			m_Traces.GetDoors().TownDoors(town, here);
			foreach (RZ_MarkedDoor d : here)
			{
				// Room beside the bear: the scratches never push it (or anything) off the door
				RZ_DoorMark bear = d.Find(RZ_EDoorSymbol.BEAR);
				if (!bear || !bear.Has(RZ_DoorMark.F_RING) || facts.IsNearLivedIn(d.m_vPos, 60) || d.m_aLayers.Count() >= m_Traces.GetSettings().m_iLayersPerDoor)
					continue;

				RZ_DoorMark m = RZ_DoorMark.Make(RZ_EDoorSymbol.SCRATCH_OUT, facts.Day(), 1, 0, t.m_iId + 1, RZ_DoorMark.F_YOUNG | RZ_DoorMark.F_TEMP);
				m.m_iUntil = RZ_SeasonClock.Now() + RZ_Util.RoundToInt(RZ_SeasonClock.DayMinutes() * 60);
				m_Traces.GetDoors().Write(town, d.m_iWant, d.m_vAnchor, m, "the young come home", d.m_sKey);
				RZ_Log.Info(string.Format("Traces: the young come home to %1#%2 (%3)", s.m_sName, d.m_iSlot, t.m_sPlace));
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The thumbprint: an exile (Hunted by the Haven) who stays 20 minutes in a town nobody holds
	//! sleeps there; 2 real hours later, if they aren't within 300 m, the crews' small red print goes
	//! low on a house's frame there. One a season day an exile. Only an Ex-Con reads it.
	protected void Exiles()
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses)
			return;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		foreach (int i, IEntity p : players)
		{
			if (!p || !houses.IsExiled(ids[i]))
				continue;

			RZ_PlayerProfile profile = m_GM.GetProfile(ids[i]);
			int town = RZ_TraceWriters.TownAt(p.GetOrigin());
			if (!profile || town < 0 || facts.IsLivedIn(town))
				continue;

			string key = profile.m_sIdentity + "|" + town.ToString();
			float seen;
			m_mExileAt.Find(key, seen);
			seen += 30;
			m_mExileAt.Set(key, seen);
			m_mExilePos.Set(key, p.GetOrigin());
			if (seen >= THUMB_SECONDS && !m_mExileDue.Contains(key))
				m_mExileDue.Set(key, RZ_SeasonClock.Now() + THUMB_DELAY);
		}

		array<string> written = {};
		foreach (string due, int at : m_mExileDue)
		{
			if (RZ_SeasonClock.Now() < at)
				continue;

			vector pos;
			m_mExilePos.Find(due, pos);
			if (RZ_Util.NearestDistance(pos, players) <= 300)
				continue;	// never where someone is now

			written.Insert(due);
			array<string> parts = {};
			due.Split("|", parts, false);
			string done = parts[0] + "|" + RZ_SeasonClock.Day().ToString();
			if (parts.Count() < 2 || m_aExileDone.Contains(done))
				continue;

			m_aExileDone.Insert(done);
			int where = parts[1].ToInt();
			m_Traces.GetDoors().Write(where, RZ_EDoorWant.HOUSE, pos, RZ_DoorMark.Make(RZ_EDoorSymbol.THUMB, facts.Day()), "an exile sleeps here");
		}

		foreach (string w : written)
		{
			m_mExileDue.Remove(w);
			m_mExileAt.Remove(w);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The thumbprint's count across a restart: each exile's minutes in a town, and when it's due.
	void SaveExiles(notnull array<ref RZ_ExileSave> result)
	{
		foreach (string key, float seen : m_mExileAt)
		{
			RZ_ExileSave e = new RZ_ExileSave();
			e.key = key;
			e.seen = seen;
			vector pos;
			m_mExilePos.Find(key, pos);
			e.x = pos[0];
			e.z = pos[2];
			int due;
			m_mExileDue.Find(key, due);
			e.due = due;
			result.Insert(e);
		}
	}

	//------------------------------------------------------------------------------------------------
	void RestoreExiles(array<ref RZ_ExileSave> saved)
	{
		if (!saved)
			return;

		foreach (RZ_ExileSave e : saved)
		{
			m_mExileAt.Set(e.key, e.seen);
			m_mExilePos.Set(e.key, RZ_Util.GroundPos(Vector(e.x, 0, e.z)));
			if (e.due > 0)
				m_mExileDue.Set(e.key, e.due);
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
