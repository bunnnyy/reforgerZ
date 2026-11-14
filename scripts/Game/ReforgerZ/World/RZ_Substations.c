//! The grid past the station (docs/DESIGN.md, "The grid, region by region"): the map's own transformer
//! yards and substations near towns outside the station's region. Each is switched on in person (a
//! fuse fitted from a coil of wire, a breaker closed) and carries the station's power to the towns
//! round it while the station runs. Lines from a yard to a town break (wear, a storm, the infected) and
//! are spliced at a real pole with wire; the Red Hand cut one now and then and charge to put it back.
//! A lit region is worth something to the factions (Halcyon's lab works faster, the Haven's and
//! Bastion's morale), lit streets at night keep the infected off a little but draw packs from further,
//! and anyone can cut a yard's breaker or a mast's cable for the Red Hand's pay, at a price in Infamy.
//!
//! Owned by RZ_PowerGrid (its settings are Power grid > Substations; its save is the grid's). Server
//! only; bodies (the yard's control, the break's spot, a toll crewman) only near players.

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class RZ_SubstationSettings
{
	[Attribute("1", desc: "Substations: the map's transformer yards near towns outside the station's region carry its power further, switched on in person; lines that break and are spliced; the Red Hand's tolls; sabotage (docs/DESIGN.md, The grid, region by region)")]
	bool m_bEnabled;

	[Attribute("6", UIWidgets.Slider, "At most this many substations", "0 10 1")]
	int m_iMax;

	[Attribute("1500", UIWidgets.Slider, "Towns within this many metres of a substation are on it", "300 4000 50")]
	float m_fReach;

	[Attribute("2000", UIWidgets.Slider, "Two substations stand at least this far apart (m)", "500 6000 100")]
	float m_fSpacing;

	[Attribute("12", UIWidgets.Slider, "Seconds of the hold that closes a substation's breaker (once a fuse is in)", "3 60 1")]
	float m_fSwitchSeconds;

	[Attribute("5", UIWidgets.Slider, "Seconds of the hold that fits a fuse in a substation's cabinet (a fuse: Crafting > Fuse, or found in garages)", "2 30 1")]
	float m_fFuseSeconds;

	[Attribute("0.8", UIWidgets.Slider, "The Haven's cold room: a safehouse in a lit town eats its stores this much as fast (the food keeps; 1 = off)", "0.5 1 0.05")]
	float m_fColdRoomMult;

	[Attribute("15", UIWidgets.Slider, "Seconds of the hold that splices a broken line", "3 60 1")]
	float m_fSpliceSeconds;

	[Attribute("8", UIWidgets.Slider, "Seconds of the hold that cuts a breaker or a mast's cable (it's loud)", "3 60 1")]
	float m_fCutSeconds;

	[Attribute("0.25", UIWidgets.Slider, "Breaks a live substation's lines take per hour with players online (doubled in a storm, half again with the infected stirred round it)", "0 2 0.05")]
	float m_fBreaksPerHour;

	[Attribute("0.3", UIWidgets.Slider, "Of the breaks at a substation with a Red Hand holding within 3 km, this share is the crews cutting it for a toll", "0 1 0.05")]
	float m_fTollShare;

	[Attribute("1.2", UIWidgets.Slider, "Halcyon's lab works this much faster while its town has power (capped at 1.5)", "1 1.5 0.05")]
	float m_fLabMult;

	[Attribute("1", UIWidgets.Slider, "Morale an hour for the Haven and Bastion while a town they hold has power", "0 5 0.25")]
	float m_fMoralePerHour;

	[Attribute("6", UIWidgets.Slider, "At most this much morale a real day from the power, each", "0 20 1")]
	float m_fMoraleDayCap;

	[Attribute("0.85", UIWidgets.Slider, "At night the infected in a lit town are this many (lit streets keep them off a little)", "0.5 1 0.05")]
	float m_fLitSpawnMult;

	[Attribute("1", UIWidgets.Slider, "Heat a minute a lit town makes at night (the lamps draw packs from further)", "0 10 0.25")]
	float m_fLitHeatPerMinute;

	[Attribute("8", UIWidgets.Slider, "Infamy with each faction whose town or mast a cut darkens", "0 40 1")]
	int m_iCutInfamy;

	[Attribute("10", UIWidgets.Slider, "Red Hand Favor for a rival's yard or mast cut (once a real day per player)", "0 40 1")]
	int m_iCutFavor;
}

//------------------------------------------------------------------------------------------------
class RZ_SubstationSave
{
	float x;
	float z;
	string name;
	bool on;
	int broken = -1;		// the town index whose line is down, -1 = none
	float bx;
	float bz;
	int kind;
	string keeper;
	bool fused;				// a fuse is in (an old save: whatever was on)
	bool cut;				// its breaker was cut on purpose (a rival's job to put it back)
}

//------------------------------------------------------------------------------------------------
class RZ_SubstationsSave
{
	ref array<ref RZ_SubstationSave> list = {};
	ref array<string> paid = {};	// "<identity>|<day>": a cut paid by the Red Hand that day
}

//------------------------------------------------------------------------------------------------
class RZ_Substation
{
	int m_iId;
	vector m_vPos;				// the yard
	vector m_vControl;			// its cabinet's face (a spot) or our control
	float m_fYaw;
	bool m_bSpot;				// on the map's own cabinet: an invisible spot
	string m_sName;				// "the substation in Levie"
	ref array<int> m_aTowns = {};
	bool m_bOn;
	bool m_bFused;				// a fuse is in: the breaker can be closed
	bool m_bCut;				// cut on purpose and not yet put back (RZ_Mission_LightsBack)
	int m_iCutBy;				// this session's player who cut it last (0 none)
	int m_iOnBy;				// and who closed its breaker last
	int m_iBroken = -1;			// the town whose line is down
	vector m_vBreak;
	int m_iKind;				// RZ_Substations.BREAK_*
	string m_sKeeper;			// the toll crewman's name
	bool m_bPole;				// the break is at one of the map's own poles (else at the yard)

	IEntity m_Body;				// weak: the control
	IEntity m_BreakBody;		// weak: the spot at the break
	IEntity m_Crew;				// weak: the toll crewman
	RZ_PuzzleObjectComponent m_Comp;
}

//------------------------------------------------------------------------------------------------
//! A substation's or a break's object menu (RZ_ObjectScreen.OpenRows).
class RZ_SubstationRows : RZ_ObjectRows
{
	protected int m_iSub;
	protected bool m_bBreak;
	protected ref array<string> m_aActs = {};
	protected ref array<string> m_aLabels = {};
	protected ref array<string> m_aLocks = {};

