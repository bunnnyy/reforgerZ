//! The Director runs the outbreak: it moves the server through the stages and
//! spreads infection from town to town. Players push back by clearing towns
//! and by researching the cure.
//!
//! Its clock only runs while someone is playing, and the season speed scales it
//! (docs/DESIGN.md, "Season pacing"): stage lengths are divided by it, spread multiplied. The
//! built-in stages (RZ_StageSettings.CreateDefault) fit a season of about 45 real days.
class RZ_Director
{
	protected RZ_GameModeComponent m_GM;
	protected ref array<ref RZ_StageSettings> m_aStages = {};
	protected RZ_EOutbreakStage m_eStage = RZ_EOutbreakStage.EARLY_OUTBREAK;
	protected float m_fStageMinutes;
	protected float m_fPlayedMinutes = -1;	// minutes played since the outbreak began (the season day reads it); -1 = not known yet

	protected static const float NEIGHBOUR_DISTANCE = 2500;
	protected ref array<ref array<int>> m_aNeighbours = {};	// per settlement, the others within NEIGHBOUR_DISTANCE
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
	//! the time in this one. The broadcast's day count falls back on it, so it needn't be exact.
	float GetTotalMinutes()
	{
		float total = m_fStageMinutes;
		for (int i = 0; i < m_eStage && i < m_aStages.Count(); i++)
		{
			total += m_aStages[i].m_fDurationMinutes / m_GM.GetSeasonSpeed();
		}

		return total;
	}

	//------------------------------------------------------------------------------------------------
	//! Minutes played since the outbreak began, counted on the tick (only while someone plays). An
	//! older save without the count starts from GetTotalMinutes (docs/DESIGN.md, "Clocks").
	float GetPlayedMinutes()
	{
		if (m_fPlayedMinutes < 0)
			return GetTotalMinutes();

		return m_fPlayedMinutes;
	}

	//------------------------------------------------------------------------------------------------
	//! Played minutes in one season day: the first three stages' set lengths / 30 (/ season speed),
	//! 480 with the built-in stages (about one real day at the 45-day pacing).
	float GetSeasonDayMinutes()
	{
		float total = 0;
		for (int i = 0; i < RZ_EOutbreakStage.AFTERMATH && i < m_aStages.Count(); i++)
		{
			total += m_aStages[i].m_fDurationMinutes;
		}

		if (total <= 0)
			total = 14400;

		return Math.Max(1, total / 30 / m_GM.GetSeasonSpeed());
	}

	//------------------------------------------------------------------------------------------------
	RZ_StageSettings GetStageSettings()
	{
		return m_aStages[m_eStage];
	}

	//------------------------------------------------------------------------------------------------
	void Restore(RZ_EOutbreakStage stage, float minutes, float played = -1)
	{
		m_eStage = Math.ClampInt(stage, RZ_EOutbreakStage.EARLY_OUTBREAK, RZ_EOutbreakStage.AFTERMATH);
		m_fStageMinutes = Math.Max(0, minutes);
		m_fPlayedMinutes = played;
		if (played <= 0)
			m_fPlayedMinutes = -1;
	}

	//------------------------------------------------------------------------------------------------
	//! online = at least one player is playing. An empty server's outbreak stands still.
	void Tick(float dt, bool online)
	{
		if (!online)
			return;

		if (m_fPlayedMinutes < 0)
			m_fPlayedMinutes = GetTotalMinutes();

		m_fStageMinutes += dt / 60;
		m_fPlayedMinutes += dt / 60;
		SpreadInfection(dt);
		CheckStageAdvance();
	}

	//------------------------------------------------------------------------------------------------
	//! announce = false: no line to everyone (the cure moves the stage on with its own news).
	void SetStage(RZ_EOutbreakStage stage, bool announce = true)
	{
		if (stage == m_eStage)
			return;

		m_eStage = stage;
		m_fStageMinutes = 0;
		RZ_Log.Info("Outbreak stage -> " + RZ_StageSettings.GetStageName(stage));
		if (announce)
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
		float perTick = stage.m_fSpreadPerMinute * (dt / 60) * mult * m_GM.GetSeasonSpeed() * RZ_Seasons.Mods().m_fSpreadMult;

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

			array<int> nearby = Neighbours(list, i);
			foreach (int j : nearby)
			{
				delta[j] = delta[j] + perTick * (source.m_fInfection / 100);
			}
		}

		foreach (int k, RZ_Settlement settlement : list)
		{
			settlement.AddInfection(delta[k]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The settlements within NEIGHBOUR_DISTANCE of settlement i. Settlements don't move, so the
	//! lists are built once (again if the list changes size) instead of every tick for every pair.
	protected array<int> Neighbours(array<ref RZ_Settlement> list, int i)
	{
		if (m_aNeighbours.Count() != list.Count())
		{
			m_aNeighbours.Clear();
			foreach (int a, RZ_Settlement here : list)
			{
				array<int> near = {};
				foreach (int b, RZ_Settlement there : list)
				{
					if (a != b && RZ_Util.DistXZ(here.m_vCenter, there.m_vCenter) <= NEIGHBOUR_DISTANCE)
						near.Insert(b);
				}

				m_aNeighbours.Insert(near);
			}
		}

		return m_aNeighbours[i];
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckStageAdvance()
	{
		if (m_eStage == RZ_EOutbreakStage.AFTERMATH)
			return;

		RZ_StageSettings stage = GetStageSettings();
		bool timeUp = stage.m_fDurationMinutes > 0 && m_fStageMinutes >= stage.m_fDurationMinutes / m_GM.GetSeasonSpeed();

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
