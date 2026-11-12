//! Task Force Bastion's hard edges as contracts (docs/LORE.md, "Task Force Bastion", "The hard
//! edges"; docs/MISSIONS.md, "Task Force Bastion": #21 No Reply, #22 Struck Off).

//------------------------------------------------------------------------------------------------
//! Shared by the lore contracts.
class RZ_BastionLoreHelp
{
	//------------------------------------------------------------------------------------------------
	//! A Bastion post in a town no other mission is using: an outpost commander's seat, else a
	//! territory outpost, else the stronghold. False if Bastion has none.
	static bool PickPost(RZ_GameModeComponent gm, out vector pos, out string name)
	{
		int leaderId;
		return PickPostOf(gm, pos, name, leaderId);
	}

	//------------------------------------------------------------------------------------------------
	//! PickPost, and the leader whose seat it is (-1 for a territory outpost without one).
	static bool PickPostOf(RZ_GameModeComponent gm, out vector pos, out string name, out int leaderId)
	{
		array<vector> spots = {};
		array<string> names = {};
		array<int> seats = {};
		RZ_Leaders leaders = gm.GetLeaders();
		if (leaders)
		{
			array<RZ_Leader> list = {};
			leaders.GetFactionLeaders(RZ_EFaction.MILITARY, list);
			foreach (RZ_Leader l : list)
			{
				if (!l.IsStronghold())
					AddPost(gm, l.m_vPos, string.Format("%1's post in %2", l.m_sShort, l.m_sPlace), l.m_iId, spots, names, seats);
			}
		}

		RZ_Territory territory = gm.GetTerritory();
		if (spots.IsEmpty() && territory && territory.IsActive())
		{
			array<RZ_Holding> posts = {};
			territory.GetOutposts(RZ_EFaction.MILITARY, posts);
			foreach (RZ_Holding h : posts)
			{
				AddPost(gm, h.m_vPos, "the Bastion post in " + h.m_sName, -1, spots, names, seats);
			}
		}

		if (spots.IsEmpty() && leaders)
		{
			RZ_Leader top = leaders.GetStronghold(RZ_EFaction.MILITARY);
			if (top && top.IsPresent())
				AddPost(gm, top.m_vPos, "the stronghold in " + top.m_sPlace, top.m_iId, spots, names, seats);
		}

		leaderId = -1;
		if (spots.IsEmpty())
			return false;

		int pick = Math.RandomInt(0, spots.Count());
		pos = RZ_Mission.LandPointNear(spots[pick], 4, 9);
		name = names[pick];
		leaderId = seats[pick];
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void AddPost(RZ_GameModeComponent gm, vector p, string label, int leaderId, notnull array<vector> spots, notnull array<string> names, notnull array<int> seats)
	{
		int idx = gm.FindNearestSettlement(p);
		if (idx >= 0 && gm.GetMissions().IsSettlementBusy(idx))
			return;

		spots.Insert(p);
		names.Insert(label);
		seats.Insert(leaderId);
	}

	//------------------------------------------------------------------------------------------------
	static void Morale(RZ_GameModeComponent gm, float delta)
	{
		if (gm.GetFactionLife())
			gm.GetFactionLife().AddMorale(RZ_EFaction.MILITARY, delta);
	}
}

//------------------------------------------------------------------------------------------------
//! #21 No Reply: after the nightly call, a post's radio operator heard an answer on the Command
//! net, and it has come back on the hour ever since. He has one bearing from the post. Take a
//! second at a relay mast (★ Radio Op: on your own set, from wherever you stand), draw both
//! lines, find where they cross, then tell him what goes in the log. The source is never
//! Command and never anyone outside: a tape loop on a timer, a set someone inside the Zone is
//! keying, or nothing at all. Who is behind it is never said.
//! - **"No reply"**, as every night: paid in full.
//! - **What you found:** ×1.25 Fame, Bastion morale −4.
//! - **"Command answered"**: Bastion morale +6, 60% pay; 35% the stronghold asks for the
//!   transcript: no pay, +10 Bastion Infamy for whoever chose it.
class RZ_Mission_NoReply : RZ_PuzzleMission
{
	protected vector m_vPost;
	protected string m_sPost;
	protected vector m_vMast;
	protected vector m_vSource;
	protected int m_iSource;
	protected string m_sOperator;
	protected bool m_bPostShown;
	protected bool m_bMastShown;
	protected bool m_bSourceShown;
	protected bool m_bBriefed;
	protected bool m_bSecondBearing;
	protected bool m_bFound;
	protected bool m_bLogged;
	protected bool m_bChecked;
	protected int m_iChooser = -1;
	protected int m_iOperatorClue = -1;
	protected int m_iSourceClue = -1;
	protected int m_iPostLeader = -1;
	protected bool m_bLeadsOn;			// the tape at a post Await Relief can start from: the reel has the Day 3 burst too
	protected ref map<int, float> m_mLastFix = new map<int, float>();

