// Test-only site audit driver (tools/runtime_test/run.sh). Not part of the mod.
// In the site audit's own worlds (run.sh sites): waits for the land grid, measures every camp-kit
// prop's real box ("RZS footprint ..." lines, for tools/camp_kit/footprints.json), lays out every
// site on the map as data with camp life's headless audit (RZ_CampLife.AuditAll), prints the
// summary and closes the server. In the full runtime test it runs the audit alongside and stays up.

//------------------------------------------------------------------------------------------------
modded class RZ_GameModeComponent
{
	protected int m_iRZSSec;
	protected int m_iRZSPhase;
	protected int m_iRZSNext;
	protected bool m_bRZSOwnWorld;

	protected static const int RZS_LAND_WAIT = 1200;	// seconds to wait for the land grid at most
	protected static const int RZS_AUDIT_MS = 60;		// the audit's reading budget a second

	override void OnGameModeStart()
	{
		super.OnGameModeStart();
		string world = GetGame().GetWorldFile();
		m_bRZSOwnWorld = world.Contains("RZSites_");
		Print(string.Format("RZS start: world %1, %2 props to measure, own world %3", world, RZS_Props.List().Count(), m_bRZSOwnWorld));
		GetGame().GetCallqueue().CallLater(RZS_Tick, 1000, true);
	}

	void RZS_Tick()
	{
		m_iRZSSec++;
		RZ_CampLife camp = GetCampLife();
		if (!camp || m_iRZSPhase >= 4)
			return;

		// The land grid first (lairs and wild land are part of a site's surroundings)
		if (m_iRZSPhase == 0)
		{
			RZ_Land land = GetLand();
			bool ready = !land || !land.IsEnabled() || land.IsReady();
			if (!ready && m_iRZSSec < RZS_LAND_WAIT)
			{
				if (m_iRZSSec % 60 == 0)
					Print(string.Format("RZS t=%1 s: waiting for the land grid", m_iRZSSec));

				return;
			}

			Print(string.Format("RZS land ready (%1) after %2 s", ready, m_iRZSSec));
			m_iRZSPhase = 1;
			if (!m_bRZSOwnWorld)
				m_iRZSPhase = 2;

			return;
		}

		// Every prop's box, ten a second
		if (m_iRZSPhase == 1)
		{
			for (int k = 0; k < 10 && m_iRZSNext < RZS_Props.List().Count(); k++)
			{
				RZS_Measure(RZS_Props.List()[m_iRZSNext]);
				m_iRZSNext++;
			}

			if (m_iRZSNext >= RZS_Props.List().Count())
			{
				Print("RZS footprints done");
				m_iRZSPhase = 2;
			}

			return;
		}

		if (m_iRZSPhase == 2)
		{
			RZ_CampLife.s_iAuditReadMs = RZS_AUDIT_MS;
			RZ_SiteGrid.s_bNameBig = true;
			int queued = camp.AuditAll();
			Print(string.Format("RZS audit: %1 sites queued at %2 s", queued, m_iRZSSec));
			m_iRZSPhase = 3;
			return;
		}

		if (camp.AuditPending() > 0)
		{
			if (m_iRZSSec % 30 == 0)
				Print(string.Format("RZS t=%1 s: %2 sites still to read", m_iRZSSec, camp.AuditPending()));

			return;
		}

		m_iRZSPhase = 4;
		int under = camp.AuditSummary();
		Print(string.Format("RZS done: %1 strongholds under the threshold (%2), at %3 s", under, camp.GetSettings().m_iAuditThreshold, m_iRZSSec));
		if (m_bRZSOwnWorld)
			GetGame().GetCallqueue().CallLater(RZS_Close, 2000, false);
	}

	//------------------------------------------------------------------------------------------------
	//! One prop spawned far off, its box (and its children's) read with no turn, then deleted.
	protected void RZS_Measure(ResourceName prefab)
	{
		Resource res = Resource.Load(prefab);
		if (!res || !res.IsValid())
		{
			Print("RZS footprint missing " + prefab);
			return;
		}

		vector at = Vector(200, 0, 200);
		at[1] = GetGame().GetWorld().GetSurfaceY(at[0], at[2]) + 50;
		EntitySpawnParams sp = new EntitySpawnParams();
		sp.TransformMode = ETransformMode.WORLD;
		sp.Transform[3] = at;
		IEntity e = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), sp);
		if (!e)
		{
			Print("RZS footprint failed " + prefab);
			return;
		}

		vector lo = Vector(float.MAX, float.MAX, float.MAX);
		vector hi = Vector(-float.MAX, -float.MAX, -float.MAX);
		RZS_Bounds(e, 0, lo, hi);
		vector origin = e.GetOrigin();
		SCR_EntityHelper.DeleteEntityAndChildren(e);
		if (lo[0] > hi[0])
		{
			Print("RZS footprint empty " + prefab);
			return;
		}

		float w = hi[0] - lo[0];
		float d = hi[2] - lo[2];
		float h = hi[1] - lo[1];
		float ox = (lo[0] + hi[0]) * 0.5 - origin[0];
		float oz = (lo[2] + hi[2]) * 0.5 - origin[2];
		float base = lo[1] - origin[1];
		Print(string.Format("RZS footprint %1 %2 %3 %4 %5 %6 %7", FilePath.StripPath(prefab.GetPath()), w, d, h, ox, oz, base));
	}

	//------------------------------------------------------------------------------------------------
	protected void RZS_Bounds(IEntity e, int depth, inout vector lo, inout vector hi)
	{
		if (!e || depth > 3)
			return;

		// A lamp's light, a fire's flames and sound reach far: not the prop's shape
		if (LightEntity.Cast(e) || DecalEntity.Cast(e) || ParticleEffectEntity.Cast(e))
			return;

		vector mins;
		vector maxs;
		e.GetWorldBounds(mins, maxs);
		if (maxs[0] - mins[0] > 0.001 || maxs[2] - mins[2] > 0.001)
		{
			for (int i = 0; i < 3; i++)
			{
				lo[i] = Math.Min(lo[i], mins[i]);
				hi[i] = Math.Max(hi[i], maxs[i]);
			}
		}

		IEntity child = e.GetChildren();
		while (child)
		{
			RZS_Bounds(child, depth + 1, lo, hi);
			child = child.GetSibling();
		}
	}

	void RZS_Close()
	{
		GetGame().RequestClose();
	}
}
