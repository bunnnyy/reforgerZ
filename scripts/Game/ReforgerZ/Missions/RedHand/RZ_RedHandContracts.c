//! The Red Hand's later contracts (docs/MISSIONS.md, "The Red Hand": #3, 9, 10, 11, 13, 17, 18,
//! 19, 20), built on the mission building blocks (Missions/Blocks/).

//------------------------------------------------------------------------------------------------
//! #3 Car Theft: a particular car is parked in a town. Drive it to the chop shop (a Red Hand drop
//! point, RZ_VehicleZone). Leave it there: they strip it, and it leaves the world.
//! *Simplified:* spawned cars aren't locked, so there's no hotwiring; ★ Mechanic: +25% Fame.
class RZ_Mission_CarTheft : RZ_Mission
{
	protected ref RZ_VehicleZone m_Zone = new RZ_VehicleZone();
	protected IEntity m_Car;
	protected bool m_bSpawned;
	protected bool m_bArrived;
	protected vector m_vCar;
	protected string m_sShop;
	protected ResourceName m_sPrefab;

	//------------------------------------------------------------------------------------------------
	override bool NeedsVehicle()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.STRANGER, 18, 25);
		m_Req.Bonus("mechanic");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 2400;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		RZ_CarSettings cars = gm.GetCarSettings();
		if (!cars || !cars.m_aCarPrefabs || cars.m_aCarPrefabs.IsEmpty())
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 70);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		vector shop;
		if (!RZ_LeaderJob.MeetingPoint(gm, RZ_EFaction.BANDITS, s.m_vCenter, shop, m_sShop))
			return false;

		m_sPrefab = gm.GetCarSettings().m_aCarPrefabs.GetRandomElement();
		m_vCar = LandPointNear(s.m_vCenter, s.m_fRadius * 0.2, s.m_fRadius * 0.8);
		m_vPos = m_vCar;
		m_Zone.SetZone(shop, 25);
		m_sTitle = "Car Theft";
		m_sBrief = string.Format("There's a car in %1 somebody wants. That one, not any one. Drive it to %2 and walk away from it.", s.m_sName, m_sShop);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bSpawned)
		{
			if (!RZ_ContractHelp.AnyNear(players, m_vCar, 300))
				return;

			m_bSpawned = true;
			m_Car = RZ_Util.SpawnPrefab(m_sPrefab, m_vCar + Vector(0, 0.4, 0), Math.RandomFloat(0, 360));
			if (!m_Car)
			{
				m_eState = RZ_EMissionState.FAILED;
				return;
			}

			RZ_Util.SetVehicleFuel(m_Car, 0.3);
			m_Zone.SetVehicle(m_Car);
			SpawnGuards(gm, m_vCar, 2, 10, 30);
		}

		if (m_Zone.IsLost())
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		m_vPos = m_Car.GetOrigin();
		if (!m_bArrived)
		{
			RZ_ContractHelp.AddNear(ids, players, m_vPos, 6, m_aParticipants);
			if (!m_Zone.Check(ids, players))
				return;

			m_bArrived = true;
			m_Zone.GetCrew(ids, players, 30, m_aParticipants);
			RZ_ContractHelp.NotifyNear(gm, m_vPos, 60, m_sTitle, "Two men with grinders are already waiting. \"Out. Leave the keys.\"");
			return;
		}

		if (RZ_VehicleZone.DriverOf(m_Car, ids, players) < 0)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return string.Format("%1, %2\nChop shop: %3 (%4)", GetLocationName(gm), RZ_Util.DirectionText(from, m_vPos), m_sShop, RZ_Util.DirectionText(from, m_Zone.m_vPos));
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		// Stripped at the shop, or never stolen: it goes, unless someone sits in it
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		if (m_Car && RZ_VehicleZone.DriverOf(m_Car, ids, players) < 0)
			RZ_Util.DeleteUnit(m_Car);
	}
}

//------------------------------------------------------------------------------------------------
//! #9 Loot the Armory: break into the gun cage at a police station (the town centre if the map
//! has none): 60 s at the cage (40 s for a Police officer, who knows the layout), and it's loud.
//! The case goes to a Red Hand drop. Pays mostly Favor; nobody walks off with the guns.
class RZ_Mission_LootTheArmory : RZ_Mission_Delivery
{
	protected float m_fCracked;
	protected bool m_bGuarded;

