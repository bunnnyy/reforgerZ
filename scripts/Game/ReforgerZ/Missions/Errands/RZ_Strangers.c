//! Strangers (docs/ERRANDS.md, "Strangers"): a passer-by who asks one small thing, met only where
//! one would make sense (the author's decision 14), never because a timer ran out. Spots are data
//! from facts the server already has, looked for 150-350 m ahead along the way a player is walking:
//! - a **road people use**: a 250 m footfall cell players walked a lot lately (by day);
//! - an **approach**: 100-800 m out from a safehouse or a holding, outside its wire;
//! - a **town edge at dusk** (fog counts as dusk): within 100 m of a settlement's edge;
//! - a **fresh event**: within 400 m of a bite, death, fire or shelling in the last 2 played hours.
//! A stranger routine also needs its Why to be true. Never: another player outside your group within
//! 300 m, within 2 km of the Long Table's farm, driving, a shot within 60 m in 60 s, infected within
//! 30 m, a running mission's 300 m, a storm, the first 5 minutes of a life, a spot that gave anyone a
//! stranger in 2 real hours or you one in a real day. The ceiling only stops one: one per player every
//! 15-25 minutes of play, 6 on the server (the body budget). They call out once, wait 5 minutes or
//! until you're 150 m away, and pay nothing but a line or the house's few Favor. Server only.
class RZ_StrangerVisit
{
	int m_iId;
	int m_iAsker;
	string m_sKind;			// "roof", "tin"
	vector m_vPos;
	string m_sSpot;			// its cooldown key
	int m_iHouse = -1;
	string m_sTip;
	IEntity m_Body;
	float m_fAge;
	bool m_bCalled;
	bool m_bDone;
}

//------------------------------------------------------------------------------------------------
class RZ_Strangers
{
	static const int TAG = -14;		// RZ_PuzzleObjectComponent mission id: a stranger

	protected static const float CELL = 250;
	protected static const float HALF_LIFE = 21600;		// the footfall halves every 6 real hours
	protected static const float ROAD_FOOTFALL = 20;	// player-ticks in a cell for a road people use
	protected static const float WAIT = 300;
	protected static const float LEAVE = 150;
	protected static const float CALL_RANGE = 40;
	protected static const int MAX_LIVE = 6;
	protected static const float SPOT_COOL = 7200;		// a spot that gave anyone a stranger
	protected static const float SPOT_COOL_MINE = 86400;	// a spot that gave you one

	protected RZ_GameModeComponent m_GM;
	protected RZ_Errands m_Errands;
	protected ref map<int, float> m_mFootfall = new map<int, float>();
	protected ref map<int, vector> m_mLastPos = new map<int, vector>();
	protected ref map<string, float> m_mNext = new map<string, float>();		// ident -> clock of the next allowed
	protected ref map<string, float> m_mSpotCool = new map<string, float>();	// spot -> clock
	protected ref array<ref RZ_StrangerVisit> m_aVisits = {};
	protected int m_iNextId = 1;
	protected float m_fClock;

	//------------------------------------------------------------------------------------------------
	void RZ_Strangers(RZ_GameModeComponent gm, RZ_Errands errands)
	{
		m_GM = gm;
		m_Errands = errands;
	}

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
		float keep = Math.Pow(0.5, dt / HALF_LIFE);
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

