//! Task Force Bastion's later contracts (docs/MISSIONS.md, "Task Force Bastion": #8, 9, 10, 11,
//! 12, 14, 16, 18, 19, 20), built on the mission building blocks (Missions/Blocks/).

//------------------------------------------------------------------------------------------------
//! Shared by Bastion's contracts.
class RZ_BastionContractHelp
{
	//------------------------------------------------------------------------------------------------
	//! Bastion intel on the radio: where infected are massing, for the players given and every
	//! player Known or better with Bastion.
	static void ShareHordeIntel(RZ_GameModeComponent gm, array<int> also)
	{
		vector hot;
		float heat;
		string text = "All quiet out there. No hordes forming.";
		if (gm.GetHeat().GetHottest(hot, heat))
		{
			RZ_Settlement s = gm.GetSettlement(gm.FindNearestSettlement(hot));
			string where = RZ_SettlementScanner.GridReference(hot);
			if (s)
				where = s.m_sName + " (" + where + ")";

			text = "Relay traffic says infected are massing near " + where + ".";
		}

		array<int> all = {};
		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int playerId : all)
		{
			if (also.Contains(playerId) || RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.MILITARY), RZ_ETier.KNOWN))
				gm.NotifyPlayer(playerId, "Bastion intel", text);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! #9 Curfew Patrol: walk four patrol points in a town at night (15 s at each) and put down six
//! infected inside it. Offered only at or just before nightfall; it lasts until dawn.
//! ★ Police: the beat goes 1.5 times faster.
class RZ_Mission_CurfewPatrol : RZ_Mission_VisitPoints
{
	protected int m_iKills;
	protected bool m_bWalked;