	protected static const float CRACK_SECONDS = 60;
	protected static const float POLICE_SECONDS = 40;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 20, 35);
		m_Req.Bonus("police");
		m_fTimeLeft = 2700;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!CanRun(gm, RZ_EFaction.BANDITS))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 90);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		array<string> kinds = {"police"};
		RZ_Landmark station = RZ_LandmarkFinder.FindNearest(s.m_vCenter, s.m_fRadius, kinds);
		string place = "the police post";
		if (station)
		{
			m_vPos = LandPointNear(station.m_vPos, 3, 8);
			place = station.m_sName;
		}
		else
		{
			m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.4);
		}

		vector meet;
		string meetName;
		if (!RZ_LeaderJob.MeetingPoint(gm, RZ_EFaction.BANDITS, s.m_vCenter, meet, meetName) || !SpawnDrop(gm, RZ_EFaction.BANDITS, meet))
			return false;

		m_sTitle = "Loot the Armory";
		m_sBrief = string.Format("The gun cage in %1 in %2 was never emptied. Cut it open (it'll be loud) and bring what's inside to %3.", place, s.m_sName, meetName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDeliveryTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bGuarded && RZ_ContractHelp.AnyNear(players, m_vPos, 300))
		{
			m_bGuarded = true;
			SpawnGuards(gm, m_vPos, 4, 10, 40);
		}

		if (m_bItemSpawned || CountPlayersNear(players, m_vPos, 8) == 0)
			return;

		float needed = CRACK_SECONDS;
		if (AnyBonusNear(gm, ids, players, m_vPos, 8))
			needed = POLICE_SECONDS;

		m_fCracked += dt;
		gm.GetHeat().AddHeat(m_vPos, 1 * dt);
		if (m_fCracked >= needed && SpawnItem(gm, m_vPos))
			RZ_ContractHelp.NotifyNear(gm, m_vPos, 50, m_sTitle, "The last bar gives with a shriek. A long case, heavier than it looks. Every infected in town heard that.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return super.GetWhereText(gm, from) + "\nDrop: " + RZ_Util.DirectionText(from, GetDropPos(0));
	}
}

//------------------------------------------------------------------------------------------------
//! #10 Protection Racket: "collect" from a Haven safehouse. Ask the keeper: they pay (the
//! safehouse loses 3 food) or they refuse. Refused: lean on them. With the survivor group prefab
//! set, the keeper's people stand up to you and it's a fight (they pay once it's over); without
//! it, they pay with worse grace (+5 Haven Infamy more). Not for Elders.
class RZ_Mission_ProtectionRacket : RZ_PuzzleMission
{
	protected int m_iHouse = -1;
	protected bool m_bAsked;
	protected bool m_bRefused;
	protected bool m_bLeaned;
	protected int m_iCollector = -1;
	protected IEntity m_Locals;
	protected bool m_bFight;		// the group deletes itself when its last member dies: track the fight here
	protected float m_fFightAge;
	protected bool m_bSeenLocals;
	protected bool m_bExtraInfamy;
	protected int m_iTalk = -1;

