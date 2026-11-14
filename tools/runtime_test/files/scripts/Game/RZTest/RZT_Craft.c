// Test-only (not part of the mod): the new recipes and the lockpick (docs/DESIGN.md, "Light crafting").
// Three minutes in: the test world's Crafting outputs are set to the repo's prefabs (the test world
// leaves them empty) and the tester gets a pack. Then each new recipe, one per tick: its inputs (and
// tool) are put in his pack, and on the next tick it's made through the bench's own FinishRecipe: its
// output must come out (as many as it makes), its inputs must be gone and a kept tool must stay.
// (Putting an item in a pack lands a moment later, so the make waits a tick.) Then the nearest locked
// building's door is picked with a lockpick (the pick's chance set to 1 for the test): it must open.
// Lines: "RZT craft: ..." and "RZT craft FAIL ...".

class RZT_CraftCase
{
	string m_sName;
	bool m_bStove;
	ref array<ResourceName> m_aInputs = {};
	ResourceName m_sTool;
	ResourceName m_sOutput;
	int m_iMade;
}

class RZT_Craft
{
	static const float START = 180;		// seconds in
	static float s_fClock;
	static int s_iFails;
	static int s_iStep;
	static float s_fWait;
	static bool s_bDone;
	static ref array<ref RZT_CraftCase> s_aCases = {};
	static int s_iCase;
	static ref array<IEntity> s_aGiven = {};
	static IEntity s_Tool;
	static int s_iBefore;
	static int s_iPhase;		// a case: 0 put its inputs, 1 make it, 2 count what it made

	static const ResourceName TROUSERS = "{6925E6C63EBCF623}Prefabs/Characters/Uniforms/Pants_Trousers_01/Pants_Trousers_01_DarkBrown.et";
	static const ResourceName BLADE = "{3805C0B87B9D2AB0}prefabs/weapons/Melee/Knife.et";
	static const ResourceName WIRE = "{2E0FAA5F39CBA66C}Prefabs/ReforgerZ/Crafting/RZ_Wire.et";
	static const ResourceName NAILS = "{267D3FAF0F92499F}Prefabs/ReforgerZ/Crafting/RZ_Nails.et";
	static const ResourceName SCRAP = "{6CB732A4814E2F43}Prefabs/ReforgerZ/Crafting/RZ_Scrap.et";
	static const ResourceName SALT = "{6403BD4E5B4F0C04}Prefabs/ReforgerZ/Crafting/RZ_Salt.et";
	static const ResourceName CELLS = "{F51872C09AA3A89A}Prefabs/ReforgerZ/Crafting/RZ_Cells.et";
	static const ResourceName PAD = "{268E3FCBD96622AD}Prefabs/ReforgerZ/Crafting/RZ_WritingPad.et";
	static const ResourceName RAGS = "{9B0770D65A7CBEC2}Prefabs/ReforgerZ/Crafting/RZ_Rags.et";
	static const ResourceName PICK = "{5CB9F2D4C344D632}Prefabs/ReforgerZ/Crafting/RZ_Lockpick.et";
	static const ResourceName CHARCOAL = "{86CC36A7C8FC6221}Prefabs/ReforgerZ/Crafting/RZ_Charcoal.et";
	static const ResourceName FILTER = "{349CE5A1F50F9799}Prefabs/ReforgerZ/Crafting/RZ_WaterFilter_Purification.et";
	static const ResourceName STERILE = "{7CAFE5B9FFAE60EA}Prefabs/ReforgerZ/Crafting/RZ_SterileDressing_Swab.et";
	static const ResourceName MAPCOPY = "{EDDE944F7E4788F2}Prefabs/ReforgerZ/Crafting/RZ_MapCopy.et";
	static const ResourceName STEW = "{5250D25C2785A697}Prefabs/ReforgerZ/Crafting/RZ_Stew.et";
	static const ResourceName LURE = "{CD06FAC27E299703}Prefabs/ReforgerZ/Crafting/RZ_RadioLure.et";
	static const ResourceName PLANKS = "{0A101873E8C9274C}Prefabs/ReforgerZ/Interactions/RZ_Planks.et";
	static const ResourceName PLUGS = "{6CEB9445ED635609}Prefabs/ReforgerZ/Interactions/RZ_SparkPlugs.et";
	static const ResourceName JERKY = "{C22A2F123CD609FC}Prefabs/Items/Food/Meat_01_Dry.et";
	static const ResourceName RADIO = "{73950FBA2D7DB5C5}Prefabs/Items/Equipment/Radios/Radio_ANPRC68.et";
	static const ResourceName WRENCH = "{33B2DFDCD0EBA3DB}Prefabs/Items/Equipment/Kits/RepairKit_01/RepairKit_01_wrench.et";
	static const ResourceName WOOD = "{B5B3D0AFBB4DE757}Prefabs/Items/Core/FM_WoodStick.et";
	static const ResourceName WATER = "{D5E10AA9B0808D76}prefabs/Items/Water/WaterBottle.et";
	static const ResourceName MRE = "{EB21D26B8D5A3232}Prefabs/Items/Food/MRE.et";
	static const ResourceName MEAT = "{128A19AA9F731095}prefabs/Items/Food/MeatLeg_01.et";
	static const ResourceName PACK = "{06B68C58B72EAAC6}Prefabs/Items/Equipment/Backpacks/Backpack_ALICE_Medium.et";

