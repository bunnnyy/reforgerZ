//! Halcyon (Institute) contracts. See docs/MISSIONS.md, "Halcyon".

//------------------------------------------------------------------------------------------------
//! #1 The Lone One (Alpha Sighting, renamed): a lone one when there is one (a leader with no pack,
//! walking alone; Zombies/RZ_TribeLife.c), else today's special in a bad town. Kill it for its
//! samples. A lone one the Haven knew is the Haven's job instead: lay it down by name.
class RZ_Mission_SampleHunt : RZ_TribeContract
{
	protected bool m_bLone;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.STRANGER, 15, 15);
		m_Req.m_bNotCured = true;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 2.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_sTitle = "The Lone One";
		RZ_Tribes tribes = RZ_TribeContract.Tribes(gm);
		if (tribes)
		{
			foreach (RZ_Tribe r : tribes.GetRovers())
			{
				if (r.m_eKind != RZ_ETribeKind.LONE || !ClaimSubject(gm, RZ_TribeContract.Claim(r.m_iId)))
					continue;

				m_iTribe = r.m_iId;
				m_bLone = true;
				m_vPos = r.m_vLair;
				m_fParticipationRadius = 300;
				RZ_TribePart body = r.FindPart(RZ_ETribePart.ALPHA);
				if (body)
					m_vPos = body.m_vPos;

				bool known = !r.m_sAlphaName.IsEmpty() && (r.m_iSeed + r.m_sAlphaName.Length()) % 3 == 0;
				if (known)
				{
					// The Haven knew who it was: lay it down by name
					m_eGiver = RZ_EFaction.SURVIVORS;
					m_Req.m_eGiver = RZ_EFaction.SURVIVORS;
					m_sBrief = string.Format("%1 walks alone now, out past %2. We knew %1. Lay %1 down, please, and tell us when it's done.", r.m_sAlphaName, r.m_sPlace);
					return true;
				}

				m_sBrief = string.Format("We've been following an isolate out past %1: a big one, on its own, with no pack round it. Kill it and bring its samples to the lab.", r.m_sPlace);
				return true;
			}
		}

		m_iSettlement = PickEmptySettlement(gm, 40, 100, this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.8);
		m_fParticipationRadius = s.m_fRadius + 100;
		m_sBrief = string.Format("We've tracked a big one to %1, on its own, further along than anything round it. The furthest along we've seen. Kill it and bring its samples to the lab while they're fresh.", s.m_sName);

		RZ_TrackedZombie alpha = gm.GetSpawner().SpawnZombie(m_vPos, true, m_iSettlement, m_iId);
		return alpha != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bLone)
			return;

		RZ_Tribes tribes = RZ_TribeContract.Tribes(gm);
		RZ_Tribe r;
		if (tribes)
			r = tribes.Find(m_iTribe);

		if (!r)
		{
			Withdraw("It's gone: died out there on its own, or found others.");
			return;
		}

		RZ_TribePart body = r.FindPart(RZ_ETribePart.ALPHA);
		if (body)
			m_vPos = body.m_vPos;
	}

	//------------------------------------------------------------------------------------------------
	override void OnTribeKill(RZ_GameModeComponent gm, RZ_Tribe t, RZ_EInfectedRole role, RZ_TribePart p, vector at, bool silent)
	{
		if (!m_bLone || t.m_iId != m_iTribe || role != RZ_EInfectedRole.ALPHA)
			return;

		m_vPos = at;
		AddParticipant(NearestPlayer(at, 300));
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnZombiesKilled(RZ_GameModeComponent gm, int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		if (!m_bLone && missionId == m_iId && special)
		{
			m_vPos = pos;
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		// The kill already dropped its samples (a lone one 2, a special 1); a town's special is worth more
		if (!m_bLone)
			gm.SpawnSamples(m_vPos, 2);
	}
}

//------------------------------------------------------------------------------------------------
//! #2 Field Samples: kill infected in an infested town while Halcyon's people
//! take readings. Kills only count while someone eligible is in town. Amends:
//! works off Halcyon Infamy. ★ Lab Tech/Nurse: each kill counts double.
class RZ_Mission_FieldSamples : RZ_Mission
{
	protected int m_iTaken;
	protected bool m_bPresent;
	protected bool m_bBonusPresent;

	protected static const int NEEDED = 5;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.STRANGER, 12, 15);
		m_bAmends = true;
		m_Req.m_bAllowWatched = true;
		m_Req.m_bNotCured = true;
		m_Req.Bonus("labtech");
		m_Req.Bonus("nurse");
		m_Req.Bonus("doctor");
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickEmptySettlement(gm, 40, 100, this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = s.m_vCenter;
		m_fParticipationRadius = s.m_fRadius;
		m_sTitle = "Field Samples";
		m_sDeed = "brought samples back from {town}";
		m_sBrief = string.Format("The specials get the attention, but the cure has to work on the ordinary ones too: the postman, the farmer, whoever was in %2 when it started. Put down %1 of them there. The lab takes its readings from what's left.", NEEDED, s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_bPresent = false;
		m_bBonusPresent = false;
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), m_vPos) > m_fParticipationRadius || !IsEligible(gm, ids[i]))
				continue;

			m_bPresent = true;
			if (HasBonus(gm, ids[i]))
				m_bBonusPresent = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnZombiesKilled(RZ_GameModeComponent gm, int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		if (settlementIdx != m_iSettlement || !m_bPresent || special)
			return;

		int worth = count;
		if (m_bBonusPresent)
			worth = count * 2;

		m_iTaken += worth;
		if (m_iTaken >= NEEDED)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.GetCure().AddResearch(1.5, "Field samples from " + GetLocationName(gm) + " came in: tissue from the ordinary infected, the ones the cure has to work on most.");
	}
}

//------------------------------------------------------------------------------------------------
//! #3 Water Survey: take water samples from wells in three towns (stand at each
//! for 10 s). Reveals the town most at risk. ★ Lab Tech: +50% Fame.
class RZ_Mission_WaterSurvey : RZ_Mission_VisitPoints
{
	protected int m_iWhere;		// the one choice: 0 Halcyon only, 1 the Haven too

	//------------------------------------------------------------------------------------------------
	void RZ_Mission_WaterSurvey()
	{
		m_Story = new RZ_MissionStory("survey");
		m_Choice = new RZ_OneChoice();
		m_Choice.Set("The samples are in. The analyst reads the strip twice: one town is next. \"Halcyon logs it. Whether the Haven hears is up to whoever carried the bottles.\" Decide at the last point.",
			"Halcyon only|Tell the Haven too", "decide who hears about the town at risk", 0);
		m_Choice.Asset(1, RZ_EFaction.SURVIVORS);
	}

	//------------------------------------------------------------------------------------------------
	override bool AsksChoice()
	{
		return super.AsksChoice() && Type() == RZ_Mission_WaterSurvey;
	}

