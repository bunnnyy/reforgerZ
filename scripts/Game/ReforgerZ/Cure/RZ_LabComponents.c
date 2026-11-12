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
			total += ItemValue(item);
		}

		return total;
	}

	//------------------------------------------------------------------------------------------------
	//! How many samples one item counts as (0 if it isn't one).
	static int ItemValue(IEntity item)
	{
		if (!item)
			return 0;

		RZ_SampleItemComponent s = RZ_SampleItemComponent.Cast(item.FindComponent(RZ_SampleItemComponent));
		if (s)
			return s.GetValue();

		return 0;
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

		return CanTakeNow();
	}

	//------------------------------------------------------------------------------------------------
	//! On every machine, before the action shows as possible. Default: yes.
	protected bool CanTakeNow()
	{
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

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		if (!CanAccept(gm, playerId, pOwnerEntity))
			return;

		// How many the receiver takes now (the lab: what fits in its backlog); the rest stay
		int want = TakeCount(gm, playerId, value, pOwnerEntity);
		if (want <= 0)
			return;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(pUserEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return;

		int taken = 0;
		foreach (IEntity item : items)
		{
			if (taken >= want)
				break;

			// The item counts only once it's really gone (its value read first, while it still exists)
			int itemValue = RZ_LabHelper.ItemValue(item);
			if (inv.TryDeleteItem(item))
				taken += itemValue;
		}

		if (taken <= 0)
			return;

		Deliver(gm, playerId, taken, pOwnerEntity);
		if (taken < value)
			OnPartlyTaken(gm, playerId, taken, value);
	}

	//------------------------------------------------------------------------------------------------
	//! On the server, after CanAccept: how many of the player's samples (by value) to take. Default: all.
	protected int TakeCount(RZ_GameModeComponent gm, int playerId, int value, IEntity receiver)
	{
		return value;
	}

	//------------------------------------------------------------------------------------------------
	//! Only some were taken (TakeCount); the rest are still on the player.
	protected void OnPartlyTaken(RZ_GameModeComponent gm, int playerId, int taken, int offered)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Checked on the server before any sample is taken. Tell the player why if it refuses.
	protected bool CanAccept(RZ_GameModeComponent gm, int playerId, IEntity receiver)
	{
		return playerId > 0;
	}

	//------------------------------------------------------------------------------------------------
	protected void Deliver(RZ_GameModeComponent gm, int playerId, int value, IEntity receiver)
	{
	}
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

	//------------------------------------------------------------------------------------------------
	//! After the cure, or while the lab's backlog is full, the lab takes no samples (the replicated
	//! flags, so clients agree).
	override protected bool CanTakeNow()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm && gm.IsReplicatedCured())
		{
			SetCannotPerformReason("The cure is out");
			return false;
		}

		if (gm && gm.IsReplicatedLabFull())
		{
			SetCannotPerformReason("The cold store is full");
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! On the server, before anything is taken: the cure is out, or the cold store is full, so the
	//! samples stay with you (no Fame for them).
	override protected bool CanAccept(RZ_GameModeComponent gm, int playerId, IEntity receiver)
	{
		if (playerId <= 0)
			return false;

		if (gm.GetFacts().IsCured())
		{
			gm.NotifyPlayer(playerId, "Halcyon", "\"Keep them. The cure is out: we don't need them any more. I never thought I'd say that.\"");
			return false;
		}

		if (gm.GetCure().IsLabFull())
		{
			if (gm.GetCure().IsAtFinaleGate())
				gm.NotifyPlayer(playerId, "Halcyon", "\"The cold store is full, and the work has stopped at the last step. Samples won't finish it. Keep them.\"");
			else
				gm.NotifyPlayer(playerId, "Halcyon", "\"The cold store is full. We can't take another sample until the lab works through what's waiting. Come back tomorrow.\"");

			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! As many as fit in the lab's backlog.
	override protected int TakeCount(RZ_GameModeComponent gm, int playerId, int value, IEntity receiver)
	{
		return gm.GetCure().AcceptsSamples(playerId, value, gm.GetResearchPerSample());
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnPartlyTaken(RZ_GameModeComponent gm, int playerId, int taken, int offered)
	{
		gm.NotifyPlayer(playerId, "Halcyon", string.Format("\"We can take %1 of them. The cold store is full after that: keep the other %2 and come back tomorrow.\"", taken, offered - taken));
	}
}

//------------------------------------------------------------------------------------------------
//! "Sell samples": on the Red Hand dead-drop prefab. Pays loot, Red Hand Fame and
//! Favor, costs Halcyon standing, and slows the cure for everyone. Only players
//! the Red Hand deals with (contract requirements) can sell.
class RZ_SellSamplesToBanditsAction : RZ_SampleDeliveryActionBase
{
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Leave samples for the Red Hand";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool CanAccept(RZ_GameModeComponent gm, int playerId, IEntity receiver)
	{
		if (playerId <= 0)
			return false;

		// After the cure nobody pays for samples, not even the Red Hand
		if (gm.GetFacts().IsCured())
		{
			gm.NotifyPlayer(playerId, "The Red Hand", "Nobody comes out to meet you. The cure is out: samples aren't worth anything to anyone now.");
			return false;
		}

		return gm.GetMissions().CanSellToBandits(playerId, receiver);
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

		// "Research 63 percent. Waiting: 14 samples' worth, about 4 days of work."
		string text = string.Format("%1\nOutbreak stage: %2", gm.DescribeLabReplicated(),
			RZ_StageSettings.GetStageName(gm.GetReplicatedStage()));

		SCR_HintManagerComponent.ShowCustomHint(text, "Halcyon lab", 8);
	}
}
