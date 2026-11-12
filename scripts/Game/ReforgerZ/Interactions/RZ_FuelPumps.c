//! Petrol stations' pumps as vanilla fuel (docs/DESIGN.md, "Fuel"). With **Cars > Fuel pump
//! station** set, a petrol station's pump has no prompt of ours: while a player is near, an
//! invisible vanilla fuel station stands at it, and vanilla's own jerrycan fills there (its Refuel
//! canister action), as at any vanilla fuel point. A filled can fuels a car the vanilla way. The
//! station's litres are ours: kept as data per pump when nobody is near, drawn down by what's
//! taken, and back to full over **Pump cooldown minutes**. Every litre drawn makes heat, as the
//! hand pump's squeal did. Without the slot the pump is the old hand pump (RZ_Cars.HandlePump), in
//! the object menu.

//------------------------------------------------------------------------------------------------
class RZ_FuelPump
{
	string m_sKey;
	vector m_vPos;
	float m_fLitres = -1;		// -1: never put out (full)
	float m_fAt;				// hub clock when m_fLitres was last read
	float m_fLastRead;			// litres in the station at the last tick
	IEntity m_Station;
}

//------------------------------------------------------------------------------------------------
class RZ_FuelPumps
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_Interactions m_Hub;
	protected RZ_CarSettings m_Settings;
	protected ref map<string, ref RZ_FuelPump> m_mPumps = new map<string, ref RZ_FuelPump>();

	//------------------------------------------------------------------------------------------------
	void RZ_FuelPumps(RZ_GameModeComponent gm, RZ_Interactions hub, RZ_CarSettings settings)
	{
		m_GM = gm;
		m_Hub = hub;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	//! Pumps are vanilla fuel points: the slot is set.
	bool IsOn()
	{
		return m_Settings.m_bEnabled && !m_Settings.m_sPumpStationPrefab.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! A pump's spot comes out near a player: the vanilla station stands at the pump with the litres
	//! it has now.
	IEntity PutOut(RZ_Spot s)
	{
		RZ_FuelPump p = Record(s);
		float litres = LitresNow(p);
		p.m_Station = RZ_Util.SpawnPrefab(m_Settings.m_sPumpStationPrefab, s.m_vPos, 0);
		if (!p.m_Station)
			return null;

		SetLitres(p.m_Station, litres);
		p.m_fLastRead = litres;
		p.m_fLitres = litres;
		p.m_fAt = m_Hub.GetClock();
		return p.m_Station;
	}

	//------------------------------------------------------------------------------------------------
	//! Everyone left: what's left goes back to data.
	void PutAway(RZ_Spot s)
	{
		RZ_FuelPump p;
		if (!m_mPumps.Find(s.m_sKey, p))
			return;

		if (p.m_Station)
		{
			p.m_fLitres = Litres(p.m_Station);
			p.m_fAt = m_Hub.GetClock();
		}

		p.m_Station = null;
	}

	//------------------------------------------------------------------------------------------------
	//! On the 5 s tick: litres drawn since the last look make heat (the pump is loud), and a station
	//! out a long time slowly fills back up.
	void Tick()
	{
		if (!IsOn())
			return;

		foreach (string key, RZ_FuelPump p : m_mPumps)
		{
			if (!p.m_Station)
				continue;

			float now = Litres(p.m_Station);
			float drawn = p.m_fLastRead - now;
			if (drawn > 0.5)
			{
				m_GM.GetHeat().AddHeat(p.m_vPos, m_Settings.m_fPumpHeat * Math.Min(2, drawn / 20));
				RZ_Log.Info(string.Format("Fuel pump: %1 litres drawn at %2 (%3 left).", RZ_Util.RoundToInt(drawn), RZ_SettlementScanner.GridReference(p.m_vPos), RZ_Util.RoundToInt(now)));
			}

			p.m_fLitres = now;
			p.m_fAt = m_Hub.GetClock();
			float back = Math.Min(Full(), LitresNow(p));
			if (back > now + 0.5)
			{
				SetLitres(p.m_Station, back);
				now = back;
				p.m_fLitres = back;
			}

			p.m_fLastRead = now;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_FuelPump Record(RZ_Spot s)
	{
		RZ_FuelPump p;
		if (m_mPumps.Find(s.m_sKey, p))
			return p;

		p = new RZ_FuelPump();
		p.m_sKey = s.m_sKey;
		p.m_vPos = s.m_vPos;
		m_mPumps.Insert(s.m_sKey, p);
		return p;
	}

	//------------------------------------------------------------------------------------------------
	protected float Full()
	{
		return Math.Max(1, m_Settings.m_fPumpLitres);
	}

	//------------------------------------------------------------------------------------------------
	//! The litres a pump has now: what it had, plus what came back since (full over the cooldown).
	protected float LitresNow(RZ_FuelPump p)
	{
		if (p.m_fLitres < 0)
			return Full();

		float minutes = Math.Max(1, m_Settings.m_fPumpCooldownMinutes);
		float back = (m_Hub.GetClock() - p.m_fAt) / (minutes * 60) * Full();
		return Math.Min(Full(), p.m_fLitres + back);
	}

	//------------------------------------------------------------------------------------------------
	//! The station's litres (every fuel node on it).
	static float Litres(IEntity station)
	{
		FuelManagerComponent fuel = FuelManagerComponent.Cast(station.FindComponent(FuelManagerComponent));
		if (!fuel)
			return 0;

		return fuel.GetTotalFuel();
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the station's litres on the server (vanilla replicates its fuel).
	protected void SetLitres(IEntity station, float litres)
	{
		SCR_FuelManagerComponent fuel = SCR_FuelManagerComponent.Cast(station.FindComponent(SCR_FuelManagerComponent));
		if (!fuel)
		{
			RZ_Log.Warn("Fuel pump station: no SCR_FuelManagerComponent on " + m_Settings.m_sPumpStationPrefab + " (Cars > Fuel pump station)");
			return;
		}

		float max = fuel.GetTotalMaxFuel();
		if (max <= 0)
			return;

		fuel.SetTotalFuelPercentage(Math.Clamp(litres / max, 0, 1)); // VERIFY: SetTotalFuelPercentage on the server replicates the station's fuel, so vanilla's Refuel canister draws from it on every client and stops when it's empty
	}

	// ================================================================ Cans
	//------------------------------------------------------------------------------------------------
	//! A carried fuel can that can give `litres`: a vanilla jerrycan (a fuel manager) holding that
	//! much, or an old-style full can (no fuel manager: a full can by its name). Null if none.
	static IEntity FindFuel(IEntity character, string words, float litres)
	{
		if (!character)
			return null;

		array<string> list = {};
		RZ_ItemHelper.Words(words, list);
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv || list.IsEmpty())
			return null;

		array<IEntity> items = {};
		inv.GetItems(items);
		foreach (IEntity item : items)
		{
			if (!RZ_ItemHelper.Matches(item, list))
				continue;

			FuelManagerComponent fuel = FuelManagerComponent.Cast(item.FindComponent(FuelManagerComponent));
			if (!fuel || fuel.GetTotalFuel() >= litres)
				return item;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Uses `litres` from a can FindFuel gave: a vanilla jerrycan is drained by that much and kept; an
	//! old-style can is used up. Server only.
	static bool UseFuel(IEntity character, IEntity can, float litres)
	{
		if (!can)
			return false;

		SCR_FuelManagerComponent fuel = SCR_FuelManagerComponent.Cast(can.FindComponent(SCR_FuelManagerComponent));
		if (!fuel)
		{
			RZ_ItemHelper.DeleteCarried(character, can);
			return true;
		}

		float max = fuel.GetTotalMaxFuel();
		float have = fuel.GetTotalFuel();
		if (max <= 0 || have < litres)
			return false;

		fuel.SetTotalFuelPercentage((have - litres) / max);
		return true;
	}
}