	//------------------------------------------------------------------------------------------------
	override void OnChosen(RZ_GameModeComponent gm, int decider, int option)
	{
		m_iWhere = option;
		string value = "halcyon";
		if (option == 1)
			value = "haven";

		RZ_StoryKit.Record(this, "warned", RZ_WorldFacts.Town(m_iSettlement), value, m_aParticipants);
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.STRANGER, 15, 15);
		m_Req.Bonus("labtech");
		m_fBonusFameMult = 1.5;
		m_fVisitRadius = 12;
		m_fVisitSeconds = 10;
		m_sPointName = "Well";
		PointWork(RZ_EWork.FILL, "Take a water sample", 10);
		m_fTimeLeft = 3600;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetSettlements().Count() < 3)
			return 0;

		return 1.2;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlementNotHeldBy(gm, 0, 80, RZ_Factions.Bit(RZ_EFaction.BANDITS), this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = s.m_vCenter;
		m_fParticipationRadius = 0;
		AddPoint(LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.4));

		array<int> used = {m_iSettlement};
		int guard = 0;
		while (m_aPoints.Count() < 3 && guard < 20)
		{
			guard++;
			int idx = PickSettlementAwayNotHeldBy(gm, s.m_vCenter, 500, 3500, 0, 100, RZ_Factions.Bit(RZ_EFaction.BANDITS), m_iSettlement, this);
			RZ_Settlement other = gm.GetSettlement(idx);
			if (!other || used.Contains(idx))
				continue;

			used.Insert(idx);
			AddPoint(LandPointNear(other.m_vCenter, 0, other.m_fRadius * 0.4));
		}

		if (m_aPoints.Count() < 3)
			return false;

		m_sTitle = "Water Survey";
		m_sDeed = "tested the water at {town}";
		m_sBrief = string.Format("The Haven boils its water and still gets sick, and the roads say the Red Hand poisons wells. We'd like a number instead of a rumour. Take samples from the wells in %1 and two towns nearby: ten seconds at each well.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		// The town most likely to fall next: the most infected one that hasn't yet
		RZ_Settlement risk;
		foreach (RZ_Settlement s : gm.GetSettlements())
		{
			if (s.m_fInfection >= 50)
				continue;

			if (!risk || s.m_fInfection > risk.m_fInfection)
				risk = s;
		}

		if (risk)
			NotifyParticipants(gm, "Halcyon", "The samples say " + risk.m_sName + " is next. Warn anyone there.");

		// The Haven heard too: someone moves out of a house there, to another
		if (risk && m_iWhere == 1)
			WarnHaven(gm, risk);
	}

	//------------------------------------------------------------------------------------------------
	protected void WarnHaven(RZ_GameModeComponent gm, RZ_Settlement risk)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses)
			return;

		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			if (!h || h.m_iPopulation <= 1 || RZ_Util.DistXZ(h.m_vPos, risk.m_vCenter) > risk.m_fRadius + 300)
				continue;

			int other = houses.FindNearest(h.m_vPos + Vector(1500, 0, 0));
			if (other < 0 || other == i)
				continue;

			houses.RemovePopulation(i, 1);
			houses.AddPopulationAt(other, 1);
			NotifyParticipants(gm, m_sTitle, string.Format("The Haven hears. One family packs up and leaves %1 for %2 before dark.", h.m_sName, houses.Get(other).m_sName));
			return;
		}
	}
}

//------------------------------------------------------------------------------------------------
//! #5 Lost Colleague (story key `colleague`; also The Missing Courier arc): find a missing
//! researcher. The truth, kept per name for 3 season days:
//! - **Hiding, hurt:** alive with his notes and a broken ankle. He asks you not to say he went out
//!   alone: Say nothing, or Tell the field lead. Research +2 either way.
//! - **Turned:** he has to be put down. His notebook is on him: all of it to the lab (+2), or tear out
//!   the last pages he asked to keep back (+1, 60% pay).
//! - **Walked out** (a safehouse within 3 km): his camp is empty and a note points to the house, where
//!   he works on the water under another name. Persuade him back (Trust with Halcyon 2, ★ Lab Tech),
//!   press him (Fear 2, +4 Haven Infamy), trade the house 4 food for his notes (+1) and his tag (say he
//!   turned: 60% pay and a lie a Halcyon-Known visitor to that house finds out, +10 Infamy), or leave
//!   him be (no pay; the filter works).
class RZ_Mission_LostColleague : RZ_PuzzleMission
{
	protected string m_sTruth;
	protected string m_sName;
	protected vector m_vHouse;
	protected string m_sHouse;
	protected bool m_bReached;
	protected bool m_bHouseShown;
	protected bool m_bDone;
	protected string m_sHow;
	protected int m_iResearch = 2;
	protected int m_iHim = -1;
	protected int m_iNote = -1;
	protected int m_iBook = -1;
	protected int m_iSister = -1;
	protected vector m_vFence;				// Sold: the fence's table, by the nearest Red Hand holding
	protected ref array<int> m_aHeard = {};	// ★ Radio Op: told where his set is
	protected ref RZ_Ways m_Ways = new RZ_Ways("Lost Colleague");
	protected ref RZ_StepChain m_Chain = new RZ_StepChain();
	protected ref RZ_Escort m_Escort = new RZ_Escort();
	protected vector m_vHim;				// where he is (the job's spot follows the trail to it)
	protected vector m_vLead;				// his field station or the lab: where he's walked in to
	protected vector m_vTrader;				// the Haven trader who saw "a man with a lab badge"
	protected vector m_vScav;				// a scavenger by the cold box, who sells the last sighting
	protected int m_iPack = -1;
	protected int m_iTrader = -1;
	protected int m_iScav = -1;
	protected bool m_bTrail;				// the trail is followed (or a shortcut taken): his spot is known
	protected bool m_bTraderShown;
	protected bool m_bScavShown;
	protected bool m_bWalking;				// hiding: walked in, under escort
	protected int m_iWalkOption = -1;		// 0 say nothing, 1 tell the field lead
	protected IEntity m_Him;				// hiding: the figure in the back room until he's walked in
	bool m_bCourier;	// The Missing Courier (Director/RZ_StoryArcs.c): set before StartMission so the offer reads right

	protected static const int OBJ_HIM = 0;
	protected static const int OBJ_BOOK = 1;
	protected static const int OBJ_HOUSE = 2;
	protected static const int OBJ_FENCE = 3;
	protected static const int OBJ_SCAV = 4;
	protected static const float LEAD_RANGE = 3000;
	protected static const int BRIBE_FAVOR = 25;
	protected static const int BUY_FOOD = 5;
	protected static const int TRADE_FOOD = 4;
	protected static const int PRESS_INFAMY = 4;
	protected static const int LIE_INFAMY = 10;

