//! Named faction leaders (docs/REPUTATION.md, "Faction leaders"; docs/DESIGN.md, "Leaders").
//!
//! One leader per place a faction holds: a stronghold leader per faction (at its
//! quartermaster; Halcyon's is at the lab), a council head per Haven safehouse, and a
//! commander, crew boss or lead researcher per outpost. With territory on (Factions/RZ_Territory.c),
//! the territory registers each real outpost through RegisterOutpost() and unregisters it when
//! it's lost or abandoned. With territory off, each faction but the Haven gets
//! RZ_LeaderSettings.m_iAutoOutposts interim "leader posts" in a town that suits it instead.
//! Mission makers can place RZ_OutpostMarkerEntity entities either way.
//!
//! Leaders are data. Only the one at the place a player is standing near has a body, and
//! at a stronghold or safehouse that body is the quartermaster unless a leader prefab is
//! set (RZ_LeaderAction on the quartermaster talks to them). Each has a seeded name, a
//! title, a backstory, two traits with small capped effects, a loyalty (Loyal,
//! Independent, Rogue; stronghold leaders are always Loyal and never die), and a memory
//! of each player (helped, crossed, saved, met). Independent and Rogue leaders give
//! personal jobs off the stronghold's books (Missions/Leaders/).

enum RZ_ELeaderRole
{
	STRONGHOLD,
	OUTPOST,
	SAFEHOUSE
}

enum RZ_ELoyalty
{
	LOYAL,			// the stronghold's line; faction standing applies fully
	INDEPENDENT,	// own agenda; their own memory of you counts for more; off-book jobs
	ROGUE			// works around or against the stronghold; may help people it hates
}

enum RZ_ELeaderTrait
{
	GREEDY,		// pays more Favor on their contracts, charges more in their shop
	PARANOID,	// Fame from their contracts counts less; faction Infamy hits harder (stronghold)
	MERCIFUL,	// amends work off more Infamy; forgives faster
	RUTHLESS,	// darker contracts more often
	IDEALIST,	// hopeful work more often; hates sample-selling
	PRAGMATIST,	// deals with rivals: cross-faction and choice work more often
	VETERAN,	// harder, better-paid contracts; respects Soldier and Police backgrounds
	TALKER		// more rumours and cross-faction hints; their secrets leak more easily
}

//! What an RZ_LeaderAction (on a quartermaster) does.
enum RZ_ELeaderAction
{
	TALK,
	WORK,
	NEWS,
	STANDING,
	// The epilogue week (docs/EPILOGUE.md): the season topics and a stronghold's grey choice
	SEASON,
	LOOK,
	ANSWER_FIRST,
	ANSWER_OTHER
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Leaders.
[BaseContainerProps()]
class RZ_LeaderSettings
{
	[Attribute("1", desc: "Named faction leaders on")]
	bool m_bEnabled;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Haven stronghold leader body (optional). Empty = the Haven quartermaster is the leader", "et")]
	ResourceName m_sHavenLeaderPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bastion stronghold leader body (optional). Empty = the Bastion quartermaster is the leader", "et")]
	ResourceName m_sBastionLeaderPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Red Hand stronghold leader body (optional). Empty = the Red Hand quartermaster is the leader", "et")]
	ResourceName m_sRedHandLeaderPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Halcyon research director body (optional). Empty = the Halcyon quartermaster is the leader", "et")]
	ResourceName m_sHalcyonLeaderPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Haven safehouse council head body (optional). Empty = the safehouse quartermaster is the leader", "et")]
	ResourceName m_sCouncilHeadPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bastion outpost commander body (optional; falls back to the Bastion leader, then the puzzle civilian)", "et")]
	ResourceName m_sBastionOutpostPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Red Hand crew boss body (optional; falls back to the Red Hand leader, then the puzzle civilian)", "et")]
	ResourceName m_sRedHandOutpostPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Halcyon field lead body (optional; falls back to the Halcyon leader, then the puzzle civilian)", "et")]
	ResourceName m_sHalcyonOutpostPrefab;

	[Attribute("1", desc: "Without a leader prefab, the quartermaster at a stronghold or safehouse is the leader (give it RZ_LeaderAction). Off = a puzzle civilian stands in")]
	bool m_bQuartermasterIsLeader;

	[Attribute("1", UIWidgets.Slider, "Interim leader posts per faction (Bastion, Red Hand; Halcyon at most 1), only while Territory is off. Placing RZ_OutpostMarkerEntity for a faction turns this off for it", "0 3 1")]
	int m_iAutoOutposts;

	[Attribute("200", desc: "A leader's body exists only while a player is this close (m)")]
	float m_fBodyRadius;

	[Attribute("120", desc: "Real minutes before a dead or dismissed outpost or safehouse leader is replaced")]
	float m_fReplaceMinutes;

	[Attribute("30", desc: "Real minutes between personal jobs from one leader")]
	float m_fJobMinutes;

	[Attribute("1", desc: "Scales the chance of outpost and safehouse leaders dying off-screen (bites, raids). 0 = they only die if killed")]
	float m_fDangerScale;

	[Attribute("10", desc: "Infamy with the leader's faction when a rogue job is discovered")]
	int m_iDiscoveryInfamy;

	[Attribute("0.15", desc: "Base chance a finished rogue job is discovered")]
	float m_fDiscoveryChance;

	[Attribute("0.35", desc: "Extra discovery chance if the faction's own people saw the job")]
	float m_fSeenChance;

	[Attribute("15", desc: "Infamy with the faction for killing one of its leaders")]
	int m_iKillInfamy;
}

//------------------------------------------------------------------------------------------------
class RZ_LeaderSave
{
	int id;
	int generation;
	int faction;
	int role;
	int source;
	bool alive = true;
	bool active = true;
	float diedAt;
	string name;
	string shortName;
	string title;
	string backstory;
	int traitA;
	int traitB;
	int loyalty;
	int score;
	float x;
	float z;
	string place;
	int safehouse = -1;
	float nextJobAt;
	ref array<string> memIdent = {};
	ref array<int> memFlags = {};
	ref array<int> memLife = {};		// the life each entry's flags are from
	ref array<int> memOld = {};			// what's remembered of earlier lives
}

//------------------------------------------------------------------------------------------------
class RZ_LeaderRosterSave
{
	int seed;
	int nextId = 1;
	float clock;
	ref array<ref RZ_LeaderSave> list = {};
	ref array<string> headlines = {};
}

//------------------------------------------------------------------------------------------------
//! One leader. Data first; a body exists only near players.
class RZ_Leader
{
	static const int SOURCE_AUTO = 0;		// stronghold, safehouse or interim post picked by the mode
	static const int SOURCE_MARKER = 1;		// RZ_OutpostMarkerEntity
	static const int SOURCE_EXTERNAL = 2;	// RegisterOutpost() (territory)

	int m_iId;
	int m_iGeneration;
	RZ_EFaction m_eFaction;
	RZ_ELeaderRole m_eRole;
	int m_iSource;
	bool m_bAlive = true;
	bool m_bActive = true;		// the seat exists (an external outpost waits for RegisterOutpost)
	float m_fDiedAt;
	string m_sName;
	string m_sShort;
	string m_sTitle;
	string m_sBackstory;
	int m_iTraitA;
	int m_iTraitB;
	RZ_ELoyalty m_eLoyalty = RZ_ELoyalty.LOYAL;
	int m_iScore = 100;			// loyalty 0..100: Loyal >= 60, Rogue < 25
	vector m_vPos;
	string m_sPlace;
	int m_iSafehouse = -1;
	float m_fNextJobAt;
	ref array<string> m_aMemIdent = {};
	ref array<int> m_aMemFlags = {};
	ref array<int> m_aMemLife = {};
	ref array<int> m_aMemOld = {};

	// Runtime only
	IEntity m_Body;
	bool m_bHasBody;
	IEntity m_TaggedQm;
	int m_iHeldBy = -1;
	int m_iLastShooter = -1;
	float m_fLastShotAt = -1000;
	int m_iKilledBy = -1;		// the player blamed for their death (Kill the Officer)
	bool m_bWasShort;
	float m_fDriftWindow;
	int m_iDriftSum;
	float m_fNextReinAt;
	float m_fNextJobRoll;

	//------------------------------------------------------------------------------------------------
	bool HasTrait(int trait)
	{
		return m_iTraitA == trait || m_iTraitB == trait;
	}

	//------------------------------------------------------------------------------------------------
	bool IsStronghold()
	{
		return m_eRole == RZ_ELeaderRole.STRONGHOLD;
	}

	//------------------------------------------------------------------------------------------------
	bool IsPresent()
	{
		return m_bAlive && m_bActive;
	}

	//------------------------------------------------------------------------------------------------
	//! "Major Weber, outpost commander in Morton"
	string GetHeader()
	{
		return m_sName + ", " + m_sTitle;
	}

