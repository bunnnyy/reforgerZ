//! Processing (the epilogue deck, errands' card): a pinned local wants walking to the queue at the
//! north road, never to the barrier itself: "Processing. I've been processed since Day 2." They sit
//! in the queue, and go with the Walk Out when it opens. Or talk them into supper, where they are the
//! next night. Unarmed. Only with the queue within 1.5 km of the house.
class RZ_Errand_Processing : RZ_EpiCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "epi_processing";
		m_sTitle = "Processing";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("{local} wants walking to the queue. Processing. I've been processed since Day 2, {local} says. Not past the queue. Never to the barrier.",
			"{local} has a bag packed and the Day 2 slip in a pocket. Walk them up to the queue? Just the queue.");
		Over("Never mind. {local} went up with the morning lot.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Walk {local} to the queue";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 20;
		w.m_fRadius = 40;
		w.m_sStep = "Walk {local} up to the queue on the north road (20 s there, nothing in your hands), then tell {giver}.";
		w.m_iFavor = 4;
		w.m_iFame = 1;
		w.Done("Sitting in the queue, with the slip out. Thank you. That's where they wanted to be.");

		w = Way("talk");
		w.m_sLabel = "Talk {local} into supper";
		w.Done("\"Supper first, then. Tomorrow's queue is the same queue.\" They put the bag down by the door.");
	}

	//------------------------------------------------------------------------------------------------
	override bool WhyCard(RZ_Errands dealer, RZ_Errand e)
	{
		vector q = Queue();
		if (q == vector.Zero || RZ_Util.DistXZ(q, e.m_vGiver) > 1500)
			return false;

		RZ_Locals locals = dealer.GetLocals();
		if (!locals)
			return false;

		array<RZ_Local> here = {};
		locals.AtHouse(e.m_iHouse, here);
		foreach (RZ_Local l : here)
		{
			if (!l.alive || !l.pinned || l.memos.Contains("queued"))
				continue;

			e.m_iValue = l.id;
			e.m_vSubject = q;
			e.m_sSubject = "epi.processing:" + l.id.ToString();
			e.SetText("local", l.name);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool Still(RZ_Errands dealer, RZ_Errand e)
	{
		return !ArmedAtStep(e);
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_Local l = dealer.GetLocals().Get(e.m_iValue);
		if (!l)
			return;

		if (w.m_iIndex == 0)
		{
			dealer.GetLocals().Remember(l, playerId, "queued");
			return;
		}

		dealer.GetLocals().Remember(l, playerId, "supper");
	}
}
