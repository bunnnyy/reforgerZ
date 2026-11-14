//! Work sites (docs/DESIGN.md, "Mission building blocks", Work site): the one way a step that
//! says you DO something with your hands is done. A visible thing stands at the step's place (the
//! "before" look: a marked pit, a pile of planks, a crate), with one world prompt on it ("Dig the
//! latrine"). Hold the prompt (vanilla's hold-progress bar, RZ_WorkSiteAction) for the work's time,
//! 5-20 s, and the world shows the result: the before look goes and the "after" look stands there
//! (the latrine screen, the boarded door, the grave), for the step's life.
//!
//! Owners (errands, the openings, strangers, missions) open a site by key and poll IsDone(key) on
//! their own tick; they Close(key) when their step is over. Nothing here pays or decides anything.
//!
//! Server-checked: the server keeps who has been within reach of each spawned site, unhurt, and
//! since when (once a second). The hold's end (PerformAction on the server) counts only when that
//! player has been there for the work's time, less a second's slack; moving away or being hurt
//! starts the count again. The client's bar starts again too when you're hurt (RZ_WorkSiteAction).
//!
//! Props spawn only within the marks' spawn radius of a player and go past the despawn radius; a
//! site is data the rest of the time, never saved (an owner reopens it). With no site prefab
//! (Shared > Work sites > Site prefab) or Work sites off, Available() is false and owners keep their
//! old rule (stand there for the time).
//!
//! Several things to do at one spot are one prompt and a list (docs/UI.md, "Default first, then
//! menus"): the owner opens a site for each and gathers them (Gather) under the spot's name ("The
//! far door"). One box stands for them all; a press opens the object menu with each as a row
//! (RZ_ObjectScreen.OpenRows), and the row picked becomes the box's hold, so the work is still held
//! with the hands and the legs locked, and still counted here. Walk off, or leave it a minute, and
//! it's the list again. With one of them left, the box is that one's plain hold. With no object menu
//! (UI > Object row layout empty), or for a player whose client failed one, each stands as its own
//! prompt as before.
//! Server only, except the enum and the settings.

//------------------------------------------------------------------------------------------------
//! What kind of work: picks the before and after looks (Shared > Work sites > Looks).
enum RZ_EWork
{
	NONE,
	LATRINE,	// dig a latrine: a marked pit, then the latrine screen
	GRAVE,		// dig a grave, raise a cairn: a marked plot, then a mound and a post
	BOARDS,		// board up a door or window: planks and nails, then the boarded door
	SANDBAGS,	// fill and stack sandbags: a pile of empty sacks, then a sandbag wall
	REPAIR,		// fit a part, fix a machine: a toolbox and parts, then packed away
	INSTALL,	// set something up (a sensor): parts, then the thing on its crate
	SEARCH,		// go through a heap, a shed, a wreck's box: a crate, then rummaged clutter
	TAKE,		// take things off a body, out of a kit bag: the effects, then gone
	FILL,		// fill cans or bottles at water or a pump: empty cans, then carried off
	PAINT,		// paint a number, a sign, a hand: a bare board and a tin, then the painted board
	WRITE,		// write or pin something up: a bare board, then a board with notices
	BURN,		// burn a pile: the pile, then ash
	CARE,		// dress a wound, see to someone lying down: the bed (it stays)
	BENCH,		// work at a table: cleaning, folding, labelling (the table stays)
	DIG,		// dig for something: a stake and a sack, then a heap of turned earth
	HANDS		// labour at something already there (push a car): no looks of its own
}

//------------------------------------------------------------------------------------------------
//! One kind's looks (Shared > Work sites > Looks).
[BaseContainerProps()]
class RZ_WorkLook
{
	[Attribute("0", UIWidgets.ComboBox, "The work these looks are for", "", ParamEnumArray.FromEnum(RZ_EWork))]
	RZ_EWork m_eKind;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Before: what stands there while the work is to do (a marked pit, planks, a crate). A composition of vanilla props, nothing with actions. Empty = only the prompt", "et")]
	ResourceName m_sBefore;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "After: what stands there once it's done (the latrine screen, the boarded door, the grave). Empty = nothing (the thing was taken or packed away)", "et")]
	ResourceName m_sAfter;
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Shared > Work sites.
[BaseContainerProps()]
class RZ_WorkSiteSettings
{
	[Attribute("1", desc: "Work sites: a step that says you do something with your hands (dig, build, board up, fix, search, fill, paint, burn, dress a wound) is a thing at its place with one hold prompt, and the world shows the result. Off, or no Site prefab: those steps are done by staying there for their time, as before (docs/DESIGN.md, Work site)")]
	bool m_bEnabled;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Site prefab: the invisible interaction box that carries the prompt (RZ_WorkSiteComponent, RZ_WorkSiteAction with a Duration, an RplComponent). Empty = work sites off", "et")]
	ResourceName m_sSitePrefab;

	[Attribute(desc: "The looks: before and after, one entry per kind of work (a kind with no entry has only the prompt)")]
	ref array<ref RZ_WorkLook> m_aLooks;

	[Attribute("3", UIWidgets.Slider, "Reach (m): you work within this of the site; further away, or hurt, and the count starts again", "1.5 6 0.5")]
	float m_fReach;

	[Attribute("1", UIWidgets.Slider, "Time scale: every hold takes its time x this (each step sets its own, 5-20 s)", "0.25 3 0.05")]
	float m_fTimeScale;

	[Attribute("1", desc: "Lock movement: while the hold runs you can't walk (as with vanilla's building). It's let go the moment the hold ends, you're hit or you go down. Off = you can walk off, and past 1.5 m the bar starts again")]
	bool m_bLockMovement;

	[Attribute("1", desc: "Hands animation: while the hold runs you work with your hands at the thing: crouched at a low one, reaching down and taking hold again and again (vanilla's pick-up gesture; if that won't play, the item use vanilla plays at a mine being disarmed). Off = no animation unless digging with a tool")]
	bool m_bHandsAnimation;

	[Attribute("1", desc: "Dig with the tool: at a latrine, a grave or a dig, a carried entrenching tool is taken in hand and vanilla's building animation plays. The tool is never needed: without one, the hands animation")]
	bool m_bDigWithTool;

	[Attribute("1", desc: "Carry for real: a task whose words say carry, stack or bring in is done by moving the things one by one, from a pile to where the words say, slowed while you carry (docs/DESIGN.md, Carrying). Off: one hold where the things are, as before")]
	bool m_bCarry;

	[Attribute("0.45", UIWidgets.Slider, "Carrying speed: the carrier's top speed x this while a thing is in their arms (under about 0.5 there's no running)", "0.2 1 0.05")]
	float m_fCarrySpeed;

	[Attribute("{6E1B3C5A2D4F7081}Prefabs/ReforgerZ/Work/RZ_Carried_Box.et", UIWidgets.ResourcePickerThumbnail, "Carried box: what shows in the arms while a crate, a box or a sack is carried. Look only: a model, no physics, nothing to use. Empty = nothing shown in the arms", "et")]
	ResourceName m_sCarriedBox;

	[Attribute("{7A2C4D6B3E5F8192}Prefabs/ReforgerZ/Work/RZ_Carried_Chair.et", UIWidgets.ResourcePickerThumbnail, "Carried chair: what shows in the arms while a chair is carried. Look only, as the Carried box. Empty = nothing shown in the arms", "et")]
	ResourceName m_sCarriedChair;

	[Attribute("40", UIWidgets.Slider, "Sites at once, server-wide. A new one past this drops the oldest whose step is over (its result still standing); one whose step is under way is never dropped", "8 200 1")]
	int m_iMaxSites;

