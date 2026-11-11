enum RZ_EMissionState
{
	ACTIVE,
	SUCCEEDED,
	FAILED
}

//! Base for every mission. There are two kinds (docs/MISSIONS.md):
//!
//! - World events (m_bContract = false) are announced to everyone, and anyone
//!   who takes part shares the reward.
//! - Faction contracts (m_bContract = true) are offered only to players who meet
//!   m_Req. Anyone can turn up, but only eligible players get paid.
//!
//! A mission picks a settlement on the current map, so every type works on any
//! terrain. Anyone who comes within m_fParticipationRadius while it runs counts
//! as taking part.
//!
//! Life cycle, driven by RZ_MissionManager:
//!   new -> Configure() -> GetWeight() -> Setup() -> Update() each tick ->
//!   OnSuccess()/OnFailed() -> rewards -> Cleanup()
class RZ_Mission
{
	int m_iId;
	string m_sTitle;
	string m_sBrief;
	int m_iSettlement = -1;
	vector m_vPos;

	bool m_bContract = true;		// false = world event, open to everyone
	bool m_bGlobal;					// no single location (Night of the Dead, Storm...)
	bool m_bHasGiver = true;		// false = no faction pays (most world events)
	RZ_EFaction m_eGiver = RZ_EFaction.SURVIVORS;
	int m_iFameReward = 10;
	int m_iFavorReward = 10;
	bool m_bHasRival;
	RZ_EFaction m_eRival = RZ_EFaction.BANDITS;
	int m_iRivalInfamy;
	bool m_bAmends;					// Watched players work off Infamy instead of gaining Fame
	float m_fBonusFameMult = 1;		// extra Fame for ★ backgrounds (see m_Req.m_aBonusBackgrounds)
	float m_fRewardScale = 1;		// the mission can lower or raise the payout (e.g. mercy kill)
	ref RZ_MissionRequirements m_Req = new RZ_MissionRequirements();

	float m_fTimeLeft = 2400;
	float m_fParticipationRadius = 150;
	RZ_EMissionState m_eState = RZ_EMissionState.ACTIVE;
	ref array<int> m_aParticipants = {};
	ref array<int> m_aOfferedTo = {};
	int m_iMarkerId = -1;			// map marker handle (RZ_MapMarkers), -1 = none

	// Faction leaders (Factions/RZ_Leaders.c, Missions/Leaders/)
	int m_iLeaderId = -1;			// the leader who gives (voices and pays) this contract, -1 = none
	bool m_bInPersonOnly;			// a leader's personal job: offered by them in person, never on the radio
	bool m_bRogueJob;				// off the stronghold's books: may be discovered when it's done
	bool m_bSeenByFaction;			// a rogue job seen by the leader's own people
	bool m_bExposed;				// the player turned the rogue leader in instead
	bool m_bNoLeaderCredit;			// done, but not the way the leader wanted: they don't remember it as help
	int m_iJobFaction = -1;			// personal jobs: the faction and loyalty of the leaders who give them
	int m_iJobLoyalty = -1;
	int m_iJobTone;					// RZ_Leaders.TONE_* bits, which traits lean toward

	// Questlines (Director/RZ_Questlines.c): the run that started this mission as one of its stages
	int m_iQuestRun = -1;			// -1 = not part of a questline

	// Rough areas (docs/MISSIONS.md, "Who gets offered what"): contracts name a circle, not a spot
	ref array<int> m_aExactFor = {};	// players who came close: they know the spot now
	protected vector m_vRoughOffset;	// the circle's centre, from the real spot (follows it if it moves)
	protected bool m_bRoughReady;

	//------------------------------------------------------------------------------------------------
	//! Set giver, rival, rewards and requirements here. Runs before GetWeight and Setup.
	void Configure(RZ_GameModeComponent gm)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Shortcut for Configure(): a contract from `giver`.
	protected void SetContract(RZ_EFaction giver, RZ_ETier minTier, int fame, int favor)
	{
		m_bContract = true;
		m_bHasGiver = true;
		m_eGiver = giver;
		m_Req.m_eGiver = giver;
		m_Req.m_eMinTier = minTier;
		m_iFameReward = fame;
		m_iFavorReward = favor;
	}

	//------------------------------------------------------------------------------------------------
	//! Shortcut for Configure(): a world event. Pass hasGiver = true if a faction still pays for it.
	protected void SetEvent(bool hasGiver, RZ_EFaction giver = RZ_EFaction.SURVIVORS, int fame = 0, int favor = 0)
	{
		m_bContract = false;
		m_bHasGiver = hasGiver;
		m_eGiver = giver;
		m_Req.m_eGiver = giver;
		m_iFameReward = fame;
		m_iFavorReward = favor;
	}

