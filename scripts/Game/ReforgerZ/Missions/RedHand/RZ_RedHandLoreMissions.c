//! The Red Hand's hard edges (docs/LORE.md, "The Red Hand"; docs/MISSIONS.md, "The Red Hand's
//! hard edges"): what the crews don't say out loud, as contracts with a choice in them. Built on
//! the mission building blocks (Missions/Blocks/) and the Perimeter helper (Missions/Perimeter/).
//! Each one turns itself off when a prefab slot it needs is empty.

//------------------------------------------------------------------------------------------------
//! Shared by the hard-edge contracts.
class RZ_RedHandEdges
{
	//------------------------------------------------------------------------------------------------
	//! A debtor's name: "a lad in the book".
	static string DebtorName(RZ_Rng rng)
	{
		array<string> names = {"Pavel", "Henri", "Tomas", "Emil", "Anton", "Viktor", "Karel", "Oskar", "Milan", "Jakub", "Bruno", "Leon"};
		return rng.Pick(names);
	}

	//------------------------------------------------------------------------------------------------
	//! The Red Hand's pit: one all season, the canon spot redhand.pit (docs/DESIGN.md, "World
	//! facts": behind the boss's seat at the stronghold, else a crew boss's, else outside a Red Hand
	//! holding or a quiet town). Fight Pit and The Pit's Debtor use it. `place` is for a brief: it
	//! holds the seat's tokens, filled when the brief is shown. False if the map has no spot for it.
	static bool FindPit(RZ_GameModeComponent gm, out vector pos, out string place)
	{
		RZ_WorldFacts facts = gm.GetFacts();
		if (facts.CanonText("redhand.pit").IsEmpty())
			return false;

		pos = facts.CanonPos("redhand.pit");
		place = PitPlace(gm);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The leader id of the seat the pit is behind (redhand.pit.seat), -1 if it isn't behind one.
	static int PitSeat(RZ_GameModeComponent gm)
	{
		RZ_WorldFacts facts = gm.GetFacts();
		if (facts.CanonText("redhand.pit").IsEmpty())
			return -1;

		string seat = facts.CanonText("redhand.pit.seat");
		if (seat.IsEmpty())
			return -1;

		return seat.ToInt();
	}

	//------------------------------------------------------------------------------------------------
	//! "the garages behind {leader_short:7}'s camp in {leader_place:7}", or "the old yard outside
	//! Morton": words for a brief (Fill turns the tokens into whoever sits there now).
	static string PitPlace(RZ_GameModeComponent gm)
	{
		int seat = PitSeat(gm);
		if (seat >= 0)
			return string.Format("the garages behind {leader_short:%1}'s camp in {leader_place:%1}", seat);

		RZ_Settlement s = gm.GetSettlement(gm.GetFacts().CanonSettlement("redhand.pit.town"));
		if (s)
			return "the old yard outside " + s.m_sName;

		return "the old yard";
	}

	//------------------------------------------------------------------------------------------------
	//! Where someone you got away is taken: the nearest Haven safehouse 300 m to 6 km from `from`,
	//! else the least infected town at least 300 m away. False if neither.
	static bool HavenRefuge(RZ_GameModeComponent gm, vector from, out vector pos, out string name)
	{
		int house = RZ_LeaderJob.SafehouseNear(gm, from, 300, 6000, -1);
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, house);
		if (h)
		{
			pos = RZ_Mission.LandPointNear(h.m_vPos, 4, 10);
			name = "the Haven safehouse in " + h.m_sName;
			return true;
		}

		int town = RZ_Mission.PickSafestSettlement(gm);
		RZ_Settlement s = gm.GetSettlement(town);
		if (!s || RZ_Util.DistXZ(s.m_vCenter, from) < 300)
			return false;

		pos = RZ_Mission.LandPointNear(s.m_vCenter, 0, Math.Max(20, s.m_fRadius * 0.4));
		name = "the Haven's people in " + s.m_sName;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! An optional AI group near pos, watched for retaliation. Null if the prefab is empty.
	static IEntity SpawnCrew(RZ_GameModeComponent gm, ResourceName prefab, vector pos, float minR, float maxR)
	{
		if (prefab.IsEmpty())
			return null;

		IEntity crew = RZ_Util.SpawnPrefab(prefab, RZ_Mission.LandPointNear(pos, minR, maxR), Math.RandomFloat(0, 360));
		if (crew && gm.GetHostility())
			gm.GetHostility().WatchUnit(crew, RZ_EFaction.BANDITS);

		return crew;
	}

	//------------------------------------------------------------------------------------------------
	//! Did the player choose against the Red Hand while it pays? The Haven pays instead: anyone it
	//! doesn't hunt. The Red Hand leader who voiced the job doesn't count it as help.
	static void PaidByHaven(RZ_Mission m, int fame, int favor, int redHandInfamy)
	{
		m.m_eGiver = RZ_EFaction.SURVIVORS;
		m.m_iFameReward = fame;
		m.m_iFavorReward = favor;
		m.m_bHasRival = true;
		m.m_eRival = RZ_EFaction.BANDITS;
		m.m_iRivalInfamy = redHandInfamy;
		m.m_bNoLeaderCredit = true;
		m.m_iLeaderId = -1;
	}
}

//------------------------------------------------------------------------------------------------
//! Learning the Drones (Red Hand, Known). The gap in the drones' rotation was found by sending
//! people toward the line and timing what happened. A crew is doing it again: a debtor waits with
//! them outside the warning zone. **Walk him to the line**: he follows a route to a marker just
//! short of the kill zone (RZ_Escort), walks on alone, and you count from the marker
//! (TIMING seconds with a player within TIMING_RADIUS of it). The drones fire a warning shot;
//! then, DEATH_CHANCE of the time, the second one. Either way the gap is timed: Red Hand Fame and
//! Favor. **Get him away**: he follows you to the Haven (the crew fights, if the Red Hand group
//! prefab is set); the Haven pays, Red Hand Infamy, and he joins the safehouse.
//! ★ Radio Operator: hears the drones' net, so counts twice as fast, and he's out there for half
//! as long (BONUS_DEATH_CHANCE). Spots come from RZ_PerimeterMissionHelper.FindSpots, so it works
//! where the edge runs over land and where it runs out at sea (he wades out from the shore).
class RZ_Mission_LearningTheDrones : RZ_PuzzleMission
{
	protected ref RZ_Escort m_Escort = new RZ_Escort();
	protected vector m_vStart;
	protected vector m_vMark;
	protected vector m_vOut;
	protected vector m_vRefuge;
	protected string m_sRefuge;
	protected string m_sDebtor;
	protected bool m_bWater;
	protected bool m_bShown;
	protected IEntity m_Debtor;
	protected IEntity m_Crew;
	protected int m_iTalk = -1;
	protected int m_iPath;
	protected float m_fTimed;
	protected bool m_bBonus;
	protected bool m_bWarned;
	protected string m_sFail;

