//------------------------------------------------------------------------------------------------
//! People's looks (docs/DESIGN.md, "People's looks"). A faction person slot (a guard, an occupant, a
//! quartermaster, a road-post or truck crew, a hunter squad, a council head, an outpost leader) can
//! list variant prefabs. Each body picks one from a stable seed, so the same person always wears
//! the same clothes and the next one along wears something else. A slot without a list keeps its
//! own prefab. The game mode registers every slot's list once (RZ_GameModeComponent.RegisterLooks)
//! and every spawn of a faction person goes through Pick. Server only.
class RZ_Looks
{
	protected ref map<string, ref array<ResourceName>> m_mLooks = new map<string, ref array<ResourceName>>();

	//------------------------------------------------------------------------------------------------
	//! A slot's variants. Nothing for an empty prefab or list; a second list for the same prefab
	//! adds to the first (two slots can share a prefab).
	void Add(ResourceName prefab, array<ResourceName> looks)
	{
		if (prefab.IsEmpty() || !looks || looks.IsEmpty())
			return;

		array<ResourceName> known;
		if (!m_mLooks.Find(prefab, known))
		{
			known = {};
			m_mLooks.Insert(prefab, known);
		}

		foreach (ResourceName look : looks)
		{
			if (!look.IsEmpty() && known.Find(look) < 0)
				known.Insert(look);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The look a body of this slot wears: one of its variants by the seed, else the prefab itself.
	ResourceName Pick(ResourceName prefab, int seed)
	{
		if (prefab.IsEmpty())
			return prefab;

		array<ResourceName> looks;
		if (!m_mLooks.Find(prefab, looks))
			return prefab;

		return PickFrom(prefab, looks, seed);
	}

	//------------------------------------------------------------------------------------------------
	//! One of a list by the seed (any int, negative too), else the fallback. Consecutive seeds give
	//! different looks, so give the people of one place consecutive seeds.
	static ResourceName PickFrom(ResourceName fallback, array<ResourceName> looks, int seed)
	{
		if (!looks || looks.IsEmpty())
			return fallback;

		int i = RZ_Util.PickIndex(seed, looks.Count());
		if (i < 0 || looks[i].IsEmpty())
			return fallback;

		return looks[i];
	}

	//------------------------------------------------------------------------------------------------
	//! A seed from a place, for bodies with no lasting identity (a mission's group, a hunter squad):
	//! the same spot gives the same look.
	static int SeedAt(vector pos)
	{
		return RZ_Util.RoundToInt(pos[0]) * 31 + RZ_Util.RoundToInt(pos[2]);
	}

	//------------------------------------------------------------------------------------------------
	//! A seed for the index-th person of something with an id: places start their row at different looks.
	static int SeedOf(int id, int index)
	{
		return id * 7 + index;
	}
}
