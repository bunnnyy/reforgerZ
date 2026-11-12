//! Paint (docs/ERRANDS.md, crews; with Name Day's Brush): a Bastion road post stands at night within
//! 2.5 km of a Red Hand camp. "New faces paint a red hand on." Paint one on the barrier (20 s there):
//! in Bastion kit, asking the crew for a light (you must be passing as Bastion at the barrier, else
//! it's over); from the back, unseen; or in plain sight, which is a fight (Bastion's timed grudge and
//! +3 Bastion Infamy). +2 Red Hand Fame, a crew name for the life if you have none ("Brush"), and a
//! line on Bastion's board: "Civilian with a paint tin." The post comes down by itself; nothing stays.
class RZ_Errand_Paint : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "paint";
		m_sTitle = "Paint";
		m_sFamily = "crews";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		m_bTimed = true;
		Offer("New faces paint a red hand on, and you're a new face. Bastion has a post on the road {where_place} tonight. Their barrier, our hand, in red paint, and you walk back. Red hand means stop. They'll get the joke.",
			"Here's a tin of red. There's a Bastion post on the road {where_place}, and its barrier hasn't got our hand on it yet. Everybody in the crews does it once. Tonight it's you.");
		Over("The Bastion post packed up and went. Paint a fence. Paint your boots. Doesn't count.");

		RZ_ErrandWay w = Way("trick");
		w.m_sLabel = "In their kit: ask the crew for a light";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 20;
		w.m_fRadius = 20;
		w.m_sStep = "Walk up to the Bastion post {where_place} in Bastion kit, pass their look, ask for a light and paint the hand on the barrier while they smoke (20 s there). Then come back to {giver}.";
		w.m_iFame = 2;
		w.Done("In their own green, with their own match? That's going round every fire tonight.");

		w = Way("sneak");
		w.m_sLabel = "From the back, unseen";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 20;
		w.m_fRadius = 25;
		w.m_bUnseen = true;
		w.m_sStep = "Get to the back of the Bastion post {where_place} without being seen and paint the hand on the barrier (20 s there, unseen). Then come back to {giver}.";
		w.m_iFame = 2;
		w.Done("They'll find it at the shift change. I'd pay to see the sergeant's face.");

		w = Way("press");
		w.m_sLabel = "In plain sight";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 20;
		w.m_fRadius = 25;
		w.m_sStep = "Walk up to the Bastion post {where_place} and paint the hand on the barrier in front of them (20 s there). They won't like it. Then come back to {giver}.";
		w.m_iFame = 2;
		w.Done("In front of them, under fire? You're mad. Sit down, have a drink, and keep your head down near that post for a while.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		return gm.GetHostility() && gm.GetHostility().GetDisguise().IsOn() && RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.KNOWN);
	}

	//------------------------------------------------------------------------------------------------
	//! At night, a Bastion road post within 2.5 km of the camp.
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		float hour = RZ_GameTime.GetHour();
		if (hour >= 5 && hour < 20.5)
			return false;

		RZ_Checkpoints posts = dealer.GetGM().GetCheckpoints();
		if (!posts)
			return false;

		array<RZ_RoadPost> near = {};
		posts.PostsNear(RZ_EFaction.MILITARY, e.m_vGiver, 2500, near);
		if (near.IsEmpty())
			return false;

		Place(e, near[0].m_vPos, "");
		e.m_iValue = near[0].m_iId;
		e.m_sSubject = "post:" + near[0].m_iId.ToString();
		e.SetText("where_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The post still stands; the trick: at the barrier and not passing as Bastion, it's over.
	override bool Still(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (!gm.GetCheckpoints() || !gm.GetCheckpoints().FindPost(e.m_iValue))
			return false;

		if (e.m_eState != RZ_EErrandState.HELD || e.m_bStepDone || e.m_iWay != 0)
			return true;

		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(e.m_iAsker);
		if (!me || RZ_Util.DistXZ(me.GetOrigin(), e.m_vSubject) > 25)
			return true;

		return gm.GetHostility().GetDisguise().IsPassing(e.m_iAsker, RZ_EFaction.MILITARY);
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_RoadPost post = gm.GetCheckpoints().FindPost(e.m_iValue);
		string place = "the road";
		if (post)
			place = post.m_sNear;

		// In plain sight it's a fight: their timed grudge, and Infamy for the act
		if (w.m_iIndex == 2)
		{
			gm.AddInfamy(playerId, RZ_EFaction.MILITARY, 3, "painted a red hand on a checkpoint", RZ_RepFlag.ACT);
			if (gm.GetHostility())
				gm.GetHostility().AddGrudge(playerId, RZ_EFaction.MILITARY, e.m_vSubject);
		}

		RZ_PlayerProfile p = gm.GetProfile(playerId);
		if (p.m_sCrewName.IsEmpty() && RZ_ReputationSettings.Get().m_bCrewNames)
		{
			p.m_sCrewName = "Brush";
			said.Insert("Somebody at the fire's already calling you Brush. It'll stick. Crew names stick like paint.");
			RZ_Log.Info(string.Format("Reputation: crew name Brush for %1 (Paint)", RZ_Util.PlayerName(playerId)));
		}

		gm.GetHostility().GetDisguise().GetMarks().AddBoard(RZ_EFaction.MILITARY, place, "a civilian with a paint tin", "");
		array<int> who = {playerId};
		RZ_WorldFacts.Get().Memory().Record("errand.paint", e.m_sSubject, w.m_sWay, RZ_WorldMemory.SOURCE_ERRAND, false, who);
	}
}
