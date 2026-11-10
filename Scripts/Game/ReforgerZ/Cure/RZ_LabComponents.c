//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "ReforgerZ", description: "Marks an entity as the cure research lab. Add RZ_DepositSamplesAction and RZ_LabStatusAction to its ActionsManagerComponent.")]
class RZ_LabComponentClass : ScriptComponentClass
{
}

//! Put this on a lab prefab or composition. If a mission maker places one in the
//! world, that one is used. Otherwise the game mode spawns its lab prefab in a
//! random town.
class RZ_LabComponent : ScriptComponent
{
	protected static ref array<RZ_LabComponent> s_aLabs;

	//------------------------------------------------------------------------------------------------
	static RZ_LabComponent GetFirst()
	{
		if (s_aLabs && !s_aLabs.IsEmpty())
			return s_aLabs[0];

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (!s_aLabs)
			s_aLabs = new array<RZ_LabComponent>();

		s_aLabs.Insert(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (s_aLabs)
			s_aLabs.RemoveItem(this);

		super.OnDelete(owner);
	}
}

//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "ReforgerZ", description: "Marks an item as an infected research sample.")]
class RZ_SampleItemComponentClass : ScriptComponentClass
{
}

//! Put this on the sample item prefab (an item with an InventoryItemComponent).
class RZ_SampleItemComponent : ScriptComponent
{
	[Attribute("1", desc: "How many samples this item counts as")]
	protected int m_iValue;

	//------------------------------------------------------------------------------------------------
	int GetValue()
	{
		return m_iValue;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_SampleSearchPredicate : InventorySearchPredicate
{
	//------------------------------------------------------------------------------------------------
	void RZ_SampleSearchPredicate()
	{
		QueryComponentTypes.Insert(RZ_SampleItemComponent);
	}

	//------------------------------------------------------------------------------------------------
	// VERIFY: IsMatch signature in your game version
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		return true;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_LabHelper
{
	//------------------------------------------------------------------------------------------------
	static void FindSamples(IEntity user, notnull array<IEntity> outItems)
	{
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return;

		RZ_SampleSearchPredicate predicate = new RZ_SampleSearchPredicate();
		inv.FindItems(outItems, predicate);
	}

	//------------------------------------------------------------------------------------------------
	static int CountValue(array<IEntity> items)
	{
		int total = 0;
		foreach (IEntity item : items)
		{
			RZ_SampleItemComponent s = RZ_SampleItemComponent.Cast(item.FindComponent(RZ_SampleItemComponent));
			if (s)
				total += s.GetValue();
		}

		return total;
	}
}

//------------------------------------------------------------------------------------------------
//! Shared logic for handing in the samples a player is carrying. Subclasses
//! decide who receives them: the lab (research) or bandits (loot, no research).
class RZ_SampleDeliveryActionBase : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		array<IEntity> items = {};
		RZ_LabHelper.FindSamples(user, items);
		if (items.IsEmpty())
		{
			SetCannotPerformReason("No samples");
			return false;
		}

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

		array<IEntity> items = {};
		RZ_LabHelper.FindSamples(pUserEntity, items);
		int value = RZ_LabHelper.CountValue(items);
		if (value <= 0)
			return;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(pUserEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		foreach (IEntity item : items)
		{
			inv.TryDeleteItem(item); // VERIFY: TryDeleteItem()
		}

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		Deliver(gm, playerId, value, pOwnerEntity);
	}

	//------------------------------------------------------------------------------------------------
	protected void Deliver(RZ_GameModeComponent gm, int playerId, int value, IEntity receiver) {}
}

//------------------------------------------------------------------------------------------------
//! "Deliver infected samples": the lab takes every sample the player is carrying.
class RZ_DepositSamplesAction : RZ_SampleDeliveryActionBase
{
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Deliver infected samples";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void Deliver(RZ_GameModeComponent gm, int playerId, int value, IEntity receiver)
	{
		gm.DeliverSamples(playerId, value);
	}
}

//------------------------------------------------------------------------------------------------
//! "Sell samples": on the bandit dead-drop prefab. Pays loot and bandit standing,
//! costs Institute standing, and slows the cure for everyone.
class RZ_SellSamplesToBanditsAction : RZ_SampleDeliveryActionBase
{
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Leave samples for the bandits";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void Deliver(RZ_GameModeComponent gm, int playerId, int value, IEntity receiver)
	{
		gm.GetMissions().OnSamplesSoldToBandits(playerId, value, receiver);
	}
}

//------------------------------------------------------------------------------------------------
//! "Check research progress": a local-only readout from replicated values.
class RZ_LabStatusAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Check research progress";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return;

		string text = string.Format("Research: %1 percent\nOutbreak stage: %2",
			RZ_Util.RoundToInt(gm.GetReplicatedCureProgress()),
			RZ_StageSettings.GetStageName(gm.GetReplicatedStage()));

		if (gm.IsReplicatedCured())
			text = "The cure is complete.\n" + text;

		SCR_HintManagerComponent.ShowCustomHint(text, "Institute Lab", 8); // VERIFY: ShowCustomHint
	}
}
