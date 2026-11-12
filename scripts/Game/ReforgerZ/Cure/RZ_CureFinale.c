//! The cure finale's controller (docs/FINALE.md): the fight for control of the cure at 90 percent.
//!
//! Research stops at the 90 percent gate while the five route questlines are registered
//! (Missions/Questlines/Finale/). This class holds everything the routes share, saved with the
//! season:
//! - six tracks: one per route (the Haven, Bastion, the Red Hand, Halcyon, Independent), 10 points
//!   each for the last tenth of the cure, and the lab-alone track the built fallback feeds;
//! - locks: one route per identity for the season (Commit, RouteOf), active locks (2 hours played,
//!   online in the last 2 real days), price doors, and who may decide open and sworn work;
//! - the finish (the first route to 10): the route's capped Fame chunk, the holder, the frozen race,
//!   the batch hold (12 real hours and 2 hours of play), the seal calls and the seal;
//! - pacts (tallies of active locks), the deal at the gate, fighting routes, the trays, live tissue
//!   (Act III, through the tissue missions) and Last Mile's reports;
//! - the stand as a control fight: the clean room, sides and counts, take times, overtime, Take the
//!   Lab's holder change, the Coat Run, the stand's hostility bits and Fame;
//! - the controller, how it was won, the terms and the deciders (finale.* season flags), and the
//!   cure handed over to the season's Fame leader when nobody holds the room.
//! Read on the server tick. Nothing polls per frame and nothing moves on an empty server.

//------------------------------------------------------------------------------------------------
enum RZ_EFinaleContest
{
	AUTO_MODE,
	FORCE_MODE,
	TALLY_MODE
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Director > Finale. Attribute defaults only.
[BaseContainerProps()]
class RZ_FinaleSettings
{
	[Attribute("1", desc: "The cure finale on: research stops at 90 percent and five routes race to finish the batch, then the stand decides who controls the cure (docs/FINALE.md). Needs Questlines on. Off: research runs to 100 and the cure goes to the faction with the most season Fame")]
	bool m_bEnabled;

	[Attribute("0.25", desc: "Route worth: the finisher's Fame chunk is this share of the leading faction's season Fame when research first reached 90")]
	float m_fRouteWorth;

	[Attribute("150", desc: "The Fame chunk is never less than this")]
	int m_iFloor;

	[Attribute("0", desc: "The Fame chunk is never more than this (0: no ceiling)")]
	int m_iCeiling;

	[Attribute("1.1", desc: "Soft line: the chunk counts in full until the finisher's faction reaches this many times the best other faction's season Fame; above it, half")]
	float m_fSoftLine;

	[Attribute("12", desc: "Batch hold: real hours after the finish before the batch is ready (both this and the play hours; x the questlines' wait scale)")]
	float m_fBatchRealHours;

	[Attribute("2", desc: "Batch hold: hours of play after the finish before the batch is ready")]
	float m_fBatchPlayHours;

	[Attribute("60", desc: "Minutes of warning before the seal (calls at 60, 30 and 10)")]
	int m_iSealCallMinutes;

	[Attribute("-1", desc: "Seal hour: the seal waits for the first time this server hour comes round after the batch is ready (-1: off)")]
	int m_iSealHour;

	[Attribute("90", desc: "Take time in seconds for the clean room at track 0")]
	float m_fTakeSeconds;

	[Attribute("5", desc: "Seconds off the take time per point on the side's frozen track")]
	float m_fTakePerPoint;

	[Attribute("45", desc: "The take time is never under this (and the finisher retakes in this)")]
	float m_fTakeFloor;

	[Attribute("180", desc: "Overtime: at most this many seconds after the stand's 15 minutes while a challenger is counting")]
	float m_fOvertimeSeconds;

	[Attribute("10", desc: "Clean room radius (m)")]
	float m_fRoomRadius;

	[Attribute("120", desc: "Take the Lab's hold at the loading bay under a finale (seconds)")]
	float m_fBaySeconds;

	[Attribute("0", UIWidgets.ComboBox, "Contest: Auto (Force while the server lets players hurt each other, Tally when it doesn't), Force (only side inside counts up) or Tally (most counted players inside counts up; groups never target players)", "", ParamEnumArray.FromEnum(RZ_EFinaleContest))]
	RZ_EFinaleContest m_eContest;

	[Attribute("1", desc: "The Coat Run (the Independent's once-a-season re-label during another route's batch hold)")]
	bool m_bCoatRun;

	[Attribute("1", desc: "Take the Lab under a finale (the Red Hand route's once-a-season holder change)")]
	bool m_bTakeTheLab;

	[Attribute("1", desc: "Pacts between routes")]
	bool m_bPacts;

	[Attribute("2", desc: "At most this many friendly faction groups at the lab during the stand")]
	int m_iGroups;

	[Attribute("2", desc: "Fight memory: real days two routes stay fighting after a hostile act")]
	float m_fFightDays;

	[Attribute("2", desc: "Active lock: online in the last this many real days")]
	float m_fActiveDays;

	[Attribute("2", desc: "Active lock: at least this many hours played this season (x the questlines' wait scale)")]
	float m_fActiveHours;

	[Attribute("1", desc: "Fame leader rule: when the season's Fame leader isn't the controller, its rules apply at half next season (after a deal, the partner's instead)")]
	bool m_bFameLeaderRule;
}

//------------------------------------------------------------------------------------------------
class RZ_FinaleSave
{
	bool routesOpen;
	bool pitched;
	int gateUnix;
	int worth;
	ref array<float> tracks = {};
	ref array<int> state = {};
	ref array<int> act1 = {};
	ref array<int> tissue = {};
	ref array<int> tissueUnix = {};
	ref array<int> lastMile = {};
	float labTrack;
	int finisher = -2;
	int holder = -2;
	int partner = -1;
	int finishUnix;
	float batchPlay;
	int readyUnix;
	int sealUnix;
	int calls;
	bool batchReady;
	bool remaking;						// a lost stand: only a remake step or the backlog makes the batch
	bool isSealed;
	ref array<string> lockIdent = {};
	ref array<int> lockRoute = {};
	ref array<int> lockUnix = {};
	ref array<string> playIdent = {};
	ref array<float> playSecs = {};
	ref array<int> seenUnix = {};
	ref array<int> pactWith = {};
	ref array<int> propTo = {};
	ref array<int> propUnix = {};
	ref array<string> votes = {};
	ref array<int> fight = {};
	ref array<string> priceKeys = {};
	int trays = 3;
	int trayUnix;
	ref array<int> trayOut = {};
	ref array<int> lockSeen = {};		// the last real time each route had an active lock
	int cameMask;						// routes that counted toward the room at a stand (finale.came)
	bool coatUsed;
	bool takeUsed;
	bool coatPassed;
	int controller = -2;
	int how;
	int terms;
	int condition;
	ref array<string> deciders = {};
	ref array<string> holdIdent = {};
	ref array<float> labSecs = {};
	ref array<float> roomSecs = {};
	ref array<float> counts = {};
	ref array<int> standsLost = {};
	ref array<string> payKeys = {};
	ref array<int> payValues = {};
	ref array<string> shotOwn = {};
	float roomX;
	float roomZ;
	float bayX;
	float bayZ;
	ref RZ_FinaleItemsSave items;
	ref RZ_FinalePeopleSave people;
	ref RZ_FinaleAssetsSave assets;
	ref RZ_FinaleRestSave rest;
}

//------------------------------------------------------------------------------------------------
class RZ_CureFinale
{
	// Routes: the faction index for the four faction routes, 4 for the Independent
	static const int HAVEN = 0;
	static const int BASTION = 1;
	static const int REDHAND = 2;
	static const int HALCYON = 3;
	static const int INDEP = 4;
	static const int ROUTES = 5;
	static const int NONE = -2;		// no route (no finisher, no holder)
	static const int LAB = -1;		// the lab alone (the slow finish)

	// finale.how
	static const int HOW_HELD = 1;
	static const int HOW_FORCE = 2;
	static const int HOW_DEAL = 3;
	static const int HOW_BETRAYAL = 4;
	static const int HOW_THEFT = 5;
	static const int HOW_HANDED = 6;

	// Pay caps (docs/FINALE.md, "Personal pay and caps")
	static const int CAP_FAME = 50;			// per faction per player, over all finale content
	static const int CAP_INFAMY = 40;		// per faction per player per route
	static const int CAP_FAVOR = 60;
	static const int KILL_INFAMY = 3;		// killing a locked player outside the stand
	static const int KILL_INFAMY_HOUR = 12;

	static const float STAND_RADIUS = 1500;	// hostility bits and the disguise rule around the lab
	static const float LAB_RADIUS = 150;	// at the lab (the built stand's radius)
	static const float BAY_GUARD = 15;
	static const float DECIDE_RADIUS = 30;

	protected static const int DAY = 86400;
	protected static const int HOUR = 3600;
	protected static const float CHECK_SECONDS = 5;
	protected static const float MILE_SECONDS = 600;
	protected static const int PACT_SECONDS = 86400;

	protected static RZ_CureFinale s_Instance;

	protected RZ_GameModeComponent m_GM;
	protected RZ_FinaleSettings m_Settings;
	protected ref RZ_FinaleSave m_S = new RZ_FinaleSave();
	protected ref RZ_FinaleItems m_Items;
	protected ref RZ_FinalePeople m_People;
	protected ref RZ_FinaleAssets m_Assets;
	protected ref RZ_FinaleRest m_Rest;

	protected float m_fCheck;
	protected float m_fMile;
	protected float m_fOvertime;
	protected bool m_bInOvertime;
	protected bool m_bSealing;
	protected bool m_bStandOn;
	protected float m_fStandLine;
	protected float m_fStandHud;
	protected int m_iLastCount;			// whole seconds of the leading count last told
	protected ref array<IEntity> m_aGroups = {};		// the friendly groups at the stand (never saved)
	protected ref array<int> m_aGroupFactions = {};		// and their factions
	protected bool m_bStandPrep;						// the stand's groups and Take the Lab are due
	protected ref array<int> m_aDesks = {-1, -1, -1, -1, -1};	// each route's desk mission
	protected int m_iDeskTry;							// real time of the next desk and tissue hunt tries
	protected int m_iTissueTry;
	protected ref map<string, int> m_mKillHour = new map<string, int>();
	protected ref map<string, int> m_mKillUsed = new map<string, int>();

	//------------------------------------------------------------------------------------------------
	void RZ_CureFinale(RZ_GameModeComponent gm, RZ_FinaleSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		s_Instance = this;
		m_Items = new RZ_FinaleItems(gm, this);
		m_People = new RZ_FinalePeople(gm, this);
		m_Assets = new RZ_FinaleAssets(gm);
		m_Rest = new RZ_FinaleRest(gm, this);
		Pad();
	}

	//------------------------------------------------------------------------------------------------
	static RZ_CureFinale Get()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//! The finale runs: Finale on and Questlines on.
	static bool IsOn()
	{
		if (!s_Instance || !s_Instance.m_Settings || !s_Instance.m_Settings.m_bEnabled)
			return false;

		RZ_Questlines q = s_Instance.m_GM.GetQuestlines();
		return q && q.IsEnabled();
	}

	//------------------------------------------------------------------------------------------------
	RZ_FinaleSettings GetSettings() { return m_Settings; }
	RZ_FinaleItems GetItems() { return m_Items; }
	RZ_FinalePeople GetPeople() { return m_People; }
	RZ_FinaleAssets GetAssets() { return m_Assets; }
	RZ_FinaleRest GetRest() { return m_Rest; }

	//------------------------------------------------------------------------------------------------
	protected void Pad()
	{
		while (m_S.tracks.Count() < ROUTES) { m_S.tracks.Insert(0); }
		while (m_S.state.Count() < ROUTES) { m_S.state.Insert(0); }
		while (m_S.act1.Count() < ROUTES) { m_S.act1.Insert(0); }
		while (m_S.tissue.Count() < ROUTES) { m_S.tissue.Insert(0); }
		while (m_S.tissueUnix.Count() < ROUTES) { m_S.tissueUnix.Insert(0); }
		while (m_S.lastMile.Count() < ROUTES) { m_S.lastMile.Insert(0); }
		while (m_S.pactWith.Count() < ROUTES) { m_S.pactWith.Insert(-1); }
		while (m_S.propTo.Count() < ROUTES) { m_S.propTo.Insert(-1); }
		while (m_S.propUnix.Count() < ROUTES) { m_S.propUnix.Insert(0); }
		while (m_S.fight.Count() < ROUTES * ROUTES) { m_S.fight.Insert(0); }
		while (m_S.trayOut.Count() < ROUTES) { m_S.trayOut.Insert(0); }
		while (m_S.counts.Count() < ROUTES) { m_S.counts.Insert(0); }
		while (m_S.lockSeen.Count() < ROUTES) { m_S.lockSeen.Insert(0); }
	}

