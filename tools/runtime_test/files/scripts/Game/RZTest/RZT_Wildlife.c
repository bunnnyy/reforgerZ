// Test-only (not part of the mod): wildlife's herds (World/RZ_Wildlife.c; docs/DESIGN.md, "Wildlife").
// Once, on the fake player, in one go (with no real player on the test server, the game mode's own
// tick puts every herd away at once):
// - The rates: the settings' defaults (4 herds, a 1-in-2 chance a minute, 8 minutes a player, 80 m
//   from a town's edge, 6 deer a minute that leave meat).
// - Putting out: in open country a herd comes 150 to 300 m from the tester, of two or three, in the
//   country; up to four herds, never a fifth.
// - Meat: a believed kill lays the herd's meat beside the deer.
// - Putting away: with the tester far off, every herd and its deer go.
// WildlifeExpansion isn't loaded on the Linux server (MOD_STACK.md, "Wildlife"), so a wooden box stands
// in for the doe and the buck; the herd rules are the same whatever the prefab.
// Lines: "RZT wildlife OK|FAIL <what>", "RZT wildlife skipped: <why>" and "RZT wildlife: N checks, M failed".

class RZT_Wildlife
{
	static bool s_bDone;
	static int s_iChecks;
	static int s_iFails;
	static const ResourceName STAND_IN = "{2B1D0128ED660BE9}Prefabs/Props/Crates/BoxWooden_01.et";

	//------------------------------------------------------------------------------------------------
	static void Check(bool ok, string what)
	{
		s_iChecks++;
		if (!ok)
			s_iFails++;

		string verdict = "OK";
		if (!ok)
			verdict = "FAIL";

		Print(string.Format("RZT wildlife %1 %2", verdict, what));
	}

	//------------------------------------------------------------------------------------------------
	static void Tick(RZ_GameModeComponent gm)
	{
		if (s_bDone || !RZT_Fake.s_Ent)
			return;

		s_bDone = true;
		RZ_Wildlife w = gm.GetWildlife();
		RZ_WildlifeSettings s = gm.GetWildlifeSettings();
		if (!w || !s)
		{
			Print("RZT wildlife skipped: no wildlife on the game mode");
			return;
		}

		if (!s.m_bEnabled)
		{
			Print("RZT wildlife skipped: Wildlife is off (the Features switch or its own setting)");
			return;
		}

		// 1. The rates
		Check(s.m_iMaxHerds == 4, string.Format("four herds on the server (%1)", s.m_iMaxHerds));
		Check(s.m_fChancePerMinute > 0.49 && s.m_fChancePerMinute < 0.51, string.Format("a 1-in-2 chance a minute (%1)", s.m_fChancePerMinute));
		Check(s.m_fPlayerMinutes > 7.9 && s.m_fPlayerMinutes < 8.1, string.Format("8 minutes a player (%1)", s.m_fPlayerMinutes));
		Check(s.m_fTownMargin > 79 && s.m_fTownMargin < 81, string.Format("80 m from a town's edge (%1)", s.m_fTownMargin));
		Check(s.m_iMeatPerMinute == 6, string.Format("6 deer a minute leave meat (%1)", s.m_iMeatPerMinute));

		// What's put back after
		IEntity me = RZT_Fake.s_Ent;
		vector home = me.GetOrigin();
		ResourceName doe = s.m_sDoePrefab;
		ResourceName buck = s.m_sBuckPrefab;
		s.m_sDoePrefab = STAND_IN;
		s.m_sBuckPrefab = STAND_IN;
		w.RZT_Force();

		// 2. Four country spots far apart, a herd at each
		array<vector> spots = {};
		w.RZT_CountrySpots(gm, 4, spots);
		Check(spots.Count() == 4, string.Format("four open-country spots on the map (%1)", spots.Count()));
		array<IEntity> players = {};
		players.Insert(me);
		int put = 0;
		foreach (vector spot : spots)
		{
			RZT_Fake.MoveTo(spot);
			vector from = me.GetOrigin();
			if (!w.RZT_PutOut(RZT_Fake.ID, me, players))
				continue;

			put++;
			vector at = w.RZT_LastHerdPos();
			float d = vector.DistanceXZ(at, from);
			int n = w.RZT_LastHerdSize();
			Check(d >= 149 && d <= 301, string.Format("herd %1 is 150 to 300 m off (%2 m)", put, RZ_Util.RoundToInt(d)));
			Check(n >= 2 && n <= 4, string.Format("herd %1 has two or three and maybe a buck (%2)", put, n));
			Check(w.RZT_IsCountry(at), string.Format("herd %1 is in the country", put));
		}

		Check(put == spots.Count() && put > 0, string.Format("a herd at every spot (%1 of %2)", put, spots.Count()));
		Check(w.CountHerds() == put, string.Format("the herds are counted (%1)", w.CountHerds()));

		// The cap: a fifth isn't put out by the minute's check, however sure the chance
		if (put == 4 && !spots.IsEmpty())
		{
			RZT_Fake.MoveTo(spots[0]);
			float chance = s.m_fChancePerMinute;
			s.m_fChancePerMinute = 1;
			// The herds' own deer stand in as the players near them, so none is put away first; the
			// tester's id goes with the first deer, which stands in the country
			array<IEntity> near = {};
			w.RZT_AllDeer(near);
			array<int> ids = {};
			ids.Insert(RZT_Fake.ID);
			w.RZT_ForgetTimers();
			w.Tick(61, ids, near);
			s.m_fChancePerMinute = chance;
			Check(w.CountHerds() == 4, string.Format("never a fifth herd (%1)", w.CountHerds()));
		}

		// 3. Meat beside a shot deer
		if (put > 0)
		{
			IEntity deer = w.RZT_FirstDeer();
			Check(deer != null, "the first herd has a deer standing");
			if (deer)
			{
				int laid = w.RZT_LayMeat(RZT_Fake.ID, deer.GetOrigin());
				Check(laid >= s.m_iMeatMin && laid >= 1 && laid <= s.m_iMeatMax, string.Format("a shot deer leaves %1 to %2 meat (%3)", s.m_iMeatMin, s.m_iMeatMax, laid));
			}
		}

		// 4. Put away once everyone is far off
		array<IEntity> kept = {};
		w.RZT_AllDeer(kept);
		RZT_Fake.MoveTo(home);
		array<IEntity> far = {};
		far.Insert(me);
		bool allFar = true;
		foreach (vector spot2 : spots)
		{
			if (vector.DistanceXZ(spot2, me.GetOrigin()) <= s.m_fAwayRadius + 300)
				allFar = false;
		}

		if (!allFar)
		{
			vector away = w.RZT_FarFrom(spots, s.m_fAwayRadius + 400);
			RZT_Fake.MoveTo(away);
		}

		w.RZT_PutAwayFar(far);
		Check(w.CountHerds() == 0, string.Format("every herd put away with the tester far off (%1 left)", w.CountHerds()));
		int standing = 0;
		foreach (IEntity left : kept)
		{
			if (left)
				standing++;
		}

		Check(standing == 0, string.Format("their deer are gone (%1 of %2 still standing)", standing, kept.Count()));

		// Put back
		RZT_Fake.MoveTo(home);
		s.m_sDoePrefab = doe;
		s.m_sBuckPrefab = buck;
		w.Setup();
		Print(string.Format("RZT wildlife: %1 checks, %2 failed", s_iChecks, s_iFails));
	}
}

