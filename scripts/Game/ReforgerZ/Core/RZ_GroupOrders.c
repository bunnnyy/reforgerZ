//! An AI group's one held move waypoint (docs/DESIGN.md, Principles: one move waypoint per group).
//! Every order moves it and puts it back on the group's list; it's deleted with the unit.
class RZ_HeldWaypoint
{
	IEntity m_Group;			// weak: null once the group is gone (it deletes itself when its last member dies)
	AIWaypoint m_Waypoint;		// spawned on the first order, then moved: one entity for the group's life

	//------------------------------------------------------------------------------------------------
	//! Takes the waypoint off the group's list if it's still on it, and deletes it.
	void DeleteWaypoint()
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(m_Group);
		if (group && m_Waypoint && RZ_Util.GroupHasWaypoint(group, m_Waypoint))
			group.RemoveWaypoint(m_Waypoint);

		if (m_Waypoint)
			SCR_EntityHelper.DeleteEntityAndChildren(m_Waypoint);

		m_Waypoint = null;
	}
}

//! Move orders for AI groups, one held waypoint each, so no order spawns an entity and none is left
//! lying in the world. Measured with Bacon Zombies (tools/zombie_lab/): a finished waypoint drops
//! off the group's list but stays in the world; a deleted group leaves its pending waypoint behind;
//! moving a waypoint that's still on the list does nothing until it's taken off and added again;
//! taken off, moved and re-added, one waypoint served 8 orders in a row. RZ_Util.SendGroupTo and
//! RedirectGroup go through here, and RZ_Util.DeleteUnit deletes a group's waypoint with it. The
//! spawner's tracked records own their group's (RZ_TrackedZombie.m_Held). A group that dies out by
//! itself loses its waypoint on the next server tick (Sweep).
class RZ_GroupOrders
{
	protected static ref array<ref RZ_HeldWaypoint> s_aHeld;

	//------------------------------------------------------------------------------------------------
	//! The held waypoint record of an AI group, made if it has none (no waypoint is spawned until
	//! the first order). Null for anything that isn't a group.
	static RZ_HeldWaypoint Hold(IEntity e)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(e);
		if (!group)
			return null;

		RZ_HeldWaypoint found = Find(group);
		if (found)
			return found;

		if (!s_aHeld)
			s_aHeld = {};

		RZ_HeldWaypoint made = new RZ_HeldWaypoint();
		made.m_Group = group;
		s_aHeld.Insert(made);
		return made;
	}

	//------------------------------------------------------------------------------------------------
	//! The group's held waypoint record, or null.
	static RZ_HeldWaypoint Find(IEntity group)
	{
		if (!group || !s_aHeld)
			return null;

		foreach (RZ_HeldWaypoint h : s_aHeld)
		{
			if (h.m_Group == group)
				return h;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Sends an AI group to target with its held waypoint: moved there and put back at the end of the
	//! group's list, after any waypoints of its own ahead of it. With clearOthers, every other
	//! waypoint is taken off the list and deleted first, so the group goes straight there. The first
	//! order spawns the waypoint from waypointPrefab. Returns the waypoint, or null (not a group, or
	//! no waypoint prefab).
	static AIWaypoint Order(IEntity e, vector target, ResourceName waypointPrefab, bool clearOthers)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(e);
		if (!group || waypointPrefab.IsEmpty())
			return null;

		Sweep();
		RZ_HeldWaypoint h = Hold(group);

		array<AIWaypoint> current = {};
		group.GetWaypoints(current);
		foreach (AIWaypoint wp : current)
		{
			if (!wp)
				continue;

			if (wp == h.m_Waypoint)
			{
				group.RemoveWaypoint(wp);
				continue;
			}

			if (!clearOthers)
				continue;

			group.RemoveWaypoint(wp);
			SCR_EntityHelper.DeleteEntityAndChildren(wp);
		}

		if (h.m_Waypoint)
		{
			// Moving it while it's on the list does nothing (measured): it was taken off above
			h.m_Waypoint.SetOrigin(target);
			h.m_Waypoint.Update();
		}
		else
		{
			IEntity spawned = RZ_Util.SpawnPrefab(waypointPrefab, target);
			h.m_Waypoint = AIWaypoint.Cast(spawned);
			if (!h.m_Waypoint)
			{
				if (spawned)
				{
					RZ_Log.Warn("Not a waypoint prefab: " + waypointPrefab);
					SCR_EntityHelper.DeleteEntityAndChildren(spawned);
				}

				return null;
			}
		}

		group.AddWaypoint(h.m_Waypoint);
		return h.m_Waypoint;
	}

	//------------------------------------------------------------------------------------------------
	//! Deletes an AI group's held waypoint and forgets the group (RZ_Util.DeleteUnit).
	static void Release(IEntity group)
	{
		if (!group || !s_aHeld)
			return;

		for (int i = s_aHeld.Count() - 1; i >= 0; i--)
		{
			if (s_aHeld[i].m_Group != group)
				continue;

			s_aHeld[i].DeleteWaypoint();
			s_aHeld.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Deletes a record's waypoint and forgets it (the spawner, when it lets a tracked group go).
	static void Drop(RZ_HeldWaypoint h)
	{
		if (!h)
			return;

		h.DeleteWaypoint();
		if (s_aHeld)
			s_aHeld.RemoveItem(h);
	}

	//------------------------------------------------------------------------------------------------
	//! Deletes the waypoints of groups that are gone and forgets them. Every server tick and before
	//! each order. Returns how many records went.
	static int Sweep()
	{
		if (!s_aHeld)
			return 0;

		int gone = 0;
		for (int i = s_aHeld.Count() - 1; i >= 0; i--)
		{
			RZ_HeldWaypoint h = s_aHeld[i];
			if (h.m_Group)
				continue;

			h.DeleteWaypoint();
			s_aHeld.Remove(i);
			gone++;
		}

		return gone;
	}

	//------------------------------------------------------------------------------------------------
	//! Held waypoints in the world now (the status line): one per group with an order at most.
	static int CountWaypoints()
	{
		if (!s_aHeld)
			return 0;

		int n = 0;
		foreach (RZ_HeldWaypoint h : s_aHeld)
		{
			if (h.m_Waypoint)
				n++;
		}

		return n;
	}
}
