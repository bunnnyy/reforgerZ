//! You're the One (docs/ERRANDS.md, champions): Champion with the Haven. A man at the gate wants his
//! daughter's name in the council book, written by you. Write it (a line on the board), or say you're
//! no one. Pays nothing.
class RZ_Errand_YoureTheOne : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "youre_the_one";
		m_sTitle = "You're the One";
		m_sFamily = "champion";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("There's a man at our gate who lost his daughter in the first week. There's no grave. He wants her name, {name}, written into the council book, by you, because of what people say you've done for the houses. In your hand, he says. It would mean something to him.",
			"There's a man at the gate asking for you by what people call you. He wants his daughter's name written in the council book, {name}. By you, he says. Only you.");
		Over("He's gone home. He said he'd come back and ask again. He will.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Write the name in the book";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 10;
		w.m_fRadius = 15;
		w.m_sStep = "Write {name} into the council book at {here} (10 s).";
		w.Work(RZ_EWork.BENCH, "Write the name in the book", 6);
		w.m_sInside = "any";
		w.Done("He read it twice, upside down, and said it out loud once. Then he said: there, now she's somewhere. Thank you.");

		w = Way("talk");
		w.m_sLabel = "Say you're no one";
		w.Done("You're no one, you say. That's what the last one said. I'll tell him.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return dealer.GetGM().GetTier(playerId, RZ_EFaction.SURVIVORS) == RZ_ETier.CHAMPION;
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Rng rng = new RZ_Rng(RZ_WorldFacts.Get().GetSeed() + e.m_iHouse * 53 + RZ_SeasonClock.Day());
		e.SetText("name", RZ_FactionVoice.PersonName(rng, RZ_EFaction.SURVIVORS));
		e.m_sSubject = RZ_WorldFacts.Safehouse(e.m_iHouse) + ".book";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex != 0)
			return;

		RZ_Safehouses houses = dealer.GetGM().GetSafehouses();
		if (houses && houses.GetBoard())
			houses.GetBoard().PostUnsigned(e.m_vGiver, e.Fill("{name}, written into the council book by someone the house trusts. Her father keeps coming to look at it."));
	}
}
