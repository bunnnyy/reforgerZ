//! Openings: how every life starts (docs/DESIGN.md, "Openings").
//!
//! Each life starts at a random place that fits its background (RZ_OpeningRoll, rolled when the
//! character spawns), coming out of its hiding place: a few props by the door you open (the scene,
//! and your background's own things), the calendar with the days crossed off, and someone who knew
//! you before the outbreak, standing a few metres off. They speak first, on the talk screen, and
//! the chain's first step is tracked straight away: the only job in the game that starts without
//! you talking to anyone or using anything. The chain (RZ_OpeningChain) is three errand steps:
//! close by and quiet (a building of your background's kind when one is near), your background's
//! own system (RZ_OpeningSteps.c), and the walk to where your background belongs, which pays a
//! little Fame with that faction. Players past learning (guidance level 2+) get the first step only.
//!
//! It replaces the spawn scenarios' clues and "a wanderer finds you" for that life; the gentle
//! start, the wind-up radio, the tips and the guidance levels stay. Not in the epilogue week (the
//! place is still rolled). One log line when it starts, per step, and when it ends. Server only.

//------------------------------------------------------------------------------------------------
//! The stages a chain is made of, in its plan (RZ_Opening.m_aPlan).
enum RZ_EOpeningStage
{
	NONE,
	SEARCH,		// step 1: a building close by, quietly
	TASK,		// step 2: your background's own system (the Drifter: up to three asks)
	WALK		// step 3: to where your background belongs
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
	IEntity m_Contact;			// weak: someone from before
	bool m_bHadContact;
	bool m_bContactDead;
	string m_sName;				// "Vera Holm"
	string m_sWho;				// "a patient's mother"
	string m_sSpeaker;			// the title their lines come under; whoever finds you, once they're dead
	int m_iEnd = -1;			// the faction the chain ends with
	vector m_vEnd;
	string m_sEndPlace;			// "the safehouse in Morton"
	ref array<int> m_aPlan = {};	// the stages, in order (RZ_EOpeningStage)
	int m_iStage = -1;			// the plan's stage under way (-1 before they speak)
	int m_iStep;				// 0 before they speak, 1-3 the step under way (the conversations' opening.step)
	bool m_bNext;				// the next stage starts at m_fNextAt
	string m_sSearch;			// step 1's place: "the chemist's over the way"
	int m_iTask;				// step 2's task (RZ_EOpeningTask)
	ref array<int> m_aTasks = {};	// step 2's tasks still to try (the background's, or the Drifter's asks)
	int m_iAsks;				// the Drifter's asks done
	IEntity m_Target;			// weak: the task's building (KEYS)
	int m_iTargetId = -1;		// the task's wanderer (SWAP) or road post (TOLL)
	float m_fPulseAt;			// GATE: the step's progress at its next noise
	bool m_bTold;				// the task said once what's missing
	int m_iErrand = -1;			// the current step's errand id
	bool m_bResolved;			// the current step is done
	bool m_bSpoke;
	bool m_bAskedAgain;
	bool m_bAnnounce;			// the talk screen is open because they spoke first (a step starting)
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

	protected static const float POLL = 1;
	protected static const float SPEAK_DELAY = 4;		// seconds after the spawn before they speak
	protected static const float SPEAK_RANGE = 30;
	protected static const float CONTACT_OUT = 6;		// the contact stands this far out of the door
	protected static const float NEXT_STEP_DELAY = 4;
	protected static const float ASK_AGAIN = 120;		// "Not now": asked again once, this long into step 1
	protected static const float CONTACT_GONE = 150;	// the contact goes once nobody is this close (the walk, or over)
	protected static const float CONTACT_MAX = 2700;	// and once the opening is 45 minutes old, with nobody within 50 m
	protected static const float SEARCH_MIN = 50;		// step 1's house: this far from the contact...
	protected static const float SEARCH_MAX = 250;		// ...and no further

