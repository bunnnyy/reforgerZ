//! The New Number (docs/ERRANDS.md, Halcyon): the cure reached a milestone in the last season day (the
//! reason cure.<percent>). The board at the station's gate still shows the old one. Paint the new
//! number on it: the queue reads it every morning.
class RZ_Errand_NewNumber : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "new_number";
		m_sTitle = "The New Number";
		m_sFamily = "lost";
		m_eAnchor = RZ_EErrandAnchor.HALCYON_STATION;
		m_iFaction = RZ_EFaction.INSTITUTE;
		Offer("The board at the gate still says the old number. The queue reads it every morning. A small request: paint the new one. Neatly. People photograph it. Well. They would.",
			"We've moved on, scientifically. The board hasn't. Paint the number at the gate. Please don't round it up.");
		Over("Never mind the board. Somebody painted it. Badly.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Paint the new number";
		w.m_ePlace = RZ_EErrandPlace.GATE;
		w.m_fSeconds = 10;
		w.m_fRadius = 20;
		w.m_sStep = "Paint the new number on the board at the gate {place} (10 s there), then tell {giver}.";
		w.m_iFavor = 3;
		w.Done("It looks smaller than it felt. They always do.");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		array<RZ_Reason> all = {};
		RZ_WorldFacts.Get().Memory().AllReasons(all);
		foreach (RZ_Reason r : all)
		{
			if (!r.key.StartsWith("cure."))
				continue;

			e.m_sSubject = "holding:" + e.m_iHolding.ToString() + ".board";
			return true;
		}

		return false;
	}
}
