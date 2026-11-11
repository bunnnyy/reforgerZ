//! A Word to {other_house} (docs/ERRANDS.md, the house): the clerk wants a message carried to a nearby
//! house, after one "borrowed" from the other: one is short, the other fed. Say it straight, soften
//! it (Trusted with the Haven), or add a threat of your own (Fear: a threat's cost, said at the gate,
//! never inside). Which you chose is a world record the houses' next quarrel can read.
class RZ_Errand_AWordTo : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "a_word_to";
		m_sTitle = "A Word to {other_house}";
		m_sFamily = "house";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "clerk";
		Offer("\"{other_house} \"borrowed\" from our stores, and they're eating better than we are. Someone should tell them so. Politely, or not.\"",
			"\"Carry a word to {other_house} for me? About the tins that walked over there. I'd go, but I'd say it badly.\"");
		Over("\"Leave it. {other_house} sent something back. Pride's cheaper on a full stomach.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Say it as it was said";
		w.m_iMinTier = RZ_ETier.KNOWN;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 40;
		w.m_sStep = "Carry the clerk's words to the safehouse at {other_house} (at its gate), then come back to {giver}.";
		w.m_iFavor = 5;
		w.Done("\"Straight? Good. Now they know we know. That's the whole point.\"");

		w = Way("talk");
		w.m_sLabel = "Soften it on the way";
		w.m_iTrust = 2;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 40;
		w.m_sStep = "Carry a kinder version to the safehouse at {other_house} (at its gate), then come back to {giver}.";
		w.m_iFavor = 5;
		w.Done("\"You softened it, didn't you. I can tell. They sent a jar of pickles. Pickles.\"");

		w = Way("press");
		w.m_sLabel = "Add a threat of your own";
		w.m_bThreat = true;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 40;
		w.m_sStep = "Say it at the gate of the safehouse at {other_house}, your way, then come back to {giver}.";
		w.m_iFavor = 5;
		w.Done("\"You said what? No, don't repeat it. They'll remember it. So will we.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses || !RZ_ErrandRoutine.HouseShort(gm, e.m_iHouse))
			return false;

		int other = HouseNear(gm, e.m_vGiver, 1500, e.m_iHouse, 2);
		if (other < 0)
			return false;

		RZ_Safehouse h = houses.Get(other);
		e.m_iValue = other;
		e.m_vSubject = h.m_vPos;
		e.m_sSubject = RZ_WorldFacts.Safehouse(other);
		e.SetText("other_house", h.m_sName);
		e.SetText("subject_place", h.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		array<string> how = {"straight", "softened", "threatened"};
		array<int> who = {playerId};
		string subject = string.Format("%1|%2", RZ_WorldFacts.Safehouse(e.m_iHouse), e.m_sSubject);
		RZ_WorldFacts.Get().Memory().Record("errand.word", subject, how[w.m_iIndex], RZ_WorldMemory.SOURCE_ERRAND, false, who);
	}
}
