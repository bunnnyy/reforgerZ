//! Loose items (docs/DESIGN.md, "Loose items"). Things the mode spawns into the world for players
//! (reward crates, dropped samples, key items, shop purchases, search finds and crafted items that
//! didn't fit in a pack) are tracked here. One left alone for 30 minutes (World > Loose item
//! minutes; 0 = off), with no player within 150 m, is removed, so the world keeps no physical
//! traces and the entity count stays flat.
//!
//! Never removed: anything in an inventory or a vehicle's storage (carried: the clock starts again
//! when it's put down), vehicles, and anything within 300 m of a running mission (its loot, its
//! props). Capped: past the cap the oldest carried entry is forgotten, else the oldest unattended
//! item goes early, else the oldest entry is forgotten (never deleted while it might be in use).

//------------------------------------------------------------------------------------------------
class RZ_LooseItem
{
	IEntity m_Entity;
	float m_fSince;		// clock time it was last carried, near a player or near a running mission
}

//------------------------------------------------------------------------------------------------
class RZ_LooseItems
{
	protected static const float CHECK_INTERVAL = 30;
	protected static const float PLAYER_RADIUS = 150;
	protected static const float MISSION_RADIUS = 300;
	protected static const int CAP = 300;

	protected RZ_GameModeComponent m_GM;
	protected float m_fAbandonSeconds;		// 0 = off
	protected ref array<ref RZ_LooseItem> m_aItems = {};
	protected float m_fClock;
	protected float m_fCheckTimer;

	//------------------------------------------------------------------------------------------------
	void RZ_LooseItems(RZ_GameModeComponent gm, float minutes)
	{
		m_GM = gm;
		m_fAbandonSeconds = Math.Max(0, minutes) * 60;
	}

	//------------------------------------------------------------------------------------------------
	int Count() { return m_aItems.Count(); }

	//------------------------------------------------------------------------------------------------
	//! Start watching a spawned item. Vehicles are never tracked (a bought car isn't litter).
	void Track(IEntity e)
	{
		if (!e || m_fAbandonSeconds <= 0 || Vehicle.Cast(e))
			return;

		foreach (RZ_LooseItem known : m_aItems)
		{
			if (known.m_Entity == e)
				return;
		}

		if (m_aItems.Count() >= CAP)
			MakeRoom();

		RZ_LooseItem item = new RZ_LooseItem();
		item.m_Entity = e;
		item.m_fSince = m_fClock;
		m_aItems.Insert(item);
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<IEntity> players)
	{
		m_fClock += dt;
		m_fCheckTimer += dt;
		if (m_fCheckTimer < CHECK_INTERVAL)
			return;

		m_fCheckTimer = 0;
		array<vector> missions = {};
		RunningMissionSpots(missions);

		// Ordered removal keeps the oldest entries first (MakeRoom)
		for (int i = m_aItems.Count() - 1; i >= 0; i--)
		{
			RZ_LooseItem item = m_aItems[i];
			IEntity e = item.m_Entity;
			if (!e)
			{
				m_aItems.RemoveOrdered(i);
				continue;
			}

			if (IsInUse(e, players, missions))
			{
				item.m_fSince = m_fClock;
				continue;
			}

			if (m_fClock - item.m_fSince < m_fAbandonSeconds)
				continue;

			m_aItems.RemoveOrdered(i);
			SCR_EntityHelper.DeleteEntityAndChildren(e);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Carried (in a pack, a vehicle's storage, a container), with a player near, or at a running
	//! mission: it stays, and its clock starts again.
	protected bool IsInUse(IEntity e, array<IEntity> players, array<vector> missions)
	{
		if (IsStored(e))
			return true;

		vector pos = e.GetOrigin();
		if (RZ_Util.NearestDistance(pos, players) <= PLAYER_RADIUS)
			return true;

		foreach (vector spot : missions)
		{
			if (RZ_Util.DistXZ(spot, pos) <= MISSION_RADIUS)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! In an inventory or any storage: attached to something, or sitting in a storage slot.
	protected static bool IsStored(IEntity e)
	{
		IEntity root = e.GetRootParent();
		if (root && root != e)
			return true;

		InventoryItemComponent inv = InventoryItemComponent.Cast(e.FindComponent(InventoryItemComponent));
		if (!inv)
			return false;

		return inv.GetParentSlot() != null;
	}

	//------------------------------------------------------------------------------------------------
	protected void RunningMissionSpots(notnull array<vector> result)
	{
		RZ_MissionManager missions = m_GM.GetMissions();
		if (!missions)
			return;

		foreach (RZ_Mission m : missions.GetActive())
		{
			if (m && m.m_eState == RZ_EMissionState.ACTIVE)
				result.Insert(m.m_vPos);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! At the cap: forget the oldest carried entry; else remove the oldest item nobody is using;
	//! else forget the oldest entry.
	protected void MakeRoom()
	{
		for (int i = 0; i < m_aItems.Count(); i++)
		{
			IEntity e = m_aItems[i].m_Entity;
			if (!e || IsStored(e))
			{
				m_aItems.RemoveOrdered(i);
				return;
			}
		}

		array<IEntity> players = {};
		array<int> ids = {};
		RZ_Util.GetAlivePlayers(ids, players);
		array<vector> missions = {};
		RunningMissionSpots(missions);
		for (int j = 0; j < m_aItems.Count(); j++)
		{
			IEntity old = m_aItems[j].m_Entity;
			if (IsInUse(old, players, missions))
				continue;

			m_aItems.RemoveOrdered(j);
			SCR_EntityHelper.DeleteEntityAndChildren(old);
			return;
		}

		m_aItems.RemoveOrdered(0);
	}
}
