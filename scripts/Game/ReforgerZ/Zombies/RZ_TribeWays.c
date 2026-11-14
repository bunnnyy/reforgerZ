//! What the tribes remember and who they are (docs/INFECTED.md, "Memory" and "Who they are"): fear
//! places and getting used to them, habits, marks per character, worn trails, bonds, the alpha's
//! name, kind and echo, histories in the headlines, looks that mix, and the lines a Hunter, a
//! Soldier or a Police Officer reads from them. Owned by RZ_Tribes. Server only.
class RZ_TribeWays
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_Tribes m_Tribes;		// weak: the owner
	protected ref array<vector> m_aShotPos = {};
	protected ref array<float> m_aShotAt = {};			// the tribe clock
	protected ref array<IEntity> m_aMixed = {};
	protected ref map<int, float> m_mEchoToldAt = new map<int, float>();
	protected ref map<int, float> m_mBondToldAt = new map<int, float>();
	protected ref array<ResourceName> m_aMix = {};

	static const float FEAR_RADIUS = 125;
	static const int MAX_FEAR = 4;
	static const int MAX_HABITS = 3;
	static const int MAX_MARKS = 4;

	//------------------------------------------------------------------------------------------------
	void RZ_TribeWays(RZ_GameModeComponent gm, RZ_Tribes tribes)
	{
		m_GM = gm;
		m_Tribes = tribes;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsOn()
	{
		return m_Tribes.GetSettings().m_bLifeCycle;
	}

	// ================================================================ Who the alpha was
	//------------------------------------------------------------------------------------------------
	//! A new alpha's name and who they were (its echo): the column's leaders are the column's; a
	//! military tribe's mostly soldiers; a farm valley's mostly farmers; else anyone.
	void NameAlpha(RZ_Tribe t)
	{
		RZ_Rng rng = new RZ_Rng(t.m_iSeed + RZ_Util.RoundToInt(RZ_SeasonClock.Days() * 97) + t.m_aTaken.Count() * 13);
		t.m_sAlphaName = RZ_NoteText.FullName(rng);
		RZ_EAlphaKind kind = RZ_EAlphaKind.RESIDENT;
		float roll = Math.RandomFloat01();
		if (t.m_bColumn)
			kind = RZ_EAlphaKind.COLUMN;
		else if (t.m_eLook == RZ_ELook.MILITARY && roll < 0.6)
			kind = RZ_EAlphaKind.SOLDIER;
		else if (t.m_eLook == RZ_ELook.WORK && roll < 0.5)
			kind = RZ_EAlphaKind.FARMER;
		else
		{
			array<int> kinds = {RZ_EAlphaKind.RESIDENT, RZ_EAlphaKind.FARMER, RZ_EAlphaKind.DRIVER, RZ_EAlphaKind.RUNNER, RZ_EAlphaKind.SOLDIER};
			array<float> weights = {30, 20, 20, 15, 15};
			float r = Math.RandomFloat(0, 100);
			foreach (int i, int k : kinds)
			{
				r -= weights[i];
				if (r <= 0)
				{
					kind = k;
					break;
				}
			}
		}

		t.m_eAlphaKind = kind;
		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(t.m_vLair));
		string town = "the roads";
		if (s)
			town = s.m_sName;

		t.m_sAlphaWho = string.Format("%1 from %2", AlphaKindName(kind), town);
		if (kind == RZ_EAlphaKind.COLUMN)
			t.m_sAlphaWho = AlphaKindName(kind);
	}

	//------------------------------------------------------------------------------------------------
	static string AlphaKindName(RZ_EAlphaKind kind)
	{
		switch (kind)
		{
			case RZ_EAlphaKind.SOLDIER: return "a soldier";
			case RZ_EAlphaKind.FARMER: return "a farmer";
			case RZ_EAlphaKind.DRIVER: return "a driver";
			case RZ_EAlphaKind.RUNNER: return "a runner";
			case RZ_EAlphaKind.COLUMN: return "a driver from the Day 2 column";
		}

		return "someone who lived there";
	}

	//------------------------------------------------------------------------------------------------
	//! A name, not a description ("Marek the runner", not "someone who'd been a hunter").
	static bool IsAName(string who)
	{
		if (who.IsEmpty())
			return false;

		string low = who;
		low.ToLower();
		return !low.StartsWith("someone") && !low.StartsWith("a ") && !low.StartsWith("an ") && !low.StartsWith("the ");
	}

	//------------------------------------------------------------------------------------------------
	//! A daughter's call: the mother's, one note different.
	static string OneNoteDifferent(string pattern)
	{
		if (pattern.Contains("one note"))
			return pattern;

		return pattern + ", one note higher";
	}

	//------------------------------------------------------------------------------------------------
	//! A headline (never broadcast: players see the latest when they join), always with the place.
	void Headline(RZ_Tribe t, string text)
	{
		RZ_Leaders leaders = m_GM.GetLeaders();
		if (leaders)
			leaders.AddHeadline(text, false);
	}

	//------------------------------------------------------------------------------------------------
	//! The alpha's last events for the admin dump: "took Marek the runner; won the bridge off ...".
	static string HistoryText(RZ_Tribe t)
	{
		string text = "";
		foreach (string h : t.m_aHistory)
		{
			if (!text.IsEmpty())
				text += "; ";

			text += h;
		}

		return text;
	}

	// ================================================================ Looks
	//------------------------------------------------------------------------------------------------
	//! Where it grew up sets its look: military near a Bastion holding or a checkpoint; dust-grey at a
	//! quarry; work clothes at a barn, silo or mill; outdoor in the woods; else civilian.
	RZ_ELook LookFor(RZ_Tribe t)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			foreach (RZ_Holding h : territory.GetHoldings())
			{
				if (h.m_eFaction == RZ_EFaction.MILITARY && RZ_Util.DistXZ(h.m_vPos, t.m_vLair) < 1500)
					return RZ_ELook.MILITARY;
			}
		}

		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		if (perimeter && perimeter.IsActive())
		{
			array<vector> checkpoints = {};
			perimeter.GetCheckpoints(checkpoints);
			foreach (vector c : checkpoints)
			{
				if (RZ_Util.DistXZ(c, t.m_vLair) < 1500)
					return RZ_ELook.MILITARY;
			}
		}

		switch (t.m_sLairKind)
		{
			case "quarry": return RZ_ELook.DUST;
			case "barn": return RZ_ELook.WORK;
			case "silo": return RZ_ELook.WORK;
			case "mill": return RZ_ELook.WORK;
			case "woods": return RZ_ELook.OUTDOOR;
		}

		return RZ_ELook.CIVILIAN;
	}

	//------------------------------------------------------------------------------------------------
	//! Someone taken who was a soldier makes the tribe's look a little more military (its young keep
	//! their own clothes): the share of its members who were.
	void OnJoined(RZ_Tribe t, bool soldier)
	{
		float n = Math.Max(1, t.Members());
		float s = t.m_fMilitaryShare * n;
		if (soldier)
			s += 1;

		t.m_fMilitaryShare = Math.Clamp(s / (n + 1), 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! The prefabs for a group of hunters, scouts or young: mostly its look's list (if set), with its
	//! soldiers in military clothes (Military prefabs, if set), else the base list. A mixed list of
	//! ten, from which each member is picked.
	array<ResourceName> MixFor(RZ_Tribe t, array<ResourceName> base)
	{
		RZ_TribeSettings ts = m_Tribes.GetSettings();
		array<ResourceName> look = LookList(t.m_eLook);
		bool hasLook = look && !look.IsEmpty();
		bool hasMil = ts.m_aMilitaryPrefabs && !ts.m_aMilitaryPrefabs.IsEmpty();
		if (!hasLook && (!hasMil || t.m_fMilitaryShare <= 0))
			return base;

		m_aMix.Clear();
		for (int i = 0; i < 10; i++)
		{
			if (hasMil && Math.RandomFloat01() < t.m_fMilitaryShare)
				m_aMix.Insert(ts.m_aMilitaryPrefabs.GetRandomElement());
			else if (hasLook && Math.RandomFloat01() < 0.7)
				m_aMix.Insert(look.GetRandomElement());
			else if (base && !base.IsEmpty())
				m_aMix.Insert(base.GetRandomElement());
		}

		if (m_aMix.IsEmpty())
			return base;

		return m_aMix;
	}

	//------------------------------------------------------------------------------------------------
	protected array<ResourceName> LookList(RZ_ELook look)
	{
		RZ_TribeSettings ts = m_Tribes.GetSettings();
		switch (look)
		{
			case RZ_ELook.MILITARY: return ts.m_aMilitaryPrefabs;
			case RZ_ELook.WORK: return ts.m_aWorkPrefabs;
			case RZ_ELook.DUST: return ts.m_aDustPrefabs;
			case RZ_ELook.OUTDOOR: return ts.m_aOutdoorPrefabs;
		}

		return null;
	}

	// ================================================================ Fear places (rule 20)
	//------------------------------------------------------------------------------------------------
	//! A member died at night: remembered for fear places and habits (not silently).
	void OnDiedAtNight(RZ_Tribe t, vector at)
	{
		t.m_aDeadTonight.Insert(at);
		while (t.m_aDeadTonight.Count() > 40)
		{
			t.m_aDeadTonight.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Its trails and day walks go round its fear places (and its clan's), except a nomad, which has
	//! none.
	bool FearAvoids(RZ_Tribe t, vector pos)
	{
		// A frenzy night ignores fear places (its trails are worked out again at dusk and at dawn)
		if (!t || !IsOn() || t.m_fFrenzy > 0)
			return false;

		foreach (RZ_FearPlace f : t.m_aFear)
		{
			if (RZ_Util.DistXZ(f.m_vPos, pos) <= FEAR_RADIUS + 25)
				return true;
		}

		if (t.m_iClan <= 0)
			return false;

		foreach (RZ_Tribe c : m_Tribes.GetTribeList())
		{
			if (c == t || c.m_iClan != t.m_iClan)
				continue;

			foreach (RZ_FearPlace cf : c.m_aFear)
			{
				if (RZ_Util.DistXZ(cf.m_vPos, pos) <= FEAR_RADIUS + 25)
					return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A counted night: 8 or more of a tribe killed within 300 m make a fear place 250 m across, for
	//! 3 counted nights (5 cautious or cunning, 1 bold; nomads make none, 4 at most). Kills inside one
	//! don't extend it; once it's over, no new one within 500 m for 3 counted nights.
	protected bool CountFear(RZ_Tribe t, int night)
	{
		bool changed = false;
		for (int i = t.m_aFear.Count() - 1; i >= 0; i--)
		{
			if (night < t.m_aFear[i].m_iUntil)
				continue;

			RZ_FearPlace gone = t.m_aFear[i];
			gone.m_iUntil = night + 3;
			t.m_aFearGone.Insert(gone);
			t.m_aFear.Remove(i);
			changed = true;
		}

		for (int g = t.m_aFearGone.Count() - 1; g >= 0; g--)
		{
			if (night >= t.m_aFearGone[g].m_iUntil)
				t.m_aFearGone.Remove(g);
		}

		if (t.m_eTemper == RZ_ETemperament.NOMADIC || t.m_aDeadTonight.Count() < 8)
			return changed;

		foreach (vector at : t.m_aDeadTonight)
		{
			vector sum = vector.Zero;
			int n = 0;
			foreach (vector other : t.m_aDeadTonight)
			{
				if (RZ_Util.DistXZ(at, other) <= 300)
				{
					sum = sum + other;
					n++;
				}
			}

			if (n < 8)
				continue;

			float fn = n;
			vector centre = RZ_Util.GroundPos(sum * (1 / fn));
			if (Near(t.m_aFear, centre, FEAR_RADIUS) || Near(t.m_aFearGone, centre, 500) || t.m_aFear.Count() >= MAX_FEAR)
				return changed;

			int nights = 3;
			if (t.m_eTemper == RZ_ETemperament.CAUTIOUS || t.m_eTemper == RZ_ETemperament.CUNNING)
				nights = 5;
			else if (t.m_eTemper == RZ_ETemperament.BOLD)
				nights = 1;

			RZ_FearPlace f = new RZ_FearPlace();
			f.m_vPos = centre;
			f.m_iUntil = night + nights;
			t.m_aFear.Insert(f);
			RZ_Log.Info(string.Format("Tribes: %1 fears %2 for %3 counted nights (%4 of it died there).", t.m_sPlace, RZ_SettlementScanner.GridReference(centre), nights, n));
			return true;
		}

		return changed;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Near(array<ref RZ_FearPlace> list, vector pos, float radius)
	{
		foreach (RZ_FearPlace f : list)
		{
			if (RZ_Util.DistXZ(f.m_vPos, pos) <= radius)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! What a Hunter reads at a fear place, or where one has just worn off. "" if neither.
	string FearLine(vector pos)
	{
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (Near(t.m_aFear, pos, FEAR_RADIUS))
				return "Old tracks, all of them going round this place, none through it. They don't come through here any more.";

			if (Near(t.m_aFearGone, pos, FEAR_RADIUS))
				return "Fresh tracks through here again, where there were none. They're coming through here again.";
		}

		return "";
	}

	// ================================================================ Habits (rule 20)
	//------------------------------------------------------------------------------------------------
	//! A player fired (the game mode): remembered 5 minutes, for the shots habit.
	void OnShot(vector at)
	{
		float now = m_Tribes.GetClock();
		m_aShotPos.Insert(at);
		m_aShotAt.Insert(now);
		while (m_aShotPos.Count() > 24 || (!m_aShotAt.IsEmpty() && now - m_aShotAt[0] > 300))
		{
			m_aShotPos.RemoveOrdered(0);
			m_aShotAt.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool ShotNear(vector at, float radius)
	{
		float now = m_Tribes.GetClock();
		foreach (int i, vector p : m_aShotPos)
		{
			if (now - m_aShotAt[i] <= 300 && RZ_Util.DistXZ(p, at) <= radius)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Someone taken teaches the tribe where: a crossing within 150 m, the wire of a lived-in place
	//! within 400 m (the back door), shots within 300 m in the last 5 minutes, or a loud noise within
	//! 500 m in the last 10 (the bells). A driver on a road teaches the road hour.
	void OnTaken(RZ_Tribe t, vector at, string who)
	{
		if (!IsOn() || t.IsRover())
			return;

		RZ_Land land = m_GM.GetLand();
		for (int c = 0; c < land.GetCrossingCount(); c++)
		{
			if (RZ_Util.DistXZ(land.GetCrossing(c), at) <= 150)
			{
				LearnHabit(t, RZ_EHabit.CROSSING, land.GetCrossing(c));
				break;
			}
		}

		vector edge;
		if (land.NearestNoMansLand(at, edge) && RZ_Util.DistXZ(edge, at) <= 400)
			LearnHabit(t, RZ_EHabit.BACKDOOR, at);

		if (ShotNear(at, 300))
		{
			LearnHabit(t, RZ_EHabit.SHOTS, at);
		}
		else
		{
			vector noise;
			if (m_GM.GetHeat().FindRecentNoise(at, 500, 600, noise))
				LearnHabit(t, RZ_EHabit.BELLS, at);
		}

		string low = who;
		low.ToLower();
		int idx = land.CellAt(at);
		if (low.Contains("driver") && idx >= 0 && land.GetRoadWidth(idx) > 0)
			LearnHabit(t, RZ_EHabit.ROAD, at);

		// Bonds: two taken at one kill site within 10 minutes
		float now = m_Tribes.GetClock();
		if (now - t.m_fLastTakenAt < 600 && RZ_Util.DistXZ(at, t.m_vLastTaken) < 100)
			t.m_iBonds = Math.Min(3, t.m_iBonds + 1);

		t.m_fLastTakenAt = now;
		t.m_vLastTaken = at;
	}

	//------------------------------------------------------------------------------------------------
	//! A habit grows 40 (x1.5 cunning) for each person taken there (3 a tribe; the weakest goes).
	void LearnHabit(RZ_Tribe t, RZ_EHabit kind, vector at)
	{
		float add = 40;
		if (t.m_eTemper == RZ_ETemperament.CUNNING)
			add = 60;

		foreach (RZ_TribeHabit h : t.m_aHabits)
		{
			if (h.m_eKind == kind && RZ_Util.DistXZ(h.m_vPos, at) <= 400)
			{
				h.m_fStrength = Math.Min(100, h.m_fStrength + add);
				h.m_fHour = RZ_GameTime.GetHour();
				h.m_iPreyNight = m_Tribes.CountedNights();
				return;
			}
		}

		if (t.m_aHabits.Count() >= MAX_HABITS)
		{
			int weakest = 0;
			foreach (int i, RZ_TribeHabit w : t.m_aHabits)
			{
				if (w.m_fStrength < t.m_aHabits[weakest].m_fStrength)
					weakest = i;
			}

			t.m_aHabits.Remove(weakest);
		}

		RZ_TribeHabit nh = new RZ_TribeHabit();
		nh.m_eKind = kind;
		nh.m_vPos = at;
		nh.m_fHour = RZ_GameTime.GetHour();
		nh.m_fStrength = add;
		nh.m_iPreyNight = m_Tribes.CountedNights();
		t.m_aHabits.Insert(nh);
		RZ_Log.Info(string.Format("Tribes: %1 learns a habit: %2 at %3.", t.m_sPlace, typename.EnumToString(RZ_EHabit, kind), RZ_SettlementScanner.GridReference(at)));
	}

	//------------------------------------------------------------------------------------------------
	//! A counted night: every habit falls 10, and 30 more where 2 or more of the tribe died within
	//! 300 m of it; gone under 10.
	protected void CountHabits(RZ_Tribe t)
	{
		for (int i = t.m_aHabits.Count() - 1; i >= 0; i--)
		{
			RZ_TribeHabit h = t.m_aHabits[i];
			h.m_fStrength -= 10;
			int dead = 0;
			foreach (vector at : t.m_aDeadTonight)
			{
				if (RZ_Util.DistXZ(at, h.m_vPos) <= 300)
					dead++;
			}

			if (dead >= 2)
				h.m_fStrength -= 30;

			if (h.m_fStrength < 10)
			{
				RZ_Log.Info(string.Format("Tribes: %1 loses its habit at %2.", t.m_sPlace, RZ_SettlementScanner.GridReference(h.m_vPos)));
				t.m_aHabits.Remove(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The data hearing's reach: x1.5 with the shots or the bells habit, and for a bold tribe's day
	//! party.
	float HearingMult(RZ_Tribe t, RZ_TribePart p)
	{
		if (!IsOn())
			return 1;

		if (t.StrongHabit(RZ_EHabit.SHOTS) || t.StrongHabit(RZ_EHabit.BELLS))
			return 1.5;

		if (t.m_eTemper == RZ_ETemperament.BOLD && p.m_eKind == RZ_ETribePart.DAY)
			return 1.5;

		return 1;
	}

	// ================================================================ Marks (rule 20)
	//------------------------------------------------------------------------------------------------
	//! The tribe marks a player's character: it killed its alpha or a guard, or burned its lair.
	//! Strength 100, 4 a tribe; a new life is a new character. They're told once.
	void Mark(RZ_Tribe t, int playerId)
	{
		if (!IsOn() || t.IsRover() || playerId <= 0)
			return;

		int life = LifeOf(playerId);
		foreach (RZ_TribeMark known : t.m_aMarks)
		{
			if (known.m_iPlayer == playerId && known.m_iLife == life)
			{
				known.m_fStrength = 100;
				return;
			}
		}

		if (t.m_aMarks.Count() >= MAX_MARKS)
		{
			int weakest = 0;
			foreach (int i, RZ_TribeMark w : t.m_aMarks)
			{
				if (w.m_fStrength < t.m_aMarks[weakest].m_fStrength)
					weakest = i;
			}

			t.m_aMarks.Remove(weakest);
		}

		RZ_TribeMark m = new RZ_TribeMark();
		m.m_iPlayer = playerId;
		m.m_iLife = life;
		m.m_fStrength = 100;
		t.m_aMarks.Insert(m);
		m_GM.NotifyPlayer(playerId, "Marked", string.Format("They know you at %1 now.", t.m_sPlace));
		RZ_Log.Info(string.Format("Tribes: %1 marks player %2.", t.m_sPlace, playerId));
	}

	//------------------------------------------------------------------------------------------------
	protected int LifeOf(int playerId)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		if (!profile)
			return 0;

		return profile.m_iCharacter;
	}

	//------------------------------------------------------------------------------------------------
	//! This tribe knows this player's character, and they're inside its range.
	bool Knows(RZ_Tribe t, int playerId, vector at)
	{
		if (!IsOn() || t.m_aMarks.IsEmpty() || RZ_Util.DistXZ(at, t.RangeCentre()) > t.m_fRadius)
			return false;

		int life = LifeOf(playerId);
		foreach (RZ_TribeMark m : t.m_aMarks)
		{
			if (m.m_iPlayer == playerId && m.m_iLife == life && m.m_fStrength > 0)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A member died near a player: an alpha or a guard marks the nearest player within 100 m.
	void OnKilledNear(RZ_Tribe t, RZ_EInfectedRole role, vector at)
	{
		if (role != RZ_EInfectedRole.ALPHA && role != RZ_EInfectedRole.GUARD)
			return;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		int best = -1;
		float bestDist = 100;
		foreach (int i, IEntity p : players)
		{
			float d = RZ_Util.DistXZ(p.GetOrigin(), at);
			if (d <= bestDist)
			{
				bestDist = d;
				best = ids[i];
			}
		}

		if (best > 0)
			Mark(t, best);
	}

	// ================================================================ Worn trails and the night
	//------------------------------------------------------------------------------------------------
	//! A counted night: each trail walked tonight wears (+1, 10 at most), the others fade (-1);
	//! fear places, habits and marks (-25) are counted. Returns true if a tribe's trails need
	//! working out again (a fear place came or went).
	void OnCountedNight()
	{
		int night = m_Tribes.CountedNights();
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			while (t.m_aWear.Count() < t.m_aTrails.Count())
			{
				t.m_aWear.Insert(0);
			}

			for (int i = 0; i < t.m_aWear.Count(); i++)
			{
				if (t.m_aTrailsTonight.Find(i) >= 0)
					t.m_aWear[i] = Math.Min(10, t.m_aWear[i] + 1);
				else
					t.m_aWear[i] = Math.Max(0, t.m_aWear[i] - 1);
			}

			if (!IsOn())
				continue;

			if (CountFear(t, night))
				m_Tribes.BuildTrails(t);

			CountHabits(t);
			for (int k = t.m_aMarks.Count() - 1; k >= 0; k--)
			{
				t.m_aMarks[k].m_fStrength = t.m_aMarks[k].m_fStrength - 25;
				if (t.m_aMarks[k].m_fStrength <= 0)
					t.m_aMarks.Remove(k);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Tonight's first trail, weighted: worn trails more (+0.3 a night of wear), a driver's roads and
	//! crossings (+2), never one toward a neighbour it lost to in the last 3 counted nights.
	int PickTrail(RZ_Tribe t)
	{
		int n = t.m_aTrails.Count();
		if (n <= 0)
			return -1;

		RZ_Land land = m_GM.GetLand();
		RZ_Tribe winner;
		if (t.m_iNoHuntToward >= 0 && m_Tribes.CountedNights() < t.m_iNoHuntUntil)
			winner = m_Tribes.Find(t.m_iNoHuntToward);

		array<float> w = {};
		float total = 0;
		for (int i = 0; i < n; i++)
		{
			float weight = 1;
			if (i < t.m_aWear.Count())
				weight += 0.3 * t.m_aWear[i];

			vector ground = t.m_aGrounds[Math.Min(i, t.m_aGrounds.Count() - 1)];
			int idx = land.CellAt(ground);
			if (t.Echo(RZ_EAlphaKind.DRIVER) && idx >= 0 && (land.GetRoadWidth(idx) > 0 || land.HasBit(idx, RZ_Land.BIT_BRIDGE) || land.HasBit(idx, RZ_Land.BIT_FORD)))
				weight += 2;

			// Where the prey is (RZ_TribeRest): up to +4
			weight += m_Tribes.GetRest().PreyWeight(t, i);
			if (winner && RZ_Util.DistXZ(ground, winner.RangeCentre()) < RZ_Util.DistXZ(ground, t.RangeCentre()))
				weight = 0.01;

			w.Insert(weight);
			total += weight;
		}

		float roll = Math.RandomFloat(0, total);
		for (int k = 0; k < n; k++)
		{
			roll -= w[k];
			if (roll <= 0)
				return k;
		}

		return n - 1;
	}

	//------------------------------------------------------------------------------------------------
	//! How worn a trail is (0-10), for the Hunter's tracks.
	static int WearOf(RZ_Tribe t, int trail)
	{
		if (trail < 0 || trail >= t.m_aWear.Count())
			return 0;

		return t.m_aWear[trail];
	}

	// ================================================================ Bonds (rule 20)
	//------------------------------------------------------------------------------------------------
	//! One of a bonded pair was put down near a player: the other, if its body is near, runs reckless
	//! until it's put down too, and a Hunter close by sees it.
	void OnYoungKilled(RZ_Tribe t, vector at)
	{
		if (!IsOn() || t.m_iBonds <= 0)
			return;

		RZ_Behaviour bh = RZ_Behaviour.Get();
		if (!bh)
			return;

		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (!p.HasBody() || RZ_Util.DistXZ(p.m_vPos, at) > 120)
				continue;

			if (p.m_eKind != RZ_ETribePart.YOUNG && p.m_iYoung <= 0)
				continue;

			if (!bh.Reckless(p.m_Body, at))
				continue;

			t.m_iBonds = t.m_iBonds - 1;
			TellHunters(at, 150, "That one came straight at us, after we put down the other.");
			RZ_Log.Info(string.Format("Tribes: a bonded young of %1 runs reckless.", t.m_sPlace));
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void TellHunters(vector at, float radius, string text)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		float now = m_Tribes.GetClock();
		foreach (int i, IEntity p : players)
		{
			int id = ids[i];
			RZ_Background bg = m_GM.GetPlayerBackground(id);
			if (!bg || bg.m_sId != "hunter" || RZ_Util.DistXZ(p.GetOrigin(), at) > radius)
				continue;

			float told;
			if (m_mBondToldAt.Find(id, told) && now - told < 600)
				continue;

			m_mBondToldAt.Set(id, now);
			m_GM.NotifyPlayer(id, "Tracks", text);
		}
	}

	// ================================================================ Echoes
	//------------------------------------------------------------------------------------------------
	//! A soldier's tribe sets out on the shift change: a Soldier or a Police Officer within 2 km knows
	//! it for what it is (once an hour).
	void OnSetOut(RZ_Tribe t)
	{
		if (!IsOn() || !t.Echo(RZ_EAlphaKind.SOLDIER))
			return;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		float now = m_Tribes.GetClock();
		foreach (int i, IEntity p : players)
		{
			int id = ids[i];
			RZ_Background bg = m_GM.GetPlayerBackground(id);
			if (!bg || (bg.m_sId != "soldier" && bg.m_sId != "police") || RZ_Util.DistXZ(p.GetOrigin(), t.m_vLair) > 2000)
				continue;

			float told;
			if (m_mEchoToldAt.Find(id, told) && now - told < 3600)
				continue;

			m_mEchoToldAt.Set(id, now);
			m_GM.NotifyPlayer(id, "The shift change", string.Format("They set out from %1 on the shift change, half five on the dot. Somebody taught them that.", t.m_sPlace));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The column's dawn: one of its groups walks out toward the line and stops 600 m short of the
	//! warning zone, then goes home. True if it went.
	bool ColumnDawn(RZ_Tribe t)
	{
		if (!IsOn() || !t.Echo(RZ_EAlphaKind.COLUMN))
			return false;

		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		if (!perimeter || !perimeter.IsActive())
			return false;

		float edge = perimeter.EdgeDistance(t.m_vLair);
		float walk = edge - perimeter.GetWarnDistance() - 600;
		if (walk < 200 || walk > 4000)
			return false;

		vector outward = -perimeter.InwardAt(t.m_vLair);
		outward[1] = 0;
		if (outward.Length() < 0.1)
			return false;

		outward.Normalize();
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (p.m_eKind != RZ_ETribePart.HUNTERS || p.m_iTarget >= 0 || p.m_iCount <= 0)
				continue;

			vector stop = RZ_Util.GroundPos(t.m_vLair + outward * walk);
			array<vector> path = {p.m_vPos, stop, p.m_vRest};
			p.SetPath(path);
			p.m_iStep = 1;
			p.m_vTarget = stop;
			p.m_bOut = true;
			p.m_bHome = true;
			p.m_fHoldUntil = m_Tribes.GetClock();
			m_Tribes.Tell(stop, 1500, "First light", "At first light some of them walk out toward the line, stop short of it, and stand there a while.");
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A farmer's tribe comes home at first light the long way round, by a field near the lair.
	void FarmerHome(RZ_Tribe t)
	{
		RZ_Land land = m_GM.GetLand();
		vector field;
		bool hasField = land.NearestCellWith(t.m_vLair, RZ_Land.BIT_FIELD, 150, t.m_fRadius * 1.5, field);
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (!p.m_bOut || p.m_bHome || p.m_iTarget >= 0)
				continue;

			if (!hasField)
			{
				m_Tribes.PathHome(t, p);
				continue;
			}

			array<vector> path = {p.m_vPos, RZ_Util.GroundPos(field), p.m_vRest};
			p.SetPath(path);
			p.m_iStep = 1;
			p.m_vTarget = path[1];
			p.m_bHome = true;
			p.m_bHop = false;
			p.m_fHoldUntil = m_Tribes.GetClock() + Math.RandomFloat(0, 300);
		}
	}

	// ================================================================ The Hunter's reading
	//------------------------------------------------------------------------------------------------
	//! What a Hunter adds to a track: "A path worn through the bracken." on a trail walked 6 nights or
	//! more, "Faint: new ground for them." on a new one; "By the fields, the long way round." for a
	//! farmer's tribe coming home.
	static string WearLine(int wear)
	{
		if (wear >= 6)
			return " A path worn through the bracken. Most nights.";

		if (wear >= 3)
			return " They come this way most nights.";

		if (wear == 0)
			return " Faint: new ground for them.";

		return "";
	}

	// ================================================================ Save
	//------------------------------------------------------------------------------------------------
	static void SaveTribe(RZ_Tribe t, RZ_TribeSave s)
	{
		s.kind = t.m_eKind;
		s.goalX = t.m_vGoal[0];
		s.goalZ = t.m_vGoal[2];
		s.roverUntil = t.m_fRoverUntil;
		s.fromTribe = t.m_iFrom;
		s.alphaName = t.m_sAlphaName;
		s.alphaKind = t.m_eAlphaKind;
		foreach (string h : t.m_aHistory)
		{
			s.history.Insert(h);
		}

		s.challenged = t.m_bChallenged;
		s.look = t.m_eLook;
		s.militaryShare = t.m_fMilitaryShare;
		s.disturb = t.m_fDisturb;
		foreach (float d : t.m_aDisturbBy)
		{
			s.disturbBy.Insert(d);
		}

		s.movedDay = t.m_iMovedDay;
		s.weakUntil = t.m_fWeakUntil;
		s.restless = t.m_bRestless;
		s.mother = t.m_iMother;
		s.clan = t.m_iClan;
		s.clanHold = t.m_iClanHold;
		foreach (RZ_FearPlace f : t.m_aFear)
		{
			s.fearX.Insert(f.m_vPos[0]);
			s.fearZ.Insert(f.m_vPos[2]);
			s.fearUntil.Insert(f.m_iUntil);
		}

		foreach (RZ_TribeHabit h2 : t.m_aHabits)
		{
			s.habitKind.Insert(h2.m_eKind);
			s.habitX.Insert(h2.m_vPos[0]);
			s.habitZ.Insert(h2.m_vPos[2]);
			s.habitHour.Insert(h2.m_fHour);
			s.habitStrength.Insert(h2.m_fStrength);
		}

		foreach (RZ_TribeMark m : t.m_aMarks)
		{
			s.markPlayer.Insert(m.m_iPlayer);
			s.markLife.Insert(m.m_iLife);
			s.markStrength.Insert(m.m_fStrength);
		}

		foreach (int w : t.m_aWear)
		{
			s.wear.Insert(w);
		}

		s.driftX = t.m_vDrift[0];
		s.driftZ = t.m_vDrift[2];
		foreach (vector l : t.m_aLoop)
		{
			s.loopX.Insert(l[0]);
			s.loopZ.Insert(l[2]);
		}

		s.loopAt = t.m_iLoopAt;
		s.nextMove = t.m_fNextMove;
		s.starveNights = t.m_iStarveNights;
		s.bonds = t.m_iBonds;
		s.noHuntToward = t.m_iNoHuntToward;
		s.noHuntUntil = t.m_iNoHuntUntil;
	}

	//------------------------------------------------------------------------------------------------
	static void RestoreTribe(RZ_Tribe t, RZ_TribeSave s)
	{
		t.m_eKind = Math.ClampInt(s.kind, 0, 2);
		t.m_vGoal = RZ_Util.GroundPos(Vector(s.goalX, 0, s.goalZ));
		t.m_fRoverUntil = s.roverUntil;
		t.m_iFrom = s.fromTribe;
		t.m_sAlphaName = s.alphaName;
		t.m_eAlphaKind = Math.ClampInt(s.alphaKind, 0, 5);
		if (s.history)
		{
			foreach (string h : s.history)
			{
				t.m_aHistory.Insert(h);
			}
		}

		t.m_bChallenged = s.challenged;
		t.m_eLook = Math.ClampInt(s.look, 0, 4);
		t.m_fMilitaryShare = s.militaryShare;
		t.m_fDisturb = s.disturb;
		if (s.disturbBy)
		{
			for (int d = 0; d < s.disturbBy.Count() && d < t.m_aDisturbBy.Count(); d++)
			{
				t.m_aDisturbBy[d] = s.disturbBy[d];
			}
		}

		t.m_iMovedDay = s.movedDay;
		t.m_fWeakUntil = s.weakUntil;
		t.m_bRestless = s.restless;
		t.m_iMother = s.mother;
		t.m_iClan = s.clan;
		t.m_iClanHold = s.clanHold;
		if (s.fearX && s.fearZ && s.fearUntil)
		{
			for (int f = 0; f < s.fearX.Count() && f < s.fearZ.Count() && f < s.fearUntil.Count(); f++)
			{
				RZ_FearPlace fp = new RZ_FearPlace();
				fp.m_vPos = RZ_Util.GroundPos(Vector(s.fearX[f], 0, s.fearZ[f]));
				fp.m_iUntil = s.fearUntil[f];
				t.m_aFear.Insert(fp);
			}
		}

		if (s.habitKind && s.habitX && s.habitZ && s.habitHour && s.habitStrength)
		{
			for (int h2 = 0; h2 < s.habitKind.Count() && h2 < s.habitStrength.Count() && h2 < s.habitX.Count() && h2 < s.habitZ.Count() && h2 < s.habitHour.Count(); h2++)
			{
				RZ_TribeHabit hb = new RZ_TribeHabit();
				hb.m_eKind = Math.ClampInt(s.habitKind[h2], 0, 4);
				hb.m_vPos = RZ_Util.GroundPos(Vector(s.habitX[h2], 0, s.habitZ[h2]));
				hb.m_fHour = s.habitHour[h2];
				hb.m_fStrength = s.habitStrength[h2];
				t.m_aHabits.Insert(hb);
			}
		}

		if (s.markPlayer && s.markLife && s.markStrength)
		{
			for (int m = 0; m < s.markPlayer.Count() && m < s.markLife.Count() && m < s.markStrength.Count(); m++)
			{
				RZ_TribeMark mk = new RZ_TribeMark();
				mk.m_iPlayer = s.markPlayer[m];
				mk.m_iLife = s.markLife[m];
				mk.m_fStrength = s.markStrength[m];
				t.m_aMarks.Insert(mk);
			}
		}

		if (s.wear)
		{
			foreach (int w : s.wear)
			{
				t.m_aWear.Insert(w);
			}
		}

		t.m_vDrift = Vector(s.driftX, 0, s.driftZ);
		if (s.loopX && s.loopZ)
		{
			for (int l = 0; l < s.loopX.Count() && l < s.loopZ.Count(); l++)
			{
				t.m_aLoop.Insert(RZ_Util.GroundPos(Vector(s.loopX[l], 0, s.loopZ[l])));
			}
		}

		t.m_iLoopAt = s.loopAt;
		t.m_fNextMove = s.nextMove;
		t.m_iStarveNights = s.starveNights;
		t.m_iBonds = s.bonds;
		t.m_iNoHuntToward = s.noHuntToward;
		t.m_iNoHuntUntil = s.noHuntUntil;
	}
}
