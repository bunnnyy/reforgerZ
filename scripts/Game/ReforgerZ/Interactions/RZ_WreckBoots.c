//! Car wrecks (docs/DESIGN.md, "Car wrecks"): a wreck is one container, its boot. One prompt,
//! "Open the boot", opens vanilla's inventory with the boot's things in it, and you drag out what
//! you want. Nothing else: wrecks are burnt-out shells, so there's no fuel in them and no strip
//! action (a car part is sometimes in the boot instead).
//!
//! - Data first. A wreck the look-around sees (RZ_Interactions, a wreck spot) gets a small record:
//!   when it was rolled and emptied, and its things as data (prefab, a key's id and tag).
//! - Only near players. While the wreck's spot is out (a player within the look-around's reach),
//!   an invisible storage (Cars > Wreck boot prefab) stands at the wreck's rear, filled from the
//!   record. When everyone has gone, what's left is kept as data and the storage is deleted with
//!   it. Nothing physical is saved.
//! - Rolled the first time it's put out: the car boot table (RZ_Search.PickBootLoot: something
//!   Find chance x the town's loot level of the time, sometimes two), two good finds more for a
//!   stash someone wrote about, one treatment roll, sometimes a car part (battery, tyre or spark
//!   plugs), and sometimes something to read (left beside the boot, a key taped inside goes in it).
//! - Taken: on the 5 s tick a thing that's no longer in the boot was taken. A loot find takes the
//!   town's loot down (RZ_TownMemory.ReportSearched); from then on it's an ordinary item.
//! - Refill: an emptied boot has things again after Wreck return minutes; one with leftovers
//!   keeps them and rolls fresh after three times that. Only the time is saved.
//! - Take only: nothing from a character's inventory goes into a boot (RZ_WreckBootStorageComponent),
//!   so a wreck is never a stash that could lose what's in it.

//------------------------------------------------------------------------------------------------
class RZ_WreckBootStorageComponentClass : SCR_UniversalInventoryStorageComponentClass
{
}

//------------------------------------------------------------------------------------------------
//! The wreck boot's storage (the Wreck boot prefab): vanilla's universal storage, take only.
//! VERIFY: dragging an item from your inventory into the boot asks CanStoreItem (on the client
//! and the server) and is refused; the server's own fill (a fresh item with no parent) and its
//! replication to clients aren't.
class RZ_WreckBootStorageComponent : SCR_UniversalInventoryStorageComponent
{
	//------------------------------------------------------------------------------------------------
	override bool CanStoreItem(IEntity item, int slotID)
	{
		if (RZ_WreckBoots.CarriedByCharacter(item))
			return false;

		return super.CanStoreItem(item, slotID);
	}
}

//------------------------------------------------------------------------------------------------
//! One thing in a wreck's boot. Data; m_Ent exists only while the boot is out.
class RZ_BootItem
{
	ResourceName m_sPrefab;
	string m_sKey;			// a key item: the lock it opens ("" for anything else)
	string m_sTag;
	bool m_bTownLoot;		// a loot find: taking it uses up the town's loot
	IEntity m_Ent;
}

//------------------------------------------------------------------------------------------------
//! One wreck's boot.
class RZ_WreckBoot
{
	string m_sKey;
	vector m_vPos;
	float m_fRolledAt = -1;
	float m_fEmptiedAt = -1;
	float m_fSeen;
	ref array<ref RZ_BootItem> m_aItems = {};
	IEntity m_Box;
}

//------------------------------------------------------------------------------------------------
class RZ_WreckBoots
{
	protected static const float STALE_MULT = 3;	// leftovers roll again after this many return times
	protected static const int RECORDS_KEPT = 300;
	protected static const int TIMES_KEPT = 800;
	protected static const string SAVE_PREFIX = "boot:";
	protected static const float BOOT_HEIGHT = 0.9;	// the boot's point this high (m) above the wreck's bottom, at most
	protected static const float BOOT_GAP = 0.1;	// and this far (m) behind its rear

