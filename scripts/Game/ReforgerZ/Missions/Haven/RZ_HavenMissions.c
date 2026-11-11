//! The Haven (Survivors) contracts. See docs/MISSIONS.md, "The Haven".

//------------------------------------------------------------------------------------------------
//! #1 The Cache (Supply Cache, with Raid the Cache merged in): reach a spot in a town and hold it.
//! Guarded by zombies. A mirror offer: one time in ten the Red Hand wants it too, for players the
//! Haven Watches (they hear 5 minutes later); whoever has more holders there when the hold ends
//! gets it, and a draw is half each. Players race each other to it, so expect PvP.
class RZ_Mission_SupplyCache : RZ_Mission_Hold
{
	protected bool m_bGuarded;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 10, 10);
		m_fHoldNeeded = 60;
		m_fHoldRadius = 25;
		m_Mirror = new RZ_Mirror(RZ_EFaction.BANDITS, RZ_ETier.KNOWN);
		m_Mirror.m_iNeedFaction = RZ_EFaction.SURVIVORS;
		m_Mirror.m_eNeed = RZ_EStandingCheck.AT_LEAST_WATCHED;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 3;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickEmptySettlement(gm, 15, 85, this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.7);
		m_sTitle = "The Cache";
		m_sDeed = "found a supply cache near {town}";
		m_sBrief = string.Format("A Haven scout radioed about an untouched supply cache in %1. Secure it and hold the area for a minute.", s.m_sName);
		m_Mirror.Roll();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The guards turn up when someone comes near.
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bGuarded && RZ_ContractHelp.AnyNear(players, m_vPos, 300))
		{
			m_bGuarded = true;
			SpawnGuards(gm, m_vPos, 3, 15, 40);
		}

		super.OnUpdate(gm, dt, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		m_Mirror.Apply(gm, this, m_Mirror.HoldWinner(gm, this, m_vPos, m_fHoldRadius));
		gm.SpawnRewardCrate(m_vPos);
	}
}

//------------------------------------------------------------------------------------------------
//! #2 Survivors Trapped (story key `trapped`; one real choice): reach a barricaded family, then hold
//! out through three waves until help arrives. With a survivor group prefab the survivors are real
//! AI (spawned when someone comes within 300 m), and the mission fails if they all die (fallout:
//! The Name on the Tag). Amends: works off Haven Infamy, except for a Watched player who sends them
//! to Bastion. ★ Nurse: bonus Fame, and one more of them makes it.
//! After the third wave the mother asks where they're going (anyone who took part, after the 5 s
//! hold): the nearest house (3, or 4 with a ★), the hungriest house (the same, and Haven morale +1),
//! or Bastion's post (Bastion pays the base instead; the Haven pays nothing). Nobody answers in
//! 90 s: the nearest house. One time in four the father is bitten: ★ Nurse or Doctor sees it at once;
//! everyone else when the house checks him at the door (he's walked out at dusk: one fewer arrives).
class RZ_Mission_SurvivorHoldout : RZ_Mission_Hold
{
	protected ref RZ_UnitWatch m_Survivors = new RZ_UnitWatch();
	protected bool m_bShown;
	protected bool m_bDied;
	protected bool m_bAsking;
	protected bool m_bChosen;
	protected bool m_bBitten;
	protected float m_fAsked;
	protected int m_iWhere = WHERE_NEAREST;
	protected string m_sSurname;
	protected IEntity m_Mother;
	protected ref array<int> m_aSeenBite = {};

	protected static const int WHERE_NEAREST = 0;
	protected static const int WHERE_HUNGRY = 1;
	protected static const int WHERE_BASTION = 2;
	protected static const float ASK_SECONDS = 90;

