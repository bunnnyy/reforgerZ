//! Unarmed missions, part three: Car Keys, a Mechanic wanderer's job (docs/MISSIONS.md,
//! "Unarmed missions"). Built on the fixable cars (Interactions/RZ_Cars.c: a mission car that
//! already runs and needs only its keys), loose loot (the keys lie in the right house: RZ_Loot)
//! and the vehicle zone block.

//------------------------------------------------------------------------------------------------
//! Car Keys (Haven, from a Mechanic wanderer): a car the Mechanic fixed stands in a town, but its
//! keys lie in one of the five houses nearest it, on a table or the floor like any loose loot. Go
//! through them quietly (picking the keys up is finding them; heat 45 or more at that house when
//! you do means you were noticed: a horde comes and the pay is ×0.7), start it and drive it back
//! to where the Mechanic waits.
//! Hotwiring works too, but it's loud and pays half. The Mechanic keeps the car: it leaves the
//! world once nobody sits in it. Started by "Ask about cars", one at a time, one per player per
//! 30 minutes.
class RZ_Mission_CarKeys : RZ_Mission
{
	protected vector m_vMechanic;
	protected string m_sMechanic;
	protected bool m_bPrepared;
	protected int m_iFor = -1;
	protected int m_iCar = -1;
	protected vector m_vCar;
	protected vector m_vKeyHouse;
	protected ref array<vector> m_aHouses = {};
	protected ref RZ_VehicleZone m_Zone = new RZ_VehicleZone();
	protected bool m_bKeysSeen;
	protected bool m_bNoticed;
	protected bool m_bArrived;
	protected bool m_bStarted;

	protected static const int HOUSES = 5;
	protected static const float NOTICE_HEAT = 45;
	protected static const float ZONE_RADIUS = 30;

