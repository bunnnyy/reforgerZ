//! Live capture block (docs/DESIGN.md, "Mission building blocks"). Halcyon wants a special
//! alive (Live Capture, Private Collection). A puzzle mission owns one RZ_LiveCapture, starts it,
//! ticks it, routes the cage's choice to TryCloseCage() and asks it about hand-ins.
//!
//!   BAIT     a cage trap stands near the special's lair; both appear only when a player comes
//!            close. Lure the special (it chases you) to the cage and drop the door while it's
//!            within 4 m. Or wound it to a third of its health and get within 3 m: it's pinned
//!            and tied where it lies. Killing it fails the job.
//!   CAUGHT   it fights the bars: someone stays within 15 m for 30 s while the noise draws
//!            infected (heat), then the sedative takes.
//!   CRATED   a transport case (the mission item) to carry to the drop. It wakes after 12 real
//!            minutes (Contracts > Specimen wake minutes): the case breaks and the special is loose.
//!
//! The caught special leaves the world without a kill (RZ_ZombieSpawner.RemoveSilently), so it
//! drops no sample. At most Contracts > Max captures run at once. Cleanup() removes the cage,
//! the case and nothing else (a special still loose is released to the spawner).
enum RZ_ECaptureState
{
	BAIT,
	CAUGHT,
	CRATED,
	DELIVERED,
	LOST
}

class RZ_LiveCapture
{
	protected static int s_iRunning;

	RZ_ECaptureState m_eState = RZ_ECaptureState.BAIT;
	string m_sLostReason;
	string m_sCageLine;		// what the cage door's catch says ("": Halcyon's line)
	vector m_vCage;
	vector m_vLair;
	vector m_vHeld;

	protected int m_iMissionId = -1;
	protected int m_iSettlement = -1;
	protected int m_iCageObj = -1;
	protected bool m_bCounted;
	protected IEntity m_Cage;
	protected bool m_bSpecialSpawned;
	protected float m_fStartHealth = -1;
	protected float m_fHeldFor;
	protected IEntity m_Item;
	protected float m_fWakeLeft;
	protected int m_iCarrier = -1;
	protected bool m_bWarnedWake;
	protected vector m_vItemLastPos;
	protected bool m_bItemWasCarried;
	protected int m_iItemReturns;
	protected ResourceName m_sPrefab;		// the special's prefab ("": the spawner's specials)

	protected static const float SPAWN_RADIUS = 250;
	protected static const float HOLD_SECONDS = 30;
	protected static const float HOLD_RADIUS = 15;
	protected static const float RESTRAIN_REACH = 3;

	//------------------------------------------------------------------------------------------------
	//! A new game: no capture runs yet (statics outlive a session).
	static void ResetCount()
	{
		s_iRunning = 0;
	}

