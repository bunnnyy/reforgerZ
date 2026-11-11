// Test-only (not part of the mod): helpers for the behaviour tests B1-B20 (RZL_Hooks.c has the hooks).

//------------------------------------------------------------------------------------------------
//! The key our speed caps are stored under (SCR_ChimeraCharacter.SetSpeedLimit keeps one per source).
class RZL_SpeedSource
{
}

//------------------------------------------------------------------------------------------------
class RZL_B
{
	static ref RZL_SpeedSource s_Speed;

	//------------------------------------------------------------------------------------------------
	static Bacon_622120A5448725E3_InfectedCharacter Inf(IEntity e)
	{
		return Bacon_622120A5448725E3_InfectedCharacter.Cast(e);
	}

	//------------------------------------------------------------------------------------------------
	//! The perception factor: through stealth's RZ_TrySetPerceptionFactor (stores it for the AI's own
	//! update, applies it if the AI is ready), and straight on the perception if not. Returns the path.
	static string Factor(IEntity e, float f)
	{
		if (!e)
			return "none";

		string path = "none";
		SCR_AICombatComponent combat = SCR_AICombatComponent.Cast(e.FindComponent(SCR_AICombatComponent));
		if (combat)
		{
			if (combat.RZ_TrySetPerceptionFactor(f))
				return "combat";

			path = "stored";
		}

		PerceptionComponent perc = PerceptionComponent.Cast(e.FindComponent(PerceptionComponent));
		if (perc)
		{
			perc.SetPerceptionFactor(f);
			path += "+direct";
		}
		return path;
	}

	//------------------------------------------------------------------------------------------------
	static void Mode(IEntity e, int mode)
	{
		Bacon_622120A5448725E3_InfectedCharacter inf = Inf(e);
		if (inf)
			inf.m_iRZLMode = mode;
	}

	//------------------------------------------------------------------------------------------------
	static void Slot(IEntity e, vector slot)
	{
		Bacon_622120A5448725E3_InfectedCharacter inf = Inf(e);
		if (!inf)
			return;

		inf.m_bRZLSlot = true;
		inf.m_vRZLSlot = RZL.Ground(slot);
	}

	//------------------------------------------------------------------------------------------------
	static void ClearSlot(IEntity e)
	{
		Bacon_622120A5448725E3_InfectedCharacter inf = Inf(e);
		if (inf)
			inf.m_bRZLSlot = false;
	}

	//------------------------------------------------------------------------------------------------
	static vector SlotOf(IEntity e)
	{
		Bacon_622120A5448725E3_InfectedCharacter inf = Inf(e);
		if (!inf)
			return vector.Zero;

		return inf.m_vRZLSlot;
	}

