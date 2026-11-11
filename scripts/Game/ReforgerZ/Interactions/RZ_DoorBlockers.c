//! Keeping AI and the infected out of locked buildings (docs/DESIGN.md, "Locks and keys").
//! They don't use the door action, so RZ_DoorHooks.c can't stop them.
//!
//! - Blockers: while a locked place's outside door is shut and locked and a player is within
//!   range (Search > blocker range, 150 m), a thin invisible collision box (Search > doorway
//!   blocker) stands in the doorway, just outside the door leaf. At most Search > blocker cap
//!   (20) on the server, the doorways nearest to players first.
//! - Navmesh: when a blocker comes or goes, the navmesh around that doorway is rebuilt
//!   (SCR_AIWorld, as Game Master placement does), so AI route around the locked door instead
//!   of opening it. Requests are queued, merged per doorway and sent two per tick. Off
//!   (Search > navmesh rebuild) = blockers only: AI walk into them and stop.
//! - Cooldown: a doorway whose blocker came or went can't get one back for Search > blocker
//!   cooldown (60 s), so lock and unlock spam can't force rebuilds. Removing is never delayed.
//! - Banging: a tracked infected pressing at a locked doorway (blocked or not) makes heat,
//!   at most once per Search > bang seconds per doorway, and players nearby hear it.
//!
//! Safety: a blocker never traps a player. It only appears while the door is shut and
//! locked, with nobody within 2.5 m of the door and nobody inside the building. It goes at
//! once on unlock, force or the inside latch (RZ_DoorLocks.OnLockChanged), and on the next
//! tick when a player is inside the building or on the inside of an open locked door.
//! Blockers are never saved: a restart starts with none.

//------------------------------------------------------------------------------------------------
//! One locked doorway near players: its blocker (if any) and its timers.
class RZ_DoorBlock
{
	string m_sKey;				// building key + "#" + the door's index in its building
	string m_sBuildingKey;
	IEntity m_Door;
	IEntity m_Blocker;
	bool m_bPlaced;				// a blocker was spawned here (the navmesh must be rebuilt when it goes)
	vector m_vPlacedAt;
	vector m_vCentre;			// the doorway, learned while the door was shut
	bool m_bHasCentre;
	float m_fChangedAt = -100000;
	float m_fNextBang;
	float m_fNextNotice;
	float m_fSeen;
}

//------------------------------------------------------------------------------------------------
class RZ_DoorwayCandidate
{
	string m_sKey;
	RZ_BuildingDoors m_Building;
	int m_iDoor;
	float m_fDist;
}

//------------------------------------------------------------------------------------------------
//! The engine side of a local navmesh rebuild, kept in one place.
class RZ_NavRebuild
{
	//------------------------------------------------------------------------------------------------
	//! Rebuilds the navmesh in a small box around `centre`. False if there's no AI world.
	static bool RequestArea(vector centre, float half)
	{
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld)
			return false;

