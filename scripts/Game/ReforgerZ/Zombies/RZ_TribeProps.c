//! Signs and sounds (docs/INFECTED.md, "Signs and sounds"), phase 5 of the tribes design: the props at
//! kill sites, abandoned lairs and the factions' marks, the call sounds, and the bright moon. Every
//! prop and sound is an optional prefab under Zombies > Tribes; without one its line is all there is
//! (the lines are always told). Props appear only while a player is within 200 m (6 a pack, 30 on the
//! server), go once nobody is within 250 m, and are never saved. Owned by RZ_Tribes. Server only,
//! except RZ_CallSoundComponent, which plays on clients.

//------------------------------------------------------------------------------------------------
//! One prop in the world.
class RZ_TribeProp
{
	string m_sKey;
	IEntity m_Entity;
	int m_iTribe = -1;
}

//------------------------------------------------------------------------------------------------
class RZ_TribeProps
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_Tribes m_Tribes;			// weak: the owner
	protected ref array<ref RZ_TribeProp> m_aProps = {};
	protected ref array<IEntity> m_aSounds = {};
	protected ref array<float> m_aSoundUntil = {};
	protected float m_fTimer;
	protected int m_iShown;
	protected int m_iSounds;

	static const float SHOW = 200;
	static const float HIDE = 250;
	static const int PER_TRIBE = 6;
	static const int ON_SERVER = 30;
	static const float CHECK = 5;
	static const float SOUND_SECONDS = 40;
	static const float SOUND_REACH = 1500;
	static const int MAX_SOUNDS = 6;

	//------------------------------------------------------------------------------------------------
	void RZ_TribeProps(RZ_GameModeComponent gm, RZ_Tribes tribes)
	{
		m_GM = gm;
		m_Tribes = tribes;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_TribeSettings Settings()
	{
		return m_Tribes.GetSettings();
	}

	// ================================================================ Sounds
	//------------------------------------------------------------------------------------------------
	//! A call as a sound at pos, when a player is within 1.5 km: the call sound prefab (the great one's
	//! own when great and it's set). Removed after 40 s. Nothing without a prefab.
	void CallSound(vector pos, bool great)
	{
		RZ_TribeSettings s = Settings();
		if (!s)
			return;

		ResourceName prefab = s.m_sCallSoundPrefab;
		if (great && !s.m_sGreatCallSoundPrefab.IsEmpty())
			prefab = s.m_sGreatCallSoundPrefab;

		if (prefab.IsEmpty() || m_aSounds.Count() >= MAX_SOUNDS || !RZ_Tribes.PlayerWithin(pos, SOUND_REACH))
			return;

		// VERIFY: a spawned sound prefab replicates and plays on clients through RZ_CallSoundComponent
		IEntity e = RZ_Util.SpawnPrefab(prefab, RZ_Util.GroundPos(pos) + Vector(0, 2, 0));
		if (!e)
			return;

		m_aSounds.Insert(e);
		m_aSoundUntil.Insert(RZ_BaconBody.Now() + SOUND_SECONDS);
		m_iSounds++;
	}

	//------------------------------------------------------------------------------------------------
	protected void SoundsTick()
	{
		float now = RZ_BaconBody.Now();
		for (int i = m_aSounds.Count() - 1; i >= 0; i--)
		{
			if (m_aSounds[i] && now < m_aSoundUntil[i])
				continue;

			if (m_aSounds[i])
				SCR_EntityHelper.DeleteEntityAndChildren(m_aSounds[i]);

			m_aSounds.Remove(i);
			m_aSoundUntil.Remove(i);
		}
	}

	// ================================================================ Props
	//------------------------------------------------------------------------------------------------
	//! Every 5 s: the props near players come, the ones nobody is near go.
	void Tick(float dt, array<IEntity> players)
	{
		SoundsTick();
		m_fTimer += dt;
		if (m_fTimer < CHECK)
			return;

		m_fTimer = 0;
		array<string> keys = {};
		array<vector> spots = {};
		array<ResourceName> prefabs = {};
		array<int> tribes = {};
		if (!players.IsEmpty())
			Wanted(players, keys, spots, prefabs, tribes);

		// Gone: no longer wanted, or nobody within 250 m
		for (int i = m_aProps.Count() - 1; i >= 0; i--)
		{
			RZ_TribeProp prop = m_aProps[i];
			int at = keys.Find(prop.m_sKey);
			if (prop.m_Entity && at >= 0 && RZ_Util.NearestDistance(spots[at], players) <= HIDE)
				continue;

			if (prop.m_Entity)
				SCR_EntityHelper.DeleteEntityAndChildren(prop.m_Entity);

			m_aProps.Remove(i);
		}

		// Come: wanted, within 200 m, inside the caps
		foreach (int k, string key : keys)
		{
			if (m_aProps.Count() >= ON_SERVER)
				break;

			if (Has(key) || RZ_Util.NearestDistance(spots[k], players) > SHOW)
				continue;

			if (tribes[k] >= 0 && CountFor(tribes[k]) >= PER_TRIBE)
				continue;

			IEntity e = RZ_Util.SpawnPrefab(prefabs[k], RZ_Util.GroundPos(spots[k]), Yaw(spots[k]));
			if (!e)
				continue;

			RZ_TribeProp prop2 = new RZ_TribeProp();
			prop2.m_sKey = key;
			prop2.m_Entity = e;
			prop2.m_iTribe = tribes[k];
			m_aProps.Insert(prop2);
			m_iShown++;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! What should stand where, for spots within 1 km of a player: one thing at each kill site, at each
	//! abandoned lair, and at each faction's mark that's written.
	protected void Wanted(array<IEntity> players, array<string> keys, array<vector> spots, array<ResourceName> prefabs, array<int> tribes)
	{
		RZ_TribeSettings s = Settings();
		if (!s)
			return;

		// Kill sites: a bicycle, a rucksack, one shoe (picked by the spot, so it's the same each time)
		RZ_Crawlers crawlers = m_GM.GetSpawner().GetCrawlers();
		if (crawlers && s.m_aKillSitePrefabs && !s.m_aKillSitePrefabs.IsEmpty())
		{
			foreach (RZ_KillSite site : crawlers.GetKillSites())
			{
				if (RZ_Util.NearestDistance(site.m_vPos, players) > 1000)
					continue;

				int seed = Math.AbsInt(RZ_Util.RoundToInt(site.m_vPos[0] + site.m_vPos[2]));
				int pick = seed % s.m_aKillSitePrefabs.Count();
				Add(keys, spots, prefabs, tribes, "k" + SpotKey(site.m_vPos), site.m_vPos, s.m_aKillSitePrefabs[pick], -1);
			}
		}

		// Abandoned lairs
		if (!s.m_sOldLairPrefab.IsEmpty())
		{
			foreach (vector lair : m_Tribes.GetLife().GetAbandonedLairs())
			{
				if (RZ_Util.NearestDistance(lair, players) <= 1000)
					Add(keys, spots, prefabs, tribes, "l" + SpotKey(lair), lair, s.m_sOldLairPrefab, -1);
			}
		}

		// The factions' marks, a step apart where a road enters the range (rewritten: a new key)
		RZ_TribePeople people = m_Tribes.GetPeople();
		if (!people.IsOn())
			return;

		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (RZ_Util.NearestDistance(t.RangeCentre(), players) > t.m_fRadius + 1000)
				continue;

			vector spot;
			bool found = false;
			for (int f = 0; f < 4; f++)
			{
				ResourceName prefab = MarkPrefab(s, f);
				if (prefab.IsEmpty() || t.m_aMarkText[f].IsEmpty())
					continue;

				if (!found)
				{
					spot = people.MarkSpot(t);
					found = true;
				}

				vector at = spot + Vector((f - 1.5) * 1.5, 0, 0);
				Add(keys, spots, prefabs, tribes, string.Format("m%1:%2:%3", t.m_iId, f, t.m_aMarkDay[f]), at, prefab, t.m_iId);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static ResourceName MarkPrefab(RZ_TribeSettings s, int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return s.m_sHavenMarkPrefab;
			case RZ_EFaction.MILITARY: return s.m_sBastionMarkPrefab;
			case RZ_EFaction.BANDITS: return s.m_sRedHandMarkPrefab;
			case RZ_EFaction.INSTITUTE: return s.m_sHalcyonMarkPrefab;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected static void Add(array<string> keys, array<vector> spots, array<ResourceName> prefabs, array<int> tribes, string key, vector spot, ResourceName prefab, int tribe)
	{
		keys.Insert(key);
		spots.Insert(spot);
		prefabs.Insert(prefab);
		tribes.Insert(tribe);
	}

	//------------------------------------------------------------------------------------------------
	protected static string SpotKey(vector pos)
	{
		return string.Format("%1:%2", RZ_Util.RoundToInt(pos[0]), RZ_Util.RoundToInt(pos[2]));
	}

	//------------------------------------------------------------------------------------------------
	//! The same facing every time for a spot.
	protected static float Yaw(vector pos)
	{
		int seed = Math.AbsInt(RZ_Util.RoundToInt(pos[0] * 3 + pos[2]));
		int yaw = seed % 360;
		return yaw;
	}

	//------------------------------------------------------------------------------------------------
	protected bool Has(string key)
	{
		foreach (RZ_TribeProp p : m_aProps)
		{
			if (p.m_sKey == key)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected int CountFor(int tribe)
	{
		int n = 0;
		foreach (RZ_TribeProp p : m_aProps)
		{
			if (p.m_iTribe == tribe)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	string StatusText()
	{
		return string.Format("props %1 standing (%2 shown so far), call sounds %3 playing (%4 so far), moon %5", m_aProps.Count(), m_iShown, m_aSounds.Count(), m_iSounds, Math.Round(RZ_TribeSky.MoonPhase() * 100) / 100);
	}
}

//------------------------------------------------------------------------------------------------
//! The sky, for the bright moon. The engine's moon (BaseWeatherManagerEntity) in one helper.
class RZ_TribeSky
{
	//------------------------------------------------------------------------------------------------
	//! The moon's phase now, 0 (new) to 1 (full); -1 if it can't be read.
	static float MoonPhase()
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return -1;

		TimeAndWeatherManagerEntity tm = world.GetTimeAndWeatherManager();
		if (!tm)
			return -1;

		vector sunDir;
		vector moonDir;
		float phase;
		tm.GetCurrentSunMoonDirAndPhase(sunDir, moonDir, phase);
		return phase;
	}

	//------------------------------------------------------------------------------------------------
	//! A bright moon tonight: the phase at least `from` (0: off), and a clear night (no rain, fog or
	//! storm).
	static bool BrightMoon(RZ_GameModeComponent gm, float from)
	{
		if (from <= 0)
			return false;

		RZ_Environment env = gm.GetEnvironment();
		if (env && (env.IsRaining() || env.IsFoggy() || env.IsStorm()))
			return false;

		return MoonPhase() >= from;
	}
}

//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "ReforgerZ", description: "A pack's call (or any one-shot sound): plays its sound event once on each client when the entity appears. Put it with a SoundComponent on the call sound prefabs (docs/WORKBENCH_SETUP.md).")]
class RZ_CallSoundComponentClass : ScriptComponentClass
{
}

//! On the call sound prefabs: the server spawns the prefab where the call is and deletes it after
//! 40 s; each client that streams it in plays the sound event once.
class RZ_CallSoundComponent : ScriptComponent
{
	[Attribute("SOUND_CALL", desc: "The SoundComponent's event to play once when the entity appears")]
	string m_sEvent;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		// Only where someone can hear it (not on a dedicated server)
		if (System.IsConsoleApp())
			return;

		GetGame().GetCallqueue().CallLater(Play, 100, false);
	}

	//------------------------------------------------------------------------------------------------
	//! VERIFY: SoundComponent.SoundEvent plays a one-shot event on a client-side copy of the entity.
	protected void Play()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		SoundComponent sound = SoundComponent.Cast(owner.FindComponent(SoundComponent));
		if (sound)
			sound.SoundEvent(m_sEvent);
	}
}
