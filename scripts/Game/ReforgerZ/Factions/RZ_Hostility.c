//! Per-player AI hostility (docs/REPUTATION.md, "What makes AI shoot you").
//!
//! Reforger decides hostility per faction, and every player and every friendly civilian
//! AI is on SURV. So each player's *character* (never the player controller: the deploy
//! menu, chat, groups and map markers stay on SURV) is moved onto a hidden copy of SURV
//! whose key says which AI factions want that player dead:
//!   SURV_H1..SURV_H15   bit 0 Haven, 1 Bastion, 2 Red Hand, 3 Halcyon (RZ_Factions.KeyForMask)
//!   SURV_OUTLAW         the fallback mode: hostile to every AI faction
//! The modded SCR_Faction check (RZ_FactionRelations.c) turns the key into real hostility.
//!
//! What sets a bit, per faction (RZ_HostilitySettings):
//!   - standing: Hunted or Kill on Sight by default (Watched, strangers or never per setting)
//!   - the Haven only while an armed exile is near a safehouse (unarmed exiles get the shut door)
//!   - fights: while a mission's hostile group is alive, players near it (raids, raiders,
//!     the Witness camp) and the one player a hunter squad was sent after
//!   - wanderers: near an armed AI-group wanderer who holds a grudge (the Haven bit, since
//!     wanderer bodies are on SURV)
//!   - retaliation (setting, on by default; RZ_Retaliation): for a few minutes after shooting
//!     or killing a faction's AI, near the spot
//! A disguise (RZ_Disguise) clears a faction's standing bit while the player wears its uniform
//! and isn't Hunted by it; fights and retaliation still set it.
//! Standing is cached and recomputed only when a tier changes or the player spawns; the
//! nearness checks are a few distance tests on the server tick. The character's faction
//! is only touched when the wanted key differs from the one it has.

enum RZ_EHostilityMode
{
	BITMASK,	// SURV_H1..SURV_H15: hostile only to the factions that want you
	OUTLAW,		// fallback: SURV_OUTLAW, hostile to every AI faction, for anyone with any bit
	OFF			// vanilla: AI never singles a player out
}

//! From which standing a faction's AI shoots a player.
enum RZ_EHostileFrom
{
	HUNTED,		// Hunted or Kill on Sight (default for all four)
	WATCHED,	// Watched or worse
	STRANGERS,	// anyone below Known (Stranger, Uneasy, Watched, Hunted, Kill on Sight)
	NEVER		// only fights (missions, hunter squads) and, for the Haven, wanderer grudges
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Factions > Hostility.
[BaseContainerProps()]
class RZ_HostilitySettings
{
	[Attribute("0", UIWidgets.ComboBox, "Bitmask: hidden SURV_H1..SURV_H15 factions, each hostile only to the AI factions that want the player. Outlaw (fallback): one SURV_OUTLAW faction, hostile to every AI faction, for anyone wanted by anyone. Off: vanilla", "", ParamEnumArray.FromEnum(RZ_EHostilityMode))]
	RZ_EHostilityMode m_eMode;

	[Attribute("0", UIWidgets.ComboBox, "Haven AI (guards, occupants, wanderers) shoots from this standing. See Exile: armed only", "", ParamEnumArray.FromEnum(RZ_EHostileFrom))]
	RZ_EHostileFrom m_eHavenFrom;

	[Attribute("0", UIWidgets.ComboBox, "Bastion AI (checkpoints, patrols, hunter squads) shoots from this standing. Default: wary, only the Hunted", "", ParamEnumArray.FromEnum(RZ_EHostileFrom))]
	RZ_EHostileFrom m_eBastionFrom;

	[Attribute("0", UIWidgets.ComboBox, "Red Hand AI shoots from this standing. Default: neutral, only the Hunted (plus fights). Strangers = their roadblocks shoot anyone below Known", "", ParamEnumArray.FromEnum(RZ_EHostileFrom))]
	RZ_EHostileFrom m_eRedHandFrom;

	[Attribute("0", UIWidgets.ComboBox, "Halcyon AI (lab guards) shoots from this standing. Default: peaceful, only the Hunted", "", ParamEnumArray.FromEnum(RZ_EHostileFrom))]
	RZ_EHostileFrom m_eHalcyonFrom;