	protected static const int OBJ_OPERATOR = 0;
	protected static const int OBJ_MAST = 1;
	protected static const int OBJ_SOURCE = 2;
	protected static const int SOURCE_TAPE = 0;
	protected static const int SOURCE_SET = 1;
	protected static const int SOURCE_NOTHING = 2;
	protected static const float FIX_GAP = 120;
	protected static const float FIX_MIN_FROM_POST = 400;
	protected static const float PROP_FIND = 30;
	protected static const float TRUTH_SCALE = 1.25;
	protected static const float ANSWERED_SCALE = 0.6;
	protected static const float CHECKED_CHANCE = 0.35;
	protected static const int CHECKED_INFAMY = 10;
	protected static const float TRUTH_MORALE = -4;
	protected static const float ANSWERED_MORALE = 6;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 22, 20);
		m_Req.Bonus("radioham");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.OBJECT) || !HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN) || !HasPrefab(gm, RZ_EPuzzleProp.RADIO_MAST))
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		if (!RZ_BastionLoreHelp.PickPostOf(gm, m_vPost, m_sPost, m_iPostLeader))
			return false;

		// The post whose log is Await Relief's story right now has enough going on
		if (RZ_Quest_AwaitRelief.IsAtPost(gm, m_iPostLeader))
			return false;

		m_iSettlement = gm.FindNearestSettlement(m_vPost);
		m_vSource = LandPointNear(RZ_Compass.PointAt(m_vPost, m_Rng.Range(0, 360), m_Rng.Range(800, 1800)), 0, 60);
		if (RZ_Util.IsWater(m_vSource))
			return false;

		// The mast sees the source at 55-125 degrees to the post's line, so the lines cross cleanly
		float side = 1;
		if (m_Rng.Chance(0.5))
			side = -1;

		float toMast = RZ_Compass.Bearing(m_vSource, m_vPost) + side * m_Rng.Range(55, 125);
		m_vMast = HighPointNear(RZ_Compass.PointAt(m_vSource, toMast, m_Rng.Range(600, 1300)), 0, 200, 12);
		if (RZ_Util.IsWater(m_vMast) || RZ_Util.DistXZ(m_vMast, m_vSource) < 400)
			return false;

		m_iSource = m_Rng.Int(0, 3);
		m_bLeadsOn = m_iSource == SOURCE_TAPE && RZ_Quest_AwaitRelief.CanFollowTape(gm, m_iPostLeader);
		m_sOperator = "Corporal " + RZ_NoteText.Surname(m_Rng);
		m_iOperatorClue = AddClue(m_sOperator, string.Format("He keeps his voice down. \"Last night. I read the numbers, said 'Bastion out', and somebody came back: 'Bastion, Command. Roger. Hold the line.' Then a carrier, then nothing. It's been back every hour since. Same words, same pause.\" He taps the log. \"I took a bearing off the loop. %1 from here. I need a second one. The relay mast at grid %2 has a direction loop. Take one there, draw both lines, find out what it is. Then come back and we'll decide what goes in the log.\"",
			RZ_Compass.BearingText(RZ_Compass.Bearing(m_vPost, m_vSource)), RZ_SettlementScanner.GridReference(m_vMast)));
		m_iSourceClue = AddClue("Where the lines cross", SourceText());

		m_vPos = m_vPost;
		m_sTitle = "No Reply";
		m_sBrief = string.Format("The radio operator at %1 wants a word, in person. Something about last night's call to Command, after the numbers were read and 'No reply' written in the log. Go and hear the operator out. Keep it off the open channel: a post that starts waiting again stops looking after itself.", m_sPost);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected string SourceText()
	{
		if (m_iSource == SOURCE_TAPE)
		{
			string tape = "A battery recorder in a plastic bag, wired to a dead man's manpack set and a kitchen timer. You play the tape back. The Command net on Day 2, the last clear message: \"Hold the line. Await relief.\" Somebody cut \"Bastion\" into it from another reel. The timer is set to the hour. No telling who set it, or when.";
			if (m_bLeadsOn)
				tape += " After the voice the reel runs on: a long burst of broken signal, no voice in it at all, then hiss.";

			return tape;
		}

		if (m_iSource == SOURCE_SET)
			return "A Bastion manpack set under a groundsheet, still warm. The handset hangs off a branch. Boot prints going away, not army issue. Four cigarette ends, one still burning. Whoever was answering as Command heard you coming.";

		return "Wet grass and a fence post. No set, no wire, no prints. Your receiver goes quiet as you get here: no carrier, no hiss. The hour comes and goes and nothing answers.";
	}

	//------------------------------------------------------------------------------------------------
	//! What the "truth" line in the log says.
	protected string TruthText()
	{
		if (m_iSource == SOURCE_TAPE)
			return "Signal traced to grid %1. Recording on a timer. Day 2 traffic. Not Command.";

		if (m_iSource == SOURCE_SET)
			return "Signal traced to grid %1. Our own set, someone else's hand. Not Command.";

		return "Signal traced to grid %1. Nothing found. Not Command.";
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		result.Insert(m_vPost);
		result.Insert(m_vMast);
		result.Insert(m_vSource);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = m_sPost + ", " + RZ_Util.DirectionText(from, m_vPost);
		if (!m_bBriefed || m_bFound)
			return text;

		text += string.Format("\nBearing from the post: %1. Relay mast (grid): %2", RZ_Compass.BearingText(RZ_Compass.Bearing(m_vPost, m_vSource)), RZ_SettlementScanner.GridReference(m_vMast));
		if (m_bSecondBearing)
			text += string.Format("\nBearing from the mast: %1", RZ_Compass.BearingText(RZ_Compass.Bearing(m_vMast, m_vSource)));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		if (idx == m_iOperatorClue && !m_bBriefed)
		{
			m_bBriefed = true;
			if (!m_bFound)
				m_vPos = m_vMast;
		}

		if (idx == m_iSourceClue)
			Found(gm);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bPostShown && RZ_ContractHelp.AnyNear(players, m_vPost, 300))
		{
			m_bPostShown = true;
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPost, OBJ_OPERATOR, "", "", m_iOperatorClue, "Talk to the radio operator");
			RZ_GuidanceSettings guide = gm.GetGuidanceSettings();
			if (guide && !guide.m_sTableRadioPrefab.IsEmpty())
				TrackProp(RZ_Util.SpawnPrefab(guide.m_sTableRadioPrefab, RZ_Util.GroundPos(RZ_Util.RandomPointInRing(m_vPost, 1, 1.5)) + Vector(0, 0.3, 0)));

			if (m_bFound)
				SetObjectLabels(OBJ_OPERATOR, "", LogLabels());
		}

		if (!m_bMastShown && RZ_ContractHelp.AnyNear(players, m_vMast, 300))
		{
			m_bMastShown = true;
			SpawnObject(gm, RZ_EPuzzleProp.RADIO_MAST, m_vMast, OBJ_MAST, "Take a bearing on the Command net", "");
			SpawnGuards(gm, m_vMast, 3, 15, 40);
		}

		if (!m_bSourceShown && m_iSource != SOURCE_NOTHING && RZ_ContractHelp.AnyNear(players, m_vSource, 300))
		{
			m_bSourceShown = true;
			SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vSource, OBJ_SOURCE, "", "", m_iSourceClue, "Look at the set");
			SpawnGuards(gm, m_vSource, 2, 20, 50);
		}

		if (m_bLogged)
			return;

		// ★ Radio Operators take their own bearings on the loop, from anywhere away from the post
		if (m_bBriefed && !m_bFound)
		{
			foreach (int i, IEntity p : players)
			{
				int playerId = ids[i];
				if (!IsInvolved(playerId) || !RZ_Radio.IsRadioOperator(gm, playerId))
					continue;

				vector at = p.GetOrigin();
				if (RZ_Util.DistXZ(at, m_vPost) < FIX_MIN_FROM_POST || RZ_Util.DistXZ(at, m_vSource) < RZ_HiddenStash.FIND_RADIUS)
					continue;

				float last;
				if (m_mLastFix.Find(playerId, last) && m_fClock - last < FIX_GAP)
					continue;

				m_mLastFix.Set(playerId, m_fClock);
				gm.NotifyPlayer(playerId, "On your set", string.Format("The loop comes in again: \"Bastion, Command. Roger. Hold the line.\" You turn until it nulls. Bearing %1 from where you stand (grid %2).",
					RZ_Compass.BearingText(RZ_Compass.Bearing(at, m_vSource)), RZ_SettlementScanner.GridReference(at)));
			}
		}

		float findAt = RZ_HiddenStash.FIND_RADIUS;
		if (m_iSource != SOURCE_NOTHING)
			findAt = PROP_FIND;

		if (!m_bFound && ReachedBy(ids, players, m_vSource, findAt))
			Found(gm);
	}

	//------------------------------------------------------------------------------------------------
	protected void Found(RZ_GameModeComponent gm)
	{
		if (m_bFound)
			return;

		m_bFound = true;
		m_vPos = m_vPost;
		SetObjectLabels(OBJ_OPERATOR, "", LogLabels());
		RZ_ContractHelp.NotifyNear(gm, m_vSource, 60, m_sTitle, SourceText() + string.Format("\nGo back to %1. %2 is waiting to write the log.", m_sPost, m_sOperator));
	}

	//------------------------------------------------------------------------------------------------
	protected string LogLabels()
	{
		return "Log it: \"No reply\"|Log what you found|Log it: \"Command answered\"";
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx != OBJ_MAST)
			return;

		AddParticipant(playerId);
		m_bSecondBearing = true;
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("You swing the loop until the voice is weakest. \"Bastion, Command. Roger. Hold the line.\" The same words, the same pause. Bearing %1 from the mast.",
			RZ_Compass.BearingText(RZ_Compass.Bearing(m_vMast, m_vSource))));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_OPERATOR || !m_bFound || m_bLogged || option < 0 || option > 2)
			return;

		if (!IsEligible(gm, playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 closes the log. \"Post business. Who sent you?\"", m_sOperator));
			return;
		}

		m_bLogged = true;
		m_iChooser = playerId;
		AddParticipant(playerId);
		SetObjectLabels(OBJ_OPERATOR, "", "");
		string grid = RZ_SettlementScanner.GridReference(m_vSource);
		if (option == 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("He writes it in the same hand as every other night: \"No reply.\" Then he tears last night's page out of the back of the log and feeds it to the stove. %1 chalks up one more day.", RZ_Util.Capitalize(m_sPost)));
		}
		else if (option == 1)
		{
			m_fRewardScale = TRUTH_SCALE;
			RZ_BastionLoreHelp.Morale(gm, TRUTH_MORALE);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("He writes it down and signs it: %1 The sergeant reads it twice. Nobody at the post says much tonight. The stronghold asks for a copy. Everything is signed for.", string.Format(TruthText(), grid)));
		}
		else
		{
			m_fRewardScale = ANSWERED_SCALE;
			RZ_BastionLoreHelp.Morale(gm, ANSWERED_MORALE);
			if (m_Rng.Chance(CHECKED_CHANCE))
			{
				m_bChecked = true;
				m_fRewardScale = 0;
				gm.NotifyPlayer(playerId, m_sTitle, "He writes \"Command answered. Hold the line.\" and underlines it. By evening the whole post has heard. Next morning the stronghold asks for the transcript. There isn't one, and they know whose idea it was.");
			}
			else
			{
				gm.NotifyPlayer(playerId, m_sTitle, "He writes \"Command answered. Hold the line.\" and underlines it. By evening the whole post has heard. Somebody laughs for the first time in a week. He doesn't look at you when he pays.");
			}
		}

		// The burst at the end of the reel is the Await Relief questline (Missions/Questlines/Bastion/)
		if (m_bLeadsOn && RZ_Quest_AwaitRelief.CanFollowTape(gm, m_iPostLeader))
		{
			RZ_Leader cmd = RZ_BastionQuest.Leader(gm, m_iPostLeader);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("Before you go, %1 plays the end of the reel again: the long burst after the voice. \"That one isn't on the hour. It comes after the nightly call, every night. I've been crossing it out.\" %2 will want a word about it. In person.", m_sOperator, cmd.m_sShort));
			RZ_Quest_AwaitRelief.OnTapeFound(gm, m_iPostLeader, m_sOperator);
		}

		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
		if (!m_bChecked || playerId != m_iChooser)
			return;

		gm.AddInfamy(playerId, RZ_EFaction.MILITARY, CHECKED_INFAMY);
		gm.NotifyPlayer(playerId, RZ_Factions.GetName(RZ_EFaction.MILITARY), string.Format("A false entry in a post log. +%1 Infamy.", CHECKED_INFAMY));
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return string.Format("No Reply: the hour passed. At %1 the log says what it always says.", m_sPost);
	}
}