	//------------------------------------------------------------------------------------------------
	static bool HasRoom(RZ_GameModeComponent gm)
	{
		RZ_ContractSettings cs = gm.GetContractSettings();
		int cap = 2;
		if (cs)
			cap = cs.m_iMaxCaptures;

		return s_iRunning < cap && !gm.GetMissionItemPrefab().IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! cageObj: the puzzle object index the mission uses for the cage's choice ("Drop the door").
	bool Begin(RZ_GameModeComponent gm, int missionId, int settlement, vector cagePos, vector lairPos, int cageObj)
	{
		if (!m_bCounted)
		{
			if (!HasRoom(gm))
				return false;

			s_iRunning++;
			m_bCounted = true;
		}

		m_iMissionId = missionId;
		m_iSettlement = settlement;
		m_vCage = RZ_Util.GroundPos(cagePos);
		m_vLair = RZ_Util.GroundPos(lairPos);
		m_vHeld = m_vCage;
		m_iCageObj = cageObj;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The mission brings its own special (Capture the Index: a pack's leader, tagged with the
	//! mission's id): the block never spawns one at the lair.
	void UseOwnSpecial()
	{
		m_bSpecialSpawned = true;
	}

	//------------------------------------------------------------------------------------------------
	//! The special is this prefab (Live Capture and Stock the Pit: one of a pack's guard or young).
	void UsePrefab(ResourceName prefab)
	{
		m_sPrefab = prefab;
	}

	//------------------------------------------------------------------------------------------------
	//! Where the case is (or was last seen): its pack follows it (Capture the Index).
	vector CasePos()
	{
		return m_vItemLastPos;
	}

	//------------------------------------------------------------------------------------------------
	bool IsLost()
	{
		return m_eState == RZ_ECaptureState.LOST;
	}

	//------------------------------------------------------------------------------------------------
	void Update(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		switch (m_eState)
		{
			case RZ_ECaptureState.BAIT:
				UpdateBait(gm, players);
				return;
			case RZ_ECaptureState.CAUGHT:
				UpdateHeld(gm, dt, ids, players);
				return;
			case RZ_ECaptureState.CRATED:
				UpdateCrate(gm, dt);
				return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateBait(RZ_GameModeComponent gm, array<IEntity> players)
	{
		if (!m_Cage && RZ_Util.NearestDistance(m_vCage, players) <= SPAWN_RADIUS)
			SpawnCage(gm);

		if (!m_bSpecialSpawned && RZ_Util.NearestDistance(m_vLair, players) <= SPAWN_RADIUS)
		{
			m_bSpecialSpawned = gm.GetSpawner().SpawnZombie(m_vLair, true, m_iSettlement, m_iMissionId, m_sPrefab) != null;
			return;
		}

		RZ_TrackedZombie z = gm.GetSpawner().FindMissionSpecial(m_iMissionId);
		if (!z)
			return;

		// Wounded and someone's on it: pinned and tied
		IEntity body = SpecialBody(z.m_Entity);
		float health = RZ_Util.GetCharacterHealth(body);
		if (health <= 0)
			return;

		if (m_fStartHealth < 0)
			m_fStartHealth = health;

		float share = Math.Max(0.05, Settings(gm).m_fRestrainShare);
		if (health > m_fStartHealth * share)
			return;

		vector at = body.GetOrigin();
		if (RZ_Util.NearestDistance(at, players) > RESTRAIN_REACH)
			return;

		Catch(gm, z.m_Entity, at, "It goes down hard and you're on it before it gets up: knee on its back, belt round its wrists, a sack over its head. It's still fighting. Hold it here until it's under.");
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity SpecialBody(IEntity e)
	{
		array<IEntity> members = {};
		RZ_Util.GetUnitMembers(e, members);
		if (!members.IsEmpty())
			return members[0];

		return e;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_ContractSettings Settings(RZ_GameModeComponent gm)
	{
		RZ_ContractSettings cs = gm.GetContractSettings();
		if (!cs)
			cs = RZ_ContractSettings.Cast(RZ_Util.NewWithDefaults("RZ_ContractSettings"));

		return cs;
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnCage(RZ_GameModeComponent gm)
	{
		ResourceName prefab = Settings(gm).m_sCagePrefab;
		if (prefab.IsEmpty())
			prefab = gm.GetPuzzlePrefab(RZ_EPuzzleProp.OBJECT);

		m_Cage = RZ_Util.SpawnPrefab(prefab, m_vCage, Math.RandomFloat(0, 360));
		if (m_Cage)
			RZ_PuzzleObjectComponent.Tag(m_Cage, m_iMissionId, m_iCageObj, "", "Drop the cage door");
	}

	//------------------------------------------------------------------------------------------------
	//! "Drop the cage door" (the mission routes the cage's choice here). True if it caught it.
	bool TryCloseCage(RZ_GameModeComponent gm, int playerId)
	{
		if (m_eState != RZ_ECaptureState.BAIT)
		{
			gm.NotifyPlayer(playerId, "The cage", "The door's already down.");
			return false;
		}

		RZ_TrackedZombie z = gm.GetSpawner().FindMissionSpecial(m_iMissionId);
		float reach = Settings(gm).m_fCaptureRadius;
		if (!z || RZ_Util.DistXZ(RZ_Util.GetUnitPosition(z.m_Entity), m_vCage) > reach)
		{
			gm.GetHeat().AddHeat(m_vCage, 6);
			gm.NotifyPlayer(playerId, "The cage", "The door slams on nothing. Everything nearby heard it. You haul it back up.");
			return false;
		}

		string line = m_sCageLine;
		if (line.IsEmpty())
			line = "The door drops. It hits the bars so hard the whole cage jumps, again and again. Stay close until Halcyon's sedative takes.";

		Catch(gm, z.m_Entity, m_vCage, line);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Catch(RZ_GameModeComponent gm, IEntity special, vector at, string text)
	{
		gm.GetSpawner().RemoveSilently(special);
		m_vHeld = at;
		m_eState = RZ_ECaptureState.CAUGHT;
		m_fHeldFor = 0;
		if (m_Cage)
			RZ_PuzzleObjectComponent.Tag(m_Cage, m_iMissionId, m_iCageObj, "", "");

		NotifyNear(gm, at, 80, "Caught", text);
	}

	//------------------------------------------------------------------------------------------------
	//! It fights: heat every tick. Someone near for 30 s and it's crated.
	protected void UpdateHeld(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		gm.GetHeat().AddHeat(m_vHeld, 2 * dt);
		if (RZ_Mission.CountPlayersNear(players, m_vHeld, HOLD_RADIUS) == 0)
			return;

		m_fHeldFor += dt;
		if (m_fHeldFor < HOLD_SECONDS)
			return;

		m_Item = RZ_Util.SpawnPrefab(gm.GetMissionItemPrefab(), m_vHeld + Vector(0, 0.3, 0));
		if (!m_Item)
		{
			m_eState = RZ_ECaptureState.LOST;
			m_sLostReason = "There was nothing to put it in. (The mission item prefab is missing.)";
			return;
		}

		RZ_MissionItemComponent.Tag(m_Item, m_iMissionId);
		m_fWakeLeft = Math.Max(1, Settings(gm).m_fSpecimenWakeMinutes) * 60;
		m_eState = RZ_ECaptureState.CRATED;
		NotifyNear(gm, m_vHeld, 80, "Caught", string.Format("It's under. Into the transport case, and the case into your pack. The sedative lasts about %1 minutes.", RZ_Util.RoundToInt(m_fWakeLeft / 60)));
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateCrate(RZ_GameModeComponent gm, float dt)
	{
		if (!m_Item)
		{
			// Gone with a carrier who left the server (or a body that was cleared): it's put back
			// where it was, still sedated, a few times at most
			if (m_bItemWasCarried && m_iItemReturns < 3)
			{
				m_bItemWasCarried = false;
				m_Item = RZ_Util.SpawnPrefab(gm.GetMissionItemPrefab(), RZ_Mission.LandPointNear(m_vItemLastPos, 0, 2) + Vector(0, 0.3, 0));
				if (m_Item)
				{
					m_iItemReturns++;
					m_iCarrier = -1;
					RZ_MissionItemComponent.Tag(m_Item, m_iMissionId);
					NotifyNear(gm, m_vItemLastPos, 300, "The case", "Whoever carried the transport case is gone. The case is lying where they were.");
					return;
				}
			}

			m_eState = RZ_ECaptureState.LOST;
			m_sLostReason = "The case is gone, and what was in it.";
			return;
		}

		IEntity root = m_Item.GetRootParent(); // the same call RZ_Util.GetCarrierPlayerId makes (listed there)
		m_bItemWasCarried = root && root != m_Item;
		if (m_bItemWasCarried)
			m_vItemLastPos = root.GetOrigin();
		else
			m_vItemLastPos = m_Item.GetOrigin();

		int carrier = RZ_Util.GetCarrierPlayerId(m_Item);
		if (carrier > 0)
			m_iCarrier = carrier;

		m_fWakeLeft -= dt;
		if (!m_bWarnedWake && m_fWakeLeft <= 120)
		{
			m_bWarnedWake = true;
			if (m_iCarrier > 0)
				gm.NotifyPlayer(m_iCarrier, "The case", "Something inside the case shifts. Two minutes, maybe less.");
		}

		if (m_fWakeLeft > 0)
			return;

		// It wakes: the case splits and it's loose where the case is
		vector at = m_vItemLastPos;

		RZ_Util.DeleteUnit(m_Item);
		m_Item = null;
		gm.GetSpawner().SpawnZombie(RZ_Util.RandomPointInRing(at, 2, 4), true, -1, -1);
		m_eState = RZ_ECaptureState.LOST;
		m_sLostReason = "The sedative wore off. The case split open from the inside.";
	}

	//------------------------------------------------------------------------------------------------
	//! The special was killed before it was caught (from the mission's OnZombiesKilled).
	void OnSpecialKilled(RZ_GameModeComponent gm, int missionId, bool special)
	{
		if (m_eState != RZ_ECaptureState.BAIT || missionId != m_iMissionId || !special)
			return;

		m_eState = RZ_ECaptureState.LOST;
		m_sLostReason = "It's dead. Halcyon needed it breathing.";
	}

	//------------------------------------------------------------------------------------------------
	//! The case was handed in at an accepted drop (the mission decides which drops count).
	bool Deliver()
	{
		if (m_eState != RZ_ECaptureState.CRATED)
			return false;

		m_eState = RZ_ECaptureState.DELIVERED;
		m_Item = null;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Minutes until it wakes, for the carrier's messages. 0 when not crated.
	int MinutesLeft()
	{
		if (m_eState != RZ_ECaptureState.CRATED)
			return 0;

		return Math.Max(1, RZ_Util.RoundToInt(m_fWakeLeft / 60));
	}

	//------------------------------------------------------------------------------------------------
	protected void NotifyNear(RZ_GameModeComponent gm, vector pos, float radius, string title, string text)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius)
				gm.NotifyPlayer(ids[i], title, text);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Where the objective is done (docs/MISSIONS.md, "Objectives"): the cage while you bait it (the
	//! lure comes to the cage), where it's held while the sedative takes, then the case where it
	//! lies, and once someone carries the case `drop` (where the mission takes it; zero: none).
	RZ_ObjPlace ObjPlace(vector drop)
	{
		switch (m_eState)
		{
			case RZ_ECaptureState.BAIT:
				return RZ_ObjPlace.Spot(m_vCage);
			case RZ_ECaptureState.CAUGHT:
				return RZ_ObjPlace.Spot(m_vHeld, HOLD_RADIUS);
			case RZ_ECaptureState.CRATED:
			{
				if (m_bItemWasCarried && drop != vector.Zero)
					return RZ_ObjPlace.Spot(drop);

				if (m_vItemLastPos == vector.Zero)
					return RZ_ObjPlace.Spot(m_vHeld);

				return RZ_ObjPlace.Spot(m_vItemLastPos);
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The cage and where the special is held, for the mission's GetPropSpots.
	void GetSpots(notnull array<vector> result)
	{
		result.Insert(m_vCage);
		if (m_vHeld != m_vCage)
			result.Insert(m_vHeld);

		if (m_Item && !m_bItemWasCarried)
			result.Insert(m_Item.GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	void Cleanup()
	{
		RZ_Util.DeleteUnit(m_Cage);
		m_Cage = null;
		if (m_Item && m_eState != RZ_ECaptureState.DELIVERED)
			RZ_Util.DeleteUnit(m_Item);

		m_Item = null;
		if (m_bCounted)
		{
			s_iRunning = Math.Max(0, s_iRunning - 1);
			m_bCounted = false;
		}
	}
}
