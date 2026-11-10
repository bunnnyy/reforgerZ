//! The four factions a player can stand well or badly with. Standing is per
//! life (see RZ_Reputation and docs/REPUTATION.md): a new character starts with
//! their background's standing, not the last one's.
//!
//! Reforger's own factions are global (one hostility table for everyone), so
//! standing is tracked here per player and drives missions, rewards, shops,
//! perks and hunter squads. The AI factions in the world (configured in the
//! FactionManager) are:
//!   SURV   The Haven: players and friendly survivors
//!   MIL    Task Force Bastion: remnant military, holds checkpoints
//!   BAND   The Red Hand: raiders and opportunists
//!   INST   Halcyon: scientists running the cure lab
//!
//! The enum names are kept from the first version; only the display names changed.
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
			case RZ_EFaction.SURVIVORS: return "The Haven";
			case RZ_EFaction.MILITARY: return "Task Force Bastion";
			case RZ_EFaction.BANDITS: return "The Red Hand";
			case RZ_EFaction.INSTITUTE: return "Halcyon";
		}

		return "Unknown";
	}

	//------------------------------------------------------------------------------------------------
	//! Short name for compact lines ("Haven", "Bastion", "Red Hand", "Halcyon").
	static string GetShortName(RZ_EFaction f)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return "Haven";
			case RZ_EFaction.MILITARY: return "Bastion";
			case RZ_EFaction.BANDITS: return "Red Hand";
			case RZ_EFaction.INSTITUTE: return "Halcyon";
		}

		return "Unknown";
	}

	//------------------------------------------------------------------------------------------------
	static bool IsValid(int f)
	{
		return f >= 0 && f < COUNT;
	}
}
