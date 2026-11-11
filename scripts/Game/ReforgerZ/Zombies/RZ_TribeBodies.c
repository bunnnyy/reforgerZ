//! The tribes' bodies (docs/INFECTED.md, "Tribe bodies"): when and where a part gets bodies (near
//! players only, never in sight within 300 m), the orders each group's plan gets from the data (the
//! lair's ring, a day walk, a trail, the way home, a kill site, the vigil), the scout rule, a
//! scout's pack, and the night's hunt by placement with its commitment. Owned by RZ_Tribes. Server
//! only.
class RZ_TribeBodies
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_Tribes m_Tribes;		// weak: the owner
	protected ref array<vector> m_aPackAt = {};
	protected ref array<float> m_aPackTime = {};
	protected ref map<string, int> m_mSeen = new map<string, int>();	// "scout part:player": seconds seen running
	protected int m_iSpawned;
	protected int m_iHunts;
	protected int m_iBreakOffs;
	protected int m_iCalls;
	protected int m_iPacks;

	static const float LAIR_RANGE = 300;		// the lair's bodies within this of a player
	static const float SLEEP_RANGE = 150;		// during post-feed sleep
	static const float PART_RANGE = 600;		// scouts, day parties and hunting parts
	static const float SCOUT_CHECK = 200;
	static const int SCOUT_SECONDS = 5;
	static const float PACK_GAP = 600;			// s between packs at one player cluster
	static const float LEASH = 600;				// s at the target before a hunt breaks off
	static const float HIDE_LOST = 60;

	//------------------------------------------------------------------------------------------------
	void RZ_TribeBodies(RZ_GameModeComponent gm, RZ_Tribes tribes)
	{
		m_GM = gm;
		m_Tribes = tribes;
	}

	//------------------------------------------------------------------------------------------------
	protected float Clock()
	{
		return m_Tribes.GetClock();
	}

	// ================================================================ The tick
	//------------------------------------------------------------------------------------------------
	//! Every server tick: hunts, then each part's bodies (spawn, or orders for the ones it has).
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (t.m_Hunt)
				HuntTick(t, dt, players);

			MovePursuers(t, dt);
			TrackAlpha(t);
			foreach (RZ_TribePart p : t.m_aParts)
			{
				if (p.HasBody())
				{
					Orders(t, p);
					Noticed(t, p, players);
				}
				else if (!players.IsEmpty())
				{
					TrySpawn(t, p, players);
				}
			}

			NightTargets(t, ids, players);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Parts after a player as data (a hunt or a pack) close on them at the data speed.
	protected void MovePursuers(RZ_Tribe t, float dt)
	{
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (p.m_iTarget < 0 || p.HasBody())
				continue;

			IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(p.m_iTarget);
			if (!ent || !RZ_Util.IsCharacterAlive(ent) || RZ_Util.DistXZ(ent.GetOrigin(), p.m_vPos) > 2500)
			{
				p.m_iTarget = -1;
				p.m_bPack = false;
				m_Tribes.PathHome(t, p);
				continue;
			}

			if (p.m_bHeldBack)
				continue;

			p.m_vTarget = ent.GetOrigin();
			p.m_bHasTarget = true;
			p.MoveAsData(dt);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The alpha's health as data, and when it was last hurt (a wounded alpha isn't removed near players).
	protected void TrackAlpha(RZ_Tribe t)
	{
		IEntity body = t.AlphaBody();
		if (!body)
			return;

		float hp = HealthOf(body);
		if (hp < 0)
			return;

		if (hp < t.m_fAlphaHealth - 0.01)
			t.m_fAlphaHurtAt = Clock();

		t.m_fAlphaHealth = hp;
	}

	//------------------------------------------------------------------------------------------------
	//! A character's health, 0-1 (-1 without a damage manager).
	float HealthOf(IEntity e)
	{
		if (!e)
			return -1;

		ChimeraCharacter c = ChimeraCharacter.Cast(e);
		if (!c || !c.GetDamageManager())
			return -1;

		return c.GetDamageManager().GetHealthScaled();
	}

	// ================================================================ Spawning
	//------------------------------------------------------------------------------------------------
	//! How near a player a part gets bodies (0: never now). By day scouts and day parties within
	//! 600 m, the lair (the core and the resting ring) within 300 m (150 m in post-feed sleep); at
	//! night hunting parts within 600 m and the core within 300 m; a scout's pack at once.
	protected float Trigger(RZ_Tribe t, RZ_TribePart p)
	{
		RZ_ETribePhase phase = m_Tribes.GetPhase();
		if (p.m_bPack)
			return 5000;

		if (p.m_eKind == RZ_ETribePart.SCOUT)
		{
			if (t.m_bFed && phase == RZ_ETribePhase.DAY)
				return 0;

			return PART_RANGE;
		}

		if (p.m_iTarget >= 0)
		{
			if (p.m_bHeldBack)
				return 0;

			return PART_RANGE;
		}

		if (p.m_eKind == RZ_ETribePart.DAY)
			return PART_RANGE;

		if (p.m_bOut)
		{
			if (p.IsCore())
				return LAIR_RANGE;

			return PART_RANGE;
		}

		if (t.m_bFed)
			return SLEEP_RANGE;

		return LAIR_RANGE;
	}

	//------------------------------------------------------------------------------------------------
	//! A part near a player gets bodies, inside every cap: the agent budget and the spawns a tick,
	//! a tribe's presence at one player cluster (14, less when the server binds), day parties' 6
	//! bodies near a cluster, 3 alpha bodies on the server (never two within 1 km of one player);
	//! never within 400 m of a player in their gentle start (day parties); a weakened tribe hunts as
	//! data only.
	protected void TrySpawn(RZ_Tribe t, RZ_TribePart p, array<IEntity> players)
	{
		if (p.m_iCount <= 0)
			return;

		float trigger = Trigger(t, p);
		if (trigger <= 0)
			return;

		IEntity closest = NearestPlayer(p.m_vPos, players);
		if (!closest)
			return;

		vector nearPos = closest.GetOrigin();
		float d = RZ_Util.DistXZ(p.m_vPos, nearPos);
		if (d > trigger && !p.m_bPack)
			return;

		bool night = m_Tribes.GetPhase() == RZ_ETribePhase.NIGHT;
		if (night && t.IsWeakened() && p.m_bOut && p.IsHunting() && p.m_iTarget < 0)
			return;

		if (p.m_eKind == RZ_ETribePart.DAY && Gentle(p.m_vPos, 400))
			return;

		if (p.m_eKind == RZ_ETribePart.DAY && DayBodiesNear(nearPos) + p.m_iCount > m_Tribes.GetSettings().m_iDayBodies)
			return;

		if (p.m_eKind == RZ_ETribePart.ALPHA && !AlphaRoom(nearPos))
			return;

		RZ_ZombieSpawner spawner = m_GM.GetSpawner();
		int share = spawner.GetBudget().GetShare();
		int presence = Math.Max(3, RZ_Util.RoundToInt(14.0 * share / 28));
		if (TribeBodiesNear(t, nearPos, PART_RANGE) + p.m_iCount > presence)
			return;

		vector pos;
		if (!Placement(t, p, players, pos))
			return;

		if (spawner.SpawnRoom(pos) < p.m_iCount)
			return;

		RZ_ZombieSettings zs = spawner.GetSettings();
		RZ_TrackedZombie z = spawner.SpawnTribeUnit(pos, m_Tribes.PrefabsFor(p.m_eKind, zs), m_Tribes.YoungPrefabs(zs), p.m_iCount, p.m_iYoung, p.Role(), t.m_iId, p.BodyKind());
		if (!z)
			return;

		m_iSpawned++;
		z.m_Part = p;
		p.m_Body = z;
		p.m_vPos = pos;
		z.m_vHome = t.m_vLair;
		z.m_bHunting = p.m_iTarget >= 0 && !p.m_bPack;
		RZ_GroupPlan plan = new RZ_GroupPlan();
		plan.m_eKind = p.BodyKind();
		plan.m_iYoungFirst = p.m_iYoung;
		plan.m_iTemper = t.m_eTemper;
		plan.m_bRising = t.Stage() == 0;
		plan.m_bDay = !night;
		plan.m_bSenses = p.m_eKind != RZ_ETribePart.SCOUT;
		z.m_Plan = plan;
		if (p.m_eKind == RZ_ETribePart.ALPHA && t.m_fAlphaHealth < 1)
			SetHealth(RZ_Tribe.FirstMember(z.m_Entity), t.m_fAlphaHealth);

		if (p.m_iTarget >= 0)
			JoinHunt(t, p, plan);

		if (p.m_eKind == RZ_ETribePart.DAY && !p.m_bPack && d < 350 && Clock() - t.m_fToldBirdsAt > 3600)
		{
			t.m_fToldBirdsAt = Clock();
			m_Tribes.Tell(pos, 350, "Birds", "The crows went up off the hedge line and didn't come back down.");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Sets an alpha's health as it was when its body last went (phase 0: SetHealthScaled applies,
	//! though it returns false).
	protected void SetHealth(IEntity e, float health)
	{
		if (!e)
			return;

		ChimeraCharacter c = ChimeraCharacter.Cast(e);
		if (c && c.GetDamageManager())
			c.GetDamageManager().SetHealthScaled(Math.Clamp(health, 0.05, 1));
	}

	//------------------------------------------------------------------------------------------------
	protected bool Gentle(vector pos, float radius)
	{
		RZ_Guidance guide = m_GM.GetGuidance();
		return guide && guide.GetDensityMult(pos, radius) < 1;
	}

	//------------------------------------------------------------------------------------------------
	protected static IEntity NearestPlayer(vector pos, array<IEntity> players)
	{
		IEntity best;
		float bestDist = 1000000;
		foreach (IEntity p : players)
		{
			float d = RZ_Util.DistXZ(p.GetOrigin(), pos);
			if (d < bestDist)
			{
				bestDist = d;
				best = p;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Day-party bodies within 600 m of pos, every tribe's.
	protected int DayBodiesNear(vector pos)
	{
		int n = 0;
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			foreach (RZ_TribePart p : t.m_aParts)
			{
				if (p.m_eKind == RZ_ETribePart.DAY && p.HasBody() && RZ_Util.DistXZ(p.m_vPos, pos) <= PART_RANGE)
					n += p.m_iCount;
			}
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! One tribe's bodies within radius of pos.
	protected int TribeBodiesNear(RZ_Tribe t, vector pos, float radius)
	{
		int n = 0;
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (p.HasBody() && RZ_Util.DistXZ(p.m_vPos, pos) <= radius)
				n += p.m_iCount;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Room for another alpha's body: 3 on the server, never a second within 1 km of this player.
	protected bool AlphaRoom(vector player)
	{
		int n = 0;
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			IEntity body = t.AlphaBody();
			if (!body)
				continue;

			n++;
			if (RZ_Util.DistXZ(body.GetOrigin(), player) < 1000)
				return false;
		}

		return n < 3;
	}

	//------------------------------------------------------------------------------------------------
	//! Where a part's bodies appear: out of every player's sight within 300 m (phase 0: a body
	//! spawned in sight charges at once) and 120 m from everyone (x0.75 in fog). A hunt's parts by
	//! placement (the young ahead, flankers to the sides, the core back on the trail), a pack 150-200
	//! m out on the lair's side, everything else at its data spot or pushed back toward the lair.
	protected bool Placement(RZ_Tribe t, RZ_TribePart p, array<IEntity> players, out vector pos)
	{
		IEntity target;
		if (p.m_iTarget >= 0)
			target = GetGame().GetPlayerManager().GetPlayerControlledEntity(p.m_iTarget);

		if (target)
		{
			vector tp = target.GetOrigin();
			vector toward = Flat(p.m_vPos - tp);
			vector lairward = Flat(t.m_vLair - tp);
			for (int i = 0; i < 6; i++)
			{
				vector cand;
				if (p.m_bPack)
					cand = tp + Rotate(lairward, Math.RandomFloat(-30, 30)) * Math.RandomFloat(150, 200);
				else if (p.IsCore())
					cand = tp + Rotate(lairward, Math.RandomFloat(-20, 20)) * Math.RandomFloat(150, 250);
				else if (p.m_eKind == RZ_ETribePart.YOUNG)
					cand = tp + Rotate(toward, Math.RandomFloat(-20, 20)) * Math.RandomFloat(200, 260);
				else
					cand = tp + Rotate(toward, p.m_iSide * Math.RandomFloat(60, 90)) * Math.RandomFloat(130, 180);

				if (i >= 4)
					cand = tp + Rotate(toward, Math.RandomFloat(-90, 90)) * Math.RandomFloat(260, 320);

				cand = RZ_Util.GroundPos(cand);
				if (SpotOk(cand, players))
				{
					pos = cand;
					return true;
				}
			}

			return false;
		}

		vector spot = p.m_vPos;
		for (int k = 0; k < 8; k++)
		{
			if (SpotOk(spot, players))
			{
				pos = spot;
				return true;
			}

			// Pushed back toward the lair; at the lair, round to its far side
			vector back = t.m_vLair - spot;
			back[1] = 0;
			if (back.Length() > 50)
			{
				back.Normalize();
				spot = RZ_Util.GroundPos(spot + back * 40);
				continue;
			}

			IEntity who = NearestPlayer(t.m_vLair, players);
			vector away = Flat(t.m_vLair - who.GetOrigin());
			spot = RZ_Util.GroundPos(t.m_vLair + Rotate(away, Math.RandomFloat(-60, 60)) * Math.RandomFloat(20, 60));
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool SpotOk(vector pos, array<IEntity> players)
	{
		if (RZ_Util.IsWater(pos) || RZ_Behaviour.WaterDepth(pos) > 0.3)
			return false;

		float minDist = m_GM.GetSpawner().MinSpawnDistance(pos);
		if (RZ_Util.NearestDistance(pos, players) < minDist)
			return false;

		return !RZ_InfectedBodies.InSight(pos, players, false);
	}

	//------------------------------------------------------------------------------------------------
	protected static vector Flat(vector v)
	{
		vector f = v;
		f[1] = 0;
		if (f.Length() < 0.01)
			return RZ_Behaviour.RandomDir();

		f.Normalize();
		return f;
	}

	//------------------------------------------------------------------------------------------------
	protected static vector Rotate(vector v, float deg)
	{
		float a = deg * Math.DEG2RAD;
		return Vector(v[0] * Math.Cos(a) - v[2] * Math.Sin(a), 0, v[0] * Math.Sin(a) + v[2] * Math.Cos(a));
	}

	// ================================================================ Orders
	//------------------------------------------------------------------------------------------------
	//! A part with bodies: its plan's order from the data. A hunt or a pack leaves it to the plan.
	protected void Orders(RZ_Tribe t, RZ_TribePart p)
	{
		RZ_GroupPlan plan = p.m_Body.m_Plan;
		if (!plan)
			return;

		float now = Clock();
		RZ_ETribePhase phase = m_Tribes.GetPhase();
		plan.m_bDay = phase == RZ_ETribePhase.DAY;
		plan.m_bRising = t.Stage() == 0;
		if (plan.IsHunting())
			return;

		if (p.m_iTarget >= 0)
		{
			// Its hunt or pack is over for these bodies: home
			p.m_iTarget = -1;
			p.m_bPack = false;
			p.m_bHeldBack = false;
			p.m_Body.m_bHunting = false;
			m_Tribes.PathHome(t, p);
			plan.OrderHome(p.m_vRest);
			return;
		}

		if (p.m_eKind == RZ_ETribePart.GUARD)
			plan.m_KeepClear = t.AlphaBody();

		// Feeding: in order round the kill site (docs/INFECTED.md, Group behaviour)
		if (t.m_fFeedUntil > now && p.m_eKind != RZ_ETribePart.SCOUT && RZ_Util.DistXZ(p.m_vPos, t.m_vFeedAt) < 400)
		{
			FeedingRing(plan, p, t.m_vFeedAt);
			return;
		}

		// The vigil: round where the alpha fell, 30 real minutes at most
		if (t.m_bVigil && RZ_BaconBody.Now() < t.m_fVigilBodiesUntil && p.m_eKind != RZ_ETribePart.SCOUT)
		{
			plan.OrderHold(t.m_vVigilAt, 10, 30);
			return;
		}

		if (p.m_eKind == RZ_ETribePart.SCOUT)
		{
			plan.OrderHold(p.m_vRest, 0, 2);
			return;
		}

		if (now < p.m_fNoiseUntil)
		{
			plan.OrderWalk(p.m_vNoise, RZ_EGait.WALKSTOP, RZ_EFormation.LINE);
			return;
		}

		if (!p.m_bOut)
		{
			LairRing(plan, p, t);
			return;
		}

		if (now < p.m_fHoldUntil || !p.m_bHasTarget)
		{
			if (plan.GetOrder() != RZ_EPlanOrder.HOLD)
				plan.OrderHold(plan.Centre(), 2, 6);

			return;
		}

		RZ_EGait gait = RZ_EGait.MARCH;
		RZ_EFormation formation = RZ_EFormation.FILE;
		if (p.m_eKind == RZ_ETribePart.DAY || p.m_bHome)
		{
			gait = RZ_EGait.WALKSTOP;
			formation = RZ_EFormation.LINE;
		}
		else if (p.m_eKind == RZ_ETribePart.YOUNG)
		{
			formation = RZ_EFormation.LINE;
		}
		else if (p.IsCore())
		{
			formation = RZ_EFormation.WEDGE;
		}

		plan.OrderWalk(p.m_vTarget, gait, formation);
	}

	//------------------------------------------------------------------------------------------------
	//! The lair by day: the alpha in the middle, the guard 10-25 m round it, the resting ring 20-60 m
	//! out. They turn to no noise.
	protected void LairRing(RZ_GroupPlan plan, RZ_TribePart p, RZ_Tribe t)
	{
		plan.m_bDeaf = m_Tribes.GetPhase() == RZ_ETribePhase.DAY;
		switch (p.m_eKind)
		{
			case RZ_ETribePart.ALPHA: plan.OrderHold(t.m_vLair, 0, 3); return;
			case RZ_ETribePart.GUARD: plan.OrderHold(t.m_vLair, 10, 25); return;
		}

		plan.OrderHold(t.m_vLair, 20, 60);
	}

	//------------------------------------------------------------------------------------------------
	//! Feeding order: the alpha at the site, the guard 8-15 m out, the hunters 20-40 m, the young
	//! 40-70 m. Walk up and you meet the young first.
	protected void FeedingRing(RZ_GroupPlan plan, RZ_TribePart p, vector site)
	{
		switch (p.m_eKind)
		{
			case RZ_ETribePart.ALPHA: plan.OrderHold(site, 0, 2); return;
			case RZ_ETribePart.GUARD: plan.OrderHold(site, 8, 15); return;
			case RZ_ETribePart.YOUNG: plan.OrderHold(site, 40, 70); return;
		}

		plan.OrderHold(site, 20, 40);
	}

	//------------------------------------------------------------------------------------------------
	//! Someone was taken: the tribe's parts out within 1 km walk to the kill site and feed there.
	void Feed(RZ_Tribe t)
	{
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (p.m_eKind == RZ_ETribePart.SCOUT || p.m_iTarget >= 0 || RZ_Util.DistXZ(p.m_vPos, t.m_vFeedAt) > 1000)
				continue;

			array<vector> path = {p.m_vPos, RZ_Util.GroundPos(RZ_Util.RandomPointInRing(t.m_vFeedAt, 5, 40))};
			p.SetPath(path);
			p.m_iStep = 1;
			p.m_vTarget = path[1];
			p.m_bOut = true;
			p.m_bHome = false;
			p.m_fHoldUntil = Clock();
		}
	}

	// ================================================================ Noticing
	//------------------------------------------------------------------------------------------------
	//! A group noticed a player by itself (its notice meter filled). At night that's the tribe's hunt
	//! (if it may hunt them). By day a day party or a pack hunts them on its own (rush, stalk, or a
	//! pincer with another group near), never bringing the alpha or the guard; at the lair (a player
	//! within 100 m) everyone there fights, the alpha too.
	protected void Noticed(RZ_Tribe t, RZ_TribePart p, array<IEntity> players)
	{
		RZ_GroupPlan plan = p.m_Body.m_Plan;
		if (!plan)
			return;

		int id = plan.TakeNoticed();
		if (id < 0)
			return;

		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(id);
		if (!ent || Gentle(ent.GetOrigin(), 50))
			return;

		RZ_ETribePhase phase = m_Tribes.GetPhase();
		if (phase == RZ_ETribePhase.NIGHT && p.m_bOut)
		{
			StartHunt(t, id, false);
			return;
		}

		bool atLair = RZ_Util.DistXZ(ent.GetOrigin(), t.m_vLair) <= 100;
		array<RZ_GroupPlan> plans = {};
		int bodies = 0;
		foreach (RZ_TribePart q : t.m_aParts)
		{
			if (!q.HasBody() || !q.m_Body.m_Plan || q.m_Body.m_Plan.IsHunting() || q.m_eKind == RZ_ETribePart.SCOUT)
				continue;

			if (RZ_Util.DistXZ(q.m_vPos, p.m_vPos) > 300)
				continue;

			// By day the alpha and the guard come only at the lair
			if (q.IsCore() && !atLair)
				continue;

			if (!q.m_bOut && !atLair && q != p)
				continue;

			plans.Insert(q.m_Body.m_Plan);
			bodies += q.m_iCount;
		}

		if (plans.IsEmpty())
			return;

		RZ_EStrategy strategy = RZ_EStrategy.NONE;
		if (atLair)
			strategy = RZ_EStrategy.RUSH;

		RZ_HuntLink.Start(plans, id, strategy, t.m_eTemper, t.Stage() == 0, true, false);
		if (atLair)
			RZ_Log.Info(string.Format("Tribes: someone walked into the lair at %1.", t.m_sPlace));
	}

	// ================================================================ The night's hunt
	//------------------------------------------------------------------------------------------------
	//! A hunt gets a target (docs/INFECTED.md, How a hunt comes for you): a scout called on them, they
	//! made 30 heat at night inside its range, or they're on its trail as it passes. Never a player in
	//! their gentle start, in no man's land, or in a cluster a hunt broke off from tonight; never
	//! while the tribe is weakened, at its vigil, or with a hunt on someone else (it never switches).
	//! force: an admin's test, any time.
	bool StartHunt(RZ_Tribe t, int playerId, bool force)
	{
		if (t.m_Hunt)
			return t.m_Hunt.m_iTarget == playerId;

		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!ent || !RZ_Util.IsCharacterAlive(ent))
			return false;

		vector at = ent.GetOrigin();
		if (!force)
		{
			if (m_Tribes.GetPhase() != RZ_ETribePhase.NIGHT || !t.m_bHunted || t.IsWeakened() || t.m_bVigil)
				return false;

			if (Gentle(at, 50) || m_GM.GetLand().InNoMansLand(at))
				return false;

			foreach (vector blocked : t.m_aBlocked)
			{
				if (RZ_Util.DistXZ(blocked, at) < 600)
					return false;
			}
		}

		RZ_TribeHunt hunt = new RZ_TribeHunt();
		hunt.m_iTarget = playerId;
		hunt.m_fStartedAt = Clock();
		hunt.m_iSetOut = Math.Max(1, t.Members());
		hunt.m_eTemper = t.m_eTemper;
		hunt.m_iWaves = 1;
		t.m_Hunt = hunt;

		// The first wave: up to 3 young and hunter groups up to 8 (less when the server binds)
		int share = m_GM.GetSpawner().GetBudget().GetShare();
		int cap = Math.Max(3, RZ_Util.RoundToInt(8.0 * share / 28));
		int used = 0;
		int groups = 0;
		RZ_TribePart young = t.FindPart(RZ_ETribePart.YOUNG);
		if (young && young.m_iCount <= cap)
		{
			Target(t, young, playerId, 1);
			used += young.m_iCount;
		}

		while (groups < 2)
		{
			RZ_TribePart next = NearestFree(t, at);
			if (!next || used + next.m_iCount > cap)
				break;

			Target(t, next, playerId, 1);
			next.m_iSide = 1;
			if (groups == 1)
				next.m_iSide = -1;

			used += next.m_iCount;
			groups++;
		}

		hunt.m_eStrategy = RZ_HuntLink.Choose(t.m_eTemper, groups, used, false, t.Stage() == 0);
		// The core, held back until the first wave is down to 2 (a bold alpha comes at once)
		foreach (RZ_TribePart core : t.m_aParts)
		{
			if (!core.IsCore() || core.m_iCount <= 0 || (core.m_eKind == RZ_ETribePart.ALPHA && t.m_fAlphaHealth < 0.5))
				continue;

			if (t.m_eTemper == RZ_ETemperament.CAUTIOUS && t.Band() < 2)
				continue;

			Target(t, core, playerId, 1);
			core.m_bHeldBack = t.m_eTemper != RZ_ETemperament.BOLD;
		}

		m_iHunts++;
		RZ_Log.Info(string.Format("Tribes: %1 hunts player %2 (%3 in the first wave, %4).", t.m_sPlace, playerId, used, typename.EnumToString(RZ_EStrategy, hunt.m_eStrategy)));
		return used > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest hunter group not in the hunt yet.
	protected RZ_TribePart NearestFree(RZ_Tribe t, vector at)
	{
		RZ_TribePart best;
		float bestDist = 100000;
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (p.m_iTarget >= 0 || p.m_iCount <= 0 || (p.m_eKind != RZ_ETribePart.HUNTERS && p.m_eKind != RZ_ETribePart.DAY))
				continue;

			float d = RZ_Util.DistXZ(p.m_vPos, at);
			if (d < bestDist)
			{
				bestDist = d;
				best = p;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! A part joins a hunt: as data it closes on the target; with bodies its plan joins at once.
	protected void Target(RZ_Tribe t, RZ_TribePart p, int playerId, int wave)
	{
		p.m_iTarget = playerId;
		p.m_iWave = wave;
		p.m_bOut = true;
		p.m_bHome = false;
		p.m_fNoiseUntil = 0;
		if (!p.HasBody())
			return;

		p.m_Body.m_bHunting = true;
		if (p.m_Body.m_Plan)
			JoinHunt(t, p, p.m_Body.m_Plan);
	}

	//------------------------------------------------------------------------------------------------
	//! A part's plan joins the hunt's link for its kind: the young rush, the hunters by the hunt's
	//! strategy (a pincer's sides), the core rushes. Links on the night's trail are relentless (they
	//! end by the hide rule and the commitment, not by stealth). A pack rushes its seen player.
	protected void JoinHunt(RZ_Tribe t, RZ_TribePart p, RZ_GroupPlan plan)
	{
		bool day = m_Tribes.GetPhase() == RZ_ETribePhase.DAY;
		array<RZ_GroupPlan> one = {plan};
		if (p.m_bPack || !t.m_Hunt)
		{
			RZ_HuntLink.Start(one, p.m_iTarget, RZ_EStrategy.RUSH, t.m_eTemper, t.Stage() == 0, day, false);
			return;
		}

		RZ_TribeHunt hunt = t.m_Hunt;
		if (p.m_eKind == RZ_ETribePart.YOUNG)
		{
			if (hunt.m_Young && hunt.m_Young.IsAlive())
				plan.JoinHunt(hunt.m_Young, hunt.m_Young.CountPlans(), hunt.m_Young.CountPlans() + 1);
			else
				hunt.m_Young = RZ_HuntLink.Start(one, p.m_iTarget, RZ_EStrategy.RUSH, t.m_eTemper, false, false, true);

			return;
		}

		if (p.IsCore())
		{
			if (hunt.m_Core && hunt.m_Core.IsAlive())
				plan.JoinHunt(hunt.m_Core, hunt.m_Core.CountPlans(), hunt.m_Core.CountPlans() + 1);
			else
				hunt.m_Core = RZ_HuntLink.Start(one, p.m_iTarget, RZ_EStrategy.RUSH, t.m_eTemper, false, false, true);

			return;
		}

		if (hunt.m_Hunters && hunt.m_Hunters.IsAlive())
		{
			hunt.m_iFlankers++;
			plan.JoinHunt(hunt.m_Hunters, hunt.m_iFlankers, hunt.m_iFlankers + 1);
			return;
		}

		hunt.m_iFlankers = 0;
		hunt.m_Hunters = RZ_HuntLink.Start(one, p.m_iTarget, hunt.m_eStrategy, t.m_eTemper, false, false, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Each tick of a hunt: the core comes once the first wave is down to 2; the leash (10 minutes
	//! at the target); the hide rule (lost 60 s and 300 m away, or in no man's land: it goes home);
	//! the commitment (a third of what it set out with lost, a quarter for a cautious tribe and half
	//! for a bold one; two waves spent, one for a cautious tribe): one long call and home.
	protected void HuntTick(RZ_Tribe t, float dt, array<IEntity> players)
	{
		RZ_TribeHunt hunt = t.m_Hunt;
		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(hunt.m_iTarget);
		if (!ent || !RZ_Util.IsCharacterAlive(ent))
		{
			EndHunt(t, false);
			return;
		}

		vector at = ent.GetOrigin();
		float now = Clock();
		int wave = 0;
		float nearest = 100000;
		bool anyBody = false;
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (p.m_iTarget != hunt.m_iTarget)
				continue;

			float d = RZ_Util.DistXZ(p.m_vPos, at);
			nearest = Math.Min(nearest, d);
			if (!p.IsCore())
				wave += p.m_iCount;

			if (p.HasBody())
			{
				anyBody = true;
				if (d <= 150 && hunt.m_fLeashAt < 0)
					hunt.m_fLeashAt = now;
			}
		}

		// Seconds since any of its groups last saw the target (the behaviour layer's clock)
		float lostFor = 0;
		RZ_Behaviour bh = RZ_Behaviour.Get();
		array<RZ_HuntLink> links = {hunt.m_Hunters, hunt.m_Young, hunt.m_Core};
		bool anyLink = false;
		foreach (RZ_HuntLink link : links)
		{
			if (!link || !bh)
				continue;

			float since = bh.Now() - link.m_fLastSeenAt;
			if (!anyLink || since < lostFor)
				lostFor = since;

			anyLink = true;
		}

		// The core comes up
		if (wave <= 2 && anyBody)
		{
			foreach (RZ_TribePart core : t.m_aParts)
			{
				if (core.m_iTarget == hunt.m_iTarget && core.m_bHeldBack)
					core.m_bHeldBack = false;
			}
		}

		// The hide rule
		bool far = nearest >= 300;
		if (anyBody && anyLink && lostFor > HIDE_LOST && (far || m_GM.GetLand().InNoMansLand(at)))
		{
			RZ_Log.Info(string.Format("Tribes: %1 lost player %2 and goes home.", t.m_sPlace, hunt.m_iTarget));
			EndHunt(t, false);
			return;
		}

		if (m_GM.GetLand().InNoMansLand(at) && nearest > 200)
		{
			EndHunt(t, false);
			return;
		}

		// The commitment
		float loss = 0.34;
		int maxWaves = 2;
		if (t.m_eTemper == RZ_ETemperament.CAUTIOUS)
		{
			loss = 0.25;
			maxWaves = 1;
		}
		else if (t.m_eTemper == RZ_ETemperament.BOLD)
		{
			loss = 0.5;
		}

		bool broke = hunt.m_iLost >= Math.Max(2, hunt.m_iSetOut * loss);
		if (hunt.m_fLeashAt > 0 && now - hunt.m_fLeashAt >= LEASH)
			broke = true;

		if (!broke && anyBody && wave <= 2)
		{
			if (hunt.m_iWaves >= maxWaves)
			{
				broke = true;
			}
			else
			{
				RZ_TribePart more = NearestFree(t, at);
				if (more)
				{
					hunt.m_iWaves++;
					more.m_iSide = 1 - 2 * (hunt.m_iWaves % 2);
					Target(t, more, hunt.m_iTarget, hunt.m_iWaves);
				}
				else
				{
					broke = true;
				}
			}
		}

		if (broke)
			BreakOff(t);
	}

	//------------------------------------------------------------------------------------------------
	//! The long call: the hunt breaks off, every group beyond 80 m of a player goes home (groups in a
	//! fight are Bacon's until they win, die or are taken back), and the tribe won't come for that
	//! cluster again tonight.
	protected void BreakOff(RZ_Tribe t)
	{
		RZ_TribeHunt hunt = t.m_Hunt;
		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(hunt.m_iTarget);
		if (ent)
			t.m_aBlocked.Insert(ent.GetOrigin());

		m_iBreakOffs++;
		m_Tribes.Tell(t.m_vLair, RZ_Tribes.LONG_CALL_RANGE, "In the dark", string.Format("One long call from %1, and then they pull back.", t.m_sPlace));
		if (ent)
			m_Tribes.Tell(ent.GetOrigin(), RZ_Tribes.LONG_CALL_RANGE, "In the dark", string.Format("One long call from %1, and then they pull back.", t.m_sPlace));

		RZ_Log.Info(string.Format("Tribes: %1 breaks off (lost %2 of %3, waves %4).", t.m_sPlace, hunt.m_iLost, hunt.m_iSetOut, hunt.m_iWaves));
		EndHunt(t, true);
	}

	//------------------------------------------------------------------------------------------------
	//! The hunt is over: its parts go home (their plans give the player up for 10 minutes).
	void EndHunt(RZ_Tribe t, bool broke)
	{
		RZ_TribeHunt hunt = t.m_Hunt;
		if (!hunt)
			return;

		array<RZ_HuntLink> links = {hunt.m_Hunters, hunt.m_Young, hunt.m_Core};
		foreach (RZ_HuntLink link : links)
		{
			if (link)
				link.m_bOver = true;
		}

		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (p.m_iTarget != hunt.m_iTarget || p.m_bPack)
				continue;

			p.m_iTarget = -1;
			p.m_bHeldBack = false;
			if (p.HasBody())
			{
				p.m_Body.m_bHunting = false;
				if (p.m_Body.m_Plan)
					p.m_Body.m_Plan.OrderHome(p.m_vRest);
			}

			if (broke || m_Tribes.GetPhase() != RZ_ETribePhase.NIGHT)
				m_Tribes.PathHome(t, p);
			else if (!p.m_aPath.IsEmpty())
				p.m_bHasTarget = true;
		}

		t.m_Hunt = null;
	}

	//------------------------------------------------------------------------------------------------
	//! A tribe body died: a hunt counts its losses.
	void OnLoss(RZ_Tribe t, RZ_TribePart p)
	{
		if (t.m_Hunt && p && p.m_iTarget == t.m_Hunt.m_iTarget)
			t.m_Hunt.m_iLost = t.m_Hunt.m_iLost + 1;
	}

	//------------------------------------------------------------------------------------------------
	//! At night, out on its trails: a player who makes 30 heat inside its range, or who's within 120 m
	//! of one of its groups as it passes, becomes its hunt's target.
	protected void NightTargets(RZ_Tribe t, array<int> ids, array<IEntity> players)
	{
		if (t.m_Hunt || !t.m_bHunted || m_Tribes.GetPhase() != RZ_ETribePhase.NIGHT || t.IsWeakened() || t.m_bVigil)
			return;

		RZ_HeatMap heat = m_GM.GetHeat();
		foreach (int i, IEntity p : players)
		{
			vector at = p.GetOrigin();
			if (RZ_Util.DistXZ(at, t.m_vLair) <= t.HuntRadius() && heat.GetHeat(at) >= 30)
			{
				if (StartHunt(t, ids[i], false))
					return;
			}

			foreach (RZ_TribePart part : t.m_aParts)
			{
				if (part.m_bOut && part.IsHunting() && RZ_Util.DistXZ(part.m_vPos, at) <= 120)
				{
					if (StartHunt(t, ids[i], false))
						return;
				}
			}
		}
	}

	// ================================================================ Scouts
	//------------------------------------------------------------------------------------------------
	//! The scout rule (docs/INFECTED.md, The scout rule), once a second for posted scouts with a
	//! player within 200 m: in stealth's sight radius (Scout sight, x1.5 in a vehicle, x0.75 for a
	//! Hunter) and in terrain line of sight (and within 80 m nothing in between) for 5 checks
	//! running, it calls. Kill it inside those 5 seconds and nothing comes. A Hunter gets a warning
	//! at the first check.
	void ScoutRule(array<int> ids, array<IEntity> players)
	{
		RZ_Behaviour bh = RZ_Behaviour.Get();
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			foreach (RZ_TribePart p : t.m_aParts)
			{
				if (p.m_eKind != RZ_ETribePart.SCOUT || !p.HasBody())
					continue;

				IEntity scout = Leader(p.m_Body);
				if (!scout)
					continue;

				vector sp = scout.GetOrigin();
				foreach (int i, IEntity pl : players)
				{
					int id = ids[i];
					string key = string.Format("%1:%2", p.m_iId, id);
					float d = RZ_Util.DistXZ(sp, pl.GetOrigin());
					if (d > SCOUT_CHECK || Gentle(pl.GetOrigin(), 5))
					{
						m_mSeen.Remove(key);
						continue;
					}

					float radius = RZ_Stealth.SightRadius(id, m_Tribes.GetSettings().m_fScoutSight);
					if (RZ_Util.IsInVehicle(pl))
						radius = radius * 1.5;

					RZ_Background bg = m_GM.GetPlayerBackground(id);
					bool hunter = bg && bg.m_sId == "hunter";
					if (hunter)
						radius = radius * 0.75;

					bool inSight = d <= radius && !RZ_InfectedBodies.TerrainBlocks(sp, pl.GetOrigin());
					if (inSight && d <= 80 && bh)
						inSight = bh.LineOfSight(scout, pl);

					if (!inSight)
					{
						m_mSeen.Remove(key);
						continue;
					}

					int run = 0;
					m_mSeen.Find(key, run);
					run++;
					m_mSeen.Set(key, run);
					if (run == 1 && hunter)
						m_GM.NotifyPlayer(id, "Birds", "The birds just went up off the ridge.");

					if (run >= SCOUT_SECONDS)
					{
						m_mSeen.Remove(key);
						ScoutCall(t, p, scout, id, pl.GetOrigin());
					}
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static IEntity Leader(RZ_TrackedZombie z)
	{
		array<IEntity> members = {};
		RZ_Util.GetUnitMembers(z.m_Entity, members);
		foreach (IEntity m : members)
		{
			if (RZ_Util.IsCharacterAlive(m))
				return m;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The call (one a tribe every 10 minutes): +20 heat at the scout, a line within 600 m, +10 stir
	//! in towns within 600 m, the scout comes for you, and the pack: by day at most 4 (the nearest day
	//! party, topped up from the resting ring; never the alpha or a guard; 2 day packs a tribe a game
	//! day), at dusk and in fog the nearest hunter group (and a bold Prime alpha with its guard if the
	//! lair is within 600 m), at night the hunt. A weakened tribe's scouts bring nobody. A pack at one
	//! player cluster at most every 10 minutes.
	protected void ScoutCall(RZ_Tribe t, RZ_TribePart scoutPart, IEntity scout, int playerId, vector at)
	{
		float now = Clock();
		if (now - t.m_fLastCall < 600)
			return;

		t.m_fLastCall = now;
		m_iCalls++;
		vector sp = scout.GetOrigin();
		m_GM.GetHeat().AddHeat(sp, 20);
		m_Tribes.Tell(at, 600, "A shriek", string.Format("A shriek from the ridge, and an answer from %1.", t.m_sPlace));
		int idx = m_GM.FindNearestSettlement(sp);
		RZ_Settlement s = m_GM.GetSettlement(idx);
		RZ_Crawlers crawlers = m_GM.GetSpawner().GetCrawlers();
		if (s && crawlers && crawlers.IsEnabled() && RZ_Util.DistXZ(s.m_vCenter, sp) <= s.m_fRadius + 600)
			crawlers.AddStir(sp, 10);

		if (scoutPart.m_Body.m_Plan)
			scoutPart.m_Body.m_Plan.ReleaseAll();

		RZ_Log.Info(string.Format("Tribes: a scout of %1 called on player %2.", t.m_sPlace, playerId));
		if (t.IsWeakened())
			return;

		foreach (int i, vector packAt : m_aPackAt)
		{
			if (now - m_aPackTime[i] < PACK_GAP && RZ_Util.DistXZ(packAt, at) < 600)
				return;
		}

		RZ_ETribePhase phase = m_Tribes.GetPhase();
		if (phase == RZ_ETribePhase.NIGHT)
		{
			StartHunt(t, playerId, false);
			return;
		}

		m_aPackAt.Insert(at);
		m_aPackTime.Insert(now);
		while (m_aPackAt.Count() > 16)
		{
			m_aPackAt.RemoveOrdered(0);
			m_aPackTime.RemoveOrdered(0);
		}

		m_iPacks++;
		if (m_Tribes.DuskLike())
		{
			DuskPack(t, playerId, at);
			return;
		}

		DayPack(t, playerId, at);
	}

	//------------------------------------------------------------------------------------------------
	//! By day: at most 4. Day-party bodies already near get the order; the rest from the resting ring.
	protected void DayPack(RZ_Tribe t, int playerId, vector at)
	{
		if (t.m_iDayPacks >= 2)
			return;

		t.m_iDayPacks++;
		int n = 0;
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (n >= 4)
				break;

			if (p.m_eKind != RZ_ETribePart.DAY || p.m_iTarget >= 0 || RZ_Util.DistXZ(p.m_vPos, at) > PART_RANGE)
				continue;

			p.m_bPack = true;
			p.m_iTarget = playerId;
			n += p.m_iCount;
			if (p.HasBody() && p.m_Body.m_Plan)
				JoinHunt(t, p, p.m_Body.m_Plan);
		}

		int more = 0;
		if (n < 4)
		{
			foreach (RZ_TribePart rest : t.m_aParts)
			{
				if (n + more >= 4)
					break;

				if (rest.m_eKind != RZ_ETribePart.HUNTERS || rest.HasBody() || rest.m_bOut || rest.m_iCount - rest.m_iYoung <= 0)
					continue;

				int take = Math.Min(4 - n - more, rest.m_iCount - rest.m_iYoung);
				rest.m_iCount = rest.m_iCount - take;
				more += take;
			}
		}

		if (more <= 0)
			return;

		RZ_TribePart pack = m_Tribes.AddPart(t, RZ_ETribePart.DAY, more, 0);
		pack.m_vPos = t.m_vLair;
		pack.m_vRest = t.m_vLair;
		pack.m_bPack = true;
		pack.m_bOut = true;
		pack.m_iTarget = playerId;
	}

	//------------------------------------------------------------------------------------------------
	//! At dusk and in fog: the nearest hunter group, and a bold Prime alpha with its guard if the lair
	//! is within 600 m.
	protected void DuskPack(RZ_Tribe t, int playerId, vector at)
	{
		RZ_TribePart group = NearestFree(t, at);
		if (group)
		{
			group.m_bPack = true;
			group.m_iTarget = playerId;
			group.m_bOut = true;
			if (group.HasBody() && group.m_Body.m_Plan)
				JoinHunt(t, group, group.m_Body.m_Plan);
		}

		if (t.m_eTemper != RZ_ETemperament.BOLD || t.Stage() != 1 || RZ_Util.DistXZ(t.m_vLair, at) > 600)
			return;

		foreach (RZ_TribePart core : t.m_aParts)
		{
			if (!core.IsCore() || core.m_iTarget >= 0)
				continue;

			core.m_bPack = true;
			core.m_iTarget = playerId;
			core.m_bOut = true;
			if (core.HasBody() && core.m_Body.m_Plan)
				JoinHunt(t, core, core.m_Body.m_Plan);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "pack bodies spawned 14, hunts 2, break-offs 1, scout calls 3, packs 1"
	string StatusText()
	{
		return string.Format("pack bodies spawned %1, hunts %2, break-offs %3, scout calls %4, packs %5", m_iSpawned, m_iHunts, m_iBreakOffs, m_iCalls, m_iPacks);
	}
}
