// Test-only (not part of the mod): helpers for the zombie lab (tools/zombie_lab/).
class RZL
{
	// Bacon Zombies 1.8.0 prefabs
	static const string CIV = "{6734045E1A07091D}622120A5448725E3/Prefabs/Characters/Variants/Variant_CIV_Regular1.et";
	static const string CIV_BASE = "{1228A830F9454F4B}622120A5448725E3/Prefabs/Characters/Zombie_CIV.et";
	static const string MIL = "{2A5BF0C801520AE5}622120A5448725E3/Prefabs/Characters/Variants/Variant_US_Rifleman.et";
	static const string RUNNER = "{84862F078179934E}622120A5448725E3/Prefabs/Characters/Variants/Variant_Exploder1.et";
	static const string HEAVY = "{7F6C5AD53D6B039D}622120A5448725E3/Prefabs/Characters/Heavy/Zombie_Heavy_Tank.et";
	static const string BOMBER = "{CC9EC4802FA55B64}622120A5448725E3/Prefabs/Characters/BlobThrower/Character_Bomber.et";
	static const string GROUP_EMPTY = "{E15B91D284F1ED15}622120A5448725E3/Prefabs/Groups/Group_Zombies_Base.et";
	static const string GROUP_CIV = "{67E76BA71804B965}622120A5448725E3/Prefabs/Groups/Group_Zombies_CIV.et";
	// Vanilla
	static const string TARGET = "{8C7093AF368F496A}Prefabs/Characters/Factions/CIV/GenericCivilians/Character_CIV_CottonShirt_1.et";
	static const string SHOOTER = "{26A9756790131354}Prefabs/Characters/Factions/BLUFOR/US_Army/Character_US_Rifleman.et";
	static const string WAYPOINT = "{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et";

	//------------------------------------------------------------------------------------------------
	static void Log(string s)
	{
		Print("RZL " + s);
	}

	//------------------------------------------------------------------------------------------------
	//! A result row for tools/zombie_lab/report.py: "RZL R|table|key=value|key=value"
	static void Row(string table, string kv)
	{
		Print("RZL R|" + table + "|" + kv);
	}

	//------------------------------------------------------------------------------------------------
	static string F(float v)
	{
		return v.ToString(-1, 1);
	}

	//------------------------------------------------------------------------------------------------
	static string F2(float v)
	{
		return v.ToString(-1, 2);
	}

	//------------------------------------------------------------------------------------------------
	static float Now()
	{
		return GetGame().GetWorld().GetWorldTime() * 0.001;
	}

	//------------------------------------------------------------------------------------------------
	static vector Ground(vector p)
	{
		p[1] = GetGame().GetWorld().GetSurfaceY(p[0], p[2]);
		return p;
	}

	//------------------------------------------------------------------------------------------------
	static float DistXZ(vector a, vector b)
	{
		float dx = a[0] - b[0];
		float dz = a[2] - b[2];
		return Math.Sqrt(dx * dx + dz * dz);
	}

	//------------------------------------------------------------------------------------------------
	static float YawTo(vector from, vector to)
	{
		return Math.Atan2(to[0] - from[0], to[2] - from[2]) * Math.RAD2DEG;
	}

	//------------------------------------------------------------------------------------------------
	//! A ground point at a distance along a yaw (degrees) from a point.
	static vector At(vector origin, float yaw, float dist)
	{
		vector d = vector.FromYaw(yaw);
		return Ground(origin + d * dist);
	}

