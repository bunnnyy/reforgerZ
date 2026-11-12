//! The epilogue's places (docs/EPILOGUE.md, Places; Scenes): what stands at the north road (the
//! queue, the route's table, the Names board, in processing the voice at the barrier), the lab
//! gate, Rudi's crossroads, the dose outlets, the nail behind Bastion's radios, the houses, the
//! keepsakes lying out, the open questlines' people (Loose Ends), the radio station's tally and the
//! farm's basket. Each is data; a person (Puzzles > Civilian) or a table (Epilogue > Table prefab,
//! else Puzzles > Object) is spawned only near a player and removed past it, tagged so its choices
//! come here (RZ_PuzzleActionHelper, RZ_Epilogue.TAG). Nothing is saved.
//!
//! Scenes with bodies (the queue and the lab gate, inside the one body budget's epilogue share)
//! follow the safe-ground rule: within 80 m, firing with no infected within 40 m costs 2 Infamy
//! with the scene's faction and draws its guards' fire; shooting at its people costs 4 Haven
//! Infamy; killing one 15, plus 5 with the route's faction, and the queue stays one short until
//! the next step.

//------------------------------------------------------------------------------------------------
class RZ_EpiSpot
{
	int id;
	string key;
	bool person;
	vector pos;
	string title;
	string talk;
	ref array<string> labels = {};
	ref array<string> verbs = {};
	int people = 1;
	int faction = -1;
	bool scene;
	IEntity ent;
	ref array<IEntity> extras = {};
	int lastShooter = -1;
	float lastShotAt = -1000;
	bool alive = true;

	//------------------------------------------------------------------------------------------------
	void Verb(string label, string verb)
	{
		if (labels.Count() >= 4)
			return;

		labels.Insert(label);
		verbs.Insert(verb);
	}

