//! Searching the world (docs/WORLD.md, "World interactions"): cupboards, fridges, desks,
//! whole buildings, car boots (RZ_Cars) and the bodies of infected killed near players.
//!
//! - Loot suits the building: pharmacy, garage, farm, police, shop or house, from the prefab
//!   name of the building around the spot. Tables are configurable per type; anything left
//!   empty is filled from one flat loot pool by keywords (bandage words for pharmacies, tool
//!   and fuel words for garages...).
//! - Nothing exists until someone searches: the roll happens then, scaled by the town's loot
//!   level (RZ_TownMemory.GetLootLevel), and the search is reported back (ReportSearched) so
//!   the town runs dry and recovers over about two hours. Each spot is remembered as searched
//!   for an hour. Electronics get the power grid's multiplier (RZ_PowerGrid.LootMult).
//! - Locks: some pharmacies, garages, police stations, shops and houses are locked (seeded
//!   from the building's position, so the same ones stay locked). Doors can't be locked from
//!   script, so a lock is on the loot: the stockroom, the cabinets, the safe. A key found on a
//!   body, in a note or from a wanderer opens that kind of building in that town; a crowbar
//!   forces it, loudly. Locked places have better loot.
//! - Readable things: ID cards, letters, receipts and photos, in the Zone's voice, sometimes
//!   with a key taped inside, a stash on the back, or a puzzle circled on a map.

//------------------------------------------------------------------------------------------------
//! A loot table for one building type. Empty lists are filled from the loot pool by keywords.
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sType")]
class RZ_LootTable
{
	[Attribute("house", desc: "Building type: pharmacy, garage, farm, police, shop, house, body (infected bodies) or car (car boots)")]
	string m_sType = "house";

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Everyday finds", "et")]
	ref array<ResourceName> m_aCommon = {};

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Rare finds (more likely behind a lock)", "et")]
	ref array<ResourceName> m_aRare = {};

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Electronics (radios, batteries, torches): more likely where the power is on", "et")]
	ref array<ResourceName> m_aElectronics = {};
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Interactions > Search.
[BaseContainerProps()]
class RZ_SearchSettings
{
	[Attribute("1", desc: "Searching cupboards, fridges, desks, buildings, car boots and bodies")]
	bool m_bEnabled = true;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Loot pool: every item a search can find. Tables left empty are filled from it by keywords in the prefab names (see docs/WORKBENCH_SETUP.md)", "et")]
	ref array<ResourceName> m_aLootPool = {};

	[Attribute(desc: "Loot tables per building type (optional; empty lists use the pool)")]
	ref array<ref RZ_LootTable> m_aTables;

	[Attribute("0.6", desc: "Chance a search finds anything, at full town loot (scaled by what the town has left)")]
	float m_fFindChance = 0.6;

	[Attribute("0.1", desc: "Chance a find is rare (doubled behind a lock)")]
	float m_fRareChance = 0.1;

	[Attribute("0.1", desc: "Chance a find is electronics (x the power grid's electronics multiplier)")]
	float m_fElectronicsChance = 0.1;

	[Attribute("0.02", desc: "Town loot one container search takes (RZ_TownMemory.ReportSearched)")]
	float m_fSpotShare = 0.02;

	[Attribute("0.04", desc: "Town loot a whole-building search takes")]
	float m_fBuildingShare = 0.04;

	[Attribute("60", desc: "Real minutes before a searched spot has anything again")]
	float m_fSpotReturnMinutes = 60;

	[Attribute("4", desc: "Seconds a search takes (stay close)")]
	float m_fSearchSeconds = 4;

	[Attribute("1", desc: "Heat a search makes (doors, drawers)")]
	float m_fSearchHeat = 1;

	// ---------------------------------------------------------------- Locks and keys
	[Attribute("1", desc: "Some buildings are locked: their loot is behind the lock (doors can't be locked from script)")]
	bool m_bLocks = true;

	[Attribute("0.5", desc: "Share of pharmacies that are locked")]
	float m_fLockPharmacy = 0.5;

	[Attribute("0.5", desc: "Share of police stations that are locked")]
	float m_fLockPolice = 0.5;

	[Attribute("0.35", desc: "Share of garages that are locked")]
	float m_fLockGarage = 0.35;

	[Attribute("0.25", desc: "Share of shops that are locked")]
	float m_fLockShop = 0.25;

	[Attribute("0.1", desc: "Share of houses (and farmhouses) that are locked")]
	float m_fLockHouse = 0.1;

	[Attribute("crowbar,crow_bar,prybar,pry_bar,wreckingbar,wrecking_bar", desc: "Crowbar keywords (forces a lock)")]
	string m_sCrowbarWords = "crowbar,crow_bar,prybar,pry_bar,wreckingbar,wrecking_bar";

	[Attribute("10", desc: "Seconds to force a lock")]
	float m_fForceSeconds = 10;

	[Attribute("25", desc: "Heat forcing a lock makes")]
	float m_fForceHeat = 25;

	[Attribute("180", desc: "Real minutes before an opened lock is shut again (someone came back)")]
	float m_fRelockMinutes = 180;

	// ---------------------------------------------------------------- Readable things and bodies
	[Attribute("0.12", desc: "Chance a search turns up something readable (ID card, letter, receipt, photo)")]
	float m_fReadableChance = 0.12;

