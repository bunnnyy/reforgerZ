//! The first days (docs/TRACES.md, "Who writes on doors, and when"): Days 1-10 are history, seeded
//! once a season from its seed and the canon spots, the same after every restart and never saved.
//! The season starts on Day 11 (the broadcast's count); from then on marks are written live.
//!
//! - Bastion's sweep: towns within 4 km of the north road's checkpoint (Day 2, the living sent to
//!   the north road before 14:00, to the gym after, to the church from Day 4) and towns within 3 km
//!   of the road from the closed site to the cordon's roadblock (Day 1). At most 12 towns, 5 doors
//!   each. No X after Day 5; a slash about 1 X in 20 from Day 3; a ring (Day 5) only where the
//!   first sweep found a bitten person or a dead one.
//! - Residents' words on 1-2 doors in a third of towns.
//! - The Haven: the bear on the gym's doors (Day 2), bears round the gym and the church, the
//!   patched bear on every safehouse with candles, hatching and the drop at its pump.
//! - The Red Hand: tallies near the crossroads from Day 4, hands from Day 9, round every crew.
//! - Halcyon: beaks round the lab from Day 7. Bastion: post codes, ATT and C round its posts.
//! - The infected: scratches in towns already at 60 or more, smears at 65.
//! The new canon keys (rules here, made once a season): day2.gym, day4.church, day1.roadblock,
//! day2.queue, day4.hill (the last three are for the first days' scenes, planned).
//! Server only.

//------------------------------------------------------------------------------------------------
//! The Day 2 gym: the school within 3 km of the Haven's stronghold house, preferring one between
//! it and the north road; else the middle of the nearest town to the north road nobody holds.
class RZ_TraceCanonGym : RZ_CanonRule
{
	override string Make(RZ_GameModeComponent gm, RZ_WorldFacts facts, RZ_Rng rng)
	{
		vector home = RZ_FirstDays.HavenHome(gm);
		vector cp = facts.CanonPos("day2.checkpoint");
		if (home == vector.Zero)
			home = cp;

		if (home == vector.Zero)
			return "";

		array<ref RZ_Landmark> schools = {};
		RZ_LandmarkFinder.Find(home, 3000, {"school"}, schools);
		vector best = vector.Zero;
		float bestScore = float.MAX;
		foreach (RZ_Landmark l : schools)
		{
			if (RZ_Perimeter.IsNearLineAt(l.m_vPos))
				continue;

			float score = RZ_Util.DistXZ(home, l.m_vPos);
			if (cp != vector.Zero)
				score += RZ_Util.DistXZ(l.m_vPos, cp) * 0.5;

			if (score < bestScore)
			{
				bestScore = score;
				best = l.m_vPos;
			}
		}

		if (best != vector.Zero)
			return RZ_WorldFacts.PosText(best);

		int town = RZ_FirstDays.QuietTownNear(gm, facts, cp, -1);
		RZ_Settlement s = gm.GetSettlement(town);
		if (!s)
			return "";

		return RZ_WorldFacts.PosText(s.m_vCenter);
	}
}

//------------------------------------------------------------------------------------------------
//! The Day 4 church: a church in another town from the gym's, within 4 km of it, in a town nobody
//! holds; else any church within 6 km; else the middle of the gym's town (its town hall).
class RZ_TraceCanonChurch : RZ_CanonRule
{
	override string Make(RZ_GameModeComponent gm, RZ_WorldFacts facts, RZ_Rng rng)
	{
		vector gym = facts.CanonPos("day2.gym", new RZ_TraceCanonGym());
		if (gym == vector.Zero)
			return "";

		int gymTown = gm.FindNearestSettlement(gym);
		array<ref RZ_Landmark> churches = {};
		RZ_LandmarkFinder.Find(gym, 6000, {"church"}, churches);
		vector best = vector.Zero;
		float bestDist = float.MAX;
		vector anyChurch = vector.Zero;
		float anyDist = float.MAX;
		foreach (RZ_Landmark l : churches)
		{
			if (RZ_Perimeter.IsNearLineAt(l.m_vPos))
				continue;

			float d = RZ_Util.DistXZ(gym, l.m_vPos);
			int town = gm.FindNearestSettlement(l.m_vPos);
			if (d < anyDist)
			{
				anyDist = d;
				anyChurch = l.m_vPos;
			}

			if (d > 4000 || town == gymTown || facts.IsLivedIn(town))
				continue;

			if (d < bestDist)
			{
				bestDist = d;
				best = l.m_vPos;
			}
		}

		if (best == vector.Zero)
			best = anyChurch;

		if (best == vector.Zero)
		{
			RZ_Settlement s = gm.GetSettlement(gymTown);
			if (!s)
				return "";

			best = RZ_Mission.LandPointNear(s.m_vCenter, 0, 40);
		}

		return RZ_WorldFacts.PosText(best);
	}
}