	protected static const int FOOD = 3;
	protected static const float PAY_CHANCE = 0.55;
	protected static const int LEAN_INFAMY = 5;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.TRUSTED, 25, 35);
		SetRival(RZ_EFaction.SURVIVORS, 10);
		m_Req.Deny("elder");
		m_fTimeLeft = 2400;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (RZ_ContractHelp.CountSafehouses(gm) == 0 || !HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iHouse = gm.GetSafehouses().PickRandom();
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, m_iHouse);
		if (!h)
			return false;

		m_vPos = LandPointNear(h.m_vPos, 5, 12);
		m_iSettlement = h.m_iSettlement;
		m_iTalk = AddClue("The keeper", "\"I know who sent you. We've nothing to spare. We've people to feed.\"");
		SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPos, 0, "", "Ask for the money|Leave it", m_iTalk, "Talk to the keeper");
		m_sTitle = "Protection Racket";
		m_sBrief = string.Format("The safehouse in %1 is late. Go and remind the keeper what our protection is worth. They pay, or they learn.", h.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0 || m_bLeaned)
			return;

		if (option == 1)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You walk away. The crew will hear you went soft.");
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (option != 0)
			return;

		m_iCollector = playerId;
		if (!m_bAsked)
		{
			m_bAsked = true;
			if (Math.RandomFloat01() < PAY_CHANCE)
			{
				Pay(gm, playerId, "The keeper counts it out in tins and batteries without looking at you. \"Get out.\"");
				return;
			}

			m_bRefused = true;
			SetObjectLabels(0, "", "Lean on them|Leave it");
			gm.NotifyPlayer(playerId, m_sTitle, "\"No.\" The keeper folds their arms. People are watching from the doorways.");
			return;
		}

		if (!m_bRefused)
			return;

		m_bLeaned = true;
		ResourceName prefab = gm.GetSurvivorGroupPrefab();
		if (prefab.IsEmpty())
		{
			m_bExtraInfamy = true;
			Pay(gm, playerId, "You lean in close and say it quietly. The keeper pays. Nobody in that doorway will ever forget your face.");
			return;
		}

		m_bFight = true;
		m_Locals = RZ_Util.SpawnPrefab(prefab, LandPointNear(m_vPos, 8, 15));
		RZ_Hostility.MarkFight(RZ_EFaction.SURVIVORS, m_Locals, 120);
		RZ_ContractHelp.NotifyNear(gm, m_vPos, 120, m_sTitle, "The keeper's people come out with whatever they could grab. They're not paying. Not while they're standing.");
	}

	//------------------------------------------------------------------------------------------------
	protected void Pay(RZ_GameModeComponent gm, int playerId, string text)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		RZ_Safehouse h = houses.Get(m_iHouse);
		if (h)
			h.m_iFood = Math.Max(0, h.m_iFood - FOOD);

		m_aParticipants.Clear();
		m_aParticipants.Insert(playerId);
		gm.NotifyPlayer(playerId, m_sTitle, text);
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bFight || m_eState != RZ_EMissionState.ACTIVE)
			return;

		// A null group (deleted when empty) counts as zero alive
		m_fFightAge += dt;
		int alive = RZ_Util.CountAlive(m_Locals);
		if (alive > 0)
			m_bSeenLocals = true;

		if (alive == 0 && (m_bSeenLocals || m_fFightAge > 15))
			Pay(gm, m_iCollector, "It's over. The keeper pays with blood on the floor. You did this.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
		if (m_bExtraInfamy)
			gm.AddInfamy(playerId, RZ_EFaction.SURVIVORS, LEAN_INFAMY);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		RZ_Util.DeleteUnit(m_Locals);
	}
}

//------------------------------------------------------------------------------------------------
//! #11 Hijack the Convoy: Bastion's next supply truck leaves its stronghold for an outpost in two
//! minutes (RZ_ConvoyWatch). Kill the crew, burn it, or drive it off: anyone within 300 m when it
//! happens is paid. It arrives: failed. Big Bastion Infamy.
class RZ_Mission_HijackTheConvoy : RZ_Mission
{
	protected ref RZ_ConvoyWatch m_Watch = new RZ_ConvoyWatch();
	protected float m_fWaiting;