//------------------------------------------------------------------------------------------------
//! #22 Struck Off (Hunt the Deserter merged in; story key `struck`): a soldier on a post's roster as
//! killed in action has been seen alive. His pack by the post, a Haven trader in a nearby town, or
//! the Deserter wanderer (asked about Bastion) leads to where he sleeps. The truth behind it, kept per
//! name: he stole and ran, he was written off (he never took a tin), the quartermaster's gap (the
//! rations were sold), or a Haven door (he's hiding at a house). Talking to him tells it to the room;
//! the post's stores book, read at night, shows the last two. Then, at his kit:
//! - **Back on duty:** persuade him (Trust with Bastion, difficulty 2, ★ Soldier +1). The KIA is
//!   rubbed out; he walks with you to the post. 80% pay, Bastion morale +3 and strength +1.
//! - **Arrest him:** he walks with you to the post and the stockade. Full pay.
//! - **The Haven:** walk him to a safehouse (it gains one). The Haven pays instead (12 Fame,
//!   15 Favor). A lie to Bastion, found out by a Bastion-Known player at that house: +15 Infamy.
//! - **Bastion's way:** he stays struck off. Carry his tag to the empty nail at the post: 60%.
//! - **Let him go:** no pay; he walks off.
//! - **The stores book** (written off or the gap, once read): report it from the board for +5
//!   Bastion Fame, the chooser only.
//! If he dies any other way, his tag is all that's left: the same 60% at the nail.
//! In ten days only one soldier walked out (docs/LORE.md, Private Fiala on Day 7), so his KIA is
//! dated from Day 11 to two days ago, the day the trader saw him: never before Day 13.
class RZ_Mission_StruckOff : RZ_PuzzleMission
{
	protected ref RZ_Escort m_Escort = new RZ_Escort();
	protected ref RZ_StepChain m_Chain = new RZ_StepChain();
	protected ref RZ_Ways m_Ways = new RZ_Ways("Struck Off");
	protected string m_sTruth;
	protected bool m_bBookRead;
	protected bool m_bReported;
	protected int m_iBookClue = -1;
	protected int m_iPackClue = -1;
	protected vector m_vPost;
	protected string m_sPost;
	protected vector m_vWitness;
	protected vector m_vHide;
	protected string m_sTown;
	protected int m_iHouse = -1;
	protected vector m_vHouse;
	protected string m_sHouse;
	protected string m_sName;
	protected string m_sSurname;
	protected int m_iMode;
	protected bool m_bPostShown;
	protected bool m_bWitnessShown;
	protected bool m_bHideShown;
	protected bool m_bTold;
	protected bool m_bFound;
	protected IEntity m_Tag;
	protected bool m_bTagSpawned;
	protected vector m_vTagLast;
	protected int m_iTagSpawns;
	protected int m_iWitnessClue = -1;
	protected int m_iTalk = -1;
	protected int m_iRosterClue = -1;
	protected ref array<int> m_aTracked = {};	// ★ Hunter and ★ Soldier: told their shortcut once

