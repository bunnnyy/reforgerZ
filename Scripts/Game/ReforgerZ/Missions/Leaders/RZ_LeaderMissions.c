//! Leader missions (docs/MISSIONS.md, "Leader missions"): personal jobs that Independent
//! and Rogue outpost and safehouse leaders give in person, off the stronghold's books, plus
//! the stronghold's Rein In contract against a Rogue place.
//!
//! A job is never picked by the mission manager's random draw (GetWeight is 0). RZ_Leaders
//! creates one when a player is near a leader who has none, choosing from the jobs for the
//! leader's faction and loyalty, weighted by GetLeaderWeight() and the leader's traits.
//! Rogue jobs may be discovered when they're done (RZ_Leaders.OnMissionFinished); most
//! rogue item jobs also have a drop at the stronghold, where you can turn the leader in.

//------------------------------------------------------------------------------------------------
//! Every leader job type. RZ_Leaders keeps its own list: none of these are in RZ_MissionRegistry.
class RZ_LeaderJobRegistry
{
	//------------------------------------------------------------------------------------------------
	static void RegisterAll(notnull array<typename> types)
	{
		// The Haven (Missions/Leaders/RZ_LeaderJobsHaven.c)
		types.Insert(RZ_Job_OurOwnFirst);
		types.Insert(RZ_Job_BringThemHome);
		types.Insert(RZ_Job_OverTheFence);
		types.Insert(RZ_Job_OldClinic);
		types.Insert(RZ_Job_RationRun);
		types.Insert(RZ_Job_NamesForBastion);
		types.Insert(RZ_Job_ShutTheDoor);
		types.Insert(RZ_Job_HiddenStores);

		// Task Force Bastion (RZ_LeaderJobsBastion.c)
		types.Insert(RZ_Job_HeartsAndMinds);
		types.Insert(RZ_Job_RealOrders);
		types.Insert(RZ_Job_TrainingDay);
		types.Insert(RZ_Job_LocalLaw);
		types.Insert(RZ_Job_RiflesForSale);
		types.Insert(RZ_Job_OffBookRequisition);
		types.Insert(RZ_Job_BurnTheLedger);

		// The Red Hand (RZ_LeaderJobsRedHand.c)
		types.Insert(RZ_Job_OurCrossroads);
		types.Insert(RZ_Job_HonourAmongThieves);
		types.Insert(RZ_Job_HouseRules);
		types.Insert(RZ_Job_DebtCollection);
		types.Insert(RZ_Job_SoftTouch);
		types.Insert(RZ_Job_CoupTalk);
		types.Insert(RZ_Job_SellOutRivalCrew);
		types.Insert(RZ_Job_SkimTheToll);

		// Halcyon (RZ_LeaderJobsHalcyon.c)
		types.Insert(RZ_Job_FieldStation);
		types.Insert(RZ_Job_SecondOpinion);
		types.Insert(RZ_Job_PatientsNotSamples);
		types.Insert(RZ_Job_TheNotebook);
		types.Insert(RZ_Job_UnsanctionedTrial);
		types.Insert(RZ_Job_LeakTheData);
		types.Insert(RZ_Job_SellTheFormula);
	}
}