	//------------------------------------------------------------------------------------------------
	static IEntity Spawn(string prefab, vector pos, float yaw)
	{
		Resource res = Resource.Load(prefab);
		if (!res || !res.IsValid())
		{
			Log("SPAWN FAILED (invalid prefab) " + prefab);
			return null;
		}

		EntitySpawnParams p = new EntitySpawnParams();
		p.TransformMode = ETransformMode.WORLD;
		Math3D.AnglesToMatrix(Vector(yaw, 0, 0), p.Transform);
		p.Transform[3] = Ground(pos);
		return GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), p);
	}

	//------------------------------------------------------------------------------------------------
	static SCR_AIGroup SpawnEmptyGroup(vector pos)
	{
		return SCR_AIGroup.Cast(Spawn(GROUP_EMPTY, pos, 0));
	}

	//------------------------------------------------------------------------------------------------
	//! A Bacon character added to a group with AddAIEntityToGroup (the design's runtime group).
	static IEntity SpawnMember(SCR_AIGroup g, string prefab, vector pos, float yaw)
	{
		IEntity ch = Spawn(prefab, pos, yaw);
		if (!ch || !g)
			return ch;

		bool added = g.AddAIEntityToGroup(ch);
		if (!added)
			Log("AddAIEntityToGroup returned false for " + prefab);
		return ch;
	}

	//------------------------------------------------------------------------------------------------
	//! A one-member runtime group around a fresh character, LOD forced to 0 (as if a player were near).
	static IEntity SpawnSolo(string prefab, vector pos, float yaw, notnull array<IEntity> groupsOut)
	{
		SCR_AIGroup g = SpawnEmptyGroup(pos);
		IEntity ch = SpawnMember(g, prefab, pos, yaw);
		groupsOut.Insert(g);
		ForceLOD(ch, 0);
		return ch;
	}

	//------------------------------------------------------------------------------------------------
	static AIAgent Agent(IEntity ch)
	{
		if (!ch)
			return null;

		AIControlComponent ctrl = AIControlComponent.Cast(ch.FindComponent(AIControlComponent));
		if (!ctrl)
			return null;

		return ctrl.GetControlAIAgent();
	}

	//------------------------------------------------------------------------------------------------
	static void ForceLOD(IEntity ch, int lod)
	{
		AIAgent a = Agent(ch);
		if (a)
			a.SetPermanentLOD(lod);
	}

	//------------------------------------------------------------------------------------------------
	static int LOD(IEntity ch)
	{
		AIAgent a = Agent(ch);
		if (!a)
			return -9;

		return a.GetLOD();
	}

	//------------------------------------------------------------------------------------------------
	static bool AIActive(IEntity ch)
	{
		AIAgent a = Agent(ch);
		if (!a)
			return false;

		return a.IsAIActivated();
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the perception factor the way vanilla does (through SCR_AICombatComponent, which multiplies
	//! it with its threat-state factor), or straight on the PerceptionComponent if that isn't there.
	static string SetFactor(IEntity ch, float f)
	{
		if (!ch)
			return "none";

		SCR_AICombatComponent combat = SCR_AICombatComponent.Cast(ch.FindComponent(SCR_AICombatComponent));
		AIAgent a = Agent(ch);
		SCR_AIUtilityComponent util;
		if (a)
			util = SCR_AIUtilityComponent.Cast(a.FindComponent(SCR_AIUtilityComponent));
		if (combat && util && util.m_ThreatSystem)
		{
			combat.SetPerceptionFactor(f);
			return "combat";
		}

		PerceptionComponent perc = PerceptionComponent.Cast(ch.FindComponent(PerceptionComponent));
		if (perc)
		{
			perc.SetPerceptionFactor(f);
			return "direct";
		}

		return "none";
	}

	//------------------------------------------------------------------------------------------------
	//! Perception state of observer toward target: -1 not known, else ETargetCategory.
	static int Category(IEntity observer, IEntity target)
	{
		if (!observer || !target)
			return -1;

		PerceptionComponent perc = PerceptionComponent.Cast(observer.FindComponent(PerceptionComponent));
		if (!perc)
			return -2;

		BaseTarget bt = perc.FindTargetPerceptionObject(target);
		if (!bt)
			return -1;

		return bt.GetTargetCategory();
	}

	//------------------------------------------------------------------------------------------------
	//! In the observer's perception and seen within the last second
	static bool Seen(IEntity observer, IEntity target)
	{
		if (Category(observer, target) < 0)
			return false;

		float since = SinceSeen(observer, target);
		return since >= 0 && since < 1;
	}

	//------------------------------------------------------------------------------------------------
	//! How far terrain rises above the sight line from a (eye ha above ground) to b (hb above ground):
	//! negative = clear by that much, positive = hidden by that much.
	static float LosBlock(vector a, float ha, vector b, float hb)
	{
		float ya = a[1] + ha;
		float yb = b[1] + hb;
		float worst = -100;
		for (int i = 1; i < 30; i++)
		{
			float fi = i;
			float f = fi / 30;
			vector p = Ground(a + (b - a) * f);
			worst = Math.Max(worst, p[1] - (ya + (yb - ya) * f));
		}
		return worst;
	}

	//------------------------------------------------------------------------------------------------
	static float SinceSeen(IEntity observer, IEntity target)
	{
		if (!observer || !target)
			return -1;

		PerceptionComponent perc = PerceptionComponent.Cast(observer.FindComponent(PerceptionComponent));
		if (!perc)
			return -1;

		BaseTarget bt = perc.FindTargetPerceptionObject(target);
		if (!bt)
			return -1;

		return bt.GetTimeSinceSeen();
	}

	//------------------------------------------------------------------------------------------------
	static bool Alive(IEntity e)
	{
		ChimeraCharacter c = ChimeraCharacter.Cast(e);
		if (!c)
			return false;

		SCR_DamageManagerComponent dmg = c.GetDamageManager();
		if (!dmg)
			return true;

		return dmg.GetState() != EDamageState.DESTROYED;
	}

	//------------------------------------------------------------------------------------------------
	static void Kill(IEntity e)
	{
		ChimeraCharacter c = ChimeraCharacter.Cast(e);
		if (!c)
			return;

		SCR_DamageManagerComponent dmg = c.GetDamageManager();
		if (dmg)
			dmg.Kill(Instigator.CreateInstigator(null));
	}

	//------------------------------------------------------------------------------------------------
	static void Del(IEntity e)
	{
		if (e)
			SCR_EntityHelper.DeleteEntityAndChildren(e);
	}

	//------------------------------------------------------------------------------------------------
	//! Deletes groups and characters in a list (characters first).
	static void DelAll(array<IEntity> chars, array<IEntity> groups)
	{
		if (chars)
		{
			foreach (IEntity c : chars)
				Del(c);
			chars.Clear();
		}

		if (groups)
		{
			foreach (IEntity g : groups)
				Del(g);
			groups.Clear();
		}
	}

	//------------------------------------------------------------------------------------------------
	static void Teleport(IEntity e, vector pos, float yaw)
	{
		if (!e)
			return;

		vector mat[4];
		Math3D.AnglesToMatrix(Vector(yaw, 0, 0), mat);
		mat[3] = Ground(pos);
		// Characters need the engine's teleport (SetWorldTransform alone leaves their physics behind)
		BaseGameEntity bge = BaseGameEntity.Cast(e);
		if (bge)
		{
			bge.Teleport(mat);
			return;
		}

		e.SetWorldTransform(mat);
		e.Update();
	}

	//------------------------------------------------------------------------------------------------
	static void SetLoiter(IEntity ch, vector pos)
	{
		Bacon_622120A5448725E3_InfectedCharacter inf = Bacon_622120A5448725E3_InfectedCharacter.Cast(ch);
		if (inf)
			inf.SetLoiterPoint(pos);
	}

	//------------------------------------------------------------------------------------------------
	static AIWaypoint NewWaypoint(vector pos)
	{
		return AIWaypoint.Cast(Spawn(WAYPOINT, pos, 0));
	}

	//------------------------------------------------------------------------------------------------
	static int WaypointCount(SCR_AIGroup g)
	{
		if (!g)
			return -1;

		array<AIWaypoint> wps = {};
		g.GetWaypoints(wps);
		return wps.Count();
	}

	//------------------------------------------------------------------------------------------------
	static bool HasWaypoint(SCR_AIGroup g, AIWaypoint wp)
	{
		if (!g || !wp)
			return false;

		array<AIWaypoint> wps = {};
		g.GetWaypoints(wps);
		return wps.Find(wp) >= 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The fix the design proposes: one waypoint per group, moved and re-added on each order.
	static void MoveHeld(SCR_AIGroup g, AIWaypoint wp, vector pos)
	{
		if (!g || !wp)
			return;

		if (HasWaypoint(g, wp))
			g.RemoveWaypoint(wp);
		wp.SetOrigin(Ground(pos));
		wp.Update();
		g.AddWaypoint(wp);
	}

	//------------------------------------------------------------------------------------------------
	static int CountAlive(array<IEntity> list)
	{
		int n;
		foreach (IEntity e : list)
		{
			if (e)
				n++;
		}
		return n;
	}

	//------------------------------------------------------------------------------------------------
	static vector GroupPos(SCR_AIGroup g)
	{
		if (!g)
			return vector.Zero;

		IEntity leader = g.GetLeaderEntity();
		if (leader)
			return leader.GetOrigin();

		return g.GetOrigin();
	}

	//------------------------------------------------------------------------------------------------
	static string FactionKey(IEntity e)
	{
		FactionAffiliationComponent fac = FactionAffiliationComponent.Cast(e.FindComponent(FactionAffiliationComponent));
		if (!fac || !fac.GetAffiliatedFaction())
			return "none";

		return fac.GetAffiliatedFaction().GetFactionKey();
	}

	//------------------------------------------------------------------------------------------------
	static string TimeOfDay()
	{
		TimeAndWeatherManagerEntity tm = Weather();
		if (!tm)
			return "?";

		return F2(tm.GetTimeOfTheDay());
	}

	//------------------------------------------------------------------------------------------------
	static TimeAndWeatherManagerEntity Weather()
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return null;

		return world.GetTimeAndWeatherManager();
	}
}

//------------------------------------------------------------------------------------------------
//! One tracked character: position history for speed and heading.
class RZL_Mover
{
	IEntity m_Ent;
	vector m_vLast;
	float m_fLastAt = -1;
	float m_fSpeed;
	vector m_vVel;

	void RZL_Mover(IEntity e)
	{
		m_Ent = e;
		if (e)
			m_vLast = e.GetOrigin();
		m_fLastAt = RZL.Now();
	}

	//------------------------------------------------------------------------------------------------
	//! Call regularly; keeps the speed over the last interval (horizontal).
	void Sample()
	{
		if (!m_Ent)
			return;

		float t = RZL.Now();
		float dt = t - m_fLastAt;
		if (dt < 0.2)
			return;

		vector p = m_Ent.GetOrigin();
		vector d = p - m_vLast;
		d[1] = 0;
		m_fSpeed = d.Length() / dt;
		m_vVel = d * (1 / dt);
		m_vLast = p;
		m_fLastAt = t;
	}

	//------------------------------------------------------------------------------------------------
	//! After a teleport: forget the jump.
	void Reset()
	{
		if (m_Ent)
			m_vLast = m_Ent.GetOrigin();
		m_fLastAt = RZL.Now();
		m_fSpeed = 0;
		m_vVel = vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	//! Moving toward a point faster than minSpeed (m/s)?
	bool Toward(vector pos, float minSpeed)
	{
		if (!m_Ent || m_fSpeed < minSpeed)
			return false;

		vector to = pos - m_Ent.GetOrigin();
		to[1] = 0;
		float len = to.Length();
		if (len < 0.5)
			return true;

		float dot = vector.Dot(m_vVel, to) / (len * m_fSpeed);
		return dot > 0.6;
	}
}

//------------------------------------------------------------------------------------------------
//! Collects trees (and other vegetation) near a point.
class RZL_TreeCounter
{
	int m_iCount;
	ref array<vector> m_aPos = {};
	bool m_bKeep;

	//------------------------------------------------------------------------------------------------
	int Count(vector center, float radius, bool keepPositions)
	{
		m_iCount = 0;
		m_aPos.Clear();
		m_bKeep = keepPositions;
		GetGame().GetWorld().QueryEntitiesBySphere(center, radius, OnEntity, null, EQueryEntitiesFlags.STATIC);
		return m_iCount;
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnEntity(IEntity e)
	{
		if (BaseTree.Cast(e))
		{
			// only trees tall enough to hide a standing man (bushes don't count)
			vector mins, maxs;
			e.GetWorldBounds(mins, maxs);
			if (maxs[1] - mins[1] < 3)
				return true;

			m_iCount++;
			if (m_bKeep)
				m_aPos.Insert(e.GetOrigin());
		}
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! Loose inventory items (dropped, no parent) near a point.
class RZL_ItemCounter
{
	int m_iCount;

	//------------------------------------------------------------------------------------------------
	int Count(vector center, float radius)
	{
		m_iCount = 0;
		GetGame().GetWorld().QueryEntitiesBySphere(center, radius, OnEntity, null, EQueryEntitiesFlags.DYNAMIC);
		return m_iCount;
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnEntity(IEntity e)
	{
		if (!e.GetParent() && e.FindComponent(InventoryItemComponent))
			m_iCount++;
		return true;
	}
}
