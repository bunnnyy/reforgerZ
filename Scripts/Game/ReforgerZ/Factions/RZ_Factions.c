//! Who you can be on good or bad terms with. Reputation is per life: a new
//! character starts with their background's standing, not the last one's.
//!
//! Reforger's own factions are global (one hostility table for everyone), so
//! reputation is tracked here per player and drives missions, rewards, and
//! later trader prices and safe-zone access. The AI factions in the world
//! (configured in the FactionManager) are:
//!   SURV   players and friendly survivors
//!   MIL    remnant military, holds checkpoints
//!   BAND   bandits / raiders
//!   INST   the Institute, scientists running the cure lab
enum RZ_EFaction
{
	SURVIVORS,
	MILITARY,
	BANDITS,
	INSTITUTE
}

class RZ_Factions
{
	static const int COUNT = 4;

	//------------------------------------------------------------------------------------------------
	static string GetName(RZ_EFaction f)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return "Survivors";
			case RZ_EFaction.MILITARY: return "Military";
			case RZ_EFaction.BANDITS: return "Bandits";
			case RZ_EFaction.INSTITUTE: return "Institute";
		}

		return "Unknown";
	}

	//------------------------------------------------------------------------------------------------
	static string GetStanding(int rep)
	{
		if (rep >= 60)
			return "Trusted";
		if (rep >= 20)
			return "Friendly";
		if (rep > -20)
			return "Neutral";
		if (rep > -60)
			return "Distrusted";
		return "Hostile";
	}
}
