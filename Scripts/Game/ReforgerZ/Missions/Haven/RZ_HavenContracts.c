//! The Haven's later contracts (docs/MISSIONS.md, "The Haven": #6, 8, 12, 14, 15, 18, 19, 20),
//! built on the mission building blocks (Missions/Blocks/).

//------------------------------------------------------------------------------------------------
//! #6 Fuel for the Lights: pour two jerrycans into a safehouse generator. The lights then run for
//! 10 minutes (20 if a Mechanic poured), and the light and the engine draw infected (heat).
//! Paid when they go out. ★ Mechanic: the generator runs twice as long.
class RZ_Mission_FuelForTheLights : RZ_PuzzleMission
{
	protected int m_iHouse = -1;
	protected vector m_vGen;
	protected bool m_bGenSpawned;
	protected int m_iPoured;
	protected bool m_bMechanic;
	protected bool m_bLit;
	protected float m_fRunLeft;

	protected static const int CANS = 2;
	protected static const float RUN_SECONDS = 600;
	protected static const float HEAT_PER_SECOND = 0.4;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 18, 20);
		m_Req.Bonus("mechanic");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (RZ_ContractHelp.CountSafehouses(gm) == 0 || !HasPrefab(gm, RZ_EPuzzleProp.OBJECT))
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

		m_vPos = h.m_vPos;
		m_iSettlement = h.m_iSettlement;
		m_vGen = LandPointNear(h.m_vPos, 6, 14);
		m_sTitle = "Fuel for the Lights";
		m_sBrief = string.Format("The safehouse in %1 has a generator and nothing to put in it. Bring two jerrycans and pour them in. The lights will draw them, so stay a while.", h.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bGenSpawned && RZ_ContractHelp.AnyNear(players, m_vGen, 250))
		{
			m_bGenSpawned = true;
			SpawnObject(gm, RZ_EPuzzleProp.FUSEBOX, m_vGen, 0, "Pour in a jerrycan", "");
		}

		if (!m_bLit)
			return;

		gm.GetHeat().AddHeat(m_vPos, HEAT_PER_SECOND * dt);
		m_fRunLeft -= dt;
		if (m_fRunLeft <= 0 && m_eState == RZ_EMissionState.ACTIVE)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx != 0 || !user)
			return;

		if (m_bLit)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "It's running. The windows are lit for the first time in days.");
			return;
		}

		if (RZ_ItemHelper.Take(user, RZ_ContractHelp.FuelWords(gm), 1) <= 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You've no fuel on you. A full jerrycan, from a wreck or a petrol pump.");
			return;
		}

		m_iPoured++;
		if (HasBonus(gm, playerId))
			m_bMechanic = true;

		if (m_iPoured < CANS)
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("Glug, glug. %1 more.", CANS - m_iPoured));
			return;
		}

		m_bLit = true;
		m_fRunLeft = RUN_SECONDS;
		string text = "It coughs, catches, settles. Somewhere inside a cheer goes up as the bulbs come on. Every infected for a kilometre can see it too.";
		if (m_bMechanic)
		{
			m_fRunLeft = RUN_SECONDS * 2;
			text += " You lean the mixture and clean the plug: it'll run twice as long.";
		}

		RZ_ContractHelp.NotifyNear(gm, m_vPos, 150, m_sTitle, text);
		m_fTimeLeft = Math.Max(m_fTimeLeft, m_fRunLeft + 60);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return string.Format("The lights in %1 went out on their own, for once. People got a night's work done under them.", GetLocationName(gm));
	}
}

//------------------------------------------------------------------------------------------------
//! #8 Harvest Guard: farmers bring in what's left in the fields outside a quiet town. Hold the
//! field for 10 minutes against waves. If the survivor group prefab is set they're really
//! there and must live. ★ Hunter: they stay hidden longer (the first wave comes later).
//! Pays food to the nearest safehouse.
class RZ_Mission_HarvestGuard : RZ_Mission_Hold
{
	protected IEntity m_Farmers;
	protected float m_fFarmerAge;
	protected bool m_bSeenFarmers;
	protected bool m_bBonusChecked;

