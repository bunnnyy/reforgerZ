//! Missions tied to the Haven's safehouses and wanderers (docs/MISSIONS.md,
//! "Haven house contracts"): three grey contracts about the safehouses' rules and
//! their dark side, and the Courier's unarmed delivery job.

//------------------------------------------------------------------------------------------------
//! The Courier wanderer's job: carry letters to a safehouse (or a quiet town) 0.8-4 km
//! away. No fighting needed. Never picked at random: RZ_Wanderers starts it when a player
//! asks a Courier for work. Needs the mission item and the Haven drop point.
class RZ_Mission_CourierJob : RZ_Mission_Delivery
{
	protected vector m_vStart;
	protected string m_sCourier;
	protected bool m_bPrepared;
	protected int m_iFor = -1;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 6, 12);
		m_fTimeLeft = 2400;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Where the courier stands and who they are. False if the prefabs aren't set.
	bool Prepare(RZ_GameModeComponent gm, vector at, string courier, int playerId)
	{
		if (!CanRun(gm, RZ_EFaction.SURVIVORS))
			return false;

		m_vStart = at;
		m_sCourier = courier;
		m_iFor = playerId;
		m_bPrepared = true;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The job belongs to whoever asked the courier (and whoever walks in with them).
	override bool IsEligible(RZ_GameModeComponent gm, int playerId)
	{
		// Someone else is paid only alongside the one who asked: taking the job off them (or
		// finishing it after they left) pays nobody
		if (m_iFor > 0 && playerId != m_iFor && (!m_aParticipants.Contains(playerId) || !m_aParticipants.Contains(m_iFor)))
			return false;

		return super.IsEligible(gm, playerId);
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		if (!m_bPrepared)
			return false;

		vector dest;
		string destName = "";
		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses)
		{
			array<int> options = {};
			for (int i = 0; i < houses.Count(); i++)
			{
				float d = RZ_Util.DistXZ(houses.Get(i).m_vPos, m_vStart);
				if (d >= 800 && d <= 4000)
					options.Insert(i);
			}

			if (!options.IsEmpty())
			{
				RZ_Safehouse h = houses.Get(options.GetRandomElement());
				dest = LandPointNear(h.m_vPos, 2, 6);
				destName = "the safehouse in " + h.m_sName;
			}
		}

		if (destName.IsEmpty())
		{
			int town = PickSettlementAway(gm, m_vStart, 800, 3500, 0, 60);
			RZ_Settlement s = gm.GetSettlement(town);
			if (!s)
				return false;

			dest = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.4);
			destName = s.m_sName;
		}

		if (!SpawnItem(gm, LandPointNear(m_vStart, 1, 3)))
			return false;

		if (!SpawnDrop(gm, RZ_EFaction.SURVIVORS, dest))
			return false;

		m_vPos = dest;
		m_iSettlement = gm.FindNearestSettlement(dest);
		m_sTitle = "Letters for " + destName;
		m_sBrief = string.Format("%1 puts a bundle of letters tied with string at your feet. \"For %2. You won't need a gun. Stay off the main road, keep quiet, don't stop for anyone.\"",
			m_sCourier, destName);
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! Haven lookouts: people of a safehouse sitting on walls by a road, or standing about a yard (A
//! Quiet Arrangement's walk; the Haven questlines' "past two lookouts"). Props, not AI: they see
//! whoever comes within a radius of them, and so does a Haven wanderer who happens to be near.
class RZ_HavenLookouts
{
	string m_sWho = "A Haven lookout on a wall by the road";
	protected ref array<IEntity> m_aBodies = {};

	//------------------------------------------------------------------------------------------------
	//! `count` lookouts along the way from `from` to `to`, a little off the straight line, as people
	//! of safehouse `house` (-1: nobody's). Returns how many stand.
	int SpawnAlong(RZ_GameModeComponent gm, vector from, vector to, int house, int count)
	{
		vector line = to - from;
		vector side = Vector(-line[2], 0, line[0]);
		float sideLen = side.Length();
		if (sideLen > 1)
			side = side * (1 / sideLen);

		for (int i = 0; i < count; i++)
		{
			float share = Math.Min(0.85, 0.35 * (i + 1));
			vector at = from + line * share + side * Math.RandomFloat(-40, 40);
			Put(gm, RZ_Mission.LandPointNear(at, 0, 10), house);
		}

		return m_aBodies.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! `count` lookouts minR-maxR from center (a yard, a store room's door).
	int SpawnAround(RZ_GameModeComponent gm, vector center, float minR, float maxR, int house, int count)
	{
		for (int i = 0; i < count; i++)
		{
			Put(gm, RZ_Mission.LandPointNear(center, minR, maxR), house);
		}

		return m_aBodies.Count();
	}

	//------------------------------------------------------------------------------------------------
	protected void Put(RZ_GameModeComponent gm, vector pos, int house)
	{
		IEntity lookout = RZ_Util.SpawnPrefab(gm.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN), pos, Math.RandomFloat(0, 360));
		if (!lookout)
			return;

		if (house >= 0)
			RZ_Safehouses.TagOccupant(lookout, house);

		m_aBodies.Insert(lookout);
	}

	//------------------------------------------------------------------------------------------------
	//! Who sees someone standing at pos: a lookout within radius (m_sWho), a Haven wanderer's name
	//! (with `wanderers`), or "" if nobody does. With the player's id, the radius shrinks while
	//! they're hard to see (Stealth, RZ_Stealth.SightRadius).
	string SeenBy(RZ_GameModeComponent gm, vector pos, float radius, bool wanderers, int playerId = -1)
	{
		float reach = RZ_Stealth.SightRadius(playerId, radius);
		foreach (IEntity lookout : m_aBodies)
		{
			if (lookout && RZ_Util.DistXZ(lookout.GetOrigin(), pos) <= reach)
				return m_sWho;
		}

		if (!wanderers || !gm.GetWanderers())
			return "";

		return gm.GetWanderers().SeenByHaven(pos, reach);
	}

	//------------------------------------------------------------------------------------------------
	int Count()
	{
		return m_aBodies.Count();
	}

	//------------------------------------------------------------------------------------------------
	void Cleanup()
	{
		foreach (IEntity e : m_aBodies)
		{
			RZ_Util.DeleteUnit(e);
		}

		m_aBodies.Clear();
	}
}

//------------------------------------------------------------------------------------------------
//! Grey contract: Walk Him Out. A bitten Haven member told the council himself. At dusk
//! he walks out past the edge of town so nobody has to watch what comes after; you walk
//! with him. At the edge you choose: end it, or let him walk on. Both are allowed.
//! End it: full Fame. Walk on: less Fame, and the town's infection rises a little.
//! If nobody comes before his time runs out, he turns there. ★ Nurse: can read the bite.
//! ★ Elder: more Fame. Done once, the Edge of Town questline can start (world flag
//! "walk.contract"); its stages reuse this scene (Missions/Questlines/Haven/RZ_Quest_EdgeOfTown.c).
class RZ_Mission_WalkHimOut : RZ_PuzzleMission
{
	protected int m_iHouse = -1;
	protected string m_sMan;
	protected string m_sTown;
	protected int m_iTalk = -1;
	protected bool m_bGuarded;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 15, 15);
		m_Req.Bonus("nurse");
		m_Req.Bonus("elder");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 2400;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Only offered around dusk (16:30 to 21:00 game time), or any time if the clock is unknown.
	override float GetWeight(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN) || !houses || houses.Count() == 0)
			return 0;

		float hour = RZ_GameTime.GetHour();
		if (hour >= 0 && (hour < 16.5 || hour > 21))
			return 0;

		return 0.7;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		m_iHouse = PickHouse(gm);
		RZ_Safehouse h = houses.Get(m_iHouse);
		if (!h || h.m_iPopulation < 3)
			return false;

		m_iSettlement = h.m_iSettlement;
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s || IsTownBusy(gm, m_iSettlement))
			return false;

		float dir = m_Rng.Int(0, 8) * 45;
		vector edge = LandPointNear(RZ_Compass.PointAt(s.m_vCenter, dir, s.m_fRadius + 60), 0, 20);
		m_vPos = edge;
		m_sTown = s.m_sName;
		m_sMan = PickName(gm);

		m_iTalk = AddClue(m_sMan, string.Format("\"I told them myself. I didn't want anyone finding out the other way.\" He rolls his sleeve up to show you, like it's a splinter. \"I'd like to keep walking from here. Or I'd like you to make sure I don't. I can't decide. You can.\" He looks back at %1 once, and then doesn't again.",
			m_sTown));
		SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, edge, 0, "", ChoiceLabels(gm), m_iTalk, "Talk to " + m_sMan);

		// He's left the safehouse for good, whatever happens next
		houses.RemovePopulation(m_iHouse, 1);

		m_sTitle = "Walk Him Out";
		m_sBrief = string.Format("%1 was bitten on a supply run. He told the council himself. At dusk he walks out past the %2 edge of %3, so the children don't have to see what comes after. Walk with him. At the edge he'll ask you for one last thing.",
			m_sMan, RZ_Compass.DirectionWord(dir), m_sTown);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Which safehouse he comes from (a questline stage picks its own house).
	protected int PickHouse(RZ_GameModeComponent gm)
	{
		return gm.GetSafehouses().PickRandom();
	}

	//------------------------------------------------------------------------------------------------
	protected string PickName(RZ_GameModeComponent gm)
	{
		array<string> names = {"Pavel", "Henri", "Tomas", "Emil", "Anton", "Viktor", "Karel", "Oskar", "Milan", "Jakub", "Bruno", "Leon"};
		return m_Rng.Pick(names);
	}

	//------------------------------------------------------------------------------------------------
	//! His choices (a questline stage can add one).
	protected string ChoiceLabels(RZ_GameModeComponent gm)
	{
		return "End it here|Let him walk on";
	}

	//------------------------------------------------------------------------------------------------
	//! Someone walked him out: the Edge of Town questline may start now (docs/QUESTLINES.md, H3).
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (gm.GetQuestlines())
			gm.GetQuestlines().SetWorldFlag("walk.contract", 1);
	}

	//------------------------------------------------------------------------------------------------
	override string GetClueText(RZ_GameModeComponent gm, int playerId, int idx)
	{
		string text = super.GetClueText(gm, playerId, idx);
		if (idx == m_iTalk && HasBonus(gm, playerId))
			text += string.Format("\n\nYou look at the bite properly. The red lines are past the elbow. He has maybe %1 minutes. You don't tell him. He knows.", RZ_Util.RoundToInt(m_fTimeLeft / 60));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		// The infected on the edge of town turn up when someone comes near
		if (!m_bGuarded && RZ_ContractHelp.AnyNear(players, m_vPos, 300))
		{
			m_bGuarded = true;
			SpawnGuards(gm, m_vPos, 2, 60, 120);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Only someone the council sent may settle it: a passer-by gets nothing from him.
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0)
			return;

		if (!IsEligible(gm, playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 looks at you and shakes his head. \"The council didn't send you. I'll wait for the one they did.\"", m_sMan));
			return;
		}

		m_aParticipants.Clear();
		m_aParticipants.Insert(playerId);
		if (option == 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("You stay with him until it's done. Then you walk back to %1 alone. Someone has left a light on for you.", m_sTown));
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (option == 1)
		{
			m_fRewardScale = 0.6;
			RZ_Settlement s = gm.GetSettlement(m_iSettlement);
			if (s)
				s.AddInfection(5);

			gm.NotifyPlayer(playerId, m_sTitle, "He shakes your hand, and walks on toward the trees. You don't watch him go. Somewhere out there, later tonight, he'll stop being him. The council won't ask which you chose.");
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Nobody came. He turned at the edge of town.
	override void OnFailed(RZ_GameModeComponent gm)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		if (RZ_Util.NearestDistance(m_vPos, players) <= 600)
			gm.GetSpawner().SpawnZombie(m_vPos, true, m_iSettlement, -1);
		else
		{
			RZ_Settlement s = gm.GetSettlement(m_iSettlement);
			if (s)
				s.AddInfection(5);
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return string.Format("Nobody walked %1 out of %2. He waited at the edge of town as long as he could.", m_sMan, m_sTown);
	}
}