	protected RZ_GameModeComponent m_GM;
	protected RZ_Guidance m_Guide;
	protected RZ_GuidanceSettings m_Settings;
	protected ref RZ_OpeningRoll m_Roll;
	protected ref RZ_OpeningChain m_Chain;
	protected ref RZ_OpeningTasks m_Tasks;
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
			if (!place.m_bFits)
				RZ_Log.Info(string.Format("Openings: no %1 place fits for %2, a house instead", bg.m_sId, RZ_Util.PlayerName(playerId)));
		}
		else
		{
			// The fallback's end: the spawn point the spawn screen asked for, and the contact only
			place = new RZ_OpeningPlace();
			place.m_vPos = character.GetOrigin();
			place.m_iTown = m_GM.FindNearestSettlement(place.m_vPos);
			RZ_Log.Info(string.Format("Openings: no place for %1 (%2), the spawn point instead", RZ_Util.PlayerName(playerId), bg.m_sId));
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
		Plan(o, rng);
		o.m_sName = RZ_OpeningText.Name(rng);
		o.m_sWho = RZ_OpeningText.Who(bg.m_sId);
		o.m_sSpeaker = o.m_sName + ", " + o.m_sWho;
		o.m_iEnd = EndFactionFor(playerId, bg.m_sId);
		FindEnd(o, ent.GetOrigin());

		// Nobody to speak first (no body prefab, or the story budget is full): no opening this life,
		// and the spawn scenario, the wanderer and "Find the Haven" stand in as they did before
		SpawnContact(o, ent.GetOrigin());
		if (!o.m_bHadContact)
		{
			RZ_Log.Info(string.Format("Openings: no contact could stand for %1 at %2, the spawn scenario instead", RZ_Util.PlayerName(playerId), RZ_SettlementScanner.GridReference(ent.GetOrigin())));
			return;
		}

		int day = 1;
		if (m_GM.GetBroadcast())
			day = m_GM.GetBroadcast().GetDay();

		if (place.m_bScene)
			PlaceScene(o, day);

		m_Guide.MarkOpening(playerId);
		m_Guide.SetSpawnLine(playerId, RZ_OpeningText.SpawnLine(place.m_sKind, place.m_bScene, o.m_sTown, day));
		o.m_fNextAt = m_fClock + SPEAK_DELAY;
		m_aOpenings.Insert(o);

		string chain = PlanText(o);

		string kind = place.m_sKind;
		if (kind.IsEmpty())
			kind = "spawn point";

		RZ_Log.Info(string.Format("Openings: started for %1: %2, %3 in %4 (%5), level %6, someone from before (%7), chain %8 to %9",
			RZ_Util.PlayerName(playerId), bg.m_sId, kind, o.m_sTown, RZ_SettlementScanner.GridReference(ent.GetOrigin()), o.m_iLevel, o.m_sWho, chain, o.m_sEndPlace));
	}

	//------------------------------------------------------------------------------------------------
	//! The chain's plan: levels 0-1 all three steps, others the first only; nothing with errands
	//! off. Step 2's tasks to try: the background's own, or the Drifter's asks.
	protected void Plan(RZ_Opening o, RZ_Rng rng)
	{
		RZ_Errands errands = m_GM.GetErrands();
		if (errands && errands.IsOn())
		{
			o.m_aPlan.Insert(RZ_EOpeningStage.SEARCH);
			if (o.m_iLevel <= 1)
			{
				o.m_aPlan.Insert(RZ_EOpeningStage.TASK);
				o.m_aPlan.Insert(RZ_EOpeningStage.WALK);
			}
		}

		o.m_bFull = o.m_aPlan.Contains(RZ_EOpeningStage.WALK);
		if (o.m_sBackground == "drifter")
			RZ_OpeningTasks.DrifterPool(rng, o.m_aTasks);
		else
			RZ_OpeningTasks.Preferred(o.m_sBackground, o.m_aTasks);
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
	//! By the door: your background's own things on one side (Guidance > Opening scenes, its id;
	//! none: nothing), the calendar to read on the other, and the door left open. The clue clean-up
	//! removes them in time.
	protected void PlaceScene(RZ_Opening o, int day)
	{
		RZ_OpeningPlace place = o.m_Place;
		RZ_GuidanceWorld world = m_Guide.GetWorld();
		vector side = Vector(place.m_vOut[2], 0, -place.m_vOut[0]);
		ResourceName props = SceneFor(o.m_sBackground);
		if (!props.IsEmpty())
		{
			vector propsAt = RZ_Util.GroundPos(place.m_vPos + side * 1.8 - place.m_vOut * 0.5);
			world.TrackProp(RZ_Util.SpawnPrefab(props, propsAt, place.m_fYaw));
		}

		vector noteAt = RZ_Util.GroundPos(place.m_vPos - side * 1.2);
		world.PlaceReadable(noteAt, "Read the calendar", "The calendar", RZ_OpeningText.Calendar(day));
		RZ_OpeningRoll.OpenDoor(place.m_Door);
	}

	//------------------------------------------------------------------------------------------------
	//! A composition from Guidance > Opening scenes by its key (a background's id); "" if none.
	protected ResourceName SceneFor(string key)
	{
		if (!m_Settings.m_aOpeningScenes)
			return "";

		foreach (RZ_OpeningSceneSlot slot : m_Settings.m_aOpeningScenes)
		{
			if (slot && slot.m_sKey == key)
				return slot.m_sPrefab;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Someone from before, a few metres out of the door, facing you (Guidance > Contact; empty: the
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
		string first = o.m_sName;
		int space = first.IndexOf(" ");
		if (space > 0)
			first = first.Substring(0, space);

		RZ_PuzzleObjectComponent.Tag(body, TAG, o.m_iId, "", "What was it you needed?");
		RZ_ClueComponent.Tag(body, TAG, o.m_iId, "Talk to " + first);
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
	//! From the guidance's tick: they speak first, the next step starts, a contact who died, a step
	//! that lapsed, "Not now" asked again once, and the contact leaves once nobody's near.
	void Tick(float dt)
	{
		m_fClock += dt;
		m_fPoll += dt;
		if (m_fPoll < POLL || m_aOpenings.IsEmpty())
			return;

		m_fPoll = 0;
		m_aIds.Clear();
		m_aPlayers.Clear();
		RZ_Util.GetAlivePlayers(m_aIds, m_aPlayers);
		for (int i = m_aOpenings.Count() - 1; i >= 0; i--)
		{
			RZ_Opening o = m_aOpenings[i];
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
		if (o.m_bHadContact && !o.m_bContactDead && o.m_iStep <= 2 && (!o.m_Contact || !RZ_Util.IsCharacterAlive(o.m_Contact)))
		{
			ContactDied(o);
			return;
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
				StartStage(o, me);
			}

			return;
		}

		if (Stage(o) == RZ_EOpeningStage.WALK)
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

		AskAgain(o, me, e);
	}

	//------------------------------------------------------------------------------------------------
	//! A step lapsed. The clock's (a bite doesn't wait): they say so, and the chain goes on; any
	//! other ends the opening.
	protected void Lapsed(RZ_Opening o)
	{
		o.m_bResolved = true;
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
	//! The contact speaks first, and the first step is tracked at once, whatever you answer.
	protected void Speak(RZ_Opening o, IEntity me)
	{
		o.m_bSpoke = true;
		SetFlag(o, FLAG_END, o.m_iEnd + 1);
		o.m_iStage = 0;
		StartStage(o, me);
	}

	//------------------------------------------------------------------------------------------------
	//! A step starts with the contact close by: they speak first, on the talk screen (or, without
	//! it, their lines as subtitles), with that step's piece of the story; the step's ask follows
	//! into the same screen (RZ_Errands.Hand). Nothing when they're gone or out of earshot.
	protected void Announce(RZ_Opening o, IEntity me)
	{
		if (!o.m_Contact || !RZ_Util.IsCharacterAlive(o.m_Contact) || RZ_Util.DistXZ(o.m_Contact.GetOrigin(), me.GetOrigin()) > SPEAK_RANGE)
			return;

		o.m_bAnnounce = true;
		bool talking = OpenTalk(o, me);
		o.m_bAnnounce = false;
		string how = "the talk screen";
		if (!talking)
		{
			array<string> lines = {};
			StepLines(o, false, lines);
			foreach (string line : lines)
			{
				m_GM.Say(o.m_iPlayer, o.m_sSpeaker, -1, line, o.m_Contact);
			}

			how = "subtitles";
		}

		RZ_Log.Info(string.Format("Openings: %1 spoke first to %2 at step %3 (%4)", o.m_sName, RZ_Util.PlayerName(o.m_iPlayer), o.m_iStep, how));
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

		if (o.m_iStep <= 1 && !tree)
			lines.Insert(RZ_OpeningText.GreetPlain());

		if (o.m_iStep == 2 && o.m_iAsks > 0)
		{
			lines.Insert(RZ_OpeningText.NextAsk(o.m_iAsks));
			return;
		}

		RZ_OpeningText.Beat(Math.Max(1, o.m_iStep), day, lines);
	}

	//------------------------------------------------------------------------------------------------
	protected bool OpenTalk(RZ_Opening o, IEntity me)
	{
		RZ_Dialogue dialogue = m_GM.GetDialogue();
		if (!dialogue || dialogue.GetSession(o.m_iPlayer) || RZ_Util.IsInVehicle(me))
			return false;

		return dialogue.Open(o.m_iPlayer, o.m_Contact, me);
	}

	//------------------------------------------------------------------------------------------------
	//! "Not now", or nothing at all: two minutes into step 1, with nothing done and still by the
	//! contact, they ask once more.
	protected void AskAgain(RZ_Opening o, IEntity me, RZ_Errand e)
	{
		if (o.m_bAskedAgain || o.m_iStep != 1 || m_fClock - o.m_fStepAt < ASK_AGAIN || e.m_fProgress > 0 || e.m_bStepDone)
			return;

		if (!o.m_Contact || RZ_Util.DistXZ(o.m_Contact.GetOrigin(), me.GetOrigin()) > SPEAK_RANGE)
			return;

		o.m_bAskedAgain = true;
		if (!OpenTalk(o, me))
			m_GM.Say(o.m_iPlayer, o.m_sSpeaker, -1, RZ_OpeningText.AskAgain(o.m_iStep, o.m_iTask, o.m_sSearch), o.m_Contact);
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
	//! with nothing that can run here goes straight on to the next stage.
	protected void StartStage(RZ_Opening o, IEntity me)
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

			SearchPlace(o, giverAt, place, placeText);
			o.m_sSearch = placeText;
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
				o.m_iStage++;
				StartStage(o, me);
				return;
			}

			SetFlag(o, FLAG_TASK, o.m_iTask);
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
		Announce(o, me);
		RZ_Errand e = errands.Hand(d, o.m_iPlayer, "opening:" + o.m_iId.ToString(), o.m_sName, o.m_sSpeaker, faction, giverAt, place, placeText, me);
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
	//! Step 1's building: 50-250 m from the contact, not locked, one of your background's kinds when
	//! one is there (the chemist's for a Nurse, a garage for a Mechanic), else any; without one, a
	//! point among the houses over the way. A Doctor's clinic has an antidote by its door (Cure >
	//! Antidote prefab), for step 2's clock.
	protected void SearchPlace(RZ_Opening o, vector from, out vector place, out string text)
	{
		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(from, SEARCH_MAX, buildings, 40);
		array<string> kinds = {};
		RZ_OpeningText.SearchKinds(o.m_sBackground, kinds);
		array<IEntity> fit = {};
		array<IEntity> mine = {};
		string mineKind = "";
		foreach (IEntity b : buildings)
		{
			float d = RZ_Util.DistXZ(from, b.GetOrigin());
			if (d < SEARCH_MIN || RZ_LockedSpots.IsLockedBuilding(b))
				continue;

			fit.Insert(b);
			string kind = RZ_OpeningRoll.KindOf(b, kinds);
			if (!kind.IsEmpty())
			{
				mine.Insert(b);
				mineKind = kind;
			}
		}

		if (fit.IsEmpty())
		{
			place = RZ_Mission.LandPointNear(from, 80, 200);
			text = "the houses over the way";
			return;
		}

		IEntity house;
		if (!mine.IsEmpty())
		{
			house = mine.GetRandomElement();
			text = RZ_OpeningText.SearchPlace(RZ_OpeningRoll.KindOf(house, kinds));
		}
		else
		{
			house = fit.GetRandomElement();
			text = RZ_OpeningText.SearchPlace("");
		}

		place = RZ_Mission.LandPointNear(house.GetOrigin(), 2, 6);
		ResourceName antidote = m_GM.GetAntidotePrefab();
		if (o.m_sBackground == "doctor" && o.m_aPlan.Contains(RZ_EOpeningStage.TASK) && !antidote.IsEmpty())
		{
			IEntity dose = RZ_Util.SpawnPrefab(antidote, RZ_Util.GroundPos(place) + Vector(0, 0.3, 0), Math.RandomFloat(0, 360));
			if (dose)
				RZ_GameModeComponent.TrackLooseItem(dose);
		}
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
	//! The contact was killed: the step under way ends, whoever finds you says so, and the walk
	//! goes to the nearest safehouse (when the plan has it). Nothing else depends on them.
	protected void ContactDied(RZ_Opening o)
	{
		o.m_bContactDead = true;
		DropStep(o, "the contact died");
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

		RZ_Errand e = errands.FindById(o.m_iErrand);
		if (e)
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
		SetFlag(o, FLAG_STEP, 4);
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
		if (!o.m_Contact)
			return true;

		float nearest = RZ_Util.NearestDistance(o.m_Contact.GetOrigin(), m_aPlayers);
		bool old = m_fClock - o.m_fBorn > CONTACT_MAX && nearest > 50;
		if (nearest <= CONTACT_GONE && !old)
			return false;

		RZ_Util.DeleteUnit(o.m_Contact);
		o.m_Contact = null;
		return true;
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
	//! ..." without it): the step's lines when they spoke first; else, before they've spoken, their
	//! greeting; while a step runs, the step again, short; once it's over, a send-off. Anyone else
	//! is told they're waiting for someone. `tree`: Dialogue > Trees' greet says the greeting.
	void TalkLines(int playerId, int id, bool tree, notnull array<string> lines)
	{
		RZ_Opening o = ById(id);
		if (!o || o.m_iPlayer != playerId)
		{
			lines.Insert(RZ_OpeningText.NotYou());
			return;
		}

		if (o.m_bAnnounce)
		{
			StepLines(o, tree, lines);
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
				lines.Insert(RZ_OpeningText.GreetPlain());

			return;
		}

		lines.Insert(RZ_OpeningText.AskAgain(o.m_iStep, o.m_iTask, o.m_sSearch));
	}

	//------------------------------------------------------------------------------------------------
	//! "Talk to ..." without the talk screen (RZ_PuzzleActionHelper.RouteRead): their lines as
	//! subtitles.
	void HandleTalk(int playerId, int id, IEntity owner)
	{
		array<string> lines = {};
		TalkLines(playerId, id, false, lines);
		string title = "Someone";
		RZ_Opening o = ById(id);
		if (o)
			title = o.m_sSpeaker;

		foreach (string line : lines)
		{
			m_GM.Say(playerId, title, -1, line, owner);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "What was it you needed?" (their choice slot 0): the step again, short.
	void HandleChoice(int playerId, int id, int option, IEntity owner)
	{
		RZ_Opening o = ById(id);
		if (!o || o.m_iPlayer != playerId)
		{
			HandleTalk(playerId, id, owner);
			return;
		}

		int step = Math.Max(1, o.m_iStep);
		if (o.m_bEnded)
			step = 3;

		m_GM.Say(playerId, o.m_sSpeaker, -1, RZ_OpeningText.AskAgain(step, o.m_iTask, o.m_sSearch), owner);
	}
}
