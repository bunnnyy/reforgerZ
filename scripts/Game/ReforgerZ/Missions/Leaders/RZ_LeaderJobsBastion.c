//! Task Force Bastion leader jobs (docs/MISSIONS.md, "Leader missions"): from outpost
//! commanders who interpret their orders (Independent) or have stopped waiting for any (Rogue).

//------------------------------------------------------------------------------------------------
//! Independent. Rations to a Haven safehouse against orders: that safehouse +4 food, a little
//! Haven Fame, and the commander drifts a little further from the stronghold.
class RZ_Job_HeartsAndMinds : RZ_LeaderDelivery
{
	protected int m_iHouse = -1;

	protected static const int FOOD = 4;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.MILITARY, RZ_ELoyalty.INDEPENDENT, RZ_Leaders.TONE_HOPEFUL | RZ_Leaders.TONE_CROSS, 6, 20);
	}

	//------------------------------------------------------------------------------------------------
	override float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		if (!CanRun(gm, RZ_EFaction.SURVIVORS) || RZ_LeaderJob.SafehouseNear(gm, leader.m_vPos, 300, 6000, -1) < 0)
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupJob(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		m_iHouse = RZ_LeaderJob.SafehouseNear(gm, l.m_vPos, 300, 6000, -1);
		RZ_Safehouse h = gm.GetSafehouses().Get(m_iHouse);
		if (!h || !SpawnItemAtSeat(gm))
			return false;

		if (!AddDrop(gm, RZ_EFaction.SURVIVORS, LandPointNear(h.m_vPos, 3, 8), "the Haven safehouse in " + h.m_sName))
			return false;

		m_sTitle = "Hearts and Minds";
		m_sBrief = string.Format("Rations stay in Bastion hands. That's the order. Command also hasn't called in nine days. There's a Haven safehouse in %1 with children in it. Take them a crate. Don't sign for it.",
			h.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnJobDelivered(RZ_GameModeComponent gm, int playerId, int dropIndex)
	{
		gm.GetSafehouses().AddFood(m_iHouse, FOOD);
		RZ_Leader l = GetLeader(gm);
		if (l)
			gm.GetLeaders().ShiftLoyalty(l, -2, false);

		gm.NotifyPlayer(playerId, m_sTitle, "The crate has a Bastion stencil on it. Somebody at the safehouse scrubs it off before the children see.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
		RZ_LeaderJob.CrossFame(gm, playerId, RZ_EFaction.SURVIVORS, 4);
	}
}

//------------------------------------------------------------------------------------------------
//! Independent. Command's last orders are in a dead courier's satchel. Picking it up shows
//! what they say (60%: the Zone's garrison was written off). Bring them to the commander
//! (loyalty -8 if abandoned, +5 if not) or to the stronghold, which buries them (more
//! Bastion Fame; the commander doesn't forgive it).
class RZ_Job_RealOrders : RZ_LeaderDelivery
{
	protected bool m_bAbandoned;
	protected bool m_bRead;
	protected string m_sOrders;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.MILITARY, RZ_ELoyalty.INDEPENDENT, RZ_Leaders.TONE_TALK, 6, 20);
	}

	//------------------------------------------------------------------------------------------------
	override float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		RZ_Leader top = RZ_LeaderJob.Stronghold(gm, RZ_EFaction.MILITARY);
		if (!CanRun(gm, RZ_EFaction.MILITARY) || !top || top == leader)
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupJob(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		RZ_Leader top = RZ_LeaderJob.Stronghold(gm, RZ_EFaction.MILITARY);
		if (!top)
			return false;

		m_iSettlement = PickSettlementAway(gm, l.m_vPos, 1000, 3500, 20, 90);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.6);
		if (!SpawnItem(gm, m_vPos))
			return false;

		if (!AddDrop(gm, RZ_EFaction.MILITARY, LandPointNear(l.m_vPos, 6, 12), string.Format("%1, %2", l.m_sShort, l.m_sPlace)))
			return false;

		if (!AddDrop(gm, RZ_EFaction.MILITARY, LandPointNear(top.m_vPos, 6, 12), "the stronghold in " + top.m_sPlace))
			return false;

		SpawnGuards(gm, m_vPos, 4, 10, 40);
		m_bAbandoned = Math.RandomFloat01() < 0.6;
		if (m_bAbandoned)
			m_sOrders = "FLASH. All cordon personnel inside the line: hold current positions indefinitely. No relief, no extraction. Assets inside the Zone are to be considered expended. Acknowledge.";
		else
			m_sOrders = "PRIORITY. Hold current positions. Relief column assembling, ETA unconfirmed. Maintain discipline. You have not been forgotten.";

		m_sTitle = "The Real Orders";
		m_sBrief = string.Format("The courier carrying command's last orders never reached us. His satchel is somewhere in %1. I want to read them before anyone tells me what they say. Bring them to me. The stronghold will want them too.",
			s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnCarrierChanged(RZ_GameModeComponent gm, int previous, int carrier)
	{
		super.OnCarrierChanged(gm, previous, carrier);
		if (carrier <= 0 || m_bRead)
			return;

		m_bRead = true;
		gm.NotifyPlayer(carrier, "The orders", m_sOrders);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnJobDelivered(RZ_GameModeComponent gm, int playerId, int dropIndex)
	{
		RZ_Leader l = GetLeader(gm);
		if (dropIndex == 0)
		{
			if (l && m_bAbandoned)
			{
				gm.GetLeaders().ShiftLoyalty(l, -8, true);
				gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 reads it twice, then a third time. \"Expended.\" They fold it very small. \"Thank you. Don't tell the others yet.\"", l.m_sShort));
			}
			else if (l)
			{
				gm.GetLeaders().ShiftLoyalty(l, 5, true);
				gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 reads it and breathes out for what looks like the first time in a week. \"Not forgotten. Right.\"", l.m_sShort));
			}

			return;
		}

		m_iFameReward = 10;
		m_bNoLeaderCredit = true;
		if (l)
			l.Remember(gm.GetProfile(playerId).m_sIdentity, RZ_Leaders.MEM_CROSSED);

		gm.NotifyPlayer(playerId, m_sTitle, "The duty officer at the stronghold takes the satchel without opening it. It goes into the stove. \"Good work. These never arrived.\"");
	}
}

