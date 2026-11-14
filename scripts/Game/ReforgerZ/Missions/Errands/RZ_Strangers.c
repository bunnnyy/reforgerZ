//! Strangers (docs/ERRANDS.md, "Strangers"): a passer-by who asks one small thing, met only where
//! one would make sense (the author's decision 14), never because a timer ran out. Spots are data
//! from facts the server already has, looked for 150-350 m ahead along the way a player is walking
//! (a stopped car: at the car):
//! - a **road people use**: a 250 m footfall cell players walked a lot lately, or a wanderer's leg;
//!   by day, at night only within 300 m of a fire or a held place;
//! - a **crossroads**: 80-300 m from a road post, outside its barrier, any hour while it stands;
//! - a **wanderer's route**: within 300 m of the leg a living wanderer is walking;
//! - an **approach**: 100-800 m out from a safehouse or a holding, outside its wire;
//! - a **campfire**: within 60 m of a stove fire lit in the last 30 minutes, or a Red Hand road
//!   post's brazier after dark;
//! - a **stopped car**: your own car, stopped 20 s with you out of it, or a wreck on a road people use;
//! - a **fresh event**: within 400 m of a bite, death, fire or shelling in the last 2 played hours,
//!   or a job's site just after it ended (a found hook);
//! - a **town edge at dusk** (fog counts as dusk): within 100 m of a settlement's edge;
//! - **their own place**: the place a routine's Why names (his farm), by day;
//! - **the line**: the 150 m warning point on a road into a sealed checkpoint.
//! A stranger routine also needs its Why to be true. Never: another player outside your group within
//! 300 m, within 2 km of the Long Table's farm, driving, a shot within 60 m in 60 s, infected within
//! 30 m, a running mission's 300 m, a storm (in rain only approaches and fires), the first 5
//! minutes of a life, a lair within 600 m, a day party's range by day, a range at night away from a
//! fire or a held place, a spot that gave anyone a stranger in 2 real hours or you one in a real
//! day. The ceiling only stops one: one per player every 15-25 minutes of play, 6 on the server (the
//! body budget). They call out once, wait 5 minutes or until you're 150 m away (longer while you do
//! what they asked), and pay nothing but a line, one common find, or the house's few Favor. Killing
//! one costs what killing a wanderer costs, and no stranger finds you for a real day. Server only.

//------------------------------------------------------------------------------------------------
//! The spot kinds (bits), docs/ERRANDS.md "Where a stranger can find you".
class RZ_StrangerSpot
{
	static const int ROAD = 1;
	static const int CROSSROADS = 2;
	static const int ROUTE = 4;
	static const int APPROACH = 8;
	static const int FIRE = 16;
	static const int CAR = 32;
	static const int FRESH = 64;
	static const int EDGE = 128;
	static const int OWN = 256;
	static const int LINE = 512;

	//------------------------------------------------------------------------------------------------
	static string Names(int kinds)
	{
		array<string> names = {"road", "crossroads", "route", "approach", "fire", "car", "fresh", "edge", "own", "line"};
		string text = "";
		foreach (int i, string n : names)
		{
			if ((kinds & (1 << i)) == 0)
				continue;

			if (!text.IsEmpty())
				text += ", ";

			text += n;
		}

		if (text.IsEmpty())
			return "none";

		return text;
	}
}

//------------------------------------------------------------------------------------------------
//! What made a point a spot, found once per look ahead.
class RZ_StrangerHere
{
	int m_iKinds;
	vector m_vAt;			// where the stranger would stand
	vector m_vCar;			// the car or the wreck
	bool m_bOwnCar;			// the player's own car (else a wreck)
	vector m_vFire;
	vector m_vLine;			// the warning point
	int m_iPost = -1;		// the road post of a crossroads (-1: a junction of two used roads)
	int m_iWanderer = -1;	// whose route
	string m_sFresh;		// the fresh signal's kind ("death", "bite", "hook:<kind>")
	string m_sFreshText;
	vector m_vFresh;
}

//------------------------------------------------------------------------------------------------
//! One stranger out in the world, for one player (and their group).
class RZ_StrangerVisit
{
	int m_iId;
	int m_iAsker;
	RZ_StrangerKind m_Kind;		// owned by RZ_Strangers
	int m_iSpot;				// the spot kind it was dealt at (one bit)
	vector m_vPos;
	string m_sSpot;				// its cooldown key
	int m_iHouse = -1;			// their house (or the one they're going to)
	float m_fDeadAt = -1;		// the dealer's clock when they died (the robbery window)
	int m_iPockets;				// what the body carried then
	bool m_bRobbed;
	float m_fUnseenWait;		// seconds waited to go out of sight
	int m_iValue = -1;			// the routine's own number
	vector m_vSubject;			// the routine's own place
	string m_sSubject;
	bool m_bTwist;				// the routine's twist is true this time
	ref map<string, string> m_mText = new map<string, string>();
	IEntity m_Body;
	float m_fAge;
	bool m_bCalled;
	bool m_bDone;
	// A way that asks you to go somewhere or stay a while
	int m_iStep = -1;			// the option being done, -1 none
	vector m_vStep;
	float m_fStepRadius = 20;
	float m_fStepNeed;
	float m_fStepDone;
	bool m_bStepUnseen;
	float m_fStepUntil;			// the visit's clock

	//------------------------------------------------------------------------------------------------
	void SetText(string key, string value)
	{
		m_mText.Set(key, value);
	}

	//------------------------------------------------------------------------------------------------
	string Fill(string text)
	{
		string s = text;
		foreach (string key, string value : m_mText)
		{
			s.Replace("{" + key + "}", value);
		}

		return RZ_WorldFacts.Get().Fill(s);
	}
}

//------------------------------------------------------------------------------------------------
//! A found hook (docs/ERRANDS.md, "Found hooks"): one job site in five, just after its job, holds
//! a stranger for a while (someone hiding, someone who got there first, a bite on a stranger), or
//! an old mark nobody asks about.
class RZ_FoundHook
{
	string kind;		// hiding, first, bitten, mark
	vector at;
	float until;		// the strangers' clock
	string text;		// an old mark: its door's key in Traces' grammar (RZ_Traces.OldMarkKey)
	bool read;
}

//------------------------------------------------------------------------------------------------
//! "Come back at dawn" (Behind the Door): they're there at dawn, if nothing changed.
class RZ_StrangerLater
{
	int playerId;
	vector at;
	int house = -1;
	float until;		// the strangers' clock
}

//------------------------------------------------------------------------------------------------
class RZ_Strangers
{
	static const int TAG = -14;		// RZ_PuzzleObjectComponent mission id: a stranger

