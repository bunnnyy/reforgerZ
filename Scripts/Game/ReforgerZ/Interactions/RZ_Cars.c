//! Cars as easy projects, kept rare (docs/DESIGN.md, "World interactions"). At most one
//! fixable car per town and a small server-wide cap (about one per three settlements); every
//! other wreck is just a wreck you can search, strip for one part or siphon.
//!
//! A fixable car is data until it runs: where it stands, what it's missing (one or two of
//! fuel, battery, tyre, spark plugs), whether its keys are in a house in that town and whether
//! someone hotwired it. Near players it's a prop you can inspect ("Needs: fuel, battery"),
//! fit parts to, fuel, hotwire (about 20 s, 5 s for a Mechanic, a little noise) and start. On
//! the first start the prop is swapped for a real vehicle prefab, and from then on it's an
//! ordinary vehicle: vanilla repair handles damage, vanilla persistence (if the server runs
//! it) saves it. A new project turns up somewhere else a few hours later.
//!
//! Parts come from garages and car boots (search), stripping wrecks (one part each), a
//! Mechanic wanderer or the Haven quartermaster (shop actions) for Favor. Fuel comes from
//! siphoning wrecks or a petrol station's hand pump into a container.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Interactions > Cars.
[BaseContainerProps()]
class RZ_CarSettings
{
	[Attribute("1", desc: "Fixable cars and wrecks you can strip and siphon")]
	bool m_bEnabled = true;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Drivable vehicles a fixed car becomes (one at random). Empty = no fixable cars (wrecks still work)", "et")]
	ref array<ResourceName> m_aCarPrefabs = {};

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "The car before it runs: a non-drivable car prop with the generic puzzle object's components. Empty = the interaction spot on the drivable prefab's position (Puzzles > Generic object)", "et")]
	ResourceName m_sProjectPrefab;

	[Attribute("0.34", desc: "Fixable cars per settlement on the map (about 1 per 3), at most one per town")]
	float m_fPerSettlement = 0.34;

	[Attribute("6", desc: "Most fixable cars on the server at once")]
	int m_iMaxCars = 6;

	[Attribute("6", desc: "Real hours before a fixed car's project is replaced somewhere else")]
	float m_fReplaceHours = 6;

	[Attribute("0.25", desc: "Fuel in a freshly fixed car (0..1 of its tank)")]
	float m_fStartFuel = 0.25;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Car battery item (stripping, the Mechanic)", "et")]
	ResourceName m_sBatteryPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Tyre item", "et")]
	ResourceName m_sTyrePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Spark plugs item", "et")]
	ResourceName m_sSparkPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "A full fuel can (siphoning, the hand pump). Empty = no siphoning or pumping", "et")]
	ResourceName m_sFuelPrefab;

	[Attribute("battery,accumulator", desc: "Car battery keywords")]
	string m_sBatteryWords = "battery,accumulator";

	[Attribute("tyre,tire,wheel", desc: "Tyre keywords")]
	string m_sTyreWords = "tyre,tire,wheel";

	[Attribute("spark", desc: "Spark plug keywords")]
	string m_sSparkWords = "spark";

	[Attribute("jerrycan,jerry_can,fuelcan,fuel_can,canister", desc: "Fuel keywords (one is used up fuelling a car)")]
	string m_sFuelWords = "jerrycan,jerry_can,fuelcan,fuel_can,canister";

	[Attribute("jerrycan_empty,empty_jerrycan,canister_empty,empty_canister,fuelcan_empty", desc: "Empty fuel container keywords (filled by siphoning or pumping)")]
	string m_sContainerWords = "jerrycan_empty,empty_jerrycan,canister_empty,empty_canister,fuelcan_empty";

	[Attribute("wreck,carwreck,car_wreck,burntcar,burnt_car,car_burnt", desc: "Wreck keywords (search the boot, strip a part, siphon)")]
	string m_sWreckWords = "wreck,carwreck,car_wreck,burntcar,burnt_car,car_burnt";

	[Attribute("20", desc: "Seconds to hotwire")]
	float m_fHotwireSeconds = 20;

	[Attribute("5", desc: "Seconds to hotwire for a Mechanic")]
	float m_fMechanicHotwireSeconds = 5;

	[Attribute("8", desc: "Heat hotwiring makes (sparks, the starter)")]
	float m_fHotwireHeat = 8;

	[Attribute("15", desc: "Heat an engine starting makes")]
	float m_fStartHeat = 15;

	[Attribute("0.5", desc: "Chance a fixable car's keys are in a house in its town")]
	float m_fKeysChance = 0.5;

	[Attribute("0.2", desc: "Chance a search in that town finds them")]
	float m_fKeyFindChance = 0.2;

	[Attribute("8", desc: "Seconds to fit parts or fuel")]
	float m_fFitSeconds = 8;

	[Attribute("12", desc: "Seconds to strip a part from a wreck")]
	float m_fStripSeconds = 12;

	[Attribute("15", desc: "Seconds to siphon a wreck")]
	float m_fSiphonSeconds = 15;

	[Attribute("0.6", desc: "Chance a wreck still has fuel to siphon")]
	float m_fSiphonChance = 0.6;

	[Attribute("30", desc: "Seconds to fill a can at a petrol station's hand pump")]
	float m_fPumpSeconds = 30;

	[Attribute("10", desc: "Heat the hand pump makes")]
	float m_fPumpHeat = 10;

	[Attribute("20", desc: "Real minutes before the same pump gives fuel again")]
	float m_fPumpCooldownMinutes = 20;

	[Attribute("120", desc: "Real minutes before a stripped or siphoned wreck has anything again")]
	float m_fWreckReturnMinutes = 120;

	[Attribute("12", desc: "Haven Favor for a part from a Mechanic wanderer")]
	int m_iPartCost = 12;
}