	protected static const int FOOD = 3;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 22, 20);
		m_Req.Bonus("hunter");
		m_fHoldNeeded = 600;
		m_fHoldRadius = 70;
		m_fWaveInterval = 120;
		m_iMaxWaves = 4;
		m_fWaveDistance = 250;
		m_fTimeLeft = 2400;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 45);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius + 60, s.m_fRadius + 220);
		m_fParticipationRadius = 120;
		m_sTitle = "Harvest Guard";
		m_sBrief = string.Format("Farmers from the safehouses are bringing in what's left in the fields outside %1. Ten minutes of work. Keep the infected off them.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnHoldStarted(RZ_GameModeComponent gm)
	{
		ResourceName prefab = gm.GetSurvivorGroupPrefab();
		if (!prefab.IsEmpty())
			m_Farmers = RZ_Util.SpawnPrefab(prefab, LandPointNear(m_vPos, 5, 20));

		NotifyParticipants(gm, m_sTitle, "The farmers are out in the rows with sacks. Keep them working.");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnHoldTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bBonusChecked)
		{
			m_bBonusChecked = true;
			if (AnyBonusNear(gm, ids, players, m_vPos, m_fHoldRadius))
			{
				m_fWaveTimer = -120;
				NotifyParticipants(gm, m_sTitle, "You show them how to work low between the rows. Nothing notices them for a while.");
			}
		}

		if (!m_Farmers)
			return;

		m_fFarmerAge += dt;
		int alive = RZ_Util.CountAlive(m_Farmers);
		if (alive > 0)
			m_bSeenFarmers = true;

		if (alive == 0 && (m_bSeenFarmers || m_fFarmerAge > 15) && m_eState == RZ_EMissionState.ACTIVE)
			m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses)
			return;

		int idx = houses.FindNearest(m_vPos);
		RZ_Safehouse h = houses.Get(idx);
		if (!h)
			return;

		houses.AddFood(idx, FOOD);
		NotifyParticipants(gm, m_sTitle, string.Format("The sacks go to the safehouse in %1: %2 days of food.", h.m_sName, FOOD));
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return string.Format("Harvest Guard (%1): the farmers didn't make it back.", GetLocationName(gm));
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		RZ_Util.DeleteUnit(m_Farmers);
	}
}

//------------------------------------------------------------------------------------------------
//! #12 Refugee Column: a group waits at the edge of a bad town. Walk them along the road to a
//! safehouse 0.8-3.5 km away (RZ_Escort). They only walk while someone is with them and hide
//! when infected come close. Three join the safehouse. Big Fame.
class RZ_Mission_RefugeeColumn : RZ_Mission
{
	protected ref RZ_Escort m_Escort = new RZ_Escort();
	protected int m_iHouse = -1;
	protected bool m_bMet;
	protected string m_sDestName;

	protected static const int PEOPLE = 3;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.TRUSTED, 40, 35);
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (RZ_ContractHelp.CountSafehouses(gm) == 0 || !RZ_Escort.HasRoom(gm))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 20, 85);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_iHouse = RZ_ContractHelp.PickSafehouse(gm, s.m_vCenter, 800, 3500);
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, m_iHouse);
		if (!h)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius, s.m_fRadius + 80);
		m_sDestName = "the safehouse in " + h.m_sName;
		array<vector> route = {RZ_ContractHelp.Between(m_vPos, h.m_vPos, 0.5), h.m_vPos};
		m_Escort.m_sName = "the refugees";
		m_Escort.m_sPrefab = RZ_Escort.PrefabFor(gm, 0);
		m_Escort.SetRoute(route);
		m_sTitle = "Refugee Column";
		m_sBrief = string.Format("A family group is waiting at the edge of %1 with everything they own. Walk them to %2. They won't move without someone with them.", s.m_sName, m_sDestName);
		return m_Escort.Begin(gm, m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_ContractHelp.AddNear(ids, players, m_Escort.m_vPos, 120, m_aParticipants);
		if (!m_bMet && CountPlayersNear(players, m_Escort.m_vPos, 30) > 0)
		{
			m_bMet = true;
			m_Escort.Go();
			RZ_ContractHelp.NotifyNear(gm, m_Escort.m_vPos, 60, m_sTitle, string.Format("They pick up their bags. \"Lead on.\" Head for %1; they'll stop if you get too far ahead or the infected come close.", m_sDestName));
		}

		m_Escort.Update(gm, dt, ids, players);
		m_vPos = m_Escort.m_vPos;
		if (m_Escort.IsDead())
			m_eState = RZ_EMissionState.FAILED;
		else if (m_Escort.IsArrived())
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, m_iHouse);
		if (h)
			gm.GetSafehouses().AddRescued(h.m_vPos, PEOPLE, m_aParticipants);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return string.Format("%1, %2\nTo: %3", GetLocationName(gm), RZ_Util.DirectionText(from, m_vPos), m_sDestName);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Refugee Column: the column never reached " + m_sDestName + ".";
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		m_Escort.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! #14 Water Purifier: three parts of a purifier are scattered through a town. Carry them to the
//! Haven drop at a safehouse. ★ Mechanic/Lab Tech: two parts are enough (they improvise the third).
class RZ_Mission_WaterPurifier : RZ_Mission
{
	protected ref array<vector> m_aSpots = {};
	protected ref array<IEntity> m_aParts = {};
	protected ref array<bool> m_aSpawned = {};
	protected ref array<bool> m_aIn = {};		// handed in at the drop
	protected ref array<int> m_aSpawns = {};
	protected IEntity m_Drop;
	protected int m_iDelivered;
	protected bool m_bBonusDelivered;
	protected int m_iHouse = -1;

