//! Hand to Hand (docs/ERRANDS.md, the Independent's margin card): research is at the gate (the finale's
//! routes are open), you haven't locked to a route, and a local at this house knows you. They ask you,
//! not a faction, to bring theirs when it's made. Promise, or don't. A promise moves no track. At most
//! six a player a season (`errand.hands.<identity>`).
class RZ_Errand_HandToHand : RZ_ErrandCard
{
	static const int MAX_PROMISES = 6;

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "hand_to_hand";
		m_sTitle = "Hand to Hand";
		m_sFamily = "house";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("{local} wants a word, and not with the council: with you. The cure's close now, and when it's made the factions will hand it out by their own rules. {local} wants to know if you'll bring this house's doses. You. Not them.",
			"{local} asked for you by name. When the cure's made, Bastion will have a roster, the crews a book and the lab a list, and {local} doesn't trust any of them to reach this house. {local} trusts you, apparently. Will you bring ours?");
		Over("{local} doesn't need to ask you any more.");

		RZ_ErrandWay w = Way("talk");
		w.m_sLabel = "Promise";
		w.Done("You meant that. I watched you mean it. I'll tell {local}, and we'll hold you to it.");

		w = Way("talk");
		w.m_sLabel = "Don't promise";
		w.Done("No. Fair. Nobody should promise anything, this close to the end.");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_CureFinale finale = gm.GetFinale();
		if (!finale || !finale.IsOpen() || finale.RouteOfPlayer(e.m_iAsker) != RZ_CureFinale.NONE)
			return false;

		RZ_PlayerProfile p = gm.GetProfile(e.m_iAsker);
		if (!p || gm.GetFacts().Flag("errand.hands." + p.m_sIdentity) >= MAX_PROMISES)
			return false;

		RZ_Local l = KnownLocal(dealer, e);
		if (!l)
			return false;

		e.m_iValue = l.id;
		e.SetText("local", l.name);
		e.m_sSubject = "local:" + l.id.ToString() + ".hands";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A living local at this house who knows the asker; null if none.
	static RZ_Local KnownLocal(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Locals locals = dealer.GetLocals();
		RZ_PlayerProfile p = dealer.GetGM().GetProfile(e.m_iAsker);
		if (!locals || !p)
			return null;

		array<RZ_Local> here = {};
		locals.AtHouse(e.m_iHouse, here);
		foreach (RZ_Local l : here)
		{
			if (l.alive && l.idents.Contains(p.m_sIdentity))
				return l;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex != 0)
			return;

		RZ_GameModeComponent gm = dealer.GetGM();
		string key = "errand.hands." + gm.GetProfile(playerId).m_sIdentity;
		gm.GetFacts().SetFlag(key, gm.GetFacts().Flag(key) + 1);
		RZ_Local l = dealer.GetLocals().Get(e.m_iValue);
		if (l)
		{
			l.pinned = true;
			dealer.GetLocals().Remember(l, playerId, "promised");
		}

		array<int> who = {playerId};
		RZ_WorldFacts.Get().Memory().Record("errand.hands", e.GetText("local"), "promised", RZ_WorldMemory.SOURCE_ERRAND, true, who,
			e.Fill("{local} still waits at the house, hands in their lap, for whoever promised."), "voice");
	}
}
