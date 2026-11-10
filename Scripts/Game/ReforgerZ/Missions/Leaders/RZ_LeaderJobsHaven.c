//! Haven leader jobs (docs/MISSIONS.md, "Leader missions"): from council heads who run their
//! safehouse their own way (Independent) or behind the council's back (Rogue).

//------------------------------------------------------------------------------------------------
//! Independent. Supplies meant for another safehouse are waiting in a town: bring them here
//! (this safehouse +4 food) or where they were meant to go (that one +4 food, half pay, and
//! the council head doesn't count it as a favour).
class RZ_Job_OurOwnFirst : RZ_LeaderDelivery
{
	protected int m_iOwn = -1;
	protected int m_iOther = -1;
	protected string m_sOwn;
	protected string m_sOther;

	protected static const int FOOD = 4;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.SURVIVORS, RZ_ELoyalty.INDEPENDENT, RZ_Leaders.TONE_DARK, 4, 20);
	}

	//------------------------------------------------------------------------------------------------
	override float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!CanRun(gm, RZ_EFaction.SURVIVORS) || !houses || houses.Count() < 2 || leader.m_iSafehouse < 0)
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupJob(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		RZ_Safehouses houses = gm.GetSafehouses();
		m_iOwn = l.m_iSafehouse;
		RZ_Safehouse own = houses.Get(m_iOwn);
		if (!own)
			return false;

		m_iOther = RZ_LeaderJob.SafehouseNear(gm, own.m_vPos, 500, 8000, m_iOwn);
		RZ_Safehouse other = houses.Get(m_iOther);
		if (!other)
			return false;

		m_iSettlement = PickSettlementAway(gm, own.m_vPos, 600, 2500, 0, 80);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.5);
		if (!SpawnItem(gm, m_vPos))
			return false;

		m_sOwn = own.m_sName;
		m_sOther = other.m_sName;
		if (!AddDrop(gm, RZ_EFaction.SURVIVORS, LandPointNear(own.m_vPos, 3, 8), "here, at the safehouse in " + m_sOwn))
			return false;

		if (!AddDrop(gm, RZ_EFaction.SURVIVORS, LandPointNear(other.m_vPos, 3, 8), "where it was meant to go, the safehouse in " + m_sOther))
			return false;

		m_sTitle = "Our Own First";
		m_sBrief = string.Format("There's a bundle of tins and bandages waiting in %1 for the safehouse in %2. The council decided that. Nobody asked us, and we have %3 mouths here. Bring it to me instead. Or take it where it was meant to go, if you must. I'll understand. I won't like it.",
			s.m_sName, m_sOther, own.m_iPopulation);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnJobDelivered(RZ_GameModeComponent gm, int playerId, int dropIndex)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		RZ_Leader l = GetLeader(gm);
		if (dropIndex == 0)
		{
			houses.AddFood(m_iOwn, FOOD);
			if (l)
				gm.GetLeaders().ShiftLoyalty(l, -2, false);

			gm.NotifyPlayer(playerId, m_sTitle, string.Format("The tins go on the shelf in %1. Nobody here asks where they came from. In %2, someone is checking the road.", m_sOwn, m_sOther));
			return;
		}

		houses.AddFood(m_iOther, FOOD);
		m_fRewardScale = 0.5;
		m_bNoLeaderCredit = true;
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("The safehouse in %1 gets what it was promised. The council head in %2 pays you, but only half, and doesn't look at you while doing it.", m_sOther, m_sOwn));
	}
}

