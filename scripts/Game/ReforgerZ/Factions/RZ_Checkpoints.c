//! Faction posts and camps inside the Zone (docs/REPUTATION.md, "Checkpoints, roadblocks and
//! camps"). Server only, owned by the game mode, polled on its tick. The Perimeter's sealed
//! checkpoints are a different thing (RZ_Perimeter) and stay sealed to everyone.
//!
//! - Road posts: now and then a Bastion checkpoint or a Red Hand roadblock goes up on a road
//!   between a player and one of that faction's holdings (at most two at once, for 45 minutes).
//!   Data first: the barrier and crew exist only while a player is near.
//!     Bastion: Known or better are waved through unsearched; anyone else who walks up to the
//!     barrier is searched (contraband taken); walking off once you're at the barrier is running
//!     it (a little Infamy, and the post opens fire for a few minutes).
//!     Red Hand: Trusted or better pass; Known pay a toll in Favor; anyone else is told to turn
//!     around, and shot at (a few minutes' hostility) if they come on.
//! - Surrender: a player with Bastion Infamy (Watched or Hunted) who walks up to a Bastion
//!   checkpoint with no firearm and waits gives themselves up: searched, a little Bastion Infamy
//!   off (once an hour), and Bastion's AI holds its fire for five minutes. A Hunted player walking
//!   up unarmed isn't shot while they come in (a short stand-down, renewed while they do).
//! - Camps: a faction's outposts and stronghold warn off players it watches and turn them away at
//!   the gate; armed and coming on after the warning, they're fired on for a few minutes.
//! - Safe zones: strongholds and the lab. Entering one, you're told the rule; firing inside one
//!   (with no infected near) costs a little Infamy with the faction and draws their fire.
//! Every shooting here is the timed grudge from retaliation (RZ_Hostility.AddGrudge): it restarts,
//! never adds up, so none of it becomes lasting hostility by itself.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Factions > Checkpoints.
[BaseContainerProps()]
class RZ_CheckpointSettings
{
	[Attribute("1", desc: "Bastion checkpoints and Red Hand roadblocks on roads inside the Zone (needs territory and a road network)")]
	bool m_bRoadPosts;

	[Attribute("2", UIWidgets.Slider, "Road posts up at once on the map", "0 4 1")]
	int m_iMaxPosts;

	[Attribute("15", desc: "Real minutes between tries to put up a new post near a player")]
	float m_fPostMinutes;

	[Attribute("45", desc: "Real minutes a post stays up (it goes once nobody is near)")]
	float m_fPostLifetimeMinutes;

	[Attribute("2500", desc: "A post goes up within this distance (m) of one of its faction's holdings")]
	float m_fHoldingRange;

	[Attribute("600", desc: "The barrier and crew exist only while a player is this close (m)")]
	float m_fSpawnRadius;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bastion checkpoint barrier: a small composition (sandbags, a barrier pole). Optional: the crew alone without it", "et")]
	ResourceName m_sBastionBarrierPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Red Hand roadblock: a car wreck or a log across the road. Optional", "et")]
	ResourceName m_sRedHandBarrierPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bastion checkpoint crew: an AI group or character on MIL (optional; falls back to Territory > Bastion guard). Without one, no checkpoints", "et")]
	ResourceName m_sBastionCrewPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Red Hand roadblock crew: an AI group on BAND (optional; falls back to Territory > Red Hand guard, then Missions > Red Hand group). Without one, no roadblocks", "et")]
	ResourceName m_sRedHandCrewPrefab;

	[Attribute("150", desc: "Posts call out to players within this distance (m)")]
	float m_fWarnRadius;

	[Attribute("35", desc: "The barrier: within this distance (m) you're stopped, searched or charged")]
	float m_fStopRadius;

	[Attribute("15", desc: "Seconds a Bastion search takes (stay at the barrier)")]
	float m_fSearchSeconds;

	[Attribute("suppressor,silencer,explosive,c4,demo_charge", desc: "What a Bastion search takes (keywords, lower case, in prefab names). At most three items")]
	string m_sContrabandWords;

	[Attribute("3", desc: "Bastion Infamy for running a checkpoint (leaving the barrier before the search is done)")]
	int m_iRunInfamy;

	[Attribute("10", desc: "Red Hand toll (Red Hand Favor) for players Known with them")]
	int m_iToll;