//------------------------------------------------------------------------------------------------
class RZ_CarSave
{
	int id;
	int town;
	float x;
	float z;
	float yaw;
	int missing;
	bool hasKeys;
	bool hotwired;
	string noun;
}

//------------------------------------------------------------------------------------------------
class RZ_CarRosterSave
{
	int nextId = 1;
	float nextAt;
	ref array<ref RZ_CarSave> list = {};
}

//------------------------------------------------------------------------------------------------
//! One fixable car. Data; the prop exists only near players.
class RZ_CarProject
{
	int m_iId;
	int m_iTown = -1;
	vector m_vPos;
	float m_fYaw;
	int m_iMissing;			// RZ_Cars.PART_* bits
	bool m_bHasKeys;		// its keys are in a house in its town
	bool m_bKeysFound;		// someone has them (this session)
	bool m_bHotwired;
	string m_sNoun;			// "the blue hatchback"

	IEntity m_Body;
	int m_iSpot = -1;
	ref array<int> m_aInspected = {};
}

//------------------------------------------------------------------------------------------------
class RZ_Cars
{
	static const int PART_FUEL = 1;
	static const int PART_BATTERY = 2;
	static const int PART_TYRE = 4;
	static const int PART_SPARK = 8;
	static const int PARTS_NOT_FUEL = 14;	// battery, tyre and spark plugs

	protected static const float SHOW_RADIUS = 150;
	protected static const float HIDE_RADIUS = 200;

	protected RZ_GameModeComponent m_GM;
	protected RZ_Interactions m_Hub;
	protected RZ_CarSettings m_Settings;
	protected ref array<ref RZ_CarProject> m_aCars = {};
	protected ref map<string, float> m_mWreck = new map<string, float>();
	protected ref array<string> m_aWreckWords = {};
	protected int m_iNextId = 1;
	protected float m_fNextAt;
	protected bool m_bOn;

