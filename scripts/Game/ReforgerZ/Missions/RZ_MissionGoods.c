//! Mission goods (docs/DESIGN.md, "Story blocks"): anything a mission or an errand put in a player's
//! hands as goods (a delivery's food, a post's ration paid in kind, an errand's carried item) is
//! tagged with whose it is. Tagged goods never count for Trade, and food delivery, the fence, the
//! quartermasters and every errand refuse them: "These are Levie's tins. Take them back to Levie."
//! The tag is a server-side list of the entities (as the fence's bought list is): it follows the item
//! through inventories, and a restart forgets it. VERIFY: an item keeps its entity through inventory
//! moves, a body and a car's boot. Server only.
class RZ_MissionGoods
{
	protected static const int MEMORY = 200;

	protected static ref array<IEntity> s_aItems;
	protected static ref array<string> s_aOwners;

	//------------------------------------------------------------------------------------------------
	//! `item` is `owner`'s goods ("Levie", "the Haven", "the post at Morton").
	static void Tag(IEntity item, string owner)
	{
		if (!item || owner.IsEmpty())
			return;

		if (!s_aItems)
		{
			s_aItems = new array<IEntity>();
			s_aOwners = new array<string>();
		}

		// Deleted items leave nulls: drop those first, then the oldest
		for (int i = s_aItems.Count() - 1; i >= 0; i--)
		{
			if (!s_aItems[i])
			{
				s_aItems.RemoveOrdered(i);
				s_aOwners.RemoveOrdered(i);
			}
		}

		while (s_aItems.Count() >= MEMORY)
		{
			s_aItems.RemoveOrdered(0);
			s_aOwners.RemoveOrdered(0);
		}

		s_aItems.Insert(item);
		s_aOwners.Insert(owner);
	}

	//------------------------------------------------------------------------------------------------
	static bool IsGoods(IEntity item)
	{
		return item && s_aItems && s_aItems.Contains(item);
	}

	//------------------------------------------------------------------------------------------------
	//! Whose goods `item` is, "" if it's nobody's.
	static string Owner(IEntity item)
	{
		if (!IsGoods(item))
			return "";

		return s_aOwners[s_aItems.Find(item)];
	}

	//------------------------------------------------------------------------------------------------
	//! The refusal for a character carrying someone's goods ("" if they carry none): the first owner.
	static string Refusal(IEntity character)
	{
		if (!character || !s_aItems || s_aItems.IsEmpty())
			return "";

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return "";

		array<IEntity> items = {};
		inv.GetItems(items);
		foreach (IEntity item : items)
		{
			string owner = Owner(item);
			if (!owner.IsEmpty())
				return string.Format("\"These are %1's. Take them back to %1.\"", owner);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A prefab put in this character's pack (or at their feet when it won't fit), tagged as `owner`'s.
	//! The item, or null with no prefab.
	static IEntity Give(IEntity character, ResourceName prefab, string owner)
	{
		if (!character || prefab.IsEmpty())
			return null;

		IEntity item = RZ_Util.SpawnPrefab(prefab, RZ_Util.GroundPos(RZ_Util.RandomPointInRing(character.GetOrigin(), 0.5, 1.2)) + Vector(0, 0.2, 0));
		if (!item)
			return null;

		RZ_Util.InsertIntoInventory(character, item);
		Tag(item, owner);
		return item;
	}
}