//------------------------------------------------------------------------------------------------
//! Independent. A local's family is stuck in another town, and one of them is bitten.
//! Bring all of them (+3 people, the safehouse town's infection +5), the well ones only
//! (+2 people, less pay), or leave them (fails).
class RZ_Job_BringThemHome : RZ_LeaderPuzzle
{
	protected int m_iTalk = -1;
	protected string m_sFamily;
	protected vector m_vHome;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.SURVIVORS, RZ_ELoyalty.INDEPENDENT, RZ_Leaders.TONE_HOPEFUL, 8, 15);
	}

	//------------------------------------------------------------------------------------------------
	override float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		if (!gm.GetSafehouses() || gm.GetSafehouses().Count() == 0)
			return 0;

		return super.GetLeaderWeight(gm, leader);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		if (!l)
			return false;

		m_vHome = l.m_vPos;
		m_iSettlement = PickSettlementAway(gm, l.m_vPos, 1000, 3500, 0, 85);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.3, s.m_fRadius * 0.8);
		string local = RZ_NoteText.FirstName(m_Rng);
		m_sFamily = RZ_NoteText.Surname(m_Rng);
		string bitten = RZ_NoteText.FirstName(m_Rng);
		m_iTalk = AddClue("The " + m_sFamily + "s", string.Format("\"%1 sent you? Thank God. There are three of us.\" She glances at the back room. \"%2 got bitten on Tuesday. He says it's a scratch. It isn't, is it.\"", local, bitten));
		SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPos, 0, "", "Bring all three home|Bring the two who are well|Leave them where they are", m_iTalk, "Talk to the " + m_sFamily + "s");

		m_sTitle = "Bring Them Home";
		m_sBrief = string.Format("%1 hasn't slept since the gates closed. Her sister's family, the %2s, are holed up in %3. Bring them here. We'll find the beds. We always do.",
			local, m_sFamily, s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0)
			return;

		ChosenBy(playerId);
		if (option == 0)
		{
			RZ_LeaderJob.AddPeople(gm, m_vHome, 3);
			RZ_LeaderJob.TownInfection(gm, m_vHome, 5);
			gm.NotifyPlayer(playerId, m_sTitle, "All three walk home with you. The safehouse makes room. Three days later, there's a quiet argument about a locked door, and a fever in the town.");
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (option == 1)
		{
			RZ_LeaderJob.AddPeople(gm, m_vHome, 2);
			m_fRewardScale = 0.8;
			gm.NotifyPlayer(playerId, m_sTitle, "Two of them come with you. The third waves from the window and doesn't come down. Nobody talks on the way back.");
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (option == 2)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You tell them it isn't safe to move. Maybe it isn't. They lock the door behind you.");
			m_eState = RZ_EMissionState.FAILED;
		}
	}
}

