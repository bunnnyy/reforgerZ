//! The task place audit (docs/PLACES.md, "The audit"): asks the game's own pickers for places across
//! the map and checks each against its need with the same code the live check uses
//! (World/RZ_PlaceNeeds.c). Headless: the test driver (tools/runtime_test/run.sh tasks) starts it
//! once the land grid is ready and steps it with a time budget; it spawns only for its spawn tests
//! (a step's remains and sample, the contact, a key), each deleted again at once. Prints per kind of
//! task: how many were asked, placed and passed, the failing places with why, a score, and the
//! lines tools/runtime_test/places.py draws. Server only; never runs in play.

//------------------------------------------------------------------------------------------------
class RZ_PlaceAuditKind
{
	string m_sKey;
	int m_iAsked;
	int m_iPlaced;
	int m_iPassed;
	int m_iShown;
	ref map<string, int> m_mWhy = new map<string, int>();
	string m_sFallbacks;

	//------------------------------------------------------------------------------------------------
	int Score()
	{
		if (m_iPlaced <= 0)
			return 100;

		return RZ_Util.RoundToInt(m_iPassed * 100.0 / m_iPlaced);
	}
}

//------------------------------------------------------------------------------------------------
//! A start the openings' roll gave: its contact's spot, for the chain's steps.
class RZ_PlaceAuditStart
{
	string m_sBackground;
	vector m_vContact;
}

//------------------------------------------------------------------------------------------------
//! One errand way to place from one giver.
class RZ_PlaceAuditAsk
{
	RZ_ErrandDef m_Def;
	RZ_ErrandWay m_Way;
	vector m_vGiver;
}

//------------------------------------------------------------------------------------------------
class RZ_PlaceAudit
{
	static int s_iThreshold = 95;	// a kind of task that places and scores under this fails the audit
	static int s_iRolls = 3;		// the openings' place roll, this many times per background
	static int s_iGivers = 6;		// errand givers asked per anchor
	static int s_iShown = 8;		// failing places printed per kind

	protected static const int PHASE_STARTS = 0;
	protected static const int PHASE_STEPS = 1;
	protected static const int PHASE_ERRANDS = 2;
	protected static const int PHASE_SPAWNS = 3;
	protected static const int PHASE_WORLD = 4;
	protected static const int PHASE_DONE = 5;

	protected RZ_GameModeComponent m_GM;
	protected ref array<ref RZ_PlaceAuditKind> m_aKinds = {};
	protected ref array<ref RZ_PlaceAuditStart> m_aStarts = {};
	protected ref array<ref RZ_PlaceAuditAsk> m_aAsks = {};
	protected ref array<string> m_aBackgrounds = {"nurse", "doctor", "mechanic", "hunter", "police", "excon", "labtech", "radioham", "unemployed", "soldier", "raider", "elder", "drifter"};
	protected int m_iPhase;
	protected int m_iNext;

	//------------------------------------------------------------------------------------------------
	void RZ_PlaceAudit(RZ_GameModeComponent gm)
	{
		m_GM = gm;
	}

	//------------------------------------------------------------------------------------------------
	bool IsDone()
	{
		return m_iPhase >= PHASE_DONE;
	}

	//------------------------------------------------------------------------------------------------
	//! The towns, for the drawing.
	void Start()
	{
		foreach (RZ_Settlement s : m_GM.GetSettlements())
		{
			RZ_Log.Info(string.Format("Places town: %1 %2 %3 %4", Num(s.m_vCenter[0]), Num(s.m_vCenter[2]), Num(s.m_fRadius), s.m_sName));
		}

		array<string> keys = {};
		RZ_PlaceNeeds.Keys(keys);
		RZ_Log.Info(string.Format("Places audit: %1 needs, %2 backgrounds x %3 rolls, %4 givers an anchor, threshold %5", keys.Count(), m_aBackgrounds.Count(), s_iRolls, s_iGivers, s_iThreshold));
	}

	//------------------------------------------------------------------------------------------------
	//! Works for about `budgetMs`; true once it's all done.
	bool Step(int budgetMs)
	{
		float start = System.GetTickCount();
		while (m_iPhase < PHASE_DONE && System.GetTickCount() - start < budgetMs)
		{
			StepOnce();
		}

		return IsDone();
	}

