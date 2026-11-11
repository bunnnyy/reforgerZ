//! The tribes' records (docs/INFECTED.md, "Tribes"): a tribe, its parts, its tracks, a hunt, and
//! what's saved. The manager is RZ_Tribes (RZ_Tribes.c); bodies are RZ_TribeBodies.

//------------------------------------------------------------------------------------------------
//! A tribe's groups, as data. ALPHA is a one-member group of its own.
enum RZ_ETribePart
{
	ALPHA,
	GUARD,
	HUNTERS,
	YOUNG,
	SCOUT,
	DAY			// a day party group (hunters and young), or a scout's pack
}

//------------------------------------------------------------------------------------------------
//! How a tribe hunts (docs/INFECTED.md, Temperament). The numbers are the strategy table's columns.
enum RZ_ETemperament
{
	BOLD,
	CAUTIOUS,
	CUNNING,
	TERRITORIAL,
	NOMADIC
}

//------------------------------------------------------------------------------------------------
//! The time of day, as the tribes live it.
enum RZ_ETribePhase
{
	DAY,		// 06:00-18:30 (and always, with no game clock)
	DUSK,		// 18:30-20:30
	NIGHT,		// 20:30-05:00
	DAWN		// 05:00-06:00
}

//------------------------------------------------------------------------------------------------
//! What a record in the tribes' lists is: a tribe, or a rover (a lone one, a splinter).
enum RZ_ETribeKind
{
	TRIBE,
	LONE,		// an alpha or a challenger with no tribe (docs/INFECTED.md, Lone ones)
	SPLINTER	// a loose pack after a succession or a break-up
}

//------------------------------------------------------------------------------------------------
//! Who the alpha was: it sets the tribe's echo (docs/INFECTED.md, Echoes).
enum RZ_EAlphaKind
{
	RESIDENT,	// no echo
	SOLDIER,	// sets out on the 17:30 shift change, home at 05:30, a guard at the lair's edge at 20:30
	FARMER,		// home at first light the long way round, by the fields; prefers barns
	DRIVER,		// hunts along the roads first
	RUNNER,		// a runner or a courier: the young wait at a house's wire at dusk
	COLUMN		// the Day 2 column: at dawn it walks toward the line and stops short
}

//------------------------------------------------------------------------------------------------
//! A habit (docs/INFECTED.md, Habits): learned where it took people, faded by nights without.
enum RZ_EHabit
{
	ROAD,		// it took a truck's crew or a driver on a road: it hunts that road at that hour
	CROSSING,	// it took someone at a bridge or a ford: a hunter group waits there from dusk
	BACKDOOR,	// it took someone at the wire of a lived-in place: the young wait there at dusk
	SHOTS,		// it took someone after a firefight: it comes to gunfire from 1.5 times as far
	BELLS		// it took someone after a loud noise: it comes to noise from 1.5 times as far
}

//------------------------------------------------------------------------------------------------
//! A tribe's look, from where it grew up (docs/INFECTED.md, Looks).
enum RZ_ELook
{
	CIVILIAN,
	MILITARY,
	WORK,		// farm valleys (Zombies > Tribes > Work clothes prefabs)
	DUST,		// a quarry or a depot (Dust-grey prefabs)
	OUTDOOR		// forest and hills (Outdoor prefabs)
}

//------------------------------------------------------------------------------------------------
//! Where many of a tribe died in one counted night: its trails and day walks go round (rule 20).
class RZ_FearPlace
{
	vector m_vPos;
	int m_iUntil;			// the counted night it lasts until
}

//------------------------------------------------------------------------------------------------
class RZ_TribeHabit
{
	RZ_EHabit m_eKind;
	vector m_vPos;
	float m_fHour;			// game hour it was learned at (the road hour)
	float m_fStrength;		// 0-100: sets that night's trail at 50 or more, gone under 10
}

//------------------------------------------------------------------------------------------------
//! A character a tribe knows (rule 20): it killed its leader or a guard, or burned its lair.
class RZ_TribeMark
{
	int m_iPlayer;
	int m_iLife;			// the player's life number (RZ_PlayerProfile.m_iLives): a new life is a new character
	float m_fStrength;		// 100, falling 25 a counted night
}

