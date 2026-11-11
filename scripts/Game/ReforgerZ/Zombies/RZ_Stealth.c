//! Stealth (docs/DESIGN.md, "Stealth"): how visible each living player is, and what the infected
//! near them make of it. Server only, once a second on the game mode's fast loop, never per frame.
//!
//! - Visibility, 0 (hidden) to 1 (in plain sight), per player: stance x speed x light x weather,
//!   plus recent noise (their own shots, loud noises where they stand). In a vehicle: 1.
//! - The infected near players: each tracked unit's members get a perception factor from their
//!   kind's base times one from the MOST visible player within the radius (a prone friend never
//!   hides a standing one). Back to the base once nobody is near. Never lowered for infected that
//!   must keep coming (mission hordes, the final stand).
//! - Losing them: a group whose player has stayed hidden and unseen for a while walks to where it
//!   last had them, looks about, and gives up (back to its spot, or on to where a horde was sent).
//!   Never for mission infected (their mission steers them) or the final stand.
//!
//! Works with any zombie mod. With the engine's perception (a PerceptionComponent, and the vanilla
//! SCR_AICombatComponent if the mod uses it) the factor changes how fast they recognise a player
//! and "lost" reads their targets. Without it (a plain aggro radius) the factor does nothing and
//! contact is guessed by distance. Engine calls are in RZ_Senses.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Zombies > Stealth.
[BaseContainerProps()]
class RZ_StealthSettings
{
	[Attribute("1", desc: "Stealth: each player's visibility (stance, speed, light, weather, noise) changes how well the infected near them see them, and infected that lose a hidden player search and give up. Off: infected perception is never touched, and missions' noticed checks work as before (docs/DESIGN.md, Stealth)")]
	bool m_bEnabled;

	[Attribute("1", desc: "Seconds between updates (1 to 5)")]
	float m_fUpdateSeconds;

	[Attribute("60", desc: "Infected within this distance (m) of a player have their senses set from the most visible player near them")]
	float m_fRadius;

	[Attribute("40", desc: "Infected characters touched per update at most (the rest wait for the next one)")]
	int m_iAgentsPerUpdate;

	[Attribute("1", desc: "Visibility: standing")]
	float m_fStandMult;

	[Attribute("0.6", desc: "Visibility: crouched")]
	float m_fCrouchMult;

	[Attribute("0.3", desc: "Visibility: prone")]
	float m_fProneMult;

	[Attribute("0.7", desc: "Visibility: keeping still (under 0.4 m/s)")]
	float m_fStillMult;

	[Attribute("0.85", desc: "Visibility: walking (under 2.6 m/s)")]
	float m_fWalkMult;

	[Attribute("1", desc: "Visibility: running (under 5 m/s)")]
	float m_fRunMult;

	[Attribute("1.25", desc: "Visibility: sprinting")]
	float m_fSprintMult;

	[Attribute("1", desc: "Light by the clock: daytime (used where the engine's light reading isn't, see Engine light weight)")]
	float m_fDayLight;

	[Attribute("0.6", desc: "Light by the clock: dusk (18:30 to 20:30)")]
	float m_fDuskLight;

	[Attribute("0.25", desc: "Light by the clock: night (20:30 to 05:00)")]
	float m_fNightLight;

	[Attribute("0.6", desc: "Share of the light taken from the engine's illumination of the player (the sun, the moon, lamps, their own torch); the rest is the clock. 0 = the clock only")]
	float m_fEngineLightWeight;

	[Attribute("0.4", desc: "Visibility in total darkness, as a share of daylight (the infected still see shapes up close)")]
	float m_fDarkFloor;

	[Attribute("0.75", desc: "Visibility in fog or a fog bank")]
	float m_fFogMult;

	[Attribute("0.85", desc: "Visibility in rain")]
	float m_fRainMult;

	[Attribute("0.7", desc: "Visibility in a storm (instead of rain)")]
	float m_fStormMult;

	[Attribute("0.5", desc: "Noise: visibility added right after a shot (fades over Noise seconds)")]
	float m_fNoiseWeight;

	[Attribute("10", desc: "Noise: seconds a shot or a loud noise where you stand keeps you visible")]
	float m_fNoiseSeconds;

	[Attribute("0.35", desc: "Noise: a suppressed shot counts this much of a shot")]
	float m_fSuppressedNoise;

	[Attribute("1", desc: "Base perception factor: common infected")]
	float m_fBaseCommon;

	[Attribute("1.2", desc: "Base perception factor: specials (sharper)")]
	float m_fBaseSpecial;

