//! Unarmed missions (docs/MISSIONS.md, "Unarmed missions"):
//! low-level contracts about stealth, noise and thinking for new characters and unarmed
//! backgrounds. They use the noisemakers (any loud noise: RZ_HeatMap.FindRecentNoise) and
//! the "being noticed" check (RZ_NoticeTracker), and pay small Fame. This file has the ones where
//! something is carried; RZ_UnarmedTasks.c has the rest.

//------------------------------------------------------------------------------------------------
//! Shared bits for the unarmed missions.
class RZ_UnarmedHelper
{
	static const float LURE_RADIUS = 120;	// a noise this close to the infected a mission steers turns them
	static const float LURE_AGE = 12;		// ...if it's this fresh (s)
	static const float LURE_HOLD = 90;		// they stay at the noise this long before drifting back

	//------------------------------------------------------------------------------------------------
	//! A place to hand something to the Haven near `from`: the nearest safehouse within 4 km,
	//! else the least infected town. False if neither.
	static bool HavenDrop(RZ_GameModeComponent gm, vector from, out vector pos, out string name, out int house)
	{
		house = -1;
		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses)
		{
			int idx = houses.FindNearest(from, 4000);
			RZ_Safehouse h = houses.Get(idx);
			if (h)
			{
				pos = RZ_Mission.LandPointNear(h.m_vPos, 2, 6);
				name = "the safehouse in " + h.m_sName;
				house = idx;
				return true;
			}
		}

		RZ_Settlement s = gm.GetSettlement(RZ_Mission.PickSafestSettlement(gm));
		if (!s)
			return false;