//------------------------------------------------------------------------------------------------
//! Independent. Trade bread for medicine with a Bastion post nearby, against the council's
//! line on Bastion. Small Bastion Fame; the safehouse town's infection -3.
class RZ_Job_OverTheFence : RZ_LeaderDelivery
{
	protected vector m_vHome;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.SURVIVORS, RZ_ELoyalty.INDEPENDENT, RZ_Leaders.TONE_CROSS, 4, 20);
	}

	//------------------------------------------------------------------------------------------------
	override float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		if (!CanRun(gm, RZ_EFaction.MILITARY) || !RZ_LeaderJob.SeatNear(gm, RZ_EFaction.MILITARY, leader.m_vPos, 300, 5000, -1))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupJob(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		RZ_Leader post = RZ_LeaderJob.SeatNear(gm, RZ_EFaction.MILITARY, l.m_vPos, 300, 5000, -1);
		if (!post || !SpawnItemAtSeat(gm))
			return false;

		m_vHome = l.m_vPos;
		if (!AddDrop(gm, RZ_EFaction.MILITARY, LandPointNear(post.m_vPos, 6, 12), string.Format("%1, Bastion, %2", post.m_sShort, post.m_sPlace)))
			return false;

		m_sTitle = "Over the Fence";
		m_sBrief = string.Format("The Bastion post in %1 has medicine it can't use and no bread. We have bread. The council says Bastion can't be trusted. The council isn't feeding this house. Take this sack to %2 and they'll send back what we agreed.",
			post.m_sPlace, post.m_sShort);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnJobDelivered(RZ_GameModeComponent gm, int playerId, int dropIndex)
	{
		RZ_LeaderJob.TownInfection(gm, m_vHome, -3);
		gm.NotifyPlayer(playerId, m_sTitle, "A soldier counts the loaves twice and hands over a box with a red cross on it. \"Tell your people we're not all bad.\" The medicine goes back the same way.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
		RZ_LeaderJob.CrossFame(gm, playerId, RZ_EFaction.MILITARY, 3);
	}
}

//------------------------------------------------------------------------------------------------
//! Independent. Hold an old clinic in the next town while it's cleaned out: an annex.
//! The town's infection -10 and one more person at the safehouse (a nurse moves in).
class RZ_Job_OldClinic : RZ_LeaderHold
{
	protected vector m_vHome;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.SURVIVORS, RZ_ELoyalty.INDEPENDENT, RZ_Leaders.TONE_HOPEFUL | RZ_Leaders.TONE_HARD, 8, 15);
		m_fHoldNeeded = 180;
		m_fHoldRadius = 25;
		m_fWaveInterval = 60;
		m_iMaxWaves = 2;
		m_fWaveTimer = 30;
		m_fParticipationRadius = 40;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		if (!l)
			return false;

		m_vHome = l.m_vPos;
		m_iSettlement = PickSettlementAway(gm, l.m_vPos, 500, 2500, 15, 75);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.4);
		RZ_Rng rng = new RZ_Rng(m_iId * 7919 + 13);
		string nurse = RZ_NoteText.FirstName(rng);
		m_sTitle = "The Old Clinic";
		m_sBrief = string.Format("There's a clinic in %1: shelves, a bed, a door that locks. Hold it while we clear it out and it's ours, an annex. %2 says she'll move in and run it if someone makes it safe first.",
			s.m_sName, nurse);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
			s.AddInfection(-10);

		RZ_LeaderJob.AddPeople(gm, m_vHome, 1);
		NotifyParticipants(gm, m_sTitle, "The clinic's open. Somebody paints a red cross on the door with a finger. One more name goes on the safehouse list: the nurse who'll run it.");
	}
}

//------------------------------------------------------------------------------------------------
//! Rogue. Three days of the safehouse's food go to the Red Hand for antibiotics. The stores
//! lose 3 food now; the town's infection -3 when the medicine comes back. Turn it in at the
//! stronghold and the food goes back on the shelf.
class RZ_Job_RationRun : RZ_LeaderDelivery
{
	protected int m_iOwn = -1;
	protected vector m_vHome;

	protected static const int FOOD = 3;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.SURVIVORS, RZ_ELoyalty.ROGUE, RZ_Leaders.TONE_DARK | RZ_Leaders.TONE_CROSS, 0, 30);
	}

	//------------------------------------------------------------------------------------------------
	override float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!CanRun(gm, RZ_EFaction.BANDITS) || !houses || !houses.Get(leader.m_iSafehouse) || houses.Get(leader.m_iSafehouse).m_iFood < FOOD + 1)
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
		m_iOwn = l.m_iSafehouse;
		m_vHome = l.m_vPos;
		RZ_Safehouse own = gm.GetSafehouses().Get(m_iOwn);
		if (own)
			own.m_iFood = Math.Max(0, own.m_iFood - FOOD);

		m_sTitle = "Ration Run";
		m_sBrief = string.Format("Three of ours have the fever, and the Red Hand has the only antibiotics between here and the coast. They want food for them. I've put three days of our stores in this sack. The council would call it theft. I call it arithmetic. Take it to %1.",
			meetName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnJobDelivered(RZ_GameModeComponent gm, int playerId, int dropIndex)
	{
		RZ_LeaderJob.TownInfection(gm, m_vHome, -3);
		gm.NotifyPlayer(playerId, m_sTitle, "A man in a red scarf weighs the sack in one hand and tosses you a paper bag of pills. \"Pleasure. Tell your boss the price goes up next time.\"");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnExposed(RZ_GameModeComponent gm, int playerId)
	{
		gm.GetSafehouses().AddFood(m_iOwn, FOOD);
	}

	//------------------------------------------------------------------------------------------------
	//! Never delivered: the sack goes back on the shelf.
	override void OnFailed(RZ_GameModeComponent gm)
	{
		if (gm.GetSafehouses())
			gm.GetSafehouses().AddFood(m_iOwn, FOOD);
	}

	//------------------------------------------------------------------------------------------------
	override void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
		if (!m_bExposed)
			RZ_LeaderJob.CrossFame(gm, playerId, RZ_EFaction.BANDITS, 3);
	}
}

