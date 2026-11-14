//! The Haven's later contracts (docs/MISSIONS.md, "The Haven": #8, 12, 15, 18, 19, 20; #6 and #14
//! are errands now, docs/ERRANDS.md),
//! built on the mission building blocks (Missions/Blocks/).

//------------------------------------------------------------------------------------------------
//! #8 Harvest Guard: farmers bring in what's left in the fields outside a quiet town. Hold the
//! field for 10 minutes against waves. If the survivor group prefab is set they're really
//! there and must live. ★ Hunter: they stay hidden longer (the first wave comes later).
//! Pays food to the nearest safehouse.
class RZ_Mission_HarvestGuard : RZ_Mission_Hold
{
	protected int m_iWhere;		// the one choice: 0 the farm's own house, 1 the hungriest

	//------------------------------------------------------------------------------------------------
	void RZ_Mission_HarvestGuard()
	{
		m_Story = new RZ_MissionStory("harvest");
		m_Choice = new RZ_OneChoice();
		m_Choice.Set("The sacks are filled. The farmer leans on his spade: \"They're promised to our own house. I hear there's a hungrier one down the road. Your call, you stood the field.\" Answer him at the field.",
			"The farm's own house|The hungriest house", "decide which house gets the food", 0);
		m_Choice.Asset(1, RZ_EFaction.SURVIVORS);
	}

	//------------------------------------------------------------------------------------------------
	override void OnChosen(RZ_GameModeComponent gm, int decider, int option)
	{
		m_iWhere = option;
		string value = "own";
		if (option == 1)
			value = "hungriest";

		RZ_StoryKit.Record(this, "went", RZ_WorldFacts.Town(m_iSettlement), value, m_aParticipants);
	}

	protected ref RZ_UnitWatch m_Farmers = new RZ_UnitWatch();
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
		// The fields of a town the Haven can work: never in a Bastion or Red Hand holding
		m_iSettlement = PickSettlementNotHeldBy(gm, 0, 45, RZ_Factions.Bit(RZ_EFaction.MILITARY) | RZ_Factions.Bit(RZ_EFaction.BANDITS), this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius + 60, s.m_fRadius + 220);
		m_fParticipationRadius = 120;
		m_sTitle = "Harvest Guard";
		m_sDeed = "guarded the harvest at {town}";
		m_sBrief = string.Format("Farmers from the safehouses are bringing in what's left in the fields outside %1: potatoes, mostly, and the last of the cabbages. That's a week of soup at half portions. Ten minutes of work, with their backs to the hedges. Keep the infected off them.", s.m_sName);
		m_sObjective = string.Format("Keep the infected off the farmers in the fields outside %1.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnHoldStarted(RZ_GameModeComponent gm)
	{
		ResourceName prefab = gm.GetSurvivorGroupPrefab();
		if (!prefab.IsEmpty())
			m_Farmers.Spawn(prefab, LandPointNear(m_vPos, 5, 20));

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
	}

	//------------------------------------------------------------------------------------------------
	//! The farmers are watched every tick, guarded or not. A group deletes itself when the last of
	//! them dies: once spawned, gone counts as dead.
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_Farmers.IsDead(dt))
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		super.OnUpdate(gm, dt, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses)
			return;

		int idx = houses.FindNearest(m_vPos);
		if (m_iWhere == 1)
		{
			int hungry = RZ_OneChoice.HungriestHouse(gm);
			if (hungry >= 0 && hungry != idx)
			{
				idx = hungry;
				NotifyParticipants(gm, m_sTitle, "The farm's own house watches the sacks go down the road to somebody else's. The farmer doesn't speak to you again this season.");
			}
		}

		RZ_Safehouse h = houses.Get(idx);
		if (!h)
			return;

		houses.AddFood(idx, FOOD);
		NotifyParticipants(gm, m_sTitle, string.Format("The sacks go to the safehouse in %1: %2 days of food.", h.m_sName, FOOD));
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return string.Format("Harvest Guard (%1): the farmers didn't make it back from the fields. The sacks are lying in the rows, half full, and the house eats less this week.", GetLocationName(gm));
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		m_Farmers.Delete();
	}
}