	//------------------------------------------------------------------------------------------------
	//! The five houses are listed by grid: never a rough area.
	override bool NeedsExactLocation()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool NeedsVehicle()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 10, 12);
		m_fTimeLeft = 2700;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Where the Mechanic waits and who they are. False if fixable cars are off.
	bool Prepare(RZ_GameModeComponent gm, vector at, string mechanic, int playerId)
	{
		RZ_Interactions hub = gm.GetInteractions();
		if (!hub || !hub.IsActive() || !hub.GetCars().IsOn())
			return false;

		m_vMechanic = at;
		m_sMechanic = mechanic;
		m_iFor = playerId;
		m_bPrepared = true;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The job belongs to whoever asked (and whoever drives in with them).
	override bool IsEligible(RZ_GameModeComponent gm, int playerId)
	{
		// Someone else is paid only alongside the one who asked: taking the job off them (or
		// finishing it after they left) pays nobody
		if (m_iFor > 0 && playerId != m_iFor && (!m_aParticipants.Contains(playerId) || !m_aParticipants.Contains(m_iFor)))
			return false;

		return super.IsEligible(gm, playerId);
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		if (!m_bPrepared)
			return false;

		RZ_Interactions hub = gm.GetInteractions();
		array<int> towns = {};
		foreach (int i, RZ_Settlement t : gm.GetSettlements())
		{
			float d = RZ_Util.DistXZ(t.m_vCenter, m_vMechanic);
			if (d < 500 || d > 2500 || t.m_fInfection < 10 || t.m_fInfection > 80)
				continue;

			if (!gm.GetMissions().IsSettlementBusy(i) && !RZ_EventHelp.IsLivedIn(gm, i))
				towns.Insert(i);
		}

		for (int tries = 0; tries < 4 && !towns.IsEmpty(); tries++)
		{
			int pick = Math.RandomInt(0, towns.Count());
			int idx = towns[pick];
			towns.Remove(pick);
			RZ_Settlement s = gm.GetSettlement(idx);
			vector car = LandPointNear(s.m_vCenter, 10, Math.Max(25, s.m_fRadius * 0.5));
			if (!PickHouses(hub, s, car))
				continue;

			m_vKeyHouse = m_aHouses.GetRandomElement();
			m_iCar = hub.GetCars().AddMissionCar(idx, car, m_vKeyHouse, m_iId);
			if (m_iCar < 0)
				return false;

			m_iSettlement = idx;
			m_vCar = car;
			m_vPos = car;
			m_Zone.SetZone(m_vMechanic, ZONE_RADIUS);
			m_sTitle = "Car Keys";
			m_sBrief = string.Format("%1: \"There's %2 in %3 that runs. I fixed it myself, then the owner locked it and dropped the keys in one of the five houses nearest it. Go quietly, the houses aren't empty. Bring it back to me here. Hotwire it if you must, but the whole town will hear, and I'll pay you less.\"",
				m_sMechanic, hub.GetCars().GetNoun(m_iCar), s.m_sName);
			m_sObjective = string.Format("Bring %1 from %2 back to %3.", hub.GetCars().GetNoun(m_iCar), s.m_sName, m_sMechanic);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The five unlocked houses nearest the car (never a locked one: the keys must be reachable).
	protected bool PickHouses(RZ_Interactions hub, RZ_Settlement s, vector car)
	{
		m_aHouses.Clear();
		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(car, s.m_fRadius + 100, buildings, 80);
		array<vector> options = {};
		foreach (IEntity b : buildings)
		{
			vector pos = b.GetOrigin();
			string type = hub.GetSearch().BuildingType(RZ_ItemHelper.LowerName(b));
			if (type != "house" && type != "farm")
				continue;

			// Never a house with its own lock (the keys would sit behind its door)
			if (hub.GetSearch().HasOwnLock(pos, type) || hub.IsProtected(pos) || hub.GetTraps().IsBurned(pos))
				continue;

			options.Insert(pos);
		}

		for (int n = 0; n < HOUSES && !options.IsEmpty(); n++)
		{
			int best = 0;
			float bestDist = float.MAX;
			foreach (int j, vector o : options)
			{
				float d = RZ_Util.DistXZ(o, car);
				if (d < bestDist)
				{
					bestDist = d;
					best = j;
				}
			}

			m_aHouses.Insert(options[best]);
			options.Remove(best);
		}

		return m_aHouses.Count() >= HOUSES;
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string houses = "";
		foreach (int i, vector h : m_aHouses)
		{
			if (i > 0)
				houses += ", ";

			houses += RZ_SettlementScanner.GridReference(h);
		}

		return super.GetWhereText(gm, from) + string.Format("\nThe five houses (grid): %1\nBack to %2 at grid %3", houses, m_sMechanic, RZ_SettlementScanner.GridReference(m_vMechanic));
	}

	//------------------------------------------------------------------------------------------------
	//! The car (the five houses are round it) until it runs, then back to the mechanic.
	override RZ_ObjPlace ObjectivePlace(RZ_GameModeComponent gm, int playerId, vector from)
	{
		if (m_bStarted)
		{
			RZ_ObjPlace at = m_Zone.ObjPlace(playerId);
			if (at)
				return at;
		}

		return super.ObjectivePlace(gm, playerId, from);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_Interactions hub = gm.GetInteractions();
		if (!hub)
			return;

		RZ_Cars cars = hub.GetCars();

		// The keys turned up: was anyone listening?
		if (!m_bKeysSeen && cars.AreKeysFound(m_iCar))
		{
			m_bKeysSeen = true;
			RZ_ContractHelp.AddNear(ids, players, m_vKeyHouse, 25, m_aParticipants);
			if (gm.GetHeat().GetHeat(m_vKeyHouse) >= NOTICE_HEAT)
			{
				m_bNoticed = true;
				gm.GetSpawner().SpawnHorde(m_vKeyHouse, 200, m_iId);
				RZ_ContractHelp.NotifyNear(gm, m_vKeyHouse, 100, m_sTitle, "Keys, at last. But you've made enough noise in this street that something heard. They're coming.");
			}
		}

		if (!m_bStarted)
		{
			IEntity vehicle = cars.GetStartedCar(m_iCar);
			if (!vehicle)
				return;

			m_bStarted = true;
			m_Zone.SetVehicle(vehicle);
			RZ_ContractHelp.AddNear(ids, players, vehicle.GetOrigin(), 10, m_aParticipants);
			return;
		}

		if (!m_bArrived)
		{
			if (m_Zone.IsLost())
			{
				m_eState = RZ_EMissionState.FAILED;
				return;
			}

			if (!m_Zone.Check(ids, players))
				return;

			m_bArrived = true;
			m_Zone.GetCrew(ids, players, 40, m_aParticipants);
			RZ_ContractHelp.NotifyNear(gm, m_vMechanic, 80, m_sTitle, string.Format("%1 walks round it twice, pats the bonnet and grins. \"Leave it running. I'll take it from here.\"", m_sMechanic));
			return;
		}

		// Paid once nobody sits in it any more
		if (RZ_VehicleZone.DriverOf(m_Zone.m_Arrived, ids, players) < 0)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Interactions hub = gm.GetInteractions();
		if (hub && hub.GetCars().WasHotwired(m_iCar))
			m_fRewardScale = m_fRewardScale * 0.5;

		if (m_bNoticed)
			m_fRewardScale = m_fRewardScale * 0.7;
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		// Delivered: the Mechanic keeps it (it leaves the world). Not delivered: an unstarted car
		// goes; a started one stays where it is, an ordinary car now.
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		IEntity kept = m_Zone.m_Arrived;
		if (kept && RZ_VehicleZone.DriverOf(kept, ids, players) < 0)
			RZ_Util.DeleteUnit(kept);

		RZ_Interactions hub = gm.GetInteractions();
		if (hub && m_iCar >= 0)
			hub.GetCars().RemoveMissionCar(m_iCar);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (m_bStarted)
			return string.Format("%1 never got the car back. It was a good car.", m_sMechanic);

		return string.Format("%1 has stopped waiting for the car.", m_sMechanic);
	}
}
