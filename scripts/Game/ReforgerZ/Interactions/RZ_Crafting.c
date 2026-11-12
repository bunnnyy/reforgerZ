//! Light crafting and water (docs/DESIGN.md, "World interactions").
//!
//! - Workbenches in garages and sheds (found by prefab name, or any placed prefab with
//!   "workbench" in its name) offer a short recipe list: improvised bandages, molotovs,
//!   noisemakers, a spear, a tin-can tripwire, a spike trap, purification tablets and
//!   purified water. Inputs are item keywords (prefab names); outputs are prefabs on the game
//!   mode. A recipe without an output prefab is hidden. More recipes can be added as data.
//! - "Use the workbench" opens the crafting screen (docs/UI.md, "The crafting screen") where
//!   UI > Craft row layout is set: every recipe, with what it takes against what you carry. A
//!   pick comes back as an index and runs the same code as the bench's actions, which stay the
//!   fallback (the first eight recipes) where the screen isn't used.
//! - Water: fill an empty bottle at a well or a pump (untreated water), then boil it on a
//!   stove or fireplace, or purify it with a tablet.
//! - The home remedy (docs/DESIGN.md, "Treatments") is brewed on a stove from alcohol and herbs,
//!   since the workbench's eight slots are taken. The water items are the hunger and thirst
//!   mod's own if it has suitable ones (docs/MOD_STACK.md); ReforgerZ doesn't model drinking.
//! - "Use the stove" opens the cooking screen (docs/UI.md, "The cooking screen") where UI > Cook
//!   row layout is set: boiled water, the home remedy and the stove recipes (Interactions >
//!   Crafting > Stove recipes, e.g. Metabolism's raw meat cooked), each with what it takes
//!   against what you carry, under the stove's status strip (FIRE, FUEL, ON THE HOB). A pick runs
//!   the same code as the stove's actions, which stay the fallback (the first four, in the object
//!   menu with the same strip and locked rows) where the screen isn't used.

//------------------------------------------------------------------------------------------------
//! A recipe: up to three inputs (keywords, comma separated, and a count), one output prefab.
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sName")]
class RZ_Recipe
{
	[Attribute("", desc: "Name in the action menu and on the crafting screen")]
	string m_sName;

	[Attribute("", desc: "First input: item keywords, comma separated")]
	string m_sIn1;

	[Attribute("1", desc: "How many of the first input")]
	int m_iN1;

	[Attribute("", desc: "Second input keywords (empty = none)")]
	string m_sIn2;

	[Attribute("1", desc: "How many of the second input")]
	int m_iN2;

	[Attribute("", desc: "Third input keywords (empty = none)")]
	string m_sIn3;

	[Attribute("1", desc: "How many of the third input")]
	int m_iN3;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "What it makes. Empty = the recipe is hidden", "et")]
	ResourceName m_sOutput;

	[Attribute("8", desc: "Seconds at the workbench or the stove")]
	float m_fSeconds;

	[Attribute("2", desc: "Heat it makes when it's done (hammering at the bench, the smell of food at a stove, where lighting the fire adds 2 more)")]
	float m_fHeat;

	// Set by code for the built-in recipes (not attributes): what the crafting screen calls each
	// input ("" = from its first keyword) and the recipe's icon in the imageset ("" = none).
	string m_sShow1;
	string m_sShow2;
	string m_sShow3;
	string m_sIcon;
	// A stove's: what it is (RZ_Crafting.STOVE_*; 0 a stove recipe, and every bench recipe) and the
	// button's verb ("Boil"; "" at the bench, where it's Craft)
	int m_iStove;
	string m_sVerb;
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Interactions > Crafting.
[BaseContainerProps()]
class RZ_CraftSettings
{
	[Attribute("1", desc: "Crafting at workbenches")]
	bool m_bEnabled;

	[Attribute("workbench,work_bench,toolbench,tool_bench,workshop_table,vise", desc: "Workbench prefab name keywords (a placed prefab with one of these in its name works too)")]
	string m_sWorkbenchWords;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Improvised bandage (from a rag or cloth)", "et")]
	ResourceName m_sBandagePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Molotov: a throwable bottle with RZ_MolotovComponent (bottle, fuel, rag)", "et")]
	ResourceName m_sMolotovPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Noisemaker: a tin rattle with RZ_NoisemakerComponent (two tins)", "et")]
	ResourceName m_sNoisemakerPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Spear: a melee weapon (knife and a pole)", "et")]
	ResourceName m_sSpearPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Tin-can tripwire kit: an item with RZ_PlaceableComponent (Tripwire) and RZ_PlaceAction (two tins, wire)", "et")]
	ResourceName m_sTripwirePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Spike trap kit: an item with RZ_PlaceableComponent (Spike trap) and RZ_PlaceAction (planks, nails)", "et")]
	ResourceName m_sSpikeTrapPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Water purification tablets (bleach or iodine)", "et")]
	ResourceName m_sTabletsPrefab;

	[Attribute("rag,cloth,shirt,towel,bandana,sheet,tshirt", desc: "Cloth keywords")]
	string m_sClothWords;

	[Attribute("bottle_empty,empty_bottle,emptybottle,glass_bottle,bottle", desc: "Bottle keywords (molotov)")]
	string m_sBottleWords;

	[Attribute("jerrycan,jerry_can,fuelcan,fuel_can,canister,fuel", desc: "Fuel keywords (molotov)")]
	string m_sFuelWords;

	[Attribute("tin_can,tincan,emptycan,empty_can,can_empty,tin_empty", desc: "Empty tin keywords")]
	string m_sTinWords;

	[Attribute("wire,string,twine,cord,rope", desc: "Wire and string keywords")]
	string m_sWireWords;

	[Attribute("knife,bayonet,machete", desc: "Blade keywords (spear)")]
	string m_sBladeWords;

	[Attribute("plank,board,stick,pole,broom,shovel_handle", desc: "Plank and pole keywords")]
	string m_sPlankWords;

	[Attribute("nail,screw", desc: "Nail keywords (spike trap)")]
	string m_sNailWords;

	[Attribute("bleach,chlorine,iodine", desc: "Keywords for what makes purification tablets")]
	string m_sBleachWords;

	[Attribute(desc: "More recipes (added after the built-in ones; sixteen in all on the crafting screen, the first eight in the action menu)")]
	ref array<ref RZ_Recipe> m_aRecipes;