	[Attribute("1", desc: "Surrender at Bastion checkpoints: unarmed, with Bastion Infamy, wait at the barrier")]
	bool m_bSurrender;

	[Attribute("10", desc: "Surrender: seconds at the barrier with no firearm")]
	float m_fSurrenderSeconds;

	[Attribute("8", desc: "Surrender: Bastion Infamy worked off (before the stronghold leader's temper)")]
	int m_iSurrenderInfamy;

	[Attribute("60", desc: "Surrender: real minutes before the same player can surrender again")]
	float m_fSurrenderCooldownMinutes;

	[Attribute("300", desc: "Surrender: seconds Bastion's AI holds its fire afterwards")]
	float m_fStandDownSeconds;

	[Attribute("1", desc: "Camps: outposts and strongholds warn off players their faction watches, and turn them away at the gate")]
	bool m_bCamps;

	[Attribute("150", desc: "Camps: the warning comes at this distance (m)")]
	float m_fCampWarnRadius;

	[Attribute("50", desc: "Camps: the gate. Watched and armed past it after the warning, you're fired on for a few minutes")]
	float m_fCampRefuseRadius;

	[Attribute("1", desc: "Safe zones: strongholds and the lab tell you the rule as you come in, and firing inside costs Infamy")]
	bool m_bSafeZones;

	[Attribute("80", desc: "Safe zones: radius (m) around a stronghold or the lab")]
	float m_fSafeZoneRadius;

	[Attribute("2", desc: "Safe zones: Infamy with the faction for firing inside (at most once every 20 s; never with an infected within 40 m)")]
	int m_iSafeZoneShotInfamy;
}

//------------------------------------------------------------------------------------------------
//! One player at one post.
class RZ_PostContact
{
	bool m_bWarned;
	bool m_bCleared;		// waved through, searched or paid
	bool m_bAtBarrier;		// reached the barrier (leaving now is running it)
	bool m_bPunished;
	float m_fWaited;		// seconds at the barrier
}

//------------------------------------------------------------------------------------------------
//! A Bastion checkpoint or a Red Hand roadblock. Data, plus bodies while a player is near.
class RZ_RoadPost
{
	RZ_EFaction m_eFaction;
	vector m_vPos;
	float m_fYaw;
	float m_fUntil;
	string m_sNear;
	bool m_bSpawned;
	ref array<IEntity> m_aBodies = {};
	ref map<int, ref RZ_PostContact> m_mContacts = new map<int, ref RZ_PostContact>();

	//------------------------------------------------------------------------------------------------
	string GetTitle()
	{
		if (m_eFaction == RZ_EFaction.BANDITS)
			return "Red Hand roadblock";

		return "Bastion checkpoint";
	}
}

