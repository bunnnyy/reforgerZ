//! Errands (docs/ERRANDS.md): the small, personal jobs one named person asks of one player, each
//! true because something in the world is true right now. This file holds the parts every errand
//! is made of: its ways, its definition (a routine row or a card class) and the live record.
//! The dealer is RZ_Errands; the routines are RZ_ErrandRoutines; the cards are in Cards/.
//! Server only.

//------------------------------------------------------------------------------------------------
//! What an errand's giver is: where it can be dealt.
enum RZ_EErrandAnchor
{
	HAVEN_HOUSE,		// a Haven safehouse's named people
	BASTION_POST,		// a Bastion outpost or stronghold
	REDHAND_CAMP,		// a Red Hand outpost or stronghold
	HALCYON_STATION,	// Halcyon's field station or the lab
	ANY_HOLDING,		// any faction's outpost or stronghold (the card says which faction gives it)
	WANDERER			// a wanderer, through "Need a hand?"
}

//------------------------------------------------------------------------------------------------
//! What a way asks you to hand over.
enum RZ_EErrandItem
{
	NONE,
	FOOD,
	FUEL,		// a jerrycan or a fuel can
	BATTERY,
	MEDICAL,	// bandages, pills, a medkit
	ANTIDOTE,
	MEAT		// a raw meat leg (a hunt's, a herd's: World/RZ_Wildlife.c), for a kitchen's pot
}

//------------------------------------------------------------------------------------------------
//! A prop at a step's place while it's open (Missions > Errands > Props; none set: words only).
enum RZ_EErrandProp
{
	NONE,
	EFFECTS,	// a dead man's things
	CRATE,		// a small crate or box
	PAPERS,		// a book or papers
	PARTS,		// a toolbox or a machine part
	CAR			// a car at the verge (Guidance > Car wreck prefab: the openings' car in the ditch)
}

//------------------------------------------------------------------------------------------------
//! Where a way's step happens (filled when it's offered; all within 1.5 km).
enum RZ_EErrandPlace
{
	NONE,			// done on the spot, at the giver
	HERE,			// at the giver's place (within 15 m)
	GATE,			// the place's gate or wire (15-25 m out)
	OUT,			// 200-400 m out from the place
	FAR,			// 300-500 m out
	EMPTY_VILLAGE,	// the nearest empty village within 1.2 km
	NEXT_TOWN,		// the nearest other town within 1.5 km
	SUBJECT,		// the card's own place (a wreck, an impact, another house)
	SUBJECT2		// the card's second place
}

//------------------------------------------------------------------------------------------------
//! An errand's state.
enum RZ_EErrandState
{
	OFFERED,	// said, not yet taken
	HELD,		// taken: its step is under way, or done and waiting to be reported
	DONE		// paid, lapsed, declined or overtaken: about to go
}

//------------------------------------------------------------------------------------------------
//! One way through an errand (docs/ERRANDS.md, "Ways, checks and talk"): what it needs (shown as
//! a tag before you choose), the one step it takes, and what it pays.
class RZ_ErrandWay
{
	int m_iIndex;
	string m_sWay = "do";		// telemetry: do, talk, pay, sneak, press, give, report, leave
	string m_sLabel;			// "Hand over two tins" (the tag is added from what it needs)

	// What it needs (checked when picked; tags from RZ_Checks)
	RZ_EErrandItem m_eItem = RZ_EErrandItem.NONE;
	int m_iItems;				// how many, handed over when picked (or at the place, m_bGiveThere)
	bool m_bGiveThere;			// the items are handed over at the step's place
	RZ_EErrandPlace m_eFirst = RZ_EErrandPlace.NONE;	// a place to go first (First), then the step's own
	string m_sFirst;			// its objective line
	float m_fFirstSeconds;		// how long there
	int m_iFavorCost;			// paid when picked, with the giver's faction
	string m_sBackground;		// one of these backgrounds ("nurse,doctor")
	int m_iTrust;				// a Trust check with the giver's faction (difficulty 1-3), 0 none
	int m_iTrustFaction = -1;	// the faction the Trust check reads (-1: the giver's)
	int m_iMinTier = -1;		// standing needed (RZ_ETier, a positive tier), -1 none
	int m_iTierFaction = -1;	// with this faction (-1: the giver's)
	bool m_bThreat;				// Press: a threat through RZ_Checks.Threaten (Fear 2)