	[Attribute("1", desc: "Exile: armed only. The Haven only shoots a player it wants dead while they carry a firearm near a safehouse. Off = everywhere, armed or not")]
	bool m_bHavenArmedOnly;

	[Attribute("100", desc: "Exile: how close to a safehouse (m) an armed exile is shot at")]
	float m_fExileRadius;

	[Attribute("350", desc: "Fights: players this close (m) to a mission's hostile group (raids, raiders, the Witness camp) are hostile to its faction while it lives")]
	float m_fFightRadius;

	[Attribute("1", desc: "Wanderers fight back: an armed wanderer AI group shoots players who robbed or shot at them (and the Deserter shoots Bastion's Champions)")]
	bool m_bWanderersFightBack;

	[Attribute("80", desc: "Wanderers fight back within this distance (m)")]
	float m_fGrudgeRadius;

	[Attribute("1", desc: "Retaliation: shooting or killing a faction's AI (outpost guards, truck crews, safehouse guards) makes its AI near the spot hostile to the shooter for a few minutes. Never permanent by itself")]
	bool m_bRetaliation;

	[Attribute("300", desc: "Retaliation: how far from the spot (m) the faction's AI turns on the shooter")]
	float m_fRetaliationRadius;

	[Attribute("180", desc: "Retaliation: seconds it lasts after the last hit (a new hit restarts it, never adds up)")]
	float m_fRetaliationSeconds;

	[Attribute("3", desc: "Retaliation: Infamy with the faction for killing one of its AI")]
	int m_iRetaliationKillInfamy;

	[Attribute("12", desc: "Retaliation: at most this much kill Infamy per player and faction per hour")]
	int m_iRetaliationInfamyCap;

	[Attribute("1", desc: "Disguises: wearing enough of a faction's uniform makes its AI look past you while you're not Hunted by it; Infamy with it doubles if you're caught (docs/REPUTATION.md)")]
	bool m_bDisguises;

	[Attribute("us_army,usarmy,bdu,m81,ocp,woodland_us", desc: "Disguises: keywords of Bastion uniform pieces (prefab names, lower case). Check them against your Bastion loadouts")]
	string m_sBastionUniformWords;

	[Attribute("bandit,redhand,red_hand,armband_red", desc: "Disguises: keywords of Red Hand clothing (e.g. a red armband or bandana)")]
	string m_sRedHandUniformWords;

	[Attribute("hazmat,labcoat,lab_coat,cbrn,scientist", desc: "Disguises: keywords of Halcyon clothing")]
	string m_sHalcyonUniformWords;

	[Attribute("2", desc: "Disguises: pieces of one faction's uniform needed (jacket and trousers, say)")]
	int m_iDisguiseItems;

	[Attribute("10", desc: "Disguises: seconds between checks of what each player wears")]
	float m_fDisguiseCheckSeconds;

	[Attribute("15", desc: "Disguises: real minutes a blown disguise stops working")]
	float m_fDisguiseBlownMinutes;

	[Attribute("MIL", desc: "FactionManager key of Bastion's AI")]
	string m_sBastionKey;

	[Attribute("BAND", desc: "FactionManager key of the Red Hand's AI")]
	string m_sRedHandKey;

	[Attribute("INST", desc: "FactionManager key of Halcyon's AI")]
	string m_sHalcyonKey;

	//------------------------------------------------------------------------------------------------
	RZ_EHostileFrom GetFrom(RZ_EFaction f)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return m_eHavenFrom;
			case RZ_EFaction.MILITARY: return m_eBastionFrom;
			case RZ_EFaction.BANDITS: return m_eRedHandFrom;
			case RZ_EFaction.INSTITUTE: return m_eHalcyonFrom;
		}

		return RZ_EHostileFrom.NEVER;
	}
}