//------------------------------------------------------------------------------------------------
modded class RZ_Wildlife
{
	//------------------------------------------------------------------------------------------------
	void RZT_Force()
	{
		m_bOn = true;
		m_bFailed = false;
	}

	//------------------------------------------------------------------------------------------------
	void RZT_ForgetTimers()
	{
		m_mNextFor.Clear();
		m_fCheckIn = 0;
	}

	//------------------------------------------------------------------------------------------------
	bool RZT_PutOut(int playerId, IEntity player, array<IEntity> players)
	{
		return PutOutFor(playerId, player, players);
	}

	//------------------------------------------------------------------------------------------------
	vector RZT_LastHerdPos()
	{
		if (m_aHerds.IsEmpty())
			return vector.Zero;

		return m_aHerds[m_aHerds.Count() - 1].m_vPos;
	}

	//------------------------------------------------------------------------------------------------
	int RZT_LastHerdSize()
	{
		if (m_aHerds.IsEmpty())
			return 0;

		return m_aHerds[m_aHerds.Count() - 1].m_aDeer.Count();
	}

	//------------------------------------------------------------------------------------------------
	bool RZT_IsCountry(vector p)
	{
		return IsCountry(p, 0);
	}

	//------------------------------------------------------------------------------------------------
	IEntity RZT_FirstDeer()
	{
		foreach (RZ_Herd h : m_aHerds)
		{
			foreach (IEntity deer : h.m_aDeer)
			{
				if (deer)
					return deer;
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	void RZT_AllDeer(array<IEntity> result)
	{
		foreach (RZ_Herd h : m_aHerds)
		{
			foreach (IEntity deer : h.m_aDeer)
			{
				if (deer)
					result.Insert(deer);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	int RZT_LayMeat(int playerId, vector ground)
	{
		return LayMeat(playerId, ground);
	}

	//------------------------------------------------------------------------------------------------
	void RZT_PutAwayFar(array<IEntity> players)
	{
		PutAwayFar(players);
	}

	//------------------------------------------------------------------------------------------------
	//! Open-country spots, each 650 m from the others (so their herds, 150 to 300 m off, can stand 300 m
	//! apart even on a small map), round the map's towns; several round one town if need be.
	void RZT_CountrySpots(RZ_GameModeComponent gm, int want, array<vector> result)
	{
		for (int t = 0; t < 40; t++)
		{
			foreach (RZ_Settlement s : gm.GetSettlements())
			{
				if (!s || result.Count() >= want)
					continue;

				vector p = RZ_Mission.LandPointNear(s.m_vCenter, s.m_fRadius + 400, s.m_fRadius + 1400);
				if (p == vector.Zero || RZ_Util.IsWater(p) || !IsCountry(p, 250))
					continue;

				bool apart = true;
				foreach (vector q : result)
				{
					if (vector.DistanceXZ(p, q) < 650)
						apart = false;
				}

				if (apart)
					result.Insert(p);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A land point at least `dist` from every spot.
	vector RZT_FarFrom(array<vector> spots, float dist)
	{
		foreach (RZ_Settlement s : m_GM.GetSettlements())
		{
			if (!s)
				continue;

			bool ok = true;
			foreach (vector q : spots)
			{
				if (vector.DistanceXZ(s.m_vCenter, q) < dist)
					ok = false;
			}

			if (ok)
				return s.m_vCenter;
		}

		return Vector(0, 0, 0);
	}
}
