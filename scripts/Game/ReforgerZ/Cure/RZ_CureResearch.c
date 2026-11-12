//! Server-wide cure research. Players kill special infected, carry the samples
//! to the lab and deliver them. Progress changes the world for everyone:
//!
//! The pace (docs/DESIGN.md, "Cure research" and "Season pacing"): research counts x Cure pace x
//! the pace by play (PlayPace: the player-hours the server really plays, measured over the last
//! week, so a season takes about as long with 2 regulars as with 15). Delivered research (samples
//! and missions) waits in a backlog that the lab works through at Cure > Lab research per day, in
//! real time, online or not. The backlog is capped (Lab backlog days), a backstop for spikes: while
//! it's full the lab refuses samples and mission research beyond the cap is lost. The part of a
//! delivery that fills a near-empty backlog counts more (Empty lab bonus). The milestones fire on
//! worked research, and their news waits until someone is in the world to hear it.
//! While the cure finale runs (its five routes registered, docs/FINALE.md), research stops at 90
//! percent: the routes race to finish the batch (RZ_CureFinale), and one AddFinaleResearch call at the
//! seal takes it to 100. After a week at the gate the built fallback feeds the finale's lab-alone track.
//!
//!  25%  Field antiviral    infection spreads slower, bites take longer to kill
//!  50%  Bite treatment     (no world effect of its own; supply below)
//!  75%  Immunity serum     spread slows further, bites take even longer
//! 100%  Final stand        hold the lab against waves for 15 minutes (it starts once a player reaches
//!                          the lab, also after a restart, and waits while nobody is online).
//!                          With the finale it's also the fight for the clean room (RZ_CureFinale).
//!                          Win: the cure goes out and the season ends.
//!                          Abandon the lab: the batch is ruined and research drops back to 90%
//!                          (the lab itself never falls: it's Halcyon's stronghold).
//!
//! Bite treatment exists from the start; research only scales supply and price
//! (docs/DESIGN.md, "Treatments"). The lab makes antidote doses into a stock (1 per 30 min,
//! cap 2; 1/20 min, cap 3 after 25%; 1/15, cap 4 after 50%; 1/10, cap 5 after 75%) that
//! Halcyon's quartermaster sells; the Red Hand sells only what it has stolen from it. Stabilisers
//! are a second, smaller stock: rare and dear at first, plentiful after 75%.
class RZ_CureResearch
{
	protected RZ_GameModeComponent m_GM;

	float m_fProgress;
	int m_iMilestone;
	bool m_bCured;

	protected bool m_bFinalStand;
	protected bool m_bStandHeldTold;
	protected float m_fFinalStandLeft;
	protected float m_fAbandonedFor;
	protected bool m_bStandReached;		// a player has been at the lab since the stand (re)started
	protected float m_fWaveTimer;
	protected float m_fAntidoteTimer;
	protected float m_fStabiliserTimer;
	protected int m_iLabStabilisers;
	protected float m_fAntidoteBoostLeft;	// Power the Lab: doses come twice as fast for a while (not saved)
	protected IEntity m_Annex;				// Mobile Lab: a field annex taking samples for a while (not saved)
	protected float m_fAnnexLeft;

	//! Doses the lab has made and not yet sold, and doses the Red Hand has stolen from it
	protected int m_iLabAntidotes;
	protected int m_iStolenAntidotes;

	//! Final stands lost since the server started, and how many of them the Red Hand stole (not
	//! saved: a restart starts the stand again). A questline compares them before and after a stand.
	protected int m_iBatchesLost;
	protected int m_iBatchesStolen;

	//! The lab's capacity (saved): research delivered and not yet worked through, the real time
	//! (System.GetUnixTime) the lab last worked, whether research has reached 100 percent once (the
	//! finale's gate then stays open: a lost batch goes back to the lab's normal work), and the
	//! finale research each faction's route has added.
	protected float m_fBacklog;
	protected int m_iLabUnix;
	protected bool m_bGateOpen;
	protected ref array<float> m_aFinaleResearch = {};
	protected float m_fWorkTimer;
	protected float m_fSyncTimer;
	protected bool m_bWorkedSinceSync;

	//! Real seconds research has waited at the finale's gate (saved; the fallback starts after a week).
	protected float m_fGateSeconds;

	//! Breakthrough news not heard yet (saved): bit 1 << milestone for 1 to 3, NEWS_GATE for the gate.
	//! The lab also works with nobody online, so its news waits until a player is in the world.
	protected int m_iNews;

	//! Pace by play (saved): player-hours played on each real day, [0] = today (real day m_iPlayDay),
	//! [1] = yesterday and so on, and the first whole day counted (the season's first day is partial).
	protected ref array<float> m_aPlayHours = {};
	protected int m_iPlayDay;
	protected int m_iPlayFrom;

	static const float FINAL_STAND_SECONDS = 900;
	static const float ABANDON_LIMIT_SECONDS = 90;
	static const float WAVE_INTERVAL_SECONDS = 150;
	static const int LAB_ANTIDOTE_CAP = 5;		// the most the stock ever holds (after 75%)
	static const int STABILISER_CAP = 5;
	static const float MAX_RESEARCH_MULT = 2;	// background x Champion perk, per sample
	static const int STOLEN_ANTIDOTE_CAP = 4;
	static const int BLACK_MARKET_EXTRA = 2;	// season rules: the black market everywhere holds two more
	static const float LAB_DEFENCE_RADIUS = 150;
	static const float FINALE_GATE = 90;			// research stops here while a finale questline is registered
	protected static const int DAY_SECONDS = 86400;
	protected static const int MAX_CATCH_UP = 2592000;	// the lab catches up at most 30 days at once (a server down for long)
	protected static const float WORK_SECONDS = 60;		// the lab works once a minute: whole steps, not float dust
	protected static const float SYNC_SECONDS = 60;		// clients hear of the lab's work at most once a minute
	protected static const float EMPTY = 0.01;			// a backlog below this is empty
	protected static const float IDLE_DAYS = 0.5;		// the empty lab bonus fills the backlog up to half a day of work
	protected static const float GATE_GRACE_DAYS = 7;	// the finale fallback starts after a week at the gate (/ season speed)
	protected static const int PLAY_DAYS = 7;			// pace by play: the whole real days it averages over
	static const float PLAY_PACE_MIN = 0.25;			// and its limits
	static const float PLAY_PACE_MAX = 2.5;
	protected static const int NEWS_GATE = 16;			// m_iNews: the 90 percent gate's news

