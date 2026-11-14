//! Lived-in outposts and camps (docs/DESIGN.md, "Camp life").
//!
//! While a place's bodies exist (territory holdings, Haven safehouses, road posts: a player is
//! near), its faction's scenery stands around it: compositions from Camp life > Kits. Each kit
//! is a plan, the camp drawn whole (tools/camp_kit/pieces.py). With Solver on (docs/SITES.md) the
//! plan is a preference: RZ_SiteSolver lays the site out in layers by its faction's doctrine (the
//! wire, the boards, the watch, then work and life) by each piece's rules. Off, the plan is laid
//! down whole: turned to face the road on open ground; by the building in its town that suits its faction
//! (Bastion a school, town hall or police station; the Red Hand a garage or petrol station; the
//! Haven a church or a big house; Halcyon the lab or a house), in its yard with the plan's front
//! against the wall, the approach pieces at the road entries (the gate on the road in from the
//! next town, barricades at the others) and the door pieces by the front door; at a road post
//! beside the road. Every piece's whole real footprint must fit, tested on the site's ground read
//! on a 0.5 m grid (RZ_SiteGrid, docs/SITES.md): dry, flat, off the roads, out of every building,
//! clear of walls, trees and the other pieces; one that doesn't is dropped, never nudged or moved
//! elsewhere. A quartermaster's counter stands in front of him, and he turns to serve over it.
//! Always from a seed (the place's position). The grid is read a little each second the first
//! time a run dresses a place; the layout is kept for the run (and audited: RZ_SiteAudit), so
//! later visits lay it out at once. Pieces spawn a few per tick. Nothing is saved, and the pieces
//! go with the bodies. RZ_DressingGuard strips anything a player could
//! use for gain, and the interaction scan ignores the pieces (IsDressing).
//!
//! At the Live places nearest players, the people get jobs from their faction's routine:
//! the sentry, the patrol, sitting at the fire, work at the line or the cold box, washing in the
//! morning, chopping wood by day, cooking before the meal, gathering for the roll or the meal,
//! sleeping (off-shift bodies despawn while nobody is close). Jobs are
//! vanilla waypoints and smart actions; combat is the AI's own. Elsewhere, bodies stand.

//------------------------------------------------------------------------------------------------
enum RZ_ECampPlace
{
	ANY,		// a kit for every kind of place of its faction
	STRONGHOLD,
	OUTPOST,	// Halcyon's outposts are its field stations
	SAFEHOUSE,	// Haven safehouses
	ROAD_POST	// Bastion checkpoints and Red Hand roadblocks
}

//------------------------------------------------------------------------------------------------
enum RZ_EDressRole
{
	CENTRE,		// the working area: the fire, the table, the roster board
	INNER,		// the sleeping and the cooking areas: tents, the cook pot, the wash stand
	APPROACH,	// the perimeter, toward the road in: the nest, the barrier, barricades (by a building: the road entries)
	EDGE		// the far side: the washing line, the garden, the generator, the latrine
}

//------------------------------------------------------------------------------------------------
enum RZ_EDressUse
{
	NONE,		// scenery only
	SIT,		// sit sentinels (tag RZ_Sit): the fire pit, a table, the card table, a field table
	WORK,		// loiter points (RZ_Work): the washing line, the garden, the water barrel, the cold box
	WATCH,		// an observation sentinel facing out (RZ_Watch): the sentry nest
	GATHER,		// where people stand for the roll or one at a time by day (RZ_Gather): the roster board
	LEAN,		// lean points (RZ_Lean): the fire barrel, the toll barrier
	SLEEP,		// tents: off-shift people sleep here (as data); extras come out of them
	COOK,		// a cook point (RZ_Cook): the cook pot; someone cooks before the meal
	WASH,		// a wash point (RZ_Wash): the wash stand, the shaving stand, the decon shower; mornings
	CHOP		// a chop point (RZ_Chop): the chopping block; someone chops wood by day
}

//------------------------------------------------------------------------------------------------
//! Where a piece stands when its place has moved into a building (RZ_CampSite). Auto: an approach
//! piece at the road entries (a watch or lean piece the gate, the rest the closed ones), anything
//! else in the yard where the plan draws it. Counter is the same everywhere.
enum RZ_EDressAt
{
	AUTO,
	YARD,		// in the yard, where the plan draws it: behind the building (the Red Hand: its forecourt)
	FRONT,		// along the front wall, facing out: a market stall, a trophy wall
	DOOR,		// beside the front door, facing out: the bear board, the memorial, the decon shower
	GATE,		// beside the road at the entry left open (the road in from the next town): the nest, the toll barrier, the sign
	CLOSED,		// beside the road at the other road entries: barricades
	COUNTER		// in front of a quartermaster, where he stands (indoors too): his counter. Not by the plan
}

//------------------------------------------------------------------------------------------------
//! One piece in a kit's plan. Written by tools/camp_kit/build.py from pieces.py.
[BaseContainerProps()]
class RZ_DressingPiece
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "A composition from Prefabs/ReforgerZ/Dressing/ (docs/WORKBENCH_SETUP.md, Camp life): scenery only. Anything a player could use is deleted when it spawns", "et")]
	ResourceName m_sPrefab;

	[Attribute("0", UIWidgets.ComboBox, "Its part of the camp: Centre (the working area), Inner (sleeping, cooking), Approach (the perimeter; by a building, the road entries), Edge (the far side)", "", ParamEnumArray.FromEnum(RZ_EDressRole))]
	RZ_EDressRole m_eRole;

	[Attribute("0", UIWidgets.ComboBox, "What people do at it (its sentinels carry the matching tag: RZ_Sit, RZ_Work, RZ_Watch, RZ_Gather, RZ_Lean, RZ_Cook, RZ_Wash, RZ_Chop). Sleep: a tent. None: scenery only", "", ParamEnumArray.FromEnum(RZ_EDressUse))]
	RZ_EDressUse m_eUse;

	[Attribute("2", UIWidgets.Slider, "People who use one at once: the number of sentinels with its tag", "1 6 1")]
	int m_iSeats;

	[Attribute("", desc: "Smart-action tag on its sentinels if not the use's own (RZ_Sit, RZ_Work, RZ_Watch, RZ_Gather, RZ_Lean, RZ_Cook, RZ_Wash, RZ_Chop)")]
	string m_sTag;

	[Attribute("0", UIWidgets.ComboBox, "Where it stands when the place has moved into a building: Auto (approach pieces at the road entries, the rest in the yard as planned), Yard, Front (along the front wall), Door (by the front door), Gate (the road entry left open), Closed (the other road entries), Counter (in front of a quartermaster, anywhere)", "", ParamEnumArray.FromEnum(RZ_EDressAt))]
	RZ_EDressAt m_eAt;

	[Attribute("0 0 0", desc: "Where it stands in the kit's plan: x and z metres from the camp's middle, +Z toward its front (the road in)")]
	vector m_vPlan;

	[Attribute("0", desc: "Its turn in the plan: degrees clockwise from facing the front")]
	float m_fPlanYaw;

	[Attribute("-1 0 -1", desc: "Its footprint in its own space, the low corner (x, z): its props and a person round each seat. The whole footprint must fit where it stands")]
	vector m_vFootMin;

	[Attribute("1 0 1", desc: "Its footprint's high corner (x, z)")]
	vector m_vFootMax;

	[Attribute("", desc: "Its real footprint (docs/SITES.md): each prop's box as \"x z width depth turn\" and each seat's person as \"s x z\", separated by ;, in its own space (tools/camp_kit/build.py, from footprints.json). Empty: the box from Foot min to Foot max")]
	string m_sFoot;

	[Attribute("0", UIWidgets.Slider, "How important it is (docs/SITES.md, Importance tiers): 1 security, 2 what the site is for, 3 life, 4 character, 5 clutter. The solver places tier by tier; 0 = read from its name", "0 5 1")]
	int m_iTier;

	[Attribute("0", UIWidgets.ComboBox, "The part of the site it belongs to (docs/SITES.md, Zones): Yard (inside the wire), Wire, Gate, Kill (nothing), Court, Stores, Work, Kitchen, Sleep, Sick, Dump, Dead", "", ParamEnumArray.FromEnum(RZ_ESiteZone))]
	RZ_ESiteZone m_eZone;

	[Attribute("", desc: "Its rules (docs/SITES.md, Rule types), ; between them: near, away, door, wall, window, corner, kind, over, covers, behind, faces, guarded, high, bank, soil, jitter, noline, flat (tools/camp_kit/pieces.py)")]
	string m_sRules;

	[Attribute("0", UIWidgets.ComboBox, "On a kit's Layers list: what it is for (Wire: a run of the barrier; Window, Door: boards; Lookout; Light; Stores: the stores board). Plan on the plan's own list", "", ParamEnumArray.FromEnum(RZ_ESiteLayer))]
	RZ_ESiteLayer m_eLayer;

	[Attribute("0", desc: "Valuable or delicate: at a site with a building it goes indoors, as data (docs/SITES.md, Work and life)")]
	bool m_bIndoors;

	protected ref RZ_SiteFoot m_Foot;
	protected ref RZ_SiteRules m_Rules;

	//------------------------------------------------------------------------------------------------
	//! The footprint as rectangles, read from m_sFoot once.
	RZ_SiteFoot Foot()
	{
		if (!m_Foot)
			m_Foot = RZ_SiteFoot.Parse(m_sFoot, m_vFootMin, m_vFootMax);

		return m_Foot;
	}

	//------------------------------------------------------------------------------------------------
	//! Its rules, read from m_sRules once.
	RZ_SiteRules Rules()
	{
		if (!m_Rules)
			m_Rules = RZ_SiteRules.Parse(m_sRules);

		return m_Rules;
	}
}

//------------------------------------------------------------------------------------------------
//! A faction's scenery for one kind of place (or Any): its plan.
[BaseContainerProps()]
class RZ_DressingKit
{
	[Attribute("0", UIWidgets.ComboBox, "Faction", "", ParamEnumArray.FromEnum(RZ_EFaction))]
	RZ_EFaction m_eFaction;

	[Attribute("0", UIWidgets.ComboBox, "Places it dresses: Any, or only one kind. A kind's own kit wins over the faction's Any kit", "", ParamEnumArray.FromEnum(RZ_ECampPlace))]
	RZ_ECampPlace m_ePlace;

	[Attribute(desc: "The plan, most important first: a place takes its pieces in this order until it reaches its piece count")]
	ref array<ref RZ_DressingPiece> m_aPieces;

	[Attribute("0", UIWidgets.ComboBox, "Its doctrine (docs/SITES.md, Building in layers): how much of the wire line it closes. None, Road (the road's side), Two (and the most threatened other side), All", "", ParamEnumArray.FromEnum(RZ_ESiteClose))]
	RZ_ESiteClose m_eClose;

	[Attribute("0", UIWidgets.ComboBox, "How hard its building is made: None, Doors (one entrance, the other doors blocked), Threat (and the windows on the threatened sides boarded), All (every ground-floor window)", "", ParamEnumArray.FromEnum(RZ_ESiteHarden))]
	RZ_ESiteHarden m_eHarden;

	[Attribute("8", desc: "Metres from the building (or the plan on open ground) to the wire line on each side")]
	float m_fRing;

	[Attribute("14", desc: "Metres from the building to the wire on the yard's side, at least (more where the plan's yard needs it)")]
	float m_fYard;

	[Attribute(desc: "The doctrine's own pieces: the barrier's runs (Wire), window and door boards, lookouts, lights and the stores board")]
	ref array<ref RZ_DressingPiece> m_aLayers;
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Territory > Camp life.
[BaseContainerProps()]
class RZ_CampLifeSettings
{
	[Attribute("1", desc: "Dress holdings, Haven safehouses, field stations and road posts with their faction's scenery (Kits) while their people are there. Nothing is saved")]
	bool m_bDressing;

	[Attribute("1", desc: "Give their people jobs from their faction's routine (sentry, patrol, sit, work, wash, chop, cook, gather, sleep) at the Live places nearest players")]
	bool m_bRoutines;

	[Attribute("1", desc: "Holdings, safehouses and field stations move into the building in their town that suits their faction (by kind and size) and lay their pieces out by it and its road entries. Off, or no building fits: rings round the centre")]
	bool m_bBuildings;

	[Attribute("20", UIWidgets.Slider, "Pieces at a stronghold", "0 30 1")]
	int m_iPiecesStronghold;

	[Attribute("13", UIWidgets.Slider, "Pieces at an outpost", "0 20 1")]
	int m_iPiecesOutpost;

	[Attribute("8", UIWidgets.Slider, "Pieces at a Haven safehouse", "0 15 1")]
	int m_iPiecesSafehouse;

	[Attribute("9", UIWidgets.Slider, "Pieces at a Halcyon field station (its outposts)", "0 15 1")]
	int m_iPiecesFieldStation;

	[Attribute("4", UIWidgets.Slider, "Pieces at a road post (beside the barrier)", "0 8 1")]
	int m_iPiecesRoadPost;

	[Attribute("200", desc: "Pieces standing (or waiting to spawn) on the whole server at most, the layers' with them. They spawn a few per tick")]
	int m_iPiecesMax;

	[Attribute("3", UIWidgets.Slider, "Places nearest players whose people do their routine (elsewhere they stand)", "0 8 1")]
	int m_iLivePlaces;

	[Attribute("3", desc: "Real minutes between shuffles: one person moves to another job, and anyone idle goes back to work")]
	float m_fShuffleMinutes;

	[Attribute("1", UIWidgets.Slider, "Extra people (the place's guard prefab: off-shift workers, the courier) at each live place, on top of the built counts", "0 3 1")]
	int m_iExtrasPerPlace;

	[Attribute("6", desc: "Extra people on the whole server at most")]
	int m_iExtrasMax;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Patrol waypoint (vanilla AIWaypoint_Patrol.et). Empty = Zombies > Move waypoint", "et")]
	ResourceName m_sPatrolWaypoint;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Cycle waypoint (vanilla AIWaypoint_Cycle.et): the patrol loops. Empty = it walks the loop once per shuffle", "et")]
	ResourceName m_sCycleWaypoint;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Loiter waypoint (vanilla AIWaypoint_Loiter_CO.et), used where a piece has no free smart action. Empty = Zombies > Move waypoint", "et")]
	ResourceName m_sLoiterWaypoint;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Observation post waypoint (vanilla AIWaypoint_ObservationPost.et) for a sentry without a nest: used only where an observation-post smart action stands within 6 m of the sentry's spot. Otherwise, or empty, the loiter waypoint", "et")]
	ResourceName m_sObservationWaypoint;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Smart-action waypoint (Prefabs/ReforgerZ/AI/RZ_WP_SmartAction.et, an SCR_SmartActionWaypoint): people use a piece's sit, lean and watch points. Empty = loiter at the piece", "et")]
	ResourceName m_sSmartActionWaypoint;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Empty AI group (vanilla Group_Base.et): a lone character (an occupant, a single guard) is put in one so it can take jobs. Empty = lone characters stand", "et")]
	ResourceName m_sGroupPrefab;

	[Attribute(desc: "Kits: each faction's scenery, per kind of place or for Any. A place with no kit stays bare")]
	ref array<ref RZ_DressingKit> m_aKits;

	[Attribute("1", desc: "Read each site's ground on a 0.5 m grid before it's laid out (docs/SITES.md): every part of a piece's real footprint is tested against water, slopes, buildings, walls, trees, roads and the other pieces, and its surroundings and threat are read. Off: the older point checks")]
	bool m_bSiteGrid;

	[Attribute("2", UIWidgets.Slider, "Milliseconds of server time a second for reading a site's grid (one site at a time; its pieces spawn once it's read and laid out)", "1 20 1")]
	int m_iSiteReadMs;

	[Attribute("6", UIWidgets.Slider, "Milliseconds of server time each tenth of a second for putting a place's pieces out near players (nearest place first, what its people use first, scenery and the layers last; docs/SITES.md, How fast a place fills). 0: the old way, 4 pieces a main tick", "0 20 1")]
	int m_iSpawnMs;

	[Attribute("8", UIWidgets.Slider, "Milliseconds of server time each tenth of a second for reading and laying out a site a player has come near before its pre-read was done (the background pre-read keeps Site read ms a second)", "1 30 1")]
	int m_iSiteReadNearMs;

	[Attribute("70", UIWidgets.Slider, "The audit's pass mark (docs/SITES.md, The audit): a site scoring under it is listed in the log's summary, and the runtime test's site audit fails if a stronghold is under it", "0 100 1")]
	int m_iAuditThreshold;

	[Attribute("1", desc: "Lay sites out by their pieces' rules (docs/SITES.md, The solver): zones, near and away, the ground, tier by tier. Off: the plans laid down whole as before. Needs Site grid")]
	bool m_bSolver;

	[Attribute("24", UIWidgets.Slider, "Spots scored for a piece before it's dropped", "4 64 1")]
	int m_iCandidates;

	[Attribute("1", desc: "Build in layers by each faction's doctrine (docs/SITES.md): close it off (the wire and its gates), harden the building, watch, then work and life inside. Off: no wire, boards, lookouts or lights")]
	bool m_bLayers;

	[Attribute("1", desc: "Harden: board ground-floor windows with the kit's window board (the doctrine says which). Off: only side doors are blocked")]
	bool m_bBoardWindows;

	[Attribute("1", desc: "Read and lay out every site in the background from the start (strongholds and the lab first, a little each second within Site read ms), so a camp is laid out before anyone arrives; only its pieces and people wait for a player near. Off: a site is read the first time a player comes near")]
	bool m_bPreRead;

	[Attribute("60", UIWidgets.Slider, "The layers' pieces at a site at most (the wire's runs, boards, lookouts, lights), on top of its piece count", "0 100 1")]
	int m_iLayerPieces;

	[Attribute("1", desc: "Story (docs/SITES.md, The story of each site): each site shows what happened to it through Traces' scene props (the kit's pyre, graves, boards, wrecks...), aged as scenes are, and its people say a line in its state when you come in. Off: no story elements or lines")]
	bool m_bStory;

	[Attribute("1", desc: "Alert states (docs/SITES.md, The people): a site's posts and movement change with a threat (wary, alert, recently hit). Off: always calm")]
	bool m_bAlertStates;

	[Attribute("1", desc: "Road post run: Bastion and the Red Hand walk to their nearest road post (within 1.5 km) and back once a day, one at a time on the server, within the extras' cap")]
	bool m_bRoadPostRun;

	[Attribute("{237F141902888302}Prefabs/ReforgerZ/World/HandBuilt/RZ_HandSpot.et", UIWidgets.ResourcePickerThumbnail, "Hand-built sites: a role spot in play (docs/SITES.md, Role spots): an invisible point with one smart action for each of camp life's tags (sit, lean, work, watch, gather, cook, wash, chop). Empty: hand-placed spots are read as positions only (people stand there, nobody sits)", "et")]
	ResourceName m_sHandSpot;

	[Attribute("{DA6496B7DC1880E6}Prefabs/ReforgerZ/World/HandBuilt/RZ_HandSpot_Meal.et", UIWidgets.ResourcePickerThumbnail, "Hand-built sites: the canteen table's spot in play: eight seats round a point (put a table there). Empty: the Hand spot, one seat", "et")]
	ResourceName m_sHandSpotMeal;

