//! The great one (docs/INFECTED.md, "The great one"), phase 5 of the tribes design. A clan of 3 or
//! more packs that holds together 8 season days, from Overrun, makes its oldest leader in its prime or
//! older the great one: one on the map at a time, and none for 8 season days after one dies.
//! - Its guard is 4. Its dusk call is the server's one dusk line that evening, and a sound within
//!   1.5 km (Zombies > Tribes > Great call sound prefab).
//! - The Gathering, every 4 season days and on frenzy nights: each pack of its clan sends one hunter
//!   group along its way, by its lair, to the nearest lived-in place's wire, and waits there until
//!   dawn. The bodies stay inside the per-cluster shares, so it arrives in waves (RZ_Event_Gathering).
//! - It doesn't die of age while its clan holds 3 packs (it can still be challenged).
//! - When it dies: every pack of the clan holds the vigil, so nothing in it hunts that night; the clan
//!   breaks up, with no clan for 8 season days; the next dusk is the Night of Many Calls, when every
//!   border reopens for 2 counted nights (RZ_Event_NightOfManyCalls).
//! Caught alive it would count +3 on a finale route's track: that goes in with the finale's live
//! tissue. Needs Zombies > Tribes > Life cycle and Great one. Owned by RZ_Tribes. Server only.

//------------------------------------------------------------------------------------------------
//! The great one's clock in state.json (the flag itself is on the tribe's save).
class RZ_TribeGreatSave
{
	int clan;
	int clanDays;
	float lockUntil;
	float clansFreeAt;
	int lastGathering = -100;
	int gatherNight = -1;
	bool manyCallsDue;
	int gatherings;
	int fallen;
}

