//! The packs' contracts (docs/MISSIONS.md, "The packs"; docs/INFECTED.md, "People"). Each holds its
//! pack still with a claim ("tribe:N"), counts its research x0.75 through the lab's backlog, and is
//! offered only within an hour of play before its window, so it can't outlive its night. Halcyon's
//! stop with the cure; Bring Them Home, Cull and Clear the Trail run on after it. Player text never
//! says "alpha", "tribe" or "crawler": each giver uses its own word, always with the place.

//------------------------------------------------------------------------------------------------
//! The base: the pack it's about, its offer and window, and the kills, tags and fires the packs
//! report (RZ_Tribes.OnMemberKilled, RZ_TagAction, RZ_Tribes.OnFire, RZ_TribeBorders.Resolve).
class RZ_TribeContract : RZ_Mission
{
	protected int m_iTribe = -1;
	protected float m_fOfferFrom = -1;		// game hour the offer opens (-1: any time)
	protected float m_fOfferHours = 1;		// and for how long
	protected float m_fWindowEnd = -1;		// game hour the window closes: time up then (-1: m_fTimeLeft)

	//------------------------------------------------------------------------------------------------
	//! The claim subject that holds a pack still: no lair move or split while it's held.
	static string Claim(int tribeId)
	{
		return "tribe:" + tribeId.ToString();
	}

