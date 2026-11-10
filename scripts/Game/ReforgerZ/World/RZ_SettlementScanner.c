//! Finds settlements on whatever map is loaded. This is what makes the mode work on any terrain.
//!
//! Order of preference:
//!  1. RZ_SettlementMarkerEntity placed by hand (full control).
//!  2. The map's own name labels (MapDescriptorComponent of type city/town/village).
//!  3. Clusters of buildings with no city/town/village label nearby: named after the nearest
//!     local label (hamlets such as Arland's Highfield), else "Outskirts" and a grid reference.
//!
//! The scan touches every static entity once, so it can take a few seconds on big maps.
//! The result is saved with the rest of the state, so it only runs on a fresh start or a new map.
class RZ_SettlementScanner
{
	protected ref array<ref RZ_Settlement> m_aOut;
	protected ref map<int, int> m_mGridCount = new map<int, int>();
	protected ref map<int, vector> m_mGridSum = new map<int, vector>();
	protected float m_fCell = 300;
	protected ref array<string> m_aLocalNames = {};
	protected ref array<vector> m_aLocalPos = {};

	protected static const int GRID_STRIDE = 100000;
	protected static const float LOCAL_NAME_RANGE = 400;

	//------------------------------------------------------------------------------------------------
	void Scan(float cellSize, int minBuildings, notnull array<ref RZ_Settlement> outSettlements)
	{
		m_aOut = outSettlements;
		m_fCell = cellSize;

		// 1. Hand-placed markers
		array<RZ_SettlementMarkerEntity> markers = RZ_SettlementMarkerEntity.GetAll();
		if (!markers.IsEmpty())
		{
			foreach (RZ_SettlementMarkerEntity marker : markers)
			{
				m_aOut.Insert(marker.ToSettlement());
			}

			RZ_Log.Info(string.Format("Using %1 hand-placed settlement markers.", m_aOut.Count()));
			return;
		}

		// 2 + 3. One pass over the world collects name labels and buildings
		float start = System.GetTickCount();
		BaseWorld world = GetGame().GetWorld();
		vector mins, maxs;
		world.GetBoundBox(mins, maxs);
		world.QueryEntitiesByAABB(mins, maxs, OnEntity, null, EQueryEntitiesFlags.STATIC);

		int named = m_aOut.Count();
		AddBuildingClusters(minBuildings);

		RZ_Log.Info(string.Format("Settlement scan: %1 named, %2 building clusters, took %3 ms.",
			named, m_aOut.Count() - named, System.GetTickCount() - start));
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnEntity(IEntity e)
	{
		MapDescriptorComponent descr = MapDescriptorComponent.Cast(e.FindComponent(MapDescriptorComponent));
		if (descr)
			TryAddDescriptor(e, descr);

		// Map buildings are SCR_DestructibleBuildingEntity (the Arland test found its clusters this way).
		if (SCR_DestructibleBuildingEntity.Cast(e))
			AddBuilding(e.GetOrigin());

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void TryAddDescriptor(IEntity e, MapDescriptorComponent descr)
	{
		int type = descr.GetBaseType();
		int size = 0;
		if (type == EMapDescriptorType.MDT_NAME_CITY)
			size = 3;
		else if (type == EMapDescriptorType.MDT_NAME_TOWN)
			size = 2;
		else if (type == EMapDescriptorType.MDT_NAME_VILLAGE || type == EMapDescriptorType.MDT_NAME_SETTLEMENT)
			size = 1;

		if (type == EMapDescriptorType.MDT_NAME_LOCAL)
		{
			// Hamlets and spots (Arland's Highfield, Newfield): too small to be a settlement on
			// their own, but they name the building cluster around them.
			string local = LabelName(descr);
			if (!local.IsEmpty())
			{
				m_aLocalNames.Insert(local);
				m_aLocalPos.Insert(e.GetOrigin());
			}

			return;
		}

		if (size == 0)
			return;

		vector pos = RZ_Util.GroundPos(e.GetOrigin());
		foreach (RZ_Settlement existing : m_aOut)
		{
			if (RZ_Util.DistXZ(existing.m_vCenter, pos) < 150)
				return;
		}

		string name = LabelName(descr);
		if (name.IsEmpty())
			name = "Settlement";

		RZ_Settlement s = new RZ_Settlement();
		s.m_sName = name;
		s.m_vCenter = pos;
		s.m_iSize = size;
		s.m_fRadius = RZ_Settlement.DefaultRadiusForSize(size);
		s.m_fInfection = StartInfectionForSize(size);
		m_aOut.Insert(s);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddBuilding(vector pos)
	{
		int gx = Math.Floor(pos[0] / m_fCell);
		int gz = Math.Floor(pos[2] / m_fCell);
		int key = gx * GRID_STRIDE + gz;

		int count;
		m_mGridCount.Find(key, count);
		m_mGridCount.Set(key, count + 1);

		vector sum;
		m_mGridSum.Find(key, sum);
		m_mGridSum.Set(key, sum + pos);
	}

	//------------------------------------------------------------------------------------------------
	//! Building clusters away from any named place become settlements of their own.
	protected void AddBuildingClusters(int minBuildings)
	{
		foreach (int key, int count : m_mGridCount)
		{
			if (count < minBuildings)
				continue;

			vector sum = m_mGridSum.Get(key);
			vector center = RZ_Util.GroundPos(sum / count);

			bool nearNamed = false;
			foreach (RZ_Settlement named : m_aOut)
			{
				if (RZ_Util.DistXZ(named.m_vCenter, center) < named.m_fRadius + m_fCell * 0.5)
				{
					nearNamed = true;
					break;
				}
			}

			if (nearNamed)
				continue;

			RZ_Settlement cluster = new RZ_Settlement();
			cluster.m_sName = ClusterName(center);
			cluster.m_vCenter = center;
			cluster.m_iSize = 1;
			cluster.m_fRadius = m_fCell * 0.6;
			cluster.m_fInfection = StartInfectionForSize(1);
			m_aOut.Insert(cluster);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The label's name in English. Labels hold string-table keys (#AR-MapLocation_...), which
	//! the server translates with its own language (English on a dedicated server).
	protected static string LabelName(MapDescriptorComponent descr)
	{
		MapItem item = descr.Item();
		if (!item)
			return "";

		return RZ_Util.Localize(item.GetDisplayName());
	}

	//------------------------------------------------------------------------------------------------
	//! A cluster takes the nearest local place name within LOCAL_NAME_RANGE; a second cluster
	//! near the same name is its outskirts; otherwise (or with no name near) the grid reference.
	protected string ClusterName(vector center)
	{
		int best = -1;
		float bestDist = LOCAL_NAME_RANGE;
		for (int i = 0; i < m_aLocalPos.Count(); i++)
		{
			float d = RZ_Util.DistXZ(m_aLocalPos[i], center);
			if (d < bestDist)
			{
				bestDist = d;
				best = i;
			}
		}

		if (best < 0)
			return string.Format("Outskirts %1", GridReference(center));

		string name = m_aLocalNames[best];
		if (!NameTaken(name))
			return name;

		string outskirts = string.Format("%1 outskirts", name);
		if (!NameTaken(outskirts))
			return outskirts;

		return string.Format("Outskirts %1", GridReference(center));
	}

	//------------------------------------------------------------------------------------------------
	protected bool NameTaken(string name)
	{
		foreach (RZ_Settlement other : m_aOut)
		{
			if (other.m_sName == name)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	static string GridReference(vector pos)
	{
		int gx = Math.Floor(pos[0] / 100);
		int gz = Math.Floor(pos[2] / 100);
		return string.Format("%1-%2", gx, gz);
	}

	//------------------------------------------------------------------------------------------------
	//! Bigger places start worse off; a little randomness so no two servers start the same.
	protected float StartInfectionForSize(int size)
	{
		return Math.Clamp(10 + size * 12 + Math.RandomFloat(-10, 15), 0, 100);
	}
}