//------------------------------------------------------------------------------------------------
//! Independent. Stand with raw recruits on their first patrol at the edge of a town for four
//! minutes. With the survivor group prefab, the recruits are real and must live.
class RZ_Job_TrainingDay : RZ_LeaderHold
{
	protected IEntity m_Recruits;
	protected bool m_bRecruitsSpawned;	// the group deletes itself when its last member dies: track it here
	protected bool m_bSeenRecruits;
	protected float m_fRecruitAge;
	protected int m_iMostRecruits;

	protected static const float SPAWN_RADIUS = 300;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.MILITARY, RZ_ELoyalty.INDEPENDENT, RZ_Leaders.TONE_HARD, 8, 20);
		m_fHoldNeeded = 240;
		m_fHoldRadius = 40;
		m_fWaveInterval = 60;
		m_iMaxWaves = 3;
		m_fWaveTimer = 50;
		m_fParticipationRadius = 60;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		if (!l)
			return false;

		m_iSettlement = PickSettlementAway(gm, l.m_vPos, 500, 2000, 20, 65);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.8, s.m_fRadius + 40);
		m_sTitle = "Training Day";
		m_sBrief = string.Format("Six recruits. Last week they were students. Today they do their first patrol, at the edge of %1, and they are going to panic. Stand with them for four minutes. Bring them all back.",
			s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		// The recruits turn up when someone comes to stand with them (AI only near players)
		if (!m_bRecruitsSpawned && RZ_ContractHelp.AnyNear(players, m_vPos, SPAWN_RADIUS))
		{
			m_bRecruitsSpawned = true;
			m_Recruits = RZ_Util.SpawnPrefab(gm.GetSurvivorGroupPrefab(), m_vPos);
		}

		if (m_bRecruitsSpawned)
		{
			// A null group (deleted when empty) counts as zero alive
			m_fRecruitAge += dt;
			int alive = RZ_Util.CountAlive(m_Recruits);
			if (alive > 0)
				m_bSeenRecruits = true;

			if (alive > m_iMostRecruits)
				m_iMostRecruits = alive;

			if (alive == 0 && (m_bSeenRecruits || m_fRecruitAge > 15))
			{
				m_eState = RZ_EMissionState.FAILED;
				return;
			}
		}

		super.OnUpdate(gm, dt, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (m_bRecruitsSpawned && RZ_Util.CountAlive(m_Recruits) < m_iMostRecruits)
		{
			NotifyParticipants(gm, m_sTitle, "They walk back in step, more or less. Not all of them. Nobody says the names on the way in.");
			return;
		}

		NotifyParticipants(gm, m_sTitle, "They walk back in step, more or less. One of them is sick behind the truck. All of them are alive.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		RZ_Util.DeleteUnit(m_Recruits);
	}
}

//------------------------------------------------------------------------------------------------
//! Independent. Two families in a village are fighting over a well. Settle it with words:
//! one shot fired near them and it's over. Splitting it pays in full.
class RZ_Job_LocalLaw : RZ_LeaderPuzzle
{
	protected string m_sFamilyA;
	protected string m_sFamilyB;
	protected int m_iJudge = -1;

	protected static const float NO_SHOT_RADIUS = 80;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.MILITARY, RZ_ELoyalty.INDEPENDENT, RZ_Leaders.TONE_TALK | RZ_Leaders.TONE_HOPEFUL, 6, 20);
	}

	//------------------------------------------------------------------------------------------------
	override float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.WELL))
			return 0;

		return super.GetLeaderWeight(gm, leader);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		if (!l)
			return false;

		m_iSettlement = PickSettlementAway(gm, l.m_vPos, 800, 3000, 0, 60);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.4);
		m_sFamilyA = RZ_NoteText.Surname(m_Rng);
		m_sFamilyB = RZ_NoteText.Surname(m_Rng);
		if (m_sFamilyB == m_sFamilyA)
			m_sFamilyB = "Kowal";

		int a = AddClue("The " + m_sFamilyA + "s", string.Format("\"Our grandfather dug that well. The %1s moved here in the spring and now they drink first and wash their car in it. Their car! There's no petrol in the Zone!\"", m_sFamilyB));
		int b = AddClue("The " + m_sFamilyB + "s", string.Format("\"We have a baby and the %1s have goats. Ask them who's been sleeping by the well with a shotgun. Go on, ask.\"", m_sFamilyA));
		m_iJudge = AddClue("The village", "Everyone is watching to see whose side Bastion takes. Nobody has a gun out. Yet.");
		SpawnClue(gm, RZ_EPuzzleProp.CIVILIAN, LandPointNear(m_vPos, 6, 10), a, "Talk to the " + m_sFamilyA + "s");
		SpawnClue(gm, RZ_EPuzzleProp.CIVILIAN, LandPointNear(m_vPos, 6, 10), b, "Talk to the " + m_sFamilyB + "s");
		string labels = string.Format("Give the well to the %1s|Give the well to the %2s|Split the days between them", m_sFamilyA, m_sFamilyB);
		SpawnObject(gm, RZ_EPuzzleProp.WELL, m_vPos, 0, "", labels, m_iJudge, "Look at the well");

		m_sTitle = "Local Law";
		m_sBrief = string.Format("Two families in %1 are about to kill each other over a well. If Bastion settles it with rifles, we're the Red Hand with better boots. Settle it with words. No shots. Not one.",
			s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerShot(RZ_GameModeComponent gm, int playerId, vector pos)
	{
		if (m_eState != RZ_EMissionState.ACTIVE || RZ_Util.DistXZ(pos, m_vPos) > NO_SHOT_RADIUS)
			return;

		if (gm.GetSpawner().IsZombieNear(m_vPos, 40))
			return;

		gm.NotifyPlayer(playerId, m_sTitle, "The shot ends the argument the worst way. Both families go for their guns. Whatever happens next, it isn't law.");
		m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0 || option < 0 || option > 2)
			return;

		ChosenBy(playerId);
		if (option == 2)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "Odd days and even days. Nobody likes it. Nobody shoots anybody. That's the job.");
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		string loser = m_sFamilyB;
		if (option == 1)
			loser = m_sFamilyA;

		m_fRewardScale = 0.8;
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("You decide. The %1s pack a handcart that night and leave the village. It's quiet, which isn't the same as peace.", loser));
		m_eState = RZ_EMissionState.SUCCEEDED;
	}
}