//------------------------------------------------------------------------------------------------
//! The cordon's Day 1 roadblock: a road point 1.5-3 km from the closed site, toward the lab's town,
//! off the line.
class RZ_TraceCanonRoadblock : RZ_CanonRule
{
	override string Make(RZ_GameModeComponent gm, RZ_WorldFacts facts, RZ_Rng rng)
	{
		vector site = facts.CanonPos("origin.site");
		if (site == vector.Zero)
			return "";

		vector toward = gm.GetLabPosition();
		array<vector> points = {};
		RZ_Perimeter.RoadPointsNear(site, 3000, points);
		vector best = vector.Zero;
		float bestScore = float.MAX;
		foreach (vector p : points)
		{
			float d = RZ_Util.DistXZ(p, site);
			if (d < 1500 || d > 3000 || RZ_Perimeter.IsNearLineAt(p))
				continue;

			float score = rng.Range(0, 300);
			if (toward != vector.Zero)
				score += RZ_Util.DistXZ(p, toward);

			if (score < bestScore)
			{
				bestScore = score;
				best = p;
			}
		}

		if (best == vector.Zero)
			best = RZ_Mission.LandPointNear(site, 1500, 3000);

		return RZ_WorldFacts.PosText(best);
	}
}

//------------------------------------------------------------------------------------------------
//! The column's queue: the road point about 400 m back from the north road's checkpoint, inward
//! (toward the Haven's stronghold house).
class RZ_TraceCanonQueue : RZ_CanonRule
{
	override string Make(RZ_GameModeComponent gm, RZ_WorldFacts facts, RZ_Rng rng)
	{
		vector cp = facts.CanonPos("day2.checkpoint");
		if (cp == vector.Zero)
			return "";

		vector home = RZ_FirstDays.HavenHome(gm);
		array<vector> points = {};
		RZ_Perimeter.RoadPointsNear(cp, 600, points);
		vector best = vector.Zero;
		float bestScore = float.MAX;
		foreach (vector p : points)
		{
			float d = RZ_Util.DistXZ(p, cp);
			if (d < 300 || d > 500)
				continue;

			float score = Math.AbsFloat(d - 400);
			if (home != vector.Zero)
				score += RZ_Util.DistXZ(p, home) * 0.1;

			if (score < bestScore)
			{
				bestScore = score;
				best = p;
			}
		}

		if (best == vector.Zero)
			return "";

		return RZ_WorldFacts.PosText(best);
	}
}

//------------------------------------------------------------------------------------------------
//! The hill over the north road: high ground 500-900 m from the checkpoint, off the line, with the
//! terrain clear between it and the barrier. "none" when no point qualifies.
class RZ_TraceCanonHill : RZ_CanonRule
{
	override string Make(RZ_GameModeComponent gm, RZ_WorldFacts facts, RZ_Rng rng)
	{
		vector cp = facts.CanonPos("day2.checkpoint");
		if (cp == vector.Zero)
			return "none";

		for (int i = 0; i < 12; i++)
		{
			vector p = RZ_Mission.HighPointNear(cp, 500, 900, 12);
			if (RZ_Perimeter.IsNearLineAt(p) || RZ_Util.IsWater(p))
				continue;

			if (RZ_FirstDays.TerrainClear(p, cp))
				return RZ_WorldFacts.PosText(p);
		}

		return "none";
	}
}

