//! The world half of new-player guidance (docs/DESIGN.md, "New-player guidance"): things
//! you find and voices you hear.
//! - a spawn scenario with a small clue near every new life, each nudging toward a faction;
//! - a wind-up radio at every spawn, and the open channels it picks up;
//! - Bastion loudspeakers and Red Hand taunts near their seats;
//! - Halcyon flyers in Haven safehouses, wanderer gossip, and a wanderer who finds a
//!   brand-new player.
//!
//! Clue props are tagged with RZ_ClueComponent (mission id TAG) and their reads routed
//! here by RZ_PuzzleActionHelper. Nothing physical stays: clues are removed after a while.

enum RZ_EGuideScenario
{
	CAR,		// crashed car on a back road: a note pointing at the nearest Haven safehouse
	BASEMENT,	// scratched door: the roads out are concrete, the line shoots
	CHURCH,		// Halcyon flyer: samples wanted at the lab
	FARMHOUSE,	// a wind-up radio on the table: the broadcast, then the Haven's open channel
	ATTIC,		// a dead raider with a red hand on his jacket and a list of toll roads
	BARRICADE	// a dead Bastion soldier: "Hold the line. Await relief."
}

//------------------------------------------------------------------------------------------------
//! A clue prop (and the scenery next to it) spawned for a player, removed after a while.
class RZ_GuideClue
{
	IEntity m_Entity;		// the readable prop, if any
	IEntity m_Extra;		// scenery: a wreck, a barricade, a radio without a clue component
	string m_sTitle;
	string m_sText;
	float m_fBorn;
}

//------------------------------------------------------------------------------------------------
//! Something to do a few seconds from now (give the radio, play the next radio line).
class RZ_GuidePending
{
	int m_iPlayer;
	float m_fAt;
	string m_sTitle;
	string m_sText;
	bool m_bGiveRadio;
}

//------------------------------------------------------------------------------------------------
class RZ_GuidanceWorld
{
	static const int TAG = -5;					// RZ_ClueComponent / RZ_PuzzleObjectComponent mission id
	static const int FLYER_BASE = 100000;		// clue index of a safehouse flyer: FLYER_BASE + safehouse index

	protected static const float RADIO_GIVE_DELAY = 3;
	protected static const float FARMHOUSE_DELAY = 25;
	protected static const float CLUE_KEEP_RADIUS = 30;
	protected static const float CHURCH_RADIUS = 300;
	protected static const float SHOUT_MIN = 80;
	protected static const float SHOUT_MAX = 450;
	protected static const float NUDGE_START = 600;		// a far wanderer starts (as data) this far away
	protected static const float NUDGE_RETRY = 120;
	protected static const float NUDGE_UNTIL = 900;		// only in the first 15 minutes
	protected static const float GOSSIP_CHANCE = 0.35;	// for familiar players and veterans

	protected RZ_GameModeComponent m_GM;
	protected RZ_Guidance m_Guide;
	protected RZ_GuidanceSettings m_Settings;
	protected ref array<ref RZ_GuideClue> m_aClues = {};
	protected ref array<ref RZ_GuidePending> m_aPending = {};
	protected ref map<string, float> m_mShoutAt = new map<string, float>();

	//------------------------------------------------------------------------------------------------
	void RZ_GuidanceWorld(RZ_GameModeComponent gm, RZ_Guidance guide, RZ_GuidanceSettings settings)
	{
		m_GM = gm;
		m_Guide = guide;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Rng NewRng()
	{
		return new RZ_Rng(Math.RandomInt(1, 2000000000));
	}

	//------------------------------------------------------------------------------------------------
	protected string TownNear(vector pos)
	{
		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(pos));
		if (s)
			return s.m_sName;

		return "town";
	}

