//! The masts (docs/DESIGN.md, "The masts"): three to five of the map's own masts and aerials on high
//! ground, each over a region. Down, each is missing one part and has one trouble at its foot (a man
//! who lives there, a Red Hand crewman, a pack below, or nothing). Whoever settles the trouble and
//! climbs it with the part puts it up, and picks whose voice it is: a faction's, everyone's, or the
//! Red Hand's to sell. Up, a mast clears the map's fog round it for the one who put it up, sends its
//! region warnings, lets a radio overhear factions' working traffic, carries the drone operators'
//! chatter near the line, and with two up a trapped man's voice can be found by taking bearings at
//! two masts: a Distress Call starts where the bearings cross. Server side.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > World > Masts.
[BaseContainerProps()]
class RZ_MastSettings
{
	[Attribute("1", desc: "The masts: the map's own masts and aerials players put up, each over a region (docs/DESIGN.md, The masts)")]
	bool m_bEnabled;

	[Attribute("3", UIWidgets.Slider, "At least this many masts (the highest ground by a town makes up the number)", "1 5 1")]
	int m_iMin;

	[Attribute("5", UIWidgets.Slider, "At most this many masts", "1 8 1")]
	int m_iMax;

	[Attribute("2500", UIWidgets.Slider, "A mast's region: metres round it its warnings, listening and bearings reach", "500 5000 100")]
	float m_fRegion;

	[Attribute("25", UIWidgets.Slider, "Seconds of the hold that climbs a mast and fits its part", "5 120 1")]
	float m_fClimbSeconds;

	[Attribute("10", UIWidgets.Slider, "Seconds of the hold that takes a bearing on the voice at a mast", "3 60 1")]
	float m_fBearingSeconds;

	[Attribute("1000", UIWidgets.Slider, "The map's fog cleared round a mast for the one who puts it up (metres)", "0 3000 50")]
	float m_fFogRadius;

	[Attribute("6", UIWidgets.Slider, "Real minutes between an up mast's warnings to the people it serves", "1 60 1")]
	float m_fWarnMinutes;

	[Attribute("9", UIWidgets.Slider, "Real minutes between overheard lines in an up mast's region", "1 60 1")]
	float m_fListenMinutes;

	[Attribute("15", UIWidgets.Slider, "Real minutes between the drone operators' chatter at a mast near the line", "1 120 1")]
	float m_fChatterMinutes;

	[Attribute("90", UIWidgets.Slider, "Real minutes between trapped voices on the band (two masts up)", "5 600 5")]
	float m_fVoiceMinutes;

	[Attribute("30", UIWidgets.Slider, "Real minutes a trapped voice keeps calling before it stops", "5 120 5")]
	float m_fVoiceLastsMinutes;

	[Attribute("5", UIWidgets.Slider, "Fame with the faction a mast is handed to (once a mast a season per player)", "0 30 1")]
	int m_iHandFame;

	[Attribute("15", UIWidgets.Slider, "Red Hand Favor for a mast sold to them (once a mast a season per player)", "0 60 1")]
	int m_iSellFavor;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "The cabinet at a mast's foot when the map has none within 25 m: a prop with the generic puzzle object components (as the radio station's console). Empty = Puzzles > Radio mast, then the generic object", "et")]
	ResourceName m_sCabinetPrefab;
}

//------------------------------------------------------------------------------------------------
//! Whose voice a mast is.
enum RZ_EMastOwner
{
	NOBODY,		// down, or up and not yet given
	OPEN,		// everyone's
	HAVEN,
	BASTION,
	HALCYON,
	REDHAND		// sold to the Red Hand
}

//------------------------------------------------------------------------------------------------
//! What a mast is missing.
enum RZ_EMastPart
{
	BATTERY,
	WIRE,
	RADIO,
	CELLS,
	FUEL
}

//------------------------------------------------------------------------------------------------
//! What stands at a mast's foot before you can climb it.
enum RZ_EMastTrouble
{
	NONE,
	MAN,		// an old radio man lives there and wants one thing first
	CREW,		// a Red Hand crewman holds it for the book
	PACK		// a tribe's lair below: climbing makes noise, and at night the pack comes
}

//------------------------------------------------------------------------------------------------
class RZ_MastSave
{
	float x;
	float z;
	string name;
	string town;
	int part;
	int trouble;
	bool up;
	int owner;
	bool settled;			// the trouble is settled (the man given his thing, the crewman paid)
	bool promised;			// promised to the Red Hand at the foot: sold when it goes up
	string keeper;			// the man's or the crewman's name
	int want;				// what the man wants (RZ_EErrandItem)
	string putUpBy;			// the identity of who put it up
	ref array<string> paid = {};	// "<identity>" paid for this mast (Fame or Favor), once a season
}

//------------------------------------------------------------------------------------------------
class RZ_MastsSave
{
	ref array<ref RZ_MastSave> masts = {};
}

//------------------------------------------------------------------------------------------------
class RZ_Mast
{
	int m_iId;
	vector m_vPos;
	string m_sName;				// "the TV mast above Levie"
	string m_sShort;			// "the mast above Levie"
	string m_sTown;
	RZ_EMastPart m_ePart;
	RZ_EMastTrouble m_eTrouble;
	bool m_bUp;
	RZ_EMastOwner m_eOwner;
	bool m_bSettled;
	bool m_bPromised;
	string m_sKeeper;
	RZ_EErrandItem m_eWant;
	string m_sPutUpBy;
	int m_iPutUpPlayer = -1;	// this session's player who put it up (the first to choose)
	vector m_vLair;				// a tribe's lair below (PACK)
	bool m_bNearLine;
	ref array<string> m_aPaid = {};

	IEntity m_Cabinet;			// weak
	IEntity m_Keeper;			// weak
	RZ_PuzzleObjectComponent m_Comp;
	bool m_bOwnCabinet;			// the cabinet is ours (spawned), not the map's
	int m_iClimber;				// the player whose climb is open (0 none)
}

//------------------------------------------------------------------------------------------------
//! The mast's object menu (RZ_ObjectScreen.OpenRows): its rows as they stand for the one at it.
class RZ_MastRows : RZ_ObjectRows
{
	protected int m_iMast;
	protected ref array<string> m_aActs = {};
	protected ref array<string> m_aLabels = {};
	protected ref array<string> m_aLocks = {};

	//------------------------------------------------------------------------------------------------
	void RZ_MastRows(int mast)
	{
		m_iMast = mast;
	}

	//------------------------------------------------------------------------------------------------
	protected void Build(IEntity user)
	{
		m_aActs.Clear();
		m_aLabels.Clear();
		m_aLocks.Clear();
		RZ_Masts masts = RZ_Masts.Get();
		if (masts)
			masts.Rows(m_iMast, user, m_aActs, m_aLabels, m_aLocks);
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

		RZ_Masts masts = RZ_Masts.Get();
		if (masts)
			masts.Act(m_iMast, m_aActs[index], user);
	}

	//------------------------------------------------------------------------------------------------
	override string Text()
	{
		RZ_Masts masts = RZ_Masts.Get();
		if (!masts)
			return "";

		return masts.TextOf(m_iMast);
	}
}

//------------------------------------------------------------------------------------------------
class RZ_Masts
{
	static const int TAG = -18;				// RZ_PuzzleObjectComponent / RZ_ClueComponent mission id: a mast's cabinet
	static const int TAG_PERSON = -19;		// a mast's keeper (the man, the crewman)