	[Attribute("0.9", desc: "Base perception factor: horde members (a crowd on the move looks ahead, not around)")]
	float m_fBaseHorde;

	[Attribute("0.3", desc: "Perception factor from a fully hidden player (x the base). The engine already sees stance and light, so this is on top: keep it well above 0")]
	float m_fMinFactor;

	[Attribute("1.2", desc: "Perception factor from a player in plain sight (x the base)")]
	float m_fMaxFactor;

	[Attribute("0.3", desc: "A player counts as hidden under this visibility (the HUD, missions asking IsHidden)")]
	float m_fHiddenBelow;

	[Attribute("0.35", desc: "Losing them: the player must stay under this visibility")]
	float m_fLoseBelow;

	[Attribute("8", desc: "Losing them: seconds hidden and unseen before they go and look")]
	float m_fLoseSeconds;

	[Attribute("45", desc: "Losing them: seconds they search where they last had you before they give up")]
	float m_fSearchSeconds;

	[Attribute("20", desc: "Losing them, for infected without engine perception (an aggro radius): one this close to a player it can see is on them")]
	float m_fContactRadius;

	[Attribute("0.6", desc: "Missions: a watcher's sight radius (lookouts, Quiet Count's specials) against a fully hidden player, as a share (1 at normal visibility)")]
	float m_fMissionSightMin;

	[Attribute("0.35", desc: "Missions: how fast an infected close by notices a fully hidden player (the noticed checks' closeness timer), as a share (1 at normal visibility)")]
	float m_fMissionNoticeMin;
}

//------------------------------------------------------------------------------------------------
//! What kind of infected a tracked unit is, for its base senses (Stealth > Base factors). The coming
//! kinds (crawlers, scouts, alphas) go here, with a base in RZ_Stealth.KindBase.
enum RZ_EInfectedKind
{
	COMMON,
	SPECIAL,
	HORDE,
}

//------------------------------------------------------------------------------------------------
//! One living player's stealth, worked out each update.
class RZ_StealthPlayer
{
	float m_fVis = 1;			// what everyone reads: m_fBase plus the noise
	float m_fBase = 1;			// stance x speed x light x weather
	float m_fNoise;				// 0..1, fades over Noise seconds
	vector m_vLastPos;
	bool m_bHasPos;
}

//------------------------------------------------------------------------------------------------
//! What stealth keeps on a tracked unit (RZ_TrackedZombie.m_Sense), made the first time a player
//! is near it.
class RZ_SenseState
{
	float m_fFactor = -1;		// the perception factor last set on its members, -1 = never
	float m_fOwn = 1;			// the AI's own multiplier before we touched it
	bool m_bOwnRead;
	int m_iMembers;				// living members it was last set on
	float m_fSinceSet;
	bool m_bNoPerception;		// nothing took the factor: contact is guessed by distance
	float m_fLastAt;			// stealth clock when this unit was last looked at
	int m_iTarget = -1;			// the player it has
	float m_fHiddenFor;			// seconds that player has stayed hidden and unseen
	float m_fSearchLeft;		// > 0: searching where it last had them
	float m_fNearAt = -1;		// no perception: stealth clock when a member was last on the player
	vector m_vNearPos;
	IEntity m_Waypoint;			// the last move waypoint stealth gave it (deleted with it: nothing is left lying)

	//------------------------------------------------------------------------------------------------
	void ~RZ_SenseState()
	{
		if (m_Waypoint && GetGame() && GetGame().GetWorld())
			SCR_EntityHelper.DeleteEntityAndChildren(m_Waypoint);
	}
}

