//! Standing with one faction, worked out from Fame and Infamy (docs/REPUTATION.md).
//!
//!   Fame \ Infamy   < 15       15-39       40-74      75+
//!   < 15            Stranger   Watched     Hunted     Kill on Sight
//!   15-39           Known      Uneasy      Hunted     Kill on Sight
//!   40-74           Trusted    Tolerated   Wildcard   Hunted
//!   75+             Champion   Respected   Wildcard   Wildcard
enum RZ_ETier
{
	STRANGER,
	KNOWN,
	TRUSTED,
	CHAMPION,
	WATCHED,
	HUNTED,
	KILL_ON_SIGHT,
	UNEASY,
	TOLERATED,
	RESPECTED,
	WILDCARD
}

//! What a mission or shop can ask of a player's standing with one faction.
enum RZ_EStandingCheck
{
	NOT_HUNTED,				// anything except Hunted / Kill on Sight
	AT_LEAST_WATCHED,		// Watched, Hunted or Kill on Sight
	AT_LEAST_HUNTED,		// Hunted or Kill on Sight
	NOT_AT_LEAST_TRUSTED,	// below Trusted (Stranger, Known, Watched, Uneasy...)
	AT_LEAST_KNOWN,			// Known or better (mixed tiers count, see RZ_Reputation.GetAccessRank)
	AT_LEAST_TRUSTED
}

class RZ_Reputation
{
	static const int FAME_MAX = 100;
	static const int INFAMY_MAX = 100;
	static const int FAVOR_MAX = 9999;

	//! Fame given with every faction a past character was Champion with ("word gets around").
	static const int WORD_GETS_AROUND_FAME = 15;

