//! The vanilla door action, hooked for door locks (RZ_DoorLocks.c; docs/DESIGN.md, "Locks
//! and keys"). A locked building's outside door won't open from outside: the client greys
//! the action out as "Locked" (from the replicated list of locked buildings near players),
//! and the server refuses it again in case the client check is bypassed or out of date.
//! Shutting a door, interior doors and opening from inside are never refused.
//!
//! If this file doesn't compile because the class name is wrong, rename the modded class to
//! the action on the vanilla door prefabs; if there's no scripted door action at all, delete
//! this file and turn Interactions > Search > Door locks off (the lock goes back on the loot).

// VERIFY: SCR_DoorUserAction is the vanilla "Open / Close" action on door prefabs and its owner is the door entity (else DoorUserAction or the name in the door prefab's ActionsManagerComponent)
modded class SCR_DoorUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!super.CanBePerformedScript(user))
			return false;

		if (RZ_DoorGeometry.LooksLocked(GetOwner(), user))
		{
			SetCannotPerformReason("Locked"); // VERIFY: SetCannotPerformReason(string) on the user action base class
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		// VERIFY: the door action's PerformAction also runs on the server, which owns the door state
		if (Replication.IsServer() && !RZ_DoorLocks.ServerAllowsOpen(pOwnerEntity, pUserEntity))
		{
			RZ_DoorGeometry.Close(pOwnerEntity);
			return;
		}

		super.PerformAction(pOwnerEntity, pUserEntity);
	}
}