//------------------------------------------------------------------------------------------------
//! A fight: while the anchor (an AI group or character) is alive, players within the radius
//! are hostile to the faction. Only one player if m_iPlayerId > 0 (a hunter squad's target).
class RZ_HostileZone
{
	RZ_EFaction m_eFaction;
	IEntity m_Anchor;
	vector m_vPos;
	float m_fRadius;
	int m_iPlayerId = -1;
	bool m_bSpareFriends;		// players Known or better with the faction are left alone
}

//------------------------------------------------------------------------------------------------
class RZ_Hostility
{
	//! Standing-mask flag: the Haven wants them, but only shoots while they're armed near a safehouse.
	protected static const int HAVEN_IF_ARMED = 16;
	protected static const float WARN_COOLDOWN = 120;
	protected static const float EXILE_SPAWN_GRACE = 60;	// an exile who spawns at a safehouse has this long to walk away

	protected RZ_GameModeComponent m_GM;
	protected RZ_HostilitySettings m_Settings;
	protected float m_fClock;
	protected ref map<int, int> m_mStanding = new map<int, int>();	// playerId -> standing bits (+ HAVEN_IF_ARMED), cached
	protected ref map<int, int> m_mMask = new map<int, int>();		// playerId -> last full mask
	protected ref map<int, int> m_mBaseMask = new map<int, int>();	// playerId -> last mask without retaliation
	protected ref RZ_Retaliation m_Retaliation;
	protected ref RZ_Disguise m_Disguise;
	protected ref map<int, float> m_mWarnedAt = new map<int, float>();
	protected ref map<int, float> m_mSpawnedAt = new map<int, float>();
	protected ref array<ref RZ_HostileZone> m_aZones = {};
	protected ref set<string> m_aMissing = new set<string>();
	protected ref map<string, float> m_mStandDown = new map<string, float>();	// "player:faction" -> clock until (a surrender)

	//------------------------------------------------------------------------------------------------
	void RZ_Hostility(RZ_GameModeComponent gm, RZ_HostilitySettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		m_Retaliation = new RZ_Retaliation(gm, settings);
		m_Disguise = new RZ_Disguise(gm, settings);
	}

