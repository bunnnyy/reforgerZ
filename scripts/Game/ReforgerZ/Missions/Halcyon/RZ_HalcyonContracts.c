//! Halcyon's later contracts (docs/MISSIONS.md, "Halcyon": #4, 7, 8, 9, 10, 11, 16, 17, 18, 19,
//! 20), built on the mission building blocks (Missions/Blocks/).

//------------------------------------------------------------------------------------------------
//! Shared: a Halcyon drop at the lab.
class RZ_HalcyonContractHelp
{
	//------------------------------------------------------------------------------------------------
	static vector DropPos(RZ_GameModeComponent gm)
	{
		return RZ_Mission.LandPointNear(gm.GetLabPosition(), 8, 15);
	}

	//------------------------------------------------------------------------------------------------
	static IEntity SpawnLabDrop(RZ_GameModeComponent gm)
	{
		return RZ_Util.SpawnPrefab(gm.GetDropPointPrefab(RZ_EFaction.INSTITUTE), RZ_Util.GroundPos(DropPos(gm)), Math.RandomFloat(0, 360));
	}
}

//------------------------------------------------------------------------------------------------
//! #4 Power the Lab: the lab's generator needs a jerrycan and a replacement part (the mission item,
//! in a garage or a factory in a nearby town). Pour and fit both at the generator by the lab:
//! antidote doses come twice as fast for two hours. ★ Mechanic: +50% Fame.
class RZ_Mission_PowerTheLab : RZ_PuzzleMission
{
	protected vector m_vGen;
	protected vector m_vPart;
	protected bool m_bGenShown;
	protected bool m_bPartShown;
	protected bool m_bFuel;
	protected bool m_bPart;
	protected string m_sPartPlace;
	protected IEntity m_Part;
	protected int m_iPartSpawns;

	protected static const float BOOST_SECONDS = 7200;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.STRANGER, 15, 20);
		m_Req.Bonus("mechanic");
		m_Req.m_bNotCured = true;
		m_fBonusFameMult = 1.5;
		m_fTimeLeft = 3000;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!gm.HasLab() || gm.GetMissionItemPrefab().IsEmpty() || !HasPrefab(gm, RZ_EPuzzleProp.OBJECT))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		vector lab = gm.GetLabPosition();
		RZ_Settlement s = gm.GetSettlement(PickSettlementAway(gm, lab, 500, 3000, 0, 90));
		if (!s)
			return false;

		array<string> kinds = {"industrial", "fuel", "power"};
		RZ_Landmark l = RZ_LandmarkFinder.FindNearest(s.m_vCenter, s.m_fRadius, kinds);
		m_sPartPlace = s.m_sName;
		if (l)
		{
			m_vPart = LandPointNear(l.m_vPos, 4, 10);
			m_sPartPlace = l.m_sName + " in " + s.m_sName;
		}
		else
		{
			m_vPart = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.6);
		}

		m_vGen = LandPointNear(lab, 10, 18);
		m_vPos = m_vGen;
		m_iSettlement = gm.GetLabSettlement();
		m_sTitle = "Power the Lab";
		m_sBrief = string.Format("The lab's generator is running on fumes and a cracked regulator. There's a replacement at %1. Bring it and a jerrycan of fuel.", m_sPartPlace);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bGenShown && RZ_ContractHelp.AnyNear(players, m_vGen, 250))
		{
			m_bGenShown = true;
			SpawnObject(gm, RZ_EPuzzleProp.FUSEBOX, m_vGen, 0, "Pour in a jerrycan|Fit the regulator", "");
		}

		// The regulator went with a carrier who left (or a body that was cleared): it turns up
		// where it was found again, a few times at most
		if (m_bPartShown && !m_Part && !m_bPart && m_iPartSpawns < 4)
			m_bPartShown = false;

		if (!m_bPartShown && RZ_ContractHelp.AnyNear(players, m_vPart, 200))
		{
			m_bPartShown = true;
			m_iPartSpawns++;
			m_Part = RZ_Util.SpawnPrefab(gm.GetMissionItemPrefab(), m_vPart + Vector(0, 0.3, 0));
			RZ_MissionItemComponent.Tag(m_Part, m_iId);
			TrackProp(m_Part);
			if (m_iPartSpawns == 1)
				SpawnGuards(gm, m_vPart, 2, 8, 30);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx != 0 || !user)
			return;

		if (value == 0 && !m_bFuel)
		{
			if (RZ_ItemHelper.Take(user, RZ_ContractHelp.FuelWords(gm), 1) <= 0)
			{
				gm.NotifyPlayer(playerId, m_sTitle, "You've no fuel on you.");
				return;
			}

			m_bFuel = true;
			gm.NotifyPlayer(playerId, m_sTitle, "The tank takes all of it.");
		}
		else if (value == 1 && !m_bPart)
		{
			if (!RZ_ContractHelp.CarriesItemOf(user, m_iId, true))
			{
				gm.NotifyPlayer(playerId, m_sTitle, string.Format("You've no regulator. It's at %1.", m_sPartPlace));
				return;
			}

			m_bPart = true;
			gm.NotifyPlayer(playerId, m_sTitle, "Four bolts and a gasket. It fits.");
		}

		if (m_bFuel && m_bPart)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.GetCure().BoostAntidotes(BOOST_SECONDS);
		NotifyParticipants(gm, "Halcyon", "The generator settles into a steady note. With clean power the synthesis runs twice as fast for a while.");
	}
}

