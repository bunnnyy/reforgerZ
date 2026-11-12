//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "ReforgerZ", description: "Marks an item as an antidote dose, so scripts can find it in an inventory.")]
class RZ_AntidoteItemComponentClass : ScriptComponentClass
{
}

//! Put this on the antidote prefab next to RZ_InjectAntidoteAction. Missions such
//! as The Bitten Friend look for it in players' inventories.
class RZ_AntidoteItemComponent : ScriptComponent
{
}

//------------------------------------------------------------------------------------------------
class RZ_AntidoteSearchPredicate : InventorySearchPredicate
{
	//------------------------------------------------------------------------------------------------
	void RZ_AntidoteSearchPredicate()
	{
		QueryComponentTypes.Insert(RZ_AntidoteItemComponent);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		return true;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_AntidoteHelper
{
	//------------------------------------------------------------------------------------------------
	//! True if the character carries an antidote (the talk screen shows the need before you choose).
	static bool Has(IEntity character)
	{
		if (!character)
			return false;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return false;

		array<IEntity> items = {};
		RZ_AntidoteSearchPredicate predicate = new RZ_AntidoteSearchPredicate();
		inv.FindItems(items, predicate);
		return !items.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! Removes one antidote from the character's inventory. True only if one was really removed.
	static bool TakeOne(IEntity character)
	{
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return false;

		array<IEntity> items = {};
		RZ_AntidoteSearchPredicate predicate = new RZ_AntidoteSearchPredicate();
		inv.FindItems(items, predicate);
		if (items.IsEmpty())
			return false;

		return inv.TryDeleteItem(items[0]);
	}
}
