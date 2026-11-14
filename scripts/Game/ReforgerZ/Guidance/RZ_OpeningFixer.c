//! The fixer (docs/DESIGN.md, "The fixer"): beat 5 of the opening, after the runner's case.
//!
//! Two to four minutes after you're back in the street you started from with the case (or on the
//! way, if you don't go back), Ferda Kroupa comes, who collects for the Red Hand's book, with two of
//! the crews. Polite, patient. He asks the contact first, then you, and you pick a side, each way in
//! the world: hand it over (a reply: he pays; the street thinks less of you; Proof opens as Get It
//! Back), talk him off (a reply with a check: your background or the good liar's habit), rally the
//! street (three neighbours' jobs at work sites, then a reply: he backs off), slip away (walk off
//! with it), or sell it and warn the street (the sale, then a reply to a neighbour). Firing on his
//! men is a fight. The radio mentions it once, that evening; afterwards he's a thread (his note on a
//! door near you, his men at a toll asking after you, his voice once on the crews' channel), never a
//! squad that hunts you. A familiar player (level 2) gets him as one scene where you are, a veteran
//! (3) only his note. What you did is kept on the profile for this character
//! (RZ_OpeningCase.CharacterStamp). Bodies only near the player; everyone is a man. Server only.

//------------------------------------------------------------------------------------------------
//! What you did with Ferda (RZ_PlayerProfile.m_iFerdaWay).
enum RZ_EFerdaWay
{
	NONE,
	HANDED,		// handed it over (sold to him)
	TALKED,		// talked him off: he'll be back to check
	RALLIED,	// the street stood with you
	SLIPPED,	// you walked off with it; the street had his visit
	WARNED,		// sold it, then warned the street
	FOUGHT,		// someone fired on his men
	NOTE,		// a veteran: only his note
	WAITED,		// he waited for the street too long and went
	GONE		// the case was gone before he came
}

//------------------------------------------------------------------------------------------------
//! A step of the fixer's: talk to him, the street's three jobs, stand with the street, warn it.
class RZ_FixerStepDef : RZ_ErrandDef
{
	int m_iKind;		// RZ_OpeningFixer.KIND_*

	//------------------------------------------------------------------------------------------------
	void RZ_FixerStepDef(string key, string title, int kind)
	{
		m_sKey = key;
		m_sTitle = title;
		m_iKind = kind;
		m_sFamily = "opening";
		m_bHanded = true;
		m_eAnchor = RZ_EErrandAnchor.WANDERER;
		m_fLapseMinutes = 30;
		Over("He's gone. It'll keep.");
	}

	//------------------------------------------------------------------------------------------------
	//! Handed, not asked for: tracked at once.
	override bool IsOpening()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool StepCounts(RZ_Errands dealer, RZ_Errand e, IEntity me)
	{
		RZ_OpeningFixer f = RZ_OpeningFixer.Get();
		return f && f.StepCounts(this);
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_OpeningFixer f = RZ_OpeningFixer.Get();
		if (f)
			f.OnStepDone(this, playerId);
	}
}

//------------------------------------------------------------------------------------------------
//! One player's visit. Runtime only: what it came to is kept on the profile.
class RZ_FixerVisit
{
	int m_iId;
	int m_iPlayer;
	int m_iLevel;
	int m_iPhase;
	bool m_bShort;				// a familiar player: one scene where you are, no street to rally
	float m_fDue;				// he comes this soon after you're back in the street...
	float m_fLatest;			// ...and meets you on the way at the latest
	float m_fPhaseAt;
	float m_fAway;				// how long you've been out of reach of him (slipping away)
	vector m_vTown;				// the street you started from
	string m_sTown;
	IEntity m_Contact;			// weak: the opening's contact, asked first
	string m_sContact;
	string m_sContactTitle;
	IEntity m_Ferda;			// weak
	ref array<IEntity> m_aCrew = {};
	IEntity m_Neighbour;		// weak: the man you warn, after a sale
	string m_sNeighbour;
	ref array<string> m_aNames = {};	// the street's three: the door, the rifle, the lookout
	int m_iErrand = -1;			// the step under way
	int m_iRally;				// the street's jobs done
	bool m_bGreeted;
	int m_iWay;
}

//------------------------------------------------------------------------------------------------
class RZ_OpeningFixer
{
	static const int TAG = -23;			// RZ_PuzzleObjectComponent / RZ_ClueComponent mission id: Ferda, a neighbour, his note
	static const int ROLE_FERDA = 0;
	static const int ROLE_NEIGHBOUR = 1;
	static const int NOTE_BASE = 500000;	// a note's clue index: this + the player it's for

	static const int KIND_TALK = 1;
	static const int KIND_BOARD = 2;
	static const int KIND_RIFLE = 3;
	static const int KIND_LOOKOUT = 4;
	static const int KIND_STAND = 5;
	static const int KIND_WARN = 6;

	static const int PHASE_WAIT = 0;
	static const int PHASE_TALK = 1;
	static const int PHASE_RALLY = 2;
	static const int PHASE_STAND = 3;
	static const int PHASE_WARN = 4;
	static const int PHASE_LEAVING = 5;
	static const int PHASE_DONE = 6;

	// The talk screen's replies (RZ_TalkFixer): ids free below the conversations' own
	static const int REPLY_HAND = 70;
	static const int REPLY_BLUFF = 71;
	static const int REPLY_RALLY = 72;
	static const int REPLY_STAND = 73;
	static const int REPLY_WARN = 74;

	// What came after (RZ_PlayerProfile.m_iFerdaAfter)
	static const int AFTER_RADIO = 1;
	static const int AFTER_NOTE = 2;
	static const int AFTER_TOLL = 4;
	static const int AFTER_CHANNEL = 8;

	protected static const float DUE_MIN = 120;		// he comes 2-4 minutes after you're back in the street
	protected static const float DUE_MAX = 240;
	protected static const float SHORT_DUE = 30;		// a familiar player: half a minute after the case
	protected static const float LATEST = 420;		// and meets you on the way at the latest
	protected static const float IN_STREET = 250;		// "back in the street": this close to where you started
	protected static const float SPOT_MIN = 18;		// he stands this far from you when he comes...
	protected static const float SPOT_MAX = 30;
	protected static const float CONTACT_NEAR = 60;	// he asks the contact first when he's this close to you
	protected static const float SLIP_RANGE = 120;		// out of his reach this far...
	protected static const float SLIP_SECONDS = 10;	// ...this long, with the case: you slipped away
	protected static const float FIGHT_RANGE = 40;		// a shot this close to his men is a fight
	protected static const float PATIENCE = 900;		// he waits this long for the street
	protected static const float WARN_LAPSE = 600;		// the street can be warned this long after a sale
	protected static const float GONE_RANGE = 150;		// he and his men go once you're this far...
	protected static const float GONE_MAX = 180;		// ...or this long after
	protected static const float JOB_REACH = 90;		// the street's jobs: within this of where you stand
	protected static const float SHARED_TOWN = 400;	// two openings this close share his visit (the second has his note)
	protected static const float NOTE_AFTER = 1200;	// his note: this long after the visit (or another season day)
	protected static const float NOTE_DOOR = 25;		// on the step of a real door this close to you
	protected static const float NOTE_GONE = 200;		// taken up once you're this far from it
	protected static const float TOLL_RANGE = 40;		// his men at a toll ask after you this close
	protected static const float EVENING = 19;			// the radio and his voice: from this hour...
	protected static const float LATE = 23.9;			// ...to this

