//! The tribes' life cycle (docs/INFECTED.md, "The life cycle"): successions with the guard fight
//! and splinters, lone ones, ageing and the Clash, break-ups, new tribes (from the taken, from a
//! town gone dark, round a lone one), splits with the kept slot and its room, clans, disturbance and
//! lair moves with displacement and its caps, nomads' loops, abandoned lairs, vacuums, seasonal
//! pull and Winter, burned land, and starvation into crawlers. Borders are RZ_TribeBorders, memory
//! (fear, habits, marks, trails, bonds, echoes, histories) RZ_TribeWays. Owned by RZ_Tribes, which
//! calls it at its phases, its data tick and its counted nights. Server only.
class RZ_TribeLife
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_Tribes m_Tribes;		// weak: the owner
	protected ref array<ref RZ_Tribe> m_aRovers = {};
	// Burned land (rule 23)
	protected ref array<vector> m_aBurnPos = {};
	protected ref array<float> m_aBurnUntil = {};		// season days: no lair (and no trail for an event fire)
	protected ref array<int> m_aBurnTrailUntil = {};	// counted night trails avoid it until
	// Vacuums (rule 18)
	protected ref array<vector> m_aVacPos = {};
	protected ref array<float> m_aVacUntil = {};
	protected ref array<string> m_aVacPlace = {};
	// The taken in the open, outside every range (rule 2: five make a tribe)
	protected ref array<vector> m_aOpenPos = {};
	protected ref array<float> m_aOpenDay = {};
	protected ref array<string> m_aOpenWho = {};
	// Abandoned lairs (rule 21)
	protected ref array<vector> m_aLairPos = {};
	protected ref array<float> m_aLairUntil = {};
	protected ref array<string> m_aLairFinds = {};
	protected ref map<string, bool> m_mLairTold = new map<string, bool>();
	// Towns gone dark (season days since, -1 not dark)
	protected ref array<float> m_aDarkSince = {};
	protected ref array<float> m_aWalkersToday = {};
	protected ref array<int> m_aDisplaced = {0, 0, 0, 0, 0};	// displacements this season, by causing faction
	protected int m_iSeasonDay = -1;
	protected int m_iLastSplitDay = -1;
	protected int m_iLastNewDay = -1;
	protected int m_iMovesToday;
	protected int m_iLastTruckNight = -10;

	static const float VACUUM_DAYS = 3;
	static const float BURN_RADIUS = 150;
	static const float BURN_DAYS = 3;
	static const float LONE_DAYS = 6;
	static const int MAX_LONE = 2;
	static const int MAX_SPLINTERS = 4;
	static const int MAX_MEMBERS = 30;
	static const float CLAN_KM = 4000;
	static const float CLASH_SECONDS = 180;

	//------------------------------------------------------------------------------------------------
	void RZ_TribeLife(RZ_GameModeComponent gm, RZ_Tribes tribes)
	{
		m_GM = gm;
		m_Tribes = tribes;
	}

	//------------------------------------------------------------------------------------------------
	array<ref RZ_Tribe> GetRovers() { return m_aRovers; }

	//------------------------------------------------------------------------------------------------
	//! Zombies > Tribes > Life cycle (off: phase 2's tribes, where one of the guard takes the call).
	bool IsOn()
	{
		return m_Tribes.GetSettings().m_bLifeCycle;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Land Land()
	{
		return m_GM.GetLand();
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_TribeWays Ways()
	{
		return m_Tribes.GetWays();
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_EOutbreakStage Stage()
	{
		if (!m_GM.GetDirector())
			return RZ_EOutbreakStage.EARLY_OUTBREAK;

		return m_GM.GetDirector().GetStage();
	}

	// ================================================================ The tick
	//------------------------------------------------------------------------------------------------
	//! Every played minute (the tribes' data tick): the season day turning, disturbance at the
	//! lairs, the rovers walking, the Clash ending, abandoned lairs found.
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		int day = RZ_SeasonClock.Day();
		if (m_iSeasonDay < 0)
			m_iSeasonDay = day;
		else if (day != m_iSeasonDay)
		{
			m_iSeasonDay = day;
			if (IsOn())
				SeasonDay();
		}

		if (!IsOn())
			return;

		float clock = m_Tribes.GetClock();
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			LairDisturbance(t, players, dt);
			if (t.m_bClash && clock >= t.m_fClashUntil)
				ResolveChallenge(t, true);
		}

		for (int i = m_aRovers.Count() - 1; i >= 0; i--)
		{
			RZ_Tribe r = m_aRovers[i];
			StepRover(r, dt, players);
			if (r.Members() <= 0)
			{
				RZ_Log.Info(string.Format("Tribes: the %1 from %2 is gone.", RoverWord(r), r.m_sPlace));
				m_aRovers.Remove(i);
			}
		}

		ReadAbandonedLairs(ids, players);
	}

	//------------------------------------------------------------------------------------------------
	protected static string RoverWord(RZ_Tribe r)
	{
		if (r.m_eKind == RZ_ETribeKind.LONE)
			return "lone one";

		return "splinter";
	}

	// ================================================================ Members
	//------------------------------------------------------------------------------------------------
	//! Takes up to n members of a kind from the tribe's parts with no bodies (lying up or walking as
	//! data): a body that's in a fight is never taken away. The young inside hunter groups count as
	//! young. Returns how many.
	int TakeMembers(RZ_Tribe t, RZ_ETribePart kind, int n)
	{
		int got = 0;
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (got >= n)
				break;

			if (p.HasBody() || p.m_iCount <= 0 || p.m_iTarget >= 0)
				continue;

			bool mixed = p.m_eKind == RZ_ETribePart.HUNTERS || p.m_eKind == RZ_ETribePart.DAY;
			int avail = 0;
			if (p.m_eKind == kind && !mixed)
				avail = p.m_iCount;
			else if (mixed && kind == RZ_ETribePart.HUNTERS)
				avail = p.m_iCount - p.m_iYoung;
			else if (mixed && kind == RZ_ETribePart.YOUNG)
				avail = p.m_iYoung;

			int take = Math.Min(avail, n - got);
			if (take <= 0)
				continue;

			p.m_iCount = p.m_iCount - take;
			if (mixed && kind == RZ_ETribePart.YOUNG)
				p.m_iYoung = p.m_iYoung - take;

			got += take;
		}

		t.CountRoles();
		return got;
	}

	//------------------------------------------------------------------------------------------------
	//! Takes n of the young and hunters (young first, then hunters), as data. Out: how many young.
	int TakeMixed(RZ_Tribe t, int n, out int youngTaken)
	{
		youngTaken = TakeMembers(t, RZ_ETribePart.YOUNG, n / 2);
		int hunters = TakeMembers(t, RZ_ETribePart.HUNTERS, n - youngTaken);
		if (youngTaken + hunters < n)
			youngTaken += TakeMembers(t, RZ_ETribePart.YOUNG, n - youngTaken - hunters);

		return youngTaken + hunters;
	}

	//------------------------------------------------------------------------------------------------
	//! Members join a tribe, up to 30: guards to its guard, the rest to its resting ring.
	void Join(RZ_Tribe t, int guards, int hunters, int young)
	{
		int room = MAX_MEMBERS - t.Members();
		if (room <= 0)
			return;

		int g = Math.Min(guards, room);
		room -= g;
		int h = Math.Min(hunters, room);
		room -= h;
		int y = Math.Min(young, room);
		if (g > 0)
		{
			RZ_TribePart gp = m_Tribes.RestingPart(t, RZ_ETribePart.GUARD);
			if (gp)
				gp.m_iCount = gp.m_iCount + g;
			else
				m_Tribes.AddPart(t, RZ_ETribePart.GUARD, g, 0);
		}

		m_Tribes.GiveToRest(t, y, h);
		t.CountRoles();
	}

	//------------------------------------------------------------------------------------------------
	//! A new voice: one of the guard (else a hunter) leads, Rising, with a name and who they were.
	//! False if nobody was left to take the call.
	bool NewAlpha(RZ_Tribe t, string how)
	{
		int took = TakeMembers(t, RZ_ETribePart.GUARD, 1);
		if (took <= 0)
			took = TakeMembers(t, RZ_ETribePart.HUNTERS, 1);

		if (took <= 0)
			return false;

		t.m_bHasAlpha = true;
		t.m_fLedSince = RZ_SeasonClock.Days();
		t.m_fAlphaHealth = 1;
		t.m_iAlphaTaken = 0;
		t.m_bChallenged = false;
		t.m_aHistory.Clear();
		Ways().NameAlpha(t);
		t.AddHistory(how);
		if (!t.FindPart(RZ_ETribePart.ALPHA))
			m_Tribes.AddPart(t, RZ_ETribePart.ALPHA, 1, 0);

		t.CountRoles();
		return true;
	}

	// ================================================================ Succession (rule 15)
	//------------------------------------------------------------------------------------------------
	//! At dawn after the vigil: the guard fight. The strongest guard becomes the new alpha (Rising);
	//! one other guard dies, or leaves as a lone one a quarter of the time; 30-40% of the rest break
	//! off as 1-2 splinters of 3-6; no day party that morning; two weak season days; the borders
	//! reopen. With no guard left the whole tribe splinters and its slot frees.
	void Succession(RZ_Tribe t)
	{
		if (t.m_bHasAlpha)
			return;

		// After the cure nobody takes up the call: the rest scatter and die out (the last week)
		if (Cured())
		{
			m_Tribes.Tell(t.m_vLair, 2000, "First light", string.Format("Nothing calls from %1 this morning. What was left of it has gone off into the trees to lie down.", t.m_sPlace));
			RZ_Log.Info(string.Format("Tribes: %1 lost its leader after the cure, and ends.", t.m_sPlace));
			RemoveTribe(t, false);
			return;
		}

		int guards = t.Count(RZ_ETribePart.GUARD);
		if (guards <= 0)
		{
			BreakUp(t, "nobody of its guard was left");
			return;
		}

		if (!NewAlpha(t, "took the call at first light, after a fight among the guard"))
		{
			BreakUp(t, "nobody could take the call");
			return;
		}

		if (guards >= 2 && TakeMembers(t, RZ_ETribePart.GUARD, 1) > 0 && Math.RandomFloat01() < 0.25)
			AddLone(t, t.m_vLair, 0.8, "a guard who lost the fight");

		int rest = t.Members() - 1;
		int n = Math.Min(12, RZ_Util.RoundToInt(Math.RandomFloat(0.3, 0.4) * rest));
		if (n >= 3 && !Cured())
			Splinter(t, n, t.m_vLair);

		t.m_fWeakUntil = RZ_SeasonClock.Days() + 2;
		t.m_bNoDayParty = true;
		m_Tribes.GetBorders().Reopen(t);
		m_Tribes.Tell(t.m_vLair, 2000, "First light", string.Format("Shrieks from %1 at first light, a fight by the sound of it. Then one voice, and the rest go quiet.", t.m_sPlace));
		Ways().Headline(t, string.Format("Something new leads at %1 now: %2. Some of the rest broke off and went down onto the roads.", t.m_sPlace, t.m_sAlphaName));
		RZ_Log.Info(string.Format("Tribes: %1's guard fought at first light; %2 leads now (%3), %4 splintered off.", t.m_sPlace, t.m_sAlphaName, Ways().AlphaKindName(t.m_eAlphaKind), n));
		CheckBreakUp(t);
	}

	//------------------------------------------------------------------------------------------------
	//! The tribe is gone: its alpha (if any) leaves as a lone one, the rest as splinters (or with the
	//! crawlers, past the server's 4 splinters), and its range is a vacuum.
	void BreakUp(RZ_Tribe t, string why)
	{
		if (t.m_bHasAlpha)
			AddLone(t, t.m_vLair, t.m_fAlphaHealth, "the leader of a pack that broke up");

		int n = t.Members();
		if (t.m_bHasAlpha)
			n--;

		if (n > 0 && !Cured())
			Splinter(t, n, t.m_vLair);

		m_Tribes.Tell(t.m_vLair, 2000, "Quiet", string.Format("Nothing calls from %1 any more. What was left of it scattered.", t.m_sPlace));
		RZ_Log.Info(string.Format("Tribes: %1 broke up (%2); its slot is free.", t.m_sPlace, why));
		RemoveTribe(t, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Under 4 members (never in its first 3 season days), a tribe breaks up.
	void CheckBreakUp(RZ_Tribe t)
	{
		// After the cure a tribe keeps its lair and its leader however small it gets
		if (Cured())
			return;

		if (t.Members() < 4 && RZ_SeasonClock.DaysSince(t.m_fFounded) >= 3 && m_Tribes.GetTribeList().Find(t) >= 0)
			BreakUp(t, "under 4 members");
	}

	//------------------------------------------------------------------------------------------------
	//! A tribe leaves the map: its hunt ends, its range is a vacuum (unless it's a nomad, or absorbed).
	void RemoveTribe(RZ_Tribe t, bool vacuum)
	{
		m_Tribes.GetBodies().EndHunt(t, false);
		if (vacuum && t.m_eTemper != RZ_ETemperament.NOMADIC)
			AddVacuum(t.RangeCentre(), t.m_sPlace);

		m_Tribes.GetTribeList().RemoveItem(t);
		m_Tribes.GetBorders().Forget(t.m_iId);
	}

	// ================================================================ Rovers: lone ones and splinters
	//------------------------------------------------------------------------------------------------
	//! A lone one (rule 16, 2 at most): it walks between ranges by night and lies up alone by day;
	//! it dies after 6 season days alone, unless it gathers splinters into a tribe.
	RZ_Tribe AddLone(RZ_Tribe from, vector at, float health, string who)
	{
		if (CountRovers(RZ_ETribeKind.LONE) >= MAX_LONE || Cured())
			return null;

		RZ_Tribe r = NewRover(from, at, RZ_ETribeKind.LONE);
		r.m_bHasAlpha = true;
		r.m_fAlphaHealth = Math.Clamp(health, 0.2, 1);
		r.m_sAlphaName = from.m_sAlphaName;
		r.m_eAlphaKind = from.m_eAlphaKind;
		r.m_sAlphaWho = who;
		r.m_fLedSince = from.m_fLedSince;
		foreach (string h : from.m_aHistory)
		{
			r.m_aHistory.Insert(h);
		}

		r.m_fRoverUntil = RZ_SeasonClock.Days() + LONE_DAYS;
		RZ_TribePart p = m_Tribes.AddPart(r, RZ_ETribePart.ALPHA, 1, 0);
		p.m_vRest = at;
		p.m_vPos = at;
		LoneGoal(r);
		RZ_Log.Info(string.Format("Tribes: a lone one leaves %1 (%2).", from.m_sPlace, who));
		return r;
	}

	//------------------------------------------------------------------------------------------------
	//! n members of a tribe break off as 1-2 splinters of 3-6 (4 on the server at most; the rest end
	//! up with the nearest town's crawlers). They wander the roads for 1-2 season days.
	void Splinter(RZ_Tribe t, int n, vector at)
	{
		int left = n;
		while (left >= 3 && CountRovers(RZ_ETribeKind.SPLINTER) < MAX_SPLINTERS)
		{
			int k = Math.Min(left, Math.RandomIntInclusive(3, 6));
			if (left - k > 0 && left - k < 3)
				k = left;

			int young;
			int got = TakeMixed(t, k, young);
			if (got <= 0)
				break;

			RZ_Tribe r = NewRover(t, at, RZ_ETribeKind.SPLINTER);
			r.m_fRoverUntil = RZ_SeasonClock.Days() + Math.RandomFloat(1, 2);
			RZ_TribePart p = m_Tribes.AddPart(r, RZ_ETribePart.HUNTERS, got, young);
			p.m_vPos = at;
			p.m_vRest = at;
			SplinterGoal(r);
			left -= got;
			RZ_Log.Info(string.Format("Tribes: %1 of %2 splinter off onto the roads.", got, t.m_sPlace));
		}

		if (left > 0)
		{
			int youngLeft;
			int gone = TakeMixed(t, left, youngLeft);
			ToCrawlers(at, gone);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Tribe NewRover(RZ_Tribe from, vector at, RZ_ETribeKind kind)
	{
		RZ_Tribe r = new RZ_Tribe();
		r.m_iId = m_Tribes.NextId();
		r.m_eKind = kind;
		r.m_iSeed = Math.RandomInt(1, 1000000);
		r.m_iFrom = from.m_iId;
		r.m_sPlace = from.m_sPlace;
		r.m_vLair = at;
		r.m_sLairKind = from.m_sLairKind;
		r.m_eTemper = from.m_eTemper;
		r.m_eLook = from.m_eLook;
		r.m_fMilitaryShare = from.m_fMilitaryShare;
		r.m_sPattern = from.m_sPattern;
		r.m_fFounded = RZ_SeasonClock.Days();
		r.m_fRadius = 300;
		r.m_fHunger = from.m_fHunger;
		m_aRovers.Insert(r);
		return r;
	}

	//------------------------------------------------------------------------------------------------
	int CountRovers(RZ_ETribeKind kind)
	{
		int n = 0;
		foreach (RZ_Tribe r : m_aRovers)
		{
			if (r.m_eKind == kind)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! A splinter's way: a road 1-2 km off, else open wild land (they're "on the roads").
	protected void SplinterGoal(RZ_Tribe r)
	{
		RZ_Land land = Land();
		vector start = r.m_vLair;
		RZ_TribePart p = r.FindPart(RZ_ETribePart.HUNTERS);
		if (p)
			start = p.m_vPos;

		vector best = RZ_Util.GroundPos(RZ_Util.RandomPointInRing(start, 1000, 2000));
		for (int i = 0; i < 8; i++)
		{
			vector cand = RZ_Util.GroundPos(RZ_Util.RandomPointInRing(start, 1000, 2000));
			int idx = land.CellAt(cand);
			if (idx < 0 || !land.IsDry(idx) || land.InNoMansLand(cand))
				continue;

			best = cand;
			if (land.GetRoadWidth(idx) > 0)
				break;
		}

		r.m_vGoal = best;
		if (!p)
			return;

		array<vector> path = {p.m_vPos, best};
		p.SetPath(path);
		p.m_iStep = 1;
		p.m_bOut = true;
		p.m_bHome = false;
		p.m_fHoldUntil = m_Tribes.GetClock();
	}

	//------------------------------------------------------------------------------------------------
	//! A lone one's night: along the edge of a range (or into a vacuum, or toward splinters it could
	//! gather) within 4 km.
	protected void LoneGoal(RZ_Tribe r)
	{
		RZ_TribePart p = r.FindPart(RZ_ETribePart.ALPHA);
		if (!p)
			return;

		vector goal = RZ_Util.GroundPos(RZ_Util.RandomPointInRing(p.m_vPos, 800, 2000));
		foreach (vector vac : m_aVacPos)
		{
			if (RZ_Util.DistXZ(vac, p.m_vPos) < 4000)
			{
				goal = vac;
				break;
			}
		}

		foreach (RZ_Tribe sp : m_aRovers)
		{
			RZ_TribePart sPart = sp.FindPart(RZ_ETribePart.HUNTERS);
			if (sp.m_eKind == RZ_ETribeKind.SPLINTER && sPart && RZ_Util.DistXZ(sPart.m_vPos, p.m_vPos) < 4000)
			{
				goal = sPart.m_vPos;
				break;
			}
		}

		if (goal == p.m_vPos)
		{
			RZ_Tribe near = m_Tribes.NearestTribe(p.m_vPos, 5000);
			if (near)
			{
				vector away = p.m_vPos - near.RangeCentre();
				away[1] = 0;
				if (away.Length() < 1)
					away = RZ_Behaviour.RandomDir();

				away.Normalize();
				vector side = Vector(away[2], 0, -away[0]);
				goal = RZ_Util.GroundPos(near.RangeCentre() + away * near.m_fRadius + side * 300);
			}
		}

		if (Land().InNoMansLand(goal))
			goal = p.m_vPos;

		r.m_vGoal = goal;
		array<vector> path = {p.m_vPos, goal};
		p.SetPath(path);
		p.m_iStep = 1;
		p.m_bOut = true;
		p.m_bHome = false;
		p.m_fHoldUntil = m_Tribes.GetClock();
	}

	//------------------------------------------------------------------------------------------------
	//! A rover as data: its parts walk (bodies within reach of players, as a tribe's); a splinter
	//! picks a new road when it gets there, a lone one lies up where it stopped at dawn.
	protected void StepRover(RZ_Tribe r, float dt, array<IEntity> players)
	{
		for (int i = r.m_aParts.Count() - 1; i >= 0; i--)
		{
			RZ_TribePart p = r.m_aParts[i];
			if (p.m_iCount <= 0 && !p.HasBody())
			{
				r.m_aParts.Remove(i);
				continue;
			}

			m_Tribes.StepPart(r, p, dt, players);
			if (!p.m_bOut && p.m_aPath.IsEmpty() && r.m_eKind == RZ_ETribeKind.SPLINTER)
				SplinterGoal(r);
		}

		RZ_TribePart first = r.FindPart(RZ_ETribePart.ALPHA);
		if (!first)
			first = r.FindPart(RZ_ETribePart.HUNTERS);

		if (first)
			r.m_vLair = first.m_vRest;
	}

	//------------------------------------------------------------------------------------------------
	//! Night: lone ones walk; day: they lie up alone on the highest wild ground near them.
	void RoversAtPhase(RZ_ETribePhase phase)
	{
		foreach (RZ_Tribe r : m_aRovers)
		{
			if (r.m_eKind != RZ_ETribeKind.LONE)
				continue;

			RZ_TribePart p = r.FindPart(RZ_ETribePart.ALPHA);
			if (!p || p.HasBody())
				continue;

			if (phase == RZ_ETribePhase.NIGHT)
			{
				LoneGoal(r);
				continue;
			}

			if (phase == RZ_ETribePhase.DAY)
			{
				p.m_vRest = HighGround(p.m_vPos, 400);
				m_Tribes.PathHome(r, p);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The highest dry wild square within reach ("a big one alone on a ridge").
	protected vector HighGround(vector pos, float reach)
	{
		RZ_Land land = Land();
		vector best = pos;
		float bestH = -10000;
		int r = reach / RZ_Land.CELL;
		for (int dz = -r; dz <= r; dz++)
		{
			for (int dx = -r; dx <= r; dx++)
			{
				vector c = pos + Vector(dx * RZ_Land.CELL, 0, dz * RZ_Land.CELL);
				int idx = land.CellAt(c);
				if (idx < 0 || !land.IsDry(idx) || !land.HasBit(idx, RZ_Land.BIT_WILD))
					continue;

				float hgt = land.GetHeight(idx);
				if (land.HasBit(idx, RZ_Land.BIT_RIDGE))
					hgt += 30;

				if (hgt > bestH)
				{
					bestH = hgt;
					best = RZ_Util.GroundPos(land.CellCenter(idx));
				}
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! A rover body died (the spawner's per-member tracking). A lone one drops 2 samples once, with a
	//! player within 150 m (never after the cure).
	void OnRoverKilled(RZ_Tribe r, RZ_EInfectedRole role, vector at, bool silent)
	{
		if (role != RZ_EInfectedRole.ALPHA || r.m_eKind != RZ_ETribeKind.LONE)
			return;

		r.m_bHasAlpha = false;
		if (!silent && !Cured() && RZ_Tribes.PlayerWithin(at, 150))
		{
			m_GM.SpawnSamples(at, 2);
			m_Tribes.OnSample(2, true);
		}

		RZ_Log.Info(string.Format("Tribes: the lone one from %1 was killed at %2.", r.m_sPlace, RZ_SettlementScanner.GridReference(at)));
	}

	//------------------------------------------------------------------------------------------------
	//! Season day: splinters rejoin (50%), join the nearest tribe, their clan first (25%), or end up
	//! with the crawlers (25%; after the cure they just die out); lone ones die after 6 days alone, or
	//! gather 6 or more splinter members within 1 km into a new tribe.
	protected void RoversDay()
	{
		float now = RZ_SeasonClock.Days();
		for (int i = m_aRovers.Count() - 1; i >= 0; i--)
		{
			RZ_Tribe r = m_aRovers[i];
			RZ_TribePart p = r.FindPart(RZ_ETribePart.ALPHA);
			if (!p)
				p = r.FindPart(RZ_ETribePart.HUNTERS);

			if (!p || r.Members() <= 0)
			{
				m_aRovers.Remove(i);
				continue;
			}

			if (p.HasBody() || now < r.m_fRoverUntil)
				continue;

			if (r.m_eKind == RZ_ETribeKind.LONE)
			{
				RZ_Log.Info(string.Format("Tribes: the lone one from %1 died alone.", r.m_sPlace));
				m_aRovers.Remove(i);
				continue;
			}

			SplinterFate(r, p);
			m_aRovers.Remove(i);
		}

		GatherRoundLone();
	}

	//------------------------------------------------------------------------------------------------
	protected void SplinterFate(RZ_Tribe r, RZ_TribePart p)
	{
		int n = p.m_iCount;
		int young = p.m_iYoung;
		if (Cured())
		{
			RZ_Log.Info(string.Format("Tribes: the splinter from %1 dies out.", r.m_sPlace));
			return;
		}

		float roll = Math.RandomFloat01();
		RZ_Tribe home = m_Tribes.Find(r.m_iFrom);
		if (home && home.IsRover())
			home = null;

		if (roll < 0.5 && home)
		{
			Join(home, 0, n - young, young);
			RZ_Log.Info(string.Format("Tribes: the splinter from %1 rejoins it (%2).", r.m_sPlace, n));
			return;
		}

		if (roll < 0.75)
		{
			RZ_Tribe near = NearestForSplinter(p.m_vPos, home);
			if (near)
			{
				Join(near, 0, n - young, young);
				RZ_Log.Info(string.Format("Tribes: the splinter from %1 joins %2 (%3).", r.m_sPlace, near.m_sPlace, n));
				return;
			}
		}

		ToCrawlers(p.m_vPos, n);
	}

	//------------------------------------------------------------------------------------------------
	//! The tribe a splinter joins: its old tribe's clan first, else the nearest within 4 km.
	protected RZ_Tribe NearestForSplinter(vector pos, RZ_Tribe home)
	{
		if (home && home.m_iClan > 0)
		{
			foreach (RZ_Tribe c : m_Tribes.GetTribeList())
			{
				if (c != home && c.m_iClan == home.m_iClan)
					return c;
			}
		}

		return m_Tribes.NearestTribe(pos, 4000);
	}

	//------------------------------------------------------------------------------------------------
	//! A lone one within 1 km of 6 or more splinter members gathers them into a tribe, as its alpha,
	//! at the nearest free lair candidate (a free slot that isn't kept, one new tribe a season day).
	protected void GatherRoundLone()
	{
		foreach (RZ_Tribe lone : m_aRovers)
		{
			if (lone.m_eKind != RZ_ETribeKind.LONE)
				continue;

			RZ_TribePart lp = lone.FindPart(RZ_ETribePart.ALPHA);
			if (!lp || lp.HasBody())
				continue;

			array<RZ_Tribe> near = {};
			int members = 0;
			foreach (RZ_Tribe sp : m_aRovers)
			{
				RZ_TribePart spp = sp.FindPart(RZ_ETribePart.HUNTERS);
				if (sp.m_eKind != RZ_ETribeKind.SPLINTER || !spp || spp.HasBody() || RZ_Util.DistXZ(spp.m_vPos, lp.m_vPos) > 1000)
					continue;

				near.Insert(sp);
				members += spp.m_iCount;
			}

			if (members < 6 || !CanFound(lp.m_vPos))
				continue;

			vector lair;
			if (!FreeLair(lp.m_vPos, 2500, lair))
				continue;

			RZ_Tribe t = m_Tribes.NewTribe(lair, 1, false);
			t.m_iHunters = 0;
			t.m_iGuards = 0;
			t.m_iScouts = 0;
			t.m_iYoung = 0;
			t.m_aParts.Clear();
			t.m_bHasAlpha = true;
			t.m_sAlphaName = lone.m_sAlphaName;
			t.m_eAlphaKind = lone.m_eAlphaKind;
			t.m_fAlphaHealth = lone.m_fAlphaHealth;
			t.m_fLedSince = RZ_SeasonClock.Days();
			t.AddHistory("gathered the loose ones round itself");
			m_Tribes.AddPart(t, RZ_ETribePart.ALPHA, 1, 0);
			int guards = 0;
			foreach (RZ_Tribe g : near)
			{
				RZ_TribePart gp = g.FindPart(RZ_ETribePart.HUNTERS);
				int add = gp.m_iCount;
				int addYoung = gp.m_iYoung;
				// The two oldest become its guard
				int toGuard = Math.Min(2 - guards, add - addYoung);
				if (toGuard < 0)
					toGuard = 0;

				guards += toGuard;
				Join(t, toGuard, add - addYoung - toGuard, addYoung);
				m_aRovers.RemoveItem(g);
			}

			m_aRovers.RemoveItem(lone);
			FoundDone(t, "round a lone one");
			return;
		}
	}

	// ================================================================ New tribes (rule 2)
	//------------------------------------------------------------------------------------------------
	//! A new tribe may found at pos now: one a season day, into a free slot that isn't kept; while a
	//! kept slot is free, not within 3 km of a tribe of 18 or more (room kept for a split).
	bool CanFound(vector pos)
	{
		if (Cured() || m_iLastNewDay == RZ_SeasonClock.Day())
			return false;

		RZ_Land land = Land();
		int slots = land.Slots();
		int kept = RZ_Land.KeptForSplits(slots);
		int count = m_Tribes.GetTribeList().Count();
		if (count >= slots - kept)
			return false;

		if (kept > 0 && count < slots)
		{
			foreach (RZ_Tribe big : m_Tribes.GetTribeList())
			{
				if (big.Members() >= 18 && RZ_Util.DistXZ(big.m_vLair, pos) < 3000)
					return false;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A free slot for a split (a kept one first, then any).
	bool CanSplit()
	{
		if (Cured() || m_iLastSplitDay == RZ_SeasonClock.Day())
			return false;

		return m_Tribes.GetTribeList().Count() < Land().Slots();
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest lair candidate within reach of pos that's outside every range, not burned and not
	//! in no man's land. From Overrun a candidate nearer prey scores higher (seasonal pull).
	bool FreeLair(vector pos, float reach, out vector lair, RZ_Tribe skip = null, float minDist = 0)
	{
		RZ_Land land = Land();
		float best = -100000;
		bool found = false;
		int r = reach / RZ_Land.CELL;
		bool late = Stage() >= RZ_EOutbreakStage.OVERRUN;
		for (int dz = -r; dz <= r; dz++)
		{
			for (int dx = -r; dx <= r; dx++)
			{
				vector c = pos + Vector(dx * RZ_Land.CELL, 0, dz * RZ_Land.CELL);
				float d = RZ_Util.DistXZ(c, pos);
				if (d > reach || d < minDist)
					continue;

				int idx = land.CellAt(c);
				if (idx < 0 || !land.HasBit(idx, RZ_Land.BIT_LAIR))
					continue;

				vector cc = land.CellCenter(idx);
				if (!OutsideRanges(cc, skip) || IsBurned(cc, false))
					continue;

				float score = -d / 1000;
				if (late)
					score += PreyPull(cc);

				if (skip && skip.m_eAlphaKind == RZ_EAlphaKind.FARMER && land.GetMark(idx) > 0 && land.GetLairKindName(land.GetMark(idx)) == "barn")
					score += 1;

				if (score > best)
				{
					best = score;
					lair = RZ_Util.GroundPos(cc);
					found = true;
				}
			}
		}

		return found;
	}

	//------------------------------------------------------------------------------------------------
	//! Outside every tribe's range (by its radius + 300 m).
	bool OutsideRanges(vector pos, RZ_Tribe skip = null)
	{
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (t != skip && RZ_Util.DistXZ(t.RangeCentre(), pos) < t.m_fRadius + 300)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Seasonal pull (from Overrun): up to +2 for a lair within 3 km of the nearest lived-in place.
	protected float PreyPull(vector pos)
	{
		vector edge;
		if (!Land().NearestNoMansLand(pos, edge))
			return 0;

		return 2 * (1 - Math.Min(RZ_Util.DistXZ(edge, pos), 3000) / 3000);
	}

	//------------------------------------------------------------------------------------------------
	//! Season day: at most one new tribe on the server: from a town gone dark (70+ infection, not
	//! lived in, no range within 2 km for 3 season days: a tribe of 6), from the taken (5 people taken
	//! in the open within 1.5 km of each other in 3 season days, outside every range; 3 in a vacuum;
	//! the first of them leads).
	protected void NewTribes()
	{
		float now = RZ_SeasonClock.Days();
		array<ref RZ_Settlement> towns = m_GM.GetSettlements();
		while (m_aDarkSince.Count() < towns.Count())
		{
			m_aDarkSince.Insert(-1);
		}

		RZ_WorldFacts facts = m_GM.GetFacts();
		foreach (int i, RZ_Settlement s : towns)
		{
			bool dark = s.m_fInfection >= 70 && !(facts && facts.IsLivedIn(i)) && OutsideRangesBy(s.m_vCenter, 2000);
			if (!dark)
			{
				m_aDarkSince[i] = -1;
				continue;
			}

			if (m_aDarkSince[i] < 0)
				m_aDarkSince[i] = now;

			if (now - m_aDarkSince[i] < 3 || !CanFound(s.m_vCenter))
				continue;

			vector lair;
			if (!FreeLair(s.m_vCenter, 4000, lair))
				continue;

			RZ_Tribe t = m_Tribes.NewTribe(lair, 6, false);
			t.m_sAlphaWho = "someone from " + s.m_sName;
			t.AddHistory(string.Format("ran from %1 when it went quiet", s.m_sName));
			m_aDarkSince[i] = -1;
			FoundDone(t, "from " + s.m_sName + ", gone dark");
			if (m_GM.GetLeaders())
				m_GM.GetLeaders().AddHeadline(string.Format("Half of %1 went quiet this week. Not all of them stayed.", s.m_sName), false);
			return;
		}

		// From the taken
		for (int k = m_aOpenPos.Count() - 1; k >= 0; k--)
		{
			if (now - m_aOpenDay[k] > 3)
			{
				m_aOpenPos.Remove(k);
				m_aOpenDay.Remove(k);
				m_aOpenWho.Remove(k);
			}
		}

		foreach (int a, vector at : m_aOpenPos)
		{
			array<int> near = {};
			foreach (int b, vector other : m_aOpenPos)
			{
				if (RZ_Util.DistXZ(at, other) <= 1500)
					near.Insert(b);
			}

			int need = 5;
			if (InVacuum(at))
				need = 3;

			if (near.Count() < need || !CanFound(at))
				continue;

			vector lair2;
			if (!FreeLair(at, 2500, lair2))
				continue;

			RZ_Tribe t2 = m_Tribes.NewTribe(lair2, near.Count(), false);
			string first = m_aOpenWho[near[0]];
			if (Ways().IsAName(first))
				t2.m_sAlphaName = first;

			t2.m_sAlphaWho = first;
			t2.AddHistory("was among the first taken out there");
			for (int j = near.Count() - 1; j >= 0; j--)
			{
				int at2 = near[j];
				m_aOpenPos.Remove(at2);
				m_aOpenDay.Remove(at2);
				m_aOpenWho.Remove(at2);
			}

			FoundDone(t2, "from the taken");
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool OutsideRangesBy(vector pos, float margin)
	{
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (RZ_Util.DistXZ(t.RangeCentre(), pos) < t.m_fRadius + margin)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void FoundDone(RZ_Tribe t, string how)
	{
		m_iLastNewDay = RZ_SeasonClock.Day();
		t.CountRoles();
		ClearVacuumNear(t.m_vLair);
		RZ_Log.Info(string.Format("Tribes: a new pack forms at %1 (%2): %3.", t.m_sPlace, how, m_Tribes.Describe(t)));
	}

	//------------------------------------------------------------------------------------------------
	//! Someone died to the infected in the open with no tribe's range near (rule 2's taken).
	void OnOpenDeath(vector at, string who)
	{
		if (!IsOn() || Cured())
			return;

		m_aOpenPos.Insert(at);
		m_aOpenDay.Insert(RZ_SeasonClock.Days());
		m_aOpenWho.Insert(who);
		while (m_aOpenPos.Count() > 24)
		{
			m_aOpenPos.RemoveOrdered(0);
			m_aOpenDay.RemoveOrdered(0);
			m_aOpenWho.RemoveOrdered(0);
		}
	}

	// ================================================================ Splits and clans (rule 11)
	//------------------------------------------------------------------------------------------------
	//! Season day: a tribe at Split at splits at the next dusk when there's a free slot and room (a
	//! free lair candidate 1.5-3 km away outside every range, or a bordering tribe of under 10's wood,
	//! which is displaced); one split a season day. Without a slot or room it's restless.
	protected void Splits()
	{
		int at = m_Tribes.GetSettings().m_iSplitAt;
		if (at <= 0)
			return;

		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			// The Red Hand's fed tribe stays settled and doesn't split (rule 19); a held one waits
			if (t.Members() < at || t.m_bMoat || RZ_TribeContract.IsHeld(m_GM, t))
			{
				t.m_bRestless = false;
				continue;
			}

			if (t.m_bSplitDue)
				continue;

			bool slot = CanSplit();
			vector room;
			int host = -1;
			bool found = slot && FreeLair(t.m_vLair, 3000, room, null, 1500);
			if (slot && !found)
			{
				foreach (RZ_Tribe weak : m_Tribes.GetTribeList())
				{
					if (weak == t || weak.Members() >= 10 || !m_Tribes.GetBorders().Bordering(t, weak))
						continue;

					if (FreeLair(weak.m_vLair, weak.m_fRadius, room, weak, 0) && RZ_Util.DistXZ(room, t.m_vLair) >= 1500)
					{
						found = true;
						host = weak.m_iId;
						break;
					}
				}
			}

			if (!found)
			{
				if (!t.m_bRestless)
					RZ_Log.Info(string.Format("Tribes: %1 is restless (%2 members, no slot or no room to split).", t.m_sPlace, t.Members()));

				t.m_bRestless = true;
				continue;
			}

			t.m_bRestless = false;
			t.m_bSplitDue = true;
			t.m_vSplitTo = room;
			t.m_iSplitHost = host;
			m_iLastSplitDay = RZ_SeasonClock.Day();
			RZ_Log.Info(string.Format("Tribes: %1 splits at the next dusk (%2 members).", t.m_sPlace, t.Members()));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The split, at dusk: its strongest guard leads 40% of it (one guard, one scout, hunters and
	//! young in proportion) to the new lair, as the daughter's alpha (Rising). The daughter keeps the
	//! mother's look, its call with one note different, and its habits at half strength.
	protected void Split(RZ_Tribe t)
	{
		t.m_bSplitDue = false;
		if (t.Members() < m_Tribes.GetSettings().m_iSplitAt - 2)
			return;

		RZ_Tribe host = m_Tribes.Find(t.m_iSplitHost);
		if (host && !host.IsRover())
			MoveTribe(host, -1, true);

		int hunters = RZ_Util.RoundToInt(t.Count(RZ_ETribePart.HUNTERS) * 0.4);
		int young = RZ_Util.RoundToInt(t.Count(RZ_ETribePart.YOUNG) * 0.4);
		int guard = 0;
		if (t.Count(RZ_ETribePart.GUARD) >= 2)
			guard = TakeMembers(t, RZ_ETribePart.GUARD, 1);

		int scout = 0;
		if (t.Count(RZ_ETribePart.SCOUT) >= 2)
			scout = TakeMembers(t, RZ_ETribePart.SCOUT, 1);

		int h = TakeMembers(t, RZ_ETribePart.HUNTERS, hunters);
		int y = TakeMembers(t, RZ_ETribePart.YOUNG, young);
		if (guard <= 0 && h > 0)
		{
			h--;
			guard = 1;
		}

		if (guard <= 0)
		{
			Join(t, 0, h, y);
			return;
		}

		RZ_Tribe d = m_Tribes.NewTribe(t.m_vSplitTo, 1, false);
		d.m_aParts.Clear();
		d.m_iGuards = 0;
		d.m_iHunters = h;
		d.m_iYoung = y;
		if (scout <= 0 && h > 2)
		{
			h--;
			scout = 1;
		}

		d.m_iHunters = h;
		d.m_iScouts = scout;
		d.m_bHasAlpha = true;
		d.m_iMother = t.m_iId;
		d.m_eLook = t.m_eLook;
		d.m_fMilitaryShare = t.m_fMilitaryShare;
		d.m_eTemper = t.m_eTemper;
		d.m_fHunger = t.m_fHunger;
		d.m_sPattern = Ways().OneNoteDifferent(t.m_sPattern);
		foreach (RZ_TribeHabit hb : t.m_aHabits)
		{
			RZ_TribeHabit copy = new RZ_TribeHabit();
			copy.m_eKind = hb.m_eKind;
			copy.m_vPos = hb.m_vPos;
			copy.m_fHour = hb.m_fHour;
			copy.m_fStrength = hb.m_fStrength * 0.5;
			d.m_aHabits.Insert(copy);
		}

		m_Tribes.BuildParts(d);
		d.AddHistory(string.Format("led some of %1 away to its own wood", t.m_sPlace));
		t.AddHistory(string.Format("split, and some of it went to %1", d.m_sPlace));
		t.CountRoles();
		d.CountRoles();
		m_Tribes.GetBorders().Reopen(t);
		m_Tribes.GetBorders().Reopen(d);
		ClearVacuumNear(d.m_vLair);
		m_Tribes.Tell(t.m_vLair, 3000, "At dusk", string.Format("Two calls from %1 at dusk, the same call, and one of them moves off toward %2 with some of the rest.", t.m_sPlace, d.m_sPlace));
		Ways().Headline(d, string.Format("There's two of them up by %1 now. Same call, one note different.", t.m_sPlace));
		RZ_Log.Info(string.Format("Tribes: %1 split; the daughter %2 has %3.", t.m_sPlace, d.m_sPlace, d.Members()));
	}

	//------------------------------------------------------------------------------------------------
	//! Season day: a daughter and its mother, lairs within 4 km, both 8 or more for 3 season days,
	//! are one clan; a clan ends when one is left or they drift apart.
	protected void Clans()
	{
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			RZ_Tribe m = m_Tribes.Find(t.m_iMother);
			if (!m || m.IsRover() || RZ_Util.DistXZ(m.m_vLair, t.m_vLair) > CLAN_KM)
			{
				t.m_iClanHold = 0;
				if (t.m_iClan > 0 && !ClanHasOther(t))
					t.m_iClan = 0;

				continue;
			}

			if (t.Members() >= 8 && m.Members() >= 8)
				t.m_iClanHold = t.m_iClanHold + 1;
			else
				t.m_iClanHold = 0;

			if (t.m_iClanHold >= 3 && t.m_iClan == 0)
			{
				int clan = m.m_iClan;
				if (clan == 0)
					clan = m.m_iId;

				t.m_iClan = clan;
				m.m_iClan = clan;
				RZ_Log.Info(string.Format("Tribes: %1 and %2 are one clan now.", m.m_sPlace, t.m_sPlace));
			}
		}

		foreach (RZ_Tribe c : m_Tribes.GetTribeList())
		{
			if (c.m_iClan > 0 && !ClanHasOther(c))
				c.m_iClan = 0;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool ClanHasOther(RZ_Tribe t)
	{
		foreach (RZ_Tribe o : m_Tribes.GetTribeList())
		{
			if (o != t && o.m_iClan == t.m_iClan && RZ_Util.DistXZ(o.m_vLair, t.m_vLair) <= CLAN_KM)
				return true;
		}

		return false;
	}

	// ================================================================ Growing up (rule 3)
	//------------------------------------------------------------------------------------------------
	//! Season day: about a quarter of the young come of age as hunters (a young lasts about 4 season
	//! days); the guard's place goes to the oldest hunter, one a season day, up to 2 under a Rising
	//! alpha and 3 under an older one.
	protected void GrowUp(RZ_Tribe t)
	{
		int young = t.Count(RZ_ETribePart.YOUNG);
		if (young > 0)
		{
			int grown = TakeMembers(t, RZ_ETribePart.YOUNG, Math.Max(1, RZ_Util.RoundToInt(young * 0.25)));
			if (grown > 0)
				m_Tribes.GiveToRest(t, 0, grown);
		}

		int want = 3;
		if (t.Stage() == 0)
			want = 2;

		if (t.Count(RZ_ETribePart.GUARD) < want && t.Count(RZ_ETribePart.HUNTERS) > 2 && TakeMembers(t, RZ_ETribePart.HUNTERS, 1) > 0)
			Join(t, 1, 0, 0);

		t.CountRoles();
	}

	// ================================================================ Ageing and the Clash (rule 14)
	//------------------------------------------------------------------------------------------------
	//! Season day, an alpha in Decline: a challenge 15% of the time (x2 restless), the strongest
	//! guard against it; with a player within 1 km it's the Clash at the next dusk. Never challenged
	//! 15 season days into Decline, it dies at the lair (a succession).
	protected void Ageing(RZ_Tribe t)
	{
		if (!t.m_bHasAlpha || t.Stage() < 2 || t.m_bClashDue || t.m_bClash)
			return;

		float into = RZ_SeasonClock.DaysSince(t.m_fLedSince) - 21;
		float chance = 0.15;
		if (t.m_bRestless)
			chance = 0.3;

		if (t.Count(RZ_ETribePart.GUARD) > 0 && Math.RandomFloat01() < chance)
		{
			t.m_bChallenged = true;
			if (RZ_Tribes.PlayerWithin(t.m_vLair, 1000))
			{
				t.m_bClashDue = true;
				RZ_Log.Info(string.Format("Tribes: one of %1's guard will challenge its old leader at dusk (a player is near).", t.m_sPlace));
				return;
			}

			ResolveChallenge(t, false);
			return;
		}

		if (into >= 15 && !t.m_bChallenged)
			DieOfAge(t);
	}

	//------------------------------------------------------------------------------------------------
	//! The old alpha dies at the lair: the vigil, then the succession at dawn.
	protected void DieOfAge(RZ_Tribe t)
	{
		RZ_TribePart a = t.FindPart(RZ_ETribePart.ALPHA);
		t.AddHistory("died of age at the lair");
		Ways().Headline(t, string.Format("The old voice at %1 stopped calling. %2 had led them a long time.", t.m_sPlace, t.m_sAlphaName));
		RZ_Log.Info(string.Format("Tribes: %1's leader died of age.", t.m_sPlace));
		if (a && a.HasBody())
		{
			IEntity body = t.AlphaBody();
			if (body && m_GM.GetSpawner().KillSilently(body, RZ_ESilentKill.KILL_AND_MARK))
				return;
		}

		if (a)
			a.m_iCount = 0;

		m_Tribes.StartVigil(t, t.m_vLair);
	}

	//------------------------------------------------------------------------------------------------
	//! Dusk: the Clash begins if a player is still within 1 km (the tribe rings the lair at 20-40 m
	//! while the two meet, up to 3 minutes), else the challenge is settled out of sight.
	protected void StartClash(RZ_Tribe t)
	{
		t.m_bClashDue = false;
		if (!t.m_bHasAlpha || t.Count(RZ_ETribePart.GUARD) <= 0)
			return;

		if (!RZ_Tribes.PlayerWithin(t.m_vLair, 1000))
		{
			ResolveChallenge(t, false);
			return;
		}

		t.m_bClash = true;
		t.m_fClashUntil = m_Tribes.GetClock() + CLASH_SECONDS;
		m_Tribes.Tell(t.m_vLair, 1000, "At dusk", string.Format("Two voices at %1, at each other, not calling. The rest have gone quiet round them.", t.m_sPlace));
		RZ_Log.Info(string.Format("Tribes: the Clash at %1.", t.m_sPlace));
	}

	//------------------------------------------------------------------------------------------------
	//! A challenge settled: the challenger wins 50%, +5% each season day of Decline, up to 85%. The
	//! loser walks away alone as a lone one (an old alpha under 30% health dies instead); the ring
	//! follows the winner. No vigil and no splinters.
	void ResolveChallenge(RZ_Tribe t, bool staged)
	{
		t.m_bClash = false;
		if (!t.m_bHasAlpha)
			return;

		float into = Math.Max(0, RZ_SeasonClock.DaysSince(t.m_fLedSince) - 21);
		float win = Math.Min(0.85, 0.5 + 0.05 * into);
		bool challengerWins = Math.RandomFloat01() < win;
		if (t.Count(RZ_ETribePart.GUARD) <= 0)
			challengerWins = false;

		if (challengerWins)
		{
			Deposed(t, staged);
			return;
		}

		if (TakeMembers(t, RZ_ETribePart.GUARD, 1) > 0)
			AddLone(t, t.m_vLair, 0.7, "a guard who challenged and lost");

		t.AddHistory("saw off a challenge from its own guard");
		if (staged)
			m_Tribes.Tell(t.m_vLair, 1000, "At dusk", string.Format("One of them walks off from %1 alone. The old voice calls on.", t.m_sPlace));

		RZ_Log.Info(string.Format("Tribes: %1's old leader saw off a challenge.", t.m_sPlace));
	}

	//------------------------------------------------------------------------------------------------
	//! The challenger won: the old alpha walks away alone (its body, if it has one, goes with it), or
	//! dies if it's under 30% health; the guard leads now.
	protected void Deposed(RZ_Tribe t, bool staged)
	{
		string oldName = t.m_sAlphaName;
		RZ_TribePart a = t.FindPart(RZ_ETribePart.ALPHA);
		RZ_Tribe lone;
		if (t.m_fAlphaHealth >= 0.3)
			lone = AddLone(t, t.m_vLair, t.m_fAlphaHealth, "an old leader who lost to its guard");

		if (a && a.HasBody())
		{
			RZ_TribePart lp;
			if (lone)
				lp = lone.FindPart(RZ_ETribePart.ALPHA);

			RZ_TrackedZombie z = a.m_Body;
			if (lp)
			{
				// The old one's body walks away as the lone one; the new leader's comes later
				lp.m_Body = z;
				lp.m_vPos = a.m_vPos;
				z.m_Part = lp;
				z.m_iTribe = lone.m_iId;
				a.m_Body = null;
			}
			else if (z && z.m_Entity)
			{
				m_GM.GetSpawner().KillSilently(z.m_Entity, RZ_ESilentKill.KILL_AND_MARK);
			}
		}

		t.m_bHasAlpha = false;
		if (a)
		{
			t.m_aParts.RemoveItem(a);
		}

		NewAlpha(t, string.Format("took the lead from %1, its old leader", oldName));
		m_Tribes.GetBorders().Reopen(t);
		Ways().Headline(t, string.Format("%1 doesn't lead at %2 any more: its own guard saw to that. %3 does now.", oldName, t.m_sPlace, t.m_sAlphaName));
		if (staged)
			m_Tribes.Tell(t.m_vLair, 1000, "At dusk", string.Format("One of them walks off from %1 alone. The rest follow the other.", t.m_sPlace));

		RZ_Log.Info(string.Format("Tribes: %1's old leader lost to a challenger; %2 leads now.", t.m_sPlace, t.m_sAlphaName));
	}

	//------------------------------------------------------------------------------------------------
	//! A member died during the Clash: the challenger killed, the old alpha keeps the tribe a while
	//! longer; the old alpha killed, the challenger takes over with no vigil and no splinters. True
	//! if it was handled (no vigil).
	bool OnClashKill(RZ_Tribe t, RZ_EInfectedRole role)
	{
		if (!t.m_bClash)
			return false;

		if (role == RZ_EInfectedRole.GUARD)
		{
			t.m_bClash = false;
			t.AddHistory("kept the lead when its challenger was shot");
			RZ_Log.Info(string.Format("Tribes: the Clash at %1 ends: the challenger is dead.", t.m_sPlace));
			return false;
		}

		if (role != RZ_EInfectedRole.ALPHA)
			return false;

		t.m_bClash = false;
		t.m_bHasAlpha = false;
		string oldName = t.m_sAlphaName;
		NewAlpha(t, string.Format("took the lead when %1 was shot", oldName));
		m_Tribes.GetBorders().Reopen(t);
		RZ_Log.Info(string.Format("Tribes: the Clash at %1 ends: the old leader is dead, the challenger leads.", t.m_sPlace));
		return true;
	}

	// ================================================================ Disturbance and lair moves (rules 10, 17)
	//------------------------------------------------------------------------------------------------
	//! Disturbance (0-100; x1.2 bold, x0.6 territorial) by a faction (RZ_EFaction) or -1 anyone. At 100
	//! the lair moves (once a tribe a season day, two on the server).
	void Disturb(RZ_Tribe t, float amount, int faction)
	{
		if (!IsOn() || t.IsRover() || amount <= 0)
			return;

		float mult = 1;
		if (t.m_eTemper == RZ_ETemperament.BOLD)
			mult = 1.2;
		else if (t.m_eTemper == RZ_ETemperament.TERRITORIAL)
			mult = 0.6;

		float add = amount * mult;
		t.m_fDisturb = Math.Min(100, t.m_fDisturb + add);
		int idx = 4;
		if (faction >= 0 && faction < 4)
			idx = faction;

		t.m_aDisturbBy[idx] = t.m_aDisturbBy[idx] + add;
		if (t.m_fDisturb >= 100)
			TryMove(t);
	}

	//------------------------------------------------------------------------------------------------
	//! Disturbance at the lair, each played minute: a player within 150 m of it by day +2; a new
	//! remembered noise (8+ heat) within 200 m +5.
	protected void LairDisturbance(RZ_Tribe t, array<IEntity> players, float dt)
	{
		if (m_Tribes.GetPhase() == RZ_ETribePhase.DAY && RZ_Util.NearestDistance(t.m_vLair, players) <= 150)
			Disturb(t, 2 * dt / 60, -1);

		int id;
		vector at;
		if (m_GM.GetHeat().NewestNoise(t.m_iDisturbNoise, t.m_vLair, 200, 120, id, at))
		{
			t.m_iDisturbNoise = id;
			Disturb(t, 5, -1);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! At 100 disturbance: the faction that caused most of it (more than half) displaces the tribe 2 km
	//! or more from its holdings (2 a season per faction), else the lair moves within the range.
	protected void TryMove(RZ_Tribe t)
	{
		int today = RZ_SeasonClock.Day();
		if (t.m_iMovedDay == today)
			return;

		// A running job or questline stage holds its pack still (its claim, "tribe:N")
		if (RZ_TribeContract.IsHeld(m_GM, t))
			return;

		if (m_iSeasonDay != today)
			m_iMovesToday = 0;

		if (m_iMovesToday >= 2)
			return;

		float total = 0;
		int top = -1;
		foreach (int i, float v : t.m_aDisturbBy)
		{
			total += v;
			if (top < 0 || v > t.m_aDisturbBy[top])
				top = i;
		}

		int cause = -1;
		if (top >= 0 && top < 4 && t.m_aDisturbBy[top] > total * 0.5)
			cause = top;

		MoveTribe(t, cause, false);
	}

	//------------------------------------------------------------------------------------------------
	//! A lair move: within the range (400 m or more away), else the tribe moves 1.5-4 km to free wild
	//! land. Displaced by a faction: 2 km or more from that faction's holdings, never within 1.2 km of
	//! a safehouse or a holding. forced: a daughter takes its wood. The old lair is abandoned.
	void MoveTribe(RZ_Tribe t, int cause, bool forced)
	{
		t.m_iMovedDay = RZ_SeasonClock.Day();
		m_iMovesToday++;
		t.m_fDisturb = 0;
		for (int i = 0; i < t.m_aDisturbBy.Count(); i++)
		{
			t.m_aDisturbBy[i] = 0;
		}

		bool displace = forced || (cause >= 0 && m_aDisplaced[cause] < 2);
		vector lair;
		bool found = false;
		if (!displace)
			found = FreeLairIn(t, lair);

		if (!found)
		{
			found = DisplacedLair(t, cause, lair);
			if (found && cause >= 0)
				m_aDisplaced[cause] = m_aDisplaced[cause] + 1;
		}

		if (!found && displace)
			found = FreeLairIn(t, lair);

		if (!found)
		{
			RZ_Log.Info(string.Format("Tribes: %1 is disturbed but has nowhere to go.", t.m_sPlace));
			return;
		}

		bool outOfRange = RZ_Util.DistXZ(lair, t.m_vLair) > t.m_fRadius;
		string was = t.m_sPlace;
		AbandonLair(t);
		if (outOfRange && t.m_eTemper != RZ_ETemperament.NOMADIC)
			AddVacuum(t.RangeCentre(), was);

		vector from = t.m_vLair;
		RelocateLair(t, lair);
		if (outOfRange && m_Tribes.GetPeople().IsOn())
			RZ_Event_TribeMoves.Start(m_GM, t, from, t.m_vLair, was);

		string why = "disturbed";
		if (cause >= 0)
			why = "driven off by " + RZ_Factions.GetShortName(cause);

		if (forced)
			why = "pushed out by a bigger neighbour";

		t.AddHistory(string.Format("moved from %1 to %2", was, t.m_sPlace));
		m_Tribes.Tell(RZ_Util.GroundPos(lair), 2500, "On the move", string.Format("Something big is moving through the trees from %1 toward %2, a lot of them.", was, t.m_sPlace));
		RZ_Log.Info(string.Format("Tribes: %1 moves to %2 (%3).", was, t.m_sPlace, why));
	}

	//------------------------------------------------------------------------------------------------
	//! Another lair candidate in its range, 400 m or more from the lair.
	protected bool FreeLairIn(RZ_Tribe t, out vector lair)
	{
		return FreeLair(t.m_vLair, t.m_fRadius, lair, t, 400);
	}

	//------------------------------------------------------------------------------------------------
	//! A new range 1.5-4 km away in free wild land; displaced by a faction, 2 km or more from its
	//! holdings; never within 1.2 km of a safehouse or a holding.
	protected bool DisplacedLair(RZ_Tribe t, int cause, out vector lair)
	{
		array<vector> avoid = {};
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive() && cause >= 0)
		{
			foreach (RZ_Holding h : territory.GetHoldings())
			{
				if (h.m_eFaction == cause)
					avoid.Insert(h.m_vPos);
			}
		}

		RZ_WorldFacts facts = m_GM.GetFacts();
		for (int tries = 0; tries < 3; tries++)
		{
			vector cand;
			if (!FreeLair(t.m_vLair, 4000, cand, t, 1500 + tries * 500))
				return false;

			bool ok = !(facts && facts.IsNearLivedIn(cand, 1200));
			foreach (vector a : avoid)
			{
				if (RZ_Util.DistXZ(a, cand) < 2000)
					ok = false;
			}

			if (ok)
			{
				lair = cand;
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The tribe's lair is somewhere new: its place, range, trails and posts are worked out again,
	//! and its parts at the lair walk there as data (bodies walk too, as they do home).
	void RelocateLair(RZ_Tribe t, vector lair)
	{
		RZ_Land land = Land();
		t.m_vLair = lair;
		t.m_vDrift = vector.Zero;
		int idx = land.CellAt(lair);
		t.m_sLairKind = "woods";
		if (idx >= 0 && land.GetMark(idx) > 0)
			t.m_sLairKind = land.GetLairKindName(land.GetMark(idx));

		RZ_Settlement town = m_GM.GetSettlement(m_GM.FindNearestSettlement(lair));
		if (town && RZ_Util.DistXZ(town.m_vCenter, lair) <= town.m_fRadius + 100)
			t.m_sLairKind = "barns";

		t.m_sPlace = m_Tribes.PlaceName(t);
		m_Tribes.BuildPlaces(t);
		foreach (RZ_TribePart p : t.m_aParts)
		{
			float r = 40;
			switch (p.m_eKind)
			{
				case RZ_ETribePart.ALPHA: r = 0; break;
				case RZ_ETribePart.GUARD: r = 14; break;
				case RZ_ETribePart.YOUNG: r = 55; break;
			}

			p.m_vRest = RZ_Util.GroundPos(lair + RZ_Behaviour.RandomDir() * r);
			p.m_sNote = t.m_sPlace;
			if (p.m_eKind == RZ_ETribePart.SCOUT && !t.m_aPosts.IsEmpty())
			{
				p.m_iPost = Math.ClampInt(p.m_iPost, 0, t.m_aPosts.Count() - 1);
				p.m_vRest = t.m_aPosts[p.m_iPost];
			}

			if (p.m_iTarget < 0)
			{
				p.m_bOut = true;
				m_Tribes.PathHome(t, p);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Season day, a nomad: every 2-4 season days it moves to the next lair of its loop of 3-4. It
	//! leaves no vacuum and no fear places.
	protected void NomadMove(RZ_Tribe t)
	{
		if (t.m_eTemper != RZ_ETemperament.NOMADIC || RZ_SeasonClock.Days() < t.m_fNextMove || RZ_TribeContract.IsHeld(m_GM, t))
			return;

		t.m_fNextMove = RZ_SeasonClock.Days() + Math.RandomFloat(2, 4);
		if (t.m_aLoop.Count() < 2)
			BuildLoop(t);

		if (t.m_aLoop.Count() < 2)
			return;

		t.m_iLoopAt = (t.m_iLoopAt + 1) % t.m_aLoop.Count();
		vector next = t.m_aLoop[t.m_iLoopAt];
		if (IsBurned(next, false))
			return;

		string was = t.m_sPlace;
		AbandonLair(t);
		RelocateLair(t, next);
		RZ_Log.Info(string.Format("Tribes: %1 moves on round its loop, to %2.", was, t.m_sPlace));
	}

	//------------------------------------------------------------------------------------------------
	//! A nomad's loop: its lair and 2-3 lair candidates within 2 km.
	void BuildLoop(RZ_Tribe t)
	{
		t.m_aLoop.Clear();
		t.m_aLoop.Insert(t.m_vLair);
		int want = Math.RandomIntInclusive(3, 4);
		for (int i = 0; i < 6 && t.m_aLoop.Count() < want; i++)
		{
			vector c;
			if (!FreeLair(t.m_vLair + RZ_Behaviour.RandomDir() * 1200, 900, c, t, 0))
				continue;

			bool apart = true;
			foreach (vector l : t.m_aLoop)
			{
				if (RZ_Util.DistXZ(l, c) < 600)
					apart = false;
			}

			if (apart)
				t.m_aLoop.Insert(c);
		}

		t.m_iLoopAt = 0;
		if (t.m_fNextMove <= 0)
			t.m_fNextMove = RZ_SeasonClock.Days() + Math.RandomFloat(2, 4);
	}

	// ================================================================ Abandoned lairs (rule 21)
	//------------------------------------------------------------------------------------------------
	//! The old lair stays a find for 2 season days: the things of the people the tribe took (one a
	//! person, 3 at most), and a sample if a courier was among them.
	protected void AbandonLair(RZ_Tribe t)
	{
		string finds = "";
		int n = 0;
		foreach (string who : t.m_aTaken)
		{
			if (n >= 3)
				break;

			if (!finds.IsEmpty())
				finds += "|";

			finds += who;
			n++;
		}

		if (t.m_eTemper == RZ_ETemperament.NOMADIC && finds.IsEmpty())
			finds = "-";

		m_aLairPos.Insert(t.m_vLair);
		m_aLairUntil.Insert(RZ_SeasonClock.Days() + 2);
		m_aLairFinds.Insert(finds);
		while (m_aLairPos.Count() > 8)
		{
			m_aLairPos.RemoveOrdered(0);
			m_aLairUntil.RemoveOrdered(0);
			m_aLairFinds.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A player within 25 m of an abandoned lair finds it, once: what's there, and the sample.
	protected void ReadAbandonedLairs(array<int> ids, array<IEntity> players)
	{
		foreach (int i, vector pos : m_aLairPos)
		{
			foreach (int k, IEntity p : players)
			{
				if (RZ_Util.DistXZ(p.GetOrigin(), pos) > 25)
					continue;

				string key = string.Format("%1:%2", i, ids[k]);
				if (m_mLairTold.Contains(key))
					continue;

				m_mLairTold.Set(key, true);
				string finds = m_aLairFinds[i];
				if (finds == "-")
				{
					m_GM.NotifyPlayer(ids[k], "Found", "The ground's laid flat under the trees, and it's empty. Whatever lay up here has moved on. Never the same place twice.");
					continue;
				}

				if (finds.IsEmpty())
				{
					m_GM.NotifyPlayer(ids[k], "Found", "The ground's laid flat under the trees and nothing lies up here now. Whatever did has gone.");
					continue;
				}

				array<string> who = {};
				finds.Split("|", who, true);
				m_GM.NotifyPlayer(ids[k], "Found", string.Format("Somebody's things at the back of it: what's left of %1.", who[0]));
				if (finds.Contains("courier") && !finds.Contains("|taken") && !Cured())
				{
					m_aLairFinds[i] = finds + "|taken";
					m_GM.SpawnSamples(pos, 1);
					m_Tribes.OnSample(1, true);
				}
			}
		}
	}

	// ================================================================ Vacuums (rule 18)
	//------------------------------------------------------------------------------------------------
	//! A range left empty is a vacuum for 3 season days: tribes within 3 km drift their ranges 200 m
	//! into it, a new tribe forms there from 3 taken, lone ones head for it.
	void AddVacuum(vector pos, string place)
	{
		m_aVacPos.Insert(pos);
		m_aVacUntil.Insert(RZ_SeasonClock.Days() + VACUUM_DAYS);
		m_aVacPlace.Insert(place);
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			float d = RZ_Util.DistXZ(t.RangeCentre(), pos);
			if (d > 3000 || d < 1)
				continue;

			vector dir = pos - t.RangeCentre();
			dir[1] = 0;
			dir.Normalize();
			t.m_vDrift = ClampDrift(t.m_vDrift + dir * 200);
		}

		RZ_Log.Info(string.Format("Tribes: %1 is empty now (a vacuum for %2 season days).", place, VACUUM_DAYS));
	}

	//------------------------------------------------------------------------------------------------
	protected static vector ClampDrift(vector drift)
	{
		vector d = drift;
		d[1] = 0;
		float len = d.Length();
		if (len <= 500)
			return d;

		d.Normalize();
		return d * 500;
	}

	//------------------------------------------------------------------------------------------------
	bool InVacuum(vector pos)
	{
		foreach (vector v : m_aVacPos)
		{
			if (RZ_Util.DistXZ(v, pos) < 1500)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearVacuumNear(vector pos)
	{
		for (int i = m_aVacPos.Count() - 1; i >= 0; i--)
		{
			if (RZ_Util.DistXZ(m_aVacPos[i], pos) < 1500)
			{
				m_aVacPos.Remove(i);
				m_aVacUntil.Remove(i);
				m_aVacPlace.Remove(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The dusk line of a vacuum, for a player within 2.5 km of it who heard no call: "Nothing calls
	//! from the quarry by Morton tonight." "" if none.
	string VacuumLine(vector pos)
	{
		foreach (int i, vector v : m_aVacPos)
		{
			if (RZ_Util.DistXZ(v, pos) <= 2500)
				return string.Format("Nothing calls from %1 tonight. Nothing has for a while.", m_aVacPlace[i]);
		}

		return "";
	}

	// ================================================================ Seasonal pull and Winter (rule 22)
	//------------------------------------------------------------------------------------------------
	//! Season day: from Aftermath each range's centre drifts 100 m toward the nearest lived-in place,
	//! stopping at its no man's land plus P. With Winter, from Overrun, a tribe within 2.5 km of an
	//! empty village moves into the barns at its edge (they share it with the crawlers at night).
	protected void SeasonalPull(RZ_Tribe t)
	{
		RZ_EOutbreakStage stage = Stage();
		if (stage == RZ_EOutbreakStage.AFTERMATH)
		{
			vector edge;
			vector centre = t.RangeCentre();
			if (Land().NearestNoMansLand(centre, edge) && RZ_Util.DistXZ(centre, edge) > RZ_InfectedBodies.P + 100)
			{
				vector dir = edge - centre;
				dir[1] = 0;
				dir.Normalize();
				t.m_vDrift = ClampDrift(t.m_vDrift + dir * 100);
			}
		}

		if (stage < RZ_EOutbreakStage.OVERRUN || !IsWinter() || t.m_sLairKind == "barns" || t.m_iMovedDay == RZ_SeasonClock.Day())
			return;

		RZ_WorldFacts facts = m_GM.GetFacts();
		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			if (facts && facts.IsLivedIn(i))
				continue;

			if (s.m_fInfection < 40 || RZ_Util.DistXZ(s.m_vCenter, t.m_vLair) > 2500 + s.m_fRadius)
				continue;

			vector toLair = t.m_vLair - s.m_vCenter;
			toLair[1] = 0;
			if (toLair.Length() < 1)
				continue;

			toLair.Normalize();
			vector barns = RZ_Util.GroundPos(s.m_vCenter + toLair * (s.m_fRadius * 0.9));
			if (Land().InNoMansLand(barns) || !OutsideRanges(barns, t))
				continue;

			string was = t.m_sPlace;
			t.m_iMovedDay = RZ_SeasonClock.Day();
			AbandonLair(t);
			RelocateLair(t, barns);
			t.AddHistory(string.Format("moved into the barns at %1 for the cold", s.m_sName));
			RZ_Log.Info(string.Format("Tribes: %1 moves into the barns at the edge of %2 for the winter.", was, s.m_sName));
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The season's rule sets include Winter (Season > Rule sets).
	static bool IsWinter()
	{
		RZ_SeasonMods mods = RZ_Seasons.Mods();
		if (!mods)
			return false;

		foreach (string n : mods.m_aNames)
		{
			string low = n;
			low.ToLower();
			if (low == "winter")
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A line for the dusk call when its range has drifted toward people: "" if not.
	string DriftLine(RZ_Tribe t)
	{
		if (t.m_vDrift.Length() < 150 || Stage() != RZ_EOutbreakStage.AFTERMATH)
			return "";

		vector edge;
		if (!Land().NearestNoMansLand(t.RangeCentre(), edge))
			return "";

		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(edge));
		if (!s)
			return "";

		return string.Format(" They come closer to %1 every night.", s.m_sName);
	}

	// ================================================================ Burned land (rule 23)
	//------------------------------------------------------------------------------------------------
	//! A fire (the fire system): an event fire (playerId <= 0: Burning Town, a gas station) gives no
	//! lair and no trail for 3 season days; a player's fire no lair for 3 season days and no trail for
	//! one counted night, so nobody keeps a corridor open with matches. Fire within 50 m of a lair
	//! disturbs it (+60), within 30 m burns it (+100) and marks whoever set it.
	void OnFire(vector pos, int playerId)
	{
		if (!IsOn())
			return;

		bool merged = false;
		foreach (int i, vector b : m_aBurnPos)
		{
			if (RZ_Util.DistXZ(b, pos) < 100)
			{
				m_aBurnUntil[i] = RZ_SeasonClock.Days() + BURN_DAYS;
				merged = true;
			}
		}

		if (!merged)
		{
			int trailUntil = m_Tribes.CountedNights() + 1;
			if (playerId <= 0)
				trailUntil = m_Tribes.CountedNights() + 6;

			m_aBurnPos.Insert(pos);
			m_aBurnUntil.Insert(RZ_SeasonClock.Days() + BURN_DAYS);
			m_aBurnTrailUntil.Insert(trailUntil);
			while (m_aBurnPos.Count() > 40)
			{
				m_aBurnPos.RemoveOrdered(0);
				m_aBurnUntil.RemoveOrdered(0);
				m_aBurnTrailUntil.RemoveOrdered(0);
			}
		}

		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			float d = RZ_Util.DistXZ(t.m_vLair, pos);
			if (d <= 30)
			{
				t.AddHistory("was burned out of its lair");
				if (playerId > 0)
					Ways().Mark(t, playerId);

				Disturb(t, 100, -1);
			}
			else if (d <= 50)
			{
				Disturb(t, 60, -1);
			}

			if (d <= t.m_fRadius + BURN_RADIUS)
				m_Tribes.BuildTrails(t);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Burned land at pos: no lair there; with forTrail, no trail either (while it lasts for trails).
	bool IsBurned(vector pos, bool forTrail)
	{
		float now = RZ_SeasonClock.Days();
		int nights = m_Tribes.CountedNights();
		foreach (int i, vector b : m_aBurnPos)
		{
			if (RZ_Util.DistXZ(b, pos) > BURN_RADIUS)
				continue;

			if (forTrail && nights < m_aBurnTrailUntil[i])
				return true;

			if (!forTrail && now < m_aBurnUntil[i])
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Shells landed (the Fire Mission perk and contract, RZ_WorldWriters.Shell): +50 on a lair within
	//! 200 m, by Bastion.
	void OnShell(vector pos)
	{
		if (!IsOn())
			return;

		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (RZ_Util.DistXZ(t.m_vLair, pos) <= 200)
				Disturb(t, 50, RZ_EFaction.MILITARY);
		}
	}

	// ================================================================ Starvation (rule 6)
	//------------------------------------------------------------------------------------------------
	//! A counted night: a tribe starving (hunger 100) for 2 counted nights loses a fifth (at least 1)
	//! to the nearest town, as its crawlers (+2 infection each, +10 a town a season day at most), and
	//! its hunger goes to 70.
	void Starvation()
	{
		if (!IsOn() || Cured())
			return;

		array<RZ_Tribe> list = {};
		foreach (RZ_Tribe each : m_Tribes.GetTribeList())
		{
			list.Insert(each);
		}

		foreach (RZ_Tribe t : list)
		{
			if (t.m_fHunger < 100)
			{
				t.m_iStarveNights = 0;
				continue;
			}

			t.m_iStarveNights = t.m_iStarveNights + 1;
			if (t.m_iStarveNights < 2)
				continue;

			t.m_iStarveNights = 0;
			t.m_fHunger = 70;
			int want = Math.Max(1, RZ_Util.RoundToInt(t.Members() * 0.2));
			int young;
			int got = TakeMixed(t, want, young);
			if (got <= 0)
				continue;

			RZ_Settlement s = ToCrawlers(t.m_vLair, got);
			if (s)
				m_Tribes.Tell(s.m_vCenter, s.m_fRadius + 1500, "In the night", string.Format("Some of the quick ones from %1 came down into %2 in the night, and stayed with the slow ones.", t.m_sPlace, s.m_sName));

			t.AddHistory("starved, and some of it walked into the towns");
			RZ_Log.Info(string.Format("Tribes: %1 is starving: %2 walk into the nearest town.", t.m_sPlace, got));
			CheckBreakUp(t);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Members end up with the nearest town's crawlers: +2 infection each, +10 a town a season day at
	//! most (never after the cure). The town, or null.
	RZ_Settlement ToCrawlers(vector pos, int n)
	{
		if (n <= 0 || Cured())
			return null;

		int idx = m_GM.FindNearestSettlement(pos);
		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (!s)
			return null;

		while (m_aWalkersToday.Count() <= idx)
		{
			m_aWalkersToday.Insert(0);
		}

		float add = Math.Min(2 * n, 10 - m_aWalkersToday[idx]);
		if (add > 0)
		{
			s.AddInfection(add);
			m_aWalkersToday[idx] = m_aWalkersToday[idx] + add;
		}

		RZ_Log.Info(string.Format("Tribes: %1 loose ones end up with the slow ones in %2 (+%3 infection).", n, s.m_sName, RZ_Util.RoundToInt(Math.Max(0, add))));
		return s;
	}

	// ================================================================ Trucks taken as data (rule 7)
	//------------------------------------------------------------------------------------------------
	//! At night, with no player within 1 km, a hunting part within 200 m of a supply truck takes it
	//! 15% of the time, one on the server every 3 counted nights: the truck's own failure path (a
	//! shortage where it was going), its crew taken, and a road habit at that hour.
	bool TryTruck(RZ_Tribe t, RZ_TribePart p)
	{
		if (!IsOn() || m_Tribes.CountedNights() - m_iLastTruckNight < 3 || Math.RandomFloat01() > 0.15)
			return false;

		RZ_SupplyLines supply = m_GM.GetSupply();
		if (!supply || !supply.IsActive())
			return false;

		string driver;
		vector at;
		if (!supply.TakeByInfected(p.m_vPos, 200, 1000, driver, at))
			return false;

		m_iLastTruckNight = m_Tribes.CountedNights();
		m_Tribes.Taken(t, at, driver, true);
		Ways().LearnHabit(t, RZ_EHabit.ROAD, at);
		t.AddHistory(string.Format("took a supply truck near %1", RZ_SettlementScanner.GridReference(at)));
		Ways().Headline(t, string.Format("A supply truck never got where it was going. The road runs past %1.", t.m_sPlace));
		return true;
	}

	// ================================================================ Phases
	//------------------------------------------------------------------------------------------------
	//! Dusk: splits due, the Clash.
	void OnDusk()
	{
		if (!IsOn())
			return;

		array<RZ_Tribe> list = {};
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			list.Insert(t);
		}

		foreach (RZ_Tribe s : list)
		{
			if (s.m_bSplitDue)
				Split(s);

			if (s.m_bClashDue)
				StartClash(s);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The season day turns: rovers, ageing, drift, nomads, clans, break-ups, new tribes, splits,
	//! merges; vacuums, burned land, abandoned lairs and walkers' caps expire or reset.
	protected void SeasonDay()
	{
		m_iMovesToday = 0;
		m_aWalkersToday.Clear();
		float now = RZ_SeasonClock.Days();
		for (int i = m_aVacPos.Count() - 1; i >= 0; i--)
		{
			if (now >= m_aVacUntil[i])
			{
				m_aVacPos.Remove(i);
				m_aVacUntil.Remove(i);
				m_aVacPlace.Remove(i);
			}
		}

		for (int b = m_aBurnPos.Count() - 1; b >= 0; b--)
		{
			if (now >= m_aBurnUntil[b] && m_Tribes.CountedNights() >= m_aBurnTrailUntil[b])
			{
				m_aBurnPos.Remove(b);
				m_aBurnUntil.Remove(b);
				m_aBurnTrailUntil.Remove(b);
			}
		}

		for (int l = m_aLairPos.Count() - 1; l >= 0; l--)
		{
			if (now >= m_aLairUntil[l])
			{
				m_aLairPos.Remove(l);
				m_aLairUntil.Remove(l);
				m_aLairFinds.Remove(l);
				m_mLairTold.Clear();
			}
		}

		RoversDay();
		array<RZ_Tribe> list = {};
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			list.Insert(t);
		}

		foreach (RZ_Tribe tr : list)
		{
			tr.m_fDisturb = Math.Max(0, tr.m_fDisturb - 10);
			if (tr.m_fDisturb <= 0)
			{
				for (int k = 0; k < tr.m_aDisturbBy.Count(); k++)
				{
					tr.m_aDisturbBy[k] = 0;
				}
			}

			GrowUp(tr);
			Ageing(tr);
			SeasonalPull(tr);
			NomadMove(tr);
			CheckBreakUp(tr);
		}

		Clans();
		m_Tribes.GetBorders().Merges();
		NewTribes();
		Splits();
	}

	//------------------------------------------------------------------------------------------------
	protected bool Cured()
	{
		return m_GM.GetCure() && m_GM.GetCure().IsCured();
	}

	// ================================================================ Save
	//------------------------------------------------------------------------------------------------
	RZ_TribeLifeSave Save()
	{
		RZ_TribeLifeSave data = new RZ_TribeLifeSave();
		foreach (int i, vector b : m_aBurnPos)
		{
			data.burnX.Insert(b[0]);
			data.burnZ.Insert(b[2]);
			data.burnUntil.Insert(m_aBurnUntil[i]);
			data.burnTrailUntil.Insert(m_aBurnTrailUntil[i]);
		}

		foreach (int v, vector vp : m_aVacPos)
		{
			data.vacX.Insert(vp[0]);
			data.vacZ.Insert(vp[2]);
			data.vacUntil.Insert(m_aVacUntil[v]);
			data.vacPlace.Insert(m_aVacPlace[v]);
		}

		foreach (int o, vector op : m_aOpenPos)
		{
			data.openX.Insert(op[0]);
			data.openZ.Insert(op[2]);
			data.openDay.Insert(m_aOpenDay[o]);
			data.openWho.Insert(m_aOpenWho[o]);
		}

		foreach (int l, vector lp : m_aLairPos)
		{
			data.lairX.Insert(lp[0]);
			data.lairZ.Insert(lp[2]);
			data.lairUntil.Insert(m_aLairUntil[l]);
			data.lairFinds.Insert(m_aLairFinds[l]);
		}

		foreach (float d : m_aDarkSince)
		{
			data.darkSince.Insert(d);
		}

		foreach (int n : m_aDisplaced)
		{
			data.displaced.Insert(n);
		}

		data.lastTruckNight = m_iLastTruckNight;
		data.lastSplitDay = m_iLastSplitDay;
		data.lastNewDay = m_iLastNewDay;
		return data;
	}

	//------------------------------------------------------------------------------------------------
	void Restore(RZ_TribeLifeSave data)
	{
		if (!data)
			return;

		for (int i = 0; i < data.burnX.Count() && i < data.burnUntil.Count() && i < data.burnTrailUntil.Count(); i++)
		{
			m_aBurnPos.Insert(RZ_Util.GroundPos(Vector(data.burnX[i], 0, data.burnZ[i])));
			m_aBurnUntil.Insert(data.burnUntil[i]);
			m_aBurnTrailUntil.Insert(data.burnTrailUntil[i]);
		}

		for (int v = 0; v < data.vacX.Count() && v < data.vacUntil.Count() && v < data.vacPlace.Count(); v++)
		{
			m_aVacPos.Insert(RZ_Util.GroundPos(Vector(data.vacX[v], 0, data.vacZ[v])));
			m_aVacUntil.Insert(data.vacUntil[v]);
			m_aVacPlace.Insert(data.vacPlace[v]);
		}

		for (int o = 0; o < data.openX.Count() && o < data.openDay.Count() && o < data.openWho.Count(); o++)
		{
			m_aOpenPos.Insert(RZ_Util.GroundPos(Vector(data.openX[o], 0, data.openZ[o])));
			m_aOpenDay.Insert(data.openDay[o]);
			m_aOpenWho.Insert(data.openWho[o]);
		}

		for (int l = 0; l < data.lairX.Count() && l < data.lairUntil.Count() && l < data.lairFinds.Count(); l++)
		{
			m_aLairPos.Insert(RZ_Util.GroundPos(Vector(data.lairX[l], 0, data.lairZ[l])));
			m_aLairUntil.Insert(data.lairUntil[l]);
			m_aLairFinds.Insert(data.lairFinds[l]);
		}

		foreach (float d : data.darkSince)
		{
			m_aDarkSince.Insert(d);
		}

		for (int f = 0; f < data.displaced.Count() && f < m_aDisplaced.Count(); f++)
		{
			m_aDisplaced[f] = data.displaced[f];
		}

		m_iLastTruckNight = data.lastTruckNight;
		m_iLastSplitDay = data.lastSplitDay;
		m_iLastNewDay = data.lastNewDay;
	}

	//------------------------------------------------------------------------------------------------
	//! "rovers: lone 1, splinters 2; vacuums 1, burned 3, abandoned lairs 1"
	string StatusText()
	{
		return string.Format("rovers: lone %1, splinters %2; vacuums %3, burned %4, abandoned lairs %5, open taken %6", CountRovers(RZ_ETribeKind.LONE), CountRovers(RZ_ETribeKind.SPLINTER), m_aVacPos.Count(), m_aBurnPos.Count(), m_aLairPos.Count(), m_aOpenPos.Count());
	}
}
