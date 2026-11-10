//! Unarmed missions, part two (see RZ_UnarmedCarry.c for the shared helper and the carrying
//! jobs): leading a horde, a rescue timed between waves, a trail of clues, a barter chain
//! across wanderers, a distraction, and counting specials from a distance.

//------------------------------------------------------------------------------------------------
//! #5 Lead Them Away (Haven): a horde is drifting toward a safehouse. Make noise to lead it
//! (each loud noise within 200 m of it turns it there), step by step, to a quarry pit. It
//! settles there: done. It reaches the safehouse: two people there don't get to the cellar.
//! Noticed near it, it turns on you instead. Killing it all also ends it, for half.
class RZ_Mission_LeadThemAway : RZ_Mission
{
	protected int m_iHouse = -1;
	protected vector m_vHouse;
	protected vector m_vQuarry;
	protected ref array<IEntity> m_aHorde = {};
	protected ref array<IEntity> m_aWaypoints = {};
	protected ref RZ_Stealth m_Stealth = new RZ_Stealth();
	protected float m_fCheck;
	protected float m_fSinceLure = 1000;
	protected bool m_bHeadingHome;
	protected vector m_vLure;

	protected static const float CHECK_INTERVAL = 5;
	protected static const float LURE_RANGE = 200;
	protected static const float ARRIVE_QUARRY = 70;
	protected static const float ARRIVE_HOUSE = 100;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 12, 15);
		m_Req.Bonus("radioham");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 1800;
		m_fParticipationRadius = 250;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses || houses.Count() == 0 || gm.GetSpawner().GetMoveWaypointPrefab().IsEmpty())
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		m_iHouse = houses.PickRandom();
		RZ_Safehouse h = houses.Get(m_iHouse);
		if (!h || gm.GetMissions().IsSettlementBusy(h.m_iSettlement))
			return false;

		m_vHouse = h.m_vPos;
		vector start = LandPointNear(m_vHouse, 750, 950);
		float from = RZ_Compass.Bearing(m_vHouse, start);
		m_vQuarry = LandPointNear(RZ_Compass.PointAt(m_vHouse, from + Math.RandomFloat(80, 140), Math.RandomFloat(550, 750)), 0, 40);
		if (RZ_Util.IsWater(start) || RZ_Util.IsWater(m_vQuarry))
			return false;

		gm.GetSpawner().SpawnHordeAt(start, m_vHouse, m_iId, m_aHorde);
		if (m_aHorde.IsEmpty())
			return false;

		m_iSettlement = h.m_iSettlement;
		m_vPos = start;
		m_bHeadingHome = true;
		m_Stealth.m_fHeatThreshold = 1000;	// they're following noise on purpose: only closeness counts
		m_Stealth.m_fZombieRadius = 12;
		m_Stealth.m_fZombieSeconds = 8;
		m_sTitle = "Lead Them Away";
		m_sBrief = string.Format("A horde is drifting toward the safehouse in %1. We can't fight it. Make noise, a bottle, a firecracker, anything, and lead it step by step to the old quarry pit (grid %2). Don't let it see you.",
			h.m_sName, RZ_SettlementScanner.GridReference(m_vQuarry));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		vector at;
		if (!RZ_UnarmedHelper.UnitsPosition(m_aHorde, at))
		{
			// All dead: it's over, just not the quiet way
			m_fRewardScale = 0.5;
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		m_vPos = at;
		m_Stealth.Track(gm, ids, players, dt, at, 150);
		m_fSinceLure += dt;
		m_fCheck += dt;
		if (m_fCheck < CHECK_INTERVAL)
			return;

		m_fCheck = 0;
		if (RZ_Util.DistXZ(at, m_vQuarry) <= ARRIVE_QUARRY)
		{
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (RZ_Util.DistXZ(at, m_vHouse) <= ARRIVE_HOUSE)
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		// Seen: it turns on whoever it saw
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), at) > 150 || !m_Stealth.IsNoticed(gm, p))
				continue;

			RZ_UnarmedHelper.SteerUnits(gm, m_aHorde, p.GetOrigin(), m_aWaypoints);
			m_Stealth.Forget(ids[i]);
			m_fSinceLure = 0;
			m_bHeadingHome = false;
			gm.NotifyPlayer(ids[i], m_sTitle, "It saw you. The whole crowd turns your way. Break off, then lead it again.");
			return;
		}

		vector noise;
		if (gm.GetHeat().FindRecentNoise(at, LURE_RANGE, RZ_UnarmedHelper.LURE_AGE, noise))
		{
			// The same noise heard again: they're already on their way
			if (m_bHeadingHome || RZ_Util.DistXZ(noise, m_vLure) > 20)
				RZ_UnarmedHelper.SteerUnits(gm, m_aHorde, noise, m_aWaypoints);

			m_vLure = noise;
			m_fSinceLure = 0;
			m_bHeadingHome = false;
			return;
		}

		// Nothing to follow for a while: it drifts back toward the safehouse
		if (!m_bHeadingHome && m_fSinceLure >= RZ_UnarmedHelper.LURE_HOLD)
		{
			RZ_UnarmedHelper.SteerUnits(gm, m_aHorde, m_vHouse, m_aWaypoints);
			m_bHeadingHome = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailed(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses && RZ_Util.DistXZ(m_vPos, m_vHouse) <= ARRIVE_HOUSE * 1.5)
			houses.RemovePopulation(m_iHouse, 2);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (RZ_Util.DistXZ(m_vPos, m_vHouse) <= ARRIVE_HOUSE * 1.5)
			return string.Format("%1: the horde reached the safehouse. Two of them didn't get to the cellar in time.", m_sTitle);

		return string.Format("%1: nobody led it anywhere. It wandered off on its own, for now.", m_sTitle);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return string.Format("%1: the horde is milling about at the bottom of the quarry pit, a long way from anyone.", m_sTitle);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return super.GetWhereText(gm, from) + "\nQuarry: grid " + RZ_SettlementScanner.GridReference(m_vQuarry);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		RZ_UnarmedHelper.DeleteAll(m_aWaypoints);
		m_aHorde.Clear();
	}
}