	//------------------------------------------------------------------------------------------------
	protected void StepOnce()
	{
		if (m_iPhase == PHASE_STARTS)
		{
			if (m_iNext >= m_aBackgrounds.Count() * s_iRolls)
			{
				NextPhase();
				return;
			}

			AskStart(m_aBackgrounds[m_iNext % m_aBackgrounds.Count()]);
			m_iNext++;
			return;
		}

		if (m_iPhase == PHASE_STEPS)
		{
			if (m_iNext >= m_aStarts.Count())
			{
				NextPhase();
				BuildErrandAsks();
				return;
			}

			AskSteps(m_aStarts[m_iNext]);
			m_iNext++;
			return;
		}

		if (m_iPhase == PHASE_ERRANDS)
		{
			if (m_iNext >= m_aAsks.Count())
			{
				NextPhase();
				return;
			}

			AskErrand(m_aAsks[m_iNext]);
			m_iNext++;
			return;
		}

		if (m_iPhase == PHASE_SPAWNS)
		{
			if (m_iNext >= m_GM.GetSettlements().Count())
			{
				NextPhase();
				return;
			}

			AskSpawnPoint(m_iNext);
			m_iNext++;
			return;
		}

		if (m_iPhase == PHASE_WORLD)
		{
			AskWorld();
			NextPhase();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void NextPhase()
	{
		m_iPhase++;
		m_iNext = 0;
	}

	// ================================================================ Openings
	//------------------------------------------------------------------------------------------------
	//! The place roll for a brand-new player of this background (no ends: anywhere).
	protected void AskStart(string bg)
	{
		RZ_Openings openings = RZ_Openings.Get();
		if (!openings)
			return;

		RZ_PlayerProfile profile = new RZ_PlayerProfile();
		array<vector> ends = {};
		RZ_OpeningPlace p = openings.GetRoll().Roll(bg, 0, profile, ends);
		if (!p)
		{
			Record("opening.start", vector.Zero, vector.Zero, false, bg + ": no place", -1, "");
			RZ_Log.Info(string.Format("Places declined: opening.start for %1: %2", bg, openings.GetRoll().WhyText()));

			// The life then starts at a spawn point, the contact on it: its steps are asked from there
			vector spot = AnySpawnPoint();
			if (spot == vector.Zero)
				return;

			RZ_PlaceAuditStart sp = new RZ_PlaceAuditStart();
			sp.m_sBackground = bg;
			sp.m_vContact = spot;
			m_aStarts.Insert(sp);
			RZ_Log.Info(string.Format("Places roll: %1: the spawn point at %2", bg, RZ_SettlementScanner.GridReference(spot)));
			return;
		}

		string text = bg + ": " + p.m_sKind;
		if (!p.m_bFits)
			text += " (no fitting kind, a house)";

		string pass = "first pass";
		if (p.m_bRelaxed)
			pass = "second pass";

		RZ_Log.Info(string.Format("Places roll: %1: rank %2, %3; doors refused: %4", text, p.m_iRank, pass, openings.GetRoll().WhyText()));

		Record("opening.start", p.m_vPos, p.m_vPos, true, text, -1, "");
		RZ_PlaceAuditStart s = new RZ_PlaceAuditStart();
		s.m_sBackground = bg;
		s.m_vContact = RZ_Openings.ContactSpot(p);
		m_aStarts.Insert(s);

		// The contact: its slot, and a body made from it stands where the contact would
		SpawnTest("opening.start", s.m_vContact, RZ_PlaceThings.ContactPrefab(m_GM), text);
	}

	//------------------------------------------------------------------------------------------------
	//! A spawn point's spot in a random town, as the game mode would make one (zero if none takes).
	protected vector AnySpawnPoint()
	{
		array<ref RZ_Settlement> towns = m_GM.GetSettlements();
		for (int i = 0; i < 12 && !towns.IsEmpty(); i++)
		{
			vector at = m_GM.SpawnPointSpot(towns[Math.RandomInt(0, towns.Count())]);
			if (at != vector.Zero)
				return at;
		}

		return vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	//! From one start's contact: step 1's place and every step 2 that can run there.
	protected void AskSteps(RZ_PlaceAuditStart s)
	{
		RZ_Openings openings = RZ_Openings.Get();
		if (!openings)
			return;

		vector place;
		string text;
		string kind;
		bool found = openings.PickSearch(s.m_sBackground, s.m_vContact, place, text, kind);
		string building = kind;
		if (building.IsEmpty())
			building = "house";

		Record("opening.search", s.m_vContact, place, found, s.m_sBackground + ": " + text, -1, building);

		RZ_OpeningTasks tasks = openings.GetTasks();
		array<int> list = {RZ_EOpeningTask.WATER, RZ_EOpeningTask.PUMP, RZ_EOpeningTask.DOWNWIND, RZ_EOpeningTask.KEYS, RZ_EOpeningTask.SAMPLE, RZ_EOpeningTask.GATE, RZ_EOpeningTask.DEBT, RZ_EOpeningTask.SWAP, RZ_EOpeningTask.TOLL};
		foreach (int task : list)
		{
			RZ_Opening o = new RZ_Opening();
			o.m_iPlayer = -1;
			o.m_sName = "the contact";
			o.m_sBackground = s.m_sBackground;
			vector at;
			string said;
			bool fits = tasks.Fits(o, null, task, s.m_vContact, at, said);
			string key = RZ_OpeningTasks.NeedKey(task, said);
			if (key.IsEmpty())
				key = "opening.sample";

			Record(key, s.m_vContact, at, fits, said, -1, "");
			if (fits && task == RZ_EOpeningTask.SAMPLE)
				SampleTest(tasks, key, at, said);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A Sample's things: the remains and the sample, made by the step's own Start code, checked,
	//! and deleted again.
	protected void SampleTest(RZ_OpeningTasks tasks, string key, vector at, string said)
	{
		array<IEntity> made = {};
		tasks.SpawnSampleScene(at, made);
		string why;
		RZ_PlaceThings.Present(at, RZ_PlaceNeeds.Get(key), why);
		RecordThings(key, at, said, why);
		foreach (IEntity e : made)
		{
			if (e)
				SCR_EntityHelper.DeleteEntityAndChildren(e);
		}
	}

	// ================================================================ Errands
	//------------------------------------------------------------------------------------------------
	//! Every way with a step out of the giver's own place, from a few givers of its anchor each;
	//! one ask per need, place kind and giver.
	protected void BuildErrandAsks()
	{
		RZ_Errands errands = m_GM.GetErrands();
		if (!errands)
			return;

		array<string> seen = {};
		foreach (RZ_ErrandDef d : errands.GetDefs())
		{
			if (d.m_bHanded)
				continue;

			array<vector> givers = {};
			Givers(d.m_eAnchor, givers);
			foreach (RZ_ErrandWay w : d.m_aWays)
			{
				if (!w.HasStep() || w.m_ePlace == RZ_EErrandPlace.SUBJECT || w.m_ePlace == RZ_EErrandPlace.SUBJECT2)
					continue;

				foreach (int gi, vector g : givers)
				{
					string tag = string.Format("%1|%2|%3|%4", w.NeedKey(), w.m_ePlace, d.m_eAnchor, gi);
					if (w.m_eProp != RZ_EErrandProp.NONE)
						tag += "|prop";

					if (seen.Contains(tag))
						continue;

					seen.Insert(tag);
					RZ_PlaceAuditAsk a = new RZ_PlaceAuditAsk();
					a.m_Def = d;
					a.m_Way = w;
					a.m_vGiver = g;
					m_aAsks.Insert(a);
				}
			}
		}

		RZ_Log.Info(string.Format("Places audit: %1 starts rolled, %2 errand asks", m_aStarts.Count(), m_aAsks.Count()));
	}

	//------------------------------------------------------------------------------------------------
	//! Up to s_iGivers places of this anchor, spread over the list.
	protected void Givers(int anchor, notnull array<vector> result)
	{
		array<vector> all = {};
		RZ_Territory territory = m_GM.GetTerritory();
		if (anchor == RZ_EErrandAnchor.HAVEN_HOUSE)
		{
			RZ_Safehouses houses = m_GM.GetSafehouses();
			for (int i = 0; houses && i < houses.Count(); i++)
			{
				if (houses.Get(i))
					all.Insert(houses.Get(i).m_vPos);
			}
		}
		else if (anchor != RZ_EErrandAnchor.WANDERER && territory)
		{
			foreach (RZ_Holding h : territory.GetHoldings())
			{
				if (anchor == RZ_EErrandAnchor.ANY_HOLDING || h.m_eFaction == AnchorFaction(anchor))
					all.Insert(h.m_vPos);
			}

			if (anchor == RZ_EErrandAnchor.HALCYON_STATION && m_GM.HasLab())
				all.Insert(m_GM.GetLabPosition());
		}

		if (all.IsEmpty())
			return;

		int step = Math.Max(1, all.Count() / s_iGivers);
		for (int k = 0; k < all.Count() && result.Count() < s_iGivers; k += step)
		{
			result.Insert(all[k]);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static int AnchorFaction(int anchor)
	{
		switch (anchor)
		{
			case RZ_EErrandAnchor.BASTION_POST: return RZ_EFaction.MILITARY;
			case RZ_EErrandAnchor.REDHAND_CAMP: return RZ_EFaction.BANDITS;
			case RZ_EErrandAnchor.HALCYON_STATION: return RZ_EFaction.INSTITUTE;
		}

		return RZ_EFaction.SURVIVORS;
	}

	//------------------------------------------------------------------------------------------------
	protected void AskErrand(RZ_PlaceAuditAsk a)
	{
		RZ_Errands errands = m_GM.GetErrands();
		RZ_Errand e = new RZ_Errand();
		e.m_Def = a.m_Def;
		e.m_vGiver = a.m_vGiver;
		e.m_vOffer = a.m_vGiver;
		e.m_iSettlement = m_GM.FindNearestSettlement(a.m_vGiver);
		vector at = errands.PlaceFor(e, a.m_Way.m_ePlace, a.m_Way.NeedKey());
		string text = a.m_Def.m_sKey + ": " + a.m_Way.m_sStep;
		Record(a.m_Way.NeedKey(), a.m_vGiver, at, at != vector.Zero, text, -1, "");

		// The prop its words name: its slot is set, and the prop stands at the place
		if (at == vector.Zero || a.m_Way.m_eProp == RZ_EErrandProp.NONE)
			return;

		string propKey = "errand." + RZ_PlaceThings.PropSlot(a.m_Way.m_eProp);
		Record(propKey, a.m_vGiver, at, true, text, -1, "");
		SpawnTest(propKey, at, RZ_PlaceThings.Slot(RZ_PlaceThings.PropSlot(a.m_Way.m_eProp)), text);
	}

	// ================================================================ Spawn points
	//------------------------------------------------------------------------------------------------
	protected void AskSpawnPoint(int town)
	{
		RZ_Settlement s = m_GM.GetSettlement(town);
		if (!s)
			return;

		vector at = vector.Zero;
		for (int i = 0; i < 3 && at == vector.Zero; i++)
		{
			at = m_GM.SpawnPointSpot(s);
		}

		Record("spawn.point", s.m_vCenter, at, at != vector.Zero, s.m_sName, town, "");
	}

	// ================================================================ What stands all season
	//------------------------------------------------------------------------------------------------
	//! The towns, holdings, safehouses, the lab, the Long Table's farm and the wanderers: each on the
	//! mainland and dry (an island one is named in the fail lines).
	protected void AskWorld()
	{
		RZ_Mainland land = m_GM.GetMainland();
		if (land)
			RZ_Log.Info("Places mainland: " + land.Describe(m_GM.GetSettlements()));

		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			Record("world.place", s.m_vCenter, s.m_vCenter, true, "town " + s.m_sName, i, "");
		}

		RZ_Territory territory = m_GM.GetTerritory();
		if (territory)
		{
			foreach (RZ_Holding h : territory.GetHoldings())
			{
				Record("world.place", h.m_vPos, h.m_vPos, true, "holding " + h.m_sName, -1, "");
			}
		}

		RZ_Safehouses houses = m_GM.GetSafehouses();
		for (int k = 0; houses && k < houses.Count(); k++)
		{
			RZ_Safehouse sh = houses.Get(k);
			if (sh)
				Record("world.place", sh.m_vPos, sh.m_vPos, true, "safehouse " + sh.m_sName, -1, "");
		}

		if (m_GM.HasLab())
			Record("world.place", m_GM.GetLabPosition(), m_GM.GetLabPosition(), true, "the lab", -1, "");

		RZ_LongTable table = m_GM.GetLongTable();
		if (table && table.GetHousePos() != vector.Zero)
			Record("world.place", table.GetHousePos(), table.GetHousePos(), true, "the Long Table", -1, "");

		RZ_Wanderers wanderers = m_GM.GetWanderers();
		if (!wanderers)
			return;

		array<int> ids = {};
		wanderers.GetLiving(RZ_Wanderers.AllRoles(), ids);
		foreach (int id : ids)
		{
			vector at;
			RZ_Wanderer w = wanderers.FindById(id);
			if (w && wanderers.GetPosition(id, at))
				Record("world.place", at, at, true, "wanderer " + w.GetTitle(), -1, "");
		}
	}

	// ================================================================ Recording
	//------------------------------------------------------------------------------------------------
	protected RZ_PlaceAuditKind Kind(string key)
	{
		foreach (RZ_PlaceAuditKind k : m_aKinds)
		{
			if (k.m_sKey == key)
				return k;
		}

		RZ_PlaceAuditKind made = new RZ_PlaceAuditKind();
		made.m_sKey = key;
		m_aKinds.Insert(made);
		return made;
	}

	//------------------------------------------------------------------------------------------------
	//! One answer from a picker: placed or not, and if placed, checked against its need (the place
	//! and its things' slots).
	protected void Record(string key, vector from, vector at, bool placed, string text, int town, string building)
	{
		RZ_PlaceAuditKind k = Kind(key);
		k.m_iAsked++;
		if (!placed)
		{
			// Declining is allowed (nothing here would make the words true); the first few say why
			if (k.m_iAsked - k.m_iPlaced <= s_iShown && !key.Contains("opening.start"))
				RZ_Log.Info(string.Format("Places declined: %1 from %2 (\"%3\"): %4", key, RZ_SettlementScanner.GridReference(from), text, RZ_PlaceFind.LastWhy()));

			return;
		}

		k.m_iPlaced++;
		RZ_PlaceNeed need = RZ_PlaceNeeds.Get(key);
		string why;
		RZ_PlaceCheck.Check(at, need, town, building, why);
		string slots;
		string fallbacks;
		RZ_PlaceThings.SlotsSet(need, slots, fallbacks);
		why = RZ_PlaceCheck.Add(why, slots);
		if (!fallbacks.IsEmpty())
			k.m_sFallbacks = fallbacks;

		Result(k, from, at, text, why);
	}

	//------------------------------------------------------------------------------------------------
	//! A spawn test's answer: the things its words name stood at the place.
	protected void RecordThings(string key, vector at, string text, string why)
	{
		RZ_PlaceAuditKind k = Kind(key + " things");
		k.m_iAsked++;
		k.m_iPlaced++;
		Result(k, at, at, text, why);
	}

	//------------------------------------------------------------------------------------------------
	protected void Result(RZ_PlaceAuditKind k, vector from, vector at, string text, string why)
	{
		bool ok = why.IsEmpty();
		if (ok)
		{
			k.m_iPassed++;
		}
		else
		{
			array<string> parts = {};
			why.Split("; ", parts, true);
			foreach (string part : parts)
			{
				string gist = Gist(part);
				int n;
				k.m_mWhy.Find(gist, n);
				k.m_mWhy.Set(gist, n + 1);
			}

			if (k.m_iShown < s_iShown)
			{
				k.m_iShown++;
				RZ_Log.Info(string.Format("Places fail: %1 at %2 (\"%3\"): %4", k.m_sKey, RZ_SettlementScanner.GridReference(at), text, why));
			}
		}

		RZ_Log.Info(string.Format("Places draw: %1 %2 %3 %4 %5 %6 %7 | %8", k.m_sKey, YesNo(ok), Num(at[0]), Num(at[2]), Num(from[0]), Num(from[2]), RZ_SettlementScanner.GridReference(at), text));
	}

	//------------------------------------------------------------------------------------------------
	//! A failure without its numbers, to count alike ones together ("slope 34 deg" -> "slope").
	protected static string Gist(string why)
	{
		if (why.IndexOf("slope") == 0)
			return "too steep";

		if (why.Contains(" m from "))
			return "outside the town it names";

		if (why.Contains(" houses within"))
			return "too few houses";

		if (why.IndexOf("only ") == 0)
			return "not high ground";

		return why;
	}

	//------------------------------------------------------------------------------------------------
	//! A thing made from `prefab` at `at`, then looked for there with the same check as in play,
	//! then deleted. An empty slot is the need's slot check's business.
	protected void SpawnTest(string key, vector at, ResourceName prefab, string text)
	{
		if (prefab.IsEmpty())
			return;

		IEntity e = RZ_Util.SpawnPrefab(prefab, RZ_Util.GroundPos(at) + Vector(0, 0.2, 0), 0);
		string why;
		RZ_PlaceThings.Present(at, RZ_PlaceNeeds.Get(key), why);
		if (!e)
			why = RZ_PlaceCheck.Add(why, "the prefab didn't spawn");

		RecordThings(key, at, text, why);
		if (e)
			SCR_EntityHelper.DeleteEntityAndChildren(e);
	}

	//------------------------------------------------------------------------------------------------
	//! Per kind of task, the totals, and the kinds under the threshold. Returns how many are under.
	int Summary()
	{
		int placed = 0;
		int passed = 0;
		string under = "";
		int underCount = 0;
		foreach (RZ_PlaceAuditKind k : m_aKinds)
		{
			placed += k.m_iPlaced;
			passed += k.m_iPassed;
			string whyText = "";
			foreach (string why, int n : k.m_mWhy)
			{
				whyText = RZ_PlaceCheck.Add(whyText, string.Format("%1 x%2", why, n));
			}

			string flag = "";
			if (k.m_iPlaced > 0 && k.Score() < s_iThreshold)
			{
				flag = " UNDER";
				underCount++;
				under = RZ_PlaceCheck.Add(under, string.Format("%1 (%2)", k.m_sKey, k.Score()));
			}

			string line = string.Format("Places: %1: %2 of %3 placed, %4 pass, %5 fail, score %6%7", k.m_sKey, k.m_iPlaced, k.m_iAsked, k.m_iPassed, k.m_iPlaced - k.m_iPassed, k.Score(), flag);
			if (!whyText.IsEmpty())
				line += "; fails: " + whyText;

			if (!k.m_sFallbacks.IsEmpty())
				line += "; fallback: " + k.m_sFallbacks;

			RZ_Log.Info(line);
			string needKey = k.m_sKey;
			needKey.Replace(" things", "");
			RZ_PlaceNeed need = RZ_PlaceNeeds.Get(needKey);
			if (need)
				RZ_Log.Info(string.Format("Places need: %1: %2 | %3", k.m_sKey, need.FlagsText(), need.m_sWords));
		}

		int score = 100;
		if (placed > 0)
			score = RZ_Util.RoundToInt(passed * 100.0 / placed);

		if (under.IsEmpty())
			under = "none";

		RZ_Log.Info(string.Format("Places audit: %1 kinds, %2 places, %3 fail, score %4; %5 under %6: %7", m_aKinds.Count(), placed, placed - passed, score, underCount, s_iThreshold, under));
		return underCount;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Num(float f)
	{
		return RZ_Util.RoundToInt(f).ToString();
	}

	//------------------------------------------------------------------------------------------------
	protected static string YesNo(bool b)
	{
		if (b)
			return "1";

		return "0";
	}
}