	protected static const float VISIBLE_RADIUS = 300;
	protected static const float DESPAWN_MARGIN = 50;
	protected static const float SPACING = 1500;
	protected static const float STATION_GAP = 800;
	protected static const float LAIR_RADIUS = 800;
	protected static const float LINE_CHATTER = 3000;
	protected static const float CLIMB_HEAT = 6;
	protected static const float PACK_NEAR = 1000;
	protected static const string CLIMB = "mast.climb.";
	protected static const string BEARING = "mast.bearing.";

	protected static RZ_Masts s_Instance;

	protected RZ_GameModeComponent m_GM;
	protected RZ_MastSettings m_Settings;
	protected ref RZ_MastsSave m_Restored;
	protected ref array<ref RZ_Mast> m_aMasts = {};
	protected float m_fWarnTimer;
	protected float m_fListenTimer;
	protected float m_fChatterTimer;
	protected float m_fVoiceTimer;
	protected int m_iStep;

	// The trapped voice: where it is, until when, and which masts each player has a bearing from
	protected bool m_bVoice;
	protected vector m_vVoice;
	protected string m_sVoiceName;
	protected float m_fVoiceLeft;
	protected ref map<int, ref array<int>> m_mBearings = new map<int, ref array<int>>();
	// The last call the bearings started (the runtime test reads them)
	vector m_vLastCall;
	vector m_vLastCallAt;
	bool m_bLastCallStarted;

