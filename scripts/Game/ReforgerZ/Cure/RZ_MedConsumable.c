//------------------------------------------------------------------------------------------------
//! A treatment as an inventory consumable, like RZ_ConsumableAntidote. Set it as the consumable
//! effect on the prefab's SCR_ConsumableItemComponent and pick the kind.
//!
//! Only on yourself, and only once the HUD shows the bite has taken (RZ_ConsumableGate). A dose
//! the server still refuses (one already taken this life) is used up all the same.
//!
//! VERIFY: as for RZ_ConsumableAntidote, ApplyEffect also runs on the server. RZ_UseMedAction
//! (Cure/RZ_Meds.c) still works from the ground either way.
[BaseContainerProps()]
class RZ_ConsumableMed : SCR_ConsumableEffectBase
{
	[Attribute("1", UIWidgets.ComboBox, "Which treatment", "", ParamEnumArray.FromEnum(RZ_EMed))]
	protected RZ_EMed m_eKind;

	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffect(notnull IEntity target, notnull IEntity user, out SCR_EConsumableFailReason failReason)
	{
		return RZ_ConsumableGate.Allows(target, user, true, failReason);
	}

	//------------------------------------------------------------------------------------------------
	override void ApplyEffect(notnull IEntity target, notnull IEntity user, IEntity item, ItemUseParameters animParams)
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