//------------------------------------------------------------------------------------------------
//! #7 Install Sensors: place sensors in three infested towns (15 s at each spot). Each town draws
//! twice the specials for two hours (more samples, more danger). ★ Radio Op/Lab Tech: twice as fast.
class RZ_Mission_InstallSensors : RZ_Mission_VisitPoints
{
	protected ref array<int> m_aPointTowns = {};

	protected static const float SENSOR_SECONDS = 7200;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.KNOWN, 22, 20);
		m_Req.Bonus("radioham");
		m_Req.Bonus("labtech");
		m_Req.m_bNotCured = true;
		m_fBonusVisitSpeed = 2;
		m_fVisitRadius = 6;
		m_fVisitSeconds = 15;
		m_sPointName = "Sensor";
		m_fTimeLeft = 3600;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 50, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		array<int> towns = {m_iSettlement};
		if (!RZ_StepChain.PickTowns(gm, s.m_vCenter, 3, 400, 3000, 50, 100, towns))
			return false;

		foreach (int idx : towns)
		{
			RZ_Settlement t = gm.GetSettlement(idx);
			AddPoint(LandPointNear(t.m_vCenter, 0, t.m_fRadius * 0.5));
			m_aPointTowns.Insert(idx);
		}

		m_vPos = s.m_vCenter;
		m_fParticipationRadius = 0;
		m_sTitle = "Install Sensors";
		m_sBrief = string.Format("Halcyon wants sensors in three infested towns, starting with %1. Fifteen seconds to set each one up. They'll draw the specials in; that's the point.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnPointDone(RZ_GameModeComponent gm, int idx)
	{
		RZ_Settlement t = gm.GetSettlement(m_aPointTowns[idx]);
		if (t)
			t.m_fSensorSeconds = SENSOR_SECONDS;
	}
}

//------------------------------------------------------------------------------------------------
//! #8 Blood Drive: give blood at the lab (RZ_HealthCost: a quarter of your health, not below 70%,
//! once per life). Amends: works off Halcyon Infamy while Watched. Not for the infected.
class RZ_Mission_BloodDrive : RZ_PuzzleMission
{
	protected int m_iTalk = -1;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.KNOWN, 10, 12);
		m_bAmends = true;
		m_Req.m_bAllowWatched = true;
		m_Req.m_iInfection = -1;
		m_Req.m_bNotCured = true;
		m_fTimeLeft = 2400;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!gm.HasLab() || !HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_vPos = LandPointNear(gm.GetLabPosition(), 6, 12);
		m_iSettlement = gm.GetLabSettlement();
		m_iTalk = AddClue("The nurse", "A tired nurse with a cooler and a stack of bags. \"Uninfected blood is the only control we have. Sit, if you're willing.\"");
		SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPos, 0, "", "Give blood", m_iTalk, "Talk to the nurse");
		m_sTitle = "Blood Drive";
		m_sBrief = "The lab needs clean blood to test against. Come and give some. You'll feel it for a while.";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0 || option != 0)
			return;

		if (gm.GetProfile(playerId).IsInfected())
		{
			gm.NotifyPlayer(playerId, m_sTitle, "She looks at your eyes and puts the needle down. \"Not yours. I'm sorry.\"");
			return;
		}

		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		RZ_ContractSettings cs = gm.GetContractSettings();
		string why = RZ_HealthCost.Take(gm, playerId, user, "blood", cs.m_fBloodCost, cs.m_fBloodMinHealth);
		if (!why.IsEmpty())
		{
			gm.NotifyPlayer(playerId, m_sTitle, why);
			return;
		}

		m_aParticipants.Clear();
		m_aParticipants.Insert(playerId);
		gm.NotifyPlayer(playerId, m_sTitle, "A pinch, a long ten minutes, a biscuit. The room tilts when you stand up.");
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.GetCure().AddResearch(0.5, "A blood donation gave the lab a clean control sample.");
	}
}