	protected RZ_GameModeComponent m_GM;
	protected RZ_Interactions m_Hub;
	protected RZ_CarSettings m_Settings;
	protected ref map<string, ref RZ_WreckBoot> m_mBoots = new map<string, ref RZ_WreckBoot>();
	protected ref map<string, float> m_mTimes = new map<string, float>();	// wreck key: emptied at, with no record (saved, or forgotten)
	protected bool m_bFitWarned;

	//------------------------------------------------------------------------------------------------
	void RZ_WreckBoots(RZ_GameModeComponent gm, RZ_Interactions hub, RZ_CarSettings settings)
	{
		m_GM = gm;
		m_Hub = hub;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	//! Wrecks are on (Cars) and the boot prefab is set.
	bool IsOn()
	{
		return m_Settings.m_bEnabled && !m_Settings.m_sBootPrefab.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! In a character's inventory (or held): somewhere under a character, a few levels up.
	static bool CarriedByCharacter(IEntity item)
	{
		if (!item)
			return false;

		IEntity p = item.GetParent();
		for (int i = 0; i < 8 && p; i++)
		{
			if (ChimeraCharacter.Cast(p))
				return true;

			p = p.GetParent();
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Where a wreck's boot is: just behind the middle of its rear, at boot height, from its local
	//! box. Its origin a little up if it has no usable box.
	//! VERIFY: a wreck prop faces its local +Z like a vehicle (so its rear is the box's -Z side)
	static vector BootPoint(IEntity wreck)
	{
		if (!wreck)
			return vector.Zero;

		vector mins;
		vector maxs;
		wreck.GetBounds(mins, maxs);
		if (maxs[0] - mins[0] < 1 || maxs[2] - mins[2] < 1)
			return wreck.GetOrigin() + Vector(0, 0.6, 0);

		float up = Math.Min(BOOT_HEIGHT, (maxs[1] - mins[1]) * 0.6);
		vector local = Vector((mins[0] + maxs[0]) * 0.5, mins[1] + up, mins[2] - BOOT_GAP);
		vector mat[4];
		wreck.GetWorldTransform(mat);
		return mat[3] + mat[0] * local[0] + mat[1] * local[1] + mat[2] * local[2];
	}

	// ================================================================ Save / load (times only)
	//------------------------------------------------------------------------------------------------
	//! Items aren't saved: after a restart a boot counts as emptied when it was emptied (or rolled),
	//! so it has things again once Wreck return minutes have passed.
	void SaveInto(RZ_InteractionSave data)
	{
		float now = m_Hub.GetClock();
		float keep = m_Settings.m_fWreckReturnMinutes * 60;
		int saved = 0;
		foreach (string key, RZ_WreckBoot b : m_mBoots)
		{
			float at = EmptiedForSave(b);
			if (at >= 0 && now - at < keep && saved < TIMES_KEPT)
			{
				data.wreckKey.Insert(SAVE_PREFIX + key);
				data.wreckAt.Insert(at);
				saved++;
			}
		}

		foreach (string tkey, float tat : m_mTimes)
		{
			if (now - tat < keep && saved < TIMES_KEPT)
			{
				data.wreckKey.Insert(SAVE_PREFIX + tkey);
				data.wreckAt.Insert(tat);
				saved++;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected float EmptiedForSave(RZ_WreckBoot b)
	{
		if (b.m_fEmptiedAt >= 0)
			return b.m_fEmptiedAt;

		return b.m_fRolledAt;
	}

	//------------------------------------------------------------------------------------------------
	void RestoreFrom(RZ_InteractionSave data)
	{
		int len = SAVE_PREFIX.Length();
		for (int i = 0; i < data.wreckKey.Count() && i < data.wreckAt.Count(); i++)
		{
			string key = data.wreckKey[i];
			if (key.Length() > len && key.Substring(0, len) == SAVE_PREFIX)
				m_mTimes.Set(key.Substring(len, key.Length() - len), data.wreckAt[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A key RZ_Cars keeps (the pump's), not a boot's.
	static bool IsBootKey(string key)
	{
		int len = SAVE_PREFIX.Length();
		return key.Length() > len && key.Substring(0, len) == SAVE_PREFIX;
	}

	// ================================================================ From the hub's wreck spots
	//------------------------------------------------------------------------------------------------
	//! The wreck's spot is out: its boot stands at the spot, filled. The storage, null if wrecks
	//! are off or it didn't spawn.
	IEntity PutOut(RZ_Spot s)
	{
		if (!s || !IsOn())
			return null;

		RZ_WreckBoot b = Record(s);
		if (b.m_Box)
			return b.m_Box;

		if (NeedsRoll(b))
			Roll(b);

		b.m_Box = RZ_Util.SpawnPrefab(m_Settings.m_sBootPrefab, b.m_vPos, 0);
		if (!b.m_Box)
			return null;

		Fill(b);
		return b.m_Box;
	}

	//------------------------------------------------------------------------------------------------
	//! The wreck's spot goes: what's left in the boot is kept as data and the storage is deleted.
	void PutAway(RZ_Spot s)
	{
		if (!s)
			return;

		RZ_WreckBoot b;
		if (!m_mBoots.Find(s.m_sKey, b) || !b.m_Box)
			return;

		CheckTaken(b);

		// Something put in from the ground (only that can go in): kept like the rest, as data
		array<IEntity> inside = {};
		Contents(b.m_Box, inside);
		foreach (IEntity e : inside)
		{
			if (!e || Holds(b, e))
				continue;

			// Inside something else in the boot: it goes with that
			IEntity parent = e.GetParent();
			if (parent && parent != b.m_Box && inside.Find(parent) >= 0)
				continue;

			ResourceName prefab = RZ_Util.GetPrefabName(e);
			if (prefab.IsEmpty())
				continue;

			RZ_BootItem extra = new RZ_BootItem();
			extra.m_sPrefab = prefab;
			extra.m_Ent = e;
			RZ_KeyItemComponent key = RZ_KeyItemComponent.Of(e);
			if (key)
			{
				extra.m_sKey = key.GetKeyId();
				extra.m_sTag = key.GetTag();
			}

			b.m_aItems.Insert(extra);
		}

		InventoryStorageManagerComponent mgr = InventoryStorageManagerComponent.Cast(b.m_Box.FindComponent(InventoryStorageManagerComponent));
		foreach (RZ_BootItem it : b.m_aItems)
		{
			if (it.m_Ent && (!mgr || !mgr.TryDeleteItem(it.m_Ent)))
				SCR_EntityHelper.DeleteEntityAndChildren(it.m_Ent);

			it.m_Ent = null;
		}

		SCR_EntityHelper.DeleteEntityAndChildren(b.m_Box);
		b.m_Box = null;
		b.m_fSeen = m_Hub.GetClock();
	}

	//------------------------------------------------------------------------------------------------
	//! On the 5 s tick: what was taken out of the boots that are out.
	void Tick()
	{
		foreach (string key, RZ_WreckBoot b : m_mBoots)
		{
			if (b.m_Box)
			{
				CheckTaken(b);
				b.m_fSeen = m_Hub.GetClock();
			}
		}
	}

	// ================================================================ Records
	//------------------------------------------------------------------------------------------------
	protected RZ_WreckBoot Record(RZ_Spot s)
	{
		RZ_WreckBoot b;
		if (m_mBoots.Find(s.m_sKey, b))
		{
			b.m_vPos = s.m_vPos;
			return b;
		}

		b = new RZ_WreckBoot();
		b.m_sKey = s.m_sKey;
		b.m_vPos = s.m_vPos;
		b.m_fSeen = m_Hub.GetClock();
		float at;
		if (m_mTimes.Find(s.m_sKey, at))
		{
			// Saved, or forgotten: empty since then
			b.m_fRolledAt = at;
			b.m_fEmptiedAt = at;
			m_mTimes.Remove(s.m_sKey);
		}

		m_mBoots.Set(s.m_sKey, b);
		if (m_mBoots.Count() > RECORDS_KEPT)
			Prune();

		return b;
	}

	//------------------------------------------------------------------------------------------------
	//! The boot seen longest ago (and not out) goes; its time is kept.
	protected void Prune()
	{
		string oldest;
		float oldestAt = float.MAX;
		foreach (string key, RZ_WreckBoot b : m_mBoots)
		{
			if (!b.m_Box && b.m_fSeen < oldestAt)
			{
				oldestAt = b.m_fSeen;
				oldest = key;
			}
		}

		if (oldest.IsEmpty())
			return;

		RZ_WreckBoot gone = m_mBoots.Get(oldest);
		float at = EmptiedForSave(gone);
		if (at >= 0 && m_mTimes.Count() < TIMES_KEPT)
			m_mTimes.Set(oldest, at);

		m_mBoots.Remove(oldest);
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasItems(RZ_WreckBoot b)
	{
		return !b.m_aItems.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	protected bool Holds(RZ_WreckBoot b, IEntity e)
	{
		foreach (RZ_BootItem it : b.m_aItems)
		{
			if (it.m_Ent == e)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool NeedsRoll(RZ_WreckBoot b)
	{
		if (b.m_fRolledAt < 0)
			return true;

		float now = m_Hub.GetClock();
		float back = m_Settings.m_fWreckReturnMinutes * 60;
		if (now - b.m_fRolledAt >= back * STALE_MULT)
			return true;

		return !HasItems(b) && b.m_fEmptiedAt >= 0 && now - b.m_fEmptiedAt >= back;
	}

	// ================================================================ Rolling, filling, taking
	//------------------------------------------------------------------------------------------------
	//! What's in the boot now: the car boot table, a stash's two good finds, a treatment,
	//! sometimes a car part and sometimes something to read.
	protected void Roll(RZ_WreckBoot b)
	{
		b.m_aItems.Clear();
		b.m_fRolledAt = m_Hub.GetClock();
		b.m_fEmptiedAt = -1;

		RZ_Search search = m_Hub.GetSearch();
		if (search.IsEnabled())
		{
			array<ResourceName> picks = {};
			search.PickBootLoot(b.m_vPos, false, picks);

			// A stash someone wrote about: two more good finds
			if (search.TakeStashNear(b.m_vPos))
			{
				search.PickBootLoot(b.m_vPos, true, picks);
				search.PickBootLoot(b.m_vPos, true, picks);
			}

			foreach (ResourceName pick : picks)
			{
				Add(b, pick, "", "", true);
			}
		}

		// Treatments (docs/DESIGN.md, "Treatments"): one roll
		RZ_Infection infection = m_GM.GetInfection();
		if (infection)
		{
			int med;
			ResourceName treatment = infection.PickMedFind("car", false, med);
			if (!treatment.IsEmpty())
				Add(b, treatment, "", "", false);
		}

		// What stripping a wreck used to give: one car part, sometimes
		if (Math.RandomFloat01() < m_Settings.m_fWreckPartChance)
		{
			ResourceName part = m_Hub.GetCars().RandomPart();
			if (!part.IsEmpty())
				Add(b, part, "", "", false);
		}

		// Something to read, left beside the boot; a key taped inside goes in it
		if (search.IsEnabled() && Math.RandomFloat01() < search.GetReadableChance())
			AddReadable(b);

		if (!HasItems(b))
			b.m_fEmptiedAt = b.m_fRolledAt;
	}

	//------------------------------------------------------------------------------------------------
	protected void AddReadable(RZ_WreckBoot b)
	{
		RZ_Search search = m_Hub.GetSearch();
		string keyId;
		string keyTag;
		string line;
		int note = search.CreateReadable(b.m_vPos, false, false, keyId, keyTag, line);
		if (note < 0)
			return;

		vector at = RZ_Util.GroundPos(RZ_Util.RandomPointInRing(b.m_vPos, 0.6, 1.2));
		search.SpawnNote(note, at + Vector(0, 0.2, 0), Math.RandomFloat(0, 360));
		if (!keyId.IsEmpty())
			Add(b, search.GetKeyPrefab(), keyId, keyTag, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void Add(RZ_WreckBoot b, ResourceName prefab, string keyId, string tag, bool townLoot)
	{
		if (prefab.IsEmpty())
			return;

		RZ_BootItem it = new RZ_BootItem();
		it.m_sPrefab = prefab;
		it.m_sKey = keyId;
		it.m_sTag = tag;
		it.m_bTownLoot = townLoot;
		b.m_aItems.Insert(it);
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns the record's things into the storage. One that doesn't fit lies beside the wreck,
	//! an ordinary loose item.
	protected void Fill(RZ_WreckBoot b)
	{
		for (int i = b.m_aItems.Count() - 1; i >= 0; i--)
		{
			RZ_BootItem it = b.m_aItems[i];
			vector at = RZ_Util.GroundPos(RZ_Util.RandomPointInRing(b.m_vPos, 0.6, 1)) + Vector(0, 0.3, 0);
			if (it.m_sKey.IsEmpty())
				it.m_Ent = RZ_Util.SpawnPrefab(it.m_sPrefab, at, Math.RandomFloat(0, 360));
			else
				it.m_Ent = RZ_KeyItemComponent.Spawn(it.m_sPrefab, at, Math.RandomFloat(0, 360), it.m_sKey, it.m_sTag);

			if (!it.m_Ent)
			{
				b.m_aItems.Remove(i);
				continue;
			}

			if (RZ_Util.InsertIntoStorage(b.m_Box, it.m_Ent))
				continue;

			if (!m_bFitWarned)
			{
				m_bFitWarned = true;
				RZ_Log.Warn("Wreck boot: " + it.m_sPrefab + " doesn't fit in the boot (Cars > Wreck boot prefab: its storage's volume, size or weight); it lies beside the wreck.");
			}

			RZ_GameModeComponent.TrackLooseItem(it.m_Ent);
			b.m_aItems.Remove(i);
		}

		if (!HasItems(b) && b.m_fEmptiedAt < 0)
			b.m_fEmptiedAt = m_Hub.GetClock();
	}

	//------------------------------------------------------------------------------------------------
	//! Things no longer in the boot were taken (or deleted by something else).
	protected void CheckTaken(RZ_WreckBoot b)
	{
		array<IEntity> inside = {};
		Contents(b.m_Box, inside);
		for (int i = b.m_aItems.Count() - 1; i >= 0; i--)
		{
			RZ_BootItem it = b.m_aItems[i];
			if (!it.m_Ent)
			{
				b.m_aItems.Remove(i);
				continue;
			}

			if (inside.Find(it.m_Ent) >= 0 || it.m_Ent.GetParent() == b.m_Box)
				continue;

			if (it.m_bTownLoot)
				RZ_TownMemory.ReportSearched(b.m_vPos, m_Hub.GetLoot().GetTownLootPerItem());

			// Dropped and forgotten later: cleaned up in time
			RZ_GameModeComponent.TrackLooseItem(it.m_Ent);
			b.m_aItems.Remove(i);
		}

		if (!HasItems(b) && b.m_fEmptiedAt < 0)
			b.m_fEmptiedAt = m_Hub.GetClock();
	}

	//------------------------------------------------------------------------------------------------
	//! Everything in the storage.
	protected static void Contents(IEntity box, notnull array<IEntity> items)
	{
		if (!box)
			return;

		BaseInventoryStorageComponent storage = BaseInventoryStorageComponent.Cast(box.FindComponent(BaseInventoryStorageComponent));
		if (storage)
			storage.GetAll(items);
	}
}
