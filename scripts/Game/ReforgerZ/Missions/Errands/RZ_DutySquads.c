//! Duty squads (docs/ERRANDS.md, "With their people"): the place's own people who go out with a
//! player on a duty. A patrol walks a round from the gate and back (the player walks it with them:
//! they wait while nobody's within the escort leash); a hunter follows the player out to the ground
//! and home. Each is one escort block (Missions/Blocks/RZ_Escort.c): data until a player is near,
//! a body only then, inside Contracts > Max escorts. What a patrol finds at each point is one line
//! from the world as it is there. Owned by the errand dealer. Server only.

//------------------------------------------------------------------------------------------------
class RZ_DutySquad
{
	static const int MODE_ROUND = 0;	// walks its points and comes home
	static const int MODE_FOLLOW = 1;	// follows the player

	int m_iErrand;
	int m_iPlayer;
	int m_iMode;
	string m_sTitle;					// the duty's title, for its lines
	vector m_vHome;
	ref RZ_Escort m_Escort;
	ref array<vector> m_aPoints = {};	// a round's points (home isn't one)
	int m_iReached;						// points whose find has been said
	bool m_bLostSaid;
}

//------------------------------------------------------------------------------------------------
class RZ_DutySquads
{
	protected static const float POINT_RANGE = 25;		// the squad is at a point
	protected static const float WITH_RANGE = 40;		// the player is with them

	protected RZ_GameModeComponent m_GM;
	protected RZ_Errands m_Dealer;
	protected ref array<ref RZ_DutySquad> m_aSquads = {};

	//------------------------------------------------------------------------------------------------
	void RZ_DutySquads(RZ_GameModeComponent gm, RZ_Errands dealer)
	{
		m_GM = gm;
		m_Dealer = dealer;
	}

