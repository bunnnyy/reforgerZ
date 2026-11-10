//! The vanilla door action, hooked for door locks (RZ_DoorLocks.c; docs/DESIGN.md, "Locks
//! and keys"). A locked building's outside door won't open from outside: the client greys
//! the action out as "Locked" (from the replicated list of locked buildings near players),
//! and the server refuses it again in case the client check is bypassed or out of date.
//! Shutting a door, interior doors and opening from inside are never refused.

// VERIFY: the vanilla door prefabs use SCR_DoorUserAction (else hook the action named in the door prefab's ActionsManagerComponent, or turn Interactions > Search > Door locks off)
modded class SCR_DoorUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!super.CanBePerformedScript(user))
			return false;

		if (RZ_DoorGeometry.LooksLocked(GetOwner(), user))
		{
			SetCannotPerformReason("Locked");
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