	//------------------------------------------------------------------------------------------------
	RZ_WorkLook Look(RZ_EWork kind)
	{
		if (!m_aLooks)
			return null;

		foreach (RZ_WorkLook l : m_aLooks)
		{
			if (l && l.m_eKind == kind)
				return l;
		}

		return null;
	}
}

//------------------------------------------------------------------------------------------------
//! One site: data always, props only near players.
class RZ_WorkSite
{
	int m_iId;
	string m_sKey;				// the owner's key ("errand.12", "mission.4.point.1")
	RZ_EWork m_eKind;
	string m_sLabel;			// the prompt: "Dig the latrine"
	float m_fSeconds;			// how long the hold takes
	float m_fFastSeconds;		// for the players in m_aFast (a background's bonus), 0 = none
	ref array<int> m_aFast = {};
	vector m_vPos;
	bool m_bKeepHeight;			// indoors: m_vPos is on a floor, not on the terrain
	float m_fYaw;
	int m_iOnly;				// only this player may do it (0: anyone)
	bool m_bUnseen;				// only while hidden (built stealth): seen, and the count starts again
	ResourceName m_sBefore;
	ResourceName m_sAfter;
	bool m_bDone;
	int m_iDoneBy;
	bool m_bClosed;				// the owner is done with it: dropped once nobody is near
	float m_fOpenedAt;

	int m_iFit;					// the before look's spot: 0 not checked yet, 1 its box fits, -1 nowhere (left out)
	ResourceName m_sPerson;		// who the work is for (a patient, the newcomer), standing by it while it's open
	vector m_vPerson;
	bool m_bPersonLies;			// they lie on the work's spot (its bed) instead, and the prompt is at their side

	// Props (null while nobody is near)
	IEntity m_Box;
	IEntity m_BeforeProp;
	IEntity m_AfterProp;
	IEntity m_Person;

	// What it needs from the pack (a menu row is locked without it): the owner checks again when done
	RZ_EErrandItem m_eNeed;
	int m_iNeed;

	// Who is working at it now: player, since when (the sites' clock), health then
	ref array<int> m_aNear = {};
	ref array<float> m_aSince = {};
	ref array<float> m_aHealth = {};

	//------------------------------------------------------------------------------------------------
	float SecondsFor(int playerId)
	{
		if (m_fFastSeconds > 0 && m_aFast.Contains(playerId))
			return m_fFastSeconds;

		return m_fSeconds;
	}

	//------------------------------------------------------------------------------------------------
	string FastText()
	{
		string text = "|";
		foreach (int id : m_aFast)
		{
			text = text + id.ToString() + "|";
		}

		return text;
	}
}

//------------------------------------------------------------------------------------------------
//! Several sites at one spot, as one prompt and a list (RZ_WorkSites.Gather).
class RZ_WorkChoice
{
	int m_iId;						// from the sites' ids: the box's prompt sends it
	string m_sKey;
	string m_sTitle;				// the spot's name: the prompt and the menu's title
	string m_sText;					// the menu's description
	ref array<string> m_aKeys = {};	// its sites, in the menu's order
	vector m_vPos;
	float m_fYaw;
	int m_iOnly;
	IEntity m_Box;
	string m_sArmed;				// the row picked: its site's key ("" the list)
	int m_iArmedBy;
	float m_fArmedAt;
	bool m_bSplit;					// no menu after all: each site its own prompt
	string m_sShown;				// what the box last showed (sent again only when it changes)
}

//------------------------------------------------------------------------------------------------
//! A choice's rows for the object menu: its open sites, by label.
class RZ_WorkChoiceRows : RZ_ObjectRows
{
	string m_sChoice;

	//------------------------------------------------------------------------------------------------
	override void Labels(notnull array<string> labels)
	{
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (sites)
			sites.ChoiceLabels(m_sChoice, labels);
	}

	//------------------------------------------------------------------------------------------------
	override bool Locked(int index, IEntity user, out string reason, out string tag)
	{
		RZ_WorkSites sites = RZ_WorkSites.Get();
		return sites && sites.ChoiceLocked(m_sChoice, index, user, reason, tag);
	}

	//------------------------------------------------------------------------------------------------
	override void Pick(int index, IEntity user)
	{
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (sites)
			sites.ChoicePick(m_sChoice, index, user);
	}

	//------------------------------------------------------------------------------------------------
	override string Text()
	{
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (!sites)
			return "";

		return sites.ChoiceText(m_sChoice);
	}

	//------------------------------------------------------------------------------------------------
	override void Failed(int playerId)
	{
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (sites)
			sites.Split(m_sChoice);
	}
}

//------------------------------------------------------------------------------------------------
class RZ_WorkSites
{
	protected static const float ARMED_FOR = 60;		// a picked row waits this long for its hold, then it's the list again
	protected static const float SLACK = 1.5;			// seconds the server forgives (its 1 s poll, the hold's network trip)
	protected static const float LIFT = 0.6;			// the prompt stands this high over the ground
	protected static const float PERSON_OFF = 1.2;		// the person stands this far behind the site
	protected static const float LIE_LIFT = 0.12;		// someone lying on the work's bed lies this far above its spot (a mattress)
	protected static const float LIE_SIDE = 0.8;		// and its prompt is this far to their side
	protected static const float LIE_DELAY = 600;		// ms after spawning before they're laid down (as The Round's)
	protected static const float PERSON_HALF = 0.3;
	protected static const ResourceName TAKE_FALLBACK = "{1284F9C5D25E263E}Prefabs/Props/Crates/CardboardBox/CardboardBox_01.et";	// the base game's cardboard box (world_kit.txt; footprints.json 0.4 x 0.7 m)		// a person's box: a 0.6 m square (RZ_SiteFoot.SEAT)

	protected RZ_SharedSettings m_Shared;
	protected ref RZ_Carries m_Carries;
	protected ref array<ref RZ_WorkSite> m_aSites = {};
	protected ref array<ref RZ_WorkChoice> m_aChoices = {};
	protected ref map<string, ref RZ_HandsOnGame> m_mHands = new map<string, ref RZ_HandsOnGame>();	// choices shown up close (RZ_HandsOn) instead of as a list
	protected int m_iNextId = 1;
	protected float m_fClock;
	protected float m_fSpawnTimer;
	protected bool m_bWarnedComponent;
	protected bool m_bWarnedCap;
	// Looks' real boxes in their own space (BoxOf), measured once per prefab
	protected ref map<ResourceName, vector> m_mBoxMin = new map<ResourceName, vector>();
	protected ref map<ResourceName, vector> m_mBoxMax = new map<ResourceName, vector>();

	//------------------------------------------------------------------------------------------------
	void RZ_WorkSites(RZ_SharedSettings shared)
	{
		m_Shared = shared;
		m_Carries = new RZ_Carries(this);
	}