	//------------------------------------------------------------------------------------------------
	//! Its position follows the moving truck: nothing of its own stands anywhere, so no building is kept unlocked.
	override void GetPropSpots(notnull array<vector> result)
	{
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.TRUSTED, 40, 40);
		m_Req.Need(RZ_EFaction.MILITARY, RZ_EStandingCheck.NOT_AT_LEAST_TRUSTED);
		SetRival(RZ_EFaction.MILITARY, 25);
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!RZ_ConvoyWatch.CanRun(gm, RZ_EFaction.MILITARY))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		if (!m_Watch.Request(gm, RZ_EFaction.MILITARY, 150))
			return false;

		m_vPos = m_Watch.GetPos(gm);
		m_iSettlement = gm.FindNearestSettlement(m_vPos);
		m_sTitle = "Hijack the Convoy";
		m_sBrief = string.Format("Bastion's truck leaves %1 for %2 in a couple of minutes. Find a bend in the road. Whatever's on it is ours.", m_Watch.m_sFrom, m_Watch.m_sTo);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_Watch.Update(gm);
		m_vPos = m_Watch.GetPos(gm);
		int result = m_Watch.m_iResult;
		if (result < 0)
		{
			m_fWaiting += dt;
			if (!m_Watch.HasLeft() && m_fWaiting > 900)
				m_eState = RZ_EMissionState.FAILED;

			return;
		}

		if (result == RZ_SupplyLines.TRUCK_ARRIVED || result == RZ_SupplyLines.TRUCK_LATE)
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		RZ_ContractHelp.AddNear(ids, players, m_Watch.m_vResultPos, 300, m_aParticipants);
		if (m_Watch.m_iDriver > 0 && !m_aParticipants.Contains(m_Watch.m_iDriver))
			m_aParticipants.Insert(m_Watch.m_iDriver);

		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnTruckEvent(RZ_GameModeComponent gm, int faction, int what, vector pos, int driver)
	{
		m_Watch.OnTruckEvent(faction, what, pos, driver);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return string.Format("%1 to %2, the truck is %3", m_Watch.m_sFrom, m_Watch.m_sTo, RZ_Util.DirectionText(from, m_Watch.GetPos(gm)));
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return string.Format("Hijack the Convoy: Bastion's truck got through to %1.", m_Watch.m_sTo);
	}
}

//------------------------------------------------------------------------------------------------
//! #13 Bust Out: Red Hand prisoners in a Bastion stockade near one of its posts. Cut the lock (20 s
//! at the gate, 8 s for an Ex-Con). If a Bastion squad guards it and you're not in Bastion's uniform
//! (RZ_Disguise), they see you and it's a fight. Then walk the prisoners (RZ_Escort) to a Red Hand
//! drop. Bastion Infamy.
class RZ_Mission_BustOut : RZ_PuzzleMission
{
	protected ref RZ_Escort m_Escort = new RZ_Escort();
	protected vector m_vGate;
	protected vector m_vMeet;
	protected string m_sMeetName;
	protected bool m_bShown;
	protected IEntity m_Guards;
	protected int m_iCutter = -1;
	protected float m_fCut;
	protected float m_fCutNeeded = 20;
	protected bool m_bOpen;
	protected bool m_bAlarm;

