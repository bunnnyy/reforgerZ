//! Player actions for puzzles. All of them only report to the server, which
//! routes to the owning mission (RZ_PuzzleMission). Nothing is decided on the client.

//------------------------------------------------------------------------------------------------
//! "Read": on any entity with RZ_ClueComponent (note, plaque, dog tags, wreck, civilian).
class RZ_ReadClueAction : ScriptedUserAction
{
	[Attribute("Read", desc: "Label when the mission didn't set one (e.g. Read the note, Look at the plaque, Listen)")]
	protected string m_sLabel;

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = m_sLabel;
		RZ_ClueComponent comp = RZ_ClueComponent.Cast(GetOwner().FindComponent(RZ_ClueComponent));
		if (comp && !comp.GetReadLabel().IsEmpty())
			outName = comp.GetReadLabel();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Hidden when the read label is RZ_ClueComponent.NO_READ (a burn spot has only its choice).
	override bool CanBeShownScript(IEntity user)
	{
		RZ_ClueComponent comp = RZ_ClueComponent.Cast(GetOwner().FindComponent(RZ_ClueComponent));
		return !comp || comp.GetReadLabel() != RZ_ClueComponent.NO_READ;
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
class RZ_KeypadAction : ScriptedUserAction
{
	[Attribute("0", UIWidgets.Slider, "Key: 0-9 digit, 10 = Clear, 11 = Enter", "0 11 1")]
	protected int m_iKey;

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
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
class RZ_PuzzleChoiceAction : ScriptedUserAction
{
	[Attribute("0", UIWidgets.Slider, "Option index", "0 7 1")]
	protected int m_iOption;

	[Attribute("Choose", desc: "Label when the mission didn't set one")]
	protected string m_sLabel;

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
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
	override bool CanBeShownScript(IEntity user)
	{
		RZ_PuzzleObjectComponent comp = RZ_PuzzleObjectComponent.Cast(GetOwner().FindComponent(RZ_PuzzleObjectComponent));
		if (comp && comp.IsSlotHidden(true, m_iOption))
			return false;

		// A burn spot (world interactions): only for someone whose own hands hold fuel
		if (comp && comp.GetSlotLabel(true, m_iOption) == RZ_Traps.BURN_LABEL)
			return RZ_Traps.FireFuel(user) != null;

		// On a fixable car, the jobs are in the repair screen; at a workbench, the recipes in the crafting screen
		if (RZ_CarMenu.HidesSlotsOn(GetOwner()) || RZ_CraftMenu.HidesSlotsOn(GetOwner()))
			return false;

		// On a person, the replies are in the talk screen
		return !RZ_TalkMenu.HidesSlotsOn(GetOwner());
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
		int playerId = RZ_PuzzleActionHelper.GetPlayerId(pUserEntity);
		if (!gm || playerId <= 0)
			return;

		RZ_PuzzleActionHelper.DoChoice(gm, pOwnerEntity, playerId, m_iOption, pUserEntity);
	}
}

//------------------------------------------------------------------------------------------------
//! Use: pull a lever, flip a switch, ring a bell, tune a radio, switch on a beacon,
//! seal a well. The value tells the mission which (e.g. switch 0 or 1 on a fuse box).
class RZ_PuzzleUseAction : ScriptedUserAction
{
	[Attribute("0", UIWidgets.Slider, "Value passed to the mission (which switch, lever...)", "0 7 1")]
	protected int m_iValue;

	[Attribute("Use", desc: "Label when the mission didn't set one")]
	protected string m_sLabel;

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
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
	override bool CanBeShownScript(IEntity user)
	{
		RZ_PuzzleObjectComponent comp = RZ_PuzzleObjectComponent.Cast(GetOwner().FindComponent(RZ_PuzzleObjectComponent));
		if (comp && comp.IsSlotHidden(false, m_iValue))
			return false;

		// At a workbench, the recipes are in the crafting screen
		if (RZ_CraftMenu.HidesSlotsOn(GetOwner()))
			return false;

		return !RZ_TalkMenu.HidesSlotsOn(GetOwner());
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
		int playerId = RZ_PuzzleActionHelper.GetPlayerId(pUserEntity);
		if (!gm || playerId <= 0)
			return;

		RZ_PuzzleActionHelper.DoUse(gm, pOwnerEntity, playerId, m_iValue, pUserEntity);
	}
}
