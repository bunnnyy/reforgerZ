//! Sign Here (the epilogue deck): Bastion's corporal needs three names from the queue on the roster
//! before the next shift. Walk up and take them, or write three names yourself (no walk; three in
//! ten the corporal notices: 4 Bastion Infamy). Under Bastion's roster or census, for players
//! Bastion knows.
class RZ_Errand_SignHere : RZ_EpiCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "epi_sign_here";
		m_sTitle = "Sign Here";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		Offer("The roster's short three names for the next shift at the barrier, and I can't hold it with what I've got. The queue's full of people with nothing to do but wait. Go and get me three volunteers, signed.",
			"I need three signatures from the queue before the shift changes: people willing to stand a shift at the barrier while they wait. Name, house, signature. No initials.");
		Over("Never mind. The queue sent three up on their own.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Take three names at the queue";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 30;
		w.m_fRadius = 40;
		w.m_sStep = "Walk the queue on the north road and get three volunteers' names and signatures (30 s there), then bring the sheet to {giver}.";
		w.m_iFavor = 5;
		w.m_iFame = 1;
		w.Done("Three, and legible. You'd be surprised how rare that is on a roster.");

		w = Way("sneak");
		w.m_sLabel = "Write three names yourself";
		w.m_iFavor = 5;
		w.Done("Three names. Fine. I'm not going to look up from the sheet. Fine.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return RZ_Reputation.IsAtLeast(dealer.GetGM().GetTier(playerId, RZ_EFaction.MILITARY), RZ_ETier.KNOWN);
	}

	//------------------------------------------------------------------------------------------------
	override bool WhyCard(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Epilogue epi = Week();
		if (!epi || Route() != RZ_CureFinale.BASTION || epi.Terms() == 3)
			return false;

		vector q = Queue();
		if (q == vector.Zero)
			return false;

		e.m_vSubject = q;
		e.m_sSubject = "epi.sign:" + RZ_SeasonClock.Day().ToString();
		e.SetText("subject_place", "the queue on the north road");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex != 1 || Math.RandomFloat01() >= 0.3)
			return;

		dealer.GetGM().AddInfamy(playerId, RZ_EFaction.MILITARY, 4, "forged three names on the roster", 0, "forged names on Bastion's roster");
		said.Insert("Tomorrow morning I read those three names out at the queue. If nobody answers to them, I'll know who wrote them.");
	}
}