	protected static const int KILLS = 6;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 22, 20);
		m_Req.Bonus("police");
		m_fBonusVisitSpeed = 1.5;
		m_fVisitRadius = 15;
		m_fVisitSeconds = 15;
		m_sPointName = "Patrol point";
		m_fTimeLeft = 2400;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (RZ_GameTime.IsNight())
			return 1.2;

		float secs = RZ_GameTime.RealSecondsUntil(RZ_GameTime.NIGHT_START);
		if (secs >= 0 && secs <= 600)
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		// Curfew where people are about: never in a Red Hand town or the lab's
		m_iSettlement = PickSettlementNotHeldBy(gm, 15, 65, RZ_Factions.Bit(RZ_EFaction.BANDITS) | RZ_Factions.Bit(RZ_EFaction.INSTITUTE), this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = s.m_vCenter;
		m_fParticipationRadius = s.m_fRadius;
		for (int i = 0; i < 4; i++)
		{
			AddPoint(LandPointNear(s.m_vCenter, s.m_fRadius * 0.2, s.m_fRadius * 0.8));
		}

		float secs = RZ_GameTime.RealSecondsUntil(RZ_GameTime.NIGHT_END);
		if (secs > 0)
			m_fTimeLeft = Math.Clamp(secs + 300, 1200, 3600);

		m_sTitle = "Curfew Patrol";
		m_sDeed = "walked the curfew in {town}";
		m_sBrief = string.Format("Curfew in %1 tonight. Walk the beat, four points, and put down at least %2 of them inside the town. Lights low.", s.m_sName, KILLS);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		super.OnUpdate(gm, dt, ids, players);
		if (m_eState != RZ_EMissionState.SUCCEEDED || m_iKills >= KILLS)
			return;

		// The beat is walked; the town isn't clear yet
		m_eState = RZ_EMissionState.ACTIVE;
		if (!m_bWalked)
		{
			m_bWalked = true;
			NotifyParticipants(gm, m_sTitle, string.Format("Beat walked. %1 more infected to put down inside the town.", KILLS - m_iKills));
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnZombiesKilled(RZ_GameModeComponent gm, int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		if (settlementIdx != m_iSettlement)
			return;

		m_iKills += count;
		if (m_bWalked && m_iKills >= KILLS && m_eState == RZ_EMissionState.ACTIVE)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}
}

//------------------------------------------------------------------------------------------------
//! #10 The Armory (Weapons Cache, with Loot the Armory merged in): an armoury in an overrun town (a
//! barracks or police station if the map has one). The case lies inside (like loose loot, placed
//! once someone comes near); carry it to Bastion. Bastion gets stronger; you get paid, not armed.
//! A mirror offer: one time in ten the Red Hand (Known) wants it too and hears 5 minutes later; its
//! drop goes up then. Whoever carries the case to a drop decides (a choice: taken to the other side
//! it's a switch). "Nobody": drop the case down the town's well (with the well prop): both sides
//! are angry, nobody pays, and the world remembers. ★ Police knows the layout (the exact spot).
//! A story job (its key `weaponscache`, kept per town): rifles, as they said; emptied (others got there
//! first: one case left, 70%); or sold (the post's own sergeant sold them to a crew within 6 km). The
//! caretaker (the talk scene): [Known - Bastion] or a tin and he shows you the case (no gun, no
//! standing); sold: name the sergeant to the post (Bastion morale -1, the nearest Bastion leader's
//! loyalty -3, +3 Red Hand Infamy).
class RZ_Mission_WeaponsCache : RZ_Mission_Choice
{
	protected string m_sCrew = "a crew";
	protected static const int WAY_SHOW = 0;
	protected static const int WAY_NAME = 1;

	//------------------------------------------------------------------------------------------------
	void RZ_Mission_WeaponsCache()
	{
		m_Job = new RZ_StoryJob();
	}

	protected bool m_bGuarded;
	protected bool m_bRedDrop;
	protected vector m_vWell;
	protected IEntity m_Well;
	protected float m_fAtWell;

	protected static const float WELL_SECONDS = 10;
	protected static const int WELL_INFAMY = 5;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.TRUSTED, 30, 30);
		m_Req.Bonus("police");
		m_eChoiceOther = RZ_EFaction.BANDITS;
		m_fTimeLeft = 3000;
		m_fParticipationRadius = 0;
		m_Mirror = new RZ_Mirror(RZ_EFaction.BANDITS, RZ_ETier.KNOWN);
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!CanRun(gm, RZ_EFaction.MILITARY))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! ★ Police: they know the layout, so it's exact for them.
	override protected bool RoughByStanding(RZ_GameModeComponent gm, int playerId)
	{
		if (HasBonus(gm, playerId))
			return false;

		return super.RoughByStanding(gm, playerId);
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		// Left when the town fell: a town nobody lives in now
		m_iSettlement = PickEmptySettlement(gm, 60, 100, this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		array<string> kinds = {"military", "police"};
		RZ_Landmark armoury = RZ_LandmarkFinder.FindNearest(s.m_vCenter, s.m_fRadius, kinds);
		string place = "the armoury";
		if (armoury)
		{
			m_vPos = armoury.m_vPos;
			place = armoury.m_sName;
		}
		else
		{
			m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.5);
		}

		vector meet;
		string meetName;
		if (!RZ_LeaderJob.MeetingPoint(gm, RZ_EFaction.MILITARY, s.m_vCenter, meet, meetName))
			return false;

		if (!SpawnDrop(gm, RZ_EFaction.MILITARY, meet))
			return false;

		if (!gm.GetPuzzlePrefab(RZ_EPuzzleProp.WELL).IsEmpty())
			m_vWell = LandPointNear(s.m_vCenter, s.m_fRadius * 0.2, s.m_fRadius * 0.6);

		PlaceInsideLater(m_vPos);
		m_sTitle = "The Armory";
		m_sDeed = "found the rifles near {town}";
		m_sBrief = string.Format("Our people left weapons in %1 in %2 when it fell: a long case with Bastion stencils, somewhere inside. Find it and bring it to %3. Don't get ideas about keeping it.", place, s.m_sName, meetName);
		m_Mirror.Roll();
		Truth(gm);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Truth(RZ_GameModeComponent gm)
	{
		array<string> values = {"rifles", "emptied", "sold"};
		array<float> weights = {0.5, 0.25, 0};
		RZ_Holding crew = RZ_ErrandCard.HoldingNear(gm, RZ_EFaction.BANDITS, m_vPos, 6000);
		if (crew)
		{
			weights[2] = 0.25;
			m_sCrew = crew.m_sName;
		}

		string truth = m_Job.Draw(this, RZ_WorldFacts.Town(m_iSettlement), values, weights);
		if (truth == "emptied")
			m_fRewardScale = 0.7;

		m_Job.m_vReadAt = m_vPos;
		m_Job.Read("emptied", "police", "The caretaker's lying about nobody having been. Not about the rest.");
		m_Job.Read("sold", "soldier", "The rack log's last line: 'released, by order', a sergeant's signature, no order number.");
		m_Job.Read("sold", "excon,raider", string.Format("Crew chalk on the door frame: %1's hand and a price. Somebody was paid for these.", m_sCrew));

		string reveal = "\"Nobody. Crows. A dog once. The dog took nothing.\"";
		if (truth == "emptied")
			reveal = "\"Scavengers, a week back. Took what they could carry. One case was too heavy for them.\"";
		else if (truth == "sold")
			reveal = string.Format("\"Your sergeant came with a lorry. %1 paid him at the gate. I swept up after.\"", m_sCrew);

		RZ_TalkScene sc = m_Job.Scene("The caretaker", "armory.caretaker", LandPointNear(m_vPos, 5, 10));
		int n0 = sc.Node("An old man with a broom. \"Nobody's been. Except everybody.\"");
		int n1 = sc.Node("He leans on the broom. \"I sweep. I see. Nobody asks the man with the broom.\"");
		sc.Ask(n0, "\"Who's everybody?\"", n1, reveal, true);
		sc.Way(n0, "\"Show me where.\"", RZ_TalkScene.TRUST, RZ_EFaction.MILITARY, 1, WAY_SHOW, "He takes you through the back and kicks the case out from under a bench.", "\"Show you? You've got the uniform and still no idea.\"");
		sc.Way(n0, "A tin for the keys", RZ_TalkScene.FOOD, -1, 1, WAY_SHOW, "He drops a ring of keys in your hand and points with the broom.");
		if (truth == "sold")
			sc.Decide(n1, "Name the sergeant to the post", "name the sergeant to the post", WAY_NAME, "You'll put his name on the form. The caretaker sweeps a little more cheerfully.");
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

		// The sergeant named: the post hears it, the crew hears who
		RZ_OneChoice.Morale(gm, RZ_EFaction.MILITARY, -1, "a sergeant sold the rifles");
		RZ_StoryKit.BlameDecider(gm, this, playerId, RZ_EFaction.BANDITS, 3, "named a crew's seller");
		RZ_Leader l = RZ_StoryKit.Remember(gm, RZ_EFaction.MILITARY, m_vPos, 100000, playerId, RZ_Leaders.MEM_HELPED);
		RZ_StoryKit.Loyalty(gm, l, -3);
		array<int> who = {playerId};
		RZ_StoryKit.Record(this, "sold", RZ_WorldFacts.Town(m_iSettlement), "named", who);
		RZ_StoryKit.Later(gm, playerId, "Bastion's nightly call", "The operator reads a sergeant's name and the words 'reduced to the ranks'. Then the weather, at the same speed.", m_vPos);
		// The scene's done, but the case still has to come out: the next scene person never comes
		if (m_Job.m_Scene)
			m_Job.m_Scene.m_bDone = true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDeliveryTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bGuarded && RZ_ContractHelp.AnyNear(players, m_vPos, 300))
		{
			m_bGuarded = true;
			SpawnGuards(gm, m_vPos, 6, 8, 40, false);
			SpawnGuards(gm, m_vPos, 1, 10, 30, true);
		}

		// The mirror's side has heard: the Red Hand's drop goes up
		if (!m_bRedDrop && m_Mirror.IsHinted() && !gm.GetDropPointPrefab(RZ_EFaction.BANDITS).IsEmpty())
		{
			m_bRedDrop = true;
			vector meet;
			string meetName;
			RZ_Settlement s = gm.GetSettlement(m_iSettlement);
			if (s && RZ_LeaderJob.MeetingPoint(gm, RZ_EFaction.BANDITS, s.m_vCenter, meet, meetName))
				SpawnDrop(gm, RZ_EFaction.BANDITS, meet);
		}

		TickWell(gm, dt, players);
	}

	//------------------------------------------------------------------------------------------------
	//! "Nobody": 10 s at the well with the case, and it goes down.
	protected void TickWell(RZ_GameModeComponent gm, float dt, array<IEntity> players)
	{
		if (m_vWell == vector.Zero || m_eState != RZ_EMissionState.ACTIVE)
			return;

		if (!m_Well && RZ_ContractHelp.AnyNear(players, m_vWell, 300))
			m_Well = RZ_Util.SpawnPrefab(gm.GetPuzzlePrefab(RZ_EPuzzleProp.WELL), RZ_Util.GroundPos(m_vWell), Math.RandomFloat(0, 360));

		IEntity carrier = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iCarrier);
		if (m_iCarrier <= 0 || !carrier || RZ_Util.DistXZ(carrier.GetOrigin(), m_vWell) > 6)
		{
			m_fAtWell = 0;
			return;
		}

		if (m_fAtWell == 0)
			gm.NotifyPlayer(m_iCarrier, m_sTitle, "The well. Ten seconds, and nobody gets these rifles. Nobody.");

		m_fAtWell += dt;
		if (m_fAtWell < WELL_SECONDS)
			return;

		// Nobody: both sides are angry, nobody pays, the world remembers
		RZ_Beneficiaries parties = m_Story.m_Parties;
		int nobody = parties.m_aParties.Count();
		parties.AddNobody("the well");
		m_Story.m_Deciding.Step(gm, m_iCarrier);
		parties.Choose(gm, this, m_iCarrier, nobody, m_vWell);
		gm.AddInfamy(m_iCarrier, RZ_EFaction.MILITARY, WELL_INFAMY, "armory well", RZ_RepFlag.ACT);
		gm.AddInfamy(m_iCarrier, RZ_EFaction.BANDITS, WELL_INFAMY, "armory well", RZ_RepFlag.ACT);
		RZ_WorldFacts.Get().Memory().Record("armory.case", RZ_WorldFacts.Town(m_iSettlement), "the well", RZ_WorldMemory.SOURCE_CHOICE, false);
		NotifyInterested(gm, m_sTitle, "A long splash, a longer silence. The rifles are at the bottom of the well. Bastion and the Red Hand will both want a word.");
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoiceMade(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction)
	{
		if (gm.GetTerritory() && gm.GetTerritory().IsActive())
			gm.GetTerritory().AddStrength(faction, 1);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return super.GetWhereText(gm, from) + "\nDrop: " + RZ_Util.DirectionText(from, GetDropPos(0));
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		RZ_Util.DeleteUnit(m_Well);
	}
}