		pos = RZ_Mission.LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.3);
		name = s.m_sName;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A building of one of `kinds` in the town, else a point near its centre.
	static vector BuildingIn(RZ_Settlement s, array<string> kinds, out string what)
	{
		RZ_Landmark l = RZ_LandmarkFinder.FindNearest(s.m_vCenter, s.m_fRadius, kinds);
		if (l)
		{
			what = l.m_sName;
			return RZ_Util.RandomPointInRing(l.m_vPos, 2, 5);
		}

		what = "the house with the red door";
		return RZ_Mission.LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.3);
	}

	//------------------------------------------------------------------------------------------------
	//! Send every unit in `units` to target (drops the dead ones). Returns the waypoints made,
	//! for the mission to delete later.
	static void SteerUnits(RZ_GameModeComponent gm, array<IEntity> units, vector target, array<IEntity> waypoints)
	{
		ResourceName wp = gm.GetSpawner().GetMoveWaypointPrefab();
		for (int i = units.Count() - 1; i >= 0; i--)
		{
			IEntity unit = units[i];
			if (!unit)
			{
				units.Remove(i);
				continue;
			}

			IEntity made = RZ_Util.RedirectGroup(unit, target, wp);
			if (made && waypoints)
				waypoints.Insert(made);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Position of the first living unit, false if all are gone.
	static bool UnitsPosition(array<IEntity> units, out vector pos)
	{
		foreach (IEntity unit : units)
		{
			if (unit && RZ_Util.CountAlive(unit) > 0)
			{
				pos = RZ_Util.GetUnitPosition(unit);
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	static void DeleteAll(array<IEntity> list)
	{
		foreach (IEntity e : list)
		{
			RZ_Util.DeleteUnit(e);
		}

		list.Clear();
	}
}

//------------------------------------------------------------------------------------------------
//! #1 Pharmacy Lure (Haven): infected fill a pharmacy. Throw something loud away from it to
//! draw them out (they walk to the noise and stay there a while), sneak in, take the medicine
//! and bring it to the Haven. Taking it while noticed works, but they come for you and the
//! pay is cut. ★ Nurse/Doctor: more Fame (they know what to take).
class RZ_Mission_PharmacyLure : RZ_Mission_Delivery
{
	protected vector m_vPharmacy;
	protected ref array<IEntity> m_aGuards = {};
	protected ref array<IEntity> m_aWaypoints = {};
	protected ref RZ_NoticeTracker m_Notice = new RZ_NoticeTracker();
	protected float m_fLuredFor;
	protected bool m_bLured;
	protected bool m_bTaken;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 8, 12);
		m_Req.Bonus("nurse");
		m_Req.Bonus("doctor");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 2400;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!CanRun(gm, RZ_EFaction.SURVIVORS) || gm.GetSpawner().GetMoveWaypointPrefab().IsEmpty())
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 25, 85);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		string what;
		array<string> kinds = {"pharmacy"};
		m_vPharmacy = RZ_UnarmedHelper.BuildingIn(s, kinds, what);
		if (what != "the pharmacy")
			what = "the chemist's";

		vector dropPos;
		string dropName;
		int house;
		if (!RZ_UnarmedHelper.HavenDrop(gm, s.m_vCenter, dropPos, dropName, house))
			return false;

		if (!SpawnDrop(gm, RZ_EFaction.SURVIVORS, dropPos))
		{
			Cleanup(gm);
			return false;
		}

		// The medicine lies on the pharmacy's shelves, placed once someone comes near
		PlaceInsideLater(m_vPharmacy);

		for (int i = 0; i < 4; i++)
		{
			RZ_TrackedZombie z = gm.GetSpawner().SpawnZombie(LandPointNear(m_vPharmacy, 3, 12), false, m_iSettlement, m_iId);
			if (z)
				m_aGuards.Insert(z.m_Entity);
		}

		m_vPos = m_vPharmacy;
		m_Notice.m_fHeatThreshold = 45;
		m_Notice.m_fZombieRadius = 10;
		m_Notice.m_fZombieSeconds = 6;
		m_sTitle = "Pharmacy Lure";
		m_sBrief = string.Format("%1 in %2 still has antibiotics on the shelves, and a crowd of them standing in the aisles. Don't fight them. Throw something loud well away from the door, let them go and look, then walk in. Bring the medicine to %3.",
			what, s.m_sName, dropName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDeliveryTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_Notice.Track(gm, ids, players, dt, m_vPharmacy, 150);

		// A noise near the pharmacy pulls them out; after a while they drift back in
		vector noise;
		if (gm.GetHeat().FindRecentNoise(m_vPharmacy, RZ_UnarmedHelper.LURE_RADIUS, RZ_UnarmedHelper.LURE_AGE, noise) && RZ_Util.DistXZ(noise, m_vPharmacy) > 25)
		{
			if (!m_bLured)
				RZ_UnarmedHelper.SteerUnits(gm, m_aGuards, noise, m_aWaypoints);

			m_bLured = true;
			m_fLuredFor = 0;
		}
		else if (m_bLured)
		{
			m_fLuredFor += dt;
			if (m_fLuredFor >= RZ_UnarmedHelper.LURE_HOLD && !m_bTaken)
			{
				m_bLured = false;
				RZ_UnarmedHelper.SteerUnits(gm, m_aGuards, m_vPharmacy, m_aWaypoints);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnCarrierChanged(RZ_GameModeComponent gm, int previous, int carrier)
	{
		super.OnCarrierChanged(gm, previous, carrier);
		if (m_bTaken || carrier <= 0)
			return;

		m_bTaken = true;
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(carrier);
		if (player && m_Notice.IsNoticed(gm, player))
		{
			m_fRewardScale = 0.6;
			RZ_UnarmedHelper.SteerUnits(gm, m_aGuards, player.GetOrigin(), m_aWaypoints);
			gm.NotifyPlayer(carrier, m_sTitle, m_Notice.GetReason(gm, player) + " They turn from the shelves toward you. Run.");
			return;
		}

		gm.NotifyPlayer(carrier, m_sTitle, "You sweep the boxes into your bag. Nothing in the aisles turns its head.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		RZ_UnarmedHelper.DeleteAll(m_aWaypoints);
		m_aGuards.Clear();
	}
}

//------------------------------------------------------------------------------------------------
//! #2 Silent Courier (Haven, for a Courier wanderer): carry a letter through an infested town to
//! the far side. While it's in the town, heat where the carrier stands must stay under 35:
//! louder, and the people waiting for it don't open the door (it fails). Going round the town
//! still delivers it, for half. ★ Hunter: the limit is 45.
class RZ_Mission_SilentCourier : RZ_Mission_Delivery
{
	protected int m_iTown = -1;
	protected bool m_bThrough;

	protected static const float HEAT_LIMIT = 35;
	protected static const float HEAT_LIMIT_BONUS = 45;
	protected static const int MAX_ATTEMPTS = 8;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 8, 12);
		m_Req.Bonus("hunter");
		m_fTimeLeft = 2400;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!CanRun(gm, RZ_EFaction.SURVIVORS))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		// Start and destination on opposite sides of the town, so the short way is through it.
		// One try often lands the far side in the sea or at the line: have a few.
		RZ_Settlement town = null;
		RZ_Settlement start = null;
		int from = -1;
		vector dest;
		bool found = false;
		for (int attempt = 0; attempt < MAX_ATTEMPTS && !found; attempt++)
		{
			m_iTown = PickSettlement(gm, 40, 90);
			town = gm.GetSettlement(m_iTown);
			if (!town)
				return false;

			from = PickSettlementAway(gm, town.m_vCenter, 600, 1500, 0, 70);
			start = gm.GetSettlement(from);
			if (!start)
				continue;

			float back = RZ_Compass.Bearing(town.m_vCenter, start.m_vCenter);
			dest = LandPointNear(RZ_Compass.PointAt(town.m_vCenter, back + 180 + Math.RandomFloat(-35, 35), Math.RandomFloat(700, 1200)), 0, 60);
			if (!RZ_Util.IsWater(dest) && !RZ_Perimeter.IsNearLineAt(dest))
				found = true;
		}

		if (!found)
			return false;

		if (!SpawnItem(gm, LandPointNear(start.m_vCenter, 0, start.m_fRadius * 0.3)) || !SpawnDrop(gm, RZ_EFaction.SURVIVORS, dest))
		{
			Cleanup(gm);
			return false;
		}

		m_iSettlement = from;
		m_vPos = start.m_vCenter;
		string courier = "";
		if (gm.GetWanderers())
			courier = gm.GetWanderers().AnyName(RZ_Wanderers.RoleBit(RZ_EWandererRole.COURIER));

		string who = "The courier";
		if (!courier.IsEmpty())
			who = courier + " the courier";

		m_sTitle = "Silent Courier";
		m_sBrief = string.Format("%1 can't make this one. A letter waits in %2, for people hiding on the far side of %3 (grid %4). The way is straight through %3. Keep quiet in there: if they hear you coming, the people you're bringing it to won't open the door.",
			who, start.m_sName, town.m_sName, RZ_SettlementScanner.GridReference(dest));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDeliveryTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_iCarrier <= 0 || m_eState != RZ_EMissionState.ACTIVE)
			return;

		RZ_Settlement town = gm.GetSettlement(m_iTown);
		IEntity carrier = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iCarrier);
		if (!town || !carrier || RZ_Util.DistXZ(carrier.GetOrigin(), town.m_vCenter) > town.m_fRadius + 50)
			return;

		m_bThrough = true;
		float limit = HEAT_LIMIT;
		if (HasBonus(gm, m_iCarrier))
			limit = HEAT_LIMIT_BONUS;

		if (gm.GetHeat().GetHeat(carrier.GetOrigin()) < limit)
			return;

		gm.NotifyPlayer(m_iCarrier, m_sTitle, string.Format("Too loud. Half of %1 heard you, and so did the people waiting for that letter. They won't open the door now.", town.m_sName));
		m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelivered(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction)
	{
		if (m_bThrough)
			return;

		m_fRewardScale = 0.5;
		gm.NotifyPlayer(playerId, m_sTitle, "You went the long way round. It got here, and late. They pay what they think it's worth.");
	}
}

//------------------------------------------------------------------------------------------------
//! #11 Water Run (Haven): carry water from a well or water tower to a safehouse. The fast road
//! goes through an infested town: get through it unnoticed and the Haven pays a third more;
//! noticed there, and some of it spills (less). The slow road pays the normal rate. Delivered
//! water stretches the safehouse's stores (+2 food).
class RZ_Mission_WaterRun : RZ_Mission_Delivery
{
	protected int m_iHouse = -1;
	protected int m_iRisky = -1;
	protected bool m_bFast;
	protected bool m_bSpilled;
	protected ref RZ_NoticeTracker m_Notice = new RZ_NoticeTracker();

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 8, 12);
		m_fTimeLeft = 2700;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!CanRun(gm, RZ_EFaction.SURVIVORS) || !houses || houses.Count() == 0)
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		m_iHouse = houses.PickRandom();
		RZ_Safehouse h = houses.Get(m_iHouse);
		if (!h)
			return false;

		m_iSettlement = PickSettlementAway(gm, h.m_vPos, 800, 2500, 0, 70);
		RZ_Settlement source = gm.GetSettlement(m_iSettlement);
		if (!source)
			return false;

		string what;
		array<string> kinds = {"well", "watertower"};
		vector well = RZ_UnarmedHelper.BuildingIn(source, kinds, what);
		if (what != "the well" && what != "the water tower")
			what = "the pump by the square";

		// The fast road: an infested town close to the straight line between them
		vector mid = (well + h.m_vPos) * 0.5;
		float half = RZ_Util.DistXZ(well, h.m_vPos) * 0.5;
		float best = float.MAX;
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			if (i == m_iSettlement || s.m_fInfection < 40 || i == h.m_iSettlement)
				continue;

			float d = RZ_Util.DistXZ(s.m_vCenter, mid);
			if (d < half && d < best)
			{
				best = d;
				m_iRisky = i;
			}
		}

		if (!SpawnItem(gm, well) || !SpawnDrop(gm, RZ_EFaction.SURVIVORS, LandPointNear(h.m_vPos, 2, 6)))
		{
			Cleanup(gm);
			return false;
		}

		m_vPos = well;
		m_sTitle = "Water Run";
		string road = "Take whichever road you like.";
		RZ_Settlement risky = gm.GetSettlement(m_iRisky);
		if (risky)
			road = string.Format("The fast road goes through %1. Get through there without them noticing and we'll pay a third more; if they notice you, half of it ends up on the road. The slow road goes round.", risky.m_sName);

		m_sBrief = string.Format("The safehouse in %1 is down to the bottom of the barrel. There's clean water at %2 in %3. It's heavy and it sloshes. %4",
			h.m_sName, what, source.m_sName, road);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDeliveryTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_Settlement risky = gm.GetSettlement(m_iRisky);
		if (!risky)
			return;

		m_Notice.Track(gm, ids, players, dt, risky.m_vCenter, risky.m_fRadius + 50);
		if (m_iCarrier <= 0)
			return;

		IEntity carrier = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iCarrier);
		if (!carrier || RZ_Util.DistXZ(carrier.GetOrigin(), risky.m_vCenter) > risky.m_fRadius + 50)
			return;

		m_bFast = true;
		if (!m_bSpilled && m_Notice.IsNoticed(gm, carrier))
		{
			m_bSpilled = true;
			gm.NotifyPlayer(m_iCarrier, m_sTitle, m_Notice.GetReason(gm, carrier) + " You run, and the water slops out with every step.");
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelivered(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction)
	{
		if (m_bSpilled)
			m_fRewardScale = 0.7;
		else if (m_bFast)
			m_fRewardScale = 1.33;

		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses)
			houses.AddFood(m_iHouse, 2);
	}
}