	//------------------------------------------------------------------------------------------------
	//! Most stolen doses the Red Hand's black market holds (4, or 6 with the season's open black market).
	static int StolenCap()
	{
		if (RZ_Seasons.Mods().m_bOpenBlackMarket)
			return STOLEN_ANTIDOTE_CAP + BLACK_MARKET_EXTRA;

		return STOLEN_ANTIDOTE_CAP;
	}

	//------------------------------------------------------------------------------------------------
	//! A fresh season's story rule (Two Tins a Copy): the black market starts with `count` stolen
	//! doses, within its cap.
	void SeedStolen(int count)
	{
		m_iStolenAntidotes = Math.Min(StolenCap(), m_iStolenAntidotes + Math.Max(0, count));
		RZ_Log.Info(string.Format("Story rules: the black market starts with %1 stolen doses.", m_iStolenAntidotes));
	}

	//------------------------------------------------------------------------------------------------
	void RZ_CureResearch(RZ_GameModeComponent gm)
	{
		m_GM = gm;
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			m_aFinaleResearch.Insert(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Restore(float progress, int milestone, bool cured, int labAntidotes, int stolenAntidotes, int labStabilisers)
	{
		m_fProgress = progress;
		m_iMilestone = milestone;
		m_bCured = cured;
		m_iLabAntidotes = Math.ClampInt(labAntidotes, 0, AntidoteCap());
		m_iStolenAntidotes = Math.ClampInt(stolenAntidotes, 0, STOLEN_ANTIDOTE_CAP + BLACK_MARKET_EXTRA);
		m_iLabStabilisers = Math.ClampInt(labStabilisers, 0, StabiliserCap());

		// A restart in the middle of the final stand starts it again from the beginning (with the
		// finale, unless the batch still waits for its seal)
		if (m_iMilestone >= 4 && !m_bCured && !FinaleHolds())
			StartFinalStand();
	}

	//------------------------------------------------------------------------------------------------
	//! The lab's backlog, its clock, the finale's gate, the news not heard yet and the pace by play
	//! (older saves: all empty, and the pace by play counts from now). After Restore.
	void RestoreLab(notnull RZ_SaveData data)
	{
		m_fBacklog = Math.Max(0, data.cureBacklog);
		m_iLabUnix = data.labUnix;
		m_bGateOpen = data.finaleGateOpen || m_iMilestone >= 4;
		m_fGateSeconds = Math.Max(0, data.gateSeconds);
		m_iNews = data.cureNews;
		if (data.finaleResearch)
		{
			for (int f = 0; f < data.finaleResearch.Count() && f < m_aFinaleResearch.Count(); f++)
			{
				m_aFinaleResearch[f] = Math.Max(0, data.finaleResearch[f]);
			}
		}

		if (data.playHours && data.playDay > 0)
		{
			m_aPlayHours.Clear();
			for (int d = 0; d < data.playHours.Count() && d <= PLAY_DAYS; d++)
			{
				m_aPlayHours.Insert(Math.Max(0, data.playHours[d]));
			}

			m_iPlayDay = data.playDay;
			m_iPlayFrom = data.playFrom;
		}

		// What the lab did while the server was down is worked out at the end of the server's start
		// (CatchUp), once every system is set up for the milestones it may cross
	}

	//------------------------------------------------------------------------------------------------
	void SaveLab(notnull RZ_SaveData data)
	{
		data.cureBacklog = m_fBacklog;
		data.labUnix = m_iLabUnix;
		data.finaleGateOpen = m_bGateOpen;
		data.gateSeconds = m_fGateSeconds;
		data.cureNews = m_iNews;
		data.finaleResearch.Clear();
		foreach (float v : m_aFinaleResearch)
		{
			data.finaleResearch.Insert(v);
		}

		data.playHours.Clear();
		foreach (float h : m_aPlayHours)
		{
			data.playHours.Insert(h);
		}

		data.playDay = m_iPlayDay;
		data.playFrom = m_iPlayFrom;
	}

	//------------------------------------------------------------------------------------------------
	//! What the lab did while the server was down, worked out now: at the end of the server's start,
	//! after every system the milestones touch is set up, so the first replicated state is current.
	//! The pace by play moves on to today too (days the server was down count as unplayed).
	void CatchUp()
	{
		TrackPlay(0, 0);
		Work();
	}

	//------------------------------------------------------------------------------------------------
	float GetBacklog() { return m_fBacklog; }

	//------------------------------------------------------------------------------------------------
	//! Finale research a faction's route has added this season (AddFinaleResearch).
	float GetFinaleResearch(int faction)
	{
		if (faction < 0 || faction >= m_aFinaleResearch.Count())
			return 0;

		return m_aFinaleResearch[faction];
	}

	// ================================================================ Pace by play
	//------------------------------------------------------------------------------------------------
	//! Research counts this many times for how much the server really plays: Cure > Pace by play
	//! (the player-hours a day the pace is set for, 12.5 = 5 regulars) over the player-hours played a
	//! day on the last 7 whole real days, x0.25 to x2.5. 1 while it's off, and until a whole day after
	//! the season's first has been counted. A server down for days counts those days as unplayed.
	float PlayPace()
	{
		float target = m_GM.GetPlayPaceHours();
		if (target <= 0 || m_iPlayDay <= 0)
			return 1;

		int whole = Math.ClampInt(m_iPlayDay - m_iPlayFrom, 0, PLAY_DAYS);
		if (whole <= 0)
			return 1;

		float sum = 0;
		for (int d = 1; d <= whole && d < m_aPlayHours.Count(); d++)
		{
			sum += m_aPlayHours[d];
		}

		float perDay = sum / whole;
		if (perDay <= 0)
			return PLAY_PACE_MAX;

		return Math.Clamp(target / perDay, PLAY_PACE_MIN, PLAY_PACE_MAX);
	}

	//------------------------------------------------------------------------------------------------
	//! Counts the player-hours of this tick into today's real day (players in the world).
	protected void TrackPlay(float dt, int count)
	{
		int now = RZ_Guidance.UnixNow();
		if (now <= 0)
			return;

		int day = now / DAY_SECONDS;
		if (m_iPlayDay <= 0 || m_aPlayHours.IsEmpty())
		{
			// The season's first day (or an older save's) is partial: counting starts with the next
			m_aPlayHours.Clear();
			m_aPlayHours.Insert(0);
			m_iPlayDay = day;
			m_iPlayFrom = day + 1;
		}

		// A new real day (or days, with the server down): today's slot moves back, the oldest drops off
		int shift = Math.ClampInt(day - m_iPlayDay, 0, PLAY_DAYS + 1);
		for (int i = 0; i < shift; i++)
		{
			m_aPlayHours.InsertAt(0, 0);
			if (m_aPlayHours.Count() > PLAY_DAYS + 1)
				m_aPlayHours.Remove(m_aPlayHours.Count() - 1);
		}

		if (day > m_iPlayDay)
			m_iPlayDay = day;

		if (count > 0)
			m_aPlayHours[0] = m_aPlayHours[0] + dt * count / 3600;
	}

	// ================================================================ The lab's capacity
	//------------------------------------------------------------------------------------------------
	//! The lab has a capacity at all (Cure > Lab research per day above 0).
	bool HasCapacity()
	{
		return m_GM.GetLabPerDay() > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Percent the lab works through in a real day (x season speed). 0 = no limit.
	float LabPerDay()
	{
		return m_GM.GetLabPerDay() * m_GM.GetSeasonSpeed();
	}

	//------------------------------------------------------------------------------------------------
	//! The most research that can wait in the backlog. -1 = no cap.
	float BacklogCap()
	{
		if (!HasCapacity() || m_GM.GetLabBacklogDays() <= 0)
			return -1;

		return LabPerDay() * m_GM.GetLabBacklogDays();
	}

	//------------------------------------------------------------------------------------------------
	//! Room left in the backlog. A big number when there's no cap.
	float BacklogRoom()
	{
		float cap = BacklogCap();
		if (cap < 0)
			return 100000;

		return Math.Max(0, cap - m_fBacklog);
	}

	//------------------------------------------------------------------------------------------------
	//! The backlog is at its cap: the lab refuses samples and mission research is lost.
	bool IsLabFull()
	{
		return !m_bCured && BacklogCap() >= 0 && BacklogRoom() <= EMPTY;
	}

	//------------------------------------------------------------------------------------------------
	//! A cure finale questline is registered (the five routes, docs/FINALE.md): research then stops
	//! at 90 percent until the finale takes it on.
	bool HasFinale()
	{
		return m_GM.GetQuestlines() && m_GM.GetQuestlines().HasCureFinale();
	}

	//------------------------------------------------------------------------------------------------
	//! Research waits at 90 percent for a finale: one is registered and research hasn't reached 100 yet.
	bool IsAtFinaleGate()
	{
		return !m_bCured && !m_bGateOpen && !m_bFinalStand && m_fProgress >= FINALE_GATE && HasFinale();
	}

	//------------------------------------------------------------------------------------------------
	//! How far the lab's own work goes: 90 percent while the finale's gate is shut, else 100.
	protected float WorkLimit()
	{
		if (!m_bGateOpen && HasFinale())
			return FINALE_GATE;

		return 100;
	}

	//------------------------------------------------------------------------------------------------
	//! Research one of this player's samples is worth, before the empty lab bonus: Research per
	//! sample x the background and Champion multipliers (x2 at most together) x Cure pace x the pace
	//! by play x season speed. playerId 0: no personal multiplier.
	float SampleWorth(int playerId, float pointsPerSample)
	{
		float mult = 1;
		if (playerId > 0)
		{
			// Halcyon Champion perk "Lab access": deliveries count 25% more
			float perk = 1;
			if (m_GM.GetTier(playerId, RZ_EFaction.INSTITUTE) == RZ_ETier.CHAMPION)
				perk = 1.25;

			float bgMult = 1;
			RZ_Background bg = m_GM.GetPlayerBackground(playerId);
			if (bg)
				bgMult = bg.m_fResearchMult;

			mult = Math.Min(MAX_RESEARCH_MULT, bgMult * perk);
		}

		return pointsPerSample * mult * m_GM.GetCurePace() * PlayPace() * m_GM.GetSeasonSpeed();
	}

	//------------------------------------------------------------------------------------------------
	//! Research delivered now, with the Empty lab bonus on the part of it that fills the backlog up
	//! to half a day of the lab's work: an idle lab doesn't waste its days, and there's nothing to
	//! gain by waiting for it to empty and handing everything in at once.
	protected float WithBonus(float research)
	{
		if (!HasCapacity() || research <= 0)
			return research;

		float gap = Math.Max(0, LabPerDay() * IDLE_DAYS - m_fBacklog);
		return research + m_GM.GetEmptyLabBonus() * Math.Min(research, gap);
	}

	//------------------------------------------------------------------------------------------------
	//! How many of `count` samples the lab takes now: all of them, or as many as fit in the backlog
	//! (the last one may overfill it a little); 0 when it's full or the cure is out.
	int AcceptsSamples(int playerId, int count, float pointsPerSample)
	{
		if (m_bCured || count <= 0)
			return 0;

		float room = BacklogRoom();
		if (room <= EMPTY)
			return 0;

		float worth = SampleWorth(playerId, pointsPerSample);
		if (worth <= 0)
			return count;

		int fit = RZ_Util.RoundToInt(Math.Ceil(room / worth));
		return Math.ClampInt(fit, 1, count);
	}

	//------------------------------------------------------------------------------------------------
	//! The lab works through its backlog for the real time since it last did (also with the server
	//! empty or down), up to 90 percent while the finale's gate is shut; after a week at the gate it
	//! inches up by itself (with or without a capacity). Milestones fire here.
	protected void Work()
	{
		int now = RZ_Guidance.UnixNow();
		if (now <= 0)
			return;

		int elapsed = 0;
		if (m_iLabUnix > 0 && now > m_iLabUnix)
			elapsed = Math.Min(now - m_iLabUnix, MAX_CATCH_UP);

		m_iLabUnix = now;
		if (m_bCured)
			return;

		float before = m_fProgress;
		if (!HasCapacity())
			WorkBacklog(m_fBacklog);
		else if (elapsed > 0)
			WorkBacklog(LabPerDay() * elapsed / DAY_SECONDS);

		if (elapsed > 0 && IsAtFinaleGate())
			Fallback(before, elapsed);

		AfterWork(before);
	}

	//------------------------------------------------------------------------------------------------
	//! Up to `amount` of the backlog becomes research, as far as the work goes (WorkLimit).
	protected void WorkBacklog(float amount)
	{
		float done = Math.Min(m_fBacklog, amount);
		done = Math.Max(0, Math.Min(done, WorkLimit() - m_fProgress));
		m_fBacklog = Math.Max(0, m_fBacklog - done);
		m_fProgress = Math.Min(100, m_fProgress + done);
	}

	//------------------------------------------------------------------------------------------------
	//! At the gate with a finale registered: after a week there (/ season speed) the lab inches on by
	//! itself at Finale fallback a real day (x season speed), so a season whose finale nobody finishes
	//! still ends. before: research before this step's work (the part of the step spent reaching
	//! the gate doesn't count as waiting at it).
	protected void Fallback(float before, int elapsed)
	{
		float waited = elapsed;
		if (before < FINALE_GATE && HasCapacity())
			waited = Math.Max(0, elapsed - (FINALE_GATE - before) / LabPerDay() * DAY_SECONDS);

		float grace = GATE_GRACE_DAYS * DAY_SECONDS / m_GM.GetSeasonSpeed();
		float past = Math.Max(0, m_fGateSeconds + waited - Math.Max(grace, m_fGateSeconds));
		m_fGateSeconds += waited;
		if (past <= 0)
			return;

		// The finale runs: the fallback is the lab-alone track (1 percent = 1 point), never research
		float points = m_GM.GetFinaleFallbackPerDay() * m_GM.GetSeasonSpeed() * past / DAY_SECONDS;
		RZ_CureFinale finale = RZ_CureFinale.Get();
		if (finale && RZ_CureFinale.IsOn())
		{
			finale.AddLabAlone(points);
			return;
		}

		m_fProgress = Math.Min(100, m_fProgress + points);
	}

	//------------------------------------------------------------------------------------------------
	//! No capacity (Lab research per day 0): the whole backlog counts at once, as before the lab had one.
	protected void WorkAll()
	{
		if (m_bCured || m_fBacklog <= 0)
			return;

		float before = m_fProgress;
		WorkBacklog(m_fBacklog);
		AfterWork(before);
	}

	//------------------------------------------------------------------------------------------------
	protected void AfterWork(float before)
	{
		if (m_fProgress <= before)
			return;

		m_bWorkedSinceSync = true;
		if (before < FINALE_GATE && m_fProgress >= FINALE_GATE && IsAtFinaleGate())
		{
			// Heard when someone is in the world (FlushNews)
			RZ_Log.Info("Cure: research reached the finale's 90 percent gate.");
			m_iNews = m_iNews | NEWS_GATE;
			if (RZ_CureFinale.Get())
				RZ_CureFinale.Get().OnGate();
		}

		int milestone = m_iMilestone;
		CheckMilestones();
		if (m_iMilestone != milestone)
			m_GM.OnCureChanged();
	}

	//------------------------------------------------------------------------------------------------
	//! The finale's hook (docs/DESIGN.md, "Cure research"; the finale questlines are planned): research
	//! from a finale route goes straight into the lab's result, past the 90 percent gate, not through
	//! the backlog, the Cure pace or the season speed. faction: whose route it was (-1 none), kept
	//! for the ending. Returns how much counted (0 once the cure is out or while the final stand runs).
	float AddFinaleResearch(float amount, int faction, string reason)
	{
		if (m_bCured || m_bFinalStand || amount <= 0)
			return 0;

		float before = m_fProgress;
		m_fProgress = Math.Min(100, m_fProgress + amount);
		float added = m_fProgress - before;
		if (faction >= 0 && faction < m_aFinaleResearch.Count())
			m_aFinaleResearch[faction] = m_aFinaleResearch[faction] + added;

		RZ_Log.Info(string.Format("Cure: finale research +%1 (faction %2), now %3 percent.", added, faction, RZ_Util.RoundToInt(m_fProgress)));
		m_GM.NotifyAll("Cure research", string.Format("%1 Research: %2 percent.", reason, RZ_Util.RoundToInt(m_fProgress)));
		CheckMilestones();
		m_GM.OnCureChanged();
		return added;
	}

	//------------------------------------------------------------------------------------------------
	//! The finale holds the stand: the batch waits for its seal (RZ_CureFinale).
	protected bool FinaleHolds()
	{
		RZ_CureFinale finale = RZ_CureFinale.Get();
		return finale && HasFinale() && finale.HoldsTheStand();
	}

	//------------------------------------------------------------------------------------------------
	//! The seal (RZ_CureFinale): research to 100 in one AddFinaleResearch call, credited to the
	//! finisher's faction (-1 for the Independent or the lab alone), and the stand starts.
	void Seal(int faction)
	{
		if (m_bCured || m_bFinalStand)
			return;

		if (m_fProgress < 100)
		{
			AddFinaleResearch(100 - m_fProgress, faction, "The batch is sealed.");
			return;
		}

		m_iMilestone = 4;
		StartFinalStand();
		m_GM.OnCureChanged();
	}

	//------------------------------------------------------------------------------------------------
	//! The cure percent everyone sees: with the finale's routes open, 90 plus the furthest track (at
	//! most 99 until the seal); else the lab's own.
	float ShownProgress()
	{
		RZ_CureFinale finale = RZ_CureFinale.Get();
		if (finale && !m_bCured && !m_bFinalStand)
		{
			float shown = finale.ShownPercent();
			if (shown >= 0)
				return shown;
		}

		return m_fProgress;
	}

	//------------------------------------------------------------------------------------------------
	//! This server's line for the lab: "Research 63 percent. Waiting: 14 samples' worth, about 4 days of work."
	string Describe()
	{
		return DescribeLab(ShownProgress(), m_fBacklog, SampleWorth(0, m_GM.GetResearchPerSample()), LabPerDay(), IsLabFull(), IsAtFinaleGate(), m_bCured);
	}

	//------------------------------------------------------------------------------------------------
	//! The lab's line from plain numbers, so the lab status action (on a client, from replicated
	//! values), the journal and the first-spawn news say the same. sampleWorth: research one sample
	//! is worth; perDay: what the lab works through in a real day (0 = no limit).
	static string DescribeLab(float progress, float backlog, float sampleWorth, float perDay, bool full, bool gate, bool cured)
	{
		if (cured)
			return "The cure is complete.";

		// With no capacity (perDay 0) research only waits at the finale's gate, with no days to count
		string s = string.Format("Research %1 percent.", RZ_Util.RoundToInt(progress));
		if (backlog < EMPTY)
			s += " Nothing waiting: the lab is ready for more.";
		else if (perDay <= 0)
			s += string.Format(" Waiting: %1.", SamplesText(backlog, sampleWorth));
		else
			s += string.Format(" Waiting: %1, %2 of work.", SamplesText(backlog, sampleWorth), WorkText(backlog / perDay));

		if (gate)
			s += " The work has stopped at 90 percent: the last step needs more than samples.";
		else if (full)
			s += " The cold store is full: no more samples until the lab works through some.";

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! "14 samples' worth", "1 sample's worth", "less than a sample's worth".
	protected static string SamplesText(float backlog, float sampleWorth)
	{
		if (sampleWorth <= 0)
			return "some research";

		int n = RZ_Util.RoundToInt(backlog / sampleWorth);
		if (n < 1)
			return "less than a sample's worth";

		if (n == 1)
			return "1 sample's worth";

		return string.Format("%1 samples' worth", n);
	}

	//------------------------------------------------------------------------------------------------
	//! "about 4 days", "about a day", "about 9 hours", "under an hour".
	protected static string WorkText(float days)
	{
		if (days >= 1.5)
			return string.Format("about %1 days", RZ_Util.RoundToInt(days));

		if (days >= 0.75)
			return "about a day";

		int hours = RZ_Util.RoundToInt(days * 24);
		if (hours < 1)
			return "under an hour";

		if (hours == 1)
			return "about an hour";

		return string.Format("about %1 hours", hours);
	}

	//------------------------------------------------------------------------------------------------
	int GetLabAntidotes() { return m_iLabAntidotes; }

	//------------------------------------------------------------------------------------------------
	int GetStolenAntidotes() { return m_iStolenAntidotes; }

	//------------------------------------------------------------------------------------------------
	int GetBatchesLost() { return m_iBatchesLost; }

	//------------------------------------------------------------------------------------------------
	int GetBatchesStolen() { return m_iBatchesStolen; }

	//------------------------------------------------------------------------------------------------
	int GetLabStabilisers() { return m_iLabStabilisers; }

	// ================================================================ Treatment supply by research
	//------------------------------------------------------------------------------------------------
	//! Seconds between antidote doses: 30, 20, 15 and 10 minutes by breakthrough.
	float AntidoteInterval()
	{
		switch (Math.ClampInt(m_iMilestone, 0, 3))
		{
			case 0: return 1800;
			case 1: return 1200;
			case 2: return 900;
		}

		return 600;
	}

	//------------------------------------------------------------------------------------------------
	//! Most doses in the lab's stock: 2, 3, 4 and 5 by breakthrough.
	int AntidoteCap()
	{
		return Math.ClampInt(2 + m_iMilestone, 2, LAB_ANTIDOTE_CAP);
	}

	//------------------------------------------------------------------------------------------------
	//! Seconds between stabilisers: rare at the start, more common after 25%, plentiful after 75%.
	float StabiliserInterval()
	{
		switch (Math.ClampInt(m_iMilestone, 0, 3))
		{
			case 0: return 5400;
			case 1: return 2700;
			case 2: return 2700;
		}

		return 900;
	}

	//------------------------------------------------------------------------------------------------
	int StabiliserCap()
	{
		switch (Math.ClampInt(m_iMilestone, 0, 3))
		{
			case 0: return 1;
			case 1: return 2;
			case 2: return 2;
		}

		return STABILISER_CAP;
	}

	//------------------------------------------------------------------------------------------------
	//! Stabiliser price multiplier: double until 50%, then the shop's own price.
	float StabiliserPriceMult()
	{
		if (m_iMilestone >= 2)
			return 1;

		return 2;
	}

	//------------------------------------------------------------------------------------------------
	//! Sells one stabiliser from Halcyon's stock. False if there's none.
	bool TakeStabiliser()
	{
		if (m_iLabStabilisers <= 0)
			return false;

		m_iLabStabilisers--;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Sells one dose from the lab's stock (Halcyon's quartermaster). False if there's none.
	bool TakeLabAntidote()
	{
		if (m_iLabAntidotes <= 0)
			return false;

		m_iLabAntidotes--;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Sells one stolen dose (the Red Hand's black market). False if there's none.
	bool TakeStolenAntidote()
	{
		if (m_iStolenAntidotes <= 0)
			return false;

		m_iStolenAntidotes--;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The Red Hand robs the lab's store: up to `count` doses move to its black market.
	//! Returns how many were taken.
	int StealLabAntidotes(int count)
	{
		int taken = Math.Min(count, m_iLabAntidotes);
		taken = Math.Min(taken, StolenCap() - m_iStolenAntidotes);
		if (taken <= 0)
			return 0;

		m_iLabAntidotes -= taken;
		m_iStolenAntidotes += taken;
		return taken;
	}

	//------------------------------------------------------------------------------------------------
	bool IsCured()
	{
		return m_bCured;
	}

	//------------------------------------------------------------------------------------------------
	bool IsFinalStand()
	{
		return m_bFinalStand;
	}

	//------------------------------------------------------------------------------------------------
	//! Whole minutes of the final stand still to hold (at least 1 while it runs).
	int GetFinalStandMinutesLeft()
	{
		return Math.Max(1, RZ_Util.RoundToInt(m_fFinalStandLeft / 60));
	}

	//------------------------------------------------------------------------------------------------
	float GetSpreadMultiplier()
	{
		if (m_bCured)
			return 0.2;

		// -15% at 25%, -45% in all at 75%; the 50% breakthrough only changes treatment supply
		if (m_iMilestone >= 3)
			return 0.55;

		if (m_iMilestone >= 1)
			return 0.85;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	float GetInfectionTimeMultiplier()
	{
		if (m_iMilestone >= 3)
			return 1.5;
		if (m_iMilestone >= 1)
			return 1.25;
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Samples the lab has taken (ask AcceptsSamples first: the ones it refuses stay with the player).
	//! They go into the backlog, and the lab works through them over the next days. Each counts
	//! SampleWorth (the Lab Technician's x2 and the Champion's "Lab access" +25%, x2 at most; the
	//! pace by play), and the Empty lab bonus on the part that fills a near-empty backlog.
	void AddSamples(int playerId, int count, float pointsPerSample)
	{
		if (m_bCured || count <= 0)
			return;

		RZ_ETier halcyon = m_GM.GetTier(playerId, RZ_EFaction.INSTITUTE);
		float research = WithBonus(count * SampleWorth(playerId, pointsPerSample));
		m_fBacklog += research;

		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		profile.m_iSamplesDelivered += count;
		profile.m_iLifeSamples += count;
		if (m_GM.GetHall())
			m_GM.GetHall().AddSamples(playerId, count);

		// Hunted by Halcyon: the lab takes the samples, but you get nothing for them
		if (RZ_Reputation.IsHunted(halcyon))
			m_GM.NotifyPlayer(playerId, "Halcyon", "The guards take your samples and point you to the door. No thanks, no Fame.");
		else
			m_GM.AddFame(playerId, RZ_EFaction.INSTITUTE, 3 * count);

		// No capacity (Lab research per day 0): it counts at once
		if (!HasCapacity())
			WorkAll();

		string name = GetGame().GetPlayerManager().GetPlayerName(playerId);
		m_GM.NotifyAll("Cure research", string.Format("%1 delivered %2 sample(s). %3", name, count, Describe()));
		m_GM.OnCureChanged();
	}

	//------------------------------------------------------------------------------------------------
	//! Research from missions (data, readings, field samples) rather than delivered samples. It goes
	//! into the backlog like samples do; what doesn't fit is lost, and the line everyone hears (the
	//! mission's result) says so. Returns the research kept.
	float AddResearch(float points, string reason)
	{
		if (m_bCured || points <= 0)
			return 0;

		// Missions count a share of what they say (Cure > Mission research), x Cure pace, the pace by
		// play and season speed, with the Empty lab bonus on the part that fills a near-empty backlog
		float worth = WithBonus(points * m_GM.GetMissionResearchMult() * m_GM.GetCurePace() * PlayPace() * m_GM.GetSeasonSpeed());
		float kept = Math.Min(worth, BacklogRoom());
		string note = "";
		if (kept < worth - EMPTY)
		{
			if (kept <= EMPTY)
			{
				kept = 0;
				note = " The lab's cold store is full: none of it could be kept.";
			}
			else
			{
				note = " The lab's cold store is full: only part of it could be kept.";
			}

			RZ_Log.Info(string.Format("Cure: the lab's backlog is full; %1 of %2 research from a mission was lost.", worth - kept, worth));
		}

		m_fBacklog += kept;
		if (!HasCapacity())
			WorkAll();

		m_GM.NotifyAll("Cure research", string.Format("%1%2 %3", reason, note, Describe()));
		m_GM.OnCureChanged();
		return kept;
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<IEntity> players)
	{
		TickAnnex(dt, players);
		if (m_bCured)
			return;

		TrackPlay(dt, players.Count());

		// The lab works in real time, once a minute (Work counts the real seconds, online or not);
		// clients hear of its progress at most once a minute
		m_fWorkTimer += dt;
		if (m_fWorkTimer >= WORK_SECONDS)
		{
			m_fWorkTimer = 0;
			Work();
		}

		// Breakthroughs the lab made with nobody in the world are told to the first who is
		if (m_iNews != 0 && !players.IsEmpty())
			FlushNews();

		m_fSyncTimer += dt;
		if (m_fSyncTimer >= SYNC_SECONDS)
		{
			m_fSyncTimer = 0;
			if (m_bWorkedSinceSync)
			{
				m_bWorkedSinceSync = false;
				m_GM.OnCureChanged();
			}
		}

		// Antidotes from day one; research makes them faster and the stock bigger
		m_fAntidoteTimer += dt;
		if (m_fAntidoteBoostLeft > 0)
		{
			m_fAntidoteBoostLeft -= dt;
			m_fAntidoteTimer += dt;
		}

		if (m_fAntidoteTimer >= AntidoteInterval())
		{
			m_fAntidoteTimer = 0;
			MakeAntidote();
		}

		m_fStabiliserTimer += dt;
		if (m_fStabiliserTimer >= StabiliserInterval())
		{
			m_fStabiliserTimer = 0;
			if (m_GM.HasLab() && m_iLabStabilisers < StabiliserCap())
				m_iLabStabilisers++;
		}

		// Like the outbreak's clock, the stand waits while nobody is online: an empty server (or one
		// restarted mid-stand) doesn't lose the batch to "nobody held the lab"
		if (m_bFinalStand && !players.IsEmpty())
			TickFinalStand(dt, players);
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckMilestones()
	{
		while (m_iMilestone < 4 && m_fProgress >= (m_iMilestone + 1) * 25)
		{
			m_iMilestone++;
			OnMilestone(m_iMilestone);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The breakthrough's effects come at once; its news waits for someone in the world (FlushNews),
	//! since the lab also works with nobody online. The final stand has its own first-spawn notice.
	protected void OnMilestone(int milestone)
	{
		RZ_WorldWriters.CureMilestone(milestone * 25, m_GM.GetLabPosition());
		if (milestone < 4 && m_GM.GetFactionLife())
			m_GM.GetFactionLife().OnBreakthrough(milestone);

		// 25%: Halcyon's range map for its Known (Zombies/RZ_TribePeople.c)
		if (m_GM.GetTribes())
			m_GM.GetTribes().GetPeople().OnMilestone(milestone);

		if (milestone < 4)
			m_iNews = m_iNews | (1 << milestone);

		if (milestone == 2)
			MakeAntidote();

		if (milestone == 4)
		{
			// The finale: research at 100 from the backlog after a lost stand makes the batch ready,
			// and the stand waits for its seal
			if (FinaleHolds())
			{
				RZ_CureFinale.Get().OnBacklogBatch();
				return;
			}

			StartFinalStand();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The breakthroughs and the gate the lab reached since the last player was in the world, told
	//! now (in the order they came). Once the final stand has started they're history: nothing is told.
	protected void FlushNews()
	{
		int news = m_iNews;
		m_iNews = 0;
		if (m_bFinalStand || m_iMilestone >= 4)
			return;

		bool told = false;
		for (int m = 1; m <= 3; m++)
		{
			if ((news & (1 << m)) == 0)
				continue;

			told = true;
			switch (m)
			{
				case 1:
					m_GM.NotifyAll("Breakthrough: Field antiviral", "Halcyon has slowed the infection. It spreads slower and bites take longer to kill. The lab makes antidotes faster, and stabilisers are easier to come by.");
					break;
				case 2:
					m_GM.NotifyAll("Breakthrough: Bite treatment", "The lab has the antidote right: a dose every quarter of an hour now, more in stock, and stabilisers at a fair price. Pharmacies may still hold the odd dose.");
					break;
				case 3:
					m_GM.NotifyAll("Breakthrough: Immunity serum", "Infection spread is slowing a lot. Antidotes every ten minutes, stabilisers on every shelf at the lab. One final push is all that's left.");
					break;
			}
		}

		if ((news & NEWS_GATE) != 0 && IsAtFinaleGate())
			m_GM.NotifyAll("Cure research", "The lab has gone as far as samples can take it: 90 percent. What's missing isn't in a sample case, and every faction in the Zone thinks it knows where it is.");

		// The broadcast's "Reports of research inside the zone" run from when people heard it
		if (told && m_GM.GetBroadcast())
			m_GM.GetBroadcast().OnBreakthrough();
	}

	//------------------------------------------------------------------------------------------------
	//! One dose into the lab's stock, if there's room. Nothing is spawned in the world.
	protected void MakeAntidote()
	{
		if (!m_GM.HasLab() || m_iLabAntidotes >= AntidoteCap())
			return;

		m_iLabAntidotes++;

		// Only people Halcyon knows hear about it (it's every 10 to 30 minutes)
		string text = string.Format("A fresh antidote dose is ready at the lab. Doses in stock: %1.", m_iLabAntidotes);
		array<int> all = {};
		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int playerId : all)
		{
			if (RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, RZ_EFaction.INSTITUTE), RZ_ETier.KNOWN))
				m_GM.NotifyPlayer(playerId, "Halcyon", text);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void StartFinalStand()
	{
		// Season > No stand in the last hours: near a fixed wipe the stand is held back instead of
		// the wipe being pushed (docs/EPILOGUE.md, Edge cases)
		if (m_GM.GetSeasons() && m_GM.GetSeasons().HoldsStand())
		{
			if (!m_bStandHeldTold)
			{
				m_bStandHeldTold = true;
				m_GM.NotifyAll("Halcyon", "The batch is ready, and the gates open soon either way. The director won't start the synthesis this late.");
				RZ_Log.Info("Cure: the final stand is held back in the season's last hours (Season > No stand in the last hours).");
			}

			return;
		}

		m_bFinalStand = true;
		// Research has reached 100 once: a lost batch goes back to the lab's normal work, not to the finale
		m_bGateOpen = true;
		m_fFinalStandLeft = FINAL_STAND_SECONDS;
		m_fAbandonedFor = 0;
		m_bStandReached = false;
		m_fWaveTimer = WAVE_INTERVAL_SECONDS;
		RZ_CureFinale finale = RZ_CureFinale.Get();
		if (finale && HasFinale())
			finale.OnStandStarted();

		// The questlines settle what must be settled before the stand (Proof's Before the stand)
		if (m_GM.GetQuestlines())
			m_GM.GetQuestlines().OnFinalStandStart();

		if (finale && HasFinale() && finale.IsOpen())
		{
			m_GM.NotifyAllAs("THE FINAL STAND", "The cure is being synthesized. Every infected for miles can sense it. Hold the lab for 15 minutes. Whoever holds the clean room when it's done controls the cure.", RZ_ENoticeTag.URGENT);
			return;
		}

		m_GM.NotifyAllAs("THE FINAL STAND", "The cure is being synthesized. Every infected for miles can sense it. Hold the lab for 15 minutes! A handful of steady people can do it; more of you draw more of them.", RZ_ENoticeTag.URGENT);
	}

	//------------------------------------------------------------------------------------------------
	protected void TickFinalStand(float dt, array<IEntity> players)
	{
		vector lab = m_GM.GetLabPosition();
		int atLab = RZ_Mission.CountPlayersNear(players, lab, LAB_DEFENCE_RADIUS);

		// The stand (its clock, waves and "abandoned") only starts once a player has reached the
		// lab since it (re)started, so a restart doesn't lose the batch before anyone can get there
		if (!m_bStandReached)
		{
			if (atLab == 0)
				return;

			m_bStandReached = true;
		}

		if (atLab == 0)
			m_fAbandonedFor += dt;
		else
			m_fAbandonedFor = 0;

		if (m_fAbandonedFor >= ABANDON_LIMIT_SECONDS)
		{
			LoseBatch("The batch is lost", "Nobody held the lab. The infected got into the clean room before Halcyon could seal it, and the batch is ruined. Research has dropped to 90 percent.");
			return;
		}

		// Waves scale with the defenders, so about five players can hold it: one horde a wave,
		// one more per 6 players at the lab, three at most
		m_fWaveTimer += dt;
		if (m_fWaveTimer >= WAVE_INTERVAL_SECONDS)
		{
			m_fWaveTimer = 0;
			int defenders = RZ_Mission.CountPlayersNear(players, lab, LAB_DEFENCE_RADIUS);
			int hordes = Math.ClampInt(1 + defenders / 6, 1, 3);

			// With the packs: from the crawlers of the towns within 3 km and the nearest pack's
			// hunters, 70-130% of these, the last wave bringing a Furious pack (RZ_TribePeople)
			bool last = m_fFinalStandLeft <= WAVE_INTERVAL_SECONDS;
			int sent = 0;
			if (m_GM.GetTribes())
				sent = m_GM.GetTribes().GetPeople().StandWave(lab, hordes, last);

			for (int h = 0; h < hordes && sent == 0; h++)
			{
				m_GM.GetSpawner().SpawnHorde(lab, 350 + 50 * h, -1, true);
			}
		}

		// The finale: who holds the clean room, and overtime while a challenger is counting
		RZ_CureFinale finale = RZ_CureFinale.Get();
		bool contest = finale && HasFinale();
		if (contest)
			finale.StandTick(dt, players);

		m_fFinalStandLeft -= dt;
		if (m_fFinalStandLeft > 0)
			return;

		if (contest && finale.WantsOvertime(dt))
			return;

		CompleteCure(players);
	}

	//------------------------------------------------------------------------------------------------
	//! The final stand ends without a cure: research back to 90 percent. The lab never falls.
	protected void LoseBatch(string title, string text)
	{
		m_bFinalStand = false;
		m_fProgress = 90;
		m_iMilestone = 3;
		m_iBatchesLost++;
		m_GM.NotifyAll(title, text);
		if (RZ_CureFinale.Get() && HasFinale())
			RZ_CureFinale.Get().OnStandLost();

		if (m_GM.GetTribes())
			m_GM.GetTribes().GetPeople().OnStandLost(m_GM.GetLabPosition());
		if (m_GM.GetFactionLife())
			m_GM.GetFactionLife().OnFinalStandLost();

		if (m_GM.GetQuestlines())
			m_GM.GetQuestlines().OnFinalStandEnd(false);

		m_GM.OnCureChanged();
	}

	//------------------------------------------------------------------------------------------------
	//! The Red Hand's Take the Lab: the batch is stolen during the final stand, which ends it as
	//! if it were ruined (research back to 90 percent, a new final stand at 100). Two doses go to
	//! the black market, within its cap. Nobody wins the cure this way. False if no stand runs.
	bool StealBatch()
	{
		// Under a finale Take the Lab changes who holds the clean room instead (RZ_CureFinale)
		if (!m_bFinalStand || HasFinale())
			return false;

		m_iStolenAntidotes = Math.Min(StolenCap(), m_iStolenAntidotes + 2);
		m_iBatchesStolen++;
		LoseBatch("The batch is gone", "The Red Hand walked out of the clean room with the batch while Halcyon was holding the doors. It won't cure anyone in their hands: it needs the lab. Research has dropped to 90 percent, and Halcyon has to start the synthesis again.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Mobile Lab: a second lab prefab where samples can be handed in, for `seconds`. One at a time;
	//! a new one replaces the old. Removed when its time is up and nobody is within 100 m.
	void SetAnnex(IEntity annex, float seconds)
	{
		if (m_Annex && m_Annex != annex)
			RZ_Util.DeleteUnit(m_Annex);

		m_Annex = annex;
		m_fAnnexLeft = seconds;
	}

	//------------------------------------------------------------------------------------------------
	protected void TickAnnex(float dt, array<IEntity> players)
	{
		if (!m_Annex)
			return;

		m_fAnnexLeft -= dt;
		if (m_fAnnexLeft > 0 || RZ_Util.NearestDistance(m_Annex.GetOrigin(), players) <= 100)
			return;

		RZ_Util.DeleteUnit(m_Annex);
		m_Annex = null;
		m_GM.NotifyAll("Halcyon", "The field annex has packed up and gone back to the lab.");
	}

	//------------------------------------------------------------------------------------------------
	//! Power the Lab: antidote doses come twice as fast for `seconds` (once the lab makes them).
	void BoostAntidotes(float seconds)
	{
		m_fAntidoteBoostLeft = Math.Max(m_fAntidoteBoostLeft, seconds);
	}

	//------------------------------------------------------------------------------------------------
	protected void CompleteCure(array<IEntity> players)
	{
		m_bFinalStand = false;
		m_bCured = true;

		foreach (RZ_Settlement s : m_GM.GetSettlements())
		{
			s.m_fInfection = s.m_fInfection * 0.3;
		}

		// The finale: whoever holds the clean room controls the cure (before the winner is chosen)
		RZ_CureFinale finale = RZ_CureFinale.Get();
		bool contest = finale && HasFinale();
		if (contest)
			finale.OnStandWon();

		if (m_GM.GetQuestlines())
			m_GM.GetQuestlines().OnFinalStandEnd(true);

		vector lab = m_GM.GetLabPosition();
		array<int> defenders = {};
		foreach (IEntity p : players)
		{
			if (contest || RZ_Util.DistXZ(p.GetOrigin(), lab) > LAB_DEFENCE_RADIUS)
				continue;

			int id = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(p);
			defenders.Insert(id);

			// The hall of fame: they held the final stand
			if (m_GM.GetHall())
				m_GM.GetHall().AddStand(id);
		}

		m_GM.NotifyAll("THE CURE IS HERE", "The news is going out over every frequency, and with it the proof. The quarantine is lifted: the drones have gone home and the line has stopped shooting. In the last days the north road opens for processing; the other barriers open with the wipe, and the next season starts a new outbreak, in a new Zone.");
		if (m_GM.GetFactionLife())
			m_GM.GetFactionLife().OnCureDone();

		// Work that ends with the cure (research, the jobs at the line) is called off now, not
		// left running for its remaining time
		if (m_GM.GetMissions())
			m_GM.GetMissions().OnCured();

		// The winner is chosen before the stand's own reward, so the stand's Fame can't decide it:
		// with the finale, whoever holds the clean room (nobody: the season's Fame leader); without
		// it, the season's Fame leader
		m_GM.OnCureCompleted();
		if (contest)
			finale.PayStandFame();

		foreach (int defender : defenders)
		{
			m_GM.AddFame(defender, RZ_EFaction.INSTITUTE, 50);
			m_GM.AddFame(defender, RZ_EFaction.SURVIVORS, 30);
		}

		// Aftermath, quietly: the cure news is the line everyone hears, not the stage's
		m_GM.GetDirector().SetStage(RZ_EOutbreakStage.AFTERMATH, false);
		m_GM.OnCureChanged();
	}
}