//------------------------------------------------------------------------------------------------
//! One group of a tribe as data: a count at a position, walking a path or holding.
class RZ_TribePart : RZ_InfectedPart
{
	RZ_ETribePart m_eKind;
	int m_iYoung;						// of m_iCount, the young (a day party, a pack)
	ref array<vector> m_aPath = {};		// a trail, a day walk, or home
	int m_iStep;
	int m_iDir = 1;						// +1 out along the path, -1 back
	float m_fHoldUntil;					// the tribe clock (played seconds)
	bool m_bHop;						// a day party: legs of 100-200 m, held 4-6 minutes between
	float m_fLegLeft;					// metres left of this leg
	vector m_vLastStep;					// where it was at the last data tick (a hop's progress)
	vector m_vRest;						// its spot at the lair by day
	int m_iPost = -1;					// a scout's post
	bool m_bOut;						// on a trail or a day walk (not at the lair)
	bool m_bHome;						// walking home
	int m_iHeard;						// the last remembered noise it turned to
	vector m_vNoise;
	float m_fNoiseUntil;
	bool m_bPack;						// a scout's pack
	int m_iTarget = -1;					// the player a hunt or a pack is after, -1
	bool m_bHeldBack;					// the core in a hunt, until the first wave is down to 2
	int m_iWave;
	int m_iSide = 1;					// a flanker's side
	bool m_bStirred;					// stirred a town tonight
	int m_iTrail = -1;					// the trail it walks tonight (worn trails)
	bool m_bWaits;						// holds at the end of its path until it's sent home (a crossing, a wire, a border)

	//------------------------------------------------------------------------------------------------
	//! The role its bodies carry (stealth reads it).
	RZ_EInfectedRole Role()
	{
		switch (m_eKind)
		{
			case RZ_ETribePart.ALPHA: return RZ_EInfectedRole.ALPHA;
			case RZ_ETribePart.GUARD: return RZ_EInfectedRole.GUARD;
			case RZ_ETribePart.YOUNG: return RZ_EInfectedRole.YOUNG;
			case RZ_ETribePart.SCOUT: return RZ_EInfectedRole.SCOUT;
			case RZ_ETribePart.DAY: return RZ_EInfectedRole.DAY_PARTY;
		}

		return RZ_EInfectedRole.HUNTER;
	}

	//------------------------------------------------------------------------------------------------
	//! The behaviour layer's kind for its bodies (the young of a day party come first).
	RZ_EBodyKind BodyKind()
	{
		switch (m_eKind)
		{
			case RZ_ETribePart.ALPHA: return RZ_EBodyKind.ALPHA;
			case RZ_ETribePart.GUARD: return RZ_EBodyKind.GUARD;
			case RZ_ETribePart.YOUNG: return RZ_EBodyKind.YOUNG;
			case RZ_ETribePart.SCOUT: return RZ_EBodyKind.SCOUT;
		}

		return RZ_EBodyKind.HUNTER;
	}

	//------------------------------------------------------------------------------------------------
	//! A hunting group: hunters, young or a day party (not the core, not a scout).
	bool IsHunting()
	{
		return m_eKind == RZ_ETribePart.HUNTERS || m_eKind == RZ_ETribePart.YOUNG || m_eKind == RZ_ETribePart.DAY;
	}

	//------------------------------------------------------------------------------------------------
	bool IsCore()
	{
		return m_eKind == RZ_ETribePart.ALPHA || m_eKind == RZ_ETribePart.GUARD;
	}

	//------------------------------------------------------------------------------------------------
	//! Walk this path from its start (dir +1).
	void SetPath(array<vector> path)
	{
		m_aPath.Clear();
		foreach (vector p : path)
		{
			m_aPath.Insert(p);
		}

		m_iStep = 0;
		m_iDir = 1;
		m_bHasTarget = !m_aPath.IsEmpty();
		if (m_bHasTarget)
			m_vTarget = m_aPath[0];
	}
}

//------------------------------------------------------------------------------------------------
//! Where some of them passed (the Hunter's tracks).
class RZ_Track
{
	vector m_vPos;
	float m_fAt;			// the tribe clock
	int m_iCount;
	vector m_vHeading;
	bool m_bBig;			// the alpha was with them
	bool m_bYoung;
	bool m_bNight;
	int m_iWear;			// nights it was walked
}