	//------------------------------------------------------------------------------------------------
	void RZ_SubstationRows(int sub, bool atBreak)
	{
		m_iSub = sub;
		m_bBreak = atBreak;
	}

	//------------------------------------------------------------------------------------------------
	protected void Build(IEntity user)
	{
		m_aActs.Clear();
		m_aLabels.Clear();
		m_aLocks.Clear();
		RZ_Substations subs = RZ_Substations.Get();
		if (subs)
			subs.Rows(m_iSub, m_bBreak, user, m_aActs, m_aLabels, m_aLocks);
	}

	//------------------------------------------------------------------------------------------------
	override void Labels(notnull array<string> labels)
	{
		Build(null);
		labels.Copy(m_aLabels);
	}

	//------------------------------------------------------------------------------------------------
	override bool Locked(int index, IEntity user, out string reason, out string tag)
	{
		Build(user);
		if (index < 0 || index >= m_aLocks.Count() || m_aLocks[index].IsEmpty())
			return false;

		reason = m_aLocks[index];
		tag = "LOCKED";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Pick(int index, IEntity user)
	{
		Build(user);
		if (index < 0 || index >= m_aActs.Count())
			return;

		RZ_Substations subs = RZ_Substations.Get();
		if (subs)
			subs.Act(m_iSub, m_aActs[index], user);
	}

	//------------------------------------------------------------------------------------------------
	override string Text()
	{
		RZ_Substations subs = RZ_Substations.Get();
		if (!subs)
			return "";

		return subs.TextOf(m_iSub, m_bBreak);
	}
}

//------------------------------------------------------------------------------------------------
class RZ_Substations
{
	static const int TAG = -31;				// RZ_PuzzleObjectComponent / RZ_ClueComponent mission id: a substation's control
	static const int TAG_BREAK = -32;		// the spot at a broken line

	static const int BREAK_NONE = 0;
	static const int BREAK_WEAR = 1;
	static const int BREAK_STORM = 2;
	static const int BREAK_INFECTED = 3;
	static const int BREAK_TOLL = 4;		// the Red Hand cut it, and charge to put it back

	protected static const float VISIBLE_RADIUS = 300;
	protected static const float DESPAWN_MARGIN = 50;
	protected static const float STATION_GAP = 600;		// a yard this close to the station is the station's own
	protected static const float POLE_SEARCH = 90;		// a pole this close to the line's middle carries the break
	protected static const float TOLL_REACH = 3000;		// a Red Hand holding this close to a yard may cut its lines
	protected static const float TOWN_MARGIN = 150;		// a lit town's lamps, this far past its edge
	protected static const float CUT_HEAT = 8;
	protected static const int TOLL_TINS = 2;
	protected static const string SWITCH = "grid.switch.";
	protected static const string FUSE = "grid.fuse.";
	protected static const string SPLICE = "grid.splice.";
	protected static const string CUT = "grid.cut.";

	static int s_iTestHurt = -1;		// the runtime test only: the faction that holds every substation's towns (a test map may have none there)
	static bool s_bTestNoParts;			// the runtime test only: a fuse or a splice needs no wire (its items don't go into a pack there)

	protected RZ_GameModeComponent m_GM;
	protected RZ_PowerGrid m_Grid;
	protected RZ_SubstationSettings m_Settings;
	protected ref RZ_SubstationsSave m_Restored;
	protected ref array<ref RZ_Substation> m_aSubs = {};
	protected ref array<string> m_aPaid = {};
	protected ref array<string> m_aContracted = {};	// "sub:<id>" / "mast:<id>": a Lights Out job pays for this cut, not the book's tip
	protected ref map<int, float> m_mMoraleToday = new map<int, float>();
	protected int m_iMoraleDay = -1;
	protected bool m_bOn;

	//------------------------------------------------------------------------------------------------
	void RZ_Substations(RZ_GameModeComponent gm, RZ_PowerGrid grid, RZ_SubstationSettings settings)
	{
		m_GM = gm;
		m_Grid = grid;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_Substations Get()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetPowerGrid())
			return null;

		return gm.GetPowerGrid().GetSubstations();
	}

	bool IsOn() { return m_bOn; }
	RZ_SubstationSettings GetSettings() { return m_Settings; }

	//------------------------------------------------------------------------------------------------
	//! The real day (Unix days): the Red Hand's pay and the morale cap reset with it.
	protected static int RealDay()
	{
		return RZ_Guidance.UnixNow() / 86400;
	}
	int Count() { return m_aSubs.Count(); }

	//------------------------------------------------------------------------------------------------
	RZ_Substation GetSub(int index)
	{
		if (index < 0 || index >= m_aSubs.Count())
			return null;

		return m_aSubs[index];
	}