	protected static const int PARTS = 3;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.TRUSTED, 30, 30);
		m_Req.Bonus("mechanic");
		m_Req.Bonus("labtech");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (RZ_ContractHelp.CountSafehouses(gm) == 0 || !RZ_Mission_Delivery.CanRun(gm, RZ_EFaction.SURVIVORS))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 15, 75);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_iHouse = RZ_ContractHelp.PickSafehouse(gm, s.m_vCenter, 500, 4000);
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, m_iHouse);
		if (!h)
			return false;

		m_vPos = s.m_vCenter;
		for (int i = 0; i < PARTS; i++)
		{
			m_aSpots.Insert(LandPointNear(s.m_vCenter, 10, s.m_fRadius * 0.7));
			m_aParts.Insert(null);
			m_aSpawned.Insert(false);
			m_aIn.Insert(false);
			m_aSpawns.Insert(0);
		}

		m_Drop = RZ_Util.SpawnPrefab(gm.GetDropPointPrefab(RZ_EFaction.SURVIVORS), RZ_Util.GroundPos(LandPointNear(h.m_vPos, 4, 10)), Math.RandomFloat(0, 360));
		if (!m_Drop)
			return false;

		m_sTitle = "Water Purifier";
		m_sBrief = string.Format("A water purifier came apart in %1: three parts, somewhere in the streets. Bring them to the Haven drop at the safehouse in %2 and nobody there gets sick from the well again.", s.m_sName, h.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		int lost = 0;
		foreach (int i, vector spot : m_aSpots)
		{
			if (m_aIn[i])
				continue;

			if (!m_aSpawned[i])
			{
				if (!RZ_ContractHelp.AnyNear(players, spot, 200))
					continue;

				m_aSpawned[i] = true;
				m_aSpawns[i] = m_aSpawns[i] + 1;
				IEntity part = RZ_Util.SpawnPrefab(gm.GetMissionItemPrefab(), spot + Vector(0, 0.3, 0));
				RZ_MissionItemComponent.Tag(part, m_iId);
				m_aParts[i] = part;
				if (m_aSpawns[i] == 1)
					SpawnGuards(gm, spot, 2, 8, 30);
				continue;
			}

			if (!m_aParts[i])
			{
				// Gone with a carrier who left, or a body that was cleared: it turns up where it
				// was found again (a few times); after that it's lost
				if (m_aSpawns[i] < 4)
					m_aSpawned[i] = false;
				else
					lost++;

				continue;
			}

			int carrier = RZ_Util.GetCarrierPlayerId(m_aParts[i]);
			if (carrier > 0 && !m_aParticipants.Contains(carrier))
				m_aParticipants.Insert(carrier);
		}

		// Too many parts gone for good to finish it
		if (lost > 0 && PARTS - lost < Needed() && m_eState == RZ_EMissionState.ACTIVE)
			m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	protected int Needed()
	{
		if (m_bBonusDelivered)
			return PARTS - 1;

		return PARTS;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnItemDelivered(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction, IEntity drop)
	{
		if (faction != RZ_EFaction.SURVIVORS || drop != m_Drop || m_eState != RZ_EMissionState.ACTIVE)
			return false;

		m_iDelivered++;
		if (!m_aParticipants.Contains(playerId))
			m_aParticipants.Insert(playerId);

		// Which part this is: the one the deliverer carries, else the nearest to the drop (so a
		// handed-in part is never put back in the street)
		int which = -1;
		float bestDist = float.MAX;
		foreach (int i, IEntity part : m_aParts)
		{
			if (!part || m_aIn[i])
				continue;

			if (RZ_Util.GetCarrierPlayerId(part) == playerId)
			{
				which = i;
				break;
			}

			float d = RZ_Util.DistXZ(part.GetOrigin(), drop.GetOrigin());
			if (d < bestDist)
			{
				bestDist = d;
				which = i;
			}
		}

		if (which >= 0)
			m_aIn[which] = true;

		if (HasBonus(gm, playerId))
			m_bBonusDelivered = true;

		int needed = Needed();
		if (m_iDelivered >= needed)
		{
			m_eState = RZ_EMissionState.SUCCEEDED;
			if (m_iDelivered < PARTS)
				gm.NotifyPlayer(playerId, m_sTitle, "Two parts and some wire and tape: it'll work. You've seen worse.");

			return true;
		}

		gm.NotifyPlayer(playerId, m_sTitle, string.Format("Part %1 of %2 is in.", m_iDelivered, needed));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, m_iHouse);
		if (h)
			return string.Format("The purifier at the safehouse in %1 is running. Clean water.", h.m_sName);

		return super.GetSuccessText(gm);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		RZ_Util.DeleteUnit(m_Drop);
		foreach (IEntity part : m_aParts)
		{
			RZ_Util.DeleteUnit(part);
		}

		m_aParts.Clear();
	}
}

