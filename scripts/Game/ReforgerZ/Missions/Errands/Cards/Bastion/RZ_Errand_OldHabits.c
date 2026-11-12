//! Old Habits (docs/ERRANDS.md, backgrounds: Ex-Con): a Red Hand camp within 1.5 km of a Bastion post.
//! The sergeant wants to know how a crew lad thinks: walk the wire and tell him where you'd come in. Or
//! tell him the wrong place (a lie the crews' next visit finds).
class RZ_Errand_OldHabits : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "old_habits";
		m_sTitle = "Old Habits";
		m_sFamily = "backgrounds";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		Offer("You've done time, they tell me. Good. Walk my wire and tell me where you'd come in. The crews {subject_place} have been in twice this month and taken diesel both times, and I still can't find the hole.",
			"I need someone who thinks like a thief. No offence. Somebody's been through my wire at night, twice, and the crews {subject_place} are the ones with full cans. Walk it and tell me where.");
		Over("Never mind the wire. We found the hole ourselves. The hard way, at three in the morning.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Walk the wire, and tell him";
		w.m_ePlace = RZ_EErrandPlace.GATE;
		w.m_fSeconds = 60;
		w.m_fRadius = 30;
		w.m_sStep = "Walk the post's wire at {place} (a minute there), then tell {giver} where you'd get in.";
		w.m_iFavor = 5;
		w.m_iFame = 1;
		w.Done("There? Under the water butt? I've walked past that a hundred times. Thank you. I think.");

		w = Way("lie");
		w.m_sLabel = "Tell him the wrong place";
		w.m_iFavor = 3;
		w.Done("The front gate? Bold. Right. I'll put two on the front gate tonight.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return RZ_QuestApproach.HasBackground(dealer.GetGM(), playerId, "excon");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Holding camp = HoldingNear(dealer.GetGM(), RZ_EFaction.BANDITS, e.m_vGiver, 1500);
		if (!camp)
			return false;

		e.SetText("subject_place", camp.m_sName);
		e.m_sSubject = "holding:" + e.m_iHolding.ToString() + ".wire";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (w.m_iIndex == 1)
			gm.GetFacts().Memory().Lies().Tell(gm, playerId, "errand.old_habits", e.m_sSubject, RZ_EFaction.MILITARY, e.m_sGiver, e.m_vGiver, RZ_ELieTest.RUMOUR, 3,
				"The crews came in under the water butt at the post, not the front gate. The sergeant remembers who told him the front gate.");
	}
}