//------------------------------------------------------------------------------------------------
//! Grey contract: The Ledger. A safehouse council asks you to "remind" a member who has
//! been taking extra from the stores. Reach them, hear them out, then: threaten them
//! (full Fame; they remember it), pay their debt with your own Favor (full Fame; they
//! remember that too), or warn them off (no pay, a little Haven Infamy, and a friend on
//! the roads). Uses a real Haven wanderer when one is free, else a generated name.
//! ★ Elder: more Fame.
class RZ_Mission_TheLedger : RZ_PuzzleMission
{
	protected int m_iHouse = -1;
	protected int m_iWanderer = -1;
	protected string m_sMember;
	protected string m_sCouncil;
	protected int m_iTalk = -1;

	protected static const int DEBT_FAVOR = 20;
	protected static const int WARN_INFAMY = 3;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 15, 10);
		m_Req.Bonus("elder");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 2700;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN) || !houses || houses.Count() == 0)
			return 0;

		return 0.7;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		m_iHouse = houses.PickRandom();
		RZ_Safehouse h = houses.Get(m_iHouse);
		if (!h)
			return false;

		m_sCouncil = h.m_sName;
		m_iSettlement = PickSettlementAway(gm, h.m_vPos, 500, 2500, 0, 70);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.3, s.m_fRadius * 0.8);

		RZ_Wanderers wanderers = gm.GetWanderers();
		if (wanderers)
		{
			int mask = RZ_Wanderers.RoleBit(RZ_EWandererRole.TRADER) | RZ_Wanderers.RoleBit(RZ_EWandererRole.SCAVENGER) | RZ_Wanderers.RoleBit(RZ_EWandererRole.COURIER);
			m_iWanderer = wanderers.FindForMission(mask);
		}

		if (m_iWanderer >= 0)
		{
			wanderers.Hold(m_iWanderer, m_iId);
			m_sMember = wanderers.GetName(m_iWanderer);
		}
		else
		{
			m_sMember = RZ_NoteText.FullName(m_Rng);
		}

		string child = RZ_NoteText.Surname(m_Rng);
		m_iTalk = AddClue(m_sMember, string.Format("\"Two extra tins a week. For the %1 girl, three doors down from the council room. Her mother died the first week, and she's not on anyone's list, so nobody counts her. So I counted her.\" A shrug. \"Tell them whatever you like. I'll still be counting her.\"",
			child));
		string labels = string.Format("Threaten %1|Pay what %1 owes (%2 Haven Favor)|Tell %1 to disappear", m_sMember, DEBT_FAVOR);
		SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPos, 0, "", labels, m_iTalk, "Talk to " + m_sMember);

		m_sTitle = "The Ledger";
		m_sBrief = string.Format("The council at the safehouse in %1 keeps a ledger of the stores, and %2's name is in it more often than it should be. They want %2 reminded. They didn't say how. %2 is in %3.",
			m_sCouncil, m_sMember, s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0)
			return;

		// Only someone the council sent may settle it, and before any Favor changes hands
		if (!IsEligible(gm, playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 looks you over. \"You're not from the council. This is between me and them.\"", m_sMember));
			return;
		}

		if (option == 1 && !gm.SpendFavor(playerId, RZ_EFaction.SURVIVORS, DEBT_FAVOR))
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("The debt is %1 Haven Favor. You have %2.", DEBT_FAVOR, gm.GetProfile(playerId).GetFavor(RZ_EFaction.SURVIVORS)));
			return;
		}

		m_aParticipants.Clear();
		m_aParticipants.Insert(playerId);
		RZ_Wanderers wanderers = gm.GetWanderers();

		if (option == 0)
		{
			Remember(wanderers, playerId, RZ_Wanderers.MEM_WRONGED);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("You say it close and quiet, the way it has to be said. %1 nods and doesn't look at you again. The council will have its tins back. The girl won't.", m_sMember));
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (option == 1)
		{
			Remember(wanderers, playerId, RZ_Wanderers.MEM_HELPED);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("You square the ledger out of your own Favor. %1 stares at you like you've done something foolish. Maybe you have. The council is satisfied, and the girl still eats.", m_sMember));
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (option == 2)
		{
			m_fRewardScale = 0;
			gm.AddInfamy(playerId, RZ_EFaction.SURVIVORS, WARN_INFAMY);
			Remember(wanderers, playerId, RZ_Wanderers.MEM_SAVED);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("\"Go. Tonight. Take the girl if she'll come.\" %1 is gone before you've finished the sentence. The council will hear you let them walk. (+%2 Haven Infamy)",
				m_sMember, WARN_INFAMY));
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Remember(RZ_Wanderers wanderers, int playerId, int flag)
	{
		if (wanderers && m_iWanderer >= 0)
			wanderers.RememberPlayer(m_iWanderer, playerId, flag);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		RZ_Wanderers wanderers = gm.GetWanderers();
		if (wanderers && m_iWanderer >= 0)
			wanderers.Release(m_iWanderer, m_vPos);

		m_iWanderer = -1;
	}
}