//------------------------------------------------------------------------------------------------
//! A night hunt on one player (docs/INFECTED.md, "How a hunt comes for you"), with the night's
//! commitment: a third of what it set out with lost, two waves spent, or 10 minutes at the target.
class RZ_TribeHunt
{
	int m_iTarget = -1;
	float m_fStartedAt;		// the tribe clock
	int m_iSetOut;			// members when it started
	int m_iLost;
	int m_iWaves;
	float m_fLeashAt = -1;	// a body first within 150 m of the target
	float m_fLostAt = -1;	// the target lost since
	RZ_ETemperament m_eTemper;
	RZ_EStrategy m_eStrategy;
	RZ_HuntLink m_Hunters;	// weak (owned by RZ_Behaviour)
	RZ_HuntLink m_Young;
	RZ_HuntLink m_Core;
	int m_iFlankers;
	ref RZ_HuntContext m_Context;	// the target's ground (flush, bait, herd, ambush)
}

//------------------------------------------------------------------------------------------------
//! One tribe (docs/INFECTED.md, "Tribes").
class RZ_Tribe
{
	int m_iId;
	int m_iSeed;
	string m_sPlace;			// "the woods above Levie": every line about it carries the place
	vector m_vLair;
	string m_sLairKind;
	float m_fRadius = 700;
	RZ_ETemperament m_eTemper;
	string m_sPattern;			// "three short and a long"
	bool m_bColumn;				// the column (the first seed)
	float m_fHunger = 30;
	float m_fBank;				// the ones who ran: infection points banked (8 at most)
	int m_iPendingYoung;		// young who join at the next dusk
	int m_iRanDay;				// the season day of m_iRanToday
	int m_iRanToday;
	int m_iPlayerYoung;			// players' characters among its young (3 at most)
	ref array<string> m_aTaken = {};	// newest first, 12
	bool m_bFed;				// took someone in the last counted night: it sleeps the next day
	bool m_bTookTonight;
	int m_iAlphaSamples;		// alphas' sample drops this season (3 at most)
	float m_fGuardSampleAt = -10000;	// played minutes
	float m_fFounded;			// season days
	// The alpha
	bool m_bHasAlpha;
	string m_sAlphaWho;			// "a driver from Levie" (for the log; faction words are planned)
	float m_fLedSince;			// season days
	float m_fAlphaHealth = 1;
	float m_fAlphaHurtAt = -10000;	// the tribe clock
	int m_iAlphaTaken;
	// Members as saved (the parts are rebuilt from them)
	int m_iGuards;
	int m_iHunters;
	int m_iScouts;
	int m_iYoung;
	// Places (worked out again on load)
	ref array<vector> m_aGrounds = {};
	ref array<ref array<vector>> m_aTrails = {};
	ref array<vector> m_aPosts = {};
	ref array<ref RZ_TribePart> m_aParts = {};
	ref array<ref RZ_Track> m_aTracks = {};
	// Today and tonight
	int m_iAtDusk;
	int m_iLostTonight;
	bool m_bCalled;
	bool m_bSetOut;
	float m_fSetOutHour = 20;
	bool m_bHunted;				// set out tonight (the dawn count)
	bool m_bDawnCalled;
	bool m_bDayOut;
	bool m_bDayBack;
	int m_iDayPacks;
	float m_fLastCall = -10000;	// the tribe clock
	ref array<vector> m_aBlocked = {};	// player clusters a hunt broke off from tonight
	bool m_bVigil;
	vector m_vVigilAt;
	float m_fVigilBodiesUntil;	// world seconds: bodies stand the vigil 30 real minutes at most
	vector m_vFeedAt;
	float m_fFeedUntil = -1;	// the tribe clock
	float m_fToldBirdsAt = -10000;
	ref RZ_TribeHunt m_Hunt;
	// The life cycle (docs/INFECTED.md, "The life cycle")
	RZ_ETribeKind m_eKind;		// a tribe, or a rover (a lone one, a splinter)
	vector m_vGoal;				// a rover: where it's walking
	float m_fRoverUntil;		// a rover: season day it ends (a splinter's fate, a lone one's death)
	int m_iFrom = -1;			// a rover: the tribe it came from
	string m_sAlphaName;		// "Josef Hruby"
	RZ_EAlphaKind m_eAlphaKind;
	ref array<string> m_aHistory = {};	// its alpha's last 6 events, newest first
	bool m_bChallenged;			// its alpha has been challenged (no death of age)
	RZ_ELook m_eLook;
	float m_fMilitaryShare;		// of its members, those who were soldiers (its look mixes)
	float m_fDisturb;			// 0-100: at 100 the lair moves (rule 10)
	ref array<float> m_aDisturbBy = {0, 0, 0, 0, 0};	// by faction (RZ_EFaction), and [4] everyone else
	int m_iMovedDay = -1;		// season day of its last lair move
	float m_fWeakUntil;			// season days: the two weak days after a succession
	bool m_bNoDayParty;			// the morning of a succession: the splinters are on the roads instead
	bool m_bRestless;			// at Split at with no slot or no room
	bool m_bSplitDue;			// splits at the next dusk
	vector m_vSplitTo;
	int m_iSplitHost = -1;		// the weak neighbour whose wood the daughter takes
	int m_iMother = -1;
	int m_iClan;				// clan id (0 none)
	int m_iClanHold;			// season days both halves held 8 or more
	ref array<ref RZ_FearPlace> m_aFear = {};
	ref array<ref RZ_FearPlace> m_aFearGone = {};	// expired: no new one within 500 m until m_iUntil
	ref array<ref RZ_TribeHabit> m_aHabits = {};
	ref array<ref RZ_TribeMark> m_aMarks = {};
	ref array<int> m_aWear = {};				// per trail, 0-10
	ref array<int> m_aTrailsTonight = {};
	ref array<vector> m_aDeadTonight = {};		// where its members died this night (fear, habits)
	vector m_vDrift;			// the range's centre off the lair (vacuums, seasonal pull)
	ref array<vector> m_aLoop = {};			// a nomad's lairs
	int m_iLoopAt;
	float m_fNextMove;			// season days: a nomad's next move
	int m_iStarveNights;
	int m_iBonds;				// pairs taken together at one kill site, among its young
	float m_fLastTakenAt = -10000;	// the tribe clock
	vector m_vLastTaken;
	bool m_bClashDue;
	bool m_bClash;				// the Clash at the lair, now
	float m_fClashUntil;		// the tribe clock
	bool m_bLateHome;			// an echo walks it home late (soldier 05:30, farmer by the fields at 06:00)
	int m_iNoHuntToward = -1;	// a tribe it lost a border fight to...
	int m_iNoHuntUntil;			// ...it doesn't hunt toward until this counted night
	bool m_bShortNight;			// the night after a border fight: one trail
	int m_iDisturbNoise;		// the last remembered noise that disturbed the lair

