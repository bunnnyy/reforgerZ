//! The errand dealer (docs/ERRANDS.md): small, personal jobs one named person asks of one player,
//! each true because something in the world is true right now, and over the moment it stops being
//! true. It runs on the game mode's tick beside the mission manager, outside the random picks and
//! the contract cap. Givers are named people (World/RZ_NamedPeople.c) at the four holding kinds and
//! the wanderers; they answer "Anything need doing?" (or "Need a hand?"), and a holding's people may
//! wave a passing player over (one unasked contact, through the notice lanes' interruption budget).
//! An errand's replies are its giver's choice slots while that one player talks to them; anyone
//! else at the same person hears "Give us a minute."
//! Pay is small and capped (Fame at most 2 an errand, never above 39 with a faction, 10 a faction a
//! season day, left out of season Fame; Favor 3-10 inside 40 an hour with diminishing returns;
//! amends instead at 15+ Infamy). Errands never move votes or leaders and never close a route.
//! Server only.

//------------------------------------------------------------------------------------------------
//! The props a step's place may show while it's open (Missions > Errands > Props). All optional:
//! empty = that step is words only, as before.
[BaseContainerProps()]
class RZ_ErrandProps
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Effects (optional): a dead man's things in a heap, a kit bag with a jacket on it (His Effects, The Name on the Tape, Notebook 31, Into the Record). Empty = words only", "et")]
	ResourceName m_sEffects;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Crate (optional): a small supply crate or a box of odds and ends (The Truck Didn't Come, Clean Glasses, Pack for the Queue). Empty = words only", "et")]
	ResourceName m_sCrate;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Papers (optional): a book or a few papers on a stool or a seat (Last Orders). Empty = words only", "et")]
	ResourceName m_sPapers;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Parts (optional): a toolbox or a machine part on the ground (Filter Parts, Buy It Back's box). Empty = words only", "et")]
	ResourceName m_sParts;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Pay ration (optional): a Bastion ration a post pays in kind, one errand in three, never while Bastion is short (tagged: nobody else takes it for pay). Empty = Favor only", "et")]
	ResourceName m_sPayRation;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Pay magazine (optional): a magazine a Red Hand camp pays in kind, the same way. Empty = Favor only", "et")]
	ResourceName m_sPayMagazine;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Carried (optional): what a carried way puts in your pack (His Effects' effects, The Name on the Tag's ID): an item with RZ_MissionItemComponent that can't be equipped. Empty = the mission item", "et")]
	ResourceName m_sCarried;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Soldier's ID (optional): the card The Patrol That Didn't Come Back carries home, an item with RZ_MissionItemComponent (RZ_SoldiersID_Carried.et). Empty = Carried", "et")]
	ResourceName m_sSoldiersID;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Forged papers (optional): the set False Hope at the Line buys off the forger as proof, an item with RZ_MissionItemComponent (RZ_Passport_Forged.et). Empty = Carried", "et")]
	ResourceName m_sForgedPapers;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Watch (optional): The Last Tin's watch, a real wristwatch traded for two tins. Empty = a common find", "et")]
	ResourceName m_sWatch;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Duty body (optional): one of the dead laid out for the duty that sorts them, a body under a sheet or a body bag on a board, as a composition of props with nothing to use. Empty = Effects", "et")]
	ResourceName m_sDutyBody;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Duty person (optional): someone waiting in the line at the gate, a standing civilian with no weapon and no AI, or a prop figure. Empty = Puzzles > Civilian", "et")]
	ResourceName m_sDutyPerson;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Duty person looks (optional): people in their own clothes for the line at the gate, so no two waiting side by side are the same; each picks one by the errand and its place in the line. Empty = three vanilla civilians and the Duty person", "et")]
	ref array<ResourceName> m_aDutyPersonLooks;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Duty tray (optional): a sample tray on Halcyon's bench, a small rack or case of vials. Empty = Crate", "et")]
	ResourceName m_sDutyTray;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Duty vodka (optional): the bottle the Red Hand's still gives whoever ran the batch (BanditGear's vodka bottle). Empty = no bottle, only the pay", "et")]
	ResourceName m_sDutyVodka;

	[Attribute("{C9CFDED29542A968}Prefabs/Props/Military/Furniture/CotMilitary_US_01.et", UIWidgets.ResourcePickerThumbnail, "Round bed (optional): the cot each of The Round's sick lies on, indoors, under the tent or in the yard. Only a prop measured in tools/camp_kit/footprints.json and known to RZ_DutyRound.Box is used. Empty or unmeasured = no cots: they lie on the floor in the tent or yard, and never indoors", "et")]
	ResourceName m_sRoundBed;

	[Attribute("{6204943E9EC70EC2}Prefabs/Structures/Military/Camps/Canvas_Covers/US/CanvasCover_Medium_US.et", UIWidgets.ResourcePickerThumbnail, "Round tent (optional): the shelter The Round's sick lie under by the safehouse wall when the house has no room for them, with the cots (Round bed) laid out under it as JTF Bronze Props' casualty point lays them. Only a prop measured in tools/camp_kit/footprints.json and known to RZ_DutyRound.Box is used. Empty or unmeasured = the yard by the wall", "et")]
	ResourceName m_sRoundTent;

	//------------------------------------------------------------------------------------------------
	ResourceName For(RZ_EErrandProp kind)
	{
		switch (kind)
		{
			case RZ_EErrandProp.EFFECTS: return m_sEffects;
			case RZ_EErrandProp.CRATE: return m_sCrate;
			case RZ_EErrandProp.PAPERS: return m_sPapers;
			case RZ_EErrandProp.PARTS: return m_sParts;
			case RZ_EErrandProp.CAR: return CarPrefab();
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The car a step names (Shift Change's private asleep in one): the openings' car in the ditch.
	static ResourceName CarPrefab()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetGuidance() || !gm.GetGuidance().GetSettings())
			return "";

		return gm.GetGuidance().GetSettings().m_sCarWreckPrefab;
	}
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Missions > Errands.
[BaseContainerProps()]
class RZ_ErrandSettings
{
	[Attribute("1", desc: "Errands on: named people at safehouses, posts, camps and stations, and the wanderers, ask small jobs that are true right now (docs/ERRANDS.md)")]
	bool m_bEnabled;

	[Attribute("8", UIWidgets.Slider, "Offer gap: real minutes between two unasked errand offers to one player (a holding's people waving you over). Asking is never gapped", "1 60 1")]
	float m_fOfferGapMinutes;

	[Attribute("2", UIWidgets.Slider, "Errands one player may hold at once", "1 4 1")]
	int m_iMaxHeld;

	[Attribute("2", UIWidgets.Slider, "Cards (the hand-written errands, once per player per season) a player may be dealt a real day", "0 10 1")]
	int m_iCardsPerDay;

	[Attribute("1500", desc: "Errand reach (m): a giver deals an errand whose subject lies further than this from them only when no nearer one is true, and then says it's a long walk. Kept to 500-10000; 0 = off")]
	float m_fErrandReach;

	[Attribute("1", UIWidgets.Slider, "Pay scale: errand Favor is multiplied by this", "0 3 0.05")]
	float m_fPayScale;

	[Attribute("39", UIWidgets.Slider, "Fame ceiling: errand Fame never takes Fame with a faction above this (errands can make you Known, never Trusted)", "0 74 1")]
	int m_iFameCeiling;

	[Attribute("10", UIWidgets.Slider, "Errand Fame per faction per season day, per player", "0 30 1")]
	int m_iFamePerDay;

	[Attribute("8", UIWidgets.Slider, "Amends cap: Infamy errands work off per faction per life (2 at a time, never below 15)", "0 30 1")]
	int m_iAmendsCap;

	[Attribute("40", UIWidgets.Slider, "Errand Favor per player per real hour (diminishing: x1, x1, x0.75, x0.5, x0.25)", "0 200 5")]
	int m_iFavorPerHour;

	[Attribute("45", UIWidgets.Slider, "Real minutes a held errand lapses after it's taken (20 if the player goes offline)", "10 180 5")]
	float m_fLapseMinutes;

	[Attribute("1", desc: "Duties: a named person with nothing true to ask gives one of the place's own duties (the kitchen, the wood and water, washing down...), so work is there from the first day. Off: errands only while something is true (docs/ERRANDS.md, \"Duties\")")]
	bool m_bDuties;

	[Attribute("20", UIWidgets.Slider, "Duty gap: real minutes before one player is given the same kind of duty again (duties skip the routines' cooldowns)", "1 240 1")]
	float m_fDutyGapMinutes;

	[Attribute("0.25", UIWidgets.Slider, "Fault chance: each fault a place can have (a post's relief, a set low, a generator dry) rolls at this once a shift, with a player within 2 km", "0 1 0.05")]
	float m_fFaultChance;

	[Attribute("1", desc: "A holding's or safehouse's named people wave a passing player over when they have something true to ask (one unasked contact, inside the interruption budget)")]
	bool m_bCallOut;

	[Attribute("1", desc: "Wanderers' asks: \"Need a hand?\" on the wanderers (the Doctor's Before Dark, Barter Chain, Wanted Poster)")]
	bool m_bWandererAsks;

	[Attribute("1", desc: "Strangers: a passer-by met only at a spot where one makes sense (a road people use, an approach, a town edge at dusk, a fresh event), at most one per player every 15-25 minutes and 6 on the server")]
	bool m_bStrangers;

	[Attribute("15", UIWidgets.Slider, "Stranger gap: real minutes, at least, before one player can meet another stranger (a ceiling only: a stranger still needs a spot that makes sense)", "5 120 1")]
	float m_fStrangerGapMin;

	[Attribute("25", UIWidgets.Slider, "...and at most (the gap is drawn between the two)", "5 180 1")]
	float m_fStrangerGapMax;

	[Attribute("6", UIWidgets.Slider, "Strangers out on the server at once (never more than the body budget's 6)", "0 6 1")]
	int m_iStrangersMax;

	[Attribute("6", UIWidgets.Slider, "Footfall half-life: real hours for a road's player count to halve (how fast a road stops being one people use)", "1 48 1")]
	float m_fFootfallHours;

	[Attribute("30", UIWidgets.Slider, "Pinned locals on the server (saved, promised, sold: kept to the season's end; 4 a house)", "0 60 1")]
	int m_iPinnedLocals;

	[Attribute("20", UIWidgets.Slider, "Unpinned locals on the server (past this, the oldest nobody has met for 3 real days goes)", "0 60 1")]
	int m_iUnpinnedLocals;

	[Attribute("0", desc: "Majors read errand memory: a major's sitting may count +1 per head a season for a house whose clerk remembers your errands. Off: the majors ignore errands (the design's default)")]
	bool m_bMajorsReadMemory;

	[Attribute("0.2", UIWidgets.Slider, "Found hook chance: after a job ends at a site, this share of sites holds a stranger (someone hiding, someone who got there first, a bite) or an old mark, for 2 hours", "0 1 0.05")]
	float m_fHookChance;

	[Attribute("1", desc: "Dark cards: the strangers and cards where you can sell someone (Wrong Side of the Toll, She Waves at the Line, My Sister at Morton's knowing way)")]
	bool m_bDarkCards;

	[Attribute("0.2", UIWidgets.Slider, "Twist chance: a card that can have a twist (the giver lies or hides something) has it this often, at most one errand in three a player and never two in a row", "0 1 0.05")]
	float m_fTwistChance;

	[Attribute("1", desc: "Board asks: a safehouse's notice board shows up to two asks from the house, signed with its name, apart from the players' notes")]
	bool m_bBoardAsks;

	[Attribute("1", desc: "Radio asks: now and then one player near a house hears a personal line that someone there could use a hand (never broadcast), inside the interruption budget")]
	bool m_bRadioAsks;

	[Attribute("60", UIWidgets.Slider, "Radio asks: real minutes between two to one player", "10 360 5")]
	float m_fRadioAskMinutes;

	[Attribute("1", desc: "The hall of fame's \"Most people helped\" line: errands done, until the cure. It pays nothing")]
	bool m_bHelpedLine;

	[Attribute("30", UIWidgets.Slider, "Telemetry: real minutes between the errands' status log lines", "5 240 5")]
	float m_fTelemetryMinutes;

	[Attribute("0", desc: "Errands debug log: every deal and why the rest weren't dealt")]
	bool m_bDebugLog;

	[Attribute("1", desc: "Pay in kind: a Bastion post pays a ration (a Red Hand camp a magazine) in place of 3 Favor, one errand in three, never while it's short (needs Props > Pay ration or Pay magazine)")]
	bool m_bPayInKind;

	[Attribute("1", desc: "Meals as pay: a house errand at a house fed above one upkeep pays supper (the house -1 food, a little healing, 2 Favor) in place of 4 Favor or less")]
	bool m_bMeals;

	[Attribute("1", desc: "When the truck comes: a post short of supplies pays its errands at its next truck, to the life that earned it (a real day at most)")]
	bool m_bTruckPay;

	[Attribute("2", UIWidgets.Slider, "Errand headlines a real day, at most (a stranger's or a card's)", "0 10 1")]
	int m_iHeadlinesPerDay;

	[Attribute(desc: "Props a step's place shows while it's open (all optional; empty = words only)")]
	ref RZ_ErrandProps m_Props;

	[Attribute(desc: "Things on show at a duty: display copies with nothing to pick up, and the lock-up's dressing (all optional; empty = words only)")]
	ref RZ_DutyThingProps m_Things;

	protected static ref RZ_ErrandSettings s_Defaults;

	//------------------------------------------------------------------------------------------------
	RZ_ErrandProps Props()
	{
		if (!m_Props)
			m_Props = RZ_ErrandProps.Cast(RZ_Util.NewWithDefaults("RZ_ErrandProps"));

		return m_Props;
	}

	//------------------------------------------------------------------------------------------------
	RZ_DutyThingProps Things()
	{
		if (!m_Things)
			m_Things = RZ_DutyThingProps.Cast(RZ_Util.NewWithDefaults("RZ_DutyThingProps"));

		return m_Things;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_ErrandSettings Get()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm && gm.GetErrandSettings())
			return gm.GetErrandSettings();

		if (!s_Defaults)
			s_Defaults = RZ_ErrandSettings.Cast(RZ_Util.NewWithDefaults("RZ_ErrandSettings"));

		return s_Defaults;
	}
}

//------------------------------------------------------------------------------------------------
//! One player talking to one giver about an offer: the giver's choice slots are the offer's ways
//! until they pick, walk off or 90 s pass.
class RZ_ErrandTalk
{
	string m_sGiver;
	IEntity m_Owner;
	int m_iPlayer;
	int m_iErrand;
	float m_fUntil;
	string m_sUse;			// the slots as they were
	string m_sChoice;
	ref array<int> m_aWays = {};	// slot -> way index; -1 "Not now"
}

//------------------------------------------------------------------------------------------------
//! Telemetry for one family (docs/ERRANDS.md, "Telemetry").
class RZ_ErrandStats
{
	int deals;
	int declines;
	int lapses;
	int overtakes;
	int done;
	int favor;
	int fame;
	int amends;
	ref map<string, int> ways = new map<string, int>();
}

//------------------------------------------------------------------------------------------------
//! A failed story job's fallout (docs/ERRANDS.md, "Fallout"): one errand of `card` at `at`, dealt to
//! the job's participants first for 20 minutes, then to anyone, for 3 hours, paying at most `cap`
//! Favor. Not saved: a restart lets it go.
class RZ_Fallout
{
	string card;
	vector at;
	string who;					// the card's {who} line
	string source;				// the job's title
	int cap;
	float privateUntil;			// the dealer's clock
	float until;
	ref array<string> idents = {};
}

//------------------------------------------------------------------------------------------------
class RZ_Errands
{
	protected static const float SLOW_TICK = 30;
	protected static const float BOARD_DOOR_REACH = 40;	// boarding up: the place's real door this close (m)
	protected static const float INSIDE_RADIUS = 60;	// an indoor work site: the nearest fitting building this far from the place
	protected static const float TALK_SECONDS = 90;
	protected static const float TALK_RANGE = 20;
	protected static const float OFFER_MINUTES = 10;
	protected static const float CALL_RANGE = 30;
	protected static const float RETURN_RANGE = 15;		// back at the giver: widened to the Arrive radius (RZ_Arrive)
	protected static const float MAX_DISTANCE = 1500;
	protected static const float PLACE_COOL = 1200;		// per place per routine, server-wide
	protected static const int COOL_ANYWHERE = 10800;	// per player per routine, anywhere: 3 real hours
	protected static const int COOL_HERE = 86400;		// per player per place: a real day
	protected static const float HELPER_RANGE = 50;
	protected static const float HELPER_SECONDS = 120;
	protected static const int MAX_HELPERS = 3;
	protected static const int MAX_LIVE = 12;
	protected static const int DUTIES_PER_PLACE = 3;	// duties live at one holding or house at once
	static const int KNOWN_FACE = 3;					// errands and duties done for a faction this season: its second rung
	protected static const int AMENDS_FLOOR = 15;
	protected static const int CALLOUTS_PER_TICK = 3;
	protected static const float FALLOUT_PRIVATE = 1200;	// the participants' 20 minutes
	protected static const float FALLOUT_LIFE = 10800;		// then anyone's, for 3 hours
	protected static const float FALLOUT_RANGE = 3000;		// a giver within this of its subject
	protected static const float FIRST_LIFE_WAIT = 600;		// a first life: no call-outs until 10 minutes after Find the Haven

	protected RZ_GameModeComponent m_GM;
	protected RZ_ErrandSettings m_Settings;
	protected ref RZ_NamedPeople m_People;
	protected ref RZ_Faults m_Faults;
	protected ref RZ_Strangers m_Strangers;
	protected ref RZ_Locals m_Locals;
	protected ref array<ref RZ_ErrandDef> m_aDefs = {};
	protected ref array<ref RZ_Errand> m_aErrands = {};
	protected ref array<ref RZ_Fallout> m_aFallout = {};
	protected ref map<string, ref RZ_ErrandLedger> m_mLedgers = new map<string, ref RZ_ErrandLedger>();
	protected ref map<string, ref RZ_ErrandTalk> m_mTalks = new map<string, ref RZ_ErrandTalk>();
	protected ref map<string, float> m_mPlaceCool = new map<string, float>();
	protected ref map<string, float> m_mLastUnasked = new map<string, float>();	// ident -> clock
	protected ref map<int, string> m_mLastWhy = new map<int, string>();			// playerId -> the last deal and why the rest weren't
	protected ref map<int, string> m_mPrefer = new map<int, string>();			// playerId -> the errand the talk screen named (Peek), dealt first when asked
	protected ref map<int, string> m_mPreferDuty = new map<int, string>();		// the same for the duty named beside it
	static const int DEAL_ANY = 0;		// the pressing one, else a duty
	static const int DEAL_DUTY = 1;		// a duty only (the one offered beside the pressing errand)
	protected ref map<string, ref RZ_ErrandStats> m_mStats = new map<string, ref RZ_ErrandStats>();
	protected ref map<string, int> m_mCounters = new map<string, int>();		// "house.<idx>": turns done there this season
	protected ref map<int, int> m_mHousePop = new map<int, int>();			// safehouse -> people last seen
	protected ref map<int, int> m_mTookIn = new map<int, int>();			// safehouse -> season day it last took someone in
	protected ref array<vector> m_aShots = {};
	protected ref array<float> m_aShotAt = {};
	protected ref RZ_ErrandsSave m_Restored;
	protected int m_iNextId = 1;
	protected int m_iCallCursor;
	protected int m_iRadioCursor;
	protected ref map<string, float> m_mLastRadio = new map<string, float>();	// ident -> clock of the last radio ask
	protected ref map<string, float> m_mHavenFound = new map<string, float>();	// ident -> clock a first life was first seen at the Haven
	protected float m_fClock;
	protected float m_fSlow;
	protected float m_fTelemetry;
	protected int m_iUnaskedDeals;
	protected int m_iAskedDeals;
	protected int m_iTwists;									// twists dealt, for the telemetry line
	protected ref array<string> m_aLiesSeen = {};				// errands' lies in the store at the last telemetry line

	//------------------------------------------------------------------------------------------------
	void RZ_Errands(RZ_GameModeComponent gm, RZ_ErrandSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		m_People = new RZ_NamedPeople(gm);
		m_Faults = new RZ_Faults(gm);
		m_Strangers = new RZ_Strangers(gm, this);
		m_Locals = new RZ_Locals(gm);
		m_PayKinds = new RZ_ErrandsPay(this);
		m_More = new RZ_ErrandsDealer(this);
		m_Games = new RZ_DutyGames(gm, this);
		m_Squads = new RZ_DutySquads(gm, this);
		m_Scenes = new RZ_DutyScenes(gm, this);
	}

	protected ref RZ_ErrandsPay m_PayKinds;
	protected ref RZ_ErrandsDealer m_More;
	protected ref RZ_DutyGames m_Games;			// the sorting duties, played at their place (RZ_DutyGame.c)
	protected ref RZ_DutySquads m_Squads;		// the people who go out with a player on a duty (RZ_DutySquads.c)
	protected ref RZ_DutyScenes m_Scenes;		// what a duty has you see, find and decide (RZ_DutyScenes.c)

	//------------------------------------------------------------------------------------------------
	RZ_ErrandsDealer Dealer() { return m_More; }

	//------------------------------------------------------------------------------------------------
	RZ_ErrandsPay PayKinds() { return m_PayKinds; }

	//------------------------------------------------------------------------------------------------
	//! A supply truck event (RZ_MissionManager.OnTruckEvent): pay waiting for the truck goes out.
	void OnTruck(int faction, int what, vector pos)
	{
		if (what == RZ_SupplyLines.TRUCK_ARRIVED)
			m_PayKinds.OnTruck(faction, pos);
	}

	//------------------------------------------------------------------------------------------------
	//! His Effects in full (docs/ERRANDS.md, the dead): at the fence, a held His Effects whose step is
	//! done and whose effects this player carries sells for the jacket (3) and the photo frame (10 Red
	//! Hand Favor); seen by the giver's people (the witness rule), +5 Infamy with them. A kit piece off
	//! a man you killed is refused ("You'd know. You were there.", +3 Infamy with his faction). True if
	//! the fence dealt with it.
	bool SellEffects(int playerId, IEntity user)
	{
		foreach (RZ_Errand e : m_aErrands)
		{
			if (e.m_eState != RZ_EErrandState.HELD || e.m_Def.m_sKey != "his_effects" || !e.m_bStepDone || !e.m_Carried)
				continue;

			if (RZ_ErrandsPay.Carrier(e) != playerId)
				continue;

			if (RZ_ErrandCard.CausedBy(e.m_sValue, playerId))
			{
				if (RZ_Factions.IsValid(e.m_iFaction))
					m_GM.AddInfamy(playerId, e.m_iFaction, 3, "sold a dead man's things you'd made dead");

				m_GM.NotifyPlayer(playerId, "The fence", "No. Not this jacket, not from you. You'd know whose it was. You were there when it happened.");
				End(e, "refused at the fence");
				return true;
			}

			SmallFavor(playerId, RZ_EFaction.BANDITS, 3);
			SmallFavor(playerId, RZ_EFaction.BANDITS, 10);
			if (RZ_Factions.IsValid(e.m_iFaction) && e.m_iFaction != RZ_EFaction.BANDITS && RZ_Witness.Seen(m_GM, playerId, user.GetOrigin(), e.m_iFaction))
				m_GM.AddInfamy(playerId, e.m_iFaction, 5, "sold a dead man's things");

			m_GM.NotifyPlayer(playerId, "The fence", "Jacket on the hook, frame under the counter, face down. People like a frame. Nobody wants the photo. Here's your Favor.");
			array<int> seller = {playerId};
			RZ_WorldFacts.Get().Memory().Record("errand.his_effects", e.m_sSubject, "sold", RZ_WorldMemory.SOURCE_ERRAND, true, seller);
			ConsumeCarried(e);
			End(e, "sold at the fence");
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The carried item handed over: out of the pack and gone.
	protected void ConsumeCarried(RZ_Errand e)
	{
		if (!e.m_Carried)
			return;

		IEntity root = e.m_Carried.GetRootParent();
		if (root && root != e.m_Carried)
			RZ_MissionItemHelper.RemoveItem(root, e.m_Carried);
		else
			RZ_Util.DeleteUnit(e.m_Carried);

		e.m_Carried = null;
	}

	//------------------------------------------------------------------------------------------------
	RZ_Locals GetLocals() { return m_Locals; }

	// ================================================================ Hand to Hand, for the finale
	//------------------------------------------------------------------------------------------------
	//! Hand to Hand's promises (docs/ERRANDS.md, the cards), read-only for the finale: the living
	//! locals of safehouse `house` promised by hand, by anyone or, with `ident`, by that identity.
	int PromisedAt(int house, string ident = "")
	{
		if (!m_Locals || house < 0)
			return 0;

		array<RZ_Local> here = {};
		m_Locals.AtHouse(house, here);
		int n = 0;
		foreach (RZ_Local l : here)
		{
			if (l.alive && Promised(l, ident))
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! The living promised locals whose house is within `radius` of pos (by anyone, or by `ident`):
	//! the ones who'd be there at a gate or a sit-in near it.
	int PromisedNear(vector pos, float radius, string ident = "")
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses)
			return 0;

		int n = 0;
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			if (h && RZ_Util.DistXZ(h.m_vPos, pos) <= radius)
				n += PromisedAt(i, ident);
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Promised(RZ_Local l, string ident)
	{
		foreach (int i, string who : l.idents)
		{
			if (l.memos[i] == "promised" && (ident.IsEmpty() || who == ident))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A player's shot within `radius` of pos in the last `seconds` (strangers' "a fight" rule).
	bool RecentShot(vector pos, float radius, float seconds)
	{
		return ShotNear(pos, radius, seconds);
	}

	//------------------------------------------------------------------------------------------------
	bool IsOn()
	{
		return m_Settings && m_Settings.m_bEnabled;
	}

	//! The wanderers ask through "Need a hand?" (Missions > Errands > Wanderer asks).
	bool AsksOn() { return IsOn() && m_Settings.m_bWandererAsks; }
	RZ_NamedPeople GetPeople() { return m_People; }
	RZ_Faults GetFaults() { return m_Faults; }
	RZ_GameModeComponent GetGM() { return m_GM; }
	RZ_DutyGames GetGames() { return m_Games; }
	RZ_DutySquads GetSquads() { return m_Squads; }
	RZ_DutyScenes GetScenes() { return m_Scenes; }
	RZ_Strangers GetStrangers() { return m_Strangers; }
	float GetClock() { return m_fClock; }

	//------------------------------------------------------------------------------------------------
	//! After everything else is set up: the routines and cards, and the town log's season marks.
	void Setup()
	{
		RZ_ErrandRegistry.Build(m_aDefs);
		foreach (RZ_ErrandDef d : m_aDefs)
		{
			d.Define();
		}

		if (m_Restored)
		{
			if (m_Restored.ledgers)
			{
				foreach (RZ_ErrandLedger l : m_Restored.ledgers)
				{
					if (!l || l.ident.IsEmpty())
						continue;

					l.Ensure();
					m_mLedgers.Set(l.ident, l);
				}
			}

			m_People.Restore(m_Restored.houses);
			m_Locals.Restore(m_Restored.locals);
			m_PayKinds.Restore(m_Restored.deferred);
			if (m_Restored.counterKeys && m_Restored.counterValues)
			{
				for (int i = 0; i < m_Restored.counterKeys.Count() && i < m_Restored.counterValues.Count(); i++)
				{
					m_mCounters.Set(m_Restored.counterKeys[i], m_Restored.counterValues[i]);
				}
			}
		}

		m_Restored = null;
		RZ_WorldFacts facts = m_GM.GetFacts();
		if (facts)
			facts.Memory().PlaceSeasonMarks();

		int cards = 0;
		int duties = 0;
		foreach (RZ_ErrandDef def : m_aDefs)
		{
			if (def.m_bCard)
				cards++;
			else if (def.m_bDuty)
				duties++;
		}

		RZ_Log.Info(string.Format("Errands: %1, %2 routines, %3 duties (%4) and %5 cards, %6 players with a ledger.", OnOff(IsOn()), m_aDefs.Count() - cards - duties, duties, OnOff(m_Settings.m_bDuties), cards, m_mLedgers.Count()));
	}

	//------------------------------------------------------------------------------------------------
	protected static string OnOff(bool on)
	{
		if (on)
			return "on";

		return "off";
	}

	// ================================================================ Save
	//------------------------------------------------------------------------------------------------
	RZ_ErrandsSave Save()
	{
		RZ_ErrandsSave data = new RZ_ErrandsSave();
		foreach (string ident, RZ_ErrandLedger l : m_mLedgers)
		{
			data.ledgers.Insert(l);
		}

		m_People.Save(data.houses);
		m_Locals.Save(data.locals);
		m_PayKinds.Save(data.deferred);
		foreach (string key, int value : m_mCounters)
		{
			data.counterKeys.Insert(key);
			data.counterValues.Insert(value);
		}

		return data;
	}

	//------------------------------------------------------------------------------------------------
	void Restore(RZ_ErrandsSave data)
	{
		m_Restored = data;
	}

	//------------------------------------------------------------------------------------------------
	RZ_ErrandLedger Ledger(string ident)
	{
		RZ_ErrandLedger l;
		if (m_mLedgers.Find(ident, l))
			return l;

		l = new RZ_ErrandLedger();
		l.ident = ident;
		l.Ensure();
		m_mLedgers.Set(ident, l);
		return l;
	}

	//------------------------------------------------------------------------------------------------
	//! A saved server counter (Fresh Chalk's reading day: "read.<tribe>").
	void SetCounter(string key, int value)
	{
		m_mCounters.Set(key, value);
	}

	//------------------------------------------------------------------------------------------------
	int Counter(string key)
	{
		int v;
		m_mCounters.Find(key, v);
		return v;
	}

	// ================================================================ Hooks
	//------------------------------------------------------------------------------------------------
	//! From the gunfire hook: a shot spoils a quiet way near it, and is remembered a minute for the
	//! "no shot within 60 m" rule.
	void OnPlayerShot(int playerId, vector pos)
	{
		m_aShots.Insert(pos);
		m_aShotAt.Insert(m_fClock);
		while (m_aShots.Count() > 24)
		{
			m_aShots.RemoveOrdered(0);
			m_aShotAt.RemoveOrdered(0);
		}

		foreach (RZ_Errand e : m_aErrands)
		{
			if (e.m_eState == RZ_EErrandState.HELD && !e.m_bNoisy && RZ_Util.DistXZ(e.m_vPlace, pos) <= 100)
				e.m_bNoisy = true;
		}

		if (m_Strangers && IsOn())
			m_Strangers.OnShot(playerId, pos);
	}

	//------------------------------------------------------------------------------------------------
	protected bool ShotNear(vector pos, float radius, float seconds)
	{
		for (int i = m_aShots.Count() - 1; i >= 0; i--)
		{
			if (m_fClock - m_aShotAt[i] > seconds)
				break;

			if (RZ_Util.DistXZ(m_aShots[i], pos) <= radius)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A player died: their errands lapse quietly ("Heard what happened. We'll manage.").
	void OnPlayerDied(int playerId)
	{
		foreach (RZ_Errand e : m_aErrands)
		{
			if (e.m_iAsker == playerId && e.m_eState != RZ_EErrandState.DONE)
				End(e, "died");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A player left: a held errand waits 20 minutes for them.
	void OnDisconnect(int playerId)
	{
		foreach (RZ_Errand e : m_aErrands)
		{
			if (e.m_iAsker == playerId && e.m_eState == RZ_EErrandState.HELD)
				e.m_fOfflineSince = m_fClock;
			else if (e.m_iAsker == playerId)
				End(e, "left");
		}
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		m_fClock += dt;
		m_fSlow += dt;
		bool slow = m_fSlow >= SLOW_TICK;
		if (slow)
		{
			m_fSlow = 0;
			// The world memory's town reasons are written whether errands are on or not
			RZ_WorldWriters.WatchTowns(m_GM);
			// The reasons read from other systems' facts (faults, holdings, leaders)
			RZ_ReasonPosts.Watch(m_GM, m_Faults);
			// The lie store's events (visits, road news, the rumour rounds), errands on or not
			RZ_WorldFacts lieFacts = m_GM.GetFacts();
			if (lieFacts)
				lieFacts.Memory().Lies().Tick(m_GM, ids, players);

			// A story job's pay for someone who wasn't online when it ended
			PayPending(ids);
		}

		if (!IsOn())
			return;

		m_Squads.Tick(dt, ids, players);
		TickSteps(dt, ids, players);
		m_Strangers.TickSteps(dt, ids, players);
		m_More.Tick(dt, ids, players);
		if (slow)
		{
			m_Faults.Tick(players, m_Settings.m_fFaultChance);
			WatchHouses();
			m_Locals.Watch();
			Keep(ids, players);
			m_Strangers.Tick(SLOW_TICK, ids, players, m_Settings.m_bStrangers);
			CallOuts(ids, players);
			RadioAsks(ids, players);
			ExpireTalks(ids, players);
			MarkLines(ids, players);
		}

		m_fTelemetry += dt;
		if (m_fTelemetry >= m_Settings.m_fTelemetryMinutes * 60)
		{
			m_fTelemetry = 0;
			LogTelemetry();
		}

		for (int i = m_aErrands.Count() - 1; i >= 0; i--)
		{
			if (m_aErrands[i].m_eState == RZ_EErrandState.DONE)
				m_aErrands.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Steps, on the 5 s tick: being at the place for long enough (unseen, before the hour), items
	//! handed over there, the helpers who came, and coming back to the giver.
	protected void TickSteps(float dt, array<int> ids, array<IEntity> players)
	{
		foreach (RZ_Errand e : m_aErrands)
		{
			if (e.m_eState != RZ_EErrandState.HELD)
				continue;

			int idx = ids.Find(e.m_iAsker);
			if (idx < 0)
				continue;

			IEntity me = players[idx];
			vector at = me.GetOrigin();
			NoteHelpers(e, dt, ids, players, at);
			RZ_ErrandWay w = e.GetWay();
			if (!w)
				continue;

			if (!e.m_bStepDone)
			{
				if (w.m_fBefore >= 0 && RZ_GameTime.IsCalibrated() && PastHour(w.m_fBefore))
				{
					Say(e, Pick(e.m_Def.m_aOver), true);
					Count(e.m_Def.m_sFamily).lapses++;
					End(e, "late");
					continue;
				}

				// A first place before the step's own (RZ_ErrandWay.First): there long enough, then on
				if (!FirstWorked(e, w, me, dt))
					continue;

				if (!e.m_Def.StepCounts(this, e, me))
					continue;

				// Work at a visible thing: done when its hold is done (RZ_WorkSites); else stay there
				if (!StepWorked(e, w, me, dt))
					continue;

				if (w.m_bGiveThere && w.m_iItems > 0)
				{
					if (TakeItems(me, w.m_eItem, w.m_iItems) < w.m_iItems)
					{
						if (!e.m_bTold)
						{
							e.m_bTold = true;
							m_GM.NotifyPlayer(e.m_iAsker, "Errand: " + e.Fill(e.m_Def.m_sTitle), "You haven't got what this needs on you. " + ItemNeed(w.m_eItem, w.m_iItems) + ".");
						}

						continue;
					}
				}

				e.m_bStepDone = true;
				e.m_bTold = false;
				RemoveProp(e);
				if (!e.m_Def.Still(this, e))
				{
					Overtake(e);
					continue;
				}

				// An opening's last step: done where it's done, nobody to go back to
				if (w.m_bEndsThere)
				{
					Resolve(e, w);
					continue;
				}

				if (w.m_bCarry)
				{
					m_PayKinds.GiveCarried(e, me);
					m_GM.NotifyPlayer(e.m_iAsker, "Errand: " + e.Fill(e.m_Def.m_sTitle), e.Fill("You have it. Carry it back to {giver} at {here} yourself: nobody else can hand it in."));
					continue;
				}

				m_GM.NotifyPlayer(e.m_iAsker, "Errand: " + e.Fill(e.m_Def.m_sTitle), e.Fill("That's done. Go back to {giver} at {here}."));
				continue;
			}

			// Done: back to the giver (a wanderer: where they stand now). With the talk screen the hand-in
			// is a reply in their talk ("[Errand] It's done: ...", Ask: HandIn); without it, or for a
			// handed errand (an opening's step, its owner's), being back with them is enough
			bool byTalk = RZ_Dialogue.IsOn() && !e.m_Def.m_bHanded;
			vector giverAt = GiverAt(e);

			// A carried item: only the asker or a helper hands it in; anyone else has spoiled it
			if (w.m_bCarry && e.m_Carried)
			{
				int carrier = RZ_ErrandsPay.Carrier(e);
				if (m_PayKinds.Stolen(e, carrier))
				{
					End(e, "carried item taken");
					continue;
				}

				IEntity carrierEnt = GetGame().GetPlayerManager().GetPlayerControlledEntity(carrier);
				if (!byTalk && carrier > 0 && RZ_Arrive.AtSpot(carrierEnt, giverAt, RETURN_RANGE))
					Resolve(e, w);

				continue;
			}

			if (w.m_bCarry && e.m_bStepDone && !e.m_Carried && m_PayKinds.HadCarried(e))
			{
				Say(e, Pick(e.m_Def.m_aOver), true);
				End(e, "carried item lost");
				continue;
			}

			if (!byTalk && RZ_Arrive.AtSpot(me, giverAt, RETURN_RANGE))
				Resolve(e, w);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Handing a done errand in, in the giver's talk (their "[Errand] It's done: ..." reply). Something
	//! carried back has to be with you, or with a helper standing by: it's taken off whoever has it
	//! (Resolve, ConsumeCarried).
	protected void HandIn(RZ_Errand e, RZ_ErrandWay w, int playerId)
	{
		if (w.m_bCarry && e.m_Carried)
		{
			int carrier = RZ_ErrandsPay.Carrier(e);
			IEntity carrierEnt = GetGame().GetPlayerManager().GetPlayerControlledEntity(carrier);
			vector giverAt = e.m_vGiver;
			if (e.m_iWanderer >= 0 && m_GM.GetWanderers())
				m_GM.GetWanderers().GetPosition(e.m_iWanderer, giverAt);

			bool near = carrier > 0 && (carrier == playerId || e.m_aHelpers.Contains(carrier)) && RZ_Arrive.AtSpot(carrierEnt, giverAt, RETURN_RANGE);
			if (!near)
			{
				Say(e, "Where is it, then? Bring it here with you.", false);
				return;
			}
		}

		Resolve(e, w);
	}

	//------------------------------------------------------------------------------------------------
	//! Game time past `hour` today, counted from noon (so 20.5 is "this evening").
	protected static bool PastHour(float hour)
	{
		float now = RZ_GameTime.GetHour();
		if (now < 0)
			return false;

		if (now < 12 && hour >= 12)
			return true;

		return now >= hour;
	}

	//------------------------------------------------------------------------------------------------
	//! Group members within 50 m of the asker for 2 minutes, or there when the step was done, are
	//! helpers (three at most).
	protected void NoteHelpers(RZ_Errand e, float dt, array<int> ids, array<IEntity> players, vector at)
	{
		foreach (int i, IEntity p : players)
		{
			int other = ids[i];
			if (other == e.m_iAsker || e.m_aHelpers.Contains(other) || RZ_Util.DistXZ(p.GetOrigin(), at) > HELPER_RANGE)
				continue;

			if (!RZ_Util.SameGroup(e.m_iAsker, other))
				continue;

			float together;
			e.m_mWith.Find(other, together);
			together += dt;
			e.m_mWith.Set(other, together);
			if ((together >= HELPER_SECONDS || e.m_bStepDone) && e.m_aHelpers.Count() < MAX_HELPERS)
				e.m_aHelpers.Insert(other);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A safehouse whose people went up took someone in (The Newcomer's fact).
	protected void WatchHouses()
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses)
			return;

		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			if (!h)
				continue;

			int before;
			if (m_mHousePop.Find(i, before) && h.m_iPopulation > before)
				m_mTookIn.Set(i, RZ_SeasonClock.Day());

			// Its locals are among the people a house loses, as often as their share
			if (m_mHousePop.Find(i, before) && h.m_iPopulation < before)
				m_Locals.OnHouseLost(i, before - h.m_iPopulation, before);

			m_mHousePop.Set(i, h.m_iPopulation);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The house took someone in this season day.
	bool TookSomeoneIn(int house)
	{
		int day;
		if (!m_mTookIn.Find(house, day))
			return false;

		return day == RZ_SeasonClock.Day();
	}

	//------------------------------------------------------------------------------------------------
	//! A capped morale change for a faction, in the contracts and errands lane of the season budget;
	//! anything clipped becomes words only.
	void Morale(int f, int delta, string why)
	{
		RZ_FactionLife life = m_GM.GetFactionLife();
		RZ_Shared shared = m_GM.GetShared();
		if (!life || !RZ_Factions.IsValid(f))
			return;

		float allowed = delta;
		if (shared)
			allowed = shared.GetBudget().TakeMorale(RZ_EBudgetLane.CONTRACTS, f, delta, "", "errand " + why);

		if (allowed != 0)
			life.AddMorale(f, allowed);
	}

	//------------------------------------------------------------------------------------------------
	//! Every 30 s: the guards (still true, nobody's claim on the subject, the giver alive), the
	//! lapse and the offers that ran out.
	protected void Keep(array<int> ids, array<IEntity> players)
	{
		RZ_WorldFacts facts = m_GM.GetFacts();
		foreach (RZ_Errand e : m_aErrands)
		{
			if (e.m_eState == RZ_EErrandState.DONE)
				continue;

			if (e.m_eState == RZ_EErrandState.OFFERED)
			{
				if (m_fClock - e.m_fOfferedAt > OFFER_MINUTES * 60)
				{
					Count(e.m_Def.m_sFamily).declines++;
					End(e, "offer ran out");
				}

				continue;
			}

			float lapse = LapseSeconds(e);
			if (e.m_fOfflineSince >= 0)
			{
				if (ids.Contains(e.m_iAsker))
					e.m_fOfflineSince = -1;
				else if (m_fClock - e.m_fOfflineSince > 1200)
					lapse = 0;
			}

			if (m_fClock - e.m_fTakenAt > lapse)
			{
				// Its end card says so (RememberEnd: "Errand failed: ...", "{giver} stopped waiting.")
				Count(e.m_Def.m_sFamily).lapses++;
				End(e, "lapsed");
				continue;
			}

			if ((!e.m_Def.m_bHanded && !m_People.Get(e.m_sGiver)) || !e.m_Def.Still(this, e) || Overtaken(e, facts))
				Overtake(e);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A mission's claim on the errand's subject overtakes any errand; a questline's or an arc's long
	//! hold overtakes only a card whose premise lists the subject (people cook while the council sits).
	protected bool Overtaken(RZ_Errand e, RZ_WorldFacts facts)
	{
		if (!facts || e.m_sSubject.IsEmpty())
			return false;

		string holder = facts.ClaimedBy(e.m_sSubject);
		if (holder.IsEmpty())
			return false;

		if (holder.IndexOf("mission:") == 0)
			return true;

		if (!e.m_Def.m_bCard)
			return false;

		array<string> premise = {};
		e.m_Def.Premise(e, premise);
		return premise.Contains(e.m_sSubject);
	}

	//------------------------------------------------------------------------------------------------
	//! The world changed: the errand ends, no pay and no penalty, and the giver says so.
	protected void Overtake(RZ_Errand e)
	{
		Count(e.m_Def.m_sFamily).overtakes++;
		Say(e, Pick(e.m_Def.m_aOver), true);
		RZ_Log.Info(string.Format("Errands: %1 for %2 overtaken", e.m_Def.m_sKey, RZ_Util.PlayerName(e.m_iAsker)));
		End(e, "overtaken");
	}

	//------------------------------------------------------------------------------------------------
	//! The way's first place (RZ_ErrandWay.First) is done: true at once without one. Stay there its
	//! seconds; then the step's own place is the objective, and the asker is told.
	protected bool FirstWorked(RZ_Errand e, RZ_ErrandWay w, IEntity me, float dt)
	{
		if (e.m_bFirstDone || !w.HasFirst() || e.m_vFirst == vector.Zero)
			return true;

		if (!w.IsAtPlace(me, e.m_vFirst))
			return false;

		e.m_fFirstProgress += dt;
		if (e.m_fFirstProgress < w.m_fFirstSeconds)
			return false;

		e.m_bFirstDone = true;
		m_GM.NotifyPlayer(e.m_iAsker, "Errand: " + e.Fill(e.m_Def.m_sTitle), e.Fill(RZ_QuestTracker.WithoutBack(w.StepText())));
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The step's own rule, on the 5 s tick: work at a work site is done when its hold is done (the
	//! server counted the asker at it); otherwise, being at the place long enough (unseen, for an
	//! unseen way).
	protected bool StepWorked(RZ_Errand e, RZ_ErrandWay w, IEntity me, float dt)
	{
		RZ_WorkSites sites = RZ_WorkSites.Get();
		RZ_Carries carries = RZ_Carries.Get();
		if (carries && carries.Has(WorkKey(e)))
			return carries.IsDone(WorkKey(e));

		if (sites && sites.Has(WorkKey(e)))
			return sites.IsDone(WorkKey(e)) && (!w.m_bGiveThere || w.IsAtPlace(me, e.m_vPlace));

		if (!w.IsAtPlace(me, e.m_vPlace))
			return false;

		if (w.m_bUnseen && !RZ_Stealth.IsHidden(e.m_iAsker))
		{
			if (!e.m_bTold)
			{
				e.m_bTold = true;
				m_GM.NotifyPlayer(e.m_iAsker, "Errand: " + e.Fill(e.m_Def.m_sTitle), "They'd see you like that. Get low, and keep out of the light.");
			}

			return false;
		}

		e.m_fProgress += dt;
		return e.m_fProgress >= w.m_fSeconds;
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second (the game mode's second tick): the sorting duties read what the player's hands did.
	void SecondTick(array<int> ids, array<IEntity> players)
	{
		if (!IsOn())
			return;

		m_Games.SecondTick(ids, players);
		m_Scenes.SecondTick(ids, players);
	}

	//------------------------------------------------------------------------------------------------
	//! The errand's work site key (one step at a time).
	static string WorkKey(RZ_Errand e)
	{
		return "errand." + e.m_iId.ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! A way done with your hands (RZ_ErrandWay.Work): its work site at the place, for the asker
	//! only, with the way's prop as its before look when it has one. False: work sites are off, and
	//! the step keeps the old rule.
	protected bool OpenWork(RZ_Errand e, RZ_ErrandWay w)
	{
		if (!w.IsWork() || e.m_vPlace == vector.Zero)
			return false;

		// Carried for real: a pile of the things, and where they go
		if (w.m_iCarry > 0 && OpenCarry(e, w))
			return true;

		RZ_WorkSites sites = RZ_WorkSites.Get();
		vector at = e.m_vPlace;
		// At the giver's place or its gate: beside it, not on whoever stands there
		if (w.m_ePlace == RZ_EErrandPlace.HERE || w.m_ePlace == RZ_EErrandPlace.GATE)
			at = RZ_WorkSites.Spot(e.m_vPlace, 3, 6);
		else if (w.m_eWork == RZ_EWork.FILL)
			at = RZ_WorkSites.Spot(e.m_vPlace, 1.5, 3);

		// Work the words put indoors (a sick man, a bench, a search of a shop): on a ground floor near
		// the place, else beside it in the open as before
		bool inside;
		if (!w.m_sInside.IsEmpty())
		{
			string kind = w.m_sInside;
			if (kind == "any")
				kind = "";

			vector room;
			if (RZ_WorkSites.Indoors(e.m_vPlace, INSIDE_RADIUS, kind, room))
			{
				at = room;
				inside = true;
			}
		}

		// A place its picker put on a floor (the openings' search, inside the building it names)
		if (!inside && at[1] > RZ_Util.GroundPos(at)[1] + 0.3)
			inside = true;

		// Boarding up is done at a real door of the place (the boards go in its doorway): never a
		// door of our own in the open. No real door near: the boards are left out (DoorPropFits)
		float yaw = -1;
		if (w.m_eWork == RZ_EWork.BOARDS)
		{
			IEntity door;
			vector centre;
			vector normal;
			if (RZ_DoorGeometry.NearestOuterDoor(e.m_vPlace, 0, BOARD_DOOR_REACH, false, door, centre, normal))
			{
				at = RZ_Util.GroundPos(centre + normal * 0.1);
				yaw = Math.Atan2(normal[0], normal[2]) * Math.RAD2DEG;
				if (yaw < 0)
					yaw += 360;
			}
		}

		ResourceName before = w.m_sLook;
		if (before.IsEmpty() && w.m_eProp != RZ_EErrandProp.NONE)
			before = m_Settings.Props().For(w.m_eProp);

		// Work at something that isn't there (HANDS has no look of its own): the words name a thing
		// nothing stands for, which the audit should catch (docs/ERRANDS.md, "The work")
		if (before.IsEmpty() && w.m_eWork == RZ_EWork.HANDS && !w.m_bPerson)
			RZ_Log.Info(string.Format("Errands: %1 '%2' has nothing standing at its work site (no look, no person)", e.m_Def.m_sKey, w.m_sWorkLabel));

		string key = WorkKey(e);
		float hold = w.m_fWorkSeconds;
		if (hold <= 0)
			hold = RZ_WorkSites.HoldFor(w.m_fSeconds);

		if (sites.Open(key, w.m_eWork, w.m_sWorkLabel, hold, at, e.m_iAsker, before, "", yaw, inside) < 0)
			return false;

		if (w.m_bPerson)
			sites.SetPerson(key, PersonPrefab(), w.m_bPersonLies);

		if (w.m_bUnseen)
			sites.SetUnseen(key);

		e.m_vPlace = at;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A way carried for real (RZ_ErrandWay.Carry): the pile and where it goes. Into the place's
	//! building: the pile in the open beside the place, the stack inside on its ground floor (else
	//! by the place's door, said so). Out of it: the pile inside, the stack in the open beside it.
	//! False: carrying is off, or it won't open (the plain work site instead).
	protected bool OpenCarry(RZ_Errand e, RZ_ErrandWay w)
	{
		RZ_Carries carries = RZ_Carries.Get();
		if (!carries || !RZ_Carries.Available())
			return false;

		string kind = w.m_sInside;
		if (kind == "any")
			kind = "";

		vector room;
		bool indoors = RZ_WorkSites.Indoors(e.m_vPlace, INSIDE_RADIUS, kind, room);
		vector open = RZ_WorkSites.Spot(e.m_vPlace, 6, 12);
		vector pick = open;
		vector drop = room;
		bool pickIn = false;
		bool dropIn = indoors;
		string pickName = "outside";
		string dropName = "indoors, on the ground floor";
		if (!w.m_bCarryIn)
		{
			pick = room;
			pickIn = indoors;
			pickName = "inside, on the ground floor";
			drop = open;
			dropIn = false;
			dropName = "outside, by the door";
			if (!indoors)
			{
				pick = RZ_WorkSites.Spot(e.m_vPlace, 2, 4);
				pickName = "by the door";
			}
		}
		else if (!indoors)
		{
			drop = RZ_WorkSites.Spot(e.m_vPlace, 2, 4);
			dropName = "by the door, under cover if you can";
		}

		if (!indoors)
			RZ_Log.Info(string.Format("Errands: %1 carries %2, but no building within %3 m of %4: carried to its door", e.m_Def.m_sKey, w.m_sCarryThings, INSIDE_RADIUS, RZ_SettlementScanner.GridReference(e.m_vPlace)));

		if (!carries.Open(WorkKey(e), e.m_iAsker, w.m_iCarry, w.m_sCarryProp, w.m_sCarryThing, w.m_sCarryThings, pick, pickIn, pickName, drop, dropIn, dropName, w.m_bCarryStack))
			return false;

		e.m_vPlace = pick;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Who stands at a work site the words put someone at: Props > Duty person, else Puzzles > Civilian.
	protected ResourceName PersonPrefab()
	{
		ResourceName prefab = m_Settings.Props().m_sDutyPerson;
		if (!prefab.IsEmpty())
			return prefab;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return "";

		return gm.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);
	}

	//------------------------------------------------------------------------------------------------
	//! The way's prop (Missions > Errands > Props) at its place while the step is open: a marks-pool
	//! prop, up only while a player is near (RZ_Marks), gone when the step is done or the errand ends.
	//! A way done with your hands gets its work site instead (OpenWork).
	protected void PlaceProp(RZ_Errand e, RZ_ErrandWay w)
	{
		if (OpenWork(e, w))
			return;

		if (w.m_eProp == RZ_EErrandProp.NONE || w.m_ePlace == RZ_EErrandPlace.NONE || w.m_ePlace == RZ_EErrandPlace.HERE)
			return;

		ResourceName prefab = m_Settings.Props().For(w.m_eProp);
		RZ_Marks marks = RZ_Marks.Get();
		if (prefab.IsEmpty() || !marks)
			return;

		if (marks.Place(RZ_EMarkPool.PROPS, PropKey(e), RZ_Util.GroundPos(e.m_vPlace), prefab, "", 1, -1, false, false))
			RZ_Log.Info(string.Format("Errands: %1 prop %2 at %3", e.m_Def.m_sKey, typename.EnumToString(RZ_EErrandProp, w.m_eProp), RZ_SettlementScanner.GridReference(e.m_vPlace)));
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveProp(RZ_Errand e)
	{
		RZ_Marks marks = RZ_Marks.Get();
		if (marks)
			marks.Remove(PropKey(e));
	}

	//------------------------------------------------------------------------------------------------
	protected static string PropKey(RZ_Errand e)
	{
		return "errand." + e.m_iId.ToString();
	}

	//------------------------------------------------------------------------------------------------
	protected void End(RZ_Errand e, string why)
	{
		if (e.m_eState == RZ_EErrandState.DONE)
			return;

		// A held errand that ends any other way than done goes into its holder's past jobs
		if (e.m_eState == RZ_EErrandState.HELD && why != "done")
			RememberEnd(e, why);

		e.m_eState = RZ_EErrandState.DONE;
		if (e.m_Def)
			e.m_Def.Ended(this, e, why);

		m_Games.Drop(e.m_iId);
		m_Squads.Drop(e.m_iId);
		m_Scenes.Drop(e.m_iId);

		RemoveProp(e);
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (sites)
			sites.Close(WorkKey(e));

		RZ_Carries carries = RZ_Carries.Get();
		if (carries)
			carries.Close(WorkKey(e));

		if (e.m_Carried)
		{
			RZ_Util.DeleteUnit(e.m_Carried);
			e.m_Carried = null;
		}

		RZ_WorldFacts facts = m_GM.GetFacts();
		if (facts)
			facts.SoftReleaseAll(e.Owner());

		ReleaseTalk(e.m_sGiver);
		if (m_Settings.m_bDebugLog)
			RZ_Log.Info(string.Format("Errands: %1 (%2) for %3 ended: %4", e.m_Def.m_sKey, e.m_iId, RZ_Util.PlayerName(e.m_iAsker), why));
	}

	//------------------------------------------------------------------------------------------------
	//! The notebook's past jobs (Missions/RZ_JobHistory.c): a held errand that ended.
	protected void Remember(RZ_Errand e, int result, string line)
	{
		if (!e || !e.m_Def)
			return;

		string by = e.m_sGiverName;
		if (by.IsEmpty())
			by = e.m_sGiverTitle;

		RZ_JobHistory.AddFor(m_GM, e.m_iAsker, RZ_EMissionKindUI.ERRAND, e.m_iFaction, e.Fill(e.m_Def.m_sTitle), by, e.GetText("here"), result, line);
	}

	//------------------------------------------------------------------------------------------------
	//! Why a held errand ended, as its past job: failed when it was yours to lose (late, lapsed, the
	//! thing you carried gone, you died), abandoned when you left it or the world moved on.
	protected void RememberEnd(RZ_Errand e, string why)
	{
		int result = RZ_EJobResult.ABANDONED;
		string line = RZ_Util.Capitalize(why) + ".";
		if (why == "late" || why == "lapsed")
		{
			result = RZ_EJobResult.FAILED;
			line = e.Fill("{giver} stopped waiting.");
		}
		else if (why == "died")
		{
			result = RZ_EJobResult.FAILED;
			line = "You died before it was done.";
		}
		else if (why == "carried item taken" || why == "carried item lost")
		{
			result = RZ_EJobResult.FAILED;
			line = "What you were carrying never got there.";
		}
		else if (why == "overtaken")
		{
			line = "It stopped being true before you got to it.";
		}
		else if (why == "left")
		{
			line = "You left before it was done.";
		}
		else if (why == "sold at the fence" || why == "refused at the fence")
		{
			line = "You took it to the fence instead.";
		}

		Remember(e, result, line);

		// The end card (RZ_JobCards): "Errand failed: <title>" or "Errand abandoned: <title>"
		string how = "abandoned";
		if (result == RZ_EJobResult.FAILED)
			how = "failed";

		if (GetGame().GetPlayerManager().GetPlayerControlledEntity(e.m_iAsker))
			RZ_JobCards.Ended(m_GM, e.m_iAsker, "Errand", how, e.Fill(e.m_Def.m_sTitle), line);
	}

	//------------------------------------------------------------------------------------------------
	//! Who an errand is from, for its start card: "Mara, the cook at Morton".
	protected string GiverWords(RZ_Errand e)
	{
		string who = e.m_sGiverName;
		if (who.IsEmpty())
			who = e.m_sGiverTitle;

		string here = e.GetText("here");
		if (!here.IsEmpty() && !who.Contains(here))
			who += " at " + here;

		return who;
	}

	// ================================================================ Talking to a giver
	//------------------------------------------------------------------------------------------------
	//! From RZ_PuzzleActionHelper.RouteChoice, before the giver's own handler: a choice on a person
	//! whose slots are an offer's ways right now. True if handled.
	bool HandleEngagedChoice(IEntity owner, int playerId, int option, IEntity user)
	{
		if (!owner)
			return false;

		RZ_ErrandTalk found;
		foreach (string key, RZ_ErrandTalk talk : m_mTalks)
		{
			if (talk.m_Owner == owner)
			{
				found = talk;
				break;
			}
		}

		if (!found)
			return false;

		if (found.m_iPlayer != playerId)
		{
			RZ_NamedPerson busy = m_People.Get(found.m_sGiver);
			if (busy)
				m_GM.Say(playerId, busy.m_sTitle, busy.m_iFaction, "Give us a minute. I'm in the middle of something with someone else.");

			return true;
		}

		PickWay(found, option, user);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A choice on a holding's named person (RZ_Territory.TAG): slot 1, "Anything need doing?", is
	//! the errands'. True if handled.
	bool HandleHoldingChoice(int playerId, int code, int option, IEntity owner, IEntity user)
	{
		if (!IsOn() || option != 1)
			return false;

		Ask(playerId, RZ_NamedPeople.HoldingKey(code / 10, code % 10), owner, user);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A safehouse's named person (RZ_NamedPeople.TAG_HOUSE): Talk greets you in their own voice,
	//! slot 0 is the house's news, slot 1 is "Anything need doing?".
	void HandleHouseTalk(int playerId, int code, int option, IEntity owner, IEntity user)
	{
		int house = code / 10;
		RZ_NamedPerson p = m_People.FromHouse(house, code % 10);
		if (!p)
			return;

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses && houses.IsExiled(playerId))
		{
			// An owed exile's knock: the council head answers through the door (RZ_Parley)
			RZ_Checkpoints posts = m_GM.GetCheckpoints();
			if (posts && posts.GetParley().Knock(playerId, user, p.m_vPos, p.m_sPlace))
				return;

			m_GM.Say(playerId, p.m_sTitle, RZ_EFaction.SURVIVORS, "No. You were put out of this house, and you know why. Not near the children. The road's that way.");
			return;
		}

		if (option == 1)
		{
			Ask(playerId, p.m_sKey, owner, user);
			return;
		}

		if (option == 0)
		{
			if (houses)
				houses.HandleOccupantTalk(playerId, house);

			return;
		}

		// Talk: the greeting in their own voice, and what's wrong at the house this shift. On the talk
		// screen only what's wrong is said at hello (no filler greeting)
		string text = "";
		if (!RZ_Dialogue.Quiet(m_GM, playerId))
		{
			RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
			int phase = RZ_Territory.RoutinePhase(RZ_GameTime.GetHour());
			text = RZ_FactionVoice.PersonLine(rng, RZ_EFaction.SURVIVORS, p.m_sRole, phase, p.m_sPlace, playerId, p.m_vPos);
		}

		string fault = m_Faults.HouseLine(house);
		if (!fault.IsEmpty())
			text += "\n" + fault;

		text = text.Trim();
		if (!text.IsEmpty())
			m_GM.NotifyPlayer(playerId, p.m_sTitle, text);
		RZ_CampLife camp = m_GM.GetCampLife();
		if (camp)
			camp.HoldPersonNear(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! "Anything need doing?" on a named person: the errand they hold for you, the offer they made
	//! you, or a new one that's true now. Asked errands skip the gaps; safety and the giver's will
	//! still apply, and the giver says the real reason in their own voice.
	//! A story job failed at `at` (RZ_StoryKit.Fallout): its fallout errand `card` waits there for the
	//! participants, then anyone. One per job; a new one at the same card and place replaces it.
	void Fallout(string card, vector at, array<int> participants, int cap, string source, string who)
	{
		RZ_Fallout f = new RZ_Fallout();
		f.card = card;
		f.at = at;
		f.who = who;
		f.source = source;
		f.cap = Math.Max(0, cap);
		f.privateUntil = m_fClock + FALLOUT_PRIVATE;
		f.until = m_fClock + FALLOUT_LIFE;
		if (participants)
		{
			foreach (int playerId : participants)
			{
				string ident = m_GM.GetProfile(playerId).m_sIdentity;
				if (!ident.IsEmpty() && !f.idents.Contains(ident))
					f.idents.Insert(ident);
			}
		}

		for (int i = m_aFallout.Count() - 1; i >= 0; i--)
		{
			if (m_aFallout[i].card == card && RZ_Util.DistXZ(m_aFallout[i].at, at) < 50)
				m_aFallout.Remove(i);
		}

		m_aFallout.Insert(f);
		RZ_Log.Info(string.Format("Errands: fallout %1 at %2 from %3 (cap %4 Favor, %5 participant(s) first)", card,
			RZ_SettlementScanner.GridReference(at), source, f.cap, f.idents.Count()));
	}

	//------------------------------------------------------------------------------------------------
	//! A waiting fallout of `card` that a giver at `giver` can deal to this player now; null if none.
	RZ_Fallout FalloutFor(string card, vector giver, int playerId)
	{
		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		for (int i = m_aFallout.Count() - 1; i >= 0; i--)
		{
			RZ_Fallout f = m_aFallout[i];
			if (m_fClock > f.until)
			{
				m_aFallout.Remove(i);
				continue;
			}

			if (f.card != card || RZ_Util.DistXZ(f.at, giver) > FALLOUT_RANGE)
				continue;

			if (m_fClock < f.privateUntil && !f.idents.Contains(ident))
				continue;

			return f;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! A fallout errand was done: it's gone.
	protected void EndFallout(RZ_Errand e)
	{
		if (!e.m_bFallout)
			return;

		for (int i = m_aFallout.Count() - 1; i >= 0; i--)
		{
			if (m_aFallout[i].card == e.m_Def.m_sKey && RZ_Util.DistXZ(m_aFallout[i].at, e.m_vSubject) < 50)
				m_aFallout.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Ask(int playerId, string giverKey, IEntity owner, IEntity user)
	{
		RZ_NamedPerson p = m_People.Get(giverKey);
		if (!p || !user)
			return;

		if (!IsOn())
		{
			m_GM.Say(playerId, p.m_sTitle, p.m_iFaction, "Nothing today. We're managing, just about.");
			return;
		}

		// A held one from this giver: done and back, or still to do
		RZ_Errand mine = FindFor(playerId, giverKey);
		if (mine && mine.m_eState == RZ_EErrandState.HELD)
		{
			RZ_ErrandWay w = mine.GetWay();
			if (mine.m_bStepDone && w)
				HandIn(mine, w, playerId);
			else if (w)
				m_GM.NotifyPlayer(playerId, "Errand: " + mine.Fill(mine.m_Def.m_sTitle), mine.Fill(w.StepText()));

			return;
		}

		string refusal = Refusal(playerId, p, user);
		if (!refusal.IsEmpty())
		{
			m_GM.Say(playerId, p.m_sTitle, p.m_iFaction, refusal);
			return;
		}

		if (!mine)
		{
			string why;
			mine = Deal(playerId, p, user.GetOrigin(), false, why);
			m_mLastWhy.Set(playerId, why);
			if (!mine)
			{
				string noWork = NothingWhy(why, playerId, p);
				if (noWork == WHY_NOTHING)
					m_GM.Say(playerId, p.m_sTitle, p.m_iFaction, NothingLine(p));
				else
					m_GM.Say(playerId, p.m_sTitle, p.m_iFaction, string.Format("%1.", noWork));

				return;
			}
		}

		ShowOffer(mine, owner);
	}

	// Why a deal dealt nothing (Deal's reasons that are said to the player, NothingWhy)
	protected static const string NO_GAP = "on the duty gap";
	protected static const string NO_TAKEN = "every chore here is taken";
	protected static const string NO_MINE = "one chore here at a time";
	static const string WHY_NOTHING = "Nothing today. Ask again tomorrow";

	//------------------------------------------------------------------------------------------------
	//! Why a giver has nothing for you now, short, for the locked reply: the stated reason among what
	//! Deal said (`dealWhy`), the first that applies. With duties on, "nothing today" is left for a
	//! wanderer.
	protected string NothingWhy(string dealWhy, int playerId, RZ_NamedPerson p)
	{
		if (dealWhy.Contains("the server holds"))
			return "Everyone has work in hand. Ask again shortly";

		// The Watched get the amends work when there is some, and no duties
		if (RZ_Factions.IsValid(p.m_iFaction) && m_GM.GetTier(playerId, p.m_iFaction) == RZ_ETier.WATCHED)
			return string.Format("Nothing while you're Watched with %1, except to make amends", RZ_Factions.GetShortName(p.m_iFaction));

		if (dealWhy.Contains(NO_MINE))
			return "You've a chore of theirs already. Finish that one";

		if (dealWhy.Contains(NO_GAP))
			return string.Format("You've done your shift. Ask again in a while (%1 min at most)", RZ_Util.RoundToInt(m_Settings.m_fDutyGapMinutes));

		if (dealWhy.Contains(NO_TAKEN))
			return "Every chore here is taken. Ask again when one is done";

		return WHY_NOTHING;
	}

	//------------------------------------------------------------------------------------------------
	//! This person's slots are an errand's ways right now (an offer waiting on your answer): the talk
	//! screen says them as what you'd agree to (RZ_TalkWords.Agree).
	bool OffersOn(IEntity owner)
	{
		if (!owner)
			return false;

		foreach (string giver, RZ_ErrandTalk talk : m_mTalks)
		{
			if (talk.m_Owner == owner && talk.m_fUntil > m_fClock)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! This person's slots are an errand's ways for this player now (their answer is awaited).
	bool OffersTo(IEntity owner, int playerId)
	{
		if (!owner)
			return false;

		foreach (string giver, RZ_ErrandTalk talk : m_mTalks)
		{
			if (talk.m_Owner == owner && talk.m_iPlayer == playerId && talk.m_fUntil > m_fClock)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The talk screen's errand reply on a giver (docs/DIALOGUE.md, "Jobs on the first replies"):
	//! true with the errand's `title` when they have one for this player (`held`: theirs already, to
	//! report back on); else false and `why`, short, for the locked reply. Deals nothing: the errand
	//! looked at is the one dealt when the reply is picked (Deal's preference), while it's still true.
	bool TalkErrand(int playerId, string giverKey, IEntity user, out string title, out bool held, out string why)
	{
		title = "";
		held = false;
		why = "";
		RZ_NamedPerson p = m_People.Get(giverKey);
		if (!IsOn() || !p || !user)
		{
			why = "Nothing today";
			return false;
		}

		RZ_Errand mine = FindFor(playerId, giverKey);
		if (mine && (mine.m_eState == RZ_EErrandState.HELD || mine.m_eState == RZ_EErrandState.OFFERED))
		{
			title = mine.Fill(mine.m_Def.m_sTitle);
			held = mine.m_eState == RZ_EErrandState.HELD;
			return true;
		}

		string refusal = Refusal(playerId, p, user);
		if (!refusal.IsEmpty())
		{
			why = refusal;
			return false;
		}

		string dealWhy;
		RZ_Errand next = Deal(playerId, p, user.GetOrigin(), false, dealWhy, true);
		if (!next)
		{
			why = NothingWhy(dealWhy, playerId, p);
			return false;
		}

		m_mPrefer.Set(playerId, next.m_Def.m_sKey);
		title = next.Fill(next.m_Def.m_sTitle);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The duty a giver offers beside their pressing errand (docs/ERRANDS.md, "Duties"): true with
	//! its `title` when they have something pressing for this player and a duty as well. False when
	//! the errand reply is already their duty (nothing pressing), when the player holds something of
	//! theirs, or when there's no duty to give now. Deals nothing.
	bool TalkDuty(int playerId, string giverKey, IEntity user, out string title)
	{
		title = "";
		RZ_NamedPerson p = m_People.Get(giverKey);
		if (!IsOn() || !m_Settings.m_bDuties || !p || !user || p.m_eKind == RZ_ENamedPlace.WANDERER)
			return false;

		if (FindFor(playerId, giverKey) || !Refusal(playerId, p, user).IsEmpty())
			return false;

		string why;
		RZ_Errand pressing = Deal(playerId, p, user.GetOrigin(), false, why, true);
		if (!pressing || pressing.m_Def.m_bDuty)
			return false;

		RZ_Errand duty = Deal(playerId, p, user.GetOrigin(), false, why, true, DEAL_DUTY);
		if (!duty)
			return false;

		m_mPreferDuty.Set(playerId, duty.m_Def.m_sKey);
		title = duty.Fill(duty.m_Def.m_sTitle);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The duty beside the pressing errand, asked for (the talk screen's [Duty] reply).
	void AskDuty(int playerId, string giverKey, IEntity owner, IEntity user)
	{
		RZ_NamedPerson p = m_People.Get(giverKey);
		if (!IsOn() || !p || !user || FindFor(playerId, giverKey))
			return;

		string refusal = Refusal(playerId, p, user);
		if (!refusal.IsEmpty())
		{
			m_GM.Say(playerId, p.m_sTitle, p.m_iFaction, refusal);
			return;
		}

		string why;
		RZ_Errand duty = Deal(playerId, p, user.GetOrigin(), false, why, false, DEAL_DUTY);
		m_mLastWhy.Set(playerId, why);
		if (!duty)
		{
			m_GM.Say(playerId, p.m_sTitle, p.m_iFaction, string.Format("%1.", NothingWhy(why, playerId, p)));
			return;
		}

		ShowOffer(duty, owner);
	}

	//------------------------------------------------------------------------------------------------
	//! A held errand of this player's from this giver whose step is done: reporting back hands it in.
	bool ReadyToHandIn(int playerId, string giverKey)
	{
		RZ_Errand mine = FindFor(playerId, giverKey);
		return mine && mine.m_eState == RZ_EErrandState.HELD && mine.m_bStepDone;
	}

	//------------------------------------------------------------------------------------------------
	//! What stops this player taking the way in slot `option` of the offer on `owner`, short, for its
	//! locked reply ("Needs a Mechanic", "Needs 2 food", "Needs 10 Haven Favor"); "" when nothing
	//! does. The talk checks (Trust, a threat) aren't locks: their tags show the score.
	string WayLock(IEntity owner, int playerId, int option, IEntity user)
	{
		foreach (string giver, RZ_ErrandTalk talk : m_mTalks)
		{
			if (talk.m_Owner != owner || talk.m_iPlayer != playerId || option < 0 || option >= talk.m_aWays.Count())
				continue;

			int wayIdx = talk.m_aWays[option];
			RZ_Errand e = FindById(talk.m_iErrand);
			if (wayIdx < 0 || !e || wayIdx >= e.m_Def.m_aWays.Count())
				return "";

			RZ_ErrandWay w = e.m_Def.m_aWays[wayIdx];
			int f = w.PayFaction(e.m_iFaction);
			if (!w.m_sBackground.IsEmpty() && !RZ_QuestApproach.HasBackground(m_GM, playerId, w.m_sBackground))
				return "Needs " + BackgroundWords(w.m_sBackground);

			if (w.m_iMinTier >= 0)
			{
				int tf = w.m_iTierFaction;
				if (tf < 0)
					tf = f;

				if (!RZ_Factions.IsValid(tf) || !RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, tf), w.m_iMinTier))
					return string.Format("Needs %1 with %2", RZ_Reputation.GetTierName(w.m_iMinTier), RZ_Factions.GetShortName(tf));
			}

			if (w.m_iItems > 0 && !w.m_bGiveThere && user && CountItems(user, w.m_eItem) < w.m_iItems)
				return "Needs " + ItemNeed(w.m_eItem, w.m_iItems);

			if (w.m_iFavorCost > 0 && RZ_Factions.IsValid(f) && m_GM.GetProfile(playerId).GetFavor(f) < w.m_iFavorCost)
				return string.Format("Needs %1 %2 Favor", w.m_iFavorCost, RZ_Factions.GetShortName(f));

			return "";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! "Need a hand?" on a wanderer (RZ_Wanderers, slot 2 when it's free). True if handled.
	bool AskWanderer(int playerId, RZ_Wanderer w, IEntity owner, IEntity user)
	{
		if (!IsOn() || !m_Settings.m_bWandererAsks || !w || !user)
			return false;

		RZ_NamedPerson p = m_People.FromWanderer(w.m_iId);
		if (!p)
			return false;

		RZ_Errand mine = FindFor(playerId, p.m_sKey);
		if (mine && mine.m_eState == RZ_EErrandState.HELD)
		{
			RZ_ErrandWay held = mine.GetWay();
			if (mine.m_bStepDone && held)
				HandIn(mine, held, playerId);
			else if (held)
				m_GM.NotifyPlayer(playerId, "Errand: " + mine.Fill(mine.m_Def.m_sTitle), mine.Fill(held.StepText()));

			return true;
		}

		string refusal = Refusal(playerId, p, user);
		if (!refusal.IsEmpty())
		{
			m_GM.Say(playerId, p.m_sTitle, -1, refusal);
			return true;
		}

		if (!mine)
		{
			string why;
			mine = Deal(playerId, p, user.GetOrigin(), false, why);
			m_mLastWhy.Set(playerId, why);
		}

		// Barter Chain, as built: three wanderers and a chain of wants, started from one of them
		if (!mine && RZ_ErrandRoutine.StartBarterChain(m_GM, playerId, w, user))
			return true;

		if (!mine)
		{
			m_GM.Say(playerId, p.m_sTitle, -1, "Not today. Ask me again on the road somewhere.");
			return true;
		}

		ShowOffer(mine, owner);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Why this giver won't ask anything of you now, in their own voice; "" if they will.
	protected string Refusal(int playerId, RZ_NamedPerson p, IEntity user)
	{
		vector at = user.GetOrigin();
		if (ShotNear(at, 60, 60) || m_GM.GetSpawner().CountZombiesNear(at, 30, 1) > 0)
			return "Not while that lot's at the fence.";

		if (RZ_Util.IsInVehicle(user))
			return "Get out of the car first. I'm not shouting.";

		if (RZ_Factions.IsValid(p.m_iFaction) && RZ_Reputation.IsHunted(m_GM.GetTier(playerId, p.m_iFaction)))
			return "You've a nerve. Walk on.";

		if (CountHeld(playerId) >= m_Settings.m_iMaxHeld)
			return "You've your hands full already. Finish what you've got.";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected string NothingLine(RZ_NamedPerson p)
	{
		switch (p.m_iFaction)
		{
			case RZ_EFaction.SURVIVORS: return "Not today. The house is managing, just about. Ask the others tomorrow: someone always wants something.";
			case RZ_EFaction.MILITARY: return "Nothing on the duty board for you. Check back after the shift change.";
			case RZ_EFaction.BANDITS: return "Nothing little going. Come back when something's broken, or someone is.";
			case RZ_EFaction.INSTITUTE: return "No requests at present. The shift is quiet, which we're told is good.";
		}

		return "Not today.";
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Errand FindFor(int playerId, string giverKey)
	{
		foreach (RZ_Errand e : m_aErrands)
		{
			if (e.m_iAsker == playerId && e.m_sGiver == giverKey && e.m_eState != RZ_EErrandState.DONE)
				return e;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Errands this player holds. An opening's chain step is the life's first quest, not an errand
	//! (errands and quests are separate: docs/ERRANDS.md, "Errands and jobs are separate"), so it
	//! never counts against the errands a player may hold.
	protected int CountHeld(int playerId)
	{
		int n = 0;
		foreach (RZ_Errand e : m_aErrands)
		{
			if (e.m_iAsker == playerId && e.m_eState == RZ_EErrandState.HELD && !e.IsOpening())
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected float ErrandReach()
	{
		if (m_Settings.m_fErrandReach <= 0)
			return 0;

		return Math.Clamp(m_Settings.m_fErrandReach, 500, 10000);
	}

	//------------------------------------------------------------------------------------------------
	//! The errand's subject lies past errand reach from its giver (docs/ERRANDS.md, "Errand reach").
	bool IsFar(RZ_Errand e)
	{
		float reach = ErrandReach();
		if (reach <= 0 || e.m_vSubject == vector.Zero || e.m_vGiver == vector.Zero)
			return false;

		return RZ_Util.DistXZ(e.m_vSubject, e.m_vGiver) > reach;
	}

	//------------------------------------------------------------------------------------------------
	//! "It's a long walk: about 3 km north-east." for a far errand; empty otherwise.
	protected string FarLine(RZ_Errand e)
	{
		if (!IsFar(e))
			return "";

		int km = RZ_Util.RoundToInt(RZ_Util.DistXZ(e.m_vGiver, e.m_vSubject) / 1000);
		if (km < 1)
			km = 1;

		return string.Format("It's a long walk: about %1 km %2.", km, RZ_Compass.DirectionWord(RZ_Compass.Bearing(e.m_vGiver, e.m_vSubject)));
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasOpenOffer(int playerId)
	{
		foreach (RZ_Errand e : m_aErrands)
		{
			if (e.m_iAsker == playerId && e.m_eState == RZ_EErrandState.OFFERED && !e.IsOpening())
				return true;
		}

		return false;
	}

	// ================================================================ The offer
	//------------------------------------------------------------------------------------------------
	//! The giver asks, and their choice slots become the ways this player may take (three at most,
	//! and "Not now"), each with what it needs as a tag.
	protected void ShowOffer(RZ_Errand e, IEntity owner)
	{
		// A thread: someone else at this place says what you did for them, once
		string entry = ThreadLine(e.m_iAsker, PlaceOf(e), e.m_sGiver, true);
		if (!entry.IsEmpty())
			Say(e, entry, false);

		// The suspect variant: the giver has a question first
		if (e.m_bSuspect)
			Say(e, "Where were you on Tuesday? Never mind. Don't answer that.", false);

		Say(e, Pick(e.m_Def.m_aOffer), false);
		string farLine = FarLine(e);
		if (!farLine.IsEmpty())
			Say(e, farLine, false);

		if (!owner)
			return;

		RZ_PuzzleObjectComponent comp = RZ_PuzzleObjectComponent.Cast(owner.FindComponent(RZ_PuzzleObjectComponent));
		if (!comp)
			return;

		ReleaseTalk(e.m_sGiver);
		RZ_ErrandTalk talk = new RZ_ErrandTalk();
		talk.m_sGiver = e.m_sGiver;
		talk.m_Owner = owner;
		talk.m_iPlayer = e.m_iAsker;
		talk.m_iErrand = e.m_iId;
		talk.m_fUntil = m_fClock + TALK_SECONDS;
		talk.m_sUse = SlotText(comp, false);
		talk.m_sChoice = SlotText(comp, true);
		array<string> labels = {};
		foreach (RZ_ErrandWay w : e.m_Def.m_aWays)
		{
			if (labels.Count() >= 3)
				break;

			if (!WayShown(e, w))
				continue;

			labels.Insert(Label(e, w));
			talk.m_aWays.Insert(w.m_iIndex);
		}

		labels.Insert("Not now");
		talk.m_aWays.Insert(-1);
		foreach (string shown : labels)
		{
			if (shown.Contains("["))
			{
				RZ_Guidance.Report(e.m_iAsker, "tags");
				break;
			}
		}
		comp.SetLabels("", RZ_PuzzleLabels.Join(labels));
		m_mTalks.Set(e.m_sGiver, talk);
	}

	//------------------------------------------------------------------------------------------------
	//! The way can be offered at all: its place exists and the card allows it.
	protected bool WayShown(RZ_Errand e, RZ_ErrandWay w)
	{
		if (!e.m_Def.Offers(this, e, w))
			return false;

		if (w.HasStep() && PlaceFor(e, w.m_ePlace, w.NeedKey()) == vector.Zero)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static string SlotText(RZ_PuzzleObjectComponent comp, bool choice)
	{
		array<string> labels = {};
		int last = -1;
		for (int i = 0; i < 4; i++)
		{
			string l = comp.GetSlotLabel(choice, i);
			labels.Insert(l);
			if (!l.IsEmpty())
				last = i;
		}

		while (labels.Count() > last + 1)
		{
			labels.Remove(labels.Count() - 1);
		}

		return RZ_PuzzleLabels.Join(labels);
	}

	//------------------------------------------------------------------------------------------------
	//! The person's own slots again.
	protected void ReleaseTalk(string giverKey)
	{
		RZ_ErrandTalk talk;
		if (!m_mTalks.Find(giverKey, talk))
			return;

		m_mTalks.Remove(giverKey);
		if (!talk.m_Owner)
			return;

		RZ_PuzzleObjectComponent comp = RZ_PuzzleObjectComponent.Cast(talk.m_Owner.FindComponent(RZ_PuzzleObjectComponent));
		if (comp)
			comp.SetLabels(talk.m_sUse, talk.m_sChoice);
	}

	//------------------------------------------------------------------------------------------------
	//! Talks that ran out, or whose player walked off.
	protected void ExpireTalks(array<int> ids, array<IEntity> players)
	{
		array<string> gone = {};
		foreach (string key, RZ_ErrandTalk talk : m_mTalks)
		{
			bool stale = m_fClock > talk.m_fUntil || !talk.m_Owner;
			int idx = ids.Find(talk.m_iPlayer);
			if (idx < 0)
				stale = true;
			else if (talk.m_Owner && RZ_Util.DistXZ(players[idx].GetOrigin(), talk.m_Owner.GetOrigin()) > TALK_RANGE)
				stale = true;

			if (stale)
				gone.Insert(key);
		}

		foreach (string k : gone)
		{
			ReleaseTalk(k);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "Hand over two tins [2 food]": the label, then what it needs.
	protected string Label(RZ_Errand e, RZ_ErrandWay w)
	{
		string tag = "";
		int f = w.PayFaction(e.m_iFaction);
		if (!w.m_sBackground.IsEmpty())
			tag += "[" + BackgroundWords(w.m_sBackground) + "] ";

		if (w.m_iMinTier >= 0)
		{
			int tf = w.m_iTierFaction;
			if (tf < 0)
				tf = f;

			tag += string.Format("[%1: %2] ", RZ_Reputation.GetTierName(w.m_iMinTier), RZ_Factions.GetShortName(tf));
		}

		if (w.m_iTrust > 0)
		{
			int trustF = w.m_iTrustFaction;
			if (trustF < 0)
				trustF = f;

			tag += RZ_Checks.TrustTag(trustF, RZ_StoryKit.AtHouse(m_GM, e.m_iAsker, e.m_iHouse, w.m_iTrust)) + " ";
		}

		if (w.m_bThreat)
			tag += RZ_Checks.FearTag(m_GM, e.m_iAsker, f, RZ_Checks.THREAT_INFAMY, 2) + " ";

		if (w.m_iItems > 0 && w.m_eItem != RZ_EErrandItem.NONE)
			tag += "[" + ItemNeed(w.m_eItem, w.m_iItems) + "] ";

		if (w.m_iFavorCost > 0)
			tag += string.Format("(%1 Favor) ", w.m_iFavorCost);

		string label = e.Fill(w.m_sLabel);
		if (!tag.IsEmpty())
			label += " " + tag.Trim();

		return label;
	}

	//------------------------------------------------------------------------------------------------
	//! "nurse,doctor" -> "Nurse or Doctor"
	protected string BackgroundWords(string ids)
	{
		array<string> list = {};
		ids.Split(",", list, true);
		string text = "";
		foreach (int i, string id : list)
		{
			string name = id.Trim();
			foreach (RZ_Background bg : m_GM.GetBackgrounds())
			{
				if (bg.m_sId == name)
					name = bg.m_sName;
			}

			if (i > 0)
				text += " or ";

			text += RZ_Util.Capitalize(name);
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! "1 fuel can", "2 food"
	static string ItemNeed(RZ_EErrandItem item, int count)
	{
		switch (item)
		{
			case RZ_EErrandItem.FOOD: return string.Format("%1 food", count);
			case RZ_EErrandItem.FUEL: return string.Format("%1 fuel can", count);
			case RZ_EErrandItem.BATTERY: return string.Format("%1 battery", count);
			case RZ_EErrandItem.MEDICAL: return string.Format("%1 medical", count);
			case RZ_EErrandItem.ANTIDOTE: return string.Format("%1 antidote", count);
			case RZ_EErrandItem.MEAT: return string.Format("%1 raw meat", count);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A pick on the giver's slots while this player talks to them.
	protected void PickWay(RZ_ErrandTalk talk, int option, IEntity user)
	{
		RZ_Errand e = FindById(talk.m_iErrand);
		if (!e || e.m_eState != RZ_EErrandState.OFFERED || option < 0 || option >= talk.m_aWays.Count())
		{
			ReleaseTalk(talk.m_sGiver);
			return;
		}

		int wayIdx = talk.m_aWays[option];
		if (wayIdx < 0)
		{
			// Not now is always free: the offer stands a while, nothing is remembered
			Say(e, "Fair enough. I'll find someone else, or I'll do it myself.", false);
			Count(e.m_Def.m_sFamily).declines++;
			ReleaseTalk(talk.m_sGiver);
			return;
		}

		if (!e.m_Def.Still(this, e))
		{
			ReleaseTalk(talk.m_sGiver);
			Overtake(e);
			return;
		}

		RZ_ErrandWay w = e.m_Def.m_aWays[wayIdx];
		string no = Needs(e, w, user);
		if (!no.IsEmpty())
		{
			Say(e, no, false);
			return;
		}

		ReleaseTalk(talk.m_sGiver);
		Take(e, w, user);
	}

	//------------------------------------------------------------------------------------------------
	//! What stops this player taking this way, in the giver's voice; "" if nothing. Pays the costs
	//! of the ways that cost something now (Favor, a threat, items handed over on the spot).
	protected string Needs(RZ_Errand e, RZ_ErrandWay w, IEntity user)
	{
		int playerId = e.m_iAsker;
		int f = w.PayFaction(e.m_iFaction);
		if (!w.m_sBackground.IsEmpty() && !RZ_QuestApproach.HasBackground(m_GM, playerId, w.m_sBackground))
			return "Not that way. You'd need to know what you're looking at.";

		if (w.m_iMinTier >= 0)
		{
			int tf = w.m_iTierFaction;
			if (tf < 0)
				tf = f;

			if (!RZ_Factions.IsValid(tf) || !RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, tf), w.m_iMinTier))
				return "Not from you. Maybe from someone they know.";
		}

		if (w.m_iTrust > 0)
		{
			int trustF = w.m_iTrustFaction;
			if (trustF < 0)
				trustF = f;

			// A house's people read a bite hidden at their door: one step harder there
			RZ_ECheck result = RZ_Checks.Result(RZ_Checks.TrustScore(m_GM, playerId, trustF, w.m_sBackground), RZ_StoryKit.AtHouse(m_GM, playerId, e.m_iHouse, w.m_iTrust));
			RZ_Log.Info(string.Format("Story: errand.%1 talk %2 by %3", e.m_Def.m_sKey, RZ_Checks.ResultName(result), RZ_Util.PlayerName(playerId)));
			if (result != RZ_ECheck.PASS)
				return "Not from you. Maybe from someone the house knows.";
		}

		if (w.m_iItems > 0 && !w.m_bGiveThere && CountItems(user, w.m_eItem) < w.m_iItems)
			return "You've nothing on you I can use for that. " + RZ_Util.Capitalize(ItemNeed(w.m_eItem, w.m_iItems)) + ", it needs.";

		if (w.m_iFavorCost > 0 && !m_GM.SpendFavor(playerId, f, w.m_iFavorCost))
			return string.Format("That's %1 Favor. You haven't got it with us.", w.m_iFavorCost);

		if (w.m_bThreat)
		{
			string here = RZ_Checks.ThreatRefusedAt(m_GM, user.GetOrigin());
			if (!here.IsEmpty())
				return here;

			RZ_ECheck threat = RZ_Checks.Threaten(m_GM, playerId, e.m_sGiver, e.m_iFaction, e.m_iFaction, 2, false);
			if (threat != RZ_ECheck.PASS)
				return "Is that meant to frighten me? Get away from my door.";
		}

		if (w.m_iItems > 0 && !w.m_bGiveThere)
			TakeItems(user, w.m_eItem, w.m_iItems);

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The way is taken: on the spot it resolves now, else the step starts (soft hold kept).
	protected void Take(RZ_Errand e, RZ_ErrandWay w, IEntity user)
	{
		e.m_iWay = w.m_iIndex;
		e.m_fTakenAt = m_fClock;
		e.m_fProgress = 0;
		e.m_bStepDone = !w.HasStep();
		e.m_eState = RZ_EErrandState.HELD;
		RZ_ErrandStats stats = Count(e.m_Def.m_sFamily);
		if (e.m_Def.m_bCard)
		{
			RZ_ErrandLedger ledger = Ledger(e.m_sIdent);
			string doneKey = CardDoneKey(e.m_Def, e.m_sSubject, e.m_iLife);
			if (!ledger.cards.Contains(doneKey))
				ledger.cards.Insert(doneKey);
		}

		int n;
		stats.ways.Find(w.m_sWay, n);
		stats.ways.Set(w.m_sWay, n + 1);

		if (!w.HasStep())
		{
			Resolve(e, w);
			return;
		}

		e.m_vPlace = PlaceFor(e, w.m_ePlace, w.NeedKey());
		e.SetText("place", PlaceText(e, w.m_ePlace, user.GetOrigin()));
		e.m_bFirstDone = !w.HasFirst();
		if (w.HasFirst())
			e.m_vFirst = PlaceFor(e, w.m_eFirst);
		RZ_PlaceNeeds.Verify(w.NeedKey(), e.m_vPlace, e.m_Def.m_sKey + ": " + e.Fill(w.m_sStep));
		if (w.m_eProp != RZ_EErrandProp.NONE)
			RZ_PlaceNeeds.Verify("errand." + RZ_PlaceThings.PropSlot(w.m_eProp), e.m_vPlace, e.m_Def.m_sKey + ": " + e.Fill(w.m_sStep));
		// A local's spot at its hour (the gate at dusk, the stove at supper), for someone who knows them
		vector spot;
		string localName;
		if (w.m_ePlace == RZ_EErrandPlace.HERE && m_More.LocalSpot(e, spot, localName))
		{
			e.m_vPlace = spot;
			m_GM.NotifyPlayer(e.m_iAsker, "Errand: " + e.Fill(e.m_Def.m_sTitle), string.Format("%1 is there at this hour. Do it where they are.", localName));
		}

		PlaceProp(e, w);
		e.m_Def.Taken(this, e, w);
		Say(e, Pick(w.m_aTake), false);
		string step = e.Fill(w.StepText());
		if (w.m_fBefore >= 0)
			step += string.Format(" Before %1 (about %2 real minutes).", RZ_GameTime.Clock(w.m_fBefore), RZ_Util.RoundToInt(RZ_GameTime.RealMinutesUntil(w.m_fBefore)));

		// The start card (RZ_JobCards): what, from whom, and the step with its direction
		RZ_JobCards.Started(m_GM, e.m_iAsker, "Errand", e.Fill(e.m_Def.m_sTitle), GiverWords(e), step + "\n" + RZ_Util.Capitalize(RZ_Util.DirectionText(user.GetOrigin(), e.m_vPlace)) + ".");
		RZ_Log.Info(string.Format("Errands: %1 taken by %2, way %3 (%4)", e.m_Def.m_sKey, RZ_Util.PlayerName(e.m_iAsker), w.m_iIndex, w.m_sWay));
	}

	// ================================================================ Dealing
	//------------------------------------------------------------------------------------------------
	//! Deal this giver's best true errand to this player: routines and cards anchored to the giver's
	//! place, checked cheapest first (on, cooldowns, why, for whom, still, holds), scored, and picked
	//! weighted among the top three. `why` says what was dealt and why the rest weren't (/errands).
	RZ_Errand Deal(int playerId, RZ_NamedPerson p, vector from, bool unasked, out string why, bool peek = false, int mode = 0)
	{
		why = "";
		if (m_aErrands.Count() >= MAX_LIVE)
		{
			why = "the server holds 12 errands already";
			return null;
		}

		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		RZ_ErrandLedger ledger = Ledger(profile.m_sIdentity);
		int now = RZ_SeasonClock.Now();
		int today = now / 86400;
		if (ledger.cardDay != today)
		{
			ledger.cardDay = today;
			ledger.cardsToday = 0;
		}

		array<ref RZ_Errand> made = {};
		array<float> scores = {};
		array<bool> far = {};
		string place = PlaceKey(p);
		// Chore slots: 1 + 1 per 5 players online (3 at most) routines live at a holding or house at once,
		// and one of them per player there
		bool holdingPlace = p.m_eKind == RZ_ENamedPlace.HOLDING || p.m_eKind == RZ_ENamedPlace.HOUSE;
		int choresHere = ChoresAt(place, -1);
		bool myChoreHere = ChoresAt(place, playerId) > 0;
		int dutiesHere = DutiesAt(place);
		foreach (RZ_ErrandDef d : m_aDefs)
		{
			if (!Anchored(d, p))
				continue;

			// A duty is asked for, never called out or waved over
			if (d.m_bDuty && (!m_Settings.m_bDuties || unasked))
				continue;

			if (mode == DEAL_DUTY && !d.m_bDuty)
				continue;

			string no = "";
			// The epilogue deck: only in the week, outside the day's cards, once per subject (below)
			if (d.m_bAfterCure && !RZ_SeasonClock.IsEpilogue())
				no = "only after the cure";
			else if (d.m_bCard && !d.m_bAfterCure && d.m_iPerLife <= 0 && ledger.cards.Contains(d.m_sKey))
				no = "done this season";
			else if (d.m_iPerLife > 0 && PerLifeCount(ledger, d, profile.m_iLives) >= d.m_iPerLife)
				no = "enough of these this life";
			else if (d.m_bCard && !d.m_bAfterCure && ledger.cardsToday >= m_Settings.m_iCardsPerDay)
				no = "two cards today";
			else if (d.m_bDuty && ledger.CoolUntil(DutyKey(d)) > now)
				no = NO_GAP;
			else if (d.m_bDuty && dutiesHere >= DUTIES_PER_PLACE)
				no = NO_TAKEN;
			else if (!d.m_bCard && (ledger.CoolUntil(d.m_sKey) > now || ledger.CoolUntil(d.m_sKey + "|" + place) > now))
				no = "cooling down";
			else if (!d.m_bCard && PlaceCooling(d.m_sKey, place))
				no = "done here lately";
			else if (!d.m_bCard && holdingPlace && myChoreHere)
				no = NO_MINE;
			else if (!d.m_bCard && !d.m_bDuty && holdingPlace && choresHere >= ChoreSlots())
				no = NO_TAKEN;
			else if (d.m_bTimed && !RZ_GameTime.IsCalibrated())
				no = "no clock";
			else if (!d.ForWhom(this, playerId, p))
				no = "not for you";

			if (no.IsEmpty())
			{
				RZ_Errand e = NewErrand(d, playerId, p, from);
				if (!d.Why(this, e))
				{
					no = "not true";
				}
				else if (d.m_bAfterCure && ledger.cards.Contains(CardDoneKey(d, e.m_sSubject)))
				{
					no = "done for this one";
				}
				else if (d.m_iPerLife > 0 && PerSubjectDone(ledger, d, e.m_sSubject))
				{
					no = "done for this one";
				}
				else if (!d.Still(this, e))
				{
					no = "not still";
				}
				else if (!e.m_sSubject.IsEmpty() && !SubjectFree(e))
				{
					no = "its subject is busy";
				}
				else if (!AnyWay(e))
				{
					no = "no way fits here";
				}
				else
				{
					// A failed story job's fallout of this kind waits here: dealt first, at its cap
					float score = Score(e, p, ledger);
					RZ_Fallout fo = FalloutFor(d.m_sKey, e.m_vGiver, playerId);
					if (fo && !e.m_bFallout)
					{
						e.m_bFallout = true;
						e.m_iCap = fo.cap;
						score *= 3;
					}

					made.Insert(e);
					scores.Insert(score);
					far.Insert(IsFar(e));
				}
			}

			if (m_Settings.m_bDebugLog || !no.IsEmpty())
				why += string.Format("%1: %2. ", d.m_sKey, no);
		}

		// The pressing one overrides: with a routine or a card to ask, this deal is that, and the
		// giver's duty is offered beside it (TalkDuty, AskDuty); with nothing pressing, it's a duty
		bool anyTrue;
		foreach (RZ_Errand mt : made)
		{
			if (!mt.m_Def.m_bDuty)
				anyTrue = true;
		}

		for (int md = made.Count() - 1; md >= 0 && anyTrue; md--)
		{
			if (!made[md].m_Def.m_bDuty)
				continue;

			made.Remove(md);
			scores.Remove(md);
			far.Remove(md);
		}

		// Errand reach: one whose subject is a long walk off is dealt only when nothing nearer is
		bool anyNear;
		foreach (bool isFar : far)
		{
			if (!isFar)
				anyNear = true;
		}

		for (int fd = made.Count() - 1; fd >= 0 && anyNear; fd--)
		{
			if (!far[fd])
				continue;

			why += string.Format("%1: a long walk. ", made[fd].m_Def.m_sKey);
			made.Remove(fd);
			scores.Remove(fd);
			far.Remove(fd);
		}

		if (made.IsEmpty())
			return null;

		// Weighted among the top three
		array<int> order = {};
		for (int i = 0; i < made.Count(); i++)
		{
			order.Insert(i);
		}

		for (int a = 0; a < order.Count(); a++)
		{
			for (int b = a + 1; b < order.Count(); b++)
			{
				if (scores[order[b]] > scores[order[a]])
				{
					int tmp = order[a];
					order[a] = order[b];
					order[b] = tmp;
				}
			}
		}

		int top = Math.Min(3, order.Count());
		float total = 0;
		for (int t = 0; t < top; t++)
		{
			total += scores[order[t]];
		}

		float roll = Math.RandomFloat(0, total);
		int pick = order[0];
		for (int r = 0; r < top; r++)
		{
			roll -= scores[order[r]];
			if (roll <= 0)
			{
				pick = order[r];
				break;
			}
		}

		// The talk screen's look ahead (Peek): the best, and nothing is dealt. The errand the talk
		// screen named on its reply is the one dealt when that reply is picked, while it's still true
		if (peek)
			return made[order[0]];

		string prefer;
		if (mode == DEAL_DUTY && m_mPreferDuty.Find(playerId, prefer))
		{
			m_mPreferDuty.Remove(playerId);
			foreach (int di, RZ_Errand dutyCand : made)
			{
				if (dutyCand.m_Def.m_sKey == prefer)
					pick = di;
			}
		}
		else if (mode != DEAL_DUTY && m_mPrefer.Find(playerId, prefer))
		{
			m_mPrefer.Remove(playerId);
			foreach (int pi, RZ_Errand cand : made)
			{
				if (cand.m_Def.m_sKey == prefer)
					pick = pi;
			}
		}

		RZ_Errand dealt = made[pick];
		// Twists vary per deal: at Twist chance, one errand in three a player at most, never two in a row
		ledger.sinceTwist++;
		if (dealt.m_Def.m_bTwists && ledger.sinceTwist >= 3 && Math.RandomFloat01() < m_Settings.m_fTwistChance)
		{
			dealt.m_bTwist = true;
			ledger.sinceTwist = 0;
			RZ_Log.Info(string.Format("Errands: %1 for %2 has its twist", dealt.m_Def.m_sKey, RZ_Util.PlayerName(playerId)));
			m_iTwists++;
		}

		dealt.m_iId = m_iNextId;
		m_iNextId++;
		dealt.m_bUnasked = unasked;
		dealt.m_fOfferedAt = m_fClock;
		if (!dealt.m_sSubject.IsEmpty() && m_GM.GetFacts())
			m_GM.GetFacts().SoftHold(dealt.m_sSubject, dealt.Owner());

		m_aErrands.Insert(dealt);
		// A card offered counts toward the day's (not the epilogue deck's); taken, it's done for the
		// season (Take)
		if (dealt.m_Def.m_bCard && !dealt.m_Def.m_bAfterCure)
			ledger.cardsToday++;

		ledger.NoteFamily(dealt.m_Def.m_sFamily);
		Count(dealt.m_Def.m_sFamily).deals++;
		if (unasked)
			m_iUnaskedDeals++;
		else
			m_iAskedDeals++;

		m_More.OpenWithLie(dealt);
		m_More.OpenWithReason(dealt);
		why = string.Format("dealt %1. ", dealt.m_Def.m_sKey) + why;
		RZ_Log.Info(string.Format("Errands: dealt %1 to %2 by %3 (%4)", dealt.m_Def.m_sKey, RZ_Util.PlayerName(playerId), p.m_sName, AskedWord(unasked)));
		return dealt;
	}

	//------------------------------------------------------------------------------------------------
	//! What a taken card is remembered by in the ledger: its key, and for the epilogue deck its key
	//! and subject (once per identity per beat or place); a card dealt per subject a few times a life
	//! (His Effects: once per dead man, 3 a life) by key, life and subject.
	static string CardDoneKey(RZ_ErrandDef d, string subject, int life = 0)
	{
		if (d.m_iPerLife > 0)
			return string.Format("%1|L%2|%3", d.m_sKey, life, subject);

		if (!d.m_bAfterCure)
			return d.m_sKey;

		return d.m_sKey + "|" + subject;
	}

	//------------------------------------------------------------------------------------------------
	protected static int PerLifeCount(RZ_ErrandLedger ledger, RZ_ErrandDef d, int life)
	{
		string prefix = string.Format("%1|L%2|", d.m_sKey, life);
		int n = 0;
		foreach (string c : ledger.cards)
		{
			if (c.StartsWith(prefix))
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool PerSubjectDone(RZ_ErrandLedger ledger, RZ_ErrandDef d, string subject)
	{
		foreach (string c : ledger.cards)
		{
			if (c.StartsWith(d.m_sKey + "|L") && c.EndsWith("|" + subject))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static string AskedWord(bool unasked)
	{
		if (unasked)
			return "called out";

		return "asked";
	}

	//------------------------------------------------------------------------------------------------
	//! At least one way can be offered: its place exists.
	protected bool AnyWay(RZ_Errand e)
	{
		foreach (RZ_ErrandWay w : e.m_Def.m_aWays)
		{
			if (WayShown(e, w))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A soft hold elsewhere, or a mission's claim (a stage is happening there now), keeps an errand
	//! off a subject; a story's long hold keeps off only the cards whose premise lists it.
	protected bool SubjectFree(RZ_Errand e)
	{
		RZ_WorldFacts facts = m_GM.GetFacts();
		if (!facts)
			return true;

		string soft = facts.SoftHeldBy(e.m_sSubject);
		if (!soft.IsEmpty())
			return false;

		return !Overtaken(e, facts);
	}

	//------------------------------------------------------------------------------------------------
	//! score = weight x need x fit x novelty x story (docs/ERRANDS.md, "The dealer"). At a giver,
	//! everything is near; freshness is the Why's to hold.
	protected float Score(RZ_Errand e, RZ_NamedPerson p, RZ_ErrandLedger ledger)
	{
		RZ_ErrandDef d = e.m_Def;
		float s = Math.Max(0.05, d.m_fWeight) * 1.3;
		if (e.m_bSuspect)
			s *= 0.5;

		RZ_FactionLife life = m_GM.GetFactionLife();
		if (d.m_iNeed >= 0 && life && RZ_Factions.IsValid(p.m_iFaction) && life.GetNeed(p.m_iFaction) == d.m_iNeed)
			s *= 1.2;

		if (!d.m_sRoles.IsEmpty() && d.m_sRoles.Contains(p.m_sRole))
			s *= 1.2;

		// A duty: who you ask decides which you get (their own work first), and a rung you've only
		// just reached is the one they try you on
		if (d.m_bDuty && !d.m_sRoles.IsEmpty() && d.m_sRoles.Contains(p.m_sRole))
			s *= 2;

		if (d.m_bDuty && d.m_iRung > 1)
			s *= 1 + 0.25 * (d.m_iRung - 1);

		int regard = Regard(e.m_iAsker, p.m_sKey);
		if (regard >= 2)
			s *= 1.2;
		else if (regard <= -1)
			s *= 0.8;

		if (ledger.families.Contains(d.m_sFamily))
			s *= 0.3;

		if (d.m_bCard)
			s *= 1.5;

		// A house whose own you sold: x0.5 for the season (Wrong Side of the Toll)
		if (e.m_iHouse >= 0 && ledger.shy.Contains(e.m_iHouse))
			s *= 0.5;

		// Nearness and freshness (the context cache), and a lie found out on a dead life
		s *= m_More.Factor(e);
		s *= RZ_ErrandsDealer.LiedFactor(ledger, e.m_iFaction);
		return s;
	}

	//------------------------------------------------------------------------------------------------
	protected int Regard(int playerId, string key)
	{
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		if (!trees)
			return 0;

		return trees.GetMemory().Regard(playerId, key);
	}

	//------------------------------------------------------------------------------------------------
	//! The giver's place fits the definition's anchor.
	protected bool Anchored(RZ_ErrandDef d, RZ_NamedPerson p)
	{
		switch (d.m_eAnchor)
		{
			case RZ_EErrandAnchor.HAVEN_HOUSE: return p.m_eKind == RZ_ENamedPlace.HOUSE;
			case RZ_EErrandAnchor.BASTION_POST: return p.m_eKind == RZ_ENamedPlace.HOLDING && p.m_iFaction == RZ_EFaction.MILITARY;
			case RZ_EErrandAnchor.REDHAND_CAMP: return p.m_eKind == RZ_ENamedPlace.HOLDING && p.m_iFaction == RZ_EFaction.BANDITS;
			case RZ_EErrandAnchor.HALCYON_STATION: return p.m_eKind == RZ_ENamedPlace.HOLDING && p.m_iFaction == RZ_EFaction.INSTITUTE;
			case RZ_EErrandAnchor.ANY_HOLDING: return p.m_eKind != RZ_ENamedPlace.WANDERER && (d.m_iFaction < 0 || d.m_iFaction == p.m_iFaction);
			case RZ_EErrandAnchor.WANDERER: return p.m_eKind == RZ_ENamedPlace.WANDERER && (d.m_iWandererRole < 0 || m_GM.GetWanderers().GetRole(p.m_iPlace) == d.m_iWandererRole);
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static string PlaceKey(RZ_NamedPerson p)
	{
		switch (p.m_eKind)
		{
			case RZ_ENamedPlace.HOLDING: return RZ_Faults.HoldingPlace(p.m_iPlace);
			case RZ_ENamedPlace.HOUSE: return RZ_Faults.HousePlace(p.m_iPlace);
		}

		return "w" + p.m_iPlace.ToString();
	}

	// ================================================================ Pay at the next join
	//------------------------------------------------------------------------------------------------
	//! Pay for someone who earned it and wasn't online (a story job, the finale, a questline's
	//! ending): paid at their next join, to that life only. 12 at most, oldest dropped.
	void AddOwed(string ident, RZ_ErrandOwed o)
	{
		if (ident.IsEmpty() || !o || !RZ_Factions.IsValid(o.faction))
			return;

		RZ_ErrandLedger ledger = Ledger(ident);
		ledger.Ensure();
		if (ledger.owed.Count() >= 12)
			ledger.owed.RemoveOrdered(0);

		ledger.owed.Insert(o);
	}

	//------------------------------------------------------------------------------------------------
	//! Every 30 s: the online players' owed pay, through AddFame, Forgive, AddFavor and AddInfamy,
	//! to the life that earned it only, and only while the giver would still have them. All of one
	//! player's owed pay is one quiet notebook entry, "While you were away", never a card each
	//! (docs/UI.md, "Notices").
	protected void PayPending(array<int> ids)
	{
		foreach (int pid : ids)
		{
			RZ_PlayerProfile profile = m_GM.GetProfile(pid);
			if (!profile || !m_mLedgers.Contains(profile.m_sIdentity))
				continue;

			RZ_ErrandLedger ledger = Ledger(profile.m_sIdentity);
			ledger.Ensure();
			if (!ledger.pending.IsEmpty())
			{
				RZ_Log.Info(string.Format("Errands: %1 old pending pay entries for %2 dropped (no life on them)", ledger.pending.Count(), RZ_Util.PlayerName(pid)));
				ledger.pending.Clear();
			}

			string summary = "";
			foreach (RZ_ErrandOwed o : ledger.owed)
			{
				if (!o)
					continue;

				string paid = PayOwed(pid, profile, o);
				if (!paid.IsEmpty())
					summary += paid + "\n\n";
			}

			ledger.owed.Clear();
			summary = summary.Trim();
			if (!summary.IsEmpty())
				m_GM.NotifyPlayerAs(pid, "While you were away", summary, RZ_ENoticeTag.QUIET);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Pays one owed entry; returns its line for the summary ("" if nothing is said).
	protected string PayOwed(int pid, RZ_PlayerProfile profile, RZ_ErrandOwed o)
	{
		RZ_EFaction f = o.faction;
		if (!RZ_Factions.IsValid(f))
			return "";

		if (o.life != profile.m_iLives)
		{
			RZ_Log.Info(string.Format("Errands: '%1' owed %2 on life %3; they're on life %4: not paid", o.title, RZ_Util.PlayerName(pid), o.life, profile.m_iLives));
			return "";
		}

		string name = RZ_Factions.GetName(f);
		RZ_ETier minTier = o.minTier;
		if (o.checkTier && !RZ_MissionRequirements.GiverTierOk(m_GM.GetTier(pid, f), minTier, o.allowWatched, o.leaderDecides))
		{
			return string.Format("%1: finished while you were away. %2 won't pay you for it now.", o.title, name);
		}

		string text = o.title + ": finished while you were away. Your part counted.";
		bool forgiven = false;
		if (o.amends && o.fame > 0 && RZ_Reputation.IsWatchedOrWorse(m_GM.GetTier(pid, f)))
		{
			int off = m_GM.Forgive(pid, f, o.fame, "amends contract");
			if (off > 0)
			{
				forgiven = true;
				text += string.Format("\n%1 is willing to forget a little: -%2 Infamy.", name, off);
			}
		}

		if (!forgiven && o.fame > 0)
		{
			m_GM.AddFame(pid, f, o.fame, o.applyRivals, o.flags, o.deed);
			text += string.Format("\n+%1 Fame with %2.", o.fame, name);
		}

		if (o.favor > 0)
		{
			m_GM.AddFavor(pid, f, o.favor);
			text += string.Format("\n+%1 Favor with %2.", o.favor, name);
		}

		o.Ensure();
		foreach (int i, int rf : o.infamyFactions)
		{
			if (i >= o.infamyAmounts.Count() || i >= o.infamyWhy.Count() || i >= o.infamyFlags.Count())
				break;

			RZ_EFaction ef = rf;
			m_GM.AddInfamy(pid, ef, o.infamyAmounts[i], o.infamyWhy[i], o.infamyFlags[i]);
			text += string.Format("\n%1 won't forget this: +%2 Infamy.", RZ_Factions.GetName(ef), o.infamyAmounts[i]);
		}

		return text;
	}

	// ================================================================ Threads
	//------------------------------------------------------------------------------------------------
	//! The place key of an errand's giver ("" for a wanderer).
	protected string PlaceOf(RZ_Errand e)
	{
		if (e.m_iHolding >= 0)
			return RZ_Faults.HoldingPlace(e.m_iHolding);

		if (e.m_iHouse >= 0 && e.m_iWanderer < 0)
			return RZ_Faults.HousePlace(e.m_iHouse);

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A done errand whose definition has a thread phrase: "place|giver|what you did", 12 a season at
	//! most (the oldest goes).
	protected void AddThread(RZ_Errand e, RZ_ErrandWay w, RZ_ErrandLedger ledger)
	{
		string place = PlaceOf(e);
		if (e.m_Def.m_sThread.IsEmpty() || place.IsEmpty() || w.m_sWay == "leave")
			return;

		ledger.Ensure();
		string entry = string.Format("%1|%2|%3", place, e.m_sGiverName, e.Fill(e.m_Def.m_sThread));
		if (ledger.threads.Contains(entry))
			return;

		ledger.threads.Insert(entry);
		while (ledger.threads.Count() > 12)
		{
			ledger.threads.Remove(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A line someone at `place` says about what this player did there for someone else
	//! (*"Mara says you brought tins for the pot."*): the newest thread there not from `notGiver`; ""
	//! if none. With `once`, each thread is said back once. Read-only for other systems (a leader's
	//! first line, dialogue) through ThreadLineFor.
	string ThreadLine(int playerId, string place, string notGiver, bool once)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		if (!profile || place.IsEmpty())
			return "";

		RZ_ErrandLedger ledger = Ledger(profile.m_sIdentity);
		ledger.Ensure();
		RZ_NamedPerson other = m_People.Get(notGiver);
		string otherName = "";
		if (other)
			otherName = other.m_sName;

		for (int i = ledger.threads.Count() - 1; i >= 0; i--)
		{
			string entry = ledger.threads[i];
			array<string> parts = {};
			entry.Split("|", parts, false);
			if (parts.Count() < 3 || parts[0] != place || parts[1] == otherName)
				continue;

			if (once && ledger.threadsSaid.Contains(entry))
				continue;

			if (once)
				ledger.threadsSaid.Insert(entry);

			return string.Format("%1 says you %2.", parts[1], parts[2]);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! For a leader or a talk line at a holding or house (holdingId or houseIdx, the other -1): the
	//! newest thing this player did for the people there, as a line; "" if nothing. With `once`
	//! (a leader's greeting), each thread is said back once, as errand givers say it; without (the
	//! majors' sittings), it only reads.
	string ThreadLineFor(int playerId, int holdingId, int houseIdx, bool once = false)
	{
		string place = "";
		if (holdingId >= 0)
			place = RZ_Faults.HoldingPlace(holdingId);
		else if (houseIdx >= 0)
			place = RZ_Faults.HousePlace(houseIdx);

		return ThreadLine(playerId, place, "", once);
	}

	//------------------------------------------------------------------------------------------------
	//! Chore slots a holding or house has now: 1, plus 1 per 5 players online, 3 at most.
	protected static int ChoreSlots()
	{
		return Math.MinInt(3, 1 + GetGame().GetPlayerManager().GetPlayerCount() / 5);
	}

	//------------------------------------------------------------------------------------------------
	//! Live routines (offered or held) at this place, all of them or one player's (playerId > 0).
	protected int ChoresAt(string place, int playerId)
	{
		int n = 0;
		foreach (RZ_Errand e : m_aErrands)
		{
			if (e.m_eState == RZ_EErrandState.DONE || !e.m_Def || e.m_Def.m_bCard || e.IsOpening() || (playerId > 0 && e.m_iAsker != playerId))
				continue;

			// A duty is one of yours here, but takes none of the place's chore slots
			if (playerId <= 0 && e.m_Def.m_bDuty)
				continue;

			string at = "";
			if (e.m_iHolding >= 0)
				at = RZ_Faults.HoldingPlace(e.m_iHolding);
			else if (e.m_iHouse >= 0 && e.m_iWanderer < 0)
				at = RZ_Faults.HousePlace(e.m_iHouse);

			if (at == place)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Live duties (offered or held) at this place.
	protected int DutiesAt(string place)
	{
		int n = 0;
		foreach (RZ_Errand e : m_aErrands)
		{
			if (e.m_eState == RZ_EErrandState.DONE || !e.m_Def || !e.m_Def.m_bDuty)
				continue;

			if (e.m_iHolding >= 0 && RZ_Faults.HoldingPlace(e.m_iHolding) == place)
				n++;
			else if (e.m_iHouse >= 0 && e.m_iWanderer < 0 && RZ_Faults.HousePlace(e.m_iHouse) == place)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! What a duty's gap is kept under in the ledger: its kind, so the same duty in another place's
	//! voice is the same duty.
	static string DutyKey(RZ_ErrandDef d)
	{
		if (d.m_sDuty.IsEmpty())
			return d.m_sKey;

		return "duty." + d.m_sDuty;
	}

	//------------------------------------------------------------------------------------------------
	//! Errands and duties this player has done for a faction this season (the trust ladder's second
	//! rung: KNOWN_FACE of them, RZ_Duties.Rung).
	int WorkedFor(int playerId, int faction)
	{
		RZ_PlayerProfile profile = m_GM.FindProfile(playerId);
		if (!profile || faction < 0 || faction >= 4)
			return 0;

		return Ledger(profile.m_sIdentity).worked[faction];
	}

	//------------------------------------------------------------------------------------------------
	protected bool PlaceCooling(string key, string place)
	{
		float at;
		if (!m_mPlaceCool.Find(key + "|" + place, at))
			return false;

		return m_fClock - at < PLACE_COOL;
	}

	//------------------------------------------------------------------------------------------------
	//! A fresh record for this giver and player, its giver's slots filled.
	protected RZ_Errand NewErrand(RZ_ErrandDef d, int playerId, RZ_NamedPerson p, vector from)
	{
		RZ_Errand e = new RZ_Errand();
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		e.m_Def = d;
		e.m_iAsker = playerId;
		e.m_sIdent = profile.m_sIdentity;
		e.m_iLife = profile.m_iLives;
		e.m_sGiver = p.m_sKey;
		e.m_sGiverName = p.m_sName;
		e.m_sGiverTitle = p.m_sTitle;
		e.m_iFaction = p.m_iFaction;
		if (!RZ_Factions.IsValid(e.m_iFaction))
			e.m_iFaction = RZ_EFaction.SURVIVORS;

		e.m_vGiver = p.m_vPos;
		e.m_vOffer = from;
		if (p.m_eKind == RZ_ENamedPlace.HOLDING)
			e.m_iHolding = p.m_iPlace;
		else if (p.m_eKind == RZ_ENamedPlace.HOUSE)
			e.m_iHouse = p.m_iPlace;
		else
			e.m_iWanderer = p.m_iPlace;

		e.m_iSettlement = m_GM.FindNearestSettlement(p.m_vPos);
		e.SetText("giver", p.m_sName);
		e.SetText("role", p.RoleText());
		e.SetText("here", p.m_sPlace);
		if (e.m_iHouse >= 0)
			e.SetText("house", p.m_sPlace);

		return e;
	}

	protected static const int TOWNS_TRIED = 4;			// towns a way's need is looked for in, nearest first
	protected static const float TOWN_REACH = 150;		// and this far out of a town's radius at most

	//------------------------------------------------------------------------------------------------
	//! Every errand definition (the place audit reads their ways, RZ_PlaceAudit).
	array<ref RZ_ErrandDef> GetDefs()
	{
		return m_aDefs;
	}

	//------------------------------------------------------------------------------------------------
	//! Where a way's step is, from the giver's place: within 1.5 km of the offer, never in the water
	//! or at the line, and true to its words: a place that meets the way's need (docs/PLACES.md; empty
	//! `needKey`: the place kind's own), the need's own kind of spot tried first (a house, the well,
	//! the farmhouse, a road's verge...). Zero if there's no such place here: the way isn't offered.
	vector PlaceFor(RZ_Errand e, RZ_EErrandPlace kind, string needKey = "")
	{
		if (needKey.IsEmpty())
		{
			RZ_ErrandWay probe = new RZ_ErrandWay();
			probe.m_ePlace = kind;
			needKey = probe.NeedKey();
		}

		RZ_PlaceNeed need = RZ_PlaceNeeds.Get(needKey);
		vector base = e.m_vGiver;
		switch (kind)
		{
			case RZ_EErrandPlace.HERE: return RZ_PlaceFind.Near(base, 4, 12, need);
			case RZ_EErrandPlace.GATE: return RZ_PlaceFind.Near(base, 15, 25, need);
			case RZ_EErrandPlace.OUT: return RZ_PlaceFind.NearKept(base, 200, 400, need);
			case RZ_EErrandPlace.FAR: return RZ_PlaceFind.NearKept(base, 300, 500, need);
			case RZ_EErrandPlace.EMPTY_VILLAGE: return TownNear(e, 1200, true, need);
			case RZ_EErrandPlace.NEXT_TOWN: return TownNear(e, MAX_DISTANCE, false, need);
			case RZ_EErrandPlace.SUBJECT: return Within(e, e.m_vSubject);
			case RZ_EErrandPlace.SUBJECT2: return Within(e, e.m_vSubject2);
		}

		return vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	protected vector Within(RZ_Errand e, vector at)
	{
		if (at == vector.Zero || RZ_Util.DistXZ(at, e.m_vOffer) > MAX_DISTANCE)
			return vector.Zero;

		return at;
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest other town within `radius` of the giver (an empty one: nobody holds it) that has
	//! a place meeting `need` (by a house in it, its well, its school...): that place, which is nearer
	//! that town than any other, so the {place} its words name is that town; zero if none of the
	//! TOWNS_TRIED nearest has one.
	protected vector TownNear(RZ_Errand e, float radius, bool empty, RZ_PlaceNeed need)
	{
		RZ_WorldFacts facts = m_GM.GetFacts();
		array<int> towns = {};
		array<float> dists = {};
		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			if (i == e.m_iSettlement || s.m_bIsland)
				continue;

			float d = RZ_Util.DistXZ(s.m_vCenter, e.m_vGiver);
			if (d > radius || d < 150)
				continue;

			if (empty && facts && facts.IsLivedIn(i))
				continue;

			// Nearest first
			int at = 0;
			while (at < dists.Count() && dists[at] < d)
			{
				at++;
			}

			towns.InsertAt(i, at);
			dists.InsertAt(d, at);
		}

		for (int k = 0; k < towns.Count() && k < TOWNS_TRIED; k++)
		{
			RZ_Settlement town = m_GM.GetSettlement(towns[k]);
			float reach = town.m_fRadius;
			if (need)
				reach += Math.Min(need.m_fTownReach, TOWN_REACH);

			vector found = RZ_PlaceFind.NearKept(town.m_vCenter, 0, reach, need);
			if (found != vector.Zero && m_GM.FindNearestSettlement(found) == towns[k])
				return found;
		}

		return vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	//! "the farm buildings at Tisy", "about 300 m NE"
	protected string PlaceText(RZ_Errand e, RZ_EErrandPlace kind, vector from)
	{
		if (kind == RZ_EErrandPlace.SUBJECT || kind == RZ_EErrandPlace.SUBJECT2)
		{
			string sub = e.GetText("subject_place");
			if (!sub.IsEmpty())
				return sub;
		}

		vector at = e.m_vPlace;
		if (kind == RZ_EErrandPlace.EMPTY_VILLAGE || kind == RZ_EErrandPlace.NEXT_TOWN)
		{
			RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(at));
			if (s)
				return s.m_sName;
		}

		if (kind == RZ_EErrandPlace.HERE || kind == RZ_EErrandPlace.GATE)
			return e.GetText("here");

		return RZ_Util.DirectionText(from, at);
	}

	// ================================================================ Paying
	//------------------------------------------------------------------------------------------------
	//! Done: the giver's line, the pay through the one small-pay ledger, the definition's capped
	//! effect, the giver's regard and the records.
	protected void Resolve(RZ_Errand e, RZ_ErrandWay w)
	{
		if (e.m_eState == RZ_EErrandState.DONE)
			return;

		array<string> said = {};
		e.m_Def.Resolve(this, e, w, e.m_iAsker, said);
		string doneLine = Pick(w.m_aDone);
		Say(e, doneLine, false);
		foreach (string line : said)
		{
			Say(e, line, false);
		}

		ConsumeCarried(e);
		// A handed errand (an opening's step) is paid by its owner, never through the ledger
		string pay = "";
		if (!e.m_Def.m_bHanded)
			pay = Pay(e, w, e.m_iAsker, false);

		// The end card (RZ_JobCards): "Errand done: <title>" with the pay (a handed errand's owner
		// sends its own, with its pay)
		string title = e.Fill(e.m_Def.m_sTitle);
		if (!e.m_Def.m_bHanded)
		{
			if (pay.IsEmpty())
				pay = e.Fill("{giver} thanks you.");

			RZ_JobCards.Ended(m_GM, e.m_iAsker, "Errand", "done", title, pay);
		}

		foreach (int helper : e.m_aHelpers)
		{
			if (e.m_Def.m_bHanded)
				break;

			string helperPay = Pay(e, w, helper, true);
			if (!helperPay.IsEmpty())
				RZ_JobCards.Ended(m_GM, helper, "Errand", "done", title, helperPay);
		}

		// Memory: what you did for them (dialogue's regard, on their own key), the town, the hall
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		if (trees && e.m_iWanderer < 0)
			trees.GetMemory().AddRegard(e.m_iAsker, e.m_sGiver, "thanks");

		if (e.m_iWanderer >= 0 && m_GM.GetWanderers())
			m_GM.GetWanderers().RememberPlayer(e.m_iWanderer, e.m_iAsker, RZ_Wanderers.MEM_HELPED);

		RZ_WorldFacts facts = m_GM.GetFacts();
		if (facts && e.m_iSettlement >= 0)
			facts.Memory().Visit(e.m_iSettlement, e.m_sIdent);

		RZ_ErrandLedger ledger = Ledger(e.m_sIdent);
		ledger.helped++;
		AddThread(e, w, ledger);
		// The trust ladder's second rung: the third piece of work for a faction, and they know your face
		if (!e.m_Def.m_bHanded && e.m_iFaction >= 0 && e.m_iFaction < 4)
		{
			ledger.worked[e.m_iFaction] = ledger.worked[e.m_iFaction] + 1;
			if (ledger.worked[e.m_iFaction] == KNOWN_FACE && m_Settings.m_bDuties && RZ_Reputation.GetAccessRank(m_GM.GetTier(e.m_iAsker, e.m_iFaction)) == 0)
				m_GM.NotifyPlayer(e.m_iAsker, RZ_Factions.GetShortName(e.m_iFaction), string.Format("That's three pieces of work for %1. They know your face now, and their people will trust you with more: ask them for work.", RZ_Factions.GetShortName(e.m_iFaction)));
		}

		// A callback: what the giver says later, near them (words only)
		RZ_Shared callShared = m_GM.GetShared();
		if (!w.m_aLater.IsEmpty() && callShared && callShared.GetCallbacks())
			callShared.GetCallbacks().Queue(e.m_iAsker, e.m_sGiverTitle, e.Fill(Pick(w.m_aLater)), e.m_vGiver);
		if (m_Settings.m_bHelpedLine && !e.m_Def.m_bHanded && m_GM.GetHall() && !(m_GM.GetCure() && m_GM.GetCure().IsCured()))
			m_GM.GetHall().AddHelped(e.m_iAsker);

		if (e.m_Def.m_bDuty)
		{
			ledger.SetCool(DutyKey(e.m_Def), RZ_SeasonClock.Now() + RZ_Util.RoundToInt(m_Settings.m_fDutyGapMinutes * 60));
		}
		else if (!e.m_Def.m_bCard && !e.m_Def.m_bHanded)
		{
			int now = RZ_SeasonClock.Now();
			string place = PlaceKey(m_People.Get(e.m_sGiver));
			ledger.SetCool(e.m_Def.m_sKey, now + COOL_ANYWHERE);
			ledger.SetCool(e.m_Def.m_sKey + "|" + place, now + COOL_HERE);
			m_mPlaceCool.Set(e.m_Def.m_sKey + "|" + place, m_fClock);
		}

		if (e.m_iHouse >= 0)
			m_mCounters.Set("house." + e.m_iHouse.ToString(), Counter("house." + e.m_iHouse.ToString()) + 1);

		EndFallout(e);
		// A light callback deed (Core/RZ_Deeds.c): at most four of the twelve
		RZ_PlayerProfile doer = m_GM.FindProfile(e.m_iAsker);
		if (doer && !e.m_sGiverName.IsEmpty() && !e.m_Def.m_bHanded)
			RZ_Deeds.Callback(doer, "errand." + e.m_Def.m_sKey, "did " + e.m_sGiverName + " a favour", RZ_Deeds.LIGHT);

		Count(e.m_Def.m_sFamily).done++;
		RZ_Log.Info(string.Format("Errands: %1 done by %2, way %3 (%4), with %5 helper(s)", e.m_Def.m_sKey, RZ_Util.PlayerName(e.m_iAsker), w.m_iIndex, w.m_sWay, e.m_aHelpers.Count()));
		Remember(e, RZ_EJobResult.DONE, e.Fill(doneLine));
		End(e, "done");
	}

	//------------------------------------------------------------------------------------------------
	//! The one small-pay ledger: Favor (x the pay scale, x the quiet bonus, diminishing, inside 40 an
	//! hour), Fame (at most 2, never above 39, 10 a faction a season day, small pay), or amends at 15+
	//! Infamy (2 off through Forgive, 8 a life per faction, never below 15). Hunted: nothing. A helper
	//! gets half the Favor and at most 1 Fame. The system line, "" if nothing was paid.
	protected string Pay(RZ_Errand e, RZ_ErrandWay w, int playerId, bool helper)
	{
		int f = w.PayFaction(e.m_iFaction);
		if (!RZ_Factions.IsValid(f) || playerId <= 0)
			return "";

		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		RZ_ErrandLedger ledger = Ledger(profile.m_sIdentity);
		RZ_ErrandStats stats = Count(e.m_Def.m_sFamily);
		if (RZ_Reputation.IsHunted(profile.GetTier(f)))
			return "";

		string text = "";
		int infamy = profile.GetInfamy(f);
		bool amends = w.m_bAmends || infamy >= AMENDS_FLOOR;
		if (amends && !helper)
		{
			if (ledger.amendsLife != profile.m_iLives)
			{
				ledger.amendsLife = profile.m_iLives;
				for (int a = 0; a < ledger.amendsDone.Count(); a++)
				{
					ledger.amendsDone[a] = 0;
				}
			}

			int room = Math.Min(m_Settings.m_iAmendsCap - ledger.amendsDone[f], infamy - AMENDS_FLOOR);
			int off = 0;
			if (room > 0)
				off = m_GM.Forgive(playerId, f, Math.Min(2, room), "errand amends");

			if (off > 0)
			{
				ledger.amendsDone[f] = ledger.amendsDone[f] + off;
				stats.amends += off;
				text += string.Format("-%1 Infamy with %2. ", off, RZ_Factions.GetName(f));
			}
		}

		// Favor: x the pay scale and the quiet bonus, diminishing in the real hour, inside its cap
		int hour = RZ_SeasonClock.Now() / 3600;
		if (ledger.hourUnix != hour)
		{
			ledger.hourUnix = hour;
			ledger.hourFavor = 0;
			ledger.hourCount = 0;
		}

		// An amends way pays the amends instead of its Favor (the Latrine: Watched, amends; a Stranger, Favor)
		float favor = Math.Max(0, w.m_iFavor + Math.Min(e.m_iBonus, 3)) * m_Settings.m_fPayScale;
		if (w.m_bAmends && infamy >= AMENDS_FLOOR)
			favor = 0;

		// The asker's pay may come another way: supper at a fed house, or part of it in kind
		if (!helper && favor > 0)
		{
			string meal = m_PayKinds.Meal(e, w, playerId);
			if (!meal.IsEmpty())
			{
				favor = 0;
				text += meal + " ";
			}

			string kindLine;
			int cut = 0;
			if (favor > 0)
				cut = m_PayKinds.InKind(e, w, playerId, kindLine);

			if (cut > 0)
			{
				favor = Math.Max(0, favor - cut);
				text += kindLine + " ";
			}
		}

		if (!e.m_bNoisy)
			favor *= w.m_fQuietMult;

		if (helper)
			favor *= 0.5;

		array<float> steps = {1.0, 1.0, 0.75, 0.5, 0.25};
		favor *= steps[Math.Min(ledger.hourCount, steps.Count() - 1)];
		int paid = Math.Min(RZ_Util.RoundToInt(favor), Math.Max(0, m_Settings.m_iFavorPerHour - ledger.hourFavor));
		if (e.m_iCap >= 0)
			paid = Math.Min(paid, e.m_iCap);	// a fallout: at most 30% of the failed job's base

		// A post short of supplies pays when its truck comes
		if (paid > 0 && !helper && m_PayKinds.Defer(e, playerId, f, paid))
		{
			ledger.hourFavor += paid;
			text += string.Format("Owed when the next truck comes: %1 %2 Favor. ", paid, RZ_Factions.GetName(f));
			paid = 0;
		}

		if (paid > 0)
		{
			if (e.m_bFallout)
				RZ_StoryStats.FalloutPaid(paid);

			m_GM.AddFavor(playerId, f, paid);
			ledger.hourFavor += paid;
			stats.favor += paid;
			text += string.Format("+%1 %2 Favor. ", paid, RZ_Factions.GetName(f));
		}

		ledger.hourCount++;

		// Fame: small pay, under the ceiling and the day's cap; never while it's amends
		int fame = w.m_iFame;
		if (helper)
			fame = Math.Min(1, fame);

		if (e.m_iFameCap >= 0)
			fame = Math.Min(fame, e.m_iFameCap);

		if (fame > 0 && !amends)
		{
			int day = RZ_SeasonClock.Day();
			if (ledger.fameDay != day)
			{
				ledger.fameDay = day;
				for (int d = 0; d < ledger.fameToday.Count(); d++)
				{
					ledger.fameToday[d] = 0;
				}
			}

			int give = Math.Min(fame, Math.Max(0, m_Settings.m_iFameCeiling - profile.GetFame(f)));
			give = Math.Min(give, Math.Max(0, m_Settings.m_iFamePerDay - ledger.fameToday[f]));
			if (give > 0)
			{
				m_GM.AddFame(playerId, f, give, false, RZ_RepFlag.SMALL);
				ledger.fameToday[f] = ledger.fameToday[f] + give;
				stats.fame += give;
				text += string.Format("+%1 Fame with %2.", give, RZ_Factions.GetName(f));
			}
		}

		return text.Trim();
	}

	//------------------------------------------------------------------------------------------------
	//! Small Fame with a second faction (a card's way that pleases two), inside the same ceiling and
	//! day's cap, and never at 15+ Infamy. Returns what was given.
	int SmallFame(int playerId, int f, int amount)
	{
		if (amount <= 0 || !RZ_Factions.IsValid(f))
			return 0;

		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		if (profile.GetInfamy(f) >= AMENDS_FLOOR || RZ_Reputation.IsHunted(profile.GetTier(f)))
			return 0;

		RZ_ErrandLedger ledger = Ledger(profile.m_sIdentity);
		int day = RZ_SeasonClock.Day();
		if (ledger.fameDay != day)
		{
			ledger.fameDay = day;
			for (int d = 0; d < ledger.fameToday.Count(); d++)
			{
				ledger.fameToday[d] = 0;
			}
		}

		int give = Math.Min(Math.Min(amount, 2), Math.Max(0, m_Settings.m_iFameCeiling - profile.GetFame(f)));
		give = Math.Min(give, Math.Max(0, m_Settings.m_iFamePerDay - ledger.fameToday[f]));
		if (give <= 0)
			return 0;

		m_GM.AddFame(playerId, f, give, false, RZ_RepFlag.SMALL);
		ledger.fameToday[f] = ledger.fameToday[f] + give;
		m_GM.NotifyPlayer(playerId, RZ_Factions.GetName(f), string.Format("+%1 Fame with %2.", give, RZ_Factions.GetName(f)));
		return give;
	}

	//------------------------------------------------------------------------------------------------
	//! A house whose fit for this player is x0.5 for the season.
	void HouseShy(int playerId, int house)
	{
		RZ_ErrandLedger ledger = Ledger(m_GM.GetProfile(playerId).m_sIdentity);
		if (house >= 0 && !ledger.shy.Contains(house))
			ledger.shy.Insert(house);
	}

	//------------------------------------------------------------------------------------------------
	//! A few Favor outside an errand (a stranger's house, a card's second payer), through the same
	//! hourly ledger: x the pay scale, diminishing, inside the hour's cap. Hunted: nothing. Returns what
	//! was paid, and tells the player.
	int SmallFavor(int playerId, int f, int amount)
	{
		if (amount <= 0 || !RZ_Factions.IsValid(f) || playerId <= 0)
			return 0;

		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		if (RZ_Reputation.IsHunted(profile.GetTier(f)))
			return 0;

		RZ_ErrandLedger ledger = Ledger(profile.m_sIdentity);
		int hour = RZ_SeasonClock.Now() / 3600;
		if (ledger.hourUnix != hour)
		{
			ledger.hourUnix = hour;
			ledger.hourFavor = 0;
			ledger.hourCount = 0;
		}

		array<float> steps = {1.0, 1.0, 0.75, 0.5, 0.25};
		float favor = amount * m_Settings.m_fPayScale * steps[Math.Min(ledger.hourCount, steps.Count() - 1)];
		int paid = Math.Min(RZ_Util.RoundToInt(favor), Math.Max(0, m_Settings.m_iFavorPerHour - ledger.hourFavor));
		ledger.hourCount++;
		if (paid <= 0)
			return 0;

		m_GM.AddFavor(playerId, f, paid);
		ledger.hourFavor += paid;
		m_GM.NotifyPlayer(playerId, RZ_Factions.GetName(f), string.Format("+%1 %2 Favor.", paid, RZ_Factions.GetName(f)));
		return paid;
	}

	// ================================================================ Unasked: a holding's people wave you over
	//------------------------------------------------------------------------------------------------
	//! Every 30 s, three players at most (round-robin): a player passing within 30 m of a safehouse's
	//! or a holding's shown named people may be waved over, when something is true there and every
	//! rule holds (alive 3 minutes, no fight, not driving, not near a running mission or a set piece,
	//! fewer than the most held, no open offer, the offer gap, and the interruption budget).
	protected void CallOuts(array<int> ids, array<IEntity> players)
	{
		if (!m_Settings.m_bCallOut || ids.IsEmpty())
			return;

		int tries = Math.Min(CALLOUTS_PER_TICK, ids.Count());
		for (int t = 0; t < tries; t++)
		{
			m_iCallCursor = (m_iCallCursor + 1) % ids.Count();
			int playerId = ids[m_iCallCursor];
			IEntity me = players[m_iCallCursor];
			RZ_NamedPerson caller = CallerNear(me.GetOrigin());
			if (!caller || m_mTalks.Contains(caller.m_sKey) || !MayInterrupt(playerId, me))
				continue;

			string why;
			RZ_Errand e = Deal(playerId, caller, me.GetOrigin(), true, why);
			m_mLastWhy.Set(playerId, why);
			if (!e)
				continue;

			RZ_NoticeLanes lanes = RZ_NoticeLanes.Get();
			if (lanes && !lanes.TryContact(playerId, RZ_EContact.ERRAND))
			{
				// The budget is a ceiling only: the offer goes, nothing is lost
				End(e, "no room in the budget");
				m_iUnaskedDeals--;
				continue;
			}

			// Jobs only by interaction: they wave you over and the errand waits as their open offer;
			// they say what it is only when you ask ("Anything need doing?", Ask)
			m_mLastUnasked.Set(m_GM.GetProfile(playerId).m_sIdentity, m_fClock);
			m_GM.Say(playerId, caller.m_sTitle, caller.m_iFaction, e.Fill("You there. Have you got a minute? Something wants doing, when you're free."));
		}
	}

	// ================================================================ Board and radio asks
	//------------------------------------------------------------------------------------------------
	//! The first routine this giver would deal this player now, without dealing it; null if none.
	//! (A board or radio ask is only ever something true: the ask itself is still "Anything need doing?")
	RZ_ErrandDef DryDeal(int playerId, RZ_NamedPerson p)
	{
		RZ_ErrandLedger ledger = Ledger(m_GM.GetProfile(playerId).m_sIdentity);
		int now = RZ_SeasonClock.Now();
		string place = PlaceKey(p);
		foreach (RZ_ErrandDef d : m_aDefs)
		{
			if (d.m_bCard || d.m_bDuty || !Anchored(d, p))
				continue;

			if (ledger.CoolUntil(d.m_sKey) > now || ledger.CoolUntil(d.m_sKey + "|" + place) > now || PlaceCooling(d.m_sKey, place))
				continue;

			if ((d.m_bTimed && !RZ_GameTime.IsCalibrated()) || !d.ForWhom(this, playerId, p))
				continue;

			RZ_Errand e = NewErrand(d, playerId, p, p.m_vPos);
			if (d.Why(this, e) && d.Still(this, e) && (e.m_sSubject.IsEmpty() || SubjectFree(e)) && AnyWay(e))
				return d;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! A safehouse board's strip of asks (Missions > Errands > Board asks): up to two lines, signed
	//! with the house's name, apart from the players' notes. Only what's true for the reader now.
	void BoardAsks(int playerId, int house, notnull array<string> result)
	{
		if (!IsOn() || !m_Settings.m_bBoardAsks)
			return;

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses || houses.IsExiled(playerId))
			return;

		array<string> seen = {};
		for (int i = 0; i < RZ_NamedPeople.HOUSE_PEOPLE && result.Count() < 2; i++)
		{
			RZ_NamedPerson p = m_People.FromHouse(house, i);
			if (!p)
				continue;

			RZ_ErrandDef d = DryDeal(playerId, p);
			if (!d || seen.Contains(d.m_sKey))
				continue;

			seen.Insert(d.m_sKey);
			result.Insert(string.Format("%1. Ask %2, the %3.", d.m_sTitle, p.m_sName, p.m_sRole));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Radio asks (Missions > Errands > Radio asks): every 30 s one player, round-robin. Near a house
	//! (300 m to 1.5 km) where something is true for them, and inside every rule of an unasked offer
	//! and the interruption budget, one personal line: "If you're near Levie, Mara at the stove could
	//! use a hand." Never broadcast, so it can't bait an ambush.
	protected void RadioAsks(array<int> ids, array<IEntity> players)
	{
		if (!m_Settings.m_bRadioAsks || ids.IsEmpty())
			return;

		m_iRadioCursor = (m_iRadioCursor + 1) % ids.Count();
		int playerId = ids[m_iRadioCursor];
		IEntity me = players[m_iRadioCursor];
		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		float last;
		if (m_mLastRadio.Find(ident, last) && m_fClock - last < m_Settings.m_fRadioAskMinutes * 60)
			return;

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses || houses.IsExiled(playerId) || !MayInterrupt(playerId, me))
			return;

		int house = houses.FindNearest(me.GetOrigin(), 1500);
		RZ_Safehouse h = houses.Get(house);
		if (!h || RZ_Util.DistXZ(h.m_vPos, me.GetOrigin()) < 300)
			return;

		for (int i = 0; i < RZ_NamedPeople.HOUSE_PEOPLE; i++)
		{
			RZ_NamedPerson p = m_People.FromHouse(house, i);
			if (!p || !DryDeal(playerId, p))
				continue;

			RZ_NoticeLanes lanes = RZ_NoticeLanes.Get();
			if (lanes && !lanes.TryContact(playerId, RZ_EContact.ERRAND))
				return;

			m_mLastRadio.Set(ident, m_fClock);
			m_GM.NotifyPlayer(playerId, "On the Haven's channel", string.Format("If you're near %1, %2 the %3 could use a hand. Only if you're passing.", h.m_sName, p.m_sName, p.m_sRole));
			RZ_Log.Info(string.Format("Errands: radio ask to %1 for %2 at %3", RZ_Util.PlayerName(playerId), p.m_sName, h.m_sName));
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A named person with a body within 30 m of `pos`.
	protected RZ_NamedPerson CallerNear(vector pos)
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
		{
			int h = houses.FindNearest(pos, CALL_RANGE + 20);
			if (h >= 0)
			{
				array<ref RZ_NamedPerson> here = {};
				m_People.ShownAt(RZ_ENamedPlace.HOUSE, h, here);
				if (!here.IsEmpty())
					return here.GetRandomElement();
			}
		}

		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			RZ_Holding hold = territory.FindHolding(-1, pos, CALL_RANGE + 20);
			if (hold)
			{
				array<ref RZ_NamedPerson> there = {};
				m_People.ShownAt(RZ_ENamedPlace.HOLDING, hold.m_iId, there);
				if (!there.IsEmpty())
					return there.GetRandomElement();
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The rules every unasked offer must pass (docs/ERRANDS.md, "The dealer").
	protected bool MayInterrupt(int playerId, IEntity me)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		if (profile.m_fLifeSeconds < 180 || profile.m_iInfectionStage >= RZ_EInfectionStage.FINAL)
			return false;

		// Nobody waves over a player whose opening's chain is still running (Guidance/RZ_Openings.c)
		if (RZ_Openings.IsRunningFor(playerId))
			return false;

		// A first life finding the Haven: nobody waves them over until 10 minutes after they get there
		if (profile.m_iObjType == RZ_EObjective.FIND_HAVEN)
		{
			if (!profile.m_bObjDone)
				return false;

			float found;
			if (!m_mHavenFound.Find(profile.m_sIdentity, found))
			{
				m_mHavenFound.Set(profile.m_sIdentity, m_fClock);
				return false;
			}

			if (m_fClock - found < FIRST_LIFE_WAIT)
				return false;
		}

		vector at = me.GetOrigin();
		if (ShotNear(at, 60, 60) || m_GM.GetSpawner().CountZombiesNear(at, 30, 1) > 0 || RZ_Util.IsInVehicle(me))
			return false;

		if (CountHeld(playerId) >= m_Settings.m_iMaxHeld || HasOpenOffer(playerId))
			return false;

		float last;
		if (m_mLastUnasked.Find(profile.m_sIdentity, last) && m_fClock - last < m_Settings.m_fOfferGapMinutes * 60)
			return false;

		return QuietHere(at);
	}

	//------------------------------------------------------------------------------------------------
	//! Not within 300 m of a running mission's spot, 600 m of the set piece holding the evening,
	//! 1 km of the lab during the final stand, or 2 km of the Long Table's farm.
	bool QuietHere(vector at)
	{
		foreach (RZ_Mission m : m_GM.GetMissions().GetActive())
		{
			if (m.m_eState == RZ_EMissionState.ACTIVE && !m.m_bGlobal && RZ_Util.DistXZ(m.m_vPos, at) <= 300)
				return false;
		}

		RZ_Shared shared = m_GM.GetShared();
		if (shared && shared.GetCalendar().IsQuietAt(at))
			return false;

		if (m_GM.HasLab() && m_GM.GetCure() && m_GM.GetCure().IsFinalStand() && RZ_Util.DistXZ(m_GM.GetLabPosition(), at) <= 1000)
			return false;

		RZ_LongTable table = m_GM.GetLongTable();
		if (table && table.Exists() && table.IsNearHouse(at, 2000))
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The town log's season marks have no prop without the prefab: their line goes to whoever
	//! passes (an approach line, once per place per session).
	protected void MarkLines(array<int> ids, array<IEntity> players)
	{
		RZ_Marks marks = RZ_Marks.Get();
		RZ_NoticeLanes lanes = RZ_NoticeLanes.Get();
		if (!marks || !lanes)
			return;

		foreach (int i, IEntity p : players)
		{
			string line = marks.TextNear(p.GetOrigin(), 40);
			if (!line.IsEmpty() && lanes.TryAmbient(ids[i], "mark:" + line))
				m_GM.NotifyPlayer(ids[i], "By the road", line);
		}
	}

	// ================================================================ Items
	//------------------------------------------------------------------------------------------------
	protected static void Keywords(RZ_EErrandItem item, notnull array<string> result)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		switch (item)
		{
			case RZ_EErrandItem.FOOD:
			{
				if (gm && gm.GetSafehouses())
					result.InsertAll(gm.GetSafehouses().GetFoodKeywords());
				else
					RZ_FoodHelper.DefaultKeywords(result);

				break;
			}

			case RZ_EErrandItem.FUEL: result.Insert("jerrycan"); result.Insert("canister"); result.Insert("fuelcan"); break;
			case RZ_EErrandItem.BATTERY: result.Insert("battery"); break;
			case RZ_EErrandItem.MEAT: result.Insert("meatleg"); break;
			case RZ_EErrandItem.MEDICAL: result.Insert("bandage"); result.Insert("dressing"); result.Insert("gauze"); result.Insert("aspirin"); result.Insert("medkit"); result.Insert("morphine"); result.Insert("tourniquet"); result.Insert("salin"); result.Insert("pill"); result.Insert("antibiotic"); break;
			case RZ_EErrandItem.ANTIDOTE:
			{
				if (gm && !gm.GetAntidotePrefab().IsEmpty())
				{
					// The prefab's file name, as the inventory reads it ("{GUID}Prefabs/.../Antidote_01.et")
					string res = gm.GetAntidotePrefab();
					res.ToLower();
					int slash = res.LastIndexOf("/");
					string name = res.Substring(slash + 1, res.Length() - slash - 1);
					name.Replace(".et", "");
					if (!name.IsEmpty())
						result.Insert(name);
				}

				result.Insert("antidote");
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! How many items of this kind the character carries (by prefab name; mission items never count).
	static int CountItems(IEntity character, RZ_EErrandItem item)
	{
		if (!character || item == RZ_EErrandItem.NONE)
			return 0;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return 0;

		array<string> words = {};
		Keywords(item, words);
		array<IEntity> items = {};
		inv.GetItems(items);
		int n = 0;
		foreach (IEntity it : items)
		{
			if (it && !it.FindComponent(RZ_MissionItemComponent) && RZ_FoodHelper.IsFood(it, words))
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Takes up to `count`; returns how many were taken.
	static int TakeItems(IEntity character, RZ_EErrandItem item, int count)
	{
		if (!character || item == RZ_EErrandItem.NONE || count <= 0)
			return 0;

		array<string> words = {};
		Keywords(item, words);
		return RZ_FoodHelper.TakeFood(character, words, count);
	}

	// ================================================================ Speaking
	//------------------------------------------------------------------------------------------------
	protected static string Pick(array<string> lines)
	{
		if (!lines || lines.IsEmpty())
			return "";

		return lines.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! The giver says a line to the asker (owner-only; the talk screen catches it under their title).
	//! asNotice: a card, for when the asker may be far away (overtaken, late).
	protected void Say(RZ_Errand e, string line, bool asNotice)
	{
		if (line.IsEmpty())
			return;

		string text = e.Fill(line);
		if (asNotice)
			m_GM.NotifyPlayer(e.m_iAsker, e.m_sGiverTitle, text);
		else
			m_GM.Say(e.m_iAsker, e.m_sGiverTitle, e.m_iFaction, text);
	}

	//------------------------------------------------------------------------------------------------
	//! A system line to the asker about this errand (a card's later consequence).
	void Notify(RZ_Errand e, string text)
	{
		m_GM.NotifyPlayer(e.m_iAsker, "Errand: " + e.Fill(e.m_Def.m_sTitle), e.Fill(text));
	}

	//------------------------------------------------------------------------------------------------
	//! Where the giver is to be found now, for "go back": a wanderer where they walk, else where it
	//! was asked (TickSteps counts you back there; the tracker points there).
	vector GiverAt(RZ_Errand e)
	{
		vector giverAt = e.m_vGiver;

		// Their body where it stands now, when it's out: the point is the person, not their spot
		if (m_People)
		{
			IEntity body = m_People.BodyOf(e.m_sGiver);
			if (body)
				return body.GetOrigin();
		}

		vector walking;
		if (e.m_iWanderer >= 0 && m_GM.GetWanderers() && m_GM.GetWanderers().GetPosition(e.m_iWanderer, walking))
			giverAt = walking;

		return giverAt;
	}

	//------------------------------------------------------------------------------------------------
	RZ_Errand FindById(int id)
	{
		foreach (RZ_Errand e : m_aErrands)
		{
			if (e.m_iId == id)
				return e;
		}

		return null;
	}

	// ================================================================ Handed errands
	//------------------------------------------------------------------------------------------------
	//! An errand another system hands a player, taken already (an opening's chain step,
	//! Guidance/RZ_Openings.c): its definition's first way, its step at `place` ("{place}" in the
	//! step's text is `placeText`), its giver someone outside the named people who stands at
	//! `giverAt`. The giver's take line, the step's notice and the tracker, as for any taken errand.
	//! Null with errands off, or for a definition that isn't handed or has no way.
	RZ_Errand Hand(RZ_ErrandDef d, int playerId, string giverKey, string giverName, string giverTitle, int faction, vector giverAt, vector place, string placeText, IEntity user)
	{
		if (!IsOn() || !d || !d.m_bHanded || d.m_aWays.IsEmpty() || !user)
			return null;

		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		RZ_ErrandWay w = d.m_aWays[0];
		RZ_Errand e = new RZ_Errand();
		e.m_Def = d;
		e.m_iAsker = playerId;
		e.m_sIdent = profile.m_sIdentity;
		e.m_iLife = profile.m_iLives;
		e.m_sGiver = giverKey;
		e.m_sGiverName = giverName;
		e.m_sGiverTitle = giverTitle;
		e.m_iFaction = faction;
		if (!RZ_Factions.IsValid(e.m_iFaction))
			e.m_iFaction = RZ_EFaction.SURVIVORS;

		e.m_vGiver = giverAt;
		e.m_vOffer = giverAt;
		e.m_iSettlement = m_GM.FindNearestSettlement(giverAt);
		e.SetText("giver", giverName);
		RZ_Settlement town = m_GM.GetSettlement(e.m_iSettlement);
		if (town)
			e.SetText("here", town.m_sName);

		e.SetText("place", placeText);
		e.m_iId = m_iNextId;
		m_iNextId++;
		e.m_fOfferedAt = m_fClock;
		e.m_iWay = 0;
		e.m_fTakenAt = m_fClock;
		e.m_bStepDone = !w.HasStep();
		e.m_eState = RZ_EErrandState.HELD;
		e.m_vPlace = place;
		m_aErrands.Insert(e);
		Count(d.m_sFamily).deals++;
		PlaceProp(e, w);

		Say(e, Pick(w.m_aTake), false);
		if (w.HasStep())
			RZ_JobCards.Started(m_GM, playerId, "Errand", e.Fill(d.m_sTitle), GiverWords(e), e.Fill(w.StepText()) + "\n" + RZ_Util.Capitalize(RZ_Util.DirectionText(user.GetOrigin(), place)) + ".");

		RZ_Log.Info(string.Format("Errands: %1 handed to %2 (%3)", d.m_sKey, RZ_Util.PlayerName(playerId), giverName));
		return e;
	}

	//------------------------------------------------------------------------------------------------
	//! A handed step finished there and then by a reply the player picked (an opening's hand-over):
	//! its thing taken from them and the errand resolved now, inside that conversation, never later
	//! for being near. False: it isn't held, or the thing isn't on them (nothing changes).
	bool FinishNow(RZ_Errand e, IEntity me)
	{
		if (!e || !me || e.m_eState != RZ_EErrandState.HELD)
			return false;

		RZ_ErrandWay w = e.GetWay();
		if (!w)
			return false;

		if (w.m_bGiveThere && w.m_iItems > 0)
		{
			if (CountItems(me, w.m_eItem) < w.m_iItems)
				return false;

			TakeItems(me, w.m_eItem, w.m_iItems);
		}

		e.m_bStepDone = true;
		e.m_bTold = false;
		RemoveProp(e);
		Resolve(e, w);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A handed errand its owner ends (its giver died, the chain moved on): quietly, no pay.
	void Drop(RZ_Errand e, string why)
	{
		if (e && e.m_Def && e.m_Def.m_bHanded)
			End(e, why);
	}

	// ================================================================ The journal
	//------------------------------------------------------------------------------------------------
	//! This player's errands, for the journal (RZ_UIFeed): the giver as "by", the exact place, the
	//! real minutes left, and what you said you'd do. Never on the map.
	void ForJournal(int playerId, notnull array<RZ_Errand> result)
	{
		foreach (RZ_Errand e : m_aErrands)
		{
			if (e.m_iAsker == playerId && e.m_eState == RZ_EErrandState.HELD)
				result.Insert(e);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Real minutes before a held errand lapses.
	int MinutesLeft(RZ_Errand e)
	{
		return Math.Max(0, RZ_Util.RoundToInt((LapseSeconds(e) - (m_fClock - e.m_fTakenAt)) / 60));
	}

	//------------------------------------------------------------------------------------------------
	//! Real seconds a held errand waits: its definition's own (an opening's step), else Lapse minutes.
	protected float LapseSeconds(RZ_Errand e)
	{
		if (e.m_Def && e.m_Def.m_fLapseMinutes > 0)
			return e.m_Def.m_fLapseMinutes * 60;

		return m_Settings.m_fLapseMinutes * 60;
	}

	//------------------------------------------------------------------------------------------------
	//! What the journal says you're doing.
	string JournalText(RZ_Errand e)
	{
		RZ_ErrandWay w = e.GetWay();
		if (!w)
			return "";

		if (e.m_bStepDone && !w.m_bEndsThere)
			return e.Fill("Done. Go back to {giver} at {here}.");

		// A sorting under way: where to take the one you're leading, or the rules (RZ_DutyGames.NowLine)
		string now = m_Games.NowLine(e.m_iId);
		if (!now.IsEmpty())
			return e.Fill(w.StepText()) + "\n" + now;

		return e.Fill(w.StepText());
	}

	// ================================================================ Telemetry and the admin view
	//------------------------------------------------------------------------------------------------
	RZ_ErrandStats Count(string family)
	{
		RZ_ErrandStats s;
		if (!m_mStats.Find(family, s))
		{
			s = new RZ_ErrandStats();
			m_mStats.Set(family, s);
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! Every 30 minutes (Missions > Errands > Telemetry): deals, declines, lapses, overtakes, ways
	//! taken, Favor and Fame paid, per family.
	protected void LogTelemetry()
	{
		if (m_mStats.IsEmpty())
			return;

		string text = string.Format("Errands | live %1 | asked %2, called out %3 | ", m_aErrands.Count(), m_iAskedDeals, m_iUnaskedDeals);
		foreach (string family, RZ_ErrandStats s : m_mStats)
		{
			string ways = "";
			foreach (string way, int n : s.ways)
			{
				ways += string.Format(" %1 %2", way, n);
			}

			text += string.Format("%1: dealt %2, declined %3, lapsed %4, overtaken %5, done %6, Favor %7, Fame %8 (amends %9), ways", family, s.deals, s.declines, s.lapses, s.overtakes,
				s.done, s.favor, s.fame, s.amends) + ways + " | ";
		}

		RZ_Log.Info(text + m_Strangers.Telemetry() + " | " + LieTelemetry());
	}

	//------------------------------------------------------------------------------------------------
	//! Errands' lies since the last line: told (new in the store), and gone (found out, or buried with
	//! the life); and the twists dealt so far.
	protected string LieTelemetry()
	{
		array<string> now = {};
		RZ_WorldFacts facts = m_GM.GetFacts();
		if (facts)
		{
			array<ref RZ_Lie> lies = {};
			facts.Memory().Lies().Save(lies);
			foreach (RZ_Lie l : lies)
			{
				if (l.key.StartsWith("errand."))
					now.Insert(l.ident + "|" + l.key + "|" + l.subject);
			}
		}

		int told = 0;
		foreach (string lie : now)
		{
			if (!m_aLiesSeen.Contains(lie))
				told++;
		}

		int gone = 0;
		foreach (string seen : m_aLiesSeen)
		{
			if (!now.Contains(seen))
				gone++;
		}

		m_aLiesSeen.Clear();
		m_aLiesSeen.InsertAll(now);
		return string.Format("lies: told %1, found or buried %2, open %3 | twists dealt %4", told, gone, now.Count(), m_iTwists);
	}

	//------------------------------------------------------------------------------------------------
	//! The admin view (no typing: a reply on the talk screen for server admins, and the status log):
	//! the nearest player's last deal and why the rest weren't dealt, their errands, the live count.
	string AdminText(int adminId, vector at)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		int nearest = adminId;
		float best = float.MAX;
		foreach (int i, IEntity p : players)
		{
			if (ids[i] == adminId)
				continue;

			float d = RZ_Util.DistXZ(p.GetOrigin(), at);
			if (d < best)
			{
				best = d;
				nearest = ids[i];
			}
		}

		string why;
		m_mLastWhy.Find(nearest, why);
		string text = string.Format("Errands %1: %2 live, asked %3, called out %4.\n%5: ", OnOff(IsOn()), m_aErrands.Count(), m_iAskedDeals, m_iUnaskedDeals, RZ_Util.PlayerName(nearest));
		if (why.IsEmpty())
			text += "nothing dealt yet.";
		else
			text += why;

		foreach (RZ_Errand e : m_aErrands)
		{
			if (e.m_iAsker == nearest)
				text += string.Format("\n%1 from %2 (%3)", e.m_Def.m_sTitle, e.m_sGiverName, StateWord(e));
		}

		// Where a stranger could find someone standing here now (decision 14), and the locals
		text += string.Format("\nStranger spots here: %1. Strangers out: %2. Locals: %3.", m_Strangers.SpotsAt(at, nearest), m_Strangers.LiveCount(), m_Locals.Count());

		RZ_Log.Info("Errands admin view: " + text);
		return text;
	}

	//------------------------------------------------------------------------------------------------
	protected static string StateWord(RZ_Errand e)
	{
		if (e.m_eState == RZ_EErrandState.OFFERED)
			return "offered";

		if (e.m_bStepDone)
			return "done, not reported";

		return "held";
	}
}