	// ================================================================ The tick (the errands' 30 s)
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players, bool dealing)
	{
		m_fClock += dt;
		Walk(dt, players);
		KeepVisits(ids, players, dt);
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
	//! A stranger ahead of this player, if a spot and a Why fit and nothing says no.
	protected void LookAhead(int playerId, IEntity me, array<int> ids, array<IEntity> players)
	{
		vector last;
		if (!m_mLastPos.Find(playerId, last))
			return;

		vector pos = me.GetOrigin();
		vector move = pos - last;
		move[1] = 0;
		float moved = move.Length();
		if (moved < 20 || m_aVisits.Count() >= MAX_LIVE)
			return;		// standing still meets nobody

		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		float next;
		if (m_mNext.Find(ident, next) && m_fClock < next)
			return;

		if (!Allowed(playerId, me, ids, players))
			return;

		vector dir = move / moved;
		array<float> ahead = {150, 250, 350};
		foreach (float d : ahead)
		{
			vector at = RZ_Util.GroundPos(pos + dir * d);
			if (TryDeal(playerId, ident, at))
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

		if (m_GM.GetMissions())
		{
			foreach (RZ_Mission m : m_GM.GetMissions().GetActive())
			{
				if (!m.m_bGlobal && RZ_Util.DistXZ(m.m_vPos, pos) <= 300)
					return false;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool TryDeal(int playerId, string ident, vector at)
	{
		// The spot kinds plausible here now
		bool fresh = FreshNear(at, playerId);
		bool edge = TownEdge(at);
		bool approach = Approach(at);
		bool road = Footfall(at) >= ROAD_FOOTFALL && !RZ_GameTime.IsNight();
		if (!fresh && !edge && !approach && !road)
			return false;

		string spot = string.Format("%1", CellKey(at));
		float cool;
		if (m_mSpotCool.Find(spot, cool) && m_fClock < cool)
			return false;

		if (m_mSpotCool.Find(ident + "|" + spot, cool) && m_fClock < cool)
			return false;

		RZ_StrangerVisit v = new RZ_StrangerVisit();
		v.m_iAsker = playerId;
		v.m_vPos = at;
		v.m_sSpot = spot;
		if ((fresh || edge) && RoofWhy(v))
			v.m_sKind = "roof";
		else if ((road || approach) && TinWhy(v))
			v.m_sKind = "tin";
		else
			return false;

		return Place(v, ident);
	}

	//------------------------------------------------------------------------------------------------
	protected bool FreshNear(vector at, int playerId)
	{
		array<string> kinds = {"bite", "death", "fire", "shell"};
		foreach (string kind : kinds)
		{
			if (RZ_ErrandCard.FreshSignal(kind, at, 400, 2, -1))
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

	// ================================================================ The routines
	//------------------------------------------------------------------------------------------------
	//! Up on the Roof (replaces Rooftop Rescue): a town the infected took, or a fresh shelling or fire,
	//! within 800 m; someone up on a roof since.
	protected bool RoofWhy(RZ_StrangerVisit v)
	{
		array<RZ_Reason> reasons = {};
		RZ_WorldFacts.Get().Memory().ReasonsNear(v.m_vPos, 800, reasons, "town.infested");
		bool hit = !reasons.IsEmpty() || RZ_ErrandCard.FreshSignal("shell", v.m_vPos, 800, 2, -1) || RZ_ErrandCard.FreshSignal("fire", v.m_vPos, 800, 2, -1);
		if (!hit || !m_GM.GetSafehouses() || m_GM.GetSafehouses().Count() == 0)
			return false;

		v.m_iHouse = m_GM.GetSafehouses().FindNearest(v.m_vPos);
		return v.m_iHouse >= 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The Last Tin: you carry 2+ food, and the house within 1.5 km is short.
	protected bool TinWhy(RZ_StrangerVisit v)
	{
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(v.m_iAsker);
		if (!me || RZ_HavenQuest.CountFood(m_GM, me) < 2)
			return false;

		array<RZ_Reason> reasons = {};
		RZ_WorldFacts.Get().Memory().ReasonsNear(v.m_vPos, 1500, reasons, "house.hungry");
		if (reasons.IsEmpty() || !m_GM.GetSafehouses())
			return false;

		v.m_iHouse = m_GM.GetSafehouses().FindNearest(v.m_vPos, 1500);
		if (v.m_iHouse < 0)
			return false;

		// His tip: where the infected are thick, from the world's own reasons
		array<RZ_Reason> towns = {};
		RZ_WorldFacts.Get().Memory().ReasonsNear(v.m_vPos, 3000, towns, "town.infested");
		if (towns.IsEmpty())
			v.m_sTip = "\"Keep off the low road after dark. That's all I know, and it's true.\"";
		else
			v.m_sTip = string.Format("\"%1 Go round it, not through.\"", towns[0].text);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool Place(RZ_StrangerVisit v, string ident)
	{
		RZ_Shared shared = m_GM.GetShared();
		ResourceName prefab = m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);
		if (prefab.IsEmpty() || !shared || !shared.CanSpawnBody(RZ_BodyBudget.STRANGER, v.m_vPos))
			return false;

		// VERIFY: the stranger stands at the spot 150-350 m ahead; the design's out-of-sight point 60-90 m
		// from the player (a door or a hedge line) needs a line-of-sight test
		v.m_Body = RZ_Util.SpawnPrefab(prefab, v.m_vPos, Math.RandomFloat(0, 360));
		if (!v.m_Body)
			return false;

		shared.AddBody(RZ_BodyBudget.STRANGER, v.m_Body);
		v.m_iId = m_iNextId;
		m_iNextId++;
		string labels = "Talk them down|Leave them";
		if (v.m_sKind == "tin")
			labels = "Give him a tin|Not today";

		RZ_PuzzleObjectComponent.Tag(v.m_Body, TAG, v.m_iId, "", labels);
		m_aVisits.Insert(v);
		m_mNext.Set(ident, m_fClock + Math.RandomFloat(900, 1500));
		m_mSpotCool.Set(v.m_sSpot, m_fClock + SPOT_COOL);
		m_mSpotCool.Set(ident + "|" + v.m_sSpot, m_fClock + SPOT_COOL_MINE);
		RZ_Log.Info(string.Format("Errands: stranger %1 for %2 at %3", v.m_sKind, RZ_Util.PlayerName(v.m_iAsker), RZ_SettlementScanner.GridReference(v.m_vPos)));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! They call out once when you're close, wait 5 minutes or until you're 150 m away, then go.
	protected void KeepVisits(array<int> ids, array<IEntity> players, float dt)
	{
		for (int i = m_aVisits.Count() - 1; i >= 0; i--)
		{
			RZ_StrangerVisit v = m_aVisits[i];
			v.m_fAge += dt;
			IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(v.m_iAsker);
			float dist = 100000;
			if (me)
				dist = RZ_Util.DistXZ(me.GetOrigin(), v.m_vPos);

			if (!v.m_bCalled && dist <= CALL_RANGE)
			{
				v.m_bCalled = true;
				v.m_fAge = 0;
				m_GM.NotifyPlayer(v.m_iAsker, "A stranger", CallOut(v));
			}

			bool gone = v.m_bDone || !v.m_Body || (v.m_bCalled && (dist > LEAVE || v.m_fAge > WAIT)) || (!v.m_bCalled && v.m_fAge > 900);
			if (!gone)
				continue;

			RZ_Util.DeleteUnit(v.m_Body);
			m_aVisits.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string CallOut(RZ_StrangerVisit v)
	{
		if (v.m_sKind == "roof")
			return "\"Up here. On the roof. Are they gone? Tell me they're gone.\" Someone on the roof edge, waving one arm, holding on with the other.";

		return "\"Got a tin? Just the one. The house is on half portions and I've a girl there.\" He holds up an empty sack, as if that proves it.";
	}

	//------------------------------------------------------------------------------------------------
	//! A choice slot on a stranger (RZ_PuzzleActionHelper.RouteChoice).
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

		if (v.m_sKind == "roof")
		{
			if (option != 0)
			{
				m_GM.NotifyPlayer(playerId, "A stranger", "\"Don't go. Please.\" You go. The roof is quiet behind you.");
				v.m_bDone = true;
				return;
			}

			if (m_GM.GetSpawner() && m_GM.GetSpawner().IsZombieNear(v.m_vPos, 25))
			{
				m_GM.NotifyPlayer(playerId, "A stranger", "\"Not with them down there.\" Clear the street first.");
				return;
			}

			v.m_bDone = true;
			RZ_Safehouses houses = m_GM.GetSafehouses();
			RZ_Safehouse h = houses.Get(v.m_iHouse);
			if (h)
				houses.AddPopulationAt(v.m_iHouse, 1);

			m_GM.AddFavor(playerId, RZ_EFaction.SURVIVORS, 4);
			string house = "the nearest house";
			if (h)
				house = h.m_sName;

			m_GM.NotifyPlayer(playerId, "A stranger", string.Format("She comes down the drainpipe faster than you'd think, and goes off toward %1 without looking back. The house sends 4 Favor your way when she gets there.", house));
			RZ_Log.Info(string.Format("Errands: stranger roof done by %1", RZ_Util.PlayerName(playerId)));
			return;
		}

		// The Last Tin
		v.m_bDone = true;
		if (option != 0)
		{
			m_GM.NotifyPlayer(playerId, "A stranger", "\"I'd eat a Bastion ration. That's how bad it is.\" He goes back up the road.");
			return;
		}

		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!me || !RZ_HavenQuest.TakeFood(m_GM, me, 1))
		{
			m_GM.NotifyPlayer(playerId, "A stranger", "You reach for a tin and there isn't one. He sees. \"Never mind. Thanks for looking.\"");
			return;
		}

		m_GM.NotifyPlayer(playerId, "A stranger", "He holds the tin with both hands. Then, quietly: " + v.m_sTip);
		RZ_Log.Info(string.Format("Errands: stranger tin done by %1", RZ_Util.PlayerName(playerId)));
	}

	//------------------------------------------------------------------------------------------------
	int LiveCount()
	{
		return m_aVisits.Count();
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