	//------------------------------------------------------------------------------------------------
	void RZ_Mission_SurvivorHoldout()
	{
		m_Story = new RZ_MissionStory("trapped");
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 20, 20);
		m_bAmends = true;
		m_Req.m_bAllowWatched = true;
		m_Req.Bonus("nurse");
		m_Req.Bonus("doctor");
		m_fBonusFameMult = 1.5;
		m_fHoldNeeded = 150;
		m_fHoldRadius = 40;
		m_fWaveInterval = 45;
		m_iMaxWaves = 3;
		m_fWaveTimer = 45; // first wave comes as soon as you arrive
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetDirector().GetStage() == RZ_EOutbreakStage.EARLY_OUTBREAK)
			return 1;

		return 2.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickEmptySettlement(gm, 30, 90, this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.6);
		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		m_sSurname = RZ_NoteText.Surname(rng);
		m_bBitten = rng.Chance(0.25);	// a fixed fact about this family, never a gotcha
		m_sTitle = "Survivors Trapped";
		m_sDeed = "got people out of {town}";
		m_sBrief = string.Format("A family is barricaded inside a house in %1 and they're running out of time. Get to them and hold until help arrives.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		// The family is only there in body once someone comes near
		if (!m_bShown && RZ_ContractHelp.AnyNear(players, m_vPos, 300))
		{
			m_bShown = true;
			m_Survivors.Spawn(gm.GetSurvivorGroupPrefab(), m_vPos);
		}

		// A group deletes itself when the last of them dies: gone counts as dead
		if (m_Survivors.IsDead(dt))
		{
			m_bDied = true;
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		// A Nurse or Doctor sees the bite on arrival
		if (m_bBitten && m_bShown)
		{
			foreach (int i, IEntity p : players)
			{
				if (!m_aSeenBite.Contains(ids[i]) && RZ_Util.DistXZ(p.GetOrigin(), m_vPos) <= 30)
				{
					m_aSeenBite.Insert(ids[i]);
					RZ_StoryKit.Read(gm, this, ids[i], "nurse,doctor", "The father keeps his left sleeve down and his arm close. He's bitten, and he knows you know.");
				}
			}
		}

		if (m_bAsking)
		{
			m_fAsked += dt;
			if (m_fAsked >= ASK_SECONDS && !m_bChosen)
				Choose(gm, -1, WHERE_NEAREST);

			return;
		}

		super.OnUpdate(gm, dt, ids, players);

		// Held: before they go, the mother asks where
		if (m_eState == RZ_EMissionState.SUCCEEDED && !m_bChosen)
		{
			m_eState = RZ_EMissionState.ACTIVE;
			Ask(gm);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Ask(RZ_GameModeComponent gm)
	{
		m_bAsking = true;
		ResourceName prefab = gm.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);
		if (!prefab.IsEmpty())
		{
			m_Mother = RZ_Util.SpawnPrefab(prefab, RZ_Util.GroundPos(RZ_Util.RandomPointInRing(m_vPos, 3, 6)));
			if (m_Mother)
				RZ_PuzzleObjectComponent.Tag(m_Mother, m_iId, 0, "", "The nearest house|The hungriest house: \"They can work\"|Bastion's post: \"The soldiers\"");
		}

		if (!m_Mother)
		{
			Choose(gm, -1, WHERE_NEAREST);
			return;
		}

		NotifyParticipants(gm, m_sTitle, string.Format("It's quiet. Mrs %1 has the kids by the hands. \"Where are we going? Tell me where.\" Talk to her.", m_sSurname));
	}

	//------------------------------------------------------------------------------------------------
	override void HandleObjectChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option)
	{
		if (!m_bAsking || m_bChosen || option < 0 || option > WHERE_BASTION)
			return;

		if (!RZ_StoryKit.MayDecide(gm, this, playerId))
			return;

		m_Story.m_Deciding.StartHold(gm, this, playerId, option, "decide where the family goes");
	}

	//------------------------------------------------------------------------------------------------
	override void OnHoldDone(RZ_GameModeComponent gm, int decider, int option)
	{
		super.OnHoldDone(gm, decider, option);
		if (!m_bChosen)
			Choose(gm, decider, option);
	}

	//------------------------------------------------------------------------------------------------
	protected void Choose(RZ_GameModeComponent gm, int decider, int where)
	{
		m_bChosen = true;
		m_iWhere = where;
		if (where == WHERE_BASTION)
		{
			m_eGiver = RZ_EFaction.MILITARY;
			m_bAmends = false;
			m_bNoLeaderCredit = true;
			m_iLeaderId = -1;
		}

		if (decider > 0)
			gm.NotifyPlayer(decider, m_sTitle, "She nods once and doesn't argue. They're ready.");

		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnHoldStarted(RZ_GameModeComponent gm)
	{
		m_fTimeLeft = Math.Max(m_fTimeLeft, m_fHoldNeeded + 300);
		NotifyParticipants(gm, "Hold the house", "They've heard the commotion. Hold on until help gets here!");
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.SpawnRewardCrate(m_vPos);

		int rescued = 3;
		foreach (int playerId : m_aParticipants)
		{
			if (HasBonus(gm, playerId))
				rescued = 4;
		}

		// The house checks everyone at the door: a bitten father is walked out at dusk
		if (m_bBitten && m_iWhere != WHERE_BASTION)
		{
			rescued--;
			NotifyParticipants(gm, m_sTitle, "At the door the house checks everyone, as it always does. The father's arm. He goes out at dusk with a lamp, and nobody follows him.");
		}

		RZ_Safehouses houses = gm.GetSafehouses();
		string went = "house";
		if (m_iWhere == WHERE_BASTION)
		{
			went = "bastion";
			NotifyParticipants(gm, m_sTitle, "Bastion's truck takes them to the post. The mother wanted the soldiers. Tonight the call will say three more civilians sheltered.");
		}
		else if (m_iWhere == WHERE_HUNGRY && houses && houses.Count() > 0)
		{
			went = "hungry";
			int hungriest = Hungriest(gm);
			houses.AddPopulationAt(hungriest, rescued);
			if (gm.GetErrands())
				gm.GetErrands().Morale(RZ_EFaction.SURVIVORS, 1, "a family came to work");
		}
		else if (houses && houses.Count() > 0)
		{
			houses.AddRescued(m_vPos, rescued, m_aParticipants);
		}

		string epilogue = string.Format("The %1s' boy chalks the days on %2's gym wall.", m_sSurname, GetLocationName(gm));
		RZ_StoryKit.Record(this, "family", m_sSurname, went, m_aParticipants, epilogue, "board");
	}

	//------------------------------------------------------------------------------------------------
	protected int Hungriest(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		int best = houses.FindNearest(m_vPos);
		int lowest = 1000000;
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			if (h && h.m_iFood < lowest)
			{
				lowest = h.m_iFood;
				best = i;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailed(RZ_GameModeComponent gm)
	{
		if (m_bDied)
			RZ_StoryKit.Fallout(gm, this, "name_on_tag", m_vPos, string.Format("the %1 family", m_sSurname));
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		// The survivors leave with the rescue team (or they're gone)
		m_Survivors.Delete();
		RZ_Util.DeleteUnit(m_Mother);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (m_bDied)
			return string.Format("Survivors Trapped (%1): the family didn't make it.", GetLocationName(gm));

		return super.GetFailText(gm);
	}
}

//------------------------------------------------------------------------------------------------
//! #3 Radio Relay: find high ground near a town and keep a beacon running.
//! It's loud, so it builds heat fast and pulls a horde. ★ Radio Op: half the hold time.
class RZ_Mission_RadioRelay : RZ_Mission_Hold
{
	//------------------------------------------------------------------------------------------------
	void RZ_Mission_RadioRelay()
	{
		m_Story = new RZ_MissionStory("relay");
		m_Choice = new RZ_OneChoice();
		m_Choice.Set("The relay's up. The house's radio man comes on: \"Bastion's on this band too. Do we carry their traffic? They'd send a patrol past the house for it.\" Answer him at the relay.",
			"The Haven's alone|Bastion's too, for a patrol", "decide whose net the relay carries", 0);
	}

	//------------------------------------------------------------------------------------------------
	override void OnChosen(RZ_GameModeComponent gm, int decider, int option)
	{
		string value = "haven";
		if (option == 1)
		{
			value = "shared";
			RZ_OneChoice.Morale(gm, RZ_EFaction.SURVIVORS, 1, "a patrol past the house");
			m_Choice.PayWith(gm, this, RZ_EFaction.MILITARY, 5);
			NotifyParticipants(gm, m_sTitle, "Bastion's traffic goes out on the Haven's relay now. A patrol walks past the house at dusk. The crews listen to both, and write it down.");
		}
		else
		{
			NotifyParticipants(gm, m_sTitle, "The relay carries the Haven's net and nothing else. Quieter that way. The house is on its own at dusk, as it always was.");
		}

		RZ_StoryKit.Record(this, "net", RZ_WorldFacts.Town(m_iSettlement), value, m_aParticipants);
	}

	protected bool m_bHordeSent;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 15, 15);
		m_Req.Bonus("radioham");
		m_fBonusHoldSpeed = 2;
		m_fHoldNeeded = 180;
		m_fHoldRadius = 30;
		m_fHeatPerSecond = 2;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlementNotHeldBy(gm, 0, 100, RZ_Factions.Bit(RZ_EFaction.MILITARY) | RZ_Factions.Bit(RZ_EFaction.BANDITS), this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = HighPointNear(s.m_vCenter, s.m_fRadius * 0.5, s.m_fRadius * 2);
		m_sTitle = "Radio Relay";
		m_sDeed = "fixed the radio relay at {town}";
		m_sBrief = string.Format("Get to the high ground near %1 and keep the Haven relay broadcasting for 3 minutes. Every infected nearby will hear it.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnHoldTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bHordeSent && m_fHeld >= 60)
		{
			m_bHordeSent = true;
			gm.GetSpawner().SpawnHorde(m_vPos, 350, m_iId);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.SpawnRewardCrate(m_vPos);
	}
}

//------------------------------------------------------------------------------------------------
//! #11 Burn the Nest: destroy a zombie hive in a heavily infested town. Needs a
//! destructible nest prefab (e.g. from the Zombie Hive mod).
class RZ_Mission_BurnTheNest : RZ_Mission_DestroyTarget
{
	protected bool m_bSampled;	// the one choice: a piece left for Halcyon

	//------------------------------------------------------------------------------------------------
	void RZ_Mission_BurnTheNest()
	{
		m_Story = new RZ_MissionStory("nest");
		m_Choice = new RZ_OneChoice();
		m_Choice.Set("It's burning. A Halcyon runner who followed the smoke stands off at the edge: \"Leave me a piece before it's ash. The lab can use it.\" The house that asked you wants it gone, all of it. Choose at the fire.",
			"Burn it all|Leave a piece for Halcyon", "decide what's left of the nest", 0);
		m_Choice.Asset(1, RZ_EFaction.INSTITUTE);
	}

	//------------------------------------------------------------------------------------------------
	override void OnChosen(RZ_GameModeComponent gm, int decider, int option)
	{
		string value = "burned";
		if (option == 1)
		{
			value = "sampled";
			m_bSampled = true;
			RZ_OneChoice.Morale(gm, RZ_EFaction.SURVIVORS, -1, "the nest left for the lab");
			NotifyParticipants(gm, m_sTitle, "The runner takes his piece and goes. The house that asked hears about it by supper. They don't say anything. They don't need to.");
		}

		RZ_StoryKit.Record(this, "fate", RZ_WorldFacts.Town(m_iSettlement), value, m_aParticipants);
	}

	protected bool m_bGuarded;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.TRUSTED, 20, 25);
		m_fTimeLeft = 3600;
	}

	//------------------------------------------------------------------------------------------------
	//! Its guards turn up when someone comes near.
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bGuarded && RZ_ContractHelp.AnyNear(players, m_vPos, 300))
		{
			m_bGuarded = true;
			SpawnGuards(gm, m_vPos, 4, 10, 30);
		}

		super.OnUpdate(gm, dt, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetNestPrefab().IsEmpty())
			return 0;

		return 1.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickEmptySettlement(gm, 60, 100, this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.5);
		if (!SpawnTarget(gm.GetNestPrefab(), m_vPos))
			return false;

		m_fParticipationRadius = s.m_fRadius + 100;
		m_sTitle = "Burn the Nest";
		m_sDeed = "burnt the dead in {town}";
		m_sBrief = string.Format("The slow ones lie up thick in %1, all in one place. Find it and burn it out.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
		{
			s.AddInfection(-30);
			gm.NotifyAll("Nest destroyed", s.m_sName + " is breathing easier.");
		}

		// Left for Halcyon: a second sample, and the research goes through the lab
		if (m_bSampled)
			gm.SpawnSamples(m_vPos, 2);
		else
			gm.SpawnSamples(m_vPos, 1);
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailed(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
			s.AddInfection(15);
	}
}

//------------------------------------------------------------------------------------------------
//! #4 Medicine Run: bring medical supplies lying in a town's pharmacy (on its shelves or floor,
//! like loose loot; the building nearest the middle if the town has no pharmacy) to a Haven
//! safehouse. Amends: works off Haven Infamy. ★ Nurse/Doctor: bonus Fame and the exact grid of
//! the pharmacy from the start (never the rough area).
//! Pharmacy Lure merged in: [Sneak] throw something loud well away from the door and the infected in
//! the aisles walk to the noise for a while; take the medicine unnoticed and it's the quiet way
//! (the same pay, no extras). Noticed taking it, they turn on you. Open to anyone, unarmed.
class RZ_Mission_MedicineRun : RZ_Mission_Delivery
{
	protected bool m_bGuarded;
	protected bool m_bLured;
	protected bool m_bTaken;
	protected float m_fLuredFor;
	protected ref array<IEntity> m_aGuards = {};
	protected ref array<IEntity> m_aWaypoints = {};
	protected ref RZ_NoticeTracker m_Notice = new RZ_NoticeTracker();

	//------------------------------------------------------------------------------------------------
	//! ★ Nurse/Doctor: they know the pharmacy, so it's pre-marked for them.
	override protected bool RoughByStanding(RZ_GameModeComponent gm, int playerId)
	{
		if (HasBonus(gm, playerId))
			return false;

		return super.RoughByStanding(gm, playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! The pharmacy's guards turn up when someone comes near; a noise well away from the door draws
	//! them out for a while.
	override protected void OnDeliveryTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bGuarded && RZ_ContractHelp.AnyNear(players, m_vPos, 300))
		{
			m_bGuarded = true;
			for (int i = 0; i < 3; i++)
			{
				RZ_TrackedZombie z = gm.GetSpawner().SpawnZombie(LandPointNear(m_vPos, 3, 12), false, m_iSettlement, m_iId);
				if (z)
					m_aGuards.Insert(z.m_Entity);
			}
		}

		if (!m_bGuarded || m_bTaken || gm.GetSpawner().GetMoveWaypointPrefab().IsEmpty())
			return;

		m_Notice.Track(gm, ids, players, dt, m_vPos, 150);
		vector noise;
		if (gm.GetHeat().FindRecentNoise(m_vPos, RZ_UnarmedHelper.LURE_RADIUS, RZ_UnarmedHelper.LURE_AGE, noise) && RZ_Util.DistXZ(noise, m_vPos) > 25)
		{
			if (!m_bLured)
				RZ_UnarmedHelper.SteerUnits(gm, m_aGuards, noise, m_aWaypoints);

			m_bLured = true;
			m_fLuredFor = 0;
		}
		else if (m_bLured)
		{
			m_fLuredFor += dt;
			if (m_fLuredFor >= RZ_UnarmedHelper.LURE_HOLD)
			{
				m_bLured = false;
				RZ_UnarmedHelper.SteerUnits(gm, m_aGuards, m_vPos, m_aWaypoints);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Taken while they were drawn off and nobody noticed: the quiet way.
	override protected void OnCarrierChanged(RZ_GameModeComponent gm, int previous, int carrier)
	{
		super.OnCarrierChanged(gm, previous, carrier);
		if (m_bTaken || carrier <= 0 || !m_bLured)
			return;

		m_bTaken = true;
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(carrier);
		if (player && m_Notice.IsNoticed(gm, player))
		{
			RZ_UnarmedHelper.SteerUnits(gm, m_aGuards, player.GetOrigin(), m_aWaypoints);
			gm.NotifyPlayer(carrier, m_sTitle, m_Notice.GetReason(gm, player) + " They turn from the shelves toward you. Run.");
			return;
		}

		RZ_Log.Info(string.Format("Story: medicine sneak pass by %1", RZ_Util.PlayerName(carrier)));
		gm.NotifyPlayer(carrier, m_sTitle, "You sweep the boxes into your bag. Nothing in the aisles turns its head.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		RZ_UnarmedHelper.DeleteAll(m_aWaypoints);
		m_aGuards.Clear();
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 15, 20);
		m_bAmends = true;
		m_Req.m_bAllowWatched = true;
		m_Req.Bonus("nurse");
		m_Req.Bonus("doctor");
		m_fBonusFameMult = 1.5;
		m_fTimeLeft = 2700;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!CanRun(gm, RZ_EFaction.SURVIVORS))
			return 0;

		return 2;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		int safe = PickSafestSettlement(gm);
		RZ_Settlement home = gm.GetSettlement(safe);
		if (!home)
			return false;

		// The drop is at a real Haven safehouse when there are any
		vector dropPos = LandPointNear(home.m_vCenter, 0, home.m_fRadius * 0.4);
		string homeName = home.m_sName;
		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses && houses.Count() > 0)
		{
			RZ_Safehouse house = houses.Get(houses.PickRandom());
			dropPos = LandPointNear(house.m_vPos, 2, 6);
			homeName = house.m_sName;
		}

		m_iSettlement = PickEmptySettlementAway(gm, dropPos, 800, 4000, 20, 90, -1, this);
		RZ_Settlement pharmacy = gm.GetSettlement(m_iSettlement);
		if (!pharmacy)
			return false;

		// The supplies lie in the pharmacy, placed once someone comes near
		array<string> kinds = {"pharmacy"};
		RZ_Landmark shop = RZ_LandmarkFinder.FindNearest(pharmacy.m_vCenter, pharmacy.m_fRadius, kinds);
		if (shop)
			m_vPos = shop.m_vPos;
		else
			m_vPos = LandPointNear(pharmacy.m_vCenter, 0, pharmacy.m_fRadius * 0.6);

		if (!SpawnDrop(gm, RZ_EFaction.SURVIVORS, dropPos))
			return false;

		PlaceInsideLater(m_vPos);
		m_sTitle = "Medicine Run";
		m_sDeed = "brought medicine to {town}";
		m_sBrief = string.Format("The Haven safehouse in %1 is out of medicine. There's a stocked pharmacy in %2: the supplies are on its shelves, and something stands in the aisles. Fight it, or throw something loud well away from the door and walk in while it looks. Bring them back.", homeName, pharmacy.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return super.GetWhereText(gm, from) + "\nPharmacy grid: " + RZ_SettlementScanner.GridReference(m_vPos);
	}
}

//------------------------------------------------------------------------------------------------
//! #5 The Bitten Friend: a Haven member is bitten. Bring an antidote before he
//! turns, or end it yourself (mercy, half the Fame). ★ Nurse: slows his infection.
//! He's there in body once someone comes within 300 m. *Simplified:* there's no one-person slot,
//! so the survivor group stands in for him (all of it has to die). Mercy counts only if a player
//! fired within 30 m of him in the 10 s before he died; anything else that kills him fails it.
//! His clock runs from the start either way.
class RZ_Mission_BittenFriend : RZ_Mission
{
	protected ref RZ_UnitWatch m_Friend = new RZ_UnitWatch();
	protected float m_fTurnIn = 900;
	protected bool m_bSlowed;
	protected bool m_bShown;
	protected bool m_bKilled;		// died without a player's mercy
	protected float m_fClock;
	protected float m_fShotAt = -1000;
	protected int m_iShooter = -1;

	protected static const float MERCY_RADIUS = 30;
	protected static const float MERCY_WINDOW = 10;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 20, 20);
		m_Req.Bonus("nurse");
		m_Req.Bonus("doctor");
		m_fTimeLeft = 1200;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetSurvivorGroupPrefab().IsEmpty() || gm.GetAntidotePrefab().IsEmpty())
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlementNotHeldBy(gm, 0, 60, RZ_Factions.Bit(RZ_EFaction.MILITARY) | RZ_Factions.Bit(RZ_EFaction.BANDITS), this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.5);
		m_fParticipationRadius = 60;
		m_sTitle = "The Bitten Friend";
		m_sBrief = string.Format("One of ours was bitten in %1. Bring him an antidote before he turns, or do what has to be done.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Who fired close to him, and when (mercy).
	override void OnPlayerShot(RZ_GameModeComponent gm, int playerId, vector pos)
	{
		if (!m_Friend.m_Unit || RZ_Util.DistXZ(pos, m_Friend.GetPos(m_vPos)) > MERCY_RADIUS)
			return;

		m_iShooter = playerId;
		m_fShotAt = m_fClock;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_fClock += dt;
		if (!m_bShown && RZ_ContractHelp.AnyNear(players, m_vPos, 300))
		{
			m_bShown = true;
			if (!m_Friend.Spawn(gm.GetSurvivorGroupPrefab(), m_vPos))
			{
				m_eState = RZ_EMissionState.FAILED;
				return;
			}
		}

		// He died. A player put him down before he turned (mercy), or something else got him
		if (m_Friend.IsDead(dt))
		{
			if (m_iShooter > 0 && m_fClock - m_fShotAt <= MERCY_WINDOW)
			{
				if (!m_aParticipants.Contains(m_iShooter))
					m_aParticipants.Insert(m_iShooter);

				m_fRewardScale = 0.5;
				m_eState = RZ_EMissionState.SUCCEEDED;
				return;
			}

			m_bKilled = true;
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		vector at = m_Friend.GetPos(m_vPos);
		if (m_Friend.m_Unit && !m_bSlowed && AnyBonusNear(gm, ids, players, at, 10))
		{
			m_bSlowed = true;
			m_fTurnIn = m_fTurnIn + 300;
			NotifyParticipants(gm, m_sTitle, "You clean and dress the bite. That buys him about five more minutes.");
		}

		foreach (int i, IEntity p : players)
		{
			if (!m_Friend.m_Unit || RZ_Util.DistXZ(p.GetOrigin(), at) > 4)
				continue;

			if (RZ_AntidoteHelper.TakeOne(p))
			{
				if (!m_aParticipants.Contains(ids[i]))
					m_aParticipants.Insert(ids[i]);

				m_eState = RZ_EMissionState.SUCCEEDED;
				return;
			}
		}

		m_fTurnIn -= dt;
		if (m_fTurnIn <= 0)
		{
			// Only a body near players turns into a real one
			if (m_Friend.m_Unit)
				gm.GetSpawner().SpawnZombie(at, true, m_iSettlement, -1);

			NotifyParticipants(gm, m_sTitle, "Too late. He's one of them now.");
			m_eState = RZ_EMissionState.FAILED;
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (m_bKilled)
			return string.Format("The Bitten Friend (%1): something got to him before anyone could help.", GetLocationName(gm));

		return super.GetFailText(gm);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		m_Friend.Delete();
	}
}

//------------------------------------------------------------------------------------------------
//! #9 Missing Scout: search an area for a scout's body and bring his notes home.
//! The notes point to a stash, which is revealed to whoever hands them in. The notes go to a
//! Haven safehouse (the safest town if there are none).
class RZ_Mission_MissingScout : RZ_Mission_Delivery
{
	protected vector m_vSearchCenter;
	protected bool m_bGuarded;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 15, 15);
		m_fTimeLeft = 2700;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!CanRun(gm, RZ_EFaction.SURVIVORS))
			return 0;

		return 1.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickEmptySettlement(gm, 10, 80, this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		// The body is somewhere in a 250 m area outside town; only the area is given
		m_vSearchCenter = LandPointNear(s.m_vCenter, s.m_fRadius, s.m_fRadius + 400);
		m_vPos = LandPointNear(m_vSearchCenter, 0, 200);

		// Home is a real Haven safehouse (not the town he went missing in), else the safest town
		vector dropPos;
		string homeName;
		int house = RZ_ContractHelp.PickSafehouse(gm, m_vPos, 500, 100000);
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, house);
		if (h && h.m_iSettlement != m_iSettlement)
		{
			dropPos = LandPointNear(h.m_vPos, 2, 6);
			homeName = h.m_sName;
		}
		else
		{
			RZ_Settlement homeTown = gm.GetSettlement(PickSafestSettlement(gm, m_iSettlement));
			if (!homeTown)
				return false;

			dropPos = LandPointNear(homeTown.m_vCenter, 0, homeTown.m_fRadius * 0.4);
			homeName = homeTown.m_sName;
		}

		if (!SpawnItem(gm, m_vPos))
			return false;

		if (!SpawnDrop(gm, RZ_EFaction.SURVIVORS, dropPos))
			return false;

		m_sTitle = "Missing Scout";
		m_sDeed = "found the scout near {town}";
		m_sBrief = string.Format("A Haven scout never came back from %1. Search the area, find his notes and bring them to the safehouse in %2.", s.m_sName, homeName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Whatever got him is still around: it turns up when someone comes near.
	override protected void OnDeliveryTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bGuarded && RZ_ContractHelp.AnyNear(players, m_vPos, 300))
		{
			m_bGuarded = true;
			SpawnGuards(gm, m_vPos, 2, 10, 30);
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return string.Format("search area %1, %2", RZ_SettlementScanner.GridReference(m_vSearchCenter), RZ_Util.DirectionText(from, m_vSearchCenter));
	}

	//------------------------------------------------------------------------------------------------
	override void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
		if (m_aParticipants.IsEmpty() || m_aParticipants[0] != playerId)
			return;

		// His notes mention a stash
		int idx = PickEmptySettlement(gm, 0, 100);
		RZ_Settlement s = gm.GetSettlement(idx);
		if (!s)
			return;

		vector stash = LandPointNear(s.m_vCenter, s.m_fRadius * 0.5, s.m_fRadius);
		gm.SpawnRewardCrate(stash);
		gm.NotifyPlayer(playerId, "The scout's notes", string.Format("He marked a stash near %1, grid %2.", s.m_sName, RZ_SettlementScanner.GridReference(stash)));
	}
}

//------------------------------------------------------------------------------------------------
//! #16 Trader Caravan: defend a Haven trader at a crossroads from a Red Hand raid.
//! Needs the survivor group and Red Hand group prefabs. The trader is there in body once someone
//! comes within 300 m; the raid comes 90 s after someone reaches him.
class RZ_Mission_TraderCaravan : RZ_Mission
{
	protected ref RZ_UnitWatch m_Trader = new RZ_UnitWatch();
	protected ref RZ_UnitWatch m_Raiders = new RZ_UnitWatch();
	protected float m_fRaidIn = 90;
	protected bool m_bRaidSent;
	protected bool m_bShown;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.TRUSTED, 25, 30);
		m_Req.Need(RZ_EFaction.BANDITS, RZ_EStandingCheck.NOT_AT_LEAST_TRUSTED);
		SetRival(RZ_EFaction.BANDITS, 15);
		m_fTimeLeft = 1800;
		m_fParticipationRadius = 120;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetSurvivorGroupPrefab().IsEmpty() || gm.GetRedHandGroupPrefab().IsEmpty())
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlementNotHeldBy(gm, 0, 60, RZ_Factions.Bit(RZ_EFaction.BANDITS), this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius, s.m_fRadius + 150);
		m_sTitle = "Trader Caravan";
		m_sBrief = string.Format("A Haven trader stopped at the crossroads outside %1. The Red Hand knows. Get there and keep him alive.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bShown)
		{
			if (!RZ_ContractHelp.AnyNear(players, m_vPos, 300))
				return;

			m_bShown = true;
			if (!m_Trader.Spawn(gm.GetSurvivorGroupPrefab(), m_vPos))
			{
				m_eState = RZ_EMissionState.FAILED;
				return;
			}
		}

		// A group deletes itself when the last of them dies: gone counts as dead
		if (m_Trader.IsDead(dt))
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (!m_bRaidSent)
		{
			// The raid starts once someone is there to see it
			if (CountPlayersNear(players, m_vPos, 120) == 0)
				return;

			m_fRaidIn -= dt;
			if (m_fRaidIn > 0)
				return;

			m_bRaidSent = true;
			vector from = LandPointNear(m_vPos, 250, 350);
			if (!m_Raiders.Spawn(gm.GetRedHandGroupPrefab(), from))
			{
				m_eState = RZ_EMissionState.FAILED;
				return;
			}

			RZ_Util.SendGroupTo(m_Raiders.m_Unit, m_vPos, gm.GetSpawner().GetMoveWaypointPrefab());
			// The raiders fight the players near them and go for the trader, who stays friendly to players
			RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, m_Raiders.m_Unit);
			if (gm.GetHostility())
				gm.GetHostility().MakeUnitHostileTo(m_Trader.m_Unit, RZ_EFaction.BANDITS);
			NotifyParticipants(gm, m_sTitle, "Red Hand raiders incoming!");
			return;
		}

		if (m_Raiders.IsDead(dt))
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.SpawnRewardCrate(RZ_Util.RandomPointInRing(m_vPos, 3, 6));
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		m_Trader.Delete();
		m_Raiders.Delete();
	}
}

