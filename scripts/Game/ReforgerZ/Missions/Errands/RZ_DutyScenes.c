//! Duty scenes (docs/ERRANDS.md, "Things that happen"): what a duty has you see, find and decide,
//! in person. A scene is a run of beats. A beat is a moment at a spot: one line said as you reach
//! it, and one to three things you can do about it, each a work site (a hold, the hands, the world
//! showing it); several at the one spot stand as one prompt, the spot's name, whose press lists them
//! (RZ_WorkSites.Gather), and a place of its own to go and look at keeps its own prompt. What you choose is applied at once and remembered: the giver's
//! regard, a local made, a line someone says to you later, a record in the world's memory, the
//! place's food or morale, a town's infection, your Favor for the job, small Fame or Infamy.
//! A shady choice costs only if it's seen: their people within 20 m, or the place itself within
//! 25 m. A scene can be scored (each beat has a right call) and then pays by the share right.
//! Beats come from the duty (RZ_DutyFinds, RZ_ErrandDuties): drawn from the world as it is.
//! Owned by the errand dealer. Server only.

//------------------------------------------------------------------------------------------------
//! One thing you can do at a beat.
class RZ_DutyOption
{
	string m_sLabel;			// the prompt: "Chalk it in the book"
	string m_sSaid;				// what follows, one line
	float m_fHold = 3;
	RZ_EWork m_eWork = RZ_EWork.HANDS;
	int m_iRight;				// +1 the right call, -1 the wrong one, 0 neither (a scored scene)
	int m_iFavor;				// added to the job's Favor (can be negative)
	int m_iFame;				// small Fame with the giver's faction, inside the ledger's caps
	int m_iInfamy;				// Infamy with the giver's faction (a shady one: only if seen)
	bool m_bShady;				// costs only if their people or the place see it
	RZ_EErrandItem m_eItem;		// needs this from your pack (taken)
	int m_iItems;
	string m_sFx;				// effects, comma separated (RZ_DutyScenes.Apply)
	string m_sWho;				// a local made by "local": who they are ("who was in the shed")
	string m_sLater;			// a line for "later", "local" or "rumour"
	string m_sNext;				// the beat this leads to ("": the next in order; "end": the scene ends)
	string m_sFlag;				// set on the scene when chosen
	string m_sTold;				// what the giver says of it when you report ("" nothing)
	float m_fOut;				// > 0: its own spot, this far from the beat's (a place to go and look), not side by side
	bool m_bDone;				// taken already (a beat with several picks)
	bool m_bEnds;				// ends a beat with several picks at once ("I've seen enough")

