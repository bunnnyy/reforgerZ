//! Infection adapts, a little (docs/WORLD.md, section 4). Small, capped nudges that keep
//! the late game interesting without making it a wall:
//!
//! - Specials: each cure breakthrough makes them slightly more common (+5% of the stage's
//!   chance per breakthrough, +15% at most). A relative bump, never a new floor.
//! - One trait per outbreak stage, never more: Collapse swaps a share of common spawns for
//!   runners, Overrun adds helmeted ones, Aftermath makes some specials screamers that raise
//!   heat when they see a player. Each trait needs its prefab list; empty = no trait.
//! - Favourite spots: an area players keep coming back to gets a small group of infected
//!   checking it out, at most once an hour per area. A nudge, not a punishment.
//!
//! Everything here is data on the server tick; bodies come from the normal spawner.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Zombies > Adaptation.
[BaseContainerProps()]
class RZ_AdaptationSettings
{
	[Attribute("1", desc: "Infection adapts a little: more specials after breakthroughs, one trait per stage, favourite spots (docs/WORLD.md, section 4)")]
	bool m_bEnabled = true;

	[Attribute("0.05", desc: "Specials: extra share of the stage's special chance per cure breakthrough (0.05 = +5%)")]
	float m_fSpecialPerBreakthrough = 0.05;

	[Attribute("0.15", desc: "Specials: the most that breakthroughs add (0.15 = +15%)")]
	float m_fSpecialCap = 0.15;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Collapse trait: faster runners (zombie group or character prefabs). Empty = no runner trait", "et")]
	ref array<ResourceName> m_aRunnerPrefabs = {};

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Overrun trait: helmeted infected. Empty = no helmet trait", "et")]
	ref array<ResourceName> m_aHelmetPrefabs = {};

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Aftermath trait: screamers (a special prefab). Empty = some ordinary specials act as screamers", "et")]
	ref array<ResourceName> m_aScreamerPrefabs = {};

	[Attribute("0.15", desc: "Share of common spawns that get a stage trait (runner, helmet) once the stage has one")]
	float m_fTraitShare = 0.15;

	[Attribute("0.1", desc: "Aftermath: share of specials that are screamers")]
	float m_fScreamerShare = 0.1;

	[Attribute("40", desc: "A screamer 'sees' a player this close (m)")]
	float m_fScreamerSight = 40;

	[Attribute("20", desc: "Heat a screamer adds where it sees a player (at most once a minute per screamer)")]
	float m_fScreamerHeat = 20;

	[Attribute("1", desc: "Favourite spots: an area players keep returning to gets a small horde checking it out now and then")]
	bool m_bFavouriteSpots = true;

	[Attribute("500", desc: "Favourite spots: area size (m, one grid square)")]
	float m_fSpotCell = 500;

	[Attribute("60", desc: "Favourite spots: player-minutes in one area (fading by a fifth every 10 min) before it counts")]
	float m_fSpotMinutes = 60;

	[Attribute("60", desc: "Favourite spots: real minutes between two checks of the same area (at least)")]
	float m_fSpotCooldownMinutes = 60;

	[Attribute("3", UIWidgets.Slider, "Favourite spots: infected sent to look (commons)", "1 6 1")]
	int m_iSpotHordeSize = 3;
}

//------------------------------------------------------------------------------------------------
class RZ_Adaptation
{
	protected static const int GRID_STRIDE = 100000;
	protected static const float SPOT_FADE_INTERVAL = 600;
	protected static const float SPOT_FADE = 0.8;
	protected static const float SHRIEK_COOLDOWN = 60;
	protected static const float SHRIEK_HEAR = 150;
	protected static const float SPOT_SAFE_RADIUS = 150;

	protected RZ_GameModeComponent m_GM;
	protected RZ_AdaptationSettings m_Settings;
	protected ref map<int, float> m_mSpotMinutes = new map<int, float>();
	protected ref map<int, float> m_mSpotChecked = new map<int, float>();
	protected float m_fClock;
	protected float m_fFadeTimer;

