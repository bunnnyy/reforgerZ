//! The Chalk on the Door (docs/ERRANDS.md, the house): a town within 1.5 km has the work tally (marks
//! on a wall for every job done there, docs/MISSIONS.md). The clerk wants to know if the last mark is
//! still true: go and look. Or say it is without looking (a lie the next reading finds).
class RZ_Errand_ChalkOnTheDoor : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "chalk_door";
		m_sTitle = "The Chalk on the Door";
		m_sFamily = "house";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "clerk";
		Offer("\"There are {count} marks on the wall at {subject_place}. The last one says it's clear. Is it still? Go and look before anyone believes it.\"",
			"\"Somebody chalked {subject_place} as done. Chalk lies, after a while. Go and read it for the book.\"");
		Over("\"Never mind. Somebody's been and looked.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Go and read the wall";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 60;
		w.m_sStep = "Read the marks on the wall in {subject_place} and look around (15 s there), then tell {giver}.";
		w.m_iFavor = 3;
		w.Done("\"Written. With today's date. The book's only as good as the last person who looked.\"");

		w = Way("lie");
		w.m_sLabel = "Say it's still true";
		w.m_iFavor = 2;
		w.Done("\"Still true. Good. I'll write that.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			int n = RZ_WorkRows.Tally(i);
			if (n <= 0 || RZ_Util.DistXZ(s.m_vCenter, e.m_vGiver) > 1500)
				continue;

			Place(e, RZ_Mission.LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.3), s.m_sName);
			e.SetText("count", n.ToString());
			e.m_sSubject = RZ_WorldFacts.Town(i) + ".chalk";
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (w.m_iIndex == 1)
			gm.GetFacts().Memory().Lies().Tell(gm, playerId, "errand.chalk", e.m_sSubject, RZ_EFaction.SURVIVORS, e.m_sGiver, e.m_vSubject, RZ_ELieTest.RUMOUR, 2,
				e.Fill("Someone walked into {subject_place} on the book's word and came back running. The clerk crossed out your 'still true'."));
	}
}
