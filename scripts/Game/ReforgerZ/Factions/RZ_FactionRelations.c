//! Per-player AI hostility, faction side (docs/REPUTATION.md, "What makes AI shoot you").
//!
//! Reforger decides who shoots whom per faction. RZ_Hostility moves a player's character
//! onto a hidden copy of SURV whose key says which AI factions want them dead
//! (SURV_H1..SURV_H15, or SURV_OUTLAW). This check decodes the key, so the 16 extra
//! factions need no friend lists of their own in the FactionManager:
//! - two player-side factions are friendly, except a variant with the Haven bit and base
//!   SURV (the Haven's guards, occupants and wanderers, but also players on mask 0)
//! - a variant is hostile to MIL / BAND / INST when their bit is set
//! - everything else (mask 0, the zombies, AI against AI) is the relation configured in
//!   the FactionManager, untouched
//! The decoding itself is RZ_Factions.DecideFriendly.
modded class SCR_Faction
{
	//------------------------------------------------------------------------------------------------
	// DoCheckIfFactionFriendly is the engine event behind Faction.IsFactionFriendly; vanilla
	// notes it "can be called very often, especially by AI".
	// VERIFY: AI perception and target selection pick up a changed answer quickly (no long
	// caching). If they don't, use the Outlaw mode and give SURV_OUTLAW real enemies in the
	// FactionManager (docs/WORKBENCH_SETUP.md).
	override bool DoCheckIfFactionFriendly(Faction faction)
	{
		if (!faction)
			return super.DoCheckIfFactionFriendly(faction);

		string mine = GetFactionKey();
		string theirs = faction.GetFactionKey();
		int verdict = RZ_Factions.DecideFriendly(mine, theirs);
		if (verdict == RZ_Factions.VERDICT_FRIENDLY)
			return true;

		if (verdict == RZ_Factions.VERDICT_HOSTILE)
			return false;

		if (verdict == RZ_Factions.VERDICT_AS_BASE)
		{
			// A variant without this faction's bit behaves exactly like SURV toward it
			Faction surv = RZ_Factions.GetBaseFaction();
			if (surv && surv != this && surv != faction)
			{
				if (RZ_Factions.IsPlayerSide(mine))
					return surv.IsFactionFriendly(faction);

				return super.DoCheckIfFactionFriendly(surv);
			}
		}

		return super.DoCheckIfFactionFriendly(faction);
	}
}
