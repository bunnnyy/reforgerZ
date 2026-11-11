//! The infected behaviour layer (docs/INFECTED.md, "How they move and hunt"): our code drives the
//! bodies of the tribes beyond striking range, and Bacon fights at striking range. Each body is in
//! one of four modes (RZ_EBodyMode); a steered body is blind (perception factor 0, set in its spawn
//! frame: B1) and walks to a slot we give it (L2), so its group can move in a formation, flank,
//! stalk or ring a target. Close in, or hit, or shot at, it's released to Bacon; when the fight
//! moves away it's taken back. Server only; runs once a second from the game mode, only for groups
//! a tribe owns (RZ_TrackedZombie.m_Plan). The hooks are in RZ_BaconHooks.c; the strategies,
//! formations and senses in RZ_HuntPlan.c.

//------------------------------------------------------------------------------------------------
//! Zombies > Behaviour. Off: every body is Bacon's own (OFF), with stealth's senses, and tribe
//! groups are sent with held waypoints only.
[BaseContainerProps()]
class RZ_BehaviourSettings
{
	[Attribute("1", desc: "Behaviour layer: our code drives the tribes' bodies beyond striking range (blind steering, slots, formations, flanks, stalks, rings, the climb gate, water and stairs rules, speed caps per kind, LOD for far bodies) and Bacon fights at striking range. Needs Bacon Zombies; other zombie mods' bodies stay Bacon-less and are sent with waypoints. Off: tribe groups take held waypoints and Bacon's own senses (with stealth)")]
	bool m_bEnabled;

	[Attribute("0", desc: "Steer factor: the perception factor of a steered body. 0 = blind (lab B1: 0 of 24 engaged at 6-150 m). The fallback if blind bodies ever misbehave is 0.25 (0% engaged by day in the open)")]
	float m_fSteerFactor;

	[Attribute("8", desc: "Contact radius (m): a player this close in line of sight releases a steered body to Bacon at once (3 m without line of sight, fixed)")]
	float m_fContactRadius;

	[Attribute("2.5", desc: "Take-back multiple: an engaged body whose target is beyond this times its kind's engage radius for 8 s goes back to our code")]
	float m_fTakeBackMult;

	[Attribute("12", desc: "Sight checks a second: line-of-sight checks the layer's senses may make (terrain samples, and a physics trace within 80 m), nearest players first. Cached 2 s")]
	int m_iSightChecks;

	[Attribute("0.15", desc: "Notice rate: how fast a group's notice meter fills, per second, for a fully visible player at its feet (docs/INFECTED.md, Senses). Tune so a hunter by day notices a walker at 100 m in about 20 s")]
	float m_fNoticeRate;

	[Attribute("220", desc: "Hunter sight by day (m). Other kinds scale with it (young 180, guard 200, alpha 160)")]
	float m_fHunterSightDay;

	[Attribute("190", desc: "Hunter sight at night (m)")]
	float m_fHunterSightNight;

	[Attribute("170", desc: "Shot hearing (m): an unsuppressed shot gives a steered group a lead from this far (lab: Bacon's blind listeners came from 80-170 m); suppressed 30 m. Each kind multiplies it")]
	float m_fShotHearing;

	[Attribute("3", desc: "Spacing (m): the least distance between two bodies' slots in a group (8 round an alpha). Lab B20: slots alone let pair members come within 0.8 m, so the code enforces it")]
	float m_fSpacing;

	[Attribute("140", desc: "Flank radius by day (m): how far out a flanking pair circles before it strikes")]
	float m_fFlankDay;

	[Attribute("100", desc: "Flank radius at night (m)")]
	float m_fFlankNight;

	[Attribute("70", desc: "Flank angle (degrees off the line of approach)")]
	float m_fFlankAngle;

	[Attribute("115", desc: "Stalk distance by day (m): a stalking group shadows its target this far off, from cover (80-150)")]
	float m_fStalkDay;

	[Attribute("75", desc: "Stalk distance at night (m) (50-100)")]
	float m_fStalkNight;

	[Attribute("1", desc: "Strategies: groups stalk, flank and ring by temperament. Off: every group rushes once it notices you")]
	bool m_bStrategies;

	[Attribute("1", desc: "Own attacks: a hunter's lunge from 6-10 m out of your view, then Bacon's swing; landing it makes the bite chance x1.5 (never over 0.6) and opens a light bleed. Off: plain Bacon melee")]
	bool m_bOwnAttacks;

	[Attribute("10", desc: "Special move gap (s): at most one special move lands on one player in this long, from all the infected together")]
	float m_fSpecialMoveGap;

	[Attribute("1", desc: "Crawler life: a town's crawlers lie up by day (asleep beyond 40 m of a player with Sleep lying crawlers on), wake to a player or a noise near them, stand in herds round their doorways at dusk and drift the streets in slow herds at night, giving way to a pack passing through. Off: Bacon's own (with the placement, drift and stir that are built)")]
	bool m_bCrawlerLife;

	[Attribute("1", desc: "Sleep lying crawlers: a crawler group lying up by day with no player within 40 m has its AI switched off, and back on when a player or a noise comes near (lab B12: awake in 0.2 s, moving in 6.5 s). Fewer active AI, but a sleeping body doesn't turn until it wakes")]
	bool m_bSleepLying;

	[Attribute("0", desc: "Crawler speed cap: a fraction of a body's sprint speed for crawlers (0.4 = about 1.8 m/s, 0.5 about 2.2). 0 = Bacon's own speed. Lab B7: the cap works, but the body probably runs in slow motion; set it once a client has seen how it looks")]
	float m_fCrawlerSpeedCap;

	[Attribute("1.4", desc: "Climb limit (m of grab height) for the young, hunters and scouts; the guard 0.2 less; alphas never climb. Lab B8: a 1.2 m limit refused a 1.39 m fence every time, and a tall rail fence is climbed at its low rail")]
	float m_fClimbLimit;

