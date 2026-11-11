//! They Say You're With Bastion (docs/ERRANDS.md, the lock cards; one per route): you've locked to a
//! route after 90%, other than the Haven's, and a local at this house who knows you stands with the
//! house. Talk it through (nothing unlocks, nothing locks for them), walk them out of the stand's way, or
//! carry one true word from them to your side. Pays nothing.
class RZ_Errand_WithSide : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "with_side";
		m_sTitle = "{title}";
		m_sFamily = "house";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("\"{local} heard. {said}\"",
			"\"{local} wants a word about who you're standing with. {said}\"");
		Over("\"{local} has stopped asking.\"");

		RZ_ErrandWay w = Way("talk");
		w.m_sLabel = "Talk it through";
		w.Done("\"I don't like it. I understand it. Those aren't the same thing.\"");

		w = Way("do");
		w.m_sLabel = "Walk them out of the stand's way";
		w.m_ePlace = RZ_EErrandPlace.NEXT_TOWN;
		w.m_fSeconds = 5;
		w.m_fRadius = 80;
		w.m_sStep = "Walk {local} to {place}, well away from the lab road (5 s there), then come back to {giver}.";
		w.Done("\"Out of the way. As if there's an out of the way. Thank you.\"");

		w = Way("talk");
		w.m_sLabel = "Carry one true word to your side";
		w.Done("\"Tell them we're still here. That's the true word. We're still here.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_CureFinale finale = gm.GetFinale();
		if (!finale || !RZ_CureFinale.IsOn())
			return false;

		int route = finale.RouteOfPlayer(e.m_iAsker);
		if (route < 0 || route == RZ_EFaction.SURVIVORS)
			return false;

		RZ_Local l = RZ_Errand_HandToHand.KnownLocal(dealer, e);
		if (!l)
			return false;

		e.m_iValue = l.id;
		e.SetText("local", l.name);
		e.SetText("title", TitleFor(route));
		e.SetText("said", SaidFor(route));
		e.m_sSubject = "local:" + l.id.ToString() + ".side";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static string TitleFor(int route)
	{
		switch (route)
		{
			case RZ_EFaction.MILITARY: return "They Say You're With Bastion";
			case RZ_EFaction.BANDITS: return "They Say You're With the Red Hand";
			case RZ_EFaction.INSTITUTE: return "They Say You're With Halcyon";
		}

		return "They Say You're Doing It Yourself";
	}

	//------------------------------------------------------------------------------------------------
	static string SaidFor(int route)
	{
		switch (route)
		{
			case RZ_EFaction.MILITARY: return "\"They say you're with the soldiers now. For the cure. Are you?\"";
			case RZ_EFaction.BANDITS: return "\"They say you're with the crews. That they'll sell it, and you'll help. Tell me that's wrong.\"";
			case RZ_EFaction.INSTITUTE: return "\"They say you're with the lab. The ones who made it in the first place. Is that true?\"";
		}

		return "\"You're doing it yourself? Then do it properly.\"";
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_Local l = dealer.GetLocals().Get(e.m_iValue);
		if (l)
			dealer.GetLocals().Remember(l, playerId, "talked");

		if (w.m_iIndex == 2)
		{
			array<int> who = {playerId};
			RZ_WorldFacts.Get().Memory().Record("errand.true_word", e.GetText("local"), "carried", RZ_WorldMemory.SOURCE_ERRAND, true, who);
		}
	}
}