	// Base-game stand-ins for the recipes' own items (the keywords and outputs the test sets)
	static const ResourceName S_SMALL = "{C98FCC6CA6C56069}Prefabs/Items/Medicine/AspirinBottle.et";				// nails, cells, the pad, salt
	static const ResourceName S_WIRE = "{61D4F80E49BF9B12}Prefabs/Items/Equipment/Compass/Compass_SY183.et";		// wire, and the lockpick
	static const ResourceName S_SCRAP = "{80E75A71C29190DB}Prefabs/Items/Medicine/Tourniquet_01/Tourniquet_USSR_01.et";	// scrap
	static const ResourceName S_CLOTH = "{AF3411AAF94D18C2}Prefabs/Items/Medicine/Gauze_02/Gauze_02.et";			// a rag
	static const ResourceName S_CHARCOAL = "{61A705D76908160C}Prefabs/Items/Equipment/Watches/Watch_Orlik38/Watch_Orlik38.et";	// charcoal
	static const ResourceName S_RAGS = "{CDCC313B2FB76FE7}Prefabs/Items/Medicine/FieldDressing_01/Gauze_01.et";
	static const ResourceName S_PICK = "{6FD6C96121905202}Prefabs/Items/Equipment/Watches/Watch_Vostok.et";
	static const ResourceName S_PLANKS = "{0D9A5DCF89AE7AA9}Prefabs/Items/Medicine/MorphineInjection_01/MorphineInjection_01.et";
	static const ResourceName S_FILTER = "{D70216B1B2889129}Prefabs/Items/Medicine/Tourniquet_01/Tourniquet_US_01.et";
	static const ResourceName S_LURE = "{7CEF68E2BC68CE71}Prefabs/Items/Equipment/Compass/Compass_Adrianov.et";
	static const ResourceName S_PLUGS = "{00E36F41CA310E2A}Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_01.et";
	static const ResourceName S_MAP = "{13772C903CB5E4F7}Prefabs/Items/Equipment/Maps/Map_Paper_01/PaperMap_01_folded.et";
	static const ResourceName S_STERILE = "{A81F501D3EF6F38E}Prefabs/Items/Medicine/FieldDressing_01/FieldDressing_US_01.et";
	static const ResourceName S_CHARCOAL_OUT = "{C3F1FA1E2EC2B345}Prefabs/Items/Medicine/FieldDressing_01/FieldDressing_USSR_01.et";
	static const ResourceName S_STEW = "{50A68300B537EDFA}Prefabs/Items/Food/ArmyCrackers_Soviet_01.et";

