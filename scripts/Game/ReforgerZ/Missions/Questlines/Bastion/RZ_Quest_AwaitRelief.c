//! Task Force Bastion's questlines (docs/QUESTLINES.md, "Task Force Bastion"; docs/MISSIONS.md,
//! "Questlines"): Await Relief (this file), The Census (RZ_Quest_Census.c) and The Last Pilot
//! (RZ_Quest_LastPilot.c). What the three share is here too: RZ_BastionQuest (lookups) and
//! RZ_QuestCarry (an item carried from one person to another).

//------------------------------------------------------------------------------------------------
//! Lookups Bastion's questlines share.
class RZ_BastionQuest
{
	//! A leader id no leader has (ids start at 1): a stage mission with it is in nobody's voice,
	//! and the mission manager doesn't give it one (RZ_Leaders.AssignVoice).
	static const int NO_VOICE = 0;

	//------------------------------------------------------------------------------------------------
	//! A leader who is there (alive, seat held), null otherwise.
	static RZ_Leader Leader(RZ_GameModeComponent gm, int leaderId)
	{
		if (!gm.GetLeaders() || leaderId <= 0)
			return null;

		RZ_Leader l = gm.GetLeaders().Get(leaderId);
		if (!l || !l.IsPresent())
			return null;

		return l;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_Leader Stronghold(RZ_GameModeComponent gm, RZ_EFaction faction)
	{
		if (!gm.GetLeaders())
			return null;

		RZ_Leader l = gm.GetLeaders().GetStronghold(faction);
		if (!l || !l.IsPresent())
			return null;

		return l;
	}

	//------------------------------------------------------------------------------------------------
	//! Remember who sits in a chair (seat id and generation), so a successor isn't taken for them.
	static void Keep(RZ_QuestRun run, string key, RZ_Leader l)
	{
		if (!run || !l)
			return;

		run.SetFlag(key, l.m_iId);
		run.SetFlag(key + "_gen", l.m_iGeneration);
	}

	//------------------------------------------------------------------------------------------------
	//! The person Keep() remembered, if they're still in that chair (not dead, dismissed or replaced).
	static RZ_Leader Kept(RZ_GameModeComponent gm, RZ_QuestRun run, string key)
	{
		if (!run)
			return null;

		RZ_Leader l = Leader(gm, run.Flag(key, -1));
		if (!l || l.m_iGeneration != run.Flag(key + "_gen", -1))
			return null;

		return l;
	}

	//------------------------------------------------------------------------------------------------
	//! A stage uses this leader: their own body goes, the stage's stand-in speaks for them (as Rein
	//! In). False if another mission holds them already.
	static bool Hold(RZ_GameModeComponent gm, RZ_Leader l, int missionId)
	{
		if (!l || l.m_iHeldBy >= 0 || !gm.GetLeaders())
			return false;

		gm.GetLeaders().Hold(l.m_iId, missionId);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static void Release(RZ_GameModeComponent gm, int leaderId, int missionId)
	{
		if (!gm.GetLeaders() || leaderId <= 0)
			return;

		RZ_Leader l = gm.GetLeaders().Get(leaderId);
		if (l && l.m_iHeldBy == missionId)
			gm.GetLeaders().Release(leaderId);
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest present leader of `faction` within maxDist of pos, any role. Null if none.
	static RZ_Leader Nearest(RZ_GameModeComponent gm, RZ_EFaction faction, vector pos, float maxDist)
	{
		if (!gm.GetLeaders())
			return null;

		return gm.GetLeaders().FindNearest(faction, pos, maxDist);
	}

	//------------------------------------------------------------------------------------------------
	//! The safehouse the Haven's chair sits in (the stronghold's), -1 if none.
	static int ChairHouse(RZ_GameModeComponent gm)
	{
		RZ_Leader chair = Stronghold(gm, RZ_EFaction.SURVIVORS);
		if (!chair || !gm.GetSafehouses())
			return -1;

		return gm.GetSafehouses().FindNearest(chair.m_vPos, 150);
	}

	//------------------------------------------------------------------------------------------------
	//! After the nightly call (20:30): night. A server whose clock doesn't move counts as always.
	static bool IsAfterCall()
	{
		if (RZ_GameTime.GetHour() < 0 || !RZ_GameTime.IsCalibrated())
			return true;

		return RZ_GameTime.IsNight();
	}

	//------------------------------------------------------------------------------------------------
	//! "at dawn", "in the night"... for headlines that say when something happened.
	static string When()
	{
		float h = RZ_GameTime.GetHour();
		if (h < 0)
			return "today";

		if (h >= 5 && h < 8)
			return "at dawn";

		if (h >= 8 && h < 17.5)
			return "in daylight";

		if (h >= 17.5 && h < RZ_GameTime.NIGHT_START)
			return "at dusk";

		return "in the night";
	}

	//------------------------------------------------------------------------------------------------
	//! What `faction`'s people think of this player: Stranger 0, Known 1, Trusted 2, Champion 3,
	//! Watched -1, Hunted -2 (RZ_QuestApproach.PersuadeScore without a background).
	static int Rank(RZ_GameModeComponent gm, int playerId, int faction)
	{
		return RZ_QuestApproach.PersuadeScore(gm, playerId, faction, "");
	}

	//------------------------------------------------------------------------------------------------
	static string Ident(RZ_GameModeComponent gm, int playerId)
	{
		if (playerId <= 0)
			return "";

		return gm.GetProfile(playerId).m_sIdentity;
	}

	//------------------------------------------------------------------------------------------------
	//! Did this identity take `approach` at `stage` of the run?
	static bool DidAt(RZ_QuestRun run, string ident, int approach, int stage)
	{
		if (!run || ident.IsEmpty())
			return false;

		foreach (RZ_QuestPart p : run.m_aParts)
		{
			if (p.m_sIdent == ident && p.m_iApproach == approach && p.m_iStage == stage)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A season-wide world flag (docs/QUESTLINES.md, "Crossings"), `def` without questlines.
	static int World(RZ_GameModeComponent gm, string key, int def = 0)
	{
		if (!gm || !gm.GetQuestlines())
			return def;

		return gm.GetQuestlines().WorldFlag(key, def);
	}

	//------------------------------------------------------------------------------------------------
	static void SetWorld(RZ_GameModeComponent gm, string key, int value)
	{
		if (gm && gm.GetQuestlines())
			gm.GetQuestlines().SetWorldFlag(key, value);
	}

	//------------------------------------------------------------------------------------------------
	//! "two hundred and eleven", for what the radio reads out (0 to 999; digits above).
	static string NumberWords(int n)
	{
		array<string> ones = {"no", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen"};
		array<string> tens = {"", "", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"};
		if (n < 0)
			n = 0;

		if (n < 20)
			return ones[n];

		if (n < 100)
		{
			string t = tens[n / 10];
			if (n % 10 != 0)
				t += "-" + ones[n % 10];

			return t;
		}

		if (n < 1000)
		{
			string h = ones[n / 100] + " hundred";
			if (n % 100 != 0)
				h += " and " + NumberWords(n % 100);

			return h;
		}

		return n.ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! A safehouse index as a bit, for flags that hold a set of houses (0 past the 31st house).
	static int HouseBit(int house)
	{
		if (house < 0 || house > 30)
			return 0;

		return 1 << house;
	}

	//------------------------------------------------------------------------------------------------
	//! A stage that runs this many real hours (a consequence that lasts), times the wait scale.
	static int RealSecondsFrom(RZ_GameModeComponent gm, float hours)
	{
		float scale = 1;
		if (gm.GetQuestlines())
			scale = gm.GetQuestlines().WaitScale();

		return RZ_Util.RoundToInt(hours * 3600 * scale);
	}

	//------------------------------------------------------------------------------------------------
	//! A mission of this type is running and not finished (one finishing this tick doesn't count).
	static bool IsActive(RZ_GameModeComponent gm, typename type)
	{
		if (!gm.GetMissions())
			return false;

		foreach (RZ_Mission m : gm.GetMissions().GetActive())
		{
			if (m.Type() == type && m.m_eState == RZ_EMissionState.ACTIVE)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! An in-person job: word reaches each eligible player online once, off the radio. They count
	//! as offered it (so it's in their journal). `told` keeps who has heard.
	static void Whisper(RZ_GameModeComponent gm, RZ_Mission m, string title, string text, notnull array<int> told)
	{
		array<int> all = {};
		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int playerId : all)
		{
			if (told.Contains(playerId) || !m.IsEligible(gm, playerId))
				continue;

			told.Insert(playerId);
			if (!m.m_aOfferedTo.Contains(playerId))
				m.m_aOfferedTo.Insert(playerId);

			gm.NotifyPlayer(playerId, title, text);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! An item a questline stage hands from person to person (the runner's page, a ration crate, a
//! black box): the mission item, put on the ground, followed into inventories, put back where it
//! was last seen if its carrier leaves the server (3 times at most), and taken by a use action on
//! whoever receives it (TakeFrom).
class RZ_QuestCarry
{
	protected IEntity m_Item;
	protected int m_iMission = -1;
	protected vector m_vLast;
	protected bool m_bSpawned;
	protected bool m_bCarried;
	protected bool m_bDelivered;
	protected int m_iReturns;
	protected int m_iCarrier = -1;

	protected static const int MAX_RETURNS = 3;

	//------------------------------------------------------------------------------------------------
	bool IsSpawned()
	{
		return m_bSpawned;
	}

	//------------------------------------------------------------------------------------------------
	bool IsDelivered()
	{
		return m_bDelivered;
	}

	//------------------------------------------------------------------------------------------------
	//! On the ground (not in anyone's inventory), for the mission's prop spots.
	bool IsOnGround()
	{
		return m_Item && !m_bCarried && !m_bDelivered;
	}

	//------------------------------------------------------------------------------------------------
	vector GetPos()
	{
		return m_vLast;
	}

	//------------------------------------------------------------------------------------------------
	//! Puts it on the ground at pos, tagged for mission `missionId`. False without the prefab.
	bool Spawn(RZ_GameModeComponent gm, int missionId, vector pos)
	{
		ResourceName prefab = gm.GetMissionItemPrefab();
		if (prefab.IsEmpty())
			return false;

		m_iMission = missionId;
		m_Item = RZ_Util.SpawnPrefab(prefab, RZ_Util.GroundPos(pos) + Vector(0, 0.3, 0));
		if (!m_Item)
			return false;

		RZ_MissionItemComponent.Tag(m_Item, missionId);
		m_bSpawned = true;
		m_bCarried = false;
		m_vLast = m_Item.GetOrigin();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Once per mission tick. False once it's lost for good (gone from the ground, or put back too
	//! often). Whoever carries it takes part in mission `m`.
	bool Update(RZ_GameModeComponent gm, RZ_Mission m)
	{
		if (!m_bSpawned || m_bDelivered)
			return true;

		if (m_Item)
		{
			IEntity root = m_Item.GetRootParent();
			m_bCarried = root && root != m_Item;
			if (m_bCarried)
				m_vLast = root.GetOrigin();
			else
				m_vLast = m_Item.GetOrigin();

			m_iCarrier = RZ_Util.GetCarrierPlayerId(m_Item);
			if (m_iCarrier > 0 && !m.m_aParticipants.Contains(m_iCarrier))
				m.m_aParticipants.Insert(m_iCarrier);

			return true;
		}

		// Its carrier left the server (or their body was cleared away) and took it with them
		if (!m_bCarried || m_iReturns >= MAX_RETURNS)
			return false;

		string who = "";
		if (m_iCarrier > 0)
			who = RZ_Util.PlayerName(m_iCarrier);

		if (who.IsEmpty())
			who = "Whoever had it";

		m_iReturns++;
		m_iCarrier = -1;
		if (!Spawn(gm, m_iMission, RZ_Mission.LandPointNear(m_vLast, 0, 2)))
			return false;

		m.NotifyInterested(gm, m.m_sTitle, string.Format("%1 is gone. What they carried is lying where they were last seen (grid %2).", who, RZ_SettlementScanner.GridReference(m_vLast)));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The user hands it over: this very item is taken from their inventory (a stage with two of
	//! them, the ration crates, tells them apart). False if they haven't got it.
	bool TakeFrom(IEntity user)
	{
		if (m_bDelivered || !user || !m_Item)
			return false;

		array<IEntity> items = {};
		RZ_MissionItemHelper.FindItems(user, items);
		if (!items.Contains(m_Item))
			return false;

		RZ_MissionItemHelper.RemoveItem(user, m_Item);
		m_bDelivered = true;
		m_Item = null;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void Cleanup()
	{
		if (m_Item && !m_bDelivered)
			RZ_Util.DeleteUnit(m_Item);

		m_Item = null;
	}
}

//------------------------------------------------------------------------------------------------
//! Await Relief (Task Force Bastion; B1 in docs/QUESTLINES.md). An Independent outpost commander's
//! radio operator wrote something other than "No reply" in the log after the nightly call, then
//! crossed it out: a long, broken burst that comes in every night. It isn't Command. The relay on
//! the hill still loops the Day 3 signal, because nobody ever acknowledged it; the runner who
//! carried the decoded text never arrived, and his page says "expended".
//!
//! It also follows on from the No Reply contract: when that one's source is the tape, the reel
//! carries the burst after the voice, and this questline starts at that post (OnTapeFound).
//!
//! 1. The operator's tape (mission, in person, after the nightly call): take it to the relay
//!    and keep the relay running 3 minutes; or copy it unseen (sneak), which keeps a copy.
//! 2. The runner (mission): his trail through three towns (step chain): his pack, his fire, his body.
//! 3. Who reads it (mission): the page to the commanding officer (the stove), to the commander
//!    (read to his men), to a Red Hand crew (the open channel) or to the Haven's chair.
//! 4. Word gets round (wait: two hours of play or 12 real hours).
//! 5. The reckoning (mission, by branch): the CO's squad comes for the commander (fight, arrest
//!    him yourself, talk the CO down); the operator who "leaked" it has run (bring him in or hide
//!    him); a copy of the tape can still get out (once); the chair's price (rations).
//!
//! Endings: Into the stove, The men were told, On the open channel, The Haven's leverage.
class RZ_Quest_AwaitRelief : RZ_Questline
{
	static const string KEY = "orders";

	static const int S_TAPE = 0;
	static const int S_RUNNER = 1;
	static const int S_WHO = 2;
	static const int S_WAIT = 3;
	static const int S_RECKON = 4;

	static const int OUT_LOOP = 10;		// stage 1: compared at the relay
	static const int OUT_PAGE = 11;		// stage 2: the runner found, his page read
	static const int OUT_STOVE = 12;	// stages 3 and 5: to the commanding officer
	static const int OUT_READ = 13;		// to the commander who sent you
	static const int OUT_MOCKED = 14;	// to a Red Hand crew
	static const int OUT_HAVEN = 15;	// to the Haven's chair
	static const int OUT_FOUGHT = 16;	// stage 5 (read): the squad killed
	static const int OUT_ARRESTED = 17;	// walked him in yourself
	static const int OUT_TALKED = 18;	// the CO talked down
	static const int OUT_TAKEN = 19;	// the squad took him
	static const int OUT_BROUGHT = 20;	// stage 5 (mocked): the operator brought in
	static const int OUT_HIDDEN = 21;	// walked to a safehouse
	static const int OUT_PAID = 22;		// stage 5 (the Haven): the chair's price delivered

	static const int BRANCH_STOVE = 0;
	static const int BRANCH_READ = 1;
	static const int BRANCH_MOCKED = 2;
	static const int BRANCH_HAVEN = 3;

	static const int E_STOVE = 0;
	static const int E_READ = 1;
	static const int E_MOCKED = 2;
	static const int E_HAVEN = 3;

	//! The No Reply contract's tape (OnTapeFound), for the CanStart right after it
	protected static int s_iTapeLeader = -1;
	protected static string s_sTapeOperator;

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		Info(KEY, "Await Relief", RZ_EFaction.MILITARY);

		RZ_QuestStage tape = MissionStage(S_TAPE, "The operator's tape", "{operator}, the radio operator at {post}, wrote something other than \"No reply\" in the log after the nightly call, then crossed it out. He has it on tape. {cmd} wants to know what it is before the stronghold does. Take the tape to the relay on the hill and compare.");
		tape.GiveUp(4, "Nobody took {operator}'s tape anywhere. In the log at {post}, the crossed-out line is still there.");
		tape.Visits(2);

		RZ_QuestStage runner = MissionStage(S_RUNNER, "The runner", "The Day 3 signal came in too broken for voice and was decoded by hand. {runner} carried the page from the relay at 04:40. He never reached {stronghold}. Follow his trail: his pack, his fire, him.");
		runner.GiveUp(4, "Nobody walked {runner}'s trail. Wherever he is, the page is still in his pocket.");
		runner.Visits(2);

		RZ_QuestStage who = MissionStage(S_WHO, "Who reads it", "{runner}'s page says, in parts: \"...hold the line... no relief will... authorised to... expended.\" It's on his body in {body_town}. Somebody has to read it: the commanding officer, {cmd}, the Red Hand or the Haven's chair.");
		who.GiveUp(3, "The page stayed on {runner}. The rain is finishing what the water started.");
		who.Visits(2);

		RZ_QuestStage wait = WaitStage(S_WAIT, "Word gets round", "Whoever has the page has read it. By tomorrow, so will everyone it matters to.");
		wait.Wait(120, 12);
		wait.GiveUp(4, "It went quiet. Whatever the page started, it stopped there.");

		RZ_QuestStage reckon = MissionStage(S_RECKON, "The reckoning", "The page has done what pages do. Now someone pays for it.");
		reckon.GiveUp(4, "Nobody came to settle it, and in time it settled itself.");
	}

	// ================================================================ Starting
	//------------------------------------------------------------------------------------------------
	//! Could No Reply's tape at this post lead here now? Questlines on, this one not running or
	//! done, and the post an Independent Bastion commander's.
	static bool CanFollowTape(RZ_GameModeComponent gm, int leaderId)
	{
		RZ_Questlines q = gm.GetQuestlines();
		if (!q || !q.IsEnabled() || !q.FindLine(KEY) || q.FindRunByKey(KEY) || q.HasWorldFlag(KEY + ".ending"))
			return false;

		return IsCandidate(RZ_BastionQuest.Leader(gm, leaderId));
	}

	//------------------------------------------------------------------------------------------------
	//! No Reply's log is written after the tape was found: remember the post (saved, for when
	//! Bastion's questline slot is free) and start now if the rules allow.
	static void OnTapeFound(RZ_GameModeComponent gm, int leaderId, string operatorName)
	{
		RZ_Leader l = RZ_BastionQuest.Leader(gm, leaderId);
		if (!l || !gm.GetQuestlines())
			return;

		RZ_BastionQuest.SetWorld(gm, KEY + ".tape", l.m_iId);
		RZ_BastionQuest.SetWorld(gm, KEY + ".tape_gen", l.m_iGeneration);
		s_iTapeLeader = l.m_iId;
		s_sTapeOperator = operatorName;
		gm.GetQuestlines().TryBegin(KEY);
		s_iTapeLeader = -1;
		s_sTapeOperator = "";
	}

	//------------------------------------------------------------------------------------------------
	//! While this runs, the Day 3 orders are this story's: no Real Orders job (Missions/Leaders/)
	//! tells another version of the same lost courier.
	static bool IsRunning(RZ_GameModeComponent gm)
	{
		return gm.GetQuestlines() && gm.GetQuestlines().FindRunByKey(KEY) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! This leader's post is this story's while it runs: No Reply picks another post.
	static bool IsAtPost(RZ_GameModeComponent gm, int leaderId)
	{
		if (!gm.GetQuestlines() || leaderId <= 0)
			return false;

		RZ_QuestRun run = gm.GetQuestlines().FindRunByKey(KEY);
		return run && run.Flag("cmd", -1) == leaderId;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IsCandidate(RZ_Leader l)
	{
		return l && l.m_eFaction == RZ_EFaction.MILITARY && !l.IsStronghold() && l.m_eLoyalty == RZ_ELoyalty.INDEPENDENT && l.m_iHeldBy < 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Who it starts with: No Reply's tape post, else the commander on the Red Hand's page (R1's
	//! book.bastion) if he's still in the chair, else any Independent outpost commander.
	protected RZ_Leader PickCommander(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		int tapeId = RZ_BastionQuest.World(gm, KEY + ".tape");
		RZ_Leader tape = RZ_BastionQuest.Leader(gm, tapeId);
		if (tape && tape.m_iGeneration == RZ_BastionQuest.World(gm, KEY + ".tape_gen", -1) && IsCandidate(tape))
		{
			run.SetFlag("from_tape", 1);
			return tape;
		}

		RZ_Leader book = RZ_BastionQuest.Leader(gm, RZ_BastionQuest.World(gm, "book.bastion") - 1);	// R1: the leader id + 1
		if (IsCandidate(book))
		{
			run.SetFlag("rifles", 1);
			return book;
		}

		if (!gm.GetLeaders())
			return null;

		array<RZ_Leader> list = {};
		gm.GetLeaders().GetFactionLeaders(RZ_EFaction.MILITARY, list);
		array<RZ_Leader> options = {};
		foreach (RZ_Leader l : list)
		{
			if (IsCandidate(l))
				options.Insert(l);
		}

		if (options.IsEmpty())
			return null;

		return options.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! Needs an Independent Bastion outpost commander, the commanding officer, the note, a person
	//! to talk to and the mission item (the page). Not once the cure is done.
	override bool CanStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (!RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN) || gm.GetMissionItemPrefab().IsEmpty())
			return false;

		if (gm.GetCure().IsCured() || gm.GetDirector().GetStageSettings().m_fMilitaryPresence <= 0)
			return false;

		RZ_Leader co = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY);
		RZ_Leader cmd = PickCommander(gm, run);
		if (!co || !cmd)
			return false;

		// The tape's hook is used up either way
		if (run.Flag("from_tape") == 1)
			RZ_BastionQuest.SetWorld(gm, KEY + ".tape", 0);

		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		RZ_BastionQuest.Keep(run, "cmd", cmd);
		RZ_BastionQuest.Keep(run, "co", co);
		run.SetText("cmd", cmd.m_sShort);
		run.SetText("commander", cmd.m_sName);
		run.SetText("post", cmd.m_sPlace);
		run.SetText("co", co.m_sShort);
		run.SetText("stronghold", co.m_sPlace);

		string op = s_sTapeOperator;
		if (op.IsEmpty() || cmd.m_iId != s_iTapeLeader)
			op = "Corporal " + RZ_NoteText.Surname(rng);

		run.SetText("operator", op);
		run.SetText("runner", "Private " + RZ_NoteText.Surname(rng));
		run.SetText("body_town", "the last town on his road");
		run.SetPlace(cmd.m_vPos, cmd.m_sPlace);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.Flag("from_tape") == 1)
			run.Note("The reel from the field had more on it than the voice: a long, broken burst after it, then hiss. {operator} at {post} knows that burst.");
		else
			run.Note("At {post}, {operator} wrote \"Reply received\" in the log after the nightly call, then crossed it out. It's happened four nights running.");

		if (run.Flag("rifles") == 1)
			run.Note("{cmd} is the commander whose name was on the Red Hand's page, for the rifles. He can't afford another mark against him.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnStageBegin(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (stage != S_WAIT)
			return;

		switch (run.m_iBranch)
		{
			case BRANCH_STOVE:
			{
				run.Note("{operator} has been moved to {stronghold}, \"for his own good\". The log at {post} says \"No reply\" again.");
				break;
			}
			case BRANCH_READ:
			{
				Headline(gm, run.Fill("Two men walked off the Bastion post in {post} after the shift change. At the roll that night, nobody answered for them."));
				run.Note("Two of {cmd}'s men walked off {post} before dawn. The stronghold will have heard by now.");
				break;
			}
			case BRANCH_MOCKED:
			{
				Headline(gm, "The Red Hand read one word on the open channel every hour last night: \"expended\". Bastion's channel stayed silent.");
				run.Note("The Red Hand reads \"expended\" on the open channel every hour. Bastion has gone quiet. Somebody at {stronghold} wants to know who leaked it.");
				break;
			}
			case BRANCH_HAVEN:
			{
				run.Note("The chair keeps the page in a locked drawer. She says nothing, yet.");
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The operator's tape waits for the commander to still be there, and for the nightly call.
	override bool IsReady(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (gm.GetCure().IsCured())
			return false;

		if (stage == S_TAPE)
		{
			RZ_Leader cmd = RZ_BastionQuest.Kept(gm, run, "cmd");
			return cmd && cmd.m_iHeldBy < 0 && RZ_BastionQuest.IsAfterCall();
		}

		if (stage == S_RECKON && run.m_iBranch == BRANCH_HAVEN)
			return RZ_BastionQuest.ChairHouse(gm) >= 0;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override RZ_Mission MakeMission(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		switch (stage)
		{
			case S_TAPE: return new RZ_QM_OrdersTape();
			case S_RUNNER: return new RZ_QM_OrdersRunner();
			case S_WHO: return new RZ_QM_OrdersWho();
		}

		if (stage != S_RECKON)
			return null;

		switch (run.m_iBranch)
		{
			case BRANCH_READ: return new RZ_QM_OrdersSquad();
			case BRANCH_MOCKED: return new RZ_QM_OrdersOperator();
			case BRANCH_STOVE: return new RZ_QM_OrdersWho();
			case BRANCH_HAVEN: return new RZ_QM_OrdersPrice();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Who a stage's contract is in the voice of: the commander while he's in the chair, else
	//! nobody (the stronghold mustn't hear about it).
	static int Voice(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_Leader cmd = RZ_BastionQuest.Kept(gm, run, "cmd");
		if (cmd)
			return cmd.m_iId;

		return RZ_BastionQuest.NO_VOICE;
	}

	// ================================================================ The story
	//------------------------------------------------------------------------------------------------
	override int Next(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		switch (stage)
		{
			case S_TAPE:
			{
				if (outcome == OUT_LOOP)
				{
					run.Note("It isn't an answer. The relay on the hill still loops the Day 3 signal, because nobody ever acknowledged it. {runner} left with the decoded text at 04:40 and never arrived.");
					return S_RUNNER;
				}

				if (!RZ_BastionQuest.Kept(gm, run, "cmd"))
					return Fizzle(run, "{cmd} is gone from {post}, and nobody else there wants to talk about the log.");

				run.Note("Nobody got the tape to the relay. {operator} plays it to himself after the call.");
				return S_TAPE;
			}
			case S_RUNNER:
			{
				if (outcome == OUT_PAGE)
				{
					run.Note("{runner}'s page, in parts: \"...hold the line... no relief will... authorised to... expended.\" It's still on him, in {body_town}.");
					return S_WHO;
				}

				run.Note("{runner}'s trail went cold. Someone will have to walk it again.");
				return S_RUNNER;
			}
			case S_WHO:
				return AfterWho(gm, run, outcome);
			case S_WAIT:
				return AfterWait(gm, run);
			case S_RECKON:
				return AfterReckoning(gm, run, outcome);
		}

		return Fizzle(run, "");
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterWho(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		if (outcome == OUT_STOVE)
		{
			run.SetBranch(BRANCH_STOVE);
			RZ_BastionQuest.SetWorld(gm, KEY + ".stove", 1);
			run.Note("The page went into the stove at {stronghold}. \"These never arrived.\"");
			return S_WAIT;
		}

		if (outcome == OUT_READ)
		{
			run.SetBranch(BRANCH_READ);
			RZ_BastionQuest.SetWorld(gm, KEY + ".read", 1);
			run.Note("{cmd} read the page to his men at the shift change, every word that's left.");
			return S_WAIT;
		}

		if (outcome == OUT_MOCKED)
		{
			run.SetBranch(BRANCH_MOCKED);
			RZ_BastionQuest.SetWorld(gm, KEY + ".mocked", 1);
			run.Note("A Red Hand crew has the page. They're reading it on the open channel.");
			return S_WAIT;
		}

		if (outcome == OUT_HAVEN)
		{
			run.SetBranch(BRANCH_HAVEN);
			RZ_BastionQuest.SetWorld(gm, KEY + ".haven", 1);
			run.Note("The Haven's chair has the page: proof of what Bastion was told.");
			return S_WAIT;
		}

		run.Note("Nobody carried the page anywhere. It's back where it was, on {runner}.");
		return S_WHO;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterWait(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.m_iBranch == BRANCH_READ && !RZ_BastionQuest.Kept(gm, run, "cmd"))
		{
			run.SetFlag("gone", 1);
			return Finish(run, E_READ, "The men were told. By the time {co}'s squad came for {cmd}, he was already gone from {post}.");
		}

		// The stove: only a copy of the tape can still get out
		if (run.m_iBranch == BRANCH_STOVE && run.Flag("copy") != 1)
			return Finish(run, E_STOVE, "Into the stove. The page is ash, and the log at {post} says \"No reply\" again.");

		// Nowhere for the operator to run to, or no house for the chair to be paid at
		if (run.m_iBranch == BRANCH_MOCKED && RZ_ContractHelp.CountSafehouses(gm) == 0)
			return Finish(run, E_MOCKED, "On the open channel. Bastion found {operator} before anyone else did. The Red Hand is still laughing.");

		if (run.m_iBranch == BRANCH_HAVEN && RZ_BastionQuest.ChairHouse(gm) < 0)
			return Finish(run, E_HAVEN, "The Haven's leverage. The chair has the page, and asks Bastion for nothing. Yet.");

		return S_RECKON;
	}

	//------------------------------------------------------------------------------------------------
	protected int AfterReckoning(RZ_GameModeComponent gm, RZ_QuestRun run, int outcome)
	{
		switch (run.m_iBranch)
		{
			case BRANCH_READ:
			{
				if (outcome == OUT_FOUGHT)
				{
					run.SetFlag("fought", 1);
					return Finish(run, E_READ, "The men were told, and {cmd} kept {post}. {co}'s squad didn't go back to {stronghold}.");
				}

				if (outcome == OUT_ARRESTED)
				{
					run.SetFlag("arrested", 1);
					if (run.Flag("cmd_dead") == 1)
						return Finish(run, E_READ, "The men were told. {cmd} never reached {stronghold}.");

					return Finish(run, E_READ, "The men were told. {cmd} walked into {stronghold} under arrest, and nobody had to drag him.");
				}

				if (outcome == OUT_TALKED)
				{
					run.SetFlag("talked", 1);
					return Finish(run, E_READ, "The men were told. {co} called the squad back, and {cmd} is back in line. For now.");
				}

				run.SetFlag("taken", 1);
				return Finish(run, E_READ, "The men were told. {co}'s squad took {cmd} to {stronghold} in the night.");
			}
			case BRANCH_MOCKED:
			{
				if (outcome == OUT_BROUGHT)
				{
					run.SetFlag("brought", 1);
					return Finish(run, E_MOCKED, "On the open channel. {operator} was walked into {stronghold} for a leak he didn't make. The Red Hand is still laughing.");
				}

				if (outcome == OUT_HIDDEN)
				{
					run.SetFlag("hidden", 1);
					return Finish(run, E_MOCKED, "On the open channel. {operator} sleeps at a Haven safehouse now, and the Red Hand is still laughing.");
				}

				return Finish(run, E_MOCKED, "On the open channel. Bastion found {operator} before anyone else did. The Red Hand is still laughing.");
			}
			case BRANCH_STOVE:
			{
				if (outcome == OUT_READ)
				{
					run.SetBranch(BRANCH_READ);
					run.SetFlag("copy_used", 1);
					RZ_BastionQuest.SetWorld(gm, KEY + ".read", 1);
					return Finish(run, E_READ, "The men were told. The page burned, but a copy of the tape didn't, and {cmd} played it at the shift change.");
				}

				if (outcome == OUT_MOCKED)
				{
					run.SetBranch(BRANCH_MOCKED);
					run.SetFlag("copy_used", 1);
					RZ_BastionQuest.SetWorld(gm, KEY + ".mocked", 1);
					return Finish(run, E_MOCKED, "On the open channel. The page burned, but a copy of the tape didn't. The Red Hand plays it every hour.");
				}

				if (outcome == OUT_HAVEN)
				{
					run.SetBranch(BRANCH_HAVEN);
					run.SetFlag("copy_used", 1);
					RZ_BastionQuest.SetWorld(gm, KEY + ".haven", 1);
					return Finish(run, E_HAVEN, "The Haven's leverage. The page burned, but the chair has a copy of the tape, and Bastion knows it.");
				}

				return Finish(run, E_STOVE, "Into the stove. The page is ash, the copy went the same way, and the log at {post} says \"No reply\".");
			}
			case BRANCH_HAVEN:
			{
				if (outcome == OUT_PAID)
				{
					run.SetFlag("paid", 1);
					return Finish(run, E_HAVEN, "The Haven's leverage. The chair has the page, and Bastion's rations stop at her door.");
				}

				return Finish(run, E_HAVEN, "The Haven's leverage. Bastion's rations never came, and the chair still has the page.");
			}
		}

		return Fizzle(run, "");
	}

	//------------------------------------------------------------------------------------------------
	override void OnEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
		switch (ending)
		{
			case E_STOVE:
			{
				EndStove(gm, run);
				break;
			}
			case E_READ:
			{
				EndRead(gm, run);
				break;
			}
			case E_MOCKED:
			{
				NudgeMorale(gm, RZ_EFaction.MILITARY, -10);
				ChangeStanding(gm, run, RZ_EFaction.BANDITS, 10, 0, RZ_EQuestApproach.SIDE);
				ChangeStanding(gm, run, RZ_EFaction.MILITARY, 0, 12, RZ_EQuestApproach.SIDE);
				if (gm.GetFactionLife())
					gm.GetFactionLife().StartRelation(RZ_EFaction.MILITARY, RZ_EFaction.BANDITS, RZ_FactionLife.REL_FEUD);

				SpreadRumour(gm, "\"Expended. That's what Bastion's orders say. The Red Hand reads it out every hour, in a posh voice.\"", 12);
				break;
			}
			case E_HAVEN:
			{
				NudgeMorale(gm, RZ_EFaction.SURVIVORS, 5);
				ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 10, 0);
				ChangeStanding(gm, run, RZ_EFaction.MILITARY, 5, 0);
				RZ_BastionQuest.SetWorld(gm, KEY + ".haven", 1);
				SpreadRumour(gm, "\"The chair's got something on Bastion. You can tell: their trucks slow down past her door now.\"", 10);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void EndStove(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		NudgeMorale(gm, RZ_EFaction.MILITARY, 5);
		ChangeStanding(gm, run, RZ_EFaction.MILITARY, 10, 0);

		// The commanding officer remembers who helped
		RZ_Leader co = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY);
		if (co)
		{
			array<string> idents = {};
			run.GetIdents(idents);
			foreach (string ident : idents)
			{
				co.Remember(ident, RZ_Leaders.MEM_HELPED);
			}
		}

		// The stronghold cleans house: the commander who sent you is relieved
		RZ_Leader cmd = RZ_BastionQuest.Kept(gm, run, "cmd");
		if (cmd && ReplaceLeader(gm, cmd.m_iId, run.Fill("{commander} has been relieved of {post}. The stronghold gave no reason.")))
			run.Note("{cmd} has been relieved of {post}.");

		SpreadRumour(gm, run.Fill("\"The log at {post} says 'No reply' again. Their operator's at the stronghold now. Nobody asks why.\""), 8);
	}

	//------------------------------------------------------------------------------------------------
	protected void EndRead(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		NudgeMorale(gm, RZ_EFaction.MILITARY, -8);
		ChangeStanding(gm, run, RZ_EFaction.MILITARY, 8, 0);
		ChangeStanding(gm, run, RZ_EFaction.MILITARY, 0, 5, RZ_EQuestApproach.FORCE);

		// The two who walked off reach the nearest safehouse
		RZ_Leader seat = RZ_BastionQuest.Leader(gm, run.Flag("cmd", -1));
		vector from = run.m_vPlace;
		if (seat)
			from = seat.m_vPos;

		int house = -1;
		if (gm.GetSafehouses())
			house = gm.GetSafehouses().FindNearest(from, 6000);

		RZ_Safehouse h = GetHouse(gm, house);
		if (h)
		{
			ChangePopulation(gm, house, 2);
			ChangeStanding(gm, run, RZ_EFaction.SURVIVORS, 8, 0);
			run.SetText("refuge", h.m_sName);
			Headline(gm, run.Fill("Two Bastion deserters from {post} turned up at the safehouse in {refuge}. The council asked them nothing but their first names."), false);
		}

		RZ_Leader cmd = RZ_BastionQuest.Kept(gm, run, "cmd");
		if (!cmd || !gm.GetLeaders())
			return;

		if (run.Flag("fought") == 1)
		{
			// Rogue, unless another of Bastion's leaders already is (then Independent, as ever)
			int drop = Math.MinInt(-1, 15 - cmd.m_iScore);
			gm.GetLeaders().ShiftLoyalty(cmd, drop, true);
			if (cmd.m_eLoyalty != RZ_ELoyalty.ROGUE)
				Headline(gm, run.Fill("{commander} still holds {post}. The stronghold's squad didn't come back from it."));

			return;
		}

		if (run.Flag("talked") == 1)
		{
			gm.GetLeaders().ShiftLoyalty(cmd, 25, true);
			return;
		}

		if (run.Flag("arrested") == 1 || run.Flag("taken") == 1 || run.Flag("copy_used") == 1)
			ReplaceLeader(gm, cmd.m_iId, run.Fill("{commander} has been relieved of {post} and is held at {stronghold}."));
	}

	// ================================================================ Words
	//------------------------------------------------------------------------------------------------
	override string JournalText(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		bool cmdHere = RZ_BastionQuest.Kept(gm, run, "cmd") != null;
		switch (run.m_iStage)
		{
			case S_TAPE:
			{
				if (!cmdHere)
					return run.Fill("{cmd} is gone from {post}. Nobody else there wants to talk about the log.");

				if (run.Flag("from_tape") == 1)
					return run.Fill("The reel from the field had more on it than the voice: a long, broken burst after it. {operator} at {post} has heard that burst after the nightly call for four nights and crossed it out of the log every time. {cmd} wants it looked at before the stronghold hears. Take the tape to the relay on the hill and compare.");

				break;
			}
			case S_WHO:
			{
				if (!cmdHere)
					return run.Fill("{runner}'s page says, in parts: \"...hold the line... no relief will... authorised to... expended.\" It's on his body in {body_town}. {cmd} is gone from {post}. Somebody has to read it: the commanding officer, the Red Hand or the Haven's chair.");

				break;
			}
			case S_WAIT:
				return WaitText(run);
			case S_RECKON:
				return ReckonText(gm, run, cmdHere);
		}

		return super.JournalText(gm, run);
	}

	//------------------------------------------------------------------------------------------------
	protected string WaitText(RZ_QuestRun run)
	{
		switch (run.m_iBranch)
		{
			case BRANCH_STOVE: return run.Fill("The page went into the stove at {stronghold}. {operator} has been moved there, for his own good. The log at {post} says \"No reply\".");
			case BRANCH_READ: return run.Fill("{cmd} read the page to his men at the shift change. Two of them walked off {post} before dawn. The stronghold will have heard by now.");
			case BRANCH_MOCKED: return run.Fill("The Red Hand reads \"expended\" on the open channel every hour. Bastion's channel has gone quiet. Somebody at {stronghold} wants to know who leaked it.");
			case BRANCH_HAVEN: return run.Fill("The chair keeps the page in a locked drawer. She says nothing, yet.");
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected string ReckonText(RZ_GameModeComponent gm, RZ_QuestRun run, bool cmdHere)
	{
		switch (run.m_iBranch)
		{
			case BRANCH_READ: return run.Fill("{co} has sent a squad to {post} to arrest {cmd}. Fight for him, take him in yourself, or talk {co} down.");
			case BRANCH_MOCKED: return run.Fill("The stronghold blames {operator} for the leak, and he has run. Find him: bring him in, or walk him to a safehouse.");
			case BRANCH_STOVE:
			{
				if (!cmdHere)
					return run.Fill("The page is ash. A copy of {operator}'s tape is still out there, with whoever made it. It can still go to a Red Hand crew or the chair. Once.");

				return run.Fill("The page is ash. A copy of {operator}'s tape is still out there, with whoever made it. It can still go to {cmd}, a Red Hand crew or the chair. Once.");
			}
			case BRANCH_HAVEN: return run.Fill("The chair's price for keeping quiet: Bastion rations for her house. Carry them to her.");
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override string Rumour(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		if (run.m_iStage <= S_RUNNER)
			return "\"An operator at {post} wrote 'reply received' in the log. Then he crossed it out. They say he's stopped sleeping.\"";

		if (run.m_iStage == S_WHO)
			return "\"Bastion lost a runner on Day 3. Somebody's out looking for what he was carrying. Bastion would rather they didn't find it.\"";

		if (run.m_iBranch == BRANCH_MOCKED)
			return "\"The Red Hand's got Bastion's real orders. One word. They won't stop saying it.\"";

		if (run.m_iBranch == BRANCH_READ)
			return "\"Two soldiers walked off {post} with their rifles. Something they were told at the shift change.\"";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The commander, while the runner is out there: he wants the page first.
	override string OnLeaderTalk(RZ_GameModeComponent gm, RZ_QuestRun run, int playerId, int leaderId)
	{
		if (run.m_iStage != S_RUNNER && run.m_iStage != S_WHO)
			return "";

		RZ_Leader cmd = RZ_BastionQuest.Kept(gm, run, "cmd");
		if (!cmd || cmd.m_iId != leaderId)
			return "";

		return string.Format("%1 keeps his voice down. \"The runner. Whatever he was carrying, bring it to me first. Not to the stronghold.\"", cmd.m_sShort);
	}
}

//------------------------------------------------------------------------------------------------
//! Await Relief, stage 1 (Bastion contract, Known, in person from the commander, after the nightly
//! call; ★ Radio Op): the operator's tape. Take it from him at the post, or copy it onto your own
//! set while he isn't looking (sneak: unseen, a radio on you; it keeps a copy). Play it into the
//! relay on the hill and keep the relay running 3 minutes (a horde after the first; ★ Radio Op:
//! twice as fast) while the two line up.
class RZ_QM_OrdersTape : RZ_QuestMission
{
	protected vector m_vPost;
	protected vector m_vOperator;
	protected vector m_vRelay;
	protected bool m_bPostShown;
	protected bool m_bRelayShown;
	protected bool m_bPlaying;
	protected bool m_bHorde;
	protected float m_fHeld;
	protected float m_fWhisper;
	protected int m_iStep;
	protected int m_iPlayer = -1;
	protected int m_iOperatorClue = -1;
	protected int m_iLogClue = -1;
	protected ref array<int> m_aHolders = {};
	protected ref array<int> m_aCopiers = {};
	protected ref array<int> m_aTold = {};

	protected static const int OBJ_OPERATOR = 0;
	protected static const int OBJ_RELAY = 1;
	protected static const float HOLD_SECONDS = 180;
	protected static const float HOLD_RADIUS = 30;
	protected static const float HORDE_AT = 60;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 12, 15);
		m_Req.Bonus("radioham");
		m_fTimeLeft = 3000;
		m_fParticipationRadius = 0;

		// The commander's own job, off the radio: he offers it at his post
		m_bInPersonOnly = true;
		RZ_QuestRun run = Run(gm);
		if (run)
			m_iLeaderId = run.Flag("cmd", -1);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		RZ_Leader cmd = RZ_BastionQuest.Kept(gm, run, "cmd");
		if (!cmd)
			return false;

		m_vPost = cmd.m_vPos;
		m_iSettlement = gm.FindNearestSettlement(m_vPost);
		m_vOperator = LandPointNear(m_vPost, 5, 10);
		m_vRelay = HighPointNear(m_vPost, 700, 1600, 16);
		if (RZ_Util.IsWater(m_vRelay) || RZ_Util.DistXZ(m_vRelay, m_vPost) < 500)
			return false;

		run.SetText("relaygrid", RZ_SettlementScanner.GridReference(m_vRelay));
		string talk = "He keeps his voice down. \"After the call. I read the numbers, I wait, and it comes: a long burst, like someone leaning on the key, all broken up. Then hiss. Same time every night.\" He shows you the log: \"Reply received\", and a line through it. \"I've got it on tape. The relay on the hill, grid {relaygrid}, would know if it came through there.\"";
		if (run.Flag("from_tape") == 1)
			talk = "\"That reel you brought in. After the voice. That's it, that burst.\" He shows you the log: four nights of \"Reply received\", each one crossed out. \"I taped it off my own set too. The relay on the hill, grid {relaygrid}, would know if it came through there.\"";

		m_iOperatorClue = AddClue(QText(gm, "operator"), QFill(gm, talk));
		m_iLogClue = AddClue("The relay's log", QFill(gm, "A signals log, the last page filled in on Day 3: \"04:10 long message, Command net, too broken for voice. Decoded by hand. 04:40 runner dispatched to {stronghold}: {runner}. 05:00 rebroadcast set to LOOP until acknowledged.\" Nothing after that. Nobody ever acknowledged."));

		m_vPos = m_vPost;
		m_sTitle = "Await Relief: the operator's tape";
		m_sBrief = QFill(gm, "My operator wrote \"Reply received\" in the log last night and crossed it out. He's not a liar and he's not mad. He's got it on tape. Take it up to the relay on the hill and find out what it is. You report to me. Not to the stronghold.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		result.Insert(m_vPost);
		result.Insert(m_vOperator);
		result.Insert(m_vRelay);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return QFill(gm, string.Format("{post}, %1\nThe relay on the hill (grid %2): %3", RZ_Util.DirectionText(from, m_vPost), RZ_SettlementScanner.GridReference(m_vRelay), RZ_Util.DirectionText(from, m_vRelay)));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		// Word reaches Bastion's people that the commander wants someone, off the radio
		m_fWhisper -= dt;
		if (m_fWhisper <= 0)
		{
			m_fWhisper = 60;
			RZ_BastionQuest.Whisper(gm, this, "Task Force Bastion", QFill(gm, "A corporal stops you, voice low: \"{cmd} at {post} wants a word. In person. Not on the radio.\""), m_aTold);
		}

		if (!m_bPostShown && RZ_ContractHelp.AnyNear(players, m_vPost, 300))
		{
			m_bPostShown = true;
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vOperator, OBJ_OPERATOR, "", "Take the tape|Copy it onto your own set while he isn't looking", m_iOperatorClue, "Talk to " + QText(gm, "operator"));
			m_Quiet.Watch(m_vPost, 60);
			RZ_GuidanceSettings guide = gm.GetGuidanceSettings();
			if (guide && !guide.m_sTableRadioPrefab.IsEmpty())
				TrackProp(RZ_Util.SpawnPrefab(guide.m_sTableRadioPrefab, RZ_Util.GroundPos(RZ_Util.RandomPointInRing(m_vOperator, 1, 1.5)) + Vector(0, 0.3, 0)));
		}

		if (!m_bRelayShown && RZ_ContractHelp.AnyNear(players, m_vRelay, 300))
		{
			m_bRelayShown = true;
			SpawnObject(gm, RZ_EPuzzleProp.RADIO_MAST, m_vRelay, OBJ_RELAY, "Play the tape into the relay", "", m_iLogClue, "Read the relay's log");
			SpawnGuards(gm, m_vRelay, 3, 15, 40);
		}

		if (m_bPlaying && !m_bResolved)
			UpdateHold(gm, dt, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateHold(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (CountPlayersNear(players, m_vRelay, HOLD_RADIUS) == 0)
			return;

		RZ_ContractHelp.AddNear(ids, players, m_vRelay, HOLD_RADIUS, m_aParticipants);
		float rate = 1;
		if (AnyBonusNear(gm, ids, players, m_vRelay, HOLD_RADIUS))
			rate = 2;

		m_fHeld += dt * rate;
		gm.GetHeat().AddHeat(m_vRelay, 2 * dt);
		if (!m_bHorde && m_fHeld >= HORDE_AT)
		{
			m_bHorde = true;
			gm.GetSpawner().SpawnHorde(m_vRelay, 350, m_iId);
			NotifyParticipants(gm, m_sTitle, "The relay is loud, and the hill carries it. They're coming up the slope.");
		}

		int step = Math.Floor(3 * m_fHeld / HOLD_SECONDS);
		if (step > m_iStep && step < 3)
		{
			m_iStep = step;
			NotifyParticipants(gm, m_sTitle, "The two signals are drifting closer together. Keep it running.");
		}

		if (m_fHeld < HOLD_SECONDS)
			return;

		int approach = RZ_EQuestApproach.TALK;
		string what = "took the operator's tape to the relay";
		if (m_aCopiers.Contains(m_iPlayer))
		{
			approach = RZ_EQuestApproach.SNEAK;
			what = "copied the tape unseen and took it to the relay";
		}

		RZ_QuestRun run = Run(gm);
		if (run)
			run.SetPlace(m_vRelay, QFill(gm, "the relay above {post}"));

		NotifyParticipants(gm, m_sTitle, QFill(gm, "They line up to the second. It isn't Command answering. It's the Day 3 signal, going round on the hill because nobody ever acknowledged it. The relay's log names the runner who carried the decoded page: {runner}. He left at 04:40 and never arrived."));
		Resolve(gm, m_iPlayer, approach, RZ_Quest_AwaitRelief.OUT_LOOP, what);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_OPERATOR)
			return;

		if (m_aHolders.Contains(playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You've got the tape. The relay is on the hill, grid {relaygrid}."));
			return;
		}

		if (option == 0)
		{
			m_aHolders.Insert(playerId);
			m_vPos = m_vRelay;
			Remember(gm, playerId, RZ_EQuestApproach.TALK, "took the operator's tape");
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "He hands it over and doesn't let go of it straight away. \"Bring it back. Or don't. Just tell me what it is.\" The relay is on the hill, grid {relaygrid}."));
			return;
		}

		if (option != 1)
			return;

		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!RZ_Radio.HasRadio(gm, playerId, user))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You'd need a set of your own to copy it onto.");
			return;
		}

		m_aHolders.Insert(playerId);
		m_vPos = m_vRelay;
		if (!m_Quiet.IsQuiet(gm, playerId))
		{
			string why = m_Quiet.Why(gm, playerId);
			Remember(gm, playerId, RZ_EQuestApproach.TALK, "was seen copying the tape, and took it instead");
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, why + " {operator} turns round. \"That's mine.\" He lets you take it, but he watches you go."));
			return;
		}

		m_aCopiers.Insert(playerId);
		SetQFlag(gm, "copy", 1);
		Remember(gm, playerId, RZ_EQuestApproach.SNEAK, "copied the operator's tape unseen");
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "While he reads the numbers into the handset, you run his tape through your own set. He never looks round. The copy is yours. The relay is on the hill, grid {relaygrid}."));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx != OBJ_RELAY)
			return;

		if (m_bPlaying)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "It's playing. Keep the relay running.");
			return;
		}

		if (!m_aHolders.Contains(playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You've no tape to play. {operator} has it, at {post}."));
			return;
		}

		m_bPlaying = true;
		m_iPlayer = playerId;
		m_vPos = m_vRelay;
		string text = "The relay's speaker crackles. Under the tape, the relay's own transmission comes up: a long burst, gaps, hiss. Keep the relay running while they line up.";
		if (RZ_Radio.IsRadioOperator(gm, playerId))
			text += " You know a loop when you hear one: same burst, same gaps. It isn't an answer. It's us.";

		RZ_ContractHelp.NotifyNear(gm, m_vRelay, 60, m_sTitle, text);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		if (idx == m_iLogClue)
			QNote(gm, "The relay's log, Day 3: the long message was decoded by hand, {runner} left with it at 04:40, and the relay was set to loop it until someone acknowledged. Nobody did.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "Await Relief: it isn't Command. It's the Day 3 signal, looping on the hill.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "Await Relief: nobody compared the tape. At {post}, the log still has its crossed-out line.");
	}
}

//------------------------------------------------------------------------------------------------
//! Await Relief, stage 2 (Bastion contract, Known): the runner's trail through three towns (a
//! step chain): his satchel, his fire, then him, with turned soldiers round his body. Reading the
//! page in his pocket ends the stage; it stays on him for the next one.
class RZ_QM_OrdersRunner : RZ_QuestMission
{
	protected ref RZ_StepChain m_Chain = new RZ_StepChain();
	protected int m_iPageClue = -1;
	protected int m_iReader = -1;
	protected vector m_vBody;
	protected string m_sBodyTown;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 15, 15);
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
		m_iLeaderId = RZ_Quest_AwaitRelief.Voice(gm, Run(gm));
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run || !run.m_bHasPlace)
			return false;

		array<int> towns = {};
		if (!RZ_StepChain.PickTowns(gm, run.m_vPlace, 3, 700, 2500, 0, 95, towns))
			return false;

		RZ_Settlement s1 = gm.GetSettlement(towns[0]);
		RZ_Settlement s2 = gm.GetSettlement(towns[1]);
		RZ_Settlement s3 = gm.GetSettlement(towns[2]);
		m_Chain.SetMission(m_iId);

		RZ_ChainStep pack = m_Chain.AddStep(gm, towns[0], LandPointNear(s1.m_vCenter, s1.m_fRadius * 0.5, s1.m_fRadius + 60), 6, 0);
		pack.m_iClue = AddClue("A runner's satchel", QFill(gm, string.Format("Bastion issue, the strap cut through. Inside the flap, in pencil: \"{runner}. Day 3. By hand to {stronghold}.\" The map in the pocket has %1 circled, and one road marked \"avoid\".", s2.m_sName)));
		pack.m_sReadLabel = "Go through the satchel";

		RZ_ChainStep fire = m_Chain.AddStep(gm, towns[1], LandPointNear(s2.m_vCenter, s2.m_fRadius * 0.4, s2.m_fRadius), 6, 0);
		fire.m_iClue = AddClue("A cold fire", string.Format("A fire pit, cold a long time. A ration tin with \"D3\" scratched on the lid, and boot prints in the dried mud heading for %1. One of them drags.", s3.m_sName));
		fire.m_sReadLabel = "Look around the fire";
		fire.m_iGuards = 2;

		RZ_ChainStep body = m_Chain.AddStep(gm, towns[2], LandPointNear(s3.m_vCenter, s3.m_fRadius * 0.3, s3.m_fRadius), 8, 0);
		m_iPageClue = AddClue(QText(gm, "runner"), QFill(gm, "{runner}, or what the infected left of him. One tag still round his neck. In his breast pocket, folded twice, a page of signal decoded by hand, water-stained: \"...TF CORDON... HOLD THE LINE... NO RELIEF WILL... AUTHORISED TO... EXPENDED.\" The rest has run. You leave it where it is for now."));
		body.m_iClue = m_iPageClue;
		body.m_eClueProp = RZ_EPuzzleProp.BODY;
		body.m_sReadLabel = "Search the runner";
		body.m_iGuards = 5;
		body.m_bSpecialGuard = true;
		m_vBody = body.m_vPos;
		m_sBodyTown = s3.m_sName;

		m_vPos = pack.m_vPos;
		m_iSettlement = towns[0];
		m_sTitle = "Await Relief: the runner";
		m_sBrief = QFill(gm, string.Format("The relay's log says a runner left with the decoded text on Day 3 and never arrived. {runner}. His trail starts in %1. Find him, and what he was carrying. Read it before anyone else does.", s1.m_sName));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		m_Chain.GetSpots(result);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		m_Chain.OnClueRead(idx);
		if (idx == m_iPageClue)
			m_iReader = playerId;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved)
			return;

		int done = m_Chain.Update(gm, dt, ids, players, m_aParticipants);
		RZ_ChainStep next = m_Chain.GetCurrent();
		if (next)
		{
			m_vPos = next.m_vPos;
			m_iSettlement = next.m_iSettlement;
		}

		if (done >= 0 && !m_Chain.IsDone())
			NotifyParticipants(gm, m_sTitle, "His trail goes on.");

		if (!m_Chain.IsDone())
			return;

		RZ_QuestRun run = Run(gm);
		if (run)
		{
			run.SetPlace(m_vBody, m_sBodyTown);
			run.SetText("body_town", m_sBodyTown);
		}

		int who = m_iReader;
		if (who <= 0 && !m_aParticipants.IsEmpty())
			who = m_aParticipants[0];

		Resolve(gm, who, RZ_EQuestApproach.NONE, RZ_Quest_AwaitRelief.OUT_PAGE, "found the runner and read his page");
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "Await Relief: you found {runner}, and you've read what he carried.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "Await Relief: {runner}'s trail went cold.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Chain.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! Await Relief, stage 3 (Bastion contract, Known): who reads it. Take the page from the runner's
//! body and give it to the commanding officer's duty officer (Trusted, or ★ Soldier: the stove),
//! the commander who sent you (held for the stage; a stand-in at his post), a Red Hand crew boss's
//! radio man (side with the Red Hand: 40 Red Hand Favor) or the Haven chair's clerk (side with
//! the Haven). Stage 5 of the stove's branch reuses it for a copy of the tape (only whoever made
//! the copy at stage 1 can give it; no page to carry).
class RZ_QM_OrdersWho : RZ_QuestMission
{
	protected ref RZ_QuestCarry m_Page = new RZ_QuestCarry();
	protected bool m_bCopy;
	protected vector m_vPage;
	protected vector m_vCo;
	protected vector m_vCmd;
	protected vector m_vCrew;
	protected vector m_vChair;
	protected bool m_bHasCo;
	protected bool m_bHasCmd;
	protected bool m_bHasCrew;
	protected bool m_bHasChair;
	protected bool m_bCoShown;
	protected bool m_bCmdShown;
	protected bool m_bCrewShown;
	protected bool m_bChairShown;
	protected int m_iCmdId = -1;
	protected int m_iChosen = -1;
	protected string m_sCrew;
	protected string m_sChair;

	protected static const int OBJ_CO = 0;
	protected static const int OBJ_CMD = 1;
	protected static const int OBJ_CREW = 2;
	protected static const int OBJ_CHAIR = 3;
	protected static const int CREW_FAVOR = 40;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 10, 10);
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
		m_iLeaderId = RZ_Quest_AwaitRelief.Voice(gm, Run(gm));
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run)
			return false;

		m_bCopy = run.m_iStage == RZ_Quest_AwaitRelief.S_RECKON;
		vector from = run.m_vPlace;
		if (!m_bCopy)
		{
			if (!run.m_bHasPlace)
				return false;

			m_vPage = LandPointNear(from, 0, 3);
		}

		RZ_Leader co = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY);
		if (co)
		{
			m_bHasCo = true;
			m_vCo = LandPointNear(co.m_vPos, 6, 12);
			if (m_bCopy)
				from = co.m_vPos;
		}

		RZ_Leader cmd = RZ_BastionQuest.Kept(gm, run, "cmd");
		if (RZ_BastionQuest.Hold(gm, cmd, m_iId))
		{
			m_bHasCmd = true;
			m_iCmdId = cmd.m_iId;
			m_vCmd = LandPointNear(cmd.m_vPos, 2, 4);
		}

		RZ_Leader boss = RZ_BastionQuest.Nearest(gm, RZ_EFaction.BANDITS, from, 6000);
		if (boss)
		{
			m_bHasCrew = true;
			m_vCrew = LandPointNear(boss.m_vPos, 6, 12);
			m_sCrew = boss.m_sShort + "'s radio man";
		}
		else
		{
			vector meet;
			string meetName;
			m_bHasCrew = RZ_LeaderJob.MeetingPoint(gm, RZ_EFaction.BANDITS, from, meet, meetName);
			m_vCrew = meet;
			m_sCrew = "a Red Hand runner";
		}

		RZ_Leader chair = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.SURVIVORS);
		if (chair)
		{
			m_bHasChair = true;
			m_vChair = LandPointNear(chair.m_vPos, 6, 12);
			m_sChair = chair.m_sShort + "'s clerk";
		}

		if (!m_bHasCo && !m_bHasCmd && !m_bHasCrew && !m_bHasChair)
			return false;

		string who = "a Red Hand crew or the Haven's chair";
		if (m_bHasCmd)
			who = "{cmd}, a Red Hand crew or the Haven's chair";

		if (m_bCopy)
		{
			m_vPos = from;
			m_sTitle = "Await Relief: the copy";
			m_sBrief = QFill(gm, "The page is ash, but whoever copied {operator}'s tape still has the Day 3 signal on it. It can go to " + who + ". Or into the stove after the page.");
		}
		else
		{
			m_vPos = m_vPage;
			m_iSettlement = gm.FindNearestSettlement(m_vPage);
			m_sTitle = "Await Relief: who reads it";
			m_sBrief = QFill(gm, "Take the page from {runner}'s body in {body_town}, and decide who reads it: the commanding officer, " + who + ". Everyone will want it.");
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		if (m_Page.IsOnGround())
			result.Insert(m_Page.GetPos());

		if (m_bHasCo)
			result.Insert(m_vCo);

		if (m_bHasCmd)
			result.Insert(m_vCmd);

		if (m_bHasCrew)
			result.Insert(m_vCrew);

		if (m_bHasChair)
			result.Insert(m_vChair);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = "";
		if (!m_bCopy)
			text = QFill(gm, "{runner}'s body, {body_town}: ") + RZ_Util.DirectionText(from, m_vPage);

		if (m_bHasCo)
			text += "\nThe duty officer, for the commanding officer: " + RZ_Util.DirectionText(from, m_vCo);

		if (m_bHasCmd)
			text += QFill(gm, "\n{cmd}, at {post}: ") + RZ_Util.DirectionText(from, m_vCmd);

		if (m_bHasCrew)
			text += "\n" + RZ_Util.Capitalize(m_sCrew) + " (the Red Hand): " + RZ_Util.DirectionText(from, m_vCrew);

		if (m_bHasChair)
			text += "\n" + RZ_Util.Capitalize(m_sChair) + " (the Haven's chair): " + RZ_Util.DirectionText(from, m_vChair);

		return text.Trim();
	}

	//------------------------------------------------------------------------------------------------
	protected string GiveLabel(string to)
	{
		if (m_bCopy)
			return "Give your copy of the tape to " + to;

		return "Give the page to " + to;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bCopy && !m_Page.IsSpawned() && RZ_ContractHelp.AnyNear(players, m_vPage, 300))
		{
			if (!m_Page.Spawn(gm, m_iId, m_vPage))
			{
				m_eState = RZ_EMissionState.FAILED;
				return;
			}
		}

		if (!m_bCopy && !m_Page.Update(gm, this))
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (m_bHasCo && !m_bCoShown && RZ_ContractHelp.AnyNear(players, m_vCo, 300))
		{
			m_bCoShown = true;
			int coClue = AddClue("The duty officer", QFill(gm, "\"Post business goes through channels.\" He looks at what you're holding, then at your face. \"If that's for {co}, give it to me.\""));
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vCo, OBJ_CO, GiveLabel("the duty officer (Trusted, or a Soldier)"), "", coClue, "Talk to the duty officer");
		}

		if (m_bHasCmd && !m_bCmdShown && RZ_ContractHelp.AnyNear(players, m_vCmd, 300))
		{
			m_bCmdShown = true;
			int cmdClue = AddClue(QText(gm, "cmd"), QFill(gm, "{cmd} doesn't get up. \"Well? What did he die carrying?\""));
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vCmd, OBJ_CMD, GiveLabel(QText(gm, "cmd")), "", cmdClue, "Talk to " + QText(gm, "cmd"));
		}

		if (m_bHasCrew && !m_bCrewShown && RZ_ContractHelp.AnyNear(players, m_vCrew, 300))
		{
			m_bCrewShown = true;
			int crewClue = AddClue(RZ_Util.Capitalize(m_sCrew), "He has a set on his knee and a grin on his face. \"Got something for the open channel? We pay for a good laugh.\"");
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vCrew, OBJ_CREW, GiveLabel(m_sCrew) + string.Format(" (%1 Red Hand Favor)", CREW_FAVOR), "", crewClue, "Talk to " + m_sCrew);
		}

		if (m_bHasChair && !m_bChairShown && RZ_ContractHelp.AnyNear(players, m_vChair, 300))
		{
			m_bChairShown = true;
			int chairClue = AddClue(RZ_Util.Capitalize(m_sChair), "\"The chair reads everything that comes to this door. Bastion walked us to the line on Day 2. She'd like to know what they were told.\"");
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vChair, OBJ_CHAIR, GiveLabel(m_sChair), "", chairClue, "Talk to " + m_sChair);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! It's this player's to give: the page in their pack, or (stage 5) the copy they made.
	protected bool Hand(RZ_GameModeComponent gm, int playerId, IEntity user)
	{
		if (!m_bCopy)
		{
			if (m_Page.TakeFrom(user))
				return true;

			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You haven't got the page. It's on {runner}, in {body_town}."));
			return false;
		}

		RZ_QuestRun run = Run(gm);
		if (RZ_BastionQuest.DidAt(run, RZ_BastionQuest.Ident(gm, playerId), RZ_EQuestApproach.SNEAK, RZ_Quest_AwaitRelief.S_TAPE))
			return true;

		gm.NotifyPlayer(playerId, m_sTitle, "You've no copy of the tape. Whoever made it has it.");
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		switch (objIdx)
		{
			case OBJ_CO:
			{
				if (!RZ_QuestApproach.Persuade(gm, playerId, RZ_EFaction.MILITARY, 2, "soldier"))
				{
					gm.NotifyPlayer(playerId, m_sTitle, "\"Through channels.\" The duty officer won't take anything from someone the stronghold doesn't trust.");
					return;
				}

				if (Hand(gm, playerId, user))
					ToStove(gm, playerId);

				return;
			}
			case OBJ_CMD:
			{
				if (Hand(gm, playerId, user))
					ToCommander(gm, playerId);

				return;
			}
			case OBJ_CREW:
			{
				if (RZ_BastionQuest.Rank(gm, playerId, RZ_EFaction.BANDITS) < 0)
				{
					gm.NotifyPlayer(playerId, m_sTitle, "\"You? We don't buy from you.\" He spits.");
					return;
				}

				if (Hand(gm, playerId, user))
					ToCrew(gm, playerId);

				return;
			}
			case OBJ_CHAIR:
			{
				if (RZ_BastionQuest.Rank(gm, playerId, RZ_EFaction.SURVIVORS) < 0)
				{
					gm.NotifyPlayer(playerId, m_sTitle, "\"The chair doesn't take things from you.\" The door stays where it is.");
					return;
				}

				if (Hand(gm, playerId, user))
					ToChair(gm, playerId);

				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ToStove(RZ_GameModeComponent gm, int playerId)
	{
		m_iChosen = OBJ_CO;
		RZ_QuestRun run = Run(gm);
		if (!m_bCopy && run)
			run.Reward(gm, playerId, RZ_EFaction.MILITARY, 8, 30, QFill(gm, "{co} remembers who brought it in."));

		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "The duty officer reads the first line and stops. He walks it through to {co} without reading the rest, and you hear the stove door. \"These never arrived. Thank you.\""));
		Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_AwaitRelief.OUT_STOVE, "gave it to the commanding officer");
	}

	//------------------------------------------------------------------------------------------------
	protected void ToCommander(RZ_GameModeComponent gm, int playerId)
	{
		m_iChosen = OBJ_CMD;
		if (m_bCopy)
		{
			// After the stove, this goes against the stronghold: it pays nothing for it
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{cmd} plays your copy twice with the volume low. Then he calls his men in. \"The page burned. This didn't.\""));
			Resolve(gm, playerId, RZ_EQuestApproach.BETRAY, RZ_Quest_AwaitRelief.OUT_READ, "played the copy to the commander", false);
			return;
		}

		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{cmd} reads it twice in the light from the radio. \"Expended.\" He folds it into his breast pocket. \"They'll hear it from me. At the shift change.\""));
		Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_AwaitRelief.OUT_READ, "gave it to the commander who sent them");
	}

	//------------------------------------------------------------------------------------------------
	protected void ToCrew(RZ_GameModeComponent gm, int playerId)
	{
		m_iChosen = OBJ_CREW;
		RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.BANDITS, RZ_EFaction.MILITARY, 5, 8);
		RZ_QuestRun run = Run(gm);
		if (run)
			run.Reward(gm, playerId, RZ_EFaction.BANDITS, 0, CREW_FAVOR, "The crew pays for a good laugh.");

		gm.NotifyPlayer(playerId, m_sTitle, "He reads it and laughs until he coughs. \"Oh, they'll love this.\" By the hour it's on the open channel: one word, over and over, in a posh voice. \"Expended.\"");
		Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_AwaitRelief.OUT_MOCKED, "sold it to the Red Hand", false);
	}

	//------------------------------------------------------------------------------------------------
	protected void ToChair(RZ_GameModeComponent gm, int playerId)
	{
		m_iChosen = OBJ_CHAIR;
		RZ_QuestApproach.SideWith(gm, playerId, RZ_EFaction.SURVIVORS, -1, 5, 0);
		gm.NotifyPlayer(playerId, m_sTitle, "The clerk takes it in to the chair. You hear a drawer lock. When the clerk comes back out, all he says is: \"She knows now what they were told. Thank you.\"");
		Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_AwaitRelief.OUT_HAVEN, "gave it to the Haven's chair", false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_BastionQuest.Release(gm, m_iCmdId, m_iId);
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailed(RZ_GameModeComponent gm)
	{
		RZ_BastionQuest.Release(gm, m_iCmdId, m_iId);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "Await Relief: it's been read.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (m_iChosen == OBJ_CREW)
			return "Await Relief: the Red Hand has it now. Bastion won't thank you.";

		if (m_iChosen == OBJ_CHAIR)
			return "Await Relief: the Haven's chair has it now. Bastion won't thank you.";

		if (m_bCopy)
			return "Await Relief: the copy stayed where it was.";

		return QFill(gm, "Await Relief: nobody carried the page anywhere.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Page.Cleanup();
		RZ_BastionQuest.Release(gm, m_iCmdId, m_iId);
	}
}

//------------------------------------------------------------------------------------------------
//! Await Relief, stage 5 of the "read" branch (Bastion contract from the commanding officer,
//! Known): the CO's squad comes to arrest the commander. He's held for the stage and stands at his
//! post as an escort. Fight for him (kill the squad, with the Bastion squad prefab set), take him
//! in yourself (he walks with you to the stronghold), or talk the CO down at the squad's sergeant
//! (Champion, or Trusted and ★ Soldier). Stand aside, or nobody comes: the squad takes him.
class RZ_QM_OrdersSquad : RZ_QuestMission
{
	protected ref RZ_Escort m_Him = new RZ_Escort();
	protected ref RZ_UnitWatch m_Squad = new RZ_UnitWatch();
	protected vector m_vPost;
	protected vector m_vDest;
	protected vector m_vSergeant;
	protected int m_iCmdId = -1;
	protected int m_iSergeantClue = -1;
	protected bool m_bShown;
	protected bool m_bArrested;
	protected bool m_bFought;

	protected static const int OBJ_HIM = 0;
	protected static const int OBJ_SERGEANT = 1;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 15, 15);
		m_Req.Bonus("soldier");
		m_fTimeLeft = 2700;
		m_fParticipationRadius = 0;
		RZ_Leader co = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY);
		if (co)
			m_iLeaderId = co.m_iId;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		RZ_Leader cmd = RZ_BastionQuest.Kept(gm, run, "cmd");
		RZ_Leader co = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY);
		if (!cmd || !co || !RZ_Escort.HasRoom(gm) || !RZ_BastionQuest.Hold(gm, cmd, m_iId))
			return false;

		m_iCmdId = cmd.m_iId;
		m_vPost = cmd.m_vPos;
		m_vDest = co.m_vPos;
		m_vSergeant = LandPointNear(m_vPost, 8, 14);
		m_iSettlement = gm.FindNearestSettlement(m_vPost);

		int talk = AddClue(cmd.m_sShort, QFill(gm, "{cmd} has his kit packed. \"They're coming for me. I told my own men the truth, and that's the charge.\" He doesn't reach for his rifle. \"If it's going to be anyone, I'd rather it was you.\""));
		m_iSergeantClue = AddClue("The squad's sergeant", QFill(gm, "\"Orders from {co}: {cmd} to {stronghold}, under arrest.\" He doesn't look happy about it. \"I'd rather walk back with him than carry him.\""));
		m_Him.m_sName = cmd.m_sShort;
		m_Him.m_sPrefab = RZ_Escort.PrefabFor(gm, 2);
		m_Him.m_bHide = false;
		m_Him.SetTalk(m_iId, OBJ_HIM, QFill(gm, "Take him in yourself (walk him to {stronghold})"), talk, "Talk to " + cmd.m_sShort);

		m_vPos = m_vPost;
		m_sTitle = "Await Relief: the reckoning";
		m_sBrief = QFill(gm, "{cmd} read classified traffic to his men and two of them walked. A squad is on its way to {post} to bring him in. Go with them. See it done properly.");
		return m_Him.Begin(gm, LandPointNear(m_vPost, 2, 4));
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		result.Insert(m_vPost);
		result.Insert(m_vSergeant);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = QFill(gm, "{post}: ") + RZ_Util.DirectionText(from, m_vPost);
		if (m_bArrested)
			text += QFill(gm, "\n{stronghold}: ") + RZ_Util.DirectionText(from, m_vDest);

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved)
			return;

		if (!m_bShown && RZ_ContractHelp.AnyNear(players, m_vPost, 400))
		{
			m_bShown = true;
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vSergeant, OBJ_SERGEANT, "", QFill(gm, "Talk {co} down on his radio (Champion, or Trusted and a Soldier)|Stand aside and let them take him"), m_iSergeantClue, "Talk to the sergeant");
			ResourceName squad = RZ_Mission_BustOut.SquadPrefab(gm);
			if (!squad.IsEmpty() && m_Squad.Spawn(squad, LandPointNear(m_vPost, 20, 35)))
			{
				TrackProp(m_Squad.m_Unit);
				if (gm.GetHostility())
					gm.GetHostility().WatchUnit(m_Squad.m_Unit, RZ_EFaction.MILITARY);
			}
		}

		m_Him.Update(gm, dt, ids, players);

		// He died on the road: the leader goes with him
		if (m_Him.IsDead())
		{
			RZ_BastionQuest.Release(gm, m_iCmdId, m_iId);
			if (gm.GetLeaders())
				gm.GetLeaders().KillOffscreen(m_iCmdId, QFill(gm, "was killed on the road to {stronghold}, under arrest"));

			SetQFlag(gm, "cmd_dead", 1);
			int who = -1;
			if (!m_aParticipants.IsEmpty())
				who = m_aParticipants[0];

			Resolve(gm, who, RZ_EQuestApproach.NONE, RZ_Quest_AwaitRelief.OUT_ARRESTED, "was there when he died", false);
			return;
		}

		if (m_bArrested)
		{
			m_vPos = m_Him.m_vPos;
			RZ_ContractHelp.AddNear(ids, players, m_vPos, 100, m_aParticipants);
			if (m_Him.IsNear(m_vDest, 30))
			{
				m_Him.Arrive();
				Resolve(gm, m_Him.GetFollowed(), RZ_EQuestApproach.TALK, RZ_Quest_AwaitRelief.OUT_ARRESTED, "walked him in under arrest");
			}

			return;
		}

		// Fight for him: the squad is dead, and whoever was close did it
		if (!m_Squad.IsDead(dt))
			return;

		array<int> near = {};
		GetPlayersNear(ids, players, m_vPost, 150, near);
		if (near.IsEmpty())
			return;

		foreach (int playerId : near)
		{
			Remember(gm, playerId, RZ_EQuestApproach.FORCE, "fought the stronghold's squad for him");
			RZ_QuestApproach.SideWith(gm, playerId, -1, RZ_EFaction.MILITARY, 0, 15);
		}

		m_bFought = true;
		NotifyParticipants(gm, m_sTitle, QFill(gm, "It's over quickly. {cmd} looks at the bodies for a long time. \"Then I'm not going anywhere.\""));
		Resolve(gm, near[0], RZ_EQuestApproach.FORCE, RZ_Quest_AwaitRelief.OUT_FOUGHT, "fought the stronghold's squad for him", false);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (m_bArrested)
			return;

		if (objIdx == OBJ_HIM && option == 0)
		{
			m_bArrested = true;
			m_Him.Follow(playerId);
			m_Him.Go();
			Remember(gm, playerId, RZ_EQuestApproach.TALK, "took him in themselves");
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "He hands you his rifle, stock first. The sergeant nods and lets you lead. \"Your prisoner, then.\" Walk {cmd} to {stronghold}."));
			return;
		}

		if (objIdx != OBJ_SERGEANT)
			return;

		if (option == 0)
		{
			if (TryPersuade(gm, playerId, RZ_EFaction.MILITARY, 3, "soldier"))
			{
				gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "The sergeant hands you his handset. You tell {co} what the page said, and that {cmd} only said it out loud. A long silence. Then: \"Squad, return to base. {cmd} stays. He answers to me.\""));
				Resolve(gm, playerId, RZ_EQuestApproach.PERSUADE, RZ_Quest_AwaitRelief.OUT_TALKED, "talked the commanding officer down");
				return;
			}

			Remember(gm, playerId, RZ_EQuestApproach.PERSUADE, "tried to talk the commanding officer down");
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{co} listens, then says: \"Noted. Carry on, sergeant.\" The handset goes back on the sergeant's belt."));
			return;
		}

		if (option == 1)
		{
			m_fRewardScale = 0.5;
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You step aside. They're not rough about it. {cmd} walks between two of them and doesn't look back at his post."));
			Resolve(gm, playerId, RZ_EQuestApproach.NONE, RZ_Quest_AwaitRelief.OUT_TAKEN, "stood aside");
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_BastionQuest.Release(gm, m_iCmdId, m_iId);
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailed(RZ_GameModeComponent gm)
	{
		RZ_BastionQuest.Release(gm, m_iCmdId, m_iId);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "Await Relief: it's settled at {post}.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (m_bFought)
			return QFill(gm, "Await Relief: the stronghold's squad is dead at {post}. {co} will want to know why.");

		return QFill(gm, "Await Relief: the squad took {cmd} without you.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Him.Cleanup();
		m_Squad.Delete();
		RZ_BastionQuest.Release(gm, m_iCmdId, m_iId);
	}
}

//------------------------------------------------------------------------------------------------
//! Await Relief, stage 5 of the "open channel" branch (Bastion contract from the commanding
//! officer, Known): the stronghold blames the operator for the leak, and he has run. Find him
//! outside a town 1-3 km from the post, and bring him in (he walks with you to the stronghold) or
//! walk him to a safehouse instead (it gains one; the Haven thanks you; Bastion pays nothing).
class RZ_QM_OrdersOperator : RZ_QuestMission
{
	protected ref RZ_Escort m_Him = new RZ_Escort();
	protected vector m_vHide;
	protected vector m_vStronghold;
	protected vector m_vHouse;
	protected int m_iHouse = -1;
	protected string m_sHouse;
	protected string m_sTown;
	protected bool m_bFound;
	protected int m_iMode;

	protected static const int OBJ_HIM = 0;
	protected static const int MODE_NONE = 0;
	protected static const int MODE_IN = 1;
	protected static const int MODE_HOUSE = 2;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 10, 12);
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
		RZ_Leader co = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY);
		if (co)
			m_iLeaderId = co.m_iId;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		RZ_Leader co = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY);
		if (!run || !co || !RZ_Escort.HasRoom(gm))
			return false;

		vector post = run.m_vPlace;
		RZ_Leader seat = RZ_BastionQuest.Leader(gm, run.Flag("cmd", -1));
		if (seat)
			post = seat.m_vPos;

		m_iSettlement = PickSettlementAway(gm, post, 1000, 3000, 0, 90, gm.FindNearestSettlement(post));
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vHide = LandPointNear(s.m_vCenter, s.m_fRadius + 80, s.m_fRadius + 300);
		m_iHouse = RZ_LeaderJob.SafehouseNear(gm, m_vHide, 300, 6000, -1);
		RZ_Safehouse h = RZ_Questline.GetHouse(gm, m_iHouse);
		if (!h)
			return false;

		m_vHouse = h.m_vPos;
		m_sHouse = h.m_sName;
		m_sTown = s.m_sName;
		m_vStronghold = co.m_vPos;

		int talk = AddClue(QText(gm, "operator"), QFill(gm, "{operator} has a blanket round his shoulders and no rifle. \"They think I gave it to the Red Hand.\" He looks at you for a long moment. \"I didn't. You know who did.\""));
		string labels = QFill(gm, string.Format("Bring him in (walk him to {stronghold})|Walk him to the safehouse in %1 instead", m_sHouse));
		m_Him.m_sName = QText(gm, "operator");
		m_Him.m_sPrefab = RZ_Escort.PrefabFor(gm, 2);
		m_Him.m_bHide = false;
		m_Him.SetTalk(m_iId, OBJ_HIM, labels, talk, "Talk to " + QText(gm, "operator"));

		m_vPos = m_vHide;
		m_sTitle = "Await Relief: the leak";
		m_sBrief = QFill(gm, string.Format("{operator}, the operator from {post}, leaked classified traffic to the Red Hand and deserted. He was seen outside %1. Bring him in.", m_sTown));
		return m_Him.Begin(gm, m_vHide);
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		result.Insert(m_vHide);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = m_sTown + ", " + RZ_Util.DirectionText(from, m_vHide);
		if (m_iMode == MODE_IN)
			text += QFill(gm, "\n{stronghold}: ") + RZ_Util.DirectionText(from, m_vStronghold);
		else if (m_iMode == MODE_HOUSE)
			text += "\nThe safehouse in " + m_sHouse + ": " + RZ_Util.DirectionText(from, m_vHouse);

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved)
			return;

		m_Him.Update(gm, dt, ids, players);
		m_vPos = m_Him.m_vPos;
		if (!m_bFound && ReachedBy(ids, players, m_Him.m_vPos, 25))
		{
			m_bFound = true;
			RZ_ContractHelp.NotifyNear(gm, m_Him.m_vPos, 40, m_sTitle, QFill(gm, "A man in Bastion fatigues under a blanket, sitting very still. It's {operator}. Talk to him."));
		}

		if (m_Him.IsDead())
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (m_iMode == MODE_NONE)
			return;

		RZ_ContractHelp.AddNear(ids, players, m_vPos, 100, m_aParticipants);
		if (m_iMode == MODE_IN && m_Him.IsNear(m_vStronghold, 30))
		{
			m_Him.Arrive();
			Resolve(gm, m_Him.GetFollowed(), RZ_EQuestApproach.TALK, RZ_Quest_AwaitRelief.OUT_BROUGHT, "brought the operator in");
			return;
		}

		if (m_iMode == MODE_HOUSE && m_Him.IsNear(m_vHouse, 25))
		{
			m_Him.Arrive();
			int who = m_Him.GetFollowed();
			RZ_Questline.ChangePopulation(gm, m_iHouse, 1);
			RZ_QuestRun run = Run(gm);
			if (run)
				run.Reward(gm, who, RZ_EFaction.SURVIVORS, 5, 0, "The Haven took him in and asked him nothing.");

			Resolve(gm, who, RZ_EQuestApproach.SIDE, RZ_Quest_AwaitRelief.OUT_HIDDEN, "walked the operator to a safehouse", false);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_HIM || m_iMode != MODE_NONE)
			return;

		m_bFound = true;
		m_Him.Follow(playerId);
		m_Him.Go();
		if (option == 0)
		{
			m_iMode = MODE_IN;
			Remember(gm, playerId, RZ_EQuestApproach.TALK, "brought the operator in");
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "He gets up without a word and falls in behind you. Walk him to {stronghold}."));
			return;
		}

		m_iMode = MODE_HOUSE;
		Remember(gm, playerId, RZ_EQuestApproach.SIDE, "hid the operator at a safehouse");
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("He stares at you. Then he folds the blanket, very neatly, and follows. The safehouse in %1 (%2).", m_sHouse, RZ_Util.DirectionText(m_Him.m_vPos, m_vHouse)));
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "Await Relief: {operator} is at {stronghold}. The log calls it a leak.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (m_iMode == MODE_HOUSE && m_bResolved)
			return QFill(gm, "Await Relief: {operator} sleeps at a Haven safehouse now. Bastion paid nothing for that.");

		return QFill(gm, "Await Relief: Bastion's own people found {operator} first.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Him.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! Await Relief, stage 5 of the Haven's branch (Bastion contract, Known): the chair's price for
//! keeping quiet. A crate of rations at the commander's post (the stronghold if he's gone), carried
//! to the chair's clerk at her safehouse: +4 food there.
class RZ_QM_OrdersPrice : RZ_QuestMission
{
	protected ref RZ_QuestCarry m_Crate = new RZ_QuestCarry();
	protected vector m_vCrate;
	protected vector m_vClerk;
	protected int m_iHouse = -1;
	protected string m_sFrom;
	protected bool m_bClerkShown;

	protected static const int OBJ_CLERK = 0;
	protected static const int FOOD = 4;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 8, 10);
		m_fTimeLeft = 3000;
		m_fParticipationRadius = 0;
		m_iLeaderId = RZ_Quest_AwaitRelief.Voice(gm, Run(gm));
		if (m_iLeaderId == RZ_BastionQuest.NO_VOICE)
		{
			RZ_Leader co = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY);
			if (co)
				m_iLeaderId = co.m_iId;
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		m_iHouse = RZ_BastionQuest.ChairHouse(gm);
		RZ_Safehouse h = RZ_Questline.GetHouse(gm, m_iHouse);
		if (!run || !h)
			return false;

		RZ_Leader giver = RZ_BastionQuest.Kept(gm, run, "cmd");
		if (!giver)
			giver = RZ_BastionQuest.Stronghold(gm, RZ_EFaction.MILITARY);

		if (!giver)
			return false;

		m_vCrate = LandPointNear(giver.m_vPos, 4, 8);
		m_sFrom = giver.m_sPlace;
		m_vClerk = LandPointNear(h.m_vPos, 4, 9);
		m_iSettlement = gm.FindNearestSettlement(m_vCrate);
		m_vPos = m_vCrate;
		m_sTitle = "Await Relief: the chair's price";
		m_sBrief = QFill(gm, string.Format("The Haven's chair has something of ours, and her price for keeping it in a drawer is rations. A crate is waiting at %1. Take it to her house in %2. Don't sign for it.", m_sFrom, h.m_sName));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		if (m_Crate.IsOnGround())
			result.Insert(m_Crate.GetPos());

		result.Insert(m_vClerk);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return string.Format("The crate, %1: %2\nThe chair's house: %3", m_sFrom, RZ_Util.DirectionText(from, m_vCrate), RZ_Util.DirectionText(from, m_vClerk));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bResolved)
			return;

		if (!m_Crate.IsSpawned() && RZ_ContractHelp.AnyNear(players, m_vCrate, 300))
		{
			if (!m_Crate.Spawn(gm, m_iId, m_vCrate))
			{
				m_eState = RZ_EMissionState.FAILED;
				return;
			}
		}

		if (!m_Crate.Update(gm, this))
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (m_Crate.IsSpawned() && !m_Crate.IsOnGround())
			m_vPos = m_vClerk;

		if (!m_bClerkShown && RZ_ContractHelp.AnyNear(players, m_vClerk, 300))
		{
			m_bClerkShown = true;
			int clue = AddClue("The chair's clerk", "\"Bastion's rations? The chair said they'd come. She didn't say when.\" He holds the door.");
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vClerk, OBJ_CLERK, "Hand over Bastion's rations", "", clue, "Talk to the chair's clerk");
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx != OBJ_CLERK)
			return;

		if (!m_Crate.TakeFrom(user))
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("You haven't got Bastion's crate. It's waiting at %1.", m_sFrom));
			return;
		}

		RZ_Questline.ChangeFood(gm, m_iHouse, FOOD);
		gm.NotifyPlayer(playerId, m_sTitle, "The clerk counts the tins, writes the number in a book, and carries the crate inside. Nobody mentions the page.");
		Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_AwaitRelief.OUT_PAID, "carried Bastion's rations to the chair");
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "Await Relief: the chair's price is paid, this time.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Await Relief: Bastion's rations never reached the chair.";
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Crate.Cleanup();
	}
}
