//! The tribes' small rules (docs/INFECTED.md), owned by RZ_Tribes (GetRest) and called from its
//! phases and ticks. Server only; everything here is data or a line, on the tribes' own ticks.
//! - Prey: trails scored by the prey seen at their grounds, grounds that follow the prey, habits
//!   broken when their prey is taken away (rerouted trucks, nobody walking there any more).
//! - Borders moving the ranges; a Bastion patrol and a leader's bite counted as the taken; members
//!   crossing between ranges at night; the clans' relay and their frenzy nights on one ground.
//! - The rhythm: echoes shifting the hours, stragglers after a bad night, the Hunter's and the Radio
//!   Operator's ears at a call, a second caller when restless, a short-handed call, calls relayed
//!   from a ridge.
//! - Who they are: the factions speaking of a new leader, where a pack lies up by its look.
//! - Bodies: scouts posted above tonight's trails at dusk and lower in fog, nobody seen indoors, the
//!   leader's corpse and its samples kept through the vigil, a second infected faction (a setting).
//! - Starving walkers as loose ones, trucks taken where a trail crosses a road, the moat's rules
//!   waiting for a pack whose leader was taken alive, an abandoned lair's things as items, Bastion's
//!   "no contact", the beats handed to the epilogue in the last week.

//------------------------------------------------------------------------------------------------
class RZ_TribeRest
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_Tribes m_Tribes;			// weak: the owner
	protected int m_iPatrolNight = -10;		// the counted night a Bastion patrol was last taken
	protected int m_iCrossNight = -1;		// the counted night members last crossed between ranges
	protected ref map<int, ref array<IEntity>> m_mVigilSamples = new map<int, ref array<IEntity>>();
	protected ref map<int, IEntity> m_mVigilCorpse = new map<int, IEntity>();
	protected ref map<int, float> m_mInsideAt = new map<int, float>();
	protected ref map<int, bool> m_mInside = new map<int, bool>();
	protected ref map<string, bool> m_mStragglerTold = new map<string, bool>();
	protected ref map<int, bool> m_mRelayTold = new map<int, bool>();
	protected ref array<int> m_aStraggler = {};			// part ids kept out after a bad night
	protected ref array<vector> m_aWalkFrom = {};		// starving walkers: from the lair...
	protected ref array<int> m_aWalkTown = {};			// ...to the town...
	protected ref array<float> m_aWalkUntil = {};		// ...until (season days)
	protected ref array<IEntity> m_aRivalUnits = {};	// a Border Night's second side, in the rival faction
	protected ref array<string> m_aRivalKeys = {};		// their own faction before
	protected float m_fRivalTimer;
	protected int m_iPreyMoves;
	protected int m_iHabitsStarved;
	protected int m_iPatrols;
	protected int m_iCrossings;
	protected int m_iWalkersSpawned;
	protected int m_iBeats;
	protected ref array<int> m_aDiedTribe = {};
	protected ref array<vector> m_aDiedAt = {};
	protected ref array<float> m_aDiedWhen = {};

	static const float PREY_NEAR = 300;		// prey this close to a ground counts for its trail
	static const float PREY_CAP = 4;
	static const int PREY_SPOTS = 6;
	static const int PREY_HITS = 3;			// nights with prey at a spot before a ground moves there
	static const float RIDGE_REACH = 1.5;	// a call from a ridge carries this much further
	static const float INSIDE_CHECK = 3;	// seconds a player's indoor check is kept

	//------------------------------------------------------------------------------------------------
	void RZ_TribeRest(RZ_GameModeComponent gm, RZ_Tribes tribes)
	{
		m_GM = gm;
		m_Tribes = tribes;
	}

	//------------------------------------------------------------------------------------------------
	protected bool LifeOn()
	{
		return m_Tribes.GetSettings().m_bLifeCycle;
	}

	//------------------------------------------------------------------------------------------------
	protected bool Cured()
	{
		return m_GM.GetCure() && m_GM.GetCure().IsCured();
	}

	// ================================================================ Prey
	//------------------------------------------------------------------------------------------------
	//! Once a played minute at dusk and night: where the prey is. Players and wanderers in a range
	//! count for the ground they're near (its trail is picked more often), or as a prey spot away
	//! from every ground (three nights there and a ground moves to it at the next season day); prey
	//! within 300 m of a habit's place keeps the habit fed.
	void SeePrey(array<IEntity> players)
	{
		RZ_ETribePhase phase = m_Tribes.GetPhase();
		if (phase != RZ_ETribePhase.DUSK && phase != RZ_ETribePhase.NIGHT)
			return;

		array<vector> prey = {};
		foreach (IEntity p : players)
		{
			prey.Insert(p.GetOrigin());
		}

		RZ_Wanderers wanderers = m_GM.GetWanderers();
		if (wanderers)
		{
			array<int> living = {};
			wanderers.GetLiving(-1, living);
			foreach (int id : living)
			{
				vector wpos;
				if (wanderers.GetPosition(id, wpos))
					prey.Insert(wpos);
			}
		}

		RZ_SupplyLines supply = m_GM.GetSupply();
		if (supply && supply.IsActive())
			supply.TruckSpots(prey);

		if (prey.IsEmpty())
			return;

		int night = m_Tribes.CountedNights();
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			PadPrey(t);
			vector centre = t.RangeCentre();
			float reach = t.m_fRadius * 1.3 + PREY_NEAR;
			foreach (vector at : prey)
			{
				float d = RZ_Util.DistXZ(at, centre);
				if (d > reach)
					continue;

				bool atGround = false;
				foreach (int gi, vector g : t.m_aGrounds)
				{
					if (RZ_Util.DistXZ(g, at) <= PREY_NEAR)
					{
						t.m_aPrey[gi] = Math.Min(PREY_CAP, t.m_aPrey[gi] + 0.25);
						atGround = true;
					}
				}

				if (!atGround && d <= t.m_fRadius * 1.3)
					PreySpot(t, at, night);

				foreach (RZ_TribeHabit h : t.m_aHabits)
				{
					if (RZ_Util.DistXZ(h.m_vPos, at) <= PREY_NEAR)
						h.m_iPreyNight = night;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void PadPrey(RZ_Tribe t)
	{
		while (t.m_aPrey.Count() < t.m_aGrounds.Count())
		{
			t.m_aPrey.Insert(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Prey away from its grounds: a spot within 300 m of one already kept counts once a night;
	//! else a new spot (6 kept, the least visited goes).
	protected void PreySpot(RZ_Tribe t, vector at, int night)
	{
		foreach (int i, vector spot : t.m_aPreyAt)
		{
			if (RZ_Util.DistXZ(spot, at) > PREY_NEAR)
				continue;

			if (t.m_aPreyNight[i] != night)
			{
				t.m_aPreyNight[i] = night;
				t.m_aPreyHits[i] = t.m_aPreyHits[i] + 1;
			}

			return;
		}

		if (t.m_aPreyAt.Count() >= PREY_SPOTS)
		{
			int least = 0;
			for (int k = 1; k < t.m_aPreyHits.Count(); k++)
			{
				if (t.m_aPreyHits[k] < t.m_aPreyHits[least])
					least = k;
			}

			t.m_aPreyAt.Remove(least);
			t.m_aPreyHits.Remove(least);
			t.m_aPreyNight.Remove(least);
		}

		t.m_aPreyAt.Insert(at);
		t.m_aPreyHits.Insert(1);
		t.m_aPreyNight.Insert(night);
	}

	//------------------------------------------------------------------------------------------------
	//! Someone taken near a ground: that trail's prey +2.
	void OnTaken(RZ_Tribe t, vector at)
	{
		PadPrey(t);
		foreach (int i, vector g : t.m_aGrounds)
		{
			if (RZ_Util.DistXZ(g, at) <= 400)
				t.m_aPrey[i] = Math.Min(PREY_CAP, t.m_aPrey[i] + 2);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The trail weight its prey adds (RZ_TribeWays.PickTrail): 0 to 4.
	float PreyWeight(RZ_Tribe t, int trail)
	{
		if (trail < 0 || trail >= t.m_aPrey.Count())
			return 0;

		return t.m_aPrey[trail];
	}

	//------------------------------------------------------------------------------------------------
	//! At the season day: a prey spot visited 3 nights becomes a hunting ground (in place of the one
	//! with the least prey when it has 4), and its trails are worked out again.
	void PreyDay()
	{
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			int best = -1;
			foreach (int i, int hits : t.m_aPreyHits)
			{
				if (hits >= PREY_HITS && (best < 0 || hits > t.m_aPreyHits[best]))
					best = i;
			}

			if (best < 0)
				continue;

			vector at = RZ_Util.GroundPos(t.m_aPreyAt[best]);
			t.m_aPreyAt.Remove(best);
			t.m_aPreyHits.Remove(best);
			t.m_aPreyNight.Remove(best);
			if (MoveGround(t, at))
			{
				t.m_aPreyGrounds.Insert(at);
				while (t.m_aPreyGrounds.Count() > 2)
				{
					t.m_aPreyGrounds.Remove(0);
				}

				m_iPreyMoves++;
				m_Tribes.BuildTrails(t);
				RZ_Log.Info(string.Format("Tribes: %1 follows its prey: a new hunting ground at %2.", t.m_sPlace, RZ_SettlementScanner.GridReference(at)));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A ground at `at`: added with room, else in place of the one with the least prey. False on
	//! water, in no man's land or within 300 m of a ground it has.
	protected bool MoveGround(RZ_Tribe t, vector at)
	{
		RZ_Land land = m_GM.GetLand();
		if (RZ_Util.IsWater(at) || (land && land.InNoMansLand(at)))
			return false;

		foreach (vector g : t.m_aGrounds)
		{
			if (RZ_Util.DistXZ(g, at) < PREY_NEAR)
				return false;
		}

		PadPrey(t);
		if (t.m_aGrounds.Count() < 4)
		{
			t.m_aGrounds.Insert(at);
			t.m_aPrey.Insert(1);
			return true;
		}

		int least = 0;
		for (int i = 1; i < t.m_aGrounds.Count(); i++)
		{
			if (t.m_aPrey[i] < t.m_aPrey[least])
				least = i;
		}

		t.m_aGrounds[least] = at;
		t.m_aPrey[least] = 1;
		if (least < t.m_aWear.Count())
			t.m_aWear[least] = 0;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! BuildPlaces: the grounds it followed its prey to come back after a restart or a rebuild.
	void PreyGrounds(RZ_Tribe t)
	{
		foreach (vector g : t.m_aPreyGrounds)
		{
			MoveGround(t, g);
		}
	}

	// ================================================================ Borders
	//------------------------------------------------------------------------------------------------
	//! After the dusk's calling contests: each border's offset moves both ranges along the line
	//! between the lairs (toward B when it has moved toward B: A's range takes ground, B's gives it),
	//! half the offset each, 200 m at most a pack. Range checks then use the moved centre.
	void BorderShifts()
	{
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			t.m_vBorderShift = vector.Zero;
		}

		if (!LifeOn())
			return;

		array<int> aIds = {};
		array<int> bIds = {};
		array<float> offsets = {};
		m_Tribes.GetBorders().Offsets(aIds, bIds, offsets);
		foreach (int i, int aId : aIds)
		{
			RZ_Tribe a = m_Tribes.Find(aId);
			RZ_Tribe b = m_Tribes.Find(bIds[i]);
			if (!a || !b || Math.AbsFloat(offsets[i]) < 1)
				continue;

			vector dir = RZ_TribePeople.Dir(a.m_vLair, b.m_vLair);
			a.m_vBorderShift = a.m_vBorderShift + dir * (offsets[i] * 0.5);
			b.m_vBorderShift = b.m_vBorderShift + dir * (offsets[i] * 0.5);
		}

		foreach (RZ_Tribe each : m_Tribes.GetTribeList())
		{
			float len = each.m_vBorderShift.Length();
			if (len > 200)
				each.m_vBorderShift = each.m_vBorderShift * (200 / len);
		}
	}

	// ================================================================ Deaths at the lair
	//------------------------------------------------------------------------------------------------
	//! A member died (any time of day): kept for the burned lair's splinter, 20 at most.
	void OnDied(RZ_Tribe t, vector at)
	{
		m_aDiedTribe.Insert(t.m_iId);
		m_aDiedAt.Insert(at);
		m_aDiedWhen.Insert(m_Tribes.GetClock());
		while (m_aDiedTribe.Count() > 20)
		{
			m_aDiedTribe.Remove(0);
			m_aDiedAt.Remove(0);
			m_aDiedWhen.Remove(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Its lair burned (RZ_TribeLife.OnFire): with 4 or more of it killed within 150 m of the lair
	//! in the last 30 played minutes, 3 to 6 of the rest break off as a splinter (Burn the Lair).
	void LairBurned(RZ_Tribe t)
	{
		if (Cured() || !LifeOn())
			return;

		int dead = 0;
		float now = m_Tribes.GetClock();
		foreach (int i, int id : m_aDiedTribe)
		{
			if (id == t.m_iId && now - m_aDiedWhen[i] <= 1800 && RZ_Util.DistXZ(m_aDiedAt[i], t.m_vLair) <= 150)
				dead++;
		}

		if (dead < 4)
			return;

		int n = Math.Min(Math.RandomIntInclusive(3, 6), t.Members() - 2);
		if (n < 3)
			return;

		m_Tribes.GetLife().Splinter(t, n, t.m_vLair);
		RZ_Log.Info(string.Format("Tribes: %1 burned out with %2 of it dead: %3 break off.", t.m_sPlace, dead, n));
	}

	// ================================================================ The counted night
	//------------------------------------------------------------------------------------------------
	//! After a counted night: prey fades; a habit whose place saw no prey all night falls 15 more
	//! (its trucks rerouted, its walkers stopped); Bastion patrols; members crossing between ranges.
	void OnCountedNight()
	{
		int night = m_Tribes.CountedNights();
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			foreach (int i, float f : t.m_aPrey)
			{
				t.m_aPrey[i] = f * 0.8;
			}

			if (!LifeOn())
				continue;

			for (int h = t.m_aHabits.Count() - 1; h >= 0; h--)
			{
				RZ_TribeHabit habit = t.m_aHabits[h];
				if (habit.m_iPreyNight < 0 || habit.m_iPreyNight >= night - 1)
					continue;

				habit.m_fStrength = habit.m_fStrength - 15;
				m_iHabitsStarved++;
				if (habit.m_fStrength < 10)
				{
					t.m_aHabits.Remove(h);
					RZ_Log.Info(string.Format("Tribes: %1 loses a habit: nothing comes past its place any more.", t.m_sPlace));
				}
			}
		}

		Patrol(night);
		Crossings(night);
	}

	//------------------------------------------------------------------------------------------------
	//! The taken (rule 4): a Bastion patrol, as data. Once on the server every 6 counted nights, a
	//! hungry pack that set out within 2.5 km of a Bastion holding, with Bastion out on the roads
	//! (presence 0.2 or more) and no player within 1 km, takes one 20% of the time.
	protected void Patrol(int night)
	{
		if (night - m_iPatrolNight < 6 || Cured() || RZ_WorldFacts.Get().BastionPresence() < 0.2)
			return;

		RZ_Territory territory = m_GM.GetTerritory();
		if (!territory || !territory.IsActive())
			return;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (!t.m_bHunted || t.Band() < 2 || t.IsWeakened())
				continue;

			RZ_Holding post = territory.FindHolding(RZ_EFaction.MILITARY, t.RangeCentre(), t.m_fRadius + 2500);
			if (!post)
				continue;

			vector at = RZ_Util.GroundPos(t.RangeCentre() + RZ_TribePeople.Dir(t.RangeCentre(), post.m_vPos) * t.m_fRadius);
			if (RZ_Util.NearestDistance(at, players) < 1000 || Math.RandomFloat01() > 0.2)
				continue;

			m_iPatrolNight = night;
			m_iPatrols++;
			array<string> calls = {"Two-Six", "Four-One", "Three-Three", "One-Nine", "Five-Two"};
			string call = "Patrol " + calls[RZ_Util.PickIndex(t.m_iSeed + night, calls.Count())];
			m_Tribes.Taken(t, at, "a Bastion patrol", true);
			t.AddHistory("took a Bastion patrol");
			RZ_TribePeople people = m_Tribes.GetPeople();
			if (people.IsOn())
				people.TellFaction(RZ_EFaction.MILITARY, "Task Force Bastion", "patrol", t, string.Format("\"%1, last position %2. No reply since the small hours. Bastion out.\"", call, people.NameAt(t, RZ_EFaction.MILITARY)));

			m_Tribes.GetWays().Headline(t, string.Format("A Bastion patrol went out past %1 in the night and didn't come back.", t.m_sPlace));
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Members crossing between ranges at night: once a counted night, one border where neither side
	//! fought and one has 6 or more more than the other: one of the bigger side's young or hunters
	//! crosses to the smaller (never a clan's, a job's, or after the cure).
	protected void Crossings(int night)
	{
		if (!LifeOn() || m_iCrossNight == night || Cured())
			return;

		m_iCrossNight = night;
		foreach (RZ_Tribe a : m_Tribes.GetTribeList())
		{
			foreach (RZ_Tribe b : m_Tribes.GetTribeList())
			{
				if (a == b || !m_Tribes.GetBorders().Bordering(a, b) || a.Members() - b.Members() < 6)
					continue;

				if (a.m_iClan > 0 && a.m_iClan == b.m_iClan)
					continue;

				if (RZ_TribeContract.IsHeld(m_GM, a) || RZ_TribeContract.IsHeld(m_GM, b) || Math.RandomFloat01() > 0.3)
					continue;

				int young;
				if (m_Tribes.GetLife().TakeMixed(a, 1, young) <= 0)
					continue;

				m_Tribes.GetLife().Join(b, 0, 1 - young, young);
				m_iCrossings++;
				RZ_Log.Info(string.Format("Tribes: one of %1 crossed in the night to %2.", a.m_sPlace, b.m_sPlace));
				return;
			}
		}
	}

	// ================================================================ The rhythm
	//------------------------------------------------------------------------------------------------
	//! Tonight's set-out hour (19:30-20:30), shifted by its leader's echo: a driver's at 19:30, when
	//! the evening's traffic was; a farmer's late, after the work; a runner's early.
	float SetOutHour(RZ_Tribe t)
	{
		if (LifeOn() && t.Echo(RZ_EAlphaKind.DRIVER))
			return Math.RandomFloat(19.5, 19.75);

		if (LifeOn() && t.Echo(RZ_EAlphaKind.FARMER))
			return Math.RandomFloat(20.25, 20.5);

		if (LifeOn() && t.Echo(RZ_EAlphaKind.RUNNER))
			return Math.RandomFloat(19.5, 20);

		return Math.RandomFloat(19.5, 20.5);
	}

	//------------------------------------------------------------------------------------------------
	//! When its day party goes out: a farmer's at 06:30, everyone else's at 07:00.
	float DayOutHour(RZ_Tribe t)
	{
		if (LifeOn() && t.Echo(RZ_EAlphaKind.FARMER))
			return 6.5;

		return 7;
	}

	//------------------------------------------------------------------------------------------------
	//! Dawn after a bad night (a quarter or more of who set out lost, 2 at least): one hunting group
	//! still out stays out, and comes home late between 06:00 and 07:30.
	void Dawn(RZ_Tribe t)
	{
		t.m_bBadNight = t.m_bHunted && t.m_iLostTonight >= 2 && t.m_iLostTonight * 4 >= t.m_iAtDusk;
		if (!t.m_bBadNight)
			return;

		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (!p.m_bOut || p.HasBody() || p.m_iCount <= 0 || p.m_bGathering || (p.m_eKind != RZ_ETribePart.HUNTERS && p.m_eKind != RZ_ETribePart.YOUNG))
				continue;

			if (RZ_Util.DistXZ(p.m_vPos, t.m_vLair) < 300)
				continue;

			p.m_aPath.Clear();
			p.m_bHasTarget = false;
			p.m_bHome = false;
			p.m_fHoldUntil = float.MAX;
			m_aStraggler.Insert(p.m_iId);
			RZ_Log.Info(string.Format("Tribes: %1 had a bad night; %2 of it are stragglers, home late.", t.m_sPlace, p.m_iCount));
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The stragglers set off home between 06:00 and 07:30, slow (0.7 m/s), and anyone within
	//! 800 m of them hears it once; by 07:30 every straggler is on its way.
	void StragglerTick(float h, array<int> ids, array<IEntity> players)
	{
		if (m_aStraggler.IsEmpty() || h < 6)
			return;

		for (int i = m_aStraggler.Count() - 1; i >= 0; i--)
		{
			int id = m_aStraggler[i];
			RZ_Tribe owner;
			RZ_TribePart part = FindPart(id, owner);
			if (!part)
			{
				m_aStraggler.Remove(i);
				continue;
			}

			float when = 6 + RZ_Util.PickIndex(id, 6) * 0.25;
			if (h < when && h < 7.5)
				continue;

			m_aStraggler.Remove(i);
			m_Tribes.PathHome(owner, part);
			part.m_fHoldUntil = m_Tribes.GetClock();
			part.m_fSpeed = 0.7;
			foreach (int k, IEntity pl : players)
			{
				string key = string.Format("%1:%2", ids[k], RZ_SeasonClock.Day());
				if (RZ_Util.DistXZ(pl.GetOrigin(), part.m_vPos) > 800 || m_mStragglerTold.Contains(key))
					continue;

				m_mStragglerTold.Set(key, true);
				m_GM.NotifyPlayer(ids[k], "First light", string.Format("Stragglers from last night, coming in late toward %1. Slow, and not many of them.", owner.m_sPlace));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A straggler stays out when the rest are sent home (SendHome).
	bool IsStraggler(RZ_TribePart p)
	{
		return !m_aStraggler.IsEmpty() && m_aStraggler.Find(p.m_iId) >= 0;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_TribePart FindPart(int partId, out RZ_Tribe owner)
	{
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			foreach (RZ_TribePart p : t.m_aParts)
			{
				if (p.m_iId == partId)
				{
					owner = t;
					return p;
				}
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! How far its dusk call carries: 3 km, half as far again from a lair or a scout post on a ridge
	//! (relayed ridge to ridge).
	float CallReach(RZ_Tribe t)
	{
		RZ_Land land = m_GM.GetLand();
		if (!land)
			return RZ_Tribes.CALL_RANGE;

		array<vector> spots = {t.m_vLair};
		foreach (vector post : t.m_aPosts)
		{
			spots.Insert(post);
		}

		foreach (vector s : spots)
		{
			int idx = land.CellAt(s);
			if (idx >= 0 && land.HasBit(idx, RZ_Land.BIT_RIDGE))
				return RZ_Tribes.CALL_RANGE * RIDGE_REACH;
		}

		return RZ_Tribes.CALL_RANGE;
	}

	//------------------------------------------------------------------------------------------------
	//! A short-handed call: some of it is still out when it calls (a group waiting at a crossing or
	//! a wire, walkers not back, the Gathering). "" if everyone is home.
	string ShortHanded(RZ_Tribe t)
	{
		int away = 0;
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (p.m_bOut && !p.IsCore() && p.m_eKind != RZ_ETribePart.SCOUT && RZ_Util.DistXZ(p.m_vPos, t.m_vLair) > 300)
				away += p.m_iCount;
		}

		if (away < 2)
			return "";

		return " Short-handed: some of them are still out.";
	}

	//------------------------------------------------------------------------------------------------
	//! What a Hunter or a Radio Operator adds to the call they hear: the Hunter which way and how
	//! far, and whether something leads it; either of them names a pack they've heard before by its
	//! call (and has it Read). "" for anyone else.
	string SkillLine(RZ_Tribe t, int playerId, vector at)
	{
		RZ_Background bg = m_GM.GetPlayerBackground(playerId);
		if (!bg || (bg.m_sId != "hunter" && bg.m_sId != "radioham"))
			return "";

		string text = "";
		RZ_TribePeople people = m_Tribes.GetPeople();
		if (people.IsOn() && people.Level(playerId, t) >= RZ_EKnow.HEARD)
		{
			text += string.Format(" %1: you'd know that call anywhere. It's %2.", RZ_Util.Capitalize(t.m_sPattern), t.m_sPlace);
			people.Learn(playerId, t, RZ_EKnow.READ, -1);
		}

		if (bg.m_sId != "hunter")
			return text;

		float d = RZ_Util.DistXZ(at, t.m_vLair);
		string far = "under a kilometre";
		if (d >= 1500)
			far = string.Format("about %1 kilometres", RZ_Util.RoundToInt(d / 1000));
		else if (d >= 1000)
			far = "a kilometre or so";

		string lead = "Nothing leading it.";
		if (t.m_bHasAlpha)
			lead = "One deep voice leading.";

		return text + string.Format(" By ear: %1 %2 of you. %3", far, RZ_Tribes.CompassWord(at, t.m_vLair), lead);
	}

	//------------------------------------------------------------------------------------------------
	//! A call is made: a restless pack's second caller is a sound of its own at its first scout post.
	void OnCall(RZ_Tribe t, bool silent)
	{
		t.m_bSilentCall = silent;
		if (silent || !t.m_bRestless || t.m_aPosts.IsEmpty())
			return;

		m_Tribes.GetProps().CallSound(t.m_aPosts[0], false);
	}

	//------------------------------------------------------------------------------------------------
	//! The clans' relay: players within 1 km of the line between two packs of one clan that both
	//! called tonight hear the call pass along the ridges, once a dusk.
	void ClanRelay(array<int> ids, array<IEntity> players)
	{
		m_mRelayTold.Clear();
		if (!LifeOn())
			return;

		array<ref RZ_Tribe> list = m_Tribes.GetTribeList();
		for (int i = 0; i < list.Count(); i++)
		{
			RZ_Tribe a = list[i];
			if (a.m_iClan <= 0 || !a.m_bCalled || a.m_bSilentCall)
				continue;

			for (int j = i + 1; j < list.Count(); j++)
			{
				RZ_Tribe b = list[j];
				if (b.m_iClan != a.m_iClan || !b.m_bCalled || b.m_bSilentCall)
					continue;

				foreach (int k, IEntity p : players)
				{
					if (m_mRelayTold.Contains(ids[k]) || !RZ_TribePeople.Between(p.GetOrigin(), a.m_vLair, b.m_vLair, 1000))
						continue;

					m_mRelayTold.Set(ids[k], true);
					m_GM.NotifyPlayer(ids[k], "At dusk", string.Format("The call goes along the ridges from %1 to %2 and back again, the same call one note apart.", a.m_sPlace, b.m_sPlace));
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A clan on a frenzy night hunts toward one ground: the wire of the lived-in place nearest the
	//! clan's middle. False with no clan or no wire.
	bool ClanGround(RZ_Tribe t, out vector ground)
	{
		if (t.m_iClan <= 0 || t.m_fFrenzy <= 0)
			return false;

		vector sum = vector.Zero;
		int n = 0;
		foreach (RZ_Tribe other : m_Tribes.GetTribeList())
		{
			if (other.m_iClan == t.m_iClan)
			{
				sum = sum + other.m_vLair;
				n++;
			}
		}

		if (n < 2)
			return false;

		vector mid = sum * (1.0 / n);
		return m_Tribes.WireEnd(mid, ground);
	}

	// ================================================================ Who they are
	//------------------------------------------------------------------------------------------------
	//! At dusk, once a leader has led a season day: each faction that knows the pack says who leads
	//! it now, in its own words (one pack an evening). Never for a season's first leaders.
	void LeaderVoices()
	{
		RZ_TribePeople people = m_Tribes.GetPeople();
		if (!people.IsOn() || !LifeOn())
			return;

		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (!t.m_bHasAlpha || t.m_fVoicedLeader == t.m_fLedSince || RZ_SeasonClock.DaysSince(t.m_fLedSince) < 1)
				continue;

			t.m_fVoicedLeader = t.m_fLedSince;
			if (t.m_fLedSince <= t.m_fFounded + 0.01)
				continue;

			int day = RZ_SeasonClock.Day();
			string haven = string.Format("\"Something new leads them up at %1. Nobody knows who it was.\"", t.m_sPlace);
			if (people.HavenKnowsLeader(t))
				haven = string.Format("\"It's %1 leading them up at %2 now. %3, before.\"", t.m_sAlphaName, t.m_sPlace, RZ_Util.Capitalize(t.m_sAlphaWho));

			people.TellFaction(RZ_EFaction.SURVIVORS, "The Haven", "leader", t, haven);
			if (t.m_iContact > 0)
				people.TellFaction(RZ_EFaction.MILITARY, "Task Force Bastion", "leader", t, string.Format("\"Contact log, %1: heavy replaced, day %2. Bastion out.\"", people.NameAt(t, RZ_EFaction.MILITARY), day));

			if (t.m_bMoat || NearCamp(t))
				people.TellFaction(RZ_EFaction.BANDITS, "The Red Hand", "leader", t, string.Format("\"%1 have a new boss. We'll see what this one wants.\"", RZ_Util.Capitalize(people.NameAt(t, RZ_EFaction.BANDITS))));

			if (t.m_iTagDay >= 0)
				people.TellFaction(RZ_EFaction.INSTITUTE, "Halcyon", "leader", t, string.Format("\"%1: a new index, day %2. The old one's file is closed.\"", people.NameAt(t, RZ_EFaction.INSTITUTE), day));

			RZ_Log.Info(string.Format("Tribes: the factions speak of %1's new leader.", t.m_sPlace));
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool NearCamp(RZ_Tribe t)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		return territory && territory.FindHolding(RZ_EFaction.BANDITS, t.RangeCentre(), 3000) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Where it lies up by its look: an outdoor pack has the widest range (x1.15) and posts on the
	//! highest ground in it and beyond; a dust-grey one lies up close, under cover (its ring x0.6).
	float LookRange(RZ_Tribe t, float radius, float hi)
	{
		if (LifeOn() && t.m_eLook == RZ_ELook.OUTDOOR)
			return Math.Min(hi * 1.15, radius * 1.15);

		return radius;
	}

	//------------------------------------------------------------------------------------------------
	float RingScale(RZ_Tribe t)
	{
		if (LifeOn() && t.m_eLook == RZ_ELook.DUST)
			return 0.6;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	float PostReach(RZ_Tribe t)
	{
		if (LifeOn() && t.m_eLook == RZ_ELook.OUTDOOR)
			return 1.3;

		return 1;
	}

	// ================================================================ Bodies
	//------------------------------------------------------------------------------------------------
	//! At the set-out: its scouts move to the highest wild ground within 300 m of the middle of
	//! tonight's first trail. Back on their own posts at dawn (Posts).
	void DuskPosts(RZ_Tribe t, array<vector> trail)
	{
		if (!LifeOn() || trail.Count() < 2)
			return;

		vector mid = trail[trail.Count() / 2];
		vector best;
		if (!HighNear(mid, 300, best))
			return;

		int n = 0;
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (p.m_eKind != RZ_ETribePart.SCOUT)
				continue;

			p.m_vRest = RZ_Util.GroundPos(best + Vector(n * 40, 0, n * 25));
			n++;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The scouts on their own posts, or in fog by day a little lower: 80 m down toward the lair.
	void Posts(RZ_Tribe t, bool fog)
	{
		if (t.m_aPosts.IsEmpty())
			return;

		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (p.m_eKind != RZ_ETribePart.SCOUT || p.m_iPost < 0)
				continue;

			vector post = t.m_aPosts[p.m_iPost % t.m_aPosts.Count()];
			if (fog && LifeOn())
				post = RZ_Util.GroundPos(post + RZ_TribePeople.Dir(post, t.m_vLair) * 80);

			p.m_vRest = post;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool HighNear(vector at, float reach, out vector best)
	{
		RZ_Land land = m_GM.GetLand();
		if (!land)
			return false;

		int steps = reach / RZ_Land.CELL;
		steps++;
		float top = -10000;
		bool found = false;
		for (int dz = -steps; dz <= steps; dz++)
		{
			for (int dx = -steps; dx <= steps; dx++)
			{
				vector p = at + Vector(dx * RZ_Land.CELL, 0, dz * RZ_Land.CELL);
				if (RZ_Util.DistXZ(p, at) > reach + RZ_Land.CELL * 0.5)
					continue;

				int idx = land.CellAt(p);
				if (idx < 0 || !land.HasBit(idx, RZ_Land.BIT_WILD) || !land.IsDry(idx))
					continue;

				float hgt = land.GetHeight(idx);
				if (land.HasBit(idx, RZ_Land.BIT_RIDGE))
					hgt += 50;

				if (hgt > top)
				{
					top = hgt;
					best = RZ_Util.GroundPos(land.CellCenter(idx));
					found = true;
				}
			}
		}

		return found;
	}

	//------------------------------------------------------------------------------------------------
	//! The scout rule's building query: a player inside a building is never seen (checked once in 3 s
	//! a player: the buildings within 15 m and whether they stand inside one's box).
	bool IsInside(int playerId, IEntity player)
	{
		float now = m_Tribes.GetClock();
		float at;
		bool inside;
		if (m_mInsideAt.Find(playerId, at) && now - at < INSIDE_CHECK && m_mInside.Find(playerId, inside))
			return inside;

		inside = false;
		vector pos = player.GetOrigin();
		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(pos, 15, buildings, 4);
		foreach (IEntity b : buildings)
		{
			if (b && RZ_DoorGeometry.InBuilding(b, pos))
			{
				inside = true;
				break;
			}
		}

		m_mInsideAt.Set(playerId, now);
		m_mInside.Set(playerId, inside);
		return inside;
	}

	//------------------------------------------------------------------------------------------------
	//! A leader killed: its samples aren't cleaned up while its pack holds the vigil, and its body is
	//! kept from the engine's cleanup until dawn. Spawns the samples itself (n of them).
	void VigilKeep(RZ_Tribe t, RZ_TrackedZombie z, vector at, int n)
	{
		ResourceName sample = m_GM.GetSamplePrefab();
		array<IEntity> kept = {};
		m_mVigilSamples.Find(t.m_iId, kept);
		if (!kept)
		{
			kept = {};
			m_mVigilSamples.Set(t.m_iId, kept);
		}

		if (!sample.IsEmpty() && !Cured())
		{
			for (int i = 0; i < n; i++)
			{
				IEntity e = RZ_Util.SpawnPrefab(sample, RZ_Util.RandomPointInRing(at, 0.3, 1.5) + Vector(0, 0.3, 0));
				if (e)
					kept.Insert(e);
			}
		}

		if (!z || z.m_aMembers.IsEmpty())
			return;

		IEntity corpse = z.m_aMembers[0];
		if (!corpse)
			return;

		SCR_GarbageSystem garbage = SCR_GarbageSystem.GetByEntityWorld(corpse);
		if (!garbage)
			return;

		// VERIFY: a dead character withdrawn from the garbage system and blacklisted stays where it
		// fell until it's inserted again (the base game's own corpse cleanup is the garbage system)
		garbage.Withdraw(corpse);
		garbage.UpdateBlacklist(corpse, true);
		m_mVigilCorpse.Set(t.m_iId, corpse);
	}

	//------------------------------------------------------------------------------------------------
	//! The vigil ends at dawn: the samples are loose items again, the body goes back to the cleanup.
	void VigilEnds(RZ_Tribe t)
	{
		array<IEntity> kept;
		if (m_mVigilSamples.Find(t.m_iId, kept) && kept)
		{
			foreach (IEntity e : kept)
			{
				if (e)
					RZ_GameModeComponent.TrackLooseItem(e);
			}
		}

		m_mVigilSamples.Remove(t.m_iId);
		IEntity corpse;
		if (m_mVigilCorpse.Find(t.m_iId, corpse) && corpse)
		{
			SCR_GarbageSystem garbage = SCR_GarbageSystem.GetByEntityWorld(corpse);
			if (garbage)
			{
				garbage.UpdateBlacklist(corpse, false);
				garbage.Insert(corpse);
			}
		}

		m_mVigilCorpse.Remove(t.m_iId);
	}

	// ================================================================ A second infected faction
	//------------------------------------------------------------------------------------------------
	//! Every 5 s: with Zombies > Tribes > Rival faction set (a faction key in the faction manager,
	//! hostile to the infected's own), the second side's hunter bodies within 150 m of a staged
	//! Border Night's point join it, so the two sides really fight; when the night is settled they
	//! go back. Off by default.
	void RivalTick(float dt)
	{
		m_fRivalTimer += dt;
		if (m_fRivalTimer < 5)
			return;

		m_fRivalTimer = 0;
		string key = m_Tribes.GetSettings().m_sRivalFaction;
		if (key.IsEmpty() || !RZ_Util.HasFaction(key))
			return;

		array<int> staged = {};
		array<vector> points = {};
		m_Tribes.GetBorders().StagedNow(staged, points);
		if (staged.IsEmpty())
		{
			RivalsBack();
			return;
		}

		foreach (int i, int tribeId : staged)
		{
			RZ_Tribe t = m_Tribes.Find(tribeId);
			if (!t)
				continue;

			foreach (RZ_TribePart p : t.m_aParts)
			{
				if (!p.HasBody() || !p.m_Body.m_Entity || p.m_eKind != RZ_ETribePart.HUNTERS || m_aRivalUnits.Find(p.m_Body.m_Entity) >= 0)
					continue;

				if (RZ_Util.DistXZ(p.m_vPos, points[i]) > 150)
					continue;

				array<IEntity> members = {};
				RZ_Util.GetUnitMembers(p.m_Body.m_Entity, members);
				string before = "";
				if (!members.IsEmpty())
					before = RZ_Util.GetCharacterFactionKey(members[0]);

				// VERIFY: Bacon's infected fight bodies of a faction set hostile to theirs in the faction
				// manager, and a group moved onto it mid-fight takes the new faction's enemies
				if (before.IsEmpty() || RZ_Util.SetUnitFaction(p.m_Body.m_Entity, key) <= 0)
					continue;

				m_aRivalUnits.Insert(p.m_Body.m_Entity);
				m_aRivalKeys.Insert(before);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RivalsBack()
	{
		foreach (int i, IEntity unit : m_aRivalUnits)
		{
			if (unit)
				RZ_Util.SetUnitFaction(unit, m_aRivalKeys[i]);
		}

		m_aRivalUnits.Clear();
		m_aRivalKeys.Clear();
	}

	// ================================================================ Starving walkers
	//------------------------------------------------------------------------------------------------
	//! A starving pack's members walk into a town (RZ_TribeLife.Starvation): until the next season
	//! day, a loose one wanted near their way is one of them, walking to that town.
	void StarvingWalk(RZ_Tribe t, int town)
	{
		m_aWalkFrom.Insert(t.m_vLair);
		m_aWalkTown.Insert(town);
		m_aWalkUntil.Insert(RZ_SeasonClock.Days() + 1);
	}

	//------------------------------------------------------------------------------------------------
	//! The spawner asks for a loose one near p: a starving walker's town when p is within 1 km of
	//! the line from its lair to the town. False if none.
	bool WalkerGoal(vector p, out vector goal, out int town)
	{
		float now = RZ_SeasonClock.Days();
		for (int i = m_aWalkFrom.Count() - 1; i >= 0; i--)
		{
			if (now >= m_aWalkUntil[i])
			{
				m_aWalkFrom.Remove(i);
				m_aWalkTown.Remove(i);
				m_aWalkUntil.Remove(i);
				continue;
			}

			RZ_Settlement s = m_GM.GetSettlement(m_aWalkTown[i]);
			if (!s || !RZ_TribePeople.Between(p, m_aWalkFrom[i], s.m_vCenter, 1000))
				continue;

			town = m_aWalkTown[i];
			goal = RZ_Util.GroundPos(RZ_Util.RandomPointInRing(s.m_vCenter, 0, s.m_fRadius * 0.6));
			m_iWalkersSpawned++;
			return true;
		}

		return false;
	}

	// ================================================================ Trucks where a trail crosses a road
	//------------------------------------------------------------------------------------------------
	//! Where a trail crosses a road (decision 15: trucks are taken on any road a trail crosses):
	//! points every 100 m along it in a square with a road, at the nearest road point within 150 m.
	void RoadCrossings(RZ_Tribe t)
	{
		t.m_aRoadCross.Clear();
		RZ_Land land = m_GM.GetLand();
		if (!land)
			return;

		foreach (array<vector> trail : t.m_aTrails)
		{
			for (int i = 0; i + 1 < trail.Count(); i++)
			{
				vector a = trail[i];
				vector b = trail[i + 1];
				float len = RZ_Util.DistXZ(a, b);
				int steps = Math.Max(1, len / 100);
				for (int s = 0; s <= steps; s++)
				{
					float f = s;
					vector p = a + (b - a) * (f / steps);
					int idx = land.CellAt(p);
					if (idx < 0 || land.GetRoadWidth(idx) <= 0)
						continue;

					bool known = false;
					foreach (vector c : t.m_aRoadCross)
					{
						if (RZ_Util.DistXZ(c, p) < 400)
							known = true;
					}

					if (known || t.m_aRoadCross.Count() >= 8)
						continue;

					array<vector> road = {};
					RZ_Perimeter.RoadPointsNear(p, 150, road);
					if (!road.IsEmpty())
						t.m_aRoadCross.Insert(road[0]);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! At night, each data tick: a tribe out on its trails tries a truck at one of its road
	//! crossings (the life cycle's truck rule and its rate: one on the server every 3 counted nights).
	void TrucksAtCrossings()
	{
		if (m_Tribes.GetPhase() != RZ_ETribePhase.NIGHT)
			return;

		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (!t.m_bHunted || t.m_aRoadCross.IsEmpty() || t.IsWeakened())
				continue;

			vector at = t.m_aRoadCross[RZ_Util.PickIndex(t.m_iSeed + m_Tribes.CountedNights() * 7 + RZ_Util.RoundToInt(m_Tribes.GetClock() / 60), t.m_aRoadCross.Count())];
			if (m_Tribes.GetLife().TryTruckAt(t, at))
				return;
		}
	}

	// ================================================================ The moat and the taken alive
	//------------------------------------------------------------------------------------------------
	//! Its leader was taken alive (Capture the Index, or a finale route's live tissue): the moat
	//! rules wait for it until it has a new leader.
	void OnTakenAlive(RZ_Tribe t)
	{
		t.m_fTakenAliveAt = RZ_SeasonClock.Days();
	}

	//------------------------------------------------------------------------------------------------
	//! The moat rule and the fat moat skip a pack whose leader was taken alive, until its succession.
	static bool MoatWaits(RZ_Tribe t)
	{
		return t.m_fTakenAliveAt >= 0 && t.m_fLedSince <= t.m_fTakenAliveAt;
	}

	// ================================================================ Abandoned lairs
	//------------------------------------------------------------------------------------------------
	//! An abandoned lair's things (RZ_TribeLife): one or two items from the body search's table at
	//! its back, once, loose items like any find. Nothing if the server has no table.
	void LairFinds(vector at)
	{
		RZ_Interactions hub = m_GM.GetInteractions();
		if (!hub || !hub.GetSearch())
			return;

		array<ResourceName> picks = {};
		hub.GetSearch().PickHouseLoot("body", at, false, Math.RandomIntInclusive(1, 2), true, picks);
		foreach (ResourceName pick : picks)
		{
			IEntity e = RZ_Util.SpawnPrefab(pick, RZ_Util.RandomPointInRing(at, 0.5, 2) + Vector(0, 0.3, 0));
			RZ_GameModeComponent.TrackLooseItem(e);
		}
	}

	// ================================================================ Bastion's board
	//------------------------------------------------------------------------------------------------
	//! At dusk, with nothing new for the board: a board pack that didn't call tonight is "no
	//! contact" on Bastion's net. True if it was told.
	bool NoContact()
	{
		RZ_TribePeople people = m_Tribes.GetPeople();
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (t.m_iContact <= 0 || !t.m_bSilentCall)
				continue;

			people.TellFaction(RZ_EFaction.MILITARY, "Task Force Bastion", "nocontact", t, string.Format("\"%1: no contact. Bastion out.\"", people.Name(t, RZ_EFaction.MILITARY)));
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A board pack gone (broken up, absorbed, died out): struck off on Bastion's net.
	void OffBoard(RZ_Tribe t)
	{
		RZ_TribePeople people = m_Tribes.GetPeople();
		if (t.m_iContact <= 0 || !people.IsOn())
			return;

		people.TellFaction(RZ_EFaction.MILITARY, "Task Force Bastion", "nocontact", t, string.Format("\"%1: no contact. Struck off the board. Bastion out.\"", people.Name(t, RZ_EFaction.MILITARY)));
	}

	// ================================================================ The epilogue's beats
	//------------------------------------------------------------------------------------------------
	//! A beat after the cure goes to the epilogue's since-the-cure beats too, at the pack it names
	//! (the first pack whose place is in it; with none, it stays a /tribes beat only).
	void BeatToEpilogue(string beat)
	{
		RZ_Epilogue epi = m_GM.GetEpilogue();
		if (!epi || !Cured() || !epi.IsBuilt() || !epi.GetBeats())
			return;

		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (!beat.Contains(t.m_sPlace))
				continue;

			m_iBeats++;
			epi.GetBeats().AddSince("pack." + t.m_iId.ToString() + "." + m_iBeats.ToString(), t.m_sPlace, RZ_Mission.LandPointNear(t.m_vLair, 400, 600), beat, string.Format("Something changed at %1 this week.", t.m_sPlace));
			return;
		}
	}

	// ================================================================ The next season
	//------------------------------------------------------------------------------------------------
	//! Zombies > Tribes > Prime seeds after Halcyon (off): after a season Halcyon won, the seeds'
	//! leaders start in their prime (6 season days into leading), not Rising.
	float SeedLedSince()
	{
		RZ_Seasons seasons = m_GM.GetSeasons();
		if (m_Tribes.GetSettings().m_bPrimeAfterHalcyon && seasons && seasons.GetRulesWinner() == RZ_EFaction.INSTITUTE)
			return RZ_SeasonClock.Days() - Math.RandomFloat(6, 9);

		return RZ_SeasonClock.Days() - Math.RandomFloat(2, 5);
	}

	//------------------------------------------------------------------------------------------------
	string StatusText()
	{
		return string.Format("small rules: grounds moved to prey %1, habits starved %2, patrols taken %3, crossings %4, starving walkers %5, rivals now %6", m_iPreyMoves, m_iHabitsStarved, m_iPatrols, m_iCrossings, m_iWalkersSpawned, m_aRivalUnits.Count());
	}

	//------------------------------------------------------------------------------------------------
	int GetPatrolNight()
	{
		return m_iPatrolNight;
	}

	//------------------------------------------------------------------------------------------------
	void SetPatrolNight(int night)
	{
		m_iPatrolNight = night;
	}
}