	[Attribute("1", desc: "Unstick out of sight: a body stuck through every other step may be moved up to 15 m ahead, only where no player is within 60 m or has terrain line of sight within 150 m")]
	bool m_bUnstickOutOfSight;

	[Attribute("60,150,300", desc: "LOD bands (m): bodies nearer every player than the first think every loop (LOD 0); then LOD 2, 4, and 6 beyond the last (lab B12: p99 -2.3 ms with 100 at LOD 6)")]
	string m_sLodBands;

	[Attribute("0", desc: "Debug log: one line a minute with bodies per mode, hunts running, releases, take-backs, unsticks and moves landed")]
	bool m_bDebugLog;
}

//------------------------------------------------------------------------------------------------
//! Who drives a body. The numbers are the ones the hooks read (RZ_BaconHooks.c).
enum RZ_EBodyMode
{
	OFF,		// Bacon's own, as before (missions, crawlers, everything not a tribe's)
	STEER,		// our code: blind, intake closed, walking to its slot
	ENGAGE,		// Bacon's chase and melee; our overlay sets the gait and our attacks
	RECOVER		// taken back from Bacon: blind again, waiting for Bacon's own target to clear
}

//------------------------------------------------------------------------------------------------
//! The kinds of body (docs/INFECTED.md, "The kinds"): each has its speeds, senses, reach and climb.
enum RZ_EBodyKind
{
	CRAWLER,
	YOUNG,
	HUNTER,
	SCOUT,
	GUARD,
	ALPHA
}

//------------------------------------------------------------------------------------------------
//! One kind's numbers. Speed caps are fractions of a body's maximum (sprint) speed, about 4.4 m/s
//! for every prefab (lab B7: 0.2 = 0.85 m/s, 0.4 = 1.8, 0.6 = 3.0, no effect above about 0.7).
class RZ_KindProfile
{
	float m_fWalkCap = 1;		// walk-stop (no group waypoint): 1 = Bacon's walk (1.45 m/s moving)
	float m_fMarchCap = 1;		// a held waypoint at RUN: 0.45 = about 2 m/s
	float m_fSight = 1;			// x Hunter sight
	float m_fHearing = 1;		// x the shot radius
	float m_fMemory = 30;		// s after losing a target before giving up
	float m_fEngage = 25;		// E: released to Bacon this close to its target
	float m_fClimb = 1.4;		// grab height it may climb; < 0 never
	float m_fStanceMin;			// 0 = Bacon's (0.82-0.92)
	float m_fStanceMax;
	bool m_bMuteStalking;
	bool m_bBursts;				// sprint bursts in a chase (the young)
}

//------------------------------------------------------------------------------------------------
//! One body the layer knows.
class RZ_BodyState
{
	IEntity m_Body;				// weak
	RZ_EBodyKind m_eKind;
	RZ_EBodyMode m_eMode;
	float m_fModeAt;
	vector m_vSlot;
	vector m_vFormSlot;			// its last formation slot, checked dry
	float m_fSpeedFactor = 1;	// 0.88-1 on its kind's caps
	vector m_vJitter;
	float m_fStance;
	float m_fCap = -1;			// the speed cap applied now
	int m_iLod = -1;
	float m_fLodAt;
	float m_fHealth = -1;
	bool m_bHit;				// took damage this tick
	// Stuck watch
	vector m_vWatchFrom;
	float m_fWatchAt;
	int m_iLadder;
	float m_fLadderUntil;
	vector m_vLadderSlot;
	// Engaged
	float m_fFarSince;
	float m_fBelowSince;
	bool m_bBelow;
	float m_fClosing = -1;		// distance to target at the last engaged check
	float m_fClosingAt;
	// Attacks
	float m_fLungeReady;
	float m_fLungeAt = -100;
	float m_fBurstAt;
	float m_fLeapReady;
	float m_fChargeReady;
	float m_fChargeUntil = -1;		// charging until (the alpha)
	float m_fStumbleUntil = -1;		// a missed charge: capped low until
}

//------------------------------------------------------------------------------------------------
//! The sprint key (SCR_ChimeraCharacter.SetSpeedLimit keeps one cap per source).
class RZ_SpeedSource
{
}

//------------------------------------------------------------------------------------------------
class RZ_Behaviour
{
	protected static RZ_Behaviour s_Instance;

	protected RZ_GameModeComponent m_GM;
	protected RZ_BehaviourSettings m_Settings;
	protected ref array<ref RZ_KindProfile> m_aKinds = {};
	protected ref array<ref RZ_HuntLink> m_aLinks = {};
	protected ref RZ_SpeedSource m_SpeedKey = new RZ_SpeedSource();
	protected ref array<IEntity> m_aSprinting = {};		// weak: bodies with SPRINT wanted until m_aSprintUntil
	protected ref array<float> m_aSprintUntil = {};
	protected ref array<float> m_aLodBands = {};
	protected ref map<int, vector> m_mPlayerPos = new map<int, vector>();
	protected ref map<int, float> m_mPlayerStillSince = new map<int, float>();
	protected ref map<int, float> m_mSpecialAt = new map<int, float>();
	protected ref map<int, float> m_mBleedAt = new map<int, float>();
	protected ref array<vector> m_aStuckMarks = {};
	protected ref array<float> m_aStuckUntil = {};
	protected float m_fNow;
	protected int m_iChecksLeft;
	protected bool m_bNight;
	protected bool m_bFastRunning;
	protected float m_fDebugTimer;
	// Counters for the debug line and the status log
	protected int m_iReleases;
	protected int m_iTakeBacks;
	protected int m_iUnsticks;
	protected int m_iMovesLanded;
	protected int m_iHuntsStarted;
	protected int m_iHandOffs;
	protected int m_iLeaps;
	protected int m_iCharges;
	protected int m_iStumbles;
	protected ref map<int, float> m_mChargedAt = new map<int, float>();
	protected ref RZ_CrawlerLife m_Crawlers;