	//------------------------------------------------------------------------------------------------
	void RZ_Masts(RZ_GameModeComponent gm, RZ_MastSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_Masts Get()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	bool IsOn()
	{
		return m_Settings && m_Settings.m_bEnabled && !m_aMasts.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	int Count() { return m_aMasts.Count(); }

	//------------------------------------------------------------------------------------------------
	RZ_Mast GetMast(int index)
	{
		if (index < 0 || index >= m_aMasts.Count())
			return null;

		return m_aMasts[index];
	}

	//------------------------------------------------------------------------------------------------
	int UpCount()
	{
		int n = 0;
		foreach (RZ_Mast m : m_aMasts)
		{
			if (m.m_bUp)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	bool HasVoice() { return m_bVoice; }
	vector VoiceAt() { return m_vVoice; }

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	void Restore(RZ_MastsSave saved)
	{
		m_Restored = saved;
	}

	//------------------------------------------------------------------------------------------------
	RZ_MastsSave Save()
	{
		RZ_MastsSave data = new RZ_MastsSave();
		foreach (RZ_Mast m : m_aMasts)
		{
			RZ_MastSave s = new RZ_MastSave();
			s.x = m.m_vPos[0];
			s.z = m.m_vPos[2];
			s.name = m.m_sName;
			s.town = m.m_sTown;
			s.part = m.m_ePart;
			s.trouble = m.m_eTrouble;
			s.up = m.m_bUp;
			s.owner = m.m_eOwner;
			s.settled = m.m_bSettled;
			s.promised = m.m_bPromised;
			s.keeper = m.m_sKeeper;
			s.want = m.m_eWant;
			s.putUpBy = m.m_sPutUpBy;
			s.paid.Copy(m.m_aPaid);
			data.masts.Insert(s);
		}

		return data;
	}

	// ================================================================ Setup
	//------------------------------------------------------------------------------------------------
	void Setup()
	{
		if (!m_Settings || !m_Settings.m_bEnabled || m_GM.GetSettlements().IsEmpty())
		{
			RZ_Log.Info("Masts: off.");
			return;
		}

		string from = "found";
		if (m_Restored && m_Restored.masts && !m_Restored.masts.IsEmpty())
		{
			foreach (RZ_MastSave s : m_Restored.masts)
			{
				RZ_Mast m = new RZ_Mast();
				m.m_iId = m_aMasts.Count();
				m.m_vPos = RZ_Util.GroundPos(Vector(s.x, 0, s.z));
				m.m_sName = s.name;
				m.m_sTown = s.town;
				m.m_ePart = s.part;
				m.m_eTrouble = s.trouble;
				m.m_bUp = s.up;
				m.m_eOwner = s.owner;
				m.m_bSettled = s.settled;
				m.m_bPromised = s.promised;
				m.m_sKeeper = s.keeper;
				m.m_eWant = s.want;
				m.m_sPutUpBy = s.putUpBy;
				if (s.paid)
					m.m_aPaid.Copy(s.paid);

				Finish(m);
				m_aMasts.Insert(m);
			}

			from = "save";
		}
		else
		{
			Find();
		}

		m_Restored = null;
		foreach (RZ_Mast mast : m_aMasts)
		{
			RZ_Log.Info(string.Format("Masts: %1 (%2) at %3, missing %4, trouble %5, %6 (%7).", mast.m_sName, mast.m_iId, RZ_SettlementScanner.GridReference(mast.m_vPos), PartWord(mast.m_ePart), typename.EnumToString(RZ_EMastTrouble, mast.m_eTrouble), StateWord(mast), from));
		}

		RZ_Log.Info(string.Format("Masts: %1 on this map, %2 up.", m_aMasts.Count(), UpCount()));
	}

	//------------------------------------------------------------------------------------------------
	//! The map's own masts and aerials near towns (the radio station's finder), spread out, then the
	//! highest ground by the bigger towns to make up the number.
	protected void Find()
	{
		array<ref RZ_Settlement> towns = m_GM.GetSettlements();
		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		vector station = vector.Zero;
		if (m_GM.GetRadioStation() && m_GM.GetRadioStation().IsActive())
			station = m_GM.GetRadioStation().GetPosition();

		array<string> kinds = {"radio"};
		int max = Math.Max(1, m_Settings.m_iMax);
		foreach (RZ_Settlement s : towns)
		{
			if (m_aMasts.Count() >= max)
				break;

			RZ_Landmark l = RZ_LandmarkFinder.FindNearest(s.m_vCenter, s.m_fRadius + 1500, kinds);
			if (!l)
				continue;

			vector at = RZ_Util.GroundPos(l.m_vPos);
			if (!Fits(at, perimeter, station))
				continue;

			string what = l.m_sName;
			if (what.IndexOf("the ") != 0)
				what = "the " + what;

			Add(at, string.Format("%1 above %2", what, s.m_sName), s.m_sName);
		}

		// Too few on the map: the highest ground by the bigger towns (size 2 first, then 1)
		for (int size = 2; size >= 0 && m_aMasts.Count() < m_Settings.m_iMin; size--)
		{
			foreach (RZ_Settlement t : towns)
			{
				if (m_aMasts.Count() >= m_Settings.m_iMin)
					break;

				if (t.m_iSize != size)
					continue;

				vector high = RZ_Mission.HighPointNear(t.m_vCenter, t.m_fRadius + 100, t.m_fRadius + 700);
				if (!Fits(high, perimeter, station))
					continue;

				Add(high, "the relay mast above " + t.m_sName, t.m_sName);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool Fits(vector at, RZ_Perimeter perimeter, vector station)
	{
		if (at == vector.Zero || RZ_Util.IsWater(at) || !RZ_Mainland.OnMainland(at))
			return false;

		if (perimeter && perimeter.IsNearLine(at))
			return false;

		if (station != vector.Zero && RZ_Util.DistXZ(at, station) < STATION_GAP)
			return false;

		foreach (RZ_Mast m : m_aMasts)
		{
			if (RZ_Util.DistXZ(m.m_vPos, at) < SPACING)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A new mast, its part, trouble and keeper dealt from where it stands (the same all season).
	protected void Add(vector at, string name, string town)
	{
		RZ_Mast m = new RZ_Mast();
		m.m_iId = m_aMasts.Count();
		m.m_vPos = at;
		m.m_sName = name;
		m.m_sTown = town;
		RZ_Rng rng = new RZ_Rng(Math.AbsInt(RZ_Util.RoundToInt(at[0] * 7 + at[2] * 13)) + 1 + m.m_iId * 977);
		m.m_ePart = (m.m_iId + rng.Int(0, 5)) % 5;

		// The trouble: a pack below if a lair is near, else a man, the crew or nothing by turns
		m.m_eTrouble = RZ_EMastTrouble.NONE;
		if (LairNear(at) != vector.Zero)
			m.m_eTrouble = RZ_EMastTrouble.PACK;
		else
		{
			int roll = (m.m_iId + rng.Int(0, 3)) % 3;
			if (roll == 0)
				m.m_eTrouble = RZ_EMastTrouble.MAN;
			else if (roll == 1)
				m.m_eTrouble = RZ_EMastTrouble.CREW;
		}

		if (m.m_eTrouble == RZ_EMastTrouble.MAN || m.m_eTrouble == RZ_EMastTrouble.CREW)
			m.m_sKeeper = RZ_NoteText.ManName(rng) + " " + RZ_NoteText.Surname(rng);

		array<RZ_EErrandItem> wants = {RZ_EErrandItem.FOOD, RZ_EErrandItem.MEDICAL, RZ_EErrandItem.FUEL};
		m.m_eWant = wants[rng.Int(0, 3)];
		m.m_bSettled = m.m_eTrouble == RZ_EMastTrouble.NONE || m.m_eTrouble == RZ_EMastTrouble.PACK;
		Finish(m);
		m_aMasts.Insert(m);
	}

	//------------------------------------------------------------------------------------------------
	protected void Finish(RZ_Mast m)
	{
		m.m_sShort = m.m_sName;
		int above = m.m_sName.IndexOf(" above ");
		if (above > 0)
			m.m_sShort = "the mast" + m.m_sName.Substring(above, m.m_sName.Length() - above);

		m.m_vLair = LairNear(m.m_vPos);
		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		m.m_bNearLine = perimeter && perimeter.IsNearLineWithin(m.m_vPos, LINE_CHATTER);
	}

	//------------------------------------------------------------------------------------------------
	protected vector LairNear(vector at)
	{
		RZ_Tribes tribes = m_GM.GetTribes();
		if (!tribes || !tribes.IsEnabled())
			return vector.Zero;

		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			if (t && RZ_Util.DistXZ(t.m_vLair, at) <= LAIR_RADIUS)
				return t.m_vLair;
		}

		return vector.Zero;
	}

	// ================================================================ Words
	//------------------------------------------------------------------------------------------------
	static string PartWord(RZ_EMastPart part)
	{
		switch (part)
		{
			case RZ_EMastPart.BATTERY: return "a battery";
			case RZ_EMastPart.WIRE: return "a coil of wire";
			case RZ_EMastPart.RADIO: return "a radio set";
			case RZ_EMastPart.CELLS: return "two torch cells";
			case RZ_EMastPart.FUEL: return "a can of fuel";
		}

		return "a part";
	}

	//------------------------------------------------------------------------------------------------
	//! What the tag on the cabinet says is wrong.
	static string FaultWord(RZ_EMastPart part)
	{
		switch (part)
		{
			case RZ_EMastPart.BATTERY: return "The battery box is empty. Somebody took the battery.";
			case RZ_EMastPart.WIRE: return "The feed cable's been cut and the coil taken.";
			case RZ_EMastPart.RADIO: return "The set in the cabinet has been pulled out. The shelf's empty.";
			case RZ_EMastPart.CELLS: return "The beacon's dead: no cells in it.";
			case RZ_EMastPart.FUEL: return "The generator under the cabinet is dry.";
		}

		return "Something's missing.";
	}

	//------------------------------------------------------------------------------------------------
	//! The words a part's items carry in their prefab names.
	protected static void PartWords(RZ_EMastPart part, notnull array<string> words)
	{
		switch (part)
		{
			case RZ_EMastPart.BATTERY: words.Insert("battery"); break;
			case RZ_EMastPart.WIRE: words.Insert("rz_wire"); words.Insert("wire_coil"); words.Insert("cablecoil"); break;
			case RZ_EMastPart.RADIO: words.Insert("radio_"); words.Insert("anprc"); words.Insert("r148"); words.Insert("radiosmall"); break;
			case RZ_EMastPart.CELLS: words.Insert("rz_cells"); words.Insert("cells"); break;
			case RZ_EMastPart.FUEL: words.Insert("jerrycan"); words.Insert("canister"); words.Insert("fuelcan"); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static int PartCount(RZ_EMastPart part)
	{
		if (part == RZ_EMastPart.CELLS)
			return 2;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Counts the part on a character (a radio lure is never a radio set).
	static int CountPart(IEntity character, RZ_EMastPart part)
	{
		if (!character)
			return 0;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return 0;

		array<string> words = {};
		PartWords(part, words);
		array<IEntity> items = {};
		inv.GetItems(items);
		int n = 0;
		foreach (IEntity it : items)
		{
			if (it && IsPart(it, words))
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IsPart(IEntity item, array<string> words)
	{
		string name = RZ_Util.GetPrefabName(item);
		name.ToLower();
		if (name.IsEmpty() || name.Contains("lure"))
			return false;

		foreach (string w : words)
		{
			if (name.Contains(w))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static int TakePart(IEntity character, RZ_EMastPart part, int count)
	{
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return 0;

		array<string> words = {};
		PartWords(part, words);
		array<IEntity> items = {};
		inv.GetItems(items);
		int taken = 0;
		foreach (IEntity it : items)
		{
			if (taken >= count)
				break;

			if (it && IsPart(it, words) && inv.TryDeleteItem(it))
				taken++;
		}

		return taken;
	}

	//------------------------------------------------------------------------------------------------
	static string OwnerWord(RZ_EMastOwner owner)
	{
		switch (owner)
		{
			case RZ_EMastOwner.OPEN: return "everyone's";
			case RZ_EMastOwner.HAVEN: return "the Haven's";
			case RZ_EMastOwner.BASTION: return "Bastion's";
			case RZ_EMastOwner.HALCYON: return "Halcyon's";
			case RZ_EMastOwner.REDHAND: return "the Red Hand's";
		}

		return "nobody's yet";
	}

	//------------------------------------------------------------------------------------------------
	//! The faction a mast serves (-1: everyone's or nobody's).
	static int FactionOf(RZ_EMastOwner owner)
	{
		switch (owner)
		{
			case RZ_EMastOwner.HAVEN: return RZ_EFaction.SURVIVORS;
			case RZ_EMastOwner.BASTION: return RZ_EFaction.MILITARY;
			case RZ_EMastOwner.HALCYON: return RZ_EFaction.INSTITUTE;
			case RZ_EMastOwner.REDHAND: return RZ_EFaction.BANDITS;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	protected static string StateWord(RZ_Mast m)
	{
		if (!m.m_bUp)
			return "down";

		return "up, " + OwnerWord(m.m_eOwner);
	}

	//------------------------------------------------------------------------------------------------
	protected string Speaker(RZ_Mast m)
	{
		return "Radio: " + m.m_sShort;
	}

	//------------------------------------------------------------------------------------------------
	protected string KeeperTitle(RZ_Mast m)
	{
		if (m.m_eTrouble == RZ_EMastTrouble.CREW)
			return m.m_sKeeper + ", Red Hand crewman at " + m.m_sShort;

		return m.m_sKeeper + ", the radio man at " + m.m_sShort;
	}

	//------------------------------------------------------------------------------------------------
	//! The menu's description: what the tag says, and who it serves.
	string TextOf(int index)
	{
		RZ_Mast m = GetMast(index);
		if (!m)
			return "";

		string text;
		if (!m.m_bUp)
		{
			text = "A tag wired to the cabinet: " + FaultWord(m.m_ePart) + string.Format(" It needs %1.", PartWord(m.m_ePart));
			if (m.m_eTrouble == RZ_EMastTrouble.MAN && !m.m_bSettled)
				text = text + string.Format(" A hut at the foot, smoke from its pipe: %1 lives here.", m.m_sKeeper);
			else if (m.m_eTrouble == RZ_EMastTrouble.CREW && !m.m_bSettled)
				text = text + string.Format(" A Red Hand crewman, %1, sits on the cabinet's step.", m.m_sKeeper);
			else if (m.m_eTrouble == RZ_EMastTrouble.PACK)
				text = text + " Bones and torn cloth in the grass below. Something lairs down there: the climb will be heard.";

			if (m.m_bPromised)
				text = text + " You promised it to the Red Hand.";

			return text;
		}

		text = string.Format("Up, and humming. It's %1.", OwnerWord(m.m_eOwner));
		if (m.m_eOwner == RZ_EMastOwner.NOBODY)
			text = text + " Whoever put it up hasn't said whose it is.";

		if (m_bVoice)
			text = text + " Under the hiss, faint: someone's calling.";

		return text;
	}

	// ================================================================ The menu
	//------------------------------------------------------------------------------------------------
	//! The rows for `user` at mast `index`: an act key, its label, and why it's locked ("" if not).
	void Rows(int index, IEntity user, notnull array<string> acts, notnull array<string> labels, notnull array<string> locks)
	{
		RZ_Mast m = GetMast(index);
		if (!m)
			return;

		int playerId = 0;
		if (user)
			playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);

		if (!m.m_bUp)
		{
			string why = "";
			if (!m.m_bSettled && m.m_eTrouble == RZ_EMastTrouble.MAN)
				why = string.Format("%1 won't have anyone up his mast until you've talked to him.", m.m_sKeeper);
			else if (!m.m_bSettled && m.m_eTrouble == RZ_EMastTrouble.CREW)
				why = string.Format("%1 is sitting on the step. Talk to him first.", m.m_sKeeper);
			else if (user && CountPart(user, m.m_ePart) < PartCount(m.m_ePart))
				why = string.Format("You haven't got %1 on you.", PartWord(m.m_ePart));
			else if (m.m_iClimber > 0 && m.m_iClimber != playerId)
				why = "Someone's already climbing it.";

			acts.Insert("climb");
			labels.Insert(string.Format("Climb and fit %1", PartWord(m.m_ePart)));
			locks.Insert(why);
			return;
		}

		if (m.m_eOwner == RZ_EMastOwner.NOBODY && !m.m_bPromised)
		{
			string wait = "";
			if (m.m_iPutUpPlayer > 0 && playerId != m.m_iPutUpPlayer && GetGame().GetPlayerManager().GetPlayerControlledEntity(m.m_iPutUpPlayer))
				wait = "Whoever put it up says whose it is.";

			acts.Insert("give:haven");
			labels.Insert("Hand it to the Haven");
			locks.Insert(wait);
			acts.Insert("give:bastion");
			labels.Insert("Hand it to Bastion");
			locks.Insert(wait);
			acts.Insert("give:halcyon");
			labels.Insert("Hand it to Halcyon");
			locks.Insert(wait);
			acts.Insert("give:open");
			labels.Insert("Keep it open, for everyone");
			locks.Insert(wait);
			acts.Insert("give:redhand");
			labels.Insert("Sell it to the Red Hand");
			locks.Insert(wait);
		}

		if (m_bVoice)
		{
			string bearingWhy = "";
			if (HasBearing(playerId, m.m_iId))
				bearingWhy = "You've a bearing from this mast already. Take one from another.";

			acts.Insert("bearing");
			labels.Insert("Take a bearing on the voice");
			locks.Insert(bearingWhy);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A row picked (checked again here: the menu's view may be old).
	void Act(int index, string act, IEntity user)
	{
		RZ_Mast m = GetMast(index);
		if (!m || !user)
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		if (playerId <= 0)
			return;

		if (act == "climb")
		{
			OpenClimb(m, playerId, user);
			return;
		}

		if (act == "bearing")
		{
			OpenBearing(m, playerId);
			return;
		}

		if (act.IndexOf("give:") == 0)
			Give(m, playerId, act.Substring(5, act.Length() - 5));
	}

	//------------------------------------------------------------------------------------------------
	//! "Look at the mast" (RZ_ReadClueAction): its menu, else its tag as a line and its rows as prompts.
	void HandleRead(int playerId, IEntity owner)
	{
		RZ_Mast m = MastOfCabinet(owner);
		if (!m)
			return;

		if (RZ_ObjectScreen.Available() && RZ_ObjectScreen.OpenRows(playerId, owner, Capitalize(m.m_sName), new RZ_MastRows(m.m_iId)))
			return;

		m_GM.NotifyPlayer(playerId, Capitalize(m.m_sName), TextOf(m.m_iId));
		UpdateLabels(m);
	}

	//------------------------------------------------------------------------------------------------
	//! Without the object menu: the first four rows as the cabinet's prompts.
	void HandleChoice(int playerId, int option, IEntity owner, IEntity user)
	{
		RZ_Mast m = MastOfCabinet(owner);
		if (!m)
			return;

		array<string> acts = {};
		array<string> labels = {};
		array<string> locks = {};
		Rows(m.m_iId, user, acts, labels, locks);
		if (option < 0 || option >= acts.Count())
			return;

		if (!locks[option].IsEmpty())
		{
			m_GM.NotifyPlayer(playerId, Capitalize(m.m_sName), locks[option]);
			return;
		}

		Act(m.m_iId, acts[option], user);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateLabels(RZ_Mast m)
	{
		if (!m.m_Comp)
			return;

		array<string> acts = {};
		array<string> labels = {};
		array<string> locks = {};
		Rows(m.m_iId, null, acts, labels, locks);
		string joined = "";
		for (int i = 0; i < labels.Count() && i < 4; i++)
		{
			if (i > 0)
				joined = joined + "|";

			joined = joined + labels[i];
		}

		m.m_Comp.SetLabels("", joined);
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Mast MastOfCabinet(IEntity owner)
	{
		foreach (RZ_Mast m : m_aMasts)
		{
			if (m.m_Cabinet && m.m_Cabinet == owner)
				return m;
		}

		RZ_ClueComponent clue = RZ_ClueComponent.Cast(owner.FindComponent(RZ_ClueComponent));
		if (clue && clue.GetMissionId() == TAG)
			return GetMast(clue.GetClueIndex());

		return null;
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

	// ================================================================ The climb
	//------------------------------------------------------------------------------------------------
	protected void OpenClimb(RZ_Mast m, int playerId, IEntity user)
	{
		if (m.m_bUp || !m.m_bSettled || CountPart(user, m.m_ePart) < PartCount(m.m_ePart))
			return;

		m.m_iClimber = playerId;
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (!sites || !RZ_WorkSites.Available())
		{
			// No work sites on this server: the climb is done at once
			PutUp(m, playerId);
			return;
		}

		string key = CLIMB + m.m_iId.ToString();
		sites.Close(key);
		sites.Open(key, RZ_EWork.HANDS, string.Format("Climb and fit %1", PartWord(m.m_ePart)), m_Settings.m_fClimbSeconds, CabinetAt(m), playerId);
		// At the cabinet itself: nothing more is put out
		sites.SetAtPiece(key, CabinetAt(m), 0);
		RZ_Log.Info(string.Format("Masts: %1 climbs %2.", RZ_Names.Public(playerId), m.m_sName));
	}

	//------------------------------------------------------------------------------------------------
	protected vector CabinetAt(RZ_Mast m)
	{
		if (m.m_Cabinet)
			return m.m_Cabinet.GetOrigin();

		return m.m_vPos;
	}

	//------------------------------------------------------------------------------------------------
	//! The climb's hold is done (or there are no holds): the part goes in and the mast is up.
	protected void PutUp(RZ_Mast m, int playerId, IEntity who = null)
	{
		IEntity ent = who;
		if (!ent)
			ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);

		m.m_iClimber = 0;
		if (!ent || TakePart(ent, m.m_ePart, PartCount(m.m_ePart)) < PartCount(m.m_ePart))
		{
			if (ent)
				m_GM.NotifyPlayer(playerId, Capitalize(m.m_sName), string.Format("At the top you reach for %1, and it isn't on you any more.", PartWord(m.m_ePart)));

			return;
		}

		m.m_bUp = true;
		m.m_iPutUpPlayer = playerId;
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		if (p)
			m.m_sPutUpBy = p.m_sIdentity;

		// The climb is heard; a pack below comes at night
		if (m_GM.GetHeat())
			m_GM.GetHeat().AddHeat(m.m_vPos, CLIMB_HEAT);

		if (m.m_eTrouble == RZ_EMastTrouble.PACK && RZ_GameTime.IsNight())
			DrawPack(m);

		// The land round it, from the top
		RZ_MapFog fog = m_GM.GetMapFog();
		int cleared = 0;
		if (fog && fog.IsOn() && m_Settings.m_fFogRadius > 0)
			cleared = fog.RevealDisc(playerId, m.m_vPos, m_Settings.m_fFogRadius);

		RZ_Log.Info(string.Format("Masts: %1 put %2 up (%3 squares of fog cleared).", RZ_Names.Public(playerId), m.m_sName, cleared));
		m_GM.Say(playerId, Speaker(m), -1, "The needle swings up off the stop and the hiss comes in clean. It's up. Whose voice is it going to be?");

		if (m.m_bPromised)
		{
			Give(m, playerId, "redhand");
			return;
		}

		UpdateLabels(m);
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawPack(RZ_Mast m)
	{
		RZ_Tribes tribes = m_GM.GetTribes();
		if (!tribes)
			return;

		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			if (t && RZ_Util.DistXZ(t.m_vLair, m.m_vPos) <= LAIR_RADIUS)
			{
				int sent = tribes.DrawTo(t, m.m_vPos, true);
				RZ_Log.Info(string.Format("Masts: the climb at %1 woke %2 below it (%3 sent).", m.m_sName, t.m_sPlace, sent));
				return;
			}
		}
	}

	// ================================================================ Whose it is
	//------------------------------------------------------------------------------------------------
	protected void Give(RZ_Mast m, int playerId, string who)
	{
		if (!m.m_bUp || m.m_eOwner != RZ_EMastOwner.NOBODY)
			return;

		RZ_EMastOwner owner = RZ_EMastOwner.OPEN;
		if (who == "haven")
			owner = RZ_EMastOwner.HAVEN;
		else if (who == "bastion")
			owner = RZ_EMastOwner.BASTION;
		else if (who == "halcyon")
			owner = RZ_EMastOwner.HALCYON;
		else if (who == "redhand")
			owner = RZ_EMastOwner.REDHAND;

		m.m_eOwner = owner;
		string ident = "";
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		if (p)
			ident = p.m_sIdentity;

		bool firstPay = !ident.IsEmpty() && !m.m_aPaid.Contains(ident);
		int f = FactionOf(owner);
		string said;
		if (owner == RZ_EMastOwner.REDHAND)
		{
			if (firstPay)
				m_GM.AddFavor(playerId, RZ_EFaction.BANDITS, m_Settings.m_iSellFavor);

			said = "A crew voice comes on before you've let go of the set: \"That's ours now. Anyone wants to listen round here, they pay the book.\"";
		}
		else if (f >= 0)
		{
			if (firstPay)
				m_GM.AddFame(playerId, f, m_Settings.m_iHandFame, false, 0, string.Format("put %1 up for us", m.m_sShort));

			said = string.Format("%1 comes on, crackling: \"We hear you. That's our voice over %2 now. We won't forget who climbed it.\"", RZ_FactionVoice.ChannelTitle(f), m.m_sTown);
		}
		else
		{
			said = string.Format("You leave the set on the open band. Anyone with a radio round %1 will hear what it hears.", m.m_sTown);
			if (m.m_eTrouble == RZ_EMastTrouble.MAN && m.m_Keeper)
				m_GM.Say(playerId, KeeperTitle(m), -1, "Open. Good. Then I'll stay and keep it, and you'll hear from me.", m.m_Keeper);
		}

		if (firstPay)
			m.m_aPaid.Insert(ident);

		m_GM.Say(playerId, Speaker(m), -1, said);
		RZ_Log.Info(string.Format("Masts: %1 made %2 %3.", RZ_Names.Public(playerId), m.m_sName, OwnerWord(owner)));
		UpdateLabels(m);
	}

	// ================================================================ The keeper (talk)
	//------------------------------------------------------------------------------------------------
	//! Who a keeper's body is: its mast, else null.
	RZ_Mast MastOfKeeper(int index)
	{
		return GetMast(index);
	}

	//------------------------------------------------------------------------------------------------
	string KeeperTitleOf(int index)
	{
		RZ_Mast m = GetMast(index);
		if (!m)
			return "";

		return KeeperTitle(m);
	}

	//------------------------------------------------------------------------------------------------
	//! What the keeper says when you come to him.
	string KeeperGreeting(int index, int playerId)
	{
		RZ_Mast m = GetMast(index);
		if (!m)
			return "";

		if (m.m_eTrouble == RZ_EMastTrouble.CREW)
		{
			if (m.m_bSettled)
				return "Go on, then. The book has you down.";

			return "This one's the book's. Nobody climbs it without the book saying so. Two tins, or a word that you're one of ours. Or promise it's ours when it's up.";
		}

		if (m.m_bUp)
		{
			if (m.m_eOwner == RZ_EMastOwner.OPEN)
				return "Hear it? Everyone can, now. I listen all night. Ask and I'll tell you what came through.";

			return string.Format("It's %1 now. I'll be off when the weather turns.", OwnerWord(m.m_eOwner));
		}

		if (m.m_bSettled)
			return "Go on up. Mind the third rung.";

		return string.Format("I've kept that mast in my head since the roads shut. You want to climb it, you can do something for me first: %1. And when it's up, keep it open. Everyone's.", RZ_Errands.ItemNeed(m.m_eWant, WantCount(m)));
	}

	//------------------------------------------------------------------------------------------------
	protected int WantCount(RZ_Mast m)
	{
		if (m.m_eWant == RZ_EErrandItem.FOOD)
			return 2;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! The keeper's replies: id, label, tag, need ("" when it can be picked).
	void KeeperReplies(int index, int playerId, IEntity user, notnull array<int> ids, notnull array<string> labels, notnull array<string> tags, notnull array<string> needs)
	{
		RZ_Mast m = GetMast(index);
		if (!m)
			return;

		if (m.m_eTrouble == RZ_EMastTrouble.MAN)
		{
			if (!m.m_bSettled)
			{
				string need = "";
				if (RZ_Errands.CountItems(user, m.m_eWant) < WantCount(m))
					need = RZ_Errands.ItemNeed(m.m_eWant, WantCount(m));

				ids.Insert(1);
				labels.Insert("\"Here. Now let me up.\"");
				tags.Insert("[Errand]");
				needs.Insert(need);
			}
			else if (m.m_bUp && m.m_eOwner == RZ_EMastOwner.OPEN)
			{
				ids.Insert(4);
				labels.Insert("\"What's come through?\"");
				tags.Insert("");
				needs.Insert("");
			}

			return;
		}

		if (m.m_eTrouble != RZ_EMastTrouble.CREW || m.m_bSettled)
			return;

		string tins = "";
		if (RZ_Errands.CountItems(user, RZ_EErrandItem.FOOD) < 2)
			tins = RZ_Errands.ItemNeed(RZ_EErrandItem.FOOD, 2);

		ids.Insert(1);
		labels.Insert("\"Two tins for the book.\"");
		tags.Insert("[Trade]");
		needs.Insert(tins);

		string known = "";
		if (!RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.KNOWN))
			known = "Known with the Red Hand";

		ids.Insert(2);
		labels.Insert("\"I'm one of yours. Let me up.\"");
		tags.Insert("[Known · Red Hand]");
		needs.Insert(known);

		ids.Insert(3);
		labels.Insert("\"Let me up and it's the Red Hand's when it's done.\"");
		tags.Insert("[Promise]");
		needs.Insert("");
	}

	//------------------------------------------------------------------------------------------------
	//! A keeper's reply picked. Returns what he says.
	string KeeperPick(int index, int playerId, int replyId, IEntity user)
	{
		RZ_Mast m = GetMast(index);
		if (!m || !user)
			return "";

		if (replyId == 4)
			return HeardLine(m);

		if (m.m_bSettled)
			return "It's settled. Go on.";

		if (m.m_eTrouble == RZ_EMastTrouble.MAN && replyId == 1)
		{
			if (RZ_Errands.TakeItems(user, m.m_eWant, WantCount(m)) < WantCount(m))
				return "That's not it. Come back when you have it.";

			m.m_bSettled = true;
			RZ_Log.Info(string.Format("Masts: %1 settled with %2 at %3.", RZ_Names.Public(playerId), m.m_sKeeper, m.m_sName));
			UpdateLabels(m);
			return "Thank you. Go on up. Mind the third rung. And keep it open.";
		}

		if (m.m_eTrouble != RZ_EMastTrouble.CREW)
			return "";

		if (replyId == 1)
		{
			if (RZ_Errands.TakeItems(user, RZ_EErrandItem.FOOD, 2) < 2)
				return "Two tins. I can count.";

			m.m_bSettled = true;
		}
		else if (replyId == 2)
		{
			if (!RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.KNOWN))
				return "You? I've never seen you at the crossroads.";

			m.m_bSettled = true;
		}
		else if (replyId == 3)
		{
			m.m_bSettled = true;
			m.m_bPromised = true;
		}

		if (!m.m_bSettled)
			return "";

		RZ_Log.Info(string.Format("Masts: %1 settled with the crew at %2 (reply %3).", RZ_Names.Public(playerId), m.m_sName, replyId));
		UpdateLabels(m);
		if (m.m_bPromised)
			return "Then it's ours. I'll write it down. Go on up.";

		return "Go on. I'll put it in the book.";
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsKeeperHere(RZ_Mast m)
	{
		if (m.m_eTrouble == RZ_EMastTrouble.CREW)
			return !m.m_bSettled || !m.m_bUp;

		if (m.m_eTrouble == RZ_EMastTrouble.MAN)
			return !m.m_bUp || m.m_eOwner == RZ_EMastOwner.OPEN || m.m_eOwner == RZ_EMastOwner.NOBODY;

		return false;
	}

	// ================================================================ Bodies
	//------------------------------------------------------------------------------------------------
	protected ResourceName CabinetPrefab()
	{
		if (!m_Settings.m_sCabinetPrefab.IsEmpty())
			return m_Settings.m_sCabinetPrefab;

		return m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.RADIO_MAST);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateBodies(RZ_Mast m, array<IEntity> players)
	{
		float nearest = RZ_Util.NearestDistance(m.m_vPos, players);
		if (nearest > VISIBLE_RADIUS + DESPAWN_MARGIN)
		{
			if (m.m_Cabinet)
				RZ_Util.DeleteUnit(m.m_Cabinet);

			if (m.m_Keeper)
				RZ_Util.DeleteUnit(m.m_Keeper);

			m.m_Cabinet = null;
			m.m_Keeper = null;
			m.m_Comp = null;
			return;
		}

		if (nearest > VISIBLE_RADIUS)
			return;

		if (!m.m_Cabinet)
		{
			m.m_Cabinet = RZ_Util.SpawnPrefab(CabinetPrefab(), m.m_vPos, Math.RandomFloat(0, 360));
			if (m.m_Cabinet)
			{
				m.m_Comp = RZ_PuzzleObjectComponent.Tag(m.m_Cabinet, TAG, m.m_iId, "", "");
				RZ_ClueComponent.Tag(m.m_Cabinet, TAG, m.m_iId, "Look at the mast");
				UpdateLabels(m);
			}
		}

		bool wantKeeper = IsKeeperHere(m);
		if (!wantKeeper && m.m_Keeper)
		{
			RZ_Util.DeleteUnit(m.m_Keeper);
			m.m_Keeper = null;
		}

		if (!wantKeeper || m.m_Keeper)
			return;

		ResourceName prefab = m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);
		if (m.m_eTrouble == RZ_EMastTrouble.CREW && m_GM.GetTerritory())
		{
			ResourceName crew = m_GM.GetTerritory().SpeakerPrefab(RZ_EFaction.BANDITS);
			if (!crew.IsEmpty())
				prefab = crew;
		}

		if (prefab.IsEmpty())
			return;

		vector at = RZ_Mission.LandPointNear(m.m_vPos, 3, 6);
		m.m_Keeper = RZ_Util.SpawnPrefab(prefab, at, RZ_Compass.Bearing(at, m.m_vPos));
		if (!m.m_Keeper)
			return;

		RZ_PuzzleObjectComponent.Tag(m.m_Keeper, TAG_PERSON, m.m_iId, "", "");
		RZ_ClueComponent.Tag(m.m_Keeper, TAG_PERSON, m.m_iId, "Talk to " + m.m_sKeeper);
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!IsOn())
			return;

		foreach (RZ_Mast m : m_aMasts)
		{
			UpdateBodies(m, players);
			TickHolds(m);
		}

		TickVoice(dt, ids, players);

		m_fWarnTimer += dt;
		if (m_fWarnTimer >= m_Settings.m_fWarnMinutes * 60)
		{
			m_fWarnTimer = 0;
			Warn(ids, players);
		}

		m_fListenTimer += dt;
		if (m_fListenTimer >= m_Settings.m_fListenMinutes * 60)
		{
			m_fListenTimer = 0;
			Overhear(ids, players);
		}

		m_fChatterTimer += dt;
		if (m_fChatterTimer >= m_Settings.m_fChatterMinutes * 60)
		{
			m_fChatterTimer = 0;
			DroneChatter(ids, players);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void TickHolds(RZ_Mast m)
	{
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (!sites)
			return;

		string climb = CLIMB + m.m_iId.ToString();
		if (sites.IsDone(climb))
		{
			int by = sites.DoneBy(climb);
			sites.Close(climb);
			if (!m.m_bUp && by > 0)
				PutUp(m, by);
		}

		string bearing = BEARING + m.m_iId.ToString();
		if (sites.IsDone(bearing))
		{
			int who = sites.DoneBy(bearing);
			sites.Close(bearing);
			if (who > 0)
				TookBearing(m, who);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Players alive in an up mast's region, with a radio.
	protected void Listeners(RZ_Mast m, array<int> ids, array<IEntity> players, notnull array<int> result)
	{
		result.Clear();
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), m.m_vPos) > m_Settings.m_fRegion)
				continue;

			if (RZ_Radio.HasRadio(m_GM, ids[i], p))
				result.Insert(ids[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Its warnings reach this player: everyone on an open mast, those Known with its faction else.
	protected bool Serves(RZ_Mast m, int playerId)
	{
		if (m.m_eOwner == RZ_EMastOwner.OPEN)
			return true;

		int f = FactionOf(m.m_eOwner);
		return f >= 0 && RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, f), RZ_ETier.KNOWN);
	}

	// ================================================================ Warnings
	//------------------------------------------------------------------------------------------------
	protected void Warn(array<int> ids, array<IEntity> players)
	{
		array<int> listeners = {};
		foreach (RZ_Mast m : m_aMasts)
		{
			if (!m.m_bUp || m.m_eOwner == RZ_EMastOwner.NOBODY)
				continue;

			Listeners(m, ids, players, listeners);
			foreach (int playerId : listeners)
			{
				if (!Serves(m, playerId))
					continue;

				IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
				string line = WarningFor(m, ent);
				if (line.IsEmpty())
					continue;

				m_GM.Say(playerId, Speaker(m), FactionOf(m.m_eOwner), line);
				RZ_Log.Info(string.Format("Masts: warning from %1 to %2: %3", m.m_sName, RZ_Names.Public(playerId), line));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Something true now for someone in the region: a pack near them, a storm, a truck due.
	protected string WarningFor(RZ_Mast m, IEntity ent)
	{
		if (!ent)
			return "";

		RZ_Tribes tribes = m_GM.GetTribes();
		if (tribes && tribes.IsEnabled())
		{
			foreach (RZ_Tribe t : tribes.GetTribeList())
			{
				if (t && RZ_Util.DistXZ(t.m_vLair, ent.GetOrigin()) <= PACK_NEAR)
				{
					string dir = RZ_Compass.DirectionWord(RZ_Compass.Bearing(ent.GetOrigin(), t.m_vLair));
					return string.Format("Movement %1 of you, in %2. A lot of them. Keep your heads down.", dir, t.m_sPlace);
				}
			}

			if (tribes.IsStorm())
				return string.Format("Weather's turning over %1. Get under a roof.", m.m_sTown);
		}

		// A road patrol on a road near you (Factions/RZ_RoadPatrols.c)
		RZ_RoadPatrols patrols = RZ_RoadPatrols.Get();
		if (patrols)
		{
			string patrol = patrols.WarningNear(ent.GetOrigin(), 1500);
			if (!patrol.IsEmpty())
				return patrol;
		}

		RZ_SupplyLines supply = m_GM.GetSupply();
		if (supply)
		{
			array<int> order = {RZ_EFaction.MILITARY, RZ_EFaction.SURVIVORS, RZ_EFaction.INSTITUTE, RZ_EFaction.BANDITS};
			foreach (int f : order)
			{
				string dest = supply.TruckDestination(f);
				if (!dest.IsEmpty())
					return string.Format("A truck's on the road for %1. Stay off it, or wave.", dest);
			}
		}

		return "";
	}

	// ================================================================ Listening in
	//------------------------------------------------------------------------------------------------
	protected void Overhear(array<int> ids, array<IEntity> players)
	{
		array<int> listeners = {};
		foreach (RZ_Mast m : m_aMasts)
		{
			if (!m.m_bUp)
				continue;

			Listeners(m, ids, players, listeners);
			if (listeners.IsEmpty())
				continue;

			foreach (int playerId : listeners)
			{
				// The Red Hand's mast: theirs to listen on; anyone else hears the price
				if (m.m_eOwner == RZ_EMastOwner.REDHAND && !RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.KNOWN))
				{
					m_GM.Say(playerId, "Radio: open channel", RZ_EFaction.BANDITS, string.Format("This is the crew on %1. You're on our wire. Anyone wants to hear what it hears, come and see the book.", m.m_sShort));
					continue;
				}

				string heard = HeardLine(m);
				if (heard.IsEmpty())
					continue;

				m_GM.Say(playerId, "Radio: overheard", -1, heard);
				RZ_Log.Info(string.Format("Masts: overheard at %1 by %2: %3", m.m_sName, RZ_Names.Public(playerId), heard));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! One faction's working traffic, true now: a truck's departure, a patrol's change, a toll.
	string HeardLine(RZ_Mast m)
	{
		RZ_SupplyLines supply = m_GM.GetSupply();
		RZ_Territory territory = m_GM.GetTerritory();
		array<string> options = {};
		array<int> order = {RZ_EFaction.MILITARY, RZ_EFaction.BANDITS, RZ_EFaction.SURVIVORS, RZ_EFaction.INSTITUTE};
		foreach (int f : order)
		{
			if (f == FactionOf(m.m_eOwner))
				continue;

			string who = RZ_FactionVoice.ChannelTitle(f);
			who.Replace("Radio: ", "");
			if (supply)
			{
				float secs = supply.SecondsToDeparture(f);
				string dest = supply.TruckDestination(f);
				if (!dest.IsEmpty())
					options.Insert(string.Format("On %1, a tired voice: \"Truck's through for %2. Keep the road clear.\"", who, dest));
				else if (secs >= 0 && secs <= 1800)
					options.Insert(string.Format("On %1: \"Truck rolls in %2 minutes. Load it and shut up about it.\"", who, RZ_Util.RoundToInt(Math.Max(60, secs) / 60)));
			}

			// One of its road patrols checking in (Factions/RZ_RoadPatrols.c)
			if (RZ_RoadPatrols.Get())
			{
				string walking = RZ_RoadPatrols.Get().HeardLine(f);
				if (!walking.IsEmpty())
					options.Insert(string.Format("On %1, a patrol checking in: %2", who, walking));
			}

			if (territory && territory.IsActive())
			{
				array<RZ_Holding> posts = {};
				territory.GetOutposts(f, posts);
				string clock = territory.NextRoutineClock();
				if (!posts.IsEmpty() && !clock.IsEmpty())
				{
					RZ_Holding post = posts[m_iStep % posts.Count()];
					if (f == RZ_EFaction.BANDITS)
						options.Insert(string.Format("On %1: \"Toll at %2's light tonight. Count it twice.\"", who, post.m_sName));
					else
						options.Insert(string.Format("On %1: \"%2, your relief's at %3. Don't go wandering.\"", who, post.m_sName, clock));
				}
			}
		}

		if (options.IsEmpty())
			return "";

		m_iStep++;
		return options[m_iStep % options.Count()];
	}

	// ================================================================ The drone operators
	//------------------------------------------------------------------------------------------------
	protected void DroneChatter(array<int> ids, array<IEntity> players)
	{
		array<string> lines = {
			"Under the hiss, a flat voice: \"Sector four, nothing moving. Same as yesterday.\"",
			"Two voices, bored: \"You eat yet?\" \"Twice. It's the same thing twice.\"",
			"\"Drone three's coming back on the battery. Somebody log it.\" Nobody answers him.",
			"\"Heat signature at the treeline. Probably a deer. Probably.\"",
			"\"How long's the rotation now?\" \"Ask someone who knows.\""};
		array<int> listeners = {};
		foreach (RZ_Mast m : m_aMasts)
		{
			if (!m.m_bUp || !m.m_bNearLine)
				continue;

			Listeners(m, ids, players, listeners);
			string line = lines[m_iStep % lines.Count()];
			foreach (int playerId : listeners)
			{
				m_GM.Say(playerId, "Radio: past the line", -1, line);
			}

			m_iStep++;
		}
	}

	// ================================================================ Finding a voice
	//------------------------------------------------------------------------------------------------
	protected void TickVoice(float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bVoice)
		{
			m_fVoiceLeft -= dt;
			if (m_fVoiceLeft <= 0)
			{
				m_bVoice = false;
				m_mBearings.Clear();
				RZ_Log.Info("Masts: the trapped voice stopped calling.");
			}

			return;
		}

		if (UpCount() < 2)
			return;

		m_fVoiceTimer += dt;
		if (m_fVoiceTimer < m_Settings.m_fVoiceMinutes * 60)
			return;

		m_fVoiceTimer = 0;
		StartVoice(ids, players);
	}

	//------------------------------------------------------------------------------------------------
	//! A trapped man starts calling from a house in an up mast's region. True if one started.
	bool StartVoice(array<int> ids, array<IEntity> players)
	{
		array<RZ_Mast> up = {};
		foreach (RZ_Mast m : m_aMasts)
		{
			if (m.m_bUp)
				up.Insert(m);
		}

		if (up.Count() < 2)
			return false;

		RZ_Mast from = up[m_iStep % up.Count()];
		vector spot = RZ_PlaceFind.Near(from.m_vPos, 600, m_Settings.m_fRegion * 0.8, RZ_PlaceNeeds.Get("errand.town"));
		if (spot == vector.Zero)
			spot = RZ_Mission.LandPointNear(from.m_vPos, 600, m_Settings.m_fRegion * 0.8);

		if (spot == vector.Zero || !RZ_Mainland.OnMainland(spot))
			return false;

		m_bVoice = true;
		m_vVoice = spot;
		m_fVoiceLeft = m_Settings.m_fVoiceLastsMinutes * 60;
		m_mBearings.Clear();
		RZ_Rng rng = new RZ_Rng(Math.AbsInt(RZ_Util.RoundToInt(spot[0] + spot[2])) + 3);
		m_sVoiceName = RZ_NoteText.ManName(rng);
		RZ_Log.Info(string.Format("Masts: a trapped voice is calling from %1 (%2).", RZ_SettlementScanner.GridReference(spot), m_sVoiceName));

		array<int> listeners = {};
		foreach (RZ_Mast u : up)
		{
			Listeners(u, ids, players, listeners);
			foreach (int playerId : listeners)
			{
				m_GM.Say(playerId, Speaker(u), -1, string.Format("Faint, on the band: '...this is %1... can't get out... anyone...' Take a bearing at two masts and you'd have him.", m_sVoiceName));
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasBearing(int playerId, int mast)
	{
		ref array<int> list;
		if (!m_mBearings.Find(playerId, list))
			return false;

		return list.Contains(mast);
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenBearing(RZ_Mast m, int playerId)
	{
		if (!m_bVoice || HasBearing(playerId, m.m_iId))
			return;

		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (!sites || !RZ_WorkSites.Available())
		{
			TookBearing(m, playerId);
			return;
		}

		string key = BEARING + m.m_iId.ToString();
		sites.Close(key);
		sites.Open(key, RZ_EWork.HANDS, "Take a bearing on the voice", m_Settings.m_fBearingSeconds, CabinetAt(m), playerId);
		sites.SetAtPiece(key, CabinetAt(m), 0);
	}

	//------------------------------------------------------------------------------------------------
	//! A bearing taken at mast `m`. Two from two masts fix the spot: a Distress Call starts there.
	void TookBearing(RZ_Mast m, int playerId)
	{
		if (!m_bVoice)
			return;

		ref array<int> list;
		if (!m_mBearings.Find(playerId, list))
		{
			list = new array<int>();
			m_mBearings.Set(playerId, list);
		}

		if (!list.Contains(m.m_iId))
			list.Insert(m.m_iId);

		string dir = RZ_Compass.DirectionWord(RZ_Compass.Bearing(m.m_vPos, m_vVoice));
		RZ_Log.Info(string.Format("Masts: %1 took a bearing at %2: %3 (%4 of 2).", RZ_Names.Public(playerId), m.m_sName, dir, list.Count()));
		if (list.Count() < 2)
		{
			m_GM.Say(playerId, Speaker(m), -1, string.Format("The needle peaks to the %1. One line. Another mast would cross it.", dir));
			return;
		}

		m_GM.Say(playerId, Speaker(m), -1, string.Format("The needle peaks to the %1, and the two lines cross. He's in a house %2.", dir, RZ_Util.DirectionText(m.m_vPos, m_vVoice)));
		StartCall(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! The Distress Call, at the spot the bearings crossed.
	protected bool StartCall(int playerId)
	{
		vector spot = m_vVoice;
		m_vLastCall = spot;
		m_bVoice = false;
		m_mBearings.Clear();
		RZ_MissionManager missions = m_GM.GetMissions();
		if (!missions)
			return false;

		RZ_Event_DistressBase.ForceSpot(spot);
		RZ_Event_DistressCall call = new RZ_Event_DistressCall();
		call.Configure(m_GM);
		bool started = missions.StartMission(call);
		RZ_Event_DistressBase.ForceSpot(vector.Zero);
		RZ_Log.Info(string.Format("Masts: the bearings crossed at %1: Distress Call %2.", RZ_SettlementScanner.GridReference(spot), started));
		m_bLastCallStarted = started;
		m_vLastCallAt = call.m_vPos;
		return started;
	}

	// ================================================================ Test hooks (tools/runtime_test)
	//------------------------------------------------------------------------------------------------
	//! A mast up and given, as if climbed (the runtime test).
	void TestPutUp(int index, int playerId, IEntity ent, string who)
	{
		RZ_Mast m = GetMast(index);
		if (!m)
			return;

		m.m_bSettled = true;
		m.m_bPromised = false;
		m.m_eOwner = RZ_EMastOwner.NOBODY;
		m.m_bUp = false;
		if (ent)
			GiveTestPart(m, ent);

		PutUp(m, playerId, ent);
		if (!who.IsEmpty() && m.m_bUp && m.m_eOwner == RZ_EMastOwner.NOBODY)
			Give(m, playerId, who);
	}

	//------------------------------------------------------------------------------------------------
	//! Gives the part's base-game item (the runtime test's stand-in: the fuel can is a jerrycan).
	protected void GiveTestPart(RZ_Mast m, IEntity ent)
	{
		m.m_ePart = RZ_EMastPart.FUEL;
		IEntity can = RZ_Util.SpawnPrefab("{12D5AD21E383B768}Prefabs/Items/Fuel/Jerrycan_01/Jerrycan_01_item.et", ent.GetOrigin(), 0);
		if (can && !RZ_Util.InsertIntoInventory(ent, can))
			RZ_Util.DeleteUnit(can);
	}

	//------------------------------------------------------------------------------------------------
	//! A warning line for the test, from mast `index` to whoever's at `ent`.
	string TestWarning(int index, IEntity ent)
	{
		RZ_Mast m = GetMast(index);
		if (!m)
			return "";

		return WarningFor(m, ent);
	}
}