	//------------------------------------------------------------------------------------------------
	//! Shortcut for Configure(): a leader's personal job (docs/MISSIONS.md, "Leader missions").
	//! Only leaders of this faction and loyalty give it, in person. Their attitude, not the
	//! giver tier, decides who may take it. Rogue jobs pay no faction Fame by default.
	protected void SetLeaderJob(RZ_EFaction faction, RZ_ELoyalty loyalty, int tone, int fame, int favor)
	{
		SetContract(faction, RZ_ETier.STRANGER, fame, favor);
		m_Req.m_bLeaderDecides = true;
		m_bInPersonOnly = true;
		m_bRogueJob = loyalty == RZ_ELoyalty.ROGUE;
		m_iJobFaction = faction;
		m_iJobLoyalty = loyalty;
		m_iJobTone = tone;
		m_fParticipationRadius = 0;
		m_fTimeLeft = 2700;
	}

	//------------------------------------------------------------------------------------------------
	//! Personal jobs: how much this leader wants this job done (before trait bias). 0 = can't
	//! run here (missing prefabs, no second safehouse...). Never called for other missions.
	float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Personal jobs are only listed in person: near the leader's seat.
	bool CanOfferAt(RZ_GameModeComponent gm, vector playerPos)
	{
		if (!m_bInPersonOnly)
			return true;

		if (!gm.GetLeaders())
			return false;

		RZ_Leader l = gm.GetLeaders().Get(m_iLeaderId);
		if (!l)
			return false;

		return RZ_Util.DistXZ(l.m_vPos, playerPos) <= 80;
	}

