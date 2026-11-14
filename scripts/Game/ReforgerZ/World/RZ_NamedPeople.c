//! Named people (docs/ERRANDS.md, "Givers"): the small, named residents of every holding kind who
//! give errands. Territory's built ones (the cook, the sentry, the toll man: three per outpost and
//! stronghold, saved with the holding) are read from RZ_Territory; the Haven's safehouses get four
//! of their own here (a cook, a lookout, a storekeeper and a clerk), saved with the errands. Two of
//! a place's people are dressed onto the bodies the place already shows while a player is near:
//! territory's named occupants, and the first two of a safehouse's occupants. They are never
//! leaders: no errand reads or writes a leader's memory, regard, loyalty or attitude. What they
//! remember of you is dialogue's regard, on their own keys only ("named:<holding>:<i>",
//! "house:<safehouse>:<i>"). Server only.

//------------------------------------------------------------------------------------------------
//! Where a named person lives.
enum RZ_ENamedPlace
{
	HOLDING,	// an outpost or stronghold (RZ_Territory)
	HOUSE,		// a Haven safehouse (RZ_Safehouses)
	WANDERER	// a wanderer on the roads (RZ_Wanderers), for their own asks
}

//------------------------------------------------------------------------------------------------
//! One named person, as the errands see them. Built on demand; never stored.
class RZ_NamedPerson
{
	string m_sKey;			// "named:12:1", "house:3:0", "wanderer:5"
	string m_sName;			// "Mara Kovac"
	string m_sRole;			// "cook"
	int m_iFaction = -1;	// RZ_EFaction, -1 for a wanderer
	RZ_ENamedPlace m_eKind;
	int m_iPlace = -1;		// holding id, safehouse index or wanderer id
	int m_iIndex;
	bool m_bStronghold;
	vector m_vPos;			// the place (a wanderer: where they stand)
	string m_sPlace;		// "Levie"
	string m_sTitle;		// the speaker's title, as their own lines come under it

