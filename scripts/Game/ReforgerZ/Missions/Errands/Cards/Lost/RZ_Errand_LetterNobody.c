//! The Letter Nobody Can Take (docs/ERRANDS.md, the lost): the Courier carries a letter for someone
//! from a house that lost people lately. If there's somewhere they went (another lived-in house
//! within 1.5 km), carry it there; or give it back to the Courier for the others he can't deliver.
class RZ_Errand_LetterNobody : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "letter_nobody";
		m_sTitle = "The Letter Nobody Can Take";
		m_sFamily = "lost";
		m_eAnchor = RZ_EErrandAnchor.WANDERER;
		m_iWandererRole = RZ_EWandererRole.COURIER;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("\"Letter for someone from {house}. They left. The house thinks they went to {other_house}. I've a bag of letters for people who went somewhere. Take this one?\"");
		Over("\"Never mind. Someone from {other_house} came for it.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Carry it to {other_house}";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 40;
		w.m_sStep = "Take the letter to the safehouse at {other_house} (at its gate), then find {giver} on the road again.";
		w.m_iFavor = 4;
		w.Done("\"Delivered? Good. One out of the bag. The bag doesn't notice.\"");

		w = Way("talk");
		w.m_sLabel = "Give it back to him";
		w.Done("\"Fair. It goes back in with the others. The post is mostly waiting, now.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses)
			return false;

		int from = -1;
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_HouseFacts facts = RZ_WorldFacts.Get().House(i);
			if (facts.m_bValid && facts.m_iLost > 0 && RZ_Util.DistXZ(facts.m_vPos, e.m_vGiver) <= 3000)
			{
				from = i;
				break;
			}
		}

		if (from < 0)
			return false;

		int to = HouseNear(gm, e.m_vGiver, 1500, from, 0);
		if (to < 0)
			return false;

		RZ_Safehouse fromHouse = houses.Get(from);
		RZ_Safehouse toHouse = houses.Get(to);
		e.m_iValue = to;
		e.m_vSubject = toHouse.m_vPos;
		e.m_sSubject = RZ_WorldFacts.Safehouse(to);
		e.SetText("house", fromHouse.m_sName);
		e.SetText("other_house", toHouse.m_sName);
		e.SetText("subject_place", toHouse.m_sName);
		return true;
	}
}