//------------------------------------------------------------------------------------------------
//! Rogue. Army rifles to a Red Hand buyer for fuel and food. Small Red Hand Fame. Turn it in
//! at the stronghold instead and the commander is exposed.
class RZ_Job_RiflesForSale : RZ_LeaderDelivery
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.MILITARY, RZ_ELoyalty.ROGUE, RZ_Leaders.TONE_DARK | RZ_Leaders.TONE_CROSS, 0, 35);
	}

	//------------------------------------------------------------------------------------------------
	override float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		if (!CanRun(gm, RZ_EFaction.BANDITS))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupJob(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		vector meet;
		string meetName;
		if (!RZ_LeaderJob.MeetingPoint(gm, RZ_EFaction.BANDITS, l.m_vPos, meet, meetName))
			return false;

		if (!SpawnItemAtSeat(gm) || !AddDrop(gm, RZ_EFaction.BANDITS, meet, meetName))
			return false;

		AddExposeDrop(gm);
		m_sTitle = "Rifles for Sale";
		m_sBrief = string.Format("Twelve rifles fell off the inventory. Inventory is a state of mind. A Red Hand buyer has fuel and fresh food, and we have neither. Deliver the crate to %1. Don't open it, don't count it, don't ask.",
			meetName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnJobDelivered(RZ_GameModeComponent gm, int playerId, int dropIndex)
	{
		gm.NotifyPlayer(playerId, m_sTitle, "The buyer opens it, counts it and grins at the serial numbers. \"Tell your commander we'll be in touch.\" You'll see those rifles again.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
		if (!m_bExposed)
			RZ_LeaderJob.CrossFame(gm, playerId, RZ_EFaction.BANDITS, 4);
	}
}

//------------------------------------------------------------------------------------------------
//! Rogue. Collect a "contribution" from a village for the commander's stores. Take it all
//! (Haven Infamy; the nearest safehouse loses 4 food if it's that village), take half and lie
//! (less of both), or pay it yourself (15 Bastion Favor; nobody is hurt).
class RZ_Job_OffBookRequisition : RZ_LeaderPuzzle
{
	protected int m_iTalk = -1;

	protected static const int FULL_INFAMY = 6;
	protected static const int PAY_FAVOR = 15;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.MILITARY, RZ_ELoyalty.ROGUE, RZ_Leaders.TONE_DARK, 0, 30);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		if (!l)
			return false;

		// A safehouse village if there's one close, else any quiet village
		int house = RZ_LeaderJob.SafehouseNear(gm, l.m_vPos, 500, 4000, -1);
		RZ_Safehouse h;
		if (gm.GetSafehouses())
			h = gm.GetSafehouses().Get(house);

		if (h)
			m_iSettlement = h.m_iSettlement;
		else
			m_iSettlement = PickSettlementAway(gm, l.m_vPos, 800, 3000, 0, 60);

		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s || IsTownBusy(gm, m_iSettlement))
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.4);
		m_iTalk = AddClue("The village elder", "\"Bastion. Of course. We gave last week, to the other one. Do they not talk to each other?\" She already knows they don't.");
		string labels = string.Format("Take the full contribution|Take half and say it was all|Pay it yourself (%1 Bastion Favor)", PAY_FAVOR);
		SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPos, 0, "", labels, m_iTalk, "Talk to the elder");

		m_sTitle = "Off-Book Requisition";
		m_sBrief = string.Format("%1 has been eating well while my people eat paste. Collect a contribution for our stores. Don't call it a tax. Don't put it in writing.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0)
			return;

		if (option == 2 && !gm.SpendFavor(playerId, RZ_EFaction.MILITARY, PAY_FAVOR))
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("You'd need %1 Bastion Favor. You have %2.", PAY_FAVOR, gm.GetProfile(playerId).GetFavor(RZ_EFaction.MILITARY)));
			return;
		}

		ChosenBy(playerId);
		RZ_Safehouses houses = gm.GetSafehouses();
		if (option == 0)
		{
			if (houses && houses.FindNearest(m_vPos, 400) >= 0)
				houses.TakeFood(m_vPos, 4);

			gm.AddInfamy(playerId, RZ_EFaction.SURVIVORS, FULL_INFAMY);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("You take what was asked for. The elder writes your face down in her head. (+%1 Haven Infamy)", FULL_INFAMY));
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (option == 1)
		{
			if (houses && houses.FindNearest(m_vPos, 400) >= 0)
				houses.TakeFood(m_vPos, 2);

			m_fRewardScale = 0.6;
			gm.AddInfamy(playerId, RZ_EFaction.SURVIVORS, FULL_INFAMY / 2);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("You take half and tell the commander it was all. The elder doesn't thank you. She does remember. (+%1 Haven Infamy)", FULL_INFAMY / 2));
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (option == 2)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You pay it out of your own Bastion credit and carry a box of the village's own tins back to the post. The commander never knows the difference.");
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}
}

