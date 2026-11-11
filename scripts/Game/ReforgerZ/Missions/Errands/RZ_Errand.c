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
	ANTIDOTE
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
	float m_fRadius = 25;		// within this of the place
	bool m_bUnseen;				// only while hidden (built stealth): seen, and the clock stops
	bool m_bQuiet;				// a shot near it spoils the "unnoticed" bonus
	float m_fBefore = -1;		// must be done before this game hour (20.5 = half past eight), -1 none
	string m_sStep;				// the system line: "Search the farm buildings at {place}."

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

	//------------------------------------------------------------------------------------------------
	bool HasStep()
	{
		return m_ePlace != RZ_EErrandPlace.NONE;
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
	RZ_EErrandAnchor m_eAnchor;
	int m_iFaction = -1;		// the giver's faction (ANY_HOLDING: the card picks)
	string m_sRoles;			// roles that give it first ("cook,storekeeper"); others may speak for them
	int m_iWandererRole = -1;	// WANDERER: the role that asks, -1 any
	int m_iNeed = -1;			// RZ_ENeed it answers (score x1.2 while it's the giver faction's need)
	float m_fWeight = 1;
	bool m_bTimed;				// needs a running game clock (RZ_GameTime.IsCalibrated)
	bool m_bTwists;				// can have a twist (the giver lies or hides something): rolled at Twist chance
	ref array<ref RZ_ErrandWay> m_aWays = {};
	ref array<string> m_aOffer = {};	// the ask, in the giver's voice
	ref array<string> m_aOver = {};		// the world changed: "Never mind. The truck came in after all."

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
	//! Still true (every 30 s while offered or held, and at every step). False: overtaken.
	bool Still(RZ_Errands dealer, RZ_Errand e)
	{
		return true;
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
	bool m_bNoisy;				// a shot near it while it ran
	bool m_bUnasked;			// a call-out, not an ask
	bool m_bFallout;			// a failed story job's fallout (RZ_Fallout): pays at most m_iCap
	bool m_bTwist;				// its twist is true this time (the same replies show either way)
	int m_iCap = -1;			// Favor cap, -1 none
	float m_fOfferedAt;			// the dealer's clock
	float m_fTakenAt;
	float m_fOfflineSince = -1;
	bool m_bTold;				// the "come back" line has been said
	ref array<int> m_aHelpers = {};
	ref map<int, float> m_mWith = new map<int, float>();	// group members near the asker: seconds
	ref map<string, string> m_mText = new map<string, string>();

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
	ref array<string> families = {};		// the last three families dealt
	int helped;								// errands done this season ("Most people helped")
	ref array<int> shy = {};				// houses whose fit for you is x0.5 for the season (sold one of theirs)
	int sinceTwist = 9;						// errands dealt since the last twist (one in three at most)

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

		if (!families)
			families = new array<string>();

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
}