//------------------------------------------------------------------------------------------------
//! Grey contract (Red Hand): A Quiet Arrangement. A Haven safehouse pays the Red Hand to
//! be left alone, and its keeper can't be seen carrying the money. Take the envelope from
//! the safehouse to a Red Hand drop without a Haven lookout or wanderer seeing you with it.
//! Small Red Hand Fame; Haven Infamy if you're seen. If it never arrives, the Red Hand
//! comes to collect in person (the safehouse loses two people). ★ Ex-Con: harder to spot.
//! Needs the mission item and the Red Hand drop point. The Price of the Door's first stage is
//! this walk, with the same lookouts (Missions/Questlines/Haven/RZ_Quest_PriceOfTheDoor.c).
class RZ_Mission_QuietArrangement : RZ_Mission_Delivery
{
	protected int m_iHouse = -1;
	protected string m_sHouse;
	protected ref RZ_HavenLookouts m_Lookouts = new RZ_HavenLookouts();
	protected bool m_bSeen;

	protected static const float SEEN_RADIUS = 35;
	protected static const float SEEN_RADIUS_BONUS = 25;
	protected static const int SEEN_INFAMY = 12;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 8, 15);
		m_Req.Bonus("excon");
		m_fTimeLeft = 2700;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!CanRun(gm, RZ_EFaction.BANDITS) || !houses || houses.Count() == 0)
			return 0;

		return 0.7;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		m_iHouse = houses.PickRandom();
		RZ_Safehouse h = houses.Get(m_iHouse);
		if (!h || IsTownBusy(gm, h.m_iSettlement))
			return false;

		m_iSettlement = h.m_iSettlement;
		m_sHouse = h.m_sName;
		int camp = PickSettlementAway(gm, h.m_vPos, 800, 2500, 20, 90);
		RZ_Settlement s = gm.GetSettlement(camp);
		if (!s)
			return false;

		vector drop = LandPointNear(s.m_vCenter, s.m_fRadius, s.m_fRadius + 80);
		m_vPos = h.m_vPos;
		if (!SpawnItem(gm, LandPointNear(h.m_vPos, 2, 5)))
			return false;

		if (!SpawnDrop(gm, RZ_EFaction.BANDITS, drop))
		{
			RZ_Util.DeleteUnit(m_Item);
			return false;
		}

		// Two Haven lookouts sit along the way, a little off the straight line
		m_Lookouts.SpawnAlong(gm, h.m_vPos, drop, m_iHouse, 2);

		m_sTitle = "A Quiet Arrangement";
		m_sBrief = string.Format("The keeper at the Haven safehouse in %1 pays us so nobody burns it down. Sweet arrangement. Trouble is, he can't be seen walking it over, and you can. The envelope's under the loose board by the door. Bring it to our drop outside %2. If his own people see you with it, that's your problem, not ours.",
			m_sHouse, s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDeliveryTick(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bSeen || m_iCarrier <= 0)
			return;

		IEntity carrier = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iCarrier);
		if (!carrier)
			return;

		float radius = SEEN_RADIUS;
		if (HasBonus(gm, m_iCarrier))
			radius = SEEN_RADIUS_BONUS;

		// Leaving the safehouse itself with it is fine: the keeper handed it over in private
		if (RZ_Util.DistXZ(carrier.GetOrigin(), m_vPos) < 40)
			return;

		string who = m_Lookouts.SeenBy(gm, carrier.GetOrigin(), radius, true, m_iCarrier);
		if (who.IsEmpty())
			return;

		// The Haven holds it against the carrier now, delivered or not
		m_bSeen = true;
		gm.AddInfamy(m_iCarrier, RZ_EFaction.SURVIVORS, SEEN_INFAMY);
		gm.NotifyPlayer(m_iCarrier, m_sTitle, string.Format("%1 sees the envelope in your hand and knows exactly what it is. By tonight, so will the safehouse in %2. (+%3 Haven Infamy)", who, m_sHouse, SEEN_INFAMY));
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailed(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses)
			houses.RemovePopulation(m_iHouse, 2);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return string.Format("The envelope never reached the Red Hand. They went to the safehouse in %1 to collect in person. Two people there won't be coming back.", m_sHouse);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return super.GetWhereText(gm, from) + "\nRed Hand drop: grid " + RZ_SettlementScanner.GridReference(GetDropPos(0));
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Lookouts.Cleanup();
	}
}
