//! Named wanderers (docs/DESIGN.md, "Wanderers"): a small roster of survivors who
//! walk between settlements and safehouses. Simulated as data (a position moving in
//! a straight line on the server tick, no AI pathing) and spawned as a body only
//! within 400 m of a player, at most two per player. They remember each player
//! (helped / robbed or shot at / saved my life), their deaths make the news, and a
//! replacement turns up about a real day later.
//!
//! Interactions reuse the generic puzzle civilian: the body is tagged with
//! RZ_PuzzleObjectComponent / RZ_ClueComponent (mission id TAG) and the puzzle
//! actions route here (RZ_PuzzleActionHelper).

enum RZ_EWandererRole
{
	TRADER,
	DOCTOR,
	SCAVENGER,
	DESERTER,
	PREACHER,
	COURIER,
	KID,
	MECHANIC	// cars (world interactions): where a fixable car is, parts for Favor, sometimes its keys
}

enum RZ_EWandererState
{
	TRAVEL,		// walking to m_vDest
	REST,		// resting at a safehouse, or trading and talking in a town
	SHELTER,	// Every Ridge Calls or a storm: waiting it out
	HIDE		// a horde or too much noise nearby: keeping still
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Wanderers.
[BaseContainerProps()]
class RZ_WandererSettings
{
	[Attribute("1", desc: "Named wanderers on")]
	bool m_bEnabled;

	[Attribute("6", desc: "Roster size on a small map")]
	int m_iMinCount;

	[Attribute("10", desc: "Roster size on a big map (one more per 6 settlements above 8)")]
	int m_iMaxCount;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Trader body (falls back to the puzzle civilian). Needs the generic puzzle object components to be talked to. An AI group walks visibly; a character without AI or a prop stands still while seen", "et")]
	ResourceName m_sTraderPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Doctor body (falls back to the puzzle civilian)", "et")]
	ResourceName m_sDoctorPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Scavenger body (falls back to the puzzle civilian)", "et")]
	ResourceName m_sScavengerPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Deserter body (falls back to the puzzle civilian)", "et")]
	ResourceName m_sDeserterPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Preacher body (falls back to the puzzle civilian)", "et")]
	ResourceName m_sPreacherPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Courier body (falls back to the puzzle civilian)", "et")]
	ResourceName m_sCourierPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Kid body (falls back to the puzzle civilian)", "et")]
	ResourceName m_sKidPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Mechanic body (falls back to the puzzle civilian)", "et")]
	ResourceName m_sMechanicPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "What the Trader sells (one at random per purchase). Empty = the Trader only talks", "et")]
	ref array<ResourceName> m_aTraderGoods;

	[Attribute("15", desc: "Haven Favor for one Trader purchase (a quarter off if they remember you helped)")]
	int m_iTradeCost;

	[Attribute("10", desc: "Haven Favor for a Scavenger's rumour (or one food item)")]
	int m_iRumourCost;

	[Attribute("20", desc: "Haven Favor for the Doctor's treatment")]
	int m_iTreatCost;

	[Attribute("400", desc: "Bodies exist only within this distance (m) of a player")]
	float m_fSpawnRadius;

	[Attribute("1.3", desc: "Walking speed while simulated (m/s)")]
	float m_fWalkSpeed;

	[Attribute("1440", desc: "Real minutes before a dead wanderer's replacement appears")]
	float m_fReplaceMinutes;

	[Attribute("0.05", desc: "Chance per check (30 s) that a wanderer with an infected within 12 m is bitten")]
	float m_fBiteChance;

	[Attribute("15", desc: "Haven Infamy for killing a wanderer")]
	int m_iKillInfamy;

	[Attribute("4", desc: "Haven Infamy for shooting at one (once per wanderer)")]
	int m_iShotInfamy;

	[Attribute("8", desc: "Haven Infamy for robbing one")]
	int m_iRobInfamy;
}

//------------------------------------------------------------------------------------------------
class RZ_WandererSave
{
	int id;
	int generation;
	string name;
	string epithet;
	int role;
	bool alive = true;
	bool settled;
	float diedAt;
	float x;
	float z;
	float destX;
	float destZ;
	string destName;
	int destSafehouse = -1;
	int state;
	float stateTimer;
	float bittenLeft = -1;
	ref array<string> memIdent = {};
	ref array<int> memFlags = {};
	ref array<int> memLife = {};		// the life each entry's flags are from
	ref array<int> memOld = {};			// what's remembered of earlier lives
}

//------------------------------------------------------------------------------------------------
class RZ_WandererRosterSave
{
	int seed;
	int nextId = 1;
	float clock;
	ref array<ref RZ_WandererSave> list = {};
}

//------------------------------------------------------------------------------------------------
//! One wanderer. Data first; the body exists only near players.
class RZ_Wanderer
{
	int m_iId;
	int m_iGeneration;
	string m_sName;
	string m_sEpithet;
	RZ_EWandererRole m_eRole;
	bool m_bAlive = true;
	bool m_bSettled;			// left the roads for a safehouse (the Kid)
	float m_fDiedAt;
	vector m_vPos;
	vector m_vDest;
	string m_sDestName;
	int m_iDestSafehouse = -1;
	RZ_EWandererState m_eState = RZ_EWandererState.TRAVEL;
	float m_fStateTimer;
	float m_fBittenLeft = -1;
	ref array<string> m_aMemIdent = {};
	ref array<int> m_aMemFlags = {};
	ref array<int> m_aMemLife = {};
	ref array<int> m_aMemOld = {};

	// Runtime only
	IEntity m_Body;
	bool m_bHasBody;
	float m_fHiddenUntil;		// no body until then (just fled)
	int m_iLastShooter = -1;
	float m_fLastShotAt = -1000;
	int m_iHeldBy = -1;			// a mission is using this person
	int m_iNudgePlayer = -1;	// walking toward this player (new-player guidance)
	float m_fNudgeLeft;
	int m_iEscortPlayer = -1;	// the Kid follows this player to a safehouse
	bool m_bBodyWalks;			// the last body spawned was an AI group (it walks by itself)
	bool m_bTagged;				// the body's talk/choice actions are routed to us
	float m_fBellsOffUntil;		// the Preacher agreed to stop for a while

	//------------------------------------------------------------------------------------------------
	string GetRoleName()
	{
		return RZ_Wanderers.RoleName(m_eRole);
	}

	//------------------------------------------------------------------------------------------------
	//! "Old Marta"
	string GetShortName()
	{
		return m_sEpithet + m_sName;
	}

	//------------------------------------------------------------------------------------------------
	//! "Old Marta the trader"
	string GetTitle()
	{
		return GetShortName() + " the " + GetRoleName();
	}

	//------------------------------------------------------------------------------------------------
	bool IsHaven()
	{
		return m_eRole != RZ_EWandererRole.DESERTER;
	}

