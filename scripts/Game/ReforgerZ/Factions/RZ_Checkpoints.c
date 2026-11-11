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
//!   up unarmed isn't shot while they come in (a short stand-down, renewed while they get closer
//!   or wait at the barrier: at most 90 s an approach, never while a Bastion squad after them
//!   is within 1 km).
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
	float m_fHandsUp;		// seconds a Hunted player has hung back while unarmed in range (the hold has a limit)
	float m_fLastDist = -1;	// distance at the last tick, to tell coming in from loitering
}

//------------------------------------------------------------------------------------------------
//! A Bastion checkpoint or a Red Hand roadblock. Data, plus bodies while a player is near.
class RZ_RoadPost
{
	int m_iId;
	RZ_EFaction m_eFaction;
	vector m_vPos;
	float m_fYaw;
	float m_fUntil;
	string m_sNear;
	bool m_bSpawned;
	ref array<IEntity> m_aBodies = {};
	ref map<int, ref RZ_PostContact> m_mContacts = new map<int, ref RZ_PostContact>();
	IEntity m_Speaker;		// Dialogue > Trees: someone to talk to at the barrier (weak; also in m_aBodies)

	//------------------------------------------------------------------------------------------------
	//! Who you talk to at the barrier.
	string SpeakerName()
	{
		if (m_eFaction == RZ_EFaction.BANDITS)
			return "The toll man";

		return "The corporal at the barrier";
	}

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
	static const int TAG_POST = -12;	// RZ_PuzzleObjectComponent / RZ_ClueComponent mission id: a post's speaker

	protected RZ_GameModeComponent m_GM;
	protected int m_iNextPostId = 1;
	protected RZ_CheckpointSettings m_Settings;
	protected float m_fClock;
	protected float m_fLastDt = 1;
	protected float m_fPostTimer;
	protected ref array<ref RZ_RoadPost> m_aPosts = {};
	protected ref array<string> m_aContraband = {};
	protected ref map<string, float> m_mSurrenderAt = new map<string, float>();	// identity -> clock
	protected ref map<string, float> m_mToldAt = new map<string, float>();			// "player:place:kind" -> clock
	protected bool m_bToldNoRoads;
	protected ref RZ_Parley m_Parley;
	protected ref map<string, float> m_mGatePass = new map<string, float>();		// "player:place" -> clock a gate look let them in until

	protected static const float TELL_GAP = 600;
	protected static const float GATE_GAP = 180;
	protected static const float SHOT_GAP = 20;
	protected static const float DEFEND_RADIUS = 40;
	protected static const float HANDS_UP_SECONDS = 15;	// a Hunted player walking up unarmed: they hold fire this long, renewed each tick
	protected static const float HANDS_UP_MAX = 30;		// ...until they've hung back (not coming closer) this long in one approach
	protected static const float HANDS_UP_SQUAD_RADIUS = 1000;	// ...and never while a Bastion squad after them is this close

