//! Not Past the Chapel (docs/ERRANDS.md, the infected; re-angled): dusk within 20 real minutes, and a
//! hungry lot's ground within 1 km of an old man's farmhouse near the house. Bring him in (+1 person),
//! board his doors, or leave him. He's rude to you whichever. Never turning a hunting party.
class RZ_Errand_NotPastChapel : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "not_past_chapel";
		m_sTitle = "Not Past the Chapel";
		m_sFamily = "tribes";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_bTimed = true;
		Offer("There's an old man in the farmhouse out past the chapel who won't come in. Says nothing comes past the chapel. Something's been coming past the chapel all week: {lot}, and they're hungry. Before dark, please.",
			"The old man past the chapel is stubborn, and on his own, and {lot} are hungry and moving at dusk. Bring him in to us, or at least board him in.");
		Over("Never mind. He came in on his own. Complaining. But in.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Bring him in";
		w.m_ePlace = RZ_EErrandPlace.OUT;
		w.m_fSeconds = 10;
		w.m_fRadius = 30;
		w.m_fBefore = 20.5;
		w.m_sStep = "Fetch the old man from the farmhouse {place} and walk him back to {here} (10 s there).";
		w.Later("The old man from the farmhouse says the soup's too thin. He's had three bowls. Thank you.");
		w.m_iFavor = 5;
		w.Done("He says he'd have been fine, and he's only here because of the soup. He's been rude to everyone at supper. We're all very glad.");

		w = Way("do");
		w.m_sLabel = "Board his doors";
		w.m_ePlace = RZ_EErrandPlace.OUT;
		w.m_fSeconds = 25;
		w.m_fRadius = 30;
		w.m_fBefore = 20.5;
		w.m_sStep = "Board the old man's doors at the farmhouse {place}, two planks each (25 s there), then tell {giver}.";
		w.Work(RZ_EWork.BOARDS, "Board up the doors", 15);
		w.m_iFavor = 4;
		w.Done("Boarded? He shouted the whole time, I expect. Yes, I thought so.");

		w = Way("leave");
		w.m_sLabel = "Leave him to it";
		w.Done("He'd only have been rude to you. He's rude to everyone.");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		if (!RZ_GameTime.IsCalibrated() || RZ_GameTime.RealMinutesUntil(19.5) > 20)
			return false;

		RZ_Tribe t = TribeNear(dealer.GetGM(), e.m_vGiver, 4000);
		if (!t || t.m_fHunger < 60 || RZ_Util.DistXZ(t.m_vLair, e.m_vGiver) - t.m_fRadius > 1000)
			return false;

		e.SetText("lot", "the lot from " + t.m_sPlace);
		e.m_sSubject = RZ_WorldFacts.Safehouse(e.m_iHouse) + ".old_man";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex == 0 && dealer.GetGM().GetSafehouses())
			dealer.GetGM().GetSafehouses().AddPopulationAt(e.m_iHouse, 1);
	}
}
