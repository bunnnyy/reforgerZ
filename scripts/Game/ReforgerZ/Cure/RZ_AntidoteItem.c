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
	// VERIFY: IsMatch signature in your game version (same as RZ_SampleSearchPredicate)
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		return true;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_AntidoteHelper
{
	//------------------------------------------------------------------------------------------------
	//! Removes one antidote from the character's inventory. True if there was one.
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

		inv.TryDeleteItem(items[0]); // VERIFY: TryDeleteItem()
		return true;
	}
}
