//! Vehicle delivery block (docs/DESIGN.md, "Mission building blocks"): a specific vehicle, or any
//! vehicle of a type, driven into a zone. Works with the vehicles a mission spawns, fixed cars
//! (they become ordinary vehicles from Cars > Car prefabs) and supply trucks
//! (RZ_SupplyLines.GetActiveTruck).
//!
//! - **A specific vehicle** (SetVehicle): it arrives when it stands in the zone with a player at
//!   the wheel (or, with m_bNeedDriver off, on its own). Lost when it's destroyed or deleted.
//! - **A type** (AddWords / AddPrefabs): any vehicle a player drives into the zone whose prefab
//!   name has one of the words, or is one of the prefabs.
//!
//! Who drove it in is read from the player's seat (RZ_Util.GetVehicleOf). A mission ticks it
//! with Check() and reads m_Arrived and m_iDriver.
class RZ_VehicleZone
{
	vector m_vPos;
	float m_fRadius = 30;
	bool m_bNeedDriver = true;

	IEntity m_Arrived;			// the vehicle that came in
	int m_iDriver = -1;			// who drove it, -1 if nobody (m_bNeedDriver off)

	protected IEntity m_Target;
	protected bool m_bHasTarget;
	protected ref array<string> m_aWords = {};
	protected ref array<string> m_aPrefabs = {};

	//------------------------------------------------------------------------------------------------
	void SetZone(vector pos, float radius)
	{
		m_vPos = pos;
		m_fRadius = radius;
	}

	//------------------------------------------------------------------------------------------------
	void SetVehicle(IEntity vehicle)
	{
		m_Target = vehicle;
		m_bHasTarget = vehicle != null;
	}

	//------------------------------------------------------------------------------------------------
	IEntity GetVehicle()
	{
		return m_Target;
	}

	//------------------------------------------------------------------------------------------------
	//! "bus,truck,van": any vehicle whose prefab name contains one of these counts.
	void AddWords(string wordList)
	{
		array<string> words = {};
		RZ_ItemHelper.Words(wordList, words);
		foreach (string w : words)
		{
			m_aWords.Insert(w);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Exact prefabs that count (Cars > Car prefabs: what a fixed car becomes).
	void AddPrefabs(array<ResourceName> prefabs)
	{
		if (!prefabs)
			return;

		foreach (ResourceName r : prefabs)
		{
			string name = r;
			name.ToLower();
			if (!name.IsEmpty())
				m_aPrefabs.Insert(name);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The specific vehicle is gone or burnt out. Always false without one.
	bool IsLost()
	{
		if (!m_bHasTarget)
			return false;

		return !m_Target || RZ_Util.IsDestroyed(m_Target);
	}

	//------------------------------------------------------------------------------------------------
	//! Does this vehicle count?
	bool Matches(IEntity vehicle)
	{
		if (!vehicle)
			return false;

		if (m_bHasTarget)
			return vehicle == m_Target;

		string name = RZ_Util.GetPrefabName(vehicle);
		name.ToLower();
		if (name.IsEmpty())
			return false;

		if (m_aPrefabs.Contains(name))
			return true;

		return RZ_ItemHelper.NameHas(name, m_aWords);
	}

	//------------------------------------------------------------------------------------------------
	//! True once a matching vehicle is in the zone (m_Arrived, m_iDriver are set).
	bool Check(array<int> ids, array<IEntity> players)
	{
		if (m_Arrived)
			return true;

		// The specific vehicle standing in the zone on its own
		if (m_bHasTarget && !m_bNeedDriver)
		{
			if (!IsLost() && RZ_Util.DistXZ(m_Target.GetOrigin(), m_vPos) <= m_fRadius)
			{
				m_Arrived = m_Target;
				m_iDriver = DriverOf(m_Target, ids, players);
				return true;
			}

			return false;
		}

		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), m_vPos) > m_fRadius)
				continue;

			IEntity vehicle = RZ_Util.GetVehicleOf(p);
			if (!vehicle && m_bHasTarget && m_Target && RZ_Util.IsInVehicle(p) && RZ_Util.DistXZ(p.GetOrigin(), m_Target.GetOrigin()) <= 6)
				vehicle = m_Target;		// the seat lookup failed: a player in a vehicle right where ours is

			if (!Matches(vehicle) || RZ_Util.IsDestroyed(vehicle))
				continue;

			m_Arrived = vehicle;
			m_iDriver = ids[i];
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A player sitting in this vehicle, -1 if none.
	static int DriverOf(IEntity vehicle, array<int> ids, array<IEntity> players)
	{
		if (!vehicle)
			return -1;

		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), vehicle.GetOrigin()) > 8 || !RZ_Util.IsInVehicle(p))
				continue;