//------------------------------------------------------------------------------------------------
//! #11 Convoy Escort: Bastion's next supply truck leaves its stronghold for an outpost within two
//! minutes (RZ_ConvoyWatch). Ride along; it's the supply system's own truck. Halfway it breaks
//! down: someone has to stand by it for 30 s (10 s for a Mechanic) to get it going, and if the Red
//! Hand group prefab is set, raiders come for it (Red Hand Infamy for everyone paid).
//! Arrives: paid to those who stayed with it. Robbed, destroyed or late: failed.
class RZ_Mission_ConvoyEscort : RZ_Mission
{
	//------------------------------------------------------------------------------------------------
	void RZ_Mission_ConvoyEscort()
	{
		m_Story = new RZ_MissionStory("convoy");
		m_Choice = new RZ_OneChoice();
		m_Choice.Set("The truck's in. The driver catches your sleeve by the tailgate: \"Two crates fell off on the way. Raiders. You saw them. A hungry house is going to find them on its step tonight.\" Answer him at the truck.",
			"Don't. They go in the store|Let him. Raiders took them", "decide what happened to two crates", 0);
		m_Choice.Asset(1, RZ_CureFinale.INDEP);
	}

	//------------------------------------------------------------------------------------------------
	override void OnChosen(RZ_GameModeComponent gm, int decider, int option)
	{
		if (option != 1)
		{
			RZ_StoryKit.Record(this, "crates", "convoy:" + RZ_SettlementScanner.GridReference(m_vPos), "in store", m_aParticipants);
			return;
		}

		int house = RZ_OneChoice.HungriestHouse(gm);
		if (house >= 0)
			gm.GetSafehouses().AddFood(house, 2);

		NotifyParticipants(gm, m_sTitle, "Two crates are \"lost to raiders\". Somewhere a hungry house finds them on its step and doesn't ask.");
		if (decider > 0)
			gm.GetFacts().Memory().Lies().Tell(gm, decider, "convoy.crates", "convoy:" + RZ_SettlementScanner.GridReference(m_vPos), RZ_EFaction.MILITARY, "", m_vPos, RZ_ELieTest.RUMOUR, 8,
				"Bastion's quartermaster counted the house's crates back to the convoy's manifest. Two of them had the post's stencil. Somebody said raiders.");

		RZ_StoryKit.Record(this, "crates", "convoy:" + RZ_SettlementScanner.GridReference(m_vPos), "lost to raiders", m_aParticipants);
	}

	protected ref RZ_ConvoyWatch m_Watch = new RZ_ConvoyWatch();
	protected ref map<int, int> m_mNear = new map<int, int>();
	protected int m_iChecks;
	protected float m_fCheckTimer;
	protected float m_fWaiting;
	protected float m_fTripLength;
	protected bool m_bBroken;
	protected bool m_bFixed;
	protected float m_fRepair;
	protected IEntity m_Raiders;

	protected static const float CHECK_SECONDS = 30;
	protected static const float NEAR = 150;
	protected static const float REPAIR_SECONDS = 30;
	protected static const float MECHANIC_SECONDS = 10;

	//------------------------------------------------------------------------------------------------
	//! Its position follows the moving truck: nothing of its own stands anywhere, so no building is kept unlocked.
	override void GetPropSpots(notnull array<vector> result)
	{
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.TRUSTED, 40, 35);
		m_Req.Bonus("mechanic");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!RZ_ConvoyWatch.CanRun(gm, RZ_EFaction.MILITARY))
			return 0;

		return 1.2;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		if (!m_Watch.Request(gm, RZ_EFaction.MILITARY, 120))
			return false;

		m_vPos = m_Watch.GetPos(gm);
		m_iSettlement = gm.FindNearestSettlement(m_vPos);
		m_sTitle = "Convoy Escort";
		m_sDeed = "brought the convoy through {town}";
		m_sBrief = string.Format("A supply truck leaves %1 for our post in %2 in two minutes. Ride with it and get it there. It will break down; they always do.", m_Watch.m_sFrom, m_Watch.m_sTo);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_Watch.Update(gm);
		m_vPos = m_Watch.GetPos(gm);
		if (m_Watch.m_iResult == RZ_SupplyLines.TRUCK_CALLED_OFF)
		{
			// Nobody's fault: the post changed hands before the truck got there
			Withdraw(string.Format("Called off. The post in %1 isn't ours any more, so the truck turned round.", m_Watch.m_sTo));
			return;
		}

		if (m_Watch.m_iResult >= 0)
		{
			if (m_Watch.m_iResult == RZ_SupplyLines.TRUCK_ARRIVED)
			{
				PayEscorts();
				m_eState = RZ_EMissionState.SUCCEEDED;
			}
			else
			{
				m_eState = RZ_EMissionState.FAILED;
			}

			return;
		}

		RZ_SupplyTruck t = m_Watch.GetTruck(gm);
		if (!t)
		{
			m_fWaiting += dt;
			if (!m_Watch.HasLeft() && m_fWaiting > 900)
				m_eState = RZ_EMissionState.FAILED;

			return;
		}

		if (!t.m_Vehicle)
			return;

		vector at = t.m_Vehicle.GetOrigin();
		if (m_fTripLength <= 0)
			m_fTripLength = Math.Max(1, RZ_Util.DistXZ(at, t.m_vDest));

		m_fCheckTimer += dt;
		if (m_fCheckTimer >= CHECK_SECONDS)
		{
			m_fCheckTimer = 0;
			m_iChecks++;
			foreach (int i, IEntity p : players)
			{
				if (RZ_Util.DistXZ(p.GetOrigin(), at) > NEAR)
					continue;

				int seen;
				m_mNear.Find(ids[i], seen);
				m_mNear.Set(ids[i], seen + 1);
			}
		}

		UpdateBreakdown(gm, dt, t, at, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateBreakdown(RZ_GameModeComponent gm, float dt, RZ_SupplyTruck t, vector at, array<int> ids, array<IEntity> players)
	{
		ResourceName wp = gm.GetSpawner().GetMoveWaypointPrefab();
		if (!m_bBroken)
		{
			float left = RZ_Util.DistXZ(at, t.m_vDest) / m_fTripLength;
			if (left > 0.6 || left < 0.3 || CountPlayersNear(players, at, NEAR) == 0 || t.m_bCrewDead)
				return;

			m_bBroken = true;
			RZ_Util.RedirectGroup(t.m_Crew, at, wp);
			RZ_ContractHelp.NotifyNear(gm, at, NEAR + 50, m_sTitle, "A bang from under the bonnet and the truck rolls to a stop. The driver gets out swearing. Someone has to stand with it and help.");
			SendRaiders(gm, at);
			return;
		}

		if (m_bFixed)
			return;

		// Someone by the truck gets it going; a Mechanic in a third of the time
		float rate = 0;
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), at) > 6)
				continue;

			float r = 1;
			if (HasBonus(gm, ids[i]))
				r = REPAIR_SECONDS / MECHANIC_SECONDS;

			rate = Math.Max(rate, r);
		}

		m_fRepair += dt * rate;
		if (m_fRepair < REPAIR_SECONDS)
			return;

		m_bFixed = true;
		RZ_Util.RedirectGroup(t.m_Crew, t.m_vDest, wp);
		RZ_ContractHelp.NotifyNear(gm, at, NEAR + 50, m_sTitle, "It coughs back to life. The driver slaps the door twice and pulls away.");
	}

	//------------------------------------------------------------------------------------------------
	protected void SendRaiders(RZ_GameModeComponent gm, vector at)
	{
		ResourceName prefab = gm.GetRedHandGroupPrefab();
		if (prefab.IsEmpty())
			return;

		m_Raiders = RZ_Util.SpawnPrefab(prefab, LandPointNear(at, 250, 350));
		if (!m_Raiders)
			return;

		RZ_Util.SendGroupTo(m_Raiders, at, gm.GetSpawner().GetMoveWaypointPrefab());
		RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, m_Raiders);
		SetRival(RZ_EFaction.BANDITS, 10);
		RZ_ContractHelp.NotifyNear(gm, at, NEAR + 50, m_sTitle, "Movement on the ridge. The Red Hand heard the bang too.");
	}

	//------------------------------------------------------------------------------------------------
	//! Paid: players who were with the truck for at least 40% of the checks (two at least), and
	//! neither they nor anyone in their group shot near it while its crew lived.
	protected void PayEscorts()
	{
		m_aParticipants.Clear();
		int needed = Math.Max(2, RZ_Util.RoundToInt(m_iChecks * 0.4));
		foreach (int playerId, int seen : m_mNear)
		{
			if (seen >= needed && !m_Watch.ShotBy(playerId))
				m_aParticipants.Insert(playerId);
		}
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
		return string.Format("Convoy Escort: the truck to %1 never made it.", m_Watch.m_sTo);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		RZ_Util.DeleteUnit(m_Raiders);
	}
}