//------------------------------------------------------------------------------------------------
//! #15 Fortify the Safehouse: hold a safehouse for 3 minutes while the barricades go up (the
//! hammering carries). ★ Mechanic: the work goes 1.5 times faster. The town is barricaded.
class RZ_Mission_FortifySafehouse : RZ_Mission_Hold
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.TRUSTED, 30, 30);
		m_Req.Bonus("mechanic");
		m_fBonusHoldSpeed = 1.5;
		m_fHoldNeeded = 180;
		m_fHoldRadius = 30;
		m_fWaveInterval = 50;
		m_iMaxWaves = 3;
		m_fWaveDistance = 220;
		m_fHeatPerSecond = 0.3;
		m_fTimeLeft = 2400;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (RZ_ContractHelp.CountSafehouses(gm) == 0)
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, gm.GetSafehouses().PickRandom());
		if (!h)
			return false;

		m_vPos = h.m_vPos;
		m_iSettlement = h.m_iSettlement;
		m_fParticipationRadius = 80;
		m_sTitle = "Fortify the Safehouse";
		m_sBrief = string.Format("The safehouse in %1 is putting boards over its windows. The hammering will bring them. Stand with the people working.", h.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_TownMemory.ReportBarricaded(m_vPos, true);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return string.Format("The safehouse in %1 is boarded up tight.", GetLocationName(gm));
	}
}

//------------------------------------------------------------------------------------------------
//! #18 The Informant: wait at a Red Hand dead drop until a seller turns up, pose as a buyer, and
//! take what you heard to any safehouse. Some sellers notice who's asking: a little Red Hand
//! Infamy for the buyer if spotted (more likely carrying a gun). Only for players the Red Hand
//! is already wary of (Watched or worse); not for Police or Soldiers (they'd be made at once).
class RZ_Mission_TheInformant : RZ_PuzzleMission
{
	protected float m_fWaited;
	protected bool m_bContact;
	protected bool m_bBought;
	protected int m_iBuyer = -1;
	protected bool m_bSpotted;
	protected int m_iTalk = -1;
	protected string m_sBoss = "the crew boss";

