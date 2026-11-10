//! The four factions a player can stand well or badly with. Standing is per
//! life (see RZ_Reputation and docs/REPUTATION.md): a new character starts with
//! their background's standing, not the last one's.
//!
//! Standing is tracked here per player and drives missions, rewards, shops, perks
//! and hunter squads. The AI factions in the world (configured in the FactionManager) are:
//!   SURV   The Haven: players and friendly survivors (guards, occupants, wanderers)
//!   MIL    Task Force Bastion: remnant military, holds checkpoints
//!   BAND   The Red Hand: raiders and opportunists
//!   INST   Halcyon: scientists running the cure lab
//!
//! Reforger decides hostility per faction, so for AI to shoot one player and not another,
//! each player's character is moved onto a hidden player-side variant of SURV
//! (RZ_Hostility): SURV_H1..SURV_H15, whose number is a bitmask of the AI factions that
//! are hostile to it (bit 0 Haven, 1 Bastion, 2 Red Hand, 3 Halcyon), or SURV_OUTLAW
//! (hostile to all of them) in the fallback mode. The player controller stays on SURV.
//! Compare player faction keys with IsPlayerSide(), never with == "SURV".
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

	// ================================================================ Faction keys and hostility masks
	//! Bit 0 Haven, 1 Bastion, 2 Red Hand, 3 Halcyon: the RZ_EFaction value is the bit number.
	static const int ALL_BITS = 15;
	static const string VARIANT_TAG = "_H";
	static const string OUTLAW_TAG = "_OUTLAW";

	//! Answers from DecideFriendly()
	static const int VERDICT_VANILLA = -1;		// not about a player-side variant: use the configured relation
	static const int VERDICT_HOSTILE = 0;
	static const int VERDICT_FRIENDLY = 1;
	static const int VERDICT_AS_BASE = 2;		// a variant with no bit for this faction: same as base SURV

	// Set on every machine by RZ_GameModeComponent.OnPostInit (Configure). The defaults match
	// docs/WORKBENCH_SETUP.md, so the friendliness check also works before that.
	protected static string s_sPlayerKey = "SURV";
	protected static string s_sBastionKey = "MIL";
	protected static string s_sRedHandKey = "BAND";
	protected static string s_sHalcyonKey = "INST";
	protected static ref map<string, int> s_mMasks;
	protected static Faction s_BaseFaction;

	//------------------------------------------------------------------------------------------------
	//! The keys in the FactionManager: the player faction (the Haven) and the three AI factions.
	static void Configure(string playerKey, string bastionKey, string redHandKey, string halcyonKey)
	{
		if (!playerKey.IsEmpty())
			s_sPlayerKey = playerKey;
		if (!bastionKey.IsEmpty())
			s_sBastionKey = bastionKey;
		if (!redHandKey.IsEmpty())
			s_sRedHandKey = redHandKey;
		if (!halcyonKey.IsEmpty())
			s_sHalcyonKey = halcyonKey;

		s_mMasks = null;
		s_BaseFaction = null;
	}

	//------------------------------------------------------------------------------------------------
	//! The base player faction key ("SURV"): spawn points, the deploy menu, the player controller.
	static string GetPlayerKey()
	{
		return s_sPlayerKey;
	}

	//------------------------------------------------------------------------------------------------
	//! The FactionManager key of a faction's AI. The Haven's AI is on the player faction.
	static string GetAIKey(RZ_EFaction f)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return s_sPlayerKey;
			case RZ_EFaction.MILITARY: return s_sBastionKey;
			case RZ_EFaction.BANDITS: return s_sRedHandKey;
			case RZ_EFaction.INSTITUTE: return s_sHalcyonKey;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	static int Bit(RZ_EFaction f)
	{
		return 1 << f;
	}

	//------------------------------------------------------------------------------------------------
	//! "SURV" for 0, "SURV_H5" for 5 (hostile to the Haven and the Red Hand). Masks are clamped to 0-15.
	static string KeyForMask(int mask)
	{
		if (mask <= 0 || mask > ALL_BITS)
			return s_sPlayerKey;

		return string.Format("%1%2%3", s_sPlayerKey, VARIANT_TAG, mask);
	}

	//------------------------------------------------------------------------------------------------
	//! The fallback faction, hostile to every AI faction ("SURV_OUTLAW").
	static string OutlawKey()
	{
		return s_sPlayerKey + OUTLAW_TAG;
	}

	//------------------------------------------------------------------------------------------------
	//! The reverse of KeyForMask: 0 for SURV, 1-15 for a variant, 15 for SURV_OUTLAW,
	//! -1 for anything that isn't on the player side (MIL, BAND, the zombies...).
	static int DecodeMask(string key)
	{
		if (key.IsEmpty())
			return -1;

		if (!s_mMasks)
			s_mMasks = new map<string, int>();

		int cached;
		if (s_mMasks.Find(key, cached))
			return cached;

		int mask = ParseMask(key);
		s_mMasks.Insert(key, mask);
		return mask;
	}

	//------------------------------------------------------------------------------------------------
	protected static int ParseMask(string key)
	{
		if (key == s_sPlayerKey)
			return 0;

		if (key == OutlawKey())
			return ALL_BITS;

		string prefix = s_sPlayerKey + VARIANT_TAG;
		if (key.Length() <= prefix.Length() || key.IndexOf(prefix) != 0)
			return -1;

		string digits = key.Substring(prefix.Length(), key.Length() - prefix.Length());
		int mask = digits.ToInt();
		if (mask < 1 || mask > ALL_BITS || KeyForMask(mask) != key)
			return -1;

		return mask;
	}

	//------------------------------------------------------------------------------------------------
	//! True for SURV and every hidden variant of it. Use this instead of comparing with "SURV".
	static bool IsPlayerSide(string key)
	{
		return DecodeMask(key) >= 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Which bit an AI faction key stands for: 1 Bastion, 2 Red Hand, 3 Halcyon, -1 for any
	//! other key (the zombies, player-side keys). The Haven's bit is decided on the player side.
	protected static int BitIndexForAIKey(string key)
	{
		if (key == s_sBastionKey)
			return RZ_EFaction.MILITARY;
		if (key == s_sRedHandKey)
			return RZ_EFaction.BANDITS;
		if (key == s_sHalcyonKey)
			return RZ_EFaction.INSTITUTE;

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! The relation between two factions when a player-side variant is involved
	//! (modded SCR_Faction, Factions/RZ_FactionRelations.c):
	//! - neither is a variant (mask 0 or not player-side): VERDICT_VANILLA, the configured relation
	//! - variant and variant: friendly (players stay non-hostile to each other at the AI level)
	//! - variant and base SURV: hostile if the variant has the Haven bit, else friendly
	//! - variant and MIL / BAND / INST: hostile if its bit is set, else the same as base SURV
	//! - variant and anything else (the zombies): the same as base SURV
	static int DecideFriendly(string a, string b)
	{
		int ma = DecodeMask(a);
		int mb = DecodeMask(b);
		if (ma <= 0 && mb <= 0)
			return VERDICT_VANILLA;

		if (ma >= 0 && mb >= 0)
		{
			int variant = ma;
			if (ma == 0)
				variant = mb;
			else if (mb != 0)
				return VERDICT_FRIENDLY;

			if ((variant & Bit(RZ_EFaction.SURVIVORS)) != 0)
				return VERDICT_HOSTILE;

			return VERDICT_FRIENDLY;
		}

		int mask = ma;
		string other = b;
		if (ma < 0)
		{
			mask = mb;
			other = a;
		}

		int bitIdx = BitIndexForAIKey(other);
		if (bitIdx >= 0 && (mask & (1 << bitIdx)) != 0)
			return VERDICT_HOSTILE;

		return VERDICT_AS_BASE;
	}

	//------------------------------------------------------------------------------------------------
	//! The base player faction object, cached. Null without a FactionManager or a SURV entry.
	static Faction GetBaseFaction()
	{
		if (s_BaseFaction)
			return s_BaseFaction;

		FactionManager fm = GetGame().GetFactionManager();
		if (fm)
			s_BaseFaction = fm.GetFactionByKey(s_sPlayerKey);

		return s_BaseFaction;
	}
}