//------------------------------------------------------------------------------------------------
//! The season's start, kept as canon so the first days come out the same after every restart
//! whatever has changed since: ";3;7;" lists of towns. Mode 0: towns at 60+ infection; 1: towns
//! Bastion held; 2: towns the Red Hand held; 3: safehouses at Community or bigger.
class RZ_TraceCanonStart : RZ_CanonRule
{
	protected int m_iMode;

	//------------------------------------------------------------------------------------------------
	void RZ_TraceCanonStart(int mode)
	{
		m_iMode = mode;
	}

	//------------------------------------------------------------------------------------------------
	override string Make(RZ_GameModeComponent gm, RZ_WorldFacts facts, RZ_Rng rng)
	{
		string list = ";";
		if (m_iMode == 3)
		{
			RZ_Safehouses houses = gm.GetSafehouses();
			if (houses)
			{
				for (int h = 0; h < houses.Count(); h++)
				{
					if (facts.House(h).m_sTier != "Shelter")
						list += h.ToString() + ";";
				}
			}

			return list;
		}

		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			bool take = false;
			if (m_iMode == 0)
				take = s.m_fInfection >= 60 && !facts.IsLivedIn(i);
			else if (m_iMode == 1)
				take = facts.TownOwner(i).IsHeldBy(RZ_EFaction.MILITARY);
			else
				take = facts.TownOwner(i).IsHeldBy(RZ_EFaction.BANDITS);

			if (take)
				list += i.ToString() + ";";
		}

		return list;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_FirstDays
{
	protected static const int MAX_SWEPT = 12;
	protected static const int MAX_X = 5;

	protected RZ_GameModeComponent m_GM;
	protected RZ_DoorMarks m_Doors;
	protected ref map<int, int> m_mNext = new map<int, int>();	// town * 10 + stage -> next door number
	protected int m_iStage;			// which part of the seeding writes now (its own door keys)
	protected string m_sInfected;	// the season's start, as canon (RZ_TraceCanonStart)
	protected string m_sBastion;
	protected string m_sCrews;
	protected string m_sBigHouses;

	//------------------------------------------------------------------------------------------------
	void RZ_FirstDays(RZ_GameModeComponent gm, RZ_DoorMarks doors)
	{
		m_GM = gm;
		m_Doors = doors;
	}

	//------------------------------------------------------------------------------------------------
	//! The Haven's stronghold house (the chair's), else the first safehouse; zero without one.
	static vector HavenHome(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses || houses.Count() == 0)
			return vector.Zero;

		int chair = RZ_HavenQuest.ChairHouse(gm);
		if (chair >= 0 && houses.Get(chair))
			return houses.Get(chair).m_vPos;

		return houses.Get(0).m_vPos;
	}

