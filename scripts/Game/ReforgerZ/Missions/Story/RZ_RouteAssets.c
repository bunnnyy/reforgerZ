//! Route assets and the 90% lock in missions (docs/DESIGN.md, "Story blocks"): a few outcomes give
//! the one who decided an ally, a proof or standing for one of the finale's routes. An asset only
//! tilts a route and never closes one; it does nothing unless they lock to that route. Three a
//! season per identity, kept in the season flags (`asset.<ident>.<route>`, `asset.<ident>`). After a
//! player has locked to a route, an option whose asset belongs to another route refuses them.
//! Server only.
class RZ_RouteAssets
{
	static const int MAX_SEASON = 3;

	//------------------------------------------------------------------------------------------------
	//! "" if this player may take an option whose asset belongs to `route`; else the refusal.
	static string Refuse(RZ_GameModeComponent gm, int playerId, int route)
	{
		if (route < 0 || !RZ_CureFinale.IsOn())
			return "";

		RZ_CureFinale finale = gm.GetFinale();
		if (!finale)
			return "";

		int mine = finale.RouteOfPlayer(playerId);
		if (mine == RZ_CureFinale.NONE || mine == route)
			return "";

		return "\"You've picked your side. This isn't it.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! One asset for `route` to this player, if they have fewer than three this season. True if added.
	static bool Add(RZ_GameModeComponent gm, int playerId, int route, string what)
	{
		RZ_WorldFacts facts = gm.GetFacts();
		RZ_PlayerProfile p = gm.GetProfile(playerId);
		if (!facts || !p || p.m_sIdentity.IsEmpty() || route < 0)
			return false;

		string all = "asset." + p.m_sIdentity;
		if (facts.Flag(all) >= MAX_SEASON)
			return false;

		string key = string.Format("asset.%1.%2", p.m_sIdentity, route);
		facts.SetFlag(key, facts.Flag(key) + 1);
		facts.SetFlag(all, facts.Flag(all) + 1);
		RZ_Log.Info(string.Format("Story: route asset %1 for %2 to %3 (%4 of %5)", what, RZ_CureFinale.RouteName(route), RZ_Util.PlayerName(playerId), facts.Flag(all), MAX_SEASON));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! How many assets this identity holds for `route` this season.
	static int Count(string ident, int route)
	{
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		if (!facts)
			return 0;

		return facts.Flag(string.Format("asset.%1.%2", ident, route));
	}
}
