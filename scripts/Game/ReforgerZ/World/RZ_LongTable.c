//! The Long Table (docs/DESIGN.md, "The Long Table"; who they are: docs/LORE.md; how players deal
//! with them: docs/REPUTATION.md). Some seasons, one small family in one farmhouse in an empty
//! part of the map. They're kind to strangers, there's always a place set at supper, and they
//! eat people. Never described: the writing stays on ordinary things (the extra bowl, the boots
//! by the door, the names read at grace, jars labelled by the day).
//!
//! Not a faction: no territory, strength, morale, truck, leader seat or stronghold, and no Fame,
//! Infamy or Favor with them. The house is data (one record, ticked every 10 minutes); its
//! people get bodies (the puzzle civilian) only while a player is within 300 m. What it takes is
//! capped data too: never from a safehouse or a holding, never a player. Its two questlines
//! (Missions/Questlines/LongTable/) read and change it through the public methods below, and its
//! state is published as season-wide world flags ("table.*", see docs/QUESTLINES.md, Crossings).

//------------------------------------------------------------------------------------------------
enum RZ_ETableState
{
	NONE,		// no house this season (not rolled yet, rolled none, or off)
	ACTIVE,		// the farm is open: a place set at supper
	QUIET,		// moving: nobody at the farm for a real day
	EXPOSED,	// everyone knows: a real day before Bastion burns it, or it moves
	BROKEN		// gone for the season
}

//------------------------------------------------------------------------------------------------
enum RZ_ETableRole
{
	MOTHER,		// the kitchen, and so the house
	HOST,		// the elder son, who meets people on the road
	SON,		// the quiet son
	BOY,		// not theirs: he came with the mouths
	OTHER		// a cousin, an uncle, a lodger who stayed
}

//------------------------------------------------------------------------------------------------
//! How the house ended (RZ_LongTable.BreakUp).
enum RZ_ETableEnd
{
	WALKED_OUT,	// the Haven walked them to the edge of town
	BURNT,		// Bastion's guns
	STUDIED,	// the mother to the lab, the rest scattered
	BY_HAND,	// a fight
	SCATTERED	// found out, with nowhere left to go
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > World > Long Table.
[BaseContainerProps()]
class RZ_LongTableSettings
{
	[Attribute("1", desc: "The Long Table on: some seasons, one small family in one farmhouse in an empty part of the map, and its two questlines (docs/DESIGN.md, The Long Table). Off = no house, nothing taken, its questlines never start")]
	bool m_bEnabled;

	[Attribute("0.75", desc: "Chance that a season has a Long Table (rolled once, at the season's first start)")]
	float m_fChance;

	[Attribute("6", desc: "Hunger the family gains per hour of play (0-100, x season speed). It takes someone only above 50, and each one lowers it by 40")]
	float m_fHungerPerHour;

	[Attribute("3", desc: "Hours of play between its chances to take someone while it's hungry (divided by the season speed; twice as long once its buyer is gone or it was paid to move)")]
	float m_fTakeHours;

	[Attribute("300", desc: "The family's people exist as bodies (Puzzles > Civilian) only while a player is this close (m) to the farm")]
	float m_fBodyRadius;

	[Attribute("1", desc: "The host meets lone players on the road within 1.5 km of the farm, once a session each: true road news and an invitation")]
	bool m_bHostOnRoad;

	[Attribute("1", desc: "The guest rule: a guest who ate there and is still at the farm after 20:30, alone and not infected, is set on. A fight with Missions > Events > Rival group (else Missions > Survivor group); without either, a narrow escape")]
	bool m_bGuestRule;

	[Attribute("mag,magazine,ammo,cartridge,bandage,medkit,firstaid,first_aid,morphine,saline,tourniquet", desc: "Trade: what the family takes for a jar (prefab name keywords, lower case: ammunition and medicine). The jar is a food item from Interactions > Search > Loot pool")]
	string m_sTradeWords;
}

//------------------------------------------------------------------------------------------------
class RZ_TableMemberSave
{
	int role;
	string name;
	bool alive = true;
}

//------------------------------------------------------------------------------------------------
class RZ_TableMemorySave
{
	string ident;
	int life;
	int meals;
	int flags;
	int bowlUnix;
}

//------------------------------------------------------------------------------------------------
class RZ_TableTellSave
{
	float x;
	float z;
	string text;
	int untilUnix;
	bool found;
}

//------------------------------------------------------------------------------------------------
class RZ_LongTableSave
{
	bool rolled;
	int state;
	int settlement = -1;
	float x;
	float z;
	string surname;
	float hunger;
	float suspicion;
	float takeTimer;
	float clock;
	int stateUnix;
	int exposedTo = -1;
	bool everExposed;
	bool moved;
	bool slowed;
	bool burnt;
	bool rumours;
	bool heard;
	int wandererUnix;
	int taken;
	string lostName;
	string lostFrom;
	string lostTo;
	int lostRole = -1;
	int lostUnix;
	float lostX;
	float lostZ;
	int c1Begun;
	string offerIdent;
	ref array<string> noticers = {};
	ref array<string> consumed = {};
	ref array<ref RZ_TableMemberSave> members = {};
	ref array<ref RZ_TableMemorySave> memory = {};
	ref array<ref RZ_TableTellSave> tells = {};
}

//------------------------------------------------------------------------------------------------
//! One of the family. Data; a body only near players.
class RZ_TableMember
{
	RZ_ETableRole m_eRole;
	string m_sName;
	bool m_bAlive = true;

	// Runtime
	IEntity m_Body;
	bool m_bOnRoad;		// the host, meeting someone on the road
}

//------------------------------------------------------------------------------------------------
//! What the family remembers of one character (identity and life), in three words and a few
//! more: guest, family, crossed (docs/REPUTATION.md, The Long Table).
class RZ_TableMemory
{
	string m_sIdent;
	int m_iLife;
	int m_iMeals;
	int m_iFlags;
	int m_iBowlUnix;
}

//------------------------------------------------------------------------------------------------
//! One visit to the farm (runtime): from coming within the body radius to leaving it.
class RZ_TableVisit
{
	bool m_bNear;
	bool m_bAte;
	bool m_bTraded;
	float m_fAteAt;
}

//------------------------------------------------------------------------------------------------
//! A tell on the road (a boot on the verge): data, a prop only while a player is near.
class RZ_TableTell
{
	vector m_vPos;
	string m_sText;
	int m_iUntilUnix;
	bool m_bFound;
	IEntity m_Prop;
}

//------------------------------------------------------------------------------------------------
//! Someone who died to the infected lately (A Place at the Table: "the dead"). Runtime only.
class RZ_TableDeath
{
	vector m_vPos;
	string m_sWho;
	float m_fAt;
}

//------------------------------------------------------------------------------------------------
//! People who left a safehouse onto the roads alone lately (runtime only).
class RZ_TableLeaver
{
	vector m_vFrom;
	string m_sHouse;
	int m_iCount;
	float m_fAt;
}

//------------------------------------------------------------------------------------------------
//! The house. Owned by the game mode, ticked on the server tick.
class RZ_LongTable
{
	static const int TAG = -11;				// RZ_PuzzleObjectComponent / RZ_ClueComponent mission id
	static const int CLUE_TELL = 1000;		// clue indices from here on are tells on the road

	// What the family remembers of a character (this life)
	static const int MEM_GUEST = 1;			// ate with them
	static const int MEM_FAMILY = 2;		// A Place at the Table: sat and said a name
	static const int MEM_CROSSED = 4;		// refused them, or took proof to someone
	static const int MEM_BLESSED = 8;		// declared a bite: never set on while infected
	static const int MEM_RULE_USED = 16;	// the guest rule came for them (once a life)
	static const int MEM_WALKED = 32;		// walked away: the host stops meeting them on the road
	static const int MEM_DAYLIGHT = 64;		// refused the stray: the guest rule holds in daylight too
	static const int MEM_UNDERCOVER = 128;	// A Place at the Table: walked the stray somewhere safe
	static const int MEM_DEAD = 256;		// (you) told the host where the dead were
	static const int MEM_DYING = 512;		// (you) sent the dying to the farm
	static const int MEM_STRAY = 1024;		// (you) brought the stray
	static const int MEM_TOLD = 2048;		// took proof to a faction
	static const int MEM_SET_ON = 4096;		// the family set on them: shooting them costs nothing
	static const int MEM_KNOWS = 8192;		// knows what the house is (a questline): shooting them costs nothing
	static const int MEM_BOOTS = 16384;		// saw the boots by the door (once a life)
	static const int MEM_OFFERED = 32768;	// the host made his offer (A Place at the Table)

	static const string C1_KEY = "table";		// The House That Went Quiet
	static const string C2_KEY = "place";		// A Place at the Table
	static const string SLOT = "cannibals";
	static const int C1_ENDING_PAID = 3;		// RZ_Quest_HouseWentQuiet.E_PAID: the house was paid to move

	protected static const float DATA_SECONDS = 600;
	protected static const float HOST_CHECK = 30;
	protected static const float TELL_CHECK = 10;
	protected static const float HOUSE_GAP = 2000;		// from every holding, safehouse and the lab
	protected static const float HUB_RADIUS = 1600;
	protected static const float MOVE_MIN = 2000;
	protected static const float MOVE_MAX = 5000;
	protected static const float ROAD_RADIUS = 1500;	// the host's road, and the routes it takes from
	protected static const float TAKE_CLEAR = 600;		// nobody is taken with a player this close
	protected static const float GUEST_RADIUS = 30;
	protected static const float GUEST_ALONE = 100;
	protected static const float GUEST_HOUR = 20.5;
	protected static const float GUEST_GRACE = 60;		// seconds after a meal before the rule can come
	protected static const float TELL_RADIUS = 150;
	protected static const float DEATH_SECONDS = 3600;
	protected static const float LEAVER_SECONDS = 21600;
	protected static const float FIGHT_RADIUS = 200;
	protected static const float FIGHT_AWAY = 60;
	protected static const int DAY = 86400;
	protected static const float START_HUNGER = 45;
	protected static const float START_SUSPICION = 10;
	protected static const float TAKE_HUNGER = 40;
	protected static const float TAKE_SUSPICION = 10;
	protected static const float TELL_SUSPICION = 5;
	protected static const float RUMOUR_SUSPICION = 30;
	protected static const float MOVE_SUSPICION = 80;
	protected static const float LIST_READ_SUSPICION = 30;	// H1's The names read (list.read)
	static const int ROLE_NO_BAG = -2;			// Taken: H2's exile, with nothing to carry (no satchel, no trail)
	protected static const int SHOT_INFAMY = 6;
	protected static const int FAMILY_HAVEN_INFAMY = 40;
	protected static const int FAMILY_BASTION_INFAMY = 20;
	protected static const int MAX_TELLS = 4;
	protected static const int MAX_NOTICERS = 24;

	protected RZ_GameModeComponent m_GM;
	protected RZ_LongTableSettings m_Settings;
	protected ref RZ_LongTableSave m_Restored;

	protected bool m_bRolled;
	protected RZ_ETableState m_eState = RZ_ETableState.NONE;
	protected int m_iSettlement = -1;
	protected vector m_vHouse;
	protected string m_sSurname;
	protected float m_fHunger;
	protected float m_fSuspicion;
	protected float m_fTakeTimer;
	protected float m_fClock;
	protected int m_iStateUnix;
	protected int m_iExposedTo = -1;
	protected bool m_bEverExposed;
	protected bool m_bMoved;
	protected bool m_bSlowed;
	protected bool m_bBurnt;
	protected bool m_bRumours;
	protected bool m_bHeard;
	protected int m_iWandererUnix;
	protected int m_iTaken;
	protected string m_sLostName;
	protected string m_sLostFrom;
	protected string m_sLostTo;
	protected int m_iLostRole = -1;
	protected int m_iLostUnix;
	protected vector m_vLost;
	protected int m_iC1Begun;				// begins since the questline's last ending
	protected float m_fC1EndedAt = -1;		// play clock when it last stopped running (runtime)
	protected string m_sOfferIdent;
	protected ref array<string> m_aNoticers = {};
	protected ref array<string> m_aConsumed = {};
	protected ref array<ref RZ_TableMember> m_aMembers = {};
	protected ref array<ref RZ_TableMemory> m_aMemory = {};
	protected ref array<ref RZ_TableTell> m_aTells = {};

	// Runtime
	protected ref map<int, ref RZ_TableVisit> m_mVisits = new map<int, ref RZ_TableVisit>();
	protected ref array<string> m_aMetHost = {};			// identities the host met on the road this session
	protected ref array<ref RZ_TableDeath> m_aDeaths = {};
	protected ref array<ref RZ_TableLeaver> m_aLeavers = {};
	protected bool m_bSpawned;
	protected int m_iSceneHold = -1;						// a questline stage mission has the farm
	protected int m_iRoadPlayer = -1;						// the host is out meeting this player
	protected float m_fRoadLeft;
	protected float m_fDataTimer;
	protected float m_fHostTimer;
	protected float m_fTellTimer;
	protected bool m_bPublished;
	protected bool m_bC1Was;
	protected bool m_bPurgeDue;							// found out: the Red Hand purges its buyer next round

	// The fight (the guest rule, or a questline's)
	protected IEntity m_FightGroup;
	protected ref array<IEntity> m_aFightMen = {};
	protected ref array<bool> m_aFightDead = {};
	protected ref array<int> m_aFightTargets = {};
	protected bool m_bFightSetOn;
	protected float m_fFightAge;
	protected float m_fFightAway;
	protected int m_iFightResult = -1;	// -1 none or running, 0 over (players gone), 1 the family's fighters are dead