	//------------------------------------------------------------------------------------------------
	//! Disguises (RZ_Disguise): who wears whose uniform.
	RZ_Disguise GetDisguise()
	{
		return m_Disguise;
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_GameModeComponent.AddInfamy: caught in a faction's uniform doubles the Infamy.
	int ScaleInfamyForDisguise(int playerId, RZ_EFaction faction, int amount)
	{
		return m_Disguise.ScaleInfamy(playerId, faction, amount);
	}

	//------------------------------------------------------------------------------------------------
	//! Retaliation: AI that ReforgerZ spawned for a faction (outpost guards and occupants,
	//! truck crews, safehouse guards). Shooting it makes its faction's AI nearby turn on you.
	void WatchUnit(IEntity unit, RZ_EFaction faction)
	{
		m_Retaliation.Watch(unit, faction);
	}

	//------------------------------------------------------------------------------------------------
	//! From the gunfire hook (retaliation needs to know who fired, and when).
	void OnPlayerShot(int playerId, vector pos)
	{
		m_Retaliation.OnPlayerShot(playerId, pos);
	}

	//------------------------------------------------------------------------------------------------
	//! Timed hostility from a faction's AI around `at` (RZ_Checkpoints: a refused toll, a run
	//! checkpoint, an armed approach to a camp after a warning). Like retaliation: it restarts,
	//! never adds up.
	void AddGrudge(int playerId, RZ_EFaction faction, vector at)
	{
		m_Retaliation.Grudge(playerId, faction, at);
	}

	//------------------------------------------------------------------------------------------------
	//! The faction's AI holds its fire on this player for `seconds`, whatever their standing, fights
	//! or grudges say (RZ_Checkpoints: walking up unarmed to surrender, then the surrender itself).
	//! With forgive, their grudges and fights with it are dropped for good.
	void StandDown(int playerId, RZ_EFaction faction, float seconds, bool forgive)
	{
		if (playerId <= 0 || !RZ_Factions.IsValid(faction))
			return;

		string key = string.Format("%1:%2", playerId, faction);
		float until;
		if (!m_mStandDown.Find(key, until) || until < m_fClock + seconds)
			m_mStandDown.Set(key, m_fClock + seconds);

		if (!forgive)
			return;

		m_Retaliation.ClearGrudge(playerId, faction);
		for (int i = m_aZones.Count() - 1; i >= 0; i--)
		{
			if (m_aZones[i].m_iPlayerId == playerId && m_aZones[i].m_eFaction == faction)
				m_aZones.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Bits of the factions standing down for this player now.
	protected int StandDownMask(int playerId)
	{
		if (m_mStandDown.IsEmpty())
			return 0;

		int mask = 0;
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			float until;
			if (m_mStandDown.Find(string.Format("%1:%2", playerId, f), until) && m_fClock < until)
				mask = mask | RZ_Factions.Bit(f);
		}

		return mask;
	}

	//------------------------------------------------------------------------------------------------
	//! Logs which of the hidden factions the FactionManager has.
	void Init()
	{
		if (m_Settings.m_eMode == RZ_EHostilityMode.OFF)
		{
			RZ_Log.Info("Hostility: off. AI never singles a player out.");
			return;
		}

		if (m_Settings.m_eMode == RZ_EHostilityMode.OUTLAW)
		{
			if (RZ_Util.HasFaction(RZ_Factions.OutlawKey()))
				RZ_Log.Info("Hostility: outlaw mode (" + RZ_Factions.OutlawKey() + ").");
			else
				RZ_Log.Warn("Hostility: outlaw mode, but the FactionManager has no " + RZ_Factions.OutlawKey() + ". Wanted players stay on " + RZ_Factions.GetPlayerKey() + ".");
			return;
		}

		int found = 0;
		for (int mask = 1; mask <= RZ_Factions.ALL_BITS; mask++)
		{
			if (RZ_Util.HasFaction(RZ_Factions.KeyForMask(mask)))
				found++;
		}

		if (found == RZ_Factions.ALL_BITS)
			RZ_Log.Info(string.Format("Hostility: bitmask mode, all %1 variant factions found.", found));
		else
			RZ_Log.Warn(string.Format("Hostility: bitmask mode, only %1 of %2 variant factions (%3 to %4) in the FactionManager. Players whose variant is missing stay on %5.",
				found, RZ_Factions.ALL_BITS, RZ_Factions.KeyForMask(1), RZ_Factions.KeyForMask(RZ_Factions.ALL_BITS), RZ_Factions.GetPlayerKey()));
	}

	//------------------------------------------------------------------------------------------------
	float GetFightRadius()
	{
		return m_Settings.m_fFightRadius;
	}

	//------------------------------------------------------------------------------------------------
	//! A tier changed (Hunted, Kill on Sight, exile, a bounty...): recompute on the next tick.
	void OnStandingChanged(int playerId)
	{
		m_mStanding.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! A new character spawns on SURV with a new life's standing: put it on its variant now.
	void OnPlayerSpawned(int playerId, IEntity ent)
	{
		m_mStanding.Remove(playerId);
		m_mMask.Remove(playerId);
		m_mBaseMask.Remove(playerId);
		m_Retaliation.Forget(playerId);
		m_mSpawnedAt.Set(playerId, m_fClock);

		// Fights aimed at the last life (a hunter squad's target) don't follow this one
		for (int i = m_aZones.Count() - 1; i >= 0; i--)
		{
			if (m_aZones[i].m_iPlayerId == playerId)
				m_aZones.Remove(i);
		}

		Update(playerId, ent);
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		m_fClock += dt;
		UpdateZones();
		m_Disguise.Tick(dt, ids, players);
		m_Retaliation.Tick(dt, ids, players, m_mBaseMask);

		foreach (int i, IEntity p : players)
		{
			Update(ids[i], p);
		}

		// Forget the dead and the disconnected; a respawn starts over
		array<int> stale = {};
		foreach (int playerId, int mask : m_mMask)
		{
			if (!ids.Contains(playerId))
				stale.Insert(playerId);
		}

		foreach (int gone : stale)
		{
			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				m_mStandDown.Remove(string.Format("%1:%2", gone, f));
			}

			m_mMask.Remove(gone);
			m_mBaseMask.Remove(gone);
			m_mStanding.Remove(gone);
			m_mSpawnedAt.Remove(gone);
		}

		// Warnings whose cooldown has run out (players who left among them)
		array<int> warned = {};
		foreach (int warnedId, float at : m_mWarnedAt)
		{
			if (m_fClock - at >= WARN_COOLDOWN)
				warned.Insert(warnedId);
		}

		foreach (int expired : warned)
		{
			m_mWarnedAt.Remove(expired);
		}
	}

	// ================================================================ Fights
	//------------------------------------------------------------------------------------------------
	//! While `anchor` (an AI group or a character) is alive, players within `radius` of it are
	//! hostile to `faction`: only `playerId` if it's > 0. With spareFriends, players Known or
	//! better with the faction are left alone. Missions call this for groups that are meant to
	//! fight; hunter squads for their target.
	void MarkHostileAround(RZ_EFaction faction, IEntity anchor, float radius, int playerId = -1, bool spareFriends = false)
	{
		if (!anchor || !RZ_Factions.IsValid(faction))
			return;

		RZ_HostileZone z = new RZ_HostileZone();
		z.m_eFaction = faction;
		z.m_Anchor = anchor;
		z.m_vPos = RZ_Util.GetUnitPosition(anchor);
		z.m_fRadius = radius;
		z.m_iPlayerId = playerId;
		z.m_bSpareFriends = spareFriends;
		m_aZones.Insert(z);
	}

	//------------------------------------------------------------------------------------------------
	//! Shorthand for missions: a group spawned to fight is hostile to the players near it.
	//! radius < 0 = the Fight radius setting.
	static void MarkFight(RZ_EFaction faction, IEntity group, float radius = -1, bool spareFriends = false)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetHostility() || !group)
			return;

		RZ_Hostility hostility = gm.GetHostility();
		if (radius < 0)
			radius = hostility.GetFightRadius();

		hostility.MarkHostileAround(faction, group, radius, -1, spareFriends);
	}