	[Attribute("0.3", desc: "Chance an infected body has something readable on it")]
	float m_fBodyReadableChance = 0.3;

	[Attribute("0.08", desc: "Chance an infected body has a key on it")]
	float m_fBodyKeyChance = 0.08;

	[Attribute("4", desc: "Real minutes an infected body can be searched (players within 60 m when it died)")]
	float m_fBodyMinutes = 4;

	[Attribute("8", desc: "Most searchable bodies at once")]
	int m_iBodyCap = 8;
}

//------------------------------------------------------------------------------------------------
//! A readable thing a search turned up (its text stays on the server).
class RZ_Readable
{
	int m_iId;
	string m_sTitle;
	string m_sText;
	IEntity m_Body;
	float m_fAt;
}

//------------------------------------------------------------------------------------------------
//! A stash someone wrote about: searching near it finds it.
class RZ_Stash
{
	vector m_vPos;
	string m_sPlace;
	float m_fUntil;
}

//------------------------------------------------------------------------------------------------
class RZ_Search
{
	protected static const int NOTES_KEPT = 30;
	protected static const float NOTE_MINUTES = 30;
	protected static const float STASH_RADIUS = 30;
	protected static const float STASH_MINUTES = 90;
	protected static const int STASHES_KEPT = 6;
	protected static const float BODY_WITNESS = 60;

	protected RZ_GameModeComponent m_GM;
	protected RZ_Interactions m_Hub;
	protected RZ_SearchSettings m_Settings;
	protected ref map<string, ref RZ_LootTable> m_mTables = new map<string, ref RZ_LootTable>();
	protected ref map<string, float> m_mSearched = new map<string, float>();
	protected ref map<string, float> m_mOpened = new map<string, float>();
	protected ref map<string, ref array<string>> m_mKeys = new map<string, ref array<string>>();
	protected ref array<ref RZ_Readable> m_aNotes = {};
	protected ref array<ref RZ_Stash> m_aStashes = {};
	protected ref array<string> m_aFurnitureWords = {};
	protected ref array<string> m_aElectronicsWords = {};
	protected int m_iNextNote = 1;

