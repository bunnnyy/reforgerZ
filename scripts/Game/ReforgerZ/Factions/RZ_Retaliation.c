//! Retaliation (docs/REPUTATION.md, "What makes AI shoot you", "Shooting back"). Behind a
//! setting (Hostility > Retaliation, on by default).
//!
//! Shooting or killing a faction's AI makes that faction's AI near the spot hostile to the
//! shooter for a few minutes, through the per-player hostility bits (RZ_Hostility), plus a
//! little Infamy for a kill. It's one timed bit per player and faction: hitting them again
//! only restarts the timer, so it can't build up into lasting hostility on its own (only the
//! Infamy can, slowly, and that's capped per hour).
//!
//! Only AI ReforgerZ spawns itself is watched: outpost and stronghold guards and occupants,
//! supply truck crews and safehouse guards (RZ_Hostility.WatchUnit). Mission fights, hunter
//! squads and wanderers already have their own rules; leaders have theirs (RZ_Leaders).
//!
//! How a hit is noticed: each watched character's health is read on the server tick. When it
//! drops (or they die) and a player fired within the last few seconds within 500 m, the
//! nearest such player is the shooter. Not counted: an infected within 3 m of the character
//! (it was them), or within 40 m (the player was defending against them, the safehouse
//! house-rule exception), a mission fight or hunter squad within 350 m (a crossfire), or a
//! player the faction already shoots for another reason.

//------------------------------------------------------------------------------------------------
//! AI spawned by ReforgerZ for a faction (a group or a character), with its members' health.
class RZ_WatchedUnit
{
	IEntity m_Anchor;
	RZ_EFaction m_eFaction;
	float m_fAge;
	bool m_bComplete;	// every member has spawned (or we stopped waiting): no more are added
	ref array<IEntity> m_aMembers = {};
	ref array<float> m_aHealth = {};
	ref array<bool> m_aAlive = {};
}

//------------------------------------------------------------------------------------------------
//! One player's temporary trouble with one faction around one spot.
class RZ_RetaliationGrudge
{
	int m_iPlayerId;
	RZ_EFaction m_eFaction;
	vector m_vPos;
	float m_fRadius;
	float m_fUntil;
}

//------------------------------------------------------------------------------------------------
class RZ_Retaliation
{
	protected static const float SHOT_MEMORY = 8;			// seconds a player's shot can explain a hit
	protected static const float SHOOTER_RANGE = 500;
	protected static const float MELEE_RADIUS = 3;
	protected static const float DEFEND_RADIUS = 40;
	protected static const float MIN_DROP = 1;
	protected static const float EXPAND_GIVE_UP = 30;		// seconds to wait for a group's members
	protected static const float INFAMY_WINDOW = 3600;
	protected static const float FIGHT_RADIUS = 350;

	protected RZ_GameModeComponent m_GM;
	protected RZ_HostilitySettings m_Settings;
	protected ref array<ref RZ_WatchedUnit> m_aUnits = {};
	protected ref array<ref RZ_RetaliationGrudge> m_aGrudges = {};
	protected ref map<int, float> m_mShotAt = new map<int, float>();
	protected ref map<int, vector> m_mShotPos = new map<int, vector>();
	protected ref map<string, int> m_mInfamySum = new map<string, int>();
	protected ref map<string, float> m_mInfamyWindow = new map<string, float>();
	protected float m_fClock;