	//------------------------------------------------------------------------------------------------
	bool IsRover()
	{
		return m_eKind != RZ_ETribeKind.TRIBE;
	}

	//------------------------------------------------------------------------------------------------
	//! The range's centre: the lair, drifted (into a vacuum, toward people late in the season).
	vector RangeCentre()
	{
		return m_vLair + m_vDrift;
	}

	//------------------------------------------------------------------------------------------------
	//! Its echo shows 3 season days into the alpha's lead (docs/INFECTED.md, Echoes).
	bool Echo(RZ_EAlphaKind kind)
	{
		return m_bHasAlpha && m_eAlphaKind == kind && RZ_SeasonClock.DaysSince(m_fLedSince) >= 3;
	}

	//------------------------------------------------------------------------------------------------
	//! The two weak season days after a succession: one trail, no flank, half a day party.
	bool IsWeakDays()
	{
		return RZ_SeasonClock.Days() < m_fWeakUntil;
	}

	//------------------------------------------------------------------------------------------------
	//! The strongest habit of a kind at 50 or more, or null.
	RZ_TribeHabit StrongHabit(RZ_EHabit kind)
	{
		RZ_TribeHabit best;
		foreach (RZ_TribeHabit h : m_aHabits)
		{
			if (h.m_eKind == kind && h.m_fStrength >= 50 && (!best || h.m_fStrength > best.m_fStrength))
				best = h;
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! An event in its alpha's history (6 kept, newest first).
	void AddHistory(string line)
	{
		m_aHistory.InsertAt(line, 0);
		while (m_aHistory.Count() > 6)
		{
			m_aHistory.Remove(m_aHistory.Count() - 1);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every member, from the parts (the alpha counts one).
	int Members()
	{
		int n = 0;
		foreach (RZ_TribePart p : m_aParts)
		{
			n += p.m_iCount;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Members by role: the young of hunter groups and day parties count as YOUNG, the rest of them
	//! as HUNTERS.
	int Count(RZ_ETribePart kind)
	{
		int n = 0;
		foreach (RZ_TribePart p : m_aParts)
		{
			bool mixed = p.m_eKind == RZ_ETribePart.HUNTERS || p.m_eKind == RZ_ETribePart.DAY;
			if (mixed && kind == RZ_ETribePart.YOUNG)
				n += p.m_iYoung;
			else if (mixed && kind == RZ_ETribePart.HUNTERS)
				n += p.m_iCount - p.m_iYoung;
			else if (p.m_eKind == kind)
				n += p.m_iCount;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! The saved counts, from the parts.
	void CountRoles()
	{
		m_iGuards = Count(RZ_ETribePart.GUARD);
		m_iHunters = Count(RZ_ETribePart.HUNTERS);
		m_iScouts = Count(RZ_ETribePart.SCOUT);
		m_iYoung = Count(RZ_ETribePart.YOUNG);
		m_bHasAlpha = Count(RZ_ETribePart.ALPHA) > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Under 8 members: hunts as data only, no day party, scouts call with no pack (rule 8).
	bool IsWeakened()
	{
		return Members() < 8;
	}

	//------------------------------------------------------------------------------------------------
	//! The alpha's stage by season days as leader: 0 Rising (0-5), 1 Prime (6-20), 2 Decline (21+).
	int Stage()
	{
		float led = RZ_SeasonClock.DaysSince(m_fLedSince);
		if (led < 6)
			return 0;

		if (led < 21)
			return 1;

		return 2;
	}

	//------------------------------------------------------------------------------------------------
	static string StageName(int stage)
	{
		switch (stage)
		{
			case 1: return "Prime";
			case 2: return "Decline";
		}

		return "Rising";
	}

	//------------------------------------------------------------------------------------------------
	//! Hunger's band: 0 fed (under 40), 1 lean, 2 hungry (60-84), 3 desperate (85-99), 4 starving.
	int Band()
	{
		if (m_fHunger >= 100)
			return 4;

		if (m_fHunger >= 85)
			return 3;

		if (m_fHunger >= 60)
			return 2;

		if (m_fHunger >= 40)
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	static string BandName(int band)
	{
		switch (band)
		{
			case 1: return "lean";
			case 2: return "hungry";
			case 3: return "desperate";
			case 4: return "starving";
		}

		return "fed";
	}

	//------------------------------------------------------------------------------------------------
	//! The range for hunting tonight: x1.25 hungry, x1.5 desperate or worse.
	float HuntRadius()
	{
		// A territorial tribe never hunts beyond its range
		if (m_eTemper == RZ_ETemperament.TERRITORIAL)
			return m_fRadius;

		int band = Band();
		if (band >= 3)
			return m_fRadius * 1.5;

		if (band == 2)
			return m_fRadius * 1.25;

		return m_fRadius;
	}

	//------------------------------------------------------------------------------------------------
	RZ_TribePart FindPart(RZ_ETribePart kind)
	{
		foreach (RZ_TribePart p : m_aParts)
		{
			if (p.m_eKind == kind && p.m_iCount > 0)
				return p;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The alpha's body (the character, not its group), if it has one.
	IEntity AlphaBody()
	{
		RZ_TribePart p = FindPart(RZ_ETribePart.ALPHA);
		if (!p || !p.HasBody())
			return null;

		return FirstMember(p.m_Body.m_Entity);
	}

	//------------------------------------------------------------------------------------------------
	//! The first living character of a unit (a group's member, or the character itself).
	static IEntity FirstMember(IEntity unit)
	{
		if (!unit)
			return null;

		array<IEntity> members = {};
		RZ_Util.GetUnitMembers(unit, members);
		foreach (IEntity m : members)
		{
			if (RZ_Util.IsCharacterAlive(m))
				return m;
		}

		if (ChimeraCharacter.Cast(unit))
			return unit;

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! A taken person's name or description, newest first (12 kept).
	void AddTaken(string who)
	{
		m_aTaken.InsertAt(who, 0);
		while (m_aTaken.Count() > 12)
		{
			m_aTaken.Remove(m_aTaken.Count() - 1);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! A tribe in state.json. Parts, trails, posts and tracks are worked out again on load.
class RZ_TribeSave
{
	int id;
	int seed;
	string place;
	float lairX;
	float lairZ;
	string lairKind;
	float radius;
	int temper;
	string pattern;
	bool column;
	float hunger;
	float bank;
	int pendingYoung;
	int ranDay;
	int ranToday;
	int playerYoung;
	ref array<string> taken = {};
	bool fed;
	int alphaSamples;
	float founded;
	bool hasAlpha;
	string alphaWho;
	float ledSince;
	float alphaHealth;
	int alphaTaken;
	int guards;
	int hunters;
	int scouts;
	int young;
	// The life cycle
	int kind;
	float goalX;
	float goalZ;
	float roverUntil;
	int fromTribe;
	string alphaName;
	int alphaKind;
	ref array<string> history = {};
	bool challenged;
	int look;
	float militaryShare;
	float disturb;
	ref array<float> disturbBy = {};
	int movedDay;
	float weakUntil;
	bool restless;
	int mother;
	int clan;
	int clanHold;
	ref array<float> fearX = {};
	ref array<float> fearZ = {};
	ref array<int> fearUntil = {};
	ref array<int> habitKind = {};
	ref array<float> habitX = {};
	ref array<float> habitZ = {};
	ref array<float> habitHour = {};
	ref array<float> habitStrength = {};
	ref array<int> markPlayer = {};
	ref array<int> markLife = {};
	ref array<float> markStrength = {};
	ref array<int> wear = {};
	float driftX;
	float driftZ;
	ref array<float> loopX = {};
	ref array<float> loopZ = {};
	int loopAt;
	float nextMove;
	int starveNights;
	int bonds;
	int noHuntToward;
	int noHuntUntil;
}

//------------------------------------------------------------------------------------------------
//! Two bordering tribes (docs/INFECTED.md, Borders), in state.json.
class RZ_BorderSave
{
	int a;
	int b;
	float offset;
	float lastFight;
	int reopenUntil;
}

//------------------------------------------------------------------------------------------------
//! The life cycle's lists in state.json: burned land, vacuums, the taken in the open, abandoned
//! lairs, towns gone dark, the displacement counts.
class RZ_TribeLifeSave
{
	ref array<float> burnX = {};
	ref array<float> burnZ = {};
	ref array<float> burnUntil = {};
	ref array<int> burnTrailUntil = {};
	ref array<float> vacX = {};
	ref array<float> vacZ = {};
	ref array<float> vacUntil = {};
	ref array<string> vacPlace = {};
	ref array<float> openX = {};
	ref array<float> openZ = {};
	ref array<float> openDay = {};
	ref array<string> openWho = {};
	ref array<float> lairX = {};
	ref array<float> lairZ = {};
	ref array<float> lairUntil = {};
	ref array<string> lairFinds = {};
	ref array<float> darkSince = {};
	ref array<int> displaced = {};
	int lastTruckNight;
	int lastSplitDay;
	int lastNewDay;
}

//------------------------------------------------------------------------------------------------
//! The tribes in state.json: the tribes, whether the season's seeds were made, the night's online
//! time (a counted night), and each settlement's last infection (for the ones who ran).
class RZ_TribesSave
{
	ref array<ref RZ_TribeSave> tribes = {};
	bool seeded;
	int nextId;
	float nightOnline;
	ref array<float> lastInfection = {};
	float tribeSamples;
	float allSamples;
	int countedNights;
	ref array<ref RZ_TribeSave> rovers = {};
	ref array<ref RZ_BorderSave> borders = {};
	ref RZ_TribeLifeSave life;
}
