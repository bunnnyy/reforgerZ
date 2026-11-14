//! A character who lasts (docs/DESIGN.md, "A character who lasts"): a death costs what you carried
//! and nothing else. The character keeps its background, standing, quests and memory until the
//! player resets it. At a death the quest items come off the body as data (RZ_PlayerProfile.m_aQi*)
//! and go back in the pack on coming round; the character comes round at the nearest camp that will
//! take them in (a holding or a safehouse at Stranger or better, the lab at Known), else at a random
//! start (the openings' place rules, without the opening). Reset character ends one, once a real day.
//! Server only.
class RZ_Character
{
	static const int TAKEN_IN_MAX = 3;			// a camp takes the same character in this many times ...
	static const int TAKEN_IN_WINDOW = 3600;	// ... in this many real seconds, then "We're not your hospital."
	static const int RESET_EVERY = 86400;		// one reset a real day
	static const float HOME_REACH = 120;		// a camp's people within this of its holding: come round in their yard
	static const int NO_MISSION = -2147;		// a held item with no mission id
	static const int WATCHED_INFAMY = 15;		// Infamy's "some" band (RZ_Reputation.Band) ...
	static const int WATCHED_FAME_BELOW = 15;	// ... with Fame in its "low" band: Watched

	protected static ref map<int, bool> s_mMoved;		// the spawn handler moved this player's character to a camp
	protected static ref map<int, string> s_mCamp;		// ... and the camp's name, for the line
	protected static ref map<int, int> s_mCampFaction;	// ... and its people's faction

	// ================================================================ Death
	//------------------------------------------------------------------------------------------------
	//! A player died (RZ_FactionPerks.OnPlayerDied, after everyone else has heard of it): the quest
	//! items come off the body as data, and the bite goes with the body. Nothing else changes.
	static void OnDied(RZ_GameModeComponent gm, int playerId, IEntity body, vector at)
	{
		RZ_PlayerProfile profile = gm.GetProfile(playerId);
		if (!profile)
			return;

		bool bitten = profile.m_fInfectionLeft >= 0 || profile.m_fWoundLeft > 0;
		int held = HoldQuestItems(profile, body);
		profile.OnDeath(bitten, at);
		RZ_Log.Info(string.Format("Character: %1 died (death %2 of character %3)%4; %5 quest items held for coming round",
			RZ_Util.PlayerName(playerId), profile.m_iDeaths, profile.m_iCharacter, BittenText(bitten), held));
	}