	protected static const int OBJ_BOARD = 0;
	protected static const int OBJ_KIT = 1;
	protected static const int MODE_NONE = 0;
	protected static const int MODE_ROSTER = 1;
	protected static const int MODE_ARREST = 2;
	protected static const int MODE_HAVEN = 3;
	protected static const int MODE_TAG = 4;
	protected static const float ROSTER_SCALE = 0.8;
	protected static const float TAG_SCALE = 0.6;
	protected static const float ROSTER_MORALE = 3;
	protected static const int HAVEN_FAME = 12;
	protected static const int HAVEN_FAVOR = 15;
	protected static const int FOUND_OUT_INFAMY = 15;
	protected static const int MODE_GONE = 5;
	protected static const int REPORT_FAME = 5;
	protected static const int MAX_TAG_SPAWNS = 4;
	protected static const int FIRST_KIA_DAY = 11;	// after the first ten days (Fiala, Day 7, was the only one)
	protected static const int MIN_DAY = 13;		// the trader saw him two days ago, on Day 11 or later
	protected static const int TRADE_FOOD = 4;
	protected static const int PRESS_INFAMY = 3;
	protected static const int BOOK_LOYALTY = 5;

	//------------------------------------------------------------------------------------------------
	void RZ_Mission_StruckOff()
	{
		m_Story = new RZ_MissionStory("struck");
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.TRUSTED, 35, 30);
		m_Req.m_iMinDay = MIN_DAY;
		m_fTimeLeft = 4200;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!RZ_Escort.HasRoom(gm) || gm.GetMissionItemPrefab().IsEmpty() || !HasPrefab(gm, RZ_EPuzzleProp.OBJECT) || !HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return 0;

		if (RZ_ContractHelp.CountSafehouses(gm) == 0)
			return 0;

		// A post that cooked its count, or orders read on the air, wants the numbers right (x1.2 each,
		// inside the one weight clamp)
		float weight = 1.2;
		if (RZ_WorldFacts.Get().Memory().NewestRecord("errand.the_count", 3))
			weight *= 1.2;

		if (gm.GetQuestlines() && gm.GetQuestlines().HasWorldFlag("orders.read"))
			weight *= 1.2;

		return weight;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		if (!RZ_BastionLoreHelp.PickPost(gm, m_vPost, m_sPost))
			return false;