//------------------------------------------------------------------------------------------------
//! #12 Rescue the Pilot: a Bastion pilot went down outside a town. Reach him, help him up and he
//! follows you (RZ_Escort) to Bastion. He's bleeding: 20 minutes from when you find him, unless a
//! Nurse or Doctor patches him up (★). An AI body that dies fails it.
//! A story job (key `pilot`, kept per helicopter): a crash, as they said; he put her down on purpose
//! (the flight plan ends at the line); or he carried a crew's cargo (a Red Hand holding within 6 km).
//! The farmer whose cabbages he landed in (the talk scene) lends a cart ([Known - Bastion], or two
//! tins): he rides in, no bleeding, in under three minutes. At the post the sergeant asks: the
//! engine; ditched: report it (Bastion morale -1) or say it was the engine (a lie the rumour round
//! can find, 60%, an Independent asset); cargo: tell the crew he's home (10 Red Hand Favor, Bastion
//! pays nothing); a crash: ask the post to pay the farmer (half pay, a callback at his gate).
class RZ_Mission_RescueThePilot : RZ_PuzzleMission
{
	protected bool m_bCart;
	protected float m_fCart;
	protected string m_sCrew = "a crew";

	protected static const int WAY_CART = 0;
	protected static const float CART_SECONDS = 150;
	protected static const int OPT_POST = 0;
	protected static const int OPT_OTHER = 1;

	//------------------------------------------------------------------------------------------------
	void RZ_Mission_RescueThePilot()
	{
		m_Story = new RZ_MissionStory("pilot");
		m_Choice = new RZ_OneChoice();
		m_Job = new RZ_StoryJob();
	}

	//------------------------------------------------------------------------------------------------
	//! The helicopter's crash is the story: it holds the helicopter (a claim, docs/DESIGN.md "World facts")
	override void GetClaims(RZ_GameModeComponent gm, notnull array<string> result)
	{
		result.Insert(RZ_WorldFacts.HELI);
	}

	protected ref RZ_Escort m_Escort = new RZ_Escort();
	protected vector m_vWreck;
	protected vector m_vDest;
	protected string m_sDestName;
	protected bool m_bShown;
	protected bool m_bFound;
	protected bool m_bPatched;
	protected float m_fBleed;
	protected float m_fBeside;
	protected int m_iTalk = -1;

