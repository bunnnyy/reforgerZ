//! Is It True? (docs/ERRANDS.md, aftermath): a rumour going round names a town within 1.5 km of a
//! safehouse. Someone there wants to know. Go and look, or tell them what they want to hear: the
//! rumour then runs six hours longer, and pays nothing.
class RZ_Errand_IsItTrue : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "is_it_true";
		m_sTitle = "Is It True?";
		m_sFamily = "aftermath";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("Somebody said: {rumour} Is it true? You could go and look. {town}'s not far.",
			"They're saying {rumour} I want to know before I start believing it.");
		Over("Never mind. Someone came back from {town} and told us.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Go to {town} and look";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 80;
		w.m_sStep = "Go to {town} and see for yourself (15 s there), then tell {giver} what you saw.";
		w.m_iFavor = 4;
		w.Done("So that's how it is. Better to know. Usually.");

		w = Way("talk");
		w.m_sLabel = "Tell them it's true";
		w.Done("I knew it. I'll tell the others.");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		array<string> rumours = {};
		gm.GetFacts().GetRumours(rumours);
		foreach (string rumour : rumours)
		{
			foreach (int i, RZ_Settlement s : gm.GetSettlements())
			{
				if (i == e.m_iSettlement || !rumour.Contains(s.m_sName) || RZ_Util.DistXZ(s.m_vCenter, e.m_vGiver) > 1500)
					continue;

				e.m_vSubject = RZ_Mission.LandPointNear(s.m_vCenter, 0, 30);
				e.m_sSubject = RZ_WorldFacts.Town(i);
				e.m_sValue = rumour;
				e.SetText("town", s.m_sName);
				e.SetText("rumour", rumour);
				e.SetText("subject_place", s.m_sName);
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		// Telling them what they want to hear keeps it going round six hours longer
		if (w.m_iIndex == 1)
			dealer.GetGM().GetFacts().AddRumour(e.m_sValue + " Somebody went and looked, apparently.", 6);
	}
}
