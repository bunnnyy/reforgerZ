//! Is She Gone? (docs/ERRANDS.md, the infected): a pack within 1.5 km lost its leader lately (it stands
//! its vigil), and the house knew who she'd been. "Ilse ran the bakery. Is it true?" Go to the lair's
//! edge and see whether they still gather there (15 s unseen), or, an Elder, sit at supper while the house
//! says her name. No keepsake: that's Bring Them Home's.
class RZ_Errand_IsSheGone : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "is_she_gone";
		m_sTitle = "Is She Gone?";
		m_sFamily = "tribes";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("\"{alpha} ran the bakery. They're saying the calling at {lair} stopped. Is it true? Go and look. From a long way off.\"",
			"\"The calling stopped at {lair}. If that means {alpha}'s gone, I want to know. I'd like to stop listening for her.\"");
		Over("\"Never mind. Somebody went. It's true.\"");

		RZ_ErrandWay w = Way("sneak");
		w.m_sLabel = "Look from the edge, unseen";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 30;
		w.m_bUnseen = true;
		w.m_sStep = "Watch {lair} from its edge {subject_place}, unseen (15 s hidden there), then tell {giver}.";
		w.m_iFavor = 4;
		w.Done("\"They're standing round where she lay. Just standing. Then it's true.\"");

		w = Way("talk");
		w.m_sLabel = "Sit at supper while they say her name";
		w.m_sBackground = "elder";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 60;
		w.m_fRadius = 15;
		w.m_sStep = "Sit at the house's supper (a minute at the table) while they say {alpha}'s name.";
		w.m_iFavor = 3;
		w.Done("\"Thank you for sitting. You knew how. Most people don't.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Tribe t = TribeNear(dealer.GetGM(), e.m_vGiver, 1500);
		if (!t || !t.m_bVigil || t.m_sAlphaName.IsEmpty())
			return false;

		vector dir = e.m_vGiver - t.m_vLair;
		dir[1] = 0;
		float len = Math.Max(1, dir.Length());
		Place(e, RZ_Mission.LandPointNear(t.m_vLair + dir * (130 / len), 0, 15), "");

		// Near Traces' kill-site candle (L1) on the way, when one stands within 600 m of the lair
		vector sceneAt;
		string sceneKey;
		if (OnScene("L1", t.m_vLair, 600, sceneAt, sceneKey))
			Place(e, sceneAt, "");
		e.SetText("lair", t.m_sPlace);
		e.SetText("alpha", t.m_sAlphaName);
		e.m_sSubject = "tribe:" + t.m_iId.ToString() + ".gone";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex == 1)
		{
			array<int> who = {playerId};
			RZ_WorldFacts.Get().Memory().Record("errand.said_name", e.GetText("alpha"), "supper", RZ_WorldMemory.SOURCE_ERRAND, true, who,
				e.Fill("At supper the house still leaves the end of the bench for {alpha}, who ran the bakery."), "board");
		}
	}
}
