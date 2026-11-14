//! Borrowed Green (docs/ERRANDS.md, help across lines; a disguise card): a bitten person at a house,
//! a Bastion road post between the house and the lab. Walk the family past it for the trial dose; on
//! this card Bastion turns the visibly bitten back at its barrier (containment). Sneak round the post
//! (more than 150 m off), ask the sergeant straight (persuade 1 with Bastion), walk through in Bastion
//! kit ("Casualty for the lab, sergeant's orders": you must pass their look at the barrier), or, with
//! disguises off, pay their way (15 Bastion Favor). +2 Haven Fame, 8 Favor. If the dose takes, they
//! stay at the house (+1 person).
class RZ_Errand_BorrowedGreen : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "borrowed_green";
		m_sTitle = "Borrowed Green";
		m_sFamily = "across";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("There's a man in our back room with a bite on his arm. He declared it at supper, the way the house asks. The lab gives the bitten one trial dose for nothing, and eight in ten walk out, but Bastion's post on the lab road turns the bitten back. Get him and his family past it. I don't much mind how.",
			"We've a family here with a bite among them, and a day or two before it decides for them. The lab's trial dose is his chance. Between us and the lab there's a green post that sends the bitten home. Walk them through it, or round it, or anything.");
		Over("It's too late for the lab now. Or it isn't needed any more. Don't make me say which.");

		RZ_ErrandWay w = Way("sneak");
		w.m_sLabel = "Walk them round the post";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT2;
		w.m_fSeconds = 10;
		w.m_fRadius = 40;
		w.m_sStep = "Walk the family round the Bastion post {subject_place}, well off the road and more than 150 m from the barrier (10 s there), then come back to {giver}.";
		w.Done("They're past the post and on the lab road. Thank you. His brother is carrying his bag and he's telling him off for it, which is a good sign.");

		w = Way("talk");
		w.m_sLabel = "Ask the sergeant straight";
		w.m_iTrust = 1;
		w.m_iTrustFaction = RZ_EFaction.MILITARY;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 25;
		w.m_sStep = "Take the family to the barrier and ask the sergeant to let them through to the lab (10 s there), then come back to {giver}.";
		w.Done("The sergeant let them through, just for asking? Well. There are still a few of them like that, then.");

		w = Way("trick");
		w.m_sLabel = "Walk them through in Bastion kit";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 20;
		w.m_sStep = "Walk the family up to the barrier in Bastion kit and tell the sentry it's a casualty for the lab, on the sergeant's orders. Pass their look (10 s at the barrier), then come back to {giver}.";
		w.Done("Sergeant's orders, you told them. Which sergeant? You. He laughed when the family told him that, the first time since the bite.");

		w = Way("pay");
		w.m_sLabel = "Pay their way through";
		w.m_iFavorCost = 15;
		w.m_iPayFaction = RZ_EFaction.MILITARY;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 25;
		w.m_sStep = "Take the family to the barrier, where your Favor with Bastion gets them through (10 s there), then come back to {giver}.";
		w.Done("That cost you with Bastion. I know it did, and we can't pay it back. So thank you, and I mean it.");
	}

	//------------------------------------------------------------------------------------------------
	protected static bool DisguisesOn(RZ_GameModeComponent gm)
	{
		return gm.GetHostility() && gm.GetHostility().GetDisguise().IsOn();
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (!gm.HasLab() || (gm.GetCure() && gm.GetCure().IsCured()))
			return false;

		RZ_Reason bitten = RZ_WorldFacts.Get().Memory().FindReason("house.bitten", RZ_WorldFacts.Safehouse(e.m_iHouse));
		if (!bitten)
			return false;

		RZ_Checkpoints posts = gm.GetCheckpoints();
		if (!posts)
			return false;

		vector lab = gm.GetLabPosition();
		array<RZ_RoadPost> near = {};
		posts.PostsNear(RZ_EFaction.MILITARY, e.m_vGiver, 1500, near);
		foreach (RZ_RoadPost p : near)
		{
			if (RZ_Strangers.DistToSegment(p.m_vPos, e.m_vGiver, lab) > 300)
				continue;

			Place(e, p.m_vPos, "");
			vector side = Vector(p.m_vPos[2] - e.m_vGiver[2], 0, e.m_vGiver[0] - p.m_vPos[0]);
			side.Normalize();
			e.m_vSubject2 = RZ_Mission.LandPointNear(p.m_vPos + side * 180, 0, 25);
			e.m_sSubject = RZ_WorldFacts.Safehouse(e.m_iHouse) + ".lab_walk";
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool Offers(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w)
	{
		bool on = DisguisesOn(dealer.GetGM());
		if (w.m_iIndex == 2)
			return on;

		if (w.m_iIndex == 3)
			return !on;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The trick: at the barrier and not passing as Bastion, they're turned back and it's over.
	override bool Still(RZ_Errands dealer, RZ_Errand e)
	{
		if (e.m_eState != RZ_EErrandState.HELD || e.m_bStepDone || e.m_iWay != 2)
			return true;

		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(e.m_iAsker);
		if (!me || RZ_Util.DistXZ(me.GetOrigin(), e.m_vSubject) > 25)
			return true;

		return dealer.GetGM().GetHostility().GetDisguise().IsPassing(e.m_iAsker, RZ_EFaction.MILITARY);
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		dealer.SmallFavor(playerId, RZ_EFaction.SURVIVORS, 8);
		dealer.SmallFame(playerId, RZ_EFaction.SURVIVORS, 2);
		RZ_WorldFacts.Get().Memory().ClearReason("house.bitten", RZ_WorldFacts.Safehouse(e.m_iHouse));
		array<int> who = {playerId};
		string took = "didn't take";
		if (Math.RandomFloat01() < 0.5)
		{
			took = "took";
			if (gm.GetSafehouses())
				gm.GetSafehouses().AddPopulationAt(e.m_iHouse, 1);

			said.Insert("Word came back from the lab: the dose took. He'll be home at {house} by the end of the week, thin and furious and alive.");
		}
		else
		{
			said.Insert("Word came back from the lab: the dose didn't take. They let the family sit with him at the end. That's something the lab does now.");
		}

		RZ_WorldFacts.Get().Memory().Record("errand.borrowed_green", e.m_sSubject, took, RZ_WorldMemory.SOURCE_ERRAND, false, who,
			"A family walked a bitten man past a green post to the lab. The house still argues about how.", "voice");
	}
}