		array<ref Tuple2<vector, vector>> areas = {};
		array<bool> redoRoads = {};
		vector extent = Vector(half, half, half);
		areas.Insert(new Tuple2<vector, vector>(centre - extent, centre + extent));
		redoRoads.Insert(false);
		aiWorld.RequestNavmeshRebuildAreas(areas, redoRoads); // what destruction and Game Master placement use
		return true;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_DoorBlockers
{
	protected static const float SPAWN_CLEAR = 2.5;		// no blocker appears with a player this close (m) to the door
	protected static const float INSIDE_CLEAR = 4;		// a player this close to an open locked door, on its inside, removes the blocker
	protected static const float OUTSIDE_OFFSET = 0.1;	// the blocker stands this far (m) outside the door leaf
	protected static const float BANG_RADIUS = 1.8;
	protected static const float NOTICE_RADIUS = 30;
	protected static const float NOTICE_SECONDS = 90;
	protected static const float REBUILD_HALF = 2.5;	// the rebuilt box is 5 m across
	protected static const int REBUILDS_PER_TICK = 2;
	protected static const int REBUILD_QUEUE_MAX = 60;
	protected static const float DISCOVER_SECONDS = 30;
	protected static const float DISCOVER_MOVE = 40;
	protected static const int DISCOVER_MAX = 300;
	protected static const float INSIDE_CHECK = 60;		// players further than this from a building aren't checked for being inside it
	protected static const float FORGET_SECONDS = 600;	// a doorway's timers are dropped after this long unwanted

	protected RZ_GameModeComponent m_GM;
	protected RZ_Interactions m_Hub;
	protected RZ_DoorLocks m_Doors;
	protected RZ_SearchSettings m_Settings;
	protected ref map<string, ref RZ_DoorBlock> m_mBlocks = new map<string, ref RZ_DoorBlock>();
	protected ref array<vector> m_aRebuild = {};
	protected ref map<int, float> m_mDiscoverAt = new map<int, float>();
	protected ref map<int, vector> m_mDiscoverPos = new map<int, vector>();
	protected ref array<IEntity> m_aFound = {};
	protected int m_iDiscoverNext;

	//------------------------------------------------------------------------------------------------
	void RZ_DoorBlockers(RZ_GameModeComponent gm, RZ_Interactions hub, RZ_DoorLocks doors, RZ_SearchSettings settings)
	{
		m_GM = gm;
		m_Hub = hub;
		m_Doors = doors;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	int CountBlocked()
	{
		int n = 0;
		foreach (string key, RZ_DoorBlock k : m_mBlocks)
		{
			if (k.m_Blocker)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! A blocker stands within radius of pos (mission spots keep clear of them, RZ_LockedSpots).
	bool IsNear(vector pos, float radius)
	{
		foreach (string key, RZ_DoorBlock k : m_mBlocks)
		{
			if (k.m_Blocker && RZ_Util.DistXZ(k.m_vPlacedAt, pos) <= radius)
				return true;
		}

		return false;
	}

	// ================================================================ Lock changes
	//------------------------------------------------------------------------------------------------
	//! A building was unlocked, latched, forced or locked. Unlocked: its blockers go now.
	void OnLockChanged(string buildingKey)
	{
		RZ_BuildingDoors b;
		if (m_Doors.GetBuildings().Find(buildingKey, b) && m_Doors.IsLockedNow(b))
			return;

		float now = m_Hub.GetClock();
		foreach (string key, RZ_DoorBlock k : m_mBlocks)
		{
			if (k.m_sBuildingKey == buildingKey)
				RemoveBlocker(k, now);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Locks off: every blocker goes.
	void ClearAll()
	{
		if (m_mBlocks.IsEmpty())
			return;

		float now = m_Hub.GetClock();
		foreach (string key, RZ_DoorBlock k : m_mBlocks)
		{
			RemoveBlocker(k, now);
		}

		m_mBlocks.Clear();
		SendRebuilds();
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(array<int> ids, array<IEntity> players)
	{
		float now = m_Hub.GetClock();
		Discover(ids, players, now);

		// The locked doorways near players, nearest first, up to the cap
		array<ref RZ_DoorwayCandidate> cands = {};
		Collect(players, cands);
		array<int> order = {};
		int wanted = cands.Count();
		if (wanted > m_Settings.m_iBlockerCap)
			wanted = Math.Max(0, m_Settings.m_iBlockerCap);
		while (order.Count() < wanted)
		{
			int best = -1;
			for (int i = 0; i < cands.Count(); i++)
			{
				if (order.Find(i) >= 0)
					continue;

				if (best < 0 || cands[i].m_fDist < cands[best].m_fDist)
					best = i;
			}

			if (best < 0)
				break;

			order.Insert(best);
		}

		array<string> keep = {};
		foreach (int idx : order)
		{
			RZ_DoorwayCandidate c = cands[idx];
			keep.Insert(c.m_sKey);
			RZ_DoorBlock k = Ensure(c);
			k.m_fSeen = now;
			if (!k.m_Blocker && !RZ_DoorGeometry.IsOpen(k.m_Door))
			{
				k.m_vCentre = RZ_DoorGeometry.DoorwayCentre(k.m_Door);
				k.m_bHasCentre = true;
			}

			if (k.m_bPlaced && !k.m_Blocker)
				RemoveBlocker(k, now);	// deleted by something else: rebuild without it

			if (k.m_Blocker && PlayerOnInside(c.m_Building, k, players))
				RemoveBlocker(k, now);
			else if (!k.m_Blocker)
				MaybeSpawn(k, c.m_Building, players, now);

			Bang(k, now);
		}

		// Doorways no longer wanted (unlocked, out of range, past the cap, someone inside)
		array<string> drop = {};
		foreach (string key, RZ_DoorBlock old : m_mBlocks)
		{
			if (keep.Find(key) >= 0)
				continue;

			if (old.m_bPlaced)
				RemoveBlocker(old, now);

			if (now - old.m_fSeen > FORGET_SECONDS || !old.m_Door)
				drop.Insert(key);
		}

		foreach (string gone : drop)
		{
			m_mBlocks.Remove(gone);
		}

		SendRebuilds();
	}

	//------------------------------------------------------------------------------------------------
	//! Every outside door of a learned, locked place within range, unless a player is inside it.
	protected void Collect(array<IEntity> players, notnull array<ref RZ_DoorwayCandidate> cands)
	{
		float range = m_Settings.m_fBlockerRange;
		foreach (string bkey, RZ_BuildingDoors b : m_Doors.GetBuildings())
		{
			if (!b.m_Building || b.m_aDoors.IsEmpty())
				continue;

			if (RZ_Util.NearestDistance(b.m_vPos, players) > range + 30)
				continue;

			if (!m_Doors.IsLockedNow(b) || AnyoneInside(b.m_Building, players))
				continue;

			foreach (int i, IEntity door : b.m_aDoors)
			{
				if (!door)
					continue;

				float d = RZ_Util.NearestDistance(door.GetOrigin(), players);
				if (d > range)
					continue;

				RZ_DoorwayCandidate c = new RZ_DoorwayCandidate();
				c.m_sKey = b.m_sKey + "#" + i.ToString();
				c.m_Building = b;
				c.m_iDoor = i;
				c.m_fDist = d;
				cands.Insert(c);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_DoorBlock Ensure(RZ_DoorwayCandidate c)
	{
		RZ_DoorBlock k;
		if (m_mBlocks.Find(c.m_sKey, k))
		{
			if (!k.m_Door)
				k.m_Door = c.m_Building.m_aDoors[c.m_iDoor];

			return k;
		}

		k = new RZ_DoorBlock();
		k.m_sKey = c.m_sKey;
		k.m_sBuildingKey = c.m_Building.m_sKey;
		k.m_Door = c.m_Building.m_aDoors[c.m_iDoor];
		m_mBlocks.Set(c.m_sKey, k);
		return k;
	}

	//------------------------------------------------------------------------------------------------
	protected bool AnyoneInside(IEntity building, array<IEntity> players)
	{
		vector at = building.GetOrigin();
		foreach (IEntity p : players)
		{
			if (!p || RZ_Util.DistXZ(p.GetOrigin(), at) > INSIDE_CHECK)
				continue;

			if (RZ_DoorGeometry.InBuilding(building, p.GetOrigin()))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A player on the inside of this door, close to it (say an AI opened it and the player
	//! wants out): the blocker must go.
	protected bool PlayerOnInside(RZ_BuildingDoors b, RZ_DoorBlock k, array<IEntity> players)
	{
		if (!k.m_Door)
			return false;

		vector at = k.m_Door.GetOrigin();
		foreach (IEntity p : players)
		{
			if (!p || RZ_Util.DistXZ(p.GetOrigin(), at) > INSIDE_CLEAR)
				continue;

			if (RZ_DoorGeometry.IsInside(b.m_Building, k.m_Door, p.GetOrigin()))
				return true;
		}

		return false;
	}

	// ================================================================ Blockers
	//------------------------------------------------------------------------------------------------
	protected void MaybeSpawn(RZ_DoorBlock k, RZ_BuildingDoors b, array<IEntity> players, float now)
	{
		ResourceName prefab = m_Settings.m_sDoorBlockerPrefab;
		if (prefab.IsEmpty() || !k.m_Door || !k.m_bHasCentre)
			return;

		if (now - k.m_fChangedAt < m_Settings.m_fBlockerCooldown && now >= k.m_fChangedAt)
			return;

		if (RZ_DoorGeometry.IsOpen(k.m_Door))
			return;

		if (RZ_Util.NearestDistance(k.m_Door.GetOrigin(), players) <= SPAWN_CLEAR || RZ_Util.NearestDistance(k.m_vCentre, players) <= SPAWN_CLEAR)
			return;

		vector n = RZ_DoorGeometry.WallNormal(b.m_Building, k.m_Door);
		if (n.Length() < 0.5)
			return;

		vector at = k.m_vCentre + n * OUTSIDE_OFFSET;
		IEntity e = SpawnFacing(prefab, at, n);
		if (!e)
			return;

		k.m_Blocker = e;
		k.m_bPlaced = true;
		k.m_vPlacedAt = at;
		k.m_fChangedAt = now;
		QueueRebuild(at);
	}

	//------------------------------------------------------------------------------------------------
	//! Removing is never delayed (safety). The navmesh follows on the queue.
	protected void RemoveBlocker(RZ_DoorBlock k, float now)
	{
		if (!k.m_bPlaced && !k.m_Blocker)
			return;

		if (k.m_Blocker)
			SCR_EntityHelper.DeleteEntityAndChildren(k.m_Blocker);

		k.m_Blocker = null;
		if (k.m_bPlaced)
			QueueRebuild(k.m_vPlacedAt);

		k.m_bPlaced = false;
		k.m_fChangedAt = now;
	}

	//------------------------------------------------------------------------------------------------
	//! The blocker's local Z points out of the wall, X along it, Y up (no yaw convention needed).
	protected IEntity SpawnFacing(ResourceName prefab, vector pos, vector normal)
	{
		Resource res = Resource.Load(prefab);
		if (!res || !res.IsValid())
		{
			RZ_Log.Warn("Invalid doorway blocker prefab: " + prefab);
			return null;
		}

		vector up = Vector(0, 1, 0);
		vector side = Vector(normal[2], 0, -normal[0]);	// up x normal
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[0] = side;
		params.Transform[1] = up;
		params.Transform[2] = normal;
		params.Transform[3] = pos;
		return GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params);
	}

	// ================================================================ Navmesh
	//------------------------------------------------------------------------------------------------
	//! One request per doorway: a queued one already covers whatever the doorway is when it's sent.
	protected void QueueRebuild(vector pos)
	{
		if (!m_Settings.m_bNavmeshRebuild)
			return;

		foreach (vector q : m_aRebuild)
		{
			if (vector.Distance(q, pos) < 1)
				return;
		}

		if (m_aRebuild.Count() >= REBUILD_QUEUE_MAX)
			m_aRebuild.RemoveOrdered(0);

		m_aRebuild.Insert(pos);
	}

	//------------------------------------------------------------------------------------------------
	protected void SendRebuilds()
	{
		if (!m_Settings.m_bNavmeshRebuild)
		{
			m_aRebuild.Clear();
			return;
		}

		int sent = 0;
		while (!m_aRebuild.IsEmpty() && sent < REBUILDS_PER_TICK)
		{
			if (!RZ_NavRebuild.RequestArea(m_aRebuild[0], REBUILD_HALF))
			{
				m_aRebuild.Clear();
				return;
			}

			m_aRebuild.RemoveOrdered(0);
			sent++;
		}
	}

	// ================================================================ Banging
	//------------------------------------------------------------------------------------------------
	//! An infected pressing at the locked doorway: heat, like a barricade, rate-limited.
	protected void Bang(RZ_DoorBlock k, float now)
	{
		if (m_Settings.m_fDoorBangHeat <= 0 || now < k.m_fNextBang || !k.m_Door)
			return;

		vector at = k.m_Door.GetOrigin();
		if (k.m_bHasCentre)
			at = k.m_vCentre;

		if (!m_GM.GetSpawner().IsZombieNear(at, BANG_RADIUS))
			return;

		m_GM.GetHeat().AddHeat(at, m_Settings.m_fDoorBangHeat);
		k.m_fNextBang = now + Math.Max(1, m_Settings.m_fDoorBangSeconds);
		if (now < k.m_fNextNotice)
			return;

		k.m_fNextNotice = now + NOTICE_SECONDS;
		m_Hub.NotifyNear(at, NOTICE_RADIUS, -1, "Banging", "Something is hammering on a locked door close by. It isn't giving up.");
	}

	// ================================================================ Finding locked places
	//------------------------------------------------------------------------------------------------
	//! One player per tick, every 30 s and only after moving 40 m: locked places within range
	//! are learned (the look-around only reaches 30 m).
	protected void Discover(array<int> ids, array<IEntity> players, float now)
	{
		if (players.IsEmpty())
			return;

		if (m_iDiscoverNext >= players.Count())
			m_iDiscoverNext = 0;

		int i = m_iDiscoverNext;
		m_iDiscoverNext++;
		IEntity p = players[i];
		if (!p || RZ_Util.IsInVehicle(p))
			return;

		int id = ids[i];
		vector pos = p.GetOrigin();
		float last;
		if (m_mDiscoverAt.Find(id, last))
		{
			if (now - last < DISCOVER_SECONDS && now >= last)
				return;

			vector lastPos;
			if (m_mDiscoverPos.Find(id, lastPos) && RZ_Util.DistXZ(lastPos, pos) < DISCOVER_MOVE)
			{
				m_mDiscoverAt.Set(id, now);
				return;
			}
		}

		m_mDiscoverAt.Set(id, now);
		m_mDiscoverPos.Set(id, pos);
		m_aFound.Clear();
		GetGame().GetWorld().QueryEntitiesBySphere(pos, m_Settings.m_fBlockerRange, OnFound, null, EQueryEntitiesFlags.STATIC);
		foreach (IEntity e : m_aFound)
		{
			if (e)
				m_Doors.LearnIfLocked(e, RZ_ItemHelper.LowerName(e));
		}

		m_aFound.Clear();
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnFound(IEntity e)
	{
		if (m_aFound.Count() < DISCOVER_MAX && SCR_DestructibleBuildingEntity.Cast(e))
			m_aFound.Insert(e);

		return true;
	}
}