	//------------------------------------------------------------------------------------------------
	//! STEER: blind, intake closed, holding a slot.
	static string Steer(IEntity e, vector slot)
	{
		Mode(e, 1);
		Slot(e, slot);
		return Factor(e, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! A Bacon body steered from its spawn frame: fields set before it joins its group, the factor set
	//! before and right after AddAIEntityToGroup. Returns "before path / after path".
	static IEntity SpawnSteered(string prefab, vector pos, float yaw, vector slot, notnull array<IEntity> groupsOut, out string path)
	{
		SCR_AIGroup g = RZL.SpawnEmptyGroup(pos);
		IEntity ch = RZL.Spawn(prefab, pos, yaw);
		groupsOut.Insert(g);
		path = "-";
		if (!ch)
			return null;

		Mode(ch, 1);
		Slot(ch, slot);
		string before = Factor(ch, 0);
		if (g)
			g.AddAIEntityToGroup(ch);
		string after = Factor(ch, 0);
		RZL.ForceLOD(ch, 0);
		path = before + " / " + after;
		return ch;
	}

	//------------------------------------------------------------------------------------------------
	//! Our share event at the body itself, naming the target (L10).
	static void Inject(IEntity body, IEntity target)
	{
		if (!body || !target)
			return;

		AIDangerEvent ev = new AIDangerEvent();
		ev.SetDangerType(EAIDangerEventType.BaconZombies_TargetSharing);
		ev.SetVictim(body);
		ev.SetObject(target);
		ev.SetPosition(body.GetOrigin());
		GetGame().GetAIWorld().RequestBroadcastDangerEvent(ev);
	}

	//------------------------------------------------------------------------------------------------
	//! Release to Bacon (STEER to ENGAGE): factor back, intake open, slot dropped, optionally injected.
	static void Release(IEntity body, IEntity target, bool inject)
	{
		Mode(body, 2);
		ClearSlot(body);
		Factor(body, 1);
		if (inject)
			Inject(body, target);
	}

	//------------------------------------------------------------------------------------------------
	//! A target that can't be hurt (so a fight lasts as long as the test needs).
	static IEntity Target(vector pos, float yaw, bool invulnerable)
	{
		IEntity t = RZL.Spawn(RZL.TARGET, pos, yaw);
		if (invulnerable)
			Invulnerable(t, true);
		return t;
	}

	//------------------------------------------------------------------------------------------------
	static void Invulnerable(IEntity t, bool on)
	{
		ChimeraCharacter c = ChimeraCharacter.Cast(t);
		if (!c || !c.GetDamageManager())
			return;

		c.GetDamageManager().EnableDamageHandling(!on);
	}

	//------------------------------------------------------------------------------------------------
	//! Engaged: counts the target an enemy, or Bacon picked it as its target in the last 1.5 s.
	static bool Engaged(IEntity body, IEntity target)
	{
		if (!body || !target)
			return false;

		if (RZL.Category(body, target) == ETargetCategory.ENEMY)
			return true;

		Bacon_622120A5448725E3_InfectedCharacter inf = Inf(body);
		return inf && inf.m_RZLBroadcastTarget == target && RZL_Hooks.Now() - inf.m_fRZLBroadcastAt < 1.5;
	}

	//------------------------------------------------------------------------------------------------
	//! L4: a speed cap, as a fraction (1 clears it). Through SCR_ChimeraCharacter.SetSpeedLimit, instant.
	static void Cap(IEntity e, float fraction)
	{
		SCR_ChimeraCharacter c = SCR_ChimeraCharacter.Cast(e);
		if (!c)
			return;

		if (!s_Speed)
			s_Speed = new RZL_SpeedSource();
		c.SetSpeedLimit(s_Speed, fraction, true);
	}

	//------------------------------------------------------------------------------------------------
	//! L5: the movement type our code wants (EMovementType).
	static bool WantMove(IEntity e, EMovementType type)
	{
		if (!e)
			return false;

		AICharacterMovementComponent mc = AICharacterMovementComponent.Cast(e.FindComponent(AICharacterMovementComponent));
		if (!mc)
			return false;

		mc.SetMovementTypeWanted(type);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static int MoveWanted(IEntity e)
	{
		if (!e)
			return -1;

		AICharacterMovementComponent mc = AICharacterMovementComponent.Cast(e.FindComponent(AICharacterMovementComponent));
		if (!mc)
			return -1;

		return mc.GetMovementTypeWanted();
	}

	//------------------------------------------------------------------------------------------------
	static CharacterControllerComponent Ctrl(IEntity e)
	{
		ChimeraCharacter c = ChimeraCharacter.Cast(e);
		if (!c)
			return null;

		return c.GetCharacterController();
	}

	//------------------------------------------------------------------------------------------------
	//! Depth of water at a point (0 = dry), from the engine's water surface.
	static float WaterDepth(vector pos)
	{
		vector ground = RZL.Ground(pos);
		vector probe = Vector(ground[0], ground[1] + 0.2, ground[2]);
		vector surface;
		EWaterSurfaceType type;
		vector transform[4];
		vector extents;
		if (!ChimeraWorldUtils.TryGetWaterSurface(GetGame().GetWorld(), probe, surface, type, transform, extents))
			return 0;

		if (type == EWaterSurfaceType.WST_NONE)
			return 0;

		return Math.Max(0, surface[1] - ground[1]);
	}

	//------------------------------------------------------------------------------------------------
	static EWaterSurfaceType WaterType(vector pos)
	{
		vector ground = RZL.Ground(pos);
		vector probe = Vector(ground[0], ground[1] + 0.2, ground[2]);
		vector surface;
		EWaterSurfaceType type;
		vector transform[4];
		vector extents;
		if (!ChimeraWorldUtils.TryGetWaterSurface(GetGame().GetWorld(), probe, surface, type, transform, extents))
			return EWaterSurfaceType.WST_NONE;

		return type;
	}

	//------------------------------------------------------------------------------------------------
	//! Our slot check (section 7.1): a straight line from a to b crosses water (1 m steps).
	static bool CrossesWater(vector a, vector b)
	{
		float len = RZL.DistXZ(a, b);
		int n = len;
		for (int i = 0; i <= n; i++)
		{
			float fi = i;
			float f = 1;
			if (n > 0)
				f = fi / n;
			if (WaterDepth(a + (b - a) * f) > 0.05)
				return true;
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Our snap (L18): the closest navmesh point within `half` metres, through the group's pathfinding.
	static bool Snap(IEntity groupOrChar, vector pos, float half, out vector snapped)
	{
		snapped = pos;
		if (!groupOrChar)
			return false;

		AIPathfindingComponent pf = AIPathfindingComponent.Cast(groupOrChar.FindComponent(AIPathfindingComponent));
		if (!pf)
			return false;

		return pf.GetClosestPositionOnNavmesh(pos, Vector(half, half, half), snapped);
	}

	//------------------------------------------------------------------------------------------------
	//! A physics line from a to b (buildings, walls, props; not terrain): true if clear.
	static bool TraceClear(vector a, vector b, IEntity ex1, IEntity ex2)
	{
		TraceParam tp = new TraceParam();
		tp.Start = a;
		tp.End = b;
		tp.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
		tp.LayerMask = EPhysicsLayerPresets.Projectile;
		array<IEntity> ex = {};
		if (ex1)
			ex.Insert(ex1);
		if (ex2)
			ex.Insert(ex2);
		tp.ExcludeArray = ex;
		float f = GetGame().GetWorld().TraceMove(tp, null);
		return f >= 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Terrain line of sight by sampling GetSurfaceY every `step` metres (the addendum's 5.5 step 1).
	static bool TerrainClear(vector a, float ha, vector b, float hb, float step)
	{
		float len = RZL.DistXZ(a, b);
		int n = len / step;
		if (n < 2)
			return true;

		float ya = a[1] + ha;
		float yb = b[1] + hb;
		BaseWorld world = GetGame().GetWorld();
		for (int i = 1; i < n; i++)
		{
			float fi = i;
			float f = fi / n;
			float x = a[0] + (b[0] - a[0]) * f;
			float z = a[2] + (b[2] - a[2]) * f;
			if (world.GetSurfaceY(x, z) > ya + (yb - ya) * f)
				return false;
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static float Height(IEntity e)
	{
		if (!e)
			return -99;

		return e.GetOrigin()[1] - GetGame().GetWorld().GetSurfaceY(e.GetOrigin()[0], e.GetOrigin()[2]);
	}

	//------------------------------------------------------------------------------------------------
	static float Health(IEntity e)
	{
		ChimeraCharacter c = ChimeraCharacter.Cast(e);
		if (!c || !c.GetDamageManager())
			return -1;

		return c.GetDamageManager().GetHealthScaled();
	}

	//------------------------------------------------------------------------------------------------
	//! A static prop (wall, fence) spawned with its local box centred on `centre`, its length across `yaw`.
	static IEntity SpawnProp(string prefab, vector centre, float yaw, vector localMins, vector localMaxs)
	{
		vector c = (localMins + localMaxs) * 0.5;
		vector right = vector.FromYaw(yaw + 90);
		vector fwd = vector.FromYaw(yaw);
		vector origin = centre - right * c[0] - fwd * c[2];
		return RZL.Spawn(prefab, origin, yaw);
	}
}

//------------------------------------------------------------------------------------------------
//! Wall prefabs for the climb course (B8, B9, B17), measured once at start (spawned far away, read,
//! deleted): height above the pivot, length, and whether it's destructible.
class RZL_Walls
{
	static ref array<string> s_aPrefabs = {
		"{458419F02F587136}Prefabs/Structures/Walls/Wooden/WoodenFence_04/WoodenFence_04_8m.et",
		"{AB044B4917B05086}Prefabs/Structures/Walls/Wooden/WoodenFence_01/WoodenFence_01_8m.et",
		"{A1A11B04A1A0A301}Prefabs/Structures/Walls/Wooden/WoodenFence_02/WoodenFence_02_8m.et",
		"{666D8B9954F6E00D}Prefabs/Structures/Walls/Wooden/WoodenFence_03/WoodenFence_03_8m.et",
		"{422C2BC6C73E34D2}Prefabs/Structures/Walls/Wooden/WoodenFenceOld_01/WoodenFenceOld_01_8m.et",
		"{5EE5B7AA967CD253}Prefabs/Structures/Walls/Wooden/WoodenRural_01/WoodenRural_01_8m.et",
		"{EEE23553967B32A8}Prefabs/Structures/Walls/Stone/StoneWall_01/StoneWall_01_8m.et",
		"{42BA975ABE8D6175}Prefabs/Structures/Walls/Stone/StoneWall_03/StoneWall_03_8m_A.et",
		"{A00A15B03B48A2AF}Prefabs/Structures/Walls/Brick/BrickWall_01/BrickWall_01_8m.et",
		"{AAAF45FD8D585128}Prefabs/Structures/Walls/Brick/BrickWall_02/BrickWall_02_8m.et",
		"{2FBF5DEDFA25D947}Prefabs/Structures/Walls/Concrete/ConcreteWall_01/ConcreteWall_01_6m_A.et",
		"{8B4B7B5842D5C255}Prefabs/Structures/Walls/Net/NetFence_05/NetFence_05_short_6m_V1.et",
		"{BECA0D0A34726F45}Prefabs/Structures/Walls/Net/NetFence_05/NetFence_05_tall_6m_V1.et",
		"{15BE3A6617F91789}Prefabs/Structures/Walls/Pipe/PipeRailing_01/PipeRailing_01_6m.et",
		"{AE9E6F639440A51E}Prefabs/Structures/Walls/Wooden/WoodenWall_01/WoodenWall_01_8m.et",
		"{F55802C087BC218D}Prefabs/Structures/Walls/Metal/TinWall_01/TinWall_01_6m_A.et",
		"{052DDF0E3A5E2AF6}Prefabs/Structures/Walls/Metal/MetalWall_01/MetalWall_01_9m.et",
		"{425B3F37AE13AE3C}Prefabs/Structures/Walls/Crashbarrier/CrashBarrier_01/CrashBarrier_01_8m_V1.et",
		"{6B08AEEC99A1CFE8}Prefabs/Structures/Walls/Stone/StoneWall_02/StoneWall_02_4m_v1.et",
		"{CBB323D90F2FF29F}Prefabs/Structures/Walls/Net/NetFence_02/NetFence_02_8m.et",
		"{262BF377567092C5}Prefabs/Structures/Walls/Brick/BrickWallOld_01/BrickWallOld_01_8m_A.et"
	};
	static ref array<float> s_aHeight = {};
	static ref array<float> s_aLength = {};
	static ref array<vector> s_aMins = {};
	static ref array<vector> s_aMaxs = {};
	static ref array<bool> s_aDestructible = {};
	static ref array<bool> s_aAlongX = {};
	static bool s_bMeasured;

	//------------------------------------------------------------------------------------------------
	static void Measure(vector farAway)
	{
		if (s_bMeasured)
			return;

		s_bMeasured = true;
		for (int i = 0; i < s_aPrefabs.Count(); i++)
		{
			IEntity e = RZL.Spawn(s_aPrefabs[i], farAway + Vector(i * 15, 0, 0), 0);
			vector mins = vector.Zero;
			vector maxs = vector.Zero;
			bool destr = false;
			if (e)
			{
				e.GetBounds(mins, maxs);
				destr = SCR_DestructibleEntity.Cast(e) != null;
			}
			s_aMins.Insert(mins);
			s_aMaxs.Insert(maxs);
			s_aHeight.Insert(maxs[1]);
			bool alongX = maxs[0] - mins[0] >= maxs[2] - mins[2];
			s_aAlongX.Insert(alongX);
			float len = maxs[2] - mins[2];
			if (alongX)
				len = maxs[0] - mins[0];
			s_aLength.Insert(len);
			s_aDestructible.Insert(destr);
			string cls = "none";
			if (e)
				cls = e.ClassName();
			RZL.Row("walls", string.Format("i=%1|prefab=%2|spawned=%3|class=%4|height=%5|bottom=%6|length=%7|depth=%8|destructible=%9", i, FilePath.StripPath(s_aPrefabs[i]), e != null, cls, RZL.F2(maxs[1]), RZL.F2(mins[1]), RZL.F2(len), RZL.F2(Math.Min(maxs[0] - mins[0], maxs[2] - mins[2])), destr));
			RZL.Del(e);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The prefab whose height is nearest `h` (only spawned ones). -1 if none.
	static int Nearest(float h, bool wantDestructible)
	{
		int best = -1;
		float bestD = 1000;
		for (int i = 0; i < s_aHeight.Count(); i++)
		{
			if (s_aLength[i] < 2)
				continue;

			if (wantDestructible && !s_aDestructible[i])
				continue;

			float d = Math.AbsFloat(s_aHeight[i] - h);
			if (d < bestD)
			{
				bestD = d;
				best = i;
			}
		}
		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! A straight wall of `pieces` pieces of prefab i, centred on `centre`, across the approach `yaw`.
	static void Build(int i, vector centre, float yaw, int pieces, notnull array<IEntity> props)
	{
		if (i < 0)
			return;

		float len = s_aLength[i];
		float propYaw = yaw;
		if (!s_aAlongX[i])
			propYaw = yaw + 90;
		vector right = vector.FromYaw(yaw + 90);
		for (int k = 0; k < pieces; k++)
		{
			float off = (k - (pieces - 1) * 0.5) * len;
			IEntity e = RZL_B.SpawnProp(s_aPrefabs[i], centre + right * off, propYaw, s_aMins[i], s_aMaxs[i]);
			if (e)
				props.Insert(e);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! A behaviour test: an experiment that also cleans up props it spawned.
class RZL_BExp : RZL_Exp
{
	ref array<IEntity> m_aProps = {};

	//------------------------------------------------------------------------------------------------
	protected void ClearProps()
	{
		foreach (IEntity p : m_aProps)
			RZL.Del(p);
		m_aProps.Clear();
	}

	//------------------------------------------------------------------------------------------------
	override protected void Finish()
	{
		ClearProps();
		super.Finish();
	}

	//------------------------------------------------------------------------------------------------
	//! Mean and max horizontal speed over a window, from a mover's samples.
	protected static string Spd(float sum, int n, float mx)
	{
		if (n <= 0)
			return "-1";

		return RZL.F2(sum / n) + " (max " + RZL.F2(mx) + ")";
	}
}

//------------------------------------------------------------------------------------------------
//! Experiments run one after another at one site.
class RZL_Lane : RZL_Exp
{
	ref array<ref RZL_Exp> m_aSeq = {};
	protected int m_iCur;

	//------------------------------------------------------------------------------------------------
	void RZL_Lane(string name)
	{
		m_sName = name;
	}

	//------------------------------------------------------------------------------------------------
	void Add(RZL_Exp e)
	{
		if (e && RZL_LabComponent.Wanted(e.m_sName))
			m_aSeq.Insert(e);
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float now)
	{
		if (m_iCur >= m_aSeq.Count())
		{
			m_bDone = true;
			RZL.Log(m_sName + " done");
			return;
		}

		RZL_Exp e = m_aSeq[m_iCur];
		if (!e.m_Site)
			e.m_Site = m_Site;
		if (!e.m_bStarted)
			e.Begin(now);
		e.Tick(now);
		if (e.m_bDone)
			m_iCur++;
	}
}
