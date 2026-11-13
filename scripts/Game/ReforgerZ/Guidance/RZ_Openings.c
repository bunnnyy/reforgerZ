//! Openings: how every life starts (docs/DESIGN.md, "Openings").
//!
//! Each life starts at a random place that fits its background (RZ_OpeningRoll, rolled when the
//! character spawns), coming out of a door for a reason (RZ_EOpeningReason: the hiding place, your
//! group gone, the fever broken). By the door, the reason's scene, your background's own things,
//! something to read and a clue (RZ_OpeningScenes). A few metres off, whoever's outside
//! (RZ_EOpeningWho: someone from before, a stranger, or a wanderer who met an earlier life of
//! yours) calls you over (a subtitle with their name), and "Talk to <name>" is tracked straight away
//! (the HUD and the map's diamond on them). The talk screen opens only when you talk to them: each
//! step's piece of the story and its ask come in that conversation, and the step is handed there
//! (between steps, "Talk to <name>" again). With nobody outside, or once they're dead, a step
//! starts by itself, tracked: the only job in the game that does. The chain
//! (RZ_OpeningChain) is errand steps by your guidance level: close by and quiet, your background's
//! own system (RZ_OpeningSteps.c), and the walk to where your background belongs, which pays a
//! little Fame with that faction (levels 0-1); your background's step, then the nearest place with
//! work (level 2); the nearest place with work only, and someone outside one time in three
//! (level 3). With nobody outside, the scene's read points you at the work.
//!
//! It replaces "a wanderer finds you" for a life with someone outside; the gentle start, the
//! wind-up radio, the tips and the guidance levels stay. Not in the epilogue week (the place is
//! still rolled). One log line when it starts, per step, and when it ends. Server only.

//------------------------------------------------------------------------------------------------
//! The stages a chain is made of, in its plan (RZ_Opening.m_aPlan).
enum RZ_EOpeningStage
{
	NONE,
	SEARCH,		// step 1: a building close by, quietly
	TASK,		// step 2: your background's own system (the Drifter: up to three asks)
	WALK,		// step 3: to where your background belongs
	LEAD		// the last step for levels 2-3 and a contact-less opening: to the nearest place with work
}

//------------------------------------------------------------------------------------------------
//! Who's outside the door (the conversations' opening.who).
enum RZ_EOpeningWho
{
	NOBODY,
	BEFORE,		// someone from before the outbreak, by your background
	STRANGER,	// a stranger: who heard the door, who saw them go, who wrote on the door
	EARLIER		// a wanderer who met you in an earlier life: "You look like someone I knew."
}

//------------------------------------------------------------------------------------------------
//! One life's opening. Runtime only: a restart or a death ends it.
class RZ_Opening
{
	int m_iId;
	int m_iPlayer;
	int m_iLife;
	string m_sBackground;
	int m_iLevel;
	bool m_bFull;				// its plan ends with the walk (levels 0-1): it replaces "Find the Haven"
	ref RZ_OpeningPlace m_Place;
	string m_sTown;
	int m_iReason;				// why today (RZ_EOpeningReason)
	int m_iScene;				// the reason's composition (1-3)
	int m_iWho;					// who's outside (RZ_EOpeningWho)
	int m_iWanderer = -1;		// EARLIER: the wanderer they are, held while they stand here
	IEntity m_Contact;			// weak: whoever's outside (walking with you: the escort's person; handed on: the next person)
	ref RZ_Escort m_Escort;		// they walk with you (the escort pattern), null otherwise
	bool m_bWalks;				// the escort pattern: they come with you, step to step
	bool m_bHandOff;			// the hand-off pattern: step 1's giver sends you on to the next person
	IEntity m_Left;				// weak: the giver you were sent on from, gone once nobody's near
	int m_iLeftWanderer = -1;	// an earlier life's wanderer the first giver was, back on the road with them
	vector m_vNext;				// the next person's spot (hand-off), zero once they stand there
	string m_sNextName;
	string m_sNextShort;
	string m_sNextWho;
	string m_sNextPlace;		// "Ruth's house in Morton"
	string m_sLeftShort;		// the first giver's short name, once you're sent on
	string m_sLeftSpeaker;		// and their title
	bool m_bNextCalled;			// the next person called you over as you came up
	float m_fWaitNear;			// walking with you: the next step waits since then for them to catch up
	bool m_bHadContact;
	bool m_bContactDead;
	string m_sName;				// "Vera Holm"
	string m_sShort;			// "Vera" ("Old Marta" for a wanderer)
	string m_sWho;				// "a patient's mother"
	string m_sSpeaker;			// the title their lines come under; whoever finds you, once they're dead
	int m_iEnd = -1;			// the faction the chain ends with
	vector m_vEnd;
	string m_sEndPlace;			// "the safehouse in Morton"
	ref array<int> m_aPlan = {};	// the stages, in order (RZ_EOpeningStage)
	int m_iStage = -1;			// the plan's stage under way (-1 before they call you over)
	int m_iStep;				// 0 before the first step, 1-3 the step under way (the conversations' opening.step)
	bool m_bNext;				// the next stage starts at m_fNextAt
	string m_sSearch;			// step 1's place: "the chemist's down the road"
	int m_iTask;				// step 2's task (RZ_EOpeningTask)
	ref array<int> m_aTasks = {};	// step 2's tasks still to try (the background's, or the Drifter's asks)
	int m_iAsks;				// the Drifter's asks done
	IEntity m_Target;			// weak: the task's building (KEYS)
	IEntity m_Remains;			// weak: SAMPLE's remains, kept from the corpse cleanup until the opening ends
	int m_iTargetId = -1;		// the task's wanderer (SWAP) or road post (TOLL)
	float m_fPulseAt;			// GATE: the step's progress at its next noise
	bool m_bTold;				// the task said once what's missing
	bool m_bTended;				// CUT, FEVER, CLOCK, CLOCK_MED: the hand-over reply was picked
	int m_iErrand = -1;			// the current step's errand id
	bool m_bResolved;			// the current step is done
	bool m_bSpoke;				// they've called you over (the chain has begun)
	bool m_bAskedAgain;
	bool m_bWaitTalk;			// the plan's stage under way waits for you to talk to them
	float m_fWaitAt;			// when that wait began
	bool m_bCalledAgain;		// they called once more, two minutes into the wait
	bool m_bEnded;
	float m_fBorn;
	float m_fStepAt;
	float m_fNextAt;
}

//------------------------------------------------------------------------------------------------
class RZ_Openings
{
	static const int TAG = -17;					// RZ_PuzzleObjectComponent / RZ_ClueComponent mission id: a contact
	static const string FLAG_STEP = "opening.step";	// dialogue's character flag: the step under way (1-3), 4 when it's over
	static const string FLAG_END = "opening.end";		// the ending faction + 1 (1 the Haven ... 4 Halcyon)
	static const string FLAG_TASK = "opening.task";		// step 2's task (RZ_EOpeningTask)
	static const string FLAG_WHY = "opening.why";		// why today (RZ_EOpeningReason: 1 hiding, 2 group gone, 3 fever)
	static const string FLAG_WHO = "opening.who";		// who's outside (RZ_EOpeningWho: 1 from before, 2 stranger, 3 an earlier life's)
	static const string FLAG_EARLIER = "opening.earlier";	// an earlier life's: what they remember (1 helped, 2 wronged, 3 only met)
	static const string FLAG_FIRST = "opening.first";	// 1 while the first step runs (their greeting)

	protected static const float POLL = 1;
	protected static const float SPEAK_DELAY = 4;		// seconds after the spawn before they call you over
	protected static const float SPEAK_RANGE = 30;		// they call again (and nudge) only this close
	protected static const float CONTACT_OUT = 6;		// the contact stands this far out of the door
	protected static const float NEXT_STEP_DELAY = 4;
	protected static const float ASK_AGAIN = 120;		// called again once, this long into a wait or into step 1
	protected static const float CONTACT_GONE = 150;	// the contact goes once nobody is this close (the walk, or over)
	protected static const float CONTACT_MAX = 2700;	// and once the opening is 45 minutes old, with nobody within 50 m
	protected static const float SEARCH_MIN = 150;		// step 1's house: this far from the contact...
	protected static const float SEARCH_MAX = 450;		// ...and no further
	protected static const float NEXT_MIN = 200;		// the next person (hand-off): this far on from step 1's place...
	protected static const float NEXT_MAX = 600;		// ...and no further
	protected static const float NEXT_FROM_FIRST = 150;	// and at least this far from the first giver
	protected static const float CATCH_UP = 60;		// walking with you: the next step waits this long for them to catch up
	protected static const float NEXT_CALL = 25;		// the next person calls you over this close
	protected static const int SEARCH_TRIES = 6;		// buildings tried before giving up on a kind
	protected static const float SEARCH_OUT = 3;		// the spot: this far out of the building's side facing the contact
	protected static const float STRANGER_CHANCE = 0.25;	// a stranger instead of someone from before
	protected static const float EARLIER_CHANCE = 0.35;	// an earlier life's wanderer, when one remembers you
	protected static const float VETERAN_CONTACT = 0.34;	// a veteran meets someone one time in three
	protected static const int HOLD_BASE = 800000;		// RZ_Wanderers.Hold's id for an opening (+ its id)

	protected RZ_GameModeComponent m_GM;
	protected RZ_Guidance m_Guide;
	protected RZ_GuidanceSettings m_Settings;
	protected ref RZ_OpeningRoll m_Roll;
	protected ref RZ_OpeningChain m_Chain;
	protected ref RZ_OpeningTasks m_Tasks;
	protected ref RZ_OpeningScenes m_Scenes;
	protected ref array<ref RZ_Opening> m_aOpenings = {};
	protected ref map<int, ref RZ_OpeningPlace> m_mPending = new map<int, ref RZ_OpeningPlace>();
	protected ref map<int, bool> m_mMoved = new map<int, bool>();
	protected ref array<int> m_aIds = {};
	protected ref array<IEntity> m_aPlayers = {};
	protected int m_iNextId = 1;
	protected float m_fClock;
	protected float m_fPoll;

	//------------------------------------------------------------------------------------------------
	void RZ_Openings(RZ_GameModeComponent gm, RZ_Guidance guide, RZ_GuidanceSettings settings)
	{
		m_GM = gm;
		m_Guide = guide;
		m_Settings = settings;
		m_Roll = new RZ_OpeningRoll(gm);
		m_Chain = new RZ_OpeningChain();
		m_Tasks = new RZ_OpeningTasks(gm);
		m_Scenes = new RZ_OpeningScenes(gm, guide, settings);
	}

