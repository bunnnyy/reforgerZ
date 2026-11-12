//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "ReforgerZ", description: "Marks an item as a mission objective (black box, data drive, package). The mission sets its id when it spawns the item.")]
class RZ_MissionItemComponentClass : ScriptComponentClass
{
}

//! Put this on the mission item prefab (an item with an InventoryItemComponent).
//! The mission id lives only on the server, which is the only place it's read.
class RZ_MissionItemComponent : ScriptComponent
{
	protected int m_iMissionId = -1;

	//------------------------------------------------------------------------------------------------
	void SetMissionId(int id)
	{
		m_iMissionId = id;
	}

	//------------------------------------------------------------------------------------------------
	int GetMissionId()
	{
		return m_iMissionId;
	}

	//------------------------------------------------------------------------------------------------
	static void Tag(IEntity item, int missionId)
	{
		if (!item)
			return;

		RZ_MissionItemComponent comp = RZ_MissionItemComponent.Cast(item.FindComponent(RZ_MissionItemComponent));
		if (comp)
			comp.SetMissionId(missionId);
		else
			RZ_Log.Warn("Mission item prefab has no RZ_MissionItemComponent; it can't be handed in.");
	}
}

//------------------------------------------------------------------------------------------------
class RZ_MissionItemSearchPredicate : InventorySearchPredicate
{
	//------------------------------------------------------------------------------------------------
	void RZ_MissionItemSearchPredicate()
	{
		QueryComponentTypes.Insert(RZ_MissionItemComponent);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		return true;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_MissionItemHelper
{
	//------------------------------------------------------------------------------------------------
	static void FindItems(IEntity user, notnull array<IEntity> outItems)
	{
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return;

		RZ_MissionItemSearchPredicate predicate = new RZ_MissionItemSearchPredicate();
		inv.FindItems(outItems, predicate);
	}

	//------------------------------------------------------------------------------------------------
	//! Removes a handed-in mission item. If the inventory won't let go of it, the entity is deleted
	//! outright, so a paid hand-in never leaves the item behind.
	static void RemoveItem(IEntity user, IEntity item)
	{
		if (!item)
			return;

		SCR_InventoryStorageManagerComponent inv;
		if (user)
			inv = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));

		if (inv && inv.TryDeleteItem(item))
			return;

		SCR_EntityHelper.DeleteEntityAndChildren(item);
	}
}

//------------------------------------------------------------------------------------------------
//! "Hand over to <faction>". Put this on a faction's drop-point prefab (and,
//! if you like, on that faction's quartermaster prefab). Set the faction in the
//! action's attributes. Each mission decides whether it takes the item, so the
//! same drop point serves deliveries and choice missions.
class RZ_ChoiceDeliveryAction : ScriptedUserAction
{
	[Attribute("0", UIWidgets.ComboBox, "Faction that receives items handed in here", "", ParamEnumArray.FromEnum(RZ_EFaction))]
	protected RZ_EFaction m_eFaction;

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Hand over to " + RZ_Factions.GetName(m_eFaction);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		array<IEntity> items = {};
		RZ_MissionItemHelper.FindItems(user, items);
		if (items.IsEmpty())
		{
			SetCannotPerformReason("Nothing to hand over");
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
		if (!gm || !gm.GetMissions())
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		if (playerId <= 0)
			return;

		array<IEntity> items = {};
		RZ_MissionItemHelper.FindItems(pUserEntity, items);
		bool anyTaken = false;
		foreach (IEntity item : items)
		{
			RZ_MissionItemComponent comp = RZ_MissionItemComponent.Cast(item.FindComponent(RZ_MissionItemComponent));
			if (!comp)
				continue;

			if (gm.GetMissions().OnItemDelivered(playerId, comp.GetMissionId(), m_eFaction, pOwnerEntity))
			{
				anyTaken = true;
				RZ_MissionItemHelper.RemoveItem(pUserEntity, item);
			}
		}

		if (!anyTaken)
			gm.NotifyPlayer(playerId, RZ_Factions.GetName(m_eFaction), "They look at what you're carrying and shake their heads. It's not something they asked for.");
	}
}