		m_iSettlement = PickSettlementAway(gm, m_vPost, 900, 3000, 0, 90, gm.FindNearestSettlement(m_vPost));
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vHide = LandPointNear(s.m_vCenter, s.m_fRadius + 120, s.m_fRadius + 400);
		m_iHouse = RZ_LeaderJob.SafehouseNear(gm, m_vHide, 400, 6000, -1);
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, m_iHouse);
		if (!h)
			return false;

		m_vHouse = h.m_vPos;
		m_sHouse = h.m_sName;
		m_sTown = s.m_sName;
		m_vWitness = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.5);
		m_sSurname = RZ_NoteText.Surname(m_Rng);
		m_sName = "Private " + m_sSurname;
		// Struck off from Day 11 to the day the trader saw him, two days ago (Int's max is exclusive)
		int today = MIN_DAY;
		if (gm.GetFacts())
			today = Math.Max(MIN_DAY, gm.GetFacts().Day());

		string day = RZ_NoteText.Day(m_Rng.Int(FIRST_KIA_DAY, today - 1));

		// The truth, kept per name: a Haven door needs a house within 3 km of where he sleeps
		array<string> values = {"stole", "written_off", "qm_gap", "haven_door"};
		array<float> weights = {0.35, 0.3, 0.2, 0};
		if (RZ_Util.DistXZ(m_vHouse, m_vHide) <= 3000)
			weights[3] = 0.15;

		m_sTruth = m_Story.Truth(m_sName, values, weights, this);
		if (m_sTruth == "haven_door")
			m_vHide = LandPointNear(m_vHouse, 30, 90);

		// The old trail: his pack by the post, pointing on to the town
		m_Chain.SetMission(m_iId);
		int postTown = gm.FindNearestSettlement(m_vPost);
		RZ_ChainStep pack = m_Chain.AddStep(gm, postTown, LandPointNear(m_vPost, 250, 500), 6, 0);
		pack.m_iClue = AddClue("A dropped pack", string.Format("Army issue, the name tape cut off. Two empty ration tins, and a map with %1 circled in pencil. Under the circle: grid %2.", s.m_sName, RZ_SettlementScanner.GridReference(m_vHide)));
		pack.m_sReadLabel = "Go through the pack";
		m_iPackClue = pack.m_iClue;

		m_iWitnessClue = AddClue("A Haven trader", string.Format("\"Your lot's uniform, patch cut off. Paid me in rifle rounds for two tins of fish and asked if the Haven takes soldiers.\" She points past the last houses. \"He sleeps rough out there, grid %1. He jumps at every sound. Don't tell him it was me.\"",
			RZ_SettlementScanner.GridReference(m_vHide)));
		m_iTalk = AddClue(m_sName, string.Format("He doesn't reach for the rifle. \"%1. They wrote me up, didn't they? Killed in action, %2.\" A short laugh. \"I was. Most of me. I walked away from what was left of the section and nobody came looking. Until you.\"", m_sSurname, day));
		m_iRosterClue = AddClue("The roster", string.Format("Grease pencil on the board: \"%1. KIA %2. Body not recovered.\" On the wall behind the radio, a row of nails with a tag on each. The one under his name is empty.", m_sName, day));
		m_iBookClue = AddClue("The stores book", BookText());

		m_Escort.m_sName = m_sName;
		m_Escort.m_sPrefab = RZ_Escort.PrefabFor(gm, 2);
		m_Escort.m_bHide = false;
		m_Escort.SetTalk(m_iId, OBJ_KIT, ChoiceLabels(), m_iTalk, "Talk to him");

		m_vPos = m_vWitness;
		m_sTitle = "Struck Off";
		m_sBrief = string.Format("%1 walked off post at %2 with a week's rations, they say. He's on the board as killed in action. A Haven trader in %3 says she sold him fish two days ago. Bring him back, or bring back his tag.", m_sName, m_sPost, m_sTown);
		return m_Escort.Begin(gm, m_vHide);
	}

	//------------------------------------------------------------------------------------------------
	protected string ChoiceLabels()
	{
		string labels = "[Trusted · Bastion] Talk him back|Arrest him|Walk him to the Haven|Settle it Bastion's way|Let him go";
		if (m_sTruth == "haven_door")
			labels += "|[Feared] Press him, in front of the kids|Leave the kids four tins, for him [4 food]";

		return labels;
	}

	//------------------------------------------------------------------------------------------------
	//! The last roll in Bastion's coda (the majors): the struck-off men brought home this season answer
	//! "Here", and for each one still out, nobody answers. "" if none.
	static string LastRoll()
	{
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		if (!facts)
			return "";

		int home = facts.Flag("struck.home");
		int away = facts.Flag("struck.out");
		if (home <= 0 && away <= 0)
			return "";

		string line = "";
		if (home == 1)
			line = "One struck-off man, walked home this season, answers \"Here\".";
		else if (home > 1)
			line = string.Format("%1 struck-off men, walked home this season, answer \"Here\".", home);

		if (away > 0)
		{
			if (!line.IsEmpty())
				line += " ";

			line += string.Format("For %1 still out there, nobody answers, and Sedlak reads the next name.", away);
		}

		return line;
	}

	//------------------------------------------------------------------------------------------------
	//! The post's stores book: what it shows depends on the truth.
	protected string BookText()
	{
		if (m_sTruth == "written_off")
			return "The week he walked, the ration count is short by the same number it was short the week before. And the week before that. He never took a tin. Somebody needed a name to put it against.";

		if (m_sTruth == "qm_gap")
			return "A neat column of rations \"lost in transit\", in the quartermaster's hand, dated the night before he walked. The pages after it are in a different pen.";

		return "The ration count drops by a week's worth on the day he walked. It's all there, signed for and missing.";
	}

	//------------------------------------------------------------------------------------------------
	//! What he says, by the truth (told to the room).
	protected string HisSide()
	{
		if (m_sTruth == "written_off")
			return "Rations? I never took a tin. Check the book. They needed the numbers to come out.";

		if (m_sTruth == "qm_gap")
			return "The quartermaster sold half the stores the night before. Me walking off was very handy for him.";

		if (m_sTruth == "haven_door")
			return "The tins went to two kids at the house. Their mum was in my section. Somebody had to.";

		return "I took them. I'd take them again. I'm not going back to that post.";
	}

	//------------------------------------------------------------------------------------------------
	//! The Deserter wanderer, asked about Bastion while this runs, knows where he sleeps. "" if
	//! there's nothing to tell (no Struck Off running, or he was already found).
	static string DeserterTip(RZ_GameModeComponent gm)
	{
		if (!gm.GetMissions())
			return "";

		foreach (RZ_Mission m : gm.GetMissions().GetActive())
		{
			RZ_Mission_StruckOff job = RZ_Mission_StruckOff.Cast(m);
			if (job && job.m_eState == RZ_EMissionState.ACTIVE && !job.m_bFound)
				return job.Tip();
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected string Tip()
	{
		m_bTold = true;
		m_vPos = m_vHide;
		return string.Format("\"%1? Struck off, is it? He's alive. Sleeps rough outside %2, grid %3. He was in my section.\" A long look. \"They'll send someone either way. Might as well be you.\"",
			m_sSurname, m_sTown, RZ_SettlementScanner.GridReference(m_vHide));
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		result.Insert(m_vPost);
		result.Insert(m_vWitness);
		result.Insert(m_vHide);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = m_sTown + ", " + RZ_Util.DirectionText(from, m_vWitness);
		if (m_bTold && m_iMode == MODE_NONE)
			text += "\nWhere he sleeps (grid): " + RZ_SettlementScanner.GridReference(m_vHide);

		if (m_iMode == MODE_HAVEN)
			text += "\nThe Haven safehouse in " + m_sHouse + ": " + RZ_Util.DirectionText(from, m_vHouse);
		else if (m_iMode != MODE_NONE)
			text += string.Format("\n%1: %2", RZ_Util.Capitalize(m_sPost), RZ_Util.DirectionText(from, m_vPost));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! The stores book is only read when nobody's watching: after dark.
	override string GetClueText(RZ_GameModeComponent gm, int playerId, int idx)
	{
		if (idx == m_iBookClue && !RZ_GameTime.IsNight())
			return "The quartermaster sits right beside it, and he's looking at you. Come back after dark.";

		return super.GetClueText(gm, playerId, idx);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		m_Chain.OnClueRead(idx);
		if (idx == m_iTalk)
		{
			m_Story.m_Deciding.Step(gm, playerId);
			m_Story.Reveal(gm, this, m_sName, playerId, "talk", HisSide());
		}

		// The stores book shows it only to someone reading by night
		if (idx == m_iBookClue && RZ_GameTime.IsNight() && (m_sTruth == "written_off" || m_sTruth == "qm_gap") && !m_bBookRead)
		{
			m_bBookRead = true;
			m_Story.Reveal(gm, this, m_sName, playerId, "book", "The stores book doesn't add up the way the roster does.");
			SetObjectLabels(OBJ_BOARD, "Hang his tag on the nail", "Report the stores book to the duty officer");
		}

		if ((idx == m_iWitnessClue || idx == m_iPackClue) && !m_bTold)
		{
			m_bTold = true;
			if (m_iMode == MODE_NONE)
				m_vPos = m_vHide;
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		Shortcuts(gm, ids, players);
		if (!m_bPostShown && RZ_ContractHelp.AnyNear(players, m_vPost, 300))
		{
			m_bPostShown = true;
			SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vPost, OBJ_BOARD, "Hang his tag on the nail", "", m_iRosterClue, "Read the roster");
			SpawnClue(gm, RZ_EPuzzleProp.CLUE, LandPointNear(m_vPost, 3, 6), m_iBookClue, "Read the stores book");
		}

		if (!m_Chain.IsDone() && !m_bFound)
			m_Chain.Update(gm, dt, ids, players, m_aParticipants);

		if (!m_bWitnessShown && RZ_ContractHelp.AnyNear(players, m_vWitness, 300))
		{
			m_bWitnessShown = true;
			SpawnClue(gm, RZ_EPuzzleProp.CIVILIAN, m_vWitness, m_iWitnessClue, "Talk to the trader");
		}

		if (!m_bHideShown && m_iMode == MODE_NONE && RZ_ContractHelp.AnyNear(players, m_vHide, 300))
		{
			m_bHideShown = true;
			SpawnObject(gm, RZ_EPuzzleProp.OBJECT, RZ_Util.RandomPointInRing(m_vHide, 2, 4), OBJ_KIT, "", ChoiceLabels(), m_iTalk, "Talk to him");
			SpawnGuards(gm, m_vHide, 2, 40, 80);
		}

		if (m_iMode == MODE_TAG)
		{
			UpdateTag(gm);
			return;
		}

		m_Escort.Update(gm, dt, ids, players);
		if (m_iMode != MODE_NONE)
			m_vPos = m_Escort.m_vPos;

		if (!m_bFound && ReachedBy(ids, players, m_Escort.m_vPos, 25))
		{
			m_bFound = true;
			m_bTold = true;
			m_vPos = m_Escort.m_vPos;
			RZ_ContractHelp.NotifyNear(gm, m_Escort.m_vPos, 40, m_sTitle, string.Format("A man in Bastion fatigues, the name tape cut off, sitting by his kit. It's %1. Talk to him.", m_sName));
		}

		// Dead some other way: the roster was right after all
		if (m_Escort.IsDead())
		{
			BecomeTag(gm, m_Escort.m_vPos, "He's dead. The roster was only early. His tag is where he fell: take it to the empty nail at " + m_sPost + ".");
			return;
		}

		if (m_iMode == MODE_NONE)
			return;

		RZ_ContractHelp.AddNear(ids, players, m_Escort.m_vPos, 100, m_aParticipants);
		vector dest = m_vPost;
		if (m_iMode == MODE_HAVEN)
			dest = m_vHouse;

		if (m_Escort.IsNear(dest, 25))
		{
			m_Escort.Arrive();
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! ★ Hunter: his tracks from the pack lead straight to where he sleeps. ★ Soldier: within 200 m of
	//! it, he shows himself to a soldier. Each once per player.
	protected void Shortcuts(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players)
	{
		if (m_bFound || m_iMode != MODE_NONE)
			return;

		foreach (int i, IEntity p : players)
		{
			int pid = ids[i];
			if (m_aTracked.Contains(pid))
				continue;

			float toHide = RZ_Util.DistXZ(p.GetOrigin(), m_vHide);
			if (toHide <= 200 && RZ_StoryKit.Read(gm, this, pid, "soldier", string.Format("A man steps out of the trees with his hands where you can see them. \"You walk like a soldier. %1. You came for me.\"", m_sSurname)))
			{
				m_aTracked.Insert(pid);
				m_bTold = true;
				m_vPos = m_vHide;
				continue;
			}

			if (RZ_Util.DistXZ(p.GetOrigin(), m_vPost) <= 600 && RZ_StoryKit.Read(gm, this, pid, "hunter", string.Format("Army boots, one heel worn down, a week old and heading out. They don't wander: grid %1, and he's sleeping rough.", RZ_SettlementScanner.GridReference(m_vHide))))
			{
				m_aTracked.Insert(pid);
				m_bTold = true;
				m_vPos = m_vHide;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The tag: put back where it was last seen if its carrier left (a few times at most).
	protected void UpdateTag(RZ_GameModeComponent gm)
	{
		if (m_Tag)
		{
			IEntity root = m_Tag.GetRootParent();
			if (root)
				m_vTagLast = root.GetOrigin();
			else
				m_vTagLast = m_Tag.GetOrigin();

			int carrier = RZ_Util.GetCarrierPlayerId(m_Tag);
			if (carrier > 0)
				AddParticipant(carrier);

			return;
		}

		if (!m_bTagSpawned || m_iTagSpawns >= MAX_TAG_SPAWNS)
			return;

		SpawnTag(gm, m_vTagLast);
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnTag(RZ_GameModeComponent gm, vector pos)
	{
		m_bTagSpawned = true;
		m_iTagSpawns++;
		m_vTagLast = LandPointNear(pos, 0, 2);
		m_Tag = RZ_Util.SpawnPrefab(gm.GetMissionItemPrefab(), RZ_Util.GroundPos(m_vTagLast) + Vector(0, 0.3, 0));
		RZ_MissionItemComponent.Tag(m_Tag, m_iId);
		TrackProp(m_Tag);
	}

	//------------------------------------------------------------------------------------------------
	//! From here only his tag matters: Bastion pays 60% at the nail, whoever chose what before.
	protected void BecomeTag(RZ_GameModeComponent gm, vector at, string text)
	{
		m_iMode = MODE_TAG;
		m_eGiver = RZ_EFaction.MILITARY;
		m_iFameReward = 35;
		m_iFavorReward = 30;
		m_bHasRival = false;
		m_fRewardScale = TAG_SCALE;
		m_vPos = m_vPost;
		m_Escort.Cleanup();
		SetObjectLabels(OBJ_KIT, "", "");
		SpawnTag(gm, at);
		RZ_ContractHelp.NotifyNear(gm, at, 300, m_sTitle, text);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx == OBJ_BOARD)
		{
			OnReport(gm, playerId);
			return;
		}

		if (objIdx != OBJ_KIT || m_iMode != MODE_NONE || option < 0 || option > 6)
			return;

		if (option > 4 && m_sTruth != "haven_door")
			return;

		if (!IsEligible(gm, playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "He looks past you, at the road. \"You're not from the post. Who sent you?\"");
			return;
		}

		if (!RZ_StoryKit.MayDecide(gm, this, playerId))
			return;

		// Talking him back is a persuade, once
		if (option == 0)
		{
			if (m_Ways.TryTrust(gm, playerId, "deserter", RZ_EFaction.MILITARY, 2, "soldier") != RZ_ECheck.PASS)
			{
				gm.NotifyPlayer(playerId, m_sTitle, "\"Back on duty? With them?\" He shakes his head. He won't hear it from you. Arrest him, or something else.");
				return;
			}
		}

		// [Feared] Press him, at a Haven door: a threat the mission writes; he comes, under arrest
		if (option == 5)
		{
			RZ_StoryKit.Dirty(gm, this, playerId);
			if (m_Ways.TryFear(gm, playerId, "deserter." + m_sSurname, RZ_EFaction.SURVIVORS, RZ_EFaction.MILITARY, 2, false, PRESS_INFAMY, "excon,raider") == RZ_ECheck.REFUSED)
			{
				gm.NotifyPlayer(playerId, m_sTitle, "The kids stand in front of him. He isn't going anywhere with you, not like this.");
				return;
			}

			option = 1;
		}

		// Leave the kids four tins: the rations stay with them, and he goes back with you
		if (option == 6)
		{
			IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (!me || RZ_HavenQuest.CountFood(gm, me) < TRADE_FOOD || !RZ_StoryStats.TakeFood(gm, me, TRADE_FOOD))
			{
				gm.NotifyPlayer(playerId, m_sTitle, "Four tins. For them, not me. Then I'll come.");
				return;
			}

			if (gm.GetSafehouses())
				gm.GetSafehouses().AddFood(m_iHouse, 2);

			m_Escort.Follow(playerId);
			m_Escort.Go();
			m_bFound = true;
			m_bTold = true;
			AddParticipant(playerId);
			SetObjectLabels(OBJ_KIT, "", "");
			m_iMode = MODE_ROSTER;
			m_fRewardScale = ROSTER_SCALE;
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("He watches the tins go into the kids' cupboard and picks up his kit. \"Right. Back on duty.\" Walk him to %1.", m_sPost));
			return;
		}

		if (option == 4)
		{
			m_bFound = true;
			m_iMode = MODE_GONE;
			SetObjectLabels(OBJ_KIT, "", "");
			array<int> chooser = {playerId};
			RZ_StoryKit.Record(this, "fate", m_sName, "gone", chooser);
			// He takes to the roads as the Deserter, if there's a free place on them
			if (gm.GetWanderers())
				gm.GetWanderers().AddWithRole(RZ_EWandererRole.DESERTER, m_sSurname, m_vHide);

			if (gm.GetFacts())
				gm.GetFacts().SetFlag("struck.out", gm.GetFacts().Flag("struck.out") + 1);

			gm.NotifyPlayer(playerId, m_sTitle, "You step aside. He looks at you a long moment, then he's gone into the trees. On the board he's still dead. Nobody pays for this.");
			m_Escort.Cleanup();
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		m_bFound = true;
		m_bTold = true;
		AddParticipant(playerId);
		SetObjectLabels(OBJ_KIT, "", "");
		if (option == 3)
		{
			m_aParticipants.Clear();
			m_aParticipants.Insert(playerId);
			BecomeTag(gm, m_Escort.m_vPos, string.Format("He sees it in your face before you move. \"Yeah,\" he says. \"Thought so.\" Afterwards you take the tag from round his neck. Hang it on the empty nail at %1. The numbers add up now.", m_sPost));
			return;
		}

		m_Escort.Follow(playerId);
		m_Escort.Go();
		if (option == 0)
		{
			m_iMode = MODE_ROSTER;
			m_fRewardScale = ROSTER_SCALE;
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("\"Back on duty. Just like that.\" He shoulders his kit. \"They'll have to rub out the KIA.\" Walk him to %1.", m_sPost));
			return;
		}

		if (option == 1)
		{
			m_iMode = MODE_ARREST;
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("He hands you the rifle, stock first, and puts his hands on his head without being told. Walk him to %1 and the stockade.", m_sPost));
			return;
		}

		m_iMode = MODE_HAVEN;
		m_eGiver = RZ_EFaction.SURVIVORS;
		m_iFameReward = HAVEN_FAME;
		m_iFavorReward = HAVEN_FAVOR;

		gm.NotifyPlayer(playerId, m_sTitle, string.Format("He stares at you. \"You'd do that?\" Then he's up, kit on, before you can change your mind. The Haven safehouse in %1 (%2). On Bastion's books he's still dead.", m_sHouse, RZ_Util.DirectionText(m_Escort.m_vPos, m_vHouse)));
	}

	//------------------------------------------------------------------------------------------------
	//! The stores book, reported from the board: the post's quartermaster answers for it. +5 Bastion
	//! Fame for the chooser only; the job goes on.
	protected void OnReport(RZ_GameModeComponent gm, int playerId)
	{
		if (!m_bBookRead || m_bReported || !RZ_StoryKit.MayDecide(gm, this, playerId))
			return;

		m_bReported = true;
		SetObjectLabels(OBJ_BOARD, "Hang his tag on the nail", "");
		gm.AddFame(playerId, RZ_EFaction.MILITARY, REPORT_FAME, false, RZ_RepFlag.SMALL);
		// The post's commander answers for his book: his loyalty, and he remembers who reported it
		if (m_sTruth == "written_off" || m_sTruth == "qm_gap")
		{
			RZ_Leader commander = RZ_StoryKit.Remember(gm, RZ_EFaction.MILITARY, m_vPost, 1500, playerId, RZ_Leaders.MEM_CROSSED);
			RZ_StoryKit.Loyalty(gm, commander, -BOOK_LOYALTY);
		}
		array<int> chooser = {playerId};
		RZ_StoryKit.Record(this, "book", m_sPost, m_sTruth, chooser);
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("The duty officer reads the page twice and closes the book on his thumb. \"Leave it with me.\" +%1 Bastion Fame.", REPORT_FAME));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx != OBJ_BOARD)
			return;

		if (m_iMode != MODE_TAG || !RZ_ContractHelp.CarriesItemOf(user, m_iId, true))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "The nail under his name is empty. You've nothing to hang on it.");
			return;
		}

		m_aParticipants.Clear();
		m_aParticipants.Insert(playerId);
		gm.NotifyPlayer(playerId, m_sTitle, "You hang the tag on the nail under his name. The sergeant looks at it, then at you, and goes back to the board. The roster balances.");
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_StoryKit.Record(this, "fate", m_sName, ModeWord(), m_aParticipants, FateLine(), "radio");
		if (m_iMode == MODE_ROSTER)
		{
			RZ_BastionLoreHelp.Morale(gm, ROSTER_MORALE);
			if (gm.GetFacts())
				gm.GetFacts().SetFlag("struck.home", gm.GetFacts().Flag("struck.home") + 1);

			if (gm.GetTerritory() && gm.GetTerritory().IsActive())
				gm.GetTerritory().AddStrength(RZ_EFaction.MILITARY, 1);

			NotifyParticipants(gm, m_sTitle, string.Format("The sergeant rubs out \"KIA\" with his thumb and writes it again: \"%1. Returned to duty.\" Nobody asks where he's been.", m_sName));
			return;
		}

		if (m_iMode == MODE_ARREST)
		{
			NotifyParticipants(gm, m_sTitle, "They take his belt and bootlaces and put him in the stockade. On the board, \"KIA\" becomes \"Absent without leave. Held.\" The numbers add up again.");
			return;
		}

		if (m_iMode != MODE_HAVEN)
			return;

		gm.GetSafehouses().AddRescued(m_vHouse, 1, m_aParticipants);
		NotifyParticipants(gm, m_sTitle, "The Haven asks him nothing but his first name. On a board in a Bastion post, he's still dead.");

		// Bastion's books say dead: a lie, found out when someone Bastion knows comes to that house
		string found = string.Format("Someone from %1 saw %2 at the Haven house in %3. They don't change the roster. They change their mind about you.", m_sPost, m_sName, m_sHouse);
		foreach (int playerId : m_aParticipants)
		{
			RZ_WorldFacts.Get().Memory().Lies().Tell(gm, playerId, "struck.haven", m_sName, RZ_EFaction.MILITARY, "", m_vHouse, RZ_ELieTest.VISIT, FOUND_OUT_INFAMY, found);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string ModeWord()
	{
		if (m_iMode == MODE_ROSTER)
			return "roster";

		if (m_iMode == MODE_ARREST)
			return "stockade";

		if (m_iMode == MODE_HAVEN)
			return "haven";

		if (m_iMode == MODE_TAG)
			return "nail";

		return "gone";
	}

	//------------------------------------------------------------------------------------------------
	protected string FateLine()
	{
		if (m_iMode == MODE_ROSTER)
			return string.Format("At the last roll at %1, %2 answers \"Here\".", m_sPost, m_sName);

		if (m_iMode == MODE_TAG)
			return string.Format("His tag hangs on the nail at %1. Nobody answers for %2.", m_sPost, m_sName);

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailed(RZ_GameModeComponent gm)
	{
		if (m_iMode == MODE_GONE)
			return;

		// Time up: a hunter squad finds him, as data, and his tag goes on the nail
		RZ_StoryKit.Record(this, "fate", m_sName, "nail", m_aParticipants, string.Format("Struck off twice: %1's tag hangs at %2.", m_sName, m_sPost), "board");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (m_iMode == MODE_GONE)
			return string.Format("Struck Off: %1 walked away, and the roster still says he's dead.", m_sName);

		return string.Format("Struck Off: a hunter squad found %1 first. Struck off twice.", m_sName);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Escort.Cleanup();
		m_Chain.Cleanup();
	}
}
