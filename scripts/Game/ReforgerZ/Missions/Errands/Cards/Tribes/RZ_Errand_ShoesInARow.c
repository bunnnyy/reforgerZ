//! Shoes in a Row (docs/ERRANDS.md, the infected): the Haven's clerk, by day, a lair within 1.5 km
//! whose leader the Haven knew by name. Count the shoes by the lair from about 100 m unnoticed (a
//! Hunter from 150 m): the clerk writes the number in the book. A made-up number is read at supper as
//! "about", never sent on, and it's a lie.
class RZ_Errand_ShoesInARow : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "shoes_in_row";
		m_sTitle = "Shoes in a Row";
		m_sFamily = "tribes";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "clerk";
		Offer("They leave their shoes in a row by {lair}. Nobody knows why. Count them for the book. From a distance. A long distance.",
			"The book has a page for {lair}. It wants a number. Count the shoes, unseen, and come back with it.");
		Over("Never mind. The count's been done. Somebody braver. Or stupider.");

		RZ_ErrandWay w = Way("sneak");
		w.m_sLabel = "Count the shoes, unseen";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 30;
		w.m_fRadius = 30;
		w.m_bUnseen = true;
		w.m_sStep = "Count the shoes by {lair} from {subject_place}, unseen (30 s hidden there), then tell {giver}.";
		w.m_iFavor = 5;
		w.Done("{count}. Written. In ink, even. Thank you.");

		w = Way("sneak");
		w.m_sLabel = "Count them from further off";
		w.m_sBackground = "hunter";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT2;
		w.m_fSeconds = 30;
		w.m_fRadius = 40;
		w.m_bUnseen = true;
		w.m_sStep = "Count the shoes by {lair} from 150 m out, unseen (30 s hidden there), then tell {giver}.";
		w.m_iFavor = 5;
		w.Done("{count}, from that far? I'll put a note: counted by someone who knew how.");

		w = Way("lie");
		w.m_sLabel = "Make a number up";
		w.m_iFavor = 3;
		w.Done("About that many. I'll write 'about'.");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		if (RZ_GameTime.IsNight())
			return false;

		RZ_Tribe t = TribeNear(dealer.GetGM(), e.m_vGiver, 1500);
		if (!t || !t.m_bHasAlpha || t.m_sAlphaName.IsEmpty())
			return false;

		vector dir = e.m_vGiver - t.m_vLair;
		dir[1] = 0;
		float len = Math.Max(1, dir.Length());
		Place(e, RZ_Mission.LandPointNear(t.m_vLair + dir * (110 / len), 0, 15), "");
		e.m_vSubject2 = RZ_Mission.LandPointNear(t.m_vLair + dir * (150 / len), 0, 15);
		int count = t.m_iGuards + t.m_iHunters + t.m_iScouts + t.m_iYoung + 1;
		e.SetText("count", count.ToString());
		e.SetText("lair", t.m_sPlace);
		e.m_iValue = t.m_iId;
		e.m_sSubject = "tribe:" + t.m_iId.ToString() + ".shoes";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (w.m_iIndex == 2)
		{
			gm.GetFacts().Memory().Lies().Tell(gm, playerId, "errand.shoes", e.m_sSubject, RZ_EFaction.SURVIVORS, e.m_sGiver, e.m_vSubject, RZ_ELieTest.RUMOUR, 2,
				"Someone counted the shoes properly. The clerk crossed out your 'about' and wrote the real number next to it.");
			return;
		}

		array<int> who = {playerId};
		RZ_WorldFacts.Get().Memory().Record("errand.shoes", e.m_sSubject, e.GetText("count"), RZ_WorldMemory.SOURCE_ERRAND, false, who,
			e.Fill("The house's book has a page for {lair}, and a number on it in someone else's hand."), "board");
	}
}
