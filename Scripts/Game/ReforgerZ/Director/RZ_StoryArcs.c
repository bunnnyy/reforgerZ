//! Story director: arcs (docs/DESIGN.md, "Story arcs and headlines"). Short storylines strung together from
//! what the mode already has: missions and events, wanderers, leaders, territory, supply
//! trucks, safehouses and the headlines. Each step only fires if the previous one really
//! happened in the world (a mission ran, a player heard a rumour, a truck left), so every
//! server lives through different plots, and an arc whose next step can't happen just
//! fizzles out quietly.
//!
//! At most one arc runs at a time. A new one is considered every hour or so with players
//! online; the same arc doesn't come back within four hours. The Haven's dark side (The Quiet
//! Congregation) is rare on purpose: it needs a safehouse that has lost a lot of people and
//! the Preacher alive, and it always resolves.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Director > Story arcs.
[BaseContainerProps()]
class RZ_StorySettings
{
	[Attribute("1", desc: "Story arcs on: short storylines made of missions, rumours and headlines (docs/DESIGN.md, Story arcs and headlines)")]
	bool m_bEnabled = true;

	[Attribute("60", desc: "Average real minutes (with players online) between chances for a new arc")]
	float m_fMinutesBetween = 60;

	[Attribute("0.6", desc: "Chance that an arc starts when one is considered")]
	float m_fStartChance = 0.6;

	[Attribute("4", desc: "Hours before the same arc can run again")]
	float m_fRepeatHours = 4;

	[Attribute("6", desc: "The Quiet Congregation: people a safehouse must have lost recently (to hunger, raids, contracts)")]
	int m_iDarkSideLosses = 6;

	[Attribute("12", desc: "The Quiet Congregation: hours before it can happen again")]
	float m_fDarkSideRepeatHours = 12;

	[Attribute("0.35", desc: "The Quiet Congregation: chance it's picked when it could run")]
	float m_fDarkSideChance = 0.35;
}

//------------------------------------------------------------------------------------------------
class RZ_ArcSave
{
	int kind = -1;
	int step;
	float stepAt;
	ref array<int> ints = {};
	ref array<float> floats = {};
	ref array<string> strs = {};
}

//------------------------------------------------------------------------------------------------
class RZ_StorySave
{
	float clock;
	float nextChance;
	ref array<float> lastRun = {};
	ref RZ_ArcSave active;
}

//------------------------------------------------------------------------------------------------
//! One running arc. Data first: ints/floats/strs are whatever the arc needs and are saved.
class RZ_StoryArc
{
	int m_iKind;
	int m_iStep;
	float m_fStepAt;
	bool m_bDone;
	ref array<int> m_aInts = {};
	ref array<float> m_aFloats = {};
	ref array<string> m_aStrs = {};

	//------------------------------------------------------------------------------------------------
	int I(int idx)
	{
		if (idx < 0 || idx >= m_aInts.Count())
			return -1;

		return m_aInts[idx];
	}

	//------------------------------------------------------------------------------------------------
	float F(int idx)
	{
		if (idx < 0 || idx >= m_aFloats.Count())
			return 0;

		return m_aFloats[idx];
	}

	//------------------------------------------------------------------------------------------------
	string S(int idx)
	{
		if (idx < 0 || idx >= m_aStrs.Count())
			return "";

		return m_aStrs[idx];
	}

	//------------------------------------------------------------------------------------------------
	void SetI(int idx, int v)
	{
		while (m_aInts.Count() <= idx)
		{
			m_aInts.Insert(-1);
		}

		m_aInts[idx] = v;
	}

	//------------------------------------------------------------------------------------------------
	void SetF(int idx, float v)
	{
		while (m_aFloats.Count() <= idx)
		{
			m_aFloats.Insert(0);
		}

		m_aFloats[idx] = v;
	}

	//------------------------------------------------------------------------------------------------
	void SetS(int idx, string v)
	{
		while (m_aStrs.Count() <= idx)
		{
			m_aStrs.Insert("");
		}

		m_aStrs[idx] = v;
	}

	//------------------------------------------------------------------------------------------------
	RZ_ArcSave ToSave()
	{
		RZ_ArcSave s = new RZ_ArcSave();
		s.kind = m_iKind;
		s.step = m_iStep;
		s.stepAt = m_fStepAt;
		foreach (int i : m_aInts)
		{
			s.ints.Insert(i);
		}

		foreach (float f : m_aFloats)
		{
			s.floats.Insert(f);
		}

		foreach (string t : m_aStrs)
		{
			s.strs.Insert(t);
		}

		return s;
	}
}

//------------------------------------------------------------------------------------------------
//! The arcs (the same values as RZ_StoryArcs.ARC_*), for switch statements.
enum RZ_EArc
{
	COURIER,
	CONVOY,
	FLOCK,
	MAP,
	FEVER,
	CONGREGATION
}

//------------------------------------------------------------------------------------------------
class RZ_StoryArcs
{
	static const int ARC_COURIER = 0;		// The Missing Courier
	static const int ARC_CONVOY = 1;		// The Last Convoy
	static const int ARC_FLOCK = 2;			// The Preacher's Flock
	static const int ARC_MAP = 3;			// The Deserter's Map
	static const int ARC_FEVER = 4;			// Fever Town
	static const int ARC_CONGREGATION = 5;	// The Quiet Congregation (the Haven's dark side)
	static const int ARC_COUNT = 6;

	protected static const float RUMOUR_WAIT = 2400;
	protected static const float STEP_GAP = 600;
	protected static const float FLOCK_HORDE_GAP = 600;
	protected static const float FLOCK_MINUTES = 3600;
	protected static const float CONGREGATION_SECONDS = 5400;
	protected static const int CONGREGATION_FOOD = 6;