//------------------------------------------------------------------------------------------------
//! #17 Rescue the Kids: get children out of a school without making noise. Hold
//! the school for 90 s; if the area gets too loud, a horde comes.
//! ★ Hunter: quieter approach (hold time runs a third faster).
class RZ_Mission_RescueTheKids : RZ_Mission_Hold
{
	protected int m_iWhere;		// the one choice: 0 the nearest house, 1 their aunt at Bastion's post

	//------------------------------------------------------------------------------------------------
	void RZ_Mission_RescueTheKids()
	{
		m_Story = new RZ_MissionStory("kids");
		m_Choice = new RZ_OneChoice();
		m_Choice.Set("The kids are out. The oldest one says their aunt is at Bastion's post: \"She's a cook there. She said to come if.\" Decide with them, here.",
			"The nearest house|Their aunt at Bastion's post", "decide where the kids go", 0);
	}

	//------------------------------------------------------------------------------------------------
	override void OnChosen(RZ_GameModeComponent gm, int decider, int option)
	{
		m_iWhere = option;
		if (option == 1)
		{
			m_eGiver = RZ_EFaction.MILITARY;
			m_bNoLeaderCredit = true;
			m_iLeaderId = -1;
		}

		string value = "house";
		if (option == 1)
			value = "aunt";

		RZ_StoryKit.Record(this, "went", RZ_WorldFacts.Town(m_iSettlement), value, m_aParticipants);
	}