	//------------------------------------------------------------------------------------------------
	void RZ_Adaptation(RZ_GameModeComponent gm, RZ_AdaptationSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled() { return m_Settings.m_bEnabled; }

	// ================================================================ Specials
	//------------------------------------------------------------------------------------------------
	//! Multiplier on the special chance from cure breakthroughs: 1 + 0.05 per breakthrough, capped.
	float SpecialMult()
	{
		if (!m_Settings.m_bEnabled || !m_GM.GetCure())
			return 1;

		int breakthroughs = Math.Min(m_GM.GetCure().m_iMilestone, 4);
		float bonus = Math.Min(m_Settings.m_fSpecialCap, breakthroughs * m_Settings.m_fSpecialPerBreakthrough);
		return 1 + Math.Max(0, bonus);
	}

	// ================================================================ Stage traits
	//------------------------------------------------------------------------------------------------
	//! The traits this stage has unlocked (one per stage after the first): 0 none, 1 runners,
	//! 2 runners and helmets, 3 runners, helmets and screamers.
	int TraitCount()
	{
		if (!m_Settings.m_bEnabled || !m_GM.GetDirector())
			return 0;

		return m_GM.GetDirector().GetStage();
	}

	//------------------------------------------------------------------------------------------------
	//! A common spawn: a trait prefab (runner, helmet) for a share of them, else "" (normal).
	ResourceName PickTraitPrefab()
	{
		int traits = TraitCount();
		if (traits <= 0 || Math.RandomFloat01() >= m_Settings.m_fTraitShare)
			return "";

		array<ResourceName> pool = {};
		if (traits >= 1 && m_Settings.m_aRunnerPrefabs)
		{
			foreach (ResourceName runner : m_Settings.m_aRunnerPrefabs)
			{
				pool.Insert(runner);
			}
		}

		if (traits >= 2 && m_Settings.m_aHelmetPrefabs)
		{
			foreach (ResourceName helmet : m_Settings.m_aHelmetPrefabs)
			{
				pool.Insert(helmet);
			}
		}

		if (pool.IsEmpty())
			return "";

		return pool.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! Aftermath only: is this special a screamer?
	bool RollScreamer()
	{
		if (TraitCount() < 3)
			return false;

		return Math.RandomFloat01() < m_Settings.m_fScreamerShare;
	}

	//------------------------------------------------------------------------------------------------
	//! The screamer prefab, "" = use an ordinary special prefab (it still acts as a screamer).
	ResourceName ScreamerPrefab()
	{
		if (!m_Settings.m_aScreamerPrefabs || m_Settings.m_aScreamerPrefabs.IsEmpty())
			return "";

		return m_Settings.m_aScreamerPrefabs.GetRandomElement();
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_Settings.m_bEnabled)
			return;

		m_fClock += dt;
		Screamers(players);

		if (m_Settings.m_bFavouriteSpots)
			FavouriteSpots(dt, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	//! A screamer that sees a player shrieks: heat where the player stands, once a minute.
	protected void Screamers(array<IEntity> players)
	{
		array<RZ_TrackedZombie> screamers = {};
		m_GM.GetSpawner().GetScreamers(screamers);
		foreach (RZ_TrackedZombie z : screamers)
		{
			if (m_fClock < z.m_fShriekAt)
				continue;

			foreach (IEntity p : players)
			{
				if (RZ_Util.DistXZ(p.GetOrigin(), z.m_vLastPos) > m_Settings.m_fScreamerSight)
					continue;

				z.m_fShriekAt = m_fClock + SHRIEK_COOLDOWN;
				m_GM.GetHeat().AddHeat(p.GetOrigin(), m_Settings.m_fScreamerHeat);
				NotifyNear(z.m_vLastPos, "Screamer", "Something shrieks, close and high. Every one of them for a mile heard that.");
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void NotifyNear(vector pos, string title, string text)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= SHRIEK_HEAR)
				m_GM.NotifyPlayer(ids[i], title, text);
		}
	}

	// ================================================================ Favourite spots
	//------------------------------------------------------------------------------------------------
	protected int SpotKey(vector p)
	{
		float cell = Math.Max(100, m_Settings.m_fSpotCell);
		int gx = Math.Floor(p[0] / cell);
		int gz = Math.Floor(p[2] / cell);
		return gx * GRID_STRIDE + gz;
	}

	//------------------------------------------------------------------------------------------------
	//! Player-minutes per area, fading slowly. A busy area gets one small look per cooldown.
	protected void FavouriteSpots(float dt, array<int> ids, array<IEntity> players)
	{
		foreach (IEntity p : players)
		{
			int key = SpotKey(p.GetOrigin());
			float minutes;
			m_mSpotMinutes.Find(key, minutes);
			m_mSpotMinutes.Set(key, minutes + dt / 60);
		}

		m_fFadeTimer += dt;
		if (m_fFadeTimer >= SPOT_FADE_INTERVAL)
		{
			m_fFadeTimer = 0;
			FadeSpots();
		}

		foreach (int i, IEntity player : players)
		{
			vector at = player.GetOrigin();
			int spot = SpotKey(at);
			float busy;
			if (!m_mSpotMinutes.Find(spot, busy) || busy < m_Settings.m_fSpotMinutes)
				continue;

			float last;
			if (m_mSpotChecked.Find(spot, last) && m_fClock - last < m_Settings.m_fSpotCooldownMinutes * 60)
				continue;

			// Not at a safehouse (it has its own guards and rules), nor around a brand-new player
			if (IsSheltered(at))
				continue;

			m_mSpotChecked.Set(spot, m_fClock);
			m_mSpotMinutes.Set(spot, busy * 0.5);
			SendLookers(at);
			return; // one per tick at most
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsSheltered(vector at)
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses && houses.FindNearest(at, SPOT_SAFE_RADIUS) >= 0)
			return true;

		RZ_Guidance guidance = m_GM.GetGuidance();
		if (guidance && guidance.GetDensityMult(at, 200) < 1)
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void FadeSpots()
	{
		array<int> keys = {};
		foreach (int k, float v : m_mSpotMinutes)
		{
			keys.Insert(k);
		}

		foreach (int key : keys)
		{
			float faded = m_mSpotMinutes.Get(key) * SPOT_FADE;
			if (faded < 1)
				m_mSpotMinutes.Remove(key);
			else
				m_mSpotMinutes.Set(key, faded);
		}

		// Forget checks older than a few hours, so the map stays small
		array<int> old = {};
		foreach (int c, float at : m_mSpotChecked)
		{
			if (m_fClock - at > m_Settings.m_fSpotCooldownMinutes * 60 * 4)
				old.Insert(c);
		}

		foreach (int gone : old)
		{
			m_mSpotChecked.Remove(gone);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A few ordinary infected, out of sight, walking over to see what all the coming and going is.
	protected void SendLookers(vector target)
	{
		RZ_ZombieSpawner spawner = m_GM.GetSpawner();
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);

		vector from = RZ_Mission.LandPointNear(target, 250, 350);
		if (RZ_Util.IsWater(from) || RZ_Util.NearestDistance(from, players) < 150)
			return;

		for (int n = 0; n < m_Settings.m_iSpotHordeSize; n++)
		{
			RZ_TrackedZombie z = spawner.SpawnZombie(RZ_Util.RandomPointInRing(from, 0, 15), false, -1, -1);
			if (z)
				RZ_Util.SendGroupTo(z.m_Entity, target, spawner.GetMoveWaypointPrefab());
		}

		RZ_Log.Info("Adaptation: a few infected are checking out a busy spot at grid " + RZ_SettlementScanner.GridReference(target));
	}
}
