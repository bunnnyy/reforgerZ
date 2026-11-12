//! The notebook item (docs/UI.md, "The notebook item"): a small notebook every new life is given
//! (UI > Notebook prefab, RZ_UIFeed.GiveNotebook). It's a gadget like vanilla's map: taken into the
//! left hand (N, a quick slot, the inventory's Equip, or its Equip action on the ground) it opens
//! the notebook, as vanilla's map opens when the map comes into the hand; when the notebook closes,
//! it's put away again. On the ground, and held up and looked at, it also has **Read the notebook**.

[ComponentEditorProps(category: "ReforgerZ", description: "The notebook item: a gadget held in the left hand like the MX991 flashlight. Taken into the hand by its owner, it opens the notebook; put away again when the notebook closes. Put on Prefabs/ReforgerZ/UI/RZ_Notebook.et with Gadget anim variable MX991 (docs/WORKBENCH_SETUP.md).")]
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
	//! The hold the character's animation plays for this gadget. With none set (NONE) the character
	//! has no hold to play, so the take never finishes and the item never reaches the hand: that was
	//! why the notebook didn't open in play. The prefab sets MX991; this is the fallback.
	//! VERIFY: with MX991 the character takes the notebook into the left hand as it does the flashlight.
	override EGadgetAnimVariable GetAnimVariable()
	{
		if (m_eAnimVariable == EGadgetAnimVariable.NONE)
			return EGadgetAnimVariable.MX991;

		return m_eAnimVariable;
	}

	//------------------------------------------------------------------------------------------------
	//! As vanilla's map (SCR_MapGadgetComponent.ModeSwitch): in the hand of this machine's
	//! character, it opens.
	override protected void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		super.ModeSwitch(mode, charOwner);
		if (mode != EGadgetMode.IN_HAND)
			return;

		IEntity controlled = SCR_PlayerController.GetLocalControlledEntity();
		if (!controlled || controlled != m_CharacterOwner)
			return;

		RZ_NotebookItem.OnInHand(controlled);
	}
}

//------------------------------------------------------------------------------------------------
//! "Read the notebook": the fallback on the item itself. Shown on a notebook lying on the ground,
//! and on your own held up and looked at (vanilla's inspection). Opens the notebook on this machine.
class RZ_ReadNotebookAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Read the notebook";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		IEntity owner = GetOwner();
		if (!owner || !user)
			return false;

		// VERIFY: a carried item has a parent (as the radio's View radio); one on the ground has none
		if (!owner.GetParent())
			return true;

		if (owner.GetRootParent() != user)
			return false;

		CharacterControllerComponent control = CharacterControllerComponent.Cast(user.FindComponent(CharacterControllerComponent));
		return control && control.GetInspect();
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! VERIFY: a local-only action's PerformAction runs on the machine of the player who used it
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (System.IsConsoleApp())
			return;

		IEntity controlled = SCR_PlayerController.GetLocalControlledEntity();
		if (!controlled || controlled != pUserEntity)
			return;

		if (RZ_NotebookItem.IsHeld(controlled))
			RZ_Notebook.OpenFromItem(controlled);
		else
			RZ_Notebook.Open(-1);
	}
}

//------------------------------------------------------------------------------------------------
//! The notebook item on a character: finding it, taking it into the hand, opening the notebook
//! when it's there (client), and whether a character carries one (server).
class RZ_NotebookItem
{
	static const int OPEN_DELAY_MS = 300;	// as vanilla's map: the hold shows before the menu
	static const int TAKE_WAIT_MS = 1500;	// N: not in the hand by then, the notebook opens anyway

	protected static bool s_bWasHeld;
	protected static bool s_bPending;		// an open is queued
	protected static bool s_bWaitingTake;	// N asked for the item to come into the hand
	protected static IEntity s_SlottedItem;	// the item last put on a quick slot (once per item)

