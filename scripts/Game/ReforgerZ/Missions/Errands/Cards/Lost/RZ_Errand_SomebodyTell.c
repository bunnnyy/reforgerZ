//! Somebody Should Tell Them (docs/ERRANDS.md, the lost): someone at the house lost family, and the
//! Haven thinks they run with the infected near here now. Read the marks at the edge of their ground,
//! tell them what a Hunter knows, or check the house's book of names, and come back with which lot.
//! It posts a reason (`house.lost_to`) that Bring Them Home answers. No keepsake, never a player's
//! character, no pay but regard.
class RZ_Errand_SomebodyTell : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "somebody_tell";
		m_sTitle = "Somebody Should Tell Them";
		m_sFamily = "lost";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "clerk";
		Offer("There's a woman here who lost her husband on the road last week. People say he's one of them now, walking with {lot}. Somebody should find out for sure, and tell her properly.",
			"There's a woman here whose husband turned on the road. She keeps asking which of the infected he walks with now. Nobody will answer her. Somebody should.");
		Over("Never mind. She knows now. Someone told her. Badly, but they told her.");

		RZ_ErrandWay w = Way("sneak");
		w.m_sLabel = "Read the marks at their ground's edge";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_bUnseen = true;
		w.m_sStep = "Read the marks where the road enters the ground of {lot}, {subject_place}, without being seen (15 s there), then tell {giver}.";
		w.Done("So it's them. Thank you. I'll tell her. No. You tell her. She'll want to hear it from whoever looked.");

		w = Way("talk");
		w.m_sLabel = "Tell them what you know of that lot";
		w.m_sBackground = "hunter";
		w.Done("You'd know. You've tracked them. I'll tell her it's them.");

		w = Way("do");
		w.m_sLabel = "Check the house's book of names";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 20;
		w.m_fRadius = 15;
		w.m_sStep = "Go through the house's book of names at {here} with the clerk (20 s there), then tell {giver}.";
		w.Done("His name, the day, and a note in the margin about which way he walked. That's enough to tell her.");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_HouseFacts house = RZ_WorldFacts.Get().House(e.m_iHouse);
		if (!house.m_bValid || (house.m_iDied <= 0 && house.m_iLost <= 0))
			return false;

		RZ_Tribe t = TribeNear(dealer.GetGM(), e.m_vGiver, 3500);
		if (!t)
			return false;

		vector dir = e.m_vGiver - t.m_vLair;
		dir[1] = 0;
		float len = Math.Max(1, dir.Length());
		Place(e, RZ_Mission.LandPointNear(t.m_vLair + dir * (t.m_fRadius / len), 0, 25), "");
		if (RZ_Util.DistXZ(e.m_vSubject, e.m_vGiver) > 1500)
			return false;

		e.m_iValue = t.m_iId;
		e.SetText("lot", "the lot from " + t.m_sPlace);
		e.m_sSubject = RZ_WorldFacts.Safehouse(e.m_iHouse) + ".lost_to";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_WorldMemory mem = RZ_WorldFacts.Get().Memory();
		mem.PostReason("house.lost_to", RZ_WorldFacts.Safehouse(e.m_iHouse), e.Fill("{house} knows one of theirs goes out with {lot} now."), e.m_vGiver, RZ_EFaction.SURVIVORS, 3);
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		if (trees)
			trees.GetMemory().AddRegard(playerId, e.m_sGiver, "kept");
	}
}
