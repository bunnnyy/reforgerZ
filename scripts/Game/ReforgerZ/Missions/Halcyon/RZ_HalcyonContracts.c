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
		// A part from a town nobody lives in, else any town in reach
		int partTown = PickEmptySettlementAway(gm, lab, 500, 3000, 0, 90, -1, this);
		if (partTown < 0)
			partTown = PickSettlementAway(gm, lab, 500, 3000, 0, 90, -1, this);

		RZ_Settlement s = gm.GetSettlement(partTown);
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
		m_sDeed = "kept the lab's power on";
		m_sBrief = string.Format("The lab's generator is running on fumes and a cracked regulator. When it stops, the freezers warm up, and every sample since Day 1 goes with them. There's a replacement regulator at %1. Bring it back, and a jerrycan of fuel, before the night shift dims the lights again.", m_sPartPlace);
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
		PointWork(RZ_EWork.INSTALL, "Set up the sensor", 15);
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
		m_iSettlement = PickEmptySettlement(gm, 50, 100, this);
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
		m_sDeed = "put the sensors up near {town}";
		m_sBrief = string.Format("We need readings from inside the infection, not from behind our fence. Place sensors in three infested towns, starting with %1: fifteen seconds to set up each one. They hum, and the specials come to see what's humming. That's the point, and it's why we're asking you and not a researcher.", s.m_sName);
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
		m_sDeed = "gave blood when it was asked for";
		m_sBrief = "Every trial dose is measured against blood that's never met the virus, and our clean supply is down to a few tubes. Come to the lab and give some. A needle, a chair and a biscuit after. You'll feel light-headed for a while.";
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
		gm.GetCure().AddResearch(0.5, "Somebody gave blood at the lab today. Every trial dose is measured against clean blood, and the lab had only a few tubes of it left.");
	}
}

//------------------------------------------------------------------------------------------------
//! #9 Cold Chain: an antidote batch for a Haven safehouse's clinic. It spoils 15 minutes after
//! it's picked up (25 for a Nurse, who packs it properly). Fame with Halcyon and the Haven.
//! A story job (key `coldchain`, kept per house): a batch, as they said; short (two doses gone: the
//! manifest says twelve); or a trial batch with no lot number (the house is the trial). The porter at
//! the lab's door (the talk scene): [Known - Halcyon] or two tins and he packs it properly (the
//! Nurse's 25 minutes for anyone). At the house's clinic: hand it over; short: tell them two are
//! missing (+3 Halcyon Infamy, 5 Haven Favor, a Haven asset); trial: tell the nurse (she refuses it:
//! Halcyon pays nothing, 5 Haven Favor, an Independent asset).
class RZ_Mission_ColdChain : RZ_Mission_Delivery
{
	protected float m_fSpoilLeft = -1;
	protected bool m_bWarned;
	protected bool m_bPacked;
	protected int m_iHouse = -1;

	protected static const int WAY_PACK = 0;
	protected static const int OPT_HAND = 0;
	protected static const int OPT_TELL = 1;