	// ================================================================ Spawn
	//------------------------------------------------------------------------------------------------
	//! A new life: the wind-up radio (a moment later, once the inventory exists), a scenario,
	//! the radio and wanderer timers.
	void OnPlayerSpawned(int playerId, IEntity ent, RZ_PlayerProfile p, RZ_GuidePlayer st, int level)
	{
		if (!m_Settings.m_sRadioPrefab.IsEmpty())
		{
			RZ_GuidePending give = new RZ_GuidePending();
			give.m_iPlayer = playerId;
			give.m_fAt = m_Guide.GetClock() + RADIO_GIVE_DELAY;
			give.m_bGiveRadio = true;
			m_aPending.Insert(give);
		}

		// The first open-channel message comes about halfway through the usual interval
		st.m_fRadioTimer = RadioInterval(level) * 0.5;
		st.m_fNudgeAt = Math.RandomFloat(120, 420);

		if (m_Settings.m_bScenarios)
			RunScenario(playerId, ent, p, level);
	}

	//------------------------------------------------------------------------------------------------
	protected void GiveRadio(int playerId)
	{
		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!ent || !RZ_Util.IsCharacterAlive(ent) || RZ_Radio.HasRadio(m_GM, playerId, ent))
			return;

		IEntity item = RZ_Util.SpawnPrefab(m_Settings.m_sRadioPrefab, ent.GetOrigin() + Vector(0, 0.3, 0));
		if (!item)
			return;

		RZ_GameModeComponent.TrackLooseItem(item); // dropped and forgotten, it's cleaned up in time

