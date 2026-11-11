//! The parley, the knock and settling up (docs/REPUTATION.md, "The parley and the knock"; reputation
//! phase 3). Server only, owned by the checkpoints.
//!
//! The owed: Hunted or Kill on Sight with a faction, Earned Fame 15+ with it this life, and a best
//! deed with it recorded this life. At a Bastion checkpoint or camp gate, a Red Hand roadblock or
//! camp gate, or Halcyon's lab or field station gate, an owed player gets one hearing:
//!  1. the shout at 150 m ("Stop there. Empty hands, or not at all.");
//!  2. walking in empty-handed (no firearm anywhere on you, no grenade or charge: RZ_Util.IsEmptyHanded,
//!     every tick): they hold fire only while you're closing, 90 s at most, 5 s at a time;
//!  3. at the barrier, 10 s searched; then the seat's leader speaks over the radio from inside: one
//!     deed callback and one amends offer (a contract reserved for you for 60 minutes, though you're
//!     Hunted, finishing by its own rules where it says);
//!  4. walking away: the hold lasts while you're leaving, to 200 m or 90 s.
//! Closed (no shout at all) while a hunter squad or fight of that faction has you within 1 km,
//! another player is within 150 m of the gate, or this identity parleyed with that faction in the
//! last 2 real hours (server time, so a reconnect or restart resets nothing). Arming yourself or
//! firing breaks it; another player arriving within 150 m pauses it ("Back off. We're talking.").
//! *Simplified:* the engine's hold is faction-wide; it's made local by its conditions.
//!
//! The Haven's knock: an owed exile, empty-handed within 10 m of a safehouse's named person, is
//! answered through the door by the council head (callback and offer). The same limits.
//! Settling up: Make amends at a quartermaster gains a second price, 10 Fame for 5 Infamy, at most
//! 10 Infamy a real day per faction, from the one forgiveness budget.
class RZ_ParleyRun
{
	int m_iPlayer;
	int m_iFaction;
	vector m_vGate;
	string m_sTitle;
	string m_sPlace;
	int m_iPhase;			// 0 walking in, 1 searched at the barrier, 2 walking away
	float m_fPhaseTime;
	float m_fLastDist = -1;
	bool m_bWarnedOthers;
	int m_iStarted;			// server time it opened (a run left behind goes after 5 minutes)
}

//------------------------------------------------------------------------------------------------
class RZ_Parley
{
	static const float SHOUT_RADIUS = 150;
	static const float HOLD_RADIUS = 200;
	static const float BARRIER = 35;
	static const float WALK_SECONDS = 90;
	static const float SEARCH_SECONDS = 10;
	static const float HOLD_STEP = 7;			// the stand-down, refreshed each tick
	static const float SQUAD_RADIUS = 1000;
	static const float ALONE_RADIUS = 150;
	static const int LIMIT_SECONDS = 7200;		// once per 2 real hours per faction, by identity
	static const int RESERVE_SECONDS = 3600;
	static const int OWED_FAME = 15;
	static const int SETTLE_FAME = 10;			// settling up: 10 Fame...
	static const int SETTLE_INFAMY = 5;			// ...for 5 Infamy
	static const int SETTLE_DAY_CAP = 10;		// at most this much Infamy settled a real day per faction

	protected RZ_GameModeComponent m_GM;
	protected ref map<int, ref RZ_ParleyRun> m_mRuns = new map<int, ref RZ_ParleyRun>();
	protected ref array<string> m_aCharges = {};

	//------------------------------------------------------------------------------------------------
	void RZ_Parley(RZ_GameModeComponent gm)
	{
		m_GM = gm;
		RZ_ItemHelper.Words("grenade,m67,rgd5,rgd_5,charge,c4,explosive,demo_charge,satchel,mine", m_aCharges);
	}

