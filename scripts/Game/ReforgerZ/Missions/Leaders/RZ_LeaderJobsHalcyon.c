//! Halcyon leader jobs (docs/MISSIONS.md, "Leader missions"): from field leads who do science
//! their own way (Independent) or behind the director's back (Rogue).

//------------------------------------------------------------------------------------------------
//! Independent. Take the field lead's notes to a Haven medic for a second opinion: +1 research,
//! a little Haven Fame.
class RZ_Job_SecondOpinion : RZ_LeaderDelivery
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.INSTITUTE, RZ_ELoyalty.INDEPENDENT, RZ_Leaders.TONE_HOPEFUL | RZ_Leaders.TONE_CROSS, 6, 20);
	}

	//------------------------------------------------------------------------------------------------
	override float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		if (!CanRun(gm, RZ_EFaction.SURVIVORS) || RZ_LeaderJob.SafehouseNear(gm, leader.m_vPos, 300, 7000, -1) < 0)
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupJob(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		RZ_Safehouses houses = gm.GetSafehouses();

		// A safehouse with a medic if there is one in reach, else the nearest
		int house = -1;
		float best = 7000;
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse c = houses.Get(i);
			float d = RZ_Util.DistXZ(c.m_vPos, l.m_vPos);
			if (c.HasMedic() && d >= 300 && d < best)
			{
				best = d;
				house = i;
			}
		}

		if (house < 0)
			house = RZ_LeaderJob.SafehouseNear(gm, l.m_vPos, 300, 7000, -1);

		RZ_Safehouse h = houses.Get(house);
		if (!h || !SpawnItemAtSeat(gm))
			return false;

		if (!AddDrop(gm, RZ_EFaction.SURVIVORS, LandPointNear(h.m_vPos, 3, 8), "the medic at the safehouse in " + h.m_sName))
			return false;

		m_sTitle = "Second Opinion";
		m_sBrief = string.Format("Our results disagree with themselves. The Haven's medic in %1 has seen more bites than anyone in this building. Take his my notes and the slides. Ask him what he sees.",
			h.m_sName);
		m_sObjective = string.Format("Take the notes and slides to the Haven medic in %1.", h.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnJobDelivered(RZ_GameModeComponent gm, int playerId, int dropIndex)
	{
		gm.GetCure().AddResearch(1, "A Haven medic looked at the lab's slides and said what Halcyon had been getting wrong. A field lead wrote it all down.");
		gm.NotifyPlayer(playerId, m_sTitle, "The medic reads for an hour without a word, then writes two lines in the margin. \"Tell your doctor they're measuring the wrong thing.\"");
	}

	//------------------------------------------------------------------------------------------------
	override void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
		RZ_LeaderJob.CrossFame(gm, playerId, RZ_EFaction.SURVIVORS, 3);
	}
}

//------------------------------------------------------------------------------------------------
//! Independent. A dying man in a town. Sit with him (Haven Fame, no research), or take the
//! sample the protocol wants (+1 research, half pay, and the field lead remembers).
class RZ_Job_PatientsNotSamples : RZ_LeaderPuzzle
{
	protected int m_iTalk = -1;
	protected bool m_bCared;
	protected string m_sPatient;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.INSTITUTE, RZ_ELoyalty.INDEPENDENT, RZ_Leaders.TONE_HOPEFUL, 8, 15);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		if (!l)
			return false;

		m_iSettlement = PickSettlementAway(gm, l.m_vPos, 800, 3000, 20, 80);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.2, s.m_fRadius * 0.7);
		m_sPatient = RZ_NoteText.FirstName(m_Rng);
		m_iTalk = AddClue(m_sPatient, string.Format("\"Are you the doctor? No. That's all right.\" %1 is grey and soaked through, and very polite about it. \"Could you stay a bit? It's the waiting I don't like.\"", m_sPatient));
		SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPos, 0, "", "Sit with him until the end|Take the sample now", m_iTalk, "Talk to " + m_sPatient);

		m_sTitle = "Patients Not Samples";
		m_sBrief = string.Format("There's a man dying in %1. The protocol says collect tissue at the moment of turning. The protocol was written by people who never had to look at him. Go and sit with him. Make it easier. If you take a sample instead, I will know.",
			s.m_sName);
		m_sObjective = string.Format("Sit with the dying man in %1.", s.m_sName);
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
			m_bCared = true;
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("You stay. %1 talks about his allotment, and his wife's terrible singing, and then he stops talking. You do what has to be done after.", m_sPatient));
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (option == 1)
		{
			gm.GetCure().AddResearch(1, "A tissue sample taken at the moment of turning reached Halcyon, as the protocol asks. Somebody had to be there with the kit.");
			m_fRewardScale = 0.5;
			m_bNoLeaderCredit = true;
			if (gm.GetLeaders())
				gm.GetLeaders().Remember(m_iLeaderId, playerId, RZ_Leaders.MEM_CROSSED);

			gm.NotifyPlayer(playerId, m_sTitle, "You do it properly, by the protocol. The sample is clean. He watches you the whole time.");
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
		if (m_bCared)
			RZ_LeaderJob.CrossFame(gm, playerId, RZ_EFaction.SURVIVORS, 3);
	}
}