	protected static const float BLEED_SECONDS = 1200;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.TRUSTED, 35, 30);
		m_Req.Bonus("nurse");
		m_Req.Bonus("doctor");
		m_Req.m_fMinMilitary = RZ_Quest_LastPilot.MIN_PRESENCE;
		m_Req.m_bNeedsHeli = true;
		// Not while The Last Pilot's story has its pilot
		m_Req.NotWhileQuest("pilot");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!RZ_Escort.HasRoom(gm) || !HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return 0;

		// Only while the helicopter flies (m_bNeedsHeli; the questline's own grounding too)
		if (RZ_Quest_LastPilot.IsHelicopterDown(gm))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlementNotHeldBy(gm, 20, 95, RZ_Factions.Bit(RZ_EFaction.BANDITS), this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vWreck = LandPointNear(s.m_vCenter, s.m_fRadius + 100, s.m_fRadius + 500);
		if (!RZ_LeaderJob.MeetingPoint(gm, RZ_EFaction.MILITARY, m_vWreck, m_vDest, m_sDestName))
			return false;

		m_vPos = m_vWreck;
		m_iTalk = AddClue("The pilot", "\"Took you long enough.\" His flight suit is dark down one side. \"Leg's bad. I can walk if you go slow.\"");
		m_Escort.m_sName = "the pilot";
		m_Escort.m_sPrefab = RZ_Escort.PrefabFor(gm, 2);
		m_Escort.m_fFollowGap = 5;
		m_Escort.SetTalk(m_iId, 0, "Help him up (he follows you)|Patch him up (Nurse or Doctor)", m_iTalk, "Talk to the pilot");
		m_sTitle = "Rescue the Pilot";
		m_sDeed = "brought the pilot in from {town}";
		m_sBrief = string.Format("The last helicopter put down hard outside %1 on its run. The mechanics will get the airframe back; the pilot is still out there, hurt, and Bastion has nobody else to fly it. Find him and walk him to %2.", s.m_sName, m_sDestName);
		if (!m_Escort.Begin(gm, LandPointNear(m_vWreck, 3, 8)))
			return false;

		Truth(gm);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Truth(RZ_GameModeComponent gm)
	{
		array<string> values = {"crash", "ditched", "cargo"};
		array<float> weights = {0.5, 0.3, 0};
		RZ_Holding crew = RZ_ErrandCard.HoldingNear(gm, RZ_EFaction.BANDITS, m_vWreck, 6000);
		if (crew)
		{
			weights[2] = 0.2;
			m_sCrew = crew.m_sName;
		}

		string truth = m_Job.Draw(this, RZ_WorldFacts.HELI, values, weights);
		m_Job.m_vReadAt = m_vWreck;
		m_Job.Read("crash", "mechanic", "A hydraulic line split clean through. She didn't land. She fell, and he flew the fall.");
		m_Job.Read("ditched", "mechanic", "Nothing wrong with this engine. He put her down gently and walked away from a good aircraft.");
		m_Job.Read("ditched", "soldier", "The flight plan clipped in the cockpit doesn't end at the post. It ends at the line.");
		m_Job.Read("cargo", "excon,raider", string.Format("Handcart ruts from the cargo door to the road. That's %1's pickup, and they knew where to wait.", m_sCrew));

		string reveal = "\"Smoke, then a bang, then cabbages everywhere. He swore all the way down. I liked him.\"";
		if (truth == "ditched")
			reveal = "\"No smoke. He came down slow as you like, sat a while, then tried to walk off east. His leg had other ideas.\"";
		else if (truth == "cargo")
			reveal = "\"Two men with a handcart got here before me. They took a crate and left him. Polite about it.\"";

		RZ_TalkScene sc = m_Job.Scene("The farmer", "pilot.farmer", LandPointNear(m_vWreck, 12, 20));
		int n0 = sc.Node("A farmer leans on a fence. \"He landed in my cabbages. I'd like that written down somewhere.\"");
		int n1 = sc.Node("\"He asked me not to tell anyone. So I'm telling you quietly.\"");
		sc.Ask(n0, "\"What did you see?\"", n1, reveal, true);
		sc.Way(n0, "\"Lend us your cart.\"", RZ_TalkScene.TRUST, RZ_EFaction.MILITARY, 1, WAY_CART, "He brings the cart round and you lift the pilot on. \"Mind the cabbages.\"", "\"Bastion borrowed my last cart. I'm still waiting.\"");
		sc.Way(n0, "Two tins for the cart", RZ_TalkScene.FOOD, -1, 2, WAY_CART, "The tins buy the cart and the farmer's silence about the cabbages.");
		sc.Service(n1, "Fix his water pump", "mechanic", 25, WAY_CART, "The pump coughs and runs. \"Take the cart. Take the pump if you like.\"");

		string labels = "Engine failure";
		if (truth == "ditched")
		{
			labels = "Report what he did|Say it was the engine";
			m_Choice.Asset(OPT_OTHER, RZ_CureFinale.INDEP);
		}
		else if (truth == "cargo")
		{
			labels = "Report the cargo|Tell the crew he's home";
			m_Choice.Asset(OPT_OTHER, RZ_EFaction.BANDITS);
		}
		else
		{
			labels += "|Ask the post to pay the farmer";
		}

		m_Choice.Set("The post's medic gets him on a stretcher. The duty sergeant has a form. \"What happened out there?\"", labels, "decide what goes on the form", OPT_POST);
	}

	//------------------------------------------------------------------------------------------------
	override vector ChoiceAt()
	{
		return m_vDest;
	}

	//------------------------------------------------------------------------------------------------
	override void OnStoryWay(RZ_GameModeComponent gm, int playerId, int way)
	{
		m_bCart = true;
		m_bFound = true;
		m_bPatched = true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnChosen(RZ_GameModeComponent gm, int decider, int option)
	{
		array<int> who = {decider};
		string truth = m_Job.m_sTruth;
		if (truth == "ditched" && option == OPT_POST)
		{
			RZ_OneChoice.Morale(gm, RZ_EFaction.MILITARY, -1, "a pilot who tried for the line");
			RZ_StoryKit.Record(this, "fate", RZ_WorldFacts.HELI, "reported", who);
			return;
		}

		if (truth == "ditched")
		{
			m_fRewardScale = 0.6;
			RZ_StoryKit.Dirty(gm, this, decider);
			gm.GetFacts().Memory().Lies().Tell(gm, decider, "pilot.engine", RZ_WorldFacts.HELI, RZ_EFaction.MILITARY, "", m_vDest, RZ_ELieTest.RUMOUR, 6, "\"The engine was fine. You knew. He told the mechanic, the mechanic told the cook.\"");
			RZ_StoryKit.Record(this, "fate", RZ_WorldFacts.HELI, "covered", who);
			return;
		}

		if (truth == "cargo" && option == OPT_OTHER)
		{
			m_bHasGiver = false;
			m_Choice.PayWith(gm, this, RZ_EFaction.BANDITS, 10);
			RZ_StoryKit.Record(this, "fate", RZ_WorldFacts.HELI, "crew", who);
			NotifyParticipants(gm, m_sTitle, string.Format("The form says engine failure. %1 hears he's home and keeps paying him.", m_sCrew));
			return;
		}

		if (truth == "crash" && option == OPT_OTHER)
		{
			m_fRewardScale = 0.5;
			RZ_StoryKit.Record(this, "fate", RZ_WorldFacts.HELI, "farmer", who);
			RZ_StoryKit.LaterWith(gm, this, "The farmer", "A man at the gate with a sack. \"Bastion paid for the cabbages. In tins. Four of them are cabbages.\"", m_vWreck, true);
			return;
		}

		RZ_StoryKit.Record(this, "fate", RZ_WorldFacts.HELI, truth, who);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bShown && RZ_ContractHelp.AnyNear(players, m_vWreck, 300))
		{
			m_bShown = true;
			IEntity wreck = RZ_Util.SpawnPrefab(gm.GetHeliWreckPrefab(), m_vWreck, Math.RandomFloat(0, 360));
			TrackProp(wreck);
			SpawnGuards(gm, m_vWreck, 4, 15, 50);
		}

		if (!m_bFound && ReachedBy(ids, players, m_Escort.m_vPos, 20))
		{
			m_bFound = true;
			RZ_ContractHelp.NotifyNear(gm, m_Escort.m_vPos, 40, m_sTitle, "He's alive, propped against a tree with his sidearm in his lap. He's lost a lot of blood. Twenty minutes, maybe.");
		}

		// The farmer's cart: he rides in, and is there in under three minutes
		if (m_bCart)
		{
			m_fCart += dt;
			if (m_fCart >= CART_SECONDS)
			{
				m_Escort.Arrive();
				m_eState = RZ_EMissionState.SUCCEEDED;
				return;
			}
		}

		if (m_bFound && !m_bPatched)
		{
			m_fBleed += dt;
			if (m_fBleed >= BLEED_SECONDS)
			{
				m_eState = RZ_EMissionState.FAILED;
				return;
			}
		}

		// Nobody can talk to his body (no talk components): stay beside him and he gets up anyway
		if (m_bFound && m_Escort.GetFollowed() <= 0)
		{
			int beside = RZ_EscortHelp.PlayerBeside(ids, players, m_Escort.m_vPos, 4);
			if (beside > 0)
				m_fBeside += dt;

			if (beside > 0 && m_fBeside >= 10)
				OnChoice(gm, beside, 0, 0, null);
		}

		m_Escort.Update(gm, dt, ids, players);
		m_vPos = m_Escort.m_vPos;
		if (m_Escort.GetFollowed() > 0)
			RZ_ContractHelp.AddNear(ids, players, m_vPos, 100, m_aParticipants);

		if (m_Escort.IsDead())
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (m_Escort.GetFollowed() > 0 && m_Escort.IsNear(m_vDest, 25))
		{
			m_Escort.Arrive();
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0)
			return;

		m_bFound = true;
		if (option == 1)
		{
			if (m_bPatched)
			{
				gm.NotifyPlayer(playerId, m_sTitle, "He's already bandaged. It's holding.");
				return;
			}

			if (!HasBonus(gm, playerId))
			{
				gm.NotifyPlayer(playerId, m_sTitle, "You press on the wound and it soaks through your fingers. You don't know how to stop this. Get him moving.");
				return;
			}

			m_bPatched = true;
			gm.NotifyPlayer(playerId, m_sTitle, "Tourniquet, packing, pressure. The bleeding stops. \"You've done this before.\" He'll make it if you get him there.");
			return;
		}

		if (option != 0)
			return;

		m_Escort.Follow(playerId);
		m_Escort.Go();
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("He gets an arm over your shoulder, then lets go and limps after you. Take him to %1 (%2).", m_sDestName, RZ_Util.DirectionText(m_Escort.m_vPos, m_vDest)));
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Rescue the Pilot: the pilot didn't make it. The helicopter stays on the ground until someone else can fly it.";
	}

	//------------------------------------------------------------------------------------------------
	//! He was the one flying it: no Last Helicopter until a new pilot is found (The Last Pilot's flags)
	override void OnFailed(RZ_GameModeComponent gm)
	{
		RZ_Quest_LastPilot.OnPilotLost(gm);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Escort.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! #18 Secure the Shipment: a Halcyon courier walks a sample case to the lab. Bastion wants it.
//! Escort her (RZ_Escort); on the way, talk to her: **see it to the lab** (Halcyon pays, research)
//! or **divert it to Bastion** (Bastion pays, Halcyon Infamy). Whoever gets it pays, as in a
//! choice mission.
class RZ_Mission_SecureTheShipment : RZ_PuzzleMission
{
	protected ref RZ_Escort m_Escort = new RZ_Escort();
	protected vector m_vLab;
	protected vector m_vBastion;
	protected string m_sBastionName;
	protected bool m_bMet;
	protected bool m_bDiverted;
	protected int m_iTalk = -1;

	protected static const int CROSS_INFAMY = 15;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.CHAMPION, 45, 40);
		m_Req.m_bNotCured = true;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!gm.HasLab() || !RZ_Escort.HasRoom(gm) || !HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_vLab = gm.GetLabPosition();
		m_iSettlement = PickSettlementAwayNotHeldBy(gm, m_vLab, 1000, 4000, 0, 90, RZ_Factions.Bit(RZ_EFaction.BANDITS), -1, this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		if (!RZ_LeaderJob.MeetingPoint(gm, RZ_EFaction.MILITARY, s.m_vCenter, m_vBastion, m_sBastionName))
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.5, s.m_fRadius + 40);
		m_iTalk = AddClue("The courier", "A Halcyon courier with a steel case chained to her wrist. \"Samples for the lab. Are you my escort? You don't look like Bastion.\"");
		m_Escort.m_sName = "the courier";
		m_Escort.m_sPrefab = RZ_Escort.PrefabFor(gm, 1);
		m_Escort.SetTalk(m_iId, 0, "See it to the lab|Divert it to Bastion", m_iTalk, "Talk to the courier");
		m_Escort.SetDestination(LandPointNear(m_vLab, 6, 12));
		m_sTitle = "Secure the Shipment";
		m_sBrief = string.Format("Halcyon is walking a case of samples from %1 to their lab. We want it. Walk with the courier and, when nobody's looking, bring it to %2 instead. Or don't, and answer for it.", s.m_sName, m_sBastionName);
		return m_Escort.Begin(gm, m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bMet && ReachedBy(ids, players, m_Escort.m_vPos, 25))
		{
			m_bMet = true;
			m_Escort.Go();
			RZ_ContractHelp.NotifyNear(gm, m_Escort.m_vPos, 50, m_sTitle, "She sets off for the lab without waiting to be asked. Talk to her if you mean to change that.");
		}

		m_Escort.Update(gm, dt, ids, players);
		m_vPos = m_Escort.m_vPos;
		RZ_ContractHelp.AddNear(ids, players, m_vPos, 100, m_aParticipants);
		if (m_Escort.IsDead())
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (!m_Escort.IsArrived())
			return;

		if (m_bDiverted)
		{
			SetRival(RZ_EFaction.INSTITUTE, CROSS_INFAMY);
		}
		else
		{
			// It reached the lab: Halcyon pays instead
			m_eGiver = RZ_EFaction.INSTITUTE;
			gm.GetCure().AddResearch(2, "A sample shipment reached the lab under escort.");
		}

		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0)
			return;

		if (option == 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"Good.\" She keeps walking toward the lab.");
			return;
		}

		if (option != 1 || m_bDiverted)
			return;

		m_bDiverted = true;
		m_Escort.SetDestination(m_vBastion);
		m_Escort.Go();
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("You tell her where she's going now. She looks at your rifle, then at the road. \"They'll know it was you.\" She walks toward %1.", m_sBastionName));
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return string.Format("%1, the courier is %2\nLab: %3. Bastion: %4", GetLocationName(gm), RZ_Util.DirectionText(from, m_vPos), RZ_Util.DirectionText(from, m_vLab), RZ_Util.DirectionText(from, m_vBastion));
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Escort.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! #19 Blow the Bridge: a horde is walking toward a safe town. Plant charges at the narrow point
//! on its way (three, two with a Mechanic or Soldier there; 10 s each), then blow them from the
//! detonator before it passes. It turns back. It reaches the town: +10 infection there, failed.
//! Killed to the last instead: the town is safe, paid all the same. Each charge spot is a puzzle
//! object (use it to see how far its wiring is). The clock covers the horde's whole walk.
//! With the packs on, first a pack crossing a real bridge (the land grid's) toward a safe town beyond
//! it: the horde is its hunters and young, in its look, from its lair, and the charges go at the
//! bridge. *Simplified* otherwise: no real bridge, a choke point on the line between the horde and
//! the town. The horde is data (walking 1 m/s) until a player is within 600 m.
class RZ_Mission_BlowTheBridge : RZ_PuzzleMission
{
	//------------------------------------------------------------------------------------------------
	void RZ_Mission_BlowTheBridge()
	{
		m_Story = new RZ_MissionStory("bridge");
		m_Choice = new RZ_OneChoice();
		RZ_HordeWhere.Set(m_Choice, RZ_HordeWhere.EMPTY);
	}