	//------------------------------------------------------------------------------------------------
	//! Hunted or Kill on Sight, Earned Fame 15+ with them this life, and a best deed with them.
	bool IsOwed(int playerId, int f)
	{
		if (!RZ_ReputationSettings.Get().m_bParley || !RZ_Factions.IsValid(f) || !RZ_Reputation.IsHunted(m_GM.GetTier(playerId, f)))
			return false;

		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		return profile.EarnedFame(f) >= OWED_FAME && !profile.m_aBestDeed[f].IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	bool IsRunning(int playerId)
	{
		return m_mRuns.Contains(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Why there's no parley for this player at this gate now ("" if there may be one).
	protected string Closed(int playerId, int f, vector gate, IEntity me)
	{
		RZ_Hostility hostility = m_GM.GetHostility();
		if (hostility && hostility.IsFightFor(playerId, f, me.GetOrigin(), SQUAD_RADIUS))
			return "a squad or a fight";

		if (OtherPlayerNear(playerId, gate, ALONE_RADIUS))
			return "not alone";

		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		if (RZ_SeasonClock.Now() - profile.m_aParleyUnix[f] < LIMIT_SECONDS)
			return "too soon";

		// The cure finale: closed with a faction whose route is fighting yours
		RZ_CureFinale finale = RZ_CureFinale.Get();
		if (finale && finale.ParleyClosed(playerId, f))
			return "fighting routes";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected bool OtherPlayerNear(int playerId, vector at, float radius)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (ids[i] != playerId && RZ_Util.DistXZ(p.GetOrigin(), at) <= radius)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! From the checkpoints, for a player near a gate of `f` (a road post's barrier or a camp's gate).
	//! True while a parley holds this player here (the post's own handling is skipped).
	bool Contact(int playerId, IEntity me, int f, vector gate, float dist, float dt, string title, string place)
	{
		RZ_ParleyRun run;
		if (!m_mRuns.Find(playerId, run))
		{
			if (dist > SHOUT_RADIUS || !IsOwed(playerId, f) || !Closed(playerId, f, gate, me).IsEmpty())
				return false;

			run = new RZ_ParleyRun();
			run.m_iPlayer = playerId;
			run.m_iFaction = f;
			run.m_vGate = gate;
			run.m_sTitle = title;
			run.m_sPlace = place;
			run.m_iStarted = RZ_SeasonClock.Now();
			m_mRuns.Set(playerId, run);
			m_GM.Say(playerId, title, f, "\"Stop there. Empty hands, or not at all.\"");
			RZ_Log.Info(string.Format("Parley: %1 at %2 (%3) opened", RZ_Util.PlayerName(playerId), place, RZ_Factions.GetName(f)));
			return true;
		}

		if (run.m_vGate != gate)
			return false;

		Step(run, me, dist, dt);
		return m_mRuns.Contains(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void Step(RZ_ParleyRun run, IEntity me, float dist, float dt)
	{
		int playerId = run.m_iPlayer;
		int f = run.m_iFaction;
		RZ_Hostility hostility = m_GM.GetHostility();
		if (!RZ_Util.IsEmptyHanded(me, m_aCharges))
		{
			Break(run, "They see the weapon. That's the end of talking.", true);
			return;
		}

		if (hostility && hostility.IsFightFor(playerId, f, me.GetOrigin(), SQUAD_RADIUS))
		{
			Break(run, "\"They've got friends coming. So have we.\"", false);
			return;
		}

		// Another player arriving pauses it: no hold, the clock stops, a word for the newcomer
		if (OtherPlayerNear(playerId, run.m_vGate, ALONE_RADIUS))
		{
			if (!run.m_bWarnedOthers)
			{
				run.m_bWarnedOthers = true;
				TellOthers(run, "\"Back off. We're talking.\"");
			}

			run.m_fLastDist = dist;
			return;
		}

		run.m_fPhaseTime += dt;
		bool closing = run.m_fLastDist < 0 || dist < run.m_fLastDist - 0.3;
		bool leaving = run.m_fLastDist >= 0 && dist > run.m_fLastDist + 0.3;
		run.m_fLastDist = dist;

		if (run.m_iPhase == 0)
		{
			if (dist > HOLD_RADIUS || run.m_fPhaseTime > WALK_SECONDS)
			{
				End(run, "");
				return;
			}

			if (dist <= BARRIER)
			{
				run.m_iPhase = 1;
				run.m_fPhaseTime = 0;
				m_GM.Say(playerId, run.m_sTitle, f, "\"Hands on the barrier. Nobody's coming out to you, so don't wait for it.\"");
			}

			if (closing || dist <= BARRIER)
				Hold(run);

			return;
		}

		if (run.m_iPhase == 1)
		{
			if (dist > BARRIER + 10)
			{
				End(run, "You walk off before they're done. That was your hearing.");
				return;
			}

			Hold(run);
			if (run.m_fPhaseTime < SEARCH_SECONDS)
				return;

			Hearing(run);
			run.m_iPhase = 2;
			run.m_fPhaseTime = 0;
			return;
		}

		// Walking away: held while leaving, to 200 m or 90 s; coming back in breaks it
		if (dist > HOLD_RADIUS || run.m_fPhaseTime > WALK_SECONDS)
		{
			End(run, "");
			return;
		}

		if (dist < 10)
		{
			Break(run, "\"Past the barrier? No.\"", true);
			return;
		}

		if (leaving)
			Hold(run);
	}

	//------------------------------------------------------------------------------------------------
	protected void Hold(RZ_ParleyRun run)
	{
		RZ_Hostility hostility = m_GM.GetHostility();
		if (hostility)
			hostility.StandDown(run.m_iPlayer, run.m_iFaction, HOLD_STEP, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void TellOthers(RZ_ParleyRun run, string line)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (ids[i] != run.m_iPlayer && RZ_Util.DistXZ(p.GetOrigin(), run.m_vGate) <= ALONE_RADIUS)
				m_GM.Say(ids[i], run.m_sTitle, run.m_iFaction, line);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The leader's voice from inside: one deed callback and one amends offer.
	protected void Hearing(RZ_ParleyRun run)
	{
		int playerId = run.m_iPlayer;
		int f = run.m_iFaction;
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		profile.m_aParleyUnix[f] = RZ_SeasonClock.Now();
		string speaker = run.m_sTitle;
		RZ_Leaders leaders = m_GM.GetLeaders();
		if (leaders)
		{
			RZ_Leader l = leaders.FindNearest(f, run.m_vGate, 3000);
			if (l)
				speaker = l.m_sTitle + " " + l.m_sName + ", over the radio";
		}

		string text = string.Format("\"Halt. Hands. You %1.", profile.m_aBestDeed[f]);
		if (!profile.m_aWorstDeed[f].IsEmpty())
			text += string.Format(" You also %1. Both go in the log.", profile.m_aWorstDeed[f]);

		text += " Talk.\"";
		string offer = ReserveAmends(playerId, f);
		if (offer.IsEmpty())
			text += " \"Nothing for you today. Come back later.\"";
		else
			text += "\n" + offer;

		// Nothing to place: the 2 hours aren't used
		if (offer.IsEmpty())
			profile.m_aParleyUnix[f] = 0;

		m_GM.NotifyPlayer(playerId, speaker, text);
		RZ_Log.Info(string.Format("Parley: %1 heard by %2 (offer: %3)", RZ_Util.PlayerName(playerId), RZ_Factions.GetName(f), !offer.IsEmpty()));
	}

	//------------------------------------------------------------------------------------------------
	//! One of the faction's amends contracts, reserved for this player for 60 minutes though they're
	//! Hunted: the Haven's Medicine Run (else Survivors Trapped), Bastion's Burn the Bodies (else
	//! Recon), the Red Hand's Fence the Goods, Halcyon's Field Samples. The brief, "" if none placed.
	string ReserveAmends(int playerId, int f)
	{
		array<typename> types = {};
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: types.Insert(RZ_Mission_MedicineRun); types.Insert(RZ_Mission_SurvivorHoldout); break;
			case RZ_EFaction.MILITARY: types.Insert(RZ_Mission_BurnTheBodies); types.Insert(RZ_Mission_Recon); break;
			case RZ_EFaction.BANDITS: types.Insert(RZ_Mission_FenceTheGoods); break;
			case RZ_EFaction.INSTITUTE: types.Insert(RZ_Mission_FieldSamples); break;
		}

		RZ_MissionManager missions = m_GM.GetMissions();
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (typename t : types)
		{
			RZ_Mission m = RZ_Mission.Cast(t.Spawn());
			if (!m)
				continue;

			m.Configure(m_GM);
			if (m.m_eGiver != f)
				continue;

			m.Reserve(m_GM, playerId, RESERVE_SECONDS);
			if (!missions.StartIfCan(m, ids.Count()))
				continue;

			RZ_Log.Info(string.Format("Parley: %1 reserved for %2", m.m_sTitle, RZ_Util.PlayerName(playerId)));
			return string.Format("\"There's work. %1. It's yours for the hour, and it pays in what we'll forget.\" (Amends: works off Infamy, finishes where the job says.)\n%2", m.m_sTitle, m.m_sBrief);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! From the gunfire hook: a parleyer who fires breaks it.
	void OnPlayerShot(int playerId)
	{
		RZ_ParleyRun run;
		if (m_mRuns.Find(playerId, run))
			Break(run, "A shot. Talking's over.", true);
	}

	//------------------------------------------------------------------------------------------------
	protected void Break(RZ_ParleyRun run, string line, bool grudge)
	{
		m_GM.Say(run.m_iPlayer, run.m_sTitle, run.m_iFaction, line);
		RZ_Hostility hostility = m_GM.GetHostility();
		if (grudge && hostility)
			hostility.AddGrudge(run.m_iPlayer, run.m_iFaction, run.m_vGate);

		m_GM.GetProfile(run.m_iPlayer).m_aParleyUnix[run.m_iFaction] = RZ_SeasonClock.Now();
		RZ_Log.Info(string.Format("Parley: %1 at %2 broken", RZ_Util.PlayerName(run.m_iPlayer), run.m_sPlace));
		m_mRuns.Remove(run.m_iPlayer);
	}

	//------------------------------------------------------------------------------------------------
	protected void End(RZ_ParleyRun run, string line)
	{
		if (!line.IsEmpty())
			m_GM.Say(run.m_iPlayer, run.m_sTitle, run.m_iFaction, line);

		m_mRuns.Remove(run.m_iPlayer);
	}

	//------------------------------------------------------------------------------------------------
	//! A player gone from the server: their parley ends.
	void Forget(array<int> ids)
	{
		array<int> gone = {};
		int now = RZ_SeasonClock.Now();
		foreach (int playerId, RZ_ParleyRun run : m_mRuns)
		{
			if (!ids.Contains(playerId) || now - run.m_iStarted > 300)
				gone.Insert(playerId);
		}

		foreach (int g : gone)
		{
			m_mRuns.Remove(g);
		}
	}

	// ================================================================ The knock
	//------------------------------------------------------------------------------------------------
	//! An owed exile at a safehouse's named person (RZ_Errands.HandleHouseTalk): empty-handed, alone,
	//! within the 2 hours. The council head answers through the door, which stays shut. True if it
	//! was a knock (the line has been said).
	bool Knock(int playerId, IEntity me, vector door, string houseName)
	{
		int f = RZ_EFaction.SURVIVORS;
		if (!IsOwed(playerId, f) || !me || RZ_Util.DistXZ(me.GetOrigin(), door) > 15)
			return false;

		string title = "The door at " + houseName;
		if (!RZ_Util.IsEmptyHanded(me, m_aCharges))
		{
			m_GM.Say(playerId, title, f, "Nobody answers a knock from someone carrying a gun.");
			return true;
		}

		string closed = Closed(playerId, f, door, me);
		if (closed == "not alone")
		{
			m_GM.Say(playerId, title, f, "\"Just you. Send the others down the road and knock again.\"");
			return true;
		}

		if (!closed.IsEmpty())
		{
			m_GM.Say(playerId, title, f, "The door stays shut. Footsteps inside, and then none.");
			return true;
		}

		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		profile.m_aParleyUnix[f] = RZ_SeasonClock.Now();
		string text = string.Format("You knock under the bear on the door. The council head answers through it, and it stays shut. \"You %1. I remember. You also know why this door's shut.\"", profile.m_aBestDeed[f]);
		string offer = ReserveAmends(playerId, f);
		if (offer.IsEmpty())
		{
			profile.m_aParleyUnix[f] = 0;
			text += " \"Nothing I can give you today. Come back.\"";
		}
		else
		{
			text += "\n" + offer;
		}

		m_GM.NotifyPlayer(playerId, title, text);
		RZ_Log.Info(string.Format("Parley: %1 knocked at %2", RZ_Util.PlayerName(playerId), houseName));
		return true;
	}

	// ================================================================ Settling up
	//------------------------------------------------------------------------------------------------
	//! Make amends' second price: 10 Fame for 5 Infamy, through the forgiveness budget, at most 10
	//! a real day per faction. The line said; nothing is taken if it can't be done.
	string Settle(int playerId, int f)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		if (profile.GetInfamy(f) <= 0)
			return "\"You don't owe us anything to settle.\"";

		if (profile.ForgiveLeft(f) <= 0)
			return m_GM.ForgiveSpentText(f);

		int day = RZ_SeasonClock.Now() / 86400;
		if (profile.m_aSettleDay[f] != day)
		{
			profile.m_aSettleDay[f] = day;
			profile.m_aSettled[f] = 0;
		}

		if (profile.m_aSettled[f] >= SETTLE_DAY_CAP)
			return "\"That's enough settling for one day. Come back tomorrow.\"";

		if (profile.GetFame(f) < SETTLE_FAME)
			return string.Format("\"Settling costs %1 Fame with us. You haven't got it.\"", SETTLE_FAME);

		int off = m_GM.Forgive(playerId, f, Math.Min(SETTLE_INFAMY, profile.GetInfamy(f)), "settled up");
		if (off <= 0)
			return m_GM.ForgiveSpentText(f);

		profile.AddFame(f, -SETTLE_FAME);
		profile.m_aSettled[f] = profile.m_aSettled[f] + off;
		RZ_Log.Info(string.Format("Reputation: %1 settled up with %2: -%3 Fame, -%4 Infamy", RZ_Util.PlayerName(playerId), RZ_Factions.GetName(f), SETTLE_FAME, off));
		return string.Format("\"Square-ish.\" -%1 Fame, -%2 Infamy with %3. Spent Fame is gone.", SETTLE_FAME, off, RZ_Factions.GetNameInText(f));
	}
}