//------------------------------------------------------------------------------------------------
class RZ_TribeGreat
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_Tribes m_Tribes;			// weak: the owner
	protected int m_iClan;					// the clan holding together toward a great one
	protected int m_iClanDays;				// season days it has held 3 or more packs
	protected float m_fLockUntil;			// season days: no great one before this
	protected float m_fClansFreeAt;			// season days: no clan before this
	protected int m_iLastGathering = -100;	// season day
	protected int m_iGatherNight = -1;		// counted night of the last Gathering (one a night)
	protected bool m_bGatherDue;
	protected bool m_bManyCallsDue;
	protected bool m_bDuskDone;
	protected int m_iGatherings;
	protected int m_iFallen;

	static const int CLAN_SIZE = 3;
	static const int HOLD_DAYS = 8;
	static const float LOCK_DAYS = 8;
	static const int GATHER_EVERY = 4;
	static const int GREAT_GUARD = 4;
	static const float GREAT_SOUND = 1500;

	//------------------------------------------------------------------------------------------------
	void RZ_TribeGreat(RZ_GameModeComponent gm, RZ_Tribes tribes)
	{
		m_GM = gm;
		m_Tribes = tribes;
	}

	//------------------------------------------------------------------------------------------------
	bool IsOn()
	{
		RZ_TribeSettings s = m_Tribes.GetSettings();
		return m_Tribes.IsEnabled() && s && s.m_bLifeCycle && s.m_bGreatOne;
	}

	//------------------------------------------------------------------------------------------------
	//! The great one's pack, or null.
	RZ_Tribe GetGreat()
	{
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (t.m_bGreat)
				return t;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Packs in a clan (0: none).
	int ClanSize(int clan)
	{
		if (clan <= 0)
			return 0;

		int n = 0;
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (t.m_iClan == clan)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! For 8 season days after the great one dies there are no clans (RZ_TribeLife.Clans).
	bool ClansLocked()
	{
		return IsOn() && RZ_SeasonClock.Days() < m_fClansFreeAt;
	}

	//------------------------------------------------------------------------------------------------
	//! The great one doesn't die of age while its clan holds 3 packs (RZ_TribeLife.Ageing).
	bool Ageless(RZ_Tribe t)
	{
		return t && t.m_bGreat && ClanSize(t.m_iClan) >= CLAN_SIZE;
	}

	// ================================================================ Rising
	//------------------------------------------------------------------------------------------------
	//! Each season day (RZ_TribeLife.SeasonDay, after the clans): the great one keeps its guard of 4
	//! and calls the Gathering every 4 days; with none, a clan of 3 or more that holds 8 days, from
	//! Overrun, raises one.
	void SeasonDay()
	{
		if (!IsOn())
			return;

		RZ_Tribe great = GetGreat();
		if (great)
		{
			// Its clan is gone (a split-up, a break-up): it's an ordinary leader again
			if (ClanSize(great.m_iClan) < 2)
			{
				great.m_bGreat = false;
				RZ_Log.Info(string.Format("Tribes: %1's great one has no clan left to answer it.", great.m_sPlace));
				return;
			}

			KeepGuard(great);
			if (RZ_SeasonClock.Day() - m_iLastGathering >= GATHER_EVERY)
				m_bGatherDue = true;

			return;
		}

		RZ_Director dir = m_GM.GetDirector();
		if (!dir || dir.GetStage() < RZ_EOutbreakStage.OVERRUN || RZ_SeasonClock.Days() < m_fLockUntil)
		{
			m_iClan = 0;
			m_iClanDays = 0;
			return;
		}

		int best = 0;
		int bestSize = 0;
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			int size = ClanSize(t.m_iClan);
			if (size >= CLAN_SIZE && size > bestSize)
			{
				best = t.m_iClan;
				bestSize = size;
			}
		}

		if (best == 0)
		{
			m_iClan = 0;
			m_iClanDays = 0;
			return;
		}

		if (best != m_iClan)
		{
			m_iClan = best;
			m_iClanDays = 0;
		}

		m_iClanDays++;
		if (m_iClanDays >= HOLD_DAYS)
			Rise(best);
	}

	//------------------------------------------------------------------------------------------------
	//! The clan's oldest leader in its prime or older becomes the great one.
	protected void Rise(int clan)
	{
		RZ_Tribe oldest;
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (t.m_iClan != clan || !t.m_bHasAlpha || t.Stage() < 1)
				continue;

			if (!oldest || t.m_fLedSince < oldest.m_fLedSince)
				oldest = t;
		}

		if (!oldest)
			return;

		oldest.m_bGreat = true;
		m_iClan = 0;
		m_iClanDays = 0;
		m_iLastGathering = RZ_SeasonClock.Day();
		oldest.AddHistory("became the one every call in its clan answers to");
		KeepGuard(oldest);
		m_Tribes.GetWays().Headline(oldest, string.Format("Every call along the ridges answers one voice now: %1's, at %2.", oldest.m_sAlphaName, oldest.m_sPlace));
		m_Tribes.GetPeople().AddBeat(string.Format("The great one rose at %1.", oldest.m_sPlace));
		RZ_Log.Info(string.Format("Tribes: %1's leader %2 is the great one of a clan of %3.", oldest.m_sPlace, oldest.m_sAlphaName, ClanSize(clan)));
	}

	//------------------------------------------------------------------------------------------------
	//! Its guard is 4: hunters lying up at the lair step up (the Guard prefabs are the helmeted ones).
	protected void KeepGuard(RZ_Tribe great)
	{
		int guards = great.Count(RZ_ETribePart.GUARD);
		if (guards >= GREAT_GUARD)
			return;

		RZ_TribeLife life = m_Tribes.GetLife();
		int moved = life.TakeMembers(great, RZ_ETribePart.HUNTERS, GREAT_GUARD - guards);
		if (moved > 0)
			life.Join(great, moved, 0, 0);
	}

	// ================================================================ Dusk
	//------------------------------------------------------------------------------------------------
	//! At the dusk call (18:45), before the other packs call: the Night of Many Calls when it's due,
	//! and the great one's call, the server's one dusk line this evening (everyone online hears it
	//! instead of their nearest pack's) and a sound within 1.5 km. Then the Gathering when it's due.
	void Dusk(array<int> ids, array<IEntity> players)
	{
		if (m_bDuskDone || !IsOn())
			return;

		m_bDuskDone = true;
		if (m_bManyCallsDue)
		{
			m_bManyCallsDue = false;
			m_Tribes.GetBorders().ReopenAll();
			RZ_Event_NightOfManyCalls.Start(m_GM);
			RZ_Log.Info("Tribes: the Night of Many Calls: every border reopens for 2 counted nights.");
		}

		RZ_Tribe great = GetGreat();
		if (!great || great.m_bVigil || great.m_bFed)
			return;

		string line = string.Format("One call from %1 at dusk, deeper than the rest, and every ridge from there to %2 answers it.", great.m_sPlace, FarthestPlace(great));
		foreach (int i, IEntity p : players)
		{
			if (m_Tribes.MarkCalled(ids[i]))
				m_GM.NotifyPlayer(ids[i], "At dusk", line);
		}

		m_Tribes.GetProps().CallSound(great.m_vLair, true);
		if (m_bGatherDue)
			StartGathering(great);
	}

	//------------------------------------------------------------------------------------------------
	//! Out of dusk: the next dusk may call.
	void NotDusk()
	{
		m_bDuskDone = false;
	}

	//------------------------------------------------------------------------------------------------
	//! The place of the clan's pack farthest from the great one (its own, if it's alone).
	protected string FarthestPlace(RZ_Tribe great)
	{
		string place = great.m_sPlace;
		float far = 0;
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (t == great || t.m_iClan != great.m_iClan)
				continue;

			float d = RZ_Util.DistXZ(t.m_vLair, great.m_vLair);
			if (d > far)
			{
				far = d;
				place = t.m_sPlace;
			}
		}

		return place;
	}

	//------------------------------------------------------------------------------------------------
	//! A frenzy night (RZ_TribePeople.Frenzy): the great one's clan gathers, if the great one is in it.
	void OnFrenzy()
	{
		if (!IsOn())
			return;

		RZ_Tribe great = GetGreat();
		if (great && great.m_fFrenzy > 0 && !great.m_bVigil)
			StartGathering(great);
	}

	// ================================================================ The Gathering
	//------------------------------------------------------------------------------------------------
	//! Each pack of the clan sends one hunter group along the great one's way to the nearest lived-in
	//! place's wire; at most one a night.
	protected void StartGathering(RZ_Tribe great)
	{
		int night = m_Tribes.CountedNights();
		if (m_iGatherNight == night)
			return;

		vector wire;
		if (!m_Tribes.WireEnd(great.m_vLair, wire))
			return;

		m_iGatherNight = night;
		m_iLastGathering = RZ_SeasonClock.Day();
		m_bGatherDue = false;
		int sent = 0;
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (t.m_iClan != great.m_iClan || t.m_bVigil || t.IsWeakened())
				continue;

			if (m_Tribes.GatherTo(t, great, wire))
				sent++;
		}

		if (sent == 0)
			return;

		m_iGatherings++;
		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(wire));
		string town = "the nearest wire";
		if (s)
			town = s.m_sName;

		RZ_Event_Gathering.Start(m_GM, wire, great.m_sPlace, town);
		RZ_Log.Info(string.Format("Tribes: the Gathering: %1 packs send a group each by %2 to the wire at %3.", sent, great.m_sPlace, RZ_SettlementScanner.GridReference(wire)));
	}

	// ================================================================ The fall
	//------------------------------------------------------------------------------------------------
	//! The great one fell (RZ_Tribes.StartVigil): every pack of its clan holds the vigil tonight, the
	//! clan breaks up, no clan and no great one for 8 season days, and the next dusk is the Night of
	//! Many Calls.
	void OnGreatDown(RZ_Tribe great)
	{
		great.m_bGreat = false;
		int clan = great.m_iClan;
		float now = RZ_SeasonClock.Days();
		m_fLockUntil = now + LOCK_DAYS;
		m_fClansFreeAt = now + LOCK_DAYS;
		m_bManyCallsDue = true;
		m_iClan = 0;
		m_iClanDays = 0;
		m_iFallen++;
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (clan <= 0 || t.m_iClan != clan)
				continue;

			t.m_iClan = 0;
			t.m_iClanHold = 0;
			if (t == great || t.m_bVigil)
				continue;

			// Its own vigil at its own lair: nothing in the clan hunts tonight
			t.m_bVigil = true;
			t.m_vVigilAt = t.m_vLair;
			t.m_fVigilBodiesUntil = RZ_BaconBody.Now() + 1800;
			m_Tribes.GetBodies().EndHunt(t, false);
		}

		m_Tribes.Tell(great.m_vLair, 5000, "Quiet", string.Format("Every call along the ridges stops at once, from %1 outward. Nothing answers anything.", great.m_sPlace));
		m_Tribes.GetWays().Headline(great, string.Format("The deep voice at %1 has stopped. Every ridge that answered it is quiet tonight.", great.m_sPlace));
		m_Tribes.GetPeople().AddBeat(string.Format("The great one at %1 fell, and its clan broke up.", great.m_sPlace));
		RZ_Log.Info(string.Format("Tribes: the great one of %1 fell; its clan breaks up, no clan or great one for %2 season days.", great.m_sPlace, LOCK_DAYS));
	}

	// ================================================================ Status and save
	//------------------------------------------------------------------------------------------------
	string StatusText()
	{
		if (!IsOn())
			return "off";

		string who = "none";
		RZ_Tribe great = GetGreat();
		if (great)
			who = string.Format("%1 at %2 (clan of %3)", great.m_sAlphaName, great.m_sPlace, ClanSize(great.m_iClan));

		return string.Format("%1, clan holding %2/%3 days, gatherings %4, fallen %5, clans locked %6, many calls due %7", who, m_iClanDays, HOLD_DAYS, m_iGatherings, m_iFallen, ClansLocked(), m_bManyCallsDue);
	}

	//------------------------------------------------------------------------------------------------
	RZ_TribeGreatSave Save()
	{
		RZ_TribeGreatSave d = new RZ_TribeGreatSave();
		d.clan = m_iClan;
		d.clanDays = m_iClanDays;
		d.lockUntil = m_fLockUntil;
		d.clansFreeAt = m_fClansFreeAt;
		d.lastGathering = m_iLastGathering;
		d.gatherNight = m_iGatherNight;
		d.manyCallsDue = m_bManyCallsDue;
		d.gatherings = m_iGatherings;
		d.fallen = m_iFallen;
		return d;
	}

	//------------------------------------------------------------------------------------------------
	void Restore(RZ_TribeGreatSave d)
	{
		if (!d)
			return;

		m_iClan = d.clan;
		m_iClanDays = d.clanDays;
		m_fLockUntil = d.lockUntil;
		m_fClansFreeAt = d.clansFreeAt;
		m_iLastGathering = d.lastGathering;
		m_iGatherNight = d.gatherNight;
		m_bManyCallsDue = d.manyCallsDue;
		m_iGatherings = d.gatherings;
		m_iFallen = d.fallen;
	}
}
