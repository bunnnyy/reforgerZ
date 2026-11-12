//! Scenes (docs/TRACES.md, "Scenes"): the first days frozen in place (F1-F8), the line (N1, N2),
//! vignettes dealt where the data says something happened (V1-V8) and live traces that follow the
//! simulation (L1, L2, L5, L6; L3, L4, L7 and L8 are door marks). A scene is a record: a kind, a
//! place, a made day, how long it lasts and a few fields. Its words always come (an approach line
//! once a session within 40 m, the one more line a background gives, its paper in the reader);
//! its props come through the marks spawner's Traces pool near players only, when the kit has
//! them, at most 3 scenes with props within 300 m.
//!
//! - The first days and N2 are seeded at every start from the season's seed and the canon spots,
//!   never saved, the same after a restart. V6 is seeded once a season (canon).
//! - Vignettes: 12 at once, 3 dealt a season day, one a town (World > Traces > Vignettes).
//! - Live traces: 24 at once, the oldest goes (World > Traces > Live traces).
//! - Never in the warning zone, within 2 km of the Long Table's farm, within 300 m of the closed
//!   site, within 60 m of a lived-in place; dealt scenes also never within 300 m of a running job,
//!   600 m of a lair or on the evening's set piece. After the cure nothing new is dealt.
//! Nothing lootable (every prop through the dressing guard), nothing pays. Server only.

//------------------------------------------------------------------------------------------------
//! One scene. Saved in state.json (vignettes, live traces and N1's crosses); seeded ones aren't.
class RZ_Scene
{
	string key;			// "f1", "v3.12"
	string kind;		// "F1".."F8", "N1", "N2", "V1".."V8", "L1", "L2", "L5", "L6"
	float x;
	float z;
	float yaw;			// degrees: the way the pieces face (the cars toward the barrier)
	int town = -1;
	float made;			// season days
	float until = -1;	// season days; -1 = the season
	int a;				// by kind: F1 cars, F3 1 = its town is lived in, F6 the notices' bits, N1 crosses,
	int b;				// V3/V8 the house, L1 the post, L2 1 = Haven chalk, L5 1 robbed 2 a pack; b: the day
	int faction = -1;
	string text;		// V4 the town on the IOU, L1 who, L5 the stencil
	bool epilogue;		// L5: one of the last four wrecks, the epilogue's for the week
}

//------------------------------------------------------------------------------------------------
//! V6, the waiting room: a clinic in a town nobody holds, never the first patient's town. Once a
//! season. "none" when no clinic qualifies.
class RZ_TraceCanonWaiting : RZ_CanonRule
{
	override string Make(RZ_GameModeComponent gm, RZ_WorldFacts facts, RZ_Rng rng)
	{
		int origin = facts.CanonSettlement("origin.clinic");
		array<int> order = {};
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			order.Insert(i);
		}

		rng.ShuffleInts(order);
		int tries = 0;
		foreach (int t : order)
		{
			if (tries >= 30)
				break;

			RZ_Settlement st = gm.GetSettlement(t);
			if (!st || t == origin || facts.IsLivedIn(t) || RZ_Perimeter.IsNearLineAt(st.m_vCenter))
				continue;

			tries++;
			RZ_Landmark clinic = RZ_LandmarkFinder.FindNearest(st.m_vCenter, st.m_fRadius, {"pharmacy"});
			if (clinic)
				return RZ_WorldFacts.PosText(clinic.m_vPos);
		}

		return "none";
	}
}

//------------------------------------------------------------------------------------------------
class RZ_Scenes
{
	protected static const float APPROACH = 40;
	protected static const float PAPER = 6;
	protected static const float RADIO = 12;
	protected static const int PAPER_SECONDS = 2;
	protected static const float PROPS_NEAR = 300;
	protected static const int PROPS_SCENES = 3;
	protected static const int PIECES = 9;
	protected static const int CROSSES = 7;
	protected static const float LINE_DEATH = 1000;

	protected RZ_GameModeComponent m_GM;
	protected RZ_Traces m_Traces;
	protected ref array<ref RZ_Scene> m_aScenes = {};
	protected ref set<string> m_aPropped = new set<string>();		// scenes with props placed
	protected ref map<string, int> m_mStage = new map<string, int>();	// the stage each was shown at
	protected ref set<string> m_aSeen = new set<string>();			// "player|scene": the approach line
	protected ref set<string> m_aPaperRead = new set<string>();
	protected ref set<string> m_aRadio = new set<string>();
	protected ref map<int, string> m_mPaperAt = new map<int, string>();
	protected ref map<int, int> m_mPaperSeconds = new map<int, int>();
	protected ref set<string> m_aHandled = new set<string>();		// triggers already dealt this run
	protected ref map<string, int> m_mDeedDay = new map<string, int>();	// "kind|town" -> season day
	protected ref map<string, vector> m_mSites = new map<string, vector>();	// kill sites, last look
	protected ref map<string, string> m_mSiteWho = new map<string, string>();
	protected ref map<int, vector> m_mPosts = new map<int, vector>();	// Bastion's road posts, last look
	protected bool m_bLooked;
	protected int m_iDealDay = -1;
	protected int m_iDealt;
	protected int m_iNext = 1;

