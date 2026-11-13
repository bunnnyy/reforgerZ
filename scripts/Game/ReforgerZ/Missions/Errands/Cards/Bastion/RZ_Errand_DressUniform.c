//! Dress Uniform (docs/ERRANDS.md, Bastion; backgrounds: the Soldier): Soldier and Known with
//! Bastion. The sergeant wants a dead lad's dress uniform folded for the locker, and you fold it right
//! while the private watches and learns. 4 Favor. In your own kit you're a soldier, not a disguise.
class RZ_Errand_DressUniform : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "dress_uniform";
		m_sTitle = "Dress Uniform";
		m_sFamily = "background";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		Offer("The sergeant wants a dress uniform folded for a dead lad's locker, the regulation way, so it's ready if his family ever comes for it. None of us know the fold. You were in, weren't you? You've got the walk.",
			"We lost a private this week. His dress uniform goes in his locker, folded properly, and the sergeant said find someone who knows how. I'm told that's you.");
		Over("Never mind. The sergeant did it himself. It took him an hour, and he wouldn't let anyone help.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Fold it the regulation way";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 30;
		w.m_fRadius = 15;
		w.m_sStep = "Fold the dead man's dress uniform at {here} the regulation way while the private watches (30 s).";
		w.Work(RZ_EWork.BENCH, "Fold the uniform", 12);
		w.m_sInside = "any";
		w.m_iFavor = 4;
		w.Done("Regulation fold. Regulation nothing else, these days. I'll do the next one myself. Nearly right, probably.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		return RZ_QuestApproach.HasBackground(gm, playerId, "soldier") && RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.MILITARY), RZ_ETier.KNOWN);
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		// A lad of theirs died lately near the post
		if (!FreshSignal("death", e.m_vGiver, 2000, RZ_SeasonClock.DayMinutes() / 60, -1))
			return false;

		e.m_sSubject = "holding:" + e.m_iHolding.ToString() + ".locker";
		return true;
	}
}