	//------------------------------------------------------------------------------------------------
	//! Each real prefab the recipes and the loot use loads and spawns with an inventory item on it.
	static void Assets()
	{
		array<ResourceName> real = {WIRE, NAILS, SCRAP, SALT, CELLS, PAD, RAGS, PICK, CHARCOAL, FILTER, STERILE, MAPCOPY, STEW, LURE, PLANKS, PLUGS,
			"{7A359D186C2E013F}Prefabs/ReforgerZ/Crafting/RZ_TinCan_Empty.et", "{9D7456C2552534E9}Prefabs/ReforgerZ/Crafting/RZ_Bleach.et"};
		int ok = 0;
		foreach (ResourceName r : real)
		{
			IEntity e = RZ_Util.SpawnPrefab(r, RZT_Fake.s_Ent.GetOrigin() + Vector(2, 0.5, 0), 0);
			if (!e)
			{
				Fail(string.Format("%1 doesn't load", RZ_Util.PrettyPrefabName(r)));
				continue;
			}

			if (!e.FindComponent(InventoryItemComponent))
				Fail(string.Format("%1 has no inventory item on it", RZ_Util.PrettyPrefabName(r)));
			else
				ok++;

			SCR_EntityHelper.DeleteEntityAndChildren(e);
		}

		Print(string.Format("RZT craft: %1 of %2 of the recipes' and the loot's own prefabs load as items", ok, real.Count()));
	}

	//------------------------------------------------------------------------------------------------
	static void Fail(string what)
	{
		s_iFails++;
		Print("RZT craft FAIL " + what);
	}

	//------------------------------------------------------------------------------------------------
	static void Add(string name, bool stove, ResourceName tool, ResourceName output, int made, ResourceName a, ResourceName b = "", ResourceName c = "", ResourceName d = "")
	{
		RZT_CraftCase k = new RZT_CraftCase();
		k.m_sName = name;
		k.m_bStove = stove;
		k.m_sTool = tool;
		k.m_sOutput = output;
		k.m_iMade = made;
		k.m_aInputs.Insert(a);
		if (!b.IsEmpty())
			k.m_aInputs.Insert(b);

		if (!c.IsEmpty())
			k.m_aInputs.Insert(c);

		if (!d.IsEmpty())
			k.m_aInputs.Insert(d);

		s_aCases.Insert(k);
	}

