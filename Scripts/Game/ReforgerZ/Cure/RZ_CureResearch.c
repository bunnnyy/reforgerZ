//! Server-wide cure research. Players kill special infected, carry the samples
//! to the lab and deliver them. Progress changes the world for everyone:
//!
//!  25%  Field antiviral    infection spreads slower, bites take longer to kill
//!  50%  Bite treatment     the lab starts producing antidotes (players will fight over them)
//!  75%  Immunity serum     spread slows further, bites take even longer
//! 100%  Final stand        hold the lab against wave after wave for 15 minutes.
//!                          Win: the cure goes out and the season ends.
//!                          Abandon the lab: it falls and research drops back to 90%.
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

	static const float FINAL_STAND_SECONDS = 900;
	static const float ABANDON_LIMIT_SECONDS = 90;
	static const float WAVE_INTERVAL_SECONDS = 120;
	static const float ANTIDOTE_INTERVAL_SECONDS = 900;
	static const float LAB_DEFENCE_RADIUS = 150;

	//------------------------------------------------------------------------------------------------
	void RZ_CureResearch(RZ_GameModeComponent gm)
	{
		m_GM = gm;
	}

	//------------------------------------------------------------------------------------------------
	void Restore(float progress, int milestone, bool cured)
	{
		m_fProgress = progress;
		m_iMilestone = milestone;
		m_bCured = cured;

		// A restart in the middle of the final stand starts it again from the beginning
		if (m_iMilestone >= 4 && !m_bCured)
			StartFinalStand();
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
		float before = m_fProgress;
		m_fProgress = Math.Min(100, m_fProgress + count * pointsPerSample * bg.m_fResearchMult);

		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		profile.m_iSamplesDelivered += count;
		profile.m_iLifeSamples += count;
		m_GM.AddFame(playerId, RZ_EFaction.INSTITUTE, 3 * count);

		string name = GetGame().GetPlayerManager().GetPlayerName(playerId);
		m_GM.NotifyAll("Cure research", string.Format("%1 delivered %2 sample(s). Research: %3 percent.",
			name, count, RZ_Util.RoundToInt(m_fProgress)));

		if (m_fProgress != before)
			CheckMilestones();

		m_GM.OnCureChanged();
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<IEntity> players)
	{
		if (m_bCured)
			return;

		if (m_iMilestone >= 2)
		{
			m_fAntidoteTimer += dt;
			if (m_fAntidoteTimer >= ANTIDOTE_INTERVAL_SECONDS)
			{
				m_fAntidoteTimer = 0;
				m_GM.SpawnAntidote();
			}
		}

		if (m_bFinalStand)
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
		switch (milestone)
		{
			case 1:
				m_GM.NotifyAll("Breakthrough: Field antiviral", "The Institute has slowed the infection. It spreads slower and bites take longer to kill.");
				break;
			case 2:
				m_GM.NotifyAll("Breakthrough: Bite treatment", "The lab can make antidotes now. Fresh doses will show up at the lab regularly.");
				m_GM.SpawnAntidote();
				break;
			case 3:
				m_GM.NotifyAll("Breakthrough: Immunity serum", "Infection spread is slowing a lot. One final push is all that's left.");
				break;
			case 4:
				StartFinalStand();
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void StartFinalStand()
	{
		m_bFinalStand = true;
		m_fFinalStandLeft = FINAL_STAND_SECONDS;
		m_fAbandonedFor = 0;
		m_fWaveTimer = WAVE_INTERVAL_SECONDS;
		m_GM.NotifyAll("THE FINAL STAND", "The cure is being synthesized. Every infected for miles can sense it. Hold the lab for 15 minutes!");
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
			m_bFinalStand = false;
			m_fProgress = 90;
			m_iMilestone = 3;
			m_GM.NotifyAll("The lab has fallen", "No one held the line. The batch is ruined. Research has dropped to 90 percent.");
			m_GM.OnCureChanged();
			return;
		}

		m_fWaveTimer += dt;
		if (m_fWaveTimer >= WAVE_INTERVAL_SECONDS)
		{
			m_fWaveTimer = 0;
			m_GM.GetSpawner().SpawnHorde(lab, 350);
			m_GM.GetSpawner().SpawnHorde(lab, 400);
		}

		m_fFinalStandLeft -= dt;
		if (m_fFinalStandLeft <= 0)
			CompleteCure(players);
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
		foreach (IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), lab) > LAB_DEFENCE_RADIUS)
				continue;

			int id = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(p);
			m_GM.AddFame(id, RZ_EFaction.INSTITUTE, 50);
			m_GM.AddFame(id, RZ_EFaction.SURVIVORS, 30);
		}

		m_GM.NotifyAll("THE CURE IS HERE", "The cure is going out over every frequency. You did it. This season is over: the next wipe starts a new outbreak.");
		m_GM.GetDirector().SetStage(RZ_EOutbreakStage.AFTERMATH);
		m_GM.OnCureChanged();
	}
}
