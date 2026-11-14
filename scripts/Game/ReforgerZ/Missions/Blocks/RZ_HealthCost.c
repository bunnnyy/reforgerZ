//! Health-cost block (docs/DESIGN.md, "Mission building blocks"): an action that costs the
//! player a share of their health (the Blood Drive). Reads health with the same accessor as the
//! bite detector (RZ_Util.GetCharacterHealth) against the default hit zone's maximum, and never
//! takes anyone below the floor: it refuses instead. The health comes back the vanilla way.
//! Every health cost tells the bite detector to forget the old reading (ForgetBiteHealth), so
//! the drop never counts as a bite. Any new health-taking action must go through here.
//! Once per character per key (a giver remembers who already gave), so it can't be farmed.
class RZ_HealthCost
{
	protected static ref map<string, int> s_mGiven;	// "key|identity" -> the character it was given in

	//------------------------------------------------------------------------------------------------
	//! Health as a share of full health (0..1), -1 if it can't be read.
	static float Share(IEntity character)
	{
		float health = RZ_Util.GetCharacterHealth(character);
		float full = RZ_Util.GetCharacterMaxHealth(character);
		if (health < 0 || full <= 0)
			return -1;

		return Math.Clamp(health / full, 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Already paid as this character for `key`?
	static bool AlreadyGiven(RZ_GameModeComponent gm, int playerId, string key)
	{
		if (!s_mGiven)
			return false;

		RZ_PlayerProfile profile = gm.GetProfile(playerId);
		int life;
		if (!s_mGiven.Find(key + "|" + profile.m_sIdentity, life))
			return false;

		return life == profile.m_iCharacter;
	}

	//------------------------------------------------------------------------------------------------
	//! Takes `cost` (a share of full health) if the player has at least `floor` now. Returns ""
	//! on success, else why not (for the player).
	static string Take(RZ_GameModeComponent gm, int playerId, IEntity character, string key, float cost, float floor)
	{
		if (AlreadyGiven(gm, playerId, key))
			return "You've already given. Once is all they'll take from one person.";

		float share = Share(character);
		if (share < 0)
			return "They can't find a vein. (Health can't be read on this character.)";

		if (share < floor)
			return string.Format("They take one look at you and shake their heads. Come back when you're stronger (at least %1 percent).", RZ_Util.RoundToInt(floor * 100));

		float full = RZ_Util.GetCharacterMaxHealth(character);
		if (!RZ_Util.SetCharacterHealth(character, (share - cost) * full))
			return "Something's wrong with the kit. Not today.";

		// The mode took this health itself: the bite detector must not read the drop as a bite
		gm.ForgetBiteHealth(playerId);

		if (!s_mGiven)
			s_mGiven = new map<string, int>();

		RZ_PlayerProfile profile = gm.GetProfile(playerId);
		s_mGiven.Set(key + "|" + profile.m_sIdentity, profile.m_iCharacter);
		return "";
	}
}