			IEntity seat = RZ_Util.GetVehicleOf(p);
			if (!seat || seat == vehicle)
				return ids[i];
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Players riding in or standing by the vehicle that came in, added once each to `result`.
	void GetCrew(array<int> ids, array<IEntity> players, float radius, notnull array<int> result)
	{
		vector at = m_vPos;
		if (m_Arrived)
			at = m_Arrived.GetOrigin();

		RZ_ContractHelp.AddNear(ids, players, at, radius, result);
	}
}

//------------------------------------------------------------------------------------------------
//! Convoys for missions (Convoy Escort, Hijack the Convoy): asks RZ_SupplyLines for the faction's
//! next truck to go to one of its outposts soon, finds that truck when it leaves, and keeps the
//! result (RZ_SupplyLines.TRUCK_*). The truck is the supply system's own: it drives, delivers,
//! is robbed or stolen exactly as any other (docs/DESIGN.md, "Supply trucks").
class RZ_ConvoyWatch
{
	int m_iFaction = -1;
	int m_iHolding = -1;
	int m_iResult = -1;				// -1 still on its way
	int m_iDriver = -1;				// who drove it in or away
	vector m_vResultPos;
	string m_sFrom;
	string m_sTo;
	protected bool m_bFound;

	//------------------------------------------------------------------------------------------------
	//! Trucks run for the faction, none is on the road or asked for, and it has an outpost to go to.
	static bool CanRun(RZ_GameModeComponent gm, int f)
	{
		RZ_SupplyLines supply = gm.GetSupply();
		if (!supply || !supply.IsFactionRunning(f) || supply.GetActiveTruck(f) || supply.HasConvoyRequest(f))
			return false;

		return supply.PickConvoyOutpost(f) >= 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The faction's next truck goes to one of its outposts within delaySeconds.
	bool Request(RZ_GameModeComponent gm, int f, float delaySeconds)
	{
		RZ_SupplyLines supply = gm.GetSupply();
		if (!supply)
			return false;

		int holding = supply.PickConvoyOutpost(f);
		if (holding < 0 || !supply.RequestConvoy(f, holding, delaySeconds))
			return false;

		m_iFaction = f;
		m_iHolding = holding;
		RZ_Holding h = gm.GetTerritory().GetHolding(holding);
		RZ_Holding top = gm.GetTerritory().GetStronghold(f);
		if (h)
			m_sTo = h.m_sName;
		if (top)
			m_sFrom = top.m_sName;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The truck on the road, once it has left (null before and after). Looked up each time:
	//! the supply system owns it and may drop it at any tick.
	RZ_SupplyTruck GetTruck(RZ_GameModeComponent gm)
	{
		if (!m_bFound || m_iResult >= 0 || !gm.GetSupply())
			return null;

		RZ_SupplyTruck t = gm.GetSupply().GetActiveTruck(m_iFaction);
		if (!IsOurs(t) || t.m_bDone)
			return null;

		return t;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsOurs(RZ_SupplyTruck t)
	{
		return t && t.m_bConvoy && t.m_iDestKind == RZ_SupplyLines.KIND_OUTPOST && t.m_iDestIndex == m_iHolding;
	}

	//------------------------------------------------------------------------------------------------
	bool HasLeft()
	{
		return m_bFound;
	}

	//------------------------------------------------------------------------------------------------
	void Update(RZ_GameModeComponent gm)
	{
		if (m_bFound || !gm.GetSupply())
			return;

		if (IsOurs(gm.GetSupply().GetActiveTruck(m_iFaction)))
			m_bFound = true;
	}

	//------------------------------------------------------------------------------------------------
	//! From the mission's OnTruckEvent.
	void OnTruckEvent(int faction, int what, vector pos, int driver)
	{
		if (!m_bFound || faction != m_iFaction || m_iResult >= 0)
			return;

		m_iResult = what;
		m_vResultPos = pos;
		m_iDriver = driver;
	}

	//------------------------------------------------------------------------------------------------
	//! Where the truck is now (or where it ended up).
	vector GetPos(RZ_GameModeComponent gm)
	{
		RZ_SupplyTruck t = GetTruck(gm);
		if (t && t.m_Vehicle)
			return t.m_Vehicle.GetOrigin();

		if (m_iResult >= 0)
			return m_vResultPos;

		RZ_Holding top = gm.GetTerritory().GetStronghold(m_iFaction);
		if (top)
			return top.m_vPos;

		return vector.Zero;
	}
}