	protected RZ_GameModeComponent m_GM;
	protected RZ_StorySettings m_Settings;
	protected ref RZ_StorySave m_Restored;
	protected ref RZ_StoryArc m_Arc;
	protected ref array<float> m_aLastRun = {-100000, -100000, -100000, -100000, -100000, -100000};
	protected float m_fClock;
	protected float m_fNextChance;
	protected float m_fCheck;

	//------------------------------------------------------------------------------------------------
	void RZ_StoryArcs(RZ_GameModeComponent gm, RZ_StorySettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled() { return m_Settings.m_bEnabled; }

	//------------------------------------------------------------------------------------------------
	static string ArcName(int kind)
	{
		switch (kind)
		{
			case RZ_EArc.COURIER: return "The Missing Courier";
			case RZ_EArc.CONVOY: return "The Last Convoy";
			case RZ_EArc.FLOCK: return "The Preacher's Flock";
			case RZ_EArc.MAP: return "The Deserter's Map";
			case RZ_EArc.FEVER: return "Fever Town";
			case RZ_EArc.CONGREGATION: return "The Quiet Congregation";
		}

		return "";
	}

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	void Restore(RZ_StorySave saved)
	{
		m_Restored = saved;
	}

	//------------------------------------------------------------------------------------------------
	RZ_StorySave Save()
	{
		RZ_StorySave data = new RZ_StorySave();
		data.clock = m_fClock;
		data.nextChance = m_fNextChance;
		foreach (float t : m_aLastRun)
		{
			data.lastRun.Insert(t);
		}

		if (m_Arc && !HoldsMission())
			data.active = m_Arc.ToSave();

		return data;
	}

	//------------------------------------------------------------------------------------------------
	//! Missions don't survive a restart, so an arc waiting on one isn't saved (it just ends).
	protected bool HoldsMission()
	{
		if (!m_Arc)
			return false;

		switch (m_Arc.m_iKind)
		{
			case RZ_EArc.COURIER: return m_Arc.m_iStep == 0 || m_Arc.m_iStep == 3;
			case RZ_EArc.MAP: return true;
			case RZ_EArc.FEVER: return m_Arc.m_iStep >= 1;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	void Setup()
	{
		if (!m_Settings.m_bEnabled)
		{
			RZ_Log.Info("Story arcs are off.");
			return;
		}

		m_fNextChance = Math.Max(20, m_Settings.m_fMinutesBetween * 0.5) * 60;
		if (m_Restored)
		{
			m_fClock = m_Restored.clock;
			m_fNextChance = m_Restored.nextChance;
			if (m_Restored.lastRun)
			{
				for (int k = 0; k < m_Restored.lastRun.Count() && k < ARC_COUNT; k++)
				{
					m_aLastRun[k] = m_Restored.lastRun[k];
				}
			}

			RZ_ArcSave a = m_Restored.active;
			if (a && a.kind >= 0 && a.kind < ARC_COUNT)
			{
				m_Arc = new RZ_StoryArc();
				m_Arc.m_iKind = a.kind;
				m_Arc.m_iStep = a.step;
				m_Arc.m_fStepAt = a.stepAt;
				foreach (int i : a.ints)
				{
					m_Arc.m_aInts.Insert(i);
				}

				foreach (float f : a.floats)
				{
					m_Arc.m_aFloats.Insert(f);
				}

				foreach (string s : a.strs)
				{
					m_Arc.m_aStrs.Insert(s);
				}
			}
		}

		m_Restored = null;
		string running = "none";
		if (m_Arc)
			running = ArcName(m_Arc.m_iKind);

		RZ_Log.Info("Story arcs: on, running " + running + ".");
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_Settings.m_bEnabled)
			return;

		m_fClock += dt;
		m_fCheck += dt;
		if (m_fCheck < 10)
			return;

		float step = m_fCheck;
		m_fCheck = 0;

		if (m_Arc)
		{
			TickArc(step, ids, players);
			if (m_Arc && m_Arc.m_bDone)
				m_Arc = null;

			return;
		}

		if (ids.IsEmpty() || m_fClock < m_fNextChance)
			return;

		float mean = Math.Max(10, m_Settings.m_fMinutesBetween) * 60;
		m_fNextChance = m_fClock + Math.RandomFloat(mean * 0.75, mean * 1.25);
		if (Math.RandomFloat01() >= m_Settings.m_fStartChance)
			return;

		TryStartAny();
	}

	//------------------------------------------------------------------------------------------------
	protected void TryStartAny()
	{
		// The dark side first, rarely, and only where it makes sense
		int house = DarkSideHouse();
		float darkGap = m_Settings.m_fDarkSideRepeatHours * 3600;
		if (house >= 0 && m_fClock - m_aLastRun[ARC_CONGREGATION] >= darkGap && Math.RandomFloat01() < m_Settings.m_fDarkSideChance)
		{
			if (Begin(ARC_CONGREGATION))
				return;
		}

		array<int> kinds = {ARC_COURIER, ARC_CONVOY, ARC_FLOCK, ARC_MAP, ARC_FEVER};
		for (int i = kinds.Count() - 1; i > 0; i--)
		{
			int j = Math.RandomInt(0, i + 1);
			int tmp = kinds[i];
			kinds[i] = kinds[j];
			kinds[j] = tmp;
		}

		foreach (int kind : kinds)
		{
			if (m_fClock - m_aLastRun[kind] < m_Settings.m_fRepeatHours * 3600)
				continue;

			if (Begin(kind))
				return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Start an arc now (also for testing from other code). False if it can't run.
	bool Begin(int kind)
	{
		if (m_Arc || kind < 0 || kind >= ARC_COUNT)
			return false;

		RZ_StoryArc arc = new RZ_StoryArc();
		arc.m_iKind = kind;
		arc.m_fStepAt = m_fClock;
		m_Arc = arc;

		bool ok = false;
		switch (kind)
		{
			case RZ_EArc.COURIER: ok = StartCourier(); break;
			case RZ_EArc.CONVOY: ok = StartConvoy(); break;
			case RZ_EArc.FLOCK: ok = StartFlock(); break;
			case RZ_EArc.MAP: ok = StartMap(); break;
			case RZ_EArc.FEVER: ok = StartFever(); break;
			case RZ_EArc.CONGREGATION: ok = StartCongregation(); break;
		}

		if (!ok)
		{
			m_Arc = null;
			return false;
		}

		m_aLastRun[kind] = m_fClock;
		RZ_Log.Info("Story arc started: " + ArcName(kind));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void NextStep()
	{
		m_Arc.m_iStep = m_Arc.m_iStep + 1;
		m_Arc.m_fStepAt = m_fClock;
	}

	//------------------------------------------------------------------------------------------------
	protected float StepAge()
	{
		return m_fClock - m_Arc.m_fStepAt;
	}

	//------------------------------------------------------------------------------------------------
	protected void End(string headline)
	{
		if (!headline.IsEmpty())
			Headline(headline);

		RZ_Log.Info("Story arc ended: " + ArcName(m_Arc.m_iKind));
		m_Arc.m_bDone = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Headline(string text)
	{
		if (m_GM.GetLeaders())
			m_GM.GetLeaders().AddHeadline(text, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void NotifyKnown(RZ_EFaction faction, string title, string text)
	{
		array<int> all = {};
		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int playerId : all)
		{
			if (RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, faction), RZ_ETier.KNOWN) || RZ_Radio.IsRadioOperator(m_GM, playerId))
				m_GM.NotifyPlayer(playerId, title, text);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Start a mission made here (configured, then set up); its id, or -1.
	protected int StartMission(RZ_Mission m)
	{
		if (!m.m_Req.IsWorldOk(m_GM, 1))
			return -1;

		if (!m_GM.GetMissions().StartMission(m))
			return -1;

		return m.m_iId;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsMissionRunning(int id)
	{
		return id > 0 && m_GM.GetMissions().FindById(id) != null;
	}

	//------------------------------------------------------------------------------------------------
	protected void TickArc(float dt, array<int> ids, array<IEntity> players)
	{
		switch (m_Arc.m_iKind)
		{
			case RZ_EArc.COURIER: TickCourier(); break;
			case RZ_EArc.CONVOY: TickConvoy(); break;
			case RZ_EArc.FLOCK: TickFlock(players); break;
			case RZ_EArc.MAP: TickMap(); break;
			case RZ_EArc.FEVER: TickFever(dt, players); break;
			case RZ_EArc.CONGREGATION: TickCongregation(); break;
		}
	}

	// ================================================================ Hooks from other systems
	//------------------------------------------------------------------------------------------------
	//! A mission ended (RZ_GameModeComponent.OnContractFinished, every mission).
	void OnMissionFinished(RZ_Mission m, bool success)
	{
		if (!m_Arc || m_Arc.m_bDone)
			return;

		// Only these arcs keep a mission id in slot 0
		int kind = m_Arc.m_iKind;
		bool usesMission = kind == ARC_COURIER || kind == ARC_MAP || (kind == ARC_FEVER && m_Arc.m_iStep >= 1);
		if (!usesMission || m.m_iId != m_Arc.I(0))
		{
			// The Preacher's bells as cover: a job done near him while he rings them
			if (m_Arc.m_iKind == ARC_FLOCK && success && m_Arc.I(2) < 1 && !m.m_bGlobal)
			{
				vector at;
				if (m_GM.GetWanderers() && m_GM.GetWanderers().GetPosition(m_Arc.I(1), at) && RZ_Util.DistXZ(at, m.m_vPos) <= 800)
				{
					m_Arc.SetI(2, 1);
					Headline(string.Format("Someone used %1's bells as cover for a job near %2. He doesn't seem to mind.", m_Arc.S(0), m.GetLocationName(m_GM)));
				}
			}

			return;
		}

		m_Arc.SetI(0, -1);
		m_Arc.SetI(9, 1);			// "the mission ran"
		if (success)
			m_Arc.SetI(8, 1);
		else
			m_Arc.SetI(8, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! A wanderer is talked to: an arc may add a line (a rumour, a map piece). "" if none.
	string OnWandererTalk(int playerId, int wandererId)
	{
		if (!m_Arc || m_Arc.m_bDone)
			return "";

		if (m_Arc.m_iKind == ARC_COURIER && m_Arc.m_iStep == 1)
		{
			m_Arc.SetI(3, 1);	// someone heard it
			return string.Format("\"You know the Halcyon courier who went missing? I saw Red Hand men near %1 carrying lab cases. White ones, with the stickers.\"", m_Arc.S(1));
		}

		if (m_Arc.m_iKind == ARC_MAP)
		{
			RZ_Mission_DeserterMap map = RZ_Mission_DeserterMap.Cast(m_GM.GetMissions().FindById(m_Arc.I(0)));
			if (map)
				return map.WandererPiece(m_GM, playerId, wandererId);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A line for wanderers' and leaders' gossip about the running arc, "" if none.
	string Rumour()
	{
		if (!m_Arc || m_Arc.m_bDone)
			return "";

		switch (m_Arc.m_iKind)
		{
			case RZ_EArc.COURIER:
			{
				if (m_Arc.m_iStep >= 2)
					return string.Format("\"The Red Hand camp near %1 has antidotes for sale. In Halcyon cases. Make of that what you like.\"", m_Arc.S(1));

				break;
			}
			case RZ_EArc.CONVOY: return string.Format("\"Bastion's pulling %1 back. Big convoy. Every Red Hand lookout from here to the coast is watching that road.\"", m_Arc.S(0));
			case RZ_EArc.FLOCK: return string.Format("\"%1 rings the bells in every town now. And they follow him. Hundreds.\"", m_Arc.S(0));
			case RZ_EArc.FEVER: return string.Format("\"Half the safehouse in %1 is running a fever. Somebody says the Red Hand put something in the well.\"", m_Arc.S(0));
			case RZ_EArc.CONGREGATION:
			{
				if (m_Arc.m_iStep >= 1)
					return string.Format("\"The safehouse in %1 has gone strange. They pray with the Preacher, eat half portions and 'borrow' from the other houses. Nobody says no to them.\"", m_Arc.S(0));

				break;
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A supply truck the arc asked for left (The Last Convoy).
	void OnConvoyDeparted(int f, string dest)
	{
		if (!m_Arc || m_Arc.m_iKind != ARC_CONVOY || m_Arc.m_iStep != 0)
			return;

		NextStep();
		Headline(string.Format("Bastion's convoy has left for %1. Red Hand lookouts were seen on the ridge above the road.", dest));
		NotifyKnown(RZ_EFaction.BANDITS, "The Red Hand", string.Format("\"Bastion's big convoy is on the road to %1. Lot of stock on one truck. Just saying.\"", dest));
		NotifyKnown(RZ_EFaction.MILITARY, "Task Force Bastion", string.Format("\"Convoy rolling for %1. Anyone who wants to be useful, ride along.\"", dest));
	}

	//------------------------------------------------------------------------------------------------
	//! The convoy arrived or was lost.
	void OnConvoyResult(int f, string dest, bool delivered)
	{
		if (!m_Arc || m_Arc.m_iKind != ARC_CONVOY || m_Arc.m_iStep != 1)
			return;

		if (delivered)
			End(string.Format("Bastion's convoy made it to %1. The post is restocked, and they're staying after all.", dest));
		else
			End(string.Format("Bastion's convoy never reached %1. The post there is running dry.", dest));
	}

	//------------------------------------------------------------------------------------------------
	//! The Preacher was asked to stop his bells (RZ_Wanderers.StopBells).
	void OnPreacherStopped(int wandererId, vector at)
	{
		if (!m_Arc)
			return;

		if (m_Arc.m_iKind == ARC_FLOCK && wandererId == m_Arc.I(1))
		{
			End(string.Format("%1 has put down his bell rope. Someone talked him out of it, for now.", m_Arc.S(0)));
			return;
		}

		if (m_Arc.m_iKind == ARC_CONGREGATION && wandererId == m_Arc.I(1) && m_Arc.m_iStep >= 1)
		{
			RZ_Safehouse h = House(m_Arc.I(0));
			if (h && RZ_Util.DistXZ(at, h.m_vPos) <= 300)
				EndCongregation(string.Format("%1 walked out of the safehouse in %2 at dawn. Without him the prayers stopped by noon, and they're arguing about supper again.", m_Arc.S(1), m_Arc.S(0)));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Food handed to a safehouse (RZ_Safehouses.DeliverFood).
	void OnFoodDelivered(int houseIdx, int amount)
	{
		if (!m_Arc || m_Arc.m_iKind != ARC_CONGREGATION || m_Arc.I(0) != houseIdx || m_Arc.m_iStep < 1)
			return;

		int total = m_Arc.I(2) + amount;
		m_Arc.SetI(2, total);
		if (total >= CONGREGATION_FOOD)
			EndCongregation(string.Format("The safehouse in %1 ate a full meal for the first time in a week. Fewer of them went to the Preacher's prayers after.", m_Arc.S(0)));
	}

	//------------------------------------------------------------------------------------------------
	//! Upkeep multiplier for a safehouse: the congregation fasts (half portions).
	float UpkeepMult(int houseIdx)
	{
		if (m_Arc && m_Arc.m_iKind == ARC_CONGREGATION && m_Arc.I(0) == houseIdx && m_Arc.m_iStep >= 1)
			return 0.5;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! A line for a safehouse's status while the congregation holds it, "" otherwise.
	string HouseLine(int houseIdx)
	{
		if (m_Arc && m_Arc.m_iKind == ARC_CONGREGATION && m_Arc.I(0) == houseIdx && m_Arc.m_iStep >= 1)
			return "\"We eat half and pray the rest. Brother says the hunger is a door. You can come in, if you leave the gun outside and bring something for the table.\"";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The Preacher's bells: louder during his flock, and he heads for the congregation's house.
	float BellMult(int wandererId)
	{
		if (m_Arc && m_Arc.m_iKind == ARC_FLOCK && m_Arc.I(1) == wandererId)
			return 2;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Where the Preacher should walk instead of choosing (the congregation's safehouse). -1 = his choice.
	int PreacherHouse(int wandererId)
	{
		if (m_Arc && m_Arc.m_iKind == ARC_CONGREGATION && m_Arc.I(1) == wandererId)
			return m_Arc.I(0);

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! A wanderer started resting somewhere (RZ_Wanderers.CheckArrival).
	void OnWandererRested(int wandererId, vector at)
	{
		if (!m_Arc || m_Arc.m_iKind != ARC_FLOCK || m_Arc.I(1) != wandererId)
			return;

		int towns = m_Arc.I(3) + 1;
		m_Arc.SetI(3, towns);
		Headline(string.Format("%1 rang the bells at %2 until his hands bled.", m_Arc.S(0), NearTown(at)));
	}

	// ================================================================ Helpers
	//------------------------------------------------------------------------------------------------
	protected string NearTown(vector pos)
	{
		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(pos));
		if (s)
			return s.m_sName;

		return RZ_SettlementScanner.GridReference(pos);
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Safehouse House(int idx)
	{
		if (!m_GM.GetSafehouses())
			return null;

		return m_GM.GetSafehouses().Get(idx);
	}

	//------------------------------------------------------------------------------------------------
	protected int PreacherId()
	{
		if (!m_GM.GetWanderers())
			return -1;

		array<int> ids = {};
		m_GM.GetWanderers().GetLiving(RZ_Wanderers.RoleBit(RZ_EWandererRole.PREACHER), ids);
		if (ids.IsEmpty())
			return -1;

		return ids[0];
	}

	//------------------------------------------------------------------------------------------------
	//! A safehouse that has lost a lot of people lately, -1 if none (or no Preacher).
	protected int DarkSideHouse()
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses || PreacherId() < 0)
			return -1;

		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			if (h && h.m_iLost >= m_Settings.m_iDarkSideLosses)
				return i;
		}

		return -1;
	}

	// ================================================================ 1. The Missing Courier
	// ints: 0 mission id, 8 last mission success, 9 mission ran, 3 rumour heard. strs: 1 camp town.
	//------------------------------------------------------------------------------------------------
	protected bool StartCourier()
	{
		if (!m_GM.HasLab() || m_GM.GetCure().IsCured() || !m_GM.GetWanderers())
			return false;

		RZ_Mission_LostColleague m = new RZ_Mission_LostColleague();
		m.Configure(m_GM);
		int id = StartMission(m);
		if (id < 0)
			return false;

		m.m_sTitle = "The Missing Courier";
		m.m_sBrief = string.Format("A Halcyon courier carrying samples between the lab and %1 stopped reporting. Find him, or what's left of him.", m.GetLocationName(m_GM));
		m_Arc.SetI(0, id);

		// Where the Red Hand will turn up: one of its holdings, else a rough town
		string camp = "";
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			array<RZ_Holding> posts = {};
			territory.GetOutposts(RZ_EFaction.BANDITS, posts);
			if (!posts.IsEmpty())
				camp = posts.GetRandomElement().m_sName;
		}

		if (camp.IsEmpty())
		{
			RZ_Settlement s = m_GM.GetSettlement(RZ_Mission.PickSettlement(m_GM, 30, 90));
			if (s)
				camp = s.m_sName;
		}

		if (camp.IsEmpty())
			camp = "the crossroads";

		m_Arc.SetS(1, camp);
		Headline("A Halcyon courier stopped reporting on the road from the lab. The lab is asking anyone who'll listen.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void TickCourier()
	{
		switch (m_Arc.m_iStep)
		{
			case 0:
			{
				// Step 1 only once the search really happened (the mission ended either way)
				if (m_Arc.I(9) == 1)
				{
					NextStep();
					return;
				}

				if (!IsMissionRunning(m_Arc.I(0)))
					End("");

				return;
			}
			case 1:
			{
				// Someone has to hear the rumour from a wanderer
				if (m_Arc.I(3) == 1)
				{
					NextStep();
					Headline(string.Format("A Red Hand camp near %1 is selling antidotes. In Halcyon cases.", m_Arc.S(1)));
					NotifyKnown(RZ_EFaction.INSTITUTE, "Halcyon", string.Format("\"Our cases. Our antidotes. Near %1. Whatever they took from our courier, they'll be back for more.\"", m_Arc.S(1)));
					return;
				}

				if (StepAge() >= RUMOUR_WAIT)
					End("Nobody followed up on the missing Halcyon courier. The trail went cold.");

				return;
			}
			case 2:
			{
				if (StepAge() < STEP_GAP)
					return;

				RZ_Mission_LabDefence defence = new RZ_Mission_LabDefence();
				defence.Configure(m_GM);
				int id = StartMission(defence);
				if (id < 0)
				{
					End("The Red Hand sold the last of the stolen antidotes and moved on.");
					return;
				}

				defence.m_sBrief = "The Red Hand ran out of stolen antidotes. Now they're coming to the lab for more. Hold it for four minutes.";
				m_Arc.SetI(0, id);
				m_Arc.SetI(9, 0);
				NextStep();
				return;
			}
			case 3:
			{
				if (m_Arc.I(9) != 1)
				{
					if (!IsMissionRunning(m_Arc.I(0)))
						End("");

					return;
				}

				if (m_Arc.I(8) == 1)
					End("The lab held. Whatever happened to the Halcyon courier, the Red Hand won't sell his antidotes twice.");
				else
				{
					// The raid really empties the store: up to two doses go to the black market
					if (m_GM.GetCure())
						m_GM.GetCure().StealLabAntidotes(2);

					End("The Red Hand got into the lab's stores. The antidotes they sell now are Halcyon's own.");
				}
			}
		}
	}

	// ================================================================ 2. The Last Convoy
	// ints: 0 holding id. strs: 0 outpost name.
	//------------------------------------------------------------------------------------------------
	protected bool StartConvoy()
	{
		RZ_Territory territory = m_GM.GetTerritory();
		RZ_SupplyLines supply = m_GM.GetSupply();
		if (!territory || !territory.IsActive() || !supply || !supply.IsActive() || !supply.IsFactionRunning(RZ_EFaction.MILITARY))
			return false;

		array<RZ_Holding> posts = {};
		territory.GetOutposts(RZ_EFaction.MILITARY, posts);
		RZ_Holding pick;
		foreach (RZ_Holding h : posts)
		{
			if (!h.m_bFixed)
				pick = h;
		}

		if (!pick || !supply.RequestConvoy(RZ_EFaction.MILITARY, pick.m_iId, 600))
			return false;

		m_Arc.SetI(0, pick.m_iId);
		m_Arc.SetS(0, pick.m_sName);
		string home = "its stronghold";
		RZ_Holding top = territory.GetStronghold(RZ_EFaction.MILITARY);
		if (top)
			home = top.m_sName;

		string text = string.Format("Bastion is sending a convoy to pull %1's people and stock back to %2. It leaves within the hour.", pick.m_sName, home);
		Headline(text);
		m_GM.NotifyAll("Task Force Bastion", text);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void TickConvoy()
	{
		// Step 0 waits for the departure, step 1 for the truck: both come through hooks
		if (m_Arc.m_iStep == 0 && StepAge() >= 2400)
			End("Bastion's convoy never left. Someone up the chain changed their mind, again.");

		if (m_Arc.m_iStep == 1 && StepAge() >= 7200)
			End("");
	}

	// ================================================================ 3. The Preacher's Flock
	// ints: 1 the Preacher's wanderer id, 2 cover used, 3 towns rung, 4 hordes sent. strs: 0 his name.
	//------------------------------------------------------------------------------------------------
	protected bool StartFlock()
	{
		int id = PreacherId();
		if (id < 0)
			return false;

		m_Arc.SetI(1, id);
		m_Arc.SetI(2, 0);
		m_Arc.SetI(3, 0);
		m_Arc.SetI(4, 0);
		m_Arc.SetS(0, m_GM.GetWanderers().GetName(id));
		Headline(string.Format("%1 has started ringing the bells in every town he passes. People say he's calling them home.", m_Arc.S(0)));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void TickFlock(array<IEntity> players)
	{
		RZ_Wanderers wanderers = m_GM.GetWanderers();
		vector at;
		if (!wanderers || !wanderers.GetPosition(m_Arc.I(1), at))
		{
			End(string.Format("%1's bells have gone quiet for good. Nobody knows who found him first.", m_Arc.S(0)));
			return;
		}

		// After two towns, the hordes follow him: a small one now and then, only near players
		if (m_Arc.m_iStep == 0 && m_Arc.I(3) >= 2)
		{
			NextStep();
			Headline(string.Format("The hordes are following %1's bells from town to town. Find him, stop him, or stay out of the way.", m_Arc.S(0)));
			return;
		}

		if (m_Arc.m_iStep == 1)
		{
			if (StepAge() >= FLOCK_MINUTES)
			{
				End(string.Format("%1 walked on past the last town, still ringing. The hordes lost interest somewhere behind him.", m_Arc.S(0)));
				return;
			}

			float since = m_fClock - m_Arc.F(0);
			if (m_Arc.I(4) < 3 && since >= FLOCK_HORDE_GAP && RZ_Util.NearestDistance(at, players) <= 600)
			{
				m_Arc.SetF(0, m_fClock);
				m_Arc.SetI(4, m_Arc.I(4) + 1);
				m_GM.GetSpawner().SpawnHorde(at, 400);
				m_GM.NotifyHorde(at);
			}
		}

		// The flock fizzles if he never reaches a second town
		if (m_Arc.m_iStep == 0 && StepAge() >= FLOCK_MINUTES * 2)
			End("");
	}

	// ================================================================ 4. The Deserter's Map
	// ints: 0 mission id, 9 ran.
	//------------------------------------------------------------------------------------------------
	protected bool StartMap()
	{
		RZ_Wanderers wanderers = m_GM.GetWanderers();
		if (!wanderers || !RZ_PuzzleMission.HasPrefab(m_GM, RZ_EPuzzleProp.BODY))
			return false;

		RZ_Mission_DeserterMap m = new RZ_Mission_DeserterMap();
		m.Configure(m_GM);
		int id = StartMission(m);
		if (id < 0)
			return false;

		m_Arc.SetI(0, id);
		Headline("A Bastion deserter was found dying by a road with part of a map sewn into his jacket. Two more pieces are out there, with people on the roads.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void TickMap()
	{
		if (m_Arc.I(9) == 1)
		{
			if (m_Arc.I(8) == 1)
				End("Someone found the bunker on the deserter's map. Bastion is pretending it never existed.");
			else
				End("The deserter's map went cold. Maybe the bunker was never there.");

			return;
		}

		if (!IsMissionRunning(m_Arc.I(0)))
			End("");
	}

	// ================================================================ 5. Fever Town
	// ints: 0 mission id, 1 safehouse, 2 settlement, 5 seen, 8 success, 9 ran. floats: 1 infection added. strs: 0 town.
	//------------------------------------------------------------------------------------------------
	protected bool StartFever()
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses || houses.Count() == 0)
			return false;

		int idx = houses.PickRandom();
		RZ_Safehouse h = houses.Get(idx);
		if (!h || h.m_iSettlement < 0 || m_GM.GetMissions().IsSettlementBusy(h.m_iSettlement))
			return false;

		m_Arc.SetI(1, idx);
		m_Arc.SetI(2, h.m_iSettlement);
		m_Arc.SetI(5, 0);
		m_Arc.SetF(1, 0);
		m_Arc.SetS(0, h.m_sName);
		Headline(string.Format("People at the safehouse in %1 are running fevers. Nobody's been bitten.", h.m_sName));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void TickFever(float dt, array<IEntity> players)
	{
		RZ_Safehouse h = House(m_Arc.I(1));
		RZ_Settlement s = m_GM.GetSettlement(m_Arc.I(2));
		if (!h || !s)
		{
			End("");
			return;
		}

		switch (m_Arc.m_iStep)
		{
			case 0:
			{
				// The infection creeps up (20 points at most), and someone has to come and see it
				if (m_Arc.F(1) < 20)
				{
					float add = dt / 120;
					s.AddInfection(add);
					m_Arc.SetF(1, m_Arc.F(1) + add);
				}

				if (RZ_Util.NearestDistance(h.m_vPos, players) <= 300)
					m_Arc.SetI(5, 1);

				if (m_Arc.I(5) == 1 && StepAge() >= 300)
				{
					RZ_Mission_FeverWater survey = new RZ_Mission_FeverWater();
					survey.Configure(m_GM);
					survey.Prepare(m_Arc.I(2));
					int id = StartMission(survey);
					if (id < 0)
					{
						End(string.Format("The fevers in %1 burned out on their own. Nobody ever found out why.", h.m_sName));
						return;
					}

					m_Arc.SetI(0, id);
					m_Arc.SetI(9, 0);
					NextStep();
					return;
				}

				if (StepAge() >= 2400)
					End(string.Format("The fevers in %1 burned out on their own. Nobody came to look.", h.m_sName));

				return;
			}
			case 1:
			{
				if (m_Arc.I(9) != 1)
				{
					if (!IsMissionRunning(m_Arc.I(0)))
						End("");

					return;
				}

				if (m_Arc.I(8) != 1)
				{
					End(string.Format("Nobody finished checking the water. The fevers in %1 ran their course.", h.m_sName));
					return;
				}

				// Only after the water was checked: the evacuation, and the poison rumour
				RZ_Mission_FeverEvacuation evac = new RZ_Mission_FeverEvacuation();
				evac.Configure(m_GM);
				evac.Prepare(m_Arc.I(1));
				int evacId = StartMission(evac);
				Headline(string.Format("Halcyon says the well at %1 is bad. A Red Hand rumour says someone put something in it on purpose.", h.m_sName));
				if (evacId < 0)
				{
					End("");
					return;
				}

				m_Arc.SetI(0, evacId);
				m_Arc.SetI(9, 0);
				NextStep();
				return;
			}
			case 2:
			{
				if (m_Arc.I(9) != 1)
				{
					if (!IsMissionRunning(m_Arc.I(0)))
						End("");

					return;
				}

				if (m_Arc.I(8) == 1)
				{
					s.AddInfection(-m_Arc.F(1));
					End(string.Format("The sick and the old are out of %1, and the well is boarded up. The fevers are passing.", h.m_sName));
				}
				else
				{
					End(string.Format("Nobody got the vulnerable out of %1 in time. Two of them didn't make it.", h.m_sName));
				}
			}
		}
	}

	// ================================================================ 6. The Quiet Congregation
	// ints: 0 safehouse, 1 the Preacher, 2 food brought. strs: 0 town, 1 his name.
	//------------------------------------------------------------------------------------------------
	protected bool StartCongregation()
	{
		int idx = DarkSideHouse();
		int preacher = PreacherId();
		RZ_Safehouse h = House(idx);
		if (!h || preacher < 0)
			return false;

		m_Arc.SetI(0, idx);
		m_Arc.SetI(1, preacher);
		m_Arc.SetI(2, 0);
		m_Arc.SetS(0, h.m_sName);
		m_Arc.SetS(1, m_GM.GetWanderers().GetName(preacher));
		h.m_iLost = 0;
		Headline(string.Format("The safehouse in %1 has buried too many this week. People there have started waiting for %2 by the gate.", h.m_sName, m_Arc.S(1)));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void TickCongregation()
	{
		RZ_Safehouse h = House(m_Arc.I(0));
		RZ_Wanderers wanderers = m_GM.GetWanderers();
		vector at;
		if (!h || !wanderers || !wanderers.GetPosition(m_Arc.I(1), at))
		{
			EndCongregation("");
			return;
		}

		if (m_Arc.m_iStep == 0)
		{
			// He comes (the wanderers send him there); then the house turns inward
			if (RZ_Util.DistXZ(at, h.m_vPos) <= 200 || StepAge() >= 1800)
			{
				NextStep();
				Headline(string.Format("The safehouse in %1 prays with %2 now. Half portions, bells at dusk, and they've started 'borrowing' from the other safehouses.", h.m_sName, m_Arc.S(1)));
			}

			return;
		}

		// Bells at the gate
		m_GM.GetHeat().AddHeat(h.m_vPos, 1);

		// It always ends: if nobody helps, he walks on and takes a couple of them with him
		if (StepAge() >= CONGREGATION_SECONDS)
		{
			RZ_Safehouses houses = m_GM.GetSafehouses();
			if (houses)
				houses.RemovePopulation(m_Arc.I(0), 2);

			EndCongregation(string.Format("%1 left the safehouse in %2. Two of them went with him. The rest are eating again.", m_Arc.S(1), h.m_sName));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void EndCongregation(string headline)
	{
		End(headline);
	}
}

//------------------------------------------------------------------------------------------------
//! The Deserter's Map (story arc 4): a world event. A dying deserter's body holds one piece of a
//! map; two living wanderers hold the others (talking to them while it runs gives them up).
//! With all three read, the bunker is at a grid: reach it and its stores are yours (a reward
//! crate, guarded). Lasts an hour.
class RZ_Mission_DeserterMap : RZ_PuzzleMission
{
	protected vector m_vBody;
	protected vector m_vBunker;
	protected ref array<int> m_aHolders = {};
	protected ref array<bool> m_aGiven = {};
	protected bool m_bFirst;
	protected bool m_bRevealed;
	protected bool m_bGuarded;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Only the story arcs start it.
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Wanderers wanderers = gm.GetWanderers();
		if (!wanderers)
			return false;

		int mask = RZ_Wanderers.AllRoles() - RZ_Wanderers.RoleBit(RZ_EWandererRole.KID);
		array<int> people = {};
		wanderers.GetLiving(mask, people);
		if (people.Count() < 2)
			return false;

		m_aHolders.Insert(people[0]);
		m_aHolders.Insert(people[1]);
		m_aGiven.Insert(false);
		m_aGiven.Insert(false);

		m_iSettlement = PickSettlement(gm, 0, 70);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vBody = LandPointNear(s.m_vCenter, s.m_fRadius, s.m_fRadius + 150);
		RZ_Settlement distant = gm.GetSettlement(PickSettlementAway(gm, s.m_vCenter, 1500, 5000, 0, 100));
		if (!distant)
			return false;

		m_vBunker = HighPointNear(distant.m_vCenter, distant.m_fRadius + 300, distant.m_fRadius + 700);
		if (RZ_Util.IsWater(m_vBunker))
			return false;

		int piece = AddClue("A piece of a map, sewn into a jacket", string.Format("Pencil on oilcloth: a river, a road, and an X just off the edge. In the corner, in a soldier's hand: \"The rest is with %1 and %2. I paid them to keep it. Don't let Bastion have it back.\"",
			wanderers.GetName(m_aHolders[0]), wanderers.GetName(m_aHolders[1])));
		SpawnClue(gm, RZ_EPuzzleProp.BODY, m_vBody, piece, "Search the deserter's jacket");

		m_vPos = m_vBody;
		m_sTitle = "The Deserter's Map";
		m_sBrief = string.Format("A Bastion deserter died by the road outside %1 with part of a map on him. People say it leads to a bunker Bastion never told anyone about.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		if (m_bFirst)
			return;

		m_bFirst = true;
		RZ_Wanderers wanderers = gm.GetWanderers();
		if (!wanderers)
			return;

		gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 is %2. %3 is %4. Talk to them.",
			wanderers.GetName(m_aHolders[0]), wanderers.WhereIs(m_aHolders[0]), wanderers.GetName(m_aHolders[1]), wanderers.WhereIs(m_aHolders[1])));
	}

	//------------------------------------------------------------------------------------------------
	//! From the story arcs, when a wanderer is talked to: their piece of the map, once the first
	//! piece was found. "" if they don't hold one (or it's already given).
	string WandererPiece(RZ_GameModeComponent gm, int playerId, int wandererId)
	{
		int which = m_aHolders.Find(wandererId);
		if (which < 0 || !m_bFirst || m_eState != RZ_EMissionState.ACTIVE)
			return "";

		AddParticipant(playerId);
		if (m_aGiven[which])
			return "\"I gave my piece away already. Ask around.\"";

		m_aGiven[which] = true;
		string text = "They look around before they open their coat. \"He paid me to keep this. He's dead, then.\" A strip of oilcloth: a ridge line and a number.";
		if (m_aGiven[0] && m_aGiven[1])
		{
			m_bRevealed = true;
			text += string.Format(" Laid next to the others, it fits: the X is at grid %1.", RZ_SettlementScanner.GridReference(m_vBunker));
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_Wanderers wanderers = gm.GetWanderers();
		foreach (int i, int holder : m_aHolders)
		{
			if (!m_aGiven[i] && (!wanderers || !wanderers.IsAlive(holder)))
			{
				m_eState = RZ_EMissionState.FAILED;
				return;
			}
		}

		if (!m_bRevealed)
			return;

		if (!m_bGuarded && CountPlayersNear(players, m_vBunker, 400) > 0)
		{
			m_bGuarded = true;
			SpawnGuards(gm, m_vBunker, 4, 20, 60);
		}

		if (ReachedBy(ids, players, m_vBunker, 15))
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		SpawnRewardAt(gm, m_vBunker);
		NotifyParticipants(gm, m_sTitle, "Under the brush: a steel hatch, and Bastion's stencils on the crates inside.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "Someone found the bunker on the deserter's map.";
	}
}

//------------------------------------------------------------------------------------------------
//! Fever Town (story arc 5), step 2: Halcyon checks the water, starting at the sick safehouse's
//! town. The Water Survey with a fixed first well.
class RZ_Mission_FeverWater : RZ_Mission_WaterSurvey
{
	protected int m_iTown = -1;

	//------------------------------------------------------------------------------------------------
	void Prepare(int settlement)
	{
		m_iTown = settlement;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(m_iTown);
		if (!s)
			return false;

		m_iSettlement = m_iTown;
		m_vPos = s.m_vCenter;
		m_fParticipationRadius = 0;
		AddPoint(LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.4));

		array<int> used = {m_iTown};
		int guard = 0;
		while (m_aPoints.Count() < 3 && guard < 20)
		{
			guard++;
			int idx = PickSettlementAway(gm, s.m_vCenter, 500, 3500, 0, 100);
			RZ_Settlement other = gm.GetSettlement(idx);
			if (!other || used.Contains(idx))
				continue;

			used.Insert(idx);
			AddPoint(LandPointNear(other.m_vCenter, 0, other.m_fRadius * 0.4));
		}

		if (m_aPoints.Count() < 3)
			return false;

		m_sTitle = "Fever Town: the water";
		m_sBrief = string.Format("People at the safehouse in %1 are running fevers and nobody's been bitten. Halcyon wants samples from the well there and from two towns nearby. Stand at each for 10 seconds.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		NotifyParticipants(gm, "Halcyon", "The samples from the safehouse well are bad. The others are clean.");
	}
}

//------------------------------------------------------------------------------------------------
//! Fever Town (story arc 5), step 3: the Haven gets the sick and the old out. Hold the
//! safehouse for two minutes (two waves) while they're carried out; done, two of them go to
//! another safehouse (or stay, with the well boarded up). Fails: two of them die.
class RZ_Mission_FeverEvacuation : RZ_Mission_Hold
{
	protected int m_iHouse = -1;

	//------------------------------------------------------------------------------------------------
	void Prepare(int house)
	{
		m_iHouse = house;
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 12, 15);
		m_Req.Bonus("nurse");
		m_Req.Bonus("doctor");
		m_fBonusHoldSpeed = 1.5;
		m_fHoldNeeded = 120;
		m_fHoldRadius = 40;
		m_fWaveInterval = 55;
		m_iMaxWaves = 2;
		m_fWaveTimer = 30;
		m_fTimeLeft = 1800;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses)
			return false;

		RZ_Safehouse h = houses.Get(m_iHouse);
		if (!h)
			return false;

		m_iSettlement = h.m_iSettlement;
		m_vPos = h.m_vPos;
		m_fParticipationRadius = 80;
		m_sTitle = "Fever Town: get them out";
		m_sBrief = string.Format("The well at the safehouse in %1 is poisoned. The old and the sick can't walk far. Hold the gate for two minutes while we carry them out.", h.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		RZ_Safehouse h = houses.Get(m_iHouse);
		if (!h || houses.Count() < 2)
			return;

		// The vulnerable go to the next nearest safehouse with room
		houses.RemovePopulation(m_iHouse, 2);
		houses.AddPopulationExcept(h.m_vPos, 2, m_iHouse);
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailed(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses)
			houses.RemovePopulation(m_iHouse, 2);
	}
}
