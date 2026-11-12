//------------------------------------------------------------------------------------------------
//! A player's name in public text (docs/EPILOGUE.md, "Names"): every broadcast and
//! every name a system stores to show other players later goes through here. The name is left out
//! ("a survivor") when the player chose /anonymous (the hall of fame's list), chose /mydead hidden
//! (Infected > Tribes' opt-out), or the name filter (Tribes > Name filter) catches it. A live
//! bounty's notice and its contract name the hunted on purpose and don't come here. Server only.
class RZ_Names
{
	static const string NOBODY = "a survivor";

	//------------------------------------------------------------------------------------------------
	//! The player's name as public text may show it: the name, NOBODY when it's kept out, "" for
	//! no such player.
	static string Public(int playerId)
	{
		string name = RZ_Util.PlayerName(playerId);
		if (name.IsEmpty())
			return "";

		if (!Allowed(playerId, name))
			return NOBODY;

		return name;
	}

	//------------------------------------------------------------------------------------------------
	//! This player's name may be shown in public text.
	static bool Allowed(int playerId, string name)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || playerId <= 0)
			return true;

		RZ_PlayerProfile p = gm.GetProfile(playerId);
		if (!p)
			return true;

		RZ_HallOfFame hall = gm.GetHall();
		if (hall && hall.IsAnonymous(p.m_sIdentity))
			return false;

		RZ_Tribes tribes = gm.GetTribes();
		if (tribes && tribes.GetPeople())
		{
			if (tribes.GetPeople().IsHidden(p.m_sIdentity) || tribes.GetPeople().Filtered(name))
				return false;
		}

		return true;
	}
}
