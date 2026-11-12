//! Unknown Face (docs/ERRANDS.md, help across lines; reputation's "Who's Asking", renamed): a guard,
//! for a Stranger with the giver's faction. A letter carried into a rival's camp within 1.5 km,
//! because nobody there knows your face. Deliver it; read and swap it (a lie, found by events); or
//! sell it to the camp (seen by the giver's people, it costs you with them).
class RZ_Errand_UnknownFace : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "unknown_face";
		m_sTitle = "Unknown Face";
		m_sFamily = "across";
		m_eAnchor = RZ_EErrandAnchor.ANY_HOLDING;
		Offer("Nobody at {subject_place} knows your face, and nobody here does much either. That's useful. I've a letter for someone in there: the price of diesel, mostly, and something I should have said a month ago. Carry it in. Don't read it. You'll read it.",
			"I need a letter carried into {subject_place}. They'd shoot any of us on sight, after the last month. They won't shoot a stranger, probably.");
		Over("Never mind the letter. It's out of date. Everything's out of date this week.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Deliver it";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 40;
		w.m_sStep = "Carry the letter into {subject_place} and hand it over (5 s there). It says the price of diesel is going up, and that someone's sorry. Then come back to {giver}.";
		w.m_iFavor = 5;
		w.Done("Delivered. Good. Did they say anything? No. They wouldn't, would they.");

		w = Way("lie");
		w.m_sLabel = "Swap it for one you wrote";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 40;
		w.m_sStep = "Write your own letter and carry it into {subject_place} instead (5 s there), then come back to {giver}.";
		w.m_iFavor = 3;
		w.Done("Delivered? Good. Good. You look like you've something to say. No? All right.");

		w = Way("betray");
		w.m_sLabel = "Sell it to the camp";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 40;
		w.m_sStep = "Sell the letter to whoever runs {subject_place} (5 s there), then go back to {giver}.";
		w.Done("Back already? And it was all right? Good. I'll sleep tonight, then.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return RZ_Factions.IsValid(giver.m_iFaction) && dealer.GetGM().GetTier(playerId, giver.m_iFaction) == RZ_ETier.STRANGER;
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		for (int f = 0; f < 4; f++)
		{
			if (f == e.m_iFaction || f == RZ_EFaction.SURVIVORS)
				continue;

			RZ_Holding h = HoldingNear(gm, f, e.m_vGiver, 1500);
			if (!h)
				continue;

			Place(e, h.m_vPos, h.m_sName);
			e.m_iValue = f;
			e.m_sSubject = "holding:" + h.m_iId.ToString() + ".letter";
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (w.m_iIndex == 1)
		{
			gm.GetFacts().Memory().Lies().Tell(gm, playerId, "errand.unknown_face", e.m_sSubject, e.m_iFaction, e.m_sGiver, e.m_vGiver, RZ_ELieTest.RUMOUR, 3,
				"An answer came back to a letter nobody here wrote. They know whose face carried it.");
			return;
		}

		if (w.m_iIndex == 2)
		{
			dealer.SmallFavor(playerId, e.m_iValue, 6);
			if (RZ_Witness.Seen(gm, playerId, e.m_vGiver, e.m_iFaction))
				gm.AddInfamy(playerId, e.m_iFaction, 3, "sold a letter to a rival", 0, "sold a letter to the people it was meant to fool");
		}
	}
}