//------------------------------------------------------------------------------------------------
//! #12 Refugee Column: a group waits at the edge of a bad town. Walk them along the road to a
//! safehouse 0.8-3.5 km away (RZ_Escort). They only walk while someone is with them and hide
//! when infected come close. Three join the safehouse. Big Fame.
//! A story job (key `column`, kept per town): a family, as they said; one of them bitten (a cousin with
//! his sleeve down); or one who owes a crew within 6 km. The grandfather (the talk scene): a nearer
//! house ([Known - Haven], when there is one) or two tins for a carter (they ride: in two minutes
//! they're there). At the door the house asks: all in, or Bastion's post (sheltered, 5 Bastion Favor,
//! the Haven pays nothing); bitten: tell them (two join) or say nothing (three; a lie a visit finds
//! out, 60%); owed: hand him to the crew (two join, 10 Red Hand Favor, +4 Haven Infamy, half pay).
class RZ_Mission_RefugeeColumn : RZ_Mission
{
	protected ref RZ_Escort m_Escort = new RZ_Escort();
	protected int m_iHouse = -1;
	protected bool m_bMet;
	protected string m_sDestName;
	protected int m_iJoin = PEOPLE;
	protected int m_iNearer = -1;
	protected string m_sCrew = "a crew";
	protected string m_sSurname;
	protected bool m_bCart;
	protected float m_fCart;

	protected static const int PEOPLE = 3;
	protected static const int WAY_NEAR = 0;
	protected static const int WAY_CART = 1;
	protected static const float CART_SECONDS = 120;
	protected static const int OPT_IN = 0;
	protected static const int OPT_OTHER = 1;

	//------------------------------------------------------------------------------------------------
	void RZ_Mission_RefugeeColumn()
	{
		m_Story = new RZ_MissionStory("column");
		m_Choice = new RZ_OneChoice();
		m_Job = new RZ_StoryJob();
	}

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
		m_iSettlement = PickEmptySettlement(gm, 20, 85, this);
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
		m_sBrief = string.Format("A family is waiting at the edge of %1 with everything they own on their backs and in a pram. They've been sleeping in a garage since the gates closed. Walk them to %2. They won't move without someone with them, and I don't blame them.", s.m_sName, m_sDestName);
		m_sObjective = string.Format("Walk the family from the edge of %1 to %2.", s.m_sName, m_sDestName);
		if (!m_Escort.Begin(gm, m_vPos))
			return false;

		// A pack that hunts their road at its hour: they ask for the walk past it (and it pays a little more)
		string hunt = RZ_TribeContract.RoadHuntLine(gm, m_vPos, h.m_vPos, 300, RZ_EFaction.SURVIVORS);
		if (!hunt.IsEmpty())
		{
			m_sBrief += " " + hunt;
			m_fRewardScale = 1.15;
		}

		int nearest = gm.GetSafehouses().FindNearest(m_vPos, 3500);
		if (nearest >= 0 && nearest != m_iHouse)
			m_iNearer = nearest;

		Truth(gm);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Truth(RZ_GameModeComponent gm)
	{
		array<string> values = {"family", "bitten", "owed"};
		array<float> weights = {0.5, 0.25, 0};
		RZ_Holding crew = RZ_ErrandCard.HoldingNear(gm, RZ_EFaction.BANDITS, m_vPos, 6000);
		if (crew)
		{
			weights[2] = 0.25;
			m_sCrew = crew.m_sName;
		}

		RZ_Rng rng = new RZ_Rng(RZ_WorldFacts.Get().GetSeed() + m_iSettlement * 131 + 7);
		m_sSurname = RZ_NoteText.Surname(rng);
		string truth = m_Job.Draw(this, RZ_WorldFacts.Town(m_iSettlement), values, weights);
		m_Job.m_vReadAt = m_vPos;
		m_Job.Read("bitten", "nurse,doctor", "The cousin keeps his left sleeve down in this heat. Under it, a bite, a day old at most.");
		m_Job.Read("owed", "excon,raider", string.Format("The thin one won't look up the east road. That's a man who owes %1, and knows it.", m_sCrew));
		m_Job.Read("", "elder", string.Format("Three generations of %1s and one pram. The grandfather is pushing the pram, and the pram is full of spoons.", m_sSurname));

		string reveal = string.Format("The %1s. All of us, and the spoons. Nobody else.", m_sSurname);
		if (truth == "bitten")
			reveal = "\"My nephew. He fell on a dog.\" The nephew nods too quickly.";
		else if (truth == "owed")
			reveal = string.Format("And him. He married in. He owes %1 for the wedding, if you can believe it.", m_sCrew);

		RZ_TalkScene sc = m_Job.Scene("The grandfather", "column.gran", LandPointNear(m_vPos, 4, 7));
		int n0 = sc.Node("An old man sits on a suitcase. \"We packed the good spoons. Priorities.\"");
		int n1 = sc.Node("So where are you walking us, and how slow?");
		sc.Ask(n0, "Who's with you?", n1, reveal, true);
		if (m_iNearer >= 0)
			sc.Way(n1, "There's a nearer house.", RZ_TalkScene.TRUST, RZ_EFaction.SURVIVORS, 1, WAY_NEAR, string.Format("\"Nearer is better.\" He points the pram at %1.", gm.GetSafehouses().Get(m_iNearer).m_sName), "Nearer to what? We were told the other one.");

		sc.Way(n1, "Two tins for a carter", RZ_TalkScene.FOOD, -1, 2, WAY_CART, "A man with a horse cart takes the tins and the grandfather. The rest climb on after him.");
		sc.Service(n1, "Fix the carter's axle", "mechanic", 25, WAY_CART, "The axle holds. The carter takes the grandfather first, then the spoons, then the rest.");

		string labels = "Let them all in";
		if (truth == "bitten")
		{
			labels = "Tell them about the arm|Say nothing";
		}
		else if (truth == "owed")
		{
			labels += "|Hand him to the crew";
			m_Choice.Asset(OPT_OTHER, RZ_EFaction.BANDITS);
		}
		else if (RZ_ErrandCard.HoldingNear(gm, RZ_EFaction.MILITARY, m_vPos, 100000))
		{
			labels += "|Send them on to Bastion's post";
			m_Choice.Asset(OPT_OTHER, RZ_EFaction.MILITARY);
		}

		m_Choice.Set("The house's door opens a crack. \"How many, and are they all well?\"", labels, "decide who goes in", OPT_IN);
	}