	protected static const float CELL = 250;
	protected static const float ROAD_FOOTFALL = 20;	// player-ticks in a cell for a road people use
	protected static const float WAIT = 300;
	protected static const float LEAVE = 150;
	protected static const float CALL_RANGE = 40;
	protected static const float EARSHOT = 60;		// a stranger is heard this far; further, a card
	//! Strangers out at once: the setting, never over the body budget's strangers.
	protected static int MaxLive()
	{
		return Math.MinInt(RZ_ErrandSettings.Get().m_iStrangersMax, RZ_BodyBudget.STRANGER_CAP);
	}

	protected static const float SPOT_COOL = 7200;		// a spot that gave anyone a stranger
	protected static const float SPOT_COOL_MINE = 86400;	// a spot that gave you one
	protected static const float FIRE_FRESH = 1800;		// a stove fire counts 30 minutes
	protected static const float CAR_STOPPED = 20;
	protected static const float HOOK_LIFE = 7200;		// a found hook stays fresh 2 hours
	protected static const float STEP_LIFE = 600;		// a way under way waits 10 real minutes
	protected static const float KILLED_PAUSE = 86400;	// no stranger for a real day after you kill one

	protected RZ_GameModeComponent m_GM;
	protected RZ_Errands m_Errands;
	protected ref array<ref RZ_StrangerKind> m_aKinds = {};
	protected ref map<int, float> m_mFootfall = new map<int, float>();
	protected ref map<int, vector> m_mLastPos = new map<int, vector>();
	protected ref map<string, float> m_mNext = new map<string, float>();		// ident -> clock of the next allowed
	protected ref map<string, float> m_mSpotCool = new map<string, float>();	// spot -> clock
	protected ref array<ref RZ_StrangerVisit> m_aVisits = {};
	protected ref array<vector> m_aFires = {};
	protected ref array<float> m_aFireAt = {};
	protected ref map<int, IEntity> m_mCar = new map<int, IEntity>();		// player -> the last car they sat in
	protected ref map<int, vector> m_mCarPos = new map<int, vector>();
	protected ref map<int, float> m_mCarStill = new map<int, float>();		// seconds it hasn't moved
	protected ref array<ref RZ_FoundHook> m_aHooks = {};
	protected ref array<ref RZ_StrangerLater> m_aLater = {};
	protected ref map<int, int> m_mGate = new map<int, int>();				// safehouse -> someone at the gate (1, 2 bitten)
	protected ref map<int, vector> m_mJunction = new map<int, vector>();	// footfall cell -> its junction (zero: none)
	protected static const float ROB_WINDOW = 120;
	protected static const int ROB_INFAMY = 8;
	protected ref map<string, int> m_mDealt = new map<string, int>();		// telemetry: dealt by spot kind
	protected ref map<string, int> m_mHelped = new map<string, int>();		// telemetry: helped by routine
	protected int m_iNextId = 1;
	protected float m_fClock;

	//------------------------------------------------------------------------------------------------
	void RZ_Strangers(RZ_GameModeComponent gm, RZ_Errands errands)
	{
		m_GM = gm;
		m_Errands = errands;
		RZ_StrangerKind.Build(m_aKinds);
		foreach (RZ_StrangerKind k : m_aKinds)
		{
			k.Define();
		}
	}

	RZ_GameModeComponent GetGM() { return m_GM; }
	RZ_Errands GetErrands() { return m_Errands; }
	float GetClock() { return m_fClock; }

	// ================================================================ The footfall grid
	//------------------------------------------------------------------------------------------------
	protected static int CellKey(vector pos)
	{
		int cx = Math.Floor(pos[0] / CELL);
		int cz = Math.Floor(pos[2] / CELL);
		return cx * 100000 + cz;
	}

	//------------------------------------------------------------------------------------------------
	float Footfall(vector pos)
	{
		float v;
		m_mFootfall.Find(CellKey(pos), v);
		return v;
	}

	//------------------------------------------------------------------------------------------------
	protected void Walk(float dt, array<IEntity> players)
	{
		float keep = Math.Pow(0.5, dt / Math.Max(3600, RZ_ErrandSettings.Get().m_fFootfallHours * 3600));
		array<int> keys = {};
		array<float> values = {};
		foreach (int key, float v : m_mFootfall)
		{
			keys.Insert(key);
			values.Insert(v * keep);
		}

		foreach (int k, int cellKey : keys)
		{
			if (values[k] < 0.5)
				m_mFootfall.Remove(cellKey);
			else
				m_mFootfall.Set(cellKey, values[k]);
		}

		foreach (IEntity p : players)
		{
			int cell = CellKey(p.GetOrigin());
			float was;
			m_mFootfall.Find(cell, was);
			m_mFootfall.Set(cell, was + 1);
		}
	}