		// If it doesn't fit (or the insert call doesn't work), it lies at their feet
		if (!RZ_Util.InsertIntoInventory(ent, item))
			m_GM.NotifyPlayer(playerId, RZ_Guidance.RADIO_TITLE, "There's a wind-up radio at your feet. Somebody's. Yours now.");
	}

	// ================================================================ Scenarios
	//------------------------------------------------------------------------------------------------
	protected void Offer(notnull array<int> pool, int scenario, int newWeight, int level, int last)
	{
		if (scenario == last)
			return;

		int weight = 2;
		if (level == RZ_Guidance.LEVEL_NEW)
			weight = newWeight;

		for (int i = 0; i < weight; i++)
		{
			pool.Insert(scenario);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void FillPool(notnull array<int> pool, int level, int last)
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		bool haven = houses && houses.Count() > 0;
		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		RZ_Leaders leaders = m_GM.GetLeaders();

		// Brand-new players lean toward the Haven, whose safehouse is their first goal
		if (haven)
		{
			Offer(pool, RZ_EGuideScenario.CAR, 4, level, last);
			Offer(pool, RZ_EGuideScenario.FARMHOUSE, 3, level, last);
		}

		// The basement's "NOBODY GETS OUT" is only true while the line is sealed (not after the cure)
		if (perimeter && perimeter.IsActive() && m_GM.GetFacts().PerimeterSealed())
			Offer(pool, RZ_EGuideScenario.BASEMENT, 2, level, last);

		// "Infected samples wanted": not once the cure is out
		if (m_GM.HasLab() && !m_GM.GetFacts().IsCured())
			Offer(pool, RZ_EGuideScenario.CHURCH, 2, level, last);

		if (leaders && leaders.GetStronghold(RZ_EFaction.BANDITS))
			Offer(pool, RZ_EGuideScenario.ATTIC, 1, level, last);

		if (leaders && leaders.GetStronghold(RZ_EFaction.MILITARY))
			Offer(pool, RZ_EGuideScenario.BARRICADE, 1, level, last);
	}

	//------------------------------------------------------------------------------------------------
	//! Picks a scenario by experience (never the same as last life's if there's another)
	//! and puts its clue a few metres from the player. Veterans get a one-line scene: the
	//! clue is always there.
	protected void RunScenario(int playerId, IEntity ent, RZ_PlayerProfile p, int level)
	{
		array<int> pool = {};
		FillPool(pool, level, p.m_iGuideLastScenario);
		if (pool.IsEmpty())
			FillPool(pool, level, -1);

		if (pool.IsEmpty())
			return;

		int scenario = pool.GetRandomElement();
		p.m_iGuideLastScenario = scenario;

		bool brief = level >= 2;
		vector spawn = ent.GetOrigin();
		string town = TownNear(spawn);
		RZ_Rng rng = NewRng();

		switch (scenario)
		{
			case RZ_EGuideScenario.CAR: RunCar(playerId, spawn, town, brief, rng); break;
			case RZ_EGuideScenario.BASEMENT: RunBasement(playerId, spawn, town, brief, rng); break;
			case RZ_EGuideScenario.CHURCH: RunChurch(playerId, spawn, town, brief); break;
			case RZ_EGuideScenario.FARMHOUSE: RunFarmhouse(playerId, spawn, town, brief, rng); break;
			case RZ_EGuideScenario.ATTIC: RunAttic(playerId, spawn, town, brief, rng); break;
			case RZ_EGuideScenario.BARRICADE: RunBarricade(playerId, spawn, town, brief, rng); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The scene message. If no prop could be spawned at all, the clue's text comes with it.
	protected void Scene(int playerId, int scenario, string scene, bool placed, string clueTitle, string clueText)
	{
		string text = scene;
		if (!placed && !clueText.IsEmpty())
			text += string.Format("\n\n%1:\n%2", clueTitle, clueText);

		m_GM.NotifyPlayer(playerId, RZ_GuidanceText.ScenarioTitle(scenario), text);
	}

	//------------------------------------------------------------------------------------------------
	protected void RunCar(int playerId, vector spawn, string town, bool brief, RZ_Rng rng)
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		RZ_Safehouse h = houses.Get(houses.FindNearest(spawn));
		if (!h)
			return;

		vector pos = RZ_Mission.LandPointNear(spawn, 4, 8);
		string direction = RZ_Compass.DirectionWord(RZ_Compass.Bearing(pos, h.m_vPos));
		string text = RZ_GuidanceText.CarNote(rng, h.m_sName, direction);
		string title = "Note on the dashboard";
		bool placed = PlaceClue(m_Settings.m_sCarWreckPrefab, RZ_EPuzzleProp.CLUE, true, pos, "Read the note on the dashboard", title, text);
		Scene(playerId, RZ_EGuideScenario.CAR, RZ_GuidanceText.Scene(RZ_EGuideScenario.CAR, town, "", brief), placed, title, text);
	}

	//------------------------------------------------------------------------------------------------
	protected void RunBasement(int playerId, vector spawn, string town, bool brief, RZ_Rng rng)
	{
		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		string roadDir = "";
		string edgeDir = "";
		vector checkpoint;
		if (perimeter.GetNearestCheckpoint(spawn, checkpoint))
		{
			roadDir = RZ_Compass.DirectionWord(RZ_Compass.Bearing(spawn, checkpoint));
			roadDir.ToUpper();
		}
		else
		{
			vector inward = perimeter.InwardAt(spawn);
			edgeDir = RZ_Compass.DirectionWord(RZ_Compass.Bearing(spawn, spawn - inward * 100));
			edgeDir.ToUpper();
		}

		int day = m_GM.GetBroadcast().GetDay();
		string text = RZ_GuidanceText.DoorText(rng, day, roadDir, edgeDir);
		string title = "Scratched into the door";
		bool placed = PlaceClue(m_Settings.m_sDoorPrefab, RZ_EPuzzleProp.CLUE, true, RZ_Mission.LandPointNear(spawn, 2, 4), "Look at the door", title, text);
		Scene(playerId, RZ_EGuideScenario.BASEMENT, RZ_GuidanceText.Scene(RZ_EGuideScenario.BASEMENT, town, "", brief), placed, title, text);
	}

	//------------------------------------------------------------------------------------------------
	//! The flyer goes on the nearest church within 300 m if the town has one.
	protected void RunChurch(int playerId, vector spawn, string town, bool brief)
	{
		array<string> kinds = {"church"};
		RZ_Landmark church = RZ_LandmarkFinder.FindNearest(spawn, CHURCH_RADIUS, kinds);
		vector pos;
		string where = "";
		if (church)
		{
			pos = RZ_Mission.LandPointNear(church.m_vPos, 3, 8);
			where = RZ_Util.DirectionText(spawn, pos);
		}
		else
		{
			pos = RZ_Mission.LandPointNear(spawn, 2, 5);
		}

		string text = RZ_GuidanceText.FlyerText(LabTown(), false);
		string title = "Flyer";
		bool placed = PlaceClue(FlyerPrefab(), RZ_EPuzzleProp.CLUE, true, pos, "Read the flyer", title, text);
		Scene(playerId, RZ_EGuideScenario.CHURCH, RZ_GuidanceText.Scene(RZ_EGuideScenario.CHURCH, town, where, brief), placed, title, text);
	}

	//------------------------------------------------------------------------------------------------
	//! The radio on the table plays the broadcast now and the Haven's open channel a little later.
	protected void RunFarmhouse(int playerId, vector spawn, string town, bool brief, RZ_Rng rng)
	{
		string havenTown = m_Guide.SafehouseText(spawn);
		string channel = RZ_GuidanceText.HavenChannel(rng, havenTown);
		ResourceName prop = m_Settings.m_sTableRadioPrefab;
		if (prop.IsEmpty())
			prop = m_Settings.m_sRadioPrefab;

		// No note next to a radio: if the prop can't be read it's just a radio on the floor
		PlaceClue(prop, RZ_EPuzzleProp.CLUE, false, RZ_Mission.LandPointNear(spawn, 2, 4), "Listen to the radio", RZ_Guidance.RADIO_TITLE, channel);

		string broadcast = m_GM.GetBroadcast().GetText();
		m_GM.NotifyPlayer(playerId, RZ_GuidanceText.ScenarioTitle(RZ_EGuideScenario.FARMHOUSE), RZ_GuidanceText.Scene(RZ_EGuideScenario.FARMHOUSE, town, broadcast, brief));

		RZ_GuidePending next = new RZ_GuidePending();
		next.m_iPlayer = playerId;
		next.m_fAt = m_Guide.GetClock() + FARMHOUSE_DELAY;
		next.m_sTitle = RZ_Guidance.RADIO_TITLE;
		next.m_sText = "The broadcast cuts out. Static, then a woman's voice, closer, tired:\n" + channel;
		m_aPending.Insert(next);
	}

	//------------------------------------------------------------------------------------------------
	protected void RunAttic(int playerId, vector spawn, string town, bool brief, RZ_Rng rng)
	{
		RZ_Leader boss = m_GM.GetLeaders().GetStronghold(RZ_EFaction.BANDITS);
		if (!boss)
			return;

		// Toll roads: the Red Hand's other seats first, then any towns
		array<string> places = {};
		array<RZ_Leader> seats = {};
		m_GM.GetLeaders().GetFactionLeaders(RZ_EFaction.BANDITS, seats);
		foreach (RZ_Leader seat : seats)
		{
			if (seat != boss && !places.Contains(seat.m_sPlace))
				places.Insert(seat.m_sPlace);
		}

		int guard = 0;
		array<ref RZ_Settlement> settlements = m_GM.GetSettlements();
		while (places.Count() < 2 && !settlements.IsEmpty() && guard < 20)
		{
			guard++;
			string name = settlements[rng.Int(0, settlements.Count())].m_sName;
			if (name != boss.m_sPlace && !places.Contains(name))
				places.Insert(name);
		}

		while (places.Count() < 2)
		{
			places.Insert("the old mill");
		}

		string text = RZ_GuidanceText.RaiderText(places[0], places[1], boss.m_sPlace, boss.m_sShort);
		string title = "The dead man";
		bool placed = PlaceClue(m_Settings.m_sRaiderBodyPrefab, RZ_EPuzzleProp.BODY, true, RZ_Mission.LandPointNear(spawn, 2, 5), "Search the body", title, text);
		Scene(playerId, RZ_EGuideScenario.ATTIC, RZ_GuidanceText.Scene(RZ_EGuideScenario.ATTIC, town, "", brief), placed, title, text);
	}

	//------------------------------------------------------------------------------------------------
	protected void RunBarricade(int playerId, vector spawn, string town, bool brief, RZ_Rng rng)
	{
		RZ_Leader bastion = m_GM.GetLeaders().GetStronghold(RZ_EFaction.MILITARY);
		if (!bastion)
			return;

		vector pos = RZ_Mission.LandPointNear(spawn, 3, 6);
		TrackScenery(RZ_Util.SpawnPrefab(m_Settings.m_sBarricadePrefab, RZ_Mission.LandPointNear(pos, 1, 2), Math.RandomFloat(0, 360)));

		string text = RZ_GuidanceText.OrdersText(rng, m_GM.GetBroadcast().GetDay(), bastion.m_sPlace);
		string title = "The soldier's orders";
		bool placed = PlaceClue(m_Settings.m_sSoldierBodyPrefab, RZ_EPuzzleProp.BODY, true, pos, "Read his orders", title, text);
		Scene(playerId, RZ_EGuideScenario.BARRICADE, RZ_GuidanceText.Scene(RZ_EGuideScenario.BARRICADE, town, "", brief), placed, title, text);
	}

	// ================================================================ Clue props
	//------------------------------------------------------------------------------------------------
	protected ResourceName FlyerPrefab()
	{
		if (!m_Settings.m_sFlyerPrefab.IsEmpty())
			return m_Settings.m_sFlyerPrefab;

		return m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CLUE);
	}

	//------------------------------------------------------------------------------------------------
	protected string LabTown()
	{
		RZ_Settlement s = m_GM.GetSettlement(m_GM.GetLabSettlement());
		if (s)
			return s.m_sName;

		return "the lab town";
	}

	//------------------------------------------------------------------------------------------------
	protected int AddEntry(RZ_GuideClue c)
	{
		foreach (int i, RZ_GuideClue old : m_aClues)
		{
			if (!old)
			{
				m_aClues[i] = c;
				return i;
			}
		}

		m_aClues.Insert(c);
		return m_aClues.Count() - 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Scenery to remove with the clues (a barricade).
	protected void TrackScenery(IEntity e)
	{
		if (!e)
			return;

		RZ_GuideClue c = new RZ_GuideClue();
		c.m_Extra = e;
		c.m_fBorn = m_Guide.GetClock();
		AddEntry(c);
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns `prop` at pos. If it can be read (RZ_ClueComponent) it's the clue; otherwise it's
	//! scenery and, with noteFallback, the fallback puzzle prop (a note or a body) lies next to
	//! it. True if something readable is in the world.
	protected bool PlaceClue(ResourceName prop, RZ_EPuzzleProp fallback, bool noteFallback, vector pos, string label, string title, string text)
	{
		IEntity clue;
		IEntity extra;
		if (!prop.IsEmpty())
		{
			IEntity spawned = RZ_Util.SpawnPrefab(prop, pos, Math.RandomFloat(0, 360));
			if (spawned && spawned.FindComponent(RZ_ClueComponent))
				clue = spawned;
			else
				extra = spawned;
		}

		if (!clue && noteFallback)
		{
			vector notePos = pos;
			if (extra)
				notePos = RZ_Mission.LandPointNear(pos, 1, 2.5);

			clue = RZ_Util.SpawnPrefab(m_GM.GetPuzzlePrefab(fallback), notePos, Math.RandomFloat(0, 360));
		}

		if (!clue && !extra)
			return false;

		RZ_GuideClue c = new RZ_GuideClue();
		c.m_Entity = clue;
		c.m_Extra = extra;
		c.m_sTitle = title;
		c.m_sText = text;
		c.m_fBorn = m_Guide.GetClock();
		int idx = AddEntry(c);
		if (!clue)
			return false;

		TagReadable(clue, idx, label);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Routes the read here, and hides the generic object's use and choice slots ("|" labels
	//! every slot with nothing, which hides them all).
	protected void TagReadable(IEntity e, int idx, string label)
	{
		RZ_ClueComponent.Tag(e, TAG, idx, label);
		RZ_PuzzleObjectComponent.Tag(e, TAG, idx, "|", "|");
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_ReadClueAction (RZ_PuzzleActionHelper.RouteRead).
	void HandleRead(int playerId, int idx, IEntity owner)
	{
		if (idx >= FLYER_BASE)
		{
			RZ_Reader.Show(m_GM, playerId, "Flyer on the wall", "Printed, pinned at the corners", RZ_GuidanceText.FlyerText(LabTown(), true));
			m_Guide.OnClueRead(playerId);
			return;
		}

		RZ_GuideClue c;
		if (idx >= 0 && idx < m_aClues.Count())
			c = m_aClues[idx];

		if (!c || c.m_sText.IsEmpty())
		{
			m_GM.NotifyPlayer(playerId, "Nothing", "Whatever was written here has run in the rain. You can't make out a word.");
			return;
		}

		RZ_Reader.Show(m_GM, playerId, c.m_sTitle, "", c.m_sText);
		m_Guide.OnClueRead(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Halcyon's flyer on a safehouse wall, spawned with the safehouse's people (and removed
	//! with them). Null if flyers are off, there's no lab or no prefab.
	IEntity SpawnSafehouseFlyer(int safehouseIdx, vector center)
	{
		if (!m_Settings.m_bEnabled || !m_Settings.m_bSafehouseFlyers || !m_GM.HasLab())
			return null;

		IEntity e = RZ_Util.SpawnPrefab(FlyerPrefab(), RZ_Mission.LandPointNear(center, 2, 6), Math.RandomFloat(0, 360));
		if (e)
			TagReadable(e, FLYER_BASE + safehouseIdx, "Read the flyer");

		return e;
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(array<IEntity> players, bool poll)
	{
		for (int i = m_aPending.Count() - 1; i >= 0; i--)
		{
			RZ_GuidePending job = m_aPending[i];
			if (job.m_fAt > m_Guide.GetClock())
				continue;

			if (job.m_bGiveRadio)
				GiveRadio(job.m_iPlayer);
			else
				m_GM.NotifyPlayer(job.m_iPlayer, job.m_sTitle, job.m_sText);

			m_aPending.Remove(i);
		}

		if (poll)
			CleanupClues(players);
	}

	//------------------------------------------------------------------------------------------------
	//! Clues go after m_fClueMinutes once nobody is within 30 m, and after twice that anyway.
	//! Anything a player picked up stays theirs.
	protected void CleanupClues(array<IEntity> players)
	{
		float life = m_Settings.m_fClueMinutes * 60;
		for (int i = 0; i < m_aClues.Count(); i++)
		{
			RZ_GuideClue c = m_aClues[i];
			if (!c)
				continue;

			float age = m_Guide.GetClock() - c.m_fBorn;
			if (age < life)
				continue;

			IEntity probe = c.m_Entity;
			if (!probe)
				probe = c.m_Extra;

			if (probe && age < life * 2 && RZ_Util.NearestDistance(probe.GetOrigin(), players) <= CLUE_KEEP_RADIUS)
				continue;

			RemoveProp(c.m_Entity);
			RemoveProp(c.m_Extra);
			m_aClues[i] = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveProp(IEntity e)
	{
		if (!e || RZ_Util.GetCarrierPlayerId(e) > 0)
			return;

		RZ_Util.DeleteUnit(e);
	}

	//------------------------------------------------------------------------------------------------
	//! Every 10 s per player: the radio, the wanderer who comes looking, voices near faction seats.
	void Poll(int playerId, IEntity ent, RZ_PlayerProfile p, RZ_GuidePlayer st, int level, float dt)
	{
		PollRadio(playerId, ent, st, level, dt);
		PollWanderer(playerId, p, st, level);
		PollShouts(playerId, ent, p, level);
	}

	// ================================================================ The wind-up radio
	//------------------------------------------------------------------------------------------------
	//! Low-frequency: about every 12 minutes for a brand-new player, every 42 for a veteran.
	protected float RadioInterval(int level)
	{
		array<float> mult = {1, 1.5, 2.5, 3.5};
		int i = Math.ClampInt(level, 0, 3);
		return Math.Max(60, m_Settings.m_fRadioMinutes * 60 * mult[i]);
	}

	//------------------------------------------------------------------------------------------------
	protected void PollRadio(int playerId, IEntity ent, RZ_GuidePlayer st, int level, float dt)
	{
		if (m_Settings.m_fRadioMinutes <= 0)
			return;

		float interval = RadioInterval(level);
		st.m_fRadioTimer = st.m_fRadioTimer + dt;
		if (st.m_fRadioTimer < interval)
			return;

		// Inventory is only looked at when a message is due; without a radio, look again in a minute
		if (!RZ_Radio.HasRadio(m_GM, playerId, ent))
		{
			st.m_fRadioTimer = interval - 60;
			return;
		}

		st.m_fRadioTimer = 0;
		PlayRadio(playerId, ent.GetOrigin(), st.m_iRadioStep);
		st.m_iRadioStep = st.m_iRadioStep + 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Open channels only: the Haven every other time, Bastion or the Red Hand in between.
	//! The government broadcast already reaches everyone; the radio only adds it when the
	//! server's broadcast loop is off, or after the cure (the loop has stopped and the radio is
	//! where people hear the new order).
	protected void PlayRadio(int playerId, vector pos, int step)
	{
		RZ_Rng rng = NewRng();
		RZ_Leaders leaders = m_GM.GetLeaders();
		string havenTown = m_Guide.SafehouseText(pos);

		if (step % 4 == 3 && (m_GM.GetBroadcastMinutes() <= 0 || m_GM.GetCure().IsCured()))
		{
			m_GM.NotifyPlayer(playerId, RZ_Guidance.RADIO_TITLE, m_GM.GetBroadcast().GetText());
			return;
		}

		if (step % 2 == 1 && leaders)
		{
			RZ_Leader bastion = leaders.GetStronghold(RZ_EFaction.MILITARY);
			RZ_Leader boss = leaders.GetStronghold(RZ_EFaction.BANDITS);
			bool redHandTurn = (step / 2) % 2 == 1;
			if (boss && (redHandTurn || !bastion))
			{
				m_GM.NotifyPlayer(playerId, RZ_Guidance.RADIO_TITLE + ": open channel", RZ_GuidanceText.RedHandChatter(rng, boss.m_sShort, boss.m_sPlace));
				return;
			}

			if (bastion)
			{
				m_GM.NotifyPlayer(playerId, RZ_Guidance.RADIO_TITLE + ": Bastion net", RZ_GuidanceText.BastionChatter(rng, havenTown));
				return;
			}
		}

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses && houses.Count() > 0)
			m_GM.NotifyPlayer(playerId, RZ_Guidance.RADIO_TITLE + ": the Haven", RZ_GuidanceText.HavenChannel(rng, havenTown));
	}

	// ================================================================ A wanderer finds you
	//------------------------------------------------------------------------------------------------
	//! Brand-new players: a Courier or a Scavenger comes looking for them a few minutes in.
	protected void PollWanderer(int playerId, RZ_PlayerProfile p, RZ_GuidePlayer st, int level)
	{
		if (level != RZ_Guidance.LEVEL_NEW || !m_Settings.m_bWandererFinds || st.m_bNudged)
			return;

		if (p.m_fLifeSeconds < st.m_fNudgeAt)
			return;

		if (p.m_fLifeSeconds > NUDGE_UNTIL)
		{
			st.m_bNudged = true;
			return;
		}

		int mask = RZ_Wanderers.RoleBit(RZ_EWandererRole.COURIER) | RZ_Wanderers.RoleBit(RZ_EWandererRole.SCAVENGER);
		if (RZ_Wanderers.NudgeToward(playerId, mask, NUDGE_START))
			st.m_bNudged = true;
		else
			st.m_fNudgeAt = p.m_fLifeSeconds + NUDGE_RETRY;
	}

	// ================================================================ Voices near faction seats
	//------------------------------------------------------------------------------------------------
	protected void PollShouts(int playerId, IEntity ent, RZ_PlayerProfile p, int level)
	{
		if (m_Settings.m_fShoutMinutes <= 0 || !m_GM.GetLeaders())
			return;

		float cooldown = m_Settings.m_fShoutMinutes * 60;
		if (level >= RZ_Guidance.LEVEL_VETERAN)
			cooldown = cooldown * 2;

		TryShout(playerId, ent.GetOrigin(), p, RZ_EFaction.MILITARY, cooldown);
		TryShout(playerId, ent.GetOrigin(), p, RZ_EFaction.BANDITS, cooldown);
	}

	//------------------------------------------------------------------------------------------------
	//! Between 80 and 450 m from a Bastion seat you hear its loudspeaker (naming the Haven,
	//! warning of armed groups); near a Red Hand seat, someone on a roof makes fun of Bastion.
	protected void TryShout(int playerId, vector pos, RZ_PlayerProfile p, RZ_EFaction faction, float cooldown)
	{
		string key = string.Format("%1:%2", p.m_sIdentity, faction);
		float last;
		if (m_mShoutAt.Find(key, last) && m_Guide.GetClock() - last < cooldown)
			return;

		array<RZ_Leader> seats = {};
		m_GM.GetLeaders().GetFactionLeaders(faction, seats);
		foreach (RZ_Leader l : seats)
		{
			float d = RZ_Util.DistXZ(pos, l.m_vPos);
			if (d < SHOUT_MIN || d > SHOUT_MAX)
				continue;

			m_mShoutAt.Set(key, m_Guide.GetClock());
			RZ_Rng rng = NewRng();
			if (faction == RZ_EFaction.MILITARY)
			{
				m_GM.NotifyPlayer(playerId, "Loudspeaker, Bastion post", RZ_GuidanceText.BastionLoudspeaker(rng, m_Guide.SafehouseText(l.m_vPos)));
				return;
			}

			string boss = l.m_sShort;
			RZ_Leader top = m_GM.GetLeaders().GetStronghold(RZ_EFaction.BANDITS);
			if (top)
				boss = top.m_sShort;

			m_GM.NotifyPlayer(playerId, "Someone on a roof", RZ_GuidanceText.RedHandShout(rng, boss));
			return;
		}
	}

	// ================================================================ Wanderer gossip
	//------------------------------------------------------------------------------------------------
	//! One line about a faction for a wanderer's "Talk": always for new and learning players,
	//! sometimes for the rest. Prefers a faction whose stronghold leader they haven't met.
	string WandererGossip(int playerId, int role, vector from)
	{
		RZ_Leaders leaders = m_GM.GetLeaders();
		if (!m_Settings.m_bEnabled || !leaders)
			return "";

		if (m_Guide.GetPlayerLevel(playerId) >= 2 && Math.RandomFloat01() > GOSSIP_CHANCE)
			return "";

		array<RZ_Leader> unmet = {};
		array<RZ_Leader> all = {};
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			RZ_Leader l = leaders.GetStronghold(f);
			if (!l || !l.IsPresent())
				continue;

			all.Insert(l);
			if (!leaders.HasMet(l.m_iId, playerId))
				unmet.Insert(l);
		}

		if (all.IsEmpty())
			return "";

		RZ_Leader pick;
		if (!unmet.IsEmpty())
			pick = unmet.GetRandomElement();
		else
			pick = all.GetRandomElement();

		return RZ_GuidanceText.Gossip(NewRng(), role, pick.m_eFaction, pick.m_sPlace);
	}
}