	//------------------------------------------------------------------------------------------------
	//! "the cook", "the lookout"
	string RoleText()
	{
		return "the " + m_sRole;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_NamedHouseSave
{
	int house;
	ref array<string> names = {};
	ref array<string> roles = {};
}

//------------------------------------------------------------------------------------------------
class RZ_NamedPeople
{
	static const int TAG_HOUSE = -13;	// RZ_PuzzleObjectComponent / RZ_ClueComponent mission id: a safehouse's named person
	static const int HOUSE_PEOPLE = 4;
	static const int HOUSE_SHOWN = 2;
	static const string LABELS = "Ask how things are|Anything need doing?";

	protected RZ_GameModeComponent m_GM;
	protected ref map<int, ref RZ_NamedHouseSave> m_mHouses = new map<int, ref RZ_NamedHouseSave>();
	protected ref map<string, IEntity> m_mBodies = new map<string, IEntity>();	// giver key -> the body wearing them (runtime)

	//------------------------------------------------------------------------------------------------
	void RZ_NamedPeople(RZ_GameModeComponent gm)
	{
		m_GM = gm;
	}

	//------------------------------------------------------------------------------------------------
	static string HoldingKey(int holdingId, int idx)
	{
		return string.Format("named:%1:%2", holdingId, idx);
	}

	//------------------------------------------------------------------------------------------------
	static string HouseKey(int house, int idx)
	{
		return string.Format("house:%1:%2", house, idx);
	}

	//------------------------------------------------------------------------------------------------
	static string WandererKey(int wandererId)
	{
		return "wanderer:" + wandererId.ToString();
	}

	// ================================================================ The safehouses' own people
	//------------------------------------------------------------------------------------------------
	//! A safehouse's four, made once from the season's seed and the house.
	protected RZ_NamedHouseSave House(int idx)
	{
		RZ_NamedHouseSave h;
		if (m_mHouses.Find(idx, h))
			return h;

		h = new RZ_NamedHouseSave();
		h.house = idx;
		RZ_Rng rng = new RZ_Rng(RZ_WorldFacts.Get().GetSeed() + idx * 7919 + 17);
		array<string> roles = {"cook", "lookout", "storekeeper", "clerk"};
		for (int i = 0; i < HOUSE_PEOPLE; i++)
		{
			string name = RZ_FactionVoice.PersonName(rng, RZ_EFaction.SURVIVORS);
			if (h.names.Contains(name))
				name = RZ_FactionVoice.PersonName(rng, RZ_EFaction.SURVIVORS);

			h.names.Insert(name);
			h.roles.Insert(roles[i]);
		}

		m_mHouses.Set(idx, h);
		return h;
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_Safehouses.SpawnBodies: the first two occupants wear the house's named people (their
	//! actions now come here, RZ_Errands.HandleHouseTalk); the rest stay plain occupants.
	void DressHouse(int idx, notnull array<IEntity> occupants)
	{
		RZ_NamedHouseSave h = House(idx);
		int dressed = 0;
		foreach (IEntity body : occupants)
		{
			if (dressed >= HOUSE_SHOWN || dressed >= h.names.Count())
				break;

			if (!body || !body.FindComponent(RZ_PuzzleObjectComponent))
				continue;

			int code = idx * 10 + dressed;
			RZ_PuzzleObjectComponent.Tag(body, TAG_HOUSE, code, "", LABELS);
			RZ_ClueComponent.Tag(body, TAG_HOUSE, code, "Talk to " + h.names[dressed]);
			m_mBodies.Set(HouseKey(idx, dressed), body);
			dressed++;
		}

		// The plain occupants after them may wear the house's locals (World/RZ_Locals.c)
		RZ_Errands errands = m_GM.GetErrands();
		if (!errands || dressed >= occupants.Count())
			return;

		array<IEntity> spare = {};
		for (int i = dressed; i < occupants.Count(); i++)
		{
			spare.Insert(occupants[i]);
		}

		errands.GetLocals().Dress(idx, spare);
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_Territory.SpawnBodies: a named occupant's body, so errands can reach it.
	void NoteBody(string key, IEntity body)
	{
		if (body)
			m_mBodies.Set(key, body);
	}

	//------------------------------------------------------------------------------------------------
	//! A holding's named people killed since the last call (their body is there, and dead): their keys
	//! and where they fell. A body that was only cleared away is forgotten quietly. Safehouse people
	//! are left alone: the house counts its own dead (RZ_Safehouses).
	void TakeDeadAtHoldings(notnull array<string> keys, notnull array<vector> at)
	{
		array<string> drop = {};
		foreach (string k, IEntity body : m_mBodies)
		{
			if (k.IndexOf("named:") != 0)
				continue;

			if (!body)
			{
				drop.Insert(k);
				continue;
			}

			if (RZ_Util.IsCharacterAlive(body))
				continue;

			keys.Insert(k);
			at.Insert(body.GetOrigin());
			drop.Insert(k);
		}

		foreach (string d : drop)
		{
			m_mBodies.Remove(d);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The body wearing this person now, or null (nobody near, or never shown).
	IEntity BodyOf(string key)
	{
		IEntity e;
		m_mBodies.Find(key, e);
		return e;
	}

	// ================================================================ Who someone is
	//------------------------------------------------------------------------------------------------
	//! The person behind a giver key, or null if they don't exist (the holding is gone, the house
	//! emptied, the wanderer died).
	RZ_NamedPerson Get(string key)
	{
		array<string> parts = {};
		key.Split(":", parts, false);
		if (parts.Count() < 2)
			return null;

		if (parts[0] == "named" && parts.Count() == 3)
			return FromHolding(parts[1].ToInt(), parts[2].ToInt());

		if (parts[0] == "house" && parts.Count() == 3)
			return FromHouse(parts[1].ToInt(), parts[2].ToInt());

		if (parts[0] == "wanderer")
			return FromWanderer(parts[1].ToInt());

		return null;
	}

	//------------------------------------------------------------------------------------------------
	RZ_NamedPerson FromHolding(int holdingId, int idx)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		if (!territory)
			return null;

		RZ_Holding hold = territory.GetHolding(holdingId);
		if (!hold || idx < 0 || idx >= hold.m_aPeople.Count() || idx >= hold.m_aRoles.Count())
			return null;

		RZ_NamedPerson p = new RZ_NamedPerson();
		p.m_sKey = HoldingKey(holdingId, idx);
		p.m_sName = hold.m_aPeople[idx];
		p.m_sRole = hold.m_aRoles[idx];
		p.m_iFaction = hold.m_eFaction;
		p.m_eKind = RZ_ENamedPlace.HOLDING;
		p.m_iPlace = holdingId;
		p.m_iIndex = idx;
		p.m_bStronghold = hold.m_bStronghold;
		p.m_vPos = hold.m_vPos;
		p.m_sPlace = hold.m_sName;
		p.m_sTitle = hold.m_aPeople[idx] + ", " + hold.GetTitle();
		return p;
	}

	//------------------------------------------------------------------------------------------------
	RZ_NamedPerson FromHouse(int house, int idx)
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses)
			return null;

		RZ_Safehouse h = houses.Get(house);
		if (!h || h.m_iPopulation <= 0)
			return null;

		RZ_NamedHouseSave people = House(house);
		if (idx < 0 || idx >= people.names.Count())
			return null;

		RZ_NamedPerson p = new RZ_NamedPerson();
		p.m_sKey = HouseKey(house, idx);
		p.m_sName = people.names[idx];
		p.m_sRole = people.roles[idx];
		p.m_iFaction = RZ_EFaction.SURVIVORS;
		p.m_eKind = RZ_ENamedPlace.HOUSE;
		p.m_iPlace = house;
		p.m_iIndex = idx;
		p.m_vPos = h.m_vPos;
		p.m_sPlace = h.m_sName;
		p.m_sTitle = string.Format("%1, %2 at the safehouse in %3", people.names[idx], people.roles[idx], h.m_sName);
		return p;
	}

	//------------------------------------------------------------------------------------------------
	RZ_NamedPerson FromWanderer(int wandererId)
	{
		RZ_Wanderers wanderers = m_GM.GetWanderers();
		if (!wanderers)
			return null;

		RZ_Wanderer w = wanderers.FindById(wandererId);
		if (!w || !w.m_bAlive)
			return null;

		RZ_NamedPerson p = new RZ_NamedPerson();
		p.m_sKey = WandererKey(wandererId);
		p.m_sName = w.GetShortName();
		p.m_sRole = w.GetRoleName();
		p.m_iFaction = -1;
		p.m_eKind = RZ_ENamedPlace.WANDERER;
		p.m_iPlace = wandererId;
		p.m_vPos = w.m_vPos;
		p.m_sPlace = w.m_sDestName;
		p.m_sTitle = w.GetTitle();
		return p;
	}

	//------------------------------------------------------------------------------------------------
	//! Everyone shown at a place now (their bodies stand there), first the ones with a body.
	void ShownAt(RZ_ENamedPlace kind, int place, notnull array<ref RZ_NamedPerson> result)
	{
		for (int i = 0; i < HOUSE_PEOPLE; i++)
		{
			string key = HoldingKey(place, i);
			if (kind == RZ_ENamedPlace.HOUSE)
				key = HouseKey(place, i);

			if (!BodyOf(key))
				continue;

			RZ_NamedPerson p = Get(key);
			if (p)
				result.Insert(p);
		}
	}

	// ================================================================ Save
	//------------------------------------------------------------------------------------------------
	void Save(notnull array<ref RZ_NamedHouseSave> result)
	{
		foreach (int idx, RZ_NamedHouseSave h : m_mHouses)
		{
			result.Insert(h);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Restore(array<ref RZ_NamedHouseSave> saved)
	{
		if (!saved)
			return;

		foreach (RZ_NamedHouseSave h : saved)
		{
			if (!h || !h.names || !h.roles || h.names.Count() != h.roles.Count())
				continue;

			// An older save's women's names (everyone met is a man now)
			for (int i = 0; i < h.names.Count(); i++)
			{
				h.names[i] = RZ_NoteText.ManFromOld(h.names[i]);
			}

			m_mHouses.Set(h.house, h);
		}
	}
}