//------------------------------------------------------------------------------------------------
//! Shared lookups for leader jobs.
class RZ_LeaderJob
{
	//------------------------------------------------------------------------------------------------
	static RZ_Leader Leader(RZ_GameModeComponent gm, RZ_Mission m)
	{
		if (!gm || !gm.GetLeaders() || !m)
			return null;

		RZ_Leader l = gm.GetLeaders().Get(m.m_iLeaderId);
		if (!l || !l.IsPresent())
			return null;

		return l;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_Leader Stronghold(RZ_GameModeComponent gm, RZ_EFaction faction)
	{
		if (!gm.GetLeaders())
			return null;

		return gm.GetLeaders().GetStronghold(faction);
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest present leader of `faction` between minDist and maxDist from pos (not
	//! excludeId). null if none.
	static RZ_Leader SeatNear(RZ_GameModeComponent gm, RZ_EFaction faction, vector pos, float minDist, float maxDist, int excludeId)
	{
		if (!gm.GetLeaders())
			return null;

		array<RZ_Leader> list = {};
		gm.GetLeaders().GetFactionLeaders(faction, list);
		RZ_Leader best;
		float bestDist = maxDist;
		foreach (RZ_Leader l : list)
		{
			if (l.m_iId == excludeId)
				continue;

			float d = RZ_Util.DistXZ(pos, l.m_vPos);
			if (d >= minDist && d <= bestDist)
			{
				bestDist = d;
				best = l;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest safehouse between minDist and maxDist from pos (not excludeIdx). -1 if none.
	static int SafehouseNear(RZ_GameModeComponent gm, vector pos, float minDist, float maxDist, int excludeIdx)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses)
			return -1;

		int best = -1;
		float bestDist = maxDist;
		for (int i = 0; i < houses.Count(); i++)
		{
			if (i == excludeIdx)
				continue;

			float d = RZ_Util.DistXZ(pos, houses.Get(i).m_vPos);
			if (d >= minDist && d <= bestDist)
			{
				bestDist = d;
				best = i;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Where to meet `faction`: one of its seats within 5 km, else the edge of a town 0.8-3 km
	//! away. False if neither exists.
	static bool MeetingPoint(RZ_GameModeComponent gm, RZ_EFaction faction, vector from, out vector pos, out string name)
	{
		RZ_Leader seat = SeatNear(gm, faction, from, 300, 5000, -1);
		if (seat)
		{
			pos = RZ_Mission.LandPointNear(seat.m_vPos, 6, 14);
			name = string.Format("%1's people in %2", seat.m_sShort, seat.m_sPlace);
			return true;
		}

		int town = RZ_Mission.PickSettlementAway(gm, from, 800, 3000, 0, 90);
		RZ_Settlement s = gm.GetSettlement(town);
		if (!s)
			return false;

		pos = RZ_Mission.LandPointNear(s.m_vCenter, s.m_fRadius, s.m_fRadius + 80);
		name = string.Format("the %1 drop outside %2", RZ_Factions.GetShortName(faction), s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Fame with a faction other than the job's, told to the player.
	static void CrossFame(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction, int amount)
	{
		if (amount <= 0 || playerId <= 0)
			return;

		gm.AddFame(playerId, faction, amount, false);
		gm.NotifyPlayer(playerId, RZ_Factions.GetName(faction), string.Format("Word reaches them of what you did. +%1 Fame.", amount));
	}

	//------------------------------------------------------------------------------------------------
	//! People join the safehouse nearest pos. Returns how many found a place.
	static int AddPeople(RZ_GameModeComponent gm, vector pos, int count)
	{
		if (!gm.GetSafehouses())
			return 0;

		return gm.GetSafehouses().AddPopulation(pos, count);
	}

	//------------------------------------------------------------------------------------------------
	//! The town's infection changes (the settlement nearest pos).
	static void TownInfection(RZ_GameModeComponent gm, vector pos, float amount)
	{
		RZ_Settlement s = gm.GetSettlement(gm.FindNearestSettlement(pos));
		if (s)
			s.AddInfection(amount);
	}
}

//------------------------------------------------------------------------------------------------
//! A leader job built on a delivery: an item to carry to one of several named drops. Rogue
//! jobs can add a drop at the stronghold (AddExposeDrop): hand it in there and the leader is
//! turned in; the stronghold pays instead.
class RZ_LeaderDelivery : RZ_Mission_Delivery
{
	protected int m_iExposeDrop = -1;
	protected int m_iDropIndex = -1;
	protected ref array<string> m_aDropNames = {};

	protected static const int EXPOSE_FAME = 10;
	protected static const int EXPOSE_FAVOR = 10;

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Subclasses implement SetupJob(); a failed setup removes what it already spawned.
	override bool Setup(RZ_GameModeComponent gm)
	{
		if (RZ_LeaderJob.Leader(gm, this) && SetupJob(gm))
			return true;

		Cleanup(gm);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool SetupJob(RZ_GameModeComponent gm)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Leader GetLeader(RZ_GameModeComponent gm)
	{
		return RZ_LeaderJob.Leader(gm, this);
	}

	//------------------------------------------------------------------------------------------------
	//! The item at the leader's seat, where they hand it over.
	protected bool SpawnItemAtSeat(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		if (!l)
			return false;

		m_vPos = LandPointNear(l.m_vPos, 2, 5);
		m_iSettlement = gm.FindNearestSettlement(l.m_vPos);
		return SpawnItem(gm, m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	protected bool AddDrop(RZ_GameModeComponent gm, RZ_EFaction faction, vector pos, string name)
	{
		if (!SpawnDrop(gm, faction, pos))
			return false;

		m_aDropNames.Insert(name);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Rogue jobs: a drop at the stronghold, where the item turns the leader in instead.
	protected void AddExposeDrop(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		if (!m_bRogueJob || !l)
			return;

		RZ_Leader top = RZ_LeaderJob.Stronghold(gm, l.m_eFaction);
		if (!top || top == l || gm.GetDropPointPrefab(l.m_eFaction).IsEmpty())
			return;

		if (AddDrop(gm, l.m_eFaction, LandPointNear(top.m_vPos, 6, 12), "or turn it in at the stronghold in " + top.m_sPlace))
			m_iExposeDrop = m_aDrops.Count() - 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnItemDelivered(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction, IEntity drop)
	{
		int idx = m_aDrops.Find(drop);
		if (idx < 0 || m_bDelivered)
			return false;

		m_iDropIndex = idx;
		if (idx == m_iExposeDrop)
		{
			m_bExposed = true;
			m_bRogueJob = false;
			m_bHasRival = false;
			m_iFameReward = EXPOSE_FAME;
			m_iFavorReward = EXPOSE_FAVOR;
		}

		return super.OnItemDelivered(gm, playerId, faction, drop);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelivered(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction)
	{
		if (m_bExposed)
		{
			RZ_Leader l = GetLeader(gm);
			string name = "them";
			if (l)
				name = l.m_sName;

			gm.NotifyPlayer(playerId, m_sTitle, string.Format("The stronghold takes it, and your account of who sent you. Nobody thanks you, exactly. %1 will hear by tonight who turned them in.", name));
			OnExposed(gm, playerId);
			return;
		}

		OnJobDelivered(gm, playerId, m_iDropIndex);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnJobDelivered(RZ_GameModeComponent gm, int playerId, int dropIndex)
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void OnExposed(RZ_GameModeComponent gm, int playerId)
	{
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = super.GetWhereText(gm, from);
		foreach (int i, string name : m_aDropNames)
		{
			text += string.Format("\nDrop: %1 (%2)", name, RZ_Util.DirectionText(from, GetDropPos(i)));
		}

		return text;
	}
}

//------------------------------------------------------------------------------------------------
//! A leader job built on the puzzle framework: people to talk to and choices to make.
class RZ_LeaderPuzzle : RZ_PuzzleMission
{
	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Leader GetLeader(RZ_GameModeComponent gm)
	{
		return RZ_LeaderJob.Leader(gm, this);
	}

	//------------------------------------------------------------------------------------------------
	//! Only the player who chose is paid for a choice.
	protected void ChosenBy(int playerId)
	{
		m_aParticipants.Clear();
		m_aParticipants.Insert(playerId);
	}
}

//------------------------------------------------------------------------------------------------
//! A leader job built on holding an area.
class RZ_LeaderHold : RZ_Mission_Hold
{
	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Leader GetLeader(RZ_GameModeComponent gm)
	{
		return RZ_LeaderJob.Leader(gm, this);
	}
}

//------------------------------------------------------------------------------------------------
//! A leader job built on visiting points.
class RZ_LeaderVisit : RZ_Mission_VisitPoints
{
	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Leader GetLeader(RZ_GameModeComponent gm)
	{
		return RZ_LeaderJob.Leader(gm, this);
	}
}

//------------------------------------------------------------------------------------------------
//! The stronghold's contract against a Rogue place (on the radio, from the stronghold
//! leader). Go to the rogue leader and either read them the terms (they come back toward
//! the line) or take their counter-offer (their Favor, their gratitude, and a chance the
//! stronghold finds out). RZ_Leaders starts it; the rogue leader is shown by a stand-in.
class RZ_Mission_ReinIn : RZ_PuzzleMission
{
	protected int m_iTarget = -1;
	protected string m_sTarget;
	protected string m_sPlace;
	protected bool m_bPrepared;
	protected int m_iTalk = -1;

	protected static const int COUNTER_FAVOR = 25;
	protected static const float COUNTER_FOUND_CHANCE = 0.35;
	protected static const int REIN_LOYALTY = 25;
	protected static const int COUNTER_LOYALTY = -5;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 15, 20);
		m_fTimeLeft = 3000;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	int GetTarget()
	{
		return m_iTarget;
	}

	//------------------------------------------------------------------------------------------------
	//! Who to rein in. The stronghold leader gives (and voices) the contract. False without
	//! the puzzle civilian or a stronghold.
	bool Prepare(RZ_GameModeComponent gm, RZ_Leader target)
	{
		if (!target || !gm.GetLeaders() || !HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return false;

		RZ_Leader top = gm.GetLeaders().GetStronghold(target.m_eFaction);
		if (!top || top == target)
			return false;

		SetContract(target.m_eFaction, RZ_ETier.KNOWN, 15, 20);
		m_iTarget = target.m_iId;
		m_iLeaderId = top.m_iId;
		m_sTarget = target.m_sName;
		m_sPlace = target.m_sPlace;
		m_bPrepared = true;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Leader l = gm.GetLeaders().Get(m_iTarget);
		if (!m_bPrepared || !l || !l.IsPresent() || l.m_eLoyalty != RZ_ELoyalty.ROGUE)
			return false;

		m_vPos = LandPointNear(l.m_vPos, 2, 5);
		m_iSettlement = gm.FindNearestSettlement(l.m_vPos);
		gm.GetLeaders().Hold(m_iTarget, m_iId);

		int favor = RZ_Util.RoundToInt(COUNTER_FAVOR * gm.GetLeaders().FavorMult(m_iTarget));
		m_iTalk = AddClue(l.m_sName, CounterText(l, favor));
		string labels = string.Format("Read them the stronghold's terms|Take their counter-offer (%1 %2 Favor)", favor, RZ_Factions.GetShortName(l.m_eFaction));
		SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPos, 0, "", labels, m_iTalk, "Talk to " + l.m_sShort);

		m_sTitle = "Rein In " + m_sPlace;
		m_sBrief = string.Format("%1 in %2 has stopped answering to us. Go there and read them our terms: they keep their place, they stop the side deals, and they answer when we call. They'll try to buy you. Don't let them.",
			m_sTarget, m_sPlace);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected string CounterText(RZ_Leader l, int favor)
	{
		string pitch;
		switch (l.m_eFaction)
		{
			case RZ_EFaction.SURVIVORS: pitch = "\"They sent you. Of course they did. Look around: these people eat because of what I do. Tell the council I listened, and take this for your trouble.\""; break;
			case RZ_EFaction.MILITARY: pitch = "\"So the stronghold remembered we exist. Here's the situation: they can't hold this post without me, and you can't eat their gratitude. Report that I complied. This is for the walk.\""; break;
			case RZ_EFaction.BANDITS: pitch = "\"The boss sent a messenger. That's sweet. Here's a better message: you never found me, and you're a little richer.\""; break;
			case RZ_EFaction.INSTITUTE: pitch = "\"The director would like me to stop doing the useful part of my job. I'd like you to tell the director I agreed. I can make that worth your while.\""; break;
		}

		return string.Format("%1 (%2 Favor.)", pitch, favor);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_Leader l = gm.GetLeaders().Get(m_iTarget);
		if (!l || !l.IsPresent())
			m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		RZ_Leaders leaders = gm.GetLeaders();
		RZ_Leader l = leaders.Get(m_iTarget);
		if (objIdx != 0 || !l)
			return;

		m_aParticipants.Clear();
		m_aParticipants.Insert(playerId);
		if (option == 0)
		{
			leaders.ShiftLoyalty(l, REIN_LOYALTY, true);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("You read the terms out. %1 listens to all of it, says nothing, and then nods once. It's not a promise. It's a start.", l.m_sShort));
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (option != 1)
			return;

		int favor = RZ_Util.RoundToInt(COUNTER_FAVOR * leaders.FavorMult(m_iTarget));
		gm.AddFavor(playerId, l.m_eFaction, favor);
		l.Remember(gm.GetProfile(playerId).m_sIdentity, RZ_Leaders.MEM_HELPED);
		leaders.ShiftLoyalty(l, COUNTER_LOYALTY, true);
		m_fRewardScale = 0;
		m_bNoLeaderCredit = true;
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("You take it. %1 smiles like they knew you would. (+%2 Favor)", l.m_sShort, favor));

		if (Math.RandomFloat01() < COUNTER_FOUND_CHANCE)
		{
			int infamy = leaders.GetSettings().m_iDiscoveryInfamy;
			gm.AddInfamy(playerId, l.m_eFaction, infamy);
			RZ_Leader top = leaders.Get(m_iLeaderId);
			if (top)
				top.Remember(gm.GetProfile(playerId).m_sIdentity, RZ_Leaders.MEM_CROSSED);

			gm.NotifyPlayer(playerId, RZ_Factions.GetName(l.m_eFaction), string.Format("Someone at %1 saw the envelope change hands. The stronghold knows you were bought. (+%2 Infamy)", m_sPlace, infamy));
		}

		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return string.Format("Nobody went to %1. %2 is still doing as they please.", m_sPlace, m_sTarget);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		if (gm.GetLeaders() && m_iTarget >= 0)
			gm.GetLeaders().Release(m_iTarget);
	}
}
