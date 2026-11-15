//! Eating and drinking (docs/DESIGN.md, "Eating and drinking"). The hunger and thirst mod
//! (Metabolism) makes its food and water a gadget (B_FoodWaterGadgetComponent): taken into the hand,
//! its own key (vanilla's GadgetActivate: R, or the right trigger on a pad) eats it. ReforgerZ adds:
//! - left click eats or drinks what's in the hand, the same as R (the RZ_Eat input, mouse only, so a
//!   pad's right trigger never eats twice), with a hint naming the button (RZ_HudDisplay, HeldPanel);
//! - the inventory's Use eats or drinks a food or drink where it lies in your kit (no hotbar needed),
//!   on the server: Metabolism's own values, its sickness roll, the item used up;
//! - drinks the mod can't drink (a canteen: Interactions > Drink keywords) the same Use, with a
//!   water value from the settings.
//! Core: no Features switch (hunger is on from stage 0).

//------------------------------------------------------------------------------------------------
//! Metabolism's food item, read by ReforgerZ (its values are protected): what it gives and its
//! effect on a character, applied on the server by RZ_Eating.
modded class B_FoodWaterGadgetComponent
{
	//------------------------------------------------------------------------------------------------
	//! More water than food: a drink ("Drink", not "Eat").
	bool RZ_IsDrink()
	{
		return m_iThirstValue > m_iFoodValue;
	}

	//------------------------------------------------------------------------------------------------
	//! Metabolism's own effect on this character, as its ApplyEffect does it (the food and water
	//! coming in over a short time, the sickness roll), without deleting: the caller uses the item up.
	//! Server only. False if the character has no metabolism.
	bool RZ_ApplyTo(IEntity character)
	{
		if (!character)
			return false;

		B_MetabolismComponent metabolism = B_MetabolismComponent.Cast(character.FindComponent(B_MetabolismComponent));
		if (!metabolism)
			return false;

		metabolism.TriggerFoodDrinkIncrease(m_iFoodValue, m_iThirstValue);
		if (Math.RandomFloat(0.0, 1.0) < m_fSicknessChance)
			metabolism.ApplySickness();

		return true;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_Eating
{
	protected static const float CLICK_GAP = 1.0;		// a second between clicks: one bite per click
	protected static float s_fLastClick = -100;
	protected static ref array<string> s_aDrinkWords;
	protected static string s_sDrinkWordsFrom = "-";

	//------------------------------------------------------------------------------------------------
	//! The hunger and thirst mod's food or water item.
	static B_FoodWaterGadgetComponent FoodOf(IEntity item)
	{
		if (!item)
			return null;

		return B_FoodWaterGadgetComponent.Cast(item.FindComponent(B_FoodWaterGadgetComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! A drink the mod can't drink (Interactions > Drink keywords: a canteen), never an empty one.
	static bool IsOtherDrink(IEntity item)
	{
		if (!item || FoodOf(item))
			return false;

		RZ_InteractionSettings s = Settings();
		if (!s || s.m_sDrinkWords.IsEmpty())
			return false;

		if (s_sDrinkWordsFrom != s.m_sDrinkWords)
		{
			s_aDrinkWords = {};
			RZ_ItemHelper.Words(s.m_sDrinkWords, s_aDrinkWords);
			s_sDrinkWordsFrom = s.m_sDrinkWords;
		}

		if (!RZ_ItemHelper.Matches(item, s_aDrinkWords))
			return false;

		string name = RZ_ItemHelper.LowerName(item);
		return !name.Contains("empty");
	}

	//------------------------------------------------------------------------------------------------
	//! Something you can eat or drink here.
	static bool IsEdible(IEntity item)
	{
		return FoodOf(item) || IsOtherDrink(item);
	}

	//------------------------------------------------------------------------------------------------
	//! "Drink" or "Eat".
	static string Verb(IEntity item)
	{
		B_FoodWaterGadgetComponent food = FoodOf(item);
		if (food && !food.RZ_IsDrink())
			return "Eat";

		return "Drink";
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_InteractionSettings Settings()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetInteractions())
			return null;

		return gm.GetInteractions().GetSettings();
	}

	// ================================================================ Client
	//------------------------------------------------------------------------------------------------
	//! The food or drink in the local player's hand (the held gadget), or null.
	static IEntity HeldFood()
	{
		IEntity me = SCR_PlayerController.GetLocalControlledEntity();
		if (!me)
			return null;

		SCR_GadgetManagerComponent gadgets = SCR_GadgetManagerComponent.GetGadgetManager(me);
		if (!gadgets)
			return null;

		SCR_GadgetComponent held = gadgets.GetHeldGadgetComponent();
		if (!held || !B_FoodWaterGadgetComponent.Cast(held))
			return null;

		return held.GetOwner();
	}

	//------------------------------------------------------------------------------------------------
	//! Left click with food in the hand (RZ_HudDisplay's RZ_Eat listener): the same as the gadget's own
	//! key, Metabolism's path (its effect on the server, the item gone). Once a second at most.
	static void ClickEat()
	{
		IEntity item = HeldFood();
		if (!item)
			return;

		float now = GetGame().GetWorld().GetWorldTime() / 1000;
		if (now - s_fLastClick < CLICK_GAP)
			return;

		s_fLastClick = now;
		B_FoodWaterGadgetComponent food = FoodOf(item);
		if (food)
			food.ActivateAction(); // VERIFY: a held gadget's ActivateAction from script eats as the GadgetActivate key does (ToggleActive, then the server's ApplyEffect)
	}

	//------------------------------------------------------------------------------------------------
	//! The hint under the crosshair while food is in the hand: "Eat: left click or R" ("RT" on a
	//! pad); "" for none.
	static string HeldHint()
	{
		IEntity item = HeldFood();
		if (!item)
			return "";

		string verb = Verb(item);
		verb.ToUpper();
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			return verb + ": LEFT CLICK OR R";

		return verb + ": RT";
	}

	//------------------------------------------------------------------------------------------------
	//! The inventory's Use on a food or drink: the server eats it where it lies in your kit.
	static void AskEat(IEntity item)
	{
		if (!item)
			return;

		RplComponent rpl = RplComponent.Cast(item.FindComponent(RplComponent));
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!rpl || !ui)
			return;

		int id = rpl.Id();
		ui.SendCommand(RZ_PlayerUIComponent.CMD_EAT, id.ToString());
	}

	// ================================================================ Server
	//------------------------------------------------------------------------------------------------
	//! CMD_EAT: eat or drink this item, if it's in the player's own kit (or hand). The command's
	//! two-second gap guards a double use; the item is gone after.
	static void ServerEat(int playerId, string arg)
	{
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!me || !RZ_Util.IsCharacterAlive(me))
			return;

		RplId rid = arg.ToInt();
		RplComponent rpl = RplComponent.Cast(Replication.FindItem(rid));
		if (!rpl)
			return;

		if (EatBy(me, rpl.GetEntity()))
			RZ_Log.Info(string.Format("Eating: %1 ate or drank from the kit", RZ_Util.PlayerName(playerId)));
	}

	//------------------------------------------------------------------------------------------------
	//! This character eats or drinks this item from their kit (server): the mod's values and its
	//! sickness roll (a canteen: Interactions > Drink water), the item used up. False if it isn't
	//! edible, isn't theirs, or they have no metabolism.
	static bool EatBy(IEntity me, IEntity item)
	{
		if (!me || !item || !IsEdible(item) || !Carries(me, item))
			return false;

		B_FoodWaterGadgetComponent food = FoodOf(item);
		bool done = false;
		if (food)
		{
			done = food.RZ_ApplyTo(me);
		}
		else
		{
			B_MetabolismComponent metabolism = B_MetabolismComponent.Cast(me.FindComponent(B_MetabolismComponent));
			RZ_InteractionSettings s = Settings();
			if (metabolism && s)
			{
				metabolism.TriggerFoodDrinkIncrease(0, s.m_fDrinkWater);
				done = true;
			}
		}

		if (!done)
			return false;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(me.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv || !inv.TryDeleteItem(item))
			SCR_EntityHelper.DeleteEntityAndChildren(item);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The item is somewhere on this character (in a storage, or in the hand).
	static bool Carries(IEntity me, IEntity item)
	{
		IEntity at = item.GetParent();
		int guard = 0;
		while (at && guard < 12)
		{
			if (at == me)
				return true;

			at = at.GetParent(); // VERIFY: an item in a character's storages (pack, vest, pockets) has the character up its parent chain
			guard++;
		}

		return false;
	}
}

//------------------------------------------------------------------------------------------------
//! The inventory's Use on food and drink (vanilla's own button: its key and its pad button).
modded class SCR_CharacterInventoryStorageComponent
{
	//------------------------------------------------------------------------------------------------
	override bool CanUseItem_Inventory(notnull IEntity item, ESlotFunction slotFunction = ESlotFunction.TYPE_GENERIC)
	{
		if (RZ_Eating.IsEdible(item))
			return true;

		return super.CanUseItem_Inventory(item, slotFunction);
	}
}

//------------------------------------------------------------------------------------------------
modded class SCR_InventoryMenuUI
{
	//------------------------------------------------------------------------------------------------
	override protected void Action_UseItem()
	{
		if (m_pFocusedSlotUI)
		{
			InventoryItemComponent itemComponent = m_pFocusedSlotUI.GetInventoryItemComponent();
			if (itemComponent && RZ_Eating.IsEdible(itemComponent.GetOwner()))
			{
				RZ_Eating.AskEat(itemComponent.GetOwner()); // VERIFY: the inventory's Use (Inventory_Use) on a food item reaches Action_UseItem with it focused
				return;
			}
		}

		super.Action_UseItem();
	}
}