	//------------------------------------------------------------------------------------------------
	//! True for contracts that must give exact positions: puzzles and others that give their own
	//! clues or grid references, points to visit, a rough ping of their own. Never a rough area.
	bool NeedsExactLocation()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! True for missions that hand players a vehicle (off under the season's No Vehicles rule).
	bool NeedsVehicle()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Gunfire by a player (all missions hear it; e.g. Local Law fails on bloodshed).
	void OnPlayerShot(RZ_GameModeComponent gm, int playerId, vector pos)
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void SetRival(RZ_EFaction rival, int infamy)
	{
		m_bHasRival = true;
		m_eRival = rival;
		m_iRivalInfamy = infamy;
	}

	//------------------------------------------------------------------------------------------------
	//! Relative chance of this mission type being picked right now. 0 = never.
	float GetWeight(RZ_GameModeComponent gm)
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Pick a location and fill in the text. Return false if no valid location exists.
	bool Setup(RZ_GameModeComponent gm)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Contracts: does this player meet the requirements? World events are open to everyone.
	bool IsEligible(RZ_GameModeComponent gm, int playerId)
	{
		if (!m_bContract)
			return true;

		if (!m_Req.IsEligible(gm, playerId))
			return false;

		// A leader's personal job: the leader decides who gets it
		if (m_bInPersonOnly && m_iLeaderId >= 0 && gm.GetLeaders())
			return gm.GetLeaders().CanWorkFor(m_iLeaderId, playerId);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! ★ background for this mission?
	bool HasBonus(RZ_GameModeComponent gm, int playerId)
	{
		if (playerId <= 0)
			return false;

		return m_Req.IsBonus(gm.GetProfile(playerId).m_sBackgroundId);
	}

	//------------------------------------------------------------------------------------------------
	//! True if an eligible player with a ★ background is within radius of pos.
	bool AnyBonusNear(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players, vector pos, float radius)
	{
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius && HasBonus(gm, ids[i]) && IsEligible(gm, ids[i]))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool AnyEligible(RZ_GameModeComponent gm, array<int> ids)
	{
		foreach (int playerId : ids)
		{
			if (IsEligible(gm, playerId))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	void Update(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_fTimeLeft -= dt;

		if (m_fParticipationRadius > 0)
		{
			foreach (int i, IEntity p : players)
			{
				if (RZ_Util.DistXZ(p.GetOrigin(), m_vPos) <= m_fParticipationRadius && !m_aParticipants.Contains(ids[i]))
					m_aParticipants.Insert(ids[i]);
			}
		}

		RevealNear(gm, ids, players);
		OnUpdate(gm, dt, ids, players);

		if (m_eState == RZ_EMissionState.ACTIVE && m_fTimeLeft <= 0)
			OnTimeUp(gm);
	}

	// ================================================================ Rough areas
	//------------------------------------------------------------------------------------------------
	//! The rough-area radius for this mission, 0 if it gives exact positions.
	protected float RoughRadius(RZ_GameModeComponent gm)
	{
		if (!m_bContract || m_bGlobal || NeedsExactLocation())
			return 0;

		RZ_ContractSettings cs = gm.GetContractSettings();
		if (!cs)
			return 0;

		return Math.Max(0, cs.m_fRoughRadius);
	}

	//------------------------------------------------------------------------------------------------
	protected float RevealDistance(RZ_GameModeComponent gm)
	{
		RZ_ContractSettings cs = gm.GetContractSettings();
		if (!cs)
			return 500;

		return Math.Max(50, cs.m_fRevealDistance);
	}

	//------------------------------------------------------------------------------------------------
	//! Who knows only the rough area, whatever their distance: everyone but Radio Operators,
	//! players Trusted (or more) with the giver, and those who have been close.
	protected bool RoughByStanding(RZ_GameModeComponent gm, int playerId)
	{
		if (RoughRadius(gm) <= 0)
			return false;

		if (playerId <= 0)
			return true;

		if (m_aExactFor.Contains(playerId))
			return false;

		RZ_Background bg = gm.GetPlayerBackground(playerId);
		if (bg && bg.m_bRadioIntel)
			return false;

		RZ_ETier tier = gm.GetTier(playerId, m_Req.m_eGiver);
		return !RZ_Reputation.IsAtLeast(tier, RZ_ETier.TRUSTED) || RZ_Reputation.IsHunted(tier);
	}

	//------------------------------------------------------------------------------------------------
	//! Does this player, standing at `from`, get only the rough area?
	bool IsRoughFor(RZ_GameModeComponent gm, int playerId, vector from)
	{
		if (!RoughByStanding(gm, playerId))
			return false;

		return from == vector.Zero || RZ_Util.DistXZ(from, m_vPos) > RevealDistance(gm);
	}

	//------------------------------------------------------------------------------------------------
	//! The rough circle's centre: the real spot moved a random 30-70% of the radius, once.
	vector GetRoughCenter(RZ_GameModeComponent gm)
	{
		if (!m_bRoughReady)
		{
			m_bRoughReady = true;
			float r = RoughRadius(gm);
			float angle = Math.RandomFloat(0, Math.PI2);
			float dist = Math.RandomFloat(r * 0.3, r * 0.7);
			m_vRoughOffset = Vector(Math.Cos(angle) * dist, 0, Math.Sin(angle) * dist);
		}

		return m_vPos + m_vRoughOffset;
	}

	//------------------------------------------------------------------------------------------------
	//! "Somewhere around Morton: within 500 m of a point 1.2 km NE".
	string GetRoughWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = string.Format("Somewhere %1: within %2 m of a point", GetRoughAreaName(gm), RZ_Util.RoundToInt(RoughRadius(gm)));
		if (from != vector.Zero)
			text += " " + RZ_Util.DirectionText(from, GetRoughCenter(gm));
		else
			text += " at grid " + RZ_SettlementScanner.GridReference(GetRoughCenter(gm));

		return text + string.Format(". You'll know the spot within %1 m.", RZ_Util.RoundToInt(RevealDistance(gm)));
	}

	//------------------------------------------------------------------------------------------------
	//! "around Morton", or "near grid 045-112" without a settlement.
	string GetRoughAreaName(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
			return "around " + s.m_sName;

		return "near grid " + RZ_SettlementScanner.GridReference(GetRoughCenter(gm));
	}

	//------------------------------------------------------------------------------------------------
	//! Where text for one player: the rough area, or the full text (GetWhereText) once they know
	//! the spot. Every brief, radio list and journal entry for a player goes through here.
	string GetWhereTextFor(RZ_GameModeComponent gm, vector from, int playerId)
	{
		if (IsRoughFor(gm, playerId, from))
			return GetRoughWhereText(gm, from);

		if (from == vector.Zero)
			return GetLocationName(gm);

		return GetWhereText(gm, from);
	}

	//------------------------------------------------------------------------------------------------
	//! The place's name for one player (the journal's "where"): "around Morton" while rough.
	string GetLocationNameFor(RZ_GameModeComponent gm, int playerId, vector from)
	{
		if (IsRoughFor(gm, playerId, from))
			return GetRoughAreaName(gm) + string.Format(" (within %1 m)", RZ_Util.RoundToInt(RoughRadius(gm)));

		return GetLocationName(gm);
	}

	//------------------------------------------------------------------------------------------------
	//! Distance and direction for one player (the journal and the guidance): to the rough
	//! circle's centre while rough, "" without a position.
	string GetDirectionFor(RZ_GameModeComponent gm, vector from, int playerId)
	{
		if (m_bGlobal || from == vector.Zero)
			return "";

		if (IsRoughFor(gm, playerId, from))
			return "about " + RZ_Util.DirectionText(from, GetRoughCenter(gm));

		return RZ_Util.DirectionText(from, m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	//! Players who were only given the rough area and come within the reveal distance learn the
	//! spot (and are told, if the contract was offered to them).
	protected void RevealNear(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players)
	{
		if (RoughRadius(gm) <= 0)
			return;

		float reveal = RevealDistance(gm);
		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];
			if (m_aExactFor.Contains(playerId) || RZ_Util.DistXZ(p.GetOrigin(), m_vPos) > reveal)
				continue;

			bool wasRough = RoughByStanding(gm, playerId);
			m_aExactFor.Insert(playerId);
			if (wasRough && m_aOfferedTo.Contains(playerId))
				gm.NotifyPlayer(playerId, m_sTitle, "This is the place. " + RZ_Util.Capitalize(GetWhereText(gm, p.GetOrigin())));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Default: running out of time is a failure. Survival events override this to succeed.
	protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
	}

	//------------------------------------------------------------------------------------------------
	void OnZombiesKilled(RZ_GameModeComponent gm, int settlementIdx, int missionId, bool special, vector pos, int count)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! A mission item was handed in at a drop point. Return true if this mission takes it.
	bool OnItemDelivered(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction, IEntity drop)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! "Talk to"/"Read" on a person or thing a non-puzzle mission tagged with RZ_ClueComponent
	//! (e.g. the patient in Medicine Before Dark). True if handled.
	bool HandleTalk(RZ_GameModeComponent gm, int playerId, int clueIdx)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A player died (or turned) at pos.
	void OnPlayerDied(RZ_GameModeComponent gm, int playerId, vector pos)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! A supply truck arrived or was lost (RZ_SupplyLines.TRUCK_*). driver: the player who drove
	//! it in or away, -1 if none. Convoy missions (Missions/Blocks/RZ_VehicleZone.c) use it.
	void OnTruckEvent(RZ_GameModeComponent gm, int faction, int what, vector pos, int driver)
	{
	}

	//------------------------------------------------------------------------------------------------
	void OnSuccess(RZ_GameModeComponent gm)
	{
	}

	//------------------------------------------------------------------------------------------------
	void OnFailed(RZ_GameModeComponent gm)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Extra per-player reward after Fame/Favor were paid (only eligible participants).
	void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Where this mission has props, items, people or crates on the ground. No building within
	//! RZ_MissionManager.PROP_CLEAR of one has a lock while it runs (docs/DESIGN.md, "Locks and
	//! keys"), so nothing a mission put down ends up behind a door. Default: its position.
	void GetPropSpots(notnull array<vector> result)
	{
		if (!m_bGlobal && m_vPos != vector.Zero)
			result.Insert(m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	//! A building whose lock is part of this mission (Locked Bunker's keypad): it keeps its lock
	//! even with the mission's props beside it.
	bool KeepsLock(string buildingKey)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Delete whatever the mission spawned. Runs after OnSuccess/OnFailed.
	void Cleanup(RZ_GameModeComponent gm)
	{
	}

	//------------------------------------------------------------------------------------------------
	string GetSuccessText(RZ_GameModeComponent gm)
	{
		return string.Format("%1 (%2) is done.", m_sTitle, GetLocationName(gm));
	}

	//------------------------------------------------------------------------------------------------
	string GetFailText(RZ_GameModeComponent gm)
	{
		return string.Format("%1 (%2): the window has closed.", m_sTitle, GetLocationName(gm));
	}

	//------------------------------------------------------------------------------------------------
	//! "Supply cache: Morton (1.2 km NE)" for the player at `from`.
	string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		if (m_bGlobal)
			return GetLocationName(gm);

		return GetLocationName(gm) + ", " + RZ_Util.DirectionText(from, m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	void NotifyParticipants(RZ_GameModeComponent gm, string title, string text)
	{
		foreach (int playerId : m_aParticipants)
		{
			gm.NotifyPlayer(playerId, title, text);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Contracts tell the players they were offered to; world events tell everyone.
	void NotifyInterested(RZ_GameModeComponent gm, string title, string text)
	{
		if (!m_bContract)
		{
			gm.NotifyAll(title, text);
			return;
		}

		foreach (int playerId : m_aOfferedTo)
		{
			gm.NotifyPlayer(playerId, title, text);
		}

		foreach (int playerId2 : m_aParticipants)
		{
			if (!m_aOfferedTo.Contains(playerId2))
				gm.NotifyPlayer(playerId2, title, text);
		}
	}

	//------------------------------------------------------------------------------------------------
	string GetLocationName(RZ_GameModeComponent gm)
	{
		if (m_bGlobal)
			return "everywhere";

		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
			return s.m_sName;

		return "grid " + RZ_SettlementScanner.GridReference(m_vPos);
	}

	// ================================================================ Helpers for mission types
	//------------------------------------------------------------------------------------------------
	//! Random settlement whose infection is within [minInf, maxInf]. -1 if none.
	static int PickSettlement(RZ_GameModeComponent gm, float minInf, float maxInf)
	{
		array<int> candidates = {};
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			if (s.m_fInfection >= minInf && s.m_fInfection <= maxInf && !gm.GetMissions().IsSettlementBusy(i))
				candidates.Insert(i);
		}

		if (candidates.IsEmpty())
			return -1;

		return candidates.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! Random settlement between minDist and maxDist from `from`, within the infection range, other
	//! than `exclude`. -1 if none.
	static int PickSettlementAway(RZ_GameModeComponent gm, vector from, float minDist, float maxDist, float minInf, float maxInf, int exclude = -1)
	{
		array<int> candidates = {};
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			if (i == exclude)
				continue;

			float d = RZ_Util.DistXZ(from, s.m_vCenter);
			if (d < minDist || d > maxDist)
				continue;

			if (s.m_fInfection < minInf || s.m_fInfection > maxInf || gm.GetMissions().IsSettlementBusy(i))
				continue;

			candidates.Insert(i);
		}

		if (candidates.IsEmpty())
			return -1;

		return candidates.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! Least infected settlement (a likely Haven safehouse). -1 if none.
	static int PickSafestSettlement(RZ_GameModeComponent gm, int exclude = -1)
	{
		int best = -1;
		float bestInf = float.MAX;
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			if (i == exclude)
				continue;

			if (s.m_fInfection < bestInf)
			{
				bestInf = s.m_fInfection;
				best = i;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Highest of a few sampled dry points in a ring, never behind a lock. Works on any terrain.
	static vector HighPointNear(vector center, float minR, float maxR, int samples = 16)
	{
		vector best = LandPointNear(center, minR, maxR);
		for (int i = 0; i < samples; i++)
		{
			vector p = RZ_Util.RandomPointInRing(center, minR, maxR);
			if (RZ_Util.IsWater(p) || p[1] <= best[1])
				continue;

			if (!RZ_LockedSpots.IsBlocked(p))
				best = p;
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! A dry point in a ring, away from the quarantine line and never inside a locked building or
	//! by its doorway blocker (RZ_LockedSpots), so mission props are never behind a lock. Falls
	//! back to the first dry point moved out past the locked building's wall, then the last try.
	static vector LandPointNear(vector center, float minR, float maxR)
	{
		vector p = RZ_Util.RandomPointInRing(center, minR, maxR);
		vector dry = p;
		bool haveDry = false;
		for (int i = 0; i < 10; i++)
		{
			if (!RZ_Util.IsWater(p) && !RZ_Perimeter.IsNearLineAt(p))
			{
				if (!RZ_LockedSpots.IsBlocked(p))
					return p;

				if (!haveDry)
				{
					dry = p;
					haveDry = true;
				}
			}

			p = RZ_Util.RandomPointInRing(center, minR, maxR);
		}

		if (haveDry)
			return RZ_LockedSpots.MoveOut(dry);

		return p;
	}

	//------------------------------------------------------------------------------------------------
	//! Zombies tagged with this mission, so they don't despawn while it runs.
	void SpawnGuards(RZ_GameModeComponent gm, vector center, int count, float minR, float maxR, bool special = false)
	{
		for (int i = 0; i < count; i++)
		{
			gm.GetSpawner().SpawnZombie(LandPointNear(center, minR, maxR), special, m_iSettlement, m_iId);
		}
	}

	//------------------------------------------------------------------------------------------------
	static int CountPlayersNear(array<IEntity> players, vector pos, float radius)
	{
		int n = 0;
		foreach (IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Player ids within radius of pos.
	static void GetPlayersNear(array<int> ids, array<IEntity> players, vector pos, float radius, notnull array<int> result)
	{
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius)
				result.Insert(ids[i]);
		}
	}
}
