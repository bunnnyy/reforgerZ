//! OPTIONAL: the AI's own door opening, hooked for door locks (docs/DESIGN.md, "Locks and
//! keys"; docs/WORKBENCH_SETUP.md, section 6). Layer 1 of 3; the doorway blockers and
//! the banging (RZ_DoorBlockers.c) work without it.
//!
//! SCR_AIOpenDoor is the vanilla behaviour-tree task that opens a door through its door
//! actions (AI/ScriptedNodes/Soldier). Nothing else refers to this file.
//! - If AI still open locked doors, they open doors on their path some other way (natively,
//!   or through another task). Find it in the Script Editor (search "Door" under AI) and move
//!   the one check below into it, or delete this file.
//! - Search > AI door hook turns the check off without deleting the file.
//!
//! The check itself is RZ_DoorLocks.AiMayOpenNear(character): false only at a locked place's
//! outside door, from outside, while it's locked.

// VERIFY: AI (soldiers and the infected) open doors on their path through SCR_AIOpenDoor
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