	protected static const float WAIT_SECONDS = 60;
	protected static const float SPOT_CHANCE = 0.2;
	protected static const float ARMED_CHANCE = 0.3;
	protected static const int SPOTTED_INFAMY = 10;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.TRUSTED, 30, 25);
		m_Req.Need(RZ_EFaction.BANDITS, RZ_EStandingCheck.AT_LEAST_WATCHED);
		m_Req.Deny("police");
		m_Req.Deny("soldier");
		m_fTimeLeft = 3000;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (RZ_ContractHelp.CountSafehouses(gm) == 0 || !HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 10, 85);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.8, s.m_fRadius + 60);
		if (gm.GetLeaders())
		{
			RZ_Leader boss = gm.GetLeaders().FindNearest(RZ_EFaction.BANDITS, m_vPos, 100000);
			if (boss)
				m_sBoss = string.Format("%1 in %2", boss.m_sName, boss.m_sPlace);
		}

		m_iTalk = AddClue("The seller", "A thin man in a good coat, who looks at your hands before your face. \"You're new. What are you buying?\"");
		m_sTitle = "The Informant";
		m_sBrief = string.Format("The Red Hand sells out of a dead drop outside %1. Wait there like a buyer, buy what they're selling, and bring us the names you hear. Any safehouse will do. Don't get made.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bContact)
		{
			if (!ReachedBy(ids, players, m_vPos, 15))
				return;

			m_fWaited += dt;
			if (m_fWaited < WAIT_SECONDS)
				return;

			m_bContact = true;
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, LandPointNear(m_vPos, 3, 6), 0, "", "Buy what he's selling|Walk away", m_iTalk, "Talk to the seller");
			RZ_ContractHelp.NotifyNear(gm, m_vPos, 40, m_sTitle, "Someone comes up the path with his hands in his pockets, and stops a few metres off.");
			return;
		}

		if (!m_bBought)
			return;

		IEntity buyer = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iBuyer);
		if (buyer && gm.GetSafehouses().FindNearest(buyer.GetOrigin(), 30) >= 0)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0)
			return;

		if (option == 1)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You shake your head and walk. He watches you go. The chance is gone.");
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (option != 0 || m_bBought)
			return;

		m_bBought = true;
		m_iBuyer = playerId;
		m_aParticipants.Clear();
		m_aParticipants.Insert(playerId);

		float chance = SPOT_CHANCE;
		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (user && RZ_Util.IsArmed(user))
			chance += ARMED_CHANCE;

		m_bSpotted = Math.RandomFloat01() < chance;
		string text = string.Format("You pay for a bag of pills you don't want. He talks while he counts: whose crew this is (%1), where the stock comes from, when the next run goes. Take it to any safehouse.", m_sBoss);
		if (m_bSpotted)
			text += "\nAt the end he looks at you a moment too long. He's made you, or he will.";

		gm.NotifyPlayer(playerId, m_sTitle, text);
	}

	//------------------------------------------------------------------------------------------------
	override void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
		if (playerId != m_iBuyer || !m_bSpotted)
			return;

		gm.AddInfamy(playerId, RZ_EFaction.BANDITS, SPOTTED_INFAMY);
		gm.NotifyPlayer(playerId, RZ_Factions.GetName(RZ_EFaction.BANDITS), string.Format("Word gets back to the crew about the buyer who asked too many questions. (+%1 Infamy)", SPOTTED_INFAMY));
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		// The Haven knows their runs now: the crew loses a little ground
		if (gm.GetTerritory() && gm.GetTerritory().IsActive())
			gm.GetTerritory().AddStrength(RZ_EFaction.BANDITS, -1);
	}
}

//------------------------------------------------------------------------------------------------
//! #19 Recover the Bus: a bus (or truck) sits at a depot in another town with a splash of fuel.
//! Drive it to a safehouse (RZ_VehicleZone): that one, or any bus, truck or van, counts. Leave it
//! with them; it's theirs. ★ Mechanic: tunes it on the spot (half a tank).
class RZ_Mission_RecoverTheBus : RZ_Mission
{
	protected ref RZ_VehicleZone m_Zone = new RZ_VehicleZone();
	protected IEntity m_Bus;
	protected bool m_bSpawned;
	protected bool m_bTuned;
	protected bool m_bArrived;
	protected int m_iHouse = -1;
	protected vector m_vDepot;
	protected ResourceName m_sPrefab;