	// The step (none: it resolves on the spot)
	RZ_EErrandPlace m_ePlace = RZ_EErrandPlace.NONE;
	float m_fSeconds;			// how long to stay there
	float m_fRadius = 25;		// within this of the place (at least the Arrive radius: IsAtPlace)
	bool m_bUnseen;				// only while hidden (built stealth): seen, and the clock stops
	bool m_bQuiet;				// a shot near it spoils the "unnoticed" bonus
	float m_fBefore = -1;		// must be done before this game hour (20.5 = half past eight), -1 none
	string m_sStep;				// the system line: "Search the farm buildings at {place}."
	RZ_EErrandProp m_eProp = RZ_EErrandProp.NONE;	// a prop at the place while the step is open
	bool m_bCarry;				// the step gives a carried item: only the asker or a helper hands it in (RZ_ErrandsPay)
	bool m_bEndsThere;			// done at the place: no going back to the giver (an opening's last step, Guidance/RZ_Openings.c)
	RZ_EWork m_eWork = RZ_EWork.NONE;	// done with your hands: a work site at the place, held, not stood at (Work())
	string m_sWorkLabel;		// its prompt ("Dig the latrine")
	string m_sInside;			// its work site is inside a building of this kind ("any", "house", "shop", "police", "school", "public", "clinic"), on the ground floor; empty: in the open
	bool m_bPerson;				// someone the work is for stands at its work site (the newcomer, the bitten man): Props > Duty person
	float m_fWorkSeconds;		// its hold (0: from m_fSeconds, RZ_WorkSites.HoldFor)
	string m_sNeed;				// the place need its words make (World/RZ_PlaceNeeds.c; empty: by its place kind, NeedKey)
	int m_iGame;				// a sorting duty (RZ_DutyGame.KIND_*): its row is laid out at the place, and its verdicts are the step
	bool m_bSquad;				// a duty walked with the place's people (RZ_DutySquads): the step is coming back with them

	// What it pays (docs/ERRANDS.md, "Pay: nothing for free")
	int m_iFavor;
	int m_iFame;				// 0-2, inside the ceiling
	int m_iPayFaction = -1;		// -1: the giver's faction
	bool m_bAmends;				// an amends way: 2 Infamy off instead (Watched)
	float m_fQuietMult = 1;		// x this if nobody heard a shot near it
	int m_iEffect;				// what the definition does on resolve (its own ids)

	// Lines, in the giver's voice (two or three variants each)
	ref array<string> m_aTake = {};
	ref array<string> m_aDone = {};
	ref array<string> m_aLater = {};	// a callback some hours of the asker's play later, near the giver (words only)

	//------------------------------------------------------------------------------------------------
	//! At the step's place (RZ_Arrive): m_fRadius, widened to the Arrive radius; in a vehicle, where
	//! the vehicle is.
	bool IsAtPlace(IEntity me, vector place)
	{
		return RZ_Arrive.AtSpot(me, place, m_fRadius);
	}

	//------------------------------------------------------------------------------------------------
	//! A place to go first, before the step's own (find the ring in the grass, then take it to the
	//! mother's door): its own objective line (docs/MISSION_RULES.md, rule 23), done by staying there
	//! `seconds`. `line` is its objective ("Search the grass {subject_place} for the ring.").
	void First(RZ_EErrandPlace place, string line, float seconds)
	{
		m_eFirst = place;
		m_sFirst = line;
		m_fFirstSeconds = seconds;
	}

