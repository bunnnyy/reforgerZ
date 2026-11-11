//! The season budget (docs/DESIGN.md, "The season budget"): how far anything that isn't the
//! built background simulation may move a faction's morale and strength, in lanes. Anything
//! clipped is logged and becomes words only. Saved with the season, wiped with it.
//!
//! | Lane                    | Morale                                   | Strength                          |
//! | Majors                  | +-20 per faction a season; +-15 a major; | +-8 per faction a season; +-5 a   |
//! |                         | +-10 a call                              | major; +-5 a call                 |
//! | Contracts and errands   | +-10 per faction a season day            | +-4 per faction a season day      |
//! | The finale              | +-10 per faction (never clipped by the   | +-5 per faction                   |
//! |                         | majors)                                  |                                   |
//! | The epilogue            | +-5 per faction, inside the majors lane  | -                                 |
//!
//! "+-20" is a band on the net change: the season's sum stays within it.
//! Also here: the seat ledger (contracts never take a leader below 30 loyalty; a major swings at
//! most two seats, a faction three, never a stronghold, never past one Rogue), one outpost through
//! the majors a season, research +3 per major, and the season weights (SetContractWeight, combined
//! and clamped to x0.8-1.2). The majors (phase 5) and the finale (phase 6) draw on their lanes;
//! contracts draw on theirs now.
//! Server only.

//------------------------------------------------------------------------------------------------
enum RZ_EBudgetLane
{
	MAJORS,
	CONTRACTS,
	FINALE,
	EPILOGUE
}

//------------------------------------------------------------------------------------------------
class RZ_MajorBudgetSave
{
	ref array<float> morale = {};		// lane * 4 + faction: the net used
	ref array<float> strength = {};
	int contractDay;					// the season day the contracts lane counts
	ref array<string> keys = {};		// per major and seat counters: "major.morale.<key>", "seat.<major>", ...
	ref array<float> values = {};
	ref array<string> weightKeys = {};	// the season weights
	ref array<float> weightValues = {};
}

//------------------------------------------------------------------------------------------------
class RZ_MajorBudget
{
	static const int LANES = 4;
	static const float WEIGHT_MIN = 0.8;
	static const float WEIGHT_MAX = 1.2;
	static const float RESEARCH_PER_MAJOR = 3;
	static const int SEATS_PER_MAJOR = 2;
	static const int SEATS_PER_FACTION = 3;

	protected RZ_SharedSettings m_Settings;
	protected ref array<float> m_aMorale = {};
	protected ref array<float> m_aStrength = {};
	protected int m_iContractDay;
	protected ref map<string, float> m_mUsed = new map<string, float>();
	protected ref map<string, float> m_mWeights = new map<string, float>();

	//------------------------------------------------------------------------------------------------
	void RZ_MajorBudget(RZ_SharedSettings settings)
	{
		m_Settings = settings;
		Pad();
	}