	//------------------------------------------------------------------------------------------------
	static bool IsHeld(RZ_GameModeComponent gm, RZ_Tribe t)
	{
		return gm && t && gm.GetFacts() && !gm.GetFacts().ClaimedBy(Claim(t.m_iId)).IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	static RZ_Tribes Tribes(RZ_GameModeComponent gm)
	{
		RZ_Tribes tribes = gm.GetTribes();
		if (!tribes || !tribes.IsEnabled() || !tribes.GetPeople().IsOn())
			return null;

		return tribes;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_TribePeople People(RZ_GameModeComponent gm)
	{
		RZ_Tribes tribes = Tribes(gm);
		if (!tribes)
			return null;

		return tribes.GetPeople();
	}

	//------------------------------------------------------------------------------------------------
	RZ_Tribe GetTribe(RZ_GameModeComponent gm)
	{
		RZ_Tribes tribes = Tribes(gm);
		if (!tribes || m_iTribe < 0)
			return null;

		return tribes.Find(m_iTribe);
	}

	//------------------------------------------------------------------------------------------------
	//! Offered now: the packs are on and the clock is within the offer.
	protected bool InOffer(RZ_GameModeComponent gm)
	{
		if (!Tribes(gm))
			return false;

		if (m_fOfferFrom < 0)
			return true;

		float h = RZ_GameTime.GetHour();
		return h >= 0 && RZ_GameTime.IsWithin(h, m_fOfferFrom, m_fOfferHours);
	}

	//------------------------------------------------------------------------------------------------
	//! In Setup: holds the pack, and times the job to its window.
	protected bool TakeTribe(RZ_GameModeComponent gm, RZ_Tribe t)
	{
		if (!t || !ClaimSubject(gm, Claim(t.m_iId)))
			return false;

		m_iTribe = t.m_iId;
		if (m_fWindowEnd >= 0)
		{
			float secs = RZ_GameTime.RealSecondsUntil(m_fWindowEnd);
			if (secs > 0)
				m_fTimeLeft = secs + 30;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A free pack (not held by another story) that passes `ok`.
	protected bool Free(RZ_GameModeComponent gm, RZ_Tribe t)
	{
		return t && !t.IsRover() && IsSubjectFree(gm, Claim(t.m_iId));
	}

	//------------------------------------------------------------------------------------------------
	protected void AddParticipant(int playerId)
	{
		if (playerId > 0 && !m_aParticipants.Contains(playerId))
			m_aParticipants.Insert(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Research through the lab's backlog, x0.75 like every pack job's.
	protected void Research(RZ_GameModeComponent gm, float points, string reason)
	{
		if (gm.GetCure())
			gm.GetCure().AddResearch(points * 0.75, reason);
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest player within radius of at (an id), or -1.
	static int NearestPlayer(vector at, float radius)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		int best = -1;
		float bestD = radius;
		foreach (int i, IEntity p : players)
		{
			float d = RZ_Util.DistXZ(p.GetOrigin(), at);
			if (d <= bestD)
			{
				bestD = d;
				best = ids[i];
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Is the pack after this player (a hunt or a scout's pack)? "It noticed you."
	static bool Noticed(RZ_Tribe t, int playerId)
	{
		if (t.m_Hunt && t.m_Hunt.m_iTarget == playerId)
			return true;

		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (p.m_iTarget == playerId)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A habit of a kind loses strength (gone under 10, as the habits' own fading).
	static void WeakenHabit(RZ_Tribe t, RZ_EHabit kind, float by)
	{
		for (int i = t.m_aHabits.Count() - 1; i >= 0; i--)
		{
			RZ_TribeHabit h = t.m_aHabits[i];
			if (h.m_eKind != kind)
				continue;

			h.m_fStrength -= by;
			if (h.m_fStrength < 10)
				t.m_aHabits.Remove(i);
		}
	}

	// ================================================================ Reports from the packs
	//------------------------------------------------------------------------------------------------
	void OnTribeKill(RZ_GameModeComponent gm, RZ_Tribe t, RZ_EInfectedRole role, RZ_TribePart p, vector at, bool silent)
	{
	}

	//------------------------------------------------------------------------------------------------
	RZ_NamedYoung Wanted(RZ_GameModeComponent gm, RZ_Tribe t, vector at)
	{
		return null;
	}

	//------------------------------------------------------------------------------------------------
	void OnTag(RZ_GameModeComponent gm, RZ_Tribe t, int playerId)
	{
	}

	//------------------------------------------------------------------------------------------------
	void OnFireNear(RZ_GameModeComponent gm, vector pos, int playerId)
	{
	}

	//------------------------------------------------------------------------------------------------
	void OnBorderFight(RZ_GameModeComponent gm, RZ_Tribe winner, RZ_Tribe loser)
	{
	}

	//------------------------------------------------------------------------------------------------
	protected static void Running(RZ_GameModeComponent gm, notnull array<RZ_TribeContract> result)
	{
		if (!gm || !gm.GetMissions())
			return;

		foreach (RZ_Mission m : gm.GetMissions().GetActive())
		{
			RZ_TribeContract c = RZ_TribeContract.Cast(m);
			if (c && c.m_eState == RZ_EMissionState.ACTIVE)
				result.Insert(c);
		}
	}

	//------------------------------------------------------------------------------------------------
	static void OnKill(RZ_GameModeComponent gm, RZ_Tribe t, RZ_EInfectedRole role, RZ_TribePart p, vector at, bool silent)
	{
		array<RZ_TribeContract> list = {};
		Running(gm, list);
		foreach (RZ_TribeContract c : list)
		{
			c.OnTribeKill(gm, t, role, p, at, silent);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A young put down: the named young a running Bring Them Home wants, or null.
	static RZ_NamedYoung WantedYoung(RZ_GameModeComponent gm, RZ_Tribe t, vector at)
	{
		array<RZ_TribeContract> list = {};
		Running(gm, list);
		foreach (RZ_TribeContract c : list)
		{
			RZ_NamedYoung y = c.Wanted(gm, t, at);
			if (y)
				return y;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	static void OnTagged(RZ_GameModeComponent gm, RZ_Tribe t, int playerId)
	{
		array<RZ_TribeContract> list = {};
		Running(gm, list);
		foreach (RZ_TribeContract c : list)
		{
			c.OnTag(gm, t, playerId);
		}
	}

	//------------------------------------------------------------------------------------------------
	static void OnFire(RZ_GameModeComponent gm, vector pos, int playerId)
	{
		array<RZ_TribeContract> list = {};
		Running(gm, list);
		foreach (RZ_TribeContract c : list)
		{
			c.OnFireNear(gm, pos, playerId);
		}
	}

	//------------------------------------------------------------------------------------------------
	static void OnBorder(RZ_GameModeComponent gm, RZ_Tribe winner, RZ_Tribe loser)
	{
		array<RZ_TribeContract> list = {};
		Running(gm, list);
		foreach (RZ_TribeContract c : list)
		{
			c.OnBorderFight(gm, winner, loser);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! Not Tonight (Haven, anyone, unarmed is fine; ★ Radio Operator: more Fame; offered from 17:00): a
//! hunting party will reach a house's wire tonight (a back-door habit, or a hungry pack testing the
//! wire). Before 21:00, make noise at a marked spot off its way and get clear: it turns for it. If
//! it reaches the wire anyway, two people are lost from the house.
class RZ_Mission_NotTonight : RZ_TribeContract
{
	protected int m_iHouse = -1;
	protected bool m_bLured;
	protected float m_fClearFor;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 20, 20);
		m_Req.m_bNotCured = true;
		m_Req.Bonus("radioham");
		m_fBonusFameMult = 1.5;
		m_fOfferFrom = 17;
		m_fWindowEnd = 21;
		m_fParticipationRadius = 80;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!InOffer(gm) || RZ_ContractHelp.CountSafehouses(gm) == 0)
			return 0;

		return 1.2;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Tribes tribes = Tribes(gm);
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!tribes || !houses)
			return false;

		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			if (!Free(gm, t) || t.IsWeakened())
				continue;

			RZ_TribeHabit back = t.StrongHabit(RZ_EHabit.BACKDOOR);
			bool testing = t.Band() >= 2 && t.m_eTemper != RZ_ETemperament.TERRITORIAL;
			if (!back && !testing)
				continue;

			vector toward = t.m_vLair;
			if (back)
				toward = back.m_vPos;

			int house = houses.FindNearest(toward, 2500);
			RZ_Safehouse h = houses.Get(house);
			if (!h || !TakeTribe(gm, t))
				continue;

			m_iHouse = house;
			vector mid = RZ_ContractHelp.Between(t.m_vLair, h.m_vPos, 0.5);
			vector dir = RZ_TribePeople.Dir(t.m_vLair, h.m_vPos);
			vector side = Vector(dir[2], 0, -dir[0]);
			if (Math.RandomFloat01() < 0.5)
				side = side * -1;

			m_vPos = LandPointNear(mid + side * 600, 0, 80);
			RZ_TribePeople people = tribes.GetPeople();
			m_sTitle = "Not Tonight";
			m_sDeed = "drew the dark off a house";
			m_sBrief = string.Format("%1 will be at the wire of the house in %2 tonight. Before nine, make a noise at the marked spot, well off their way, and get clear. They'll turn for it. Nobody needs to fire a shot.", RZ_Tribes.Capitalize(people.NameAt(t, RZ_EFaction.SURVIVORS)), h.m_sName);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_Tribe t = GetTribe(gm);
		if (!t)
		{
			Withdraw("Nothing comes to that wire any more.");
			return;
		}

		if (!m_bLured)
		{
			vector noise;
			if (CountPlayersNear(players, m_vPos, 60) > 0 && gm.GetHeat().FindRecentNoise(m_vPos, 50, 60, noise))
			{
				m_bLured = true;
				t.m_iNoWireNight = Tribes(gm).CountedNights();
				WeakenHabit(t, RZ_EHabit.BACKDOOR, 30);
				if (Tribes(gm).GetPhase() == RZ_ETribePhase.NIGHT)
					Tribes(gm).DrawTo(t, m_vPos, true);

				NotifyParticipants(gm, m_sTitle, "Far off in the trees, a call, and then another, nearer. They heard it. Get clear.");
			}

			return;
		}

		// Clear: nobody within 300 m for 20 s
		if (CountPlayersNear(players, m_vPos, 300) == 0)
			m_fClearFor += dt;
		else
			m_fClearFor = 0;

		if (m_fClearFor >= 20)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	//! Nine o'clock: they reached the wire. Two people lost from the house.
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		if (m_bLured)
		{
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		m_eState = RZ_EMissionState.FAILED;
		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses && houses.Get(m_iHouse))
			houses.RemoveDead(m_iHouse, 2);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "Not Tonight: the calls went off the other way, and the house kept its lamps on.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Not Tonight: they came to the wire. Two from the house went out to the gate and didn't come back.";
	}
}

//------------------------------------------------------------------------------------------------
//! Bring Them Home (Haven, Known; ★ Elder, ★ Nurse): someone named runs with a pack's young (a
//! walk-out, a runner, a wanderer, a player's character). Find them with the young (by day perhaps
//! out with the walkers), put them down, and bring what's in their pockets (the mission item) to any
//! house. Never for someone in your group, someone who died within 300 m of you or under a season
//! day ago; one delivery per player a season day. After the cure it's closing work.
class RZ_Mission_BringThemHome : RZ_TribeContract
{
	protected RZ_NamedYoung m_Young;	// weak: the pack's record holds it until it's put down
	protected ref RZ_NamedYoung m_Kept;	// once put down, the mission keeps it
	protected bool m_bDown;
	protected int m_iBy = -1;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 25, 25);
		m_Req.Bonus("elder");
		m_Req.Bonus("nurse");
		m_fTimeLeft = 5400;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!InOffer(gm) || gm.GetMissionItemPrefab().IsEmpty() || RZ_ContractHelp.CountSafehouses(gm) == 0)
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Tribes tribes = Tribes(gm);
		if (!tribes)
			return false;

		array<int> ids = {};
		GetGame().GetPlayerManager().GetPlayers(ids);
		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			if (!Free(gm, t))
				continue;

			foreach (RZ_NamedYoung y : t.m_aNamed)
			{
				bool someone = false;
				foreach (int id : ids)
				{
					if (tribes.GetPeople().CanBringHome(id, y))
						someone = true;
				}

				if (!someone || !TakeTribe(gm, t))
					continue;

				m_Young = y;
				m_vPos = t.m_vLair;
				string who = tribes.GetPeople().PublicNameOf(y);
				m_sTitle = "Bring Them Home";
				m_sDeed = "brought someone home";
				m_sBrief = string.Format("%1 runs with %2 now. Find them with the young ones (by day they may be out walking with a few others), lay them down, and bring back what's in their pockets to any of our houses. We'll read the name.", RZ_Tribes.Capitalize(who), tribes.GetPeople().NameAt(t, RZ_EFaction.SURVIVORS));
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsEligible(RZ_GameModeComponent gm, int playerId)
	{
		if (!super.IsEligible(gm, playerId))
			return false;

		RZ_TribePeople people = People(gm);
		if (!people)
			return false;

		if (m_bDown)
			return true;

		return people.CanBringHome(playerId, m_Young);
	}

	//------------------------------------------------------------------------------------------------
	override RZ_NamedYoung Wanted(RZ_GameModeComponent gm, RZ_Tribe t, vector at)
	{
		if (m_bDown || t.m_iId != m_iTribe || t.m_aNamed.Find(m_Young) < 0)
			return null;

		int by = NearestPlayer(at, 100);
		if (by <= 0 || !IsEligible(gm, by))
			return null;

		IEntity item = RZ_Util.SpawnPrefab(gm.GetMissionItemPrefab(), RZ_Util.GroundPos(at) + Vector(0, 0.3, 0));
		if (!item)
			return null;

		RZ_MissionItemComponent.Tag(item, m_iId);
		m_bDown = true;
		m_iBy = by;
		m_Kept = m_Young;
		m_vPos = at;
		AddParticipant(by);
		gm.NotifyPlayer(by, m_sTitle, "Take what's in the pockets. Any of the houses will do.");
		return m_Young;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_Tribe t = GetTribe(gm);
		if (!m_bDown)
		{
			if (!t || t.m_aNamed.Find(m_Young) < 0)
			{
				Withdraw("Someone else laid them down first.");
				return;
			}

			RZ_TribePart young = t.FindPart(RZ_ETribePart.YOUNG);
			if (young)
				m_vPos = young.m_vPos;

			return;
		}

		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses)
			return;

		foreach (int i, IEntity p : players)
		{
			int h = houses.FindNearest(p.GetOrigin(), 25);
			if (h < 0 || !RZ_ContractHelp.CarriesItemOf(p, m_iId, true))
				continue;

			m_iBy = ids[i];
			AddParticipant(ids[i]);
			People(gm).Delivered(ids[i]);
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_TribePeople people = People(gm);
		if (!people || !m_Kept)
			return;

		string name = people.PublicNameOf(m_Kept);
		people.AddBeat(string.Format("Brought home: %1, carried back to the houses.", name));
		array<int> all = {};
		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int playerId : all)
		{
			if (RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.SURVIVORS), RZ_ETier.KNOWN))
				gm.NotifyPlayer(playerId, "The Haven", string.Format("At every house tonight they read a name: %1. Someone brought them home.", name));
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "Bring Them Home: the card is on the table at the house, and the kettle's on.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Bring Them Home: nobody brought them back in time.";
	}
}

//------------------------------------------------------------------------------------------------
//! Escort at Dawn (Haven, Known; ★ Hunter; offered from 03:30): walk 3-5 people from a house across
//! a pack's ground to another house between 05:00 and 06:30, while the pack walks home and before
//! its walkers are out. One is lost for each group of them that comes within 150 m of them.
class RZ_Mission_EscortAtDawn : RZ_TribeContract
{
	protected ref RZ_Escort m_Escort = new RZ_Escort();
	protected int m_iTo = -1;
	protected int m_iPeople;
	protected bool m_bMet;
	protected string m_sDestName;
	protected ref array<RZ_TribePart> m_aMet = {};

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 30, 25);
		m_Req.m_bNotCured = true;
		m_Req.Bonus("hunter");
		m_fOfferFrom = 3.5;
		m_fWindowEnd = 6.5;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!InOffer(gm) || RZ_ContractHelp.CountSafehouses(gm) < 2 || !RZ_Escort.HasRoom(gm))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Tribes tribes = Tribes(gm);
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!tribes || !houses)
			return false;

		for (int a = 0; a < houses.Count(); a++)
		{
			for (int b = 0; b < houses.Count(); b++)
			{
				RZ_Safehouse ha = houses.Get(a);
				RZ_Safehouse hb = houses.Get(b);
				float d = RZ_Util.DistXZ(ha.m_vPos, hb.m_vPos);
				if (a == b || d < 800 || d > 3500)
					continue;

				foreach (RZ_Tribe t : tribes.GetTribeList())
				{
					if (!Free(gm, t) || !RZ_TribePeople.Between(t.RangeCentre(), ha.m_vPos, hb.m_vPos, t.m_fRadius) || !TakeTribe(gm, t))
						continue;

					m_iTo = b;
					m_iPeople = Math.RandomIntInclusive(3, 5);
					m_vPos = LandPointNear(ha.m_vPos, 6, 14);
					m_sDestName = "the house in " + hb.m_sName;
					array<vector> route = {RZ_ContractHelp.Between(m_vPos, hb.m_vPos, 0.5), hb.m_vPos};
					m_Escort.m_sName = "the walkers";
					m_Escort.m_sPrefab = RZ_Escort.PrefabFor(gm, 0);
					m_Escort.SetRoute(route);
					m_sTitle = "Escort at Dawn";
					m_sDeed = "walked people across at first light";
					m_sBrief = string.Format("%1 people at the house in %2 need to get to %3, and the way crosses %4. Go at five, while they walk home, and before the walkers are out. Keep off their paths.", m_iPeople, ha.m_sName, m_sDestName, tribes.GetPeople().NameAt(t, RZ_EFaction.SURVIVORS));
					return m_Escort.Begin(gm, m_vPos);
				}
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_ContractHelp.AddNear(ids, players, m_Escort.m_vPos, 120, m_aParticipants);
		float h = RZ_GameTime.GetHour();
		if (!m_bMet && h >= 5 && h < 7 && CountPlayersNear(players, m_Escort.m_vPos, 30) > 0)
		{
			m_bMet = true;
			m_Escort.Go();
			RZ_ContractHelp.NotifyNear(gm, m_Escort.m_vPos, 60, m_sTitle, string.Format("They're ready at the door, coats on. Head for %1, and keep off the paths.", m_sDestName));
		}

		m_Escort.Update(gm, dt, ids, players);
		m_vPos = m_Escort.m_vPos;
		RZ_Tribe t = GetTribe(gm);
		if (m_bMet && t)
		{
			foreach (RZ_TribePart p : t.m_aParts)
			{
				if (p.m_iCount <= 0 || m_aMet.Find(p) >= 0 || RZ_Util.DistXZ(p.m_vPos, m_Escort.m_vPos) > 150)
					continue;

				m_aMet.Insert(p);
				m_iPeople--;
				RZ_ContractHelp.NotifyNear(gm, m_Escort.m_vPos, 150, m_sTitle, "Shapes in the half-light, coming through the field. Someone at the back doesn't make it.");
			}
		}

		if (m_Escort.IsDead() || m_iPeople <= 0)
			m_eState = RZ_EMissionState.FAILED;
		else if (m_Escort.IsArrived())
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, m_iTo);
		if (h)
			gm.GetSafehouses().AddRescued(h.m_vPos, m_iPeople, m_aParticipants);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Escort at Dawn: nobody reached " + m_sDestName + ".";
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		m_Escort.Cleanup();
	}
}

//------------------------------------------------------------------------------------------------
//! Contact Report (Bastion, anyone, an amends contract; ★ Hunter, ★ Radio Operator, ★ Soldier;
//! offered from 17:30): at dusk, listen 30 s at each of two points within 600 m of a pack's lair
//! without its scouts or its pack noticing you. The pack is Mapped for you and goes on Bastion's
//! contact board (Read for Bastion's Known).
class RZ_Mission_ContactReport : RZ_TribeContract
{
	protected vector m_vSecond;
	protected float m_fFirst;
	protected float m_fSecondHeard;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.STRANGER, 20, 15);
		m_bAmends = true;
		m_Req.m_bAllowWatched = true;
		m_Req.m_bNotCured = true;
		m_Req.Bonus("hunter");
		m_Req.Bonus("radioham");
		m_Req.Bonus("soldier");
		m_fOfferFrom = 17.5;
		m_fWindowEnd = 20.5;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!InOffer(gm))
			return 0;

		return 1.2;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Tribes tribes = Tribes(gm);
		if (!tribes)
			return false;

		RZ_Tribe pick;
		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			if (Free(gm, t) && t.m_bHasAlpha && (!pick || (pick.m_iContact > 0 && t.m_iContact <= 0)))
				pick = t;
		}

		if (!TakeTribe(gm, pick))
			return false;

		m_vPos = LandPointNear(pick.m_vLair, 350, 600);
		for (int i = 0; i < 8; i++)
		{
			m_vSecond = LandPointNear(pick.m_vLair, 350, 600);
			if (RZ_Util.DistXZ(m_vSecond, m_vPos) >= 250)
				break;
		}

		m_sTitle = "Contact Report";
		m_sDeed = "counted the calls for Bastion";
		m_sBrief = string.Format("Count the calls at dusk from two listening points near %1: thirty seconds at each. If anything up there sees you, the count's no good. Report it in.", tribes.GetPeople().NameAt(pick, RZ_EFaction.MILITARY));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_Tribe t = GetTribe(gm);
		if (!t)
		{
			Withdraw("There's nothing left there to count.");
			return;
		}

		bool dusk = Tribes(gm).GetPhase() == RZ_ETribePhase.DUSK;
		foreach (int i, IEntity p : players)
		{
			if (!IsEligible(gm, ids[i]))
				continue;

			if (Noticed(t, ids[i]) && m_aParticipants.Contains(ids[i]))
			{
				m_eState = RZ_EMissionState.FAILED;
				return;
			}

			if (!dusk)
				continue;

			if (RZ_Util.DistXZ(p.GetOrigin(), m_vPos) <= 30)
			{
				AddParticipant(ids[i]);
				m_fFirst += dt;
			}
			else if (RZ_Util.DistXZ(p.GetOrigin(), m_vSecond) <= 30)
			{
				AddParticipant(ids[i]);
				m_fSecondHeard += dt;
			}
		}

		if (m_fFirst >= 30 && m_fSecondHeard >= 30)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Tribe t = GetTribe(gm);
		RZ_TribePeople people = People(gm);
		if (!t || !people)
			return;

		people.AddToBoard(t);
		foreach (int playerId : m_aParticipants)
		{
			people.Learn(playerId, t, RZ_EKnow.MAPPED, RZ_EFaction.MILITARY);
		}

		people.BoardToKnown(t);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		RZ_Tribe t = GetTribe(gm);
		RZ_TribePeople people = People(gm);
		if (!t || !people)
			return "Contact Report: logged.";

		return "Contact Report: " + people.ContactLine(t) + " Logged.";
	}
}

//------------------------------------------------------------------------------------------------
//! Clear the Trail (Bastion, Trusted; presence 0.2+; offered at the set-out): kill 6 of a pack in
//! one night on a trail within 3 km of a Bastion holding. The trail is dropped for 3 counted
//! nights, the habit there falls 50, and the pack is disturbed (+30).
class RZ_Mission_ClearTheTrail : RZ_TribeContract
{
	protected int m_iTrail = -1;
	protected int m_iKills;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.TRUSTED, 35, 25);
		m_Req.m_fMinMilitary = 0.2;
		m_fOfferFrom = 19.5;
		m_fWindowEnd = 5;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!InOffer(gm) || !gm.GetTerritory())
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Tribes tribes = Tribes(gm);
		RZ_Territory territory = gm.GetTerritory();
		if (!tribes || !territory)
			return false;

		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			if (!Free(gm, t))
				continue;

			foreach (int i, array<vector> trail : t.m_aTrails)
			{
				if (trail.IsEmpty())
					continue;

				vector mid = trail[trail.Count() / 2];
				if (!territory.FindHolding(RZ_EFaction.MILITARY, mid, 3000) || !TakeTribe(gm, t))
					continue;

				m_iTrail = i;
				m_vPos = mid;
				m_sTitle = "Clear the Trail";
				m_sDeed = "cleared a trail for Bastion";
				m_sBrief = string.Format("%1 come down a path near one of our posts most nights. Tonight, put six of them down on it. Make it not worth their while.", RZ_Tribes.Capitalize(tribes.GetPeople().NameAt(t, RZ_EFaction.MILITARY)));
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnTribeKill(RZ_GameModeComponent gm, RZ_Tribe t, RZ_EInfectedRole role, RZ_TribePart p, vector at, bool silent)
	{
		if (silent || t.m_iId != m_iTribe || m_iTrail < 0 || m_iTrail >= t.m_aTrails.Count())
			return;

		bool onIt = false;
		foreach (vector point : t.m_aTrails[m_iTrail])
		{
			if (RZ_Util.DistXZ(point, at) <= 300)
				onIt = true;
		}

		int by = NearestPlayer(at, 200);
		if (!onIt || by <= 0)
			return;

		AddParticipant(by);
		m_iKills++;
		if (m_iKills < 6)
			return;

		t.m_iTrailOff = m_iTrail;
		t.m_iTrailOffUntil = Tribes(gm).CountedNights() + 3;
		WeakenHabit(t, RZ_EHabit.CROSSING, 50);
		WeakenHabit(t, RZ_EHabit.ROAD, 50);
		Tribes(gm).GetLife().Disturb(t, 30, RZ_EFaction.MILITARY);
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "Clear the Trail: six down on the path. They'll find another way for a while.";
	}
}

//------------------------------------------------------------------------------------------------
//! Cull (Bastion, Trusted, Champion for a leader in its prime or older; presence 0.2+): kill the
//! leader, or 3 of its guard, of a pack within 3 km of a Bastion holding. Bastion's smoke is allowed.
//! Disturbance 100: the pack is pushed off, away from Bastion, and the Haven hears where it went.
class RZ_Mission_Cull : RZ_TribeContract
{
	protected int m_iGuards;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.TRUSTED, 50, 35);
		m_Req.m_fMinMilitary = 0.2;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!InOffer(gm) || !gm.GetTerritory())
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Tribes tribes = Tribes(gm);
		RZ_Territory territory = gm.GetTerritory();
		if (!tribes || !territory)
			return false;

		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			if (!Free(gm, t) || !t.m_bHasAlpha || t.Members() < 12 || !territory.FindHolding(RZ_EFaction.MILITARY, t.RangeCentre(), 3000))
				continue;

			if (!TakeTribe(gm, t))
				continue;

			if (t.Stage() >= 1)
				m_Req.m_eMinTier = RZ_ETier.CHAMPION;

			m_vPos = t.m_vLair;
			m_sTitle = "Cull";
			m_sDeed = "culled a pack for Bastion";
			m_sBrief = string.Format("%1 is too close to our post. Kill the heavy, or three of the ones round it. Smoke's authorised. Bastion out.", RZ_Tribes.Capitalize(tribes.GetPeople().NameAt(t, RZ_EFaction.MILITARY)));
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnTribeKill(RZ_GameModeComponent gm, RZ_Tribe t, RZ_EInfectedRole role, RZ_TribePart p, vector at, bool silent)
	{
		if (t.m_iId != m_iTribe)
			return;

		int by = NearestPlayer(at, 400);
		if (by > 0)
			AddParticipant(by);

		if (role == RZ_EInfectedRole.GUARD)
			m_iGuards++;

		if (role == RZ_EInfectedRole.ALPHA || m_iGuards >= 3)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Tribe t = GetTribe(gm);
		RZ_TribePeople people = People(gm);
		if (!t || !people)
			return;

		string was = t.m_sPlace;
		Tribes(gm).GetLife().Disturb(t, 100, RZ_EFaction.MILITARY);
		people.TellFaction(RZ_EFaction.MILITARY, "Task Force Bastion", "cull", t, string.Format("%1, culled. Not our sector now. Bastion out.", people.NameAt(t, RZ_EFaction.MILITARY)));
		if (t.m_sPlace != was)
			people.TellFaction(RZ_EFaction.SURVIVORS, "The Haven", "moved", t, string.Format("\"Bastion's been at %1. What's left of them is at %2 now, and that's our side of the valley.\"", was, t.m_sPlace));

		people.AddBeat(string.Format("Culled: the leader of %1, for Bastion.", people.NameAt(t, RZ_EFaction.MILITARY)));
	}
}

//------------------------------------------------------------------------------------------------
//! Burn the Lair (Haven or Bastion, Trusted; needs fire on; offered 09:00-13:00): a lair someone has
//! Mapped. Between 11:00 and 14:00, while the walkers are out and the rest lie up, set fire within
//! 30 m of it and get out. The lair moves and the land is burned (the life cycle's fire).
class RZ_Mission_BurnTheLair : RZ_TribeContract
{
	protected bool m_bFired;
	protected string m_sLairPlace;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		RZ_EFaction giver = RZ_EFaction.SURVIVORS;
		if (Math.RandomFloat01() < 0.5)
			giver = RZ_EFaction.MILITARY;

		SetContract(giver, RZ_ETier.TRUSTED, 40, 30);
		m_Req.m_bNotCured = true;
		m_fOfferFrom = 9;
		m_fOfferHours = 4;
		m_fWindowEnd = 14;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		RZ_TrapSettings traps = gm.GetTrapSettings();
		if (!InOffer(gm) || !traps || !traps.m_bFire)
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Tribes tribes = Tribes(gm);
		if (!tribes)
			return false;

		array<int> ids = {};
		GetGame().GetPlayerManager().GetPlayers(ids);
		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			if (!Free(gm, t))
				continue;

			bool mapped = false;
			foreach (int id : ids)
			{
				if (tribes.GetPeople().Level(id, t) >= RZ_EKnow.MAPPED)
					mapped = true;
			}

			if (!mapped || !TakeTribe(gm, t))
				continue;

			m_vPos = t.m_vLair;
			m_sLairPlace = t.m_sPlace;
			m_sTitle = "Burn the Lair";
			m_sDeed = "burned out a lair";
			m_sBrief = string.Format("You know where %1 lie up. Between eleven and two, while the walkers are out and the rest are asleep, set a fire within thirty metres of it, then get out before the big ones are up.", Name(gm, t));
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected string Name(RZ_GameModeComponent gm, RZ_Tribe t)
	{
		return People(gm).NameAt(t, m_eGiver);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsEligible(RZ_GameModeComponent gm, int playerId)
	{
		RZ_TribePeople people = People(gm);
		RZ_Tribe t = GetTribe(gm);
		if (!super.IsEligible(gm, playerId) || !people)
			return false;

		return !t || people.Level(playerId, t) >= RZ_EKnow.MAPPED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnFireNear(RZ_GameModeComponent gm, vector pos, int playerId)
	{
		RZ_Tribe t = GetTribe(gm);
		float h = RZ_GameTime.GetHour();
		if (!t || m_bFired || h < 11 || h >= 14 || RZ_Util.DistXZ(pos, t.m_vLair) > 30)
			return;

		m_bFired = true;
		if (playerId > 0)
			AddParticipant(playerId);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_bFired && CountPlayersNear(players, m_vPos, 150) == 0)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_TribePeople people = People(gm);
		if (people)
			people.AddBeat(string.Format("Burned out: the lair at %1.", m_sLairPlace));
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "Burn the Lair: smoke over the trees, and nothing will lie up there for a while.";
	}
}

//------------------------------------------------------------------------------------------------
//! Break the Habit (Bastion or the Red Hand, whoever's trucks use the road; Known; offered an hour
//! before the habit's hour): a pack that hunts a supply road at an hour. Be there at its hour and
//! kill 4 of its hunters there. The habit falls 60 (gone under 10, so usually two contracts).
class RZ_Mission_BreakTheHabit : RZ_TribeContract
{
	protected int m_iKills;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		RZ_EFaction giver = RZ_EFaction.MILITARY;
		if (Math.RandomFloat01() < 0.5)
			giver = RZ_EFaction.BANDITS;

		SetContract(giver, RZ_ETier.KNOWN, 30, 25);
		m_Req.m_bNotCured = true;
		m_fParticipationRadius = 0;
		m_fTimeLeft = 2400;
	}

	//------------------------------------------------------------------------------------------------
	//! A road habit whose hour is within the next game hour.
	protected RZ_Tribe DueHabit(RZ_GameModeComponent gm, out RZ_TribeHabit habit)
	{
		RZ_Tribes tribes = Tribes(gm);
		float h = RZ_GameTime.GetHour();
		if (!tribes || h < 0)
			return null;

		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			RZ_TribeHabit road = t.StrongHabit(RZ_EHabit.ROAD);
			if (road && Free(gm, t) && RZ_GameTime.IsWithin(h, road.m_fHour - 1, 1))
			{
				habit = road;
				return t;
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		RZ_TribeHabit habit;
		if (!InOffer(gm) || !DueHabit(gm, habit))
			return 0;

		return 1.2;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_TribeHabit habit;
		RZ_Tribe t = DueHabit(gm, habit);
		if (!t)
			return false;

		m_fWindowEnd = habit.m_fHour + 2;
		if (m_fWindowEnd >= 24)
			m_fWindowEnd -= 24;

		if (!TakeTribe(gm, t))
			return false;

		m_vPos = habit.m_vPos;
		m_sTitle = "Break the Habit";
		m_sDeed = "broke a pack of a bad habit";
		m_sBrief = string.Format("%1 have learned the trucks' hour on the road by %2. Be there at %3 and put four of them down. They'll learn something else.", RZ_Tribes.Capitalize(People(gm).NameAt(t, m_eGiver)), GetLocationName(gm), RZ_GameTime.Clock(habit.m_fHour));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnTribeKill(RZ_GameModeComponent gm, RZ_Tribe t, RZ_EInfectedRole role, RZ_TribePart p, vector at, bool silent)
	{
		if (silent || t.m_iId != m_iTribe || RZ_Util.DistXZ(at, m_vPos) > 200)
			return;

		if (role != RZ_EInfectedRole.HUNTER && role != RZ_EInfectedRole.DAY_PARTY)
			return;

		int by = NearestPlayer(at, 200);
		if (by <= 0)
			return;

		AddParticipant(by);
		m_iKills++;
		if (m_iKills < 4)
			return;

		WeakenHabit(t, RZ_EHabit.ROAD, 60);
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "Break the Habit: four down on the road at their hour. The trucks might get through next time.";
	}
}

//------------------------------------------------------------------------------------------------
//! Bait the Crew (Red Hand, Trusted; ★ Radio Operator: twice the noise; offered at dusk, only with no
//! other player within 500 m of the border): make enough noise across the border between two packs
//! that one's hunters cross into the other's ground: a Border Night with you in it. The weaker loses.
class RZ_Mission_BaitTheCrew : RZ_TribeContract
{
	protected int m_iOther = -1;
	protected float m_fNoise;
	protected bool m_bStaged;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.TRUSTED, 35, 30);
		m_Req.m_bNotCured = true;
		m_Req.Bonus("radioham");
		m_fOfferFrom = 18.5;
		m_fWindowEnd = 5;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!InOffer(gm))
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Tribes tribes = Tribes(gm);
		if (!tribes)
			return false;

		foreach (RZ_Tribe a : tribes.GetTribeList())
		{
			foreach (RZ_Tribe b : tribes.GetTribeList())
			{
				if (a == b || !Free(gm, a) || !Free(gm, b) || !tribes.GetBorders().Bordering(a, b))
					continue;

				vector at;
				if (!tribes.GetBorders().BorderPoint(a, b, at) || NearestPlayer(at, 500) > 0)
					continue;

				if (!TakeTribe(gm, a) || !ClaimSubject(gm, Claim(b.m_iId)))
					continue;

				m_iOther = b.m_iId;
				m_vPos = at;
				RZ_TribePeople people = tribes.GetPeople();
				m_sTitle = "Bait the Crew";
				m_sDeed = "set two crews on each other";
				m_sBrief = string.Format("%1 and %2 don't get on. Make a racket on the line between them tonight, enough to bring one over, and let them sort it out. Then don't be there.", RZ_Tribes.Capitalize(people.NameAt(a, RZ_EFaction.BANDITS)), people.NameAt(b, RZ_EFaction.BANDITS));
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_Tribes tribes = Tribes(gm);
		RZ_Tribe a = GetTribe(gm);
		RZ_Tribe b;
		if (tribes)
			b = tribes.Find(m_iOther);

		if (!a || !b)
		{
			Withdraw("One of the crews isn't there any more.");
			return;
		}

		if (m_bStaged || tribes.GetPhase() == RZ_ETribePhase.DAY)
			return;

		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), m_vPos) > 150 || !IsEligible(gm, ids[i]))
				continue;

			AddParticipant(ids[i]);
			float heat = gm.GetHeat().GetHeat(m_vPos);
			if (HasBonus(gm, ids[i]))
				heat = heat * 2;

			m_fNoise = Math.Max(m_fNoise, heat);
		}

