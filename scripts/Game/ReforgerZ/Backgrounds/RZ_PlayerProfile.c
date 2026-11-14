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
	int m_iOpeningLastScene = -1;		// the last opening's scene (reason x 10 + its composition), so the next one differs
	ref array<string> m_aGuideSeen = {};	// once-only tips already given
	ref array<float> m_aStarts = {};	// openings (Guidance/RZ_Openings.c): the last two starts, x and z, newest first

	// The runner's case (Guidance/RZ_OpeningCase.c): a quest item that stays yours across a death
	int m_iCaseState;					// RZ_OpeningCase.STATE_*: none, carried (the lead), to Halcyon, sold, to Bastion
	int m_iCaseLife = -1;				// the character the state belongs to (RZ_OpeningCase.CharacterStamp)
	int m_iCaseSeason = -1;				// the season this player's case last counted (one a season)
	bool m_bCaseHeld;					// died carrying it: it comes back in the next spawn's pack
	float m_fCaseWarm;					// and how warm it was then (0 cold, 1 warm)
	ref array<int> m_aPapers = {};		// the papers from the case's lid (Guidance/RZ_CasePapers.c): each one's state, three long
	int m_iPapersLife = -1;				// the character they belong to (RZ_OpeningCase.CharacterStamp)

	// The map's fog (World/RZ_MapFog.c): one bit per square seen, kept across deaths (per character
	// once characters last: designs/PERSISTENCE.md); m_sFogGrid is the grid it was kept for
	ref array<int> m_aFog = {};
	string m_sFogGrid;

	// The habit (Backgrounds/RZ_Habits.c): picked at the opening's door, kept for the character
	int m_iHabit;						// RZ_EPlayerHabit
	int m_iHabitLife = -1;				// the character it belongs to (RZ_OpeningCase.CharacterStamp)

	// The fixer (Guidance/RZ_OpeningFixer.c): what this character did when Ferda Kroupa came for the case
	int m_iFerdaWay;					// RZ_EFerdaWay
	int m_iFerdaLife = -1;				// the character it belongs to (RZ_OpeningCase.CharacterStamp)
	int m_iFerdaDay;					// the season day of his visit
	string m_sFerdaTown;				// the street he came to
	int m_iFerdaAfter;					// what came after (RZ_OpeningFixer.AFTER_*: the radio, his note, the toll, his voice)

	// The quest tracker (UI/RZ_Quests.c): the job this player follows, kept across relogs
	int m_iTrackedId;					// its journal row id, 0 = none
	string m_sTrackedTitle;				// its title, to find it again after a restart
	bool m_bTrackOff;					// untracked on purpose: nothing is tracked until a new job is taken

	// Past jobs (the notebook's Journal, Missions/RZ_JobHistory.c): finished, failed and abandoned,
	// oldest first, at most RZ_JobHistory.MAX; one entry is the same index in every array
	ref array<string> m_aPastTitle = {};
	ref array<string> m_aPastBy = {};		// who gave it ("Sergeant Hale", "the Haven")
	ref array<string> m_aPastWhere = {};	// the place's name
	ref array<string> m_aPastLine = {};		// how it ended, one line
	ref array<int> m_aPastKind = {};		// RZ_EMissionKindUI (the row's icon)
	ref array<int> m_aPastGiver = {};		// the faction, -1 none
	ref array<int> m_aPastResult = {};		// RZ_EJobResult
	ref array<int> m_aPastDay = {};			// the season day it ended
	ref array<int> m_aPastSeason = {};		// the season it ended in

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
	int m_iLifeInfected;			// infected put down near this character (UI/RZ_DeathBrief.c)
	float m_fLifeMetres;			// ground covered, on foot or in a car (UI/RZ_DeathBrief.c)
	bool m_bBiteDeclared;		// told a safehouse about this bite (can't be caught hiding it; cleared with the bite)
	bool m_bDeclareSlowed;		// a declared bite was slowed this life (once per life)
	bool m_bBiteCaught;			// caught hiding a bite in a safehouse
	bool m_bDoctorTreated;		// a Doctor wanderer already slowed this life's infection
	bool m_bMedicSlowed;		// a Nurse or Doctor player already slowed this life's infection
	bool m_bGuideCounted;		// this life already counts as experience
	bool m_bBoosted;			// Halcyon's immunity booster: the next bite doesn't take (once)
	int m_iCluesGiven;			// bitmask of factions whose reputation clue (armband, ribbon) this life has had
	ref array<int> m_aAmendsPaid = {0, 0, 0, 0};	// paid amends per faction this life (each costs more)
	ref array<int> m_aAmendsWorked = {0, 0, 0, 0};	// Infamy worked off by paying, per faction this life (capped)

	// Reputation this life (docs/REPUTATION.md, "Reputations"), per faction
	ref array<int> m_aFameGained = {0, 0, 0, 0};	// every positive Fame gain (Earned Fame = the lower of this and Fame)
	ref array<int> m_aInfamyDirect = {0, 0, 0, 0};	// positive Infamy gains, rival shares left out (Feared = the lower of this and Infamy)
	ref array<int> m_aForgiven = {0, 0, 0, 0};		// the forgiveness budget used (Forgive)
	ref array<int> m_aThreats = {0, 0, 0, 0};		// Fear threats used (RZ_Checks.Threaten)
	ref array<int> m_aRepSeen = {-1, -1, -1, -1};	// the reputation cell last told (RZ_ERep), -1 = not yet
	// Borrowed kit this life (Factions/RZ_Disguise.c), per faction: catches, and blown until this
	// many seconds of this life's play
	ref array<int> m_aCatches = {0, 0, 0, 0};
	ref array<float> m_aBlownUntil = {0, 0, 0, 0};
	// The parley and settling up (Factions/RZ_Parley.c), per faction, in server time (a reconnect or
	// a restart resets nothing): the last parley, the real day of the last settling and how much
	ref array<int> m_aParleyUnix = {0, 0, 0, 0};
	ref array<int> m_aSettleDay = {-1, -1, -1, -1};
	ref array<int> m_aSettled = {0, 0, 0, 0};

	// Deeds this life (Core/RZ_Deeds.c, life scope), per faction: the biggest single Fame gain and
	// direct Infamy gain with their phrase and season day, and the tallies
	ref array<string> m_aBestDeed = {"", "", "", ""};
	ref array<int> m_aBestAmount = {0, 0, 0, 0};
	ref array<int> m_aBestDay = {0, 0, 0, 0};
	ref array<string> m_aWorstDeed = {"", "", "", ""};
	ref array<int> m_aWorstAmount = {0, 0, 0, 0};
	ref array<int> m_aWorstDay = {0, 0, 0, 0};
	ref array<int> m_aKills = {0, 0, 0, 0};			// that faction's people killed
	ref array<int> m_aJobs = {0, 0, 0, 0};			// contracts completed for it
	// Life records (what this character did: "pressed", "hid_bite"...): key, season day, and the
	// season day a bad one stays pinned until (0 = not pinned). At most RZ_Deeds.LIFE_RECORDS
	ref array<string> m_aRecKeys = {};
	ref array<float> m_aRecDays = {};
	ref array<float> m_aRecPinned = {};
	// Callback deeds, this life (Core/RZ_Deeds.c, tone's 12 weighted slots): key, phrase, weight
	// (3 heavy, 2 medium, 1 light) and season day
	ref array<string> m_aCallKeys = {};
	ref array<string> m_aCallPhrases = {};
	ref array<int> m_aCallWeights = {};
	ref array<float> m_aCallDays = {};

	// Reputation's voice, this life (Factions/RZ_RepVoice.c)
	string m_sCrewName;							// what the Red Hand's crews call you ("" = new face)
	ref array<string> m_aSetWord = {"", "", "", ""};	// a word the majors or questlines gave, per faction
	int m_iRanDay = -1;							// season day a Bastion checkpoint was last run (Legs)
	int m_iCrewKillDay = -1;					// season day a Red Hand person was last killed (Sorry)
	int m_iBites;								// bites this life (Snack)
	// Tone's joke memory, across lives (Factions/RZ_VoiceBank.c): the last jokes this player heard
	ref array<string> m_aJokeIds = {};
	ref array<int> m_aJokeUnix = {};

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
		m_iLifeInfected = 0;
		m_fLifeMetres = 0;
		m_bBiteDeclared = false;
		m_bDeclareSlowed = false;
		m_bBiteCaught = false;
		m_bDoctorTreated = false;
		m_bMedicSlowed = false;
		m_bGuideCounted = false;
		m_bBoosted = false;
		m_iCluesGiven = 0;
		m_sCrewName = "";
		m_iRanDay = -1;
		m_iCrewKillDay = -1;
		m_iBites = 0;
		m_iObjType = -1;
		m_iObjTarget = -1;
		m_iObjGoal = 0;
		m_bObjDone = false;

		EnsureArrays();
		for (int i = 0; i < RZ_Factions.COUNT; i++)
		{
			int fame = bg.GetStartFame(i);
			int infamy = bg.GetStartInfamy(i);
			// Word gets around: a past Champion starts at least Known (no starting Infamy spoils it)
			if (m_aChampionOf.Contains(i))
			{
				fame = Math.Max(fame, RZ_Reputation.WORD_GETS_AROUND_FAME);
				infamy = Math.Min(infamy, RZ_Reputation.WORD_GETS_AROUND_INFAMY_MAX);
			}

			m_aFame[i] = fame;
			m_aInfamy[i] = infamy;
			m_aFavor[i] = 0;
			m_aAmendsPaid[i] = 0;
			m_aAmendsWorked[i] = 0;
			m_aFameGained[i] = 0;
			m_aInfamyDirect[i] = 0;
			m_aForgiven[i] = 0;
			m_aThreats[i] = 0;
			m_aCatches[i] = 0;
			m_aBlownUntil[i] = 0;
			m_aSettleDay[i] = -1;
			m_aSettled[i] = 0;
			m_aRepSeen[i] = RZ_RepNames.GetRep(fame, infamy);
			m_aBestDeed[i] = "";
			m_aBestAmount[i] = 0;
			m_aBestDay[i] = 0;
			m_aWorstDeed[i] = "";
			m_aWorstAmount[i] = 0;
			m_aWorstDay[i] = 0;
			m_aKills[i] = 0;
			m_aJobs[i] = 0;
			m_aSetWord[i] = "";
		}

		m_aRecKeys.Clear();
		m_aRecDays.Clear();
		m_aRecPinned.Clear();
		m_aCallKeys.Clear();
		m_aCallPhrases.Clear();
		m_aCallWeights.Clear();
		m_aCallDays.Clear();
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
		if (!m_aStarts)
			m_aStarts = new array<float>();
		if (!m_aAmendsPaid)
			m_aAmendsPaid = new array<int>();
		if (!m_aAmendsWorked)
			m_aAmendsWorked = new array<int>();
		while (m_aAmendsPaid.Count() < RZ_Factions.COUNT)
		{
			m_aAmendsPaid.Insert(0);
		}
		while (m_aAmendsWorked.Count() < RZ_Factions.COUNT)
		{
			m_aAmendsWorked.Insert(0);
		}

		// Reputation and deeds (older saves have none: zero, and the cell not told yet)
		if (!m_aFameGained)
			m_aFameGained = new array<int>();
		if (!m_aInfamyDirect)
			m_aInfamyDirect = new array<int>();
		if (!m_aForgiven)
			m_aForgiven = new array<int>();
		if (!m_aThreats)
			m_aThreats = new array<int>();
		if (!m_aRepSeen)
			m_aRepSeen = new array<int>();
		if (!m_aBestDeed)
			m_aBestDeed = new array<string>();
		if (!m_aBestAmount)
			m_aBestAmount = new array<int>();
		if (!m_aBestDay)
			m_aBestDay = new array<int>();
		if (!m_aWorstDeed)
			m_aWorstDeed = new array<string>();
		if (!m_aWorstAmount)
			m_aWorstAmount = new array<int>();
		if (!m_aWorstDay)
			m_aWorstDay = new array<int>();
		if (!m_aKills)
			m_aKills = new array<int>();
		if (!m_aJobs)
			m_aJobs = new array<int>();
		if (!m_aSetWord)
			m_aSetWord = new array<string>();
		while (m_aSetWord.Count() < RZ_Factions.COUNT)
		{
			m_aSetWord.Insert("");
		}
		if (!m_aJokeIds)
			m_aJokeIds = new array<string>();
		if (!m_aJokeUnix)
			m_aJokeUnix = new array<int>();
		if (!m_aRecKeys)
			m_aRecKeys = new array<string>();
		if (!m_aRecDays)
			m_aRecDays = new array<float>();
		if (!m_aRecPinned)
			m_aRecPinned = new array<float>();
		if (!m_aCallKeys)
			m_aCallKeys = new array<string>();
		if (!m_aCallPhrases)
			m_aCallPhrases = new array<string>();
		if (!m_aCallWeights)
			m_aCallWeights = new array<int>();
		if (!m_aCallDays)
			m_aCallDays = new array<float>();

		RZ_JobHistory.Ensure(this);

		while (m_aCallPhrases.Count() < m_aCallKeys.Count())
		{
			m_aCallPhrases.Insert("");
		}

		while (m_aCallWeights.Count() < m_aCallKeys.Count())
		{
			m_aCallWeights.Insert(1);
		}

		while (m_aCallDays.Count() < m_aCallKeys.Count())
		{
			m_aCallDays.Insert(0);
		}

		while (m_aFameGained.Count() < RZ_Factions.COUNT)
		{
			m_aFameGained.Insert(0);
		}

		while (m_aInfamyDirect.Count() < RZ_Factions.COUNT)
		{
			m_aInfamyDirect.Insert(0);
		}

		while (m_aForgiven.Count() < RZ_Factions.COUNT)
		{
			m_aForgiven.Insert(0);
		}

		while (m_aThreats.Count() < RZ_Factions.COUNT)
		{
			m_aThreats.Insert(0);
		}

		if (!m_aCatches)
			m_aCatches = new array<int>();
		if (!m_aBlownUntil)
			m_aBlownUntil = new array<float>();
		if (!m_aParleyUnix)
			m_aParleyUnix = new array<int>();
		if (!m_aSettleDay)
			m_aSettleDay = new array<int>();
		if (!m_aSettled)
			m_aSettled = new array<int>();
		while (m_aCatches.Count() < RZ_Factions.COUNT)
		{
			m_aCatches.Insert(0);
		}
		while (m_aBlownUntil.Count() < RZ_Factions.COUNT)
		{
			m_aBlownUntil.Insert(0);
		}
		while (m_aParleyUnix.Count() < RZ_Factions.COUNT)
		{
			m_aParleyUnix.Insert(0);
		}
		while (m_aSettleDay.Count() < RZ_Factions.COUNT)
		{
			m_aSettleDay.Insert(-1);
		}
		while (m_aSettled.Count() < RZ_Factions.COUNT)
		{
			m_aSettled.Insert(0);
		}

		while (m_aRepSeen.Count() < RZ_Factions.COUNT)
		{
			m_aRepSeen.Insert(-1);
		}

		while (m_aBestDeed.Count() < RZ_Factions.COUNT)
		{
			m_aBestDeed.Insert("");
		}

		while (m_aBestAmount.Count() < RZ_Factions.COUNT)
		{
			m_aBestAmount.Insert(0);
		}

		while (m_aBestDay.Count() < RZ_Factions.COUNT)
		{
			m_aBestDay.Insert(0);
		}

		while (m_aWorstDeed.Count() < RZ_Factions.COUNT)
		{
			m_aWorstDeed.Insert("");
		}

		while (m_aWorstAmount.Count() < RZ_Factions.COUNT)
		{
			m_aWorstAmount.Insert(0);
		}

		while (m_aWorstDay.Count() < RZ_Factions.COUNT)
		{
			m_aWorstDay.Insert(0);
		}

		while (m_aKills.Count() < RZ_Factions.COUNT)
		{
			m_aKills.Insert(0);
		}

		while (m_aJobs.Count() < RZ_Factions.COUNT)
		{
			m_aJobs.Insert(0);
		}

		while (m_aRecDays.Count() < m_aRecKeys.Count())
		{
			m_aRecDays.Insert(0);
		}

		while (m_aRecPinned.Count() < m_aRecKeys.Count())
		{
			m_aRecPinned.Insert(0);
		}

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
	//! Returns how much actually changed after clamping. A gain counts toward Earned Fame.
	int AddFame(RZ_EFaction f, int amount)
	{
		EnsureArrays();
		int before = m_aFame[f];
		m_aFame[f] = Math.ClampInt(before + amount, 0, RZ_Reputation.FAME_MAX);
		int changed = m_aFame[f] - before;
		if (changed > 0)
			m_aFameGained[f] = m_aFameGained[f] + changed;

		return changed;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns how much actually changed after clamping. share = true: a rival share, which never
	//! counts toward Feared.
	int AddInfamy(RZ_EFaction f, int amount, bool share = false)
	{
		EnsureArrays();
		int before = m_aInfamy[f];
		m_aInfamy[f] = Math.ClampInt(before + amount, 0, RZ_Reputation.INFAMY_MAX);
		int changed = m_aInfamy[f] - before;
		if (changed > 0 && !share)
			m_aInfamyDirect[f] = m_aInfamyDirect[f] + changed;

		return changed;
	}

	//------------------------------------------------------------------------------------------------
	//! The lower of current Fame and the Fame gained this life: a background's start and Word gets
	//! around don't count (docs/REPUTATION.md, "Earned this life").
	int EarnedFame(int f)
	{
		EnsureArrays();
		return Math.Min(m_aFame[f], m_aFameGained[f]);
	}

	//------------------------------------------------------------------------------------------------
	//! The lower of current Infamy and the direct Infamy gained this life (no rival shares, no
	//! background start).
	int Feared(int f)
	{
		EnsureArrays();
		return Math.Min(m_aInfamy[f], m_aInfamyDirect[f]);
	}

	//------------------------------------------------------------------------------------------------
	//! Infamy this faction will still forgive this life (Reputation > Forgiveness budget).
	int ForgiveLeft(int f)
	{
		EnsureArrays();
		int budget = RZ_ReputationSettings.Get().m_iForgiveBudget;
		if (budget <= 0)
			return RZ_Reputation.INFAMY_MAX;

		return Math.Max(0, budget - m_aForgiven[f]);
	}

	//------------------------------------------------------------------------------------------------
	int ThreatsUsed(int f)
	{
		EnsureArrays();
		return m_aThreats[f];
	}

	//------------------------------------------------------------------------------------------------
	void UseThreat(int f)
	{
		EnsureArrays();
		m_aThreats[f] = m_aThreats[f] + 1;
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
		m_bBiteDeclared = false;
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
