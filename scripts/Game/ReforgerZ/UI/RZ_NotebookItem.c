//! The notebook item (docs/UI.md, "The notebook"): a small notebook every new life is given
//! (UI > Notebook prefab, RZ_UIFeed.GiveNotebook). Take it into your hand, from the inventory or
//! a quick slot, and the notebook opens; when the notebook closes, it's put away again. It's the
//! diegetic way in, and with the pause menu's button the pad's way in.

[ComponentEditorProps(category: "ReforgerZ", description: "The notebook item: taken into the hand by its owner, it opens the notebook; put away again when the notebook closes. Put on Prefabs/ReforgerZ/UI/RZ_Notebook.et (docs/WORKBENCH_SETUP.md).")]
class RZ_NotebookGadgetComponentClass : SCR_GadgetComponentClass
{
}

//------------------------------------------------------------------------------------------------
class RZ_NotebookGadgetComponent : SCR_GadgetComponent
{
	//------------------------------------------------------------------------------------------------
	override EGadgetType GetType()
	{
		return EGadgetType.SPECIALIST_ITEM;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeRaised()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! VERIFY: the mode change to IN_HAND reaches the owning client (vanilla's gadgets switch their
	//! visuals here on every machine).
	override protected void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		super.ModeSwitch(mode, charOwner);
		if (mode != EGadgetMode.IN_HAND || !charOwner || System.IsConsoleApp())
			return;

		PlayerController pc = GetGame().GetPlayerController();
		if (!pc || pc.GetControlledEntity() != charOwner)
			return;

		RZ_Notebook.OpenFromItem(charOwner);
	}
}

//------------------------------------------------------------------------------------------------
//! Finds the notebook item on a character (server).
class RZ_NotebookItem
{
	protected static bool s_bWasHeld;

	//------------------------------------------------------------------------------------------------
	//! Client, on the HUD's 0.2 s refresh: the notebook item just taken into the hand opens the
	//! notebook. The gadget's own IN_HAND mode switch didn't reach the client in play (2026-11-12),
	//! so this watches the held gadget instead; it opens once per time the item is taken in hand.
	static void CheckHeld()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;

		IEntity character = pc.GetControlledEntity();
		bool held = false;
		SCR_GadgetManagerComponent gadgets = SCR_GadgetManagerComponent.GetGadgetManager(character);
		if (gadgets)
		{
			IEntity gadget = gadgets.GetHeldGadget();
			if (gadget && gadget.FindComponent(RZ_NotebookGadgetComponent))
				held = true;
		}

		if (held && !s_bWasHeld && !RZ_Notebook.IsOpen())
			RZ_Notebook.OpenFromItem(character);

		s_bWasHeld = held;
	}

	//------------------------------------------------------------------------------------------------
	static bool Carries(IEntity character)
	{
		if (!character)
			return false;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return false;

		array<IEntity> items = {};
		inv.GetItems(items);
		foreach (IEntity item : items)
		{
			if (item && item.FindComponent(RZ_NotebookGadgetComponent))
				return true;
		}

		return false;
	}
}
