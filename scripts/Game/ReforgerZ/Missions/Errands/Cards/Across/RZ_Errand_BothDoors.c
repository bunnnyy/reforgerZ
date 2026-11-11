//! Both Doors (docs/ERRANDS.md, help across lines; backgrounds: the Drifter): a Drifter Known with all
//! four, a Bastion post and a Red Hand camp within 1.5 km of each other. Each wants the other's shift
//! times. Carry true ones both ways (paid by both), carry the crews a false one (a lie, found at the
//! next shift), or carry nothing and say so. "Nobody trusts a man everyone trusts. Except everyone,
//! apparently."
class RZ_Errand_BothDoors : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "both_doors";
		m_sTitle = "Both Doors";
		m_sFamily = "background";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		Offer("\"You go in at {subject_place} and nobody shoots. Remarkable. We'd like their shift times. They'd like ours. You'd be the man in the middle. Again.\"",
			"\"The crews at {subject_place} want our shift times and we want theirs. You're the only one either side lets through both doors.\"");
		Over("\"Never mind. They changed their shifts. So did we. Everyone's guessing again.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Carry true times both ways";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 40;
		w.m_sStep = "Take Bastion's shift times to the crews at {subject_place} and bring theirs back (5 s there).";
		w.m_iFavor = 5;
		w.m_iFame = 1;
		w.Done("\"Nobody trusts a man everyone trusts. Except everyone, apparently.\"");

		w = Way("lie");
		w.m_sLabel = "Give the crews a false time";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 40;
		w.m_sStep = "Take the crews at {subject_place} a shift time that's wrong, and bring theirs back (5 s there).";
		w.m_iFavor = 3;
		w.Done("\"Their times. Lovely. And ours? You gave them ours. Good.\"");

		w = Way("talk");
		w.m_sLabel = "Carry nothing, and say so";
		w.Done("\"Neither side, then. Fair. Infuriating, but fair.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (!RZ_QuestApproach.HasBackground(gm, playerId, "drifter"))
			return false;

		for (int f = 0; f < 4; f++)
		{
			if (!RZ_Reputation.IsAtLeast(gm.GetTier(playerId, f), RZ_ETier.KNOWN))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Holding camp = HoldingNear(dealer.GetGM(), RZ_EFaction.BANDITS, e.m_vGiver, 1500);
		if (!camp)
			return false;

		Place(e, camp.m_vPos, camp.m_sName);
		e.m_sSubject = "holding:" + camp.m_iId.ToString() + ".shifts";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (w.m_iIndex == 0)
		{
			dealer.SmallFavor(playerId, RZ_EFaction.BANDITS, 5);
			dealer.SmallFame(playerId, RZ_EFaction.BANDITS, 1);
			return;
		}

		if (w.m_iIndex == 1)
			gm.GetFacts().Memory().Lies().Tell(gm, playerId, "errand.both_doors", e.m_sSubject, RZ_EFaction.BANDITS, "", e.m_vSubject, RZ_ELieTest.RUMOUR, 3,
				"The crews waited for a shift change that never came. They know who brought them the time.");
	}
}
