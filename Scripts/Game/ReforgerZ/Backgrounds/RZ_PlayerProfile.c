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

	// New-player guidance (RZ_Guidance): experience and what they've already been told
	int m_iGuideLives;					// lives that lasted long enough to count (10 min by default)
	int m_iGuideLastUnix;				// real-world time last seen playing (refresher after a long break)
	int m_iGuideRefreshLeft;			// counted lives left with refreshed (stronger) guidance
	int m_iGuideLastScenario = -1;		// the last spawn scenario, so the next one differs
	ref array<string> m_aGuideSeen = {};	// once-only tips already given

	// Remembered (Haven Champion perk): a Haven Champion died; the next character is kin
	bool m_bFallenPending;			// set at the Champion's death, used up by the next life
	string m_sFallenPlace;			// where they fell (for the greeting)

	// Current life
	string m_sBackgroundId;
	float m_fInfectionLeft = -1;	// seconds until turning, -1 = not infected
	float m_fInfectionTotal;		// seconds from the bite to turning, as set at the bite (stages)
	float m_fWoundLeft = -1;		// golden window: seconds before the bite takes hold (> 0 = fresh)
	bool m_bWoundCleaned;			// the one cleaning of this bite was tried
	float m_fPauseLeft;				// stabiliser or vet antiviral: seconds the timer stands still
	int m_iInfectionStage;			// RZ_EInfectionStage
	int m_iMedsUsed;				// bitmask of RZ_EMed taken this life (each works once per life)
	float m_fMedExtension;			// seconds low-grade meds have added (capped)
	int m_iSideEffect = -1;			// RZ_EMed whose side effect is running, -1 = none
	float m_fSideEffectLeft;
	float m_fSymptomsHiddenLeft;	// fever suppressant: the symptoms don't show
	bool m_bTrialDoseTaken;			// Halcyon's trial dose, once per life
	bool m_bFallenKin;				// Remembered: this life is kin to a fallen Haven Champion
	bool m_bKinGreeted;				// ...and has had its free treatment at a safehouse
	ref array<int> m_aFame = {0, 0, 0, 0};
	ref array<int> m_aInfamy = {0, 0, 0, 0};
	ref array<int> m_aFavor = {0, 0, 0, 0};
	float m_fLifeSeconds;
	int m_iLifeSamples;
	int m_iLifeMissions;
	bool m_bBiteDeclared;		// told a safehouse about the bite (slowed once, can't be caught)
	bool m_bBiteCaught;			// caught hiding a bite in a safehouse
	bool m_bDoctorTreated;		// a Doctor wanderer already slowed this life's infection
	bool m_bMedicSlowed;		// a Nurse or Doctor player already slowed this life's infection
	bool m_bGuideCounted;		// this life already counts as experience

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
		ClearInfection();
		m_iMedsUsed = 0;
		m_fMedExtension = 0;
		m_iSideEffect = -1;
		m_fSideEffectLeft = 0;
		m_fSymptomsHiddenLeft = 0;
		m_bTrialDoseTaken = false;
		m_bFallenKin = m_bFallenPending;
		m_bFallenPending = false;
		m_bKinGreeted = false;
		m_fLifeSeconds = 0;
		m_iLifeSamples = 0;
		m_iLifeMissions = 0;
		m_bBiteDeclared = false;
		m_bBiteCaught = false;
		m_bDoctorTreated = false;
		m_bMedicSlowed = false;
		m_bGuideCounted = false;
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
	//! Never spawned and holds nothing that lasts (a junk entry for a player id that came and went).
	bool IsBlank()
	{
		EnsureArrays();
		return m_iLives == 0 && m_iLegacy == 0 && m_iGuideLives == 0 && m_iMissionsCompleted == 0 && m_iSamplesDelivered == 0 && m_aUnlocked.IsEmpty() && m_aChampionOf.IsEmpty();
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
		if (!m_aGuideSeen)
			m_aGuideSeen = new array<string>();

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

	//------------------------------------------------------------------------------------------------
	//! Bitten, but still inside the golden window: cleaning the wound can stop it.
	bool IsWoundFresh()
	{
		return IsInfected() && m_fWoundLeft > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The bite has taken hold (the stages run).
	bool HasInfectionTaken()
	{
		return IsInfected() && m_fWoundLeft <= 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Per-bite state back to "not infected" (cured, turned or a new life). Med use, side effects
	//! and the trial dose belong to the life and stay.
	void ClearInfection()
	{
		m_fInfectionLeft = -1;
		m_fInfectionTotal = 0;
		m_fWoundLeft = -1;
		m_bWoundCleaned = false;
		m_fPauseLeft = 0;
		m_iInfectionStage = 0;
	}

	//------------------------------------------------------------------------------------------------
	bool HasUsedMed(int kind)
	{
		return (m_iMedsUsed & (1 << kind)) != 0;
	}

	//------------------------------------------------------------------------------------------------
	void MarkMedUsed(int kind)
	{
		m_iMedsUsed = m_iMedsUsed | (1 << kind);
	}
}