	//------------------------------------------------------------------------------------------------
	protected static string BittenText(bool bitten)
	{
		if (bitten)
			return ", bitten";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Every quest item on the body (a job's own item, RZ_MissionItemComponent, or a job's goods,
	//! RZ_MissionGoods) is kept as data and taken off the body, so nobody can loot it. The runner's
	//! case keeps itself (Guidance/RZ_OpeningCase.c). Returns how many.
	static int HoldQuestItems(RZ_PlayerProfile profile, IEntity body)
	{
		if (!body)
			return 0;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(body.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return 0;

		profile.EnsureArrays();
		array<IEntity> items = {};
		inv.GetItems(items);
		int n = 0;
		foreach (IEntity item : items)
		{
			if (!item || RZ_ColdCaseComponent.Of(item))
				continue;

			RZ_MissionItemComponent job = RZ_MissionItemComponent.Cast(item.FindComponent(RZ_MissionItemComponent));
			string goods = RZ_MissionGoods.Owner(item);
			if (!job && goods.IsEmpty())
				continue;

			EntityPrefabData data = item.GetPrefabData();
			if (!data)
				continue;

			int mission = NO_MISSION;
			if (job)
				mission = job.GetMissionId();

			profile.m_aQiPrefab.Insert(data.GetPrefabName());
			profile.m_aQiMission.Insert(mission);
			profile.m_aQiGoods.Insert(goods);
			if (!inv.TryDeleteItem(item))
				SCR_EntityHelper.DeleteEntityAndChildren(item); // VERIFY: deleting an item still in a dead body's storage leaves the storage sound (as the case does)

			n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! The held quest items back in the pack, each only while its job still runs (an errand's item
	//! points its errand at the new item). Returns how many came back.
	static int GiveBackQuestItems(RZ_GameModeComponent gm, int playerId, IEntity me, RZ_PlayerProfile profile)
	{
		profile.EnsureArrays();
		int back = 0;
		int gone = 0;
		for (int i = 0; i < profile.m_aQiPrefab.Count(); i++)
		{
			int mission = NO_MISSION;
			if (i < profile.m_aQiMission.Count())
				mission = profile.m_aQiMission[i];

			string goods = "";
			if (i < profile.m_aQiGoods.Count())
				goods = profile.m_aQiGoods[i];

			if (!JobRuns(gm, playerId, mission))
			{
				gone++;
				continue;
			}

			IEntity item = RZ_Util.SpawnPrefab(profile.m_aQiPrefab[i], RZ_Util.RandomPointInRing(me.GetOrigin(), 0.3, 0.6) + Vector(0, 0.3, 0), 0);
			if (!item)
				continue;

			RZ_Util.InsertIntoInventory(me, item);
			if (mission != NO_MISSION)
				RZ_MissionItemComponent.Tag(item, mission);
			if (!goods.IsEmpty())
				RZ_MissionGoods.Tag(item, goods);

			if (mission <= -1000 && gm.GetErrands())
				gm.GetErrands().CarriedComesBack(-1000 - mission, item);

			back++;
		}

		profile.m_aQiPrefab.Clear();
		profile.m_aQiMission.Clear();
		profile.m_aQiGoods.Clear();
		if (gone > 0)
			gm.NotifyPlayer(playerId, "Your job", "The job's over while you were gone. They took their things back.");

		if (back + gone > 0)
			RZ_Log.Info(string.Format("Character: %1 came round with %2 quest items (%3 gone with their jobs)", RZ_Util.PlayerName(playerId), back, gone));

		return back;
	}

	//------------------------------------------------------------------------------------------------
	//! The job a held item belongs to still runs: an errand still held by this player, a mission still
	//! active; goods with no job id always come back.
	protected static bool JobRuns(RZ_GameModeComponent gm, int playerId, int mission)
	{
		if (mission == NO_MISSION || mission == -1)
			return true;

		if (mission <= -1000)
			return gm.GetErrands() && gm.GetErrands().HeldBy(-1000 - mission, playerId);

		RZ_MissionManager missions = gm.GetMissions();
		return missions && missions.FindById(mission) != null;
	}

	// ================================================================ Coming round
	//------------------------------------------------------------------------------------------------
	//! From the spawn handler (RZ_OpeningPlaces.c's modded SCR_SpawnPointSpawnHandlerComponent),
	//! before the character is handed over: a character who goes on comes round at the nearest camp
	//! that takes them in, else at a random start. False for a new character (the opening places it).
	static bool PlaceComeRound(RZ_GameModeComponent gm, int playerId, IEntity character)
	{
		if (!s_mMoved)
		{
			s_mMoved = new map<int, bool>();
			s_mCamp = new map<int, string>();
			s_mCampFaction = new map<int, int>();
		}

		s_mMoved.Remove(playerId);
		s_mCamp.Remove(playerId);
		s_mCampFaction.Remove(playerId);
		RZ_PlayerProfile profile = gm.GetProfile(playerId);
		if (!profile || !character)
			return false;

		profile.EnsureArrays();
		if (!profile.HasCharacter())
			return false;

		vector at;
		string name;
		int faction = -1;
		string why = FindCamp(gm, playerId, profile, at, name, faction);
		if (why.IsEmpty())
		{
			character.SetOrigin(at + Vector(0, 0.1, 0));
			s_mMoved.Set(playerId, true);
			s_mCamp.Set(playerId, name);
			s_mCampFaction.Set(playerId, faction);
			RZ_Log.Info(string.Format("Character: %1 comes round at %2 (%3)", RZ_Util.PlayerName(playerId), name, RZ_SettlementScanner.GridReference(at)));
			return true;
		}

		RZ_Openings openings = RZ_Openings.Get();
		bool rolled = openings && openings.RandomStart(playerId, character);
		if (rolled)
			s_mMoved.Set(playerId, true);

		RZ_Log.Info(string.Format("Character: %1 comes round at a random start (%2)%3", RZ_Util.PlayerName(playerId), why, NoRoll(rolled)));
		return rolled;
	}

	//------------------------------------------------------------------------------------------------
	protected static string NoRoll(bool rolled)
	{
		if (rolled)
			return "";

		return ": no start rolled, the spawn point";
	}

	//------------------------------------------------------------------------------------------------
	//! The spawn handler moved this player's character (it preloads the world there).
	static bool Moved(int playerId)
	{
		return s_mMoved && s_mMoved.Contains(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest camp to the last death that takes this character in: "" and its spot, name and
	//! faction; else why none did ("bitten", "no camp takes you in").
	static string FindCamp(RZ_GameModeComponent gm, int playerId, RZ_PlayerProfile profile, inout vector at, inout string name, inout int faction)
	{
		if (profile.m_bDiedBitten)
			return "bitten: nobody carries in a bitten body";

		vector from = Vector(profile.m_fDiedX, 0, profile.m_fDiedZ);
		float best = float.MAX;
		bool found = false;
		RZ_CampLife camps = gm.GetCampLife();
		RZ_Territory territory = gm.GetTerritory();
		if (territory)
		{
			foreach (RZ_Holding h : territory.GetHoldings())
			{
				if (!h || !TakesIn(gm, playerId, profile, h.m_eFaction, RZ_ETier.STRANGER, h.m_vPos, "holding." + h.m_iId.ToString()))
					continue;

				float d = RZ_Util.DistXZ(from, h.m_vPos);
				if (d >= best)
					continue;

				best = d;
				at = h.m_vPos;
				name = h.m_sName;
				faction = h.m_eFaction;
				found = true;
			}
		}

		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses && !houses.IsExiled(playerId))
		{
			for (int i = 0; i < houses.Count(); i++)
			{
				RZ_Safehouse sh = houses.Get(i);
				if (!sh || !TakesIn(gm, playerId, profile, RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, sh.m_vPos, "house." + i.ToString()))
					continue;

				float dh = RZ_Util.DistXZ(from, sh.m_vPos);
				if (dh >= best)
					continue;

				best = dh;
				at = sh.m_vPos;
				name = "the safehouse in " + sh.m_sName;
				faction = RZ_EFaction.SURVIVORS;
				found = true;
			}
		}

		bool stand = gm.GetCure() && gm.GetCure().IsFinalStand();
		if (gm.HasLab() && !stand && TakesIn(gm, playerId, profile, RZ_EFaction.INSTITUTE, RZ_ETier.KNOWN, gm.GetLabPosition(), "lab"))
		{
			float dl = RZ_Util.DistXZ(from, gm.GetLabPosition());
			if (dl < best)
			{
				best = dl;
				at = gm.GetLabPosition();
				name = "Halcyon's lab";
				faction = RZ_EFaction.INSTITUTE;
				found = true;
			}
		}

		if (!found)
			return "no camp takes you in";

		// Their yard, where their people are; a step off a body or a wall
		vector home;
		if (camps && camps.HomeNear(at, HOME_REACH, home))
			at = home;

		at = RZ_Mission.LandPointNear(at, 2, 6);
		Remember(profile, CampKeyAt(gm, at, name));
		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A camp at pos of `faction` takes this character in: their standing is at least `need` (by
	//! access: Stranger, Uneasy and better; never Watched, Hunted or Kill on Sight), it isn't under
	//! attack, and it hasn't taken them in three times in the last real hour.
	protected static bool TakesIn(RZ_GameModeComponent gm, int playerId, RZ_PlayerProfile profile, int faction, RZ_ETier need, vector pos, string key)
	{
		if (!RZ_Reputation.IsAtLeast(gm.GetTier(playerId, faction), need))
			return false;

		RZ_CampLife camps = gm.GetCampLife();
		if (camps && camps.UnderAttackNear(pos, HOME_REACH))
			return false;

		return TimesTakenIn(profile, key) < TAKEN_IN_MAX;
	}

	//------------------------------------------------------------------------------------------------
	//! A camp's key for the taken-in count.
	static string CampKeyAt(RZ_GameModeComponent gm, vector at, string name)
	{
		return name;
	}

	//------------------------------------------------------------------------------------------------
	protected static int TimesTakenIn(RZ_PlayerProfile profile, string key)
	{
		int now = RZ_SeasonClock.Now();
		int n = 0;
		for (int i = 0; i < profile.m_aInCamps.Count() && i < profile.m_aInUnix.Count(); i++)
		{
			if (now - profile.m_aInUnix[i] < TAKEN_IN_WINDOW && profile.m_aInCamps[i] == key)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Remember(RZ_PlayerProfile profile, string key)
	{
		int now = RZ_SeasonClock.Now();
		for (int i = profile.m_aInUnix.Count() - 1; i >= 0; i--)
		{
			if (now - profile.m_aInUnix[i] >= TAKEN_IN_WINDOW && i < profile.m_aInCamps.Count())
			{
				profile.m_aInUnix.Remove(i);
				profile.m_aInCamps.Remove(i);
			}
		}

		profile.m_aInCamps.Insert(key);
		profile.m_aInUnix.Insert(now);
	}

	//------------------------------------------------------------------------------------------------
	//! From the game mode's spawn, for a character who goes on: the kit, the quest items and the
	//! line. The spawn card's line ("" for none).
	static string ComeRound(RZ_GameModeComponent gm, int playerId, IEntity me, RZ_PlayerProfile profile, RZ_Background bg)
	{
		int kept = StripKit(me, bg);
		GiveBackQuestItems(gm, playerId, me, profile);
		string camp;
		int faction = -1;
		if (s_mCamp)
		{
			s_mCamp.Find(playerId, camp);
			s_mCampFaction.Find(playerId, faction);
		}

		RZ_Log.Info(string.Format("Character: %1 came round as %2 (death %3), kit trimmed to %4 things", RZ_Util.PlayerName(playerId), bg.m_sName, profile.m_iDeaths, kept));
		if (camp.IsEmpty())
			return "You came round. Your things are where you fell.";

		// Somebody there says how you got back (a person speaks: docs/MISSION_RULES.md, rule 29)
		gm.Say(playerId, FactionVoiceName(faction), faction, ComeRoundLine(faction));
		return string.Format("You came round at %1. Your things are where you fell.", camp);
	}

	//------------------------------------------------------------------------------------------------
	protected static string FactionVoiceName(int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "A Haven man";
			case RZ_EFaction.MILITARY: return "A Bastion medic";
			case RZ_EFaction.BANDITS: return "A Red Hand crewman";
			case RZ_EFaction.INSTITUTE: return "A Halcyon orderly";
		}

		return "Someone";
	}

	//------------------------------------------------------------------------------------------------
	protected static string ComeRoundLine(int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "Found you on the road. You owe us nothing. Eat something.";
			case RZ_EFaction.MILITARY: return "Patrol brought you in. Don't make a habit of it.";
			case RZ_EFaction.BANDITS: return "We dragged you in. It's in the book.";
			case RZ_EFaction.INSTITUTE: return "You were brought in. You're clean. Sign here.";
		}

		return "Found you on the road.";
	}

	// ================================================================ The kit
	//------------------------------------------------------------------------------------------------
	//! What a character comes round with: what they wear, one bandage, one water bottle and the
	//! background's own tool. Never a weapon, ammunition or anything else the loadout carried.
	//! Returns how many carried things were kept.
	static int StripKit(IEntity me, RZ_Background bg)
	{
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(me.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return 0;

		array<string> bandage = {};
		RZ_ItemHelper.Words("bandage,dressing,gauze", bandage);
		array<string> water = {};
		RZ_ItemHelper.Words("canteen,water,bottle,flask", water);
		array<string> tool = {};
		RZ_ItemHelper.Words(SignatureTool(bg), tool);
		bool hasBandage = false;
		bool hasWater = false;
		bool hasTool = false;
		int kept = 0;
		array<IEntity> items = {};
		inv.GetItems(items);
		foreach (IEntity item : items)
		{
			if (!item || IsWorn(me, item))
				continue;

			if (!hasBandage && RZ_ItemHelper.Matches(item, bandage))
			{
				hasBandage = true;
				kept++;
				continue;
			}

			if (!hasWater && RZ_ItemHelper.Matches(item, water))
			{
				hasWater = true;
				kept++;
				continue;
			}

			if (!hasTool && !tool.IsEmpty() && RZ_ItemHelper.Matches(item, tool) && !IsWeapon(item))
			{
				hasTool = true;
				kept++;
				continue;
			}

			if (!inv.TryDeleteItem(item))
				SCR_EntityHelper.DeleteEntityAndChildren(item); // VERIFY: a fresh character's loadout items delete cleanly from its storages
		}

		return kept;
	}

	//------------------------------------------------------------------------------------------------
	//! A background's own tool, kept at coming round (never a weapon): keywords.
	static string SignatureTool(RZ_Background bg)
	{
		if (!bg)
			return "";

		switch (bg.m_sId)
		{
			case "mechanic": return "wrench,repairkit,repair_kit";
			case "radio_operator": return "radio";
			case "nurse": return "dressing,bandage";
			case "doctor": return "dressing,bandage";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A weapon, a magazine or ammunition (never kept at coming round).
	protected static bool IsWeapon(IEntity item)
	{
		if (item.FindComponent(WeaponComponent) || item.FindComponent(MagazineComponent))
			return true;

		string name = RZ_ItemHelper.LowerName(item);
		return name.Contains("ammo") || name.Contains("magazine") || name.Contains("grenade") || name.Contains("knife") || name.Contains("machete");
	}

	//------------------------------------------------------------------------------------------------
	//! Worn clothing, a vest, a helmet or a pack, or what's held: what the character wears stays (a
	//! held weapon doesn't: it's not clothing).
	protected static bool IsWorn(IEntity me, IEntity item)
	{
		InventoryItemComponent iic = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!iic)
			return false;

		InventoryStorageSlot slot = iic.GetParentSlot();
		if (!slot)
			return false;

		BaseInventoryStorageComponent storage = slot.GetStorage();
		if (!storage)
			return false;

		// VERIFY: worn clothes sit in the character's loadout storage (as the crafting's worn check reads)
		return EquipedLoadoutStorageComponent.Cast(storage) && storage.GetOwner() == me;
	}

	// ================================================================ Reset character
	//------------------------------------------------------------------------------------------------
	//! Seconds until this player may reset again (0: now).
	static int ResetWait(RZ_PlayerProfile profile)
	{
		if (!profile || profile.m_iResetUnix <= 0)
			return 0;

		return Math.Max(0, RESET_EVERY - (RZ_SeasonClock.Now() - profile.m_iResetUnix));
	}

	//------------------------------------------------------------------------------------------------
	//! Reset character, confirmed twice on the client (the Standing page): this person is gone. The
	//! character is killed where they stand; the next spawn picks a background and runs the opening.
	//! The reason it can't be done, or "" when it was.
	static string Reset(RZ_GameModeComponent gm, int playerId)
	{
		RZ_PlayerProfile profile = gm.GetProfile(playerId);
		if (!profile || !profile.HasCharacter())
			return "You have no character to reset.";

		int wait = ResetWait(profile);
		if (wait > 0)
			return string.Format("One reset a day. You can reset again in %1 hours.", Math.Ceil(wait / 3600.0));

		profile.m_bResetPending = true;
		profile.m_iResetUnix = RZ_SeasonClock.Now();
		RZ_Log.Info(string.Format("Character: %1 reset character %2 (%3 deaths)", RZ_Util.PlayerName(playerId), profile.m_iCharacter, profile.m_iDeaths));
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (me)
			RZ_Util.KillCharacter(me);

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A new character's standing after a reset: with any faction the last one was Hunted or worse
	//! with, the new one starts Watched (they don't know the face; they know the name). From the game
	//! mode, after StartCharacter. hunted: the factions that hunted the last character.
	static void CarryTheName(RZ_PlayerProfile profile, array<int> hunted)
	{
		foreach (int f : hunted)
		{
			if (f < 0 || f >= RZ_Factions.COUNT)
				continue;

			// Watched: Fame under the Known line, Infamy at the Watched line (RZ_Reputation.GetTier)
			profile.m_aFame[f] = Math.Min(profile.m_aFame[f], WATCHED_FAME_BELOW - 1);
			profile.m_aInfamy[f] = Math.Max(profile.m_aInfamy[f], WATCHED_INFAMY);
			profile.m_aRepSeen[f] = RZ_RepNames.GetRep(profile.m_aFame[f], profile.m_aInfamy[f]);
			RZ_Log.Info(string.Format("Character: a new character starts Watched with %1 (the last was hunted)", RZ_Factions.GetName(f)));
		}
	}
}