//------------------------------------------------------------------------------------------------
//! #9 Cold Chain: an antidote batch for a Haven safehouse's clinic. It spoils 15 minutes after
//! it's picked up (25 for a Nurse, who packs it properly). Fame with Halcyon and the Haven.
class RZ_Mission_ColdChain : RZ_Mission_Delivery
{
	protected float m_fSpoilLeft = -1;
	protected bool m_bWarned;

	protected static const float SPOIL_SECONDS = 900;
	protected static const float NURSE_SECONDS = 1500;
	protected static const int HAVEN_FAME = 10;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.KNOWN, 22, 20);
		m_Req.Bonus("nurse");
		m_Req.Bonus("doctor");
		m_Req.m_fMinCure = 50;
		m_Req.m_bNotCured = true;
		m_fTimeLeft = 3000;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!gm.HasLab() || RZ_ContractHelp.CountSafehouses(gm) == 0 || !CanRun(gm, RZ_EFaction.SURVIVORS))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		vector lab = gm.GetLabPosition();
		int idx = RZ_ContractHelp.PickSafehouse(gm, lab, 1000, 5000);
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, idx);
		if (!h)
			return false;

		m_vPos = LandPointNear(lab, 5, 10);
		m_iSettlement = gm.GetLabSettlement();
		if (!SpawnItem(gm, m_vPos) || !SpawnDrop(gm, RZ_EFaction.SURVIVORS, LandPointNear(h.m_vPos, 4, 10)))
			return false;

		m_sTitle = "Cold Chain";
		m_sBrief = string.Format("A batch of antidotes for the clinic at the safehouse in %1. Once it's out of the cold box you have fifteen minutes. Run.", h.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnCarrierChanged(RZ_GameModeComponent gm, int previous, int carrier)
	{
		super.OnCarrierChanged(gm, previous, carrier);
		if (carrier <= 0 || m_fSpoilLeft >= 0)
			return;

		m_fSpoilLeft = SPOIL_SECONDS;
		string text = "The clock's running: fifteen minutes.";
		if (HasBonus(gm, carrier))
		{
			m_fSpoilLeft = NURSE_SECONDS;
			text = "You pack it in ice and newspaper the way they taught you. Twenty-five minutes.";
		}

		gm.NotifyPlayer(carrier, m_sTitle, text);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDeliveryTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_fSpoilLeft < 0 || m_bDelivered)
			return;

		m_fSpoilLeft -= dt;
		if (!m_bWarned && m_fSpoilLeft <= 180 && m_iCarrier > 0)
		{
			m_bWarned = true;
			gm.NotifyPlayer(m_iCarrier, m_sTitle, "The vials are warm to the touch. Three minutes.");
		}

		if (m_fSpoilLeft <= 0)
			m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
		gm.AddFame(playerId, RZ_EFaction.SURVIVORS, HAVEN_FAME, false);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Cold Chain: the batch spoiled on the way.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return super.GetWhereText(gm, from) + "\nClinic: " + RZ_Util.DirectionText(from, GetDropPos(0));
	}
}

//------------------------------------------------------------------------------------------------
//! #10 Escort the Scientist: a researcher takes readings at three sites. She follows the player
//! who leads her (RZ_Escort) and needs 20 s within 15 m of each site. Research when she's done.
class RZ_Mission_EscortTheScientist : RZ_PuzzleMission
{
	protected ref RZ_Escort m_Escort = new RZ_Escort();
	protected ref array<vector> m_aSites = {};
	protected ref array<string> m_aSiteNames = {};
	protected int m_iSite;
	protected float m_fReading;
	protected float m_fBeside;
	protected int m_iTalk = -1;

