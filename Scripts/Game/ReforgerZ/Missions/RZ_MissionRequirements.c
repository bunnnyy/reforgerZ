//! One "standing with another faction" condition, e.g. not Hunted by Bastion.
class RZ_StandingRequirement
{
	RZ_EFaction m_eFaction;
	RZ_EStandingCheck m_eCheck;
}

//! Who a faction contract is offered to (docs/MISSIONS.md, "Who gets offered what").
//! Everything is checked on the server. World conditions (stage, cure, player
//! count) decide whether the contract appears at all; player conditions decide
//! who gets offered it and who gets paid.
class RZ_MissionRequirements
{
	RZ_EFaction m_eGiver = RZ_EFaction.SURVIVORS;

	// 1. Tier with the giver. "Any" = Stranger or better and not Hunted.
	RZ_ETier m_eMinTier = RZ_ETier.STRANGER;
	//! Amends contracts can also be taken while Watched by the giver.
	bool m_bAllowWatched;

	// 2. Standing with other factions
	ref array<ref RZ_StandingRequirement> m_aChecks = {};

	// 3. Character type
	ref array<string> m_aAllowedBackgrounds = {};	// empty = every background
	ref array<string> m_aDeniedBackgrounds = {};
	bool m_bDeniedUnlessChampion;					// denied backgrounds may still take it as a Champion of the giver
	ref array<string> m_aBonusBackgrounds = {};		// ★ backgrounds: the mission does something better for them
	int m_iInfection;								// 0 = either, 1 = must be infected, -1 = must not be

	// 4. World state
	int m_iMinStage = RZ_EOutbreakStage.EARLY_OUTBREAK;
	int m_iMaxStage = RZ_EOutbreakStage.AFTERMATH;
	float m_fMinCure = 0;
	bool m_bNotCured;
	float m_fMinMilitary = 0;		// stage military presence, 0..1
	float m_fMaxMilitary = 1;
	int m_iMinPlayers = 1;

	//------------------------------------------------------------------------------------------------
	void Need(RZ_EFaction faction, RZ_EStandingCheck check)
	{
		RZ_StandingRequirement r = new RZ_StandingRequirement();
		r.m_eFaction = faction;
		r.m_eCheck = check;
		m_aChecks.Insert(r);
	}

	//------------------------------------------------------------------------------------------------
	void Deny(string backgroundId)
	{
		m_aDeniedBackgrounds.Insert(backgroundId);
	}

	//------------------------------------------------------------------------------------------------
	void Allow(string backgroundId)
	{
		m_aAllowedBackgrounds.Insert(backgroundId);
	}

	//------------------------------------------------------------------------------------------------
	void Bonus(string backgroundId)
	{
		m_aBonusBackgrounds.Insert(backgroundId);
	}

	//------------------------------------------------------------------------------------------------
	bool IsBonus(string backgroundId)
	{
		return m_aBonusBackgrounds.Contains(backgroundId);
	}

	//------------------------------------------------------------------------------------------------
	//! World conditions only. Checked before a contract is created.
	bool IsWorldOk(RZ_GameModeComponent gm, int playerCount)
	{
		if (playerCount < m_iMinPlayers)
			return false;

		int stage = gm.GetDirector().GetStage();
		if (stage < m_iMinStage || stage > m_iMaxStage)
			return false;

		RZ_CureResearch cure = gm.GetCure();
		if (cure.m_fProgress < m_fMinCure)
			return false;

		if (m_bNotCured && cure.IsCured())
			return false;

		float military = gm.GetDirector().GetStageSettings().m_fMilitaryPresence;
		if (military < m_fMinMilitary || military > m_fMaxMilitary)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Player conditions: tier with the giver, standing with others, background, infection.
	bool IsEligible(RZ_GameModeComponent gm, int playerId)
	{
		if (playerId <= 0)
			return false;

		RZ_PlayerProfile profile = gm.GetProfile(playerId);
		RZ_ETier giverTier = profile.GetTier(m_eGiver);

		if (RZ_Reputation.IsHunted(giverTier))
			return false;

		bool watchedOk = m_bAllowWatched && giverTier == RZ_ETier.WATCHED;
		if (!watchedOk && !RZ_Reputation.IsAtLeast(giverTier, m_eMinTier))
			return false;

		foreach (RZ_StandingRequirement r : m_aChecks)
		{
			if (!RZ_Reputation.Check(profile.GetTier(r.m_eFaction), r.m_eCheck))
				return false;
		}

		string bgId = profile.m_sBackgroundId;
		if (!m_aAllowedBackgrounds.IsEmpty() && !m_aAllowedBackgrounds.Contains(bgId))
			return false;

		if (m_aDeniedBackgrounds.Contains(bgId))
		{
			if (!m_bDeniedUnlessChampion || giverTier != RZ_ETier.CHAMPION)
				return false;
		}

		if (m_iInfection > 0 && !profile.IsInfected())
			return false;

		if (m_iInfection < 0 && profile.IsInfected())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! "Trusted with Task Force Bastion, not Trusted by The Red Hand"
	string Describe()
	{
		string text;
		if (m_eMinTier == RZ_ETier.STRANGER)
			text = "Open to anyone not Hunted by " + RZ_Factions.GetName(m_eGiver);
		else
			text = RZ_Reputation.GetTierName(m_eMinTier) + " with " + RZ_Factions.GetName(m_eGiver);

		foreach (RZ_StandingRequirement r : m_aChecks)
		{
			text += ", " + RZ_Reputation.GetCheckText(r.m_eCheck, r.m_eFaction);
		}

		if (m_iMinPlayers > 1)
			text += string.Format(", %1+ players online", m_iMinPlayers);

		return text;
	}
}