	// ================================================================ Hooks
	//------------------------------------------------------------------------------------------------
	//! A town on a switched-on substation whose line is whole (the station's running is the grid's to ask).
	bool LitAt(vector pos)
	{
		if (!m_bOn)
			return false;

		foreach (RZ_Substation s : m_aSubs)
		{
			if (!s.m_bOn)
				continue;

			foreach (int idx : s.m_aTowns)
			{
				if (idx == s.m_iBroken)
					continue;

				RZ_Settlement town = m_GM.GetSettlement(idx);
				if (town && RZ_Util.DistXZ(pos, town.m_vCenter) <= town.m_fRadius + TOWN_MARGIN)
					return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The lamps' circles for the towns the substations light (";x z r" each), "" when none.
	string LampCircles()
	{
		string list = "";
		if (!m_bOn || !m_Grid.IsPowered())
			return list;

		foreach (RZ_Substation s : m_aSubs)
		{
			if (!s.m_bOn)
				continue;

			foreach (int idx : s.m_aTowns)
			{
				RZ_Settlement town = m_GM.GetSettlement(idx);
				if (!town || idx == s.m_iBroken)
					continue;

				list += string.Format(";%1 %2 %3", RZ_Util.RoundToInt(town.m_vCenter[0]), RZ_Util.RoundToInt(town.m_vCenter[2]), RZ_Util.RoundToInt(town.m_fRadius + TOWN_MARGIN));
			}
		}

		return list;
	}

	//------------------------------------------------------------------------------------------------
	//! Halcyon's lab works faster while its town has power (Cure/RZ_CureResearch.c).
	static float LabMult(RZ_GameModeComponent gm)
	{
		if (!gm || !gm.HasLab() || !gm.GetPowerGrid())
			return 1;

		RZ_Substations subs = gm.GetPowerGrid().GetSubstations();
		if (!subs || !subs.m_bOn || !RZ_PowerGrid.IsGridPoweredAt(gm.GetLabPosition()))
			return 1;

		return Math.Clamp(subs.m_Settings.m_fLabMult, 1, 1.5);
	}

	//------------------------------------------------------------------------------------------------
	//! At night the infected in a lit town are a few less (Zombies/RZ_ZombieSpawner.c).
	static float LitStreetsMult(vector townCenter)
	{
		RZ_Substations subs = Get();
		if (!subs || !subs.m_bOn || !RZ_GameTime.IsNight() || !RZ_PowerGrid.IsGridPoweredAt(townCenter))
			return 1;

		return Math.Clamp(subs.m_Settings.m_fLitSpawnMult, 0.5, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! The Red Hand pay for a rival's light or voice cut, once a real day per player (masts too).
	//! The faction (not the Red Hand) holding a town on substation `index`: who its lights are for, and
	//! who minds a cut. -1 when none does.
	int HurtFaction(int index)
	{
		RZ_Substation s = GetSub(index);
		if (s && s_iTestHurt >= 0)
			return s_iTestHurt;

		RZ_Territory territory = m_GM.GetTerritory();
		if (!s || !territory)
			return -1;

		foreach (RZ_Holding h : territory.GetHoldings())
		{
			if (h && h.m_eFaction != RZ_EFaction.BANDITS && s.m_aTowns.Contains(h.m_iSettlement))
				return h.m_eFaction;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! A Lights Out job (Missions/RZ_PowerJobs.c) pays for this cut: the book's own tip isn't paid too.
	void SetContracted(string key, bool on)
	{
		if (on && !m_aContracted.Contains(key))
			m_aContracted.Insert(key);
		else if (!on)
			m_aContracted.RemoveItem(key);
	}

	//------------------------------------------------------------------------------------------------
	bool IsContracted(string key)
	{
		return m_aContracted.Contains(key);
	}

	//------------------------------------------------------------------------------------------------
	//! The Haven's cold room (docs/DESIGN.md, "The grid, region by region"): a safehouse whose town
	//! has the grid's power eats its stores this much as fast (RZ_Safehouses' upkeep). 1 elsewhere.
	static float ColdRoomMult(vector pos)
	{
		RZ_Substations subs = Get();
		if (!RZ_PowerGrid.IsPoweredAt(pos))
			return 1;

		if (!subs || !subs.m_Settings)
			return 0.8;

		return Math.Clamp(subs.m_Settings.m_fColdRoomMult, 0.5, 1);
	}

	//------------------------------------------------------------------------------------------------
	static void PayCutter(RZ_GameModeComponent gm, int playerId, string what)
	{
		RZ_Substations subs = Get();
		if (!subs || !gm || subs.m_Settings.m_iCutFavor <= 0)
			return;

		RZ_PlayerProfile p = gm.GetProfile(playerId);
		if (!p || p.m_sIdentity.IsEmpty())
			return;

		string key = p.m_sIdentity + "|" + RealDay().ToString();
		if (subs.m_aPaid.Contains(key))
			return;

		subs.m_aPaid.Insert(key);
		gm.AddFavor(playerId, RZ_EFaction.BANDITS, subs.m_Settings.m_iCutFavor);
		gm.Say(playerId, "Radio: the Red Hand's channel", RZ_EFaction.BANDITS, string.Format("Saw %1 go. The book's paid you. Don't spend it all on candles.", what));
	}

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	void Restore(RZ_SubstationsSave saved)
	{
		m_Restored = saved;
	}

	//------------------------------------------------------------------------------------------------
	RZ_SubstationsSave Save()
	{
		RZ_SubstationsSave data = new RZ_SubstationsSave();
		foreach (RZ_Substation s : m_aSubs)
		{
			RZ_SubstationSave e = new RZ_SubstationSave();
			e.x = s.m_vPos[0];
			e.z = s.m_vPos[2];
			e.name = s.m_sName;
			e.on = s.m_bOn;
			e.broken = s.m_iBroken;
			e.bx = s.m_vBreak[0];
			e.bz = s.m_vBreak[2];
			e.kind = s.m_iKind;
			e.keeper = s.m_sKeeper;
			e.fused = s.m_bFused;
			e.cut = s.m_bCut;
			data.list.Insert(e);
		}

		// Only today's pay is kept
		string today = "|" + RealDay().ToString();
		foreach (string key : m_aPaid)
		{
			if (key.EndsWith(today))
				data.paid.Insert(key);
		}

		return data;
	}

	// ================================================================ Setup
	//------------------------------------------------------------------------------------------------
	//! After the grid has its station and region.
	void Setup(vector station, float region)
	{
		if (!m_Settings || !m_Settings.m_bEnabled || m_Settings.m_iMax <= 0)
		{
			RZ_Log.Info("Substations: off.");
			return;
		}

		Find(station, region);
		if (m_Restored)
		{
			foreach (RZ_SubstationSave e : m_Restored.list)
			{
				RZ_Substation match = Nearest(Vector(e.x, 0, e.z), 150);
				if (!match)
					continue;

				match.m_bOn = e.on;
				match.m_bFused = e.fused || e.on;
				match.m_bCut = e.cut && !e.on;
				match.m_iKind = e.kind;
				match.m_sKeeper = e.keeper;
				if (e.broken >= 0 && match.m_aTowns.Contains(e.broken))
				{
					match.m_iBroken = e.broken;
					match.m_vBreak = RZ_Util.GroundPos(Vector(e.bx, 0, e.bz));
					match.m_bPole = RZ_Util.DistXZ(match.m_vBreak, match.m_vPos) > 30;
				}
			}

			if (m_Restored.paid)
				m_aPaid.Copy(m_Restored.paid);
		}

		m_Restored = null;
		m_bOn = !m_aSubs.IsEmpty();
		foreach (RZ_Substation s : m_aSubs)
		{
			RZ_Log.Info(string.Format("Substations: %1 (%2) at %3, %4 towns (%5), %6.", s.m_sName, s.m_iId, RZ_SettlementScanner.GridReference(s.m_vPos), s.m_aTowns.Count(), TownNames(s), StateWord(s)));
		}

		RZ_Log.Info(string.Format("Substations: %1 on this map, %2 switched on.", m_aSubs.Count(), OnCount()));
		ApplyTowns();
	}

	//------------------------------------------------------------------------------------------------
	//! The map's power infrastructure near each town outside the station's region (biggest towns
	//! first): a substation building, a transformer yard, a lone transformer; spread out, on dry
	//! mainland, off the line. No such thing on the map: no substations, and the station alone.
	protected void Find(vector station, float region)
	{
		array<ref RZ_Settlement> towns = m_GM.GetSettlements();
		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		for (int size = 2; size >= 0; size--)
		{
			foreach (int i, RZ_Settlement t : towns)
			{
				if (m_aSubs.Count() >= m_Settings.m_iMax)
					return;

				if (t.m_iSize != size || RZ_Util.DistXZ(t.m_vCenter, station) <= region)
					continue;

				int score;
				IEntity yard = RZ_PowerSite.FindBest(t.m_vCenter, t.m_fRadius + 800, RZ_PowerSite.MODE_POWER, score);
				if (!yard)
					continue;

				vector at = RZ_Util.GroundPos(yard.GetOrigin());
				if (RZ_Util.DistXZ(at, station) < STATION_GAP || RZ_Util.IsWater(at) || !RZ_Mainland.OnMainland(at))
					continue;

				if (perimeter && perimeter.IsNearLine(at))
					continue;

				if (Nearest(at, m_Settings.m_fSpacing))
					continue;

				bool inTown = RZ_Util.DistXZ(at, t.m_vCenter) < t.m_fRadius;
				Add(yard, at, RZ_PowerSite.NameFor(RZ_PowerSite.MODE_POWER, score, t.m_sName, inTown), station, region);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Add(IEntity yard, vector at, string name, vector station, float region)
	{
		RZ_Substation s = new RZ_Substation();
		s.m_iId = m_aSubs.Count();
		s.m_vPos = at;
		s.m_sName = name;
		foreach (int idx, RZ_Settlement t : m_GM.GetSettlements())
		{
			// The station's own towns stay the station's
			if (RZ_Util.DistXZ(t.m_vCenter, station) <= region)
				continue;

			if (RZ_Util.DistXZ(t.m_vCenter, at) <= m_Settings.m_fReach)
				s.m_aTowns.Insert(idx);
		}

		if (s.m_aTowns.IsEmpty())
			return;

		// Its control: the map's own cabinet in the yard when there is one (an invisible spot on its
		// face), else the grid's control prefab beside it
		vector toward = at;
		RZ_Settlement first = m_GM.GetSettlement(s.m_aTowns[0]);
		if (first)
			toward = first.m_vCenter;

		RZ_PowerPlacement p = new RZ_PowerPlacement();
		RZ_PowerSite.Place(yard, at, toward, m_Grid.CanUseCabinetsPublic(), 45, p);
		s.m_vControl = p.m_vPos;
		s.m_fYaw = p.m_fYaw;
		s.m_bSpot = p.m_bSpot;
		m_aSubs.Insert(s);
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Substation Nearest(vector at, float within)
	{
		foreach (RZ_Substation s : m_aSubs)
		{
			if (RZ_Util.DistXZ(s.m_vPos, at) < within)
				return s;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected int OnCount()
	{
		int n = 0;
		foreach (RZ_Substation s : m_aSubs)
		{
			if (s.m_bOn)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	string TownNames(RZ_Substation s)
	{
		string text = "";
		int shown = Math.Min(s.m_aTowns.Count(), 3);
		for (int i = 0; i < shown; i++)
		{
			RZ_Settlement t = m_GM.GetSettlement(s.m_aTowns[i]);
			if (!t)
				continue;

			if (i > 0 && i == shown - 1)
				text += " and ";
			else if (i > 0)
				text += ", ";

			text += t.m_sName;
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	protected string StateWord(RZ_Substation s)
	{
		if (!s.m_bOn)
			return "switched off";

		if (s.m_iBroken >= 0)
			return "on, with a line down to " + TownName(s.m_iBroken);

		return "on";
	}

	//------------------------------------------------------------------------------------------------
	protected string TownName(int idx)
	{
		RZ_Settlement t = m_GM.GetSettlement(idx);
		if (t)
			return t.m_sName;

		return "the town";
	}

	//------------------------------------------------------------------------------------------------
	//! Town memory's power flags and the lamps, after anything changed (the station, a switch, a break).
	void ApplyTowns()
	{
		RZ_TownMemory mem = m_GM.GetTownMemory();
		if (mem)
		{
			bool station = m_Grid.IsPowered();
			foreach (RZ_Substation s : m_aSubs)
			{
				foreach (int idx : s.m_aTowns)
				{
					mem.SetPowered(idx, station && s.m_bOn && idx != s.m_iBroken);
				}
			}
		}

		m_Grid.RefreshLamps();
	}

	// ================================================================ The menu
	//------------------------------------------------------------------------------------------------
	string TextOf(int index, bool atBreak)
	{
		RZ_Substation s = GetSub(index);
		if (!s)
			return "";

		if (atBreak)
		{
			string where = "The line to " + TownName(s.m_iBroken);
			if (s.m_iKind == BREAK_TOLL)
				return where + " has been cut clean, and the end tied off neat. " + s.m_sKeeper + " sits under it with a tin of his own, waiting for someone who wants the lights back.";

			if (s.m_iKind == BREAK_STORM)
				return where + " came down in the wind and hangs off the crossarm. It wants splicing.";

			if (s.m_iKind == BREAK_INFECTED)
				return where + " is down: something hit the pole hard enough to snap the stay, and the wire went with it. It wants splicing.";

			return where + " has parted where it was old. It wants splicing.";
		}

		string text = string.Format("A cabinet with a breaker for each line out: %1.", TownNames(s));
		if (!s.m_bOn && !s.m_bFused)
			text += " The breaker's open and the fuse holder's empty. It wants a fuse before anything else.";
		else if (!s.m_bOn)
			text += " A fuse is in. The breaker's still open.";
		else if (!m_Grid.IsPowered())
			text += string.Format(" The breaker's closed, but nothing comes in: %1 isn't running.", m_Grid.GetName());
		else
			text += " It hums.";

		if (s.m_iBroken >= 0)
			text += string.Format(" The line to %1 reads dead.", TownName(s.m_iBroken));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	void Rows(int index, bool atBreak, IEntity user, notnull array<string> acts, notnull array<string> labels, notnull array<string> locks)
	{
		RZ_Substation s = GetSub(index);
		if (!s)
			return;

		if (atBreak)
		{
			if (s.m_iBroken < 0)
				return;

			string why = "";
			if (user && RZ_Masts.CountPart(user, RZ_EMastPart.WIRE) < 1)
				why = "You haven't got any wire on you.";

			acts.Insert("splice");
			labels.Insert("Splice the line");
			locks.Insert(why);
			if (s.m_iKind == BREAK_TOLL)
			{
				string tins = "";
				if (user && RZ_Errands.CountItems(user, RZ_EErrandItem.FOOD) < TOLL_TINS)
					tins = RZ_Errands.ItemNeed(RZ_EErrandItem.FOOD, TOLL_TINS);

				acts.Insert("toll");
				labels.Insert(string.Format("Pay %1 two tins to put it back", s.m_sKeeper));
				locks.Insert(tins);
			}

			return;
		}

		if (!s.m_bOn && !s.m_bFused)
		{
			string fuse = "";
			if (user && RZ_Masts.CountPart(user, RZ_EMastPart.FUSE) < 1)
				fuse = "You haven't got a fuse. A bench makes one from wire and scrap, and garages have them.";

			acts.Insert("fuse");
			labels.Insert("Fit a fuse");
			locks.Insert(fuse);
			return;
		}

		if (!s.m_bOn)
		{
			acts.Insert("switch");
			labels.Insert("Close the breaker");
			locks.Insert("");
			return;
		}

		acts.Insert("cut");
		labels.Insert("Cut the breaker");
		locks.Insert("");
	}

	//------------------------------------------------------------------------------------------------
	void Act(int index, string act, IEntity user)
	{
		RZ_Substation s = GetSub(index);
		if (!s || !user)
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		if (playerId <= 0)
			return;

		if (act == "fuse" && !s.m_bOn && !s.m_bFused && (s_bTestNoParts || RZ_Masts.CountPart(user, RZ_EMastPart.FUSE) >= 1))
			OpenHold(FUSE, s, "Fit a fuse", m_Settings.m_fFuseSeconds, s.m_vControl, playerId);
		else if (act == "switch" && !s.m_bOn && s.m_bFused)
			OpenHold(SWITCH, s, "Close the breaker", m_Settings.m_fSwitchSeconds, s.m_vControl, playerId);
		else if (act == "cut" && s.m_bOn)
			OpenHold(CUT, s, "Cut the breaker", m_Settings.m_fCutSeconds, s.m_vControl, playerId);
		else if (act == "splice" && s.m_iBroken >= 0 && RZ_Masts.CountPart(user, RZ_EMastPart.WIRE) >= 1)
			OpenHold(SPLICE, s, "Splice the line", m_Settings.m_fSpliceSeconds, s.m_vBreak, playerId);
		else if (act == "toll" && s.m_iKind == BREAK_TOLL)
			PayToll(s, playerId, user);
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenHold(string prefix, RZ_Substation s, string label, float seconds, vector at, int playerId)
	{
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (!sites || !RZ_WorkSites.Available())
		{
			Done(prefix, s, playerId);
			return;
		}

		string key = prefix + s.m_iId.ToString();
		sites.Close(key);
		sites.Open(key, RZ_EWork.HANDS, label, seconds, at, playerId);
		// At the cabinet or the pole itself: nothing more is put out
		sites.SetAtPiece(key, at, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! "Look at the substation" / "Look at the line": the menu, else its text and its rows as prompts.
	void HandleRead(int playerId, IEntity owner, bool atBreak)
	{
		RZ_Substation s = SubOf(owner, atBreak);
		if (!s)
			return;

		string title = Capitalize(s.m_sName);
		if (atBreak)
			title = "The line to " + TownName(s.m_iBroken);

		if (RZ_ObjectScreen.Available() && RZ_ObjectScreen.OpenRows(playerId, owner, title, new RZ_SubstationRows(s.m_iId, atBreak)))
			return;

		m_GM.NotifyPlayer(playerId, title, TextOf(s.m_iId, atBreak));
	}

	//------------------------------------------------------------------------------------------------
	//! Without the object menu: the first rows as the prop's prompts.
	void HandleChoice(int playerId, int option, IEntity owner, IEntity user, bool atBreak)
	{
		RZ_Substation s = SubOf(owner, atBreak);
		if (!s)
			return;

		array<string> acts = {};
		array<string> labels = {};
		array<string> locks = {};
		Rows(s.m_iId, atBreak, user, acts, labels, locks);
		if (option < 0 || option >= acts.Count())
			return;

		if (!locks[option].IsEmpty())
		{
			m_GM.NotifyPlayer(playerId, Capitalize(s.m_sName), locks[option]);
			return;
		}

		Act(s.m_iId, acts[option], user);
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Substation SubOf(IEntity owner, bool atBreak)
	{
		if (!owner)
			return null;

		foreach (RZ_Substation s : m_aSubs)
		{
			if ((!atBreak && s.m_Body == owner) || (atBreak && s.m_BreakBody == owner))
				return s;
		}

		RZ_ClueComponent clue = RZ_ClueComponent.Cast(owner.FindComponent(RZ_ClueComponent));
		if (clue)
			return GetSub(clue.GetClueIndex());

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateLabels(RZ_Substation s)
	{
		if (!s.m_Comp)
			return;

		array<string> acts = {};
		array<string> labels = {};
		array<string> locks = {};
		Rows(s.m_iId, false, null, acts, labels, locks);
		string joined = "";
		for (int i = 0; i < labels.Count() && i < 4; i++)
		{
			if (i > 0)
				joined = joined + "|";

			joined = joined + labels[i];
		}

		s.m_Comp.SetLabels("", joined);
	}

	//------------------------------------------------------------------------------------------------
	protected static string Capitalize(string text)
	{
		if (text.IsEmpty())
			return text;

		string first = text.Substring(0, 1);
		first.ToUpper();
		return first + text.Substring(1, text.Length() - 1);
	}

	// ================================================================ What a hold does
	//------------------------------------------------------------------------------------------------
	protected void Done(string prefix, RZ_Substation s, int playerId)
	{
		if (prefix == FUSE)
			FitFuse(s, playerId, null);
		else if (prefix == SWITCH)
			SwitchOn(s, playerId, null);
		else if (prefix == SPLICE)
			Splice(s, playerId, null);
		else if (prefix == CUT)
			CutBreaker(s, playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! A fuse into the empty holder (the first of two steps): the breaker can be closed now.
	bool FitFuse(RZ_Substation s, int playerId, IEntity who)
	{
		IEntity ent = who;
		if (!ent)
			ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);

		if (s.m_bOn || s.m_bFused || !ent)
			return false;

		if (!s_bTestNoParts && RZ_Masts.TakePart(ent, RZ_EMastPart.FUSE, 1) < 1)
			return false;

		s.m_bFused = true;
		UpdateLabels(s);
		RZ_Log.Info(string.Format("Substations: %1 fitted a fuse at %2.", RZ_Names.Public(playerId), s.m_sName));
		m_GM.SayAt(playerId, Capitalize(s.m_sName), -1, "The fuse seats with a click. Now the breaker.", s.m_vControl);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The breaker closed over a fitted fuse: the towns on it light while the station runs.
	bool SwitchOn(RZ_Substation s, int playerId, IEntity who)
	{
		IEntity ent = who;
		if (!ent)
			ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);

		if (s.m_bOn || !s.m_bFused || !ent)
			return false;

		s.m_bOn = true;
		s.m_bCut = false;
		s.m_iOnBy = playerId;
		ApplyTowns();
		UpdateLabels(s);
		RZ_Log.Info(string.Format("Substations: %1 switched on %2 (%3).", RZ_Names.Public(playerId), s.m_sName, TownNames(s)));
		string line = string.Format("The breaker drops in with a clunk. If %1 is running, %2 have lights tonight.", m_Grid.GetName(), TownNames(s));
		if (m_Grid.IsPowered())
			line = string.Format("The breaker drops in with a clunk and the cabinet starts to hum. %1: lights tonight.", TownNames(s));

		m_GM.SayAt(playerId, Capitalize(s.m_sName), -1, line, s.m_vControl);
		if (m_Grid.IsPowered() && m_GM.GetLeaders())
			m_GM.GetLeaders().AddHeadline(string.Format("Lights in %1 again: somebody went out to %2 and got the breaker in.", TownNames(s), s.m_sName), false);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The broken line spliced with a length of wire: the town lights again.
	bool Splice(RZ_Substation s, int playerId, IEntity who)
	{
		IEntity ent = who;
		if (!ent)
			ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);

		if (s.m_iBroken < 0 || !ent)
			return false;

		if (!s_bTestNoParts && RZ_Masts.TakePart(ent, RZ_EMastPart.WIRE, 1) < 1)
			return false;

		string town = TownName(s.m_iBroken);
		bool toll = s.m_iKind == BREAK_TOLL;
		ClearBreak(s);
		RZ_Log.Info(string.Format("Substations: %1 spliced the line to %2 from %3.", RZ_Names.Public(playerId), town, s.m_sName));
		if (toll)
		{
			// Spliced past the toll: the crews notice
			m_GM.AddInfamy(playerId, RZ_EFaction.BANDITS, 3, "spliced past a Red Hand toll");
			m_GM.Say(playerId, "Radio: the Red Hand's channel", RZ_EFaction.BANDITS, string.Format("Somebody's spliced the line to %1 without paying. The book's got a page for people like that.", town));
			return true;
		}

		m_GM.SayAt(playerId, "The line to " + town, -1, string.Format("The splice holds. Down the hill the lamps in %1 come on, one street after another.", town), s.m_vBreak);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void PayToll(RZ_Substation s, int playerId, IEntity user)
	{
		if (s.m_iBroken < 0 || s.m_iKind != BREAK_TOLL)
			return;

		if (RZ_Errands.TakeItems(user, RZ_EErrandItem.FOOD, TOLL_TINS) < TOLL_TINS)
			return;

		string town = TownName(s.m_iBroken);
		string keeper = s.m_sKeeper;
		ClearBreak(s);
		RZ_Log.Info(string.Format("Substations: %1 paid the Red Hand's toll on the line to %2.", RZ_Names.Public(playerId), town));
		m_GM.Say(playerId, keeper + ", Red Hand crewman", RZ_EFaction.BANDITS, string.Format("Pleasure doing business. Lights in %1, soon as I've twisted these back. Tell them who to thank.", town), s.m_Crew);
	}

	//------------------------------------------------------------------------------------------------
	//! A breaker cut on purpose: the towns on it go dark, the factions there remember, the Red Hand pay.
	bool CutBreaker(RZ_Substation s, int playerId)
	{
		if (!s.m_bOn)
			return false;

		s.m_bOn = false;
		s.m_bFused = false;		// whoever cuts it pockets the fuse: putting it back takes a new one
		s.m_bCut = true;
		s.m_iCutBy = playerId;
		ApplyTowns();
		UpdateLabels(s);
		if (m_GM.GetHeat())
			m_GM.GetHeat().AddHeat(s.m_vControl, CUT_HEAT);

		// Every faction holding a town on it takes it badly (the Red Hand's own don't mind)
		array<int> hurt = {};
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory)
		{
			foreach (RZ_Holding h : territory.GetHoldings())
			{
				if (!h || h.m_eFaction == RZ_EFaction.BANDITS || hurt.Contains(h.m_eFaction) || !s.m_aTowns.Contains(h.m_iSettlement))
					continue;

				hurt.Insert(h.m_eFaction);
			}
		}

		foreach (int f : hurt)
		{
			if (m_Settings.m_iCutInfamy > 0)
				m_GM.AddInfamy(playerId, f, m_Settings.m_iCutInfamy, "cut the power to " + TownNames(s));
		}

		RZ_Log.Info(string.Format("Substations: %1 cut the breaker at %2 (%3 factions hurt).", RZ_Names.Public(playerId), s.m_sName, hurt.Count()));
		if (!hurt.IsEmpty() && !IsContracted("sub:" + s.m_iId.ToString()))
			PayCutter(m_GM, playerId, "the lights in " + TownNames(s));

		if (m_GM.GetLeaders())
			m_GM.GetLeaders().AddHeadline(string.Format("%1 went dark tonight. Somebody cut the breaker at %2.", TownNames(s), s.m_sName), false);

		return true;
	}

	// ================================================================ Breaks
	//------------------------------------------------------------------------------------------------
	//! A line from `s` breaks: one of its towns goes dark until someone splices it at a pole between
	//! (else at the yard), or pays the Red Hand. False when it can't (off, already broken).
	bool Break(RZ_Substation s, int kind)
	{
		if (!s.m_bOn || s.m_iBroken >= 0 || s.m_aTowns.IsEmpty())
			return false;

		int idx = s.m_aTowns.GetRandomElement();
		RZ_Settlement town = m_GM.GetSettlement(idx);
		if (!town)
			return false;

		s.m_iBroken = idx;
		s.m_iKind = kind;
		s.m_vBreak = PoleBetween(s.m_vPos, town.m_vCenter);
		s.m_bPole = s.m_vBreak != vector.Zero;
		if (!s.m_bPole)
			s.m_vBreak = s.m_vControl;

		if (kind == BREAK_TOLL)
		{
			RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 100000));
			s.m_sKeeper = RZ_NoteText.ManName(rng) + " " + RZ_NoteText.Surname(rng);
		}

		ApplyTowns();
		UpdateLabels(s);
		RZ_Log.Info(string.Format("Substations: the line from %1 to %2 is down (%3) at %4, %5.", s.m_sName, town.m_sName, KindWord(kind), RZ_SettlementScanner.GridReference(s.m_vBreak), PoleWord(s)));
		Announce(s, town);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static string KindWord(int kind)
	{
		switch (kind)
		{
			case BREAK_STORM: return "the storm";
			case BREAK_INFECTED: return "the infected";
			case BREAK_TOLL: return "the Red Hand's toll";
		}

		return "wear";
	}

	//------------------------------------------------------------------------------------------------
	protected static string PoleWord(RZ_Substation s)
	{
		if (s.m_bPole)
			return "at a pole";

		return "at the yard (no pole found)";
	}

	//------------------------------------------------------------------------------------------------
	//! One of the map's own power poles near the middle of the line from the yard to the town, else
	//! vector.Zero (the break is then the yard's).
	protected vector PoleBetween(vector from, vector to)
	{
		vector mid = (from + to) * 0.5;
		RZ_SubstationPoleQuery q = new RZ_SubstationPoleQuery();
		q.m_vCenter = mid;
		q.m_fBest = float.MAX;
		GetGame().GetWorld().QueryEntitiesBySphere(mid, POLE_SEARCH, q.OnEntity, null, EQueryEntitiesFlags.STATIC); // VERIFY: the map's power-line poles are static entities with their prefab's name (Powerline/PowerLine_*_pole*), also where a power-line generator placed them
		if (!q.m_Best)
			return vector.Zero;

		vector at = q.m_Best.GetOrigin();
		vector side = to - from;
		side[1] = 0;
		side.Normalize();
		// A step to the side of the pole's foot, out of its concrete
		return RZ_Util.GroundPos(at + Vector(side[2], 0, -side[0]) * 1.2);
	}

	//------------------------------------------------------------------------------------------------
	//! Who says the lights went out: the Red Hand's channel for a toll, else the open channel, to
	//! players with a radio round the town.
	protected void Announce(RZ_Substation s, RZ_Settlement town)
	{
		array<int> ids = {};
		GetGame().GetPlayerManager().GetPlayers(ids);
		foreach (int id : ids)
		{
			IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(id);
			if (!ent || RZ_Util.DistXZ(ent.GetOrigin(), town.m_vCenter) > m_Settings.m_fReach + town.m_fRadius)
				continue;

			if (!RZ_Radio.HasRadio(m_GM, id, ent))
				continue;

			if (s.m_iKind == BREAK_TOLL)
				m_GM.Say(id, "Radio: the Red Hand's channel", RZ_EFaction.BANDITS, string.Format("The lights in %1 are on the book now. Two tins and our man at the line puts them back. Or sit in the dark. Your choice.", town.m_sName));
			else
				m_GM.Say(id, "Radio: the open channel", -1, string.Format("Lights are out in %1. Line's down somewhere between us and %2. Anyone with a bit of wire, go and have a look.", town.m_sName, s.m_sName));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearBreak(RZ_Substation s)
	{
		s.m_iBroken = -1;
		s.m_iKind = BREAK_NONE;
		s.m_sKeeper = "";
		s.m_bPole = false;
		if (s.m_BreakBody)
			RZ_Util.DeleteUnit(s.m_BreakBody);

		if (s.m_Crew)
			RZ_Util.DeleteUnit(s.m_Crew);

		s.m_BreakBody = null;
		s.m_Crew = null;
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (sites)
			sites.Close(SPLICE + s.m_iId.ToString());

		ApplyTowns();
		UpdateLabels(s);
	}

	//------------------------------------------------------------------------------------------------
	//! The kind of break at a substation now: a toll when a Red Hand holding is near and the roll says
	//! so, a storm's in a storm, the infected's where they're stirred, else wear.
	protected int KindNow(RZ_Substation s)
	{
		if (m_Settings.m_fTollShare > 0 && Math.RandomFloat01() < m_Settings.m_fTollShare && RedHandNear(s.m_vPos))
			return BREAK_TOLL;

		RZ_Environment env = m_GM.GetEnvironment();
		if (env && env.IsStorm())
			return BREAK_STORM;

		if (m_GM.GetHeat() && m_GM.GetHeat().GetHeat(s.m_vPos) >= 30)
			return BREAK_INFECTED;

		return BREAK_WEAR;
	}

	//------------------------------------------------------------------------------------------------
	protected bool RedHandNear(vector at)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		if (!territory)
			return false;

		foreach (RZ_Holding h : territory.GetHoldings())
		{
			if (h && h.m_eFaction == RZ_EFaction.BANDITS && RZ_Util.DistXZ(h.m_vPos, at) <= TOLL_REACH)
				return true;
		}

		return false;
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	//! The grid's tick (every few seconds), whatever the station's step.
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bOn)
			return;

		bool live = m_Grid.IsPowered();
		RZ_Environment env = m_GM.GetEnvironment();
		bool storm = env && env.IsStorm();
		foreach (RZ_Substation s : m_aSubs)
		{
			UpdateBodies(s, players);
			TickHolds(s);
			if (!live || !s.m_bOn || s.m_iBroken >= 0 || ids.IsEmpty() || m_Settings.m_fBreaksPerHour <= 0)
				continue;

			float chance = m_Settings.m_fBreaksPerHour * dt / 3600;
			if (storm)
				chance = chance * 2;

			if (m_GM.GetHeat() && m_GM.GetHeat().GetHeat(s.m_vPos) >= 30)
				chance = chance * 1.5;

			if (Math.RandomFloat01() < chance)
				Break(s, KindNow(s));
		}

		if (!live)
			return;

		TickLitTowns(dt);
		TickMorale(dt, ids);
	}

	//------------------------------------------------------------------------------------------------
	//! At night every lit town's lamps draw the infected from further (a little heat at its middle).
	protected void TickLitTowns(float dt)
	{
		if (m_Settings.m_fLitHeatPerMinute <= 0 || !RZ_GameTime.IsNight() || !m_GM.GetHeat())
			return;

		foreach (RZ_Settlement t : m_GM.GetSettlements())
		{
			if (RZ_PowerGrid.IsGridPoweredAt(t.m_vCenter))
				m_GM.GetHeat().AddHeat(t.m_vCenter, m_Settings.m_fLitHeatPerMinute * dt / 60);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The Haven and Bastion are a little happier while a town they hold has power (capped a day).
	protected void TickMorale(float dt, array<int> ids)
	{
		RZ_FactionLife life = m_GM.GetFactionLife();
		RZ_Territory territory = m_GM.GetTerritory();
		if (!life || !territory || ids.IsEmpty() || m_Settings.m_fMoralePerHour <= 0)
			return;

		int day = RealDay();
		if (day != m_iMoraleDay)
		{
			m_iMoraleDay = day;
			m_mMoraleToday.Clear();
		}

		array<int> lit = {};
		foreach (RZ_Holding h : territory.GetHoldings())
		{
			if (!h || lit.Contains(h.m_eFaction))
				continue;

			if (h.m_eFaction != RZ_EFaction.SURVIVORS && h.m_eFaction != RZ_EFaction.MILITARY)
				continue;

			if (RZ_PowerGrid.IsGridPoweredAt(h.m_vPos))
				lit.Insert(h.m_eFaction);
		}

		foreach (int f : lit)
		{
			float given = 0;
			if (m_mMoraleToday.Contains(f))
				given = m_mMoraleToday.Get(f);

			float add = Math.Min(m_Settings.m_fMoralePerHour * dt / 3600, m_Settings.m_fMoraleDayCap - given);
			if (add <= 0)
				continue;

			life.AddMorale(f, add);
			m_mMoraleToday.Set(f, given + add);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void TickHolds(RZ_Substation s)
	{
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (!sites)
			return;

		array<string> prefixes = {FUSE, SWITCH, SPLICE, CUT};
		foreach (string prefix : prefixes)
		{
			string key = prefix + s.m_iId.ToString();
			if (!sites.IsDone(key))
				continue;

			int by = sites.DoneBy(key);
			sites.Close(key);
			if (by > 0)
				Done(prefix, s, by);
		}
	}

	// ================================================================ Bodies
	//------------------------------------------------------------------------------------------------
	protected void UpdateBodies(RZ_Substation s, array<IEntity> players)
	{
		float nearControl = RZ_Util.NearestDistance(s.m_vControl, players);
		if (nearControl > VISIBLE_RADIUS + DESPAWN_MARGIN)
		{
			if (s.m_Body)
				RZ_Util.DeleteUnit(s.m_Body);

			s.m_Body = null;
			s.m_Comp = null;
		}
		else if (!s.m_Body && nearControl <= VISIBLE_RADIUS)
		{
			ResourceName prefab = m_Grid.GetControlPrefab();
			if (s.m_bSpot && m_GM.GetInteractions())
				prefab = m_GM.GetInteractions().GetSpotPrefab();

			s.m_Body = RZ_Util.SpawnPrefab(prefab, s.m_vControl, s.m_fYaw);
			if (s.m_Body)
			{
				s.m_Comp = RZ_PuzzleObjectComponent.Tag(s.m_Body, TAG, s.m_iId, "", "");
				RZ_ClueComponent.Tag(s.m_Body, TAG, s.m_iId, "Look at " + s.m_sName);
				UpdateLabels(s);
			}
		}

		bool wantBreak = s.m_iBroken >= 0 && s.m_bPole;
		float nearBreak = RZ_Util.NearestDistance(s.m_vBreak, players);
		if (!wantBreak || nearBreak > VISIBLE_RADIUS + DESPAWN_MARGIN)
		{
			if (s.m_BreakBody)
				RZ_Util.DeleteUnit(s.m_BreakBody);

			if (s.m_Crew)
				RZ_Util.DeleteUnit(s.m_Crew);

			s.m_BreakBody = null;
			s.m_Crew = null;
		}

		// A break at the yard is read at the yard's own control; one at a pole gets a spot on the pole
		if (s.m_iBroken >= 0 && nearBreak <= VISIBLE_RADIUS)
		{
			if (wantBreak && !s.m_BreakBody && m_GM.GetInteractions())
			{
				s.m_BreakBody = RZ_Util.SpawnPrefab(m_GM.GetInteractions().GetSpotPrefab(), s.m_vBreak, 0);
				if (s.m_BreakBody)
				{
					RZ_PuzzleObjectComponent.Tag(s.m_BreakBody, TAG_BREAK, s.m_iId, "", "");
					RZ_ClueComponent.Tag(s.m_BreakBody, TAG_BREAK, s.m_iId, "Look at the line");
				}
			}

			if (s.m_iKind == BREAK_TOLL && !s.m_Crew && m_GM.GetTerritory())
			{
				ResourceName crew = m_GM.GetTerritory().SpeakerPrefab(RZ_EFaction.BANDITS);
				if (!crew.IsEmpty())
				{
					vector at = RZ_Mission.LandPointNear(s.m_vBreak, 2, 4);
					s.m_Crew = RZ_Util.SpawnPrefab(crew, at, RZ_Compass.Bearing(at, s.m_vBreak));
				}
			}
		}
	}

	// ================================================================ The test's hand
	//------------------------------------------------------------------------------------------------
	//! Test only: find the substations again as if the station's region were `region` m (a small map's
	//! station covers every town, so the test shrinks it to have some). Returns how many.
	int TestFind(vector station, float region)
	{
		foreach (RZ_Substation old : m_aSubs)
		{
			if (old.m_Body)
				RZ_Util.DeleteUnit(old.m_Body);

			if (old.m_BreakBody)
				RZ_Util.DeleteUnit(old.m_BreakBody);

			if (old.m_Crew)
				RZ_Util.DeleteUnit(old.m_Crew);
		}

		m_aSubs.Clear();
		Find(station, region);
		m_bOn = !m_aSubs.IsEmpty();
		ApplyTowns();
		return m_aSubs.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Test only: morale given to a faction today by the power.
	float TestMoraleToday(int f)
	{
		if (!m_mMoraleToday.Contains(f))
			return 0;

		return m_mMoraleToday.Get(f);
	}

	//------------------------------------------------------------------------------------------------
	//! Test only: run the morale for `seconds` of play at once.
	void TestMorale(float seconds, array<int> ids)
	{
		TickMorale(seconds, ids);
	}

	//------------------------------------------------------------------------------------------------
	//! Test only: the first town a substation would light, or -1.
	int TestFirstTown(int index)
	{
		RZ_Substation s = GetSub(index);
		if (!s || s.m_aTowns.IsEmpty())
			return -1;

		return s.m_aTowns[0];
	}
}

//------------------------------------------------------------------------------------------------
//! The nearest of the map's own power poles to a point (RZ_Substations.PoleBetween).
class RZ_SubstationPoleQuery
{
	vector m_vCenter;
	IEntity m_Best;
	float m_fBest;

	//------------------------------------------------------------------------------------------------
	bool OnEntity(IEntity e)
	{
		if (!e)
			return true;

		string name = RZ_Util.GetPrefabName(e);
		name.ToLower();
		if (!name.Contains("powerline") || !name.Contains("pole") || name.Contains("/dst/"))
			return true;

		float d = RZ_Util.DistXZ(e.GetOrigin(), m_vCenter);
		if (d < m_fBest)
		{
			m_fBest = d;
			m_Best = e;
		}

		return true;
	}
}
