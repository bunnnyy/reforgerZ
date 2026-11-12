//! Route assets and the 90% lock in missions (docs/DESIGN.md, "Story blocks"): an outcome a job tags
//! by hand gives the one who decided an ally, a proof or standing for one of the finale's routes. The
//! assets themselves live in the finale's store (RZ_FinaleAssets: one per outcome, three per route a
//! season); story records reach it on their own (RZ_StoryKit.Record). After a player has locked to a
//! route, an option whose asset belongs to another route refuses them. Server only.
class RZ_RouteAssets
{
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

		return "You've picked your side. This isn't it.";
	}

	//------------------------------------------------------------------------------------------------
	//! One asset of `kind` (RZ_FinaleAssets.ALLY, PROOF, STANDING) for `route` to this player, from
	//! the job `what`. True if the finale's store took it.
	static bool Add(RZ_GameModeComponent gm, int playerId, int route, string what, int kind)
	{
		RZ_CureFinale finale = gm.GetFinale();
		RZ_PlayerProfile p = gm.GetProfile(playerId);
		if (!finale || !RZ_CureFinale.IsOn() || !finale.GetAssets() || !p || p.m_sIdentity.IsEmpty() || route < 0)
			return false;

		return finale.GetAssets().Add(route, kind, p.m_sIdentity, "job:" + what);
	}
}
