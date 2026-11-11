//! The Smoke Was Ours (docs/ERRANDS.md, aftermath): shells came down within 1.5 km of a Bastion post
//! in the last 2 played hours (a Fire Mission or a shelling, the signal ring). The gunner wants the
//! count. Walk to the impact and count; carry two of the post's rations to the nearest house; or
//! tell the Haven whose shells they were. The arsonist rule: never to whoever called them.
class RZ_Errand_SmokeWasOurs : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "smoke_was_ours";
		m_sTitle = "The Smoke Was Ours";
		m_sFamily = "aftermath";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		Offer("\"Shells went down {where_place}. Command wants a count. Command always wants a count. Walk over and count for me?\"",
			"\"That smoke {where_place} was ours. Somebody has to say what it hit. Preferably not me.\"");
		Over("\"Never mind. Count's been filed. Somebody guessed.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Walk to the impact and count";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 40;
		w.m_sStep = "Walk to where the shells landed, {where_place}, and count (10 s there), then report to {giver}.";
		w.m_iFavor = 5;
		w.Done("\"Four infected, one shed, a bicycle. I'll write four. The shed was empty. The bicycle was somebody's.\"");

		w = Way("do");
		w.m_sLabel = "Take two rations to {other_house}";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT2;
		w.m_fSeconds = 5;
		w.m_fRadius = 40;
		w.m_sStep = "Carry two of the post's rations to the safehouse at {other_house} (at its gate), then report to {giver}.";
		w.m_iFame = 1;
		w.Done("\"Two rations off the books. I'll call it spoilage. Spoilage is very common this season.\"");

		w = Way("talk");
		w.m_sLabel = "Tell the Haven whose shells";
		w.Done("\"You told them. Fine. They'd have worked it out. We're the only ones with shells.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Signal s = FreshSignal("shell", e.m_vGiver, 1500, 2, e.m_iAsker);
		if (!s)
			return false;

		e.m_vSubject = RZ_Util.GroundPos(Vector(s.x, 0, s.z));
		e.m_sSubject = "shell:" + s.text;
		e.SetText("where_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
		e.SetText("subject_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
		int house = HouseNear(dealer.GetGM(), e.m_vGiver, 1500, -1, 0);
		if (house >= 0)
		{
			RZ_Safehouse h = dealer.GetGM().GetSafehouses().Get(house);
			e.m_iValue = house;
			e.m_vSubject2 = h.m_vPos;
			e.SetText("other_house", h.m_sName);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Offers(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w)
	{
		return w.m_iIndex != 1 || e.m_iValue >= 0;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (w.m_iIndex == 1 && e.m_iValue >= 0 && gm.GetSafehouses())
		{
			gm.GetSafehouses().AddFood(e.m_iValue, 2);
			dealer.SmallFame(playerId, RZ_EFaction.SURVIVORS, 1);
		}

		if (w.m_iIndex == 2)
		{
			array<int> who = {playerId};
			RZ_WorldFacts.Get().Memory().Record("errand.smoke_told", e.m_sSubject, "told", RZ_WorldMemory.SOURCE_ERRAND, false, who);
		}
	}
}