	//------------------------------------------------------------------------------------------------
	bool HasFirst()
	{
		return m_eFirst != RZ_EErrandPlace.NONE;
	}

	//------------------------------------------------------------------------------------------------
	bool HasStep()
	{
		return m_ePlace != RZ_EErrandPlace.NONE;
	}

	//------------------------------------------------------------------------------------------------
	//! The step is work you do at a visible thing (docs/DESIGN.md, Work site): with work sites on, a
	//! hold prompt at its place (`label`, held `hold` s, else RZ_WorkSites.HoldFor(m_fSeconds)), and the world shows the
	//! result; off, it's done by staying there, as before.
	void Work(RZ_EWork kind, string label, float hold = 0)
	{
		m_eWork = kind;
		m_sWorkLabel = label;
		m_fWorkSeconds = hold;
	}

	//------------------------------------------------------------------------------------------------
	//! Done at a work site now (work sites on, and it has work).
	bool IsWork()
	{
		return m_eWork != RZ_EWork.NONE && HasStep() && RZ_WorkSites.Available();
	}

	//------------------------------------------------------------------------------------------------
	//! The step's line for the player. Done at a work site, its time there ("(30 s there)", "(a minute,
	//! nothing in your hands)") goes: the hold's bar is the time.
	string StepText()
	{
		if (!IsWork())
			return m_sStep;

		return RZ_WorkSites.WithoutTime(m_sStep);
	}

	//------------------------------------------------------------------------------------------------
	//! Its place need's key (docs/PLACES.md): its own, else by where the step is.
	string NeedKey()
	{
		if (!m_sNeed.IsEmpty())
			return m_sNeed;

		switch (m_ePlace)
		{
			case RZ_EErrandPlace.HERE: return "errand.here";
			case RZ_EErrandPlace.GATE: return "errand.here";
			case RZ_EErrandPlace.OUT: return "errand.out";
			case RZ_EErrandPlace.FAR: return "errand.out";
			case RZ_EErrandPlace.EMPTY_VILLAGE: return "errand.town";
			case RZ_EErrandPlace.NEXT_TOWN: return "errand.town";
		}

		return "errand.subject";
	}

	//------------------------------------------------------------------------------------------------
	//! The faction this way's checks and pay read.
	int PayFaction(int giver)
	{
		if (m_iPayFaction >= 0)
			return m_iPayFaction;

		return giver;
	}

	//------------------------------------------------------------------------------------------------
	void Take(string a, string b = "", string c = "")
	{
		Add(m_aTake, a, b, c);
	}

	//------------------------------------------------------------------------------------------------
	void Done(string a, string b = "", string c = "")
	{
		Add(m_aDone, a, b, c);
	}

	//------------------------------------------------------------------------------------------------
	//! What the giver says later (the one callback queue): 2-6 hours of the asker's play on, near them.
	void Later(string a, string b = "", string c = "")
	{
		Add(m_aLater, a, b, c);
	}

	//------------------------------------------------------------------------------------------------
	static void Add(notnull array<string> list, string a, string b, string c)
	{
		if (!a.IsEmpty())
			list.Insert(a);

		if (!b.IsEmpty())
			list.Insert(b);

		if (!c.IsEmpty())
			list.Insert(c);
	}
}