//------------------------------------------------------------------------------------------------
//! Rogue. An audit is coming: burn the supply ledger behind the post within 15 minutes (heat),
//! or keep it for the auditor (the commander is exposed; the stronghold pays).
class RZ_Job_BurnTheLedger : RZ_LeaderPuzzle
{
	protected int m_iRead = -1;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.MILITARY, RZ_ELoyalty.ROGUE, RZ_Leaders.TONE_DARK, 0, 30);
		m_fTimeLeft = 900;
	}

	//------------------------------------------------------------------------------------------------
	override float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.OBJECT))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		if (!l)
			return false;

		m_vPos = LandPointNear(l.m_vPos, 40, 120);
		m_iSettlement = gm.FindNearestSettlement(m_vPos);
		m_iRead = AddClue("Supply ledger", string.Format("Columns that don't add up: 40 rations in, 22 out, 18 'spoiled'. Fuel: 200 litres in, 60 'evaporated'. Every page signed %1.", l.m_sName));
		SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vPos, 0, "Burn the ledger|Keep it for the auditor", "", m_iRead, "Read the ledger");

		string auditor = "the stronghold";
		RZ_Leader top = RZ_LeaderJob.Stronghold(gm, RZ_EFaction.MILITARY);
		if (top)
			auditor = top.m_sPlace;

		m_sTitle = "Burn the Ledger";
		m_sBrief = string.Format("There's an audit coming from %1. The supply ledger is in a crate behind the post. It needs to not exist in fifteen minutes. You understand.", auditor);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx != 0)
			return;

		ChosenBy(playerId);
		if (value == 0)
		{
			gm.GetHeat().AddHeat(m_vPos, 25);
			gm.NotifyPlayer(playerId, m_sTitle, "Paper burns faster than you'd think. The smoke goes straight up. Anything nearby that can smell smoke knows where you are.");
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (value == 1)
		{
			m_bExposed = true;
			m_bRogueJob = false;
			m_iFameReward = 10;
			m_iFavorReward = 10;
			gm.NotifyPlayer(playerId, m_sTitle, "You tuck the ledger under your arm and wait for the auditor by the road. The stronghold will want to talk to whoever signed it.");
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}
}

