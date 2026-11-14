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
	bool m_bHeld;						// stopped at a point while the player sees to what's there
	bool m_bCull;						// a cull: at its point the strays come, and they stand until it's clear
	float m_fCullAt = -1;				// the squads' clock when they came
	int m_iFinds;						// points that held something to go and look at
	int m_iFaction = -1;
	string m_sLead;						// who leads them, by name: "Lance Corporal Petr Novak"
	string m_sLeadTitle;				// what he's said under: "Lance Corporal Petr Novak, Bastion patrol"
	vector m_vWait;						// where they walk over to wait by a find off the round
	bool m_bDetour;						// walking over to wait there
}

//------------------------------------------------------------------------------------------------
class RZ_DutySquads
{
	protected static const float POINT_RANGE = 25;		// the squad is at a point
	protected static const float WITH_RANGE = 40;		// the player is with them
	protected static const float EARSHOT = 60;			// the lead is heard this far; further, a notice
	protected static const float DETOUR_FROM = 35;		// a find further than this from them: they walk over to wait by it
	protected static const float WAIT_OFF = 15;			// and wait this far short of it

	protected RZ_GameModeComponent m_GM;
	protected RZ_Errands m_Dealer;
	protected ref array<ref RZ_DutySquad> m_aSquads = {};
	protected float m_fClock;

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
	bool Start(RZ_Errand e, string title, string name, vector home, array<vector> points, ResourceName prefab = "")
	{
		if (Find(e.m_iId))
			return true;

		RZ_Escort escort = new RZ_Escort();
		escort.m_sName = name;
		escort.m_sPrefab = prefab;
		if (prefab.IsEmpty())
			escort.m_sPrefab = PrefabFor(m_GM, e.m_iFaction);

		escort.m_fArriveRadius = POINT_RANGE;
		escort.m_bHide = false;		// they're armed, and out there to look
		if (!escort.Begin(m_GM, home))
			return false;

		RZ_DutySquad s = new RZ_DutySquad();
		s.m_iErrand = e.m_iId;
		s.m_iPlayer = e.m_iAsker;
		s.m_iFaction = e.m_iFaction;
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

		LeadFor(s, e);
		escort.Go();
		m_aSquads.Insert(s);
		RZ_Log.Info(string.Format("Duty squad: %1 out with %2 from %3, %4 points (errand %5).", name, RZ_Util.PlayerName(e.m_iAsker), RZ_SettlementScanner.GridReference(home), s.m_aPoints.Count(), e.m_iId));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A name for who leads them (always a man: docs/MISSION_RULES.md, rule 10), with a rank in
	//! Bastion, and the title his lines are said under.
	protected void LeadFor(RZ_DutySquad s, RZ_Errand e)
	{
		RZ_Rng rng = new RZ_Rng(e.m_iId * 7919 + 13);
		string name = RZ_NoteText.FullName(rng);
		string what = "patrol";
		if (s.m_iMode == RZ_DutySquad.MODE_FOLLOW)
			what = "hunter";

		if (e.m_iFaction == RZ_EFaction.MILITARY)
			name = "Lance Corporal " + name;

		s.m_sLead = name;
		s.m_sLeadTitle = name;
		if (RZ_Factions.IsValid(e.m_iFaction))
			s.m_sLeadTitle = string.Format("%1, %2 %3", name, RZ_Factions.GetShortName(e.m_iFaction), what);
	}

	//------------------------------------------------------------------------------------------------
	//! This errand's round is a cull: at its point the infected come, a handful, and the squad stands
	//! with the player until none is left near.
	void SetCull(int errandId)
	{
		RZ_DutySquad s = Find(errandId);
		if (s)
			s.m_bCull = true;
	}

	//------------------------------------------------------------------------------------------------
	//! The strays at a cull's point: up to five, 40-60 m out, as the spawner has room. How many came.
	protected int Strays(vector point)
	{
		RZ_ZombieSpawner spawner = m_GM.GetSpawner();
		int room = Math.Min(5, spawner.SpawnRoom(point));
		int town = m_GM.FindNearestSettlement(point);
		int came = 0;
		for (int i = 0; i < room; i++)
		{
			vector at = RZ_Util.GroundPos(RZ_Util.RandomPointInRing(point, 40, 60));
			if (spawner.SpawnZombie(at, false, town, -1))
				came++;
		}

		return came;
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
	//! The lead says it (docs/MISSION_RULES.md, rule 29): from his body when he's out and the player
	//! is within earshot, from where the squad walks when they're data; a notice only when he's
	//! out of earshot or gone.
	protected void SayLead(RZ_DutySquad s, string line)
	{
		if (line.IsEmpty())
			return;

		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(s.m_iPlayer);
		vector at = s.m_Escort.NowAt();
		if (me && !s.m_Escort.IsDead() && RZ_Util.DistXZ(me.GetOrigin(), at) <= EARSHOT)
		{
			IEntity body = s.m_Escort.GetPerson();
			if (body)
				m_GM.Say(s.m_iPlayer, s.m_sLeadTitle, s.m_iFaction, line, body);
			else
				m_GM.SayAt(s.m_iPlayer, s.m_sLeadTitle, s.m_iFaction, line, at);

			RZ_Log.Info(string.Format("Duty squad: %1 says to %2: %3", s.m_sLead, RZ_Util.PlayerName(s.m_iPlayer), line));
			return;
		}

		m_GM.NotifyPlayer(s.m_iPlayer, s.m_sTitle, line);
	}

	//------------------------------------------------------------------------------------------------
	//! Their name, said where they speak and in the tracker: "Lance Corporal Petr Novak".
	string LeadOf(int errandId)
	{
		RZ_DutySquad s = Find(errandId);
		if (!s)
			return "";

		return s.m_sLead;
	}

	//------------------------------------------------------------------------------------------------
	//! A find's choice is answered by the lead, when this errand has a squad. False: it has none.
	bool Answer(int errandId, string line)
	{
		RZ_DutySquad s = Find(errandId);
		if (!s)
			return false;

		SayLead(s, line);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The squad's own steps for the tracker (docs/UI.md, "The quest tracker"): walking the round,
	//! the marker rides on the lead's body; stopped at a find, it stands on the thing to look at,
	//! with walking on after it. False: no squad, or one that follows the player (its errand's own
	//! steps say where).
	bool Objectives(RZ_Errand e, notnull RZ_ObjList list, RZ_DutyScenes scenes)
	{
		RZ_DutySquad s = Find(e.m_iId);
		if (!s || s.m_iMode != RZ_DutySquad.MODE_ROUND || s.m_Escort.IsDead())
			return false;

		vector lead = s.m_Escort.NowAt();
		vector findAt;
		string goal;
		if (s.m_bHeld && s.m_fCullAt < 0 && scenes && scenes.OpenFind(e.m_iId, findAt, goal))
		{
			list.Add(e.Fill(goal), RZ_EObjState.CURRENT, RZ_ObjPlace.Spot(findAt, 3));
			list.Add(string.Format("Walk on with %1", s.m_sLead), RZ_EObjState.NEXT, RZ_ObjPlace.Spot(lead));
			return true;
		}

		if (s.m_bHeld && s.m_fCullAt >= 0)
		{
			list.Add(string.Format("Stand with %1 until it's clear", s.m_sLead), RZ_EObjState.CURRENT, RZ_ObjPlace.Spot(lead));
			return true;
		}

		if (s.m_iReached < s.m_aPoints.Count())
		{
			list.Add(string.Format("Walk the round with %1: point %2 of %3", s.m_sLead, s.m_iReached + 1, s.m_aPoints.Count()), RZ_EObjState.CURRENT, RZ_ObjPlace.Spot(lead));
			return true;
		}

		list.Add(string.Format("Come back in with %1", s.m_sLead), RZ_EObjState.CURRENT, RZ_ObjPlace.Spot(lead));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! On the errands' tick: the escorts move, and a round stops where it finds something.
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		m_fClock += dt;
		foreach (RZ_DutySquad s : m_aSquads)
		{
			s.m_Escort.Update(m_GM, dt, ids, players);
			if (s.m_Escort.IsDead())
			{
				if (!s.m_bLostSaid)
				{
					s.m_bLostSaid = true;
					// Nobody of theirs is left to say it
					m_GM.NotifyPlayer(s.m_iPlayer, s.m_sTitle, string.Format("%1 didn't make it. Go back and say so.", RZ_Util.Capitalize(s.m_Escort.m_sName)));
				}

				continue;
			}

			// Walking over to wait by a find off the round: they stop when they're there
			if (s.m_bDetour && s.m_Escort.IsNear(s.m_vWait, 8))
			{
				s.m_bDetour = false;
				s.m_Escort.Stop();
			}

			// A cull's stand: until none of them is left near, or three minutes
			if (s.m_bHeld && s.m_fCullAt >= 0)
			{
				bool clear = m_fClock - s.m_fCullAt > 15 && m_GM.GetSpawner().CountZombiesNear(s.m_Escort.m_vPos, 90, 1) == 0;
				if (!clear && m_fClock - s.m_fCullAt < 180)
					continue;

				s.m_fCullAt = -1;
				s.m_bHeld = false;
				s.m_Escort.Resume();
				SayLead(s, "Clear. Count your people, and we take them home.");
				continue;
			}

			// Stopped at a find: they wait until the player has seen to it (or let it be)
			if (s.m_bHeld)
			{
				if (!m_Dealer.GetScenes().IsDone(s.m_iErrand))
					continue;

				s.m_bHeld = false;
				s.m_bDetour = false;
				s.m_Escort.Resume();
				SayLead(s, "Right. On we go.");
			}

			if (s.m_iMode != RZ_DutySquad.MODE_ROUND || s.m_iReached >= s.m_aPoints.Count())
				continue;

			vector point = s.m_aPoints[s.m_iReached];
			if (!s.m_Escort.IsNear(point, POINT_RANGE + 5))
				continue;

			s.m_iReached++;
			if (s.m_bCull)
			{
				int came = Strays(point);
				s.m_bHeld = true;
				s.m_fCullAt = m_fClock;
				s.m_Escort.Stop();
				if (came > 0)
					SayLead(s, string.Format("Here they come. %1 of them, off the field. Stand with us until it's clear.", came));
				else
					SayLead(s, "Nothing showing itself. We stand a while, and we make sure.");

				RZ_Log.Info(string.Format("Duty squad: cull at %1, %2 strays (errand %3).", RZ_SettlementScanner.GridReference(point), came, s.m_iErrand));
				continue;
			}

			// Something off the path, for the player to go and see to: at two points in three, and
			// always at one of them; only where its thing really stands near (RZ_DutyFinds)
			RZ_Errand e = m_Dealer.FindById(s.m_iErrand);
			bool last = s.m_iReached >= s.m_aPoints.Count();
			if (e && (Math.RandomFloat01() < 0.66 || (last && s.m_iFinds == 0)))
			{
				array<ref RZ_DutyBeat> beats = {};
				string call;
				if (RZ_DutyFinds.Patrol(m_GM, e, point, s.m_sLead, beats, call))
				{
					RZ_DutyScene scene = m_Dealer.GetScenes().Ensure(e, s.m_sTitle, s.m_vHome, true);
					scene.m_vLast = point;
					foreach (RZ_DutyBeat beat : beats)
					{
						scene.m_aBeats.Insert(beat);
					}

					s.m_iFinds++;
					s.m_bHeld = true;
					s.m_Escort.Stop();
					Detour(s, beats[0].m_vAt);
					SayLead(s, call);
					continue;
				}
			}

			if (s.m_iReached < s.m_aPoints.Count())
				SayLead(s, Found(point) + " On to the next.");
			else
				SayLead(s, Found(point) + " That's the round. Home.");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A find further off than the stop: the squad walks over to wait in sight of it (the round bent
	//! to pass it), then on round from there.
	protected void Detour(RZ_DutySquad s, vector find)
	{
		if (RZ_Util.DistXZ(find, s.m_Escort.NowAt()) <= DETOUR_FROM)
			return;

		vector wait = RZ_DutyScenes.Toward(find, s.m_Escort.NowAt(), WAIT_OFF);
		array<vector> route = {wait};
		for (int i = s.m_iReached; i < s.m_aPoints.Count(); i++)
		{
			route.Insert(s.m_aPoints[i]);
		}

		route.Insert(s.m_vHome);
		s.m_Escort.SetRoute(route);
		s.m_Escort.Resume();
		s.m_vWait = wait;
		s.m_bDetour = true;
		RZ_Log.Info(string.Format("Duty squad: %1 walks over to wait by the find at %2 (errand %3).", s.m_sLead, RZ_SettlementScanner.GridReference(wait), s.m_iErrand));
	}

	//------------------------------------------------------------------------------------------------
	//! What the lead says about a point where they stop for nothing, from the world as it is there:
	//! the infected near it, the nearest town and how sick it is, the weather, the hour, the stage.
	protected string Found(vector at)
	{
		array<string> so = {};
		RZ_WorldFacts facts = m_GM.GetFacts();
		int near = m_GM.GetSpawner().CountZombiesNear(at, 80, 4);
		if (near >= 3)
			so.Insert("Several of them, close, in the open. Write it down, and we go round.");
		else if (near > 0)
			so.Insert("One of them, standing over there. Hasn't seen us. Let's keep it that way.");

		RZ_Settlement town = m_GM.GetSettlement(m_GM.FindNearestSettlement(at));
		if (town && town.m_fInfection >= 60)
			so.Insert(string.Format("Hear that? That's %1. Carries this far now.", town.m_sName));
		else if (town && town.m_fInfection < 20)
			so.Insert(string.Format("Quiet towards %1. A door's open that was shut yesterday.", town.m_sName));

		if (m_GM.GetEnvironment() && m_GM.GetEnvironment().IsRaining())
			so.Insert("Fresh tracks in the mud. Two sets, going the other way.");

		if (facts && facts.IsNight())
			so.Insert("A light, far off, where no light should be. There. It's gone.");
		else if (facts && facts.IsDusk())
			so.Insert("Smoke on the skyline. Someone's cooking, or was.");

		if (so.IsEmpty())
		{
			so.Insert("Nothing moving. Marker's where we left it.");
			so.Insert("Fence post down. Stand it up, would you.");
			so.Insert("Crows on something in the field. Not worth the walk.");
		}

		return so[Math.RandomInt(0, so.Count())];
	}
}
