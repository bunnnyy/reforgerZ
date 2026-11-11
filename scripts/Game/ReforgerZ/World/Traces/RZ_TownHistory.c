//! A town's history (docs/TRACES.md, "Data model"): up to 8 events a town (what happened, by whom,
//! the day, how many, where), the facts the door marks and scenes are generated from. Written by
//! RZ_TraceWriters; read by the admin view, and later by the epilogue's census. Saved.
//! Server only.

//------------------------------------------------------------------------------------------------
class RZ_TownEvent
{
	string kind;		// "rescue", "walkout", "dead", "taken", "founded", "lost", "racket", "debt",
						// "requisition", "cull", "samples", "infested", "dark", "emptied", "death", "pyre"
	int faction = -1;
	int day;			// as the broadcast counts it
	int count;
	float x;
	float z;
}

//------------------------------------------------------------------------------------------------
class RZ_TownHistorySave
{
	int town;
	ref array<ref RZ_TownEvent> events = {};
}

//------------------------------------------------------------------------------------------------
class RZ_TownHistory
{
	static const int PER_TOWN = 8;

	protected ref map<int, ref array<ref RZ_TownEvent>> m_mTowns = new map<int, ref array<ref RZ_TownEvent>>();

	//------------------------------------------------------------------------------------------------
	void Add(int town, string kind, int faction, int count, vector pos)
	{
		if (town < 0)
			return;

		array<ref RZ_TownEvent> list;
		if (!m_mTowns.Find(town, list))
		{
			list = {};
			m_mTowns.Set(town, list);
		}

		RZ_TownEvent e = new RZ_TownEvent();
		e.kind = kind;
		e.faction = faction;
		e.day = RZ_WorldFacts.Get().Day();
		e.count = count;
		e.x = pos[0];
		e.z = pos[2];
		list.Insert(e);
		while (list.Count() > PER_TOWN)
		{
			list.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The town's events, oldest first (null if none).
	array<ref RZ_TownEvent> Get(int town)
	{
		array<ref RZ_TownEvent> list;
		m_mTowns.Find(town, list);
		return list;
	}

	//------------------------------------------------------------------------------------------------
	int Count()
	{
		int n = 0;
		foreach (int town, array<ref RZ_TownEvent> list : m_mTowns)
		{
			n += list.Count();
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	void Clear(int town)
	{
		m_mTowns.Remove(town);
	}

	//------------------------------------------------------------------------------------------------
	void Save(notnull array<ref RZ_TownHistorySave> result)
	{
		foreach (int town, array<ref RZ_TownEvent> list : m_mTowns)
		{
			RZ_TownHistorySave s = new RZ_TownHistorySave();
			s.town = town;
			foreach (RZ_TownEvent e : list)
			{
				s.events.Insert(e);
			}

			result.Insert(s);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Restore(array<ref RZ_TownHistorySave> saved)
	{
		if (!saved)
			return;

		foreach (RZ_TownHistorySave s : saved)
		{
			if (!s || !s.events)
				continue;

			array<ref RZ_TownEvent> list = {};
			foreach (RZ_TownEvent e : s.events)
			{
				if (e)
					list.Insert(e);
			}

			m_mTowns.Set(s.town, list);
		}
	}
}