	//------------------------------------------------------------------------------------------------
	//! The town nearest `from` that nobody holds, other than `skip`. -1 if none.
	static int QuietTownNear(RZ_GameModeComponent gm, RZ_WorldFacts facts, vector from, int skip)
	{
		int best = -1;
		float bestDist = float.MAX;
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			if (i == skip || facts.IsLivedIn(i))
				continue;

			float d = RZ_Util.DistXZ(s.m_vCenter, from);
			if (d < bestDist)
			{
				bestDist = d;
				best = i;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! The ground between a and b stays below the line from a's eye (1.7 m) to b (2 m up): ten
	//! height samples.
	static bool TerrainClear(vector a, vector b)
	{
		vector from = RZ_Util.GroundPos(a) + Vector(0, 1.7, 0);
		vector to = RZ_Util.GroundPos(b) + Vector(0, 2, 0);
		for (int i = 1; i < 10; i++)
		{
			float t = i / 10.0;
			vector p = from + (to - from) * t;
			if (RZ_Util.GroundPos(p)[1] > p[1])
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A canon spot, made by its rule; zero if it can't be ("none" for the hill).
	protected vector Spot(RZ_WorldFacts facts, string key, RZ_CanonRule rule)
	{
		string v = facts.CanonText(key, rule);
		if (v.IsEmpty() || v == "none")
			return vector.Zero;

		return RZ_WorldFacts.ParsePos(v);
	}

	//------------------------------------------------------------------------------------------------
	//! The new canon, then every town's first days. Same seed, same marks.
	void Seed()
	{
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		vector cp = facts.CanonPos("day2.checkpoint");
		vector site = facts.CanonPos("origin.site");
		vector gym = Spot(facts, "day2.gym", new RZ_TraceCanonGym());
		vector church = Spot(facts, "day4.church", new RZ_TraceCanonChurch());
		vector roadblock = Spot(facts, "day1.roadblock", new RZ_TraceCanonRoadblock());
		Spot(facts, "day2.queue", new RZ_TraceCanonQueue());
		Spot(facts, "day4.hill", new RZ_TraceCanonHill());
		int gymTown = -1;
		if (gym != vector.Zero)
		{
			gymTown = m_GM.FindNearestSettlement(gym);
			if (!facts.HasCanon("day2.gym.town") && gymTown >= 0)
				facts.SetCanon("day2.gym.town", gymTown.ToString());
		}

		int churchTown = -1;
		if (church != vector.Zero)
		{
			churchTown = m_GM.FindNearestSettlement(church);
			if (!facts.HasCanon("day4.church.town") && churchTown >= 0)
				facts.SetCanon("day4.church.town", churchTown.ToString());
		}

		m_sInfected = facts.CanonText("traces.start.infected", new RZ_TraceCanonStart(0));
		m_sBastion = facts.CanonText("traces.start.bastion", new RZ_TraceCanonStart(1));
		m_sCrews = facts.CanonText("traces.start.crews", new RZ_TraceCanonStart(2));
		m_sBigHouses = facts.CanonText("traces.start.houses", new RZ_TraceCanonStart(3));
		int seed = facts.GetSeed();
		m_iStage = 1;
		int swept = SeedSweep(seed, cp, site, roadblock);
		m_iStage = 2;
		SeedResidents(seed, gym, church);
		m_iStage = 3;
		SeedHaven(seed, gym, gymTown, church, churchTown);
		m_iStage = 4;
		SeedFactions(seed, facts.CanonPos("redhand.crossroads"));
		m_iStage = 5;
		SeedInfected(seed);

		string roadText = "none";
		if (roadblock != vector.Zero)
			roadText = RZ_SettlementScanner.GridReference(roadblock);

		RZ_Log.Info(string.Format("Traces: first days seeded (gym %1, church %2, roadblock %3, %4 towns swept)", m_Doors.TownName(gymTown), m_Doors.TownName(churchTown), roadText, swept));
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Rng TownRng(int seed, int town, int salt)
	{
		return new RZ_Rng(seed + town * 7919 + salt * 104729 + 17);
	}

	//------------------------------------------------------------------------------------------------
	//! Writes a seeded layer on the town's next first-days door of this part of the seeding.
	protected void Put(int town, int want, vector anchor, RZ_DoorMark m)
	{
		int slot = town * 10 + m_iStage;
		int n;
		m_mNext.Find(slot, n);
		m_mNext.Set(slot, n + 1);
		m_Doors.Seed(town, want, anchor, m, string.Format("fd.%1.%2.%3", town, m_iStage, n));
	}

	//------------------------------------------------------------------------------------------------
	protected static bool InList(string list, int idx)
	{
		return list.Contains(";" + idx.ToString() + ";");
	}

	//------------------------------------------------------------------------------------------------
	//! Writes a seeded layer on a named first-days door (the gym, a safehouse's door), so a live
	//! write can find the same door again.
	protected void PutAt(string key, int town, int want, vector anchor, RZ_DoorMark m)
	{
		m_Doors.Seed(town, want, anchor, m, key);
	}

	// ================================================================ Bastion's sweep
	//------------------------------------------------------------------------------------------------
	protected int SeedSweep(int seed, vector cp, vector site, vector roadblock)
	{
		array<int> towns = {};
		array<float> dists = {};
		array<bool> cordon = {};
		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			float dNorth = float.MAX;
			if (cp != vector.Zero)
				dNorth = RZ_Util.DistXZ(s.m_vCenter, cp);

			float dCordon = float.MAX;
			if (site != vector.Zero && roadblock != vector.Zero)
				dCordon = RZ_Util.DistToSegmentXZ(s.m_vCenter, site, roadblock);
			else if (site != vector.Zero)
				dCordon = RZ_Util.DistXZ(s.m_vCenter, site);

			if (dNorth > 4000 && dCordon > 3000)
				continue;

			// The Long Table's ground: Bastion's sweep never reached within 1.5 km of the farm
			RZ_LongTable lt = m_GM.GetLongTable();
			if (lt && lt.Exists() && lt.IsNearHouse(s.m_vCenter, 1500))
				continue;

			// Nearest first, so the cap keeps the towns the sweeps really walked
			float key = Math.Min(dNorth, dCordon * 1.2);
			int at = 0;
			while (at < dists.Count() && dists[at] <= key)
			{
				at++;
			}

			towns.InsertAt(i, at);
			dists.InsertAt(key, at);
			cordon.InsertAt(dCordon * 1.2 < dNorth, at);
		}

		int n = 0;
		for (int t = 0; t < towns.Count() && n < MAX_SWEPT; t++)
		{
			SweepTown(seed, towns[t], cordon[t]);
			n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! One town's X's: the cordon detail on Day 1, else the north road's sweeps (Days 2-5).
	protected void SweepTown(int seed, int town, bool cordon)
	{
		RZ_Rng rng = TownRng(seed, town, 1);
		int unit = rng.Int(0, 5);
		if (cordon)
			unit = 5;	// HQ: the cordon detail

		int count = rng.Int(2, MAX_X + 1);
		for (int i = 0; i < count; i++)
		{
			int day = 1;
			int hour = rng.Int(0, 6);
			if (!cordon)
			{
				day = rng.Int(2, 6);
				hour = rng.Int(6, 20);
			}

			int minute = rng.Int(0, 6) * 10;
			int hhmm = hour * 100 + minute;

			// The slash: about 1 in 20, never before Day 3
			if (day >= 3 && rng.Chance(0.05))
			{
				Put(town, RZ_EDoorWant.HOUSE, vector.Zero, RZ_DoorMark.Make(RZ_EDoorSymbol.SLASH, day, hhmm, unit));
				continue;
			}

			int hazard = 0;
			float roll = rng.Float01();
			if (roll < 0.25)
				hazard = 1;
			else if (roll < 0.35 && day >= 2)
				hazard = 2;
			else if (roll < 0.45)
				hazard = 4;
			else if (roll < 0.5)
				hazard = 5;

			int living = 0;
			if (rng.Chance(0.55))
				living = rng.Int(1, 5);

			int dead = 0;
			if (rng.Chance(0.25))
				dead = rng.Int(1, 3);

			int dest = 0;
			if (living > 0)
			{
				dest = 1;	// Day 1 and Day 2 before 14:00: the north road
				if (day >= 4)
					dest = 3;
				else if (day >= 3 || (day == 2 && hhmm >= 1400))
					dest = 2;
			}

			int flags = 0;
			if (day <= 4 && (hazard == 2 || dead > 0) && rng.Chance(0.3))
				flags = RZ_DoorMark.F_RING;

			int want = RZ_EDoorWant.HOUSE;
			if (i == 0 && rng.Chance(0.5))
				want = RZ_EDoorWant.SHOP;
			else if (i == 1 && rng.Chance(0.3))
				want = RZ_EDoorWant.CLINIC;

			Put(town, want, vector.Zero, RZ_DoorMark.Make(RZ_EDoorSymbol.X, day, hhmm, unit, RZ_TraceText.PackFound(hazard, living, dead, dest), flags));
		}
	}

	// ================================================================ Residents
	//------------------------------------------------------------------------------------------------
	protected void SeedResidents(int seed, vector gym, vector church)
	{
		int towns = m_GM.GetSettlements().Count();
		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			RZ_Rng rng = TownRng(seed, i, 2);
			if (!rng.Chance(0.33))
				continue;

			int count = rng.Int(1, 3);
			for (int k = 0; k < count; k++)
			{
				array<int> pool = {0, 2, 3, 4};
				if (gym != vector.Zero && RZ_Util.DistXZ(s.m_vCenter, gym) < 5000)
					pool.Insert(1);

				if (church != vector.Zero && RZ_Util.DistXZ(s.m_vCenter, church) < 4000)
					pool.Insert(5);

				int line = pool[rng.Int(0, pool.Count())];
				int day = 1;
				if (line == 1 || line == 2)
					day = 2;
				else if (line == 5)
					day = 4;
				else if (line == 3)
					day = rng.Int(1, 4);

				int other = 0;
				if (line == 0 && towns > 1)
				{
					other = rng.Int(0, towns);
					if (other == i)
						other = (other + 1) % towns;

					other++;
				}

				Put(i, RZ_EDoorWant.HOUSE, vector.Zero, RZ_DoorMark.Make(RZ_EDoorSymbol.WORDS, day, line, other));
			}
		}
	}

	// ================================================================ The Haven
	//------------------------------------------------------------------------------------------------
	protected void SeedHaven(int seed, vector gym, int gymTown, vector church, int churchTown)
	{
		// The gym's bear, Day 2; bears on houses round it, some ringed with an arrow to a house
		if (gymTown >= 0)
		{
			PutAt("fd.gym", gymTown, RZ_EDoorWant.NEAR, gym, RZ_DoorMark.Make(RZ_EDoorSymbol.BEAR, 2));
			RZ_Rng rng = TownRng(seed, gymTown, 3);
			int bears = rng.Int(1, 3);
			for (int b = 0; b < bears; b++)
			{
				Put(gymTown, RZ_EDoorWant.HOUSE, vector.Zero, MovedBear(rng, gym, rng.Int(2, 5)));
			}
		}

		if (churchTown >= 0)
		{
			RZ_Rng rngC = TownRng(seed, churchTown, 4);
			Put(churchTown, RZ_EDoorWant.HOUSE, vector.Zero, MovedBear(rngC, church, rngC.Int(4, 8)));
			// The first dead read at supper, Day 5
			Put(churchTown, RZ_EDoorWant.NEAR, church, Candle(rngC, rngC.Int(1, 4), 5));
		}

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses)
			return;

		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		for (int h = 0; h < houses.Count(); h++)
		{
			RZ_HouseFacts f = facts.House(h);
			if (!f.m_bValid || f.m_iSettlement < 0)
				continue;

			RZ_Rng rngH = TownRng(seed, f.m_iSettlement, 10 + h);
			int opened = rngH.Int(3, 10);
			PutAt(HouseDoorKey(h), f.m_iSettlement, RZ_EDoorWant.NEAR, f.m_vPos, RZ_DoorMark.Make(RZ_EDoorSymbol.PATCHED_BEAR, opened));
			if (InList(m_sBigHouses, h))
			{
				int extra = rngH.Int(1, 3);
				for (int e = 0; e < extra; e++)
				{
					Put(f.m_iSettlement, RZ_EDoorWant.HOUSE, f.m_vPos, RZ_DoorMark.Make(RZ_EDoorSymbol.BEAR, rngH.Int(opened, 11)));
				}
			}

			if (rngH.Chance(0.4))
				Put(f.m_iSettlement, RZ_EDoorWant.HOUSE, f.m_vPos, Candle(rngH, rngH.Int(1, 3), rngH.Int(5, 11)));

			if (rngH.Chance(0.2))
				Put(f.m_iSettlement, RZ_EDoorWant.HOUSE, f.m_vPos, RZ_DoorMark.Make(RZ_EDoorSymbol.HATCH, rngH.Int(6, 11)));

			// The drop: a working well or pump near the house
			RZ_Landmark well = RZ_LandmarkFinder.FindNearest(f.m_vPos, 300, {"well"});
			if (well)
				Put(f.m_iSettlement, RZ_EDoorWant.NEAR, well.m_vPos, RZ_DoorMark.Make(RZ_EDoorSymbol.DROP, rngH.Int(6, 11)));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The first-days door key of safehouse `idx` (its patched bear), for the live writes about it.
	static string HouseDoorKey(int idx)
	{
		return "fd.house." + idx.ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! A bear on a house with children; half the time ringed, with an arrow to the nearest house.
	protected RZ_DoorMark MovedBear(RZ_Rng rng, vector near, int day)
	{
		if (!rng.Chance(0.5))
			return RZ_DoorMark.Make(RZ_EDoorSymbol.BEAR, day);

		int to = 0;
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
		{
			RZ_Safehouse h = houses.Get(houses.FindNearest(near));
			if (h)
				to = h.m_iSettlement + 1;
		}

		return RZ_DoorMark.Make(RZ_EDoorSymbol.BEAR, day + rng.Int(1, 4), to, 0, 0, RZ_DoorMark.F_RING);
	}

	//------------------------------------------------------------------------------------------------
	//! A candle with its number and the names under it (seeded first names; the Elder reads them).
	protected RZ_DoorMark Candle(RZ_Rng rng, int count, int day)
	{
		RZ_DoorMark m = RZ_DoorMark.Make(RZ_EDoorSymbol.CANDLE, day, count);
		string names = "";
		for (int i = 0; i < count; i++)
		{
			if (i > 0)
				names += ", ";

			names += RZ_NoteText.FirstName(rng);
		}

		m.m_sText = names;
		return m;
	}

	// ================================================================ The factions
	//------------------------------------------------------------------------------------------------
	protected void SeedFactions(int seed, vector crossroads)
	{
		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			RZ_Rng rng = TownRng(seed, i, 5);

			// The Red Hand: tallies near the crossroads (the first toll was Day 4), hands from Day 9
			bool nearCross = crossroads != vector.Zero && RZ_Util.DistXZ(s.m_vCenter, crossroads) < 1500 + s.m_fRadius;
			if (nearCross)
			{
				Put(i, RZ_EDoorWant.SHOP, vector.Zero, RZ_DoorMark.Make(RZ_EDoorSymbol.TALLY, rng.Int(4, 9), rng.Int(3, 15)));
				Put(i, RZ_EDoorWant.GARAGE, vector.Zero, RZ_DoorMark.Make(RZ_EDoorSymbol.HAND, rng.Int(9, 11), rng.Int(1, 3)));
			}
			else if (InList(m_sCrews, i))
			{
				Put(i, RZ_EDoorWant.SHOP, vector.Zero, RZ_DoorMark.Make(RZ_EDoorSymbol.HAND, rng.Int(9, 11), rng.Int(1, 3)));
			}

			// Bastion round its posts: the post code, ATT and C (from Day 7)
			if (InList(m_sBastion, i))
			{
				int post = 1 + i % 9;
				Put(i, RZ_EDoorWant.HOUSE, vector.Zero, RZ_DoorMark.Make(RZ_EDoorSymbol.POSTCODE, rng.Int(7, 11), post));
				if (rng.Chance(0.5))
					Put(i, RZ_EDoorWant.HOUSE, vector.Zero, RZ_DoorMark.Make(RZ_EDoorSymbol.ATT, rng.Int(8, 11)));

				if (rng.Chance(0.5))
					Put(i, RZ_EDoorWant.FARM, vector.Zero, RZ_DoorMark.Make(RZ_EDoorSymbol.CONTRIB, rng.Int(7, 11)));
			}

			// Halcyon round the lab (the trial was Day 6)
			if (m_GM.HasLab() && i != m_GM.GetLabSettlement() && RZ_Util.DistXZ(s.m_vCenter, m_GM.GetLabPosition()) < 3000 && rng.Chance(0.6))
				Put(i, RZ_EDoorWant.FARM, vector.Zero, RZ_DoorMark.Make(RZ_EDoorSymbol.BEAK, rng.Int(7, 11), rng.Int(10, 60), rng.Int(1, 5)));
		}
	}

	// ================================================================ The infected
	//------------------------------------------------------------------------------------------------
	protected void SeedInfected(int seed)
	{
		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			if (!InList(m_sInfected, i))
				continue;

			RZ_Rng rng = TownRng(seed, i, 6);
			int count = rng.Int(1, 3);
			for (int k = 0; k < count; k++)
			{
				int tall = 0;
				if (rng.Chance(0.3))
					tall = 1;

				Put(i, RZ_EDoorWant.HOUSE, vector.Zero, RZ_DoorMark.Make(RZ_EDoorSymbol.SCRATCH_OUT, rng.Int(3, 11), rng.Int(1, 6), tall));
			}

			if (rng.Chance(0.5))
				Put(i, RZ_EDoorWant.HOUSE, vector.Zero, RZ_DoorMark.Make(RZ_EDoorSymbol.SMEARS, rng.Int(6, 11)));
		}
	}
}