	//------------------------------------------------------------------------------------------------
	void RZ_Mission_LostColleague()
	{
		m_Story = new RZ_MissionStory("colleague");
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.KNOWN, 20, 20);
		m_Req.m_bNotCured = true;
		m_fParticipationRadius = 80;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN) || !HasPrefab(gm, RZ_EPuzzleProp.OBJECT))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickEmptySettlement(gm, 20, 100, this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.5, s.m_fRadius + 300);
		m_sName = "Dr " + RZ_NoteText.Surname(m_Rng);

		// The truth, kept per name: walking out needs a house within 3 km; sold needs a Red Hand
		// holding within 5 km (x3 in The Missing Courier)
		array<string> values = {"hiding", "turned", "walked_out", "sold"};
		array<float> weights = {0.35, 0.3, 0, 0};
		RZ_Holding fence = RZ_ErrandCard.HoldingNear(gm, RZ_EFaction.BANDITS, m_vPos, 5000);
		if (fence)
		{
			m_vFence = LandPointNear(fence.m_vPos, 8, 20);
			weights[3] = 0.1;
			if (m_bCourier)
				weights[3] = 0.3;
		}

		int house = -1;
		if (gm.GetSafehouses())
			house = gm.GetSafehouses().FindNearest(m_vPos, 3000);

		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, house);
		if (h)
		{
			m_vHouse = h.m_vPos;
			m_sHouse = h.m_sName;
			weights[2] = 0.25;
			if (RZ_WorldFacts.Get().Memory().FindRecord("weeks.public", "") != null)
				weights[2] = 0.375;
		}

		m_sTruth = m_Story.Truth(m_sName, values, weights, this);
		if ((m_sTruth == "walked_out" && !h) || (m_sTruth == "sold" && !fence))
			m_sTruth = "hiding";

		if (m_sTruth == "hiding")
		{
			m_iHim = AddClue(m_sName, string.Format("%1, in a back room with his notes in a bin bag and his ankle the wrong shape. \"I went out alone. Against orders. Don't tell them that part. Please.\"", m_sName));
			m_Him = SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPos, OBJ_HIM, "", "Walk him in and say nothing|Walk him in and tell the field lead|[Nurse or Doctor] Patch him up first", m_iHim, "Talk to " + m_sName);
			if (!m_Him)
				return false;
		}
		else if (m_sTruth == "sold")
		{
			// His camp says nothing; his sister at the nearest house does, and the notes are on the fence's table
			m_iNote = AddClue("An empty camp", "His bedroll, cut open. His sample case, empty. A crew's toll chit, folded small, in the ash of the fire.");
			if (!SpawnClue(gm, RZ_EPuzzleProp.CLUE, m_vPos, m_iNote, "Look around the camp"))
				return false;

			if (h)
			{
				m_iSister = AddClue("His sister", string.Format("\"He got them from the crew. The doses. For me.\" She holds her arm like it still hurts. \"Don't tell anyone. They'd say he sold %1.\"", "the lab"));
				SpawnClue(gm, RZ_EPuzzleProp.CIVILIAN, LandPointNear(m_vHouse, 6, 14), m_iSister, "Talk to the woman at the door");
			}

			int notes = AddClue("His notes", string.Format("%1's notes, in a crew's ledger box, priced in doses. Three pages already torn out and gone somewhere else.", m_sName));
			SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vFence, OBJ_FENCE, "", "Bribe the fence [25 Red Hand Favor]|Take them off the table at night, unseen|[Known · Red Hand] Buy them back [5 food]|Leave them with the crew", notes, "Look at the fence's table");
		}
		else if (m_sTruth == "walked_out")
		{
			m_iNote = AddClue("An empty camp", string.Format("His bedroll, rolled. His lab badge, gone. A water-filter manual with \"%1\" written in the margin, underlined twice.", m_sHouse));
			if (!SpawnClue(gm, RZ_EPuzzleProp.CLUE, m_vPos, m_iNote, "Look around the camp"))
				return false;
		}

		m_iBook = AddClue("His notebook", string.Format("%1's notebook. The last twenty pages are his own turning, hour by hour, in a hand that gets worse. He was bitten a day before he left. On the last page: \"Don't give them this part.\"", m_sName));
		m_sTitle = "Lost Colleague";
		m_sBrief = string.Format("%1 stopped answering near %2. Find him. He may not be himself any more.", m_sName, s.m_sName);
		if (m_bCourier)
		{
			m_sTitle = "The Missing Courier";
			m_sBrief = string.Format("One of our couriers was walking a steel case of samples between the lab and %1 and missed the last check-in. Couriers go on the list of authors, the same as researchers. Find him, or what's left of him, and the case.", s.m_sName);
		}

		m_vHim = m_vPos;
		Trail(gm, s);
		RZ_Log.Info(string.Format("Story: colleague %1 is %2", m_sName, m_sTruth));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The search (docs/MISSIONS.md, Lost Colleague): a step chain from his field station (his cold
	//! box, then his pack), and two shortcuts: a scavenger by the cold box sells the last sighting for
	//! a tin, and a Haven trader in the nearest lived-in town saw "a man with a lab badge". Until the
	//! trail is followed the job's spot is the cold box.
	protected void Trail(RZ_GameModeComponent gm, RZ_Settlement s)
	{
		vector station;
		RZ_Holding post = RZ_ErrandCard.HoldingNear(gm, RZ_EFaction.INSTITUTE, m_vHim, 4000);
		if (post)
			station = post.m_vPos;

		if (gm.HasLab() && (!post || RZ_Util.DistXZ(gm.GetLabPosition(), m_vHim) < RZ_Util.DistXZ(station, m_vHim)))
			station = gm.GetLabPosition();

		if (station != vector.Zero && RZ_Util.DistXZ(station, m_vHim) <= LEAD_RANGE)
			m_vLead = LandPointNear(station, 6, 14);

		// The cold box: on his way out from the station, else beyond the town's edge
		vector box;
		if (station != vector.Zero && RZ_Util.DistXZ(station, m_vHim) > 900)
			box = LandPointNear(station + (m_vHim - station) * 0.25, 0, 60);
		else
			box = LandPointNear(s.m_vCenter, s.m_fRadius + 350, s.m_fRadius + 600);

		m_Chain.SetMission(m_iId);
		RZ_ChainStep first = m_Chain.AddStep(gm, gm.FindNearestSettlement(box), box, 8, 0);
		first.m_iClue = AddClue("His cold box", string.Format("A Halcyon cold box in the grass, latched, the ice packs gone soft. %1's number on the lid. Boot prints go on toward %2.", m_sName, s.m_sName));
		first.m_sReadLabel = "Look at the cold box";

		vector pack = LandPointNear(box + (m_vHim - box) * 0.6, 0, 50);
		RZ_ChainStep second = m_Chain.AddStep(gm, gm.FindNearestSettlement(pack), pack, 8, 0);
		m_iPack = AddClue("His pack", string.Format("His pack, dropped, one strap torn. A sample log with the last entry two days old, and a pencilled grid: %1.", RZ_SettlementScanner.GridReference(m_vHim)));
		second.m_iClue = m_iPack;
		second.m_sReadLabel = "Go through the pack";

		m_vScav = LandPointNear(box, 30, 60);
		m_iScav = AddClue("A scavenger", "\"Lab coat? Saw one. Limping, or walking like a man who wished he wasn't.\" He looks at your pack, not at you. \"Memory's better with a tin in it.\"");

		string sighting = "A man with a lab badge. Bought matches and asked nothing. Went out past the last houses.";
		if (m_sTruth == "walked_out")
			sighting = string.Format("A man with a lab badge. Asked the way to %1, and whether they needed hands.", m_sHouse);
		else if (m_sTruth == "sold")
			sighting = "A man with a lab badge, with one of the crew's lads. Nobody walks with them by choice.";

		int town = PickSettlementAway(gm, m_vHim, 300, 2500, 0, 60);
		RZ_Settlement t = gm.GetSettlement(town);
		if (t && RZ_WorldFacts.Get().IsLivedIn(town))
		{
			m_vTrader = LandPointNear(t.m_vCenter, 0, t.m_fRadius * 0.4);
			m_iTrader = AddClue("A Haven trader", sighting + string.Format(" She points: grid %1.", RZ_SettlementScanner.GridReference(m_vHim)));
		}

		m_vPos = box;
	}

	//------------------------------------------------------------------------------------------------
	//! The trail is followed, or a shortcut taken: the job's spot is where he is now.
	protected void FoundTrail(RZ_GameModeComponent gm, int playerId, string how)
	{
		if (m_bTrail)
			return;

		m_bTrail = true;
		if (m_vPos != m_vHouse)
			m_vPos = m_vHim;

		RZ_Log.Info(string.Format("Story: colleague %1 trail found by %2 (%3)", m_sName, RZ_Util.PlayerName(playerId), how));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		// ★ Radio Op: his set is still on, off the net, a shortcut to him
		foreach (int i, IEntity p : players)
		{
			if (m_aHeard.Contains(ids[i]) || m_bDone || RZ_Util.DistXZ(p.GetOrigin(), m_vHim) > 1500)
				continue;

			m_aHeard.Insert(ids[i]);
			if (RZ_StoryKit.Read(gm, this, ids[i], "radioham", string.Format("His set's still on, off the net, keying every few minutes. Somewhere near grid %1.", RZ_SettlementScanner.GridReference(m_vHim))))
				FoundTrail(gm, ids[i], "his set");
		}

		if (!m_bTrail && !m_Chain.IsDone())
			m_Chain.Update(gm, dt, ids, players, m_aParticipants);

		if (!m_bScavShown && !m_bTrail && RZ_ContractHelp.AnyNear(players, m_vScav, 300))
		{
			m_bScavShown = true;
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vScav, OBJ_SCAV, "", "Give him a tin for it [1 food]|Leave him to it", m_iScav, "Talk to the scavenger");
		}

		if (!m_bTraderShown && m_iTrader >= 0 && RZ_ContractHelp.AnyNear(players, m_vTrader, 300))
		{
			m_bTraderShown = true;
			SpawnClue(gm, RZ_EPuzzleProp.CIVILIAN, m_vTrader, m_iTrader, "Talk to the trader");
		}

		if (m_bWalking)
		{
			WalkIn(gm, dt, ids, players);
			return;
		}

		if (m_sTruth == "walked_out" && !m_bHouseShown && RZ_ContractHelp.AnyNear(players, m_vHouse, 300))
		{
			m_bHouseShown = true;
			int him = AddClue("The new man", "\"We've a new man. Good with pumps,\" they said at the door. He's at the filter, sleeves up, and he stops when he sees you look at his hands.");
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, LandPointNear(m_vHouse, 6, 14), OBJ_HOUSE, "", "[Trusted · Halcyon] Persuade him back|[Feared] Press him back|Trade the house for his notes [4 food]|Take his tag: tell them he turned|Leave him be", him, "Talk to the new man");
		}

		if (m_sTruth != "turned" || m_bReached || CountPlayersNear(players, m_vHim, 20) == 0)
			return;

		m_bReached = true;
		NotifyParticipants(gm, m_sTitle, "You're too late. He's turned. Put him down; his notebook is still on him.");
		if (!gm.GetSpawner().SpawnZombie(RZ_Util.RandomPointInRing(m_vHim, 8, 15), true, m_iSettlement, m_iId))
			ShowBook(gm, m_vHim);
	}

	//------------------------------------------------------------------------------------------------
	//! Hiding: walked in to his field station (or the lab) under escort. Dead on the way: the job
	//! fails, and his notebook is where he fell (Notebook 31).
	protected void WalkIn(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_Escort.Update(gm, dt, ids, players);
		m_vPos = m_Escort.m_vPos;
		RZ_ContractHelp.AddNear(ids, players, m_Escort.m_vPos, 100, m_aParticipants);
		if (m_Escort.IsDead())
		{
			m_bWalking = false;
			m_vHim = m_Escort.m_vPos;
			NotifyParticipants(gm, m_sTitle, string.Format("%1 doesn't make it. His notes are in the bin bag where he fell.", m_sName));
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (!m_Escort.IsNear(m_vLead, 25))
			return;

		m_Escort.Arrive();
		m_bWalking = false;
		int walker = m_Escort.GetFollowed();
		if (walker <= 0 && !m_aParticipants.IsEmpty())
			walker = m_aParticipants[0];

		if (m_iWalkOption == 0)
			Finish(gm, walker, 2, 1, "lab", string.Format("\"Thank you.\" At the gate %1 tells them he got turned round on his rounds. Nobody asks you.", m_sName));
		else
			Finish(gm, walker, 2, 1, "told", string.Format("You tell the field lead the rest. She reads your face, then his. %1 won't hold a field post again.", m_sName));
	}

	//------------------------------------------------------------------------------------------------
	//! Hiding, Say nothing or Tell: walk him in, an escort to his field station or the lab within 3 km.
	//! No room for an escort, or nowhere near: Halcyon sends a stretcher, as before.
	protected void StartWalk(RZ_GameModeComponent gm, int playerId, int option)
	{
		bool walk = m_vLead != vector.Zero && RZ_Escort.HasRoom(gm);
		if (walk)
		{
			m_Escort.m_sName = m_sName;
			m_Escort.m_sPrefab = RZ_Escort.PrefabFor(gm, 1);
			m_Escort.m_bHide = false;
			walk = m_Escort.Begin(gm, m_vHim);
		}

		if (!walk)
		{
			if (option == 0)
				Finish(gm, playerId, 2, 1, "lab", string.Format("\"Thank you.\" Halcyon sends a stretcher for %1. The report says he was found on his rounds.", m_sName));
			else
				Finish(gm, playerId, 2, 1, "told", string.Format("Halcyon sends a stretcher. The field lead reads your report twice. %1 won't hold a field post again.", m_sName));

			return;
		}

		RZ_Util.DeleteUnit(m_Him);
		m_Him = null;
		m_Escort.SetDestination(m_vLead);
		m_Escort.Follow(playerId);
		m_Escort.Go();
		m_bWalking = true;
		m_iWalkOption = option;
		AddParticipant(playerId);
		if (m_fTimeLeft < 1200)
			m_fTimeLeft = 1200;

		gm.NotifyPlayer(playerId, m_sTitle, string.Format("He gets an arm round your shoulders. \"Slowly. It's my ankle you're walking on.\" Walk him in to his people, %1.", RZ_Util.DirectionText(m_vHim, m_vLead)));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		m_Chain.OnClueRead(idx);
		if (idx == m_iPack || idx == m_iTrader)
			FoundTrail(gm, playerId, "trail");

		if (idx == m_iNote)
		{
			m_Story.Reveal(gm, this, m_sName, playerId, "camp", string.Format("He walked out. The trail leads to the Haven house in %1.", m_sHouse));
			m_vPos = m_vHouse;
		}

		if (idx == m_iNote && m_sTruth == "sold")
		{
			m_Story.Reveal(gm, this, m_sName, playerId, "camp", "A crew's chit in his fire. He didn't turn and he didn't walk: he traded.");
			RZ_StoryKit.Read(gm, this, playerId, "excon", "That chit's a fence's. Notes for doses, by the look of the sums.");
		}

		if (idx == m_iSister)
			m_Story.Reveal(gm, this, m_sName, playerId, "sister", string.Format("%1 sold his notes to a crew for doses for his sister. They're on the fence's table.", m_sName));

		if (idx == m_iHim)
			RZ_StoryKit.Read(gm, this, playerId, "nurse,doctor", "A clean break, splinted badly. He'll walk on it in a week if nobody lets him try sooner.");

		if (idx == m_iBook)
			RZ_StoryKit.Read(gm, this, playerId, "nurse,doctor", "The bite on the body matches the notebook's first page. He knew before he left.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnZombiesKilled(RZ_GameModeComponent gm, int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		if (missionId == m_iId && m_sTruth == "turned")
			ShowBook(gm, pos);
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowBook(RZ_GameModeComponent gm, vector pos)
	{
		if (m_bDone)
			return;

		SpawnObject(gm, RZ_EPuzzleProp.OBJECT, LandPointNear(pos, 0, 2), OBJ_BOOK, "", "Take the whole notebook to the lab|Tear out the last pages|Give it to a Haven medic", m_iBook, "Read his notebook");
		RZ_ContractHelp.NotifyNear(gm, pos, 60, m_sTitle, "His notebook is on him. Read it before you decide what goes to the lab.");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (m_bDone || !RZ_StoryKit.MayDecide(gm, this, playerId))
			return;

		if (objIdx == OBJ_HIM)
		{
			if (option == 2)
			{
				if (!RZ_QuestApproach.HasBackground(gm, playerId, "nurse,doctor"))
				{
					gm.NotifyPlayer(playerId, m_sTitle, "You'd make it worse. That needs someone who knows how.");
					return;
				}

				RZ_OneChoice.Morale(gm, RZ_EFaction.INSTITUTE, 1, "a researcher patched and walking");
				Finish(gm, playerId, 2, 1, "patched", string.Format("You set it properly and bind it. %1 walks back on his own with a stick, his notes under his arm, and tells the field lead he got lost.", m_sName));
				return;
			}

			if (!m_bWalking)
				StartWalk(gm, playerId, option);

			return;
		}

		if (objIdx == OBJ_SCAV)
		{
			OnScavenger(gm, playerId, option);
			return;
		}

		if (objIdx == OBJ_BOOK)
		{
			if (option == 0)
				Finish(gm, playerId, 2, 1, "turned", "The whole notebook goes to the lab, last pages and all. Someone there will read every word.");
			else if (option == 1)
				Finish(gm, playerId, 1, 0.6, "torn", "You tear out the last pages and burn them. The rest goes to the lab. It's still worth something. Less than it was.");
			else
				ToMedic(gm, playerId);

			return;
		}

		if (objIdx == OBJ_FENCE)
		{
			OnFenceChoice(gm, playerId, option);
			return;
		}

		if (objIdx == OBJ_HOUSE)
			OnHouseChoice(gm, playerId, option, source);
	}

	//------------------------------------------------------------------------------------------------
	//! The scavenger by the cold box: a tin for the last sighting, a shortcut past the pack.
	protected void OnScavenger(RZ_GameModeComponent gm, int playerId, int option)
	{
		if (option != 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "Suit yourself. He's not getting any less lost.");
			return;
		}

		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!me || !RZ_StoryStats.TakeFood(gm, me, 1))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "A tin. A real one, with food in it.");
			return;
		}

		FoundTrail(gm, playerId, "scavenger");
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("He pockets it. \"Grid %1. Two nights back. He was talking to himself, so I didn't.\"", RZ_SettlementScanner.GridReference(m_vHim)));
	}

	//------------------------------------------------------------------------------------------------
	//! Turned: the notebook to a Haven medic instead. No research and nobody pays; the medic's town
	//! breathes a little easier, and the Haven has its proof.
	protected void ToMedic(RZ_GameModeComponent gm, int playerId)
	{
		string refusal = RZ_RouteAssets.Refuse(gm, playerId, RZ_EFaction.SURVIVORS);
		if (!refusal.IsEmpty())
		{
			gm.NotifyPlayer(playerId, m_sTitle, refusal);
			return;
		}

		RZ_RouteAssets.Add(gm, playerId, RZ_EFaction.SURVIVORS, m_sTitle, RZ_FinaleAssets.PROOF);
		RZ_Safehouses houses = gm.GetSafehouses();
		int house = -1;
		if (houses)
			house = houses.FindNearest(m_vHim);

		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, house);
		if (h && gm.GetSettlement(h.m_iSettlement))
			gm.GetSettlement(h.m_iSettlement).AddInfection(-3);

		m_iFameReward = 0;
		m_bNoLeaderCredit = true;
		Finish(gm, playerId, 0, 0, "medic", "The Haven's medic reads it by the stove all night. In the morning she knows how it goes in, and how fast. Halcyon never sees it.");
	}

	//------------------------------------------------------------------------------------------------
	//! Sold: his notes on the fence's table.
	protected void OnFenceChoice(RZ_GameModeComponent gm, int playerId, int option)
	{
		if (option == 0)
		{
			if (!m_Ways.TryBribe(gm, playerId, RZ_EFaction.BANDITS, BRIBE_FAVOR))
				return;

			Finish(gm, playerId, 1, 1, "bought", "The fence counts the favour twice and slides the box over. \"Pleasure. Tell the doctor his sister's fine.\"");
			return;
		}

		if (option == 1)
		{
			if (!RZ_GameTime.IsNight() || !RZ_Stealth.IsHidden(playerId))
			{
				gm.NotifyPlayer(playerId, m_sTitle, "Not like this. After dark, and with nobody looking your way.");
				return;
			}

			Finish(gm, playerId, 1, 1, "lifted", "You lift the box off the table while the fence counts somebody else's money. The crew will notice by breakfast.");
			return;
		}

		if (option == 2)
		{
			IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (!RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.KNOWN))
			{
				gm.NotifyPlayer(playerId, m_sTitle, "I don't sell to faces I don't know.");
				return;
			}

			if (!me || RZ_HavenQuest.CountFood(gm, me) < BUY_FOOD || !RZ_StoryStats.TakeFood(gm, me, BUY_FOOD))
			{
				gm.NotifyPlayer(playerId, m_sTitle, "Five tins. Not four. I can count.");
				return;
			}

			Finish(gm, playerId, 1, 1, "sold back", "\"Five tins for a book of sums. You're a strange one.\" The notes go back to the lab.");
			return;
		}

		// Leave them: the notes stay with the crews, who have their proof now
		string refusal = RZ_RouteAssets.Refuse(gm, playerId, RZ_EFaction.BANDITS);
		if (!refusal.IsEmpty())
		{
			gm.NotifyPlayer(playerId, m_sTitle, refusal);
			return;
		}

		m_bDone = true;
		RZ_RouteAssets.Add(gm, playerId, RZ_EFaction.BANDITS, m_sTitle, RZ_FinaleAssets.PROOF);
		array<int> who = {playerId};
		RZ_StoryKit.Record(this, "fate", m_sName, "sold", who);
		gm.NotifyPlayer(playerId, m_sTitle, "You leave them on the table. His sister keeps her doses. The crews keep his sums.");
		m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHouseChoice(RZ_GameModeComponent gm, int playerId, int option, IEntity source)
	{
		if (option == 0)
		{
			if (m_Ways.TryTrust(gm, playerId, "colleague", RZ_EFaction.INSTITUTE, 2, "labtech") != RZ_ECheck.PASS)
			{
				gm.NotifyPlayer(playerId, m_sTitle, "\"Back to sampling? No.\" He turns back to the filter.");
				return;
			}

			Finish(gm, playerId, 2, 1, "back", "\"As a medic. Not a sampler. You tell them that.\" He packs his tools. The house loses a pump man.");
			return;
		}

		if (option == 1)
		{
			RZ_StoryKit.Dirty(gm, this, playerId);
			RZ_ECheck pressed = m_Ways.TryFear(gm, playerId, "colleague." + m_sName, RZ_EFaction.SURVIVORS, RZ_EFaction.INSTITUTE, 2, false, PRESS_INFAMY);
			if (pressed == RZ_ECheck.REFUSED)
			{
				gm.NotifyPlayer(playerId, m_sTitle, "The house's people stand round him. He isn't going anywhere with you.");
				return;
			}

			RZ_Deeds.AddLifeRecord(gm.GetProfile(playerId), "pressed_colleague", true);
			Finish(gm, playerId, 2, 1, "pressed", "He comes, because the house saw what you'd do. Nobody waves him off.");
			return;
		}

		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (option == 2 || option == 3)
		{
			if (!me || RZ_HavenQuest.CountFood(gm, me) < TRADE_FOOD || !RZ_StoryStats.TakeFood(gm, me, TRADE_FOOD))
			{
				gm.NotifyPlayer(playerId, m_sTitle, "Four tins, and the notes are yours. We need the tins more.");
				return;
			}

			if (option == 2)
			{
				Finish(gm, playerId, 1, 1, "notes", "\"Tell them what you like.\" The notes go to the lab. He stays at the filter.");
				return;
			}

			// His tag: a lie to Halcyon, found when someone Halcyon knows comes to the house
			RZ_WorldFacts.Get().Memory().Lies().Tell(gm, playerId, "colleague.turned", m_sName, RZ_EFaction.INSTITUTE, "", m_vHouse, RZ_ELieTest.VISIT, LIE_INFAMY,
				string.Format("You told us he turned. He signed a Haven ration card in %1 last week.", m_sHouse));
			Finish(gm, playerId, 1, 0.6, "walked", "He gives you his tag with the notes. \"Tell them I turned.\" You will.");
			return;
		}

		// Leave him be: no pay. The house's filter works, he takes a Doctor's place on the roads if
		// one is free, and the Independent route has an ally
		m_bDone = true;
		RZ_Errands errands = gm.GetErrands();
		int house = -1;
		if (gm.GetSafehouses())
			house = gm.GetSafehouses().FindNearest(m_vHouse);

		if (errands && house >= 0)
			errands.GetFaults().Clear(RZ_Faults.HousePlace(house), RZ_EFault.WATER);

		if (gm.GetWanderers())
			gm.GetWanderers().AddWithRole(RZ_EWandererRole.DOCTOR, m_sName.Substring(3, m_sName.Length() - 3), m_vHouse);

		array<int> who = {playerId};
		RZ_StoryKit.Record(this, "fate", m_sName, "left", who, string.Format("%1 is on the paper as an author, deceased. In %2 he's the man who fixed the filter.", m_sName, m_sHouse), "board");
		gm.NotifyPlayer(playerId, m_sTitle, "You leave him at the filter. Nobody pays you for that.");
		m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	protected void Finish(RZ_GameModeComponent gm, int playerId, int research, float scale, string how, string text)
	{
		m_bDone = true;
		m_iResearch = research;
		m_fRewardScale = scale;
		m_sHow = how;
		AddParticipant(playerId);
		gm.NotifyPlayer(playerId, m_sTitle, text);
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (gm.GetCure() && m_iResearch > 0)
			gm.GetCure().AddResearch(m_iResearch, "A lost researcher's notebook reached the lab. It will be read into the record, page by page.");

		RZ_StoryKit.Record(this, "fate", m_sName, m_sHow, m_aParticipants);
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailed(RZ_GameModeComponent gm)
	{
		// Time up while he hid: he dies overnight, and his notebook is still where he fell (Notebook 31)
		if (!m_bDone && m_sTruth == "hiding")
			RZ_StoryKit.Fallout(gm, this, "notebook31", m_vHim, m_sName);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Escort.Cleanup();
		m_Chain.Cleanup();
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (m_sHow.IsEmpty() && m_sTruth == "hiding")
			return string.Format("%1: nobody came. %2 died in the back room, with his notes.", m_sTitle, m_sName);

		return super.GetFailText(gm);
	}
}

//------------------------------------------------------------------------------------------------
//! #6 Research Data: recover a hard drive lying in an overrun clinic or pharmacy (on its shelves
//! or floor, like loose loot), or, in a town with none, where a courier fell near the middle. It's
//! placed once a player comes near. Choice: Halcyon (research) or sell it to Bastion (Halcyon
//! Infamy).
//! A story job (its key `researchdata`, kept per town): research, as they said; a trial log with
//! Haven names in it (a house within 5 km); or already copied (Bastion has it: the lab gets +1, Bastion
//! pays half). The orderly sweeping the glass (the talk scene): [Known - Halcyon] or a tin and he
//! brings the drive (no gun, no standing); trials: give the names to the house (it ends: 8 Haven
//! Favor, +5 Halcyon Infamy, nobody else pays, a Haven route asset).
class RZ_Mission_ResearchData : RZ_Mission_Choice
{
	protected int m_iHouse = -1;
	protected static const int WAY_SHOW = 0;
	protected static const int WAY_HOUSE = 1;

	//------------------------------------------------------------------------------------------------
	void RZ_Mission_ResearchData()
	{
		m_Job = new RZ_StoryJob();
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.KNOWN, 20, 20);
		m_eChoiceOther = RZ_EFaction.MILITARY;
		m_iDoubleCrossInfamy = 15;
		m_Req.m_bNotCured = true;
		m_fTimeLeft = 3000;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!CanRunChoice(gm, RZ_EFaction.INSTITUTE, RZ_EFaction.MILITARY))
			return 0;

		return 1.2;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickEmptySettlement(gm, 55, 100, this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		// Halcyon's drop sits at the lab when there is one
		vector halcyonPos;
		if (gm.HasLab())
		{
			halcyonPos = LandPointNear(gm.GetLabPosition(), 8, 15);
		}
		else
		{
			RZ_Settlement h = gm.GetSettlement(PickSettlementAway(gm, s.m_vCenter, 800, 5000, 0, 70));
			if (!h)
				return false;

			halcyonPos = LandPointNear(h.m_vCenter, 0, h.m_fRadius * 0.5);
		}

		// Bastion's drop where Bastion really is, when it holds a town in reach
		int bastionTown = PickHeldSettlementAway(gm, RZ_EFaction.MILITARY, s.m_vCenter, 800, 5000, m_iSettlement, this);
		if (bastionTown < 0)
			bastionTown = PickSettlementAway(gm, s.m_vCenter, 800, 5000, 0, 100, m_iSettlement, this);

		RZ_Settlement b = gm.GetSettlement(bastionTown);
		if (!b)
			return false;

		// A real clinic, hospital or pharmacy when the town has one; otherwise no building is named
		array<string> kinds = {"pharmacy"};
		RZ_Landmark clinic = RZ_LandmarkFinder.FindNearest(s.m_vCenter, s.m_fRadius, kinds);
		if (clinic)
			m_vPos = clinic.m_vPos;
		else
			m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.5);

		// In the clinic (a courier's: where they fell, on the ground)
		PlaceInsideLater(m_vPos);

		SpawnDrop(gm, RZ_EFaction.INSTITUTE, halcyonPos);
		SpawnDrop(gm, RZ_EFaction.MILITARY, LandPointNear(b.m_vCenter, b.m_fRadius * 0.8, b.m_fRadius + 50));
		SpawnGuards(gm, m_vPos, 4, 10, 40);
		m_fParticipationRadius = 0;
		m_sTitle = "Research Data";
		m_sDeed = "recovered the data at {town}";
		if (clinic)
			m_sBrief = string.Format("There's a hard drive with Halcyon's research somewhere in %1 in %2: a padded case, on a shelf or on the floor. Find it and take it to the lab. Bastion's drop in %3 would pay for it too.", clinic.m_sName, s.m_sName, b.m_sName);
		else
			m_sBrief = string.Format("A Halcyon courier went down near the middle of %1 with a hard drive of our research. It's lying where they fell. Find it and take it to the lab. Bastion's drop in %2 would pay for it too.", s.m_sName, b.m_sName);

		Truth(gm);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Truth(RZ_GameModeComponent gm)
	{
		array<string> values = {"research", "trials", "copied"};
		array<float> weights = {0.5, 0, 0.2};
		if (gm.GetSafehouses())
			m_iHouse = gm.GetSafehouses().FindNearest(m_vPos, 5000);

		if (m_iHouse >= 0)
			weights[1] = 0.3;

		string truth = m_Job.Draw(this, RZ_WorldFacts.Town(m_iSettlement), values, weights);
		m_Job.m_vReadAt = m_vPos;
		m_Job.Read("trials", "labtech", "The folder names on the drive's label are surnames. Haven surnames. This is a trial log, not research.");
		m_Job.Read("trials", "nurse,doctor", "Bite charts on the clinic wall, dated and initialled. People were treated here, and written down.");
		m_Job.Read("copied", "labtech", "The access sticker's been peeled and put back. Somebody read this drive two days ago, on a machine that isn't the lab's.");

		string reveal = "Numbers. Lots of numbers. The doctor said they'd save us. Then he left.";
		if (truth == "trials")
			reveal = "They tried things on people from the house. Paid them in tins. The names are all on there.";
		else if (truth == "copied")
			reveal = "A soldier came Tuesday with a laptop. Sat right there an hour. Said thank you. Lovely manners.";

		RZ_TalkScene sc = m_Job.Scene("The orderly", "researchdata.orderly", LandPointNear(m_vPos, 4, 8));
		int n0 = sc.Node("A man in a stained orderly's coat sweeps broken glass into neat piles. \"Mind the floor. I just did it.\"");
		int n1 = sc.Node("He leans the broom on the wall. \"Somebody should know what was on it. I'm somebody, I suppose.\"");
		sc.Ask(n0, "What happened here?", n1, reveal, true);
		sc.Way(n0, "Where's the drive?", RZ_TalkScene.TRUST, RZ_EFaction.INSTITUTE, 1, WAY_SHOW, "He fetches it from a drawer and wipes it on his coat before he hands it over.", "Which drive? There are no drives. There's a floor.");
		sc.Way(n0, "A tin for the drive", RZ_TalkScene.FOOD, -1, 1, WAY_SHOW, "He takes the tin and points under the counter with the broom.");
		if (truth == "trials")
			sc.Decide(n1, "Give the names to the house", "give the trial's names to the house", WAY_HOUSE, "The drive goes to the house that's on it. Halcyon will hear where.", RZ_EFaction.SURVIVORS);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStoryWay(RZ_GameModeComponent gm, int playerId, int way)
	{
		if (way == WAY_SHOW)
		{
			vector at = m_vPos;
			if (m_Job.m_Scene)
				at = m_Job.m_Scene.m_vAt;

			PutItemAt(gm, at);
			return;
		}

		array<int> who = {playerId};
		RZ_StoryKit.PayDecider(gm, this, playerId, RZ_EFaction.SURVIVORS, 8);
		RZ_StoryKit.BlameDecider(gm, this, playerId, RZ_EFaction.INSTITUTE, 5, "gave a trial log to the people in it");
		RZ_StoryKit.Record(this, "names", RZ_WorldFacts.Town(m_iSettlement), "house", who);
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, m_iHouse);
		if (h)
			RZ_StoryKit.Later(gm, playerId, h.m_sName, "An old man at the door with a printout folded small. \"My number was 31. I always wondered.\"", h.m_vPos, true);

		RZ_StoryKit.EndedOtherWay(this);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoiceMade(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction)
	{
		bool copied = m_Job && m_Job.m_sTruth == "copied";
		if (faction == RZ_EFaction.INSTITUTE && copied)
			gm.GetCure().AddResearch(1, "A lost drive of Halcyon's research reached the lab, and Bastion's drop paid for a copy on the way.");
		else if (faction == RZ_EFaction.INSTITUTE)
			gm.GetCure().AddResearch(3, "A lost drive of Halcyon's research reached the lab, its padded case scuffed but the data whole.");
		else
			gm.NotifyAll("Halcyon", "Our research data was sold to Bastion. We know who did it.");

		if (faction == RZ_EFaction.MILITARY && copied)
			m_fRewardScale = 0.5;
	}
}

//------------------------------------------------------------------------------------------------
//! #12 Hunt the Screamer: kill a Screamer before it alerts the town. After four
//! minutes it screams: a horde comes, but it can still be killed. ★ Hunter: +50% Fame.
class RZ_Mission_HuntScreamer : RZ_TribeContract
{
	protected float m_fScreamIn = 240;
	protected bool m_bScreamed;
	protected bool m_bScout;		// a pack's posted scout (its premise), not a town's screamer

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.TRUSTED, 25, 25);
		m_Req.Bonus("hunter");
		m_fBonusFameMult = 1.5;
		m_fTimeLeft = 1500;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_sTitle = "Hunt the Screamer";

		// A pack's scout on its post, in daylight, when there is one: the scout rule is the premise
		RZ_Tribes tribes = RZ_TribeContract.Tribes(gm);
		if (tribes && tribes.GetPhase() == RZ_ETribePhase.DAY)
		{
			foreach (RZ_Tribe t : tribes.GetTribeList())
			{
				RZ_TribePart scout = t.FindPart(RZ_ETribePart.SCOUT);
				if (!scout || scout.m_iCount <= 0 || scout.m_iPost < 0 || t.m_bFed || !Free(gm, t) || !TakeTribe(gm, t))
					continue;

				m_bScout = true;
				m_vPos = scout.m_vPos;
				m_fParticipationRadius = 250;
				m_sBrief = string.Format("Something sits on the ridge above %1 all day and watches. When it sees you, it shrieks, and the rest come. Kill it before it does. Halcyon wants what's left.", t.m_sPlace);
				return true;
			}
		}

		m_iSettlement = PickEmptySettlement(gm, 30, 100, this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.7);
		m_fParticipationRadius = s.m_fRadius + 100;
		m_sBrief = string.Format("Something in %1 shrieks when it sees anyone, and every infected in town comes to the sound. Kill it before it sees you. We want what's left of it: it's the only kind we've found that calls the others.", s.m_sName);
		return gm.GetSpawner().SpawnZombie(m_vPos, true, m_iSettlement, m_iId) != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		// A scout's own rule decides whether it shrieks (Zombies/RZ_TribeBodies.c)
		if (m_bScout)
			return;

		// The clock only runs once someone is hunting it
		if (m_bScreamed || m_aParticipants.IsEmpty())
			return;

		m_fScreamIn -= dt;
		if (m_fScreamIn > 0)
			return;

		m_bScreamed = true;
		gm.GetHeat().AddHeat(m_vPos, 80);
		gm.GetSpawner().SpawnHorde(m_vPos, 300, m_iId);
		NotifyParticipants(gm, m_sTitle, "It screamed. Everything in town heard it.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnZombiesKilled(RZ_GameModeComponent gm, int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		if (missionId == m_iId && special && !m_bScout)
		{
			m_vPos = pos;
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnTribeKill(RZ_GameModeComponent gm, RZ_Tribe t, RZ_EInfectedRole role, RZ_TribePart p, vector at, bool silent)
	{
		if (!m_bScout || t.m_iId != m_iTribe || role != RZ_EInfectedRole.SCOUT)
			return;

		m_vPos = at;
		m_bScreamed = t.m_fLastCall > Tribes(gm).GetClock() - 60;
		AddParticipant(NearestPlayer(at, 300));
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (!m_bScreamed)
			m_fRewardScale = 1.25;

		gm.SpawnSamples(m_vPos, 1);
	}
}

//------------------------------------------------------------------------------------------------
//! #15 Lab Defence: repel a raid on the lab (hold 4 minutes against waves). If a Red
//! Hand group prefab is set, raiders join the attack and the Red Hand remembers.
class RZ_Mission_LabDefence : RZ_Mission_Hold
{
	//------------------------------------------------------------------------------------------------
	void RZ_Mission_LabDefence()
	{
		m_Story = new RZ_MissionStory("labraid");
		m_Choice = new RZ_OneChoice();
		m_Choice.Set("A raider throws his rifle out of the hedge and his hands up after it: \"Done. I'm done.\" The guard at the gate looks at you. Decide at the gate.",
			"Bastion's stockade|Give him back to the crew|To Halcyon, for the trial", "decide what happens to the raider", 0);
		m_Choice.Asset(0, RZ_EFaction.MILITARY);
		m_Choice.Asset(1, RZ_EFaction.BANDITS);
		m_Choice.Asset(2, RZ_EFaction.INSTITUTE);
	}

	//------------------------------------------------------------------------------------------------
	override bool AsksChoice()
	{
		return super.AsksChoice() && Type() == RZ_Mission_LabDefence;
	}

	//------------------------------------------------------------------------------------------------
	override void OnChosen(RZ_GameModeComponent gm, int decider, int option)
	{
		string value = "stockade";
		if (option == 1)
		{
			value = "crew";
			m_Choice.PayWith(gm, this, RZ_EFaction.BANDITS, 6);
			NotifyParticipants(gm, m_sTitle, "He walks back to his crew with his hands in his pockets. They'll owe you for that. So will he, and he knows the gate now.");
		}
		else if (option == 2)
		{
			value = "trial";
			m_Choice.BlameWith(gm, this, RZ_EFaction.BANDITS, 4, "gave one of theirs to the trial");
			NotifyParticipants(gm, m_sTitle, "Halcyon takes him in by the back door, for the trial. Somebody writes his weight down. Nobody writes his name.");
		}
		else
		{
			m_Choice.PayWith(gm, this, RZ_EFaction.MILITARY, 4);
			NotifyParticipants(gm, m_sTitle, "Bastion's truck takes him to the stockade. He'll be there when the gates open, one way or another.");
		}

		RZ_StoryKit.Record(this, "raider", "lab", value, m_aParticipants);
	}

	protected IEntity m_Raiders;

	//------------------------------------------------------------------------------------------------
	//! On its own it holds the lab (a claim, docs/DESIGN.md "World facts"); a story that runs it
	//! holds the lab itself
	override void GetClaims(RZ_GameModeComponent gm, notnull array<string> result)
	{
		if (IsStandalone())
			result.Insert(RZ_WorldFacts.LAB);
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.TRUSTED, 30, 30);
		m_Req.m_bNotCured = true;
		m_fHoldNeeded = 240;
		m_fHoldRadius = 60;
		m_fWaveInterval = 60;
		m_iMaxWaves = 4;
		m_fWaveDistance = 300;
		m_fWaveTimer = 55;
		m_fTimeLeft = 1800;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!gm.HasLab() || gm.GetCure().IsFinalStand())
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = gm.GetLabSettlement();
		m_vPos = gm.GetLabPosition();
		m_fParticipationRadius = 100;
		m_sTitle = "Lab Defence";
		m_sBrief = "Something is coming for the lab: the fence sensors in the hills went off one after another, all heading this way. Get there and hold it for four minutes while we move the samples into the cold room. If the freezers go, the cure goes with them.";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnHoldStarted(RZ_GameModeComponent gm)
	{
		if (gm.GetRedHandGroupPrefab().IsEmpty())
			return;

		m_Raiders = RZ_Util.SpawnPrefab(gm.GetRedHandGroupPrefab(), LandPointNear(m_vPos, 250, 350));
		if (!m_Raiders)
			return;

		RZ_Util.SendGroupTo(m_Raiders, m_vPos, gm.GetSpawner().GetMoveWaypointPrefab());
		RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, m_Raiders);
		SetRival(RZ_EFaction.BANDITS, 10);
		NotifyParticipants(gm, m_sTitle, "A Red Hand crew is coming in behind the infected, using them as cover. Watch the gaps between the bodies: the ones carrying rifles are the crew.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		RZ_Util.DeleteUnit(m_Raiders);
	}
}
