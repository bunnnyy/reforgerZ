//! Time and environment (docs/DESIGN.md, "Time and environment"). Never skips or changes time or weather;
//! it reads them and nudges other systems:
//!
//! - Night (RZ_GameTime): the infected hear further (heat also reaches the neighbouring
//!   squares) and specials are a little more common; human AI patrols less (fewer guards at
//!   holdings, staying closer in; hunter squads go out half as often).
//! - Rain: noise carries less (heat added x0.7).
//! - Fog: shorter sight lines for everyone; infected may spawn a little closer.
//! - Storms (the Storm event, or real heavy rain with strong wind): Bastion's helicopter
//!   stays on the ground (no Last Helicopter contract), wanderers shelter.
//!
//! - Fog banks (the Fog Bank event): one region at a time where heat is muffled and infected
//!   spawn closer. The engine's fog is global and is never changed; an optional effect prop
//!   is the look.
//!
//! The weather accessors are engine calls in RZ_Weather; if they don't work, only the
//! night and the Storm event apply.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Director > Environment.
[BaseContainerProps()]
class RZ_EnvironmentSettings
{
	[Attribute("1", desc: "Time and weather nudge the infected and the factions (docs/DESIGN.md, Time and environment). Time is never skipped or changed")]
	bool m_bEnabled = true;

	[Attribute("1", desc: "Read the real weather (rain, fog, wind). Off = only night and the Storm event apply")]
	bool m_bReadWeather = true;

	[Attribute("0.25", desc: "Night: share of every bit of heat that also reaches the four neighbouring heat squares (they hear further). 0 = off")]
	float m_fNightHeatSpread = 0.25;

	[Attribute("1.25", desc: "Night: special chance multiplier")]
	float m_fNightSpecialMult = 1.25;

	[Attribute("1", UIWidgets.Slider, "Night: fewer guards at each holding (never under 1 if it had any)", "0 3 1")]
	int m_iNightGuardDrop = 1;

	[Attribute("0.5", desc: "Night: chance a hunter squad round is skipped")]
	float m_fNightHunterSkip = 0.5;

	[Attribute("0.7", desc: "Rain: heat added is multiplied by this")]
	float m_fRainHeatMult = 0.7;

	[Attribute("0.3", desc: "Rain counts from this intensity (0..1)")]
	float m_fRainFrom = 0.3;

	[Attribute("0.4", desc: "Fog counts from this amount (0..1)")]
	float m_fFogFrom = 0.4;

	[Attribute("0.75", desc: "Fog: the closest distance infected spawn from players is multiplied by this")]
	float m_fFogSpawnMult = 0.75;

	[Attribute("0.6", desc: "Real storm: rain at least this heavy...")]
	float m_fStormRain = 0.6;

	[Attribute("10", desc: "...with wind at least this strong (m/s)")]
	float m_fStormWind = 10;

	[Attribute("1", desc: "Storms ground Bastion's helicopter: no Last Helicopter contract during a storm")]
	bool m_bStormGroundsHelicopter = true;
}

//------------------------------------------------------------------------------------------------
//! Engine weather, kept in small helpers. Each returns -1 if it can't be read.
class RZ_Weather
{
	//------------------------------------------------------------------------------------------------
	protected static TimeAndWeatherManagerEntity Manager()
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return null;

		return world.GetTimeAndWeatherManager();
	}

	//------------------------------------------------------------------------------------------------
	//! Rain 0..1, -1 unknown.
	static float Rain()
	{
		TimeAndWeatherManagerEntity tm = Manager();
		if (!tm)
			return -1;

		return tm.GetRainIntensity();
	}

	//------------------------------------------------------------------------------------------------
	//! Fog 0..1, -1 unknown.
	static float Fog()
	{
		TimeAndWeatherManagerEntity tm = Manager();
		if (!tm)
			return -1;

		return tm.GetFogAmount();
	}

	//------------------------------------------------------------------------------------------------
	//! Wind in m/s, -1 unknown.
	static float Wind()
	{
		TimeAndWeatherManagerEntity tm = Manager();
		if (!tm)
			return -1;

		return tm.GetWindSpeed(); // m/s
	}
}

//------------------------------------------------------------------------------------------------
class RZ_Environment
{
	protected static const float CHECK_INTERVAL = 30;
	protected static const float NEWS_COOLDOWN = 1200;

	protected RZ_GameModeComponent m_GM;
	protected RZ_EnvironmentSettings m_Settings;
	protected float m_fClock;
	protected float m_fCheckTimer = CHECK_INTERVAL;
	protected bool m_bNight;
	protected bool m_bRain;
	protected bool m_bFog;
	protected bool m_bRealStorm;
	protected float m_fRainNewsAt = -10000;
	protected float m_fFogNewsAt = -10000;
	protected bool m_bFogBank;