	//------------------------------------------------------------------------------------------------
	void RZ_Checkpoints(RZ_GameModeComponent gm, RZ_CheckpointSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		m_Parley = new RZ_Parley(gm);
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
		m_fLastDt = dt;
		m_Parley.Forget(ids);
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

			// Bastion puts up posts as often as it's still out on the roads (world facts' Bastion
			// presence): every time at full strength, a third of the time in Aftermath (0.1).
			// A curfew's night posts always go up
			if (f == RZ_EFaction.MILITARY && bastionFirst < 1 && Math.RandomFloat01() > BastionPostChance())
				continue;

			RZ_Holding home = NearestHolding(territory, f, at);
			if (!home || RZ_Util.DistXZ(home.m_vPos, at) > m_Settings.m_fHoldingRange + 1000)
				continue;

			vector spot;
			if (!PickRoadSpot(at, home.m_vPos, spot))
				continue;

			RZ_RoadPost post = new RZ_RoadPost();
			post.m_iId = m_iNextPostId;
			m_iNextPostId++;
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
	//! The chance a Bastion road post goes up when it's Bastion's turn: 0.3 + 0.7 x Bastion presence
	//! (1 at Early Outbreak, 0.72 at Collapse, 0.48 at Overrun, 0.37 in Aftermath).
	protected float BastionPostChance()
	{
		float presence = Math.Clamp(RZ_WorldFacts.Get().BastionPresence(), 0, 1);
		return 0.3 + 0.7 * presence;
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

			UpdateSpeaker(post, nearest);
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

		// Camp life: a fire barrel and a nest beside the barrier, the crew leaning on it
		RZ_CampLife camp = m_GM.GetCampLife();
		if (!camp)
			return;

		array<IEntity> crewBodies = {crew};
		array<IEntity> none = {};
		camp.Enter(PostKey(post), post.GetTitle(), post.m_eFaction, RZ_ECampPlace.ROAD_POST, post.m_vPos, post.m_fYaw, "", crewBodies, none, none);
	}

	// ================================================================ The speaker at the barrier
	//------------------------------------------------------------------------------------------------
	//! Dialogue > Trees only: one of the crew stands at the barrier to talk to while a player is
	//! within 100 m (the body budget's speaker), and goes when nobody is. The crew stays an AI group.
	protected void UpdateSpeaker(RZ_RoadPost post, float nearest)
	{
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		bool wanted = trees && trees.IsOn() && nearest <= RZ_BodyBudget.SPEAKER_RANGE;
		if (post.m_Speaker && (!wanted || !RZ_Util.IsCharacterAlive(post.m_Speaker)))
		{
			if (!wanted)
			{
				post.m_aBodies.RemoveItem(post.m_Speaker);
				RZ_Util.DeleteUnit(post.m_Speaker);
			}

			post.m_Speaker = null;
			return;
		}

		if (post.m_Speaker || !wanted)
			return;

		RZ_Territory territory = m_GM.GetTerritory();
		RZ_Shared shared = m_GM.GetShared();
		if (!territory || !shared)
			return;

		vector at = RZ_Mission.LandPointNear(post.m_vPos, 2, 4);
		if (!shared.CanSpawnBody(RZ_BodyBudget.SPEAKER, at))
			return;

		IEntity e = RZ_Util.SpawnPrefab(territory.SpeakerPrefab(post.m_eFaction), at, post.m_fYaw);
		if (!e)
			return;

		shared.AddBody(RZ_BodyBudget.SPEAKER, e);
		RZ_PuzzleObjectComponent.Tag(e, TAG_POST, post.m_iId, "", ""); // VERIFY: the speaker prefab (territory's occupant, else the puzzle civilian) carries the puzzle components, so Talk shows on it
		RZ_ClueComponent.Tag(e, TAG_POST, post.m_iId, "Talk to the " + post.SpeakerName().Substring(4, post.SpeakerName().Length() - 4));
		post.m_Speaker = e;
		post.m_aBodies.Insert(e);
		RZ_Hostility hostility = m_GM.GetHostility();
		if (hostility)
			hostility.WatchUnit(e, post.m_eFaction);
	}

	//------------------------------------------------------------------------------------------------
	//! The road posts of `faction` (-1 any) standing now, nearest first within `radius` of pos.
	void PostsNear(int faction, vector pos, float radius, notnull array<RZ_RoadPost> result)
	{
		foreach (RZ_RoadPost post : m_aPosts)
		{
			if ((faction < 0 || post.m_eFaction == faction) && RZ_Util.DistXZ(post.m_vPos, pos) <= radius)
				result.Insert(post);
		}
	}

	//------------------------------------------------------------------------------------------------
	RZ_RoadPost FindPost(int id)
	{
		foreach (RZ_RoadPost post : m_aPosts)
		{
			if (post.m_iId == id)
				return post;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Talk to the speaker at the barrier: what this post does to you, in its voice. Its topics
	//! come from the conversation trees (dialogue/posts/).
	void HandleSpeakerTalk(int playerId, int id)
	{
		RZ_RoadPost post = FindPost(id);
		if (!post)
			return;

		string title = post.SpeakerName() + ", " + post.GetTitle();
		RZ_PostContact c = GetContact(post, playerId);
		RZ_ETier tier = m_GM.GetTier(playerId, post.m_eFaction);

		// In their kit and below Known, talking to the speaker is the claim: the look decides
		RZ_Disguise disguise = Disguise();
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!c.m_bCleared && disguise && me && disguise.InTheirKit(playerId, post.m_eFaction) && !RZ_Reputation.IsHunted(tier))
		{
			c.m_bCleared = true;
			if (disguise.Look(playerId, post.m_eFaction, 2, PostKey(post), "", 0))
				m_GM.Say(playerId, title, post.m_eFaction, "\"Right. Go on, then.\" You walk through as one of theirs.");
			else
				c.m_bPunished = true;

			return;
		}
		if (post.m_eFaction == RZ_EFaction.BANDITS)
		{
			if (c.m_bCleared)
				m_GM.Say(playerId, title, post.m_eFaction, "\"You've paid. Go on, before I think of a reason you haven't.\"");
			else if (RZ_Reputation.IsAtLeast(tier, RZ_ETier.TRUSTED))
				m_GM.Say(playerId, title, post.m_eFaction, "\"You're one of ours. Why are you talking to me? Drive.\"");
			else if (RZ_Reputation.IsAtLeast(tier, RZ_ETier.KNOWN))
				m_GM.Say(playerId, title, post.m_eFaction, string.Format("\"Red hand means stop. Stop means pay. %1 Favor, or walk round.\"", TollFor(playerId)));
			else
				m_GM.Say(playerId, title, post.m_eFaction, "\"You don't know us and we don't know you. That's a turn-round face.\"");

			return;
		}

		if (c.m_bCleared)
			m_GM.Say(playerId, title, post.m_eFaction, "\"You're done. Move along. Keep to the left of the road.\"");
		else if (RZ_Reputation.IsAtLeast(tier, RZ_ETier.KNOWN) && tier != RZ_ETier.WILDCARD)
			m_GM.Say(playerId, title, post.m_eFaction, "\"Cleared. You don't need to talk to me. Nobody needs to talk to me.\"");
		else
			m_GM.Say(playerId, title, post.m_eFaction, "\"Checkpoint. Hands on the barrier, a search, and you're through. Fifteen seconds if you don't argue.\"");
	}

	//------------------------------------------------------------------------------------------------
	//! The Red Hand toll for this player now: the setting, times the Wildcard's danger money; in
	//! Halcyon's colours double ("Beaks pay double."). With both, the higher of the two, never both.
	int TollFor(int playerId)
	{
		int base = Math.Max(0, m_Settings.m_iToll);
		int toll = base;
		RZ_ReputationSettings rs = RZ_ReputationSettings.Get();
		if (m_GM.GetTier(playerId, RZ_EFaction.BANDITS) == RZ_ETier.WILDCARD && rs.m_bWildcardPosts)
			toll = RZ_Util.RoundToInt(base * Math.Max(1, rs.m_fWildcardTollMult));

		RZ_Disguise disguise = Disguise();
		if (disguise && disguise.WearsColours(playerId, RZ_EFaction.INSTITUTE))
			toll = Math.Max(toll, base * 2);

		// The night rate: one more on a road one of the crews in the woods hunts at night
		if (!NightRate(playerId).IsEmpty())
			toll++;

		return toll;
	}

	//------------------------------------------------------------------------------------------------
	//! "Night rate. The Postmen are out." at night on a road a pack hunts at its hour, else "".
	protected string NightRate(int playerId)
	{
		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		RZ_Tribes tribes = m_GM.GetTribes();
		if (!ent || !tribes || !tribes.IsEnabled())
			return "";

		return tribes.GetPeople().NightRate(ent.GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	//! call:toll: pay the built toll at this roadblock now (Known or better, as built). The line said.
	string PayToll(int playerId, int id)
	{
		RZ_RoadPost post = FindPost(id);
		if (!post || post.m_eFaction != RZ_EFaction.BANDITS)
			return "";

		RZ_PostContact c = GetContact(post, playerId);
		if (c.m_bCleared)
			return "\"You've paid already. We're bandits, not thieves.\"";

		if (!RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.KNOWN))
			return "\"Toll's for people we know. You're not people we know.\"";

		int toll = TollFor(playerId);
		int had = m_GM.GetProfile(playerId).GetFavor(RZ_EFaction.BANDITS);
		if (!m_GM.SpendFavor(playerId, RZ_EFaction.BANDITS, toll))
			return string.Format("\"%1 Favor. You've got %2. Walk round.\"", toll, had);

		c.m_bCleared = true;
		if (had == toll)
			RZ_RepVoice.CrewEvent(m_GM, playerId, RZ_RepVoice.CREW_SKINT);

		return string.Format("You pay the toll: %1 Favor. \"Pleasure. Mind the potholes.\" %2 Favor left.", toll, m_GM.GetProfile(playerId).GetFavor(RZ_EFaction.BANDITS));
	}

	//------------------------------------------------------------------------------------------------
	//! call:post_wave: this post lets you through without its search or toll, once, while it stands.
	string WaveThrough(int playerId, int id)
	{
		RZ_RoadPost post = FindPost(id);
		if (!post)
			return "";

		GetContact(post, playerId).m_bCleared = true;
		if (post.m_eFaction == RZ_EFaction.BANDITS)
			return "The car they use as a gate rolls back.";

		return "The pole goes up.";
	}

	//------------------------------------------------------------------------------------------------
	protected static string PostKey(RZ_RoadPost post)
	{
		return string.Format("p%1_%2", RZ_Util.RoundToInt(post.m_vPos[0]), RZ_Util.RoundToInt(post.m_vPos[2]));
	}

	//------------------------------------------------------------------------------------------------
	protected void Despawn(RZ_RoadPost post)
	{
		RZ_CampLife camp = m_GM.GetCampLife();
		if (camp)
			camp.Leave(PostKey(post));

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
			RedHandContact(post, c, playerId, ent, dist, dt);
		else
			BastionContact(post, c, playerId, ent, dist, dt);
	}

	//------------------------------------------------------------------------------------------------
	protected void BastionContact(RZ_RoadPost post, RZ_PostContact c, int playerId, IEntity ent, float dist, float dt)
	{
		string title = post.GetTitle();
		// The owed get a hearing at the barrier instead (RZ_Parley)
		if (m_Parley.Contact(playerId, ent, RZ_EFaction.MILITARY, post.m_vPos, dist, dt, title, PostKey(post)))
			return;

		RZ_ETier tier = m_GM.GetTier(playerId, RZ_EFaction.MILITARY);
		bool hunted = RZ_Reputation.IsHunted(tier);
		bool armed = RZ_Util.IsArmed(ent);
		bool canSurrender = m_Settings.m_bSurrender && m_GM.GetProfile(playerId).GetInfamy(RZ_EFaction.MILITARY) >= 15 && !armed;
		RZ_Disguise disguise = Disguise();

		// Known perk: waved through unsearched. Not the Wildcard tier: they're searched like a
		// stranger, because they don't trust you (Reputation > Wildcard at posts). Not anyone in Red
		// Hand colours (the colours rule), and not anyone in a car being searched (the company you keep)
		RZ_ReputationSettings rs = RZ_ReputationSettings.Get();
		bool wildcard = tier == RZ_ETier.WILDCARD && rs.m_bWildcardPosts;
		bool redColours = disguise && disguise.WearsColours(playerId, RZ_EFaction.BANDITS);
		bool wholeCar = rs.m_bGroupRules && GroupSearched(post, playerId, ent);
		if (!hunted && !wildcard && !redColours && !wholeCar && RZ_Reputation.IsAtLeast(tier, RZ_ETier.KNOWN))
		{
			if (dist <= m_Settings.m_fWarnRadius)
			{
				c.m_bCleared = true;
				m_GM.Say(playerId, title, RZ_EFaction.MILITARY, "The sentry knows your face. A nod, the pole goes up, and you're through without a search.");
			}

			return;
		}

		// In Bastion's kit and below Known: a claim, judged by the look. Surrender comes first
		bool claim = disguise && !hunted && !canSurrender && disguise.InTheirKit(playerId, RZ_EFaction.MILITARY);
		if (!c.m_bWarned && dist <= m_Settings.m_fWarnRadius)
		{
			c.m_bWarned = true;
			if (hunted)
				m_GM.Say(playerId, title, RZ_EFaction.MILITARY, "\"We know who you are. Drop your weapons and walk to the barrier, slowly, if you want to talk. Otherwise turn around.\"");
			else if (claim)
				disguise.ShowComing(playerId, RZ_EFaction.MILITARY, 2, PostKey(post), "", title);
			else if (wholeCar && RZ_Reputation.IsAtLeast(tier, RZ_ETier.KNOWN))
				m_GM.Say(playerId, title, RZ_EFaction.MILITARY, "The sentry knows your face, then looks at who you came with. \"We search the whole car.\"");
			else if (redColours)
				m_GM.Say(playerId, title, RZ_EFaction.MILITARY, "The sentry looks at the red on you, then at you. \"Arms out. We search everyone in those colours.\"");
			else if (wildcard)
				m_GM.Say(playerId, title, RZ_EFaction.MILITARY, "The sentry knows your face, and doesn't like what he knows. \"Arms out. Walk to the barrier. We search you today.\"");
			else
				m_GM.Say(playerId, title, RZ_EFaction.MILITARY, "A loudhailer: \"Checkpoint. Walk to the barrier and wait to be searched. Leave once you're there and we'll treat it as running.\"");
		}

		// Hunted but coming in with empty hands: they hold their fire while you walk up and wait.
		// Only while you come in (closer each tick, or waiting at the barrier, which the surrender
		// wait bounds), until you've hung back HANDS_UP_MAX seconds in one approach, with your
		// surrender hour free, and not while a Bastion squad after you is near: a post is no place
		// to sit out a hunt. A slow walk in never uses the limit up
		RZ_Hostility hostility = m_GM.GetHostility();
		if (hunted && canSurrender && hostility && dist <= m_Settings.m_fWarnRadius)
		{
			bool closing = c.m_fLastDist < 0 || dist < c.m_fLastDist - 0.3 || dist <= m_Settings.m_fStopRadius;
			if (!closing)
				c.m_fHandsUp = c.m_fHandsUp + dt;

			if (closing && c.m_fHandsUp <= HANDS_UP_MAX && CanSurrenderNow(playerId) && !hostility.IsFightFor(playerId, RZ_EFaction.MILITARY, ent.GetOrigin(), HANDS_UP_SQUAD_RADIUS))
				hostility.StandDown(playerId, RZ_EFaction.MILITARY, HANDS_UP_SECONDS, false);
		}

		c.m_fLastDist = dist;

		if (dist <= m_Settings.m_fStopRadius)
		{
			c.m_bAtBarrier = true;
			if (hunted && !canSurrender)
				return; // they shoot (RZ_Hostility); nothing to wait for

			// The claim: walking up in full kit (Hostility > claim in full kit), or by talking to the
			// speaker (HandleSpeakerTalk). Pass: waved through as one of theirs. Caught: held and searched
			if (claim && Hostility().m_bClaimInFullKit && disguise.KitLevel(playerId, RZ_EFaction.MILITARY) >= 2)
			{
				c.m_bCleared = true;
				if (disguise.Look(playerId, RZ_EFaction.MILITARY, 2, PostKey(post), "", 0))
					m_GM.Say(playerId, title, RZ_EFaction.MILITARY, "\"Not on the board.\" A long look, then a nod. The pole goes up, and you walk through as one of theirs.");
				else
					c.m_bPunished = true;

				return;
			}

			c.m_fWaited = c.m_fWaited + dt;
			float needed = m_Settings.m_fSearchSeconds;
			if (canSurrender)
				needed = m_Settings.m_fSurrenderSeconds;

			if (c.m_fWaited < needed)
				return;

			c.m_bCleared = true;
			if (canSurrender && Surrender(playerId, ent, post))
				return;

			int taken = Search(ent);
			string evidence = TakeArmband(ent, playerId);
			if (taken >= 3)
				RZ_RepVoice.CrewEvent(m_GM, playerId, RZ_RepVoice.CREW_POCKETS);

			if (taken > 0)
				m_GM.Say(playerId, title, RZ_EFaction.MILITARY, string.Format("Hands on the barrier. They go through everything. %1 things go into a crate marked CONFISCATED, and you're waved on.%2", taken, evidence));
			else
				m_GM.Say(playerId, title, RZ_EFaction.MILITARY, "Hands on the barrier. They go through everything, find nothing they care about, and wave you on." + evidence);

			return;
		}

		// Reached the barrier, then left before it was done: running the checkpoint
		if (c.m_bAtBarrier && !hunted && dist > m_Settings.m_fStopRadius + 25)
		{
			c.m_bPunished = true;
			// Running a Wildcard's search proves them right: more Infamy
			int runInfamy = m_Settings.m_iRunInfamy;
			if (wildcard)
				runInfamy = rs.m_iWildcardRunInfamy;

			m_GM.AddInfamy(playerId, RZ_EFaction.MILITARY, runInfamy, "ran the checkpoint at " + post.m_sNear, RZ_RepFlag.ACT);
			RZ_RepVoice.NoteRan(m_GM, playerId);
			if (hostility)
				hostility.AddGrudge(playerId, RZ_EFaction.MILITARY, post.m_vPos);

			m_GM.Say(playerId, title, RZ_EFaction.MILITARY, "\"Stop! STOP!\" You ran the checkpoint. They'll remember the face.");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion's search at a barrier: up to three contraband items taken. How many.
	int Search(IEntity ent)
	{
		if (!ent)
			return 0;

		return RZ_FoodHelper.TakeFood(ent, m_aContraband, 3);
	}

	//------------------------------------------------------------------------------------------------
	//! A worn red armband or bandana is taken at a Bastion search ("Evidence."), never one in a pack.
	protected string TakeArmband(IEntity ent, int playerId)
	{
		RZ_Disguise disguise = Disguise();
		if (!disguise || !Hostility().m_bConfiscation || !disguise.WearsColours(playerId, RZ_EFaction.BANDITS))
			return "";

		array<string> words = {};
		RZ_ItemHelper.Words(Hostility().m_sRedHandUniformWords, words);
		array<IEntity> pieces = {};
		RZ_Util.GetWornMatching(ent, words, pieces);
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(ent.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv || pieces.IsEmpty())
			return "";

		foreach (IEntity piece : pieces)
		{
			inv.TryDeleteItem(piece);
		}

		return " The red comes off you too. \"Evidence.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! The company you keep: someone in this player's vanilla group within 50 m of them, at this post,
	//! would be searched (not Known, or Wildcard, or in Red Hand colours): then everyone there is.
	//! Applies before any look: a disguised member of a searched car gets the look at the search.
	protected bool GroupSearched(RZ_RoadPost post, int playerId, IEntity ent)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		RZ_Disguise disguise = Disguise();
		foreach (int i, IEntity p : players)
		{
			int other = ids[i];
			if (other == playerId || !RZ_Util.SameGroup(playerId, other) || RZ_Util.DistXZ(p.GetOrigin(), ent.GetOrigin()) > 50)
				continue;

			if (RZ_Util.DistXZ(p.GetOrigin(), post.m_vPos) > m_Settings.m_fWarnRadius)
				continue;

			RZ_ETier tier = m_GM.GetTier(other, RZ_EFaction.MILITARY);
			bool searched = !RZ_Reputation.IsAtLeast(tier, RZ_ETier.KNOWN) || tier == RZ_ETier.WILDCARD;
			if (disguise && disguise.WearsColours(other, RZ_EFaction.BANDITS))
				searched = true;

			if (searched)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Disguise Disguise()
	{
		RZ_Hostility hostility = m_GM.GetHostility();
		if (!hostility || !hostility.GetDisguise().IsOn())
			return null;

		return hostility.GetDisguise();
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_HostilitySettings Hostility()
	{
		return m_GM.GetHostility().GetSettings();
	}

	//------------------------------------------------------------------------------------------------
	RZ_Parley GetParley()
	{
		return m_Parley;
	}

	//------------------------------------------------------------------------------------------------
	//! The surrender hour: false while this player (by identity) surrendered too recently.
	protected bool CanSurrenderNow(int playerId)
	{
		float last;
		if (!m_mSurrenderAt.Find(m_GM.GetProfile(playerId).m_sIdentity, last))
			return true;

		return m_fClock - last >= m_Settings.m_fSurrenderCooldownMinutes * 60;
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
			m_GM.Say(playerId, title, RZ_EFaction.MILITARY, "\"You again. We've had this talk.\" A search, and nothing more.");
			return false;
		}

		m_mSurrenderAt.Set(ident, m_fClock);
		RZ_FoodHelper.TakeFood(ent, m_aContraband, 3);
		// From the one forgiveness budget: once it's spent, surrender is the search and the hold
		int amount = m_GM.Forgive(playerId, RZ_EFaction.MILITARY, m_Settings.m_iSurrenderInfamy, "surrender");
		RZ_Hostility hostility = m_GM.GetHostility();
		if (hostility)
			hostility.StandDown(playerId, RZ_EFaction.MILITARY, m_Settings.m_fStandDownSeconds, true);

		int minutes = Math.Max(1, RZ_Util.RoundToInt(m_Settings.m_fStandDownSeconds / 60));
		if (amount <= 0)
		{
			m_GM.NotifyPlayer(playerId, title, string.Format("You go down on your knees with your hands where they can see them. A search, a long look, a name in a book, and nothing crossed out. %1 They'll hold their fire for %2 minutes: use them.",
				m_GM.ForgiveSpentText(RZ_EFaction.MILITARY), minutes));
			return true;
		}

		m_GM.NotifyPlayer(playerId, title, string.Format("You go down on your knees with your hands where they can see them. A search, an hour's talk about the rules, a name in a book. Then they let you go. Bastion Infamy -%1. They'll hold their fire for %2 minutes: use them.",
			amount, minutes));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void RedHandContact(RZ_RoadPost post, RZ_PostContact c, int playerId, IEntity ent, float dist, float dt)
	{
		string title = post.GetTitle();
		if (m_Parley.Contact(playerId, ent, RZ_EFaction.BANDITS, post.m_vPos, dist, dt, title, PostKey(post)))
			return;

		RZ_ETier tier = m_GM.GetTier(playerId, RZ_EFaction.BANDITS);
		if (RZ_Reputation.IsHunted(tier))
			return; // they shoot (RZ_Hostility)

		// Colours: in Bastion's colours you're turned round whatever your tier, and fired on at the
		// barrier ("We don't sell passes to the army.")
		RZ_Disguise disguise = Disguise();
		bool armyColours = disguise && disguise.WearsColours(playerId, RZ_EFaction.MILITARY);
		bool claim = disguise && !armyColours && disguise.InTheirKit(playerId, RZ_EFaction.BANDITS);
		if (!armyColours && RZ_Reputation.IsAtLeast(tier, RZ_ETier.TRUSTED))
		{
			if (dist <= m_Settings.m_fWarnRadius)
			{
				c.m_bCleared = true;
				m_GM.Say(playerId, title, RZ_EFaction.BANDITS, "One of them lifts a hand. You're one of theirs. The car they use as a gate rolls back.");
			}

			return;
		}

		bool known = !armyColours && RZ_Reputation.IsAtLeast(tier, RZ_ETier.KNOWN);
		// The Wildcard tier pays danger money, and there's a night rate (TollFor)
		int toll = TollFor(playerId);
		if (!c.m_bWarned && dist <= m_Settings.m_fWarnRadius)
		{
			c.m_bWarned = true;
			if (armyColours)
				m_GM.Say(playerId, title, RZ_EFaction.BANDITS, "\"We don't sell passes to the army. Turn round, soldier.\"");
			else if (claim)
				disguise.ShowComing(playerId, RZ_EFaction.BANDITS, 2, PostKey(post), "", title);
			else if (known)
				m_GM.Say(playerId, title, RZ_EFaction.BANDITS, string.Format("\"Toll road. %1 Favor and you're a friend. Walk up slow.\" %2", toll, NightRate(playerId)).Trim());
			else
				m_GM.Say(playerId, title, RZ_EFaction.BANDITS, "\"This road's ours. Turn around. Come any closer and we start shooting.\"");
		}

		if (dist > m_Settings.m_fStopRadius)
			return;

		// Reached a roadblock the same day you ran a Bastion checkpoint: the crews call that Legs
		RZ_RepVoice.NoteRoadblock(m_GM, playerId);

		// The claim: walking up in full kit. Pass: waved through like crew. Caught: the toll for liars
		if (claim && Hostility().m_bClaimInFullKit && disguise.KitLevel(playerId, RZ_EFaction.BANDITS) >= 2)
		{
			c.m_bCleared = true;
			if (disguise.Look(playerId, RZ_EFaction.BANDITS, 2, PostKey(post), "", 0))
				m_GM.Say(playerId, title, RZ_EFaction.BANDITS, "\"What do they call you?\" You tell them. A grunt, and the car rolls back. You walk through like crew.");
			else
				c.m_bPunished = true;

			return;
		}

		// Known perk: the toll instead of a bullet
		int hadFavor = m_GM.GetProfile(playerId).GetFavor(RZ_EFaction.BANDITS);
		if (known && m_GM.SpendFavor(playerId, RZ_EFaction.BANDITS, toll))
		{
			c.m_bCleared = true;
			if (hadFavor == toll)
				RZ_RepVoice.CrewEvent(m_GM, playerId, RZ_RepVoice.CREW_SKINT);

			m_GM.NotifyPlayer(playerId, title, string.Format("You pay the toll: %1 Favor. A grin with too few teeth. \"Safe travels.\" %2 Favor left.", toll, m_GM.GetProfile(playerId).GetFavor(RZ_EFaction.BANDITS)));
			return;
		}

		c.m_bPunished = true;
		RZ_Hostility hostility = m_GM.GetHostility();
		if (hostility)
			hostility.AddGrudge(playerId, RZ_EFaction.BANDITS, post.m_vPos);

		if (armyColours)
			m_GM.Say(playerId, title, RZ_EFaction.BANDITS, "\"Told you. No passes for the army.\" The rifles come up.");
		else if (known)
			m_GM.Say(playerId, title, RZ_EFaction.BANDITS, "\"No Favor? Then you're no friend.\" The rifles come up.");
		else
			m_GM.Say(playerId, title, RZ_EFaction.BANDITS, "\"You were told.\" The rifles come up.");
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

		// Halcyon's stronghold seated at the lab was handled with the holdings above
		vector lab = m_GM.GetLabPosition();
		if (territory && territory.IsActive())
		{
			RZ_Holding halcyon = territory.GetStronghold(RZ_EFaction.INSTITUTE);
			if (halcyon && RZ_Util.DistXZ(halcyon.m_vPos, lab) <= m_Settings.m_fCampWarnRadius)
				return;
		}

		foreach (int j, IEntity q : players)
		{
			float ld = RZ_Util.DistXZ(q.GetOrigin(), lab);
			if (ld <= m_Settings.m_fCampWarnRadius)
				CampContact(ids[j], q, ld, RZ_EFaction.INSTITUTE, true, "lab", "Halcyon lab", lab);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A player near a camp. Hunted: they shoot (RZ_Hostility), unless they're owed a hearing at the
	//! gate (RZ_Parley). Watched, or in the Red Hand's colours at Bastion's or Halcyon's gate: warned
	//! off and turned away; armed past the gate after the warning, fired on for a few minutes. In the
	//! camp's own kit while Watched: a look at the gate, and a pass lets you in for a while (Hostility >
	//! Gate pass minutes); services still go by your real standing. Welcome: the safe-zone rule.
	protected void CampContact(int playerId, IEntity ent, float dist, RZ_EFaction f, bool safeZone, string place, string title, vector pos)
	{
		if (f != RZ_EFaction.SURVIVORS && m_Parley.Contact(playerId, ent, f, pos, dist, m_fLastDt, title, place))
			return;

		RZ_ETier tier = m_GM.GetTier(playerId, f);
		if (RZ_Reputation.IsHunted(tier))
			return;

		RZ_Disguise disguise = Disguise();
		bool refused = RZ_Reputation.IsWatchedOrWorse(tier);
		bool redColours = !refused && disguise && f != RZ_EFaction.BANDITS && disguise.WearsColours(playerId, RZ_EFaction.BANDITS);
		if (redColours)
			refused = true;

		// The Haven's own holdings follow the house rules instead (RZ_Safehouses)
		if (m_Settings.m_bCamps && f != RZ_EFaction.SURVIVORS && refused)
		{
			// The lab still gives the bitten their trial dose (RZ_Infection.TrialDose refuses only
			// the Hunted): an unarmed bitten player who hasn't had it is let in, not turned away,
			// whatever they wear. By position, not place: Halcyon's stronghold seated at the lab is a
			// holding here
			RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
			bool atLab = f == RZ_EFaction.INSTITUTE && m_GM.HasLab() && RZ_Util.DistXZ(pos, m_GM.GetLabPosition()) <= m_Settings.m_fCampWarnRadius;
			if (atLab && profile.IsInfected() && !profile.m_bTrialDoseTaken && !RZ_Util.IsArmed(ent))
			{
				if (Tell(playerId, place, "trial", TELL_GAP))
					m_GM.NotifyPlayer(playerId, title, "The guard looks at the bite before he looks at your face. \"Trial's inside. Hands where I can see them.\"");

				return;
			}

			// In their kit: a look at the gate instead of being turned away
			string passKey = playerId.ToString() + ":" + place;
			float until;
			if (m_mGatePass.Find(passKey, until) && m_fClock < until && disguise && disguise.IsPassing(playerId, f))
				return;

			if (!redColours && disguise && disguise.InTheirKit(playerId, f))
			{
				if (dist > m_Settings.m_fCampRefuseRadius)
				{
					disguise.ShowComing(playerId, f, 2, place, "", title);
					return;
				}

				if (!Tell(playerId, place, "look", GATE_GAP))
					return;

				float pass = Math.Max(1, Hostility().m_fGatePassMinutes) * 60;
				if (disguise.Look(playerId, f, 2, place, "", pass))
				{
					m_mGatePass.Set(passKey, m_fClock + pass);
					m_GM.Say(playerId, title, f, string.Format("The sentry looks you up and down, then waves you in. You've about %1 minutes before somebody looks again.", RZ_Util.RoundToInt(pass / 60)));
				}

				return;
			}

			if (Tell(playerId, place, "warn", TELL_GAP))
			{
				if (redColours)
					m_GM.Say(playerId, title, f, "\"Not in those colours. Take the red off before you come up here.\"");
				else
					m_GM.Say(playerId, title, f, RZ_FactionVoice.CampWarning(f));
			}

			if (dist > m_Settings.m_fCampRefuseRadius)
				return;

			if (!RZ_Util.IsArmed(ent))
			{
				if (Tell(playerId, place, "gate", GATE_GAP))
				{
					string gate = "The gate stays shut. Nobody here will deal with you until you've made it right with them.";
					if (FriendWelcome(playerId, ent, f))
						gate += " \"Your friend can come in. You wait outside.\"";

					m_GM.Say(playerId, title, f, gate);
				}

				return;
			}

			if (!Tell(playerId, place, "fire", GATE_GAP))
				return;

			RZ_Hostility hostility = m_GM.GetHostility();
			if (hostility)
				hostility.AddGrudge(playerId, f, pos);

			m_GM.Say(playerId, title, f, "\"We told you once.\" Armed and still coming: they open fire.");
			return;
		}

		if (m_Settings.m_bSafeZones && safeZone && f != RZ_EFaction.SURVIVORS && dist <= m_Settings.m_fSafeZoneRadius && Tell(playerId, place, "rule", TELL_GAP * 1.5))
			m_GM.Say(playerId, title, f, RZ_FactionVoice.SafeZoneRule(f));
	}

	//------------------------------------------------------------------------------------------------
	//! The company you keep at a camp: someone in this player's group, within 50 m, whom the camp lets in.
	protected bool FriendWelcome(int playerId, IEntity ent, int f)
	{
		if (!RZ_ReputationSettings.Get().m_bGroupRules)
			return false;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (ids[i] != playerId && RZ_Util.SameGroup(playerId, ids[i]) && RZ_Util.DistXZ(p.GetOrigin(), ent.GetOrigin()) <= 50 && !RZ_Reputation.IsWatchedOrWorse(m_GM.GetTier(ids[i], f)))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! From the gunfire hook: firing inside a stronghold's or the lab's safe zone, with no infected
	//! close, costs a little Infamy with the faction and draws its fire for a few minutes.
	void OnPlayerShot(int playerId, vector pos)
	{
		m_Parley.OnPlayerShot(playerId);
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

		// The cure finale's stand: fire between locked players at the lab is war, not a breach
		RZ_CureFinale finale = RZ_CureFinale.Get();
		if (f == RZ_EFaction.INSTITUTE && finale && finale.SafeZoneExempt(playerId, pos))
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