	protected RZ_GameModeComponent m_GM;
	protected ref RZ_FixerStepDef m_Talk;
	protected ref RZ_FixerStepDef m_Board;
	protected ref RZ_FixerStepDef m_Rifle;
	protected ref RZ_FixerStepDef m_Lookout;
	protected ref RZ_FixerStepDef m_Stand;
	protected ref RZ_FixerStepDef m_Warn;
	protected ref array<ref RZ_FixerVisit> m_aVisits = {};
	protected ref map<int, IEntity> m_mNotes = new map<int, IEntity>();	// player -> his note out in the world
	protected ref map<int, float> m_mEnded = new map<int, float>();		// player -> when the visit ended (this run)
	protected int m_iNextId = 1;
	protected float m_fClock;
	protected float m_fSlow;

	//------------------------------------------------------------------------------------------------
	void RZ_OpeningFixer(RZ_GameModeComponent gm)
	{
		m_GM = gm;

		m_Talk = new RZ_FixerStepDef("opening.fixer.talk", "Ferda Kroupa", KIND_TALK);
		RZ_ErrandWay w = m_Talk.Way("talk");
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 1;
		w.m_fRadius = 8;
		w.m_bEndsThere = true;
		w.m_sStep = "Talk to Ferda Kroupa. He's asking who picked up the runner's case.";

		m_Board = new RZ_FixerStepDef("opening.fixer.board", "Board Up the Door", KIND_BOARD);
		w = m_Board.Way("do");
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 10;
		w.m_bEndsThere = true;
		w.m_bPerson = true;
		w.m_sStep = "Board up {giver}'s door, {place}.";
		w.Work(RZ_EWork.BOARDS, "Board up the door", 15);
		w.Take("If he comes in, he comes in through wood. Help me with the planks.");
		w.Done("That'll hold a minute. A minute is more than nothing.");

		m_Rifle = new RZ_FixerStepDef("opening.fixer.rifle", "Clean the Rifle", KIND_RIFLE);
		w = m_Rifle.Way("do");
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 12;
		w.m_fRadius = 10;
		w.m_bEndsThere = true;
		w.m_bPerson = true;
		w.m_sStep = "Clean {giver}'s rifle, {place}.";
		w.Work(RZ_EWork.BENCH, "Clean the rifle", 12);
		w.Take("It's been in the loft since my father had it. It might fire. Might.");
		w.Done("It fires now. Let's hope it doesn't have to.");

		m_Lookout = new RZ_FixerStepDef("opening.fixer.lookout", "Post a Lookout", KIND_LOOKOUT);
		w = m_Lookout.Way("do");
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 8;
		w.m_fRadius = 10;
		w.m_bEndsThere = true;
		w.m_bPerson = true;
		w.m_sStep = "Show {giver} where to watch the road from, {place}.";
		w.Work(RZ_EWork.HANDS, "Show him the road", 8);
		w.Take("Put me where I can see the road. I'll whistle.");
		w.Done("I see the road. I'll whistle if there's more of them.");

		m_Stand = new RZ_FixerStepDef("opening.fixer.stand", "Stand With the Street", KIND_STAND);
		w = m_Stand.Way("talk");
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 1;
		w.m_fRadius = 8;
		w.m_bEndsThere = true;
		w.m_sStep = "Stand with the street. Tell Ferda Kroupa.";

		m_Warn = new RZ_FixerStepDef("opening.fixer.warn", "Warn the Street", KIND_WARN);
		w = m_Warn.Way("talk");
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 1;
		w.m_fRadius = 8;
		w.m_bEndsThere = true;
		w.m_sStep = "Warn {giver}, {place}: Ferda will be back for more.";
	}

	//------------------------------------------------------------------------------------------------
	static RZ_OpeningFixer Get()
	{
		RZ_Openings openings = RZ_Openings.Get();
		if (!openings)
			return null;

		return openings.GetFixer();
	}

	//------------------------------------------------------------------------------------------------
	//! Ferda's body and his men's: the Red Hand's occupant (by its looks), else the puzzle civilian.
	protected ResourceName ManPrefab(int seed)
	{
		ResourceName prefab;
		if (m_GM.GetTerritory())
			prefab = m_GM.GetTerritory().SpeakerPrefab(RZ_EFaction.BANDITS);

		if (prefab.IsEmpty())
			prefab = m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);

		if (m_GM.GetLooks())
			prefab = m_GM.GetLooks().Pick(prefab, seed);