	protected static const float EXCON_SECONDS = 8;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.TRUSTED, 35, 35);
		SetRival(RZ_EFaction.MILITARY, 15);
		m_Req.Bonus("excon");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Contracts > Bastion squad, else the Bastion hunter squad, else the Bastion truck crew.
	static ResourceName SquadPrefab(RZ_GameModeComponent gm)
	{
		RZ_ContractSettings cs = gm.GetContractSettings();
		if (cs && !cs.m_sBastionSquadPrefab.IsEmpty())
			return cs.m_sBastionSquadPrefab;

		ResourceName hunters = gm.GetHunterPrefab(RZ_EFaction.MILITARY);
		if (!hunters.IsEmpty())
			return hunters;

		RZ_SupplySettings ss = gm.GetSupplySettings();
		if (ss)
			return ss.m_sBastionCrew;

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!RZ_Escort.HasRoom(gm) || !HasPrefab(gm, RZ_EPuzzleProp.OBJECT))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 80);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		vector post;
		string postName;
		if (!RZ_LeaderJob.MeetingPoint(gm, RZ_EFaction.MILITARY, s.m_vCenter, post, postName))
			return false;

		m_vGate = LandPointNear(post, 40, 70);
		if (!RZ_LeaderJob.MeetingPoint(gm, RZ_EFaction.BANDITS, m_vGate, m_vMeet, m_sMeetName))
			return false;

		m_vPos = m_vGate;
		m_iSettlement = gm.FindNearestSettlement(m_vGate);
		m_Escort.m_sName = "the prisoners";
		m_Escort.m_sPrefab = RZ_Escort.PrefabFor(gm, 0);
		m_Escort.SetDestination(m_vMeet);
		m_sTitle = "Bust Out";
		m_sBrief = string.Format("Bastion is holding some of ours in a stockade near %1. Cut them out and walk them to %2. In one of their uniforms, the guards won't look twice.", postName, m_sMeetName);
		return m_Escort.Begin(gm, LandPointNear(m_vGate, 3, 6));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bShown && RZ_ContractHelp.AnyNear(players, m_vGate, 300))
		{
			m_bShown = true;
			SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vGate, 0, "", "Cut the lock");
			m_Guards = RZ_Util.SpawnPrefab(SquadPrefab(gm), LandPointNear(m_vGate, 10, 25));
			TrackProp(m_Guards);
			if (m_Guards && gm.GetHostility())
				gm.GetHostility().WatchUnit(m_Guards, RZ_EFaction.MILITARY);
		}

		if (!m_bOpen)
		{
			UpdateCutting(gm, dt, ids, players);
			return;
		}

		m_Escort.Update(gm, dt, ids, players);
		m_vPos = m_Escort.m_vPos;
		RZ_ContractHelp.AddNear(ids, players, m_vPos, 120, m_aParticipants);
		if (m_Escort.IsDead())
			m_eState = RZ_EMissionState.FAILED;
		else if (m_Escort.IsArrived())
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateCutting(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_iCutter <= 0)
			return;

		IEntity cutter = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iCutter);
		if (!cutter || RZ_Util.DistXZ(cutter.GetOrigin(), m_vGate) > 4)
		{
			m_iCutter = -1;
			m_fCut = 0;
			return;
		}

		m_fCut += dt;
		if (m_fCut < m_fCutNeeded)
			return;

		m_bOpen = true;
		m_Escort.Go();
		RZ_ContractHelp.NotifyNear(gm, m_vGate, 80, m_sTitle, string.Format("The chain drops. Four thin, filthy men come out blinking. \"Took your time.\" Walk them to %1; they'll keep up.", m_sMeetName));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0 || option != 0 || m_bOpen)
			return;

		m_iCutter = playerId;
		m_fCut = 0;
		m_fCutNeeded = 20;
		if (HasBonus(gm, playerId))
			m_fCutNeeded = EXCON_SECONDS;

		string text = "You get the cutters on the chain. Stay at the gate.";
		bool disguised = gm.GetHostility() && gm.GetHostility().GetDisguise().IsDisguisedAs(playerId, RZ_EFaction.MILITARY);
		if (!m_bAlarm && RZ_Util.CountAlive(m_Guards) > 0 && !disguised)
		{
			m_bAlarm = true;
			RZ_Hostility.MarkFight(RZ_EFaction.MILITARY, m_Guards, 150);
			text = "A shout from the wire: the guards have seen you. Cut fast.";
		}
		else if (disguised)
		{
			text = "A guard glances at your uniform and looks away. Cut, quietly.";
		}

		gm.NotifyPlayer(playerId, m_sTitle, text);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return string.Format("%1, %2\nTo: %3 (%4)", GetLocationName(gm), RZ_Util.DirectionText(from, m_vPos), m_sMeetName, RZ_Util.DirectionText(from, m_vMeet));
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Escort.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! #17 Double Agent: carry a dossier of fake intel from the Red Hand to a Bastion drop. Bastion
//! may see through it (35%, 15% for a Police officer they believe): +20 Bastion Infamy for whoever
//! handed it in. Believed: a little Bastion Fame too, and Bastion wastes a patrol on ghosts.
class RZ_Mission_DoubleAgent : RZ_Mission_Delivery
{
	protected int m_iAgent = -1;
	protected bool m_bCaught;