	//------------------------------------------------------------------------------------------------
	override bool NeedsVehicle()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.CHAMPION, 45, 40);
		m_Req.Bonus("mechanic");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Contracts > Bus, else the Haven's supply truck, else a car a fixed car becomes.
	static ResourceName BusPrefab(RZ_GameModeComponent gm)
	{
		RZ_ContractSettings cs = gm.GetContractSettings();
		if (cs && !cs.m_sBusPrefab.IsEmpty())
			return cs.m_sBusPrefab;

		RZ_SupplySettings ss = gm.GetSupplySettings();
		if (ss && !ss.m_sHavenTruck.IsEmpty())
			return ss.m_sHavenTruck;

		RZ_CarSettings cars = gm.GetCarSettings();
		if (cars && cars.m_aCarPrefabs && !cars.m_aCarPrefabs.IsEmpty())
			return cars.m_aCarPrefabs.GetRandomElement();

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (RZ_ContractHelp.CountSafehouses(gm) == 0 || BusPrefab(gm).IsEmpty())
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iHouse = gm.GetSafehouses().PickRandom();
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, m_iHouse);
		if (!h)
			return false;

		m_iSettlement = PickSettlementAway(gm, h.m_vPos, 1000, 4000, 15, 90);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_sPrefab = BusPrefab(gm);
		m_vDepot = LandPointNear(s.m_vCenter, s.m_fRadius * 0.5, s.m_fRadius + 40);
		m_vPos = m_vDepot;
		m_Zone.SetZone(h.m_vPos, 40);
		RZ_ContractSettings cs = gm.GetContractSettings();
		if (cs)
			m_Zone.AddWords(cs.m_sBusWords);

		array<ResourceName> own = {m_sPrefab};
		m_Zone.AddPrefabs(own);
		m_sTitle = "Recover the Bus";
		m_sBrief = string.Format("There's a bus at the depot in %1 with a splash of fuel in it. Get it running and drive it to the safehouse in %2: that's twenty people moved in one trip. Any bus or truck will do if you find a better one.", s.m_sName, h.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bSpawned)
		{
			if (!RZ_ContractHelp.AnyNear(players, m_vDepot, 300))
				return;

			m_bSpawned = true;
			m_Bus = RZ_Util.SpawnPrefab(m_sPrefab, m_vDepot + Vector(0, 0.5, 0), Math.RandomFloat(0, 360));
			if (!m_Bus)
			{
				m_eState = RZ_EMissionState.FAILED;
				return;
			}

			RZ_Util.SetVehicleFuel(m_Bus, 0.1);
			SpawnGuards(gm, m_vDepot, 4, 10, 40);
		}

		if (!m_bArrived && (!m_Bus || RZ_Util.IsDestroyed(m_Bus)))
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (m_Bus)
			RZ_ContractHelp.AddNear(ids, players, m_Bus.GetOrigin(), 15, m_aParticipants);

		if (!m_bTuned && m_Bus)
		{
			foreach (int i, IEntity p : players)
			{
				if (RZ_Util.DistXZ(p.GetOrigin(), m_Bus.GetOrigin()) > 10 || !HasBonus(gm, ids[i]) || !IsEligible(gm, ids[i]))
					continue;

				m_bTuned = true;
				RZ_Util.SetVehicleFuel(m_Bus, 0.5);
				gm.NotifyPlayer(ids[i], m_sTitle, "You find the fuel line kinked and a second tank nobody drained. Half a tank. She'll go.");
				break;
			}
		}

		if (!m_bArrived)
		{
			if (!m_Zone.Check(ids, players))
				return;

			m_bArrived = true;
			m_Zone.GetCrew(ids, players, 40, m_aParticipants);
			RZ_ContractHelp.NotifyNear(gm, m_Zone.m_vPos, 80, m_sTitle, "People come out to look at it. Park it and leave the keys in: it's theirs now.");
			return;
		}

		// Paid once nobody sits in it any more
		if (m_Zone.m_Arrived && RZ_VehicleZone.DriverOf(m_Zone.m_Arrived, ids, players) < 0)
			m_eState = RZ_EMissionState.SUCCEEDED;
		else if (!m_Zone.m_Arrived)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	//! A faction's supply truck on the road (left to the supply system).
	protected static bool IsSupplyTruck(RZ_GameModeComponent gm, IEntity v)
	{
		RZ_SupplyLines supply = gm.GetSupply();
		if (!supply || !v)
			return false;

		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			RZ_SupplyTruck t = supply.GetActiveTruck(f);
			if (t && t.m_Vehicle == v)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, m_iHouse);
		string text = super.GetWhereText(gm, from);
		if (h)
			text += string.Format("\nTo: the safehouse in %1 (%2)", h.m_sName, RZ_Util.DirectionText(from, h.m_vPos));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		// Delivered: the Haven keeps it (it leaves the world). Not delivered: the bus goes,
		// unless someone is sitting in it.
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		IEntity kept = m_Zone.m_Arrived;
		if (kept && !IsSupplyTruck(gm, kept) && RZ_VehicleZone.DriverOf(kept, ids, players) < 0)
			RZ_Util.DeleteUnit(kept);

		if (m_Bus && m_Bus != kept && RZ_VehicleZone.DriverOf(m_Bus, ids, players) < 0)
			RZ_Util.DeleteUnit(m_Bus);
	}
}

