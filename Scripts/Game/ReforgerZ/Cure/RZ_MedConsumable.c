//------------------------------------------------------------------------------------------------
//! A treatment as an inventory consumable, like RZ_ConsumableAntidote. Set it as the consumable
//! effect on the prefab's SCR_ConsumableItemComponent and pick the kind.
//!
//! VERIFY: the whole class, like RZ_ConsumableAntidote. If this file doesn't compile, delete
//! it: RZ_UseMedAction (Cure/RZ_Meds.c) still works from the ground.
//! (The item is used up by the consumable either way, even when the treatment refuses.)
[BaseContainerProps()]
class RZ_ConsumableMed : SCR_ConsumableEffectBase
{
	[Attribute("1", UIWidgets.ComboBox, "Which treatment", "", ParamEnumArray.FromEnum(RZ_EMed))]
	protected RZ_EMed m_eKind;

	//------------------------------------------------------------------------------------------------
	override void ApplyEffect(notnull IEntity target, notnull IEntity user, IEntity item, ItemUseParameters animParams) // VERIFY: signature, as in RZ_ConsumableAntidote
	{
		super.ApplyEffect(target, user, item, animParams);

		if (!Replication.IsServer())
			return;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetInfection())
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(target);
		if (playerId > 0)
			gm.GetInfection().UseMed(playerId, m_eKind);
	}
}
