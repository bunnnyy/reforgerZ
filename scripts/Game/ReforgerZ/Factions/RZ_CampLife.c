//! Lived-in outposts and camps (docs/DESIGN.md, "Camp life").
//!
//! While a place's bodies exist (territory holdings, Haven safehouses, road posts: a player is
//! near), its faction's scenery stands around it: compositions from Camp life > Kits. The place
//! moves into the building in its town that suits its faction (Bastion a school, town hall or
//! police station; the Red Hand a garage or petrol station; the Haven a church or a big house;
//! Halcyon the lab or a house), picked by kind and size from the buildings' prefab names and
//! boxes, and its pieces are laid out by that building: the working area in its yard, the cooking
//! to one side, the rest along its walls, the gate on the road in from the next town and
//! barricades at the other road entries. With no building that fits, a small compound on rings
//! round its centre. Always from a seed (the place's position), on flat open ground, off roads
//! and out of buildings. Pieces spawn a few per tick.
//! Nothing is saved: the same seed gives the same layout on every
//! visit, and the pieces go with the bodies. RZ_DressingGuard strips anything a player could
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
	CENTRE,		// 8-14 m out, the working area: the fire, the table, the roster board
	INNER,		// 14-22 m: tents at the back (the sleeping area), the cook pot to one side (the cooking area)
	APPROACH,	// 24-34 m toward the nearest road (or the next town), the perimeter: the nest, the barrier, barricades
	EDGE		// 28-46 m on the other sides: the washing line, the garden, the generator, the latrine
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
//! Where a piece stands when its place has moved into a building (RZ_CampSite). Auto: from its
//! role and use (Centre the yard, Inner the back or the cooking side, Approach the gate or the
//! closed entries, Edge the walls).
enum RZ_EDressAt
{
	AUTO,
	YARD,		// the working area: behind the building (the Red Hand: its forecourt)
	FRONT,		// along the front, toward the road: a market stall, a trophy wall
	BACK,		// further behind the yard: the wash stand, the cold box, the clutter
	SIDE,		// the cooking side, left or right by the seed: the cook pot, the woodpile
	WALLS,		// along the back and side walls: the line, the garden, the generator, the latrine
	DOOR,		// by the front door: the bear board, the memorial, the decon shower
	GATE,		// at the road entry left open (the road in from the next town): the nest, the toll barrier, the sign
	CLOSED		// at the other road entries, beside the road: barricades
}

//------------------------------------------------------------------------------------------------
//! One kind of piece in a kit.
[BaseContainerProps()]
class RZ_DressingPiece
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "A composition from Prefabs/ReforgerZ/Dressing/ (docs/WORKBENCH_SETUP.md, Camp life): scenery only. Anything a player could use is deleted when it spawns", "et")]
	ResourceName m_sPrefab;

	[Attribute("0", UIWidgets.ComboBox, "Where it stands: Centre (8-14 m, the working area), Inner (14-22 m: tents at the back, cook and chop pieces to one side), Approach (24-34 m toward the road, the perimeter), Edge (28-46 m elsewhere). Road posts put everything beside the road", "", ParamEnumArray.FromEnum(RZ_EDressRole))]
	RZ_EDressRole m_eRole;

	[Attribute("0", UIWidgets.ComboBox, "What people do at it (its sentinels carry the matching tag: RZ_Sit, RZ_Work, RZ_Watch, RZ_Gather, RZ_Lean, RZ_Cook, RZ_Wash, RZ_Chop). Sleep: a tent. None: scenery only", "", ParamEnumArray.FromEnum(RZ_EDressUse))]
	RZ_EDressUse m_eUse;

	[Attribute("1", UIWidgets.Slider, "How many of it (a place stops at its piece count, in list order)", "1 4 1")]
	int m_iCount;

	[Attribute("2", UIWidgets.Slider, "People who use one at once: the number of sentinels with its tag", "1 6 1")]
	int m_iSeats;

	[Attribute("", desc: "Smart-action tag on its sentinels if not the use's own (RZ_Sit, RZ_Work, RZ_Watch, RZ_Gather, RZ_Lean, RZ_Cook, RZ_Wash, RZ_Chop)")]
	string m_sTag;

	[Attribute("0", UIWidgets.ComboBox, "Where it stands when the place has moved into a building: Auto (from its role and use), Yard, Front, Back, Side (the cooking side), Walls, Door, Gate (the road entry left open), Closed (the other road entries). Written by tools/camp_kit/build.py", "", ParamEnumArray.FromEnum(RZ_EDressAt))]
	RZ_EDressAt m_eAt;
}

//------------------------------------------------------------------------------------------------
//! A faction's scenery for one kind of place (or Any).
[BaseContainerProps()]
class RZ_DressingKit
{
	[Attribute("0", UIWidgets.ComboBox, "Faction", "", ParamEnumArray.FromEnum(RZ_EFaction))]
	RZ_EFaction m_eFaction;

	[Attribute("0", UIWidgets.ComboBox, "Places it dresses: Any, or only one kind. A kind's own kit wins over the faction's Any kit", "", ParamEnumArray.FromEnum(RZ_ECampPlace))]
	RZ_ECampPlace m_ePlace;

	[Attribute(desc: "Pieces, most important first: a place takes them in this order until it reaches its piece count")]
	ref array<ref RZ_DressingPiece> m_aPieces;
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