//------------------------------------------------------------------------------------------------
//! #6 Rooftop Rescue (Haven): a survivor is stuck up a building, the ladder at the back. Every
//! 90 s a few infected wander in. Talk them down when none are within 25 m of the ladder and
//! you haven't been noticed; too early and it's "not now" (and noise). Saved, they join the
//! nearest safehouse. ★ Mechanic: knows a shorter way down (15 m is clear enough).
class RZ_Mission_RooftopRescue : RZ_PuzzleMission
{
	protected vector m_vLadder;
	protected float m_fWaveTimer;
	protected int m_iWaves;

	protected static const float WAVE_INTERVAL = 90;
	protected static const int MAX_WAVES = 8;
	protected static const float CLEAR_RADIUS = 25;
	protected static const float CLEAR_RADIUS_BONUS = 15;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 10, 12);
		m_Req.Bonus("mechanic");
		m_fTimeLeft = 1800;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 20, 80);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		string what;
		array<string> kinds = {"school", "station", "bank", "church"};
		m_vLadder = RZ_UnarmedHelper.BuildingIn(s, kinds, what);
		if (what == "the house with the red door")
			what = "the tall house by the square";

		RZ_Rng rng = m_Rng;
		string name = RZ_NoteText.FirstName(rng);
		int talk = AddClue(name, string.Format("%1 calls down from the roof, very quietly: \"The ladder's round the back. Tell me when they've gone. Not before.\"", name));
		if (!SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vLadder, 0, "", "Talk them down the ladder", talk, "Call up to " + name))
			return false;

		m_vPos = m_vLadder;
		m_fParticipationRadius = 120;
		m_fWaveTimer = 20;
		GetStealth().m_fHeatThreshold = 50;
		GetStealth().m_fZombieRadius = 10;
		GetStealth().m_fZombieSeconds = 6;
		m_sTitle = "Rooftop Rescue";
		m_sBrief = string.Format("Someone is stuck on the roof of %1 in %2. They come in waves around there. Find the ladder at the back, wait for a gap, and talk them down.", what, s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		GetStealth().Track(gm, ids, players, dt, m_vLadder, 120);
		if (m_iWaves >= MAX_WAVES || CountPlayersNear(players, m_vLadder, 400) == 0)
			return;

		m_fWaveTimer -= dt;
		if (m_fWaveTimer > 0)
			return;

		m_fWaveTimer = WAVE_INTERVAL;
		m_iWaves++;
		vector from = LandPointNear(m_vLadder, 70, 100);
		for (int i = 0; i < 3; i++)
		{
			RZ_TrackedZombie z = gm.GetSpawner().SpawnZombie(RZ_Util.RandomPointInRing(from, 0, 10), false, m_iSettlement, m_iId);
			if (z)
				RZ_Util.SendGroupTo(z.m_Entity, LandPointNear(m_vLadder, 5, 30), gm.GetSpawner().GetMoveWaypointPrefab());
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		float clear = CLEAR_RADIUS;
		if (HasBonus(gm, playerId))
			clear = CLEAR_RADIUS_BONUS;

		if (gm.GetSpawner().IsZombieNear(m_vLadder, clear) || (user && GetStealth().IsNoticed(gm, user)))
		{
			gm.GetHeat().AddHeat(m_vLadder, 5);
			gm.NotifyPlayer(playerId, m_sTitle, "\"Not now! They're right below!\" A hiss from the roof, and every head in the street turns. Wait for the gap.");
			return;
		}

		gm.NotifyPlayer(playerId, m_sTitle, "Rung by rung, slow. Then they're down, and you're both walking, not running, away.");
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses && houses.Count() > 0)
			houses.AddRescued(m_vLadder, 1, m_aParticipants);
	}
}