	static const float CONTACT_BLIND = 3;		// any player this close releases a body, sight or not
	static const float SHOT_CONTACT = 30;		// an unsuppressed shot this close releases it
	static const float SHARE_RELEASE = 15;		// a released member pulls the others this close in its group
	static const float MIN_ENGAGED = 10;		// s in ENGAGE before a take-back
	static const float MIN_RECOVER = 5;			// s in RECOVER before a release
	static const float RECOVER_CLEAR = 15;		// s before a recovered body is steered again (B5: target clears at 8 s, suspect at 15 s)
	static const float NEVER_TAKE_BACK = 15;	// never with a player this close in sight
	static const float STAIRS_UP = 2.5;
	static const float STAIRS_ACROSS = 15;
	static const float STAIRS_WAIT = 20;
	static const float STUCK_MOVE = 1.5;
	static const float STUCK_SECONDS = 10;
	static const float MAX_SPEED = 4.4;			// m/s a cap of 1 means (B7)

	//------------------------------------------------------------------------------------------------
	void RZ_Behaviour(RZ_GameModeComponent gm, RZ_BehaviourSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		s_Instance = this;
		BuildKinds();
		ParseLodBands();
		m_Crawlers = new RZ_CrawlerLife(gm, this);
	}

	//------------------------------------------------------------------------------------------------
	void ~RZ_Behaviour()
	{
		if (s_Instance == this)
			s_Instance = null;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_Behaviour Get() { return s_Instance; }
	RZ_BehaviourSettings GetSettings() { return m_Settings; }
	float Now() { return m_fNow; }
	bool IsNight() { return m_bNight; }

	//------------------------------------------------------------------------------------------------
	//! The layer drives bodies (Zombies > Behaviour > Enabled).
	static bool IsOn()
	{
		return s_Instance && s_Instance.m_Settings && s_Instance.m_Settings.m_bEnabled;
	}

	//------------------------------------------------------------------------------------------------
	void Setup()
	{
		if (!IsOn())
		{
			RZ_Log.Info("Behaviour: off (tribe groups take held waypoints; Bacon's own senses, with stealth).");
			return;
		}

		RZ_Log.Info(string.Format("Behaviour: on. Tribe bodies are steered blind beyond striking range (factor %1), released to Bacon within their reach or on contact (%2 m), taken back beyond x%3; strategies %4, own attacks %5.",
			m_Settings.m_fSteerFactor, RZ_Util.RoundToInt(m_Settings.m_fContactRadius), m_Settings.m_fTakeBackMult, m_Settings.m_bStrategies, m_Settings.m_bOwnAttacks));
	}

	//------------------------------------------------------------------------------------------------
	//! The kinds' table (docs/INFECTED.md, "The kinds").
	protected void BuildKinds()
	{
		m_aKinds.Clear();
		// CRAWLER: not steered (crawler life only sets its herd slots, its sleep and its cap)
		RZ_KindProfile crawler = new RZ_KindProfile();
		crawler.m_fWalkCap = 0.24;
		crawler.m_fMarchCap = 0.45;
		crawler.m_fSight = 0.27;
		crawler.m_fHearing = 1.5;
		crawler.m_fMemory = 20;
		crawler.m_fEngage = 12;
		crawler.m_fClimb = -1;
		crawler.m_fStanceMin = 0.6;
		crawler.m_fStanceMax = 0.7;
		m_aKinds.Insert(crawler);

		RZ_KindProfile young = new RZ_KindProfile();
		young.m_fSight = 0.82;
		young.m_fHearing = 1.2;
		young.m_fMemory = 30;
		young.m_fEngage = 35;
		young.m_fStanceMin = 0.85;
		young.m_fStanceMax = 0.95;
		young.m_bBursts = true;
		m_aKinds.Insert(young);

		RZ_KindProfile hunter = new RZ_KindProfile();
		hunter.m_fMarchCap = 0.45;
		hunter.m_fMemory = 90;
		hunter.m_fEngage = 25;
		hunter.m_fStanceMin = 0.72;
		hunter.m_fStanceMax = 0.82;
		hunter.m_bMuteStalking = true;
		m_aKinds.Insert(hunter);

		RZ_KindProfile scout = new RZ_KindProfile();
		scout.m_fMemory = 10;
		scout.m_fEngage = 15;
		scout.m_fStanceMin = 0.9;
		scout.m_fStanceMax = 0.9;
		m_aKinds.Insert(scout);

		RZ_KindProfile guard = new RZ_KindProfile();
		guard.m_fMarchCap = 0.45;
		guard.m_fSight = 0.91;
		guard.m_fMemory = 60;
		guard.m_fEngage = 30;
		guard.m_fClimb = 1.2;
		m_aKinds.Insert(guard);

		RZ_KindProfile alpha = new RZ_KindProfile();
		alpha.m_fWalkCap = 0.32;
		alpha.m_fMarchCap = 0.45;
		alpha.m_fSight = 0.73;
		alpha.m_fHearing = 0.8;
		alpha.m_fMemory = 120;
		alpha.m_fEngage = 40;
		alpha.m_fClimb = -1;
		m_aKinds.Insert(alpha);
	}

	//------------------------------------------------------------------------------------------------
	RZ_KindProfile Kind(RZ_EBodyKind kind)
	{
		int i = kind;
		if (i < 0 || i >= m_aKinds.Count())
			return m_aKinds[RZ_EBodyKind.HUNTER];

		return m_aKinds[i];
	}

	//------------------------------------------------------------------------------------------------
	//! A kind's sight radius now (Hunter sight by day or night, times the kind's share).
	float SightOf(RZ_EBodyKind kind)
	{
		float base = m_Settings.m_fHunterSightDay;
		if (m_bNight)
			base = m_Settings.m_fHunterSightNight;

		return base * Kind(kind).m_fSight;
	}

	//------------------------------------------------------------------------------------------------
	//! A kind's climb limit (grab height), < 0 never.
	float ClimbOf(RZ_EBodyKind kind)
	{
		RZ_KindProfile k = Kind(kind);
		if (k.m_fClimb < 0)
			return -1;

		if (kind == RZ_EBodyKind.GUARD)
			return Math.Max(0.5, m_Settings.m_fClimbLimit - 0.2);

		return m_Settings.m_fClimbLimit;
	}

	//------------------------------------------------------------------------------------------------
	protected void ParseLodBands()
	{
		m_aLodBands.Clear();
		array<string> parts = {};
		m_Settings.m_sLodBands.Split(",", parts, true);
		foreach (string p : parts)
		{
			float v = p.Trim().ToFloat();
			if (v > 0)
				m_aLodBands.Insert(v);
		}

		if (m_aLodBands.Count() != 3)
		{
			m_aLodBands.Clear();
			m_aLodBands.Insert(60);
			m_aLodBands.Insert(150);
			m_aLodBands.Insert(300);
		}
	}

	// ================================================================ A body's first frame
	//------------------------------------------------------------------------------------------------
	//! In the spawn frame of a tribe body, before and after it joins its group: steered and blind,
	//! with its kind's climb limit and stance (B1: the factor set in the spawn frame holds down to
	//! 6 m). Does nothing for a body the layer can't drive.
	static void Blind(IEntity body, RZ_EBodyKind kind)
	{
		if (!IsOn() || !RZ_BaconBody.IsBacon(body))
			return;

		RZ_BaconBody.SetMode(body, RZ_EBodyMode.STEER);
		RZ_BaconBody.SetSlot(body, body.GetOrigin());
		RZ_BaconBody.SetClimb(body, s_Instance.ClimbOf(kind));
		SetFactor(body, s_Instance.m_Settings.m_fSteerFactor);
	}

	//------------------------------------------------------------------------------------------------
	//! The perception factor, through stealth's path (it stores it for the AI's own update), and
	//! straight on the perception when the AI isn't ready yet.
	static void SetFactor(IEntity body, float factor)
	{
		if (!body)
			return;

		if (RZ_Senses.SetFactor(body, factor))
			return;

		PerceptionComponent perception = PerceptionComponent.Cast(body.FindComponent(PerceptionComponent));
		if (perception)
			perception.SetPerceptionFactor(factor);
	}

	//------------------------------------------------------------------------------------------------
	//! A body steered or recovering: stealth leaves its factor alone (our senses judge for it).
	static bool IsSteered(IEntity body)
	{
		int mode = RZ_BaconBody.GetMode(body);
		return mode == RZ_EBodyMode.STEER || mode == RZ_EBodyMode.RECOVER;
	}

	// ================================================================ The tick
	//------------------------------------------------------------------------------------------------
	//! Once a second, from the game mode: every tribe group's senses, plan and bodies.
	void Tick(array<int> ids, array<IEntity> players)
	{
		m_fNow = RZ_BaconBody.Now();
		RZ_WorldFacts facts = m_GM.GetFacts();
		m_bNight = facts && facts.IsNight();
		m_iChecksLeft = Math.Max(1, m_Settings.m_iSightChecks);
		TrackPlayers(ids, players);

		for (int li = m_aLinks.Count() - 1; li >= 0; li--)
		{
			if (!m_aLinks[li].IsAlive())
				m_aLinks.Remove(li);
		}

		array<ref RZ_TrackedZombie> tracked = m_GM.GetSpawner().GetTracked();
		foreach (RZ_TrackedZombie z : tracked)
		{
			if (!z.m_Plan || !z.m_Entity)
				continue;

			z.m_Plan.Tick(this, z, ids, players);
		}

		m_Crawlers.Tick(players);

		if (m_Settings.m_bDebugLog)
		{
			m_fDebugTimer += 1;
			if (m_fDebugTimer >= 60)
			{
				m_fDebugTimer = 0;
				RZ_Log.Info("Behaviour | " + StatusText());
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Where each player is and how long they've stood still (a strike's "moment").
	protected void TrackPlayers(array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity p : players)
		{
			int id = ids[i];
			vector at = p.GetOrigin();
			vector was;
			if (!m_mPlayerPos.Find(id, was) || RZ_Util.DistXZ(was, at) > 1)
				m_mPlayerStillSince.Set(id, m_fNow);

			m_mPlayerPos.Set(id, at);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Seconds a player has stood still (0 moving or unknown).
	float StillFor(int playerId)
	{
		float since;
		if (!m_mPlayerStillSince.Find(playerId, since))
			return 0;

		return m_fNow - since;
	}

	//------------------------------------------------------------------------------------------------
	//! One sight check may still be made this second.
	bool TakeCheck()
	{
		if (m_iChecksLeft <= 0)
			return false;

		m_iChecksLeft--;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A new hunt (a group of groups after one player), owned here; plans hold it weakly.
	RZ_HuntLink NewLink(int playerId, RZ_EStrategy strategy, bool relentless, bool day)
	{
		RZ_HuntLink link = new RZ_HuntLink();
		link.m_iTarget = playerId;
		link.m_eStrategy = strategy;
		link.m_bRelentless = relentless;
		link.m_bDay = day;
		link.m_fStartedAt = m_fNow;
		link.m_fLastSeenAt = m_fNow;
		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		link.m_TargetEnt = ent;
		if (ent)
			link.m_vLastKnown = ent.GetOrigin();

		link.m_vGap = RandomDir();
		m_aLinks.Insert(link);
		m_iHuntsStarted++;
		return link;
	}

	//------------------------------------------------------------------------------------------------
	static vector RandomDir()
	{
		float a = Math.RandomFloat(0, Math.PI2);
		return Vector(Math.Sin(a), 0, Math.Cos(a));
	}

	// ================================================================ Modes
	//------------------------------------------------------------------------------------------------
	//! STEER or RECOVER to ENGAGE: Bacon fights. The factor comes back (stealth tunes it within its
	//! next update), the intake opens, and the body runs (B6: without RUN set, a released body walks).
	void Release(RZ_BodyState b)
	{
		if (!b.m_Body || b.m_eMode == RZ_EBodyMode.ENGAGE)
			return;

		b.m_eMode = RZ_EBodyMode.ENGAGE;
		b.m_fModeAt = m_fNow;
		b.m_fFarSince = 0;
		b.m_fClosing = -1;
		b.m_bBelow = false;
		RZ_BaconBody.SetMode(b.m_Body, RZ_EBodyMode.ENGAGE);
		RZ_BaconBody.ClearSlot(b.m_Body);
		RZ_BaconBody.SetMute(b.m_Body, false);
		RZ_BaconBody.SetStance(b.m_Body, 0);
		SetFactor(b.m_Body, 1);
		SetCap(b, 1);
		WantMove(b.m_Body, EMovementType.RUN);
		m_iReleases++;
	}

	//------------------------------------------------------------------------------------------------
	//! ENGAGE to RECOVER: blind again, its slot where we want it, and Bacon's loiter point there too
	//! so Roam stops running to where the target really was (B5).
	void TakeBack(RZ_BodyState b, vector slot)
	{
		if (!b.m_Body || b.m_eMode == RZ_EBodyMode.RECOVER)
			return;

		b.m_eMode = RZ_EBodyMode.RECOVER;
		b.m_fModeAt = m_fNow;
		b.m_vSlot = slot;
		RZ_BaconBody.SetMode(b.m_Body, RZ_EBodyMode.RECOVER);
		RZ_BaconBody.SetSlot(b.m_Body, slot);
		RZ_BaconBody.SetLoiter(b.m_Body, slot);
		SetFactor(b.m_Body, m_Settings.m_fSteerFactor);
		m_iTakeBacks++;
	}

	//------------------------------------------------------------------------------------------------
	//! RECOVER to STEER once Bacon's own target has cleared.
	void Resteer(RZ_BodyState b)
	{
		b.m_eMode = RZ_EBodyMode.STEER;
		b.m_fModeAt = m_fNow;
		b.m_fWatchAt = m_fNow;
		b.m_vWatchFrom = b.m_Body.GetOrigin();
		b.m_iLadder = 0;
		RZ_BaconBody.SetMode(b.m_Body, RZ_EBodyMode.STEER);
	}

	//------------------------------------------------------------------------------------------------
	//! A speed cap as a fraction of the maximum (1 clears it). Only set when it changes.
	void SetCap(RZ_BodyState b, float fraction)
	{
		float f = Math.Clamp(fraction, 0.1, 1);
		if (Math.AbsFloat(f - b.m_fCap) < 0.01)
			return;

		b.m_fCap = f;
		SCR_ChimeraCharacter c = SCR_ChimeraCharacter.Cast(b.m_Body);
		if (c)
			c.SetSpeedLimit(m_SpeedKey, f, true);
	}

	//------------------------------------------------------------------------------------------------
	//! L5: the movement type the AI wants (RUN at a release; SPRINT must be set again every 0.25 s).
	static void WantMove(IEntity body, EMovementType type)
	{
		if (!body)
			return;

		AICharacterMovementComponent mc = AICharacterMovementComponent.Cast(body.FindComponent(AICharacterMovementComponent));
		if (mc)
			mc.SetMovementTypeWanted(type);
	}

	//------------------------------------------------------------------------------------------------
	//! A sprint for `seconds` (a young's burst, a hunter's lunge): SPRINT set every 0.25 s until then
	//! (lab B7: set once it fades to a run in a second).
	void Sprint(IEntity body, float seconds)
	{
		if (!body)
			return;

		int at = m_aSprinting.Find(body);
		if (at >= 0)
		{
			m_aSprintUntil[at] = m_fNow + seconds;
		}
		else
		{
			m_aSprinting.Insert(body);
			m_aSprintUntil.Insert(m_fNow + seconds);
		}

		WantMove(body, EMovementType.SPRINT);
		if (!m_bFastRunning)
		{
			m_bFastRunning = true;
			GetGame().GetCallqueue().CallLater(FastTick, 250, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every 0.25 s while anything sprints: SPRINT again, and back to RUN when it ends.
	protected void FastTick()
	{
		float now = RZ_BaconBody.Now();
		for (int i = m_aSprinting.Count() - 1; i >= 0; i--)
		{
			IEntity body = m_aSprinting[i];
			if (!body || !RZ_Util.IsCharacterAlive(body))
			{
				m_aSprinting.Remove(i);
				m_aSprintUntil.Remove(i);
				continue;
			}

			if (now >= m_aSprintUntil[i])
			{
				WantMove(body, EMovementType.RUN);
				m_aSprinting.Remove(i);
				m_aSprintUntil.Remove(i);
				continue;
			}

			WantMove(body, EMovementType.SPRINT);
		}

		if (m_aSprinting.IsEmpty())
		{
			m_bFastRunning = false;
			GetGame().GetCallqueue().Remove(FastTick);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! L13: the think rate by distance (bands from Zombies > Behaviour > LOD bands). Checked every 5 s.
	void UpdateLod(RZ_BodyState b, float nearest)
	{
		if (m_fNow - b.m_fLodAt < 5)
			return;

		b.m_fLodAt = m_fNow;
		int lod = 6;
		if (nearest < m_aLodBands[0])
			lod = 0;
		else if (nearest < m_aLodBands[1])
			lod = 2;
		else if (nearest < m_aLodBands[2])
			lod = 4;

		if (lod == b.m_iLod)
			return;

		AIControlComponent ctrl = AIControlComponent.Cast(b.m_Body.FindComponent(AIControlComponent));
		if (!ctrl)
			return;

		AIAgent agent = ctrl.GetControlAIAgent();
		if (!agent)
			return;

		agent.SetPermanentLOD(lod);
		b.m_iLod = lod;
	}

	// ================================================================ Senses helpers
	//------------------------------------------------------------------------------------------------
	//! Line of sight from an infected's eye to a player (docs/INFECTED.md, Senses): the terrain in
	//! between (pure script), and within 80 m a physics trace for buildings and walls. Never through
	//! a hill or a house; false if the second's checks are used up.
	bool LineOfSight(IEntity from, IEntity to)
	{
		if (!from || !to || !TakeCheck())
			return false;

		vector a = from.GetOrigin();
		vector b = to.GetOrigin();
		if (RZ_InfectedBodies.TerrainBlocks(a, b))
			return false;

		if (RZ_Util.DistXZ(a, b) > 80)
			return true;

		TraceParam param = new TraceParam();
		param.Start = a + Vector(0, RZ_InfectedBodies.EYE, 0);
		param.End = b + Vector(0, RZ_InfectedBodies.EYE, 0);
		param.Exclude = from;
		param.LayerMask = EPhysicsLayerDefs.Projectile;
		param.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
		float hit = GetGame().GetWorld().TraceMove(param, null);
		if (hit >= 0.97)
			return true;

		// It stopped on the player's own body
		return param.TraceEnt == to;
	}

	//------------------------------------------------------------------------------------------------
	//! The direction a character faces, flat.
	static vector Facing(IEntity e)
	{
		vector mat[4];
		e.GetWorldTransform(mat);
		vector f = mat[2];
		f[1] = 0;
		if (f.Length() < 0.01)
			return Vector(0, 0, 1);

		f.Normalize();
		return f;
	}

	//------------------------------------------------------------------------------------------------
	//! Degrees between a's facing and the direction from a to b.
	static float AngleOff(IEntity a, vector b)
	{
		vector d = b - a.GetOrigin();
		d[1] = 0;
		if (d.Length() < 0.1)
			return 0;

		d.Normalize();
		float dot = Math.Clamp(vector.Dot(Facing(a), d), -1, 1);
		return Math.Acos(dot) * Math.RAD2DEG;
	}

	// ================================================================ Water and stuck marks
	//------------------------------------------------------------------------------------------------
	//! Water deep enough to wade at a point (0 dry), from the engine's water surface (sea, rivers
	//! and ponds; RZ_Util.IsWater sees only the sea).
	static float WaterDepth(vector pos)
	{
		vector ground = RZ_Util.GroundPos(pos);
		vector probe = Vector(ground[0], ground[1] + 0.2, ground[2]);
		vector surface;
		EWaterSurfaceType type;
		vector transform[4];
		vector extents;
		if (!ChimeraWorldUtils.TryGetWaterSurface(GetGame().GetWorld(), probe, surface, type, transform, extents))
			return 0;

		if (type == EWaterSurfaceType.WST_NONE)
			return 0;

		return Math.Max(0, surface[1] - ground[1]);
	}

	//------------------------------------------------------------------------------------------------
	//! The land grid says there's no water anywhere near a and b (both squares, and the one between).
	protected bool LandSaysDry(vector a, vector b)
	{
		RZ_Land land = m_GM.GetLand();
		if (!land || !land.IsReady())
			return false;

		array<vector> pts = {a, b, (a + b) * 0.5};
		foreach (vector p : pts)
		{
			int idx = land.CellAt(p);
			if (idx < 0)
				return false;

			if (land.SeaSamples(idx) > 0 || land.InlandSamples(idx) > 0 || land.HasBit(idx, RZ_Land.BIT_RIVER) || land.HasBit(idx, RZ_Land.BIT_LAKE))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The water line check (lab B16: the engine paths straight through a shallow river): a slot is
	//! pulled back toward `from` until neither it nor the line to it is wet.
	vector DrySlot(vector from, vector slot)
	{
		if (LandSaysDry(from, slot))
			return slot;

		float len = RZ_Util.DistXZ(from, slot);
		int steps = Math.ClampInt(len / 2, 1, 30);
		vector lastDry = from;
		for (int i = 1; i <= steps; i++)
		{
			float t = i;
			t = t / steps;
			vector p = from + (slot - from) * t;
			if (WaterDepth(p) > 0.05)
			{
				vector back = lastDry - from;
				back[1] = 0;
				if (back.Length() > 1.5)
				{
					back.Normalize();
					return RZ_Util.GroundPos(lastDry - back * 1.5);
				}

				return from;
			}

			lastDry = p;
		}

		return slot;
	}

	//------------------------------------------------------------------------------------------------
	//! A 10 m square where a body got stuck twice is avoided for 30 minutes (64 at most).
	void MarkStuck(vector pos)
	{
		for (int i = m_aStuckUntil.Count() - 1; i >= 0; i--)
		{
			if (m_fNow >= m_aStuckUntil[i])
			{
				m_aStuckMarks.Remove(i);
				m_aStuckUntil.Remove(i);
			}
		}

		m_aStuckMarks.Insert(pos);
		m_aStuckUntil.Insert(m_fNow + 1800);
		while (m_aStuckMarks.Count() > 64)
		{
			m_aStuckMarks.RemoveOrdered(0);
			m_aStuckUntil.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The slot moved off a stuck mark, sideways from `heading`.
	vector AvoidMarks(vector slot, vector heading)
	{
		int hits = 0;
		foreach (vector m : m_aStuckMarks)
		{
			if (RZ_Util.DistXZ(m, slot) < 7)
				hits++;
		}

		if (hits < 2)
			return slot;

		vector side = Vector(heading[2], 0, -heading[0]);
		return RZ_Util.GroundPos(slot + side * 10);
	}

	//------------------------------------------------------------------------------------------------
	//! No player within 60 m, and none with terrain line of sight within 150 m: a body may be
	//! moved here unseen (the unstick ladder's step 4).
	bool UnseenSpot(vector pos, array<IEntity> players)
	{
		foreach (IEntity p : players)
		{
			float d = RZ_Util.DistXZ(p.GetOrigin(), pos);
			if (d < 60)
				return false;

			if (d < 150 && !RZ_InfectedBodies.TerrainBlocks(p.GetOrigin(), pos))
				return false;
		}

		return true;
	}

	// ================================================================ Attacks
	//------------------------------------------------------------------------------------------------
	//! A hunter's lunge (docs/INFECTED.md, Attacks): from 6-10 m, out of the target's view (more than
	//! 60 degrees off their facing), a 1.5 s sprint into Bacon's swing. 20 s apart per hunter, and
	//! never in a player's gentle start.
	void TryLunge(RZ_BodyState b, IEntity target, int targetId)
	{
		if (!m_Settings.m_bOwnAttacks || b.m_eKind != RZ_EBodyKind.HUNTER || !target || m_fNow < b.m_fLungeReady)
			return;

		float d = RZ_Util.DistXZ(b.m_Body.GetOrigin(), target.GetOrigin());
		if (d < 6 || d > 10)
			return;

		if (AngleOff(target, b.m_Body.GetOrigin()) <= 60)
			return;

		RZ_Guidance guide = m_GM.GetGuidance();
		if (guide && guide.GetDensityMult(target.GetOrigin(), 5) < 1)
			return;

		b.m_fLungeReady = m_fNow + 20;
		b.m_fLungeAt = m_fNow;
		Sprint(b.m_Body, 1.5);
	}

	//------------------------------------------------------------------------------------------------
	//! A young's bursts in a chase: 2-4 s of sprint every 8-15 s.
	void TryBurst(RZ_BodyState b)
	{
		if (!Kind(b.m_eKind).m_bBursts || m_fNow < b.m_fBurstAt)
			return;

		b.m_fBurstAt = m_fNow + Math.RandomFloat(8, 15);
		Sprint(b.m_Body, Math.RandomFloat(2, 4));
	}

	//------------------------------------------------------------------------------------------------
	//! The bite detector saw this player hit (RZ_BiteDetector): if a hunter's lunge landed it (a body
	//! whose melee test passed within 1 s and 2.2 m, or the nearest within 2.5 m, that lunged in the
	//! last 3 s), the bite chance is x1.5 and a light bleed opens on an arm or a leg (once a player
	//! in 30 s). One special move a player in Special move gap. Returns the bite multiplier.
	float OnPlayerHit(int playerId, IEntity player)
	{
		if (!m_Settings.m_bOwnAttacks || !player)
			return 1;

		float now = RZ_BaconBody.Now();
		float last;
		if (m_mSpecialAt.Find(playerId, last) && now - last < m_Settings.m_fSpecialMoveGap)
			return 1;

		RZ_BodyState lunger = FindLunger(player.GetOrigin(), now);
		if (!lunger)
			return 1;

		m_mSpecialAt.Set(playerId, now);
		m_iMovesLanded++;
		float bled;
		if (!m_mBleedAt.Find(playerId, bled) || now - bled >= 30)
		{
			m_mBleedAt.Set(playerId, now);
			SCR_ChimeraCharacter c = SCR_ChimeraCharacter.Cast(player);
			if (c)
			{
				SCR_CharacterDamageManagerComponent dmg = SCR_CharacterDamageManagerComponent.Cast(c.GetDamageManager());
				array<string> zones = {"LArm", "RArm", "LLeg", "RLeg"};
				if (dmg)
					dmg.AddParticularBleeding(zones.GetRandomElement(), ECharacterDamageState.WOUNDED, 0.95); // VERIFY: a light bleed opened from the server on a player's limb replicates and bleeds at the usual rate
			}
		}

		return 1.5;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_BodyState FindLunger(vector at, float now)
	{
		array<ref RZ_TrackedZombie> tracked = m_GM.GetSpawner().GetTracked();
		RZ_BodyState best;
		float bestDist = 2.5;
		foreach (RZ_TrackedZombie z : tracked)
		{
			if (!z.m_Plan || RZ_Util.DistXZ(z.m_vLastPos, at) > 80)
				continue;

			foreach (RZ_BodyState b : z.m_Plan.GetBodies())
			{
				if (!b.m_Body || now - b.m_fLungeAt > 3)
					continue;

				float d = RZ_Util.DistXZ(b.m_Body.GetOrigin(), at);
				float swung = now - RZ_BaconBody.MeleeAt(b.m_Body);
				if (swung <= 1 && d <= 2.2)
					return b;

				if (d < bestDist)
				{
					bestDist = d;
					best = b;
				}
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! The young's leap, as the lab allows it (B14: a scripted jump goes up, not forward): a young
	//! running at 3 m/s or more with its target 3-6 m ahead (within 25 degrees) sprints the last of it
	//! into Bacon's swing. 15 s apart per young. The stagger needs a client and waits.
	void TryLeap(RZ_BodyState b, IEntity target)
	{
		if (!m_Settings.m_bOwnAttacks || b.m_eKind != RZ_EBodyKind.YOUNG || !target || m_fNow < b.m_fLeapReady)
			return;

		vector pos = b.m_Body.GetOrigin();
		vector to = target.GetOrigin() - pos;
		to[1] = 0;
		float d = to.Length();
		if (d < 3 || d > 6 || !b.m_Body.GetPhysics())
			return;

		vector vel = b.m_Body.GetPhysics().GetVelocity();
		vel[1] = 0;
		float speed = vel.Length();
		if (speed < 3)
			return;

		vel.Normalize();
		to.Normalize();
		if (Math.Acos(Math.Clamp(vector.Dot(vel, to), -1, 1)) * Math.RAD2DEG > 25)
			return;

		RZ_Guidance guide = m_GM.GetGuidance();
		if (guide && guide.GetDensityMult(target.GetOrigin(), 5) < 1)
			return;

		b.m_fLeapReady = m_fNow + 15;
		m_iLeaps++;
		Sprint(b.m_Body, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! The alpha's charge, as the lab allows it (B7: the sprint holds when set every 0.25 s; the
	//! knockdown needs a client and waits): its target 12-30 m away on foot, in sight, and never with
	//! 3 or more other infected within 10 m of them; a roar, then a straight sprint of up to 4 s. If
	//! it doesn't get within 1.6 m it overruns and stumbles (capped low 2.5 s): that's the window to
	//! hurt it. 30 s apart per alpha and per player; never in the gentle start.
	void TryCharge(RZ_BodyState b, IEntity target, int targetId, float d)
	{
		if (b.m_eKind != RZ_EBodyKind.ALPHA)
			return;

		// A charge running, or its stumble
		if (b.m_fChargeUntil > 0)
		{
			if (d <= 1.6)
			{
				b.m_fChargeUntil = -1;
				return;
			}

			if (m_fNow >= b.m_fChargeUntil)
			{
				b.m_fChargeUntil = -1;
				b.m_fStumbleUntil = m_fNow + 2.5;
				m_iStumbles++;
				SetCap(b, 0.2);
			}

			return;
		}

		if (b.m_fStumbleUntil > 0)
		{
			if (m_fNow >= b.m_fStumbleUntil)
			{
				b.m_fStumbleUntil = -1;
				SetCap(b, 1);
				WantMove(b.m_Body, EMovementType.RUN);
			}

			return;
		}

		if (!m_Settings.m_bOwnAttacks || !target || m_fNow < b.m_fChargeReady || d < 12 || d > 30 || RZ_Util.IsInVehicle(target))
			return;

		float last;
		if (m_mChargedAt.Find(targetId, last) && m_fNow - last < 30)
			return;

		RZ_Guidance guide = m_GM.GetGuidance();
		if (guide && guide.GetDensityMult(target.GetOrigin(), 5) < 1)
			return;

		// Never into a crowd: 3 or more others within 10 m of the player
		if (m_GM.GetSpawner().CountZombiesNear(target.GetOrigin(), 10, 3) >= 3)
			return;

		if (!LineOfSight(b.m_Body, target))
			return;

		b.m_fChargeReady = m_fNow + 30;
		b.m_fChargeUntil = m_fNow + 4;
		m_mChargedAt.Set(targetId, m_fNow);
		m_iCharges++;
		RZ_BaconBody.Say(b.m_Body, "SOUND_TARGET");
		Sprint(b.m_Body, 4);
	}

	//------------------------------------------------------------------------------------------------
	//! A bonded young (docs/INFECTED.md, Bonds): its other half was put down, so it runs reckless:
	//! released at once with a long sprint. False if the group has no young body the layer drives.
	bool Reckless(RZ_TrackedZombie z, vector at)
	{
		if (!IsOn() || !z || !z.m_Plan || !z.m_Plan.IsDriven())
			return false;

		foreach (RZ_BodyState b : z.m_Plan.GetBodies())
		{
			if (!b.m_Body || b.m_eKind != RZ_EBodyKind.YOUNG || RZ_Util.DistXZ(b.m_Body.GetOrigin(), at) > 120)
				continue;

			Release(b);
			Sprint(b.m_Body, 4);
			b.m_fBurstAt = m_fNow + 4;
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A body Bacon is fighting with within radius of pos (bait: the young have the target busy).
	bool EngagedNear(vector pos, float radius)
	{
		array<ref RZ_TrackedZombie> tracked = m_GM.GetSpawner().GetTracked();
		foreach (RZ_TrackedZombie z : tracked)
		{
			if (!z.m_Plan || RZ_Util.DistXZ(z.m_vLastPos, pos) > radius + 60)
				continue;

			foreach (RZ_BodyState b : z.m_Plan.GetBodies())
			{
				if (b.m_Body && b.m_eMode == RZ_EBodyMode.ENGAGE && RZ_Util.DistXZ(b.m_Body.GetOrigin(), pos) <= radius)
					return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	void CountHandOff()
	{
		m_iHandOffs++;
	}

	//------------------------------------------------------------------------------------------------
	//! A player fired (the game mode): steered groups get a lead (170 m x the kind, suppressed 30 m),
	//! and a steered body within 30 m of an unsuppressed shot is released at once.
	void OnShot(int playerId, vector at, bool suppressed)
	{
		m_Crawlers.OnShot(at, suppressed);
		if (!IsOn())
			return;

		float radius = m_Settings.m_fShotHearing;
		if (suppressed)
			radius = 30;

		array<ref RZ_TrackedZombie> tracked = m_GM.GetSpawner().GetTracked();
		foreach (RZ_TrackedZombie z : tracked)
		{
			if (!z.m_Plan || !z.m_Entity)
				continue;

			float d = RZ_Util.DistXZ(z.m_vLastPos, at);
			if (d > radius * 1.5 + 50)
				continue;

			z.m_Plan.OnShot(this, playerId, at, d, radius, suppressed);
		}
	}

	// ================================================================ Status
	//------------------------------------------------------------------------------------------------
	//! "bodies steer 12, engage 3, recover 1; hunts 2; releases 14, take-backs 5, unsticks 1, moves landed 0"
	string StatusText()
	{
		if (!IsOn())
			return "behaviour off";

		int steer = 0;
		int engage = 0;
		int recover = 0;
		int undriven = 0;
		array<ref RZ_TrackedZombie> tracked = m_GM.GetSpawner().GetTracked();
		foreach (RZ_TrackedZombie z : tracked)
		{
			if (!z.m_Plan)
				continue;

			if (!z.m_Plan.IsDriven())
			{
				undriven += z.Agents();
				continue;
			}

			foreach (RZ_BodyState b : z.m_Plan.GetBodies())
			{
				if (!b.m_Body)
					continue;

				switch (b.m_eMode)
				{
					case RZ_EBodyMode.STEER: steer++; break;
					case RZ_EBodyMode.ENGAGE: engage++; break;
					case RZ_EBodyMode.RECOVER: recover++; break;
				}
			}
		}

		string text = string.Format("behaviour: steer %1, engage %2, recover %3, by waypoint %4; hunts %5 (started %6)", steer, engage, recover, undriven, m_aLinks.Count(), m_iHuntsStarted);
		text += string.Format(", releases %1, take-backs %2, unsticks %3, moves landed %4, hand-offs %5, leaps %6, charges %7 (stumbles %8)", m_iReleases, m_iTakeBacks, m_iUnsticks, m_iMovesLanded, m_iHandOffs, m_iLeaps, m_iCharges, m_iStumbles);
		text += " | " + m_Crawlers.StatusText();
		return text;
	}

	//------------------------------------------------------------------------------------------------
	void CountUnstick()
	{
		m_iUnsticks++;
	}
}
