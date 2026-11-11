//! Both Hands (docs/ERRANDS.md, hidden truths and the crews): someone needs you because you're liked
//! and feared at once (Mixed Bag, Useful Trouble or Favourite Rascal with the giver's faction): talk a
//! brother out of a crew, or (for the Red Hand) an informer out of a post. He doesn't want to come,
//! and his crew doesn't want him to go: seen by them, it costs a little with them.
class RZ_Errand_BothHands : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "both_hands";
		m_sTitle = "Both Hands";
		m_sFamily = "crews";
		m_eAnchor = RZ_EErrandAnchor.ANY_HOLDING;
		Offer("\"My brother's at {subject_place}, with the wrong people. You're the only one they'd let talk to him and the only one he'd listen to. Bring him home.\"",
			"\"You're liked and you're feared. That's rare. There's a lad at {subject_place} who needs both. Talk him out.\"");
		Over("\"He came home on his own. Or he didn't. Either way, it's done.\"");

		RZ_ErrandWay w = Way("talk");
		w.m_sLabel = "Go and talk him out";
		w.m_iTrust = 2;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 20;
		w.m_fRadius = 40;
		w.m_sStep = "Find him at {subject_place} and talk him out (20 s there), then come back to {giver}.";
		w.m_iFavor = 6;
		w.m_iFame = 1;
		w.Done("\"He's back. Sulking, but back. I owe you, and I hate owing.\"");

		w = Way("leave");
		w.m_sLabel = "Leave him where he is";
		w.Done("\"He chose. I know. I still had to ask.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		if (!RZ_Factions.IsValid(giver.m_iFaction))
			return false;

		RZ_ERep rep = RepOf(dealer.GetGM(), playerId, giver.m_iFaction);
		return rep == RZ_ERep.MIXED_BAG || rep == RZ_ERep.USEFUL_TROUBLE || rep == RZ_ERep.FAVOURITE_RASCAL;
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		int other = RZ_EFaction.BANDITS;
		if (e.m_iFaction == RZ_EFaction.BANDITS)
			other = RZ_EFaction.MILITARY;

		RZ_Holding h = HoldingNear(dealer.GetGM(), other, e.m_vGiver, 1500);
		if (!h)
			return false;

		Place(e, h.m_vPos, h.m_sName);
		e.m_iValue = other;
		e.m_sSubject = "holding:" + h.m_iId.ToString() + ".brother";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (w.m_iIndex != 0)
			return;

		// His crew saw him go, and saw who took him
		if (RZ_Witness.Seen(gm, playerId, e.m_vSubject, e.m_iValue))
		{
			gm.AddInfamy(playerId, e.m_iValue, 2, "talked one of theirs out", 0, "talked one of theirs into leaving");
			said.Insert("His lot watched him walk out with you. They'll remember whose idea it was.");
		}
	}
}
