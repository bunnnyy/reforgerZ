//! Masks Off (docs/ERRANDS.md, Halcyon): Known with Halcyon, a house within 2 km declared a bite in the
//! last season day (the reason house.bitten). A man in a lab coat and a beak's respirator has been taking
//! "samples" from the bitten there: a Haven orderly selling them to the crews for his daughter's
//! antidote. Turn him in, let him go, or take his coat (number 17, signed out to Dr Brandt: useless).
class RZ_Errand_MasksOff : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "masks_off";
		m_sTitle = "Masks Off";
		m_sFamily = "lost";
		m_eAnchor = RZ_EErrandAnchor.HALCYON_STATION;
		m_iFaction = RZ_EFaction.INSTITUTE;
		Offer("\"Someone in one of our coats is taking samples at {subject_place}. We didn't send anyone. A small request: find out who.\"",
			"\"{subject_place} declared a bite. A man in a lab coat turned up within the hour with a kit. Not ours. We'd like the coat back. And the man, ideally.\"");
		Over("\"Never mind the coat. He's been dealt with.\"");

		RZ_ErrandWay w = Way("report");
		w.m_sLabel = "Find him, and turn him in";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_sStep = "Find the man in the lab coat at {subject_place} (15 s there), then tell {giver}.";
		w.m_iFavor = 4;
		w.m_iFame = 2;
		w.Done("\"An orderly. For his daughter. Yes, well. The coat goes back in the record.\"");

		w = Way("talk");
		w.m_sLabel = "Find him, and let him go";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_sStep = "Find the man in the lab coat at {subject_place} (15 s there), let him go, and tell {giver} nobody was there.";
		w.m_iFame = 2;
		w.m_iPayFaction = RZ_EFaction.SURVIVORS;
		w.Done("\"Gone. I see. I'll write 'gone'.\"");

		w = Way("do");
		w.m_sLabel = "Take his coat";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_sStep = "Find the man in the lab coat at {subject_place} and take the coat off him (15 s there).";
		w.Done("\"It's a good coat. It's also number seventeen, and seventeen is signed out to Dr Brandt. Nobody at our gate will believe you're Dr Brandt.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return RZ_Reputation.IsAtLeast(dealer.GetGM().GetTier(playerId, RZ_EFaction.INSTITUTE), RZ_ETier.KNOWN);
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		array<RZ_Reason> reasons = {};
		RZ_WorldFacts.Get().Memory().ReasonsNear(e.m_vGiver, 2000, reasons, "house.bitten");
		if (reasons.IsEmpty())
			return false;

		RZ_Reason r = reasons[0];
		vector at = Vector(r.x, 0, r.z);
		RZ_Safehouses houses = dealer.GetGM().GetSafehouses();
		string name = "";
		if (houses)
		{
			RZ_Safehouse h = houses.Get(houses.FindNearest(at));
			if (h)
				name = h.m_sName;
		}

		Place(e, RZ_Util.GroundPos(at), name);
		e.m_sSubject = r.subject + ".masks";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex == 1)
			said.Insert("He takes the mask off at the end of the lane. He's younger than you thought. He runs.");
	}
}