	//------------------------------------------------------------------------------------------------
	void RZ_Environment(RZ_GameModeComponent gm, RZ_EnvironmentSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	bool IsNight() { return m_Settings.m_bEnabled && m_bNight; }
	bool IsRaining() { return m_Settings.m_bEnabled && m_bRain; }
	bool IsFoggy() { return m_Settings.m_bEnabled && m_bFog; }

	//------------------------------------------------------------------------------------------------
	//! The Fog Bank event: a region where heat added is multiplied by heatMult and infected may
	//! spawn closer (the closest distance multiplied by spawnMult). One at a time.
	void StartFogBank(vector pos, float radius, float heatMult, float spawnMult)
	{
		m_bFogBank = true;
		m_GM.GetHeat().SetFogBank(true, pos, radius, heatMult);
		m_GM.GetSpawner().SetFogBank(true, pos, radius, spawnMult);
	}

	//------------------------------------------------------------------------------------------------
	void EndFogBank()
	{
		m_bFogBank = false;
		m_GM.GetHeat().SetFogBank(false, vector.Zero, 0, 1);
		m_GM.GetSpawner().SetFogBank(false, vector.Zero, 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! The Storm event, or real heavy rain with strong wind.
	bool IsStorm()
	{
		if (m_GM.GetMissions() && m_GM.GetMissions().IsTypeRunning(RZ_Event_Storm))
			return true;

		return m_Settings.m_bEnabled && m_bRealStorm;
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion's helicopter stays on the ground in a storm (Last Helicopter weight 0).
	bool IsHelicopterGrounded()
	{
		return m_Settings.m_bStormGroundsHelicopter && IsStorm();
	}

	//------------------------------------------------------------------------------------------------
	//! Guards at a holding: fewer at night, never under one if it had any. Under a curfew
	//! (season rules) Bastion keeps its full guard.
	int GuardCount(int normal, int faction = -1)
	{
		if (!IsNight() || normal <= 0)
			return normal;

		if (faction == RZ_EFaction.MILITARY && RZ_Seasons.Mods().m_bCurfew)
			return normal;

		return Math.Max(1, normal - m_Settings.m_iNightGuardDrop);
	}

	//------------------------------------------------------------------------------------------------
	//! Hunter squads go out less at night.
	bool SkipHunterRound()
	{
		return IsNight() && Math.RandomFloat01() < m_Settings.m_fNightHunterSkip;
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		m_fClock += dt;
		m_fCheckTimer += dt;
		if (m_fCheckTimer < CHECK_INTERVAL)
			return;

		m_fCheckTimer = 0;
		if (!m_Settings.m_bEnabled)
		{
			m_GM.GetHeat().SetEnvironmentModifiers(1, 0);
			m_GM.GetSpawner().SetEnvironmentModifiers(1, 1);
			return;
		}

		m_bNight = RZ_GameTime.IsNight();

		bool rain = false;
		bool fog = false;
		bool storm = false;
		if (m_Settings.m_bReadWeather)
		{
			float r = RZ_Weather.Rain();
			float f = RZ_Weather.Fog();
			float w = RZ_Weather.Wind();
			rain = r >= m_Settings.m_fRainFrom;
			fog = f >= m_Settings.m_fFogFrom / RZ_Seasons.Mods().m_fFogMult;
			storm = r >= m_Settings.m_fStormRain && w >= m_Settings.m_fStormWind;
		}

		if (rain && !m_bRain && m_fClock - m_fRainNewsAt >= NEWS_COOLDOWN && !ids.IsEmpty())
		{
			m_fRainNewsAt = m_fClock;
			m_GM.NotifyAll("Rain", "Rain on the roofs. It'll swallow some of your noise while it lasts.");
		}

		if (fog && !m_bFog && m_fClock - m_fFogNewsAt >= NEWS_COOLDOWN && !ids.IsEmpty())
		{
			m_fFogNewsAt = m_fClock;
			m_GM.NotifyAll("Fog", "Fog's coming in. You won't see them coming, and they won't see you until they're close.");
		}

		m_bRain = rain;
		m_bFog = fog;
		m_bRealStorm = storm;

		float heatMult = 1;
		if (m_bRain)
			heatMult = m_Settings.m_fRainHeatMult;

		// Season rules: Winter nights carry sound further and bring out more specials; Fog
		// shortens sight (infected spawn closer) all the time
		RZ_SeasonMods season = RZ_Seasons.Mods();
		float spread = 0;
		if (m_bNight)
			spread = Math.Min(0.5, m_Settings.m_fNightHeatSpread * season.m_fNightHearingMult);

		m_GM.GetHeat().SetEnvironmentModifiers(heatMult, spread);

		float specials = 1;
		if (m_bNight)
			specials = m_Settings.m_fNightSpecialMult * season.m_fNightSpecialMult;

		float spawnDist = season.m_fSightMult;
		if (m_bFog)
			spawnDist = spawnDist * m_Settings.m_fFogSpawnMult;

		m_GM.GetSpawner().SetEnvironmentModifiers(specials, spawnDist);
	}

	//------------------------------------------------------------------------------------------------
	//! "night, rain" for the status log.
	string Describe()
	{
		string text = "day";
		if (m_bNight)
			text = "night";

		if (m_bRain)
			text += ", rain";

		if (m_bFog)
			text += ", fog";

		if (m_bFogBank)
			text += ", fog bank";

		if (IsStorm())
			text += ", storm";

		return text;
	}
}