	//------------------------------------------------------------------------------------------------
	void RZ_Scenes(RZ_GameModeComponent gm, RZ_Traces traces)
	{
		m_GM = gm;
		m_Traces = traces;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_TraceSettings Settings()
	{
		return m_Traces.GetSettings();
	}

	//------------------------------------------------------------------------------------------------
	int Count()
	{
		return m_aScenes.Count();
	}

	//------------------------------------------------------------------------------------------------
	array<ref RZ_Scene> GetScenes()
	{
		return m_aScenes;
	}

	//------------------------------------------------------------------------------------------------
	static vector Pos(RZ_Scene s)
	{
		return Vector(s.x, 0, s.z);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IsSeededKind(string kind)
	{
		return kind.StartsWith("F") || kind == "N2" || kind == "V6";
	}

	// ================================================================ Save
	//------------------------------------------------------------------------------------------------
	void Save(notnull array<ref RZ_Scene> result)
	{
		foreach (RZ_Scene s : m_aScenes)
		{
			if (!IsSeededKind(s.kind))
				result.Insert(s);
		}
	}

	//------------------------------------------------------------------------------------------------
	int DealDay() { return m_iDealDay; }
	int Dealt() { return m_iDealt; }

	//------------------------------------------------------------------------------------------------
	//! Before Seed: the saved scenes still in their days come back.
	void Restore(array<ref RZ_Scene> saved, int dealDay, int dealt, int next)
	{
		m_iDealDay = dealDay;
		m_iDealt = dealt;
		m_iNext = Math.Max(1, next);
		if (!saved)
			return;

		float today = RZ_SeasonClock.Days();
		foreach (RZ_Scene s : saved)
		{
			if (s && !s.kind.IsEmpty() && (s.until < 0 || s.until > today))
				m_aScenes.Insert(s);
		}
	}

	//------------------------------------------------------------------------------------------------
	int NextKey()
	{
		return m_iNext;
	}

	// ================================================================ Where scenes may stand
	//------------------------------------------------------------------------------------------------
	//! The rules every scene keeps: not in the warning zone, not on the Long Table's ground, not by
	//! the closed site, not in a lived-in place's rings.
	protected bool Allowed(vector at)
	{
		if (at == vector.Zero || RZ_Perimeter.IsNearLineAt(at) || RZ_Util.IsWater(RZ_Util.GroundPos(at)))
			return false;

		RZ_LongTable lt = m_GM.GetLongTable();
		if (lt && lt.Exists() && lt.IsNearHouse(at, 2000))
			return false;

		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		vector site = facts.CanonPos("origin.site");
		if (site != vector.Zero && RZ_Util.DistXZ(site, at) < 300)
			return false;

		return !facts.IsNearLivedIn(at, 60);
	}

	//------------------------------------------------------------------------------------------------
	//! Dealt scenes also keep off a running job (300 m), a lair (600 m, unless `lairs` is false) and
	//! the evening's set piece.
	protected bool AllowedNow(vector at, bool lairs)
	{
		if (!Allowed(at))
			return false;

		RZ_MissionManager missions = m_GM.GetMissions();
		if (missions)
		{
			foreach (RZ_Mission m : missions.GetActive())
			{
				if (m && m.m_eState == RZ_EMissionState.ACTIVE && !m.m_bGlobal && RZ_Util.DistXZ(m.m_vPos, at) <= 300)
					return false;
			}
		}

		RZ_Shared shared = m_GM.GetShared();
		if (shared && shared.GetCalendar().IsQuietAt(at))
			return false;

		RZ_Tribes tribes = m_GM.GetTribes();
		if (lairs && tribes)
		{
			foreach (RZ_Tribe t : tribes.GetTribeList())
			{
				if (t && RZ_Util.DistXZ(t.m_vLair, at) < 600)
					return false;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A dry, flat point in a ring round `center`, out of every building's footprint (2 m). The
	//! centre itself if none of ten tries fits.
	static vector Ground(vector center, float minR, float maxR)
	{
		for (int i = 0; i < 10; i++)
		{
			vector p = RZ_Util.GroundPos(RZ_Mission.LandPointNear(center, minR, maxR));
			if (GoodGround(p))
				return p;
		}

		return RZ_Util.GroundPos(center);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool GoodGround(vector p)
	{
		if (RZ_Util.IsWater(p) || RZ_Perimeter.IsNearLineAt(p))
			return false;

		BaseWorld world = GetGame().GetWorld();
		float east = world.GetSurfaceY(p[0] + 1.5, p[2]);
		float west = world.GetSurfaceY(p[0] - 1.5, p[2]);
		float north = world.GetSurfaceY(p[0], p[2] + 1.5);
		float south = world.GetSurfaceY(p[0], p[2] - 1.5);
		if (Math.AbsFloat(east - west) >= 0.64 || Math.AbsFloat(north - south) >= 0.64)
			return false;

		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(p, 25, buildings, 8);
		foreach (IEntity b : buildings)
		{
			if (b && RZ_LockedSpots.InFootprint(b, p, 2))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A door of `want` in a town whose doors are known, nearest `near`, that takes marks; null if
	//! the town isn't resolved or has none.
	protected RZ_DoorSlot DoorNear(int town, int want, vector near)
	{
		RZ_TownSlots slots = m_Traces.GetSlots().Get(town);
		if (!slots)
			return null;

		RZ_DoorSlot best = null;
		float bestDist = float.MAX;
		foreach (RZ_DoorSlot d : slots.m_aSlots)
		{
			if (d.m_bOff || d.m_bLocked || (want != RZ_EDoorWant.ANY && d.m_iKind != want))
				continue;

			float dist = RZ_Util.DistXZ(d.m_vPos, near);
			if (dist < bestDist)
			{
				bestDist = dist;
				best = d;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! A point `dist` metres in front of a door (on its side of the wall), on the ground.
	protected static vector BeforeDoor(RZ_DoorSlot d, float dist)
	{
		return RZ_Util.GroundPos(d.m_vPos + d.m_vNormal * dist);
	}

	//------------------------------------------------------------------------------------------------
	//! A road point about `dist` metres from a barrier, inward (outside the warning zone), else a
	//! point straight in from the edge.
	protected vector LinePoint(vector cp, float minD, float maxD)
	{
		array<vector> points = {};
		RZ_Perimeter.RoadPointsNear(cp, maxD, points);
		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		vector best = vector.Zero;
		float bestEdge = -1;
		foreach (vector p : points)
		{
			float d = RZ_Util.DistXZ(p, cp);
			if (d < minD || d > maxD || RZ_Perimeter.IsNearLineAt(p))
				continue;

			float edge = 0;
			if (perimeter)
				edge = perimeter.EdgeDistance(p);

			if (edge > bestEdge)
			{
				bestEdge = edge;
				best = p;
			}
		}

		if (best != vector.Zero || !perimeter)
			return best;

		vector p2 = cp + perimeter.InwardAt(cp) * ((minD + maxD) * 0.5);
		if (RZ_Perimeter.IsNearLineAt(p2))
			return vector.Zero;

		return RZ_Util.GroundPos(p2);
	}

	// ================================================================ Making scenes
	//------------------------------------------------------------------------------------------------
	protected RZ_Scene Make(string kind, string key, vector at, int town, float days)
	{
		RZ_Scene s = new RZ_Scene();
		s.kind = kind;
		s.key = key;
		s.x = at[0];
		s.z = at[2];
		s.yaw = -1;
		s.town = town;
		s.made = RZ_SeasonClock.Days();
		if (days > 0)
			s.until = s.made + days;

		return s;
	}

	//------------------------------------------------------------------------------------------------
	protected void Add(RZ_Scene s, string trigger)
	{
		m_aScenes.Insert(s);
		Show(s);
		RZ_Log.Info(string.Format("Traces: scene %1 at %2 (%3)", s.key, RZ_SettlementScanner.GridReference(Pos(s)), trigger));
	}

	//------------------------------------------------------------------------------------------------
	protected bool VignettesOn()
	{
		return Settings().m_bVignettes && !RZ_WorldFacts.Get().IsCured();
	}

	//------------------------------------------------------------------------------------------------
	protected bool LiveOn()
	{
		return Settings().m_bLiveTraces && !RZ_WorldFacts.Get().IsCured();
	}

	//------------------------------------------------------------------------------------------------
	protected int CountPrefix(string prefix)
	{
		int n = 0;
		foreach (RZ_Scene s : m_aScenes)
		{
			if (s.kind.StartsWith(prefix))
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Scene FindKind(string kind, int town)
	{
		foreach (RZ_Scene s : m_aScenes)
		{
			if (s.kind == kind && (town < 0 || s.town == town))
				return s;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! A vignette: 12 at once, 3 dealt a season day, one a town; null if it can't be dealt now.
	protected RZ_Scene Deal(string kind, int town, vector at, float days, string trigger, bool lairs = true)
	{
		if (!VignettesOn() || at == vector.Zero)
			return null;

		int today = RZ_SeasonClock.Day();
		if (m_iDealDay != today)
		{
			m_iDealDay = today;
			m_iDealt = 0;
		}

		if (m_iDealt >= Settings().m_iVignettesPerDay || CountPrefix("V") >= Settings().m_iVignettes)
			return null;

		foreach (RZ_Scene other : m_aScenes)
		{
			if (other.kind.StartsWith("V") && town >= 0 && other.town == town)
				return null;
		}

		if (!AllowedNow(at, lairs))
		{
			RZ_Log.Info(string.Format("Traces: no anchor for %1 in %2", kind, m_Traces.GetDoors().TownName(town)));
			return null;
		}

		m_iDealt++;
		RZ_Scene s = Make(kind, NewKey(kind), at, town, days);
		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! "v3.12": the kind in lower case and the next number (saved, so keys never repeat).
	protected string NewKey(string kind)
	{
		string lower = kind;
		lower.ToLower();
		string key = string.Format("%1.%2", lower, m_iNext);
		m_iNext++;
		return key;
	}

	//------------------------------------------------------------------------------------------------
	//! A live trace: 24 at once (the oldest goes); a player's deed makes one a town a season day.
	protected RZ_Scene Live(string kind, int town, vector at, float days, bool deed, bool lairs = true)
	{
		if (!LiveOn() || at == vector.Zero || !AllowedNow(at, lairs))
			return null;

		if (deed && town >= 0)
		{
			string dk = string.Format("%1|%2", kind, town);
			int was;
			if (m_mDeedDay.Find(dk, was) && was == RZ_SeasonClock.Day())
				return null;

			m_mDeedDay.Set(dk, RZ_SeasonClock.Day());
		}

		while (CountPrefix("L") >= Math.Max(1, Settings().m_iLiveTraces))
		{
			if (!DropOldestLive())
				break;
		}

		RZ_Scene s = Make(kind, NewKey(kind), at, town, days);
		return s;
	}

	//------------------------------------------------------------------------------------------------
	protected bool DropOldestLive()
	{
		int drop = -1;
		foreach (int i, RZ_Scene s : m_aScenes)
		{
			if (s.kind.StartsWith("L") && !s.epilogue && (drop < 0 || s.made < m_aScenes[drop].made))
				drop = i;
		}

		if (drop < 0)
			return false;

		Remove(drop, "made room");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Remove(int i, string why)
	{
		RZ_Scene s = m_aScenes[i];
		Hide(s);
		RZ_Log.Info(string.Format("Traces: scene %1 gone (%2)", s.key, why));
		m_aScenes.Remove(i);
	}

	// ================================================================ The first days and the line
	//------------------------------------------------------------------------------------------------
	//! At every start: F1-F8 from the canon spots (made once a season), N2 at every barrier, V6, and
	//! the north road's first two crosses if they aren't back from the save.
	void Seed()
	{
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		int seed = facts.GetSeed();
		vector cp = facts.CanonPos("day2.checkpoint");
		vector queue = Spot(facts, "day2.queue", new RZ_TraceCanonQueue());
		int n = 0;
		if (Settings().m_bFirstDayScenes)
		{
			n += SeedColumn(seed, cp, queue);
			n += SeedPlaces(facts);
			n += SeedSquares(facts, seed, cp);
			n += SeedLine(cp);
		}

		if (Settings().m_bVignettes)
		{
			vector clinic = Spot(facts, "traces.v6", new RZ_TraceCanonWaiting());
			if (clinic != vector.Zero && Allowed(clinic))
			{
				RZ_Scene v6 = Make("V6", "v6", Ground(clinic, 4, 9), m_GM.FindNearestSettlement(clinic), -1);
				Add(v6, "first days");
				n++;
			}
		}

		// The restored ones go back into the spawner
		foreach (RZ_Scene s : m_aScenes)
		{
			if (!IsSeededKind(s.kind))
				Show(s);
		}

		RZ_Log.Info(string.Format("Traces: %1 scenes seeded, %2 back from the save.", n, m_aScenes.Count() - n));
	}

	//------------------------------------------------------------------------------------------------
	protected static vector Spot(RZ_WorldFacts facts, string key, RZ_CanonRule rule)
	{
		string v = facts.CanonText(key, rule);
		if (v.IsEmpty() || v == "none")
			return vector.Zero;

		return RZ_WorldFacts.ParsePos(v);
	}

	//------------------------------------------------------------------------------------------------
	//! F1 the column's tail and F2 where they turned back, on the road in from the north barrier.
	protected int SeedColumn(int seed, vector cp, vector queue)
	{
		if (cp == vector.Zero || queue == vector.Zero)
			return 0;

		int n = 0;
		RZ_Rng rng = new RZ_Rng(seed + 4101);
		vector inward = queue - cp;
		inward[1] = 0;
		inward.Normalize();
		if (Allowed(queue))
		{
			RZ_Scene f1 = Make("F1", "f1", queue, m_GM.FindNearestSettlement(queue), -1);
			f1.a = rng.Int(4, 9);
			f1.yaw = RZ_Compass.Bearing(queue, cp);
			Add(f1, "first days");
			n++;
		}

		vector turned = RZ_Util.GroundPos(cp + inward * 360);
		if (Allowed(turned))
		{
			RZ_Scene f2 = Make("F2", "f2", turned, m_GM.FindNearestSettlement(turned), -1);
			f2.yaw = RZ_Compass.Bearing(turned, cp);
			Add(f2, "first days");
			n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! F3 the gym, F4 the church, F5 the roadblock, F7 Rudi's shutter, F8 the hill.
	protected int SeedPlaces(RZ_WorldFacts facts)
	{
		int n = 0;
		vector gym = Spot(facts, "day2.gym", new RZ_TraceCanonGym());
		if (gym != vector.Zero && Allowed(gym))
		{
			int gymTown = m_GM.FindNearestSettlement(gym);
			RZ_Scene f3 = Make("F3", "f3", Ground(gym, 8, 16), gymTown, -1);
			if (facts.IsLivedIn(gymTown))
				f3.a = 1;	// camp life owns the rest: the bear and the list only

			Add(f3, "first days");
			n++;
		}

		vector church = Spot(facts, "day4.church", new RZ_TraceCanonChurch());
		if (church != vector.Zero && Allowed(church))
		{
			Add(Make("F4", "f4", Ground(church, 6, 12), m_GM.FindNearestSettlement(church), -1), "first days");
			n++;
		}

		vector roadblock = Spot(facts, "day1.roadblock", new RZ_TraceCanonRoadblock());
		if (roadblock != vector.Zero && Allowed(roadblock))
		{
			Add(Make("F5", "f5", roadblock, m_GM.FindNearestSettlement(roadblock), -1), "first days");
			n++;
		}

		vector cross = facts.CanonPos("redhand.crossroads");
		if (cross != vector.Zero && Allowed(cross))
		{
			Add(Make("F7", "f7", Ground(cross, 12, 50), m_GM.FindNearestSettlement(cross), -1), "first days");
			n++;
		}

		vector hill = Spot(facts, "day4.hill", new RZ_TraceCanonHill());
		if (hill != vector.Zero && Allowed(hill))
		{
			Add(Make("F8", "f8", hill, m_GM.FindNearestSettlement(hill), -1), "first days");
			n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! F6: one town in five nobody lives in, its square's notices layered by what's near it.
	protected int SeedSquares(RZ_WorldFacts facts, int seed, vector cp)
	{
		vector gym = facts.CanonPos("day2.gym");
		vector cross = facts.CanonPos("redhand.crossroads");
		int n = 0;
		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			RZ_Rng rng = new RZ_Rng(seed + i * 7919 + 61);
			if (!rng.Chance(0.2) || facts.IsLivedIn(i) || !Allowed(s.m_vCenter))
				continue;

			RZ_Scene f6 = Make("F6", "f6." + i.ToString(), Ground(s.m_vCenter, 5, 40), i, -1);
			if (cp != vector.Zero && RZ_Util.DistXZ(s.m_vCenter, cp) < 4000)
				f6.a = f6.a | 1;

			if (gym != vector.Zero && RZ_Util.DistXZ(s.m_vCenter, gym) < 5000)
				f6.a = f6.a | 2;

			if (m_GM.HasLab() && RZ_Util.DistXZ(s.m_vCenter, m_GM.GetLabPosition()) < 4000)
				f6.a = f6.a | 4;

			if (cross != vector.Zero && RZ_Util.DistXZ(s.m_vCenter, cross) < 3000)
				f6.a = f6.a | 8;

			Add(f6, "first days");
			n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! N2 the Day 1 sheets at every barrier, and N1 the north road's two crosses (D4, D6).
	protected int SeedLine(vector north)
	{
		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		if (!perimeter || !perimeter.IsActive())
			return 0;

		int n = 0;
		array<vector> barriers = {};
		perimeter.GetCheckpoints(barriers);
		foreach (int i, vector cp : barriers)
		{
			vector sheets = LinePoint(cp, 350, 700);
			if (sheets == vector.Zero || !Allowed(sheets))
				continue;

			Add(Make("N2", "n2." + i.ToString(), sheets, m_GM.FindNearestSettlement(sheets), -1), "first days");
			n++;
		}

		if (north != vector.Zero && !CrossesAt(north))
		{
			vector at = LinePoint(north, 320, 420);
			if (at != vector.Zero && Allowed(at))
			{
				RZ_Scene n1 = Make("N1", NewKey("N1"), at, m_GM.FindNearestSettlement(at), -1);
				n1.a = 2;
				Add(n1, "first days");
				n++;
			}
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! The crosses standing for the barrier at cp, or null.
	protected RZ_Scene CrossesAt(vector cp)
	{
		foreach (RZ_Scene s : m_aScenes)
		{
			if (s.kind == "N1" && RZ_Util.DistXZ(Pos(s), cp) <= 800)
				return s;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! N1: a death in the kill zone within 1 km of a barrier: one more cross outside the warning
	//! zone (7 a place at most). Saved.
	void LineDeath(vector pos)
	{
		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		if (!Settings().m_bFirstDayScenes || !perimeter || !perimeter.IsActive() || RZ_WorldFacts.Get().IsCured())
			return;

		if (perimeter.GetZone(pos) != RZ_EPerimeterZone.KILL)
			return;

		vector cp;
		if (!perimeter.GetNearestCheckpoint(pos, cp) || RZ_Util.DistXZ(cp, pos) > LINE_DEATH)
			return;

		RZ_Scene s = CrossesAt(cp);
		if (s)
		{
			if (s.a >= CROSSES)
				return;

			s.a++;
			Show(s);
			RZ_Log.Info(string.Format("Traces: scene %1 at %2 (a death at the line: %3 crosses)", s.key, RZ_SettlementScanner.GridReference(Pos(s)), s.a));
			return;
		}

		vector at = LinePoint(cp, 320, 420);
		if (at == vector.Zero || !Allowed(at))
			return;

		RZ_Scene n1 = Make("N1", NewKey("N1"), at, m_GM.FindNearestSettlement(at), -1);
		n1.a = 1;
		Add(n1, "a death at the line");
	}

	// ================================================================ Vignettes
	//------------------------------------------------------------------------------------------------
	//! V1: a town nobody holds went dark. A table in a garden, 4-10 m from a house's door.
	void TownDark(int town)
	{
		RZ_Settlement st = m_GM.GetSettlement(town);
		if (!st)
			return;

		vector at = Ground(st.m_vCenter, 15, Math.Max(20, st.m_fRadius * 0.6));
		RZ_DoorSlot d = DoorNear(town, RZ_EDoorWant.HOUSE, st.m_vCenter);
		if (d)
			at = Ground(BeforeDoor(d, 6), 0, 4);

		RZ_Scene s = Deal("V1", town, at, 3, "town gone dark");
		if (s)
			Add(s, "town gone dark");
	}

	//------------------------------------------------------------------------------------------------
	//! V2: a rescue failed, or a town went infested with a barricade standing. The house nearest.
	void HeldOut(int town, vector near, string trigger)
	{
		if (town < 0)
			return;

		vector at = Ground(near, 2, 10);
		RZ_DoorSlot d = DoorNear(town, RZ_EDoorWant.HOUSE, near);
		if (d)
			at = BeforeDoor(d, 2);

		RZ_Scene s = Deal("V2", town, at, 3, trigger);
		if (s)
			Add(s, trigger);
	}

	//------------------------------------------------------------------------------------------------
	//! V3: a walk-out at safehouse `house`: a chair 100-200 m out, toward the nearest pack.
	void Chair(int house, string trigger)
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses)
			return;

		RZ_Safehouse h = houses.Get(house);
		if (!h)
			return;

		vector toward = h.m_vPos + Vector(0, 0, 150);
		RZ_Tribes tribes = m_GM.GetTribes();
		if (tribes)
		{
			RZ_Tribe t = tribes.NearestTribe(h.m_vPos, 6000);
			if (t)
				toward = t.m_vLair;
		}

		vector dir = toward - h.m_vPos;
		dir[1] = 0;
		dir.Normalize();
		vector at = Ground(h.m_vPos + dir * 150, 0, 40);
		RZ_Scene s = Deal("V3", h.m_iSettlement, at, 2, trigger, false);
		if (!s)
			return;

		s.b = house;
		s.yaw = RZ_Compass.Bearing(h.m_vPos, at);
		Add(s, trigger);
	}

	//------------------------------------------------------------------------------------------------
	//! V5: a Halcyon run lost on the road, on the verge where it happened.
	void DroppedRun(vector pos, string trigger)
	{
		int town = RZ_TraceWriters.TownAt(pos);
		RZ_Scene s = Deal("V5", town, RZ_Marks.Verge(pos, 6, 10), 2, trigger);
		if (s)
			Add(s, trigger);
	}

	//------------------------------------------------------------------------------------------------
	//! V7: a Bastion outpost lost or taken: its roster board where its centre was.
	void FallenPost(vector pos, int town)
	{
		RZ_Scene s = Deal("V7", town, Ground(pos, 8, 25), 3, "Bastion outpost lost");
		if (s)
			Add(s, "Bastion outpost lost");
	}

	// ================================================================ Live traces
	//------------------------------------------------------------------------------------------------
	//! L2: a building burned: scorch at its doorway; Haven chalk if a Haven job was running there.
	void Fire(vector pos)
	{
		int town = RZ_TraceWriters.TownAt(pos);
		RZ_Scene s = Live("L2", town, RZ_Util.GroundPos(pos), 2, true);
		if (!s)
			return;

		s.b = RZ_WorldFacts.Get().Day();
		RZ_MissionManager missions = m_GM.GetMissions();
		if (missions)
		{
			foreach (RZ_Mission m : missions.GetActive())
			{
				if (m && m.m_bHasGiver && m.m_eGiver == RZ_EFaction.SURVIVORS && !m.m_bGlobal && RZ_Util.DistXZ(m.m_vPos, pos) <= m.m_fParticipationRadius)
					s.a = 1;
			}
		}

		Add(s, "a building burned");
	}

	//------------------------------------------------------------------------------------------------
	//! L5: a supply truck lost: its wreck on the verge. how: 1 robbed, 2 taken by a pack, 0 else.
	void Truck(int faction, vector pos, int how, string dest)
	{
		int town = RZ_TraceWriters.TownAt(pos);
		RZ_Scene s = Live("L5", town, RZ_Marks.Verge(pos, 6, 10), 2, false);
		if (!s)
			return;

		s.faction = faction;
		s.a = how;
		string stencil = dest;
		stencil.ToUpper();
		s.text = stencil;
		Add(s, "a truck lost");
	}

	//------------------------------------------------------------------------------------------------
	//! L6: a Bastion road post pulled: sandbags and its sign where it stood.
	void RoadPostGone(vector pos)
	{
		RZ_Scene s = Live("L6", RZ_TraceWriters.TownAt(pos), RZ_Marks.Verge(pos, 6, 10), 2, false);
		if (s)
			Add(s, "a road post pulled");
	}

	//------------------------------------------------------------------------------------------------
	//! L1: a kill site ended in a place the Haven or Bastion knows: a candle chalked on a post, or
	//! Bastion's stencil. With nobody knowing, nothing.
	void KillSiteGone(vector pos, string who)
	{
		int faction = -1;
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
		{
			for (int i = 0; i < houses.Count(); i++)
			{
				RZ_Safehouse h = houses.Get(i);
				if (h && h.m_iPopulation > 0 && RZ_Util.DistXZ(h.m_vPos, pos) <= 3000)
					faction = RZ_EFaction.SURVIVORS;
			}
		}

		RZ_Territory territory = m_GM.GetTerritory();
		if (faction < 0 && territory && territory.IsActive() && territory.FindHolding(RZ_EFaction.MILITARY, pos, 3000))
			faction = RZ_EFaction.MILITARY;

		if (faction < 0)
			return;

		// The Haven's candle goes on the nearest door when the site was in a town (within 60 m of it)
		int town = RZ_TraceWriters.TownAt(pos);
		RZ_Settlement here = m_GM.GetSettlement(town);
		if (faction == RZ_EFaction.SURVIVORS && here && RZ_Util.DistXZ(here.m_vCenter, pos) <= here.m_fRadius + 60)
		{
			RZ_DoorMark candle = RZ_DoorMark.Make(RZ_EDoorSymbol.CANDLE, RZ_WorldFacts.Get().Day(), 1, 0, 0, RZ_DoorMark.F_SHOWN);
			candle.m_sText = "someone";
			if (who.StartsWith("someone who"))
				candle.m_sText = who;

			RZ_Traces traces = RZ_Traces.Get();
			if (traces && traces.GetDoors().Write(town, RZ_EDoorWant.NEAR, pos, candle, "a kill site ended"))
				return;
		}

		RZ_Scene s = Live("L1", town, RZ_Marks.Verge(pos, 3, 6), 2, false, false);
		if (!s)
			return;

		s.faction = faction;
		s.b = RZ_WorldFacts.Get().Day();
		int post = Math.AbsInt(RZ_Util.RoundToInt(pos[0] + pos[2]));
		s.a = 1 + post % 9;
		s.text = "someone";
		if (who.StartsWith("someone who"))
			s.text = who;

		Add(s, "a kill site ended");
	}

	// ================================================================ Polls (30 s)
	//------------------------------------------------------------------------------------------------
	//! What no hook tells: a bite declared at a safehouse (V3), an honest shop (V4) and its PAID, a
	//! queue at a short house's pump (V8), kill sites ending (L1), Bastion's posts pulled (L6).
	void Look()
	{
		bool first = !m_bLooked;
		m_bLooked = true;
		Bitten();
		Shops();
		Queues();
		KillSites(first);
		Posts(first);
	}

	//------------------------------------------------------------------------------------------------
	protected void Bitten()
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses)
			return;

		RZ_WorldMemory mem = RZ_WorldFacts.Get().Memory();
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Reason r = mem.FindReason("house.bitten", RZ_WorldFacts.Safehouse(i));
			if (!r)
				continue;

			string key = string.Format("bitten|%1|%2", i, RZ_Util.RoundToInt(r.madeDay * 100));
			if (m_aHandled.Contains(key))
				continue;

			m_aHandled.Insert(key);
			Chair(i, "a bite declared");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! V4 in a town whose doors are known: nobody holds it, a Haven house within 2 km, no Red Hand
	//! holding within 1.5 km (tried once a town a run). PAID when the crews take ground within 1.5 km.
	protected void Shops()
	{
		RZ_Territory territory = m_GM.GetTerritory();
		bool crews = territory && territory.IsActive();
		foreach (RZ_Scene s : m_aScenes)
		{
			if (s.kind != "V4" || s.b > 0 || !crews || !territory.FindHolding(RZ_EFaction.BANDITS, Pos(s), 1500))
				continue;

			s.b = 1;
			s.until = RZ_SeasonClock.Days() + 3;
			Show(s);
			RZ_Log.Info(string.Format("Traces: scene %1 at %2 (PAID across the IOUs)", s.key, RZ_SettlementScanner.GridReference(Pos(s))));
		}

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses || !VignettesOn())
			return;

		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		foreach (int i, RZ_Settlement st : m_GM.GetSettlements())
		{
			string tried = "shop|" + i.ToString();
			if (m_aHandled.Contains(tried) || !m_Traces.GetSlots().IsResolved(i))
				continue;

			m_aHandled.Insert(tried);
			RZ_Rng rng = new RZ_Rng(facts.GetSeed() + i * 104729 + 404);
			if (!rng.Chance(0.35) || facts.IsLivedIn(i))
				continue;

			int near = houses.FindNearest(st.m_vCenter, 2000);
			RZ_Safehouse h = houses.Get(near);
			if (!h || h.m_iPopulation <= 0 || (crews && territory.FindHolding(RZ_EFaction.BANDITS, st.m_vCenter, 1500)))
				continue;

			RZ_DoorSlot d = DoorNear(i, RZ_EDoorWant.SHOP, st.m_vCenter);
			if (!d)
				continue;

			RZ_Scene v4 = Deal("V4", i, BeforeDoor(d, 1.5), -1, "an honest shop");
			if (!v4)
				continue;

			v4.text = m_Traces.GetDoors().TownName(h.m_iSettlement);
			v4.yaw = RZ_Compass.Bearing(BeforeDoor(d, 3), d.m_vPos);
			Add(v4, "an honest shop");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! V8 while a Haven house within 1.5 km is short and a pump stands within 400 m of it; it goes
	//! a season day after the house is fed.
	protected void Queues()
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses)
			return;

		foreach (RZ_Scene s : m_aScenes)
		{
			if (s.kind != "V8" || s.until >= 0)
				continue;

			RZ_Safehouse fed = houses.Get(s.b);
			if (!fed || !fed.m_bShort)
				s.until = RZ_SeasonClock.Days() + 1;
		}

		if (!VignettesOn())
			return;

		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			if (!h || !h.m_bShort || h.m_iPopulation <= 0)
				continue;

			string key = "queue|" + i.ToString();
			if (m_aHandled.Contains(key))
				continue;

			m_aHandled.Insert(key);
			RZ_Landmark well = RZ_LandmarkFinder.FindNearest(h.m_vPos, 400, {"well"});
			if (!well)
				continue;

			int town = m_GM.FindNearestSettlement(well.m_vPos);
			RZ_Scene s = Deal("V8", town, Ground(well.m_vPos, 2, 5), -1, "a house short of food", false);
			if (!s)
			{
				m_aHandled.RemoveItem(key);
				continue;
			}

			s.b = i;
			s.yaw = RZ_Compass.Bearing(Pos(s), well.m_vPos);
			Add(s, "a house short of food");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! L1: kill sites that were there last look and aren't now (their second dawn).
	protected void KillSites(bool first)
	{
		RZ_Crawlers crawlers = null;
		if (m_GM.GetSpawner())
			crawlers = m_GM.GetSpawner().GetCrawlers();

		if (!crawlers)
			return;

		map<string, vector> now = new map<string, vector>();
		map<string, string> who = new map<string, string>();
		foreach (RZ_KillSite site : crawlers.GetKillSites())
		{
			string key = string.Format("%1|%2", RZ_Util.RoundToInt(site.m_vPos[0]), RZ_Util.RoundToInt(site.m_vPos[2]));
			now.Set(key, site.m_vPos);
			who.Set(key, site.m_sWho);
		}

		if (!first && LiveOn())
		{
			foreach (string k, vector p : m_mSites)
			{
				if (!now.Contains(k))
				{
					string w;
					m_mSiteWho.Find(k, w);
					KillSiteGone(p, w);
				}
			}
		}

		m_mSites = now;
		m_mSiteWho = who;
	}

	//------------------------------------------------------------------------------------------------
	//! L6: Bastion's road posts that were there last look and aren't now.
	protected void Posts(bool first)
	{
		RZ_Checkpoints checkpoints = m_GM.GetCheckpoints();
		if (!checkpoints)
			return;

		array<RZ_RoadPost> posts = {};
		checkpoints.PostsNear(RZ_EFaction.MILITARY, vector.Zero, float.MAX, posts);
		map<int, vector> now = new map<int, vector>();
		foreach (RZ_RoadPost p : posts)
		{
			now.Set(p.m_iId, p.m_vPos);
		}

		if (!first && LiveOn())
		{
			foreach (int id, vector pos : m_mPosts)
			{
				if (!now.Contains(id))
					RoadPostGone(pos);
			}
		}

		m_mPosts = now;
	}

	// ================================================================ Age and props
	//------------------------------------------------------------------------------------------------
	//! The first days by the outbreak's stage (0 Early to 3 Aftermath); dealt scenes by thirds of
	//! their life (0 fresh, 1 weathered, 2 old), or by days for the ones that last the season.
	int Stage(RZ_Scene s)
	{
		if (s.kind.StartsWith("F"))
			return RZ_WorldFacts.Get().Stage();

		if (s.kind.StartsWith("N"))
			return 0;

		float age = RZ_SeasonClock.Days() - s.made;
		if (s.until < 0)
		{
			if (age < 3)
				return 0;

			if (age < 10)
				return 1;

			return 2;
		}

		float life = Math.Max(0.01, s.until - s.made);
		float part = age / life;
		if (part < 0.34)
			return 0;

		if (part < 0.67)
			return 1;

		return 2;
	}

	//------------------------------------------------------------------------------------------------
	protected static string PieceKey(RZ_Scene s, int i)
	{
		return string.Format("traces.scene.%1.%2", s.key, i);
	}

	//------------------------------------------------------------------------------------------------
	//! Its props into the marks spawner (each comes up near players only), or text only.
	protected void Show(RZ_Scene s)
	{
		Hide(s);
		m_mStage.Set(s.key, Stage(s));
		RZ_Marks marks = RZ_Marks.Get();
		if (!marks)
			return;

		array<ResourceName> prefabs = {};
		array<vector> spots = {};
		array<float> yaws = {};
		Pieces(s, prefabs, spots, yaws);
		if (prefabs.IsEmpty() || !PropRoom(s))
			return;

		float days = -1;
		if (s.until >= 0)
			days = Math.Max(0.01, s.until - RZ_SeasonClock.Days());

		bool any = false;
		foreach (int i, ResourceName prefab : prefabs)
		{
			if (!prefab.IsEmpty() && marks.Place(RZ_EMarkPool.TRACES, PieceKey(s, i), spots[i], prefab, "", 2, days, false, false, yaws[i]))
				any = true;
		}

		if (any)
			m_aPropped.Insert(s.key);
	}

	//------------------------------------------------------------------------------------------------
	protected void Hide(RZ_Scene s)
	{
		RZ_Marks marks = RZ_Marks.Get();
		if (marks)
		{
			for (int i = 0; i < PIECES; i++)
			{
				marks.Remove(PieceKey(s, i));
			}
		}

		m_aPropped.RemoveItem(s.key);
	}

	//------------------------------------------------------------------------------------------------
	//! At most 3 scenes with props within 300 m of each other.
	protected bool PropRoom(RZ_Scene s)
	{
		int n = 0;
		foreach (RZ_Scene other : m_aScenes)
		{
			if (other != s && m_aPropped.Contains(other.key) && RZ_Util.DistXZ(Pos(other), Pos(s)) <= PROPS_NEAR)
				n++;
		}

		if (n < PROPS_SCENES)
			return true;

		RZ_Log.Info(string.Format("Traces: prop budget full (%1)", RZ_SettlementScanner.GridReference(Pos(s))));
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static float Yaw(RZ_Scene s, float def)
	{
		if (s.yaw >= 0)
			return s.yaw;

		return def;
	}

	//------------------------------------------------------------------------------------------------
	//! The kit's prefabs for a scene and where each stands.
	protected void Pieces(RZ_Scene s, notnull array<ResourceName> prefabs, notnull array<vector> spots, notnull array<float> yaws)
	{
		RZ_TraceKit kit = m_Traces.GetKit();
		vector at = Pos(s);
		int spin = Math.AbsInt(RZ_Util.RoundToInt(s.x * 7 + s.z * 13));
		spin = spin % 360;
		float yaw = Yaw(s, spin);
		switch (s.kind)
		{
			case "F1":
			{
				// The column in full: spread along the road from the queue's point (400 m back from the
				// barrier) to 1.2 km back, never within 25 m of a junction; nose to tail 9 m apart from the
				// queue's point where the road can't be followed
				array<ResourceName> cars = {kit.m_sQueueCarA, kit.m_sQueueCarB, kit.m_sQueueCarC};
				vector back = Vector(-Math.Sin(yaw * Math.DEG2RAD), 0, -Math.Cos(yaw * Math.DEG2RAD));
				int count = Math.Min(s.a, PIECES);
				array<vector> along = {};
				RZ_TraceColumn.Spread(RZ_WorldFacts.Get().CanonPos("day2.checkpoint"), at, back, count, along);
				for (int c = 0; c < count; c++)
				{
					ResourceName car = cars[c % 3];
					if (car.IsEmpty())
						car = FirstOf(cars);

					vector carAt = at + back * (c * 9);
					float carYaw = yaw;
					if (c < along.Count())
					{
						carAt = along[c];
						carYaw = RZ_Compass.Bearing(carAt, at);
						if (c == 0)
							carYaw = yaw;
					}

					prefabs.Insert(car);
					spots.Insert(RZ_Marks.Verge(carAt, 6, 9));
					yaws.Insert(carYaw);
				}

				return;
			}
			case "F2": Piece(kit.m_sSuitcases, RZ_Marks.Verge(at, 6, 10), yaw, prefabs, spots, yaws); return;
			case "F3":
			{
				if (s.a > 0)
					return;

				Piece(kit.m_sGymDoor, at, yaw, prefabs, spots, yaws);
				Piece(kit.m_sBus, Ground(at, 12, 28), yaw, prefabs, spots, yaws);
				return;
			}
			case "F4": Piece(kit.m_sChurchDoor, at, yaw, prefabs, spots, yaws); return;
			case "F5": Piece(kit.m_sRoadblock, RZ_Marks.Verge(at, 6, 9), yaw, prefabs, spots, yaws); return;
			case "F6": Piece(kit.m_sNoticeWall, at, yaw, prefabs, spots, yaws); return;
			case "F7": Piece(kit.m_sShutter, at, yaw, prefabs, spots, yaws); return;
			case "F8": Piece(kit.m_sObservationPost, at, yaw, prefabs, spots, yaws); return;
			case "N1":
			{
				vector along = Vector(Math.Cos(yaw * Math.DEG2RAD), 0, -Math.Sin(yaw * Math.DEG2RAD));
				for (int k = 0; k < Math.Min(s.a, CROSSES); k++)
				{
					Piece(kit.m_sCross, RZ_Util.GroundPos(at + along * (k * 1.6)), yaw, prefabs, spots, yaws);
				}

				return;
			}
			case "N2":
			{
				Piece(kit.m_sSheets, RZ_Marks.Verge(at, 6, 10), yaw, prefabs, spots, yaws);
				Piece(kit.m_sSheets, RZ_Marks.Verge(at + Vector(15, 0, 10), 7, 12), yaw + 70, prefabs, spots, yaws);
				return;
			}
			case "V1": Piece(kit.m_sLastMeal, at, yaw, prefabs, spots, yaws); return;
			case "V2": Piece(kit.m_sHeldOut, at, yaw, prefabs, spots, yaws); return;
			case "V3": Piece(kit.m_sEdgeChair, at, yaw, prefabs, spots, yaws); return;
			case "V4": Piece(kit.m_sIOUs, at, yaw, prefabs, spots, yaws); return;
			case "V5": Piece(kit.m_sDroppedRun, at, yaw, prefabs, spots, yaws); return;
			case "V6": Piece(kit.m_sWaitingRoom, at, yaw, prefabs, spots, yaws); return;
			case "V7": Piece(kit.m_sFallenPost, at, yaw, prefabs, spots, yaws); return;
			case "V8": Piece(kit.m_sWaterQueue, at, yaw, prefabs, spots, yaws); return;
			case "L2": Piece(kit.m_sScorch, at, yaw, prefabs, spots, yaws); return;
			case "L5": Piece(TruckPrefab(kit, s.faction), at, yaw, prefabs, spots, yaws); return;
			case "L6": Piece(kit.m_sRoadPostRelic, at, yaw, prefabs, spots, yaws); return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void Piece(ResourceName prefab, vector at, float yaw, notnull array<ResourceName> prefabs, notnull array<vector> spots, notnull array<float> yaws)
	{
		if (prefab.IsEmpty())
			return;

		prefabs.Insert(prefab);
		spots.Insert(at);
		yaws.Insert(yaw);
	}

	//------------------------------------------------------------------------------------------------
	protected static ResourceName FirstOf(array<ResourceName> list)
	{
		foreach (ResourceName r : list)
		{
			if (!r.IsEmpty())
				return r;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected static ResourceName TruckPrefab(RZ_TraceKit kit, int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.MILITARY: return kit.m_sTruckBastion;
			case RZ_EFaction.BANDITS: return kit.m_sTruckRedHand;
			case RZ_EFaction.SURVIVORS: return kit.m_sTruckHaven;
			case RZ_EFaction.INSTITUTE: return kit.m_sTruckHalcyon;
		}

		return kit.m_sTruckBastion;
	}

	//------------------------------------------------------------------------------------------------
	//! Every minute: scenes past their day go; a scene whose stage changed is shown again.
	void Prune()
	{
		float today = RZ_SeasonClock.Days();
		for (int i = m_aScenes.Count() - 1; i >= 0; i--)
		{
			RZ_Scene s = m_aScenes[i];
			if (s.until >= 0 && today > s.until)
			{
				Remove(i, "its days are up");
				continue;
			}

			int was;
			if (m_mStage.Find(s.key, was) && was != Stage(s))
				Show(s);
		}
	}

	// ================================================================ Reading
	//------------------------------------------------------------------------------------------------
	//! Once a second: a scene's approach line once a session within 40 m (with the one more line a
	//! background gives), its paper after 2 s standing within 6 m (on foot), the radio within 12 m.
	void SecondTick(array<int> ids, array<IEntity> players)
	{
		if (m_aScenes.IsEmpty())
			return;

		RZ_NoticeLanes lanes = RZ_NoticeLanes.Get();
		foreach (int i, IEntity p : players)
		{
			if (!p)
				continue;

			int pid = ids[i];
			vector pos = p.GetOrigin();
			bool onFoot = !RZ_Util.IsInVehicle(p);
			RZ_Scene paper = null;
			foreach (RZ_Scene s : m_aScenes)
			{
				float d = RZ_Util.DistXZ(Pos(s), pos);
				if (d > APPROACH)
					continue;

				Approach(pid, s, lanes);
				if (!onFoot)
					continue;

				if (d <= RADIO)
					Radio(pid, s);

				if (!paper && d <= PAPER && !RZ_SceneText.Paper(s).IsEmpty())
					paper = s;
			}

			StandAt(pid, paper);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Approach(int pid, RZ_Scene s, RZ_NoticeLanes lanes)
	{
		string key = string.Format("%1|%2", pid, s.key);
		if (m_aSeen.Contains(key))
			return;

		m_aSeen.Insert(key);
		if (lanes && !lanes.TryAmbient(pid, "scene:" + s.key))
			return;

		string line = RZ_SceneText.Approach(s, Stage(s));
		if (line.IsEmpty())
			return;

		string more = RZ_SceneText.More(s, RZ_TraceReader.For(m_GM, pid));
		if (!more.IsEmpty())
			line += " " + more;

		m_GM.NotifyPlayerAs(pid, RZ_SceneText.Title(s.kind), line, RZ_ENoticeTag.LOW);
	}

	//------------------------------------------------------------------------------------------------
	//! S3: the radio in an empty house (the last meal's, the roadblock's until Overrun) says the
	//! broadcast's line once a session.
	protected void Radio(int pid, RZ_Scene s)
	{
		if (!HasRadio(s))
			return;

		string key = string.Format("%1|%2", pid, s.key);
		if (m_aRadio.Contains(key))
			return;

		m_aRadio.Insert(key);
		RZ_Broadcast broadcast = m_GM.GetBroadcast();
		if (!broadcast)
			return;

		m_GM.Say(pid, "A radio", -1, RZ_SceneText.Radio(broadcast.GetText()));
		if (RZ_TraceReader.For(m_GM, pid).Is("radioham"))
			m_GM.NotifyPlayerAs(pid, "The radio", RZ_SceneText.RadioMore(), RZ_ENoticeTag.LOW);
	}

	//------------------------------------------------------------------------------------------------
	//! A scene with a radio that still plays: the last meal's, the roadblock's until Overrun.
	static bool HasRadio(RZ_Scene s)
	{
		if (s.kind == "V1")
			return true;

		return s.kind == "F5" && RZ_WorldFacts.Get().Stage() < RZ_EOutbreakStage.OVERRUN;
	}

	//------------------------------------------------------------------------------------------------
	//! Standing at a scene's paper for 2 s: it opens in the reader (UI > Reader), else as a notice.
	protected void StandAt(int pid, RZ_Scene s)
	{
		if (!s)
		{
			m_mPaperAt.Remove(pid);
			return;
		}

		string was;
		int seconds = 0;
		if (m_mPaperAt.Find(pid, was) && was == s.key)
			m_mPaperSeconds.Find(pid, seconds);

		seconds++;
		m_mPaperAt.Set(pid, s.key);
		m_mPaperSeconds.Set(pid, seconds);
		if (seconds < PAPER_SECONDS)
			return;

		string key = string.Format("%1|%2", pid, s.key);
		if (m_aPaperRead.Contains(key))
			return;

		m_aPaperRead.Insert(key);
		string text = RZ_SceneText.Paper(s);
		if (RZ_UISettings.Get().m_bReader)
		{
			RZ_Reader.Show(m_GM, pid, RZ_SceneText.Title(s.kind), RZ_SceneText.PaperHow(s.kind), text);
			return;
		}

		string flat = text;
		flat.Replace("\n\n", " ");
		flat.Replace("\n", " ");
		m_GM.NotifyPlayerAs(pid, RZ_SceneText.PaperHow(s.kind), flat, RZ_ENoticeTag.LOW);
	}

	// ================================================================ The cure and the epilogue
	//------------------------------------------------------------------------------------------------
	//! At the commit: the last four wrecks become the epilogue's for the week; the rest finish their
	//! days and nothing new is dealt. How many wrecks were handed over.
	int HandOver()
	{
		int handed = 0;
		for (int pass = 0; pass < 4; pass++)
		{
			RZ_Scene newest = null;
			foreach (RZ_Scene s : m_aScenes)
			{
				if (s.kind == "L5" && !s.epilogue && (!newest || s.made > newest.made))
					newest = s;
			}

			if (!newest)
				break;

			newest.epilogue = true;
			newest.until = -1;
			Show(newest);
			handed++;
		}

		return handed;
	}

	//------------------------------------------------------------------------------------------------
	//! The scenes within `radius` of pos (the admin's /traces), to the server log. How many.
	int LogNear(vector pos, float radius)
	{
		int n = 0;
		foreach (RZ_Scene s : m_aScenes)
		{
			if (RZ_Util.DistXZ(Pos(s), pos) > radius)
				continue;

			string props = "text";
			if (m_aPropped.Contains(s.key))
				props = "props";

			RZ_Log.Info(string.Format("Traces: scene %1 (%2) at %3, stage %4, %5 | %6", s.key, s.kind, RZ_SettlementScanner.GridReference(Pos(s)), Stage(s), props, RZ_SceneText.Approach(s, Stage(s))));
			n++;
		}

		return n;
	}
}

//------------------------------------------------------------------------------------------------
//! Placement's last check (docs/TRACES.md, "Where scenes stand"): a scene's prop never comes up while
//! a player or a vehicle is within 8 m of its spot (RZ_Marks asks before it spawns a Traces prop).
class RZ_TraceSpotClear
{
	static const float CLEAR = 8;

	protected bool m_bBlocked;

	//------------------------------------------------------------------------------------------------
	static bool IsClear(vector at, float closestPlayer)
	{
		if (closestPlayer <= CLEAR)
			return false;

		RZ_TraceSpotClear q = new RZ_TraceSpotClear();
		GetGame().GetWorld().QueryEntitiesBySphere(at, CLEAR, q.OnEntity, null, EQueryEntitiesFlags.DYNAMIC);
		return !q.m_bBlocked;
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnEntity(IEntity e)
	{
		if (Vehicle.Cast(e))
		{
			m_bBlocked = true;
			return false;
		}

		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! The column in full (F1, docs/TRACES.md, "The first days and the line"): where its cars stand on
//! the road back from the barrier, 400 m to 1.2 km, at least 25 m from a junction (a place three road
//! ends meet). Read once from the road network when the scene's props are placed.
class RZ_TraceColumn
{
	static const float NEAR = 400;
	static const float FAR = 1200;
	static const float JUNCTION = 25;

	//------------------------------------------------------------------------------------------------
	//! count road points, the first at the queue's point, the rest spread to 1.2 km back from the
	//! barrier `cp`; fewer (or none) where the road can't be followed: the caller packs the rest.
	static void Spread(vector cp, vector queue, vector back, int count, notnull array<vector> result)
	{
		RoadNetworkManager roads = RZ_Perimeter.GetRoadNetwork();
		if (!roads || cp == vector.Zero || count <= 1)
			return;

		vector mid = queue + back * ((FAR - NEAR) * 0.5);
		float reach = FAR - NEAR;
		vector mins = Vector(mid[0] - reach, -1000, mid[2] - reach);
		vector maxs = Vector(mid[0] + reach, 10000, mid[2] + reach);
		array<BaseRoad> found = {};
		roads.GetRoadsInAABB(mins, maxs, found);
		array<vector> points = {};
		array<vector> ends = {};
		foreach (BaseRoad road : found)
		{
			if (!road)
				continue;

			array<vector> pts = {};
			road.GetPoints(pts);
			if (pts.IsEmpty())
				continue;

			ends.Insert(pts[0]);
			ends.Insert(pts[pts.Count() - 1]);
			foreach (vector pt : pts)
			{
				points.Insert(pt);
			}
		}

		result.Insert(queue);
		for (int c = 1; c < count; c++)
		{
			float want = NEAR + (FAR - NEAR) * c / (count - 1);
			vector line = cp + Dir(cp, queue) * want;
			vector best = vector.Zero;
			float bestScore = 200;
			foreach (vector p : points)
			{
				float d = RZ_Util.DistXZ(p, cp);
				if (d < NEAR || d > FAR + 50 || AtJunction(p, ends))
					continue;

				float score = Math.AbsFloat(d - want) + RZ_Util.DistXZ(p, line) * 0.5;
				if (score < bestScore)
				{
					bestScore = score;
					best = p;
				}
			}

			if (best == vector.Zero)
				return;

			result.Insert(RZ_Util.GroundPos(best));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Three road ends within 25 m of p (two only join one road's pieces): a junction.
	protected static bool AtJunction(vector p, array<vector> ends)
	{
		int near = 0;
		foreach (vector e : ends)
		{
			if (RZ_Util.DistXZ(e, p) <= JUNCTION)
				near++;

			if (near >= 3)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static vector Dir(vector from, vector to)
	{
		vector d = to - from;
		d[1] = 0;
		if (d.Length() < 0.01)
			return Vector(0, 0, 1);

		d.Normalize();
		return d;
	}
}