//------------------------------------------------------------------------------------------------
//! #7 Trail of Clues (Haven, from a wanderer's worry): a missing person's note in one town leads
//! to their bag in a second, which leads to where they ended up in a third. Each clue only
//! appears once the one before it has been read. Half the time they're alive (bring them home:
//! they join a safehouse); otherwise their last letter (less Fame). ★ Police: grid references.
class RZ_Mission_TrailOfClues : RZ_PuzzleMission
{
	protected ref array<int> m_aTowns = {};
	protected ref array<vector> m_aSpots = {};
	protected string m_sName;
	protected bool m_bAlive;
	protected int m_iStep;
	protected int m_iFinal = -1;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 8, 10);
		m_Req.Bonus("police");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		int a = PickSettlement(gm, 0, 70);
		RZ_Settlement first = gm.GetSettlement(a);
		if (!first)
			return false;

		array<int> used = {a};
		array<int> options = {};
		CollectSettlements(gm, first.m_vCenter, 600, 1500, used, options);
		if (options.IsEmpty())
			return false;

		int b = options[0];
		used.Insert(b);
		RZ_Settlement second = gm.GetSettlement(b);
		array<int> more = {};
		CollectSettlements(gm, second.m_vCenter, 600, 1500, used, more);
		if (more.IsEmpty())
			return false;

		int c = more[0];
		m_aTowns.Insert(a);
		m_aTowns.Insert(b);
		m_aTowns.Insert(c);
		foreach (int idx : m_aTowns)
		{
			RZ_Settlement s = gm.GetSettlement(idx);
			m_aSpots.Insert(LandPointNear(s.m_vCenter, s.m_fRadius * 0.2, s.m_fRadius * 0.6));
		}

		m_sName = RZ_NoteText.FirstName(m_Rng);
		m_bAlive = m_Rng.Chance(0.5);
		RZ_Settlement third = gm.GetSettlement(c);
		int note = AddClue("A note on the kitchen table", string.Format("Gone to %1 to look for Mum. She was at the school there last week, someone said. Don't wait up. Back by dark.\n%2",
			second.m_sName, RZ_NoteText.Signoff(m_Rng, m_sName)));
		SpawnClue(gm, RZ_EPuzzleProp.CLUE, m_aSpots[0], note, "Read the note on the table");

		m_iSettlement = a;
		m_vPos = m_aSpots[0];
		m_sTitle = "Trail of Clues";
		m_sBrief = string.Format("%1 walked out of %2 three days ago and nobody's heard since. Their family asked a wanderer, the wanderer asked us. Start at the house in %2. Follow what they left behind.",
			m_sName, first.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected string Where(RZ_GameModeComponent gm, int playerId, int step)
	{
		RZ_Settlement s = gm.GetSettlement(m_aTowns[step]);
		string text = "";
		if (s)
			text = s.m_sName;

		if (HasBonus(gm, playerId))
			text += string.Format(" (grid %1)", RZ_SettlementScanner.GridReference(m_aSpots[step]));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		if (idx == 0 && m_iStep == 0)
		{
			m_iStep = 1;
			int bag = AddClue(m_sName + "'s bag", string.Format("A school bag with %1 written inside the flap. A map with %2 circled twice, and a page torn from a diary: \"%3. Mum isn't here. They say a doctor went toward %2. Going.\"",
				m_sName, Where(gm, playerId, 2), RZ_NoteText.Day(DayNow(gm))));
			SpawnClue(gm, RZ_EPuzzleProp.CLUE, m_aSpots[1], bag, "Look in the bag");
			MoveToStep(gm, 1);
			gm.NotifyPlayer(playerId, m_sTitle, "Next: " + Where(gm, playerId, 1) + ".");
			return;
		}

		if (idx == 1 && m_iStep == 1)
		{
			m_iStep = 2;
			if (m_bAlive)
			{
				int voice = AddClue(m_sName, "Thin, dirty, alive. \"I couldn't find her. I couldn't go back without her.\"");
				SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_aSpots[2], 0, "", "Bring them home", voice, "Talk to " + m_sName);
			}
			else
			{
				m_iFinal = AddClue("A last letter", string.Format("Mum, if they find this before you do: I got as far as %1. I'm not scared. Tell the others not to come this way.\n%2",
					Where(gm, playerId, 2), RZ_NoteText.Signoff(m_Rng, m_sName)));
				SpawnClue(gm, RZ_EPuzzleProp.BODY, m_aSpots[2], m_iFinal, "Search the body");
			}

			MoveToStep(gm, 2);
			gm.NotifyPlayer(playerId, m_sTitle, "Next: " + Where(gm, playerId, 2) + ".");
			return;
		}

		if (m_iFinal >= 0 && idx == m_iFinal)
		{
			m_fRewardScale = 0.7;
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The journal follows the trail: the next town (its centre: the exact spot stays the clue's
	//! job, or the ★ grid's).
	protected void MoveToStep(RZ_GameModeComponent gm, int step)
	{
		m_iSettlement = m_aTowns[step];
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
			m_vPos = s.m_vCenter;
		else
			m_vPos = m_aSpots[step];
	}

	//------------------------------------------------------------------------------------------------
	protected static int DayNow(RZ_GameModeComponent gm)
	{
		if (gm.GetBroadcast())
			return gm.GetBroadcast().GetDay() - 2;

		return 12;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (m_iStep < 2 || !m_bAlive)
			return;

		gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 nods and follows you. \"Is it far?\"", m_sName));
		m_eState = RZ_EMissionState.SUCCEEDED;
		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses && houses.Count() > 0)
			houses.AddRescued(m_aSpots[2], 1, m_aParticipants);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		if (m_bAlive)
			return string.Format("%1: %2 is found, and going home.", m_sTitle, m_sName);

		return string.Format("%1: %2 was found. Their family will know, at least.", m_sTitle, m_sName);
	}
}

