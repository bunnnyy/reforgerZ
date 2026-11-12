//! Background territory (docs/DESIGN.md, "Factions in the background").
//!
//! Every faction has one Stronghold (where its stronghold leader sits: its quartermaster,
//! Halcyon's lab) that can never be captured or destroyed; at worst it is Under Pressure
//! for a while (no supply trucks leave, the top shop tier is locked), then it recovers.
//! Each faction also holds a few Outposts in towns that suit it. Most of the map stays
//! abandoned: at most about a fifth of all towns are held, and every faction keeps its
//! Stronghold and at least one Outpost.
//!
//! A quiet territory tick every 20-30 minutes does at most ONE thing, chosen by faction
//! strength: a new outpost in a nearby abandoned town, taking one from a weaker neighbour,
//! or abandoning a weak, quiet one. Lost outposts tend to swing back. Changes are headlines
//! only (stored, shown on a player's first spawn and in leaders' news; never broadcast).
//!
//! Holdings are data. Guards, named occupants and an outpost quartermaster are spawned only
//! while a player is near. Each outpost has a leader through RZ_Leaders.RegisterOutpost;
//! with territory on, the leaders' interim posts are replaced by real outposts.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Territory.
[BaseContainerProps()]
class RZ_TerritorySettings
{
	[Attribute("1", desc: "Background territory on: one Stronghold per faction and a few Outposts in towns that suit it. Off = the leaders' interim posts instead")]
	bool m_bEnabled;

	[Attribute("0", UIWidgets.Slider, "Outposts per faction at the start. 0 = by map size (1 under 24 settlements, like Arland; 2 under 48; 3 above, like Everon). Halcyon never has more than 1", "0 4 1")]
	int m_iOutpostsPerFaction;

	[Attribute("0.2", desc: "Expansion stops once this share of all settlements is held (strongholds included). The starting outposts always fit")]
	float m_fHeldShare;

	[Attribute("45", desc: "Shortest real minutes between territory ticks (each does at most one thing; only while players are online; divided by season speed)")]
	float m_fTickMinMinutes;

	[Attribute("75", desc: "Longest real minutes between territory ticks")]
	float m_fTickMaxMinutes;

	[Attribute("25", desc: "Real minutes a stronghold stays Under Pressure (no supply trucks, top shop tier locked)")]
	float m_fPressureMinutes;

	[Attribute("300", desc: "Guards, named occupants and the outpost quartermaster exist only while a player is this close (m)")]
	float m_fVisibleRadius;

	[Attribute("1", UIWidgets.Slider, "Guard bodies (the guard prefab, a character or a group) at each holding while a player is near", "0 4 1")]
	int m_iGuardCount;

	[Attribute("2", UIWidgets.Slider, "Named occupants shown at each holding while a player is near", "0 3 1")]
	int m_iOccupantCount;

	[Attribute("1", desc: "Spawn the faction's quartermaster prefab at its outposts while a player is near (its shop; Bastion's Requisition)")]
	bool m_bOutpostQuartermaster;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Haven guard at Haven holdings (optional): an armed AI character or group on SURV", "et")]
	ResourceName m_sHavenGuardPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bastion guard (optional): an AI character or group on MIL", "et")]
	ResourceName m_sBastionGuardPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Red Hand guard (optional): an AI character or group on BAND", "et")]
	ResourceName m_sRedHandGuardPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Halcyon guard (optional): an AI character or group on INST", "et")]
	ResourceName m_sHalcyonGuardPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Haven occupant (optional; empty = Puzzles > Civilian). Give it the generic puzzle object components to be talked to", "et")]
	ResourceName m_sHavenOccupantPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bastion occupant (optional; empty = Puzzles > Civilian)", "et")]
	ResourceName m_sBastionOccupantPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Red Hand occupant (optional; empty = Puzzles > Civilian)", "et")]
	ResourceName m_sRedHandOccupantPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Halcyon occupant (optional; empty = Puzzles > Civilian)", "et")]
	ResourceName m_sHalcyonOccupantPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Haven guard looks (optional): variant guard groups (or characters) in other clothes; each guard picks one by its holding and place in the row (docs/DESIGN.md, People's looks). Empty = the guard prefab", "et")]
	ref array<ResourceName> m_aHavenGuardLooks;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bastion guard looks (optional)", "et")]
	ref array<ResourceName> m_aBastionGuardLooks;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Red Hand guard looks (optional)", "et")]
	ref array<ResourceName> m_aRedHandGuardLooks;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Halcyon guard looks (optional)", "et")]
	ref array<ResourceName> m_aHalcyonGuardLooks;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Haven occupant looks (optional): variants of the occupant in other clothes; a named person always wears the same one. Empty = the occupant prefab", "et")]
	ref array<ResourceName> m_aHavenOccupantLooks;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bastion occupant looks (optional)", "et")]
	ref array<ResourceName> m_aBastionOccupantLooks;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Red Hand occupant looks (optional)", "et")]
	ref array<ResourceName> m_aRedHandOccupantLooks;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Halcyon occupant looks (optional)", "et")]
	ref array<ResourceName> m_aHalcyonOccupantLooks;

	[Attribute("1", desc: "Haven guard: after Proof's The Whole of It (proof.haven_gate), a Haven group (Haven guard prefab) stands at the lab's gate in the final stand (docs/QUESTLINES.md, The stand's inputs)")]
	bool m_bHavenGuard;
}

//------------------------------------------------------------------------------------------------
class RZ_HoldingSave
{
	int id;
	int faction;
	bool stronghold;
	int settlement = -1;
	float x;
	float z;
	string name;
	float since;
	float traffic;
	ref array<string> people = {};
	ref array<string> roles = {};
}

//------------------------------------------------------------------------------------------------
class RZ_TerritorySave
{
	int seed;
	int nextId = 1;
	float clock;
	float nextTick;
	ref array<float> strength = {};
	ref array<float> pressureUntil = {};
	ref array<float> pressureCooldown = {};
	ref array<ref RZ_HoldingSave> holdings = {};
	ref array<int> lostTown = {};
	ref array<int> lostFaction = {};
	ref array<float> lostAt = {};
}

//------------------------------------------------------------------------------------------------
//! A stronghold or an outpost. Data, plus bodies while a player is near.
class RZ_Holding
{
	int m_iId;
	RZ_EFaction m_eFaction;
	bool m_bStronghold;
	bool m_bFixed;				// a placed RZ_OutpostMarkerEntity: never abandoned or taken
	int m_iSettlement = -1;
	vector m_vPos;
	string m_sName;
	float m_fSince;
	float m_fTraffic;			// player-minutes nearby, decays every territory tick
	int m_iLeaderId = -1;
	ref array<string> m_aPeople = {};	// named guards and occupants
	ref array<string> m_aRoles = {};

	// Runtime
	ref array<IEntity> m_aBodies = {};
	bool m_bSpawned;
	int m_iPhase = -2;

	//------------------------------------------------------------------------------------------------
	//! "Bastion post, Morton"
	string GetTitle()
	{
		return RZ_FactionVoice.HoldingKind(m_eFaction, m_bStronghold) + ", " + m_sName;
	}