//------------------------------------------------------------------------------------------------
//! Desertion Route (Rogue): two soldiers want out, and the commander is letting them go. Walk them
//! (RZ_Escort) from the post to a Haven safehouse; they join it. Or talk to them on the way and
//! march them to the stronghold instead: the commander is turned in.
//! *Simplified:* a safehouse only, not the Blind Spot; the two are one escorted group.
class RZ_Job_DesertionRoute : RZ_PuzzleMission
{
	protected ref RZ_Escort m_Escort = new RZ_Escort();
	protected int m_iHouse = -1;
	protected vector m_vTop;
	protected string m_sTopPlace;
	protected bool m_bMet;
	protected bool m_bTurnIn;
	protected int m_iTalk = -1;

	protected static const int PEOPLE = 2;
	protected static const int EXPOSE_FAME = 10;
	protected static const int EXPOSE_FAVOR = 10;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.MILITARY, RZ_ELoyalty.ROGUE, RZ_Leaders.TONE_HOPEFUL | RZ_Leaders.TONE_CROSS, 0, 35);
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		if (!RZ_Escort.HasRoom(gm) || !leader || RZ_LeaderJob.SafehouseNear(gm, leader.m_vPos, 300, 5000, -1) < 0)
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Leader l = RZ_LeaderJob.Leader(gm, this);
		if (!l)
			return false;

		m_iHouse = RZ_LeaderJob.SafehouseNear(gm, l.m_vPos, 300, 5000, -1);
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, m_iHouse);
		RZ_Leader top = RZ_LeaderJob.Stronghold(gm, RZ_EFaction.MILITARY);
		if (!h || !top || top == l)
			return false;

		m_vTop = LandPointNear(top.m_vPos, 6, 12);
		m_sTopPlace = top.m_sPlace;
		m_vPos = LandPointNear(l.m_vPos, 4, 8);
		m_iSettlement = gm.FindNearestSettlement(l.m_vPos);
		m_iTalk = AddClue("Two soldiers", "Two of them, packs already on, rifles left behind. \"The commander says you'll get us somewhere they won't look.\"");
		m_Escort.m_sName = "the two soldiers";
		m_Escort.m_sPrefab = RZ_Escort.PrefabFor(gm, 2);
		m_Escort.SetDestination(h.m_vPos);
		m_Escort.SetTalk(m_iId, 0, "Walk them to the safehouse|March them to the stronghold instead", m_iTalk, "Talk to the soldiers");
		m_sTitle = "Desertion Route";
		m_sBrief = string.Format("Two of my people are done. I'm not going to make them stay, and I'm not going to report it. Walk them to the safehouse in %1. The Haven takes anyone.", h.m_sName);
		return m_Escort.Begin(gm, m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!RZ_LeaderJob.Leader(gm, this) && !m_bTurnIn)
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (!m_bMet && ReachedBy(ids, players, m_Escort.m_vPos, 20))
		{
			m_bMet = true;
			m_Escort.Go();
		}

		m_Escort.Update(gm, dt, ids, players);
		m_vPos = m_Escort.m_vPos;
		RZ_ContractHelp.AddNear(ids, players, m_vPos, 100, m_aParticipants);
		if (m_Escort.IsDead())
			m_eState = RZ_EMissionState.FAILED;
		else if (m_Escort.IsArrived())
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0 || option != 1 || m_bTurnIn)
			return;

		m_bTurnIn = true;
		m_bExposed = true;
		m_bRogueJob = false;
		m_bHasRival = false;
		m_iFameReward = EXPOSE_FAME;
		m_iFavorReward = EXPOSE_FAVOR;
		m_Escort.SetDestination(m_vTop);
		m_Escort.Go();
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("You tell them where they're really going. Neither of them says a word all the way to %1.", m_sTopPlace));
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (m_bTurnIn)
			return;

		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, m_iHouse);
		if (h)
			gm.GetSafehouses().AddRescued(h.m_vPos, PEOPLE, m_aParticipants);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Escort.Cleanup();
	}
}
