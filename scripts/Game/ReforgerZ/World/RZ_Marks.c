//! The marks spawner (docs/DESIGN.md, "Marks"): one prop spawner for every physical thing that
//! marks the world, in separate pools so one never crowds out another.
//!
//! | Pool     | At once | For                                                                  |
//! | SEASON   | 40      | the town log's season marks, memorials                               |
//! | EPILOGUE | 100     | the epilogue's marks                                                 |
//! | PROPS    | 30      | missions' props and chalk, tone's ambient notice prop, the Long      |
//! |          |         | Table's tells                                                        |
//! | SIGNS    | 30      | the tribes' signs                                                    |
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
	SIGNS
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
		}

		return 0;
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
	//! the new mark is text only. True if it has a prop.
	bool Place(RZ_EMarkPool pool, string key, vector at, ResourceName prefab, string text, int salience = 1, float days = -1, bool protect = false, bool onVerge = true)
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
			if (!m.m_Entity && closest <= m_Settings.m_fMarkSpawnRadius)
			{
				m.m_Entity = RZ_Util.SpawnPrefab(m.m_sPrefab, RZ_Util.GroundPos(m.m_vPos), m.m_fYaw);
				m.m_fUpAt = m_fClock;
				if (!m.m_Entity)
					m.m_bTextOnly = true;	// a broken prefab: text from now on
			}
			else if (m.m_Entity && closest > m_Settings.m_fMarkDespawnRadius && m_fClock - m.m_fUpAt >= m_Settings.m_fMarkMinutes * 60)
			{
				Despawn(m);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Despawn(RZ_Mark m)
	{
		if (m.m_Entity)
			SCR_EntityHelper.DeleteEntityAndChildren(m.m_Entity);

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