	//------------------------------------------------------------------------------------------------
	RZ_HoldingSave ToSave()
	{
		RZ_HoldingSave s = new RZ_HoldingSave();
		s.id = m_iId;
		s.faction = m_eFaction;
		s.stronghold = m_bStronghold;
		s.settlement = m_iSettlement;
		s.x = m_vPos[0];
		s.z = m_vPos[2];
		s.name = m_sName;
		s.since = m_fSince;
		s.traffic = m_fTraffic;
		foreach (int i, string person : m_aPeople)
		{
			s.people.Insert(person);
			if (i < m_aRoles.Count())
				s.roles.Insert(m_aRoles[i]);
			else
				s.roles.Insert("lookout");
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_Holding FromSave(RZ_HoldingSave s)
	{
		RZ_Holding h = new RZ_Holding();
		h.m_iId = s.id;
		h.m_eFaction = s.faction;
		h.m_bStronghold = s.stronghold;
		h.m_iSettlement = s.settlement;
		h.m_vPos = RZ_Util.GroundPos(Vector(s.x, 0, s.z));
		h.m_sName = s.name;
		h.m_fSince = s.since;
		h.m_fTraffic = s.traffic;
		if (s.people && s.roles)
		{
			for (int i = 0; i < s.people.Count() && i < s.roles.Count(); i++)
			{
				h.m_aPeople.Insert(s.people[i]);
				h.m_aRoles.Insert(s.roles[i]);
			}
		}

		return h;
	}
}

//------------------------------------------------------------------------------------------------
//! All holdings and faction strength. Owned by the game mode, ticked on the server tick.
class RZ_Territory
{
	static const int TAG = -6;				// RZ_PuzzleObjectComponent / RZ_ClueComponent mission id for named people

	//! Routine phases at holdings (atmosphere only; RZ_GameTime hours)
	static const int PHASE_NONE = -1;
	static const int PHASE_MORNING = 0;		// 05:30-07:00 shift change
	static const int PHASE_DAY = 1;
	static const int PHASE_EVENING = 2;		// 17:30-18:30 shift change
	static const int PHASE_MEAL = 3;		// 18:30-20:30 the dusk meal
	static const int PHASE_NIGHT = 4;		// 20:30-05:30 night watch

	protected static const float CHECK_INTERVAL = 30;
	protected static const float DESPAWN_MARGIN = 50;
	protected static const float TRAFFIC_RADIUS = 400;
	protected static const float DRIFT_INTERVAL = 3600;
	protected static const float STRENGTH_MIN = 10;
	protected static const float STRENGTH_MAX = 90;
	protected static const float STRENGTH_MID = 50;
	protected static const float CONTRACT_CAP = 6;			// strength from contracts, per faction per hour
	protected static const float SWING_BACK_AFTER = 1800;
	protected static const float LOST_FORGOTTEN = 21600;
	protected static const float PRESSURE_COOLDOWN = 7200;
	protected static const float ROUTINE_RADIUS = 150;
	protected static const float ROUTINE_COOLDOWN = 1200;
	protected static const float TAKE_RANGE = 4000;
	protected static const float NEAR_OWN = 3500;
	protected static const float HUB_RADIUS = 1600;
	protected static const float TOWN_GAP = 400;

	protected RZ_GameModeComponent m_GM;
	protected RZ_TerritorySettings m_Settings;
	protected ref array<ref RZ_Holding> m_aHoldings = {};
	protected ref RZ_TerritorySave m_Restored;
	protected ref array<float> m_aStrength = {50, 50, 50, 50};
	protected ref array<float> m_aPressureUntil = {0, 0, 0, 0};
	protected ref array<float> m_aPressureCooldown = {0, 0, 0, 0};
	protected ref array<float> m_aContractWindow = {0, 0, 0, 0};
	protected ref array<float> m_aContractSum = {0, 0, 0, 0};
	protected ref array<int> m_aLostTown = {};
	protected ref array<int> m_aLostFaction = {};
	protected ref array<float> m_aLostAt = {};
	protected ref array<int> m_aQmTowns;	// settlements with a placed quartermaster (QuartermasterTowns)
	protected ref map<string, float> m_mRoutineTold = new map<string, float>();
	protected int m_iSeed;
	protected int m_iNextId = 1;
	protected float m_fClock;
	protected float m_fNextTick;
	protected float m_fCheckTimer;
	protected float m_fDriftTimer;
	protected bool m_bActive;

	//------------------------------------------------------------------------------------------------
	void RZ_Territory(RZ_GameModeComponent gm, RZ_TerritorySettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	//! On when the setting is on and the map has settlements (after Setup).
	bool IsActive() { return m_bActive; }
	RZ_TerritorySettings GetSettings() { return m_Settings; }
	bool IsEnabled() { return m_Settings.m_bEnabled; }
	float GetClock() { return m_fClock; }
	array<ref RZ_Holding> GetHoldings() { return m_aHoldings; }

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	void Restore(RZ_TerritorySave saved)
	{
		m_Restored = saved;
	}

	//------------------------------------------------------------------------------------------------
	RZ_TerritorySave Save()
	{
		RZ_TerritorySave data = new RZ_TerritorySave();
		data.seed = m_iSeed;
		data.nextId = m_iNextId;
		data.clock = m_fClock;
		data.nextTick = m_fNextTick;
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			data.strength.Insert(m_aStrength[f]);
			data.pressureUntil.Insert(m_aPressureUntil[f]);
			data.pressureCooldown.Insert(m_aPressureCooldown[f]);
		}

		foreach (RZ_Holding h : m_aHoldings)
		{
			// Placed markers come back from the world on every start
			if (!h.m_bFixed)
				data.holdings.Insert(h.ToSave());
		}

		foreach (int i, int town : m_aLostTown)
		{
			data.lostTown.Insert(town);
			data.lostFaction.Insert(m_aLostFaction[i]);
			data.lostAt.Insert(m_aLostAt[i]);
		}

		return data;
	}

	// ================================================================ Setup
	//------------------------------------------------------------------------------------------------
	//! After the leaders (their stronghold seats and the dormant interim posts exist). Saved
	//! holdings come back; on the first run the old interim posts are adopted as outposts,
	//! then each faction is filled up to its starting count.
	void Setup()
	{
		if (!m_Settings.m_bEnabled || m_GM.GetSettlements().IsEmpty())
		{
			RZ_Log.Info("Territory is off. The leaders' interim posts stand in for outposts.");
			return;
		}

		m_bActive = true;
		array<ref RZ_Holding> saved = {};
		if (m_Restored)
		{
			m_iSeed = m_Restored.seed;
			m_iNextId = Math.Max(1, m_Restored.nextId);
			m_fClock = m_Restored.clock;
			m_fNextTick = m_Restored.nextTick;
			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				if (m_Restored.strength && f < m_Restored.strength.Count())
					m_aStrength[f] = Math.Clamp(m_Restored.strength[f], STRENGTH_MIN, STRENGTH_MAX);
				if (m_Restored.pressureUntil && f < m_Restored.pressureUntil.Count())
					m_aPressureUntil[f] = m_Restored.pressureUntil[f];
				if (m_Restored.pressureCooldown && f < m_Restored.pressureCooldown.Count())
					m_aPressureCooldown[f] = m_Restored.pressureCooldown[f];
			}

			if (m_Restored.holdings)
			{
				foreach (RZ_HoldingSave hs : m_Restored.holdings)
				{
					saved.Insert(RZ_Holding.FromSave(hs));
				}
			}

			if (m_Restored.lostTown && m_Restored.lostFaction && m_Restored.lostAt)
			{
				for (int k = 0; k < m_Restored.lostTown.Count() && k < m_Restored.lostFaction.Count() && k < m_Restored.lostAt.Count(); k++)
				{
					m_aLostTown.Insert(m_Restored.lostTown[k]);
					m_aLostFaction.Insert(m_Restored.lostFaction[k]);
					m_aLostAt.Insert(m_Restored.lostAt[k]);
				}
			}
		}

		bool fresh = !m_Restored;
		m_Restored = null;
		if (fresh)
		{
			m_iSeed = Math.RandomInt(1, 2000000000);

			// Season rules: a faction starts a little stronger or weaker (and settles there)
			for (int of = 0; of < RZ_Factions.COUNT; of++)
			{
				m_aStrength[of] = StrengthMid(of);
			}
		}

		if (m_fNextTick <= m_fClock)
			ScheduleTick();

		// Strongholds: where each stronghold leader sits (its quartermaster; Halcyon's lab)
		for (int sf = 0; sf < RZ_Factions.COUNT; sf++)
		{
			vector spos;
			string splace;
			if (!StrongholdSeat(sf, spos, splace))
				continue;

			RZ_Holding top = TakeSaved(saved, sf, true, spos);
			if (!top)
			{
				top = NewHolding(sf, true, m_GM.FindNearestSettlement(spos), spos, splace);
				GeneratePeople(top);
			}

			top.m_vPos = RZ_Util.GroundPos(spos);
			top.m_sName = splace;
			m_aHoldings.Insert(top);
		}

		// Placed markers: fixed outposts (their leaders are the marker seats)
		foreach (RZ_OutpostMarkerEntity marker : RZ_OutpostMarkerEntity.GetAll())
		{
			if (!marker || !RZ_Factions.IsValid(marker.GetFaction()))
				continue;

			vector mpos = RZ_Util.GroundPos(marker.GetOrigin());
			string mname = marker.GetPlaceName();
			if (mname.IsEmpty())
				mname = PlaceName(mpos);

			RZ_Holding fixedOne = AddOutpost(marker.GetFaction(), m_GM.FindNearestSettlement(mpos), mpos, mname, TakeSaved(saved, marker.GetFaction(), false, mpos));
			fixedOne.m_bFixed = true;
		}

		string source = "saved";
		if (!saved.IsEmpty())
		{
			// Saved outposts keep their towns, people and leaders
			foreach (RZ_Holding old : saved)
			{
				if (old.m_bStronghold || IsHoldingNear(old.m_vPos, 150))
					continue;

				AddOutpost(old.m_eFaction, old.m_iSettlement, old.m_vPos, old.m_sName, old);
			}
		}
		else if (fresh)
		{
			source = string.Format("new, seed %1", m_iSeed);
			AdoptInterimPosts();
		}

		// Everyone up to their starting count (and never below one outpost)
		for (int ff = 0; ff < RZ_Factions.COUNT; ff++)
		{
			int guard = 0;
			while (CountOutposts(ff) < BaseOutposts(ff) && guard < 4)
			{
				guard++;
				if (!Found(ff, true, false))
					break;
			}
		}

		RZ_Log.Info(string.Format("Territory: %1 holdings in %2 settlements (%3). %4", m_aHoldings.Count(), m_GM.GetSettlements().Count(), source, DescribeAll()));
	}

	//------------------------------------------------------------------------------------------------
	//! The saved holding of this faction and kind near pos that isn't in use yet, else null.
	//! (It stays in `saved`; the restore loop skips anything already standing there.)
	protected RZ_Holding TakeSaved(array<ref RZ_Holding> saved, RZ_EFaction f, bool stronghold, vector pos)
	{
		foreach (RZ_Holding h : saved)
		{
			if (h.m_eFaction != f || h.m_bStronghold != stronghold || m_aHoldings.Contains(h))
				continue;

			if (!stronghold && RZ_Util.DistXZ(h.m_vPos, pos) > 150)
				continue;

			return h;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! First run with territory on an old save: the leaders' interim posts become outposts,
	//! so their leaders (names, loyalty, memories) keep their seats.
	protected void AdoptInterimPosts()
	{
		RZ_Leaders leaders = m_GM.GetLeaders();
		if (!leaders)
			return;

		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			array<RZ_Leader> dormant = {};
			leaders.GetDormantOutposts(f, dormant);
			foreach (RZ_Leader l : dormant)
			{
				if (CountOutposts(f) >= BaseOutposts(f))
					break;

				if (IsHoldingNear(l.m_vPos, TOWN_GAP) || RZ_Perimeter.IsNearLineAt(l.m_vPos))
					continue;

				AddOutpost(f, m_GM.FindNearestSettlement(l.m_vPos), l.m_vPos, l.m_sPlace, null);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Where a faction's stronghold is: its stronghold leader's seat, else (leaders off) its
	//! quartermaster, Halcyon's lab, the first safehouse for the Haven, or a town that suits it.
	protected bool StrongholdSeat(RZ_EFaction f, out vector pos, out string place)
	{
		RZ_Leaders leaders = m_GM.GetLeaders();
		if (leaders)
		{
			RZ_Leader top = leaders.GetStronghold(f);
			if (top)
			{
				pos = top.m_vPos;
				place = top.m_sPlace;
				return true;
			}
		}

		bool found = m_GM.GetPerks().GetQuartermasterPos(f, pos);
		if (!found && f == RZ_EFaction.INSTITUTE && m_GM.HasLab())
		{
			pos = m_GM.GetLabPosition();
			found = true;
		}

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!found && f == RZ_EFaction.SURVIVORS && houses && houses.Count() > 0)
		{
			pos = houses.Get(0).m_vPos;
			found = true;
		}

		if (!found)
		{
			int idx = PickTown(f, false);
			RZ_Settlement s = m_GM.GetSettlement(idx);
			if (!s)
				return false;

			pos = RZ_Mission.LandPointNear(s.m_vCenter, 5, Math.Max(15, s.m_fRadius * 0.3));
		}

		place = PlaceName(pos);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Holding NewHolding(RZ_EFaction f, bool stronghold, int settlement, vector pos, string place)
	{
		RZ_Holding h = new RZ_Holding();
		h.m_iId = m_iNextId;
		m_iNextId++;
		h.m_eFaction = f;
		h.m_bStronghold = stronghold;
		h.m_iSettlement = settlement;
		h.m_vPos = RZ_Util.GroundPos(pos);
		h.m_sName = place;
		h.m_fSince = m_fClock;
		return h;
	}

	//------------------------------------------------------------------------------------------------
	//! A new outpost (or `existing`, restored), with its leader registered. Supply state is
	//! created on demand by the supply lines.
	protected RZ_Holding AddOutpost(RZ_EFaction f, int settlement, vector pos, string place, RZ_Holding existing)
	{
		RZ_Holding h = existing;
		if (!h)
		{
			h = NewHolding(f, false, settlement, pos, place);
			GeneratePeople(h);
		}

		h.m_eFaction = f;
		h.m_bStronghold = false;
		h.m_vPos = RZ_Util.GroundPos(pos);
		h.m_iSettlement = settlement;
		if (h.m_aPeople.IsEmpty())
			GeneratePeople(h);

		m_aHoldings.Insert(h);
		RegisterLeader(h);
		return h;
	}

	//------------------------------------------------------------------------------------------------
	//! The outpost's leader sits where its people spawn: the yard of the building camp life moves it
	//! into (RZ_CampLife.HomeFor), else the town's middle. A saved leader is moved there too.
	protected void RegisterLeader(RZ_Holding h)
	{
		h.m_iLeaderId = -1;
		RZ_Leaders leaders = m_GM.GetLeaders();
		if (!leaders)
			return;

		vector seat = h.m_vPos;
		RZ_CampLife camp = m_GM.GetCampLife();
		if (camp)
			seat = camp.HomeFor("h" + h.m_iId.ToString(), h.m_eFaction, RZ_ECampPlace.OUTPOST, h.m_vPos);

		h.m_iLeaderId = leaders.RegisterOutpost(h.m_eFaction, seat, h.m_sName, false);
		RZ_Leader l = leaders.Get(h.m_iLeaderId);
		if (l && l.m_iSource != RZ_Leader.SOURCE_MARKER)
			l.m_vPos = RZ_Util.GroundPos(seat);
	}

	//------------------------------------------------------------------------------------------------
	//! A named guard or occupant died (RZ_ToneGuard.NamedDeaths): a newcomer takes the role, under a
	//! new name, so the dead one isn't back at the next spawn. The dead one's name, "" if none.
	string ReplacePerson(int holdingId, int idx)
	{
		RZ_Holding h = GetHolding(holdingId);
		if (!h || idx < 0 || idx >= h.m_aPeople.Count())
			return "";

		string dead = h.m_aPeople[idx];
		RZ_Rng rng = new RZ_Rng(m_iSeed + h.m_iId * 6007 + idx * 131 + RZ_Util.RoundToInt(m_fClock));
		string name = RZ_FactionVoice.PersonName(rng, h.m_eFaction);
		if (h.m_aPeople.Contains(name) || name == dead)
			name = RZ_FactionVoice.PersonName(rng, h.m_eFaction);

		h.m_aPeople[idx] = name;
		return dead;
	}

	//------------------------------------------------------------------------------------------------
	//! Two or three named guards and occupants, seeded from the holding.
	protected void GeneratePeople(RZ_Holding h)
	{
		h.m_aPeople.Clear();
		h.m_aRoles.Clear();
		RZ_Rng rng = new RZ_Rng(m_iSeed + h.m_iId * 6007 + h.m_eFaction * 101 + RZ_Util.RoundToInt(m_fClock));
		int count = 3;
		for (int i = 0; i < count; i++)
		{
			string name = RZ_FactionVoice.PersonName(rng, h.m_eFaction);
			if (h.m_aPeople.Contains(name))
				name = RZ_FactionVoice.PersonName(rng, h.m_eFaction);

			h.m_aPeople.Insert(name);
			h.m_aRoles.Insert(RZ_FactionVoice.PersonRole(rng, h.m_eFaction));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string PlaceName(vector pos)
	{
		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(pos));
		if (s)
			return s.m_sName;

		return RZ_SettlementScanner.GridReference(pos);
	}

	//------------------------------------------------------------------------------------------------
	protected string DescribeAll()
	{
		string text = "";
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (f > 0)
				text += ", ";

			text += string.Format("%1 1+%2", RZ_Factions.GetShortName(f), CountOutposts(f));
		}

		return text;
	}

	// ================================================================ Sizes and caps
	//------------------------------------------------------------------------------------------------
	//! Starting outposts: 1 on a small map, 2 on a mid-sized one, 3 on a large one. Halcyon 1.
	int BaseOutposts(RZ_EFaction f)
	{
		int n = m_Settings.m_iOutpostsPerFaction;
		if (n <= 0)
		{
			int towns = m_GM.GetSettlements().Count();
			n = 3;
			if (towns < 24)
				n = 1;
			else if (towns < 48)
				n = 2;
		}

		n = Math.ClampInt(n, 1, 4);
		if (f == RZ_EFaction.INSTITUTE)
			return 1;

		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected int MaxOutposts(RZ_EFaction f)
	{
		if (f == RZ_EFaction.INSTITUTE)
			return 1;

		return BaseOutposts(f) + 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Most holdings (strongholds included) the map carries before expansion stops.
	protected int HeldCap()
	{
		int start = RZ_Factions.COUNT;
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			start += BaseOutposts(f);
		}

		int share = RZ_Util.RoundToInt(m_GM.GetSettlements().Count() * Math.Clamp(m_Settings.m_fHeldShare, 0.05, 0.5));
		return Math.Max(start, share);
	}

	//------------------------------------------------------------------------------------------------
	int CountOutposts(int f)
	{
		int n = 0;
		foreach (RZ_Holding h : m_aHoldings)
		{
			if (!h.m_bStronghold && h.m_eFaction == f)
				n++;
		}

		return n;
	}

	// ================================================================ Lookups
	//------------------------------------------------------------------------------------------------
	RZ_Holding GetHolding(int id)
	{
		foreach (RZ_Holding h : m_aHoldings)
		{
			if (h.m_iId == id)
				return h;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	RZ_Holding GetStronghold(int f)
	{
		foreach (RZ_Holding h : m_aHoldings)
		{
			if (h.m_bStronghold && h.m_eFaction == f)
				return h;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	void GetOutposts(int f, notnull array<RZ_Holding> result)
	{
		foreach (RZ_Holding h : m_aHoldings)
		{
			if (!h.m_bStronghold && (f < 0 || h.m_eFaction == f))
				result.Insert(h);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Nearest holding of `faction` (any if -1) within radius of pos; outposts only if outpostsOnly.
	RZ_Holding FindHolding(int faction, vector pos, float radius, bool outpostsOnly = false)
	{
		RZ_Holding best;
		float bestDist = radius;
		foreach (RZ_Holding h : m_aHoldings)
		{
			if (faction >= 0 && h.m_eFaction != faction)
				continue;

			if (outpostsOnly && h.m_bStronghold)
				continue;

			float d = RZ_Util.DistXZ(h.m_vPos, pos);
			if (d <= bestDist)
			{
				bestDist = d;
				best = h;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsHoldingNear(vector pos, float dist)
	{
		return FindHolding(-1, pos, dist) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! A holding in this settlement, null if the town is abandoned.
	RZ_Holding FindInTown(int settlement)
	{
		RZ_Settlement s = m_GM.GetSettlement(settlement);
		foreach (RZ_Holding h : m_aHoldings)
		{
			if (h.m_iSettlement == settlement)
				return h;

			if (s && RZ_Util.DistXZ(h.m_vPos, s.m_vCenter) < Math.Max(TOWN_GAP, s.m_fRadius))
				return h;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected float DistToOwn(RZ_EFaction f, vector pos)
	{
		float best = 1000000;
		foreach (RZ_Holding h : m_aHoldings)
		{
			if (h.m_eFaction != f)
				continue;

			float d = RZ_Util.DistXZ(h.m_vPos, pos);
			if (d < best)
				best = d;
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	float GetStrength(int f)
	{
		if (!RZ_Factions.IsValid(f))
			return STRENGTH_MID;

		return m_aStrength[f];
	}

	//------------------------------------------------------------------------------------------------
	bool IsUnderPressure(int f)
	{
		if (!m_bActive || !RZ_Factions.IsValid(f))
			return false;

		return m_fClock < m_aPressureUntil[f];
	}

	//------------------------------------------------------------------------------------------------
	//! "Morton and Levie" for leaders' and chatter lines. "" if the faction has no outposts.
	string DescribeOutposts(RZ_EFaction f)
	{
		string text = "";
		foreach (RZ_Holding h : m_aHoldings)
		{
			if (h.m_bStronghold || h.m_eFaction != f)
				continue;

			if (!text.IsEmpty())
				text += ", ";

			text += h.m_sName;
		}

		return text;
	}

	// ================================================================ Choosing towns
	//------------------------------------------------------------------------------------------------
	//! How well a town suits a faction (0 = no): Bastion likes towns on roads and airfields,
	//! the Red Hand crossroads and outskirts, the Haven quiet villages, Halcyon quiet places.
	//! "On the roads" is read from how many towns are near (a hub), which works on any map.
	protected float TownScore(RZ_EFaction f, int idx, RZ_Settlement s)
	{
		if (FindInTown(idx) || IsHoldingNear(s.m_vCenter, TOWN_GAP))
			return 0;

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses && houses.IsSafehouseTown(idx))
			return 0;

		if (idx == m_GM.GetLabSettlement() || s.m_fInfection >= 85 || RZ_Perimeter.IsNearLineAt(s.m_vCenter))
			return 0;

		// Nobody sets up next door to the Long Table's farm (World/RZ_LongTable.c)
		if (m_GM.GetLongTable() && m_GM.GetLongTable().IsNearHouse(s.m_vCenter, 1500))
			return 0;

		// A placed quartermaster's town is some faction's stronghold, even before it is seated
		if (QuartermasterTowns().Contains(idx))
			return 0;

		int hub = Neighbours(idx);
		string lower = s.m_sName;
		lower.ToLower();
		float score = 0.2;
		switch (f)
		{
			case RZ_EFaction.MILITARY:
			{
				if (s.m_iSize >= 2)
					score += 2;
				score += hub * 0.5;
				if (lower.Contains("air") || lower.Contains("base") || lower.Contains("field") || lower.Contains("port"))
					score += 3;
				if (s.m_fInfection <= 60)
					score += 0.5;
				break;
			}
			case RZ_EFaction.BANDITS:
			{
				score += hub;
				if (s.m_iSize == 1)
					score += 1;
				if (s.m_fInfection >= 15)
					score += 0.5;
				// Its fed pack in the woods between the town and Bastion (the fat moat)
				if (m_GM.GetTribes())
					score = score * m_GM.GetTribes().GetPeople().CampScore(s.m_vCenter);
				break;
			}
			case RZ_EFaction.SURVIVORS:
			{
				if (s.m_iSize == 1)
					score += 2;
				if (s.m_fInfection < 30)
					score += 2;
				else if (s.m_fInfection < 50)
					score += 1;
				if (hub <= 1)
					score += 1;
				if (DistToOwn(RZ_EFaction.BANDITS, s.m_vCenter) > 1500)
					score += 1;
				break;
			}
			case RZ_EFaction.INSTITUTE:
			{
				if (s.m_fInfection < 40)
					score += 2;
				if (s.m_iSize == 1)
					score += 1;
				if (hub <= 2)
					score += 1;
				break;
			}
		}

		// Not on top of their own stronghold, not on the far side of the map
		RZ_Holding top = GetStronghold(f);
		if (top)
		{
			float d = RZ_Util.DistXZ(top.m_vPos, s.m_vCenter);
			if (d < 700)
				score = score * 0.2;
			else if (d > 6000)
				score = score * 0.6;
		}

		return score;
	}

	//------------------------------------------------------------------------------------------------
	//! Settlements within HUB_RADIUS: a rough "how many roads meet here".
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
	//! Settlements with a placed quartermaster in them (worked out once: quartermasters don't move).
	protected array<int> QuartermasterTowns()
	{
		if (m_aQmTowns)
			return m_aQmTowns;

		m_aQmTowns = {};
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			vector qm;
			if (m_GM.GetPerks().GetQuartermasterPos(f, qm))
				m_aQmTowns.Insert(m_GM.FindNearestSettlement(qm));
		}

		return m_aQmTowns;
	}

	//------------------------------------------------------------------------------------------------
	//! A weighted pick among the towns that suit the faction. nearOwn favours towns near its holdings.
	protected int PickTown(RZ_EFaction f, bool nearOwn)
	{
		array<int> options = {};
		array<float> weights = {};
		float total = 0;
		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			float score = TownScore(f, i, s);
			if (score <= 0)
				continue;

			if (nearOwn)
			{
				if (DistToOwn(f, s.m_vCenter) <= NEAR_OWN)
					score = score * 1.5;
				else
					score = score * 0.6;
			}

			options.Insert(i);
			weights.Insert(score);
			total += score;
		}

		if (options.IsEmpty())
			return -1;

		float roll = Math.RandomFloat(0, total);
		foreach (int k, int option : options)
		{
			roll -= weights[k];
			if (roll <= 0)
				return option;
		}

		return options[options.Count() - 1];
	}

	// ================================================================ Strength
	//------------------------------------------------------------------------------------------------
	//! Where a faction's strength settles: the middle, moved by the season's rules (at most 20).
	protected float StrengthMid(int f)
	{
		return Math.Clamp(STRENGTH_MID + RZ_Seasons.Mods().StrengthOffset(f), STRENGTH_MIN + 10, STRENGTH_MAX - 10);
	}

	//------------------------------------------------------------------------------------------------
	//! Faction strength 10..90 (starts at 50, drifts back to the middle).
	void AddStrength(int f, float delta)
	{
		if (!m_bActive || !RZ_Factions.IsValid(f))
			return;

		m_aStrength[f] = Math.Clamp(m_aStrength[f] + delta, STRENGTH_MIN, STRENGTH_MAX);
	}

	//------------------------------------------------------------------------------------------------
	//! Contracts move strength a little: +2 for the giver on success, -1 for a rival it hurt,
	//! -1 on failure. At most 6 points an hour per faction from contracts.
	void OnContractFinished(RZ_Mission m, bool success)
	{
		if (!m_bActive || !m || !m.m_bContract || !m.m_bHasGiver)
			return;

		if (!success)
		{
			ContractStrength(m.m_eGiver, -1);
			return;
		}

		ContractStrength(m.m_eGiver, 2);
		if (m.m_bHasRival && m.m_iRivalInfamy > 0)
			ContractStrength(m.m_eRival, -1);

		// Work done near an outpost counts as traffic there
		RZ_Holding h = FindHolding(m.m_eGiver, m.m_vPos, 1500, true);
		if (h)
			h.m_fTraffic = h.m_fTraffic + 5;
	}

	//------------------------------------------------------------------------------------------------
	protected void ContractStrength(RZ_EFaction f, float delta)
	{
		if (m_fClock - m_aContractWindow[f] > DRIFT_INTERVAL)
		{
			m_aContractWindow[f] = m_fClock;
			m_aContractSum[f] = 0;
		}

		float room = CONTRACT_CAP - Math.AbsFloat(m_aContractSum[f]);
		if (room <= 0)
			return;

		float applied = Math.Clamp(delta, -room, room);
		// The contracts and errands lane of the season budget (Director/RZ_MajorBudget.c)
		RZ_MajorBudget budget = RZ_MajorBudget.Get();
		if (budget)
			applied = budget.TakeStrength(RZ_EBudgetLane.CONTRACTS, f, applied, "", "contract");

		if (applied == 0)
			return;

		m_aContractSum[f] = m_aContractSum[f] + applied;
		AddStrength(f, applied);
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bActive)
			return;

		m_fClock += dt;
		UpdateBodies(players);

		m_fCheckTimer += dt;
		if (m_fCheckTimer >= CHECK_INTERVAL)
		{
			m_fCheckTimer = 0;
			CountTraffic(players);
			Routines(ids, players);
		}

		UpdatePressure();

		// The territory only moves while someone is playing
		if (ids.IsEmpty())
		{
			if (m_fNextTick < m_fClock + 60)
				m_fNextTick = m_fClock + 60;

			return;
		}

		m_fDriftTimer += dt;
		if (m_fDriftTimer >= DRIFT_INTERVAL)
		{
			m_fDriftTimer = 0;
			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				float mid = StrengthMid(f);
				if (m_aStrength[f] > mid + 1)
					AddStrength(f, -2);
				else if (m_aStrength[f] < mid - 1)
					AddStrength(f, 2);
			}
		}

		if (m_fClock >= m_fNextTick)
		{
			ScheduleTick();
			TerritoryTick(players);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ScheduleTick()
	{
		float lo = Math.Max(1, m_Settings.m_fTickMinMinutes);
		float hi = Math.Max(lo, m_Settings.m_fTickMaxMinutes);
		m_fNextTick = m_fClock + Math.RandomFloat(lo, hi) * 60 / m_GM.GetSeasonSpeed();
	}

	//------------------------------------------------------------------------------------------------
	//! At most ONE change: the floor first, then each faction in random order by its strength.
	protected void TerritoryTick(array<IEntity> players)
	{
		foreach (RZ_Holding h : m_aHoldings)
		{
			h.m_fTraffic = h.m_fTraffic * 0.75;
		}

		StartPressureIfWeak();
		ForgetOldLosses();

		// Floor: every faction keeps at least one outpost
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (CountOutposts(f) < 1 && Found(f, true, true))
				return;
		}

		// The world settles in the epilogue: only the floor is kept (docs/EPILOGUE.md)
		if (RZ_WorldFacts.Get().HoldingsFrozen())
			return;

		array<int> order = {0, 1, 2, 3};
		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		rng.ShuffleInts(order);
		foreach (int faction : order)
		{
			if (Decide(faction, players))
				return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool Decide(RZ_EFaction f, array<IEntity> players)
	{
		float s = m_aStrength[f];
		int count = CountOutposts(f);
		bool below = count < BaseOutposts(f);
		bool room = count < MaxOutposts(f) && m_aHoldings.Count() < HeldCap();

		if (s >= 60)
		{
			if (TrySwingBack(f, players))
				return true;

			if ((below || (room && Math.RandomFloat01() < 0.5)) && Found(f, true, true))
				return true;

			if (s >= 65 && TryTake(f, players))
				return true;

			return false;
		}

		if (s <= 40)
		{
			if (count > 1 && Math.RandomFloat01() < 0.6)
				return TryAbandon(f, false, players);

			return false;
		}

		if (below && Math.RandomFloat01() < 0.5)
			return Found(f, true, true);

		if (count > 1 && Math.RandomFloat01() < 0.2)
			return TryAbandon(f, true, players);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A new outpost in an abandoned town that suits the faction.
	protected bool Found(RZ_EFaction f, bool nearOwn, bool headline)
	{
		int idx = PickTown(f, nearOwn);
		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (!s)
			return false;

		return FoundAt(f, idx, headline);
	}

	//------------------------------------------------------------------------------------------------
	protected bool FoundAt(RZ_EFaction f, int idx, bool headline)
	{
		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (!s)
			return false;

		vector pos = RZ_Mission.LandPointNear(s.m_vCenter, 5, Math.Max(15, s.m_fRadius * 0.4));
		RZ_Holding h = AddOutpost(f, idx, pos, s.m_sName, null);
		if (!headline)
			return true;

		Headline(RZ_FactionVoice.Founded(f, h.m_sName, LeaderName(h)));
		RZ_FactionLife life = m_GM.GetFactionLife();
		if (life)
			life.OnOutpostGained(f, h.m_sName);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Take the nearest outpost of a weaker neighbour (not the Haven or Halcyon, who don't
	//! conquer; never below the other faction's floor; never during a truce).
	protected bool TryTake(RZ_EFaction f, array<IEntity> players)
	{
		if (f == RZ_EFaction.SURVIVORS || f == RZ_EFaction.INSTITUTE)
			return false;

		if (CountOutposts(f) >= MaxOutposts(f))
			return false;

		RZ_FactionLife life = m_GM.GetFactionLife();
		RZ_Holding best;
		float bestDist = TAKE_RANGE;
		foreach (RZ_Holding h : m_aHoldings)
		{
			if (!CanChangeHands(h, f, players))
				continue;

			float margin = 10;
			if (life && life.HasFeud(f, h.m_eFaction))
				margin = 0;

			if (m_aStrength[h.m_eFaction] > m_aStrength[f] - margin)
				continue;

			float d = DistToOwn(f, h.m_vPos);
			if (d < bestDist)
			{
				bestDist = d;
				best = h;
			}
		}

		if (!best)
			return false;

		// The moat rule: a take across the ground of a pack whose leader is in its prime or older
		// waits; the third time, they find a way round (docs/INFECTED.md, The factions)
		RZ_Holding from = FindHolding(f, best.m_vPos, 100000);
		RZ_Tribes tribes = m_GM.GetTribes();
		if (from && tribes && tribes.GetPeople().BlocksTake(best.m_iId, from.m_vPos, best.m_vPos))
			return false;

		TakeOver(best, f);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! An outpost that can go to `taker` now: not a stronghold or marker, someone else's, not the
	//! owner's last one, no truce between them, and nobody watching.
	protected bool CanChangeHands(RZ_Holding h, RZ_EFaction taker, array<IEntity> players)
	{
		if (h.m_bStronghold || h.m_bFixed || h.m_eFaction == taker)
			return false;

		if (CountOutposts(h.m_eFaction) <= 1)
			return false;

		RZ_FactionLife life = m_GM.GetFactionLife();
		if (life && life.HasTruce(taker, h.m_eFaction))
			return false;

		return RZ_Util.NearestDistance(h.m_vPos, players) > m_Settings.m_fVisibleRadius + 100;
	}

	//------------------------------------------------------------------------------------------------
	//! A story hands an outpost to another faction (a questline's ending). The same rules as the
	//! territory tick: never a stronghold or a placed marker, never the owner's last outpost, not
	//! during a truce, and only while no player is watching. False if it can't happen now.
	bool HandOver(int holdingId, RZ_EFaction taker)
	{
		RZ_Holding h = GetHolding(holdingId);
		if (!m_bActive || !h || !RZ_Factions.IsValid(taker))
			return false;

		// The world settles in the epilogue: an ending that would hand over an outpost is words only
		if (RZ_WorldFacts.Get().HoldingsFrozen())
		{
			RZ_Log.Info(string.Format("Territory: %1 stays as it is (the world settles in the epilogue).", h.m_sName));
			return false;
		}

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		if (!CanChangeHands(h, taker, players))
			return false;

		TakeOver(h, taker);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void TakeOver(RZ_Holding h, RZ_EFaction taker)
	{
		RZ_EFaction loser = h.m_eFaction;
		RZ_Leaders leaders = m_GM.GetLeaders();
		if (leaders)
			leaders.UnregisterOutpost(h.m_iLeaderId, "");

		DespawnBodies(h);
		h.m_eFaction = taker;
		h.m_fSince = m_fClock;
		h.m_fTraffic = 0;
		GeneratePeople(h);
		RegisterLeader(h);

		m_aLostTown.Insert(h.m_iSettlement);
		m_aLostFaction.Insert(loser);
		m_aLostAt.Insert(m_fClock);
		AddStrength(loser, -5);
		AddStrength(taker, 3);

		Headline(RZ_FactionVoice.Taken(taker, loser, h.m_sName));
		RZ_WorldWriters.HoldingTaken(h.m_iSettlement, taker, loser, h.m_sName, h.m_vPos);
		RZ_FactionLife life = m_GM.GetFactionLife();
		if (life)
		{
			life.OnOutpostLost(loser, h.m_sName, taker);
			life.OnOutpostGained(taker, h.m_sName);
		}

		// Wanderers avoid the place for a while and talk about it
		RZ_Wanderers wanderers = m_GM.GetWanderers();
		if (wanderers)
			wanderers.OnOutpostLost(loser, taker, h.m_vPos, h.m_sName);

		RZ_SupplyLines supply = m_GM.GetSupply();
		if (supply)
			supply.OnHoldingChanged(h.m_iId);
	}

	//------------------------------------------------------------------------------------------------
	//! A strong faction tends to win back a town it lost: re-found it if it's empty again, or
	//! take it from a weaker holder (not the Haven or Halcyon, who never take towns).
	protected bool TrySwingBack(RZ_EFaction f, array<IEntity> players)
	{
		if (CountOutposts(f) >= MaxOutposts(f))
			return false;

		for (int i = m_aLostTown.Count() - 1; i >= 0; i--)
		{
			if (m_aLostFaction[i] != f || m_fClock - m_aLostAt[i] < SWING_BACK_AFTER)
				continue;

			int town = m_aLostTown[i];
			RZ_Holding holder = FindInTown(town);
			if (!holder)
			{
				RZ_Settlement s = m_GM.GetSettlement(town);
				if (!s || TownScore(f, town, s) <= 0)
					continue;

				RemoveLoss(i);
				return FoundAt(f, town, true);
			}

			if (f == RZ_EFaction.SURVIVORS || f == RZ_EFaction.INSTITUTE)
				continue;

			if (!CanChangeHands(holder, f, players) || m_aStrength[holder.m_eFaction] >= m_aStrength[f])
				continue;

			RemoveLoss(i);
			TakeOver(holder, f);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveLoss(int i)
	{
		m_aLostTown.Remove(i);
		m_aLostFaction.Remove(i);
		m_aLostAt.Remove(i);
	}

	//------------------------------------------------------------------------------------------------
	protected void ForgetOldLosses()
	{
		for (int i = m_aLostTown.Count() - 1; i >= 0; i--)
		{
			if (m_fClock - m_aLostAt[i] > LOST_FORGOTTEN)
				RemoveLoss(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Drop the quietest outpost. strict: only one with almost no traffic that has stood 3 hours.
	protected bool TryAbandon(RZ_EFaction f, bool strict, array<IEntity> players)
	{
		if (CountOutposts(f) <= 1)
			return false;

		RZ_Holding weakest;
		float lowest = 1000000;
		foreach (RZ_Holding h : m_aHoldings)
		{
			if (h.m_bStronghold || h.m_bFixed || h.m_eFaction != f)
				continue;

			if (m_fClock - h.m_fSince < 3600)
				continue;

			if (RZ_Util.NearestDistance(h.m_vPos, players) <= m_Settings.m_fVisibleRadius + 100)
				continue;

			if (h.m_fTraffic < lowest)
			{
				lowest = h.m_fTraffic;
				weakest = h;
			}
		}

		if (!weakest)
			return false;

		if (strict && (weakest.m_fTraffic >= 2 || m_fClock - weakest.m_fSince < 10800))
			return false;

		if (!strict && weakest.m_fTraffic >= 15)
			return false;

		Abandon(weakest);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Abandon(RZ_Holding h)
	{
		RZ_Leaders leaders = m_GM.GetLeaders();
		if (leaders)
			leaders.UnregisterOutpost(h.m_iLeaderId, "");

		DespawnBodies(h);
		Headline(RZ_FactionVoice.Abandoned(h.m_eFaction, h.m_sName));
		RZ_FactionLife life = m_GM.GetFactionLife();
		if (life)
			life.AddMorale(h.m_eFaction, -2);

		RZ_Wanderers wanderers = m_GM.GetWanderers();
		if (wanderers)
			wanderers.OnOutpostLost(h.m_eFaction, -1, h.m_vPos, h.m_sName);

		RZ_SupplyLines supply = m_GM.GetSupply();
		if (supply)
			supply.OnHoldingChanged(h.m_iId);

		m_aHoldings.RemoveItem(h);
	}

	//------------------------------------------------------------------------------------------------
	protected string LeaderName(RZ_Holding h)
	{
		RZ_Leaders leaders = m_GM.GetLeaders();
		if (!leaders)
			return "";

		RZ_Leader l = leaders.Get(h.m_iLeaderId);
		if (!l)
			return "";

		return l.m_sName;
	}

	//------------------------------------------------------------------------------------------------
	//! Stored in the headlines (first spawn, leaders' news); never broadcast.
	protected void Headline(string text)
	{
		RZ_Log.Info("Territory: " + text);
		if (m_GM.GetLeaders())
			m_GM.GetLeaders().AddHeadline(text, false);
	}

	// ================================================================ Under Pressure
	//------------------------------------------------------------------------------------------------
	protected void StartPressureIfWeak()
	{
		RZ_FactionLife life = m_GM.GetFactionLife();
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (IsUnderPressure(f) || m_fClock < m_aPressureCooldown[f])
				continue;

			float morale = 50;
			if (life)
				morale = life.GetMorale(f);

			if ((m_aStrength[f] > 25 && morale > 20) || Math.RandomFloat01() >= 0.5)
				continue;

			m_aPressureUntil[f] = m_fClock + Math.Max(5, m_Settings.m_fPressureMinutes) * 60;
			RZ_Holding top = GetStronghold(f);
			if (top)
				Headline(RZ_FactionVoice.PressureStart(f, top.m_sName));

			if (life)
				life.AddMorale(f, -5);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdatePressure()
	{
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (m_aPressureUntil[f] <= 0 || m_fClock < m_aPressureUntil[f])
				continue;

			m_aPressureUntil[f] = 0;
			m_aPressureCooldown[f] = m_fClock + PRESSURE_COOLDOWN;
			AddStrength(f, 5);
			RZ_Holding top = GetStronghold(f);
			if (top)
				Headline(RZ_FactionVoice.PressureEnd(f, top.m_sName));

			RZ_FactionLife life = m_GM.GetFactionLife();
			if (life)
				life.AddMorale(f, 5);
		}
	}

	// ================================================================ Traffic and routines
	//------------------------------------------------------------------------------------------------
	protected void CountTraffic(array<IEntity> players)
	{
		foreach (RZ_Holding h : m_aHoldings)
		{
			int n = RZ_Mission.CountPlayersNear(players, h.m_vPos, TRAFFIC_RADIUS);
			if (n > 0)
				h.m_fTraffic = Math.Min(500, h.m_fTraffic + n * CHECK_INTERVAL / 60);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! 05:30 and 17:30 shift change, 18:30 the dusk meal, 20:30 night watch. -1 without a clock.
	static int RoutinePhase(float hour)
	{
		if (hour < 0)
			return PHASE_NONE;

		if (hour >= 5.5 && hour < 7)
			return PHASE_MORNING;
		if (hour >= 7 && hour < 17.5)
			return PHASE_DAY;
		if (hour >= 17.5 && hour < 18.5)
			return PHASE_EVENING;
		if (hour >= 18.5 && hour < 20.5)
			return PHASE_MEAL;

		return PHASE_NIGHT;
	}

	//------------------------------------------------------------------------------------------------
	//! Atmosphere only: players near a holding hear the routine change. Leaders, quartermasters
	//! and occupants stay available to talk and trade whatever the hour.
	protected void Routines(array<int> ids, array<IEntity> players)
	{
		int phase = RoutinePhase(RZ_GameTime.GetHour());
		if (phase == PHASE_NONE)
			return;

		foreach (RZ_Holding h : m_aHoldings)
		{
			if (!h.m_bSpawned)
				continue;

			if (h.m_iPhase == phase)
				continue;

			bool first = h.m_iPhase == -2;
			h.m_iPhase = phase;
			if (first || phase == PHASE_DAY)
				continue;

			RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
			string line = RZ_FactionVoice.RoutineLine(rng, h.m_eFaction, phase, h.m_sName);
			if (line.IsEmpty())
				continue;

			foreach (int i, IEntity p : players)
			{
				if (RZ_Util.DistXZ(p.GetOrigin(), h.m_vPos) > ROUTINE_RADIUS)
					continue;

				string key = string.Format("%1:%2", h.m_iId, ids[i]);
				float last;
				if (m_mRoutineTold.Find(key, last) && m_fClock - last < ROUTINE_COOLDOWN)
					continue;

				m_mRoutineTold.Set(key, m_fClock);
				m_GM.Say(ids[i], h.GetTitle(), h.m_eFaction, line);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A routine line for chatter ("night watch rotates at 20:30"): the next shift or watch.
	string NextRoutineClock()
	{
		float hour = RZ_GameTime.GetHour();
		if (hour < 0)
			return "";

		if (hour < 17.5 && hour >= 5.5)
			return RZ_GameTime.Clock(17.5);

		if (hour < 20.5 && hour >= 17.5)
			return RZ_GameTime.Clock(20.5);

		return RZ_GameTime.Clock(5.5);
	}

	// ================================================================ Bodies near players
	//------------------------------------------------------------------------------------------------
	protected void UpdateBodies(array<IEntity> players)
	{
		foreach (RZ_Holding h : m_aHoldings)
		{
			float nearest = RZ_Util.NearestDistance(h.m_vPos, players);
			if (h.m_bSpawned && nearest > m_Settings.m_fVisibleRadius + DESPAWN_MARGIN)
				DespawnBodies(h);
			else if (!h.m_bSpawned && nearest <= m_Settings.m_fVisibleRadius)
				SpawnBodies(h);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnBodies(RZ_Holding h)
	{
		h.m_bSpawned = true;
		RZ_Hostility hostility = m_GM.GetHostility();

		// Night (RZ_Environment): fewer guards, and they stay closer in
		int guards = m_Settings.m_iGuardCount;
		float guardMin = 15;
		float guardMax = 35;
		RZ_Environment env = m_GM.GetEnvironment();
		if (env && env.IsNight())
		{
			guards = env.GuardCount(guards, h.m_eFaction);
			guardMin = 5;
			guardMax = 15;
		}

		// Camp life gives them jobs and dresses the place (RZ_CampLife). It moves the place into the
		// building in town that suits the faction: its people come out in that building's yard
		RZ_CampLife camp = m_GM.GetCampLife();
		RZ_ECampPlace kind = RZ_ECampPlace.OUTPOST;
		if (h.m_bStronghold)
			kind = RZ_ECampPlace.STRONGHOLD;

		string campKey = "h" + h.m_iId.ToString();
		vector home = h.m_vPos;
		if (camp)
			home = camp.HomeFor(campKey, h.m_eFaction, kind, h.m_vPos);

		array<IEntity> guardBodies = {};
		array<IEntity> personBodies = {};
		array<IEntity> keeperBodies = {};

		// A missing relief this shift (the faults table) is one guard fewer at the gate
		RZ_Errands errands = m_GM.GetErrands();
		if (errands)
			guards = errands.GetFaults().Guards(h.m_iId, guards);

		ResourceName guardPrefab = GuardPrefab(h.m_eFaction);
		for (int g = 0; g < guards && !guardPrefab.IsEmpty(); g++)
		{
			IEntity guard = RZ_Util.SpawnPrefab(m_GM.PickLook(guardPrefab, RZ_Looks.SeedOf(h.m_iId, g)), RZ_Mission.LandPointNear(home, guardMin, guardMax), Math.RandomFloat(0, 360));
			if (!guard)
				continue;

			h.m_aBodies.Insert(guard);
			guardBodies.Insert(guard);
			if (hostility)
				hostility.WatchUnit(guard, h.m_eFaction);
		}

		ResourceName occupantPrefab = OccupantPrefab(h.m_eFaction);
		int shown = Math.Min(m_Settings.m_iOccupantCount, h.m_aPeople.Count());
		for (int p = 0; p < shown && !occupantPrefab.IsEmpty(); p++)
		{
			// The same named person wears the same clothes each time (People's looks)
			IEntity person = RZ_Util.SpawnPrefab(m_GM.PickLook(occupantPrefab, RZ_Looks.SeedOf(h.m_iId, p)), RZ_Mission.LandPointNear(home, 4, 14), Math.RandomFloat(0, 360));
			if (!person)
				continue;

			int code = h.m_iId * 10 + p;
			string labels = "Ask how things are";
			if (errands && errands.IsOn())
				labels = RZ_NamedPeople.LABELS;

			RZ_PuzzleObjectComponent.Tag(person, TAG, code, "", labels);
			RZ_ClueComponent.Tag(person, TAG, code, "Talk to " + h.m_aPeople[p]);
			if (errands)
				errands.GetPeople().NoteBody(RZ_NamedPeople.HoldingKey(h.m_iId, p), person);
			h.m_aBodies.Insert(person);
			personBodies.Insert(person);
			if (hostility)
				hostility.WatchUnit(person, h.m_eFaction);
		}

		// An outpost's own quartermaster (the stronghold already has the real one)
		if (!h.m_bStronghold && m_Settings.m_bOutpostQuartermaster)
		{
			IEntity qm = RZ_Util.SpawnPrefab(m_GM.PickLook(m_GM.GetQuartermasterPrefab(h.m_eFaction), h.m_iId), RZ_Mission.LandPointNear(home, 1, 4), Math.RandomFloat(0, 360));
			if (qm)
			{
				h.m_aBodies.Insert(qm);
				keeperBodies.Insert(qm);
			}
		}

		if (!camp)
			return;

		camp.Enter(campKey, h.GetTitle(), h.m_eFaction, kind, h.m_vPos, 0, guardPrefab, guardBodies, personBodies, keeperBodies);
	}

	//------------------------------------------------------------------------------------------------
	protected void DespawnBodies(RZ_Holding h)
	{
		RZ_CampLife camp = m_GM.GetCampLife();
		if (camp)
			camp.Leave("h" + h.m_iId.ToString());

		foreach (IEntity e : h.m_aBodies)
		{
			RZ_Util.DeleteUnit(e);
		}

		h.m_aBodies.Clear();
		h.m_bSpawned = false;
		h.m_iPhase = -2;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GuardPrefab(RZ_EFaction f)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return m_Settings.m_sHavenGuardPrefab;
			case RZ_EFaction.MILITARY: return m_Settings.m_sBastionGuardPrefab;
			case RZ_EFaction.BANDITS: return m_Settings.m_sRedHandGuardPrefab;
			case RZ_EFaction.INSTITUTE: return m_Settings.m_sHalcyonGuardPrefab;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A road post's speaker at the barrier (Dialogue > Trees): this faction's occupant prefab.
	ResourceName SpeakerPrefab(RZ_EFaction f)
	{
		return OccupantPrefab(f);
	}

	//------------------------------------------------------------------------------------------------
	protected ResourceName OccupantPrefab(RZ_EFaction f)
	{
		ResourceName prefab;
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: prefab = m_Settings.m_sHavenOccupantPrefab; break;
			case RZ_EFaction.MILITARY: prefab = m_Settings.m_sBastionOccupantPrefab; break;
			case RZ_EFaction.BANDITS: prefab = m_Settings.m_sRedHandOccupantPrefab; break;
			case RZ_EFaction.INSTITUTE: prefab = m_Settings.m_sHalcyonOccupantPrefab; break;
		}

		if (prefab.IsEmpty())
			prefab = m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);

		return prefab;
	}

	// ================================================================ Talking to named people
	//------------------------------------------------------------------------------------------------
	//! From RZ_ReadClueAction / RZ_PuzzleChoiceAction on a named guard or occupant (TAG).
	void HandlePersonTalk(int playerId, int code)
	{
		RZ_Holding h = GetHolding(code / 10);
		int idx = code % 10;
		if (!h || idx < 0 || idx >= h.m_aPeople.Count())
			return;

		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		int phase = RoutinePhase(RZ_GameTime.GetHour());
		string text = RZ_FactionVoice.PersonLine(rng, h.m_eFaction, h.m_aRoles[idx], phase, h.m_sName, playerId, h.m_vPos);

		RZ_FactionLife life = m_GM.GetFactionLife();
		if (life)
		{
			string extra = life.TalkExtra(h.m_eFaction, h.m_vPos);
			if (!extra.IsEmpty())
				text += "\n" + extra;
		}

		m_GM.NotifyPlayer(playerId, h.m_aPeople[idx] + ", " + h.GetTitle(), text);

		// They stop what they're doing while you talk
		RZ_CampLife camp = m_GM.GetCampLife();
		if (camp)
			camp.HoldPersonNear(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion promotions (faction life events): "Private" becomes "Corporal", "Corporal"
	//! becomes "Sergeant" for one named person at a Bastion holding. False if nobody fits.
	bool PromoteSomeone(out string oldName, out string newName, out string place)
	{
		array<RZ_Holding> bastion = {};
		foreach (RZ_Holding h : m_aHoldings)
		{
			if (h.m_eFaction == RZ_EFaction.MILITARY && !h.m_aPeople.IsEmpty())
				bastion.Insert(h);
		}

		if (bastion.IsEmpty())
			return false;

		RZ_Holding pick = bastion.GetRandomElement();
		int i = Math.RandomInt(0, pick.m_aPeople.Count());
		string name = pick.m_aPeople[i];
		string promoted = "";
		if (name.IndexOf("Private ") == 0)
			promoted = "Corporal " + name.Substring(8, name.Length() - 8);
		else if (name.IndexOf("Corporal ") == 0)
			promoted = "Sergeant " + name.Substring(9, name.Length() - 9);

		if (promoted.IsEmpty())
			return false;

		pick.m_aPeople[i] = promoted;
		oldName = name;
		newName = promoted;
		place = pick.m_sName;
		return true;
	}
}