	//------------------------------------------------------------------------------------------------
	protected void Pad()
	{
		while (m_aMorale.Count() < LANES * RZ_Factions.COUNT)
		{
			m_aMorale.Insert(0);
		}

		while (m_aStrength.Count() < LANES * RZ_Factions.COUNT)
		{
			m_aStrength.Insert(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	static RZ_MajorBudget Get()
	{
		RZ_Shared shared = RZ_Shared.Get();
		if (!shared)
			return null;

		return shared.GetBudget();
	}

	//------------------------------------------------------------------------------------------------
	//! A new season day: the contracts lane starts again.
	void Tick()
	{
		int day = RZ_SeasonClock.Day();
		if (day == m_iContractDay)
			return;

		m_iContractDay = day;
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			m_aMorale[RZ_EBudgetLane.CONTRACTS * RZ_Factions.COUNT + f] = 0;
			m_aStrength[RZ_EBudgetLane.CONTRACTS * RZ_Factions.COUNT + f] = 0;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected int LaneMorale(RZ_EBudgetLane lane)
	{
		switch (lane)
		{
			case RZ_EBudgetLane.MAJORS: return m_Settings.m_iMajorMorale;
			case RZ_EBudgetLane.CONTRACTS: return m_Settings.m_iContractMorale;
			case RZ_EBudgetLane.FINALE: return m_Settings.m_iFinaleMorale;
			case RZ_EBudgetLane.EPILOGUE: return m_Settings.m_iEpilogueMorale;
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected int LaneStrength(RZ_EBudgetLane lane)
	{
		switch (lane)
		{
			case RZ_EBudgetLane.MAJORS: return m_Settings.m_iMajorStrength;
			case RZ_EBudgetLane.CONTRACTS: return m_Settings.m_iContractStrength;
			case RZ_EBudgetLane.FINALE: return m_Settings.m_iFinaleStrength;
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The part of `delta` that keeps `used + part` within +-cap.
	protected static float Fit(float used, float delta, float cap)
	{
		if (cap <= 0)
			return 0;

		float next = Math.Clamp(used + delta, -cap, cap);
		return next - used;
	}

	//------------------------------------------------------------------------------------------------
	//! Morale `delta` for `faction` from `lane`. major: the major's key in the majors and epilogue
	//! lanes (its own +-15), "" otherwise. Returns what may be applied; the caller applies it. The
	//! epilogue lane also draws on the majors lane. Clipped amounts are logged.
	float TakeMorale(RZ_EBudgetLane lane, int faction, float delta, string major = "", string why = "")
	{
		if (delta == 0 || !RZ_Factions.IsValid(faction))
			return 0;

		float allowed = delta;
		if (lane == RZ_EBudgetLane.MAJORS)
			allowed = Math.Clamp(allowed, -m_Settings.m_iPerCallMorale, m_Settings.m_iPerCallMorale);

		int idx = lane * RZ_Factions.COUNT + faction;
		allowed = Fit(m_aMorale[idx], allowed, LaneMorale(lane));
		int majorsIdx = RZ_EBudgetLane.MAJORS * RZ_Factions.COUNT + faction;
		if (lane == RZ_EBudgetLane.EPILOGUE)
			allowed = Fit(m_aMorale[majorsIdx], allowed, m_Settings.m_iMajorMorale);

		string majorKey = "major.morale." + major;
		if (!major.IsEmpty())
			allowed = Fit(Used(majorKey), allowed, m_Settings.m_iPerMajorMorale);

		m_aMorale[idx] = m_aMorale[idx] + allowed;
		if (lane == RZ_EBudgetLane.EPILOGUE)
			m_aMorale[majorsIdx] = m_aMorale[majorsIdx] + allowed;

		if (!major.IsEmpty())
			m_mUsed.Set(majorKey, Used(majorKey) + allowed);

		if (allowed != delta)
			RZ_Log.Info(string.Format("Budget: %1 morale %2 for %3 clipped to %4 (%5)", LaneName(lane), delta, RZ_Factions.GetName(faction), allowed, why));

		return allowed;
	}

	//------------------------------------------------------------------------------------------------
	//! Strength `delta` for `faction` from `lane`, as TakeMorale.
	float TakeStrength(RZ_EBudgetLane lane, int faction, float delta, string major = "", string why = "")
	{
		if (delta == 0 || !RZ_Factions.IsValid(faction))
			return 0;

		float allowed = delta;
		if (lane == RZ_EBudgetLane.MAJORS)
			allowed = Math.Clamp(allowed, -m_Settings.m_iPerCallStrength, m_Settings.m_iPerCallStrength);

		int idx = lane * RZ_Factions.COUNT + faction;
		allowed = Fit(m_aStrength[idx], allowed, LaneStrength(lane));
		string majorKey = "major.strength." + major;
		if (!major.IsEmpty())
			allowed = Fit(Used(majorKey), allowed, m_Settings.m_iPerMajorStrength);

		m_aStrength[idx] = m_aStrength[idx] + allowed;
		if (!major.IsEmpty())
			m_mUsed.Set(majorKey, Used(majorKey) + allowed);

		if (allowed != delta)
			RZ_Log.Info(string.Format("Budget: %1 strength %2 for %3 clipped to %4 (%5)", LaneName(lane), delta, RZ_Factions.GetName(faction), allowed, why));

		return allowed;
	}

	//------------------------------------------------------------------------------------------------
	//! The seat ledger for a contract's loyalty change: never a stronghold, never below the floor
	//! (Shared > Contract loyalty floor, 30). Returns the change to apply.
	int ContractLoyalty(int score, bool stronghold, int delta)
	{
		if (stronghold)
			return 0;

		if (delta >= 0)
			return delta;

		int minScore = m_Settings.m_iContractLoyaltyFloor;
		if (score <= minScore)
			return 0;

		return Math.Max(delta, minScore - score);
	}

	//------------------------------------------------------------------------------------------------
	//! A major swings or replaces a seat of `faction`: at most two per major and three per faction a
	//! season, never a stronghold. True (and counted) if it may.
	bool TakeSeat(string major, int faction, bool stronghold)
	{
		if (stronghold || !RZ_Factions.IsValid(faction))
			return false;

		string mk = "seat." + major;
		string fk = "seat.f" + faction.ToString();
		if (Used(mk) >= SEATS_PER_MAJOR || Used(fk) >= SEATS_PER_FACTION)
			return false;

		m_mUsed.Set(mk, Used(mk) + 1);
		m_mUsed.Set(fk, Used(fk) + 1);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The one outpost a season that may change hands through the majors. True (and used) if free.
	bool TakeOutpost(string major)
	{
		if (Used("outpost") >= 1)
			return false;

		m_mUsed.Set("outpost", 1);
		RZ_Log.Info("Budget: the season's outpost goes to " + major);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Research a major may add (+3 per major, inner lines included). Returns what may be added.
	float TakeResearch(string major, float amount)
	{
		string key = "research." + major;
		float room = Math.Max(0, RESEARCH_PER_MAJOR - Used(key));
		float allowed = Math.Clamp(amount, 0, room);
		m_mUsed.Set(key, Used(key) + allowed);
		return allowed;
	}

	//------------------------------------------------------------------------------------------------
	protected float Used(string key)
	{
		float v;
		m_mUsed.Find(key, v);
		return v;
	}

	//------------------------------------------------------------------------------------------------
	//! A season weight on a contract type (a major's multiplier); several combine into one value,
	//! clamped to x0.8-1.2.
	void SetContractWeight(string type, float mult)
	{
		float now = ContractWeight(type);
		m_mWeights.Set(type, Math.Clamp(now * mult, WEIGHT_MIN, WEIGHT_MAX));
	}

	//------------------------------------------------------------------------------------------------
	float ContractWeight(string type)
	{
		float w = 1;
		if (!m_mWeights.Find(type, w))
			return 1;

		return w;
	}

	//------------------------------------------------------------------------------------------------
	//! Net morale `faction` has used in `lane` (the contracts lane: this season day).
	float MoraleUsed(RZ_EBudgetLane lane, int faction)
	{
		return m_aMorale[lane * RZ_Factions.COUNT + faction];
	}

	//------------------------------------------------------------------------------------------------
	float StrengthUsed(RZ_EBudgetLane lane, int faction)
	{
		return m_aStrength[lane * RZ_Factions.COUNT + faction];
	}

	//------------------------------------------------------------------------------------------------
	static string LaneName(RZ_EBudgetLane lane)
	{
		switch (lane)
		{
			case RZ_EBudgetLane.MAJORS: return "majors";
			case RZ_EBudgetLane.CONTRACTS: return "contracts";
			case RZ_EBudgetLane.FINALE: return "finale";
			case RZ_EBudgetLane.EPILOGUE: return "epilogue";
		}

		return "?";
	}

	//------------------------------------------------------------------------------------------------
	RZ_MajorBudgetSave Save()
	{
		RZ_MajorBudgetSave data = new RZ_MajorBudgetSave();
		data.morale.Copy(m_aMorale);
		data.strength.Copy(m_aStrength);
		data.contractDay = m_iContractDay;
		foreach (string k, float v : m_mUsed)
		{
			data.keys.Insert(k);
			data.values.Insert(v);
		}

		foreach (string wk, float wv : m_mWeights)
		{
			data.weightKeys.Insert(wk);
			data.weightValues.Insert(wv);
		}

		return data;
	}

	//------------------------------------------------------------------------------------------------
	void Restore(RZ_MajorBudgetSave data)
	{
		if (!data)
			return;

		if (data.morale)
			m_aMorale.Copy(data.morale);

		if (data.strength)
			m_aStrength.Copy(data.strength);

		Pad();
		m_iContractDay = data.contractDay;
		if (data.keys && data.values)
		{
			for (int i = 0; i < data.keys.Count() && i < data.values.Count(); i++)
			{
				m_mUsed.Set(data.keys[i], data.values[i]);
			}
		}

		if (data.weightKeys && data.weightValues)
		{
			for (int w = 0; w < data.weightKeys.Count() && w < data.weightValues.Count(); w++)
			{
				m_mWeights.Set(data.weightKeys[w], data.weightValues[w]);
			}
		}
	}
}

//------------------------------------------------------------------------------------------------
class RZ_CalendarSave
{
	int evening;			// evenings counted (a game dusk with a player online)
	int usedEvening = -1;	// the evening the last set piece held
	string holder;			// the set piece holding the evening ("" = free)
	float holderX;
	float holderZ;
	ref array<string> waitKeys = {};
	ref array<int> waitCounts = {};
}

//------------------------------------------------------------------------------------------------
//! The set-piece calendar (docs/DESIGN.md, "The set-piece calendar"): the one calendar. An evening
//! is a game dusk window (18:30 to 20:30) with a player online. One set piece an evening,
//! server-wide: a set piece holds the evening from its offer hour until it resolves. One that falls
//! due on a held evening waits for the next free one, at most twice (Shared > Set piece waits),
//! then runs at the next free hour. Errands deal nothing within 600 m of a set piece on its
//! evening. The majors (phase 5) and the epilogue's pieces book it.
class RZ_SetPieceCalendar
{
	protected RZ_SharedSettings m_Settings;
	protected int m_iEvening;
	protected int m_iUsedEvening = -1;
	protected string m_sHolder;
	protected vector m_vHolder;
	protected bool m_bInDusk;
	protected ref map<string, int> m_mWaits = new map<string, int>();

	//------------------------------------------------------------------------------------------------
	void RZ_SetPieceCalendar(RZ_SharedSettings settings)
	{
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_SetPieceCalendar Get()
	{
		RZ_Shared shared = RZ_Shared.Get();
		if (!shared)
			return null;

		return shared.GetCalendar();
	}

	//------------------------------------------------------------------------------------------------
	//! Counts evenings: a new one each time the game clock enters dusk with someone online.
	void Tick(float dt, bool online)
	{
		bool dusk = RZ_WorldFacts.Get().IsDusk();
		if (dusk && !m_bInDusk && online)
			m_iEvening++;

		m_bInDusk = dusk;
	}

	//------------------------------------------------------------------------------------------------
	//! The current evening's number (the last dusk counted).
	int Evening()
	{
		return m_iEvening;
	}

	//------------------------------------------------------------------------------------------------
	//! Free for a set piece now: nothing holds the calendar, and no set piece has held this evening.
	bool IsFree()
	{
		return m_sHolder.IsEmpty() && m_iUsedEvening != m_iEvening;
	}

	//------------------------------------------------------------------------------------------------
	//! A set piece `key` at `pos` asks for the evening. True: it holds it until Release(key). False:
	//! it waits for the next free one (counted); after the waits, MustRun(key) says it runs anyway.
	bool Hold(string key, vector pos)
	{
		if (m_sHolder == key)
			return true;

		if (!IsFree())
		{
			int waited;
			m_mWaits.Find(key, waited);
			m_mWaits.Set(key, waited + 1);
			RZ_Log.Info(string.Format("Calendar: %1 waits (evening %2 held by %3)", key, m_iEvening, m_sHolder));
			return false;
		}

		m_sHolder = key;
		m_vHolder = pos;
		m_iUsedEvening = m_iEvening;
		m_mWaits.Remove(key);
		RZ_Log.Info(string.Format("Calendar: %1 holds evening %2", key, m_iEvening));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A set piece that has waited its evenings runs at the next free hour, held or not.
	bool MustRun(string key)
	{
		int waited;
		m_mWaits.Find(key, waited);
		return waited >= m_Settings.m_iSetPieceWaits;
	}

	//------------------------------------------------------------------------------------------------
	//! The set piece resolved: the calendar is free (this evening stays used).
	void Release(string key)
	{
		if (m_sHolder != key)
			return;

		m_sHolder = "";
		RZ_Log.Info(string.Format("Calendar: %1 released", key));
	}

	//------------------------------------------------------------------------------------------------
	string Holder()
	{
		return m_sHolder;
	}

	//------------------------------------------------------------------------------------------------
	//! Errands' quiet rule: true within the quiet radius of the set piece holding this evening.
	bool IsQuietAt(vector pos)
	{
		if (m_sHolder.IsEmpty())
			return false;

		return RZ_Util.DistXZ(pos, m_vHolder) <= m_Settings.m_fSetPieceQuietRadius;
	}

	//------------------------------------------------------------------------------------------------
	RZ_CalendarSave Save()
	{
		RZ_CalendarSave data = new RZ_CalendarSave();
		data.evening = m_iEvening;
		data.usedEvening = m_iUsedEvening;
		data.holder = m_sHolder;
		data.holderX = m_vHolder[0];
		data.holderZ = m_vHolder[2];
		foreach (string k, int n : m_mWaits)
		{
			data.waitKeys.Insert(k);
			data.waitCounts.Insert(n);
		}

		return data;
	}

	//------------------------------------------------------------------------------------------------
	void Restore(RZ_CalendarSave data)
	{
		if (!data)
			return;

		m_iEvening = data.evening;
		m_iUsedEvening = data.usedEvening;
		m_sHolder = data.holder;
		m_vHolder = Vector(data.holderX, 0, data.holderZ);
		if (data.waitKeys && data.waitCounts)
		{
			for (int i = 0; i < data.waitKeys.Count() && i < data.waitCounts.Count(); i++)
			{
				m_mWaits.Set(data.waitKeys[i], data.waitCounts[i]);
			}
		}
	}
}
