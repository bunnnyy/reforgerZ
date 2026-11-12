//! Sleeve Down (docs/ERRANDS.md, the house): someone at a house of five or more asks quietly for
//! something for "the fever", after a bite on the roads nearby. The truth deck decides whether it's
//! flu or a bite (six in ten a bite), the same for everyone at that house for 3 season days; the
//! same three replies show either way, and a Nurse or Doctor can find out before deciding.
class RZ_Errand_SleeveDown : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "sleeve_down";
		m_sTitle = "Sleeve Down";
		m_sFamily = "house";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("Something for the fever. Don't make a fuss. It's only a fever.",
			"Have you anything for a temperature? It's for me. It's nothing. Keep your voice down.");
		Over("Never mind. It's passed. Forget I asked.");

		RZ_ErrandWay w = Way("give");
		w.m_sLabel = "Hand over something for the fever";
		w.m_eItem = RZ_EErrandItem.MEDICAL;
		w.m_iItems = 1;
		w.m_iFavor = 4;

		w = Way("talk");
		w.m_sLabel = "Ask to see their arm";
		w.m_sBackground = "nurse,doctor";
		w.m_iFavor = 3;

		w = Way("report");
		w.m_sLabel = "Tell the clerk";
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_HouseFacts house = RZ_WorldFacts.Get().House(e.m_iHouse);
		if (!house.m_bValid || house.m_iPeople < 5)
			return false;

		if (!FreshSignal("bite", e.m_vGiver, 1500, 3, e.m_iAsker))
			return false;

		e.m_sSubject = RZ_WorldFacts.Safehouse(e.m_iHouse);
		array<string> truths = {"flu", "bite"};
		array<float> weights = {0.4, 0.6};
		e.m_sValue = RZ_WorldFacts.Get().Memory().DrawTruth(m_sKey, e.m_sSubject, truths, weights);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_WorldMemory mem = RZ_WorldFacts.Get().Memory();
		bool bite = e.m_sValue == "bite";
		RZ_Safehouses houses = gm.GetSafehouses();
		RZ_Safehouse h;
		if (houses)
			h = houses.Get(e.m_iHouse);

		switch (w.m_iIndex)
		{
			case 0:
			{
				said.Insert("Thank you. I'll be better by morning.");
				if (bite && h && h.m_iPopulation > 2)
				{
					// Kept quiet: the house loses someone in the night
					houses.RemoveDead(e.m_iHouse, 1);
					dealer.Notify(e, "In the night, someone at {house} turned. They'd said it was a fever.");
				}

				break;
			}

			case 1:
			{
				mem.FoundTruth(m_sKey, e.m_sSubject, "nurse");
				if (bite)
				{
					said.Insert("They roll the sleeve up without a word. The marks are a day old. \"I'll tell the clerk myself. Will you stand with me?\"");
					dealer.SmallFame(playerId, RZ_EFaction.SURVIVORS, 2);
					mem.Record("errand.sleeve_down", e.m_sSubject, "declared", RZ_WorldMemory.SOURCE_ERRAND, false);
				}
				else
				{
					said.Insert("A rash, a cough, a temperature. \"It's flu,\" you say, and they laugh for the first time in a week.");
				}

				break;
			}

			case 2:
			{
				if (bite && h && h.m_iPopulation > 2)
				{
					houses.RemoveDead(e.m_iHouse, 1);
					said.Insert("The clerk takes the arm, and then takes the person outside. At dusk, two of them walk out. One comes back.");
					mem.Record("errand.sleeve_down", e.m_sSubject, "walked out", RZ_WorldMemory.SOURCE_ERRAND, false);
				}
				else
				{
					said.Insert("The clerk looks, and finds a rash. Nobody at {house} looks at you at supper.");
					RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
					if (trees)
						trees.GetMemory().AddRegard(playerId, e.m_sGiver, "slander");
				}

				break;
			}
		}

		mem.UseTruth(m_sKey, e.m_sSubject);
	}
}