	//------------------------------------------------------------------------------------------------
	//! Carrying things somewhere (RZ_Carry.c), built on these sites.
	RZ_Carries Carries()
	{
		return m_Carries;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_WorkSites Get()
	{
		RZ_Shared shared = RZ_Shared.Get();
		if (!shared)
			return null;

		return shared.GetWorkSites();
	}

	//------------------------------------------------------------------------------------------------
	RZ_WorkSiteSettings Settings()
	{
		return m_Shared.WorkSites();
	}

	//------------------------------------------------------------------------------------------------
	//! Work sites are on and have a prompt to carry: owners use them; otherwise their old rule.
	static bool Available()
	{
		RZ_WorkSites sites = Get();
		if (!sites)
			return false;

		RZ_WorkSiteSettings s = sites.Settings();
		return s && s.m_bEnabled && !s.m_sSitePrefab.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! The hold for a step that used to take `stayed` seconds there: 5 s, plus up to 15 s by how long
	//! it was (2 minutes or more: 20 s), so a quicker way (a background's) stays quicker.
	static float HoldFor(float stayed)
	{
		return 5 + 15 * Math.Clamp(stayed / 120, 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! A hold as it's used: kept to 3-30 s, x Time scale.
	static float Scaled(float hold)
	{
		float scale = 1;
		RZ_WorkSites sites = Get();
		if (sites && sites.Settings())
			scale = sites.Settings().m_fTimeScale;

		if (hold <= 0)
			hold = 10;

		return Math.Clamp(hold, 3, 30) * scale;
	}

	//------------------------------------------------------------------------------------------------
	//! The prompt when the owner gives none.
	static string DefaultLabel(RZ_EWork kind)
	{
		switch (kind)
		{
			case RZ_EWork.LATRINE: return "Dig the latrine";
			case RZ_EWork.GRAVE: return "Dig the grave";
			case RZ_EWork.BOARDS: return "Board it up";
			case RZ_EWork.SANDBAGS: return "Fill and stack the sandbags";
			case RZ_EWork.REPAIR: return "Fix it";
			case RZ_EWork.INSTALL: return "Set it up";
			case RZ_EWork.SEARCH: return "Search it";
			case RZ_EWork.TAKE: return "Take them";
			case RZ_EWork.FILL: return "Fill the cans";
			case RZ_EWork.PAINT: return "Paint it";
			case RZ_EWork.WRITE: return "Write it up";
			case RZ_EWork.BURN: return "Burn it";
			case RZ_EWork.CARE: return "Dress the wound";
			case RZ_EWork.BENCH: return "Do the work";
			case RZ_EWork.DIG: return "Dig";
			case RZ_EWork.HANDS: return "Lend a hand";
		}

		return "Work";
	}

	//------------------------------------------------------------------------------------------------
	//! A step's line without its time there, for a step held at a work site: "(30 s there)" goes,
	//! "(20 s there, nothing in your hands)" keeps "(nothing in your hands)". Only the first
	//! parenthesis whose first part is a time.
	static string WithoutTime(string text)
	{
		int open = text.IndexOf("(");
		while (open >= 0)
		{
			int close = text.IndexOfFrom(open, ")");
			if (close < 0)
				return text;

			string inner = text.Substring(open + 1, close - open - 1);
			string first = inner;
			string rest;
			int comma = inner.IndexOf(", ");
			if (comma >= 0)
			{
				first = inner.Substring(0, comma);
				rest = inner.Substring(comma + 2, inner.Length() - comma - 2);
			}

			if (IsTime(first))
			{
				string head = text.Substring(0, open);
				string tail = text.Substring(close + 1, text.Length() - close - 1);
				if (!rest.IsEmpty())
					return head + "(" + rest + ")" + tail;

				if (head.Length() > 0 && head.Substring(head.Length() - 1, 1) == " ")
					head = head.Substring(0, head.Length() - 1);

				return head + tail;
			}

			open = text.IndexOfFrom(close, "(");
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! "30 s there", "2 real minutes there", "a minute", "half a minute at the table".
	protected static bool IsTime(string part)
	{
		if (part.IndexOf("a minute") == 0 || part.IndexOf("half a minute") == 0)
			return true;

		if (part.IsEmpty() || !"0123456789".Contains(part.Substring(0, 1)))
			return false;

		return part.Contains(" s") || part.Contains("minute");
	}

	//------------------------------------------------------------------------------------------------
	//! A site's spot `minR`-`maxR` from `place` that meets the place need `work.site` (docs/PLACES.md:
	//! dry, not steep, not behind a lock), looking twice and four times as far when none does (a job
	//! by the shore); `place` itself only when there's still none.
	static vector Spot(vector place, float minR, float maxR)
	{
		float reach = maxR;
		for (int wider = 0; wider < 3; wider++)
		{
			vector at = RZ_PlaceFind.Near(place, minR, reach, RZ_PlaceNeeds.Get("work.site"));
			if (at != vector.Zero)
				return at;

			reach = reach * 2;
		}

		return place;
	}

	// ================================================================ Owners
	//------------------------------------------------------------------------------------------------
	//! Opens (or reopens, by key) a site at `pos`. hold: the hold in seconds (Scaled is applied);
	//! only: the one player who may do it (0 anyone); before/after: this site's own looks (empty:
	//! the kind's). Returns its id, -1 when work sites aren't available.
	int Open(string key, RZ_EWork kind, string label, float hold, vector pos, int only = 0, ResourceName before = "", ResourceName after = "", float yaw = -1, bool keepHeight = false)
	{
		if (!Available() || key.IsEmpty())
			return -1;

		RZ_WorkSite s = Find(key);
		if (s && !s.m_bClosed)
			return s.m_iId;

		if (s)
			Drop(s);

		MakeRoom();
		s = new RZ_WorkSite();
		s.m_iId = m_iNextId;
		m_iNextId++;
		s.m_sKey = key;
		s.m_eKind = kind;
		s.m_sLabel = label;
		if (label.IsEmpty())
			s.m_sLabel = DefaultLabel(kind);

		s.m_fSeconds = Scaled(hold);
		s.m_vPos = RZ_Util.GroundPos(pos);
		// Indoors (Indoors below): on the building's floor, which the terrain doesn't know
		if (keepHeight && pos[1] > s.m_vPos[1])
		{
			s.m_vPos = pos;
			s.m_bKeepHeight = true;
		}

		s.m_fYaw = yaw;
		if (yaw < 0)
			s.m_fYaw = Math.RandomFloat(0, 360);

		s.m_iOnly = only;
		RZ_WorkLook look = Settings().Look(kind);
		s.m_sBefore = before;
		s.m_sAfter = after;
		if (look && before.IsEmpty())
			s.m_sBefore = look.m_sBefore;

		if (look && after.IsEmpty())
			s.m_sAfter = look.m_sAfter;

		// Something to take always stands at a "Take ..." prompt: with no look or prop, a base-game box
		if (kind == RZ_EWork.TAKE && s.m_sBefore.IsEmpty())
			s.m_sBefore = TAKE_FALLBACK;

		// A door is never spawned in the open: a look that is a door (a boarded doorway) stands only
		// in a real doorway (RZ_DoorGeometry.DoorPropFits), else it's left out
		if (!RZ_DoorGeometry.DoorPropFits(s.m_sBefore, s.m_vPos, key))
			s.m_sBefore = "";

		if (!RZ_DoorGeometry.DoorPropFits(s.m_sAfter, s.m_vPos, key))
			s.m_sAfter = "";

		s.m_fOpenedAt = m_fClock;
		m_aSites.Insert(s);
		RZ_PlaceNeeds.Verify("work.site", s.m_vPos, s.m_sLabel);
		RZ_Log.Info(string.Format("Work sites: %1 \"%2\" (%3, %4 s) at %5", key, s.m_sLabel, typename.EnumToString(RZ_EWork, kind), s.m_fSeconds, RZ_SettlementScanner.GridReference(s.m_vPos)));
		return s.m_iId;
	}

	//------------------------------------------------------------------------------------------------
	//! A faster hold for this player (a background's bonus: seconds / speed). Owners call it when they
	//! learn who it is; the prompt picks it up on that player's machine.
	void SetFast(string key, int playerId, float speed)
	{
		RZ_WorkSite s = Find(key);
		if (!s || s.m_bDone || speed <= 1 || s.m_aFast.Contains(playerId))
			return;

		s.m_fFastSeconds = Math.Max(3, s.m_fSeconds / speed);
		s.m_aFast.Insert(playerId);
		Push(s);
	}

	//------------------------------------------------------------------------------------------------
	//! Someone the work is for stands beside the site while it's open and a player is near (the
	//! newcomer to check over, the bitten man to sit with): a body in the errands' budget, gone with
	//! the site's props. Empty prefab or no room in the budget: nobody, the work is done all the same.
	void SetPerson(string key, ResourceName prefab, bool lies = false)
	{
		RZ_WorkSite s = Find(key);
		if (!s || prefab.IsEmpty())
			return;

		s.m_sPerson = prefab;
		s.m_bPersonLies = lies;
		float rad = s.m_fYaw * Math.DEG2RAD;
		s.m_vPerson = s.m_vPos + Vector(Math.Sin(rad), 0, Math.Cos(rad)) * PERSON_OFF;
	}

	//------------------------------------------------------------------------------------------------
	//! A spot on the ground floor of the building nearest `near` within `radius` (of `kind`: "house",
	//! "shop", "police", "school", "public", "church"...; empty: any): inside its walls, under its
	//! roof, clear of furniture (the loose loot's anchors, RZ_LootAnchors). False when there's no such
	//! building or no room inside: the caller keeps its spot in the open.
	static bool Indoors(vector near, float radius, string kind, out vector at)
	{
		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(near, radius, buildings, 24);
		IEntity best;
		float bestDist = float.MAX;
		array<string> kinds = {kind};
		foreach (IEntity b : buildings)
		{
			if (!kind.IsEmpty() && RZ_OpeningRoll.KindOf(b, kinds).IsEmpty())
				continue;

			float d = RZ_Util.DistXZ(b.GetOrigin(), near);
			if (d < bestDist)
			{
				bestDist = d;
				best = b;
			}
		}

		if (!best)
			return false;

		return FloorOf(best, at);
	}

	//------------------------------------------------------------------------------------------------
	//! A spot on this building's ground floor: one of the loose loot's anchors (RZ_LootAnchors) at its
	//! lowest level, at the floor's height. False: no anchors (a shed, a ruin): nowhere inside.
	static bool FloorOf(IEntity building, out vector at)
	{
		array<vector> anchors = {};
		RZ_LootAnchors.Find(building, anchors, RZ_LootAnchors.MAX_ANCHORS);
		if (anchors.IsEmpty())
			return false;

		// The ground floor: the lowest anchors (a shelf top or an upper storey is no place to work)
		float low = float.MAX;
		foreach (vector a : anchors)
		{
			low = Math.Min(low, a[1]);
		}

		array<vector> floor = {};
		foreach (vector f : anchors)
		{
			if (f[1] <= low + 0.4)
				floor.Insert(f);
		}

		at = floor.GetRandomElement();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! It needs `count` of `item` from the pack: in a choice's menu its row is locked without them,
	//! with why. The owner still checks (and takes them) when it's done.
	void SetNeeds(string key, RZ_EErrandItem item, int count)
	{
		RZ_WorkSite s = Find(key);
		if (!s)
			return;

		s.m_eNeed = item;
		s.m_iNeed = count;
	}

	//------------------------------------------------------------------------------------------------
	//! Done unseen (built stealth): the count runs only while the worker is hidden.
	void SetUnseen(string key)
	{
		RZ_WorkSite s = Find(key);
		if (s)
			s.m_bUnseen = true;
	}

	//------------------------------------------------------------------------------------------------
	bool Has(string key)
	{
		RZ_WorkSite s = Find(key);
		return s && !s.m_bClosed;
	}

	//------------------------------------------------------------------------------------------------
	bool IsDone(string key)
	{
		RZ_WorkSite s = Find(key);
		return s && s.m_bDone;
	}

	//------------------------------------------------------------------------------------------------
	//! Who finished it (0 nobody yet).
	int DoneBy(string key)
	{
		RZ_WorkSite s = Find(key);
		if (!s)
			return 0;

		return s.m_iDoneBy;
	}

	//------------------------------------------------------------------------------------------------
	//! The owner's step is over. An open site goes now; a done one stays (its after look) until
	//! nobody is near.
	void Close(string key)
	{
		RZ_WorkSite s = Find(key);
		if (!s)
			return;

		if (!s.m_bDone)
		{
			Drop(s);
			return;
		}

		s.m_bClosed = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Seconds the server has counted `playerId` at it without a break (0 when they aren't there):
	//! for an owner that times something mid-way (a guard's second look).
	float WorkedFor(string key, int playerId)
	{
		RZ_WorkSite s = Find(key);
		if (!s)
			return 0;

		int idx = s.m_aNear.Find(playerId);
		if (idx < 0)
			return 0;

		return m_fClock - s.m_aSince[idx];
	}

	//------------------------------------------------------------------------------------------------
	//! The site's place (vector.Zero if none).
	vector PosOf(string key)
	{
		RZ_WorkSite s = Find(key);
		if (!s)
			return vector.Zero;

		return s.m_vPos;
	}

	//------------------------------------------------------------------------------------------------
	//! Marks it done for `playerId` without the hold: the runtime test's fake player, which can't
	//! press anything. Not used by the mod itself.
	bool CompleteFor(string key, int playerId)
	{
		RZ_WorkSite s = Find(key);
		if (!s || s.m_bDone)
			return false;

		Finish(s, playerId);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Every open site whose key starts with `prefix`, done for `playerId` (the runtime test's fake
	//! player at a mission's sites), only those within `within` m of `at` when it's above 0 (the
	//! sites he stands at). Returns how many.
	int CompleteAllFor(string prefix, int playerId, vector at = vector.Zero, float within = 0)
	{
		int n = 0;
		foreach (RZ_WorkSite s : m_aSites)
		{
			if (s.m_bDone || s.m_bClosed || s.m_sKey.IndexOf(prefix) != 0)
				continue;

			if (within > 0 && RZ_Util.DistXZ(s.m_vPos, at) > within)
				continue;

			Finish(s, playerId);
			n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Sites open now (the runtime test's check).
	int Count()
	{
		return m_aSites.Count();
	}

	// ================================================================ Choices
	//------------------------------------------------------------------------------------------------
	//! The sites `keys` (opened already, at or about `at`) stand as one prompt named `title` at `at`,
	//! whose press lists them (docs/UI.md, "Default first, then menus"). `text` is the list's
	//! description (what the place is). Called again with the same `key`, it takes the new list and
	//! text (a site gone from it, or reopened under its old key, is fine). Nothing happens without the
	//! object menu (each site keeps its own prompt), or with fewer than two keys. A choice goes by
	//! itself once none of its sites is open.
	void Gather(string key, string title, notnull array<string> keys, vector at, string text = "", float yaw = -1)
	{
		if (!Available() || !RZ_ObjectScreen.Available() || key.IsEmpty())
			return;

		RZ_WorkChoice c = FindChoice(key);
		if (!c)
		{
			if (keys.Count() < 2)
				return;

			c = new RZ_WorkChoice();
			c.m_iId = m_iNextId;
			m_iNextId++;
			c.m_sKey = key;
			c.m_vPos = RZ_Util.GroundPos(at);
			c.m_fYaw = yaw;
			if (yaw < 0)
				c.m_fYaw = Math.RandomFloat(0, 360);

			m_aChoices.Insert(c);
			RZ_Log.Info(string.Format("Work sites: %1 gathered as \"%2\" (%3 things to do) at %4", key, title, keys.Count(), RZ_SettlementScanner.GridReference(c.m_vPos)));
		}

		c.m_sTitle = title;
		c.m_sText = text;
		c.m_aKeys.Clear();
		c.m_iOnly = 0;
		foreach (string k : keys)
		{
			c.m_aKeys.Insert(k);
			RZ_WorkSite s = Find(k);
			if (!s)
				continue;

			c.m_iOnly = s.m_iOnly;
			// Its own box goes: the choice's stands for it
			if (!c.m_bSplit && s.m_Box)
			{
				RZ_Util.DeleteUnit(s.m_Box);
				s.m_Box = null;
			}
		}

		PushChoice(c);
	}

	//------------------------------------------------------------------------------------------------
	//! The choice's description, changed (looks left, what's been seen): an open menu shows it.
	void SetChoiceText(string key, string text)
	{
		RZ_WorkChoice c = FindChoice(key);
		if (c)
			c.m_sText = text;
	}

	//------------------------------------------------------------------------------------------------
	//! The choice `key` opens up close in the hands-on view (Interactions/RZ_HandsOn.c) for players
	//! whose client can show it; the list stays for the rest. Null takes it off.
	void SetHandsOn(string key, RZ_HandsOnGame game)
	{
		if (game)
			m_mHands.Set(key, game);
		else
			m_mHands.Remove(key);
	}

	//------------------------------------------------------------------------------------------------
	//! As if the player had picked site `siteKey` from the choice's list: its prompt waits for their
	//! hold (the hands-on view's buttons that are work done with the body, like taking someone up).
	void ArmChoice(string key, string siteKey, int playerId)
	{
		RZ_WorkChoice c = FindChoice(key);
		RZ_WorkSite s = Find(siteKey);
		if (!c || !s || s.m_bDone || s.m_bClosed)
			return;

		c.m_sArmed = siteKey;
		c.m_iArmedBy = playerId;
		c.m_fArmedAt = m_fClock;
		PushChoice(c);
	}

	//------------------------------------------------------------------------------------------------
	//! The choice is put away now (its sites are the owner's to close).
	void Scatter(string key)
	{
		RZ_WorkChoice c = FindChoice(key);
		if (c)
			DropChoice(c);
	}

	//------------------------------------------------------------------------------------------------
	//! The open sites of a choice, in order.
	protected void OpenOf(RZ_WorkChoice c, notnull array<RZ_WorkSite> open)
	{
		open.Clear();
		foreach (string k : c.m_aKeys)
		{
			RZ_WorkSite s = Find(k);
			if (s && !s.m_bDone && !s.m_bClosed)
				open.Insert(s);
		}
	}

	//------------------------------------------------------------------------------------------------
	void ChoiceLabels(string key, notnull array<string> labels)
	{
		labels.Clear();
		RZ_WorkChoice c = FindChoice(key);
		if (!c)
			return;

		array<RZ_WorkSite> open = {};
		OpenOf(c, open);
		foreach (RZ_WorkSite s : open)
		{
			labels.Insert(s.m_sLabel);
		}
	}

	//------------------------------------------------------------------------------------------------
	string ChoiceText(string key)
	{
		RZ_WorkChoice c = FindChoice(key);
		if (!c)
			return "";

		return c.m_sText;
	}

	//------------------------------------------------------------------------------------------------
	bool ChoiceLocked(string key, int index, IEntity user, out string reason, out string tag)
	{
		RZ_WorkChoice c = FindChoice(key);
		if (!c)
			return false;

		array<RZ_WorkSite> open = {};
		OpenOf(c, open);
		if (index < 0 || index >= open.Count())
			return false;

		RZ_WorkSite s = open[index];
		if (s.m_iNeed <= 0 || RZ_Errands.CountItems(user, s.m_eNeed) >= s.m_iNeed)
			return false;

		reason = string.Format("You haven't got that on you: %1.", RZ_Errands.ItemNeed(s.m_eNeed, s.m_iNeed));
		tag = "NEEDS";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A row picked: the box is that row's hold now, for whoever picked it, and the menu closes so
	//! the hands can do it.
	void ChoicePick(string key, int index, IEntity user)
	{
		RZ_WorkChoice c = FindChoice(key);
		if (!c || !user)
			return;

		array<RZ_WorkSite> open = {};
		OpenOf(c, open);
		if (index < 0 || index >= open.Count())
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		c.m_sArmed = open[index].m_sKey;
		c.m_iArmedBy = playerId;
		c.m_fArmedAt = m_fClock;
		PushChoice(c);
		RZ_ObjectScreen.CloseFor(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! No menu for this choice after all (a client couldn't show one): each site stands as its own
	//! prompt, as without the object menu.
	void Split(string key)
	{
		RZ_WorkChoice c = FindChoice(key);
		if (!c || c.m_bSplit)
			return;

		c.m_bSplit = true;
		DeleteChoiceBox(c);
		m_fSpawnTimer = 2;
		RZ_Log.Info(string.Format("Work sites: %1 split into its prompts (no object menu on the client).", key));
	}

	//------------------------------------------------------------------------------------------------
	//! The box was pressed in its list form (RZ_WorkSiteAction, on the server): the menu opens.
	protected void OpenChoice(RZ_WorkChoice c, IEntity user)
	{
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		if (playerId <= 0 || !c.m_Box)
			return;

		if (c.m_iOnly > 0 && playerId != c.m_iOnly)
		{
			RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
			if (gm)
				gm.NotifyPlayer(playerId, c.m_sTitle, "This isn't your job. Whoever took it has to do it.");

			return;
		}

		// Up close, where the game has a view of it and this player's client can show one
		RZ_HandsOnGame hands = m_mHands.Get(c.m_sKey);
		if (hands && RZ_HandsOn.CanShow(playerId))
		{
			RZ_HandsOn.Open(playerId, hands);
			return;
		}

		if (!RZ_ObjectScreen.MenuFor(playerId))
		{
			Split(c.m_sKey);
			return;
		}

		RZ_WorkChoiceRows rows = new RZ_WorkChoiceRows();
		rows.m_sChoice = c.m_sKey;
		RZ_ObjectScreen.OpenRows(playerId, c.m_Box, c.m_sTitle, rows);
	}

	//------------------------------------------------------------------------------------------------
	//! The choice this site is one of, when that stands as one box (none of the site's own then).
	protected RZ_WorkChoice ChoiceOf(RZ_WorkSite s)
	{
		foreach (RZ_WorkChoice c : m_aChoices)
		{
			if (!c.m_bSplit && c.m_aKeys.Contains(s.m_sKey))
				return c;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The box's prompt: the list, the row picked, or the one left.
	protected void PushChoice(RZ_WorkChoice c)
	{
		if (!c.m_Box)
			return;

		RZ_WorkSiteComponent comp = RZ_WorkSiteComponent.Cast(c.m_Box.FindComponent(RZ_WorkSiteComponent));
		if (!comp)
			return;

		array<RZ_WorkSite> open = {};
		OpenOf(c, open);
		RZ_WorkSite one;
		if (open.Count() == 1)
			one = open[0];

		foreach (RZ_WorkSite s : open)
		{
			if (s.m_sKey == c.m_sArmed)
				one = s;
		}

		string shown = "list";
		if (one)
			shown = one.m_sKey;

		shown = shown + "|" + open.Count().ToString() + "|" + c.m_sTitle;
		if (shown == c.m_sShown)
			return;

		c.m_sShown = shown;
		if (one)
		{
			int only = one.m_iOnly;
			if (one.m_sKey == c.m_sArmed && c.m_iArmedBy > 0)
				only = c.m_iArmedBy;

			comp.Set(one.m_iId, one.m_sLabel, one.m_fSeconds, one.m_fFastSeconds, one.FastText(), only, LookFor(one.m_eKind));
			return;
		}

		comp.Set(c.m_iId, c.m_sTitle, 0, 0, "", c.m_iOnly, 0, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Every 2 s with the props: a choice with nothing open goes, its box stands near players, and a
	//! row picked and left (walked off, or a minute) is the list again.
	protected void TickChoices(array<IEntity> players, array<int> ids, float spawn, float despawn)
	{
		float reach = Settings().m_fReach + 1;
		for (int i = m_aChoices.Count() - 1; i >= 0; i--)
		{
			RZ_WorkChoice c = m_aChoices[i];
			array<RZ_WorkSite> open = {};
			OpenOf(c, open);
			if (open.IsEmpty())
			{
				DropChoice(c);
				continue;
			}

			if (c.m_bSplit)
				continue;

			if (!AnyNear(players, c.m_vPos, despawn))
			{
				DeleteChoiceBox(c);
				continue;
			}

			if (!c.m_Box && AnyNear(players, c.m_vPos, spawn))
			{
				vector at = c.m_vPos;
				at[1] = at[1] + LIFT;
				c.m_Box = RZ_Util.SpawnPrefab(Settings().m_sSitePrefab, at, c.m_fYaw);
				c.m_sShown = "";
			}

			if (!c.m_sArmed.IsEmpty())
			{
				bool stays = m_fClock - c.m_fArmedAt < ARMED_FOR;
				RZ_WorkSite armed = Find(c.m_sArmed);
				if (!armed || armed.m_bDone || armed.m_bClosed)
					stays = false;

				int idx = ids.Find(c.m_iArmedBy);
				if (idx < 0 || !players[idx] || RZ_Util.DistXZ(players[idx].GetOrigin(), c.m_vPos) > reach)
					stays = false;

				if (!stays)
					c.m_sArmed = "";
			}

			PushChoice(c);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteChoiceBox(RZ_WorkChoice c)
	{
		if (c.m_Box)
			RZ_Util.DeleteUnit(c.m_Box);

		c.m_Box = null;
		c.m_sShown = "";
	}

	//------------------------------------------------------------------------------------------------
	protected void DropChoice(RZ_WorkChoice c)
	{
		RZ_HandsOnGame hands = m_mHands.Get(c.m_sKey);
		if (hands)
		{
			RZ_HandsOn.CloseGame(hands);
			m_mHands.Remove(c.m_sKey);
		}

		DeleteChoiceBox(c);
		m_aChoices.RemoveItem(c);
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_WorkChoice FindChoice(string key)
	{
		foreach (RZ_WorkChoice c : m_aChoices)
		{
			if (c.m_sKey == key)
				return c;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_WorkChoice FindChoiceId(int id)
	{
		foreach (RZ_WorkChoice c : m_aChoices)
		{
			if (c.m_iId == id)
				return c;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The prop up at the site now (its box while open, its after look once done), or null.
	IEntity PropOf(string key)
	{
		RZ_WorkSite s = Find(key);
		if (!s)
			return null;

		if (s.m_bDone)
			return s.m_AfterProp;

		if (!s.m_Box)
		{
			RZ_WorkChoice c = ChoiceOf(s);
			if (c)
				return c.m_Box;
		}

		return s.m_Box;
	}

	// ================================================================ The hold
	//------------------------------------------------------------------------------------------------
	//! The hold finished on `user`'s machine (RZ_WorkSiteAction.PerformAction, on the server).
	void TryComplete(int siteId, IEntity user)
	{
		RZ_WorkSite s = FindId(siteId);
		if (!s && user)
		{
			RZ_WorkChoice choice = FindChoiceId(siteId);
			if (choice)
				OpenChoice(choice, user);

			return;
		}

		if (!s || s.m_bDone || !user)
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		if (playerId <= 0)
			return;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (s.m_iOnly > 0 && playerId != s.m_iOnly)
		{
			if (gm)
				gm.NotifyPlayer(playerId, s.m_sLabel, "This isn't your job. Whoever took it has to do it.");

			return;
		}

		if (s.m_bUnseen && !RZ_Stealth.IsHidden(playerId))
		{
			if (gm)
				gm.NotifyPlayer(playerId, s.m_sLabel, "They'd see you like that. Get low, keep out of the light, and start again.");

			return;
		}

		int idx = s.m_aNear.Find(playerId);
		float need = s.SecondsFor(playerId) - SLACK;
		if (idx < 0 || m_fClock - s.m_aSince[idx] < need)
		{
			if (gm)
				gm.NotifyPlayer(playerId, s.m_sLabel, string.Format("Not finished. It takes about %1 s at it without stopping, and a knock starts it again.", RZ_Util.RoundToInt(s.SecondsFor(playerId))));

			return;
		}

		Finish(s, playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void Finish(RZ_WorkSite s, int playerId)
	{
		s.m_bDone = true;
		s.m_iDoneBy = playerId;
		DeleteProps(s);
		if (HasPlayerNear(s.m_vPos, DespawnRadius()))
			SpawnAfter(s);

		RZ_Log.Info(string.Format("Work sites: %1 \"%2\" done by %3", s.m_sKey, s.m_sLabel, RZ_Util.PlayerName(playerId)));
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	//! Once a second (RZ_Shared.SecondTick): who is at which site, and props near players.
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		m_fClock += dt;
		// Carrying first: a put-down's own prompt is a site, so the workers below see it this second
		m_Carries.Tick(ids, players, SpawnRadius(), DespawnRadius());
		if (m_aSites.IsEmpty())
			return;

		WatchWorkers(ids, players);
		m_fSpawnTimer += dt;
		if (m_fSpawnTimer < 2)
			return;

		m_fSpawnTimer = 0;
		float spawn = SpawnRadius();
		float despawn = DespawnRadius();
		TickChoices(players, ids, spawn, despawn);
		for (int i = m_aSites.Count() - 1; i >= 0; i--)
		{
			RZ_WorkSite s = m_aSites[i];
			bool near = AnyNear(players, s.m_vPos, spawn);
			bool far = !AnyNear(players, s.m_vPos, despawn);
			if (s.m_bClosed && far)
			{
				Drop(s);
				continue;
			}

			if (far)
			{
				DeleteProps(s);
				continue;
			}

			if (!near)
				continue;

			if (s.m_bDone)
			{
				if (!s.m_AfterProp)
					SpawnAfter(s);

				continue;
			}

			// One of a choice: its look stands, its prompt is the choice's box
			if (ChoiceOf(s))
			{
				if (s.m_Box)
				{
					RZ_Util.DeleteUnit(s.m_Box);
					s.m_Box = null;
				}

				if (!s.m_BeforeProp && !s.m_sBefore.IsEmpty())
					s.m_BeforeProp = RZ_Util.SpawnPrefab(s.m_sBefore, s.m_vPos, s.m_fYaw);

				continue;
			}

			if (!s.m_Box)
				SpawnBefore(s);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Each spawned open site: a player within reach is working there since they came; one who left
	//! or was hurt starts again.
	protected void WatchWorkers(array<int> ids, array<IEntity> players)
	{
		float reach = Settings().m_fReach;
		foreach (RZ_WorkSite s : m_aSites)
		{
			// One of a choice is worked at the choice's box
			vector where = s.m_vPos;
			bool up = s.m_Box != null;
			RZ_WorkChoice choice = ChoiceOf(s);
			if (choice)
			{
				where = choice.m_vPos;
				up = choice.m_Box != null;
			}

			if (s.m_bDone || !up)
			{
				s.m_aNear.Clear();
				s.m_aSince.Clear();
				s.m_aHealth.Clear();
				continue;
			}

			for (int k = s.m_aNear.Count() - 1; k >= 0; k--)
			{
				int at = ids.Find(s.m_aNear[k]);
				if (at < 0 || RZ_Util.DistXZ(players[at].GetOrigin(), where) > reach)
				{
					s.m_aNear.Remove(k);
					s.m_aSince.Remove(k);
					s.m_aHealth.Remove(k);
				}
			}

			foreach (int i, int playerId : ids)
			{
				IEntity p = players[i];
				if (!p || RZ_Util.DistXZ(p.GetOrigin(), where) > reach)

					continue;

				float health = RZ_Util.GetCharacterHealth(p);
				int idx = s.m_aNear.Find(playerId);
				if (idx < 0)
				{
					s.m_aNear.Insert(playerId);
					s.m_aSince.Insert(m_fClock);
					s.m_aHealth.Insert(health);
					continue;
				}

				// Hurt, or seen at unseen work: the count starts again from now
				if (health >= 0 && s.m_aHealth[idx] >= 0 && health < s.m_aHealth[idx] - 0.01)
					s.m_aSince[idx] = m_fClock;

				if (s.m_bUnseen && !RZ_Stealth.IsHidden(playerId))
					s.m_aSince[idx] = m_fClock;

				s.m_aHealth[idx] = health;
			}
		}
	}

	// ================================================================ Props
	//------------------------------------------------------------------------------------------------
	protected void SpawnBefore(RZ_WorkSite s)
	{
		if (!s.m_sBefore.IsEmpty() && s.m_iFit == 0)
			FitLook(s);

		if (!s.m_sBefore.IsEmpty() && s.m_iFit > 0 && !s.m_BeforeProp)
		{
			s.m_BeforeProp = RZ_Util.SpawnPrefab(s.m_sBefore, s.m_vPos, s.m_fYaw);
			// A scene from the world kit can carry things a player could use: they go
			if (s.m_BeforeProp && !RZ_DressingGuard.Strip(s.m_BeforeProp, s.m_sBefore))
				s.m_BeforeProp = null;
		}

		SpawnPerson(s);

		vector at = s.m_vPos;
		// Someone lying on the spot: the prompt at their side, where you'd kneel to them
		if (s.m_bPersonLies && s.m_Person)
		{
			float side = (s.m_fYaw + 90) * Math.DEG2RAD;
			at = at + Vector(Math.Sin(side), 0, Math.Cos(side)) * LIE_SIDE;
		}

		at[1] = at[1] + LIFT;
		s.m_Box = RZ_Util.SpawnPrefab(Settings().m_sSitePrefab, at, s.m_fYaw);
		Push(s);
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnAfter(RZ_WorkSite s)
	{
		if (s.m_sAfter.IsEmpty() || s.m_AfterProp)
			return;

		// Its own box where the work was (the before look is gone by now)
		vector amin;
		vector amax;
		if (!BoxOf(s.m_sAfter, s.m_vPos, amin, amax) || !RZ_CampLife.BoxFits(s.m_vPos, s.m_fYaw, amin, amax, s.m_bKeepHeight))
		{
			RZ_Log.Info(string.Format("Work sites: %1 after look %2 doesn't fit at %3, left out", s.m_sKey, RZ_Util.PrettyPrefabName(s.m_sAfter), RZ_SettlementScanner.GridReference(s.m_vPos)));
			s.m_sAfter = "";
			return;
		}

		s.m_AfterProp = RZ_Util.SpawnPrefab(s.m_sAfter, s.m_vPos, s.m_fYaw);
		if (s.m_AfterProp && !RZ_DressingGuard.Strip(s.m_AfterProp, s.m_sAfter))
			s.m_AfterProp = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Where the before look goes (docs/DESIGN.md, "Work site"): its real box must fit, as a camp's
	//! piece must (RZ_CampLife.BoxFits). Its own spot and heading first, then the other headings, then
	//! spots around it (up to 3 m outdoors, 0.75 m indoors). Nowhere: a small box stands there instead
	//! (the base game's cardboard box), and only when not even that fits does the site keep only its
	//! prompt (logged). The site moves with its look.
	protected void FitLook(RZ_WorkSite s)
	{
		vector fmin;
		vector fmax;
		s.m_iFit = -1;
		if (!BoxOf(s.m_sBefore, s.m_vPos, fmin, fmax))
		{
			s.m_sBefore = "";
			return;
		}

		float step = 1;
		int rings = 4;
		if (s.m_bKeepHeight)
		{
			step = 0.75;
			rings = 2;
		}

		for (int ring = 0; ring < rings; ring++)
		{
			int around = 1;
			if (ring > 0)
				around = 8;

			for (int k = 0; k < around; k++)
			{
				float rad = (k * 45 + s.m_fYaw) * Math.DEG2RAD;
				vector spot = s.m_vPos + Vector(Math.Sin(rad), 0, Math.Cos(rad)) * (ring * step);
				if (!s.m_bKeepHeight)
					spot = RZ_Util.GroundPos(spot);

				for (int turn = 0; turn < 4; turn++)
				{
					float yaw = s.m_fYaw + turn * 90;
					if (!RZ_CampLife.BoxFits(spot, yaw, fmin, fmax, s.m_bKeepHeight))
						continue;

					s.m_iFit = 1;
					if (!s.m_sPerson.IsEmpty())
						s.m_vPerson = s.m_vPerson + (spot - s.m_vPos);

					s.m_vPos = spot;
					s.m_fYaw = yaw;
					RZ_Log.Info(string.Format("Work sites: %1 look %2 fits at %3 (%4 m off, heading %5)", s.m_sKey, RZ_Util.PrettyPrefabName(s.m_sBefore), RZ_SettlementScanner.GridReference(spot), ring * step, RZ_Util.RoundToInt(yaw) % 360));
					return;
				}
			}
		}

		// Nowhere for it: a small box stands there instead (once), so the prompt is never on bare ground
		if (s.m_sBefore != TAKE_FALLBACK)
		{
			RZ_Log.Info(string.Format("Work sites: %1 look %2 doesn't fit at %3, a box instead", s.m_sKey, RZ_Util.PrettyPrefabName(s.m_sBefore), RZ_SettlementScanner.GridReference(s.m_vPos)));
			s.m_sBefore = TAKE_FALLBACK;
			FitLook(s);
			return;
		}

		RZ_Log.Info(string.Format("Work sites: %1 has nothing standing at it at %2 (not even a box fits): only its prompt", s.m_sKey, RZ_SettlementScanner.GridReference(s.m_vPos)));
		s.m_sBefore = "";
	}

	//------------------------------------------------------------------------------------------------
	//! A look's real box in its own space, measured on the server the first time it's needed: spawned
	//! 60 m under the spot at heading 0, its parts' boxes joined, then deleted (the measure the site
	//! audit gives the camps' props, docs/SITES.md "Real footprints"). False: it doesn't spawn.
	protected bool BoxOf(ResourceName prefab, vector near, out vector fmin, out vector fmax)
	{
		if (m_mBoxMin.Contains(prefab))
		{
			fmin = m_mBoxMin.Get(prefab);
			fmax = m_mBoxMax.Get(prefab);
			return true;
		}

		vector below = near;
		below[1] = below[1] - 60;
		IEntity e = RZ_Util.SpawnPrefab(prefab, below, 0);
		if (!e)
			return false;

		array<vector> span = {Vector(float.MAX, float.MAX, float.MAX), Vector(-float.MAX, -float.MAX, -float.MAX)};
		JoinBounds(e, span, 0);
		SCR_EntityHelper.DeleteEntityAndChildren(e);
		fmin = span[0] - below;
		fmax = span[1] - below;
		// Nothing with a box (an empty root): half a metre across
		if (fmax[0] - fmin[0] < 0.1 || fmax[2] - fmin[2] < 0.1)
		{
			fmin = Vector(-0.25, 0, -0.25);
			fmax = Vector(0.25, 0.5, 0.25);
		}

		m_mBoxMin.Set(prefab, fmin);
		m_mBoxMax.Set(prefab, fmax);
		RZ_Log.Info(string.Format("Work sites: measured %1: %2 x %3 m", RZ_Util.PrettyPrefabName(prefab), fmax[0] - fmin[0], fmax[2] - fmin[2]));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void JoinBounds(IEntity e, notnull array<vector> span, int depth)
	{
		if (!e || depth > 6)
			return;

		vector wmin;
		vector wmax;
		e.GetWorldBounds(wmin, wmax); // VERIFY: a composition's root and parts give their world boxes right after spawning, so a look spawned at heading 0 measures as its local box
		if (wmax[0] - wmin[0] > 0.01 || wmax[2] - wmin[2] > 0.01)
		{
			vector lo = span[0];
			vector hi = span[1];
			for (int i = 0; i < 3; i++)
			{
				lo[i] = Math.Min(lo[i], wmin[i]);
				hi[i] = Math.Max(hi[i], wmax[i]);
			}

			span[0] = lo;
			span[1] = hi;
		}

		IEntity child = e.GetChildren();
		while (child)
		{
			JoinBounds(child, span, depth + 1);
			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnPerson(RZ_WorkSite s)
	{
		if (s.m_sPerson.IsEmpty() || s.m_Person || s.m_bDone)
			return;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		RZ_Shared shared;
		if (gm)
			shared = gm.GetShared();

		if (shared && !shared.CanSpawnBody(RZ_BodyBudget.ERRAND, s.m_vPerson))
			return;

		// Lying on the work's own spot (its bed: the mattress the care look puts there), laid down as
		// The Round's sick are; the prompt stands at their side (SpawnBefore)
		if (s.m_bPersonLies)
		{
			s.m_vPerson = s.m_vPos + Vector(0, LIE_LIFT, 0);
			s.m_Person = RZ_Util.SpawnPrefab(s.m_sPerson, s.m_vPerson, s.m_fYaw);
			if (!s.m_Person)
				return;

			if (shared)
				shared.AddBody(RZ_BodyBudget.ERRAND, s.m_Person);

			// VERIFY: laid down on the mattress (The Round's LieDown), he stays on it, not through it, and the hold prompt 0.8 m to his side stays reachable over his body
			GetGame().GetCallqueue().CallLater(RZ_DutyRound.LieDown, LIE_DELAY, false, s.m_Person);
			return;
		}

		// Behind the work, else to either side or in front: wherever a person's box fits
		vector half = Vector(PERSON_HALF, 0, PERSON_HALF);
		vector at;
		float face = -1;
		for (int side = 0; side < 4 && face < 0; side++)
		{
			float bearing = s.m_fYaw + side * 90;
			float rad = bearing * Math.DEG2RAD;
			vector spot = s.m_vPos + Vector(Math.Sin(rad), 0, Math.Cos(rad)) * PERSON_OFF;
			if (!s.m_bKeepHeight)
				spot = RZ_Util.GroundPos(spot);

			if (RZ_CampLife.BoxFits(spot, bearing, -half, half, s.m_bKeepHeight))
			{
				at = spot;
				face = bearing + 180;
			}
		}

		if (face < 0)
		{
			RZ_Log.Info(string.Format("Work sites: %1 has no room for its person at %2, left out", s.m_sKey, RZ_SettlementScanner.GridReference(s.m_vPos)));
			s.m_sPerson = "";
			return;
		}

		// Facing the work
		s.m_vPerson = at;
		s.m_Person = RZ_Util.SpawnPrefab(s.m_sPerson, at, face);
		if (s.m_Person && shared)
			shared.AddBody(RZ_BodyBudget.ERRAND, s.m_Person);
	}

	//------------------------------------------------------------------------------------------------
	//! The box's prompt: label, time, who.
	protected void Push(RZ_WorkSite s)
	{
		if (!s.m_Box)
			return;

		RZ_WorkSiteComponent comp = RZ_WorkSiteComponent.Cast(s.m_Box.FindComponent(RZ_WorkSiteComponent));
		if (!comp)
		{
			if (!m_bWarnedComponent)
			{
				m_bWarnedComponent = true;
				RZ_Log.Warn("Work sites: the Site prefab has no RZ_WorkSiteComponent, so it shows no prompt (docs/WORKBENCH_SETUP.md, Work sites).");
			}

			return;
		}

		comp.Set(s.m_iId, s.m_sLabel, s.m_fSeconds, s.m_fFastSeconds, s.FastText(), s.m_iOnly, LookFor(s.m_eKind));
	}

	//------------------------------------------------------------------------------------------------
	//! How a hold at this kind of work looks on the holder's machine (RZ_WorkHold.LOOK_* bits): the
	//! lock, the hands, and digging with an entrenching tool where the work is digging.
	protected int LookFor(RZ_EWork kind)
	{
		RZ_WorkSiteSettings cfg = Settings();
		if (!cfg)
			return 0;

		int look = 0;
		if (cfg.m_bLockMovement)
			look = look | RZ_WorkHold.LOOK_LOCK;

		if (cfg.m_bHandsAnimation)
			look = look | RZ_WorkHold.LOOK_HANDS;

		if (cfg.m_bDigWithTool && (kind == RZ_EWork.LATRINE || kind == RZ_EWork.GRAVE || kind == RZ_EWork.DIG))
			look = look | RZ_WorkHold.LOOK_DIG;

		return look;
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteProps(RZ_WorkSite s)
	{
		if (s.m_Box)
			RZ_Util.DeleteUnit(s.m_Box);

		if (s.m_BeforeProp)
			RZ_Util.DeleteUnit(s.m_BeforeProp);

		if (s.m_AfterProp)
			RZ_Util.DeleteUnit(s.m_AfterProp);

		if (s.m_Person)
			RZ_Util.DeleteUnit(s.m_Person);

		s.m_Box = null;
		s.m_BeforeProp = null;
		s.m_AfterProp = null;
		s.m_Person = null;
	}

	//------------------------------------------------------------------------------------------------
	protected void Drop(RZ_WorkSite s)
	{
		DeleteProps(s);
		m_aSites.RemoveItem(s);
	}

	//------------------------------------------------------------------------------------------------
	//! At the cap: the oldest closed site goes, else the oldest done one.
	protected void MakeRoom()
	{
		if (m_aSites.Count() < Settings().m_iMaxSites)
			return;

		foreach (RZ_WorkSite s : m_aSites)
		{
			if (s.m_bClosed)
			{
				Drop(s);
				return;
			}
		}

		// None closed: every one is a step still under way, so the cap gives (logged once)
		if (!m_bWarnedCap)
		{
			m_bWarnedCap = true;
			RZ_Log.Info(string.Format("Work sites: %1 open at once, past Sites at once; none dropped (each is a step under way).", m_aSites.Count()));
		}
	}

	// ================================================================ Helpers
	//------------------------------------------------------------------------------------------------
	protected RZ_WorkSite Find(string key)
	{
		foreach (RZ_WorkSite s : m_aSites)
		{
			if (s.m_sKey == key)
				return s;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_WorkSite FindId(int id)
	{
		foreach (RZ_WorkSite s : m_aSites)
		{
			if (s.m_iId == id)
				return s;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected float SpawnRadius()
	{
		return m_Shared.m_fMarkSpawnRadius;
	}

	//------------------------------------------------------------------------------------------------
	protected float DespawnRadius()
	{
		return Math.Max(m_Shared.m_fMarkDespawnRadius, m_Shared.m_fMarkSpawnRadius + 20);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool AnyNear(array<IEntity> players, vector pos, float radius)
	{
		foreach (IEntity p : players)
		{
			if (p && RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool HasPlayerNear(vector pos, float radius)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		return AnyNear(players, pos, radius);
	}
}
