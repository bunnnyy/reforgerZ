//! Borrowed (the epilogue deck): a house that's short wants back what another, better-fed house
//! "borrowed" in the season. Go and ask for it (2 food comes back), or say you couldn't find them
//! (nothing moves, and nobody checks). Within 1.5 km.
class RZ_Errand_Borrowed : RZ_EpiCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "epi_borrowed";
		m_sTitle = "Borrowed";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "storekeeper,cook";
		Offer("In the bad week {other_house} 'borrowed' three sacks of flour from our stores, and we let it go, because they had children on water. Now the cure's out and they're eating better than we are. Go and ask them for it back. Nicely.",
			"We lent {other_house} food when they had none, and never saw it again. It's the last week before the gates open. Debts between houses ought to be square by the end of it, or they'll follow us out.");
		Over("{other_house} sent the flour over themselves. Nobody had to ask.");

		RZ_ErrandWay w = Way("talk");
		w.m_sLabel = "Ask {other_house} for it back";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 40;
		w.m_sStep = "Go to the gate at {other_house} and ask for the borrowed food back (10 s there), then carry it to {giver}.";
		w.m_iFavor = 4;
		w.Done("They gave it back? Without an argument? Then it really is the last week. I'll make bread tonight.");

		w = Way("leave");
		w.m_sLabel = "Say you couldn't find them";
		w.Done("You couldn't find a whole house. Right. Well. Maybe they need it more than we do. Maybe that's what I'll tell myself.");
	}

	//------------------------------------------------------------------------------------------------
	override bool WhyCard(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (!Week() || e.m_iHouse < 0)
			return false;

		RZ_Safehouses houses = gm.GetSafehouses();
		RZ_Safehouse h = houses.Get(e.m_iHouse);
		if (!h || h.m_iFood >= h.FoodNeed(houses.PeoplePerFood()))
			return false;

		int other = HouseNear(gm, e.m_vGiver, 1500, e.m_iHouse, 2);
		if (other < 0)
			return false;

		RZ_Safehouse o = houses.Get(other);
		e.m_iValue = other;
		e.m_vSubject = o.m_vPos;
		e.m_sSubject = "epi.borrowed:" + RZ_WorldFacts.Safehouse(other);
		e.SetText("other_house", o.m_sName);
		e.SetText("subject_place", o.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex != 0 || e.m_iValue < 0)
			return;

		RZ_Safehouses houses = dealer.GetGM().GetSafehouses();
		RZ_Safehouse o = houses.Get(e.m_iValue);
		if (!o || o.m_iFood < 2)
			return;

		houses.AddFood(e.m_iValue, -2);
		houses.AddFood(e.m_iHouse, 2);
	}
}