	[Attribute("90", desc: "Pieces standing (or waiting to spawn) on the whole server at most. They spawn a few per tick")]
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

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Observation post waypoint (vanilla AIWaypoint_ObservationPost.et) for a sentry without a nest. Empty = the loiter waypoint", "et")]
	ResourceName m_sObservationWaypoint;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Smart-action waypoint (Prefabs/ReforgerZ/AI/RZ_WP_SmartAction.et, an SCR_SmartActionWaypoint): people use a piece's sit, lean and watch points. Empty = loiter at the piece", "et")]
	ResourceName m_sSmartActionWaypoint;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Empty AI group (vanilla Group_Base.et): a lone character (an occupant, a single guard) is put in one so it can take jobs. Empty = lone characters stand", "et")]
	ResourceName m_sGroupPrefab;

	[Attribute(desc: "Kits: each faction's scenery, per kind of place or for Any. A place with no kit stays bare")]
	ref array<ref RZ_DressingKit> m_aKits;
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
	float m_fCookSide;			// +90 or -90 from the approach: the cooking area's side
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
	int m_iCookFace;			// FACE_LEFT or FACE_RIGHT, by the seed
	vector m_vHome;				// the yard's middle
	ref array<vector> m_aEntries = {};	// where roads cross the ring round the building
	ref array<float> m_aEntryRoad = {};	// the road's bearing there
	int m_iMain = -1;			// the entry left open (the gate); the others are closed
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

	protected static const int SPAWN_PER_TICK = 4;			// pieces spawned per server tick, all places together
	protected static const float CHECK_INTERVAL = 10;
	protected static const float LIVE_INTERVAL = 30;
	protected static const float ROLL_SECONDS = 180;		// the roll at 20:30, before the night watch
	protected static const float COURIER_SECONDS = 1200;	// Halcyon's courier walks out and back
	protected static const float SLEEP_CLEAR = 80;			// nobody this close sees anyone vanish or appear
	protected static const float EXTRA_CLEAR = 60;
	protected static const float HOLD_SECONDS = 45;			// someone talked to stands still
	protected static const float PERSON_RADIUS = 20;		// named people stay this close to the centre
	protected static const float KEEP_CLEAR = 4;			// metres from a quartermaster, the medic, the barrier
	protected static const float LEADER_CLEAR = 6;			// from a leader's seat (the body stands 2-5 m from it)
	protected static const float PIECE_GAP = 5.5;
	protected static const float SHARE_RADIUS = 60;			// a place this close to a dressed one isn't dressed again
	protected static const int TRIES = 10;

	static const int FACE_FRONT = 0;
	static const int FACE_BACK = 1;
	static const int FACE_LEFT = 2;
	static const int FACE_RIGHT = 3;

	protected static const float SITE_MAX_SIDE = 70;		// a building longer than this isn't a camp (a hangar, a factory)
	protected static const int SITE_SHORTLIST = 6;			// the best few by kind, size and distance; then the roads decide
	protected static const float ENTRY_MERGE = 14;			// road crossings this close are one entry
	protected static const int MAX_ENTRIES = 5;
	protected static const float LOOKOUT_OUT = 2;			// the lookout stands this far out from the front corner
	protected static const int MAX_SITES = 64;			// sites kept per run

	protected static ref array<IEntity> s_aPieces;			// every standing piece (IsDressing)

	protected RZ_GameModeComponent m_GM;
	protected RZ_CampLifeSettings m_Settings;
	protected ref array<ref RZ_CampPlace> m_aPlaces = {};
	protected ref map<string, bool> m_mToldNoKit = new map<string, bool>();
	protected ref map<string, ref RZ_CampSite> m_mSites = new map<string, ref RZ_CampSite>();
	protected ref map<string, bool> m_mToldSite = new map<string, bool>();
	protected float m_fClock;
	protected float m_fCheckTimer;
	protected float m_fLiveTimer;
	protected float m_fShuffleTimer;
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
	//! Pieces standing and waiting to spawn, on the whole server.
	int CountPieces()
	{
		for (int i = s_aPieces.Count() - 1; i >= 0; i--)
		{
			if (!s_aPieces[i])
				s_aPieces.Remove(i);
		}

		int n = s_aPieces.Count();
		foreach (RZ_CampPlace p : m_aPlaces)
		{
			foreach (RZ_CampPiece piece : p.m_aPieces)
			{
				if (!piece.m_sPending.IsEmpty())
					n++;
			}
		}

		return n;
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

		AddLeaderSeats(p);
		m_aPlaces.Insert(p);
		m_bLiveDirty = true;

		RZ_CampGround ground = new RZ_CampGround();
		ReadGround(p, ground);
		UseSite(p, ground, FindSite(p, ground));
		if (m_Settings.m_bDressing && !IsDressedNear(p))
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
		p.m_fCookSide = 90;
		RZ_Rng sideRng = new RZ_Rng(p.m_iSeed + 11);
		if (sideRng.Chance(0.5))
			p.m_fCookSide = -90;
	}

	//------------------------------------------------------------------------------------------------
	//! Where a place's people spawn: the yard of the building it moves into, else pos. The owner
	//! calls this before it spawns the bodies; Enter reuses the building it found.
	vector HomeFor(string key, RZ_EFaction faction, RZ_ECampPlace kind, vector pos)
	{
		if (!m_Settings.m_bDressing || !m_Settings.m_bBuildings || kind == RZ_ECampPlace.ROAD_POST)
			return pos;

		RZ_CampPlace p = new RZ_CampPlace();
		p.m_sKey = key;
		InitPlace(p, faction, kind, pos);
		RZ_CampGround ground = new RZ_CampGround();
		ReadGround(p, ground);
		RZ_CampSite site = FindSite(p, ground);
		if (!site || site.m_bNone)
			return pos;

		return site.m_vHome;
	}

