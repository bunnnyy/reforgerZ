//! Antidote as an inventory consumable: lets players use it from the inventory
//! like morphine, instead of dropping it and using the world action.
//!
//! Setup: on the antidote prefab's SCR_ConsumableItemComponent, set the
//! consumable effect to RZ_ConsumableAntidote. Keep RZ_InjectAntidoteAction as
//! well; it still works from the ground.
//!
//! VERIFY: the whole class. SCR_ConsumableEffectBase's ApplyEffect signature has
//! changed between game versions. If this file doesn't compile, delete it: the
//! world action keeps working without it.
[BaseContainerProps()]
class RZ_ConsumableAntidote : SCR_ConsumableEffectBase
{
	//------------------------------------------------------------------------------------------------
	override void ApplyEffect(notnull IEntity target, notnull IEntity user, IEntity item, ItemUseParameters animParams) // VERIFY: signature
	{
		super.ApplyEffect(target, user, item, animParams);

		if (!Replication.IsServer())
			return;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(target);
		if (playerId > 0)
			gm.CurePlayer(playerId);
	}
}
