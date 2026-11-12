//! Keys are real items (docs/DESIGN.md, "Locks and keys"): Interactions > Search > Key prefab,
//! spawned with the id of the lock it opens. They lie loose in houses (next to a letter, or a
//! car's keys in its town: RZ_Loot) or are handed over (on a body, by the Mechanic, as a reward:
//! RZ_Search.GiveKey). They can be dropped, traded and taken off a body like any item. The id
//! stays on the server; only the tag ("Pharmacy, Lindale") is replicated, for the read action.
//! Without the prefab there are no keys.

//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "ReforgerZ", description: "A key item: holds the id of the lock it opens and its tag. Put it on the key prefab (Interactions > Search > Key prefab) with an RplComponent; optionally add RZ_ReadKeyTagAction.")]
class RZ_KeyItemComponentClass : ScriptComponentClass
{
}

//------------------------------------------------------------------------------------------------
class RZ_KeyItemComponent : ScriptComponent
{
	protected string m_sKeyId;	// server only: "pharmacy@3", "house:Blue@5", "car@2"

	[RplProp()]
	protected string m_sTag;	// "Pharmacy, Lindale"

	//------------------------------------------------------------------------------------------------
	//! Server only.
	void SetKey(string keyId, string tag)
	{
		m_sKeyId = keyId;
		m_sTag = tag;
		Replication.BumpMe(); // needs the item's RplComponent
	}

	string GetKeyId() { return m_sKeyId; }
	string GetTag() { return m_sTag; }

	//------------------------------------------------------------------------------------------------
	static RZ_KeyItemComponent Of(IEntity e)
	{
		if (!e)
			return null;

		return RZ_KeyItemComponent.Cast(e.FindComponent(RZ_KeyItemComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns a key for `keyId` lying at pos. null if the prefab has no RZ_KeyItemComponent or
	//! didn't spawn.
	static IEntity Spawn(ResourceName prefab, vector pos, float yaw, string keyId, string tag)
	{
		if (prefab.IsEmpty())
			return null;

		IEntity item = RZ_Util.SpawnPrefab(prefab, pos, yaw);
		RZ_KeyItemComponent key = Of(item);
		if (!key)
		{
			if (item)
				SCR_EntityHelper.DeleteEntityAndChildren(item);

			return null;
		}

		key.SetKey(keyId, tag);
		return item;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns a key for `keyId` and puts it in the character's inventory (at their feet if it
	//! doesn't fit). False if the prefab has no RZ_KeyItemComponent or didn't spawn.
	static bool Give(ResourceName prefab, IEntity character, string keyId, string tag)
	{
		if (!character)
			return false;

		vector pos = RZ_Util.RandomPointInRing(character.GetOrigin(), 0.3, 0.6) + Vector(0, 0.3, 0);
		IEntity item = Spawn(prefab, pos, Math.RandomFloat(0, 360), keyId, tag);
		if (!item)
			return false;

		RZ_Util.InsertIntoInventory(character, item);
		RZ_GameModeComponent.TrackLooseItem(item); // a key dropped and forgotten is cleaned up in time
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! True if the character carries a key item with this id (server side).
	static bool Carries(IEntity character, string keyId)
	{
		if (!character || keyId.IsEmpty())
			return false;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return false;

		array<IEntity> items = {};
		inv.GetItems(items); // VERIFY: GetItems() also finds a key in a bag inside another bag
		foreach (IEntity item : items)
		{
			RZ_KeyItemComponent key = Of(item);
			if (key && key.GetKeyId() == keyId)
				return true;
		}

		return false;
	}
}

//------------------------------------------------------------------------------------------------
//! "Read the tag": on the key prefab, for a key lying on the ground or in a container. The prompt
//! names the tag already ("Key: blue door, Morton"); reading it opens the tag in the reader.
class RZ_ReadKeyTagAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Read the tag";
		RZ_KeyItemComponent key = RZ_KeyItemComponent.Of(GetOwner());
		if (key && !key.GetTag().IsEmpty())
			outName = "Key: " + key.GetTag();

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
		int playerId = RZ_PuzzleActionHelper.GetPlayerId(pUserEntity);
		if (!gm || playerId <= 0)
			return;

		RZ_KeyItemComponent key = RZ_KeyItemComponent.Of(pOwnerEntity);
		if (!key || key.GetTag().IsEmpty())
		{
			gm.NotifyPlayer(playerId, "A key", "A key on a ring. The tag's gone. It opens something, somewhere.");
			return;
		}

		// The tag opens in the reader, like any paper (a card when the reader is off)
		RZ_Reader.Show(gm, playerId, "A key", "A cardboard tag on the ring, in biro", string.Format("\"%1\"", key.GetTag()));
	}
}