	// ================================================================ Names
	//------------------------------------------------------------------------------------------------
	//! "the Haven", "Bastion", "the Red Hand", "Halcyon", "the Independent route".
	static string RouteName(int r)
	{
		if (r == INDEP)
			return "the Independent route";

		if (r == LAB)
			return "the lab alone";

		if (r >= 0 && r < INDEP)
			return RZ_Factions.GetNameInText(r);

		return "nobody";
	}

	//------------------------------------------------------------------------------------------------
	//! The questline key of a route.
	static string RouteKey(int r)
	{
		switch (r)
		{
			case HAVEN: return "route_haven";
			case BASTION: return "route_bastion";
			case REDHAND: return "route_redhand";
			case HALCYON: return "route_halcyon";
			case INDEP: return "route_one_each";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A route as conversations write it: haven, bastion, redhand, halcyon, indep. NONE otherwise.
	static int RouteOfWord(string word)
	{
		switch (word)
		{
			case "haven": return HAVEN;
			case "bastion": return BASTION;
			case "redhand": return REDHAND;
			case "halcyon": return HALCYON;
			case "indep": return INDEP;
		}

		return NONE;
	}

	//------------------------------------------------------------------------------------------------
	//! The dialogue condition route:<word> (none, haven, bastion, redhand, halcyon, indep): the
	//! player's lock this season.
	static bool RouteTerm(string word, int playerId)
	{
		int mine = NONE;
		if (s_Instance && IsOn())
			mine = s_Instance.RouteOfPlayer(playerId);

		if (word == "none")
			return mine == NONE;

		return mine != NONE && mine == RouteOfWord(word);
	}

	//------------------------------------------------------------------------------------------------
	//! The dialogue condition finale:<what>:
	//!   soon    the finale is on, research is 75 or more and the gate isn't reached ("Ask me again")
	//!   pitch   the finale is on, research is 80 or more, and the cure isn't out (the pitches)
	//!   open    the routes are open (the commit topics)
	//!   locked  this player is written down for a route
	//!   voice   this wanderer speaks for the Independent route: the Doctor, else the Courier
	static bool FinaleTerm(string what, int playerId, RZ_Wanderer w)
	{
		if (!s_Instance || !IsOn())
			return false;

		RZ_CureResearch cure = s_Instance.m_GM.GetCure();
		float pct = 0;
		if (cure)
			pct = cure.ShownProgress();

		switch (what)
		{
			case "soon": return !s_Instance.m_S.routesOpen && !s_Instance.IsCured() && pct >= 75;
			case "pitch": return !s_Instance.IsCured() && pct >= 80;
			case "open": return s_Instance.IsOpen();
			case "locked": return s_Instance.RouteOfPlayer(playerId) != NONE;
			case "voice": return s_Instance.IsIndependentVoice(w);
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The Independent's voice among the wanderers: the Doctor while one lives, else the Courier
	//! (with neither, the unsigned sheet at the lab gate: the desk).
	bool IsIndependentVoice(RZ_Wanderer w)
	{
		if (!w || !m_GM.GetWanderers())
			return false;

		array<int> doctors = {};
		m_GM.GetWanderers().GetLiving(RZ_Wanderers.RoleBit(RZ_EWandererRole.DOCTOR), doctors);
		if (w.m_eRole == RZ_EWandererRole.DOCTOR)
			return true;

		return doctors.IsEmpty() && w.m_eRole == RZ_EWandererRole.COURIER;
	}

	//------------------------------------------------------------------------------------------------
	//! True when the Independent has a voice on the road (the desk then takes no names).
	bool HasIndependentVoice()
	{
		if (!m_GM.GetWanderers())
			return false;

		array<int> voices = {};
		m_GM.GetWanderers().GetLiving(RZ_Wanderers.RoleBit(RZ_EWandererRole.DOCTOR) | RZ_Wanderers.RoleBit(RZ_EWandererRole.COURIER), voices);
		return !voices.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! The route a questline key belongs to, NONE if it isn't one.
	static int RouteOfKey(string key)
	{
		for (int r = 0; r < ROUTES; r++)
		{
			if (RouteKey(r) == key)
				return r;
		}

		return NONE;
	}

	//------------------------------------------------------------------------------------------------
	//! A route as a season flag or record value: route + 1 (5 the Independent), 0 for nobody or the
	//! lab alone.
	static int RouteFlag(int r)
	{
		if (r < 0 || r >= ROUTES)
			return 0;

		return r + 1;
	}

	//------------------------------------------------------------------------------------------------
	//! The route's faction, -1 for the Independent.
	static int RouteFaction(int r)
	{
		if (r >= 0 && r < INDEP)
			return r;

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Whose standing a route's pay and Infamy go to: the Haven for the Independent.
	static int PayFaction(int r)
	{
		if (r == INDEP)
			return RZ_EFaction.SURVIVORS;

		return RouteFaction(r);
	}

	//------------------------------------------------------------------------------------------------
	//! The route's admission, repeated at the lock.
	static string Admission(int r)
	{
		switch (r)
		{
			case HAVEN: return "\"The houses decide who goes first, and the houses kept the List.\"";
			case BASTION: return "\"It reaches those who comply, and the register is a census.\"";
			case REDHAND: return "\"The price is the queue. The cut dose was sold the same way.\"";
			case HALCYON: return "\"The paper starts on Day 1. The three weeks are on page one, or not at all.\"";
			case INDEP: return "\"Nobody guards it and nobody counts it, and every faction remembers who took it from them.\"";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The route's best argument (its pitch, before the admission).
	static string PitchLine(int r)
	{
		switch (r)
		{
			case HAVEN: return "\"The cure goes hand to hand, to the people the others would leave till last.\"";
			case BASTION: return "\"Order: a queue, a register, nobody trampled at the gate.\"";
			case REDHAND: return "\"It reaches anyone who can pay, including people nobody else would serve.\"";
			case HALCYON: return "\"Published, fast, checked, with every author named.\"";
			case INDEP: return "\"Four lots of people are about to tell you the cure is theirs. It isn't anyone's yet.\"";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! At 85 percent each route's pitch goes on air once, naming nobody: the four on their factions'
	//! open channels, the Independent's as an unsigned voice on the open channel.
	protected void AirPitches()
	{
		m_S.pitched = true;
		RZ_FactionLife life = m_GM.GetFactionLife();
		for (int r = 0; r < INDEP; r++)
		{
			if (life)
				life.OpenChannelLine(r, PitchLine(r) + " " + Admission(r));
		}

		m_GM.NotifyAllAs("Open channel", "A voice nobody signs for: " + PitchLine(INDEP) + " " + Admission(INDEP), RZ_ENoticeTag.EVENT);
		RZ_Log.Info("Finale: the routes' pitches went on air at 85 percent.");
	}

	//------------------------------------------------------------------------------------------------
	//! What locked players are called on top of their word ("attached"...).
	static string LockWord(int r)
	{
		switch (r)
		{
			case HAVEN: return "one of the houses' own";
			case BASTION: return "attached";
			case REDHAND: return "the other crew";
			case HALCYON: return "an uncontrolled variable";
			case INDEP: return "nobody's";
		}

		return "";
	}

	// ================================================================ Save
	//------------------------------------------------------------------------------------------------
	void Restore(RZ_FinaleSave data)
	{
		if (!data)
			return;

		m_S = data;
		Pad();
		m_Items.Restore(data.items);
		m_People.Restore(data.people);
		m_Assets.Restore(data.assets);
		m_Rest.Restore(data.rest);
		m_S.items = null;
		m_S.people = null;
		m_S.assets = null;
		m_S.rest = null;
	}

	//------------------------------------------------------------------------------------------------
	RZ_FinaleSave Save()
	{
		m_S.items = m_Items.Save();
		m_S.people = m_People.Save();
		m_S.assets = m_Assets.Save();
		m_S.rest = m_Rest.Save();
		return m_S;
	}

	// ================================================================ The gate
	//------------------------------------------------------------------------------------------------
	//! The routes are open: research reached the gate with the finale on, and the cure isn't out.
	bool IsOpen()
	{
		return IsOn() && m_S.routesOpen && !IsCured();
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsCured()
	{
		return m_GM.GetCure() && m_GM.GetCure().IsCured();
	}

	//------------------------------------------------------------------------------------------------
	//! Research reached the gate (RZ_CureResearch): the routes open, once a season. The Fame chunk is
	//! fixed now, from the leading faction's season Fame.
	void OnGate()
	{
		if (!IsOn() || m_S.routesOpen)
			return;

		m_S.routesOpen = true;
		m_S.gateUnix = RZ_SeasonClock.Now();
		int best = 0;
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			best = Math.MaxInt(best, m_GM.GetSeasonFame(f));
		}

		int worth = Math.MaxInt(m_Settings.m_iFloor, RZ_Util.RoundToInt(best * m_Settings.m_fRouteWorth));
		if (m_Settings.m_iCeiling > 0)
			worth = Math.MinInt(worth, m_Settings.m_iCeiling);

		m_S.worth = worth;
		m_S.trays = 3;
		m_S.trayUnix = m_S.gateUnix;
		m_People.Seed();
		RZ_WorldFacts.Get().SetFlag("finale.open", 1);
		RZ_Log.Info(string.Format("Finale: the gate. Five routes open; the route's worth is %1 season Fame (leader %2).", worth, best));

		// The lab's own line, then each route's opening (the routes' OnBegin say theirs)
		if (m_GM.GetFactionLife())
			m_GM.GetFactionLife().OpenChannelLine(RZ_EFaction.INSTITUTE, "Ninety. It won't go further on what we have. There's more than one way to finish it, and most of them aren't in this building.");
	}

	//------------------------------------------------------------------------------------------------
	//! The route's worth shown on the finale page.
	int Worth()
	{
		return m_S.worth;
	}

	// ================================================================ Tracks
	//------------------------------------------------------------------------------------------------
	float Track(int r)
	{
		if (r == LAB)
			return m_S.labTrack;

		if (r < 0 || r >= ROUTES)
			return 0;

		return m_S.tracks[r];
	}

	//------------------------------------------------------------------------------------------------
	//! A route's points (a step done or something handed in; never taken back). A track never passes
	//! 9 before its last step (last = true), which finishes it at 10. Ignored once the race is frozen.
	void AddStep(int r, float points, string why, bool last = false)
	{
		if (r < 0 || r >= ROUTES || points <= 0 || IsFrozen() || IsCured() || !m_S.routesOpen)
			return;

		float cap = 9;
		if (last)
			cap = 10;

		float before = m_S.tracks[r];
		m_S.tracks[r] = Math.Min(cap, before + points);
		if (m_S.state[r] == 0)
			m_S.state[r] = 1;

		RZ_Log.Info(string.Format("Finale: %1 +%2 (%3), track %4.", RouteName(r), RZ_Util.RoundToInt(points), why, RZ_Util.RoundToInt(m_S.tracks[r])));
		if (before < 8 && m_S.tracks[r] >= 8)
			LastMileOpens(r);

		if (last && m_S.tracks[r] >= 10)
			Finish(r);

		m_GM.OnCureChanged();
	}

	//------------------------------------------------------------------------------------------------
	//! Act I is done for a route (its tissue act can be offered from now).
	void MarkActOne(int r)
	{
		if (r >= 0 && r < ROUTES)
			m_S.act1[r] = 1;
	}

	//------------------------------------------------------------------------------------------------
	bool ActOneDone(int r)
	{
		return r >= 0 && r < ROUTES && m_S.act1[r] != 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The lab-alone track (the built fallback while a finale is registered): `points` from the
	//! fallback's percent a day. At 10 the lab makes the batch on its own work.
	void AddLabAlone(float points)
	{
		if (points <= 0 || IsFrozen() || IsCured() || !IsOn())
			return;

		float before = m_S.labTrack;
		m_S.labTrack = Math.Min(10, m_S.labTrack + points);
		if (RZ_Util.RoundToInt(before) != RZ_Util.RoundToInt(m_S.labTrack))
			RZ_Log.Info(string.Format("Finale: the lab alone, track %1.", RZ_Util.RoundToInt(m_S.labTrack)));

		if (m_S.labTrack >= 10)
			Finish(LAB);
	}

	//------------------------------------------------------------------------------------------------
	//! The cure percent shown while the finale runs: 90 plus the furthest track, at most 99 until the
	//! seal; after a lost stand, until the remake or the backlog makes the batch, the lab's own
	//! research from 90 (the backlog's progress). -1 when the finale isn't showing (the lab's own
	//! percent stands).
	float ShownPercent()
	{
		if (!IsOpen() || m_S.isSealed)
			return -1;

		if (m_S.remaking && !m_S.batchReady)
		{
			RZ_CureResearch cure = m_GM.GetCure();
			float lab = 90;
			if (cure)
				lab = cure.m_fProgress;

			return Math.Clamp(lab, 90, 99);
		}

		float best = m_S.labTrack;
		for (int r = 0; r < ROUTES; r++)
		{
			best = Math.Max(best, m_S.tracks[r]);
		}

		return Math.Min(99, 90 + best);
	}

	//------------------------------------------------------------------------------------------------
	//! The lab's readout at a change of shift: the tracks by faction, never by player.
	string Readout()
	{
		string s = string.Format("%1.", NumberWord(RZ_Util.RoundToInt(ShownPercent())));
		for (int r = 0; r < INDEP; r++)
		{
			s += string.Format(" %1's work is at %2.", RZ_Util.Capitalize(RZ_Factions.GetNameInText(r)), RZ_WorldFacts.NumberWords(RZ_Util.RoundToInt(m_S.tracks[r])));
		}

		s += string.Format(" Nobody's is at %1.", RZ_WorldFacts.NumberWords(RZ_Util.RoundToInt(m_S.tracks[INDEP])));
		if (m_S.labTrack > 0)
			s += string.Format(" The lab's own: %1.", RZ_WorldFacts.NumberWords(RZ_Util.RoundToInt(m_S.labTrack)));

		return s;
	}

	//------------------------------------------------------------------------------------------------
	protected static string NumberWord(int n)
	{
		string w = RZ_WorldFacts.NumberWords(n);
		if (w.IsEmpty())
			return n.ToString();

		string first = w.Substring(0, 1);
		first.ToUpper();
		return first + w.Substring(1, w.Length() - 1);
	}

	// ================================================================ Locks
	//------------------------------------------------------------------------------------------------
	protected string Ident(int playerId)
	{
		if (playerId <= 0)
			return "";

		return m_GM.GetProfile(playerId).m_sIdentity;
	}

	//------------------------------------------------------------------------------------------------
	//! The route an identity locked to, NONE if none.
	int RouteOf(string ident)
	{
		int idx = m_S.lockIdent.Find(ident);
		if (idx < 0 || ident.IsEmpty())
			return NONE;

		return m_S.lockRoute[idx];
	}

	//------------------------------------------------------------------------------------------------
	int RouteOfPlayer(int playerId)
	{
		return RouteOf(Ident(playerId));
	}

	//------------------------------------------------------------------------------------------------
	//! Active lock: 2+ hours played this season (x wait scale) and online in the last 2 real days.
	bool IsActive(string ident)
	{
		if (RouteOf(ident) == NONE)
			return false;

		int idx = m_S.playIdent.Find(ident);
		if (idx < 0)
			return false;

		float need = m_Settings.m_fActiveHours * HOUR * WaitScale();
		if (m_S.playSecs[idx] < need)
			return false;

		return RZ_SeasonClock.Now() - m_S.seenUnix[idx] <= m_Settings.m_fActiveDays * DAY;
	}

	//------------------------------------------------------------------------------------------------
	//! Counts at this stand: an active lock made before the batch was ready, and not barred for
	//! shooting its own side.
	bool CountsAtStand(string ident)
	{
		int idx = m_S.lockIdent.Find(ident);
		if (idx < 0 || !IsActive(ident) || m_S.shotOwn.Contains(ident))
			return false;

		return m_S.readyUnix <= 0 || m_S.lockUnix[idx] <= m_S.readyUnix;
	}

	//------------------------------------------------------------------------------------------------
	//! Active locks on a route.
	int ActiveLocks(int r)
	{
		int n = 0;
		foreach (int i, string ident : m_S.lockIdent)
		{
			if (m_S.lockRoute[i] == r && IsActive(ident))
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! "" if this player may lock to route r now, else why not (in the route's voice).
	string CanCommit(int playerId, int r)
	{
		if (!IsOn() || r < 0 || r >= ROUTES)
			return "\"Nothing to sign.\"";

		if (!m_S.routesOpen)
			return "\"Ask me again when it's nearly done.\"";

		if (IsCured())
			return "\"It's over. Talk to whoever you like.\"";

		int mine = RouteOfPlayer(playerId);
		if (mine == r)
			return "\"You're already written down.\"";

		if (mine != NONE)
			return "\"You've picked your side. This isn't it.\"";

		if (r == INDEP)
			return "";

		if (!CanSideWith(playerId, r))
			return "\"Not you. Not without paying the price for it first.\"";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! CanSideWith (the one rule; docs/FINALE.md, the lock): yes unless Hunted; the Hunted by parley (owed) or
	//! by the route's price door. The Independent: always.
	bool CanSideWith(int playerId, int r)
	{
		if (r == INDEP || !RZ_Reputation.IsHunted(m_GM.GetTier(playerId, r)) || PaidPrice(playerId, r))
			return true;

		RZ_Checkpoints checkpoints = m_GM.GetCheckpoints();
		return checkpoints && checkpoints.GetParley() && checkpoints.GetParley().IsOwed(playerId, r);
	}

	//------------------------------------------------------------------------------------------------
	//! A price door is paid (it opens the route only; Infamy stays).
	void PayPrice(int playerId, int r)
	{
		string key = string.Format("%1|%2", Ident(playerId), r);
		if (!m_S.priceKeys.Contains(key))
			m_S.priceKeys.Insert(key);
	}

	//------------------------------------------------------------------------------------------------
	bool PaidPrice(int playerId, int r)
	{
		return m_S.priceKeys.Contains(string.Format("%1|%2", Ident(playerId), r));
	}

	//------------------------------------------------------------------------------------------------
	//! The three plain lines of the lock card: the admission, what you bring, what you leave.
	string LockCard(int playerId, int r)
	{
		string bring = m_Items.CarriedText(Ident(playerId), r);
		string assets = m_Assets.Text(Ident(playerId), r);
		if (!assets.IsEmpty() && !bring.IsEmpty())
			bring += ", ";

		bring += assets;
		if (bring.IsEmpty())
			bring = "nothing yet but yourself";

		string s = string.Format("From now until the cure, you stand with %1. %2", RouteName(r), Admission(r));
		s += "\nYou bring: " + bring + ".";
		s += "\nYou leave: every other route's work. What you know stays yours. Dying doesn't change it.";
		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! Lock this player to route r for the season. False (and why, told) if they can't.
	bool Commit(int playerId, int r)
	{
		string why = CanCommit(playerId, r);
		if (!why.IsEmpty())
		{
			m_GM.NotifyPlayer(playerId, RouteTitle(r), why);
			return false;
		}

		string ident = Ident(playerId);
		m_S.lockIdent.Insert(ident);
		m_S.lockRoute.Insert(r);
		m_S.lockUnix.Insert(RZ_SeasonClock.Now());
		if (m_S.state[r] == 0)
			m_S.state[r] = 1;

		TrackPlayer(ident, 0);
		m_GM.NotifyPlayerAs(playerId, "Written down", string.Format("You're written down for %1 until the cure. Other routes' work will refuse you. At the stand you count only while you keep playing.", RouteName(r)), RZ_ENoticeTag.HIGH);
		if (m_GM.GetHostility())
			m_GM.GetHostility().OnStandingChanged(playerId);

		RZ_Log.Info(string.Format("Finale: %1 locked: %2 (active %3).", RouteName(r), CountLocks(r), ActiveLocks(r)));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	int CountLocks(int r)
	{
		int n = 0;
		foreach (int lr : m_S.lockRoute)
		{
			if (lr == r)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! "" if this player may decide (or help with) route r's work: sworn work only for its locked
	//! players; open work for anyone not locked elsewhere who can side with its faction (anyone at
	//! all for the Independent). Else the refusal.
	string MayDecide(int playerId, int r, bool sworn)
	{
		int mine = RouteOfPlayer(playerId);
		if (mine != NONE && mine != r)
			return "\"You've picked your side. This isn't it.\"";

		if (sworn && mine != r)
			return "\"This is for the ones who signed. Sign first.\"";

		if (mine == r || CanSideWith(playerId, r))
			return "";

		return "\"Not from you.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! A route with no active lock for 3 real days (x the wait scale), counted from the gate: its
	//! sworn stages pause and its claims are let go, until a lock is active again.
	bool RoutePaused(int r)
	{
		if (r < 0 || r >= ROUTES || !m_S.routesOpen || ActiveLocks(r) > 0)
			return false;

		int last = Math.MaxInt(m_S.lockSeen[r], m_S.gateUnix);
		return RZ_SeasonClock.Now() - last >= 3 * DAY * WaitScale();
	}

	//------------------------------------------------------------------------------------------------
	//! The route questline's title ("The One Who Didn't Turn"...).
	static string RouteTitle(int r)
	{
		switch (r)
		{
			case HAVEN: return "The One Who Didn't Turn";
			case BASTION: return "Held for Collection";
			case REDHAND: return "The Cook";
			case HALCYON: return "The Lower Store";
			case INDEP: return "One Each";
		}

		return "The cure";
	}

	// ================================================================ Play tracking
	//------------------------------------------------------------------------------------------------
	protected void TrackPlayer(string ident, float dt)
	{
		if (ident.IsEmpty())
			return;

		int idx = m_S.playIdent.Find(ident);
		if (idx < 0)
		{
			m_S.playIdent.Insert(ident);
			m_S.playSecs.Insert(0);
			m_S.seenUnix.Insert(0);
			idx = m_S.playIdent.Count() - 1;
		}

		m_S.playSecs[idx] = m_S.playSecs[idx] + dt;
		m_S.seenUnix[idx] = RZ_SeasonClock.Now();
	}

	//------------------------------------------------------------------------------------------------
	protected float WaitScale()
	{
		if (!m_GM.GetQuestlines())
			return 1;

		return m_GM.GetQuestlines().WaitScale();
	}

	// ================================================================ The tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!IsOn() || ids.IsEmpty())
			return;

		// Season play per identity (active locks count from here, also before the gate)
		foreach (int pid : ids)
		{
			TrackPlayer(Ident(pid), dt);
		}

		// A save from before the finale, already at the gate: the routes open now
		RZ_CureResearch cure = m_GM.GetCure();
		if (!m_S.pitched && !m_S.routesOpen && cure && !IsCured() && cure.ShownProgress() >= 85)
			AirPitches();

		if (!m_S.routesOpen && cure && cure.IsAtFinaleGate())
			OnGate();

		// The rest (seeds before the gate; the readout, marks, drains and the checkpoint after it)
		m_Rest.Tick(dt, ids, players);

		if (!m_S.routesOpen || IsCured())
			return;

		m_Items.Tick(dt, ids, players);
		m_People.Tick(dt, ids, players);
		if (m_S.finishUnix > 0 && !m_S.batchReady)
			m_S.batchPlay = m_S.batchPlay + dt;

		m_fCheck += dt;
		if (m_fCheck < CHECK_SECONDS)
			return;

		m_fCheck = 0;
		TickDesks();
		TickTrays();
		TickTissue();
		TickPacts();
		TickBatch();
		m_fMile += CHECK_SECONDS;
		if (m_fMile >= MILE_SECONDS * WaitScale())
		{
			m_fMile = 0;
			LastMileReports(ids);
		}
	}

	// ================================================================ The desks
	//------------------------------------------------------------------------------------------------
	//! Each route's desk (the commit node's fallback, RZ_Mission_FinaleDesk) runs from the gate until
	//! the stand is won: started again if it ended.
	protected void TickDesks()
	{
		RZ_MissionManager missions = m_GM.GetMissions();
		int now = RZ_SeasonClock.Now();
		if (!missions || now < m_iDeskTry)
			return;

		m_iDeskTry = now + 60;
		for (int seen = 0; seen < ROUTES; seen++)
		{
			if (ActiveLocks(seen) > 0)
				m_S.lockSeen[seen] = now;
		}

		for (int r = 0; r < ROUTES; r++)
		{
			if (m_aDesks[r] >= 0 && missions.FindById(m_aDesks[r]))
				continue;

			RZ_Mission_FinaleDesk desk = new RZ_Mission_FinaleDesk();
			desk.m_iRoute = r;
			desk.Configure(m_GM);
			if (missions.WhyNot(desk, 1, false).IsEmpty() && missions.StartMission(desk))
				m_aDesks[r] = desk.m_iId;
		}
	}

	// ================================================================ The trays
	//------------------------------------------------------------------------------------------------
	//! The lab's three trays of the 90 percent batch: one is rebuilt each real day.
	protected void TickTrays()
	{
		int now = RZ_SeasonClock.Now();
		if (m_S.trays >= 3)
		{
			m_S.trayUnix = now;
			return;
		}

		if (now - m_S.trayUnix >= DAY * WaitScale())
		{
			m_S.trays = m_S.trays + 1;
			m_S.trayUnix = now;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A tray leaves the lab for route r. False if none is in, or the route already has one out
	//! (the director refuses a second).
	bool TakeTray(int r)
	{
		if (r < 0 || r >= ROUTES || m_S.trays <= 0 || m_S.trayOut[r] > 0)
			return false;

		m_S.trays = m_S.trays - 1;
		m_S.trayOut[r] = 1;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool HasTrayOut(int r)
	{
		return r >= 0 && r < ROUTES && m_S.trayOut[r] > 0;
	}

	//------------------------------------------------------------------------------------------------
	int TraysIn()
	{
		return m_S.trays;
	}

	//------------------------------------------------------------------------------------------------
	//! The finish needs a tray in the lab (the Red Hand's case brings its own).
	bool TrayForFinish(int r)
	{
		return r == REDHAND || m_S.trays > 0;
	}

	// ================================================================ Live tissue (Act III)
	//------------------------------------------------------------------------------------------------
	//! 0 none, 1 the lab is working on it, 2 in.
	int TissueState(int r)
	{
		if (r < 0 || r >= ROUTES)
			return 0;

		return m_S.tissue[r];
	}

	//------------------------------------------------------------------------------------------------
	bool TissueIn(int r)
	{
		return TissueState(r) == 2;
	}

	//------------------------------------------------------------------------------------------------
	//! A tissue hunt handed in: the deliverer's own route if they're locked to one that still needs
	//! it, else the route the hunt was offered for. leader: a leader taken alive (+2 at once); else
	//! the lab works on it for 12 real hours first (a lone one, guards, specials).
	void OnTissue(int offeredFor, int playerId, bool leader, bool great = false)
	{
		int r = offeredFor;
		int mine = RouteOfPlayer(playerId);
		if (mine >= 0 && mine < ROUTES && m_S.tissue[mine] == 0)
			r = mine;
		else if (mine == NONE && playerId > 0)
			r = m_Rest.UnsidedChoice(playerId, offeredFor);

		if (r < 0 || r >= ROUTES || m_S.tissue[r] != 0)
			return;

		if (leader)
		{
			m_S.tissue[r] = 2;
			if (great)
				AddStep(r, 3, "live tissue, the great one's leader");
			else
				AddStep(r, 2, "live tissue, a leader");
		}
		else
		{
			m_S.tissue[r] = 1;
			m_S.tissueUnix[r] = RZ_SeasonClock.Now();
			RZ_Log.Info(string.Format("Finale: %1's tissue is in the lab's isolation room; 12 real hours of work.", RouteName(r)));
		}

		m_GM.NotifyPlayer(playerId, RouteTitle(r), string.Format("The case goes into the isolation room in %1's name.", RouteName(r)));
		if (playerId > 0)
		{
			int f = PayFaction(r);
			int fame = 10;
			if (r == INDEP)
				fame = 0;

			PayIdent(Ident(playerId), f, fame, 20, 0, "live tissue");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Tissue hunts for routes whose Act I is done and whose tissue isn't in (most waited first), and
	//! the lab's work on tissue handed in. No tissue hunt can run (no prefabs): the lab grows it from
	//! its own samples after a real day.
	protected void TickTissue()
	{
		int now = RZ_SeasonClock.Now();
		int want = NONE;
		for (int r = 0; r < ROUTES; r++)
		{
			if (m_S.tissue[r] == 1 && now - m_S.tissueUnix[r] >= 12 * HOUR * WaitScale())
			{
				m_S.tissue[r] = 2;
				AddStep(r, 2, "live tissue, after the lab's work");
				continue;
			}

			if (m_S.tissue[r] == 0 && m_S.act1[r] != 0 && want == NONE)
				want = r;
		}

		if (want == NONE || IsFrozen())
			return;

		if (m_S.tissueUnix[want] == 0)
			m_S.tissueUnix[want] = now;

		// A hunt is tried at most every 2 minutes (a Setup that misses spawns and clears its drop)
		RZ_MissionManager missions = m_GM.GetMissions();
		if (!missions || now < m_iTissueTry || missions.IsTypeRunning(RZ_QM_RouteTissueIndex) || missions.IsTypeRunning(RZ_QM_RouteTissueLone))
			return;

		m_iTissueTry = now + 120;

		RZ_QM_RouteTissueIndex index = new RZ_QM_RouteTissueIndex();
		index.m_iRoute = want;
		index.Configure(m_GM);
		if (missions.StartIfCan(index, 1))
			return;

		RZ_QM_RouteTissueLone lone = new RZ_QM_RouteTissueLone();
		lone.m_iRoute = want;
		lone.Configure(m_GM);
		if (missions.StartIfCan(lone, 1))
			return;

		// Nothing to hunt with on this server: the lab grows it from the backlog after a real day
		if (now - m_S.tissueUnix[want] >= DAY * WaitScale())
		{
			m_S.tissue[want] = 2;
			AddStep(want, 2, "tissue grown from the lab's own samples");
		}
	}

	// ================================================================ The finish and the batch
	//------------------------------------------------------------------------------------------------
	//! A finisher exists: the race is frozen.
	bool IsFrozen()
	{
		return m_S.finisher != NONE;
	}

	//------------------------------------------------------------------------------------------------
	int Finisher() { return m_S.finisher; }
	int Holder() { return m_S.holder; }
	bool IsBatchReady() { return m_S.batchReady; }

	//------------------------------------------------------------------------------------------------
	//! A route's last step landed (or the lab alone reached 10): the batch is made from it.
	void Finish(int r)
	{
		if (IsFrozen() || IsCured())
			return;

		m_S.finisher = r;
		m_S.finishUnix = RZ_SeasonClock.Now();
		m_S.batchPlay = 0;
		if (r >= 0 && r != REDHAND && m_S.trays > 0)
			m_S.trays = m_S.trays - 1;

		if (r >= 0)
		{
			m_S.state[r] = 3;
			m_S.holder = r;
			if (r != INDEP)
				PayChunk(RouteFaction(r), m_S.worth, RouteName(r));
		}
		else
		{
			m_S.holder = NONE;
			RZ_WorldFacts.Get().SetFlag("cure.slow", 1);
		}

		RZ_WorldFacts.Get().SetFlag("finale.finisher", RouteFlag(r));
		float hours = m_Settings.m_fBatchRealHours * WaitScale();
		RZ_Log.Info(string.Format("Finale: %1 finished. Batch hold %2 real hours and %3 hours of play.", RouteName(r), hours, m_Settings.m_fBatchPlayHours * WaitScale()));
		if (r >= 0)
			m_GM.NotifyAllAs("The lab", string.Format("The batch is being made from %1's work. It needs about half a day in the cold before it can be sealed.", RouteName(r)), RZ_ENoticeTag.HIGH);
		else
			m_GM.NotifyAllAs("The lab", "The lab has finished the batch on its own work. Nobody's name is on it. It needs about half a day in the cold before it can be sealed.", RZ_ENoticeTag.HIGH);

		// The race ends: the other routes' running work is called off, with no pay and no penalty
		if (m_GM.GetQuestlines())
			m_GM.GetQuestlines().FreezeRoutes(RouteKey(r));

		m_GM.OnCureChanged();
	}

	//------------------------------------------------------------------------------------------------
	//! The batch hold, the seal calls and the seal.
	protected void TickBatch()
	{
		if (!IsFrozen() || m_S.isSealed)
			return;

		int now = RZ_SeasonClock.Now();
		if (!m_S.batchReady)
		{
			// After a lost stand the hold doesn't run again: OnRemake or OnBacklogBatch
			if (m_S.remaking)
				return;

			bool realOver = now - m_S.finishUnix >= m_Settings.m_fBatchRealHours * HOUR * WaitScale();
			bool playOver = m_S.batchPlay >= m_Settings.m_fBatchPlayHours * HOUR * WaitScale();
			if (!realOver || !playOver)
				return;

			BatchReady("the batch hold");
			return;
		}

		if (m_S.sealUnix <= 0)
		{
			if (!SealHourNow())
				return;

			m_S.sealUnix = now + m_Settings.m_iSealCallMinutes * 60;
			m_S.calls = 0;
		}

		int left = m_S.sealUnix - now;
		SealCall(left, 60, 1, "The batch seals in an hour.");
		SealCall(left, 30, 2, "The batch seals in half an hour.");
		SealCall(left, 10, 4, "The batch seals in ten minutes.");
		if (left <= 0)
			Seal();
	}

	//------------------------------------------------------------------------------------------------
	protected void SealCall(int left, int minutes, int bit, string text)
	{
		if ((m_S.calls & bit) != 0 || left > minutes * 60 || minutes > m_Settings.m_iSealCallMinutes)
			return;

		m_S.calls = m_S.calls | bit;
		m_GM.NotifyAllAs("The lab", text, RZ_ENoticeTag.HIGH);
		if (m_GM.GetFactionLife())
			m_GM.GetFactionLife().OpenChannelLine(RZ_EFaction.INSTITUTE, text);
	}

	//------------------------------------------------------------------------------------------------
	//! Seal hour: the seal calls wait for this server hour (or start at once with none set).
	protected bool SealHourNow()
	{
		if (m_Settings.m_iSealHour < 0 || !RZ_GameTime.IsCalibrated())
			return true;

		int h = RZ_Util.RoundToInt(Math.Floor(RZ_GameTime.GetHour()));
		return h == m_Settings.m_iSealHour;
	}

	//------------------------------------------------------------------------------------------------
	//! The batch is ready (after the hold, a remake step or the lab's backlog after a lost stand).
	void BatchReady(string how)
	{
		if (m_S.batchReady)
			return;

		m_S.batchReady = true;
		m_S.remaking = false;
		m_S.readyUnix = RZ_SeasonClock.Now();
		m_S.sealUnix = 0;
		RZ_Log.Info(string.Format("Finale: the batch is ready (%1). Holder %2.", how, RouteName(m_S.holder)));
	}

	//------------------------------------------------------------------------------------------------
	//! The lab's backlog reached 100 after a lost stand (RZ_CureResearch): the batch is ready, then
	//! the seal. Without a finisher the lab alone made it.
	void OnBacklogBatch()
	{
		if (!IsFrozen())
		{
			m_S.finisher = LAB;
			m_S.finishUnix = RZ_SeasonClock.Now();
			m_S.holder = NONE;
		}

		BatchReady("the lab's backlog");
	}

	//------------------------------------------------------------------------------------------------
	//! A remake step after a lost stand is done (the finisher's route).
	void OnRemake(int r)
	{
		if (r != m_S.finisher || m_S.batchReady)
			return;

		BatchReady("the remake");
	}

	//------------------------------------------------------------------------------------------------
	//! The seal: research to 100 in one AddFinaleResearch call, and the stand starts (once someone
	//! reaches the lab). The finisher starts as holder again after a lost stand.
	protected void Seal()
	{
		m_S.isSealed = true;
		RZ_CureResearch cure = m_GM.GetCure();
		if (!cure)
			return;

		if (m_S.finisher >= 0 && m_S.finisher != INDEP && !m_S.coatPassed)
			m_S.holder = m_S.finisher;

		if (m_S.coatPassed)
		{
			m_S.holder = INDEP;
			m_GM.NotifyAllAs("The lab", "The duty officer at the seal: \"The seal's been changed. It says 'nobody'. Witnessed: me.\"", RZ_ENoticeTag.HIGH);
		}

		m_bSealing = true;
		cure.Seal(RouteFaction(m_S.finisher));
		m_bSealing = false;
	}

	//------------------------------------------------------------------------------------------------
	//! The cure's milestone 4 waits for the seal while a finale runs (RZ_CureResearch.OnMilestone).
	bool HoldsTheStand()
	{
		return IsOn() && m_S.routesOpen && !m_bSealing && !m_S.isSealed;
	}

	// ================================================================ The stand
	//------------------------------------------------------------------------------------------------
	//! The stand (re)started: the counts, the room and the groups.
	void OnStandStarted()
	{
		m_bStandOn = m_S.routesOpen;
		m_bInOvertime = false;
		m_fOvertime = 0;
		m_iLastCount = -1;
		for (int r = 0; r < ROUTES; r++)
		{
			m_S.counts[r] = 0;
		}

		// The groups and Take the Lab come on the stand's first tick (not while a save is loading)
		m_bStandPrep = true;

		string holder = "nobody";
		if (m_S.holder >= 0)
			holder = RouteName(m_S.holder);

		RZ_Log.Info(string.Format("Finale: the stand starts. Clean room: %1.", holder));
	}

	//------------------------------------------------------------------------------------------------
	//! The clean room's point: a land point 15-25 m from the lab, the same all season. A named child
	//! entity on the lab prefab is the planned point (docs/FINALE.md).
	vector RoomPos()
	{
		if (m_S.roomX == 0 && m_S.roomZ == 0 && m_GM.HasLab())
		{
			// The clean room: a child entity named RZ_CleanRoom on the lab prefab, else a land point
			vector p = ChildPos(m_GM.GetLabEntity(), "RZ_CleanRoom");
			if (p == vector.Zero)
				p = RZ_Mission.LandPointNear(m_GM.GetLabPosition(), 15, 25);

			m_S.roomX = p[0];
			m_S.roomZ = p[2];
		}

		return RZ_Util.GroundPos(Vector(m_S.roomX, 0, m_S.roomZ));
	}

	//------------------------------------------------------------------------------------------------
	//! The loading bay: 25-40 m from the clean room, on the side of the lab away from the room.
	vector BayPos()
	{
		if (m_S.bayX == 0 && m_S.bayZ == 0 && m_GM.HasLab())
		{
			vector lab = m_GM.GetLabPosition();
			vector away = lab - RoomPos();
			away[1] = 0;
			float len = away.Length();
			// The loading bay: a child entity named RZ_LoadingBay on the lab prefab, else away from the room
			vector p = ChildPos(m_GM.GetLabEntity(), "RZ_LoadingBay");
			if (p == vector.Zero && len > 1)
				p = lab + away * (25 / len);
			else if (p == vector.Zero)
				p = RZ_Mission.LandPointNear(lab, 25, 40);

			p = RZ_Util.GroundPos(p);
			m_S.bayX = p[0];
			m_S.bayZ = p[2];
		}

		return RZ_Util.GroundPos(Vector(m_S.bayX, 0, m_S.bayZ));
	}

	//------------------------------------------------------------------------------------------------
	//! A named child entity's position under `parent` (two levels deep), vector.Zero if none.
	// VERIFY: the lab prefab's named children (RZ_CleanRoom, RZ_LoadingBay) come back by GetName()
	// through GetChildren/GetSibling on the server
	protected static vector ChildPos(IEntity parent, string name)
	{
		if (!parent)
			return vector.Zero;

		IEntity child = parent.GetChildren();
		while (child)
		{
			if (child.GetName() == name)
				return child.GetOrigin();

			IEntity grand = child.GetChildren();
			while (grand)
			{
				if (grand.GetName() == name)
					return grand.GetOrigin();

				grand = grand.GetSibling();
			}

			child = child.GetSibling();
		}

		return vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	//! Down: dead, or unconscious (an unconscious player doesn't count in the clean room).
	// VERIFY: CharacterControllerComponent.IsUnconscious() is true on the server for a downed player
	protected static bool IsDown(IEntity p)
	{
		if (!RZ_Util.IsCharacterAlive(p))
			return true;

		ChimeraCharacter c = ChimeraCharacter.Cast(p);
		if (!c || !c.GetCharacterController())
			return false;

		return c.GetCharacterController().IsUnconscious();
	}

	//------------------------------------------------------------------------------------------------
	//! The side a route stands with: itself, or the lower route of its pact.
	int SideOf(int r)
	{
		if (r < 0 || r >= ROUTES)
			return NONE;

		int p = m_S.pactWith[r];
		if (p >= 0 && p < r)
			return p;

		return r;
	}

	//------------------------------------------------------------------------------------------------
	//! A side's take time: Take seconds minus Take per point on its frozen track (a pact uses the
	//! higher track), never under the floor. The finisher retakes in the floor.
	float TakeTime(int side)
	{
		float track = Track(side);
		for (int r = 0; r < ROUTES; r++)
		{
			if (SideOf(r) == side)
				track = Math.Max(track, Track(r));
		}

		if (SideOf(m_S.finisher) == side && m_S.finisher >= 0)
			return m_Settings.m_fTakeFloor;

		return Math.Max(m_Settings.m_fTakeFloor, m_Settings.m_fTakeSeconds - m_Settings.m_fTakePerPoint * track);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsTally()
	{
		if (m_Settings.m_eContest == RZ_EFinaleContest.AUTO_MODE)
			return !PlayerDamageOn();

		return m_Settings.m_eContest == RZ_EFinaleContest.TALLY_MODE;
	}

	//------------------------------------------------------------------------------------------------
	//! Contest Auto reads the server's player damage from play: once a player has killed another
	//! this season (`finale.pvp_seen`, OnPlayerKilled), players can hurt each other and it plays as
	//! Force; until then, as Tally (which also works with damage on).
	protected bool PlayerDamageOn()
	{
		return RZ_WorldFacts.Get().Flag("finale.pvp_seen") != 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Every stand tick (RZ_CureResearch.TickFinalStand, players online): who is in the clean room,
	//! the counts, takes, and time at the lab and in the room.
	void StandTick(float dt, array<IEntity> players)
	{
		if (!IsOn() || !m_S.routesOpen)
			return;

		if (m_bStandPrep)
		{
			m_bStandPrep = false;
			RoomPos();
			BayPos();
			if (!m_S.takeUsed && m_Settings.m_bTakeTheLab)
				OfferTakeTheLab();

			SpawnGroups();
		}

		vector lab = m_GM.GetLabPosition();
		vector room = RoomPos();
		array<int> inside = {0, 0, 0, 0, 0};
		foreach (IEntity p : players)
		{
			int pid = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(p);
			string ident = Ident(pid);
			int r = RouteOf(ident);
			float d = RZ_Util.DistXZ(p.GetOrigin(), lab);
			if (d <= LAB_RADIUS)
				AddStandTime(ident, dt, false);

			if (r == NONE || !CountsAtStand(ident) || IsDown(p))
				continue;

			if (RZ_Util.DistXZ(p.GetOrigin(), room) > m_Settings.m_fRoomRadius)
				continue;

			AddStandTime(ident, dt, true);
			int side = SideOf(r);
			inside[side] = inside[side] + 1;
		}

		int challenger = NONE;
		int sidesIn = 0;
		int best = 0;
		bool tie = false;
		for (int s = 0; s < ROUTES; s++)
		{
			if (inside[s] <= 0)
				continue;

			sidesIn++;
			if (inside[s] > best)
			{
				best = inside[s];
				challenger = s;
				tie = false;
			}
			else if (inside[s] == best)
			{
				tie = true;
			}
		}

		// Force: only the one side inside counts up. Tally: the side with the most counted players.
		int counting = NONE;
		if (IsTally())
		{
			if (!tie)
				counting = challenger;
		}
		else if (sidesIn == 1)
		{
			counting = challenger;
		}

		int holderSide = SideOf(m_S.holder);
		if (counting == holderSide)
			counting = NONE;

		for (int c = 0; c < ROUTES; c++)
		{
			if (c == counting)
				m_S.counts[c] = m_S.counts[c] + dt;
			else
				m_S.counts[c] = Math.Max(0, m_S.counts[c] - dt);
		}

		if (counting != NONE)
			m_S.cameMask = m_S.cameMask | (1 << counting);

		if (counting != NONE && m_S.counts[counting] >= TakeTime(counting))
			TakeRoom(counting, HOW_FORCE);

		TellStandLine(players, dt);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddStandTime(string ident, float dt, bool room)
	{
		if (ident.IsEmpty())
			return;

		int idx = m_S.holdIdent.Find(ident);
		if (idx < 0)
		{
			m_S.holdIdent.Insert(ident);
			m_S.labSecs.Insert(0);
			m_S.roomSecs.Insert(0);
			idx = m_S.holdIdent.Count() - 1;
		}

		if (room)
			m_S.roomSecs[idx] = m_S.roomSecs[idx] + dt;
		else
			m_S.labSecs[idx] = m_S.labSecs[idx] + dt;
	}

	//------------------------------------------------------------------------------------------------
	//! The clean room changes hands.
	void TakeRoom(int side, int how)
	{
		int before = m_S.holder;
		m_S.holder = side;
		if (before != side && m_GM.GetQuestlines())
			m_GM.GetQuestlines().OnStandHolder(before, side);

		for (int r = 0; r < ROUTES; r++)
		{
			m_S.counts[r] = 0;
		}

		// Taking it from a pact partner is a betrayal
		if (before >= 0 && m_S.pactWith[before] == side && how == HOW_FORCE)
		{
			BreakPact(side, before, "turned on its partner at the stand");
			m_S.how = HOW_BETRAYAL;
		}
		else if (how == HOW_THEFT)
		{
			m_S.how = HOW_THEFT;
		}
		else if (m_S.how != HOW_THEFT && m_S.how != HOW_BETRAYAL)
		{
			m_S.how = HOW_FORCE;
		}

		string from = "nobody";
		if (before >= 0)
			from = RouteName(before);

		RZ_Log.Info(string.Format("Finale: clean room %1 -> %2 (%3) with %4 of the stand left.", from, RouteName(side), HowWord(how), m_GM.GetCure().GetFinalStandMinutesLeft()));
		m_GM.NotifyAllAs("The clean room", string.Format("The clean room is %1's now.", RouteName(side)), RZ_ENoticeTag.URGENT);
	}

	//------------------------------------------------------------------------------------------------
	//! "CLEAN ROOM: Bastion. The Haven taking: 0:35 of 0:60." to everyone within 150 m of the lab:
	//! the HUD line under the noise meter, sent every 2 s while it's live (STAND_LINE_SECONDS); a
	//! notice instead when a count moves by 15 s or the room changes, for a player whose HUD can't
	//! take it. Names routes, never players.
	protected void TellStandLine(array<IEntity> players, float dt)
	{
		m_fStandHud += dt;
		if (m_fStandHud >= 2)
		{
			m_fStandHud = 0;
			string hud = StandLine();
			vector at = m_GM.GetLabPosition();
			foreach (IEntity hp : players)
			{
				if (RZ_Util.DistXZ(hp.GetOrigin(), at) > LAB_RADIUS)
					continue;

				RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(hp));
				if (ui)
					ui.SendStand(hud, 5);
			}
		}

		m_fStandLine += dt;
		int lead = NONE;
		float leadCount = 0;
		for (int r = 0; r < ROUTES; r++)
		{
			if (m_S.counts[r] > leadCount)
			{
				leadCount = m_S.counts[r];
				lead = r;
			}
		}

		int bucket = RZ_Util.RoundToInt(Math.Floor(leadCount / 15));
		if (bucket == m_iLastCount && m_fStandLine < 60)
			return;

		m_iLastCount = bucket;
		m_fStandLine = 0;
		string line = StandLine();
		vector lab = m_GM.GetLabPosition();
		foreach (IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), lab) > LAB_RADIUS)
				continue;

			int pid = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(p);
			if (!RZ_PlayerUIComponent.GetFor(pid))
				m_GM.NotifyPlayerAs(pid, "The stand", line, RZ_ENoticeTag.LOW);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The stand's line: who holds the room and the leading count, by route.
	string StandLine()
	{
		int lead = NONE;
		float leadCount = 0;
		for (int r = 0; r < ROUTES; r++)
		{
			if (m_S.counts[r] > leadCount)
			{
				leadCount = m_S.counts[r];
				lead = r;
			}
		}

		string holder = "nobody";
		if (m_S.holder >= 0)
			holder = RZ_Factions.GetShortName(PayFaction(m_S.holder));

		if (m_S.holder == INDEP)
			holder = "nobody's";

		string line = string.Format("CLEAN ROOM: %1.", holder);
		if (lead != NONE)
			line += string.Format(" %1 taking: %2 of %3.", RZ_Util.Capitalize(RouteName(lead)), Clock(leadCount), Clock(TakeTime(lead)));

		line.ToUpper();
		return line;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Clock(float secs)
	{
		int s = RZ_Util.RoundToInt(secs);
		int m = s / 60;
		int rest = s - m * 60;
		string pad = rest.ToString();
		if (rest < 10)
			pad = "0" + pad;

		return string.Format("%1:%2", m, pad);
	}

	//------------------------------------------------------------------------------------------------
	//! The 15 minutes are up: overtime runs while a challenger's count is above zero, 3 minutes at
	//! most. True while it runs (the cure waits).
	bool WantsOvertime(float dt)
	{
		if (!IsOn() || !m_S.routesOpen)
			return false;

		bool counting = false;
		for (int r = 0; r < ROUTES; r++)
		{
			if (m_S.counts[r] > 0)
				counting = true;
		}

		if (!counting)
			return false;

		if (!m_bInOvertime)
		{
			m_bInOvertime = true;
			m_GM.NotifyAllAs("The clean room", "Overtime. The room isn't settled yet.", RZ_ENoticeTag.URGENT);
		}

		m_fOvertime += dt;
		return m_fOvertime < m_Settings.m_fOvertimeSeconds;
	}

	//------------------------------------------------------------------------------------------------
	//! The stand was lost (nobody held the lab): research back to 90, the finisher stays finished
	//! (its Fame isn't taken back or paid again), locks and pacts stand. The next batch comes from
	//! the finisher's remake step or the lab's backlog.
	void OnStandLost()
	{
		m_bStandOn = false;
		ClearGroups();
		m_S.isSealed = false;
		m_S.batchReady = false;
		m_S.remaking = true;
		m_S.sealUnix = 0;
		m_S.readyUnix = 0;
		m_S.standsLost.Insert(RZ_SeasonClock.Day());
		if (m_S.finisher >= 0)
			m_S.holder = m_S.finisher;
		else
			m_S.holder = NONE;

		m_S.coatPassed = false;
		RZ_Log.Info(string.Format("Finale: the stand was lost (%1 this season). Remake from %2 or the backlog.", m_S.standsLost.Count(), RouteName(m_S.finisher)));
		if (m_GM.GetQuestlines())
			m_GM.GetQuestlines().RouteRemake(RouteKey(m_S.finisher));
	}

	//------------------------------------------------------------------------------------------------
	//! The faction whose rules apply in full: the controller's (the Independent's only by its one
	//! side), -1 if none yet.
	int ControllerFaction()
	{
		int c = m_S.controller;
		if (c >= 0 && c < INDEP)
			return c;

		if (c == INDEP && m_S.terms == 2 && m_S.condition > 0)
			return m_S.condition - 1;

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	int Controller() { return m_S.controller; }
	int How() { return m_S.how; }
	int Terms() { return m_S.terms; }
	int Partner() { return m_S.partner; }
	int Condition() { return m_S.condition; }

	//------------------------------------------------------------------------------------------------
	//! The stand was won (RZ_CureResearch.CompleteCure, before OnCureCompleted): whoever holds the
	//! clean room controls the cure. Nobody: handed over to the season's Fame leader.
	void OnStandWon()
	{
		m_bStandOn = false;
		ClearGroups();

		// The stand board's "CAME FOR IT: {n}": how many routes counted toward the room, the holder's
		// included (never names)
		int mask = m_S.cameMask;
		if (m_S.holder >= 0 && m_S.holder < ROUTES)
			mask = mask | (1 << m_S.holder);

		int came = 0;
		for (int bit = 0; bit < ROUTES; bit++)
		{
			if ((mask & (1 << bit)) != 0)
				came++;
		}

		RZ_WorldFacts.Get().SetFlag("finale.came", came);
		int c = m_S.holder;
		m_S.controller = c;
		if (c == NONE)
		{
			m_S.how = HOW_HANDED;
		}
		else if (m_S.how == 0)
		{
			// A deal: the holder's pact partner at 7 or more held it with it (one condition, half
			// rules next season; the chunk stays the finisher's)
			int p = m_S.pactWith[c];
			if (p >= 0 && Track(p) >= 7)
			{
				m_S.how = HOW_DEAL;
				m_S.partner = p;
			}
			else
			{
				m_S.how = HOW_HELD;
			}
		}
		else if (m_S.how == HOW_FORCE && m_S.pactWith[c] >= 0 && SideOf(c) == SideOf(m_S.pactWith[c]))
		{
			m_S.partner = m_S.pactWith[c];
		}

		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		facts.SetFlag("finale.route", RouteFlag(c));
		facts.SetFlag("finale.how", m_S.how);
		if (m_S.partner >= 0)
			facts.SetFlag("finale.partner", m_S.partner + 1);

		for (int r = 0; r < ROUTES; r++)
		{
			if (m_S.finisher == r && c != r)
				FinishedLost(r);
		}

		RZ_Log.Info(string.Format("Finale: controller %1, how %2, partner %3.", RouteName(c), HowWord(m_S.how), RouteName(m_S.partner)));

		// The routes end now; the controller's goes to its release (the terms)
		if (m_GM.GetQuestlines())
			m_GM.GetQuestlines().EndRoutes(RouteKey(c));

		// The contest ends: the stand's hostility comes off (StandMask), parley reopens with everyone
		if (c >= 0)
			m_GM.NotifyAll("The cure", "It's over. Talk.");
	}

	// ================================================================ Groups at the stand
	//------------------------------------------------------------------------------------------------
	//! At most Groups at the lab friendly faction groups (never Halcyon: its guard is the lab's own),
	//! 40-120 m out: a faction whose route has an active lock or whose major sends one (the Haven at
	//! the gate with proof.haven_gate, Bastion under The Call Stands, the Red Hand under Stood for
	//! nothing), never Bastion under Expended. The finisher's faction first, then by active locks.
	//! They fight the infected and, through the stand's hostility bits, the players of sides their
	//! route is fighting. Removed at the stand's end.
	protected void SpawnGroups()
	{
		ClearGroups();
		RZ_Territory territory = m_GM.GetTerritory();
		if (!territory || m_Settings.m_iGroups <= 0 || !m_GM.HasLab())
			return;

		array<int> order = {};
		if (m_S.finisher >= 0 && m_S.finisher < HALCYON)
			order.Insert(m_S.finisher);

		for (int pass = 0; pass < 2; pass++)
		{
			for (int f = 0; f < HALCYON; f++)
			{
				if (order.Contains(f) || !SendsGroup(f))
					continue;

				// First pass: factions with active locks, most first; second: the majors' groups
				if (pass == 0 && ActiveLocks(f) <= 0)
					continue;

				order.Insert(f);
			}
		}

		vector lab = m_GM.GetLabPosition();
		foreach (int g : order)
		{
			if (m_aGroups.Count() >= m_Settings.m_iGroups)
				break;

			ResourceName prefab = territory.GuardPrefab(g);
			if (prefab.IsEmpty())
				continue;

			// VERIFY: a spawned group honours the players' hostility bits (else it fights only the infected)
			// A major's group on the lab's road stands further out (RZ_Majors.OnLabRoad)
			float fromDist = 40;
			float toDist = 120;
			if (RZ_Majors.OnLabRoad(m_GM, g))
			{
				fromDist = 150;
				toDist = 250;
			}

			IEntity grp = RZ_Util.SpawnPrefab(prefab, RZ_Mission.LandPointNear(lab, fromDist, toDist));
			if (!grp)
				continue;

			m_aGroups.Insert(grp);
			m_aGroupFactions.Insert(g);
			RZ_Log.Info(string.Format("Finale: %1's group stands at the lab.", RZ_Factions.GetName(g)));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A faction's group stands at the lab in this stand (HC3 reads it for authors.guard).
	bool HasStandGroup(int f)
	{
		return m_bStandOn && m_aGroupFactions.Contains(f);
	}

	//------------------------------------------------------------------------------------------------
	protected bool SendsGroup(int f)
	{
		int stance = RZ_MajorHelp.World(m_GM, "orders.stance");
		if (f == BASTION && stance == 4)
			return false;

		if (ActiveLocks(f) > 0)
			return true;

		// The majors' inputs to the stand (Director/RZ_Majors.c, SendsStandGroup)
		return RZ_Majors.SendsStandGroup(m_GM, f);
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearGroups()
	{
		foreach (IEntity grp : m_aGroups)
		{
			if (grp)
				RZ_Util.DeleteUnit(grp);
		}

		m_aGroups.Clear();
		m_aGroupFactions.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! A finisher that lost the room: its faction's morale -3 (recovering), +10 Fame to its locked.
	//! The finisher lost the room: its faction's morale -3, and +10 Fame with it for each identity
	//! still actively locked to the route (docs/FINALE.md, "Fame").
	protected void FinishedLost(int r)
	{
		NudgeMorale(RouteFaction(r), -3);
		foreach (int i, string ident : m_S.lockIdent)
		{
			if (m_S.lockRoute[i] == r && IsActive(ident))
				PayIdent(ident, PayFaction(r), 10, 0, 0, "finished, lost the room");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The stand's Fame (replaces the built Halcyon +50, Haven +30): +30 with the route's faction for
	//! the controller's players at the lab half the stand or in the room 60 s; +20 Halcyon for
	//! everyone at the lab half the stand who never fired on the final holders. Paid after the winner
	//! and the record (RZ_CureResearch.CompleteCure), so it can't decide anything.
	void PayStandFame()
	{
		float half = RZ_CureResearch.FINAL_STAND_SECONDS * 0.5;
		foreach (int i, string ident : m_S.holdIdent)
		{
			int r = RouteOf(ident);
			float lab = m_S.labSecs[i];
			float room = m_S.roomSecs[i];
			if (r != NONE && r == m_S.controller && CountsAtStand(ident) && (lab + room >= half || room >= 60))
				PayIdent(ident, PayFaction(r), 30, 0, 0, "held the clean room");

			bool rival = r != NONE && SideOf(r) != SideOf(m_S.controller);
			if (!rival && lab + room >= half && !m_S.shotOwn.Contains(ident))
			{
				PayIdent(ident, RZ_EFaction.INSTITUTE, 20, 0, 0, "the lab never fell");
				int pid = m_GM.GetQuestlines().OnlineId(ident);
				if (pid > 0 && m_GM.GetHall())
					m_GM.GetHall().AddStand(pid);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	static string HowWord(int how)
	{
		switch (how)
		{
			case HOW_HELD: return "held";
			case HOW_FORCE: return "force";
			case HOW_DEAL: return "deal";
			case HOW_BETRAYAL: return "betrayal";
			case HOW_THEFT: return "theft";
			case HOW_HANDED: return "handed over";
		}

		return "none";
	}

	//------------------------------------------------------------------------------------------------
	//! The terms the controller's people chose at the release (the route's release scene), or the
	//! default. condition: the partner's or the Independent's one side (a faction + 1, 0 none).
	void OnTerms(int r, int terms, int condition, array<string> deciders)
	{
		if (r != m_S.controller || m_S.terms > 0)
			return;

		m_S.terms = Math.MaxInt(1, terms);
		m_S.condition = condition;
		if (m_S.partner >= 0 && condition <= 0)
			m_S.condition = PayFaction(m_S.partner) + 1;

		m_S.deciders.Clear();
		if (deciders)
		{
			foreach (string ident : deciders)
			{
				if (m_S.deciders.Count() < 4)
					m_S.deciders.Insert(ident);
			}
		}

		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		facts.SetFlag("finale.terms", m_S.terms);
		RZ_Log.Info(string.Format("Finale: controller %1, how %2, terms %3, deciders %4.", RouteName(r), HowWord(m_S.how), m_S.terms, m_S.deciders.Count()));
		DeciderPay(r);
		WorldEffects(r);

		// The Independent's one side pays its faction half the chunk, against its own soft line
		if (r == INDEP && m_S.terms == 2 && m_S.condition > 0)
			PayChunk(m_S.condition - 1, m_S.worth / 2, "the Independent's one side");

		m_GM.OnFinaleReleased();
	}

	//------------------------------------------------------------------------------------------------
	array<string> GetDeciders() { return m_S.deciders; }

	//------------------------------------------------------------------------------------------------
	//! The controller's deciders: +15 Fame and 30 Favor with the route's faction (the Independent by
	//! its terms: open +10 with each faction, one side +15 with it, sold or kept back +5 Infamy
	//! with every faction).
	protected void DeciderPay(int r)
	{
		foreach (string ident : m_S.deciders)
		{
			if (r != INDEP)
			{
				PayIdent(ident, RouteFaction(r), 15, 30, 0, "decided the terms");
				continue;
			}

			if (m_S.terms == 1)
			{
				for (int f = 0; f < RZ_Factions.COUNT; f++)
				{
					PayIdent(ident, f, 10, 0, 0, "the cure, open");
				}
			}
			else if (m_S.terms == 2 && m_S.condition > 0)
			{
				PayIdent(ident, m_S.condition - 1, 15, 0, 0, "the cure, one side");
			}
			else
			{
				for (int g = 0; g < RZ_Factions.COUNT; g++)
				{
					PayIdent(ident, g, 0, 0, 5, "the cure, kept from them");
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! One call each through the finale's own lane of the season budget.
	protected void WorldEffects(int r)
	{
		int t = m_S.terms;
		switch (r)
		{
			case HAVEN:
				NudgeMorale(HAVEN, 8);
				break;
			case BASTION:
				if (t == 1 || t == 4)
				{
					NudgeMorale(BASTION, 8);
					NudgeStrength(BASTION, 3);
					NudgeMorale(HAVEN, -4);
				}
				else
				{
					NudgeMorale(BASTION, 5);
					NudgeMorale(HAVEN, 5);
				}
				break;
			case REDHAND:
				if (t == 2)
				{
					NudgeMorale(REDHAND, 5);
					NudgeMorale(HAVEN, 5);
					RZ_MajorHelp.Relation(m_GM, HAVEN, REDHAND, RZ_FactionLife.REL_TRUCE);
				}
				else if (t == 3)
				{
					NudgeMorale(HAVEN, 8);
					NudgeMorale(REDHAND, -5);
				}
				else
				{
					NudgeMorale(REDHAND, 8);
					NudgeStrength(REDHAND, 3);
					NudgeMorale(HALCYON, -5);
				}
				break;
			case HALCYON:
				if (t == 1)
				{
					NudgeMorale(HALCYON, 6);
					NudgeMorale(HAVEN, -5);
				}
				else if (t == 2)
				{
					NudgeMorale(HAVEN, 5);
					NudgeMorale(HALCYON, -3);
				}
				else if (t == 3)
				{
					NudgeMorale(HALCYON, 5);
				}
				else
				{
					NudgeMorale(HALCYON, -5);
					NudgeMorale(HAVEN, 5);
				}
				break;
			case INDEP:
				if (t == 1)
				{
					for (int f = 0; f < RZ_Factions.COUNT; f++)
					{
						NudgeMorale(f, -3);
					}
				}
				else if (t == 2 && m_S.condition > 0)
				{
					NudgeMorale(m_S.condition - 1, 5);
				}
				else if (t == 3)
				{
					NudgeMorale(REDHAND, 3);
				}
				else
				{
					for (int g = 0; g < RZ_Factions.COUNT; g++)
					{
						NudgeMorale(g, -5);
					}
				}
				break;
		}

		// A deal between the Haven and Bastion: a truce for the epilogue
		if (m_S.partner >= 0 && ((r == HAVEN && m_S.partner == BASTION) || (r == BASTION && m_S.partner == HAVEN)))
			RZ_MajorHelp.Relation(m_GM, HAVEN, BASTION, RZ_FactionLife.REL_TRUCE);
	}

	//------------------------------------------------------------------------------------------------
	protected void NudgeMorale(int f, float delta)
	{
		if (!RZ_Factions.IsValid(f) || !m_GM.GetFactionLife())
			return;

		float d = delta;
		RZ_MajorBudget budget = RZ_MajorBudget.Get();
		if (budget)
			d = budget.TakeMorale(RZ_EBudgetLane.FINALE, f, delta, "finale", "the finale");

		if (d != 0)
			m_GM.GetFactionLife().AddMorale(f, d);
	}

	//------------------------------------------------------------------------------------------------
	protected void NudgeStrength(int f, float delta)
	{
		if (!RZ_Factions.IsValid(f) || !m_GM.GetTerritory())
			return;

		float d = delta;
		RZ_MajorBudget budget = RZ_MajorBudget.Get();
		if (budget)
			d = budget.TakeStrength(RZ_EBudgetLane.FINALE, f, delta, "finale", "the finale");

		if (d != 0)
			m_GM.GetTerritory().AddStrength(f, d);
	}

	// ================================================================ The Fame chunk and pay
	//------------------------------------------------------------------------------------------------
	//! B into a faction's season Fame (never a player's standing), with the soft line: in full until
	//! the faction reaches Soft line x the best other faction's season Fame, half above it.
	protected void PayChunk(int f, int amount, string why)
	{
		if (!RZ_Factions.IsValid(f) || amount <= 0)
			return;

		int other = 0;
		for (int g = 0; g < RZ_Factions.COUNT; g++)
		{
			if (g != f)
				other = Math.MaxInt(other, m_GM.GetSeasonFame(g));
		}

		int line = RZ_Util.RoundToInt(other * m_Settings.m_fSoftLine);
		int now = m_GM.GetSeasonFame(f);
		int full = Math.ClampInt(line - now, 0, amount);
		int halfPart = (amount - full) / 2;
		m_GM.AddFactionBonus(f, full + halfPart, why);
		RZ_Log.Info(string.Format("Finale: %1 paid %2 season Fame (%3 full, %4 half). Season Fame: %5 %6, %7 %8.", why, full + halfPart, full, halfPart,
			RZ_Factions.GetShortName(f), m_GM.GetSeasonFame(f), RZ_Factions.GetShortName(m_GM.GetSeasonLeader()), m_GM.GetSeasonFame(m_GM.GetSeasonLeader())));
	}

	//------------------------------------------------------------------------------------------------
	//! Pay inside the finale's caps: 50 Fame per faction per player over all finale content, 60 Favor
	//! and 40 Infamy per faction per player. Online now; offline, the Fame and Favor wait in the
	//! errands' owed pay for that life's next join (AddFame and AddFavor then), the Infamy goes onto
	//! the saved profile.
	void PayIdent(string ident, int f, int fame, int favor, int infamy, string why)
	{
		if (ident.IsEmpty() || !RZ_Factions.IsValid(f))
			return;

		fame = Clip(ident, f, "fame", fame, CAP_FAME);
		favor = Clip(ident, f, "favor", favor, CAP_FAVOR);
		infamy = Clip(ident, f, "infamy", infamy, CAP_INFAMY);
		if (fame == 0 && favor == 0 && infamy == 0)
			return;

		int pid = m_GM.GetQuestlines().OnlineId(ident);
		RZ_EFaction ef = f;
		if (pid > 0)
		{
			if (fame > 0)
				m_GM.AddFame(pid, ef, fame, false);

			if (favor > 0)
				m_GM.AddFavor(pid, ef, favor);

			if (infamy > 0)
				m_GM.AddInfamy(pid, ef, infamy, why, RZ_RepFlag.ACT);

			return;
		}

		RZ_PlayerProfile profile = m_GM.FindProfileByIdentity(ident);
		if (!profile)
			return;

		if ((fame > 0 || favor > 0) && m_GM.GetErrands())
			m_GM.GetErrands().AddOwed(ident, RZ_ErrandOwed.Make(f, Math.Max(0, fame), Math.Max(0, favor), "The cure", profile.m_iLives));

		if (infamy > 0)
			profile.AddInfamy(ef, infamy, true);
	}

	//------------------------------------------------------------------------------------------------
	protected int Clip(string ident, int f, string kind, int amount, int cap)
	{
		if (amount <= 0)
			return 0;

		string key = string.Format("%1|%2|%3", ident, f, kind);
		int idx = m_S.payKeys.Find(key);
		int used = 0;
		if (idx >= 0)
			used = m_S.payValues[idx];

		int allowed = Math.ClampInt(cap - used, 0, amount);
		if (idx >= 0)
			m_S.payValues[idx] = used + allowed;
		else
		{
			m_S.payKeys.Insert(key);
			m_S.payValues.Insert(allowed);
		}

		return allowed;
	}

	// ================================================================ Pacts and fighting routes
	//------------------------------------------------------------------------------------------------
	int PactOf(int r)
	{
		if (r < 0 || r >= ROUTES)
			return -1;

		return m_S.pactWith[r];
	}

	//------------------------------------------------------------------------------------------------
	//! A locked player of route `from` proposes a pact with `to`, agrees to one `to` proposed, or
	//! ends the one they have (a tally: one vote per active lock). Returns what they're told.
	string PactVote(int playerId, int from, int to)
	{
		if (!m_Settings.m_bPacts)
			return "There are no pacts this season.";

		string ident = Ident(playerId);
		if (RouteOf(ident) != from || from == to || to < 0 || to >= ROUTES)
			return "\"You've picked your side. This isn't it.\"";

		if (!IsActive(ident))
			return "\"Only the regulars get a say. Come back when you've put the hours in.\"";

		if (IsCured() || m_bStandOn)
			return "\"Too late for that.\"";

		// Bastion and the Red Hand can't while a feud from the majors stands (Expended)
		if ((from == BASTION && to == REDHAND) || (from == REDHAND && to == BASTION))
		{
			if (RZ_MajorHelp.World(m_GM, "book.last") == 5 || RZ_MajorHelp.World(m_GM, "orders.stance") == 4)
				return "\"Not with them. Not this season.\"";
		}

		string vote = string.Format("%1|%2|%3", from, to, ident);
		if (m_S.pactWith[from] == to)
		{
			// Ending one before the stand costs nothing
			string endVote = "end" + vote;
			if (!m_S.votes.Contains(endVote))
				m_S.votes.Insert(endVote);

			if (Votes("end", from, to) * 2 > ActiveLocks(from))
			{
				EndPact(from, to);
				return "The pact is ended. It cost nothing.";
			}

			return "Your vote to end the pact is counted.";
		}

		if (m_S.pactWith[from] >= 0 || m_S.pactWith[to] >= 0)
			return "\"One pact a route. We've got one, or they have.\"";

		if (!m_S.votes.Contains(vote))
			m_S.votes.Insert(vote);

		if (m_S.propTo[from] != to && m_S.propTo[to] != from)
		{
			m_S.propTo[from] = to;
			m_S.propUnix[from] = RZ_SeasonClock.Now();
			RZ_Log.Info(string.Format("Finale: %1 proposes a pact with %2.", RouteName(from), RouteName(to)));
			TellRoute(to, string.Format("%1 proposes a pact: standing together at the stand. Vote at your route's desk within a day.", RouteName(from)));
		}

		CheckPact(from, to);
		if (m_S.pactWith[from] == to)
			return string.Format("The pact with %1 is made.", RouteName(to));

		return "Your vote is counted.";
	}

	//------------------------------------------------------------------------------------------------
	protected int Votes(string prefix, int from, int to)
	{
		int n = 0;
		string head = string.Format("%1%2|%3|", prefix, from, to);
		foreach (string v : m_S.votes)
		{
			if (v.StartsWith(head))
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! A pact is agreed with at least one vote from each route and a majority of each route's active
	//! locks (one vote each, at once, under All Stations for the Haven and Bastion).
	protected void CheckPact(int a, int b)
	{
		int va = Votes("", a, b);
		int vb = Votes("", b, a);
		if (va <= 0 || vb <= 0)
			return;

		bool quick = RZ_MajorHelp.World(m_GM, "orders.stance") == 2 && ((a == HAVEN && b == BASTION) || (a == BASTION && b == HAVEN));
		if (!quick && (va * 2 <= ActiveLocks(a) || vb * 2 <= ActiveLocks(b)))
			return;

		m_S.pactWith[a] = b;
		m_S.pactWith[b] = a;
		m_S.propTo[a] = -1;
		m_S.propTo[b] = -1;
		RZ_Log.Info(string.Format("Finale: pact %1 + %2 (tally %3-%4).", RouteName(a), RouteName(b), va, vb));
		TellRoute(a, string.Format("The pact with %1 is made. At the stand you're one side.", RouteName(b)));
		TellRoute(b, string.Format("The pact with %1 is made. At the stand you're one side.", RouteName(a)));
	}

	//------------------------------------------------------------------------------------------------
	//! Proposals lapse after a real day.
	protected void TickPacts()
	{
		int now = RZ_SeasonClock.Now();
		for (int r = 0; r < ROUTES; r++)
		{
			if (m_S.propTo[r] >= 0 && now - m_S.propUnix[r] > PACT_SECONDS * WaitScale())
			{
				RZ_Log.Info(string.Format("Finale: %1's pact proposal lapsed.", RouteName(r)));
				m_S.propTo[r] = -1;
			}
		}

		// Under Under Seal (or the terms) a Bastion-Halcyon pact starts made
		int seal = RZ_MajorHelp.World(m_GM, "proof.first_page");
		if (seal == 4 && m_S.pactWith[BASTION] < 0 && m_S.pactWith[HALCYON] < 0 && !m_S.votes.Contains("seal"))
		{
			m_S.votes.Insert("seal");
			m_S.pactWith[BASTION] = HALCYON;
			m_S.pactWith[HALCYON] = BASTION;
			RZ_Log.Info("Finale: pact Bastion + Halcyon (Under Seal).");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void EndPact(int a, int b)
	{
		m_S.pactWith[a] = -1;
		m_S.pactWith[b] = -1;
		ClearVotes(a, b);
		RZ_Log.Info(string.Format("Finale: pact %1 + %2 ended.", RouteName(a), RouteName(b)));
		TellRoute(b, string.Format("%1 has ended the pact. No hard feelings, officially.", RouteName(a)));
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearVotes(int a, int b)
	{
		for (int i = m_S.votes.Count() - 1; i >= 0; i--)
		{
			string v = m_S.votes[i];
			if (v.Contains(string.Format("%1|%2|", a, b)) || v.Contains(string.Format("%1|%2|", b, a)))
				m_S.votes.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A broken pact: +15 Infamy with the partner's faction for every active lock of the breaking
	//! route, the word "turncoat", and the pact ends.
	void BreakPact(int breaker, int partner, string what)
	{
		if (breaker < 0 || partner < 0 || m_S.pactWith[breaker] != partner)
			return;

		m_S.pactWith[breaker] = -1;
		m_S.pactWith[partner] = -1;
		ClearVotes(breaker, partner);
		Hostile(breaker, partner);
		int f = PayFaction(partner);
		foreach (int i, string ident : m_S.lockIdent)
		{
			if (m_S.lockRoute[i] == breaker && IsActive(ident))
				PayIdent(ident, f, 0, 0, 15, "broke a pact");
		}

		RZ_Log.Info(string.Format("Finale: %1 broke its pact with %2 (%3).", RouteName(breaker), RouteName(partner), what));
		TellRoute(partner, string.Format("%1 broke the pact. Your leader won't forget it.", RouteName(breaker)));
	}

	//------------------------------------------------------------------------------------------------
	//! A hostile route act: the two routes are fighting for Fight memory days.
	void Hostile(int a, int b)
	{
		if (a < 0 || b < 0 || a >= ROUTES || b >= ROUTES || a == b)
			return;

		int now = RZ_SeasonClock.Now();
		m_S.fight[a * ROUTES + b] = now;
		m_S.fight[b * ROUTES + a] = now;
	}

	//------------------------------------------------------------------------------------------------
	//! Fighting: a hostile act in the last Fight memory days, or the stand (every two sides).
	bool Fighting(int a, int b)
	{
		if (a < 0 || b < 0 || a >= ROUTES || b >= ROUTES || SideOf(a) == SideOf(b))
			return false;

		if (m_bStandOn)
			return true;

		return RZ_SeasonClock.Now() - m_S.fight[a * ROUTES + b] <= m_Settings.m_fFightDays * DAY;
	}

	//------------------------------------------------------------------------------------------------
	//! Parley and settling up close with any faction whose route is fighting yours.
	bool ParleyClosed(int playerId, int faction)
	{
		int mine = RouteOfPlayer(playerId);
		if (!IsOpen() || mine == NONE || faction < 0 || faction >= INDEP)
			return false;

		return Fighting(mine, faction);
	}

	//------------------------------------------------------------------------------------------------
	protected void TellRoute(int r, string text)
	{
		array<int> all = {};
		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int pid : all)
		{
			if (RouteOfPlayer(pid) == r)
				m_GM.NotifyPlayer(pid, RouteTitle(r), text);
		}
	}

	// ================================================================ Last Mile
	//------------------------------------------------------------------------------------------------
	protected void LastMileOpens(int r)
	{
		if (m_S.lastMile[r] != 0)
			return;

		m_S.lastMile[r] = 1;
		m_GM.NotifyAll("Word on the roads", "One of them is close.");
		RZ_Log.Info(string.Format("Finale: Last Mile on %1 (track 8).", RouteName(r)));
	}

	//------------------------------------------------------------------------------------------------
	//! Every 10 minutes: route items on the move for a route at 8 or more, a rough area to every
	//! other route's locked players.
	protected void LastMileReports(array<int> ids)
	{
		for (int r = 0; r < ROUTES; r++)
		{
			if (m_S.lastMile[r] == 0)
				continue;

			vector at = m_Items.CarriedPos(r);
			if (at == vector.Zero)
				continue;

			RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(at));
			string where = "somewhere on the roads";
			if (s)
				where = "near " + s.m_sName;

			foreach (int pid : ids)
			{
				int mine = RouteOfPlayer(pid);
				if (mine != NONE && mine != r)
					m_GM.NotifyPlayer(pid, "Last Mile", string.Format("%1's carrier is %2.", RouteName(r), where));
			}
		}
	}

	// ================================================================ Kills, shots and hostility
	//------------------------------------------------------------------------------------------------
	//! A player killed another (RZ_FactionPerks.OnPlayerDied). Locked victims: free at the stand
	//! between different sides near the lab; else +3 Infamy with the victim's route faction (the
	//! Haven for an Independent), at most 12 an hour, tagged war. Shooting your own side: no count
	//! at this stand, +10 with your route's faction. An unlocked victim at the stand: +10 Halcyon.
	void OnPlayerKilled(int victim, int killer, vector at)
	{
		// Contest Auto: players can hurt each other on this server
		if (killer > 0 && killer != victim)
			RZ_WorldFacts.Get().SetFlag("finale.pvp_seen", 1);

		if (!IsOpen())
			return;

		// Route items drop where their carrier fell, whatever killed them
		string killerIdent = "";
		if (killer > 0)
			killerIdent = Ident(killer);

		m_Items.OnCarrierDied(Ident(victim), at, killerIdent);
		if (killer <= 0 || killer == victim)
			return;

		int vr = RouteOfPlayer(victim);
		int kr = RouteOfPlayer(killer);
		bool atLab = m_bStandOn && RZ_Util.DistXZ(at, m_GM.GetLabPosition()) <= LAB_RADIUS;
		if (vr != NONE && kr != NONE && SideOf(vr) == SideOf(kr))
		{
			string kIdent = Ident(killer);
			if (atLab && !m_S.shotOwn.Contains(kIdent))
				m_S.shotOwn.Insert(kIdent);

			PayIdent(kIdent, PayFaction(kr), 0, 0, 10, "shot your own side");
			return;
		}

		if (atLab)
		{
			if (vr == NONE)
				PayIdent(Ident(killer), RZ_EFaction.INSTITUTE, 0, 0, 10, "killed a bystander at the stand");

			return;
		}

		if (vr == NONE)
			return;

		if (kr != NONE)
			Hostile(kr, vr);

		string ident = Ident(killer);
		int hour = RZ_SeasonClock.Now() / HOUR;
		int was;
		if (!m_mKillHour.Find(ident, was) || was != hour)
		{
			m_mKillHour.Set(ident, hour);
			m_mKillUsed.Set(ident, 0);
		}

		int used;
		m_mKillUsed.Find(ident, used);
		int amount = Math.ClampInt(KILL_INFAMY_HOUR - used, 0, KILL_INFAMY);
		if (amount <= 0)
			return;

		m_mKillUsed.Set(ident, used + amount);
		RZ_EFaction f = PayFaction(vr);
		m_GM.AddInfamy(killer, f, amount, "killed a locked player (war)", RZ_RepFlag.ACT, "killed one of their route's people");
	}

	//------------------------------------------------------------------------------------------------
	//! During the stand, near the lab: a locked player is exempt from the lab's safe-zone rule.
	bool SafeZoneExempt(int playerId, vector pos)
	{
		return m_bStandOn && RouteOfPlayer(playerId) != NONE && RZ_Util.DistXZ(pos, m_GM.GetLabPosition()) <= LAB_RADIUS;
	}

	//------------------------------------------------------------------------------------------------
	//! The stand's hostility bits (RZ_Hostility.ComputeMask): within 1.5 km of the lab during the
	//! stand, a player locked to a side fighting faction f's route carries f's bit, so f's group
	//! fights them. Halcyon's bit is never set. Tally mode: groups never target players.
	int StandMask(int playerId, vector pos)
	{
		if (!m_bStandOn || IsTally() || !IsOn())
			return 0;

		int mine = RouteOfPlayer(playerId);
		if (mine == NONE || RZ_Util.DistXZ(pos, m_GM.GetLabPosition()) > STAND_RADIUS)
			return 0;

		int mask = 0;
		for (int f = 0; f < HALCYON; f++)
		{
			if (CountLocks(f) > 0 && Fighting(mine, f))
				mask = mask | RZ_Factions.Bit(f);
		}

		return mask;
	}

	//------------------------------------------------------------------------------------------------
	bool IsStandOn()
	{
		return m_bStandOn;
	}

	// ================================================================ Take the Lab and the Coat Run
	//------------------------------------------------------------------------------------------------
	//! Take the Lab under a finale: offered once a season to the Red Hand route, unless the Red
	//! Hand's last page closed it (book.last 1-4).
	protected void OfferTakeTheLab()
	{
		int last = RZ_MajorHelp.World(m_GM, "book.last");
		if (last >= 1 && last <= 4)
			return;

		if (CountLocks(REDHAND) <= 0 || !m_GM.GetMissions() || m_GM.GetMissions().IsTypeRunning(RZ_Mission_TakeTheLab))
			return;

		// Out of the random pool under a finale (weight 0), so it's started directly
		RZ_Mission_TakeTheLab take = new RZ_Mission_TakeTheLab();
		take.Configure(m_GM);
		if (m_GM.GetMissions().WhyNot(take, 1, false).IsEmpty() && m_GM.GetMissions().StartMission(take))
			RZ_Log.Info("Finale: Take the Lab is offered to the Red Hand route.");
	}

	//------------------------------------------------------------------------------------------------
	//! Take the Lab's bay hold time: 120 s, 90 with the night guard paid (authors.batch_sold), 150
	//! if a rival bought the plan.
	float BaySeconds()
	{
		float s = m_Settings.m_fBaySeconds;
		if (RZ_MajorHelp.World(m_GM, "authors.batch_sold") != 0)
			s = 90;

		if (RZ_MajorHelp.World(m_GM, "finale.plan_sold") != 0)
			s = 150;

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! A player may hold the bay for the Red Hand route: locked to it and Trusted with the Red Hand
	//! to start, any Red Hand-locked player who counts at the stand to hold.
	bool CanStartTake(int playerId)
	{
		return RouteOfPlayer(playerId) == REDHAND && RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.TRUSTED);
	}

	//------------------------------------------------------------------------------------------------
	bool CountsForBay(int playerId)
	{
		string ident = Ident(playerId);
		return RouteOf(ident) == REDHAND && CountsAtStand(ident);
	}

	//------------------------------------------------------------------------------------------------
	//! The holder's side has a counted player within 15 m of the bay: the bay's count pauses.
	bool BayContested(array<IEntity> players)
	{
		int holder = SideOf(m_S.holder);
		if (holder == NONE)
			return false;

		vector bay = BayPos();
		foreach (IEntity p : players)
		{
			int pid = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(p);
			string ident = Ident(pid);
			int r = RouteOf(ident);
			if (r != NONE && SideOf(r) == holder && CountsAtStand(ident) && RZ_Util.DistXZ(p.GetOrigin(), bay) <= BAY_GUARD)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Take the Lab succeeded: the Red Hand route holds the room at once. Using it against a pact
	//! partner is a betrayal.
	void OnTakeTheLab()
	{
		m_S.takeUsed = true;
		int before = m_S.holder;
		if (before >= 0 && m_S.pactWith[REDHAND] == before)
			BreakPact(REDHAND, before, "Take the Lab against its partner");

		RZ_WorldFacts.Get().SetFlag("finale.taken", RZ_SeasonClock.Day());
		TakeRoom(REDHAND, HOW_THEFT);
	}

	//------------------------------------------------------------------------------------------------
	//! The Coat Run can be tried now: another route's batch hold, at a change of shift, once.
	bool CoatRunOpen()
	{
		if (!m_Settings.m_bCoatRun || m_S.coatUsed || !IsFrozen() || m_S.isSealed || m_S.finisher == INDEP || m_S.finisher == LAB)
			return false;

		if (!RZ_GameTime.IsCalibrated())
			return true;

		float h = RZ_GameTime.GetHour();
		return (h >= 5.5 && h < 6.5) || (h >= 17.5 && h < 18.5);
	}

	//------------------------------------------------------------------------------------------------
	//! The Coat Run: an Independent-locked player in a clean Halcyon coat re-labels the batch inside
	//! the lab. Passed: the Independent starts the stand as holder (told at the seal). Caught: the
	//! built catch, the batch's route told at once. Spent either way.
	void CoatRun(int playerId, bool passed)
	{
		m_S.coatUsed = true;
		if (passed)
		{
			m_S.coatPassed = true;
			RZ_WorldFacts.Get().SetFlag("finale.coat_run", RZ_SeasonClock.Day());
			RZ_Log.Info("Finale: the Coat Run passed. The batch says 'Released to: nobody'.");
			return;
		}

		TellRoute(m_S.finisher, "Someone in a lab coat tried to change the label on our batch. The duty officer caught it.");
		RZ_Log.Info("Finale: the Coat Run was caught.");
	}

	// ================================================================ The journal
	//------------------------------------------------------------------------------------------------
	//! The finale page's lines for this player: the tracks, the route's worth, the seal, and their
	//! own route's notes (lock counts are never shown).
	string PageFor(int playerId)
	{
		if (!IsOpen())
			return "";

		string s = Readout();
		s += string.Format("\nAbout %1 season Fame to the faction that finishes.", m_S.worth);
		int mine = RouteOfPlayer(playerId);
		if (mine != NONE)
		{
			s += string.Format("\nYou stand with %1 until the cure.", RouteName(mine));
			int p = m_S.pactWith[mine];
			if (p >= 0)
				s += string.Format(" In a pact with %1.", RouteName(p));

			string items = m_Items.RouteText(mine);
			if (!items.IsEmpty())
				s += "\nYour route's things: " + items + ".";
		}
		else
		{
			string bring = m_Items.CarriedText(Ident(playerId), -1);
			if (!bring.IsEmpty())
				s += "\nWhat you'd bring: " + bring + ".";

			for (int r = 0; r < ROUTES; r++)
			{
				string assets = m_Assets.Text(Ident(playerId), r);
				if (!assets.IsEmpty())
					s += string.Format("\nWhat you'd bring to %1: %2.", RouteName(r), assets);
			}
		}

		if (IsFrozen())
		{
			if (m_S.batchReady && m_S.sealUnix > 0)
				s += string.Format("\nThe batch seals in about %1 minutes.", Math.MaxInt(1, (m_S.sealUnix - RZ_SeasonClock.Now()) / 60));
			else if (!m_S.batchReady)
				s += "\nThe batch is in the cold.";
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! The hall of fame's season line: "The cure: the Haven, by force. The Zone's Fame: Bastion, 1640."
	string SeasonLine(int winner)
	{
		int leader = m_GM.GetSeasonLeader();
		string zone = "nobody";
		if (leader >= 0)
			zone = string.Format("%1, %2", RZ_Factions.GetName(leader), m_GM.GetSeasonFame(leader));

		string cure = "nobody";
		if (m_S.controller >= 0)
			cure = string.Format("%1, by %2", RouteName(m_S.controller), HowWord(m_S.how));
		else if (winner >= 0)
			cure = string.Format("%1, handed over", RZ_Factions.GetNameInText(winner));

		return string.Format("The cure: %1. The Zone's Fame: %2.", cure, zone);
	}
}