//------------------------------------------------------------------------------------------------
//! Rogue. A list of "troublemakers" for Bastion, in exchange for leaving the safehouse alone.
//! The people on it (players the Haven turned out, if any are online) get Bastion Infamy.
class RZ_Job_NamesForBastion : RZ_LeaderDelivery
{
	protected static const int LIST_INFAMY = 5;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.SURVIVORS, RZ_ELoyalty.ROGUE, RZ_Leaders.TONE_DARK | RZ_Leaders.TONE_CROSS, 0, 30);
	}

	//------------------------------------------------------------------------------------------------
	override float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		if (!CanRun(gm, RZ_EFaction.MILITARY))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupJob(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		vector meet;
		string meetName;
		if (!RZ_LeaderJob.MeetingPoint(gm, RZ_EFaction.MILITARY, l.m_vPos, meet, meetName))
			return false;

		if (!SpawnItemAtSeat(gm) || !AddDrop(gm, RZ_EFaction.MILITARY, meet, meetName))
			return false;

		AddExposeDrop(gm);
		m_sTitle = "Names for Bastion";
		m_sBrief = string.Format("Bastion leaves alone the safehouses that make themselves useful. I've written a list: people who cause trouble, people the Haven turned out. Take it to %1. It keeps our door off their list. Don't read it.",
			meetName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnJobDelivered(RZ_GameModeComponent gm, int playerId, int dropIndex)
	{
		gm.NotifyPlayer(playerId, m_sTitle, "An officer reads the list slowly, folds it into a breast pocket and thanks you. You didn't read it. You recognise some of the names anyway.");

		array<int> all = {};
		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int other : all)
		{
			if (other == playerId || !gm.GetSafehouses() || !gm.GetSafehouses().IsExiled(other))
				continue;

			gm.AddInfamy(other, RZ_EFaction.MILITARY, LIST_INFAMY);
			gm.NotifyPlayer(other, RZ_Factions.GetName(RZ_EFaction.MILITARY), string.Format("Bastion has your name now. Someone at a safehouse put it on a list. (+%1 Infamy)", LIST_INFAMY));
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
		if (!m_bExposed)
			RZ_LeaderJob.CrossFame(gm, playerId, RZ_EFaction.MILITARY, 3);
	}
}

//------------------------------------------------------------------------------------------------
//! Rogue. A family at the edge of town asks to come in. Turn them away (paid; they take to
//! the roads), or let them in anyway (+3 people, no pay, and the council head remembers).
class RZ_Job_ShutTheDoor : RZ_LeaderPuzzle
{
	protected int m_iTalk = -1;
	protected vector m_vHome;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.SURVIVORS, RZ_ELoyalty.ROGUE, RZ_Leaders.TONE_DARK, 0, 25);
	}

	//------------------------------------------------------------------------------------------------
	override float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		if (leader.m_iSafehouse < 0)
			return 0;

		return super.GetLeaderWeight(gm, leader);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		if (!l)
			return false;

		m_vHome = l.m_vPos;
		m_vPos = LandPointNear(l.m_vPos, 80, 150);
		m_iSettlement = gm.FindNearestSettlement(m_vPos);
		RZ_Settlement from = gm.GetSettlement(PickSettlementAway(gm, l.m_vPos, 2000, 8000, 0, 100));
		string town = "the coast";
		if (from)
			town = from.m_sName;

		m_iTalk = AddClue("Newcomers", string.Format("\"We walked from %1. Four days. People said the Haven takes anyone in.\" A woman, two children, and a man who keeps his left hand in his pocket.", town));
		SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPos, 0, "", "Turn them away|Let them in anyway", m_iTalk, "Talk to the newcomers");

		m_sTitle = "Shut the Door";
		m_sBrief = "There's a family at the edge of town asking to come in. We can't feed who we have. The council would let them in and let us all starve politely. Tell them to keep walking. Be kind about it, if you can.";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0)
			return;

		ChosenBy(playerId);
		if (option == 0)
		{
			if (gm.GetWanderers())
				gm.GetWanderers().AddFromLeaver(m_vPos);

			gm.NotifyPlayer(playerId, m_sTitle, "You tell them there's no room. The woman says thank you, which is worse. They walk on toward the next town.");
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (option == 1)
		{
			RZ_LeaderJob.AddPeople(gm, m_vHome, 3);
			m_fRewardScale = 0;
			m_bNoLeaderCredit = true;
			m_bRogueJob = false;
			if (gm.GetLeaders())
				gm.GetLeaders().Remember(m_iLeaderId, playerId, RZ_Leaders.MEM_CROSSED);

			gm.NotifyPlayer(playerId, m_sTitle, "You walk them in through the front door, past the council head, who says nothing at all. The children get the blankets by the stove.");
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}
}