	//------------------------------------------------------------------------------------------------
	string TraitText()
	{
		return RZ_LeaderVoice.TraitName(m_iTraitA) + ", " + RZ_LeaderVoice.TraitName(m_iTraitB);
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

		return old & ~RZ_Leaders.MEM_MET;
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
	RZ_LeaderSave ToSave()
	{
		RZ_LeaderSave s = new RZ_LeaderSave();
		s.id = m_iId;
		s.generation = m_iGeneration;
		s.faction = m_eFaction;
		s.role = m_eRole;
		s.source = m_iSource;
		s.alive = m_bAlive;
		s.active = m_bActive;
		s.diedAt = m_fDiedAt;
		s.name = m_sName;
		s.shortName = m_sShort;
		s.title = m_sTitle;
		s.backstory = m_sBackstory;
		s.traitA = m_iTraitA;
		s.traitB = m_iTraitB;
		s.loyalty = m_eLoyalty;
		s.score = m_iScore;
		s.x = m_vPos[0];
		s.z = m_vPos[2];
		s.place = m_sPlace;
		s.safehouse = m_iSafehouse;
		s.nextJobAt = m_fNextJobAt;
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
	static RZ_Leader FromSave(RZ_LeaderSave s)
	{
		RZ_Leader l = new RZ_Leader();
		l.m_iId = s.id;
		l.m_iGeneration = s.generation;
		l.m_eFaction = s.faction;
		l.m_eRole = s.role;
		l.m_iSource = s.source;
		l.m_bAlive = s.alive;
		l.m_bActive = s.active;
		l.m_fDiedAt = s.diedAt;
		l.m_sName = s.name;
		l.m_sShort = s.shortName;
		l.m_sTitle = s.title;
		l.m_sBackstory = s.backstory;
		l.m_iTraitA = s.traitA;
		l.m_iTraitB = s.traitB;
		l.m_eLoyalty = s.loyalty;
		l.m_iScore = Math.ClampInt(s.score, 0, 100);
		l.m_vPos = RZ_Util.GroundPos(Vector(s.x, 0, s.z));
		l.m_sPlace = s.place;
		l.m_iSafehouse = s.safehouse;
		l.m_fNextJobAt = s.nextJobAt;
		if (s.memIdent && s.memFlags)
		{
			for (int i = 0; i < s.memIdent.Count() && i < s.memFlags.Count(); i++)
			{
				l.m_aMemIdent.Insert(s.memIdent[i]);
				l.m_aMemFlags.Insert(s.memFlags[i]);
				// An old save's memory loads as an earlier life
				int life = -1;
				int old = 0;
				if (s.memLife && i < s.memLife.Count())
					life = s.memLife[i];
				if (s.memOld && i < s.memOld.Count())
					old = s.memOld[i];

				l.m_aMemLife.Insert(life);
				l.m_aMemOld.Insert(old);
			}
		}

		if (l.IsStronghold())
		{
			l.m_eLoyalty = RZ_ELoyalty.LOYAL;
			l.m_iScore = 100;
		}

		return l;
	}
}

//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "ReforgerZ", description: "A faction outpost with a leader, placed by hand. Placing any for a faction turns its interim leader posts off.")]
class RZ_OutpostMarkerEntityClass : GenericEntityClass
{
}

//! Optional. Where a faction keeps an outpost (and so a named leader). Without any, each
//! faction but the Haven gets RZ_LeaderSettings.m_iAutoOutposts interim posts.
class RZ_OutpostMarkerEntity : GenericEntity
{
	[Attribute("1", UIWidgets.ComboBox, "Faction holding this outpost", "", ParamEnumArray.FromEnum(RZ_EFaction))]
	protected RZ_EFaction m_eFaction;

	[Attribute("", desc: "Place name (empty = the nearest settlement's name)")]
	protected string m_sName;

	protected static ref array<RZ_OutpostMarkerEntity> s_aAll;

	//------------------------------------------------------------------------------------------------
	static array<RZ_OutpostMarkerEntity> GetAll()
	{
		if (!s_aAll)
			s_aAll = new array<RZ_OutpostMarkerEntity>();

		return s_aAll;
	}

	RZ_EFaction GetFaction() { return m_eFaction; }
	string GetPlaceName() { return m_sName; }

	//------------------------------------------------------------------------------------------------
	void RZ_OutpostMarkerEntity(IEntitySource src, IEntity parent)
	{
		GetAll().Insert(this);
	}

	//------------------------------------------------------------------------------------------------
	void ~RZ_OutpostMarkerEntity()
	{
		if (s_aAll)
			s_aAll.RemoveItem(this);
	}
}

//------------------------------------------------------------------------------------------------
//! All leaders. Owned by the game mode, ticked on the server tick.
class RZ_Leaders
{
	static const int TAG = -4;				// RZ_PuzzleObjectComponent / RZ_ClueComponent mission id
	static const int TRAIT_COUNT = 8;

	static const int MEM_HELPED = 1;
	static const int MEM_CROSSED = 2;
	static const int MEM_SAVED = 4;
	static const int MEM_MET = 8;

	//! Job tones (RZ_Mission.m_iJobTone) that traits lean toward.
	static const int TONE_DARK = 1;
	static const int TONE_HOPEFUL = 2;
	static const int TONE_CROSS = 4;
	static const int TONE_HARD = 8;
	static const int TONE_TALK = 16;

	static const int LOYAL_AT = 60;
	static const int ROGUE_BELOW = 25;

	protected static const float SEAT_RADIUS = 60;
	protected static const float DESPAWN_MARGIN = 50;
	protected static const float GREET_RADIUS = 15;
	protected static const float GREET_COOLDOWN = 600;
	protected static const float DANGER_INTERVAL = 1800;
	protected static const float DRIFT_INTERVAL = 3600;
	protected static const float DRIFT_WINDOW = 3600;
	protected static const int DRIFT_CAP = 15;
	protected static const float JOB_ROLL_INTERVAL = 120;
	protected static const float JOB_ROLL_CHANCE = 0.35;
	protected static const float REIN_INTERVAL = 3600;
	protected static const float SEEN_RADIUS = 60;
	protected static const float BLAME_SECONDS = 30;
	protected static const int HEADLINES_KEPT = 10;
	protected static const float MULT_MIN = 0.8;
	protected static const float MULT_MAX = 1.2;

	protected RZ_GameModeComponent m_GM;
	protected RZ_LeaderSettings m_Settings;
	protected ref array<ref RZ_Leader> m_aList = {};
	protected ref RZ_LeaderRosterSave m_Restored;
	protected ref array<typename> m_aJobTypes = {};
	protected ref array<string> m_aHeadlines = {};
	protected ref map<string, float> m_mGreeted = new map<string, float>();
	protected int m_iSeed;
	protected int m_iNextId = 1;
	protected float m_fClock;
	protected float m_fDangerTimer;
	protected float m_fDriftTimer;
	protected float m_fJobTimer;
	protected bool m_bTerritoryOutposts;	// territory registers the outposts: no interim posts

	//------------------------------------------------------------------------------------------------
	void RZ_Leaders(RZ_GameModeComponent gm, RZ_LeaderSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		RZ_LeaderJobRegistry.RegisterAll(m_aJobTypes);
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled() { return m_Settings.m_bEnabled; }
	float GetClock() { return m_fClock; }
	RZ_LeaderSettings GetSettings() { return m_Settings; }

	//------------------------------------------------------------------------------------------------
	//! Before Setup: territory owns the outposts, so no interim posts are made. Saved interim
	//! posts wait, inactive, for the territory to adopt them (GetDormantOutposts).
	void SetTerritoryOutposts(bool on)
	{
		m_bTerritoryOutposts = on;
	}

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	void Restore(RZ_LeaderRosterSave saved)
	{
		m_Restored = saved;
	}

	//------------------------------------------------------------------------------------------------
	RZ_LeaderRosterSave Save()
	{
		RZ_LeaderRosterSave data = new RZ_LeaderRosterSave();
		data.seed = m_iSeed;
		data.nextId = m_iNextId;
		data.clock = m_fClock;
		foreach (RZ_Leader l : m_aList)
		{
			data.list.Insert(l.ToSave());
		}

		foreach (string h : m_aHeadlines)
		{
			data.headlines.Insert(h);
		}

		return data;
	}

	// ================================================================ Setup
	//------------------------------------------------------------------------------------------------
	//! After quartermasters, the lab and safehouses exist. Saved leaders are matched to the
	//! seats that exist now; new seats get a new leader.
	void Setup()
	{
		if (!m_Settings.m_bEnabled || m_GM.GetSettlements().IsEmpty())
		{
			RZ_Log.Info("Faction leaders are off.");
			return;
		}

		array<ref RZ_Leader> saved = {};
		if (m_Restored && m_Restored.list)
		{
			m_iSeed = m_Restored.seed;
			m_iNextId = Math.Max(1, m_Restored.nextId);
			m_fClock = m_Restored.clock;
			foreach (RZ_LeaderSave s : m_Restored.list)
			{
				saved.Insert(RZ_Leader.FromSave(s));
			}

			if (m_Restored.headlines)
			{
				foreach (string line : m_Restored.headlines)
				{
					m_aHeadlines.Insert(line);
				}
			}
		}

		bool fresh = saved.IsEmpty();
		if (fresh)
			m_iSeed = Math.RandomInt(1, 2000000000);

		m_Restored = null;

		// Strongholds
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			vector pos;
			string place;
			if (StrongholdSeat(f, pos, place))
				AddSeat(saved, f, RZ_ELeaderRole.STRONGHOLD, RZ_Leader.SOURCE_AUTO, pos, place, -1);
		}

		// One council head per Haven safehouse (the one at the Haven stronghold is the stronghold)
		RZ_Safehouses houses = m_GM.GetSafehouses();
		RZ_Leader havenTop = GetStronghold(RZ_EFaction.SURVIVORS);
		if (houses)
		{
			for (int i = 0; i < houses.Count(); i++)
			{
				RZ_Safehouse h = houses.Get(i);
				if (havenTop && RZ_Util.DistXZ(h.m_vPos, havenTop.m_vPos) < 80)
					continue;

				AddSeat(saved, RZ_EFaction.SURVIVORS, RZ_ELeaderRole.SAFEHOUSE, RZ_Leader.SOURCE_AUTO, h.m_vPos, h.m_sName, i);
			}
		}

		SetupOutposts(saved);

		// Outposts registered by territory come back when it registers them again
		foreach (RZ_Leader old : saved)
		{
			if (old.m_iSource != RZ_Leader.SOURCE_EXTERNAL)
				continue;

			old.m_bActive = false;
			m_aList.Insert(old);
		}

		string source = "saved";
		if (fresh)
			source = string.Format("new, seed %1", m_iSeed);

		RZ_Log.Info(string.Format("Faction leaders: %1 (%2). %3", CountPresent(), source, DescribeRoster()));
	}

	//------------------------------------------------------------------------------------------------
	//! Where a faction's stronghold leader sits: its quartermaster; Halcyon at the lab; else a
	//! town that suits the faction (the leader is then data only, with a civilian stand-in).
	protected bool StrongholdSeat(RZ_EFaction f, out vector pos, out string place)
	{
		bool found = m_GM.GetPerks().GetQuartermasterPos(f, pos);
		if (!found && f == RZ_EFaction.INSTITUTE && m_GM.HasLab())
		{
			pos = m_GM.GetLabPosition();
			found = true;
		}

		if (!found && f == RZ_EFaction.SURVIVORS && m_GM.GetSafehouses() && m_GM.GetSafehouses().Count() > 0)
		{
			pos = m_GM.GetSafehouses().Get(0).m_vPos;
			found = true;
		}

		if (!found)
		{
			int idx = PickTownFor(f, vector.Zero, 0);
			RZ_Settlement s = m_GM.GetSettlement(idx);
			if (!s)
				return false;

			pos = RZ_Mission.LandPointNear(s.m_vCenter, 5, Math.Max(15, s.m_fRadius * 0.3));
		}

		place = PlaceName(pos);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Placed markers, else interim posts in towns that suit the faction (not the Haven:
	//! its safehouses are its outposts).
	protected void SetupOutposts(array<ref RZ_Leader> saved)
	{
		array<RZ_OutpostMarkerEntity> markers = RZ_OutpostMarkerEntity.GetAll();
		array<bool> marked = {false, false, false, false};
		foreach (RZ_OutpostMarkerEntity marker : markers)
		{
			if (!marker || !RZ_Factions.IsValid(marker.GetFaction()))
				continue;

			marked[marker.GetFaction()] = true;
			vector mpos = RZ_Util.GroundPos(marker.GetOrigin());
			string mname = marker.GetPlaceName();
			if (mname.IsEmpty())
				mname = PlaceName(mpos);

			AddSeat(saved, marker.GetFaction(), RZ_ELeaderRole.OUTPOST, RZ_Leader.SOURCE_MARKER, mpos, mname, -1);
		}

		// Territory registers real outposts instead. Saved interim posts become dormant
		// external seats (kept below), so their leaders keep their places when adopted.
		if (m_bTerritoryOutposts)
		{
			foreach (RZ_Leader interim : saved)
			{
				if (interim.m_eRole == RZ_ELeaderRole.OUTPOST && interim.m_iSource == RZ_Leader.SOURCE_AUTO)
					interim.m_iSource = RZ_Leader.SOURCE_EXTERNAL;
			}

			return;
		}

		array<int> autoFactions = {RZ_EFaction.MILITARY, RZ_EFaction.BANDITS, RZ_EFaction.INSTITUTE};
		foreach (int f : autoFactions)
		{
			if (marked[f])
				continue;

			int wanted = Math.ClampInt(m_Settings.m_iAutoOutposts, 0, 3);
			if (f == RZ_EFaction.INSTITUTE)
				wanted = Math.Min(wanted, 1);

			// Saved interim posts keep their places (collected first: AddSeat takes them out of `saved`)
			array<vector> keepPos = {};
			array<string> keepPlace = {};
			foreach (RZ_Leader old : saved)
			{
				if (keepPos.Count() >= wanted)
					break;

				if (old.m_eFaction == f && old.m_eRole == RZ_ELeaderRole.OUTPOST && old.m_iSource == RZ_Leader.SOURCE_AUTO)
				{
					keepPos.Insert(old.m_vPos);
					keepPlace.Insert(old.m_sPlace);
				}
			}

			foreach (int k, vector kept : keepPos)
			{
				AddSeat(saved, f, RZ_ELeaderRole.OUTPOST, RZ_Leader.SOURCE_AUTO, kept, keepPlace[k], -1);
			}

			int have = keepPos.Count();

			RZ_Leader top = GetStronghold(f);
			vector from = vector.Zero;
			if (top)
				from = top.m_vPos;

			for (int n = have; n < wanted; n++)
			{
				int idx = PickTownFor(f, from, 900);
				RZ_Settlement s = m_GM.GetSettlement(idx);
				if (!s)
					break;

				vector opos = RZ_Mission.LandPointNear(s.m_vCenter, 5, Math.Max(15, s.m_fRadius * 0.4));
				AddSeat(saved, f, RZ_ELeaderRole.OUTPOST, RZ_Leader.SOURCE_AUTO, opos, s.m_sName, -1);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A town that suits the faction (Bastion bigger towns, Red Hand rougher ones, Halcyon quiet
	//! villages), at least minDist from `from`, not already a seat, a safehouse town, the lab's
	//! town, a placed quartermaster's town (a stronghold seat maybe not seated yet) or the line.
	protected int PickTownFor(RZ_EFaction f, vector from, float minDist)
	{
		int minSize = 1;
		float minInf = 0;
		float maxInf = 70;
		if (f == RZ_EFaction.MILITARY)
			minSize = 2;

		if (f == RZ_EFaction.BANDITS)
		{
			minInf = 15;
			maxInf = 85;
		}

		for (int pass = 0; pass < 2; pass++)
		{
			array<int> options = {};
			foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
			{
				if (pass == 0 && (s.m_iSize < minSize || s.m_fInfection < minInf || s.m_fInfection > maxInf))
					continue;

				if (minDist > 0 && RZ_Util.DistXZ(from, s.m_vCenter) < minDist)
					continue;

				if (IsSeatNear(s.m_vCenter, Math.Max(300, s.m_fRadius)) || m_GM.GetPerimeter().IsNearLine(s.m_vCenter))
					continue;

				if (m_GM.GetSafehouses() && m_GM.GetSafehouses().IsSafehouseTown(i))
					continue;

				if (i == m_GM.GetLabSettlement() || IsQuartermasterTown(i))
					continue;

				options.Insert(i);
			}

			if (!options.IsEmpty())
				return options.GetRandomElement();
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! A placed quartermaster stands in this settlement: some faction's stronghold seat.
	protected bool IsQuartermasterTown(int idx)
	{
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			vector qm;
			if (m_GM.GetPerks().GetQuartermasterPos(f, qm) && m_GM.FindNearestSettlement(qm) == idx)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsSeatNear(vector pos, float dist)
	{
		foreach (RZ_Leader l : m_aList)
		{
			if (l.m_bActive && RZ_Util.DistXZ(l.m_vPos, pos) < dist)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The saved leader for this seat (same faction and role, within 150 m), else a new one.
	protected RZ_Leader AddSeat(array<ref RZ_Leader> saved, RZ_EFaction f, RZ_ELeaderRole role, int source, vector pos, string place, int safehouse)
	{
		RZ_Leader found;
		foreach (RZ_Leader old : saved)
		{
			if (old.m_eFaction == f && old.m_eRole == role && RZ_Util.DistXZ(old.m_vPos, pos) < 150)
			{
				found = old;
				break;
			}
		}

		if (found)
		{
			// Into the live list first, so the saved list never holds the only reference
			m_aList.Insert(found);
			saved.RemoveItem(found);
			found.m_vPos = RZ_Util.GroundPos(pos);
			found.m_iSource = source;
			found.m_iSafehouse = safehouse;
			found.m_bActive = true;
			return found;
		}

		RZ_Leader l = Generate(f, role, source, pos, place, safehouse, 0);
		m_aList.Insert(l);
		return l;
	}

	//------------------------------------------------------------------------------------------------
	//! A new person in the chair: name, title, backstory, two traits and a starting loyalty.
	protected RZ_Leader Generate(RZ_EFaction f, RZ_ELeaderRole role, int source, vector pos, string place, int safehouse, int generation)
	{
		RZ_Leader l = new RZ_Leader();
		l.m_iId = m_iNextId;
		m_iNextId++;
		l.m_iGeneration = generation;
		l.m_eFaction = f;
		l.m_eRole = role;
		l.m_iSource = source;
		l.m_vPos = RZ_Util.GroundPos(pos);
		l.m_sPlace = place;
		l.m_iSafehouse = safehouse;

		RZ_Rng rng = new RZ_Rng(m_iSeed + l.m_iId * 7919 + generation * 104729);
		string display;
		string shortName;
		RZ_LeaderVoice.MakeName(rng, f, role, display, shortName);
		l.m_sName = display;
		l.m_sShort = shortName;
		l.m_sTitle = RZ_LeaderVoice.Title(f, role, place);
		l.m_sBackstory = RZ_LeaderVoice.Backstory(rng, f, place);
		l.m_iTraitA = rng.Int(0, TRAIT_COUNT);
		l.m_iTraitB = rng.Int(0, TRAIT_COUNT - 1);
		if (l.m_iTraitB >= l.m_iTraitA)
			l.m_iTraitB = l.m_iTraitB + 1;

		if (role == RZ_ELeaderRole.STRONGHOLD)
		{
			// Last season's story can seed the first boss's trait (Director/RZ_StoryRules.c)
			int seeded = RZ_StoryRules.SeedTrait(f);
			if (generation == 0 && seeded >= 0 && l.m_iTraitA != seeded && l.m_iTraitB != seeded)
				l.m_iTraitA = seeded;

			l.m_eLoyalty = RZ_ELoyalty.LOYAL;
			l.m_iScore = 100;
			return l;
		}

		// 40% Loyal, 40% Independent, 20% Rogue (one Rogue per faction at most)
		float roll = rng.Float01();
		if (roll < 0.4)
			l.m_iScore = rng.Int(62, 86);
		else if (roll < 0.8 || HasRogue(f, -1))
			l.m_iScore = rng.Int(30, 56);
		else
			l.m_iScore = rng.Int(12, 23);

		l.m_eLoyalty = BandFor(l.m_iScore, RZ_ELoyalty.INDEPENDENT);
		return l;
	}

	//------------------------------------------------------------------------------------------------
	protected int CountPresent()
	{
		int n = 0;
		foreach (RZ_Leader l : m_aList)
		{
			if (l.IsPresent())
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! "Haven 3, Bastion 2..." for the start-up log.
	protected string DescribeRoster()
	{
		string text = "";
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			int n = 0;
			foreach (RZ_Leader l : m_aList)
			{
				if (l.m_eFaction == f && l.m_bActive)
					n++;
			}

			if (f > 0)
				text += ", ";

			text += string.Format("%1 %2", RZ_Factions.GetShortName(f), n);
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	protected string PlaceName(vector pos)
	{
		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(pos));
		if (s)
			return s.m_sName;

		return RZ_SettlementScanner.GridReference(pos);
	}

	// ================================================================ Lookups
	//------------------------------------------------------------------------------------------------
	RZ_Leader Get(int id)
	{
		if (id < 0)
			return null;

		foreach (RZ_Leader l : m_aList)
		{
			if (l.m_iId == id)
				return l;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	RZ_Leader GetStronghold(RZ_EFaction f)
	{
		foreach (RZ_Leader l : m_aList)
		{
			if (l.m_eFaction == f && l.IsStronghold() && l.m_bActive)
				return l;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Nearest present leader of `faction` (any faction if -1) within maxDist of pos. With
	//! loyalOnly, only Loyal ones (on-book contracts). With excludeId, skips that leader.
	RZ_Leader FindNearest(int faction, vector pos, float maxDist, bool loyalOnly = false, int excludeId = -1)
	{
		RZ_Leader best;
		float bestDist = maxDist;
		foreach (RZ_Leader l : m_aList)
		{
			if (!l.IsPresent() || l.m_iId == excludeId)
				continue;

			if (faction >= 0 && l.m_eFaction != faction)
				continue;

			if (loyalOnly && l.m_eLoyalty != RZ_ELoyalty.LOYAL)
				continue;

			float d = RZ_Util.DistXZ(pos, l.m_vPos);
			if (d <= bestDist)
			{
				bestDist = d;
				best = l;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! The leader whose seat is at pos (a quartermaster, a safehouse), or null.
	RZ_Leader FindAt(int faction, vector pos)
	{
		return FindNearest(faction, pos, SEAT_RADIUS);
	}

	//------------------------------------------------------------------------------------------------
	//! Present leaders of a faction (for missions that need another seat).
	void GetFactionLeaders(RZ_EFaction f, notnull array<RZ_Leader> result)
	{
		foreach (RZ_Leader l : m_aList)
		{
			if (l.m_eFaction == f && l.IsPresent())
				result.Insert(l);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasRogue(RZ_EFaction f, int excludeId)
	{
		foreach (RZ_Leader l : m_aList)
		{
			if (l.m_eFaction == f && l.m_iId != excludeId && l.IsPresent() && l.m_eLoyalty == RZ_ELoyalty.ROGUE)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Recent leader news, newest last.
	array<string> GetHeadlines()
	{
		return m_aHeadlines;
	}

	// ================================================================ Hooks for other systems
	//------------------------------------------------------------------------------------------------
	//! Hook for territory (docs/DESIGN.md, "Factions in the background"): an outpost exists at pos. Returns its
	//! leader's id: the saved one for this place if there is one, else a new leader. -1 if off.
	//! announce = false: no headline (the territory writes its own).
	int RegisterOutpost(RZ_EFaction faction, vector pos, string placeName, bool announce = true)
	{
		if (!m_Settings.m_bEnabled || !RZ_Factions.IsValid(faction))
			return -1;

		if (placeName.IsEmpty())
			placeName = PlaceName(pos);

		foreach (RZ_Leader l : m_aList)
		{
			if (l.m_eFaction == faction && l.m_eRole == RZ_ELeaderRole.OUTPOST && RZ_Util.DistXZ(l.m_vPos, pos) < 150)
			{
				l.m_bActive = true;
				if (l.m_iSource != RZ_Leader.SOURCE_MARKER)
					l.m_iSource = RZ_Leader.SOURCE_EXTERNAL;

				return l.m_iId;
			}
		}

		RZ_Leader fresh = Generate(faction, RZ_ELeaderRole.OUTPOST, RZ_Leader.SOURCE_EXTERNAL, pos, placeName, -1, 0);
		m_aList.Insert(fresh);
		if (announce)
			AddHeadline(string.Format("%1 has a new outpost in %2. %3 is in charge.", RZ_Factions.GetName(faction), placeName, fresh.m_sName));

		return fresh.m_iId;
	}

	//------------------------------------------------------------------------------------------------
	//! Saved outpost leaders waiting for the territory to register their seat (old interim
	//! posts, and outposts the territory hasn't registered yet), for adoption on the first run.
	void GetDormantOutposts(RZ_EFaction f, notnull array<RZ_Leader> result)
	{
		foreach (RZ_Leader l : m_aList)
		{
			if (l.m_eFaction == f && l.m_eRole == RZ_ELeaderRole.OUTPOST && !l.m_bActive && l.m_iSource == RZ_Leader.SOURCE_EXTERNAL)
				result.Insert(l);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Hook for territory: the outpost was lost or abandoned. Its leader leaves with it.
	void UnregisterOutpost(int leaderId, string why)
	{
		RZ_Leader l = Get(leaderId);
		if (!l || l.IsStronghold())
			return;

		DespawnBody(l);
		FailJobsOf(l);
		m_aList.RemoveItem(l);
		if (!why.IsEmpty())
			AddHeadline(why);
	}

	//------------------------------------------------------------------------------------------------
	//! Hook for new-player guidance: the line a leader says the first time a player talks to
	//! them. Who the faction is, who this leader is, and a hint about the other factions in
	//! their voice, with where the nearest of them sits. leaderId -1 = the faction's stronghold
	//! leader. Marks them as met. "" if there's no such leader.
	static string GetFirstContactLine(RZ_EFaction faction, int leaderId, int playerId)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetLeaders())
			return "";

		RZ_Leaders leaders = gm.GetLeaders();
		RZ_Leader l = leaders.Get(leaderId);
		if (!l)
			l = leaders.GetStronghold(faction);

		if (!l || !l.IsPresent() || l.m_eFaction != faction)
			return "";

		return leaders.FirstContact(l, playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Hook for new-player guidance: "Major Weber (Task Force Bastion), Morton, 1.2 km NE" for
	//! a faction's stronghold leader, "" if none.
	static string DescribeStronghold(RZ_EFaction faction, vector from)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetLeaders())
			return "";

		RZ_Leader l = gm.GetLeaders().GetStronghold(faction);
		if (!l || !l.IsPresent())
			return "";

		return string.Format("%1 (%2), %3, %4", l.m_sName, RZ_Factions.GetName(faction), l.m_sPlace, RZ_Util.DirectionText(from, l.m_vPos));
	}

	//------------------------------------------------------------------------------------------------
	//! Hook for new-player guidance: a present leader whose body (or tagged quartermaster)
	//! stands within radius of pos, null if none. The nearest plausible speaker for a tip.
	RZ_Leader FindBodyNear(vector pos, float radius)
	{
		foreach (RZ_Leader l : m_aList)
		{
			if (!l.IsPresent())
				continue;

			IEntity body = l.m_Body;
			if (!body)
				body = l.m_TaggedQm;

			if (body && RZ_Util.DistXZ(body.GetOrigin(), pos) <= radius)
				return l;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Every leader this player has talked to and who is still in the chair (the standing screen).
	void ListMet(int playerId, notnull array<RZ_Leader> result)
	{
		if (playerId <= 0)
			return;

		string ident = Ident(playerId);
		foreach (RZ_Leader l : m_aList)
		{
			if (l.IsPresent() && l.Remembers(ident, MEM_MET))
				result.Insert(l);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! What they think of this player, in their voice (one line, the standing screen).
	string OpinionLine(RZ_Leader l, int playerId)
	{
		return RZ_LeaderVoice.Opinion(Attitude(l, playerId));
	}

	//------------------------------------------------------------------------------------------------
	//! True if this player has already talked to this leader.
	bool HasMet(int leaderId, int playerId)
	{
		RZ_Leader l = Get(leaderId);
		if (!l || playerId <= 0)
			return false;

		return l.Remembers(Ident(playerId), MEM_MET);
	}

	//------------------------------------------------------------------------------------------------
	string FirstContact(RZ_Leader l, int playerId)
	{
		l.Remember(Ident(playerId), MEM_MET);
		RZ_Rng rng = new RZ_Rng(m_iSeed + l.m_iId * 31 + playerId * 7);

		string text = string.Format("%1\n%2 %3", RZ_LeaderVoice.WhoWeAre(l.m_eFaction), l.m_sName, l.m_sBackstory);

		// One other faction (two for a Talker), in this leader's voice, with where they sit
		array<int> others = {};
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (f != l.m_eFaction)
				others.Insert(f);
		}

		rng.ShuffleInts(others);
		int hints = 1;
		if (l.HasTrait(RZ_ELeaderTrait.TALKER))
			hints = 2;

		for (int i = 0; i < hints && i < others.Count(); i++)
		{
			text += "\n" + RZ_LeaderVoice.HintAbout(l.m_eFaction, others[i]);
			RZ_Leader near = FindNearest(others[i], l.m_vPos, 100000);
			if (near)
				text += string.Format(" (%1: %2, %3)", RZ_Factions.GetShortName(others[i]), near.m_sPlace, RZ_Util.DirectionText(l.m_vPos, near.m_vPos));
		}

		return text;
	}

	// ================================================================ Memory and attitude
	//------------------------------------------------------------------------------------------------
	protected string Ident(int playerId)
	{
		return m_GM.GetProfile(playerId).m_sIdentity;
	}

	//------------------------------------------------------------------------------------------------
	void Remember(int leaderId, int playerId, int flag)
	{
		RZ_Leader l = Get(leaderId);
		if (l && playerId > 0)
			l.Remember(Ident(playerId), flag);
	}

	//------------------------------------------------------------------------------------------------
	//! Faction standing plus memory, weighted by loyalty (docs/REPUTATION.md):
	//!   Loyal        standing + memory
	//!   Independent  half the standing + double the memory
	//!   Rogue        double the memory; bad standing with the stronghold doesn't count if
	//!                you've helped them
	//! -2 or lower: they won't deal with you.
	int Attitude(RZ_Leader l, int playerId)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		RZ_ETier tier = profile.GetTier(l.m_eFaction);
		int rank = RZ_Reputation.GetAccessRank(tier);
		if (l.HasTrait(RZ_ELeaderTrait.PRAGMATIST) && rank < -1)
			rank = -1;

		int mem = l.GetMemory(profile.m_sIdentity);
		int memScore = 0;
		if ((mem & MEM_HELPED) != 0)
			memScore += 1;
		if ((mem & MEM_SAVED) != 0)
			memScore += 2;
		if ((mem & MEM_CROSSED) != 0)
		{
			if (l.HasTrait(RZ_ELeaderTrait.MERCIFUL))
				memScore -= 2;
			else if (l.HasTrait(RZ_ELeaderTrait.PARANOID))
				memScore -= 4;
			else
				memScore -= 3;
		}

		// Earlier lives at half, rounded toward zero; met doesn't carry (docs/REPUTATION.md)
		memScore += MemoryScore(l, l.OldMemory(profile.m_sIdentity)) / 2;

		// What they think of how you talk to them (Dialogue > Trees): regard / 2
		memScore += RZ_DialogueMemory.AttitudeFrom(playerId, string.Format("leader:%1:%2", l.m_iId, l.m_iGeneration));

		if (l.HasTrait(RZ_ELeaderTrait.VETERAN))
		{
			string bg = profile.m_sBackgroundId;
			if (bg == "soldier" || bg == "police")
				memScore += 1;
		}

		if (l.m_eLoyalty == RZ_ELoyalty.LOYAL)
		{
			if (RZ_Reputation.IsHunted(tier))
				return -5;

			return rank + memScore;
		}

		if (l.m_eLoyalty == RZ_ELoyalty.INDEPENDENT)
			return rank / 2 + memScore * 2;

		// Rogue
		bool owes = (mem & (MEM_HELPED | MEM_SAVED)) != 0;
		if (rank < 0 && owes)
			rank = 0;

		return rank / 2 + memScore * 2;
	}

	//------------------------------------------------------------------------------------------------
	//! The built weights of a set of memory bits: helped +1, saved +2, crossed -3 (Merciful -2,
	//! Paranoid -4).
	protected int MemoryScore(RZ_Leader l, int mem)
	{
		int score = 0;
		if ((mem & MEM_HELPED) != 0)
			score += 1;
		if ((mem & MEM_SAVED) != 0)
			score += 2;
		if ((mem & MEM_CROSSED) != 0)
		{
			if (l.HasTrait(RZ_ELeaderTrait.MERCIFUL))
				score -= 2;
			else if (l.HasTrait(RZ_ELeaderTrait.PARANOID))
				score -= 4;
			else
				score -= 3;
		}

		return score;
	}

	//------------------------------------------------------------------------------------------------
	//! For leader jobs (RZ_Mission.IsEligible): will this leader give this player work?
	bool CanWorkFor(int leaderId, int playerId)
	{
		RZ_Leader l = Get(leaderId);
		if (!l || !l.IsPresent() || playerId <= 0)
			return false;

		return Attitude(l, playerId) >= 0;
	}

	// ================================================================ Trait effects (all capped at +-20%)
	//------------------------------------------------------------------------------------------------
	protected static float Cap(float mult)
	{
		return Math.Clamp(mult, MULT_MIN, MULT_MAX);
	}

	//------------------------------------------------------------------------------------------------
	//! Favor paid on a contract this leader gives: Greedy x1.2, Veteran x1.1.
	float FavorMult(int leaderId)
	{
		RZ_Leader l = Get(leaderId);
		if (!l)
			return 1;

		float mult = 1;
		if (l.HasTrait(RZ_ELeaderTrait.GREEDY))
			mult = mult * 1.2;
		if (l.HasTrait(RZ_ELeaderTrait.VETERAN))
			mult = mult * 1.1;

		return Cap(mult);
	}

	//------------------------------------------------------------------------------------------------
	//! Fame (or Infamy worked off, for amends) from a contract this leader gives:
	//! Paranoid x0.85; Veteran x1.15 for Soldiers and Police; Merciful x1.15 on amends.
	float FameMult(int leaderId, int playerId, bool amends)
	{
		RZ_Leader l = Get(leaderId);
		if (!l)
			return 1;

		float mult = 1;
		if (l.HasTrait(RZ_ELeaderTrait.PARANOID))
			mult = mult * 0.85;

		if (l.HasTrait(RZ_ELeaderTrait.VETERAN) && playerId > 0)
		{
			string bg = m_GM.GetProfile(playerId).m_sBackgroundId;
			if (bg == "soldier" || bg == "police")
				mult = mult * 1.15;
		}

		// A Merciful stronghold leader already forgives faction-wide (ScaleInfamy); don't count it twice
		if (amends && l.HasTrait(RZ_ELeaderTrait.MERCIFUL) && !l.IsStronghold())
			mult = mult * 1.15;

		return Cap(mult);
	}

	//------------------------------------------------------------------------------------------------
	//! Prices at this leader's quartermaster: Greedy x1.15; their memory of you x0.9 (helped
	//! or saved) or x1.1 (crossed), doubled for an Independent or Rogue leader.
	float PriceMult(RZ_Leader l, int playerId)
	{
		if (!l || playerId <= 0)
			return 1;

		float mult = 1;
		if (l.HasTrait(RZ_ELeaderTrait.GREEDY))
			mult = mult * 1.15;

		float memWeight = 1;
		if (l.m_eLoyalty != RZ_ELoyalty.LOYAL)
			memWeight = 2;

		int mem = l.GetMemory(Ident(playerId));
		if ((mem & MEM_CROSSED) != 0)
			mult = mult * (1 + 0.1 * memWeight);
		else if ((mem & (MEM_HELPED | MEM_SAVED)) != 0)
			mult = mult * (1 - 0.1 * memWeight);

		return Cap(mult);
	}

	//------------------------------------------------------------------------------------------------
	//! A Rogue leader serves people the stronghold turned away, if they've helped them.
	bool ShopOverride(RZ_Leader l, int playerId)
	{
		if (!l || l.m_eLoyalty != RZ_ELoyalty.ROGUE || playerId <= 0)
			return false;

		int mem = l.GetMemory(Ident(playerId));
		return (mem & MEM_CROSSED) == 0 && (mem & (MEM_HELPED | MEM_SAVED)) != 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Faction-wide temper, from the stronghold leader (RZ_GameModeComponent.AddInfamy):
	//! Paranoid x1.15 on Infamy gained; Merciful x0.9 on Infamy gained and x1.2 on Infamy
	//! worked off. Never turns a change into nothing.
	int ScaleInfamy(RZ_EFaction faction, int amount)
	{
		if (amount == 0)
			return 0;

		RZ_Leader top = GetStronghold(faction);
		if (!top)
			return amount;

		float mult = 1;
		if (amount > 0)
		{
			if (top.HasTrait(RZ_ELeaderTrait.PARANOID))
				mult = mult * 1.15;
			if (top.HasTrait(RZ_ELeaderTrait.MERCIFUL))
				mult = mult * 0.9;
		}
		else if (top.HasTrait(RZ_ELeaderTrait.MERCIFUL))
		{
			mult = mult * 1.2;
		}

		int scaled = RZ_Util.RoundToInt(amount * Cap(mult));
		if (scaled == 0)
		{
			if (amount > 0)
				return 1;

			return -1;
		}

		return scaled;
	}

	//------------------------------------------------------------------------------------------------
	//! How likely a faction's contract is to be picked, from its stronghold leader's traits
	//! (RZ_MissionManager.CreateRandom). Ruthless: contracts that hurt a rival; Merciful:
	//! amends; Idealist: work that hurts nobody; Pragmatist: choice missions; Veteran: Trusted+.
	float ContractWeightMult(RZ_Mission m)
	{
		if (!m || !m.m_bContract || !m.m_bHasGiver)
			return 1;

		RZ_Leader top = GetStronghold(m.m_Req.m_eGiver);
		if (!top)
			return 1;

		float mult = 1;
		bool dark = m.m_bHasRival && m.m_iRivalInfamy > 0;
		if (top.HasTrait(RZ_ELeaderTrait.RUTHLESS) && dark)
			mult = mult * 1.2;
		if (top.HasTrait(RZ_ELeaderTrait.MERCIFUL) && m.m_bAmends)
			mult = mult * 1.2;
		if (top.HasTrait(RZ_ELeaderTrait.IDEALIST))
		{
			if (dark)
				mult = mult * 0.85;
			else
				mult = mult * 1.1;
		}

		if (top.HasTrait(RZ_ELeaderTrait.PRAGMATIST) && RZ_Mission_Choice.Cast(m) != null)
			mult = mult * 1.2;
		if (top.HasTrait(RZ_ELeaderTrait.VETERAN) && RZ_Reputation.GetAccessRank(m.m_Req.m_eMinTier) >= 2)
			mult = mult * 1.2;

		return Cap(mult);
	}

	//------------------------------------------------------------------------------------------------
	//! How much a leader leans toward a job of this tone (personal jobs).
	protected float ToneBias(RZ_Leader l, int tone)
	{
		float mult = 1;
		bool dark = (tone & TONE_DARK) != 0;
		bool hopeful = (tone & TONE_HOPEFUL) != 0;
		if (l.HasTrait(RZ_ELeaderTrait.RUTHLESS))
		{
			if (dark)
				mult = mult * 2;
			if (hopeful)
				mult = mult * 0.7;
		}

		if (l.HasTrait(RZ_ELeaderTrait.IDEALIST))
		{
			if (hopeful)
				mult = mult * 2;
			if (dark)
				mult = mult * 0.5;
		}

		if (l.HasTrait(RZ_ELeaderTrait.MERCIFUL) && dark)
			mult = mult * 0.6;
		if (l.HasTrait(RZ_ELeaderTrait.PRAGMATIST) && (tone & TONE_CROSS) != 0)
			mult = mult * 2;
		if (l.HasTrait(RZ_ELeaderTrait.GREEDY) && (tone & TONE_CROSS) != 0)
			mult = mult * 1.3;
		if (l.HasTrait(RZ_ELeaderTrait.VETERAN) && (tone & TONE_HARD) != 0)
			mult = mult * 1.75;
		if (l.HasTrait(RZ_ELeaderTrait.TALKER) && (tone & TONE_TALK) != 0)
			mult = mult * 1.75;
		if (l.HasTrait(RZ_ELeaderTrait.PARANOID) && (tone & TONE_TALK) != 0)
			mult = mult * 0.7;

		return mult;
	}

	// ================================================================ Contracts in the leader's voice
	//------------------------------------------------------------------------------------------------
	//! On-book contracts (offered by radio) are voiced and paid by the nearest Loyal leader
	//! of the giver: usually the stronghold.
	void AssignVoice(RZ_Mission m)
	{
		if (!m || !m.m_bContract || !m.m_bHasGiver || m.m_iLeaderId >= 0)
			return;

		RZ_Leader l = FindNearest(m.m_Req.m_eGiver, m.m_vPos, 100000, true);
		if (l)
			m.m_iLeaderId = l.m_iId;
	}

	//------------------------------------------------------------------------------------------------
	//! The brief with the leader's intro and outro (by trait), or the plain brief.
	string VoiceBrief(RZ_Mission m, int playerId)
	{
		RZ_Leader l = Get(m.m_iLeaderId);
		if (!l || !l.IsPresent())
			return m.GetBrief(m_GM);

		RZ_Rng rng = new RZ_Rng(m_iSeed + m.m_iId * 131 + l.m_iId);
		int lead = l.m_iTraitA;
		if (rng.Chance(0.5))
			lead = l.m_iTraitB;

		string outro;
		if (rng.Chance(0.5))
			outro = RZ_LeaderVoice.Outro(rng, lead);
		else
			outro = RZ_LeaderVoice.SignOff(rng, l.m_eFaction);

		return string.Format("%1, %2:\n\"%3\" %4 \"%5\"", l.m_sName, l.m_sTitle, RZ_LeaderVoice.Intro(rng, lead), m.GetBrief(m_GM), outro);
	}

	//------------------------------------------------------------------------------------------------
	//! After rewards (RZ_MissionManager.Finish). paid: the participants who were paid.
	void OnMissionFinished(RZ_Mission m, bool success, array<int> paid)
	{
		if (!m || !m.m_bContract)
			return;

		RZ_Leader l = Get(m.m_iLeaderId);

		// The people who did the work are remembered by whoever gave it
		if (success && l && l.IsPresent() && !m.m_bNoLeaderCredit && !m.m_bExposed)
		{
			foreach (int playerId : paid)
			{
				l.Remember(Ident(playerId), MEM_HELPED);
			}
		}

		if (m.m_bInPersonOnly)
		{
			OnJobFinished(m, l, success, paid);
			return;
		}

		// On-book work going well or badly near an outpost or safehouse moves its loyalty a little
		if (m.m_aParticipants.IsEmpty() || !m.m_bHasGiver)
			return;

		RZ_Leader local = FindNearest(m.m_Req.m_eGiver, m.m_vPos, 3000);
		if (!local || local.IsStronghold())
			return;

		// The seat ledger (Director/RZ_MajorBudget.c): contracts never take a leader below 30
		int shift = -2;
		if (success)
			shift = 1;

		RZ_MajorBudget budget = RZ_MajorBudget.Get();
		if (budget)
			shift = budget.ContractLoyalty(local.m_iScore, local.IsStronghold(), shift);

		ShiftLoyalty(local, shift, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnJobFinished(RZ_Mission m, RZ_Leader l, bool success, array<int> paid)
	{
		if (l)
			l.m_fNextJobAt = m_fClock + m_Settings.m_fJobMinutes * 60;

		if (!success || !l || !l.IsPresent())
			return;

		// A local deed at this seat, for the leader's next greeting ("You're the one who...")
		if (!m.m_bExposed)
		{
			foreach (int doer : paid)
			{
				RZ_RepVoice.SetLocalDeed(doer, string.Format("leader:%1:%2", l.m_iId, l.m_iGeneration), m.DeedPhrase(m_GM));
			}
		}

		if (m.m_bExposed)
		{
			foreach (int exposer : paid)
			{
				l.Remember(Ident(exposer), MEM_CROSSED);
			}

			Discover(l, null, string.Format("Someone turned in %1 of %2. The stronghold knows everything now.", l.m_sName, l.m_sPlace));
			return;
		}

		if (!m.m_bRogueJob || paid.IsEmpty())
			return;

		// Off the books: they get bolder, unless someone talks
		float chance = m_Settings.m_fDiscoveryChance;
		if (m.m_bSeenByFaction)
			chance += m_Settings.m_fSeenChance;
		if (l.HasTrait(RZ_ELeaderTrait.TALKER))
			chance += 0.1;
		if (l.HasTrait(RZ_ELeaderTrait.PARANOID))
			chance -= 0.05;

		chance = Math.Clamp(chance, 0.05, 0.75);
		if (Math.RandomFloat01() >= chance)
		{
			ShiftLoyalty(l, -3, false);
			return;
		}

		string what = string.Format("%1's side deals in %2 have come out: %3.", l.m_sName, l.m_sPlace, m.m_sTitle);
		Discover(l, paid, what);
	}

	//------------------------------------------------------------------------------------------------
	//! A rogue job came out. Whoever did it takes Infamy with the faction; the stronghold
	//! removes the leader (half the time) or pulls them back into line.
	void Discover(RZ_Leader l, array<int> culprits, string what)
	{
		if (!l)
			return;

		string factionName = RZ_Factions.GetName(l.m_eFaction);
		if (culprits)
		{
			foreach (int playerId : culprits)
			{
				m_GM.AddInfamy(playerId, l.m_eFaction, m_Settings.m_iDiscoveryInfamy);
				m_GM.NotifyPlayer(playerId, factionName, string.Format("%1 You were part of it. (+%2 Infamy)", what, m_Settings.m_iDiscoveryInfamy));
			}
		}

		if (l.IsStronghold())
			return;

		if (Math.RandomFloat01() < 0.5)
		{
			Dismiss(l, string.Format("%1 %2 has removed %3 from %4.", what, RZ_Factions.GetName(l.m_eFaction), l.m_sName, l.m_sPlace));
			return;
		}

		AddHeadline(what + string.Format(" %1 kept their place, for now, and is toeing the line.", l.m_sName));
		ShiftLoyalty(l, 25, true);
	}

	// ================================================================ Personal jobs
	//------------------------------------------------------------------------------------------------
	//! The leader's open personal job, or null.
	RZ_Mission FindJob(RZ_Leader l)
	{
		if (!l)
			return null;

		foreach (RZ_Mission m : m_GM.GetMissions().GetActive())
		{
			if (m.m_bInPersonOnly && m.m_iLeaderId == l.m_iId && m.m_eState == RZ_EMissionState.ACTIVE)
				return m;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! A new personal job from this leader, picked by faction, loyalty and trait. Null if none
	//! fits (or the leader is Loyal: their work is the stronghold's, on the radio).
	RZ_Mission TryCreateJob(RZ_Leader l)
	{
		if (!l || !l.IsPresent() || l.m_iHeldBy >= 0 || l.m_eLoyalty == RZ_ELoyalty.LOYAL)
			return null;

		if (m_fClock < l.m_fNextJobAt || FindJob(l))
			return null;

		array<ref RZ_Mission> candidates = {};
		array<float> weights = {};
		float total = 0;
		int online = GetGame().GetPlayerManager().GetPlayerCount();
		foreach (typename t : m_aJobTypes)
		{
			RZ_Mission m = RZ_Mission.Cast(t.Spawn());
			if (!m)
				continue;

			m.Configure(m_GM);
			if (m.m_iJobFaction != l.m_eFaction || m.m_iJobLoyalty != l.m_eLoyalty)
				continue;

			// The same check as any mission (RZ_MissionManager.CanRunNow): world gates and facts,
			// No Vehicles, one of each type, its claims free. The leader's own weight comes next
			m.m_iLeaderId = l.m_iId;
			if (!m_GM.GetMissions().CanRunNow(m, online, false))
				continue;
			float w = m.GetLeaderWeight(m_GM, l) * ToneBias(l, m.m_iJobTone);
			if (w <= 0)
				continue;

			candidates.Insert(m);
			weights.Insert(w);
			total += w;
		}

		while (!candidates.IsEmpty())
		{
			float roll = Math.RandomFloat(0, total);
			int pick = candidates.Count() - 1;
			for (int i = 0; i < candidates.Count(); i++)
			{
				roll -= weights[i];
				if (roll <= 0)
				{
					pick = i;
					break;
				}
			}

			RZ_Mission job = candidates[pick];
			if (m_GM.GetMissions().StartMission(job))
			{
				l.m_fNextJobAt = m_fClock + m_Settings.m_fJobMinutes * 60;
				return job;
			}

			total -= weights[pick];
			candidates.Remove(pick);
			weights.Remove(pick);
		}

		// Nothing fits here right now; don't try again every tick
		l.m_fNextJobAt = m_fClock + 300;
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! "Ask for work": their own job if they have one for you, else the stronghold's contracts.
	protected void OfferWork(RZ_Leader l, int playerId, IEntity user)
	{
		string header = l.GetHeader();
		RZ_Mission job = FindJob(l);
		if (!job)
			job = TryCreateJob(l);

		if (job && job.IsEligible(m_GM, playerId))
		{
			if (!job.m_aOfferedTo.Contains(playerId))
				job.m_aOfferedTo.Insert(playerId);

			string where = job.GetLocationName(m_GM);
			if (user)
				where = job.GetWhereTextFor(m_GM, user.GetOrigin(), playerId);

			string bonus = "";
			if (job.HasBonus(m_GM, playerId))
				bonus = "\nYour background gives you an edge on this one.";

			m_GM.NotifyPlayer(playerId, "Off the books: " + job.m_sTitle, string.Format("%1\n(%2)%3", VoiceBrief(job, playerId), where, bonus));
			return;
		}

		string work = "";
		if (user)
			work = m_GM.GetMissions().ListContractsFor(playerId, l.m_eFaction, user);

		if (!work.IsEmpty())
		{
			m_GM.NotifyPlayer(playerId, header, "\"This is what the stronghold wants doing:\"\n" + work);
			return;
		}

		m_GM.NotifyPlayer(playerId, header, "\"Nothing for you today. Come back tomorrow, or the day after.\"");
	}

	// ================================================================ Loyalty
	//------------------------------------------------------------------------------------------------
	//! Band for a score, with a little hysteresis around the current band.
	protected static RZ_ELoyalty BandFor(int score, RZ_ELoyalty current)
	{
		int loyalAt = LOYAL_AT;
		int rogueBelow = ROGUE_BELOW;
		if (current == RZ_ELoyalty.LOYAL)
			loyalAt = LOYAL_AT - 4;
		if (current == RZ_ELoyalty.ROGUE)
			rogueBelow = ROGUE_BELOW + 4;

		if (score >= loyalAt)
			return RZ_ELoyalty.LOYAL;

		if (score < rogueBelow)
			return RZ_ELoyalty.ROGUE;

		return RZ_ELoyalty.INDEPENDENT;
	}

	//------------------------------------------------------------------------------------------------
	//! Moves a leader's loyalty. Capped at 15 points an hour unless forced (discovery, a rein-in),
	//! never below Independent while another leader of the faction is Rogue, and stronghold
	//! leaders are always Loyal: no faction ever splits.
	void ShiftLoyalty(RZ_Leader l, int delta, bool force)
	{
		if (!l || l.IsStronghold() || delta == 0)
			return;

		if (!force)
		{
			if (m_fClock - l.m_fDriftWindow > DRIFT_WINDOW)
			{
				l.m_fDriftWindow = m_fClock;
				l.m_iDriftSum = 0;
			}

			int used = l.m_iDriftSum;
			if (used < 0)
				used = -used;

			int room = DRIFT_CAP - used;
			if (room <= 0)
				return;

			delta = Math.ClampInt(delta, -room, room);
			l.m_iDriftSum = l.m_iDriftSum + delta;
		}

		RZ_ELoyalty before = l.m_eLoyalty;
		l.m_iScore = Math.ClampInt(l.m_iScore + delta, 0, 100);
		if (l.m_iScore < ROGUE_BELOW && HasRogue(l.m_eFaction, l.m_iId))
			l.m_iScore = ROGUE_BELOW;

		l.m_eLoyalty = BandFor(l.m_iScore, before);
		if (l.m_eLoyalty != before)
			OnLoyaltyChanged(l, before);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLoyaltyChanged(RZ_Leader l, RZ_ELoyalty before)
	{
		string stronghold = "the stronghold";
		RZ_Leader top = GetStronghold(l.m_eFaction);
		if (top)
			stronghold = top.m_sPlace;

		if (l.m_eLoyalty == RZ_ELoyalty.ROGUE)
		{
			AddHeadline(string.Format("%1 in %2 has stopped taking orders from %3.", l.m_sName, l.m_sPlace, stronghold));
			l.m_fNextReinAt = m_fClock + 300;
			return;
		}

		if (l.m_eLoyalty == RZ_ELoyalty.LOYAL)
		{
			AddHeadline(string.Format("%1 in %2 is back in line with %3.", l.m_sName, l.m_sPlace, stronghold));
			return;
		}

		// Independent: only people who know the faction hear about it
		string text = string.Format("%1 in %2 is running things their own way now.", l.m_sName, l.m_sPlace);
		if (before == RZ_ELoyalty.ROGUE)
			text = string.Format("%1 in %2 has made a sort of peace with %3. A sort of one.", l.m_sName, l.m_sPlace, stronghold);

		NotifyFactionFriends(l.m_eFaction, text);
	}

	//------------------------------------------------------------------------------------------------
	//! Once an hour: loyalty drifts back toward the leader's temperament.
	protected void NaturalDrift()
	{
		foreach (RZ_Leader l : m_aList)
		{
			if (!l.IsPresent() || l.IsStronghold())
				continue;

			int baseline = 55;
			if (l.HasTrait(RZ_ELeaderTrait.IDEALIST))
				baseline += 10;
			if (l.HasTrait(RZ_ELeaderTrait.PARANOID))
				baseline += 5;
			if (l.HasTrait(RZ_ELeaderTrait.PRAGMATIST))
				baseline -= 10;
			if (l.HasTrait(RZ_ELeaderTrait.RUTHLESS))
				baseline -= 5;

			if (l.m_iScore < baseline)
				ShiftLoyalty(l, 2, false);
			else if (l.m_iScore > baseline + 10)
				ShiftLoyalty(l, -1, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A safehouse that runs short of food blames whoever isn't feeding it.
	protected void CheckShortages()
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses)
			return;

		foreach (RZ_Leader l : m_aList)
		{
			if (!l.IsPresent() || l.m_eRole != RZ_ELeaderRole.SAFEHOUSE)
				continue;

			RZ_Safehouse h = houses.Get(l.m_iSafehouse);
			if (!h)
				continue;

			if (h.m_bShort && !l.m_bWasShort)
				ShiftLoyalty(l, -5, false);
			else if (!h.m_bShort && l.m_bWasShort)
				ShiftLoyalty(l, 2, false);

			l.m_bWasShort = h.m_bShort;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The stronghold sends someone to rein in a rogue place (a radio contract).
	protected void CheckReinIns()
	{
		foreach (RZ_Leader l : m_aList)
		{
			if (!l.IsPresent() || l.m_eLoyalty != RZ_ELoyalty.ROGUE || l.m_iHeldBy >= 0)
				continue;

			if (m_fClock < l.m_fNextReinAt || IsReinInRunning(l))
				continue;

			l.m_fNextReinAt = m_fClock + REIN_INTERVAL;
			RZ_Mission_ReinIn rein = new RZ_Mission_ReinIn();
			rein.Configure(m_GM);
			if (!rein.Prepare(m_GM, l))
				continue;

			m_GM.GetMissions().StartMission(rein);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsReinInRunning(RZ_Leader l)
	{
		foreach (RZ_Mission m : m_GM.GetMissions().GetActive())
		{
			RZ_Mission_ReinIn rein = RZ_Mission_ReinIn.Cast(m);
			if (rein && rein.GetTarget() == l.m_iId)
				return true;
		}

		return false;
	}

	// ================================================================ Death and replacement
	//------------------------------------------------------------------------------------------------
	//! A mission uses this leader (a stand-in body): no body of their own until Release.
	void Hold(int leaderId, int missionId)
	{
		RZ_Leader l = Get(leaderId);
		if (!l)
			return;

		DespawnBody(l);
		l.m_iHeldBy = missionId;
	}

	//------------------------------------------------------------------------------------------------
	void Release(int leaderId)
	{
		RZ_Leader l = Get(leaderId);
		if (l)
			l.m_iHeldBy = -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Every half hour, off-screen: an outpost or safehouse leader may be bitten (more likely in
	//! an infected town) or killed in a raid (a hungry safehouse, an exposed outpost).
	protected void DangerRoll(array<IEntity> players)
	{
		if (m_Settings.m_fDangerScale <= 0)
			return;

		// While the world settles in the epilogue week, a quarter of the rate (docs/EPILOGUE.md)
		float settle = 1;
		if (m_GM.GetFacts().HoldingsFrozen())
			settle = 0.25;

		foreach (RZ_Leader l : m_aList)
		{
			if (!l.IsPresent() || l.IsStronghold() || l.m_iHeldBy >= 0)
				continue;

			// A story is counting on them (RZ_WorldFacts claims): nothing happens to them off-screen
			if (m_GM.GetFacts().IsClaimed(RZ_WorldFacts.Leader(l.m_iId)))
				continue;

			if (RZ_Util.NearestDistance(l.m_vPos, players) < 400)
				continue;

			RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(l.m_vPos));
			float infection = 0;
			if (s)
				infection = s.m_fInfection;

			float bite = (0.01 + infection / 100 * 0.03) * m_Settings.m_fDangerScale * settle;
			float raid = 0.01 * m_Settings.m_fDangerScale * settle;
			RZ_Safehouses houses = m_GM.GetSafehouses();
			if (l.m_eRole == RZ_ELeaderRole.SAFEHOUSE && houses && houses.Get(l.m_iSafehouse) && houses.Get(l.m_iSafehouse).m_bShort)
				raid = raid * 2;

			float roll = Math.RandomFloat01();
			if (roll < bite)
			{
				Die(l, BittenHow(l), -1);
				continue;
			}

			if (roll < bite + raid)
				Die(l, "was killed in a raid on " + l.m_sPlace, -1);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! How an off-screen bite is told, each faction its own way (docs/LORE.md): nobody at Bastion
	//! turns alone, and the Red Hand says "bitten" about anyone. Ends without a full stop (the title
	//! already names the place).
	protected string BittenHow(RZ_Leader l)
	{
		if (l.m_eFaction == RZ_EFaction.MILITARY)
		{
			if (m_GM.HasLab())
				return "was bitten. A friend from the post took them to the lab. The dose didn't take, and the friend brought them home";

			return "was bitten. A friend from the post sat with them to the end";
		}

		if (l.m_eFaction == RZ_EFaction.BANDITS)
			return "was bitten, the crew says";

		return "was bitten and walked out of " + l.m_sPlace + " before it took hold";
	}

	//------------------------------------------------------------------------------------------------
	protected void Die(RZ_Leader l, string how, int killer)
	{
		// Grief: no jokes near the seat for a while, and the faction's radio goes plain
		RZ_ToneGuard.Death(l.m_vPos, l.m_eFaction);
		DespawnBody(l);
		FailJobsOf(l);
		l.m_bAlive = false;
		l.m_fDiedAt = m_fClock;
		l.m_iKilledBy = killer;
		AddHeadline(string.Format("%1, %2, %3.", l.m_sName, l.m_sTitle, how));

		// Morale, a funeral later, and maybe something people remember
		if (m_GM.GetFactionLife())
			m_GM.GetFactionLife().OnLeaderDied(l.m_eFaction, l.m_sName, l.m_sPlace, killer);

		RZ_WorldWriters.LeaderDied(l.m_eFaction, l.m_iId, l.m_sName, l.m_sPlace, l.m_vPos);

		if (killer > 0)
		{
			RZ_Deeds.AddKill(m_GM, killer, l.m_eFaction);
			m_GM.AddInfamy(killer, l.m_eFaction, m_Settings.m_iKillInfamy, "killed a leader", RZ_RepFlag.ACT, "shot " + l.m_sName);
			m_GM.NotifyPlayer(killer, RZ_Factions.GetName(l.m_eFaction), string.Format("Everyone will know who killed %1. (+%2 Infamy)", l.m_sName, m_Settings.m_iKillInfamy));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A leader dies off-screen because of something a mission did (Bastion raiding a camp
	//! someone sold out). Stronghold leaders never die, and nor does one a mission holds or a
	//! story has claimed (RZ_WorldFacts): false if it didn't happen.
	bool KillOffscreen(int leaderId, string how)
	{
		RZ_Leader l = Get(leaderId);
		if (!l || !l.IsPresent() || l.IsStronghold() || l.m_iHeldBy >= 0)
			return false;

		if (m_GM.GetFacts().IsClaimed(RZ_WorldFacts.Leader(leaderId)))
			return false;

		Die(l, how, -1);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A story moves a leader out of their seat (a questline's ending): replaced like a death,
	//! with this headline. Never a stronghold leader. False if they can't be moved.
	bool RemoveFromSeat(int leaderId, string headline, string byOwner = "")
	{
		RZ_Leader l = Get(leaderId);
		if (!l || !l.IsPresent() || l.IsStronghold() || l.m_iHeldBy >= 0)
			return false;

		// Another story is counting on them (a story may move a leader it claimed itself)
		if (m_GM.GetFacts().IsClaimedByOther(RZ_WorldFacts.Leader(leaderId), byOwner))
			return false;

		Dismiss(l, headline);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Dismissed by the stronghold: gone from the seat, replaced like a death.
	protected void Dismiss(RZ_Leader l, string headline)
	{
		DespawnBody(l);
		FailJobsOf(l);
		l.m_bAlive = false;
		l.m_fDiedAt = m_fClock;
		AddHeadline(headline);
	}

	//------------------------------------------------------------------------------------------------
	protected void FailJobsOf(RZ_Leader l)
	{
		foreach (RZ_Mission m : m_GM.GetMissions().GetActive())
		{
			if (m.m_bInPersonOnly && m.m_iLeaderId == l.m_iId && m.m_eState == RZ_EMissionState.ACTIVE)
				m.m_eState = RZ_EMissionState.FAILED;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Someone new takes the empty chair: new name, new traits, new loyalty, no memories. Same seat id.
	protected RZ_Leader Replace(RZ_Leader old)
	{
		RZ_Leader l = Generate(old.m_eFaction, old.m_eRole, old.m_iSource, old.m_vPos, old.m_sPlace, old.m_iSafehouse, old.m_iGeneration + 1);
		// The seat keeps its id: territory holdings point at their leader by it (UnregisterOutpost,
		// the founder's name), so a new id would leave the holding without its leader
		l.m_iId = old.m_iId;
		l.m_bActive = old.m_bActive;
		AddHeadline(string.Format("%1 has a new %2: %3. %4 (%5)", old.m_sPlace, ShortTitle(l), l.m_sName, l.m_sBackstory, RZ_LeaderVoice.LoyaltyName(l.m_eLoyalty)));
		return l;
	}

	//------------------------------------------------------------------------------------------------
	protected string ShortTitle(RZ_Leader l)
	{
		if (l.m_eRole == RZ_ELeaderRole.SAFEHOUSE)
			return "council head";

		switch (l.m_eFaction)
		{
			case RZ_EFaction.SURVIVORS: return "council head";
			case RZ_EFaction.MILITARY: return "commander";
			case RZ_EFaction.BANDITS: return "crew boss";
			case RZ_EFaction.INSTITUTE: return "lead researcher";
		}

		return "leader";
	}

	// ================================================================ Headlines
	//------------------------------------------------------------------------------------------------
	//! Server news ("Word on the roads"), kept for first spawns and leaders' news. announce =
	//! false keeps it quiet (territory, faction life): stored, never broadcast.
	void AddHeadline(string text, bool announce = true)
	{
		m_aHeadlines.Insert(text);
		while (m_aHeadlines.Count() > HEADLINES_KEPT)
		{
			m_aHeadlines.RemoveOrdered(0);
		}

		if (announce)
			m_GM.NotifyAll("Word on the roads", text);
	}

	//------------------------------------------------------------------------------------------------
	protected void NotifyFactionFriends(RZ_EFaction faction, string text)
	{
		array<int> all = {};
		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int playerId : all)
		{
			if (RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, faction), RZ_ETier.KNOWN))
				m_GM.NotifyPlayer(playerId, RZ_Factions.GetName(faction), text);
		}
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (m_aList.IsEmpty())
			return;

		m_fClock += dt;
		for (int i = 0; i < m_aList.Count(); i++)
		{
			RZ_Leader l = m_aList[i];
			if (!l.m_bAlive)
			{
				if (!l.IsStronghold() && m_fClock - l.m_fDiedAt >= m_Settings.m_fReplaceMinutes * 60)
					m_aList[i] = Replace(l);

				continue;
			}

			UpdateBody(l, players);
		}

		GreetNear(ids, players);
		WatchRogueJobs(players);
		CheckShortages();

		if (ids.IsEmpty())
			return;

		m_fJobTimer += dt;
		if (m_fJobTimer >= JOB_ROLL_INTERVAL)
		{
			m_fJobTimer = 0;
			RollJobs(players);
			CheckReinIns();
		}

		m_fDangerTimer += dt;
		if (m_fDangerTimer >= DANGER_INTERVAL)
		{
			m_fDangerTimer = 0;
			DangerRoll(players);
		}

		m_fDriftTimer += dt;
		if (m_fDriftTimer >= DRIFT_INTERVAL)
		{
			m_fDriftTimer = 0;
			NaturalDrift();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "Occasionally": a leader with someone nearby may come up with a job of their own.
	protected void RollJobs(array<IEntity> players)
	{
		foreach (RZ_Leader l : m_aList)
		{
			if (!l.IsPresent() || l.m_eLoyalty == RZ_ELoyalty.LOYAL || m_fClock < l.m_fNextJobAt)
				continue;

			if (RZ_Util.NearestDistance(l.m_vPos, players) > m_Settings.m_fBodyRadius)
				continue;

			if (Math.RandomFloat01() < JOB_ROLL_CHANCE)
				TryCreateJob(l);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Players who walk up to a leader with a job for them hear about it (once per 10 min).
	protected void GreetNear(array<int> ids, array<IEntity> players)
	{
		foreach (RZ_Leader l : m_aList)
		{
			if (!l.IsPresent())
				continue;

			RZ_Mission job = FindJob(l);
			if (!job)
				continue;

			foreach (int i, IEntity p : players)
			{
				if (RZ_Util.DistXZ(p.GetOrigin(), l.m_vPos) > GREET_RADIUS)
					continue;

				int playerId = ids[i];
				if (job.m_aOfferedTo.Contains(playerId) || !job.IsEligible(m_GM, playerId))
					continue;

				string key = string.Format("%1:%2", l.m_iId, playerId);
				float last;
				if (m_mGreeted.Find(key, last) && m_fClock - last < GREET_COOLDOWN)
					continue;

				// An unasked contact: the interruption budget may hold it for now (UI/RZ_NoticeLanes.c)
				RZ_NoticeLanes lanes = RZ_NoticeLanes.Get();
				if (lanes && !lanes.TryContact(playerId, RZ_EContact.LEADER))
					continue;

				m_mGreeted.Set(key, m_fClock);
				m_GM.NotifyPlayer(playerId, l.GetHeader(), string.Format("%1 catches your eye and tilts their head at the door. They want a word, somewhere quieter. (Ask them for work.)", l.m_sShort));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A rogue job is "seen" if someone doing it passes another seat of the same faction, or
	//! (for the Haven) one of its wanderers.
	protected void WatchRogueJobs(array<IEntity> players)
	{
		foreach (RZ_Mission m : m_GM.GetMissions().GetActive())
		{
			if (!m.m_bRogueJob || m.m_bSeenByFaction || m.m_eState != RZ_EMissionState.ACTIVE)
				continue;

			RZ_Leader l = Get(m.m_iLeaderId);
			if (!l)
				continue;

			foreach (int playerId : m.m_aParticipants)
			{
				IEntity p = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
				if (!p)
					continue;

				vector at = p.GetOrigin();
				bool seen = FindNearest(l.m_eFaction, at, SEEN_RADIUS, false, l.m_iId) != null;
				if (!seen && l.m_eFaction == RZ_EFaction.SURVIVORS && m_GM.GetWanderers())
					seen = !m_GM.GetWanderers().SeenByHaven(at, 30).IsEmpty();

				if (!seen)
					continue;

				m.m_bSeenByFaction = true;
				m_GM.NotifyPlayer(playerId, m.m_sTitle, string.Format("Someone from %1 saw you. They may not know what they saw. They may.", RZ_Factions.GetNameInText(l.m_eFaction)));
				break;
			}
		}
	}

	// ================================================================ Bodies near players
	//------------------------------------------------------------------------------------------------
	protected void UpdateBody(RZ_Leader l, array<IEntity> players)
	{
		if (!l.m_bActive || l.m_iHeldBy >= 0)
		{
			DespawnBody(l);
			return;
		}

		float nearest = RZ_Util.NearestDistance(l.m_vPos, players);
		if (l.m_bHasBody)
		{
			if (!l.m_Body)
			{
				l.m_bHasBody = false;
			}
			else if (RZ_Util.CountAlive(l.m_Body) == 0)
			{
				OnBodyKilled(l);
				return;
			}
			else if (nearest > m_Settings.m_fBodyRadius + DESPAWN_MARGIN)
			{
				DespawnBody(l);
			}

			return;
		}

		if (nearest > m_Settings.m_fBodyRadius)
			return;

		ResourceName prefab = GetBodyPrefab(l);
		if (prefab.IsEmpty())
		{
			TagQuartermaster(l);
			return;
		}

		SpawnBody(l, prefab);
	}

	//------------------------------------------------------------------------------------------------
	//! Leader prefab for the role, else "" when the quartermaster here is the leader, else
	//! the puzzle civilian.
	protected ResourceName GetBodyPrefab(RZ_Leader l)
	{
		ResourceName prefab;
		ResourceName factionPrefab;
		switch (l.m_eFaction)
		{
			case RZ_EFaction.SURVIVORS: factionPrefab = m_Settings.m_sHavenLeaderPrefab; break;
			case RZ_EFaction.MILITARY: factionPrefab = m_Settings.m_sBastionLeaderPrefab; break;
			case RZ_EFaction.BANDITS: factionPrefab = m_Settings.m_sRedHandLeaderPrefab; break;
			case RZ_EFaction.INSTITUTE: factionPrefab = m_Settings.m_sHalcyonLeaderPrefab; break;
		}

		if (l.m_eRole == RZ_ELeaderRole.STRONGHOLD)
			prefab = factionPrefab;
		else if (l.m_eRole == RZ_ELeaderRole.SAFEHOUSE)
			prefab = m_Settings.m_sCouncilHeadPrefab;
		else
		{
			switch (l.m_eFaction)
			{
				case RZ_EFaction.MILITARY: prefab = m_Settings.m_sBastionOutpostPrefab; break;
				case RZ_EFaction.BANDITS: prefab = m_Settings.m_sRedHandOutpostPrefab; break;
				case RZ_EFaction.INSTITUTE: prefab = m_Settings.m_sHalcyonOutpostPrefab; break;
				case RZ_EFaction.SURVIVORS: prefab = m_Settings.m_sCouncilHeadPrefab; break;
			}

			if (prefab.IsEmpty())
				prefab = factionPrefab;
		}

		if (!prefab.IsEmpty())
			return prefab;

		if (l.m_eRole != RZ_ELeaderRole.OUTPOST && m_Settings.m_bQuartermasterIsLeader && HasQuartermaster(l))
			return "";

		return m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);
	}

	//------------------------------------------------------------------------------------------------
	//! A quartermaster stands (or will stand) at this seat.
	protected bool HasQuartermaster(RZ_Leader l)
	{
		if (l.m_eRole == RZ_ELeaderRole.SAFEHOUSE)
			return !m_GM.GetQuartermasterPrefab(RZ_EFaction.SURVIVORS).IsEmpty();

		vector qm;
		return m_GM.GetPerks().GetQuartermasterPos(l.m_eFaction, qm) && RZ_Util.DistXZ(qm, l.m_vPos) < SEAT_RADIUS;
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnBody(RZ_Leader l, ResourceName prefab)
	{
		IEntity e = RZ_Util.SpawnPrefab(prefab, RZ_Mission.LandPointNear(l.m_vPos, 2, 5), Math.RandomFloat(0, 360));
		if (!e)
			return;

		l.m_Body = e;
		l.m_bHasBody = true;
		if (l.IsStronghold())
			RZ_Util.MakeUntouchable(e);

		TagEntity(l, e);
	}

	//------------------------------------------------------------------------------------------------
	//! "Talk to <name>" and the three questions, if the entity has the puzzle object components.
	protected void TagEntity(RZ_Leader l, IEntity e)
	{
		IEntity target = e;
		SCR_AIGroup group = SCR_AIGroup.Cast(e);
		if (group)
		{
			AIAgent agent = group.GetLeaderAgent();
			if (!agent || !agent.GetControlledEntity())
				return;

			target = agent.GetControlledEntity();
		}

		array<string> labels = {"Ask for work", "Ask for news", "Ask what they think of you"};
		RZ_PuzzleObjectComponent.Tag(target, TAG, l.m_iId, "", RZ_PuzzleLabels.Join(labels));
		RZ_ClueComponent.Tag(target, TAG, l.m_iId, "Talk to " + l.m_sShort);
	}

	//------------------------------------------------------------------------------------------------
	//! The quartermaster at this seat is the leader: label its puzzle components, if it has any.
	protected void TagQuartermaster(RZ_Leader l)
	{
		RZ_QuartermasterComponent best;
		float bestDist = 40;
		foreach (RZ_QuartermasterComponent qm : RZ_QuartermasterComponent.GetAll())
		{
			if (!qm || qm.GetFaction() != l.m_eFaction)
				continue;

			float d = RZ_Util.DistXZ(qm.GetOwner().GetOrigin(), l.m_vPos);
			if (d < bestDist)
			{
				bestDist = d;
				best = qm;
			}
		}

		if (!best || best.GetOwner() == l.m_TaggedQm)
			return;

		l.m_TaggedQm = best.GetOwner();
		TagEntity(l, best.GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	protected void DespawnBody(RZ_Leader l)
	{
		if (l.m_Body)
			RZ_Util.DeleteUnit(l.m_Body);

		l.m_Body = null;
		l.m_bHasBody = false;
	}

	//------------------------------------------------------------------------------------------------
	//! Stronghold leaders can't die (their bodies are untouchable; if one goes down anyway,
	//! they're "wounded" and back later). Anyone else is dead, and it's news.
	protected void OnBodyKilled(RZ_Leader l)
	{
		int killer = -1;
		if (l.m_iLastShooter > 0 && m_fClock - l.m_fLastShotAt <= BLAME_SECONDS)
			killer = l.m_iLastShooter;

		if (l.IsStronghold())
		{
			DespawnBody(l);
			if (killer > 0)
			{
				l.Remember(Ident(killer), MEM_CROSSED);
				RZ_Deeds.AddKill(m_GM, killer, l.m_eFaction);
				m_GM.AddInfamy(killer, l.m_eFaction, m_Settings.m_iKillInfamy, "killed a leader", RZ_RepFlag.ACT, "shot " + l.m_sName);
			}

			AddHeadline(string.Format("%1 was wounded in %2. They're still in charge.", l.m_sName, l.m_sPlace));
			return;
		}

		string how = "was killed in " + l.m_sPlace;
		if (killer < 0 && m_GM.GetSpawner().IsZombieNear(l.m_vPos, 25))
			how = "was killed by the infected in " + l.m_sPlace;

		Die(l, how, killer);
	}

	// ================================================================ World hooks
	//------------------------------------------------------------------------------------------------
	//! Gunfire near a leader's body (with no infected about) is shooting at them.
	void OnPlayerShot(int playerId, vector pos)
	{
		foreach (RZ_Leader l : m_aList)
		{
			if (!l.IsPresent() || !l.m_Body || RZ_Util.DistXZ(l.m_vPos, pos) > 40)
				continue;

			if (m_GM.GetSpawner().IsZombieNear(l.m_vPos, 30))
				continue;

			l.m_iLastShooter = playerId;
			l.m_fLastShotAt = m_fClock;
			string ident = Ident(playerId);
			if (!l.Remembers(ident, MEM_CROSSED))
			{
				l.Remember(ident, MEM_CROSSED);
				m_GM.NotifyPlayer(playerId, l.GetHeader(), "\"Put it away. Now. I'll remember this.\"");
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Infected killed right at a leader's seat while they're there: whoever did it saved them.
	void OnZombiesKilled(vector pos)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		bool loaded = false;
		foreach (RZ_Leader l : m_aList)
		{
			if (!l.IsPresent() || RZ_Util.DistXZ(l.m_vPos, pos) > 30)
				continue;

			if (!l.m_Body && !l.m_TaggedQm)
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

			string ident = Ident(best);
			if (l.Remembers(ident, MEM_SAVED))
				continue;

			l.Remember(ident, MEM_SAVED);
			m_GM.NotifyPlayer(best, l.GetHeader(), string.Format("%1 lets out a breath. \"That one was at the door. I won't forget it.\"", l.m_sShort));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Samples sold to the Red Hand: Halcyon's Idealists take it personally.
	void OnSamplesSold(int playerId)
	{
		string ident = Ident(playerId);
		foreach (RZ_Leader l : m_aList)
		{
			if (l.IsPresent() && l.m_eFaction == RZ_EFaction.INSTITUTE && l.HasTrait(RZ_ELeaderTrait.IDEALIST))
				l.Remember(ident, MEM_CROSSED);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A contract hurt `faction` (rival Infamy): its nearest leader remembers who did it.
	void OnRivalHurt(RZ_EFaction faction, vector pos, int playerId)
	{
		RZ_Leader l = FindNearest(faction, pos, 3000);
		if (l && playerId > 0)
			l.Remember(Ident(playerId), MEM_CROSSED);
	}

	// ================================================================ Talking
	//------------------------------------------------------------------------------------------------
	//! From RZ_ReadClueAction on a leader ("Talk to ...").
	void HandleTalk(int playerId, int id, IEntity owner)
	{
		RZ_Leader l = Get(id);
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!l || !l.IsPresent() || !player)
			return;

		if (Attitude(l, playerId) <= -2)
		{
			m_GM.Say(playerId, l.GetHeader(), l.m_eFaction, RZ_LeaderVoice.Refuse(l.m_eFaction));
			return;
		}

		string text;
		if (!l.Remembers(Ident(playerId), MEM_MET))
		{
			text = FirstContact(l, playerId);
		}
		else if (RZ_RepVoice.TreesOn())
		{
			// Dialogue > Trees: one opening line in their voice (a deed here, memory, your standing)
			text = RZ_RepVoice.Opening(m_GM, playerId, l) + "\n" + News(l, player.GetOrigin());
		}
		else
		{
			RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
			text = RZ_LeaderVoice.Greeting(rng, l.m_eFaction) + "\n" + News(l, player.GetOrigin());
		}

		// What the faction needs right now, and now and then a big moment they remember
		if (m_GM.GetFactionLife())
		{
			string extra = m_GM.GetFactionLife().TalkExtra(l.m_eFaction, l.m_vPos);
			if (!extra.IsEmpty())
				text += "\n" + extra;
		}

		// A questline that runs through this leader (Director/RZ_Questlines.c)
		if (m_GM.GetQuestlines())
		{
			string quest = m_GM.GetQuestlines().OnLeaderTalk(playerId, l.m_iId);
			if (!quest.IsEmpty())
				text += "\n" + quest;
		}

		RZ_Mission job = FindJob(l);
		if (job && job.IsEligible(m_GM, playerId) && !job.m_aOfferedTo.Contains(playerId))
			text += string.Format("\n%1 lowers their voice. \"I might have something for you. Ask.\"", l.m_sShort);

		m_GM.NotifyPlayer(playerId, l.GetHeader(), text);
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_PuzzleChoiceAction on a leader: 0 work, 1 news, 2 what they think of you.
	void HandleChoice(int playerId, int id, int option, IEntity owner, IEntity user)
	{
		RZ_Leader l = Get(id);
		if (!l || !l.IsPresent() || !user)
			return;

		if (Attitude(l, playerId) <= -2)
		{
			m_GM.Say(playerId, l.GetHeader(), l.m_eFaction, RZ_LeaderVoice.Refuse(l.m_eFaction));
			return;
		}

		if (option == 0)
		{
			OfferWork(l, playerId, user);
			return;
		}

		if (option == 1)
		{
			m_GM.NotifyPlayer(playerId, l.GetHeader(), News(l, user.GetOrigin()));
			return;
		}

		if (option == 2)
			m_GM.NotifyPlayer(playerId, l.GetHeader(), OpinionText(l, playerId));
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_LeaderAction on a quartermaster: the leader whose seat this is.
	void HandleAction(int playerId, RZ_ELeaderAction kind, IEntity owner, IEntity user)
	{
		RZ_Leader l = FindSeat(owner);
		if (!l)
		{
			m_GM.NotifyPlayer(playerId, "Nobody in charge", "Whoever ran this place isn't here. Someone says to come back in a day or two.");
			return;
		}

		if (l.m_iHeldBy >= 0)
		{
			m_GM.NotifyPlayer(playerId, l.GetHeader(), "They're busy with someone from the stronghold. Come back later.");
			return;
		}

		switch (kind)
		{
			case RZ_ELeaderAction.TALK: HandleTalk(playerId, l.m_iId, owner); break;
			case RZ_ELeaderAction.WORK: HandleChoice(playerId, l.m_iId, 0, owner, user); break;
			case RZ_ELeaderAction.NEWS: HandleChoice(playerId, l.m_iId, 1, owner, user); break;
			case RZ_ELeaderAction.STANDING: HandleChoice(playerId, l.m_iId, 2, owner, user); break;
			case RZ_ELeaderAction.SEASON: SeasonPick(playerId, l, 40); break;
			case RZ_ELeaderAction.LOOK: SeasonPick(playerId, l, 41); break;
			case RZ_ELeaderAction.ANSWER_FIRST: SeasonPick(playerId, l, 42); break;
			case RZ_ELeaderAction.ANSWER_OTHER: SeasonPick(playerId, l, 43); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The epilogue's topics and grey choices (RZ_Epilogue.LeaderPick: 40 the season, 41 where to
	//! look, 42/43 the stronghold's question). Refused at attitude -2 or below.
	void SeasonPick(int playerId, RZ_Leader l, int reply)
	{
		if (!l || !RZ_Epilogue.Get())
			return;

		if (Attitude(l, playerId) <= -2)
		{
			m_GM.Say(playerId, l.GetHeader(), l.m_eFaction, RZ_LeaderVoice.Refuse(l.m_eFaction));
			return;
		}

		RZ_Epilogue.Get().LeaderPick(l, playerId, reply);
	}

	//------------------------------------------------------------------------------------------------
	//! The leader whose seat `owner` is at: on a quartermaster, that faction's leader; elsewhere,
	//! whoever's seat is nearest. Null if nobody's there.
	RZ_Leader FindSeat(IEntity owner)
	{
		if (!owner)
			return null;

		int faction = -1;
		RZ_QuartermasterComponent qm = RZ_QuartermasterComponent.Cast(owner.FindComponent(RZ_QuartermasterComponent));
		if (qm)
			faction = qm.GetFaction();

		return FindNearest(faction, owner.GetOrigin(), SEAT_RADIUS);
	}

	//------------------------------------------------------------------------------------------------
	//! What a leader thinks of a player, in a few words (the talk screen's sub-line).
	string AttitudeWords(RZ_Leader l, int playerId)
	{
		int attitude = Attitude(l, playerId);
		if (attitude >= 3)
			return "thinks well of you";
		if (attitude >= 1)
			return "knows your name";
		if (attitude == 0)
			return "doesn't know you yet";
		if (attitude == -1)
			return "has heard things about you";

		return "won't deal with you";
	}

	//------------------------------------------------------------------------------------------------
	protected string OpinionText(RZ_Leader l, int playerId)
	{
		int attitude = Attitude(l, playerId);
		string text = RZ_LeaderVoice.Opinion(attitude);
		int mem = l.GetMemory(Ident(playerId));
		if ((mem & MEM_SAVED) != 0)
			text += " \"You were there when it counted. I don't forget that.\"";
		else if ((mem & MEM_HELPED) != 0)
			text += " \"You've done work for us. It was done properly.\"";

		if ((mem & MEM_CROSSED) != 0)
			text += " \"And I haven't forgotten the other thing.\"";

		if (attitude >= 1)
		{
			string stand = RZ_LeaderVoice.OwnStand(l.m_eFaction, l.m_eLoyalty);
			if (!stand.IsEmpty())
				text += "\n" + stand;
		}

		text += string.Format("\n(You're %1 with %2. %3: %4.)", RZ_Reputation.GetTierName(m_GM.GetTier(playerId, l.m_eFaction)),
			RZ_Factions.GetName(l.m_eFaction), l.m_sShort, l.TraitText());
		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! One piece of news (two from a Talker): other leaders' loyalties and off-book work,
	//! a hungry safehouse, where the noise is.
	protected string News(RZ_Leader l, vector from)
	{
		array<string> lines = {};

		foreach (RZ_Leader other : m_aList)
		{
			if (other == l || !other.IsPresent() || other.m_eLoyalty == RZ_ELoyalty.LOYAL)
				continue;

			string boss = "their own people";
			RZ_Leader top = GetStronghold(other.m_eFaction);
			if (top)
				boss = top.m_sPlace;

			if (other.m_eLoyalty == RZ_ELoyalty.ROGUE)
				lines.Insert(string.Format("\"They say %1 in %2 doesn't answer to %3 any more.\"", other.m_sName, other.m_sPlace, boss));
			else if (FindJob(other))
				lines.Insert(string.Format("\"%1 in %2 has work going that %3 doesn't know about.\"", other.m_sShort, other.m_sPlace, boss));
		}

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
		{
			for (int i = 0; i < houses.Count(); i++)
			{
				RZ_Safehouse h = houses.Get(i);
				if (h.m_bShort)
				{
					lines.Insert(string.Format("\"The safehouse in %1 is out of food.\"", h.m_sName));
					break;
				}
			}
		}

		vector hot;
		float heat;
		if (m_GM.GetHeat().GetHottest(hot, heat) && heat >= 40 && RZ_Util.DistXZ(from, hot) <= 2500)
			lines.Insert(string.Format("\"Something's stirred them up, %1 from here.\"", RZ_Util.DirectionText(from, hot)));

		// Territory and faction news: the latest quiet headline, a rumour (true or not)
		if (m_GM.GetFactionLife())
			m_GM.GetFactionLife().AddNewsLines(l.m_eFaction, from, lines);

		if (lines.IsEmpty())
			return "\"Nothing worth repeating. That's news too, these days.\"";

		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		array<int> order = {};
		foreach (int j, string s : lines)
		{
			order.Insert(j);
		}

		rng.ShuffleInts(order);
		string text = lines[order[0]];
		if (l.HasTrait(RZ_ELeaderTrait.TALKER) && order.Count() > 1)
			text += "\n" + lines[order[1]];

		return text;
	}
}

//------------------------------------------------------------------------------------------------
//! "Talk to whoever's in charge" and friends. Put these on quartermaster prefabs (and on a
//! leader prefab without the puzzle components): they reach the leader whose seat this is.
class RZ_LeaderAction : ScriptedUserAction
{
	[Attribute("0", UIWidgets.ComboBox, "What this action does", "", ParamEnumArray.FromEnum(RZ_ELeaderAction))]
	protected RZ_ELeaderAction m_eKind;

	//------------------------------------------------------------------------------------------------
	//! With the talk screen, work, news and standing are replies there; only Talk stays.
	override bool CanBeShownScript(IEntity user)
	{
		return m_eKind == RZ_ELeaderAction.TALK || !RZ_TalkMenu.Available();
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		switch (m_eKind)
		{
			case RZ_ELeaderAction.TALK: outName = "Talk to whoever's in charge"; break;
			case RZ_ELeaderAction.WORK: outName = "Ask for work"; break;
			case RZ_ELeaderAction.NEWS: outName = "Ask for news"; break;
			case RZ_ELeaderAction.STANDING: outName = "Ask what they think of you"; break;
			case RZ_ELeaderAction.SEASON: outName = "Ask about the season"; break;
			case RZ_ELeaderAction.LOOK: outName = "Ask where to look"; break;
			case RZ_ELeaderAction.ANSWER_FIRST: outName = "Answer their question: the usual way"; break;
			case RZ_ELeaderAction.ANSWER_OTHER: outName = "Answer their question: the other way"; break;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int playerId = RZ_QuartermasterHelper.GetPlayerId(pUserEntity);
		if (!gm || playerId <= 0 || !gm.GetLeaders())
			return;

		// "Talk to whoever's in charge" opens the talk screen with the leader at this seat
		if (m_eKind == RZ_ELeaderAction.TALK && gm.GetDialogue() && gm.GetDialogue().OpenSeat(playerId, pOwnerEntity, pUserEntity))
			return;

		gm.GetLeaders().HandleAction(playerId, m_eKind, pOwnerEntity, pUserEntity);
	}
}