//------------------------------------------------------------------------------------------------
//! Independent. A dead researcher's notebook: half research, half what he thought of the
//! director. To the field lead (+1 research) or to the stronghold (+2 research, more Halcyon
//! Fame, and the field lead doesn't forgive it).
class RZ_Job_TheNotebook : RZ_LeaderDelivery
{
	protected string m_sAuthor;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.INSTITUTE, RZ_ELoyalty.INDEPENDENT, RZ_Leaders.TONE_TALK, 6, 20);
	}

	//------------------------------------------------------------------------------------------------
	override float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		RZ_Leader top = RZ_LeaderJob.Stronghold(gm, RZ_EFaction.INSTITUTE);
		if (!CanRun(gm, RZ_EFaction.INSTITUTE) || !top || top == leader)
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupJob(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		RZ_Leader top = RZ_LeaderJob.Stronghold(gm, RZ_EFaction.INSTITUTE);
		if (!top)
			return false;

		m_iSettlement = PickSettlementAway(gm, l.m_vPos, 1000, 3500, 20, 90);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.6);
		if (!SpawnItem(gm, m_vPos))
			return false;

		if (!AddDrop(gm, RZ_EFaction.INSTITUTE, LandPointNear(l.m_vPos, 6, 12), string.Format("%1, %2", l.m_sShort, l.m_sPlace)))
			return false;

		if (!AddDrop(gm, RZ_EFaction.INSTITUTE, LandPointNear(top.m_vPos, 6, 12), "the director at " + top.m_sPlace))
			return false;

		SpawnGuards(gm, m_vPos, 3, 10, 40);
		RZ_Rng rng = new RZ_Rng(m_iId * 7919 + 29);
		m_sAuthor = "Dr " + RZ_NoteText.Surname(rng);
		m_sTitle = "The Notebook";
		m_sBrief = string.Format("%1 died in %2 with his notebook on him. Half of it is research. Half of it is what he thought of the director. Bring it to me first. The director will want it too. All of it.",
			m_sAuthor, s.m_sName);
		m_sObjective = string.Format("Bring %1's notebook from %2 to {leader:%3}.", m_sAuthor, s.m_sName, m_iLeaderId);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnJobDelivered(RZ_GameModeComponent gm, int playerId, int dropIndex)
	{
		RZ_Leader l = GetLeader(gm);
		if (dropIndex == 0)
		{
			gm.GetCure().AddResearch(1, m_sAuthor + "'s notebook reached a Halcyon field lead, the pages about the director included.");
			gm.NotifyPlayer(playerId, m_sTitle, "Some of the pages come out and go into the stove. The rest go into the research. Nobody at the lab will ever read what he thought.");
			return;
		}

		gm.GetCure().AddResearch(2, m_sAuthor + "'s notebook reached the lab, to be read into the record page by page.");
		m_iFameReward = 10;
		m_bNoLeaderCredit = true;
		if (l)
			l.Remember(gm.GetProfile(playerId).m_sIdentity, RZ_Leaders.MEM_CROSSED);

		gm.NotifyPlayer(playerId, m_sTitle, "The director reads every page, including those ones, and thanks you without a flicker. The field lead will hear by tonight who brought it in.");
	}
}

