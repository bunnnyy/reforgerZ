//! What the power station looks and sounds like (docs/DESIGN.md, "The power grid"), beyond the
//! map's own buildings. Server only, and only while a player is near, like the control itself:
//!
//! - the dressing (World > Power grid > Dressing prefab): fuel drums, a jerrycan and a warning sign
//!   beside the control, at every step, so the place reads as a station before anyone touches it;
//! - the generator's sound (World > Power grid > Generator sound prefab) from the moment it catches
//!   (spinning up, ready, running): at the map's own generator sets when the station has them (a
//!   power plant's two), else at the control. The prefab plays by itself on every client that has it.
//!
//! Both are look-only: they go through the dressing guard and are marked as scenery. Empty slot =
//! that part is off. Where they stand is worked out once, at setup (World/RZ_PowerSite.c).
class RZ_PowerLooks
{
	protected ResourceName m_sDressing;
	protected ResourceName m_sSound;
	protected bool m_bDress;
	protected vector m_vDressPos;
	protected float m_fDressYaw;
	protected string m_sDressWhy;			// no room for it: what was in the way of each spot tried
	protected ref array<vector> m_aSoundPos = {};
	protected ref array<float> m_aSoundYaw = {};
	protected IEntity m_Dressing;
	protected bool m_bDressTried;			// spawned (or refused) for this visit
	protected ref array<IEntity> m_aSounds = {};
	protected bool m_bSoundsUp;

	//------------------------------------------------------------------------------------------------
	void RZ_PowerLooks(RZ_PowerSettings settings)
	{
		m_sDressing = settings.m_sDressingPrefab;
		m_sSound = settings.m_sGeneratorSoundPrefab;
	}

	//------------------------------------------------------------------------------------------------
	//! Where the dressing stands, front along compass bearing `yaw` (RZ_PowerSite.Place).
	void SetDressing(vector pos, float yaw)
	{
		m_bDress = true;
		m_vDressPos = pos;
		m_fDressYaw = yaw;
	}

	//------------------------------------------------------------------------------------------------
	//! No spot for the dressing: what was in the way of each one tried, for the log.
	void SetNoDressing(string why)
	{
		m_sDressWhy = why;
	}

	//------------------------------------------------------------------------------------------------
	//! One place the generator is heard from (a generator set of the map's, or the control).
	void AddSound(vector pos, float yaw)
	{
		m_aSoundPos.Insert(pos);
		m_aSoundYaw.Insert(yaw);
	}

	//------------------------------------------------------------------------------------------------
	//! For the log: "dressing beside it, the generator heard from 2 places".
	string Describe()
	{
		string dress;
		if (m_sDressing.IsEmpty())
			dress = "no dressing prefab";
		else if (m_bDress)
			dress = "dressing beside it";
		else
			dress = string.Format("no room for the dressing (in the way: %1)", m_sDressWhy);

		if (m_sSound.IsEmpty())
			return dress + ", no generator sound prefab";

		return string.Format("%1, the generator heard from %2 place(s)", dress, m_aSoundPos.Count());
	}

	//------------------------------------------------------------------------------------------------
	//! `near`: a player is within the control's reach; `gone`: nobody is within its despawn distance;
	//! `running`: the generator is turning (spinning up, ready or powering the towns).
	void Sync(bool near, bool gone, bool running)
	{
		if (gone)
		{
			Clear();
			return;
		}

		if (near && !m_bDressTried && m_bDress && !m_sDressing.IsEmpty())
		{
			m_bDressTried = true;
			m_Dressing = SpawnLook(m_sDressing, m_vDressPos, m_fDressYaw);
		}

		if (!running)
		{
			ClearSounds();
			return;
		}

		if (!near || m_bSoundsUp || m_sSound.IsEmpty())
			return;

		// VERIFY: a prefab with no mesh, spawned by the server, plays its StaticSoundComponent's events on
		// every client that streams it in, and stops when it's deleted (as vanilla's placed generators do)
		m_bSoundsUp = true;
		foreach (int i, vector pos : m_aSoundPos)
		{
			IEntity e = SpawnLook(m_sSound, pos, m_aSoundYaw[i]);
			if (e)
				m_aSounds.Insert(e);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Everything down (nobody near, or the grid is going away).
	void Clear()
	{
		Delete(m_Dressing);
		m_Dressing = null;
		m_bDressTried = false;
		ClearSounds();
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearSounds()
	{
		foreach (IEntity e : m_aSounds)
		{
			Delete(e);
		}

		m_aSounds.Clear();
		m_bSoundsUp = false;
	}

	//------------------------------------------------------------------------------------------------
	//! A look-only prop, exactly where it's put: through the dressing guard and marked as scenery.
	//! Null if the guard took it.
	protected static IEntity SpawnLook(ResourceName prefab, vector pos, float yaw)
	{
		IEntity e = RZ_Util.SpawnPrefab(prefab, pos, yaw);
		if (!e)
			return null;

		if (!RZ_DressingGuard.Strip(e, prefab))
			return null;

		RZ_CampLife.AddDressing(e);
		return e;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Delete(IEntity e)
	{
		if (!e)
			return;

		RZ_CampLife.RemoveDressing(e);
		SCR_EntityHelper.DeleteEntityAndChildren(e);
	}
}