	//------------------------------------------------------------------------------------------------
	//! The notebook item on this character, or null.
	static IEntity Find(IEntity character)
	{
		SCR_GadgetManagerComponent gadgets = SCR_GadgetManagerComponent.GetGadgetManager(character);
		if (gadgets)
		{
			array<SCR_GadgetComponent> specialist = gadgets.GetGadgetsByType(EGadgetType.SPECIALIST_ITEM);
			if (specialist)
			{
				foreach (SCR_GadgetComponent comp : specialist)
				{
					if (RZ_NotebookGadgetComponent.Cast(comp))
						return comp.GetOwner();
				}
			}
		}

		if (!character)
			return null;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return null;

		array<IEntity> items = {};
		inv.GetItems(items);
		foreach (IEntity item : items)
		{
			if (item && item.FindComponent(RZ_NotebookGadgetComponent))
				return item;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Is the notebook item in this character's left hand? Asks the gadget manager, the character's
	//! left-hand gadget and the item's own mode, so one of them missing doesn't hide it.
	static bool IsHeld(IEntity character)
	{
		if (!character)
			return false;

		SCR_GadgetManagerComponent gadgets = SCR_GadgetManagerComponent.GetGadgetManager(character);
		if (gadgets)
		{
			IEntity gadget = gadgets.GetHeldGadget();
			if (gadget && gadget.FindComponent(RZ_NotebookGadgetComponent))
				return true;
		}

		ChimeraCharacter ch = ChimeraCharacter.Cast(character);
		if (ch)
		{
			CharacterControllerComponent control = ch.GetCharacterController();
			if (control && control.GetInputContext())
			{
				IEntity left = control.GetInputContext().GetLeftHandGadgetEntity();
				if (left && left.FindComponent(RZ_NotebookGadgetComponent))
					return true;
			}
		}

		IEntity item = Find(character);
		if (!item)
			return false;

		RZ_NotebookGadgetComponent comp = RZ_NotebookGadgetComponent.Cast(item.FindComponent(RZ_NotebookGadgetComponent));
		return comp && comp.GetMode() == EGadgetMode.IN_HAND;
	}

	//------------------------------------------------------------------------------------------------
	//! Client: the notebook item has come into the hand of this machine's character. Opens the
	//! notebook a moment later (once), from the gadget's mode switch or the HUD's watch.
	static void OnInHand(IEntity holder)
	{
		s_bWaitingTake = false;
		if (s_bPending || RZ_Notebook.IsOpen())
			return;

		s_bPending = true;
		GetGame().GetCallqueue().CallLater(RZ_NotebookItem.OpenHeld, OPEN_DELAY_MS, false, holder);
	}

	//------------------------------------------------------------------------------------------------
	protected static void OpenHeld(IEntity holder)
	{
		s_bPending = false;
		if (!holder || holder != SCR_PlayerController.GetLocalControlledEntity() || RZ_Notebook.IsOpen())
			return;

		// Equip in the inventory: it closes first, as vanilla's map closes the pause menu
		MenuManager menus = GetGame().GetMenuManager();
		if (menus && menus.FindMenuByPreset(ChimeraMenuPreset.Inventory20Menu))
		{
			menus.CloseMenuByPreset(ChimeraMenuPreset.Inventory20Menu);
			s_bPending = true;
			GetGame().GetCallqueue().CallLater(RZ_NotebookItem.OpenHeld, 0, false, holder);
			return;
		}

		RZ_Notebook.OpenFromItem(holder);
	}

	//------------------------------------------------------------------------------------------------
	//! N (RZ_OpenNotebook): as vanilla's M with the map, the notebook item comes into the hand and
	//! opens there. Without the item, in a seat, or if it can't be held, the notebook just opens.
	static void OpenByKey()
	{
		if (RZ_Notebook.IsOpen() || s_bPending || s_bWaitingTake)
			return;

		IEntity character = SCR_PlayerController.GetLocalControlledEntity();
		if (!TakeInHand(character))
			RZ_Notebook.Open(-1);
	}

	//------------------------------------------------------------------------------------------------
	//! Client: ask for the notebook item to come into the hand. False if it can't (no item, in a
	//! seat, the character can't take it); the caller opens the notebook without it.
	protected static bool TakeInHand(IEntity character)
	{
		ChimeraCharacter ch = ChimeraCharacter.Cast(character);
		if (!ch || ch.IsInVehicle())
			return false;

		if (IsHeld(ch))
		{
			RZ_Notebook.OpenFromItem(ch);
			return true;
		}

		SCR_GadgetManagerComponent gadgets = SCR_GadgetManagerComponent.GetGadgetManager(ch);
		IEntity item = Find(ch);
		CharacterControllerComponent control = ch.GetCharacterController();
		// VERIFY: CanEquipGadget is true for the notebook item on foot (the inventory's Equip asks the same)
		if (!gadgets || !item || !control || !control.CanEquipGadget(item))
			return false;

		// As vanilla's quick slot: another gadget in the hand goes first
		IEntity held = gadgets.GetHeldGadget();
		if (held && held != item)
			gadgets.RemoveHeldGadget();

		gadgets.SetGadgetMode(item, EGadgetMode.IN_HAND);
		s_bWaitingTake = true;
		GetGame().GetCallqueue().Remove(RZ_NotebookItem.TakeTimedOut);
		GetGame().GetCallqueue().CallLater(RZ_NotebookItem.TakeTimedOut, TAKE_WAIT_MS, false, character);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! N's take didn't reach the hand in time: open the notebook anyway (and put the item away on
	//! close, if it arrives late).
	protected static void TakeTimedOut(IEntity character)
	{
		if (!s_bWaitingTake)
			return;

		s_bWaitingTake = false;
		if (s_bPending || RZ_Notebook.IsOpen() || !character || character != SCR_PlayerController.GetLocalControlledEntity())
			return;

		RZ_Log.Warn("Notebook item: not in the hand 1.5 s after N; the notebook opened without it (check the prefab's Gadget anim variable, docs/WORKBENCH_SETUP.md)");
		RZ_Notebook.OpenFromItem(character);
	}

	//------------------------------------------------------------------------------------------------
	//! Client, on the HUD's 0.2 s refresh: the notebook item just taken into the hand opens the
	//! notebook, in case the gadget's own mode switch didn't run here; and a notebook new to this
	//! character goes on a free quick slot.
	static void CheckHeld()
	{
		IEntity character = SCR_PlayerController.GetLocalControlledEntity();
		if (!character)
		{
			s_bWasHeld = false;
			return;
		}

		bool held = IsHeld(character);
		if (held && !s_bWasHeld && !RZ_Notebook.IsOpen())
			OnInHand(character);

		s_bWasHeld = held;
		SlotOnce(character);
	}

	//------------------------------------------------------------------------------------------------
	//! A notebook new to this character goes on the last free quick slot (not the weapon slots), so
	//! a pad reaches it from the quick slot wheel, as vanilla files a bandage. Once per item: if the
	//! player takes it off the slot, it stays off.
	protected static void SlotOnce(IEntity character)
	{
		IEntity item = Find(character);
		if (!item || item == s_SlottedItem)
			return;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return;

		SCR_CharacterInventoryStorageComponent storage = inv.GetCharacterStorage();
		if (!storage)
			return;

		s_SlottedItem = item;
		array<IEntity> slotted = storage.GetQuickSlotEntitiesOnly();
		if (slotted.Contains(item))
			return;

		// VERIFY: quick slots 0-3 are the weapon slots (SCR_InventoryMenuUI.WEAPON_SLOTS_COUNT)
		for (int i = slotted.Count() - 1; i >= 4; i--)
		{
			if (slotted[i] || storage.GetQuickSlotItems()[i])
				continue;

			int slot = storage.StoreItemToQuickSlot(item, i);
			if (slot >= 0 && SCR_WeaponSwitchingBaseUI.s_bOpened)
				SCR_WeaponSwitchingBaseUI.RefreshQuickSlots(slot);

			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Server: does this character carry a notebook item?
	static bool Carries(IEntity character)
	{
		return Find(character) != null;
	}
}
