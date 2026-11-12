//! Copies for the Far House (the epilogue deck): with the method published, Halcyon wants a copy
//! carried to the Haven house farthest from the lab, the one a medic will have to make the batch at.
//! Or sell the copies to the Red Hand's fence: Red Hand Favor, 5 Halcyon Infamy. Under Halcyon's
//! route with the method published.
class RZ_Errand_CopiesFarHouse : RZ_EpiCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "epi_far_copies";
		m_sTitle = "Copies for the Far House";
		m_eAnchor = RZ_EErrandAnchor.HALCYON_STATION;
		m_iFaction = RZ_EFaction.INSTITUTE;
		Offer("The method is published. Publishing means nothing until it's in {house}. It's the farthest house. Their medic needs a copy.",
			"Twelve pages, the method in full, for the medic at {house}. Nobody else on the list is that far out.");
		Over("Never mind. {house} sent someone for it.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Carry the method to {house}";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 40;
		w.m_sStep = "Take the method to the medic at {house} (at the gate, 10 s, nothing in your hands), then report to {giver}.";
		w.m_iFavor = 6;
		w.m_iFame = 1;
		w.Done("Received at {house}. Then it's published.");

		w = Way("press");
		w.m_sLabel = "Sell the copies to the fence";
		w.m_iFavor = 6;
		w.m_iPayFaction = RZ_EFaction.BANDITS;
		w.Done("\"The fence has them. Of course the fence has them.\" A note goes in a file.");
	}

	//------------------------------------------------------------------------------------------------
	override bool WhyCard(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Epilogue epi = Week();
		if (!epi || Route() != RZ_CureFinale.HALCYON || epi.Terms() != 4 || !gm.GetSafehouses())
			return false;

		RZ_Safehouses houses = gm.GetSafehouses();
		int far = -1;
		float best = 0;
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			if (!h || h.m_iPopulation <= 0)
				continue;

			float d = RZ_Util.DistXZ(h.m_vPos, gm.GetLabPosition());
			if (d > best)
			{
				best = d;
				far = i;
			}
		}

		if (far < 0)
			return false;

		RZ_Safehouse house = houses.Get(far);
		e.m_iValue = far;
		e.m_vSubject = house.m_vPos;
		e.m_sSubject = "epi.copies:" + RZ_WorldFacts.Safehouse(far);
		e.SetText("house", house.m_sName);
		e.SetText("subject_place", house.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex == 1)
			dealer.GetGM().AddInfamy(playerId, RZ_EFaction.INSTITUTE, 5, "sold the method to the fence", 0, "sold the lab's method to the Red Hand");
	}

	//------------------------------------------------------------------------------------------------
	//! Unarmed: a gun in your hands at the step spoils it.
	override bool Still(RZ_Errands dealer, RZ_Errand e)
	{
		return !ArmedAtStep(e);
	}
}
