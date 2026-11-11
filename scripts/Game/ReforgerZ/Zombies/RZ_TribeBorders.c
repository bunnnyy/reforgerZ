//! Borders between tribes (docs/INFECTED.md, "Borders"): who borders whom (ranges within 150 m with
//! no river or valley floor between, or meeting at a crossing), the calling contest each dusk (the
//! border moves 50 m toward the weaker, 300 m at most), old and reopened borders, fights on counted
//! nights (a Border Night with a player within 800 m: heard, then found), absorption and merging.
//! Clans never fight. Owned by RZ_Tribes. Server only.

//------------------------------------------------------------------------------------------------
class RZ_Border
{
	int m_iA;
	int m_iB;
	vector m_vAt;				// the border point: a crossing, or between the two ranges
	bool m_bCrossing;
	float m_fOffset;			// m the border has moved toward B (negative: toward A), 300 at most
	float m_fLastFight;			// season days
	int m_iReopenUntil = -1;	// counted night: a succession or a split on either side reopened it
	int m_iFoughtNight = -1;	// the night it was last rolled for
	bool m_bStaged;				// a Border Night now
	float m_fStagedUntil;		// the tribe clock
	int m_iLostA;				// A's and B's losses tonight when it was staged
	int m_iLostB;
}

//------------------------------------------------------------------------------------------------
class RZ_TribeBorders
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_Tribes m_Tribes;		// weak: the owner
	protected ref array<ref RZ_Border> m_aBorders = {};
	protected ref array<int> m_aAbsorbLoser = {};
	protected ref array<int> m_aAbsorbWinner = {};
	protected int m_iStagedNight = -1;
	protected int m_iStagedTonight;
	protected int m_iFights;
	protected int m_iStaged;
	protected bool m_bFresh;			// restored from the save: the border points are worked out again first

	static const float BORDER_GAP = 150;
	static const float STAGE_RANGE = 800;

	//------------------------------------------------------------------------------------------------
	void RZ_TribeBorders(RZ_GameModeComponent gm, RZ_Tribes tribes)
	{
		m_GM = gm;
		m_Tribes = tribes;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsOn()
	{
		return m_Tribes.GetSettings().m_bLifeCycle;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Border FindBorder(int a, int b)
	{
		foreach (RZ_Border br : m_aBorders)
		{
			if ((br.m_iA == a && br.m_iB == b) || (br.m_iA == b && br.m_iB == a))
				return br;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	bool Bordering(RZ_Tribe a, RZ_Tribe b)
	{
		return FindBorder(a.m_iId, b.m_iId) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Old: no fight for 5 season days, and not reopened.
	bool IsOld(RZ_Border br)
	{
		return RZ_SeasonClock.DaysSince(br.m_fLastFight) >= 5 && m_Tribes.CountedNights() >= br.m_iReopenUntil;
	}

	//------------------------------------------------------------------------------------------------
	bool IsReopened(RZ_Border br)
	{
		return m_Tribes.CountedNights() < br.m_iReopenUntil;
	}

	//------------------------------------------------------------------------------------------------
	//! Who borders whom, worked out again (cheap: 105 pairs on a big map). Known pairs keep their
	//! offset, last fight and reopening.
	void Rebuild()
	{
		RZ_Land land = m_GM.GetLand();
		array<ref RZ_Tribe> tribes = m_Tribes.GetTribeList();
		array<ref RZ_Border> next = {};
		for (int i = 0; i < tribes.Count(); i++)
		{
			for (int j = i + 1; j < tribes.Count(); j++)
			{
				RZ_Tribe a = tribes[i];
				RZ_Tribe b = tribes[j];
				vector at;
				bool crossing;
				if (!Meet(land, a, b, at, crossing))
					continue;

				RZ_Border br = FindBorder(a.m_iId, b.m_iId);
				if (!br)
				{
					br = new RZ_Border();
					br.m_iA = a.m_iId;
					br.m_iB = b.m_iId;
					br.m_fLastFight = RZ_SeasonClock.Days();
				}

				br.m_vAt = at;
				br.m_bCrossing = crossing;
				next.Insert(br);
			}
		}

		m_aBorders.Clear();
		foreach (RZ_Border keep : next)
		{
			m_aBorders.Insert(keep);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Do two ranges meet? At a crossing within both (the bridge is the border), or within 150 m of
	//! each other with no river and no valley floor between (sound carries badly over them).
	protected bool Meet(RZ_Land land, RZ_Tribe a, RZ_Tribe b, out vector at, out bool crossing)
	{
		vector ca = a.RangeCentre();
		vector cb = b.RangeCentre();
		float d = RZ_Util.DistXZ(ca, cb);
		if (d > a.m_fRadius + b.m_fRadius + 1000)
			return false;

		for (int c = 0; c < land.GetCrossingCount(); c++)
		{
			vector cp = land.GetCrossing(c);
			if (RZ_Util.DistXZ(cp, ca) <= a.m_fRadius + 300 && RZ_Util.DistXZ(cp, cb) <= b.m_fRadius + 300)
			{
				at = RZ_Util.GroundPos(cp);
				crossing = true;
				return true;
			}
		}

		if (d - a.m_fRadius - b.m_fRadius > BORDER_GAP || d < 1)
			return false;

		vector dir = cb - ca;
		dir[1] = 0;
		dir.Normalize();
		vector from = ca + dir * Math.Max(0, a.m_fRadius - 200);
		vector to = cb - dir * Math.Max(0, b.m_fRadius - 200);
		float len = RZ_Util.DistXZ(from, to);
		int steps = Math.ClampInt(len / RZ_Land.CELL, 1, 12);
		for (int s = 0; s <= steps; s++)
		{
			float f = s;
			vector p = from + (to - from) * (f / steps);
			int idx = land.CellAt(p);
			if (idx < 0)
				continue;

			if (land.HasBit(idx, RZ_Land.BIT_VALLEY))
				return false;

			if (land.HasBit(idx, RZ_Land.BIT_RIVER) && !land.HasBit(idx, RZ_Land.BIT_BRIDGE) && !land.HasBit(idx, RZ_Land.BIT_FORD))
				return false;
		}

		float mid = a.m_fRadius + (d - a.m_fRadius - b.m_fRadius) * 0.5;
		at = RZ_Util.GroundPos(ca + dir * mid);
		crossing = false;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A succession or a split on either side: its borders reopen for 2 counted nights ("a new voice
	//! testing the river"), fights twice as likely.
	void Reopen(RZ_Tribe t)
	{
		foreach (RZ_Border br : m_aBorders)
		{
			if (br.m_iA == t.m_iId || br.m_iB == t.m_iId)
				br.m_iReopenUntil = m_Tribes.CountedNights() + 2;
		}
	}

	//------------------------------------------------------------------------------------------------
	void Forget(int id)
	{
		for (int i = m_aBorders.Count() - 1; i >= 0; i--)
		{
			if (m_aBorders[i].m_iA == id || m_aBorders[i].m_iB == id)
				m_aBorders.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Strength: (hunters + guard) x the alpha's stage (Rising 0.8, Prime 1.2, Decline 0.9; no alpha
	//! 0.6), x1.3 for a territorial tribe on its own side.
	static float Strength(RZ_Tribe t)
	{
		float s = t.Count(RZ_ETribePart.HUNTERS) + t.Count(RZ_ETribePart.GUARD);
		float mult = 0.6;
		if (t.m_bHasAlpha)
		{
			switch (t.Stage())
			{
				case 0: mult = 0.8; break;
				case 1: mult = 1.2; break;
				case 2: mult = 0.9; break;
			}
		}

		if (t.m_eTemper == RZ_ETemperament.TERRITORIAL)
			mult = mult * 1.3;

		return s * mult;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool SameClan(RZ_Tribe a, RZ_Tribe b)
	{
		return a.m_iClan > 0 && a.m_iClan == b.m_iClan;
	}

	//------------------------------------------------------------------------------------------------
	protected static string What(RZ_Border br, RZ_Land land)
	{
		if (!br.m_bCrossing)
			return "the ridge";

		int idx = land.CellAt(br.m_vAt);
		if (idx >= 0 && land.HasBit(idx, RZ_Land.BIT_FORD))
			return "the ford";

		return "the bridge";
	}

	// ================================================================ Dusk: calling contests
	//------------------------------------------------------------------------------------------------
	//! Each dusk, bordering tribes call against each other: the border moves 50 m toward the weaker,
	//! 300 m from where it started at most. A Radio Operator within 3 km hears who answered weaker.
	void Contests(array<int> ids, array<IEntity> players)
	{
		if (!IsOn())
			return;

		Rebuild();
		RZ_Land land = m_GM.GetLand();
		map<int, bool> told = new map<int, bool>();
		foreach (RZ_Border br : m_aBorders)
		{
			RZ_Tribe a = m_Tribes.Find(br.m_iA);
			RZ_Tribe b = m_Tribes.Find(br.m_iB);
			if (!a || !b || SameClan(a, b))
				continue;

			RZ_Tribe weaker = a;
			if (Strength(b) < Strength(a))
			{
				weaker = b;
				br.m_fOffset = Math.Clamp(br.m_fOffset + 50, -300, 300);
			}
			else
			{
				br.m_fOffset = Math.Clamp(br.m_fOffset - 50, -300, 300);
			}

			foreach (int i, IEntity p : players)
			{
				int id = ids[i];
				if (told.Contains(id) || RZ_Util.DistXZ(p.GetOrigin(), br.m_vAt) > 3000)
					continue;

				RZ_Background bg = m_GM.GetPlayerBackground(id);
				if (!bg || bg.m_sId != "radioham")
					continue;

				told.Set(id, true);
				m_GM.NotifyPlayer(id, "On the radio", string.Format("Calls across %1 at dusk. %2 answered weaker tonight.", What(br, land), RZ_Util.Capitalize(weaker.m_sPlace)));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! What the dusk call adds about a border: " The woods by Levie answer from across the bridge."
	//! (quieter on an old border; a new voice testing it when reopened; a clan answers in the same
	//! call, one note different). "" with no border.
	string AnswerLine(RZ_Tribe t)
	{
		if (!IsOn())
			return "";

		if (m_bFresh)
		{
			m_bFresh = false;
			Rebuild();
		}

		RZ_Land land = m_GM.GetLand();
		foreach (RZ_Border br : m_aBorders)
		{
			int other = -1;
			if (br.m_iA == t.m_iId)
				other = br.m_iB;
			else if (br.m_iB == t.m_iId)
				other = br.m_iA;

			RZ_Tribe o = m_Tribes.Find(other);
			if (!o || o.m_bFed)
				continue;

			if (SameClan(t, o))
				return string.Format(" The same call answers from %1, one note different.", o.m_sPlace);

			if (IsReopened(br))
				return string.Format(" From %1, a new voice is testing %2.", o.m_sPlace, What(br, land));

			if (IsOld(br))
				return string.Format(" %1 answers quietly from across %2.", RZ_Util.Capitalize(o.m_sPlace), What(br, land));

			return string.Format(" %1 answers from across %2.", RZ_Util.Capitalize(o.m_sPlace), What(br, land));
		}

		return "";
	}

	// ================================================================ Night: fights
	//------------------------------------------------------------------------------------------------
	//! Once a counted night per border, when both have set out: a fight if either is hungry or worse,
	//! it's a crossing, or it reopened (about 20% of such nights, as both hunt toward it; half on an
	//! old border, twice on a reopened one or with a restless tribe). With a player within 800 m it's
	//! staged as a Border Night (2 on the server a night).
	void NightTick()
	{
		if (!IsOn() || m_Tribes.GetPhase() != RZ_ETribePhase.NIGHT)
			return;

		if (m_bFresh)
		{
			m_bFresh = false;
			Rebuild();
		}

		int night = m_Tribes.CountedNights();
		if (m_iStagedNight != night)
		{
			m_iStagedNight = night;
			m_iStagedTonight = 0;
		}

		float clock = m_Tribes.GetClock();
		foreach (RZ_Border br : m_aBorders)
		{
			if (br.m_bStaged)
			{
				if (clock >= br.m_fStagedUntil)
					Resolve(br, true);

				continue;
			}

			if (br.m_iFoughtNight == night)
				continue;

			RZ_Tribe a = m_Tribes.Find(br.m_iA);
			RZ_Tribe b = m_Tribes.Find(br.m_iB);
			if (!a || !b || !a.m_bHunted || !b.m_bHunted || a.m_bVigil || b.m_bVigil || SameClan(a, b))
				continue;

			br.m_iFoughtNight = night;
			bool cause = a.Band() >= 2 || b.Band() >= 2 || br.m_bCrossing || IsReopened(br);
			if (!cause)
				continue;

			float chance = 0.2;
			if (IsOld(br))
				chance = chance * 0.5;

			if (IsReopened(br))
				chance = chance * 2;

			if (a.m_bRestless || b.m_bRestless)
				chance = chance * 2;

			if (Math.RandomFloat01() >= Math.Min(0.8, chance))
				continue;

			m_iFights++;
			if (m_iStagedTonight < 2 && RZ_Tribes.PlayerWithin(br.m_vAt, STAGE_RANGE))
			{
				Stage(br, a, b);
				continue;
			}

			Resolve(br, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Where two bordering packs meet (Bait the Crew). False if they don't border.
	bool BorderPoint(RZ_Tribe a, RZ_Tribe b, out vector at)
	{
		RZ_Border br = FindBorder(a.m_iId, b.m_iId);
		if (!br)
			return false;

		at = br.m_vAt;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Bait the Crew: noise on the line brings one side's hunters over, a Border Night with the
	//! player in it (within the night's 2 staged fights). False if it can't be staged now.
	bool Provoke(RZ_Tribe a, RZ_Tribe b)
	{
		RZ_Border br = FindBorder(a.m_iId, b.m_iId);
		if (!br || br.m_bStaged || m_iStagedTonight >= 2 || a.m_bVigil || b.m_bVigil)
			return false;

		br.m_iFoughtNight = m_Tribes.CountedNights();
		m_iFights++;
		Stage(br, a, b);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A Border Night: both sides' hunter groups go to the border point and mill there with calls
	//! and shrieks, 5-20 minutes, then it's settled.
	protected void Stage(RZ_Border br, RZ_Tribe a, RZ_Tribe b)
	{
		br.m_bStaged = true;
		br.m_fStagedUntil = m_Tribes.GetClock() + Math.RandomFloat(300, 1200);
		br.m_iLostA = a.m_iLostTonight;
		br.m_iLostB = b.m_iLostTonight;
		m_iStagedTonight++;
		m_iStaged++;
		array<RZ_Tribe> sides = {a, b};
		foreach (RZ_Tribe t : sides)
		{
			foreach (RZ_TribePart p : t.m_aParts)
			{
				if (p.m_eKind != RZ_ETribePart.HUNTERS || p.m_iTarget >= 0 || p.m_iCount <= 0)
					continue;

				array<vector> path = {p.m_vPos, RZ_Util.GroundPos(RZ_Util.RandomPointInRing(br.m_vAt, 5, 35))};
				p.SetPath(path);
				p.m_iStep = 1;
				p.m_bOut = true;
				p.m_bHome = false;
				p.m_bWaits = true;
				p.m_fHoldUntil = m_Tribes.GetClock();
			}
		}

		StirNear(br.m_vAt);
		string what = What(br, m_GM.GetLand());
		m_Tribes.Tell(br.m_vAt, 1500, "In the dark", string.Format("Calls from both sides of %1, near %2. Then a great deal of shrieking.", what, a.m_sPlace));
		RZ_Log.Info(string.Format("Tribes: a Border Night at %1 between %2 and %3.", RZ_SettlementScanner.GridReference(br.m_vAt), a.m_sPlace, b.m_sPlace));
	}

	//------------------------------------------------------------------------------------------------
	//! Towns within 1 km of a border fight stir (+20).
	protected void StirNear(vector at)
	{
		RZ_Crawlers crawlers = m_GM.GetSpawner().GetCrawlers();
		if (!crawlers || !crawlers.IsEnabled())
			return;

		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(at));
		if (s && RZ_Util.DistXZ(s.m_vCenter, at) <= s.m_fRadius + 1000)
			crawlers.AddStir(at, 20);
	}

	//------------------------------------------------------------------------------------------------
	//! The fight is settled: the weaker loses (in a Border Night, the side players thinned the more).
	//! The loser loses 2-4 (found dead at the spot if nobody is within 150 m, else they just don't
	//! come home), won't hunt toward that border for 3 counted nights; the winner's hunger falls 10;
	//! both hunt short the next night. A loser under 8 with no alpha, or a Declining one, is absorbed.
	protected void Resolve(RZ_Border br, bool staged)
	{
		br.m_bStaged = false;
		br.m_fLastFight = RZ_SeasonClock.Days();
		RZ_Tribe a = m_Tribes.Find(br.m_iA);
		RZ_Tribe b = m_Tribes.Find(br.m_iB);
		if (!a || !b)
			return;

		float sa = Strength(a) * Math.RandomFloat(0.8, 1.2);
		float sb = Strength(b) * Math.RandomFloat(0.8, 1.2);
		RZ_Tribe loser = a;
		RZ_Tribe winner = b;
		if (sb < sa)
		{
			loser = b;
			winner = a;
		}

		if (staged)
		{
			int lostA = a.m_iLostTonight - br.m_iLostA;
			int lostB = b.m_iLostTonight - br.m_iLostB;
			if (lostA >= lostB + 2)
			{
				loser = a;
				winner = b;
			}
			else if (lostB >= lostA + 2)
			{
				loser = b;
				winner = a;
			}
		}

		int losses = Math.RandomIntInclusive(2, 4);
		int dead = 0;
		if (staged && !RZ_Tribes.PlayerWithin(br.m_vAt, 150))
			dead = KillAtBorder(loser, br.m_vAt, losses);

		if (dead < losses)
		{
			int young;
			m_Tribes.GetLife().TakeMixed(loser, losses - dead, young);
		}

		loser.m_iNoHuntToward = winner.m_iId;
		loser.m_iNoHuntUntil = m_Tribes.CountedNights() + 3;
		winner.m_fHunger = Math.Max(0, winner.m_fHunger - 10);
		winner.m_bShortNight = true;
		loser.m_bShortNight = true;
		string what = What(br, m_GM.GetLand());
		winner.AddHistory(string.Format("won %1 off %2", what, loser.m_sPlace));
		if (!staged)
			StirNear(br.m_vAt);

		// Both sides' groups at the border go home
		array<RZ_Tribe> sides = {a, b};
		foreach (RZ_Tribe t : sides)
		{
			foreach (RZ_TribePart p : t.m_aParts)
			{
				if (p.m_bWaits)
				{
					p.m_bWaits = false;
					m_Tribes.PathHome(t, p);
				}
			}
		}

		RZ_TribeContract.OnBorder(m_GM, winner, loser);
		RZ_Log.Info(string.Format("Tribes: %1 won a border fight at %2 against %3 (%4 lost, %5 found dead).", winner.m_sPlace, RZ_SettlementScanner.GridReference(br.m_vAt), loser.m_sPlace, losses, dead));
		if (loser.Members() < 8 && (!loser.m_bHasAlpha || loser.Stage() == 2) && m_aAbsorbLoser.Find(loser.m_iId) < 0)
		{
			m_aAbsorbLoser.Insert(loser.m_iId);
			m_aAbsorbWinner.Insert(winner.m_iId);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The loser's bodies at the border are killed where they stand, so a player who walks up finds
	//! them (the silent kill's kill-and-mark: no samples, no credit). How many.
	protected int KillAtBorder(RZ_Tribe loser, vector at, int n)
	{
		int dead = 0;
		RZ_ZombieSpawner spawner = m_GM.GetSpawner();
		foreach (RZ_TribePart p : loser.m_aParts)
		{
			if (dead >= n)
				break;

			if (!p.HasBody() || p.IsCore() || RZ_Util.DistXZ(p.m_vPos, at) > 150)
				continue;

			array<IEntity> members = {};
			RZ_Util.GetUnitMembers(p.m_Body.m_Entity, members);
			foreach (IEntity m : members)
			{
				if (dead >= n)
					break;

				if (RZ_Util.IsCharacterAlive(m) && spawner.KillSilently(m, RZ_ESilentKill.KILL_AND_MARK))
					dead++;
			}
		}

		return dead;
	}

	// ================================================================ Absorbing and merging (rule 13)
	//------------------------------------------------------------------------------------------------
	//! Season day: a beaten tribe under 8 with no alpha, or a Declining one, joins its winner (up to
	//! 30; 30% splinter off instead); two bordering tribes under 10, one with no alpha, merge under the
	//! other's. Either way a slot frees, and a Radio Operator hears one call where there were two.
	void Merges()
	{
		if (!IsOn())
			return;

		for (int i = m_aAbsorbLoser.Count() - 1; i >= 0; i--)
		{
			RZ_Tribe loser = m_Tribes.Find(m_aAbsorbLoser[i]);
			RZ_Tribe winner = m_Tribes.Find(m_aAbsorbWinner[i]);
			m_aAbsorbLoser.Remove(i);
			m_aAbsorbWinner.Remove(i);
			if (!loser || !winner || loser.IsRover() || winner.IsRover() || HasBodies(loser))
				continue;

			if (RZ_TribeContract.IsHeld(m_GM, loser) || RZ_TribeContract.IsHeld(m_GM, winner))
				continue;

			if (loser.Members() >= 8 || (loser.m_bHasAlpha && loser.Stage() < 2))
				continue;

			Absorb(winner, loser, "absorbed");
		}

		Rebuild();
		foreach (RZ_Border br : m_aBorders)
		{
			RZ_Tribe a = m_Tribes.Find(br.m_iA);
			RZ_Tribe b = m_Tribes.Find(br.m_iB);
			if (!a || !b || a.Members() >= 10 || b.Members() >= 10 || HasBodies(a) || HasBodies(b))
				continue;

			if (RZ_TribeContract.IsHeld(m_GM, a) || RZ_TribeContract.IsHeld(m_GM, b))
				continue;

			if (a.m_bHasAlpha == b.m_bHasAlpha)
				continue;

			if (a.m_bHasAlpha)
				Absorb(a, b, "merged");
			else
				Absorb(b, a, "merged");

			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static bool HasBodies(RZ_Tribe t)
	{
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (p.HasBody())
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void Absorb(RZ_Tribe winner, RZ_Tribe loser, string how)
	{
		RZ_TribeLife life = m_Tribes.GetLife();
		int n = loser.Members();
		if (loser.m_bHasAlpha)
			n--;

		int off = RZ_Util.RoundToInt(n * 0.3);
		if (how == "absorbed" && off >= 3)
			life.Splinter(loser, off, loser.m_vLair);

		if (loser.m_bHasAlpha)
			life.AddLone(loser, loser.m_vLair, loser.m_fAlphaHealth, "the old leader of a pack that was beaten");

		life.Join(winner, loser.Count(RZ_ETribePart.GUARD), loser.Count(RZ_ETribePart.HUNTERS) + loser.Count(RZ_ETribePart.SCOUT), loser.Count(RZ_ETribePart.YOUNG));
		winner.AddHistory(string.Format("took in what was left of %1", loser.m_sPlace));
		m_Tribes.Tell(winner.m_vLair, 3000, "At dusk", string.Format("One call from %1 tonight, where there used to be two.", winner.m_sPlace));
		RZ_Log.Info(string.Format("Tribes: %1 %2 %3; its slot is free.", winner.m_sPlace, how, loser.m_sPlace));
		life.RemoveTribe(loser, false);
	}

	// ================================================================ Save
	//------------------------------------------------------------------------------------------------
	void Save(array<ref RZ_BorderSave> into)
	{
		foreach (RZ_Border br : m_aBorders)
		{
			RZ_BorderSave s = new RZ_BorderSave();
			s.a = br.m_iA;
			s.b = br.m_iB;
			s.offset = br.m_fOffset;
			s.lastFight = br.m_fLastFight;
			s.reopenUntil = br.m_iReopenUntil;
			into.Insert(s);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Restore(array<ref RZ_BorderSave> from)
	{
		if (!from)
			return;

		foreach (RZ_BorderSave s : from)
		{
			RZ_Border br = new RZ_Border();
			br.m_iA = s.a;
			br.m_iB = s.b;
			br.m_fOffset = s.offset;
			br.m_fLastFight = s.lastFight;
			br.m_iReopenUntil = s.reopenUntil;
			m_aBorders.Insert(br);
		}

		m_bFresh = true;
	}

	//------------------------------------------------------------------------------------------------
	//! "borders 3 (old 1), fights 2 (staged 1)"
	string StatusText()
	{
		int old = 0;
		foreach (RZ_Border br : m_aBorders)
		{
			if (IsOld(br))
				old++;
		}

		return string.Format("borders %1 (old %2), fights %3 (staged %4)", m_aBorders.Count(), old, m_iFights, m_iStaged);
	}
}
