//! Server-side record of a player. Lifetime stats persist across lives.
//! Per-life values (background, Fame/Infamy/Favor, infection) reset on every new spawn.
//!
//! Saved as plain members by RZ_Persistence, so keep everything here to
//! simple types and arrays of simple types.
class RZ_PlayerProfile
{
	// Persistent
	string m_sIdentity;
	int m_iLives;
	int m_iMissionsCompleted;
	int m_iSamplesDelivered;
	int m_iLegacy;					// personal objectives completed over all lives (meta progression)
	ref array<int> m_aChampionOf = {};	// factions any past or current character reached Champion with
	ref array<string> m_aUnlocked = {};	// unlock ids of backgrounds this player may pick

	// Current life
	string m_sBackgroundId;
	float m_fInfectionLeft = -1;	// seconds until turning, -1 = not infected
	ref array<int> m_aFame = {0, 0, 0, 0};
	ref array<int> m_aInfamy = {0, 0, 0, 0};
	ref array<int> m_aFavor = {0, 0, 0, 0};
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

		EnsureArrays();
		for (int i = 0; i < RZ_Factions.COUNT; i++)
		{
			int fame = bg.GetStartFame(i);
			// Word gets around: a past Champion starts at least Known
			if (m_aChampionOf.Contains(i))
				fame = Math.Max(fame, RZ_Reputation.WORD_GETS_AROUND_FAME);

			m_aFame[i] = fame;
			m_aInfamy[i] = bg.GetStartInfamy(i);
			m_aFavor[i] = 0;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Old saves or a fresh `new` may have short arrays; pad them so indexing is safe.
	void EnsureArrays()
	{
		if (!m_aFame)
			m_aFame = new array<int>();
		if (!m_aInfamy)
			m_aInfamy = new array<int>();
		if (!m_aFavor)
			m_aFavor = new array<int>();
		if (!m_aChampionOf)
			m_aChampionOf = new array<int>();
		if (!m_aUnlocked)
			m_aUnlocked = new array<string>();

		while (m_aFame.Count() < RZ_Factions.COUNT)
		{
			m_aFame.Insert(0);
		}

		while (m_aInfamy.Count() < RZ_Factions.COUNT)
		{
			m_aInfamy.Insert(0);
		}

		while (m_aFavor.Count() < RZ_Factions.COUNT)
		{
			m_aFavor.Insert(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	int GetFame(RZ_EFaction f)
	{
		EnsureArrays();
		return m_aFame[f];
	}

	//------------------------------------------------------------------------------------------------
	int GetInfamy(RZ_EFaction f)
	{
		EnsureArrays();
		return m_aInfamy[f];
	}

	//------------------------------------------------------------------------------------------------
	int GetFavor(RZ_EFaction f)
	{
		EnsureArrays();
		return m_aFavor[f];
	}

	//------------------------------------------------------------------------------------------------
	RZ_ETier GetTier(RZ_EFaction f)
	{
		EnsureArrays();
		return RZ_Reputation.GetTier(m_aFame[f], m_aInfamy[f]);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns how much actually changed after clamping.
	int AddFame(RZ_EFaction f, int amount)
	{
		EnsureArrays();
		int before = m_aFame[f];
		m_aFame[f] = Math.ClampInt(before + amount, 0, RZ_Reputation.FAME_MAX);
		return m_aFame[f] - before;
	}

	//------------------------------------------------------------------------------------------------
	int AddInfamy(RZ_EFaction f, int amount)
	{
		EnsureArrays();
		int before = m_aInfamy[f];
		m_aInfamy[f] = Math.ClampInt(before + amount, 0, RZ_Reputation.INFAMY_MAX);
		return m_aInfamy[f] - before;
	}

	//------------------------------------------------------------------------------------------------
	int AddFavor(RZ_EFaction f, int amount)
	{
		EnsureArrays();
		int before = m_aFavor[f];
		m_aFavor[f] = Math.ClampInt(before + amount, 0, RZ_Reputation.FAVOR_MAX);
		return m_aFavor[f] - before;
	}

	//------------------------------------------------------------------------------------------------
	bool IsChampionOf(RZ_EFaction f)
	{
		EnsureArrays();
		return m_aChampionOf.Contains(f);
	}

	//------------------------------------------------------------------------------------------------
	bool HasUnlocked(string unlockId)
	{
		EnsureArrays();
		return m_aUnlocked.Contains(unlockId);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if this is a new unlock.
	bool Unlock(string unlockId)
	{
		EnsureArrays();
		if (unlockId.IsEmpty() || m_aUnlocked.Contains(unlockId))
			return false;

		m_aUnlocked.Insert(unlockId);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool IsInfected()
	{
		return m_fInfectionLeft >= 0;
	}
}
