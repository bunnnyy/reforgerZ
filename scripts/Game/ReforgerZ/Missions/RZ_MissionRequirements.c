//! One "standing with another faction" condition, e.g. not Hunted by Bastion.
class RZ_StandingRequirement
{
	RZ_EFaction m_eFaction;
	RZ_EStandingCheck m_eCheck;
}

//! When in the day a mission may start (RZ_MissionRequirements.m_eTime). With no game clock
//! (no time and weather manager) every value passes.
enum RZ_ETimeGate
{
	ANY,
	NIGHT,			// 20:30 to 05:00
	DUSK_OR_NIGHT,	// from 18:30 to 05:00
	DAY				// not night (05:00 to 20:30)
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
	//! A leader's personal job: the leader's attitude replaces the tier check (RZ_Leaders.CanWorkFor).
	bool m_bLeaderDecides;

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
	//! Halcyon's work is cure research, so a contract or event Halcyon gives stops once the cure is
	//! out (the default). Set this for Halcyon work that still makes sense after the cure.
	bool m_bAfterCure;
	float m_fMinMilitary = 0;		// Bastion presence (RZ_WorldFacts.BastionPresence), 0..1
	float m_fMaxMilitary = 1;
	int m_iMinPlayers = 1;

	// 5. World facts (docs/DESIGN.md, "World facts"), checked with the world state above
	RZ_ETimeGate m_eTime = RZ_ETimeGate.ANY;
	ref array<string> m_aFlagsSet = {};			// season flags that must be set (HasFlag)
	ref array<string> m_aFlagsUnset = {};		// season flags that must not be set
	ref array<string> m_aNotWhileQuest = {};	// questline keys that mustn't be running
	ref array<string> m_aNotAfterQuest = {};	// questline keys that mustn't have reached an ending

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
	//! Only while this season flag is set (RZ_WorldFacts.HasFlag).
	void NeedFlag(string key)
	{
		m_aFlagsSet.Insert(key);
	}

	//------------------------------------------------------------------------------------------------
	//! Only while this season flag isn't set.
	void NeedNoFlag(string key)
	{
		m_aFlagsUnset.Insert(key);
	}

	//------------------------------------------------------------------------------------------------
	//! Not while this questline runs (its stages tell the same story).
	void NotWhileQuest(string questKey)
	{
		m_aNotWhileQuest.Insert(questKey);
	}

	//------------------------------------------------------------------------------------------------
	//! Not once this questline has reached an ending this season (it settled the story).
	void NotAfterQuest(string questKey)
	{
		m_aNotAfterQuest.Insert(questKey);
	}

	//------------------------------------------------------------------------------------------------
	//! Neither while this questline runs nor after it ended: its premise belongs to it.
	void NotWithQuest(string questKey)
	{
		NotWhileQuest(questKey);
		NotAfterQuest(questKey);
	}

	//------------------------------------------------------------------------------------------------
	//! The world facts part of IsWorldOk: cure, time of day, season flags, questlines.
	bool AreFactsOk(RZ_GameModeComponent gm)
	{
		RZ_WorldFacts facts = gm.GetFacts();
		if (!facts)
			return true;

		if (facts.IsCured() && (m_bNotCured || (m_eGiver == RZ_EFaction.INSTITUTE && !m_bAfterCure)))
			return false;

		if (!IsTimeOk(facts))
			return false;

		foreach (string need : m_aFlagsSet)
		{
			if (!facts.HasFlag(need))
				return false;
		}

		foreach (string unset : m_aFlagsUnset)
		{
			if (facts.HasFlag(unset))
				return false;
		}

		RZ_Questlines quests = gm.GetQuestlines();
		if (quests)
		{
			foreach (string running : m_aNotWhileQuest)
			{
				if (quests.IsRunning(running))
					return false;
			}

			foreach (string ended : m_aNotAfterQuest)
			{
				if (quests.HasEnded(ended))
					return false;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsTimeOk(RZ_WorldFacts facts)
	{
		if (m_eTime == RZ_ETimeGate.ANY || !facts.KnowsTime())
			return true;

		switch (m_eTime)
		{
			case RZ_ETimeGate.NIGHT: return facts.IsNight();
			case RZ_ETimeGate.DUSK_OR_NIGHT: return facts.IsNight() || facts.IsDusk();
			case RZ_ETimeGate.DAY: return !facts.IsNight();
		}

		return true;
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

		float military = gm.GetDirector().GetStageSettings().m_fMilitaryPresence;
		if (military < m_fMinMilitary || military > m_fMaxMilitary)
			return false;

		// The cure (and Halcyon's default), the time of day, season flags and questlines
		return AreFactsOk(gm);
	}

	//------------------------------------------------------------------------------------------------
	//! Player conditions: tier with the giver, standing with others, background, infection.
	bool IsEligible(RZ_GameModeComponent gm, int playerId)
	{
		if (playerId <= 0)
			return false;

		RZ_PlayerProfile profile = gm.GetProfile(playerId);
		RZ_ETier giverTier = profile.GetTier(m_eGiver);

		if (!m_bLeaderDecides)
		{
			if (RZ_Reputation.IsHunted(giverTier))
				return false;

			bool watchedOk = m_bAllowWatched && giverTier == RZ_ETier.WATCHED;
			if (!watchedOk && !RZ_Reputation.IsAtLeast(giverTier, m_eMinTier))
				return false;
		}

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

		switch (m_eTime)
		{
			case RZ_ETimeGate.NIGHT: text += ", at night"; break;
			case RZ_ETimeGate.DUSK_OR_NIGHT: text += ", from dusk"; break;
			case RZ_ETimeGate.DAY: text += ", by day"; break;
		}

		return text;
	}
}