	//------------------------------------------------------------------------------------------------
	void RZ_Mission_ColdChain()
	{
		m_Story = new RZ_MissionStory("coldchain");
		m_Choice = new RZ_OneChoice();
		m_Job = new RZ_StoryJob();
	}

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
		m_sDeed = "kept the cold chain cold";
		m_sBrief = string.Format("A batch of antidotes for the clinic at the Haven safehouse in %1, where the bitten are waiting on it. Once it's out of our cold box it's good for fifteen minutes, and after that it's water. Run.", h.m_sName);
		m_iHouse = idx;
		Truth(gm, h);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Truth(RZ_GameModeComponent gm, RZ_Safehouse h)
	{
		array<string> values = {"batch", "short", "trial"};
		array<float> weights = {0.5, 0.3, 0.2};
		string truth = m_Job.Draw(this, "house:" + m_iHouse.ToString(), values, weights);
		m_Job.m_vReadAt = m_vPos;
		m_Job.m_fReadRange = 15;
		m_Job.Read("short", "labtech", "Twelve slots in the cold box and ten vials in them. The manifest says twelve.");
		m_Job.Read("trial", "labtech", "No batch number. Lot codes like these are for trials, not clinics.");
		m_Job.Read("trial", "nurse,doctor", "The dose on the label's wrong for a clinic. It's a trial dose, and someone means to watch what it does.");

		string reveal = "Antidotes, they tell me. I carry boxes. I'm not paid to read them.";
		if (truth == "short")
			reveal = "Ten, where it says twelve. The tech who packed it has a sister in a house. I'm only saying.";
		else if (truth == "trial")
			reveal = "No number on it. The ones with no number come back with forms.";

		RZ_TalkScene sc = m_Job.Scene("The porter", "coldchain.porter", LandPointNear(m_vPos, 3, 6));
		int n0 = sc.Node("A porter with frost on his gloves. \"Fifteen minutes, they say. I've seen it done in twelve. I've also seen it not done.\"");
		int n1 = sc.Node("Ice and newspaper. It's not science. Well. It's a bit of science.");
		sc.Ask(n0, "What's in the box?", n1, reveal, true);
		sc.Way(n1, "Pack it properly.", RZ_TalkScene.TRUST, RZ_EFaction.INSTITUTE, 1, WAY_PACK, "He packs it in ice and newspaper, tight. \"Twenty-five minutes. Don't drop it.\"", "Pack it yourself. I'm on my break.");
		sc.Way(n1, "Two tins for his ice", RZ_TalkScene.FOOD, -1, 2, WAY_PACK, "The tins buy his good ice, the stuff he keeps for himself.");

		string labels = "Hand them over";
		if (truth == "short")
		{
			labels += "|Tell them two are missing";
			m_Choice.Asset(OPT_TELL, RZ_EFaction.SURVIVORS);
		}
		else if (truth == "trial")
		{
			labels += "|Tell the nurse it's a trial";
			m_Choice.Asset(OPT_TELL, RZ_CureFinale.INDEP);
		}

		m_Choice.Set(string.Format("The nurse at %1 holds out her hands for the box. \"All of it?\"", h.m_sName), labels, "decide what the nurse hears", OPT_HAND);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStoryWay(RZ_GameModeComponent gm, int playerId, int way)
	{
		m_bPacked = true;
		if (m_fSpoilLeft >= 0)
			m_fSpoilLeft = Math.Max(m_fSpoilLeft, NURSE_SECONDS - (SPOIL_SECONDS - m_fSpoilLeft));
	}

	//------------------------------------------------------------------------------------------------
	override vector ChoiceAt()
	{
		return GetDropPos(0);
	}

	//------------------------------------------------------------------------------------------------
	override void OnChosen(RZ_GameModeComponent gm, int decider, int option)
	{
		array<int> who = {decider};
		string subject = "house:" + m_iHouse.ToString();
		string truth = m_Job.m_sTruth;
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, m_iHouse);
		if (option == OPT_TELL && truth == "short")
		{
			m_Choice.BlameWith(gm, this, RZ_EFaction.INSTITUTE, 3, "told a house its doses were short");
			m_Choice.PayWith(gm, this, RZ_EFaction.SURVIVORS, 5);
			RZ_StoryKit.Record(this, "batch", subject, "short_told", who);
			if (h)
				RZ_StoryKit.LaterWith(gm, this, h.m_sName, "The house's nurse catches your sleeve. \"Two came back. In an envelope, no note. Somebody's sister felt bad.\"", h.m_vPos, true);

			return;
		}

		if (option == OPT_TELL && truth == "trial")
		{
			m_bHasGiver = false;
			m_Choice.PayWith(gm, this, RZ_EFaction.SURVIVORS, 5);
			RZ_StoryKit.Record(this, "batch", subject, "refused", who);
			NotifyParticipants(gm, m_sTitle, "The nurse reads the label, puts the box down and doesn't open it. \"Tell them we're not their forms.\" Halcyon pays nothing for a batch nobody took.");
			return;
		}

		RZ_StoryKit.Record(this, "batch", subject, truth, who);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnCarrierChanged(RZ_GameModeComponent gm, int previous, int carrier)
	{
		super.OnCarrierChanged(gm, previous, carrier);
		if (carrier <= 0 || m_fSpoilLeft >= 0)
			return;

		m_fSpoilLeft = SPOIL_SECONDS;
		string text = "The clock's running: fifteen minutes.";
		if (HasBonus(gm, carrier) || m_bPacked)
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
		if (m_bHasGiver)
			gm.AddFame(playerId, RZ_EFaction.SURVIVORS, HAVEN_FAME, false);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Cold Chain: the batch warmed up on the road and spoiled. The bitten at the clinic waited for nothing, and the lab has to make it again from the same thin stock.";
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
//! A story job (key `scientist`, kept per town): readings, as they said; she's looking for her brother
//! (a house within 5 km of the last site); or mapping the drones' times to the line. The lab's driver
//! (the talk scene): [Known - Halcyon] or two tins and he takes the last site himself (two sites, no
//! gun, no standing). Done, she asks: brother: wait for her (research +2) or leave her with him (half
//! pay, no research, an Independent asset); the line: hand the readings in, burn the drone times
//! (research +1, nobody), or copy them for the crew (10 Red Hand Favor, +5 Halcyon Infamy); readings:
//! give the water readings to the house too (5 Haven Favor, research +1).
class RZ_Mission_EscortTheScientist : RZ_PuzzleMission
{
	protected int m_iResearch = 2;
	protected int m_iHouse = -1;
	protected string m_sBrother;

	protected static const int WAY_SHORT = 0;
	protected static const int OPT_HAND = 0;
	protected static const int OPT_SECOND = 1;
	protected static const int OPT_THIRD = 2;

	//------------------------------------------------------------------------------------------------
	void RZ_Mission_EscortTheScientist()
	{
		m_Story = new RZ_MissionStory("scientist");
		m_Choice = new RZ_OneChoice();
		m_Job = new RZ_StoryJob();
	}

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
		m_sDeed = "walked a researcher through {town}";
		m_sBrief = string.Format("One of our field researchers needs readings from %1, %2 and %3, and they can't be taken from behind a fence. Collect her at the lab and walk her round all three. She'll be watching her instruments, not the hedges. You watch the hedges.", m_aSiteNames[0], m_aSiteNames[1], m_aSiteNames[2]);
		if (!m_Escort.Begin(gm, m_vPos))
			return false;

		Truth(gm);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Truth(RZ_GameModeComponent gm)
	{
		array<string> values = {"readings", "brother", "line"};
		array<float> weights = {0.5, 0, 0.2};
		vector last = m_aSites[m_aSites.Count() - 1];
		if (gm.GetSafehouses())
			m_iHouse = gm.GetSafehouses().FindNearest(last, 5000);

		if (m_iHouse >= 0)
			weights[1] = 0.3;

		m_sBrother = RZ_NoteText.FirstName(m_Rng);
		string truth = m_Job.Draw(this, RZ_WorldFacts.Town(m_iSettlement), values, weights);
		m_Job.m_vReadAt = m_vPos;
		m_Job.Read("line", "radioop", "Her instrument case has a scanner tuned to the drones' band. Those aren't soil readings.");
		m_Job.Read("brother", "elder", string.Format("She asks every house you pass about a man called %1, with her nose. She's looking for someone.", m_sBrother));
		m_Job.Read("readings", "labtech", "Soil and water: ordinary readings, carefully taken, and labelled in a hand that's done it a thousand times.");

		string reveal = "Best we've got. She once took readings through a horde. They were good readings.";
		if (truth == "brother")
			reveal = string.Format("Her brother %1 went to a house out that way in the spring. She picks her sites.", m_sBrother);
		else if (truth == "line")
			reveal = "She times the drones. Every site's on a drone's turn. Don't tell her I said.";

		RZ_TalkScene sc = m_Job.Scene("The lab's driver", "scientist.driver", LandPointNear(m_vPos, 6, 12));
		int n0 = sc.Node("A driver leans on a van with no wheels. \"She'll walk you off your feet. Then she'll walk you off the map.\"");
		int n1 = sc.Node("I know the last site. I could take it myself, for a consideration.");
		sc.Ask(n0, "Who is she?", n1, reveal, true);
		sc.Way(n1, "Take the last one yourself.", RZ_TalkScene.TRUST, RZ_EFaction.INSTITUTE, 1, WAY_SHORT, "\"Fine. Two for you, one for me.\" He picks up a case and walks off whistling.", "For you? No.");
		sc.Way(n1, "Two tins for the last site", RZ_TalkScene.FOOD, -1, 2, WAY_SHORT, "He pockets the tins. \"Two sites for you, then. I'll do the far one.\"");

		string labels = "Hand the readings in";
		if (truth == "brother")
		{
			labels = "Wait for her|Leave her with him";
			m_Choice.Asset(OPT_SECOND, RZ_CureFinale.INDEP);
		}
		else if (truth == "line")
		{
			labels += "|Burn the drone times|Copy them for the crew";
			m_Choice.Asset(OPT_THIRD, RZ_EFaction.BANDITS);
		}
		else if (m_iHouse >= 0)
		{
			labels += "|Give the house the water readings";
			m_Choice.Asset(OPT_SECOND, RZ_EFaction.SURVIVORS);
		}

		m_Choice.Set("She snaps the case shut for the last time and looks at you. \"So. What happens to these?\"", labels, "decide what happens to the readings", OPT_HAND);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStoryWay(RZ_GameModeComponent gm, int playerId, int way)
	{
		// Never the site she's at or past: the driver takes one still ahead
		if (m_aSites.Count() > 2 && m_iSite < m_aSites.Count() - 1)
		{
			m_aSites.Remove(m_aSites.Count() - 1);
			m_aSiteNames.Remove(m_aSiteNames.Count() - 1);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnChosen(RZ_GameModeComponent gm, int decider, int option)
	{
		array<int> who = {decider};
		string town = RZ_WorldFacts.Town(m_iSettlement);
		string truth = m_Job.m_sTruth;
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, m_iHouse);
		if (truth == "brother" && option == OPT_SECOND)
		{
			m_iResearch = 0;
			m_fRewardScale = 0.5;
			if (h)
			{
				gm.GetSafehouses().AddPopulationAt(m_iHouse, 1);
				RZ_StoryKit.LaterWith(gm, this, h.m_sName, string.Format("A woman with instrument cases is fixing the house's radio with %1. They argue like twins. They aren't.", m_sBrother), h.m_vPos, true);
			}

			RZ_StoryKit.Record(this, "fate", town, "stayed", who);
			return;
		}

		if (truth == "line" && option == OPT_SECOND)
		{
			m_iResearch = 1;
			m_bHasGiver = false;
			RZ_StoryKit.Record(this, "fate", town, "burned", who);
			NotifyParticipants(gm, m_sTitle, "The drone times go in a fire. She watches them burn and says nothing. Halcyon pays nothing for ash.");
			return;
		}

		if (truth == "line" && option == OPT_THIRD)
		{
			m_Choice.PayWith(gm, this, RZ_EFaction.BANDITS, 10);
			m_Choice.BlameWith(gm, this, RZ_EFaction.INSTITUTE, 5, "sold the drones' times");
			RZ_StoryKit.Record(this, "fate", town, "crew", who);
			return;
		}

		if (option == OPT_SECOND && truth == "readings")
		{
			m_iResearch = 1;
			m_Choice.PayWith(gm, this, RZ_EFaction.SURVIVORS, 5);
			RZ_StoryKit.Record(this, "fate", town, "house", who);
			return;
		}

		RZ_StoryKit.Record(this, "fate", town, "lab", who);
	}

	//------------------------------------------------------------------------------------------------
	override vector ChoiceAt()
	{
		return m_vPos;
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
		if (m_iResearch > 0)
			gm.GetCure().AddResearch(m_iResearch, "A field researcher came back to the lab on foot, with three sites' readings in a numbered notebook.");
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
		m_iSettlement = PickEmptySettlement(gm, 50, 100, this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.6);
		if (!SpawnDrop(gm, RZ_EFaction.INSTITUTE, RZ_HalcyonContractHelp.DropPos(gm)))
			return false;

		m_sTitle = "Specimen from the Nest";
		m_sBrief = string.Format("There's a hive in %1, where they lie up together through the day. Living tissue from inside it tells us more about what they're becoming than any body on the slab. Cut a sample and bring it to the lab. Don't burn it, whatever the Haven tells you. Burnt tissue is just ash.", s.m_sName);
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

		// With work sites: Cut the tissue, a hold at the hive's edge; the hive hears it all the while
		RZ_WorkSites sites = RZ_WorkSites.Get();
		string key = string.Format("mission.%1.tissue", m_iId);
		if (RZ_WorkSites.Available() && sites)
		{
			if (!sites.Has(key))
				sites.Open(key, RZ_EWork.HANDS, "Cut the tissue", 15, LandPointNear(m_vPos, 1.5, 2.5));

			if (CountPlayersNear(players, m_vPos, 6) > 0)
				gm.GetHeat().AddHeat(m_vPos, 1 * dt);

			if (sites.IsDone(key) && SpawnItem(gm, m_vPos))
			{
				sites.Close(key);
				RZ_ContractHelp.NotifyNear(gm, m_vPos, 40, m_sTitle, "The tissue comes away wet and still twitching. Into the jar. The hive shudders; something inside heard you.");
			}

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
		gm.GetCure().AddResearch(4, "Living tissue from a hive reached the lab in a sealed jar. The night shift has it under the microscope.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		RZ_Util.DeleteUnit(m_Nest);
		if (RZ_WorkSites.Get())
			RZ_WorkSites.Get().Close(string.Format("mission.%1.tissue", m_iId));
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
		m_sBrief = string.Format("Our prototype drone dropped out of the sky near %1: the one we built from parts to watch the packs without walking out among them. Its last ping was grid %2, give or take 250 m. The airframe doesn't matter. The core holds every flight it made. Bring it back.", s.m_sName, RZ_SettlementScanner.GridReference(m_vPos));
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
		gm.GetCure().AddResearch(1, "The core of Halcyon's lost drone reached the lab, with every flight it made over the packs still on it.");
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
//! a body names the next town; the fourth is the canon clinic (origin.clinic), where the first
//! patient (origin.patient) was brought: search the records for 45 s while
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
		// Not while The Three Weeks follows the same records
		m_Req.NotWhileQuest("weeks");
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
		if (!PickChain(gm))
			return false;

		RZ_Settlement s = gm.GetSettlement(m_aTowns[0]);
		m_iSettlement = m_aTowns[0];
		RZ_WorldFacts facts = gm.GetFacts();
		string patient = facts.CanonText("origin.patient");
		string siteTown;
		RZ_Settlement site = gm.GetSettlement(facts.CanonSettlement("origin.site.town"));
		if (site)
			siteTown = site.m_sName;

		int toldDay = facts.CanonInt("origin.told", 3, 24);
		int toldMonth = facts.CanonInt("origin.told.month", 1, 12);

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
				st.m_iClue = AddClue("The clinic records", ClinicText(patient, siteTown, toldDay, toldMonth));
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
		m_sBrief = string.Format("We think we can trace the outbreak back to the first case, and so could anyone else who went looking. It starts in %1: a body with a hospital bracelet. Follow where it leads, and bring what you find to us before you show it to anyone.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Four towns, the last the canon clinic (origin.clinic: the same clinic all season, whoever
	//! follows the trail). The three before it lead toward it. Without a canon clinic (no town on
	//! the map), a chain from an infested town as before.
	protected bool PickChain(RZ_GameModeComponent gm)
	{
		int clinic = gm.GetFacts().CanonSettlement("origin.clinic");
		RZ_Settlement c = gm.GetSettlement(clinic);
		if (c && !IsTownBusy(gm, clinic))
		{
			array<int> back = {clinic};
			if (RZ_StepChain.PickTowns(gm, c.m_vCenter, 4, 600, 2500, 0, 100, back))
			{
				for (int i = back.Count() - 1; i >= 0; i--)
				{
					m_aTowns.Insert(back[i]);
				}

				return true;
			}
		}

		m_aTowns.Clear();
		int start = PickSettlement(gm, 30, 100, this);
		RZ_Settlement s = gm.GetSettlement(start);
		if (!s)
			return false;

		m_aTowns.Insert(start);
		return RZ_StepChain.PickTowns(gm, s.m_vCenter, 4, 600, 2500, 0, 100, m_aTowns);
	}

	//------------------------------------------------------------------------------------------------
	//! The first admission, with the canon facts: the patient's surname, the town below the site,
	//! the day the Institute was told (docs/LORE.md, "Halcyon").
	protected string ClinicText(string patient, string siteTown, int toldDay, int toldMonth)
	{
		string from = "a closed site in the hills";
		if (!siteTown.IsEmpty())
			from = "a closed site in the hills above " + siteTown;

		string who = "a man";
		if (!patient.IsEmpty())
			who = "a man called " + patient;

		return string.Format("A filing cabinet in a ransacked clinic. The first admission is three weeks before the gates closed: %1, brought in from %2, with a fever nobody could name. Someone wrote 'notify the Institute' in the margin and underlined it twice. Under it, in another pen: 'Told %3.%4.'", who, from, toldDay, toldMonth);
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
		gm.GetCure().AddResearch(6, "Halcyon has traced the outbreak back to its first case, a hospital bracelet at a time. Knowing where it started tells the lab what it started as.");

		// The clinic records are out, "notify the Institute" and all: The Three Weeks can begin
		RZ_Quest_ThreeWeeks.NoteClinicRecords(gm);
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
//! sedated, and carry the case to the lab before it wakes (RZ_LiveCapture). Huge research. With the
//! packs on it's one of a pack's guard (its young if it has under 2 guard), at the edge of its lair;
//! caught, it leaves the pack. Else a special in an infested town.
class RZ_Mission_LiveCapture : RZ_PuzzleMission
{
	protected ref RZ_LiveCapture m_Capture = new RZ_LiveCapture();
	protected IEntity m_Drop;
	protected int m_iPack = -1;
	protected RZ_ETribePart m_ePackKind;
	protected bool m_bPackTaken;

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
		if (SetupFromPack(gm))
			return true;

		m_iSettlement = PickEmptySettlement(gm, 40, 100, this);
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
	//! With the packs on: one of a free pack's guard (or its young), its own prefab, at its lair's edge.
	protected bool SetupFromPack(RZ_GameModeComponent gm)
	{
		RZ_Tribes tribes = RZ_TribeContract.Tribes(gm);
		if (!tribes)
			return false;

		RZ_TribePeople people = tribes.GetPeople();
		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			string subject = RZ_TribeContract.Claim(t.m_iId);
			if (t.IsRover() || !IsSubjectFree(gm, subject))
				continue;

			RZ_ETribePart kind = RZ_ETribePart.GUARD;
			ResourceName prefab = people.GuardPrefab(t);
			string what = "one of the big ones that stays by its index";
			if (t.Count(RZ_ETribePart.GUARD) < 2)
			{
				kind = RZ_ETribePart.YOUNG;
				prefab = people.YoungPrefab(t);
				what = "one of its recent conversions";
				if (t.Count(RZ_ETribePart.YOUNG) < 1)
					continue;
			}

			if (prefab.IsEmpty() || !ClaimSubject(gm, subject))
				continue;

			m_Drop = RZ_HalcyonContractHelp.SpawnLabDrop(gm);
			if (!m_Drop)
				return false;

			TrackProp(m_Drop);
			vector lair = LandPointNear(t.m_vLair, 20, 60);
			vector cage = LandPointNear(lair, 60, 120);
			m_iPack = t.m_iId;
			m_ePackKind = kind;
			m_vPos = cage;
			m_sTitle = "Live Capture";
			m_sBrief = string.Format("We need one of %1 alive: %2. There's a cage trap set near where they lie up. Get it to follow you in and drop the door, or bring it down without killing it and tie it. Keep it calm until the sedative takes, then bring it to the lab before it wakes. Dead is no use to us.", people.NameAt(t, RZ_EFaction.INSTITUTE), what);
			if (!m_Capture.Begin(gm, m_iId, -1, cage, lair, 0))
				return false;

			m_Capture.UsePrefab(prefab);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_Capture.Update(gm, dt, ids, players);
		if (m_Capture.m_eState == RZ_ECaptureState.CAUGHT)
		{
			RZ_ContractHelp.AddNear(ids, players, m_Capture.m_vHeld, 30, m_aParticipants);
			// One of a pack's: it leaves the pack
			RZ_Tribes tribes = RZ_TribeContract.Tribes(gm);
			if (m_iPack >= 0 && !m_bPackTaken && tribes && tribes.Find(m_iPack))
			{
				m_bPackTaken = true;
				tribes.GetLife().TakeMembers(tribes.Find(m_iPack), m_ePackKind, 1);
			}
		}

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
		gm.GetCure().AddResearch(5, "A live special is in the lab's isolation room, sedated and caged. For the first time Halcyon can watch the infection work, hour by hour, instead of reading it off the dead.");
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
		m_iTalk = AddClue("The trial lead", "It's not ready. It might never be. But you're dying anyway, and we're out of time. Your choice.");
		SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPos, 0, "", "Take the serum", m_iTalk, "Talk to the trial lead");
		m_sTitle = "Serum Trial";
		m_sBrief = "You're bitten. We have a trial dose: eight in ten walk out, and we write down what happens to the other two. Anyone bitten may volunteer once, for nothing. Come to the lab if you want it, and come before the fever does.";
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
		gm.GetCure().AddResearch(2, "A bitten volunteer took the experimental serum at the lab. Whatever happened next is in a numbered notebook now, and the lab learned from it.");
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
		// The annex goes where it's quiet: never a Bastion or Red Hand town
		int dest = PickSettlementAwayNotHeldBy(gm, lab, 1000, 4000, 0, 25, RZ_Factions.Bit(RZ_EFaction.MILITARY) | RZ_Factions.Bit(RZ_EFaction.BANDITS), -1, this);
		RZ_Settlement s = gm.GetSettlement(dest);
		if (!s)
			return false;

		m_sDest = s.m_sName;
		m_vPos = LandPointNear(lab, 15, 30);
		m_iSettlement = gm.GetLabSettlement();
		m_Zone.SetZone(LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.4), 35);
		m_sTitle = "Mobile Lab";
		m_sBrief = string.Format("We're setting up a field annex in %1, where it's quieter: sensors, a cold box and a radio, like the other field stations. The lab truck is outside the lab with all of it loaded. Drive it there in one piece. The centrifuge in the back is worth more than the truck.", m_sDest);
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