	//------------------------------------------------------------------------------------------------
	//! 0 = low (< 15), 1 = some (15-39), 2 = high (40-74), 3 = extreme (75+)
	static int Band(int value)
	{
		if (value >= 75)
			return 3;
		if (value >= 40)
			return 2;
		if (value >= 15)
			return 1;
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_ETier GetTier(int fame, int infamy)
	{
		int f = Band(fame);
		int i = Band(infamy);

		if (f == 0)
		{
			if (i == 0) return RZ_ETier.STRANGER;
			if (i == 1) return RZ_ETier.WATCHED;
			if (i == 2) return RZ_ETier.HUNTED;
			return RZ_ETier.KILL_ON_SIGHT;
		}

		if (f == 1)
		{
			if (i == 0) return RZ_ETier.KNOWN;
			if (i == 1) return RZ_ETier.UNEASY;
			if (i == 2) return RZ_ETier.HUNTED;
			return RZ_ETier.KILL_ON_SIGHT;
		}

		if (f == 2)
		{
			if (i == 0) return RZ_ETier.TRUSTED;
			if (i == 1) return RZ_ETier.TOLERATED;
			if (i == 2) return RZ_ETier.WILDCARD;
			return RZ_ETier.HUNTED;
		}

		if (i == 0) return RZ_ETier.CHAMPION;
		if (i == 1) return RZ_ETier.RESPECTED;
		return RZ_ETier.WILDCARD;
	}

	//------------------------------------------------------------------------------------------------
	//! How much a tier opens up, on the positive scale Stranger 0, Known 1, Trusted 2, Champion 3.
	//! Mixed tiers give partial access (one step below their Fame band), negative tiers are below 0.
	static int GetAccessRank(RZ_ETier tier)
	{
		switch (tier)
		{
			case RZ_ETier.STRANGER: return 0;
			case RZ_ETier.KNOWN: return 1;
			case RZ_ETier.TRUSTED: return 2;
			case RZ_ETier.CHAMPION: return 3;
			case RZ_ETier.UNEASY: return 0;
			case RZ_ETier.TOLERATED: return 1;
			case RZ_ETier.RESPECTED: return 2;
			case RZ_ETier.WILDCARD: return 1;
			case RZ_ETier.WATCHED: return -1;
			case RZ_ETier.HUNTED: return -2;
			case RZ_ETier.KILL_ON_SIGHT: return -3;
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! True if `tier` gives at least the access of the positive tier `required` (Stranger/Known/Trusted/Champion).
	static bool IsAtLeast(RZ_ETier tier, RZ_ETier required)
	{
		return GetAccessRank(tier) >= GetAccessRank(required);
	}

	//------------------------------------------------------------------------------------------------
	static bool IsHunted(RZ_ETier tier)
	{
		return tier == RZ_ETier.HUNTED || tier == RZ_ETier.KILL_ON_SIGHT;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsWatchedOrWorse(RZ_ETier tier)
	{
		return tier == RZ_ETier.WATCHED || IsHunted(tier);
	}

	//------------------------------------------------------------------------------------------------
	static bool Check(RZ_ETier tier, RZ_EStandingCheck check)
	{
		switch (check)
		{
			case RZ_EStandingCheck.NOT_HUNTED: return !IsHunted(tier);
			case RZ_EStandingCheck.AT_LEAST_WATCHED: return IsWatchedOrWorse(tier);
			case RZ_EStandingCheck.AT_LEAST_HUNTED: return IsHunted(tier);
			case RZ_EStandingCheck.NOT_AT_LEAST_TRUSTED: return !IsAtLeast(tier, RZ_ETier.TRUSTED);
			case RZ_EStandingCheck.AT_LEAST_KNOWN: return IsAtLeast(tier, RZ_ETier.KNOWN);
			case RZ_EStandingCheck.AT_LEAST_TRUSTED: return IsAtLeast(tier, RZ_ETier.TRUSTED);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static string GetCheckText(RZ_EStandingCheck check, RZ_EFaction f)
	{
		string name = RZ_Factions.GetName(f);
		switch (check)
		{
			case RZ_EStandingCheck.NOT_HUNTED: return "not Hunted by " + name;
			case RZ_EStandingCheck.AT_LEAST_WATCHED: return "Watched or worse by " + name;
			case RZ_EStandingCheck.AT_LEAST_HUNTED: return "Hunted by " + name;
			case RZ_EStandingCheck.NOT_AT_LEAST_TRUSTED: return "not Trusted by " + name;
			case RZ_EStandingCheck.AT_LEAST_KNOWN: return "Known by " + name;
			case RZ_EStandingCheck.AT_LEAST_TRUSTED: return "Trusted by " + name;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	static string GetTierName(RZ_ETier tier)
	{
		switch (tier)
		{
			case RZ_ETier.STRANGER: return "Stranger";
			case RZ_ETier.KNOWN: return "Known";
			case RZ_ETier.TRUSTED: return "Trusted";
			case RZ_ETier.CHAMPION: return "Champion";
			case RZ_ETier.WATCHED: return "Watched";
			case RZ_ETier.HUNTED: return "Hunted";
			case RZ_ETier.KILL_ON_SIGHT: return "Kill on Sight";
			case RZ_ETier.UNEASY: return "Uneasy";
			case RZ_ETier.TOLERATED: return "Tolerated";
			case RZ_ETier.RESPECTED: return "Respected";
			case RZ_ETier.WILDCARD: return "Wildcard";
		}

		return "Unknown";
	}

	//------------------------------------------------------------------------------------------------
	//! Share of Fame gained with `gain` that is added as Infamy with `other`.
	//! Kept modest so a few jobs for one side don't lock you out of the other.
	static float GetRivalShare(RZ_EFaction gain, RZ_EFaction other)
	{
		if (gain == other)
			return 0;

		switch (gain)
		{
			case RZ_EFaction.SURVIVORS:
			{
				if (other == RZ_EFaction.BANDITS)
					return 0.15;
				break;
			}
			case RZ_EFaction.MILITARY:
			{
				if (other == RZ_EFaction.BANDITS)
					return 0.25;
				break;
			}
			case RZ_EFaction.BANDITS:
			{
				if (other == RZ_EFaction.MILITARY)
					return 0.2;
				if (other == RZ_EFaction.SURVIVORS)
					return 0.15;
				break;
			}
			case RZ_EFaction.INSTITUTE:
			{
				if (other == RZ_EFaction.BANDITS)
					return 0.1;
				break;
			}
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The background unlocked by reaching Champion with a faction.
	static string GetChampionUnlock(RZ_EFaction f)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return "elder";
			case RZ_EFaction.MILITARY: return "soldier";
			case RZ_EFaction.BANDITS: return "raider";
			case RZ_EFaction.INSTITUTE: return "doctor";
		}

		return "";
	}
}