	//------------------------------------------------------------------------------------------------
	bool IsBitten()
	{
		return m_fBittenLeft >= 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The life a player identity is on now (-1 unknown), for life-stamped memory.
	protected static int LifeOf(string ident)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return -1;

		RZ_PlayerProfile p = gm.FindProfileByIdentity(ident);
		if (!p)
			return -1;

		return p.m_iLives;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool LifeMemoryOn()
	{
		return RZ_ReputationSettings.Get().m_bLifeMemory;
	}

	//------------------------------------------------------------------------------------------------
	//! What they remember of this player's current life (docs/REPUTATION.md, "Face and name").
	//! With Reputation > Life-stamped memory off: everything, as before.
	int GetMemory(string ident)
	{
		int idx = m_aMemIdent.Find(ident);
		if (idx < 0 || idx >= m_aMemFlags.Count())
			return 0;

		if (!LifeMemoryOn())
			return m_aMemFlags[idx] | m_aMemOld[idx];

		if (m_aMemLife[idx] != LifeOf(ident))
			return 0;

		return m_aMemFlags[idx];
	}

	//------------------------------------------------------------------------------------------------
	//! What they remember of this player's earlier lives (counted at half, voiced once as the name).
	int OldMemory(string ident)
	{
		int idx = m_aMemIdent.Find(ident);
		if (idx < 0 || idx >= m_aMemFlags.Count() || !LifeMemoryOn())
			return 0;

		int old = m_aMemOld[idx];
		if (m_aMemLife[idx] != LifeOf(ident))
			old = old | m_aMemFlags[idx];

		return old & ~0;
	}

	//------------------------------------------------------------------------------------------------
	//! The life the current memory was written in (-1 none, or an old save's).
	int MemoryLife(string ident)
	{
		int idx = m_aMemIdent.Find(ident);
		if (!LifeMemoryOn())
			return LifeOf(ident);

		if (idx < 0 || idx >= m_aMemLife.Count())
			return -1;

		return m_aMemLife[idx];
	}

	//------------------------------------------------------------------------------------------------
	bool Remembers(string ident, int flag)
	{
		return (GetMemory(ident) & flag) != 0;
	}

	//------------------------------------------------------------------------------------------------
	void Remember(string ident, int flag)
	{
		if (ident.IsEmpty())
			return; // a player id that's gone (RZ_GameModeComponent.GetProfile): nobody to remember

		int life = LifeOf(ident);
		int idx = m_aMemIdent.Find(ident);
		if (idx < 0)
		{
			m_aMemIdent.Insert(ident);
			m_aMemFlags.Insert(flag);
			m_aMemLife.Insert(life);
			m_aMemOld.Insert(0);
			return;
		}

		// A new life: what was remembered becomes the earlier life's
		if (m_aMemLife[idx] != life)
		{
			m_aMemOld[idx] = m_aMemOld[idx] | m_aMemFlags[idx];
			m_aMemFlags[idx] = 0;
			m_aMemLife[idx] = life;
		}

		m_aMemFlags[idx] = m_aMemFlags[idx] | flag;
	}

	//------------------------------------------------------------------------------------------------
	RZ_WandererSave ToSave()
	{
		RZ_WandererSave s = new RZ_WandererSave();
		s.id = m_iId;
		s.generation = m_iGeneration;
		s.name = m_sName;
		s.epithet = m_sEpithet;
		s.role = m_eRole;
		s.alive = m_bAlive;
		s.settled = m_bSettled;
		s.diedAt = m_fDiedAt;
		s.x = m_vPos[0];
		s.z = m_vPos[2];
		s.destX = m_vDest[0];
		s.destZ = m_vDest[2];
		s.destName = m_sDestName;
		s.destSafehouse = m_iDestSafehouse;
		s.state = m_eState;
		s.stateTimer = m_fStateTimer;
		s.bittenLeft = m_fBittenLeft;
		foreach (int i, string ident : m_aMemIdent)
		{
			s.memIdent.Insert(ident);
			s.memFlags.Insert(m_aMemFlags[i]);
			s.memLife.Insert(m_aMemLife[i]);
			s.memOld.Insert(m_aMemOld[i]);
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_Wanderer FromSave(RZ_WandererSave s)
	{
		RZ_Wanderer w = new RZ_Wanderer();
		w.m_iId = s.id;
		w.m_iGeneration = s.generation;
		w.m_sName = s.name;
		w.m_sEpithet = s.epithet;
		w.m_eRole = s.role;
		w.m_bAlive = s.alive;
		w.m_bSettled = s.settled;
		w.m_fDiedAt = s.diedAt;
		w.m_vPos = Vector(s.x, 0, s.z);
		w.m_vDest = Vector(s.destX, 0, s.destZ);
		w.m_sDestName = s.destName;
		w.m_iDestSafehouse = s.destSafehouse;
		w.m_eState = s.state;
		w.m_fStateTimer = s.stateTimer;
		w.m_fBittenLeft = s.bittenLeft;
		if (s.memIdent && s.memFlags)
		{
			for (int i = 0; i < s.memIdent.Count() && i < s.memFlags.Count(); i++)
			{
				w.m_aMemIdent.Insert(s.memIdent[i]);
				w.m_aMemFlags.Insert(s.memFlags[i]);
				// An old save's memory loads as an earlier life
				int life = -1;
				int old = 0;
				if (s.memLife && i < s.memLife.Count())
					life = s.memLife[i];
				if (s.memOld && i < s.memOld.Count())
					old = s.memOld[i];

				w.m_aMemLife.Insert(life);
				w.m_aMemOld.Insert(old);
			}
		}

		return w;
	}
}

//------------------------------------------------------------------------------------------------
//! The roster. Owned by the game mode; ticked on the server tick.
class RZ_Wanderers
{
	static const int TAG = -2;				// RZ_PuzzleObjectComponent / RZ_ClueComponent mission id
	static const int ROLE_COUNT = 8;
	static const int MEM_HELPED = 1;
	static const int MEM_WRONGED = 2;		// robbed or shot at
	static const int MEM_SAVED = 4;			// saved my life

	protected static const float ARRIVE_RADIUS = 30;
	protected static const float REACT_INTERVAL = 30;
	protected static const float HIDE_SECONDS = 150;
	protected static const float BITE_SECONDS = 1200;
	protected static const float DESPAWN_MARGIN = 50;
	protected static const float WARN_RADIUS = 150;
	protected static const float WARN_COOLDOWN = 600;
	protected static const float JOB_COOLDOWN = 1800;
	protected static const float BLAME_SECONDS = 30;
	protected static const int SAVED_GIFT_FAVOR = 10;
	protected static const int KID_FAME = 5;
	protected static const int CURED_FAME = 8;

	protected RZ_GameModeComponent m_GM;
	protected RZ_WandererSettings m_Settings;
	protected ref array<ref RZ_Wanderer> m_aList = {};
	protected ref RZ_WandererRosterSave m_Restored;
	protected int m_iSeed;
	protected int m_iNextId = 1;
	protected float m_fClock;
	protected float m_fReactTimer;
	protected ref map<string, float> m_mWarned = new map<string, float>();
	protected ref map<string, float> m_mRumourAt = new map<string, float>();
	protected ref map<string, float> m_mJobAt = new map<string, float>();

	// A faction lost an outpost (OnOutpostLost): the roads there are avoided for a while, the news
	// goes round, and the losers' people are in a mood. Runtime only, it fades within the hour.
	protected ref array<vector> m_aUnrestPos = {};
	protected ref array<float> m_aUnrestUntil = {};
	protected string m_sLossRumour;
	protected float m_fRumourUntil = -1;
	protected int m_iMoodFaction = -1;			// whose people are in a mood
	protected int m_iMoodWinner = -1;
	protected string m_sMoodPlace;
	protected float m_fMoodUntil = -1;

	protected static const float UNREST_RADIUS = 600;
	protected static const float UNREST_SECONDS = 1800;
	protected static const float LOSS_NEWS_SECONDS = 3600;

	//------------------------------------------------------------------------------------------------
	void RZ_Wanderers(RZ_GameModeComponent gm, RZ_WandererSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	static string RoleName(RZ_EWandererRole role)
	{
		switch (role)
		{
			case RZ_EWandererRole.TRADER: return "trader";
			case RZ_EWandererRole.DOCTOR: return "doctor";
			case RZ_EWandererRole.SCAVENGER: return "scavenger";
			case RZ_EWandererRole.DESERTER: return "deserter";
			case RZ_EWandererRole.PREACHER: return "preacher";
			case RZ_EWandererRole.COURIER: return "courier";
			case RZ_EWandererRole.KID: return "kid";
			case RZ_EWandererRole.MECHANIC: return "mechanic";
		}

		return "wanderer";
	}

	//------------------------------------------------------------------------------------------------
	//! Bit for a role, for role masks: RoleBit(COURIER) | RoleBit(SCAVENGER).
	static int RoleBit(int role)
	{
		int bit = 1;
		for (int i = 0; i < role; i++)
		{
			bit = bit * 2;
		}

		return bit;
	}

	//------------------------------------------------------------------------------------------------
	//! Every role.
	static int AllRoles()
	{
		return RoleBit(ROLE_COUNT) - 1;
	}

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	void Restore(RZ_WandererRosterSave saved)
	{
		m_Restored = saved;
	}

	//------------------------------------------------------------------------------------------------
	RZ_WandererRosterSave Save()
	{
		RZ_WandererRosterSave data = new RZ_WandererRosterSave();
		data.seed = m_iSeed;
		data.nextId = m_iNextId;
		data.clock = m_fClock;
		foreach (RZ_Wanderer w : m_aList)
		{
			data.list.Insert(w.ToSave());
		}

		return data;
	}

	// ================================================================ Setup
	//------------------------------------------------------------------------------------------------
	//! After safehouses exist. Restores the saved roster, or generates a new one from a seed.
	void Setup()
	{
		if (!m_Settings.m_bEnabled || m_GM.GetSettlements().IsEmpty())
		{
			RZ_Log.Info("Wanderers are off.");
			return;
		}

		if (m_Restored && m_Restored.list && !m_Restored.list.IsEmpty())
		{
			m_iSeed = m_Restored.seed;
			m_iNextId = Math.Max(1, m_Restored.nextId);
			m_fClock = m_Restored.clock;
			foreach (RZ_WandererSave s : m_Restored.list)
			{
				m_aList.Insert(RZ_Wanderer.FromSave(s));
			}

			m_Restored = null;
			RZ_Log.Info(string.Format("Wanderers: %1 on the roads (saved).", CountAlive()));
			return;
		}

		m_Restored = null;
		m_iSeed = Math.RandomInt(1, 2000000000);
		int target = TargetCount();
		for (int i = 0; i < target; i++)
		{
			RZ_Wanderer w = Generate(-1, StartPoint());
			m_aList.Insert(w);
		}

		RZ_Log.Info(string.Format("Wanderers: %1 on the roads (new roster, seed %2).", CountAlive(), m_iSeed));
	}

	//------------------------------------------------------------------------------------------------
	//! 6 on a small map, one more per 6 settlements above 8, at most 10.
	protected int TargetCount()
	{
		int lo = Math.Max(1, m_Settings.m_iMinCount);
		int hi = Math.Max(lo, m_Settings.m_iMaxCount);
		int n = lo + Math.Max(0, m_GM.GetSettlements().Count() - 8) / 6;
		return Math.ClampInt(n, lo, hi);
	}

	//------------------------------------------------------------------------------------------------
	protected int CountAlive()
	{
		int n = 0;
		foreach (RZ_Wanderer w : m_aList)
		{
			if (w.m_bAlive)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! A safehouse or a quiet settlement to start from.
	protected vector StartPoint()
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses && houses.Count() > 0 && Math.RandomFloat01() < 0.5)
			return RZ_Mission.LandPointNear(houses.Get(houses.PickRandom()).m_vPos, 20, 60);

		int idx = RZ_Mission.PickSettlement(m_GM, 0, 60);
		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (!s)
			s = m_GM.GetSettlement(0);

		return RZ_Mission.LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.6);
	}

	//------------------------------------------------------------------------------------------------
	//! A new person on the roads. role -1 = whatever the roster lacks, then at random.
	protected RZ_Wanderer Generate(int role, vector pos)
	{
		RZ_Wanderer w = new RZ_Wanderer();
		w.m_iId = m_iNextId;
		m_iNextId++;

		RZ_Rng rng = new RZ_Rng(m_iSeed + w.m_iId * 7919);
		if (role < 0)
			role = PickRole(rng);

		w.m_eRole = role;
		w.m_sName = UniqueName(rng);
		w.m_sEpithet = PickEpithet(rng, w.m_eRole);
		w.m_vPos = pos;
		ChooseDestination(w);
		return w;
	}

	//------------------------------------------------------------------------------------------------
	//! A role no living wanderer has yet, else any role but a second Kid.
	protected int PickRole(RZ_Rng rng)
	{
		array<int> missing = {};
		for (int r = 0; r < ROLE_COUNT; r++)
		{
			if (!HasRole(r))
				missing.Insert(r);
		}

		if (!missing.IsEmpty())
		{
			rng.ShuffleInts(missing);
			return missing[0];
		}

		// Any role but a second Kid
		array<int> roles = {RZ_EWandererRole.TRADER, RZ_EWandererRole.DOCTOR, RZ_EWandererRole.SCAVENGER, RZ_EWandererRole.DESERTER,
			RZ_EWandererRole.PREACHER, RZ_EWandererRole.COURIER, RZ_EWandererRole.MECHANIC};
		return roles[rng.Int(0, roles.Count())];
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasRole(int role)
	{
		foreach (RZ_Wanderer w : m_aList)
		{
			if (w.m_bAlive && w.m_eRole == role)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected string UniqueName(RZ_Rng rng)
	{
		string name = RZ_NoteText.FirstName(rng);
		for (int i = 0; i < 20; i++)
		{
			bool taken = false;
			foreach (RZ_Wanderer w : m_aList)
			{
				if (w.m_bAlive && w.m_sName == name)
					taken = true;
			}

			if (!taken)
				return name;

			name = RZ_NoteText.FirstName(rng);
		}

		return name;
	}

	//------------------------------------------------------------------------------------------------
	protected string PickEpithet(RZ_Rng rng, RZ_EWandererRole role)
	{
		switch (role)
		{
			case RZ_EWandererRole.TRADER:
			{
				if (rng.Chance(0.6))
					return "Old ";
				break;
			}
			case RZ_EWandererRole.SCAVENGER:
			{
				if (rng.Chance(0.5))
					return "Quiet ";
				break;
			}
			case RZ_EWandererRole.PREACHER: return "Brother ";
			case RZ_EWandererRole.KID: return "Little ";
		}

		return "";
	}

	// ================================================================ Lookups and hooks
	//------------------------------------------------------------------------------------------------
	//! The wanderer with this id, or null (Dialogue/RZ_TalkSpeakers.c).
	RZ_Wanderer FindById(int id)
	{
		return Find(id);
	}

	//------------------------------------------------------------------------------------------------
	//! What this wanderer thinks of a player (the talk screen's trees).
	int AttitudeOf(RZ_Wanderer w, int playerId)
	{
		return Attitude(w, playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Wanderer Find(int id)
	{
		foreach (RZ_Wanderer w : m_aList)
		{
			if (w.m_iId == id)
				return w;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! "Old Marta the trader", "" if unknown.
	string GetTitle(int id)
	{
		RZ_Wanderer w = Find(id);
		if (!w)
			return "";

		return w.GetTitle();
	}

	//------------------------------------------------------------------------------------------------
	string GetName(int id)
	{
		RZ_Wanderer w = Find(id);
		if (!w)
			return "";

		return w.GetShortName();
	}

	//------------------------------------------------------------------------------------------------
	//! Where a wanderer is (as data). False if unknown or dead.
	bool GetPosition(int id, out vector pos)
	{
		RZ_Wanderer w = Find(id);
		if (!w || !w.m_bAlive)
			return false;

		pos = w.m_vPos;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool IsAlive(int id)
	{
		RZ_Wanderer w = Find(id);
		if (!w)
			return false;

		return w.m_bAlive;
	}

	//------------------------------------------------------------------------------------------------
	//! A mission is using this person (Hold): no body, no movement.
	bool IsHeld(int id)
	{
		RZ_Wanderer w = Find(id);
		if (!w)
			return false;

		return w.m_iHeldBy >= 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Living wanderers with a role in roleMask (shuffled), for missions and story arcs that
	//! need people on the road without holding them.
	void GetLiving(int roleMask, notnull array<int> result)
	{
		foreach (RZ_Wanderer w : m_aList)
		{
			if (w.m_bAlive && w.m_iHeldBy < 0 && (RoleBit(w.m_eRole) & roleMask) != 0)
				result.Insert(w.m_iId);
		}

		for (int i = result.Count() - 1; i > 0; i--)
		{
			int j = Math.RandomInt(0, i + 1);
			int tmp = result[i];
			result[i] = result[j];
			result[j] = tmp;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "Old Marta" for a living wanderer of a role in roleMask, "" if none. A brief can name them.
	string AnyName(int roleMask)
	{
		array<int> ids = {};
		GetLiving(roleMask, ids);
		if (ids.IsEmpty())
			return "";

		return GetName(ids[0]);
	}

	//------------------------------------------------------------------------------------------------
	//! "near Morton" for a living wanderer, "" if unknown.
	string WhereIs(int id)
	{
		RZ_Wanderer w = Find(id);
		if (!w || !w.m_bAlive)
			return "";

		return WhereText(w);
	}

	//------------------------------------------------------------------------------------------------
	//! A living Haven wanderer with a role in roleMask, not near any player, for a mission
	//! to use as a character (it then holds them). Never one a story has claimed
	//! (RZ_WorldFacts), unless it's `asker`'s own story. -1 if none.
	int FindForMission(int roleMask, RZ_Mission asker = null)
	{
		array<int> options = {};
		RZ_WorldFacts facts = m_GM.GetFacts();
		foreach (RZ_Wanderer w : m_aList)
		{
			if (!w.m_bAlive || w.m_iHeldBy >= 0 || w.m_bHasBody || w.IsBitten() || w.m_iEscortPlayer > 0)
				continue;

			string subject = RZ_WorldFacts.Wanderer(w.m_iId);
			if (facts && facts.IsClaimed(subject) && (!asker || !asker.IsSubjectFree(m_GM, subject)))
				continue;

			if ((RoleBit(w.m_eRole) & roleMask) == 0)
				continue;

			options.Insert(w.m_iId);
		}

		if (options.IsEmpty())
			return -1;

		return options.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! A mission uses this person: no body, no movement until Release.
	void Hold(int id, int missionId)
	{
		RZ_Wanderer w = Find(id);
		if (!w)
			return;

		DespawnBody(w);
		w.m_iHeldBy = missionId;
	}

	//------------------------------------------------------------------------------------------------
	//! Back on the road, at pos, heading somewhere new.
	void Release(int id, vector pos)
	{
		RZ_Wanderer w = Find(id);
		if (!w)
			return;

		w.m_iHeldBy = -1;
		w.m_vPos = pos;
		ChooseDestination(w);
	}

	//------------------------------------------------------------------------------------------------
	void RememberPlayer(int id, int playerId, int flag)
	{
		RZ_Wanderer w = Find(id);
		if (w && playerId > 0)
			w.Remember(m_GM.GetProfile(playerId).m_sIdentity, flag);
	}

	//------------------------------------------------------------------------------------------------
	//! Like RememberPlayer, by identity: also for players who are offline (a questline's ending).
	void RememberIdentity(int id, string ident, int flag)
	{
		RZ_Wanderer w = Find(id);
		if (w && !ident.IsEmpty())
			w.Remember(ident, flag);
	}

	// ---------------------------------------------------------------- The Long Table (World/RZ_LongTable.c)
	//------------------------------------------------------------------------------------------------
	//! A living wanderer on the road as data (no body, nobody within `clear` m), walking a straight
	//! route that passes within `radius` of pos. Never the Kid, nor someone bitten, held, escorted
	//! or nudged. -1 if none.
	int FindPassingNear(vector pos, float radius, float clear, array<IEntity> players)
	{
		foreach (RZ_Wanderer w : m_aList)
		{
			if (!w.m_bAlive || w.m_bHasBody || w.m_iHeldBy >= 0 || w.IsBitten() || w.m_iEscortPlayer > 0 || w.m_iNudgePlayer > 0)
				continue;

			if (w.m_eRole == RZ_EWandererRole.KID || w.m_eState != RZ_EWandererState.TRAVEL)
				continue;

			if (RZ_Util.NearestDistance(w.m_vPos, players) < clear)
				continue;

			if (RZ_LongTable.DistToSegmentXZ(pos, w.m_vPos, w.m_vDest) <= radius)
				return w.m_iId;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Where a living wanderer is going ("the safehouse in Levie"), "" if nowhere.
	string GetDestName(int id)
	{
		RZ_Wanderer w = Find(id);
		if (!w || !w.m_bAlive)
			return "";

		return w.m_sDestName;
	}

	//------------------------------------------------------------------------------------------------
	//! RZ_EWandererRole of a wanderer, -1 if unknown.
	int GetRole(int id)
	{
		RZ_Wanderer w = Find(id);
		if (!w)
			return -1;

		return w.m_eRole;
	}

	//------------------------------------------------------------------------------------------------
	//! Someone who never arrived: gone from the roads as data, with no news of a death (the Long
	//! Table's "never arrived" is the news). Replaced like the dead, a real day later.
	bool TakeOffRoad(int id)
	{
		RZ_Wanderer w = Find(id);
		if (!w || !w.m_bAlive)
			return false;

		DespawnBody(w);
		w.m_bAlive = false;
		w.m_fDiedAt = m_fClock;
		w.m_iEscortPlayer = -1;
		w.m_iNudgePlayer = -1;
		w.m_iHeldBy = -1;
		w.m_fBittenLeft = -1;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The usual bitten wanderer (they head for a safehouse; an antidote within 20 minutes saves
	//! them), for a questline that sent a bitten one on his way. False if they can't be.
	bool BiteOnRoad(int id)
	{
		RZ_Wanderer w = Find(id);
		if (!w || !w.m_bAlive || w.IsBitten() || w.m_iHeldBy >= 0)
			return false;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		OnBitten(w, ids, players);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Someone takes to the roads with this role and first name (the Long Table's boy as the Kid,
	//! its host as a Scavenger). Fills a dead slot or a free place. Their title, or "" if the roster
	//! is full or it's the Kid and a Kid is already on the roads.
	string AddWithRole(int role, string name, vector pos)
	{
		if (!m_Settings.m_bEnabled || m_GM.GetSettlements().IsEmpty())
			return "";

		if (role == RZ_EWandererRole.KID && HasRole(RZ_EWandererRole.KID))
			return "";

		int slot = -1;
		foreach (int i, RZ_Wanderer old : m_aList)
		{
			if (!old.m_bAlive)
			{
				slot = i;
				break;
			}
		}

		if (slot < 0 && m_aList.Count() >= Math.Max(1, m_Settings.m_iMaxCount))
			return "";

		RZ_Wanderer w = Generate(role, RZ_Mission.LandPointNear(pos, 10, 40));
		if (!name.IsEmpty())
			w.m_sName = name;

		if (slot >= 0)
			m_aList[slot] = w;
		else
			m_aList.Insert(w);

		return w.GetTitle();
	}

	//------------------------------------------------------------------------------------------------
	//! A Haven wanderer's body within radius of pos (A Quiet Arrangement: "seen").
	//! Returns their name, "" if nobody saw.
	string SeenByHaven(vector pos, float radius)
	{
		foreach (RZ_Wanderer w : m_aList)
		{
			if (w.m_bAlive && w.m_Body && w.IsHaven() && RZ_Util.DistXZ(w.m_vPos, pos) <= radius)
				return w.GetShortName();
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Hook for new-player guidance: send the nearest living wanderer with a role in roleMask
	//! (e.g. RoleBit(COURIER) | RoleBit(SCAVENGER)) walking toward the player. They give up
	//! after 20 minutes. With maxStart > 0, one farther away who has no body (nobody can see
	//! them) is moved, as data, to that distance first, so they arrive in minutes, not an hour.
	//! True if someone is on the way.
	static bool NudgeToward(int playerId, int roleMask, float maxStart = 0)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetWanderers())
			return false;

		return gm.GetWanderers().Nudge(playerId, roleMask, maxStart);
	}

	//------------------------------------------------------------------------------------------------
	bool Nudge(int playerId, int roleMask, float maxStart = 0)
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!player)
			return false;

		RZ_Wanderer best;
		float bestDist = float.MAX;
		foreach (RZ_Wanderer w : m_aList)
		{
			if (!w.m_bAlive || w.m_iHeldBy >= 0 || w.IsBitten() || w.m_iEscortPlayer > 0 || w.m_iNudgePlayer > 0)
				continue;

			if ((RoleBit(w.m_eRole) & roleMask) == 0)
				continue;

			float d = RZ_Util.DistXZ(w.m_vPos, player.GetOrigin());
			if (d < bestDist)
			{
				bestDist = d;
				best = w;
			}
		}

		if (!best)
			return false;

		if (maxStart > 0 && bestDist > maxStart && !best.m_Body)
		{
			vector from = player.GetOrigin();
			vector p = from;
			p[0] = from[0] + (best.m_vPos[0] - from[0]) * maxStart / bestDist;
			p[2] = from[2] + (best.m_vPos[2] - from[2]) * maxStart / bestDist;
			best.m_vPos = RZ_Util.GroundPos(p);
		}

		best.m_iNudgePlayer = playerId;
		best.m_fNudgeLeft = 1200;
		best.m_vDest = player.GetOrigin();
		best.m_sDestName = "someone new on the road";
		best.m_iDestSafehouse = -1;
		best.m_eState = RZ_EWandererState.TRAVEL;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! "Old Marta the trader, 1.2 km NE" for the nearest living wanderer with a role in
	//! roleMask, "" if none. For guidance hints.
	string DescribeNearest(vector pos, int roleMask)
	{
		RZ_Wanderer best;
		float bestDist = float.MAX;
		foreach (RZ_Wanderer w : m_aList)
		{
			if (!w.m_bAlive || w.m_iHeldBy >= 0 || (RoleBit(w.m_eRole) & roleMask) == 0)
				continue;

			float d = RZ_Util.DistXZ(w.m_vPos, pos);
			if (d < bestDist)
			{
				bestDist = d;
				best = w;
			}
		}

		if (!best)
			return "";

		return best.GetTitle() + ", " + RZ_Util.DirectionText(pos, best.m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	//! Hook for new-player guidance: the title of a wanderer whose body stands within radius
	//! and who would still talk to this player (not the Kid, not someone they robbed or shot
	//! at). "" if none.
	string SpeakerNear(int playerId, vector pos, float radius)
	{
		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		foreach (RZ_Wanderer w : m_aList)
		{
			if (!w.m_bAlive || !w.m_Body || w.m_eRole == RZ_EWandererRole.KID || w.Remembers(ident, MEM_WRONGED))
				continue;

			if (RZ_Util.DistXZ(w.m_vPos, pos) <= radius)
				return w.GetTitle();
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Hostility hook (RZ_Hostility): the wanderer within `radius` of pos who would fight this
	//! player, or null. Only a body that's an AI group with an armed leader can fight; the Kid
	//! never does. A grudge is having been robbed or shot at by them, or, for the Deserter,
	//! facing one of Bastion's Champions (the people he'd draw on anyway).
	RZ_Wanderer FindArmedGrudgeNear(int playerId, vector pos, float radius)
	{
		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		foreach (RZ_Wanderer w : m_aList)
		{
			if (!w.m_bAlive || !w.m_Body || !w.m_bBodyWalks || w.m_eRole == RZ_EWandererRole.KID)
				continue;

			if (RZ_Util.DistXZ(RZ_Util.GetUnitPosition(w.m_Body), pos) > radius)
				continue;

			bool grudge = w.Remembers(ident, MEM_WRONGED);
			if (!grudge && w.m_eRole == RZ_EWandererRole.DESERTER && Attitude(w, playerId) <= -9)
				grudge = true;

			if (!grudge)
				continue;

			SCR_AIGroup group = SCR_AIGroup.Cast(w.m_Body);
			if (!group)
				continue;

			AIAgent leader = group.GetLeaderAgent();
			if (leader && RZ_Util.IsArmed(leader.GetControlledEntity()))
				return w;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Someone from a safehouse or a rescue takes to the roads as a named wanderer.
	//! Fills a dead slot or a free place in the roster. Returns their title, "" if full.
	string AddFromRescue(vector pos)
	{
		return AddNew(pos, true);
	}

	//------------------------------------------------------------------------------------------------
	string AddFromLeaver(vector pos)
	{
		return AddNew(pos, false);
	}

	//------------------------------------------------------------------------------------------------
	protected string AddNew(vector pos, bool rescued)
	{
		if (!m_Settings.m_bEnabled || m_GM.GetSettlements().IsEmpty())
			return "";

		int slot = -1;
		foreach (int i, RZ_Wanderer old : m_aList)
		{
			if (!old.m_bAlive)
			{
				slot = i;
				break;
			}
		}

		if (slot < 0 && m_aList.Count() >= Math.Max(1, m_Settings.m_iMaxCount))
			return "";

		// Rescued people and leavers are never the Deserter or the Kid
		array<int> roles = {RZ_EWandererRole.TRADER, RZ_EWandererRole.DOCTOR, RZ_EWandererRole.SCAVENGER, RZ_EWandererRole.PREACHER, RZ_EWandererRole.COURIER, RZ_EWandererRole.MECHANIC};
		int role = roles.GetRandomElement();
		RZ_Wanderer w = Generate(role, RZ_Mission.LandPointNear(pos, 10, 40));
		if (slot >= 0)
			m_aList[slot] = w;
		else
			m_aList.Insert(w);

		return w.GetTitle();
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! From RZ_Territory: `loser` lost its outpost at pos (taken by `taker`, or abandoned if -1).
	//! Wanderers headed that way pick somewhere else, the road talks about it for an hour, and the
	//! losing side's people are in a mood (Haven wanderers keep to the safehouses).
	void OnOutpostLost(int loser, int taker, vector pos, string place)
	{
		m_aUnrestPos.Insert(pos);
		m_aUnrestUntil.Insert(m_fClock + UNREST_SECONDS);
		m_iMoodFaction = loser;
		m_iMoodWinner = taker;
		m_sMoodPlace = place;
		m_fMoodUntil = m_fClock + LOSS_NEWS_SECONDS;
		m_fRumourUntil = m_fClock + LOSS_NEWS_SECONDS;
		if (taker >= 0)
			m_sLossRumour = string.Format("\"Heard %1 lost %2 to %3. The roads round there are bad now. I'm going the long way.\"", RZ_Factions.GetName(loser), place, RZ_Factions.GetNameInText(taker));
		else
			m_sLossRumour = string.Format("\"%1 pulled out of %2. Nobody's sure why. Nobody's going to look, either.\"", RZ_Factions.GetName(loser), place);

		foreach (RZ_Wanderer w : m_aList)
		{
			if (!w.m_bAlive || w.m_iHeldBy >= 0 || w.m_iEscortPlayer > 0 || w.m_iNudgePlayer > 0 || w.m_eState != RZ_EWandererState.TRAVEL)
				continue;

			if (RZ_Util.DistXZ(w.m_vDest, pos) <= UNREST_RADIUS)
				ChooseDestination(w);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsUnrest(vector pos)
	{
		for (int i = m_aUnrestPos.Count() - 1; i >= 0; i--)
		{
			if (m_fClock >= m_aUnrestUntil[i])
			{
				m_aUnrestPos.Remove(i);
				m_aUnrestUntil.Remove(i);
				continue;
			}

			if (RZ_Util.DistXZ(m_aUnrestPos[i], pos) <= UNREST_RADIUS)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The mood after a lost outpost, as a line before what they say. "" if none.
	protected string MoodLine(RZ_Wanderer w)
	{
		if (m_fClock >= m_fMoodUntil || m_iMoodFaction < 0)
			return "";

		if (w.m_eRole == RZ_EWandererRole.DESERTER && m_iMoodFaction == RZ_EFaction.MILITARY)
			return string.Format("\"Bastion lost %1? Told them. Nobody listens to a deserter.\" ", m_sMoodPlace);

		if (m_iMoodFaction == RZ_EFaction.SURVIVORS && w.IsHaven())
			return string.Format("They keep looking back the way they came. \"We lost %1. People we knew.\" ", m_sMoodPlace);

		if (m_iMoodWinner == RZ_EFaction.BANDITS && w.IsHaven())
			return string.Format("\"The Red Hand's in %1 now. Keep your voice down.\" ", m_sMoodPlace);

		return "";
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (m_aList.IsEmpty())
			return;

		m_fClock += dt;
		for (int i = 0; i < m_aList.Count(); i++)
		{
			RZ_Wanderer w = m_aList[i];
			if (!w.m_bAlive)
			{
				if (m_fClock - w.m_fDiedAt >= m_Settings.m_fReplaceMinutes * 60)
					m_aList[i] = Replace(w);

				continue;
			}

			if (w.m_iHeldBy >= 0)
				continue;

			UpdateBody(w, ids, players);
			if (!w.m_bAlive)
				continue;

			if (w.IsBitten())
				TickBite(w, dt, ids, players);

			if (w.m_bAlive)
				Move(w, dt, players);
		}

		m_fReactTimer += dt;
		if (m_fReactTimer >= REACT_INTERVAL)
		{
			m_fReactTimer = 0;
			ReactAll(ids, players);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A replacement: half the time the same role, otherwise whatever the roster lacks.
	protected RZ_Wanderer Replace(RZ_Wanderer dead)
	{
		int role = -1;
		if (Math.RandomFloat01() < 0.5)
			role = dead.m_eRole;

		RZ_Wanderer w = Generate(role, StartPoint());
		w.m_iGeneration = dead.m_iGeneration + 1;
		return w;
	}

	//------------------------------------------------------------------------------------------------
	protected void Move(RZ_Wanderer w, float dt, array<IEntity> players)
	{
		switch (w.m_eState)
		{
			case RZ_EWandererState.REST:
			{
				w.m_fStateTimer = w.m_fStateTimer - dt;
				if (w.m_fStateTimer <= 0)
				{
					ChooseDestination(w);
					w.m_eState = RZ_EWandererState.TRAVEL;
				}

				return;
			}
			case RZ_EWandererState.HIDE:
			{
				w.m_fStateTimer = w.m_fStateTimer - dt;
				if (w.m_fStateTimer <= 0)
					w.m_eState = RZ_EWandererState.TRAVEL;

				return;
			}
			case RZ_EWandererState.SHELTER:
				return;
		}

		// TRAVEL. A spawned AI group walks on its own (position synced in UpdateBody);
		// a prop or character without AI stands still while someone can see it.
		if (w.m_Body && !SCR_AIGroup.Cast(w.m_Body))
		{
			CheckArrival(w);
			return;
		}

		// The Kid only walks on while their escort keeps up
		if (w.m_iEscortPlayer > 0)
		{
			IEntity escort = GetGame().GetPlayerManager().GetPlayerControlledEntity(w.m_iEscortPlayer);
			if (!escort || RZ_Util.DistXZ(escort.GetOrigin(), w.m_vPos) > 150)
				return;
		}

		if (!w.m_Body)
		{
			float dist = RZ_Util.DistXZ(w.m_vPos, w.m_vDest);
			float step = Math.Min(m_Settings.m_fWalkSpeed * dt, dist);
			if (dist > 0.1)
			{
				vector p = w.m_vPos;
				p[0] = p[0] + (w.m_vDest[0] - p[0]) * step / dist;
				p[2] = p[2] + (w.m_vDest[2] - p[2]) * step / dist;
				w.m_vPos = p;
			}
		}

		CheckArrival(w);
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckArrival(RZ_Wanderer w)
	{
		if (RZ_Util.DistXZ(w.m_vPos, w.m_vDest) > ARRIVE_RADIUS)
			return;

		if (w.m_iEscortPlayer > 0 && w.m_iDestSafehouse >= 0)
		{
			SettleKid(w);
			return;
		}

		w.m_eState = RZ_EWandererState.REST;
		if (w.m_iNudgePlayer > 0)
		{
			// Found the new arrival: wait around for them a while
			w.m_iNudgePlayer = -1;
			w.m_fStateTimer = 300;
			return;
		}

		if (w.m_iDestSafehouse >= 0)
			w.m_fStateTimer = Math.RandomFloat(600, 1200);
		else
			w.m_fStateTimer = Math.RandomFloat(180, 480);

		if (m_GM.GetStoryArcs())
			m_GM.GetStoryArcs().OnWandererRested(w.m_iId, w.m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	//! Somewhere 600 m to 3.5 km away that isn't overrun, near the line, or across water.
	//! Traders and Doctors prefer safehouses, the Preacher prefers towns, the Kid wanders.
	protected void ChooseDestination(RZ_Wanderer w)
	{
		array<vector> points = {};
		array<string> names = {};
		array<int> houses = {};
		array<float> weights = {};
		float total = 0;

		RZ_Safehouses safehouses = m_GM.GetSafehouses();

		// A story arc can call the Preacher to one safehouse (The Quiet Congregation)
		if (safehouses && w.m_eRole == RZ_EWandererRole.PREACHER && m_GM.GetStoryArcs())
		{
			int calledTo = m_GM.GetStoryArcs().PreacherHouse(w.m_iId);
			RZ_Safehouse called = safehouses.Get(calledTo);
			if (called)
			{
				w.m_vDest = called.m_vPos;
				w.m_sDestName = "the safehouse in " + called.m_sName;
				w.m_iDestSafehouse = calledTo;
				return;
			}
		}

		if (safehouses && w.m_eRole != RZ_EWandererRole.KID)
		{
			float houseWeight = 1;
			if (w.m_eRole == RZ_EWandererRole.TRADER || w.m_eRole == RZ_EWandererRole.DOCTOR)
				houseWeight = 3;
			else if (w.m_eRole == RZ_EWandererRole.COURIER)
				houseWeight = 2;

			// The Haven lost an outpost: its people keep close to the safehouses for a while
			if (m_iMoodFaction == RZ_EFaction.SURVIVORS && m_fClock < m_fMoodUntil && w.IsHaven())
				houseWeight = houseWeight * 2;

			for (int h = 0; h < safehouses.Count(); h++)
			{
				RZ_Safehouse house = safehouses.Get(h);
				float hd = RZ_Util.DistXZ(w.m_vPos, house.m_vPos);
				if (hd < 300 || hd > 4500 || !IsRouteClear(w.m_vPos, house.m_vPos) || IsUnrest(house.m_vPos) || HungryWay(w, house.m_vPos))
					continue;

				points.Insert(house.m_vPos);
				names.Insert("the safehouse in " + house.m_sName);
				houses.Insert(h);
				weights.Insert(houseWeight);
				total += houseWeight;
			}
		}

		float townWeight = 1;
		if (w.m_eRole == RZ_EWandererRole.PREACHER)
			townWeight = 3;

		foreach (RZ_Settlement s : m_GM.GetSettlements())
		{
			float sd = RZ_Util.DistXZ(w.m_vPos, s.m_vCenter);
			if (sd < 600 || sd > 3500 || s.m_fInfection >= 70)
				continue;

			vector at = RZ_Mission.LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.5);
			if (!IsRouteClear(w.m_vPos, at) || IsUnrest(at) || HungryWay(w, at))
				continue;

			points.Insert(at);
			names.Insert(s.m_sName);
			houses.Insert(-1);
			weights.Insert(townWeight);
			total += townWeight;
		}

		if (points.IsEmpty())
		{
			// Nowhere good to go: stay put for a while and try again
			w.m_vDest = w.m_vPos;
			w.m_sDestName = "";
			w.m_iDestSafehouse = -1;
			w.m_eState = RZ_EWandererState.REST;
			w.m_fStateTimer = 300;
			return;
		}

		float roll = Math.RandomFloat(0, total);
		int pick = points.Count() - 1;
		for (int i = 0; i < points.Count(); i++)
		{
			roll -= weights[i];
			if (roll <= 0)
			{
				pick = i;
				break;
			}
		}

		w.m_vDest = points[pick];
		w.m_sDestName = names[pick];
		w.m_iDestSafehouse = houses[pick];
		w.m_eState = RZ_EWandererState.TRAVEL;
		if (w.m_Body)
			RZ_Util.RedirectGroup(w.m_Body, w.m_vDest, m_GM.GetSpawner().GetMoveWaypointPrefab());
	}

	//------------------------------------------------------------------------------------------------
	//! The Haven's own go round a hungry pack's ground at night (docs/INFECTED.md, The factions).
	protected bool HungryWay(RZ_Wanderer w, vector to)
	{
		RZ_Tribes tribes = m_GM.GetTribes();
		if (!w.IsHaven() || !tribes || !tribes.GetPeople().IsOn() || tribes.GetPhase() == RZ_ETribePhase.DAY)
			return false;

		return tribes.GetPeople().CrossesHungry(w.m_vPos, to);
	}

	//------------------------------------------------------------------------------------------------
	//! Five points along the straight line: no water, no quarantine line.
	protected bool IsRouteClear(vector from, vector to)
	{
		for (int i = 1; i <= 5; i++)
		{
			vector p = RZ_Util.GroundPos(from + (to - from) * (i / 6.0));
			if (RZ_Util.IsWater(p) || m_GM.GetPerimeter().IsNearLine(p))
				return false;
		}

		return true;
	}

	// ================================================================ Bodies
	//------------------------------------------------------------------------------------------------
	protected void UpdateBody(RZ_Wanderer w, array<int> ids, array<IEntity> players)
	{
		if (w.m_bHasBody)
		{
			if (!w.m_Body)
			{
				// Removed by someone else (cleanup, admin): not a death
				w.m_bHasBody = false;
			}
			else if (RZ_Util.CountAlive(w.m_Body) == 0)
			{
				OnBodyKilled(w);
				return;
			}
			else
			{
				if (!w.m_bTagged)
					TryTagBody(w);

				w.m_vPos = RZ_Util.GetUnitPosition(w.m_Body);
				if (RZ_Util.NearestDistance(w.m_vPos, players) > m_Settings.m_fSpawnRadius + DESPAWN_MARGIN)
					DespawnBody(w);

				return;
			}
		}

		if (m_fClock < w.m_fHiddenUntil)
			return;

		// The Kid without an AI body walks on ahead, unseen, while escorted
		if (w.m_iEscortPlayer > 0 && w.m_eState == RZ_EWandererState.TRAVEL && !w.m_bBodyWalks)
			return;

		foreach (IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), w.m_vPos) > m_Settings.m_fSpawnRadius)
				continue;

			if (CountBodiesNear(p.GetOrigin()) >= 2)
				continue;

			SpawnBody(w);
			if (w.m_iNudgePlayer > 0)
				OnNudgeSighted(w);

			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Guidance: the wanderer who came looking for a new player is in sight. They wave and wait.
	protected void OnNudgeSighted(RZ_Wanderer w)
	{
		IEntity target = GetGame().GetPlayerManager().GetPlayerControlledEntity(w.m_iNudgePlayer);
		if (!w.m_Body || !target || RZ_Util.DistXZ(target.GetOrigin(), w.m_vPos) > m_Settings.m_fSpawnRadius)
			return;

		m_GM.Say(w.m_iNudgePlayer, w.GetTitle(), -1, string.Format("Someone on the road, %1, is waving an arm over their head. They've seen you, and they're waiting.",
			RZ_Util.DirectionText(target.GetOrigin(), w.m_vPos)));
		w.m_iNudgePlayer = -1;
		w.m_eState = RZ_EWandererState.REST;
		w.m_fStateTimer = 300;
	}

	//------------------------------------------------------------------------------------------------
	protected int CountBodiesNear(vector pos)
	{
		int n = 0;
		foreach (RZ_Wanderer w : m_aList)
		{
			if (w.m_Body && RZ_Util.DistXZ(w.m_vPos, pos) <= m_Settings.m_fSpawnRadius + DESPAWN_MARGIN)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected ResourceName GetPrefab(RZ_EWandererRole role)
	{
		ResourceName prefab;
		switch (role)
		{
			case RZ_EWandererRole.TRADER: prefab = m_Settings.m_sTraderPrefab; break;
			case RZ_EWandererRole.DOCTOR: prefab = m_Settings.m_sDoctorPrefab; break;
			case RZ_EWandererRole.SCAVENGER: prefab = m_Settings.m_sScavengerPrefab; break;
			case RZ_EWandererRole.DESERTER: prefab = m_Settings.m_sDeserterPrefab; break;
			case RZ_EWandererRole.PREACHER: prefab = m_Settings.m_sPreacherPrefab; break;
			case RZ_EWandererRole.COURIER: prefab = m_Settings.m_sCourierPrefab; break;
			case RZ_EWandererRole.KID: prefab = m_Settings.m_sKidPrefab; break;
			case RZ_EWandererRole.MECHANIC: prefab = m_Settings.m_sMechanicPrefab; break;
		}

		if (prefab.IsEmpty())
			prefab = m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);

		return prefab;
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnBody(RZ_Wanderer w)
	{
		vector pos = RZ_Util.GroundPos(w.m_vPos);
		if (RZ_Util.IsWater(pos))
			pos = RZ_Mission.LandPointNear(pos, 5, 40);

		// The one body budget: with the agent budget full the wanderer stays data and tries again
		RZ_Shared shared = m_GM.GetShared();
		if (shared && !shared.CanSpawnBody(RZ_BodyBudget.WANDERER, pos))
			return;

		IEntity e = RZ_Util.SpawnPrefab(GetPrefab(w.m_eRole), pos, Math.RandomFloat(0, 360));
		if (!e)
			return;

		if (shared)
			shared.AddBody(RZ_BodyBudget.WANDERER, e);

		w.m_Body = e;
		w.m_bHasBody = true;
		w.m_bBodyWalks = SCR_AIGroup.Cast(e) != null;
		w.m_bTagged = false;
		w.m_vPos = pos;
		TryTagBody(w);

		if (w.m_eState == RZ_EWandererState.TRAVEL)
			RZ_Util.SendGroupTo(e, w.m_vDest, m_GM.GetSpawner().GetMoveWaypointPrefab());
	}

	//------------------------------------------------------------------------------------------------
	//! Label the body's actions. For an AI group the components are on its leader, who
	//! only exists a frame after the group, so this is retried on the next ticks.
	protected void TryTagBody(RZ_Wanderer w)
	{
		IEntity target = w.m_Body;
		SCR_AIGroup group = SCR_AIGroup.Cast(w.m_Body);
		if (group)
		{
			AIAgent leader = group.GetLeaderAgent();
			if (!leader || !leader.GetControlledEntity())
				return;

			target = leader.GetControlledEntity();
		}

		RZ_PuzzleObjectComponent.Tag(target, TAG, w.m_iId, "", ChoiceLabels(w));
		RZ_ClueComponent.Tag(target, TAG, w.m_iId, "Talk to " + w.GetShortName());
		w.m_bTagged = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void DespawnBody(RZ_Wanderer w)
	{
		if (w.m_Body)
			RZ_Util.DeleteUnit(w.m_Body);

		w.m_Body = null;
		w.m_bHasBody = false;
	}

	//------------------------------------------------------------------------------------------------
	//! Choice slots: 0 the role's special, 1 give food, 2 the Mechanic's "Ask about cars" (while
	//! bitten, anyone's "Give them your antidote"), 3 rob.
	protected string ChoiceLabels(RZ_Wanderer w)
	{
		string special = "";
		switch (w.m_eRole)
		{
			case RZ_EWandererRole.TRADER: special = string.Format("Buy supplies (%1 Haven Favor)", m_Settings.m_iTradeCost); break;
			case RZ_EWandererRole.DOCTOR: special = string.Format("Ask for treatment (%1 Haven Favor)", m_Settings.m_iTreatCost); break;
			case RZ_EWandererRole.SCAVENGER: special = string.Format("Buy a rumour (%1 Haven Favor)", m_Settings.m_iRumourCost); break;
			case RZ_EWandererRole.DESERTER: special = "Ask about Bastion"; break;
			case RZ_EWandererRole.PREACHER: special = "Ask him to stop ringing the bells"; break;
			case RZ_EWandererRole.COURIER: special = "Ask for work"; break;
			case RZ_EWandererRole.KID: special = "Show the kid the way to a safehouse"; break;
			case RZ_EWandererRole.MECHANIC: special = string.Format("Buy a car part (%1 Haven Favor)", PartCost()); break;
		}

		// Bitten: slot 2 is the antidote (a player chooses to give theirs; it's never taken)
		string extra = "";
		if (w.IsBitten())
			extra = "Give them your antidote";
		else if (w.m_eRole == RZ_EWandererRole.MECHANIC)
			extra = "Ask about cars";
		else if (m_GM.GetErrands() && m_GM.GetErrands().AsksOn() && w.m_eRole != RZ_EWandererRole.KID)
			extra = "Need a hand?";

		array<string> labels = {special, "Share some food", extra, "Rob " + w.GetShortName()};
		return RZ_PuzzleLabels.Join(labels);
	}

	//------------------------------------------------------------------------------------------------
	//! Runs away from `from`. A body that can't walk is removed and moves on unseen.
	protected void Flee(RZ_Wanderer w, vector from)
	{
		vector dir = w.m_vPos - from;
		dir[1] = 0;
		float len = dir.Length();
		if (len < 1)
			dir = Vector(1, 0, 0);
		else
			dir = dir * (1 / len);

		vector away = RZ_Mission.LandPointNear(w.m_vPos + dir * 250, 0, 30);
		if (w.m_Body && SCR_AIGroup.Cast(w.m_Body))
		{
			RZ_Util.RedirectGroup(w.m_Body, away, m_GM.GetSpawner().GetMoveWaypointPrefab());
			return;
		}

		DespawnBody(w);
		w.m_vPos = away;
		w.m_fHiddenUntil = m_fClock + 180;
	}

	// ================================================================ Reacting to the world
	//------------------------------------------------------------------------------------------------
	protected void ReactAll(array<int> ids, array<IEntity> players)
	{
		RZ_MissionManager missions = m_GM.GetMissions();
		bool shelter = missions.IsTypeRunning(RZ_Event_EveryRidgeCalls) || missions.IsTypeRunning(RZ_Event_Storm);
		if (m_GM.GetEnvironment() && m_GM.GetEnvironment().IsStorm())
			shelter = true;

		foreach (RZ_Wanderer w : m_aList)
		{
			if (!w.m_bAlive || w.m_iHeldBy >= 0)
				continue;

			React(w, shelter, ids, players);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void React(RZ_Wanderer w, bool shelter, array<int> ids, array<IEntity> players)
	{
		// Every Ridge Calls or a storm: find a doorway and wait
		if (shelter && w.m_eState != RZ_EWandererState.SHELTER && !w.IsBitten())
		{
			w.m_eState = RZ_EWandererState.SHELTER;
			return;
		}

		if (!shelter && w.m_eState == RZ_EWandererState.SHELTER)
			w.m_eState = RZ_EWandererState.TRAVEL;

		// Guidance: keep walking toward the player who was nudged
		if (w.m_iNudgePlayer > 0)
		{
			w.m_fNudgeLeft = w.m_fNudgeLeft - REACT_INTERVAL;
			IEntity target = GetGame().GetPlayerManager().GetPlayerControlledEntity(w.m_iNudgePlayer);
			if (!target || w.m_fNudgeLeft <= 0)
				w.m_iNudgePlayer = -1;
			else if (w.m_eState == RZ_EWandererState.TRAVEL)
				w.m_vDest = target.GetOrigin();
		}

		// The Kid gives up on an escort who wandered off (or left)
		if (w.m_iEscortPlayer > 0)
		{
			IEntity escort = GetGame().GetPlayerManager().GetPlayerControlledEntity(w.m_iEscortPlayer);
			if (!escort || RZ_Util.DistXZ(escort.GetOrigin(), w.m_vPos) > 400)
			{
				w.m_iEscortPlayer = -1;
				ChooseDestination(w);
			}
		}

		bool zombieClose = m_GM.GetSpawner().IsZombieNear(w.m_vPos, 40);

		// Rarely, the infected get one of them
		if (!w.IsBitten() && w.m_eRole != RZ_EWandererRole.KID && Math.RandomFloat01() < m_Settings.m_fBiteChance
			&& m_GM.GetSpawner().IsZombieNear(w.m_vPos, 12))
		{
			OnBitten(w, ids, players);
		}

		if (w.m_eState == RZ_EWandererState.TRAVEL)
		{
			// A horde or a lot of noise nearby: keep still until it passes
			if (zombieClose || m_GM.GetHeat().GetHeat(w.m_vPos) >= 50)
			{
				w.m_eState = RZ_EWandererState.HIDE;
				w.m_fStateTimer = HIDE_SECONDS;
			}
			else if (w.m_iNudgePlayer <= 0 && w.m_iEscortPlayer <= 0 && IsBadDestination(w))
			{
				ChooseDestination(w);
			}
		}

		// The Preacher's bells draw the infected to whatever town he's resting in
		if (w.m_eRole == RZ_EWandererRole.PREACHER && w.m_eState == RZ_EWandererState.REST && w.m_iDestSafehouse < 0 && m_fClock >= w.m_fBellsOffUntil)
		{
			// Louder while his flock follows him (story arcs)
			float bells = 6;
			if (m_GM.GetStoryArcs())
				bells = bells * m_GM.GetStoryArcs().BellMult(w.m_iId);

			m_GM.GetHeat().AddHeat(w.m_vPos, bells);
		}

		if (w.m_Body && w.IsHaven())
			WarnAboutExiles(w, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	//! The destination fell to the infected, got loud, or the wanderer drifted near the line.
	protected bool IsBadDestination(RZ_Wanderer w)
	{
		if (m_GM.GetPerimeter().IsNearLine(w.m_vPos) || m_GM.GetPerimeter().IsNearLine(w.m_vDest))
			return true;

		if (m_GM.GetHeat().GetHeat(w.m_vDest) >= 60)
			return true;

		if (w.m_iDestSafehouse >= 0)
			return false; // safehouses never fall

		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(w.m_vDest));
		return s && s.m_fInfection >= 75;
	}

	//------------------------------------------------------------------------------------------------
	//! Haven wanderers tell other players about someone the Haven turned out.
	protected void WarnAboutExiles(RZ_Wanderer w, array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), w.m_vPos) > WARN_RADIUS || !RZ_Reputation.IsHunted(m_GM.GetTier(ids[i], RZ_EFaction.SURVIVORS)))
				continue;

			foreach (int j, IEntity other : players)
			{
				if (j == i || RZ_Util.DistXZ(other.GetOrigin(), w.m_vPos) > WARN_RADIUS)
					continue;

				string key = string.Format("%1:%2:%3", w.m_iId, ids[i], ids[j]);
				float last;
				if (m_mWarned.Find(key, last) && m_fClock - last < WARN_COOLDOWN)
					continue;

				m_mWarned.Set(key, m_fClock);
				m_GM.Say(ids[j], w.GetTitle(), -1, string.Format("\"That one there. %1. The Haven turned them out. Don't let them follow you home.\"", RZ_Util.PlayerName(ids[i])));
			}
		}
	}

	// ================================================================ Bites and deaths
	//------------------------------------------------------------------------------------------------
	//! The bitten wanderer event: they head for the nearest safehouse; an antidote saves them.
	protected void OnBitten(RZ_Wanderer w, array<int> ids, array<IEntity> players)
	{
		w.m_fBittenLeft = BITE_SECONDS;
		w.m_bTagged = false; // relabel: "Give them your antidote"
		RZ_WorldWriters.WandererBitten(w.m_iId, w.GetShortName(), w.m_vPos);
		RZ_Safehouses houses = m_GM.GetSafehouses();
		int idx = -1;
		if (houses)
			idx = houses.FindNearest(w.m_vPos);

		RZ_Safehouse h;
		if (houses)
			h = houses.Get(idx);

		if (h)
		{
			w.m_vDest = h.m_vPos;
			w.m_sDestName = "the safehouse in " + h.m_sName;
			w.m_iDestSafehouse = idx;
			w.m_eState = RZ_EWandererState.TRAVEL;
		}

		string text = string.Format("%1 has been bitten near %2. They're making for %3. An antidote would save them, if someone gets there in the next twenty minutes.",
			w.GetTitle(), NearPlace(w.m_vPos), w.m_sDestName);
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), w.m_vPos) <= 1500)
				m_GM.NotifyPlayer(ids[i], "Bitten on the road", text + "\n(" + RZ_Util.DirectionText(p.GetOrigin(), w.m_vPos) + ")");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void TickBite(RZ_Wanderer w, float dt, array<int> ids, array<IEntity> players)
	{
		// A Doctor on the roads nearby slows it
		float rate = 1;
		foreach (RZ_Wanderer other : m_aList)
		{
			if (other != w && other.m_bAlive && other.m_eRole == RZ_EWandererRole.DOCTOR && RZ_Util.DistXZ(other.m_vPos, w.m_vPos) <= 100)
				rate = 0.5;
		}

		w.m_fBittenLeft = w.m_fBittenLeft - dt * rate;

		if (w.m_fBittenLeft > 0)
			return;

		w.m_fBittenLeft = -1;
		vector at = w.m_vPos;
		Die(w, "turned", -1);
		if (RZ_Util.NearestDistance(at, players) <= 600)
			m_GM.GetSpawner().SpawnZombie(RZ_Util.GroundPos(at), true, m_GM.FindNearestSettlement(at), -1);
	}

	//------------------------------------------------------------------------------------------------
	//! "Give them your antidote" on a bitten wanderer: the player's own dose, by their choice.
	protected void GiveAntidote(RZ_Wanderer w, int playerId, IEntity user)
	{
		if (!RZ_AntidoteHelper.TakeOne(user))
		{
			m_GM.NotifyPlayer(playerId, w.GetTitle(), "You've no antidote to give. They look at your empty hands and nod.");
			return;
		}

		w.m_fBittenLeft = -1;
		w.m_bTagged = false; // back to the usual labels
		w.Remember(m_GM.GetProfile(playerId).m_sIdentity, MEM_SAVED);
		m_GM.AddFame(playerId, RZ_EFaction.SURVIVORS, CURED_FAME);
		m_GM.NotifyPlayer(playerId, w.GetTitle(), "The needle goes in. A long minute, then the shaking stops. \"I owe you. I'll remember. All season, I'll remember.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBodyKilled(RZ_Wanderer w)
	{
		// Who killed them, read from the body (as bounties do); the infected and accidents are no one
		int killer = -1;
		array<IEntity> members = {};
		RZ_Util.GetUnitMembers(w.m_Body, members);
		if (members.IsEmpty())
			members.Insert(w.m_Body);

		foreach (IEntity member : members)
		{
			killer = RZ_Util.GetKillerPlayerId(member);
			if (killer > 0)
				break;
		}

		// The engine didn't say: whoever shot at them lately, unless the infected were on them
		// (shooting near a wanderer the infected then kill shouldn't hang a murder on you)
		bool infectedNear = m_GM.GetSpawner().IsZombieNear(w.m_vPos, 20);
		if (killer <= 0 && !infectedNear && w.m_iLastShooter > 0 && m_fClock - w.m_fLastShotAt <= BLAME_SECONDS)
			killer = w.m_iLastShooter;

		string how = "was killed";
		if (killer <= 0 && infectedNear)
			how = "was killed by the infected";

		Die(w, how, killer);
	}

	//------------------------------------------------------------------------------------------------
	//! Death is permanent and makes the news. A replacement comes a real day later.
	protected void Die(RZ_Wanderer w, string how, int killer)
	{
		// Grief: no jokes near where it happened for a while
		RZ_ToneGuard.Death(w.m_vPos, -1);

		string text = string.Format("%1 %2 %3.", w.GetTitle(), how, WhereText(w));

		// The infected's dead are "the dead" to the Long Table for an hour (A Place at the Table)
		if (m_GM.GetLongTable())
			m_GM.GetLongTable().OnWandererDied(w.m_vPos, w.GetTitle(), killer <= 0 && (how == "turned" || how.Contains("infected")));
		// Taken in the open near a tribe's range: that tribe took them (docs/INFECTED.md)
		if (m_GM.GetTribes())
			m_GM.GetTribes().OnWandererDied(w.m_vPos, w.GetTitle(), how, killer);

		DespawnBody(w);
		w.m_bAlive = false;
		w.m_fDiedAt = m_fClock;
		w.m_iEscortPlayer = -1;
		w.m_iNudgePlayer = -1;
		m_GM.NotifyAll("News from the roads", text);
		if (m_GM.GetHeadlines())
			m_GM.GetHeadlines().OnWandererLost(text);

		if (killer > 0)
		{
			m_GM.AddInfamy(killer, RZ_EFaction.SURVIVORS, m_Settings.m_iKillInfamy);
			m_GM.NotifyPlayer(killer, "The Haven", string.Format("Everyone will know who killed %1. (+%2 Haven Infamy)", w.GetShortName(), m_Settings.m_iKillInfamy));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A living wanderer within `radius` of pos (data or body): road news, a witness.
	bool AnyNear(vector pos, float radius)
	{
		foreach (RZ_Wanderer w : m_aList)
		{
			if (w.m_bAlive && RZ_Util.DistXZ(w.m_vPos, pos) <= radius)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A tribe's hunt takes a wanderer as data (docs/INFECTED.md, the taken): a living wanderer with no
	//! body within radius of pos, in the open. False if there's none.
	bool TakeNear(vector pos, float radius)
	{
		foreach (RZ_Wanderer w : m_aList)
		{
			if (!w.m_bAlive || w.m_Body || w.m_bSettled || RZ_Util.DistXZ(w.m_vPos, pos) > radius)
				continue;

			Die(w, "was taken by the infected", -1);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! "on the road to Morton" or "near Morton".
	protected string WhereText(RZ_Wanderer w)
	{
		if (w.m_eState == RZ_EWandererState.TRAVEL && !w.m_sDestName.IsEmpty() && RZ_Util.DistXZ(w.m_vPos, w.m_vDest) > 200)
			return "on the road to " + w.m_sDestName;

		return "near " + NearPlace(w.m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	protected string NearPlace(vector pos)
	{
		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(pos));
		if (s)
			return s.m_sName;

		return RZ_SettlementScanner.GridReference(pos);
	}

	//------------------------------------------------------------------------------------------------
	//! The Kid reaches the safehouse with their escort: one more mouth, one less on the roads.
	protected void SettleKid(RZ_Wanderer w)
	{
		int escort = w.m_iEscortPlayer;
		w.m_iEscortPlayer = -1;
		RZ_Safehouses houses = m_GM.GetSafehouses();
		RZ_Safehouse h;
		if (houses)
			h = houses.Get(w.m_iDestSafehouse);

		if (h)
			houses.AddPopulation(h.m_vPos, 1);

		if (escort > 0)
		{
			m_GM.AddFame(escort, RZ_EFaction.SURVIVORS, KID_FAME);
			string where = "the safehouse";
			if (h)
				where = "the safehouse in " + h.m_sName;

			m_GM.NotifyPlayer(escort, w.GetTitle(), string.Format("The kid runs the last bit to %1 and doesn't look back. Someone inside shouts a name. +%2 Haven Fame.", where, KID_FAME));
		}

		DespawnBody(w);
		w.m_bAlive = false;
		w.m_bSettled = true;
		w.m_fDiedAt = m_fClock;
	}

	// ================================================================ Gunfire and kills
	//------------------------------------------------------------------------------------------------
	//! From the gunfire hook. Shooting near a wanderer (with no infected near them) counts
	//! as shooting at them. The Kid runs from any gunfire.
	void OnPlayerShot(int playerId, vector pos)
	{
		foreach (RZ_Wanderer w : m_aList)
		{
			if (!w.m_bAlive || !w.m_Body)
				continue;

			float d = RZ_Util.DistXZ(w.m_vPos, pos);
			if (w.m_eRole == RZ_EWandererRole.KID && d <= 60 && w.m_iEscortPlayer != playerId)
			{
				Flee(w, pos);
				continue;
			}

			if (d > 40 || m_GM.GetSpawner().IsZombieNear(w.m_vPos, 30))
				continue;

			w.m_iLastShooter = playerId;
			w.m_fLastShotAt = m_fClock;
			string ident = m_GM.GetProfile(playerId).m_sIdentity;
			if (w.Remembers(ident, MEM_WRONGED))
				continue;

			w.Remember(ident, MEM_WRONGED);
			if (w.IsHaven())
				m_GM.AddInfamy(playerId, RZ_EFaction.SURVIVORS, m_Settings.m_iShotInfamy);

			m_GM.Say(playerId, w.GetTitle(), -1, "They flinch, then run. Every safehouse will hear about this by tonight.");
			Flee(w, pos);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Infected killed right next to a wanderer: whoever did it saved their life.
	void OnZombiesKilled(vector pos)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		bool loaded = false;
		foreach (RZ_Wanderer w : m_aList)
		{
			if (!w.m_bAlive || !w.m_Body || RZ_Util.DistXZ(w.m_vPos, pos) > 25)
				continue;

			if (!loaded)
			{
				RZ_Util.GetAlivePlayers(ids, players);
				loaded = true;
			}

			int best = -1;
			float bestDist = 80;
			foreach (int i, IEntity p : players)
			{
				float d = RZ_Util.DistXZ(p.GetOrigin(), pos);
				if (d <= bestDist)
				{
					bestDist = d;
					best = ids[i];
				}
			}

			if (best <= 0)
				continue;

			string ident = m_GM.GetProfile(best).m_sIdentity;
			if (w.Remembers(ident, MEM_SAVED))
				continue;

			w.Remember(ident, MEM_SAVED);
			m_GM.AddFavor(best, RZ_EFaction.SURVIVORS, SAVED_GIFT_FAVOR);
			m_GM.NotifyPlayer(best, w.GetTitle(), string.Format("%1 grabs your sleeve, still shaking. \"That one nearly had me. I won't forget it. Not this season.\" They press something into your hand. (+%2 Haven Favor)",
				w.GetShortName(), SAVED_GIFT_FAVOR));
		}
	}

	// ================================================================ Talking and trading
	//------------------------------------------------------------------------------------------------
	//! Reputation with their faction plus their memory of you. -2 or lower: they refuse.
	//! The Deserter is the other way round about Bastion, and won't deal with its Champions.
	protected int Attitude(RZ_Wanderer w, int playerId)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		int score = 0;
		if (w.m_eRole == RZ_EWandererRole.DESERTER)
		{
			RZ_ETier bastion = profile.GetTier(RZ_EFaction.MILITARY);
			if (bastion == RZ_ETier.CHAMPION)
				return -9;

			score = -RZ_Reputation.GetAccessRank(bastion);
		}
		else
		{
			RZ_ETier haven = profile.GetTier(RZ_EFaction.SURVIVORS);
			if (RZ_Reputation.IsHunted(haven))
				return -5;

			if (w.m_eRole == RZ_EWandererRole.DOCTOR && RZ_Reputation.IsAtLeast(profile.GetTier(RZ_EFaction.BANDITS), RZ_ETier.TRUSTED))
				return -4;

			score = RZ_Reputation.GetAccessRank(haven);
		}

		int mem = w.GetMemory(profile.m_sIdentity);
		if ((mem & MEM_WRONGED) != 0)
			score -= 3;
		if ((mem & MEM_HELPED) != 0)
			score += 1;
		if ((mem & MEM_SAVED) != 0)
			score += 2;

		// Earlier lives at half, rounded toward zero (docs/REPUTATION.md, "Face and name")
		int old = w.OldMemory(profile.m_sIdentity);
		int oldScore = 0;
		if ((old & MEM_WRONGED) != 0)
			oldScore -= 3;
		if ((old & MEM_HELPED) != 0)
			oldScore += 1;
		if ((old & MEM_SAVED) != 0)
			oldScore += 2;

		score += oldScore / 2;

		// How you talk to them (Dialogue > Trees): regard / 2
		score += RZ_DialogueMemory.AttitudeFrom(playerId, string.Format("wanderer:%1:%2", w.m_iId, w.m_iGeneration));

		return score;
	}

	//------------------------------------------------------------------------------------------------
	protected string RefuseText(RZ_Wanderer w, int attitude)
	{
		if (attitude <= -9)
			return "He has a pistol out before you're close. \"I know that face from the briefings. Walk away. Now.\"";

		if (w.m_eRole == RZ_EWandererRole.DOCTOR && attitude == -4)
			return "\"I've seen what the Red Hand's friends do with medicine. Not you. Not from me.\"";

		if (attitude == -5)
			return "\"The Haven turned you out. I don't argue with the Haven.\" They back away, hands where you can see them.";

		if (w.m_eRole == RZ_EWandererRole.KID)
			return "The kid takes one look at you and runs.";

		return "\"I know what you did. Stay back.\"";
	}

	//------------------------------------------------------------------------------------------------
	protected string IntroText(RZ_Wanderer w)
	{
		switch (w.m_eRole)
		{
			case RZ_EWandererRole.TRADER: return "\"Prices are what they are. Nobody's making more of anything.\"";
			case RZ_EWandererRole.DOCTOR: return "\"Sit down, let me look at you. No? Then at least drink something.\"";
			case RZ_EWandererRole.SCAVENGER: return "\"I know every cellar from here to the coast. Most of them are empty now. Most.\"";
			case RZ_EWandererRole.DESERTER: return "\"I took the patch off. Doesn't mean I forgot how to shoot. Keep your hands where I can see them.\"";
			case RZ_EWandererRole.PREACHER: return "\"The bells are for them, not for us. Somebody has to tell them where to go.\"";
			case RZ_EWandererRole.COURIER: return "\"Letters, mostly. People still write to each other. Isn't that something?\"";
			case RZ_EWandererRole.KID: return "The kid holds on to the strap of a bag that's far too big. \"Mum said to wait at the safehouse. I don't know which one.\"";
			case RZ_EWandererRole.MECHANIC: return "Oil to the elbows, a spanner in a back pocket. \"Everything's broken. That's the good news, if you know how to fix things.\"";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! One piece of news from the road: noise, a hungry safehouse, an event, where they're going.
	protected string IntelText(RZ_Wanderer w, vector from)
	{
		array<string> lines = {};

		vector hot;
		float heat;
		if (m_GM.GetHeat().GetHottest(hot, heat) && heat >= 40 && RZ_Util.DistXZ(from, hot) <= 2500)
			lines.Insert(string.Format("\"Something's stirred them up, %1 from here. Don't go that way.\"", RZ_Util.DirectionText(from, hot)));

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
		{
			for (int i = 0; i < houses.Count(); i++)
			{
				RZ_Safehouse h = houses.Get(i);
				if (h.m_bShort)
				{
					lines.Insert(string.Format("\"The safehouse in %1 is out of food. They'd take anything you can carry.\"", h.m_sName));
					break;
				}
			}
		}

		foreach (RZ_Mission m : m_GM.GetMissions().GetActive())
		{
			if (m.m_bContract || m.m_bGlobal || RZ_Util.DistXZ(m.m_vPos, from) > 3000)
				continue;

			lines.Insert(string.Format("\"People on the road were talking about %1 near %2.\"", m.m_sTitle, m.GetLocationName(m_GM)));
			break;
		}

		if (!w.m_sDestName.IsEmpty())
			lines.Insert(string.Format("\"I'm heading for %1. If you're going that way, it's quiet enough.\"", w.m_sDestName));

		// Road news about the packs, always with the place (the Haven's words)
		RZ_Tribes tribes = m_GM.GetTribes();
		if (tribes && tribes.IsEnabled())
		{
			string packs = tribes.GetPeople().RoadNews(from);
			if (!packs.IsEmpty())
			{
				lines.Insert(packs);
				lines.Insert(packs);
			}
		}

		// A lost outpost is the talk of the road for an hour
		if (m_fClock < m_fRumourUntil && !m_sLossRumour.IsEmpty())
		{
			lines.Insert(m_sLossRumour);
			lines.Insert(m_sLossRumour);
		}

		// Road gossip about the factions (not always true) or a big moment people remember
		if (m_GM.GetFactionLife())
		{
			string gossip = m_GM.GetFactionLife().WandererLine();
			if (!gossip.IsEmpty())
				lines.Insert(gossip);
		}

		if (lines.IsEmpty())
			return "\"Quiet out here. I don't trust it.\"";

		return lines.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! For people they like: a warning.
	protected string WarningText(vector from)
	{
		array<vector> specials = {};
		m_GM.GetSpawner().GetSpecialsNear(from, 500, specials);
		if (!specials.IsEmpty())
			return string.Format("\"Careful. One of the changed ones, %1. Big. Keep your distance.\"", RZ_Util.DirectionText(from, specials[0]));

		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(from));
		if (s && s.m_fInfection >= 50)
			return string.Format("\"%1 is getting worse every day. I wouldn't sleep there.\"", s.m_sName);

		return "\"Stay off the main road after dark. That's all I know.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_ReadClueAction on the body ("Talk to ...").
	void HandleTalk(int playerId, int id, IEntity owner)
	{
		RZ_Wanderer w = Find(id);
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!w || !w.m_bAlive || !player)
			return;

		int attitude = Attitude(w, playerId);
		if (attitude <= -2)
		{
			m_GM.Say(playerId, w.GetTitle(), -1, RefuseText(w, attitude));
			Flee(w, player.GetOrigin());
			return;
		}

		// A barter chain (unarmed mission) or a story arc step that runs through this person
		if (RZ_Mission_BarterChain.OnWandererTalk(m_GM, playerId, w.m_iId, w.GetTitle(), player))
		{
			w.Remember(m_GM.GetProfile(playerId).m_sIdentity, MEM_HELPED);
			return;
		}

		string text = MoodLine(w) + IntroText(w) + "\n" + IntelText(w, player.GetOrigin());
		if (m_GM.GetTribes() && m_GM.GetTribes().GetPeople().IsOn())
			m_GM.GetTribes().GetPeople().OnRoadNews(playerId, player.GetOrigin());

		if (attitude >= 2)
			text += "\n" + WarningText(player.GetOrigin());

		// Faction gossip, mostly for players who are new to the Zone (new-player guidance)
		if (m_GM.GetGuidance() && w.m_eRole != RZ_EWandererRole.KID)
		{
			string gossip = m_GM.GetGuidance().WandererGossip(playerId, w.m_eRole, player.GetOrigin());
			if (!gossip.IsEmpty())
				text += "\n" + gossip;
		}

		// A story arc that runs through the roads: a rumour, a piece of a map
		if (m_GM.GetStoryArcs())
		{
			string arcLine = m_GM.GetStoryArcs().OnWandererTalk(playerId, w.m_iId);
			if (!arcLine.IsEmpty())
				text += "\n" + arcLine;
		}

		// A questline that runs through this person (Director/RZ_Questlines.c)
		if (m_GM.GetQuestlines())
		{
			string questLine = m_GM.GetQuestlines().OnWandererTalk(playerId, w.m_iId);
			if (!questLine.IsEmpty())
				text += "\n" + questLine;
		}

		// What the road says about the farm past the last village (World/RZ_LongTable.c)
		if (m_GM.GetLongTable() && w.m_eRole != RZ_EWandererRole.KID)
		{
			string tableLine = m_GM.GetLongTable().OnWandererTalk(playerId, w.m_iId, player.GetOrigin());
			if (!tableLine.IsEmpty())
				text += "\n" + tableLine;
		}

		// Dialogue > Trees: one time in four, after the road news, what the roads say about you
		if (RZ_RepVoice.TreesOn() && w.m_eRole != RZ_EWandererRole.KID && Math.RandomFloat01() < 0.25)
			text += "\n" + RZ_RepVoice.Picture(m_GM, playerId);

		if (w.IsBitten())
			text += "\nThere's a dressing on their arm, already dark through. \"Don't. I know.\"";

		m_GM.NotifyPlayer(playerId, w.GetTitle(), text);
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_PuzzleChoiceAction on the body. 0 special, 1 share food, 3 rob.
	void HandleChoice(int playerId, int id, int option, IEntity owner, IEntity user)
	{
		RZ_Wanderer w = Find(id);
		if (!w || !w.m_bAlive || !user)
			return;

		if (option == 3)
		{
			Rob(w, playerId, owner, user);
			return;
		}

		// Nobody turns down an antidote, whatever they think of you
		if (option == 2 && w.IsBitten())
		{
			GiveAntidote(w, playerId, user);
			return;
		}

		int attitude = Attitude(w, playerId);
		if (attitude <= -2)
		{
			m_GM.Say(playerId, w.GetTitle(), -1, RefuseText(w, attitude));
			Flee(w, user.GetOrigin());
			return;
		}

		if (option == 1)
		{
			ShareFood(w, playerId, user);
			return;
		}

		if (option == 2 && w.m_eRole == RZ_EWandererRole.MECHANIC)
		{
			AskAboutCars(w, playerId, user, attitude);
			return;
		}

		// "Need a hand?": the wanderers' asks (Missions/Errands, docs/ERRANDS.md)
		if (option == 2 && m_GM.GetErrands())
		{
			m_GM.GetErrands().AskWanderer(playerId, w, owner, user);
			return;
		}

		if (option != 0)
			return;

		switch (w.m_eRole)
		{
			case RZ_EWandererRole.TRADER: Trade(w, playerId, owner, user); break;
			case RZ_EWandererRole.DOCTOR: Treat(w, playerId, user); break;
			case RZ_EWandererRole.SCAVENGER: SellRumour(w, playerId, user, true); break;
			case RZ_EWandererRole.DESERTER: AskAboutBastion(w, playerId, user); break;
			case RZ_EWandererRole.PREACHER: StopBells(w, playerId); break;
			case RZ_EWandererRole.COURIER: OfferJob(w, playerId, user); break;
			case RZ_EWandererRole.KID: Escort(w, playerId, user); break;
			case RZ_EWandererRole.MECHANIC: SellPart(w, playerId, owner, user); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected int PartCost()
	{
		RZ_Interactions hub = RZ_Interactions.Get();
		if (!hub)
			return 12;

		return hub.GetCars().GetPartCost();
	}

	//------------------------------------------------------------------------------------------------
	//! The Mechanic: where the nearest fixable car is and what it needs; its keys if they like you.
	protected void AskAboutCars(RZ_Wanderer w, int playerId, IEntity user, int attitude)
	{
		RZ_Interactions hub = RZ_Interactions.Get();
		if (!hub)
		{
			m_GM.NotifyPlayer(playerId, w.GetTitle(), "\"Cars? Nothing out here runs any more.\"");
			return;
		}

		bool friendly = attitude >= 2 || w.Remembers(m_GM.GetProfile(playerId).m_sIdentity, MEM_HELPED);
		m_GM.NotifyPlayer(playerId, w.GetTitle(), hub.GetCars().MechanicTalk(playerId, user.GetOrigin(), friendly));
		if (attitude >= 0)
			OfferCarKeys(w, playerId, user);
	}

	//------------------------------------------------------------------------------------------------
	//! The Mechanic's job (RZ_Mission_CarKeys): a car that runs, its keys in one of five houses.
	//! One at a time, one per player per 30 min (shared with the Courier's letters). Says nothing
	//! if there's no job to give.
	protected void OfferCarKeys(RZ_Wanderer w, int playerId, IEntity user)
	{
		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		float last;
		RZ_MissionManager missions = m_GM.GetMissions();
		if ((m_mJobAt.Find(ident, last) && m_fClock - last < JOB_COOLDOWN) || missions.IsTypeRunning(RZ_Mission_CarKeys))
			return;

		RZ_Mission_CarKeys job = new RZ_Mission_CarKeys();
		job.Configure(m_GM);
		if (!job.IsEligible(m_GM, playerId) || !job.Prepare(m_GM, w.m_vPos, w.GetShortName(), playerId))
			return;

		// The same check as any mission (RZ_MissionManager.CanRunNow)
		if (!missions.StartIfCan(job, GetGame().GetPlayerManager().GetPlayerCount()))
			return;

		m_mJobAt.Set(ident, m_fClock);
		job.m_aOfferedTo.Insert(playerId);
		m_GM.NotifyPlayer(playerId, "Contract: " + RZ_Factions.GetName(RZ_EFaction.SURVIVORS),
			string.Format("%1\n%2\n(%3)", job.m_sTitle, job.GetBrief(m_GM), job.GetWhereTextFor(m_GM, user.GetOrigin(), playerId)));
	}

	//------------------------------------------------------------------------------------------------
	//! A part the nearest fixable car needs, for Haven Favor (a quarter off if they remember you).
	protected void SellPart(RZ_Wanderer w, int playerId, IEntity owner, IEntity user)
	{
		RZ_Interactions hub = RZ_Interactions.Get();
		ResourceName part;
		if (hub)
			part = hub.GetCars().PartFor(user.GetOrigin());

		if (part.IsEmpty())
		{
			m_GM.NotifyPlayer(playerId, w.GetTitle(), "\"Nothing in the bag today. Strip a wreck, it's what I do.\"");
			return;
		}

		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		int cost = PartCost();
		if (w.Remembers(ident, MEM_HELPED) || w.Remembers(ident, MEM_SAVED))
			cost = (cost * 3 + 3) / 4;

		if (!Pay(w, playerId, cost))
			return;

		if (!RZ_QuartermasterHelper.SpawnNear(part, owner, user))
		{
			m_GM.AddFavor(playerId, RZ_EFaction.SURVIVORS, cost); // refund
			return;
		}

		w.Remember(ident, MEM_HELPED);
		m_GM.NotifyPlayer(playerId, w.GetTitle(), string.Format("\"%1. Took it off a car that won't be needing it.\" %2 Haven Favor.", RZ_ItemHelper.DisplayName(part), cost));
	}

	//------------------------------------------------------------------------------------------------
	protected bool Pay(RZ_Wanderer w, int playerId, int cost)
	{
		if (m_GM.SpendFavor(playerId, RZ_EFaction.SURVIVORS, cost))
			return true;

		m_GM.NotifyPlayer(playerId, w.GetTitle(), string.Format("\"That's %1 Haven Favor. You've got %2. Come back when you have it.\"",
			cost, m_GM.GetProfile(playerId).GetFavor(RZ_EFaction.SURVIVORS)));
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void Trade(RZ_Wanderer w, int playerId, IEntity owner, IEntity user)
	{
		if (!m_Settings.m_aTraderGoods || m_Settings.m_aTraderGoods.IsEmpty())
		{
			m_GM.NotifyPlayer(playerId, w.GetTitle(), "\"Nothing you'd want today. Ask me again at the next safehouse.\"");
			return;
		}

		string ident = m_GM.GetProfile(playerId).m_sIdentity;

		// Jewellery (the Wedding Party's dead) pays instead of Favor
		RZ_EventSettings events = m_GM.GetEventSettings();
		if (events && !events.m_sJewelleryWords.IsEmpty() && RZ_ItemHelper.Has(user, events.m_sJewelleryWords))
		{
			if (!RZ_QuartermasterHelper.SpawnNear(m_Settings.m_aTraderGoods.GetRandomElement(), owner, user))
				return;

			RZ_ItemHelper.Take(user, events.m_sJewelleryWords, 1);
			w.Remember(ident, MEM_HELPED);
			m_GM.NotifyPlayer(playerId, w.GetTitle(), "They hold the jewellery up to the light, turn it twice, and don't ask where it came from. \"That'll do. Gold still means something to somebody.\"");
			return;
		}

		int cost = m_Settings.m_iTradeCost;
		if (w.Remembers(ident, MEM_HELPED) || w.Remembers(ident, MEM_SAVED))
			cost = (cost * 3 + 3) / 4;

		if (!Pay(w, playerId, cost))
			return;

		if (!RZ_QuartermasterHelper.SpawnNear(m_Settings.m_aTraderGoods.GetRandomElement(), owner, user))
		{
			m_GM.AddFavor(playerId, RZ_EFaction.SURVIVORS, cost); // refund
			return;
		}

		w.Remember(ident, MEM_HELPED);
		m_GM.NotifyPlayer(playerId, w.GetTitle(), string.Format("\"Fair's fair.\" %1 Haven Favor. They count it twice anyway.", cost));
	}

	//------------------------------------------------------------------------------------------------
	//! Infected: slows it (x1.25 of what's left), once per life. Otherwise patches you up.
	protected void Treat(RZ_Wanderer w, int playerId, IEntity user)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		if (m_GM.GetInfection().SeemsInfected(playerId) && profile.m_bDoctorTreated)
		{
			m_GM.NotifyPlayer(playerId, w.GetTitle(), "\"I've done what I can for that. I'm sorry. Find an antidote.\"");
			return;
		}

		if (!Pay(w, playerId, m_Settings.m_iTreatCost))
			return;

		w.Remember(profile.m_sIdentity, MEM_HELPED);
		// A bite cleaned out in time but not told yet: the doctor can't tell either, so it looks
		// the same as a bite that took (nothing changes; the golden window ends as it would)
		float pending = m_GM.GetInfection().CleanPendingLeft(playerId);
		if (pending > 0)
		{
			profile.m_bDoctorTreated = true;
			m_GM.NotifyPlayer(playerId, w.GetTitle(), string.Format("They clean the bite, pack it with something that smells of cloves, and don't lie to you. \"Slower now. About %1 minutes. Not a cure.\"",
				RZ_Util.RoundToInt(pending * 1.25 / 60)));
			return;
		}

		if (profile.IsInfected())
		{
			profile.m_bDoctorTreated = true;
			profile.m_fInfectionLeft = profile.m_fInfectionLeft * 1.25;
			m_GM.NotifyPlayer(playerId, w.GetTitle(), string.Format("They clean the bite, pack it with something that smells of cloves, and don't lie to you. \"Slower now. About %1 minutes. Not a cure.\"",
				RZ_Util.RoundToInt(profile.m_fInfectionLeft / 60)));
			return;
		}

		RZ_Util.HealCharacter(user);
		m_GM.NotifyPlayer(playerId, w.GetTitle(), "\"Hold still.\" A few minutes, a few stitches. \"There. Try not to need me again.\"");
	}

	//------------------------------------------------------------------------------------------------
	//! A rumour that points at a puzzle in progress, or at a stash. Paid in Favor, or in food.
	protected void SellRumour(RZ_Wanderer w, int playerId, IEntity user, bool pay)
	{
		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		float last;
		if (m_mRumourAt.Find(ident, last) && m_fClock - last < JOB_COOLDOWN)
		{
			m_GM.NotifyPlayer(playerId, w.GetTitle(), "\"I've told you all I know. Give it a while.\"");
			return;
		}

		if (pay && !Pay(w, playerId, m_Settings.m_iRumourCost))
			return;

		m_mRumourAt.Set(ident, m_fClock);
		w.Remember(ident, MEM_HELPED);

		foreach (RZ_Mission m : m_GM.GetMissions().GetActive())
		{
			if (!RZ_PuzzleMission.Cast(m) || m.m_bGlobal)
				continue;

			m_GM.NotifyPlayer(playerId, w.GetTitle(), string.Format("\"Something odd near %1. Notes, people asking questions, things that don't add up. Worth a look.\" (%2)",
				m.GetLocationName(m_GM), m.GetWhereTextFor(m_GM, user.GetOrigin(), playerId)));
			return;
		}

		int idx = RZ_Mission.PickSettlement(m_GM, 0, 70);
		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (!s)
		{
			m_GM.NotifyPlayer(playerId, w.GetTitle(), "\"Funny. I had something, and now I can't remember it. Keep your Favor next time.\"");
			return;
		}

		vector stash = RZ_Mission.LandPointNear(s.m_vCenter, s.m_fRadius * 0.5, s.m_fRadius);
		m_GM.SpawnRewardCrate(stash);
		m_GM.NotifyPlayer(playerId, w.GetTitle(), string.Format("\"A family in %1 buried their good things before they left. Grid %2. Don't tell anyone where you heard it.\"",
			s.m_sName, RZ_SettlementScanner.GridReference(stash)));
	}

	//------------------------------------------------------------------------------------------------
	protected void AskAboutBastion(RZ_Wanderer w, int playerId, IEntity user)
	{
		if (RZ_Reputation.IsHunted(m_GM.GetTier(playerId, RZ_EFaction.MILITARY)))
		{
			m_GM.NotifyPlayer(playerId, w.GetTitle(), "\"You're on a list. I've seen the list. Stay off the main roads and never sleep in the same place twice.\"");
			return;
		}

		// Struck Off: he knows where the man on the roster sleeps
		string tip = RZ_Mission_StruckOff.DeserterTip(m_GM);
		if (!tip.IsEmpty())
		{
			m_GM.NotifyPlayer(playerId, w.GetTitle(), tip);
			return;
		}

		foreach (RZ_Mission m : m_GM.GetMissions().GetActive())
		{
			if (!m.m_bContract || m.m_Req.m_eGiver != RZ_EFaction.MILITARY || m.m_bGlobal)
				continue;

			m_GM.NotifyPlayer(playerId, w.GetTitle(), string.Format("\"Bastion's got people busy near %1: %2. Get paid by them or stay out of their way. Your choice. I made mine.\"",
				m.GetLocationName(m_GM), m.m_sTitle));
			return;
		}

		m_GM.NotifyPlayer(playerId, w.GetTitle(), "\"Bastion's quiet. That's never good. They're either out of fuel or planning something.\"");
	}

	//------------------------------------------------------------------------------------------------
	protected void StopBells(RZ_Wanderer w, int playerId)
	{
		w.m_fBellsOffUntil = m_fClock + JOB_COOLDOWN;
		m_GM.NotifyPlayer(playerId, w.GetTitle(), "He looks at you a long time. \"For you, a little while. They'll find their own way for an hour.\"");
		if (m_GM.GetStoryArcs())
			m_GM.GetStoryArcs().OnPreacherStopped(w.m_iId, w.m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	//! An unarmed delivery job (RZ_Mission_CourierJob): one at a time, one per player per 30 min.
	protected void OfferJob(RZ_Wanderer w, int playerId, IEntity user)
	{
		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		float last;
		RZ_MissionManager missions = m_GM.GetMissions();
		if ((m_mJobAt.Find(ident, last) && m_fClock - last < JOB_COOLDOWN) || missions.IsTypeRunning(RZ_Mission_CourierJob))
		{
			m_GM.NotifyPlayer(playerId, w.GetTitle(), "\"Nothing right now. Someone's already carrying the last of it.\"");
			return;
		}

		RZ_Mission_CourierJob job = new RZ_Mission_CourierJob();
		job.Configure(m_GM);
		if (!job.IsEligible(m_GM, playerId) || !job.Prepare(m_GM, w.m_vPos, w.GetShortName(), playerId))
		{
			m_GM.NotifyPlayer(playerId, w.GetTitle(), "\"I've letters, but nowhere they can go today. Ask me tomorrow.\"");
			return;
		}

		// The same check as any mission (RZ_MissionManager.CanRunNow)
		if (!missions.StartIfCan(job, GetGame().GetPlayerManager().GetPlayerCount()))
		{
			m_GM.NotifyPlayer(playerId, w.GetTitle(), "\"I've letters, but nowhere they can go today. Ask me tomorrow.\"");
			return;
		}

		m_mJobAt.Set(ident, m_fClock);
		w.Remember(ident, MEM_HELPED);
		job.m_aOfferedTo.Insert(playerId);
		m_GM.NotifyPlayer(playerId, "Contract: " + RZ_Factions.GetName(RZ_EFaction.SURVIVORS),
			string.Format("%1\n%2\n(%3)", job.m_sTitle, job.GetBrief(m_GM), job.GetWhereTextFor(m_GM, user.GetOrigin(), playerId)));
	}

	//------------------------------------------------------------------------------------------------
	//! The Kid follows you to the nearest safehouse while you stay close.
	protected void Escort(RZ_Wanderer w, int playerId, IEntity user)
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		int idx = -1;
		if (houses)
			idx = houses.FindNearest(w.m_vPos);

		RZ_Safehouse h;
		if (houses)
			h = houses.Get(idx);

		if (!h)
		{
			m_GM.NotifyPlayer(playerId, w.GetTitle(), "There's nowhere safe to take them. The kid sits down by the road to wait for someone who knows.");
			return;
		}

		w.m_iEscortPlayer = playerId;
		w.m_vDest = h.m_vPos;
		w.m_sDestName = "the safehouse in " + h.m_sName;
		w.m_iDestSafehouse = idx;
		w.m_eState = RZ_EWandererState.TRAVEL;
		w.Remember(m_GM.GetProfile(playerId).m_sIdentity, MEM_HELPED);
		if (w.m_Body)
		{
			if (SCR_AIGroup.Cast(w.m_Body))
				RZ_Util.RedirectGroup(w.m_Body, w.m_vDest, m_GM.GetSpawner().GetMoveWaypointPrefab());
			else
				DespawnBody(w);
		}

		m_GM.NotifyPlayer(playerId, w.GetTitle(), string.Format("The kid nods and starts walking toward %1 (%2). Stay within 150 m and they'll keep going. No shooting.",
			w.m_sDestName, RZ_Util.DirectionText(user.GetOrigin(), h.m_vPos)));
	}

	//------------------------------------------------------------------------------------------------
	//! Barter: food for goodwill and news. The Scavenger swaps a rumour for it.
	protected void ShareFood(RZ_Wanderer w, int playerId, IEntity user)
	{
		array<string> keywords = {};
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
			keywords = houses.GetFoodKeywords();
		else
			RZ_FoodHelper.DefaultKeywords(keywords);

		if (RZ_FoodHelper.TakeFood(user, keywords, 1) <= 0)
		{
			m_GM.NotifyPlayer(playerId, w.GetTitle(), "You've nothing to eat to share.");
			return;
		}

		w.Remember(m_GM.GetProfile(playerId).m_sIdentity, MEM_HELPED);
		if (w.m_eRole == RZ_EWandererRole.SCAVENGER)
		{
			SellRumour(w, playerId, user, false);
			return;
		}

		m_GM.NotifyPlayer(playerId, w.GetTitle(), "They eat it standing up, fast, like someone might take it. \"Thank you.\" Then, quieter:\n" + WarningText(user.GetOrigin()));
	}

	//------------------------------------------------------------------------------------------------
	protected void Rob(RZ_Wanderer w, int playerId, IEntity owner, IEntity user)
	{
		w.Remember(m_GM.GetProfile(playerId).m_sIdentity, MEM_WRONGED);
		m_GM.AddInfamy(playerId, RZ_EFaction.SURVIVORS, m_Settings.m_iRobInfamy);

		string text = "They empty their pockets with shaking hands. There's almost nothing in them.";
		if (w.m_eRole == RZ_EWandererRole.TRADER && m_Settings.m_aTraderGoods && !m_Settings.m_aTraderGoods.IsEmpty())
		{
			RZ_QuartermasterHelper.SpawnNear(m_Settings.m_aTraderGoods.GetRandomElement(), owner, user);
			text = "The trader drops the pack and backs away. \"Take it. Take it and remember my face.\"";
		}

		m_GM.NotifyPlayer(playerId, w.GetTitle(), string.Format("%1\nThe safehouses will hear about this. (+%2 Haven Infamy)", text, m_Settings.m_iRobInfamy));
		Flee(w, user.GetOrigin());
	}
}