	[Attribute("{D08B33729A0C2743}PrefabsEditable/Auto/Props/Infrastructure/ConeTraffic/E_ConeTraffic_01_red.et", UIWidgets.ResourcePickerThumbnail, "Hand-built sites: what marks a role spot while it's built (the site builder): something small Game Master can move. Empty: the spots show as coloured shapes only", "et")]
	ResourceName m_sSpotMarker;
}

//------------------------------------------------------------------------------------------------
//! One standing piece of scenery.
class RZ_CampPiece
{
	IEntity m_Entity;
	ResourceName m_sPending;	// not spawned yet (a few spawn per tick); cleared once it's tried
	float m_fYaw;
	vector m_vPos;
	RZ_EDressRole m_eRole;
	RZ_EDressUse m_eUse;
	string m_sTag;
	int m_iSeats;
	int m_iUsed;
	float m_fSlope;				// a barrier run on a slope: the ground's rise a metre along its x
	bool m_bStory;				// a story element (RZ_SiteStory): it goes when the story is worked out again
	string m_sName;				// its kit name ("WaterBarrel", "Latrine"): the water and the latrine walks
	bool m_bHandObj;			// a hand-built site's object (World/HandBuilt): spawned as saved, tilted and scaled
	bool m_bHandSpot;			// a hand-built site's role spot (RZ_HandSpots): where the author said the work is
	vector m_vAngles;			// its yaw, pitch and roll
	float m_fScale = 1;
}

//------------------------------------------------------------------------------------------------
//! One body at a place and its job.
class RZ_CampWorker
{
	IEntity m_Unit;			// the body as it was spawned (a character or an AI group)
	IEntity m_Group;		// what takes waypoints: the unit if it's a group, else its wrapper
	bool m_bWrapped;		// m_Group was made here (WrapInGroup)
	bool m_bTriedGroup;
	bool m_bOwned;			// spawned here (an extra, or someone woken up): deleted here
	int m_iRole;
	int m_iJob;
	int m_iAfterErrand;		// the job to go back to after the courier's walk
	int m_iPiece = -1;
	int m_iSeat = -1;			// which of the piece's sentinels with its tag (0 = the first)
	int m_iHow;					// how the job is done (RZ_CampLife.HOW_*): for the jobs log
	int m_iArg = -1;			// the job's post (a watch: RZ_SitePost index, ARG_LINE + n the threat line) or route (RZ_SiteRoute kind, ARG_*)
	int m_iAfterArg = -1;
	bool m_bWasOnWatch;			// at the handover: off the post to the fire, not straight back onto one
	float m_fHoldUntil;
	ref array<IEntity> m_aWaypoints = {};
}

//------------------------------------------------------------------------------------------------
//! A place with bodies: data plus its pieces and workers.
class RZ_CampPlace
{
	string m_sKey;
	string m_sTitle;
	RZ_EFaction m_eFaction;
	RZ_ECampPlace m_eKind;
	vector m_vPos;
	float m_fYaw;				// road posts: the post's yaw
	float m_fApproach;			// bearing to the nearest road (or town)
	vector m_vRoad;				// the nearest road point (Red Hand strolls), if m_bHasRoad
	bool m_bHasRoad;
	int m_iSeed;
	ResourceName m_sExtraPrefab;
	ref array<ref RZ_CampPiece> m_aPieces = {};
	ref array<ref RZ_CampWorker> m_aWorkers = {};
	ref array<vector> m_aKeepClear = {};		// quartermasters, the medic, the barrier
	ref array<vector> m_aLeaderSeats = {};
	bool m_bLive;
	int m_iPhase = -2;
	int m_iShift;				// +1 at each shift change: someone else takes the watch
	float m_fGatherUntil;		// the roll
	float m_fCourierTimer;
	int m_iAsleepGuards;		// slept as data: they come back in the morning
	int m_iAsleepExtras;
	ref RZ_CampSite m_Site;		// the building it has moved into; null = rings round m_vPos
	vector m_vHome;				// where people gather and stay near: the site's yard, else m_vPos
	vector m_vCentre;			// what the patrol and the courier walk round: the building, else m_vPos
	float m_fPersonRadius = 20;	// named people stay this close to m_vHome
	bool m_bHasTown;
	float m_fTownBearing;		// toward the nearest other town (the main road's way in)
	int m_iClosedNext;			// the next closed entry a barricade goes to
	int m_iGateNext;			// pieces at the gate so far (they take turns at each side of the road)
	int m_iDoorNext;			// pieces by the front door so far
	int m_iFrontNext;			// pieces along the front wall so far
	ref RZ_SitePeople m_People;	// its posts and routes (docs/SITES.md, "The people"), from the kept layout
	int m_iAlert;				// RZ_SitePeople.ALERT_*
	float m_fLossAt = -1000;	// when one of its people last died (contact)
	ref RZ_SiteFacts m_Facts;	// its story's state (docs/SITES.md, "The story of each site")
	int m_iStory = -1;
	int m_iStoryDay = -1;		// the season day the story was worked out
	int m_iRunDay = -1;			// the season day of its last road post run
	ref set<string> m_aGreeted = new set<string>();	// "player|day": the line when you come in
	float m_fEnterMs;			// when it came (System.GetTickCount), for the "Spawn timing" line
	float m_fEnterDist;			// how far the nearest player was then
	int m_iSpawnedCount;		// pieces put out since it came
	float m_fSpawnCost;			// server ms spent putting them out
	float m_fSpawnLongest;		// the longest tenth of a second spent on it (ms)
	bool m_bTimed;				// its "Spawn timing" line is written
	bool m_bHand;				// laid out from a hand-built save, or being built by hand: no automated pieces (World/HandBuilt)
}

//------------------------------------------------------------------------------------------------
//! What a place's pieces are fitted against, read once as it's dressed: the buildings' boxes, the
//! roads, the spots kept clear, other scenery and the pieces laid out so far.
class RZ_CampFit
{
	ref array<vector> m_aBoxes = {};	// every building's box grown by the margin: four corners each
	ref array<vector> m_aInside = {};	// the same boxes as they are (is a quartermaster indoors)
	ref array<vector> m_aRoad = {};		// road segments near the place: two points each
	ref array<vector> m_aKeep = {};		// spots kept clear: quartermasters, the medic, the barrier, seats, the door, customers
	ref array<float> m_aKeepR = {};
	ref array<vector> m_aOthers = {};	// other systems' scenery standing near
	ref array<vector> m_aTaken = {};	// pieces laid out (grown by half the gap): four corners each
	vector m_vSkip;					// a quartermaster's own spot: his counter may stand by it
	bool m_bSkip;
	ref RZ_SiteGrid m_Grid;			// the site's ground read on a grid (null: the point checks)
	string m_sWhy;					// why the last footprint didn't fit (the log, the audit)
}

//------------------------------------------------------------------------------------------------
//! Where a plan is laid down: the frame the camp (by a building, its yard) is drawn in.
class RZ_CampLayout
{
	vector m_vOrigin;			// the plan's middle, on the ground
	float m_fBearing;			// the way the plan's front (+Z) faces
	bool m_bMirror;				// drawn left for right (by the seed)
	string m_sWhere;			// for the log
	int m_iFace = -1;			// by a building: the side the yard is on (FACE_*)
}

//------------------------------------------------------------------------------------------------
//! Ground near a place, read once while it's dressed.
class RZ_CampGround : RZ_DressGround
{
}

//------------------------------------------------------------------------------------------------
//! The building a place has moved into, read once per run and kept (buildings don't move).
//! Its box and transform are copied, so nothing here holds the building itself.
class RZ_CampSite
{
	string m_sKey;
	RZ_EFaction m_eFaction;
	RZ_ECampPlace m_eKind;
	vector m_vPlace;			// the place it was read for
	bool m_bNone;				// no building fits: rings
	string m_sWhat;				// "a school", "a house": for the log
	vector m_vMins;				// the building's local box
	vector m_vMaxs;
	vector m_vRight;			// its world transform
	vector m_vUp;
	vector m_vFwd;
	vector m_vOrigin;
	vector m_vCentre;			// the box's middle, on the ground
	int m_iAxis;				// the local axis (0 = x, 2 = z) its front faces along
	float m_fSign = 1;			// which end of that axis is the front
	float m_fDoor;				// the front door along the front wall, -1 to 1 (0 = the middle)
	bool m_bDoor;				// a real outside door was found there
	int m_iYardFace;			// RZ_CampLife.FACE_BACK, or FACE_FRONT for the Red Hand's forecourt
	vector m_vHome;				// the yard's middle
	ref array<vector> m_aEntries = {};	// where roads cross the ring round the building
	ref array<float> m_aEntryRoad = {};	// the road's bearing there
	ref array<int> m_aEntryRoadId = {};	// which road crosses there (two entries on one road: a through-road)
	int m_iMain = -1;			// the entry left open (the gate); the others are closed
	string m_sPrefab;			// the building's prefab (the log)
	ref array<vector> m_aLookouts = {};	// the building's lookouts the solver chose: the corners that see the wire's gate
	int m_iTier;				// how well the building suits the faction and the kind (SiteTier)
}

//------------------------------------------------------------------------------------------------
//! A piece laid out at a site, or one left out, as data (the layout cache and the audit).
class RZ_SitePlaced
{
	RZ_DressingPiece m_Entry;
	vector m_vPos;
	float m_fYaw;
	string m_sWhy;				// left out: why
	int m_iLayer;				// RZ_ESiteLayer: the plan, or a layer's piece
	int m_iStory = -1;			// a story element's spot (RZ_SiteStory.E_*)
	int m_iStoryRows = 1;		// the graves' row: how many it has room for
	bool m_bIndoors;			// valuables indoors: data only, nothing spawns
	bool m_bWall;				// stands on the building's wall (a board)
	float m_fSlope;				// a barrier run tilted to follow the ground: its rise a metre along its x
}

//------------------------------------------------------------------------------------------------
//! What laying a site out gave: the pieces placed and those dropped, and why (docs/SITES.md,
//! "Drop, don't squeeze").
class RZ_SiteResult
{
	ref array<ref RZ_SitePlaced> m_aPlaced = {};
	ref array<ref RZ_SitePlaced> m_aDropped = {};
	int m_iTried;
	int m_iCounters;
	string m_sWhere;
	int m_iSolveMs;
	int m_iSolveSteps;			// the seconds it was solved in
	int m_iSolveMaxMs;			// the longest of them
	int m_iSolveMaxUnitMs;		// the longest single part (the start: the counters, the frame, the wire's trace; a piece; a side of the wire)
	ref RZ_SiteWire m_Wire;		// the solver's wire line (null: none)
	vector m_vGate;				// the gate in it
	bool m_bGate;
	int m_iIndoors;				// pieces gone indoors, as data
	int m_iDoorsBlocked;
	int m_iWindowsBoarded;
	int m_iBlockedPaths;		// no way from the door to the gate when it was kept clear
	vector m_vDoor;				// the entrance's step the solver used (a body can stand there)
	bool m_bDoor;
	vector m_vGatePost;			// the gate's post: just inside the gate, seeing it
	bool m_bGatePost;
	ref array<vector> m_aSidePosts = {};	// a lookout's post toward each threatened side nothing faced
	ref array<float> m_aSideFacing = {};
	ref RZ_SitePeople m_People;	// its posts and routes
	ref array<ref RZ_DressingPiece> m_aKeepAlive = {};	// pieces made in code (the Traces board by the door, the story's spots)
}

//------------------------------------------------------------------------------------------------
//! A site's layout, worked out once a run and kept: the next visit lays it again with no reading.
class RZ_SiteLayout
{
	RZ_EFaction m_eFaction;
	RZ_ECampPlace m_eKind;
	vector m_vPlace;
	string m_sWhere;
	ref array<ref RZ_SitePlaced> m_aPieces = {};	// counters aren't kept: they stand by the quartermaster as he is
	ref array<ref RZ_DressingPiece> m_aKeepAlive = {};
	ref RZ_SitePeople m_People;		// its posts and routes (null: Solver off)
}

//------------------------------------------------------------------------------------------------
//! A site whose grid is being read, a little each second; then it's laid out and audited.
class RZ_SiteJob
{
	string m_sKey;
	ref RZ_CampPlace m_Place;
	ref RZ_CampGround m_Ground;
	ref RZ_SiteGrid m_Grid;
	bool m_bAudit;				// as data only (the audit or the pre-read): nothing spawns
	bool m_bDraw;				// the runtime test's audit: its drawing lines too
	int m_iQueuedAt;
	ref RZ_SiteSurroundings m_Around;
	ref RZ_SiteSolver m_Solver;	// laying it out once the grid is read
	ref RZ_SiteResult m_Result;	// the older plan's layout (Solver off)
}

//------------------------------------------------------------------------------------------------
//! The hard rule: nothing at a camp can be used for gain. Runs on every piece as it spawns.
class RZ_DressingGuard
{
	//------------------------------------------------------------------------------------------------
	//! Deletes every part of the piece that a player could use. False if the piece itself was
	//! one (then the whole piece is gone).
	static bool Strip(IEntity piece, string prefab)
	{
		if (!piece)
			return false;

		string why = Abusable(piece);
		if (!why.IsEmpty())
		{
			RZ_Log.Warn(string.Format("Dressing: removed %1 from %2 (%3)", RZ_Util.PrettyPrefabName(RZ_Util.GetPrefabName(piece)), prefab, why));
			SCR_EntityHelper.DeleteEntityAndChildren(piece);
			return false;
		}

		StripChildren(piece, prefab, 0);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void StripChildren(IEntity e, string prefab, int depth)
	{
		if (depth > 6)
			return;

		array<IEntity> doomed = {};
		IEntity child = e.GetChildren();
		while (child)
		{
			string why = Abusable(child);
			if (!why.IsEmpty())
			{
				RZ_Log.Warn(string.Format("Dressing: removed %1 from %2 (%3)", RZ_Util.PrettyPrefabName(RZ_Util.GetPrefabName(child)), prefab, why));
				doomed.Insert(child);
			}
			else
			{
				StripChildren(child, prefab, depth + 1);
			}

			child = child.GetSibling();
		}

		foreach (IEntity d : doomed)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(d);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Why a part is not allowed in scenery, "" if it's fine.
	static string Abusable(IEntity e)
	{
		if (!e)
			return "";

		if (Vehicle.Cast(e) || e.FindComponent(SCR_CatalogEntitySpawnerComponent))
			return "vehicle";
		if (ChimeraCharacter.Cast(e) || AIGroup.Cast(e) || e.FindComponent(AIControlComponent))
			return "body";
		if (SCR_SpawnPoint.Cast(e))
			return "spawn point";
		if (e.FindComponent(SCR_ArsenalComponent) || e.FindComponent(SCR_ArsenalInventoryStorageManagerComponent))
			return "arsenal";
		if (e.FindComponent(BaseInventoryStorageComponent))
			return "storage";
		if (e.FindComponent(InventoryItemComponent))
			return "item";
		if (e.FindComponent(SCR_BaseSupportStationComponent))
			return "support station";
		if (e.FindComponent(SCR_ResourceComponent))
			return "supplies";
		if (e.FindComponent(SCR_FuelManagerComponent))
			return "fuel";
		if (HasReforgerZPart(e))
			return "ReforgerZ part";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected static bool HasReforgerZPart(IEntity e)
	{
		array<Managed> comps = {};
		e.FindComponents(ScriptComponent, comps);
		foreach (Managed c : comps)
		{
			// Look-only sounds (Traces' loops and one-shots) are allowed: they only play
			if (RZ_AmbientLoopComponent.Cast(c) || RZ_CallSoundComponent.Cast(c))
				continue;

			if (c && c.ClassName().IndexOf("RZ_") == 0)
				return true;
		}

		ActionsManagerComponent am = ActionsManagerComponent.Cast(e.FindComponent(ActionsManagerComponent));
		if (!am)
			return false;

		array<BaseUserAction> actions = {};
		am.GetActionsList(actions);
		foreach (BaseUserAction a : actions)
		{
			if (a && a.ClassName().IndexOf("RZ_") == 0)
				return true;
		}

		return false;
	}
}

//------------------------------------------------------------------------------------------------
//! A site camp life lays out, as the site builder lists it (World/HandBuilt/RZ_SiteBuilder.c).
class RZ_SiteEntry
{
	string m_sId;
	string m_sTitle;
	RZ_EFaction m_eFaction;
	RZ_ECampPlace m_eKind;
	vector m_vPos;
	float m_fYaw;
	float m_fRadius;

	//------------------------------------------------------------------------------------------------
	static RZ_SiteEntry Make(string title, RZ_EFaction faction, RZ_ECampPlace kind, vector pos, float yaw)
	{
		RZ_SiteEntry e = new RZ_SiteEntry();
		e.m_sId = RZ_HandBuilt.SiteId(kind, pos);
		e.m_sTitle = title;
		e.m_eFaction = faction;
		e.m_eKind = kind;
		e.m_vPos = pos;
		e.m_fYaw = yaw;
		e.m_fRadius = RZ_HandBuilt.RadiusOf(kind);
		return e;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_CampLife
{
	static const int ROLE_GUARD = 0;
	static const int ROLE_PERSON = 1;		// named people and occupants: they stay near the centre
	static const int ROLE_KEEPER = 2;		// quartermasters, the medic: never given a job
	static const int ROLE_EXTRA = 3;

	static const int JOB_NONE = 0;
	static const int JOB_WATCH = 1;
	static const int JOB_PATROL = 2;
	static const int JOB_SIT = 3;
	static const int JOB_WORK = 4;
	static const int JOB_GATHER = 5;
	static const int JOB_LEAN = 6;
	static const int JOB_STROLL = 7;
	static const int JOB_COURIER = 8;
	static const int JOB_OFF = 9;
	static const int JOB_COOK = 10;		// the cook pot, before the meal
	static const int JOB_WASH = 11;		// the wash stand (the decon shower), in the morning
	static const int JOB_CHOP = 12;		// the chopping block, by day

	// How a job is done (the jobs log)
	static const int HOW_NONE = 0;		// no group to take waypoints: they stand where they are
	static const int HOW_SEAT = 1;		// a smart action: sitting, leaning, working, watching at a seat
	static const int HOW_SPOT = 2;		// standing about at one spot (a loiter or observation waypoint)
	static const int HOW_WALK = 3;		// walking round the camp, piece to piece
	static const int HOW_MOVE = 4;		// on the move: the patrol, an errand, off to bed

	// A job's argument (RZ_CampWorker.m_iArg): a watch's post is its index (0 up), a route its
	// RZ_SiteRoute kind; these are the rest
	static const int ARG_LINE = 100;		// a watch on the wire facing the threat (the alert): ARG_LINE + n
	static const int ARG_LATRINE = 50;		// a short walk to the latrine and back
	static const int ARG_STORES = 51;		// the quartermaster's runner: to the stores door and back
	static const int ARG_WATER = 52;		// work at the water (the barrel)
	protected static const int POTTER_STOPS = 3;		// pieces someone with nothing free walks between

	protected static const int SPAWN_PER_TICK = 4;			// with Spawn ms 0: pieces spawned per main tick, all places together
	protected static const float CHECK_INTERVAL = 10;
	protected static const float CONTACT_SECONDS = 180;		// one of its people died this recently: alert
	protected static const float HEAT_WARY = 40;			// heat in its square this high: wary (half a horde's)
	protected static const float HEAT_ALERT = 80;			// and this high: alert (a horde can come)
	protected static const float HOSTILE_WARY = 300;		// a hostile base this near: wary
	protected static const float GREET_REACH = 40;			// the line when you come in, within this of the yard or the gate
	protected static const int SMALL_WALK_EVERY = 3;		// a latrine or stores walk at a live place one shuffle in this many
	protected static const float LIVE_INTERVAL = 30;
	protected static const float ROLL_SECONDS = 180;		// the roll at 20:30, before the night watch
	protected static const float COURIER_SECONDS = 1200;	// Halcyon's courier walks out and back
	protected static const float SLEEP_CLEAR = 150;			// nobody this close (or in sight) sees anyone vanish or appear
	protected static const float EXTRA_CLEAR = 60;
	protected static const float HOLD_SECONDS = 45;			// someone talked to stands still
	protected static const float PERSON_RADIUS = 20;		// named people stay this close to the centre
	protected static const float SHARE_RADIUS = 60;			// a place this close to a dressed one isn't dressed again

	// Fitting a piece: its whole footprint (docs/DESIGN.md, "Camp life", "Every piece fits")
	protected static const float FIT_REACH = 110;			// buildings, roads and scenery this far from the place are fitted against
	protected static const float BUILDING_EDGE = 0.8;		// a footprint keeps this far out of every building's box
	protected static const float NO_BOX = 3;				// a building too small to tell: this far round its origin
	protected static const float ROAD_EDGE = 4;				// every point of a footprint this far from a road's centre line
	protected static const float PIECE_GAP = 0.8;			// between two pieces' footprints
	protected static const float KEEP_EDGE = 2;				// a footprint from a quartermaster, the medic, the barrier
	protected static const float LEADER_EDGE = 3.5;			// from a leader's seat (the body stands 2-5 m from it)
	protected static const float DOOR_EDGE = 1.5;			// from the front door's step
	protected static const float OTHER_EDGE = 1;			// from another system's scenery
	protected static const float FLAT_SLOPE = 0.21;			// the ground across a footprint rises at most this per metre of its diagonal...
	protected static const float FLAT_SLACK = 0.2;			// ...plus this (about 12 degrees)
	protected static const float YARD_GAP = 2.5;			// the yard's nearest piece from the building's wall
	protected static const float ENTRY_SIDE = 4.5;			// a gate or closed-entry piece's near edge from the road's centre line
	protected static const float COUNTER_AHEAD = 0.95;		// the counter's middle in front of the quartermaster
	protected static const float CUSTOMER_AHEAD = 2.4;		// where his customers stand, kept clear
	protected static const float WALL_NEAR = 8;				// a quartermaster this close to the building serves with his back to it
	protected static const float OUTLOOK_REACH = 80;		// the camp's front never faces water or a drop this near
	protected static const float BARRIER_OUTLOOK = 50;		// a nest, barrier or sign never faces water or a drop this near

	static const int FACE_FRONT = 0;
	static const int FACE_BACK = 1;
	static const int FACE_LEFT = 2;
	static const int FACE_RIGHT = 3;

	protected static const float SITE_MAX_SIDE = 70;		// a building longer than this isn't a camp (a hangar, a factory)
	protected static const float SITE_MIN_SIDE = 5;		// narrower than this isn't a building people live in
	protected static const float SITE_MAX_RATIO = 4;		// longer than this many times its width: a pier, a wall, a shed row
	protected static const int SITE_SHORTLIST = 6;			// the best few by kind, size and distance; then the roads decide
	protected static const float ENTRY_MERGE = 14;			// road crossings this close are one entry
	protected static const int MAX_ENTRIES = 5;
	protected static const float LOOKOUT_OUT = 2;			// the lookout stands this far out from the front corner
	protected static const int MAX_SITES = 64;			// sites kept per run
	protected static const float RINGS_REACH = 62;		// a place on rings: its edge pieces stand up to this far out
	protected static const float WALL_CLEAR = 0.4;			// a spot people stand or lean at keeps this far off a wall (else a lean's head is in it)
	protected static const float GRID_POST = 16;		// a road post's grid reaches this far each way
	protected static const float GRID_OPEN = 56;		// a camp on open ground's: its plan, the shifts tried and the wire round it
	protected static const float GRID_MAX = 72;			// the most any site's grid reaches
	static const float ROLL_SIGN = -1;					// the roll's sign that raises a piece's +x (RollOf: the site audit measured AnglesToMatrix)

	protected static ref array<IEntity> s_aPieces;			// every standing piece (IsDressing)
	protected static bool s_bProbeOld;					// the pier probe: pick as before the waterside was ruled out

	protected RZ_GameModeComponent m_GM;
	protected RZ_CampLifeSettings m_Settings;
	protected ref array<ref RZ_CampPlace> m_aPlaces = {};
	protected ref map<string, bool> m_mToldNoKit = new map<string, bool>();
	protected ref map<string, ref RZ_CampSite> m_mSites = new map<string, ref RZ_CampSite>();
	protected ref map<string, bool> m_mToldSite = new map<string, bool>();
	protected ref array<ref RZ_SiteJob> m_aJobs = {};
	protected ref map<string, ref RZ_SiteLayout> m_mLayouts = new map<string, ref RZ_SiteLayout>();
	protected ref RZ_SiteAudit m_Audit = new RZ_SiteAudit();
	protected ref array<vector> m_aAudited = {};

	//! The headless audit (the runtime test): a bigger budget a second, and the drawing lines
	static int s_iAuditReadMs = 0;
	//! The runtime test's site audit queues every site itself: no pre-reading
	static bool s_bNoPreRead;
	protected bool m_bPreReadQueued;
	protected bool m_bDrawNext;			// AuditAll's sites are drawn
	protected float m_fPreReadWait;		// seconds waited for the land grid before pre-reading
	protected float m_fClock;
	protected float m_fCheckTimer;
	protected float m_fLiveTimer;
	protected float m_fShuffleTimer;
	protected float m_fStoryTimer;
	protected int m_iStoryDayAll = -1;
	protected ref set<string> m_aMentioned = new set<string>();	// "key|day": a site's quiet headline
	protected bool m_bLiveDirty;

	//------------------------------------------------------------------------------------------------
	void RZ_CampLife(RZ_GameModeComponent gm, RZ_CampLifeSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		s_aPieces = {};

		int kits = 0;
		if (settings.m_aKits)
			kits = settings.m_aKits.Count();

		RZ_Log.Info(string.Format("Camp life: dressing %1, routines %2, buildings %3, %4 kits, smart actions %5, lone characters %6.",
			OnOff(settings.m_bDressing), OnOff(settings.m_bRoutines), OnOff(settings.m_bBuildings), kits,
			OnOff(!settings.m_sSmartActionWaypoint.IsEmpty()), WrapText(!settings.m_sGroupPrefab.IsEmpty())));
		RZ_HandBuilt.Get();
	}

	//------------------------------------------------------------------------------------------------
	protected static string OnOff(bool on)
	{
		if (on)
			return "on";

		return "off";
	}

	//------------------------------------------------------------------------------------------------
	protected static string WrapText(bool on)
	{
		if (on)
			return "take jobs";

		return "stand";
	}

	//------------------------------------------------------------------------------------------------
	//! Another system's look-only prop (Traces' boards and scenes) counts as scenery too.
	static void AddDressing(IEntity e)
	{
		if (s_aPieces && e && !s_aPieces.Contains(e))
			s_aPieces.Insert(e);
	}

	//------------------------------------------------------------------------------------------------
	static void RemoveDressing(IEntity e)
	{
		if (s_aPieces && e)
			s_aPieces.RemoveItem(e);
	}

	//------------------------------------------------------------------------------------------------
	//! Camp life's own pieces standing within radius of pos (Traces finds the table lamp, the Halcyon
	//! generator and Bastion's loudspeaker pole among them: World/Traces/RZ_TraceSounds.c).
	static void PiecesNear(vector pos, float radius, notnull array<IEntity> result)
	{
		if (!s_aPieces)
			return;

		foreach (IEntity e : s_aPieces)
		{
			if (e && RZ_Util.DistXZ(e.GetOrigin(), pos) <= radius && RZ_Util.GetPrefabName(e).Contains("RZ_Dress_"))
				result.Insert(e);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A piece of scenery (or a part of one): the interaction scan leaves it alone.
	static bool IsDressing(IEntity e)
	{
		if (!e || !s_aPieces || s_aPieces.IsEmpty())
			return false;

		IEntity root = e;
		int depth = 0;
		while (root.GetParent() && depth < 8)
		{
			root = root.GetParent();
			depth++;
		}

		return s_aPieces.Contains(root);
	}

	//------------------------------------------------------------------------------------------------
	//! Camp life's own pieces standing and waiting to spawn, on the whole server (the cap). Other
	//! systems' props in s_aPieces (Traces' boards, scenes and sound looks) don't count.
	int CountPieces()
	{
		for (int i = s_aPieces.Count() - 1; i >= 0; i--)
		{
			if (!s_aPieces[i])
				s_aPieces.Remove(i);
		}

		int n = 0;
		foreach (RZ_CampPlace p : m_aPlaces)
		{
			foreach (RZ_CampPiece piece : p.m_aPieces)
			{
				if (piece.m_Entity || !piece.m_sPending.IsEmpty())
					n++;
			}
		}

		return n;
	}

	// ================================================================ Coming round (Backgrounds/RZ_Character.c)
	//------------------------------------------------------------------------------------------------
	//! A live place within `radius` of pos is under attack (its alert, docs/SITES.md "When the site
	//! is threatened"): nobody is carried in there.
	bool UnderAttackNear(vector pos, float radius)
	{
		foreach (RZ_CampPlace p : m_aPlaces)
		{
			if (p && p.m_iAlert == RZ_SitePeople.ALERT_ALERT && RZ_Util.DistXZ(p.m_vPos, pos) <= radius)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Where a place within `radius` of pos keeps its people (its yard: m_vHome). False with none.
	bool HomeNear(vector pos, float radius, inout vector home)
	{
		foreach (RZ_CampPlace p : m_aPlaces)
		{
			if (p && RZ_Util.DistXZ(p.m_vPos, pos) <= radius && p.m_vHome != vector.Zero)
			{
				home = p.m_vHome;
				return true;
			}
		}

		return false;
	}

	// ================================================================ Places come and go with their bodies
	//------------------------------------------------------------------------------------------------
	//! From the owner's SpawnBodies, after the bodies are there. guards and persons may get jobs;
	//! keepers (quartermasters, the medic) only keep the scenery clear. extraPrefab is the place's
	//! guard prefab (extras and people woken up in the morning).
	void Enter(string key, string title, RZ_EFaction faction, RZ_ECampPlace kind, vector pos, float yaw, ResourceName extraPrefab, array<IEntity> guards, array<IEntity> persons, array<IEntity> keepers)
	{
		if (!m_Settings.m_bDressing && !m_Settings.m_bRoutines)
			return;

		Leave(key);

		RZ_CampPlace p = new RZ_CampPlace();
		p.m_sKey = key;
		p.m_sTitle = title;
		p.m_eFaction = faction;
		p.m_eKind = kind;
		p.m_vPos = pos;
		p.m_fYaw = yaw;
		p.m_sExtraPrefab = extraPrefab;
		InitPlace(p, faction, kind, pos);

		AddWorkers(p, guards, ROLE_GUARD);
		AddWorkers(p, persons, ROLE_PERSON);
		AddWorkers(p, keepers, ROLE_KEEPER);
		if (keepers)
		{
			foreach (IEntity k : keepers)
			{
				if (k)
					p.m_aKeepClear.Insert(RZ_Util.GetUnitPosition(k));
			}
		}

		if (kind == RZ_ECampPlace.ROAD_POST)
			p.m_aKeepClear.Insert(pos);

		m_aPlaces.Insert(p);
		m_bLiveDirty = true;
		p.m_fEnterMs = System.GetTickCount();
		p.m_fEnterDist = NearestPlayer(p.m_vPos);

		RZ_CampGround ground = new RZ_CampGround();
		ReadGround(p, ground);
		UseSite(p, ground, FindSite(p, ground));
		if (!p.m_Site && kind != RZ_ECampPlace.ROAD_POST)
		{
			p.m_vHome = DryNear(p.m_vPos);
			p.m_vCentre = p.m_vHome;
		}
		AddLeaderSeats(p);
		if (m_Settings.m_bDressing && !IsDressedNear(p) && !LayHandBuilt(p))
			Dress(p, ground);
	}

	//------------------------------------------------------------------------------------------------
	protected void InitPlace(RZ_CampPlace p, RZ_EFaction faction, RZ_ECampPlace kind, vector pos)
	{
		p.m_eFaction = faction;
		p.m_eKind = kind;
		p.m_vPos = pos;
		p.m_vHome = pos;
		p.m_vCentre = pos;
		p.m_iSeed = Seed(pos, faction, kind);
	}

	//------------------------------------------------------------------------------------------------
	//! Where a place's people spawn: the yard of the building it moves into, else pos. The owner
	//! calls this before it spawns the bodies; Enter reuses the building it found.
	vector HomeFor(string key, RZ_EFaction faction, RZ_ECampPlace kind, vector pos)
	{
		vector hand;
		if (m_Settings.m_bDressing && HandHome(kind, pos, hand))
			return hand;

		if (!m_Settings.m_bDressing || !m_Settings.m_bBuildings || kind == RZ_ECampPlace.ROAD_POST)
			return pos;

		RZ_CampPlace p = new RZ_CampPlace();
		p.m_sKey = key;
		InitPlace(p, faction, kind, pos);
		RZ_CampGround ground = new RZ_CampGround();
		ReadGround(p, ground);
		RZ_CampSite site = FindSite(p, ground);
		if (!site || site.m_bNone)
			return DryNear(pos);

		return site.m_vHome;
	}

	//------------------------------------------------------------------------------------------------
	//! Whether a place of this kind at pos finds a building to move into (false: it would stand on
	//! rings). True with buildings off: nothing to judge by.
	bool HasBuilding(RZ_EFaction faction, RZ_ECampPlace kind, vector pos)
	{
		if (!m_Settings.m_bDressing || !m_Settings.m_bBuildings || kind == RZ_ECampPlace.ROAD_POST)
			return true;

		RZ_CampPlace p = new RZ_CampPlace();
		p.m_sKey = "probe";
		InitPlace(p, faction, kind, pos);
		RZ_CampGround ground = new RZ_CampGround();
		ReadGround(p, ground);
		RZ_CampSite site = PickSite(p, ground, false);
		if (site.m_bNone)
			site = PickSite(p, ground, true);

		return !site.m_bNone;
	}

	//------------------------------------------------------------------------------------------------
	//! Where a place of this kind should stand in a town (docs/SITES.md, "Where a place stands"): the
	//! yard of the building within `radius` of the town's middle that suits it best, on dry and even
	//! ground. `tier` is how well it suits (SiteTier). False when the town has no such building (or
	//! buildings are off): the caller tries another town, or keeps its own spot.
	bool BestSpotIn(RZ_EFaction faction, RZ_ECampPlace kind, vector centre, float radius, out vector spot, out int tier)
	{
		spot = centre;
		tier = -1;
		if (!m_Settings.m_bDressing || !m_Settings.m_bBuildings || kind == RZ_ECampPlace.ROAD_POST)
			return false;

		RZ_CampPlace p = new RZ_CampPlace();
		p.m_sKey = "probe";
		InitPlace(p, faction, kind, centre);
		RZ_CampGround ground = new RZ_CampGround();
		ReadGround(p, ground, radius);
		RZ_CampSite site = PickSite(p, ground, false, radius);
		if (site.m_bNone)
			return false;

		spot = RZ_Util.GroundPos(site.m_vHome);
		if (WetAt(spot))
			return false;

		tier = site.m_iTier;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Dry and even round a building: eight points on a ring just outside it, none over water and no
	//! more than a few metres between the highest and the lowest (a camp on a beach or a hillside
	//! loses its pieces to the water and the slope).
	protected static bool EvenGround(IEntity b, vector mins, vector maxs, RZ_ECampPlace kind)
	{
		vector mid = RZ_DoorGeometry.ToWorld(b, (mins + maxs) * 0.5);
		float w = maxs[0] - mins[0];
		float d = maxs[2] - mins[2];
		float ring = 0.5 * Math.Sqrt(w * w + d * d) + 14;
		float relief = 6;
		if (kind == RZ_ECampPlace.STRONGHOLD)
		{
			ring += 8;
			relief = 8;
		}

		float lo = float.MAX;
		float hi = -float.MAX;
		for (int k = 0; k < 8; k++)
		{
			vector pt = RZ_Util.GroundPos(RZ_Compass.PointAt(mid, k * 45, ring));
			if (WetAt(pt))
				return false;

			lo = Math.Min(lo, pt[1]);
			hi = Math.Max(hi, pt[1]);
		}

		return hi - lo <= relief;
	}

	//------------------------------------------------------------------------------------------------
	//! `pos`, or if it's over water (a holding's spot on a pier, a quay or off the shore) the nearest
	//! dry ground of sixteen ways out to 80 m.
	static vector DryNear(vector pos)
	{
		if (!WetAt(pos))
			return pos;

		for (int r = 10; r <= 80; r += 10)
		{
			for (int k = 0; k < 16; k++)
			{
				vector pt = RZ_Compass.PointAt(pos, k * 22.5, r);
				if (!WetAt(pt))
					return RZ_Util.GroundPos(pt);
			}
		}

		return pos;
	}

	//------------------------------------------------------------------------------------------------
	//! The pier probe (the site audit, test only): for a faction's stronghold at `pos`, the building
	//! picked now and the one picked before the waterside was ruled out ("prefab (what), wet" when
	//! its box is over water), as "before -> now".
	string ProbeSite(RZ_EFaction faction, vector pos)
	{
		RZ_CampPlace p = new RZ_CampPlace();
		p.m_sKey = "probe";
		InitPlace(p, faction, RZ_ECampPlace.STRONGHOLD, pos);
		RZ_CampGround ground = new RZ_CampGround();
		ReadGround(p, ground);
		s_bProbeOld = true;
		RZ_CampSite before = PickSite(p, ground, false);
		if (before.m_bNone)
			before = PickSite(p, ground, true);

		s_bProbeOld = false;
		RZ_CampSite now = PickSite(p, ground, false);
		if (now.m_bNone)
			now = PickSite(p, ground, true);

		return ProbeText(before) + " -> " + ProbeText(now);
	}

	//------------------------------------------------------------------------------------------------
	protected static string ProbeText(RZ_CampSite site)
	{
		if (site.m_bNone)
			return "open ground";

		string text = site.m_sPrefab;
		if (WetAt(site.m_vCentre))
			text += " (over water)";

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! A place's lived-in ground, for what keeps away from it (RZ_WorldFacts.IsNearLivedIn): the yard
	//! of the building it moved into (or its rings) and how far its pieces may reach from there,
	//! standing ones included. False while its site hasn't been read (with buildings off, too).
	bool SiteReachOf(string key, out vector home, out float reach)
	{
		RZ_CampSite site;
		if (!m_mSites.Find(key, site) || !site)
			return false;

		if (site.m_bNone)
		{
			home = site.m_vPlace;
			reach = RINGS_REACH;
		}
		else
		{
			home = site.m_vHome;
			vector size = site.m_vMaxs - site.m_vMins;
			float halfDiag = 0.5 * Math.Sqrt(size[0] * size[0] + size[2] * size[2]);
			reach = RZ_Util.DistXZ(site.m_vHome, site.m_vCentre) + halfDiag + EntryRing(site.m_eKind);
		}

		foreach (RZ_CampPlace p : m_aPlaces)
		{
			if (p.m_sKey != key)
				continue;

			foreach (RZ_CampPiece piece : p.m_aPieces)
			{
				reach = Math.Max(reach, RZ_Util.DistXZ(piece.m_vPos, home) + 3);
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! From the owner's DespawnBodies, before it deletes them: jobs, wrappers, extras and pieces go.
	//! Every job's waypoints come off the groups first, so no group is left with a smart-action
	//! waypoint whose piece is gone (vanilla's "Waypoint is not set properly").
	void Leave(string key)
	{
		for (int i = m_aPlaces.Count() - 1; i >= 0; i--)
		{
			RZ_CampPlace p = m_aPlaces[i];
			if (p.m_sKey != key)
				continue;

			int waitingLeft = PendingCount(p);
			if (!p.m_bTimed && waitingLeft > 0)
				RZ_Log.Info(string.Format("Spawn timing: %1 left with %2 pieces up and %3 still to come, %4 ms after it came (the nearest player %5 m off then).", p.m_sTitle, p.m_iSpawnedCount, waitingLeft, RZ_Util.RoundToInt(System.GetTickCount() - p.m_fEnterMs), RZ_Util.RoundToInt(p.m_fEnterDist)));

			foreach (RZ_CampWorker w : p.m_aWorkers)
			{
				ClearJob(p, w);
				if (w.m_bWrapped)
					RZ_Util.UnwrapGroup(w.m_Unit, w.m_Group);

				if (w.m_bOwned)
					RZ_Util.DeleteUnit(w.m_Unit);
			}

			foreach (RZ_CampPiece piece : p.m_aPieces)
			{
				if (!piece.m_Entity)
					continue;

				s_aPieces.RemoveItem(piece.m_Entity);
				SCR_EntityHelper.DeleteEntityAndChildren(piece.m_Entity);
			}

			m_aPlaces.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AddWorkers(RZ_CampPlace p, array<IEntity> units, int role)
	{
		if (!units)
			return;

		foreach (IEntity u : units)
		{
			if (!u)
				continue;

			RZ_CampWorker w = new RZ_CampWorker();
			w.m_Unit = u;
			w.m_iRole = role;
			p.m_aWorkers.Insert(w);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AddLeaderSeats(RZ_CampPlace p)
	{
		RZ_Leaders leaders = m_GM.GetLeaders();
		if (!leaders)
			return;

		// The faction's placed quartermaster (a stronghold's, the Haven's anchored safehouse)
		vector qm;
		if (m_GM.GetPerks() && m_GM.GetPerks().GetQuartermasterPos(p.m_eFaction, qm) && NearPlace(p, qm))
			p.m_aLeaderSeats.Insert(qm);

		// An outpost's leader sits in the yard (RZ_Territory.RegisterLeader), up to 90 m from m_vPos
		array<RZ_Leader> list = {};
		leaders.GetFactionLeaders(p.m_eFaction, list);
		foreach (RZ_Leader l : list)
		{
			if (l && NearPlace(p, l.m_vPos))
				p.m_aLeaderSeats.Insert(l.m_vPos);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static bool NearPlace(RZ_CampPlace p, vector pos)
	{
		return RZ_Util.DistXZ(pos, p.m_vPos) < SHARE_RADIUS || RZ_Util.DistXZ(pos, p.m_vHome) < SHARE_RADIUS;
	}

	//------------------------------------------------------------------------------------------------
	protected static int Seed(vector pos, RZ_EFaction faction, RZ_ECampPlace kind)
	{
		int x = RZ_Util.RoundToInt(pos[0]);
		int z = RZ_Util.RoundToInt(pos[2]);
		int s = x * 7919 + z * 6151 + faction * 31 + kind * 17;
		return RZ_Util.PickIndex(s, 2000000000) + 1;
	}

	//------------------------------------------------------------------------------------------------
	//! A Haven safehouse at the Haven's own holding isn't dressed twice.
	protected bool IsDressedNear(RZ_CampPlace p)
	{
		foreach (RZ_CampPlace other : m_aPlaces)
		{
			if (other != p && !other.m_aPieces.IsEmpty() && RZ_Util.DistXZ(other.m_vPos, p.m_vPos) < SHARE_RADIUS)
				return true;
		}

		// One whose ground is still being read, with players near
		foreach (RZ_SiteJob job : m_aJobs)
		{
			if (!job.m_bAudit && job.m_sKey != p.m_sKey && RZ_Util.DistXZ(job.m_Place.m_vPos, p.m_vPos) < SHARE_RADIUS)
				return true;
		}

		return false;
	}

	// ================================================================ Hand-built sites (World/HandBuilt)
	protected ref set<string> m_aBuilding = new set<string>();	// site ids being built by hand now (RZ_SiteBuilder)

	//------------------------------------------------------------------------------------------------
	//! The hand-built site id a place answers to.
	static string HandIdOf(RZ_CampPlace p)
	{
		return RZ_HandBuilt.SiteId(p.m_eKind, p.m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	//! The place is being built by hand, or has a hand-built save in use: its objects are queued
	//! (a few spawn a tick, like pieces) and no automated piece is laid. False: lay it out as usual.
	//! People keep their posts and jobs; with no pieces their jobs fall back as at a bare place.
	protected bool LayHandBuilt(RZ_CampPlace p)
	{
		string what = string.Format("%1 at %2", p.m_sTitle, RZ_SettlementScanner.GridReference(p.m_vPos));
		RZ_HandBuiltSite site = RZ_HandBuilt.Get().ForPlace(p.m_eKind, p.m_vPos);
		bool building = IsBuilding(HandIdOf(p));
		if (site && IsBuilding(site.m_sId))
			building = true;

		if (building)
		{
			p.m_bHand = true;
			RZ_Log.Info(string.Format("Camp life: %1: being built by hand, automated layout off.", what));
			return true;
		}

		if (!site || !site.m_bUse)
			return false;

		p.m_bHand = true;
		foreach (RZ_HandBuiltObject o : site.m_aObjects)
		{
			RZ_CampPiece cp = new RZ_CampPiece();
			cp.m_sPending = o.m_sPrefab;
			cp.m_vPos = o.m_vPos;
			cp.m_vAngles = o.m_vAngles;
			cp.m_fScale = o.m_fScale;
			cp.m_fYaw = o.m_vAngles[0];
			cp.m_bHandObj = true;
			cp.m_eUse = RZ_EDressUse.NONE;
			cp.m_iSeats = 1;
			cp.m_sName = "HandBuilt";
			p.m_aPieces.Insert(cp);
		}

		int spots = LayHandSpots(p, site);
		array<IEntity> qms = {};
		Quartermasters(p, qms);
		foreach (IEntity qm : qms)
		{
			if (!PlaceKeeper(p, qm, RZ_HandSpots.First(site.m_aSpots, "qm")))
				TurnToServe(p, qm);
		}

		RZ_HandSpot medicSpot = RZ_HandSpots.First(site.m_aSpots, "medic");
		foreach (RZ_CampWorker k : p.m_aWorkers)
		{
			if (medicSpot && k.m_iRole == ROLE_KEEPER && k.m_Unit && !k.m_Unit.FindComponent(RZ_QuartermasterComponent))
				PlaceKeeper(p, k.m_Unit, medicSpot);
		}

		int missing;
		string check = RZ_HandSpots.Short(p.m_eFaction, p.m_eKind, site.m_aSpots, missing);
		RZ_Log.Info(string.Format("Camp life: %1: hand-built (%2 objects, %3 role spots, saved %4), automated layout off; its spots: %5.", what, site.m_aObjects.Count(), spots, site.m_sSaved, check));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The save's role spots (docs/SITES.md, "Role spots") become the place's pieces and posts: the
	//! routine sits, works, keeps watch, cooks, washes, chops and sleeps at them as at its own pieces'
	//! smart actions; the gate, the watch posts, the stores door and the walk points are its posts and
	//! routes; the canteen is home. A need with no spot falls back as at a bare place (a seat, walking
	//! the camp). How many spots it laid.
	protected int LayHandSpots(RZ_CampPlace p, RZ_HandBuiltSite site)
	{
		if (site.m_aSpots.IsEmpty())
			return 0;

		int n = 0;
		foreach (RZ_HandSpot sp : site.m_aSpots)
		{
			if (!RZ_HandSpots.IsPiece(sp.m_sType))
				continue;

			ResourceName prefab = m_Settings.m_sHandSpot;
			if (sp.m_sType == "meal" && !m_Settings.m_sHandSpotMeal.IsEmpty())
				prefab = m_Settings.m_sHandSpotMeal;

			if (prefab.IsEmpty())
				continue;

			RZ_CampPiece cp = new RZ_CampPiece();
			cp.m_sPending = prefab;
			cp.m_vPos = RZ_HandSpots.OnFloor(sp.m_vPos);
			cp.m_fYaw = sp.m_fYaw;
			cp.m_eUse = RZ_HandSpots.UseOf(sp.m_sType);
			cp.m_sTag = UseTag(cp.m_eUse);
			cp.m_iSeats = RZ_HandSpots.SeatsOf(sp);
			if (sp.m_sType == "meal" && prefab == m_Settings.m_sHandSpot)
				cp.m_iSeats = 1;

			cp.m_sName = RZ_HandSpots.NameOf(sp.m_sType);
			cp.m_bHandSpot = true;
			p.m_aPieces.Insert(cp);
			n++;
		}

		// Home: the canteen, else where the seats are; named people stay near it
		RZ_HandSpot meal = RZ_HandSpots.First(site.m_aSpots, "meal");
		if (!meal)
			meal = RZ_HandSpots.First(site.m_aSpots, "sit");

		if (meal)
			p.m_vHome = RZ_Util.GroundPos(meal.m_vPos);

		// Posts and walks
		RZ_SitePeople people = new RZ_SitePeople();
		people.m_vMid = site.m_vCentre;
		people.m_vGate = site.m_vCentre;
		people.m_vDoor = site.m_vCentre;
		RZ_HandSpot gate = RZ_HandSpots.First(site.m_aSpots, "gate");
		if (gate)
		{
			people.m_vGate = gate.m_vPos;
			people.m_fGateOut = gate.m_fYaw;
		}

		RZ_HandSpot stores = RZ_HandSpots.First(site.m_aSpots, "stores");
		if (stores)
			people.m_vDoor = stores.m_vPos;

		RZ_SiteRoute walk = new RZ_SiteRoute();
		walk.m_iKind = RZ_SiteRoute.APPROACH;
		foreach (RZ_HandSpot hs : site.m_aSpots)
		{
			if (hs.m_sType == "gate" || hs.m_sType == "watch")
			{
				RZ_SitePost post = new RZ_SitePost();
				post.m_iKind = RZ_SitePost.LOOKOUT;
				if (hs.m_sType == "gate")
					post.m_iKind = RZ_SitePost.GATE;

				post.m_vPos = hs.m_vPos;
				post.m_fFacing = hs.m_fYaw;
				post.m_bPiece = true;
				post.m_bSeesGate = true;
				if (post.m_iKind == RZ_SitePost.GATE)
					people.m_aPosts.InsertAt(post, 0);
				else
					people.m_aPosts.Insert(post);
			}
			else if (hs.m_sType == "stores")
			{
				RZ_SitePost door = new RZ_SitePost();
				door.m_iKind = RZ_SitePost.DOOR;
				door.m_vPos = hs.m_vPos;
				door.m_fFacing = hs.m_fYaw;
				people.m_aPosts.Insert(door);
			}
			else if (hs.m_sType == "route")
			{
				walk.m_aPoints.Insert(hs.m_vPos);
				walk.m_aStop.Insert(false);
			}
		}

		if (walk.m_aPoints.Count() >= 2)
		{
			walk.m_aStop[walk.m_aStop.Count() - 1] = true;
			walk.m_fOut = RZ_Util.DistXZ(people.m_vGate, walk.m_aPoints[walk.m_aPoints.Count() - 1]);
			people.m_aRoutes.Insert(walk);

			// Halcyon's courier and the friendly side take the same walk
			RZ_SiteRoute friendly = new RZ_SiteRoute();
			friendly.m_iKind = RZ_SiteRoute.FRIENDLY;
			friendly.m_aPoints.Copy(walk.m_aPoints);
			friendly.m_aStop.Copy(walk.m_aStop);
			friendly.m_fOut = walk.m_fOut;
			people.m_aRoutes.Insert(friendly);
		}

		p.m_People = people;
		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! A quartermaster or the medic stood at their hand-placed spot, facing its way. False: no spot.
	protected bool PlaceKeeper(RZ_CampPlace p, IEntity person, RZ_HandSpot spot)
	{
		if (!person || !spot)
			return false;

		vector mat[4];
		Math3D.AnglesToMatrix(Vector(spot.m_fYaw, 0, 0), mat);
		mat[3] = RZ_Util.GroundPos(spot.m_vPos);
		person.SetWorldTransform(mat); // VERIFY: an AI character standing with no job moved and turned with SetWorldTransform on the server stays there, facing that way, on every client
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A hand-built site's own home (its leader's place, else its canteen), for HomeFor; false: none.
	protected bool HandHome(RZ_ECampPlace kind, vector pos, out vector home)
	{
		RZ_HandBuiltSite site = RZ_HandBuilt.Get().ForPlace(kind, pos);
		if (!site || !site.m_bUse)
			return false;

		RZ_HandSpot spot = RZ_HandSpots.First(site.m_aSpots, "leader");
		if (!spot)
			spot = RZ_HandSpots.First(site.m_aSpots, "meal");

		if (!spot)
			return false;

		home = RZ_Util.GroundPos(spot.m_vPos);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The pieces and posts of the place's site, as the site builder's suggestions for missing spots:
	//! where the automated layout has a piece of each use (and its gate, door and quartermasters).
	void Suggestions(string siteId, vector centre, float radius, notnull array<ref RZ_HandSpot> result)
	{
		foreach (RZ_CampPlace p : m_aPlaces)
		{
			if (HandIdOf(p) != siteId && RZ_Util.DistXZ(p.m_vPos, centre) > radius)
				continue;

			foreach (RZ_CampPiece piece : p.m_aPieces)
			{
				if (piece.m_bHandObj || piece.m_sName.IndexOf("HandSpot_") == 0)
					continue;

				string type = RZ_HandSpots.TypeOfPiece(piece.m_eUse, piece.m_sName);
				if (!type.IsEmpty())
					Suggest(result, type, piece.m_vPos, piece.m_fYaw);
			}

			if (p.m_People)
			{
				Suggest(result, "gate", p.m_People.m_vGate, p.m_People.m_fGateOut);
				Suggest(result, "stores", p.m_People.m_vDoor, 0);
			}

			array<IEntity> qms = {};
			Quartermasters(p, qms);
			foreach (IEntity qm : qms)
			{
				Suggest(result, "qm", qm.GetOrigin(), qm.GetYawPitchRoll()[0]);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Suggest(notnull array<ref RZ_HandSpot> result, string type, vector pos, float yaw)
	{
		if (pos == vector.Zero)
			return;

		RZ_HandSpot sp = new RZ_HandSpot();
		sp.m_sType = type;
		sp.m_vPos = pos;
		sp.m_fYaw = yaw;
		result.Insert(sp);
	}

	//------------------------------------------------------------------------------------------------
	bool IsBuilding(string siteId)
	{
		return m_aBuilding.Contains(siteId);
	}

	//------------------------------------------------------------------------------------------------
	//! The site builder starts on a site: the places there drop their pieces (deleted, or handed to
	//! `keep` to be built on: "Start from the automated layout") and lay none while it's built. How
	//! many pieces there were.
	int StartBuilding(string siteId, vector centre, float radius, array<IEntity> keep)
	{
		m_aBuilding.Insert(siteId);
		int n = 0;
		foreach (RZ_CampPlace p : m_aPlaces)
		{
			if (HandIdOf(p) != siteId && RZ_Util.DistXZ(p.m_vPos, centre) > radius)
				continue;

			n += DropPieces(p, keep);
			p.m_bHand = true;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! The site builder is done with a site: the places there are laid out again, from the save in use
	//! or automated.
	void StopBuilding(string siteId, vector centre, float radius)
	{
		m_aBuilding.RemoveItem(siteId);
		foreach (RZ_CampPlace p : m_aPlaces)
		{
			if (HandIdOf(p) != siteId && RZ_Util.DistXZ(p.m_vPos, centre) > radius)
				continue;

			DropPieces(p, null);
			p.m_bHand = false;
			if (!m_Settings.m_bDressing || LayHandBuilt(p))
				continue;

			RZ_CampGround ground = new RZ_CampGround();
			ReadGround(p, ground);
			Dress(p, ground);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The place's pieces go (into `keep` when given, else deleted); its people's jobs are handed out
	//! again (none of them uses a piece any more).
	protected int DropPieces(RZ_CampPlace p, array<IEntity> keep)
	{
		foreach (RZ_CampWorker w : p.m_aWorkers)
		{
			ClearJob(p, w);
			w.m_iPiece = -1;
			w.m_iSeat = -1;
		}

		int n = 0;
		foreach (RZ_CampPiece piece : p.m_aPieces)
		{
			if (!piece.m_Entity)
				continue;

			n++;
			s_aPieces.RemoveItem(piece.m_Entity);
			if (keep)
				keep.Insert(piece.m_Entity);
			else
				SCR_EntityHelper.DeleteEntityAndChildren(piece.m_Entity);
		}

		p.m_aPieces.Clear();
		p.m_iPhase = -2;
		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! How many of camp life's pieces stand at the site now (a place there is live, with a player near).
	int PiecesAt(string siteId, vector centre, float radius)
	{
		int n = 0;
		foreach (RZ_CampPlace p : m_aPlaces)
		{
			if (HandIdOf(p) != siteId && RZ_Util.DistXZ(p.m_vPos, centre) > radius)
				continue;

			foreach (RZ_CampPiece piece : p.m_aPieces)
			{
				if (piece.m_Entity)
					n++;
			}
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! A place there is live now (its people are out, a player near).
	bool IsLiveAt(string siteId, vector centre, float radius)
	{
		foreach (RZ_CampPlace p : m_aPlaces)
		{
			if (HandIdOf(p) == siteId || RZ_Util.DistXZ(p.m_vPos, centre) <= radius)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Every site camp life lays out on this map, as the site builder lists them: the holdings, the
	//! Haven safehouses and the road posts standing now (the audit's list).
	void ListSites(notnull array<ref RZ_SiteEntry> result)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory)
		{
			foreach (RZ_Holding h : territory.GetHoldings())
			{
				RZ_ECampPlace kind = RZ_ECampPlace.OUTPOST;
				if (h.m_bStronghold)
					kind = RZ_ECampPlace.STRONGHOLD;

				result.Insert(RZ_SiteEntry.Make(h.GetTitle(), h.m_eFaction, kind, h.m_vPos, 0));
			}
		}

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
		{
			for (int i = 0; i < houses.Count(); i++)
			{
				RZ_Safehouse house = houses.Get(i);
				if (house)
					result.Insert(RZ_SiteEntry.Make("Haven safehouse", RZ_EFaction.SURVIVORS, RZ_ECampPlace.SAFEHOUSE, house.m_vPos, 0));
			}
		}

		RZ_Checkpoints checkpoints = m_GM.GetCheckpoints();
		if (checkpoints)
		{
			array<RZ_RoadPost> posts = {};
			checkpoints.PostsNear(-1, vector.Zero, 1000000, posts);
			foreach (RZ_RoadPost post : posts)
			{
				result.Insert(RZ_SiteEntry.Make(post.GetTitle(), post.m_eFaction, RZ_ECampPlace.ROAD_POST, post.m_vPos, post.m_fYaw));
			}
		}
	}

	// ================================================================ Dressing
	//------------------------------------------------------------------------------------------------
	protected RZ_DressingKit FindKit(RZ_EFaction f, RZ_ECampPlace kind)
	{
		if (!m_Settings.m_aKits)
			return null;

		RZ_DressingKit any;
		foreach (RZ_DressingKit kit : m_Settings.m_aKits)
		{
			if (!kit || kit.m_eFaction != f)
				continue;

			if (kit.m_ePlace == kind)
				return kit;

			if (kit.m_ePlace == RZ_ECampPlace.ANY && !any)
				any = kit;
		}

		// Road posts are small: they only take a kit made for them
		if (kind == RZ_ECampPlace.ROAD_POST)
			return null;

		return any;
	}

	//------------------------------------------------------------------------------------------------
	protected int PieceCap(RZ_CampPlace p)
	{
		switch (p.m_eKind)
		{
			case RZ_ECampPlace.STRONGHOLD: return m_Settings.m_iPiecesStronghold;
			case RZ_ECampPlace.SAFEHOUSE: return m_Settings.m_iPiecesSafehouse;
			case RZ_ECampPlace.ROAD_POST: return m_Settings.m_iPiecesRoadPost;
		}

		if (p.m_eFaction == RZ_EFaction.INSTITUTE)
			return m_Settings.m_iPiecesFieldStation;

		return m_Settings.m_iPiecesOutpost;
	}

	//------------------------------------------------------------------------------------------------
	//! Dresses a place (docs/DESIGN.md, "Camp life"; docs/SITES.md): with Site grid on, its layout
	//! from this run's cache if it was worked out before, else its ground is read on a grid a little
	//! each second (Survey) and then it's laid out; off, it's laid out at once with the point checks.
	protected void Dress(RZ_CampPlace p, RZ_CampGround ground)
	{
		RZ_DressingKit kit = FindKit(p.m_eFaction, p.m_eKind);
		if (!kit || !kit.m_aPieces || kit.m_aPieces.IsEmpty())
		{
			array<IEntity> qms = {};
			Quartermasters(p, qms);
			foreach (IEntity lone : qms)
			{
				TurnToServe(p, lone);
			}

			string told = string.Format("%1:%2", p.m_eFaction, p.m_eKind);
			if (!m_mToldNoKit.Contains(told))
			{
				m_mToldNoKit.Set(told, true);
				RZ_Log.Info(string.Format("Dressing: no kit for %1 (%2); those places stay bare.", RZ_Factions.GetName(p.m_eFaction), typename.EnumToString(RZ_ECampPlace, p.m_eKind)));
			}

			return;
		}

		if (!m_Settings.m_bSiteGrid)
		{
			Lay(p, ground, null, new RZ_SiteResult(), false);
			return;
		}

		RZ_SiteLayout kept = KeptLayout(p);
		if (kept)
		{
			LayKept(p, ground, kept);
			return;
		}

		Survey(p, ground, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Lays the kit's plan down whole (docs/DESIGN.md, "Camp life"): first a counter in front of each
	//! quartermaster, then the plan in its order, each piece where its part of the camp puts it, kept
	//! only if its whole footprint fits right there (docs/SITES.md, "Drop, don't squeeze"), until the
	//! place's count. With a grid, every cell of the footprint is tested; result gets every piece
	//! placed and every piece dropped, with why. Audit: worked out as data only (nothing spawns, and
	//! the server's piece cap doesn't apply).
	protected void Lay(RZ_CampPlace p, RZ_CampGround ground, RZ_SiteGrid grid, RZ_SiteResult result, bool audit)
	{
		RZ_DressingKit kit = FindKit(p.m_eFaction, p.m_eKind);
		RZ_CampFit fit = ReadFit(p, ground);
		fit.m_Grid = grid;
		int cap = PieceCap(p);
		int laid = PlaceCounters(p, fit, kit, cap, result, audit);

		// The plan: in a building they sleep indoors (no tents)
		array<RZ_DressingPiece> plan = {};
		foreach (RZ_DressingPiece e : kit.m_aPieces)
		{
			if (!e || e.m_sPrefab.IsEmpty() || e.m_eAt == RZ_EDressAt.COUNTER)
				continue;

			if (p.m_Site && e.m_eUse == RZ_EDressUse.SLEEP)
				continue;

			plan.Insert(e);
		}

		RZ_CampLayout lay = new RZ_CampLayout();
		if (p.m_Site)
			ChooseYard(p, fit, plan, cap, lay);
		else
			ChooseOpen(p, ground, fit, plan, cap, lay);

		result.m_sWhere = lay.m_sWhere;
		int seaward = 0;
		foreach (RZ_DressingPiece entry : plan)
		{
			if (laid >= cap)
				break;

			result.m_iTried++;
			vector pos;
			float yaw;
			Slot(p, lay, entry, pos, yaw);

			// A nest, a barrier or a sign looks out at the land: never at the sea or over a drop
			if (FacesOut(entry) && BadOutlook(pos, yaw, BARRIER_OUTLOOK))
			{
				seaward++;
				Dropped(result, entry, pos, yaw, "faces water or a drop");
				continue;
			}

			if (!Fits(fit, pos, yaw, entry, true))
			{
				Dropped(result, entry, pos, yaw, fit.m_sWhy);
				continue;
			}

			AddPiece(p, fit, entry, pos, yaw, Spawns(audit));
			Placed(result, entry, pos, yaw);
			laid++;
		}

		if (audit)
			return;

		string what = string.Format("%1 %2", RZ_Factions.GetName(p.m_eFaction), typename.EnumToString(RZ_ECampPlace, p.m_eKind));
		RZ_Log.Info(string.Format("Camp life: %1 at %2: %3 of %4 planned pieces fit, laid out %5; %6 counter(s), %7 left out facing water or a drop.", what, RZ_SettlementScanner.GridReference(p.m_vPos), result.m_aPlaced.Count() - result.m_iCounters, result.m_iTried, lay.m_sWhere, result.m_iCounters, seaward));
	}

	//------------------------------------------------------------------------------------------------
	//! The counters: each in front of a quartermaster, who turns to serve over it. How many pieces
	//! that laid.
	protected int PlaceCounters(RZ_CampPlace p, RZ_CampFit fit, RZ_DressingKit kit, int cap, RZ_SiteResult result, bool audit)
	{
		array<IEntity> qms = {};
		Quartermasters(p, qms);
		int served = 0;
		int laid = 0;
		foreach (RZ_DressingPiece c : kit.m_aPieces)
		{
			if (!c || c.m_sPrefab.IsEmpty() || c.m_eAt != RZ_EDressAt.COUNTER)
				continue;

			if (served >= qms.Count())
				break;

			if (laid < cap && Spawns(audit) && Counter(p, fit, c, qms[served], result, audit))
				laid++;
			else if (!audit)
				TurnToServe(p, qms[served]);

			served++;
		}

		for (int k = served; k < qms.Count(); k++)
		{
			if (!audit)
				TurnToServe(p, qms[k]);
		}

		return laid;
	}

	//------------------------------------------------------------------------------------------------
	//! A piece laid out as data spawns only while the server's piece cap allows (the audit: never
	//! spawns, so it's never held back).
	protected bool Spawns(bool audit)
	{
		if (audit)
			return true;

		return CountPieces() < m_Settings.m_iPiecesMax;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Placed(RZ_SiteResult result, RZ_DressingPiece entry, vector pos, float yaw)
	{
		RZ_SitePlaced placed = new RZ_SitePlaced();
		placed.m_Entry = entry;
		placed.m_vPos = pos;
		placed.m_fYaw = yaw;
		result.m_aPlaced.Insert(placed);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Dropped(RZ_SiteResult result, RZ_DressingPiece entry, vector pos, float yaw, string why)
	{
		RZ_SitePlaced dropped = new RZ_SitePlaced();
		dropped.m_Entry = entry;
		dropped.m_vPos = pos;
		dropped.m_fYaw = yaw;
		dropped.m_sWhy = why;
		result.m_aDropped.Insert(dropped);
	}

	// ================================================================ The site grid and its layout
	//------------------------------------------------------------------------------------------------
	//! This run's layout of the place, if it was worked out before for the same faction and kind.
	protected RZ_SiteLayout KeptLayout(RZ_CampPlace p)
	{
		RZ_SiteLayout kept;
		if (!m_mLayouts.Find(p.m_sKey, kept) || !kept)
			return null;

		if (kept.m_eFaction != p.m_eFaction || kept.m_eKind != p.m_eKind || RZ_Util.DistXZ(kept.m_vPlace, p.m_vPos) > 1)
			return null;

		return kept;
	}

	//------------------------------------------------------------------------------------------------
	//! Keeps a laid-out site's pieces (not its counters) for the next visit.
	protected RZ_SiteLayout KeepLayout(RZ_CampPlace p, RZ_SiteResult result)
	{
		if (m_mLayouts.Count() >= MAX_SITES)
			m_mLayouts.Clear();

		RZ_SiteLayout kept = new RZ_SiteLayout();
		kept.m_eFaction = p.m_eFaction;
		kept.m_eKind = p.m_eKind;
		kept.m_vPlace = p.m_vPos;
		kept.m_sWhere = result.m_sWhere;
		kept.m_People = result.m_People;
		foreach (RZ_DressingPiece alive : result.m_aKeepAlive)
		{
			kept.m_aKeepAlive.Insert(alive);
		}

		foreach (RZ_SitePlaced placed : result.m_aPlaced)
		{
			if (placed.m_Entry.m_eAt != RZ_EDressAt.COUNTER)
				kept.m_aPieces.Insert(placed);
		}

		m_mLayouts.Set(p.m_sKey, kept);
		return kept;
	}

	//------------------------------------------------------------------------------------------------
	//! A place whose layout was worked out before this run: its pieces where they were, and its
	//! counters fitted afresh round them (the quartermaster stands where he stands now).
	protected void LayKept(RZ_CampPlace p, RZ_CampGround ground, RZ_SiteLayout kept)
	{
		RZ_DressingKit kit = FindKit(p.m_eFaction, p.m_eKind);
		RZ_CampFit fit = ReadFit(p, ground);
		foreach (RZ_SitePlaced placed : kept.m_aPieces)
		{
			Quad(placed.m_vPos, placed.m_fYaw, placed.m_Entry.m_vFootMin, placed.m_Entry.m_vFootMax, PIECE_GAP * 0.5, fit.m_aTaken);
		}

		int cap = PieceCap(p);
		RZ_SiteResult result = new RZ_SiteResult();
		int laid = PlaceCounters(p, fit, kit, cap, result, false);
		int layers = 0;
		foreach (RZ_SitePlaced again : kept.m_aPieces)
		{
			if (again.m_bIndoors || again.m_iLayer == RZ_ESiteLayer.STORY)
				continue;

			if (again.m_iLayer == RZ_ESiteLayer.PLAN)
			{
				if (laid >= cap)
					continue;

				laid++;
			}
			else
			{
				if (layers >= m_Settings.m_iLayerPieces)
					continue;

				layers++;
			}

			AddPiece(p, fit, again.m_Entry, again.m_vPos, again.m_fYaw, Spawns(false), again.m_fSlope);
		}

		string what = string.Format("%1 %2", RZ_Factions.GetName(p.m_eFaction), typename.EnumToString(RZ_ECampPlace, p.m_eKind));
		RZ_Log.Info(string.Format("Camp life: %1 at %2: laid out again as before (%3 pieces and %4 of its layers, %5), %6 counter(s).", what, RZ_SettlementScanner.GridReference(p.m_vPos), laid - result.m_iCounters, layers, kept.m_sWhere, result.m_iCounters));
		p.m_People = kept.m_People;
		SpawnStory(p, kept);
	}

	//------------------------------------------------------------------------------------------------
	//! A solved site's pieces (not its counters, already up, nor what went indoors) queued to spawn,
	//! while the server's cap allows.
	protected void SpawnLayout(RZ_CampPlace p, RZ_SiteResult result)
	{
		int layers = 0;
		foreach (RZ_SitePlaced placed : result.m_aPlaced)
		{
			if (placed.m_bIndoors || placed.m_Entry.m_eAt == RZ_EDressAt.COUNTER || placed.m_iLayer == RZ_ESiteLayer.STORY)
				continue;

			if (placed.m_iLayer != RZ_ESiteLayer.PLAN)
				layers++;

			QueuePiece(p, placed.m_Entry, placed.m_vPos, placed.m_fYaw, placed.m_fSlope);
		}

		string what = string.Format("%1 %2", RZ_Factions.GetName(p.m_eFaction), typename.EnumToString(RZ_ECampPlace, p.m_eKind));
		string wire = "no wire";
		if (result.m_Wire)
			wire = "the wire " + result.m_Wire.Share().ToString() + "% closed";

		RZ_Log.Info(string.Format("Camp life: %1 at %2: solved, %3 pieces and %4 of its layers (%5), %6 indoors, %7 dropped, laid out %8; %9 counter(s).", what, RZ_SettlementScanner.GridReference(p.m_vPos), result.m_aPlaced.Count() - layers - result.m_iCounters - result.m_iIndoors - StorySpotCount(result), layers, wire, result.m_iIndoors, result.m_aDropped.Count(), result.m_sWhere, result.m_iCounters));
		p.m_People = result.m_People;
		SpawnStory(p, KeptLayout(p));
	}

	//------------------------------------------------------------------------------------------------
	protected static int StorySpotCount(RZ_SiteResult result)
	{
		int n = 0;
		foreach (RZ_SitePlaced placed : result.m_aPlaced)
		{
			if (placed.m_iLayer == RZ_ESiteLayer.STORY)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! How far each way the grid reaches: round the building, its yard and its road entries; on open
	//! ground the plan and its shifts; at a road post the verges.
	protected static float GridHalf(RZ_CampPlace p)
	{
		if (p.m_eKind == RZ_ECampPlace.ROAD_POST)
			return GRID_POST;

		if (!p.m_Site)
			return GRID_OPEN;

		vector size = p.m_Site.m_vMaxs - p.m_Site.m_vMins;
		float halfDiag = 0.5 * Math.Sqrt(size[0] * size[0] + size[2] * size[2]);
		return Math.Clamp(halfDiag + EntryRing(p.m_eKind) + 6, GRID_OPEN, GRID_MAX);
	}

	//------------------------------------------------------------------------------------------------
	//! Starts reading a place's ground on a grid (SecondTick reads it, one site at a time). A place
	//! already being read takes over the job (it came back before its read was done).
	protected void Survey(RZ_CampPlace p, RZ_CampGround ground, bool audit)
	{
		foreach (RZ_SiteJob queued : m_aJobs)
		{
			if (queued.m_sKey != p.m_sKey)
				continue;

			if (!audit)
			{
				queued.m_Place = p;
				queued.m_Ground = ground;
				queued.m_bAudit = false;
				// Players are near: it goes first (a pre-read site already half read or solved)
				if (m_aJobs[0] != queued)
				{
					m_aJobs.RemoveItemOrdered(queued);
					m_aJobs.InsertAt(queued, 0);
				}
			}

			return;
		}

		vector centre = p.m_vCentre;
		if (p.m_Site)
			centre = p.m_Site.m_vCentre;

		float half = GridHalf(p);
		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(centre, half * 1.42 + 5, buildings, 200);
		foreach (IEntity b : ground.m_aBuildings)
		{
			if (b && !buildings.Contains(b))
				buildings.Insert(b);
		}

		RZ_SiteJob job = new RZ_SiteJob();
		job.m_sKey = p.m_sKey;
		job.m_Place = p;
		job.m_Ground = ground;
		job.m_bAudit = audit;
		job.m_bDraw = audit && m_bDrawNext;
		job.m_iQueuedAt = System.GetTickCount();
		job.m_Grid = new RZ_SiteGrid(centre, half, buildings, ground.m_aRoad, ground.m_aRoadWidth);
		// A place with players near goes before the audit's queue
		if (!audit)
		{
			int at = 0;
			while (at < m_aJobs.Count() && !m_aJobs[at].m_bAudit)
			{
				at++;
			}

			m_aJobs.InsertAt(job, at);
			return;
		}

		m_aJobs.Insert(job);
	}

	//------------------------------------------------------------------------------------------------
	//! Every second (the game mode's one-second tick): the site being read reads on (Site read ms);
	//! once read, it's solved (the same budget a second, docs/SITES.md "The solver"), then kept,
	//! audited and spawned, and its grid freed.
	void SecondTick()
	{
		PreReadStart();
		if (m_aJobs.IsEmpty())
			return;

		// A site a player has come near reads on in FastTick instead
		if (!m_aJobs[0].m_bAudit && m_Settings.m_iSpawnMs > 0)
			return;

		RZ_SiteJob job = m_aJobs[0];
		int budget = Math.Max(1, m_Settings.m_iSiteReadMs);
		if (job.m_bAudit && s_iAuditReadMs > 0)
			budget = s_iAuditReadMs;

		StepJob(job, budget);
	}

	//------------------------------------------------------------------------------------------------
	//! Every tenth of a second (the game mode's fast tick), with Spawn ms on: a site a player has come
	//! near reads and is laid out on its own larger budget (Site read ms near), then pieces waiting
	//! near players spawn (Spawn ms).
	void FastTick()
	{
		if (m_Settings.m_iSpawnMs <= 0)
			return;

		if (!m_aJobs.IsEmpty() && !m_aJobs[0].m_bAudit)
			StepJob(m_aJobs[0], Math.Max(1, m_Settings.m_iSiteReadNearMs));

		if (m_aPlaces.IsEmpty())
			return;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		SpawnPending(1000, m_Settings.m_iSpawnMs, players);
	}

	//------------------------------------------------------------------------------------------------
	//! The first job reads, then solves, on budget ms; done, it's laid out and freed.
	protected void StepJob(RZ_SiteJob job, int budget)
	{
		if (!job.m_Grid.IsRead())
		{
			if (job.m_Grid.Step(budget))
				StartSolve(job);

			return;
		}

		if (job.m_Solver && !job.m_Solver.Step(budget))
			return;

		m_aJobs.RemoveOrdered(0);
		FinishSurvey(job);
	}

	//------------------------------------------------------------------------------------------------
	//! The grid is read: the surroundings, then the solver (from the next second), or with Solver off
	//! the plan laid down whole at once.
	protected void StartSolve(RZ_SiteJob job)
	{
		RZ_CampPlace p = job.m_Place;
		job.m_Around = ReadSurroundings(p, job.m_Grid);
		RZ_DressingKit kit = FindKit(p.m_eFaction, p.m_eKind);
		if (m_Settings.m_bSolver && kit)
		{
			job.m_Solver = new RZ_SiteSolver(this, p, job.m_Ground, job.m_Grid, job.m_Around, kit, job.m_bAudit || !m_aPlaces.Contains(p));
			return;
		}

		bool live = !job.m_bAudit && m_aPlaces.Contains(p);
		job.m_Result = new RZ_SiteResult();
		int t0 = System.GetTickCount();
		Lay(p, job.m_Ground, job.m_Grid, job.m_Result, !live);
		job.m_Result.m_iSolveMs = System.GetTickCount() - t0;
	}

	//------------------------------------------------------------------------------------------------
	//! Kept, audited and (with players near) spawned. A place that came back while it was being
	//! solved lays the kept layout out afresh.
	protected void FinishSurvey(RZ_SiteJob job)
	{
		RZ_CampPlace p = job.m_Place;
		bool live = !job.m_bAudit && m_aPlaces.Contains(p);
		RZ_SiteResult result = job.m_Result;
		if (job.m_Solver)
			result = job.m_Solver.m_Result;

		if (!result)
			return;

		RZ_SiteLayout kept = KeepLayout(p, result);
		m_Audit.Site(this, p, job.m_Ground, job.m_Grid, job.m_Around, result, job.m_bDraw);
		if (!live)
			return;

		if (job.m_Solver && job.m_Solver.m_Place == p)
			SpawnLayout(p, result);
		else if (job.m_Solver)
			LayKept(p, job.m_Ground, kept);

		m_bLiveDirty = true;
	}

	//------------------------------------------------------------------------------------------------
	//! The site's surroundings and its threat sectors (docs/SITES.md, "The surroundings").
	protected RZ_SiteSurroundings ReadSurroundings(RZ_CampPlace p, RZ_SiteGrid grid)
	{
		RZ_SiteSurroundings around = new RZ_SiteSurroundings();
		vector middle = p.m_vPos;
		array<float> entries = {};
		float main = -1;
		int through = 0;
		if (p.m_Site)
		{
			middle = p.m_Site.m_vCentre;
			foreach (int i, vector e : p.m_Site.m_aEntries)
			{
				entries.Insert(RZ_Compass.Bearing(middle, e));
				int id = p.m_Site.m_aEntryRoadId[i];
				for (int j = i + 1; j < p.m_Site.m_aEntryRoadId.Count(); j++)
				{
					if (p.m_Site.m_aEntryRoadId[j] == id)
						through++;
				}
			}

			if (p.m_Site.m_iMain >= 0)
				main = entries[p.m_Site.m_iMain];
		}
		else if (p.m_bHasRoad)
		{
			main = p.m_fApproach;
		}

		around.Read(m_GM, p.m_eFaction, middle, p.m_vHome, grid, entries, main, through);
		return around;
	}

	//------------------------------------------------------------------------------------------------
	//! Pre-reading (Pre-read, docs/SITES.md): once the land grid is read (its lairs are part of a
	//! site's surroundings; at most 10 minutes after the start), every site on the map is queued to
	//! be read and laid out as data, strongholds first; a place a player comes near goes before them.
	protected void PreReadStart()
	{
		if (m_bPreReadQueued || s_bNoPreRead || !m_Settings.m_bPreRead || !m_Settings.m_bSiteGrid || !m_Settings.m_bDressing)
			return;

		RZ_Land land = m_GM.GetLand();
		bool ready = !land || !land.IsEnabled() || land.IsReady();
		m_fPreReadWait += 1;
		if (!ready && m_fPreReadWait < 600)
			return;

		m_bPreReadQueued = true;
		int queued = QueueAll(false);
		RZ_Log.Info(string.Format("Camp life: pre-reading %1 sites in the background, strongholds first.", queued));
	}

	//------------------------------------------------------------------------------------------------
	//! The headless audit (docs/SITES.md, "The audit"; the runtime test): one place read and laid out
	//! as data, nothing spawned. False if it has no kit or a place within 60 m was already queued.
	bool AuditPlace(string key, string title, RZ_EFaction faction, RZ_ECampPlace kind, vector pos, float yaw)
	{
		RZ_DressingKit kit = FindKit(faction, kind);
		if (!kit || !kit.m_aPieces || kit.m_aPieces.IsEmpty())
			return false;

		// A hand-built site in use isn't laid out, so it isn't scored (it's listed on its own)
		RZ_HandBuiltSite hand = RZ_HandBuilt.Get().ForPlace(kind, pos);
		if (!hand)
			RZ_HandBuilt.Get().TellNoMatch(title, kind, pos);

		if (hand && hand.m_bUse)
		{
			m_Audit.m_iHandBuilt++;
			RZ_Log.Info(string.Format("Sites audit: %1 at %2: hand-built (%3 objects, saved %4), not scored.", title, RZ_SettlementScanner.GridReference(pos), hand.m_aObjects.Count(), hand.m_sSaved));
			return false;
		}

		// Already laid out (and audited) this run, with players near
		if (m_mLayouts.Contains(key))
			return false;

		foreach (vector done : m_aAudited)
		{
			if (RZ_Util.DistXZ(done, pos) < SHARE_RADIUS)
				return false;
		}

		m_aAudited.Insert(pos);
		RZ_CampPlace p = new RZ_CampPlace();
		p.m_sKey = key;
		p.m_sTitle = title;
		p.m_fYaw = yaw;
		InitPlace(p, faction, kind, pos);
		if (kind == RZ_ECampPlace.ROAD_POST)
			p.m_aKeepClear.Insert(pos);

		RZ_CampGround ground = new RZ_CampGround();
		ReadGround(p, ground);
		UseSite(p, ground, FindSite(p, ground));
		if (!p.m_Site && kind != RZ_ECampPlace.ROAD_POST)
		{
			p.m_vHome = DryNear(p.m_vPos);
			p.m_vCentre = p.m_vHome;
		}
		AddLeaderSeats(p);
		Survey(p, ground, true);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The headless audit of every place on the map with a kit: the holdings, the Haven safehouses
	//! and the road posts standing now. How many were queued.
	int AuditAll()
	{
		int queued = QueueAll(true);
		RZ_Log.Info(string.Format("Sites audit: %1 sites queued.", queued));
		return queued;
	}

	//------------------------------------------------------------------------------------------------
	//! Every holding (strongholds and the lab first), Haven safehouse and road post standing now,
	//! queued to be read and laid out as data; `draw`: with the audit's drawing lines.
	protected int QueueAll(bool draw)
	{
		m_bDrawNext = draw;
		int queued = 0;
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory)
		{
			for (int pass = 0; pass < 2; pass++)
			{
				foreach (RZ_Holding h : territory.GetHoldings())
				{
					if (h.m_bStronghold != (pass == 0))
						continue;

					RZ_ECampPlace kind = RZ_ECampPlace.OUTPOST;
					if (h.m_bStronghold)
						kind = RZ_ECampPlace.STRONGHOLD;

					if (AuditPlace("h" + h.m_iId.ToString(), h.GetTitle(), h.m_eFaction, kind, h.m_vPos, 0))
						queued++;
				}
			}
		}

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
		{
			for (int i = 0; i < houses.Count(); i++)
			{
				RZ_Safehouse house = houses.Get(i);
				if (house && AuditPlace("s" + i.ToString(), "Haven safehouse", RZ_EFaction.SURVIVORS, RZ_ECampPlace.SAFEHOUSE, house.m_vPos, 0))
					queued++;
			}
		}

		RZ_Checkpoints checkpoints = m_GM.GetCheckpoints();
		if (checkpoints)
		{
			array<RZ_RoadPost> posts = {};
			checkpoints.PostsNear(-1, vector.Zero, 1000000, posts);
			foreach (RZ_RoadPost post : posts)
			{
				string key = string.Format("p%1_%2", RZ_Util.RoundToInt(post.m_vPos[0]), RZ_Util.RoundToInt(post.m_vPos[2]));
				if (AuditPlace(key, post.GetTitle(), post.m_eFaction, RZ_ECampPlace.ROAD_POST, post.m_vPos, post.m_fYaw))
					queued++;
			}
		}

		m_bDrawNext = false;
		return queued;
	}

	//------------------------------------------------------------------------------------------------
	//! The audit's summary line; how many strongholds scored under Audit threshold.
	int AuditSummary()
	{
		return m_Audit.Summary(m_Settings.m_iAuditThreshold);
	}

	//------------------------------------------------------------------------------------------------
	//! Sites still being read for the audit.
	int AuditPending()
	{
		int n = 0;
		foreach (RZ_SiteJob job : m_aJobs)
		{
			if (job.m_bAudit)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	RZ_SiteAudit GetAudit() { return m_Audit; }
	RZ_CampLifeSettings GetSettings() { return m_Settings; }

	//------------------------------------------------------------------------------------------------
	//! The gate of the faction place nearest `at` within `range` (RZ_SiteAudit.GatePoint: its road
	//! entry, else out toward its approach), and where its people gather inside. False: none near.
	bool GateNear(vector at, float range, out vector gate, out vector inside)
	{
		RZ_CampPlace best;
		float bestDist = range;
		foreach (RZ_CampPlace p : m_aPlaces)
		{
			float d = Math.Min(RZ_Util.DistXZ(at, p.m_vPos), RZ_Util.DistXZ(at, p.m_vHome));
			if (d <= bestDist)
			{
				bestDist = d;
				best = p;
			}
		}

		if (!best)
			return false;

		gate = RZ_SiteAudit.GatePoint(best);
		inside = best.m_vHome;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest laid-out piece of this use within range of `at` (a kit's cook pot, a hand-built
	//! site's cook spot: docs/ERRANDS.md, "Work sites"), and where someone stands to use it: in front
	//! of it, `standOff` m along its heading. False: no such piece is laid out there (yet).
	bool UseSpotNear(vector at, float range, RZ_EDressUse use, float standOff, out vector spot, out float yaw, out bool handBuilt)
	{
		RZ_CampPiece best;
		float bestDist = range;
		foreach (RZ_CampPlace p : m_aPlaces)
		{
			if (RZ_Util.DistXZ(at, p.m_vPos) > range + 150)
				continue;

			foreach (RZ_CampPiece cp : p.m_aPieces)
			{
				if (cp.m_eUse != use)
					continue;

				float d = RZ_Util.DistXZ(at, cp.m_vPos);
				if (d < bestDist)
				{
					bestDist = d;
					best = cp;
				}
			}
		}

		if (!best)
			return false;

		// A hand-placed spot is where the work is; a kit's piece is used from in front of it
		handBuilt = best.m_bHandSpot;
		float off = standOff;
		if (handBuilt)
			off = 0;

		float rad = best.m_fYaw * Math.DEG2RAD;
		spot = RZ_Util.GroundPos(best.m_vPos + Vector(Math.Sin(rad), 0, Math.Cos(rad)) * off);
		yaw = best.m_fYaw;
		return true;
	}

	// ================================================================ For the solver (RZ_SiteSolver)
	//------------------------------------------------------------------------------------------------
	RZ_CampFit ReadFitFor(RZ_CampPlace p, RZ_CampGround ground) { return ReadFit(p, ground); }
	int PieceCapOf(RZ_CampPlace p) { return PieceCap(p); }
	static RZ_EDressAt AtOfPiece(RZ_DressingPiece e) { return AtOf(e); }
	static float BoxDistanceOf(RZ_CampSite site, vector at) { return BoxDistance(site, at); }
	vector FacePointOf(RZ_CampSite site, int face, float along, float dist) { return FacePoint(site, face, along, dist); }

	//------------------------------------------------------------------------------------------------
	static void QuadOf(vector pos, float yaw, vector fmin, vector fmax, float grow, notnull array<vector> quad)
	{
		Quad(pos, yaw, fmin, fmax, grow, quad);
	}

	//------------------------------------------------------------------------------------------------
	static float QuadDistanceOf(vector pt, notnull array<vector> quad)
	{
		return QuadDistance(pt, quad, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! The quartermasters' counters (the solver's first pieces); how many pieces that laid.
	int PlaceCountersFor(RZ_CampPlace p, RZ_CampFit fit, RZ_DressingKit kit, int cap, RZ_SiteResult result, bool audit)
	{
		return PlaceCounters(p, fit, kit, cap, result, audit);
	}

	//------------------------------------------------------------------------------------------------
	//! The plan's frame: by a building its yard (ChooseYard), else on open ground (ChooseOpen).
	void ChooseFrame(RZ_CampPlace p, RZ_CampGround ground, RZ_CampFit fit, array<RZ_DressingPiece> plan, int cap, RZ_CampLayout lay)
	{
		if (p.m_Site)
			ChooseYard(p, fit, plan, cap, lay);
		else
			ChooseOpen(p, ground, fit, plan, cap, lay);
	}

	//------------------------------------------------------------------------------------------------
	//! A piece's spot in the plan (its preference): where Slot puts it.
	void PlanSpot(RZ_CampPlace p, RZ_CampLayout lay, RZ_DressingPiece e, out vector pos, out float yaw)
	{
		Slot(p, lay, e, pos, yaw);
	}

	//------------------------------------------------------------------------------------------------
	//! Nothing standing in the footprint's props' boxes (the traces), on the chosen spot.
	bool Traced(vector pos, float yaw, RZ_DressingPiece e)
	{
		return !BlockedFoot(pos, yaw, e.Foot());
	}

	//------------------------------------------------------------------------------------------------
	//! How far a plan reaches back and forward (PlanExtent).
	static void PlanReach(array<RZ_DressingPiece> plan, bool mirror, out float zlo, out float zhi)
	{
		PlanExtent(plan, mirror, zlo, zhi);
	}

	//------------------------------------------------------------------------------------------------
	//! How far a plan reaches left and right (its pieces' footprints, in the plan).
	static void PlanWidth(array<RZ_DressingPiece> plan, bool mirror, out float xlo, out float xhi)
	{
		xlo = 0;
		xhi = 0;
		foreach (RZ_DressingPiece e : plan)
		{
			float x = e.m_vPlan[0];
			float turn = e.m_fPlanYaw;
			if (mirror)
			{
				x = -x;
				turn = -turn;
			}

			array<vector> quad = {};
			Quad(Vector(x, 0, e.m_vPlan[2]), turn, e.m_vFootMin, e.m_vFootMax, 0, quad);
			foreach (vector c : quad)
			{
				xlo = Math.Min(xlo, c[0]);
				xhi = Math.Max(xhi, c[0]);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A piece that looks out from the camp: the perimeter, the gate, the closed entries.
	protected static bool FacesOut(RZ_DressingPiece e)
	{
		RZ_EDressAt at = AtOf(e);
		return e.m_eRole == RZ_EDressRole.APPROACH || at == RZ_EDressAt.GATE || at == RZ_EDressAt.CLOSED;
	}

	//------------------------------------------------------------------------------------------------
	//! The quartermasters at the place: its keepers that are one, and the faction's own if it stands here.
	protected void Quartermasters(RZ_CampPlace p, notnull array<IEntity> result)
	{
		foreach (RZ_CampWorker w : p.m_aWorkers)
		{
			if (w.m_iRole == ROLE_KEEPER && w.m_Unit && w.m_Unit.FindComponent(RZ_QuartermasterComponent))
				result.Insert(w.m_Unit);
		}

		RZ_QuartermasterComponent own = RZ_QuartermasterComponent.Find(p.m_eFaction);
		if (!own || !own.GetOwner())
			return;

		IEntity body = own.GetOwner();
		if (!result.Contains(body) && NearPlace(p, body.GetOrigin()))
			result.Insert(body);
	}

	//------------------------------------------------------------------------------------------------
	//! Turns a quartermaster to serve: the way his customers come from, never into a wall
	//! (RZ_QuartermasterHelper.ServeYaw). Returns that way.
	protected float TurnToServe(RZ_CampPlace p, IEntity qm)
	{
		vector at = qm.GetOrigin();
		float yaw = RZ_QuartermasterHelper.ServeYaw(at, ServeToward(p, at));
		RZ_QuartermasterHelper.Face(qm, yaw);
		return yaw;
	}

	//------------------------------------------------------------------------------------------------
	//! Where a quartermaster's customers come from: by the building, out of it (his back to its
	//! wall, serving into the yard; inside it, toward the front door); else from the yard's middle,
	//! or, standing on it, from the road in.
	protected vector ServeToward(RZ_CampPlace p, vector at)
	{
		if (p.m_Site)
		{
			float off = BoxDistance(p.m_Site, at);
			if (off <= 0)
				return DoorSpot(p);

			if (off < WALL_NEAR)
			{
				vector away = at - p.m_Site.m_vCentre;
				away[1] = 0;
				return at + away;
			}
		}

		if (RZ_Util.DistXZ(at, p.m_vHome) > 3)
			return p.m_vHome;

		return RZ_Compass.PointAt(at, p.m_fApproach, 10);
	}

	//------------------------------------------------------------------------------------------------
	//! How far a point is outside the site's building box (0 inside it).
	protected static float BoxDistance(RZ_CampSite site, vector at)
	{
		vector d = at - site.m_vOrigin;
		float lx = vector.Dot(d, site.m_vRight);
		float lz = vector.Dot(d, site.m_vFwd);
		float ox = Math.Max(0, Math.Max(site.m_vMins[0] - lx, lx - site.m_vMaxs[0]));
		float oz = Math.Max(0, Math.Max(site.m_vMins[2] - lz, lz - site.m_vMaxs[2]));
		return Math.Sqrt(ox * ox + oz * oz);
	}

	//------------------------------------------------------------------------------------------------
	//! A quartermaster's counter: he turns to serve, and the counter stands just in front of him
	//! across that way, if it fits there; indoors (he stands in a building, a kiosk, a shop) only if
	//! there's a floor under it and nothing in its way. His customers' spot is kept clear.
	protected bool Counter(RZ_CampPlace p, RZ_CampFit fit, RZ_DressingPiece entry, IEntity qm, RZ_SiteResult result, bool audit)
	{
		vector at = qm.GetOrigin();
		float yaw;
		if (audit)
			yaw = RZ_QuartermasterHelper.ServeYaw(at, ServeToward(p, at));
		else
			yaw = TurnToServe(p, qm);

		vector pos = FramePoint(at, yaw, 0, COUNTER_AHEAD);
		if (IsIndoors(fit, at))
		{
			pos[1] = at[1];
			if (!FitsIndoors(pos, yaw, entry.m_vFootMin, entry.m_vFootMax))
			{
				Dropped(result, entry, pos, yaw, "no room indoors");
				return false;
			}

			// Indoors: the grid's building cells don't apply
			RZ_SiteGrid grid = fit.m_Grid;
			fit.m_Grid = null;
			AddPiece(p, fit, entry, pos, yaw, Spawns(audit));
			fit.m_Grid = grid;
			if (grid)
				grid.Paint(pos, yaw, entry.Foot(), RZ_SiteGrid.TAKEN);
		}
		else
		{
			pos = RZ_Util.GroundPos(pos);
			fit.m_vSkip = at;
			fit.m_bSkip = true;
			bool fits = Fits(fit, pos, yaw, entry, true);
			fit.m_bSkip = false;
			if (!fits)
			{
				Dropped(result, entry, pos, yaw, fit.m_sWhy);
				return false;
			}

			AddPiece(p, fit, entry, pos, yaw, Spawns(audit));
		}

		Placed(result, entry, pos, yaw);
		result.m_iCounters++;
		fit.m_aKeep.Insert(FramePoint(at, yaw, 0, CUSTOMER_AHEAD));
		fit.m_aKeepR.Insert(1.2);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Standing in a building: above the ground, or in a building's box.
	protected static bool IsIndoors(RZ_CampFit fit, vector at)
	{
		if (at[1] - GetGame().GetWorld().GetSurfaceY(at[0], at[2]) > 0.4)
			return true;

		for (int i = 0; i + 3 < fit.m_aInside.Count(); i += 4)
		{
			if (InQuad(at, fit.m_aInside, i))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A laid-out piece, spawned later (SpawnPending) if `spawn`; its footprint is taken.
	protected void AddPiece(RZ_CampPlace p, RZ_CampFit fit, RZ_DressingPiece entry, vector pos, float yaw, bool spawn, float slope = 0)
	{
		Quad(pos, yaw, entry.m_vFootMin, entry.m_vFootMax, PIECE_GAP * 0.5, fit.m_aTaken);
		if (fit.m_Grid)
			fit.m_Grid.Paint(pos, yaw, entry.Foot(), RZ_SiteGrid.TAKEN);

		if (!spawn)
			return;

		QueuePiece(p, entry, pos, yaw, slope);
	}

	//------------------------------------------------------------------------------------------------
	//! A piece waiting to spawn (SpawnPending), if the server's piece cap allows.
	protected void QueuePiece(RZ_CampPlace p, RZ_DressingPiece entry, vector pos, float yaw, float slope = 0)
	{
		if (CountPieces() >= m_Settings.m_iPiecesMax)
			return;

		RZ_CampPiece cp = new RZ_CampPiece();
		cp.m_sPending = entry.m_sPrefab;
		cp.m_fYaw = yaw;
		cp.m_vPos = pos;
		cp.m_fSlope = slope;
		cp.m_eRole = entry.m_eRole;
		cp.m_eUse = entry.m_eUse;
		cp.m_iSeats = Math.Max(1, entry.m_iSeats);
		cp.m_sName = RZ_SiteAudit.PieceName(entry);
		cp.m_sTag = entry.m_sTag;
		if (cp.m_sTag.IsEmpty())
			cp.m_sTag = UseTag(entry.m_eUse);

		p.m_aPieces.Insert(cp);
	}

	//------------------------------------------------------------------------------------------------
	//! Where a plan entry goes, by a building or not: its slot and its turn. It stands there or not
	//! at all (docs/SITES.md, "Drop, don't squeeze").
	protected void Slot(RZ_CampPlace p, RZ_CampLayout lay, RZ_DressingPiece e, out vector pos, out float yaw)
	{
		RZ_EDressAt at = AtOf(e);
		if (!p.m_Site || at == RZ_EDressAt.YARD)
		{
			PlanPose(e, lay.m_vOrigin, lay.m_fBearing, lay.m_bMirror, pos, yaw);
			return;
		}

		RZ_CampSite site = p.m_Site;
		vector wall = FacePoint(site, FACE_FRONT, 0, 0);
		float outB = RZ_Compass.Bearing(wall, FacePoint(site, FACE_FRONT, 0, 10));
		float halfFront = RZ_Util.DistXZ(FacePoint(site, FACE_FRONT, -1, 0), FacePoint(site, FACE_FRONT, 1, 0)) * 0.5;
		float x;
		float z;

		// Along the front wall, its back to it and facing out: one each side of the middle in turn
		if (at == RZ_EDressAt.FRONT)
		{
			int f = p.m_iFrontNext;
			p.m_iFrontNext++;
			x = halfFront * 0.55;
			if (f % 2 == 1)
				x = -x;

			z = 2.5 - e.m_vFootMin[2];
			pos = RZ_Util.GroundPos(FramePoint(wall, outB, x, z));
			yaw = outB;
			return;
		}

		// Beside the front door, facing out: right, left, then further along
		if (at == RZ_EDressAt.DOOR)
		{
			int d = p.m_iDoorNext;
			p.m_iDoorNext++;
			vector door = FacePoint(site, FACE_FRONT, site.m_fDoor, 0);
			float reach = 1.6 + (d / 2) * 3;
			x = reach - e.m_vFootMin[0];
			if (d % 2 == 1)
				x = -(reach + e.m_vFootMax[0]);

			z = 1.2 - e.m_vFootMin[2];
			pos = RZ_Util.GroundPos(FramePoint(door, outB, x, z));
			yaw = outB;
			return;
		}

		// The gate or a closed entry: beside the road where it crosses the ring, facing out along it
		int entry = site.m_iMain;
		int turn = p.m_iGateNext;
		if (at == RZ_EDressAt.CLOSED)
		{
			int closed = site.m_aEntries.Count() - 1;
			entry = -1;
			turn = p.m_iClosedNext;
			if (closed > 0)
			{
				int pick = RZ_Util.PickIndex(p.m_iClosedNext, closed);
				entry = pick;
				if (pick >= site.m_iMain)
					entry = pick + 1;
			}

			p.m_iClosedNext++;
		}
		else
		{
			p.m_iGateNext++;
		}

		float side = 1;
		RZ_Rng sideRng = new RZ_Rng(p.m_iSeed + 29);
		if (sideRng.Chance(0.5))
			side = -1;

		if (turn % 2 == 1)
			side = -side;

		// No road in: out in front of the building instead, or off its first side that faces land
		if (entry < 0)
		{
			array<int> sides = {FACE_FRONT, FACE_LEFT, FACE_RIGHT, FACE_BACK};
			foreach (int face : sides)
			{
				vector faceWall = FacePoint(site, face, 0, 0);
				float faceOut = RZ_Compass.Bearing(faceWall, FacePoint(site, face, 0, 10));
				if (BadOutlook(faceWall, faceOut, OUTLOOK_REACH))
					continue;

				wall = faceWall;
				outB = faceOut;
				halfFront = RZ_Util.DistXZ(FacePoint(site, face, -1, 0), FacePoint(site, face, 1, 0)) * 0.5;
				break;
			}

			x = side * (halfFront * 0.6 + 2);
			z = 9 - e.m_vFootMin[2] + (turn / 2) * 4;
			pos = RZ_Util.GroundPos(FramePoint(wall, outB, x, z));
			yaw = outB;
			return;
		}

		vector gate = site.m_aEntries[entry];
		float roadOut = site.m_aEntryRoad[entry];
		float toward = RZ_Compass.Bearing(site.m_vCentre, gate);
		if (Math.AbsFloat(RZ_Compass.Normalize(roadOut - toward + 180) - 180) > 90)
			roadOut = RZ_Compass.Normalize(roadOut + 180);

		x = side * ENTRY_SIDE - e.m_vFootMin[0];
		if (side < 0)
			x = side * ENTRY_SIDE - e.m_vFootMax[0];

		z = -(turn / 2) * 4;
		pos = RZ_Util.GroundPos(FramePoint(gate, roadOut, x, z));
		yaw = roadOut;
	}

	//------------------------------------------------------------------------------------------------
	//! Where a piece stands by a building: its own setting, else an approach piece at the road
	//! entries (a watch or lean piece the gate) and anything else in the yard.
	protected static RZ_EDressAt AtOf(RZ_DressingPiece e)
	{
		if (e.m_eAt != RZ_EDressAt.AUTO)
			return e.m_eAt;

		if (e.m_eRole == RZ_EDressRole.APPROACH)
		{
			if (e.m_eUse == RZ_EDressUse.WATCH || e.m_eUse == RZ_EDressUse.LEAN)
				return RZ_EDressAt.GATE;

			return RZ_EDressAt.CLOSED;
		}

		return RZ_EDressAt.YARD;
	}

	//------------------------------------------------------------------------------------------------
	//! A plan entry laid down in a frame: its spot on the ground and its turn.
	protected static void PlanPose(RZ_DressingPiece e, vector origin, float bearing, bool mirror, out vector pos, out float yaw)
	{
		float x = e.m_vPlan[0];
		float turn = e.m_fPlanYaw;
		if (mirror)
		{
			x = -x;
			turn = -turn;
		}

		pos = RZ_Util.GroundPos(FramePoint(origin, bearing, x, e.m_vPlan[2]));
		yaw = RZ_Compass.Normalize(bearing + turn);
	}

	//------------------------------------------------------------------------------------------------
	//! On open ground (or at a road post): the plan's middle at the place, its front to the road
	//! (a road post's along the road), then a few shifts and turns of the whole plan; the one where
	//! most of it fits wins (its first pieces count most).
	protected void ChooseOpen(RZ_CampPlace p, RZ_CampGround ground, RZ_CampFit fit, array<RZ_DressingPiece> plan, int cap, RZ_CampLayout lay)
	{
		RZ_Rng rng = new RZ_Rng(p.m_iSeed + 17);
		lay.m_bMirror = rng.Chance(0.5);
		float bearing = p.m_fApproach;
		lay.m_sWhere = "on open ground, its front to the road";
		// (sideways, forward, turn): pulled back from the road, sideways, turned a little; further
		// (16 m) where the ground near is too steep
		array<float> openMoves = {0.0, 0.0, 0.0, -4.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, -4.0, 0.0, 0.0, -8.0, 0.0, 0.0, 4.0, 0.0, 0.0, 8.0, 0.0, 0.0, 0.0, 12.0, 0.0, 0.0, -12.0,
			-16.0, 0.0, 0.0, 16.0, 0.0, 0.0, 0.0, -16.0, 0.0, -12.0, -12.0, 0.0, 12.0, -12.0, 0.0};
		array<float> postMoves = {0.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, -3.0, 0.0};
		array<float> moves = openMoves;
		if (p.m_eKind == RZ_ECampPlace.ROAD_POST)
		{
			bearing = RoadAxis(p, ground);
			lay.m_sWhere = "beside the road";
			moves = postMoves;
		}

		// From dry ground: a place whose spot is over water (a pier, a quay) camps on the nearest land
		vector base = p.m_vPos;
		if (p.m_eKind != RZ_ECampPlace.ROAD_POST)
			base = DryNear(p.m_vPos);

		lay.m_vOrigin = base;
		lay.m_fBearing = bearing;
		float best = -1;
		for (int i = 0; i + 2 < moves.Count(); i += 3)
		{
			float b = RZ_Compass.Normalize(bearing + moves[i + 2]);
			vector origin = RZ_Util.GroundPos(FramePoint(base, bearing, moves[i], moves[i + 1]));
			float score = PlanScore(fit, plan, cap, origin, b, lay.m_bMirror);
			if (score > best)
			{
				best = score;
				lay.m_vOrigin = origin;
				lay.m_fBearing = b;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! By a building: the yard's part of the plan laid against the faction's side of the building
	//! (behind it; the Red Hand in front), the plan's front to the wall YARD_GAP off it (the Red
	//! Hand's back), shifted out and sideways a little where more of it fits. If most of it doesn't
	//! fit there, the other sides are tried too.
	protected void ChooseYard(RZ_CampPlace p, RZ_CampFit fit, array<RZ_DressingPiece> plan, int cap, RZ_CampLayout lay)
	{
		RZ_CampSite site = p.m_Site;
		RZ_Rng rng = new RZ_Rng(p.m_iSeed + 17);
		lay.m_bMirror = rng.Chance(0.5);

		array<RZ_DressingPiece> yard = {};
		foreach (RZ_DressingPiece e : plan)
		{
			if (AtOf(e) == RZ_EDressAt.YARD)
				yard.Insert(e);
		}

		float zlo;
		float zhi;
		PlanExtent(yard, lay.m_bMirror, zlo, zhi);
		float whole = PlanScore(null, yard, cap, vector.Zero, 0, false);

		array<int> faces = {site.m_iYardFace};
		array<int> others = {FACE_BACK, FACE_LEFT, FACE_RIGHT, FACE_FRONT};
		foreach (int o : others)
		{
			if (!faces.Contains(o))
				faces.Insert(o);
		}

		array<float> moves = {0.0, 0.0, 0.0, -4.0, 0.0, 4.0, 2.5, 0.0, 5.0, 0.0, 2.5, -4.0, 2.5, 4.0};
		float best = -1;
		foreach (int fi, int face : faces)
		{
			if (fi > 0 && best >= whole * 0.6)
				break;

			bool forecourt = face == FACE_FRONT && site.m_iYardFace == FACE_FRONT;
			vector wall = FacePoint(site, face, 0, 0);
			float outB = RZ_Compass.Bearing(wall, FacePoint(site, face, 0, 10));
			float planB = RZ_Compass.Normalize(outB + 180);
			float edge = zhi + YARD_GAP;
			if (forecourt)
			{
				planB = outB;
				edge = YARD_GAP - zlo;
			}

			for (int i = 0; i + 1 < moves.Count(); i += 2)
			{
				vector origin = FramePoint(wall, outB, 0, edge + moves[i]);
				origin = RZ_Util.GroundPos(FramePoint(origin, planB, moves[i + 1], 0));
				float score = PlanScore(fit, yard, cap, origin, planB, lay.m_bMirror);
				if (score > best)
				{
					best = score;
					lay.m_vOrigin = origin;
					lay.m_fBearing = planB;
					lay.m_sWhere = YardWhere(face, forecourt, site);
					lay.m_iFace = face;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static string YardWhere(int face, bool forecourt, RZ_CampSite site)
	{
		if (forecourt)
			return "in front of " + site.m_sWhat;

		if (face == FACE_BACK)
			return "in the yard behind " + site.m_sWhat;

		if (face == FACE_FRONT)
			return "between " + site.m_sWhat + " and the road";

		return "beside " + site.m_sWhat;
	}

	//------------------------------------------------------------------------------------------------
	//! How far a plan reaches back and forward (its pieces' footprints, in the plan).
	protected static void PlanExtent(array<RZ_DressingPiece> plan, bool mirror, out float zlo, out float zhi)
	{
		zlo = 0;
		zhi = 0;
		foreach (RZ_DressingPiece e : plan)
		{
			float turn = e.m_fPlanYaw;
			if (mirror)
				turn = -turn;

			array<vector> quad = {};
			Quad(Vector(0, 0, e.m_vPlan[2]), turn, e.m_vFootMin, e.m_vFootMax, 0, quad);
			foreach (vector c : quad)
			{
				zlo = Math.Min(zlo, c[2]);
				zhi = Math.Max(zhi, c[2]);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! How much of a plan fits laid down this way, its first pieces counting most (no traces: a
	//! quick look). With no fit given, what the whole plan would score.
	protected float PlanScore(RZ_CampFit fit, array<RZ_DressingPiece> plan, int cap, vector origin, float bearing, bool mirror)
	{
		float score = 0;
		foreach (int i, RZ_DressingPiece e : plan)
		{
			if (i >= cap)
				break;

			float weight = 1;
			if (i < 4)
				weight = 3;
			else if (i < 10)
				weight = 2;

			if (!fit)
			{
				score += weight;
				continue;
			}

			vector pos;
			float yaw;
			PlanPose(e, origin, bearing, mirror, pos, yaw);
			if (Fits(fit, pos, yaw, e, false))
				score += weight;
		}

		return score;
	}

	//------------------------------------------------------------------------------------------------
	//! A road post's road: the nearest segment's way, turned to the post's own yaw. The post's yaw
	//! if no road is that near.
	protected static float RoadAxis(RZ_CampPlace p, RZ_CampGround ground)
	{
		float best = float.MAX;
		float axis = p.m_fYaw;
		for (int i = 0; i + 1 < ground.m_aRoad.Count(); i += 2)
		{
			float d = RZ_DressPlacer.DistToSegment(p.m_vPos, ground.m_aRoad[i], ground.m_aRoad[i + 1]);
			if (d < best)
			{
				best = d;
				axis = RZ_Compass.Bearing(ground.m_aRoad[i], ground.m_aRoad[i + 1]);
			}
		}

		if (best > 15)
			return p.m_fYaw;

		if (Math.AbsFloat(RZ_Compass.Normalize(axis - p.m_fYaw + 180) - 180) > 90)
			axis = RZ_Compass.Normalize(axis + 180);

		return axis;
	}

	// ================================================================ Fitting a piece
	//------------------------------------------------------------------------------------------------
	//! The buildings' boxes, the roads, the spots kept clear and other scenery near the place.
	protected RZ_CampFit ReadFit(RZ_CampPlace p, RZ_CampGround ground)
	{
		RZ_CampFit fit = new RZ_CampFit();
		vector c = p.m_vCentre;
		foreach (IEntity b : ground.m_aBuildings)
		{
			if (b && !IsDressing(b) && RZ_Util.DistXZ(b.GetOrigin(), c) <= FIT_REACH)
				AddBox(fit, b);
		}

		for (int i = 0; i + 1 < ground.m_aRoad.Count(); i += 2)
		{
			if (RZ_DressPlacer.DistToSegment(c, ground.m_aRoad[i], ground.m_aRoad[i + 1]) <= FIT_REACH)
			{
				fit.m_aRoad.Insert(ground.m_aRoad[i]);
				fit.m_aRoad.Insert(ground.m_aRoad[i + 1]);
			}
		}

		foreach (vector keep : p.m_aKeepClear)
		{
			fit.m_aKeep.Insert(keep);
			fit.m_aKeepR.Insert(KEEP_EDGE);
		}

		foreach (vector seat : p.m_aLeaderSeats)
		{
			fit.m_aKeep.Insert(seat);
			fit.m_aKeepR.Insert(LEADER_EDGE);
		}

		if (p.m_Site)
		{
			fit.m_aKeep.Insert(DoorSpot(p));
			fit.m_aKeepR.Insert(DOOR_EDGE);
		}

		foreach (IEntity e : s_aPieces)
		{
			if (e && RZ_Util.DistXZ(e.GetOrigin(), c) <= FIT_REACH)
				fit.m_aOthers.Insert(e.GetOrigin());
		}

		return fit;
	}

	//------------------------------------------------------------------------------------------------
	//! A building's box, as it is and grown by BUILDING_EDGE; one too small to tell, a square round its origin.
	protected static void AddBox(RZ_CampFit fit, IEntity b)
	{
		vector mins;
		vector maxs;
		if (!RZ_DoorGeometry.LocalBox(b, mins, maxs))
		{
			mins = Vector(-NO_BOX, 0, -NO_BOX);
			maxs = Vector(NO_BOX, 0, NO_BOX);
		}

		for (int g = 0; g < 2; g++)
		{
			float grow = g * BUILDING_EDGE;
			array<vector> into = fit.m_aInside;
			if (g == 1)
				into = fit.m_aBoxes;

			into.Insert(RZ_DoorGeometry.ToWorld(b, Vector(mins[0] - grow, 0, mins[2] - grow)));
			into.Insert(RZ_DoorGeometry.ToWorld(b, Vector(maxs[0] + grow, 0, mins[2] - grow)));
			into.Insert(RZ_DoorGeometry.ToWorld(b, Vector(maxs[0] + grow, 0, maxs[2] + grow)));
			into.Insert(RZ_DoorGeometry.ToWorld(b, Vector(mins[0] - grow, 0, maxs[2] + grow)));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Whether a piece's whole footprint fits here: dry, off the line and flat across it, every
	//! point of it off the roads, out of every building and clear of the spots kept free and of
	//! other scenery, not on another piece; traced, nothing standing in it (a wall, a fence, a
	//! tree, a car).
	protected bool Fits(RZ_CampFit fit, vector pos, float yaw, RZ_DressingPiece entry, bool traced)
	{
		if (fit.m_Grid)
			return FitsGrid(fit, pos, yaw, entry, traced);

		vector fmin = entry.m_vFootMin;
		vector fmax = entry.m_vFootMax;
		fit.m_sWhy = "no room";
		array<vector> quad = {};
		Quad(pos, yaw, fmin, fmax, 0, quad);
		vector mid = FramePoint(pos, yaw, (fmin[0] + fmax[0]) * 0.5, (fmin[2] + fmax[2]) * 0.5);
		array<vector> pts = {mid};
		pts.InsertAll(quad);
		for (int k = 0; k < 4; k++)
		{
			pts.Insert((quad[k] + quad[(k + 1) % 4]) * 0.5);
		}

		// Dry, off the line, flat across the whole footprint
		BaseWorld world = GetGame().GetWorld();
		float lo = float.MAX;
		float hi = -float.MAX;
		foreach (vector pt : pts)
		{
			float h = world.GetSurfaceY(pt[0], pt[2]);
			if (h < 0.5)
				return false;

			lo = Math.Min(lo, h);
			hi = Math.Max(hi, h);
		}

		float diag = RZ_Util.DistXZ(quad[0], quad[2]);
		if (hi - lo > FLAT_SLOPE * diag + FLAT_SLACK)
			return false;

		if (RZ_Perimeter.IsNearLineAt(RZ_Util.GroundPos(mid)))
			return false;

		// Every point off the roads
		float reach = diag * 0.5 + ROAD_EDGE;
		for (int i = 0; i + 1 < fit.m_aRoad.Count(); i += 2)
		{
			vector a = fit.m_aRoad[i];
			vector b = fit.m_aRoad[i + 1];
			if (RZ_DressPlacer.DistToSegment(mid, a, b) > reach)
				continue;

			foreach (vector rp : pts)
			{
				if (RZ_DressPlacer.DistToSegment(rp, a, b) < ROAD_EDGE)
					return false;
			}
		}

		// Out of every building, its whole footprint
		for (int j = 0; j + 3 < fit.m_aBoxes.Count(); j += 4)
		{
			if (QuadsOverlap(quad, 0, fit.m_aBoxes, j))
				return false;
		}

		// Clear of the spots kept free (a quartermaster's counter may stand by him) and of other scenery
		foreach (int n, vector keep : fit.m_aKeep)
		{
			if (fit.m_bSkip && RZ_Util.DistXZ(keep, fit.m_vSkip) < 1.5)
				continue;

			if (QuadDistance(keep, quad, 0) < fit.m_aKeepR[n])
				return false;
		}

		foreach (vector other : fit.m_aOthers)
		{
			if (QuadDistance(other, quad, 0) < OTHER_EDGE)
				return false;
		}

		// Not on another piece
		array<vector> grown = {};
		Quad(pos, yaw, fmin, fmax, PIECE_GAP * 0.5, grown);
		for (int t = 0; t + 3 < fit.m_aTaken.Count(); t += 4)
		{
			if (QuadsOverlap(grown, 0, fit.m_aTaken, t))
				return false;
		}

		if (!traced)
			return true;

		return !Blocked(quad);
	}

	//------------------------------------------------------------------------------------------------
	//! On the site grid (docs/SITES.md): every cell under every part of the footprint is free (dry,
	//! not steep or a bank, no building, nothing solid, off the road, no other scenery or piece), the
	//! ground flat enough across it, off the line, clear of the spots kept free; traced, nothing
	//! standing in any of its props' boxes (the last check, on the chosen spot only).
	protected bool FitsGrid(RZ_CampFit fit, vector pos, float yaw, RZ_DressingPiece entry, bool traced)
	{
		RZ_SiteFoot foot = entry.Foot();
		float lo;
		float hi;
		int blocks = fit.m_Grid.Blocks(pos, yaw, foot, RZ_SiteGrid.BLOCKS, lo, hi);
		if (blocks != 0)
		{
			fit.m_sWhy = RZ_SiteGrid.FlagName(blocks);
			return false;
		}

		array<vector> quad = {};
		Quad(pos, yaw, entry.m_vFootMin, entry.m_vFootMax, 0, quad);
		float diag = RZ_Util.DistXZ(quad[0], quad[2]);
		if (hi - lo > FLAT_SLOPE * diag + FLAT_SLACK)
		{
			fit.m_sWhy = "not flat";
			return false;
		}

		vector mid = FramePoint(pos, yaw, (entry.m_vFootMin[0] + entry.m_vFootMax[0]) * 0.5, (entry.m_vFootMin[2] + entry.m_vFootMax[2]) * 0.5);
		if (RZ_Perimeter.IsNearLineAt(RZ_Util.GroundPos(mid)))
		{
			fit.m_sWhy = "the line";
			return false;
		}

		foreach (int n, vector keep : fit.m_aKeep)
		{
			if (fit.m_bSkip && RZ_Util.DistXZ(keep, fit.m_vSkip) < 1.5)
				continue;

			if (QuadDistance(keep, quad, 0) < fit.m_aKeepR[n])
			{
				fit.m_sWhy = "a spot kept clear";
				return false;
			}
		}

		if (traced && BlockedFoot(pos, yaw, foot))
		{
			fit.m_sWhy = "something in the way";
			return false;
		}

		fit.m_sWhy = "";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! One measured box fits at `pos` (a work site's look or person, docs/DESIGN.md "Work site"): the
	//! same checks the camps' pieces get. Indoors: a floor under every corner and nothing in the way
	//! (FitsIndoors). Outdoors: nothing in it at knee or chest height (Blocked), nothing overhead,
	//! and the ground under its corners within LEVEL_STEP of each other.
	static bool BoxFits(vector pos, float yaw, vector fmin, vector fmax, bool indoors)
	{
		if (indoors)
			return FitsIndoors(pos, yaw, fmin, fmax);

		array<vector> quad = {};
		Quad(pos, yaw, fmin, fmax, 0, quad);
		float low = float.MAX;
		float high = -float.MAX;
		foreach (vector c : quad)
		{
			float y = RZ_Util.GroundPos(c)[1];
			low = Math.Min(low, y);
			high = Math.Max(high, y);
		}

		if (high - low > LEVEL_STEP)
			return false;

		vector mid = RZ_Util.GroundPos(pos);
		if (TraceHits(mid + Vector(0, 0.3, 0), mid + Vector(0, 4, 0), false))
			return false;

		return !Blocked(quad);
	}

	protected static const float LEVEL_STEP = 0.4;	// BoxFits outdoors: most a box's corners may differ in height

	//------------------------------------------------------------------------------------------------
	//! Something stands in one of the footprint's props' boxes: flat traces across both its
	//! diagonals at knee and chest height (boxes under 0.3 m either way are left to the grid).
	protected static bool BlockedFoot(vector pos, float yaw, RZ_SiteFoot foot)
	{
		for (int r = 0; r < foot.m_iProps; r++)
		{
			int i = r * 5;
			if (foot.m_aRects[i + 2] < 0.3 || foot.m_aRects[i + 3] < 0.3)
				continue;

			array<vector> box = {};
			foot.Corners(r, pos, yaw, -0.1, box);
			array<vector> ground = {};
			foreach (vector c : box)
			{
				ground.Insert(RZ_Util.GroundPos(c));
			}

			for (int h = 0; h < 2; h++)
			{
				vector up = Vector(0, 0.5 + h * 0.7, 0);
				if (TraceHits(ground[0] + up, ground[2] + up, false) || TraceHits(ground[1] + up, ground[3] + up, false))
					return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Something stands in the footprint: a wall, a fence, a tree, a car, another system's prop.
	//! Flat traces along its four sides and across both diagonals, at knee and chest height.
	static bool Blocked(array<vector> quad)
	{
		array<vector> ground = {};
		foreach (vector c : quad)
		{
			ground.Insert(RZ_Util.GroundPos(c));
		}

		for (int h = 0; h < 2; h++)
		{
			vector up = Vector(0, 0.5 + h * 0.7, 0);
			for (int i = 0; i < 4; i++)
			{
				if (TraceHits(ground[i] + up, ground[(i + 1) % 4] + up, false))
					return true;
			}

			if (TraceHits(ground[0] + up, ground[2] + up, false) || TraceHits(ground[1] + up, ground[3] + up, false))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Indoors (a quartermaster's counter): a floor under every corner and nothing in the way at the
	//! counter's height or above it (walls, furniture), traced along its sides and across it.
	static bool FitsIndoors(vector pos, float yaw, vector fmin, vector fmax)
	{
		array<vector> quad = {};
		Quad(pos, yaw, fmin, fmax, 0, quad);
		foreach (vector c : quad)
		{
			if (!TraceHits(c + Vector(0, 0.8, 0), c + Vector(0, -0.4, 0), true))
				return false;
		}

		for (int h = 0; h < 2; h++)
		{
			vector up = Vector(0, 0.5 + h * 0.6, 0);
			for (int i = 0; i < 4; i++)
			{
				if (TraceHits(quad[i] + up, quad[(i + 1) % 4] + up, true))
					return false;
			}

			if (TraceHits(quad[0] + up, quad[2] + up, true) || TraceHits(quad[1] + up, quad[3] + up, true))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A physics trace hits something (entities; with `world` the terrain too). Characters don't count.
	static bool TraceHits(vector from, vector to, bool world)
	{
		TraceParam param = new TraceParam();
		param.Start = from;
		param.End = to;
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		param.Flags = TraceFlags.ENTS;
		if (world)
			param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;

		float frac = ChimeraCharacter.TraceMoveWithoutCharacters(GetGame().GetWorld(), param); // VERIFY: a flat trace (entities only) between a footprint's corners at knee and chest height stops on walls, fences, trees, cars and props, and passes over open ground, grass and small bushes
		return frac < 0.999;
	}

	//------------------------------------------------------------------------------------------------
	//! A point `x` right and `z` ahead of `origin` facing `bearing` (degrees clockwise from north);
	//! its height is origin's.
	protected static vector FramePoint(vector origin, float bearing, float x, float z)
	{
		float rad = bearing * Math.DEG2RAD;
		float s = Math.Sin(rad);
		float c = Math.Cos(rad);
		vector pt = origin;
		pt[0] = origin[0] + x * c + z * s;
		pt[2] = origin[2] - x * s + z * c;
		return pt;
	}

	//------------------------------------------------------------------------------------------------
	//! A footprint's four corners (grown by `grow`), added to `quad`.
	static void Quad(vector pos, float yaw, vector fmin, vector fmax, float grow, notnull array<vector> quad)
	{
		quad.Insert(FramePoint(pos, yaw, fmin[0] - grow, fmin[2] - grow));
		quad.Insert(FramePoint(pos, yaw, fmax[0] + grow, fmin[2] - grow));
		quad.Insert(FramePoint(pos, yaw, fmax[0] + grow, fmax[2] + grow));
		quad.Insert(FramePoint(pos, yaw, fmin[0] - grow, fmax[2] + grow));
	}

	//------------------------------------------------------------------------------------------------
	//! Two rectangles (four corners each, from `ai` and `bi`) overlap on the ground.
	static bool QuadsOverlap(notnull array<vector> a, int ai, notnull array<vector> b, int bi)
	{
		return !Separated(a, ai, b, bi) && !Separated(b, bi, a, ai);
	}

	//------------------------------------------------------------------------------------------------
	//! One of rectangle a's sides separates it from rectangle b.
	protected static bool Separated(notnull array<vector> a, int ai, notnull array<vector> b, int bi)
	{
		for (int e = 0; e < 2; e++)
		{
			vector p0 = a[ai + e];
			vector p1 = a[ai + e + 1];
			float nx = p0[2] - p1[2];
			float nz = p1[0] - p0[0];
			float aLo = float.MAX;
			float aHi = -float.MAX;
			float bLo = float.MAX;
			float bHi = -float.MAX;
			for (int k = 0; k < 4; k++)
			{
				vector qa = a[ai + k];
				vector qb = b[bi + k];
				float da = qa[0] * nx + qa[2] * nz;
				float db = qb[0] * nx + qb[2] * nz;
				aLo = Math.Min(aLo, da);
				aHi = Math.Max(aHi, da);
				bLo = Math.Min(bLo, db);
				bHi = Math.Max(bHi, db);
			}

			if (aHi < bLo || bHi < aLo)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! How far a point is from a rectangle (0 inside it).
	protected static float QuadDistance(vector pt, notnull array<vector> q, int qi)
	{
		if (InQuad(pt, q, qi))
			return 0;

		float best = float.MAX;
		for (int k = 0; k < 4; k++)
		{
			best = Math.Min(best, RZ_DressPlacer.DistToSegment(pt, q[qi + k], q[qi + (k + 1) % 4]));
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool InQuad(vector pt, notnull array<vector> q, int qi)
	{
		int pos = 0;
		int neg = 0;
		for (int k = 0; k < 4; k++)
		{
			vector a = q[qi + k];
			vector b = q[qi + (k + 1) % 4];
			float cross = (b[0] - a[0]) * (pt[2] - a[2]) - (b[2] - a[2]) * (pt[0] - a[0]);
			if (cross > 0)
				pos++;
			else if (cross < 0)
				neg++;
		}

		return pos == 0 || neg == 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Laid-out pieces spawn: at most maxPieces, and (budgetMs over 0) while this call has spent under
	//! budgetMs of server time. The place nearest a player goes first; within a place, what its people
	//! use (seats, work, watch, cook...) and a hand-built site's objects and role spots, then the rest
	//! of its scenery, then the layers, the slopes' barrier runs and its story. A place whose last
	//! piece is up hands out its jobs again, so people use the pieces that weren't there yet.
	protected void SpawnPending(int maxPieces, float budgetMs, array<IEntity> players)
	{
		array<int> order = {};
		array<float> dists = {};
		foreach (int i, RZ_CampPlace p : m_aPlaces)
		{
			if (PendingCount(p) == 0)
				continue;

			float d = RZ_Util.NearestDistance(p.m_vPos, players);
			int at = 0;
			while (at < dists.Count() && dists[at] <= d)
			{
				at++;
			}

			dists.InsertAt(d, at);
			order.InsertAt(i, at);
		}

		if (order.IsEmpty())
			return;

		float start = System.GetTickCount();
		int left = maxPieces;
		foreach (int idx : order)
		{
			RZ_CampPlace q = m_aPlaces[idx];
			float placeStart = System.GetTickCount();
			bool spawned = false;
			bool waiting = false;
			for (int rank = 0; rank < 3 && !waiting; rank++)
			{
				foreach (RZ_CampPiece cp : q.m_aPieces)
				{
					if (cp.m_sPending.IsEmpty() || SpawnRank(cp) != rank)
						continue;

					if (left <= 0 || (budgetMs > 0 && System.GetTickCount() - start >= budgetMs))
					{
						waiting = true;
						break;
					}

					left--;
					spawned = true;
					q.m_iSpawnedCount++;
					SpawnPiece(cp);
				}
			}

			float cost = System.GetTickCount() - placeStart;
			q.m_fSpawnCost += cost;
			q.m_fSpawnLongest = Math.Max(q.m_fSpawnLongest, cost);
			if (waiting)
				return;

			if (!spawned)
				continue;

			if (q.m_bLive)
				q.m_iPhase = -2;

			LogTiming(q, players);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The order a place's pieces come in: 0 what people use and a hand-built site, 1 the rest of the
	//! scenery, 2 the layers (the slopes' barrier runs, the approach and the far side) and the story.
	protected static int SpawnRank(RZ_CampPiece cp)
	{
		if (cp.m_bHandObj || cp.m_eUse != RZ_EDressUse.NONE)
			return 0;

		if (cp.m_bStory || cp.m_fSlope != 0 || cp.m_eRole == RZ_EDressRole.APPROACH || cp.m_eRole == RZ_EDressRole.EDGE)
			return 2;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	protected static int PendingCount(RZ_CampPlace p)
	{
		int n = 0;
		foreach (RZ_CampPiece cp : p.m_aPieces)
		{
			if (!cp.m_sPending.IsEmpty())
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! One piece out (it's tried once: a failed spawn isn't tried again).
	protected void SpawnPiece(RZ_CampPiece cp)
	{
		ResourceName prefab = cp.m_sPending;
		cp.m_sPending = "";
		if (cp.m_bHandObj)
		{
			RZ_HandBuiltObject ho = new RZ_HandBuiltObject();
			ho.m_sPrefab = prefab;
			ho.m_vPos = cp.m_vPos;
			ho.m_vAngles = cp.m_vAngles;
			ho.m_fScale = cp.m_fScale;
			IEntity built = RZ_HandBuilt.Spawn(ho);
			if (!built || !RZ_DressingGuard.Strip(built, prefab))
				return;

			cp.m_Entity = built;
			s_aPieces.Insert(built);
			return;
		}

		IEntity e = SpawnTilted(prefab, cp.m_vPos, cp.m_fYaw, cp.m_fSlope); // VERIFY: a spawned composition (root with an RplComponent) shows on clients, and props spawned at run time block AI paths (else AI walk through tents)
		if (!e || !RZ_DressingGuard.Strip(e, prefab))
			return;

		cp.m_Entity = e;
		s_aPieces.Insert(e);
	}

	//------------------------------------------------------------------------------------------------
	//! Once a place's last piece is up: how long it took from the moment it came (docs/SITES.md, How
	//! fast a place fills).
	protected void LogTiming(RZ_CampPlace q, array<IEntity> players)
	{
		if (q.m_bTimed)
			return;

		q.m_bTimed = true;
		RZ_Log.Info(string.Format("Spawn timing: %1 %2 pieces in %3 ms, player at %4 m (now %5 m); %6 ms of server time in all, the longest tenth %7 ms.",
			q.m_sTitle, q.m_iSpawnedCount, RZ_Util.RoundToInt(System.GetTickCount() - q.m_fEnterMs), RZ_Util.RoundToInt(q.m_fEnterDist),
			RZ_Util.RoundToInt(RZ_Util.NearestDistance(q.m_vPos, players)), RZ_Util.RoundToInt(q.m_fSpawnCost), RZ_Util.RoundToInt(q.m_fSpawnLongest)));
	}

	//------------------------------------------------------------------------------------------------
	protected static float NearestPlayer(vector pos)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		return RZ_Util.NearestDistance(pos, players);
	}

	//------------------------------------------------------------------------------------------------
	//! A piece spawned turned by yaw and, a barrier run on a slope, tilted about its own forward so
	//! its x rises `slope` a metre (the ground's along it).
	static IEntity SpawnTilted(ResourceName prefab, vector pos, float yaw, float slope)
	{
		if (slope == 0)
			return RZ_Util.SpawnPrefab(prefab, pos, yaw);

		Resource res = Resource.Load(prefab);
		if (!res || !res.IsValid())
			return null;

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		Math3D.AnglesToMatrix(Vector(yaw, 0, RollOf(slope)), params.Transform);
		params.Transform[3] = pos;
		return GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params);
	}

	//------------------------------------------------------------------------------------------------
	//! The roll (degrees, AnglesToMatrix's third angle) that raises a piece's +x by `slope` a metre.
	static float RollOf(float slope)
	{
		return ROLL_SIGN * Math.Atan2(slope, 1) * Math.RAD2DEG; // VERIFY: a positive roll in Math3D.AnglesToMatrix raises the piece's +x (ROLL_SIGN; the site audit prints "RZS roll"), and a composition spawned rolled tilts its children with it
	}

	//------------------------------------------------------------------------------------------------
	static string UseTag(RZ_EDressUse use)
	{
		switch (use)
		{
			case RZ_EDressUse.SIT: return "RZ_Sit";
			case RZ_EDressUse.WORK: return "RZ_Work";
			case RZ_EDressUse.WATCH: return "RZ_Watch";
			case RZ_EDressUse.GATHER: return "RZ_Gather";
			case RZ_EDressUse.LEAN: return "RZ_Lean";
			case RZ_EDressUse.COOK: return "RZ_Cook";
			case RZ_EDressUse.WASH: return "RZ_Wash";
			case RZ_EDressUse.CHOP: return "RZ_Chop";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Buildings and road segments near the place, and which way its approach faces.
	protected void ReadGround(RZ_CampPlace p, RZ_CampGround ground, float wide = 0)
	{
		// The shared placer's read (World/Traces/RZ_DressPlacer.c), further out for a building
		float reach = RZ_DressPlacer.BUILDINGS_NEAR;
		float roadReach = RZ_DressPlacer.ROADS_NEAR;
		if (m_Settings.m_bBuildings)
		{
			reach = Math.Max(reach, SiteReach(p.m_eKind) + 10);
			reach = Math.Max(reach, wide + 10);
			roadReach = 220;
		}

		RZ_DressPlacer.ReadGroundWithin(p.m_vPos, ground, reach, roadReach);

		// The nearest other town: the main road comes in from there
		float town = float.MAX;
		foreach (RZ_Settlement s : m_GM.GetSettlements())
		{
			float ds = RZ_Util.DistXZ(s.m_vCenter, p.m_vPos);
			if (ds > 300 && ds < town)
			{
				town = ds;
				p.m_fTownBearing = RZ_Compass.Bearing(p.m_vPos, s.m_vCenter);
				p.m_bHasTown = true;
			}
		}

		// The approach faces the nearest road point at least 15 m out, else the nearest other town
		float best = float.MAX;
		foreach (vector pt : ground.m_aRoad)
		{
			float d = RZ_Util.DistXZ(pt, p.m_vPos);
			if (d >= 15 && d < best)
			{
				best = d;
				p.m_vRoad = RZ_Util.GroundPos(pt);
				p.m_bHasRoad = true;
			}
		}

		if (p.m_bHasRoad)
		{
			p.m_fApproach = RZ_Compass.Bearing(p.m_vPos, p.m_vRoad);
		}
		else if (p.m_bHasTown)
		{
			p.m_fApproach = p.m_fTownBearing;
		}
		else
		{
			RZ_Rng rng = new RZ_Rng(p.m_iSeed + 7);
			p.m_fApproach = rng.Range(0, 360);
		}

		// Never toward the sea or over a cliff (a coast road can run between the camp and the
		// water): the dry way nearest it
		p.m_fApproach = Landward(p.m_vPos, p.m_fApproach);
	}

	//------------------------------------------------------------------------------------------------
	//! Water or a drop that way: ground heights along `bearing` and 25 degrees either side, every
	//! 10 m out to `reach`. Water is under sea level (RZ_Util.IsWater); a drop is ground 8 m below
	//! the start within 30 m (a cliff, a quay).
	protected static bool BadOutlook(vector from, float bearing, float reach)
	{
		float startY = GetGame().GetWorld().GetSurfaceY(from[0], from[2]);
		for (int side = -1; side <= 1; side++)
		{
			for (int step = 1; step * 10 <= reach; step++)
			{
				vector pt = RZ_Compass.PointAt(from, bearing + side * 25, step * 10);
				if (RZ_Util.IsWater(pt))
					return true;

				if (step <= 3 && startY - pt[1] > 8)
					return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! `want`, or the nearest of sixteen ways to it with no water or drop within 80 m; `want`
	//! if every way has one (a spit, an island).
	protected static float Landward(vector from, float want)
	{
		if (!BadOutlook(from, want, OUTLOOK_REACH))
			return want;

		for (int k = 1; k <= 8; k++)
		{
			for (int sign = -1; sign <= 1; sign += 2)
			{
				float bearing = RZ_Compass.Normalize(want + sign * k * 22.5);
				if (!BadOutlook(from, bearing, OUTLOOK_REACH))
					return bearing;
			}
		}

		return want;
	}

	// ================================================================ The building a place moves into
	//------------------------------------------------------------------------------------------------
	//! How far from the place's spot its building may be: a stronghold is at its quartermaster, an
	//! outpost at its town's middle, a safehouse in its own house.
	protected static float SiteReach(RZ_ECampPlace kind)
	{
		switch (kind)
		{
			case RZ_ECampPlace.STRONGHOLD: return 60;
			case RZ_ECampPlace.SAFEHOUSE: return 30;
			case RZ_ECampPlace.ROAD_POST: return 0;
		}

		return 90;
	}

	//------------------------------------------------------------------------------------------------
	//! The smallest footprint (square metres) worth moving into.
	protected static float SiteMinArea(RZ_ECampPlace kind)
	{
		if (kind == RZ_ECampPlace.STRONGHOLD)
			return 100;

		if (kind == RZ_ECampPlace.SAFEHOUSE)
			return 30;

		return 45;
	}

	//------------------------------------------------------------------------------------------------
	//! Road entries are where the roads cross a ring this far round the building.
	protected static float EntryRing(RZ_ECampPlace kind)
	{
		if (kind == RZ_ECampPlace.STRONGHOLD)
			return 50;

		if (kind == RZ_ECampPlace.SAFEHOUSE)
			return 28;

		return 40;
	}

	//------------------------------------------------------------------------------------------------
	//! How well a building suits the faction and the kind of place, from its prefab name (the town
	//! code's words: RZ_LandmarkFinder.Matches, RZ_DoorSlots.KindOf): 3 just right, 2 good, 1 will
	//! do, 0 any building, -1 never.
	protected static int SiteTier(RZ_EFaction f, RZ_ECampPlace kind, string name, int door)
	{
		bool church = RZ_LandmarkFinder.Matches("church", name);
		bool fuel = RZ_LandmarkFinder.Matches("fuel", name);
		bool school = RZ_LandmarkFinder.Matches("school", name);
		bool clinic = door == RZ_EDoorWant.CLINIC || RZ_LandmarkFinder.Matches("pharmacy", name);
		bool garage = door == RZ_EDoorWant.GARAGE;
		bool farm = door == RZ_EDoorWant.FARM;
		bool house = door == RZ_EDoorWant.HOUSE;
		bool hall = door == RZ_EDoorWant.PUBLIC && !church;
		bool stronghold = kind == RZ_ECampPlace.STRONGHOLD;

		if (f == RZ_EFaction.MILITARY)
		{
			// The school, the town hall or the police station; an outpost the farm or the petrol station
			if (stronghold)
			{
				if (school || RZ_LandmarkFinder.Matches("police", name) || RZ_LandmarkFinder.Matches("military", name) || name.Contains("townhall") || name.Contains("town_hall") || name.Contains("cityhall"))
					return 3;

				if (hall)
					return 2;

				if (church)
					return -1;

				return 0;
			}

			if (farm || fuel)
				return 3;

			if (house)
				return 1;

			return 0;
		}

		if (f == RZ_EFaction.BANDITS)
		{
			// The garage or the petrol station; a crew camp also a café or a shop by the road
			if (church || clinic)
				return -1;

			if (garage || fuel)
				return 3;

			if (!stronghold && door == RZ_EDoorWant.SHOP)
				return 3;

			if (farm || RZ_LandmarkFinder.Matches("industrial", name))
				return 2;

			return 0;
		}

		if (f == RZ_EFaction.SURVIVORS)
		{
			// The church (a school or a big farmhouse will do); a safehouse its own house
			if (kind == RZ_ECampPlace.SAFEHOUSE)
			{
				if (house)
					return 3;

				return 0;
			}

			if (church)
				return 3;

			// A farmhouse or a barn, not a farm's metal shed
			if (school || (farm && !name.Contains("shed") && !RZ_LandmarkFinder.Matches("industrial", name)))
				return 2;

			if (house)
				return 1;

			return 0;
		}

		// Halcyon: the lab or a clinic; a field station a house
		if (stronghold)
		{
			if (clinic || name.Contains("laborator") || name.Contains("/lab") || name.Contains("_lab") || name.Contains("institute"))
				return 3;

			if (hall)
				return 2;

			return 0;
		}

		if (house)
			return 3;

		if (farm)
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! For the log: "the school", "the garage", "a house".
	protected static string SiteWhat(string name, int door)
	{
		array<string> kinds = {"church", "school", "police", "fuel", "pharmacy", "military", "industrial"};
		foreach (string kind : kinds)
		{
			if (RZ_LandmarkFinder.Matches(kind, name))
				return RZ_LandmarkFinder.DisplayName(kind, name);
		}

		switch (door)
		{
			case RZ_EDoorWant.GARAGE: return "the garage";
			case RZ_EDoorWant.FARM: return "the farm";
			case RZ_EDoorWant.SHOP: return "the shop";
			case RZ_EDoorWant.CLINIC: return "the clinic";
			case RZ_EDoorWant.PUBLIC: return "the hall";
			case RZ_EDoorWant.HOUSE: return "a house";
		}

		return "a building";
	}

	//------------------------------------------------------------------------------------------------
	//! The building this place has moved into: found once per run and kept. A site with m_bNone
	//! means none fits (rings). Null when buildings are off.
	protected RZ_CampSite FindSite(RZ_CampPlace p, RZ_CampGround ground)
	{
		if (!m_Settings.m_bBuildings || p.m_eKind == RZ_ECampPlace.ROAD_POST)
			return null;

		RZ_CampSite kept;
		if (m_mSites.Find(p.m_sKey, kept) && kept && kept.m_eFaction == p.m_eFaction && kept.m_eKind == p.m_eKind && RZ_Util.DistXZ(kept.m_vPlace, p.m_vPos) < 1)
			return kept;

		if (m_mSites.Count() >= MAX_SITES)
			m_mSites.Clear();

		// Any building near beats a raw field: with none of the kind's size in reach, a smaller one a little further
		RZ_CampSite site = PickSite(p, ground, false);
		if (site.m_bNone)
			site = PickSite(p, ground, true);
		m_mSites.Set(p.m_sKey, site);

		if (!m_mToldSite.Contains(p.m_sKey))
		{
			m_mToldSite.Set(p.m_sKey, true);
			string what = string.Format("%1 %2", RZ_Factions.GetName(p.m_eFaction), typename.EnumToString(RZ_ECampPlace, p.m_eKind));
			string grid = RZ_SettlementScanner.GridReference(p.m_vPos);
			if (site.m_bNone)
			{
				RZ_Log.Info(string.Format("Camp life: %1 at %2: no building fits, laid out on rings.", what, grid));
			}
			else
			{
				int w = RZ_Util.RoundToInt(site.m_vMaxs[0] - site.m_vMins[0]);
				int d = RZ_Util.RoundToInt(site.m_vMaxs[2] - site.m_vMins[2]);
				RZ_Log.Info(string.Format("Camp life: %1 at %2 moves into %3 (%4 x %5 m, %6), %7 road entries.", what, grid, site.m_sWhat, w, d, site.m_sPrefab, site.m_aEntries.Count()));
			}
		}

		return site;
	}

	//------------------------------------------------------------------------------------------------
	//! The best building by kind, size and distance, then by the roads (Bastion on a road, the Red
	//! Hand where roads meet). Never one with scenery on it, a ruin, a hangar-sized one or one
	//! smaller than the kind needs; `relaxed`, one of 25 m² will do, up to half as far again.
	//! `wide` (BestSpotIn): anywhere within that many metres, distance counting for little, and only
	//! on dry, even ground.
	protected RZ_CampSite PickSite(RZ_CampPlace p, RZ_CampGround ground, bool relaxed, float wide = 0)
	{
		RZ_CampSite site = new RZ_CampSite();
		site.m_sKey = p.m_sKey;
		site.m_eFaction = p.m_eFaction;
		site.m_eKind = p.m_eKind;
		site.m_vPlace = p.m_vPos;
		site.m_bNone = true;

		float reach = SiteReach(p.m_eKind);
		float minArea = SiteMinArea(p.m_eKind);
		if (relaxed)
		{
			reach = Math.Max(reach * 1.5, 45);
			minArea = Math.Min(minArea, 25);
		}

		bool stronghold = p.m_eKind == RZ_ECampPlace.STRONGHOLD;
		float distWeight = 3;
		if (p.m_eKind == RZ_ECampPlace.SAFEHOUSE)
			distWeight = 12;

		if (wide > 0)
		{
			reach = wide;
			distWeight = 0.5;
		}

		array<IEntity> shortlist = {};
		array<float> scores = {};
		array<int> tiers = {};
		foreach (IEntity b : ground.m_aBuildings)
		{
			if (!b || IsDressing(b))
				continue;

			vector mins;
			vector maxs;
			if (!RZ_DoorGeometry.LocalBox(b, mins, maxs))
				continue;

			float w = maxs[0] - mins[0];
			float d = maxs[2] - mins[2];
			float area = w * d;
			if (w > SITE_MAX_SIDE || d > SITE_MAX_SIDE || area < minArea)
				continue;

			// Not a pier or a jetty: a long thin box isn't a house (the Haven once camped on a 5 x 50 m pier)
			if (Math.Min(w, d) < SITE_MIN_SIDE || Math.Max(w, d) > Math.Min(w, d) * SITE_MAX_RATIO)
				continue;

			vector mid = RZ_DoorGeometry.ToWorld(b, (mins + maxs) * 0.5);
			float dist = RZ_Util.DistXZ(mid, p.m_vPos);
			if (dist > reach)
				continue;

			string name = RZ_Util.GetPrefabName(b);
			name.ToLower();
			if (name.Contains("ruin") || name.Contains("wreck") || name.Contains("bridge") || name.Contains("ramp"))
				continue;

			// Never by or over the water: a pier, a quay, a harbour shed, a boathouse (docs/SITES.md)
			if (!s_bProbeOld && (IsNaval(name) || OverWater(b, mins, maxs)))
				continue;

			if (s_bProbeOld && (name.Contains("pier") || name.Contains("jetty") || name.Contains("dock") || name.Contains("pontoon") || name.Contains("wharf")))
				continue;

			int tier = SiteTier(p.m_eFaction, p.m_eKind, name, RZ_DoorSlots.KindOf(b));
			if (tier < 0)
				continue;

			if (wide > 0 && !EvenGround(b, mins, maxs, p.m_eKind))
				continue;

			// The largest for a stronghold; a middling one elsewhere
			float size = Math.Min(area, 200) * 0.75;
			if (stronghold)
				size = Math.Min(area, 600) * 0.5;

			float score = tier * 1000 + size - dist * distWeight;
			int at = 0;
			while (at < scores.Count() && scores[at] >= score)
			{
				at++;
			}

			if (at >= SITE_SHORTLIST)
				continue;

			shortlist.InsertAt(b, at);
			scores.InsertAt(score, at);
			tiers.InsertAt(tier, at);
			if (shortlist.Count() > SITE_SHORTLIST)
			{
				shortlist.Remove(SITE_SHORTLIST);
				scores.Remove(SITE_SHORTLIST);
				tiers.Remove(SITE_SHORTLIST);
			}
		}

		IEntity best;
		float bestScore = -float.MAX;
		foreach (int i, IEntity cand : shortlist)
		{
			float total = scores[i] + RoadScore(p, ground, cand);
			if (total > bestScore)
			{
				bestScore = total;
				best = cand;
				site.m_iTier = tiers[i];
			}
		}

		if (best)
			ReadSite(p, ground, site, best);

		return site;
	}

	//------------------------------------------------------------------------------------------------
	//! A prefab of the waterside: a pier, a jetty, a dock, a quay, a harbour, a breakwater, a
	//! slipway, a pontoon, a wharf, a marina, a mooring, a boathouse or a fisherman's shed, a
	//! lighthouse, or anything under the naval structures' folder.
	static bool IsNaval(string name)
	{
		array<string> words = {"pier", "jetty", "dock", "quay", "harbo", "breakwater", "slipway", "pontoon", "wharf", "marina", "mooring", "boatshed", "boathouse", "fisherman", "lighthouse", "/naval/"};
		foreach (string w : words)
		{
			if (name.Contains(w))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A building box over water: its middle, or two of its corners, over the sea, a river or a pond
	//! (a quay shed on stilts, a building on a pier, a barn on the shore).
	static bool OverWater(IEntity b, vector mins, vector maxs)
	{
		if (WetAt(RZ_DoorGeometry.ToWorld(b, (mins + maxs) * 0.5)))
			return true;

		array<vector> local = {Vector(mins[0], 0, mins[2]), Vector(maxs[0], 0, mins[2]), Vector(maxs[0], 0, maxs[2]), Vector(mins[0], 0, maxs[2])};
		int wet = 0;
		foreach (vector l : local)
		{
			if (WetAt(RZ_DoorGeometry.ToWorld(b, l)))
				wet++;
		}

		return wet >= 2;
	}

	//------------------------------------------------------------------------------------------------
	//! Over the sea (the ground under 0.5 m), a river or a pond.
	static bool WetAt(vector pt)
	{
		vector ground = RZ_Util.GroundPos(pt);
		if (RZ_Util.IsWater(ground))
			return true;

		float reach;
		EWaterSurfaceType type = RZ_Land.WaterAt(ground, reach);
		return type == EWaterSurfaceType.WST_POND || type == EWaterSurfaceType.WST_RIVER;
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion and Halcyon's lab want a road at the door (the truck); the Red Hand wants roads
	//! meeting (the toll); the rest don't mind.
	protected float RoadScore(RZ_CampPlace p, RZ_CampGround ground, IEntity b)
	{
		bool redHand = p.m_eFaction == RZ_EFaction.BANDITS;
		bool wantsRoad = p.m_eFaction == RZ_EFaction.MILITARY || (p.m_eFaction == RZ_EFaction.INSTITUTE && p.m_eKind == RZ_ECampPlace.STRONGHOLD);
		if (!redHand && !wantsRoad)
			return 0;

		vector mins;
		vector maxs;
		RZ_DoorGeometry.LocalBox(b, mins, maxs);
		vector mid = RZ_DoorGeometry.ToWorld(b, (mins + maxs) * 0.5);
		float half = Math.Max(maxs[0] - mins[0], maxs[2] - mins[2]) * 0.5;
		float onRoad = half + 25;
		float meet = half + 45;

		bool near = false;
		array<int> roads = {};
		for (int i = 0; i + 1 < ground.m_aRoad.Count(); i += 2)
		{
			float d = RZ_DressPlacer.DistToSegment(mid, ground.m_aRoad[i], ground.m_aRoad[i + 1]);
			if (d < onRoad)
				near = true;

			int id = ground.m_aRoadId[i / 2];
			if (d < meet && !roads.Contains(id))
				roads.Insert(id);
		}

		float score = 0;
		if (near)
			score = 150;

		if (redHand)
			score = score - 50 + Math.Max(0, roads.Count() - 1) * 150;

		return Math.Max(0, score);
	}

	//------------------------------------------------------------------------------------------------
	//! The building's box and transform, its road entries, its front (toward the road in) and
	//! front door, its yard and its cooking side.
	protected void ReadSite(RZ_CampPlace p, RZ_CampGround ground, RZ_CampSite site, IEntity b)
	{
		site.m_bNone = false;
		vector mins;
		vector maxs;
		RZ_DoorGeometry.LocalBox(b, mins, maxs);
		site.m_vMins = mins;
		site.m_vMaxs = maxs;
		vector mat[4];
		b.GetWorldTransform(mat);
		site.m_vRight = mat[0];
		site.m_vUp = mat[1];
		site.m_vFwd = mat[2];
		site.m_vOrigin = mat[3];
		site.m_vCentre = RZ_Util.GroundPos(LocalToWorld(site, (mins + maxs) * 0.5));

		string name = RZ_Util.GetPrefabName(b);
		name.ToLower();
		site.m_sWhat = SiteWhat(name, RZ_DoorSlots.KindOf(b));

		ReadEntries(p, ground, site);

		// The front faces the road in: the gate, else the nearest road, else the next town
		vector toward = RZ_Compass.PointAt(site.m_vCentre, p.m_fApproach, 60);
		if (site.m_iMain >= 0)
			toward = site.m_aEntries[site.m_iMain];
		else if (p.m_bHasRoad)
			toward = p.m_vRoad;

		// The front door: the outside door nearest that way, if the building has door children
		array<IEntity> doors = {};
		RZ_DoorGeometry.FindDoors(b, 0, doors);
		IEntity front;
		float frontDist = float.MAX;
		foreach (IEntity door : doors)
		{
			if (!RZ_DoorGeometry.IsOuterDoor(b, door))
				continue;

			float dd = RZ_Util.DistXZ(door.GetOrigin(), toward);
			if (dd < frontDist)
			{
				frontDist = dd;
				front = door;
			}
		}

		if (front)
		{
			FaceOf(site, RZ_DoorGeometry.ToLocal(b, front.GetOrigin()), true);
			site.m_bDoor = true;
		}
		else
		{
			FaceOf(site, RZ_DoorGeometry.ToLocal(b, toward), false);
		}

		site.m_iYardFace = FACE_BACK;
		if (p.m_eFaction == RZ_EFaction.BANDITS)
			site.m_iYardFace = FACE_FRONT;

		// The yard (where the people come out) never over water: the next side that's dry
		array<int> yardFaces = {site.m_iYardFace, FACE_LEFT, FACE_RIGHT, FACE_BACK, FACE_FRONT};
		foreach (int yf : yardFaces)
		{
			if (!WetAt(FacePoint(site, yf, 0, 6)))
			{
				site.m_iYardFace = yf;
				break;
			}
		}

		site.m_vHome = FacePoint(site, site.m_iYardFace, 0, 6);
		site.m_sPrefab = FilePath.StripPath(RZ_Util.GetPrefabName(b));
	}

	//------------------------------------------------------------------------------------------------
	//! The front is the side of the box nearest `local` (a door, or the way in), in the building's
	//! own space. For a door, also where along that side it is.
	protected static void FaceOf(RZ_CampSite site, vector local, bool isDoor)
	{
		vector mins = site.m_vMins;
		vector maxs = site.m_vMaxs;
		vector mid = (mins + maxs) * 0.5;
		float halfX = Math.Max(0.5, (maxs[0] - mins[0]) * 0.5);
		float halfZ = Math.Max(0.5, (maxs[2] - mins[2]) * 0.5);

		// A door: the wall it's closest to. The way in: the side it lies beyond, by the box's shape
		bool alongX;
		if (isDoor)
			alongX = Math.AbsFloat(local[0] - mid[0]) - halfX > Math.AbsFloat(local[2] - mid[2]) - halfZ;
		else
			alongX = Math.AbsFloat(local[0] - mid[0]) / halfX > Math.AbsFloat(local[2] - mid[2]) / halfZ;

		site.m_iAxis = 2;
		site.m_fSign = 1;
		if (local[2] < mid[2])
			site.m_fSign = -1;

		if (alongX)
		{
			site.m_iAxis = 0;
			site.m_fSign = 1;
			if (local[0] < mid[0])
				site.m_fSign = -1;
		}

		site.m_fDoor = 0;
		if (!isDoor)
			return;

		if (site.m_iAxis == 0)
			site.m_fDoor = Math.Clamp((local[2] - mid[2]) / halfZ, -1, 1);
		else
			site.m_fDoor = Math.Clamp((local[0] - mid[0]) / halfX, -1, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Where the roads cross the ring round the building: one entry per crossing. The main one
	//! (the gate) is the one toward the next town; the others are closed.
	protected void ReadEntries(RZ_CampPlace p, RZ_CampGround ground, RZ_CampSite site)
	{
		float ring = EntryRing(p.m_eKind);
		vector c = site.m_vCentre;
		for (int i = 0; i + 1 < ground.m_aRoad.Count(); i += 2)
		{
			vector a = ground.m_aRoad[i];
			vector b = ground.m_aRoad[i + 1];
			float da = RZ_Util.DistXZ(a, c);
			float db = RZ_Util.DistXZ(b, c);
			float bearing = RZ_Compass.Bearing(a, b);
			int roadId = ground.m_aRoadId[i / 2];
			if (da < ring && db >= ring)
			{
				AddCrossing(site, c, ring, a, b, bearing, roadId);
			}
			else if (db < ring && da >= ring)
			{
				AddCrossing(site, c, ring, b, a, bearing, roadId);
			}
			else if (da >= ring && db >= ring && RZ_DressPlacer.DistToSegment(c, a, b) < ring)
			{
				// A long straight passing through: in and out again
				vector close = ClosestOnSegment(c, a, b);
				AddCrossing(site, c, ring, close, a, bearing, roadId);
				AddCrossing(site, c, ring, close, b, bearing, roadId);
			}
		}

		if (site.m_aEntries.IsEmpty())
			return;

		site.m_iMain = 0;
		if (!p.m_bHasTown)
			return;

		float bestDiff = float.MAX;
		foreach (int k, vector e : site.m_aEntries)
		{
			float diff = Math.AbsFloat(RZ_Compass.Normalize(RZ_Compass.Bearing(c, e) - p.m_fTownBearing + 180) - 180);
			if (diff < bestDiff)
			{
				bestDiff = diff;
				site.m_iMain = k;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The point between `inside` and `outside` that lies on the ring (halved eight times), unless
	//! an entry is already there.
	protected static void AddCrossing(RZ_CampSite site, vector c, float ring, vector inside, vector outside, float roadBearing, int roadId)
	{
		if (site.m_aEntries.Count() >= MAX_ENTRIES)
			return;

		vector lo = inside;
		vector hi = outside;
		for (int k = 0; k < 8; k++)
		{
			vector half = (lo + hi) * 0.5;
			if (RZ_Util.DistXZ(half, c) < ring)
				lo = half;
			else
				hi = half;
		}

		vector hit = (lo + hi) * 0.5;
		foreach (vector e : site.m_aEntries)
		{
			if (RZ_Util.DistXZ(e, hit) < ENTRY_MERGE)
				return;
		}

		site.m_aEntries.Insert(RZ_Util.GroundPos(hit));
		site.m_aEntryRoad.Insert(roadBearing);
		site.m_aEntryRoadId.Insert(roadId);
	}

	//------------------------------------------------------------------------------------------------
	protected static vector ClosestOnSegment(vector pt, vector a, vector b)
	{
		float dx = b[0] - a[0];
		float dz = b[2] - a[2];
		float len2 = dx * dx + dz * dz;
		if (len2 < 0.01)
			return a;

		float t = ((pt[0] - a[0]) * dx + (pt[2] - a[2]) * dz) / len2;
		t = Math.Clamp(t, 0, 1);
		vector proj = a;
		proj[0] = a[0] + dx * t;
		proj[2] = a[2] + dz * t;
		return proj;
	}

	//------------------------------------------------------------------------------------------------
	protected static vector LocalToWorld(RZ_CampSite site, vector local)
	{
		return site.m_vOrigin + site.m_vRight * local[0] + site.m_vUp * local[1] + site.m_vFwd * local[2];
	}

	//------------------------------------------------------------------------------------------------
	//! A point `dist` m out from one side of the building (FACE_*), `along` that side from -1 to 1
	//! (0 = the middle; past 1 = beyond the corner), on the ground.
	protected static vector FacePoint(RZ_CampSite site, int face, float along, float dist)
	{
		int a = site.m_iAxis;
		int o = 0;
		if (a == 0)
			o = 2;

		float midA = (site.m_vMins[a] + site.m_vMaxs[a]) * 0.5;
		float halfA = (site.m_vMaxs[a] - site.m_vMins[a]) * 0.5;
		float midO = (site.m_vMins[o] + site.m_vMaxs[o]) * 0.5;
		float halfO = (site.m_vMaxs[o] - site.m_vMins[o]) * 0.5;
		float ca;
		float co;
		if (face == FACE_FRONT || face == FACE_BACK)
		{
			ca = midA + site.m_fSign * (halfA + dist);
			if (face == FACE_BACK)
				ca = midA - site.m_fSign * (halfA + dist);

			co = midO + along * halfO;
		}
		else
		{
			co = midO + halfO + dist;
			if (face == FACE_LEFT)
				co = midO - (halfO + dist);

			ca = midA + along * halfA;
		}

		vector local = vector.Zero;
		local[a] = ca;
		local[o] = co;
		return RZ_Util.GroundPos(LocalToWorld(site, local));
	}

	//------------------------------------------------------------------------------------------------
	//! A place that has moved into a building uses it: its yard is home, the building the centre,
	//! and the way in is the gate.
	protected void UseSite(RZ_CampPlace p, RZ_CampGround ground, RZ_CampSite site)
	{
		if (!site || site.m_bNone)
			return;

		p.m_Site = site;
		p.m_vHome = site.m_vHome;
		p.m_vCentre = site.m_vCentre;
		float halfO = (site.m_vMaxs[0] - site.m_vMins[0]) * 0.5;
		if (site.m_iAxis == 0)
			halfO = (site.m_vMaxs[2] - site.m_vMins[2]) * 0.5;

		p.m_fPersonRadius = Math.Max(PERSON_RADIUS, halfO + 8);
		if (site.m_iMain >= 0)
			p.m_fApproach = RZ_Compass.Bearing(site.m_vCentre, site.m_aEntries[site.m_iMain]);
		else
			p.m_fApproach = RZ_Compass.Bearing(site.m_vCentre, FacePoint(site, FACE_FRONT, 0, 10));

		p.m_fApproach = Landward(site.m_vCentre, p.m_fApproach);

		// The building's neighbours too, so no piece stands in one
		array<IEntity> around = {};
		RZ_BuildingQuery.FindBuildings(site.m_vCentre, 70, around);
		foreach (IEntity b : around)
		{
			if (b && !ground.m_aBuildings.Contains(b))
				ground.m_aBuildings.Insert(b);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The lookout's spot: a front corner of the building (the one nearer the gate first), facing
	//! the way in. From the ground: nobody is sent onto a roof.
	vector LookoutSpot(RZ_CampPlace p, int which)
	{
		RZ_CampSite site = p.m_Site;
		if (!site.m_aLookouts.IsEmpty())
			return site.m_aLookouts[which % site.m_aLookouts.Count()];

		if (p.m_eKind == RZ_ECampPlace.SAFEHOUSE)
			return FacePoint(site, FACE_FRONT, site.m_fDoor, LOOKOUT_OUT);

		vector left = FacePoint(site, FACE_FRONT, -1, LOOKOUT_OUT);
		vector right = FacePoint(site, FACE_FRONT, 1, LOOKOUT_OUT);
		vector gate = FacePoint(site, FACE_FRONT, 0, 30);
		if (site.m_iMain >= 0)
			gate = site.m_aEntries[site.m_iMain];

		bool leftFirst = RZ_Util.DistXZ(left, gate) <= RZ_Util.DistXZ(right, gate);
		if ((which % 2 == 0) == leftFirst)
			return left;

		return right;
	}

	//------------------------------------------------------------------------------------------------
	//! Where people go in to sleep and come out in the morning: outside the front door.
	vector DoorSpot(RZ_CampPlace p)
	{
		return FacePoint(p.m_Site, FACE_FRONT, p.m_Site.m_fDoor, 1.5);
	}

	// ================================================================ Routines
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		m_fClock += dt;
		if (m_aPlaces.IsEmpty())
			return;

		if (m_Settings.m_iSpawnMs <= 0)
			SpawnPending(SPAWN_PER_TICK, 0, players);

		m_fStoryTimer += dt;
		if (m_fStoryTimer >= CHECK_INTERVAL)
		{
			m_fStoryTimer = 0;
			StoryDay();
		}

		if (!m_Settings.m_bRoutines)
			return;

		m_fLiveTimer += dt;
		if (m_fLiveTimer >= LIVE_INTERVAL || m_bLiveDirty)
		{
			m_fLiveTimer = 0;
			m_bLiveDirty = false;
			UpdateLive(players);
		}

		m_fCheckTimer += dt;
		if (m_fCheckTimer < CHECK_INTERVAL)
			return;

		float step = m_fCheckTimer;
		m_fCheckTimer = 0;
		m_fShuffleTimer += step;
		bool shuffle = m_fShuffleTimer >= Math.Max(0.5, m_Settings.m_fShuffleMinutes) * 60;
		if (shuffle)
			m_fShuffleTimer = 0;

		int phase = RZ_Territory.RoutinePhase(RZ_GameTime.GetHour());
		if (phase == RZ_Territory.PHASE_NONE)
			phase = RZ_Territory.PHASE_DAY;

		foreach (RZ_CampPlace p : m_aPlaces)
		{
			if (p.m_bLive)
				Live(p, phase, step, shuffle, ids, players);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The Live places nearest players do their routine; the rest stand.
	protected void UpdateLive(array<IEntity> players)
	{
		array<float> dists = {};
		foreach (RZ_CampPlace p : m_aPlaces)
		{
			dists.Insert(RZ_Util.NearestDistance(p.m_vPos, players));
		}

		int live = Math.Max(0, m_Settings.m_iLivePlaces);
		foreach (int i, RZ_CampPlace q : m_aPlaces)
		{
			int closer = 0;
			foreach (int j, float d : dists)
			{
				if (d < dists[i] || (d == dists[i] && j < i))
					closer++;
			}

			bool now = closer < live && dists[i] < float.MAX;
			if (now == q.m_bLive)
				continue;

			q.m_bLive = now;
			if (now)
			{
				q.m_iPhase = -2;
				continue;
			}

			foreach (RZ_CampWorker w : q.m_aWorkers)
			{
				ClearJob(q, w);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Live(RZ_CampPlace p, int phase, float step, bool shuffle, array<int> ids, array<IEntity> players)
	{
		Prune(p);
		float nearest = RZ_Util.NearestDistance(p.m_vPos, players);
		Greet(p, ids, players);
		if (p.m_iPhase == phase && UpdateAlert(p))
		{
			AssignAll(p, phase);
			LogJobs(p, phase);
		}

		if (p.m_iPhase != phase)
		{
			bool first = p.m_iPhase == -2;
			p.m_iPhase = phase;
			OnPhase(p, phase, first);
		}
		else if (p.m_fGatherUntil > 0 && m_fClock >= p.m_fGatherUntil)
		{
			p.m_fGatherUntil = 0;
			AssignAll(p, phase);
		}

		bool night = phase == RZ_Territory.PHASE_NIGHT && p.m_fGatherUntil == 0;
		float nearHome = Math.Min(nearest, RZ_Util.NearestDistance(p.m_vHome, players));
		if (nearHome > SLEEP_CLEAR)
		{
			if (night)
				PutToBed(p, players);
			else if (!RZ_InfectedBodies.InSight(p.m_vHome, players, false))
				Wake(p, phase);
		}

		if (nearest > EXTRA_CLEAR && !night)
			AddExtra(p, phase);

		if (p.m_eFaction == RZ_EFaction.INSTITUTE && p.m_eKind != RZ_ECampPlace.ROAD_POST && !night)
		{
			p.m_fCourierTimer += step;
			if (p.m_fCourierTimer >= COURIER_SECONDS)
			{
				p.m_fCourierTimer = 0;
				SendCourier(p);
			}
		}

		if (!night && phase == RZ_Territory.PHASE_DAY)
			RoadPostRun(p, phase);

		if (shuffle)
			Shuffle(p, phase);
	}

	//------------------------------------------------------------------------------------------------
	//! Workers whose body is gone or dead lose their job, and so does anyone whose piece is gone
	//! (no waypoint is left pointing at a deleted smart action; the next shuffle gives them another).
	//! Owned bodies stay listed until Leave.
	protected void Prune(RZ_CampPlace p)
	{
		for (int i = p.m_aWorkers.Count() - 1; i >= 0; i--)
		{
			RZ_CampWorker w = p.m_aWorkers[i];
			if (w.m_iPiece >= 0 && (w.m_iPiece >= p.m_aPieces.Count() || !p.m_aPieces[w.m_iPiece].m_Entity))
				ClearJob(p, w);

			if (w.m_Unit && RZ_Util.CountAlive(w.m_Unit) > 0)
				continue;

			// One of its people died here: contact (the alert)
			if (w.m_Unit && w.m_iRole != ROLE_KEEPER)
				p.m_fLossAt = m_fClock;

			ClearJob(p, w);
			if (w.m_bWrapped)
			{
				RZ_Util.UnwrapGroup(w.m_Unit, w.m_Group);
				w.m_bWrapped = false;
				w.m_Group = null;
			}

			if (!w.m_bOwned || !w.m_Unit)
				p.m_aWorkers.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsWorking(RZ_CampWorker w)
	{
		return w.m_iRole != ROLE_KEEPER && w.m_Unit && RZ_Util.CountAlive(w.m_Unit) > 0 && w.m_fHoldUntil <= m_fClock;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPhase(RZ_CampPlace p, int phase, bool first)
	{
		if (!first && (phase == RZ_Territory.PHASE_MORNING || phase == RZ_Territory.PHASE_EVENING))
		{
			p.m_iShift++;
			// The handover: the watch walks off its posts to the fire; the relief comes from the rest
			foreach (RZ_CampWorker was : p.m_aWorkers)
			{
				was.m_bWasOnWatch = was.m_iJob == JOB_WATCH || was.m_iJob == JOB_PATROL;
			}
		}

		UpdateAlert(p);

		// Bastion's roll: when the night watch is set, everyone stands at the board for a few minutes
		if (!first && phase == RZ_Territory.PHASE_NIGHT && p.m_eFaction == RZ_EFaction.MILITARY && p.m_eKind != RZ_ECampPlace.ROAD_POST)
			p.m_fGatherUntil = m_fClock + ROLL_SECONDS;

		AssignAll(p, phase);
		LogJobs(p, phase);
	}

	//------------------------------------------------------------------------------------------------
	//! One line per place at each phase (and when its pieces are all up, or its alert changes): who
	//! is on which job and how (at a seat, walking the camp, standing at a spot, on the move, or
	//! standing still with no AI or group), then its posts filled, the routes walked and its alert
	//! (docs/SITES.md, "The people"), so a play-test can see the place is alive.
	protected void LogJobs(RZ_CampPlace p, int phase)
	{
		array<int> jobs = {};
		for (int j = 0; j <= JOB_CHOP; j++)
		{
			jobs.Insert(0);
		}

		array<int> how = {0, 0, 0, 0, 0};
		int people = 0;
		foreach (RZ_CampWorker w : p.m_aWorkers)
		{
			if (!IsWorking(w))
				continue;

			people++;
			if (w.m_iJob >= 0 && w.m_iJob <= JOB_CHOP)
				jobs[w.m_iJob] = jobs[w.m_iJob] + 1;

			int h = w.m_iHow;
			if (!w.m_Group)
				h = HOW_NONE;

			how[h] = how[h] + 1;
		}

		string list = "";
		for (int k = 1; k <= JOB_CHOP; k++)
		{
			if (jobs[k] == 0)
				continue;

			if (!list.IsEmpty())
				list += ", ";

			list += string.Format("%1 %2", JobName(k), jobs[k]);
		}

		if (list.IsEmpty())
			list = "none";

		array<int> postJobs = {};
		array<int> postArgs = {};
		foreach (RZ_CampWorker o : p.m_aWorkers)
		{
			if (IsWorking(o))
			{
				postJobs.Insert(o.m_iJob);
				postArgs.Insert(o.m_iArg);
			}
		}

		RZ_Log.Info(string.Format("Camp life jobs: %1 (phase %2), %3 people: %4; %5 at a seat, %6 walking the camp, %7 standing at a spot, %8 on the move, %9 standing still (no AI or group)",
			p.m_sTitle, phase, people, list, how[HOW_SEAT], how[HOW_WALK], how[HOW_SPOT], how[HOW_MOVE], how[HOW_NONE]) + "; " + PlanText(p, postJobs, postArgs) + HandText(p) + ".");
	}

	//------------------------------------------------------------------------------------------------
	//! At a hand-built site: "; hand-placed spots: 5 of 9 in use (sit 3, cook 1, watch 1)".
	protected string HandText(RZ_CampPlace p)
	{
		if (!p.m_bHand)
			return "";

		int spots = 0;
		int used = 0;
		map<string, int> byType = new map<string, int>();
		foreach (RZ_CampWorker w : p.m_aWorkers)
		{
			if (!IsWorking(w) || w.m_iPiece < 0 || w.m_iPiece >= p.m_aPieces.Count())
				continue;

			string name = p.m_aPieces[w.m_iPiece].m_sName;
			if (name.IndexOf("HandSpot_") != 0)
				continue;

			string type = name.Substring(9, name.Length() - 9);
			int had = 0;
			byType.Find(type, had);
			byType.Set(type, had + 1);
			used++;
		}

		foreach (RZ_CampPiece piece : p.m_aPieces)
		{
			if (piece.m_sName.IndexOf("HandSpot_") == 0)
				spots++;
		}

		string list = "";
		foreach (string t, int n : byType)
		{
			if (!list.IsEmpty())
				list += ", ";

			list += string.Format("%1 %2", t, n);
		}

		return string.Format("; hand-placed spots: %1 people at %2 spots (%3)", used, spots, list);
	}

	//------------------------------------------------------------------------------------------------
	//! "posts gate 2 of 1, nest 1 of 1, lookout 1 of 2, door 0 of 1; 0 on the wire facing the threat;
	//! walked: the wire 1, the approach 1; alert calm" for jobs and their arguments.
	protected string PlanText(RZ_CampPlace p, array<int> jobs, array<int> args)
	{
		RZ_SitePeople people = p.m_People;
		if (!people)
			return "no posts (laid out without the solver)";

		array<int> kinds = {RZ_SitePost.GATE, RZ_SitePost.NEST, RZ_SitePost.LOOKOUT, RZ_SitePost.DOOR};
		string posts = "";
		foreach (int kind : kinds)
		{
			int have = people.CountPosts(kind);
			if (have == 0)
				continue;

			int manned = 0;
			foreach (int i, int job : jobs)
			{
				int arg = args[i];
				if (job == JOB_WATCH && arg >= 0 && arg < people.m_aPosts.Count() && people.m_aPosts[arg].m_iKind == kind)
					manned++;
			}

			if (!posts.IsEmpty())
				posts += ", ";

			posts += string.Format("%1 %2 of %3", RZ_SitePost.KindName(kind), manned, have);
		}

		if (posts.IsEmpty())
			posts = "none";

		int line = 0;
		array<int> walked = {0, 0, 0, 0, 0};
		int small = 0;
		foreach (int n, int what : jobs)
		{
			int on = args[n];
			if (what == JOB_WATCH && on >= ARG_LINE)
				line++;

			if (what != JOB_PATROL && what != JOB_STROLL && what != JOB_COURIER)
				continue;

			if (on >= 0 && on < RZ_SiteRoute.KINDS)
				walked[on] = walked[on] + 1;
			else if (on == ARG_LATRINE || on == ARG_STORES)
				small++;
		}

		string routes = "";
		for (int r = 0; r < RZ_SiteRoute.KINDS; r++)
		{
			if (walked[r] == 0)
				continue;

			if (!routes.IsEmpty())
				routes += ", ";

			routes += string.Format("%1 %2", RZ_SiteRoute.KindName(r), walked[r]);
		}

		if (small > 0)
		{
			if (!routes.IsEmpty())
				routes += ", ";

			routes += string.Format("the latrine or the stores %1", small);
		}

		if (routes.IsEmpty())
			routes = "none";

		return string.Format("posts %1; %2 on the wire facing the threat; walked: %3; alert %4", posts, line, routes, RZ_SitePeople.AlertName(p.m_iAlert));
	}

	//------------------------------------------------------------------------------------------------
	protected static string JobName(int job)
	{
		switch (job)
		{
			case JOB_WATCH: return "watch";
			case JOB_PATROL: return "patrol";
			case JOB_SIT: return "sit";
			case JOB_WORK: return "work";
			case JOB_GATHER: return "gather";
			case JOB_LEAN: return "lean";
			case JOB_STROLL: return "stroll";
			case JOB_COURIER: return "courier";
			case JOB_OFF: return "off shift";
			case JOB_COOK: return "cook";
			case JOB_WASH: return "wash";
			case JOB_CHOP: return "chop";
		}

		return "none";
	}

	//------------------------------------------------------------------------------------------------
	//! The routine (docs/SITES.md, "The people"): the jobs in order for this faction, phase and alert,
	//! with their posts and routes in `args`, and the job for everyone after them. The watches come
	//! first and fill the site's posts by importance (RZ_SitePeople.Order); without posts (Solver off)
	//! they keep today's spots.
	protected int Slots(RZ_CampPlace p, int phase, notnull array<int> slots, notnull array<int> args)
	{
		bool night = phase == RZ_Territory.PHASE_NIGHT;
		if (p.m_eKind == RZ_ECampPlace.ROAD_POST)
		{
			AddSlot(slots, args, JOB_LEAN, -1);
			return JOB_SIT;
		}

		// Alert: everyone with a weapon to the wire on the threat side (named people indoors: RestJob)
		if (p.m_iAlert == RZ_SitePeople.ALERT_ALERT && p.m_People)
			return JOB_WATCH;

		bool wary = p.m_iAlert == RZ_SitePeople.ALERT_WARY;
		bool hit = p.m_iAlert == RZ_SitePeople.ALERT_HIT;
		bool stronghold = p.m_eKind == RZ_ECampPlace.STRONGHOLD;
		bool byDay = !night && phase != RZ_Territory.PHASE_MEAL;
		int watch = 0;
		array<int> after = {};
		array<int> afterArgs = {};
		int rest = JOB_SIT;
		bool threatWalk = false;
		if (p.m_eFaction == RZ_EFaction.MILITARY)
		{
			if (p.m_fGatherUntil > m_fClock)
			{
				// The roll: one on the gate, everyone else at the board
				watch = 1;
				rest = JOB_GATHER;
			}
			else if (night)
			{
				// The gate and a lookout on the covered side; the curfew adds the second gate watch,
				// else one sits up at the fire
				watch = 1;
				if (HasLookout(p))
					watch++;

				if (IsCurfew())
					watch++;
				else
					AddSlot(after, afterArgs, JOB_SIT, -1);

				rest = JOB_OFF;
			}
			else if (phase == RZ_Territory.PHASE_MEAL)
			{
				watch = 1;
				AddSlot(after, afterArgs, JOB_PATROL, RZ_SiteRoute.WIRE);
				AddSlot(after, afterArgs, JOB_SIT, -1);
				AddSlot(after, afterArgs, JOB_SIT, -1);
				rest = JOB_SIT;
			}
			else
			{
				// The gate (two at a stronghold: the barrier and the nest), the lookout by a building;
				// the wire's walk and, by turns, the approach or the threat side; the board, the chore,
				// the fire, the rest at work
				watch = 1;
				if (stronghold)
					watch++;

				if (p.m_Site)
					watch++;

				AddSlot(after, afterArgs, JOB_PATROL, RZ_SiteRoute.WIRE);
				int second = RZ_SiteRoute.APPROACH;
				if (p.m_iShift % 2 == 1 || hit)
					second = RZ_SiteRoute.THREAT;

				AddSlot(after, afterArgs, JOB_PATROL, second);
				threatWalk = second == RZ_SiteRoute.THREAT;
				AddSlot(after, afterArgs, JOB_GATHER, -1);
				AddChore(after, afterArgs, phase, true, true, true);
				AddSlot(after, afterArgs, JOB_SIT, -1);
				rest = JOB_WORK;
			}
		}
		else if (p.m_eFaction == RZ_EFaction.BANDITS)
		{
			if (night)
			{
				// One lookout, the rest drinking at the fire
				watch = 1;
				rest = JOB_SIT;
			}
			else
			{
				// Two at the barrier taking the toll, the nest at a stronghold, one strolling out along
				// the road in; wood by day, the pot before the meal (nobody washes)
				AddSlot(after, afterArgs, JOB_LEAN, -1);
				AddSlot(after, afterArgs, JOB_LEAN, -1);
				if (p.m_Site && stronghold)
					watch = 1;

				AddSlot(after, afterArgs, JOB_STROLL, RZ_SiteRoute.APPROACH);
				AddChore(after, afterArgs, phase, false, true, true);
				AddSlot(after, afterArgs, JOB_SIT, -1);
				rest = JOB_SIT;
			}
		}
		else if (p.m_eFaction == RZ_EFaction.SURVIVORS)
		{
			// The lookout always (the chair at the gate, else the door); the dusk meal at the table;
			// at night the rest inside
			watch = 1;
			if (night)
				rest = JOB_OFF;
			else if (phase == RZ_Territory.PHASE_MEAL)
				rest = JOB_GATHER;
			else
			{
				// The phase's chore, the water, then the rest
				AddChore(after, afterArgs, phase, true, true, true);
				AddSlot(after, afterArgs, JOB_WORK, ARG_WATER);
				AddSlot(after, afterArgs, JOB_WORK, -1);
				AddSlot(after, afterArgs, JOB_WORK, -1);
				rest = JOB_SIT;
			}
		}
		else
		{
			// Halcyon: the gate's guard post and the guard pair at the door; at the lab the lit posts
			// keep their watch at night and a guard walks the wire; one at the cold box
			if (night)
			{
				watch = 1;
				if (stronghold)
				{
					watch = 2;
					AddSlot(after, afterArgs, JOB_PATROL, RZ_SiteRoute.WIRE);
				}

				AddSlot(after, afterArgs, JOB_WORK, -1);
				rest = JOB_OFF;
			}
			else
			{
				watch = 2;
				if (stronghold)
					watch = 3;

				AddChore(after, afterArgs, phase, true, false, false);
				AddSlot(after, afterArgs, JOB_SIT, -1);
				AddSlot(after, afterArgs, JOB_SIT, -1);
				AddSlot(after, afterArgs, JOB_WORK, -1);
				rest = JOB_WORK;
			}
		}

		array<int> order = {};
		if (p.m_People)
			p.m_People.Order(p.m_eFaction, night, order);

		// Wary: every post manned and a second at the gate. Recently hit: the gate doubled
		if (wary && !order.IsEmpty())
		{
			watch = Math.Max(watch, order.Count());
			if (p.m_People.CountPosts(RZ_SitePost.GATE) + p.m_People.CountPosts(RZ_SitePost.NEST) < 2)
				watch++;
		}

		if (hit)
			watch++;

		for (int i = 0; i < watch; i++)
		{
			int post = -1;
			if (!order.IsEmpty())
				post = order[i % order.Count()];

			AddSlot(slots, args, JOB_WATCH, post);
		}

		// Recently hit: the threat-side walk every shift by day
		if (hit && byDay && !threatWalk && p.m_People && p.m_People.Route(RZ_SiteRoute.THREAT))
			AddSlot(slots, args, JOB_PATROL, RZ_SiteRoute.THREAT);

		foreach (int k, int job : after)
		{
			int arg = afterArgs[k];
			if (wary)
			{
				// Nobody outside the wire; no chores from dusk
				if (job == JOB_STROLL || job == JOB_COURIER)
				{
					job = JOB_SIT;
					arg = -1;
				}

				if (job == JOB_PATROL)
					arg = RZ_SiteRoute.WIRE;

				if (job == JOB_COOK)
					continue;
			}

			AddSlot(slots, args, job, arg);
		}

		return rest;
	}

	//------------------------------------------------------------------------------------------------
	protected static void AddSlot(notnull array<int> slots, notnull array<int> args, int job, int arg)
	{
		slots.Insert(job);
		args.Insert(arg);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool HasLookout(RZ_CampPlace p)
	{
		return p.m_People && p.m_People.CountPosts(RZ_SitePost.LOOKOUT) > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The season's curfew (Bastion's season: RZ_Seasons): Bastion keeps two on watch at night.
	protected static bool IsCurfew()
	{
		RZ_SeasonMods mods = RZ_Seasons.Mods();
		return mods && mods.m_bCurfew;
	}

	//------------------------------------------------------------------------------------------------
	//! The phase's one chore: a wash in the morning, wood by day, the pot before the meal.
	protected static void AddChore(notnull array<int> slots, notnull array<int> args, int phase, bool wash, bool chop, bool cook)
	{
		if (wash && phase == RZ_Territory.PHASE_MORNING)
			AddSlot(slots, args, JOB_WASH, -1);
		else if (chop && phase == RZ_Territory.PHASE_DAY)
			AddSlot(slots, args, JOB_CHOP, -1);
		else if (cook && phase == RZ_Territory.PHASE_EVENING)
			AddSlot(slots, args, JOB_COOK, -1);
	}

	//------------------------------------------------------------------------------------------------
	//! Named people never walk away: no watch, patrol, stroll, errand or bed (but under attack they
	//! go indoors).
	protected static bool PersonMay(RZ_CampPlace p, int job)
	{
		if (job == JOB_OFF && p.m_iAlert == RZ_SitePeople.ALERT_ALERT)
			return true;

		return job == JOB_SIT || job == JOB_WORK || job == JOB_GATHER || job == JOB_LEAN || job == JOB_COOK || job == JOB_WASH || job == JOB_CHOP;
	}

	//------------------------------------------------------------------------------------------------
	protected void AssignAll(RZ_CampPlace p, int phase)
	{
		foreach (RZ_CampWorker w : p.m_aWorkers)
		{
			ClearJob(p, w);
		}

		// Guards and extras in shift order (each shift change, someone else takes the watch; at the
		// handover the watch going off comes last), then the named people
		array<RZ_CampWorker> crew = {};
		array<RZ_CampWorker> people = {};
		foreach (RZ_CampWorker c : p.m_aWorkers)
		{
			if (!IsWorking(c))
				continue;

			if (c.m_iRole == ROLE_PERSON)
				people.Insert(c);
			else
				crew.Insert(c);
		}

		array<RZ_CampWorker> order = {};
		array<RZ_CampWorker> going = {};
		int n = crew.Count();
		for (int i = 0; i < n; i++)
		{
			RZ_CampWorker next = crew[(i + p.m_iShift) % n];
			if (next.m_bWasOnWatch)
				going.Insert(next);
			else
				order.Insert(next);
		}

		order.InsertAll(going);
		order.InsertAll(people);
		array<int> roles = {};
		foreach (RZ_CampWorker o : order)
		{
			roles.Insert(o.m_iRole);
		}

		array<int> jobs = {};
		array<int> args = {};
		FillPlan(p, phase, roles, jobs, args);
		foreach (int k, RZ_CampWorker who : order)
		{
			// The handover: off the post to the fire
			int job = jobs[k];
			if (who.m_bWasOnWatch && job == JOB_WORK)
				job = JOB_SIT;

			who.m_bWasOnWatch = false;
			Give(p, who, job, args[k]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The routine's jobs for people by role, in the order they get them (the crew by shift, then
	//! the named people): each slot to the first who may take it, then the job for the rest by turns.
	//! Data only: AssignAll gives them, the audit's dry run counts them. How many slots were filled.
	int FillPlan(RZ_CampPlace p, int phase, notnull array<int> roles, notnull array<int> jobs, notnull array<int> args)
	{
		array<int> slots = {};
		array<int> slotArgs = {};
		int rest = Slots(p, phase, slots, slotArgs);
		array<bool> taken = {};
		foreach (int role : roles)
		{
			jobs.Insert(JOB_NONE);
			args.Insert(-1);
			taken.Insert(false);
		}

		int filled = 0;
		foreach (int k, int job : slots)
		{
			foreach (int i, int r : roles)
			{
				if (taken[i])
					continue;

				if (r == ROLE_PERSON && !PersonMay(p, job))
					continue;

				jobs[i] = job;
				args[i] = slotArgs[k];
				taken[i] = true;
				filled++;
				break;
			}
		}

		int index = 0;
		foreach (int j, int who : roles)
		{
			if (taken[j])
				continue;

			jobs[j] = RestJob(p, who, rest, index);
			args[j] = -1;
			if (jobs[j] == JOB_WATCH)
				args[j] = ARG_LINE + index;

			index++;
		}

		return filled;
	}

	//------------------------------------------------------------------------------------------------
	//! The job for everyone after the routine's list: sitting and working by turns (recently hit,
	//! fewer at the fire); a named person who would go to bed or to the wire stays at work, or under
	//! attack goes indoors.
	protected int RestJob(RZ_CampPlace p, int role, int rest, int index)
	{
		if (rest == JOB_WATCH && role == ROLE_PERSON)
			return JOB_OFF;

		if (role == ROLE_PERSON && !PersonMay(p, rest))
			return JOB_WORK;

		if (p.m_iAlert == RZ_SitePeople.ALERT_HIT && rest == JOB_SIT)
		{
			if (index % 3 == 2)
				return JOB_SIT;

			return JOB_WORK;
		}

		if (rest == JOB_SIT && index % 2 == 1)
			return JOB_WORK;

		if (rest == JOB_WORK && index % 2 == 1)
			return JOB_SIT;

		return rest;
	}

	//------------------------------------------------------------------------------------------------
	//! A group to give waypoints to: the unit if it's a group, its own group, or a new wrapper.
	protected IEntity GroupFor(RZ_CampWorker w)
	{
		if (w.m_Group)
			return w.m_Group;

		if (!w.m_Unit || w.m_bTriedGroup)
			return null;

		w.m_bTriedGroup = true;
		if (SCR_AIGroup.Cast(w.m_Unit))
		{
			w.m_Group = w.m_Unit;
			return w.m_Group;
		}

		w.m_Group = RZ_Util.GroupOf(w.m_Unit);
		if (w.m_Group)
			return w.m_Group;

		w.m_Group = RZ_Util.WrapInGroup(w.m_Unit, m_Settings.m_sGroupPrefab);
		w.m_bWrapped = w.m_Group != null;
		return w.m_Group;
	}

	//------------------------------------------------------------------------------------------------
	protected void Give(RZ_CampPlace p, RZ_CampWorker w, int job, int arg = -1)
	{
		ClearJob(p, w);
		w.m_iJob = job;
		w.m_iArg = arg;
		IEntity group = GroupFor(w);
		if (!group)
			return;

		bool person = w.m_iRole == ROLE_PERSON;
		if (job == JOB_WATCH)
			Watch(p, w, group, arg);
		else if (job == JOB_PATROL)
			Patrol(p, w, group, arg);
		else if (job == JOB_SIT)
			AtPiece(p, w, group, RZ_EDressUse.SIT, person);
		else if (job == JOB_WORK)
		{
			if (arg != ARG_WATER || !AtNamed(p, w, group, "water", person))
				AtPiece(p, w, group, RZ_EDressUse.WORK, person);
		}
		else if (job == JOB_LEAN)
			AtPiece(p, w, group, RZ_EDressUse.LEAN, person);
		else if (job == JOB_COOK)
			AtPiece(p, w, group, RZ_EDressUse.COOK, person);
		else if (job == JOB_WASH)
			AtPiece(p, w, group, RZ_EDressUse.WASH, person);
		else if (job == JOB_CHOP)
			AtPiece(p, w, group, RZ_EDressUse.CHOP, person);
		else if (job == JOB_GATHER)
			Gather(p, w, group);
		else if (job == JOB_STROLL)
			Errand(p, w, group, false, arg);
		else if (job == JOB_COURIER)
			Errand(p, w, group, true, arg);
		else if (job == JOB_OFF)
			ToBed(p, w, group);
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearJob(RZ_CampPlace p, RZ_CampWorker w)
	{
		foreach (IEntity wp : w.m_aWaypoints)
		{
			RZ_Util.PopGroupWaypoint(w.m_Group, wp);
		}

		w.m_aWaypoints.Clear();
		if (w.m_iPiece >= 0 && w.m_iPiece < p.m_aPieces.Count())
		{
			RZ_CampPiece piece = p.m_aPieces[w.m_iPiece];
			piece.m_iUsed = Math.Max(0, piece.m_iUsed - 1);
		}

		w.m_iPiece = -1;
		w.m_iSeat = -1;
		w.m_iHow = HOW_NONE;
		w.m_iJob = JOB_NONE;
		w.m_iArg = -1;
	}

	//------------------------------------------------------------------------------------------------
	//! A free piece of this use (named people: only near the centre), or -1. A seat beside someone
	//! already sitting comes first, so two off duty sit together.
	protected int FreePiece(RZ_CampPlace p, RZ_EDressUse use, bool person)
	{
		int first = -1;
		foreach (int i, RZ_CampPiece piece : p.m_aPieces)
		{
			if (!piece.m_Entity || piece.m_eUse != use || piece.m_iUsed >= piece.m_iSeats)
				continue;

			if (person && RZ_Util.DistXZ(piece.m_vPos, p.m_vHome) > p.m_fPersonRadius)
				continue;

			if (use == RZ_EDressUse.SIT && piece.m_iUsed == 1 && piece.m_iSeats >= 2)
				return i;

			if (first < 0)
				first = i;
		}

		return first;
	}

	//------------------------------------------------------------------------------------------------
	//! Work at a piece whose name has `word` in it (the water barrel): its seat if free, else beside
	//! it. False if there's none.
	protected bool AtNamed(RZ_CampPlace p, RZ_CampWorker w, IEntity group, string word, bool person)
	{
		foreach (int i, RZ_CampPiece piece : p.m_aPieces)
		{
			if (!piece.m_Entity || !piece.m_sName.Contains(word))
				continue;

			if (person && RZ_Util.DistXZ(piece.m_vPos, p.m_vHome) > p.m_fPersonRadius)
				continue;

			if (piece.m_eUse != RZ_EDressUse.NONE && piece.m_iUsed < piece.m_iSeats)
			{
				UsePiece(p, w, group, i);
				return true;
			}

			Loiter(p, w, group, RZ_Util.RandomPointInRing(piece.m_vPos, 1.2, 2));
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Sit, work, lean, cook, wash or chop at a piece: the use asked for, else a seat, work or lean
	//! point, else a spot near the centre.
	protected void AtPiece(RZ_CampPlace p, RZ_CampWorker w, IEntity group, RZ_EDressUse use, bool person)
	{
		int idx = FreePiece(p, use, person);
		if (idx < 0 && use != RZ_EDressUse.SIT)
			idx = FreePiece(p, RZ_EDressUse.SIT, person);
		if (idx < 0 && use != RZ_EDressUse.WORK)
			idx = FreePiece(p, RZ_EDressUse.WORK, person);
		if (idx < 0 && use != RZ_EDressUse.LEAN)
			idx = FreePiece(p, RZ_EDressUse.LEAN, person);

		if (idx < 0)
		{
			Potter(p, w, group, person);
			return;
		}

		UsePiece(p, w, group, idx);
	}

	//------------------------------------------------------------------------------------------------
	//! Nothing free to sit or work at: they walk round the camp instead, between the places people
	//! idle (the fire, the bench, the table, a work point: never a wire run, a board or the gate's
	//! lane), standing a while at each (loiter waypoints in a cycle; a named person only round the
	//! pieces near home). With fewer than two such pieces standing, they stand about near home.
	protected void Potter(RZ_CampPlace p, RZ_CampWorker w, IEntity group, bool person)
	{
		array<int> near = {};
		foreach (int i, RZ_CampPiece piece : p.m_aPieces)
		{
			if (!piece.m_Entity || piece.m_bStory || piece.m_eUse == RZ_EDressUse.NONE || piece.m_eUse == RZ_EDressUse.WATCH || piece.m_eUse == RZ_EDressUse.SLEEP)
				continue;

			if (person && RZ_Util.DistXZ(piece.m_vPos, p.m_vHome) > p.m_fPersonRadius)
				continue;

			if (p.m_People && RZ_Util.DistXZ(piece.m_vPos, p.m_People.m_vGate) < 8)
				continue;

			near.Insert(i);
		}

		RZ_Rng rng = new RZ_Rng(p.m_iSeed + p.m_iShift * 13 + p.m_aWorkers.Find(w) * 7 + 5);
		array<vector> stops = {};
		for (int k = 0; k < POTTER_STOPS && !near.IsEmpty(); k++)
		{
			int pick = RZ_Util.PickIndex(rng.Next(), near.Count());
			RZ_CampPiece at = p.m_aPieces[near[pick]];
			near.Remove(pick);
			stops.Insert(StandClear(p, RZ_Util.RandomPointInRing(at.m_vPos, 2.5, 3.5)));
		}

		ResourceName prefab = m_Settings.m_sLoiterWaypoint;
		if (prefab.IsEmpty())
			prefab = m_GM.GetSpawner().GetMoveWaypointPrefab();

		array<IEntity> loop = {};
		foreach (vector stop : stops)
		{
			IEntity spawned = RZ_Util.SpawnPrefab(prefab, stop);
			AIWaypoint wp = AIWaypoint.Cast(spawned);
			if (wp)
			{
				wp.SetCompletionRadius(3);
				loop.Insert(spawned);
			}
			else if (spawned)
			{
				SCR_EntityHelper.DeleteEntityAndChildren(spawned);
			}
		}

		if (loop.Count() < 2)
		{
			foreach (IEntity l : loop)
			{
				SCR_EntityHelper.DeleteEntityAndChildren(l);
			}

			Loiter(p, w, group, RZ_Util.RandomPointInRing(p.m_vHome, 4, 10));
			return;
		}

		w.m_aWaypoints.InsertAll(loop);
		w.m_iHow = HOW_WALK;
		IEntity cycle = RZ_Util.AddGroupCycle(group, loop, m_Settings.m_sCycleWaypoint); // VERIFY: a cycle of loiter waypoints walks the group from piece to piece, standing a while at each (a loiter waypoint completes after its hold), round and round
		if (cycle)
		{
			w.m_aWaypoints.Insert(cycle);
			return;
		}

		SCR_AIGroup g = SCR_AIGroup.Cast(group);
		foreach (IEntity each : loop)
		{
			AIWaypoint a = AIWaypoint.Cast(each);
			if (g && a)
				g.AddWaypoint(a);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UsePiece(RZ_CampPlace p, RZ_CampWorker w, IEntity group, int idx)
	{
		RZ_CampPiece piece = p.m_aPieces[idx];
		piece.m_iUsed++;
		w.m_iPiece = idx;

		// A seat of its own: the sentinel itself, so the waypoint points at a real smart action
		IEntity seat = FreeSeat(p, w, idx);
		IEntity wp;
		if (seat)
			wp = RZ_Util.SetGroupSmartAction(group, seat, piece.m_sTag, m_Settings.m_sSmartActionWaypoint);

		if (wp)
		{
			w.m_aWaypoints.Insert(wp);
			w.m_iHow = HOW_SEAT;
			return;
		}

		w.m_iSeat = -1;
		Loiter(p, w, group, RZ_Util.RandomPointInRing(piece.m_vPos, 1.5, 3));
	}

	//------------------------------------------------------------------------------------------------
	//! The first of the piece's sentinels with its tag that nobody else at the place holds, or null.
	protected IEntity FreeSeat(RZ_CampPlace p, RZ_CampWorker w, int idx)
	{
		RZ_CampPiece piece = p.m_aPieces[idx];
		if (!piece.m_Entity)
			return null;

		array<int> held = {};
		foreach (RZ_CampWorker o : p.m_aWorkers)
		{
			if (o != w && o.m_iPiece == idx && o.m_iSeat >= 0)
				held.Insert(o.m_iSeat);
		}

		for (int s = 0; s < 8; s++)
		{
			if (held.Contains(s))
				continue;

			IEntity seat = RZ_Util.FindSmartAction(piece.m_Entity, piece.m_sTag, s);
			if (!seat)
				return null;

			w.m_iSeat = s;
			return seat;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Stand about at a spot (kept off the walls): the loiter waypoint, else a plain move there.
	protected void Loiter(RZ_CampPlace p, RZ_CampWorker w, IEntity group, vector at)
	{
		ResourceName prefab = m_Settings.m_sLoiterWaypoint;
		if (prefab.IsEmpty())
			prefab = m_GM.GetSpawner().GetMoveWaypointPrefab();

		IEntity wp = RZ_Util.AddGroupWaypoint(group, StandClear(p, at), prefab, 4); // VERIFY: a loiter (timed defend) waypoint with a 4 m radius uses only the smart actions within it, and holds the group there
		if (wp)
		{
			w.m_aWaypoints.Insert(wp);
			w.m_iHow = HOW_SPOT;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void MoveTo(RZ_CampWorker w, IEntity group, vector at)
	{
		IEntity wp = RZ_Util.AddGroupWaypoint(group, at, m_GM.GetSpawner().GetMoveWaypointPrefab(), 3);
		if (wp)
		{
			w.m_aWaypoints.Insert(wp);
			w.m_iHow = HOW_MOVE;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A spot someone stands at (a loiter, a lookout), out of the place's building and WALL_CLEAR off
	//! every wall, so a lean there keeps its head out of the wall.
	protected vector StandClear(RZ_CampPlace p, vector at)
	{
		vector spot = at;
		if (p.m_Site)
			spot = OutOfBox(p.m_Site, spot, WALL_CLEAR);

		return OffWalls(spot, WALL_CLEAR);
	}

	//------------------------------------------------------------------------------------------------
	//! A spot inside the building's box (grown by `margin`) moves out through its nearest side.
	protected static vector OutOfBox(RZ_CampSite site, vector at, float margin)
	{
		vector d = at - site.m_vOrigin;
		vector local = Vector(vector.Dot(d, site.m_vRight), vector.Dot(d, site.m_vUp), vector.Dot(d, site.m_vFwd));
		float loX = site.m_vMins[0] - margin;
		float hiX = site.m_vMaxs[0] + margin;
		float loZ = site.m_vMins[2] - margin;
		float hiZ = site.m_vMaxs[2] + margin;
		if (local[0] <= loX || local[0] >= hiX || local[2] <= loZ || local[2] >= hiZ)
			return at;

		// The nearest side, and just past it
		float toLoX = local[0] - loX;
		float toHiX = hiX - local[0];
		float toLoZ = local[2] - loZ;
		float toHiZ = hiZ - local[2];
		float least = Math.Min(Math.Min(toLoX, toHiX), Math.Min(toLoZ, toHiZ));
		if (least == toLoX)
			local[0] = loX - 0.05;
		else if (least == toHiX)
			local[0] = hiX + 0.05;
		else if (least == toLoZ)
			local[2] = loZ - 0.05;
		else
			local[2] = hiZ + 0.05;

		return RZ_Util.GroundPos(LocalToWorld(site, local));
	}

	//------------------------------------------------------------------------------------------------
	//! The spot pushed back from any wall (a building's, a garden wall, a fence) closer than `clear`:
	//! flat traces eight ways at waist and head height, twice (for a corner).
	protected static vector OffWalls(vector at, float clear)
	{
		vector spot = RZ_Util.GroundPos(at);
		for (int pass = 0; pass < 2; pass++)
		{
			vector push = vector.Zero;
			for (int k = 0; k < 8; k++)
			{
				float rad = k * 45 * Math.DEG2RAD;
				vector dir = Vector(Math.Sin(rad), 0, Math.Cos(rad));
				float hitAt = Math.Min(WallDistance(spot, dir, 1.0, clear), WallDistance(spot, dir, 1.6, clear));
				if (hitAt < clear)
					push = push - dir * (clear - hitAt);
			}

			float pushLen = push.Length();
			if (pushLen < 0.01)
				break;

			if (pushLen > clear)
				push = push * (clear / pushLen);

			spot = RZ_Util.GroundPos(spot + push);
		}

		return spot;
	}

	//------------------------------------------------------------------------------------------------
	//! How far a flat trace from `spot` (at `height`) goes along `dir` before it meets a wall, up to
	//! `reach`. Characters don't count.
	protected static float WallDistance(vector spot, vector dir, float height, float reach)
	{
		TraceParam param = new TraceParam();
		param.Start = spot + Vector(0, height, 0);
		param.End = param.Start + dir * reach;
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		float frac = ChimeraCharacter.TraceMoveWithoutCharacters(GetGame().GetWorld(), param); // VERIFY: a flat trace this short at waist and head height stops on a building's wall, a garden wall and a fence, and a spawned camp piece too
		return Math.Clamp(frac, 0, 1) * reach;
	}

	//------------------------------------------------------------------------------------------------
	//! The sentry at a post (docs/SITES.md, "Posts"): its watch piece's own watch point if one stands
	//! there, else standing at the post (vanilla's observation post waypoint where it may, else the
	//! loiter waypoint). Under attack (ARG_LINE) on the wire facing the threat. Without posts (Solver
	//! off) the nest's watch point, else the building's lookout at a front corner, else the approach.
	protected void Watch(RZ_CampPlace p, RZ_CampWorker w, IEntity group, int arg = -1)
	{
		RZ_SitePeople people = p.m_People;
		if (people && arg >= ARG_LINE)
		{
			int n = arg - ARG_LINE;
			if (!people.m_aThreatLine.IsEmpty())
			{
				Loiter(p, w, group, people.m_aThreatLine[n % people.m_aThreatLine.Count()]);
				return;
			}

			if (!people.m_aPosts.IsEmpty())
			{
				StandWatch(p, w, group, people.m_aPosts[n % people.m_aPosts.Count()].m_vPos);
				return;
			}
		}

		if (people && arg >= 0 && arg < people.m_aPosts.Count())
		{
			RZ_SitePost post = people.m_aPosts[arg];
			if (post.m_bPiece)
			{
				int at = WatchPieceAt(p, post.m_vPos);
				if (at >= 0)
				{
					UsePiece(p, w, group, at);
					return;
				}
			}

			StandWatch(p, w, group, post.m_vPos);
			return;
		}

		int idx = FreePiece(p, RZ_EDressUse.WATCH, false);
		if (idx >= 0)
		{
			UsePiece(p, w, group, idx);
			return;
		}

		float reach = 18;
		if (p.m_eKind == RZ_ECampPlace.SAFEHOUSE)
			reach = 8;

		vector spot = RZ_Compass.PointAt(p.m_vPos, p.m_fApproach - 20 + Math.RandomFloat(0, 40), reach);
		if (p.m_Site)
		{
			int others = 0;
			foreach (RZ_CampWorker o : p.m_aWorkers)
			{
				if (o != w && o.m_iJob == JOB_WATCH && o.m_iPiece < 0)
					others++;
			}

			spot = LookoutSpot(p, others);
		}

		StandWatch(p, w, group, spot);
	}

	//------------------------------------------------------------------------------------------------
	//! A free watch piece standing at a post (within 2.5 m), or -1.
	protected int WatchPieceAt(RZ_CampPlace p, vector at)
	{
		foreach (int i, RZ_CampPiece piece : p.m_aPieces)
		{
			if (piece.m_Entity && piece.m_eUse == RZ_EDressUse.WATCH && piece.m_iUsed < piece.m_iSeats && RZ_Util.DistXZ(piece.m_vPos, at) < 2.5)
				return i;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Standing watch at a spot: vanilla's observation post waypoint (it needs an observation-post
	//! smart action near the spot: AddGroupWaypoint refuses it otherwise), else the loiter waypoint.
	protected void StandWatch(RZ_CampPlace p, RZ_CampWorker w, IEntity group, vector at)
	{
		ResourceName prefab = m_Settings.m_sObservationWaypoint;
		if (prefab.IsEmpty())
		{
			Loiter(p, w, group, at);
			return;
		}

		vector spot = StandClear(p, at);
		IEntity wp = RZ_Util.AddGroupWaypoint(group, spot, prefab, 3);
		if (!wp)
		{
			Loiter(p, w, group, at);
			return;
		}

		w.m_aWaypoints.Insert(wp);
		w.m_iHow = HOW_SPOT;
	}

	//------------------------------------------------------------------------------------------------
	//! A patrol (docs/SITES.md, "Patrols"): the site's route of that kind: round the inside of the
	//! wire in a cycle (its corners stand a while), or out and back (the approach, the threat side).
	//! Without the route, the wire's; without posts (Solver off), a loop of four points 40-80 m out.
	//! Without a cycle waypoint they walk it once and the next shuffle sends them round again.
	protected void Patrol(RZ_CampPlace p, RZ_CampWorker w, IEntity group, int arg = -1)
	{
		RZ_SiteRoute route;
		if (p.m_People && arg >= 0 && arg < RZ_SiteRoute.KINDS)
		{
			route = p.m_People.Route(arg);
			if (!route)
				route = p.m_People.Route(RZ_SiteRoute.WIRE);
		}

		if (route && route.m_iKind == RZ_SiteRoute.WIRE)
		{
			WalkLoop(p, w, group, route);
			return;
		}

		if (route)
		{
			WalkOut(p, w, group, route);
			return;
		}

		array<vector> points = {};
		PatrolPoints(p, p.m_iShift, points);
		array<bool> stops = {false, false, false, false};
		LoopThrough(p, w, group, points, stops);
	}

	//------------------------------------------------------------------------------------------------
	//! The wire's walk in a cycle: at most 16 of its points (evenly), its corners as loiter points.
	protected void WalkLoop(RZ_CampPlace p, RZ_CampWorker w, IEntity group, RZ_SiteRoute route)
	{
		array<vector> points = {};
		array<bool> stops = {};
		int count = route.m_aPoints.Count();
		int every = Math.Max(1, Math.Ceil(count / 16.0));
		for (int i = 0; i < count; i++)
		{
			if (i % every != 0 && !route.m_aStop[i])
				continue;

			points.Insert(route.m_aPoints[i]);
			stops.Insert(route.m_aStop[i]);
		}

		LoopThrough(p, w, group, points, stops);
	}

	//------------------------------------------------------------------------------------------------
	//! Patrol waypoints (loiter waypoints at the stops) in a cycle.
	protected void LoopThrough(RZ_CampPlace p, RZ_CampWorker w, IEntity group, array<vector> points, array<bool> stops)
	{
		ResourceName prefab = m_Settings.m_sPatrolWaypoint;
		if (prefab.IsEmpty())
			prefab = m_GM.GetSpawner().GetMoveWaypointPrefab();

		if (prefab.IsEmpty())
			return;

		ResourceName stopPrefab = m_Settings.m_sLoiterWaypoint;
		if (stopPrefab.IsEmpty())
			stopPrefab = prefab;

		array<IEntity> loop = {};
		foreach (int i, vector at : points)
		{
			ResourceName use = prefab;
			if (stops[i])
				use = stopPrefab;

			IEntity wp = RZ_Util.SpawnPrefab(use, at);
			if (wp)
				loop.Insert(wp);
		}

		if (loop.IsEmpty())
			return;

		w.m_aWaypoints.InsertAll(loop);
		w.m_iHow = HOW_MOVE;
		IEntity cycle = RZ_Util.AddGroupCycle(group, loop, m_Settings.m_sCycleWaypoint); // VERIFY: a cycle waypoint made from script loops the patrol for our groups
		if (cycle)
		{
			w.m_aWaypoints.Insert(cycle);
			return;
		}

		SCR_AIGroup g = SCR_AIGroup.Cast(group);
		foreach (IEntity l : loop)
		{
			AIWaypoint a = AIWaypoint.Cast(l);
			if (g && a)
				g.AddWaypoint(a);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A walk out and back along a route (at most 6 points out, evenly), then home.
	protected void WalkOut(RZ_CampPlace p, RZ_CampWorker w, IEntity group, RZ_SiteRoute route)
	{
		array<vector> legs = {};
		int count = route.m_aPoints.Count();
		int every = Math.Max(1, Math.Ceil(count / 6.0));
		for (int i = 0; i < count; i++)
		{
			if (i % every == 0 || i == count - 1)
				legs.Insert(route.m_aPoints[i]);
		}

		foreach (vector there : legs)
		{
			MoveTo(w, group, there);
		}

		for (int k = legs.Count() - 2; k >= 0; k--)
		{
			MoveTo(w, group, legs[k]);
		}

		MoveTo(w, group, RZ_Util.RandomPointInRing(p.m_vHome, 4, 10));
	}

	//------------------------------------------------------------------------------------------------
	//! The patrol's loop on a shift: four points 40-80 m out round the centre (the audit checks them).
	static void PatrolPoints(RZ_CampPlace p, int shift, notnull array<vector> points)
	{
		RZ_Rng rng = new RZ_Rng(p.m_iSeed + shift * 101 + 3);
		for (int i = 0; i < 4; i++)
		{
			points.Insert(RZ_Compass.PointAt(p.m_vCentre, p.m_fApproach + i * 90 + rng.Range(-20, 20), rng.Range(40, 80)));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The roll or the meal: at the board (one at a time by day), else at the centre's table or fire.
	protected void Gather(RZ_CampPlace p, RZ_CampWorker w, IEntity group)
	{
		int idx = FreePiece(p, RZ_EDressUse.GATHER, false);
		if (idx >= 0)
		{
			UsePiece(p, w, group, idx);
			return;
		}

		vector at = p.m_vHome;
		bool haveSpot = false;
		foreach (RZ_CampPiece piece : p.m_aPieces)
		{
			if (piece.m_Entity && (piece.m_eUse == RZ_EDressUse.GATHER || (piece.m_eUse == RZ_EDressUse.SIT && piece.m_eRole == RZ_EDressRole.CENTRE)))
			{
				at = piece.m_vPos;
				haveSpot = true;
				break;
			}
		}

		// The table's own seats first (the meal), then standing round it
		if (haveSpot)
		{
			int seat = FreePiece(p, RZ_EDressUse.SIT, w.m_iRole == ROLE_PERSON);
			if (seat >= 0 && RZ_Util.DistXZ(p.m_aPieces[seat].m_vPos, at) < 1)
			{
				UsePiece(p, w, group, seat);
				return;
			}
		}

		Loiter(p, w, group, RZ_Util.RandomPointInRing(at, 2, 4));
	}

	//------------------------------------------------------------------------------------------------
	//! A walk out and back (docs/SITES.md, "Patrols" and "Everyday movement"): the site's route of
	//! that kind (the Red Hand's stroll out along the road in, Halcyon's courier toward the friendly
	//! side, the road post run), the latrine or the stores door and back; without them, as before:
	//! the stroll to the gate or the road, the courier 60-90 m out to the edge.
	protected void Errand(RZ_CampPlace p, RZ_CampWorker w, IEntity group, bool courier, int arg = -1)
	{
		RZ_SitePeople people = p.m_People;
		if (people && arg >= 0 && arg < RZ_SiteRoute.KINDS)
		{
			RZ_SiteRoute route = people.Route(arg);
			if (route)
			{
				WalkOut(p, w, group, route);
				return;
			}
		}

		if (arg == ARG_LATRINE)
		{
			foreach (RZ_CampPiece piece : p.m_aPieces)
			{
				if (piece.m_Entity && piece.m_sName.Contains("Latrine"))
				{
					MoveTo(w, group, StandClear(p, RZ_Util.RandomPointInRing(piece.m_vPos, 2, 3)));
					MoveTo(w, group, RZ_Util.RandomPointInRing(p.m_vHome, 4, 10));
					return;
				}
			}
		}

		if (arg == ARG_STORES && people)
		{
			MoveTo(w, group, StandClear(p, RZ_Util.RandomPointInRing(people.m_vDoor, 1, 2)));
			MoveTo(w, group, RZ_Util.RandomPointInRing(p.m_vHome, 4, 10));
			return;
		}

		vector away;
		if (courier)
			away = RZ_Compass.PointAt(p.m_vCentre, Math.RandomFloat(0, 360), Math.RandomFloat(60, 90));
		else if (p.m_Site && p.m_Site.m_iMain >= 0)
			away = p.m_Site.m_aEntries[p.m_Site.m_iMain];
		else if (p.m_bHasRoad && RZ_Util.DistXZ(p.m_vRoad, p.m_vPos) < 120)
			away = p.m_vRoad;
		else
			away = RZ_Compass.PointAt(p.m_vCentre, p.m_fApproach, 40);

		MoveTo(w, group, away);
		MoveTo(w, group, RZ_Util.RandomPointInRing(p.m_vHome, 4, 10));
	}

	//------------------------------------------------------------------------------------------------
	//! Off shift: to the tents (in a building, in at the front door). They sleep (despawn) once
	//! nobody is close.
	protected void ToBed(RZ_CampPlace p, RZ_CampWorker w, IEntity group)
	{
		if (p.m_Site)
		{
			MoveTo(w, group, DoorSpot(p));
			return;
		}

		foreach (RZ_CampPiece piece : p.m_aPieces)
		{
			if (piece.m_Entity && piece.m_eUse == RZ_EDressUse.SLEEP)
			{
				MoveTo(w, group, RZ_Util.RandomPointInRing(piece.m_vPos, 2, 4));
				return;
			}
		}

		MoveTo(w, group, RZ_Util.RandomPointInRing(p.m_vPos, 10, 16));
	}

	//------------------------------------------------------------------------------------------------
	//! Where people come out in the morning: the building's front door, a tent, else the inner ring.
	protected vector BedSpot(RZ_CampPlace p)
	{
		if (p.m_Site)
			return RZ_Mission.LandPointNear(DoorSpot(p), 0.5, 2);

		foreach (RZ_CampPiece piece : p.m_aPieces)
		{
			if (piece.m_Entity && piece.m_eUse == RZ_EDressUse.SLEEP)
				return RZ_Mission.LandPointNear(piece.m_vPos, 2, 4);
		}

		return RZ_Mission.LandPointNear(p.m_vPos, 10, 16);
	}

	//------------------------------------------------------------------------------------------------
	//! Night, nobody within 150 m: the off-shift guards and extras out of sight go to sleep (as data).
	protected void PutToBed(RZ_CampPlace p, array<IEntity> players)
	{
		for (int i = p.m_aWorkers.Count() - 1; i >= 0; i--)
		{
			RZ_CampWorker w = p.m_aWorkers[i];
			if (w.m_iJob != JOB_OFF || !IsWorking(w) || (w.m_iRole != ROLE_GUARD && w.m_iRole != ROLE_EXTRA))
				continue;

			// Someone could see them go: they turn in at a later look
			if (RZ_InfectedBodies.InSight(RZ_Util.GetUnitPosition(w.m_Unit), players, true))
				continue;

			ClearJob(p, w);
			if (w.m_bWrapped)
				RZ_Util.UnwrapGroup(w.m_Unit, w.m_Group);

			RZ_Util.DeleteUnit(w.m_Unit);
			if (w.m_iRole == ROLE_EXTRA)
				p.m_iAsleepExtras++;
			else
				p.m_iAsleepGuards++;

			p.m_aWorkers.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Morning (any phase but night), nobody within 150 m or in sight: the sleepers come out of the tents.
	protected void Wake(RZ_CampPlace p, int phase)
	{
		int total = p.m_iAsleepGuards + p.m_iAsleepExtras;
		if (total == 0)
			return;

		int extras = p.m_iAsleepExtras;
		p.m_iAsleepGuards = 0;
		p.m_iAsleepExtras = 0;
		if (p.m_sExtraPrefab.IsEmpty())
			return;

		for (int i = 0; i < total; i++)
		{
			IEntity body = SpawnOwned(p, i < extras);
			if (!body)
				break;
		}

		AssignAll(p, phase);
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity SpawnOwned(RZ_CampPlace p, bool extra)
	{
		// One of the slot's looks, by the place and the row (docs/DESIGN.md, People's looks)
		IEntity body = RZ_Util.SpawnPrefab(m_GM.PickLook(p.m_sExtraPrefab, p.m_iSeed + p.m_aWorkers.Count()), BedSpot(p), Math.RandomFloat(0, 360));
		if (!body)
			return null;

		RZ_Hostility hostility = m_GM.GetHostility();
		if (hostility)
			hostility.WatchUnit(body, p.m_eFaction);

		RZ_CampWorker w = new RZ_CampWorker();
		w.m_Unit = body;
		w.m_bOwned = true;
		w.m_iRole = ROLE_GUARD;
		if (extra)
			w.m_iRole = ROLE_EXTRA;

		p.m_aWorkers.Insert(w);
		return body;
	}

	//------------------------------------------------------------------------------------------------
	protected int CountExtras()
	{
		int n = 0;
		foreach (RZ_CampPlace p : m_aPlaces)
		{
			n += p.m_iAsleepExtras;
			foreach (RZ_CampWorker w : p.m_aWorkers)
			{
				if (w.m_iRole == ROLE_EXTRA && w.m_Unit)
					n++;
			}
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! How many people a place has in play (the faction bases' counts): Bastion's school 12, its
	//! outposts 6; the Red Hand's garage 10, a crew camp 4; the Haven's church 8, an outpost 5, a
	//! safehouse 3; Halcyon's lab 6, a field station 2. Extras fill up to it, within their caps.
	static int PeopleInPlay(RZ_EFaction f, RZ_ECampPlace kind)
	{
		bool stronghold = kind == RZ_ECampPlace.STRONGHOLD;
		if (f == RZ_EFaction.MILITARY)
		{
			if (stronghold)
				return 12;

			return 6;
		}

		if (f == RZ_EFaction.BANDITS)
		{
			if (stronghold)
				return 10;

			return 4;
		}

		if (f == RZ_EFaction.SURVIVORS)
		{
			if (stronghold)
				return 8;

			if (kind == RZ_ECampPlace.SAFEHOUSE)
				return 3;

			return 5;
		}

		if (stronghold)
			return 6;

		return 2;
	}

	//------------------------------------------------------------------------------------------------
	//! People at a place now: the living members of every body that takes jobs, and the sleepers.
	protected int CountPeople(RZ_CampPlace p)
	{
		int n = p.m_iAsleepGuards + p.m_iAsleepExtras;
		foreach (RZ_CampWorker w : p.m_aWorkers)
		{
			if (w.m_iRole != ROLE_KEEPER && w.m_Unit)
				n += RZ_Util.CountAlive(w.m_Unit);
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! One more person at a live place by day (off-shift workers, the courier), out of sight.
	protected void AddExtra(RZ_CampPlace p, int phase)
	{
		if (p.m_eKind == RZ_ECampPlace.ROAD_POST || p.m_sExtraPrefab.IsEmpty())
			return;

		int here = p.m_iAsleepExtras;
		foreach (RZ_CampWorker w : p.m_aWorkers)
		{
			if (w.m_iRole == ROLE_EXTRA && w.m_Unit)
				here++;
		}

		if (here >= m_Settings.m_iExtrasPerPlace || CountExtras() >= m_Settings.m_iExtrasMax)
			return;

		// Sized to the place: no extra where its people are already as many as it has in play
		if (CountPeople(p) >= PeopleInPlay(p.m_eFaction, p.m_eKind))
			return;

		if (SpawnOwned(p, true))
			AssignAll(p, phase);
	}

	//------------------------------------------------------------------------------------------------
	//! Halcyon's courier: the extra if there is one, else a guard (off the door if one is free)
	//! walks out toward the friendly side and back, then goes back to the job it had.
	protected void SendCourier(RZ_CampPlace p)
	{
		if (p.m_iAlert == RZ_SitePeople.ALERT_WARY || p.m_iAlert == RZ_SitePeople.ALERT_ALERT)
			return;

		RZ_CampWorker pick;
		foreach (RZ_CampWorker w : p.m_aWorkers)
		{
			if (!IsWorking(w) || w.m_iRole == ROLE_PERSON || w.m_iJob == JOB_COURIER)
				continue;

			if (!pick || w.m_iRole == ROLE_EXTRA || (pick.m_iRole != ROLE_EXTRA && pick.m_iJob == JOB_WATCH))
				pick = w;
		}

		if (!pick)
			return;

		SendOff(p, pick, JOB_COURIER, RZ_SiteRoute.FRIENDLY);
	}

	//------------------------------------------------------------------------------------------------
	//! Someone off on a walk, back to the job they had once it's walked.
	protected void SendOff(RZ_CampPlace p, RZ_CampWorker w, int job, int arg)
	{
		int after = w.m_iJob;
		int afterArg = w.m_iArg;
		Give(p, w, job, arg);
		w.m_iAfterErrand = after;
		w.m_iAfterArg = afterArg;
	}

	//------------------------------------------------------------------------------------------------
	//! Every few minutes: anyone idle (back from a walk, done talking, out of a fight) goes back to
	//! work, now and then one goes to the latrine or the stores and back, and a third of the seated
	//! and working move on.
	protected void Shuffle(RZ_CampPlace p, int phase)
	{
		array<int> unused = {};
		array<int> unusedArgs = {};
		int rest = Slots(p, phase, unused, unusedArgs);
		array<RZ_CampWorker> movers = {};
		foreach (int i, RZ_CampWorker w : p.m_aWorkers)
		{
			if (!IsWorking(w))
				continue;

			bool idle = w.m_iJob == JOB_NONE || (w.m_Group && RZ_Util.CountGroupWaypoints(w.m_Group) == 0 && w.m_iJob != JOB_OFF);
			if (idle)
			{
				// A walk out and back goes round again; a walk sent off goes back to the job before it
				int again = w.m_iJob;
				int againArg = w.m_iArg;
				if (again == JOB_COURIER || (again == JOB_STROLL && w.m_iAfterErrand != JOB_NONE))
				{
					again = w.m_iAfterErrand;
					againArg = w.m_iAfterArg;
					w.m_iAfterErrand = JOB_NONE;
				}

				if (again == JOB_NONE || again == JOB_COURIER)
				{
					again = RestJob(p, w.m_iRole, rest, i);
					againArg = -1;
					if (again == JOB_WATCH)
						againArg = ARG_LINE + i;
				}

				Give(p, w, again, againArg);
				continue;
			}

			if (p.m_fGatherUntil == 0 && (w.m_iJob == JOB_SIT || w.m_iJob == JOB_WORK || w.m_iJob == JOB_LEAN))
				movers.Insert(w);
		}

		SmallWalk(p, phase, movers);

		// A third of the seated and working move on (at least one): sitting to work, work to a seat,
		// and anyone without a free piece walks the camp
		int moves = Math.Max(1, movers.Count() / 3);
		for (int m = 0; m < moves && !movers.IsEmpty(); m++)
		{
			int pick = Math.RandomInt(0, movers.Count());
			RZ_CampWorker mover = movers[pick];
			movers.Remove(pick);
			int next = JOB_SIT;
			if (mover.m_iJob == JOB_SIT)
				next = JOB_WORK;

			Give(p, mover, next);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Now and then (one shuffle in three, calm, not at night) one of the crew off a seat or work
	//! goes to the latrine and back, or at a building to the stores door and back.
	protected void SmallWalk(RZ_CampPlace p, int phase, array<RZ_CampWorker> movers)
	{
		if (movers.IsEmpty() || p.m_iAlert != RZ_SitePeople.ALERT_CALM || phase == RZ_Territory.PHASE_NIGHT || Math.RandomInt(0, SMALL_WALK_EVERY) != 0)
			return;

		bool latrine = false;
		foreach (RZ_CampPiece piece : p.m_aPieces)
		{
			if (piece.m_Entity && piece.m_sName.Contains("Latrine"))
				latrine = true;
		}

		bool stores = p.m_Site != null && p.m_People != null;
		if (p.m_bHand)
			stores = p.m_People != null && p.m_People.CountPosts(RZ_SitePost.DOOR) > 0;
		if (!latrine && !stores)
			return;

		int arg = ARG_LATRINE;
		if (!latrine || (stores && Math.RandomInt(0, 2) == 0))
			arg = ARG_STORES;

		foreach (int i, RZ_CampWorker w : movers)
		{
			if (w.m_iRole == ROLE_PERSON)
				continue;

			movers.Remove(i);
			SendOff(p, w, JOB_STROLL, arg);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The road post run (docs/SITES.md, "Patrols"): Bastion and the Red Hand, once a season day by
	//! day, calm, one of the crew off the posts (an extra first) walks to the faction's nearest road
	//! post and back; one at a time on the server, and it counts toward the extras' cap.
	protected void RoadPostRun(RZ_CampPlace p, int phase)
	{
		if (!m_Settings.m_bRoadPostRun || !p.m_People || p.m_iAlert != RZ_SitePeople.ALERT_CALM)
			return;

		if (p.m_eFaction != RZ_EFaction.MILITARY && p.m_eFaction != RZ_EFaction.BANDITS)
			return;

		int today = RZ_SeasonClock.Day();
		RZ_SiteRoute route = p.m_People.Route(RZ_SiteRoute.ROAD_POST);
		if (p.m_iRunDay == today || !route)
			return;

		int running = 0;
		foreach (RZ_CampPlace q : m_aPlaces)
		{
			foreach (RZ_CampWorker r : q.m_aWorkers)
			{
				if (r.m_iJob == JOB_COURIER && r.m_iArg == RZ_SiteRoute.ROAD_POST)
					running++;
			}
		}

		if (running > 0 || CountExtras() + 1 > m_Settings.m_iExtrasMax)
			return;

		RZ_CampWorker pick;
		foreach (RZ_CampWorker w : p.m_aWorkers)
		{
			if (!IsWorking(w) || w.m_iRole == ROLE_PERSON || w.m_iJob == JOB_WATCH || w.m_iJob == JOB_PATROL || w.m_iJob == JOB_COURIER)
				continue;

			if (!pick || w.m_iRole == ROLE_EXTRA)
				pick = w;
		}

		if (!pick)
			return;

		p.m_iRunDay = today;
		SendOff(p, pick, JOB_COURIER, RZ_SiteRoute.ROAD_POST);
		RZ_Log.Info(string.Format("Camp life: %1 sends one on the road post run to %2 (%3 m, %4 points).", p.m_sTitle, route.m_sTo, RZ_Util.RoundToInt(route.m_fOut), route.m_aPoints.Count()));
	}

	//------------------------------------------------------------------------------------------------
	//! The site's alert now (docs/SITES.md, "When the site is threatened"): alert on contact (one of
	//! its people died in the last 3 minutes) or heat enough for a horde in its square; wary with half
	//! that heat, a hostile base within 300 m or its faction under pressure; recently hit while its
	//! story says it was attacked or burned in the last two days; else calm. True if it changed.
	protected bool UpdateAlert(RZ_CampPlace p)
	{
		int now = RZ_SitePeople.ALERT_CALM;
		if (m_Settings.m_bAlertStates && p.m_People && p.m_eKind != RZ_ECampPlace.ROAD_POST)
		{
			float heat = 0;
			if (m_GM.GetHeat())
				heat = m_GM.GetHeat().GetHeat(p.m_vPos);

			RZ_Territory territory = m_GM.GetTerritory();
			bool pressure = false;
			if (territory)
				pressure = territory.IsUnderPressure(p.m_eFaction);

			bool hostile = p.m_People.m_fHostileDist >= 0 && p.m_People.m_fHostileDist < HOSTILE_WARY;
			if (m_fClock - p.m_fLossAt < CONTACT_SECONDS || heat >= HEAT_ALERT)
				now = RZ_SitePeople.ALERT_ALERT;
			else if (heat >= HEAT_WARY || hostile || pressure)
				now = RZ_SitePeople.ALERT_WARY;
			else if (p.m_Facts && RecentlyHit(p.m_Facts))
				now = RZ_SitePeople.ALERT_HIT;
		}

		if (now == p.m_iAlert)
			return false;

		RZ_Log.Info(string.Format("Camp life: %1 is %2 (was %3).", p.m_sTitle, RZ_SitePeople.AlertName(now), RZ_SitePeople.AlertName(p.m_iAlert)));
		p.m_iAlert = now;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Attacked or burned in the last two days (the gate doubled for two season days).
	protected static bool RecentlyHit(RZ_SiteFacts f)
	{
		if (f.m_iLately != RZ_SiteFacts.HIT && f.m_iLately != RZ_SiteFacts.BURNED)
			return false;

		return RZ_SiteStory.Recent(f.m_iLatelyDay, 2);
	}

	//------------------------------------------------------------------------------------------------
	//! The line its people say when you come in on foot (within 40 m of the gate or the yard), by
	//! its state, once a season day a player and state.
	protected void Greet(RZ_CampPlace p, array<int> ids, array<IEntity> players)
	{
		if (!m_Settings.m_bStory || !p.m_Facts || !ids || p.m_eKind == RZ_ECampPlace.ROAD_POST)
			return;

		vector gate = p.m_vHome;
		if (p.m_People)
			gate = p.m_People.m_vGate;

		int day = RZ_SeasonClock.Day();
		foreach (int i, IEntity pl : players)
		{
			if (!pl || i >= ids.Count() || RZ_Util.IsInVehicle(pl))
				continue;

			vector at = pl.GetOrigin();
			if (RZ_Util.DistXZ(at, gate) > GREET_REACH && RZ_Util.DistXZ(at, p.m_vHome) > GREET_REACH)
				continue;

			string key = string.Format("%1|%2|%3", ids[i], day, p.m_Facts.m_iLately);
			if (p.m_aGreeted.Contains(key))
				continue;

			p.m_aGreeted.Insert(key);
			string line = RZ_SiteStory.Line(p.m_eFaction, p.m_Facts);
			if (!line.IsEmpty())
				m_GM.Say(ids[i], RZ_SiteStory.Speaker(p.m_eFaction), p.m_eFaction, line);
		}
	}

	// ================================================================ The story

	//------------------------------------------------------------------------------------------------
	//! The site's story (docs/SITES.md, "The story of each site"): its state read now, the story it
	//! picks and that story's elements queued on the spots the solver kept (Traces' prefabs, aged as
	//! scenes are), after its pieces (the first left out at the cap). The story it showed before goes.
	protected void SpawnStory(RZ_CampPlace p, RZ_SiteLayout kept)
	{
		RetireStory(p);
		p.m_iStoryDay = RZ_SeasonClock.Day();
		if (!m_Settings.m_bStory || !kept || p.m_eKind == RZ_ECampPlace.ROAD_POST)
			return;

		RZ_SiteFacts facts = RZ_SiteStory.Read(m_GM, p);
		p.m_Facts = facts;
		int story = RZ_SiteStory.Pick(p.m_eFaction, facts, p.m_iSeed);
		p.m_iStory = story;
		array<int> kinds = {};
		array<int> counts = {};
		RZ_SiteStory.Elements(story, facts, kinds, counts);
		string up = "";
		string textOnly = "";
		foreach (int k, int kind : kinds)
		{
			RZ_SitePlaced spot = StorySpotOf(kept.m_aPieces, kind);
			int count = RZ_SiteStory.Count(kind, facts, counts[k]);
			if (spot && kind == RZ_SiteStory.E_GRAVES)
				count = Math.Min(count, spot.m_iStoryRows);

			int stood = 0;
			int age = 0;
			for (int c = 0; c < count && spot != null; c++)
			{
				ResourceName own = RZ_SiteStory.Prefab(kind, p.m_eFaction, facts, c);
				if (own.IsEmpty())
					continue;

				ResourceName prefab = RZ_SiteStory.Aged(kind, own, RZ_SiteStory.MadeDay(kind, facts), age);
				if (QueueStory(p, prefab, StoryPos(spot, kind, c, count), spot.m_fYaw))
					stood++;
			}

			string name = RZ_SiteStory.ElementName(kind);
			if (count > 1)
				name += string.Format(" (%1)", count);

			if (stood > 0)
			{
				array<string> ages = {"fresh", "weathered", "old"};
				up = AddText(up, string.Format("%1, %2", name, ages[Math.ClampInt(age, 0, 2)]));
			}
			else if (!spot)
				textOnly = AddText(textOnly, name + ": no spot");
			else
				textOnly = AddText(textOnly, name + ": no prefab or the cap");
		}

		if (up.IsEmpty())
			up = "none";

		if (textOnly.IsEmpty())
			textOnly = "none";

		RZ_Log.Info(string.Format("Camp life story: %1 at %2: %3 (%4); up: %5; text only: %6.", p.m_sTitle, RZ_SettlementScanner.GridReference(p.m_vPos), RZ_SiteStory.Name(story), facts.Describe(), up, textOnly));
		Mention(p, facts);
	}

	//------------------------------------------------------------------------------------------------
	protected static string AddText(string list, string item)
	{
		if (list.IsEmpty())
			return item;

		return list + "; " + item;
	}

	//------------------------------------------------------------------------------------------------
	//! The kept spot for an element (null: none was found when the site was solved).
	static RZ_SitePlaced StorySpotOf(array<ref RZ_SitePlaced> pieces, int kind)
	{
		foreach (RZ_SitePlaced placed : pieces)
		{
			if (placed.m_iLayer == RZ_ESiteLayer.STORY && placed.m_iStory == kind)
				return placed;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Where the c-th of an element stands on its spot: graves in a row 1.6 m apart, the gate's two
	//! boards side by side, the rest in its middle.
	protected static vector StoryPos(RZ_SitePlaced spot, int kind, int c, int count)
	{
		float apart = 0;
		if (kind == RZ_SiteStory.E_GRAVES)
			apart = 1.6;
		else if (kind == RZ_SiteStory.E_GATEMARK)
			apart = 1.3;

		if (apart == 0 || count <= 1)
			return spot.m_vPos;

		float along = (c - (count - 1) * 0.5) * apart;
		return RZ_Util.GroundPos(RZ_Compass.PointAt(spot.m_vPos, spot.m_fYaw + 90, along));
	}

	//------------------------------------------------------------------------------------------------
	//! A story element waiting to spawn with the place's pieces (scenery only: the dressing guard
	//! strips it as it spawns), if the server's piece cap allows.
	protected bool QueueStory(RZ_CampPlace p, ResourceName prefab, vector at, float yaw)
	{
		if (prefab.IsEmpty() || CountPieces() >= m_Settings.m_iPiecesMax)
			return false;

		RZ_CampPiece cp = new RZ_CampPiece();
		cp.m_sPending = prefab;
		cp.m_vPos = at;
		cp.m_fYaw = yaw;
		cp.m_eUse = RZ_EDressUse.NONE;
		cp.m_iSeats = 1;
		cp.m_bStory = true;
		cp.m_sName = "Story";
		p.m_aPieces.Insert(cp);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The story shown before goes (its entries stay in the list as nothing, so no one's piece
	//! index moves).
	protected void RetireStory(RZ_CampPlace p)
	{
		foreach (RZ_CampPiece piece : p.m_aPieces)
		{
			if (!piece.m_bStory)
				continue;

			if (piece.m_Entity)
			{
				s_aPieces.RemoveItem(piece.m_Entity);
				SCR_EntityHelper.DeleteEntityAndChildren(piece.m_Entity);
			}

			piece.m_Entity = null;
			piece.m_sPending = "";
			piece.m_bStory = false;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Each season day the story of every place standing is worked out again (the same seed, the new
	//! state), and every holding that was hit or is mourning gets its one quiet headline.
	protected void StoryDay()
	{
		int today = RZ_SeasonClock.Day();
		if (today == m_iStoryDayAll)
			return;

		m_iStoryDayAll = today;
		foreach (RZ_CampPlace p : m_aPlaces)
		{
			if (p.m_iStoryDay >= 0 && p.m_iStoryDay != today)
				SpawnStory(p, KeptLayout(p));
		}

		if (!m_Settings.m_bStory)
			return;

		RZ_Territory territory = m_GM.GetTerritory();
		if (!territory)
			return;

		foreach (RZ_Holding h : territory.GetHoldings())
		{
			RZ_ECampPlace kind = RZ_ECampPlace.OUTPOST;
			if (h.m_bStronghold)
				kind = RZ_ECampPlace.STRONGHOLD;

			RZ_CampPlace q = new RZ_CampPlace();
			q.m_sKey = "h" + h.m_iId.ToString();
			q.m_sTitle = h.GetTitle();
			InitPlace(q, h.m_eFaction, kind, h.m_vPos);
			RZ_CampSite site;
			if (m_mSites.Find(q.m_sKey, site) && site && !site.m_bNone)
				q.m_Site = site;

			RZ_SiteLayout kept = KeptLayout(q);
			if (kept)
				q.m_People = kept.m_People;

			Mention(q, RZ_SiteStory.Read(m_GM, q));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A site hit or mourning: one quiet headline (stored with the news, never broadcast), once for
	//! what happened.
	protected void Mention(RZ_CampPlace p, RZ_SiteFacts facts)
	{
		if (!m_Settings.m_bStory || !facts)
			return;

		string text = RZ_SiteStory.Mention(p.m_eFaction, facts);
		if (text.IsEmpty())
			return;

		string key = string.Format("%1|%2|%3", p.m_sKey, facts.m_iLately, facts.m_iLatelyDay);
		if (m_aMentioned.Contains(key))
			return;

		m_aMentioned.Insert(key);
		RZ_Log.Info("Camp life story: " + text);
		if (m_GM.GetLeaders())
			m_GM.GetLeaders().AddHeadline(text, false);
	}

	//------------------------------------------------------------------------------------------------
	//! The audit's dry run (headless, docs/SITES.md "The audit"): the routine's jobs, as data, for the
	//! people the place has in play (its named people too), by day and at night, wary, under attack
	//! and recently hit: the posts filled and the routes walked.
	string PeopleDryRun(RZ_CampPlace p)
	{
		if (!p.m_People)
			return "no posts";

		int total = PeopleInPlay(p.m_eFaction, p.m_eKind);
		int persons = 1;
		if (p.m_eKind == RZ_ECampPlace.STRONGHOLD)
			persons = 2;

		persons = Math.Min(persons, total - 1);
		array<int> roles = {};
		for (int i = 0; i < total; i++)
		{
			if (i < total - persons)
				roles.Insert(ROLE_GUARD);
			else
				roles.Insert(ROLE_PERSON);
		}

		array<int> phases = {RZ_Territory.PHASE_DAY, RZ_Territory.PHASE_NIGHT, RZ_Territory.PHASE_DAY, RZ_Territory.PHASE_DAY, RZ_Territory.PHASE_DAY};
		array<int> alerts = {RZ_SitePeople.ALERT_CALM, RZ_SitePeople.ALERT_CALM, RZ_SitePeople.ALERT_WARY, RZ_SitePeople.ALERT_ALERT, RZ_SitePeople.ALERT_HIT};
		array<string> names = {"by day", "at night", "wary", "under attack", "recently hit"};
		int was = p.m_iAlert;
		string text = "";
		foreach (int k, int phase : phases)
		{
			p.m_iAlert = alerts[k];
			array<int> jobs = {};
			array<int> args = {};
			FillPlan(p, phase, roles, jobs, args);
			if (!text.IsEmpty())
				text += " | ";

			text += names[k] + ": " + PlanText(p, jobs, args);
		}

		p.m_iAlert = was;
		return string.Format("%1 people (%2 named): %3", total, persons, text);
	}

	// ================================================================ Talking
	//------------------------------------------------------------------------------------------------
	//! A player talks to someone at a camp: the nearest person within 6 m stops their job for a
	//! while; the next shuffle after that sends them back to it.
	void HoldPersonNear(int playerId)
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!player)
			return;

		vector at = player.GetOrigin();
		RZ_CampPlace bestPlace;
		RZ_CampWorker best;
		float bestDist = 6;
		foreach (RZ_CampPlace p : m_aPlaces)
		{
			if (RZ_Util.DistXZ(p.m_vPos, at) > 120)
				continue;

			foreach (RZ_CampWorker w : p.m_aWorkers)
			{
				if (!w.m_Unit || w.m_iRole == ROLE_KEEPER)
					continue;

				float d = RZ_Util.DistXZ(RZ_Util.GetUnitPosition(w.m_Unit), at);
				if (d < bestDist)
				{
					bestDist = d;
					best = w;
					bestPlace = p;
				}
			}
		}

		if (!best)
			return;

		ClearJob(bestPlace, best);
		best.m_fHoldUntil = m_fClock + HOLD_SECONDS;
	}
}