	//------------------------------------------------------------------------------------------------
	//! Puts friendly AI (a mission's survivor group) onto the variant that `faction` is hostile
	//! to, so that faction's AI attacks them while players stay friends with them. False if the
	//! variant isn't in the FactionManager or hostility is off. Call once the group's members exist.
	bool MakeUnitHostileTo(IEntity unit, RZ_EFaction faction)
	{
		if (!unit || m_Settings.m_eMode == RZ_EHostilityMode.OFF || faction == RZ_EFaction.SURVIVORS)
			return false;

		string key = RZ_Factions.KeyForMask(RZ_Factions.Bit(faction));
		if (!RZ_Util.HasFaction(key))
			return false;

		return RZ_Util.SetUnitFaction(unit, key) > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! A fight (a mission's hostile group, a hunter squad) within radius of pos. Retaliation
	//! ignores hits there: in a crossfire it can't tell who hit whom.
	bool IsFightNear(vector pos, float radius)
	{
		foreach (RZ_HostileZone z : m_aZones)
		{
			if (RZ_Util.DistXZ(z.m_vPos, pos) <= radius)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A fight of `faction` that is after this player (a hunter squad sent for them, or a mission
	//! group fighting everyone near it) within radius of pos.
	bool IsFightFor(int playerId, RZ_EFaction faction, vector pos, float radius)
	{
		foreach (RZ_HostileZone z : m_aZones)
		{
			if (z.m_eFaction != faction || (z.m_iPlayerId > 0 && z.m_iPlayerId != playerId))
				continue;

			if (RZ_Util.DistXZ(z.m_vPos, pos) <= radius)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateZones()
	{
		for (int i = m_aZones.Count() - 1; i >= 0; i--)
		{
			RZ_HostileZone z = m_aZones[i];
			if (!z.m_Anchor || RZ_Util.CountAlive(z.m_Anchor) == 0)
			{
				m_aZones.Remove(i);
				continue;
			}

			z.m_vPos = RZ_Util.GetUnitPosition(z.m_Anchor);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected int ZoneMask(int playerId, vector pos)
	{
		int mask = 0;
		foreach (RZ_HostileZone z : m_aZones)
		{
			if (z.m_iPlayerId > 0 && z.m_iPlayerId != playerId)
				continue;

			if (RZ_Util.DistXZ(z.m_vPos, pos) > z.m_fRadius)
				continue;

			if (z.m_bSpareFriends && RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, z.m_eFaction), RZ_ETier.KNOWN))
				continue;

			mask = mask | RZ_Factions.Bit(z.m_eFaction);
		}

		return mask;
	}

	// ================================================================ The mask
	//------------------------------------------------------------------------------------------------
	//! Whether a faction's AI wants this player dead because of their standing alone.
	protected bool WantsDead(int playerId, RZ_EFaction f)
	{
		RZ_ETier tier = m_GM.GetTier(playerId, f);
		switch (m_Settings.GetFrom(f))
		{
			case RZ_EHostileFrom.HUNTED: return RZ_Reputation.IsHunted(tier);
			case RZ_EHostileFrom.WATCHED: return RZ_Reputation.IsWatchedOrWorse(tier);
			case RZ_EHostileFrom.STRANGERS: return RZ_Reputation.GetAccessRank(tier) < 1;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected int GetStandingMask(int playerId)
	{
		int cached;
		if (m_mStanding.Find(playerId, cached))
			return cached;

		int mask = 0;
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (!WantsDead(playerId, f))
				continue;

			if (f == RZ_EFaction.SURVIVORS && m_Settings.m_bHavenArmedOnly)
				mask = mask | HAVEN_IF_ARMED;
			else
				mask = mask | RZ_Factions.Bit(f);
		}

		m_mStanding.Set(playerId, mask);
		return mask;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsArmedExileAtSafehouse(int playerId, int standing, IEntity ent)
	{
		if ((standing & HAVEN_IF_ARMED) == 0)
			return false;

		float spawned;
		if (m_mSpawnedAt.Find(playerId, spawned) && m_fClock - spawned < EXILE_SPAWN_GRACE)
			return false;

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses || houses.FindNearest(ent.GetOrigin(), m_Settings.m_fExileRadius) < 0)
			return false;

		return RZ_Util.IsArmed(ent);
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Wanderer FindGrudge(int playerId, IEntity ent)
	{
		RZ_Wanderers wanderers = m_GM.GetWanderers();
		if (!m_Settings.m_bWanderersFightBack || !wanderers)
			return null;

		return wanderers.FindArmedGrudgeNear(playerId, ent.GetOrigin(), m_Settings.m_fGrudgeRadius);
	}

	//------------------------------------------------------------------------------------------------
	//! Bit 0 Haven, 1 Bastion, 2 Red Hand, 3 Halcyon: the AI factions that should shoot this player now.
	int ComputeMask(int playerId, IEntity ent)
	{
		int standing = GetStandingMask(playerId);
		int mask = standing & RZ_Factions.ALL_BITS;
		int haven = RZ_Factions.Bit(RZ_EFaction.SURVIVORS);

		// In a faction's uniform, its AI looks past what it holds against you (not Hunted)
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			int bit = RZ_Factions.Bit(f);
			if ((mask & bit) != 0 && m_Disguise.Covers(playerId, f))
				mask = mask - bit;
		}

		if (IsArmedExileAtSafehouse(playerId, standing, ent))
			mask = mask | haven;

		mask = mask | ZoneMask(playerId, ent.GetOrigin());

		if ((mask & haven) == 0 && FindGrudge(playerId, ent))
			mask = mask | haven;

		return mask;
	}

	//------------------------------------------------------------------------------------------------
	//! The faction key a mask puts the character on in the current mode.
	string KeyFor(int mask)
	{
		if (mask <= 0 || m_Settings.m_eMode == RZ_EHostilityMode.OFF)
			return RZ_Factions.GetPlayerKey();

		if (m_Settings.m_eMode == RZ_EHostilityMode.OUTLAW)
			return RZ_Factions.OutlawKey();

		return RZ_Factions.KeyForMask(mask);
	}

	//------------------------------------------------------------------------------------------------
	protected void Update(int playerId, IEntity ent)
	{
		if (!ent)
			return;

		int mask = 0;
		if (m_Settings.m_eMode != RZ_EHostilityMode.OFF)
		{
			int baseMask = ComputeMask(playerId, ent);
			m_mBaseMask.Set(playerId, baseMask);
			mask = baseMask | m_Retaliation.MaskFor(playerId, ent.GetOrigin());
			int calm = StandDownMask(playerId);
			mask = mask - (mask & calm);
		}

		int before;
		if (m_mMask.Find(playerId, before) && before != mask)
			OnMaskChanged(playerId, ent, before, mask);

		m_mMask.Set(playerId, mask);
		ApplyKey(ent, KeyFor(mask));
	}

	//------------------------------------------------------------------------------------------------
	//! Only the character's faction, and only when it differs (cheap on every tick, and it
	//! also catches a respawn or anything else that put the character back on SURV).
	protected void ApplyKey(IEntity ent, string key)
	{
		string current = RZ_Util.GetCharacterFactionKey(ent);
		if (current == key)
			return;

		// Leave characters another mod put on a non-player faction alone
		if (!current.IsEmpty() && !RZ_Factions.IsPlayerSide(current))
			return;

		if (!RZ_Util.HasFaction(key))
		{
			if (!m_aMissing.Contains(key))
			{
				m_aMissing.Insert(key);
				RZ_Log.Warn("Hostility: no faction " + key + " in the FactionManager (docs/WORKBENCH_SETUP.md). The player stays on " + RZ_Factions.GetPlayerKey() + ".");
			}

			key = RZ_Factions.GetPlayerKey();
			if (current == key)
				return;
		}

		RZ_Util.SetCharacterFaction(ent, key);
	}

	//------------------------------------------------------------------------------------------------
	//! Standing changes are told by the tier message. Nearness changes are told here, so a
	//! player knows why the guards just raised their rifles.
	protected void OnMaskChanged(int playerId, IEntity ent, int before, int after)
	{
		int haven = RZ_Factions.Bit(RZ_EFaction.SURVIVORS);
		if ((before & haven) != 0 || (after & haven) == 0)
			return;

		float last;
		if (m_mWarnedAt.Find(playerId, last) && m_fClock - last < WARN_COOLDOWN)
			return;

		m_mWarnedAt.Set(playerId, m_fClock);
		if (IsArmedExileAtSafehouse(playerId, GetStandingMask(playerId), ent))
		{
			m_GM.NotifyPlayer(playerId, "The Haven", "An exile with a gun at a Haven door. The guards have their rifles on you. Leave, or come back with empty hands.");
			return;
		}

		RZ_Wanderer w = FindGrudge(playerId, ent);
		if (!w)
			return;

		if (w.m_eRole == RZ_EWandererRole.DESERTER && !w.Remembers(m_GM.GetProfile(playerId).m_sIdentity, RZ_Wanderers.MEM_WRONGED))
			m_GM.NotifyPlayer(playerId, w.GetTitle(), "\"I know that face from the briefings.\" The rifle comes up before you can say a word.");
		else
			m_GM.NotifyPlayer(playerId, w.GetTitle(), string.Format("%1 has seen you. They remember what you did, and this time they're armed.", w.GetShortName()));
	}

	//------------------------------------------------------------------------------------------------
	//! What a faction's AI does to someone it hunts, for the tier message. "" if it doesn't shoot.
	string DescribeHunted(RZ_EFaction f)
	{
		if (m_Settings.m_eMode == RZ_EHostilityMode.OFF || m_Settings.GetFrom(f) == RZ_EHostileFrom.NEVER)
			return "";

		switch (f)
		{
			case RZ_EFaction.SURVIVORS:
			{
				if (m_Settings.m_bHavenArmedOnly)
					return "Come near a safehouse with a gun and the guards will shoot.";

				return "The Haven's guards will shoot you on sight.";
			}
			case RZ_EFaction.MILITARY: return "Bastion soldiers will open fire on sight.";
			case RZ_EFaction.BANDITS: return "Red Hand crews will open fire on sight.";
			case RZ_EFaction.INSTITUTE: return "Halcyon's guards will open fire on sight.";
		}

		return "";
	}
}