		if (m_fNoise < 40)
			return;

		m_bStaged = tribes.GetBorders().Provoke(a, b);
	}

	//------------------------------------------------------------------------------------------------
	override void OnBorderFight(RZ_GameModeComponent gm, RZ_Tribe winner, RZ_Tribe loser)
	{
		if (!m_bStaged)
			return;

		bool ours = (winner.m_iId == m_iTribe && loser.m_iId == m_iOther) || (winner.m_iId == m_iOther && loser.m_iId == m_iTribe);
		if (ours)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "Bait the Crew: a lot of shrieking on the line, and one crew fewer by morning. Lovely.";
	}
}

//------------------------------------------------------------------------------------------------
//! Plant a Tag (Halcyon, Known; ★ Lab Technician): a leader in its prime or older, by day or in its
//! sleep after a feed. Pick up the tag at the lab and use it within 8 m of the leader while the pack
//! hasn't noticed you (RZ_TagAction). Research +1; the pack is tagged. Needs the tag item.
class RZ_Mission_PlantATag : RZ_TribeContract
{
	protected IEntity m_Tag;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.KNOWN, 25, 25);
		m_Req.Bonus("labtech");
		m_fOfferFrom = 6;
		m_fOfferHours = 6;
		m_fWindowEnd = 18.5;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		RZ_Tribes tribes = Tribes(gm);
		if (!InOffer(gm) || !gm.HasLab() || tribes.GetSettings().m_sTagPrefab.IsEmpty())
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Tribes tribes = Tribes(gm);
		if (!tribes)
			return false;

		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			if (!Free(gm, t) || !t.m_bHasAlpha || t.Stage() < 1 || t.m_iTagDay >= 0 || !TakeTribe(gm, t))
				continue;

			m_Tag = RZ_Util.SpawnPrefab(tribes.GetSettings().m_sTagPrefab, LandPointNear(gm.GetLabPosition(), 4, 8) + Vector(0, 0.3, 0));
			if (!m_Tag)
				return false;

			m_vPos = t.m_vLair;
			m_sTitle = "Plant a Tag";
			m_sDeed = "tagged one for Halcyon";
			m_sBrief = string.Format("There's a tag waiting at the lab. We'd like it in %1's index, by day while it sleeps, without it noticing you. Close enough to touch. It doesn't hurt it. We checked.", tribes.GetPeople().NameAt(t, RZ_EFaction.INSTITUTE));
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnTag(RZ_GameModeComponent gm, RZ_Tribe t, int playerId)
	{
		if (t.m_iId != m_iTribe)
			return;

		AddParticipant(playerId);
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		Research(gm, 1, "A tag went in. Halcyon can follow one of them now, night and day.");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		if (m_Tag && m_eState != RZ_EMissionState.SUCCEEDED)
			RZ_GameModeComponent.TrackLooseItem(m_Tag);
	}
}

