//! Server-side record of a player. Lifetime stats persist across lives.
//! Per-life values (background, reputation, infection) reset on every new spawn.
class RZ_PlayerProfile
{
	// Persistent
	string m_sIdentity;
	int m_iLives;
	int m_iMissionsCompleted;
	int m_iSamplesDelivered;
	int m_iLegacy;					// personal objectives completed over all lives (meta progression)

	// Current life
	string m_sBackgroundId;
	float m_fInfectionLeft = -1;	// seconds until turning, -1 = not infected
	ref array<int> m_aRep = {0, 0, 0, 0};
	float m_fLifeSeconds;
	int m_iLifeSamples;
	int m_iLifeMissions;

	// Personal objective for this life (see RZ_Objectives)
	int m_iObjType = -1;
	int m_iObjTarget = -1;
	int m_iObjGoal;
	bool m_bObjDone;

	//------------------------------------------------------------------------------------------------
	void StartNewLife(RZ_Background bg)
	{
		m_iLives++;
		m_sBackgroundId = bg.m_sId;
		m_fInfectionLeft = -1;
		m_fLifeSeconds = 0;
		m_iLifeSamples = 0;
		m_iLifeMissions = 0;
		m_iObjType = -1;
		m_iObjTarget = -1;
		m_iObjGoal = 0;
		m_bObjDone = false;
		m_aRep.Clear();
		for (int i = 0; i < RZ_Factions.COUNT; i++)
		{
			m_aRep.Insert(bg.GetStartRep(i));
		}
	}

	//------------------------------------------------------------------------------------------------
	int GetRep(RZ_EFaction f)
	{
		if (f >= m_aRep.Count())
			return 0;

		return m_aRep[f];
	}

	//------------------------------------------------------------------------------------------------
	int AddRep(RZ_EFaction f, int amount)
	{
		while (m_aRep.Count() < RZ_Factions.COUNT)
		{
			m_aRep.Insert(0);
		}

		m_aRep[f] = Math.ClampInt(m_aRep[f] + amount, -100, 100);
		return m_aRep[f];
	}

	//------------------------------------------------------------------------------------------------
	bool IsInfected()
	{
		return m_fInfectionLeft >= 0;
	}
}
