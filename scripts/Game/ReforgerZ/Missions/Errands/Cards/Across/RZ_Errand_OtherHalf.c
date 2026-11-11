//! The Other Half (docs/ERRANDS.md, help across lines): Known with the giver's rival, and one of the
//! rival's holdings within 1.5 km. Carry a message to the people who want the giver dead. The message is
//! a threat: carried as written, the rival remembers who brought it. Softened (Trust 2 with the rival),
//! it's a message and nothing more.
class RZ_Errand_OtherHalf : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "other_half";
		m_sTitle = "The Other Half";
		m_sFamily = "across";
		m_eAnchor = RZ_EErrandAnchor.ANY_HOLDING;
		Offer("\"You talk to that lot {subject_place}. Take them a message from us. Word for word.\"",
			"\"They know your face over there. Tell them something from us. They won't like it. Say it anyway.\"");
		Over("\"Never mind the message. They heard.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Carry it word for word";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 30;
		w.m_sStep = "Say the message at {subject_place} (5 s at the gate), then come back to {giver}.";
		w.m_iFavor = 4;
		w.Done("\"Word for word? Good. Did they go pale? Tell me they went pale.\"");

		w = Way("talk");
		w.m_sLabel = "Soften it on the way";
		w.m_iTrust = 2;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 30;
		w.m_sStep = "Say a softer version at {subject_place} (5 s at the gate), then come back to {giver}.";
		w.m_iFavor = 4;
		w.Done("\"They said what? 'Noted'? That's all? Huh.\"");
	}

	//------------------------------------------------------------------------------------------------
	//! The side that wants the giver dead.
	static int RivalOf(int f)
	{
		if (f == RZ_EFaction.BANDITS)
			return RZ_EFaction.MILITARY;

		return RZ_EFaction.BANDITS;
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		int rival = RivalOf(e.m_iFaction);
		if (!RZ_Reputation.IsAtLeast(gm.GetTier(e.m_iAsker, rival), RZ_ETier.KNOWN))
			return false;

		RZ_Holding h = HoldingNear(gm, rival, e.m_vGiver, 1500);
		if (!h)
			return false;

		Place(e, h.m_vPos, h.m_sName);
		e.m_iValue = rival;
		e.m_sSubject = "holding:" + h.m_iId.ToString() + ".message";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Offers(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w)
	{
		if (w.m_iIndex == 1)
			w.m_iTrustFaction = e.m_iValue;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex == 0)
			dealer.GetGM().AddInfamy(playerId, e.m_iValue, 2, "carried a threat to their gate");
	}
}