//------------------------------------------------------------------------------------------------
//! Rogue. A dying volunteer and an untested serum. Give it: half the time he lives (+2
//! research, he's saved), half the time he dies and turns (+1 research, a special if anyone
//! is near). Refuse: the job fails.
class RZ_Job_UnsanctionedTrial : RZ_LeaderPuzzle
{
	protected int m_iTalk = -1;
	protected string m_sVolunteer;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.INSTITUTE, RZ_ELoyalty.ROGUE, RZ_Leaders.TONE_DARK, 0, 30);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		if (!l)
			return false;

		m_iSettlement = PickSettlementAway(gm, l.m_vPos, 600, 2500, 10, 80);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.2, s.m_fRadius * 0.7);
		m_sVolunteer = RZ_NoteText.FirstName(m_Rng);
		m_iTalk = AddClue(m_sVolunteer, string.Format("\"You've got it? Good.\" %1 has the consent form folded in his shirt pocket, signed twice in case once didn't count. \"Either it works or I was going anyway. Go on.\"", m_sVolunteer));
		SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPos, 0, "", "Give him the serum|Refuse", m_iTalk, "Talk to " + m_sVolunteer);

		m_sTitle = "Unsanctioned Trial";
		m_sBrief = string.Format("%1 in %2 is bitten and dying, and he's asked for this. Signed it, even. The serum is untested and the director said no. Inject him. Write down everything. If it works, nobody will ask how.",
			m_sVolunteer, s.m_sName);
		m_sObjective = string.Format("Inject %1 in %2 with the untested serum.", m_sVolunteer, s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0)
			return;

		ChosenBy(playerId);
		if (option == 1)
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("You put the case down. %1 looks at it for a long time, then at you. \"Coward,\" he says, kindly.", m_sVolunteer));
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (option != 0)
			return;

		if (Math.RandomFloat01() < 0.5)
		{
			gm.GetCure().AddResearch(2, "A bitten man took an untested serum the director had refused, and lived. The notes are on a field lead's desk.");
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("Six hours of fever. Then %1 sits up and asks for water. You write down everything, like you were told.", m_sVolunteer));
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		gm.GetCure().AddResearch(1, "A bitten man took an untested serum the director had refused, and it didn't take. The notes survived him.");
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		if (RZ_Util.NearestDistance(m_vPos, players) <= 600)
			gm.GetSpawner().SpawnZombie(LandPointNear(m_vPos, 3, 6), true, m_iSettlement, -1);

		gm.NotifyPlayer(playerId, m_sTitle, string.Format("It goes wrong fast. %1 stops being %1 in under a minute, and what gets up is bigger. You have your notes. Run.", m_sVolunteer));
		m_eState = RZ_EMissionState.SUCCEEDED;
	}
}