	//------------------------------------------------------------------------------------------------
	void RZ_Cars(RZ_GameModeComponent gm, RZ_Interactions hub, RZ_CarSettings settings)
	{
		m_GM = gm;
		m_Hub = hub;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	int Count() { return m_aCars.Count(); }

	//------------------------------------------------------------------------------------------------
	bool IsWreckName(string name)
	{
		if (!m_Settings.m_bEnabled || !RZ_ItemHelper.NameHas(name, m_aWreckWords))
			return false;

		return !name.Contains("ship") && !name.Contains("boat") && !name.Contains("heli") && !name.Contains("plane") && !name.Contains("tank");
	}

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	void SaveInto(RZ_InteractionSave data)
	{
		RZ_CarRosterSave roster = new RZ_CarRosterSave();
		roster.nextId = m_iNextId;
		roster.nextAt = m_fNextAt;
		foreach (RZ_CarProject c : m_aCars)
		{
			RZ_CarSave cs = new RZ_CarSave();
			cs.id = c.m_iId;
			cs.town = c.m_iTown;
			cs.x = c.m_vPos[0];
			cs.z = c.m_vPos[2];
			cs.yaw = c.m_fYaw;
			cs.missing = c.m_iMissing;
			cs.hasKeys = c.m_bHasKeys;
			cs.hotwired = c.m_bHotwired;
			cs.noun = c.m_sNoun;
			roster.list.Insert(cs);
		}

		data.cars = roster;

		float now = m_Hub.GetClock();
		float keep = m_Settings.m_fWreckReturnMinutes * 60;
		foreach (string key, float at : m_mWreck)
		{
			if (now - at < keep)
			{
				data.wreckKey.Insert(key);
				data.wreckAt.Insert(at);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! After the hub restored its clock. A restart loses who carried a car's keys, so they can be
	//! found again.
	void Setup(RZ_CarRosterSave saved)
	{
		RZ_ItemHelper.Words(m_Settings.m_sWreckWords, m_aWreckWords);
		m_bOn = m_Settings.m_bEnabled && m_Settings.m_aCarPrefabs && !m_Settings.m_aCarPrefabs.IsEmpty();
		if (saved)
		{
			m_iNextId = Math.Max(1, saved.nextId);
			m_fNextAt = saved.nextAt;
			foreach (RZ_CarSave cs : saved.list)
			{
				RZ_CarProject c = new RZ_CarProject();
				c.m_iId = cs.id;
				c.m_iTown = cs.town;
				c.m_vPos = RZ_Util.GroundPos(Vector(cs.x, 0, cs.z));
				c.m_fYaw = cs.yaw;
				c.m_iMissing = cs.missing;
				c.m_bHasKeys = cs.hasKeys;
				c.m_bHotwired = cs.hotwired;
				c.m_sNoun = cs.noun;
				m_aCars.Insert(c);
			}
		}

		if (!m_bOn)
		{
			m_aCars.Clear();
			return;
		}

		if (!saved)
			Seed();
	}

	//------------------------------------------------------------------------------------------------
	void RestoreWrecks(RZ_InteractionSave data)
	{
		for (int i = 0; i < data.wreckKey.Count() && i < data.wreckAt.Count(); i++)
		{
			m_mWreck.Set(data.wreckKey[i], data.wreckAt[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected int Cap()
	{
		int n = RZ_Util.RoundToInt(m_GM.GetSettlements().Count() * m_Settings.m_fPerSettlement);
		return Math.Clamp(n, 1, Math.Max(1, m_Settings.m_iMaxCars));
	}

	//------------------------------------------------------------------------------------------------
	//! Fills up to the cap, one per town, in towns that aren't overrun or by the line.
	protected void Seed()
	{
		int guard = 0;
		while (m_aCars.Count() < Cap() && guard < 40)
		{
			guard++;
			int idx = RZ_Mission.PickSettlement(m_GM, 0, 90);
			RZ_Settlement s = m_GM.GetSettlement(idx);
			if (!s || HasCarIn(idx))
				continue;

			vector pos = RZ_Mission.LandPointNear(s.m_vCenter, 15, Math.Max(25, s.m_fRadius * 0.6));
			if (RZ_Util.IsWater(pos) || RZ_Perimeter.IsNearLineAt(pos))
				continue;

			m_aCars.Insert(Generate(idx, pos));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasCarIn(int town)
	{
		foreach (RZ_CarProject c : m_aCars)
		{
			if (c.m_iTown == town)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! One or two of fuel, battery, tyre, spark plugs.
	protected RZ_CarProject Generate(int town, vector pos)
	{
		RZ_CarProject c = new RZ_CarProject();
		c.m_iId = m_iNextId;
		m_iNextId++;
		c.m_iTown = town;
		c.m_vPos = pos;
		c.m_fYaw = Math.RandomFloat(0, 360);

		array<int> parts = {PART_FUEL, PART_BATTERY, PART_TYRE, PART_SPARK};
		int first = parts.GetRandomElement();
		c.m_iMissing = first;
		if (Math.RandomFloat01() < 0.6)
		{
			int second = parts.GetRandomElement();
			c.m_iMissing = c.m_iMissing | second;
		}

		c.m_bHasKeys = Math.RandomFloat01() < m_Settings.m_fKeysChance;
		array<string> colours = {"blue", "white", "red", "grey", "green", "rust-brown", "yellow"};
		array<string> kinds = {"hatchback", "estate", "saloon", "van", "pickup"};
		c.m_sNoun = string.Format("the %1 %2", colours.GetRandomElement(), kinds.GetRandomElement());
		return c;
	}

	//------------------------------------------------------------------------------------------------
	static string PartsText(int missing)
	{
		array<string> words = {};
		if ((missing & PART_FUEL) != 0)
			words.Insert("fuel");
		if ((missing & PART_BATTERY) != 0)
			words.Insert("a battery");
		if ((missing & PART_TYRE) != 0)
			words.Insert("a tyre");
		if ((missing & PART_SPARK) != 0)
			words.Insert("spark plugs");

		string text = "";
		foreach (int i, string w : words)
		{
			if (i > 0)
				text += ", ";

			text += w;
		}

		return text;
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<IEntity> players)
	{
		if (!m_bOn)
			return;

		if (m_fNextAt > 0 && m_Hub.GetClock() >= m_fNextAt)
		{
			m_fNextAt = 0;
			Seed();
		}

		foreach (RZ_CarProject c : m_aCars)
		{
			float nearest = RZ_Util.NearestDistance(c.m_vPos, players);
			if (c.m_Body && nearest > HIDE_RADIUS)
			{
				Hide(c);
				continue;
			}

			if (!c.m_Body && nearest <= SHOW_RADIUS)
				Show(c);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Show(RZ_CarProject c)
	{
		ResourceName prefab = m_Settings.m_sProjectPrefab;
		if (prefab.IsEmpty())
			prefab = m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.OBJECT);

		c.m_iSpot = m_Hub.AddSpot(RZ_Interactions.KIND_CAR, c.m_vPos, "car", c.m_sNoun, c.m_iId, -1);
		if (c.m_iSpot < 0)
			return;

		c.m_Body = RZ_Util.SpawnPrefab(prefab, c.m_vPos, c.m_fYaw);
		RZ_Spot s = m_Hub.FindSpot(c.m_iSpot);
		if (!c.m_Body || !s)
		{
			m_Hub.RemoveSpot(c.m_iSpot);
			c.m_iSpot = -1;
			return;
		}

		m_Hub.AttachLabels(s, c.m_Body);
	}

	//------------------------------------------------------------------------------------------------
	protected void Hide(RZ_CarProject c)
	{
		if (c.m_Body)
			RZ_Util.DeleteUnit(c.m_Body);

		c.m_Body = null;
		RZ_Spot s = m_Hub.FindSpot(c.m_iSpot);
		if (s)
			s.m_Body = null;

		m_Hub.RemoveSpot(c.m_iSpot);
		c.m_iSpot = -1;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_CarProject Find(int id)
	{
		foreach (RZ_CarProject c : m_aCars)
		{
			if (c.m_iId == id)
				return c;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected string TownName(RZ_CarProject c)
	{
		RZ_Settlement s = m_GM.GetSettlement(c.m_iTown);
		if (s)
			return s.m_sName;

		return RZ_SettlementScanner.GridReference(c.m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	protected string CarKey(RZ_CarProject c)
	{
		return string.Format("car@%1", c.m_iId);
	}

	// ================================================================ The project
	//------------------------------------------------------------------------------------------------
	void ProjectLabels(RZ_Spot s, out string read, out string choiceLabels)
	{
		read = "Inspect the car";
		choiceLabels = "Fit parts|Fuel it|Hotwire it (or use its keys)|Start the engine";
	}

	//------------------------------------------------------------------------------------------------
	//! "Needs: fuel, a battery." The first inspect gets the "car" tip.
	void Inspect(int playerId, RZ_Spot s, IEntity user)
	{
		RZ_CarProject c = Find(s.m_iRef);
		if (!c)
			return;

		string needs = "Nothing. It would run.";
		if (c.m_iMissing != 0)
			needs = string.Format("Needs: %1.", PartsText(c.m_iMissing));

		string keys = "No keys in it. You could hotwire it.";
		if (c.m_bHotwired)
			keys = "The wires under the dash are already stripped and twisted together.";
		else if (m_Hub.GetSearch().HasKey(playerId, CarKey(c)))
			keys = "Its keys are in your pocket.";
		else if (c.m_bHasKeys)
			keys = "No keys in it. A spare's often in a house nearby; or hotwire it.";

		m_GM.NotifyPlayer(playerId, m_Hub.Title(s), string.Format("%1, under a layer of dust. Nothing wrong you can't fix. %2 %3", m_Hub.Title(s), needs, keys));
		if (!c.m_aInspected.Contains(playerId))
		{
			c.m_aInspected.Insert(playerId);
			RZ_Guidance.Report(playerId, "car", TownName(c));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! 0 fit parts, 1 fuel it, 2 hotwire (or keys), 3 start.
	void HandleProject(int playerId, RZ_Spot s, int option, IEntity user)
	{
		RZ_CarProject c = Find(s.m_iRef);
		if (!c)
			return;

		string title = m_Hub.Title(s);
		if (option == 0 || option == 1)
		{
			int mask = c.m_iMissing & PARTS_NOT_FUEL;
			if (option == 1)
				mask = c.m_iMissing & PART_FUEL;

			if (mask == 0)
			{
				m_GM.NotifyPlayer(playerId, title, "It doesn't need that.");
				return;
			}

			if (PartsCarried(user, mask) == 0)
			{
				m_GM.NotifyPlayer(playerId, title, string.Format("You've nothing it needs. It wants %1.", PartsText(mask)));
				return;
			}

			m_Hub.StartTask(playerId, user, RZ_Interactions.TASK_FIT, s.m_iId, mask, m_Settings.m_fFitSeconds, "the work on the car", false);
			return;
		}

		if (option == 2)
		{
			if (c.m_bHotwired)
			{
				m_GM.NotifyPlayer(playerId, title, "It's already hotwired.");
				return;
			}

			if (m_Hub.GetSearch().HasKey(playerId, CarKey(c)))
			{
				c.m_bHotwired = true;
				m_GM.NotifyPlayer(playerId, title, "The key fits. You leave it in the ignition.");
				return;
			}

			float seconds = m_Settings.m_fHotwireSeconds;
			if (RZ_Interactions.IsBackground(m_GM, playerId, "mechanic"))
				seconds = m_Settings.m_fMechanicHotwireSeconds;

			m_GM.GetHeat().AddHeat(c.m_vPos, m_Settings.m_fHotwireHeat * 0.5);
			m_Hub.StartTask(playerId, user, RZ_Interactions.TASK_HOTWIRE, s.m_iId, c.m_iId, seconds, "the hotwiring", false);
			m_GM.NotifyPlayer(playerId, title, string.Format("You pull the panel under the dash. About %1 seconds; stay with it.", RZ_Util.RoundToInt(seconds)));
			return;
		}

		if (option == 3)
			TryStart(playerId, c, s);
	}

	//------------------------------------------------------------------------------------------------
	protected int PartsCarried(IEntity user, int mask)
	{
		int found = 0;
		if ((mask & PART_FUEL) != 0 && RZ_ItemHelper.Has(user, m_Settings.m_sFuelWords))
			found = found | PART_FUEL;
		if ((mask & PART_BATTERY) != 0 && RZ_ItemHelper.Has(user, m_Settings.m_sBatteryWords))
			found = found | PART_BATTERY;
		if ((mask & PART_TYRE) != 0 && RZ_ItemHelper.Has(user, m_Settings.m_sTyreWords))
			found = found | PART_TYRE;
		if ((mask & PART_SPARK) != 0 && RZ_ItemHelper.Has(user, m_Settings.m_sSparkWords))
			found = found | PART_SPARK;

		return found;
	}

	//------------------------------------------------------------------------------------------------
	void CompleteFit(int playerId, RZ_Spot s, int mask, IEntity user)
	{
		if (!s)
			return;

		RZ_CarProject c = Find(s.m_iRef);
		if (!c)
			return;

		int fitted = 0;
		if ((mask & PART_FUEL) != 0 && RZ_ItemHelper.Take(user, m_Settings.m_sFuelWords, 1) > 0)
			fitted = fitted | PART_FUEL;
		if ((mask & PART_BATTERY) != 0 && RZ_ItemHelper.Take(user, m_Settings.m_sBatteryWords, 1) > 0)
			fitted = fitted | PART_BATTERY;
		if ((mask & PART_TYRE) != 0 && RZ_ItemHelper.Take(user, m_Settings.m_sTyreWords, 1) > 0)
			fitted = fitted | PART_TYRE;
		if ((mask & PART_SPARK) != 0 && RZ_ItemHelper.Take(user, m_Settings.m_sSparkWords, 1) > 0)
			fitted = fitted | PART_SPARK;

		c.m_iMissing = c.m_iMissing - (c.m_iMissing & fitted);
		m_GM.GetHeat().AddHeat(c.m_vPos, 2);
		string left = "That's everything. Now it needs starting.";
		if (c.m_iMissing != 0)
			left = string.Format("It still needs %1.", PartsText(c.m_iMissing));

		if (fitted == 0)
		{
			m_GM.NotifyPlayer(playerId, m_Hub.Title(s), "Whatever you meant to fit, you haven't got it any more.");
			return;
		}

		m_GM.NotifyPlayer(playerId, m_Hub.Title(s), string.Format("Fitted: %1. %2", PartsText(fitted), left));
	}

	//------------------------------------------------------------------------------------------------
	void CompleteHotwire(int playerId, RZ_Spot s, int carId)
	{
		RZ_CarProject c = Find(carId);
		if (!c)
			return;

		c.m_bHotwired = true;
		m_GM.GetHeat().AddHeat(c.m_vPos, m_Settings.m_fHotwireHeat);
		m_GM.NotifyPlayer(playerId, c.m_sNoun, "A spark, a cough from the starter. The wires are live.");
	}

	//------------------------------------------------------------------------------------------------
	//! Runs once nothing is missing and it's hotwired (or the starter has its keys): the prop
	//! becomes a real car.
	protected void TryStart(int playerId, RZ_CarProject c, RZ_Spot s)
	{
		string title = m_Hub.Title(s);
		if (c.m_iMissing != 0)
		{
			m_GM.NotifyPlayer(playerId, title, string.Format("Nothing happens. It still needs %1.", PartsText(c.m_iMissing)));
			return;
		}

		if (!c.m_bHotwired && !m_Hub.GetSearch().HasKey(playerId, CarKey(c)))
		{
			m_GM.NotifyPlayer(playerId, title, "No keys. Hotwire it first.");
			return;
		}

		ResourceName prefab = m_Settings.m_aCarPrefabs.GetRandomElement();
		vector pos = c.m_vPos;
		float yaw = c.m_fYaw;
		string logLine = string.Format("Cars: %1 in %2 fixed by player %3.", c.m_sNoun, TownName(c), playerId);
		Hide(c);
		IEntity car = RZ_Util.SpawnPrefab(prefab, pos + Vector(0, 0.3, 0), yaw);
		if (!car)
		{
			m_GM.NotifyPlayer(playerId, title, "It turns over and dies. (The car prefab is missing.)");
			return;
		}

		RZ_Util.SetVehicleFuel(car, m_Settings.m_fStartFuel);
		m_aCars.RemoveItem(c);
		if (m_fNextAt <= 0)
			m_fNextAt = m_Hub.GetClock() + m_Settings.m_fReplaceHours * 3600;

		m_GM.GetHeat().AddHeat(pos, m_Settings.m_fStartHeat);
		m_GM.NotifyPlayer(playerId, title, "It catches on the second try and settles into a rough idle. The noise is enormous. It's yours, for as long as you can keep it.");
		RZ_Log.Info(logLine);
	}

	// ================================================================ Keys and the Mechanic
	//------------------------------------------------------------------------------------------------
	//! From a search: sometimes the keys of a fixable car in this town. "" if not.
	string MaybeFindKeys(int playerId, vector pos)
	{
		if (!m_bOn)
			return "";

		int town = m_GM.FindNearestSettlement(pos);
		foreach (RZ_CarProject c : m_aCars)
		{
			if (c.m_iTown != town || !c.m_bHasKeys || c.m_bKeysFound)
				continue;

			if (Math.RandomFloat01() >= m_Settings.m_fKeyFindChance)
				return "";

			c.m_bKeysFound = true;
			m_Hub.GetSearch().GiveKey(playerId, CarKey(c));
			return string.Format("car keys on a fob shaped like a fish: they'll fit %1 in %2", c.m_sNoun, TownName(c));
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The Mechanic wanderer's "Ask about cars": the nearest fixable car, what it needs, and its
	//! keys if they like you and know where they are.
	string MechanicTalk(int playerId, vector from, bool friendly)
	{
		if (!m_bOn)
			return "\"Cars? Nothing out here runs any more. Believe me, I've looked.\"";

		RZ_CarProject best;
		float bestDist = float.MAX;
		foreach (RZ_CarProject c : m_aCars)
		{
			float d = RZ_Util.DistXZ(c.m_vPos, from);
			if (d < bestDist)
			{
				bestDist = d;
				best = c;
			}
		}

		if (!best)
			return "\"Nothing I know of that runs. Someone beat you to the last one.\"";

		string text = string.Format("\"There's %1 in %2 (%3) that would run with %4. Most wrecks are just wrecks, mind. That one isn't.\"",
			best.m_sNoun, TownName(best), RZ_Util.DirectionText(from, best.m_vPos), PartsText(best.m_iMissing));
		if (friendly && best.m_bHasKeys && !best.m_bKeysFound)
		{
			best.m_bKeysFound = true;
			m_Hub.GetSearch().GiveKey(playerId, CarKey(best));
			text += " They dig in a pocket. \"Here. Found its keys in a kitchen drawer last week. Didn't have the parts.\"";
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! A part the nearest fixable car needs (else any), for the Mechanic's shop. Empty if none set.
	ResourceName PartFor(vector from)
	{
		int need = 0;
		float bestDist = float.MAX;
		foreach (RZ_CarProject c : m_aCars)
		{
			float d = RZ_Util.DistXZ(c.m_vPos, from);
			if (d < bestDist && (c.m_iMissing & PARTS_NOT_FUEL) != 0)
			{
				bestDist = d;
				need = c.m_iMissing;
			}
		}

		array<ResourceName> parts = {};
		if ((need & PART_BATTERY) != 0 && !m_Settings.m_sBatteryPrefab.IsEmpty())
			parts.Insert(m_Settings.m_sBatteryPrefab);
		if ((need & PART_TYRE) != 0 && !m_Settings.m_sTyrePrefab.IsEmpty())
			parts.Insert(m_Settings.m_sTyrePrefab);
		if ((need & PART_SPARK) != 0 && !m_Settings.m_sSparkPrefab.IsEmpty())
			parts.Insert(m_Settings.m_sSparkPrefab);

		if (parts.IsEmpty())
			AllParts(parts);

		if (parts.IsEmpty())
			return "";

		return parts.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	protected void AllParts(notnull array<ResourceName> parts)
	{
		if (!m_Settings.m_sBatteryPrefab.IsEmpty())
			parts.Insert(m_Settings.m_sBatteryPrefab);
		if (!m_Settings.m_sTyrePrefab.IsEmpty())
			parts.Insert(m_Settings.m_sTyrePrefab);
		if (!m_Settings.m_sSparkPrefab.IsEmpty())
			parts.Insert(m_Settings.m_sSparkPrefab);
	}

	int GetPartCost() { return m_Settings.m_iPartCost; }

	// ================================================================ Wrecks
	//------------------------------------------------------------------------------------------------
	void WreckLabels(RZ_Spot s, out string read, out string choiceLabels)
	{
		read = "Look at the wreck";
		choiceLabels = "Search the boot|Strip a part|Siphon fuel (empty can)";
	}

	//------------------------------------------------------------------------------------------------
	void DescribeWreck(int playerId, RZ_Spot s)
	{
		string text = "Flat tyres, smashed glass, a child seat in the back. Just a wreck. The boot might have something; the engine bay still has parts.";
		if (WreckUsed("strip:" + s.m_sKey))
			text += " Someone's had the parts out of it already.";

		m_GM.NotifyPlayer(playerId, "Wreck", text);
	}

	//------------------------------------------------------------------------------------------------
	protected bool WreckUsed(string key)
	{
		float at;
		return m_mWreck.Find(key, at) && m_Hub.GetClock() - at < m_Settings.m_fWreckReturnMinutes * 60;
	}

	//------------------------------------------------------------------------------------------------
	//! 0 search the boot, 1 strip a part, 2 siphon.
	void HandleWreck(int playerId, RZ_Spot s, int option, IEntity user)
	{
		if (option == 0)
		{
			m_Hub.GetSearch().StartSearch(playerId, s, user);
			return;
		}

		if (option == 1)
		{
			if (WreckUsed("strip:" + s.m_sKey))
			{
				m_GM.NotifyPlayer(playerId, "Wreck", "Stripped already. Somebody had the battery and the plugs out of it.");
				return;
			}

			array<ResourceName> parts = {};
			AllParts(parts);
			if (parts.IsEmpty())
			{
				m_GM.NotifyPlayer(playerId, "Wreck", "Nothing in there worth the skinned knuckles.");
				return;
			}

			m_Hub.StartTask(playerId, user, RZ_Interactions.TASK_STRIP, s.m_iId, 0, m_Settings.m_fStripSeconds, "the stripping", false);
			return;
		}

		if (option == 2)
		{
			if (m_Settings.m_sFuelPrefab.IsEmpty())
				return;

			if (WreckUsed("siphon:" + s.m_sKey))
			{
				m_GM.NotifyPlayer(playerId, "Wreck", "The tank's been drained. You can smell it.");
				return;
			}

			if (!RZ_ItemHelper.Has(user, m_Settings.m_sContainerWords))
			{
				m_GM.NotifyPlayer(playerId, "Wreck", "You need an empty can to siphon into.");
				return;
			}

			m_Hub.StartTask(playerId, user, RZ_Interactions.TASK_SIPHON, s.m_iId, 0, m_Settings.m_fSiphonSeconds, "the siphoning", false);
		}
	}

	//------------------------------------------------------------------------------------------------
	void CompleteStrip(int playerId, RZ_Spot s, IEntity user)
	{
		if (!s)
			return;

		m_mWreck.Set("strip:" + s.m_sKey, m_Hub.GetClock());
		array<ResourceName> parts = {};
		AllParts(parts);
		m_GM.GetHeat().AddHeat(s.m_vPos, 4);
		if (parts.IsEmpty())
			return;

		ResourceName part = parts.GetRandomElement();
		RZ_ItemHelper.Give(part, user, s.m_vPos);
		m_GM.NotifyPlayer(playerId, "Wreck", string.Format("Ten minutes of swearing and one part: %1. The rest is rust.", RZ_ItemHelper.DisplayName(part)));
	}

	//------------------------------------------------------------------------------------------------
	void CompleteSiphon(int playerId, RZ_Spot s, IEntity user)
	{
		if (!s)
			return;

		m_mWreck.Set("siphon:" + s.m_sKey, m_Hub.GetClock());
		if (Math.RandomFloat01() >= m_Settings.m_fSiphonChance)
		{
			m_GM.NotifyPlayer(playerId, "Wreck", "A mouthful of air and a taste of petrol. Dry.");
			return;
		}

		if (RZ_ItemHelper.Take(user, m_Settings.m_sContainerWords, 1) <= 0)
			return;

		RZ_ItemHelper.Give(m_Settings.m_sFuelPrefab, user, s.m_vPos);
		m_GM.NotifyPlayer(playerId, "Wreck", "The can fills slowly. Enough to get a car somewhere.");
	}

	//------------------------------------------------------------------------------------------------
	//! A petrol station's hand pump: louder and slower, but nearly always has fuel.
	void HandlePump(int playerId, RZ_Spot s, IEntity user)
	{
		if (m_Settings.m_sFuelPrefab.IsEmpty() || !m_Settings.m_bEnabled)
		{
			m_GM.NotifyPlayer(playerId, "Fuel pump", "The pump handle moves, and nothing comes.");
			return;
		}

		float at;
		if (m_mWreck.Find("pump:" + s.m_sKey, at) && m_Hub.GetClock() - at < m_Settings.m_fPumpCooldownMinutes * 60)
		{
			m_GM.NotifyPlayer(playerId, "Fuel pump", "It's sucking air. The tank needs time to settle before there's more to draw.");
			return;
		}

		if (!RZ_ItemHelper.Has(user, m_Settings.m_sContainerWords))
		{
			m_GM.NotifyPlayer(playerId, "Fuel pump", "You need an empty can to fill.");
			return;
		}

		m_GM.GetHeat().AddHeat(s.m_vPos, m_Settings.m_fPumpHeat * 0.5);
		m_Hub.StartTask(playerId, user, RZ_Interactions.TASK_PUMP, s.m_iId, 0, m_Settings.m_fPumpSeconds, "the pumping", false);
		m_GM.NotifyPlayer(playerId, "Fuel pump", string.Format("The hand pump squeals on every stroke. About %1 seconds.", RZ_Util.RoundToInt(m_Settings.m_fPumpSeconds)));
	}

	//------------------------------------------------------------------------------------------------
	void CompletePump(int playerId, RZ_Spot s, IEntity user)
	{
		if (!s)
			return;

		m_mWreck.Set("pump:" + s.m_sKey, m_Hub.GetClock());
		m_GM.GetHeat().AddHeat(s.m_vPos, m_Settings.m_fPumpHeat);
		if (RZ_ItemHelper.Take(user, m_Settings.m_sContainerWords, 1) <= 0)
			return;

		RZ_ItemHelper.Give(m_Settings.m_sFuelPrefab, user, s.m_vPos);
		m_GM.NotifyPlayer(playerId, "Fuel pump", "A full can, and arms that ache. Everything nearby heard the squealing.");
	}
}