	protected static const float CAUGHT_CHANCE = 0.35;
	protected static const float POLICE_CHANCE = 0.15;
	protected static const int CAUGHT_INFAMY = 20;
	protected static const int BELIEVED_FAME = 5;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.TRUSTED, 30, 30);
		m_Req.Need(RZ_EFaction.MILITARY, RZ_EStandingCheck.AT_LEAST_KNOWN);
		m_Req.Bonus("police");
		m_fTimeLeft = 3000;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!CanRun(gm, RZ_EFaction.MILITARY))
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 80);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		vector from;
		string fromName;
		if (!RZ_LeaderJob.MeetingPoint(gm, RZ_EFaction.BANDITS, s.m_vCenter, from, fromName))
			return false;

		vector meet;
		string meetName;
		if (!RZ_LeaderJob.MeetingPoint(gm, RZ_EFaction.MILITARY, from, meet, meetName))
			return false;

		m_vPos = from;
		m_iSettlement = gm.FindNearestSettlement(from);
		if (!SpawnItem(gm, from) || !SpawnDrop(gm, RZ_EFaction.MILITARY, meet))
			return false;

		m_sTitle = "Double Agent";
		m_sBrief = string.Format("Pick up a dossier from %1 and hand it to Bastion at %2 like you're doing them a favour. Patrol routes, a camp that isn't there. Look them in the eye.", fromName, meetName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelivered(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction)
	{
		// Paid by the Red Hand, whoever took it
		m_eGiver = RZ_EFaction.BANDITS;
		m_iAgent = playerId;
		float chance = CAUGHT_CHANCE;
		if (HasBonus(gm, playerId))
			chance = POLICE_CHANCE;

		m_bCaught = Math.RandomFloat01() < chance;
		if (m_bCaught)
		{
			gm.NotifyPlayer(playerId, "Task Force Bastion", "The lieutenant reads the first page twice, then looks up at you. \"Who gave you this?\" You leave before he finishes the question.");
			return;
		}

		gm.NotifyPlayer(playerId, "Task Force Bastion", "The lieutenant reads it, nods, thanks you. By tonight a patrol will be walking toward a camp that isn't there.");
		if (gm.GetTerritory() && gm.GetTerritory().IsActive())
			gm.GetTerritory().AddStrength(RZ_EFaction.MILITARY, -1);
	}

	//------------------------------------------------------------------------------------------------
	override void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
		if (playerId != m_iAgent)
			return;

		if (m_bCaught)
		{
			gm.AddInfamy(playerId, RZ_EFaction.MILITARY, CAUGHT_INFAMY);
			gm.NotifyPlayer(playerId, RZ_Factions.GetName(RZ_EFaction.MILITARY), string.Format("Bastion knows whose side you're on now. (+%1 Infamy)", CAUGHT_INFAMY));
			return;
		}

		gm.AddFame(playerId, RZ_EFaction.MILITARY, BELIEVED_FAME, false);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return super.GetWhereText(gm, from) + "\nBastion drop: " + RZ_Util.DirectionText(from, GetDropPos(0));
	}
}

//------------------------------------------------------------------------------------------------
//! #18 Collect the Bounty: someone has a price on their head. If a player online is Kill on Sight
//! with Bastion, the Haven or Halcyon (and not a friend of the Red Hand), it's them: paid to the
//! player who kills them (not for a death to the infected, a fall or suicide). Otherwise an armed man hiding outside a town (Contracts > Bounty target,
//! else the survivor group), spawned when someone comes near: kill him.
class RZ_Mission_CollectTheBounty : RZ_Mission
{
	protected int m_iTarget = -1;
	protected string m_sTarget;
	protected IEntity m_Man;
	protected vector m_vHide;
	protected bool m_bSpawned;
	protected float m_fManAge;
	protected bool m_bSeenMan;
	protected float m_fGone;
	protected float m_fTipTimer;

	protected static const float CLAIM_RADIUS = 150;

	//------------------------------------------------------------------------------------------------
	//! A player target moves (nothing of the mission's stands anywhere); the hiding man's spot counts.
	override void GetPropSpots(notnull array<vector> result)
	{
		if (m_iTarget <= 0)
			super.GetPropSpots(result);
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.CHAMPION, 40, 50);
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	static ResourceName TargetPrefab(RZ_GameModeComponent gm)
	{
		RZ_ContractSettings cs = gm.GetContractSettings();
		if (cs && !cs.m_sBountyTargetPrefab.IsEmpty())
			return cs.m_sBountyTargetPrefab;

		return gm.GetSurvivorGroupPrefab();
	}