	//------------------------------------------------------------------------------------------------
	RZ_DutySquad Find(int errandId)
	{
		foreach (RZ_DutySquad s : m_aSquads)
		{
			if (s.m_iErrand == errandId)
				return s;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Who a faction sends out: its holdings' guard (Factions > Territory), else the survivor group.
	//! Empty: they go as data with a civilian figure (the escort block's rule).
	static ResourceName PrefabFor(RZ_GameModeComponent gm, int faction)
	{
		ResourceName prefab;
		if (gm.GetTerritory() && RZ_Factions.IsValid(faction))
			prefab = gm.GetTerritory().GuardPrefab(faction);

		if (prefab.IsEmpty() && faction == RZ_EFaction.MILITARY && gm.GetContractSettings())
			prefab = gm.GetContractSettings().m_sBastionSquadPrefab;

		if (prefab.IsEmpty() && faction == RZ_EFaction.BANDITS)
			prefab = gm.GetRedHandGroupPrefab();

		if (prefab.IsEmpty())
			prefab = gm.GetSurvivorGroupPrefab();

		return prefab;
	}

	//------------------------------------------------------------------------------------------------
	//! A round's points: `count` places `minR`-`maxR` out from `home`, in order round the compass, so
	//! the walk is a loop. Fewer (or none) where the land has no such places.
	static void Round(vector home, float minR, float maxR, int count, notnull array<vector> points)
	{
		RZ_PlaceNeed need = RZ_PlaceNeeds.Get("errand.out");
		array<float> bearings = {};
		for (int tries = 0; tries < count * 4 && points.Count() < count; tries++)
		{
			vector p = RZ_PlaceFind.Near(home, minR, maxR, need);
			if (p == vector.Zero)
				continue;

			// Not two points on top of each other
			bool close = false;
			foreach (vector have : points)
			{
				if (RZ_Util.DistXZ(have, p) < minR * 0.6)
					close = true;
			}

			if (close)
				continue;

			float bearing = Math.Atan2(p[0] - home[0], p[2] - home[2]);
			int at = 0;
			while (at < bearings.Count() && bearings[at] < bearing)
			{
				at++;
			}

			bearings.InsertAt(bearing, at);
			points.InsertAt(p, at);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Start a squad for this errand at `home`. A round walks `points` and comes back; with none, it
	//! follows the player. False: no room for another escort (the duty is done alone).
	bool Start(RZ_Errand e, string title, string name, vector home, array<vector> points)
	{
		if (Find(e.m_iId))
			return true;

		RZ_Escort escort = new RZ_Escort();
		escort.m_sName = name;
		escort.m_sPrefab = PrefabFor(m_GM, e.m_iFaction);
		escort.m_fArriveRadius = POINT_RANGE;
		escort.m_bHide = false;		// they're armed, and out there to look
		if (!escort.Begin(m_GM, home))
			return false;

		RZ_DutySquad s = new RZ_DutySquad();
		s.m_iErrand = e.m_iId;
		s.m_iPlayer = e.m_iAsker;
		s.m_sTitle = title;
		s.m_vHome = home;
		s.m_Escort = escort;
		if (points && !points.IsEmpty())
		{
			s.m_iMode = RZ_DutySquad.MODE_ROUND;
			array<vector> route = {};
			foreach (vector p : points)
			{
				s.m_aPoints.Insert(p);
				route.Insert(p);
			}

			route.Insert(home);
			escort.SetRoute(route);
		}
		else
		{
			s.m_iMode = RZ_DutySquad.MODE_FOLLOW;
			escort.Follow(e.m_iAsker);
		}

		escort.Go();
		m_aSquads.Insert(s);
		RZ_Log.Info(string.Format("Duty squad: %1 out with %2 from %3, %4 points (errand %5).", name, RZ_Util.PlayerName(e.m_iAsker), RZ_SettlementScanner.GridReference(home), s.m_aPoints.Count(), e.m_iId));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The errand is over: its people go home (the escort's slot is freed).
	void Drop(int errandId)
	{
		for (int i = m_aSquads.Count() - 1; i >= 0; i--)
		{
			if (m_aSquads[i].m_iErrand != errandId)
				continue;

			m_aSquads[i].m_Escort.Cleanup();
			m_aSquads.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	bool IsLost(int errandId)
	{
		RZ_DutySquad s = Find(errandId);
		return s && s.m_Escort.IsDead();
	}

	//------------------------------------------------------------------------------------------------
	//! A round is walked and its people are home, with the player beside them.
	bool IsHome(int errandId, IEntity me)
	{
		RZ_DutySquad s = Find(errandId);
		if (!s || !me)
			return false;

		return s.m_Escort.IsArrived() && RZ_Util.DistXZ(me.GetOrigin(), s.m_Escort.m_vPos) <= WITH_RANGE;
	}

	//------------------------------------------------------------------------------------------------
	//! Their people are with the player now (a follower within reach).
	bool IsWith(int errandId, IEntity me)
	{
		RZ_DutySquad s = Find(errandId);
		if (!s || !me || s.m_Escort.IsDead())
			return false;

		return RZ_Util.DistXZ(me.GetOrigin(), s.m_Escort.m_vPos) <= WITH_RANGE;
	}

	//------------------------------------------------------------------------------------------------
	//! Where the squad is, for a "go back to them" line.
	string Where(int errandId, vector from)
	{
		RZ_DutySquad s = Find(errandId);
		if (!s)
			return "";

		return s.m_Escort.Where(from);
	}

	//------------------------------------------------------------------------------------------------
	//! On the errands' tick: the escorts move, and a round says what it finds at each point.
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		foreach (RZ_DutySquad s : m_aSquads)
		{
			s.m_Escort.Update(m_GM, dt, ids, players);
			if (s.m_Escort.IsDead())
			{
				if (!s.m_bLostSaid)
				{
					s.m_bLostSaid = true;
					m_GM.NotifyPlayer(s.m_iPlayer, s.m_sTitle, string.Format("%1 didn't make it. Go back and say so.", RZ_Util.Capitalize(s.m_Escort.m_sName)));
				}

				continue;
			}

			if (s.m_iMode != RZ_DutySquad.MODE_ROUND || s.m_iReached >= s.m_aPoints.Count())
				continue;

			vector point = s.m_aPoints[s.m_iReached];
			if (!s.m_Escort.IsNear(point, POINT_RANGE + 5))
				continue;

			s.m_iReached++;
			string left = "Then home.";
			if (s.m_iReached < s.m_aPoints.Count())
				left = "On to the next.";

			m_GM.NotifyPlayer(s.m_iPlayer, s.m_sTitle, string.Format("Point %1 of %2. %3 %4", s.m_iReached, s.m_aPoints.Count(), Found(point), left));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! What stands at a patrol's point, from the world as it is there: the infected near it, the
	//! nearest town and how sick it is, the weather, the hour, the outbreak's stage.
	protected string Found(vector at)
	{
		array<string> so = {};
		RZ_WorldFacts facts = m_GM.GetFacts();
		int near = m_GM.GetSpawner().CountZombiesNear(at, 80, 4);
		if (near >= 3)
			so.Insert("Several of the infected, close, in the open. Written down, and walked round.");
		else if (near > 0)
			so.Insert("One of the infected, standing. It hasn't seen anyone yet.");

		RZ_Settlement town = m_GM.GetSettlement(m_GM.FindNearestSettlement(at));
		if (town && town.m_fInfection >= 60)
			so.Insert(string.Format("The noise off %1 carries this far. Nobody says anything about it.", town.m_sName));
		else if (town && town.m_fInfection < 20)
			so.Insert(string.Format("Quiet towards %1. A door stands open that was shut yesterday.", town.m_sName));

		if (m_GM.GetEnvironment() && m_GM.GetEnvironment().IsRaining())
			so.Insert("Fresh tracks in the mud, two sets, going the other way.");

		if (facts && facts.IsNight())
			so.Insert("A light, far off, where no light should be. It goes out.");
		else if (facts && facts.IsDusk())
			so.Insert("Smoke on the skyline, thin. Someone is cooking, or was.");

		if (facts && facts.Stage() >= RZ_EOutbreakStage.OVERRUN)
			so.Insert("A cart on its side, picked clean, and a shoe.");

		if (so.IsEmpty())
		{
			so.Insert("Nothing moving. The marker is where it was left.");
			so.Insert("A fence post down. It gets stood up again.");
			so.Insert("Crows on something in the field. Not worth the walk.");
		}

		return so[Math.RandomInt(0, so.Count())];
	}
}