//------------------------------------------------------------------------------------------------
//! Track Them Home (Halcyon or Bastion, anyone, unarmed is fine; ★ Hunter; offered from 03:30): at
//! dawn, follow a pack walking home at 80-200 m to its lair without it noticing you. The lair is
//! Mapped for you; Bastion puts it on its board, Halcyon gets a little research.
class RZ_Mission_TrackThemHome : RZ_TribeContract
{
	protected float m_fLostFor;
	protected bool m_bFollowing;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		RZ_EFaction giver = RZ_EFaction.INSTITUTE;
		if (Math.RandomFloat01() < 0.5)
			giver = RZ_EFaction.MILITARY;

		SetContract(giver, RZ_ETier.STRANGER, 20, 15);
		m_Req.m_bNotCured = true;
		m_Req.Bonus("hunter");
		m_fOfferFrom = 3.5;
		m_fWindowEnd = 7;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!InOffer(gm))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Tribes tribes = Tribes(gm);
		if (!tribes)
			return false;

		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			RZ_TribePart walking = Walker(t);
			if (!Free(gm, t) || !walking || !TakeTribe(gm, t))
				continue;

			m_vPos = walking.m_vPos;
			m_sTitle = "Track Them Home";
			m_sDeed = "followed a pack home";
			m_sBrief = string.Format("%1 are out tonight. At first light they'll walk home. Follow them, not too close and not too far, and see where they lie up. If they see you, come away.", RZ_Tribes.Capitalize(tribes.GetPeople().NameAt(t, m_eGiver)));
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A group of it out on a trail (the one to follow), or null.
	protected static RZ_TribePart Walker(RZ_Tribe t)
	{
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (p.m_iCount > 0 && p.m_bOut && p.m_eKind != RZ_ETribePart.SCOUT)
				return p;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_Tribe t = GetTribe(gm);
		if (!t)
		{
			Withdraw("They never came home. Nothing there to follow.");
			return;
		}

		RZ_TribePart walker = Walker(t);
		if (walker)
			m_vPos = walker.m_vPos;

		float h = RZ_GameTime.GetHour();
		if (h < 5 || h >= 7)
			return;

		bool near = false;
		foreach (int i, IEntity p : players)
		{
			if (!IsEligible(gm, ids[i]))
				continue;

			if (Noticed(t, ids[i]) && m_aParticipants.Contains(ids[i]))
			{
				m_eState = RZ_EMissionState.FAILED;
				return;
			}

			float d = RZ_Util.DistXZ(p.GetOrigin(), m_vPos);
			if (d <= 200)
			{
				near = true;
				m_bFollowing = true;
				AddParticipant(ids[i]);
			}
		}

		// Home: nothing of it is out, and someone who followed is within 300 m of the lair
		if (!walker && m_bFollowing && CountPlayersNear(players, t.m_vLair, 300) > 0)
		{
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (!m_bFollowing)
			return;

		if (near)
			m_fLostFor = 0;
		else
			m_fLostFor += dt;

		if (m_fLostFor >= 90)
			m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Tribe t = GetTribe(gm);
		RZ_TribePeople people = People(gm);
		if (!t || !people)
			return;

		foreach (int playerId : m_aParticipants)
		{
			people.Learn(playerId, t, RZ_EKnow.MAPPED, m_eGiver);
		}

		if (m_eGiver == RZ_EFaction.MILITARY)
			people.AddToBoard(t);
		else
			Research(gm, 0.5, "Halcyon has the lair of another pack on its map.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Track Them Home: lost them in the trees, or they saw you first.";
	}
}

//------------------------------------------------------------------------------------------------
//! Capture the Index (Halcyon, Champion): a pack's leader, in its prime or older, taken alive with
//! Live Capture's rules: wound it to a third and get within 3 m to tie it, hold it 30 s while it
//! fights (its guard comes), then carry the case to the lab before it wakes. Live Capture's research.
//! The pack holds its vigil, takes a new leader, and goes Furious. The finale's live tissue builds
//! on it when the finale is built.
class RZ_Mission_CaptureTheIndex : RZ_TribeContract
{
	protected ref RZ_LiveCapture m_Capture = new RZ_LiveCapture();
	protected IEntity m_Drop;
	protected bool m_bTaken;

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		m_Capture.GetSpots(result);
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.CHAMPION, 60, 50);
		m_Req.m_bNotCured = true;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!InOffer(gm) || !gm.HasLab() || !RZ_LiveCapture.HasRoom(gm) || gm.GetDropPointPrefab(RZ_EFaction.INSTITUTE).IsEmpty())
			return 0;

		return 0.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Tribes tribes = Tribes(gm);
		if (!tribes || tribes.PrefabsFor(RZ_ETribePart.ALPHA, gm.GetSpawner().GetSettings()).IsEmpty())
			return false;

		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			if (!Free(gm, t) || !t.m_bHasAlpha || t.Stage() < 1 || t.m_iLentAlpha >= 0 || !TakeTribe(gm, t))
				continue;

			m_Drop = RZ_HalcyonContractHelp.SpawnLabDrop(gm);
			if (!m_Drop)
				return false;

			if (!m_Capture.Begin(gm, m_iId, -1, LandPointNear(t.m_vLair, 60, 120), t.m_vLair, 0))
				return false;

			m_Capture.UseOwnSpecial();
			t.m_iLentAlpha = m_iId;
			m_vPos = t.m_vLair;
			m_sTitle = "Capture the Index";
			m_sDeed = "took one alive for the lab";
			m_sBrief = string.Format("We need %1's index alive. Bring it down without killing it and tie it, hold it until the sedative takes (the rest will come for it), then the case to the lab before it wakes. We're sorry to ask.", tribes.GetPeople().NameAt(t, RZ_EFaction.INSTITUTE));
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_Tribe t = GetTribe(gm);
		if (!t && !m_bTaken)
		{
			Withdraw("The pack broke up. There's no index to take.");
			return;
		}

		// The leader's body is the one to take (it carries the mission while the job runs)
		if (t && !m_bTaken)
		{
			RZ_TribePart a = t.FindPart(RZ_ETribePart.ALPHA);
			if (a && a.m_Body)
			{
				a.m_Body.m_iMissionId = m_iId;
				a.m_Body.m_bSpecial = true;
				m_vPos = a.m_vPos;
			}
		}

		m_Capture.Update(gm, dt, ids, players);
		if (m_Capture.m_eState == RZ_ECaptureState.CAUGHT)
		{
			RZ_ContractHelp.AddNear(ids, players, m_Capture.m_vHeld, 30, m_aParticipants);
			if (!m_bTaken && t)
			{
				m_bTaken = true;
				People(gm).MakeFurious(t);
				t.m_iLentAlpha = -1;
			}
		}

		if (m_Capture.IsLost())
			m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnZombiesKilled(RZ_GameModeComponent gm, int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		if (!m_bTaken)
			m_Capture.OnSpecialKilled(gm, missionId, special);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnItemDelivered(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction, IEntity drop)
	{
		if (faction != RZ_EFaction.INSTITUTE || drop != m_Drop || !m_Capture.Deliver())
			return false;

		AddParticipant(playerId);
		m_eState = RZ_EMissionState.SUCCEEDED;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		Research(gm, 5, "An index is in the lab's isolation room, alive. Halcyon can watch how it leads.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Capture the Index: " + m_Capture.m_sLostReason;
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		RZ_Tribe t = GetTribe(gm);
		if (t && t.m_iLentAlpha == m_iId)
			t.m_iLentAlpha = -1;

		m_Capture.Cleanup();
		if (m_Drop)
			RZ_Util.DeleteUnit(m_Drop);
	}
}

//------------------------------------------------------------------------------------------------
//! Stock the Pit (Red Hand, Known): two of a pack's young alive, one after the other, for the next
//! Fight Pit ("Fresh hires, from the Postmen."). Live Capture's rules: bring it down without killing
//! it and tie it, hold it 30 s while it fights, then the case to the Red Hand's drop by their nearest
//! camp (within 6 km of the lair) before it wakes. Each one caught leaves the pack. Needs the drop
//! point and mission item prefabs, and a pack with two young.
class RZ_Mission_StockThePit : RZ_TribeContract
{
	protected ref RZ_LiveCapture m_Capture = new RZ_LiveCapture();
	protected IEntity m_Drop;
	protected int m_iCaged;
	protected bool m_bTookOne;
	protected string m_sCrew;
	protected ResourceName m_sYoung;

	protected static const float CAMP_REACH = 6000;

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		m_Capture.GetSpots(result);
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 20, 40);
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!InOffer(gm) || !RZ_LiveCapture.HasRoom(gm) || gm.GetDropPointPrefab(RZ_EFaction.BANDITS).IsEmpty() || People(gm).HasPitStock())
			return 0;

		return 0.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Tribes tribes = Tribes(gm);
		RZ_Territory territory = gm.GetTerritory();
		if (!tribes || !territory || !territory.IsActive())
			return false;

		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			if (!Free(gm, t) || t.Count(RZ_ETribePart.YOUNG) < 2)
				continue;

			RZ_Holding camp = territory.FindHolding(RZ_EFaction.BANDITS, t.m_vLair, CAMP_REACH);
			ResourceName young = People(gm).YoungPrefab(t);
			if (!camp || young.IsEmpty() || !TakeTribe(gm, t))
				continue;

			m_Drop = RZ_Util.SpawnPrefab(gm.GetDropPointPrefab(RZ_EFaction.BANDITS), RZ_Util.GroundPos(RZ_Util.RandomPointInRing(camp.m_vPos, 30, 60)), Math.RandomFloat(0, 360));
			if (!m_Drop)
				return false;

			m_sYoung = young;
			m_sCrew = People(gm).CrewName(t);
			if (!BeginCatch(gm, t))
				return false;

			m_vPos = t.m_vLair;
			m_sTitle = "Stock the Pit";
			m_sDeed = "stocked the pit";
			m_sBrief = string.Format("The pit wants fresh hires. Two of %1's new ones, alive: put one down without killing it, tie it, sit on it till it's under, and get the case to our drop by %2 before it wakes up. Then the other one.", People(gm).NameAt(t, RZ_EFaction.BANDITS), camp.GetTitle());
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A capture at the lair's edge, of one of its young.
	protected bool BeginCatch(RZ_GameModeComponent gm, RZ_Tribe t)
	{
		if (!m_Capture.Begin(gm, m_iId, -1, LandPointNear(t.m_vLair, 60, 120), LandPointNear(t.m_vLair, 30, 80), 0))
			return false;

		m_Capture.UsePrefab(m_sYoung);
		m_bTookOne = false;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_Tribe t = GetTribe(gm);
		if (!t && m_iCaged == 0 && m_Capture.m_eState == RZ_ECaptureState.BAIT)
		{
			Withdraw("That lot broke up. Nothing left to catch.");
			return;
		}

		m_Capture.Update(gm, dt, ids, players);
		if (m_Capture.m_eState == RZ_ECaptureState.CAUGHT)
		{
			RZ_ContractHelp.AddNear(ids, players, m_Capture.m_vHeld, 30, m_aParticipants);
			if (!m_bTookOne && t)
			{
				m_bTookOne = true;
				Tribes(gm).GetLife().TakeMembers(t, RZ_ETribePart.YOUNG, 1);
			}
		}

		if (m_Capture.IsLost())
			m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnZombiesKilled(RZ_GameModeComponent gm, int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		m_Capture.OnSpecialKilled(gm, missionId, special);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnItemDelivered(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction, IEntity drop)
	{
		if (faction != RZ_EFaction.BANDITS || drop != m_Drop || !m_Capture.Deliver())
			return false;

		AddParticipant(playerId);
		m_iCaged++;
		if (m_iCaged >= 2)
		{
			m_eState = RZ_EMissionState.SUCCEEDED;
			return true;
		}

		// The other one
		m_Capture.Cleanup();
		m_Capture = new RZ_LiveCapture();
		RZ_Tribe t = GetTribe(gm);
		if (!t || t.Count(RZ_ETribePart.YOUNG) < 1 || !BeginCatch(gm, t))
		{
			m_eState = RZ_EMissionState.SUCCEEDED;
			m_fRewardScale = 0.5;
			return true;
		}

		gm.NotifyPlayer(playerId, m_sTitle, "\"One in the cage. Now the other.\"");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (People(gm))
			People(gm).StockThePit(m_sCrew, m_sYoung);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return string.Format("The pit has fresh hires, from %1.", m_sCrew);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		if (m_Capture.m_eState == RZ_ECaptureState.LOST)
			return "Stock the Pit: it died, or it woke up. The pit wanted it breathing.";

		return "Stock the Pit: the cages stayed empty.";
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		m_Capture.Cleanup();
		if (m_Drop)
			RZ_Util.DeleteUnit(m_Drop);
	}
}