	//------------------------------------------------------------------------------------------------
	static void Step(float dt, RZ_Interactions hub)
	{
		if (s_bDone || !RZT_Fake.s_Ent || !RZ_Util.IsCharacterAlive(RZT_Fake.s_Ent))
			return;

		s_fClock += dt;
		if (s_fClock < START)
			return;

		RZ_Crafting craft = hub.GetCrafting();
		if (!craft)
		{
			Print("RZT craft: crafting is off in the test world; not tried");
			s_bDone = true;
			return;
		}

		IEntity user = RZT_Fake.s_Ent;
		if (s_iStep == 0)
		{
			// The real prefabs: each loads and spawns as an item (on a dedicated server a text-written
			// item doesn't go into a pack until Workbench has registered it, so the recipes below are
			// run with base-game stand-ins in its place: their keywords and outputs, set for the test)
			Assets();
			craft.RZT_Configure();
			IEntity pack = RZ_Util.SpawnPrefab(PACK, user.GetOrigin() + Vector(0, 0.5, 0), 0);
			if (!pack || !RZ_Util.InsertIntoInventory(user, pack))
				Print("RZT craft: the backpack didn't go on; the pockets will have to do");
			else
				Print("RZT craft: a backpack on the tester for the inputs");

			Add("Rags", false, BLADE, S_RAGS, 2, TROUSERS);
			Add("Lockpick", false, BLADE, S_PICK, 1, S_WIRE);
			Add("Planks for a barricade", false, "", S_PLANKS, 1, WOOD, WOOD, WOOD, S_SMALL);
			Add("Water filters", false, "", S_FILTER, 3, WATER, S_CHARCOAL, S_CLOTH);
			Add("Radio lure", false, "", S_LURE, 1, RADIO, S_SMALL);
			Add("Salvaged spark plugs", false, WRENCH, S_PLUGS, 1, S_SCRAP, S_SCRAP, S_WIRE);
			Add("Hand-drawn map", false, "", S_MAP, 1, S_SMALL);
			Add("Sterile dressing", true, "", S_STERILE, 1, S_CLOTH, WATER);
			Add("Charcoal", true, "", S_CHARCOAL_OUT, 1, WOOD, WOOD);
			Add("Jerky", true, "", JERKY, 2, MEAT, S_SMALL);
			Add("Stew", true, "", S_STEW, 1, MRE, MRE, WATER);
			s_iStep = 1;
			return;
		}

		if (s_iStep == 1)
		{
			if (s_iCase >= s_aCases.Count())
			{
				Clear(user);
				s_iStep = 2;
				return;
			}

			RZT_CraftCase k = s_aCases[s_iCase];
			if (s_iPhase == 0)
			{
				if (PutInputs(user, k))
					s_iPhase = 1;
				else
					s_iCase++;

				return;
			}

			if (s_iPhase == 1)
			{
				if (Make(craft, user, k))
					s_iPhase = 2;
				else
				{
					s_iPhase = 0;
					s_iCase++;
				}

				return;
			}

			// What it made lands in the pack a moment later, as the inputs did
			Check(user, k);
			s_iPhase = 0;
			s_iCase++;
			return;
		}

		if (s_iStep == 2)
		{
			if (!FindDoor(hub))
				return;

			s_iStep = 3;
			return;
		}

		if (s_iStep == 3)
		{
			// The pick went in the pack last tick: now the lock
			RZ_Spot spot = hub.RZT_FindSpot(s_iPickSpot);
			if (!spot)
			{
				Fail("the locked door's spot is gone");
				Finish();
				return;
			}

			hub.GetDoors().RZT_PickChance(1);
			hub.GetDoors().HandleChoice(RZT_Fake.ID, spot, 3, user);
			Print(string.Format("RZT craft: picking the lock of a locked door at %1", RZ_SettlementScanner.GridReference(spot.m_vPos)));
			// The pick's hold started for him; a fake player's hold never ticks (no controlled
			// entity), so it's done for him as the other checks do
			RZ_TimedTask task = hub.GetTask(RZT_Fake.ID);
			if (!task || task.m_iKind != RZ_Interactions.TASK_FORCE || task.m_iArg != RZ_DoorLocks.PICK_ARG)
			{
				Fail("picking the lock didn't start its hold (no lockpick found in the pack?)");
				Finish();
				return;
			}

			s_vPickBuilding = spot.m_vBuilding;
			s_sPickKey = spot.m_sBuildingKey;
			s_sPickType = spot.m_sType;
			hub.GetDoors().CompletePick(RZT_Fake.ID, spot, user);
			s_iStep = 4;
			s_fWait = 0;
			return;
		}

		// The pick's hold: done when the task completes (about Pick seconds)
		s_fWait += dt;
		// By the building, not the spot: a spot can be rebuilt with a new id once its lock changes
		if (!hub.GetSearch().IsBuildingLocked(s_vPickBuilding, s_sPickKey, s_sPickType))
		{
			Print("RZT craft: the lockpick opened the locked door, quietly");
			Finish();
			return;
		}

		if (s_fWait > 45)
		{
			Fail("the lockpick didn't open the locked door in 45 s");
			Finish();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A recipe's inputs and tool into the tester's pack. False (and a failure) when one won't go in.
	static bool PutInputs(IEntity user, RZT_CraftCase k)
	{
		Clear(user);
		s_aGiven.Clear();
		s_Tool = null;
		foreach (ResourceName input : k.m_aInputs)
		{
			IEntity item = Put(user, input);
			if (!item)
			{
				Fail(string.Format("%1: %2 won't go in the tester's pack", k.m_sName, RZ_Util.PrettyPrefabName(input)));
				return false;
			}

			s_aGiven.Insert(item);
		}

		if (!k.m_sTool.IsEmpty())
		{
			s_Tool = Put(user, k.m_sTool);
			if (!s_Tool)
			{
				Fail(string.Format("%1: its tool %2 won't go in the pack", k.m_sName, RZ_Util.PrettyPrefabName(k.m_sTool)));
				return false;
			}
		}

		s_iBefore = CountOf(user, k.m_sOutput);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool Make(RZ_Crafting craft, IEntity user, RZT_CraftCase k)
	{
		int missing = 0;
		foreach (IEntity g : s_aGiven)
		{
			if (!g || !InPack(user, g))
				missing++;
		}

		if (missing > 0)
		{
			Fail(string.Format("%1: %2 of its inputs never landed in the pack (in: %3)", k.m_sName, missing, Where(s_aGiven)));
			return false;
		}

		if (!craft.RZT_Make(user, k.m_sName, k.m_bStove))
		{
			Fail(string.Format("%1: no such recipe at the %2 (output not set?)", k.m_sName, StoveWord(k.m_bStove)));
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static void Check(IEntity user, RZT_CraftCase k)
	{
		int after = CountOf(user, k.m_sOutput);
		int left = 0;
		foreach (IEntity left1 : s_aGiven)
		{
			if (left1 && InPack(user, left1))
				left++;
		}

		bool toolKept = !s_Tool || InPack(user, s_Tool);
		if (after - s_iBefore != k.m_iMade)
			Fail(string.Format("%1: made %2, not %3 (its inputs sat in: %4)", k.m_sName, after - s_iBefore, k.m_iMade, Where(s_aGiven)));
		else if (left > 0)
			Fail(string.Format("%1: %2 of its inputs are still in the pack (%3); it used something else of the same kind", k.m_sName, left, Where(s_aGiven)));
		else if (!toolKept)
			Fail(string.Format("%1: its tool was taken", k.m_sName));
		else
			Print(string.Format("RZT craft: %1 at the %2: %3 made from %4 inputs%5", k.m_sName, StoveWord(k.m_bStove), k.m_iMade, k.m_aInputs.Count(), ToolWord(k.m_sTool)));
	}

	//------------------------------------------------------------------------------------------------
	//! The storage class each item sits in, for a failure's line.
	static string Where(array<IEntity> items)
	{
		string text = "";
		foreach (IEntity e : items)
		{
			string at = "gone";
			if (e)
			{
				InventoryItemComponent iic = InventoryItemComponent.Cast(e.FindComponent(InventoryItemComponent));
				at = string.Format("loose(%1, item %2)", RZ_Util.PrettyPrefabName(RZ_Util.GetPrefabName(e)), iic != null);
				if (iic && iic.GetParentSlot() && iic.GetParentSlot().GetStorage())
					at = RZ_Util.PrettyPrefabName(RZ_Util.GetPrefabName(e)) + " in " + iic.GetParentSlot().GetStorage().ClassName();
			}

			text += at + " ";
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	static string StoveWord(bool stove)
	{
		if (stove)
			return "stove";

		return "bench";
	}

	//------------------------------------------------------------------------------------------------
	static string ToolWord(ResourceName tool)
	{
		if (tool.IsEmpty())
			return "";

		return ", the tool kept";
	}

	// ================================================================ The lockpick
	static int s_iPickSpot = -1;
	static vector s_vPickBuilding;		// the picked door's building, kept: its spot can be rebuilt
	static string s_sPickKey;
	static string s_sPickType;

	//------------------------------------------------------------------------------------------------
	//! Goes to the nearest locked building, waits for a door spot, puts a pick in the pack. True when
	//! ready to pick (next tick); false while waiting, or after giving up (then it finishes).
	static bool FindDoor(RZ_Interactions hub)
	{
		IEntity user = RZT_Fake.s_Ent;
		RZ_Spot door = hub.RZT_LockedDoorNear(user.GetOrigin(), 3000);
		if (!door)
		{
			// No door learned near him: a door spot on the nearest locked building, as the locks make
			// one when a player comes near (its spot and lock, not its leaf); he goes to it at once,
			// before the hub drops a spot nobody is near
			door = hub.RZT_DoorSpotOnLocked(user.GetOrigin(), 3000);
			if (!door)
			{
				Print("RZT craft: no locked building within 3 km of the tester; the lockpick not tried");
				Finish();
				return false;
			}

			Print(string.Format("RZT craft: a door spot on the locked building at %1", RZ_SettlementScanner.GridReference(door.m_vPos)));
		}

		RZT_Fake.MoveTo(door.m_vPos + Vector(0.6, 0, 0));
		Clear(user);
		if (!Put(user, S_PICK))
		{
			Fail("the lockpick won't go in the pack");
			Finish();
			return false;
		}

		s_iPickSpot = door.m_iId;
		return true;
	}

	// ================================================================ The pack
	static ref array<IEntity> s_aMine = {};

	//------------------------------------------------------------------------------------------------
	static IEntity Put(IEntity user, ResourceName prefab)
	{
		IEntity item = RZ_Util.SpawnPrefab(prefab, user.GetOrigin() + Vector(0, 0.5, 0), 0);
		if (!item)
			return null;

		s_aMine.Insert(item);
		if (!RZ_Util.InsertIntoInventory(user, item))
		{
			SCR_EntityHelper.DeleteEntityAndChildren(item);
			return null;
		}

		return item;
	}

	//------------------------------------------------------------------------------------------------
	static bool InPack(IEntity user, IEntity item)
	{
		array<IEntity> items = {};
		Items(user, items);
		return items.Contains(item);
	}

	//------------------------------------------------------------------------------------------------
	static int CountOf(IEntity user, ResourceName prefab)
	{
		string want = prefab;
		want.ToLower();
		int i = want.IndexOf("}");
		if (i >= 0)
			want = want.Substring(i + 1, want.Length() - i - 1);

		array<IEntity> items = {};
		Items(user, items);
		int n = 0;
		foreach (IEntity e : items)
		{
			string name = RZ_Util.GetPrefabName(e);
			name.ToLower();
			if (name.Contains(want))
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	static void Items(IEntity user, notnull array<IEntity> items)
	{
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
		if (inv)
			inv.GetItems(items);
	}

	//------------------------------------------------------------------------------------------------
	//! Empties what the test put in and what the recipes made (the tester's own clothes stay).
	static void Clear(IEntity user)
	{
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return;

		array<IEntity> items = {};
		inv.GetItems(items);
		foreach (IEntity e : items)
		{
			string name = RZ_Util.GetPrefabName(e);
			name.ToLower();
			if (s_aMine.Contains(e) || name.Contains("reforgerz/crafting") || name.Contains("meat_01_dry") || name.Contains("gauze_01") || name.Contains("watch_vostok")
				|| name.Contains("morphine") || name.Contains("salinebag_us") || name.Contains("fielddressing_ussr") || name.Contains("tourniquet_us_01") || name.Contains("compass_adrianov") || name.Contains("watch_sandy") || name.Contains("papermap")
				|| name.Contains("fielddressing_us") || name.Contains("armycrackers"))
				inv.TryDeleteItem(e);
		}

		s_aMine.Clear();
	}

	//------------------------------------------------------------------------------------------------
	static void Finish()
	{
		Print(string.Format("RZT craft: done, %1 failures", s_iFails));
		s_bDone = true;
	}
}

modded class RZ_Crafting
{
	//------------------------------------------------------------------------------------------------
	//! The test world leaves the Crafting outputs empty: the repo's prefabs, then the lists again.
	void RZT_Configure()
	{
		// Outputs: base-game stand-ins (the real prefabs are checked by RZT_Craft.Assets)
		m_Settings.m_sRagsPrefab = RZT_Craft.S_RAGS;
		m_Settings.m_sLockpickPrefab = RZT_Craft.S_PICK;
		m_Settings.m_sBarricadePrefab = RZT_Craft.S_PLANKS;
		m_Settings.m_sFilterPrefab = RZT_Craft.S_FILTER;
		m_Settings.m_sLurePrefab = RZT_Craft.S_LURE;
		m_Settings.m_sPlugsPrefab = RZT_Craft.S_PLUGS;
		m_Settings.m_sMapCopyPrefab = RZT_Craft.S_MAP;
		m_Settings.m_sSterilePrefab = RZT_Craft.S_STERILE;
		m_Settings.m_sCharcoalPrefab = RZT_Craft.S_CHARCOAL_OUT;
		m_Settings.m_sJerkyPrefab = RZT_Craft.JERKY;
		m_Settings.m_sStewPrefab = RZT_Craft.S_STEW;
		// Inputs: the stand-ins' names
		m_Settings.m_sClothesWords = "pants_trousers";
		m_Settings.m_sWireWords = "compass_sy183";
		m_Settings.m_sNailWords = "aspirinbottle";
		m_Settings.m_sCellWords = "aspirinbottle";
		m_Settings.m_sPadWords = "aspirinbottle";
		m_Settings.m_sSaltWords = "aspirinbottle";
		m_Settings.m_sScrapWords = "tourniquet_ussr";
		m_Settings.m_sClothWords = "gauze_02";
		m_Settings.m_sCharcoalWords = "watch_orlik38";
		Setup();
		m_bStoveBuilt = false;
		Print(string.Format("RZT craft: %1 bench recipes, %2 at a stove", m_aList.Count(), RecipesFor(true).Count()));
	}

	//------------------------------------------------------------------------------------------------
	//! Makes the named recipe for the user through FinishRecipe. False when there's no such recipe.
	bool RZT_Make(IEntity user, string name, bool stove)
	{
		foreach (RZ_Recipe r : RecipesFor(stove))
		{
			if (r.m_sName == name)
			{
				FinishRecipe(RZT_Fake.ID, null, r, user, "Test", "Done: %1.");
				return true;
			}
		}

		return false;
	}
}

modded class RZ_DoorLocks
{
	//------------------------------------------------------------------------------------------------
	void RZT_PickChance(float chance)
	{
		m_Settings.m_fPickChance = chance;
		m_Settings.m_sLockpickWords = "watch_vostok";
	}
}

modded class RZ_Interactions
{
	//------------------------------------------------------------------------------------------------
	RZ_Spot RZT_FindSpot(int id)
	{
		return FindSpot(id);
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest locked door spot within `radius`, null if none.
	RZ_Spot RZT_LockedDoorNear(vector pos, float radius)
	{
		RZ_Spot best;
		float bestDist = radius;
		foreach (RZ_Spot s : m_aSpots)
		{
			if (!s || s.m_iKind != KIND_DOOR || !m_Search.IsLocked(s))
				continue;

			float d = RZ_Util.DistXZ(s.m_vPos, pos);
			if (d < bestDist)
			{
				bestDist = d;
				best = s;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! A door spot on the nearest locked building within `radius` (null if none).
	RZ_Spot RZT_DoorSpotOnLocked(vector pos, float radius)
	{
		IEntity building = RZT_LockedBuildingNear(pos, radius);
		if (!building)
			return null;

		vector bp = building.GetOrigin();
		string type = m_Search.BuildingType(RZ_ItemHelper.LowerName(building));
		string key = RZ_Interactions.PosKey(bp, 4);
		vector at = RZ_Util.GroundPos(bp + Vector(4, 0, 0));
		AddDoorSpot(at, bp, key, type, "the test door", 0, false);
		foreach (RZ_Spot s : m_aSpots)
		{
			if (s && s.m_iKind == KIND_DOOR && s.m_sBuildingKey == key)
				return s;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest locked building within `radius`, null if none.
	IEntity RZT_LockedBuildingNear(vector pos, float radius)
	{
		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(pos, radius, buildings, 400);
		float bestDist = radius;
		IEntity found;
		foreach (IEntity b : buildings)
		{
			vector bp = b.GetOrigin();
			string type = m_Search.BuildingType(RZ_ItemHelper.LowerName(b));
			if (!m_Search.HasLock(bp, type) || !m_Search.IsBuildingLocked(bp, RZ_Interactions.PosKey(bp, 4), type))
				continue;

			float d = RZ_Util.DistXZ(bp, pos);
			if (d < bestDist)
			{
				bestDist = d;
				found = b;
			}
		}

		return found;
	}

	//------------------------------------------------------------------------------------------------
	override void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		RZT_Craft.Step(dt, this);
		super.Tick(dt, ids, players);
	}
}