	// ================================================================ Facts the spots read
	//------------------------------------------------------------------------------------------------
	//! A stove fire lit (RZ_Crafting): a campfire spot for 30 minutes.
	void NoteFire(vector pos)
	{
		m_aFires.Insert(pos);
		m_aFireAt.Insert(m_fClock);
		while (m_aFires.Count() > 16)
		{
			m_aFires.RemoveOrdered(0);
			m_aFireAt.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A job ended at `at` (RZ_MissionManager): one site in five holds a found hook for 2 hours.
	void JobEnded(RZ_Mission m)
	{
		RZ_ErrandSettings settings = RZ_ErrandSettings.Get();
		if (!m || m.m_bGlobal || m.m_vPos == vector.Zero || Math.RandomFloat01() >= settings.m_fHookChance)
			return;

		RZ_FoundHook h = new RZ_FoundHook();
		h.at = m.m_vPos;
		h.until = m_fClock + HOOK_LIFE;
		float roll = Math.RandomFloat01();
		if (roll < 0.3)
			h.kind = "hiding";
		else if (roll < 0.5)
			h.kind = "first";
		else if (roll < 0.7)
			h.kind = "bitten";
		else
		{
			// An old mark comes from Traces' grammar (decision 15): none without it, or without a town
			h.kind = "mark";
			h.text = RZ_Traces.OldMarkKey(m.m_vPos);
			if (h.text.IsEmpty())
				return;
		}

		m_aHooks.Insert(h);
		while (m_aHooks.Count() > 12)
		{
			m_aHooks.RemoveOrdered(0);
		}

		RZ_Log.Info(string.Format("Errands: found hook %1 at %2 after %3", h.kind, RZ_SettlementScanner.GridReference(h.at), m.m_sTitle));
	}

	//------------------------------------------------------------------------------------------------
	//! A found hook put down by another system (a patrol marks a house where someone hides): the
	//! same hook a finished job leaves, for 2 hours.
	void PlantHook(string kind, vector at)
	{
		RZ_FoundHook h = new RZ_FoundHook();
		h.kind = kind;
		h.at = at;
		h.until = m_fClock + HOOK_LIFE;
		m_aHooks.Insert(h);
		while (m_aHooks.Count() > 12)
		{
			m_aHooks.RemoveOrdered(0);
		}

		RZ_Log.Info(string.Format("Errands: found hook %1 at %2 from a duty", kind, RZ_SettlementScanner.GridReference(at)));
	}

	//------------------------------------------------------------------------------------------------
	//! A found hook of `kind` within `radius` of pos, still fresh; null if none.
	RZ_FoundHook HookNear(string kind, vector pos, float radius)
	{
		foreach (RZ_FoundHook h : m_aHooks)
		{
			if (h.kind == kind && h.until > m_fClock && RZ_Util.DistXZ(h.at, pos) <= radius)
				return h;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	void UseHook(RZ_FoundHook h)
	{
		if (h)
			h.until = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Someone waits at a safehouse's gate (Who's at the Gate): 1 a stranger, 2 a bitten one.
	void SendToGate(int house, bool bitten)
	{
		if (house < 0)
			return;

		int v = 1;
		if (bitten)
			v = 2;

		m_mGate.Set(house, v);
	}

	//------------------------------------------------------------------------------------------------
	int AtGate(int house)
	{
		int v;
		m_mGate.Find(house, v);
		return v;
	}

	//------------------------------------------------------------------------------------------------
	void ClearGate(int house)
	{
		m_mGate.Remove(house);
	}

	// ================================================================ The tick (the errands' 30 s)
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players, bool dealing)
	{
		m_fClock += dt;
		Walk(dt, players);
		WatchCars(dt, ids, players);
		KeepVisits(ids, players, dt);
		MarkLines(ids, players);
		KeepLater(ids, players);
		if (dealing)
		{
			foreach (int i, IEntity p : players)
			{
				LookAhead(ids[i], p, ids, players);
			}
		}

		foreach (int j, IEntity q : players)
		{
			m_mLastPos.Set(ids[j], q.GetOrigin());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Each player's car: the last one they sat in, and how long it has stood still.
	protected void WatchCars(float dt, array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity p : players)
		{
			int pid = ids[i];
			IEntity car = RZ_Util.GetVehicleOf(p);
			if (car)
			{
				m_mCar.Set(pid, car);
				m_mCarPos.Set(pid, car.GetOrigin());
				m_mCarStill.Set(pid, 0);
				continue;
			}

			IEntity mine;
			if (!m_mCar.Find(pid, mine) || !mine)
				continue;

			vector was;
			m_mCarPos.Find(pid, was);
			float still;
			m_mCarStill.Find(pid, still);
			if (RZ_Util.DistXZ(was, mine.GetOrigin()) > 2)
				still = 0;
			else
				still += dt;

			m_mCarPos.Set(pid, mine.GetOrigin());
			m_mCarStill.Set(pid, still);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! An old mark at a found hook's site: read once by whoever passes within 30 m.
	protected void MarkLines(array<int> ids, array<IEntity> players)
	{
		foreach (RZ_FoundHook h : m_aHooks)
		{
			if (h.kind != "mark" || h.read || h.until < m_fClock)
				continue;

			foreach (int i, IEntity p : players)
			{
				if (RZ_Util.DistXZ(p.GetOrigin(), h.at) > 30)
					continue;

				// Read at this reader's level: what a Police Officer or a Hunter sees in it is theirs
				string line = RZ_Traces.OldMarkLine(h.text, ids[i]);
				h.read = true;
				if (!line.IsEmpty())
					m_GM.NotifyPlayer(ids[i], "An old mark", line);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A promise to come back at dawn: kept for 12 real hours.
	void Later(RZ_StrangerVisit v, int playerId)
	{
		RZ_StrangerLater l = new RZ_StrangerLater();
		l.playerId = playerId;
		l.at = v.m_vPos;
		l.house = v.m_iHouse;
		l.until = m_fClock + 43200;
		m_aLater.Insert(l);
	}

	//------------------------------------------------------------------------------------------------
	//! Dawn (05:00-08:00), and you came back within 40 m: they come out and go to the house.
	protected void KeepLater(array<int> ids, array<IEntity> players)
	{
		float hour = RZ_GameTime.GetHour();
		for (int i = m_aLater.Count() - 1; i >= 0; i--)
		{
			RZ_StrangerLater l = m_aLater[i];
			if (m_fClock > l.until)
			{
				m_aLater.Remove(i);
				continue;
			}

			int idx = ids.Find(l.playerId);
			if (idx < 0 || hour < 5 || hour > 8 || RZ_Util.DistXZ(players[idx].GetOrigin(), l.at) > 40)
				continue;

			ToHouse(l.house);
			m_GM.NotifyPlayer(l.playerId, "A stranger", string.Format("You came. You actually came. I'll walk to %1 now, in the grey light, and I won't stop once.", HouseName(l.house)));
			m_Errands.SmallFavor(l.playerId, RZ_EFaction.SURVIVORS, 3);
			m_aLater.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A stranger ahead of this player, if a spot and a Why fit and nothing says no.
	protected void LookAhead(int playerId, IEntity me, array<int> ids, array<IEntity> players)
	{
		vector last;
		if (!m_mLastPos.Find(playerId, last) || m_aVisits.Count() >= MaxLive())
			return;

		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		float next;
		if (m_mNext.Find(ident, next) && m_fClock < next)
			return;

		if (!Allowed(playerId, me, ids, players))
			return;

		vector pos = me.GetOrigin();

		// Your own car, stopped 20 s with you out of it: someone walks up from the verge
		IEntity car;
		float still;
		if (m_mCar.Find(playerId, car) && car && m_mCarStill.Find(playerId, still) && still >= CAR_STOPPED && RZ_Util.DistXZ(car.GetOrigin(), pos) <= 30)
		{
			RZ_StrangerHere atCar = new RZ_StrangerHere();
			atCar.m_iKinds = RZ_StrangerSpot.CAR;
			atCar.m_vCar = car.GetOrigin();
			atCar.m_bOwnCar = true;
			atCar.m_vAt = RZ_Mission.LandPointNear(car.GetOrigin(), 60, 90);
			if (TryDeal(playerId, ident, atCar))
				return;
		}

		vector move = pos - last;
		move[1] = 0;
		float moved = move.Length();
		if (moved < 20)
			return;		// standing still meets nobody

		vector dir = move / moved;
		array<float> ahead = {150, 250, 350};
		foreach (float d : ahead)
		{
			vector at = RZ_Util.GroundPos(pos + dir * d);
			RZ_StrangerHere here = Find(at, playerId);
			if (here.m_iKinds != 0 && TryDeal(playerId, ident, here))
				return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! What never triggers one, for this player now.
	protected bool Allowed(int playerId, IEntity me, array<int> ids, array<IEntity> players)
	{
		vector pos = me.GetOrigin();
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		if (profile.m_fLifeSeconds < 300 || RZ_Util.IsInVehicle(me))
			return false;

		foreach (int i, IEntity p : players)
		{
			if (ids[i] != playerId && !RZ_Util.SameGroup(playerId, ids[i]) && RZ_Util.DistXZ(p.GetOrigin(), pos) <= 300)
				return false;
		}

		RZ_Environment env = m_GM.GetEnvironment();
		if (env && env.IsStorm())
			return false;

		RZ_LongTable table = m_GM.GetLongTable();
		if (table && table.Exists() && RZ_Util.DistXZ(table.GetHousePos(), pos) <= 2000)
			return false;

		if (m_Errands.RecentShot(pos, 60, 60) || (m_GM.GetSpawner() && m_GM.GetSpawner().IsZombieNear(pos, 30)))
			return false;

		return m_Errands.QuietHere(pos);
	}

	//------------------------------------------------------------------------------------------------
	//! The infected's own ground: a lair within 600 m; a range by day while its party is out; a range
	//! at night away from a fire or a held place.
	protected bool TheirGround(vector at, bool fireOrHeld)
	{
		RZ_Tribes tribes = m_GM.GetTribes();
		if (!tribes)
			return false;

		RZ_Tribe t = tribes.NearestTribe(at, 3000);
		if (!t)
			return false;

		float d = RZ_Util.DistXZ(t.m_vLair, at);
		if (d <= 600)
			return true;

		if (d > t.m_fRadius)
			return false;

		if (RZ_GameTime.IsNight())
			return !fireOrHeld;

		return t.m_bDayOut;
	}

	// ================================================================ Spots
	//------------------------------------------------------------------------------------------------
	//! Every spot kind plausible at `at` now.
	RZ_StrangerHere Find(vector at, int playerId)
	{
		RZ_StrangerHere here = new RZ_StrangerHere();
		here.m_vAt = at;
		bool night = RZ_GameTime.IsNight();
		RZ_Environment env = m_GM.GetEnvironment();
		bool rain = env && env.IsRaining();

		if (Approach(at))
			here.m_iKinds |= RZ_StrangerSpot.APPROACH;

		bool fire = FireNear(at, here);
		if (fire)
			here.m_iKinds |= RZ_StrangerSpot.FIRE;

		bool held = Held(at);
		if (TheirGround(at, fire || held))
			return new RZ_StrangerHere();

		if (rain)
			return here;	// in rain, only approaches and fires

		int route = RouteNear(at);
		if (route >= 0)
		{
			here.m_iKinds |= RZ_StrangerSpot.ROUTE;
			here.m_iWanderer = route;
		}

		bool used = Footfall(at) >= ROAD_FOOTFALL || route >= 0;
		if (used && (!night || fire || held))
			here.m_iKinds |= RZ_StrangerSpot.ROAD;

		int post = PostNear(at);
		if (post >= 0)
		{
			here.m_iKinds |= RZ_StrangerSpot.CROSSROADS;
			here.m_iPost = post;
		}
		else if (JunctionNear(at))
		{
			here.m_iKinds |= RZ_StrangerSpot.CROSSROADS;
		}

		if (FreshNear(at, playerId, here))
			here.m_iKinds |= RZ_StrangerSpot.FRESH;

		if (TownEdge(at))
			here.m_iKinds |= RZ_StrangerSpot.EDGE;

		if (!night && used && WreckNear(at, here))
			here.m_iKinds |= RZ_StrangerSpot.CAR;

		if (LineNear(at, here))
			here.m_iKinds |= RZ_StrangerSpot.LINE;

		if (!night)
			here.m_iKinds |= RZ_StrangerSpot.OWN;	// a routine's own place: its Why checks the place

		return here;
	}

	//------------------------------------------------------------------------------------------------
	protected bool FreshNear(vector at, int playerId, RZ_StrangerHere here)
	{
		array<string> kinds = {"death", "bite", "fire", "shell"};
		foreach (string kind : kinds)
		{
			RZ_Signal s = RZ_ErrandCard.FreshSignal(kind, at, 400, 2, playerId);
			if (s)
			{
				here.m_sFresh = kind;
				here.m_sFreshText = s.text;
				here.m_vFresh = Vector(s.x, 0, s.z);
				return true;
			}
		}

		foreach (RZ_FoundHook h : m_aHooks)
		{
			if (h.kind != "mark" && h.until > m_fClock && RZ_Util.DistXZ(h.at, at) <= 400)
			{
				here.m_sFresh = "hook:" + h.kind;
				here.m_vFresh = h.at;
				return true;
			}
		}

		// A live trace's place (Traces' L1 kill-site candle, L2 scorched doorway) counts as fresh
		vector sceneAt;
		string sceneKey;
		if (RZ_ErrandCard.OnScene("L1", at, 400, sceneAt, sceneKey))
		{
			here.m_sFresh = "death";
			here.m_sFreshText = "someone who died here lately";
			here.m_vFresh = sceneAt;
			return true;
		}

		if (RZ_ErrandCard.OnScene("L2", at, 400, sceneAt, sceneKey))
		{
			here.m_sFresh = "fire";
			here.m_sFreshText = "a fire here lately";
			here.m_vFresh = sceneAt;
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool TownEdge(vector at)
	{
		RZ_Environment env = m_GM.GetEnvironment();
		bool dusk = RZ_WorldFacts.Get().IsDusk() || (env && env.IsFoggy());
		if (!dusk)
			return false;

		int idx = m_GM.FindNearestSettlement(at);
		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (!s)
			return false;

		return Math.AbsFloat(RZ_Util.DistXZ(s.m_vCenter, at) - s.m_fRadius) <= 100;
	}

	//------------------------------------------------------------------------------------------------
	protected bool Approach(vector at)
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
		{
			RZ_Safehouse h = houses.Get(houses.FindNearest(at, 800));
			if (h && RZ_Util.DistXZ(h.m_vPos, at) >= 100)
				return true;
		}

		RZ_Territory territory = m_GM.GetTerritory();
		if (!territory)
			return false;

		foreach (RZ_Holding hold : territory.GetHoldings())
		{
			float d = RZ_Util.DistXZ(hold.m_vPos, at);
			if (d >= 100 && d <= 800)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Within 300 m of a safehouse or a holding.
	protected bool Held(vector at)
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses && houses.FindNearest(at, 300) >= 0)
			return true;

		RZ_Territory territory = m_GM.GetTerritory();
		if (!territory || !territory.IsActive())
			return false;

		RZ_Holding hold = territory.FindHolding(-1, at, 300);
		return hold != null;
	}

	//------------------------------------------------------------------------------------------------
	//! A stove fire lit in the last 30 minutes within 60 m, or a Red Hand road post's brazier after dark.
	protected bool FireNear(vector at, RZ_StrangerHere here)
	{
		for (int i = m_aFires.Count() - 1; i >= 0; i--)
		{
			if (m_fClock - m_aFireAt[i] <= FIRE_FRESH && RZ_Util.DistXZ(m_aFires[i], at) <= 60)
			{
				here.m_vFire = m_aFires[i];
				return true;
			}
		}

		RZ_Checkpoints posts = m_GM.GetCheckpoints();
		if (!posts || !RZ_GameTime.IsNight())
			return false;

		array<RZ_RoadPost> near = {};
		posts.PostsNear(RZ_EFaction.BANDITS, at, 120, near);
		foreach (RZ_RoadPost p : near)
		{
			if (RZ_Util.DistXZ(p.m_vPos, at) >= 40)
			{
				here.m_vFire = p.m_vPos;
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A living wanderer's leg (where they are to where they're going) within 300 m: the wanderer's id.
	protected int RouteNear(vector at)
	{
		RZ_Wanderers wanderers = m_GM.GetWanderers();
		if (!wanderers)
			return -1;

		array<int> living = {};
		wanderers.GetLiving(RZ_Wanderers.AllRoles(), living);
		foreach (int id : living)
		{
			RZ_Wanderer w = wanderers.FindById(id);
			if (!w || w.m_bSettled || w.m_vDest == vector.Zero)
				continue;

			if (DistToSegment(at, w.m_vPos, w.m_vDest) <= 300)
				return id;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! A junction of two used roads 80-300 m off (three road ends within 25 m of each other, the footfall
	//! there a road people use), from the road network, cached per 250 m cell. VERIFY: reading junctions
	//! from the road network (a junction's roads end at it).
	protected bool JunctionNear(vector at)
	{
		int cell = CellKey(at);
		vector j;
		if (!m_mJunction.Find(cell, j))
		{
			j = FindJunction(at);
			m_mJunction.Set(cell, j);
		}

		if (j == vector.Zero)
			return false;

		float d = RZ_Util.DistXZ(at, j);
		return d >= 80 && d <= 300 && Footfall(j) >= ROAD_FOOTFALL;
	}

	//------------------------------------------------------------------------------------------------
	protected static vector FindJunction(vector at)
	{
		RoadNetworkManager roads = RZ_Perimeter.GetRoadNetwork();
		if (!roads)
			return vector.Zero;

		array<BaseRoad> found = {};
		roads.GetRoadsInAABB(Vector(at[0] - 300, -1000, at[2] - 300), Vector(at[0] + 300, 10000, at[2] + 300), found);
		array<vector> ends = {};
		foreach (BaseRoad road : found)
		{
			if (!road)
				continue;

			array<vector> pts = {};
			road.GetPoints(pts);
			if (pts.Count() < 2)
				continue;

			ends.Insert(pts[0]);
			ends.Insert(pts[pts.Count() - 1]);
		}

		foreach (vector e : ends)
		{
			int near = 0;
			foreach (vector o : ends)
			{
				if (RZ_Util.DistXZ(e, o) <= 25)
					near++;
			}

			if (near >= 3)
				return e;
		}

		return vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	//! A road post 80-300 m off: its crossroads or approach, outside the barrier.
	protected int PostNear(vector at)
	{
		RZ_Checkpoints posts = m_GM.GetCheckpoints();
		if (!posts)
			return -1;

		array<RZ_RoadPost> near = {};
		posts.PostsNear(-1, at, 300, near);
		foreach (RZ_RoadPost p : near)
		{
			if (RZ_Util.DistXZ(p.m_vPos, at) >= 80)
				return p.m_iId;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! A wreck (an interaction spot) within 150 m.
	protected bool WreckNear(vector at, RZ_StrangerHere here)
	{
		RZ_Interactions hub = RZ_Interactions.Get();
		if (!hub)
			return false;

		RZ_Spot s = hub.NearestSpot(RZ_Interactions.KIND_WRECK, at, 150);
		if (!s)
			return false;

		here.m_vCar = s.m_vPos;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The 150 m warning point of a sealed checkpoint within 300 m, while the Zone is sealed.
	protected bool LineNear(vector at, RZ_StrangerHere here)
	{
		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		if (!perimeter || !perimeter.IsActive() || !RZ_WorldFacts.Get().PerimeterSealed())
			return false;

		vector cp;
		if (!perimeter.GetNearestCheckpoint(at, cp) || RZ_Util.DistXZ(cp, at) > 450)
			return false;

		vector warn = perimeter.PointInside(cp, perimeter.CheckpointInward(cp), 150);
		if (RZ_Util.DistXZ(warn, at) > 300)
			return false;

		here.m_vLine = warn;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static float DistToSegment(vector p, vector a, vector b)
	{
		float abx = b[0] - a[0];
		float abz = b[2] - a[2];
		float len2 = abx * abx + abz * abz;
		if (len2 < 1)
			return RZ_Util.DistXZ(p, a);

		float t = ((p[0] - a[0]) * abx + (p[2] - a[2]) * abz) / len2;
		t = Math.Clamp(t, 0, 1);
		vector onLine = Vector(a[0] + abx * t, 0, a[2] + abz * t);
		return RZ_Util.DistXZ(p, onLine);
	}

	// ================================================================ Dealing
	//------------------------------------------------------------------------------------------------
	protected bool TryDeal(int playerId, string ident, RZ_StrangerHere here)
	{
		string spot = string.Format("%1", CellKey(here.m_vAt));
		float cool;
		if (m_mSpotCool.Find(spot, cool) && m_fClock < cool)
			return false;

		if (m_mSpotCool.Find(ident + "|" + spot, cool) && m_fClock < cool)
			return false;

		RZ_ErrandSettings settings = RZ_ErrandSettings.Get();
		array<ref RZ_StrangerVisit> made = {};
		array<float> scores = {};
		foreach (RZ_StrangerKind k : m_aKinds)
		{
			int fits = k.m_iSpots & here.m_iKinds;
			if (fits == 0 || (k.m_bDark && !settings.m_bDarkCards))
				continue;

			RZ_StrangerVisit v = new RZ_StrangerVisit();
			v.m_iAsker = playerId;
			v.m_Kind = k;
			v.m_vPos = here.m_vAt;
			v.m_sSpot = spot;
			v.m_iSpot = BestSpot(fits);
			if (!k.Why(this, v, here))
				continue;

			made.Insert(v);
			scores.Insert(k.m_fWeight * PlaceFactor(v.m_iSpot));
		}

		if (made.IsEmpty())
			return false;

		float total = 0;
		foreach (float sc : scores)
		{
			total += sc;
		}

		float roll = Math.RandomFloat(0, total);
		RZ_StrangerVisit pick = made[0];
		foreach (int i, RZ_StrangerVisit c : made)
		{
			roll -= scores[i];
			if (roll <= 0)
			{
				pick = c;
				break;
			}
		}

		return Place(pick, ident);
	}

	//------------------------------------------------------------------------------------------------
	//! The spot kind a stranger is dealt at, when several fit: the freshest reason first.
	protected static int BestSpot(int fits)
	{
		array<int> order = {RZ_StrangerSpot.FRESH, RZ_StrangerSpot.LINE, RZ_StrangerSpot.CAR, RZ_StrangerSpot.FIRE, RZ_StrangerSpot.EDGE,
			RZ_StrangerSpot.CROSSROADS, RZ_StrangerSpot.APPROACH, RZ_StrangerSpot.OWN, RZ_StrangerSpot.ROUTE, RZ_StrangerSpot.ROAD};
		foreach (int k : order)
		{
			if ((fits & k) != 0)
				return k;
		}

		return RZ_StrangerSpot.ROAD;
	}

	//------------------------------------------------------------------------------------------------
	//! x1.5 at a fresh event or a body, x1.3 at a crossroads, an approach or a town edge at dusk.
	protected static float PlaceFactor(int spot)
	{
		if (spot == RZ_StrangerSpot.FRESH)
			return 1.5;

		if (spot == RZ_StrangerSpot.CROSSROADS || spot == RZ_StrangerSpot.APPROACH || spot == RZ_StrangerSpot.EDGE)
			return 1.3;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	protected bool Place(RZ_StrangerVisit v, string ident)
	{
		RZ_Shared shared = m_GM.GetShared();
		ResourceName prefab = m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);
		if (prefab.IsEmpty() || !shared || !shared.CanSpawnBody(RZ_BodyBudget.STRANGER, v.m_vPos))
			return false;

		// Out of sight: 60-90 m from the player where nobody can see the spot (a building, a hedge, a fold
		// of ground), else the spot itself
		v.m_vPos = OutOfSight(v);
		v.m_Body = RZ_Util.SpawnPrefab(prefab, RZ_Util.GroundPos(v.m_vPos), Math.RandomFloat(0, 360));
		if (!v.m_Body)
			return false;

		shared.AddBody(RZ_BodyBudget.STRANGER, v.m_Body);
		v.m_iId = m_iNextId;
		m_iNextId++;
		RZ_PuzzleObjectComponent.Tag(v.m_Body, TAG, v.m_iId, "", v.Fill(v.m_Kind.Labels(this, v)));
		m_aVisits.Insert(v);
		RZ_ErrandSettings gapSettings = RZ_ErrandSettings.Get();
		m_mNext.Set(ident, m_fClock + 60 * Math.RandomFloat(gapSettings.m_fStrangerGapMin, Math.Max(gapSettings.m_fStrangerGapMin, gapSettings.m_fStrangerGapMax)));
		m_mSpotCool.Set(v.m_sSpot, m_fClock + SPOT_COOL);
		m_mSpotCool.Set(ident + "|" + v.m_sSpot, m_fClock + SPOT_COOL_MINE);
		string spotName = RZ_StrangerSpot.Names(v.m_iSpot);
		int n;
		m_mDealt.Find(spotName, n);
		m_mDealt.Set(spotName, n + 1);
		RZ_Log.Info(string.Format("Errands: stranger %1 for %2 at %3 (%4)", v.m_Kind.m_sKey, RZ_Util.PlayerName(v.m_iAsker), RZ_SettlementScanner.GridReference(v.m_vPos), spotName));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A point 60-90 m from the asker, toward the spot and to either side, that no player can see (the
	//! infected bodies' sight test: terrain, buildings and trees). The spot when none is. VERIFY: the
	//! out-of-sight spawn (a physics trace from the eyes is stopped by buildings and hedges).
	protected vector OutOfSight(RZ_StrangerVisit v)
	{
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(v.m_iAsker);
		if (!me)
			return v.m_vPos;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		vector from = me.GetOrigin();
		vector dir = v.m_vPos - from;
		dir[1] = 0;
		float len = dir.Length();
		if (len < 1)
			dir = Vector(1, 0, 0);
		else
			dir = dir * (1 / len);

		vector side = Vector(-dir[2], 0, dir[0]);
		array<float> offsets = {0, 0.6, -0.6, 1.2, -1.2};
		foreach (float o : offsets)
		{
			vector way = dir + side * o;
			float wl = way.Length();
			if (wl > 0.01)
				way = way * (1 / wl);

			vector at = RZ_Util.GroundPos(from + way * Math.RandomFloat(60, 90));
			if (RZ_Util.IsWater(at))
				continue;

			if (!RZ_InfectedBodies.InSight(at, players, true))
				return at;
		}

		return v.m_vPos;
	}

	// ================================================================ Visits
	//------------------------------------------------------------------------------------------------
	//! They call out once when you're close, wait 5 minutes or until you're 150 m away (a way under way
	//! waits 10 minutes wherever you are), then go. A killed one costs its killer.
	protected void KeepVisits(array<int> ids, array<IEntity> players, float dt)
	{
		for (int i = m_aVisits.Count() - 1; i >= 0; i--)
		{
			RZ_StrangerVisit v = m_aVisits[i];
			v.m_fAge += dt;
			if (v.m_Body && !v.m_bDone && !RZ_Util.IsCharacterAlive(v.m_Body))
				Killed(v);

			IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(v.m_iAsker);
			float dist = 100000;
			if (me)
				dist = RZ_Util.DistXZ(me.GetOrigin(), v.m_vPos);

			if (!v.m_bCalled && dist <= CALL_RANGE)
			{
				v.m_bCalled = true;
				v.m_fAge = 0;
				Speak(v, v.m_iAsker, v.Fill(v.m_Kind.CallOut(this, v)));
			}

			if (!v.m_bDone && !v.m_Kind.Still(this, v))
			{
				m_GM.NotifyPlayer(v.m_iAsker, "A stranger", "They've gone. Whatever it was, it sorted itself out without you.");
				v.m_bDone = true;
			}

			bool stepping = v.m_iStep >= 0 && m_fClock < v.m_fStepUntil;
			bool gone = v.m_bDone || !v.m_Body || (!stepping && v.m_bCalled && (dist > LEAVE || v.m_fAge > WAIT)) || (!v.m_bCalled && v.m_fAge > 900);
			if (v.m_iStep >= 0 && !stepping && !v.m_bDone)
			{
				m_GM.NotifyPlayer(v.m_iAsker, "A stranger", "You took too long. When you look back, they've given up waiting.");
				gone = true;
			}

			// Robbed: a dead stranger's pockets emptied by someone standing over them (2 minutes)
			if (v.m_fDeadAt >= 0 && v.m_Body)
			{
				Robbery(v, ids, players);
				if (m_fClock - v.m_fDeadAt < ROB_WINDOW)
					continue;
			}

			if (!gone)
				continue;

			// Removed out of sight: a living one who's done waits until nobody can see them (a minute at most)
			if (v.m_Body && v.m_fDeadAt < 0 && v.m_fUnseenWait < 60 && RZ_InfectedBodies.InSight(v.m_Body.GetOrigin(), players, true))
			{
				v.m_fUnseenWait += dt;
				continue;
			}

			RZ_Util.DeleteUnit(v.m_Body);
			if (RZ_WorkSites.Get())
				RZ_WorkSites.Get().Close(WorkKey(v));

			m_aVisits.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A stranger's body died: the killer pays what killing a wanderer costs (a red hand on the wrist:
	//! the Red Hand's people), and no stranger finds them for a real day.
	protected void Killed(RZ_StrangerVisit v)
	{
		v.m_bDone = true;
		v.m_fDeadAt = m_fClock;
		v.m_iPockets = CountPockets(v.m_Body);
		int killer = RZ_Util.GetKillerPlayerId(v.m_Body);
		if (killer <= 0)
			return;

		int f = v.m_Kind.m_iFaction;
		if (v.m_bTwist && v.m_Kind.m_sKey == "sister")
			f = RZ_EFaction.BANDITS;

		m_GM.AddInfamy(killer, f, 15, "killed a stranger", 0, "killed someone on the road");
		m_mNext.Set(m_GM.GetProfile(killer).m_sIdentity, m_fClock + KILLED_PAUSE);
		RZ_Log.Info(string.Format("Errands: stranger %1 killed by %2", v.m_Kind.m_sKey, RZ_Util.PlayerName(killer)));
	}

	//------------------------------------------------------------------------------------------------
	//! What a body carries. VERIFY: telling that a body's inventory was taken (the item count falls).
	protected static int CountPockets(IEntity body)
	{
		if (!body)
			return 0;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(body.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return 0;

		array<IEntity> items = {};
		inv.GetItems(items);
		return items.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! The pockets went down with a player within 3 m: they robbed the dead. +8 Haven Infamy, once, and
	//! no stranger for them for a real day.
	protected void Robbery(RZ_StrangerVisit v, array<int> ids, array<IEntity> players)
	{
		if (v.m_bRobbed)
			return;

		int now = CountPockets(v.m_Body);
		if (now >= v.m_iPockets)
			return;

		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), v.m_Body.GetOrigin()) > 3)
				continue;

			v.m_bRobbed = true;
			m_GM.AddInfamy(ids[i], RZ_EFaction.SURVIVORS, ROB_INFAMY, "robbed a dead stranger");
			m_mNext.Set(m_GM.GetProfile(ids[i]).m_sIdentity, m_fClock + KILLED_PAUSE);
			m_GM.NotifyPlayer(ids[i], "The Haven", "Someone will have seen you go through the pockets. Someone always does.");
			RZ_Log.Info(string.Format("Errands: stranger %1 robbed by %2", v.m_Kind.m_sKey, RZ_Util.PlayerName(ids[i])));
			return;
		}

		v.m_iPockets = now;
	}

	//------------------------------------------------------------------------------------------------
	//! A player fired within 25 m of a living stranger with no infected within 40 m of them (not a
	//! fight): shot at. +4 Infamy with their people, once a stranger, and they run (the visit ends).
	void OnShot(int playerId, vector pos)
	{
		foreach (RZ_StrangerVisit v : m_aVisits)
		{
			if (v.m_bDone || !v.m_Body || RZ_Util.CountAlive(v.m_Body) <= 0)
				continue;

			vector at = v.m_Body.GetOrigin();
			if (RZ_Util.DistXZ(at, pos) > 25 || (m_GM.GetSpawner() && m_GM.GetSpawner().CountZombiesNear(at, 40, 1) > 0))
				continue;

			int f = v.m_Kind.m_iFaction;
			if (v.m_bTwist && v.m_Kind.m_sKey == "sister")
				f = RZ_EFaction.BANDITS;

			v.m_bDone = true;
			m_GM.AddInfamy(playerId, f, 4, "shot at a stranger", 0, "shot at someone on the road");
			RZ_Log.Info(string.Format("Errands: stranger %1 shot at by %2", v.m_Kind.m_sKey, RZ_Util.PlayerName(playerId)));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every 5 s (the errands' step tick): a way under way at its place, for long enough (unseen if
	//! it must be).
	void TickSteps(float dt, array<int> ids, array<IEntity> players)
	{
		foreach (RZ_StrangerVisit v : m_aVisits)
		{
			if (v.m_iStep < 0 || v.m_bDone)
				continue;

			int idx = ids.Find(v.m_iAsker);
			if (idx < 0)
				continue;

			// Work with your hands: done when its hold is done (RZ_WorkSites)
			RZ_WorkSites sites = RZ_WorkSites.Get();
			string key = WorkKey(v);
			if (sites && sites.Has(key))
			{
				if (!sites.IsDone(key))
					continue;

				sites.Close(key);
			}
			else
			{
				if (RZ_Util.DistXZ(players[idx].GetOrigin(), v.m_vStep) > v.m_fStepRadius)
					continue;

				if (v.m_bStepUnseen && !RZ_Stealth.IsHidden(v.m_iAsker))
					continue;

				v.m_fStepDone += dt;
				if (v.m_fStepDone < v.m_fStepNeed)
					continue;
			}

			int step = v.m_iStep;
			v.m_iStep = -1;
			v.m_Kind.StepDone(this, v, step, v.m_iAsker, players[idx]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A way that asks you to go somewhere (or stay here) a while: done in StepDone.
	void StartStep(RZ_StrangerVisit v, int option, vector at, float seconds, float radius, bool unseen, string line)
	{
		v.m_iStep = option;
		v.m_vStep = at;
		v.m_fStepNeed = seconds;
		v.m_fStepDone = 0;
		v.m_fStepRadius = radius;
		v.m_bStepUnseen = unseen;
		v.m_fStepUntil = m_fClock + STEP_LIFE;
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(v.m_iAsker);
		string where = "";
		if (me && RZ_Util.DistXZ(me.GetOrigin(), at) > 30)
			where = " " + RZ_Util.Capitalize(RZ_Util.DirectionText(me.GetOrigin(), at)) + ".";

		Speak(v, v.m_iAsker, v.Fill(line) + where);
	}

	//------------------------------------------------------------------------------------------------
	//! A way done with your hands (docs/DESIGN.md, Work site): a work site at `at` for the asker,
	//! held `hold` s; with work sites off, the old rule (stay there `seconds`). The line's time
	//! there goes when it's held.
	void StartWork(RZ_StrangerVisit v, int option, vector at, float seconds, float radius, bool unseen, string line, RZ_EWork kind, string label, float hold)
	{
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (!RZ_WorkSites.Available() || !sites)
		{
			StartStep(v, option, at, seconds, radius, unseen, line);
			return;
		}

		string key = WorkKey(v);
		sites.Close(key);
		if (sites.Open(key, kind, label, hold, at, v.m_iAsker) < 0)
		{
			StartStep(v, option, at, seconds, radius, unseen, line);
			return;
		}

		if (unseen)
			sites.SetUnseen(key);

		StartStep(v, option, at, seconds, radius, unseen, RZ_WorkSites.WithoutTime(line));
	}

	//------------------------------------------------------------------------------------------------
	static string WorkKey(RZ_StrangerVisit v)
	{
		return "stranger." + v.m_iId.ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! A choice slot on a stranger (RZ_PuzzleActionHelper.RouteChoice). Only the one they called to,
	//! or their group, may answer.
	void HandleChoice(int playerId, int id, int option, IEntity owner, IEntity user)
	{
		RZ_StrangerVisit v;
		foreach (RZ_StrangerVisit s : m_aVisits)
		{
			if (s.m_iId == id)
				v = s;
		}

		if (!v || v.m_bDone)
			return;

		if (playerId != v.m_iAsker && !RZ_Util.SameGroup(playerId, v.m_iAsker))
		{
			m_GM.NotifyPlayer(playerId, "A stranger", "They look past you, at someone else. Whatever it is, it isn't for you.");
			return;
		}

		if (v.m_iStep >= 0)
		{
			Speak(v, playerId, "Go on, then. I'll be here.");
			return;
		}

		v.m_iAsker = playerId;
		v.m_Kind.Choose(this, v, playerId, option, user);
	}

	// ================================================================ Helpers for the routines
	//------------------------------------------------------------------------------------------------
	//! The stranger is finished: they go once you're out of sight.
	void Finish(RZ_StrangerVisit v, int playerId, string line, bool helped)
	{
		v.m_bDone = true;
		if (!line.IsEmpty())
			Speak(v, playerId, v.Fill(line));

		if (!helped)
			return;

		int n;
		m_mHelped.Find(v.m_Kind.m_sKey, n);
		m_mHelped.Set(v.m_Kind.m_sKey, n + 1);
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		RZ_ErrandLedger ledger = m_Errands.Ledger(profile.m_sIdentity);
		ledger.helped++;
		if (m_GM.GetHall() && !(m_GM.GetCure() && m_GM.GetCure().IsCured()))
			m_GM.GetHall().AddHelped(playerId);

		RZ_Log.Info(string.Format("Errands: stranger %1 done by %2", v.m_Kind.m_sKey, RZ_Util.PlayerName(playerId)));
	}

	//------------------------------------------------------------------------------------------------
	//! A line to the player, the stranger staying: said by them (docs/MISSION_RULES.md, rule 29).
	void Say(RZ_StrangerVisit v, int playerId, string line)
	{
		Speak(v, playerId, v.Fill(line));
	}

	//------------------------------------------------------------------------------------------------
	//! The stranger says it, from his body when it's out and the player is within earshot (a voice,
	//! with its arrow); a card only when he's gone or out of earshot.
	protected void Speak(RZ_StrangerVisit v, int playerId, string text)
	{
		if (text.IsEmpty())
			return;

		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (v.m_Body && me && v.m_fDeadAt < 0 && RZ_Util.DistXZ(me.GetOrigin(), v.m_Body.GetOrigin()) <= EARSHOT)
		{
			m_GM.Say(playerId, "A stranger", -1, text, v.m_Body);
			return;
		}

		m_GM.NotifyPlayer(playerId, "A stranger", text);
	}

	//------------------------------------------------------------------------------------------------
	//! The house pays its usual few Favor when they reach it (the errands' hourly ledger).
	int HouseFavor(int playerId, int faction, int amount)
	{
		return m_Errands.SmallFavor(playerId, faction, amount);
	}

	//------------------------------------------------------------------------------------------------
	//! One common find from their own pocket (a House find).
	bool Gift(int playerId, IEntity user)
	{
		RZ_Interactions hub = RZ_Interactions.Get();
		if (!hub || !user)
			return false;

		array<ResourceName> picks = {};
		hub.GetSearch().PickHouseLoot("house", user.GetOrigin(), false, 1, true, picks);
		if (picks.IsEmpty())
			return false;

		return RZ_ItemHelper.Give(picks[0], user, user.GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	//! +1 person at a house, if it has room for them (they're one of the house now).
	bool ToHouse(int house)
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses || !houses.Get(house))
			return false;

		return houses.AddPopulationAt(house, 1) > 0;
	}

	//------------------------------------------------------------------------------------------------
	string HouseName(int house)
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses && houses.Get(house))
			return houses.Get(house).m_sName;

		return "the nearest house";
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest lived-in safehouse within `radius`, -1 if none (not exiling this player: a
	//! Haven-anchored stranger refuses an exile).
	int NearestHouse(vector pos, float radius)
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses)
			return -1;

		return RZ_ErrandCard.HouseNear(m_GM, pos, radius, -1, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! A point `dist` metres from `from` toward `to`, on dry land.
	static vector Toward(vector from, vector to, float dist)
	{
		vector d = to - from;
		d[1] = 0;
		float len = d.Length();
		if (len < 1)
			return from;

		return RZ_Mission.LandPointNear(from + d * (Math.Min(dist, len) / len), 0, 15);
	}

	//------------------------------------------------------------------------------------------------
	int LiveCount()
	{
		return m_aVisits.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! The spot kinds plausible at `at` for this player now (the admin view).
	string SpotsAt(vector at, int playerId)
	{
		return RZ_StrangerSpot.Names(Find(at, playerId).m_iKinds);
	}

	//------------------------------------------------------------------------------------------------
	//! Telemetry: strangers dealt by spot kind, and helped by routine.
	string Telemetry()
	{
		string text = "strangers dealt by spot:";
		foreach (string spot, int n : m_mDealt)
		{
			text += string.Format(" %1 %2", spot, n);
		}

		text += ", helped:";
		foreach (string key, int h : m_mHelped)
		{
			text += string.Format(" %1 %2", key, h);
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	void Cleanup()
	{
		foreach (RZ_StrangerVisit v : m_aVisits)
		{
			RZ_Util.DeleteUnit(v.m_Body);
		}

		m_aVisits.Clear();
	}
}
