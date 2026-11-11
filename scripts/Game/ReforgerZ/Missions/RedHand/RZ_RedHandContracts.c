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
		m_sDeed = "brought in a car from {town}";
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
//! #10 Protection Racket (story key `racket`): "collect" from a Haven safehouse. The truth, kept per
//! house: just late, paid twice (another crew's receipt is pinned in the stores) or the storekeeper's
//! cut (her daughter in the doorway tells it). At the storekeeper: Ask; Persuade (Trust with the Haven
//! 2: two food, 75% pay); Press (Fear 2: the full three food and +5 Haven Infamy, +8 when short;
//! refused, her people come out, or the door shuts); Cover it yourself (4 food or 20 Red Hand Favor:
//! half pay, and a lie the next collection here finds out); the receipt (full pay, the house pays
//! nothing); her cut (keep her secret: one food; or tell the council: three food and she's put
//! out); Side with the house (after the hold: +10 Red Hand Infamy, nobody pays). Not for Elders.
//! Time up: the crew collects anyway, and the house's cook has The Pot for the participants first.
class RZ_Mission_ProtectionRacket : RZ_PuzzleMission
{
	protected int m_iHouse = -1;
	protected string m_sHouse;
	protected string m_sTruth;
	protected bool m_bLeaned;
	protected int m_iCollector = -1;
	protected IEntity m_Locals;
	protected bool m_bFight;		// the group deletes itself when its last member dies: track the fight here
	protected float m_fFightAge;
	protected bool m_bSeenLocals;
	protected bool m_bCutFound;
	protected bool m_bReceipt;
	protected bool m_bDone;
	protected string m_sHow;
	protected int m_iExtraInfamy;
	protected int m_iFood = FOOD;
	protected int m_iTalk = -1;
	protected int m_iReceiptClue = -1;
	protected int m_iDaughterClue = -1;
	protected RZ_WorldRecord m_SpokeFor;		// "racket.speak_for" at this house: who promised (weak; the memory owns it)
	protected ref array<int> m_aActs = {};
	protected ref RZ_Ways m_Ways = new RZ_Ways("Protection Racket");

	protected static const int FOOD = 3;
	protected static const int COVER_FOOD = 4;
	protected static const int COVER_FAVOR = 20;
	protected static const int PRESS_INFAMY = 5;
	protected static const int SHORT_INFAMY = 3;		// on top of the threat's: 8 in all
	protected static const int SIDE_INFAMY = 10;
	protected static const int COVER_FOUND_INFAMY = 5;

	// What each keeper option does (the labels change with what's been found)
	protected static const int ACT_ASK = 0;
	protected static const int ACT_PERSUADE = 1;
	protected static const int ACT_PRESS = 2;
	protected static const int ACT_COVER = 3;
	protected static const int ACT_RECEIPT = 4;
	protected static const int ACT_SECRET = 5;
	protected static const int ACT_COUNCIL = 6;
	protected static const int ACT_SIDE = 7;
	protected static const int ACT_LEAVE = 8;

	//------------------------------------------------------------------------------------------------
	void RZ_Mission_ProtectionRacket()
	{
		m_Story = new RZ_MissionStory("racket");
	}

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
		// Never a house that pays by arrangement (A Quiet Arrangement seen through in its town)
		RZ_Safehouse h;
		for (int tries = 0; tries < 4; tries++)
		{
			m_iHouse = gm.GetSafehouses().PickRandom();
			h = RZ_ContractHelp.Safehouse(gm, m_iHouse);
			if (h && !PaysByArrangement(h))
				break;

			h = null;
		}

		if (!h)
			return false;

		m_sHouse = h.m_sName;
		m_vPos = LandPointNear(h.m_vPos, 5, 12);
		m_iSettlement = h.m_iSettlement;

		// The last collection here: anyone who covered it is found out now
		RZ_WorldFacts.Get().Memory().Lies().Test(gm, "racket.covered", HouseKey());

		// The truth, kept per house: the cut never while The Ledger runs here
		array<string> values = {"just_late", "paid_twice", "cut"};
		array<float> weights = {0.45, 0.25, 0.2};
		if (gm.GetMissions().IsTypeRunning(RZ_Mission_TheLedger))
			weights[2] = 0;

