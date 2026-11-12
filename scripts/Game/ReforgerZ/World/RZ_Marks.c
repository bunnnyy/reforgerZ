//! The marks spawner (docs/DESIGN.md, "Marks"): one prop spawner for every physical thing that
//! marks the world, in separate pools so one never crowds out another.
//!
//! | Pool     | At once | For                                                                  |
//! | SEASON   | 40      | the town log's season marks, memorials                               |
//! | EPILOGUE | 100     | the epilogue's marks                                                 |
//! | PROPS    | 30      | missions' props and chalk, tone's ambient notice prop, the Long      |
//! |          |         | Table's tells                                                        |
//! | SIGNS    | 30      | the tribes' signs                                                    |
//! | DOORS    | 24      | Traces' boards beside marked doors (8 a cluster; up within 60 m)     |
//! | TRACES   | 40      | Traces' props: graves, ash, candles, the scenes (World > Traces; 12 a|
//! |          |         | cluster)                                                             |
//!
//! A mark is data: a place (on the verge, 6-10 m off a road's centre line, when it asks for it),
//! a prefab, a line of text, a salience, how long it lasts. Its prop is spawned only while a player
//! is within 150 m, stays up at least 10 minutes, and goes once nobody is within 200 m. Nothing is
//! saved: marks are runtime only, put back by their owners. With no prefab, or no room in the pool,
//! a mark is text only: TextNear() gives its line to whoever passes (an approach line).
//! Server only.

//------------------------------------------------------------------------------------------------
enum RZ_EMarkPool
{
	SEASON,
	EPILOGUE,
	PROPS,
	SIGNS,
	DOORS,		// Traces' boards beside marked doors
	TRACES		// Traces' props: the player traces and the scenes
}

//------------------------------------------------------------------------------------------------
class RZ_Mark
{
	string m_sKey;
	RZ_EMarkPool m_ePool;
	vector m_vPos;
	float m_fYaw;
	ResourceName m_sPrefab;
	string m_sText;
	int m_iSalience;
	bool m_bProtected;		// a memorial, or a best-slot mark in its first hours: never replaced
	float m_fUntilDay = -1;	// season days; -1 = until removed
	bool m_bTextOnly;
	IEntity m_Entity;
	float m_fUpAt;			// the spawner's clock when the prop went up
}

//------------------------------------------------------------------------------------------------
class RZ_Marks
{
	protected static const float CHECK_SECONDS = 5;
	protected static const float VERGE_SEARCH = 80;
	protected static const int DOOR_BOARDS = 24;
	protected static const int DOOR_BOARDS_CLUSTER = 8;
	protected static const float CLUSTER_RADIUS = 150;

	protected RZ_SharedSettings m_Settings;
	protected ref array<ref RZ_Mark> m_aMarks = {};
	protected float m_fClock;
	protected float m_fTimer;