	//------------------------------------------------------------------------------------------------
	override vector ChoiceAt()
	{
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(RZ_GameModeComponent.GetInstance(), m_iHouse);
		if (h)
			return h.m_vPos;

		return m_vPos;
	}

	//------------------------------------------------------------------------------------------------
	override void OnStoryWay(RZ_GameModeComponent gm, int playerId, int way)
	{
		if (way == WAY_CART)
		{
			m_bCart = true;
			m_bMet = true;
			return;
		}

		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, m_iNearer);
		if (!h)
			return;

		m_iHouse = m_iNearer;
		m_sDestName = "the safehouse in " + h.m_sName;
		array<vector> route = {h.m_vPos};
		m_Escort.SetRoute(route);
	}

	//------------------------------------------------------------------------------------------------
	override void OnChosen(RZ_GameModeComponent gm, int decider, int option)
	{
		array<int> who = {decider};
		string town = RZ_WorldFacts.Town(m_iSettlement);
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, m_iHouse);
		string truth = m_Job.m_sTruth;
		if (truth == "bitten" && option == OPT_IN)
		{
			m_iJoin = PEOPLE - 1;
			RZ_StoryKit.Record(this, "fate", town, "told", who);
			NotifyParticipants(gm, m_sTitle, "The house takes the nephew out to the field at dusk. The grandfather goes with him and comes back alone.");
			return;
		}

		if (truth == "bitten")
		{
			m_fRewardScale = 0.6;
			RZ_StoryKit.Dirty(gm, this, decider);
			if (h)
				gm.GetFacts().Memory().Lies().Tell(gm, decider, "column.bite", town, RZ_EFaction.SURVIVORS, "", h.m_vPos, RZ_ELieTest.VISIT, 8, "You walked a bitten man through our door and said nothing.");

			RZ_StoryKit.Record(this, "fate", town, "hid", who);
			return;
		}

		if (truth == "owed" && option == OPT_OTHER)
		{
			m_iJoin = PEOPLE - 1;
			m_fRewardScale = 0.5;
			m_Choice.PayWith(gm, this, RZ_EFaction.BANDITS, 10);
			m_Choice.BlameWith(gm, this, RZ_EFaction.SURVIVORS, 4, "handed a refugee to a crew");
			RZ_StoryKit.Record(this, "fate", town, "handed", who);
			NotifyParticipants(gm, m_sTitle, string.Format("Two men from %1 are waiting by the road. He goes with them without a word. The grandfather doesn't watch.", m_sCrew));
			return;
		}

		if (option == OPT_OTHER)
		{
			m_bHasGiver = false;
			m_iJoin = 0;
			m_Choice.PayWith(gm, this, RZ_EFaction.MILITARY, 5);
			RZ_StoryKit.Sheltered(gm, this, PEOPLE);
			RZ_StoryKit.Record(this, "fate", town, "bastion", who);
			return;
		}

		RZ_StoryKit.Record(this, "fate", town, "in", who);
		if (h)
		{
			RZ_StoryKit.BoardThanks(gm, h.m_vPos, "The good spoons are on the long table now. Thank you for walking slowly.", string.Format("The %1s", m_sSurname));
			RZ_StoryKit.LaterWith(gm, this, h.m_sName, "The grandfather is on the bench by the door, polishing a spoon. \"You walk too fast. Sit.\"", h.m_vPos, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_ContractHelp.AddNear(ids, players, m_Escort.m_vPos, 120, m_aParticipants);
		// Two tins for a carter: they ride, and are there in two minutes
		if (m_bCart)
		{
			m_fCart += dt;
			if (m_fCart >= CART_SECONDS && !m_Escort.IsArrived())
				m_Escort.Arrive();

			if (m_Escort.IsArrived())
			{
				m_eState = RZ_EMissionState.SUCCEEDED;
				return;
			}
		}

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
		if (h && m_iJoin > 0)
			gm.GetSafehouses().AddRescued(h.m_vPos, m_iJoin, m_aParticipants);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return string.Format("%1, %2\nTo: %3", GetLocationName(gm), RZ_Util.DirectionText(from, m_vPos), m_sDestName);
	}

	//------------------------------------------------------------------------------------------------
	//! The column where it waits (the rough area until you know it), then where they walk to.
	override RZ_ObjPlace ObjectivePlace(RZ_GameModeComponent gm, int playerId, vector from)
	{
		if (!m_Escort.IsStarted() && IsRoughFor(gm, playerId, from))
			return AreaPlace(gm, playerId, from);

		RZ_ObjPlace at = m_Escort.ObjPlace();
		if (at)
			return at;

		return super.ObjectivePlace(gm, playerId, from);
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
		// A safehouse whose town no other job is using
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, RZ_ContractHelp.PickFreeSafehouse(gm));
		if (!h)
			return false;

		m_vPos = h.m_vPos;
		m_iSettlement = h.m_iSettlement;
		m_fParticipationRadius = 80;
		m_sTitle = "Fortify the Safehouse";
		m_sDeed = "boarded up the house at {town}";
		m_sBrief = string.Format("The safehouse in %1 is boarding up its ground-floor windows before the nights get longer. Every nail is a noise, and the hammering will bring them from all over town. Stand with the people working until the last board is up.", h.m_sName);
		m_sObjective = string.Format("Guard the safehouse in %1 until the last board is up.", h.m_sName);
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
		return string.Format("The safehouse in %1 is boarded up tight, ground floor and back door. The children have drawn the bear on the new planks.", GetLocationName(gm));
	}
}