//------------------------------------------------------------------------------------------------
//! #20 Memorial: the Haven wants the names of the dead in an overrun town. Collect the ID tags
//! from five bodies (8 s each; the infected wait there), then bring them to any safehouse.
class RZ_Mission_Memorial : RZ_Mission
{
	protected ref array<vector> m_aBodies = {};
	protected ref array<float> m_aDwell = {};
	protected ref array<bool> m_aDone = {};
	protected ref array<IEntity> m_aProps = {};
	protected int m_iDone;
	protected bool m_bGuarded;
	protected bool m_bShown;
	protected bool m_bCollected;

	protected static const int BODIES = 5;
	protected static const float TAG_SECONDS = 8;

	//------------------------------------------------------------------------------------------------
	//! The bodies are listed by grid: never a rough area.
	override bool NeedsExactLocation()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.CHAMPION, 40, 35);
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (RZ_ContractHelp.CountSafehouses(gm) == 0)
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 65, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = s.m_vCenter;
		for (int i = 0; i < BODIES; i++)
		{
			m_aBodies.Insert(LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.8));
			m_aDwell.Insert(0);
			m_aDone.Insert(false);
		}

		m_sTitle = "Memorial";
		m_sBrief = string.Format("Nobody buried the people of %1. The Haven wants their names: the tags from five of them, brought to any safehouse. The ones who did this to them are still in the streets.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!m_bGuarded && s && RZ_ContractHelp.AnyNear(players, m_vPos, s.m_fRadius + 200))
		{
			m_bGuarded = true;
			SpawnGuards(gm, m_vPos, 5 + s.m_iSize * 2, 10, s.m_fRadius * 0.8);
		}

		if (!m_bShown && RZ_ContractHelp.AnyNear(players, m_vPos, 300))
		{
			m_bShown = true;
			foreach (vector b : m_aBodies)
			{
				IEntity e = RZ_Util.SpawnPrefab(gm.GetCorpsePilePrefab(), RZ_Util.GroundPos(b), Math.RandomFloat(0, 360));
				if (e)
					m_aProps.Insert(e);
			}
		}

		if (!m_bCollected)
		{
			CollectTags(gm, dt, ids, players);
			return;
		}

		foreach (int playerId : m_aParticipants)
		{
			IEntity p = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (p && gm.GetSafehouses().FindNearest(p.GetOrigin(), 30) >= 0)
			{
				m_eState = RZ_EMissionState.SUCCEEDED;
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CollectTags(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		foreach (int i, vector b : m_aBodies)
		{
			if (m_aDone[i] || CountPlayersNear(players, b, 4) == 0)
				continue;

			RZ_ContractHelp.AddNear(ids, players, b, 4, m_aParticipants);
			m_aDwell[i] = m_aDwell[i] + dt;
			if (m_aDwell[i] < TAG_SECONDS)
				continue;

			m_aDone[i] = true;
			m_iDone++;
			if (m_iDone < BODIES)
			{
				NotifyParticipants(gm, m_sTitle, string.Format("Tags: %1 of %2.", m_iDone, BODIES));
				continue;
			}

			m_bCollected = true;
			NotifyParticipants(gm, m_sTitle, "Five names, five chains, cold in your hand. Take them to any safehouse.");
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = super.GetWhereText(gm, from) + "\nBodies (grid):";
		foreach (int i, vector b : m_aBodies)
		{
			if (!m_aDone[i])
				text += " " + RZ_SettlementScanner.GridReference(b);
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (gm.GetLeaders())
			gm.GetLeaders().AddHeadline(string.Format("The Haven read out five names from %1 tonight, and added them to the wall.", GetLocationName(gm)), false);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		foreach (IEntity e : m_aProps)
		{
			RZ_Util.DeleteUnit(e);
		}

		m_aProps.Clear();
	}
}
