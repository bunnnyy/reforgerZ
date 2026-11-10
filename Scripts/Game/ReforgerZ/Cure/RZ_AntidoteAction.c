//! "Inject antidote". Put this in the antidote prefab's ActionsManagerComponent.
//!
//! For now it's a world action, so players drop the vial and use it from the
//! ground (or use one someone else dropped, which is a nice moment). Wiring it
//! into the inventory "use" flow through SCR_ConsumableItemComponent is a later step.
class RZ_InjectAntidoteAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Inject antidote";
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
		if (!gm)
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		if (playerId <= 0)
			return;

		// Don't waste a dose on someone who isn't sick
		if (!gm.GetProfile(playerId).IsInfected())
		{
			gm.NotifyPlayer(playerId, "Antidote", "You don't feel sick. Better save it.");
			return;
		}

		gm.CurePlayer(playerId);
		SCR_EntityHelper.DeleteEntityAndChildren(pOwnerEntity);
	}
}
