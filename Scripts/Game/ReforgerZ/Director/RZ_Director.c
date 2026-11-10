//! The Director runs the outbreak: it moves the server through the stages and
//! spreads infection from town to town. Players push back by clearing towns
//! and by researching the cure.
class RZ_Director
{
	protected RZ_GameModeComponent m_GM;
	protected ref array<ref RZ_StageSettings> m_aStages = {};
	protected RZ_EOutbreakStage m_eStage = RZ_EOutbreakStage.EARLY_OUTBREAK;
	protected float m_fStageMinutes;

	protected static const float NEIGHBOUR_DISTANCE = 2500;
	protected static const float SPREAD_SOURCE_INFECTION = 60;

	//------------------------------------------------------------------------------------------------
	void RZ_Director(RZ_GameModeComponent gm, array<ref RZ_StageSettings> configured)
	{
		m_GM = gm;

		// One default per stage, then let the Workbench config override any of them
		for (int i = 0; i <= RZ_EOutbreakStage.AFTERMATH; i++)
		{
			m_aStages.Insert(RZ_StageSettings.CreateDefault(i));
		}

		if (configured)
		{
			foreach (RZ_StageSettings s : configured)
			{
				if (s)
					m_aStages[s.m_eStage] = s;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	RZ_EOutbreakStage GetStage()
	{
		return m_eStage;
	}

	//------------------------------------------------------------------------------------------------
	float GetStageMinutes()
	{
		return m_fStageMinutes;
	}

	//------------------------------------------------------------------------------------------------
	//! Rough minutes since the outbreak began: the set length of every earlier stage plus
	//! the time in this one. Used for the broadcast's day count, so it needn't be exact.
	float GetTotalMinutes()
	{
		float total = m_fStageMinutes;
		for (int i = 0; i < m_eStage && i < m_aStages.Count(); i++)
		{
			total += m_aStages[i].m_fDurationMinutes;
		}

		return total;
	}

	//------------------------------------------------------------------------------------------------
	RZ_StageSettings GetStageSettings()
	{
		return m_aStages[m_eStage];
	}

	//------------------------------------------------------------------------------------------------
	void Restore(RZ_EOutbreakStage stage, float minutes)
	{
		m_eStage = stage;
		m_fStageMinutes = minutes;
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt)
	{
		m_fStageMinutes += dt / 60;
		SpreadInfection(dt);
		CheckStageAdvance();
	}

	//------------------------------------------------------------------------------------------------
	void SetStage(RZ_EOutbreakStage stage)
	{
		if (stage == m_eStage)
			return;

		m_eStage = stage;
		m_fStageMinutes = 0;
		RZ_Log.Info("Outbreak stage -> " + RZ_StageSettings.GetStageName(stage));
		m_GM.NotifyAll(RZ_StageSettings.GetStageName(stage), RZ_StageSettings.GetStageAnnouncement(stage));
		m_GM.OnStageChanged(stage);
	}

	//------------------------------------------------------------------------------------------------
	float GetAverageInfection()
	{
		array<ref RZ_Settlement> list = m_GM.GetSettlements();
		if (list.IsEmpty())
			return 0;

		float total = 0;
		foreach (RZ_Settlement s : list)
		{
			total += s.m_fInfection;
		}

		return total / list.Count();
	}

	//------------------------------------------------------------------------------------------------
	protected void SpreadInfection(float dt)
	{
		RZ_StageSettings stage = GetStageSettings();
		float mult = m_GM.GetCure().GetSpreadMultiplier();
		float perTick = stage.m_fSpreadPerMinute * (dt / 60) * mult;

		array<ref RZ_Settlement> list = m_GM.GetSettlements();
		array<float> delta = {};
		for (int n = 0; n < list.Count(); n++)
		{
			// Slow background creep everywhere
			delta.Insert(perTick * 0.25);
		}

		foreach (int i, RZ_Settlement source : list)
		{
			if (source.m_fSensorSeconds > 0)
				source.m_fSensorSeconds = source.m_fSensorSeconds - dt;

			if (source.m_fNoSpreadSeconds > 0)
			{
				source.m_fNoSpreadSeconds = source.m_fNoSpreadSeconds - dt;
				continue;
			}

			if (source.m_fInfection < SPREAD_SOURCE_INFECTION)
				continue;

			foreach (int j, RZ_Settlement target : list)
			{
				if (i == j)
					continue;

				if (RZ_Util.DistXZ(source.m_vCenter, target.m_vCenter) > NEIGHBOUR_DISTANCE)
					continue;

				delta[j] = delta[j] + perTick * (source.m_fInfection / 100);
			}
		}

		foreach (int k, RZ_Settlement settlement : list)
		{
			settlement.AddInfection(delta[k]);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckStageAdvance()
	{
		if (m_eStage == RZ_EOutbreakStage.AFTERMATH)
			return;

		RZ_StageSettings stage = GetStageSettings();
		bool timeUp = stage.m_fDurationMinutes > 0 && m_fStageMinutes >= stage.m_fDurationMinutes;

		float avg = GetAverageInfection();
		bool infectionUp = false;
		if (m_eStage == RZ_EOutbreakStage.EARLY_OUTBREAK && avg >= 45)
			infectionUp = true;
		else if (m_eStage == RZ_EOutbreakStage.COLLAPSE && avg >= 65)
			infectionUp = true;
		else if (m_eStage == RZ_EOutbreakStage.OVERRUN && avg >= 85)
			infectionUp = true;

		if (timeUp || infectionUp)
			SetStage(m_eStage + 1);
	}
}
