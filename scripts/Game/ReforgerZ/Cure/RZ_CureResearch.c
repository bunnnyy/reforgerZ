//! Server-wide cure research. Players kill special infected, carry the samples
//! to the lab and deliver them. Progress changes the world for everyone:
//!
//!  25%  Field antiviral    infection spreads slower, bites take longer to kill
//!  50%  Bite treatment     (no world effect of its own; supply below)
//!  75%  Immunity serum     spread slows further, bites take even longer
//! 100%  Final stand        hold the lab against waves for 15 minutes (paused while nobody is online).
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
	protected float m_fFinalStandLeft;
	protected float m_fAbandonedFor;
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

	static const float FINAL_STAND_SECONDS = 900;
	static const float ABANDON_LIMIT_SECONDS = 90;
	static const float WAVE_INTERVAL_SECONDS = 150;
	static const int LAB_ANTIDOTE_CAP = 5;		// the most the stock ever holds (after 75%)
	static const int STABILISER_CAP = 5;
	static const float MAX_RESEARCH_MULT = 2;	// background x Champion perk, per sample
	static const int STOLEN_ANTIDOTE_CAP = 4;
	static const int BLACK_MARKET_EXTRA = 2;	// season rules: the black market everywhere holds two more
	static const float LAB_DEFENCE_RADIUS = 150;

	//------------------------------------------------------------------------------------------------
	//! Most stolen doses the Red Hand's black market holds (4, or 6 with the season's open black market).
	static int StolenCap()
	{
		if (RZ_Seasons.Mods().m_bOpenBlackMarket)
			return STOLEN_ANTIDOTE_CAP + BLACK_MARKET_EXTRA;

		return STOLEN_ANTIDOTE_CAP;
	}

	//------------------------------------------------------------------------------------------------
	void RZ_CureResearch(RZ_GameModeComponent gm)
	{
		m_GM = gm;
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

		// A restart in the middle of the final stand starts it again from the beginning
		if (m_iMilestone >= 4 && !m_bCured)
			StartFinalStand();
	}

	//------------------------------------------------------------------------------------------------
	int GetLabAntidotes() { return m_iLabAntidotes; }

	//------------------------------------------------------------------------------------------------
	int GetStolenAntidotes() { return m_iStolenAntidotes; }

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

		return 1 - 0.15 * Math.Min(m_iMilestone, 3);
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
	void AddSamples(int playerId, int count, float pointsPerSample)
	{
		if (m_bCured || count <= 0)
			return;

		RZ_Background bg = m_GM.GetPlayerBackground(playerId);
		RZ_ETier halcyon = m_GM.GetTier(playerId, RZ_EFaction.INSTITUTE);

		// Halcyon Champion perk "Lab access": deliveries count 25% more. With the background's
		// multiplier (the Lab Technician's x2) it's never more than x2 a sample.
		float perk = 1;
		if (halcyon == RZ_ETier.CHAMPION)
			perk = 1.25;

		float mult = Math.Min(MAX_RESEARCH_MULT, bg.m_fResearchMult * perk);
		float before = m_fProgress;
		m_fProgress = Math.Min(100, m_fProgress + count * pointsPerSample * mult * m_GM.GetSeasonSpeed());

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

		string name = GetGame().GetPlayerManager().GetPlayerName(playerId);
		m_GM.NotifyAll("Cure research", string.Format("%1 delivered %2 sample(s). Research: %3 percent.",
			name, count, RZ_Util.RoundToInt(m_fProgress)));

		if (m_fProgress != before)
			CheckMilestones();

		m_GM.OnCureChanged();
	}

	//------------------------------------------------------------------------------------------------
	//! Research from missions (data, readings, field samples) rather than delivered samples.
	void AddResearch(float points, string reason)
	{
		if (m_bCured || points <= 0)
			return;

		// Missions count a share of what they say (Cure > Mission research), x season speed
		points = points * m_GM.GetMissionResearchMult() * m_GM.GetSeasonSpeed();
		m_fProgress = Math.Min(100, m_fProgress + points);
		m_GM.NotifyAll("Cure research", string.Format("%1 Research: %2 percent.", reason, RZ_Util.RoundToInt(m_fProgress)));
		CheckMilestones();
		m_GM.OnCureChanged();
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<IEntity> players)
	{
		TickAnnex(dt, players);
		if (m_bCured)
			return;

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
	protected void OnMilestone(int milestone)
	{
		if (milestone < 4 && m_GM.GetFactionLife())
			m_GM.GetFactionLife().OnBreakthrough(milestone);

		switch (milestone)
		{
			case 1:
				m_GM.NotifyAll("Breakthrough: Field antiviral", "Halcyon has slowed the infection. It spreads slower and bites take longer to kill. The lab makes antidotes faster, and stabilisers are easier to come by.");
				break;
			case 2:
				m_GM.NotifyAll("Breakthrough: Bite treatment", "The lab has the antidote right: a dose every quarter of an hour now, more in stock, and stabilisers at a fair price. Pharmacies may still hold the odd dose.");
				MakeAntidote();
				break;
			case 3:
				m_GM.NotifyAll("Breakthrough: Immunity serum", "Infection spread is slowing a lot. Antidotes every ten minutes, stabilisers on every shelf at the lab. One final push is all that's left.");
				break;
			case 4:
				StartFinalStand();
				break;
		}
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
		m_bFinalStand = true;
		m_fFinalStandLeft = FINAL_STAND_SECONDS;
		m_fAbandonedFor = 0;
		m_fWaveTimer = WAVE_INTERVAL_SECONDS;
		m_GM.NotifyAll("THE FINAL STAND", "The cure is being synthesized. Every infected for miles can sense it. Hold the lab for 15 minutes! A handful of steady people can do it; more of you draw more of them.");
	}

	//------------------------------------------------------------------------------------------------
	protected void TickFinalStand(float dt, array<IEntity> players)
	{
		vector lab = m_GM.GetLabPosition();
		if (RZ_Mission.CountPlayersNear(players, lab, LAB_DEFENCE_RADIUS) == 0)
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
			for (int h = 0; h < hordes; h++)
			{
				m_GM.GetSpawner().SpawnHorde(lab, 350 + 50 * h);
			}
		}

		m_fFinalStandLeft -= dt;
		if (m_fFinalStandLeft <= 0)
			CompleteCure(players);
	}

	//------------------------------------------------------------------------------------------------
	//! The final stand ends without a cure: research back to 90 percent. The lab never falls.
	protected void LoseBatch(string title, string text)
	{
		m_bFinalStand = false;
		m_fProgress = 90;
		m_iMilestone = 3;
		m_GM.NotifyAll(title, text);
		if (m_GM.GetFactionLife())
			m_GM.GetFactionLife().OnFinalStandLost();

		m_GM.OnCureChanged();
	}

	//------------------------------------------------------------------------------------------------
	//! The Red Hand's Take the Lab: the batch is stolen during the final stand, which ends it as
	//! if it were ruined (research back to 90 percent, a new final stand at 100). Two doses go to
	//! the black market, within its cap. Nobody wins the cure this way. False if no stand runs.
	bool StealBatch()
	{
		if (!m_bFinalStand)
			return false;

		m_iStolenAntidotes = Math.Min(StolenCap(), m_iStolenAntidotes + 2);
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

		vector lab = m_GM.GetLabPosition();
		array<int> defenders = {};
		foreach (IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), lab) > LAB_DEFENCE_RADIUS)
				continue;

			int id = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(p);
			defenders.Insert(id);

			// The hall of fame: they held the final stand
			if (m_GM.GetHall())
				m_GM.GetHall().AddStand(id);
		}

		m_GM.NotifyAll("THE CURE IS HERE", "The cure is going out over every frequency, and with it the proof. The quarantine is lifted and the gates are opening. This season is over: the next wipe starts a new outbreak, in a new Zone.");
		if (m_GM.GetFactionLife())
			m_GM.GetFactionLife().OnCureDone();

		// The winner is chosen on the season's Fame before the stand's own reward, so the last
		// fight (always Halcyon and the Haven) can't decide it
		m_GM.OnCureCompleted();
		foreach (int defender : defenders)
		{
			m_GM.AddFame(defender, RZ_EFaction.INSTITUTE, 50);
			m_GM.AddFame(defender, RZ_EFaction.SURVIVORS, 30);
		}

		m_GM.GetDirector().SetStage(RZ_EOutbreakStage.AFTERMATH);
		m_GM.OnCureChanged();
	}
}
