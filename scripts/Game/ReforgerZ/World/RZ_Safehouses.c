//! Haven safehouses (docs/DESIGN.md, "Haven safehouses"): a single Haven-held spot in an
//! otherwise abandoned town. Population is a number, not AI: a safehouse of 4 and
//! one of 30 show the same few occupants, and only while a player is within 300 m.
//! What population changes is what the safehouse offers (shop tier, guards, medic,
//! radio). Everything here runs on the server.

enum RZ_ESafehouseTier
{
	SHELTER,	// 0-4: a spawn point, basic Haven shop
	COMMUNITY,	// 5-11: more stock, one guard
	SETTLEMENT,	// 12-19: a medic, a radio
	TOWN		// 20-30: Champion stock, two guards
}

//! What an RZ_SafehouseAction does.
enum RZ_ESafehouseAction
{
	STATUS,			// "How are things here?" population, food, what they need
	DELIVER_FOOD,	// hand over food items for Favor (tops up the food stock)
	HEAL,			// the medic patches you up for Favor (Settlement and up)
	DECLARE_BITE,	// tell them you're bitten: slows the infection once per life (every safehouse)
	RADIO,			// mission intel from the safehouse radio (Settlement and up)
	MEDS,			// low-grade meds for a little Favor (every safehouse)
	BOARD_READ,		// read the notice board (World/RZ_NoticeBoard.c)
	BOARD_WRITE		// how to leave a note on it (the board menu: a ready-made note or typed; without it /note in chat)
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Haven. Prefabs are optional: without them the
//! safehouses still work as data, shops and spawn points, they just look empty.
[BaseContainerProps()]
class RZ_SafehouseSettings
{
	[Attribute("1", desc: "Haven safehouses on. Off = no safehouses, population or house rules")]
	bool m_bEnabled;

	[Attribute("3", desc: "Safehouses picked at start when none are placed (fewer on small maps: one per 8 settlements, at least 1)")]
	int m_iCount;

	[Attribute("4", desc: "Starting population of a new safehouse")]
	int m_iStartPopulation;

	[Attribute("12", desc: "Starting food stock (units) of a new safehouse")]
	int m_iStartFood;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Occupants: civilian characters or seated figure props (no AI needed). 3-6 are shown while a player is within 300 m. Empty = the puzzle civilian", "et")]
	ref array<ResourceName> m_aOccupantPrefabs;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Guard: a Haven AI character or group on the player faction (Community and up)", "et")]
	ResourceName m_sGuardPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Medic (optional, Settlement and up): a civilian with RZ_SafehouseAction set to HEAL and DECLARE_BITE", "et")]
	ResourceName m_sMedicPrefab;

	[Attribute("300", desc: "Occupants, guards, medic and the safehouse quartermaster exist only while a player is this close (m)")]
	float m_fVisibleRadius;

	[Attribute("180", desc: "Real minutes between upkeep (food eaten). Only counts while players are online")]
	float m_fUpkeepMinutes;

	[Attribute("5", desc: "People one unit of food feeds per upkeep")]
	int m_iPeoplePerFood;

	[Attribute("", desc: "Food items: prefab names containing any of these words (lower case) count as food. Empty = food, canned, ration, tushonka, bread, beans")]
	ref array<string> m_aFoodKeywords;

	[Attribute("2", desc: "Haven Favor paid per food item delivered")]
	int m_iFavorPerFood;

	[Attribute("15", desc: "Haven Favor the medic charges to patch you up")]
	int m_iHealCost;

	[Attribute("50", desc: "House rule: firing within this distance (m) of a safehouse costs Haven Infamy")]
	float m_fNoFireRadius;

	[Attribute("40", desc: "...unless an infected is this close (m) to the safehouse: then you're defending it")]
	float m_fDefendRadius;

	[Attribute("3", desc: "Haven Infamy for firing at a safehouse (at most once every 20 s)")]
	int m_iFireInfamy;

	[Attribute("15", desc: "Haven Infamy for being caught hiding a bite inside a safehouse")]
	int m_iCaughtInfamy;

	[Attribute("0.3", desc: "Chance per minute that a bitten player who hasn't declared it is caught inside a safehouse in the Final stage (half that in Weakness; never in Fever, the golden window, or on a fever suppressant)")]
	float m_fCaughtChance;

	[Attribute("1.15", desc: "Declaring a bite at a Shelter: the time left x this (once per life)")]
	float m_fDeclareShelter;

	[Attribute("1.25", desc: "...at a Community")]
	float m_fDeclareCommunity;

	[Attribute("1.4", desc: "...at a Settlement (it has a medic)")]
	float m_fDeclareSettlement;

	[Attribute("1.5", desc: "...at a Town")]
	float m_fDeclareTown;

	[Attribute("5", desc: "Haven Favor for a dose of expired antibiotics (every safehouse, anyone not exiled; needs Survival > Infection > Antibiotics prefab)")]
	int m_iMedsCost;

	[Attribute("4", desc: "Stragglers: a fed safehouse under 12 people takes in one person every this many hours with players online (divided by season speed). 0 = only rescues add people")]
	float m_fStragglerHours;

	[Attribute("0.25", desc: "Chance every 10 min that a safehouse short on food sells a Hunted player's rough location to bounty hunters")]
	float m_fLeakChance;

	// ---------------------------------------------------------------- Notice boards (RZ_NoticeBoard)
	[Attribute("1", desc: "Notice boards: short notes players leave at a safehouse between sessions (cosmetic and social only)")]
	bool m_bNoticeBoards;

	[Attribute("8", UIWidgets.Slider, "Notes kept per board (the newest)", "1 20 1")]
	int m_iBoardNotes;

	[Attribute("160", desc: "Longest note (characters); longer ones are cut")]
	int m_iNoteLength;

	[Attribute("10", desc: "Real minutes between two notes from the same player (any board)")]
	float m_fNoteGapMinutes;

	[Attribute("3", desc: "Real days before a note comes down by itself")]
	float m_fNoteDays;

	[Attribute("", desc: "Word filter: words (lower case, comma-separated) blanked out of notes with asterisks. Empty = no filter")]
	string m_sNoteFilter;
}

//------------------------------------------------------------------------------------------------
//! Saved form of one safehouse.
class RZ_SafehouseSave
{
	string name;
	int settlement = -1;
	float x;
	float z;
	int population;
	int food;
	float upkeep;
	bool anchored;
	bool shortFood;
	int lost;
	int died;
	float straggle;
	ref array<ref RZ_Note> notes = {};
	int seasonDied;
	int seasonRescued;
}

//------------------------------------------------------------------------------------------------
//! One safehouse. Data only, plus the bodies that exist while a player is near.
class RZ_Safehouse
{
	string m_sName;
	int m_iSettlement = -1;
	vector m_vPos;
	int m_iPopulation = 4;
	int m_iFood = 12;
	float m_fUpkeepTimer;
	bool m_bAnchored;	// sits at the Haven quartermaster, which already exists: don't spawn another
	bool m_bShort;		// the last upkeep ran short: the safehouse is under pressure
	int m_iLost;		// people gone lately for any reason (left hungry, moved, taken, dead), fading by one an hour (story arcs)
	int m_iDied;		// of those, the dead (RemoveDead), fading the same way
	int m_iSeasonDied;		// the dead this season (the epilogue's candles; never fades)
	int m_iSeasonRescued;	// rescued in this season
	float m_fLostFade;
	float m_fStraggleTimer;	// stragglers: one more person every few hours while fed
	ref array<ref RZ_Note> m_aNotes = {};	// the notice board, oldest first (RZ_NoticeBoard)

	// Runtime
	ref array<IEntity> m_aBodies = {};
	bool m_bSpawned;
	int m_iSpawnedTier = -1;
	IEntity m_SpawnPoint;

	//------------------------------------------------------------------------------------------------
	RZ_ESafehouseTier GetTier()
	{
		return TierFor(m_iPopulation);
	}

	//------------------------------------------------------------------------------------------------
	static RZ_ESafehouseTier TierFor(int population)
	{
		if (population >= 20)
			return RZ_ESafehouseTier.TOWN;
		if (population >= 12)
			return RZ_ESafehouseTier.SETTLEMENT;
		if (population >= 5)
			return RZ_ESafehouseTier.COMMUNITY;
		return RZ_ESafehouseTier.SHELTER;
	}

