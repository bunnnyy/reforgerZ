//! Player actions for puzzles. All of them only report to the server, which
//! routes to the owning mission (RZ_PuzzleMission). Nothing is decided on the client.
//! On a prop that wants it (RZ_PuzzleObjectComponent.WantsMenu) they're one object menu: the read
//! opens it and its text is the description, the use, choice and keypad slots are the rows
//! (Interactions/RZ_ObjectScreen.c).

//------------------------------------------------------------------------------------------------
//! "Read": on any entity with RZ_ClueComponent (note, plaque, dog tags, wreck, civilian).
class RZ_ReadClueAction : RZ_GroupedAction
{
	[Attribute("Read", desc: "Label when the mission didn't set one (e.g. Read the note, Look at the plaque, Listen)")]
	protected string m_sLabel;

	//------------------------------------------------------------------------------------------------
	override bool Label(out string outName)
	{
		outName = m_sLabel;
		RZ_ClueComponent comp = RZ_ClueComponent.Cast(GetOwner().FindComponent(RZ_ClueComponent));
		if (comp && !comp.GetReadLabel().IsEmpty())
			outName = comp.GetReadLabel();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Hidden when the read label is RZ_ClueComponent.NO_READ (a burn spot has only its choice), and
	//! on an object hidden until tagged that isn't (a vanilla house radio no spot uses now).
	override bool Visible(IEntity user)
	{
		RZ_PuzzleObjectComponent obj = RZ_PuzzleObjectComponent.Cast(GetOwner().FindComponent(RZ_PuzzleObjectComponent));
		if (obj && obj.IsDormant())
			return false;

		RZ_ClueComponent comp = RZ_ClueComponent.Cast(GetOwner().FindComponent(RZ_ClueComponent));
		return !comp || comp.GetReadLabel() != RZ_ClueComponent.NO_READ;
	}

	override bool IsSlot() { return true; }
	override bool Describes() { return true; }

	//------------------------------------------------------------------------------------------------
	//! A world interaction spot's own name ("Stove"); else the read without its verb ("The plaque").
	override string MenuTitle(IEntity owner)
	{
		RZ_Interactions hub = RZ_Interactions.Get();
		RZ_ClueComponent comp = RZ_ClueComponent.Cast(owner.FindComponent(RZ_ClueComponent));
		if (hub && comp && comp.GetMissionId() == RZ_Interactions.TAG)
			return hub.TitleOf(comp.GetClueIndex());

		string label;
		Label(label);
		return RZ_ActionMenus.TitleFromRead(label);
	}

	//------------------------------------------------------------------------------------------------
	override void Run(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int playerId = RZ_PuzzleActionHelper.GetPlayerId(pUserEntity);
		if (!gm || playerId <= 0)
			return;

		// A living person opens the talk screen, where it's on (docs/DESIGN.md, Talking to people)
		if (gm.GetDialogue() && gm.GetDialogue().Open(playerId, pOwnerEntity, pUserEntity))
			return;

		RZ_PuzzleActionHelper.DoRead(gm, pOwnerEntity, playerId);
	}
}

//------------------------------------------------------------------------------------------------
//! One key of a keypad: digits 0-9, 10 = Clear, 11 = Enter. Add twelve of these
//! to the keypad prefab. The Enter label shows what has been typed so far.
class RZ_KeypadAction : RZ_GroupedAction
{
	[Attribute("0", UIWidgets.Slider, "Key: 0-9 digit, 10 = Clear, 11 = Enter", "0 11 1")]
	protected int m_iKey;

	override bool IsSlot() { return true; }

	//------------------------------------------------------------------------------------------------
	override bool Label(out string outName)
	{
		if (m_iKey == 10)
		{
			outName = "Clear";
			return true;
		}

		if (m_iKey == 11)
		{
			string display = "";
			RZ_PuzzleObjectComponent comp = RZ_PuzzleObjectComponent.Cast(GetOwner().FindComponent(RZ_PuzzleObjectComponent));
			if (comp)
				display = comp.GetDisplay();

			if (display.IsEmpty())
				outName = "Enter";
			else
				outName = "Enter [" + display + "]";

			return true;
		}

		outName = "Press " + m_iKey.ToString();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Run(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int playerId = RZ_PuzzleActionHelper.GetPlayerId(pUserEntity);
		if (!gm || playerId <= 0)
			return;

		int objectIndex;
		RZ_PuzzleMission m = RZ_PuzzleActionHelper.FindForObject(gm, pOwnerEntity, objectIndex);
		if (!m)
		{
			gm.NotifyPlayer(playerId, "Keypad", "The keypad is dead.");
			return;
		}

		m.HandleKey(gm, playerId, objectIndex, m_iKey, pOwnerEntity);
	}
}

//------------------------------------------------------------------------------------------------
//! Choose / accuse / decide. Option index 0-3. The mission can relabel each option
//! ("Accuse Mirek", "Burn it"); options it leaves unlabelled are hidden.
class RZ_PuzzleChoiceAction : RZ_GroupedAction
{
	[Attribute("0", UIWidgets.Slider, "Option index", "0 7 1")]
	protected int m_iOption;