//------------------------------------------------------------------------------------------------
class RZ_Checkpoints
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_CheckpointSettings m_Settings;
	protected float m_fClock;
	protected float m_fPostTimer;
	protected ref array<ref RZ_RoadPost> m_aPosts = {};
	protected ref array<string> m_aContraband = {};
	protected ref map<string, float> m_mSurrenderAt = new map<string, float>();	// identity -> clock
	protected ref map<string, float> m_mToldAt = new map<string, float>();			// "player:place:kind" -> clock
	protected bool m_bToldNoRoads;

	protected static const float TELL_GAP = 600;
	protected static const float GATE_GAP = 180;
	protected static const float SHOT_GAP = 20;
	protected static const float DEFEND_RADIUS = 40;
	protected static const float HANDS_UP_SECONDS = 15;	// a Hunted player walking up unarmed: they hold fire this long, renewed each tick

	//------------------------------------------------------------------------------------------------
	void RZ_Checkpoints(RZ_GameModeComponent gm, RZ_CheckpointSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		RZ_ItemHelper.Words(settings.m_sContrabandWords, m_aContraband);
	}

	//------------------------------------------------------------------------------------------------
	//! At most one message of a kind per player and place every `gap` seconds.
	protected bool Tell(int playerId, string place, string kind, float gap)
	{
		string key = string.Format("%1:%2:%3", playerId, place, kind);
		float last;
		if (m_mToldAt.Find(key, last) && m_fClock - last < gap)
			return false;

		m_mToldAt.Set(key, m_fClock);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		m_fClock += dt;
		if (m_Settings.m_bRoadPosts)
		{
			UpdatePosts(dt, ids, players);
			m_fPostTimer += dt;
			if (m_fPostTimer >= Math.Max(1, m_Settings.m_fPostMinutes) * 60)
			{
				m_fPostTimer = 0;
				TryNewPost(ids, players);
			}
		}

		if (m_Settings.m_bCamps || m_Settings.m_bSafeZones)
			UpdateCamps(ids, players);
	}

	//------------------------------------------------------------------------------------------------
	//! A new life: the posts and camps forget what they said to this player.
	void OnPlayerSpawned(int playerId)
	{
		foreach (RZ_RoadPost post : m_aPosts)
		{
			post.m_mContacts.Remove(playerId);
		}
	}

	// ================================================================ Road posts
	//------------------------------------------------------------------------------------------------
	protected ResourceName CrewPrefab(RZ_EFaction f)
	{
		ResourceName prefab = m_Settings.m_sBastionCrewPrefab;
		if (f == RZ_EFaction.BANDITS)
			prefab = m_Settings.m_sRedHandCrewPrefab;

		if (!prefab.IsEmpty())
			return prefab;

		RZ_Territory territory = m_GM.GetTerritory();
		if (territory)
			prefab = territory.GuardPrefab(f);

		if (prefab.IsEmpty() && f == RZ_EFaction.BANDITS)
			prefab = m_GM.GetRedHandGroupPrefab();

		return prefab;
	}

	//------------------------------------------------------------------------------------------------
	//! Picks one player at random and tries to put a post on a road ahead of them, between them
	//! and a Bastion or Red Hand holding. At most one new post a round.
	protected void TryNewPost(array<int> ids, array<IEntity> players)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		// Season rules: one or two more posts (lawless roads, strong checkpoints), six at most
		RZ_SeasonMods season = RZ_Seasons.Mods();
		int maxPosts = Math.Min(6, m_Settings.m_iMaxPosts + season.m_iRoadPostBonus);
		if (players.IsEmpty() || m_aPosts.Count() >= maxPosts || !territory || !territory.IsActive())
			return;

		IEntity p = players.GetRandomElement();
		if (!p)
			return;

		vector at = p.GetOrigin();
		// Who tries first: half and half, or the season's share; under a curfew Bastion after dark
		float bastionFirst = season.m_fBastionPostShare;
		RZ_Environment env = m_GM.GetEnvironment();
		if (season.m_bCurfew && env && env.IsNight())
			bastionFirst = 1;

		array<int> order = {RZ_EFaction.MILITARY, RZ_EFaction.BANDITS};
		if (Math.RandomFloat01() >= bastionFirst)
		{
			order.Clear();
			order.Insert(RZ_EFaction.BANDITS);
			order.Insert(RZ_EFaction.MILITARY);
		}

		foreach (int f : order)
		{
			if (CrewPrefab(f).IsEmpty())
				continue;

			RZ_Holding home = NearestHolding(territory, f, at);
			if (!home || RZ_Util.DistXZ(home.m_vPos, at) > m_Settings.m_fHoldingRange + 1000)
				continue;

			vector spot;
			if (!PickRoadSpot(at, home.m_vPos, spot))
				continue;

			RZ_RoadPost post = new RZ_RoadPost();
			post.m_eFaction = f;
			post.m_vPos = spot;
			post.m_fYaw = RZ_Compass.Bearing(spot, at);
			post.m_fUntil = m_fClock + Math.Max(5, m_Settings.m_fPostLifetimeMinutes) * 60;
			RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(spot));
			if (s)
				post.m_sNear = s.m_sName;

			m_aPosts.Insert(post);
			RZ_Log.Info(string.Format("%1 up near %2.", post.GetTitle(), post.m_sNear));
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Holding NearestHolding(RZ_Territory territory, RZ_EFaction f, vector pos)
	{
		RZ_Holding best;
		float bestDist = float.MAX;
		foreach (RZ_Holding h : territory.GetHoldings())
		{
			if (h.m_eFaction != f)
				continue;

			float d = RZ_Util.DistXZ(h.m_vPos, pos);
			if (d < bestDist)
			{
				bestDist = d;
				best = h;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! A road point 350-1000 m from the player, within range of the holding, away from the line,
	//! safehouses, holdings and other posts.
	protected bool PickRoadSpot(vector playerPos, vector holdingPos, out vector spot)
	{
		array<vector> points = {};
		RZ_Perimeter.RoadPointsNear(playerPos, 1000, points);
		if (points.IsEmpty())
		{
			if (!m_bToldNoRoads)
			{
				m_bToldNoRoads = true;
				RZ_Log.Info("Checkpoints: no road points found near players; road posts stay down.");
			}

			return false;
		}

		RZ_Safehouses houses = m_GM.GetSafehouses();
		RZ_Territory territory = m_GM.GetTerritory();
		array<vector> options = {};
		foreach (vector pt : points)
		{
			float d = RZ_Util.DistXZ(pt, playerPos);
			if (d < 350 || RZ_Util.DistXZ(pt, holdingPos) > m_Settings.m_fHoldingRange)
				continue;

			if (RZ_Perimeter.IsNearLineAt(pt))
				continue;

			if (houses && houses.FindNearest(pt, 300) >= 0)
				continue;

			if (territory && territory.FindHolding(-1, pt, 250))
				continue;

			if (IsPostNear(pt, 800))
				continue;

			options.Insert(pt);
		}

		if (options.IsEmpty())
			return false;

		spot = RZ_Util.GroundPos(options.GetRandomElement());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsPostNear(vector pos, float dist)
	{
		foreach (RZ_RoadPost post : m_aPosts)
		{
			if (RZ_Util.DistXZ(post.m_vPos, pos) < dist)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdatePosts(float dt, array<int> ids, array<IEntity> players)
	{
		for (int i = m_aPosts.Count() - 1; i >= 0; i--)
		{
			RZ_RoadPost post = m_aPosts[i];
			float nearest = RZ_Util.NearestDistance(post.m_vPos, players);
			if (m_fClock >= post.m_fUntil && nearest > m_Settings.m_fSpawnRadius)
			{
				Despawn(post);
				m_aPosts.Remove(i);
				continue;
			}

			if (post.m_bSpawned && nearest > m_Settings.m_fSpawnRadius + 100)
				Despawn(post);
			else if (!post.m_bSpawned && nearest <= m_Settings.m_fSpawnRadius)
				Spawn(post);

			if (!post.m_bSpawned)
				continue;

			foreach (int j, IEntity p : players)
			{
				float d = RZ_Util.DistXZ(p.GetOrigin(), post.m_vPos);
				if (d > m_Settings.m_fWarnRadius * 2)
				{
					// Gone well past or away: a fresh start next time, except a paid or cleared pass
					RZ_PostContact old;
					if (post.m_mContacts.Find(ids[j], old) && !old.m_bCleared)
						post.m_mContacts.Remove(ids[j]);

					continue;
				}

				PostContact(post, ids[j], p, d, dt);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Spawn(RZ_RoadPost post)
	{
		post.m_bSpawned = true;
		ResourceName barrier = m_Settings.m_sBastionBarrierPrefab;
		if (post.m_eFaction == RZ_EFaction.BANDITS)
			barrier = m_Settings.m_sRedHandBarrierPrefab;

		IEntity prop = RZ_Util.SpawnPrefab(barrier, post.m_vPos, post.m_fYaw);
		if (prop)
			post.m_aBodies.Insert(prop);

		IEntity crew = RZ_Util.SpawnPrefab(CrewPrefab(post.m_eFaction), RZ_Mission.LandPointNear(post.m_vPos, 4, 10), post.m_fYaw);
		if (!crew)
			return;

		post.m_aBodies.Insert(crew);
		RZ_Hostility hostility = m_GM.GetHostility();
		if (hostility)
			hostility.WatchUnit(crew, post.m_eFaction);
	}

	//------------------------------------------------------------------------------------------------
	protected void Despawn(RZ_RoadPost post)
	{
		foreach (IEntity e : post.m_aBodies)
		{
			RZ_Util.DeleteUnit(e);
		}

		post.m_aBodies.Clear();
		post.m_bSpawned = false;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_PostContact GetContact(RZ_RoadPost post, int playerId)
	{
		RZ_PostContact c;
		if (!post.m_mContacts.Find(playerId, c))
		{
			c = new RZ_PostContact();
			post.m_mContacts.Set(playerId, c);
		}

		return c;
	}

	//------------------------------------------------------------------------------------------------
	protected void PostContact(RZ_RoadPost post, int playerId, IEntity ent, float dist, float dt)
	{
		RZ_PostContact c = GetContact(post, playerId);
		if (c.m_bCleared || c.m_bPunished)
			return;

		if (post.m_eFaction == RZ_EFaction.BANDITS)
			RedHandContact(post, c, playerId, dist);
		else
			BastionContact(post, c, playerId, ent, dist, dt);
	}

	//------------------------------------------------------------------------------------------------
	protected void BastionContact(RZ_RoadPost post, RZ_PostContact c, int playerId, IEntity ent, float dist, float dt)
	{
		string title = post.GetTitle();
		RZ_ETier tier = m_GM.GetTier(playerId, RZ_EFaction.MILITARY);
		bool hunted = RZ_Reputation.IsHunted(tier);
		bool armed = RZ_Util.IsArmed(ent);
		bool canSurrender = m_Settings.m_bSurrender && m_GM.GetProfile(playerId).GetInfamy(RZ_EFaction.MILITARY) >= 15 && !armed;

		// Known perk: waved through unsearched
		if (!hunted && RZ_Reputation.IsAtLeast(tier, RZ_ETier.KNOWN))
		{
			if (dist <= m_Settings.m_fWarnRadius)
			{
				c.m_bCleared = true;
				m_GM.NotifyPlayer(playerId, title, "The sentry knows your face. A nod, the pole goes up, and you're through without a search.");
			}

			return;
		}

		if (!c.m_bWarned && dist <= m_Settings.m_fWarnRadius)
		{
			c.m_bWarned = true;
			if (hunted)
				m_GM.NotifyPlayer(playerId, title, "\"We know who you are. Drop your weapons and walk to the barrier, slowly, if you want to talk. Otherwise turn around.\"");
			else
				m_GM.NotifyPlayer(playerId, title, "A loudhailer: \"Checkpoint. Walk to the barrier and wait to be searched. Leave once you're there and we'll treat it as running.\"");
		}

		// Hunted but coming in with empty hands: they hold their fire while you walk up and wait
		RZ_Hostility hostility = m_GM.GetHostility();
		if (hunted && canSurrender && hostility && dist <= m_Settings.m_fWarnRadius)
			hostility.StandDown(playerId, RZ_EFaction.MILITARY, HANDS_UP_SECONDS, false);

		if (dist <= m_Settings.m_fStopRadius)
		{
			c.m_bAtBarrier = true;
			if (hunted && !canSurrender)
				return; // they shoot (RZ_Hostility); nothing to wait for

			c.m_fWaited = c.m_fWaited + dt;
			float needed = m_Settings.m_fSearchSeconds;
			if (canSurrender)
				needed = m_Settings.m_fSurrenderSeconds;

			if (c.m_fWaited < needed)
				return;

			c.m_bCleared = true;
			if (canSurrender && Surrender(playerId, ent, post))
				return;

			int taken = RZ_FoodHelper.TakeFood(ent, m_aContraband, 3);
			if (taken > 0)
				m_GM.NotifyPlayer(playerId, title, string.Format("Hands on the barrier. They go through everything. %1 things go into a crate marked CONFISCATED, and you're waved on.", taken));
			else
				m_GM.NotifyPlayer(playerId, title, "Hands on the barrier. They go through everything, find nothing they care about, and wave you on.");

			return;
		}

		// Reached the barrier, then left before it was done: running the checkpoint
		if (c.m_bAtBarrier && !hunted && dist > m_Settings.m_fStopRadius + 25)
		{
			c.m_bPunished = true;
			m_GM.AddInfamy(playerId, RZ_EFaction.MILITARY, m_Settings.m_iRunInfamy);
			if (hostility)
				hostility.AddGrudge(playerId, RZ_EFaction.MILITARY, post.m_vPos);

			m_GM.NotifyPlayer(playerId, title, "\"Stop! STOP!\" You ran the checkpoint. They'll remember the face.");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Unarmed, with Bastion Infamy, waiting at the barrier: they take you in, search you, and let you
	//! go with a talking-to. False (and an ordinary search) if it's too soon since the last time.
	protected bool Surrender(int playerId, IEntity ent, RZ_RoadPost post)
	{
		string title = post.GetTitle();
		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		float last;
		float gap = m_Settings.m_fSurrenderCooldownMinutes * 60;
		if (m_mSurrenderAt.Find(ident, last) && m_fClock - last < gap)
		{
			m_GM.NotifyPlayer(playerId, title, "\"You again. We've had this talk.\" A search, and nothing more.");
			return false;
		}

		m_mSurrenderAt.Set(ident, m_fClock);
		int infamy = m_GM.GetProfile(playerId).GetInfamy(RZ_EFaction.MILITARY);
		int amount = Math.Min(m_Settings.m_iSurrenderInfamy, infamy);
		RZ_FoodHelper.TakeFood(ent, m_aContraband, 3);
		m_GM.AddInfamy(playerId, RZ_EFaction.MILITARY, -amount);
		RZ_Hostility hostility = m_GM.GetHostility();
		if (hostility)
			hostility.StandDown(playerId, RZ_EFaction.MILITARY, m_Settings.m_fStandDownSeconds, true);

		m_GM.NotifyPlayer(playerId, title, string.Format("You go down on your knees with your hands where they can see them. A search, an hour's talk about the rules, a name in a book. Then they let you go. Bastion Infamy -%1. They'll hold their fire for %2 minutes: use them.",
			amount, Math.Max(1, RZ_Util.RoundToInt(m_Settings.m_fStandDownSeconds / 60))));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void RedHandContact(RZ_RoadPost post, RZ_PostContact c, int playerId, float dist)
	{
		string title = post.GetTitle();
		RZ_ETier tier = m_GM.GetTier(playerId, RZ_EFaction.BANDITS);
		if (RZ_Reputation.IsHunted(tier))
			return; // they shoot (RZ_Hostility)

		if (RZ_Reputation.IsAtLeast(tier, RZ_ETier.TRUSTED))
		{
			if (dist <= m_Settings.m_fWarnRadius)
			{
				c.m_bCleared = true;
				m_GM.NotifyPlayer(playerId, title, "One of them lifts a hand. You're one of theirs. The car they use as a gate rolls back.");
			}

			return;
		}

		bool known = RZ_Reputation.IsAtLeast(tier, RZ_ETier.KNOWN);
		int toll = Math.Max(0, m_Settings.m_iToll);
		if (!c.m_bWarned && dist <= m_Settings.m_fWarnRadius)
		{
			c.m_bWarned = true;
			if (known)
				m_GM.NotifyPlayer(playerId, title, string.Format("\"Toll road. %1 Favor and you're a friend. Walk up slow.\"", toll));
			else
				m_GM.NotifyPlayer(playerId, title, "\"This road's ours. Turn around. Come any closer and we start shooting.\"");
		}

		if (dist > m_Settings.m_fStopRadius)
			return;

		// Known perk: the toll instead of a bullet
		if (known && m_GM.SpendFavor(playerId, RZ_EFaction.BANDITS, toll))
		{
			c.m_bCleared = true;
			m_GM.NotifyPlayer(playerId, title, string.Format("You pay the toll: %1 Favor. A grin with too few teeth. \"Safe travels.\" %2 Favor left.", toll, m_GM.GetProfile(playerId).GetFavor(RZ_EFaction.BANDITS)));
			return;
		}

		c.m_bPunished = true;
		RZ_Hostility hostility = m_GM.GetHostility();
		if (hostility)
			hostility.AddGrudge(playerId, RZ_EFaction.BANDITS, post.m_vPos);

		if (known)
			m_GM.NotifyPlayer(playerId, title, "\"No Favor? Then you're no friend.\" The rifles come up.");
		else
			m_GM.NotifyPlayer(playerId, title, "\"You were told.\" The rifles come up.");
	}

	// ================================================================ Camps and safe zones
	//------------------------------------------------------------------------------------------------
	protected void UpdateCamps(array<int> ids, array<IEntity> players)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			foreach (RZ_Holding h : territory.GetHoldings())
			{
				foreach (int i, IEntity p : players)
				{
					float d = RZ_Util.DistXZ(p.GetOrigin(), h.m_vPos);
					if (d <= m_Settings.m_fCampWarnRadius)
						CampContact(ids[i], p, d, h.m_eFaction, h.m_bStronghold, "h" + h.m_iId.ToString(), h.GetTitle(), h.m_vPos);
				}
			}
		}

		if (!m_GM.HasLab())
			return;

		vector lab = m_GM.GetLabPosition();
		foreach (int j, IEntity q : players)
		{
			float ld = RZ_Util.DistXZ(q.GetOrigin(), lab);
			if (ld <= m_Settings.m_fCampWarnRadius)
				CampContact(ids[j], q, ld, RZ_EFaction.INSTITUTE, true, "lab", "Halcyon lab", lab);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A player near a camp. Hunted: they shoot (RZ_Hostility). Watched: warned off and turned away;
	//! armed past the gate after the warning, fired on for a few minutes. Welcome: the safe-zone rule.
	protected void CampContact(int playerId, IEntity ent, float dist, RZ_EFaction f, bool safeZone, string place, string title, vector pos)
	{
		RZ_ETier tier = m_GM.GetTier(playerId, f);
		if (RZ_Reputation.IsHunted(tier))
			return;

		// The Haven's own holdings follow the house rules instead (RZ_Safehouses)
		if (m_Settings.m_bCamps && f != RZ_EFaction.SURVIVORS && RZ_Reputation.IsWatchedOrWorse(tier))
		{
			if (Tell(playerId, place, "warn", TELL_GAP))
				m_GM.NotifyPlayer(playerId, title, RZ_FactionVoice.CampWarning(f));

			if (dist > m_Settings.m_fCampRefuseRadius)
				return;

			if (!RZ_Util.IsArmed(ent))
			{
				if (Tell(playerId, place, "gate", GATE_GAP))
					m_GM.NotifyPlayer(playerId, title, "The gate stays shut. Nobody here will deal with you until you've made it right with them.");

				return;
			}

			if (!Tell(playerId, place, "fire", GATE_GAP))
				return;

			RZ_Hostility hostility = m_GM.GetHostility();
			if (hostility)
				hostility.AddGrudge(playerId, f, pos);

			m_GM.NotifyPlayer(playerId, title, "\"We told you once.\" Armed and still coming: they open fire.");
			return;
		}

		if (m_Settings.m_bSafeZones && safeZone && f != RZ_EFaction.SURVIVORS && dist <= m_Settings.m_fSafeZoneRadius && Tell(playerId, place, "rule", TELL_GAP * 1.5))
			m_GM.NotifyPlayer(playerId, title, RZ_FactionVoice.SafeZoneRule(f));
	}

	//------------------------------------------------------------------------------------------------
	//! From the gunfire hook: firing inside a stronghold's or the lab's safe zone, with no infected
	//! close, costs a little Infamy with the faction and draws its fire for a few minutes.
	void OnPlayerShot(int playerId, vector pos)
	{
		if (!m_Settings.m_bSafeZones)
			return;

		int f = -1;
		vector center;
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			foreach (RZ_Holding h : territory.GetHoldings())
			{
				if (h.m_bStronghold && h.m_eFaction != RZ_EFaction.SURVIVORS && RZ_Util.DistXZ(h.m_vPos, pos) <= m_Settings.m_fSafeZoneRadius)
				{
					f = h.m_eFaction;
					center = h.m_vPos;
				}
			}
		}

		if (f < 0 && m_GM.HasLab() && RZ_Util.DistXZ(m_GM.GetLabPosition(), pos) <= m_Settings.m_fSafeZoneRadius)
		{
			f = RZ_EFaction.INSTITUTE;
			center = m_GM.GetLabPosition();
		}

		if (f < 0 || RZ_Reputation.IsHunted(m_GM.GetTier(playerId, f)))
			return;

		// Defending the place is fine
		if (m_GM.GetSpawner().IsZombieNear(pos, DEFEND_RADIUS) || m_GM.GetSpawner().IsZombieNear(center, DEFEND_RADIUS))
			return;

		if (!Tell(playerId, "zone" + f.ToString(), "shot", SHOT_GAP))
			return;

		m_GM.AddInfamy(playerId, f, m_Settings.m_iSafeZoneShotInfamy);
		RZ_Hostility hostility = m_GM.GetHostility();
		if (hostility)
			hostility.AddGrudge(playerId, f, center);

		m_GM.NotifyPlayer(playerId, RZ_Factions.GetName(f), string.Format("Shots inside the wire. Every rifle in the place turns your way. +%1 Infamy with %2.", m_Settings.m_iSafeZoneShotInfamy, RZ_Factions.GetNameInText(f)));
	}
}