	//------------------------------------------------------------------------------------------------
	void RZ_LongTable(RZ_GameModeComponent gm, RZ_LongTableSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled() { return m_Settings.m_bEnabled; }
	RZ_ETableState GetState() { return m_eState; }
	vector GetHousePos() { return m_vHouse; }
	int GetSettlement() { return m_iSettlement; }
	string GetSurname() { return m_sSurname; }
	float GetSuspicion() { return m_fSuspicion; }
	int GetExposedTo() { return m_iExposedTo; }
	bool WasExposed() { return m_bEverExposed; }
	bool HasMoved() { return m_bMoved; }
	string GetLostName() { return m_sLostName; }
	string GetLostFrom() { return m_sLostFrom; }
	string GetLostTo() { return m_sLostTo; }
	vector GetLostPos() { return m_vLost; }
	int GetLostRole() { return m_iLostRole; }
	string GetOfferIdent() { return m_sOfferIdent; }
	bool WasHeard() { return m_bHeard; }

	//------------------------------------------------------------------------------------------------
	//! There is a house this season: open, moving or found out (not broken up).
	bool Exists()
	{
		return m_eState == RZ_ETableState.ACTIVE || m_eState == RZ_ETableState.QUIET || m_eState == RZ_ETableState.EXPOSED;
	}

	//------------------------------------------------------------------------------------------------
	//! Someone is at the farm (open or found out, not moving).
	bool IsOpen()
	{
		return m_eState == RZ_ETableState.ACTIVE || m_eState == RZ_ETableState.EXPOSED;
	}

	//------------------------------------------------------------------------------------------------
	bool IsExposed()
	{
		return m_eState == RZ_ETableState.EXPOSED;
	}

	//------------------------------------------------------------------------------------------------
	//! The farm the Haven sent its mouths to (H1 The List): the house's first farm, if it hasn't
	//! moved, as a settlement index; -1 in a season without one or once it has moved away.
	int MouthsFarm()
	{
		if (m_eState == RZ_ETableState.NONE || m_bMoved)
			return -1;

		return m_iSettlement;
	}

	//------------------------------------------------------------------------------------------------
	//! Somebody still lives at the farm outside settlement idx (open or found out, not moving).
	bool LivesAt(int idx)
	{
		return IsOpen() && idx >= 0 && idx == m_iSettlement;
	}

	//------------------------------------------------------------------------------------------------
	bool WasBurnt() { return m_bBurnt; }

	//------------------------------------------------------------------------------------------------
	//! "Levie", the settlement the farm is past.
	string GetTownName()
	{
		RZ_Settlement s = m_GM.GetSettlement(m_iSettlement);
		if (s)
			return s.m_sName;

		return "grid " + RZ_SettlementScanner.GridReference(m_vHouse);
	}

	//------------------------------------------------------------------------------------------------
	//! Within radius of the farm (any state but none or broken up).
	bool IsNearHouse(vector pos, float radius)
	{
		return Exists() && RZ_Util.DistXZ(pos, m_vHouse) <= radius;
	}

	//------------------------------------------------------------------------------------------------
	//! The farm's settlement (world events keep out of it: RZ_EventHelp.IsLivedIn).
	bool IsHouseTown(int idx)
	{
		return Exists() && idx >= 0 && idx == m_iSettlement;
	}

	// ================================================================ The family
	//------------------------------------------------------------------------------------------------
	//! The first living member with this role, null if none.
	RZ_TableMember Find(int role)
	{
		foreach (RZ_TableMember m : m_aMembers)
		{
			if (m.m_eRole == role && m.m_bAlive)
				return m;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	bool IsAlive(int role)
	{
		return Find(role) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! A living member's first name, "" if nobody of that role is left.
	string NameOf(int role)
	{
		RZ_TableMember m = Find(role);
		if (!m)
			return "";

		return m.m_sName;
	}

	//------------------------------------------------------------------------------------------------
	//! Whoever meets people on the road now: the host, else the quiet son, else another of them.
	RZ_TableMember Greeter()
	{
		RZ_TableMember m = Find(RZ_ETableRole.HOST);
		if (!m)
			m = Find(RZ_ETableRole.SON);

		if (!m)
			m = Find(RZ_ETableRole.OTHER);

		return m;
	}

	//------------------------------------------------------------------------------------------------
	string GreeterName()
	{
		RZ_TableMember m = Greeter();
		if (!m)
			return NameOf(RZ_ETableRole.MOTHER);

		return m.m_sName;
	}

	//------------------------------------------------------------------------------------------------
	//! Grown-ups still at the farm (the mother included).
	int CountAdults()
	{
		int n = 0;
		foreach (RZ_TableMember m : m_aMembers)
		{
			if (m.m_bAlive && m.m_eRole != RZ_ETableRole.BOY)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	int CountAlive()
	{
		int n = 0;
		foreach (RZ_TableMember m : m_aMembers)
		{
			if (m.m_bAlive)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! The questlines' fight prefab: Missions > Events > Rival group, else Missions > Survivor group.
	ResourceName FightPrefab()
	{
		return RZ_Event_RivalSurvivors.Prefab(m_GM);
	}

	// ================================================================ What they remember of you
	//------------------------------------------------------------------------------------------------
	//! This character's memory (their identity, this life), made if it's new. Null without one.
	RZ_TableMemory Memory(int playerId)
	{
		if (playerId <= 0)
			return null;

		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		if (profile.m_sIdentity.IsEmpty())
			return null;

		foreach (RZ_TableMemory mem : m_aMemory)
		{
			if (mem.m_sIdent != profile.m_sIdentity)
				continue;

			// A new life: they don't know this face
			if (mem.m_iLife != profile.m_iLives)
			{
				mem.m_iLife = profile.m_iLives;
				mem.m_iMeals = 0;
				mem.m_iFlags = 0;
				mem.m_iBowlUnix = 0;
			}

			return mem;
		}

		RZ_TableMemory made = new RZ_TableMemory();
		made.m_sIdent = profile.m_sIdentity;
		made.m_iLife = profile.m_iLives;
		m_aMemory.Insert(made);
		return made;
	}

	//------------------------------------------------------------------------------------------------
	bool Remembers(int playerId, int flag)
	{
		RZ_TableMemory mem = Memory(playerId);
		return mem && (mem.m_iFlags & flag) != 0;
	}

	//------------------------------------------------------------------------------------------------
	void Remember(int playerId, int flag)
	{
		RZ_TableMemory mem = Memory(playerId);
		if (mem)
			mem.m_iFlags = mem.m_iFlags | flag;
	}

	//------------------------------------------------------------------------------------------------
	void Forget(int playerId, int flag)
	{
		RZ_TableMemory mem = Memory(playerId);
		if (mem && (mem.m_iFlags & flag) != 0)
			mem.m_iFlags = mem.m_iFlags - flag;
	}

	//------------------------------------------------------------------------------------------------
	//! Meals eaten at the table this life.
	int Meals(int playerId)
	{
		RZ_TableMemory mem = Memory(playerId);
		if (!mem)
			return 0;

		return mem.m_iMeals;
	}

	//------------------------------------------------------------------------------------------------
	//! A questline's player knows what the house is: shooting the family costs them nothing now.
	void Sanction(int playerId)
	{
		Remember(playerId, MEM_KNOWS);
	}

	//------------------------------------------------------------------------------------------------
	//! Someone who noticed something (a rumour, a tell, a meal): the questline's first stage is
	//! offered to them.
	void Notice(int playerId)
	{
		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		if (ident.IsEmpty() || m_aNoticers.Contains(ident))
			return;

		m_aNoticers.Insert(ident);
		while (m_aNoticers.Count() > MAX_NOTICERS)
		{
			m_aNoticers.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The player ids of everyone online who noticed something.
	void GetNoticers(notnull array<int> result)
	{
		RZ_Questlines q = m_GM.GetQuestlines();
		if (!q)
			return;

		foreach (string ident : m_aNoticers)
		{
			int playerId = q.OnlineId(ident);
			if (playerId > 0 && !result.Contains(playerId))
				result.Insert(playerId);
		}
	}

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	void Restore(RZ_LongTableSave saved)
	{
		m_Restored = saved;
	}

	//------------------------------------------------------------------------------------------------
	RZ_LongTableSave Save()
	{
		RZ_LongTableSave s = new RZ_LongTableSave();
		s.rolled = m_bRolled;
		s.state = m_eState;
		s.settlement = m_iSettlement;
		s.x = m_vHouse[0];
		s.z = m_vHouse[2];
		s.surname = m_sSurname;
		s.hunger = m_fHunger;
		s.suspicion = m_fSuspicion;
		s.takeTimer = m_fTakeTimer;
		s.clock = m_fClock;
		s.stateUnix = m_iStateUnix;
		s.exposedTo = m_iExposedTo;
		s.everExposed = m_bEverExposed;
		s.moved = m_bMoved;
		s.slowed = m_bSlowed;
		s.burnt = m_bBurnt;
		s.rumours = m_bRumours;
		s.heard = m_bHeard;
		s.wandererUnix = m_iWandererUnix;
		s.taken = m_iTaken;
		s.lostName = m_sLostName;
		s.lostFrom = m_sLostFrom;
		s.lostTo = m_sLostTo;
		s.lostRole = m_iLostRole;
		s.lostUnix = m_iLostUnix;
		s.lostX = m_vLost[0];
		s.lostZ = m_vLost[2];
		s.c1Begun = m_iC1Begun;
		s.offerIdent = m_sOfferIdent;
		foreach (string ident : m_aNoticers)
		{
			s.noticers.Insert(ident);
		}

		foreach (string key : m_aConsumed)
		{
			s.consumed.Insert(key);
		}

		foreach (RZ_TableMember m : m_aMembers)
		{
			RZ_TableMemberSave ms = new RZ_TableMemberSave();
			ms.role = m.m_eRole;
			ms.name = m.m_sName;
			ms.alive = m.m_bAlive;
			s.members.Insert(ms);
		}

		foreach (RZ_TableMemory mem : m_aMemory)
		{
			RZ_TableMemorySave ys = new RZ_TableMemorySave();
			ys.ident = mem.m_sIdent;
			ys.life = mem.m_iLife;
			ys.meals = mem.m_iMeals;
			ys.flags = mem.m_iFlags;
			ys.bowlUnix = mem.m_iBowlUnix;
			s.memory.Insert(ys);
		}

		foreach (RZ_TableTell t : m_aTells)
		{
			RZ_TableTellSave ts = new RZ_TableTellSave();
			ts.x = t.m_vPos[0];
			ts.z = t.m_vPos[2];
			ts.text = t.m_sText;
			ts.untilUnix = t.m_iUntilUnix;
			ts.found = t.m_bFound;
			s.tells.Insert(ts);
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	protected void LoadRestored()
	{
		RZ_LongTableSave s = m_Restored;
		m_bRolled = s.rolled;
		m_eState = s.state;
		m_iSettlement = s.settlement;
		m_vHouse = RZ_Util.GroundPos(Vector(s.x, 0, s.z));
		m_sSurname = s.surname;
		m_fHunger = s.hunger;
		m_fSuspicion = s.suspicion;
		m_fTakeTimer = s.takeTimer;
		m_fClock = s.clock;
		m_iStateUnix = s.stateUnix;
		m_iExposedTo = s.exposedTo;
		m_bEverExposed = s.everExposed;
		m_bMoved = s.moved;
		m_bSlowed = s.slowed;
		m_bBurnt = s.burnt;
		m_bRumours = s.rumours;
		m_bHeard = s.heard;
		m_iWandererUnix = s.wandererUnix;
		m_iTaken = s.taken;
		m_sLostName = s.lostName;
		m_sLostFrom = s.lostFrom;
		m_sLostTo = s.lostTo;
		m_iLostRole = s.lostRole;
		m_iLostUnix = s.lostUnix;
		m_vLost = Vector(s.lostX, 0, s.lostZ);
		m_iC1Begun = s.c1Begun;
		m_sOfferIdent = s.offerIdent;
		if (s.noticers)
		{
			foreach (string ident : s.noticers)
			{
				m_aNoticers.Insert(ident);
			}
		}

		if (s.consumed)
		{
			foreach (string key : s.consumed)
			{
				m_aConsumed.Insert(key);
			}
		}

		if (s.members)
		{
			foreach (RZ_TableMemberSave ms : s.members)
			{
				if (!ms)
					continue;

				RZ_TableMember m = new RZ_TableMember();
				m.m_eRole = ms.role;
				m.m_sName = ms.name;
				m.m_bAlive = ms.alive;
				m_aMembers.Insert(m);
			}
		}

		if (s.memory)
		{
			foreach (RZ_TableMemorySave ys : s.memory)
			{
				if (!ys || ys.ident.IsEmpty())
					continue;

				RZ_TableMemory mem = new RZ_TableMemory();
				mem.m_sIdent = ys.ident;
				mem.m_iLife = ys.life;
				mem.m_iMeals = ys.meals;
				mem.m_iFlags = ys.flags;
				mem.m_iBowlUnix = ys.bowlUnix;
				m_aMemory.Insert(mem);
			}
		}

		if (s.tells)
		{
			foreach (RZ_TableTellSave ts : s.tells)
			{
				if (!ts)
					continue;

				RZ_TableTell t = new RZ_TableTell();
				t.m_vPos = RZ_Util.GroundPos(Vector(ts.x, 0, ts.z));
				t.m_sText = ts.text;
				t.m_iUntilUnix = ts.untilUnix;
				t.m_bFound = ts.found;
				m_aTells.Insert(t);
			}
		}

		// A house saved without its people (or broken off mid-move) is no house
		if (Exists() && (m_aMembers.IsEmpty() || !m_GM.GetSettlement(m_iSettlement)))
			m_eState = RZ_ETableState.NONE;
	}

	// ================================================================ Setup
	//------------------------------------------------------------------------------------------------
	//! After the settlements, the safehouses, the leaders and territory exist. A season rolls once
	//! for a house; the saved one comes back with its people, hunger, suspicion and memories.
	void Setup()
	{
		if (!m_Settings.m_bEnabled || m_GM.GetSettlements().IsEmpty())
		{
			m_Restored = null;
			RZ_Log.Info("Long Table: off.");
			return;
		}

		string source = "saved";
		if (m_Restored)
			LoadRestored();

		m_Restored = null;
		if (!m_bRolled)
		{
			m_bRolled = true;
			source = "new";
			if (Math.RandomFloat01() < Math.Clamp(m_Settings.m_fChance, 0, 1))
				Found();
			else
				RZ_Log.Info("Long Table: none this season (the roll).");
		}

		Publish();
		if (!Exists())
		{
			if (m_eState == RZ_ETableState.BROKEN)
				RZ_Log.Info("Long Table: broken up this season.");

			return;
		}

		RZ_Log.Info(string.Format("Long Table: the %1 farm past %2 at grid %3 (%4), %5 of them, %6, hunger %7, suspicion %8.",
			m_sSurname, GetTownName(), RZ_SettlementScanner.GridReference(m_vHouse), source, CountAlive(), StateName(), RZ_Util.RoundToInt(m_fHunger), RZ_Util.RoundToInt(m_fSuspicion)));
	}

	//------------------------------------------------------------------------------------------------
	protected string StateName()
	{
		switch (m_eState)
		{
			case RZ_ETableState.ACTIVE: return "open";
			case RZ_ETableState.QUIET: return "moving";
			case RZ_ETableState.EXPOSED: return "found out";
			case RZ_ETableState.BROKEN: return "broken up";
		}

		return "none";
	}

	//------------------------------------------------------------------------------------------------
	//! A new house: the emptiest small place on the map, its people seeded.
	protected void Found()
	{
		int idx = PickTown(vector.Zero, 0, 0);
		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (!s)
		{
			RZ_Log.Info("Long Table: none this season (no empty place far enough from everyone on this map).");
			return;
		}

		m_iSettlement = idx;
		m_vHouse = PickHouse(s);
		m_fHunger = START_HUNGER;
		m_fSuspicion = START_SUSPICION;
		m_eState = RZ_ETableState.ACTIVE;
		m_iStateUnix = RZ_Guidance.UnixNow();
		GenerateFamily();
	}

	//------------------------------------------------------------------------------------------------
	//! Four to seven of them: the mother, the host, the quiet son, the boy, and up to three more.
	protected void GenerateFamily()
	{
		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		m_sSurname = RZ_NoteText.Surname(rng);
		array<string> mothers = {"Ada", "Marta", "Irena", "Vera", "Hana", "Olga", "Edith", "Agnes", "Ruth", "Zofia"};
		array<string> sons = {"Pavel", "Karel", "Josef", "Milan", "Viktor", "Emil", "Anton", "Oskar", "Jakub", "Bruno"};
		array<string> boys = {"Tomik", "Jiri", "Petr", "Ondra", "Lukas", "Filip", "Matej", "Vit"};
		// A cousin, an uncle, a lodger who stayed (men: whoever keeps the road is "he")
		array<string> others = {"Bohus", "Stan", "Kamil", "Ota", "Lev", "Hynek", "Ales", "Dusan"};
		rng.ShuffleStrings(sons);
		rng.ShuffleStrings(others);

		AddMember(RZ_ETableRole.MOTHER, rng.Pick(mothers));
		AddMember(RZ_ETableRole.HOST, sons[0]);
		AddMember(RZ_ETableRole.SON, sons[1]);
		AddMember(RZ_ETableRole.BOY, rng.Pick(boys));
		int more = rng.Int(0, 4);
		for (int i = 0; i < more; i++)
		{
			AddMember(RZ_ETableRole.OTHER, others[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A man's first name (an exile, a stranger on the road: the writing calls them "he").
	static string ManName(RZ_Rng rng)
	{
		array<string> names = {"Pavel", "Henri", "Tomas", "Emil", "Anton", "Viktor", "Karel", "Oskar", "Milan", "Jakub", "Bruno", "Leon"};
		return rng.Pick(names);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddMember(int role, string name)
	{
		RZ_TableMember m = new RZ_TableMember();
		m.m_eRole = role;
		m.m_sName = name;
		m_aMembers.Insert(m);
	}

	//------------------------------------------------------------------------------------------------
	//! The small settlement with the fewest towns within 1.6 km, infection under 40, at least
	//! 2 km from every holding, leader seat, safehouse and the lab, away from the quarantine line.
	//! Small maps relax it a step at a time (down to 400 m). With `near` set (a move), 2-5 km from there.
	protected int PickTown(vector near, float minFrom, float maxFrom)
	{
		// The last two steps are for small maps like Arland (about 4 km across), where every place
		// has someone within 700 m: the emptiest place left, still never a lived-in town
		array<float> gaps = {HOUSE_GAP, 1500, 1000, 700, 500, 400};
		array<float> infections = {40, 50, 60, 70, 80, 80};
		array<int> sizes = {1, 2, 2, 3, 2, 2};
		for (int pass = 0; pass < gaps.Count(); pass++)
		{
			int best = -1;
			int bestHub = 1000;
			float bestTie = -1;
			foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
			{
				if (!TownFits(i, s, gaps[pass], infections[pass], sizes[pass]))
					continue;

				if (maxFrom > 0)
				{
					float d = RZ_Util.DistXZ(near, s.m_vCenter);
					if (d < minFrom || d > maxFrom)
						continue;
				}

				int hub = Neighbours(i);
				float tie = Math.RandomFloat01();
				if (hub < bestHub || (hub == bestHub && tie > bestTie))
				{
					best = i;
					bestHub = hub;
					bestTie = tie;
				}
			}

			if (best >= 0)
				return best;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	protected bool TownFits(int idx, RZ_Settlement s, float gap, float maxInfection, int maxSize)
	{
		if (s.m_fInfection >= maxInfection || s.m_iSize > maxSize || s.m_bHasLab)
			return false;

		if (RZ_Perimeter.IsNearLineAt(s.m_vCenter))
			return false;

		if (idx == m_GM.GetLabSettlement() || (m_GM.HasLab() && RZ_Util.DistXZ(s.m_vCenter, m_GM.GetLabPosition()) < gap))
			return false;

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
		{
			if (houses.IsSafehouseTown(idx))
				return false;

			for (int h = 0; h < houses.Count(); h++)
			{
				RZ_Safehouse house = houses.Get(h);
				if (house && RZ_Util.DistXZ(house.m_vPos, s.m_vCenter) < gap)
					return false;
			}
		}

		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			if (territory.FindInTown(idx) || territory.FindHolding(-1, s.m_vCenter, gap))
				return false;
		}

		// Leader seats (interim posts with territory off) and the quartermasters count as holdings
		RZ_Leaders leaders = m_GM.GetLeaders();
		if (leaders && leaders.FindNearest(-1, s.m_vCenter, gap))
			return false;

		// Not a town a running mission or story is using (world facts' claims), when it moves
		if (m_GM.GetMissions() && m_GM.GetMissions().IsSettlementBusy(idx))
			return false;

		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			vector qm;
			if (m_GM.GetPerks() && m_GM.GetPerks().GetQuartermasterPos(f, qm) && RZ_Util.DistXZ(qm, s.m_vCenter) < gap)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Settlements within HUB_RADIUS: how many roads meet here, roughly (as territory reads it).
	protected int Neighbours(int idx)
	{
		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (!s)
			return 0;

		int n = 0;
		foreach (int i, RZ_Settlement other : m_GM.GetSettlements())
		{
			if (i != idx && RZ_Util.DistXZ(s.m_vCenter, other.m_vCenter) <= HUB_RADIUS)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! A farm building there (barn, farm, mill, stable, by prefab name), the farthest one out;
	//! else the building farthest from the middle; else a point on the edge of the place.
	protected vector PickHouse(RZ_Settlement s)
	{
		array<IEntity> found = {};
		RZ_BuildingQuery.FindBuildings(s.m_vCenter, s.m_fRadius + 250, found, 160);
		array<string> farmWords = {"barn", "farm", "mill", "stable", "granary", "cowshed", "silo"};
		IEntity bestFarm;
		float farmDist = -1;
		IEntity bestOuter;
		float outerDist = -1;
		foreach (IEntity e : found)
		{
			if (!e)
				continue;

			vector at = e.GetOrigin();
			if (RZ_Util.IsWater(at) || RZ_Perimeter.IsNearLineAt(at))
				continue;

			float d = RZ_Util.DistXZ(at, s.m_vCenter);
			if (RZ_ItemHelper.NameHas(RZ_ItemHelper.LowerName(e), farmWords))
			{
				if (d > farmDist)
				{
					farmDist = d;
					bestFarm = e;
				}

				continue;
			}

			if (d > outerDist)
			{
				outerDist = d;
				bestOuter = e;
			}
		}

		if (bestFarm)
			return RZ_Util.GroundPos(bestFarm.GetOrigin());

		if (bestOuter)
			return RZ_Util.GroundPos(bestOuter.GetOrigin());

		return RZ_Mission.LandPointNear(s.m_vCenter, s.m_fRadius * 0.7, s.m_fRadius + 150);
	}

	// ================================================================ World flags
	//------------------------------------------------------------------------------------------------
	//! The house's state as season-wide world flags, for questlines and other content
	//! (docs/QUESTLINES.md, Crossings): table.exists, table.town, table.exposed, table.exposed_to,
	//! table.broken, table.burnt, table.moved, table.suspicion.
	protected void Publish()
	{
		RZ_Questlines q = m_GM.GetQuestlines();
		if (!q)
			return;

		// The town as its settlement index + 1 (0: no house, or one past no settlement). The facts
		// below are written only once true, since a line may test them with HasWorldFlag
		q.SetWorldFlag("table.exists", BoolInt(Exists()));
		int town = 0;
		if (Exists() && m_iSettlement >= 0)
			town = m_iSettlement + 1;

		q.SetWorldFlag("table.town", town);
		q.SetWorldFlag("table.suspicion", RZ_Util.RoundToInt(m_fSuspicion));
		if (m_bEverExposed)
		{
			q.SetWorldFlag("table.exposed", 1);
			q.SetWorldFlag("table.exposed_to", m_iExposedTo);
		}

		if (m_eState == RZ_ETableState.BROKEN)
			q.SetWorldFlag("table.broken", 1);

		if (m_bBurnt)
			q.SetWorldFlag("table.burnt", 1);

		if (m_bMoved)
			q.SetWorldFlag("table.moved", 1);
		m_bPublished = true;
	}

	//------------------------------------------------------------------------------------------------
	protected static int BoolInt(bool b)
	{
		if (b)
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected int WorldFlag(string key)
	{
		RZ_Questlines q = m_GM.GetQuestlines();
		if (!q)
			return 0;

		return q.WorldFlag(key);
	}

	//------------------------------------------------------------------------------------------------
	//! A crossing flag that is set and hasn't been acted on yet (then it's marked as acted on).
	protected bool TakeCrossing(string key)
	{
		if (WorldFlag(key) <= 0 || m_aConsumed.Contains(key))
			return false;

		m_aConsumed.Insert(key);
		return true;
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_Settings.m_bEnabled || m_eState == RZ_ETableState.NONE)
			return;

		if (!m_bPublished)
			Publish();

		// The house's clocks only run while someone plays, like the outbreak's
		if (!ids.IsEmpty())
		{
			m_fClock += dt;
			m_fDataTimer += dt;
			if (m_eState == RZ_ETableState.ACTIVE)
				m_fTakeTimer += dt * m_GM.GetSeasonSpeed();
		}

		CheckSceneHold();
		UpdateFight(dt, ids, players);
		if (IsOpen())
		{
			UpdateBodies(players);
			CheckKills(players);
			UpdateVisits(dt, ids, players);
		}

		m_fHostTimer += dt;
		if (m_fHostTimer >= HOST_CHECK)
		{
			m_fHostTimer = 0;
			UpdateHostOnRoad(ids, players);
		}

		m_fTellTimer += dt;
		if (m_fTellTimer >= TELL_CHECK)
		{
			m_fTellTimer = 0;
			UpdateTells(players);
		}

		if (m_fDataTimer >= DATA_SECONDS)
		{
			float hours = m_fDataTimer / 3600;
			m_fDataTimer = 0;
			DataTick(hours, players);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every 10 minutes of play: hunger, takings, suspicion's thresholds, the crossings, moving,
	//! and what happens a real day after it's found out.
	protected void DataTick(float hours, array<IEntity> players)
	{
		ReadCrossings();
		TrackC1();
		ForgetOld();
		if (m_bPurgeDue)
			RedHandPurge();

		int now = RZ_Guidance.UnixNow();
		if (m_eState == RZ_ETableState.QUIET)
		{
			if (now <= 0 || now - m_iStateUnix >= DAY)
				Relocate();

			Publish();
			return;
		}

		if (m_eState == RZ_ETableState.EXPOSED)
		{
			// A real day, and nobody ended it: Bastion burns it while Bastion is around, else it moves
			if ((now <= 0 || now - m_iStateUnix >= DAY) && !IsC1Ending())
				Unended(players);

			Publish();
			return;
		}

		if (m_eState != RZ_ETableState.ACTIVE)
			return;

		m_fHunger = Math.Clamp(m_fHunger + m_Settings.m_fHungerPerHour * hours * m_GM.GetSeasonSpeed(), 0, 100);
		float interval = Math.Max(0.25, m_Settings.m_fTakeHours) * 3600;
		if (m_bSlowed)
			interval = interval * 2;

		if (m_fTakeTimer >= interval)
		{
			m_fTakeTimer = 0;
			if (m_fHunger > 50)
				TryTake(players);
		}

		CheckSuspicion();

		// The farm's town fell to the infected: they leave, once a season, while nobody's after them
		RZ_Settlement s = m_GM.GetSettlement(m_iSettlement);
		if (m_eState == RZ_ETableState.ACTIVE && s && s.m_fInfection >= 85 && !m_bMoved && !IsLineRunning())
			StartMove(false, "The farm past " + GetTownName() + " has gone dark. The infected are thick round there now.");

		Publish();
	}

	//------------------------------------------------------------------------------------------------
	//! Suspicion's thresholds: at 30 the rumours start (and with a rumour heard, the questline);
	//! past 80 with nobody after them they move; at 100 everybody knows.
	protected void CheckSuspicion()
	{
		if (m_fSuspicion >= 100)
		{
			Expose(-1, -1);
			return;
		}

		if (m_fSuspicion >= RUMOUR_SUSPICION && !m_bRumours)
		{
			m_bRumours = true;
			RZ_Questline.SpreadRumour(m_GM, string.Format("\"There's smoke past %1 most evenings. Nobody lives out there. Nobody should.\"", GetTownName()), 12);
		}

		if (m_fSuspicion >= RUMOUR_SUSPICION && m_bHeard)
			TryBeginC1();

		if (m_fSuspicion > MOVE_SUSPICION && !m_bMoved && !IsLineRunning())
			StartMove(false, "");
	}

	//------------------------------------------------------------------------------------------------
	//! The house was told where the dead are (A Place at the Table): less hungry for the living.
	void Feed(float amount)
	{
		if (Exists() && amount > 0)
			m_fHunger = Math.Max(0, m_fHunger - amount);
	}

	//------------------------------------------------------------------------------------------------
	//! Suspicion rises (tells found, questlines' flags), capped at 100.
	void AddSuspicion(float amount, string why)
	{
		if (!Exists() || amount == 0)
			return;

		m_fSuspicion = Math.Clamp(m_fSuspicion + amount, 0, 100);
		if (!why.IsEmpty())
			RZ_Log.Info(string.Format("Long Table: suspicion %1 (%2).", RZ_Util.RoundToInt(m_fSuspicion), why));

		Publish();
	}

	//------------------------------------------------------------------------------------------------
	//! Flags other questlines set that the house reads (docs/QUESTLINES.md, Crossings). The names
	//! from the table first; the "<setter>.<flag>" spelling of the same fact as well.
	protected void ReadCrossings()
	{
		RZ_Questlines q = m_GM.GetQuestlines();
		if (!q || !Exists())
			return;

		// Any line can raise suspicion: table.suspicion_add (consumed)
		int add = q.WorldFlag("table.suspicion_add");
		if (add > 0)
		{
			q.SetWorldFlag("table.suspicion_add", 0);
			AddSuspicion(add, "a questline");
		}

		// R2 Cargo: the buyer is gone, so the house takes people half as often
		if (TakeCrossing("table.supplier_cut") || TakeCrossing("cargo.supplier_cut"))
			m_bSlowed = true;

		// H2 The Price of the Door: the exile walked to the crew is one of the people who never
		// arrived. Only the flag crosses, not his name or his house, so he's spoken of as H2 tells
		// him: the man the Haven put out for taking bread. He had nothing to carry (no satchel).
		if (TakeCrossing("door.exile_sold") && m_eState == RZ_ETableState.ACTIVE)
		{
			Taken("the man who took bread", "the Haven", "wherever the Red Hand's van was going", vector.Zero, ROLE_NO_BAG,
				"The man the Haven put out for taking bread was walked to a Red Hand crew, and went off in their van. Nobody has seen him since.");
		}

		// H1 The List, The names read: every house heard the crossed-out names, the farm's too
		if (TakeCrossing("list.read"))
			AddSuspicion(LIST_READ_SUSPICION, "the List was read out");

		// R2's Bastion raid: Bastion knows the farm
		if (TakeCrossing("table.bastion_knows") || TakeCrossing("cargo.bastion_knows"))
			Expose(RZ_EFaction.MILITARY, -1);

		// R2 told the Haven's chair where the strays go: with the farm as the buyer (cargo.buyer 1),
		// the house is found out, to the Haven (C1 from "Who to tell")
		if (TakeCrossing("cargo.told_haven") && q.WorldFlag("cargo.buyer") == 1 && m_eState == RZ_ETableState.ACTIVE)
			Expose(RZ_EFaction.SURVIVORS, -1);

		// Other lines' finds start the questline: H1 at the guest stage, R2 and C2 at "Who to tell"
		bool found = q.WorldFlag("list.found_table") > 0;
		bool proof = q.WorldFlag("cargo.buyer_known") > 0 || q.WorldFlag("place.undercover") > 0 || q.WorldFlag("place.proof") > 0;
		if (found || proof)
			TryBeginC1();
	}

	//------------------------------------------------------------------------------------------------
	//! Deaths older than an hour and leavers older than six are forgotten.
	protected void ForgetOld()
	{
		for (int i = m_aDeaths.Count() - 1; i >= 0; i--)
		{
			if (m_fClock - m_aDeaths[i].m_fAt > DEATH_SECONDS)
				m_aDeaths.Remove(i);
		}

		for (int j = m_aLeavers.Count() - 1; j >= 0; j--)
		{
			if (m_fClock - m_aLeavers[j].m_fAt > LEAVER_SECONDS || m_aLeavers[j].m_iCount <= 0)
				m_aLeavers.Remove(j);
		}

		int now = RZ_Guidance.UnixNow();
		for (int k = m_aTells.Count() - 1; k >= 0; k--)
		{
			RZ_TableTell t = m_aTells[k];
			if (t.m_bFound || (now > 0 && t.m_iUntilUnix > 0 && now >= t.m_iUntilUnix))
			{
				RZ_Util.DeleteUnit(t.m_Prop);
				m_aTells.Remove(k);
			}
		}
	}

	// ================================================================ Takings
	//------------------------------------------------------------------------------------------------
	//! One person, capped: a wanderer whose road passes the farm (one a real day), someone who left
	//! a safehouse alone in the last few hours. Never from a safehouse or a holding, never a player.
	protected void TryTake(array<IEntity> players)
	{
		RZ_Wanderers wanderers = m_GM.GetWanderers();
		int now = RZ_Guidance.UnixNow();
		bool dayOk = m_iWandererUnix <= 0 || now <= 0 || now - m_iWandererUnix >= DAY;
		if (wanderers && dayOk)
		{
			int id = wanderers.FindPassingNear(m_vHouse, ROAD_RADIUS, TAKE_CLEAR, players);
			if (id > 0)
			{
				string title = wanderers.GetTitle(id);
				string dest = wanderers.GetDestName(id);
				int role = wanderers.GetRole(id);
				vector at;
				wanderers.GetPosition(id, at);
				if (wanderers.TakeOffRoad(id))
				{
					m_iWandererUnix = now;
					string from = NearPlace(at);
					if (dest.IsEmpty())
						dest = "the next town";

					Taken(title, from, dest, at, role, "");
					return;
				}
			}
		}

		// Someone who walked out of a hungry safehouse and kept walking
		foreach (RZ_TableLeaver lv : m_aLeavers)
		{
			if (lv.m_iCount <= 0 || RZ_Util.NearestDistance(lv.m_vFrom, players) < TAKE_CLEAR)
				continue;

			lv.m_iCount = lv.m_iCount - 1;
			RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
			string name = RZ_NoteText.FirstName(rng);
			Taken(name, lv.m_sHouse, "the next safehouse", vector.Zero, -1,
				string.Format("%1 walked out of the safehouse in %2 when the food ran out, to find a house that could feed them. They never arrived anywhere.", name, lv.m_sHouse));
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Someone never arrived (a taking, here or from a questline): hunger down, suspicion up, and
	//! the tells: the news, a boot on the road, sometimes a satchel at a Red Hand fence. `who` is a
	//! name or a wanderer's title, `from` a place; `role` a wanderer role, -1 for anyone else.
	//! `news` is the headline ("" = "<who> left <from> for <dest>, and never arrived.").
	void Taken(string who, string from, string dest, vector at, int role, string news)
	{
		if (!Exists())
			return;

		m_iTaken++;
		m_fHunger = Math.Max(0, m_fHunger - TAKE_HUNGER);
		m_fSuspicion = Math.Clamp(m_fSuspicion + TAKE_SUSPICION, 0, 100);
		m_sLostName = who;
		m_sLostFrom = from;
		m_sLostTo = dest;
		m_iLostRole = role;
		m_iLostUnix = RZ_Guidance.UnixNow();

		// Where they were last on the road: given, or toward where they came from
		vector spot = at;
		if (spot == vector.Zero || RZ_Util.DistXZ(spot, m_vHouse) > 3000 || RZ_Util.DistXZ(spot, m_vHouse) < 300)
			spot = RZ_Mission.LandPointNear(m_vHouse, 900, 1400);

		m_vLost = spot;
		RZ_Log.Info(string.Format("Long Table: %1 never arrived (hunger %2, suspicion %3, %4 taken).", who, RZ_Util.RoundToInt(m_fHunger), RZ_Util.RoundToInt(m_fSuspicion), m_iTaken));

		// A boot on the verge where they were last seen (a prop only while someone is near)
		AddTell(RZ_Mission.LandPointNear(spot, 2, 12), string.Format("A walking boot on the verge, good leather, laced tight, the size of a grown man's foot. Nobody walks on with one boot. The road from here goes on toward %1.", GetTownName()));

		string text = news;
		if (text.IsEmpty())
			text = string.Format("%1 left %2 for %3, and never arrived.", RZ_Util.Capitalize(who), from, dest);

		RZ_Questline.Headline(m_GM, text, false);
		NotifyHaven(text + " Somebody has stopped waiting up.");

		// Couriers and traders carry bags: the satchel turns up at a Red Hand fence
		if (role == RZ_EWandererRole.COURIER || role == RZ_EWandererRole.TRADER || (role != ROLE_NO_BAG && Math.RandomFloat01() < 0.3))
			Satchel(who);

		Publish();
	}

	//------------------------------------------------------------------------------------------------
	//! "A Haven courier's satchel turned up at a Red Hand fence": the Red Hand's open channel
	//! laughs about it, and the road repeats it.
	protected void Satchel(string who)
	{
		string channel = "\"Anyone missing a satchel? Letters, a hairbrush, a picture of somebody's mum. Fence has it. Make an offer.\" Laughing, then the channel goes quiet.";
		RZ_Questline.SpreadRumour(m_GM, string.Format("\"%1's satchel turned up at a Red Hand fence. Letters still in it, all addressed. Nobody asked where it came from.\"", RZ_Util.Capitalize(who)), 8);

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (!RZ_Reputation.IsAtLeast(m_GM.GetTier(ids[i], RZ_EFaction.BANDITS), RZ_ETier.KNOWN) && !RZ_Radio.IsRadioOperator(m_GM, ids[i]))
				continue;

			if (RZ_Radio.HasRadio(m_GM, ids[i], p))
				m_GM.NotifyPlayer(ids[i], RZ_FactionVoice.ChannelTitle(RZ_EFaction.BANDITS), channel);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void NotifyHaven(string text)
	{
		array<int> all = {};
		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int playerId : all)
		{
			if (RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, RZ_EFaction.SURVIVORS), RZ_ETier.KNOWN))
				m_GM.NotifyPlayer(playerId, "The Haven", text);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string NearPlace(vector pos)
	{
		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(pos));
		if (s)
			return s.m_sName;

		return "grid " + RZ_SettlementScanner.GridReference(pos);
	}

	// ================================================================ Tells on the road
	//------------------------------------------------------------------------------------------------
	protected void AddTell(vector pos, string text)
	{
		RZ_TableTell t = new RZ_TableTell();
		t.m_vPos = RZ_Util.GroundPos(pos);
		t.m_sText = text;
		int now = RZ_Guidance.UnixNow();
		if (now > 0)
			t.m_iUntilUnix = now + 2 * DAY;

		m_aTells.Insert(t);
		while (m_aTells.Count() > MAX_TELLS)
		{
			RZ_Util.DeleteUnit(m_aTells[0].m_Prop);
			m_aTells.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A tell is a prop (Puzzles > Note) only while a player is within 150 m of it.
	protected void UpdateTells(array<IEntity> players)
	{
		ResourceName prefab = m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CLUE);
		foreach (int i, RZ_TableTell t : m_aTells)
		{
			if (t.m_bFound)
				continue;

			float nearest = RZ_Util.NearestDistance(t.m_vPos, players);
			if (t.m_Prop && nearest > TELL_RADIUS + 70)
			{
				RZ_Util.DeleteUnit(t.m_Prop);
				t.m_Prop = null;
				continue;
			}

			if (!t.m_Prop && nearest <= TELL_RADIUS && !prefab.IsEmpty())
			{
				t.m_Prop = RZ_Util.SpawnPrefab(prefab, t.m_vPos, Math.RandomFloat(0, 360));
				if (t.m_Prop && !RZ_ClueComponent.Tag(t.m_Prop, TAG, CLUE_TELL + i, "Look at the boot on the verge"))
				{
					RZ_Util.DeleteUnit(t.m_Prop);
					t.m_Prop = null;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A tell read: suspicion up, the reader noticed something, the prop goes.
	protected void ReadTell(int playerId, int idx)
	{
		if (idx < 0 || idx >= m_aTells.Count())
			return;

		RZ_TableTell t = m_aTells[idx];
		m_GM.NotifyPlayer(playerId, "On the verge", t.m_sText);
		if (t.m_bFound)
			return;

		t.m_bFound = true;
		RZ_Util.DeleteUnit(t.m_Prop);
		t.m_Prop = null;
		Notice(playerId);
		AddSuspicion(TELL_SUSPICION, "a tell found");
		if (m_fSuspicion >= RUMOUR_SUSPICION)
		{
			m_bHeard = true;
			TryBeginC1();
		}
	}

	// ================================================================ Bodies near players
	//------------------------------------------------------------------------------------------------
	//! A questline stage mission has the farm: the house's own people stand aside while it runs.
	void HoldScene(int missionId)
	{
		m_iSceneHold = missionId;
		DespawnBodies();
	}

	//------------------------------------------------------------------------------------------------
	void ReleaseScene(int missionId)
	{
		if (m_iSceneHold == missionId)
			m_iSceneHold = -1;
	}

	//------------------------------------------------------------------------------------------------
	bool IsSceneHeld()
	{
		return m_iSceneHold >= 0;
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckSceneHold()
	{
		if (m_iSceneHold < 0)
			return;

		RZ_Mission m = m_GM.GetMissions().FindById(m_iSceneHold);
		if (!m || m.m_eState != RZ_EMissionState.ACTIVE)
			m_iSceneHold = -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Within 300 m: the host and one or two others outside, the mother at the door, sometimes the
	//! boy. Not while a questline has the farm, the house is fighting or nobody is at the farm.
	protected void UpdateBodies(array<IEntity> players)
	{
		float nearest = RZ_Util.NearestDistance(m_vHouse, players);
		bool blocked = m_iSceneHold >= 0 || m_FightGroup;
		if (m_bSpawned && (blocked || nearest > m_Settings.m_fBodyRadius + 50))
		{
			DespawnBodies();
			return;
		}

		if (!m_bSpawned && !blocked && nearest <= m_Settings.m_fBodyRadius)
			SpawnBodies();
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnBodies()
	{
		m_bSpawned = true;
		ResourceName prefab = m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);
		if (prefab.IsEmpty())
			return;

		int outside = 0;
		bool boy = Math.RandomFloat01() < 0.5;
		foreach (int i, RZ_TableMember m : m_aMembers)
		{
			if (!m.m_bAlive || m.m_bOnRoad)
				continue;

			vector at;
			if (m.m_eRole == RZ_ETableRole.MOTHER)
			{
				at = RZ_Mission.LandPointNear(m_vHouse, 2, 5);
			}
			else if (m.m_eRole == RZ_ETableRole.BOY)
			{
				if (!boy)
					continue;

				at = RZ_Mission.LandPointNear(m_vHouse, 10, 25);
			}
			else
			{
				// The host and one or two others outside
				int cap = 2;
				if (m.m_eRole == RZ_ETableRole.HOST)
					cap = 3;

				if (outside >= cap)
					continue;

				outside++;
				at = RZ_Mission.LandPointNear(m_vHouse, 6, 18);
			}

			IEntity e = RZ_Util.SpawnPrefab(prefab, at, Math.RandomFloat(0, 360));
			if (!e)
				continue;

			m.m_Body = e;
			RZ_PuzzleObjectComponent.Tag(e, TAG, i, "", ChoiceLabels(m));
			RZ_ClueComponent.Tag(e, TAG, i, "Talk to " + m.m_sName);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The grown-ups: eat, trade, declare a bite. The boy only talks.
	protected string ChoiceLabels(RZ_TableMember m)
	{
		if (m.m_eRole == RZ_ETableRole.BOY)
			return "";

		return "Sit down and eat|Trade for a jar (ammunition or medicine)|Tell them about your bite";
	}

	//------------------------------------------------------------------------------------------------
	protected void DespawnBodies()
	{
		foreach (RZ_TableMember m : m_aMembers)
		{
			if (m.m_bOnRoad)
				continue;

			if (m.m_Body)
				RZ_Util.DeleteUnit(m.m_Body);

			m.m_Body = null;
		}

		m_bSpawned = false;
	}

	//------------------------------------------------------------------------------------------------
	//! A body that was a character and died: a member is gone. Before they're found out, a quiet
	//! farm family: +6 Haven Infamy to whoever shot them (not once they set on you, nor for a
	//! questline's player who knows, nor with the infected on them).
	protected void CheckKills(array<IEntity> players)
	{
		foreach (RZ_TableMember m : m_aMembers)
		{
			if (!m.m_bAlive || !m.m_Body || RZ_Util.IsCharacterAlive(m.m_Body))
				continue;

			m.m_bAlive = false;
			int killer = RZ_Util.GetKillerPlayerId(m.m_Body);
			vector at = m.m_Body.GetOrigin();
			RZ_Log.Info(string.Format("Long Table: %1 (%2) was killed at the farm.", m.m_sName, m.m_eRole));
			if (m.m_bOnRoad)
			{
				m.m_bOnRoad = false;
				m_iRoadPlayer = -1;
			}

			if (killer > 0 && !m_GM.GetSpawner().IsZombieNear(at, 30))
				Blame(killer, m.m_sName);
		}

		BreakIfFinished();
	}

	//------------------------------------------------------------------------------------------------
	//! One of the family was killed in a questline's scene (its own body at the farm): as here.
	void MemberKilled(int role, int killer, vector at)
	{
		RZ_TableMember m = Find(role);
		if (!m)
			return;

		m.m_bAlive = false;
		RZ_Log.Info(string.Format("Long Table: %1 (%2) was killed.", m.m_sName, role));
		if (killer > 0 && !m_GM.GetSpawner().IsZombieNear(at, 30))
			Blame(killer, m.m_sName);

		BreakIfFinished();
	}

	//------------------------------------------------------------------------------------------------
	//! One of the family left the farm for good without a death (ran off, walked out).
	void MemberGone(int role)
	{
		RZ_TableMember m = Find(role);
		if (m)
			m.m_bAlive = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void Blame(int killer, string name)
	{
		Remember(killer, MEM_CROSSED);
		if (m_eState == RZ_ETableState.EXPOSED || Remembers(killer, MEM_SET_ON) || Remembers(killer, MEM_KNOWS))
			return;

		m_GM.AddInfamy(killer, RZ_EFaction.SURVIVORS, SHOT_INFAMY);
		m_GM.NotifyPlayer(killer, "The Haven", string.Format("Word gets round that someone shot %1, of the farm past %2, who'd done nobody any harm that anyone knew of. (+%3 Haven Infamy)",
			name, GetTownName(), SHOT_INFAMY));
	}

	//------------------------------------------------------------------------------------------------
	//! The kitchen is the house: without the mother, or with fewer than two grown-ups, it's over.
	protected void BreakIfFinished()
	{
		if (!Exists() || m_eState == RZ_ETableState.QUIET)
			return;

		if (IsAlive(RZ_ETableRole.MOTHER) && CountAdults() >= 2)
			return;

		// A questline's own ending decides how it's told
		if (IsC1Ending())
			return;

		BreakUp(RZ_ETableEnd.BY_HAND, IsAlive(RZ_ETableRole.BOY), false, string.Format("The lamp at the farm past %1 didn't come on last night. Nobody has been up to see why.", GetTownName()));
	}

	// ================================================================ Visits and the guest rule
	//------------------------------------------------------------------------------------------------
	//! Who is at the farm now (a visit lasts until they're 50 m past the body radius), and the
	//! guest rule: ate here, still within 30 m after 20:30, nobody else within 100 m, not infected.
	protected void UpdateVisits(float dt, array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];
			float d = RZ_Util.DistXZ(p.GetOrigin(), m_vHouse);
			RZ_TableVisit v;
			m_mVisits.Find(playerId, v);
			if (d <= m_Settings.m_fBodyRadius)
			{
				if (!v)
				{
					v = new RZ_TableVisit();
					m_mVisits.Set(playerId, v);
				}

				v.m_bNear = true;
				CheckGuestRule(playerId, p, v, ids, players);
				continue;
			}

			// Gone: the visit is over
			if (v && d > m_Settings.m_fBodyRadius + 50)
				m_mVisits.Remove(playerId);
		}

		// Players who left the server or died
		array<int> stale = {};
		foreach (int vid, RZ_TableVisit vv : m_mVisits)
		{
			if (!ids.Contains(vid))
				stale.Insert(vid);
		}

		foreach (int gone : stale)
		{
			m_mVisits.Remove(gone);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckGuestRule(int playerId, IEntity p, RZ_TableVisit v, array<int> ids, array<IEntity> players)
	{
		if (!m_Settings.m_bGuestRule || m_iSceneHold >= 0 || m_FightGroup || m_eState != RZ_ETableState.ACTIVE)
			return;

		if (RZ_Util.DistXZ(p.GetOrigin(), m_vHouse) > GUEST_RADIUS)
			return;

		RZ_TableMemory mem = Memory(playerId);
		if (!mem || (mem.m_iFlags & MEM_RULE_USED) != 0 || (mem.m_iFlags & MEM_FAMILY) != 0)
			return;

		bool daylight = (mem.m_iFlags & MEM_DAYLIGHT) != 0;
		if (!daylight)
		{
			if (!v.m_bAte || m_fClock - v.m_fAteAt < GUEST_GRACE)
				return;

			float hour = RZ_GameTime.GetHour();
			if (hour < 0 || !RZ_GameTime.IsWithin(hour, GUEST_HOUR, 24 - GUEST_HOUR + RZ_GameTime.NIGHT_END))
				return;
		}

		// Not while they're bitten: the family learned on Day 10 what the bitten do to a family
		if (m_GM.GetProfile(playerId).IsInfected())
			return;

		foreach (int j, IEntity other : players)
		{
			if (ids[j] != playerId && RZ_Util.DistXZ(other.GetOrigin(), p.GetOrigin()) <= GUEST_ALONE)
				return;
		}

		mem.m_iFlags = mem.m_iFlags | MEM_RULE_USED | MEM_CROSSED | MEM_SET_ON;
		Notice(playerId);
		AddSuspicion(15, "the guest rule");
		array<int> targets = {playerId};
		if (StartFight(targets, p.GetOrigin(), true))
		{
			m_GM.NotifyPlayer(playerId, FarmTitle(), string.Format("It's late. %1 has cleared the table, and nobody has told you to go. The lamp by the door goes out. When you look round, the door is shut, and %2 is standing in front of it.",
				NameOf(RZ_ETableRole.MOTHER), GreeterName()));
			return;
		}

		m_GM.NotifyPlayer(playerId, FarmTitle(), "It's late, and nobody has told you to go. The lamp by the door goes out. You don't wait to see why: you're through the back window and running before anyone says your name.");
	}

	//------------------------------------------------------------------------------------------------
	//! "The Moravec farm"
	string FarmTitle()
	{
		return string.Format("The %1 farm", m_sSurname);
	}

	// ================================================================ Talking and dealings
	//------------------------------------------------------------------------------------------------
	//! "Talk to ..." on one of the family, or a tell on the road (RZ_PuzzleActionHelper.RouteRead).
	void HandleRead(int playerId, int idx, IEntity owner)
	{
		if (idx >= CLUE_TELL)
		{
			ReadTell(playerId, idx - CLUE_TELL);
			return;
		}

		RZ_TableMember m = MemberAt(idx);
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!m || !player)
			return;

		Notice(playerId);
		if (m.m_bOnRoad)
		{
			m_GM.NotifyPlayer(playerId, m.m_sName, RoadTalk(playerId, player.GetOrigin()));
			return;
		}

		m_GM.NotifyPlayer(playerId, m.m_sName, TalkText(m, playerId, player.GetOrigin()));
	}

	//------------------------------------------------------------------------------------------------
	//! A living member's role, whether they're out on the road, and their name (the talk screen's
	//! trees). False if nobody is at that index.
	bool MemberInfo(int idx, out int role, out bool onRoad, out string name)
	{
		RZ_TableMember m = MemberAt(idx);
		if (!m)
			return false;

		role = m.m_eRole;
		onRoad = m.m_bOnRoad;
		name = m.m_sName;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! host, mother, son, boy, other: the word a conversation's speaker selector uses.
	static string RoleWord(int role)
	{
		switch (role)
		{
			case RZ_ETableRole.MOTHER: return "mother";
			case RZ_ETableRole.HOST: return "host";
			case RZ_ETableRole.SON: return "son";
			case RZ_ETableRole.BOY: return "boy";
		}

		return "other";
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_TableMember MemberAt(int idx)
	{
		if (idx < 0 || idx >= m_aMembers.Count())
			return null;

		RZ_TableMember m = m_aMembers[idx];
		if (!m.m_bAlive)
			return null;

		return m;
	}

	//------------------------------------------------------------------------------------------------
	protected string TalkText(RZ_TableMember m, int playerId, vector from)
	{
		if (Remembers(playerId, MEM_CROSSED) && !Remembers(playerId, MEM_FAMILY))
			return string.Format("%1 looks at you the way you'd look at weather coming. \"We've nothing for you.\"", m.m_sName);

		switch (m.m_eRole)
		{
			case RZ_ETableRole.MOTHER:
			{
				if (Remembers(playerId, MEM_FAMILY))
					return string.Format("%1 touches your arm on the way past. \"You're thin again. There's a bowl kept back for you.\"", m.m_sName);

				return string.Format("%1 wipes her hands on her apron. \"Sit, sit. You're thin. Where've you come from, and does anyone know you're out this way?\"", m.m_sName);
			}
			case RZ_ETableRole.HOST:
				return string.Format("%1 smiles like you're an old friend. %2 \"There's always a place set. Supper's at half six.\"", m.m_sName, RoadNews(from));
			case RZ_ETableRole.SON:
				return string.Format("%1 doesn't say much. He nods at the gate, and at you, and goes back to the woodpile.", m.m_sName);
			case RZ_ETableRole.BOY:
				return string.Format("The boy is splitting kindling, badly. He doesn't look up. \"I'm not supposed to talk to guests.\" His name is %1. He says it like he's been told to forget it.", m.m_sName);
		}

		return string.Format("%1 is mending a fence that doesn't need it. \"Staying for supper? You should. Everyone does.\"", m.m_sName);
	}

	//------------------------------------------------------------------------------------------------
	//! A choice on one of the family (RZ_PuzzleActionHelper.RouteChoice): 0 eat, 1 trade a jar,
	//! 2 declare a bite; on the road: 0 accept the invitation, 1 decline.
	void HandleChoice(int playerId, int idx, int option, IEntity owner, IEntity user)
	{
		RZ_TableMember m = MemberAt(idx);
		if (!m || !user)
			return;

		Notice(playerId);
		if (m.m_bOnRoad)
		{
			RoadChoice(playerId, m, option, user);
			return;
		}

		if (m.m_eRole == RZ_ETableRole.BOY || !IsOpen())
			return;

		if (Remembers(playerId, MEM_CROSSED) && !Remembers(playerId, MEM_FAMILY))
		{
			m_GM.NotifyPlayer(playerId, FarmTitle(), "The door stays shut. Somebody inside says your name, and then nothing else.");
			return;
		}

		RZ_TableVisit v = GetVisit(playerId);
		if (option == 0)
		{
			Eat(playerId, v, user);
			return;
		}

		if (option == 1)
		{
			TradeJar(playerId, v, owner, user);
			return;
		}

		if (option == 2)
			DeclareBite(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_TableVisit GetVisit(int playerId)
	{
		RZ_TableVisit v;
		if (!m_mVisits.Find(playerId, v))
		{
			v = new RZ_TableVisit();
			v.m_bNear = true;
			m_mVisits.Set(playerId, v);
		}

		return v;
	}

	//------------------------------------------------------------------------------------------------
	//! Eat with them: a bowl at the long table, grace, true road news. Family get a bowl to take
	//! away (one food item a real day). The third meal of a life: the host's offer.
	protected void Eat(int playerId, RZ_TableVisit v, IEntity user)
	{
		if (v.m_bAte)
		{
			m_GM.NotifyPlayer(playerId, FarmTitle(), string.Format("\"Another bowl? You'll burst.\" %1 is pleased anyway.", NameOf(RZ_ETableRole.MOTHER)));
			return;
		}

		RZ_TableMemory mem = Memory(playerId);
		int before = 0;
		if (mem)
			before = mem.m_iMeals;

		RecordMeal(playerId);
		string text = string.Format("%1 puts a bowl in front of you before you've sat down. \"Sit, sit. You're thin.\" Stew, thick, hot, more than you've seen in a week. The long table has places for twenty, and most of them are set. %2", NameOf(RZ_ETableRole.MOTHER), Grace());
		if (mem && (mem.m_iFlags & MEM_BOOTS) == 0)
		{
			mem.m_iFlags = mem.m_iFlags | MEM_BOOTS;
			text += " By the door there's a row of boots, in too many sizes.";
			AddSuspicion(2, "the boots by the door");
		}

		text += "\n" + GreeterName() + " tells you the roads. " + RoadNews(user.GetOrigin());

		// Family: a bowl kept back for you, once a real day
		if (mem && (mem.m_iFlags & MEM_FAMILY) != 0)
			text += FamilyBowl(mem, user);

		m_GM.NotifyPlayer(playerId, FarmTitle(), text);

		// To someone who has eaten with him twice and come back, the host makes a small offer
		if (before >= 2 && IsOpen() && m_eState == RZ_ETableState.ACTIVE)
			MakeOffer(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! A meal at the table (here, or in a questline's scene): counts for this life, and the guest
	//! rule watches the rest of the visit.
	void RecordMeal(int playerId)
	{
		RZ_TableMemory mem = Memory(playerId);
		if (mem)
		{
			mem.m_iMeals++;
			mem.m_iFlags = mem.m_iFlags | MEM_GUEST;
		}

		RZ_TableVisit v = GetVisit(playerId);
		v.m_bAte = true;
		v.m_fAteAt = m_fClock;
		Notice(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Grace: the name read before supper is the last one who never arrived.
	string Grace()
	{
		string mother = NameOf(RZ_ETableRole.MOTHER);
		if (m_sLostName.IsEmpty())
			return string.Format("Before anyone eats, %1 says grace. \"For what we are about to receive.\" Everyone says amen. The boy says it last.", mother);

		return string.Format("Before anyone eats, %1 says grace. \"For what we are about to receive. For %2, who came from %3.\" Everyone says amen. The boy says it last.", mother, ShortLost(), m_sLostFrom);
	}

	//------------------------------------------------------------------------------------------------
	//! "Old Marta" out of "Old Marta the trader" (the name as grace reads it).
	string ShortLost()
	{
		string name = m_sLostName;
		int cut = name.IndexOf(" the ");
		if (cut > 0)
			name = name.Substring(0, cut);

		cut = name.IndexOf(",");
		if (cut > 0)
			name = name.Substring(0, cut);

		return name;
	}

	//------------------------------------------------------------------------------------------------
	//! One food item from the loot pool for family, at most once a real day. "" if none.
	protected string FamilyBowl(RZ_TableMemory mem, IEntity user)
	{
		int now = RZ_Guidance.UnixNow();
		if (now > 0 && mem.m_iBowlUnix > 0 && now - mem.m_iBowlUnix < DAY)
			return "";

		ResourceName food = PickFood();
		if (food.IsEmpty() || !RZ_ItemHelper.Give(food, user, user.GetOrigin()))
			return "";

		mem.m_iBowlUnix = now;
		return string.Format("\n%1 sends you off with a jar wrapped in a cloth. \"For later. You're family.\"", NameOf(RZ_ETableRole.MOTHER));
	}

	//------------------------------------------------------------------------------------------------
	//! A food item from the loot pool (by the Haven's food keywords), "" if the pool has none.
	protected ResourceName PickFood()
	{
		RZ_Interactions hub = RZ_Interactions.Get();
		if (!hub || !hub.GetSearch())
			return "";

		array<string> words = {};
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
			words = houses.GetFoodKeywords();
		else
			RZ_FoodHelper.DefaultKeywords(words);

		return hub.GetSearch().PickFromPool(words);
	}

	//------------------------------------------------------------------------------------------------
	//! Trade: a jar (a food item from the loot pool) for one piece of ammunition or medicine, once
	//! a visit. Trading with them before anyone knew costs nothing.
	protected void TradeJar(int playerId, RZ_TableVisit v, IEntity owner, IEntity user)
	{
		string mother = NameOf(RZ_ETableRole.MOTHER);
		if (v.m_bTraded)
		{
			m_GM.NotifyPlayer(playerId, FarmTitle(), string.Format("\"One's plenty,\" says %1. \"Come back. We'll have more put by.\"", mother));
			return;
		}

		ResourceName jar = PickFood();
		if (jar.IsEmpty())
		{
			m_GM.NotifyPlayer(playerId, FarmTitle(), string.Format("%1 shakes her head. \"Nothing put by this week. Next time.\"", mother));
			return;
		}

		if (!RZ_ItemHelper.Has(user, m_Settings.m_sTradeWords))
		{
			m_GM.NotifyPlayer(playerId, FarmTitle(), string.Format("\"Cartridges, or something for a cut,\" says %1. \"Whatever you can spare. We're a long way from anywhere.\"", mother));
			return;
		}

		if (!RZ_QuartermasterHelper.SpawnNear(jar, owner, user))
			return;

		RZ_ItemHelper.Take(user, m_Settings.m_sTradeWords, 1);

		v.m_bTraded = true;
		int day = 12;
		if (m_GM.GetBroadcast())
			day = m_GM.GetBroadcast().GetDay();

		m_GM.NotifyPlayer(playerId, FarmTitle(), string.Format("%1 takes it without looking at it and comes back from the cellar steps with a jar wrapped in newspaper. On the lid, in pencil: \"%2\". Nothing else. \"It keeps,\" says %3.",
			mother, RZ_NoteText.Day(day), GreeterName()));
	}

	//------------------------------------------------------------------------------------------------
	//! Declare your bite: a blessing, and they'll never set on you while you're infected.
	protected void DeclareBite(int playerId)
	{
		string mother = NameOf(RZ_ETableRole.MOTHER);
		if (!m_GM.GetProfile(playerId).IsInfected())
		{
			m_GM.NotifyPlayer(playerId, FarmTitle(), string.Format("%1 asks it kindly, the way every Haven house does. \"No bite? Good. Then sit.\"", mother));
			return;
		}

		Remember(playerId, MEM_BLESSED);
		m_GM.NotifyPlayer(playerId, FarmTitle(), string.Format("%1 looks at the bite for a long time, and then at you. \"Thank you for telling us. People don't, always.\" She wraps bread in a cloth, says a blessing over you with her hand on your head, and walks you to the gate. \"Go on, now, while it's light.\"", mother));
	}

	//------------------------------------------------------------------------------------------------
	//! The host's offer (A Place at the Table), on the third meal of a life. Not to the Haven's
	//! friends, not twice a life.
	protected void MakeOffer(int playerId)
	{
		RZ_TableMemory mem = Memory(playerId);
		if (!mem || (mem.m_iFlags & (MEM_OFFERED | MEM_FAMILY | MEM_WALKED | MEM_CROSSED)) != 0)
			return;

		if (RZ_Reputation.GetAccessRank(m_GM.GetTier(playerId, RZ_EFaction.SURVIVORS)) >= 2)
			return;

		if (!CanBeginLine(C2_KEY))
			return;

		// The offer is for this character only: it's read once, when the questline begins
		m_sOfferIdent = mem.m_sIdent;
		bool begun = m_GM.GetQuestlines().Begin(C2_KEY);
		m_sOfferIdent = "";
		if (!begun)
			return;

		mem.m_iFlags = mem.m_iFlags | MEM_OFFERED;
		m_GM.NotifyPlayer(playerId, GreeterName(), string.Format("%1 walks you to the gate, the way he always does. At the gate he stops. \"You've a good eye for the roads. The dead don't need what they have, do they? If you see where someone's fallen, before anyone else finds them, come and tell me. We'll feed you, and I'll tell you what the roads are doing.\"",
			GreeterName()));
	}

	// ================================================================ The host on the road
	//------------------------------------------------------------------------------------------------
	//! A lone player within 1.5 km of the farm (not at it) may meet the host, once a session: road
	//! news and an invitation. He goes home after a few minutes, or when they've walked on.
	protected void UpdateHostOnRoad(array<int> ids, array<IEntity> players)
	{
		RZ_TableMember host = Find(RZ_ETableRole.HOST);
		if (m_iRoadPlayer > 0)
		{
			m_fRoadLeft -= HOST_CHECK;
			IEntity met = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iRoadPlayer);
			bool gone = !met || !host || !host.m_Body || RZ_Util.DistXZ(met.GetOrigin(), host.m_Body.GetOrigin()) > 350;
			if (gone || m_fRoadLeft <= 0 || m_eState != RZ_ETableState.ACTIVE)
				HostHome();

			return;
		}

		if (!m_Settings.m_bHostOnRoad || m_eState != RZ_ETableState.ACTIVE || !host || m_iSceneHold >= 0 || m_FightGroup)
			return;

		ResourceName prefab = m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);
		if (prefab.IsEmpty())
			return;

		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];
			float d = RZ_Util.DistXZ(p.GetOrigin(), m_vHouse);
			if (d > ROAD_RADIUS || d <= m_Settings.m_fBodyRadius + 100 || RZ_Util.IsInVehicle(p))
				continue;

			string ident = m_GM.GetProfile(playerId).m_sIdentity;
			if (ident.IsEmpty() || m_aMetHost.Contains(ident) || Remembers(playerId, MEM_WALKED) || Remembers(playerId, MEM_CROSSED))
				continue;

			bool alone = true;
			foreach (int j, IEntity other : players)
			{
				if (j != i && RZ_Util.DistXZ(other.GetOrigin(), p.GetOrigin()) <= 150)
					alone = false;
			}

			if (!alone || Math.RandomFloat01() > 0.2)
				continue;

			// An unasked contact: the interruption budget may hold it for now (UI/RZ_NoticeLanes.c)
			RZ_NoticeLanes lanes = RZ_NoticeLanes.Get();
			if (lanes && !lanes.TryContact(playerId, RZ_EContact.HOST))
				continue;

			// He steps out ahead of them, toward the farm
			vector toward = m_vHouse - p.GetOrigin();
			toward[1] = 0;
			float len = toward.Length();
			if (len < 1)
				continue;

			vector at = RZ_Mission.LandPointNear(p.GetOrigin() + toward * (35 / len), 0, 8);
			IEntity e = RZ_Util.SpawnPrefab(prefab, at, Math.RandomFloat(0, 360));
			if (!e)
				return;

			int idx = m_aMembers.Find(host);
			if (host.m_Body)
				RZ_Util.DeleteUnit(host.m_Body);

			host.m_Body = e;
			host.m_bOnRoad = true;
			RZ_PuzzleObjectComponent.Tag(e, TAG, idx, "", "Accept the invitation|Thank him and walk on");
			RZ_ClueComponent.Tag(e, TAG, idx, "Talk to " + host.m_sName);
			m_iRoadPlayer = playerId;
			m_fRoadLeft = 300;
			m_aMetHost.Insert(ident);
			Notice(playerId);
			m_GM.Say(playerId, host.m_sName, -1, string.Format("A man steps out from the hedge %1 with a flask, smiling like he's been waiting for you.", RZ_Util.DirectionText(p.GetOrigin(), at)));
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void HostHome()
	{
		m_iRoadPlayer = -1;
		foreach (RZ_TableMember m : m_aMembers)
		{
			if (!m.m_bOnRoad)
				continue;

			m.m_bOnRoad = false;
			if (m.m_Body)
				RZ_Util.DeleteUnit(m.m_Body);

			m.m_Body = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string RoadTalk(int playerId, vector from)
	{
		return string.Format("\"Cold out. Here.\" It's tea, sweet, still hot. %1 \"There's always a place set at ours: the farm past %2, the one with the smoke. Supper's at half six.\" He looks at the road behind you. \"Does anyone know you're out this way?\"",
			RoadNews(from), GetTownName());
	}

	//------------------------------------------------------------------------------------------------
	protected void RoadChoice(int playerId, RZ_TableMember m, int option, IEntity user)
	{
		if (option == 0)
		{
			m_GM.NotifyPlayer(playerId, m.m_sName, string.Format("\"Good. I'll tell Mother to set another place.\" He points the way: the farm past %1 (%2). \"Don't be late. She worries.\"",
				GetTownName(), RZ_Util.DirectionText(user.GetOrigin(), m_vHouse)));
		}
		else
		{
			m_GM.NotifyPlayer(playerId, m.m_sName, "He doesn't push. \"Another time. The door's always open.\" He's still smiling when you look back.");
		}

		HostHome();
	}

	//------------------------------------------------------------------------------------------------
	//! Something true about the roads: a truck on the road or about to leave, a hungry safehouse,
	//! somewhere the infected are stirred up. Everything the host says is true.
	string RoadNews(vector from)
	{
		array<string> lines = {};
		string truck = TruckNews();
		if (!truck.IsEmpty())
			lines.Insert(truck);

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
		{
			for (int i = 0; i < houses.Count(); i++)
			{
				RZ_Safehouse h = houses.Get(i);
				if (h && h.m_bShort)
				{
					lines.Insert(string.Format("\"The safehouse in %1 is out of food. They'll take anything you carry in.\"", h.m_sName));
					break;
				}
			}
		}

		vector hot;
		float heat;
		if (m_GM.GetHeat().GetHottest(hot, heat) && heat >= 40 && RZ_Util.DistXZ(from, hot) <= 2500)
			lines.Insert(string.Format("\"Something's stirred them up %1 from here. We heard it at the farm.\"", RZ_Util.DirectionText(from, hot)));

		if (lines.IsEmpty())
			return "\"The roads are quiet. Mother says that's when you watch them.\"";

		return lines.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! Where the next supply truck is going, or when one leaves. "" if none is about.
	string TruckNews()
	{
		RZ_SupplyLines supply = m_GM.GetSupply();
		if (!supply || !supply.IsActive())
			return "";

		array<int> order = {RZ_EFaction.MILITARY, RZ_EFaction.BANDITS, RZ_EFaction.SURVIVORS, RZ_EFaction.INSTITUTE};
		foreach (int f : order)
		{
			string dest = supply.TruckDestination(f);
			if (!dest.IsEmpty())
				return string.Format("\"%1's truck is on the road to %2. Keep off that road if you've no business with them.\"", RZ_Factions.GetName(f), dest);
		}

		foreach (int g : order)
		{
			float secs = supply.SecondsToDeparture(g);
			if (secs >= 0 && secs <= 1800)
				return string.Format("\"%1 sends a truck out in about %2 minutes.\"", RZ_Factions.GetName(g), Math.Max(1, RZ_Util.RoundToInt(secs / 60)));
		}

		return "";
	}

	// ================================================================ Hooks from other systems
	//------------------------------------------------------------------------------------------------
	//! A wanderer is talked to (RZ_Wanderers.HandleTalk): what the road says about the farm, "" if
	//! nothing. Someone who hears of the missing while suspicion is 30 or more may start the
	//! questline (The House That Went Quiet).
	string OnWandererTalk(int playerId, int wandererId, vector from)
	{
		if (!Exists())
			return "";

		int now = RZ_Guidance.UnixNow();
		bool recent = !m_sLostName.IsEmpty() && (now <= 0 || m_iLostUnix <= 0 || now - m_iLostUnix <= 3 * DAY);
		string line = "";
		if (m_eState == RZ_ETableState.EXPOSED && Math.RandomFloat01() < 0.5)
		{
			line = string.Format("\"Everyone knows about the farm past %1 now. I keep thinking about the bowls. They set one for everybody.\"", GetTownName());
		}
		else if (recent && m_iLostRole == ROLE_NO_BAG && Math.RandomFloat01() < 0.5)
		{
			line = string.Format("\"The Haven put a man out for taking bread, and a Red Hand crew drove him off in a van. Nobody's seen him since. That van goes out past %1 at night, toward the farm with the smoke.\"", GetTownName());
		}
		else if (recent && Math.RandomFloat01() < 0.5)
		{
			line = string.Format("\"%1 left %2 for %3 a few days back. Never got there. That road goes past the farm with the smoke, past %4.\"", RZ_Util.Capitalize(m_sLostName), m_sLostFrom, m_sLostTo, GetTownName());
		}
		else if (m_fSuspicion >= RUMOUR_SUSPICION && Math.RandomFloat01() < 0.35)
		{
			line = string.Format("\"There's smoke past %1 most evenings. Nobody lives out there. Nobody should.\"", GetTownName());
		}

		if (line.IsEmpty())
			return "";

		Notice(playerId);
		if (m_fSuspicion >= RUMOUR_SUSPICION && !m_bHeard)
		{
			m_bHeard = true;
			RZ_Log.Info("Long Table: a player heard the rumour.");
		}

		if (m_bHeard && m_fSuspicion >= RUMOUR_SUSPICION)
			TryBeginC1();

		return line;
	}

	//------------------------------------------------------------------------------------------------
	//! A wanderer died (RZ_Wanderers.Die). The infected's dead are "the dead" for an hour.
	void OnWandererDied(vector pos, string who, bool byInfected)
	{
		if (byInfected)
			NoteDeath(pos, who);
	}

	//------------------------------------------------------------------------------------------------
	//! Someone died to the infected at pos (a wanderer, a truck crew): A Place at the Table's "dead".
	void NoteDeath(vector pos, string who)
	{
		RZ_TableDeath d = new RZ_TableDeath();
		d.m_vPos = pos;
		d.m_sWho = who;
		d.m_fAt = m_fClock;
		m_aDeaths.Insert(d);
		while (m_aDeaths.Count() > 6)
		{
			m_aDeaths.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The latest death to the infected within the last `seconds` of play, false if none.
	bool FindRecentDeath(float seconds, out vector pos, out string who)
	{
		for (int i = m_aDeaths.Count() - 1; i >= 0; i--)
		{
			RZ_TableDeath d = m_aDeaths[i];
			if (m_fClock - d.m_fAt > seconds)
				continue;

			pos = d.m_vPos;
			who = d.m_sWho;
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! People who left a hungry safehouse and weren't taken in anywhere (RZ_Safehouses.Upkeep).
	void OnLeftOntoRoads(vector from, string house, int count)
	{
		if (count <= 0 || m_eState != RZ_ETableState.ACTIVE)
			return;

		RZ_TableLeaver lv = new RZ_TableLeaver();
		lv.m_vFrom = from;
		lv.m_sHouse = house;
		lv.m_iCount = count;
		lv.m_fAt = m_fClock;
		m_aLeavers.Insert(lv);
	}

	// ================================================================ The questlines
	//------------------------------------------------------------------------------------------------
	//! A Long Table questline may begin now: questlines on, the house open, and nothing else in
	//! the slot (one at a time).
	bool CanBeginLine(string key)
	{
		RZ_Questlines q = m_GM.GetQuestlines();
		if (!q || !q.IsEnabled() || !IsOpen() || q.FindRunByKey(key))
			return false;

		foreach (RZ_QuestRun run : q.GetRuns())
		{
			if (!run.m_bDone && run.m_Line && run.m_Line.GetSlot() == SLOT)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The House That Went Quiet begins (a rumour heard at suspicion 30, a crossing, the other
	//! line's hand-off). Not within 6 hours of play of it fizzling; once it reached an ending, only
	//! again after the house was paid to move, and then once.
	bool TryBeginC1()
	{
		if (!CanBeginLine(C1_KEY))
			return false;

		if (m_fC1EndedAt >= 0 && m_fClock - m_fC1EndedAt < 6 * 3600)
			return false;

		RZ_Questlines q = m_GM.GetQuestlines();
		bool ended = q.HasWorldFlag(C1_KEY + ".ending");
		if (ended && (q.WorldFlag(C1_KEY + ".ending") != C1_ENDING_PAID || !m_bMoved || m_iC1Begun > 0))
			return false;

		if (!q.Begin(C1_KEY))
			return false;

		if (ended)
			m_iC1Begun++;

		m_bC1Was = true;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsC1Running()
	{
		RZ_Questlines q = m_GM.GetQuestlines();
		return q && q.FindRunByKey(C1_KEY) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Someone is after them, or dealing with them: either Long Table questline is running.
	bool IsLineRunning()
	{
		RZ_Questlines q = m_GM.GetQuestlines();
		return q && (q.FindRunByKey(C1_KEY) != null || q.FindRunByKey(C2_KEY) != null);
	}

	//------------------------------------------------------------------------------------------------
	//! The House That Went Quiet is at its last stage: its ending decides what happens to the house.
	protected bool IsC1Ending()
	{
		RZ_Questlines q = m_GM.GetQuestlines();
		if (!q)
			return false;

		RZ_QuestRun run = q.FindRunByKey(C1_KEY);
		return run && run.m_iStage >= 5;
	}

	//------------------------------------------------------------------------------------------------
	protected void TrackC1()
	{
		bool running = IsC1Running();
		if (m_bC1Was && !running)
			m_fC1EndedAt = m_fClock;

		m_bC1Was = running;
	}

	// ================================================================ Found out
	//------------------------------------------------------------------------------------------------
	//! A questline's proof reached `faction` (-1: suspicion reached 100, everybody knows). A
	//! headline; the Haven's shame, Bastion's containment, the Red Hand's purge of the crew that
	//! bought from them, Halcyon's interest. Family of the house are named and held to account.
	void Expose(int faction, int playerId)
	{
		if (!IsOpen())
			return;

		if (m_eState == RZ_ETableState.EXPOSED)
		{
			if (m_iExposedTo < 0 && faction >= 0)
				m_iExposedTo = faction;

			Publish();
			return;
		}

		m_eState = RZ_ETableState.EXPOSED;
		m_bEverExposed = true;
		m_iExposedTo = faction;
		m_iStateUnix = RZ_Guidance.UnixNow();
		m_fSuspicion = 100;
		HostHome();
		RZ_Log.Info(string.Format("Long Table: found out (to %1).", faction));

		string town = GetTownName();
		string names = HoldFamilyToAccount();
		string headline = string.Format("Everyone knows about the %1 farm past %2 now. They still set a place for whoever comes.", m_sSurname, town);
		if (!names.IsEmpty())
			headline += " Among the names in the guestbook without a cross: " + names + ".";

		RZ_Questline.Headline(m_GM, headline, true);

		// The Haven: it was one of their houses
		RZ_Questline.NudgeMorale(m_GM, RZ_EFaction.SURVIVORS, -8);
		RZ_Questline.SpreadRumour(m_GM, "\"It was one of ours. We sent it our mouths, and then we sent it nothing. Don't ask me any more.\"", 12);

		// Bastion: a line in a report
		RZ_Questline.SpreadRumour(m_GM, string.Format("\"Bastion has it down as an infected cluster past %1. Containment, they call it. They'll burn it if nobody else ends it.\"", town), 12);

		// Halcyon: interested, and ashamed of being interested
		RZ_Questline.SpreadRumour(m_GM, "\"A Halcyon researcher asked whether they ever ate the bitten. Then she wouldn't look at anyone.\"", 8);

		// The Red Hand: the crew that bought the jars is purged (on the next round, not in the middle
		// of someone's talk with a crew boss), and the open channel laughs
		RZ_Questline.SpreadRumour(m_GM, "\"Heard about the farm? Pork, they said. Pork!\" The Red Hand's open channel laughs for a long time. Then nobody says anything at all.", 8);
		m_bPurgeDue = true;
		Publish();
	}

	//------------------------------------------------------------------------------------------------
	//! Family of the house when it's found out: Haven Infamy +40, Bastion +20, online or not.
	//! Returns their names for the headline.
	protected string HoldFamilyToAccount()
	{
		string names = "";
		RZ_Questlines q = m_GM.GetQuestlines();
		foreach (RZ_TableMemory mem : m_aMemory)
		{
			if ((mem.m_iFlags & MEM_FAMILY) == 0)
				continue;

			RZ_PlayerProfile profile = m_GM.FindProfileByIdentity(mem.m_sIdent);
			if (!profile || profile.m_iLives != mem.m_iLife)
				continue;

			int playerId = -1;
			if (q)
				playerId = q.OnlineId(mem.m_sIdent);

			string name = "someone who sat at the table";
			if (playerId > 0)
			{
				name = RZ_Util.PlayerName(playerId);
				m_GM.AddInfamy(playerId, RZ_EFaction.SURVIVORS, FAMILY_HAVEN_INFAMY);
				m_GM.AddInfamy(playerId, RZ_EFaction.MILITARY, FAMILY_BASTION_INFAMY);
				m_GM.NotifyPlayer(playerId, "The farm", string.Format("Everyone knows about the farm now, and your name is in its guestbook, without a cross. (+%1 Haven Infamy, +%2 Bastion Infamy)", FAMILY_HAVEN_INFAMY, FAMILY_BASTION_INFAMY));
			}
			else
			{
				profile.AddInfamy(RZ_EFaction.SURVIVORS, FAMILY_HAVEN_INFAMY);
				profile.AddInfamy(RZ_EFaction.MILITARY, FAMILY_BASTION_INFAMY);
			}

			if (playerId > 0)
			{
				if (!names.IsEmpty())
					names += ", ";

				names += name;
			}
		}

		return names;
	}

	//------------------------------------------------------------------------------------------------
	//! The boss purges any crew that bought from them: the nearest Red Hand outpost's crew boss
	//! is replaced (never the stronghold's), once, while the house still had its buyer.
	protected void RedHandPurge()
	{
		m_bPurgeDue = false;
		RZ_Questlines q = m_GM.GetQuestlines();
		if (m_bSlowed || (q && (q.WorldFlag("table.supplier_cut") > 0 || q.WorldFlag("cargo.supplier_cut") > 0)))
			return;

		RZ_Leaders leaders = m_GM.GetLeaders();
		if (!leaders)
			return;

		array<RZ_Leader> crew = {};
		leaders.GetFactionLeaders(RZ_EFaction.BANDITS, crew);
		RZ_Leader best;
		float bestDist = float.MAX;
		foreach (RZ_Leader l : crew)
		{
			if (l.IsStronghold())
				continue;

			float d = RZ_Util.DistXZ(l.m_vPos, m_vHouse);
			if (d < bestDist)
			{
				bestDist = d;
				best = l;
			}
		}

		m_bSlowed = true;
		if (q)
			q.SetWorldFlag("table.supplier_cut", 1);

		if (!best)
			return;

		if (RZ_Questline.ReplaceLeader(m_GM, best.m_iId, string.Format("The Red Hand's boss had %1 of %2 strung up off a lorry. The code has five rules, and none of them is about this. The boss says it doesn't need one.", best.m_sName, best.m_sPlace)))
			RZ_Questline.NudgeMorale(m_GM, RZ_EFaction.BANDITS, -3);
	}

	//------------------------------------------------------------------------------------------------
	//! A real day after it was found out, nobody ended it: Bastion burns it while Bastion is
	//! around (its guns if a player is near, as data if not); otherwise it moves, or scatters if
	//! it already moved this season.
	protected void Unended(array<IEntity> players)
	{
		if (BastionAround())
		{
			bool seen = RZ_Util.NearestDistance(m_vHouse, players) <= 600;
			if (seen)
				Shell();

			RZ_Questline.NudgeMorale(m_GM, RZ_EFaction.MILITARY, 3);
			BreakUp(RZ_ETableEnd.BURNT, Math.RandomFloat01() < 0.5, false, string.Format("Bastion reports an infected cluster cleared past %1.", GetTownName()));
			return;
		}

		if (!m_bMoved)
		{
			StartMove(false, string.Format("The farm past %1 was empty by morning. The table was still laid.", GetTownName()));
			return;
		}

		BreakUp(RZ_ETableEnd.SCATTERED, IsAlive(RZ_ETableRole.BOY), true, string.Format("The farm past %1 was empty by morning. The table was still laid.", GetTownName()));
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion is still in the Zone: its stronghold stands (or territory is off and it has a leader).
	bool BastionAround()
	{
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
			return territory.GetStronghold(RZ_EFaction.MILITARY) != null;

		RZ_Leaders leaders = m_GM.GetLeaders();
		return !leaders || leaders.GetStronghold(RZ_EFaction.MILITARY) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion's mortars on the farm (Missions > Explosion prefab, if set), for anyone watching.
	void Shell()
	{
		ResourceName boom = m_GM.GetExplosionPrefab();
		if (boom.IsEmpty())
			return;

		for (int i = 0; i < 6; i++)
		{
			RZ_Util.SpawnPrefab(boom, RZ_Mission.LandPointNear(m_vHouse, 0, 40));
		}

		m_GM.GetHeat().AddHeat(m_vHouse, 100);
	}

	// ================================================================ Moving
	//------------------------------------------------------------------------------------------------
	//! Quiet for a real day, then the next place that fits, 2-5 km away. Once a season. paid: the
	//! Red Hand paid them to go, and they take people half as often after.
	void StartMove(bool paid, string headline)
	{
		if (!IsOpen() || m_bMoved)
			return;

		m_bMoved = true;
		if (paid)
			m_bSlowed = true;

		m_eState = RZ_ETableState.QUIET;
		m_iStateUnix = RZ_Guidance.UnixNow();
		DespawnBodies();
		HostHome();
		EndFight();
		RZ_Log.Info("Long Table: the farm past " + GetTownName() + " is moving (quiet for a real day).");
		if (!headline.IsEmpty())
			RZ_Questline.Headline(m_GM, headline, false);

		Publish();
	}

	//------------------------------------------------------------------------------------------------
	protected void Relocate()
	{
		int idx = PickTown(m_vHouse, MOVE_MIN, MOVE_MAX);
		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (s)
		{
			m_iSettlement = idx;
			m_vHouse = PickHouse(s);
		}

		m_eState = RZ_ETableState.ACTIVE;
		m_iStateUnix = RZ_Guidance.UnixNow();
		m_fSuspicion = 20;
		m_bRumours = false;
		m_bHeard = false;
		m_mVisits.Clear();
		RZ_Log.Info(string.Format("Long Table: the farm opens again past %1 (grid %2).", GetTownName(), RZ_SettlementScanner.GridReference(m_vHouse)));
		Publish();
	}

	// ================================================================ Broken up
	//------------------------------------------------------------------------------------------------
	//! Gone for the season. The farm is an ordinary house again (Burned, if it was burnt). The boy
	//! lives if `boyOut`: the Kid wanderer, or a safehouse if a Kid is already on the roads. With
	//! hostWanders the host becomes a Scavenger wanderer, who never asks anyone to dinner again.
	void BreakUp(int how, bool boyOut, bool hostWanders, string headline)
	{
		if (m_eState == RZ_ETableState.BROKEN || m_eState == RZ_ETableState.NONE)
			return;

		m_eState = RZ_ETableState.BROKEN;
		m_iStateUnix = RZ_Guidance.UnixNow();
		DespawnBodies();
		HostHome();
		EndFight();
		RZ_Log.Info(string.Format("Long Table: broken up (%1).", how));

		if (how == RZ_ETableEnd.BURNT)
		{
			m_bBurnt = true;
			BurnTown();
			RZ_Questline.NudgeInfection(m_GM, m_iSettlement, -15);
		}

		if (boyOut && IsAlive(RZ_ETableRole.BOY))
			BoyGoes();

		RZ_TableMember host = Find(RZ_ETableRole.HOST);
		if (hostWanders && host && m_GM.GetWanderers())
			m_GM.GetWanderers().AddWithRole(RZ_EWandererRole.SCAVENGER, host.m_sName, m_vHouse);

		if (!headline.IsEmpty())
			RZ_Questline.Headline(m_GM, headline, true);

		Publish();
	}

	//------------------------------------------------------------------------------------------------
	//! The boy is out: the Kid wanderer, or one more mouth at the nearest safehouse.
	protected void BoyGoes()
	{
		RZ_TableMember boy = Find(RZ_ETableRole.BOY);
		if (!boy)
			return;

		boy.m_bAlive = false;
		string kid = "";
		if (m_GM.GetWanderers())
			kid = m_GM.GetWanderers().AddWithRole(RZ_EWandererRole.KID, boy.m_sName, m_vHouse);

		if (!kid.IsEmpty())
			return;

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses && houses.Count() > 0)
			houses.AddPopulation(m_vHouse, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! The farm's town counts as Burned (town memory), as Bastion's guns would leave it.
	protected void BurnTown()
	{
		RZ_TownMemory mem = m_GM.GetTownMemory();
		RZ_Settlement s = m_GM.GetSettlement(m_iSettlement);
		if (!mem || !mem.IsEnabled() || !s)
			return;

		int guard = 0;
		while (!mem.HasFlag(m_iSettlement, RZ_TownMemory.FLAG_BURNED) && guard < 80)
		{
			guard++;
			RZ_TownMemory.ReportBuildingBurned(s.m_vCenter);
		}
	}

	// ================================================================ The fight
	//------------------------------------------------------------------------------------------------
	//! The family's grown men as a fight group (Missions > Events > Rival group, else Missions >
	//! Survivor group) at the farm, hostile only to `targets` (the Haven's bit, as Rival Survivors
	//! use). setOn: they started it, so killing them costs nobody anything. False without the prefab
	//! or with no fighters left.
	bool StartFight(array<int> targets, vector at, bool setOn)
	{
		if (m_FightGroup || targets.IsEmpty())
			return false;

		ResourceName prefab = FightPrefab();
		if (prefab.IsEmpty() || CountFighters() == 0)
			return false;

		HostHome();
		DespawnBodies();
		vector spot = RZ_Mission.LandPointNear(m_vHouse, 8, 20);
		if (RZ_Util.DistXZ(at, m_vHouse) > 60)
			spot = RZ_Mission.LandPointNear(at, 15, 30);

		m_FightGroup = RZ_Util.SpawnPrefab(prefab, spot, Math.RandomFloat(0, 360));
		if (!m_FightGroup)
			return false;

		m_aFightMen.Clear();
		m_aFightDead.Clear();
		m_aFightTargets.Clear();
		m_bFightSetOn = setOn;
		m_fFightAge = 0;
		m_fFightAway = 0;
		m_iFightResult = -1;
		RZ_Hostility hostility = m_GM.GetHostility();
		foreach (int playerId : targets)
		{
			m_aFightTargets.Insert(playerId);
			if (setOn)
				Remember(playerId, MEM_SET_ON);

			if (hostility)
				hostility.MarkHostileAround(RZ_EFaction.SURVIVORS, m_FightGroup, FIGHT_RADIUS, playerId);
		}

		RZ_Log.Info(string.Format("Long Table: a fight at the farm (%1 against them, set on %2).", targets.Count(), setOn));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Add a player to the running fight (they joined it).
	void JoinFight(int playerId)
	{
		if (!m_FightGroup || m_aFightTargets.Contains(playerId))
			return;

		m_aFightTargets.Insert(playerId);
		RZ_Hostility hostility = m_GM.GetHostility();
		if (hostility)
			hostility.MarkHostileAround(RZ_EFaction.SURVIVORS, m_FightGroup, FIGHT_RADIUS, playerId);
	}

	//------------------------------------------------------------------------------------------------
	bool IsFighting()
	{
		return m_FightGroup != null;
	}

	//------------------------------------------------------------------------------------------------
	//! -1 none or running, 0 it ended with the players gone, 1 the family's fighters are all dead.
	int FightResult()
	{
		return m_iFightResult;
	}

	//------------------------------------------------------------------------------------------------
	//! The grown men who'd fight: the host, the quiet son and the others.
	protected int CountFighters()
	{
		int n = 0;
		foreach (RZ_TableMember m : m_aMembers)
		{
			if (m.m_bAlive && m.m_eRole != RZ_ETableRole.MOTHER && m.m_eRole != RZ_ETableRole.BOY)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateFight(float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_FightGroup)
			return;

		m_fFightAge += dt;
		if (m_aFightMen.IsEmpty() && m_fFightAge > 3)
		{
			RZ_Util.GetUnitMembers(m_FightGroup, m_aFightMen);
			for (int k = 0; k < m_aFightMen.Count(); k++)
			{
				m_aFightDead.Insert(false);
			}
		}

		// Each death once: a member of the family, and the blame for whoever shot them
		foreach (int i, IEntity man : m_aFightMen)
		{
			if (m_aFightDead[i] || (man && RZ_Util.IsCharacterAlive(man)))
				continue;

			m_aFightDead[i] = true;
			KillFighter();
			if (man && !m_bFightSetOn)
			{
				int killer = RZ_Util.GetKillerPlayerId(man);
				if (killer > 0)
					Blame(killer, "one of the family");
			}
		}

		// All of them down
		bool allDead = !m_aFightMen.IsEmpty();
		foreach (bool dead : m_aFightDead)
		{
			if (!dead)
				allDead = false;
		}

		if (allDead || (m_fFightAge > 20 && RZ_Util.CountAlive(m_FightGroup) == 0))
		{
			FinishFight(1);
			return;
		}

		// The players they were set on are gone (fled, dead, left): the farm goes quiet again
		bool near = false;
		foreach (int i2, IEntity p : players)
		{
			if (m_aFightTargets.Contains(ids[i2]) && RZ_Util.DistXZ(p.GetOrigin(), m_vHouse) <= 300)
				near = true;
		}

		if (near)
		{
			m_fFightAway = 0;
			return;
		}

		m_fFightAway += dt;
		if (m_fFightAway >= FIGHT_AWAY)
			FinishFight(0);
	}

	//------------------------------------------------------------------------------------------------
	//! One of the fighters fell: the others first, then the quiet son, the host last.
	protected void KillFighter()
	{
		array<int> order = {RZ_ETableRole.OTHER, RZ_ETableRole.SON, RZ_ETableRole.HOST};
		foreach (int role : order)
		{
			RZ_TableMember m = Find(role);
			if (m)
			{
				m.m_bAlive = false;
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void FinishFight(int result)
	{
		m_iFightResult = result;
		RZ_Util.DeleteUnit(m_FightGroup);
		m_FightGroup = null;
		m_aFightMen.Clear();
		m_aFightDead.Clear();
		RZ_Log.Info(string.Format("Long Table: the fight is over (%1, %2 of them left).", result, CountAlive()));

		// Their men are dead: the mother is gone by morning, the boy is out
		if (result == 1 && CountFighters() == 0)
		{
			RZ_TableMember mother = Find(RZ_ETableRole.MOTHER);
			if (mother)
				mother.m_bAlive = false;
		}

		BreakIfFinished();
	}

	//------------------------------------------------------------------------------------------------
	protected void EndFight()
	{
		if (!m_FightGroup)
			return;

		RZ_Util.DeleteUnit(m_FightGroup);
		m_FightGroup = null;
		m_aFightMen.Clear();
		m_aFightDead.Clear();
		m_iFightResult = 0;
	}

	// ================================================================ Geometry
	//------------------------------------------------------------------------------------------------
	//! Distance (XZ) from p to the segment a-b.
	static float DistToSegmentXZ(vector p, vector a, vector b)
	{
		float dx = b[0] - a[0];
		float dz = b[2] - a[2];
		float len2 = dx * dx + dz * dz;
		if (len2 < 0.01)
			return RZ_Util.DistXZ(p, a);

		float t = ((p[0] - a[0]) * dx + (p[2] - a[2]) * dz) / len2;
		t = Math.Clamp(t, 0, 1);
		vector q = a;
		q[0] = a[0] + dx * t;
		q[2] = a[2] + dz * t;
		return RZ_Util.DistXZ(p, q);
	}
}