	//------------------------------------------------------------------------------------------------
	string Joined()
	{
		string s = "";
		foreach (int i, string l : labels)
		{
			if (i > 0)
				s += "|";

			s += l;
		}

		return s;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_EpiSpots
{
	protected static const float SPAWN_RANGE = 150;
	protected static const float DESPAWN_RANGE = 220;
	protected static const float SCENE_SPAWN = 300;
	protected static const float SCENE_DESPAWN = 400;
	protected static const float SAFE_RADIUS = 80;
	protected static const float SHOT_NEAR = 40;
	protected static const int MAX_UP = 30;
	protected static const int SPAWNS_PER_TICK = 2;
	protected static const float REFRESH_SECONDS = 30;

	protected RZ_GameModeComponent m_GM;
	protected RZ_Epilogue m_Epi;
	protected ref array<ref RZ_EpiSpot> m_aSpots = {};
	protected int m_iNextId = 1;
	protected float m_fRefresh = REFRESH_SECONDS;
	protected float m_fClock;
	protected vector m_vQueue;
	protected vector m_vNames;
	protected vector m_vLabGate;
	protected bool m_bPlaces;
	protected int m_iQueueDead;
	protected int m_iDeadStep = -1;
	protected ref map<string, float> m_mShotTold = new map<string, float>();

	//------------------------------------------------------------------------------------------------
	void RZ_EpiSpots(RZ_GameModeComponent gm, RZ_Epilogue epi)
	{
		m_GM = gm;
		m_Epi = epi;
	}

	// ================================================================ Places
	//------------------------------------------------------------------------------------------------
	//! Where the fixed places stand, picked once a run: the queue 200 m up the north road from the
	//! barrier, the Names board at the bend (150 m), the lab gate 20-30 m out. With no checkpoint,
	//! the queue and the Names board stand at the Haven's stronghold house.
	protected void Places()
	{
		if (m_bPlaces)
			return;

		m_bPlaces = true;
		m_vQueue = RZ_MajorHelp.NorthRoad(m_GM, 200);
		m_vNames = RZ_MajorHelp.NorthRoad(m_GM, 150);
		if (m_vQueue == vector.Zero)
		{
			int chair = RZ_HavenQuest.ChairHouse(m_GM);
			RZ_Safehouses houses = m_GM.GetSafehouses();
			if (chair >= 0 && houses)
			{
				m_vQueue = RZ_Mission.LandPointNear(houses.Get(chair).m_vPos, 20, 30);
				m_vNames = RZ_Mission.LandPointNear(houses.Get(chair).m_vPos, 30, 40);
			}
		}
		else
		{
			m_vNames = RZ_Mission.LandPointNear(m_vNames, 8, 12);
		}

		if (m_GM.HasLab())
			m_vLabGate = RZ_Mission.LandPointNear(m_GM.GetLabPosition(), 20, 30);
	}

	vector QueuePos() { Places(); return m_vQueue; }
	vector NamesPos() { Places(); return m_vNames; }
	vector LabGatePos() { Places(); return m_vLabGate; }

	//------------------------------------------------------------------------------------------------
	vector QueueTablePos()
	{
		Places();
		if (m_vQueue == vector.Zero)
			return vector.Zero;

		return m_vQueue + Vector(5, 0, 3);
	}

	//------------------------------------------------------------------------------------------------
	vector CrossroadsPos()
	{
		return RZ_MajorHelp.Crossroads();
	}

	//------------------------------------------------------------------------------------------------
	//! The voice at the north road's barrier (processing only): inside the 60 m kill zone.
	vector BarrierPos()
	{
		return RZ_MajorHelp.NorthRoad(m_GM, 30);
	}

	//------------------------------------------------------------------------------------------------
	//! How many wait in the queue now (for its sign).
	int QueueWaiting()
	{
		int n = 3;
		if (m_Epi.Phase() >= 2)
			n = 6;

		n += m_Epi.Data().walkDown.Count();
		return Math.Max(1, n - m_iQueueDead);
	}

	//------------------------------------------------------------------------------------------------
	//! The queue's since line after one of its people was killed (until the next step).
	string QueueExtra()
	{
		string extra = m_Epi.GetClosing().QueueTableLine();
		if (m_iQueueDead > 0 && m_iDeadStep == m_Epi.Step())
			extra += " A gap in the queue, and a bag on the verge nobody's touched.";

		return extra;
	}

	// ================================================================ The list
	//------------------------------------------------------------------------------------------------
	protected RZ_EpiSpot Make(string key, bool person, vector pos, string title, string talk)
	{
		RZ_EpiSpot s = new RZ_EpiSpot();
		s.key = key;
		s.person = person;
		s.pos = pos;
		s.title = title;
		s.talk = talk;
		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! Every place that should stand now, with its choices for the phase.
	protected void Wanted(notnull array<ref RZ_EpiSpot> result)
	{
		RZ_EpilogueSave s = m_Epi.Data();
		RZ_EpiClosing closing = m_Epi.GetClosing();
		RZ_EpiOutlets outlets = m_Epi.GetOutlets();
		int phase = m_Epi.Phase();
		int route = m_Epi.Route();
		Places();
		if (s.lastDay || phase <= 0)
			return;

		// The north road
		if (m_vQueue != vector.Zero)
		{
			RZ_EpiSpot queue = Make("queue", true, m_vQueue, "The queue", RZ_EpiText.QueueTalk(m_Epi.Step()));
			queue.scene = true;
			queue.faction = RZ_EFaction.SURVIVORS;
			queue.people = Math.Min(m_Epi.GetSettings().m_iScenePeople, 3);
			if (phase >= 2)
				queue.people = m_Epi.GetSettings().m_iScenePeople;

			queue.people = Math.Max(1, queue.people - m_iQueueDead);
			queue.Verb("Bring a cup", "cup");
			if (phase == 2)
				queue.Verb("Put my name down for the walk", "down");

			if (closing.CanLeaveCar())
				queue.Verb("Leave a car for the queue", "car");

			if (closing.HasStash())
				queue.Verb("Leave what you found for the queue", "stash");

			result.Insert(queue);

			RZ_EpiSpot table = Make("table", route >= 0 && route < RZ_CureFinale.INDEP, QueueTablePos(), "The table at the queue", RZ_EpiText.QueueTable(route, s.terms));
			table.scene = true;
			table.faction = route;
			if (route < 0 || route >= RZ_CureFinale.INDEP)
				table.faction = RZ_EFaction.SURVIVORS;

			RZ_EpiOutlet atQueue = outlets.Find("queue");
			if (atQueue)
				table.Verb(atQueue.label, "dose:queue");

			if (route == RZ_CureFinale.REDHAND && phase >= 3)
				table.Verb("Buy a place near the front (15 Favor)", "front");

			result.Insert(table);

			RZ_EpiSpot names = Make("names", false, m_vNames, "The Names board", closing.WallText(true));
			names.Verb("Write your line", "line");
			result.Insert(names);

			if (phase >= 3 && m_Epi.GetSettings().m_bWalkOut && BarrierPos() != vector.Zero)
			{
				RZ_EpiSpot barrier = Make("barrier", true, BarrierPos(), "The barrier", "A speaker on a post. A hand's width of road under the arm. \"Name?\"");
				barrier.scene = true;
				barrier.faction = RZ_EFaction.SURVIVORS;
				barrier.Verb("Give your name", "walk");
				result.Insert(barrier);
			}
		}

		// The lab gate
		if (m_GM.HasLab())
		{
			string gateTalk = RZ_EpiText.Director(route, s.terms) + "\n" + m_Epi.GetBeats().PlaceLine(m_vLabGate, 30);
			RZ_EpiSpot gate = Make("labgate", true, m_vLabGate, "The lab gate", gateTalk);
			gate.scene = true;
			gate.faction = RZ_EFaction.INSTITUTE;
			RZ_EpiOutlet lab = outlets.Find("lab");
			if (lab)
				gate.Verb(lab.label, "dose:lab");

			if (closing.CarryingFor(-2, RZ_EFaction.INSTITUTE))
				gate.Verb("Hand in the notebook", "home");

			result.Insert(gate);
		}

		// Rudi's crossroads: the price board, the book
		vector cross = CrossroadsPos();
		if (cross != vector.Zero)
		{
			RZ_EpiSpot board = Make("crossroads", false, cross, "Rudi's crossroads", m_Epi.GetBeats().PlaceLine(cross, 30));
			board.faction = RZ_EFaction.BANDITS;
			RZ_EpiOutlet bo = outlets.Find("board");
			if (bo)
				board.Verb(bo.label, "dose:board");

			board.Verb("Pay into the book (15 Favor)", "book");
			board.Verb("Hand in a dead crewman's lighter", "home");
			result.Insert(board);
		}

		// The houses: the dose, names brought home, the names read
		RZ_Safehouses houses = m_GM.GetSafehouses();
		for (int i = 0; houses && i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			RZ_EpiSpot house = Make("house." + i.ToString(), false, RZ_Mission.LandPointNear(h.m_vPos, 5, 8), "Safehouse: " + h.m_sName, m_Epi.GetBeats().PlaceLine(h.m_vPos, 30));
			RZ_EpiOutlet ho = outlets.Find("house." + i.ToString());
			if (!ho)
				ho = outlets.Find("batch." + i.ToString());

			if (!ho && outlets.Find("pan") && outlets.Find("pan").house == i)
				ho = outlets.Find("pan");

			if (ho)
				house.Verb(ho.label, "dose:" + ho.key);

			house.Verb("Bring a name home", "home");
			house.Verb("Read the names at supper", "read");
			result.Insert(house);
		}

		// Bastion's nail, behind the radio at every holding of Bastion's; its posts' outlets
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			foreach (RZ_Holding hold : territory.GetHoldings())
			{
				if (hold.m_eFaction != RZ_EFaction.MILITARY)
					continue;

				RZ_EpiSpot nail = Make("post." + hold.m_iId.ToString(), false, RZ_Mission.LandPointNear(hold.m_vPos, 18, 26), hold.GetTitle(), "A nail behind the radio, with tags on it.");
				RZ_EpiOutlet po = outlets.Find("post." + hold.m_iId.ToString());
				if (po)
					nail.Verb(po.label, "dose:" + po.key);

				nail.Verb("Hang the tags on the nail", "home");
				result.Insert(nail);
			}
		}

		// Keepsakes lying out, the open questlines' people, the station, the basket
		closing.Spots(result);
		m_Epi.GetRest().Spots(result);
	}

	//------------------------------------------------------------------------------------------------
	//! Every 30 s: the list is rebuilt from the state; a spot that stays keeps its body.
	protected void Refresh()
	{
		array<ref RZ_EpiSpot> want = {};
		Wanted(want);
		array<ref RZ_EpiSpot> keep = {};
		foreach (RZ_EpiSpot w : want)
		{
			RZ_EpiSpot old = FindKey(w.key);
			if (old)
			{
				w.id = old.id;
				w.ent = old.ent;
				w.extras = old.extras;
				w.lastShooter = old.lastShooter;
				w.lastShotAt = old.lastShotAt;
				w.alive = old.alive;
				// A place that stays keeps where it stands (its point was picked at random once)
				if (old.pos != w.pos)
					w.pos = old.pos;

				if (w.ent && w.Joined() != old.Joined())
					Label(w);
			}
			else
			{
				w.id = m_iNextId;
				m_iNextId++;
			}

			keep.Insert(w);
		}

		foreach (RZ_EpiSpot gone : m_aSpots)
		{
			bool stays = false;
			foreach (RZ_EpiSpot k : keep)
			{
				if (k.key == gone.key)
					stays = true;
			}

			if (!stays)
				Despawn(gone);
		}

		m_aSpots.Clear();
		foreach (RZ_EpiSpot ks : keep)
		{
			m_aSpots.Insert(ks);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_EpiSpot FindKey(string key)
	{
		foreach (RZ_EpiSpot s : m_aSpots)
		{
			if (s.key == key)
				return s;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_EpiSpot FindId(int id)
	{
		foreach (RZ_EpiSpot s : m_aSpots)
		{
			if (s.id == id)
				return s;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Force a refresh on the next tick (a choice changed what stands somewhere).
	void Dirty()
	{
		m_fRefresh = REFRESH_SECONDS;
	}

	// ================================================================ Bodies and props
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		m_fClock += dt;
		m_fRefresh += dt;
		if (m_fRefresh >= REFRESH_SECONDS)
		{
			m_fRefresh = 0;
			Refresh();
		}

		m_Epi.GetBeats().TickPlayers(ids, players);
		int up = CountUp();
		int spawned = 0;
		foreach (RZ_EpiSpot s : m_aSpots)
		{
			float near = RZ_Util.NearestDistance(s.pos, players);
			float spawnAt = SPAWN_RANGE;
			float despawnAt = DESPAWN_RANGE;
			if (s.scene)
			{
				spawnAt = SCENE_SPAWN;
				despawnAt = SCENE_DESPAWN;
			}

			if (s.ent)
			{
				CheckDead(s);
				if (near > despawnAt || players.IsEmpty())
					Despawn(s);

				continue;
			}

			if (near <= spawnAt && up < MAX_UP && spawned < SPAWNS_PER_TICK)
			{
				if (Spawn(s))
				{
					up++;
					spawned++;
				}
			}
		}

		if (m_iDeadStep >= 0 && m_iDeadStep != m_Epi.Step())
		{
			m_iQueueDead = 0;
			m_iDeadStep = -1;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected int CountUp()
	{
		int n = 0;
		foreach (RZ_EpiSpot s : m_aSpots)
		{
			if (s.ent)
				n += 1 + s.extras.Count();
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! A person (Puzzles > Civilian, in the body budget) or a table, tagged with the spot's choices.
	protected bool Spawn(RZ_EpiSpot s)
	{
		ResourceName prefab;
		if (s.person)
		{
			prefab = m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);
			RZ_Shared shared = RZ_Shared.Get();
			if (shared && !shared.CanSpawnBody(RZ_BodyBudget.EPILOGUE, s.pos))
				return false;
		}
		else
		{
			prefab = m_Epi.GetSettings().m_sTablePrefab;
			if (prefab.IsEmpty())
				prefab = m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.OBJECT);
		}

		if (prefab.IsEmpty())
			return false;

		s.ent = RZ_Util.SpawnPrefab(prefab, RZ_Util.GroundPos(s.pos), Math.RandomFloat(0, 360));
		if (!s.ent)
			return false;

		s.alive = true;
		Label(s);
		if (s.person && RZ_Shared.Get())
			RZ_Shared.Get().AddBody(RZ_BodyBudget.EPILOGUE, s.ent);

		// The rest of the scene: plain people around the first, inside the same budget
		for (int n = 1; s.person && n < s.people; n++)
		{
			vector at = RZ_Mission.LandPointNear(s.pos, 2, 6);
			RZ_Shared sh = RZ_Shared.Get();
			if (sh && !sh.CanSpawnBody(RZ_BodyBudget.EPILOGUE, at))
				break;

			IEntity e = RZ_Util.SpawnPrefab(prefab, RZ_Util.GroundPos(at), Math.RandomFloat(0, 360));
			if (!e)
				break;

			RZ_ClueComponent.Tag(e, RZ_Epilogue.TAG, s.id, "Talk to them");
			s.extras.Insert(e);
			if (sh)
				sh.AddBody(RZ_BodyBudget.EPILOGUE, e);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Label(RZ_EpiSpot s)
	{
		if (!s.ent)
			return;

		RZ_PuzzleObjectComponent.Tag(s.ent, RZ_Epilogue.TAG, s.id, "", s.Joined());
		string read = "Read it";
		if (s.person)
			read = "Talk to them";

		RZ_ClueComponent.Tag(s.ent, RZ_Epilogue.TAG, s.id, read);
	}

	//------------------------------------------------------------------------------------------------
	protected void Despawn(RZ_EpiSpot s)
	{
		if (s.ent)
			SCR_EntityHelper.DeleteEntityAndChildren(s.ent);

		foreach (IEntity e : s.extras)
		{
			if (e)
				SCR_EntityHelper.DeleteEntityAndChildren(e);
		}

		s.ent = null;
		s.extras.Clear();
	}

	// ================================================================ Safe ground
	//------------------------------------------------------------------------------------------------
	//! One of a scene's people was killed: 15 Haven Infamy, 5 with the route's faction, and the
	//! scene stays one short until the next step.
	protected void CheckDead(RZ_EpiSpot s)
	{
		if (!s.person || !s.alive)
			return;

		bool dead = !RZ_Util.IsCharacterAlive(s.ent);
		foreach (IEntity e : s.extras)
		{
			if (e && !RZ_Util.IsCharacterAlive(e))
				dead = true;
		}

		if (!dead)
			return;

		s.alive = false;
		int killer = RZ_Util.GetKillerPlayerId(s.ent);
		if (killer <= 0 && s.lastShooter > 0 && m_fClock - s.lastShotAt <= 30)
			killer = s.lastShooter;

		if (s.key == "queue")
		{
			m_iQueueDead++;
			m_iDeadStep = m_Epi.Step();
		}

		RZ_Log.Info(string.Format("Epilogue: one of the people at %1 was killed (by player %2).", s.key, killer));
		if (killer <= 0)
			return;

		m_GM.AddInfamy(killer, RZ_EFaction.SURVIVORS, 15);
		int route = m_Epi.Route();
		if (route >= 0 && route < RZ_CureFinale.INDEP && route != RZ_EFaction.SURVIVORS)
			m_GM.AddInfamy(killer, route, 5);

		if (m_GM.GetHostility())
			m_GM.GetHostility().AddGrudge(killer, s.faction, s.pos);

		m_GM.NotifyPlayer(killer, s.title, "Somebody's bag on the verge. Nobody touches it. Every house will hear who did it by tonight. (+15 Haven Infamy)");
	}

	//------------------------------------------------------------------------------------------------
	//! The gunfire hook: firing on the scenes' safe ground, and at their people.
	void OnPlayerShot(int playerId, vector pos)
	{
		if (m_GM.GetSpawner() && m_GM.GetSpawner().IsZombieNear(pos, SHOT_NEAR))
			return;

		foreach (RZ_EpiSpot s : m_aSpots)
		{
			if (!s.scene)
				continue;

			float d = RZ_Util.DistXZ(s.pos, pos);
			if (d > SAFE_RADIUS)
				continue;

			int f = s.faction;
			if (!RZ_Factions.IsValid(f))
				f = RZ_EFaction.SURVIVORS;

			string key = string.Format("%1|%2", playerId, s.key);
			float last;
			if (m_mShotTold.Find(key, last) && m_fClock - last < 20)
				continue;

			m_mShotTold.Set(key, m_fClock);
			m_GM.AddInfamy(playerId, f, 2);
			if (m_GM.GetHostility())
				m_GM.GetHostility().AddGrudge(playerId, f, s.pos);

			string text = string.Format("Shots by the queue. Everyone flinches, and somebody's guards turn your way. (+2 Infamy with %1)", RZ_Factions.GetNameInText(f));
			if (s.ent && d <= SHOT_NEAR)
			{
				s.lastShooter = playerId;
				s.lastShotAt = m_fClock;
				m_GM.AddInfamy(playerId, RZ_EFaction.SURVIVORS, 4);
				text = "You fired at the people waiting. They scatter, then come back, because there's nowhere else to wait. (+4 Haven Infamy)";
			}

			m_GM.NotifyPlayer(playerId, s.title, text);
			return;
		}
	}

	// ================================================================ Choices
	//------------------------------------------------------------------------------------------------
	//! "Talk to them" or "Read it" on a spot.
	void HandleRead(int playerId, int id)
	{
		RZ_EpiSpot s = FindId(id);
		if (!s)
			return;

		string text = s.talk;
		if (text.IsEmpty())
			text = "Nothing to read but the weather.";

		m_GM.NotifyPlayer(playerId, s.title, text);
		m_Epi.Met(playerId, s.key);
	}

	//------------------------------------------------------------------------------------------------
	//! A choice on a spot: a dose, or one of the closing work's verbs.
	void HandleChoice(int playerId, int id, int option, IEntity user)
	{
		RZ_EpiSpot s = FindId(id);
		if (!s || option < 0 || option >= s.verbs.Count())
			return;

		string verb = s.verbs[option];
		if (verb.StartsWith("dose:"))
		{
			m_Epi.GetOutlets().Ask(playerId, verb.Substring(5, verb.Length() - 5), user);
			Dirty();
			return;
		}

		m_Epi.GetClosing().Verb(playerId, verb, s, user);
		Dirty();
	}
}
