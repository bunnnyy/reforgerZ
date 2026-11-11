//! Player traces (docs/TRACES.md, "Your own traces"): what players leave behind, after the fact.
//!
//! | P1  | A town you emptied: ticked bears and ring-and-dot chalk (door marks, RZ_TraceWriters)     |
//! | P2  | Ash where the bodies were burned: a black patch and a count on a post, 2 season days     |
//! | P3  | A grave: two fence palings, a mound and a name or a description, 2 season days           |
//! | P3b | The memorial candles: a jar on every safehouse's step when a Haven Champion dies, 1 day   |
//! | P4  | A candle near where you died: chalk on the nearest door, when nobody dug a grave         |
//! | P5  | The town log made physical: the season mark's look by its kind (RZ_WorldMemory)          |
//!
//! A grave needs a notable death (a life of 2 hours or more, or a death at a running job) and
//! someone to dig: a Haven house with people within 3 km. One per identity, 8 on the server. Names
//! follow Zombies > Tribes > Your dead run: the description by default, the name only on Named,
//! with the player's opt-out and the name filter. Nothing within 2 km of the Long Table's farm.
//! Props go through the marks spawner's Traces pool (text only without a prefab); the list is
//! saved so graves and ash come back after a restart. Server only.

//------------------------------------------------------------------------------------------------
class RZ_PlayerTrace
{
	string kind;		// "grave", "pyre"
	string ident;		// a grave's identity (one each)
	float x;
	float z;
	float until;		// season days
	string text;
}

//------------------------------------------------------------------------------------------------
class RZ_PlayerTraces
{
	protected static const int MAX_GRAVES = 8;
	protected static const int MAX_TRACES = 16;
	protected static const float NOTABLE_LIFE = 7200;
	protected static const float DIG_RADIUS = 3000;

	protected RZ_GameModeComponent m_GM;
	protected RZ_Traces m_Traces;
	protected ref array<ref RZ_PlayerTrace> m_aTraces = {};

	//------------------------------------------------------------------------------------------------
	void RZ_PlayerTraces(RZ_GameModeComponent gm, RZ_Traces traces)
	{
		m_GM = gm;
		m_Traces = traces;
	}

	//------------------------------------------------------------------------------------------------
	int Count()
	{
		return m_aTraces.Count();
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsOn()
	{
		return m_Traces.IsOn() && m_Traces.GetSettings().m_bPlayerTraces && !RZ_WorldFacts.Get().IsCured();
	}

	//------------------------------------------------------------------------------------------------
	//! The Long Table's takings never reach the graves or the candles.
	protected bool LongTableGround(vector pos)
	{
		RZ_LongTable lt = m_GM.GetLongTable();
		return lt && lt.Exists() && lt.IsNearHouse(pos, 2000);
	}

	//------------------------------------------------------------------------------------------------
	//! A player's character died at pos (RZ_TraceWriters.Death): the memorial candles for a Haven
	//! Champion, then a grave if the death was notable and someone is near to dig, else a candle on
	//! the nearest door in a town.
	void OnDeath(int playerId, vector pos)
	{
		if (!IsOn() || pos == vector.Zero || LongTableGround(pos) || RZ_Perimeter.IsNearLineAt(pos))
			return;

		if (RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, RZ_EFaction.SURVIVORS), RZ_ETier.CHAMPION))
			Candles();

		if (!IsNotable(playerId, pos))
			return;

		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		string ident = "";
		if (profile)
			ident = profile.m_sIdentity;

		string described = Described(playerId);
		string who = described;
		bool named = false;
		RZ_Tribes tribes = m_GM.GetTribes();
		if (tribes)
		{
			string name = RZ_Util.PlayerName(playerId);
			who = tribes.GetPeople().PublicName(playerId, name, described);
			named = who != described;
		}

		if (named)
		{
			RZ_Background bg = m_GM.GetPlayerBackground(playerId);
			string what = "";
			if (bg)
				what = ". " + bg.m_sName;

			who = who + what;
			who.ToUpper();
		}

		int day = RZ_WorldFacts.Get().Day();
		int town = RZ_TraceWriters.TownAt(pos);
		if (HasDigger(pos))
		{
			Grave(ident, pos, RZ_TraceText.Grave(who, named, day));
			if (town >= 0)
				m_Traces.History().Add(town, "death", -1, 1, pos);

			return;
		}