//------------------------------------------------------------------------------------------------
//! An errand's definition: a routine (a row in RZ_ErrandRoutines) or a card (a class in Cards/).
//! Define() declares it; Why() says whether it's true now and fills the live record's places and
//! text; Still() re-checks it every 30 s and at every step; Resolve() does its capped effects.
class RZ_ErrandDef
{
	string m_sKey;				// saved: never change it once shipped
	string m_sTitle;
	string m_sFamily = "chore";	// telemetry and novelty: chore, house, aftermath, lost, line, dead, crews, amends, wanderer
	bool m_bCard;				// once per player per season, at most Cards a day
	bool m_bAfterCure;			// the epilogue deck: dealt only in the epilogue week, once per identity per subject, outside Cards a day
	int m_iPerLife;				// > 0: dealt once per subject, this many times a life (His Effects: 3), not once a season
	RZ_EErrandAnchor m_eAnchor;
	int m_iFaction = -1;		// the giver's faction (ANY_HOLDING: the card picks)
	string m_sRoles;			// roles that give it first ("cook,storekeeper"); others may speak for them
	int m_iWandererRole = -1;	// WANDERER: the role that asks, -1 any
	int m_iNeed = -1;			// RZ_ENeed it answers (score x1.2 while it's the giver faction's need)
	float m_fWeight = 1;
	bool m_bTimed;				// needs a running game clock (RZ_GameTime.IsCalibrated)
	bool m_bTwists;				// can have a twist (the giver lies or hides something): rolled at Twist chance
	bool m_bHanded;				// handed straight to a player by another system (an opening's chain, Guidance/RZ_Openings.c):
								// never dealt, its giver isn't a named person, no cooldowns and no small pay (its owner pays)
	float m_fLapseMinutes;		// > 0: a held one lapses after this many real minutes instead of Lapse minutes
	bool m_bDuty;				// a duty (docs/ERRANDS.md, "Duties"): the place's everyday work, always there to be asked for, on the
								// duty gap instead of the routines' cooldowns, scored under whatever is true right now
	int m_iRung = 1;			// the trust rung it needs with the giver's faction (RZ_Duties.Rung)
	string m_sDuty;				// a duty's kind ("kitchen", "line"...): one kind in many voices shares one duty gap
	ref array<ref RZ_ErrandWay> m_aWays = {};
	ref array<string> m_aOffer = {};	// the ask, in the giver's voice
	ref array<string> m_aOver = {};		// the world changed: "Never mind. The truck came in after all."
	string m_sThread;			// what you did, for a thread: "brought tins for the pot" (empty: none)

	//------------------------------------------------------------------------------------------------
	void Define()
	{
	}