//------------------------------------------------------------------------------------------------
//! #18 The Informant: wait at a Red Hand dead drop until a seller turns up, pose as a buyer, and
//! take what you heard to any safehouse. Some sellers notice who's asking: a little Red Hand
//! Infamy for the buyer the moment they're spotted (more likely carrying a gun). Only for players the Red Hand
//! is already wary of (Watched or worse); not for Police or Soldiers (they'd be made at once).
//! A story job (key `informant`, kept per town): the names, as they said; bait (he knows what you are
//! and sells you the wrong names); or he sells to Bastion too (a Bastion holding on the map). The
//! seller is the talk scene: buy (the old way: spotted 20%, +30% armed), pay in tins (no standing:
//! never spotted), or [Red Hand kit] "Mira sent me" (a trick: the full list, never spotted). At the
//! house the council asks: the names to the Haven; bait: burn the list (half pay, a record); Bastion:
//! sell Bastion a copy (10 Bastion Favor, a lie a Haven-Known visitor to the house finds out).
class RZ_Mission_TheInformant : RZ_PuzzleMission
{
	protected int m_iHouse = -1;
	protected static const int WAY_BUY = 0;
	protected static const int WAY_TINS = 1;
	protected static const int WAY_KIT = 2;
	protected static const int OPT_GIVE = 0;
	protected static const int OPT_OTHER = 1;