//------------------------------------------------------------------------------------------------
//! #12 Barter Chain (Haven, through three wanderers): the first wants something the second has;
//! the second will only swap it for something the third has; the third wants food. Talk to
//! them in the right order (each step is remembered per player): share a food item with the
//! third, swap with the second, hand it to the first. All three remember you helped. Fails if
//! one of them dies. Data only: no items change hands except the food.
class RZ_Mission_BarterChain : RZ_Mission
{
	protected ref array<int> m_aPeople = {};		// first (wants), second (middle), third (wants food)
	protected ref map<int, int> m_mStage = new map<int, int>();
	protected string m_sWant;
	protected string m_sMiddle;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 6, 10);
		m_fTimeLeft = 2700;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		RZ_Wanderers wanderers = gm.GetWanderers();
		if (!wanderers)
			return 0;

		array<int> ids = {};
		wanderers.GetLiving(BarterRoles(), ids);
		if (ids.Count() < 3)
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	protected static int BarterRoles()
	{
		return RZ_Wanderers.RoleBit(RZ_EWandererRole.TRADER) | RZ_Wanderers.RoleBit(RZ_EWandererRole.DOCTOR) | RZ_Wanderers.RoleBit(RZ_EWandererRole.SCAVENGER)
			| RZ_Wanderers.RoleBit(RZ_EWandererRole.PREACHER) | RZ_Wanderers.RoleBit(RZ_EWandererRole.COURIER);
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Wanderers wanderers = gm.GetWanderers();
		if (!wanderers)
			return false;

		wanderers.GetLiving(BarterRoles(), m_aPeople);
		if (m_aPeople.Count() < 3)
			return false;

		while (m_aPeople.Count() > 3)
		{
			m_aPeople.Remove(m_aPeople.Count() - 1);
		}
		array<string> wants = {"reading glasses", "a pair of dry boots", "a sewing kit", "a photo album someone left behind"};
		array<string> middles = {"a tin of tobacco", "a bottle of something strong", "a pack of playing cards", "a good knife"};
		m_sWant = wants.GetRandomElement();
		m_sMiddle = middles.GetRandomElement();

		vector at;
		if (!wanderers.GetPosition(m_aPeople[0], at))
			return false;

		m_vPos = at;
		m_iSettlement = -1;
		m_sTitle = "Barter Chain";
		m_sBrief = string.Format("%1 has been after %2 for weeks. %3 has some, but wants %4 for it. %5 has %4 and hasn't eaten in two days. Bring food, and talk to them in the right order. (%3 is %6; %5 is %7.)",
			wanderers.GetName(m_aPeople[0]), m_sWant, wanderers.GetName(m_aPeople[1]), m_sMiddle, wanderers.GetName(m_aPeople[2]),
			wanderers.WhereIs(m_aPeople[1]), wanderers.WhereIs(m_aPeople[2]));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_Wanderers wanderers = gm.GetWanderers();
		foreach (int id : m_aPeople)
		{
			if (!wanderers || !wanderers.IsAlive(id))
			{
				m_eState = RZ_EMissionState.FAILED;
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_Wanderers.HandleTalk: a running barter chain that goes through this wanderer.
	//! True if it took the conversation.
	static bool OnWandererTalk(RZ_GameModeComponent gm, int playerId, int wandererId, string title, IEntity player)
	{
		if (!gm.GetMissions())
			return false;

		foreach (RZ_Mission m : gm.GetMissions().GetActive())
		{
			RZ_Mission_BarterChain chain = RZ_Mission_BarterChain.Cast(m);
			if (chain && chain.m_eState == RZ_EMissionState.ACTIVE && chain.Talk(gm, playerId, wandererId, title, player))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool Talk(RZ_GameModeComponent gm, int playerId, int wandererId, string title, IEntity player)
	{
		int who = m_aPeople.Find(wandererId);
		if (who < 0 || !IsEligible(gm, playerId))
			return false;

		int stage;
		m_mStage.Find(playerId, stage);

		// Third: food for the middle thing
		if (who == 2 && stage == 0)
		{
			array<string> keywords = {};
			RZ_Safehouses houses = gm.GetSafehouses();
			if (houses)
				keywords = houses.GetFoodKeywords();
			else
				RZ_FoodHelper.DefaultKeywords(keywords);

			if (RZ_FoodHelper.TakeFood(player, keywords, 1) <= 0)
			{
				gm.NotifyPlayer(playerId, title, string.Format("\"%1? Sure. For something to eat. Anything.\"", m_sMiddle));
				return true;
			}

			m_mStage.Set(playerId, 1);
			AddParticipantId(playerId);
			gm.NotifyPlayer(playerId, title, string.Format("They eat it in four bites and hand you %1 without a word.", m_sMiddle));
			return true;
		}

		// Middle: the middle thing for what the first one wants
		if (who == 1 && stage == 1)
		{
			m_mStage.Set(playerId, 2);
			gm.NotifyPlayer(playerId, title, string.Format("They turn %1 over in their hands, grin, and give you %2.", m_sMiddle, m_sWant));
			return true;
		}

		// First: done
		if (who == 0 && stage == 2)
		{
			AddParticipantId(playerId);
			gm.NotifyPlayer(playerId, title, string.Format("\"Is that... %1? Where did you...\" They don't finish. They don't need to.", m_sWant));
			RZ_Wanderers wanderers = gm.GetWanderers();
			if (wanderers)
			{
				foreach (int id : m_aPeople)
				{
					wanderers.RememberPlayer(id, playerId, RZ_Wanderers.MEM_HELPED);
				}
			}

			m_eState = RZ_EMissionState.SUCCEEDED;
			return true;
		}

		// Out of order: a hint
		if (who == 0)
			gm.NotifyPlayer(playerId, title, string.Format("\"Still no %1, then. %2 has some, I heard.\"", m_sWant, gm.GetWanderers().GetName(m_aPeople[1])));
		else if (who == 1)
			gm.NotifyPlayer(playerId, title, string.Format("\"%1? Maybe. If you bring me %2.\"", m_sWant, m_sMiddle));
		else
			gm.NotifyPlayer(playerId, title, "\"I've nothing left you'd want. Unless you've got food.\"");

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void AddParticipantId(int playerId)
	{
		if (!m_aParticipants.Contains(playerId))
			m_aParticipants.Insert(playerId);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		RZ_Wanderers wanderers = gm.GetWanderers();
		vector at;
		if (wanderers && !m_aPeople.IsEmpty() && wanderers.GetPosition(m_aPeople[0], at))
			return string.Format("%1, %2", wanderers.GetName(m_aPeople[0]), RZ_Util.DirectionText(from, at));

		return "on the roads";
	}
}

//------------------------------------------------------------------------------------------------
//! #13 Distraction (any faction, picked each time): a stash in a house that the infected guard.
//! Somebody keeps them busy with noise at a marked spot on the other side of the street while
//! somebody else searches. The search takes three minutes and only goes on while the noise does
//! (a loud noise at the spot in the last 15 s) and the searcher isn't noticed (one of them
//! close for 8 s resets it). Doable alone with timed noisemakers.
class RZ_Mission_Distraction : RZ_PuzzleMission
{
	protected vector m_vNoise;
	protected vector m_vStash;
	protected ref array<IEntity> m_aGuards = {};
	protected ref array<IEntity> m_aWaypoints = {};
	protected float m_fSearched;
	protected int m_iSearcher = -1;
	protected bool m_bDrawn;
	protected float m_fQuiet;

	protected static const float SEARCH_SECONDS = 180;
	protected static const float NOISE_RADIUS = 60;
	protected static const float NOISE_AGE = 15;
	protected static const float SEARCH_RADIUS = 10;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		RZ_EFaction giver = Math.RandomInt(0, RZ_Factions.COUNT);
		SetContract(giver, RZ_ETier.STRANGER, 6, 12);
		m_fTimeLeft = 2400;
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
		m_iSettlement = PickSettlement(gm, 30, 90);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		float axis = m_Rng.Range(0, 360);
		m_vStash = LandPointNear(RZ_Compass.PointAt(s.m_vCenter, axis, s.m_fRadius * 0.3), 0, 15);
		m_vNoise = LandPointNear(RZ_Compass.PointAt(m_vStash, axis + 180, Math.RandomFloat(150, 220)), 0, 10);
		if (RZ_Util.IsWater(m_vStash) || RZ_Util.IsWater(m_vNoise))
			return false;

		int plate = AddClue("Chalk on the door", "A cross in chalk, and under it: CELLAR. FULL. DON'T.");
		if (!SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vStash, 0, "", "Search the cellar", plate, "Read the chalk"))
			return false;

		for (int i = 0; i < 4; i++)
		{
			RZ_TrackedZombie z = gm.GetSpawner().SpawnZombie(LandPointNear(m_vStash, 4, 15), false, m_iSettlement, m_iId);
			if (z)
				m_aGuards.Insert(z.m_Entity);
		}

		m_vPos = m_vStash;
		m_fParticipationRadius = 250;
		GetStealth().m_fHeatThreshold = 1000;	// the noise is the point: only closeness counts
		GetStealth().m_fZombieRadius = 10;
		GetStealth().m_fZombieSeconds = 8;
		m_sTitle = "Distraction";
		m_sBrief = string.Format("A cellar in %1 is full of what we need, and they're standing on top of it. Someone keeps them busy with noise at grid %2, across the street, for three minutes, while someone else empties the cellar (grid %3). Bring a friend, or a lot of firecrackers.",
			s.m_sName, RZ_SettlementScanner.GridReference(m_vNoise), RZ_SettlementScanner.GridReference(m_vStash));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		GetStealth().Track(gm, ids, players, dt, m_vStash, 60);

		vector noise;
		bool loud = gm.GetHeat().FindRecentNoise(m_vNoise, NOISE_RADIUS, NOISE_AGE, noise);
		if (loud && !m_bDrawn)
		{
			m_bDrawn = true;
			RZ_UnarmedHelper.SteerUnits(gm, m_aGuards, m_vNoise, m_aWaypoints);
		}

		if (!loud)
		{
			m_fQuiet += dt;
			if (m_bDrawn && m_fQuiet >= 30)
			{
				m_bDrawn = false;
				RZ_UnarmedHelper.SteerUnits(gm, m_aGuards, m_vStash, m_aWaypoints);
			}
		}
		else
		{
			m_fQuiet = 0;
		}

		if (m_iSearcher <= 0)
			return;

		IEntity searcher = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iSearcher);
		if (!searcher || RZ_Util.DistXZ(searcher.GetOrigin(), m_vStash) > SEARCH_RADIUS)
		{
			m_iSearcher = -1;
			return;
		}

		if (GetStealth().IsNoticed(gm, searcher))
		{
			int seen = m_iSearcher;
			m_fSearched = 0;
			m_iSearcher = -1;
			GetStealth().Forget(seen);
			RZ_UnarmedHelper.SteerUnits(gm, m_aGuards, searcher.GetOrigin(), m_aWaypoints);
			gm.NotifyPlayer(seen, m_sTitle, "One of them came back and saw you in the cellar door. Get out. You'll have to start again.");
			return;
		}

		if (!loud)
			return;

		m_fSearched += dt;
		if (m_fSearched >= SEARCH_SECONDS)
		{
			m_eState = RZ_EMissionState.SUCCEEDED;
			gm.NotifyPlayer(m_iSearcher, m_sTitle, "The last box comes up the steps. Go, quietly.");
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (m_iSearcher > 0 && m_iSearcher != playerId)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "Someone's already down there. Keep the noise going.");
			return;
		}

		m_iSearcher = playerId;
		int left = RZ_Util.RoundToInt(SEARCH_SECONDS - m_fSearched);
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("You start bringing it up, box by box. About %1 seconds of work, and only while there's noise across the street. Stay within 10 m.", left));
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		SpawnRewardAt(gm, m_vStash);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		RZ_UnarmedHelper.DeleteAll(m_aWaypoints);
		m_aGuards.Clear();
	}
}