	protected static const float READ_SECONDS = 20;
	protected static const float SITE_RADIUS = 15;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.TRUSTED, 30, 30);
		m_Req.m_bNotCured = true;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!gm.HasLab() || !RZ_Escort.HasRoom(gm) || !HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		vector lab = gm.GetLabPosition();
		array<int> towns = {};
		if (!RZ_StepChain.PickTowns(gm, lab, 3, 400, 1500, 20, 95, towns))
			return false;

		foreach (int idx : towns)
		{
			RZ_Settlement t = gm.GetSettlement(idx);
			m_aSites.Insert(LandPointNear(t.m_vCenter, 0, t.m_fRadius * 0.5));
			m_aSiteNames.Insert(t.m_sName);
		}

		m_vPos = LandPointNear(lab, 6, 12);
		m_iSettlement = gm.GetLabSettlement();
		m_iTalk = AddClue("The researcher", "A researcher with a case of instruments. \"Three sites. I need twenty seconds at each and nobody chewing on me. Lead the way.\"");
		m_Escort.m_sName = "the researcher";
		m_Escort.m_sPrefab = RZ_Escort.PrefabFor(gm, 1);
		m_Escort.SetTalk(m_iId, 0, "Lead the way (she follows you)", m_iTalk, "Talk to the researcher");
		m_sTitle = "Escort the Scientist";
		m_sBrief = string.Format("One of our researchers needs readings from %1, %2 and %3. Collect her at the lab and keep her alive.", m_aSiteNames[0], m_aSiteNames[1], m_aSiteNames[2]);
		return m_Escort.Begin(gm, m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_Escort.GetFollowed() <= 0)
		{
			int beside = RZ_EscortHelp.PlayerBeside(ids, players, m_Escort.m_vPos, 4);
			if (beside > 0)
				m_fBeside += dt;

			if (beside > 0 && m_fBeside >= 10)
				OnChoice(gm, beside, 0, 0, null);
		}

		m_Escort.Update(gm, dt, ids, players);
		m_vPos = m_Escort.m_vPos;
		if (m_Escort.IsDead())
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (m_Escort.GetFollowed() <= 0 || m_iSite >= m_aSites.Count())
			return;

		RZ_ContractHelp.AddNear(ids, players, m_vPos, 100, m_aParticipants);
		if (!m_Escort.IsNear(m_aSites[m_iSite], SITE_RADIUS))
			return;

		m_fReading += dt;
		if (m_fReading < READ_SECONDS)
			return;

		m_fReading = 0;
		m_iSite++;
		if (m_iSite >= m_aSites.Count())
		{
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		RZ_ContractHelp.NotifyNear(gm, m_vPos, 80, m_sTitle, string.Format("\"Got it.\" She snaps the case shut. Next: %1 (%2).", m_aSiteNames[m_iSite], RZ_Util.DirectionText(m_vPos, m_aSites[m_iSite])));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0 || option != 0)
			return;

		m_Escort.Follow(playerId);
		m_Escort.Go();
		if (m_iSite < m_aSites.Count())
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("She falls in behind you. First site: %1 (%2).", m_aSiteNames[m_iSite], RZ_Util.DirectionText(m_Escort.m_vPos, m_aSites[m_iSite])));
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.GetCure().AddResearch(2, "A researcher came back from the field with three sets of readings.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = string.Format("The researcher is %1", RZ_Util.DirectionText(from, m_vPos));
		for (int i = m_iSite; i < m_aSites.Count(); i++)
		{
			text += string.Format("\nSite: %1 (%2)", m_aSiteNames[i], RZ_Util.DirectionText(from, m_aSites[i]));
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Escort.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! #11 Specimen from the Nest: take tissue from a living hive in an infested town (20 s within 4 m
//! of it) without destroying it, then carry the sample to the lab. Destroyed first: failed.
//! Needs the nest prefab.
class RZ_Mission_SpecimenFromTheNest : RZ_Mission_Delivery
{
	protected IEntity m_Nest;
	protected bool m_bShown;
	protected float m_fCutting;

	protected static const float CUT_SECONDS = 20;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.TRUSTED, 35, 30);
		m_Req.m_bNotCured = true;
		m_fTimeLeft = 3000;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!gm.HasLab() || gm.GetNestPrefab().IsEmpty() || !CanRun(gm, RZ_EFaction.INSTITUTE))
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 50, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.6);
		if (!SpawnDrop(gm, RZ_EFaction.INSTITUTE, RZ_HalcyonContractHelp.DropPos(gm)))
			return false;

		m_sTitle = "Specimen from the Nest";
		m_sBrief = string.Format("There's a hive in %1. We need living tissue from inside it. Cut a sample and bring it to the lab. Don't burn it, whatever you do.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDeliveryTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bShown)
		{
			if (!RZ_ContractHelp.AnyNear(players, m_vPos, 300))
				return;

			m_bShown = true;
			m_Nest = RZ_Util.SpawnPrefab(gm.GetNestPrefab(), m_vPos, Math.RandomFloat(0, 360));
			SpawnGuards(gm, m_vPos, 6, 8, 40, false);
			SpawnGuards(gm, m_vPos, 1, 10, 30, true);
		}

		if (m_bItemSpawned)
			return;

		if (!m_Nest || RZ_Util.IsDestroyed(m_Nest))
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (CountPlayersNear(players, m_vPos, 4) == 0)
			return;

		m_fCutting += dt;
		gm.GetHeat().AddHeat(m_vPos, 1 * dt);
		if (m_fCutting >= CUT_SECONDS && SpawnItem(gm, m_vPos))
			RZ_ContractHelp.NotifyNear(gm, m_vPos, 40, m_sTitle, "The tissue comes away wet and still twitching. Into the jar. The hive shudders; something inside heard you.");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelivered(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction)
	{
		gm.GetCure().AddResearch(4, "Living tissue from a hive reached the lab.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		RZ_Util.DeleteUnit(m_Nest);
	}
}

//------------------------------------------------------------------------------------------------
//! #16 Medical Drone: a prototype drone came down somewhere within 250 m of its last ping. Find the
//! wreck (it's visible within 300 m), take its core and bring it to the lab. ★ Mechanic/Radio Op:
//! pick up its beacon within 600 m and get the exact grid.
class RZ_Mission_MedicalDrone : RZ_Mission_Delivery
{
	protected vector m_vWreck;
	protected IEntity m_Wreck;
	protected bool m_bShown;
	protected ref array<int> m_aTold = {};

	//------------------------------------------------------------------------------------------------
	//! The brief already gives a rough ping (grid, give or take 250 m): never a rough area.
	override bool NeedsExactLocation()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.TRUSTED, 30, 30);
		m_Req.Bonus("mechanic");
		m_Req.Bonus("radioham");
		m_Req.m_bNotCured = true;
		m_fTimeLeft = 3000;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!gm.HasLab() || !CanRun(gm, RZ_EFaction.INSTITUTE))
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlementAway(gm, gm.GetLabPosition(), 800, 4000, 0, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius, s.m_fRadius + 400);
		m_vWreck = LandPointNear(m_vPos, 0, 250);
		if (!SpawnDrop(gm, RZ_EFaction.INSTITUTE, RZ_HalcyonContractHelp.DropPos(gm)))
			return false;

		m_sTitle = "Medical Drone";
		m_sBrief = string.Format("Our prototype drone dropped out of the sky near %1. Its last ping was grid %2, give or take 250 m. We need its core back.", s.m_sName, RZ_SettlementScanner.GridReference(m_vPos));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDeliveryTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity p : players)
		{
			if (m_aTold.Contains(ids[i]) || RZ_Util.DistXZ(p.GetOrigin(), m_vWreck) > 600 || !HasBonus(gm, ids[i]) || !IsEligible(gm, ids[i]))
				continue;

			m_aTold.Insert(ids[i]);
			gm.NotifyPlayer(ids[i], m_sTitle, string.Format("You catch its beacon: a thin chirp on the emergency band. Grid %1.", RZ_SettlementScanner.GridReference(m_vWreck)));
		}

		if (m_bShown || !RZ_ContractHelp.AnyNear(players, m_vWreck, 300))
			return;

		m_bShown = true;
		ResourceName prefab;
		RZ_ContractSettings cs = gm.GetContractSettings();
		if (cs)
			prefab = cs.m_sDroneWreckPrefab;

		if (prefab.IsEmpty())
			prefab = gm.GetPuzzlePrefab(RZ_EPuzzleProp.OBJECT);

		m_Wreck = RZ_Util.SpawnPrefab(prefab, m_vWreck, Math.RandomFloat(0, 360));
		SpawnItem(gm, LandPointNear(m_vWreck, 1, 2));
		SpawnGuards(gm, m_vWreck, 3, 10, 40);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelivered(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction)
	{
		gm.GetCure().AddResearch(1, "A drone's flight data reached the lab.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		RZ_Util.DeleteUnit(m_Wreck);
	}
}

