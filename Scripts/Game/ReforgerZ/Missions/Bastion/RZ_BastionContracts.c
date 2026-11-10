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
//! #8 Restore Comms: a relay mast on high ground is dead. Bring the spare parts from a nearby
//! town and fit them, then stand by it for a minute while it powers up (it hums, loudly).
//! ★ Radio Op: rewires it by hand, no parts needed. Bastion shares horde intel on the radio.
class RZ_Mission_RestoreComms : RZ_PuzzleMission
{
	protected vector m_vMast;
	protected vector m_vParts;
	protected bool m_bMastSpawned;
	protected bool m_bPartsSpawned;
	protected IEntity m_Parts;
	protected bool m_bFitted;
	protected float m_fPowered;
	protected string m_sPartsTown;

	protected static const float POWER_SECONDS = 60;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 22, 20);
		m_Req.Bonus("radioop");
		m_fTimeLeft = 3000;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetMissionItemPrefab().IsEmpty() || !HasPrefab(gm, RZ_EPuzzleProp.OBJECT))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 80);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		RZ_Settlement p = gm.GetSettlement(PickSettlementAway(gm, s.m_vCenter, 500, 2500, 0, 90));
		if (!p)
			return false;

		m_vMast = HighPointNear(s.m_vCenter, s.m_fRadius, s.m_fRadius + 400);
		m_vParts = LandPointNear(p.m_vCenter, 0, p.m_fRadius * 0.6);
		m_sPartsTown = p.m_sName;
		m_vPos = m_vMast;
		m_sTitle = "Restore Comms";
		m_sBrief = string.Format("Our relay on the hill above %1 is dead. The spares are in a crate in %2. Fit them and keep the mast company while it powers up.", s.m_sName, p.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bMastSpawned && RZ_ContractHelp.AnyNear(players, m_vMast, 300))
		{
			m_bMastSpawned = true;
			SpawnObject(gm, RZ_EPuzzleProp.RADIO_MAST, m_vMast, 0, "Fit the spare parts|Rewire it by hand (Radio Operator)", "");
		}

		if (!m_bPartsSpawned && RZ_ContractHelp.AnyNear(players, m_vParts, 200))
		{
			m_bPartsSpawned = true;
			m_Parts = RZ_Util.SpawnPrefab(gm.GetMissionItemPrefab(), m_vParts + Vector(0, 0.3, 0));
			RZ_MissionItemComponent.Tag(m_Parts, m_iId);
			TrackProp(m_Parts);
			SpawnGuards(gm, m_vParts, 3, 8, 30);
		}

		if (!m_bFitted)
			return;

		if (!ReachedBy(ids, players, m_vMast, 25))
			return;

		gm.GetHeat().AddHeat(m_vMast, 0.5 * dt);
		m_fPowered += dt;
		if (m_fPowered >= POWER_SECONDS)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx != 0 || m_bFitted)
			return;

		if (value == 1)
		{
			if (!HasBonus(gm, playerId))
			{
				gm.NotifyPlayer(playerId, m_sTitle, "You stare at the wiring. It stares back. You'll need the spare parts.");
				return;
			}

			Fitted(gm, "You strip two cables, bridge the dead board and coax the transmitter awake without a single spare.");
			return;
		}

		if (!RZ_ContractHelp.CarriesItemOf(user, m_iId, true))
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("You've nothing to fit. The spares are in %1.", m_sPartsTown));
			return;
		}

		Fitted(gm, "The new boards slide in. Lights blink red, then amber. Stay by it while it warms up.");
	}

	//------------------------------------------------------------------------------------------------
	protected void Fitted(RZ_GameModeComponent gm, string text)
	{
		m_bFitted = true;
		SetObjectLabels(0, "", "");
		RZ_ContractHelp.NotifyNear(gm, m_vMast, 60, m_sTitle, text + " It hums loud enough to carry.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_BastionContractHelp.ShareHordeIntel(gm, m_aParticipants);
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
		m_iSettlement = PickSettlement(gm, 15, 65);
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
//! #10 Weapons Cache: an armoury in an overrun town (a barracks or police station if the map has
//! one). Search it for 30 s, then carry the case to Bastion. Bastion gets stronger; you get paid,
//! not armed.
class RZ_Mission_WeaponsCache : RZ_Mission_Delivery
{
	protected float m_fSearched;
	protected bool m_bGuarded;

	protected static const float SEARCH_SECONDS = 30;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.TRUSTED, 30, 30);
		m_fTimeLeft = 3000;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!CanRun(gm, RZ_EFaction.MILITARY))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 60, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		array<string> kinds = {"military", "police"};
		RZ_Landmark armoury = RZ_LandmarkFinder.FindNearest(s.m_vCenter, s.m_fRadius, kinds);
		string place = "the armoury";
		if (armoury)
		{
			m_vPos = LandPointNear(armoury.m_vPos, 4, 10);
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

		m_sTitle = "Weapons Cache";
		m_sBrief = string.Format("Our people left weapons locked up in %1 in %2 when it fell. Search it, then bring the case to %3. Don't get ideas about keeping it.", place, s.m_sName, meetName);
		return true;
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

		if (m_bItemSpawned || CountPlayersNear(players, m_vPos, 10) == 0)
			return;

		m_fSearched += dt;
		if (m_fSearched >= SEARCH_SECONDS && SpawnItem(gm, m_vPos))
			RZ_ContractHelp.NotifyNear(gm, m_vPos, 40, m_sTitle, "Behind a rack of empty rifle hooks: a long case with Bastion stencils. Heavy.");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelivered(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction)
	{
		if (gm.GetTerritory() && gm.GetTerritory().IsActive())
			gm.GetTerritory().AddStrength(RZ_EFaction.MILITARY, 1);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return super.GetWhereText(gm, from) + "\nDrop: " + RZ_Util.DirectionText(from, GetDropPos(0));
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
		m_sBrief = string.Format("A supply truck leaves %1 for our post in %2 in two minutes. Ride with it and get it there. It will break down; they always do.", m_Watch.m_sFrom, m_Watch.m_sTo);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_Watch.Update(gm);
		m_vPos = m_Watch.GetPos(gm);
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
	//! Paid: players who were with the truck for at least 40% of the checks (two at least).
	protected void PayEscorts()
	{
		m_aParticipants.Clear();
		int needed = Math.Max(2, RZ_Util.RoundToInt(m_iChecks * 0.4));
		foreach (int playerId, int seen : m_mNear)
		{
			if (seen >= needed)
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
class RZ_Mission_RescueThePilot : RZ_PuzzleMission
{
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
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!RZ_Escort.HasRoom(gm) || !HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 20, 95);
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
		m_sBrief = string.Format("One of ours went down outside %1. The airframe's gone; the pilot may not be. Find him and walk him to %2.", s.m_sName, m_sDestName);
		return m_Escort.Begin(gm, LandPointNear(m_vWreck, 3, 8));
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
		return "Rescue the Pilot: the pilot didn't make it.";
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Escort.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! #14 Clear the Airfield: a flat field outside a town where the helicopter could set down, full
//! of infected (spawned when someone comes within 400 m). Kill them all: the Last Helicopter runs
//! twice as often for the next two hours.
class RZ_Mission_ClearTheAirfield : RZ_Mission_KillTagged
{
	protected static const float BOOST_SECONDS = 7200;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.TRUSTED, 30, 25);
		m_Req.m_fMinMilitary = 0.2;
		m_fTimeLeft = 2700;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 10, 90);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius + 150, s.m_fRadius + 500);
		m_fParticipationRadius = 150;
		m_sTitle = "Clear the Airfield";
		m_sBrief = string.Format("There's a strip of flat ground outside %1 the helicopter could use, if it weren't full of them. Clear it and it can fly more often.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bSpawned && RZ_ContractHelp.AnyNear(players, m_vPos, 400))
		{
			m_bSpawned = true;
			SpawnGuards(gm, m_vPos, 8, 5, 60, false);
			SpawnGuards(gm, m_vPos, 1, 10, 40, true);
		}

		super.OnUpdate(gm, dt, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Mission_LastHelicopter.Boost(gm, BOOST_SECONDS);
		NotifyParticipants(gm, "Task Force Bastion", "Strip's clear. The bird can set down there now: expect it more often.");
	}
}

//------------------------------------------------------------------------------------------------
//! #16 Hunt the Deserter: a soldier ran with supplies. Follow his trail (RZ_StepChain: his kit,
//! then his fire, each with a note pointing on), find him, and **bring him in** (he follows you to
//! Bastion: full pay) or **end it there** (60%). Killed some other way: 60%.
class RZ_Mission_HuntTheDeserter : RZ_PuzzleMission
{
	protected ref RZ_StepChain m_Chain = new RZ_StepChain();
	protected ref RZ_Escort m_Escort = new RZ_Escort();
	protected vector m_vHide;
	protected vector m_vDest;
	protected string m_sDestName;
	protected bool m_bFound;
	protected bool m_bArrested;
	protected bool m_bEnded;
	protected float m_fBeside;
	protected int m_iTalk = -1;

	protected static const float ENDED_SCALE = 0.6;

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		m_Chain.GetSpots(result);
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.TRUSTED, 35, 30);
		m_fTimeLeft = 4200;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!RZ_Escort.HasRoom(gm) || !HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 90);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		array<int> towns = {};
		if (!RZ_StepChain.PickTowns(gm, s.m_vCenter, 1, 700, 2500, 0, 95, towns))
			return false;

		RZ_Settlement s2 = gm.GetSettlement(towns[0]);
		m_vHide = LandPointNear(s2.m_vCenter, s2.m_fRadius + 150, s2.m_fRadius + 450);
		if (!RZ_LeaderJob.MeetingPoint(gm, RZ_EFaction.MILITARY, m_vHide, m_vDest, m_sDestName))
			return false;

		m_Chain.SetMission(m_iId);
		RZ_ChainStep kit = m_Chain.AddStep(gm, m_iSettlement, LandPointNear(s.m_vCenter, s.m_fRadius * 0.6, s.m_fRadius + 60), 6, 0);
		kit.m_iClue = AddClue("A dropped pack", string.Format("Army issue, the name tape cut off. Inside: two empty ration tins and a map with %1 circled in pencil.", s2.m_sName));
		kit.m_sReadLabel = "Go through the pack";
		RZ_ChainStep fire = m_Chain.AddStep(gm, towns[0], LandPointNear(s2.m_vCenter, s2.m_fRadius * 0.4, s2.m_fRadius), 6, 0);
		fire.m_iClue = AddClue("A cold fire", string.Format("Someone slept here last night. Boot prints head out of town toward grid %1, where the trees start.", RZ_SettlementScanner.GridReference(m_vHide)));
		fire.m_sReadLabel = "Look around the fire";
		fire.m_iGuards = 3;

		m_iTalk = AddClue("The deserter", "He's young, and he doesn't raise the rifle. \"I'm not going back to that post. You don't know what they're doing out there.\"");
		m_Escort.m_sName = "the deserter";
		m_Escort.m_sPrefab = RZ_Escort.PrefabFor(gm, 2);
		m_Escort.m_bHide = false;
		m_Escort.SetTalk(m_iId, 0, "Arrest him (he walks with you)|End it here", m_iTalk, "Talk to the deserter");

		m_vPos = kit.m_vPos;
		m_sTitle = "Hunt the Deserter";
		m_sBrief = string.Format("A soldier walked out of his post with a week of rations. His trail starts at %1. Bring him back to %2 if you can. If you can't, end it.", s.m_sName, m_sDestName);
		return m_Escort.Begin(gm, m_vHide);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		m_Chain.OnClueRead(idx);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_Chain.IsDone())
		{
			int done = m_Chain.Update(gm, dt, ids, players, m_aParticipants);
			RZ_ChainStep next = m_Chain.GetCurrent();
			if (next)
				m_vPos = next.m_vPos;
			else
				m_vPos = m_vHide;

			if (done >= 0)
				NotifyParticipants(gm, m_sTitle, "The trail goes on.");

			return;
		}

		// His hideout: the body appears near players; talk to him
		m_Escort.Update(gm, dt, ids, players);
		m_vPos = m_Escort.m_vPos;
		if (!m_bFound && ReachedBy(ids, players, m_vPos, 30))
			m_bFound = true;

		// Without talk components on his body: stand over him for 10 s and he gives up
		if (m_bFound && !m_bArrested && !m_bEnded)
		{
			int beside = RZ_EscortHelp.PlayerBeside(ids, players, m_vPos, 4);
			if (beside > 0)
				m_fBeside += dt;

			if (beside > 0 && m_fBeside >= 10)
				OnChoice(gm, beside, 0, 0, null);
		}

		if (m_Escort.IsDead() && !m_bEnded)
		{
			m_bEnded = true;
			m_fRewardScale = ENDED_SCALE;
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (m_bArrested)
		{
			RZ_ContractHelp.AddNear(ids, players, m_vPos, 100, m_aParticipants);
			if (m_Escort.IsNear(m_vDest, 25))
			{
				m_Escort.Arrive();
				m_eState = RZ_EMissionState.SUCCEEDED;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0 || m_bArrested || m_bEnded)
			return;

		if (option == 0)
		{
			m_bArrested = true;
			m_Escort.Follow(playerId);
			m_Escort.Go();
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("He looks at you for a long moment, then hands you the rifle, stock first. \"Walk in front of me, then.\" Take him to %1.", m_sDestName));
			return;
		}

		if (option != 1)
			return;

		m_bEnded = true;
		m_fRewardScale = ENDED_SCALE;
		m_aParticipants.Clear();
		m_aParticipants.Insert(playerId);
		gm.NotifyPlayer(playerId, m_sTitle, "You do what they asked. Afterwards it's very quiet. Bastion pays for this too, a little less.");
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Chain.Cleanup();
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
		m_iSettlement = PickSettlementAway(gm, m_vLab, 1000, 4000, 0, 90);
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
//! *Simplified:* no real bridge: a choke point on the line between the horde and the town. The
//! horde is data (walking 1 m/s) until a player is within 600 m.
class RZ_Mission_BlowTheBridge : RZ_PuzzleMission
{
	protected vector m_vTown;
	protected vector m_vChoke;
	protected vector m_vHorde;
	protected vector m_vDetonator;
	protected ref array<vector> m_aCharges = {};
	protected ref array<float> m_aPlanted = {};
	protected ref array<IEntity> m_aHorde = {};
	protected int m_iPlanted;
	protected int m_iNeeded = 3;
	protected bool m_bBonusChecked;
	protected bool m_bDetonatorShown;
	protected bool m_bBlown;
	protected bool m_bTurned;
	protected bool m_bHordeSpawned;
	protected float m_fStartIn = 300;

	protected static const float HORDE_SPEED = 1;
	protected static const float CHARGE_SECONDS = 10;

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
		m_iSettlement = PickSettlement(gm, 0, 35);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		RZ_Settlement src = gm.GetSettlement(PickSettlementAway(gm, s.m_vCenter, 1500, 3000, 50, 100));
		if (!src)
			return false;

		m_vTown = s.m_vCenter;
		m_vHorde = LandPointNear(src.m_vCenter, 0, src.m_fRadius * 0.5);
		m_vChoke = RZ_ContractHelp.Between(m_vHorde, m_vTown, 0.6);
		for (int i = 0; i < 3; i++)
		{
			m_aCharges.Insert(LandPointNear(m_vChoke, 6, 20));
			m_aPlanted.Insert(0);
		}

		m_vDetonator = LandPointNear(m_vChoke, 45, 60);
		m_vPos = m_vChoke;
		m_sTitle = "Blow the Bridge";
		m_sBrief = string.Format("A horde out of %1 is walking toward %2. There's a narrow point on its way at grid %3. Plant the charges there and blow it before they pass. You have about five minutes before they move.",
			src.m_sName, s.m_sName, RZ_SettlementScanner.GridReference(m_vChoke));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		UpdateHorde(gm, dt, players);
		if (m_eState != RZ_EMissionState.ACTIVE)
			return;

		if (!m_bDetonatorShown && RZ_ContractHelp.AnyNear(players, m_vChoke, 300))
		{
			m_bDetonatorShown = true;
			SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vDetonator, 0, "", "Blow the charges");
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

			if (!ReachedBy(ids, players, c, 3))
				continue;

			m_aPlanted[i] = m_aPlanted[i] + dt;
			if (m_aPlanted[i] < CHARGE_SECONDS)
				continue;

			m_iPlanted++;
			RZ_ContractHelp.NotifyNear(gm, m_vChoke, 60, m_sTitle, string.Format("Charge %1 of %2 set. Get clear before you blow them.", m_iPlanted, m_iNeeded));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Data until someone is near, then real groups walking to the town.
	protected void UpdateHorde(RZ_GameModeComponent gm, float dt, array<IEntity> players)
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
				m_vHorde = RZ_Util.GetUnitPosition(units[0]);
			else
				m_bHordeSpawned = false;	// despawned or killed: walk on as data
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
				gm.GetSpawner().SpawnHordeAt(m_vHorde, m_vTown, m_iId);
			}
		}

		if (RZ_Util.DistXZ(m_vHorde, m_vTown) > 120)
			return;

		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
			s.AddInfection(10);

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
		return string.Format("Blow the Bridge: the horde reached %1.", GetLocationName(gm));
	}
}

//------------------------------------------------------------------------------------------------
//! #20 Fire Mission: three hordes mill in infested towns. Stand within 80 m of one for 10 s to
//! mark it with smoke; 30 s later Bastion's guns hit it (an explosion per shell and every infected
//! there dies). Get clear: the shells don't check who's standing there. All three: paid.
//! *Simplified:* the strike kills the marked horde outright; the shells are the explosion prefab.
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
		array<int> towns = {};
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			if (s.m_fInfection >= 50 && !gm.GetMissions().IsSettlementBusy(i))
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
		ResourceName fx = gm.GetExplosionPrefab();
		for (int n = 0; n < 4; n++)
		{
			IEntity boom = RZ_Util.SpawnPrefab(fx, RZ_Util.RandomPointInRing(at, 0, STRIKE_RADIUS * 0.6));
			if (boom)
				m_aProps.Insert(boom);
		}

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

		gm.GetHeat().AddHeat(at, 30);
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