//------------------------------------------------------------------------------------------------
//! #15 Quiet Count (Halcyon): count the specials in a town from a distance. Answer at the field
//! notebook at an observation post outside it. Getting within 50 m of one of them halves the
//! pay (and they come for you); a wrong count fails it. The answer is how many are alive when
//! you answer. A right count is also a little research. ★ Lab Tech: more Fame.
//! *Simplified:* the town's own infected are cleared and kept out while it runs (the spawner's
//! suppression, as in The Silent Village), so the only specials there are the ones being counted.
class RZ_Mission_QuietCount : RZ_PuzzleMission
{
	protected vector m_vPost;
	protected bool m_bTooClose;

	protected static const float CLOSE_RADIUS = 50;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.STRANGER, 10, 10);
		m_Req.Bonus("labtech");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 2400;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.OBJECT))
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 40, 95);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPost = HighPointNear(s.m_vCenter, s.m_fRadius + 200, s.m_fRadius + 350);
		if (RZ_Util.IsWater(m_vPost))
			return false;

		int notebook = AddClue("Field notebook", "HALCYON - FIELD OBSERVATION. Count mutated subjects only. Do not approach closer than 50 m. Do not engage. Record the number seen NOW, not earlier.");
		if (!SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vPost, 0, "", "Record: one or none|Record: two|Record: three|Record: four or more", notebook, "Read the field notebook"))
			return false;

		int specials = m_Rng.Int(2, 5);
		for (int i = 0; i < specials; i++)
		{
			gm.GetSpawner().SpawnZombie(LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.7), true, m_iSettlement, m_iId);
		}

		SpawnGuards(gm, s.m_vCenter, 3, 0, s.m_fRadius * 0.7);

		// Ambient specials would be indistinguishable from the counted ones: keep the town to the
		// mission's own infected until it ends (mission zombies are never suppressed)
		gm.GetSpawner().SetSuppressed(m_iSettlement, true);
		m_vPos = s.m_vCenter;
		m_fParticipationRadius = s.m_fRadius + 400;
		m_sTitle = "Quiet Count";
		m_sBrief = string.Format("We need to know how many of the changed ones are in %1 right now. Not a fight, a count. Binoculars, patience, and stay at least 50 m from them. Write it in the notebook at the observation post (grid %2).",
			s.m_sName, RZ_SettlementScanner.GridReference(m_vPost));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bTooClose)
			return;

		foreach (int i, IEntity p : players)
		{
			if (!IsInvolved(ids[i]) || !gm.GetSpawner().IsMissionSpecialNear(m_iId, p.GetOrigin(), CLOSE_RADIUS))
				continue;

			m_bTooClose = true;
			m_fRewardScale = 0.5;
			array<IEntity> units = {};
			gm.GetSpawner().GetMissionUnits(m_iId, units);
			array<IEntity> made = {};
			RZ_UnarmedHelper.SteerUnits(gm, units, p.GetOrigin(), made);
			foreach (IEntity wp : made)
			{
				TrackProp(wp);
			}

			gm.NotifyPlayer(ids[i], m_sTitle, "Too close. It's seen you, and now they all know you're here. The count still matters, but Halcyon will hear how you got it.");
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		int alive = gm.GetSpawner().CountSpecialsForMission(m_iId);
		int answer = option + 1;
		bool right = answer == alive || (option == 0 && alive <= 1) || (option == 3 && alive >= 4);
		if (!right)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You write it down. Back at the lab, someone checks it against the drone footage, and shakes their head.");
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		gm.NotifyPlayer(playerId, m_sTitle, "You write it down, and the number is right.");
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.GetCure().AddResearch(1, "A careful count from the field.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		if (m_iSettlement >= 0)
			gm.GetSpawner().SetSuppressed(m_iSettlement, false);

		super.Cleanup(gm);
	}
}