		// Paid twice needs another crew boss within 6 km to have taken the first payment
		RZ_Leaders leaders = gm.GetLeaders();
		if (!leaders || !leaders.FindNearest(RZ_EFaction.BANDITS, h.m_vPos, 6000))
			weights[1] = 0;

		m_sTruth = m_Story.Truth(HouseKey(), values, weights);

		m_iTalk = AddClue("The storekeeper", "\"I know who sent you. We've nothing to spare. We've people to feed.\"");
		SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPos, 0, "", Labels(), m_iTalk, "Talk to the storekeeper");
		if (m_sTruth == "paid_twice")
		{
			m_iReceiptClue = AddClue("A receipt", "Pinned up in the stores, in a crew's hand: \"Paid in full. Protected till the month's end.\" A red hand drawn under it, but not ours.");
			SpawnClue(gm, RZ_EPuzzleProp.CLUE, LandPointNear(h.m_vPos, 3, 8), m_iReceiptClue, "Read the paper on the wall");
		}

		if (m_sTruth == "cut")
		{
			m_iDaughterClue = AddClue("A girl in the doorway", "She's about nine and very serious. \"Mum says the tins are for my medicine. The man at the pit has antibiotics.\"");
			SpawnClue(gm, RZ_EPuzzleProp.CIVILIAN, LandPointNear(h.m_vPos, 6, 14), m_iDaughterClue, "Talk to the girl");
		}

		m_sTitle = "Protection Racket";
		m_sDeed = "collected in {town}";
		m_sBrief = string.Format("The safehouse in %1 is late. Everyone's late this week. Go and remind the storekeeper what our protection is worth.", h.m_sName);

		// Someone promised to speak for the house at this collection
		m_SpokeFor = RZ_WorldFacts.Get().Memory().FindRecord("racket.speak_for", HouseKey());
		if (m_SpokeFor)
			m_sBrief += " The storekeeper will remind you that somebody promised to speak for the house this time.";

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A Quiet Arrangement was seen through in the house's town: it pays the crews by arrangement.
	protected bool PaysByArrangement(RZ_Safehouse h)
	{
		RZ_WorldRecord r = RZ_WorldFacts.Get().Memory().FindRecord("t0.quiet_arrangement", RZ_WorldFacts.Town(h.m_iSettlement));
		return r && r.value == "done";
	}

	//------------------------------------------------------------------------------------------------
	protected string HouseKey()
	{
		return "house:" + m_iHouse.ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! The storekeeper's options, by what's been found; m_aActs maps each to what it does.
	protected string Labels()
	{
		m_aActs.Clear();
		array<string> labels = {};
		if (m_bCutFound)
		{
			labels.Insert("Keep her secret");
			m_aActs.Insert(ACT_SECRET);
			labels.Insert("Tell the council");
			m_aActs.Insert(ACT_COUNCIL);
		}
		else
		{
			labels.Insert("Ask");
			m_aActs.Insert(ACT_ASK);
			labels.Insert("[Trusted · Haven] Persuade");
			m_aActs.Insert(ACT_PERSUADE);
			labels.Insert("[Feared] Press");
			m_aActs.Insert(ACT_PRESS);
			labels.Insert("Cover it yourself [4 food | 20 Red Hand Favor]");
			m_aActs.Insert(ACT_COVER);
			if (m_bReceipt)
			{
				labels.Insert("[The receipt] Send it to the boss");
				m_aActs.Insert(ACT_RECEIPT);
			}
		}

		labels.Insert("Side with the house");
		m_aActs.Insert(ACT_SIDE);
		labels.Insert("Leave");
		m_aActs.Insert(ACT_LEAVE);

		string text = "";
		foreach (int i, string l : labels)
		{
			if (i > 0)
				text += "|";

			text += l;
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		if (idx == m_iReceiptClue && !m_bReceipt)
		{
			m_bReceipt = true;
			m_Story.Reveal(gm, this, HouseKey(), playerId, "receipt", "The house already paid another crew for the same weeks. Somebody wants a fight between crews.");
			SetObjectLabels(0, "", Labels());
		}

		if (idx == m_iDaughterClue && !m_bCutFound)
		{
			m_bCutFound = true;
			m_Story.Reveal(gm, this, HouseKey(), playerId, "daughter", "The house did pay. The storekeeper kept tins back to trade for her daughter's antibiotics, and the council doesn't know.");
			SetObjectLabels(0, "", Labels());
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0 || m_bLeaned || m_bDone || option < 0 || option >= m_aActs.Count())
			return;

		int act = m_aActs[option];
		m_Story.m_Deciding.Step(gm, playerId);
		if (act == ACT_LEAVE)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You walk away. Whoever else is on this can sort it out.");
			return;
		}

		if (act == ACT_ASK)
		{
			Ask(gm, playerId);
			return;
		}

		// Everything else decides the job: the contractor only
		if (!RZ_StoryKit.MayDecide(gm, this, playerId))
			return;

		if (act == ACT_SIDE)
		{
			m_Story.m_Deciding.StartHold(gm, this, playerId, ACT_SIDE, "warn the council and let them hide the stores");
			return;
		}

		m_iCollector = playerId;
		if (act == ACT_PERSUADE)
			Persuade(gm, playerId);
		else if (act == ACT_PRESS)
			Press(gm, playerId);
		else if (act == ACT_COVER)
			Cover(gm, playerId, source);
		else if (act == ACT_RECEIPT)
			Receipt(gm, playerId);
		else if (act == ACT_SECRET)
			Collect(gm, playerId, 1, 1, "secret", "She pays out of her hidden tins, one at a time, and doesn't thank you. Her daughter watches from the stairs.");
		else if (act == ACT_COUNCIL)
			Council(gm, playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void Ask(RZ_GameModeComponent gm, int playerId)
	{
		if (m_sTruth == "paid_twice")
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"We paid. Ask Janda at the pit. The receipt's on the wall in the stores, if you can read.\"");
			RZ_StoryKit.Read(gm, this, playerId, "excon", "The red hand under the receipt is Janda's crew's. You'd know it anywhere.");
			return;
		}

		if (m_sTruth == "cut")
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"We've nothing. That's all. Nothing.\" She doesn't look at the stairs.");
			RZ_StoryKit.Read(gm, this, playerId, "police", "She's lying, and badly. Whatever's missing, she knows where it went.");
			RZ_StoryKit.Read(gm, this, playerId, "nurse", "Empty antibiotic boxes in the bin by the door. Children's dose.");
			return;
		}

		gm.NotifyPlayer(playerId, m_sTitle, string.Format("\"%1 is down to the bone. Everyone's late this week. Count the shelves yourself.\" You do. She's right.", m_sHouse));
	}

	//------------------------------------------------------------------------------------------------
	protected void Persuade(RZ_GameModeComponent gm, int playerId)
	{
		// Whoever promised to speak for the house last time is heard: one step easier
		int difficulty = 2;
		if (m_SpokeFor && m_SpokeFor.idents.Contains(gm.GetProfile(playerId).m_sIdentity))
			difficulty = 1;

		RZ_ECheck result = m_Ways.TryTrust(gm, playerId, "keeper", RZ_EFaction.SURVIVORS, difficulty, "nurse,police");
		if (result == RZ_ECheck.REFUSED)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"Don't you talk to me like a friend.\" She won't hear it from you again.");
			return;
		}

		if (result == RZ_ECheck.SHORT)
		{
			RZ_Deeds.AddLifeRecord(gm.GetProfile(playerId), "speak_for", true);
			array<int> promised = {playerId};
			RZ_StoryKit.Record(this, "speak_for", HouseKey(), "1", promised);
			gm.NotifyPlayer(playerId, m_sTitle, "\"Fine. But you speak for us at the next collection. You.\"");
		}

		m_fRewardScale = 0.75;
		Collect(gm, playerId, 2, 0, "persuaded", "She pays what she can, two tins short of the full count, and looks you in the eye while she does it.");
	}

	//------------------------------------------------------------------------------------------------
	protected void Press(RZ_GameModeComponent gm, int playerId)
	{
		int difficulty = 2;
		if (gm.GetQuestlines() && gm.GetQuestlines().WorldFlag("well.done") > 0)
			difficulty = 1;		// the well story's been told

		RZ_ECheck result = m_Ways.TryFear(gm, playerId, "racket.keeper." + m_iHouse.ToString(), RZ_EFaction.SURVIVORS, RZ_EFaction.BANDITS, difficulty, false, PRESS_INFAMY, "excon,raider");
		if (result == RZ_ECheck.PASS)
		{
			Collect(gm, playerId, FOOD, 0, "pressed", "You say it quietly, close. She pays the full count. Nobody in that doorway will forget your face.");
			return;
		}

		if (result == RZ_ECheck.SHORT)
		{
			Collect(gm, playerId, FOOD, SHORT_INFAMY, "pressed", "It takes longer than it should, and louder. She pays the full count. The whole house heard.");
			return;
		}

		// Refused: the keeper's people come out, if there's a group and room; else the door shuts
		ResourceName prefab = gm.GetSurvivorGroupPrefab();
		if (prefab.IsEmpty() || !RZ_Escort.HasRoom(gm))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"No.\" The door shuts in your face, and a bar drops behind it.");
			m_bDone = true;
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		m_bLeaned = true;
		m_bFight = true;
		m_Locals = RZ_Util.SpawnPrefab(prefab, LandPointNear(m_vPos, 8, 15));
		RZ_Hostility.MarkFight(RZ_EFaction.SURVIVORS, m_Locals, 120);
		RZ_ContractHelp.NotifyNear(gm, m_vPos, 120, m_sTitle, "The keeper's people come out with whatever they could grab. They're not paying. Not while they're standing.");
	}

	//------------------------------------------------------------------------------------------------
	//! Cover it yourself: 4 food from your pack, or 20 Red Hand Favor. The house loses nothing, the boss
	//! pays half, and it's a lie the next collection here finds out.
	protected void Cover(RZ_GameModeComponent gm, int playerId, IEntity source)
	{
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		bool paid = me && RZ_HavenQuest.CountFood(gm, me) >= COVER_FOOD && RZ_HavenQuest.TakeFood(gm, me, COVER_FOOD);
		if (!paid)
			paid = m_Ways.TryBribe(gm, playerId, RZ_EFaction.BANDITS, COVER_FAVOR);

		if (!paid)
			return;

		m_fRewardScale = 0.5;
		RZ_Deeds.AddLifeRecord(gm.GetProfile(playerId), "covered", true);
		RZ_WorldFacts.Get().Memory().Lies().Tell(gm, playerId, "racket.covered", HouseKey(), RZ_EFaction.BANDITS, "", m_vPos, RZ_ELieTest.TICK, COVER_FOUND_INFAMY,
			string.Format("The crew came back to %1. The storekeeper told them a friend of the crew paid last time. Now the crew knows which friend.", m_sHouse));
		Collect(gm, playerId, 0, 0, "covered", "You count your own tins into the crew's sack. The storekeeper watches and says nothing. The boss will call it light this week.");
	}

	//------------------------------------------------------------------------------------------------
	protected void Receipt(RZ_GameModeComponent gm, int playerId)
	{
		RZ_Errands errands = gm.GetErrands();
		if (errands)
			errands.Morale(RZ_EFaction.BANDITS, -2, "crews at odds");

		Collect(gm, playerId, 0, 0, "receipt", "You send the receipt up the chain. The boss reads it twice, and goes looking for Janda's crew instead. The house pays nothing.");
	}

	//------------------------------------------------------------------------------------------------
	protected void Council(RZ_GameModeComponent gm, int playerId)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses)
			houses.RemovePopulation(m_iHouse, 1);

		Collect(gm, playerId, FOOD, 0, "council", "The council head pays from the stores and puts her out the same hour. A girl loses her mother's place by the stove.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnHoldDone(RZ_GameModeComponent gm, int decider, int option)
	{
		super.OnHoldDone(gm, decider, option);
		if (option != ACT_SIDE || m_bDone)
			return;

		m_bDone = true;
		gm.AddInfamy(decider, RZ_EFaction.BANDITS, SIDE_INFAMY, "warned a house about a collection");
		array<int> who = {decider};
		RZ_StoryKit.Record(this, m_iHouse.ToString(), HouseKey(), "warned", who);
		gm.NotifyPlayer(decider, m_sTitle, "You tell the council head what's coming. By the time anyone else looks, the stores are under the floor. Nobody pays you. The boss will remember.");
		m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	//! The house pays `food`; the collector takes `extra` Haven Infamy on top; the record says how.
	protected void Collect(RZ_GameModeComponent gm, int playerId, int food, int extra, string how, string text)
	{
		m_bDone = true;
		m_iFood = food;
		m_iExtraInfamy = extra;
		m_sHow = how;
		RZ_Safehouses houses = gm.GetSafehouses();
		RZ_Safehouse h = houses.Get(m_iHouse);
		if (h)
			h.m_iFood = Math.Max(0, h.m_iFood - food);

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
			Collect(gm, m_iCollector, FOOD, PRESS_INFAMY, "forced", "It's over. The keeper pays with blood on the floor. You did this.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
		if (m_iExtraInfamy > 0)
			gm.AddInfamy(playerId, RZ_EFaction.SURVIVORS, m_iExtraInfamy, "pressed a house");
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		string epilogue = "";
		if (m_sHow == "secret")
			epilogue = string.Format("%1's stores still have tally marks for every tin the Red Hand took. Some are in a different pencil.", m_sHouse);

		RZ_StoryKit.Record(this, m_iHouse.ToString(), HouseKey(), m_sHow, m_aParticipants, epilogue, "board");

		// The callback note: some hours of the collector's play later, near the house
		RZ_Shared shared = gm.GetShared();
		if (shared && m_iCollector > 0 && m_sHow != "receipt")
			shared.GetCallbacks().Queue(m_iCollector, "A note at " + m_sHouse, string.Format("Pinned where the receipt used to be, in the storekeeper's hand: \"Collected by the Red Hand, Day %1. We ate anyway.\"", RZ_SeasonClock.Day()), m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailed(RZ_GameModeComponent gm)
	{
		if (m_bDone)
			return;

		// Time up: the crew collects without you, and the house is short again. Its cook asks for help
		RZ_Safehouse h = gm.GetSafehouses().Get(m_iHouse);
		if (!h)
			return;

		h.m_iFood = Math.Max(0, h.m_iFood - FOOD);
		RZ_StoryKit.Fallout(gm, this, "pot", h.m_vPos, m_sHouse);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (m_bDone)
			return string.Format("Protection Racket (%1): nobody paid.", m_sHouse);

		return string.Format("Protection Racket (%1): the crew collected without you.", m_sHouse);
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
		m_sDeed = "took a convoy near {town}";
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

		// The post it was for changed hands: there's no truck to take (and nothing to pay for)
		if (result == RZ_SupplyLines.TRUCK_CALLED_OFF)
		{
			Withdraw(string.Format("Called off. Bastion lost its post in %1, and the truck turned round before it reached the road.", m_Watch.m_sTo));
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
//! #13 Bust Out: Red Hand prisoners in a Bastion stockade near one of its outposts (never the
//! stronghold; the gate 45-70 m from the seat). Cut the lock (20 s at the gate, 8 s for an Ex-Con).
//! The stockade's guards are the mission's watchers. In Bastion kit, Cut the lock is a claim: a guard
//! asks "Who sent you?" (a look at difficulty 2; the post commander's name is the cover detail). Pass
//! and there's no alarm; a 20 s cut gets a second look at 15 s ("Orders. They're being moved."), with
//! -1 for the look already passed. Fail either and it's the alarm: a fight, so you're never held.
//! Without Bastion kit, the guards see you and it's a fight. Then walk the prisoners (RZ_Escort) to a
//! Red Hand drop, whose crew never reacts to colours on the mission's players. Bastion Infamy.
class RZ_Mission_BustOut : RZ_PuzzleMission
{
	protected ref RZ_Escort m_Escort = new RZ_Escort();
	protected vector m_vGate;
	protected vector m_vMeet;
	protected string m_sMeetName;
	protected bool m_bShown;
	protected IEntity m_Guards;
	protected int m_iCutter = -1;
	protected int m_iCommander = -1;
	protected string m_sCommander;
	protected bool m_bClaimed;
	protected bool m_bSecondLook;
	protected ref RZ_Ways m_Ways = new RZ_Ways("Bust Out");
	protected static const float SECOND_LOOK = 15;
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

		// After The Book went to Bastion (world flag book.bastion), Bastion keeps Red Hand prisoners longer
		if (gm.GetQuestlines() && gm.GetQuestlines().HasWorldFlag("book.bastion"))
			return 1.2;

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

		// Only an outpost's stockade, never the stronghold's
		RZ_Territory territory = gm.GetTerritory();
		if (territory && territory.IsActive())
		{
			RZ_Holding stronghold = territory.GetStronghold(RZ_EFaction.MILITARY);
			if (stronghold && RZ_Util.DistXZ(stronghold.m_vPos, post) < 400)
				return false;
		}

		m_sCommander = "";
		if (gm.GetLeaders())
		{
			RZ_Leader seat = gm.GetLeaders().FindNearest(RZ_EFaction.MILITARY, post, 300);
			if (seat)
			{
				m_iCommander = seat.m_iId;
				m_sCommander = seat.m_sShort;
			}
		}

		m_vGate = LandPointNear(post, 45, 70);
		if (!RZ_LeaderJob.MeetingPoint(gm, RZ_EFaction.BANDITS, m_vGate, m_vMeet, m_sMeetName))
			return false;

		m_vPos = m_vGate;
		m_iSettlement = gm.FindNearestSettlement(m_vGate);
		m_Escort.m_sName = "the prisoners";
		m_Escort.m_sPrefab = RZ_Escort.PrefabFor(gm, 0);
		m_Escort.SetDestination(m_vMeet);
		m_sTitle = "Bust Out";
		m_sDeed = "broke one of ours out at {town}";
		m_sBrief = string.Format("Bastion is holding some of ours in a stockade near %1. They owe us, and nobody pays from a stockade. Cut them out and walk them to %2. In one of their uniforms, the guards won't look twice.", postName, m_sMeetName);
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
			{
				gm.GetHostility().WatchUnit(m_Guards, RZ_EFaction.MILITARY);
				gm.GetHostility().GetDisguise().AddWatcher(m_Guards, RZ_EFaction.MILITARY);
			}
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
		// The second look, at 15 s into a 20 s cut: "What are you doing with those?"
		if (m_bClaimed && !m_bSecondLook && !m_bAlarm && m_fCut >= SECOND_LOOK && m_fCutNeeded > SECOND_LOOK)
		{
			m_bSecondLook = true;
			gm.NotifyPlayer(m_iCutter, m_sTitle, "A guard looks at the cutters, then at you. \"What are you doing with those?\" \"Orders. They're being moved.\"");
			if (!gm.GetHostility().GetDisguise().Look(m_iCutter, RZ_EFaction.MILITARY, 2, m_sTitle, Paper(gm, m_iCutter), 120))
				Alarm(gm, m_iCutter, "\"Moved where?\" He's already shouting.");
		}

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
		RZ_Disguise disguise = gm.GetHostility().GetDisguise();
		bool guarded = RZ_Util.CountAlive(m_Guards) > 0;
		if (!m_bAlarm && guarded && disguise.InTheirKit(playerId, RZ_EFaction.MILITARY))
		{
			// The claim: "Who sent you?" (the commander's name is the cover detail)
			m_bClaimed = true;
			if (m_Ways.PassAs(gm, playerId, "stockade guard", RZ_EFaction.MILITARY, 2, Paper(gm, playerId)) == RZ_ECheck.PASS)
				text = "A guard asks who sent you. You tell him, and he looks away. Cut, quietly.";
			else
				Alarm(gm, playerId, "");

			gm.NotifyPlayer(playerId, m_sTitle, text);
			return;
		}

		if (!m_bAlarm && guarded)
		{
			Alarm(gm, playerId, "");
			text = "A shout from the wire: the guards have seen you. Cut fast.";
		}

		gm.NotifyPlayer(playerId, m_sTitle, text);
	}

	//------------------------------------------------------------------------------------------------
	//! The commander's name, if this player has heard it (met him): the cover detail at the stockade.
	protected string Paper(RZ_GameModeComponent gm, int playerId)
	{
		if (m_sCommander.IsEmpty() || !gm.GetLeaders() || !gm.GetLeaders().HasMet(m_iCommander, playerId))
			return "";

		return "Name: " + m_sCommander;
	}

	//------------------------------------------------------------------------------------------------
	//! The alarm: a fight with the stockade's guards (never held: it's a fight).
	protected void Alarm(RZ_GameModeComponent gm, int playerId, string line)
	{
		if (m_bAlarm)
			return;

		m_bAlarm = true;
		RZ_Hostility.MarkFight(RZ_EFaction.MILITARY, m_Guards, 150);
		if (!line.IsEmpty())
			gm.NotifyPlayer(playerId, m_sTitle, line);
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
//! may see through it (35%, 15% for a Police officer they believe; certain in Red Hand colours): +20
//! Bastion Infamy for whoever
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

		// The colours rule: handed in wearing the Red Hand's colours, Bastion sees through it at once
		RZ_Hostility hostility = gm.GetHostility();
		bool redColours = hostility && hostility.GetDisguise().WearsColours(playerId, RZ_EFaction.BANDITS);
		m_bCaught = redColours || Math.RandomFloat01() < chance;
		if (m_bCaught && redColours)
		{
			gm.NotifyPlayer(playerId, "Task Force Bastion", "The lieutenant looks at the red on you before he looks at the dossier. He doesn't open it.");
			return;
		}

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
		m_sDeed = "collected a bounty near {town}";
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
	protected bool m_bFinale;	// the cure finale's mode: the Red Hand route's holder change at the loading bay

	//------------------------------------------------------------------------------------------------
	//! The cure finale runs: Take the Lab is the Red Hand route's once-a-season theft (docs/FINALE.md).
	protected static bool FinaleMode()
	{
		return RZ_CureFinale.Get() && RZ_CureFinale.IsOn() && RZ_CureFinale.Get().IsOpen();
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		m_bFinale = FinaleMode();
		if (m_bFinale)
		{
			// Started by a Red Hand-locked player Trusted with the Red Hand; the holder change is the prize
			SetContract(RZ_EFaction.BANDITS, RZ_ETier.TRUSTED, 10, 20);
			m_fHoldNeeded = RZ_CureFinale.Get().BaySeconds();
		}
		else
		{
			SetContract(RZ_EFaction.BANDITS, RZ_ETier.CHAMPION, 60, 60);
			SetRival(RZ_EFaction.INSTITUTE, 40);
			m_fHoldNeeded = 90;
		}

		m_fHoldRadius = 10;
		m_fTimeLeft = 900;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		// While a finale runs it leaves the random pool: the controller offers it at the stand
		if (!gm.HasLab() || !gm.GetCure().IsFinalStand() || FinaleMode())
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
		if (m_bFinale)
		{
			m_vPos = RZ_CureFinale.Get().BayPos();
			m_sBrief = "The lorry backs up to the lab's loading bay. Hold the bay while the crew goes in, with none of the holders' people near it, and the clean room is ours.";
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The finale's bay hold: only Red Hand-locked players who count at the stand hold it, one of
	//! them Trusted to start it, and the count pauses while the holder's side is within 15 m.
	protected void FinaleHold(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_CureFinale finale = RZ_CureFinale.Get();
		if (!finale || !gm.GetCure().IsFinalStand())
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		bool counted = false;
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), m_vPos) > m_fHoldRadius || !finale.CountsForBay(ids[i]))
				continue;

			if (!m_bStarted && !finale.CanStartTake(ids[i]))
				continue;

			m_bStarted = true;
			counted = true;
			if (!m_aParticipants.Contains(ids[i]))
				m_aParticipants.Insert(ids[i]);
		}

		if (!counted || finale.BayContested(players))
			return;

		m_fHeld += dt;
		if (m_fHeld < m_fHoldNeeded || m_bTaken)
			return;

		m_bTaken = true;
		m_eState = RZ_EMissionState.SUCCEEDED;
		finale.OnTakeTheLab();
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
		if (m_bFinale)
		{
			FinaleHold(gm, dt, ids, players);
			return;
		}

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
