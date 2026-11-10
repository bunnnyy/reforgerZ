//! Treatment items and Halcyon's trial dose (docs/DESIGN.md, "Treatments"). What each one does is
//! in RZ_Infection.UseMed; these are the pieces that go on prefabs.

//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "ReforgerZ", description: "Marks an item as a treatment (stabiliser, expired antibiotics, veterinary antiviral, cut dose, fever suppressant, home remedy).")]
class RZ_MedItemComponentClass : ScriptComponentClass
{
}

//! Put this on each treatment prefab with RZ_UseMedAction, and set its kind.
class RZ_MedItemComponent : ScriptComponent
{
	[Attribute("1", UIWidgets.ComboBox, "Which treatment this is (not ANTIDOTE: the antidote has its own component and action)", "", ParamEnumArray.FromEnum(RZ_EMed))]
	protected RZ_EMed m_eKind;

	//------------------------------------------------------------------------------------------------
	RZ_EMed GetKind()
	{
		return m_eKind;
	}
}

//------------------------------------------------------------------------------------------------
//! "Take it": a world action on the treatment prefab, like the antidote's. Drop the item and use it
//! from the ground (or someone else's). It's only used up if it does something.
class RZ_UseMedAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Take it";
		IEntity owner = GetOwner();
		if (!owner)
			return true;

		RZ_MedItemComponent med = RZ_MedItemComponent.Cast(owner.FindComponent(RZ_MedItemComponent));
		if (med)
			outName = "Take " + RZ_Infection.MedName(med.GetKind());

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
		RZ_MedItemComponent med = RZ_MedItemComponent.Cast(pOwnerEntity.FindComponent(RZ_MedItemComponent));
		if (!gm || !gm.GetInfection() || !med)
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		if (playerId <= 0)
			return;

		if (gm.GetInfection().UseMed(playerId, med.GetKind()))
			SCR_EntityHelper.DeleteEntityAndChildren(pOwnerEntity);
	}
}

//------------------------------------------------------------------------------------------------
//! "Volunteer for the trial dose": on Halcyon's quartermaster. For anyone bitten, once per life,
//! free: about 80% cure, otherwise a faster turn, and a little Halcyon Fame either way.
class RZ_TrialDoseAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Volunteer for the trial dose (bitten only)";
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
		int playerId = RZ_QuartermasterHelper.GetPlayerId(pUserEntity);
		if (gm && gm.GetInfection() && playerId > 0)
			gm.GetInfection().TrialDose(playerId);
	}
}