	[Attribute("Choose", desc: "Label when the mission didn't set one")]
	protected string m_sLabel;

	override bool IsSlot() { return true; }

	//------------------------------------------------------------------------------------------------
	override bool Label(out string outName)
	{
		outName = m_sLabel;
		RZ_PuzzleObjectComponent comp = RZ_PuzzleObjectComponent.Cast(GetOwner().FindComponent(RZ_PuzzleObjectComponent));
		if (comp)
		{
			string label = comp.GetSlotLabel(true, m_iOption);
			if (!label.IsEmpty())
				outName = label;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Visible(IEntity user)
	{
		RZ_PuzzleObjectComponent comp = RZ_PuzzleObjectComponent.Cast(GetOwner().FindComponent(RZ_PuzzleObjectComponent));
		if (comp && comp.IsSlotHidden(true, m_iOption))
			return false;

		// A burn spot (world interactions): only for someone whose own hands hold fuel
		if (comp && comp.GetSlotLabel(true, m_iOption) == RZ_Traps.BURN_LABEL)
			return RZ_Traps.FireFuel(user) != null;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! On a vanilla house radio the options need it switched on: vanilla's Turn on is in the same
	//! prompt list (RZ_SecureRadio.IsSwitchedOff). Dim with why in the object menu.
	override bool CanBePerformedScript(IEntity user)
	{
		if (RZ_SecureRadio.IsSwitchedOff(GetOwner()))
		{
			SetCannotPerformReason(RZ_SecureRadio.SWITCHED_OFF_REASON);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool HiddenByScreens()
	{
		// On a fixable car, the jobs are in the repair screen; at a workbench, the recipes in the crafting screen
		if (RZ_CarMenu.HidesSlotsOn(GetOwner()) || RZ_CraftMenu.HidesSlotsOn(GetOwner()))
			return true;

		// On a person, the replies are in the talk screen
		return RZ_TalkMenu.HidesSlotsOn(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	override void Run(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int playerId = RZ_PuzzleActionHelper.GetPlayerId(pUserEntity);
		if (!gm || playerId <= 0)
			return;

		RZ_PuzzleActionHelper.DoChoice(gm, pOwnerEntity, playerId, m_iOption, pUserEntity);
	}
}

//------------------------------------------------------------------------------------------------
//! Use: pull a lever, flip a switch, ring a bell, tune a radio, switch on a beacon,
//! seal a well. The value tells the mission which (e.g. switch 0 or 1 on a fuse box).
class RZ_PuzzleUseAction : RZ_GroupedAction
{
	[Attribute("0", UIWidgets.Slider, "Value passed to the mission (which switch, lever...)", "0 7 1")]
	protected int m_iValue;

	[Attribute("Use", desc: "Label when the mission didn't set one")]
	protected string m_sLabel;

	override bool IsSlot() { return true; }

	//------------------------------------------------------------------------------------------------
	override bool Label(out string outName)
	{
		outName = m_sLabel;
		RZ_PuzzleObjectComponent comp = RZ_PuzzleObjectComponent.Cast(GetOwner().FindComponent(RZ_PuzzleObjectComponent));
		if (comp)
		{
			string label = comp.GetSlotLabel(false, m_iValue);
			if (!label.IsEmpty())
				outName = label;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Visible(IEntity user)
	{
		RZ_PuzzleObjectComponent comp = RZ_PuzzleObjectComponent.Cast(GetOwner().FindComponent(RZ_PuzzleObjectComponent));
		return !comp || !comp.IsSlotHidden(false, m_iValue);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool HiddenByScreens()
	{
		// At a workbench, the recipes are in the crafting screen
		if (RZ_CraftMenu.HidesSlotsOn(GetOwner()))
			return true;

		return RZ_TalkMenu.HidesSlotsOn(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	override void Run(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int playerId = RZ_PuzzleActionHelper.GetPlayerId(pUserEntity);
		if (!gm || playerId <= 0)
			return;

		RZ_PuzzleActionHelper.DoUse(gm, pOwnerEntity, playerId, m_iValue, pUserEntity);
	}
}