//------------------------------------------------------------------------------------------------
//! Rogue. Move the stash the council head kept back before the stronghold's count (20 min).
//! Fail with someone on it and the count finds it: the leader is discovered. Turn it in
//! and the stash goes back into the stores (+5 food).
class RZ_Job_HiddenStores : RZ_LeaderDelivery
{
	protected int m_iOwn = -1;
	protected string m_sOwn;

	protected static const int STASH_FOOD = 5;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.SURVIVORS, RZ_ELoyalty.ROGUE, RZ_Leaders.TONE_DARK, 0, 30);
		m_fTimeLeft = 1200;
	}

	//------------------------------------------------------------------------------------------------
	override float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		if (!CanRun(gm, RZ_EFaction.SURVIVORS) || leader.m_iSafehouse < 0)
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupJob(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		if (!SpawnItemAtSeat(gm))
			return false;

		if (!AddDrop(gm, RZ_EFaction.SURVIVORS, LandPointNear(l.m_vPos, 300, 700), "the hidden spot"))
			return false;

		AddExposeDrop(gm);
		m_iOwn = l.m_iSafehouse;
		m_sOwn = l.m_sPlace;
		string auditor = "the stronghold";
		RZ_Leader top = RZ_LeaderJob.Stronghold(gm, RZ_EFaction.SURVIVORS);
		if (top)
			auditor = top.m_sPlace;

		m_sTitle = "Hidden Stores";
		m_sBrief = string.Format("Someone from %1 is coming to count our stores. They'll find what I put aside for the winter, and they'll take it 'for the network'. Move it to the spot I've marked before they get here. Twenty minutes.",
			auditor);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnJobDelivered(RZ_GameModeComponent gm, int playerId, int dropIndex)
	{
		gm.NotifyPlayer(playerId, m_sTitle, "You push the sacks under a tarp in the ditch and kick leaves over it. When the counter arrives, the shelves look exactly as thin as they should.");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnExposed(RZ_GameModeComponent gm, int playerId)
	{
		gm.GetSafehouses().AddFood(m_iOwn, STASH_FOOD);
	}

	//------------------------------------------------------------------------------------------------
	//! The count turned it up (only if someone was carrying it: nobody took the job, nobody moved it).
	override void OnFailed(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		if (!l || m_aParticipants.IsEmpty() || m_bExposed)
			return;

		gm.GetSafehouses().AddFood(m_iOwn, STASH_FOOD);
		gm.GetLeaders().Discover(l, null, string.Format("The council's count in %1 turned up hidden stores.", m_sOwn));
	}
}