	//------------------------------------------------------------------------------------------------
	void RZ_Retaliation(RZ_GameModeComponent gm, RZ_HostilitySettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled()
	{
		return m_Settings.m_bRetaliation && m_Settings.m_eMode != RZ_EHostilityMode.OFF;
	}

	//------------------------------------------------------------------------------------------------
	void Watch(IEntity unit, RZ_EFaction faction)
	{
		if (!unit || !IsEnabled() || !RZ_Factions.IsValid(faction))
			return;

		RZ_WatchedUnit w = new RZ_WatchedUnit();
		w.m_Anchor = unit;
		w.m_eFaction = faction;
		m_aUnits.Insert(w);
	}

	//------------------------------------------------------------------------------------------------
	//! From the gunfire hook.
	void OnPlayerShot(int playerId, vector pos)
	{
		if (!IsEnabled())
			return;

		m_mShotAt.Set(playerId, m_fClock);
		m_mShotPos.Set(playerId, pos);
	}

	//------------------------------------------------------------------------------------------------
	//! baseMasks: each player's hostility bits without retaliation (so self-defence against a
	//! faction that already shoots them doesn't count again).
	void Tick(float dt, array<int> ids, array<IEntity> players, map<int, int> baseMasks)
	{
		m_fClock += dt;
		for (int g = m_aGrudges.Count() - 1; g >= 0; g--)
		{
			if (m_fClock >= m_aGrudges[g].m_fUntil)
				m_aGrudges.Remove(g);
		}

		if (!IsEnabled())
		{
			m_aUnits.Clear();
			return;
		}

		for (int i = m_aUnits.Count() - 1; i >= 0; i--)
		{
			RZ_WatchedUnit w = m_aUnits[i];
			w.m_fAge = w.m_fAge + dt;
			if (!w.m_Anchor)
			{
				// A group deletes itself when its last member dies: that death still counts
				if (!w.m_aMembers.IsEmpty())
					CheckUnit(w, ids, players, baseMasks);

				m_aUnits.Remove(i);
				continue;
			}

			// A group's members spawn over several frames: keep adding them until it's complete
			if (!w.m_bComplete)
			{
				Expand(w);
				if (RZ_Util.HasAllMembers(w.m_Anchor) || w.m_fAge > EXPAND_GIVE_UP)
					w.m_bComplete = true;
			}

			if (w.m_aMembers.IsEmpty())
			{
				if (w.m_bComplete)
					m_aUnits.Remove(i);

				continue;
			}

			CheckUnit(w, ids, players, baseMasks);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Adds the unit's members that aren't watched yet (members spawn one by one).
	protected void Expand(RZ_WatchedUnit w)
	{
		array<IEntity> found = {};
		RZ_Util.GetUnitMembers(w.m_Anchor, found);
		foreach (IEntity member : found)
		{
			if (!member || w.m_aMembers.Contains(member))
				continue;

			w.m_aMembers.Insert(member);
			w.m_aHealth.Insert(RZ_Util.GetCharacterHealth(member));
			w.m_aAlive.Insert(RZ_Util.IsCharacterAlive(member));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckUnit(RZ_WatchedUnit w, array<int> ids, array<IEntity> players, map<int, int> baseMasks)
	{
		foreach (int i, IEntity member : w.m_aMembers)
		{
			if (!member || !w.m_aAlive[i])
				continue;

			float hp = RZ_Util.GetCharacterHealth(member);
			bool alive = RZ_Util.IsCharacterAlive(member);
			bool hit = hp >= 0 && w.m_aHealth[i] >= 0 && hp < w.m_aHealth[i] - MIN_DROP;
			bool killed = !alive;
			w.m_aHealth[i] = hp;
			w.m_aAlive[i] = alive;
			if (!hit && !killed)
				continue;

			vector at = member.GetOrigin();
			if (m_GM.GetSpawner().IsZombieNear(at, DEFEND_RADIUS))
				continue; // the infected did it, or the player was fighting them off

			if (m_GM.GetHostility() && m_GM.GetHostility().IsFightNear(at, FIGHT_RADIUS))
				continue; // a mission fight or a hunter squad nearby: a crossfire, not a provocation

			int shooter = FindShooter(at, ids, players);
			if (shooter <= 0)
				continue;

			// A supply truck's crew: whoever shot them gets no Fame for driving the truck in
			if (m_GM.GetSupply())
				m_GM.GetSupply().OnCrewHit(w.m_Anchor, shooter);

			int baseMask;
			if (baseMasks && baseMasks.Find(shooter, baseMask) && (baseMask & RZ_Factions.Bit(w.m_eFaction)) != 0)
				continue; // they already shoot this player: it's a fight, not a provocation

			Trigger(shooter, w.m_eFaction, at);
			if (killed)
				KillInfamy(shooter, w.m_eFaction);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest player who fired in the last few seconds within range of the hit.
	protected int FindShooter(vector at, array<int> ids, array<IEntity> players)
	{
		int best = -1;
		float bestDist = SHOOTER_RANGE;
		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];
			float shotAt;
			if (!m_mShotAt.Find(playerId, shotAt) || m_fClock - shotAt > SHOT_MEMORY + m_GM.GetTickSeconds())
				continue;

			vector from;
			if (!m_mShotPos.Find(playerId, from))
				from = p.GetOrigin();

			float d = RZ_Util.DistXZ(from, at);
			if (d <= bestDist)
			{
				bestDist = d;
				best = playerId;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! One grudge per player and faction: a new hit moves it and restarts its timer, never adds.
	protected void Trigger(int playerId, RZ_EFaction f, vector at, bool tell = true)
	{
		float mult = 1;
		if (m_GM.GetFactionLife())
			mult = m_GM.GetFactionLife().JumpinessMult(f);

		float seconds = Math.Max(30, m_Settings.m_fRetaliationSeconds) * mult;
		float radius = Math.Max(50, m_Settings.m_fRetaliationRadius) * mult;
		foreach (RZ_RetaliationGrudge g : m_aGrudges)
		{
			if (g.m_iPlayerId == playerId && g.m_eFaction == f)
			{
				g.m_vPos = at;
				g.m_fRadius = radius;
				g.m_fUntil = m_fClock + seconds;
				return;
			}
		}

		RZ_RetaliationGrudge grudge = new RZ_RetaliationGrudge();
		grudge.m_iPlayerId = playerId;
		grudge.m_eFaction = f;
		grudge.m_vPos = at;
		grudge.m_fRadius = radius;
		grudge.m_fUntil = m_fClock + seconds;
		m_aGrudges.Insert(grudge);
		if (tell)
			m_GM.NotifyPlayer(playerId, RZ_Factions.GetName(f), RZ_FactionVoice.Retaliation(f));
	}

	//------------------------------------------------------------------------------------------------
	//! Temporary trouble from outside a shooting (a toll refused, a checkpoint run, an armed
	//! approach to a camp after a warning): the same timed grudge, so it never builds up.
	void Grudge(int playerId, RZ_EFaction f, vector at)
	{
		if (playerId > 0 && RZ_Factions.IsValid(f))
			Trigger(playerId, f, at, false); // the caller says why
	}

	//------------------------------------------------------------------------------------------------
	//! A surrender: the faction's grudge against this player goes.
	void ClearGrudge(int playerId, RZ_EFaction f)
	{
		for (int i = m_aGrudges.Count() - 1; i >= 0; i--)
		{
			if (m_aGrudges[i].m_iPlayerId == playerId && m_aGrudges[i].m_eFaction == f)
				m_aGrudges.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A little Infamy for a kill, at most the hourly cap per player and faction.
	protected void KillInfamy(int playerId, RZ_EFaction f)
	{
		// The kill tally counts every kill, capped Infamy or not (Core/RZ_Deeds.c)
		RZ_Deeds.AddKill(m_GM, playerId, f);
		int amount = m_Settings.m_iRetaliationKillInfamy;
		if (amount <= 0)
			return;

		string key = string.Format("%1:%2", m_GM.GetProfile(playerId).m_sIdentity, f);
		float window;
		if (!m_mInfamyWindow.Find(key, window) || m_fClock - window > INFAMY_WINDOW)
		{
			m_mInfamyWindow.Set(key, m_fClock);
			m_mInfamySum.Set(key, 0);
		}

		int used;
		m_mInfamySum.Find(key, used);
		int room = m_Settings.m_iRetaliationInfamyCap - used;
		if (room <= 0)
			return;

		int applied = Math.Min(amount, room);
		m_mInfamySum.Set(key, used + applied);
		m_GM.AddInfamy(playerId, f, applied);
	}

	//------------------------------------------------------------------------------------------------
	//! The retaliation bits for this player at pos (bit = RZ_Factions.Bit).
	int MaskFor(int playerId, vector pos)
	{
		int mask = 0;
		foreach (RZ_RetaliationGrudge g : m_aGrudges)
		{
			if (g.m_iPlayerId == playerId && RZ_Util.DistXZ(g.m_vPos, pos) <= g.m_fRadius)
				mask = mask | RZ_Factions.Bit(g.m_eFaction);
		}

		return mask;
	}

	//------------------------------------------------------------------------------------------------
	//! A respawn starts clean.
	void Forget(int playerId)
	{
		for (int i = m_aGrudges.Count() - 1; i >= 0; i--)
		{
			if (m_aGrudges[i].m_iPlayerId == playerId)
				m_aGrudges.Remove(i);
		}

		m_mShotAt.Remove(playerId);
		m_mShotPos.Remove(playerId);
	}
}