		// P4: no grave dug; in a town, a candle on the nearest door (a description, never a name)
		if (town < 0)
			return;

		RZ_DoorMark candle = RZ_DoorMark.Make(RZ_EDoorSymbol.CANDLE, day, 1, 0, 0, RZ_DoorMark.F_SHOWN);
		candle.m_sText = described;
		m_Traces.History().Add(town, "death", -1, 1, pos);
		m_Traces.GetDoors().Write(town, RZ_EDoorWant.NEAR, pos, candle, "a death nearby");
	}

	//------------------------------------------------------------------------------------------------
	//! "someone who'd been a mechanic"
	protected string Described(int playerId)
	{
		RZ_Background bg = m_GM.GetPlayerBackground(playerId);
		string what = "a traveller";
		if (bg && !bg.m_sName.IsEmpty())
		{
			what = bg.m_sName;
			what.ToLower();
			what = "a " + what;
		}

		return "someone who'd been " + what;
	}

	//------------------------------------------------------------------------------------------------
	//! A life of 2 hours or more, or a death at a job that was running there.
	protected bool IsNotable(int playerId, vector pos)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		if (profile && profile.m_fLifeSeconds >= NOTABLE_LIFE)
			return true;

		RZ_MissionManager missions = m_GM.GetMissions();
		if (!missions)
			return false;

		foreach (RZ_Mission m : missions.GetActive())
		{
			if (m && !m.m_bGlobal && RZ_Util.DistXZ(m.m_vPos, pos) <= m.m_fParticipationRadius)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Someone to dig: a Haven house with people within 3 km.
	protected bool HasDigger(vector pos)
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses)
			return false;

		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			if (h && h.m_iPopulation > 0 && RZ_Util.DistXZ(h.m_vPos, pos) <= DIG_RADIUS)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! P3: at the place of death if it's 100 m from another grave and 150 m from a safehouse; else
	//! on the verge of the nearest road 150-300 m on; one per identity, 8 on the server.
	protected void Grave(string ident, vector pos, string text)
	{
		for (int i = m_aTraces.Count() - 1; i >= 0; i--)
		{
			if (m_aTraces[i].kind == "grave" && !ident.IsEmpty() && m_aTraces[i].ident == ident)
				Drop(i);
		}

		while (CountKind("grave") >= MAX_GRAVES)
		{
			Drop(OldestOf("grave"));
		}

		vector at = pos;
		if (!GraveFits(at))
		{
			at = RZ_Marks.Verge(RZ_Mission.LandPointNear(pos, 150, 300), 6, 10);
			if (!GraveFits(at))
				at = pos;
		}

		Add("grave", ident, at, 2, text);
	}

	//------------------------------------------------------------------------------------------------
	protected bool GraveFits(vector at)
	{
		foreach (RZ_PlayerTrace t : m_aTraces)
		{
			if (t.kind == "grave" && RZ_Util.DistXZ(Vector(t.x, 0, t.z), at) < 100)
				return false;
		}

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses && houses.FindNearest(at, 150) >= 0)
			return false;

		return !RZ_Util.IsWater(at) && !RZ_Perimeter.IsNearLineAt(at);
	}

	//------------------------------------------------------------------------------------------------
	//! P2: ash where a job burned the bodies, with the count chalked on a post (seeded by the job).
	void Pyre(vector pos, int jobId)
	{
		if (!IsOn() || LongTableGround(pos))
			return;

		RZ_Rng rng = new RZ_Rng(RZ_WorldFacts.Get().GetSeed() + jobId * 31);
		Add("pyre", "", RZ_Mission.LandPointNear(pos, 2, 8), 2, RZ_TraceText.Pyre(rng.Int(4, 15), RZ_WorldFacts.Get().Day()));
	}

	//------------------------------------------------------------------------------------------------
	//! P3b: a Haven Champion died: candles on every safehouse's step for a season day.
	void Candles()
	{
		// S6: one stroke of the church bell at 18:30, and the church's jars that evening
		m_Traces.GetAmbience().RingBell();
		RZ_Safehouses houses = m_GM.GetSafehouses();
		RZ_Marks marks = RZ_Marks.Get();
		if (!houses || !marks)
			return;

		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			if (!h || h.m_iPopulation <= 0)
				continue;

			marks.Place(RZ_EMarkPool.TRACES, "traces.candles." + i.ToString(), RZ_Mission.LandPointNear(h.m_vPos, 3, 6), m_Traces.GetKit().m_sCandleJars, RZ_TraceText.Candles(), 3, 1, false, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Add(string kind, string ident, vector at, float days, string text)
	{
		while (m_aTraces.Count() >= MAX_TRACES)
		{
			Drop(0);
		}

		RZ_PlayerTrace t = new RZ_PlayerTrace();
		t.kind = kind;
		t.ident = ident;
		t.x = at[0];
		t.z = at[2];
		t.until = RZ_SeasonClock.Days() + days;
		t.text = text;
		m_aTraces.Insert(t);
		Show(t);
		RZ_Log.Info(string.Format("Traces: scene %1 at %2 (a player's)", kind, RZ_SettlementScanner.GridReference(at)));
	}

	//------------------------------------------------------------------------------------------------
	protected string Key(RZ_PlayerTrace t)
	{
		return string.Format("traces.%1.%2.%3", t.kind, RZ_Util.RoundToInt(t.x), RZ_Util.RoundToInt(t.z));
	}

	//------------------------------------------------------------------------------------------------
	//! Into the marks spawner: its prop near players (text only without one), an approach line.
	protected void Show(RZ_PlayerTrace t)
	{
		RZ_Marks marks = RZ_Marks.Get();
		if (!marks)
			return;

		ResourceName prefab = m_Traces.GetKit().m_sGrave;
		if (t.kind == "pyre")
			prefab = m_Traces.GetKit().m_sPyre;

		float days = Math.Max(0.01, t.until - RZ_SeasonClock.Days());
		marks.Place(RZ_EMarkPool.TRACES, Key(t), RZ_Util.GroundPos(Vector(t.x, 0, t.z)), prefab, t.text, 3, days, false, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void Drop(int i)
	{
		if (i < 0 || i >= m_aTraces.Count())
			return;

		RZ_Marks marks = RZ_Marks.Get();
		if (marks)
			marks.Remove(Key(m_aTraces[i]));

		RZ_Log.Info(string.Format("Traces: scene %1 gone (made room)", m_aTraces[i].kind));
		m_aTraces.RemoveOrdered(i);
	}

	//------------------------------------------------------------------------------------------------
	protected int CountKind(string kind)
	{
		int n = 0;
		foreach (RZ_PlayerTrace t : m_aTraces)
		{
			if (t.kind == kind)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected int OldestOf(string kind)
	{
		foreach (int i, RZ_PlayerTrace t : m_aTraces)
		{
			if (t.kind == kind)
				return i;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! The cure's handover: the in-season graves come down (the epilogue places its own from the
	//! record). How many.
	int DropGraves()
	{
		int n = 0;
		for (int i = m_aTraces.Count() - 1; i >= 0; i--)
		{
			if (m_aTraces[i].kind != "grave")
				continue;

			RZ_Marks marks = RZ_Marks.Get();
			if (marks)
				marks.Remove(Key(m_aTraces[i]));

			RZ_Log.Info("Traces: scene grave gone (the epilogue's now)");
			m_aTraces.RemoveOrdered(i);
			n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Every minute: traces past their day go (the spawner takes their props down by itself).
	void Prune()
	{
		float today = RZ_SeasonClock.Days();
		for (int i = m_aTraces.Count() - 1; i >= 0; i--)
		{
			if (today > m_aTraces[i].until)
			{
				RZ_Log.Info(string.Format("Traces: scene %1 gone (its days are up)", m_aTraces[i].kind));
				m_aTraces.RemoveOrdered(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void Save(notnull array<ref RZ_PlayerTrace> result)
	{
		foreach (RZ_PlayerTrace t : m_aTraces)
		{
			result.Insert(t);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! After a restart: the traces still in their days go back into the spawner.
	void Restore(array<ref RZ_PlayerTrace> saved)
	{
		if (!saved)
			return;

		foreach (RZ_PlayerTrace t : saved)
		{
			if (t && t.until > RZ_SeasonClock.Days())
			{
				m_aTraces.Insert(t);
				Show(t);
			}
		}
	}
}
