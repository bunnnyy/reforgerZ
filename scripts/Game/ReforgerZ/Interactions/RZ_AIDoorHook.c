//! OPTIONAL: the AI's own door opening, hooked for door locks (docs/DESIGN.md, "Locks and
//! keys"; docs/WORKBENCH_SETUP.md, section 6). Layer 1 of 3; the doorway blockers and
//! the banging (RZ_DoorBlockers.c) work without it.
//!
//! The class and method below are a best guess at the behaviour-tree task that makes an AI
//! character open a door on its path. It could not be confirmed outside Workbench.
//! - If this file doesn't compile: DELETE IT. Nothing else refers to it.
//! - If it compiles but AI still open locked doors, the guess is wrong (or door opening is
//!   native). Find the real door task in the Script Editor (search "Door" under AI) and move
//!   the one check below into it, or delete this file.
//! - Search > AI door hook turns the check off without deleting the file.
//!
//! The check itself is RZ_DoorLocks.AiMayOpenNear(character): false only at a locked place's
//! outside door, from outside, while it's locked.

// VERIFY: SCR_AIOpenDoor is the AITaskScripted behaviour-tree node AI use to open a door on their path, and EOnTaskSimulate(AIAgent, float) is where it acts (low confidence: delete this file if it doesn't compile)
modded class SCR_AIOpenDoor
{
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (owner && Replication.IsServer())
		{
			IEntity character = owner.GetControlledEntity();
			if (character && !RZ_DoorLocks.AiMayOpenNear(character))
				return ENodeResult.FAIL;
		}

		return super.EOnTaskSimulate(owner, dt);
	}
}
