//! The crawlers' daily life (docs/INFECTED.md, "Crawler life"): bodies that behave the built
//! placement. By day a town's crawlers lie up at their doorways, asleep with their AI off beyond 40 m
//! of a player (lab B12), and wake to a player or a noise near them, with a groan; at dusk they
//! stand in herds round their doorways; at night they drift the streets in slow herds after an
//! anchor walking a loop round the town (toward the noise when the town is stirred, aside when a
//! pack passes through). Only their walking slots and their sleep: their senses and their fight
//! stay Bacon's, and a drifting, walking or stirred group, or one with a player near, is left alone.
//! Run once a second by RZ_Behaviour. Server only.

//------------------------------------------------------------------------------------------------
//! A town's herd anchor at night: a slow loop between points round its streets.
class RZ_TownHerd
{
	int m_iTown;
	ref array<vector> m_aLoop = {};
	int m_iAt;
	vector m_vAnchor;
}

//------------------------------------------------------------------------------------------------
class RZ_CrawlerLife
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_Behaviour m_Behaviour;		// weak: the owner
	protected ref array<ref RZ_TownHerd> m_aHerds = {};
	protected ref array<vector> m_aShots = {};
	protected ref array<float> m_aShotAt = {};
	protected ref RZ_SpeedSource m_CapKey = new RZ_SpeedSource();
	protected int m_iWoken;
	protected int m_iHerded;
	protected int m_iSlept;

	static const float WAKE_RADIUS = 40;		// a player this close wakes a lying group
	static const float LEAVE_RADIUS = 150;		// a player this close: Bacon's own, no herd slots
	static const float ANCHOR_SPEED = 0.5;		// m/s the night herd's anchor walks
	static const float HEAR_SHOT = 255;			// crawlers hear well: the shot radius x1.5

	//------------------------------------------------------------------------------------------------
	void RZ_CrawlerLife(RZ_GameModeComponent gm, RZ_Behaviour behaviour)
	{
		m_GM = gm;
		m_Behaviour = behaviour;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsOn()
	{
		RZ_Crawlers crawlers = m_GM.GetSpawner().GetCrawlers();
		return RZ_Behaviour.IsOn() && m_Behaviour.GetSettings().m_bCrawlerLife && crawlers && crawlers.IsEnabled();
	}

	//------------------------------------------------------------------------------------------------
	//! A shot was fired: crawlers lying within 255 m of it wake (unsuppressed only).
	void OnShot(vector at, bool suppressed)
	{
		if (suppressed)
			return;

		float now = RZ_BaconBody.Now();
		m_aShots.Insert(at);
		m_aShotAt.Insert(now);
		while (m_aShots.Count() > 16 || (!m_aShotAt.IsEmpty() && now - m_aShotAt[0] > 30))
		{
			m_aShots.RemoveOrdered(0);
			m_aShotAt.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! 0 day, 1 dusk, 2 night (with no game clock, always day).
	protected static int Phase()
	{
		float h = RZ_GameTime.GetHour();
		if (h < 0)
			return 0;

		if (h >= 6 && h < 18.5)
			return 0;

		if (h >= 18.5 && h < 20.5)
			return 1;

		return 2;
	}

	// ================================================================ The tick
	//------------------------------------------------------------------------------------------------
	void Tick(array<IEntity> players)
	{
		if (!IsOn())
		{
			WakeAll();
			return;
		}

		int phase = Phase();
		float now = RZ_BaconBody.Now();
		MoveAnchors(phase);
		float cap = m_Behaviour.GetSettings().m_fCrawlerSpeedCap;
		array<ref RZ_TrackedZombie> tracked = m_GM.GetSpawner().GetTracked();
		RZ_Crawlers crawlers = m_GM.GetSpawner().GetCrawlers();
		foreach (RZ_TrackedZombie z : tracked)
		{
			if (z.m_eRole != RZ_EInfectedRole.CRAWLER || !z.m_Entity || z.m_iMissionId >= 0)
				continue;

			if (cap > 0 && !z.m_bCapped)
				Cap(z, cap);

			float nearest = RZ_Util.NearestDistance(z.m_vLastPos, players);
			RZ_Settlement s = m_GM.GetSettlement(z.m_iSettlement);
			bool stirred = s && crawlers.IsStirred(s);
			bool busy = z.m_bDrifting || z.m_bWalking || stirred;

			// Asleep: woken by a player within 40 m, a shot or a noise in hearing, the night, or stir
			if (z.m_bAsleep)
			{
				if (phase != 0 || busy || nearest <= WAKE_RADIUS || Heard(z.m_vLastPos, now))
					Wake(z, now);

				continue;
			}

			if (busy || nearest <= LEAVE_RADIUS)
			{
				ClearSlots(z);
				continue;
			}

			if (phase == 0)
			{
				ClearSlots(z);
				if (m_Behaviour.GetSettings().m_bSleepLying && nearest > WAKE_RADIUS * 2 && now - z.m_fWokeAt > 60 && !Heard(z.m_vLastPos, now))
					LieUp(z);

				continue;
			}

			// Dusk: a herd round its doorway; night: round the town's anchor, a step at a time
			vector centre = z.m_vHome;
			float blob = 6;
			if (phase == 2 && s)
			{
				RZ_TownHerd herd = HerdFor(z.m_iSettlement, s);
				vector offset = (z.m_vHome - s.m_vCenter) * 0.35;
				offset[1] = 0;
				centre = herd.m_vAnchor + offset;
				blob = 8;
			}

			Herd(z, centre, blob);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool Heard(vector pos, float now)
	{
		foreach (int i, vector at : m_aShots)
		{
			if (now - m_aShotAt[i] <= 30 && RZ_Util.DistXZ(at, pos) <= HEAR_SHOT)
				return true;
		}

		vector noise;
		return m_GM.GetHeat().FindRecentNoise(pos, 300, 60, noise);
	}

	// ================================================================ Sleep
	//------------------------------------------------------------------------------------------------
	//! Lying up: low (our dynamic stance), then the AI off (L14). Lab B12: a prone stance change
	//! didn't hold, so they're hunched where they stand, not lying flat.
	protected void LieUp(RZ_TrackedZombie z)
	{
		array<IEntity> members = {};
		RZ_Util.GetUnitMembers(z.m_Entity, members);
		foreach (IEntity m : members)
		{
			RZ_BaconBody.SetStance(m, 0.6);
			SetAI(m, false);
		}

		z.m_bAsleep = true;
		m_iSlept++;
	}

	//------------------------------------------------------------------------------------------------
	//! Waking: the AI on, standing up over its first moments, one groan (the player's warning).
	protected void Wake(RZ_TrackedZombie z, float now)
	{
		z.m_bAsleep = false;
		z.m_fWokeAt = now;
		m_iWoken++;
		array<IEntity> members = {};
		RZ_Util.GetUnitMembers(z.m_Entity, members);
		bool groaned = false;
		foreach (IEntity m : members)
		{
			SetAI(m, true);
			RZ_BaconBody.SetStance(m, 0);
			if (!groaned)
			{
				groaned = true;
				RZ_BaconBody.Say(m, "SOUND_IDLE");
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every sleeping group wakes (crawler life switched off, or the layer).
	protected void WakeAll()
	{
		if (m_iSlept == m_iWoken)
			return;

		float now = RZ_BaconBody.Now();
		foreach (RZ_TrackedZombie z : m_GM.GetSpawner().GetTracked())
		{
			if (z.m_bAsleep && z.m_Entity)
				Wake(z, now);
		}

		m_iWoken = m_iSlept;
	}

	//------------------------------------------------------------------------------------------------
	//! L14: AIAgent.DeactivateAI / ActivateAI. VERIFY: a deactivated agent stays in its group's agent
	//! list (the spawner counts living members by the character, so it isn't taken for dead either way)
	static void SetAI(IEntity body, bool on)
	{
		if (!body)
			return;

		AIControlComponent ctrl = AIControlComponent.Cast(body.FindComponent(AIControlComponent));
		if (!ctrl)
			return;

		AIAgent agent = ctrl.GetControlAIAgent();
		if (!agent)
			return;

		if (on)
			agent.ActivateAI();
		else
			agent.DeactivateAI();
	}

	// ================================================================ Herds
	//------------------------------------------------------------------------------------------------
	//! Each member walks to a slot round centre (blob m across), never more than 20 m from where it
	//! stands, so Bacon's Roam walks it there (under 25 m it walks; further it runs, lab B7).
	protected void Herd(RZ_TrackedZombie z, vector centre, float blob)
	{
		array<IEntity> members = {};
		RZ_Util.GetUnitMembers(z.m_Entity, members);
		foreach (int i, IEntity m : members)
		{
			if (RZ_BaconBody.GetMode(m) != 0)
				continue;

			float a = (i + 1) * 2.4;
			vector slot = centre + Vector(Math.Sin(a), 0, Math.Cos(a)) * (blob * 0.5 + i);
			vector pos = m.GetOrigin();
			vector step = slot - pos;
			step[1] = 0;
			float len = step.Length();
			if (len > 20)
			{
				step.Normalize();
				slot = pos + step * 20;
			}

			slot = RZ_Util.GroundPos(slot);
			if (RZ_Behaviour.WaterDepth(slot) > 0.05)
				continue;

			RZ_BaconBody.SetSlot(m, slot);
		}

		if (!z.m_bHerdSlots)
			m_iHerded++;

		z.m_bHerdSlots = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearSlots(RZ_TrackedZombie z)
	{
		if (!z.m_bHerdSlots)
			return;

		z.m_bHerdSlots = false;
		array<IEntity> members = {};
		RZ_Util.GetUnitMembers(z.m_Entity, members);
		foreach (IEntity m : members)
		{
			if (RZ_BaconBody.GetMode(m) == 0)
				RZ_BaconBody.ClearSlot(m);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The crawler speed cap (Zombies > Behaviour > Crawler speed cap), once a group.
	protected void Cap(RZ_TrackedZombie z, float cap)
	{
		z.m_bCapped = true;
		array<IEntity> members = {};
		RZ_Util.GetUnitMembers(z.m_Entity, members);
		foreach (IEntity m : members)
		{
			SCR_ChimeraCharacter c = SCR_ChimeraCharacter.Cast(m);
			if (c)
				c.SetSpeedLimit(m_CapKey, Math.Clamp(cap, 0.15, 1), true);
		}
	}

	// ================================================================ Night anchors
	//------------------------------------------------------------------------------------------------
	protected RZ_TownHerd HerdFor(int town, RZ_Settlement s)
	{
		foreach (RZ_TownHerd h : m_aHerds)
		{
			if (h.m_iTown == town)
				return h;
		}

		RZ_TownHerd herd = new RZ_TownHerd();
		herd.m_iTown = town;
		int n = Math.RandomIntInclusive(3, 4);
		for (int i = 0; i < n; i++)
		{
			float a = i * Math.PI2 / n + Math.RandomFloat(-0.4, 0.4);
			herd.m_aLoop.Insert(RZ_Util.GroundPos(s.m_vCenter + Vector(Math.Sin(a), 0, Math.Cos(a)) * (s.m_fRadius * Math.RandomFloat(0.3, 0.6))));
		}

		herd.m_vAnchor = herd.m_aLoop[0];
		m_aHerds.Insert(herd);
		return herd;
	}

	//------------------------------------------------------------------------------------------------
	//! Each town's anchor walks its loop at 0.5 m/s at night; a stirred town's toward its last noise;
	//! with a pack's group out in the town, 60 m aside from it (they give way).
	protected void MoveAnchors(int phase)
	{
		if (phase != 2)
		{
			m_aHerds.Clear();
			return;
		}

		RZ_Crawlers crawlers = m_GM.GetSpawner().GetCrawlers();
		foreach (RZ_TownHerd h : m_aHerds)
		{
			RZ_Settlement s = m_GM.GetSettlement(h.m_iTown);
			if (!s)
				continue;

			vector goal = h.m_aLoop[h.m_iAt];
			vector noise;
			if (crawlers.IsStirred(s) && m_GM.GetHeat().FindRecentNoise(s.m_vCenter, s.m_fRadius + 300, 300, noise))
				goal = noise;

			vector pack;
			if (PackInTown(s, pack))
			{
				vector aside = h.m_vAnchor - pack;
				aside[1] = 0;
				if (aside.Length() < 1)
					aside = RZ_Behaviour.RandomDir();

				aside.Normalize();
				goal = RZ_Util.GroundPos(pack + aside * 60);
			}

			vector dir = goal - h.m_vAnchor;
			dir[1] = 0;
			float len = dir.Length();
			if (len < 3)
			{
				h.m_iAt = (h.m_iAt + 1) % h.m_aLoop.Count();
				continue;
			}

			dir.Normalize();
			h.m_vAnchor = RZ_Util.GroundPos(h.m_vAnchor + dir * Math.Min(len, ANCHOR_SPEED));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A tribe's hunting group out inside the town (its radius + 100 m): where.
	protected bool PackInTown(RZ_Settlement s, out vector at)
	{
		RZ_Tribes tribes = m_GM.GetTribes();
		if (!tribes || !tribes.IsEnabled())
			return false;

		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			foreach (RZ_TribePart p : t.m_aParts)
			{
				if (p.m_bOut && p.IsHunting() && RZ_Util.DistXZ(p.m_vPos, s.m_vCenter) <= s.m_fRadius + 100)
				{
					at = p.m_vPos;
					return true;
				}
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! "town life: asleep 12 agents, slept 20, woken 30, herded 8"
	string StatusText()
	{
		int asleep = 0;
		foreach (RZ_TrackedZombie z : m_GM.GetSpawner().GetTracked())
		{
			if (z.m_bAsleep)
				asleep += z.Agents();
		}

		return string.Format("town life: asleep %1 agents, slept %2, woken %3, herded %4", asleep, m_iSlept, m_iWoken, m_iHerded);
	}
}