	protected static const int PATH_NONE = 0;
	protected static const int PATH_WALK = 1;
	protected static const int PATH_TIMING = 2;
	protected static const int PATH_AWAY = 3;

	protected static const float MARK_GAP = 20;			// the marker is this far outside the kill zone
	protected static const float TIMING = 90;
	protected static const float TIMING_RADIUS = 50;
	protected static const float DEATH_CHANCE = 0.55;
	protected static const float BONUS_DEATH_CHANCE = 0.35;
	protected static const float CLEARANCE = 500;		// from any checkpoint
	protected static const int MAX_TRIES = 40;
	protected static const int HAVEN_FAME = 15;
	protected static const int HAVEN_FAVOR = 10;
	protected static const int CROSS_INFAMY = 15;
	protected static const string KEY = "Learning the Drones";

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 20, 30);
		m_Req.m_bNotCured = true;	// the line stands down after the cure (and a running one is called off)
		m_Req.Bonus("radioham");
		m_fTimeLeft = 2700;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		RZ_Perimeter perimeter = RZ_PerimeterMissionHelper.Get(gm);
		if (!perimeter || !HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN) || !RZ_Escort.HasRoom(gm))
			return 0;

		if (!HasDroneSpots(perimeter))
			return 0;

		return 0.6;
	}

	//------------------------------------------------------------------------------------------------
	//! Worked out once per run (cached on the Perimeter, as the other missions at the line).
	protected static bool HasDroneSpots(RZ_Perimeter perimeter)
	{
		int state = perimeter.GetSpotState(KEY);
		if (state >= 0)
			return state == 1;

		array<int> spots = {};
		CollectSpots(perimeter, spots);
		bool found = !spots.IsEmpty();
		RZ_PerimeterMissionHelper.NoteSpots(perimeter, KEY, found);
		return found;
	}

	//------------------------------------------------------------------------------------------------
	//! Shore points whose marker (MARK_GAP outside the kill zone, or the shore if that's further
	//! in) is dry and still under the drones.
	protected static void CollectSpots(RZ_Perimeter perimeter, notnull array<int> result)
	{
		float markDepth = perimeter.GetKillDistance() + MARK_GAP;
		float limit = perimeter.GetWarnDistance() - 20;
		if (markDepth >= limit)
			return;

		array<int> spots = {};
		RZ_PerimeterMissionHelper.FindSpots(perimeter, markDepth, 0, CLEARANCE, limit, spots);
		foreach (int idx : spots)
		{
			if (RZ_PerimeterMissionHelper.SpotDepth(perimeter.GetShorePoint(idx), markDepth, 0) <= limit)
				result.Insert(idx);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Perimeter perimeter = RZ_PerimeterMissionHelper.Get(gm);
		if (!perimeter)
			return false;

		array<int> spots = {};
		CollectSpots(perimeter, spots);
		float markDepth = perimeter.GetKillDistance() + MARK_GAP;
		bool found = false;
		int tries = 0;
		foreach (int idx : spots)
		{
			if (tries >= MAX_TRIES)
				break;

			tries++;
			RZ_ShorePoint p = perimeter.GetShorePoint(idx);

			// The crew waits outside the warning zone, a short walk from the marker
			vector start = perimeter.PointInside(p.m_vEdge, p.m_vInward, perimeter.GetWarnDistance() + m_Rng.Range(120, 200));
			if (RZ_Util.IsWater(start) || perimeter.IsNearLine(start))
				continue;

			m_vStart = start;
			m_vMark = perimeter.PointInside(p.m_vEdge, p.m_vInward, RZ_PerimeterMissionHelper.SpotDepth(p, markDepth, 0));
			float outDepth = Math.Max(p.m_fLand, perimeter.GetKillDistance() * 0.4);
			m_vOut = perimeter.PointInside(p.m_vEdge, p.m_vInward, outDepth);
			m_bWater = p.m_fLand > perimeter.GetKillDistance() * 0.4;
			found = true;
			break;
		}

		if (!found)
		{
			if (tries == spots.Count())
				RZ_PerimeterMissionHelper.NoteSpots(perimeter, KEY, false);

			return false;
		}

		if (!RZ_RedHandEdges.HavenRefuge(gm, m_vStart, m_vRefuge, m_sRefuge))
			return false;

		m_vPos = m_vStart;
		m_iSettlement = gm.FindNearestSettlement(m_vStart);
		m_sDebtor = RZ_RedHandEdges.DebtorName(m_Rng);
		m_iTalk = AddClue(m_sDebtor, "\"They say if I come back, I'm square.\" He laughs, then stops. \"Has anyone come back? You'd know. You'd have heard.\"");

		m_Escort.m_sName = m_sDebtor;
		m_Escort.m_sPrefab = RZ_Escort.PrefabFor(gm, 0);
		m_Escort.SetDestination(m_vMark);

		m_sTitle = "Learning the Drones";
		m_sBrief = string.Format("%1 is in the book for more than he'll ever pay back, so today he pays it off a different way. The crew's waiting with him at grid %2. Walk him up toward the line, stop at grid %3 and let him go on alone. Count how long the drones take. Write it down. He walks, you count. Easy.",
			m_sDebtor, RZ_SettlementScanner.GridReference(m_vStart), RZ_SettlementScanner.GridReference(m_vMark));
		return m_Escort.Begin(gm, m_vStart);
	}

	//------------------------------------------------------------------------------------------------
	override string GetLocationName(RZ_GameModeComponent gm)
	{
		return "the quarantine line, grid " + RZ_SettlementScanner.GridReference(m_vStart);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = string.Format("%1, %2", GetLocationName(gm), RZ_Util.DirectionText(from, m_vPos));
		if (m_iPath == PATH_AWAY)
			return text + string.Format("\nTake him to: %1 (%2)", m_sRefuge, RZ_Util.DirectionText(from, m_vRefuge));

		return text + string.Format("\nThe marker: grid %1 (%2)", RZ_SettlementScanner.GridReference(m_vMark), RZ_Util.DirectionText(from, m_vMark));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bShown && RZ_ContractHelp.AnyNear(players, m_vStart, 300))
			ShowStart(gm);

		if (m_iPath == PATH_WALK)
		{
			m_Escort.Update(gm, dt, ids, players);
			m_vPos = m_Escort.m_vPos;
			RZ_ContractHelp.AddNear(ids, players, m_vPos, 120, m_aParticipants);
			if (m_Escort.IsDead())
			{
				m_sFail = string.Format("%1 never reached the line. Nobody timed anything.", m_sDebtor);
				m_eState = RZ_EMissionState.FAILED;
			}
			else if (m_Escort.IsArrived())
			{
				StartTiming(gm);
			}

			return;
		}

		if (m_iPath == PATH_TIMING)
		{
			UpdateTiming(gm, dt, ids, players);
			return;
		}

		if (m_iPath != PATH_AWAY)
			return;

		m_Escort.Update(gm, dt, ids, players);
		m_vPos = m_Escort.m_vPos;
		RZ_ContractHelp.AddNear(ids, players, m_vPos, 100, m_aParticipants);
		if (m_Escort.IsDead())
		{
			m_sFail = string.Format("%1 didn't make it to the Haven.", m_sDebtor);
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (!m_Escort.IsNear(m_vRefuge, 25))
			return;

		m_Escort.Arrive();
		RZ_RedHandEdges.PaidByHaven(this, HAVEN_FAME, HAVEN_FAVOR, CROSS_INFAMY);
		RZ_LeaderJob.AddPeople(gm, m_vRefuge, 1);
		NotifyParticipants(gm, m_sTitle, string.Format("The Haven takes %1 in without asking what he owes. Somewhere a crew boss is crossing his name out of the book, and writing yours in.", m_sDebtor));
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowStart(RZ_GameModeComponent gm)
	{
		m_bShown = true;
		m_Debtor = SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vStart, 0, "", "Walk him to the line|Get him away from here", m_iTalk, "Talk to " + m_sDebtor);
		if (!m_Debtor)
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		m_Crew = RZ_RedHandEdges.SpawnCrew(gm, gm.GetRedHandGroupPrefab(), m_vStart, 8, 15);
		TrackProp(m_Crew);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0 || m_iPath != PATH_NONE)
			return;

		if (option == 0)
		{
			m_iPath = PATH_WALK;
			RZ_Util.DeleteUnit(m_Debtor);
			m_Escort.Go();
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 nods like he expected nothing else. \"Right. Let's go, then.\" Walk with him to grid %2. He won't go without you.", m_sDebtor, RZ_SettlementScanner.GridReference(m_vMark)));
			return;
		}

		if (option != 1)
			return;

		m_iPath = PATH_AWAY;
		m_aParticipants.Clear();
		m_aParticipants.Insert(playerId);
		RZ_Util.DeleteUnit(m_Debtor);
		m_Escort.Follow(playerId);
		m_Escort.Go();
		string text = string.Format("You tell %1 to walk with you and not look back. He doesn't ask where. Take him to %2.", m_sDebtor, m_sRefuge);
		if (RZ_Util.CountAlive(m_Crew) > 0)
		{
			RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, m_Crew, 150);
			text += " Behind you, somebody shouts his name. Then yours.";
		}

		gm.NotifyPlayer(playerId, m_sTitle, text);
	}

	//------------------------------------------------------------------------------------------------
	//! At the marker he walks on alone (out of sight: the escort's body goes). Count from there.
	protected void StartTiming(RZ_GameModeComponent gm)
	{
		m_iPath = PATH_TIMING;
		m_Escort.Cleanup();
		m_vPos = m_vMark;
		string away = "walks on toward the line on his own, the way they told him to";
		if (m_bWater)
			away = "wades out into the water on his own, the way they told him to";

		RZ_ContractHelp.NotifyNear(gm, m_vMark, 150, m_sTitle, string.Format("At the marker %1 stops, looks back once, and %2. Count. Stay where you are.", m_sDebtor, away));
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateTiming(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!RZ_ContractHelp.AnyNear(players, m_vMark, TIMING_RADIUS))
			return;

		RZ_ContractHelp.AddNear(ids, players, m_vMark, TIMING_RADIUS, m_aParticipants);
		float rate = 1;
		if (AnyBonusNear(gm, ids, players, m_vMark, TIMING_RADIUS))
		{
			rate = 2;
			m_bBonus = true;
		}

		m_fTimed += dt * rate;
		RZ_Perimeter perimeter = RZ_PerimeterMissionHelper.Get(gm);
		if (!m_bWarned && m_fTimed >= TIMING * 0.35)
		{
			m_bWarned = true;
			if (perimeter)
				perimeter.SpawnEffect(RZ_Util.RandomPointInRing(m_vOut, 3, 6));

			RZ_ContractHelp.NotifyNear(gm, m_vMark, 150, m_sTitle, "A shot kicks up the ground beside him. The loudspeaker says something you can't make out. He keeps going. That's what he was told to do.");
		}

		if (m_fTimed < TIMING)
			return;

		int gap = m_Rng.Int(35, 56);
		float chance = DEATH_CHANCE;
		if (m_bBonus)
			chance = BONUS_DEATH_CHANCE;

		if (Math.RandomFloat01() < chance)
		{
			if (perimeter)
				perimeter.SpawnEffect(m_vOut);

			NotifyParticipants(gm, m_sTitle, string.Format("The second shot comes %1 seconds after the first. %2 goes down and doesn't move. You write the number down. Somebody will use it.", gap, m_sDebtor));
		}
		else
		{
			NotifyParticipants(gm, m_sTitle, string.Format("The drone over him turns away at the end of its run. %1 seconds before the next one comes round. %2 comes back very slowly, shaking, laughing at nothing. The book says he's square.", gap, m_sDebtor));
		}

		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	//! The Haven pays anyone it doesn't hunt for getting him away.
	override bool IsEligible(RZ_GameModeComponent gm, int playerId)
	{
		if (m_iPath == PATH_AWAY && m_eState != RZ_EMissionState.ACTIVE)
			return !RZ_Reputation.IsHunted(gm.GetTier(playerId, RZ_EFaction.SURVIVORS));

		return super.IsEligible(gm, playerId);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (!m_sFail.IsEmpty())
			return m_sFail;

		if (m_iPath == PATH_TIMING)
			return string.Format("Learning the Drones: nobody stayed at the marker to count. %1 walked out there for nothing.", m_sDebtor);

		return string.Format("Learning the Drones: nobody came for %1. The crew found somebody else to count.", m_sDebtor);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Escort.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! The Pit's Debtor (Red Hand, Known). The pit is the season's one pit, the canon spot Fight Pit
//! uses too (RZ_RedHandEdges.FindPit), and it holds it. Nobody volunteered, so a debtor goes in, and you're his
//! second. Talk to him: **pay what he owes** (DEBT_FAVOR Red Hand Favor, only if the Red Hand
//! hired you: checked before the Favor is taken; pays PAY_SCALE), **rig the pens** (caught
//! RIG_CAUGHT of the time: Red Hand Infamy at once, failed; else keep him standing in the ring for
//! RIG_SECONDS against two small rounds), or **get him out** (he follows you to the Haven; while
//! he's within SPOT_RADIUS of the pit the crowd may see: Red Hand Infamy at once, and the pit crew
//! fights if the Red Hand group prefab is set). Out: the Haven pays, Red Hand Infamy.
//! ★ Ex-Con: caught rigging RIG_CAUGHT_EXCON, half the chance of being seen walking him out.
class RZ_Mission_PitsDebtor : RZ_PuzzleMission
{
	//------------------------------------------------------------------------------------------------
	//! The pit is this job's while it runs (a claim): not a Fight Pit there at the same time
	override void GetClaims(RZ_GameModeComponent gm, notnull array<string> result)
	{
		result.Insert(RZ_WorldFacts.PIT);
	}

	protected ref RZ_Escort m_Escort = new RZ_Escort();
	protected vector m_vPit;
	protected string m_sPit;
	protected vector m_vRefuge;
	protected string m_sRefuge;
	protected string m_sDebtor;
	protected IEntity m_Debtor;
	protected IEntity m_Crew;
	protected bool m_bShown;
	protected int m_iTalk = -1;
	protected int m_iPath;
	protected int m_iSecond = -1;
	protected float m_fHeld;
	protected int m_iRounds;
	protected float m_fSpotTimer;
	protected bool m_bSpotted;
	protected bool m_bClear;
	protected string m_sFail;

	protected static const int PATH_NONE = 0;
	protected static const int PATH_PAY = 1;
	protected static const int PATH_RIG = 2;
	protected static const int PATH_OUT = 3;

	protected static const int DEBT_FAVOR = 25;
	protected static const float PAY_SCALE = 0.5;
	protected static const float RIG_CAUGHT = 0.25;
	protected static const float RIG_CAUGHT_EXCON = 0.1;
	protected static const int CAUGHT_INFAMY = 10;
	protected static const float RIG_SECONDS = 150;
	protected static const float RING_RADIUS = 25;
	protected static const int ROUNDS = 2;
	protected static const float FIRST_ROUND = 20;
	protected static const float ROUND_GAP = 60;
	protected static const int ROUND_SIZE = 3;
	protected static const float SPOT_RADIUS = 60;
	protected static const float SPOT_EVERY = 5;
	protected static const float SPOT_DAY = 0.3;
	protected static const float SPOT_NIGHT = 0.12;
	protected static const int SPOTTED_INFAMY = 8;
	protected static const int HAVEN_FAME = 15;
	protected static const int HAVEN_FAVOR = 10;
	protected static const int CROSS_INFAMY = 15;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 15, 30);
		m_Req.Bonus("excon");
		m_fTimeLeft = 2400;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN) || !RZ_Escort.HasRoom(gm))
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		if (!RZ_RedHandEdges.FindPit(gm, m_vPit, m_sPit))
			return false;

		if (!RZ_RedHandEdges.HavenRefuge(gm, m_vPit, m_vRefuge, m_sRefuge))
			return false;

		m_vPos = m_vPit;
		m_iSettlement = gm.FindNearestSettlement(m_vPit);
		m_sDebtor = RZ_RedHandEdges.DebtorName(m_Rng);
		m_iTalk = AddClue(m_sDebtor, "\"Sixty tins. I had a shop, before. I sold them the tins in the first place.\" He's taped his hands up like he's seen in films. It won't help.");

		m_Escort.m_sName = m_sDebtor;
		m_Escort.m_sPrefab = RZ_Escort.PrefabFor(gm, 0);
		m_Escort.m_bHide = false;

		m_sTitle = "The Pit's Debtor";
		m_sBrief = string.Format("Pit's on tonight at %1, and nobody volunteered, so %2 goes in. He owes the book sixty tins. The crowd wants three rounds out of him. Be his second. Make sure he lasts, pay what he owes, or do something stupid. Your choice. Not his.",
			m_sPit, m_sDebtor);
		return m_Escort.Begin(gm, m_vPit);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = string.Format("%1, %2", GetLocationName(gm), RZ_Util.DirectionText(from, m_vPos));
		if (m_iPath == PATH_OUT)
			text += string.Format("\nTake him to: %1 (%2)", m_sRefuge, RZ_Util.DirectionText(from, m_vRefuge));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bShown && RZ_ContractHelp.AnyNear(players, m_vPit, 300))
			ShowPit(gm);

		if (m_iPath == PATH_RIG)
		{
			UpdateRing(gm, dt, ids, players);
			return;
		}

		if (m_iPath != PATH_OUT)
			return;

		m_Escort.Update(gm, dt, ids, players);
		m_vPos = m_Escort.m_vPos;
		RZ_ContractHelp.AddNear(ids, players, m_vPos, 100, m_aParticipants);
		if (m_Escort.IsDead())
		{
			m_sFail = string.Format("%1 never got clear of the pit.", m_sDebtor);
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		UpdateSpotting(gm, dt);
		if (!m_Escort.IsNear(m_vRefuge, 25))
			return;

		m_Escort.Arrive();
		RZ_RedHandEdges.PaidByHaven(this, HAVEN_FAME, HAVEN_FAVOR, CROSS_INFAMY);
		RZ_LeaderJob.AddPeople(gm, m_vRefuge, 1);
		NotifyParticipants(gm, m_sTitle, string.Format("The Haven gives %1 a blanket and a bowl and doesn't ask what he owes. He keeps the tape on his hands for days.", m_sDebtor));
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowPit(RZ_GameModeComponent gm)
	{
		m_bShown = true;
		string labels = string.Format("Pay what he owes (%1 Red Hand Favor)|Rig the pens|Get him out of here", DEBT_FAVOR);
		m_Debtor = SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPit, 0, "", labels, m_iTalk, "Talk to " + m_sDebtor);
		if (!m_Debtor)
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		m_Crew = RZ_RedHandEdges.SpawnCrew(gm, gm.GetRedHandGroupPrefab(), m_vPit, 10, 20);
		TrackProp(m_Crew);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0 || m_iPath != PATH_NONE)
			return;

		if (option == 0)
		{
			// Only someone the Red Hand sent may settle the book, and before any Favor changes hands
			if (!IsEligible(gm, playerId))
			{
				gm.NotifyPlayer(playerId, m_sTitle, "The pit boss looks you up and down. \"Who are you to pay his debts? Get out of the ring.\"");
				return;
			}

			if (!gm.SpendFavor(playerId, RZ_EFaction.BANDITS, DEBT_FAVOR))
			{
				gm.NotifyPlayer(playerId, m_sTitle, string.Format("The debt is %1 Red Hand Favor. You have %2.", DEBT_FAVOR, gm.GetProfile(playerId).GetFavor(RZ_EFaction.BANDITS)));
				return;
			}

			m_iPath = PATH_PAY;
			m_fRewardScale = PAY_SCALE;
			m_aParticipants.Clear();
			m_aParticipants.Insert(playerId);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("You pay it out of your own Favor. The pit boss writes \"Paid\" against that line of %1's and shrugs. \"Your money.\" %1 sits down in the crowd and watches somebody else go in.", m_sDebtor));
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (option == 1)
		{
			m_iPath = PATH_RIG;
			m_iSecond = playerId;
			float caught = RIG_CAUGHT;
			if (HasBonus(gm, playerId))
				caught = RIG_CAUGHT_EXCON;

			if (Math.RandomFloat01() < caught)
			{
				gm.AddInfamy(playerId, RZ_EFaction.BANDITS, CAUGHT_INFAMY);
				m_sFail = string.Format("The pen man counts the infected twice, then looks at you. \"Cheating the book?\" The crowd hears. So does the boss. %1 goes in anyway. (+%2 Red Hand Infamy)", m_sDebtor, CAUGHT_INFAMY);
				m_eState = RZ_EMissionState.FAILED;
				return;
			}

			RZ_Util.DeleteUnit(m_Debtor);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("A word with the pen man and a few tins in his pocket. Tonight the pens open slow and half empty. Get in the ring with %1 and keep him standing.", m_sDebtor));
			return;
		}

		if (option != 2)
			return;

		m_iPath = PATH_OUT;
		m_iSecond = playerId;
		m_aParticipants.Clear();
		m_aParticipants.Insert(playerId);
		RZ_Util.DeleteUnit(m_Debtor);
		m_Escort.m_bHide = true;
		m_Escort.Follow(playerId);
		m_Escort.Go();
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("You take %1 by the arm and walk him out of the ring like you've every right to. Don't run. Take him to %2.", m_sDebtor, m_sRefuge));
	}

	//------------------------------------------------------------------------------------------------
	//! Rigged: two small rounds, and he has to be standing at the end.
	protected void UpdateRing(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_Escort.Update(gm, dt, ids, players);
		if (m_Escort.IsDead())
		{
			m_sFail = string.Format("%1 didn't last. The crowd got its show anyway.", m_sDebtor);
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (!RZ_ContractHelp.AnyNear(players, m_vPit, RING_RADIUS))
			return;

		RZ_ContractHelp.AddNear(ids, players, m_vPit, RING_RADIUS, m_aParticipants);
		m_fHeld += dt;
		if (m_iRounds < ROUNDS && m_fHeld >= FIRST_ROUND + m_iRounds * ROUND_GAP)
		{
			m_iRounds++;
			SpawnGuards(gm, m_vPit, ROUND_SIZE, 20, 35);
			NotifyParticipants(gm, m_sTitle, string.Format("Round %1. Half the crowd is shouting for him. The other half bet against.", m_iRounds));
		}

		if (m_fHeld < RIG_SECONDS)
			return;

		NotifyParticipants(gm, m_sTitle, string.Format("Last bell. %1 is still standing, just. The crowd got its show, and the book balances.", m_sDebtor));
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	//! Walking him out: while he's near the pit, the crowd may notice (once).
	protected void UpdateSpotting(RZ_GameModeComponent gm, float dt)
	{
		if (m_bSpotted || m_bClear)
			return;

		if (!m_Escort.IsNear(m_vPit, SPOT_RADIUS))
		{
			m_bClear = true;
			gm.NotifyPlayer(m_iSecond, m_sTitle, "Nobody shouts. The crowd is already watching the next one.");
			return;
		}

		m_fSpotTimer += dt;
		if (m_fSpotTimer < SPOT_EVERY)
			return;

		m_fSpotTimer = 0;
		float chance = SPOT_DAY;
		if (RZ_GameTime.IsNight())
			chance = SPOT_NIGHT;

		if (HasBonus(gm, m_iSecond))
			chance = chance * 0.5;

		if (Math.RandomFloat01() >= chance)
			return;

		m_bSpotted = true;
		gm.AddInfamy(m_iSecond, RZ_EFaction.BANDITS, SPOTTED_INFAMY);
		string text = string.Format("\"Oi! Where's he going?\" The crowd has seen. (+%1 Red Hand Infamy)", SPOTTED_INFAMY);
		if (RZ_Util.CountAlive(m_Crew) > 0)
		{
			RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, m_Crew, 150);
			text += " The pit crew is coming after you.";
		}

		RZ_ContractHelp.NotifyNear(gm, m_vPit, 150, m_sTitle, text);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsEligible(RZ_GameModeComponent gm, int playerId)
	{
		if (m_iPath == PATH_OUT && m_eState != RZ_EMissionState.ACTIVE)
			return !RZ_Reputation.IsHunted(gm.GetTier(playerId, RZ_EFaction.SURVIVORS));

		return super.IsEligible(gm, playerId);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (!m_sFail.IsEmpty())
			return m_sFail;

		return string.Format("%1 went in the pit alone. Nobody has said how many rounds he lasted.", m_sDebtor);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Escort.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! The Red Hand's people-truck (docs/LORE.md, "Cargo") parked with its back padlocked: Tied Down's
//! shed, and the crew camp in the Cargo questline (Missions/Questlines/RedHand/RZ_Quest_Cargo.c).
//! The van is Supply > Red Hand truck, parked with no fuel so nobody drives it off before the
//! story says; its guards are the Red Hand group, else the truck crew (neither = none). Cutting
//! the lock: someone stays within DOOR_REACH of the door for CUT_SECONDS, and guards who are
//! alive see it. The owning mission spawns its own people and objects at the door and ticks this.
class RZ_CargoVan
{
	IEntity m_Van;
	IEntity m_Guards;
	vector m_vDoor;
	bool m_bOpen;
	bool m_bAlarm;
	int m_iCutter = -1;
	ref RZ_VehicleZone m_Zone = new RZ_VehicleZone();
	protected float m_fCut;

	static const float CUT_SECONDS = 12;
	static const float DOOR_REACH = 4;

	//------------------------------------------------------------------------------------------------
	//! Supply > Red Hand truck. "" = no van (the missions that need one never run).
	static ResourceName VanPrefab(RZ_GameModeComponent gm)
	{
		RZ_SupplySettings ss = gm.GetSupplySettings();
		if (!ss)
			return "";

		return ss.m_sRedHandTruck;
	}

	//------------------------------------------------------------------------------------------------
	//! Supply > Red Hand crew, else the Red Hand group. "" = no crew.
	static ResourceName CrewPrefab(RZ_GameModeComponent gm)
	{
		RZ_SupplySettings ss = gm.GetSupplySettings();
		if (ss && !ss.m_sRedHandCrew.IsEmpty())
			return ss.m_sRedHandCrew;

		return gm.GetRedHandGroupPrefab();
	}

	//------------------------------------------------------------------------------------------------
	//! The guards: the Red Hand group, else the truck crew.
	static ResourceName GuardPrefab(RZ_GameModeComponent gm)
	{
		ResourceName prefab = gm.GetRedHandGroupPrefab();
		if (prefab.IsEmpty())
			prefab = CrewPrefab(gm);

		return prefab;
	}

	//------------------------------------------------------------------------------------------------
	//! Parks the van at pos with no fuel, its guards 8-18 m off (onGuard: they fight at once) and its
	//! door a few metres away. False if the van can't be spawned.
	bool Park(RZ_GameModeComponent gm, vector pos, float yaw, bool onGuard)
	{
		m_Van = RZ_Util.SpawnPrefab(VanPrefab(gm), pos + Vector(0, 0.5, 0), yaw);
		if (!m_Van)
			return false;

		RZ_Util.SetVehicleFuel(m_Van, 0);
		m_Zone.SetVehicle(m_Van);
		m_Guards = RZ_RedHandEdges.SpawnCrew(gm, GuardPrefab(gm), pos, 8, 18);
		if (onGuard && m_Guards)
			RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, m_Guards, 150, true);

		m_vDoor = RZ_Mission.LandPointNear(pos, 6, 10);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A player starts on the padlock; guards who are alive see it (once). The line for them, ""
	//! if they were already cutting.
	string StartCut(int playerId)
	{
		if (m_bOpen || m_iCutter == playerId)
			return "";

		m_iCutter = playerId;
		m_fCut = 0;
		if (!m_bAlarm && RZ_Util.CountAlive(m_Guards) > 0)
		{
			m_bAlarm = true;
			RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, m_Guards, 150);
			return "A shout from the truck: the guards have seen you. Cut fast.";
		}

		return "You get the cutters on the padlock. Stay at the door.";
	}

	//------------------------------------------------------------------------------------------------
	//! The guards see something they shouldn't (once): they fight.
	void Alarm()
	{
		if (m_bAlarm || RZ_Util.CountAlive(m_Guards) == 0)
			return;

		m_bAlarm = true;
		RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, m_Guards, 150);
	}

	//------------------------------------------------------------------------------------------------
	bool IsCutting()
	{
		return m_iCutter > 0 && !m_bOpen;
	}

	//------------------------------------------------------------------------------------------------
	//! Once per tick: true on the tick the lock gives. Walking away from the door starts it over.
	bool UpdateCut(float dt)
	{
		if (m_bOpen || m_iCutter <= 0)
			return false;

		IEntity cutter = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iCutter);
		if (!cutter || RZ_Util.DistXZ(cutter.GetOrigin(), m_vDoor) > DOOR_REACH)
		{
			m_iCutter = -1;
			m_fCut = 0;
			return false;
		}

		m_fCut += dt;
		if (m_fCut < CUT_SECONDS)
			return false;

		m_bOpen = true;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void Fuel(float share)
	{
		RZ_Util.SetVehicleFuel(m_Van, share);
	}

	//------------------------------------------------------------------------------------------------
	//! The van is gone or burnt out (false before it was parked).
	bool IsLost()
	{
		return m_Zone.IsLost();
	}

	//------------------------------------------------------------------------------------------------
	//! The guards go; the van stays only if someone sits in it.
	void Cleanup()
	{
		RZ_Util.DeleteUnit(m_Guards);
		m_Guards = null;
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		if (m_Van && RZ_VehicleZone.DriverOf(m_Van, ids, players) < 0)
			RZ_Util.DeleteUnit(m_Van);
	}
}

//------------------------------------------------------------------------------------------------
//! Tied Down (Haven, Known). A Red Hand truck carries people, not tins. A step chain (RZ_StepChain):
//! a Haven lookout's note at a roadside outside one town names the next; there the truck is parked
//! (watch it from within WATCH_RADIUS for WATCH_SECONDS without coming within SPOT_RADIUS: seen,
//! its crew fights and it leaves at once); then it drives on to a shed outside a third town and
//! stops there. The truck is data while it moves: a parked vehicle (Supply > Red Hand truck, no
//! fuel) and its crew (Supply > Red Hand crew) appear only when a player is within 300 m.
//! At the shed (RZ_CargoVan): **cut the lock** (the guards fight) and drive the truck with them in
//! the back to the Haven (RZ_VehicleZone): full pay, +3 at the safehouse, Red Hand Infamy;
//! **radio the Haven**: REPORT_SCALE, +2 at the safehouse, a little Red Hand Infamy; or ask the
//! driver for **a cut**: the Red Hand pays anyone it doesn't hunt, Haven Infamy.
//! ★ Hunter: the crew only sees you within BONUS_SPOT_RADIUS. Reaching the shed tells the Cargo
//! questline (world flag cargo.truck_seen) that the Haven knows what these trucks are; while that
//! questline runs, this contract isn't offered.
class RZ_Mission_TiedDown : RZ_PuzzleMission
{
	protected ref RZ_StepChain m_Chain = new RZ_StepChain();
	protected ref RZ_CargoVan m_Load = new RZ_CargoVan();
	protected vector m_vStop;
	protected vector m_vPen;
	protected vector m_vTruckAt;
	protected vector m_vRefuge;
	protected string m_sStop;
	protected string m_sPen;
	protected string m_sRefuge;
	protected int m_iTruck;
	protected bool m_bLeave;
	protected bool m_bStopShown;
	protected IEntity m_Truck;
	protected IEntity m_Crew;
	protected bool m_bPenShown;
	protected bool m_bSpotted;
	protected int m_iChoice;
	protected int m_iDoorTalk = -1;
	protected int m_iDriverTalk = -1;
	protected string m_sFail;

	protected static const int TRUCK_STOP = 0;
	protected static const int TRUCK_ROAD = 1;
	protected static const int TRUCK_PEN = 2;

	protected static const int CHOICE_NONE = 0;
	protected static const int CHOICE_FREE = 1;
	protected static const int CHOICE_REPORT = 2;
	protected static const int CHOICE_CUT = 3;

	protected static const float WATCH_RADIUS = 80;
	protected static const float WATCH_SECONDS = 20;
	protected static const float SPOT_RADIUS = 30;
	protected static const float BONUS_SPOT_RADIUS = 15;
	protected static const float SHOW_RADIUS = 300;
	protected static const float TRUCK_SPEED = 8;
	protected static const float MAX_DRIVE = 300;
	protected static const float REPORT_SCALE = 0.6;
	protected static const int FREE_INFAMY = 15;
	protected static const int REPORT_INFAMY = 8;
	protected static const int CUT_FAME = 10;
	protected static const int CUT_FAVOR = 40;
	protected static const int CUT_INFAMY = 15;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 30, 25);
		m_Req.Bonus("hunter");
		// The Cargo questline tells the long version of this story; not both at once
		if (m_iQuestRun < 0)
			m_Req.NotWhileQuest("cargo");

		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Freeing them means driving the truck: off under the season's No Vehicles rule.
	override bool NeedsVehicle()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (RZ_CargoVan.VanPrefab(gm).IsEmpty() || !HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !HasPrefab(gm, RZ_EPuzzleProp.OBJECT) || !HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return 0;

		return 0.7;
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		m_Chain.GetSpots(result);
		if (m_Truck)
			result.Insert(m_Truck.GetOrigin());

		if (m_Load.m_Van)
			result.Insert(m_Load.m_Van.GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		// The lookout's note is outside a town the crews don't hold
		m_iSettlement = PickSettlementNotHeldBy(gm, 0, 80, RZ_Factions.Bit(RZ_EFaction.BANDITS), this);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		array<int> towns = {};
		if (!RZ_StepChain.PickTowns(gm, s.m_vCenter, 2, 700, 2500, 0, 95, towns))
			return false;

		RZ_Settlement stopTown = gm.GetSettlement(towns[0]);
		RZ_Settlement penTown = gm.GetSettlement(towns[1]);
		m_vStop = LandPointNear(stopTown.m_vCenter, stopTown.m_fRadius * 0.6, stopTown.m_fRadius + 60);
		m_vPen = LandPointNear(penTown.m_vCenter, penTown.m_fRadius + 60, penTown.m_fRadius + 200);
		m_sStop = stopTown.m_sName;
		m_sPen = penTown.m_sName;
		if (!RZ_RedHandEdges.HavenRefuge(gm, m_vPen, m_vRefuge, m_sRefuge))
			return false;

		m_Chain.SetMission(m_iId);
		RZ_ChainStep note = m_Chain.AddStep(gm, m_iSettlement, LandPointNear(s.m_vCenter, s.m_fRadius * 0.8, s.m_fRadius + 80), 6, 0);
		note.m_iClue = AddClue("A lookout's note", string.Format("Pencil on the back of a cigarette packet, under a stone: \"Red Hand truck, canvas back, tied down. Stopped here for water. Somebody in the back asked for some too. They didn't get any. Went on toward %1. I didn't follow. I'm sorry.\"", m_sStop));
		note.m_sReadLabel = "Read the note";
		note.m_iGuards = 2;
		m_Chain.AddStep(gm, towns[0], m_vStop, WATCH_RADIUS, WATCH_SECONDS);
		m_Chain.AddStep(gm, towns[1], m_vPen, 30, 0);

		m_iDoorTalk = AddClue("Through the boards", "A woman's voice, very calm: \"They said we'd work it off. Nobody said where, or for how long. There are children in here.\"");
		m_iDriverTalk = AddClue("The driver", "He's eating out of a tin with his fingers. \"Cargo. It's in the book, same as everything else. Somebody owed, now somebody's paying. You going to make it your business? Or do you want a cut?\"");

		m_iTruck = TRUCK_STOP;
		m_vTruckAt = m_vStop;
		m_vPos = note.m_vPos;
		m_sTitle = "Tied Down";
		m_sBrief = string.Format("People keep going missing on the roads. Twice now someone's seen a Red Hand truck with the canvas tied down tight, and nobody ties it down like that for tins. One of our lookouts saw it again outside %1 and left us a note there. Please follow it and find out where it goes. Don't let them see you. Then do what you can.",
			s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		m_Chain.OnClueRead(idx);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = string.Format("%1, %2", GetLocationName(gm), RZ_Util.DirectionText(from, m_vPos));
		if (m_iTruck == TRUCK_ROAD)
			text += string.Format("\nThe truck, heading for %1: %2", m_sPen, RZ_Util.DirectionText(from, m_vTruckAt));

		if (m_iChoice == CHOICE_FREE)
			text += string.Format("\nDrive them to: %1 (%2)", m_sRefuge, RZ_Util.DirectionText(from, m_vRefuge));

		return text;
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

			if (done == 0 && m_iTruck == TRUCK_STOP)
				NotifyParticipants(gm, m_sTitle, string.Format("The truck should still be outside %1, at grid %2. Get close enough to watch it, not close enough to be seen.", m_sStop, RZ_SettlementScanner.GridReference(m_vStop)));
			else if (done == 0)
				NotifyParticipants(gm, m_sTitle, string.Format("The truck has already left %1. Look for its tracks at grid %2.", m_sStop, RZ_SettlementScanner.GridReference(m_vStop)));
			else if (done == 1)
				m_bLeave = true;
			else if (done == 2 && m_iTruck != TRUCK_PEN)
				NotifyParticipants(gm, m_sTitle, "A shed with a new padlock and fresh tyre marks in the mud. No truck yet. It's on its way.");
		}

		UpdateTruck(gm, dt, ids, players);
		if (m_iTruck != TRUCK_PEN)
			return;

		m_vPos = m_vPen;
		if (!m_bPenShown && RZ_ContractHelp.AnyNear(players, m_vPen, SHOW_RADIUS))
			ShowPen(gm);

		if (m_iChoice == CHOICE_FREE)
		{
			UpdateDrive(gm, ids, players);
			return;
		}

		UpdateCutting(gm, dt);
	}

	// ================================================================ The truck
	//------------------------------------------------------------------------------------------------
	protected void UpdateTruck(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_iTruck == TRUCK_STOP)
		{
			if (!m_bStopShown && RZ_ContractHelp.AnyNear(players, m_vStop, SHOW_RADIUS))
				ShowStop(gm);

			if (m_Truck && !m_bSpotted && IsSeen(gm, ids, players, m_Truck.GetOrigin()))
				Spotted(gm);

			// It won't drive off with someone sitting in it
			if (m_bLeave && RZ_VehicleZone.DriverOf(m_Truck, ids, players) < 0)
				Depart(gm);

			return;
		}

		if (m_iTruck != TRUCK_ROAD)
			return;

		float dist = RZ_Util.DistXZ(m_vTruckAt, m_vPen);
		float speed = Math.Max(TRUCK_SPEED, RZ_Util.DistXZ(m_vStop, m_vPen) / MAX_DRIVE);
		float step = speed * dt;
		if (step < dist)
		{
			vector p = m_vTruckAt;
			p[0] = p[0] + (m_vPen[0] - p[0]) * step / dist;
			p[2] = p[2] + (m_vPen[2] - p[2]) * step / dist;
			m_vTruckAt = RZ_Util.GroundPos(p);
			return;
		}

		m_vTruckAt = m_vPen;
		m_iTruck = TRUCK_PEN;
		NotifyInterested(gm, m_sTitle, string.Format("The truck has stopped at a shed outside %1, grid %2. It isn't going any further.", m_sPen, RZ_SettlementScanner.GridReference(m_vPen)));
	}

	//------------------------------------------------------------------------------------------------
	//! Parked at the stop: no fuel, so nobody drives it off before the shed.
	protected void ShowStop(RZ_GameModeComponent gm)
	{
		m_bStopShown = true;
		m_Truck = RZ_Util.SpawnPrefab(RZ_CargoVan.VanPrefab(gm), m_vStop + Vector(0, 0.5, 0), RZ_Compass.Bearing(m_vStop, m_vPen));
		RZ_Util.SetVehicleFuel(m_Truck, 0);
		m_Crew = RZ_RedHandEdges.SpawnCrew(gm, RZ_CargoVan.CrewPrefab(gm), m_vStop, 4, 10);
		TrackProp(m_Crew);
	}

	//------------------------------------------------------------------------------------------------
	//! A player inside the crew's sight (★ Hunter: closer). The crew sits in the cab when there's
	//! no crew prefab, so it still sees.
	protected bool IsSeen(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players, vector at)
	{
		foreach (int i, IEntity p : players)
		{
			float radius = SPOT_RADIUS;
			if (HasBonus(gm, ids[i]))
				radius = BONUS_SPOT_RADIUS;

			if (RZ_Util.DistXZ(p.GetOrigin(), at) <= radius)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void Spotted(RZ_GameModeComponent gm)
	{
		m_bSpotted = true;
		m_bLeave = true;
		if (RZ_Util.CountAlive(m_Crew) > 0)
			RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, m_Crew, 120, true);

		RZ_ContractHelp.NotifyNear(gm, m_vStop, 200, m_sTitle, string.Format("\"Who's that?\" The crew has seen you. Doors slam and the truck pulls out fast, toward %1. Wherever it's going, they'll be expecting you.", m_sPen));
	}

	//------------------------------------------------------------------------------------------------
	//! It drives on to the shed as data: the parked truck and its crew go.
	protected void Depart(RZ_GameModeComponent gm)
	{
		if (!m_bSpotted)
			RZ_ContractHelp.NotifyNear(gm, m_vStop, 200, m_sTitle, string.Format("The crew finish their smokes, climb in and pull out toward %1. Somebody bangs on the inside of the canvas, twice. Then nothing. Follow it.", m_sPen));

		RZ_Util.DeleteUnit(m_Truck);
		RZ_Util.DeleteUnit(m_Crew);
		m_Truck = null;
		m_Crew = null;
		m_bLeave = false;
		m_vTruckAt = m_vStop;
		m_iTruck = TRUCK_ROAD;
	}

	// ================================================================ The shed
	//------------------------------------------------------------------------------------------------
	protected void ShowPen(RZ_GameModeComponent gm)
	{
		m_bPenShown = true;
		if (!m_Load.Park(gm, m_vPen, Math.RandomFloat(0, 360), m_bSpotted))
		{
			m_sFail = "Tied Down: the truck never came.";
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		TrackProp(m_Load.m_Guards);
		m_Load.m_Zone.SetZone(m_vRefuge, 30);

		// The Haven has seen where these trucks unload: the Cargo questline's way in for the Haven
		if (gm.GetQuestlines())
			gm.GetQuestlines().SetWorldFlag("cargo.truck_seen", 1);

		SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_Load.m_vDoor, 0, "", "Cut the lock|Radio the Haven", m_iDoorTalk, "Listen at the door");
		SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, LandPointNear(m_vPen, 3, 5), 1, "", "Ask for a cut", m_iDriverTalk, "Talk to the driver");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (m_iChoice != CHOICE_NONE)
			return;

		if (objIdx == 1)
		{
			OnDriver(gm, playerId);
			return;
		}

		if (objIdx != 0)
			return;

		if (option == 0)
		{
			string text = m_Load.StartCut(playerId);
			if (!text.IsEmpty())
				gm.NotifyPlayer(playerId, m_sTitle, text);

			return;
		}

		if (option != 1 || m_Load.m_iCutter > 0)
			return;

		m_iChoice = CHOICE_REPORT;
		m_fRewardScale = REPORT_SCALE;
		SetRival(RZ_EFaction.BANDITS, REPORT_INFAMY);
		m_aParticipants.Clear();
		m_aParticipants.Insert(playerId);
		RZ_LeaderJob.AddPeople(gm, m_vRefuge, 2);
		RZ_WorldFacts.Get().Memory().Lies().Test(gm, "distress.cut", "");
		gm.NotifyPlayer(playerId, m_sTitle, "You read the grid out on the Haven channel, very quietly. A pause. \"Thank you. Get away from there now.\" They come at first light and find two still in the shed. The rest have gone on, wherever the cargo goes.");
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	//! A cut: the Red Hand pays instead, if it deals with you at all (checked before anything is paid).
	protected void OnDriver(RZ_GameModeComponent gm, int playerId)
	{
		if (m_Load.m_iCutter > 0 || m_Load.m_bAlarm)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "The driver isn't talking to you now.");
			return;
		}

		if (RZ_Reputation.IsHunted(gm.GetTier(playerId, RZ_EFaction.BANDITS)))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"You?\" He laughs with his mouth full. \"Go away.\"");
			return;
		}

		m_iChoice = CHOICE_CUT;
		m_eGiver = RZ_EFaction.BANDITS;
		m_iFameReward = CUT_FAME;
		m_iFavorReward = CUT_FAVOR;
		SetRival(RZ_EFaction.SURVIVORS, CUT_INFAMY);
		m_bNoLeaderCredit = true;
		m_iLeaderId = -1;
		m_aParticipants.Clear();
		m_aParticipants.Insert(playerId);
		gm.NotifyPlayer(playerId, m_sTitle, "He looks at you for a long time, then counts tins into your hands. \"You saw a truck. There was nothing in it.\" Behind you, the shed has gone very quiet.");
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateCutting(RZ_GameModeComponent gm, float dt)
	{
		if (!m_Load.UpdateCut(dt))
			return;

		m_iChoice = CHOICE_FREE;
		SetRival(RZ_EFaction.BANDITS, FREE_INFAMY);
		m_Load.Fuel(0.4);
		RZ_ContractHelp.NotifyNear(gm, m_vPen, 80, m_sTitle, string.Format("The lock gives. Nine people come out of the dark, holding on to each other. Get them into the back of the truck and drive them to %1.", m_sRefuge));
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateDrive(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players)
	{
		if (m_Load.IsLost())
		{
			m_sFail = "The truck is finished. They scatter on foot into the dark, and you'll never know how many made it.";
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		m_vPos = m_Load.m_Van.GetOrigin();
		RZ_ContractHelp.AddNear(ids, players, m_vPos, 8, m_aParticipants);
		if (!m_Load.m_Zone.Check(ids, players))
			return;

		m_aParticipants.Clear();
		m_Load.m_Zone.GetCrew(ids, players, 30, m_aParticipants);
		if (m_Load.m_Zone.m_iDriver > 0 && !m_aParticipants.Contains(m_Load.m_Zone.m_iDriver))
			m_aParticipants.Insert(m_Load.m_Zone.m_iDriver);

		RZ_LeaderJob.AddPeople(gm, m_vRefuge, 3);
		RZ_WorldFacts.Get().Memory().Lies().Test(gm, "distress.cut", "");	// the freed name whoever sold a distress call
		NotifyParticipants(gm, m_sTitle, "The Haven's people come out with blankets before the engine is off. Somebody starts counting heads, and keeps having to start again.");
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	//! A cut is paid by the Red Hand: anyone it doesn't hunt.
	override bool IsEligible(RZ_GameModeComponent gm, int playerId)
	{
		if (m_iChoice == CHOICE_CUT && m_eState != RZ_EMissionState.ACTIVE)
			return !RZ_Reputation.IsHunted(gm.GetTier(playerId, RZ_EFaction.BANDITS));

		return super.IsEligible(gm, playerId);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (!m_sFail.IsEmpty())
			return m_sFail;

		return "Tied Down: the truck is gone, and the people in it.";
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Chain.Cleanup();
		RZ_Util.DeleteUnit(m_Crew);

		// The Haven keeps a truck that was driven in; otherwise it goes, unless someone sits in it
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		if (m_Truck && RZ_VehicleZone.DriverOf(m_Truck, ids, players) < 0)
			RZ_Util.DeleteUnit(m_Truck);

		m_Load.Cleanup();
	}
}