	protected bool m_bHordeSent;
	protected bool m_bGuarded;

	protected static const float LOUD_HEAT = 40;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.TRUSTED, 30, 30);
		m_Req.Bonus("hunter");
		m_fBonusHoldSpeed = 1.33;
		m_fHoldNeeded = 90;
		m_fHoldRadius = 25;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickEmptySettlement(gm, 30, 85, this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.5);
		m_sTitle = "Rescue the Kids";
		m_sDeed = "brought the children back from {town}";
		m_sBrief = string.Format("Children are hiding in the school in %1. Get them out quietly. If it gets loud, everything in town will come.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The infected around the school turn up when someone comes near.
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bGuarded && RZ_ContractHelp.AnyNear(players, m_vPos, 300))
		{
			m_bGuarded = true;
			SpawnGuards(gm, m_vPos, 2, 20, 50);
		}

		super.OnUpdate(gm, dt, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnHoldTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bHordeSent || gm.GetHeat().GetHeat(m_vPos) < LOUD_HEAT)
			return;

		m_bHordeSent = true;
		gm.GetSpawner().SpawnHorde(m_vPos, 250, m_iId);
		NotifyParticipants(gm, m_sTitle, "Too loud! They're coming. Hold on and get the kids out.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (!m_bHordeSent)
			m_fRewardScale = 1.25; // nobody noticed a thing

		if (m_iWhere == 1)
		{
			NotifyParticipants(gm, m_sTitle, "Bastion's truck takes them to their aunt at the post. Tonight the call will say three more civilians sheltered, and get the number right.");
			return;
		}

		// Three children join the nearest Haven safehouse
		if (gm.GetSafehouses())
			gm.GetSafehouses().AddRescued(m_vPos, 3, m_aParticipants);
	}
}