//------------------------------------------------------------------------------------------------
//! The engine calls stealth needs, each small; what a play-test must confirm is marked (docs/WORKBENCH_SETUP.md, section 6).
class RZ_Senses
{
	//------------------------------------------------------------------------------------------------
	//! The character's stance: 0 standing, 1 crouched, 2 prone, -1 unknown.
	static int GetStance(IEntity e)
	{
		ChimeraCharacter c = ChimeraCharacter.Cast(e);
		if (!c)
			return -1;

		CharacterControllerComponent ctrl = c.GetCharacterController();
		if (!ctrl)
			return -1;

		ECharacterStance stance = ctrl.GetStance(); // VERIFY: on the dedicated server, a player's character reports the stance the player is in
		if (stance == ECharacterStance.PRONE)
			return 2;

		if (stance == ECharacterStance.CROUCH)
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! How lit the character is, 0..1, or -1 if it can't be read.
	static float GetIllumination(IEntity e)
	{
		PerceivableComponent perceivable = PerceivableComponent.Cast(e.FindComponent(PerceivableComponent));
		if (!perceivable)
			return -1;

		float light = perceivable.GetIlluminationFactor(); // VERIFY: on the dedicated server a player's illumination factor is 0..1 and follows the sun, the moon, lamps and their own torch
		if (light < 0)
			return -1;

		return Math.Clamp(light, 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! The AI has senses stealth can change: the vanilla combat component or a PerceptionComponent.
	static bool HasPerception(IEntity member)
	{
		if (member.FindComponent(SCR_AICombatComponent))
			return true;

		return member.FindComponent(PerceptionComponent) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! The AI's own perception multiplier (1 if there's none to read).
	static float GetOwnFactor(IEntity member)
	{
		SCR_AICombatComponent combat = SCR_AICombatComponent.Cast(member.FindComponent(SCR_AICombatComponent));
		if (!combat)
			return 1;

		return combat.GetPerceptionFactor();
	}

	//------------------------------------------------------------------------------------------------
	//! How quickly this AI recognises what it sees (1 = normal). With the vanilla combat component,
	//! through its multiplier (the AI's own update puts it on top of its alert state, and would
	//! overwrite a factor set straight on the perception); without, on its PerceptionComponent.
	//! False if neither took it.
	static bool SetFactor(IEntity member, float value)
	{
		SCR_AICombatComponent combat = SCR_AICombatComponent.Cast(member.FindComponent(SCR_AICombatComponent));
		if (combat)
			return combat.RZ_TrySetPerceptionFactor(value); // VERIFY: the zombie mod's AI uses this multiplier (vanilla AI does, through SCR_AIUtilityComponent's update), so a lower factor makes them slower to recognise a player

		PerceptionComponent perception = PerceptionComponent.Cast(member.FindComponent(PerceptionComponent));
		if (!perception)
			return false;

		perception.SetPerceptionFactor(value);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The player in `players` this AI has as a target (an enemy or a detected one) that it had most
	//! recently: its index, seconds since it last saw or heard them, and where. False if it has no
	//! perception or no player among its targets.
	static bool FindPlayerTarget(IEntity member, array<IEntity> players, out int playerIdx, out float since, out vector lastPos)
	{
		PerceptionComponent perception = PerceptionComponent.Cast(member.FindComponent(PerceptionComponent));
		if (!perception)
			return false;

		array<BaseTarget> enemies = {};
		array<BaseTarget> detected = {};
		perception.GetTargetsList(enemies, ETargetCategory.ENEMY); // VERIFY: the zombie mod's infected list players as ENEMY (or DETECTED) targets, with the time since seen and the last seen position
		perception.GetTargetsList(detected, ETargetCategory.DETECTED);
		foreach (BaseTarget t : detected)
		{
			enemies.Insert(t);
		}

		bool found = false;
		foreach (BaseTarget target : enemies)
		{
			if (!target)
				continue;

			int idx = players.Find(target.GetTargetEntity());
			if (idx < 0)
				continue;

			float seen = target.GetTimeSinceSeen();
			float heard = target.GetTimeSinceDetected();
			float latest = Math.Min(seen, heard);
			if (found && latest >= since)
				continue;

			found = true;
			playerIdx = idx;
			since = latest;
			if (seen <= heard)
				lastPos = target.GetLastSeenPosition();
			else
				lastPos = target.GetLastDetectedPosition();
		}

		return found;
	}
}

//------------------------------------------------------------------------------------------------
//! Stealth needs to set the vanilla AI's perception multiplier only once the AI is set up:
//! SetPerceptionFactor reaches into the agent's utility component and the character's perception.
modded class SCR_AICombatComponent
{
	//------------------------------------------------------------------------------------------------
	//! SetPerceptionFactor, if this AI is ready for it. False = not yet (try again later).
	bool RZ_TrySetPerceptionFactor(float value)
	{
		if (!m_Utility || !m_Utility.m_ThreatSystem || !m_Perception)
			return false;

		SetPerceptionFactor(value);
		return true;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_Stealth
{
	protected static RZ_Stealth s_Instance;

	protected RZ_GameModeComponent m_GM;
	protected RZ_StealthSettings m_Settings;
	protected ref map<int, ref RZ_StealthPlayer> m_mPlayers = new map<int, ref RZ_StealthPlayer>();
	protected float m_fClock;
	protected float m_fTimer;
	protected int m_iCursor;
	protected bool m_bToldPerception;
	protected bool m_bToldNoPerception;

	protected static const float SPEED_STILL = 0.4;
	protected static const float SPEED_WALK = 2.6;
	protected static const float SPEED_RUN = 5;
	protected static const float SPEED_JUMP = 15;		// more than this between updates is a teleport or a fall: counted as running
	protected static const float GROUP_SPREAD = 90;		// a group's members are never this far beyond its last known position
	protected static const float AREA_NOISE_RADIUS = 15;	// a loud noise this close counts as the player's own
	protected static const float AREA_NOISE_LEVEL = 0.7;
	protected static const float NORMAL_VIS = 0.6;		// missions see a player at this visibility and above as before
	protected static const float REAPPLY_SECONDS = 10;	// the factor is set again this often (new members, an AI that wasn't ready)
	protected static const float FACTOR_STEP = 0.02;
	protected static const float SEEN_AGAIN = 2;		// a target seen this recently is found: the search ends
	protected static const float TOUCH_RADIUS = 4;		// without perception: this close, they have you whatever you do
	protected static const float FINAL_STAND_RADIUS = 1500;

	//------------------------------------------------------------------------------------------------
	void RZ_Stealth(RZ_GameModeComponent gm, RZ_StealthSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		s_Instance = this;

		if (!m_Settings.m_bEnabled)
		{
			RZ_Log.Info("Stealth: off (infected perception is never touched).");
			return;
		}

		string search = "on";
		if (m_GM.GetSpawner().GetMoveWaypointPrefab().IsEmpty())
			search = "off (no move waypoint)";

		RZ_Log.Info(string.Format("Stealth: on. Infected within %1 m see the most visible player near them (perception x%2 to x%3); losing them after %4 s hidden, searching %5 s: %6.",
			RZ_Util.RoundToInt(m_Settings.m_fRadius), m_Settings.m_fMinFactor, m_Settings.m_fMaxFactor,
			RZ_Util.RoundToInt(m_Settings.m_fLoseSeconds), RZ_Util.RoundToInt(m_Settings.m_fSearchSeconds), search));
	}

	//------------------------------------------------------------------------------------------------
	void ~RZ_Stealth()
	{
		if (s_Instance == this)
			s_Instance = null;
	}

	// ================================================================ For other systems
	//------------------------------------------------------------------------------------------------
	static bool IsActive()
	{
		return s_Instance && s_Instance.m_Settings && s_Instance.m_Settings.m_bEnabled;
	}

	//------------------------------------------------------------------------------------------------
	//! How visible a living player is right now: 0 hidden, 1 in plain sight. 1 with Stealth off, or
	//! for a player it doesn't know yet (just spawned, dead, gone).
	static float GetVisibility(int playerId)
	{
		if (!IsActive())
			return 1;

		RZ_StealthPlayer st;
		if (!s_Instance.m_mPlayers.Find(playerId, st))
			return 1;

		return st.m_fVis;
	}

	//------------------------------------------------------------------------------------------------
	//! Stealth is on and the player's visibility is under `threshold` (below 0: Stealth > Hidden below).
	static bool IsHidden(int playerId, float threshold = -1)
	{
		if (!IsActive())
			return false;

		float limit = threshold;
		if (limit < 0)
			limit = s_Instance.m_Settings.m_fHiddenBelow;

		return GetVisibility(playerId) < limit;
	}

	//------------------------------------------------------------------------------------------------
	//! A watcher's sight radius against this player (a lookout, a counted special): the radius at
	//! normal visibility and above, and with Stealth off; down to Mission sight min x radius for a
	//! fully hidden player. Never more than the radius.
	static float SightRadius(int playerId, float radius)
	{
		if (!IsActive() || playerId <= 0)
			return radius;

		return radius * Share(GetVisibility(playerId), s_Instance.m_Settings.m_fMissionSightMin);
	}

	//------------------------------------------------------------------------------------------------
	//! How fast an infected close by notices this player (the noticed checks' closeness timer,
	//! RZ_NoticeTracker): 1 at normal visibility and above, and with Stealth off; down to Mission
	//! notice min for a fully hidden player.
	static float NoticeRate(int playerId)
	{
		if (!IsActive() || playerId <= 0)
			return 1;

		return Share(GetVisibility(playerId), s_Instance.m_Settings.m_fMissionNoticeMin);
	}

	//------------------------------------------------------------------------------------------------
	//! 1 at NORMAL_VIS and above, `low` at 0, straight between.
	protected static float Share(float vis, float low)
	{
		float floor = Math.Clamp(low, 0.1, 1);
		return floor + (1 - floor) * Math.Clamp(vis / NORMAL_VIS, 0, 1);
	}

	// ================================================================ Hooks
	//------------------------------------------------------------------------------------------------
	//! From RZ_GameModeComponent.OnPlayerShot: a shot gives you away at once.
	void OnPlayerShot(int playerId, bool suppressed)
	{
		if (!m_Settings.m_bEnabled)
			return;

		float level = 1;
		if (suppressed)
			level = Math.Clamp(m_Settings.m_fSuppressedNoise, 0, 1);

		RZ_StealthPlayer st = Player(playerId);
		st.m_fNoise = Math.Max(st.m_fNoise, level);
		st.m_fVis = Math.Clamp(st.m_fBase + m_Settings.m_fNoiseWeight * st.m_fNoise, 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! A new life, or the player left.
	void Forget(int playerId)
	{
		m_mPlayers.Remove(playerId);
	}

	// ================================================================ The update
	//------------------------------------------------------------------------------------------------
	//! From the game mode's one-second loop, with the living players.
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_Settings.m_bEnabled)
			return;

		m_fClock += dt;
		m_fTimer += dt;
		if (m_fTimer < Math.Clamp(m_Settings.m_fUpdateSeconds, 1, 5))
			return;

		float step = m_fTimer;
		m_fTimer = 0;
		UpdatePlayers(step, ids, players);
		UpdateInfected(ids, players);
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_StealthPlayer Player(int playerId)
	{
		RZ_StealthPlayer st;
		if (!m_mPlayers.Find(playerId, st))
		{
			st = new RZ_StealthPlayer();
			m_mPlayers.Set(playerId, st);
		}

		return st;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdatePlayers(float step, array<int> ids, array<IEntity> players)
	{
		// The dead and the gone are dropped: a new life starts in plain sight
		array<int> gone = {};
		foreach (int known, RZ_StealthPlayer kept : m_mPlayers)
		{
			if (!ids.Contains(known))
				gone.Insert(known);
		}

		foreach (int old : gone)
		{
			m_mPlayers.Remove(old);
		}

		float weather = WeatherMult();
		float clockLight = ClockLight();
		float fade = step / Math.Max(1, m_Settings.m_fNoiseSeconds);
		RZ_Environment env = m_GM.GetEnvironment();
		RZ_UIFeed feed = m_GM.GetUIFeed();
		foreach (int i, IEntity ent : players)
		{
			int playerId = ids[i];
			RZ_StealthPlayer st = Player(playerId);
			vector pos = ent.GetOrigin();
			float speed = 0;
			if (st.m_bHasPos)
				speed = RZ_Util.DistXZ(pos, st.m_vLastPos) / step;

			if (speed > SPEED_JUMP)
				speed = SPEED_WALK;

			st.m_vLastPos = pos;
			st.m_bHasPos = true;

			st.m_fNoise = Math.Max(0, st.m_fNoise - fade);
			vector heard;
			if (m_GM.GetHeat().FindRecentNoise(pos, AREA_NOISE_RADIUS, m_Settings.m_fNoiseSeconds, heard))
				st.m_fNoise = Math.Max(st.m_fNoise, AREA_NOISE_LEVEL);

			if (RZ_Util.IsInVehicle(ent))
			{
				st.m_fBase = 1;
			}
			else
			{
				float vis = StanceMult(RZ_Senses.GetStance(ent)) * MoveMult(speed) * LightMult(ent, clockLight) * weather;
				if (env && env.IsInFog(pos))
					vis = vis * m_Settings.m_fFogMult;

				st.m_fBase = Math.Clamp(vis, 0, 1);
			}

			st.m_fVis = Math.Clamp(st.m_fBase + m_Settings.m_fNoiseWeight * st.m_fNoise, 0, 1);
			if (feed)
				feed.PushVisibility(playerId, st.m_fVis, st.m_fVis < m_Settings.m_fHiddenBelow);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected float StanceMult(int stance)
	{
		if (stance == 2)
			return m_Settings.m_fProneMult;

		if (stance == 1)
			return m_Settings.m_fCrouchMult;

		return m_Settings.m_fStandMult;
	}

	//------------------------------------------------------------------------------------------------
	protected float MoveMult(float speed)
	{
		if (speed < SPEED_STILL)
			return m_Settings.m_fStillMult;

		if (speed < SPEED_WALK)
			return m_Settings.m_fWalkMult;

		if (speed < SPEED_RUN)
			return m_Settings.m_fRunMult;

		return m_Settings.m_fSprintMult;
	}

	//------------------------------------------------------------------------------------------------
	//! Day, dusk or night (RZ_WorldFacts); daytime if the clock is unknown.
	protected float ClockLight()
	{
		RZ_WorldFacts facts = m_GM.GetFacts();
		if (!facts || !facts.KnowsTime())
			return m_Settings.m_fDayLight;

		if (facts.IsNight())
			return m_Settings.m_fNightLight;

		if (facts.IsDusk())
			return m_Settings.m_fDuskLight;

		return m_Settings.m_fDayLight;
	}

	//------------------------------------------------------------------------------------------------
	//! The light on this player, mixed from the engine's reading and the clock, as a multiplier
	//! from Dark floor (none) to 1 (daylight).
	protected float LightMult(IEntity ent, float clockLight)
	{
		float light = clockLight;
		float weight = Math.Clamp(m_Settings.m_fEngineLightWeight, 0, 1);
		if (weight > 0)
		{
			float engine = RZ_Senses.GetIllumination(ent);
			if (engine >= 0)
				light = weight * engine + (1 - weight) * clockLight;
		}

		float floor = Math.Clamp(m_Settings.m_fDarkFloor, 0, 1);
		return floor + (1 - floor) * Math.Clamp(light, 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Rain or a storm, and the Fog rule set's shorter sight (RZ_Environment, the season's rules).
	//! Fog itself is per player (a fog bank is local).
	protected float WeatherMult()
	{
		float mult = Math.Clamp(RZ_Seasons.Mods().m_fSightMult, 0.7, 1);
		RZ_Environment env = m_GM.GetEnvironment();
		if (!env)
			return mult;

		if (env.IsStorm())
			mult = mult * m_Settings.m_fStormMult;
		else if (env.IsRaining())
			mult = mult * m_Settings.m_fRainMult;

		return mult;
	}

	// ================================================================ The infected
	//------------------------------------------------------------------------------------------------
	protected void UpdateInfected(array<int> ids, array<IEntity> players)
	{
		array<ref RZ_TrackedZombie> tracked = m_GM.GetSpawner().GetTracked();
		int count = tracked.Count();
		if (count == 0)
			return;

		array<float> vis = {};
		foreach (int i, IEntity p : players)
		{
			vis.Insert(GetVisibility(ids[i]));
		}

		bool finalStand = m_GM.HasLab() && m_GM.GetCure() && m_GM.GetCure().IsFinalStand();
		vector lab = m_GM.GetLabPosition();
		int budget = Math.Max(1, m_Settings.m_iAgentsPerUpdate);
		int start = m_iCursor % count;
		int k = 0;
		while (k < count && budget > 0)
		{
			RZ_TrackedZombie z = tracked[(start + k) % count];
			k++;
			if (!z.m_Entity)
				continue;

			bool held = z.m_bRelentless || (finalStand && RZ_Util.DistXZ(z.m_vLastPos, lab) <= FINAL_STAND_RADIUS);
			budget -= SenseUnit(z, held, ids, players, vis);
		}

		m_iCursor = (start + k) % count;
	}

	//------------------------------------------------------------------------------------------------
	//! Base factor for a kind (Stealth > Base factors).
	protected float KindBase(RZ_EInfectedKind kind)
	{
		switch (kind)
		{
			case RZ_EInfectedKind.SPECIAL: return m_Settings.m_fBaseSpecial;
			case RZ_EInfectedKind.HORDE: return m_Settings.m_fBaseHorde;
		}

		return m_Settings.m_fBaseCommon;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_EInfectedKind KindOf(RZ_TrackedZombie z)
	{
		if (z.m_bHorde)
			return RZ_EInfectedKind.HORDE;

		if (z.m_bSpecial)
			return RZ_EInfectedKind.SPECIAL;

		return RZ_EInfectedKind.COMMON;
	}

	//------------------------------------------------------------------------------------------------
	//! The factor from the most visible player near: Min factor (hidden) to Max factor (in plain sight).
	protected float PlayerFactor(float vis)
	{
		float low = Math.Clamp(m_Settings.m_fMinFactor, 0.05, 1);
		float high = Math.Clamp(m_Settings.m_fMaxFactor, low, 3);
		return low + (high - low) * Math.Clamp(vis, 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! One tracked unit: its members' senses, and losing contact. Returns how many members it
	//! touched (for the per-update cap).
	protected int SenseUnit(RZ_TrackedZombie z, bool held, array<int> ids, array<IEntity> players, array<float> vis)
	{
		RZ_SenseState s = z.m_Sense;
		float elapsed = 0;
		if (s)
		{
			elapsed = m_fClock - s.m_fLastAt;
			s.m_fLastAt = m_fClock;
		}

		float radius = Math.Max(5, m_Settings.m_fRadius);
		bool far = RZ_Util.NearestDistance(z.m_vLastPos, players) > radius + GROUP_SPREAD;

		// Must keep coming, or nobody near: nothing to do unless it was touched before
		if (!s && (held || far))
			return 0;

		array<IEntity> living = {};
		array<IEntity> members = {};
		RZ_Util.GetUnitMembers(z.m_Entity, members);
		foreach (IEntity m : members)
		{
			if (m && RZ_Util.IsCharacterAlive(m))
				living.Insert(m);
		}

		// The most visible player within the radius of any member
		float most = -1;
		if (!far && !held)
		{
			foreach (IEntity member : living)
			{
				vector at = member.GetOrigin();
				foreach (int j, IEntity p : players)
				{
					if (vis[j] > most && RZ_Util.DistXZ(at, p.GetOrigin()) <= radius)
						most = vis[j];
				}
			}
		}

		if (!s)
		{
			if (most < 0)
				return 0;

			s = new RZ_SenseState();
			s.m_fLastAt = m_fClock;
			z.m_Sense = s;
		}

		if (!s.m_bOwnRead && !living.IsEmpty())
		{
			s.m_fOwn = Math.Max(0.05, RZ_Senses.GetOwnFactor(living[0]));
			s.m_bOwnRead = true;
		}

		// Held: back to what it had before we touched it. Otherwise its kind's base, times the most
		// visible player near (none near: the base)
		float factor = s.m_fOwn;
		if (!held)
			factor = s.m_fOwn * KindBase(KindOf(z));

		if (most >= 0)
			factor = factor * PlayerFactor(most);

		int touched = 0;
		s.m_fSinceSet += elapsed;
		bool changed = Math.AbsFloat(factor - s.m_fFactor) > FACTOR_STEP || living.Count() != s.m_iMembers;
		if (s.m_bOwnRead && (changed || (most >= 0 && s.m_fSinceSet >= REAPPLY_SECONDS)))
			touched = Apply(s, living, factor);

		// Their mission steers them, or they must keep coming: they never lose anyone
		if (held || z.m_iMissionId != -1)
		{
			s.m_fSearchLeft = 0;
			s.m_fHiddenFor = 0;
			return touched;
		}

		if (far)
		{
			TickSearch(z, s, elapsed);
			return touched;
		}

		FollowContact(z, s, living, elapsed, ids, players, vis);
		return Math.Max(touched, living.Count());
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the factor on every living member. One that has perception but isn't set up yet (just
	//! spawned) is tried again next update.
	protected int Apply(RZ_SenseState s, array<IEntity> living, float factor)
	{
		int took = 0;
		bool senses = false;
		foreach (IEntity m : living)
		{
			if (RZ_Senses.HasPerception(m))
				senses = true;

			if (RZ_Senses.SetFactor(m, factor))
				took++;
		}

		s.m_fFactor = factor;
		s.m_iMembers = living.Count();
		s.m_fSinceSet = 0;
		if (living.IsEmpty())
			return 0;

		s.m_bNoPerception = !senses;
		if (senses && took == 0)
			s.m_fFactor = -1;

		if (took > 0 && !m_bToldPerception)
		{
			m_bToldPerception = true;
			RZ_Log.Info(string.Format("Stealth: the infected take a perception factor (%1).", RZ_Util.PrettyPrefabName(RZ_Util.GetPrefabName(living[0]))));
		}
		else if (!senses && !m_bToldNoPerception)
		{
			m_bToldNoPerception = true;
			RZ_Log.Info(string.Format("Stealth: some infected take no perception factor (%1): their senses stay as they are, and losing them goes by distance.", RZ_Util.PrettyPrefabName(RZ_Util.GetPrefabName(living[0]))));
		}

		return living.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Losing them: who this group has, whether that player has stayed hidden and unseen long
	//! enough, the search, and giving up.
	protected void FollowContact(RZ_TrackedZombie z, RZ_SenseState s, array<IEntity> living, float elapsed, array<int> ids, array<IEntity> players, array<float> vis)
	{
		if (!SCR_AIGroup.Cast(z.m_Entity) || living.IsEmpty() || m_GM.GetSpawner().GetMoveWaypointPrefab().IsEmpty())
			return;

		int idx = -1;
		float since = 0;
		vector lastPos;
		bool has = false;
		foreach (IEntity m : living)
		{
			int mIdx;
			float mSince;
			vector mPos;
			if (!RZ_Senses.FindPlayerTarget(m, players, mIdx, mSince, mPos))
				continue;

			if (has && mSince >= since)
				continue;

			has = true;
			idx = mIdx;
			since = mSince;
			lastPos = mPos;
		}

		if (!has && s.m_bNoPerception)
			has = NearContact(s, living, ids, players, vis, idx, since, lastPos);

		if (s.m_fSearchLeft > 0)
		{
			// Found again (seen or heard just now): their own AI takes over from here
			if (has && since <= SEEN_AGAIN)
			{
				s.m_fSearchLeft = 0;
				s.m_fHiddenFor = 0;
				return;
			}

			TickSearch(z, s, elapsed);
			return;
		}

		if (!has)
		{
			s.m_iTarget = -1;
			s.m_fHiddenFor = 0;
			return;
		}

		if (ids[idx] != s.m_iTarget)
		{
			s.m_iTarget = ids[idx];
			s.m_fHiddenFor = 0;
		}

		if (vis[idx] < m_Settings.m_fLoseBelow && since >= SEEN_AGAIN)
			s.m_fHiddenFor += elapsed;
		else
			s.m_fHiddenFor = 0;

		float loseAfter = Math.Max(2, m_Settings.m_fLoseSeconds);
		if (s.m_fHiddenFor < loseAfter || since < loseAfter)
			return;

		// Lost you: they go to where they last had you and look about
		if (!Send(z, s, lastPos))
			return;

		s.m_fSearchLeft = Math.Max(5, m_Settings.m_fSearchSeconds);
		s.m_fHiddenFor = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Infected without engine perception: one within Contact radius of a player it could see (not
	//! hidden), or touching them, is on that player; it stays on them, from where it last was, until
	//! they've been hidden long enough.
	protected bool NearContact(RZ_SenseState s, array<IEntity> living, array<int> ids, array<IEntity> players, array<float> vis, out int idx, out float since, out vector lastPos)
	{
		float contact = Math.Max(TOUCH_RADIUS, m_Settings.m_fContactRadius);
		foreach (int j, IEntity p : players)
		{
			vector at = p.GetOrigin();
			foreach (IEntity m : living)
			{
				float d = RZ_Util.DistXZ(m.GetOrigin(), at);
				if (d > TOUCH_RADIUS && (d > contact || vis[j] < m_Settings.m_fLoseBelow))
					continue;

				s.m_fNearAt = m_fClock;
				s.m_vNearPos = at;
				idx = j;
				since = 0;
				lastPos = at;
				return true;
			}
		}

		if (s.m_fNearAt < 0 || s.m_iTarget < 0)
			return false;

		int still = ids.Find(s.m_iTarget);
		if (still < 0)
			return false;

		idx = still;
		since = m_fClock - s.m_fNearAt;
		lastPos = s.m_vNearPos;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The search counts down; at the end they give up: a horde goes on to where it was sent,
	//! anything else back toward where it spawned.
	protected void TickSearch(RZ_TrackedZombie z, RZ_SenseState s, float elapsed)
	{
		if (s.m_fSearchLeft <= 0)
			return;

		s.m_fSearchLeft -= elapsed;
		if (s.m_fSearchLeft > 0)
			return;

		s.m_fSearchLeft = 0;
		s.m_fHiddenFor = 0;
		s.m_iTarget = -1;
		s.m_fNearAt = -1;
		vector back = z.m_vHome;
		if (z.m_bHasGoal)
			back = z.m_vGoal;

		Send(z, s, back);
	}

	//------------------------------------------------------------------------------------------------
	//! The group's waypoints replaced by one move waypoint to pos. The one stealth gave it before is
	//! deleted too, even if the group already finished it. False if none could be made.
	protected bool Send(RZ_TrackedZombie z, RZ_SenseState s, vector pos)
	{
		IEntity before = s.m_Waypoint;
		IEntity made = RZ_Util.RedirectGroup(z.m_Entity, pos, m_GM.GetSpawner().GetMoveWaypointPrefab()); // VERIFY: the zombie mod's group walks to a move waypoint once it has nobody in sight (vanilla groups do; a mod's own wandering may ignore waypoints)
		if (before && before != made)
			SCR_EntityHelper.DeleteEntityAndChildren(before);

		s.m_Waypoint = made;
		return made != null;
	}
}