	//------------------------------------------------------------------------------------------------
	void RZ_Marks(RZ_SharedSettings settings)
	{
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_Marks Get()
	{
		RZ_Shared shared = RZ_Shared.Get();
		if (!shared)
			return null;

		return shared.GetMarks();
	}

	//------------------------------------------------------------------------------------------------
	int Cap(RZ_EMarkPool pool)
	{
		switch (pool)
		{
			case RZ_EMarkPool.SEASON: return m_Settings.m_iSeasonMarks;
			case RZ_EMarkPool.EPILOGUE: return m_Settings.m_iEpilogueMarks;
			case RZ_EMarkPool.PROPS: return m_Settings.m_iPropMarks;
			case RZ_EMarkPool.SIGNS: return m_Settings.m_iSignMarks;
			case RZ_EMarkPool.DOORS: return DOOR_BOARDS;
			case RZ_EMarkPool.TRACES: return RZ_TraceSettings.Get().m_iProps;
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Props of `pool` up at once within 150 m of each other; 0 = no limit.
	protected int ClusterCap(RZ_EMarkPool pool)
	{
		if (pool == RZ_EMarkPool.DOORS)
			return DOOR_BOARDS_CLUSTER;

		if (pool == RZ_EMarkPool.TRACES)
			return RZ_TraceSettings.Get().m_iPropsCluster;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn and despawn radius: Traces' door boards come up within 60 m and go past 90 m.
	protected float SpawnRadius(RZ_EMarkPool pool)
	{
		if (pool == RZ_EMarkPool.DOORS)
			return 60;

		return m_Settings.m_fMarkSpawnRadius;
	}

	//------------------------------------------------------------------------------------------------
	protected float DespawnRadius(RZ_EMarkPool pool)
	{
		if (pool == RZ_EMarkPool.DOORS)
			return 90;

		return m_Settings.m_fMarkDespawnRadius;
	}

	//------------------------------------------------------------------------------------------------
	//! Props of `pool` standing within 150 m of pos.
	protected int SpawnedNear(RZ_EMarkPool pool, vector pos)
	{
		int n = 0;
		foreach (RZ_Mark m : m_aMarks)
		{
			if (m.m_ePool == pool && m.m_Entity && RZ_Util.DistXZ(m.m_vPos, pos) <= CLUSTER_RADIUS)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Marks in `pool` that have a prop (text-only marks don't count).
	int Count(RZ_EMarkPool pool)
	{
		int n = 0;
		foreach (RZ_Mark m : m_aMarks)
		{
			if (m.m_ePool == pool && !m.m_bTextOnly)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Put a mark down (or move the one with this key). onVerge: on the verge of the road nearest
	//! `at` (within 80 m), else `at` itself. days: season days it lasts, < 0 until Remove(). When the
	//! pool is full, the oldest of the lowest salience that isn't protected makes room; if none can,
	//! the new mark is text only. yaw: the facing in degrees (a board facing out of a wall), < 0 any.
	//! True if it has a prop.
	bool Place(RZ_EMarkPool pool, string key, vector at, ResourceName prefab, string text, int salience = 1, float days = -1, bool protect = false, bool onVerge = true, float yaw = -1)
	{
		if (key.IsEmpty())
			return false;

		Remove(key);
		RZ_Mark m = new RZ_Mark();
		m.m_sKey = key;
		m.m_ePool = pool;
		m.m_vPos = at;
		if (onVerge)
			m.m_vPos = Verge(at, m_Settings.m_fVergeMin, m_Settings.m_fVergeMax);

		m.m_fYaw = Math.RandomFloat(0, 360);
		if (yaw >= 0)
			m.m_fYaw = yaw;

		m.m_sPrefab = prefab;
		m.m_sText = text;
		m.m_iSalience = salience;
		m.m_bProtected = protect;
		if (days >= 0)
			m.m_fUntilDay = RZ_SeasonClock.Days() + days;

		m.m_bTextOnly = !m_Settings.m_bMarks || prefab.IsEmpty();
		if (!m.m_bTextOnly && Count(pool) >= Cap(pool) && !MakeRoom(pool, salience))
			m.m_bTextOnly = true;

		m_aMarks.Insert(m);
		return !m.m_bTextOnly;
	}

	//------------------------------------------------------------------------------------------------
	//! Frees a slot in `pool` for a mark of `salience`: the oldest of the lowest salience, not
	//! protected and not above it. The one it replaces becomes text only.
	protected bool MakeRoom(RZ_EMarkPool pool, int salience)
	{
		RZ_Mark drop = null;
		foreach (RZ_Mark m : m_aMarks)
		{
			if (m.m_ePool != pool || m.m_bTextOnly || m.m_bProtected || m.m_iSalience > salience)
				continue;

			if (!drop || m.m_iSalience < drop.m_iSalience)
				drop = m;
		}

		if (!drop)
			return false;

		Despawn(drop);
		drop.m_bTextOnly = true;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void Remove(string key)
	{
		for (int i = m_aMarks.Count() - 1; i >= 0; i--)
		{
			if (m_aMarks[i].m_sKey != key)
				continue;

			Despawn(m_aMarks[i]);
			m_aMarks.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	bool Has(string key)
	{
		foreach (RZ_Mark m : m_aMarks)
		{
			if (m.m_sKey == key)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The line of the nearest mark within `radius` of `pos` (any pool), "" if none: what a passer-by
	//! is told about a text-only mark, or about one whose prop they can't make out.
	string TextNear(vector pos, float radius)
	{
		string best = "";
		float bestDist = radius;
		foreach (RZ_Mark m : m_aMarks)
		{
			float d = RZ_Util.DistXZ(pos, m.m_vPos);
			if (d <= bestDist && !m.m_sText.IsEmpty())
			{
				bestDist = d;
				best = m.m_sText;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Every few seconds: marks past their day go; props go up near players and come down when
	//! nobody's near (after their minutes up).
	void Tick(float dt, array<IEntity> players)
	{
		m_fClock += dt;
		m_fTimer += dt;
		if (m_fTimer < CHECK_SECONDS || m_aMarks.IsEmpty())
			return;

		m_fTimer = 0;
		float today = RZ_SeasonClock.Days();
		for (int i = m_aMarks.Count() - 1; i >= 0; i--)
		{
			RZ_Mark m = m_aMarks[i];
			if (m.m_fUntilDay >= 0 && today > m.m_fUntilDay)
			{
				Despawn(m);
				m_aMarks.Remove(i);
				continue;
			}

			if (m.m_bTextOnly)
				continue;

			float closest = RZ_Util.NearestDistance(m.m_vPos, players);
			int cluster = ClusterCap(m.m_ePool);
			bool clear = m.m_ePool != RZ_EMarkPool.TRACES || m.m_Entity || closest > SpawnRadius(m.m_ePool) || RZ_TraceSpotClear.IsClear(m.m_vPos, closest);
			if (!m.m_Entity && clear && closest <= SpawnRadius(m.m_ePool) && (cluster <= 0 || SpawnedNear(m.m_ePool, m.m_vPos) < cluster))
			{
				m.m_Entity = RZ_Util.SpawnPrefab(m.m_sPrefab, RZ_Util.GroundPos(m.m_vPos), m.m_fYaw);
				m.m_fUpAt = m_fClock;
				if (!m.m_Entity)
					m.m_bTextOnly = true;	// a broken prefab: text from now on
				else if (m.m_ePool == RZ_EMarkPool.DOORS || m.m_ePool == RZ_EMarkPool.TRACES)
					Guard(m);
			}
			else if (m.m_Entity && closest > DespawnRadius(m.m_ePool) && m_fClock - m.m_fUpAt >= m_Settings.m_fMarkMinutes * 60)
			{
				Despawn(m);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Traces' props: nothing a player could use survives (the dressing guard), and the interaction
	//! scan, loot spots and landmark finder leave them alone (marked as dressing).
	protected void Guard(RZ_Mark m)
	{
		if (!RZ_DressingGuard.Strip(m.m_Entity, m.m_sPrefab))
		{
			m.m_Entity = null;
			m.m_bTextOnly = true;
			return;
		}

		RZ_CampLife.AddDressing(m.m_Entity);
	}

	//------------------------------------------------------------------------------------------------
	protected void Despawn(RZ_Mark m)
	{
		if (m.m_Entity)
		{
			RZ_CampLife.RemoveDressing(m.m_Entity);
			SCR_EntityHelper.DeleteEntityAndChildren(m.m_Entity);
		}

		m.m_Entity = null;
	}

	//------------------------------------------------------------------------------------------------
	//! A point on the verge of the road nearest `at`: `minOff` to `maxOff` metres off its centre
	//! line, on the side `at` is on. `at` itself (on the ground) with no road within 80 m.
	static vector Verge(vector at, float minOff, float maxOff)
	{
		RoadNetworkManager roads = RZ_Perimeter.GetRoadNetwork();
		if (!roads)
			return at;

		vector mins = Vector(at[0] - VERGE_SEARCH, -1000, at[2] - VERGE_SEARCH);
		vector maxs = Vector(at[0] + VERGE_SEARCH, 10000, at[2] + VERGE_SEARCH);
		array<BaseRoad> found = {};
		roads.GetRoadsInAABB(mins, maxs, found);
		float best = float.MAX;
		vector a;
		vector b;
		bool any = false;
		foreach (BaseRoad road : found)
		{
			if (!road)
				continue;

			array<vector> points = {};
			road.GetPoints(points);
			for (int i = 0; i + 1 < points.Count(); i++)
			{
				float d = RZ_Util.DistXZ(points[i], at);
				if (d < best)
				{
					best = d;
					a = points[i];
					b = points[i + 1];
					any = true;
				}
			}
		}

		if (!any || best > VERGE_SEARCH)
			return at;

		vector dir = b - a;
		dir[1] = 0;
		float len = dir.Length();
		if (len < 0.1)
			return at;

		dir = dir * (1 / len);
		vector side = Vector(-dir[2], 0, dir[0]);
		vector toAt = at - a;
		if (toAt[0] * side[0] + toAt[2] * side[2] < 0)
			side = side * -1;

		float off = Math.RandomFloat(Math.Min(minOff, maxOff), Math.Max(minOff, maxOff));
		return a + side * off;
	}
}
