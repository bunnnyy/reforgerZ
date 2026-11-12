//! Masks Off (docs/ERRANDS.md, help across lines): a house within 2 km took a bite declaration. A
//! "beak" (a coat, a hood, a respirator) has been taking "samples" from the bitten there: a Haven
//! orderly selling them to the Red Hand for a daughter's antidote. Find the orderly at the house
//! (15 s there), then: turn them in (+2 Halcyon Fame, coat number 17 back in the record); let them go
//! (+2 Haven Fame); or take the coat (signed out to Dr Brandt: no good at Halcyon's looks, and 35% the
//! lab finds out: +3 Halcyon Infamy). The coat is the card's, not an item.
class RZ_Errand_MasksOff : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "masks_off";
		m_sTitle = "Masks Off";
		m_sFamily = "across";
		m_eAnchor = RZ_EErrandAnchor.HALCYON_STATION;
		m_iFaction = RZ_EFaction.INSTITUTE;
		Offer("Someone in one of our coats has been taking samples from the bitten at {house}. We sent nobody. Coat number 17. Please find out who is inside it.",
			"A coat and a respirator at {house}, with a tray and no paperwork. Number 17 is signed out to Dr Brandt, who has not left the lab.");
		Over("Number 17 came back on its own, folded. We are choosing not to ask.");

		RZ_ErrandWay w = Way("report");
		w.m_sLabel = "Turn the orderly in";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_sStep = "Find the beak at {house} (15 s there): a Haven orderly with a tray of samples. Bring the coat back to {giver}, and the name.";
		w.m_iFame = 2;
		w.Done("Number 17. Thank you. We will speak to the Haven about the orderly, politely.");

		w = Way("talk");
		w.m_sLabel = "Let the orderly go";
		w.m_iPayFaction = RZ_EFaction.SURVIVORS;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_sStep = "Find the beak at {house} (15 s there). Hear out the orderly, who has a daughter and no antidote. Then tell {giver} you found nobody.";
		w.m_iFame = 2;
		w.Done("Nobody. I see. We will keep looking, and so, I expect, will the coat.");

		w = Way("take");
		w.m_sLabel = "Take the coat for yourself";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_sStep = "Find the beak at {house} (15 s there) and take the coat off the orderly. Then tell {giver} it was gone.";
		w.Done("\"Gone. Number 17 is gone.\" Someone writes that down, carefully.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return RZ_Reputation.IsAtLeast(dealer.GetGM().GetTier(playerId, RZ_EFaction.INSTITUTE), RZ_ETier.KNOWN);
	}

	//------------------------------------------------------------------------------------------------
	//! A lived-in house within 2 km that took a bite declaration (its reason is up).
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses)
			return false;

		RZ_WorldMemory mem = RZ_WorldFacts.Get().Memory();
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			if (!h || h.m_iPopulation <= 0 || RZ_Util.DistXZ(h.m_vPos, e.m_vGiver) > 2000)
				continue;

			if (!mem.FindReason("house.bitten", RZ_WorldFacts.Safehouse(i)))
				continue;

			Place(e, h.m_vPos, h.m_sName);
			e.m_iValue = i;
			e.m_sSubject = RZ_WorldFacts.Safehouse(i) + ".beak";
			e.SetText("house", h.m_sName);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_DisguiseMarks marks;
		if (gm.GetHostility())
			marks = gm.GetHostility().GetDisguise().GetMarks();

		array<int> who = {playerId};
		if (w.m_iIndex == 0 && marks)
			marks.CoatBack();

		if (w.m_iIndex == 2)
		{
			if (marks)
				marks.CoatOut();

			said.Insert("It's a good coat. It's also number 17, and number 17 is signed out to Dr Brandt. No Halcyon guard will take you for anyone in it.");
			if (Math.RandomFloat01() < 0.35)
			{
				gm.AddInfamy(playerId, RZ_EFaction.INSTITUTE, 3, "kept a numbered coat", RZ_RepFlag.ACT);
				dealer.Notify(e, "The lab counted coats at the change of shift. Number 17 was seen on you. +3 Halcyon Infamy.");
			}
		}

		RZ_WorldFacts.Get().Memory().Record("errand.masks_off", e.m_sSubject, w.m_sWay, RZ_WorldMemory.SOURCE_ERRAND, false, who);
	}
}
