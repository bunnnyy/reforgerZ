//! Light crafting and water (docs/WORLD.md, "World interactions").
//!
//! - Workbenches in garages and sheds (found by prefab name, or any placed prefab with
//!   "workbench" in its name) offer a short recipe list: improvised bandages, molotovs,
//!   noisemakers, a spear, a tin-can tripwire, a spike trap, purification tablets and
//!   purified water. Inputs are item keywords (prefab names); outputs are prefabs on the game
//!   mode. A recipe without an output prefab is hidden. More recipes can be added as data.
//! - Water: fill an empty bottle at a well or a pump (untreated water), then boil it on a
//!   stove or fireplace, or purify it with a tablet. The water items are the hunger and thirst
//!   mod's own if it has suitable ones (docs/MOD_STACK.md); ReforgerZ doesn't model drinking.

//------------------------------------------------------------------------------------------------
//! A recipe: up to three inputs (keywords, comma separated, and a count), one output prefab.
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sName")]
class RZ_Recipe
{
	[Attribute("", desc: "Name in the action menu")]
	string m_sName;

	[Attribute("", desc: "First input: item keywords, comma separated")]
	string m_sIn1;

	[Attribute("1", desc: "How many of the first input")]
	int m_iN1 = 1;

	[Attribute("", desc: "Second input keywords (empty = none)")]
	string m_sIn2;

	[Attribute("1", desc: "How many of the second input")]
	int m_iN2 = 1;

	[Attribute("", desc: "Third input keywords (empty = none)")]
	string m_sIn3;

	[Attribute("1", desc: "How many of the third input")]
	int m_iN3 = 1;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "What it makes. Empty = the recipe is hidden", "et")]
	ResourceName m_sOutput;

	[Attribute("8", desc: "Seconds at the workbench")]
	float m_fSeconds = 8;

	[Attribute("2", desc: "Heat it makes (hammering)")]
	float m_fHeat = 2;
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Interactions > Crafting.
[BaseContainerProps()]
class RZ_CraftSettings
{
	[Attribute("1", desc: "Crafting at workbenches")]
	bool m_bEnabled = true;

	[Attribute("workbench,work_bench,toolbench,tool_bench,workshop_table,vise", desc: "Workbench prefab name keywords (a placed prefab with one of these in its name works too)")]
	string m_sWorkbenchWords = "workbench,work_bench,toolbench,tool_bench,workshop_table,vise";

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
	string m_sClothWords = "rag,cloth,shirt,towel,bandana,sheet,tshirt";

	[Attribute("bottle_empty,empty_bottle,emptybottle,glass_bottle,bottle", desc: "Bottle keywords (molotov)")]
	string m_sBottleWords = "bottle_empty,empty_bottle,emptybottle,glass_bottle,bottle";

	[Attribute("jerrycan,jerry_can,fuelcan,fuel_can,canister,fuel", desc: "Fuel keywords (molotov)")]
	string m_sFuelWords = "jerrycan,jerry_can,fuelcan,fuel_can,canister,fuel";

	[Attribute("tin_can,tincan,emptycan,empty_can,can_empty,tin_empty", desc: "Empty tin keywords")]
	string m_sTinWords = "tin_can,tincan,emptycan,empty_can,can_empty,tin_empty";

	[Attribute("wire,string,twine,cord,rope", desc: "Wire and string keywords")]
	string m_sWireWords = "wire,string,twine,cord,rope";

	[Attribute("knife,bayonet,machete", desc: "Blade keywords (spear)")]
	string m_sBladeWords = "knife,bayonet,machete";

	[Attribute("plank,board,stick,pole,broom,shovel_handle", desc: "Plank and pole keywords")]
	string m_sPlankWords = "plank,board,stick,pole,broom,shovel_handle";

	[Attribute("nail,screw", desc: "Nail keywords (spike trap)")]
	string m_sNailWords = "nail,screw";

	[Attribute("bleach,chlorine,iodine", desc: "Keywords for what makes purification tablets")]
	string m_sBleachWords = "bleach,chlorine,iodine";

	[Attribute(desc: "More recipes (added after the built-in ones; at most eight are shown at a workbench)")]
	ref array<ref RZ_Recipe> m_aRecipes;
}

