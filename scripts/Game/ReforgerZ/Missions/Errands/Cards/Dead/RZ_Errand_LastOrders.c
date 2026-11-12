//! Last Orders (docs/ERRANDS.md, the dead): one of the giver's own leaders died within 3 km in the last
//! season day (the reason leader.died). His last order book is still on his seat. Bring it in before
//! someone else reads it, or read it first ([Known] with them: one true line about what he'd decided).
class RZ_Errand_LastOrders : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "last_orders";
		m_sTitle = "Last Orders";
		m_sFamily = "dead";
		m_eAnchor = RZ_EErrandAnchor.ANY_HOLDING;
		Offer("We lost our leader yesterday, {subject_place}. The order book is still on the seat: whatever was decided last, about us, is written in it. I'd like it in my hands before anyone else reads it.",
			"Our leader died {subject_place}, and the last orders are written in the book on the seat. Bring the book here. Don't read it. Or do. Just bring it.");
		Over("The order book's come in. Thank you all the same.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Bring it in unread";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 40;
		w.m_sStep = "Fetch the dead leader's order book {subject_place} (10 s there), and bring it to {giver} unread.";
		w.Work(RZ_EWork.TAKE, "Take the order book");
		w.m_eProp = RZ_EErrandProp.PAPERS;
		w.m_iFavor = 4;
		w.Done("Unread. Thank you. I'll read it tonight, alone, and then I'll decide whether anyone else does.");

		w = Way("do");
		w.m_sLabel = "Read it on the way";
		w.m_iMinTier = RZ_ETier.KNOWN;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 40;
		w.m_sStep = "Fetch the dead leader's order book {subject_place} (10 s there), read the last page, and bring it to {giver}.";
		w.Work(RZ_EWork.TAKE, "Take the order book");
		w.m_eProp = RZ_EErrandProp.PAPERS;
		w.m_iFavor = 3;
		w.Done("You read it. Of course you did. Well. Now two of us know what the last page says.");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		array<RZ_Reason> reasons = {};
		RZ_WorldFacts.Get().Memory().ReasonsNear(e.m_vGiver, 3000, reasons, "leader.died");
		foreach (RZ_Reason r : reasons)
		{
			if (r.faction >= 0 && r.faction != e.m_iFaction)
				continue;

			Place(e, RZ_Util.GroundPos(Vector(r.x, 0, r.z)), "");
			e.m_sSubject = r.subject + ".orders";
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex != 1)
			return;

		string line = "The last page is a list of names, crossed out one at a time. The last one isn't crossed out. It's the leader's own.";
		switch (e.m_iFaction)
		{
			case RZ_EFaction.MILITARY: line = "The last page says 'Hold. No relief expected. Count the rounds twice.' Then the weather, like every page. That was our leader all over."; break;
			case RZ_EFaction.BANDITS: line = "The last page is who owes what, and under it a line in a different hand: 'Settled.' Somebody's been at the book already."; break;
			case RZ_EFaction.INSTITUTE: line = "The last page is numbers, and at the bottom: 'If this is read by someone else, the samples in fridge 2 are mislabelled. On purpose.' I'll have to go and look."; break;
		}

		said.Insert(line);
	}
}
