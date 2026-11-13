//! Cars as easy projects, kept rare (docs/DESIGN.md, "World interactions"). At most one
//! fixable car per town and a small server-wide cap (about one per three settlements); every
//! other wreck is just a wreck: a burnt-out shell whose boot you open (RZ_WreckBoots.c).
//!
//! A fixable car is data until it runs: which car it is, where it stands, what it's missing (two
//! or three of fuel, battery, tyre, spark plugs by default), whether its keys are in a house in
//! that town (rare) and whether someone hotwired it. Near players it stands as that car, parked and
//! locked (vanilla's lock component, so nobody can get in), with the interaction spot just in front
//! of its grille: inspect it ("Needs: fuel, a battery, a tyre"), fit parts (with a wrench or a
//! toolkit), fuel it, hotwire it (about 45 s, 15 s for a Mechanic, loud, and it can fail) and
//! start it. With UI > Car job layout set, "Look over the car" opens the repair screen instead
//! (docs/UI.md, "The car repair screen"): a job card beside a side view of the car, each job
//! lit where it is, done through the same checks (DoJob); without it, the actions as before. On the first start the parked copy is swapped for the drivable
//! car, and from then on it's an ordinary vehicle: vanilla repair handles damage, vanilla
//! persistence (if the server runs it) saves it. A new project turns up somewhere else a few
//! hours later. Wrecks are only ever wrecks: they're never fixable.
//!
//! A mission car (Car Keys) is a project that already runs: it needs only its keys, which lie in
//! one house the mission names (loose loot, RZ_Loot), or hotwiring. It isn't saved, doesn't trigger a replacement,
//! and goes when the mission ends unless someone started it.
//!
//! Parts come from garages (loose loot), wrecks' boots (sometimes one), a Mechanic wanderer for
//! Favor (never a quartermaster: they refuse to sell parts). Fuel comes from a petrol station's
//! hand pump into a container, or a full can found as loot; never from a wreck.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Interactions > Cars.
[BaseContainerProps()]
class RZ_CarSettings
{
	[Attribute("1", desc: "Fixable cars, and wrecks whose boot you open")]
	bool m_bEnabled;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Drivable vehicles a fixed car becomes (one at random). Empty = no fixable cars (wrecks still work)", "et")]
	ref array<ResourceName> m_aCarPrefabs;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "The same cars parked and locked, in the same order as Car prefabs: what stands there until it's fixed, with the interaction spot at its front. A missing entry shows only the interaction spot", "et")]
	ref array<ResourceName> m_aParkedPrefabs;

	[Attribute("0.34", desc: "Fixable cars per settlement on the map (about 1 per 3), at most one per town")]
	float m_fPerSettlement;

	[Attribute("6", desc: "Most fixable cars on the server at once")]
	int m_iMaxCars;

	[Attribute("6", desc: "Real hours before a fixed car's project is replaced somewhere else")]
	float m_fReplaceHours;

	[Attribute("0.25", desc: "Fuel in a freshly fixed car (0..1 of its tank)")]
	float m_fStartFuel;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Car battery item (garages, wreck boots, the Mechanic)", "et")]
	ResourceName m_sBatteryPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Tyre item", "et")]
	ResourceName m_sTyrePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Spark plugs item", "et")]
	ResourceName m_sSparkPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "A full fuel can (the hand pump). Empty = no pumping", "et")]
	ResourceName m_sFuelPrefab;

	[Attribute("battery,accumulator", desc: "Car battery keywords")]
	string m_sBatteryWords;

	[Attribute("tyre,tire,wheel", desc: "Tyre keywords")]
	string m_sTyreWords;

	[Attribute("spark", desc: "Spark plug keywords")]
	string m_sSparkWords;

	[Attribute("jerrycan,jerry_can,fuelcan,fuel_can,canister", desc: "Fuel keywords (one is used up fuelling a car)")]
	string m_sFuelWords;

	[Attribute("jerrycan_empty,empty_jerrycan,canister_empty,empty_canister,fuelcan_empty", desc: "Empty fuel container keywords (filled at the hand pump)")]
	string m_sContainerWords;

	[Attribute("wreck,carwreck,car_wreck,burntcar,burnt_car,car_burnt", desc: "Wreck keywords (their boot opens: Wreck boot prefab)")]
	string m_sWreckWords;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Wreck boot: an invisible storage (a small interaction box, no mesh) with RZ_WreckBootStorageComponent, SCR_InventoryStorageManagerComponent and vanilla's SCR_OpenStorageAction (Open the boot). Stands at a wreck's rear near players. Empty = wrecks hold nothing", "et")]
	ResourceName m_sBootPrefab;

	[Attribute("0.3", desc: "Chance a wreck's boot holds a car part (a battery, a tyre or spark plugs) when it's filled")]
	float m_fWreckPartChance;

	[Attribute("2", desc: "Fewest parts a new fixable car is missing (1 to 4: fuel, battery, tyre, spark plugs)")]
	int m_iMinParts;

	[Attribute("3", desc: "Most parts a new fixable car is missing (at least Min parts, at most 4)")]
	int m_iMaxParts;

	[Attribute("wrench,spanner,toolkit,tool_kit,toolbox,tool_box,repairkit,repair_kit", desc: "Tool keywords: one of these must be carried to fit a battery, tyre or spark plugs (kept, not used up; fuel needs none). Empty = no tool needed")]
	string m_sToolWords;

	[Attribute("45", desc: "Seconds to hotwire")]
	float m_fHotwireSeconds;

	[Attribute("15", desc: "Seconds to hotwire for a Mechanic")]
	float m_fMechanicHotwireSeconds;

	[Attribute("20", desc: "Heat hotwiring makes (sparks, the starter): half when it starts, all of it at the end, whether it worked or not")]
	float m_fHotwireHeat;

	[Attribute("0.35", desc: "Chance a hotwiring fails at the end (the heat is made anyway; try again)")]
	float m_fHotwireFailChance;

	[Attribute("0.1", desc: "Chance a Mechanic's hotwiring fails")]
	float m_fMechanicHotwireFailChance;

	[Attribute("1", desc: "Wire cutting: on the repair screen, Hotwire opens the loom under the column: four wires in a tangle, and you cut the one that runs to the ignition (the timers and fail chances above are for the car's prompts, where the screen isn't used). Off = the timed hotwire on the screen too")]
	bool m_bWireCutting;

	[Attribute("2", desc: "Wire cutting: wrong cuts before the loom shorts and the attempt fails with all of Hotwire heat")]
	int m_iLoomStrikes;

	[Attribute("0.5", desc: "Wire cutting: share of Hotwire heat a wrong cut makes")]
	float m_fWrongCutHeatShare;

	[Attribute("3", desc: "Wire cutting: turns in the tangle, 1 to 4. Each turn shuffles the four wires once: more turns are harder to follow")]
	int m_iLoomTurns;

	[Attribute("15", desc: "Heat an engine starting makes")]
	float m_fStartHeat;

	[Attribute("0.2", desc: "Chance a fixable car's keys are in a house in its town")]
	float m_fKeysChance;

	[Attribute("0.1", desc: "Chance each building rolled in that town (loose loot) holds them")]
	float m_fKeyFindChance;

	[Attribute("8", desc: "Seconds to fit parts or fuel")]
	float m_fFitSeconds;

	[Attribute("30", desc: "Seconds to fill a can at a petrol station's hand pump")]
	float m_fPumpSeconds;

	[Attribute("10", desc: "Heat the hand pump makes (a vanilla fuel pump station: per 20 litres drawn, at most twice that)")]
	float m_fPumpHeat;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Fuel pump station: an invisible prop with vanilla's fuel point (SCR_FuelManagerComponent with one fuel node, vanilla's fuel support station from FuelSupportStation_FuelPump.ct). Stands at a petrol station's pump near players, so vanilla's jerrycan fills there (Refuel canister). Empty = the hand pump (Work the hand pump, in the object menu)", "et")]
	ResourceName m_sPumpStationPrefab;

	[Attribute("60", desc: "Litres a petrol station's pump holds when full (a vanilla fuel pump station); it's full again Pump cooldown minutes after it was emptied")]
	float m_fPumpLitres;

	[Attribute("5", desc: "Litres a fixable car's Fuel job takes from a vanilla jerrycan (the can is kept; a fresh vanilla can holds 5); an old-style full can without a fuel tank is used up")]
	float m_fCarFuelLitres;

	[Attribute("20", desc: "Real minutes before the same pump gives fuel again")]
	float m_fPumpCooldownMinutes;

	[Attribute("120", desc: "Real minutes before an emptied wreck boot has things again (leftovers stay, and roll fresh after three times this)")]
	float m_fWreckReturnMinutes;

	[Attribute("12", desc: "Haven Favor for a part from a Mechanic wanderer")]
	int m_iPartCost;
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
	int car = -1;		// index into Car prefabs; -1 in saves from before it was kept
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
	bool m_bKeysPlaced;		// they lie in a building's loose loot (RZ_Loot), not taken yet
	bool m_bHotwired;
	string m_sNoun;			// "the blue hatchback"
	int m_iMissionId = -1;	// a mission's car (Car Keys): not saved, not replaced
	vector m_vKeyHouse;		// its keys are in the house here (mission cars)
	bool m_bWired;			// started by hotwiring, not with its keys
	ref RZ_CarLoom m_Loom;	// the wires under its column, once someone has pulled the panel (not saved)
	int m_iCar = -1;		// index into Car prefabs (and Parked prefabs)
	bool m_bRoom;			// its spot was checked for room where it's put out (RZ_CarRoom), this session
	bool m_bNoRoomSaid;		// the log has said it found no room

	IEntity m_Body;			// the interaction spot
	IEntity m_Car;			// the parked, locked car (none if Parked prefabs has no entry for it)
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
	protected static const float ROOM_REACH = 40;	// a car with no room is moved at most this far (m)
	protected static const int ROOM_TRIES = 12;
	protected static const float HIDE_RADIUS = 200;

	//! The read label on a car project's spot. The client hides the car's action slots behind it
	//! while it uses the repair screen (RZ_CarMenu.HidesSlotsOn).
	static const string LOOK_LABEL = "Look over the car";

	//! A hotwiring that failed: the timed one at its end, the loom when it shorts.
	protected static const string HOTWIRE_FAILED = "The wires spit, the starter grinds and grinds, and then nothing. Everything nearby heard that. Strip them back and try again.";

	//! The repair screen's jobs, in the order of the job card and the markers on the car.
	static const int JOB_SPARK = 0;
	static const int JOB_BATTERY = 1;
	static const int JOB_TYRE = 2;
	static const int JOB_FUEL = 3;
	static const int JOB_IGNITION = 4;
	static const int JOB_START = 5;		// not a row: the Start the engine button
	static const int JOBS = 5;
	static const int JOB_CUT = 10;		// not a row: cut the wire at plug JOB_CUT + n of the loom (0 to 3)

	//! A job's state on the screen.
	static const int STATE_DONE = 0;
	static const int STATE_CAN = 1;		// you can do it now
	static const int STATE_NEED = 2;	// you need something first
	static const int STATE_WORKING = 3;	// you're doing it

	//! What a job's tag says about the car, in words and a colour.
	static const int TONE_HAVE = 0;		// it's there: FITTED, FUELLED, HOTWIRED (olive, a tick)
	static const int TONE_NOW = 1;		// under way, or the key is on you (amber)
	static const int TONE_MISSING = 2;	// it isn't: MISSING, EMPTY, NO KEYS (red, a cross)

	protected RZ_GameModeComponent m_GM;
	protected RZ_Interactions m_Hub;
	protected RZ_CarSettings m_Settings;
	protected ref array<ref RZ_CarProject> m_aCars = {};
	protected ref map<string, float> m_mWreck = new map<string, float>();
	protected ref array<string> m_aWreckWords = {};
	protected int m_iNextId = 1;
	protected float m_fNextAt;
	protected bool m_bOn;
	protected ref map<int, IEntity> m_mStarted = new map<int, IEntity>();	// mission car id -> the vehicle it became
	protected ref map<int, bool> m_mStartedWired = new map<int, bool>();
	protected ref map<int, int> m_mScreenSpot = new map<int, int>();		// player -> the car spot their repair screen shows
	protected ref map<int, int> m_mScreenSerial = new map<int, int>();	// player -> that screen's serial
	protected int m_iNextSerial = 1;

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
			if (c.m_iMissionId >= 0)
				continue;

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
			cs.car = c.m_iCar;
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
		// Season rules (No Vehicles): no fixable cars this season
		m_bOn = m_Settings.m_bEnabled && m_Settings.m_aCarPrefabs && !m_Settings.m_aCarPrefabs.IsEmpty() && !RZ_Seasons.Mods().m_bNoVehicles;
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
				c.m_iCar = cs.car;
				EnsureCar(c); // a save from before the car was kept: pick one and rename it now, before anything shows its name
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
	//! The pumps' times (a boot's are RZ_WreckBoots'; stripped and siphoned wrecks in older saves
	//! are dropped).
	void RestoreWrecks(RZ_InteractionSave data)
	{
		for (int i = 0; i < data.wreckKey.Count() && i < data.wreckAt.Count(); i++)
		{
			string key = data.wreckKey[i];
			if (key.Length() > 5 && key.Substring(0, 5) == "pump:")
				m_mWreck.Set(key, data.wreckAt[i]);
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

			// With room to stand (RZ_CarRoom); asked again where it's first put out
			RZ_CarProject made = Generate(idx, pos);
			vector roomPos = made.m_vPos;
			float roomYaw = made.m_fYaw;
			if (RZ_CarRoom.Find(pos, ROOM_REACH, ROOM_TRIES, true, roomPos, roomYaw))
			{
				made.m_vPos = roomPos;
				made.m_fYaw = roomYaw;
			}

			m_aCars.Insert(made);
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
	//! Min parts to Max parts (two or three by default) of fuel, battery, tyre, spark plugs, all
	//! different. Saved projects keep what they were missing.
	protected RZ_CarProject Generate(int town, vector pos)
	{
		RZ_CarProject c = new RZ_CarProject();
		c.m_iId = m_iNextId;
		m_iNextId++;
		c.m_iTown = town;
		c.m_vPos = pos;
		c.m_fYaw = Math.RandomFloat(0, 360);

		int fewest = Math.Clamp(m_Settings.m_iMinParts, 1, 4);
		int most = Math.Clamp(m_Settings.m_iMaxParts, fewest, 4);
		int count = Math.RandomIntInclusive(fewest, most);
		array<int> parts = {PART_FUEL, PART_BATTERY, PART_TYRE, PART_SPARK};
		c.m_iMissing = 0;
		for (int k = 0; k < count && !parts.IsEmpty(); k++)
		{
			int pick = Math.RandomInt(0, parts.Count());
			c.m_iMissing = c.m_iMissing | parts[pick];
			parts.Remove(pick);
		}

		c.m_bHasKeys = Math.RandomFloat01() < m_Settings.m_fKeysChance;
		EnsureCar(c);
		return c;
	}

	//------------------------------------------------------------------------------------------------
	//! Picks the car a project becomes when it has none (a new one, or a save from before this was
	//! kept, or a Car prefabs list that got shorter), and names the project after it.
	protected void EnsureCar(RZ_CarProject c)
	{
		if (!m_Settings.m_aCarPrefabs || m_Settings.m_aCarPrefabs.IsEmpty())
		{
			c.m_iCar = -1;
			return;
		}

		if (c.m_iCar >= 0 && c.m_iCar < m_Settings.m_aCarPrefabs.Count())
			return;

		c.m_iCar = Math.RandomInt(0, m_Settings.m_aCarPrefabs.Count());
		c.m_sNoun = NounFor(m_Settings.m_aCarPrefabs[c.m_iCar]);
	}

	//------------------------------------------------------------------------------------------------
	//! The drivable car this project becomes. Empty without Car prefabs.
	protected ResourceName CarPrefab(RZ_CarProject c)
	{
		EnsureCar(c);
		if (c.m_iCar < 0)
			return "";

		return m_Settings.m_aCarPrefabs[c.m_iCar];
	}

	//------------------------------------------------------------------------------------------------
	//! The same car parked and locked (the same index in Parked prefabs). Empty if there's none.
	protected ResourceName ParkedPrefab(RZ_CarProject c)
	{
		EnsureCar(c);
		if (c.m_iCar < 0 || !m_Settings.m_aParkedPrefabs || c.m_iCar >= m_Settings.m_aParkedPrefabs.Count())
			return "";

		return m_Settings.m_aParkedPrefabs[c.m_iCar];
	}

	//------------------------------------------------------------------------------------------------
	//! "the beige saloon", from the car prefab's file name (S105, V2101, T601 and UAZ are known).
	static string NounFor(ResourceName prefab)
	{
		string fileName = prefab;
		fileName.ToLower();
		int slash = fileName.LastIndexOf("/");
		if (slash >= 0)
			fileName = fileName.Substring(slash + 1, fileName.Length() - slash - 1);

		string kind = "car";
		if (fileName.Contains("s105") || fileName.Contains("v2101"))
			kind = "saloon";
		else if (fileName.Contains("t601"))
			kind = "two-door";
		else if (fileName.Contains("uaz"))
			kind = "jeep";

		array<string> keys = {"darkblue", "lightblue", "beige", "white", "red", "green", "blue", "grey", "gray", "yellow", "black", "brown", "orange"};
		array<string> words = {"dark blue", "light blue", "beige", "white", "red", "green", "blue", "grey", "grey", "yellow", "black", "brown", "orange"};
		foreach (int i, string key : keys)
		{
			if (fileName.Contains(key))
				return string.Format("the %1 %2", words[i], kind);
		}

		return "the " + kind;
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
	//! The car as it will be, parked and locked (nobody can get in), with the interaction spot that
	//! carries its actions just in front of its grille. Without a parked prefab, only the spot, on the ground.
	protected void Show(RZ_CarProject c)
	{
		if (!MakeRoom(c))
			return;

		EnsureCar(c); // before the spot takes its name
		c.m_iSpot = m_Hub.AddSpot(RZ_Interactions.KIND_CAR, c.m_vPos, "car", c.m_sNoun, c.m_iId, -1);
		if (c.m_iSpot < 0)
			return;

		vector spotPos = c.m_vPos;
		ResourceName parked = ParkedPrefab(c);
		if (!parked.IsEmpty())
		{
			c.m_Car = RZ_Util.SpawnPrefab(parked, c.m_vPos + Vector(0, 0.3, 0), c.m_fYaw);
			if (c.m_Car)
				spotPos = RZ_Util.FrontOf(c.m_Car, 0.8, -0.35); // just in front of the grille, where nothing of the car blocks a look at it
		}

		c.m_Body = RZ_Util.SpawnPrefab(m_Hub.GetSpotPrefab(), spotPos, c.m_fYaw);
		RZ_Spot s = m_Hub.FindSpot(c.m_iSpot);
		if (!c.m_Body || !s)
		{
			Hide(c);
			return;
		}

		m_Hub.AttachLabels(s, c.m_Body);
	}

	//------------------------------------------------------------------------------------------------
	//! The first time a car is put out (a player is near, so what stands around it is there): it
	//! has room where it is, or it's moved to the nearest spot that has (a saved car in a wall too).
	//! False while no spot was found: it stays data and is asked again on the next tick.
	protected bool MakeRoom(RZ_CarProject c)
	{
		if (c.m_bRoom)
			return true;

		vector was = c.m_vPos;
		vector roomPos = c.m_vPos;
		float roomYaw = c.m_fYaw;
		if (!RZ_CarRoom.Find(was, ROOM_REACH, ROOM_TRIES, true, roomPos, roomYaw))
		{
			if (!c.m_bNoRoomSaid)
				RZ_Log.Info(string.Format("Cars: no room for %1 at %2 yet; it stays put away.", c.m_sNoun, RZ_SettlementScanner.GridReference(was)));

			c.m_bNoRoomSaid = true;
			return false;
		}

		c.m_bRoom = true;
		c.m_vPos = roomPos;
		c.m_fYaw = roomYaw;
		float moved = vector.DistanceXZ(was, c.m_vPos);
		if (moved > 1)
			RZ_Log.Info(string.Format("Cars: %1 had no room at %2 (a wall, a roof or a step); moved %3 m.", c.m_sNoun, RZ_SettlementScanner.GridReference(was), RZ_Util.RoundToInt(moved)));

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Hide(RZ_CarProject c)
	{
		if (c.m_Body)
			RZ_Util.DeleteUnit(c.m_Body);

		if (c.m_Car)
			RZ_Util.DeleteUnit(c.m_Car);

		c.m_Body = null;
		c.m_Car = null;
		RZ_Spot s = m_Hub.FindSpot(c.m_iSpot);
		if (s)
			s.m_Body = null;

		CloseScreensOn(c.m_iSpot, "gone");
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

	//------------------------------------------------------------------------------------------------
	//! What a car key's fob says (a key item's tag).
	protected string CarKeyTag(RZ_CarProject c)
	{
		return string.Format("Car keys: %1, %2", c.m_sNoun, TownName(c));
	}

	// ================================================================ The project
	//------------------------------------------------------------------------------------------------
	void ProjectLabels(RZ_Spot s, out string read, out string choiceLabels)
	{
		read = LOOK_LABEL;
		choiceLabels = "Fit parts|Fuel it|Hotwire it (or use its keys)|Start the engine";
	}

	//------------------------------------------------------------------------------------------------
	//! "Look over the car": the repair screen where it's used, else "Needs: fuel, a battery." as a
	//! notice. The first look gets the "car" tip.
	void Inspect(int playerId, RZ_Spot s, IEntity user)
	{
		RZ_CarProject c = Find(s.m_iRef);
		if (!c)
			return;

		string text = InspectText(playerId, s, c, user);
		if (!OpenScreen(playerId, s, ScreenText(c, user)))
			m_GM.NotifyPlayer(playerId, m_Hub.Title(s), text);

		if (!c.m_aInspected.Contains(playerId))
		{
			c.m_aInspected.Insert(playerId);
			RZ_Guidance.Report(playerId, "car", TownName(c));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! What a look over the car says: what's missing, the tool, the keys.
	protected string InspectText(int playerId, RZ_Spot s, RZ_CarProject c, IEntity user)
	{
		string needs = "Nothing. It would run.";
		if (c.m_iMissing != 0)
			needs = string.Format("Needs: %1.", PartsText(c.m_iMissing));

		if ((c.m_iMissing & PARTS_NOT_FUEL) != 0 && NeedsTool(user))
			needs += " Nothing comes off or goes on without a wrench or a toolkit.";

		string keys = "No keys in it. You could hotwire it.";
		if (c.m_bHotwired)
			keys = "The wires under the dash are already stripped and twisted together.";
		else if (m_Hub.GetSearch().HasKey(playerId, CarKey(c)))
			keys = "Its keys are in your pocket.";
		else if (c.m_bHasKeys)
			keys = "No keys in it. A spare's often in a house nearby; or hotwire it.";

		return string.Format("%1, under a layer of dust. Nothing wrong you can't fix. %2 %3", m_Hub.Title(s), needs, keys);
	}

	//------------------------------------------------------------------------------------------------
	//! What the repair screen opens on: short, since its rows and the line in its header say what's
	//! there and what isn't. Only the tool is worth saying.
	protected string ScreenText(RZ_CarProject c, IEntity user)
	{
		if ((c.m_iMissing & PARTS_NOT_FUEL) != 0 && NeedsTool(user))
			return "Under a layer of dust. Nothing comes off or goes on without a wrench or a toolkit, and you've neither.";

		return "Under a layer of dust. Nothing wrong you can't fix.";
	}

	//------------------------------------------------------------------------------------------------
	//! 0 fit parts, 1 fuel it, 2 hotwire (or keys), 3 start.
	void HandleProject(int playerId, RZ_Spot s, int option, IEntity user)
	{
		RZ_CarProject c = Find(s.m_iRef);
		if (!c)
			return;

		string title = m_Hub.Title(s);
		if (option == 0)
		{
			Fit(playerId, s, c, PARTS_NOT_FUEL, user);
			return;
		}

		if (option == 1)
		{
			Fit(playerId, s, c, PART_FUEL, user);
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
	//! Starts fitting what the car is missing of `wanted` (PART_* bits) that the user carries: a
	//! timed task, with a tool for anything but fuel.
	protected void Fit(int playerId, RZ_Spot s, RZ_CarProject c, int wanted, IEntity user)
	{
		string title = m_Hub.Title(s);
		int mask = c.m_iMissing & wanted;
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

		if ((mask & PARTS_NOT_FUEL) != 0 && NeedsTool(user))
		{
			m_GM.NotifyPlayer(playerId, title, "The bolts are rusted solid. You can't do this with your hands: you need a wrench or a toolkit.");
			return;
		}

		m_Hub.StartTask(playerId, user, RZ_Interactions.TASK_FIT, s.m_iId, mask, m_Settings.m_fFitSeconds, "the work on the car", false);
	}

	//------------------------------------------------------------------------------------------------
	//! Fitting a battery, tyre or spark plugs needs a tool (Tool keywords) the user doesn't carry.
	//! False when Tool keywords is empty.
	protected bool NeedsTool(IEntity user)
	{
		array<string> toolWords = {};
		RZ_ItemHelper.Words(m_Settings.m_sToolWords, toolWords);
		if (toolWords.IsEmpty())
			return false;

		return !RZ_ItemHelper.Has(user, m_Settings.m_sToolWords);
	}

	//------------------------------------------------------------------------------------------------
	protected int PartsCarried(IEntity user, int mask)
	{
		int found = 0;
		if ((mask & PART_FUEL) != 0 && RZ_FuelPumps.FindFuel(user, m_Settings.m_sFuelWords + RZ_ItemHelper.NOT_EMPTY, m_Settings.m_fCarFuelLitres))
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
	//! A fit finished: on the repair screen if it's open, else as notices.
	void CompleteFit(int playerId, RZ_Spot s, int mask, IEntity user)
	{
		bool captured = CaptureFor(playerId);
		FinishFit(playerId, s, mask, user);
		Release(playerId, captured);
	}

	//------------------------------------------------------------------------------------------------
	protected void FinishFit(int playerId, RZ_Spot s, int mask, IEntity user)
	{
		if (!s)
			return;

		RZ_CarProject c = Find(s.m_iRef);
		if (!c)
			return;

		// The tool went while they worked
		if ((mask & PARTS_NOT_FUEL) != 0 && NeedsTool(user))
		{
			m_GM.NotifyPlayer(playerId, m_Hub.Title(s), "You reach for the wrench and it isn't there. Nothing gets fitted with bare hands.");
			return;
		}

		int fitted = 0;
		IEntity can = RZ_FuelPumps.FindFuel(user, m_Settings.m_sFuelWords + RZ_ItemHelper.NOT_EMPTY, m_Settings.m_fCarFuelLitres);
		if ((mask & PART_FUEL) != 0 && RZ_FuelPumps.UseFuel(user, can, m_Settings.m_fCarFuelLitres))
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
	//! The hotwiring finished: on the repair screen if it's open, else as notices.
	void CompleteHotwire(int playerId, RZ_Spot s, int carId)
	{
		bool captured = CaptureFor(playerId);
		FinishHotwire(playerId, s, carId);
		Release(playerId, captured);
	}

	//------------------------------------------------------------------------------------------------
	protected void FinishHotwire(int playerId, RZ_Spot s, int carId)
	{
		RZ_CarProject c = Find(carId);
		if (!c)
			return;

		m_GM.GetHeat().AddHeat(c.m_vPos, m_Settings.m_fHotwireHeat);
		float failChance = m_Settings.m_fHotwireFailChance;
		if (RZ_Interactions.IsBackground(m_GM, playerId, "mechanic"))
			failChance = m_Settings.m_fMechanicHotwireFailChance;

		if (Math.RandomFloat01() < failChance)
		{
			m_GM.NotifyPlayer(playerId, c.m_sNoun, HOTWIRE_FAILED);
			return;
		}

		HotwireWorked(playerId, c);
	}

	//------------------------------------------------------------------------------------------------
	protected void HotwireWorked(int playerId, RZ_CarProject c)
	{
		c.m_bHotwired = true;
		c.m_bWired = true;
		c.m_Loom = null;
		m_GM.NotifyPlayer(playerId, c.m_sNoun, "A spark, a cough from the starter. The wires are live.");
	}

	// ================================================================ The loom (the repair screen's hotwire)
	//------------------------------------------------------------------------------------------------
	//! The screen's Ignition job. With no key and Wire cutting on it pulls the panel under the
	//! column: the car's loom is rolled the first time (RZ_CarLoom) and the screen shows it. With
	//! the key, an ignition already sorted, or Wire cutting off, it's the car's own Hotwire prompt.
	protected void Ignition(int playerId, RZ_Spot s, RZ_CarProject c, IEntity user)
	{
		if (!m_Settings.m_bWireCutting || c.m_bHotwired || m_Hub.GetSearch().HasKey(playerId, CarKey(c)))
		{
			HandleProject(playerId, s, 2, user);
			return;
		}

		if (!c.m_Loom)
			c.m_Loom = RZ_CarLoom.Make(m_Settings.m_iLoomTurns);

		m_GM.NotifyPlayer(playerId, m_Hub.Title(s), "The panel's off.");
	}

	//------------------------------------------------------------------------------------------------
	//! A cut on the screen's loom (the server's word on it: RZ_CarLoom.Cut). The wire that runs to
	//! the ignition and the car is hotwired. Any other sparks: a share of Hotwire heat, a strike, and
	//! that plug is dead. At Loom strikes the loom shorts: the attempt fails as a hotwiring always
	//! could, and the next one starts on a fresh tangle.
	protected void CutWire(int playerId, RZ_Spot s, RZ_CarProject c, int plug)
	{
		if (!c.m_Loom || c.m_bHotwired || !m_Settings.m_bWireCutting)
			return;

		float heat = m_Settings.m_fHotwireHeat;
		int result = c.m_Loom.Cut(plug, RZ_Interactions.IsBackground(m_GM, playerId, "mechanic"));
		if (result == RZ_CarLoom.RESULT_NOTHING)
			return;

		if (result == RZ_CarLoom.RESULT_DONE)
		{
			m_GM.GetHeat().AddHeat(c.m_vPos, heat);
			HotwireWorked(playerId, c);
			return;
		}

		if (c.m_Loom.m_iStrikes >= Math.Max(1, m_Settings.m_iLoomStrikes))
		{
			ShortLoom(playerId, c);
			return;
		}

		m_GM.GetHeat().AddHeat(c.m_vPos, heat * Math.Clamp(m_Settings.m_fWrongCutHeatShare, 0, 1));
		m_GM.NotifyPlayer(playerId, m_Hub.Title(s), "A spark. Wrong wire.");
	}

	//------------------------------------------------------------------------------------------------
	//! The loom shorts: all of Hotwire heat, the failed hotwiring's line, and the car gets a fresh
	//! loom the next time someone pulls the panel.
	protected void ShortLoom(int playerId, RZ_CarProject c)
	{
		c.m_Loom = null;
		m_GM.GetHeat().AddHeat(c.m_vPos, m_Settings.m_fHotwireHeat);
		m_GM.NotifyPlayer(playerId, c.m_sNoun, HOTWIRE_FAILED);
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

		ResourceName prefab = CarPrefab(c);
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
		if (c.m_iMissionId >= 0)
		{
			m_mStarted.Set(c.m_iId, car);
			m_mStartedWired.Set(c.m_iId, c.m_bWired);
		}
		else if (m_fNextAt <= 0)
			m_fNextAt = m_Hub.GetClock() + m_Settings.m_fReplaceHours * 3600;

		m_GM.GetHeat().AddHeat(pos, m_Settings.m_fStartHeat);
		m_GM.NotifyPlayer(playerId, title, "It catches on the second try and settles into a rough idle. The noise is enormous. It's yours, for as long as you can keep it.");
		RZ_Log.Info(logLine);
	}

	// ================================================================ The repair screen
	//------------------------------------------------------------------------------------------------
	//! Opens the repair screen on this car for the player (docs/UI.md, "The car repair screen").
	//! False when it isn't used (no job layout, no UI component): the caller shows the notice. At
	//! the server's cap of open screens the client is told to use the actions for a minute.
	protected bool OpenScreen(int playerId, RZ_Spot s, string inspectText)
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		RZ_ScreenGuard guard = m_GM.GetScreenGuard();
		if (!ui || !guard || RZ_UISettings.Get().m_sCarJobLayout.IsEmpty())
			return false;

		if (!guard.Open(playerId, RZ_ScreenGuard.KIND_CAR, s.m_Body, m_Hub.Title(s), -1))
		{
			ui.SendPayload(RZ_UIFeed.PAYLOAD_CAR, "F|busy");
			return false;
		}

		m_mScreenSpot.Set(playerId, s.m_iId);
		m_mScreenSerial.Set(playerId, m_iNextSerial);
		m_iNextSerial++;
		SendScreen(playerId, inspectText);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The player has the repair screen open (on any car).
	bool HasScreen(int playerId)
	{
		return m_mScreenSpot.Contains(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! The screen guard or the player closed it: forget it.
	void OnScreenGone(int playerId)
	{
		m_mScreenSpot.Remove(playerId);
		m_mScreenSerial.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Closes the player's screen from here (the car started or went): the client and the guard.
	protected void CloseScreen(int playerId, string reason)
	{
		if (!HasScreen(playerId))
			return;

		OnScreenGone(playerId);
		RZ_ScreenGuard guard = m_GM.GetScreenGuard();
		if (guard)
			guard.Close(playerId);

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		if (ui)
			ui.SendPayload(RZ_UIFeed.PAYLOAD_CAR, "X|" + reason);
	}

	//------------------------------------------------------------------------------------------------
	protected void CloseScreensOn(int spotId, string reason)
	{
		if (spotId < 0 || m_mScreenSpot.IsEmpty())
			return;

		array<int> players = {};
		foreach (int playerId, int shown : m_mScreenSpot)
		{
			if (shown == spotId)
				players.Insert(playerId);
		}

		foreach (int p : players)
		{
			CloseScreen(p, reason);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Notices for this player go into their open screen until Release.
	protected bool CaptureFor(int playerId)
	{
		if (!HasScreen(playerId))
			return false;

		RZ_ScreenCapture.Begin(playerId);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Stops catching and redraws the screen with what was said; a card if the screen closed meanwhile.
	protected void Release(int playerId, bool captured)
	{
		if (!captured)
			return;

		string said = RZ_ScreenCapture.End();
		if (HasScreen(playerId))
		{
			SendScreen(playerId, said);
			return;
		}

		if (!said.IsEmpty())
			m_GM.NotifyPlayer(playerId, RZ_ScreenCapture.LastTitle(), said);
	}

	//------------------------------------------------------------------------------------------------
	//! A fit or hotwiring stopped early (the player moved or was hurt): the screen shows it's over.
	void OnTaskStopped(int playerId)
	{
		if (HasScreen(playerId))
			SendScreen(playerId, "");
	}

	//------------------------------------------------------------------------------------------------
	//! A pick on the repair screen (RZ_PlayerUIComponent.RpcAsk_CarJob). Checked here: the screen is
	//! open on this serial, the car is still there and the player beside it, nothing else is under
	//! way; then the job runs through the same code as the car's actions, and the screen redraws
	//! with what they would have said.
	void DoJob(int playerId, int serial, int job)
	{
		int spotId;
		int current;
		if (!m_mScreenSpot.Find(playerId, spotId) || !m_mScreenSerial.Find(playerId, current) || serial != current)
			return;

		RZ_ScreenGuard guard = m_GM.GetScreenGuard();
		if (!guard || !guard.IsOpen(playerId, RZ_ScreenGuard.KIND_CAR))
		{
			OnScreenGone(playerId);
			return;
		}

		guard.Touch(playerId);
		RZ_Spot s = m_Hub.FindSpot(spotId);
		RZ_CarProject c;
		if (s)
			c = Find(s.m_iRef);

		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!c || !user || !RZ_Util.IsCharacterAlive(user))
		{
			CloseScreen(playerId, "gone");
			return;
		}

		if (RZ_Util.DistXZ(user.GetOrigin(), s.m_vPos) > RZ_DialogueSettings.Get().m_fRange)
			return;

		if (m_Hub.HasTask(playerId))
		{
			SendScreen(playerId, "You're already at work on something. Finish it first, or step away to stop.");
			return;
		}

		RZ_ScreenCapture.Begin(playerId);
		switch (job)
		{
			case JOB_SPARK: Fit(playerId, s, c, PART_SPARK, user); break;
			case JOB_BATTERY: Fit(playerId, s, c, PART_BATTERY, user); break;
			case JOB_TYRE: Fit(playerId, s, c, PART_TYRE, user); break;
			case JOB_FUEL: Fit(playerId, s, c, PART_FUEL, user); break;
			case JOB_IGNITION: Ignition(playerId, s, c, user); break;
			case JOB_START: HandleProject(playerId, s, 3, user); break;
		}

		if (job >= JOB_CUT && job < JOB_CUT + RZ_CarLoom.PLUGS)
			CutWire(playerId, s, c, job - JOB_CUT);

		Release(playerId, true);
	}

	//------------------------------------------------------------------------------------------------
	//! The screen's payload: C|serial|title|done|jobs|status|can start|start need|what's missing|1 if
	//! anything is, I|inspect text, W|job|seconds left|seconds in all while a job is under way, then
	//! J|job|state|name|need|button|tag|tone per job, and the loom's records while the panel under
	//! the column is off (RZ_CarLoom.Records).
	protected void SendScreen(int playerId, string status)
	{
		int spotId;
		int serial;
		if (!m_mScreenSpot.Find(playerId, spotId) || !m_mScreenSerial.Find(playerId, serial))
			return;

		RZ_Spot s = m_Hub.FindSpot(spotId);
		RZ_CarProject c;
		if (s)
			c = Find(s.m_iRef);

		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		if (!c || !user || !ui)
		{
			CloseScreen(playerId, "gone");
			return;
		}

		// The job under way on this car, if any
		int working = -1;
		RZ_TimedTask task = m_Hub.GetTask(playerId);
		float seconds = 0;
		if (task && task.m_iSpot == spotId)
		{
			working = TaskJob(task);
			seconds = m_Settings.m_fFitSeconds;
			if (task.m_iKind == RZ_Interactions.TASK_HOTWIRE)
				seconds = HotwireSeconds(playerId);
		}

		string rows = "";
		int done = 0;
		for (int job = 0; job < JOBS; job++)
		{
			int state;
			string name;
			string need;
			string button;
			string tag;
			int tone;
			JobRow(playerId, c, job, user, working, state, name, need, button, tag, tone);
			if (state == STATE_DONE)
				done++;

			rows += string.Format("\nJ|%1|%2|%3|%4|%5|%6|%7", job, state, name, RZ_UIText.Clean(need, 80), button, tag, tone);
		}

		if (c.m_Loom && !c.m_bHotwired && m_Settings.m_bWireCutting)
			rows += c.m_Loom.Records(Math.Max(1, m_Settings.m_iLoomStrikes), RZ_Interactions.IsBackground(m_GM, playerId, "mechanic"));

		bool hasKey = m_Hub.GetSearch().HasKey(playerId, CarKey(c));
		bool canStart = c.m_iMissing == 0 && (c.m_bHotwired || hasKey) && working < 0;
		string startNeed = "";
		if (c.m_iMissing != 0)
			startNeed = "Every part first";
		else if (!c.m_bHotwired && !hasKey)
			startNeed = "Keys or the wires first";

		int startFlag = 0;
		if (canStart)
			startFlag = 1;

		string lacking = MissingLine(c, hasKey);
		int lacks = 1;
		if (lacking.IsEmpty())
		{
			lacking = "Nothing missing";
			lacks = 0;
		}

		string payload = string.Format("C|%1|%2|%3|%4|%5|%6|%7|%8|%9", serial, RZ_UIText.Clean(RZ_Util.Capitalize(c.m_sNoun), 60), done, JOBS,
			RZ_UIText.Clean(status, 400), startFlag, startNeed, lacking, lacks);
		payload += "\nI|" + RZ_UIText.Clean(InspectText(playerId, s, c, user), 400);
		if (working >= 0)
			payload += string.Format("\nW|%1|%2|%3", working, Math.Max(0, task.m_fLeft), seconds);

		ui.SendPayload(RZ_UIFeed.PAYLOAD_CAR, payload + rows);
	}

	//------------------------------------------------------------------------------------------------
	//! The job a car task is doing (a fit's part, or the ignition), -1 for any other task.
	protected int TaskJob(RZ_TimedTask t)
	{
		if (t.m_iKind == RZ_Interactions.TASK_HOTWIRE)
			return JOB_IGNITION;

		if (t.m_iKind != RZ_Interactions.TASK_FIT)
			return -1;

		if ((t.m_iArg & PART_FUEL) != 0)
			return JOB_FUEL;
		if ((t.m_iArg & PART_BATTERY) != 0)
			return JOB_BATTERY;
		if ((t.m_iArg & PART_TYRE) != 0)
			return JOB_TYRE;

		return JOB_SPARK;
	}

	//------------------------------------------------------------------------------------------------
	protected float HotwireSeconds(int playerId)
	{
		if (RZ_Interactions.IsBackground(m_GM, playerId, "mechanic"))
			return m_Settings.m_fMechanicHotwireSeconds;

		return m_Settings.m_fHotwireSeconds;
	}

	//------------------------------------------------------------------------------------------------
	//! The header's line: what the car still lacks, by name ("Missing: tyre, fuel, keys"), "" when
	//! it lacks nothing.
	protected string MissingLine(RZ_CarProject c, bool hasKey)
	{
		array<string> names = {};
		if ((c.m_iMissing & PART_SPARK) != 0)
			names.Insert("spark plugs");
		if ((c.m_iMissing & PART_BATTERY) != 0)
			names.Insert("battery");
		if ((c.m_iMissing & PART_TYRE) != 0)
			names.Insert("tyre");
		if ((c.m_iMissing & PART_FUEL) != 0)
			names.Insert("fuel");
		if (!c.m_bHotwired && !hasKey)
			names.Insert("keys");

		if (names.IsEmpty())
			return "";

		string line = "Missing: ";
		foreach (int i, string n : names)
		{
			if (i > 0)
				line += ", ";

			line += n;
		}

		return line;
	}

	//------------------------------------------------------------------------------------------------
	//! One row of the job card: its state, name, what you need (or have), the button's word, and
	//! the tag that says what the car has: its words (FITTED, MISSING...) and tone (TONE_*).
	protected void JobRow(int playerId, RZ_CarProject c, int job, IEntity user, int working, out int state, out string name, out string need, out string button, out string tag, out int tone)
	{
		button = "";
		if (job == JOB_IGNITION)
		{
			name = "Ignition";
			tag = "NO KEYS";
			tone = TONE_MISSING;
			if (working == job)
			{
				state = STATE_WORKING;
				need = "Hotwiring";
				tag = "HOTWIRING";
				tone = TONE_NOW;
			}
			else if (c.m_bHotwired && c.m_bWired)
			{
				state = STATE_DONE;
				need = "The wires are live";
				tag = "HOTWIRED";
				tone = TONE_HAVE;
			}
			else if (c.m_bHotwired)
			{
				state = STATE_DONE;
				need = "The key is in it";
				tag = "KEYS";
				tone = TONE_HAVE;
			}
			else if (m_Hub.GetSearch().HasKey(playerId, CarKey(c)))
			{
				state = STATE_CAN;
				need = "Your key fits";
				button = "Use the key";
				tag = "KEY ON YOU";
				tone = TONE_NOW;
			}
			else if (m_Settings.m_bWireCutting)
			{
				state = STATE_CAN;
				need = "One wire to cut";
				button = "Hotwire";
			}
			else
			{
				state = STATE_CAN;
				need = string.Format("About %1 s, loud, can fail", RZ_Util.RoundToInt(HotwireSeconds(playerId)));
				button = "Hotwire";
			}

			return;
		}

		int bit = PART_SPARK;
		name = "Spark plugs";
		string doing = "Fitting";
		string carried = "One in your bag";
		string none = "None on you";
		string done = "On the car";
		string have = "FITTED";
		string lack = "MISSING";
		button = "Fit";
		if (job == JOB_BATTERY)
		{
			bit = PART_BATTERY;
			name = "Battery";
		}
		else if (job == JOB_TYRE)
		{
			bit = PART_TYRE;
			name = "Tyre";
		}
		else if (job == JOB_FUEL)
		{
			bit = PART_FUEL;
			name = "Fuel";
			doing = "Pouring";
			carried = "A full can on you";
			none = "No full can on you";
			done = "In the tank";
			have = "FUELLED";
			lack = "EMPTY";
			button = "Pour";
		}

		tag = lack;
		tone = TONE_MISSING;
		if (working == job)
		{
			state = STATE_WORKING;
			need = doing;
			button = "";
			tag = "WORKING";
			tone = TONE_NOW;
			return;
		}

		if ((c.m_iMissing & bit) == 0)
		{
			state = STATE_DONE;
			need = done;
			button = "";
			tag = have;
			tone = TONE_HAVE;
			return;
		}

		if (PartsCarried(user, bit) == 0)
		{
			state = STATE_NEED;
			need = none;
			button = "";
			return;
		}

		if (bit != PART_FUEL && NeedsTool(user))
		{
			state = STATE_NEED;
			need = "Needs a wrench or a toolkit";
			button = "";
			return;
		}

		state = STATE_CAN;
		need = carried;
	}

	// ================================================================ Keys and the Mechanic
	//------------------------------------------------------------------------------------------------
	//! Loose loot (RZ_Loot): the mission car whose keys lie in the building at buildingPos (the one
	//! house the mission names), marked as placed. -1 if none.
	int ClaimMissionKeysAt(vector buildingPos)
	{
		if (!m_bOn)
			return -1;

		foreach (RZ_CarProject c : m_aCars)
		{
			if (c.m_iMissionId < 0 || c.m_bKeysFound || c.m_bKeysPlaced || RZ_Util.DistXZ(c.m_vKeyHouse, buildingPos) > 1)
				continue;

			c.m_bKeysPlaced = true;
			return c.m_iId;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! A mission car's keys wait for their house to be placed (RZ_Loot looks only then).
	bool HasUnplacedMissionKeys()
	{
		foreach (RZ_CarProject c : m_aCars)
		{
			if (c.m_iMissionId >= 0 && !c.m_bKeysFound && !c.m_bKeysPlaced)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Loose loot, when a building in a town is rolled: sometimes the keys of that town's fixable
	//! car (one that has keys in a house: Keys chance), each roll Key find chance. -1 if not.
	int ClaimTownKeysAt(vector buildingPos)
	{
		if (!m_bOn)
			return -1;

		int town = m_GM.FindNearestSettlement(buildingPos);
		foreach (RZ_CarProject c : m_aCars)
		{
			if (c.m_iMissionId >= 0 || c.m_iTown != town || !c.m_bHasKeys || c.m_bKeysFound || c.m_bKeysPlaced)
				continue;

			if (Math.RandomFloat01() >= m_Settings.m_fKeyFindChance)
				return -1;

			c.m_bKeysPlaced = true;
			return c.m_iId;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! The house that held its keys was rolled again or forgotten with the keys still there.
	void ReleaseKeys(int carId)
	{
		RZ_CarProject c = Find(carId);
		if (c)
			c.m_bKeysPlaced = false;
	}

	//------------------------------------------------------------------------------------------------
	//! Someone picked its keys up (loose loot): found.
	void OnKeysTaken(int carId)
	{
		RZ_CarProject c = Find(carId);
		if (!c)
			return;

		c.m_bKeysPlaced = false;
		c.m_bKeysFound = true;
	}

	//------------------------------------------------------------------------------------------------
	//! The key id and the fob's tag for a car's key item. False if the car is gone.
	bool KeyFor(int carId, out string keyId, out string tag)
	{
		RZ_CarProject c = Find(carId);
		if (!c)
			return false;

		keyId = CarKey(c);
		tag = CarKeyTag(c);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The Mechanic wanderer's "Ask about cars": the nearest fixable car, what it needs, and its
	//! keys if they like you and know where they are.
	string MechanicTalk(int playerId, vector from, bool friendly)
	{
		if (!m_bOn)
			return "Cars? Nothing out here runs any more. Believe me, I've looked.";

		RZ_CarProject best;
		float bestDist = float.MAX;
		foreach (RZ_CarProject c : m_aCars)
		{
			if (c.m_iMissionId >= 0)
				continue;

			float d = RZ_Util.DistXZ(c.m_vPos, from);
			if (d < bestDist)
			{
				bestDist = d;
				best = c;
			}
		}

		if (!best)
			return "Nothing I know of that runs. Someone beat you to the last one.";

		string text = string.Format("There's %1 in %2 (%3) that would run with %4. Most wrecks are just wrecks, mind. That one isn't.",
			best.m_sNoun, TownName(best), RZ_Util.DirectionText(from, best.m_vPos), PartsText(best.m_iMissing));
		if (friendly && best.m_bHasKeys && !best.m_bKeysFound)
		{
			best.m_bKeysFound = true;
			if (best.m_bKeysPlaced)
				m_Hub.GetLoot().DropCarKeys(best.m_iId); // the ones lying in a house were these

			best.m_bKeysPlaced = false;
			m_Hub.GetSearch().GiveKey(playerId, CarKey(best), CarKeyTag(best));
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
	//! One of the car parts at random (a wreck's boot), "" if none is set.
	ResourceName RandomPart()
	{
		array<ResourceName> parts = {};
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

	// ================================================================ Mission cars (Car Keys)
	//------------------------------------------------------------------------------------------------
	//! Fixable cars work: on, with car prefabs set.
	bool IsOn()
	{
		return m_bOn;
	}

	//------------------------------------------------------------------------------------------------
	//! A car that already runs, for a mission: only its keys (in the house at keyHouse) or
	//! hotwiring stand between it and the road. Returns its id, -1 if cars are off.
	int AddMissionCar(int town, vector pos, vector keyHouse, int missionId)
	{
		if (!m_bOn)
			return -1;

		RZ_CarProject c = Generate(town, pos);
		c.m_iMissing = 0;
		c.m_bHasKeys = false;
		c.m_iMissionId = missionId;
		c.m_vKeyHouse = keyHouse;
		m_aCars.Insert(c);
		return c.m_iId;
	}

	//------------------------------------------------------------------------------------------------
	//! "the blue hatchback". "" if it's gone.
	string GetNoun(int carId)
	{
		RZ_CarProject c = Find(carId);
		if (c)
			return c.m_sNoun;

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Someone has found its keys (picked them up in the right house, this session).
	bool AreKeysFound(int carId)
	{
		RZ_CarProject c = Find(carId);
		if (!c)
			return false;

		return c.m_bKeysFound;
	}

	//------------------------------------------------------------------------------------------------
	//! The vehicle a mission car became when someone started it, null if not started.
	IEntity GetStartedCar(int carId)
	{
		IEntity car;
		m_mStarted.Find(carId, car);
		return car;
	}

	//------------------------------------------------------------------------------------------------
	//! It was started by hotwiring rather than with its keys.
	bool WasHotwired(int carId)
	{
		bool wired;
		m_mStartedWired.Find(carId, wired);
		return wired;
	}

	//------------------------------------------------------------------------------------------------
	//! The mission is over: an unstarted car goes (prop and data); a started one is just a car now.
	void RemoveMissionCar(int carId)
	{
		RZ_CarProject c = Find(carId);
		if (c)
		{
			Hide(c);
			m_aCars.RemoveItem(c);
		}

		// Its keys, if they still lie in the house, go with it
		m_Hub.GetLoot().DropCarKeys(carId);

		m_mStarted.Remove(carId);
		m_mStartedWired.Remove(carId);
	}

	// ================================================================ The hand pump
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