//------------------------------------------------------------------------------------------------
//! #17 Patient Zero: follow the outbreak back through four towns (RZ_StepChain). In each a note or
//! a body names the next town; the fourth is where it started: search the records for 45 s while
//! what's left of the town comes for you. ★ Lab Tech: the clues also give the grid.
class RZ_Mission_PatientZero : RZ_PuzzleMission
{
	protected ref RZ_StepChain m_Chain = new RZ_StepChain();
	protected ref array<int> m_aTowns = {};
	protected ref array<int> m_aStepClues = {};

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		m_Chain.GetSpots(result);
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.CHAMPION, 50, 40);
		m_Req.Bonus("labtech");
		m_Req.m_bNotCured = true;
		m_fTimeLeft = 5400;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.CLUE))
			return 0;

		return 0.7;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 30, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_aTowns.Insert(m_iSettlement);
		if (!RZ_StepChain.PickTowns(gm, s.m_vCenter, 4, 600, 2500, 0, 100, m_aTowns))
			return false;

		m_Chain.SetMission(m_iId);
		for (int i = 0; i < 4; i++)
		{
			RZ_Settlement t = gm.GetSettlement(m_aTowns[i]);
			bool last = i == 3;
			float dwell = 0;
			if (last)
				dwell = 45;

			RZ_ChainStep st = m_Chain.AddStep(gm, m_aTowns[i], LandPointNear(t.m_vCenter, 0, t.m_fRadius * 0.5), 8, dwell);
			st.m_eClueProp = RZ_EPuzzleProp.BODY;
			if (last)
			{
				st.m_iGuards = 6;
				st.m_bSpecialGuard = true;
				st.m_iClue = AddClue("The clinic records", "A filing cabinet in a ransacked clinic. The first admission is three weeks before the Zone was sealed: a man brought in from a closed site in the hills, with a fever nobody could name. Someone wrote 'notify the Institute' in the margin and underlined it twice.");
				st.m_sReadLabel = "Read the records";
			}
			else
			{
				RZ_Settlement next = gm.GetSettlement(m_aTowns[i + 1]);
				st.m_iGuards = 2;
				st.m_iClue = AddClue("A body", string.Format("A hospital bracelet, dated the first week. Folded in the pocket: a transfer slip to %1, %2.", next.m_sName, RZ_Util.DirectionText(t.m_vCenter, next.m_vCenter)));
				st.m_sReadLabel = "Search the body";
			}

			m_aStepClues.Insert(st.m_iClue);
		}

		m_vPos = m_Chain.GetStep(0).m_vPos;
		m_sTitle = "Patient Zero";
		m_sBrief = string.Format("We think we can trace the outbreak back to the first case. It starts in %1: a body with a hospital bracelet. Follow where it leads.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetClueText(RZ_GameModeComponent gm, int playerId, int idx)
	{
		string text = super.GetClueText(gm, playerId, idx);
		int step = m_aStepClues.Find(idx);
		if (step < 0 || step >= 3 || !HasBonus(gm, playerId))
			return text;

		RZ_ChainStep next = m_Chain.GetStep(step + 1);
		if (next)
			text += string.Format("\nYou read the admission codes the way a lab reads them: grid %1.", RZ_SettlementScanner.GridReference(next.m_vPos));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		m_Chain.OnClueRead(idx);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		int done = m_Chain.Update(gm, dt, ids, players, m_aParticipants);
		if (m_Chain.IsDone())
		{
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		RZ_ChainStep st = m_Chain.GetCurrent();
		if (st)
			m_vPos = st.m_vPos;

		if (done >= 0 && m_Chain.GetCurrentIndex() == 3)
			NotifyParticipants(gm, m_sTitle, "This is the last town on the slips. Whatever happened, it happened here. Search the clinic, and take your time: they'll come.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.GetCure().AddResearch(6, "Halcyon has found the first case. Knowing where it started changes everything.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Chain.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! #18 Live Capture: lure a special into a cage trap, or wound and tie it, hold it while it's
//! sedated, and carry the case to the lab before it wakes (RZ_LiveCapture). Huge research.
class RZ_Mission_LiveCapture : RZ_PuzzleMission
{
	protected ref RZ_LiveCapture m_Capture = new RZ_LiveCapture();
	protected IEntity m_Drop;

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		m_Capture.GetSpots(result);
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.CHAMPION, 50, 40);
		m_Req.m_bNotCured = true;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!gm.HasLab() || !RZ_LiveCapture.HasRoom(gm) || gm.GetDropPointPrefab(RZ_EFaction.INSTITUTE).IsEmpty())
			return 0;

		return 0.7;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 40, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		vector lair = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.6);
		vector cage = LandPointNear(lair, 60, 120);
		m_Drop = RZ_HalcyonContractHelp.SpawnLabDrop(gm);
		if (!m_Drop)
			return false;

		TrackProp(m_Drop);
		m_vPos = cage;
		m_sTitle = "Live Capture";
		m_sBrief = string.Format("We need a special alive. There's a cage trap set outside its lair in %1. Get it to follow you in, drop the door, keep it calm until the sedative takes, then bring it to the lab before it wakes. Or bring it down without killing it and tie it. Dead is no use to us.", s.m_sName);
		return m_Capture.Begin(gm, m_iId, m_iSettlement, cage, lair, 0);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_Capture.Update(gm, dt, ids, players);
		if (m_Capture.m_eState == RZ_ECaptureState.CAUGHT)
			RZ_ContractHelp.AddNear(ids, players, m_Capture.m_vHeld, 30, m_aParticipants);

		if (m_Capture.IsLost())
			m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx == 0 && option == 0)
			m_Capture.TryCloseCage(gm, playerId);
	}

	//------------------------------------------------------------------------------------------------
	override void OnZombiesKilled(RZ_GameModeComponent gm, int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		m_Capture.OnSpecialKilled(gm, missionId, special);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnItemDelivered(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction, IEntity drop)
	{
		if (faction != RZ_EFaction.INSTITUTE || drop != m_Drop || !m_Capture.Deliver())
			return false;

		AddParticipant(playerId);
		m_eState = RZ_EMissionState.SUCCEEDED;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.GetCure().AddResearch(5, "A live special is in the lab's isolation room. Halcyon can watch the infection work.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Live Capture: " + m_Capture.m_sLostReason;
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Capture.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! #19 Serum Trial: for the bitten only. An untested serum at the lab: half the time it cures you,
//! half the time the turn comes twice as fast. Research either way; only the volunteer is paid.
class RZ_Mission_SerumTrial : RZ_PuzzleMission
{
	protected int m_iTalk = -1;
	protected int m_iVolunteer = -1;	// who took the serum: still paid when it cured them

	protected static const float CURE_CHANCE = 0.5;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.CHAMPION, 50, 40);
		m_Req.m_iInfection = 1;
		m_Req.m_bNotCured = true;
		m_fTimeLeft = 1800;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!gm.HasLab() || !HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return 0;

		return 2;
	}

	//------------------------------------------------------------------------------------------------
	//! The cure clears the infection the job asks for: the volunteer stays eligible for the payout.
	override bool IsEligible(RZ_GameModeComponent gm, int playerId)
	{
		if (m_iVolunteer > 0 && playerId == m_iVolunteer)
			return true;

		return super.IsEligible(gm, playerId);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_vPos = LandPointNear(gm.GetLabPosition(), 5, 10);
		m_iSettlement = gm.GetLabSettlement();
		m_iTalk = AddClue("The trial lead", "\"It's not ready. It might never be. But you're dying anyway, and we're out of time. Your choice.\"");
		SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPos, 0, "", "Take the serum", m_iTalk, "Talk to the trial lead");
		m_sTitle = "Serum Trial";
		m_sBrief = "You're bitten. We have something that might stop it, or might not. Come to the lab if you want to find out.";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0 || option != 0)
			return;

		RZ_PlayerProfile profile = gm.GetProfile(playerId);
		if (!profile.IsInfected() || !IsEligible(gm, playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"You're not the one I need.\" The case stays shut.");
			return;
		}

		m_aParticipants.Clear();
		m_aParticipants.Insert(playerId);
		m_iVolunteer = playerId;
		if (Math.RandomFloat01() < CURE_CHANCE)
		{
			gm.CurePlayer(playerId);
			gm.NotifyPlayer(playerId, m_sTitle, "It burns all the way in. An hour later the fever's gone. They take a lot of blood and write a lot of notes.");
		}
		else
		{
			profile.m_fInfectionLeft = Math.Max(60, profile.m_fInfectionLeft * 0.5);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("It burns all the way in, and then it burns more. The trial lead won't meet your eyes. You have about %1 minutes now.", Math.Max(1, RZ_Util.RoundToInt(profile.m_fInfectionLeft / 60))));
		}

		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.GetCure().AddResearch(2, "A volunteer took the experimental serum. Whatever happened, the lab learned from it.");
	}
}

