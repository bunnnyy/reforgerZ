//! The Deserter's Way Round (docs/ERRANDS.md, road posts): a Bastion road post stands on the
//! Deserter's road, and he likes you. Meet him past the post, the long way through the wild, or turn
//! him in at the barrier. A disguise clears nothing: he's searched as himself.
class RZ_Errand_DesertersWay : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "deserters_way";
		m_sTitle = "The Deserter's Way Round";
		m_sFamily = "line";
		m_eAnchor = RZ_EErrandAnchor.WANDERER;
		m_iWandererRole = RZ_EWandererRole.DESERTER;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("\"There's a post on my road. They'll know my face, and the list says I'm dead. Walk the long way round with me? Meet me past it.\"");
		Over("\"Post's gone. I'll take the road like a citizen.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Meet him past the post";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 40;
		w.m_sStep = "Go round the post through the wild and wait for him {where_place} (10 s there), then find {giver} again.";
		w.m_iFavor = 4;
		w.Done("\"Past it. I owe you. Don't collect too soon, I'm skint.\"");

		w = Way("report");
		w.m_sLabel = "Turn him in at the post";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT2;
		w.m_fSeconds = 5;
		w.m_fRadius = 30;
		w.m_iPayFaction = RZ_EFaction.MILITARY;
		w.m_iFame = 2;
		w.m_sStep = "Walk to the Bastion post {post_place} and give his name at the barrier (5 s there).";
		w.Done("\"Name taken. He'll be on the roll by morning, struck off or not.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Wanderers wanderers = gm.GetWanderers();
		RZ_Checkpoints posts = gm.GetCheckpoints();
		RZ_Wanderer w = wanderers.FindById(e.m_iWanderer);
		if (!w || !posts || wanderers.AttitudeOf(w, e.m_iAsker) < 1)
			return false;

		array<RZ_RoadPost> near = {};
		posts.PostsNear(RZ_EFaction.MILITARY, w.m_vPos, 1500, near);
		if (near.IsEmpty())
			return false;

		RZ_RoadPost post = near[0];
		vector away = post.m_vPos - w.m_vPos;
		away[1] = 0;
		float len = away.Length();
		if (len < 50)
			return false;

		away = away * (250 / len);
		e.m_vSubject = RZ_Mission.LandPointNear(post.m_vPos + away, 0, 40);
		e.m_vSubject2 = post.m_vPos;
		e.m_sSubject = RZ_WorldFacts.Wanderer(w.m_iId);
		e.SetText("where_place", RZ_Util.DirectionText(w.m_vPos, e.m_vSubject));
		e.SetText("post_place", RZ_Util.DirectionText(w.m_vPos, post.m_vPos));
		e.SetText("subject_place", RZ_Util.DirectionText(w.m_vPos, e.m_vSubject));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Still(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Wanderers wanderers = dealer.GetGM().GetWanderers();
		return wanderers && wanderers.IsAlive(e.m_iWanderer);
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex != 1)
			return;

		// Turned in: the Haven's wanderers say who
		RZ_GameModeComponent gm = dealer.GetGM();
		gm.GetFacts().AddRumour("Bastion picked up a deserter on the road. Somebody walked him to the barrier.", 6);
		array<int> who = {playerId};
		RZ_WorldFacts.Get().Memory().Record("errand.deserter", e.m_sSubject, "turned in", RZ_WorldMemory.SOURCE_ERRAND, true, who);
	}
}