	//------------------------------------------------------------------------------------------------
	//! True now for this giver and player: fill e's subject (its soft hold), places and text. Spawns
	//! nothing.
	bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A step of an opening's chain (Guidance/RZ_OpeningChain.c): the one job in the game that starts
	//! without the player talking to anyone or using anything. Every other errand starts from a person
	//! or a thing.
	bool IsOpening()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Each errand tick while a held step isn't done, before the place's radius is checked: it may
	//! move e.m_vPlace (a subject that walks) and returns false to stop the step's clock. An
	//! opening's background step reads its own system here (Guidance/RZ_OpeningSteps.c).
	bool StepCounts(RZ_Errands dealer, RZ_Errand e, IEntity me)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Still true (every 30 s while offered or held, and at every step). False: overtaken.
	bool Still(RZ_Errands dealer, RZ_Errand e)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! What a carried way puts in the pack, if this errand has its own (a soldier's card, forged
	//! papers). "": Props > Carried, else the mission item.
	ResourceName CarriedPrefab(RZ_Errand e)
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! For whom (standing, background, life): checked with the player before Why.
	bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A way is offered to this player (it may need a background or a record): false hides it.
	bool Offers(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A way was taken (its step, if it has one, is open): a duty starts what it runs here.
	void Taken(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! The errand is over, however it ended ("done", "lapsed", "overtaken"...): let go of what it ran.
	void Ended(RZ_Errands dealer, RZ_Errand e, string why)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! The way's effect, once it's done (at most: 2 food at one house, 1 person, 1 morale, 1 stock).
	//! Lines added to `said` are spoken by the giver after the done line.
	void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! The premise subjects it shares with bigger content (a card only): a questline's or an arc's
	//! long hold on one of them overtakes the card; routines go on.
	void Premise(RZ_Errand e, notnull array<string> result)
	{
		if (m_bCard && !e.m_sSubject.IsEmpty())
			result.Insert(e.m_sSubject);
	}

	// ================================================================ Building one
	//------------------------------------------------------------------------------------------------
	//! A new way, in order.
	RZ_ErrandWay Way(string kind)
	{
		RZ_ErrandWay w = new RZ_ErrandWay();
		w.m_iIndex = m_aWays.Count();
		w.m_sWay = kind;
		m_aWays.Insert(w);
		return w;
	}

	//------------------------------------------------------------------------------------------------
	void Offer(string a, string b = "", string c = "")
	{
		RZ_ErrandWay.Add(m_aOffer, a, b, c);
	}

	//------------------------------------------------------------------------------------------------
	void Over(string a, string b = "", string c = "")
	{
		RZ_ErrandWay.Add(m_aOver, a, b, c);
	}
}

//------------------------------------------------------------------------------------------------
//! One errand dealt to one player: about 120 bytes of data. Runtime only (a restart lets it lapse).
class RZ_Errand
{
	int m_iId;
	RZ_ErrandDef m_Def;			// owned by the registry
	int m_iAsker;
	string m_sIdent;
	int m_iLife;
	string m_sGiver;			// RZ_NamedPeople key
	string m_sGiverName;
	string m_sGiverTitle;		// the title their lines come under (the talk screen catches it)
	int m_iFaction = -1;		// who pays: the giver's faction (a wanderer's errand: the Haven)
	vector m_vGiver;			// where to come back to
	vector m_vOffer;			// where it was offered (sizes are measured from here)
	int m_iHolding = -1;
	int m_iHouse = -1;
	int m_iWanderer = -1;
	int m_iSettlement = -1;
	string m_sSubject;			// its soft hold
	vector m_vSubject;
	vector m_vSubject2;
	int m_iValue = -1;			// a card's own number (another house, a faction)
	string m_sValue;			// a card's own word (a drawn truth)
	RZ_EErrandState m_eState = RZ_EErrandState.OFFERED;
	int m_iWay = -1;
	vector m_vPlace;			// the taken way's place
	float m_fProgress;
	bool m_bStepDone;
	vector m_vFirst;			// the way's first place (RZ_ErrandWay.First), zero: none
	bool m_bFirstDone;
	float m_fFirstProgress;
	bool m_bNoisy;				// a shot near it while it ran
	bool m_bUnasked;			// a call-out, not an ask
	bool m_bFallout;			// a failed story job's fallout (RZ_Fallout): pays at most m_iCap
	bool m_bTwist;				// its twist is true this time (the same replies show either way)
	bool m_bSuspect;			// the arsonist rule's suspect variant: the asker's own act made the fact
	int m_iCap = -1;			// Favor cap, -1 none
	int m_iFameCap = -1;		// Fame cap, -1 none (a judged duty that wasn't done perfectly)
	int m_iBonus;				// Favor added to the way's before the caps (a duty's scene: what you chose on the way)
	float m_fOfferedAt;			// the dealer's clock
	float m_fTakenAt;
	float m_fOfflineSince = -1;
	bool m_bTold;				// the "come back" line has been said
	ref array<int> m_aHelpers = {};
	IEntity m_Carried;			// a carried way's item, weak (RZ_ErrandsPay); null on the ground's loss
	bool m_bCarried;			// it was given (so a null m_Carried means lost)
	ref map<int, float> m_mWith = new map<int, float>();	// group members near the asker: seconds
	ref map<string, string> m_mText = new map<string, string>();

	//------------------------------------------------------------------------------------------------
	//! An opening's chain step (RZ_ErrandDef.IsOpening): handed to the player at spawn, not asked for.
	bool IsOpening()
	{
		return m_Def && m_Def.IsOpening();
	}

	//------------------------------------------------------------------------------------------------
	//! A text slot the lines use as {key} (the lint knows these keys from here).
	void SetText(string key, string value)
	{
		m_mText.Set(key, value);
	}

	//------------------------------------------------------------------------------------------------
	string GetText(string key)
	{
		string v;
		m_mText.Find(key, v);
		return v;
	}

	//------------------------------------------------------------------------------------------------
	//! The errand's slots, then the world's tokens, filled when shown, so a name is never stale.
	string Fill(string text)
	{
		string s = text;
		foreach (string key, string value : m_mText)
		{
			s.Replace("{" + key + "}", value);
		}

		return RZ_WorldFacts.Get().Fill(s);
	}

	//------------------------------------------------------------------------------------------------
	RZ_ErrandWay GetWay()
	{
		if (!m_Def || m_iWay < 0 || m_iWay >= m_Def.m_aWays.Count())
			return null;

		return m_Def.m_aWays[m_iWay];
	}

	//------------------------------------------------------------------------------------------------
	//! "errand:12", its soft-hold owner.
	string Owner()
	{
		return "errand:" + m_iId.ToString();
	}
}

//------------------------------------------------------------------------------------------------
//! Pay owed to someone who wasn't online when it was earned (a story job, the finale, a questline's
//! ending): paid at their next join through AddFame and AddFavor, to the same life only. Saved in
//! the errand ledger.
class RZ_ErrandOwed
{
	int faction;
	int fame;
	int favor;
	string title;
	int life;
	int madeUnix;
	bool applyRivals;				// the rival table (false when the job names its own rival)
	int flags;						// RZ_RepFlag for the Fame
	string deed;
	bool amends;					// an amends job with clean hands: Forgive while Watched or worse
	bool checkTier;					// the giver's tier is checked again at the join
	int minTier;
	bool allowWatched;
	bool leaderDecides;
	ref array<int> infamyFactions = {};	// the explicit rival's Infamy and the job's blame, with the pay
	ref array<int> infamyAmounts = {};
	ref array<int> infamyFlags = {};
	ref array<string> infamyWhy = {};

	//------------------------------------------------------------------------------------------------
	static RZ_ErrandOwed Make(int f, int fameOwed, int favorOwed, string why, int lifeNo)
	{
		RZ_ErrandOwed o = new RZ_ErrandOwed();
		o.faction = f;
		o.fame = fameOwed;
		o.favor = favorOwed;
		o.title = why;
		o.life = lifeNo;
		o.madeUnix = RZ_SeasonClock.Now();
		return o;
	}

	//------------------------------------------------------------------------------------------------
	void AddInfamy(int f, int amount, string why, int infamyFlag)
	{
		if (amount <= 0 || !RZ_Factions.IsValid(f))
			return;

		Ensure();
		infamyFactions.Insert(f);
		infamyAmounts.Insert(amount);
		infamyFlags.Insert(infamyFlag);
		infamyWhy.Insert(why);
	}

	//------------------------------------------------------------------------------------------------
	void Ensure()
	{
		if (!infamyFactions)
			infamyFactions = new array<int>();

		if (!infamyAmounts)
			infamyAmounts = new array<int>();

		if (!infamyFlags)
			infamyFlags = new array<int>();

		if (!infamyWhy)
			infamyWhy = new array<string>();
	}
}

//------------------------------------------------------------------------------------------------
//! The errand ledger, per identity for the season (docs/ERRANDS.md, "Pay"): the cards done, the
//! routines' cooldowns, the hourly Favor and its diminishing count, cards today, errand Fame per
//! faction per season day, amends per faction this life, the last three families. Saved.
class RZ_ErrandLedger
{
	string ident;
	ref array<string> cards = {};			// card keys done this season
	ref array<string> coolKeys = {};		// "routine" (3 h anywhere) and "routine|place" (1 real day)
	ref array<int> coolUnix = {};
	int hourUnix;							// the real hour the Favor below counts in
	int hourFavor;
	int hourCount;							// errands paid this real hour (diminishing returns)
	int cardDay = -1;						// real day of cardsToday
	int cardsToday;
	int fameDay = -1;						// season day of fameToday
	ref array<int> fameToday = {0, 0, 0, 0};
	int amendsLife = -1;					// the life amendsDone counts in
	ref array<int> amendsDone = {0, 0, 0, 0};
	ref array<int> worked = {0, 0, 0, 0};	// errands and duties done this season, per giver faction (the trust ladder's second rung)
	ref array<string> families = {};		// the last three families dealt
	int helped;								// errands done this season ("Most people helped")
	ref array<int> shy = {};				// houses whose fit for you is x0.5 for the season (sold one of theirs)
	int sinceTwist = 9;						// errands dealt since the last twist (one in three at most)
	ref array<string> threads = {};			// "place|giver|what you did", newest last, 12 at most
	ref array<string> threadsSaid = {};		// the threads already said back to you
	ref array<string> pending = {};			// before owed: dropped on load (it had no life)
	ref array<ref RZ_ErrandOwed> owed = {};	// pay at the next join, to the life that earned it
	int liedFaction = -1;					// a lie found out on a dead life: that faction's givers...
	int liedUntil;							// ...open with it, fit x0.8, until this real time
	bool liedSaid;

	//------------------------------------------------------------------------------------------------
	void Ensure()
	{
		if (!cards)
			cards = new array<string>();

		if (!coolKeys)
			coolKeys = new array<string>();

		if (!coolUnix)
			coolUnix = new array<int>();

		if (!fameToday)
			fameToday = new array<int>();

		while (fameToday.Count() < 4)
		{
			fameToday.Insert(0);
		}

		if (!amendsDone)
			amendsDone = new array<int>();

		while (amendsDone.Count() < 4)
		{
			amendsDone.Insert(0);
		}

		if (!worked)
			worked = new array<int>();

		while (worked.Count() < 4)
		{
			worked.Insert(0);
		}

		if (!families)
			families = new array<string>();

		if (!threads)
			threads = new array<string>();

		if (!pending)
			pending = new array<string>();

		if (!owed)
			owed = new array<ref RZ_ErrandOwed>();

		if (!threadsSaid)
			threadsSaid = new array<string>();

		if (!shy)
			shy = new array<int>();
	}

	//------------------------------------------------------------------------------------------------
	int CoolUntil(string key)
	{
		int i = coolKeys.Find(key);
		if (i < 0 || i >= coolUnix.Count())
			return 0;

		return coolUnix[i];
	}

	//------------------------------------------------------------------------------------------------
	void SetCool(string key, int until)
	{
		int i = coolKeys.Find(key);
		if (i < 0)
		{
			coolKeys.Insert(key);
			coolUnix.Insert(until);
		}
		else
		{
			coolUnix[i] = until;
		}

		// Old ones go: they can't block anything any more
		int now = RZ_SeasonClock.Now();
		for (int k = coolKeys.Count() - 1; k >= 0; k--)
		{
			if (coolUnix[k] < now)
			{
				coolKeys.Remove(k);
				coolUnix.Remove(k);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void NoteFamily(string family)
	{
		families.Insert(family);
		while (families.Count() > 3)
		{
			families.RemoveOrdered(0);
		}
	}
}

//------------------------------------------------------------------------------------------------
class RZ_ErrandsSave
{
	ref array<ref RZ_ErrandLedger> ledgers = {};
	ref array<ref RZ_NamedHouseSave> houses = {};
	ref array<string> counterKeys = {};		// the server's errand counters (per house: turns done)
	ref array<int> counterValues = {};
	ref array<ref RZ_Local> locals = {};		// World/RZ_Locals.c
	ref array<ref RZ_ErrandDeferred> deferred = {};	// pay waiting for a post's next truck (RZ_ErrandsPay)
}