//------------------------------------------------------------------------------------------------
//! #14 Medicine Before Dark (Haven, for a Doctor wanderer): a bitten survivor waits in one town.
//! The right medicine is in a pharmacy in another. Get it to them before nightfall (20:30 game
//! time), or they turn. Offered only when nightfall is 15-90 real minutes away. Saved, they join
//! the nearest safehouse. ★ Nurse/Doctor: more Fame.
class RZ_Mission_MedicineBeforeDark : RZ_Mission_Delivery
{
	protected IEntity m_Patient;
	protected vector m_vPatient;
	protected string m_sPatient;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 10, 12);
		m_Req.Bonus("nurse");
		m_Req.Bonus("doctor");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!CanRun(gm, RZ_EFaction.SURVIVORS) || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN) || !RZ_GameTime.IsCalibrated() || RZ_GameTime.IsNight())
			return 0;

		float secs = RZ_GameTime.RealSecondsUntil(RZ_GameTime.NIGHT_START);
		if (secs < 900 || secs > 5400)
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 60);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		int other = PickSettlementAway(gm, s.m_vCenter, 600, 2000, 0, 85);
		RZ_Settlement shop = gm.GetSettlement(other);
		if (!shop)
			return false;

		float secs = RZ_GameTime.RealSecondsUntil(RZ_GameTime.NIGHT_START);
		if (secs < 0)
			return false;

		string what;
		array<string> kinds = {"pharmacy"};
		vector medicine = RZ_UnarmedHelper.BuildingIn(shop, kinds, what);
		if (what != "the pharmacy")
			what = "the doctor's surgery";

		m_vPatient = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.4);
		m_Patient = RZ_Util.SpawnPrefab(gm.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN), m_vPatient, Math.RandomFloat(0, 360));
		if (!m_Patient)
			return false;

		RZ_PuzzleObjectComponent.Tag(m_Patient, m_iId, 0, "", "");
		RZ_ClueComponent.Tag(m_Patient, m_iId, 0, "Talk to the bitten survivor");
		if (!SpawnDrop(gm, RZ_EFaction.SURVIVORS, LandPointNear(m_vPatient, 1, 3)))
		{
			Cleanup(gm);
			return false;
		}

		// The medicine lies in the pharmacy, placed once someone comes near
		PlaceInsideLater(medicine);

		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		m_sPatient = RZ_NoteText.FirstName(rng);
		m_fTimeLeft = Math.Clamp(secs, 900, 5400);
		m_vPos = m_vPatient;

		string doctor = "";
		if (gm.GetWanderers())
			doctor = gm.GetWanderers().AnyName(RZ_Wanderers.RoleBit(RZ_EWandererRole.DOCTOR));

		string who = "The doctor on the road";
		if (!doctor.IsEmpty())
			who = doctor + " the doctor";

		m_sTitle = "Medicine Before Dark";
		m_sBrief = string.Format("%1 sends word: %2 was bitten near %3 and is holding on. There's something in %4 in %5 that buys time, if it reaches %2 before dark (%6). After dark it won't matter.",
			who, m_sPatient, s.m_sName, what, shop.m_sName, RZ_GameTime.Clock(RZ_GameTime.NIGHT_START));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelivered(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction)
	{
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 takes it with shaking hands. \"Before dark. You actually did it.\"", m_sPatient));
		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses && houses.Count() > 0)
			houses.AddRescued(m_vPatient, 1, m_aParticipants);
	}

	//------------------------------------------------------------------------------------------------
	//! "Talk to the bitten survivor" (RZ_ReadClueAction routes here).
	override bool HandleTalk(RZ_GameModeComponent gm, int playerId, int clueIdx)
	{
		string left = "";
		if (RZ_GameTime.IsCalibrated())
			left = string.Format(" It's %1. Dark comes at %2.", RZ_GameTime.Clock(RZ_GameTime.GetHour()), RZ_GameTime.Clock(RZ_GameTime.NIGHT_START));

		gm.NotifyPlayer(playerId, m_sPatient, string.Format("Pale, sweating, a dressing on the forearm gone dark. \"Did they send you? Before dark. Please.\"%1", left));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailed(RZ_GameModeComponent gm)
	{
		// Dark came first: whoever is near meets what's left of them
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		if (CountPlayersNear(players, m_vPatient, 400) > 0)
			gm.GetSpawner().SpawnZombie(m_vPatient, true, m_iSettlement, -1);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return string.Format("%1: it got dark first. %2 didn't make it, not as themselves.", m_sTitle, m_sPatient);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		RZ_Util.DeleteUnit(m_Patient);
	}
}