	[Attribute(desc: "Stove recipes, listed after boiled water and the home remedy (sixteen in all on the cooking screen, the first four in a stove's action menu), e.g. Metabolism's raw meat in and its cooked meat out. A recipe without an output is hidden")]
	ref array<ref RZ_Recipe> m_aStoveRecipes;
}

//------------------------------------------------------------------------------------------------
class RZ_Crafting
{
	//! The read label on a workbench's spot. The client hides the bench's recipe slots behind it
	//! while it uses the crafting screen (RZ_CraftMenu.HidesSlotsOn).
	static const string BENCH_LABEL = "Use the workbench";
	static const string BENCH_LOOK = "A vice, a hammer with a taped handle, sawdust.";
	//! The read label on a stove's spot; the client hides the stove's slots behind it while it uses
	//! the cooking screen (RZ_CraftMenu.HidesSlotsOn, in its stove mode).
	static const string STOVE_LABEL = "Use the stove";
	static const string STOVE_LOOK = "Cold, but it would light. Whatever you put on it smokes a little.";
	static const string STOVE_LIT_LOOK = "The fire's going and the pot's on. Whatever you put on it smokes a little.";
	static const int STOVE_COOK = 0;		// a stove recipe (Interactions > Crafting > Stove recipes)
	static const int STOVE_BOIL = 1;		// boiling untreated water
	static const int STOVE_BREW = 2;		// brewing the home remedy

	protected static const int SLOTS = 8;				// recipes in the action menu: four choice slots, four use slots
	protected static const int SCREEN_RECIPES = 16;		// recipes on the crafting screen
	protected static const int INPUTS = 3;
	protected static const int PAYLOAD_ROOM = 5200;		// the screen's recipe records, leaving room for the header and a stove's tiles (RZ_UIFeed caps payloads at 6000)
	protected static const int STOVE_SLOTS = 4;			// a stove's choice slots in the action menu

	// What water and the stove say when you're short (their actions, and the object menu's locked rows)
	protected static const string NO_WATER_ITEMS = "You could drink from it with your hands, if you were desperate. (No water items are configured.)";
	protected static const string NO_BOTTLE = "You need an empty bottle or canteen to fill.";
	protected static const string NO_TABLETS = "You've no purification tablets. Fill it as it is and boil it later, or find tablets in a pharmacy.";
	protected static const string NOTHING_TO_BOIL = "You've nothing to boil. Fill a bottle at a well first.";
	protected static const string BREW_TAKES = "A home remedy takes something strong to drink and a handful of herbs. You haven't got both.";

	protected RZ_GameModeComponent m_GM;
	protected RZ_Interactions m_Hub;
	protected RZ_CraftSettings m_Settings;
	protected RZ_InteractionSettings m_Water;
	protected ref array<ref RZ_Recipe> m_aList = {};
	protected ref array<string> m_aBenchWords = {};
	protected ref map<int, int> m_mScreenSpot = new map<int, int>();		// player -> the workbench spot their screen shows
	protected ref map<int, int> m_mScreenSerial = new map<int, int>();	// player -> that screen's serial
	protected ref array<int> m_aScreenStove = {};						// players whose screen is a stove's (the cooking screen)
	protected ref array<ref RZ_Recipe> m_aStove = {};
	protected bool m_bStoveBuilt;
	protected int m_iNextSerial = 1;

	//------------------------------------------------------------------------------------------------
	void RZ_Crafting(RZ_GameModeComponent gm, RZ_Interactions hub, RZ_CraftSettings settings, RZ_InteractionSettings water)
	{
		m_GM = gm;
		m_Hub = hub;
		m_Settings = settings;
		m_Water = water;
	}

	//------------------------------------------------------------------------------------------------
	bool IsWorkbenchName(string name)
	{
		return m_Settings.m_bEnabled && RZ_ItemHelper.NameHas(name, m_aBenchWords);
	}