//------------------------------------------------------------------------------------------------
//! Rogue. Everything Halcyon knows about the bite, on a drive, to a Haven safehouse for
//! nothing: Haven Fame and that town's infection -3.
class RZ_Job_LeakTheData : RZ_LeaderDelivery
{
	protected int m_iHouse = -1;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.INSTITUTE, RZ_ELoyalty.ROGUE, RZ_Leaders.TONE_HOPEFUL | RZ_Leaders.TONE_CROSS, 0, 30);
	}

	//------------------------------------------------------------------------------------------------
	override float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		if (!CanRun(gm, RZ_EFaction.SURVIVORS) || RZ_LeaderJob.SafehouseNear(gm, leader.m_vPos, 300, 7000, -1) < 0)
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupJob(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		m_iHouse = RZ_LeaderJob.SafehouseNear(gm, l.m_vPos, 300, 7000, -1);
		RZ_Safehouse h = gm.GetSafehouses().Get(m_iHouse);
		if (!h || !SpawnItemAtSeat(gm))
			return false;

		if (!AddDrop(gm, RZ_EFaction.SURVIVORS, LandPointNear(h.m_vPos, 3, 8), "the Haven safehouse in " + h.m_sName))
			return false;

		AddExposeDrop(gm);
		m_sTitle = "Leak the Data";
		m_sBrief = string.Format("Everything we know about the bite is on this drive. The director wants to trade it to Bastion for protection. I'd rather the Haven had it for nothing. Take it to the safehouse in %1. Don't let anyone from the lab see you.",
			h.m_sName);
		m_sObjective = string.Format("Take the drive to the safehouse in %1 unseen.", h.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnJobDelivered(RZ_GameModeComponent gm, int playerId, int dropIndex)
	{
		RZ_Safehouse h = gm.GetSafehouses().Get(m_iHouse);
		if (h)
			RZ_LeaderJob.TownInfection(gm, h.m_vPos, -3);

		gm.NotifyPlayer(playerId, m_sTitle, "The medic plugs the drive into a laptop that runs on a car battery. By morning, every safehouse with a radio has the dosing tables.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
		if (!m_bExposed)
			RZ_LeaderJob.CrossFame(gm, playerId, RZ_EFaction.SURVIVORS, 5);
	}
}

//------------------------------------------------------------------------------------------------
//! Rogue. The antidote formula to the Red Hand, for things the lab needs. Red Hand Fame.
class RZ_Job_SellTheFormula : RZ_LeaderDelivery
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.INSTITUTE, RZ_ELoyalty.ROGUE, RZ_Leaders.TONE_DARK | RZ_Leaders.TONE_CROSS, 0, 40);
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
		m_sTitle = "Sell the Formula";
		m_sBrief = string.Format("The antidote formula. Every step. The Red Hand will pay for it in things the lab needs and the director won't ask for. Yes, they'll sell it on. People will be cured either way. The drop is %1.",
			meetName);
		m_sObjective = string.Format("Take the formula to the Red Hand drop at %1.", meetName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnJobDelivered(RZ_GameModeComponent gm, int playerId, int dropIndex)
	{
		gm.NotifyPlayer(playerId, m_sTitle, "The buyer can't read it and doesn't pretend to. \"Someone will.\" He pays in fuel coupons and a promise.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
		if (!m_bExposed)
			RZ_LeaderJob.CrossFame(gm, playerId, RZ_EFaction.BANDITS, 5);
	}
}

//------------------------------------------------------------------------------------------------
//! Private Collection (Rogue): the field lead wants a special alive for their own study, off the
//! lab's books (RZ_LiveCapture). Deliver the case to their seat; or take it to the lab instead and
//! they're turned in (the lab gets the research).
class RZ_Job_PrivateCollection : RZ_PuzzleMission
{
	protected ref RZ_LiveCapture m_Capture = new RZ_LiveCapture();
	protected IEntity m_SeatDrop;
	protected IEntity m_LabDrop;

	protected static const int EXPOSE_FAME = 10;
	protected static const int EXPOSE_FAVOR = 10;

	//------------------------------------------------------------------------------------------------
	//! Caught: it's in its case.
	override protected bool ObjFirstDone()
	{
		return m_Capture.m_eState == RZ_ECaptureState.CRATED || m_Capture.m_eState == RZ_ECaptureState.DELIVERED;
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		m_Capture.GetSpots(result);
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.INSTITUTE, RZ_ELoyalty.ROGUE, RZ_Leaders.TONE_DARK, 0, 45);
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		if (!RZ_LiveCapture.HasRoom(gm) || gm.GetDropPointPrefab(RZ_EFaction.INSTITUTE).IsEmpty())
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Leader l = RZ_LeaderJob.Leader(gm, this);
		if (!l)
			return false;

		m_iSettlement = PickSettlementAway(gm, l.m_vPos, 400, 3500, 40, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_SeatDrop = RZ_Util.SpawnPrefab(gm.GetDropPointPrefab(RZ_EFaction.INSTITUTE), RZ_Util.GroundPos(LandPointNear(l.m_vPos, 4, 8)), Math.RandomFloat(0, 360));
		if (!m_SeatDrop)
			return false;

		TrackProp(m_SeatDrop);
		RZ_Leader top = RZ_LeaderJob.Stronghold(gm, RZ_EFaction.INSTITUTE);
		if (top && top != l)
		{
			m_LabDrop = RZ_Util.SpawnPrefab(gm.GetDropPointPrefab(RZ_EFaction.INSTITUTE), RZ_Util.GroundPos(LandPointNear(top.m_vPos, 6, 12)), Math.RandomFloat(0, 360));
			TrackProp(m_LabDrop);
		}

		vector lair = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.6);
		vector cage = LandPointNear(lair, 60, 120);
		m_vPos = cage;
		m_sTitle = "Private Collection";
		m_sBrief = string.Format("I want one of the specials alive, for me, not for the director's committee. The trap is set outside %1. Bring it here in its case. Nobody at the lab needs to know.", s.m_sName);
		ObjThen(string.Format("Trap a special outside %1.", s.m_sName), string.Format("Bring the case to {leader:%1}.", m_iLeaderId));
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
		if (faction != RZ_EFaction.INSTITUTE || (drop != m_SeatDrop && drop != m_LabDrop) || !m_Capture.Deliver())
			return false;

		m_aParticipants.Clear();
		m_aParticipants.Insert(playerId);
		if (drop == m_LabDrop)
		{
			m_bExposed = true;
			m_bRogueJob = false;
			m_iFameReward = EXPOSE_FAME;
			m_iFavorReward = EXPOSE_FAVOR;
			gm.GetCure().AddResearch(3, "A live special meant for a field lead's own notebook was handed to the lab instead, with an account of who had asked for it.");
			gm.NotifyPlayer(playerId, m_sTitle, "The lab takes the case, and your account of who asked for it.");
		}
		else
		{
			gm.NotifyPlayer(playerId, m_sTitle, "The field lead signs for it in a notebook that isn't the lab's. \"Thank you. This stays between us.\"");
		}

		m_eState = RZ_EMissionState.SUCCEEDED;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Private Collection: " + m_Capture.m_sLostReason;
	}

	//------------------------------------------------------------------------------------------------
	//! The cage, the held special, the case, then the drop at the director's seat.
	override RZ_ObjPlace ObjectivePlace(RZ_GameModeComponent gm, int playerId, vector from)
	{
		vector drop = vector.Zero;
		if (m_SeatDrop)
			drop = m_SeatDrop.GetOrigin();

		RZ_ObjPlace at = m_Capture.ObjPlace(drop);
		if (at)
			return at;

		return super.ObjectivePlace(gm, playerId, from);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Capture.Cleanup();
	}
}