	//------------------------------------------------------------------------------------------------
	RZ_DutyOption Fx(string fx)
	{
		m_sFx = fx;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_DutyOption Pay(int favor, int fame = 0)
	{
		m_iFavor = favor;
		m_iFame = fame;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	//! A shady one: `infamy` and the giver's regard down, if it's seen.
	RZ_DutyOption Shady(int infamy)
	{
		m_bShady = true;
		m_iInfamy = infamy;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_DutyOption Needs(RZ_EErrandItem item, int count)
	{
		m_eItem = item;
		m_iItems = count;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_DutyOption Right(int right)
	{
		m_iRight = right;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_DutyOption Next(string beat)
	{
		m_sNext = beat;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_DutyOption Flag(string flag)
	{
		m_sFlag = flag;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_DutyOption Later(string who, string line)
	{
		m_sWho = who;
		m_sLater = line;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_DutyOption Told(string line)
	{
		m_sTold = line;
		return this;
	}

	//------------------------------------------------------------------------------------------------
	RZ_DutyOption Work(RZ_EWork work, float hold)
	{
		m_eWork = work;
		m_fHold = hold;
		return this;
	}
}

//------------------------------------------------------------------------------------------------
//! One moment of a scene.
class RZ_DutyBeat
{
	static const int AT_SET = 0;		// m_vAt is given
	static const int AT_HERE = 1;		// 3-8 m from the scene's home
	static const int AT_GATE = 2;		// 12-25 m
	static const int AT_OUT = 3;		// 60-140 m
	static const int AT_FAR = 4;		// 180-350 m
	static const int AT_SAME = 5;		// where the last beat was
	static const int AT_NEAR = 6;		// 12-30 m from where the last beat was (off the path)
	static const int AT_NEAR_DOOR = 9;	// a house's outside door 12-60 m from the last beat (else 5-150 m), on its doorstep: "the door of ..."
	static const int AT_FAR_DOOR = 8;	// a house's outside door 180-350 m off (else 100-600 m), on its doorstep: "the far door"
	static const int AT_ENTRY = 7;		// the way in: the place's gate, else a road where it comes in (RZ_DutyScenes.Entrance)

	string m_sKey;
	string m_sLine;				// said as the player reaches it
	string m_sName;				// its spot's name, the one prompt when its options are together ("The bed"); "" from the line (RZ_DutyScenes.SpotName)
	string m_sGoal;				// the tracker's objective from this beat on, till the next beat with one (docs/UI.md, "The tracker"); "" carries on

	int m_iWhere = AT_SAME;
	vector m_vAt;
	ResourceName m_sProp;		// what stands there (the first option's before look)
	string m_sIf;				// a flag the scene must have ("!flag": must not); "" always
	float m_fWait;				// seconds after the last beat before it opens (a watch's sightings)
	float m_fTimeout;			// > 0: left alone this long, it passes with nothing chosen
	bool m_bScored;				// counts in the scene's score
	bool m_bAlone;				// nobody of the place is watching here (a night post): only their people with you see
	int m_iPicks = 1;			// how many of its options may be taken before it moves on (looking round: fewer looks than places)
	int m_iPicked;
	string m_sGroup;			// the interaction it belongs to (its flags are its own)
	string m_sPassed;			// said when it's left alone past its timeout ("" the plain line)
	string m_sTalk;				// someone stands here to talk to: their conversations are for duty:<this> ("swap:ring")
	string m_sTalkName;			// who they are, the talk screen's name ("A woman with a ring")
	ResourceName m_sThing;		// what stands on the table or the ground in front (a display copy, RZ_DutyThings)
	int m_iBody = -1;			// the scene's own person who is already there (m_aBodies: a patient in a bed) is this beat's person to talk to
	float m_fFace = -1;			// >= 0: the bearing of the side to stand on (a bed against a wall): the options set out (m_fOut) spread along that side only
	ref array<ref RZ_DutyOption> m_aOptions = {};

	//------------------------------------------------------------------------------------------------
	void RZ_DutyBeat(string key, string line, int where = 5)
	{
		m_sKey = key;
		m_sLine = line;
		m_iWhere = where;
	}

	//------------------------------------------------------------------------------------------------
	RZ_DutyOption Opt(string label, string said)
	{
		RZ_DutyOption o = new RZ_DutyOption();
		o.m_sLabel = label;
		o.m_sSaid = said;
		m_aOptions.Insert(o);
		return o;
	}
}

//------------------------------------------------------------------------------------------------
//! One duty's scene, running.
class RZ_DutyScene
{
	int m_iErrand;
	int m_iPlayer;
	int m_iFaction = -1;
	string m_sTitle;
	string m_sGiver;				// the giver's key (regard)
	string m_sGiverTitle;
	int m_iHouse = -1;
	vector m_vHome;
	vector m_vLast;					// where the last beat was
	bool m_bLoose;					// beats are pushed as they come (a patrol's finds): never done by itself
	ref array<ref RZ_DutyBeat> m_aBeats = {};
	int m_iBeat;					// the current one
	bool m_bOpen;
	bool m_bSaid;
	bool m_bDone;
	float m_fClock;
	float m_fOpenAt;
	float m_fOpened;
	ref array<string> m_aFlags = {};
	ref array<string> m_aKeys = {};	// the open beat's work sites
	ref array<string> m_aTold = {};	// for the giver, when it's reported
	int m_iFavor;					// the choices' sum
	int m_iScored;
	int m_iRight;
	int m_iWrong;
	int m_iReported;				// finds put in the report
	int m_iSeen;					// shady choices that were seen
	IEntity m_Person;				// the open beat's person to talk to (m_sTalk), only while it's open
	IEntity m_Thing;				// the open beat's thing on show (m_sThing), only while it's open
	ResourceName m_sDress;			// what stands beside the scene's first beat until it's over (the lock-up's pen)
	ref map<string, int> m_mStock = new map<string, int>();	// a line's stocks by count (fill:water=4, use:water)
	int m_iCarry;					// carrying something by its rules (1 time, 2 no running or drops): the cold chain
	float m_fCarryFrom;
	float m_fJoltAt;
	vector m_vCarryLast;
	int m_iJolts;
	bool m_bLate;
	IEntity m_Dress;
	ref array<IEntity> m_aBodies = {};	// people the scene put there itself, lying where they are from start to end (The Round's sick)
	ref array<IEntity> m_aProps = {};	// what the scene stood there itself (their beds, the tent), until it's dropped

	//------------------------------------------------------------------------------------------------
	bool Has(string flag)
	{
		return m_aFlags.Contains(flag);
	}

	//------------------------------------------------------------------------------------------------
	//! Nothing open and nothing waiting (a patrol can walk on).
	bool IsIdle()
	{
		return !m_bOpen && m_iBeat >= m_aBeats.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! 0..1 over the scored beats (1 with none).
	float Score()
	{
		if (m_iScored <= 0)
			return 1;

		float right = m_iRight;
		return right / m_iScored;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_DutyScenes
{
	protected static const float SAY_RANGE = 9;			// a beat's line is said this close
	protected static const float SEEN_SQUAD = 20;		// their people see what your hands do within this
	protected static const float SEEN_HOME = 25;		// and so does the place
	protected static const float DOORSTEP = 1.5;		// a door beat stands this far out from the doorway
	protected static const float SAME_SPOT = 3;			// an option set out no further than this is still at the beat's spot (round a bed)
	protected static const float OPTION_STEP = 0.9;		// between a beat's options, side by side
	static const float FACE_ARC = 140;					// degrees an open side's options spread over
	static const int TAG_PERSON = -18;					// RZ_PuzzleObjectComponent mission id: a duty's person to talk to (object index: the errand)
	protected static const float PERSON_OFF = 1.8;
	protected static const float DRESS_OFF = 6;
	protected static const float CARRY_RUN = 5.2;		// m in a second: a sprint, or a ride
	protected static const float CARRY_DROP = 1.6;		// m up or down in a second: a jump, a fall, a scramble
	protected static const float CARRY_TIME = 360;		// s on the road before what can't wait warms			// the lock-up's pen stands this far to the side		// a beat's person stands this far behind its options

	protected RZ_GameModeComponent m_GM;
	protected RZ_Errands m_Dealer;
	protected ref array<ref RZ_DutyScene> m_aScenes = {};

	//------------------------------------------------------------------------------------------------
	void RZ_DutyScenes(RZ_GameModeComponent gm, RZ_Errands dealer)
	{
		m_GM = gm;
		m_Dealer = dealer;
	}

	//------------------------------------------------------------------------------------------------
	RZ_DutyScene Find(int errandId)
	{
		foreach (RZ_DutyScene s : m_aScenes)
		{
			if (s.m_iErrand == errandId)
				return s;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The scene for this errand, made if there's none. `loose`: beats are pushed as they come.
	RZ_DutyScene Ensure(RZ_Errand e, string title, vector home, bool loose)
	{
		RZ_DutyScene s = Find(e.m_iId);
		if (s)
			return s;

		s = new RZ_DutyScene();
		s.m_iErrand = e.m_iId;
		s.m_iPlayer = e.m_iAsker;
		s.m_iFaction = e.m_iFaction;
		s.m_sTitle = title;
		s.m_sGiver = e.m_sGiver;
		s.m_sGiverTitle = e.m_sGiverTitle;
		s.m_iHouse = e.m_iHouse;
		s.m_vHome = home;
		s.m_vLast = home;
		s.m_bLoose = loose;
		m_aScenes.Insert(s);
		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! Done: every beat played (a loose scene: nothing open or waiting).
	bool IsDone(int errandId)
	{
		RZ_DutyScene s = Find(errandId);
		if (!s)
			return true;

		if (s.m_bLoose)
			return s.IsIdle();

		return s.m_bDone;
	}

	//------------------------------------------------------------------------------------------------
	//! The scene's steps for the tracker, one objective each (docs/WRITING.md, "Objective lines"):
	//! every beat with a goal starts one. Those behind are ticked, the one under way is current at
	//! its beat's place, those ahead are dim (one that waits on a flag the scene hasn't got is left
	//! out). All ticked when `done`. False: there's no scene or it names no goals.
	bool Objectives(RZ_Errand e, notnull RZ_ObjList list, float radius, bool done)
	{
		RZ_DutyScene s = Find(e.m_iId);
		if (!s)
			return false;

		int count = s.m_aBeats.Count();
		int at = s.m_iBeat;
		if (s.m_bDone || done)
			at = count;

		// The step under way: the last goal at or before the current beat (the first, before any)
		int current = -1;
		int first = -1;
		foreach (int i, RZ_DutyBeat b : s.m_aBeats)
		{
			if (b.m_sGoal.IsEmpty())
				continue;

			if (first < 0)
				first = i;

			if (i <= at)
				current = i;
		}

		if (first < 0)
			return false;

		if (current < 0)
			current = first;

		vector here = e.m_vPlace;
		if (at < count && s.m_aBeats[at].m_vAt != vector.Zero)
			here = s.m_aBeats[at].m_vAt;

		foreach (int j, RZ_DutyBeat g : s.m_aBeats)
		{
			if (g.m_sGoal.IsEmpty())
				continue;

			if (j != current && !Holds(s, g))
				continue;

			string text = e.Fill(g.m_sGoal);
			if (j < current || (j == current && at >= count))
				list.Add(text, RZ_EObjState.DONE);
			else if (j == current)
				list.Add(text, RZ_EObjState.CURRENT, RZ_ObjPlace.Spot(here, radius));
			else
				list.Add(text, RZ_EObjState.NEXT);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void Drop(int errandId)
	{
		for (int i = m_aScenes.Count() - 1; i >= 0; i--)
		{
			if (m_aScenes[i].m_iErrand != errandId)
				continue;

			CloseBeat(m_aScenes[i]);
			if (m_aScenes[i].m_Dress)
				RZ_Util.DeleteUnit(m_aScenes[i].m_Dress);

			foreach (IEntity body : m_aScenes[i].m_aBodies)
			{
				if (body)
					RZ_Util.DeleteUnit(body);
			}

			foreach (IEntity prop : m_aScenes[i].m_aProps)
			{
				if (prop)
					SCR_EntityHelper.DeleteEntityAndChildren(prop);
			}

			m_aScenes.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CloseBeat(RZ_DutyScene s)
	{
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (sites)
		{
			sites.Scatter(SpotKey(s));
			foreach (string key : s.m_aKeys)
			{
				sites.Close(key);
			}
		}

		s.m_aKeys.Clear();
		s.m_bOpen = false;
		// The scene's own people stay where they lie; a beat's caller goes
		if (s.m_Person && !s.m_aBodies.Contains(s.m_Person))
			RZ_Util.DeleteUnit(s.m_Person);

		if (s.m_Thing)
			RZ_Util.DeleteUnit(s.m_Thing);

		s.m_Person = null;
		s.m_Thing = null;
	}

	// ================================================================ Talking at a beat
	//------------------------------------------------------------------------------------------------
	//! The scene whose open beat has this person to talk to (RZ_TalkDutyPerson). null if it's gone.
	RZ_DutyScene ByPerson(int errandId)
	{
		RZ_DutyScene s = Find(errandId);
		if (!s || !s.m_bOpen || s.m_iBeat >= s.m_aBeats.Count() || !s.m_Person)
			return null;

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! The open beat of this scene (null when none is open).
	static RZ_DutyBeat OpenBeatOf(RZ_DutyScene s)
	{
		if (!s || !s.m_bOpen || s.m_iBeat >= s.m_aBeats.Count())
			return null;

		return s.m_aBeats[s.m_iBeat];
	}

	//------------------------------------------------------------------------------------------------
	//! A conversation at the beat's person picked option `index` (call:slot:N): the same as doing it
	//! with your hands at its work site. Only the scene's own player; not an option already done, nor
	//! a look past the beat's picks. What follows is said into the talk screen.
	bool TalkPick(int errandId, int index, int playerId, IEntity user)
	{
		RZ_DutyScene s = ByPerson(errandId);
		if (!s || s.m_iPlayer != playerId)
			return false;

		RZ_DutyBeat b = OpenBeatOf(s);
		if (!b || index < 0 || index >= b.m_aOptions.Count())
			return false;

		RZ_DutyOption o = b.m_aOptions[index];
		if (o.m_bDone)
		{
			m_GM.NotifyPlayer(s.m_iPlayer, s.m_sTitle, "You've asked that already.");
			return false;
		}

		if (b.m_iPicks > 1 && !o.m_bEnds && b.m_iPicked >= b.m_iPicks)
		{
			m_GM.NotifyPlayer(s.m_iPlayer, s.m_sTitle, "There's no more to find out. Make the call.");
			return false;
		}

		if (o.m_iItems > 0 && RZ_Errands.CountItems(user, o.m_eItem) < o.m_iItems)
		{
			m_GM.NotifyPlayer(s.m_iPlayer, s.m_sTitle, string.Format("You haven't got that on you: %1.", RZ_Errands.ItemNeed(o.m_eItem, o.m_iItems)));
			return false;
		}

		s.m_bSaid = true;
		Choose(s, b, o, user);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Why option `index` of this person's beat can't be picked now ("" it can): a conversation's
	//! reply that runs it shows locked with it.
	string TalkLock(int errandId, int index, int playerId)
	{
		RZ_DutyScene s = ByPerson(errandId);
		RZ_DutyBeat b = OpenBeatOf(s);
		if (!b || s.m_iPlayer != playerId)
			return "Not yours to settle";

		if (index < 0 || index >= b.m_aOptions.Count())
			return "Not here";

		RZ_DutyOption o = b.m_aOptions[index];
		if (o.m_bDone)
			return "Asked already";

		if (!o.m_bEnds && b.m_iPicks > 1 && b.m_iPicked >= b.m_iPicks)
			return "No more to find out";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The person at a beat: a civilian (Missions > Errands > Props > Duty person, else Puzzles >
	//! Civilian) a step behind its options, facing them, entered in the body budget as an errand's,
	//! and tagged so the talk screen knows whose they are. With no room in the budget, no prefab, or
	//! no conversation trees, nobody stands there: the beat is done with your hands at its work sites.
	protected void PlacePerson(RZ_DutyScene s, RZ_DutyBeat b, vector at, float yaw)
	{
		// Someone the scene put there already (a patient in their bed) is the one to talk to
		if (b.m_iBody >= 0 && b.m_iBody < s.m_aBodies.Count() && s.m_aBodies[b.m_iBody])
		{
			s.m_Person = s.m_aBodies[b.m_iBody];
			return;
		}

		if (b.m_sTalk.IsEmpty() || !RZ_DialogueTrees.Get())
			return;

		ResourceName prefab = RZ_ErrandSettings.Get().Props().m_sDutyPerson;
		if (prefab.IsEmpty())
			prefab = m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);

		RZ_Shared shared = m_GM.GetShared();
		if (prefab.IsEmpty() || (shared && !shared.CanSpawnBody(RZ_BodyBudget.ERRAND, at)))
			return;

		s.m_Person = RZ_Util.SpawnPrefab(prefab, RZ_Util.GroundPos(at), yaw);
		if (!s.m_Person)
			return;

		if (shared)
			shared.AddBody(RZ_BodyBudget.ERRAND, s.m_Person);

		RZ_PuzzleObjectComponent.Tag(s.m_Person, TAG_PERSON, s.m_iErrand, "", "");
	}

	//------------------------------------------------------------------------------------------------
	//! What's on show at the beat: a display copy (nothing to pick up), on the ground in front.
	protected void PlaceThing(RZ_DutyScene s, RZ_DutyBeat b, vector at, float yaw)
	{
		if (b.m_sThing.IsEmpty())
			return;

		s.m_Thing = RZ_Util.SpawnPrefab(b.m_sThing, RZ_Util.GroundPos(at) + Vector(0, 0.05, 0), yaw);
	}

	//------------------------------------------------------------------------------------------------
	//! What stands beside the scene (the lock-up's pen or room), on the ground, once: it stays until
	//! the errand is dropped.
	protected void PlaceDress(RZ_DutyScene s, vector at, float yaw)
	{
		if (s.m_Dress || s.m_sDress.IsEmpty())
			return;

		s.m_Dress = RZ_Util.SpawnPrefab(s.m_sDress, RZ_Util.GroundPos(at), yaw);
	}

	// ================================================================ Running
	//------------------------------------------------------------------------------------------------
	//! Once a second: the next beat opens at its spot, its line is said as the player comes up, and
	//! what their hands chose is applied.
	void SecondTick(array<int> ids, array<IEntity> players)
	{
		foreach (RZ_DutyScene s : m_aScenes)
		{
			s.m_fClock += 1;
			if (s.m_bDone)
				continue;

			int idx = ids.Find(s.m_iPlayer);
			if (idx < 0)
				continue;

			Step(s, players[idx]);
			Carry(s, players[idx]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Carrying something with rules you feel (the cold chain): sprinting, a drop or a climb of more
	//! than a man's height in a second jolts it, and too long on the road warms it. Each counts as a
	//! wrong call, two at most, and is said once.
	protected void Carry(RZ_DutyScene s, IEntity me)
	{
		if (s.m_iCarry <= 0 || !me)
			return;

		vector at = me.GetOrigin();
		if (s.m_vCarryLast != vector.Zero && s.m_fClock - s.m_fJoltAt > 6 && (s.m_iCarry & 2) != 0)
		{
			float moved = RZ_Util.DistXZ(at, s.m_vCarryLast);
			float drop = Math.AbsFloat(at[1] - s.m_vCarryLast[1]);
			if (moved > CARRY_RUN || drop > CARRY_DROP)
			{
				s.m_fJoltAt = s.m_fClock;
				s.m_iJolts++;
				if (s.m_iJolts <= 2)
				{
					s.m_iScored++;
					s.m_iWrong++;
					m_GM.NotifyPlayer(s.m_iPlayer, s.m_sTitle, "Everything in the box rings at once. That was too hard for it.");
				}
			}
		}

		s.m_vCarryLast = at;
		if (!s.m_bLate && (s.m_iCarry & 1) != 0 && s.m_fClock - s.m_fCarryFrom > CARRY_TIME)
		{
			s.m_bLate = true;
			s.m_iScored++;
			s.m_iWrong++;
			m_GM.NotifyPlayer(s.m_iPlayer, s.m_sTitle, "The box is warm through the canvas now. It's been on the road too long.");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Step(RZ_DutyScene s, IEntity me)
	{
		RZ_WorkSites sites = RZ_WorkSites.Get();
		// Skip beats whose condition doesn't hold
		while (!s.m_bOpen && s.m_iBeat < s.m_aBeats.Count() && !Holds(s, s.m_aBeats[s.m_iBeat]))
		{
			s.m_iBeat++;
		}

		if (s.m_iBeat >= s.m_aBeats.Count())
		{
			if (!s.m_bLoose)
				s.m_bDone = true;

			return;
		}

		RZ_DutyBeat b = s.m_aBeats[s.m_iBeat];
		if (!s.m_bOpen)
		{
			if (s.m_fOpenAt <= 0)
				s.m_fOpenAt = s.m_fClock + b.m_fWait;

			if (s.m_fClock < s.m_fOpenAt)
				return;

			OpenBeat(s, b);
			PointTo(s, b, me);
			return;
		}

		vector at = me.GetOrigin();
		if (!s.m_bSaid && RZ_Util.DistXZ(at, b.m_vAt) <= SAY_RANGE)
		{
			s.m_bSaid = true;
			if (!b.m_sLine.IsEmpty())
				m_GM.NotifyPlayer(s.m_iPlayer, s.m_sTitle, b.m_sLine);
		}

		// No work sites here: the first thing you could do is what's done, by standing there
		if (!sites || !RZ_WorkSites.Available())
		{
			if (s.m_bSaid && !b.m_aOptions.IsEmpty())
				Choose(s, b, b.m_aOptions[0], me);
			else if (s.m_bSaid)
				Advance(s, "");

			return;
		}

		foreach (int i, RZ_DutyOption o : b.m_aOptions)
		{
			string key = Key(s, i);
			if (o.m_bDone || !sites.IsDone(key))
				continue;

			if (o.m_iItems > 0 && RZ_Errands.CountItems(me, o.m_eItem) < o.m_iItems)
			{
				m_GM.NotifyPlayer(s.m_iPlayer, s.m_sTitle, string.Format("You haven't got that on you: %1.", RZ_Errands.ItemNeed(o.m_eItem, o.m_iItems)));
				vector spot = sites.PosOf(key);
				sites.Close(key);
				sites.Open(key, o.m_eWork, o.m_sLabel, o.m_fHold, spot, s.m_iPlayer);
				sites.SetNeeds(key, o.m_eItem, o.m_iItems);
				continue;
			}

			Choose(s, b, o, me);
			return;
		}

		if (b.m_fTimeout > 0 && s.m_fClock - s.m_fOpened > b.m_fTimeout)
		{
			string passed = b.m_sPassed;
			if (passed.IsEmpty())
				passed = "Left as it was. Time to move on.";

			m_GM.NotifyPlayer(s.m_iPlayer, s.m_sTitle, passed);
			Advance(s, "");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Holds(RZ_DutyScene s, RZ_DutyBeat b)
	{
		if (b.m_sIf.IsEmpty())
			return true;

		if (b.m_sIf.StartsWith("!"))
			return !s.Has(b.m_sGroup + b.m_sIf.Substring(1, b.m_sIf.Length() - 1));

		return s.Has(b.m_sGroup + b.m_sIf);
	}

	//------------------------------------------------------------------------------------------------
	protected static string Key(RZ_DutyScene s, int option)
	{
		return string.Format("errand.%1.b%2.%3", s.m_iErrand, s.m_iBeat, option);
	}

	//------------------------------------------------------------------------------------------------
	//! The key of the beat's spot when its options stand as one prompt (RZ_WorkSites.Gather).
	protected static string SpotKey(RZ_DutyScene s)
	{
		return string.Format("errand.%1.b%2.spot", s.m_iErrand, s.m_iBeat);
	}

	//------------------------------------------------------------------------------------------------
	//! The name of a beat's spot, for its one prompt: its own name, else who stands there to talk to,
	//! else what the line names first when that's a thing ("The far door. No lamp over it." gives
	//! "The far door"), else the duty's title.
	static string SpotName(RZ_DutyScene s, RZ_DutyBeat b)
	{
		if (!b.m_sName.IsEmpty())
			return b.m_sName;

		if (!b.m_sTalkName.IsEmpty())
			return b.m_sTalkName;

		string named = NameIn(b.m_sLine);
		if (!named.IsEmpty())
			return named;

		return s.m_sTitle;
	}

	//------------------------------------------------------------------------------------------------
	//! The thing a line names first: its words up to the first stop or comma, when they're a short
	//! naming ("The far door", "The book", "The list", "A hollow under a fallen beech") and not a
	//! sentence ("The bridge is out"). "" when they aren't.
	static string NameIn(string line)
	{
		int cut = line.Length();
		array<string> stops = {".", ",", ":", "?", "!"};
		foreach (string stop : stops)
		{
			int at = line.IndexOf(stop);
			if (at >= 0 && at < cut)
				cut = at;
		}

		string head = line.Substring(0, cut).Trim();
		if (!head.StartsWith("The ") && !head.StartsWith("A ") && !head.StartsWith("An "))
			return "";

		array<string> words = {};
		head.Split(" ", words, true);
		if (words.Count() < 2 || words.Count() > 6)
			return "";

		array<string> verbs = {"is", "are", "has", "have", "was", "were", "turns", "puts", "says", "comes", "stops", "steps", "rings", "drops", "looks", "holds"};
		foreach (string w : words)
		{
			if (verbs.Contains(w) || w.Contains("'s"))
				return "";
		}

		return head;
	}

	//------------------------------------------------------------------------------------------------
	//! A beat at the way in (AT_ENTRY): by the gate or the road in, logged.
	protected vector EntrySpot(RZ_DutyScene s, RZ_DutyBeat b)
	{
		vector gate;
		vector inside;
		string how = Entrance(s.m_vHome, gate, inside);
		RZ_Log.Info(string.Format("Duty scene: errand %1, %2 at the way in: %3 (%4).", s.m_iErrand, b.m_sKey, how, RZ_SettlementScanner.GridReference(gate)));
		return RZ_WorkSites.Spot(gate, 1, 4);
	}

	//------------------------------------------------------------------------------------------------
	//! The way into the place at `home`, for what's said at a gate (a line, a barrier, a toll): the
	//! faction place's own gate within 120 m (its road entry, RZ_CampLife.GateNear); else the road
	//! where it comes into the town, the road point 25-120 m from `home` farthest from the town's
	//! middle; else `home` itself (the giver's own door). `inside` is a few steps in from it. Says
	//! which, for the log.
	static string Entrance(vector home, out vector gate, out vector inside)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm && gm.GetCampLife() && gm.GetCampLife().GateNear(home, 120, gate, inside))
		{
			inside = Toward(gate, inside, 6);
			return "the place's gate";
		}

		vector middle = home;
		if (gm)
		{
			RZ_Settlement town = gm.GetSettlement(gm.FindNearestSettlement(home));
			if (town)
				middle = town.m_vCenter;
		}

		array<vector> points = {};
		RZ_Perimeter.RoadPointsNear(home, 120, points);
		float best = -1;
		foreach (vector p : points)
		{
			float d = RZ_Util.DistXZ(p, home);
			if (d < 25 || RZ_Util.IsWater(p))
				continue;

			float outward = RZ_Util.DistXZ(p, middle);
			if (outward > best)
			{
				best = outward;
				gate = RZ_Util.GroundPos(p);
			}
		}

		if (best >= 0)
		{
			inside = Toward(gate, home, 6);
			return "the road into the town";
		}

		gate = home;
		inside = home;
		return "no gate or road in near: the giver's own door";
	}

	//------------------------------------------------------------------------------------------------
	//! `dist` m from `from` toward `to` (on the ground), or `from` when they're the same spot.
	static vector Toward(vector from, vector to, float dist)
	{
		vector flat = Vector(to[0] - from[0], 0, to[2] - from[2]);
		float len = flat.Length();
		if (len < 0.1)
			return from;

		return RZ_Util.GroundPos(from + flat * (Math.Min(dist, len) / len));
	}

	//------------------------------------------------------------------------------------------------
	//! Where a beat is, from the scene's home or the last beat.

	protected vector Place(RZ_DutyScene s, RZ_DutyBeat b)
	{
		RZ_PlaceNeed need = RZ_PlaceNeeds.Get("errand.out");
		vector at;
		switch (b.m_iWhere)
		{
			case RZ_DutyBeat.AT_SET: return b.m_vAt;
			case RZ_DutyBeat.AT_HERE: return RZ_WorkSites.Spot(s.m_vHome, 3, 8);
			case RZ_DutyBeat.AT_GATE: return RZ_WorkSites.Spot(s.m_vHome, 12, 25);
			case RZ_DutyBeat.AT_ENTRY: return EntrySpot(s, b);
			case RZ_DutyBeat.AT_OUT: at = RZ_PlaceFind.Near(s.m_vHome, 60, 140, need); break;
			case RZ_DutyBeat.AT_FAR: at = RZ_PlaceFind.Near(s.m_vHome, 180, 350, need); break;
			case RZ_DutyBeat.AT_NEAR: return RZ_WorkSites.Spot(s.m_vLast, 12, 30);
			case RZ_DutyBeat.AT_NEAR_DOOR:
			{
				if (DoorStep(s.m_vLast, 12, 60, at) || DoorStep(s.m_vLast, 5, 150, at))
					return at;

				return RZ_WorkSites.Spot(s.m_vLast, 12, 30);
			}
			case RZ_DutyBeat.AT_FAR_DOOR:
			{
				if (DoorStep(s.m_vHome, 180, 350, at) || DoorStep(s.m_vHome, 100, 600, at))
					return at;

				at = RZ_PlaceFind.Near(s.m_vHome, 180, 350, need);
				break;
			}
			case RZ_DutyBeat.AT_SAME: return s.m_vLast;
		}

		if (at == vector.Zero)
			at = RZ_WorkSites.Spot(s.m_vHome, 20, 40);

		return at;
	}

	//------------------------------------------------------------------------------------------------
	//! The doorstep of a house's outside door lo-hi m from `home` (the door finder the locks and
	//! Traces use, RZ_DoorGeometry): a beat whose words name a door, a step or a hatch stands there.
	protected static bool DoorStep(vector home, float lo, float hi, out vector at)
	{
		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(home, hi, buildings, 200);
		array<IEntity> houses = {};
		foreach (IEntity b : buildings)
		{
			float d = RZ_Util.DistXZ(home, b.GetOrigin());
			if (d >= lo && d <= hi && RZ_PlaceCheck.IsHouse(b) && !RZ_LockedSpots.IsLockedBuilding(b))
				houses.Insert(b);
		}

		for (int tries = 0; tries < 8 && !houses.IsEmpty(); tries++)
		{
			int pick = Math.RandomInt(0, houses.Count());
			IEntity house = houses[pick];
			houses.Remove(pick);
			array<IEntity> doors = {};
			RZ_DoorGeometry.FindDoors(house, 0, doors);
			foreach (IEntity door : doors)
			{
				if (!RZ_DoorGeometry.IsOuterDoor(house, door))
					continue;

				vector normal = RZ_DoorGeometry.WallNormal(house, door);
				if (normal == vector.Zero)
					continue;

				vector step = RZ_Util.GroundPos(RZ_DoorGeometry.DoorwayCentre(door) + normal * DOORSTEP);
				if (RZ_PlaceCheck.IsWet(step))
					continue;

				at = step;
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenBeat(RZ_DutyScene s, RZ_DutyBeat b)
	{
		b.m_vAt = Place(s, b);
		s.m_vLast = b.m_vAt;
		s.m_bOpen = true;
		s.m_bSaid = false;
		s.m_fOpened = s.m_fClock;
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (!sites || !RZ_WorkSites.Available())
			return;

		float yaw = Math.RandomFloat(0, 360);
		float rad = yaw * Math.DEG2RAD;
		vector axis = Vector(Math.Cos(rad), 0, -Math.Sin(rad));
		int count = b.m_aOptions.Count();
		array<string> together = {};		// the options at the one spot: one prompt and a list
		vector middle = vector.Zero;
		foreach (int i, RZ_DutyOption o : b.m_aOptions)
		{
			vector at = b.m_vAt + axis * ((i - (count - 1) * 0.5) * OPTION_STEP);
			// A place of its own to go and look at: out from the beat, spread round it
			if (o.m_fOut > 0 && b.m_fFace >= 0)
			{
				// Along one side, over FACE_ARC degrees centred on it (a bearing: clockwise from north)
				float spread = 0;
				if (count > 1)
					spread = (i / (count - 1.0) - 0.5) * FACE_ARC;

				float bearing = (b.m_fFace + spread) * Math.DEG2RAD;
				at = b.m_vAt + Vector(Math.Sin(bearing), 0, Math.Cos(bearing)) * o.m_fOut;
			}
			else if (o.m_fOut > 0)
			{
				float turn = (yaw + i * 360 / count) * Math.DEG2RAD;
				at = b.m_vAt + Vector(Math.Cos(turn), 0, -Math.Sin(turn)) * o.m_fOut;
			}

			string key = Key(s, i);
			ResourceName prop;
			if (i == 0)
				prop = b.m_sProp;

			// A thing to take stands there: the duty tray (a case of vials), else the crate; with both
			// empty, the work sites' own base-game box
			if (prop.IsEmpty() && o.m_eWork == RZ_EWork.TAKE)
			{
				prop = RZ_ErrandSettings.Get().Props().m_sDutyTray;
				if (prop.IsEmpty())
					prop = RZ_ErrandSettings.Get().Props().m_sCrate;
			}

			sites.Open(key, o.m_eWork, o.m_sLabel, o.m_fHold, at, s.m_iPlayer, prop, "", yaw);
			sites.SetNeeds(key, o.m_eItem, o.m_iItems);
			s.m_aKeys.Insert(key);
			if (o.m_fOut <= SAME_SPOT)
			{
				together.Insert(key);
				middle = middle + at;
			}
		}

		// Several things to do at the one spot (side by side, or round a bed): one prompt where they
		// stood, its name, and the list. A place of its own to go and look at keeps its own prompt.
		if (together.Count() > 1)
			sites.Gather(SpotKey(s), SpotName(s, b), together, middle * (1.0 / together.Count()), b.m_sLine, yaw);

		// Someone to talk to a step behind the options; what's on show a step in front
		vector back = Vector(Math.Sin(rad), 0, Math.Cos(rad));
		PlacePerson(s, b, b.m_vAt - back * PERSON_OFF, yaw);
		PlaceThing(s, b, b.m_vAt + back, yaw);
		PlaceDress(s, b.m_vAt + axis * DRESS_OFF, yaw);
	}

	//------------------------------------------------------------------------------------------------
	//! Where the beat is, for the player: the errand's place moves to it (so the journal and the
	//! tracker point there, and the step ends where the last beat was), and a beat more than 15 m off
	//! is said with its direction. A patrol's finds leave the errand's place alone: its step ends at
	//! the gate.
	protected void PointTo(RZ_DutyScene s, RZ_DutyBeat b, IEntity me)
	{
		RZ_Errand e = m_Dealer.FindById(s.m_iErrand);
		if (e && !s.m_bLoose)
			e.m_vPlace = b.m_vAt;

		if (RZ_Util.DistXZ(me.GetOrigin(), b.m_vAt) > 15)
			m_GM.NotifyPlayer(s.m_iPlayer, s.m_sTitle, string.Format("%1.", RZ_Util.Capitalize(RZ_Util.DirectionText(me.GetOrigin(), b.m_vAt))));
	}

	//------------------------------------------------------------------------------------------------
	//! On to the beat named `next` ("" the next in order, "end" none).
	protected void Advance(RZ_DutyScene s, string next)
	{
		CloseBeat(s);
		s.m_fOpenAt = 0;
		if (next == "end")
		{
			s.m_iBeat = s.m_aBeats.Count();
			return;
		}

		if (next.IsEmpty())
		{
			s.m_iBeat++;
			return;
		}

		foreach (int i, RZ_DutyBeat b : s.m_aBeats)
		{
			if (b.m_sKey == next)
			{
				s.m_iBeat = i;
				return;
			}
		}

		s.m_iBeat++;
	}

	//------------------------------------------------------------------------------------------------
	//! Their people, or the place itself, can see what the player's hands do here.
	protected bool IsSeen(RZ_DutyScene s, RZ_DutyBeat b, vector at)
	{
		if (!b.m_bAlone && RZ_Util.DistXZ(at, s.m_vHome) <= SEEN_HOME)
			return true;

		RZ_DutySquad squad = m_Dealer.GetSquads().Find(s.m_iErrand);
		return squad && !squad.m_Escort.IsDead() && RZ_Util.DistXZ(squad.m_Escort.m_vPos, at) <= SEEN_SQUAD;
	}

	//------------------------------------------------------------------------------------------------
	protected void Choose(RZ_DutyScene s, RZ_DutyBeat b, RZ_DutyOption o, IEntity me)
	{
		vector at = me.GetOrigin();
		if (o.m_iItems > 0)
			RZ_Errands.TakeItems(me, o.m_eItem, o.m_iItems);

		if (b.m_bScored)
		{
			s.m_iScored++;
			if (o.m_iRight > 0)
				s.m_iRight++;
			else if (o.m_iRight < 0)
				s.m_iWrong++;
		}

		if (!o.m_sFlag.IsEmpty() && !s.Has(b.m_sGroup + o.m_sFlag))
			s.m_aFlags.Insert(b.m_sGroup + o.m_sFlag);

		string said = o.m_sSaid;
		bool seen = o.m_bShady && IsSeen(s, b, at);
		if (o.m_bShady && !seen)
		{
			s.m_iFavor += o.m_iFavor;
		}
		else if (o.m_bShady)
		{
			s.m_iSeen++;
			said += " You were seen, and it was undone.";
			if (RZ_Factions.IsValid(s.m_iFaction) && o.m_iInfamy > 0)
				m_GM.AddInfamy(s.m_iPlayer, s.m_iFaction, o.m_iInfamy, "seen on duty: " + s.m_sTitle);

			Regard(s, "broke");
			if (s.m_aTold.Count() < 2)
				s.m_aTold.Insert("They told me what you did out there. I'd have told me first, in your place.");
		}
		else
		{
			s.m_iFavor += o.m_iFavor;
			if (o.m_iInfamy > 0 && RZ_Factions.IsValid(s.m_iFaction))
				m_GM.AddInfamy(s.m_iPlayer, s.m_iFaction, o.m_iInfamy, "on duty: " + s.m_sTitle);
		}

		if (o.m_iFame > 0 && !seen)
			m_Dealer.SmallFame(s.m_iPlayer, s.m_iFaction, o.m_iFame);

		if (!seen)
			Apply(s, b, o, me);

		if (!o.m_sTold.IsEmpty() && !seen && s.m_aTold.Count() < 2)
			s.m_aTold.Insert(o.m_sTold);

		if (!said.IsEmpty())
			m_GM.NotifyPlayer(s.m_iPlayer, s.m_sTitle, said);

		RZ_Log.Info(string.Format("Duty scene: errand %1, %2 at %3: %4 (seen %5).", s.m_iErrand, RZ_Util.PlayerName(s.m_iPlayer), b.m_sKey, o.m_sLabel, seen));
		// A beat with several picks (places to look) stays until they're used up; then only what
		// ends it is left to do, or it moves on if nothing does
		o.m_bDone = true;
		if (b.m_iPicks > 1 && !o.m_bEnds)
		{
			RZ_WorkSites sites = RZ_WorkSites.Get();
			b.m_iPicked++;
			if (sites)
				sites.Close(Key(s, b.m_aOptions.Find(o)));

			bool ending = false;
			foreach (int i, RZ_DutyOption other : b.m_aOptions)
			{
				if (other.m_bEnds && !other.m_bDone)
					ending = true;
				else if (b.m_iPicked >= b.m_iPicks && !other.m_bDone && sites)
					sites.Close(Key(s, i));
			}

			if (b.m_iPicked < b.m_iPicks || ending)
				return;
		}

		Advance(s, o.m_sNext);
	}

	//------------------------------------------------------------------------------------------------
	protected void Regard(RZ_DutyScene s, string word)
	{
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		if (trees && !s.m_sGiver.IsEmpty())
			trees.GetMemory().AddRegard(s.m_iPlayer, s.m_sGiver, word);
	}

	//------------------------------------------------------------------------------------------------
	//! A house fed by duties: `amount` food, on two duties a season day at most.
	static void Feed(RZ_Errands dealer, int house, int amount)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (house < 0 || !gm.GetSafehouses())
			return;

		string key = string.Format("duty.fed.%1.%2", house, RZ_SeasonClock.Now() / 86400);
		int fed = dealer.Counter(key);
		if (fed >= 2)
			return;

		dealer.SetCounter(key, fed + 1);
		gm.GetSafehouses().AddFood(house, amount);
	}

	//------------------------------------------------------------------------------------------------
	//! A choice's effects (m_sFx, comma separated): report (a find put in the report: the giver's
	//! regard, and 1 more Favor for the job, twice at most), regard+, regard-, feed, feed2, morale+,
	//! morale-, infect+, infect- (the town nearest, by 2), local (a local made at the nearest house, who
	//! remembers), later (a line said to you some hours on), rumour (a rumour for 6 hours), record (the
	//! world's memory), heat (noise where you stand), hook (someone hiding there, for the strangers),
	//! meat (a leg of meat in your pack), give (the place's small pay item in your pack), watch (Props >
	//! Watch, else that item), set:<counter>=<value> (one of the errands' saved counters).
	protected void Apply(RZ_DutyScene s, RZ_DutyBeat b, RZ_DutyOption o, IEntity me)
	{
		if (o.m_sFx.IsEmpty())
			return;

		array<string> fx = {};
		o.m_sFx.Split(",", fx, true);
		RZ_WorldFacts facts = m_GM.GetFacts();
		RZ_Settlement town = m_GM.GetSettlement(m_GM.FindNearestSettlement(b.m_vAt));
		array<int> who = {s.m_iPlayer};
		foreach (string f : fx)
		{
			// fill:<stock>=N: a stock of the scene's own, full (the decon line's tank); use:<stock>: one
			// used, and at none left the flag <stock>out is set for a beat that waits on it
			if (f.StartsWith("fill:"))
			{
				int at = f.IndexOf("=");
				if (at > 5)
				{
					string stock = f.Substring(5, at - 5);
					s.m_mStock.Set(stock, f.Substring(at + 1, f.Length() - at - 1).ToInt());
					s.m_aFlags.RemoveItem(stock + "out");
				}

				continue;
			}

			if (f.StartsWith("use:"))
			{
				string used = f.Substring(4, f.Length() - 4);
				int left = s.m_mStock.Get(used) - 1;
				s.m_mStock.Set(used, left);
				if (left <= 0 && !s.Has(used + "out"))
					s.m_aFlags.Insert(used + "out");

				continue;
			}

			// carry:<rules> starts carrying by them (1 time, 2 no jolts); carry:0 stops
			if (f.StartsWith("carry:"))
			{
				s.m_iCarry = f.Substring(6, f.Length() - 6).ToInt();
				s.m_fCarryFrom = s.m_fClock;
				s.m_vCarryLast = vector.Zero;
				continue;
			}

			// set:<counter>=<value>: one of the errands' saved counters (a garden bed's state)
			if (f.StartsWith("set:"))
			{
				int eq = f.IndexOf("=");
				if (eq > 4)
					m_Dealer.SetCounter(f.Substring(4, eq - 4), f.Substring(eq + 1, f.Length() - eq - 1).ToInt());

				continue;
			}

			switch (f)
			{
				case "report":
				{
					s.m_iReported++;
					if (s.m_iReported <= 2)
						s.m_iFavor += 1;

					Regard(s, "thanks");
					break;
				}

				case "regard+": Regard(s, "thanks"); break;
				case "regard-": Regard(s, "broke"); break;
				case "feed": Feed(m_Dealer, s.m_iHouse, 1); break;
				case "feed2": Feed(m_Dealer, s.m_iHouse, 2); break;
				case "morale+": m_Dealer.Morale(s.m_iFaction, 1, "duty"); break;
				case "morale-": m_Dealer.Morale(s.m_iFaction, -1, "duty"); break;
				case "infect+":
				{
					if (town)
						town.AddInfection(2);

					break;
				}

				case "infect-":
				{
					if (town)
						town.AddInfection(-2);

					break;
				}

				case "local":
				{
					int house = s.m_iHouse;
					if (house < 0)
						house = RZ_ErrandCard.HouseNear(m_GM, b.m_vAt, 2500, -1, 0);

					if (house >= 0 && m_Dealer.GetLocals())
						m_Dealer.GetLocals().Make(s.m_iPlayer, o.m_sWho, o.m_sLater, house, "gate", false, "helped");

					break;
				}

				case "later":
				{
					if (m_GM.GetShared() && m_GM.GetShared().GetCallbacks() && !o.m_sLater.IsEmpty())
						m_GM.GetShared().GetCallbacks().Queue(s.m_iPlayer, s.m_sGiverTitle, o.m_sLater, s.m_vHome);

					break;
				}

				case "rumour":
				{
					if (facts && !o.m_sLater.IsEmpty())
						facts.AddRumour(o.m_sLater, 6);

					break;
				}

				case "record":
				{
					if (facts)
						facts.Memory().Record("duty.find." + b.m_sKey, s.m_sGiver, o.m_sLabel, RZ_WorldMemory.SOURCE_ERRAND, false, who);

					break;
				}

				case "heat":
				{
					if (m_GM.GetHeat())
						m_GM.GetHeat().AddHeat(b.m_vAt, 4);

					break;
				}

				case "hook": m_Dealer.GetStrangers().PlantHook("hiding", b.m_vAt); break;
				case "meat":
				{
					RZ_WildlifeSettings wild = m_GM.GetWildlifeSettings();
					if (wild && !wild.m_sMeatPrefab.IsEmpty())
						RZ_ItemHelper.Give(wild.m_sMeatPrefab, me, me.GetOrigin());

					break;
				}

				case "watch":
				{
					ResourceName watch = RZ_ErrandSettings.Get().Props().m_sWatch;
					if (watch.IsEmpty())
						watch = RZ_ErrandSettings.Get().Props().m_sPayRation;

					if (!watch.IsEmpty())
						RZ_ItemHelper.Give(watch, me, me.GetOrigin());

					break;
				}

				case "vodka":
				{
					ResourceName bottle = RZ_ErrandSettings.Get().Props().m_sDutyVodka;
					if (!bottle.IsEmpty())
						RZ_ItemHelper.Give(bottle, me, me.GetOrigin());

					break;
				}

				// The ward's charts all true: a little research, once a place a season day
				case "research":
				{
					string key = string.Format("ward.research.%1", s.m_iHouse);
					int today = RZ_SeasonClock.Day();
					if (s.m_iWrong > 0 || !m_GM.GetCure() || m_Dealer.Counter(key) == today)
						break;

					m_Dealer.SetCounter(key, today);
					m_GM.GetCure().AddResearch(1, "The observation ward's charts came in true today, every bed. The lab counted them.");
					break;
				}

				// Someone to walk beside you (the toll's merchant): one body that follows, while it can
				case "walker":
				{
					RZ_Errand walked = m_Dealer.FindById(s.m_iErrand);
					if (walked && me)
						m_Dealer.GetSquads().Start(walked, s.m_sTitle, "the merchant", me.GetOrigin(), null, m_GM.GetSurvivorGroupPrefab());

					break;
				}

				case "walker_end": m_Dealer.GetSquads().Drop(s.m_iErrand); break;

				// One dose off a stock that exists, to someone of the place (never the player): the
				// black market's for the Red Hand, the lab's for the rest
				case "dose":
				{
					RZ_CureResearch cure = m_GM.GetCure();
					bool given = false;
					if (cure && s.m_iFaction == RZ_EFaction.BANDITS)
						given = cure.TakeStolenAntidote();
					else if (cure)
						given = cure.TakeLabAntidote();

					if (!given)
						m_GM.NotifyPlayer(s.m_iPlayer, s.m_sTitle, "The case is empty when it reaches the bed. Somebody got to the stock first.");

					break;
				}

				case "give":
				{
					RZ_ErrandProps props = RZ_ErrandSettings.Get().Props();
					ResourceName item = props.m_sPayRation;
					if (s.m_iFaction == RZ_EFaction.BANDITS && !props.m_sPayMagazine.IsEmpty())
						item = props.m_sPayMagazine;

					if (!item.IsEmpty())
						RZ_ItemHelper.Give(item, me, me.GetOrigin());

					break;
				}
			}
		}
	}
}