		return prefab;
	}

	// ================================================================ The visit
	//------------------------------------------------------------------------------------------------
	//! The case is in your pack (RZ_Openings, the case's step): he's on his way. A veteran gets only
	//! his note; a familiar player one scene where you are; a second opening in a street he's already
	//! coming to, his note too.
	void Schedule(int playerId, vector town, string townName, IEntity contact, string contactName, string contactTitle, int level)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		if (!profile || ManPrefab(0).IsEmpty())
		{
			RZ_Log.Info("Openings: no fixer for " + RZ_Util.PlayerName(playerId) + " (no body prefab)");
			return;
		}

		Forget(playerId);
		bool shared = false;
		foreach (RZ_FixerVisit other : m_aVisits)
		{
			if (other.m_iPhase < PHASE_LEAVING && RZ_Util.DistXZ(other.m_vTown, town) < SHARED_TOWN)
				shared = true;
		}

		if (level >= 3 || shared)
		{
			Remember(playerId, RZ_EFerdaWay.NOTE, townName);
			m_mEnded.Set(playerId, m_fClock - NOTE_AFTER + 60);
			string why = "a veteran";
			if (shared)
				why = "his visit to that street is already under way";

			RZ_Log.Info(string.Format("Openings: Ferda's note for %1 (%2)", RZ_Util.PlayerName(playerId), why));
			return;
		}

		RZ_FixerVisit v = new RZ_FixerVisit();
		v.m_iId = m_iNextId;
		m_iNextId++;
		v.m_iPlayer = playerId;
		v.m_iLevel = level;
		v.m_bShort = level >= 2;
		v.m_vTown = town;
		v.m_sTown = townName;
		v.m_Contact = contact;
		v.m_sContact = contactName;
		v.m_sContactTitle = contactTitle;
		v.m_fDue = Math.RandomFloat(DUE_MIN, DUE_MAX);
		if (v.m_bShort)
			v.m_fDue = SHORT_DUE;

		v.m_fPhaseAt = m_fClock;
		v.m_fLatest = m_fClock + LATEST;
		RZ_Rng rng = new RZ_Rng(playerId * 7919 + v.m_iId);
		for (int i = 0; i < 3; i++)
		{
			v.m_aNames.Insert(RZ_NoteText.ManName(rng) + " " + RZ_NoteText.Surname(rng));
		}

		v.m_sNeighbour = RZ_NoteText.ManName(rng) + " " + RZ_NoteText.Surname(rng);
		m_aVisits.Insert(v);
		RZ_Log.Info(string.Format("Openings: Ferda Kroupa is coming for %1 (%2, %3 s after they're back in %4)", RZ_Util.PlayerName(playerId), LevelText(v), RZ_Util.RoundToInt(v.m_fDue), townName));
	}

	//------------------------------------------------------------------------------------------------
	protected static string LevelText(RZ_FixerVisit v)
	{
		if (v.m_bShort)
			return "one scene";

		return "the full visit";
	}

	//------------------------------------------------------------------------------------------------
	//! His visit for this player is still to come or under way (the Red Hand's people don't buy the
	//! case meanwhile: he's the buyer).
	bool Waiting(int playerId)
	{
		RZ_FixerVisit v = Of(playerId);
		return v && v.m_iPhase < PHASE_LEAVING;
	}

	//------------------------------------------------------------------------------------------------
	RZ_FixerVisit Of(int playerId)
	{
		foreach (RZ_FixerVisit v : m_aVisits)
		{
			if (v.m_iPlayer == playerId && v.m_iPhase < PHASE_DONE)
				return v;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_FixerVisit ById(int id)
	{
		foreach (RZ_FixerVisit v : m_aVisits)
		{
			if (v.m_iId == id)
				return v;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! A visit under way ends without a word (a new character, the test).
	void Forget(int playerId)
	{
		RZ_FixerVisit v = Of(playerId);
		if (!v)
			return;

		DropStep(v);
		Remove(v);
		v.m_iPhase = PHASE_DONE;
	}

	//------------------------------------------------------------------------------------------------
	//! He comes: Ferda and two of the crews stand 18-30 m from you, he asks the contact first (when
	//! he's near), then "Talk to Ferda Kroupa" is tracked, its marker on him.
	protected bool Come(RZ_FixerVisit v, IEntity me)
	{
		if (!RZ_OpeningCase.Carries(me))
		{
			// It's gone already (handed in, sold, lost): he'll ask the book's way, later
			Remember(v.m_iPlayer, RZ_EFerdaWay.GONE, v.m_sTown);
			v.m_iPhase = PHASE_DONE;
			RZ_Log.Info("Openings: Ferda came too late for " + RZ_Util.PlayerName(v.m_iPlayer) + ": the case is gone");
			return false;
		}

		RZ_Shared shared = m_GM.GetShared();
		vector at = RZ_Mission.LandPointNear(me.GetOrigin(), SPOT_MIN, SPOT_MAX);
		v.m_Ferda = SpawnMan(ManPrefab(v.m_iId * 3), at, me.GetOrigin(), shared);
		if (!v.m_Ferda)
		{
			v.m_iPhase = PHASE_DONE;
			RZ_Log.Warn("Openings: Ferda couldn't be put out for " + RZ_Util.PlayerName(v.m_iPlayer));
			return false;
		}

		for (int i = 1; i <= 2; i++)
		{
			IEntity man = SpawnMan(ManPrefab(v.m_iId * 3 + i), RZ_Util.RandomPointInRing(at, 1.5, 3), me.GetOrigin(), shared);
			if (man)
				v.m_aCrew.Insert(man);
		}

		RZ_PuzzleObjectComponent.Tag(v.m_Ferda, TAG, v.m_iId * 10 + ROLE_FERDA, "", "What do you want?");
		RZ_ClueComponent.Tag(v.m_Ferda, TAG, v.m_iId * 10 + ROLE_FERDA, "Talk to Ferda Kroupa");

		if (v.m_Contact && RZ_Util.IsCharacterAlive(v.m_Contact) && RZ_Util.DistXZ(v.m_Contact.GetOrigin(), me.GetOrigin()) <= CONTACT_NEAR)
		{
			m_GM.Say(v.m_iPlayer, FerdaTitle(), RZ_EFaction.BANDITS, string.Format("%1, isn't it. The runner on the line road owed the book, and he had something on him. Somebody from this street picked it up.", FirstOf(v.m_sContact)), v.m_Ferda);
			m_GM.Say(v.m_iPlayer, v.m_sContactTitle, -1, "Ask him. I've enough to answer for.", v.m_Contact);
		}
		else
		{
			m_GM.Say(v.m_iPlayer, FerdaTitle(), RZ_EFaction.BANDITS, "A word, friend. The runner on the line road owed the book, and somebody from this street picked up what he carried.", v.m_Ferda);
		}

		v.m_iPhase = PHASE_TALK;
		v.m_fPhaseAt = m_fClock;
		HandStep(v, m_Talk, FerdaTitle(), v.m_Ferda.GetOrigin(), "Ferda Kroupa", RZ_EFaction.BANDITS);
		RZ_Log.Info(string.Format("Openings: Ferda Kroupa came for %1 at %2 with %3 men (%4)", RZ_Util.PlayerName(v.m_iPlayer), RZ_SettlementScanner.GridReference(at), v.m_aCrew.Count(), LevelText(v)));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity SpawnMan(ResourceName prefab, vector at, vector face, RZ_Shared shared)
	{
		IEntity man = RZ_Util.SpawnPrefab(prefab, RZ_Util.GroundPos(at), RZ_Compass.Bearing(at, face));
		if (man && shared)
			shared.AddBody(RZ_BodyBudget.STORY, man);

		return man;
	}

	//------------------------------------------------------------------------------------------------
	static string FerdaTitle()
	{
		return "Ferda Kroupa, the Red Hand";
	}

	//------------------------------------------------------------------------------------------------
	protected static string FirstOf(string name)
	{
		int space = name.IndexOf(" ");
		if (space <= 0)
			return name;

		return name.Substring(0, space);
	}

	//------------------------------------------------------------------------------------------------
	//! A step of his, handed and tracked.
	protected void HandStep(RZ_FixerVisit v, RZ_FixerStepDef d, string giverName, vector place, string placeText, int faction)
	{
		DropStep(v);
		RZ_Errands errands = m_GM.GetErrands();
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(v.m_iPlayer);
		if (!errands || !errands.IsOn() || !me)
			return;

		RZ_Errand e = errands.Hand(d, v.m_iPlayer, "fixer:" + v.m_iId.ToString(), giverName, giverName, faction, place, place, placeText, me);
		if (!e)
			return;

		v.m_iErrand = e.m_iId;
		RZ_UIFeed feed = m_GM.GetUIFeed();
		if (feed && feed.GetTracker())
			feed.GetTracker().Follow(v.m_iPlayer, -(RZ_UIFeed.ERRAND_ID_BASE + e.m_iId), e.Fill(d.m_sTitle));

		RZ_Log.Info(string.Format("Openings: Ferda's step %1 for %2 at %3", d.m_sKey, RZ_Util.PlayerName(v.m_iPlayer), RZ_SettlementScanner.GridReference(place)));
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Errand StepOf(RZ_FixerVisit v)
	{
		RZ_Errands errands = m_GM.GetErrands();
		if (!errands || v.m_iErrand < 0)
			return null;

		return errands.FindById(v.m_iErrand);
	}

	//------------------------------------------------------------------------------------------------
	protected void DropStep(RZ_FixerVisit v)
	{
		RZ_Errand e = StepOf(v);
		v.m_iErrand = -1;
		if (e && e.m_eState == RZ_EErrandState.HELD)
			m_GM.GetErrands().Drop(e, "done");
	}

	//------------------------------------------------------------------------------------------------
	//! The talk steps are done only by their replies (docs/MISSION_RULES.md, rule 19); the street's
	//! jobs by their work sites.
	bool StepCounts(RZ_FixerStepDef d)
	{
		return d.m_iKind == KIND_BOARD || d.m_iKind == KIND_RIFLE || d.m_iKind == KIND_LOOKOUT;
	}

	//------------------------------------------------------------------------------------------------
	//! A street job done: the next, or, with all three, stand with the street (talk to him again).
	void OnStepDone(RZ_FixerStepDef d, int playerId)
	{
		RZ_FixerVisit v = Of(playerId);
		if (!v || v.m_iPhase != PHASE_RALLY || !StepCounts(d))
			return;

		v.m_iErrand = -1;
		v.m_iRally++;
		RZ_Log.Info(string.Format("Openings: the street's job %1 of 3 done for %2 (%3)", v.m_iRally, RZ_Util.PlayerName(playerId), d.m_sKey));
		NextJob(v);
	}

	// ================================================================ The street
	//------------------------------------------------------------------------------------------------
	//! The street's next job, or stand with it once all three are done.
	protected void NextJob(RZ_FixerVisit v)
	{
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(v.m_iPlayer);
		if (!me)
			return;

		if (v.m_iRally >= 3)
		{
			v.m_iPhase = PHASE_STAND;
			v.m_fPhaseAt = m_fClock;
			if (v.m_Ferda)
				HandStep(v, m_Stand, FerdaTitle(), v.m_Ferda.GetOrigin(), "Ferda Kroupa", RZ_EFaction.BANDITS);

			return;
		}

		vector place;
		string text;
		RZ_FixerStepDef d;
		string name = v.m_aNames[v.m_iRally];
		if (v.m_iRally == 0)
		{
			d = m_Board;
			if (!DoorAt(me.GetOrigin(), 6, place))
				place = vector.Zero;

			text = "at the front door";
		}
		else if (v.m_iRally == 1)
		{
			d = m_Rifle;
			if (!DoorAt(me.GetOrigin(), 20, place))
				place = vector.Zero;

			text = "on the step of the house";
		}
		else
		{
			d = m_Lookout;
			place = RZ_Mission.LandPointNear(me.GetOrigin(), 25, 50);
			text = "at the corner";
		}

		if (place == vector.Zero)
		{
			// No real door near: that job can't be done here; the street makes do without it
			RZ_Log.Info(string.Format("Openings: no real door near %1 for the street's job %2; it makes do without", RZ_SettlementScanner.GridReference(me.GetOrigin()), d.m_sKey));
			v.m_iRally++;
			NextJob(v);
			return;
		}

		HandStep(v, d, name, place, text, RZ_EFaction.SURVIVORS);
	}

	//------------------------------------------------------------------------------------------------
	//! A real outside door's doorstep within the street's reach (beyond `lo` m), never a spawned door.
	protected bool DoorAt(vector from, float lo, out vector step)
	{
		IEntity door;
		vector centre;
		vector normal;
		if (!RZ_DoorGeometry.NearestOuterDoor(from, lo, JOB_REACH, false, door, centre, normal))
			return false;

		step = RZ_Util.GroundPos(centre + normal * 0.6);
		return true;
	}

	// ================================================================ The talk screen
	//------------------------------------------------------------------------------------------------
	//! Who they are on the talk screen (RZ_TalkFixer).
	void Setup(int code, RZ_TalkSession s)
	{
		RZ_FixerVisit v = ById(code / 10);
		if (code % 10 == ROLE_NEIGHBOUR)
		{
			string name = "A neighbour";
			if (v)
				name = v.m_sNeighbour;

			s.m_sHeader = name;
			s.m_sName = name;
			s.m_sSub = "A neighbour";
			s.m_iFaction = -1;
			return;
		}

		s.m_sHeader = FerdaTitle();
		s.m_sName = "Ferda Kroupa";
		s.m_sSub = "The Red Hand - he collects for the book";
		s.m_iFaction = RZ_EFaction.BANDITS;
	}

	//------------------------------------------------------------------------------------------------
	//! He speaks first the first time you talk to him.
	bool SpeaksFirst(int code, int playerId)
	{
		RZ_FixerVisit v = ById(code / 10);
		return v && v.m_iPlayer == playerId && code % 10 == ROLE_FERDA && !v.m_bGreeted && v.m_iPhase == PHASE_TALK;
	}

	//------------------------------------------------------------------------------------------------
	void Greet(int code, int playerId, notnull array<string> lines)
	{
		RZ_FixerVisit v = ById(code / 10);
		if (!v || v.m_iPlayer != playerId)
		{
			if (code % 10 == ROLE_FERDA)
				lines.Insert("Not you. Someone else owes the book today.");
			else
				lines.Insert("I keep my door shut and my head down. You should too.");

			return;
		}

		if (code % 10 == ROLE_NEIGHBOUR)
		{
			if (v.m_iPhase == PHASE_WARN)
				lines.Insert("I saw who you were talking to. What did he want?");

			return;
		}

		if (v.m_iPhase == PHASE_TALK && !v.m_bGreeted)
		{
			v.m_bGreeted = true;
			lines.Insert("Ferda Kroupa. I collect for the book. Nothing personal: the book doesn't do personal.");
			lines.Insert("The runner owed. What he carried pays it, and it's in your pack. A cold case, Halcyon's tag on it. Am I warm?");
			return;
		}

		if (v.m_iPhase == PHASE_RALLY)
		{
			lines.Insert("Take your time. The book is patient.");
			return;
		}

		if (v.m_iPhase == PHASE_STAND)
		{
			lines.Insert("Well?");
			return;
		}

		if (v.m_iPhase >= PHASE_WARN)
			lines.Insert("We're done, you and I. For now.");
	}

	//------------------------------------------------------------------------------------------------
	//! The replies, first in the list: at Ferda, hand it over, the story about Bastion, ask the street
	//! (the full visit), and stand with it (once its three jobs are done); at the neighbour, after a
	//! sale, the warning.
	void Replies(int code, int playerId, IEntity user, notnull array<ref RZ_TalkReply> replies)
	{
		RZ_FixerVisit v = ById(code / 10);
		if (!v || v.m_iPlayer != playerId)
			return;

		if (code % 10 == ROLE_NEIGHBOUR)
		{
			if (v.m_iPhase == PHASE_WARN)
				replies.InsertAt(Job(REPLY_WARN, "He'll be back for more. Bolt your doors.", "Errand", ""), 0);

			return;
		}

		if (v.m_iPhase == PHASE_STAND)
		{
			replies.InsertAt(Job(REPLY_STAND, "The street's with me.", "Street", ""), 0);
			return;
		}

		if (v.m_iPhase != PHASE_TALK)
			return;

		if (!v.m_bShort)
			replies.InsertAt(Job(REPLY_RALLY, "Wait here. I'll talk to the street first.", "Street", ""), 0);

		string tag;
		string need = BluffNeed(playerId, tag);
		replies.InsertAt(Job(REPLY_BLUFF, "Bastion's patrol is two streets away. You'd better go.", tag, need), 0);

		string missing = "";
		if (!RZ_OpeningCase.Carries(user))
			missing = "You don't have it on you";

		replies.InsertAt(Job(REPLY_HAND, "Hand it over", "Trade", missing), 0);
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_TalkReply Job(int id, string label, string tag, string need)
	{
		RZ_TalkReply r = new RZ_TalkReply();
		r.m_iId = id;
		r.m_sLabel = label;
		r.m_bOwn = true;
		r.m_bJob = true;
		r.m_iRank = RZ_TalkReply.RANK_ON;
		if (!tag.IsEmpty())
			r.m_sTag = RZ_TalkJobs.Tag(tag);

		if (!need.IsEmpty())
		{
			r.m_bEnabled = false;
			r.m_sNeed = need;
			r.m_iRank = RZ_TalkReply.RANK_LOCKED;
		}

		return r;
	}

	//------------------------------------------------------------------------------------------------
	//! The story about Bastion works for the good liar, and for a background he'd believe it from: a
	//! Police Officer, a Soldier, an Ex-Con, a Raider. `tag` is which; "" need when it works.
	protected string BluffNeed(int playerId, out string tag)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		if (profile && RZ_Habits.Of(profile) == RZ_EPlayerHabit.LIAR)
		{
			tag = RZ_Habits.Name(RZ_EPlayerHabit.LIAR);
			return "";
		}

		string bg = "";
		if (m_GM.GetPlayerBackground(playerId))
			bg = m_GM.GetPlayerBackground(playerId).m_sId;

		if (bg == "police" || bg == "soldier" || bg == "excon" || bg == "raider")
		{
			tag = RZ_DialogueTrees.BackgroundName(bg);
			return "";
		}

		tag = RZ_Habits.Name(RZ_EPlayerHabit.LIAR);
		return "He'd believe it from a good liar, or from police, a soldier, an ex-con or a raider";
	}

	//------------------------------------------------------------------------------------------------
	//! A reply picked: what they say goes into the open talk.
	void Pick(int code, int playerId, int replyId, IEntity user, notnull array<string> said)
	{
		RZ_FixerVisit v = ById(code / 10);
		if (!v || v.m_iPlayer != playerId)
			return;

		if (replyId == REPLY_HAND && v.m_iPhase == PHASE_TALK)
		{
			Hand(v, user, said);
			return;
		}

		if (replyId == REPLY_BLUFF && v.m_iPhase == PHASE_TALK)
		{
			string tag;
			if (!BluffNeed(playerId, tag).IsEmpty())
			{
				said.Insert("No, it isn't.");
				return;
			}

			said.Insert("Two streets. Then I'll come back when it's none.");
			said.Insert("I'll remember your face. The book remembers everything else.");
			End(v, RZ_EFerdaWay.TALKED);
			return;
		}

		if (replyId == REPLY_RALLY && v.m_iPhase == PHASE_TALK && !v.m_bShort)
		{
			said.Insert("Talk to whoever you like. The book is patient. I'm nearly as patient as the book.");
			v.m_iPhase = PHASE_RALLY;
			v.m_fPhaseAt = m_fClock;
			v.m_iRally = 0;
			DropStep(v);
			NextJob(v);
			return;
		}

		if (replyId == REPLY_STAND && v.m_iPhase == PHASE_STAND)
		{
			said.Insert("Four of you. Five. Boards on the doors and a lad on the corner.");
			said.Insert("Fine. The book has a long memory, and this street is in it now.");
			End(v, RZ_EFerdaWay.RALLIED);
			return;
		}

		if (replyId == REPLY_WARN && v.m_iPhase == PHASE_WARN)
		{
			said.Insert("He'll be back. Of course he will. Thank you for saying it, at least.");
			End(v, RZ_EFerdaWay.WARNED);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Handed over: the case's sale (RZ_OpeningCase.Sell: the book pays, Get It Back opens), the street
	//! thinks less of you; on the full visit a neighbour can still be warned.
	protected void Hand(RZ_FixerVisit v, IEntity user, notnull array<string> said)
	{
		RZ_OpeningCase c = RZ_OpeningCase.Get();
		if (!c || !c.Sell(v.m_iPlayer, user, said, "Half what the lab would give. More than the runner ever got."))
			return;

		said.Insert("A pleasure. The book will write you down as square.");
		if (v.m_bShort || !PutNeighbour(v, user))
		{
			End(v, RZ_EFerdaWay.HANDED);
			return;
		}

		v.m_iPhase = PHASE_WARN;
		v.m_fPhaseAt = m_fClock;
		HandStep(v, m_Warn, v.m_sNeighbour, v.m_Neighbour.GetOrigin(), "at his door", RZ_EFaction.SURVIVORS);
		RZ_Log.Info(string.Format("Openings: %1 sold the case to Ferda; the street can still be warned (%2)", RZ_Util.PlayerName(v.m_iPlayer), v.m_sNeighbour));
	}

	//------------------------------------------------------------------------------------------------
	//! The neighbour who saw it, at his own real door. False: no door near.
	protected bool PutNeighbour(RZ_FixerVisit v, IEntity me)
	{
		vector step;
		if (!DoorAt(me.GetOrigin(), 6, step))
			return false;

		ResourceName prefab = m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);
		if (prefab.IsEmpty())
			return false;

		v.m_Neighbour = SpawnMan(prefab, step, me.GetOrigin(), m_GM.GetShared());
		if (!v.m_Neighbour)
			return false;

		RZ_PuzzleObjectComponent.Tag(v.m_Neighbour, TAG, v.m_iId * 10 + ROLE_NEIGHBOUR, "", "What did he want?");
		RZ_ClueComponent.Tag(v.m_Neighbour, TAG, v.m_iId * 10 + ROLE_NEIGHBOUR, "Talk to " + v.m_sNeighbour);
		return true;
	}

	// ================================================================ How it ends
	//------------------------------------------------------------------------------------------------
	//! What you did: the standing it costs or earns (small, through the caps), kept on the profile, the
	//! radio that evening; he and his men go.
	protected void End(RZ_FixerVisit v, int way)
	{
		v.m_iWay = way;
		int pid = v.m_iPlayer;
		switch (way)
		{
			case RZ_EFerdaWay.HANDED:
				m_GM.AddInfamy(pid, RZ_EFaction.SURVIVORS, 3, "sold the runner's case to the book in the street", RZ_RepFlag.SMALL);
				break;
			case RZ_EFerdaWay.RALLIED:
				m_GM.AddFame(pid, RZ_EFaction.SURVIVORS, 5, false, RZ_RepFlag.SMALL);
				m_GM.AddInfamy(pid, RZ_EFaction.BANDITS, 3, "stood the street against the book's man", RZ_RepFlag.SMALL);
				break;
			case RZ_EFerdaWay.SLIPPED:
				m_GM.AddInfamy(pid, RZ_EFaction.SURVIVORS, 3, "left the street to Ferda's visit", RZ_RepFlag.SMALL);
				break;
			case RZ_EFerdaWay.WARNED:
				m_GM.AddFame(pid, RZ_EFaction.SURVIVORS, 3, false, RZ_RepFlag.SMALL);
				m_GM.AddInfamy(pid, RZ_EFaction.BANDITS, 3, "sold to the book, then warned the street", RZ_RepFlag.SMALL);
				break;
			case RZ_EFerdaWay.FOUGHT:
				m_GM.AddInfamy(pid, RZ_EFaction.BANDITS, 10, "fired on Ferda's men");
				break;
		}

		Remember(pid, way, v.m_sTown);
		m_mEnded.Set(pid, m_fClock);
		DropStep(v);
		// He and his men go: they stand till you're away, then they're put away (TickVisit)
		v.m_iPhase = PHASE_LEAVING;
		v.m_fPhaseAt = m_fClock;
		RZ_Log.Info(string.Format("Openings: Ferda's visit for %1 ended: %2", RZ_Util.PlayerName(pid), WayName(way)));
	}

	//------------------------------------------------------------------------------------------------
	protected void Remove(RZ_FixerVisit v)
	{
		Delete(v.m_Ferda);
		foreach (IEntity man : v.m_aCrew)
		{
			Delete(man);
		}

		v.m_aCrew.Clear();
		Delete(v.m_Neighbour);
		v.m_Ferda = null;
		v.m_Neighbour = null;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Delete(IEntity e)
	{
		if (e)
			SCR_EntityHelper.DeleteEntityAndChildren(e);
	}

	//------------------------------------------------------------------------------------------------
	//! What you did with him, kept for this character.
	protected void Remember(int playerId, int way, string town)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		profile.m_iFerdaWay = way;
		profile.m_iFerdaLife = RZ_OpeningCase.CharacterStamp(profile);
		profile.m_iFerdaDay = RZ_SeasonClock.Day();
		profile.m_sFerdaTown = town;
		profile.m_iFerdaAfter = 0;
		if (way == RZ_EFerdaWay.NOTE || way == RZ_EFerdaWay.GONE)
			profile.m_iFerdaAfter = AFTER_RADIO;
	}

	//------------------------------------------------------------------------------------------------
	//! This character's way with him (NONE when it was another character's).
	int WayOf(int playerId)
	{
		RZ_PlayerProfile profile = m_GM.FindProfile(playerId);
		if (!profile || profile.m_iFerdaLife != RZ_OpeningCase.CharacterStamp(profile))
			return RZ_EFerdaWay.NONE;

		return profile.m_iFerdaWay;
	}

	//------------------------------------------------------------------------------------------------
	static string WayName(int way)
	{
		string name = typename.EnumToString(RZ_EFerdaWay, way);
		name.ToLower();
		return name;
	}

	//------------------------------------------------------------------------------------------------
	//! He remembers you (his note, his men, his voice): every way but the hand-over and the note.
	protected static bool Remembers(int way)
	{
		return way == RZ_EFerdaWay.TALKED || way == RZ_EFerdaWay.RALLIED || way == RZ_EFerdaWay.SLIPPED || way == RZ_EFerdaWay.WARNED || way == RZ_EFerdaWay.FOUGHT || way == RZ_EFerdaWay.WAITED;
	}

	// ================================================================ The tick
	//------------------------------------------------------------------------------------------------
	//! From the openings' 1 s poll.
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		m_fClock += dt;
		for (int i = m_aVisits.Count() - 1; i >= 0; i--)
		{
			RZ_FixerVisit v = m_aVisits[i];
			TickVisit(v, dt, ids, players);
			if (v.m_iPhase == PHASE_DONE)
			{
				Remove(v);
				m_aVisits.Remove(i);
			}
		}

		m_fSlow += dt;
		if (m_fSlow < 5)
			return;

		m_fSlow = 0;
		foreach (int k, IEntity me : players)
		{
			After(ids[k], me);
		}

		Notes(ids, players);
	}

	//------------------------------------------------------------------------------------------------
	protected void TickVisit(RZ_FixerVisit v, float dt, array<int> ids, array<IEntity> players)
	{
		int idx = ids.Find(v.m_iPlayer);
		if (idx < 0)
		{
			// Gone (a death, a new character, left): the visit goes quietly
			if (v.m_iPhase < PHASE_LEAVING)
				DropStep(v);

			v.m_iPhase = PHASE_DONE;
			return;
		}

		IEntity me = players[idx];
		if (v.m_iPhase == PHASE_WAIT)
		{
			// The clock runs while you're back in the street; on the way, he meets you at the latest
			if (RZ_Util.DistXZ(me.GetOrigin(), v.m_vTown) > IN_STREET && !v.m_bShort)
				v.m_fPhaseAt += dt;

			if (m_fClock - v.m_fPhaseAt >= v.m_fDue || m_fClock >= v.m_fLatest)
				Come(v, me);

			return;
		}

		if (v.m_iPhase == PHASE_LEAVING)
		{
			bool away = !v.m_Ferda || RZ_Util.DistXZ(me.GetOrigin(), v.m_Ferda.GetOrigin()) > GONE_RANGE;
			if (away || m_fClock - v.m_fPhaseAt > GONE_MAX)
				v.m_iPhase = PHASE_DONE;

			return;
		}

		// A fight: one of his men down, or he is
		if (v.m_iPhase <= PHASE_STAND && Fallen(v))
		{
			Fight(v);
			return;
		}

		// The marker on him while you're to talk to him
		RZ_Errand e = StepOf(v);
		if (e && v.m_Ferda && (v.m_iPhase == PHASE_TALK || v.m_iPhase == PHASE_STAND))
			e.m_vPlace = v.m_Ferda.GetOrigin();

		if (v.m_iPhase == PHASE_WARN)
		{
			if (e && v.m_Neighbour)
				e.m_vPlace = v.m_Neighbour.GetOrigin();

			if (m_fClock - v.m_fPhaseAt > WARN_LAPSE || !v.m_Neighbour)
				End(v, RZ_EFerdaWay.HANDED);

			return;
		}

		// Slipping away: out of his reach long enough, the case still on you
		if (v.m_Ferda && RZ_Util.DistXZ(me.GetOrigin(), v.m_Ferda.GetOrigin()) > SLIP_RANGE && RZ_OpeningCase.Carries(me))
		{
			v.m_fAway += dt;
			if (v.m_fAway >= SLIP_SECONDS)
			{
				End(v, RZ_EFerdaWay.SLIPPED);
				return;
			}
		}
		else
		{
			v.m_fAway = 0;
		}

		if ((v.m_iPhase == PHASE_RALLY || v.m_iPhase == PHASE_STAND) && m_fClock - v.m_fPhaseAt > PATIENCE)
		{
			if (v.m_Ferda)
				m_GM.Say(v.m_iPlayer, FerdaTitle(), RZ_EFaction.BANDITS, "I'll come back when the street's done hammering.", v.m_Ferda);

			End(v, RZ_EFerdaWay.WAITED);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Fallen(RZ_FixerVisit v)
	{
		if (v.m_Ferda && !RZ_Util.IsCharacterAlive(v.m_Ferda))
			return true;

		foreach (IEntity man : v.m_aCrew)
		{
			if (man && !RZ_Util.IsCharacterAlive(man))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void Fight(RZ_FixerVisit v)
	{
		if (v.m_Ferda && RZ_Util.IsCharacterAlive(v.m_Ferda))
			m_GM.Say(v.m_iPlayer, FerdaTitle(), RZ_EFaction.BANDITS, "Wrong answer.", v.m_Ferda);

		End(v, RZ_EFerdaWay.FOUGHT);
	}

	//------------------------------------------------------------------------------------------------
	//! A shot (RZ_GameModeComponent): one this close to his men while he's here is a fight. VERIFY: his
	//! men answer it by their own faction's AI; nothing here makes them shoot.
	void OnPlayerShot(int playerId, vector pos)
	{
		RZ_FixerVisit v = Of(playerId);
		if (!v || !v.m_Ferda || v.m_iPhase < PHASE_TALK || v.m_iPhase > PHASE_STAND)
			return;

		if (RZ_Util.DistXZ(pos, v.m_Ferda.GetOrigin()) <= FIGHT_RANGE)
			Fight(v);
	}

	// ================================================================ Afterwards
	//------------------------------------------------------------------------------------------------
	//! The radio that evening, his men at a toll, his voice on the crews' channel another evening.
	protected void After(int playerId, IEntity me)
	{
		RZ_PlayerProfile profile = m_GM.FindProfile(playerId);
		if (!profile || !me)
			return;

		int way = WayOf(playerId);
		if (way == RZ_EFerdaWay.NONE || Of(playerId))
			return;

		float hour = RZ_GameTime.GetHour();
		bool evening = hour >= EVENING && hour <= LATE;
		RZ_FactionLife life = m_GM.GetFactionLife();
		if ((profile.m_iFerdaAfter & AFTER_RADIO) == 0 && evening && life)
		{
			profile.m_iFerdaAfter = profile.m_iFerdaAfter | AFTER_RADIO;
			int channel = RZ_EFaction.SURVIVORS;
			if (way == RZ_EFerdaWay.HANDED || way == RZ_EFerdaWay.FOUGHT)
				channel = RZ_EFaction.BANDITS;

			life.OpenChannelLine(channel, RadioLine(way, profile.m_sFerdaTown));
			RZ_Log.Info(string.Format("Openings: the radio spoke of %1's way with Ferda (%2)", RZ_Util.PlayerName(playerId), WayName(way)));
		}

		if ((profile.m_iFerdaAfter & AFTER_TOLL) == 0 && way != RZ_EFerdaWay.GONE && m_GM.GetCheckpoints())
		{
			array<RZ_RoadPost> posts = {};
			m_GM.GetCheckpoints().PostsNear(RZ_EFaction.BANDITS, me.GetOrigin(), TOLL_RANGE, posts);
			foreach (RZ_RoadPost post : posts)
			{
				if (!post.m_bSpawned)
					continue;

				profile.m_iFerdaAfter = profile.m_iFerdaAfter | AFTER_TOLL;
				m_GM.SayAt(playerId, post.SpeakerName(), RZ_EFaction.BANDITS, TollLine(way), post.m_vPos);
				RZ_Log.Info("Openings: the toll asked after " + RZ_Util.PlayerName(playerId) + " for Ferda");
				break;
			}
		}

		if ((profile.m_iFerdaAfter & AFTER_CHANNEL) == 0 && Remembers(way) && evening && life && RZ_SeasonClock.Day() > profile.m_iFerdaDay)
		{
			profile.m_iFerdaAfter = profile.m_iFerdaAfter | AFTER_CHANNEL;
			life.OpenChannelLine(RZ_EFaction.BANDITS, "Ferda. To whoever picked up the runner's case in " + profile.m_sFerdaTown + ", or had it, or gave it away: the book is patient. It isn't forgetful.");
			RZ_Log.Info("Openings: Ferda's voice on the crews' channel, for " + RZ_Util.PlayerName(playerId));
		}
	}

	//------------------------------------------------------------------------------------------------
	static string RadioLine(int way, string town)
	{
		switch (way)
		{
			case RZ_EFerdaWay.HANDED: return "The book's square on the runner from the line road. Somebody in " + town + " was sensible about it.";
			case RZ_EFerdaWay.TALKED: return "The book's man was in " + town + " today, asking after a cold case. Somebody sent him off with a story about Bastion. He'll be back to check it.";
			case RZ_EFerdaWay.RALLIED: return "The street in " + town + " stood in its doorways today, with boards up and a lad on the corner, and the book's man went home. That's how it's done.";
			case RZ_EFerdaWay.SLIPPED: return "The book's man came to " + town + " asking after a cold case. Whoever had it wasn't there. The street was.";
			case RZ_EFerdaWay.WARNED: return "Somebody in " + town + " sold the book what it came for, then told the street to bolt its doors. Make of that what you like.";
			case RZ_EFerdaWay.FOUGHT: return "Shots in " + town + " this afternoon, at the book's own. The book has written it down.";
			case RZ_EFerdaWay.WAITED: return "The book's man waited in " + town + " most of the afternoon while somebody hammered boards. Then he left. He said he'd be back.";
			case RZ_EFerdaWay.NOTE: return "A note's going round " + town + ", signed F. K. It's about a cold case. If it's yours, you know.";
		}

		return "A cold case went missing on the line road. The book is asking.";
	}

	//------------------------------------------------------------------------------------------------
	static string TollLine(int way)
	{
		if (way == RZ_EFerdaWay.HANDED)
			return "Ferda says you're square with the book. That's rare. Go on through.";

		if (way == RZ_EFerdaWay.NOTE)
			return "Ferda left word. You've got something of the book's, he says. He's patient.";

		return "Ferda's asking after you. He said you'd know why.";
	}

	//------------------------------------------------------------------------------------------------
	//! His note: on the step of a real door near you, once, a while after (a veteran's, a minute after
	//! the case); taken up again once you're away from it.
	protected void Notes(array<int> ids, array<IEntity> players)
	{
		foreach (int k, IEntity me : players)
		{
			int playerId = ids[k];
			IEntity note;
			if (m_mNotes.Find(playerId, note))
			{
				if (!note || RZ_Util.DistXZ(note.GetOrigin(), me.GetOrigin()) > NOTE_GONE)
				{
					Delete(note);
					m_mNotes.Remove(playerId);
				}

				continue;
			}

			RZ_PlayerProfile profile = m_GM.FindProfile(playerId);
			int way = WayOf(playerId);
			if (!profile || (profile.m_iFerdaAfter & AFTER_NOTE) != 0 || Of(playerId))
				continue;

			if (way != RZ_EFerdaWay.NOTE && !Remembers(way))
				continue;

			float ended;
			bool due = RZ_SeasonClock.Day() > profile.m_iFerdaDay;
			if (m_mEnded.Find(playerId, ended))
				due = due || m_fClock - ended >= NOTE_AFTER;

			if (due)
				PutNote(playerId, me);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! His note on a real door's step within a few metres (never a door of our own). False: none near.
	bool PutNote(int playerId, IEntity me)
	{
		IEntity door;
		vector centre;
		vector normal;
		if (!RZ_DoorGeometry.NearestOuterDoor(me.GetOrigin(), 0, NOTE_DOOR, false, door, centre, normal))
			return false;

		ResourceName prefab = m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CLUE);
		if (prefab.IsEmpty())
			return false;

		vector at = RZ_Util.GroundPos(centre + normal * 0.5);
		IEntity note = RZ_Util.SpawnPrefab(prefab, at + Vector(0, 0.05, 0), Math.RandomFloat(0, 360));
		if (!note)
			return false;

		RZ_ClueComponent.Tag(note, TAG, NOTE_BASE + playerId, "Read the note on the step");
		RZ_PuzzleObjectComponent.Tag(note, TAG, NOTE_BASE + playerId, "|", "|");
		m_mNotes.Set(playerId, note);
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		profile.m_iFerdaAfter = profile.m_iFerdaAfter | AFTER_NOTE;
		RZ_Log.Info(string.Format("Openings: Ferda's note on the step of a door at %1 for %2", RZ_SettlementScanner.GridReference(at), RZ_Util.PlayerName(playerId)));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The note's words, by what you did.
	string NoteText(int ownerId)
	{
		RZ_PlayerProfile profile = m_GM.FindProfile(ownerId);
		int way = RZ_EFerdaWay.NONE;
		string town = "the street";
		if (profile)
		{
			way = profile.m_iFerdaWay;
			town = profile.m_sFerdaTown;
		}

		string place = "the nearest toll";
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.GetStronghold(RZ_EFaction.BANDITS))
			place = territory.GetStronghold(RZ_EFaction.BANDITS).m_sName;

		switch (way)
		{
			case RZ_EFerdaWay.NOTE: return "You picked up something on the line road that the runner owed us. Bring it to the book at " + place + " and we'll call it square. Nobody needs to be unpleasant.\n\nF. K.";
			case RZ_EFerdaWay.TALKED: return "Bastion's patrol never came. I waited. I'm not angry: it was a good story. But the book has your face now, next to the case.\n\nF. K.";
			case RZ_EFerdaWay.RALLIED: return "Boards on the doors in " + town + ". A lad on the corner with his grandfather's rifle. I'd have done the same.\n\nThe book doesn't forget a street.\n\nF. K.";
			case RZ_EFerdaWay.SLIPPED: return "You left " + town + " by the back. They didn't. Think about that.\n\nF. K.";
			case RZ_EFerdaWay.WARNED: return "You sold me the case and told the street I'd be back. Both were true. I like that in a man. The book likes it less.\n\nF. K.";
			case RZ_EFerdaWay.FOUGHT: return "Two of mine are buried. That's written down now, under your name.\n\nF. K.";
			case RZ_EFerdaWay.WAITED: return "I waited all afternoon in " + town + ". Next time I won't.\n\nF. K.";
		}

		return "The book is patient.\n\nF. K.";
	}

	// ================================================================ Without the talk screen
	//------------------------------------------------------------------------------------------------
	//! "Talk to Ferda Kroupa", or reading his note, with no talk screen (RZ_PuzzleActionHelper): his
	//! lines as subtitles, or the note in the reader.
	void HandleRead(int playerId, int code, IEntity owner)
	{
		if (code >= NOTE_BASE)
		{
			RZ_Reader.Show(m_GM, playerId, "A note on the step", "Folded once, a stone on it", NoteText(code - NOTE_BASE));
			return;
		}

		array<string> lines = {};
		Greet(code, playerId, lines);
		string title = FerdaTitle();
		if (code % 10 == ROLE_NEIGHBOUR)
			title = "A neighbour";

		foreach (string line : lines)
		{
			m_GM.Say(playerId, title, -1, line, owner);
		}
	}

	// ================================================================ The tests'
	//------------------------------------------------------------------------------------------------
	//! He comes now (the runtime test's).
	bool ComeNow(int playerId)
	{
		RZ_FixerVisit v = Of(playerId);
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!v || !me || v.m_iPhase != PHASE_WAIT)
			return false;

		return Come(v, me);
	}

	//------------------------------------------------------------------------------------------------
	//! The test's tester isn't a player the engine knows: it hands its own entity.
	bool ComeNowAt(int playerId, IEntity me)
	{
		RZ_FixerVisit v = Of(playerId);
		if (!v || !me || v.m_iPhase != PHASE_WAIT)
			return false;

		return Come(v, me);
	}

	//------------------------------------------------------------------------------------------------
	int PhaseOf(int playerId)
	{
		RZ_FixerVisit v = Of(playerId);
		if (!v)
			return PHASE_DONE;

		return v.m_iPhase;
	}

	//------------------------------------------------------------------------------------------------
	IEntity FerdaOf(int playerId)
	{
		RZ_FixerVisit v = Of(playerId);
		if (!v)
			return null;

		return v.m_Ferda;
	}

	//------------------------------------------------------------------------------------------------
	int CodeOf(int playerId, int role)
	{
		RZ_FixerVisit v = Of(playerId);
		if (!v)
			return -1;

		return v.m_iId * 10 + role;
	}

	//------------------------------------------------------------------------------------------------
	int CrewOf(int playerId)
	{
		RZ_FixerVisit v = Of(playerId);
		if (!v)
			return 0;

		return v.m_aCrew.Count();
	}

	//------------------------------------------------------------------------------------------------
	RZ_Errand StepFor(int playerId)
	{
		RZ_FixerVisit v = Of(playerId);
		if (!v)
			return null;

		return StepOf(v);
	}

	//------------------------------------------------------------------------------------------------
	//! The note's been put down for this player (the test reads it).
	IEntity NoteOf(int playerId)
	{
		IEntity note;
		m_mNotes.Find(playerId, note);
		return note;
	}

	//------------------------------------------------------------------------------------------------
	//! Slipping away, as if you'd walked off with it long enough (the test's tester can't walk).
	void SlipNow(int playerId)
	{
		RZ_FixerVisit v = Of(playerId);
		if (v && v.m_iPhase >= PHASE_TALK && v.m_iPhase <= PHASE_STAND)
			End(v, RZ_EFerdaWay.SLIPPED);
	}
}