//------------------------------------------------------------------------------------------------
class RZ_Crafting
{
	protected static const int SLOTS = 8;

	protected RZ_GameModeComponent m_GM;
	protected RZ_Interactions m_Hub;
	protected RZ_CraftSettings m_Settings;
	protected RZ_InteractionSettings m_Water;
	protected ref array<ref RZ_Recipe> m_aList = {};
	protected ref array<string> m_aBenchWords = {};

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
	//! The built-in recipes whose output is set, then the extra ones; eight at most.
	void Setup()
	{
		RZ_ItemHelper.Words(m_Settings.m_sWorkbenchWords, m_aBenchWords);
		m_aList.Clear();
		AddBuiltIn("Improvised bandage", m_Settings.m_sClothWords, 1, "", 0, "", 0, m_Settings.m_sBandagePrefab, 6, 1);
		AddBuiltIn("Molotov", m_Settings.m_sBottleWords, 1, m_Settings.m_sFuelWords, 1, m_Settings.m_sClothWords, 1, m_Settings.m_sMolotovPrefab, 10, 1);
		AddBuiltIn("Tin rattle (noisemaker)", m_Settings.m_sTinWords, 2, m_Settings.m_sWireWords, 1, "", 0, m_Settings.m_sNoisemakerPrefab, 6, 1);
		AddBuiltIn("Spear", m_Settings.m_sBladeWords, 1, m_Settings.m_sPlankWords, 1, m_Settings.m_sWireWords, 1, m_Settings.m_sSpearPrefab, 12, 2);
		AddBuiltIn("Tin-can tripwire", m_Settings.m_sTinWords, 2, m_Settings.m_sWireWords, 1, "", 0, m_Settings.m_sTripwirePrefab, 8, 1);
		AddBuiltIn("Spike trap", m_Settings.m_sPlankWords, 2, m_Settings.m_sNailWords, 1, "", 0, m_Settings.m_sSpikeTrapPrefab, 15, 4);
		AddBuiltIn("Purification tablets", m_Settings.m_sBleachWords, 1, "", 0, "", 0, m_Settings.m_sTabletsPrefab, 8, 0);
		AddBuiltIn("Purify water", m_Water.m_sDirtyWaterWords, 1, m_Water.m_sTabletWords, 1, "", 0, m_Water.m_sCleanWaterPrefab, 5, 0);

		if (m_Settings.m_aRecipes)
		{
			foreach (RZ_Recipe r : m_Settings.m_aRecipes)
			{
				if (r && !r.m_sOutput.IsEmpty() && !r.m_sIn1.IsEmpty() && m_aList.Count() < SLOTS)
					m_aList.Insert(r);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AddBuiltIn(string name, string in1, int n1, string in2, int n2, string in3, int n3, ResourceName output, float seconds, float heat)
	{
		if (output.IsEmpty() || m_aList.Count() >= SLOTS)
			return;

		RZ_Recipe r = new RZ_Recipe();
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
		m_aList.Insert(r);
	}

	// ================================================================ Workbench
	//------------------------------------------------------------------------------------------------
	//! Choice slots 0-3 are recipes 0-3, use slots 0-3 are recipes 4-7.
	void WorkbenchLabels(RZ_Spot s, out string read, out string useLabels, out string choiceLabels)
	{
		read = "Look at the workbench";
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
	//! What each recipe takes.
	void DescribeBench(int playerId)
	{
		if (m_aList.IsEmpty())
		{
			m_GM.NotifyPlayer(playerId, "Workbench", "A vice, a hammer with a taped handle, sawdust. Nothing set up to make anything. (No recipes are configured.)");
			return;
		}

		string text = "A vice, a hammer with a taped handle, sawdust. You could make:";
		foreach (RZ_Recipe r : m_aList)
		{
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
	protected bool HasInputs(IEntity user, RZ_Recipe r)
	{
		if (CountOf(user, r.m_sIn1) < Math.Max(1, r.m_iN1))
			return false;

		if (!r.m_sIn2.IsEmpty() && r.m_iN2 > 0 && CountOf(user, r.m_sIn2) < r.m_iN2)
			return false;

		if (!r.m_sIn3.IsEmpty() && r.m_iN3 > 0 && CountOf(user, r.m_sIn3) < r.m_iN3)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected int CountOf(IEntity user, string wordList)
	{
		array<string> words = {};
		RZ_ItemHelper.Words(wordList, words);
		return RZ_ItemHelper.Count(user, words);
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
	void CompleteRecipe(int playerId, RZ_Spot s, int idx, IEntity user)
	{
		if (idx < 0 || idx >= m_aList.Count())
			return;

		RZ_Recipe r = m_aList[idx];
		if (!HasInputs(user, r))
		{
			m_GM.NotifyPlayer(playerId, "Workbench", "Something you needed is gone from your pack.");
			return;
		}

		RZ_ItemHelper.Take(user, r.m_sIn1, Math.Max(1, r.m_iN1));
		if (!r.m_sIn2.IsEmpty() && r.m_iN2 > 0)
			RZ_ItemHelper.Take(user, r.m_sIn2, r.m_iN2);

		if (!r.m_sIn3.IsEmpty() && r.m_iN3 > 0)
			RZ_ItemHelper.Take(user, r.m_sIn3, r.m_iN3);

		vector at = user.GetOrigin();
		if (s)
			at = s.m_vPos;

		RZ_ItemHelper.Give(r.m_sOutput, user, at);
		if (r.m_fHeat > 0)
			m_GM.GetHeat().AddHeat(at, r.m_fHeat);

		m_GM.NotifyPlayer(playerId, "Workbench", string.Format("Done: %1. Not pretty, but it'll do.", r.m_sName));
	}

	// ================================================================ Water
	//------------------------------------------------------------------------------------------------
	//! 0 fill a bottle (untreated), 1 fill and purify it (a tablet).
	void HandleWell(int playerId, RZ_Spot s, int option, IEntity user)
	{
		string title = m_Hub.Title(s);
		if (m_Water.m_sDirtyWaterPrefab.IsEmpty())
		{
			m_GM.NotifyPlayer(playerId, title, "You could drink from it with your hands, if you were desperate. (No water items are configured.)");
			return;
		}

		if (!RZ_ItemHelper.Has(user, m_Water.m_sContainerWords))
		{
			m_GM.NotifyPlayer(playerId, title, "You need an empty bottle or canteen to fill.");
			return;
		}

		bool purify = option == 1;
		if (purify && (m_Water.m_sCleanWaterPrefab.IsEmpty() || !RZ_ItemHelper.Has(user, m_Water.m_sTabletWords)))
		{
			m_GM.NotifyPlayer(playerId, title, "You've no purification tablets. Fill it as it is and boil it later, or find tablets in a pharmacy.");
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

	//------------------------------------------------------------------------------------------------
	void HandleStove(int playerId, RZ_Spot s, IEntity user)
	{
		string title = m_Hub.Title(s);
		if (m_Water.m_sCleanWaterPrefab.IsEmpty() || !RZ_ItemHelper.Has(user, m_Water.m_sDirtyWaterWords))
		{
			m_GM.NotifyPlayer(playerId, title, "You've nothing to boil. Fill a bottle at a well first.");
			return;
		}

		m_GM.GetHeat().AddHeat(s.m_vPos, 2);
		m_Hub.StartTask(playerId, user, RZ_Interactions.TASK_BOIL, s.m_iId, 0, m_Water.m_fBoilSeconds, "the boiling", false);
		m_GM.NotifyPlayer(playerId, title, string.Format("You get a small fire going under the pot. About %1 seconds. The smoke goes up the chimney, and someone may see it.", RZ_Util.RoundToInt(m_Water.m_fBoilSeconds)));
	}

	//------------------------------------------------------------------------------------------------
	void CompleteBoil(int playerId, RZ_Spot s, IEntity user)
	{
		if (RZ_ItemHelper.Take(user, m_Water.m_sDirtyWaterWords, 1) <= 0)
			return;

		vector at = user.GetOrigin();
		if (s)
			at = s.m_vPos;

		RZ_ItemHelper.Give(m_Water.m_sCleanWaterPrefab, user, at);
		m_GM.GetHeat().AddHeat(at, 3);
		m_GM.NotifyPlayer(playerId, "Boiled water", "It rolls for a minute, and you let it cool. Flat and warm, and safe.");
	}
}