	//------------------------------------------------------------------------------------------------
	static string TierName(RZ_ESafehouseTier tier)
	{
		switch (tier)
		{
			case RZ_ESafehouseTier.SHELTER: return "Shelter";
			case RZ_ESafehouseTier.COMMUNITY: return "Community";
			case RZ_ESafehouseTier.SETTLEMENT: return "Settlement";
			case RZ_ESafehouseTier.TOWN: return "Town";
		}

		return "Shelter";
	}

	//------------------------------------------------------------------------------------------------
	bool HasMedic()
	{
		return GetTier() >= RZ_ESafehouseTier.SETTLEMENT;
	}

	//------------------------------------------------------------------------------------------------
	bool HasRadio()
	{
		return GetTier() >= RZ_ESafehouseTier.SETTLEMENT;
	}

	//------------------------------------------------------------------------------------------------
	int GuardCount()
	{
		RZ_ESafehouseTier tier = GetTier();
		if (tier == RZ_ESafehouseTier.TOWN)
			return 2;
		if (tier == RZ_ESafehouseTier.SHELTER)
			return 0;
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! 3 to 6 bodies, never more than the people who live here (at least one keeper).
	int OccupantCount()
	{
		int shown = 3 + GetTier();
		return Math.Max(1, Math.Min(shown, m_iPopulation));
	}

	//------------------------------------------------------------------------------------------------
	//! Highest shop tier stocked here, as an access rank (Known 1, Trusted 2, Champion 3).
	int ShopRankCap()
	{
		RZ_ESafehouseTier tier = GetTier();
		if (tier == RZ_ESafehouseTier.TOWN)
			return 3;
		if (tier == RZ_ESafehouseTier.SHELTER)
			return 1;
		return 2;
	}

	//------------------------------------------------------------------------------------------------
	int FoodNeed(int peoplePerFood)
	{
		int per = Math.Max(1, peoplePerFood);
		return Math.Max(1, (m_iPopulation + per - 1) / per);
	}

	//------------------------------------------------------------------------------------------------
	RZ_SafehouseSave ToSave()
	{
		RZ_SafehouseSave s = new RZ_SafehouseSave();
		s.name = m_sName;
		s.settlement = m_iSettlement;
		s.x = m_vPos[0];
		s.z = m_vPos[2];
		s.population = m_iPopulation;
		s.food = m_iFood;
		s.upkeep = m_fUpkeepTimer;
		s.anchored = m_bAnchored;
		s.shortFood = m_bShort;
		s.lost = m_iLost;
		s.died = m_iDied;
		s.straggle = m_fStraggleTimer;
		s.seasonDied = m_iSeasonDied;
		s.seasonRescued = m_iSeasonRescued;
		foreach (RZ_Note n : m_aNotes)
		{
			s.notes.Insert(n);
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_Safehouse FromSave(RZ_SafehouseSave s)
	{
		RZ_Safehouse h = new RZ_Safehouse();
		h.m_sName = s.name;
		h.m_iSettlement = s.settlement;
		h.m_vPos = RZ_Util.GroundPos(Vector(s.x, 0, s.z));
		h.m_iPopulation = Math.ClampInt(s.population, 0, RZ_Safehouses.POP_CAP);
		h.m_iFood = Math.Max(0, s.food);
		h.m_fUpkeepTimer = s.upkeep;
		h.m_bAnchored = s.anchored;
		h.m_bShort = s.shortFood;
		h.m_iLost = Math.Max(0, s.lost);
		h.m_iDied = Math.Max(0, s.died);
		h.m_fStraggleTimer = Math.Max(0, s.straggle);
		h.m_iSeasonDied = Math.Max(0, s.seasonDied);
		h.m_iSeasonRescued = Math.Max(0, s.seasonRescued);
		if (s.notes)
		{
			foreach (RZ_Note n : s.notes)
			{
				if (n)
					h.m_aNotes.Insert(n);
			}
		}

		return h;
	}
}

//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "ReforgerZ", description: "A Haven safehouse, placed by hand. If any of these exist, safehouses aren't picked automatically.")]
class RZ_SafehouseMarkerEntityClass : GenericEntityClass
{
}

//! Optional. Drop these where the Haven should hold out. Without any, RZ_Safehouses
//! picks quiet, low-infection villages itself. Population and food are still saved;
//! a marker only fixes the position (and the starting numbers on a fresh season).
class RZ_SafehouseMarkerEntity : GenericEntity
{
	[Attribute("", desc: "Name (empty = the nearest settlement's name)")]
	protected string m_sName;

	[Attribute("4", UIWidgets.Slider, "Starting population", "0 30 1")]
	protected int m_iStartPopulation;

	[Attribute("12", UIWidgets.Slider, "Starting food stock", "0 100 1")]
	protected int m_iStartFood;

	protected static ref array<RZ_SafehouseMarkerEntity> s_aAll;

	//------------------------------------------------------------------------------------------------
	static array<RZ_SafehouseMarkerEntity> GetAll()
	{
		if (!s_aAll)
			s_aAll = new array<RZ_SafehouseMarkerEntity>();

		return s_aAll;
	}

	string GetMarkerName() { return m_sName; }
	int GetStartPopulation() { return m_iStartPopulation; }
	int GetStartFood() { return m_iStartFood; }

	//------------------------------------------------------------------------------------------------
	void RZ_SafehouseMarkerEntity(IEntitySource src, IEntity parent)
	{
		GetAll().Insert(this);
	}

	//------------------------------------------------------------------------------------------------
	void ~RZ_SafehouseMarkerEntity()
	{
		if (s_aAll)
			s_aAll.RemoveItem(this);
	}
}

//------------------------------------------------------------------------------------------------
//! Food in a player's inventory: items whose prefab name contains a food keyword.
class RZ_FoodHelper
{
	//------------------------------------------------------------------------------------------------
	static void DefaultKeywords(notnull array<string> result)
	{
		result.Clear();
		result.Insert("food");
		result.Insert("canned");
		result.Insert("ration");
		result.Insert("tushonka");
		result.Insert("bread");
		result.Insert("beans");
	}

	//------------------------------------------------------------------------------------------------
	static bool IsFood(IEntity item, array<string> keywords)
	{
		string name = RZ_Util.GetPrefabName(item);
		if (name.IsEmpty())
			return false;

		name.ToLower();
		foreach (string word : keywords)
		{
			if (!word.IsEmpty() && name.Contains(word))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Removes up to `maxCount` food items from the character. Returns how many were taken.
	static int TakeFood(IEntity character, array<string> keywords, int maxCount)
	{
		if (!character || maxCount <= 0)
			return 0;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return 0;

		array<IEntity> items = {};
		inv.GetItems(items);
		int taken = 0;
		foreach (IEntity item : items)
		{
			if (taken >= maxCount)
				break;

			if (!item || !IsFood(item, keywords))
				continue;

			inv.TryDeleteItem(item);
			taken++;
		}

		return taken;
	}
}

//------------------------------------------------------------------------------------------------
//! The Haven safehouse network: creation, population, upkeep, house rules and the
//! bodies near players. Owned by the game mode.
class RZ_Safehouses
{
	static const int POP_CAP = 30;
	static const int UPKEEP_FLOOR = 2;			// upkeep never empties a safehouse below this
	static const int TAG_OCCUPANT = -3;			// RZ_PuzzleObjectComponent mission id for occupants
	protected static const float MIN_SPACING = 1200;
	protected static const float INSIDE_RADIUS = 40;
	protected static const float CHECK_INTERVAL = 60;
	protected static const float LEAK_INTERVAL = 600;
	protected static const float LEAK_COOLDOWN = 1800;
	protected static const float FIRE_COOLDOWN = 20;
	protected static const float DESPAWN_MARGIN = 50;
	protected static const float RESCUE_WANDERER_CHANCE = 0.1;
	protected static const int STRAGGLER_CAP = 12;			// stragglers alone bring a safehouse up to a Settlement

	protected RZ_GameModeComponent m_GM;
	protected RZ_SafehouseSettings m_Settings;
	protected ref array<ref RZ_Safehouse> m_aSafehouses = {};
	protected ref array<ref RZ_SafehouseSave> m_aRestored = {};
	protected ref array<string> m_aFoodWords = {};
	protected ref map<int, float> m_mFireAt = new map<int, float>();
	protected ref map<string, float> m_mLeakAt = new map<string, float>();
	protected ref RZ_NoticeBoard m_Board;
	protected float m_fClock;
	protected float m_fCheckTimer;
	protected float m_fLeakTimer;

	//------------------------------------------------------------------------------------------------
	void RZ_Safehouses(RZ_GameModeComponent gm, RZ_SafehouseSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		if (m_Settings.m_aFoodKeywords && !m_Settings.m_aFoodKeywords.IsEmpty())
		{
			foreach (string w : m_Settings.m_aFoodKeywords)
			{
				string lower = w;
				lower.ToLower();
				m_aFoodWords.Insert(lower);
			}
		}
		else
		{
			RZ_FoodHelper.DefaultKeywords(m_aFoodWords);
		}

		m_Board = new RZ_NoticeBoard(gm, settings, this);
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled() { return m_Settings.m_bEnabled; }
	int Count() { return m_aSafehouses.Count(); }
	RZ_NoticeBoard GetBoard() { return m_Board; }
	array<string> GetFoodKeywords() { return m_aFoodWords; }
	//! People one food unit feeds at an upkeep (Haven > People per food).
	int PeoplePerFood() { return m_Settings.m_iPeoplePerFood; }

	//------------------------------------------------------------------------------------------------
	RZ_Safehouse Get(int idx)
	{
		if (idx < 0 || idx >= m_aSafehouses.Count())
			return null;

		return m_aSafehouses[idx];
	}

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	void Restore(array<ref RZ_SafehouseSave> saved)
	{
		m_aRestored.Clear();
		if (!saved)
			return;

		foreach (RZ_SafehouseSave s : saved)
		{
			m_aRestored.Insert(s);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Save(notnull array<ref RZ_SafehouseSave> result)
	{
		foreach (RZ_Safehouse h : m_aSafehouses)
		{
			result.Insert(h.ToSave());
		}
	}

	// ================================================================ Setup
	//------------------------------------------------------------------------------------------------
	//! After settlements, the Perimeter and the quartermasters exist. Placed markers win;
	//! otherwise the saved safehouses; otherwise N quiet villages are picked now.
	void Setup()
	{
		if (!m_Settings.m_bEnabled)
		{
			RZ_Log.Info("Haven safehouses are off.");
			return;
		}

		string source = "picked";
		array<RZ_SafehouseMarkerEntity> markers = RZ_SafehouseMarkerEntity.GetAll();
		if (!markers.IsEmpty())
		{
			source = "markers";
			foreach (RZ_SafehouseMarkerEntity marker : markers)
			{
				AddFromMarker(marker);
			}
		}
		else if (!m_aRestored.IsEmpty())
		{
			source = "save";
			foreach (RZ_SafehouseSave s : m_aRestored)
			{
				m_aSafehouses.Insert(RZ_Safehouse.FromSave(s));
			}
		}
		else
		{
			PickSafehouses();
		}

		m_aRestored.Clear();
		foreach (RZ_Safehouse h : m_aSafehouses)
		{
			h.m_SpawnPoint = m_GM.SpawnNamedSpawnPoint(RZ_Mission.LandPointNear(h.m_vPos, 6, 14), "Haven safehouse, " + h.m_sName);
		}

		RZ_Log.Info(string.Format("Haven safehouses: %1 (%2).", m_aSafehouses.Count(), source));
	}

	//------------------------------------------------------------------------------------------------
	protected void AddFromMarker(RZ_SafehouseMarkerEntity marker)
	{
		RZ_Safehouse h = new RZ_Safehouse();
		h.m_vPos = RZ_Util.GroundPos(marker.GetOrigin());
		h.m_iSettlement = m_GM.FindNearestSettlement(h.m_vPos);
		h.m_sName = marker.GetMarkerName();
		RZ_Settlement s = m_GM.GetSettlement(h.m_iSettlement);
		if (h.m_sName.IsEmpty() && s)
			h.m_sName = s.m_sName;
		if (h.m_sName.IsEmpty())
			h.m_sName = RZ_SettlementScanner.GridReference(h.m_vPos);

		h.m_iPopulation = Math.ClampInt(marker.GetStartPopulation(), 0, POP_CAP);
		h.m_iFood = Math.Max(0, marker.GetStartFood());

		// Keep the numbers the saved safehouse at this spot had
		foreach (RZ_SafehouseSave saved : m_aRestored)
		{
			if (RZ_Util.DistXZ(Vector(saved.x, 0, saved.z), h.m_vPos) > 100)
				continue;

			h.m_iPopulation = Math.ClampInt(saved.population, 0, POP_CAP);
			h.m_iFood = Math.Max(0, saved.food);
			h.m_fUpkeepTimer = saved.upkeep;
			h.m_bShort = saved.shortFood;
			if (saved.notes)
			{
				foreach (RZ_Note sn : saved.notes)
				{
					if (sn)
						h.m_aNotes.Insert(sn);
				}
			}
		}

		vector qm;
		if (m_GM.GetPerks().GetQuartermasterPos(RZ_EFaction.SURVIVORS, qm) && RZ_Util.DistXZ(qm, h.m_vPos) < 60)
			h.m_bAnchored = true;

		m_aSafehouses.Insert(h);
	}

	//------------------------------------------------------------------------------------------------
	//! The first safehouse sits at the Haven quartermaster (if there is one). The rest
	//! go to quiet villages: small, low infection, away from the lab, the line and each other.
	protected void PickSafehouses()
	{
		int wanted = Math.Max(1, Math.Min(m_Settings.m_iCount, 1 + m_GM.GetSettlements().Count() / 8));

		// Season rules (the Haven's season): one or two more, never more than one per 5 settlements
		int bonus = RZ_Seasons.Mods().m_iSafehouseBonus;
		if (bonus > 0)
			wanted = Math.Min(wanted + bonus, Math.Max(wanted, m_GM.GetSettlements().Count() / 5));

		vector qm;
		if (m_GM.GetPerks().GetQuartermasterPos(RZ_EFaction.SURVIVORS, qm))
		{
			RZ_Safehouse first = NewSafehouse(m_GM.FindNearestSettlement(qm), qm);
			first.m_bAnchored = true;
		}

		int labTown = m_GM.GetLabSettlement();
		for (int pass = 0; pass < 2 && m_aSafehouses.Count() < wanted; pass++)
		{
			// Pass 0: villages only, infection <= 40. Pass 1: any size, infection <= 60.
			array<int> candidates = {};
			foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
			{
				if (i == labTown || IsSafehouseTown(i))
					continue;

				if (pass == 0 && (s.m_iSize > 1 || s.m_fInfection > 40))
					continue;

				if (pass == 1 && s.m_fInfection > 60)
					continue;

				if (m_GM.GetPerimeter().IsNearLine(s.m_vCenter) || IsNearSafehouse(s.m_vCenter, MIN_SPACING))
					continue;

				candidates.Insert(i);
			}

			SortByInfection(candidates);
			foreach (int idx : candidates)
			{
				if (m_aSafehouses.Count() >= wanted)
					break;

				RZ_Settlement town = m_GM.GetSettlement(idx);
				if (IsNearSafehouse(town.m_vCenter, MIN_SPACING))
					continue;

				NewSafehouse(idx, RZ_Mission.LandPointNear(town.m_vCenter, 10, Math.Max(25, town.m_fRadius * 0.4)));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Safehouse NewSafehouse(int settlementIdx, vector pos)
	{
		RZ_Safehouse h = new RZ_Safehouse();
		h.m_iSettlement = settlementIdx;
		h.m_vPos = RZ_Util.GroundPos(pos);
		RZ_Settlement s = m_GM.GetSettlement(settlementIdx);
		if (s)
			h.m_sName = s.m_sName;
		else
			h.m_sName = RZ_SettlementScanner.GridReference(pos);

		h.m_iPopulation = Math.ClampInt(m_Settings.m_iStartPopulation, 0, POP_CAP);
		h.m_iFood = Math.Max(0, m_Settings.m_iStartFood);
		m_aSafehouses.Insert(h);
		return h;
	}

	//------------------------------------------------------------------------------------------------
	//! Insertion sort, lowest infection first (the list is short).
	protected void SortByInfection(array<int> list)
	{
		for (int i = 1; i < list.Count(); i++)
		{
			int key = list[i];
			float keyInf = m_GM.GetSettlement(key).m_fInfection;
			int j = i - 1;
			while (j >= 0 && m_GM.GetSettlement(list[j]).m_fInfection > keyInf)
			{
				list[j + 1] = list[j];
				j--;
			}

			list[j + 1] = key;
		}
	}

	//------------------------------------------------------------------------------------------------
	bool IsSafehouseTown(int settlementIdx)
	{
		foreach (RZ_Safehouse h : m_aSafehouses)
		{
			if (h.m_iSettlement == settlementIdx)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsNearSafehouse(vector pos, float dist)
	{
		foreach (RZ_Safehouse h : m_aSafehouses)
		{
			if (RZ_Util.DistXZ(h.m_vPos, pos) < dist)
				return true;
		}

		return false;
	}

	// ================================================================ Queries
	//------------------------------------------------------------------------------------------------
	//! Nearest safehouse within maxDist of pos, -1 if none.
	int FindNearest(vector pos, float maxDist = 1000000)
	{
		int best = -1;
		float bestDist = maxDist;
		foreach (int i, RZ_Safehouse h : m_aSafehouses)
		{
			float d = RZ_Util.DistXZ(pos, h.m_vPos);
			if (d <= bestDist)
			{
				bestDist = d;
				best = i;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! The safehouse pos is inside of (or right next to), -1 if none.
	int FindAt(vector pos)
	{
		return FindNearest(pos, INSIDE_RADIUS + 20);
	}

	//------------------------------------------------------------------------------------------------
	//! A safehouse whose people are shown (a player is near) within radius of pos, -1 if none.
	//! New-player guidance uses its occupants as the nearest speaker.
	int FindPeopleNear(vector pos, float radius)
	{
		foreach (int i, RZ_Safehouse h : m_aSafehouses)
		{
			if (h.m_bSpawned && !h.m_aBodies.IsEmpty() && RZ_Util.DistXZ(pos, h.m_vPos) <= radius + 10)
				return i;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Hunted by the Haven = exile: the door stays shut.
	bool IsExiled(int playerId)
	{
		return RZ_Reputation.IsHunted(m_GM.GetTier(playerId, RZ_EFaction.SURVIVORS));
	}

	//------------------------------------------------------------------------------------------------
	//! Highest Haven shop tier (access rank) the quartermaster at pos may sell. -1 if pos
	//! isn't at a safehouse (a quartermaster placed elsewhere sells everything).
	int GetShopRankCap(vector pos)
	{
		int idx = FindNearest(pos, 80);
		RZ_Safehouse h = Get(idx);
		if (!h)
			return -1;

		return h.ShopRankCap();
	}

	//------------------------------------------------------------------------------------------------
	//! A random safehouse index, -1 if there are none.
	int PickRandom()
	{
		if (m_aSafehouses.IsEmpty())
			return -1;

		return Math.RandomInt(0, m_aSafehouses.Count());
	}

	// ================================================================ Population
	//------------------------------------------------------------------------------------------------
	//! Adds people to the safehouse nearest pos; past the cap they go to the next nearest.
	//! Returns how many found a place.
	int AddPopulation(vector pos, int count)
	{
		int placed = 0;
		for (int n = 0; n < count; n++)
		{
			RZ_Safehouse h = NearestWithRoom(pos);
			if (!h)
				break;

			h.m_iPopulation = h.m_iPopulation + 1;
			placed++;
		}

		return placed;
	}

	//------------------------------------------------------------------------------------------------
	//! Like AddPopulation, but never into safehouse `except` (people moved out of it).
	int AddPopulationExcept(vector pos, int count, int except)
	{
		int placed = 0;
		for (int n = 0; n < count; n++)
		{
			RZ_Safehouse best;
			float bestDist = float.MAX;
			foreach (int i, RZ_Safehouse h : m_aSafehouses)
			{
				if (i == except || h.m_iPopulation >= POP_CAP)
					continue;

				float d = RZ_Util.DistXZ(pos, h.m_vPos);
				if (d < bestDist)
				{
					bestDist = d;
					best = h;
				}
			}

			if (!best)
				break;

			best.m_iPopulation = best.m_iPopulation + 1;
			placed++;
		}

		return placed;
	}

	//------------------------------------------------------------------------------------------------
	//! `count` people join safehouse `idx` itself (a questline's ending), up to the cap. Returns how many.
	int AddPopulationAt(int idx, int count)
	{
		RZ_Safehouse h = Get(idx);
		if (!h || count <= 0)
			return 0;

		int placed = Math.MinInt(count, POP_CAP - h.m_iPopulation);
		if (placed <= 0)
			return 0;

		h.m_iPopulation = h.m_iPopulation + placed;
		return placed;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Safehouse NearestWithRoom(vector pos)
	{
		RZ_Safehouse best;
		float bestDist = float.MAX;
		foreach (RZ_Safehouse h : m_aSafehouses)
		{
			if (h.m_iPopulation >= POP_CAP)
				continue;

			float d = RZ_Util.DistXZ(pos, h.m_vPos);
			if (d < bestDist)
			{
				bestDist = d;
				best = h;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Rescue missions: `count` survivors join the nearest safehouse. About 1 in 10 would
	//! rather walk, and becomes a named wanderer instead. Tells the rescuers.
	void AddRescued(vector pos, int count, array<int> rescuers)
	{
		if (m_aSafehouses.IsEmpty() || count <= 0)
			return;

		int joined = 0;
		string walker = "";

		// The Haven's morale: fewer stay when it's low, more when it's high
		float walkChance = RESCUE_WANDERER_CHANCE;
		if (m_GM.GetFactionLife())
			walkChance = m_GM.GetFactionLife().HavenWalkChance(walkChance);

		for (int n = 0; n < count; n++)
		{
			if (walker.IsEmpty() && Math.RandomFloat01() < walkChance && m_GM.GetWanderers())
			{
				walker = m_GM.GetWanderers().AddFromRescue(pos);
				if (!walker.IsEmpty())
					continue;
			}

			joined += AddPopulation(pos, 1);
		}

		RZ_Safehouse h = Get(FindNearest(pos));
		if (h && joined > 0 && m_GM.GetHeadlines())
			m_GM.GetHeadlines().OnRescue(joined, "the safehouse in " + h.m_sName);

		if (h && joined > 0)
		{
			RZ_WorldWriters.Rescued(h.m_vPos, joined, rescuers);
			h.m_iSeasonRescued = h.m_iSeasonRescued + joined;
		}

		// The hall of fame counts everyone brought in, safehouse or road
		if (m_GM.GetHall() && rescuers)
			m_GM.GetHall().AddRescues(rescuers, count);

		if (!h || !rescuers)
			return;

		string text = string.Format("%1 of them made it to the safehouse in %2. It's home to %3 now (%4).",
			joined, h.m_sName, h.m_iPopulation, RZ_Safehouse.TierName(h.GetTier()));
		if (!walker.IsEmpty())
			text += string.Format("\n%1 thanks you, but won't stay behind walls. You'll see them on the roads.", walker);

		foreach (int playerId : rescuers)
		{
			m_GM.NotifyPlayer(playerId, "The Haven", text);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Someone leaves (a contract took them, the Red Hand came collecting, they moved on). Counts
	//! as lost (m_iLost), not as dead. Never below 0.
	void RemovePopulation(int idx, int count)
	{
		RZ_Safehouse h = Get(idx);
		if (!h)
			return;

		int gone = Math.Min(count, h.m_iPopulation);
		h.m_iPopulation = h.m_iPopulation - gone;
		h.m_iLost = h.m_iLost + gone;
	}

	//------------------------------------------------------------------------------------------------
	//! Someone died: lost and dead (m_iDied), so a line about the buried counts only these.
	void RemoveDead(int idx, int count)
	{
		RZ_Safehouse h = Get(idx);
		if (!h)
			return;

		int dead = Math.Min(count, h.m_iPopulation);
		RemovePopulation(idx, dead);
		h.m_iDied = h.m_iDied + dead;
		h.m_iSeasonDied = h.m_iSeasonDied + dead;
		if (dead > 0)
			RZ_ToneGuard.Death(h.m_vPos, RZ_EFaction.SURVIVORS, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Food taken from the safehouse nearest pos (e.g. Bastion's Requisition). Returns its index.
	int TakeFood(vector pos, int amount)
	{
		int idx = FindNearest(pos, 3000);
		RZ_Safehouse h = Get(idx);
		if (h)
			h.m_iFood = Math.Max(0, h.m_iFood - amount);

		return idx;
	}

	//------------------------------------------------------------------------------------------------
	//! Food delivered from outside the action: a Haven supply truck that parks here
	//! (Factions/RZ_SupplyLines.c), or another mod.
	void AddFood(int idx, int amount)
	{
		RZ_Safehouse h = Get(idx);
		if (h)
			h.m_iFood = h.m_iFood + Math.Max(0, amount);
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (m_aSafehouses.IsEmpty())
			return;

		m_fClock += dt;
		UpdateBodies(players);
		m_Board.Tick(dt);

		// Losses fade: one an hour (the dead among them too)
		foreach (RZ_Safehouse lossHouse : m_aSafehouses)
		{
			if (lossHouse.m_iLost <= 0 && lossHouse.m_iDied <= 0)
				continue;

			lossHouse.m_fLostFade = lossHouse.m_fLostFade + dt;
			if (lossHouse.m_fLostFade >= 3600)
			{
				lossHouse.m_fLostFade = 0;
				lossHouse.m_iLost = Math.Max(0, lossHouse.m_iLost - 1);
				lossHouse.m_iDied = Math.Max(0, lossHouse.m_iDied - 1);
			}
		}

		// Upkeep and stragglers only run while someone is playing, so an empty server doesn't
		// starve; both follow the season speed
		if (!ids.IsEmpty())
		{
			float speed = m_GM.GetSeasonSpeed();
			foreach (int i, RZ_Safehouse h : m_aSafehouses)
			{
				h.m_fUpkeepTimer = h.m_fUpkeepTimer + dt * speed;
				if (h.m_fUpkeepTimer >= m_Settings.m_fUpkeepMinutes * 60)
				{
					h.m_fUpkeepTimer = 0;
					Upkeep(i);
				}

				TickStragglers(h, dt * speed);
			}
		}

		GreetKin(ids, players);

		m_fCheckTimer += dt;
		if (m_fCheckTimer >= CHECK_INTERVAL)
		{
			m_fCheckTimer = 0;
			CheckHiddenBites(ids, players);
		}

		m_fLeakTimer += dt;
		if (m_fLeakTimer >= LEAK_INTERVAL)
		{
			m_fLeakTimer = 0;
			CheckLeaks(ids, players);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Stragglers: a fed safehouse slowly takes in people who find it on their own, up to a
	//! Settlement. Rescues do the rest.
	protected void TickStragglers(RZ_Safehouse h, float dt)
	{
		if (m_Settings.m_fStragglerHours <= 0 || h.m_bShort || h.m_iPopulation >= STRAGGLER_CAP)
		{
			h.m_fStraggleTimer = 0;
			return;
		}

		h.m_fStraggleTimer = h.m_fStraggleTimer + dt;
		if (h.m_fStraggleTimer < m_Settings.m_fStragglerHours * 3600)
			return;

		h.m_fStraggleTimer = 0;
		RZ_ESafehouseTier before = h.GetTier();
		h.m_iPopulation = h.m_iPopulation + 1;
		if (h.GetTier() != before)
			NotifyHaven(string.Format("Someone walked into %1 last night with a bag and a story. It's a %2 now: %3 people.", h.m_sName, RZ_Safehouse.TierName(h.GetTier()), h.m_iPopulation));
	}

	//------------------------------------------------------------------------------------------------
	//! Every few hours the population eats. If there isn't enough, a few people leave:
	//! to a safehouse that can feed them, or onto the roads.
	protected void Upkeep(int idx)
	{
		RZ_Safehouse h = m_aSafehouses[idx];
		int need = h.FoodNeed(m_Settings.m_iPeoplePerFood);

		// A safehouse gone to the Preacher's prayers eats half (story arcs)
		RZ_StoryArcs arcs = m_GM.GetStoryArcs();
		if (arcs && arcs.UpkeepMult(idx) < 1)
			need = Math.Max(1, RZ_Util.RoundToInt(need * arcs.UpkeepMult(idx)));

		// Season rules (Winter): a cold season burns through more
		float seasonMult = RZ_Seasons.Mods().m_fUpkeepMult;
		if (seasonMult > 1)
			need = Math.Max(1, RZ_Util.RoundToInt(need * seasonMult));

		if (h.m_iFood >= need)
		{
			h.m_iFood = h.m_iFood - need;
			h.m_bShort = false;
			RZ_WorldWriters.HouseFood(idx, h.m_sName, h.m_vPos, h.m_iFood < need);
			return;
		}

		RZ_WorldWriters.HouseFood(idx, h.m_sName, h.m_vPos, true);
		int missing = need - h.m_iFood;
		h.m_iFood = 0;
		h.m_bShort = true;

		int leaving = Math.Min(Math.Max(1, missing), 3);
		leaving = Math.Min(leaving, h.m_iPopulation - UPKEEP_FLOOR);
		if (leaving <= 0)
		{
			NotifyHaven(string.Format("The safehouse in %1 has run out of food. The last of them are rationing water.", h.m_sName));
			return;
		}

		h.m_iPopulation = h.m_iPopulation - leaving;
		h.m_iLost = h.m_iLost + leaving;
		int moved = 0;
		int walked = 0;
		for (int n = 0; n < leaving; n++)
		{
			RZ_Safehouse other = FindFedSafehouse(h);
			if (other)
			{
				other.m_iPopulation = other.m_iPopulation + 1;
				moved++;
				continue;
			}

			// One of them may take to the roads as a named wanderer; the rest are just gone
			if (walked == 0 && m_GM.GetWanderers() && !m_GM.GetWanderers().AddFromLeaver(h.m_vPos).IsEmpty())
				walked++;
		}

		// The ones who were just gone are alone on the roads (the Long Table may meet one of them)
		if (m_GM.GetLongTable() && leaving - moved - walked > 0)
			m_GM.GetLongTable().OnLeftOntoRoads(h.m_vPos, h.m_sName, leaving - moved - walked);

		NotifyHaven(string.Format("The safehouse in %1 ran out of food. %2 people left (%3 to other safehouses). %4 remain. Bring food.",
			h.m_sName, leaving, moved, h.m_iPopulation));
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Safehouse FindFedSafehouse(RZ_Safehouse from)
	{
		RZ_Safehouse best;
		float bestDist = float.MAX;
		foreach (RZ_Safehouse h : m_aSafehouses)
		{
			if (h == from || h.m_iPopulation >= POP_CAP || h.m_iFood <= h.FoodNeed(m_Settings.m_iPeoplePerFood))
				continue;

			float d = RZ_Util.DistXZ(from.m_vPos, h.m_vPos);
			if (d < bestDist)
			{
				bestDist = d;
				best = h;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Players Known or better with the Haven hear safehouse news.
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

	// ================================================================ Bodies near players
	//------------------------------------------------------------------------------------------------
	protected void UpdateBodies(array<IEntity> players)
	{
		foreach (RZ_Safehouse h : m_aSafehouses)
		{
			float nearest = RZ_Util.NearestDistance(h.m_vPos, players);
			if (h.m_bSpawned && (nearest > m_Settings.m_fVisibleRadius + DESPAWN_MARGIN || h.m_iSpawnedTier != h.GetTier()))
				DespawnBodies(h);

			if (!h.m_bSpawned && nearest <= m_Settings.m_fVisibleRadius)
				SpawnBodies(h);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnBodies(RZ_Safehouse h)
	{
		h.m_bSpawned = true;
		h.m_iSpawnedTier = h.GetTier();
		int idx = m_aSafehouses.Find(h);
		// Camp life gives them jobs and dresses the house (RZ_CampLife)
		array<IEntity> guardBodies = {};
		array<IEntity> personBodies = {};
		array<IEntity> keeperBodies = {};

		for (int i = 0; i < h.OccupantCount(); i++)
		{
			IEntity occupant = RZ_Util.SpawnPrefab(PickOccupantPrefab(), RZ_Mission.LandPointNear(h.m_vPos, 3, 14), Math.RandomFloat(0, 360));
			if (occupant)
			{
				TagOccupant(occupant, idx);
				h.m_aBodies.Insert(occupant);
				personBodies.Insert(occupant);
			}
		}

		// The house's named people wear the first two occupants (World/RZ_NamedPeople.c)
		if (m_GM.GetErrands() && m_GM.GetErrands().IsOn())
			m_GM.GetErrands().GetPeople().DressHouse(idx, personBodies);

		for (int g = 0; g < h.GuardCount(); g++)
		{
			IEntity guard = RZ_Util.SpawnPrefab(m_Settings.m_sGuardPrefab, RZ_Mission.LandPointNear(h.m_vPos, 15, 30), Math.RandomFloat(0, 360));
			if (guard)
			{
				h.m_aBodies.Insert(guard);
				guardBodies.Insert(guard);

				// Shooting at a safehouse guard turns the Haven's people there on you for a while
				if (m_GM.GetHostility())
					m_GM.GetHostility().WatchUnit(guard, RZ_EFaction.SURVIVORS);
			}
		}

		// The safehouse's own quartermaster (the anchored one already has the real one)
		if (!h.m_bAnchored)
		{
			IEntity qm = RZ_Util.SpawnPrefab(m_GM.GetQuartermasterPrefab(RZ_EFaction.SURVIVORS), RZ_Mission.LandPointNear(h.m_vPos, 1, 4), Math.RandomFloat(0, 360));
			if (qm)
			{
				h.m_aBodies.Insert(qm);
				keeperBodies.Insert(qm);
			}
		}

		if (h.HasMedic())
		{
			IEntity medic = RZ_Util.SpawnPrefab(m_Settings.m_sMedicPrefab, RZ_Mission.LandPointNear(h.m_vPos, 3, 6), Math.RandomFloat(0, 360));
			if (medic)
			{
				h.m_aBodies.Insert(medic);
				keeperBodies.Insert(medic);
			}
		}

		// Halcyon's flyer on the wall: the Haven clinic, Bastion's curfew, samples wanted
		if (m_GM.GetGuidance())
		{
			IEntity flyer = m_GM.GetGuidance().SpawnSafehouseFlyer(idx, h.m_vPos);
			if (flyer)
				h.m_aBodies.Insert(flyer);
		}

		RZ_CampLife camp = m_GM.GetCampLife();
		if (camp)
			camp.Enter("s" + idx.ToString(), "Haven safehouse", RZ_EFaction.SURVIVORS, RZ_ECampPlace.SAFEHOUSE, h.m_vPos, 0, m_Settings.m_sGuardPrefab, guardBodies, personBodies, keeperBodies);
	}

	//------------------------------------------------------------------------------------------------
	protected void DespawnBodies(RZ_Safehouse h)
	{
		RZ_CampLife camp = m_GM.GetCampLife();
		if (camp)
			camp.Leave("s" + m_aSafehouses.Find(h).ToString());

		foreach (IEntity e : h.m_aBodies)
		{
			RZ_Util.DeleteUnit(e);
		}

		h.m_aBodies.Clear();
		h.m_bSpawned = false;
		h.m_iSpawnedTier = -1;
	}

	//------------------------------------------------------------------------------------------------
	protected ResourceName PickOccupantPrefab()
	{
		if (m_Settings.m_aOccupantPrefabs && !m_Settings.m_aOccupantPrefabs.IsEmpty())
			return m_Settings.m_aOccupantPrefabs.GetRandomElement();

		return m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);
	}

	//------------------------------------------------------------------------------------------------
	//! If the occupant prefab has the generic puzzle object components, give it one
	//! labelled choice ("Ask how things are") and a "Talk" read, both routed back here
	//! (RZ_PuzzleActionHelper). Prefabs without them are just scenery.
	static void TagOccupant(IEntity e, int safehouseIdx)
	{
		RZ_PuzzleObjectComponent.Tag(e, TAG_OCCUPANT, safehouseIdx, "", "Ask how things are");
		RZ_ClueComponent.Tag(e, TAG_OCCUPANT, safehouseIdx, "Talk to them");
	}

	// ================================================================ House rules
	//------------------------------------------------------------------------------------------------
	//! From the gunfire hook. Drawing a weapon is fine; firing in or around a safehouse
	//! isn't, unless the infected are at the door.
	void OnPlayerShot(int playerId, vector pos)
	{
		int idx = FindNearest(pos, m_Settings.m_fNoFireRadius);
		RZ_Safehouse h = Get(idx);
		if (!h)
			return;

		if (m_GM.GetSpawner().IsZombieNear(h.m_vPos, m_Settings.m_fDefendRadius))
			return; // defending it

		float last;
		if (m_mFireAt.Find(playerId, last) && m_fClock - last < FIRE_COOLDOWN)
			return;

		m_mFireAt.Set(playerId, m_fClock);
		m_GM.AddInfamy(playerId, RZ_EFaction.SURVIVORS, m_Settings.m_iFireInfamy);
		m_GM.NotifyPlayer(playerId, "The Haven", string.Format("Not here. Children sleep in %1. The Haven has one rule about guns, and you just broke it. (+%2 Infamy)",
			h.m_sName, m_Settings.m_iFireInfamy));
	}

	//------------------------------------------------------------------------------------------------
	//! A bitten player who hasn't said so, inside a safehouse once it shows (Weakness: half the
	//! chance; Final: the full chance), may be caught. Once per life. The golden window, the Fever
	//! stage and a fever suppressant don't show.
	protected void CheckHiddenBites(array<int> ids, array<IEntity> players)
	{
		RZ_Infection infection = m_GM.GetInfection();
		if (!infection)
			return;

		foreach (int i, IEntity p : players)
		{
			RZ_PlayerProfile profile = m_GM.GetProfile(ids[i]);
			if (!profile.IsInfected() || profile.m_bBiteDeclared || profile.m_bBiteCaught)
				continue;

			int shows = infection.VisibleSickness(profile);
			if (shows <= 0)
				continue;

			float chance = m_Settings.m_fCaughtChance;
			if (shows == 1)
				chance = chance * 0.5;

			int idx = FindNearest(p.GetOrigin(), INSIDE_RADIUS);
			RZ_Safehouse h = Get(idx);
			if (!h || Math.RandomFloat01() >= chance)
				continue;

			profile.m_bBiteCaught = true;
			m_GM.AddInfamy(ids[i], RZ_EFaction.SURVIVORS, m_Settings.m_iCaughtInfamy);
			m_GM.NotifyPlayer(ids[i], "The Haven", string.Format("A woman in %1 sees the sweat, the grey in your skin, the way you hold your arm. She doesn't scream. She just gets the children behind her and points at the door. (+%2 Infamy)",
				h.m_sName, m_Settings.m_iCaughtInfamy));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A hungry safehouse sometimes sells what it knows: the rough location of someone
	//! the Haven turned out, to whoever's hunting them.
	protected void CheckLeaks(array<int> ids, array<IEntity> players)
	{
		foreach (RZ_Safehouse h : m_aSafehouses)
		{
			if (!h.m_bShort || Math.RandomFloat01() >= m_Settings.m_fLeakChance)
				continue;

			foreach (int i, IEntity p : players)
			{
				if (!IsExiled(ids[i]))
					continue;

				string ident = m_GM.GetProfile(ids[i]).m_sIdentity;
				float last;
				if (m_mLeakAt.Find(ident, last) && m_fClock - last < LEAK_COOLDOWN)
					continue;

				m_mLeakAt.Set(ident, m_fClock);
				m_GM.GetPerks().LeakLocation(ids[i], p, h.m_sName);
				break; // one name per safehouse per round
			}
		}
	}

	// ================================================================ Actions
	//------------------------------------------------------------------------------------------------
	//! From RZ_SafehouseAction (on the Haven quartermaster and the medic).
	void HandleAction(int playerId, RZ_ESafehouseAction kind, IEntity owner, IEntity user)
	{
		int idx = FindNearest(owner.GetOrigin(), 150);
		RZ_Safehouse h = Get(idx);
		if (!h)
		{
			m_GM.NotifyPlayer(playerId, "The Haven", "There's no safehouse here, only a sign that says there was one.");
			return;
		}

		if (IsExiled(playerId))
		{
			m_GM.Say(playerId, "Behind the door", RZ_EFaction.SURVIVORS, "The door stays shut. Someone on the other side says your name, and then nothing else.");
			return;
		}

		switch (kind)
		{
			case RZ_ESafehouseAction.STATUS: TellStatus(playerId, h); break;
			case RZ_ESafehouseAction.DELIVER_FOOD: DeliverFood(playerId, h, user); break;
			case RZ_ESafehouseAction.HEAL: Heal(playerId, h, user); break;
			case RZ_ESafehouseAction.DECLARE_BITE: DeclareBite(playerId, h); break;
			case RZ_ESafehouseAction.RADIO: Radio(playerId, h, user); break;
			case RZ_ESafehouseAction.MEDS: SellMeds(playerId, h, owner, user); break;
			case RZ_ESafehouseAction.BOARD_READ: m_Board.Read(playerId, idx); break;
			case RZ_ESafehouseAction.BOARD_WRITE: m_Board.HowToWrite(playerId, idx); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! From an occupant's "Ask how things are".
	//! A conversation's hand-in (Dialogue > Trees, call:hand_in:N): the built food delivery at this
	//! house, for at most `count` items, paid as built.
	void HandInFood(int playerId, int idx, IEntity user, int count)
	{
		RZ_Safehouse h = Get(idx);
		if (h && user)
			DeliverFood(playerId, h, user, Math.Max(1, count));
	}

	//------------------------------------------------------------------------------------------------
	void HandleOccupantTalk(int playerId, int idx)
	{
		RZ_Safehouse h = Get(idx);
		if (!h)
			return;

		if (IsExiled(playerId))
		{
			m_GM.Say(playerId, "The Haven", RZ_EFaction.SURVIVORS, "They look straight through you. A man steps between you and the children.");
			return;
		}

		// Colours: not in red, and not in a ribbon given to someone else (RZ_Disguise)
		if (m_GM.GetHostility())
		{
			string kit = m_GM.GetHostility().GetDisguise().TalkRefusal(playerId, true, false);
			if (!kit.IsEmpty())
			{
				m_GM.Say(playerId, "The Haven", RZ_EFaction.SURVIVORS, kit);
				return;
			}
		}

		TellStatus(playerId, h);

		// They stop what they're doing while you talk
		RZ_CampLife camp = m_GM.GetCampLife();
		if (camp)
			camp.HoldPersonNear(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void TellStatus(int playerId, RZ_Safehouse h)
	{
		int need = h.FoodNeed(m_Settings.m_iPeoplePerFood);
		int meals = h.m_iFood / need;
		string mood = "\"We're managing. Don't tell anyone I said that.\"";
		if (h.m_iFood < need)
			mood = "\"There's nothing left in the stores. People are talking about leaving.\"";
		else if (meals <= 1)
			mood = "\"We've food for one more round, maybe. If you find tins, bring them here.\"";

		string services = "a bed and a spawn point";
		if (h.GuardCount() > 0)
			services += string.Format(", %1 on watch", h.GuardCount());
		if (h.HasMedic())
			services += ", a medic, a radio";

		// Who runs it (docs/REPUTATION.md, "Faction leaders")
		RZ_Leaders leaders = m_GM.GetLeaders();
		if (leaders)
		{
			RZ_Leader head = leaders.FindAt(RZ_EFaction.SURVIVORS, h.m_vPos);
			if (head)
				services += string.Format(". %1 runs the place", head.m_sName);
		}

		RZ_StoryArcs arcs = m_GM.GetStoryArcs();
		if (arcs)
		{
			string prayer = arcs.HouseLine(m_aSafehouses.Find(h));
			if (!prayer.IsEmpty())
				mood = prayer;
		}

		// What's wrong this shift (the faults table): the generator, the barrels
		if (m_GM.GetErrands())
		{
			string fault = m_GM.GetErrands().GetFaults().HouseLine(m_aSafehouses.Find(h));
			if (!fault.IsEmpty())
				mood += "\n" + fault;
		}

		// The epilogue week: a resident's line about the season, in the first person
		if (RZ_Epilogue.Get())
		{
			string week = RZ_Epilogue.Get().ResidentLine(m_aSafehouses.Find(h));
			if (!week.IsEmpty())
				mood = week;
		}

		m_GM.NotifyPlayer(playerId, "Safehouse: " + h.m_sName, string.Format("%1 people live here (%2). Food for %3 more upkeeps.\nThey have %4.\n%5",
			h.m_iPopulation, RZ_Safehouse.TierName(h.GetTier()), meals, services, mood));
	}

	//------------------------------------------------------------------------------------------------
	protected void DeliverFood(int playerId, RZ_Safehouse h, IEntity user, int maxCount = 10)
	{
		int taken = RZ_FoodHelper.TakeFood(user, m_aFoodWords, maxCount);
		if (taken <= 0)
		{
			m_GM.NotifyPlayer(playerId, "Safehouse: " + h.m_sName, "You've nothing they can eat. Tins, rations, bread: anything helps.");
			return;
		}

		h.m_iFood = h.m_iFood + taken;
		int favor = taken * m_Settings.m_iFavorPerFood;
		m_GM.AddFavor(playerId, RZ_EFaction.SURVIVORS, favor);
		int fame = Math.Min(5, taken / 3);
		if (fame > 0)
			m_GM.AddFame(playerId, RZ_EFaction.SURVIVORS, fame);

		m_GM.NotifyPlayer(playerId, "Safehouse: " + h.m_sName, string.Format("You hand over %1 bits of food. Someone writes it down in a school exercise book. +%2 Favor. The stores hold %3 now.",
			taken, favor, h.m_iFood));

		if (m_GM.GetStoryArcs())
			m_GM.GetStoryArcs().OnFoodDelivered(m_aSafehouses.Find(h), taken);
	}

	//------------------------------------------------------------------------------------------------
	protected void Heal(int playerId, RZ_Safehouse h, IEntity user)
	{
		if (!h.HasMedic())
		{
			m_GM.NotifyPlayer(playerId, "Safehouse: " + h.m_sName, "There's no medic here. Someone gives you a clean rag and an apology.");
			return;
		}

		if (!m_GM.SpendFavor(playerId, RZ_EFaction.SURVIVORS, m_Settings.m_iHealCost))
		{
			m_GM.NotifyPlayer(playerId, "Safehouse: " + h.m_sName, string.Format("The medic needs %1 Favor for the supplies. You have %2.", m_Settings.m_iHealCost,
				m_GM.GetProfile(playerId).GetFavor(RZ_EFaction.SURVIVORS)));
			return;
		}

		RZ_Util.HealCharacter(user);
		m_GM.NotifyPlayer(playerId, "Safehouse: " + h.m_sName, "The medic cleans you up and doesn't ask where you got it. She never does.");
	}

	//------------------------------------------------------------------------------------------------
	//! Time-left multiplier for a declared bite here: the bigger the house, the more it can do.
	protected float DeclareMult(RZ_Safehouse h)
	{
		switch (h.GetTier())
		{
			case RZ_ESafehouseTier.SHELTER: return m_Settings.m_fDeclareShelter;
			case RZ_ESafehouseTier.COMMUNITY: return m_Settings.m_fDeclareCommunity;
			case RZ_ESafehouseTier.SETTLEMENT: return m_Settings.m_fDeclareSettlement;
		}

		return m_Settings.m_fDeclareTown;
	}

	//------------------------------------------------------------------------------------------------
	//! Telling the truth about a bite, at any safehouse: once per life it's slowed (x1.15 at a
	//! Shelter up to x1.5 at a Town), and you can't be caught hiding that bite. A fresh bite (the
	//! golden window) is washed out properly instead, which may stop it. A bite that's been cleaned
	//! already uses nothing: come back if it takes.
	protected void DeclareBite(int playerId, RZ_Safehouse h)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		RZ_Infection infection = m_GM.GetInfection();
		string title = "Safehouse: " + h.m_sName;

		// A clean that worked but isn't told yet sounds like any cleaned bite
		bool pending = infection && !profile.IsInfected() && infection.SeemsInfected(playerId);
		if (!profile.IsInfected() && !pending)
		{
			m_GM.NotifyPlayer(playerId, title, "\"You're not bitten. Good. Keep it that way.\"");
			return;
		}

		if (profile.m_bBiteDeclared)
		{
			m_GM.NotifyPlayer(playerId, title, "\"We know. We've done what we can. Sit by the door, where it's cooler.\"");
			return;
		}

		if (pending || profile.IsWoundFresh())
		{
			if (!pending && infection && infection.CleanAtSafehouse(playerId))
			{
				// Only a bite still there afterwards stays declared (a clean that worked ends the bite)
				if (profile.IsInfected())
					profile.m_bBiteDeclared = true;

				m_GM.NotifyPlayer(playerId, title, "\"Just now? Sit. Bite your sleeve.\" Boiled water, a brush, the last of the iodine, and a clean dressing. \"If it was going to work, that did it.\"");
			}
			else
			{
				m_GM.NotifyPlayer(playerId, title, "\"It's been cleaned already. Then we wait, and we hope. Come back if it takes.\" Someone puts a blanket round you.");
			}

			return;
		}

		profile.m_bBiteDeclared = true;
		// A bite declared here: the house has a bitten resident today (errands' Sit With Him, Masks Off)
		RZ_WorldFacts facts = m_GM.GetFacts();
		int houseIdx = m_aSafehouses.Find(h);
		if (facts && houseIdx >= 0)
			facts.Memory().PostReason("house.bitten", RZ_WorldFacts.Safehouse(houseIdx), string.Format("Someone at %1 declared a bite.", h.m_sName), h.m_vPos, RZ_EFaction.SURVIVORS, 1);

		if (profile.m_bDeclareSlowed)
		{
			m_GM.NotifyPlayer(playerId, title, "\"Again? We've nothing left that would help, not twice. You can stay. Sit by the door, where it's cooler.\"");
			return;
		}

		profile.m_bDeclareSlowed = true;
		profile.m_fInfectionLeft = profile.m_fInfectionLeft * DeclareMult(h);
		int minutes = RZ_Util.RoundToInt(profile.m_fInfectionLeft / 60);
		if (h.HasMedic())
		{
			m_GM.NotifyPlayer(playerId, title, string.Format("You tell the medic. She doesn't flinch. Something cold in a needle, a clean dressing. \"It'll buy you time. About %1 minutes. Use it well.\"", minutes));
			return;
		}

		m_GM.NotifyPlayer(playerId, title, string.Format("There's no medic here, just a first-aid box and a woman who used to teach biology. She does what she can, and it's not nothing. \"About %1 minutes. Go to the lab, love. They run a trial for people like you.\"", minutes));
	}

	//------------------------------------------------------------------------------------------------
	//! Low-grade meds at every safehouse: a dose of expired antibiotics for a little Favor.
	protected void SellMeds(int playerId, RZ_Safehouse h, IEntity owner, IEntity user)
	{
		string title = "Safehouse: " + h.m_sName;
		RZ_Infection infection = m_GM.GetInfection();
		ResourceName prefab;
		if (infection)
			prefab = infection.MedPrefab(RZ_EMed.ANTIBIOTICS);

		if (prefab.IsEmpty())
		{
			m_GM.NotifyPlayer(playerId, title, "\"Aspirin and a kind word. That's the medicine cabinet.\"");
			return;
		}

		if (!m_GM.SpendFavor(playerId, RZ_EFaction.SURVIVORS, m_Settings.m_iMedsCost))
		{
			m_GM.NotifyPlayer(playerId, title, string.Format("\"We need %1 Favor for it. Medicine's the one thing we can't grow.\" You have %2.", m_Settings.m_iMedsCost,
				m_GM.GetProfile(playerId).GetFavor(RZ_EFaction.SURVIVORS)));
			return;
		}

		if (!RZ_QuartermasterHelper.SpawnNear(prefab, owner, user))
		{
			m_GM.AddFavor(playerId, RZ_EFaction.SURVIVORS, m_Settings.m_iMedsCost); // refund
			return;
		}

		m_GM.NotifyPlayer(playerId, title, "A strip of antibiotics two years out of date, from a shoebox under the stairs. \"It's that or nothing. It might buy you a little time. It'll make you sick.\"");
	}

	// ================================================================ Haven Champions (docs/REPUTATION.md)
	//------------------------------------------------------------------------------------------------
	//! Safe Harbour: a Haven Champion inside any safehouse. Hunter squads lose the trail and bounty
	//! notices only give a vague area (RZ_FactionPerks).
	bool IsSafeHarbour(int playerId, vector pos)
	{
		if (FindAt(pos) < 0 || IsExiled(playerId))
			return false;

		return RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, RZ_EFaction.SURVIVORS), RZ_ETier.CHAMPION);
	}

	//------------------------------------------------------------------------------------------------
	//! Remembered: a Haven Champion died. A memorial headline, the nearest safehouse's council reads
	//! the name out, and the player's next character is kin (one free treatment, a greeting).
	void OnChampionFallen(int playerId, vector pos)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		string place = RZ_SettlementScanner.GridReference(pos);
		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(pos));
		if (s)
			place = s.m_sName;

		profile.m_bFallenPending = true;
		profile.m_sFallenPlace = place;

		string name = RZ_Util.PlayerName(playerId);
		RZ_Leaders leaders = m_GM.GetLeaders();
		if (leaders)
			leaders.AddHeadline(string.Format("The Haven lost %1 near %2. Every safehouse lit a candle in the window last night.", name, place));

		RZ_Safehouse h = Get(FindNearest(pos));
		if (!h)
			return;

		string reader = "The council";
		if (leaders)
		{
			RZ_Leader head = leaders.FindAt(RZ_EFaction.SURVIVORS, h.m_vPos);
			if (head)
				reader = head.m_sName;
		}

		NotifyHaven(string.Format("%1 read the name out at supper in %2: %3, who stood up for the Haven when it counted. Nobody ate for a minute.", reader, h.m_sName, name));
	}

	//------------------------------------------------------------------------------------------------
	//! Kin of a fallen Haven Champion, at their first safehouse this life: the greeting and one
	//! free treatment (a heal; a bite is slowed as if declared at a Town, which also counts as declaring it).
	protected void GreetKin(array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity p : players)
		{
			RZ_PlayerProfile profile = m_GM.GetProfile(ids[i]);
			if (!profile.m_bFallenKin || profile.m_bKinGreeted || IsExiled(ids[i]))
				continue;

			RZ_Safehouse h = Get(FindAt(p.GetOrigin()));
			if (!h)
				continue;

			profile.m_bKinGreeted = true;
			RZ_Util.HealCharacter(p);
			string extra = "";
			if (profile.HasInfectionTaken() && !profile.m_bBiteDeclared && !profile.m_bDeclareSlowed)
			{
				profile.m_bBiteDeclared = true;
				profile.m_bDeclareSlowed = true;
				profile.m_fInfectionLeft = profile.m_fInfectionLeft * m_Settings.m_fDeclareTown;
				extra = string.Format(" She sees the bite and doesn't say a word about it, just works. About %1 minutes, now.", RZ_Util.RoundToInt(profile.m_fInfectionLeft / 60));
			}

			string where = profile.m_sFallenPlace;
			if (where.IsEmpty())
				where = "the road";

			m_GM.NotifyPlayer(ids[i], "Safehouse: " + h.m_sName, string.Format("Someone at the door looks twice at you. \"You're family of the one we lost near %1, aren't you. You have the look.\" They sit you down, feed you, and the medic sees to you for nothing.%2", where, extra));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Radio(int playerId, RZ_Safehouse h, IEntity user)
	{
		if (!h.HasRadio())
		{
			m_GM.NotifyPlayer(playerId, "Safehouse: " + h.m_sName, "No radio here yet. A bigger safehouse would have one.");
			return;
		}

		string text = "";
		int n = 0;
		foreach (RZ_Mission m : m_GM.GetMissions().GetActive())
		{
			if (n >= 4 || m.m_bGlobal || m.m_bInPersonOnly || RZ_Util.DistXZ(m.m_vPos, h.m_vPos) > 3000)
				continue;

			string who = "anyone";
			if (m.m_bContract)
				who = RZ_Factions.GetShortName(m.m_Req.m_eGiver);

			text += string.Format("%1 (%2): %3\n", m.m_sTitle, who, m.GetWhereTextFor(m_GM, user.GetOrigin(), playerId));
			n++;
		}

		if (text.IsEmpty())
			text = "Static, mostly. Nobody nearby is asking for anything.";

		m_GM.NotifyPlayer(playerId, "Safehouse radio: " + h.m_sName, text);
	}
}

//------------------------------------------------------------------------------------------------
//! A safehouse service. Put one per kind on the Haven quartermaster prefab (STATUS,
//! DELIVER_FOOD, RADIO, HEAL, DECLARE_BITE, MEDS, BOARD_READ, BOARD_WRITE) and HEAL +
//! DECLARE_BITE on the medic prefab.
//! It works at whichever safehouse it stands in; elsewhere it says there's none.
class RZ_SafehouseAction : ScriptedUserAction
{
	[Attribute("0", UIWidgets.ComboBox, "What this action does", "", ParamEnumArray.FromEnum(RZ_ESafehouseAction))]
	protected RZ_ESafehouseAction m_eKind;

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		switch (m_eKind)
		{
			case RZ_ESafehouseAction.STATUS: outName = "Ask how things are"; break;
			case RZ_ESafehouseAction.DELIVER_FOOD: outName = "Hand over food"; break;
			case RZ_ESafehouseAction.HEAL: outName = "Ask the medic to patch you up"; break;
			case RZ_ESafehouseAction.DECLARE_BITE: outName = "Tell them you've been bitten"; break;
			case RZ_ESafehouseAction.RADIO: outName = "Listen to the safehouse radio"; break;
			case RZ_ESafehouseAction.MEDS: outName = "Ask for something for the fever"; break;
			case RZ_ESafehouseAction.BOARD_READ: outName = "Read the notice board"; break;
			case RZ_ESafehouseAction.BOARD_WRITE: outName = "Leave a note on the board"; break;
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
		if (!gm || playerId <= 0 || !gm.GetSafehouses())
			return;

		gm.GetSafehouses().HandleAction(playerId, m_eKind, pOwnerEntity, pUserEntity);
	}
}