	//------------------------------------------------------------------------------------------------
	override void OnChosen(RZ_GameModeComponent gm, int decider, int option)
	{
		RZ_HordeWhere.Apply(gm, this, option);
	}

	protected int m_iPack = -1;			// the pack crossing (-1: a horde from a town)
	protected vector m_vTown;
	protected vector m_vChoke;
	protected vector m_vHorde;
	protected vector m_vDetonator;
	protected ref array<vector> m_aCharges = {};
	protected ref array<float> m_aPlanted = {};
	protected int m_iPlanted;
	protected int m_iNeeded = 3;
	protected bool m_bBonusChecked;
	protected bool m_bDetonatorShown;
	protected bool m_bBlown;
	protected bool m_bTurned;
	protected bool m_bHordeSpawned;
	protected bool m_bHordeSeen;		// its bodies were there once: gone again means killed (mission units never despawn)
	protected bool m_bReached;
	protected float m_fStartIn = 300;

	protected static const float HORDE_SPEED = 1;
	protected static const float CHARGE_SECONDS = 10;
	protected static const float SPARE_SECONDS = 600;	// on top of the horde's walk, before the job lapses

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.CHAMPION, 45, 40);
		m_Req.Bonus("mechanic");
		m_Req.Bonus("soldier");
		m_fTimeLeft = 3000;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.OBJECT) || gm.GetSpawner().GetMoveWaypointPrefab().IsEmpty())
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		if (SetupFromPack(gm))
			return true;

		m_iSettlement = PickSettlement(gm, 0, 35);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		RZ_Settlement src = gm.GetSettlement(PickEmptySettlementAway(gm, s.m_vCenter, 1500, 3000, 50, 100, m_iSettlement, this));
		if (!src)
			return false;

		m_vTown = s.m_vCenter;
		m_vHorde = LandPointNear(src.m_vCenter, 0, src.m_fRadius * 0.5);
		m_vChoke = RZ_ContractHelp.Between(m_vHorde, m_vTown, 0.6);
		PlaceCharges();

		// The clock covers the horde's whole walk, so it never lapses before they get there
		m_fTimeLeft = m_fStartIn + RZ_Util.DistXZ(m_vHorde, m_vTown) / HORDE_SPEED + SPARE_SECONDS;
		m_sTitle = "Blow the Bridge";
		m_sBrief = string.Format("A horde out of %1 is walking toward %2. There's a narrow point on its way at grid %3. Plant the charges there and blow it before they pass. You have about five minutes before they move.",
			src.m_sName, s.m_sName, RZ_SettlementScanner.GridReference(m_vChoke));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! With the packs on: a free pack with a bridge (not a ford) on its way, between its lair and a
	//! safe town beyond it (400 m to 3 km past the bridge, the same way on).
	protected bool SetupFromPack(RZ_GameModeComponent gm)
	{
		RZ_Tribes tribes = RZ_TribeContract.Tribes(gm);
		RZ_Land land = gm.GetLand();
		if (!tribes || !land || !land.IsReady())
			return false;

		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			string subject = RZ_TribeContract.Claim(t.m_iId);
			if (t.IsRover() || !IsSubjectFree(gm, subject))
				continue;

			for (int c = 0; c < land.GetCrossingCount(); c++)
			{
				if (land.GetCrossingKind(c) == RZ_Land.CROSS_FORD)
					continue;

				vector bridge = land.GetCrossing(c);
				float fromLair = RZ_Util.DistXZ(bridge, t.m_vLair);
				if (fromLair < 300 || fromLair > t.m_fRadius * 1.5 + 500)
					continue;

				int town = TownBeyond(gm, t.m_vLair, bridge);
				RZ_Settlement s = gm.GetSettlement(town);
				if (!s || IsTownBusy(gm, town) || !ClaimSubject(gm, subject))
					continue;

				m_iPack = t.m_iId;
				m_iSettlement = town;
				m_vTown = s.m_vCenter;
				m_vHorde = t.m_vLair;
				m_vChoke = RZ_Util.GroundPos(bridge);
				PlaceCharges();
				m_fTimeLeft = m_fStartIn + RZ_Util.DistXZ(m_vHorde, m_vTown) / HORDE_SPEED + SPARE_SECONDS;
				m_sTitle = "Blow the Bridge";
				m_sBrief = string.Format("%1 will come over the bridge at grid %2, toward %3. Plant the charges on the bridge and blow it before they cross. You have about five minutes before they move.",
					RZ_Tribes.Capitalize(tribes.GetPeople().NameAt(t, RZ_EFaction.MILITARY)), RZ_SettlementScanner.GridReference(m_vChoke), s.m_sName);
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A safe town (infection 35 or less) 400 m to 3 km past the bridge, the way on from the lair; -1.
	protected int TownBeyond(RZ_GameModeComponent gm, vector lair, vector bridge)
	{
		vector dir = bridge - lair;
		dir[1] = 0;
		if (dir.Length() < 1)
			return -1;

		dir.Normalize();
		int best = -1;
		float bestD = 3000;
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			if (s.m_fInfection > 35)
				continue;

			vector to = s.m_vCenter - bridge;
			to[1] = 0;
			float d = to.Length();
			if (d < 400 || d > bestD)
				continue;

			to.Normalize();
			if (vector.Dot(dir, to) < 0.3)
				continue;

			best = i;
			bestD = d;
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Three charge spots at the narrow point, and the detonator back from it.
	protected void PlaceCharges()
	{
		for (int i = 0; i < 3; i++)
		{
			m_aCharges.Insert(LandPointNear(m_vChoke, 6, 20));
			m_aPlanted.Insert(0);
		}

		m_vDetonator = LandPointNear(m_vChoke, 45, 60);
		m_vPos = m_vChoke;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		UpdateHorde(gm, dt, ids, players);
		if (m_eState != RZ_EMissionState.ACTIVE)
			return;

		if (!m_bDetonatorShown && RZ_ContractHelp.AnyNear(players, m_vChoke, 300))
		{
			m_bDetonatorShown = true;
			SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vDetonator, 0, "", "Blow the charges");
			foreach (int n, vector spot : m_aCharges)
			{
				SpawnObject(gm, RZ_EPuzzleProp.OBJECT, spot, n + 1, "Check the charge", "");
			}

			RZ_ContractHelp.NotifyNear(gm, m_vChoke, 300, m_sTitle, string.Format("The charges are stacked at the narrow point (grid %1). Stand by each for %2 seconds to wire it, then blow them from the detonator.", ChargeGrids(), RZ_Util.RoundToInt(CHARGE_SECONDS)));
		}

		if (!m_bBonusChecked && AnyBonusNear(gm, ids, players, m_vChoke, 40))
		{
			m_bBonusChecked = true;
			m_iNeeded = 2;
			RZ_ContractHelp.NotifyNear(gm, m_vChoke, 60, m_sTitle, "You know where to put them: two charges will do the work of three.");
		}

		foreach (int i, vector c : m_aCharges)
		{
			if (m_aPlanted[i] >= CHARGE_SECONDS || m_iPlanted >= m_iNeeded)
				continue;

			if (!ReachedBy(ids, players, c, 3, true))	// wiring a charge: at it
				continue;

			m_aPlanted[i] = m_aPlanted[i] + dt;
			if (m_aPlanted[i] < CHARGE_SECONDS)
				continue;

			m_iPlanted++;
			RZ_ContractHelp.NotifyNear(gm, m_vChoke, 60, m_sTitle, string.Format("Charge %1 of %2 set. Get clear before you blow them.", m_iPlanted, m_iNeeded));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The charge spots not wired yet, by grid: "045-112, 046-112".
	protected string ChargeGrids()
	{
		string text = "";
		foreach (int i, vector c : m_aCharges)
		{
			if (m_aPlanted[i] >= CHARGE_SECONDS)
				continue;

			if (!text.IsEmpty())
				text += ", ";

			text += RZ_SettlementScanner.GridReference(c);
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = super.GetWhereText(gm, from);
		string grids = ChargeGrids();
		if (!m_bBlown && !grids.IsEmpty())
			text += "\nCharges (grid): " + grids;

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! A charge spot: says how far its wiring is.
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		int idx = objIdx - 1;
		if (idx < 0 || idx >= m_aCharges.Count())
			return;

		if (m_aPlanted[idx] >= CHARGE_SECONDS)
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("This one's wired. %1 of %2 set.", m_iPlanted, m_iNeeded));
			return;
		}

		if (m_iPlanted >= m_iNeeded)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "Enough of them are wired. Get to the detonator.");
			return;
		}

		gm.NotifyPlayer(playerId, m_sTitle, string.Format("Stay right here while you wire it: %1 more seconds.", RZ_Util.RoundToInt(CHARGE_SECONDS - m_aPlanted[idx])));
	}

	//------------------------------------------------------------------------------------------------
	//! Data until someone is near, then real groups walking to the town.
	protected void UpdateHorde(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bTurned)
			return;

		if (m_fStartIn > 0)
		{
			m_fStartIn -= dt;
			return;
		}

		if (m_bHordeSpawned)
		{
			array<IEntity> units = {};
			gm.GetSpawner().GetMissionUnits(m_iId, units);
			if (!units.IsEmpty())
			{
				m_bHordeSeen = true;
				m_vHorde = RZ_Util.GetUnitPosition(units[0]);
			}
			else if (m_bHordeSeen)
			{
				// Killed to the last (mission units never despawn): nothing is left to turn back
				m_bTurned = true;
				RZ_ContractHelp.AddNear(ids, players, m_vHorde, 300, m_aParticipants);
				RZ_ContractHelp.NotifyNear(gm, m_vHorde, 600, m_sTitle, string.Format("The last of them goes down. Nothing is walking to %1 now. Bastion pays all the same.", GetLocationName(gm)));
				m_eState = RZ_EMissionState.SUCCEEDED;
				return;
			}
			else
			{
				m_bHordeSpawned = false;	// nothing came of the spawn: walk on as data
			}
		}
		else
		{
			float dist = RZ_Util.DistXZ(m_vHorde, m_vTown);
			float step = Math.Min(HORDE_SPEED * dt, dist);
			if (dist > 0.1)
				m_vHorde = RZ_Util.GroundPos(m_vHorde + (m_vTown - m_vHorde) * (step / dist));

			if (RZ_ContractHelp.AnyNear(players, m_vHorde, 600))
			{
				m_bHordeSpawned = true;
				// A pack's crossing: its own hunters and young
				array<ResourceName> pack = null;
				RZ_Tribes tribes = RZ_TribeContract.Tribes(gm);
				if (m_iPack >= 0 && tribes && tribes.Find(m_iPack))
					pack = tribes.GetPeople().PackHordePrefabs(tribes.Find(m_iPack));

				gm.GetSpawner().SpawnHordeAt(m_vHorde, m_vTown, m_iId, null, true, pack);
			}
		}

		if (RZ_Util.DistXZ(m_vHorde, m_vTown) > 120)
			return;

		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
			s.AddInfection(10);

		m_bReached = true;
		m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0 || option != 0 || m_bBlown)
			return;

		if (m_iPlanted < m_iNeeded)
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("The board shows %1 of %2 charges live. Plant the rest first.", m_iPlanted, m_iNeeded));
			return;
		}

		m_bBlown = true;
		ResourceName fx = gm.GetExplosionPrefab();
		foreach (vector c : m_aCharges)
		{
			IEntity boom = RZ_Util.SpawnPrefab(fx, c);
			TrackProp(boom);
		}

		gm.GetHeat().AddHeat(m_vChoke, 40);

		// Past the choke point already? Then it's too late
		float hordeLeft = RZ_Util.DistXZ(m_vHorde, m_vTown);
		float chokeLeft = RZ_Util.DistXZ(m_vChoke, m_vTown);
		if (hordeLeft < chokeLeft - 20)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "The ground jumps and earth rains down, behind them. They're already past. Get to the town ahead of them.");
			return;
		}

		m_bTurned = true;

		// The way is gone: they turn back the way they came
		array<IEntity> units = {};
		gm.GetSpawner().GetMissionUnits(m_iId, units);
		vector away = m_vHorde + (m_vHorde - m_vTown) * 0.5;
		foreach (IEntity u : units)
		{
			RZ_Util.RedirectGroup(u, RZ_Util.GroundPos(away), gm.GetSpawner().GetMoveWaypointPrefab());
		}

		m_aParticipants.Insert(playerId);
		RZ_ContractHelp.NotifyNear(gm, m_vChoke, 400, m_sTitle, "The narrow point goes up in a sheet of dirt and smoke. When it clears, the way is gone and they're turning back.");
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (m_bReached)
			return string.Format("Blow the Bridge: the horde reached %1.", GetLocationName(gm));

		return string.Format("Blow the Bridge: nobody blew the narrow point, and the horde is still on the road to %1.", GetLocationName(gm));
	}
}