	//------------------------------------------------------------------------------------------------
	//! From the owner's DespawnBodies, before it deletes them: jobs, wrappers, extras and pieces go.
	void Leave(string key)
	{
		for (int i = m_aPlaces.Count() - 1; i >= 0; i--)
		{
			RZ_CampPlace p = m_aPlaces[i];
			if (p.m_sKey != key)
				continue;

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
		if (m_GM.GetPerks() && m_GM.GetPerks().GetQuartermasterPos(p.m_eFaction, qm) && RZ_Util.DistXZ(qm, p.m_vPos) < SHARE_RADIUS)
			p.m_aLeaderSeats.Insert(qm);

		array<RZ_Leader> list = {};
		leaders.GetFactionLeaders(p.m_eFaction, list);
		foreach (RZ_Leader l : list)
		{
			if (l && RZ_Util.DistXZ(l.m_vPos, p.m_vPos) < SHARE_RADIUS)
				p.m_aLeaderSeats.Insert(l.m_vPos);
		}
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

		return false;
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
	protected void Dress(RZ_CampPlace p, RZ_CampGround ground)
	{
		RZ_DressingKit kit = FindKit(p.m_eFaction, p.m_eKind);
		if (!kit || !kit.m_aPieces || kit.m_aPieces.IsEmpty())
		{
			string told = string.Format("%1:%2", p.m_eFaction, p.m_eKind);
			if (!m_mToldNoKit.Contains(told))
			{
				m_mToldNoKit.Set(told, true);
				RZ_Log.Info(string.Format("Dressing: no kit for %1 (%2); those places stay bare.", RZ_Factions.GetName(p.m_eFaction), typename.EnumToString(RZ_ECampPlace, p.m_eKind)));
			}

			return;
		}

		int cap = PieceCap(p);
		RZ_Rng rng = new RZ_Rng(p.m_iSeed);
		int placed = 0;
		bool full = false;
		foreach (RZ_DressingPiece piece : kit.m_aPieces)
		{
			if (full)
				break;

			if (!piece || piece.m_sPrefab.IsEmpty())
				continue;

			// In a building they sleep indoors: no tents
			if (p.m_Site && piece.m_eUse == RZ_EDressUse.SLEEP)
				continue;

			RZ_EDressAt at = PieceAt(piece);
			int count = Math.Max(1, piece.m_iCount);
			for (int n = 0; n < count; n++)
			{
				if (placed >= cap || CountPieces() >= m_Settings.m_iPiecesMax)
				{
					full = true;
					break;
				}

				// Always the same draws per piece, so a refused spot doesn't move the next piece
				array<vector> spots = {};
				for (int t = 0; t < TRIES; t++)
				{
					if (p.m_Site)
						spots.Insert(SiteCandidate(p, at, rng));
					else
						spots.Insert(Candidate(p, piece.m_eRole, piece.m_eUse, rng));
				}

				vector spot;
				bool found = false;
				foreach (vector c : spots)
				{
					if (GoodSpot(p, ground, c))
					{
						spot = c;
						found = true;
						break;
					}
				}

				if (at == RZ_EDressAt.CLOSED)
					p.m_iClosedNext++;

				if (!found)
					continue;

				// Laid out now, spawned a few per tick (SpawnPending)
				RZ_CampPiece cp = new RZ_CampPiece();
				cp.m_sPending = piece.m_sPrefab;
				cp.m_fYaw = PieceYaw(p, piece.m_eRole, spot);
				cp.m_vPos = spot;
				cp.m_eRole = piece.m_eRole;
				cp.m_eUse = piece.m_eUse;
				cp.m_iSeats = Math.Max(1, piece.m_iSeats);
				cp.m_sTag = piece.m_sTag;
				if (cp.m_sTag.IsEmpty())
					cp.m_sTag = UseTag(piece.m_eUse);

				p.m_aPieces.Insert(cp);
				placed++;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A few laid-out pieces spawn each tick, places in the order they came. A place whose last piece is up
	//! hands out its jobs again, so people use the pieces that weren't there yet.
	protected void SpawnPending()
	{
		int budget = SPAWN_PER_TICK;
		foreach (RZ_CampPlace p : m_aPlaces)
		{
			if (budget <= 0)
				return;

			bool spawned = false;
			bool waiting = false;
			foreach (RZ_CampPiece cp : p.m_aPieces)
			{
				if (cp.m_sPending.IsEmpty())
					continue;

				if (budget <= 0)
				{
					waiting = true;
					break;
				}

				ResourceName prefab = cp.m_sPending;
				cp.m_sPending = "";
				budget--;
				spawned = true;
				IEntity e = RZ_Util.SpawnPrefab(prefab, cp.m_vPos, cp.m_fYaw); // VERIFY: a spawned composition (root with an RplComponent) shows on clients, and props spawned at run time block AI paths (else AI walk through tents)
				if (!e || !RZ_DressingGuard.Strip(e, prefab))
					continue;

				cp.m_Entity = e;
				s_aPieces.Insert(e);
			}

			if (spawned && !waiting && p.m_bLive)
				p.m_iPhase = -2;
		}
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
	//! A spot on the piece's ring. Always three draws, whatever the role.
	//! The layout is a small compound: the working area in the middle (Centre), the perimeter
	//! toward the approach (Approach), and on the inner ring a sleeping area at the back (tents,
	//! the wash stand just behind them) and a cooking area to one side (the cook pot, the
	//! firewood); everything else on the edge.
	protected vector Candidate(RZ_CampPlace p, RZ_EDressRole role, RZ_EDressUse use, RZ_Rng rng)
	{
		float a = rng.Float01();
		float b = rng.Float01();
		bool side = rng.Chance(0.5);

		if (p.m_eKind == RZ_ECampPlace.ROAD_POST)
		{
			float sideBearing = p.m_fYaw + 90;
			if (side)
				sideBearing = p.m_fYaw - 90;

			return RZ_Compass.PointAt(p.m_vPos, sideBearing - 35 + a * 70, 5 + b * 6);
		}

		float scale = 1;
		if (p.m_eKind == RZ_ECampPlace.STRONGHOLD)
			scale = 1.3;
		else if (p.m_eKind == RZ_ECampPlace.SAFEHOUSE)
			scale = 0.8;

		// Centre, the working area: anywhere round, close in
		float bearing = a * 360;
		float radius = 8 + b * 6;
		if (role == RZ_EDressRole.INNER)
		{
			bearing = p.m_fApproach + 40 + a * 280;
			radius = 14 + b * 8;
		}
		else if (role == RZ_EDressRole.APPROACH)
		{
			// The perimeter: spread across the approach side
			bearing = p.m_fApproach - 45 + a * 90;
			radius = 24 + b * 10;
		}
		else if (role == RZ_EDressRole.EDGE)
		{
			bearing = p.m_fApproach + 60 + a * 240;
			radius = 28 + b * 18;
		}

		// The areas (not on the perimeter): sleeping at the back, cooking to one side
		if (role != RZ_EDressRole.APPROACH)
		{
			if (use == RZ_EDressUse.SLEEP)
			{
				bearing = p.m_fApproach + 125 + a * 110;
			}
			else if (use == RZ_EDressUse.WASH)
			{
				bearing = p.m_fApproach + 110 + a * 140;
				radius = radius + 4;
			}
			else if (use == RZ_EDressUse.COOK || use == RZ_EDressUse.CHOP)
			{
				bearing = p.m_fApproach + p.m_fCookSide - 35 + a * 70;
			}
		}

		return RZ_Compass.PointAt(p.m_vPos, bearing, radius * scale);
	}

	//------------------------------------------------------------------------------------------------
	//! Pieces face the centre (in a building, its yard); the perimeter faces out; at a road post
	//! they face the road.
	protected float PieceYaw(RZ_CampPlace p, RZ_EDressRole role, vector spot)
	{
		if (p.m_eKind != RZ_ECampPlace.ROAD_POST && role == RZ_EDressRole.APPROACH)
			return RZ_Compass.Bearing(p.m_vCentre, spot);

		if (p.m_eKind == RZ_ECampPlace.ROAD_POST)
			return RZ_Compass.Bearing(spot, p.m_vPos);

		return RZ_Compass.Bearing(spot, p.m_vHome);
	}

	//------------------------------------------------------------------------------------------------
	protected bool GoodSpot(RZ_CampPlace p, RZ_CampGround ground, vector spot)
	{
		foreach (vector seat : p.m_aLeaderSeats)
		{
			if (RZ_Util.DistXZ(seat, spot) < LEADER_CLEAR)
				return false;
		}

		// The shared placer's rules (World/Traces/RZ_DressPlacer.c): dry, off the line, flat, clear
		// of the quartermasters, the medic and the barrier, apart from the other pieces, off the
		// roads and out of the buildings' footprints (locked ones and their doorway blockers too)
		array<vector> taken = {};
		foreach (RZ_CampPiece other : p.m_aPieces)
		{
			taken.Insert(other.m_vPos);
		}

		return RZ_DressPlacer.Good(spot, ground, p.m_aKeepClear, KEEP_CLEAR, taken, PIECE_GAP);
	}

	//------------------------------------------------------------------------------------------------
	//! Buildings and road segments near the place, and which way its approach faces.
	protected void ReadGround(RZ_CampPlace p, RZ_CampGround ground)
	{
		// The shared placer's read (World/Traces/RZ_DressPlacer.c), further out for a building
		float reach = RZ_DressPlacer.BUILDINGS_NEAR;
		float roadReach = RZ_DressPlacer.ROADS_NEAR;
		if (m_Settings.m_bBuildings)
		{
			reach = Math.Max(reach, SiteReach(p.m_eKind) + 10);
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
			return;
		}

		if (p.m_bHasTown)
		{
			p.m_fApproach = p.m_fTownBearing;
			return;
		}

		RZ_Rng rng = new RZ_Rng(p.m_iSeed + 7);
		p.m_fApproach = rng.Range(0, 360);
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

			if (school || farm)
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

		RZ_CampSite site = PickSite(p, ground);
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
				RZ_Log.Info(string.Format("Camp life: %1 at %2 moves into %3 (%4 x %5 m), %6 road entries.", what, grid, site.m_sWhat, w, d, site.m_aEntries.Count()));
			}
		}

		return site;
	}

	//------------------------------------------------------------------------------------------------
	//! The best building by kind, size and distance, then by the roads (Bastion on a road, the Red
	//! Hand where roads meet). Never one with scenery on it, a ruin, a hangar-sized one or one
	//! smaller than the kind needs.
	protected RZ_CampSite PickSite(RZ_CampPlace p, RZ_CampGround ground)
	{
		RZ_CampSite site = new RZ_CampSite();
		site.m_sKey = p.m_sKey;
		site.m_eFaction = p.m_eFaction;
		site.m_eKind = p.m_eKind;
		site.m_vPlace = p.m_vPos;
		site.m_bNone = true;

		float reach = SiteReach(p.m_eKind);
		float minArea = SiteMinArea(p.m_eKind);
		bool stronghold = p.m_eKind == RZ_ECampPlace.STRONGHOLD;
		float distWeight = 3;
		if (p.m_eKind == RZ_ECampPlace.SAFEHOUSE)
			distWeight = 12;

		array<IEntity> shortlist = {};
		array<float> scores = {};
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

			vector mid = RZ_DoorGeometry.ToWorld(b, (mins + maxs) * 0.5);
			float dist = RZ_Util.DistXZ(mid, p.m_vPos);
			if (dist > reach)
				continue;

			string name = RZ_Util.GetPrefabName(b);
			name.ToLower();
			if (name.Contains("ruin") || name.Contains("wreck"))
				continue;

			int tier = SiteTier(p.m_eFaction, p.m_eKind, name, RZ_DoorSlots.KindOf(b));
			if (tier < 0)
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
			if (shortlist.Count() > SITE_SHORTLIST)
			{
				shortlist.Remove(SITE_SHORTLIST);
				scores.Remove(SITE_SHORTLIST);
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
			}
		}

		if (best)
			ReadSite(p, ground, site, best);

		return site;
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

		site.m_iCookFace = FACE_LEFT;
		RZ_Rng rng = new RZ_Rng(p.m_iSeed + 13);
		if (rng.Chance(0.5))
			site.m_iCookFace = FACE_RIGHT;

		site.m_vHome = FacePoint(site, site.m_iYardFace, 0, 6);
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
			if (da < ring && db >= ring)
			{
				AddCrossing(site, c, ring, a, b, bearing);
			}
			else if (db < ring && da >= ring)
			{
				AddCrossing(site, c, ring, b, a, bearing);
			}
			else if (da >= ring && db >= ring && RZ_DressPlacer.DistToSegment(c, a, b) < ring)
			{
				// A long straight passing through: in and out again
				vector close = ClosestOnSegment(c, a, b);
				AddCrossing(site, c, ring, close, a, bearing);
				AddCrossing(site, c, ring, close, b, bearing);
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
	protected static void AddCrossing(RZ_CampSite site, vector c, float ring, vector inside, vector outside, float roadBearing)
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
	//! Where a piece stands in a building's place: its own setting, else from its role and use.
	protected static RZ_EDressAt PieceAt(RZ_DressingPiece piece)
	{
		if (piece.m_eAt != RZ_EDressAt.AUTO)
			return piece.m_eAt;

		if (piece.m_eRole == RZ_EDressRole.CENTRE)
			return RZ_EDressAt.YARD;

		if (piece.m_eRole == RZ_EDressRole.EDGE)
			return RZ_EDressAt.WALLS;

		if (piece.m_eRole == RZ_EDressRole.APPROACH)
		{
			if (piece.m_eUse == RZ_EDressUse.WATCH || piece.m_eUse == RZ_EDressUse.LEAN)
				return RZ_EDressAt.GATE;

			return RZ_EDressAt.CLOSED;
		}

		if (piece.m_eUse == RZ_EDressUse.COOK || piece.m_eUse == RZ_EDressUse.CHOP)
			return RZ_EDressAt.SIDE;

		return RZ_EDressAt.BACK;
	}

	//------------------------------------------------------------------------------------------------
	//! A spot by the building. Always four draws, whatever the spot.
	protected vector SiteCandidate(RZ_CampPlace p, RZ_EDressAt at, RZ_Rng rng)
	{
		float a = rng.Float01();
		float b = rng.Float01();
		float c = rng.Float01();
		bool side = rng.Chance(0.5);
		RZ_CampSite site = p.m_Site;
		float spread = a * 2 - 1;

		if (at == RZ_EDressAt.YARD)
			return FacePoint(site, site.m_iYardFace, spread * 0.9, 3 + b * 7);

		if (at == RZ_EDressAt.FRONT)
			return FacePoint(site, FACE_FRONT, spread * 0.9, 3 + b * 6);

		if (at == RZ_EDressAt.BACK)
			return FacePoint(site, FACE_BACK, spread * 1.1, 10 + b * 8);

		if (at == RZ_EDressAt.SIDE)
			return FacePoint(site, site.m_iCookFace, spread * 0.8, 3 + b * 6);

		if (at == RZ_EDressAt.DOOR)
			return FacePoint(site, FACE_FRONT, site.m_fDoor + spread * 0.25, 2.5 + b * 2.5);

		if (at == RZ_EDressAt.WALLS)
		{
			if (c < 0.4)
				return FacePoint(site, FACE_BACK, spread, 10 + b * 10);

			if (c < 0.7)
				return FacePoint(site, FACE_LEFT, spread, 4 + b * 10);

			return FacePoint(site, FACE_RIGHT, spread, 4 + b * 10);
		}

		// The gate, or a closed entry: beside the road where it crosses the ring
		int entry = site.m_iMain;
		if (at == RZ_EDressAt.CLOSED)
		{
			int closed = site.m_aEntries.Count() - 1;
			entry = -1;
			if (closed > 0)
			{
				int pick = RZ_Util.PickIndex(p.m_iClosedNext, closed);
				entry = pick;
				if (pick >= site.m_iMain)
					entry = pick + 1;
			}
		}

		if (entry < 0)
			return FacePoint(site, FACE_FRONT, spread, 10 + b * 10);

		float sideSign = 1;
		if (side)
			sideSign = -1;

		vector e = site.m_aEntries[entry];
		vector beside = RZ_Compass.PointAt(e, site.m_aEntryRoad[entry] + 90 * sideSign, RZ_DressPlacer.ROAD_CLEAR + 0.5 + b * 3);
		return RZ_Util.GroundPos(RZ_Compass.PointAt(beside, RZ_Compass.Bearing(site.m_vCentre, e), spread * 5));
	}

	//------------------------------------------------------------------------------------------------
	//! The lookout's spot: a front corner of the building (the one nearer the gate first), facing
	//! the way in. From the ground: nobody is sent onto a roof.
	protected vector LookoutSpot(RZ_CampPlace p, int which)
	{
		RZ_CampSite site = p.m_Site;
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
	protected vector DoorSpot(RZ_CampPlace p)
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

		SpawnPending();
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
				Live(p, phase, step, shuffle, players);
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
	protected void Live(RZ_CampPlace p, int phase, float step, bool shuffle, array<IEntity> players)
	{
		Prune(p);
		float nearest = RZ_Util.NearestDistance(p.m_vPos, players);

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
		if (nearest > SLEEP_CLEAR)
		{
			if (night)
				PutToBed(p);
			else
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

		if (shuffle)
			Shuffle(p, phase);
	}

	//------------------------------------------------------------------------------------------------
	//! Workers whose body is gone or dead lose their job. Owned bodies stay listed until Leave.
	protected void Prune(RZ_CampPlace p)
	{
		for (int i = p.m_aWorkers.Count() - 1; i >= 0; i--)
		{
			RZ_CampWorker w = p.m_aWorkers[i];
			if (w.m_Unit && RZ_Util.CountAlive(w.m_Unit) > 0)
				continue;

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
			p.m_iShift++;

		// Bastion's roll: when the night watch is set, everyone stands at the board for a few minutes
		if (!first && phase == RZ_Territory.PHASE_NIGHT && p.m_eFaction == RZ_EFaction.MILITARY && p.m_eKind != RZ_ECampPlace.ROAD_POST)
			p.m_fGatherUntil = m_fClock + ROLL_SECONDS;

		AssignAll(p, phase);
	}

	//------------------------------------------------------------------------------------------------
	//! The routine: jobs in order for this faction and phase, and the job for everyone after them.
	protected int Slots(RZ_CampPlace p, int phase, notnull array<int> slots)
	{
		bool night = phase == RZ_Territory.PHASE_NIGHT;
		if (p.m_eKind == RZ_ECampPlace.ROAD_POST)
		{
			slots.Insert(JOB_LEAN);
			return JOB_SIT;
		}

		if (p.m_eFaction == RZ_EFaction.MILITARY)
		{
			// The roll, then the night watch: the nest and one at the fire; the rest asleep
			if (p.m_fGatherUntil > m_fClock)
			{
				slots.Insert(JOB_WATCH);
				return JOB_GATHER;
			}

			// Under the season's curfew: the gate and the lookout, nobody at the fire
			if (night)
			{
				slots.Insert(JOB_WATCH);
				if (IsCurfew())
					slots.Insert(JOB_WATCH);
				else
					slots.Insert(JOB_SIT);

				return JOB_OFF;
			}

			if (phase == RZ_Territory.PHASE_MEAL)
			{
				slots.Insert(JOB_WATCH);
				slots.Insert(JOB_PATROL);
				slots.Insert(JOB_SIT);
				slots.Insert(JOB_SIT);
				return JOB_SIT;
			}

			// The nest, two on the loop (in a building, the lookout between them), one at the
			// board, the phase's chore, one at the fire
			slots.Insert(JOB_WATCH);
			slots.Insert(JOB_PATROL);
			if (p.m_Site)
				slots.Insert(JOB_WATCH);

			slots.Insert(JOB_PATROL);
			slots.Insert(JOB_GATHER);
			AddChore(slots, phase, true, true, true);
			slots.Insert(JOB_SIT);
			return JOB_WORK;
		}

		if (p.m_eFaction == RZ_EFaction.BANDITS)
		{
			// Night: one lookout, the rest drinking at the fire
			if (night)
			{
				slots.Insert(JOB_WATCH);
				return JOB_SIT;
			}

			// Two at the barrier taking the toll, one strolling to the road and back; wood by day,
			// the pot before the meal (nobody washes)
			slots.Insert(JOB_LEAN);
			slots.Insert(JOB_LEAN);
			if (p.m_Site && p.m_eKind == RZ_ECampPlace.STRONGHOLD)
				slots.Insert(JOB_WATCH);

			slots.Insert(JOB_STROLL);
			AddChore(slots, phase, false, true, true);
			slots.Insert(JOB_SIT);
			return JOB_SIT;
		}

		if (p.m_eFaction == RZ_EFaction.SURVIVORS)
		{
			// The lookout at the door always; the dusk meal at the table; at night the rest inside
			slots.Insert(JOB_WATCH);
			if (night)
				return JOB_OFF;

			if (phase == RZ_Territory.PHASE_MEAL)
				return JOB_GATHER;

			// The phase's chore first (the wash, the wood, the cooking for the meal), then the rest
			AddChore(slots, phase, true, true, true);
			slots.Insert(JOB_WORK);
			slots.Insert(JOB_WORK);
			slots.Insert(JOB_WORK);
			return JOB_SIT;
		}

		// Halcyon: a guard pair at the door, researchers at the tables, the decon shower in the
		// morning; at night one guard and one at the cold box
		slots.Insert(JOB_WATCH);
		if (night)
		{
			slots.Insert(JOB_WORK);
			return JOB_OFF;
		}

		slots.Insert(JOB_WATCH);
		AddChore(slots, phase, true, false, false);
		slots.Insert(JOB_SIT);
		slots.Insert(JOB_SIT);
		slots.Insert(JOB_WORK);
		return JOB_WORK;
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
	protected static void AddChore(notnull array<int> slots, int phase, bool wash, bool chop, bool cook)
	{
		if (wash && phase == RZ_Territory.PHASE_MORNING)
			slots.Insert(JOB_WASH);
		else if (chop && phase == RZ_Territory.PHASE_DAY)
			slots.Insert(JOB_CHOP);
		else if (cook && phase == RZ_Territory.PHASE_EVENING)
			slots.Insert(JOB_COOK);
	}

	//------------------------------------------------------------------------------------------------
	//! Named people never walk away: no watch, patrol, stroll, errand or bed.
	protected static bool PersonMay(int job)
	{
		return job == JOB_SIT || job == JOB_WORK || job == JOB_GATHER || job == JOB_LEAN || job == JOB_COOK || job == JOB_WASH || job == JOB_CHOP;
	}

	//------------------------------------------------------------------------------------------------
	protected void AssignAll(RZ_CampPlace p, int phase)
	{
		foreach (RZ_CampWorker w : p.m_aWorkers)
		{
			ClearJob(p, w);
		}

		// Guards and extras in shift order (each shift change, someone else takes the watch),
		// then the named people
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
		int n = crew.Count();
		for (int i = 0; i < n; i++)
		{
			order.Insert(crew[(i + p.m_iShift) % n]);
		}

		order.InsertAll(people);

		array<int> slots = {};
		int rest = Slots(p, phase, slots);
		foreach (int job : slots)
		{
			foreach (int k, RZ_CampWorker cand : order)
			{
				if (cand.m_iRole == ROLE_PERSON && !PersonMay(job))
					continue;

				Give(p, cand, job);
				order.Remove(k);
				break;
			}
		}

		foreach (int r, RZ_CampWorker left : order)
		{
			Give(p, left, RestJob(left, rest, r));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The job for everyone after the routine's list: sitting and working by turns; a named
	//! person who would go to bed stays at work near the centre.
	protected int RestJob(RZ_CampWorker w, int rest, int index)
	{
		if (w.m_iRole == ROLE_PERSON && !PersonMay(rest))
			return JOB_WORK;

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
	protected void Give(RZ_CampPlace p, RZ_CampWorker w, int job)
	{
		ClearJob(p, w);
		w.m_iJob = job;
		IEntity group = GroupFor(w);
		if (!group)
			return;

		bool person = w.m_iRole == ROLE_PERSON;
		if (job == JOB_WATCH)
			Watch(p, w, group);
		else if (job == JOB_PATROL)
			Patrol(p, w, group);
		else if (job == JOB_SIT)
			AtPiece(p, w, group, RZ_EDressUse.SIT, person);
		else if (job == JOB_WORK)
			AtPiece(p, w, group, RZ_EDressUse.WORK, person);
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
			Errand(p, w, group, false);
		else if (job == JOB_COURIER)
			Errand(p, w, group, true);
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
		w.m_iJob = JOB_NONE;
	}

	//------------------------------------------------------------------------------------------------
	//! A free piece of this use (named people: only near the centre), or -1.
	protected int FreePiece(RZ_CampPlace p, RZ_EDressUse use, bool person)
	{
		foreach (int i, RZ_CampPiece piece : p.m_aPieces)
		{
			if (!piece.m_Entity || piece.m_eUse != use || piece.m_iUsed >= piece.m_iSeats)
				continue;

			if (person && RZ_Util.DistXZ(piece.m_vPos, p.m_vHome) > p.m_fPersonRadius)
				continue;

			return i;
		}

		return -1;
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
			Loiter(w, group, RZ_Util.RandomPointInRing(p.m_vHome, 4, 10));
			return;
		}

		UsePiece(p, w, group, idx);
	}

	//------------------------------------------------------------------------------------------------
	protected void UsePiece(RZ_CampPlace p, RZ_CampWorker w, IEntity group, int idx)
	{
		RZ_CampPiece piece = p.m_aPieces[idx];
		piece.m_iUsed++;
		w.m_iPiece = idx;

		IEntity wp = RZ_Util.SetGroupSmartAction(group, piece.m_Entity, piece.m_sTag, m_Settings.m_sSmartActionWaypoint);
		if (wp)
		{
			w.m_aWaypoints.Insert(wp);
			return;
		}

		Loiter(w, group, RZ_Util.RandomPointInRing(piece.m_vPos, 1.5, 3));
	}

	//------------------------------------------------------------------------------------------------
	//! Stand about at a spot: the loiter waypoint, else a plain move there.
	protected void Loiter(RZ_CampWorker w, IEntity group, vector at)
	{
		ResourceName prefab = m_Settings.m_sLoiterWaypoint;
		if (prefab.IsEmpty())
			prefab = m_GM.GetSpawner().GetMoveWaypointPrefab();

		IEntity wp = RZ_Util.AddGroupWaypoint(group, at, prefab, 4); // VERIFY: a loiter (timed defend) waypoint with a 4 m radius uses only the smart actions within it, and holds the group there
		if (wp)
			w.m_aWaypoints.Insert(wp);
	}

	//------------------------------------------------------------------------------------------------
	protected void MoveTo(RZ_CampWorker w, IEntity group, vector at)
	{
		IEntity wp = RZ_Util.AddGroupWaypoint(group, at, m_GM.GetSpawner().GetMoveWaypointPrefab(), 3);
		if (wp)
			w.m_aWaypoints.Insert(wp);
	}

	//------------------------------------------------------------------------------------------------
	//! The sentry: the nest's watch point, else an observation post: in a building the lookout at
	//! a front corner (a second watcher takes the other), else at the approach.
	protected void Watch(RZ_CampPlace p, RZ_CampWorker w, IEntity group)
	{
		int idx = FreePiece(p, RZ_EDressUse.WATCH, false);
		if (idx >= 0)
		{
			UsePiece(p, w, group, idx);
			return;
		}

		float reach = 18;
		if (p.m_eKind == RZ_ECampPlace.SAFEHOUSE)
			reach = 8;

		vector at = RZ_Compass.PointAt(p.m_vPos, p.m_fApproach - 20 + Math.RandomFloat(0, 40), reach);
		if (p.m_Site)
		{
			int others = 0;
			foreach (RZ_CampWorker o : p.m_aWorkers)
			{
				if (o != w && o.m_iJob == JOB_WATCH && o.m_iPiece < 0)
					others++;
			}

			at = LookoutSpot(p, others);
		}

		ResourceName prefab = m_Settings.m_sObservationWaypoint;
		if (prefab.IsEmpty())
		{
			Loiter(w, group, at);
			return;
		}

		IEntity wp = RZ_Util.AddGroupWaypoint(group, at, prefab, 3);
		if (wp)
			w.m_aWaypoints.Insert(wp);
	}

	//------------------------------------------------------------------------------------------------
	//! Two walk a loop of four points 40-80 m out. Without a cycle waypoint they walk it once and
	//! the next shuffle sends them round again.
	protected void Patrol(RZ_CampPlace p, RZ_CampWorker w, IEntity group)
	{
		ResourceName prefab = m_Settings.m_sPatrolWaypoint;
		if (prefab.IsEmpty())
			prefab = m_GM.GetSpawner().GetMoveWaypointPrefab();

		if (prefab.IsEmpty())
			return;

		RZ_Rng rng = new RZ_Rng(p.m_iSeed + p.m_iShift * 101 + 3);
		array<IEntity> loop = {};
		for (int i = 0; i < 4; i++)
		{
			vector at = RZ_Compass.PointAt(p.m_vCentre, p.m_fApproach + i * 90 + rng.Range(-20, 20), rng.Range(40, 80));
			IEntity wp = RZ_Util.SpawnPrefab(prefab, at);
			if (wp)
				loop.Insert(wp);
		}

		if (loop.IsEmpty())
			return;

		w.m_aWaypoints.InsertAll(loop);
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

		Loiter(w, group, RZ_Util.RandomPointInRing(at, 2, 4));
	}

	//------------------------------------------------------------------------------------------------
	//! A walk out and back: the Red Hand's stroll to the road, Halcyon's courier to the edge.
	protected void Errand(RZ_CampPlace p, RZ_CampWorker w, IEntity group, bool courier)
	{
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
	//! Night, nobody within 80 m: the off-shift guards and extras go to sleep (as data).
	protected void PutToBed(RZ_CampPlace p)
	{
		for (int i = p.m_aWorkers.Count() - 1; i >= 0; i--)
		{
			RZ_CampWorker w = p.m_aWorkers[i];
			if (w.m_iJob != JOB_OFF || !IsWorking(w) || (w.m_iRole != ROLE_GUARD && w.m_iRole != ROLE_EXTRA))
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
	//! Morning (any phase but night), nobody within 80 m: the sleepers come out of the tents.
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
		IEntity body = RZ_Util.SpawnPrefab(p.m_sExtraPrefab, BedSpot(p), Math.RandomFloat(0, 360));
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
	//! walks out to the edge and back, then goes back to the job it had.
	protected void SendCourier(RZ_CampPlace p)
	{
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

		int after = pick.m_iJob;
		Give(p, pick, JOB_COURIER);
		pick.m_iAfterErrand = after;
	}

	//------------------------------------------------------------------------------------------------
	//! Every few minutes: anyone idle (back from an errand, done talking, out of a fight) goes back
	//! to work, and one person moves to another seat or job.
	protected void Shuffle(RZ_CampPlace p, int phase)
	{
		array<int> unused = {};
		int rest = Slots(p, phase, unused);
		array<RZ_CampWorker> movers = {};
		foreach (int i, RZ_CampWorker w : p.m_aWorkers)
		{
			if (!IsWorking(w))
				continue;

			bool idle = w.m_iJob == JOB_NONE || (w.m_Group && RZ_Util.CountGroupWaypoints(w.m_Group) == 0 && w.m_iJob != JOB_OFF);
			if (idle)
			{
				// A patrol without a cycle goes round again; the courier goes back to its job
				int again = w.m_iJob;
				if (again == JOB_COURIER)
					again = w.m_iAfterErrand;

				if (again == JOB_NONE || again == JOB_COURIER)
					again = RestJob(w, rest, i);

				Give(p, w, again);
				continue;
			}

			if (p.m_fGatherUntil == 0 && (w.m_iJob == JOB_SIT || w.m_iJob == JOB_WORK || w.m_iJob == JOB_LEAN))
				movers.Insert(w);
		}

		if (movers.IsEmpty())
			return;

		RZ_CampWorker mover = movers.GetRandomElement();
		int next = JOB_SIT;
		if (mover.m_iJob == JOB_SIT)
			next = JOB_WORK;

		Give(p, mover, next);
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