	//------------------------------------------------------------------------------------------------
	static RZ_Openings Get()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetGuidance())
			return null;

		return gm.GetGuidance().GetOpenings();
	}

	//------------------------------------------------------------------------------------------------
	//! Guidance > Openings on (and guidance itself).
	bool IsOn()
	{
		return m_Settings.m_bEnabled && m_Settings.m_bOpenings;
	}

	//------------------------------------------------------------------------------------------------
	//! This player's opening's chain is still running (the errands wave nobody over meanwhile).
	static bool IsRunningFor(int playerId)
	{
		RZ_Openings openings = Get();
		return openings && openings.Running(playerId) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! This life has an opening with the whole chain: its walk replaces "Find the Haven".
	bool FullChainFor(string ident)
	{
		foreach (RZ_Opening o : m_aOpenings)
		{
			if (!o.m_bEnded && o.m_bFull && m_GM.GetProfile(o.m_iPlayer).m_sIdentity == ident)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Opening Running(int playerId)
	{
		foreach (RZ_Opening o : m_aOpenings)
		{
			if (o.m_iPlayer == playerId && !o.m_bEnded)
				return o;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! The title a tagged body speaks under: the opening's contact, or (a negated id) the first
	//! giver who sent you on; "" if neither.
	string SpeakerFor(int id)
	{
		if (id < 0)
		{
			RZ_Opening left = ById(-id);
			if (left)
				return left.m_sLeftSpeaker;

			return "";
		}

		RZ_Opening o = ById(id);
		if (o)
			return o.m_sSpeaker;

		return "";
	}

	//------------------------------------------------------------------------------------------------
	RZ_Opening ById(int id)
	{
		foreach (RZ_Opening o : m_aOpenings)
		{
			if (o.m_iId == id)
				return o;
		}

		return null;
	}

	// ================================================================ The spawn
	//------------------------------------------------------------------------------------------------
	//! From the spawn point handler (RZ_OpeningPlaces.c), before the character is handed over: the
	//! place is rolled for the background the player picked, and the character moved there.
	void PlaceSpawn(int playerId, IEntity character)
	{
		m_mPending.Remove(playerId);
		m_mMoved.Remove(playerId);
		if (!IsOn() || playerId <= 0 || !character)
			return;

		RZ_Background bg = m_GM.LifeBackground(playerId);
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		int level = m_Guide.GetLevel(profile);
		array<vector> ends = {};
		EndPoints(EndFactionFor(playerId, bg.m_sId), ends);
		RZ_OpeningPlace place = m_Roll.Roll(bg.m_sId, level, profile, ends);
		if (place)
		{
			RZ_OpeningRoll.Move(character, place);
			m_mMoved.Set(playerId, true);
			RZ_PlaceNeeds.Verify("opening.start", place.m_vPos, bg.m_sId + ": " + place.m_sKind, place.m_iTown);
			if (!place.m_bFits)
				RZ_Log.Info(string.Format("Openings: no %1 place fits for %2, a house instead", bg.m_sId, RZ_Util.PlayerName(playerId)));
		}
		else
		{
			// The fallback's end: the spawn point the spawn screen asked for, and the contact only
			place = new RZ_OpeningPlace();
			place.m_vPos = character.GetOrigin();
			place.m_iTown = m_GM.FindNearestSettlement(place.m_vPos);
			RZ_Log.Info(string.Format("Openings: no place for %1 (%2), the spawn point instead: %3", RZ_Util.PlayerName(playerId), bg.m_sId, m_Roll.WhyText()));
		}

		m_mPending.Set(playerId, place);
	}

	//------------------------------------------------------------------------------------------------
	//! The spawn handler moved this player's new character (it preloads the world there).
	bool Moved(int playerId)
	{
		return m_mMoved.Contains(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! From the game mode's spawn, after the guidance's new life and before the objective is picked:
	//! the scene, the contact and the timers. Nothing without a place rolled for this spawn.
	void OnSpawned(int playerId, IEntity ent, RZ_PlayerProfile profile, RZ_Background bg)
	{
		EndFor(playerId, "a new life");
		RZ_OpeningPlace place;
		m_mPending.Find(playerId, place);
		m_mPending.Remove(playerId);
		m_mMoved.Remove(playerId);
		if (!IsOn() || !ent || !place)
			return;

		RZ_OpeningRoll.NoteStart(profile, ent.GetOrigin());
		RZ_WorldFacts facts = m_GM.GetFacts();
		if (facts && facts.IsEpilogue())
			return;

		RZ_Opening o = new RZ_Opening();
		o.m_iId = m_iNextId;
		m_iNextId++;
		o.m_iPlayer = playerId;
		o.m_iLife = profile.m_iLives;
		o.m_sBackground = bg.m_sId;
		o.m_iLevel = m_Guide.GetLevel(profile);
		o.m_Place = place;
		o.m_fBorn = m_fClock;
		RZ_Settlement town = m_GM.GetSettlement(place.m_iTown);
		if (!town)
			town = m_GM.GetSettlement(m_GM.FindNearestSettlement(ent.GetOrigin()));

		o.m_sTown = "the next town";
		if (town)
			o.m_sTown = town.m_sName;

		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		o.m_iReason = RollReason(o.m_iLevel, rng);
		Plan(o, rng);
		o.m_iEnd = EndFactionFor(playerId, bg.m_sId);
		FindEnd(o, ent.GetOrigin());

		// Who's outside: a veteran meets someone one time in three. Nobody can stand (no body prefab,
		// the story budget full): the scene's read points at the nearest work instead
		bool wanted = o.m_iLevel < RZ_Guidance.LEVEL_VETERAN || rng.Chance(VETERAN_CONTACT);
		if (wanted)
		{
			PickWho(o, profile, rng);
			SpawnContact(o, ent.GetOrigin());
		}

		if (o.m_bHadContact && o.m_Contact)
		{
			RZ_PlaceNeeds.VerifyThings("opening.start", place.m_vPos, o.m_sName);
			Pattern(o);
		}

		if (!o.m_bHadContact)
		{
			ReleaseWanderer(o, ent.GetOrigin());
			o.m_iWho = RZ_EOpeningWho.NOBODY;
			o.m_sName = RZ_OpeningText.ReadTitle(o.m_iReason);
			o.m_sShort = o.m_sName;
			o.m_sSpeaker = o.m_sName;
			NoContact(o);
			if (wanted)
				RZ_Log.Info(string.Format("Openings: no contact could stand for %1 at %2, the scene's read instead", RZ_Util.PlayerName(playerId), RZ_SettlementScanner.GridReference(ent.GetOrigin())));
		}

		int day = 1;
		if (m_GM.GetBroadcast())
			day = m_GM.GetBroadcast().GetDay();

		if (place.m_bScene)
			m_Scenes.Place(o, day, profile, rng);

		// Someone there: no wanderer comes looking
		if (o.m_bHadContact)
			m_Guide.MarkOpening(playerId);

		m_Guide.SetSpawnLine(playerId, RZ_OpeningText.SpawnLine(o.m_iReason, place.m_sKind, place.m_bScene, o.m_sTown, day));
		SetFlag(o, FLAG_WHY, o.m_iReason);
		SetFlag(o, FLAG_WHO, o.m_iWho);
		SetFlag(o, FLAG_FIRST, 0);
		SetFlag(o, FLAG_STEP, 0);
		o.m_fNextAt = m_fClock + SPEAK_DELAY;
		m_aOpenings.Insert(o);

		string kind = place.m_sKind;
		if (kind.IsEmpty())
			kind = "spawn point";

		string who = typename.EnumToString(RZ_EOpeningWho, o.m_iWho) + ": " + o.m_sWho;
		who.ToLower();
		string chain = PlanText(o);
		if (o.m_bFull)
			chain += " to " + o.m_sEndPlace;

		if (o.m_bWalks)
			chain += ", they walk with you";
		else if (o.m_bHandOff)
			chain += ", handed on";

		RZ_Log.Info(string.Format("Openings: started for %1: %2, %3 in %4 (%5), level %6, %7, %8, chain %9",
			RZ_Util.PlayerName(playerId), bg.m_sId, kind, o.m_sTown, RZ_SettlementScanner.GridReference(ent.GetOrigin()), o.m_iLevel, RZ_OpeningScenes.ReasonKey(o.m_iReason), who, chain));
	}

	//------------------------------------------------------------------------------------------------
	//! Why today: your group gone (Guidance > Group gone weight, 18) or the fever broken (Fever
	//! weight, 12), else the hiding place; a brand-new player's two weights are halved, so they lean
	//! to the hiding place.
	protected int RollReason(int level, RZ_Rng rng)
	{
		int group = Math.Max(0, m_Settings.m_iGroupGoneWeight);
		int fever = Math.Max(0, m_Settings.m_iFeverWeight);
		if (level <= RZ_Guidance.LEVEL_NEW)
		{
			group = group / 2;
			fever = fever / 2;
		}

		int roll = rng.Int(0, 100);
		if (roll < group)
			return RZ_EOpeningReason.GROUP;

		if (roll < group + fever)
			return RZ_EOpeningReason.FEVER;

		return RZ_EOpeningReason.HIDING;
	}

	//------------------------------------------------------------------------------------------------
	//! The chain's plan by guidance level, with errands on: 0-1 all three steps (level 1's step 1
	//! shorter); 2 your background's step, then the nearest place with work; 3 that place only. Step
	//! 2's tasks to try: the background's own, or the Drifter's asks.
	protected void Plan(RZ_Opening o, RZ_Rng rng)
	{
		RZ_Errands errands = m_GM.GetErrands();
		if (errands && errands.IsOn())
		{
			if (o.m_iLevel <= 1)
			{
				o.m_aPlan.Insert(RZ_EOpeningStage.SEARCH);
				o.m_aPlan.Insert(RZ_EOpeningStage.TASK);
				o.m_aPlan.Insert(RZ_EOpeningStage.WALK);
			}
			else if (o.m_iLevel == 2)
			{
				o.m_aPlan.Insert(RZ_EOpeningStage.TASK);
				o.m_aPlan.Insert(RZ_EOpeningStage.LEAD);
			}
			else
			{
				o.m_aPlan.Insert(RZ_EOpeningStage.LEAD);
			}
		}

		o.m_bFull = o.m_aPlan.Contains(RZ_EOpeningStage.WALK);
		if (o.m_sBackground == "drifter")
			RZ_OpeningTasks.DrifterPool(rng, o.m_aTasks);
		else
			RZ_OpeningTasks.Preferred(o.m_sBackground, o.m_aTasks);
	}

	//------------------------------------------------------------------------------------------------
	//! How the chain moves you on (docs/DESIGN.md, "Openings"): the contact walks with you, step to
	//! step (the escort pattern: a background whose steps are out and about, with Guidance >
	//! Companion and room for an escort); or step 1's giver sends you on to the next person, at the
	//! next place, who gives step 2 (the hand-off: a plan with both steps). Else (levels 2-3) each
	//! step starts by itself where the last one ended.
	protected void Pattern(RZ_Opening o)
	{
		o.m_bWalks = RZ_OpeningTasks.Walks(o.m_sBackground) && o.m_aPlan.Count() > 1 && !m_Settings.m_sCompanionPrefab.IsEmpty() && RZ_Escort.HasRoom(m_GM);
		o.m_bHandOff = !o.m_bWalks && o.m_aPlan.Contains(RZ_EOpeningStage.SEARCH) && o.m_aPlan.Contains(RZ_EOpeningStage.TASK);
	}

	//------------------------------------------------------------------------------------------------
	//! Nobody outside: only the nearest place with work, from the scene's read (errands on).
	protected void NoContact(RZ_Opening o)
	{
		o.m_aPlan.Clear();
		RZ_Errands errands = m_GM.GetErrands();
		if (errands && errands.IsOn())
			o.m_aPlan.Insert(RZ_EOpeningStage.LEAD);

		o.m_bFull = false;
	}

	//------------------------------------------------------------------------------------------------
	//! Who's outside: a wanderer who met an earlier life of yours, when one does (35%: they're held
	//! off the road while they stand here); else a stranger one time in four; else someone from
	//! before, by your background.
	protected void PickWho(RZ_Opening o, RZ_PlayerProfile profile, RZ_Rng rng)
	{
		RZ_Wanderers wanderers = m_GM.GetWanderers();
		int earlier = EarlierWanderer(profile);
		if (earlier >= 0 && rng.Chance(EARLIER_CHANCE))
		{
			RZ_Wanderer w = wanderers.FindById(earlier);
			o.m_iWho = RZ_EOpeningWho.EARLIER;
			o.m_iWanderer = earlier;
			o.m_sName = w.GetShortName();
			o.m_sShort = o.m_sName;
			o.m_sWho = RZ_OpeningText.EarlierWho(w.GetRoleName());
			o.m_sSpeaker = o.m_sName + ", " + o.m_sWho;
			int memory = w.OldMemory(profile.m_sIdentity);
			int remembers = 3;
			if ((memory & RZ_Wanderers.MEM_WRONGED) != 0)
				remembers = 2;
			else if ((memory & (RZ_Wanderers.MEM_HELPED | RZ_Wanderers.MEM_SAVED)) != 0)
				remembers = 1;

			SetFlag(o, FLAG_EARLIER, remembers);
			wanderers.Hold(earlier, HOLD_BASE + o.m_iId);
			return;
		}

		SetFlag(o, FLAG_EARLIER, 0);
		o.m_sName = RZ_OpeningText.Name(rng);
		o.m_sShort = o.m_sName;
		int space = o.m_sName.IndexOf(" ");
		if (space > 0)
			o.m_sShort = o.m_sName.Substring(0, space);

		if (rng.Chance(STRANGER_CHANCE))
		{
			o.m_iWho = RZ_EOpeningWho.STRANGER;
			o.m_sWho = RZ_OpeningText.StrangerWho(o.m_iReason);
		}
		else
		{
			o.m_iWho = RZ_EOpeningWho.BEFORE;
			o.m_sWho = RZ_OpeningText.Who(o.m_sBackground);
		}

		o.m_sSpeaker = o.m_sName + ", " + o.m_sWho;
	}

	//------------------------------------------------------------------------------------------------
	//! A living wanderer, free and off-screen, who remembers an earlier life of this identity and
	//! hasn't met this one (life-stamped memory, docs/REPUTATION.md); -1 if none.
	protected int EarlierWanderer(RZ_PlayerProfile profile)
	{
		RZ_Wanderers wanderers = m_GM.GetWanderers();
		if (!wanderers || profile.m_iLives <= 1)
			return -1;

		RZ_WorldFacts facts = m_GM.GetFacts();
		array<int> ids = {};
		wanderers.GetLiving(RZ_Wanderers.AllRoles(), ids);
		foreach (int id : ids)
		{
			RZ_Wanderer w = wanderers.FindById(id);
			if (!w || w.m_bHasBody || w.IsBitten())
				continue;

			if (facts && facts.IsClaimed(RZ_WorldFacts.Wanderer(id)))
				continue;

			if (w.OldMemory(profile.m_sIdentity) != 0 && w.GetMemory(profile.m_sIdentity) == 0)
				return id;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! "search, task, walk", for the log.
	protected static string PlanText(RZ_Opening o)
	{
		string text = "";
		foreach (int stage : o.m_aPlan)
		{
			if (!text.IsEmpty())
				text += ", ";

			text += typename.EnumToString(RZ_EOpeningStage, stage);
		}

		if (text.IsEmpty())
			return "none";

		text.ToLower();
		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! Whoever's outside, a few metres out of the door, facing you (Guidance > Contact; empty: the
	//! puzzle civilian). Counted as a story body, so the budget never refuses them.
	protected void SpawnContact(RZ_Opening o, vector player)
	{
		ResourceName prefab = m_Settings.m_sContactPrefab;
		if (prefab.IsEmpty())
			prefab = m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);

		RZ_Shared shared = m_GM.GetShared();
		if (prefab.IsEmpty() || !shared || !shared.CanSpawnBody(RZ_BodyBudget.STORY, player))
			return;

		vector at = RZ_Mission.LandPointNear(player, 4, 7);
		if (o.m_Place.m_bScene)
		{
			vector outside = RZ_Util.GroundPos(player + o.m_Place.m_vOut * CONTACT_OUT);
			if (!RZ_Util.IsWater(outside))
				at = outside;
		}

		IEntity body = RZ_Util.SpawnPrefab(prefab, at, RZ_Compass.Bearing(at, player));
		if (!body)
			return;

		shared.AddBody(RZ_BodyBudget.STORY, body);
		o.m_Contact = body;
		o.m_bHadContact = true;
		RZ_PuzzleObjectComponent.Tag(body, TAG, o.m_iId, "", "What was it you needed?");
		RZ_ClueComponent.Tag(body, TAG, o.m_iId, "Talk to " + o.m_sShort);
	}

	// ================================================================ Where the chain ends
	//------------------------------------------------------------------------------------------------
	//! Where the background belongs: the medical people Halcyon, trouble the Red Hand, the military
	//! and the law Bastion, everyone else the Haven.
	static int EndFaction(string bg)
	{
		switch (bg)
		{
			case "nurse": return RZ_EFaction.INSTITUTE;
			case "doctor": return RZ_EFaction.INSTITUTE;
			case "labtech": return RZ_EFaction.INSTITUTE;
			case "excon": return RZ_EFaction.BANDITS;
			case "raider": return RZ_EFaction.BANDITS;
			case "soldier": return RZ_EFaction.MILITARY;
			case "police": return RZ_EFaction.MILITARY;
		}

		return RZ_EFaction.SURVIVORS;
	}

	//------------------------------------------------------------------------------------------------
	//! Where this player's chain ends: the background's faction, or the Haven while that faction
	//! hunts them.
	int EndFactionFor(int playerId, string bg)
	{
		RZ_EFaction f = EndFaction(bg);
		if (f != RZ_EFaction.SURVIVORS && RZ_Reputation.IsHunted(m_GM.GetTier(playerId, f)))
			return RZ_EFaction.SURVIVORS;

		return f;
	}

	//------------------------------------------------------------------------------------------------
	//! The places a chain ending with this faction can walk to (the place roll tries towns near them
	//! first): its outposts and stronghold, Halcyon's lab too; the Haven's safehouses, and those
	//! whenever the faction holds nothing.
	void EndPoints(int faction, notnull array<vector> result)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		if (faction != RZ_EFaction.SURVIVORS && territory)
		{
			array<RZ_Holding> list = {};
			territory.GetOutposts(faction, list);
			RZ_Holding seat = territory.GetStronghold(faction);
			if (seat)
				list.Insert(seat);

			foreach (RZ_Holding h : list)
			{
				result.Insert(h.m_vPos);
			}
		}

		if (faction == RZ_EFaction.INSTITUTE && m_GM.HasLab())
			result.Insert(m_GM.GetLabPosition());

		if (!result.IsEmpty())
			return;

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses)
			return;

		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			if (h)
				result.Insert(h.m_vPos);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest place of the ending faction (a Haven safehouse, a Bastion or Red Hand holding,
	//! Halcyon's field station or the lab); the nearest safehouse when there's none, or when that
	//! faction hunts you; none at all, and the chain ends before the walk.
	protected void FindEnd(RZ_Opening o, vector from)
	{
		if (o.m_iEnd != RZ_EFaction.SURVIVORS && FindHolding(o, from))
			return;

		o.m_iEnd = RZ_EFaction.SURVIVORS;
		o.m_vEnd = vector.Zero;
		o.m_sEndPlace = "";
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses)
			return;

		RZ_Safehouse h = houses.Get(houses.FindNearest(from));
		if (!h)
			return;

		o.m_vEnd = h.m_vPos;
		o.m_sEndPlace = RZ_OpeningText.EndPlace(RZ_EFaction.SURVIVORS, h.m_sName, false);
	}

	//------------------------------------------------------------------------------------------------
	protected bool FindHolding(RZ_Opening o, vector from)
	{
		float best = float.MAX;
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory)
		{
			array<RZ_Holding> list = {};
			territory.GetOutposts(o.m_iEnd, list);
			RZ_Holding seat = territory.GetStronghold(o.m_iEnd);
			if (seat)
				list.Insert(seat);

			foreach (RZ_Holding h : list)
			{
				float d = RZ_Util.DistXZ(from, h.m_vPos);
				if (d < best)
				{
					best = d;
					o.m_vEnd = h.m_vPos;
					o.m_sEndPlace = RZ_OpeningText.EndPlace(o.m_iEnd, h.m_sName, false);
				}
			}
		}

		if (o.m_iEnd == RZ_EFaction.INSTITUTE && m_GM.HasLab() && RZ_Util.DistXZ(from, m_GM.GetLabPosition()) < best)
		{
			best = RZ_Util.DistXZ(from, m_GM.GetLabPosition());
			o.m_vEnd = m_GM.GetLabPosition();
			string labTown = "the lab";
			RZ_Settlement s = m_GM.GetSettlement(m_GM.GetLabSettlement());
			if (s)
				labTown = s.m_sName;

			o.m_sEndPlace = RZ_OpeningText.EndPlace(RZ_EFaction.INSTITUTE, labTown, true);
		}

		return best < float.MAX;
	}

	// ================================================================ The tick
	//------------------------------------------------------------------------------------------------
	//! A contact waiting for you (steps 1 and 2) keeps the infected off (RZ_ZombieSpawner.SetQuietSpots):
	//! none spawn near them and strays out of sight go, so they aren't killed while you talk or search.
	//! You shooting them still ends it.
	protected void KeepContactsQuiet()
	{
		RZ_ZombieSpawner spawner = m_GM.GetSpawner();
		if (!spawner)
			return;

		array<vector> spots = {};
		foreach (RZ_Opening o : m_aOpenings)
		{
			if (!o.m_bEnded && !o.m_bContactDead && o.m_iStep <= 2 && o.m_Contact && RZ_Util.IsCharacterAlive(o.m_Contact))
				spots.Insert(o.m_Contact.GetOrigin());
		}

		spawner.SetQuietSpots(spots);
	}

	//------------------------------------------------------------------------------------------------
	//! From the guidance's tick: they call you over, the next step waits for you to talk to them (or
	//! starts, with nobody outside), a contact who died, a step that lapsed, a call once more, and
	//! the contact leaves once nobody's near.
	void Tick(float dt)
	{
		m_fClock += dt;
		m_fPoll += dt;
		if (m_fPoll < POLL)
			return;

		m_fPoll = 0;
		KeepContactsQuiet();
		if (m_aOpenings.IsEmpty())
			return;

		m_aIds.Clear();
		m_aPlayers.Clear();
		RZ_Util.GetAlivePlayers(m_aIds, m_aPlayers);
		for (int i = m_aOpenings.Count() - 1; i >= 0; i--)
		{
			RZ_Opening o = m_aOpenings[i];
			LeftGone(o);
			if (o.m_Escort && !o.m_bEnded)
			{
				o.m_Escort.Update(m_GM, POLL, m_aIds, m_aPlayers);
				o.m_Contact = o.m_Escort.GetPerson();
			}

			if (o.m_bEnded)
			{
				if (ContactGone(o, true))
					m_aOpenings.Remove(i);

				continue;
			}

			TickOne(o);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void TickOne(RZ_Opening o)
	{
		int idx = m_aIds.Find(o.m_iPlayer);
		RZ_PlayerProfile profile = m_GM.FindProfile(o.m_iPlayer);
		if (idx < 0 || !profile || profile.m_iLives != o.m_iLife)
		{
			Finish(o, "died or left");
			return;
		}

		IEntity me = m_aPlayers[idx];
		bool dead;
		if (o.m_Escort)
			dead = o.m_Escort.IsDead();
		else
			dead = !o.m_Contact || !RZ_Util.IsCharacterAlive(o.m_Contact);

		if (o.m_bHadContact && !o.m_bContactDead && o.m_iStep <= 2 && dead)
		{
			ContactDied(o);
			return;
		}

		// They come with you once the first ask is out and the talk is over
		if (o.m_bWalks && !o.m_Escort && o.m_iStep >= 1 && !o.m_bContactDead)
			StartWalking(o);

		// The next person calls you over as you come up
		if (o.m_bWaitTalk && !o.m_bNextCalled && o.m_Left && HasContact(o) && RZ_Util.DistXZ(o.m_Contact.GetOrigin(), me.GetOrigin()) <= NEXT_CALL)
		{
			o.m_bNextCalled = true;
			m_GM.Say(o.m_iPlayer, o.m_sSpeaker, -1, RZ_OpeningText.NextCallOver(LeftShort(o)), o.m_Contact);
		}

		if (!o.m_bSpoke)
		{
			if (m_fClock >= o.m_fNextAt)
				Speak(o, me);

			return;
		}

		if (o.m_bNext)
		{
			if (m_fClock >= o.m_fNextAt)
			{
				o.m_bNext = false;
				Ready(o, me);
			}

			return;
		}

		// On the walk (not while it waits for you to talk to them): they leave once nobody's near
		// (walking with you, they don't)
		if (Stage(o) == RZ_EOpeningStage.WALK && !o.m_bWaitTalk && !o.m_Escort)
			ContactGone(o, false);

		if (o.m_bResolved || o.m_iErrand < 0)
			return;

		// The step ended some other way: it lapsed, or was dropped
		RZ_Errands errands = m_GM.GetErrands();
		RZ_Errand e;
		if (errands)
			e = errands.FindById(o.m_iErrand);

		if (!e || e.m_eState == RZ_EErrandState.DONE)
		{
			Lapsed(o);
			return;
		}

		// "Talk to them" points where they stand now (the tracker and the map follow the step's place)
		if (o.m_bWaitTalk && o.m_Contact)
			e.m_vPlace = o.m_Contact.GetOrigin();

		if (o.m_bWaitTalk)
			CallAgain(o, me);
		else
			AskAgain(o, me, e);
	}

	//------------------------------------------------------------------------------------------------
	//! A step lapsed. The clock's (a bite doesn't wait): they say so, and the chain goes on; any
	//! other ends the opening.
	protected void Lapsed(RZ_Opening o)
	{
		o.m_bResolved = true;
		if (o.m_bWaitTalk)
		{
			Finish(o, "nobody came to talk");
			return;
		}

		if (Stage(o) == RZ_EOpeningStage.TASK && (o.m_iTask == RZ_EOpeningTask.CLOCK || o.m_iTask == RZ_EOpeningTask.CLOCK_MED))
		{
			if (o.m_Contact)
				m_GM.Say(o.m_iPlayer, o.m_sSpeaker, -1, RZ_OpeningText.ClockLost(), o.m_Contact);

			RZ_Log.Info(string.Format("Openings: %1's bitten friend was lost (the clock ran out for %2)", o.m_sName, RZ_Util.PlayerName(o.m_iPlayer)));
			Next(o);
			return;
		}

		Finish(o, "the step lapsed");
	}

	//------------------------------------------------------------------------------------------------
	//! The chain begins: whoever's outside calls you over, and "Talk to <name>" is tracked at once
	//! (nobody outside: the first step starts by itself).
	protected void Speak(RZ_Opening o, IEntity me)
	{
		Begin(o);
		Ready(o, me);
	}

	//------------------------------------------------------------------------------------------------
	//! The chain's first stage is under way (waiting for you, or started).
	protected void Begin(RZ_Opening o)
	{
		o.m_bSpoke = true;
		SetFlag(o, FLAG_END, o.m_iEnd + 1);
		SetFlag(o, FLAG_FIRST, 1);
		o.m_iStage = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Whoever's outside is still there to talk to.
	protected static bool HasContact(RZ_Opening o)
	{
		return o.m_Contact && !o.m_bContactDead && RZ_Util.IsCharacterAlive(o.m_Contact);
	}

	//------------------------------------------------------------------------------------------------
	//! The plan's next stage is due. With someone outside it waits for you to talk to them: "Talk to
	//! <name>" is handed and tracked (the HUD, and the map's diamond on them), and they call you over
	//! as a subtitle; the talk screen never opens by itself. Talking to them already: it starts in
	//! that conversation. With nobody outside, it starts at once.
	protected void Ready(RZ_Opening o, IEntity me)
	{
		if (!HasContact(o) && !o.m_Escort)
		{
			StartStage(o, me, false, null, false);
			return;
		}

		// Talking to them already: the step's piece of the story and its ask go into that screen
		RZ_Dialogue dialogue = m_GM.GetDialogue();
		RZ_TalkSession session;
		if (dialogue)
			session = dialogue.GetSession(o.m_iPlayer);

		if (session && o.m_Contact && session.m_Speaker == o.m_Contact)
		{
			array<string> beat = {};
			RZ_Log.Info(string.Format("Openings: %1 went on talking with %2 into step %3", o.m_sName, RZ_Util.PlayerName(o.m_iPlayer), o.m_iStep + 1));
			StartStage(o, me, session.m_Tree != null, beat, true);
			return;
		}

		// Later steps never send you back: the next one starts where you are
		if (o.m_iStep >= 1 && MoveOn(o, me))
			return;

		if (!HasContact(o))
		{
			StartStage(o, me, false, null, false);
			return;
		}

		RZ_Errands errands = m_GM.GetErrands();
		if (!errands || !errands.IsOn())
		{
			Finish(o, "errands are off");
			return;
		}

		// Only before the first step, or when no next person could stand (Back to them, the old way)
		RZ_OpeningTalkDef d = m_Chain.m_TalkNext;
		if (o.m_iStep <= 0)
			d = m_Chain.m_TalkFirst;

		vector at = o.m_Contact.GetOrigin();
		RZ_Errand e = errands.Hand(d, o.m_iPlayer, "opening:" + o.m_iId.ToString(), o.m_sName, o.m_sSpeaker, RZ_EFaction.SURVIVORS, at, at, o.m_sName, me);
		if (!e)
		{
			Finish(o, "the talk couldn't be handed");
			return;
		}

		o.m_bWaitTalk = true;
		o.m_fWaitAt = m_fClock;
		o.m_bCalledAgain = false;
		o.m_bResolved = false;
		o.m_iErrand = e.m_iId;
		RZ_UIFeed feed = m_GM.GetUIFeed();
		if (feed && feed.GetTracker())
			feed.GetTracker().Follow(o.m_iPlayer, -(RZ_UIFeed.ERRAND_ID_BASE + e.m_iId), e.Fill(d.m_sTitle));

		m_GM.Say(o.m_iPlayer, o.m_sSpeaker, -1, RZ_OpeningText.CallOver(o.m_iStep, o.m_iWho, o.m_iAsks), o.m_Contact);
		RZ_Log.Info(string.Format("Openings: %1 called %2 over before step %3", o.m_sName, RZ_Util.PlayerName(o.m_iPlayer), o.m_iStep + 1));
	}

	//------------------------------------------------------------------------------------------------
	//! The next step, after the first (true: handled here; false: the old way, back to them). Walking
	//! with you: it starts as soon as they're beside you (they say it), or after a minute anyway.
	//! Handed on: after step 1 the next person stands at their place, tracked ("Find <name>"), and
	//! gives step 2 when you talk to them. Otherwise it starts where you are, said by them if they're
	//! near, else on its card.
	protected bool MoveOn(RZ_Opening o, IEntity me)
	{
		if (o.m_Escort && !o.m_bContactDead)
		{
			bool beside = o.m_Contact && RZ_Util.DistXZ(o.m_Contact.GetOrigin(), me.GetOrigin()) <= SPEAK_RANGE;
			if (o.m_fWaitNear <= 0)
				o.m_fWaitNear = m_fClock;

			if (!beside && m_fClock - o.m_fWaitNear < CATCH_UP)
			{
				o.m_bNext = true;
				return true;
			}

			o.m_fWaitNear = 0;
			StartOn(o, me, beside);
			return true;
		}

		if (o.m_bHandOff && Stage(o) == RZ_EOpeningStage.TASK && o.m_vNext != vector.Zero)
		{
			if (HandOn(o, me))
				return true;
		}

		// No next person could stand: back to them, the old way (only while one's there to go back to)
		if (o.m_bHandOff && Stage(o) == RZ_EOpeningStage.TASK && o.m_iAsks == 0 && !o.m_Left && HasContact(o))
			return false;

		bool near = HasContact(o) && RZ_Util.DistXZ(o.m_Contact.GetOrigin(), me.GetOrigin()) <= SPEAK_RANGE;
		StartOn(o, me, near);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The next stage starts where you are: its lines said by them (`said`, they're beside you), or
	//! only its card.
	protected void StartOn(RZ_Opening o, IEntity me, bool said)
	{
		if (!said)
		{
			StartStage(o, me, false, null, false);
			return;
		}

		array<string> beat = {};
		StartStage(o, me, false, beat, true);
	}

	//------------------------------------------------------------------------------------------------
	//! They come with you (the escort pattern): the body outside becomes an escort that follows you
	//! (Guidance > Companion), tagged as they were. No room or no prefab: they stay, and the next
	//! steps start where you are.
	protected void StartWalking(RZ_Opening o)
	{
		RZ_Dialogue dialogue = m_GM.GetDialogue();
		if (dialogue && dialogue.GetSession(o.m_iPlayer))
			return;

		if (!HasContact(o))
			return;

		RZ_Escort escort = new RZ_Escort();
		escort.m_sPrefab = m_Settings.m_sCompanionPrefab;
		escort.m_sName = o.m_sShort;
		escort.m_fFollowGap = 4;
		if (escort.m_sPrefab.IsEmpty() || !escort.Begin(m_GM, o.m_Contact.GetOrigin()))
		{
			o.m_bWalks = false;
			RZ_Log.Info(string.Format("Openings: %1 can't walk with %2 (no escort room), the steps start where you are", o.m_sName, RZ_Util.PlayerName(o.m_iPlayer)));
			return;
		}

		// VERIFY: a one-man civilian group (Guidance > Companion) walks behind the player on RZ_Escort's move orders, and its leader carries the talk tags
		escort.SetTalk(TAG, o.m_iId, "What was it you needed?", o.m_iId, "Talk to " + o.m_sShort);
		escort.Follow(o.m_iPlayer);
		escort.Go();
		RZ_Util.DeleteUnit(o.m_Contact);
		o.m_Contact = null;
		o.m_Escort = escort;
		RZ_Log.Info(string.Format("Openings: %1 walks with %2", o.m_sName, RZ_Util.PlayerName(o.m_iPlayer)));
	}

	//------------------------------------------------------------------------------------------------
	//! Handed on: the next person stands at their place (a body, tagged as the first was) and is
	//! tracked, "Find <name>", until you talk to them; the first giver stays behind and goes once
	//! nobody's near. False: no body could stand there.
	protected bool HandOn(RZ_Opening o, IEntity me)
	{
		RZ_Errands errands = m_GM.GetErrands();
		ResourceName prefab = m_Settings.m_sContactPrefab;
		if (prefab.IsEmpty())
			prefab = m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);

		RZ_Shared shared = m_GM.GetShared();
		if (!errands || !errands.IsOn() || prefab.IsEmpty() || !shared || !shared.CanSpawnBody(RZ_BodyBudget.STORY, o.m_vNext))
			return false;

		IEntity body = RZ_Util.SpawnPrefab(prefab, o.m_vNext, RZ_Compass.Bearing(o.m_vNext, me.GetOrigin()));
		if (!body)
			return false;

		shared.AddBody(RZ_BodyBudget.STORY, body);
		string firstShort = o.m_sShort;
		o.m_sLeftShort = firstShort;
		o.m_sLeftSpeaker = o.m_sSpeaker;
		// The first giver now only sends you on (their tags carry the opening's id negated)
		RZ_PuzzleObjectComponent.Tag(o.m_Contact, TAG, -o.m_iId, "", "What was it you needed?");
		RZ_ClueComponent.Tag(o.m_Contact, TAG, -o.m_iId, "Talk to " + firstShort);
		o.m_Left = o.m_Contact;
		o.m_iLeftWanderer = o.m_iWanderer;
		o.m_iWanderer = -1;
		o.m_Contact = body;
		o.m_sName = o.m_sNextName;
		o.m_sShort = o.m_sNextShort;
		o.m_sWho = o.m_sNextWho;
		o.m_sSpeaker = o.m_sName + ", " + o.m_sWho;
		o.m_vNext = vector.Zero;
		o.m_bNextCalled = false;
		RZ_PuzzleObjectComponent.Tag(body, TAG, o.m_iId, "", "What was it you needed?");
		RZ_ClueComponent.Tag(body, TAG, o.m_iId, "Talk to " + o.m_sShort);

		RZ_OpeningTalkDef d = m_Chain.m_TalkFind;
		vector at = body.GetOrigin();
		RZ_Errand e = errands.Hand(d, o.m_iPlayer, "opening:" + o.m_iId.ToString(), o.m_sName, o.m_sSpeaker, RZ_EFaction.SURVIVORS, at, at, o.m_sNextPlace, me);
		if (!e)
		{
			Finish(o, "the next person couldn't be handed");
			return true;
		}

		o.m_bWaitTalk = true;
		o.m_fWaitAt = m_fClock;
		o.m_bCalledAgain = false;
		o.m_bResolved = false;
		o.m_iErrand = e.m_iId;
		RZ_UIFeed feed = m_GM.GetUIFeed();
		if (feed && feed.GetTracker())
			feed.GetTracker().Follow(o.m_iPlayer, -(RZ_UIFeed.ERRAND_ID_BASE + e.m_iId), e.Fill(d.m_sTitle));

		RZ_Log.Info(string.Format("Openings: %1 sent %2 on to %3 at %4, %5", firstShort, RZ_Util.PlayerName(o.m_iPlayer), o.m_sName, o.m_sNextPlace, RZ_Util.DirectionText(me.GetOrigin(), at)));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The giver you were sent on from: their short name, for the next person's words.
	protected static string LeftShort(RZ_Opening o)
	{
		return o.m_sLeftShort;
	}

	//------------------------------------------------------------------------------------------------
	//! The first giver goes once nobody's within 150 m (an earlier life's wanderer back on the road).
	protected void LeftGone(RZ_Opening o)
	{
		if (!o.m_Left)
			return;

		if (RZ_Util.NearestDistance(o.m_Left.GetOrigin(), m_aPlayers) <= CONTACT_GONE)
			return;

		RZ_Wanderers wanderers = m_GM.GetWanderers();
		if (o.m_iLeftWanderer >= 0 && wanderers)
			wanderers.Release(o.m_iLeftWanderer, o.m_Left.GetOrigin());

		o.m_iLeftWanderer = -1;
		RZ_Util.DeleteUnit(o.m_Left);
		o.m_Left = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Handed on: the next person's place, 200-600 m on from step 1's (toward where the chain ends,
	//! when it knows), at least 150 m from the first giver: a house, not locked, the spot just outside
	//! its side facing step 1's place, checked against opening.search. False when there's none.
	protected bool FindNext(RZ_Opening o, vector from, vector first)
	{
		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(from, NEXT_MAX, buildings, 400);
		array<IEntity> houses = {};
		foreach (IEntity b : buildings)
		{
			if (!b)
				continue;

			float d = RZ_Util.DistXZ(from, b.GetOrigin());
			if (d < NEXT_MIN || d > NEXT_MAX || RZ_Util.DistXZ(first, b.GetOrigin()) < NEXT_FROM_FIRST)
				continue;

			if (RZ_LockedSpots.IsLockedBuilding(b) || !RZ_PlaceCheck.IsHouse(b))
				continue;

			houses.Insert(b);
		}

		RZ_PlaceNeed need = RZ_PlaceNeeds.Get("opening.search");
		for (int i = 0; i < SEARCH_TRIES && !houses.IsEmpty(); i++)
		{
			// The one nearest where the chain ends first (on the way), else any
			int pick = Math.RandomInt(0, houses.Count());
			if (o.m_vEnd != vector.Zero)
			{
				float best = float.MAX;
				foreach (int k, IEntity h : houses)
				{
					float toEnd = RZ_Util.DistXZ(h.GetOrigin(), o.m_vEnd);
					if (toEnd < best)
					{
						best = toEnd;
						pick = k;
					}
				}
			}

			IEntity house = houses[pick];
			houses.Remove(pick);
			vector at = RZ_Util.GroundPos(RZ_DoorGeometry.SidePoint(house, from, SEARCH_OUT));
			string why;
			if (!RZ_PlaceCheck.Check(at, need, -1, "house", why))
				continue;

			o.m_vNext = at;
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Two minutes into a wait, still not talked to and within 30 m: they call once more (a
	//! subtitle; the tracker still points at them).
	protected void CallAgain(RZ_Opening o, IEntity me)
	{
		if (o.m_bCalledAgain || m_fClock - o.m_fWaitAt < ASK_AGAIN || !HasContact(o))
			return;

		if (RZ_Util.DistXZ(o.m_Contact.GetOrigin(), me.GetOrigin()) > SPEAK_RANGE)
			return;

		RZ_Dialogue dialogue = m_GM.GetDialogue();
		if (dialogue && dialogue.GetSession(o.m_iPlayer))
			return;

		o.m_bCalledAgain = true;
		m_GM.Say(o.m_iPlayer, o.m_sSpeaker, -1, RZ_OpeningText.CallAgain(), o.m_Contact);
	}

	//------------------------------------------------------------------------------------------------
	//! You talked to them while the next step waited: the talk objective ends quietly and the step
	//! starts in this conversation (its piece of the story into `beat`, then its ask). `sayNow`:
	//! no talk screen, so the lines are said as subtitles as they come.
	protected void TalkedTo(RZ_Opening o, IEntity me, bool tree, notnull array<string> beat, bool sayNow)
	{
		o.m_bWaitTalk = false;
		DropStep(o, "done");
		RZ_Log.Info(string.Format("Openings: %1 talked to by %2 before step %3", o.m_sName, RZ_Util.PlayerName(o.m_iPlayer), o.m_iStep + 1));
		StartStage(o, me, tree, beat, sayNow);
	}

	//------------------------------------------------------------------------------------------------
	//! What they say when a step starts: step 1 their greeting (with Dialogue > Trees on, the
	//! conversation's greet by your background says it instead) and the seal; step 2 the broadcast
	//! and nobody coming (the Drifter's later asks: only the ask); step 3 the Perimeter and the cure.
	void StepLines(RZ_Opening o, bool tree, notnull array<string> lines)
	{
		int day = 1;
		if (m_GM.GetBroadcast())
			day = m_GM.GetBroadcast().GetDay();

		if (o.m_iStage <= 0 && !tree)
			lines.Insert(RZ_OpeningText.GreetPlain(o.m_iReason, o.m_iWho));

		if (o.m_iStep == 2 && o.m_iAsks > 0)
		{
			lines.Insert(RZ_OpeningText.NextAsk(o.m_iAsks));
			return;
		}

		RZ_OpeningText.Beat(Math.Max(1, o.m_iStep), day, lines);
	}

	//------------------------------------------------------------------------------------------------
	//! "Not now", or nothing at all: two minutes into step 1, with nothing done and still by the
	//! contact, they say it once more (a subtitle; the screen opens only when you talk to them).
	protected void AskAgain(RZ_Opening o, IEntity me, RZ_Errand e)
	{
		if (o.m_bAskedAgain || o.m_iStep != 1 || m_fClock - o.m_fStepAt < ASK_AGAIN || e.m_fProgress > 0 || e.m_bStepDone)
			return;

		if (!o.m_Contact || RZ_Util.DistXZ(o.m_Contact.GetOrigin(), me.GetOrigin()) > SPEAK_RANGE)
			return;

		RZ_Dialogue dialogue = m_GM.GetDialogue();
		if (dialogue && dialogue.GetSession(o.m_iPlayer))
			return;

		o.m_bAskedAgain = true;
		m_GM.Say(o.m_iPlayer, o.m_sSpeaker, -1, Remind(o, o.m_iStep), o.m_Contact);
	}

	//------------------------------------------------------------------------------------------------
	//! "What was it you needed?" while a step runs: the live step (the tracker's own text) said
	//! again in their words, then their short line. No live step: the short line alone.
	protected string Remind(RZ_Opening o, int stepNo)
	{
		string step;
		RZ_Errands errands = m_GM.GetErrands();
		RZ_Errand e;
		if (errands && o.m_iErrand >= 0 && !o.m_bResolved)
			e = errands.FindById(o.m_iErrand);

		if (e && e.m_eState != RZ_EErrandState.DONE && !RZ_OpeningTalkDef.Cast(e.m_Def))
		{
			RZ_ErrandWay way = e.GetWay();
			if (way)
				step = e.Fill(way.StepText());
		}

		string line = RZ_OpeningText.Remind(step, stepNo, o.m_iTask, o.m_sSearch, o.m_Escort != null);
		if (Stage(o) == RZ_EOpeningStage.SEARCH && o.m_vNext != vector.Zero)
			line += " " + RZ_OpeningText.ThenOn(o.m_sNextShort, o.m_sNextPlace);

		return line;
	}

	// ================================================================ The steps
	//------------------------------------------------------------------------------------------------
	protected static int Stage(RZ_Opening o)
	{
		if (o.m_iStage < 0 || o.m_iStage >= o.m_aPlan.Count())
			return RZ_EOpeningStage.NONE;

		return o.m_aPlan[o.m_iStage];
	}

	//------------------------------------------------------------------------------------------------
	//! The plan's next stage, a moment from now; the end of the plan ends the opening.
	protected void Next(RZ_Opening o)
	{
		SetFlag(o, FLAG_FIRST, 0);
		o.m_iStage++;
		if (o.m_iStage >= o.m_aPlan.Count())
		{
			Finish(o, "the chain is done");
			return;
		}

		o.m_bNext = true;
		o.m_fNextAt = m_fClock + NEXT_STEP_DELAY;
	}

	//------------------------------------------------------------------------------------------------
	//! Hands the plan's stage under way to the player through the errands and tracks it. A step 2
	//! with nothing that can run here goes straight on to the next stage. `beat` (when you talked to
	//! them): the step's piece of the story goes there before the step's ask; `sayNow` says it at once
	//! instead (as subtitles, or into a talk screen already open). `tree`: Dialogue > Trees' greet
	//! says the greeting.
	protected void StartStage(RZ_Opening o, IEntity me, bool tree, array<string> beat, bool sayNow)
	{
		RZ_Errands errands = m_GM.GetErrands();
		if (!errands || !errands.IsOn())
		{
			Finish(o, "errands are off");
			return;
		}

		int stage = Stage(o);
		if (stage == RZ_EOpeningStage.NONE)
		{
			Finish(o, "the chain is done");
			return;
		}

		vector giverAt = o.m_Place.m_vPos;
		if (o.m_Contact)
			giverAt = o.m_Contact.GetOrigin();
		else if (o.m_Escort)
			giverAt = o.m_Escort.m_vPos;

		RZ_OpeningStepDef d;
		vector place;
		string placeText;
		int faction = RZ_EFaction.SURVIVORS;
		int step = 1;
		if (stage == RZ_EOpeningStage.SEARCH)
		{
			d = m_Chain.m_Search;
			if (o.m_iLevel == 1)
				d = m_Chain.m_SearchShort;

			if (!SearchPlace(o, giverAt, place, placeText))
			{
				// Nothing down the road that the words would be true of: no step 1 here
				RZ_Log.Info(string.Format("Openings: no house down the road for %1 at %2, step 1 skipped", RZ_Util.PlayerName(o.m_iPlayer), RZ_SettlementScanner.GridReference(giverAt)));
				o.m_iStage++;
				StartStage(o, me, tree, beat, sayNow);
				return;
			}

			o.m_sSearch = placeText;
			if (o.m_bHandOff)
				NextPerson(o, place, giverAt);
		}
		else if (stage == RZ_EOpeningStage.TASK)
		{
			step = 2;
			o.m_iTask = m_Tasks.Pick(o, me, giverAt, place, placeText);
			d = m_Chain.Def(o.m_iTask);
			if (!d)
			{
				o.m_iTask = RZ_EOpeningTask.NONE;
				RZ_Log.Info(string.Format("Openings: no step 2 can run for %1 here (%2)", RZ_Util.PlayerName(o.m_iPlayer), o.m_sBackground));
				// A familiar player's one step is step 1 instead
				if (!o.m_aPlan.Contains(RZ_EOpeningStage.SEARCH))
					o.m_aPlan[o.m_iStage] = RZ_EOpeningStage.SEARCH;
				else
					o.m_iStage++;

				StartStage(o, me, tree, beat, sayNow);
				return;
			}

			SetFlag(o, FLAG_TASK, o.m_iTask);
			string taskNeed = RZ_OpeningTasks.NeedKey(o.m_iTask, placeText);
			if (!taskNeed.IsEmpty())
				RZ_PlaceNeeds.Verify(taskNeed, place, placeText);
		}
		else if (stage == RZ_EOpeningStage.LEAD)
		{
			step = 3;
			if (!FindLead(o, giverAt, place, placeText, faction))
			{
				Finish(o, "no work near");
				return;
			}

			o.m_iEnd = faction;
			o.m_sEndPlace = placeText;
			d = m_Chain.m_Lead;
			if (!o.m_Contact)
				d = m_Chain.m_LeadNote;
		}
		else
		{
			step = 3;
			if (o.m_vEnd == vector.Zero)
			{
				Finish(o, "nowhere to walk to");
				return;
			}

			d = m_Chain.Walk(o.m_iEnd);
			place = o.m_vEnd;
			placeText = o.m_sEndPlace;
			faction = o.m_iEnd;
		}

		o.m_iStep = step;
		o.m_bResolved = false;
		o.m_fStepAt = m_fClock;
		SetFlag(o, FLAG_STEP, step);
		if (beat)
		{
			StepLines(o, tree, beat);
			if (stage == RZ_EOpeningStage.SEARCH && o.m_vNext != vector.Zero)
				beat.Insert(RZ_OpeningText.SendOn(o.m_sNextShort, o.m_sNextWho, o.m_sNextPlace));

			if (o.m_bWalks && o.m_iStage == 0)
				beat.Insert(RZ_OpeningText.ComingWith());

			if (sayNow)
			{
				foreach (string line : beat)
				{
					m_GM.Say(o.m_iPlayer, o.m_sSpeaker, -1, line, o.m_Contact);
				}

				beat.Clear();
			}
		}

		string giverName = o.m_sName;
		string giverTitle = o.m_sSpeaker;
		if (!o.m_Contact && stage == RZ_EOpeningStage.LEAD)
		{
			giverName = RZ_OpeningText.ReadTitle(o.m_iReason);
			giverTitle = giverName;
		}

		RZ_Errand e = errands.Hand(d, o.m_iPlayer, "opening:" + o.m_iId.ToString(), giverName, giverTitle, faction, giverAt, place, placeText, me);
		if (!e)
		{
			Finish(o, "the step couldn't be handed");
			return;
		}

		o.m_iErrand = e.m_iId;
		if (stage == RZ_EOpeningStage.TASK)
			m_Tasks.Start(o, me, e);

		RZ_UIFeed feed = m_GM.GetUIFeed();
		if (feed && feed.GetTracker())
			feed.GetTracker().Follow(o.m_iPlayer, -(RZ_UIFeed.ERRAND_ID_BASE + e.m_iId), e.Fill(d.m_sTitle));

		RZ_Log.Info(string.Format("Openings: %1 step %2 (%3) for %4, %5", o.m_sName, step, d.m_sKey, RZ_Util.PlayerName(o.m_iPlayer), RZ_Util.DirectionText(me.GetOrigin(), place)));
	}

	//------------------------------------------------------------------------------------------------
	//! Handed on: who step 1's giver sends you to after it, and where (FindNext); none: the old way
	//! (back to them for step 2).
	protected void NextPerson(RZ_Opening o, vector searchAt, vector first)
	{
		o.m_vNext = vector.Zero;
		if (!FindNext(o, searchAt, first))
		{
			RZ_Log.Info(string.Format("Openings: no house for the next person for %1 at %2, back to %3 instead", RZ_Util.PlayerName(o.m_iPlayer), RZ_SettlementScanner.GridReference(searchAt), o.m_sName));
			return;
		}

		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		o.m_sNextName = RZ_OpeningText.Name(rng);
		while (o.m_sNextName == o.m_sName)
		{
			o.m_sNextName = RZ_OpeningText.Name(rng);
		}

		o.m_sNextShort = o.m_sNextName;
		int space = o.m_sNextName.IndexOf(" ");
		if (space > 0)
			o.m_sNextShort = o.m_sNextName.Substring(0, space);

		o.m_sNextWho = RZ_OpeningText.NextWho(o.m_sBackground);
		string town = o.m_sTown;
		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(o.m_vNext));
		if (s)
			town = s.m_sName;

		o.m_sNextPlace = o.m_sNextShort + "'s house in " + town;
	}

	//------------------------------------------------------------------------------------------------
	//! Step 1's place for this opening (false: none), and a Doctor's antidote by its door (Cure >
	//! Antidote prefab), for step 2's clock.
	protected bool SearchPlace(RZ_Opening o, vector from, out vector place, out string text)
	{
		string kind;
		if (!PickSearch(o.m_sBackground, from, place, text, kind))
			return false;

		string building = kind;
		if (building.IsEmpty())
			building = "house";

		RZ_PlaceNeeds.Verify("opening.search", place, text, -1, building);
		ResourceName antidote = m_GM.GetAntidotePrefab();
		if (o.m_sBackground == "doctor" && o.m_aPlan.Contains(RZ_EOpeningStage.TASK) && !antidote.IsEmpty())
		{
			IEntity dose = RZ_Util.SpawnPrefab(antidote, RZ_Util.GroundPos(place) + Vector(0, 0.3, 0), Math.RandomFloat(0, 360));
			if (dose)
				RZ_GameModeComponent.TrackLooseItem(dose);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Step 1's building (docs/PLACES.md, opening.search): a house 50-250 m from the contact, not
	//! locked, one of your background's kinds when one is there (the chemist's for a Nurse, a garage for
	//! a Mechanic), else a house; the spot just outside its side facing the contact, checked against
	//! the need. False when there's none: the words "down the road" would be false, so no step 1.
	//! `kind`: the building's kind ("" for a house). Spawns nothing (the place audit asks it too).
	bool PickSearch(string background, vector from, out vector place, out string text, out string kind)
	{
		kind = "";
		text = "";
		place = vector.Zero;
		array<string> kinds = {};
		RZ_OpeningText.SearchKinds(background, kinds);
		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(from, SEARCH_MAX, buildings, 400);
		array<IEntity> mine = {};
		array<IEntity> houses = {};
		foreach (IEntity b : buildings)
		{
			if (!b)
				continue;

			float d = RZ_Util.DistXZ(from, b.GetOrigin());
			if (d < SEARCH_MIN || d > SEARCH_MAX || RZ_LockedSpots.IsLockedBuilding(b))
				continue;

			if (!RZ_OpeningRoll.KindOf(b, kinds).IsEmpty())
				mine.Insert(b);
			else if (RZ_PlaceCheck.IsHouse(b))
				houses.Insert(b);
		}

		IEntity found = TrySearch(mine, kinds, from, place);
		if (found)
		{
			kind = RZ_OpeningRoll.KindOf(found, kinds);
			text = RZ_OpeningText.SearchPlace(kind);
			return true;
		}

		array<string> none = {};
		if (TrySearch(houses, none, from, place))
		{
			text = RZ_OpeningText.SearchPlace("");
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Up to SEARCH_TRIES of `list` at random, the first whose spot meets opening.search; null if none.
	protected IEntity TrySearch(notnull array<IEntity> list, notnull array<string> kinds, vector from, out vector place)
	{
		RZ_PlaceNeed need = RZ_PlaceNeeds.Get("opening.search");
		for (int i = 0; i < SEARCH_TRIES && !list.IsEmpty(); i++)
		{
			int pick = Math.RandomInt(0, list.Count());
			IEntity b = list[pick];
			list.Remove(pick);
			string kind = RZ_OpeningRoll.KindOf(b, kinds);
			if (kind.IsEmpty())
				kind = "house";

			vector at = RZ_Util.GroundPos(RZ_DoorGeometry.SidePoint(b, from, SEARCH_OUT));
			string why;
			if (RZ_PlaceCheck.Check(at, need, -1, kind, why))
			{
				place = at;
				return b;
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Where the contact stands for a rolled place: out of its door (a spawn point: on it).
	static vector ContactSpot(RZ_OpeningPlace p)
	{
		if (!p.m_bScene)
			return p.m_vPos;

		return RZ_Util.GroundPos(p.m_vPos + p.m_vOut * CONTACT_OUT);
	}

	//------------------------------------------------------------------------------------------------
	//! The place roll and step 2's tasks (the place audit asks them, RZ_PlaceAudit).
	RZ_OpeningRoll GetRoll()
	{
		return m_Roll;
	}

	//------------------------------------------------------------------------------------------------
	RZ_OpeningTasks GetTasks()
	{
		return m_Tasks;
	}

	//------------------------------------------------------------------------------------------------
	//! While an opening's step runs (RZ_OpeningStepDef.StepCounts): step 2's own system decides.
	bool StepCounts(RZ_Errands dealer, RZ_Errand e, IEntity me)
	{
		RZ_Opening o = ByErrand(e.m_iId);
		if (!o || Stage(o) != RZ_EOpeningStage.TASK || o.m_iTask == RZ_EOpeningTask.NONE)
			return true;

		return m_Tasks.Counts(dealer, o, e, me);
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Opening ByErrand(int errandId)
	{
		foreach (RZ_Opening o : m_aOpenings)
		{
			if (o.m_iErrand == errandId && !o.m_bEnded)
				return o;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! From a step's errand when it's done (RZ_OpeningStepDef.Resolve, inside the errands' tick):
	//! step 2's own effect, then the next stage waits for this tick, a moment later (the Drifter's
	//! next ask first); the walk's end is the arrival.
	void OnStepDone(RZ_Errand e, notnull array<string> said)
	{
		RZ_Opening o = ByErrand(e.m_iId);
		if (!o)
			return;

		o.m_bResolved = true;
		RZ_Log.Info(string.Format("Openings: %1 step %2 done by %3 in %4 s", o.m_sName, o.m_iStep, RZ_Util.PlayerName(o.m_iPlayer), RZ_Util.RoundToInt(m_fClock - o.m_fStepAt)));
		int stage = Stage(o);
		if (stage == RZ_EOpeningStage.WALK)
		{
			Arrive(o);
			return;
		}

		if (stage == RZ_EOpeningStage.LEAD)
		{
			m_GM.NotifyPlayerAs(o.m_iPlayer, RZ_OpeningText.LeadTitle(), RZ_OpeningText.LeadArrived(o.m_iEnd), RZ_ENoticeTag.MISSION);
			Finish(o, "found work at " + o.m_sEndPlace);
			return;
		}

		if (stage == RZ_EOpeningStage.TASK)
		{
			m_Tasks.Resolved(o, e, said);
			if (o.m_sBackground == "drifter")
			{
				o.m_iAsks++;
				if (o.m_iAsks < RZ_OpeningTasks.DrifterAsks() && !o.m_aTasks.IsEmpty())
				{
					o.m_bNext = true;
					o.m_fNextAt = m_fClock + NEXT_STEP_DELAY;
					return;
				}
			}
		}

		Next(o);
	}

	//------------------------------------------------------------------------------------------------
	//! The walk's end: what you find there, and a little Fame with them (Guidance > Opening Fame,
	//! small pay: never season Fame).
	protected void Arrive(RZ_Opening o)
	{
		if (o.m_Escort)
		{
			o.m_Escort.Arrive();
			if (o.m_Contact)
				m_GM.Say(o.m_iPlayer, o.m_sSpeaker, -1, RZ_OpeningText.WalkedYouIn(o.m_iEnd), o.m_Contact);
		}

		string text = RZ_OpeningText.Arrived(o.m_iEnd);
		int fame = m_Settings.m_iOpeningFame;
		RZ_EFaction endFaction = o.m_iEnd;
		if (fame > 0 && !RZ_Reputation.IsHunted(m_GM.GetTier(o.m_iPlayer, endFaction)))
		{
			m_GM.AddFame(o.m_iPlayer, endFaction, fame, false, RZ_RepFlag.SMALL);
			text += string.Format("\n+%1 Fame with %2.", fame, RZ_Factions.GetName(o.m_iEnd));
		}

		m_GM.NotifyPlayerAs(o.m_iPlayer, RZ_OpeningText.WalkTitle(o.m_iEnd), text, RZ_ENoticeTag.MISSION);
		Finish(o, "arrived at " + o.m_sEndPlace);
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest place with work (levels 2-3, and an opening with nobody outside): a Haven
	//! safehouse, or an outpost or stronghold of a faction that doesn't hunt you. False if none.
	protected bool FindLead(RZ_Opening o, vector from, out vector place, out string text, out int faction)
	{
		float best = float.MAX;
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
		{
			RZ_Safehouse h = houses.Get(houses.FindNearest(from));
			if (h)
			{
				best = RZ_Util.DistXZ(from, h.m_vPos);
				place = h.m_vPos;
				text = RZ_OpeningText.EndPlace(RZ_EFaction.SURVIVORS, h.m_sName, false);
				faction = RZ_EFaction.SURVIVORS;
			}
		}

		RZ_Territory territory = m_GM.GetTerritory();
		if (territory)
		{
			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				if (RZ_Reputation.IsHunted(m_GM.GetTier(o.m_iPlayer, f)))
					continue;

				array<RZ_Holding> list = {};
				territory.GetOutposts(f, list);
				RZ_Holding seat = territory.GetStronghold(f);
				if (seat)
					list.Insert(seat);

				foreach (RZ_Holding hold : list)
				{
					float d = RZ_Util.DistXZ(from, hold.m_vPos);
					if (d < best)
					{
						best = d;
						place = hold.m_vPos;
						text = RZ_OpeningText.EndPlace(f, hold.m_sName, false);
						faction = f;
					}
				}
			}
		}

		return best < float.MAX;
	}

	//------------------------------------------------------------------------------------------------
	//! The contact was killed: the step under way ends, whoever finds you says so, and the walk
	//! goes to the nearest safehouse (when the plan has it). Nothing else depends on them.
	protected void ContactDied(RZ_Opening o)
	{
		o.m_bContactDead = true;
		o.m_bWaitTalk = false;
		SetFlag(o, FLAG_FIRST, 0);
		DropStep(o, "the contact died");
		RZ_Wanderers wanderers = m_GM.GetWanderers();
		if (o.m_iWanderer >= 0 && wanderers)
			wanderers.TakeOffRoad(o.m_iWanderer);

		o.m_iWanderer = -1;
		RZ_Log.Info(string.Format("Openings: %1 died (step %2 for %3)", o.m_sName, o.m_iStep, RZ_Util.PlayerName(o.m_iPlayer)));
		RZ_Safehouses houses = m_GM.GetSafehouses();
		RZ_Safehouse h;
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(o.m_iPlayer);
		if (houses && me)
			h = houses.Get(houses.FindNearest(me.GetOrigin()));

		if (!h)
		{
			Finish(o, "the contact died");
			return;
		}

		o.m_sSpeaker = "Someone in a doorway";
		m_GM.NotifyPlayer(o.m_iPlayer, o.m_sSpeaker, RZ_OpeningText.ContactDied(h.m_sName));
		int walk = o.m_aPlan.Find(RZ_EOpeningStage.WALK);
		if (walk < 0)
			walk = o.m_aPlan.Find(RZ_EOpeningStage.LEAD);

		if (walk < 0)
		{
			Finish(o, "the contact died");
			return;
		}

		o.m_iEnd = RZ_EFaction.SURVIVORS;
		o.m_vEnd = h.m_vPos;
		o.m_sEndPlace = RZ_OpeningText.EndPlace(RZ_EFaction.SURVIVORS, h.m_sName, false);
		o.m_bSpoke = true;
		o.m_iStage = walk;
		o.m_bNext = true;
		o.m_fNextAt = m_fClock + NEXT_STEP_DELAY;
		SetFlag(o, FLAG_END, 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void DropStep(RZ_Opening o, string why)
	{
		RZ_Errands errands = m_GM.GetErrands();
		if (!errands || o.m_iErrand < 0 || o.m_bResolved)
			return;

		// A talk objective isn't a job of its own: it goes quietly, not into the past jobs
		RZ_Errand e = errands.FindById(o.m_iErrand);
		if (e && RZ_OpeningTalkDef.Cast(e.m_Def))
			errands.Drop(e, "done");
		else if (e)
			errands.Drop(e, why);

		o.m_bResolved = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Finish(RZ_Opening o, string why)
	{
		if (o.m_bEnded)
			return;

		o.m_bEnded = true;
		DropStep(o, why);
		// A Sample's remains go back to the engine's own corpse cleanup
		RZ_OpeningTasks.KeepBody(o.m_Remains, false);
		o.m_Remains = null;
		SetFlag(o, FLAG_STEP, 4);
		SetFlag(o, FLAG_FIRST, 0);
		RZ_Log.Info(string.Format("Openings: ended for %1: %2 (step %3, %4 min)", RZ_Util.PlayerName(o.m_iPlayer), why, o.m_iStep, RZ_Util.RoundToInt((m_fClock - o.m_fBorn) / 60)));
	}

	//------------------------------------------------------------------------------------------------
	protected void EndFor(int playerId, string why)
	{
		foreach (RZ_Opening o : m_aOpenings)
		{
			if (o.m_iPlayer == playerId && !o.m_bEnded)
				Finish(o, why);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The contact leaves (on the walk, or once it's over) when nobody is within 150 m, or 45
	//! minutes on with nobody within 50 m. True when they're gone.
	protected bool ContactGone(RZ_Opening o, bool over)
	{
		if (o.m_Escort)
		{
			bool stale = m_fClock - o.m_fBorn > CONTACT_MAX;
			if (!o.m_Escort.IsDead() && RZ_Util.NearestDistance(o.m_Escort.m_vPos, m_aPlayers) <= CONTACT_GONE && !stale)
				return false;

			ReleaseWanderer(o, o.m_Escort.m_vPos);
			o.m_Escort.Cleanup();
			o.m_Escort = null;
			o.m_Contact = null;
			return o.m_Left == null;
		}

		if (!o.m_Contact)
		{
			LeftGone(o);
			return o.m_Left == null;
		}

		float nearest = RZ_Util.NearestDistance(o.m_Contact.GetOrigin(), m_aPlayers);
		bool old = m_fClock - o.m_fBorn > CONTACT_MAX && nearest > 50;
		if (nearest <= CONTACT_GONE && !old)
			return false;

		ReleaseWanderer(o, o.m_Contact.GetOrigin());
		RZ_Util.DeleteUnit(o.m_Contact);
		o.m_Contact = null;
		return o.m_Left == null;
	}

	//------------------------------------------------------------------------------------------------
	//! An earlier life's wanderer goes back on the road from where they stood.
	protected void ReleaseWanderer(RZ_Opening o, vector at)
	{
		RZ_Wanderers wanderers = m_GM.GetWanderers();
		if (o.m_iWanderer >= 0 && wanderers)
			wanderers.Release(o.m_iWanderer, at);

		o.m_iWanderer = -1;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetFlag(RZ_Opening o, string key, int value)
	{
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		if (trees && trees.GetMemory())
			trees.GetMemory().SetCharFlag(o.m_iPlayer, key, value);
	}

	// ================================================================ Talking to them
	//------------------------------------------------------------------------------------------------
	//! What they say when you talk to them (the talk screen's greeting, RZ_TalkOpening, and "Talk to
	//! ..." without it). While the next step waits for you (their "Talk to <name>"), talking to them
	//! starts it: that step's piece of the story, then its ask (RZ_Errands.Hand, into the same
	//! screen). Else, while a step runs, the step again, short; once it's over, a send-off. Anyone
	//! else is told they're waiting for someone. `tree`: Dialogue > Trees' greet says the greeting.
	//! `sayNow`: no talk screen, the step's lines are said as subtitles, in order.
	void TalkLines(int playerId, int id, bool tree, notnull array<string> lines, IEntity user, bool sayNow = false)
	{
		// The first giver, after sending you on: on you go
		if (id < 0)
		{
			RZ_Opening sent = ById(-id);
			if (sent && sent.m_iPlayer == playerId && !sent.m_bEnded)
				lines.Insert(RZ_OpeningText.GoOn(sent.m_sShort));
			else
				lines.Insert(RZ_OpeningText.NotYou());

			return;
		}

		RZ_Opening o = ById(id);
		if (!o || o.m_iPlayer != playerId)
		{
			lines.Insert(RZ_OpeningText.NotYou());
			return;
		}

		// Talked to before they called you over (the first 4 s): the chain begins here
		if (!o.m_bSpoke && !o.m_bEnded && user)
		{
			Begin(o);
			o.m_bWaitTalk = true;
		}

		if (o.m_bWaitTalk && !o.m_bEnded && user)
		{
			TalkedTo(o, user, tree, lines, sayNow);
			if (o.m_bEnded && lines.IsEmpty() && !sayNow)
				lines.Insert(RZ_OpeningText.AskAgain(3, RZ_EOpeningTask.NONE, ""));

			return;
		}

		if (o.m_bEnded)
		{
			lines.Insert(RZ_OpeningText.AskAgain(3, RZ_EOpeningTask.NONE, ""));
			return;
		}

		if (o.m_iStep <= 0)
		{
			if (!tree)
				lines.Insert(RZ_OpeningText.GreetPlain(o.m_iReason, o.m_iWho));

			return;
		}

		lines.Insert(Remind(o, o.m_iStep));
	}

	//------------------------------------------------------------------------------------------------
	//! The hand-over reply on their talk screen (RZ_TalkOpening) while a hand-over step runs: its
	//! label, and `need` when you don't carry the thing (locked with it). False: no such reply now.
	bool HandOverReply(int playerId, int id, IEntity user, out string label, out string need)
	{
		label = "";
		need = "";
		RZ_Opening o = ById(id);
		if (!o || o.m_iPlayer != playerId || o.m_bEnded || o.m_bWaitTalk || o.m_bResolved || o.m_bTended)
			return false;

		if (Stage(o) != RZ_EOpeningStage.TASK || !RZ_OpeningTasks.HandsOver(o.m_iTask))
			return false;

		label = RZ_OpeningText.HandOver(o.m_iTask);
		if (!user || RZ_Errands.CountItems(user, RZ_OpeningTasks.HandItem(o.m_iTask)) <= 0)
			need = RZ_OpeningText.HandOverNeed(o.m_iTask);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The hand-over reply picked: you see to them, and the step is done (the errand takes the thing
	//! as it completes, a few seconds on). Opening the talk never does this by itself.
	void HandOver(int playerId, int id, IEntity user, notnull array<string> said)
	{
		string label;
		string need;
		if (!HandOverReply(playerId, id, user, label, need))
			return;

		if (!need.IsEmpty())
		{
			said.Insert(RZ_OpeningText.HandOverMissing());
			return;
		}

		RZ_Opening o = ById(id);
		o.m_bTended = true;
		said.Insert(RZ_OpeningText.HandedOver(o.m_iTask));
		RZ_Log.Info(string.Format("Openings: %1 handed over to %2 (step 2, task %3)", RZ_Util.PlayerName(playerId), o.m_sName, o.m_iTask));
	}

	//------------------------------------------------------------------------------------------------
	//! "Talk to ..." without the talk screen (RZ_PuzzleActionHelper.RouteRead): their lines as
	//! subtitles (a waiting step starts here too).
	void HandleTalk(int playerId, int id, IEntity owner)
	{
		array<string> lines = {};
		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		TalkLines(playerId, id, false, lines, user, true);
		string title = SpeakerFor(id);
		if (title.IsEmpty())
			title = "Someone";

		foreach (string line : lines)
		{
			m_GM.Say(playerId, title, -1, line, owner);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "What was it you needed?" (their choice slot 0): the step again, short; while the next step
	//! waits for you, it starts, as when you talk to them.
	void HandleChoice(int playerId, int id, int option, IEntity owner)
	{
		RZ_Opening o = ById(id);
		if (id < 0 || !o || o.m_iPlayer != playerId || (o.m_bWaitTalk && !o.m_bEnded))
		{
			HandleTalk(playerId, id, owner);
			return;
		}

		int step = Math.Max(1, o.m_iStep);
		if (o.m_bEnded)
			step = 3;

		if (o.m_bEnded)
		{
			m_GM.Say(playerId, o.m_sSpeaker, -1, RZ_OpeningText.AskAgain(step, o.m_iTask, o.m_sSearch), owner);
			return;
		}

		m_GM.Say(playerId, o.m_sSpeaker, -1, Remind(o, step), owner);
	}
}
