//! Antidote as an inventory consumable: lets players use it from the inventory
//! like morphine, instead of dropping it and using the world action.
//!
//! Setup: on the antidote prefab's SCR_ConsumableItemComponent, set the
//! consumable effect to RZ_ConsumableAntidote. Keep RZ_InjectAntidoteAction as
//! well; it still works from the ground.
//!
//! Only on yourself, and only while your infection HUD shows a bite (RZ_ConsumableGate), so a
//! healthy player doesn't use the dose up.
//!
//! VERIFY: ApplyEffect also runs on the server, where the cure happens (vanilla calls it from
//! SCR_ConsumableItemComponent when the item-use animation completes)
[BaseContainerProps()]
class RZ_ConsumableAntidote : SCR_ConsumableEffectBase
{
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffect(notnull IEntity target, notnull IEntity user, out SCR_EConsumableFailReason failReason)
	{
		return RZ_ConsumableGate.Allows(target, user, false, failReason);
	}

	//------------------------------------------------------------------------------------------------
	override void ApplyEffect(notnull IEntity target, notnull IEntity user, IEntity item, ItemUseParameters animParams)
	{
		super.ApplyEffect(target, user, item, animParams);

		if (!Replication.IsServer())
			return;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(target);
		if (playerId <= 0)
			return;

		if (gm.GetInfection())
			gm.GetInfection().UseAntidote(playerId);
		else
			gm.CurePlayer(playerId);
	}
}

//------------------------------------------------------------------------------------------------
//! When the inventory consumables (antidote, treatments) may be used. Vanilla asks on the using
//! player's machine, where the infection HUD state (RZ_PlayerUIComponent) is known.
class RZ_ConsumableGate
{
	//------------------------------------------------------------------------------------------------
	//! Self only, alive, and bitten as far as the HUD shows (any stage for the antidote; a bite that
	//! has taken for a treatment). Without the UI component it allows, and the server decides.
	static bool Allows(IEntity target, IEntity user, bool needsTaken, out SCR_EConsumableFailReason failReason)
	{
		failReason = SCR_EConsumableFailReason.NONE;
		if (target != user || !RZ_Util.IsCharacterAlive(target))
		{
			failReason = SCR_EConsumableFailReason.NOT_APPLICABLE;
			return false;
		}

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return true;

		int state = ui.GetInfectionState();
		bool ok = state != RZ_EInfectionUI.NONE;
		if (needsTaken)
			ok = state != RZ_EInfectionUI.NONE && state != RZ_EInfectionUI.GOLDEN;

		if (!ok)
			failReason = SCR_EConsumableFailReason.NOT_APPLICABLE;

		return ok;
	}
}