//------------------------------------------------------------------------------------------------
//! #20 Fire Mission: three hordes mill in infested towns. Stand within 80 m of one for 10 s to
//! mark it with smoke; 30 s later Bastion's guns hit it (an explosion per shell and every infected
//! there dies). Get clear: the shells don't check who's standing there. All three: paid.
//! *Simplified:* the strike kills the marked horde outright; the shells are the explosion prefab
//! (RZ_Artillery, shared with the Bastion Champion perk in RZ_FactionServices).
class RZ_Mission_FireMission : RZ_Mission
{
	protected ref array<vector> m_aTargets = {};
	protected ref array<int> m_aTowns = {};
	protected ref array<bool> m_aSpawned = {};
	protected ref array<float> m_aMarked = {};
	protected ref array<float> m_aStrikeIn = {};
	protected ref array<bool> m_aHit = {};
	protected ref array<ref array<IEntity>> m_aUnits = {};
	protected ref array<IEntity> m_aProps = {};
	protected int m_iHits;

	protected static const int TARGETS = 3;
	protected static const float MARK_SECONDS = 10;
	protected static const float MARK_RADIUS = 80;
	protected static const float STRIKE_DELAY = 30;
	protected static const float STRIKE_RADIUS = 45;
	protected static const float SAFE_DISTANCE = 300;	// m from where people live

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.CHAMPION, 45, 40);
		m_Req.m_fMinMilitary = 0.2;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		// Never where people live: no target within SAFE_DISTANCE of a safehouse, holding or the lab
		array<int> towns = {};
		RZ_WorldFacts facts = gm.GetFacts();
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			if (s.m_fInfection >= 50 && !gm.GetMissions().IsSettlementBusy(i, this) && !facts.IsNearLivedIn(s.m_vCenter, s.m_fRadius * 0.5 + SAFE_DISTANCE))
				towns.Insert(i);
		}

		if (towns.Count() < TARGETS)
			return false;

		for (int n = 0; n < TARGETS; n++)
		{
			int pick = towns.GetRandomElement();
			towns.RemoveItem(pick);
			RZ_Settlement t = gm.GetSettlement(pick);
			m_aTowns.Insert(pick);
			m_aTargets.Insert(LandPointNear(t.m_vCenter, 0, t.m_fRadius * 0.5));
			m_aSpawned.Insert(false);
			m_aMarked.Insert(0);
			m_aStrikeIn.Insert(-1);
			m_aHit.Insert(false);
			m_aUnits.Insert(new array<IEntity>());
		}

		m_iSettlement = m_aTowns[0];
		m_vPos = m_aTargets[0];
		m_sTitle = "Fire Mission";
		m_sBrief = string.Format("Three hordes are milling in %1, %2 and %3. Get eyes on each, pop smoke on them and our guns do the rest. Don't stand in the smoke.",
			gm.GetSettlement(m_aTowns[0]).m_sName, gm.GetSettlement(m_aTowns[1]).m_sName, gm.GetSettlement(m_aTowns[2]).m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		foreach (int i, vector target : m_aTargets)
		{
			if (m_aHit[i])
				continue;

			if (!m_aSpawned[i])
			{
				if (!RZ_ContractHelp.AnyNear(players, target, 500))
					continue;

				m_aSpawned[i] = true;
				gm.GetSpawner().SpawnHordeAt(target, target, m_iId, m_aUnits[i]);
				continue;
			}

			if (m_aStrikeIn[i] >= 0)
			{
				m_aStrikeIn[i] = m_aStrikeIn[i] - dt;
				if (m_aStrikeIn[i] <= 0)
					Strike(gm, i);

				continue;
			}

			if (CountPlayersNear(players, target, MARK_RADIUS) == 0)
				continue;

			RZ_ContractHelp.AddNear(ids, players, target, MARK_RADIUS, m_aParticipants);
			m_aMarked[i] = m_aMarked[i] + dt;
			if (m_aMarked[i] < MARK_SECONDS)
				continue;

			m_aStrikeIn[i] = STRIKE_DELAY;
			RZ_ContractSettings cs = gm.GetContractSettings();
			if (cs)
				m_aProps.Insert(RZ_Util.SpawnPrefab(cs.m_sSmokePrefab, target));

			RZ_ContractHelp.NotifyNear(gm, target, 300, m_sTitle, "Red smoke drifts up over the horde. The radio crackles: \"Smoke seen. Thirty seconds. Get clear.\"");
		}

		if (m_iHits >= TARGETS && m_eState == RZ_EMissionState.ACTIVE)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	protected void Strike(RZ_GameModeComponent gm, int idx)
	{
		vector at = m_aTargets[idx];
		RZ_Artillery.Shell(gm, at, STRIKE_RADIUS, 4, m_aProps);

		foreach (IEntity unit : m_aUnits[idx])
		{
			array<IEntity> members = {};
			RZ_Util.GetUnitMembers(unit, members);
			foreach (IEntity m : members)
			{
				if (RZ_Util.DistXZ(m.GetOrigin(), at) <= STRIKE_RADIUS * 2)
					RZ_Util.KillCharacter(m);
			}
		}

		m_aHit[idx] = true;
		m_iHits++;
		RZ_ContractHelp.NotifyNear(gm, at, 600, m_sTitle, string.Format("The shells land in a tight group. Then nothing moves. (%1 of %2)", m_iHits, TARGETS));
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = "Hordes:";
		foreach (int i, vector target : m_aTargets)
		{
			if (m_aHit[i])
				continue;

			RZ_Settlement s = gm.GetSettlement(m_aTowns[i]);
			if (s)
				text += string.Format("\n%1 (%2)", s.m_sName, RZ_Util.DirectionText(from, target));
		}

		return text;
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