	//------------------------------------------------------------------------------------------------
	//! The built-in recipes whose output is set, then the extra ones; sixteen at most (the action
	//! menu has the first eight).
	void Setup()
	{
		RZ_ItemHelper.Words(m_Settings.m_sWorkbenchWords, m_aBenchWords);
		m_aList.Clear();
		Show(AddBuiltIn("Improvised bandage", m_Settings.m_sClothWords, 1, "", 0, "", 0, m_Settings.m_sBandagePrefab, 6, 1), "Rag or cloth", "", "", "item_bandage");
		Show(AddBuiltIn("Molotov", m_Settings.m_sBottleWords, 1, m_Settings.m_sFuelWords + RZ_ItemHelper.NOT_EMPTY, 1, m_Settings.m_sClothWords, 1, m_Settings.m_sMolotovPrefab, 10, 1), "Empty bottle", "Fuel", "Rag or cloth", "item_molotov");
		Show(AddBuiltIn("Tin rattle (noisemaker)", m_Settings.m_sTinWords, 2, m_Settings.m_sWireWords, 1, "", 0, m_Settings.m_sNoisemakerPrefab, 6, 1), "Empty tin", "Wire or string", "", "item_noisemaker");
		Show(AddBuiltIn("Spear", m_Settings.m_sBladeWords, 1, m_Settings.m_sPlankWords, 1, m_Settings.m_sWireWords, 1, m_Settings.m_sSpearPrefab, 12, 2), "Knife", "Pole or plank", "Wire or string", "");
		Show(AddBuiltIn("Tin-can tripwire", m_Settings.m_sTinWords, 2, m_Settings.m_sWireWords, 1, "", 0, m_Settings.m_sTripwirePrefab, 8, 1), "Empty tin", "Wire or string", "", "item_tripwire");
		Show(AddBuiltIn("Spike trap", m_Settings.m_sPlankWords, 2, m_Settings.m_sNailWords, 1, "", 0, m_Settings.m_sSpikeTrapPrefab, 15, 4), "Plank", "Nails", "", "item_spiketrap");
		Show(AddBuiltIn("Purification tablets", m_Settings.m_sBleachWords, 1, "", 0, "", 0, m_Settings.m_sTabletsPrefab, 8, 0), "Bleach or iodine", "", "", "item_tablets");
		Show(AddBuiltIn("Purify water", m_Water.m_sDirtyWaterWords, 1, m_Water.m_sTabletWords, 1, "", 0, m_Water.m_sCleanWaterPrefab, 5, 0), "Untreated water", "Purification tablet", "", "item_tablets");

		if (m_Settings.m_aRecipes)
		{
			foreach (RZ_Recipe r : m_Settings.m_aRecipes)
			{
				if (r && !r.m_sOutput.IsEmpty() && !r.m_sIn1.IsEmpty() && m_aList.Count() < SCREEN_RECIPES)
					m_aList.Insert(r);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A built-in recipe's names on the crafting screen (r is null when its output isn't set).
	protected void Show(RZ_Recipe r, string show1, string show2, string show3, string icon)
	{
		if (!r)
			return;

		r.m_sShow1 = show1;
		r.m_sShow2 = show2;
		r.m_sShow3 = show3;
		r.m_sIcon = icon;
	}

	//------------------------------------------------------------------------------------------------
	//! The new recipe, or null when its output isn't set (hidden) or the list is full.
	protected RZ_Recipe AddBuiltIn(string name, string in1, int n1, string in2, int n2, string in3, int n3, ResourceName output, float seconds, float heat)
	{
		if (output.IsEmpty() || m_aList.Count() >= SCREEN_RECIPES)
			return null;

		RZ_Recipe r = NewRecipe(name, in1, n1, in2, n2, in3, n3, output, seconds, heat);
		m_aList.Insert(r);
		return r;
	}

	//------------------------------------------------------------------------------------------------
	//! A recipe made in code, in no list yet.
	protected RZ_Recipe NewRecipe(string name, string in1, int n1, string in2, int n2, string in3, int n3, ResourceName output, float seconds, float heat)
	{
		RZ_Recipe r = RZ_Recipe.Cast(RZ_Util.NewWithDefaults("RZ_Recipe"));
		r.m_sName = name;
		r.m_sIn1 = in1;
		r.m_iN1 = n1;
		r.m_sIn2 = in2;
		r.m_iN2 = n2;
		r.m_sIn3 = in3;
		r.m_iN3 = n3;
		r.m_sOutput = output;
		r.m_fSeconds = seconds;
		r.m_fHeat = heat;
		return r;
	}

	// ================================================================ Workbench
	//------------------------------------------------------------------------------------------------
	//! Choice slots 0-3 are recipes 0-3, use slots 0-3 are recipes 4-7.
	void WorkbenchLabels(RZ_Spot s, out string read, out string useLabels, out string choiceLabels)
	{
		read = BENCH_LABEL;
		array<string> choice = {"", "", "", ""};
		array<string> use = {"", "", "", ""};
		foreach (int i, RZ_Recipe r : m_aList)
		{
			if (i < 4)
				choice[i] = "Make: " + r.m_sName;
			else if (i < SLOTS)
				use[i - 4] = "Make: " + r.m_sName;
		}

		choiceLabels = RZ_PuzzleLabels.Join(choice);
		useLabels = RZ_PuzzleLabels.Join(use);
	}

	//------------------------------------------------------------------------------------------------
	//! "Use the workbench": the crafting screen where it's used, else what each recipe in the
	//! action menu takes, as a notice.
	void Look(int playerId, RZ_Spot s, IEntity user)
	{
		if (!OpenScreen(playerId, s, false))
			DescribeBench(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! What each recipe in the action menu takes.
	void DescribeBench(int playerId)
	{
		if (m_aList.IsEmpty())
		{
			m_GM.NotifyPlayer(playerId, "Workbench", BENCH_LOOK + " Nothing set up to make anything. (No recipes are configured.)");
			return;
		}

		string text = BENCH_LOOK + " You could make:";
		foreach (int i, RZ_Recipe r : m_aList)
		{
			if (i < SLOTS)
				text += string.Format("\n%1: %2", r.m_sName, InputsText(r));
		}

		m_GM.NotifyPlayer(playerId, "Workbench", text);
	}

	//------------------------------------------------------------------------------------------------
	protected string InputsText(RZ_Recipe r)
	{
		string text = InputText(r.m_sIn1, r.m_iN1);
		if (!r.m_sIn2.IsEmpty() && r.m_iN2 > 0)
			text += ", " + InputText(r.m_sIn2, r.m_iN2);

		if (!r.m_sIn3.IsEmpty() && r.m_iN3 > 0)
			text += ", " + InputText(r.m_sIn3, r.m_iN3);

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! "2 x tin can" from the first keyword.
	protected string InputText(string words, int n)
	{
		array<string> list = {};
		RZ_ItemHelper.Words(words, list);
		string first = "something";
		if (!list.IsEmpty())
			first = list[0];

		return string.Format("%1 x %2", Math.Max(1, n), first);
	}

	//------------------------------------------------------------------------------------------------
	//! Input `i`'s keywords ("" = no such input).
	protected string InputWords(RZ_Recipe r, int i)
	{
		if (i == 0)
			return r.m_sIn1;

		if (i == 1 && r.m_iN2 > 0)
			return r.m_sIn2;

		if (i == 2 && r.m_iN3 > 0)
			return r.m_sIn3;

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! How many of input `i` the recipe takes, 0 when it has no such input.
	protected int InputCount(RZ_Recipe r, int i)
	{
		if (InputWords(r, i).IsEmpty())
			return 0;

		if (i == 0)
			return Math.Max(1, r.m_iN1);

		if (i == 1)
			return r.m_iN2;

		return r.m_iN3;
	}

	//------------------------------------------------------------------------------------------------
	//! What the crafting screen calls input `i`: the built-in name, else its first keyword.
	protected string InputName(RZ_Recipe r, int i)
	{
		string show = r.m_sShow3;
		if (i == 0)
			show = r.m_sShow1;
		else if (i == 1)
			show = r.m_sShow2;

		if (!show.IsEmpty())
			return show;

		array<string> list = {};
		RZ_ItemHelper.Words(InputWords(r, i), list);
		string first = "something";
		if (!list.IsEmpty())
			first = list[0];

		first.Replace("_", " ");
		return RZ_Util.Capitalize(first);
	}

	//------------------------------------------------------------------------------------------------
	//! The carried items the recipe would use, none counted for two inputs: `haves` holds how many
	//! of each input were found (at most what it takes), `picked` the items themselves.
	protected void Allocate(IEntity user, RZ_Recipe r, notnull array<int> haves, notnull array<IEntity> picked)
	{
		haves.Clear();
		picked.Clear();
		array<IEntity> items = {};
		SCR_InventoryStorageManagerComponent inv;
		if (user)
			inv = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));

		if (inv)
			inv.GetItems(items);

		for (int i = 0; i < INPUTS; i++)
		{
			int want = InputCount(r, i);
			int found = 0;
			array<string> words = {};
			RZ_ItemHelper.Words(InputWords(r, i), words);
			foreach (IEntity item : items)
			{
				if (found >= want)
					break;

				if (picked.Contains(item) || !RZ_ItemHelper.Matches(item, words))
					continue;

				picked.Insert(item);
				found++;
			}

			haves.Insert(found);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every input found in full (after Allocate).
	protected bool AllFound(RZ_Recipe r, array<int> haves)
	{
		for (int i = 0; i < INPUTS; i++)
		{
			if (i >= haves.Count() || haves[i] < InputCount(r, i))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasInputs(IEntity user, RZ_Recipe r)
	{
		array<int> haves = {};
		array<IEntity> picked = {};
		Allocate(user, r, haves, picked);
		return AllFound(r, haves);
	}

	//------------------------------------------------------------------------------------------------
	void HandleRecipe(int playerId, RZ_Spot s, int idx, IEntity user)
	{
		if (!m_Settings.m_bEnabled || idx < 0 || idx >= m_aList.Count())
			return;

		RZ_Recipe r = m_aList[idx];
		if (!HasInputs(user, r))
		{
			m_GM.NotifyPlayer(playerId, "Workbench", string.Format("%1 takes %2. You haven't got it all.", r.m_sName, InputsText(r)));
			return;
		}

		m_Hub.StartTask(playerId, user, RZ_Interactions.TASK_CRAFT, s.m_iId, idx, r.m_fSeconds, "the work at the bench", false);
		m_GM.NotifyPlayer(playerId, "Workbench", string.Format("You clear a space and start on the %1. About %2 seconds.", r.m_sName, RZ_Util.RoundToInt(r.m_fSeconds)));
	}

	//------------------------------------------------------------------------------------------------
	//! The work is done. With the crafting screen open, what it says goes into the screen.
	void CompleteRecipe(int playerId, RZ_Spot s, int idx, IEntity user)
	{
		if (idx < 0 || idx >= m_aList.Count())
			return;

		bool captured = CaptureFor(playerId);
		FinishRecipe(playerId, s, m_aList[idx], user, "Workbench", "Done: %1. Not pretty, but it'll do.");
		Release(playerId, captured);
	}

	//------------------------------------------------------------------------------------------------
	//! Checks the inputs again and takes each with TryDeleteItem; the output is given only when
	//! every one came out. `done` says it's ready (%1 the recipe's name).
	protected void FinishRecipe(int playerId, RZ_Spot s, RZ_Recipe r, IEntity user, string title, string done)
	{
		if (!r || !user)
			return;

		array<int> haves = {};
		array<IEntity> picked = {};
		Allocate(user, r, haves, picked);
		if (!AllFound(r, haves))
		{
			m_GM.NotifyPlayer(playerId, title, "Something you needed is gone from your pack.");
			return;
		}

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
		int taken = 0;
		foreach (IEntity item : picked)
		{
			if (inv && inv.TryDeleteItem(item))
				taken++;
		}

		if (taken < picked.Count())
		{
			RZ_Log.Info(string.Format("Crafting: %1 for player %2 stopped, %3 of %4 inputs came out of the pack.", r.m_sName, playerId, taken, picked.Count()));
			m_GM.NotifyPlayer(playerId, title, "Something won't come out of your pack, and the work falls apart in your hands.");
			return;
		}

		vector at = user.GetOrigin();
		if (s)
			at = s.m_vPos;

		RZ_ItemHelper.Give(r.m_sOutput, user, at);
		if (r.m_fHeat > 0)
			m_GM.GetHeat().AddHeat(at, r.m_fHeat);

		m_GM.NotifyPlayer(playerId, title, string.Format(done, r.m_sName));
	}

	// ================================================================ The crafting screen
	//------------------------------------------------------------------------------------------------
	//! Opens the crafting screen on this workbench for the player (docs/UI.md, "The crafting
	//! screen"), or with `stove` the cooking screen on this stove ("The cooking screen"). False
	//! when it isn't used (no row layout, no UI component, nothing to make): the caller shows the
	//! notice. At the server's cap of open screens the client is told to use the actions for a minute.
	protected bool OpenScreen(int playerId, RZ_Spot s, bool stove)
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		RZ_ScreenGuard guard = m_GM.GetScreenGuard();
		if (!ui || !guard || !s || RecipesFor(stove).IsEmpty())
			return false;

		ResourceName rowLayout = RZ_UISettings.Get().m_sCraftRowLayout;
		int kind = RZ_ScreenGuard.KIND_CRAFT;
		int payloadKind = RZ_UIFeed.PAYLOAD_CRAFT;
		if (stove)
		{
			rowLayout = RZ_UISettings.Get().m_sCookRowLayout;
			kind = RZ_ScreenGuard.KIND_COOK;
			payloadKind = RZ_UIFeed.PAYLOAD_COOK;
		}

		if (rowLayout.IsEmpty())
			return false;

		if (!guard.Open(playerId, kind, s.m_Body, m_Hub.Title(s), -1))
		{
			ui.SendPayload(payloadKind, "F|busy");
			return false;
		}

		m_mScreenSpot.Set(playerId, s.m_iId);
		m_mScreenSerial.Set(playerId, m_iNextSerial);
		m_aScreenStove.RemoveItem(playerId);
		if (stove)
			m_aScreenStove.Insert(playerId);

		m_iNextSerial++;
		SendScreen(playerId, "");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The workbench's recipes, or with `stove` a stove's.
	protected array<ref RZ_Recipe> RecipesFor(bool stove)
	{
		if (!stove)
			return m_aList;

		BuildStove();
		return m_aStove;
	}

	//------------------------------------------------------------------------------------------------
	//! The player's open screen is a stove's (the cooking screen).
	protected bool IsStoveScreen(int playerId)
	{
		return m_aScreenStove.Contains(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! The player has the crafting screen open (at any workbench) or the cooking screen (at any stove).
	//! Forgotten here when the guard now holds another screen for them (one replaced it).
	bool HasScreen(int playerId)
	{
		if (!m_mScreenSpot.Contains(playerId))
			return false;

		int kind = RZ_ScreenGuard.KIND_CRAFT;
		if (IsStoveScreen(playerId))
			kind = RZ_ScreenGuard.KIND_COOK;

		RZ_ScreenGuard guard = m_GM.GetScreenGuard();
		if (guard && guard.IsOpen(playerId, kind))
			return true;

		OnScreenGone(playerId);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The screen guard or the player closed it: forget it.
	void OnScreenGone(int playerId)
	{
		m_mScreenSpot.Remove(playerId);
		m_mScreenSerial.Remove(playerId);
		m_aScreenStove.RemoveItem(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Closes the player's screen from here (the bench went): the client and the guard.
	protected void CloseScreen(int playerId, string reason)
	{
		if (!HasScreen(playerId))
			return;

		int payloadKind = RZ_UIFeed.PAYLOAD_CRAFT;
		if (IsStoveScreen(playerId))
			payloadKind = RZ_UIFeed.PAYLOAD_COOK;

		OnScreenGone(playerId);
		RZ_ScreenGuard guard = m_GM.GetScreenGuard();
		if (guard)
			guard.Close(playerId);

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		if (ui)
			ui.SendPayload(payloadKind, "X|" + reason);
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
	//! The work stopped early (the player moved or was hurt): the screen shows it's over.
	void OnTaskStopped(int playerId)
	{
		if (HasScreen(playerId))
			SendScreen(playerId, "");
	}

	//------------------------------------------------------------------------------------------------
	//! A pick on the crafting or the cooking screen (RZ_PlayerUIComponent.RpcAsk_Craft). Checked
	//! here: the screen is open on this serial, the bench or stove is still there and the player
	//! beside it, nothing else is under way; then the recipe runs through the same code as the
	//! bench's or the stove's actions (its inputs checked again when the work is done), and the
	//! screen redraws with what they would have said.
	void DoCraft(int playerId, int serial, int index)
	{
		int spotId;
		int current;
		if (!m_mScreenSpot.Find(playerId, spotId) || !m_mScreenSerial.Find(playerId, current) || serial != current)
			return;

		bool stove = IsStoveScreen(playerId);
		int kind = RZ_ScreenGuard.KIND_CRAFT;
		if (stove)
			kind = RZ_ScreenGuard.KIND_COOK;

		RZ_ScreenGuard guard = m_GM.GetScreenGuard();
		if (!guard || !guard.IsOpen(playerId, kind))
		{
			OnScreenGone(playerId);
			return;
		}

		guard.Touch(playerId);
		RZ_Spot s = m_Hub.FindSpot(spotId);
		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!s || !user || !RZ_Util.IsCharacterAlive(user))
		{
			CloseScreen(playerId, "gone");
			return;
		}

		if (index < 0 || index >= RecipesFor(stove).Count() || RZ_Util.DistXZ(user.GetOrigin(), s.m_vPos) > RZ_DialogueSettings.Get().m_fRange)
			return;

		if (m_Hub.HasTask(playerId))
		{
			SendScreen(playerId, "You're already at work on something. Finish it first, or step away to stop.");
			return;
		}

		RZ_ScreenCapture.Begin(playerId);
		if (stove)
			HandleStove(playerId, s, index, user);
		else
			HandleRecipe(playerId, s, index, user);

		Release(playerId, true);
	}

	//------------------------------------------------------------------------------------------------
	//! The screen's payload: B|serial|title|look|status|ready|recipes|town|hint|lit, W|recipe|seconds
	//! left|seconds in all while one is under way here, a stove's status tiles (S records,
	//! RZ_ObjectStatus), then per recipe R|index|name|prefab|icon|seconds|heat|ready|verb and
	//! N|index|name|have|need per input (have counts what you carry, at most need). The verb is the
	//! stove's ("Boil", "Brew", "Cook"); the bench leaves it empty (Craft). Lit: the stove's fire is going.
	protected void SendScreen(int playerId, string status)
	{
		int spotId;
		int serial;
		if (!m_mScreenSpot.Find(playerId, spotId) || !m_mScreenSerial.Find(playerId, serial))
			return;

		RZ_Spot s = m_Hub.FindSpot(spotId);
		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		if (!s || !user || !ui)
		{
			CloseScreen(playerId, "gone");
			return;
		}

		// The recipe under way here, if any
		bool stove = IsStoveScreen(playerId);
		array<ref RZ_Recipe> list = RecipesFor(stove);
		int working = -1;
		float left = 0;
		float total = 0;
		RZ_TimedTask task = m_Hub.GetTask(playerId);
		if (task && task.m_iSpot == spotId)
			working = WorkingIndex(task, stove);

		if (working >= 0 && working < list.Count())
		{
			left = Math.Max(0, task.m_fLeft);
			total = Math.Max(1, task.m_fTotal);
		}
		else
		{
			working = -1;
		}

		string rows = "";
		int ready = 0;
		int shown = 0;
		array<int> haves = {};
		array<IEntity> picked = {};
		foreach (int i, RZ_Recipe r : list)
		{
			Allocate(user, r, haves, picked);
			int readyFlag = 0;
			if (AllFound(r, haves))
				readyFlag = 1;

			string rec = string.Format("\nR|%1|%2|%3|%4|%5|%6|%7|%8", i, RZ_UIText.Clean(r.m_sName, 60), r.m_sOutput, r.m_sIcon,
				RZ_Util.RoundToInt(r.m_fSeconds), RZ_Util.RoundToInt(r.m_fHeat), readyFlag, RZ_UIText.Clean(r.m_sVerb, 12));
			for (int k = 0; k < INPUTS; k++)
			{
				int need = InputCount(r, k);
				if (need > 0)
					rec += string.Format("\nN|%1|%2|%3|%4", i, RZ_UIText.Clean(InputName(r, k), 40), haves[k], need);
			}

			if (rows.Length() + rec.Length() > PAYLOAD_ROOM)
				break;

			rows += rec;
			ready += readyFlag;
			shown++;
		}

		// A stove's status strip (FIRE, FUEL, ON THE HOB) and whether its fire is going
		string look = BENCH_LOOK;
		int payloadKind = RZ_UIFeed.PAYLOAD_CRAFT;
		RZ_ObjectStatus st = new RZ_ObjectStatus();
		int lit = 0;
		if (stove)
		{
			look = STOVE_LOOK;
			payloadKind = RZ_UIFeed.PAYLOAD_COOK;
			StoveStatus(playerId, s, st);
			if (StoveWork(playerId, spotId))
			{
				look = STOVE_LIT_LOOK;
				lit = 1;
			}
		}

		string payload = string.Format("B|%1|%2|%3|%4|%5|%6|%7|%8|%9", serial, RZ_UIText.Clean(m_Hub.Title(s), 40), look, RZ_UIText.Clean(status, 400), ready, shown,
			RZ_UIText.Clean(TownOf(s.m_vPos), 40), RZ_UIText.Clean(st.m_sHint, 120), lit);
		if (working >= 0)
			payload += string.Format("\nW|%1|%2|%3", working, left, total);

		ui.SendPayload(payloadKind, payload + st.Records() + rows);
	}

	//------------------------------------------------------------------------------------------------
	//! The town a bench or stove is in, "" out in the open (as the object menu's).
	protected string TownOf(vector pos)
	{
		RZ_Settlement here = m_GM.GetSettlement(m_GM.FindNearestSettlement(pos));
		if (here && RZ_Util.DistXZ(here.m_vCenter, pos) < 600)
			return here.m_sName;

		return "";
	}

	// ================================================================ The stove's status strip
	//------------------------------------------------------------------------------------------------
	//! Boiling, brewing or a stove recipe: work that lights a stove's fire.
	protected bool IsStoveTask(RZ_TimedTask t)
	{
		return t.m_iKind == RZ_Interactions.TASK_BOIL || t.m_iKind == RZ_Interactions.TASK_BREW || t.m_iKind == RZ_Interactions.TASK_COOK;
	}

	//------------------------------------------------------------------------------------------------
	//! The work on this stove now: the player's own if it's here, else anyone's; null: its fire is cold.
	protected RZ_TimedTask StoveWork(int playerId, int spotId)
	{
		RZ_TimedTask own = m_Hub.GetTask(playerId);
		if (own && own.m_iSpot == spotId && IsStoveTask(own))
			return own;

		array<int> ids = {};
		GetGame().GetPlayerManager().GetPlayers(ids);
		foreach (int id : ids)
		{
			RZ_TimedTask t = m_Hub.GetTask(id);
			if (t && t.m_iSpot == spotId && IsStoveTask(t))
				return t;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! A stove's tiles (RZ_ObjectStatus), on the cooking screen and in the object menu: FIRE (lit
	//! while something's on it), FUEL (not counted: there's always wood and scraps to hand) and ON
	//! THE HOB (what's cooking, counting down on the client).
	void StoveStatus(int playerId, RZ_Spot s, notnull RZ_ObjectStatus st)
	{
		RZ_TimedTask work = StoveWork(playerId, s.m_iId);
		if (work)
			st.Add("FIRE", "LIT", RZ_ObjectStatus.STATE_OK);
		else
			st.Add("FIRE", "COLD", RZ_ObjectStatus.STATE_DIM);

		st.Add("FUEL", "ENOUGH", RZ_ObjectStatus.STATE_OK);
		array<ref RZ_Recipe> list = RecipesFor(true);
		int idx = -1;
		if (work)
			idx = WorkingIndex(work, true);

		if (idx < 0 || idx >= list.Count())
		{
			st.Add("ON THE HOB", "EMPTY", RZ_ObjectStatus.STATE_DIM);
			return;
		}

		string name = list[idx].m_sName;
		name.ToUpper();
		st.Add("ON THE HOB", name, RZ_ObjectStatus.STATE_PART, RZ_Util.RoundToInt(Math.Ceil(work.m_fLeft)));
	}

	//------------------------------------------------------------------------------------------------
	//! A stove's row `option` (its place in the stove's list) can't go now: what Boil, Brew or Cook
	//! would say, and SHORT N (NEEDS WATER for boiling).
	bool StoveLock(int option, IEntity user, out string reason, out string tag)
	{
		array<ref RZ_Recipe> list = RecipesFor(true);
		if (option < 0 || option >= list.Count())
			return false;

		RZ_Recipe r = list[option];
		array<int> haves = {};
		array<IEntity> picked = {};
		Allocate(user, r, haves, picked);
		if (AllFound(r, haves))
			return false;

		int missing = 0;
		for (int i = 0; i < INPUTS; i++)
		{
			if (i < haves.Count() && haves[i] < InputCount(r, i))
				missing++;
		}

		tag = string.Format("SHORT %1", missing);
		if (r.m_iStove == STOVE_BOIL)
		{
			reason = NOTHING_TO_BOIL;
			tag = "NEEDS WATER";
		}
		else if (r.m_iStove == STOVE_BREW)
		{
			reason = BREW_TAKES;
		}
		else
		{
			reason = string.Format("%1 takes %2. You haven't got it all.", r.m_sName, InputsText(r));
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A well's tiles (RZ_ObjectStatus): the water, and whether you carry a bottle and tablets.
	void WellStatus(IEntity user, notnull RZ_ObjectStatus st)
	{
		st.Add("WATER", "UNTREATED", RZ_ObjectStatus.STATE_PART);
		st.YesNo("BOTTLE", RZ_ItemHelper.Has(user, m_Water.m_sContainerWords));
		if (m_Water.m_sCleanWaterPrefab.IsEmpty())
			st.m_sHint = "Boil it on a stove before you drink.";
		else if (RZ_ItemHelper.Has(user, m_Water.m_sTabletWords))
			st.Add("TABLETS", "YES", RZ_ObjectStatus.STATE_OK);
		else
			st.Add("TABLETS", "NO", RZ_ObjectStatus.STATE_DIM);

		if (st.m_sHint.IsEmpty())
			st.m_sHint = "Boil it on a stove or purify it before you drink.";
	}

	//------------------------------------------------------------------------------------------------
	//! A well's row can't be done now: what HandleWell would say instead.
	bool WellLock(int option, IEntity user, out string reason, out string tag)
	{
		if (m_Water.m_sDirtyWaterPrefab.IsEmpty())
		{
			reason = NO_WATER_ITEMS;
			tag = "NOT SET UP";
			return true;
		}

		if (!RZ_ItemHelper.Has(user, m_Water.m_sContainerWords))
		{
			reason = NO_BOTTLE;
			tag = "NEEDS BOTTLE";
			return true;
		}

		if (option == 1 && (m_Water.m_sCleanWaterPrefab.IsEmpty() || !RZ_ItemHelper.Has(user, m_Water.m_sTabletWords)))
		{
			reason = NO_TABLETS;
			tag = "NEEDS TABLETS";
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Which recipe of the screen's list a timed task is (-1 none): the bench's work, or on a stove
	//! the boiling, the brewing or a stove recipe.
	protected int WorkingIndex(RZ_TimedTask task, bool stove)
	{
		if (!stove)
		{
			if (task.m_iKind == RZ_Interactions.TASK_CRAFT)
				return task.m_iArg;

			return -1;
		}

		if (task.m_iKind == RZ_Interactions.TASK_COOK)
			return task.m_iArg;

		int want = -1;
		if (task.m_iKind == RZ_Interactions.TASK_BOIL)
			want = STOVE_BOIL;
		else if (task.m_iKind == RZ_Interactions.TASK_BREW)
			want = STOVE_BREW;

		if (want < 0)
			return -1;

		foreach (int i, RZ_Recipe r : RecipesFor(true))
		{
			if (r.m_iStove == want)
				return i;
		}

		return -1;
	}

	// ================================================================ Water
	//------------------------------------------------------------------------------------------------
	//! 0 fill a bottle (untreated), 1 fill and purify it (a tablet).
	void HandleWell(int playerId, RZ_Spot s, int option, IEntity user)
	{
		string title = m_Hub.Title(s);
		if (m_Water.m_sDirtyWaterPrefab.IsEmpty())
		{
			m_GM.NotifyPlayer(playerId, title, NO_WATER_ITEMS);
			return;
		}

		if (!RZ_ItemHelper.Has(user, m_Water.m_sContainerWords))
		{
			m_GM.NotifyPlayer(playerId, title, NO_BOTTLE);
			return;
		}

		bool purify = option == 1;
		if (purify && (m_Water.m_sCleanWaterPrefab.IsEmpty() || !RZ_ItemHelper.Has(user, m_Water.m_sTabletWords)))
		{
			m_GM.NotifyPlayer(playerId, title, NO_TABLETS);
			return;
		}

		RZ_ItemHelper.Take(user, m_Water.m_sContainerWords, 1);
		if (purify)
		{
			RZ_ItemHelper.Take(user, m_Water.m_sTabletWords, 1);
			RZ_ItemHelper.Give(m_Water.m_sCleanWaterPrefab, user, s.m_vPos);
			m_GM.NotifyPlayer(playerId, title, "You fill the bottle, drop a tablet in and shake it. It tastes like a swimming pool. It's safe.");
			return;
		}

		RZ_ItemHelper.Give(m_Water.m_sDirtyWaterPrefab, user, s.m_vPos);
		m_GM.NotifyPlayer(playerId, title, "Cold and clear. Don't trust it: boil it on a stove or purify it before you drink.");
	}

	// ================================================================ The stove
	//------------------------------------------------------------------------------------------------
	//! The stove's list, made once: boiled water (with Clean water set), the home remedy (with its
	//! prefab set), then the stove recipes whose output is set; sixteen at most.
	protected void BuildStove()
	{
		if (m_bStoveBuilt)
			return;

		m_bStoveBuilt = true;
		m_aStove.Clear();
		if (!m_Water.m_sCleanWaterPrefab.IsEmpty() && !m_Water.m_sDirtyWaterWords.IsEmpty())
		{
			RZ_Recipe boil = NewRecipe("Boiled water", m_Water.m_sDirtyWaterWords, 1, "", 0, "", 0, m_Water.m_sCleanWaterPrefab, m_Water.m_fBoilSeconds, 3);
			boil.m_sShow1 = "Untreated water";
			boil.m_iStove = STOVE_BOIL;
			boil.m_sVerb = "Boil";
			m_aStove.Insert(boil);
		}

		RZ_Infection infection = m_GM.GetInfection();
		ResourceName remedy = RemedyPrefab();
		if (infection && !remedy.IsEmpty())
		{
			RZ_InfectionSettings med = infection.GetSettings();
			RZ_Recipe brew = NewRecipe("Home remedy", med.m_sAlcoholWords, 1, med.m_sHerbWords, 1, "", 0, remedy, med.m_fBrewSeconds, 2);
			brew.m_sShow1 = "Something strong to drink";
			brew.m_sShow2 = "Herbs";
			brew.m_sIcon = "item_med";
			brew.m_iStove = STOVE_BREW;
			brew.m_sVerb = "Brew";
			m_aStove.Insert(brew);
		}

		if (!m_Settings.m_aStoveRecipes)
			return;

		foreach (RZ_Recipe r : m_Settings.m_aStoveRecipes)
		{
			if (!r || r.m_sOutput.IsEmpty() || r.m_sIn1.IsEmpty() || m_aStove.Count() >= SCREEN_RECIPES)
				continue;

			r.m_iStove = STOVE_COOK;
			r.m_sVerb = "Cook";
			r.m_sIcon = "need_food";
			m_aStove.Insert(r);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A stove's choice slots: the first four of its list ("Boil water", "Brew a home remedy
	//! (alcohol, herbs)", "Cook: ..."); the cooking screen holds them all where it's used.
	string StoveLabels()
	{
		array<string> choice = {"", "", "", ""};
		foreach (int i, RZ_Recipe r : RecipesFor(true))
		{
			if (i >= STOVE_SLOTS)
				break;

			if (r.m_iStove == STOVE_BOIL)
				choice[i] = "Boil water";
			else if (r.m_iStove == STOVE_BREW)
				choice[i] = "Brew a home remedy (alcohol, herbs)";
			else
				choice[i] = "Cook: " + r.m_sName;
		}

		return RZ_PuzzleLabels.Join(choice);
	}

	//------------------------------------------------------------------------------------------------
	//! A stove's read opens the cooking screen: UI > Cook row layout set and something to cook.
	bool StoveScreenOn()
	{
		return !RZ_UISettings.Get().m_sCookRowLayout.IsEmpty() && !RecipesFor(true).IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! "Use the stove": the cooking screen. False when it isn't used: the caller says what the stove is.
	bool UseStove(int playerId, RZ_Spot s, IEntity user)
	{
		return OpenScreen(playerId, s, true);
	}

	//------------------------------------------------------------------------------------------------
	protected ResourceName RemedyPrefab()
	{
		RZ_Infection infection = m_GM.GetInfection();
		if (!infection)
			return "";

		return infection.MedPrefab(RZ_EMed.REMEDY);
	}

	//------------------------------------------------------------------------------------------------
	//! Item `option` of the stove's list, from its choice slot or the cooking screen: boil water,
	//! brew a home remedy, or cook a stove recipe.
	void HandleStove(int playerId, RZ_Spot s, int option, IEntity user)
	{
		array<ref RZ_Recipe> list = RecipesFor(true);
		if (!s || option < 0 || option >= list.Count())
			return;

		RZ_Recipe r = list[option];
		if (r.m_iStove == STOVE_BOIL)
			StartBoil(playerId, s, user);
		else if (r.m_iStove == STOVE_BREW)
			StartBrew(playerId, s, user);
		else
			StartCook(playerId, s, option, user);
	}

	//------------------------------------------------------------------------------------------------
	//! A fire lit: heat, and a campfire spot for strangers for half an hour (docs/ERRANDS.md, "Strangers").
	protected void LightFire(RZ_Spot s)
	{
		m_GM.GetHeat().AddHeat(s.m_vPos, 2);
		if (m_GM.GetErrands())
			m_GM.GetErrands().GetStrangers().NoteFire(s.m_vPos);
	}

	// ================================================================ Boiling water
	//------------------------------------------------------------------------------------------------
	protected void StartBoil(int playerId, RZ_Spot s, IEntity user)
	{
		string title = m_Hub.Title(s);
		if (m_Water.m_sCleanWaterPrefab.IsEmpty() || !RZ_ItemHelper.Has(user, m_Water.m_sDirtyWaterWords))
		{
			m_GM.NotifyPlayer(playerId, title, NOTHING_TO_BOIL);
			return;
		}

		LightFire(s);
		m_Hub.StartTask(playerId, user, RZ_Interactions.TASK_BOIL, s.m_iId, 0, m_Water.m_fBoilSeconds, "the boiling", false);
		m_GM.NotifyPlayer(playerId, title, string.Format("You get a small fire going under the pot. About %1 seconds. The smoke goes up the chimney, and someone may see it.", RZ_Util.RoundToInt(m_Water.m_fBoilSeconds)));
	}

	//------------------------------------------------------------------------------------------------
	//! The boiling is done. With the cooking screen open, what it says goes into the screen.
	void CompleteBoil(int playerId, RZ_Spot s, IEntity user)
	{
		bool captured = CaptureFor(playerId);
		if (RZ_ItemHelper.Take(user, m_Water.m_sDirtyWaterWords, 1) > 0)
		{
			vector at = user.GetOrigin();
			if (s)
				at = s.m_vPos;

			RZ_ItemHelper.Give(m_Water.m_sCleanWaterPrefab, user, at);
			m_GM.GetHeat().AddHeat(at, 3);
			m_GM.NotifyPlayer(playerId, "Boiled water", "It rolls for a minute, and you let it cool. Flat and warm, and safe.");
		}

		Release(playerId, captured);
	}

	// ================================================================ The home remedy
	//------------------------------------------------------------------------------------------------
	protected void StartBrew(int playerId, RZ_Spot s, IEntity user)
	{
		string title = m_Hub.Title(s);
		RZ_Infection infection = m_GM.GetInfection();
		if (!infection || RemedyPrefab().IsEmpty())
			return;

		RZ_InfectionSettings med = infection.GetSettings();
		if (!RZ_ItemHelper.Has(user, med.m_sAlcoholWords) || !RZ_ItemHelper.Has(user, med.m_sHerbWords))
		{
			m_GM.NotifyPlayer(playerId, title, BREW_TAKES);
			return;
		}

		m_GM.GetHeat().AddHeat(s.m_vPos, 2);
		m_Hub.StartTask(playerId, user, RZ_Interactions.TASK_BREW, s.m_iId, 0, med.m_fBrewSeconds, "the brewing", false);
		m_GM.NotifyPlayer(playerId, title, string.Format("Herbs in the pot, the bottle after them, and a low flame. About %1 seconds. It smells like a grandmother's kitchen and a bar fight.", RZ_Util.RoundToInt(med.m_fBrewSeconds)));
	}

	//------------------------------------------------------------------------------------------------
	//! The brewing is done. With the cooking screen open, what it says goes into the screen.
	void CompleteBrew(int playerId, RZ_Spot s, IEntity user)
	{
		RZ_Infection infection = m_GM.GetInfection();
		if (!infection)
			return;

		bool captured = CaptureFor(playerId);
		RZ_InfectionSettings med = infection.GetSettings();
		if (!RZ_ItemHelper.Has(user, med.m_sAlcoholWords) || !RZ_ItemHelper.Has(user, med.m_sHerbWords))
		{
			m_GM.NotifyPlayer(playerId, "Home remedy", "Something you needed is gone from your pack.");
			Release(playerId, captured);
			return;
		}

		RZ_ItemHelper.Take(user, med.m_sAlcoholWords, 1);
		RZ_ItemHelper.Take(user, med.m_sHerbWords, 1);
		vector at = user.GetOrigin();
		if (s)
			at = s.m_vPos;

		RZ_ItemHelper.Give(RemedyPrefab(), user, at);
		m_GM.NotifyPlayer(playerId, "Home remedy", "A bottle of something brown and bitter. It won't cure anything. It might buy you a few minutes, and it'll go straight to your head.");
		Release(playerId, captured);
	}

	// ================================================================ Stove recipes
	//------------------------------------------------------------------------------------------------
	//! A stove recipe (Interactions > Crafting > Stove recipes): the inputs checked, the fire lit,
	//! the timed work started. `idx` is its place in the stove's list.
	protected void StartCook(int playerId, RZ_Spot s, int idx, IEntity user)
	{
		RZ_Recipe r = RecipesFor(true)[idx];
		string title = m_Hub.Title(s);
		if (!HasInputs(user, r))
		{
			m_GM.NotifyPlayer(playerId, title, string.Format("%1 takes %2. You haven't got it all.", r.m_sName, InputsText(r)));
			return;
		}

		LightFire(s);
		m_Hub.StartTask(playerId, user, RZ_Interactions.TASK_COOK, s.m_iId, idx, r.m_fSeconds, "the cooking", false);
		m_GM.NotifyPlayer(playerId, title, string.Format("You get a small fire going and start on the %1. About %2 seconds. The smoke goes up the chimney, and someone may see it.", r.m_sName, RZ_Util.RoundToInt(r.m_fSeconds)));
	}

	//------------------------------------------------------------------------------------------------
	//! The cooking is done: the inputs checked again and taken, the output given (FinishRecipe).
	//! With the cooking screen open, what it says goes into the screen.
	void CompleteCook(int playerId, RZ_Spot s, int idx, IEntity user)
	{
		array<ref RZ_Recipe> list = RecipesFor(true);
		if (idx < 0 || idx >= list.Count())
			return;

		bool captured = CaptureFor(playerId);
		FinishRecipe(playerId, s, list[idx], user, "Stove", "Done: %1. For a minute the place smells like somebody's kitchen.");
		Release(playerId, captured);
	}
}