	//------------------------------------------------------------------------------------------------
	void RZ_Search(RZ_GameModeComponent gm, RZ_Interactions hub, RZ_SearchSettings settings)
	{
		m_GM = gm;
		m_Hub = hub;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled() { return m_Settings.m_bEnabled; }

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	void SaveInto(RZ_InteractionSave data)
	{
		float now = m_Hub.GetClock();
		float keep = m_Settings.m_fSpotReturnMinutes * 60;
		foreach (string key, float at : m_mSearched)
		{
			if (now - at < keep)
			{
				data.searchedKey.Insert(key);
				data.searchedAt.Insert(at);
			}
		}

		float relock = m_Settings.m_fRelockMinutes * 60;
		foreach (string okey, float oat : m_mOpened)
		{
			if (now - oat < relock)
			{
				data.openKey.Insert(okey);
				data.openAt.Insert(oat);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void RestoreFrom(RZ_InteractionSave data)
	{
		for (int i = 0; i < data.searchedKey.Count() && i < data.searchedAt.Count(); i++)
		{
			m_mSearched.Set(data.searchedKey[i], data.searchedAt[i]);
		}

		for (int j = 0; j < data.openKey.Count() && j < data.openAt.Count(); j++)
		{
			m_mOpened.Set(data.openKey[j], data.openAt[j]);
		}
	}

	// ================================================================ Setup
	//------------------------------------------------------------------------------------------------
	void Setup()
	{
		RZ_ItemHelper.Words("cupboard,cabinet,wardrobe,fridge,refrigerator,freezer,desk,drawer,dresser,locker,bookcase,bookshelf,sideboard,nightstand,bedside,chest_of,kitchen_unit,kitchenunit,counter,shelf,shelves,medicine_cab,safe_", m_aFurnitureWords);
		RZ_ItemHelper.Words("radio,battery,batteries,flashlight,torch,electronic,gps,walkie,lamp_portable", m_aElectronicsWords);

		array<string> types = {"pharmacy", "garage", "farm", "police", "shop", "house", "body", "car"};
		foreach (string type : types)
		{
			BuildTable(type);
		}

		int pool = 0;
		if (m_Settings.m_aLootPool)
			pool = m_Settings.m_aLootPool.Count();

		RZ_Log.Info(string.Format("Search: %1 items in the loot pool, %2 tables set by hand.", pool, CountConfigured()));
	}

	//------------------------------------------------------------------------------------------------
	protected int CountConfigured()
	{
		if (!m_Settings.m_aTables)
			return 0;

		return m_Settings.m_aTables.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! A table per type: the one set by hand, with any empty list filled from the pool.
	protected void BuildTable(string type)
	{
		RZ_LootTable t = new RZ_LootTable();
		t.m_sType = type;
		if (m_Settings.m_aTables)
		{
			foreach (RZ_LootTable table : m_Settings.m_aTables)
			{
				if (!table || table.m_sType != type)
					continue;

				AppendAll(table.m_aCommon, t.m_aCommon);
				AppendAll(table.m_aRare, t.m_aRare);
				AppendAll(table.m_aElectronics, t.m_aElectronics);
			}
		}

		array<string> common = {};
		array<string> rare = {};
		TypeWords(type, common, rare);
		if (t.m_aCommon.IsEmpty())
			FromPool(common, t.m_aCommon);

		if (t.m_aRare.IsEmpty())
			FromPool(rare, t.m_aRare);

		if (t.m_aElectronics.IsEmpty())
			FromPool(m_aElectronicsWords, t.m_aElectronics);

		m_mTables.Set(type, t);
	}

	//------------------------------------------------------------------------------------------------
	protected void AppendAll(array<ResourceName> from, notnull array<ResourceName> into)
	{
		if (!from)
			return;

		foreach (ResourceName r : from)
		{
			if (!r.IsEmpty())
				into.Insert(r);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void FromPool(array<string> words, notnull array<ResourceName> into)
	{
		if (!m_Settings.m_aLootPool)
			return;

		foreach (ResourceName r : m_Settings.m_aLootPool)
		{
			string name = r;
			name.ToLower();
			if (RZ_ItemHelper.NameHas(name, words))
				into.Insert(r);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Keyword fallbacks per building type: what's usually there, and what's rare.
	protected void TypeWords(string type, notnull array<string> common, notnull array<string> rare)
	{
		string c = "food,can,tin,bottle,water,bandage,cloth,rag,knife,flashlight,battery,pill,shirt,jacket,matches,lighter";
		string r = "pistol,medkit,radio,binocular,map";
		switch (type)
		{
			case "pharmacy":
			{
				c = "bandage,medkit,med,pill,tablet,saline,tourniquet,syringe,vitamin,painkiller,dressing,gauze,purification,iodine,bleach";
				r = "morphine,antibiotic,epinephrine,adrenaline,saline,medkit";
				break;
			}
			case "garage":
			{
				c = "jerrycan,jerry_can,fuel,canister,tool,wrench,spanner,screwdriver,battery,tyre,tire,spark,wire,rope,plank,nail,crowbar,rag,bottle";
				r = "battery,spark,tyre,tire,crowbar,toolbox";
				break;
			}
			case "farm":
			{
				c = "food,can,tin,bread,water,bottle,axe,hatchet,knife,rope,cloth,plank,nail,wire,jerrycan,matches";
				r = "shotgun,hunting,rifle,binocular";
				break;
			}
			case "police":
			{
				c = "ammo,magazine,mag_,baton,handcuff,flashlight,radio,bandage,tourniquet,vest";
				r = "pistol,shotgun,vest,helmet,radio";
				break;
			}
			case "shop":
			{
				c = "food,can,tin,bottle,water,battery,flashlight,cloth,matches,lighter,bleach,rope";
				r = "radio,binocular,medkit";
				break;
			}
			case "body":
			{
				c = "bandage,ammo,magazine,food,cloth,rag,knife,pill,lighter,matches";
				r = "pistol,medkit,radio";
				break;
			}
			case "car":
			{
				c = "jerrycan,jerry_can,fuel,tool,wrench,rope,bottle,water,food,battery,tyre,tire,rag,map";
				r = "battery,spark,toolbox,binocular";
				break;
			}
		}

		RZ_ItemHelper.Words(c, common);
		RZ_ItemHelper.Words(r, rare);
	}

	// ================================================================ Recognising things
	//------------------------------------------------------------------------------------------------
	bool IsFurnitureName(string name)
	{
		if (!m_Settings.m_bEnabled)
			return false;

		return RZ_ItemHelper.NameHas(name, m_aFurnitureWords);
	}

	//------------------------------------------------------------------------------------------------
	//! The building type from its lower-case prefab or mesh name.
	string BuildingType(string name)
	{
		if (RZ_LandmarkFinder.Matches("pharmacy", name) || name.Contains("drugstore") || name.Contains("apothec"))
			return "pharmacy";

		if (name.Contains("garage") || name.Contains("workshop") || name.Contains("carservice") || name.Contains("car_service") || name.Contains("autoservice") || name.Contains("repair") || name.Contains("mechanic"))
			return "garage";

		if (name.Contains("police") || name.Contains("gendarm") || name.Contains("sheriff"))
			return "police";

		if (name.Contains("farm") || name.Contains("barn") || name.Contains("stable") || name.Contains("shed") || name.Contains("silo") || name.Contains("cowshed"))
			return "farm";

		if (name.Contains("shop") || name.Contains("store") || name.Contains("market") || name.Contains("grocery") || name.Contains("kiosk"))
			return "shop";

		return "house";
	}

	//------------------------------------------------------------------------------------------------
	string FurnitureNoun(string name)
	{
		if (name.Contains("fridge") || name.Contains("refrigerator") || name.Contains("freezer"))
			return "the fridge";

		if (name.Contains("desk"))
			return "the desk";

		if (name.Contains("wardrobe"))
			return "the wardrobe";

		if (name.Contains("locker"))
			return "the locker";

		if (name.Contains("drawer") || name.Contains("dresser") || name.Contains("nightstand") || name.Contains("bedside") || name.Contains("chest_of"))
			return "the drawers";

		if (name.Contains("shelf") || name.Contains("shelves") || name.Contains("bookcase"))
			return "the shelves";

		if (name.Contains("medicine"))
			return "the medicine cabinet";

		if (name.Contains("safe_"))
			return "the safe";

		return "the cupboard";
	}

	//------------------------------------------------------------------------------------------------
	static string TypeNoun(string type)
	{
		switch (type)
		{
			case "pharmacy": return "the pharmacy";
			case "garage": return "the garage";
			case "farm": return "the farmhouse";
			case "police": return "the police station";
			case "shop": return "the shop";
			case "body": return "the body";
		}

		return "the house";
	}

	//------------------------------------------------------------------------------------------------
	//! What a key's tag says.
	protected static string TypeTag(string type)
	{
		switch (type)
		{
			case "pharmacy": return "Pharmacy";
			case "garage": return "Garage";
			case "police": return "Police";
			case "shop": return "Shop, back door";
		}

		return "House";
	}

	//------------------------------------------------------------------------------------------------
	//! What's behind a lock in this kind of building.
	protected static string LockNoun(string type)
	{
		switch (type)
		{
			case "pharmacy": return "the dispensary's steel door";
			case "garage": return "the padlocked tool store";
			case "police": return "the evidence room";
			case "shop": return "the stockroom";
		}

		return "the locked back room";
	}

	// ================================================================ Locks
	//------------------------------------------------------------------------------------------------
	//! Seeded from the building's position: the same buildings are locked after a restart.
	bool IsLocked(RZ_Spot s)
	{
		if (!m_Settings.m_bLocks || (s.m_iKind != RZ_Interactions.KIND_FURNITURE && s.m_iKind != RZ_Interactions.KIND_BUILDING))
			return false;

		float opened;
		if (m_mOpened.Find(s.m_sBuildingKey, opened) && m_Hub.GetClock() - opened < m_Settings.m_fRelockMinutes * 60)
			return false;

		float share = m_Settings.m_fLockHouse;
		switch (s.m_sType)
		{
			case "pharmacy": share = m_Settings.m_fLockPharmacy; break;
			case "police": share = m_Settings.m_fLockPolice; break;
			case "garage": share = m_Settings.m_fLockGarage; break;
			case "shop": share = m_Settings.m_fLockShop; break;
		}

		int gx = Math.Floor(s.m_vBuilding[0] / 4);
		int gz = Math.Floor(s.m_vBuilding[2] / 4);
		RZ_Rng rng = new RZ_Rng((m_Hub.GetSeed() % 100000) + gx * 7919 + gz * 104729);
		return rng.Chance(share);
	}

	//------------------------------------------------------------------------------------------------
	protected void Open(RZ_Spot s)
	{
		m_mOpened.Set(s.m_sBuildingKey, m_Hub.GetClock());
		m_Hub.RelabelBuilding(s.m_sBuildingKey);
	}

	//------------------------------------------------------------------------------------------------
	//! "pharmacy@3": opens that kind of building in that town.
	protected string KeyFor(int town, string type)
	{
		return string.Format("%1@%2", type, town);
	}

	//------------------------------------------------------------------------------------------------
	protected array<string> KeysOf(int playerId)
	{
		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		array<string> keys;
		if (!m_mKeys.Find(ident, keys))
		{
			keys = new array<string>();
			m_mKeys.Set(ident, keys);
		}

		return keys;
	}

	//------------------------------------------------------------------------------------------------
	//! Keys are data carried with this life (a ring in your pocket), not items.
	void GiveKey(int playerId, string key)
	{
		array<string> keys = KeysOf(playerId);
		if (!keys.Contains(key))
			keys.Insert(key);
	}

	//------------------------------------------------------------------------------------------------
	bool HasKey(int playerId, string key)
	{
		return KeysOf(playerId).Contains(key);
	}

	//------------------------------------------------------------------------------------------------
	//! A key for a random lockable kind of building in a town near pos. Returns a description.
	string GiveRandomKey(int playerId, vector pos)
	{
		int town = m_GM.FindNearestSettlement(pos);
		RZ_Settlement s = m_GM.GetSettlement(town);
		if (!s)
			return "";

		array<string> types = {"pharmacy", "garage", "house", "shop", "police"};
		string type = types.GetRandomElement();
		GiveKey(playerId, KeyFor(town, type));
		return string.Format("a key with a cardboard tag: \"%1, %2\"", TypeTag(type), s.m_sName);
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerSpawned(int playerId)
	{
		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		m_mKeys.Remove(ident);
	}

	// ================================================================ Bodies
	//------------------------------------------------------------------------------------------------
	//! Infected that die with a player close by leave a body to search for a few minutes.
	void OnZombiesKilled(vector pos, int count)
	{
		if (!m_Settings.m_bEnabled || count <= 0)
			return;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		if (RZ_Util.NearestDistance(pos, players) > BODY_WITNESS)
			return;

		if (m_Hub.CountKind(RZ_Interactions.KIND_BODY) >= m_Settings.m_iBodyCap)
			return;

		m_Hub.AddSpot(RZ_Interactions.KIND_BODY, RZ_Util.GroundPos(pos), "body", "the body", -1, m_Hub.GetClock() + m_Settings.m_fBodyMinutes * 60);
	}

	// ================================================================ Labels and looking
	//------------------------------------------------------------------------------------------------
	void Labels(RZ_Spot s, out string read, out string choiceLabels)
	{
		if (s.m_iKind == RZ_Interactions.KIND_BODY)
		{
			read = "Look at the body";
			choiceLabels = "Search the body";
			return;
		}

		read = "Look at " + s.m_sNoun;
		if (s.m_iKind == RZ_Interactions.KIND_BUILDING)
			read = "Look around " + s.m_sNoun;

		string burn = "Set it alight (fuel)";
		if (IsLocked(s))
		{
			choiceLabels = string.Format("Search %1|Use a key|Force the lock (crowbar)|%2", s.m_sNoun, burn);
			return;
		}

		choiceLabels = string.Format("Search %1|||%2", s.m_sNoun, burn);
	}

	//------------------------------------------------------------------------------------------------
	void Describe(int playerId, RZ_Spot s)
	{
		string title = m_Hub.Title(s);
		if (s.m_iKind == RZ_Interactions.KIND_BODY)
		{
			m_GM.NotifyPlayer(playerId, title, "It was someone, once. Pockets, a bag, a wallet maybe. Be quick: they don't lie still for long out here.");
			return;
		}

		RZ_Traps traps = m_Hub.GetTraps();
		if (traps.IsBurned(s.m_vBuilding))
		{
			m_GM.NotifyPlayer(playerId, title, "Burned out. Black beams and ash, and the smell. Nothing left in there worth the risk.");
			return;
		}

		array<string> lines = {};
		lines.Insert(TypeLook(s.m_sType));
		if (IsLocked(s))
			lines.Insert(string.Format("Whatever's worth having is behind %1. It's locked. A key would do it, or a crowbar and a lot of noise.", LockNoun(s.m_sType)));

		if (WasSearched(s.m_sKey))
			lines.Insert("Someone's been through it already. Recently.");

		float level = RZ_TownMemory.GetLootLevel(s.m_vPos);
		if (level < 0.5)
			lines.Insert("The whole town looks picked over.");

		string text = "";
		foreach (int i, string l : lines)
		{
			if (i > 0)
				text += " ";

			text += l;
		}

		m_GM.NotifyPlayer(playerId, title, text);
	}

	//------------------------------------------------------------------------------------------------
	protected static string TypeLook(string type)
	{
		switch (type)
		{
			case "pharmacy": return "Broken glass, empty shelves at the front. The back is another matter.";
			case "garage": return "Oil on the floor, a calendar from before. Tools, if nobody took them.";
			case "farm": return "Boots by the door, a kitchen that smells of damp. Farm people kept things.";
			case "police": return "Overturned chairs, a notice board of missing faces.";
			case "shop": return "The front was looted on the first day. People forget to look behind the counter.";
		}

		return "Someone's home. Coats still on the hooks.";
	}

	//------------------------------------------------------------------------------------------------
	protected bool WasSearched(string key)
	{
		float at;
		return m_mSearched.Find(key, at) && m_Hub.GetClock() - at < m_Settings.m_fSpotReturnMinutes * 60;
	}

	// ================================================================ Choices
	//------------------------------------------------------------------------------------------------
	//! 0 search, 1 use a key, 2 force the lock, 3 set it alight.
	void HandleChoice(int playerId, RZ_Spot s, int option, IEntity user)
	{
		if (!m_Settings.m_bEnabled)
			return;

		string title = m_Hub.Title(s);
		if (option == 3)
		{
			m_Hub.GetTraps().StartBurn(playerId, s, user);
			return;
		}

		if (s.m_iKind != RZ_Interactions.KIND_BODY && m_Hub.GetTraps().IsBurned(s.m_vBuilding))
		{
			m_GM.NotifyPlayer(playerId, title, "It's burned out. There's nothing left to find.");
			return;
		}

		if (option == 1)
		{
			UseKey(playerId, s);
			return;
		}

		if (option == 2)
		{
			if (!IsLocked(s))
			{
				m_GM.NotifyPlayer(playerId, title, "It's open.");
				return;
			}

			if (!RZ_ItemHelper.Has(user, m_Settings.m_sCrowbarWords))
			{
				m_GM.NotifyPlayer(playerId, title, "You'd need a crowbar for that lock. Garages and sheds have them.");
				return;
			}

			m_Hub.StartTask(playerId, user, RZ_Interactions.TASK_FORCE, s.m_iId, 0, m_Settings.m_fForceSeconds, "the lock", false);
			m_GM.GetHeat().AddHeat(s.m_vPos, m_Settings.m_fForceHeat * 0.4);
			m_GM.NotifyPlayer(playerId, title, string.Format("You set the crowbar and lean on it. It's loud. About %1 seconds; stay close.", RZ_Util.RoundToInt(m_Settings.m_fForceSeconds)));
			return;
		}

		if (option != 0)
			return;

		if (IsLocked(s))
		{
			int town = m_GM.FindNearestSettlement(s.m_vBuilding);
			if (HasKey(playerId, KeyFor(town, s.m_sType)))
			{
				Open(s);
				m_GM.NotifyPlayer(playerId, title, "You try your keys. The third one turns.");
			}
			else
			{
				m_GM.NotifyPlayer(playerId, title, string.Format("The front has been picked clean; what's left is behind %1, and it's locked. A key, or a crowbar.", LockNoun(s.m_sType)));
				return;
			}
		}

		StartSearch(playerId, s, user);
	}

	//------------------------------------------------------------------------------------------------
	//! A search of any spot (furniture, building, body, car boot): a few seconds, then the roll.
	void StartSearch(int playerId, RZ_Spot s, IEntity user)
	{
		if (WasSearched(s.m_sKey))
		{
			m_GM.NotifyPlayer(playerId, m_Hub.Title(s), "You've been through it, or someone has. Nothing new. Give it time, or try somewhere else.");
			return;
		}

		m_Hub.StartTask(playerId, user, RZ_Interactions.TASK_SEARCH, s.m_iId, 0, m_Settings.m_fSearchSeconds, "the search", false);
	}

	//------------------------------------------------------------------------------------------------
	protected void UseKey(int playerId, RZ_Spot s)
	{
		string title = m_Hub.Title(s);
		if (!IsLocked(s))
		{
			m_GM.NotifyPlayer(playerId, title, "It's open.");
			return;
		}

		int town = m_GM.FindNearestSettlement(s.m_vBuilding);
		if (!HasKey(playerId, KeyFor(town, s.m_sType)))
		{
			m_GM.NotifyPlayer(playerId, title, "None of your keys fit. Whoever had this one kept it.");
			return;
		}

		Open(s);
		m_GM.NotifyPlayer(playerId, title, string.Format("The key turns. %1 is open.", LockNoun(s.m_sType)));
	}

	//------------------------------------------------------------------------------------------------
	//! A forced lock: open, loud.
	void CompleteForce(int playerId, RZ_Spot s, IEntity user)
	{
		if (!s)
			return;

		Open(s);
		m_GM.GetHeat().AddHeat(s.m_vPos, m_Settings.m_fForceHeat);
		m_GM.NotifyPlayer(playerId, m_Hub.Title(s), "The lock gives with a crack you'll hear in your sleep. It's open. So is every ear for a mile.");
	}

	// ================================================================ Searching
	//------------------------------------------------------------------------------------------------
	void CompleteSearch(int playerId, RZ_Spot s, IEntity user)
	{
		if (!s)
			return;

		string title = m_Hub.Title(s);
		bool body = s.m_iKind == RZ_Interactions.KIND_BODY;
		bool locked = !body && WasLockedHere(s);
		m_mSearched.Set(s.m_sKey, m_Hub.GetClock());
		PruneSearched();
		m_GM.GetHeat().AddHeat(s.m_vPos, m_Settings.m_fSearchHeat);

		float share = m_Settings.m_fSpotShare;
		if (s.m_iKind == RZ_Interactions.KIND_BUILDING)
			share = m_Settings.m_fBuildingShare;

		array<string> found = {};
		int items = RollLoot(s.m_sType, s.m_vPos, locked, body, user, s.m_vPos, found);
		if (!body)
			RZ_TownMemory.ReportSearched(s.m_vPos, share);

		// A stash someone wrote about
		RZ_Stash stash = StashNear(s.m_vPos);
		if (stash)
		{
			m_aStashes.RemoveItem(stash);
			for (int i = 0; i < 2; i++)
			{
				items += RollLoot(s.m_sType, s.m_vPos, true, false, user, s.m_vPos, found);
			}

			found.Insert("the stash from the note, wrapped in a bin bag");
		}

		// Car keys for a fixable car in this town, sometimes
		string carKeys = m_Hub.GetCars().MaybeFindKeys(playerId, s.m_vPos);
		if (!carKeys.IsEmpty())
			found.Insert(carKeys);

		// A key on a body
		if (body && Math.RandomFloat01() < m_Settings.m_fBodyKeyChance)
		{
			string key = GiveRandomKey(playerId, s.m_vPos);
			if (!key.IsEmpty())
				found.Insert(key);
		}

		string text = "";
		if (found.IsEmpty())
			text = EmptyText(s.m_sType, body, RZ_TownMemory.GetLootLevel(s.m_vPos));
		else
			text = "You find " + ListText(found) + ".";

		float readChance = m_Settings.m_fReadableChance;
		if (body)
			readChance = m_Settings.m_fBodyReadableChance;

		if (Math.RandomFloat01() < readChance)
		{
			string readable = MakeReadable(playerId, s, body);
			if (!readable.IsEmpty())
				text += "\n" + readable;
		}

		m_GM.NotifyPlayer(playerId, title, text);
		if (body)
			m_Hub.RemoveSpot(s.m_iId);
	}

	//------------------------------------------------------------------------------------------------
	//! A building that was locked when this life started looking: opened locks keep better loot
	//! for as long as they stay open.
	protected bool WasLockedHere(RZ_Spot s)
	{
		if (!m_Settings.m_bLocks)
			return false;

		float opened;
		return m_mOpened.Find(s.m_sBuildingKey, opened);
	}

	//------------------------------------------------------------------------------------------------
	protected void PruneSearched()
	{
		if (m_mSearched.Count() < 800)
			return;

		float now = m_Hub.GetClock();
		float keep = m_Settings.m_fSpotReturnMinutes * 60;
		array<string> old = {};
		foreach (string key, float at : m_mSearched)
		{
			if (now - at >= keep)
				old.Insert(key);
		}

		foreach (string k : old)
		{
			m_mSearched.Remove(k);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Rolls what a search finds and gives it. Returns how many items. Names go into `found`.
	protected int RollLoot(string type, vector pos, bool locked, bool body, IEntity user, vector at, notnull array<string> found)
	{
		RZ_LootTable t;
		if (!m_mTables.Find(type, t))
			m_mTables.Find("house", t);

		if (!t)
			return 0;

		float level = 1;
		if (!body)
			level = RZ_TownMemory.GetLootLevel(pos);

		float chance = m_Settings.m_fFindChance * level;
		if (locked)
			chance += 0.25;

		if (Math.RandomFloat01() >= Math.Clamp(chance, 0, 0.95))
			return 0;

		int count = 1;
		float twoChance = 0.2 * level;
		if (locked)
			twoChance = 0.5;

		if (Math.RandomFloat01() < twoChance)
			count = 2;

		float rare = m_Settings.m_fRareChance;
		if (locked)
			rare = rare * 2;

		float electronics = m_Settings.m_fElectronicsChance * RZ_PowerGrid.LootMult(pos, "electronics");
		int given = 0;
		for (int i = 0; i < count; i++)
		{
			ResourceName pick = PickFrom(t, rare, electronics);
			if (pick.IsEmpty())
				continue;

			if (RZ_ItemHelper.Give(pick, user, at))
			{
				found.Insert(RZ_ItemHelper.DisplayName(pick));
				given++;
			}
		}

		return given;
	}

	//------------------------------------------------------------------------------------------------
	protected ResourceName PickFrom(RZ_LootTable t, float rare, float electronics)
	{
		float roll = Math.RandomFloat01();
		if (roll < electronics && !t.m_aElectronics.IsEmpty())
			return t.m_aElectronics.GetRandomElement();

		if (roll < electronics + rare && !t.m_aRare.IsEmpty())
			return t.m_aRare.GetRandomElement();

		if (!t.m_aCommon.IsEmpty())
			return t.m_aCommon.GetRandomElement();

		if (m_Settings.m_aLootPool && !m_Settings.m_aLootPool.IsEmpty())
			return m_Settings.m_aLootPool.GetRandomElement();

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected static string ListText(array<string> parts)
	{
		string text = "";
		foreach (int i, string p : parts)
		{
			if (i > 0 && i == parts.Count() - 1)
				text += " and ";
			else if (i > 0)
				text += ", ";

			text += p;
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	protected static string EmptyText(string type, bool body, float level)
	{
		if (body)
			return "Nothing but a bus ticket and a photo too stained to make out.";

		if (level < 0.4)
			return "Empty. Drawers out, doors open. Somebody beat you to it, and somebody beat them.";

		switch (type)
		{
			case "pharmacy": return "Empty blister packs and a price gun. Nothing you can use.";
			case "garage": return "Rusted bolts and an empty oil can.";
			case "farm": return "Seed packets and a calendar with every day crossed off.";
			case "police": return "Forms. Hundreds of forms.";
		}

		return "Nothing you can use. Old letters, a broken clock.";
	}

	// ================================================================ Readable things
	//------------------------------------------------------------------------------------------------
	//! An ID card, a letter, a receipt or a photo, in the Zone's voice. Sometimes a key is taped
	//! inside, a stash is written on the back, or a town is circled. Returns the line for the
	//! search message; the text is also kept on a note left at the spot (if a note prefab is set).
	protected string MakeReadable(int playerId, RZ_Spot s, bool body)
	{
		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		int town = m_GM.FindNearestSettlement(s.m_vPos);
		RZ_Settlement here = m_GM.GetSettlement(town);
		string place = "the village";
		if (here)
			place = here.m_sName;

		int day = 11;
		if (m_GM.GetBroadcast())
			day = m_GM.GetBroadcast().GetDay();

		string name = RZ_NoteText.FullName(rng);
		string first = RZ_NoteText.FirstName(rng);
		int kind = rng.Int(0, 4);
		if (body && rng.Chance(0.5))
			kind = 0;

		string title;
		string text;
		switch (kind)
		{
			case 0:
			{
				title = "An ID card";
				text = string.Format("An identity card: %1, resident of %2. The photo is someone smiling at a camera in a different world. On the back, in biro: \"%3\"", name, place, RZ_NoteText.PersonalLine(rng, place));
				break;
			}
			case 1:
			{
				title = "A letter";
				text = string.Format("\"%1, if you get this we've gone. The radio said stay indoors, so we stayed indoors, and then the water stopped. %2\" %3", first, RZ_NoteText.PersonalLine(rng, place), RZ_NoteText.Signoff(rng, RZ_NoteText.FirstName(rng)));
				break;
			}
			case 2:
			{
				array<string> goods = {"tinned peaches", "batteries (all of them)", "bleach", "candles", "a tin opener", "dog food", "rice, 5 kg", "plasters"};
				rng.ShuffleStrings(goods);
				title = "A receipt";
				text = string.Format("A till receipt from %1, %2: %3, %4, %5. Paid in cash. Underneath, in pencil: \"enough for a week?\"", place, RZ_NoteText.Day(Math.Max(1, day - rng.Int(8, 11))), goods[0], goods[1], goods[2]);
				break;
			}
			default:
			{
				title = "A photo";
				text = string.Format("A photograph: two people squinting into the sun outside a house in %1. On the back: \"%2 and me, summer. Before.\"", place, first);
				break;
			}
		}

		// Sometimes it points somewhere
		float roll = rng.Float01();
		if (roll < 0.2)
		{
			string key = GiveRandomKey(playerId, s.m_vPos);
			if (!key.IsEmpty())
				text += string.Format(" Taped inside: %1. You keep it.", key);
		}
		else if (roll < 0.35)
		{
			string stash = AddStash(s.m_vPos, rng);
			if (!stash.IsEmpty())
				text += " " + stash;
		}
		else if (roll < 0.45)
		{
			string puzzle = PuzzleHint(s.m_vPos);
			if (!puzzle.IsEmpty())
				text += " " + puzzle;
		}

		AddNote(title, text, s.m_vPos);
		return string.Format("%1: %2", title, text);
	}

	//------------------------------------------------------------------------------------------------
	//! A stash near a landmark in a town nearby. "" if none could be placed.
	protected string AddStash(vector from, RZ_Rng rng)
	{
		int idx = RZ_Mission.PickSettlementAway(m_GM, from, 0, 2500, 0, 85);
		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (!s)
			return "";

		array<string> kinds = {"church", "school", "station", "bank", "fuel", "watertower"};
		RZ_Landmark l = RZ_LandmarkFinder.FindNearest(s.m_vCenter, s.m_fRadius + 100, kinds);
		RZ_Stash stash = new RZ_Stash();
		string near = "the middle of " + s.m_sName;
		stash.m_vPos = RZ_Mission.LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.4);
		if (l)
		{
			stash.m_vPos = l.m_vPos;
			near = string.Format("%1 in %2", l.m_sName, s.m_sName);
		}

		stash.m_sPlace = near;
		stash.m_fUntil = m_Hub.GetClock() + STASH_MINUTES * 60;
		m_aStashes.Insert(stash);
		while (m_aStashes.Count() > STASHES_KEPT)
		{
			m_aStashes.Remove(0);
		}

		return string.Format("On the back, in a hurry: \"Left the good things in the house nearest %1. Grid %2. Don't let Pavel have them.\"", near, RZ_SettlementScanner.GridReference(stash.m_vPos));
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Stash StashNear(vector pos)
	{
		float now = m_Hub.GetClock();
		foreach (RZ_Stash st : m_aStashes)
		{
			if (now < st.m_fUntil && RZ_Util.DistXZ(st.m_vPos, pos) <= STASH_RADIUS)
				return st;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! A puzzle mission in progress, circled on a map. "" if none.
	protected string PuzzleHint(vector from)
	{
		foreach (RZ_Mission m : m_GM.GetMissions().GetActive())
		{
			if (!RZ_PuzzleMission.Cast(m) || m.m_bGlobal)
				continue;

			return string.Format("Folded into it, a road map with %1 circled twice, and \"ask the right questions\" in the margin.", m.GetLocationName(m_GM));
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Leaves the readable thing at the spot, if a note prefab is set, so others can read it too.
	protected void AddNote(string title, string text, vector pos)
	{
		ResourceName prefab = m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CLUE);
		if (prefab.IsEmpty())
			return;

		RZ_Readable r = new RZ_Readable();
		r.m_iId = m_iNextNote;
		m_iNextNote++;
		r.m_sTitle = title;
		r.m_sText = text;
		r.m_fAt = m_Hub.GetClock();
		r.m_Body = RZ_Util.SpawnPrefab(prefab, RZ_Util.RandomPointInRing(pos, 0.4, 1) + Vector(0, 0.2, 0), Math.RandomFloat(0, 360));
		if (!r.m_Body)
			return;

		RZ_ClueComponent.Tag(r.m_Body, RZ_Interactions.TAG_NOTE, r.m_iId, "Read it: " + title);
		m_aNotes.Insert(r);
		while (m_aNotes.Count() > NOTES_KEPT)
		{
			RemoveNote(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveNote(int idx)
	{
		RZ_Readable r = m_aNotes[idx];
		if (r.m_Body)
			SCR_EntityHelper.DeleteEntityAndChildren(r.m_Body);

		m_aNotes.Remove(idx);
	}

	//------------------------------------------------------------------------------------------------
	void ReadNote(int playerId, int id)
	{
		foreach (RZ_Readable r : m_aNotes)
		{
			if (r.m_iId == id)
			{
				m_GM.NotifyPlayer(playerId, r.m_sTitle, r.m_sText);
				return;
			}
		}

		m_GM.NotifyPlayer(playerId, "Nothing", "The ink has run. You can't make out a word.");
	}

	//------------------------------------------------------------------------------------------------
	//! Readable things don't stay: gone after half an hour (no physical traces).
	void Tick(float dt)
	{
		float now = m_Hub.GetClock();
		for (int i = m_aNotes.Count() - 1; i >= 0; i--)
		{
			if (now - m_aNotes[i].m_fAt >= NOTE_MINUTES * 60 || !m_aNotes[i].m_Body)
				RemoveNote(i);
		}

		for (int j = m_aStashes.Count() - 1; j >= 0; j--)
		{
			if (now >= m_aStashes[j].m_fUntil)
				m_aStashes.Remove(j);
		}
	}
}