//------------------------------------------------------------------------------------------------
//! #20 Mobile Lab: drive Halcyon's lab truck from the lab to a safer town (RZ_VehicleZone). It
//! becomes a field annex there: a second lab where samples are handed in, for three real hours.
//! *Simplified:* the lab itself never moves (it's Halcyon's stronghold, and the quartermaster,
//! leader and territory sit on it); players drive the truck rather than escort an AI crew.
class RZ_Mission_MobileLab : RZ_Mission
{
	protected ref RZ_VehicleZone m_Zone = new RZ_VehicleZone();
	protected IEntity m_Truck;
	protected bool m_bSpawned;
	protected bool m_bArrived;
	protected string m_sDest;

	protected static const float ANNEX_SECONDS = 10800;

	//------------------------------------------------------------------------------------------------
	override bool NeedsVehicle()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.CHAMPION, 45, 40);
		m_Req.m_bNotCured = true;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Supply > Halcyon truck, else Contracts > Bus.
	static ResourceName TruckPrefab(RZ_GameModeComponent gm)
	{
		RZ_SupplySettings ss = gm.GetSupplySettings();
		if (ss && !ss.m_sHalcyonTruck.IsEmpty())
			return ss.m_sHalcyonTruck;

		RZ_ContractSettings cs = gm.GetContractSettings();
		if (cs)
			return cs.m_sBusPrefab;

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!gm.HasLab() || gm.GetLabPrefab().IsEmpty() || TruckPrefab(gm).IsEmpty() || gm.GetCure().IsFinalStand())
			return 0;

		return 0.6;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		vector lab = gm.GetLabPosition();
		int dest = PickSettlementAway(gm, lab, 1000, 4000, 0, 25);
		RZ_Settlement s = gm.GetSettlement(dest);
		if (!s)
			return false;

		m_sDest = s.m_sName;
		m_vPos = LandPointNear(lab, 15, 30);
		m_iSettlement = gm.GetLabSettlement();
		m_Zone.SetZone(LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.4), 35);
		m_sTitle = "Mobile Lab";
		m_sBrief = string.Format("We're setting up a field annex in %1, where it's quieter. The lab truck is outside the lab, loaded. Drive it there in one piece.", m_sDest);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bSpawned)
		{
			if (!RZ_ContractHelp.AnyNear(players, m_vPos, 300))
				return;

			m_bSpawned = true;
			m_Truck = RZ_Util.SpawnPrefab(TruckPrefab(gm), m_vPos + Vector(0, 0.5, 0), Math.RandomFloat(0, 360));
			if (!m_Truck)
			{
				m_eState = RZ_EMissionState.FAILED;
				return;
			}

			RZ_Util.SetVehicleFuel(m_Truck, 0.6);
			m_Zone.SetVehicle(m_Truck);
		}

		if (m_Zone.IsLost())
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		m_vPos = m_Truck.GetOrigin();
		RZ_ContractHelp.AddNear(ids, players, m_vPos, 100, m_aParticipants);
		if (!m_bArrived)
		{
			if (!m_Zone.Check(ids, players))
				return;

			m_bArrived = true;
			RZ_ContractHelp.NotifyNear(gm, m_vPos, 80, m_sTitle, "Park it and get out: the crew unfolds the annex off the back.");
			return;
		}

		if (RZ_VehicleZone.DriverOf(m_Truck, ids, players) < 0)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		vector at = m_Truck.GetOrigin();
		IEntity annex = RZ_Util.SpawnPrefab(gm.GetLabPrefab(), RZ_Mission.LandPointNear(at, 6, 12));
		if (annex)
		{
			gm.GetCure().SetAnnex(annex, ANNEX_SECONDS);
			gm.NotifyAll("Halcyon", string.Format("Halcyon's field annex is open in %1 for the next three hours. Samples can be handed in there.", m_sDest));
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return string.Format("The truck is %1\nTo: %2 (%3)", RZ_Util.DirectionText(from, m_vPos), m_sDest, RZ_Util.DirectionText(from, m_Zone.m_vPos));
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		if (m_Truck && RZ_VehicleZone.DriverOf(m_Truck, ids, players) < 0)
			RZ_Util.DeleteUnit(m_Truck);
	}
}