	//------------------------------------------------------------------------------------------------
	//! A player someone else has put a price on (Kill on Sight), whom the Red Hand doesn't count as
	//! a friend. -1 if none.
	protected int FindWanted(RZ_GameModeComponent gm)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int playerId : ids)
		{
			if (RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.TRUSTED))
				continue;

			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				if (f != RZ_EFaction.BANDITS && gm.GetTier(playerId, f) == RZ_ETier.KILL_ON_SIGHT)
					return playerId;
			}
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (TargetPrefab(gm).IsEmpty() && FindWanted(gm) < 0)
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsEligible(RZ_GameModeComponent gm, int playerId)
	{
		// Nobody collects on themselves
		if (playerId == m_iTarget)
			return false;

		return super.IsEligible(gm, playerId);
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iTarget = FindWanted(gm);
		m_sTitle = "Collect the Bounty";
		if (m_iTarget > 0)
		{
			IEntity t = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iTarget);
			if (!t)
				return false;

			m_sTarget = RZ_Util.PlayerName(m_iTarget);
			m_vPos = t.GetOrigin();
			m_iSettlement = gm.FindNearestSettlement(m_vPos);
			m_sBrief = string.Format("There's a price on %1 and we mean to collect it. Last seen near %2. Whoever puts them down gets paid.", m_sTarget, GetLocationName(gm));
			return true;
		}

		if (TargetPrefab(gm).IsEmpty())
			return false;

		m_iSettlement = PickSettlement(gm, 0, 80);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vHide = LandPointNear(s.m_vCenter, s.m_fRadius + 80, s.m_fRadius + 300);
		m_vPos = m_vHide;
		m_sTarget = "the man";
		m_sBrief = string.Format("A man who owes the wrong people is hiding outside %1. He's armed, and he knows we're coming. Make an example.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_iTarget > 0)
		{
			UpdatePlayerTarget(gm, dt, ids, players);
			return;
		}

		if (!m_bSpawned)
		{
			if (!RZ_ContractHelp.AnyNear(players, m_vHide, 300))
				return;

			m_bSpawned = true;
			m_Man = RZ_Util.SpawnPrefab(TargetPrefab(gm), m_vHide);
			if (!m_Man)
			{
				m_eState = RZ_EMissionState.FAILED;
				return;
			}

			RZ_Hostility.MarkFight(RZ_EFaction.SURVIVORS, m_Man, 120);
		}

		m_fManAge += dt;
		int alive = RZ_Util.CountAlive(m_Man);
		if (alive > 0)
		{
			m_bSeenMan = true;
			m_vPos = RZ_Util.GetUnitPosition(m_Man);
			RZ_ContractHelp.AddNear(ids, players, m_vPos, CLAIM_RADIUS, m_aParticipants);
			return;
		}

		if (m_bSeenMan || m_fManAge > 15)
		{
			RZ_ContractHelp.AddNear(ids, players, m_vPos, CLAIM_RADIUS, m_aParticipants);
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdatePlayerTarget(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		int idx = ids.Find(m_iTarget);
		if (idx < 0)
		{
			// Gone (offline) for five minutes: the job lapses
			m_fGone += dt;
			if (m_fGone > 300)
				m_eState = RZ_EMissionState.FAILED;

			return;
		}

		m_fGone = 0;
		m_vPos = players[idx].GetOrigin();
		m_fTipTimer += dt;
		if (m_fTipTimer < 300)
			return;

		m_fTipTimer = 0;
		foreach (int playerId : m_aOfferedTo)
		{
			if (playerId != m_iTarget)
				gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 was seen near %2 (grid %3).", m_sTarget, GetLocationName(gm), RZ_SettlementScanner.GridReference(m_vPos)));
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDied(RZ_GameModeComponent gm, int playerId, vector pos)
	{
		if (m_iTarget <= 0 || playerId != m_iTarget || m_eState != RZ_EMissionState.ACTIVE)
			return;

		// Paid to the player who killed them, and only for a kill (not the infected, a fall or suicide)
		int killer = -1;
		if (gm.GetPerks())
			killer = gm.GetPerks().KillerOf(playerId);

		if (killer <= 0 || killer == m_iTarget)
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		m_aParticipants.Clear();
		m_aParticipants.Insert(killer);
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return string.Format("Collect the Bounty: nobody collected on %1.", m_sTarget);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		RZ_Util.DeleteUnit(m_Man);
	}
}

//------------------------------------------------------------------------------------------------
//! #19 Kill the Officer: a Bastion outpost commander (never the stronghold's) has to die. Killed
//! by a player: paid, and Bastion puts you at Kill on Sight. Dead some other way: failed.
//! Needs a killable body (Leaders > Bastion outpost commander or Bastion leader).
class RZ_Mission_KillTheOfficer : RZ_Mission
{
	protected int m_iTarget = -1;
	protected string m_sTarget;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.CHAMPION, 60, 50);
		SetRival(RZ_EFaction.MILITARY, 75);
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Leader PickOfficer(RZ_GameModeComponent gm)
	{
		RZ_Leaders leaders = gm.GetLeaders();
		if (!leaders)
			return null;

		RZ_LeaderSettings ls = leaders.GetSettings();
		if (!ls || (ls.m_sBastionOutpostPrefab.IsEmpty() && ls.m_sBastionLeaderPrefab.IsEmpty()))
			return null;

		array<RZ_Leader> list = {};
		leaders.GetFactionLeaders(RZ_EFaction.MILITARY, list);
		array<RZ_Leader> options = {};
		foreach (RZ_Leader l : list)
		{
			if (l.IsPresent() && !l.IsStronghold() && l.m_iHeldBy < 0)
				options.Insert(l);
		}

		if (options.IsEmpty())
			return null;

		return options.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!PickOfficer(gm))
			return 0;

		return 0.6;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Leader l = PickOfficer(gm);
		if (!l)
			return false;

		m_iTarget = l.m_iId;
		m_sTarget = l.m_sName;
		m_vPos = l.m_vPos;
		m_iSettlement = gm.FindNearestSettlement(l.m_vPos);
		m_sTitle = "Kill the Officer";
		m_sBrief = string.Format("%1 runs Bastion's post in %2, and runs it well. That's the problem. Make it a worse post.", l.m_sName, l.m_sPlace);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_Leader l = gm.GetLeaders().Get(m_iTarget);
		if (!l)
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (l.m_bAlive)
		{
			RZ_ContractHelp.AddNear(ids, players, l.m_vPos, 150, m_aParticipants);
			return;
		}

		if (l.m_iKilledBy <= 0)
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (!m_aParticipants.Contains(l.m_iKilledBy))
			m_aParticipants.Insert(l.m_iKilledBy);

		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return string.Format("Kill the Officer: %1 is out of reach.", m_sTarget);
	}
}

//------------------------------------------------------------------------------------------------
//! #20 Take the Lab, reframed: during the final stand, get into the lab's clean room and hold it
//! for 90 s while the crew carries the batch out. The batch is stolen: the final stand ends,
//! research drops back to 90 percent (RZ_CureResearch.StealBatch) and two doses reach the black
//! market. No stronghold falls, and the Red Hand doesn't win the cure this way: it only delays it.
class RZ_Mission_TakeTheLab : RZ_Mission_Hold
{
	protected bool m_bTaken;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.CHAMPION, 60, 60);
		SetRival(RZ_EFaction.INSTITUTE, 40);
		m_fHoldNeeded = 90;
		m_fHoldRadius = 10;
		m_fTimeLeft = 900;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!gm.HasLab() || !gm.GetCure().IsFinalStand())
			return 0;

		return 3;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = gm.GetLabSettlement();
		m_vPos = LandPointNear(gm.GetLabPosition(), 15, 25);
		m_fParticipationRadius = 40;
		m_sTitle = "Take the Lab";
		m_sBrief = "Halcyon is cooking the cure and every gun in the Zone is at their door. While they're busy with the dead, walk into the clean room and hold it for ninety seconds. We'll carry the batch out ourselves.";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool CanHold(RZ_GameModeComponent gm)
	{
		if (gm.GetCure().IsFinalStand())
			return true;

		// The stand ended (won or lost) before the batch could be taken
		if (m_eState == RZ_EMissionState.ACTIVE)
			m_eState = RZ_EMissionState.FAILED;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The batch is taken the moment the hold is done: nothing stolen (the stand had already
	//! ended) is a failure, not an unpaid success.
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		super.OnUpdate(gm, dt, ids, players);
		if (m_eState != RZ_EMissionState.SUCCEEDED || m_bTaken)
			return;

		m_bTaken = true;
		if (!gm.GetCure().StealBatch())
			m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Take the Lab: the batch never left the clean room.";
	}
}