	//------------------------------------------------------------------------------------------------
	void RZ_Mission_TheInformant()
	{
		m_Story = new RZ_MissionStory("informant");
		m_Choice = new RZ_OneChoice();
		m_Job = new RZ_StoryJob();
	}

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
	//! Bought: the names are yours to carry.
	override protected bool ObjFirstDone()
	{
		return m_bBought;
	}

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
		// The Red Hand's drop: outside a town nobody else holds
		m_iSettlement = PickSettlementNotHeldBy(gm, 10, 85, OthersBits(RZ_EFaction.BANDITS), this);
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
		ObjThen(string.Format("Buy the names at the Red Hand's dead drop outside %1.", s.m_sName), "Bring the names to a Haven safehouse.");
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
			Seller(gm);
			RZ_ContractHelp.NotifyNear(gm, m_vPos, 40, m_sTitle, "Someone comes up the path with his hands in his pockets, and stops a few metres off.");
			return;
		}

		if (!m_bBought)
			return;

		IEntity buyer = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iBuyer);
		if (buyer && gm.GetSafehouses().FindNearest(buyer.GetOrigin(), 30) >= 0)
		{
			m_iHouse = gm.GetSafehouses().FindNearest(buyer.GetOrigin(), 30);
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The seller is the talk scene; the truth is drawn when he turns up.
	protected void Seller(RZ_GameModeComponent gm)
	{
		array<string> values = {"names", "bait", "both"};
		array<float> weights = {0.5, 0.3, 0};
		if (RZ_ErrandCard.HoldingNear(gm, RZ_EFaction.MILITARY, m_vPos, 100000))
			weights[2] = 0.2;

		string truth = m_Job.Draw(this, RZ_WorldFacts.Town(m_iSettlement), values, weights);
		m_Job.m_vReadAt = m_vPos;
		m_Job.m_fReadRange = 20;
		m_Job.Read("bait", "excon,raider", "The price is too low. Nobody sells names that cheap unless they're the wrong ones.");
		m_Job.Read("both", "radioop", "A handset in his coat, set to a Bastion channel. He sells twice and calls it business.");

		RZ_TalkScene sc = m_Job.Scene("The seller", "informant.seller", LandPointNear(m_vPos, 3, 6));
		int n0 = sc.Node("A thin man in a good coat, who looks at your hands before your face. \"You're new. What are you buying?\"");
		int n1 = sc.Node("Everyone pays something. The question's what.");
		sc.Way(n0, "Buy what he's selling", RZ_TalkScene.WAY, -1, 0, WAY_BUY, "He counts your money and talks while he counts.");
		sc.Ask(n0, "How do you pay, then?", n1, "Money, tins, a name. Names are the dearest. Don't ask who's buying yours.");
		sc.Way(n1, "Pay in tins", RZ_TalkScene.FOOD, -1, 2, WAY_TINS, "He likes tins. Tins don't ask questions either.");
		sc.Way(n1, "Mira sent me.", RZ_TalkScene.TRICK, RZ_EFaction.BANDITS, 1, WAY_KIT, "\"Why didn't you say?\" He gives you the long list, the one for friends.", "\"Mira's been dead a month.\" He walks off fast.");

		string labels = "Give the Haven the names";
		if (truth == "bait")
			labels += "|Burn the list";
		else if (truth == "both")
		{
			labels += "|Sell Bastion a copy";
			m_Choice.Asset(OPT_OTHER, RZ_EFaction.MILITARY);
		}

		m_Choice.Set("The council's clerk holds out a hand for the list. \"Is it good?\"", labels, "decide where the names go", OPT_GIVE);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStoryWay(RZ_GameModeComponent gm, int playerId, int way)
	{
		bool safe = way != WAY_BUY;
		Buy(gm, playerId, safe);
	}

	//------------------------------------------------------------------------------------------------
	override vector ChoiceAt()
	{
		IEntity buyer = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iBuyer);
		if (buyer)
			return buyer.GetOrigin();

		return m_vPos;
	}

	//------------------------------------------------------------------------------------------------
	override void OnChosen(RZ_GameModeComponent gm, int decider, int option)
	{
		array<int> who = {decider};
		string town = RZ_WorldFacts.Town(m_iSettlement);
		if (option == OPT_OTHER && m_Job.m_sTruth == "bait")
		{
			m_fRewardScale = 0.5;
			RZ_StoryKit.Record(this, "names", town, "burned", who);
			NotifyParticipants(gm, m_sTitle, "The list goes in the stove. The clerk writes 'wrong names, burned' in the book, and underlines it.");
			return;
		}

		if (option == OPT_OTHER && m_Job.m_sTruth == "both")
		{
			m_Choice.PayWith(gm, this, RZ_EFaction.MILITARY, 10);
			RZ_StoryKit.Dirty(gm, this, decider);
			RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, m_iHouse);
			if (h)
				gm.GetFacts().Memory().Lies().Tell(gm, decider, "informant.copy", town, RZ_EFaction.SURVIVORS, "", h.m_vPos, RZ_ELieTest.VISIT, 5, "Bastion knew the crew's runs before we did. Funny, that.");

			RZ_StoryKit.Record(this, "names", town, "copied", who);
			return;
		}

		RZ_StoryKit.Record(this, "names", town, "haven", who);
		RZ_Safehouse home = RZ_ContractHelp.Safehouse(gm, m_iHouse);
		if (home)
			RZ_StoryKit.LaterWith(gm, this, "The council's clerk", "The clerk stops you at the door. \"The Tuesday run didn't come. Your names did that.\"", home.m_vPos);
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

		Buy(gm, playerId, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Bought: the names, and maybe the seller made you. Tins or a good trick never get you made.
	protected void Buy(RZ_GameModeComponent gm, int playerId, bool safe)
	{
		if (m_bBought)
			return;

		m_bBought = true;
		m_iBuyer = playerId;
		m_aParticipants.Clear();
		m_aParticipants.Insert(playerId);

		float chance = SPOT_CHANCE;
		if (safe)
			chance = -1;
		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (user && RZ_Util.IsArmed(user))
			chance += ARMED_CHANCE;

		m_bSpotted = Math.RandomFloat01() < chance;
		string text = string.Format("You pay for a bag of pills you don't want. He talks while he counts: whose crew this is (%1), where the stock comes from, when the next run goes. Take it to any safehouse.", m_sBoss);
		if (m_bSpotted)
			text += "\nAt the end he looks at you a moment too long. He's made you.";

		gm.NotifyPlayer(playerId, m_sTitle, text);

		// Spotted: the crew hears about the buyer now, whether or not the names ever reach the Haven
		if (!m_bSpotted)
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
	protected bool m_bBusLost;
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

		m_iSettlement = PickEmptySettlementAway(gm, h.m_vPos, 1000, 4000, 15, 90, -1, this);
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
		m_sObjective = string.Format("Drive the bus from the depot in %1 to the safehouse in %2.", s.m_sName, h.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		// The depot's bus is there once someone comes near (any other bus, truck or van counts too)
		if (!m_bSpawned && RZ_ContractHelp.AnyNear(players, m_vDepot, 300))
		{
			m_bSpawned = true;
			m_Bus = RZ_Util.SpawnPrefab(m_sPrefab, m_vDepot + Vector(0, 0.5, 0), Math.RandomFloat(0, 360));
			if (m_Bus)
				RZ_Util.SetVehicleFuel(m_Bus, 0.1);

			SpawnGuards(gm, m_vDepot, 4, 10, 40);
		}

		// The depot's bus is gone: any other bus, truck or van still counts, until the clock runs out
		if (m_bSpawned && !m_bArrived && !m_bBusLost && (!m_Bus || RZ_Util.IsDestroyed(m_Bus)))
		{
			m_bBusLost = true;
			NotifyInterested(gm, m_sTitle, "The bus from the depot is wrecked. Any other bus, truck or van will do, if you can find one that runs.");
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
	//! The depot's bus until you sit in a vehicle that counts (or the bus is gone), then the safehouse.
	override RZ_ObjPlace ObjectivePlace(RZ_GameModeComponent gm, int playerId, vector from)
	{
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		bool aboard = me && m_Zone.Matches(RZ_Util.GetVehicleOf(me));
		if (m_bArrived || aboard || m_bBusLost)
			return RZ_ObjPlace.Spot(m_Zone.m_vPos, m_Zone.m_fRadius);

		if (m_Bus && !IsRoughFor(gm, playerId, from))
			return RZ_ObjPlace.Spot(m_Bus.GetOrigin());

		return super.ObjectivePlace(gm, playerId, from);
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
//! The bodies are Missions > Corpse pile, else Puzzles > Puzzle object (look only).
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
		// The bodies need something to mark them (a corpse pile, else the puzzle object)
		if (RZ_ContractHelp.CountSafehouses(gm) == 0 || RZ_ContractHelp.CorpsePilePrefab(gm).IsEmpty())
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickEmptySettlement(gm, 65, 100, this);
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
		m_sObjective = string.Format("Bring the tags of five of the dead of %1 to a Haven safehouse.", s.m_sName);
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
				IEntity e = RZ_Util.SpawnPrefab(RZ_ContractHelp.CorpsePilePrefab(gm), RZ_Util.GroundPos(b), Math.RandomFloat(0, 360));
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
