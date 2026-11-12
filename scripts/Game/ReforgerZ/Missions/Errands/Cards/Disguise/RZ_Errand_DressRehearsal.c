//! Dress Rehearsal (docs/ERRANDS.md, disguises): a Red Hand named person, Known with the Red Hand
//! and below Known with Bastion. A Bastion road post within 2 km. Walk up to it in Bastion kit, ask
//! the sentry when the shift changes (10 s at the barrier, passing as Bastion: the look at the
//! barrier, docs/REPUTATION.md "Borrowed kit"), and walk back. +2 Red Hand Fame, 10 Favor. Caught (not
//! passing while you're at the barrier): it's over, and the open channel laughs, without your name.
class RZ_Errand_DressRehearsal : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "dress_rehearsal";
		m_sTitle = "Dress Rehearsal";
		m_sFamily = "crews";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		Offer("There's a Bastion post on the road {where_place}, and we'd like to know when its shift changes. Walk up in their green, ask the sentry like you belong there, and walk back. If you can do that, so can our lads, when it matters.");
		Over("Heard it on the open channel: somebody in borrowed green at the barrier, and the whole post laughing. Leave it now.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Walk through in Bastion kit";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 20;
		w.m_sStep = "Walk up to the Bastion post {where_place} in Bastion kit, pass their look and ask the sentry when the shift changes (10 s at the barrier), then come back to {giver}.";
		w.m_iFavor = 10;
		w.m_iFame = 2;
		w.Done("They change at six and six? Thank you. We'll pick a time that isn't either.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (!gm.GetHostility() || !gm.GetHostility().GetDisguise().IsOn())
			return false;

		return RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.KNOWN)
			&& !RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.MILITARY), RZ_ETier.KNOWN);
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Checkpoints posts = dealer.GetGM().GetCheckpoints();
		if (!posts)
			return false;

		array<RZ_RoadPost> near = {};
		posts.PostsNear(RZ_EFaction.MILITARY, e.m_vGiver, 2000, near);
		if (near.IsEmpty())
			return false;

		e.m_vSubject = near[0].m_vPos;
		e.m_sSubject = "post:" + near[0].m_iId.ToString();
		e.SetText("where_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
		e.SetText("subject_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! At the barrier and not passing as Bastion: caught, and the job is over.
	override bool Still(RZ_Errands dealer, RZ_Errand e)
	{
		if (e.m_eState != RZ_EErrandState.HELD || e.m_bStepDone)
			return true;

		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(e.m_iAsker);
		if (!me || RZ_Util.DistXZ(me.GetOrigin(), e.m_vSubject) > 25)
			return true;

		RZ_GameModeComponent gm = dealer.GetGM();
		if (gm.GetHostility().GetDisguise().IsPassing(e.m_iAsker, RZ_EFaction.MILITARY))
			return true;

		gm.NotifyAllAs("Open channel", "All posts, be advised: somebody just tried to walk through our barrier in borrowed green. Sleeves too short, boots too clean. The whole post saw it. Bastion out.", RZ_ENoticeTag.EVENT);
		return false;
	}
}
