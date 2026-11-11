//! The Last Apples (docs/ERRANDS.md, the house): late in the season (Overrun or after), the house under
//! two upkeeps of food, and an empty village within 1.2 km with an orchard nobody's picked. The cook
//! wants the last of the apples before the frost and the infected take them.
class RZ_Errand_LastApples : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "last_apples";
		m_sTitle = "The Last Apples";
		m_sFamily = "house";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "cook";
		Offer("\"There's an orchard at {place} nobody's picked. The last apples of the year. Bring a bag. Don't bring anything else back.\"",
			"\"Apples, at {place}. Bruised, wormy, the last of them. I'll make them into something. Do us a turn.\"");
		Over("\"Never mind the apples. The frost had them.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Pick the last of them";
		w.m_ePlace = RZ_EErrandPlace.EMPTY_VILLAGE;
		w.m_fSeconds = 30;
		w.m_fRadius = 80;
		w.m_bQuiet = true;
		w.m_sStep = "Pick what's left in the orchard at {place} (30 s there), then bring it to {giver}.";
		w.m_iFavor = 4;
		w.Done("\"Wormy. Perfect. Nobody look in the pot.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (gm.GetDirector().GetStage() < RZ_EOutbreakStage.OVERRUN)
			return false;

		RZ_Safehouses houses = gm.GetSafehouses();
		RZ_Safehouse h = null;
		if (houses)
			h = houses.Get(e.m_iHouse);

		if (!h || h.m_iFood >= h.FoodNeed(houses.PeoplePerFood()) * 2)
			return false;

		e.m_sSubject = RZ_WorldFacts.Safehouse(e.m_iHouse) + ".apples";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (dealer.GetGM().GetSafehouses())
			dealer.GetGM().GetSafehouses().AddFood(e.m_iHouse, 2);
	}
}
