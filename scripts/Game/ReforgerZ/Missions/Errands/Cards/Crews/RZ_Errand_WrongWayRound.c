//! Wrong Way Round (docs/ERRANDS.md, the crews): Trusted with the Red Hand, a Haven house within
//! 1.5 km. The house's lads take tolls under painted red hands to keep raiders off, and painted them the
//! wrong way round. Tell the boss (they're "settled"), warn them off, or take a cut (a lie the house's
//! next count finds).
class RZ_Errand_WrongWayRound : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "wrong_way";
		m_sTitle = "Wrong Way Round";
		m_sFamily = "crews";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		Offer("\"Somebody's taking tolls on the road by {subject_place} under our hand. Painted it the wrong way round, too. Go and see who.\"",
			"\"A red hand on a gate by {subject_place}, thumb on the wrong side. Not ours. Find out whose.\"");
		Over("\"Never mind the gate. It's been settled.\"");

		RZ_ErrandWay w = Way("report");
		w.m_sLabel = "Look, and tell the boss who";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 60;
		w.m_sStep = "Watch the gate by {subject_place} (15 s there), then tell {giver} who.";
		w.m_iFavor = 4;
		w.m_iFame = 2;
		w.Done("\"Kids from the house. Right. They'll be settled.\" He doesn't say how.");

		w = Way("talk");
		w.m_sLabel = "Warn the lads off";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 60;
		w.m_sStep = "Warn the lads at the gate by {subject_place} to scrub it off (10 s there), then tell {giver} nobody was there.";
		w.m_iFame = 2;
		w.m_iPayFaction = RZ_EFaction.SURVIVORS;
		w.Done("\"Nobody there. Paint's gone. Fancy.\"");

		w = Way("lie");
		w.m_sLabel = "Take a cut of their tolls";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 60;
		w.m_sStep = "Take a cut from the lads at the gate by {subject_place} (10 s there), and tell {giver} it's nothing.";
		w.m_iFavor = 3;
		w.Done("\"Nothing. Good. I like nothing.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return RZ_Reputation.IsAtLeast(dealer.GetGM().GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.TRUSTED);
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		int house = HouseNear(gm, e.m_vGiver, 1500, -1, 0);
		if (house < 0)
			return false;

		RZ_Safehouse h = gm.GetSafehouses().Get(house);
		// Not in a town the crews hold: then the hand would be theirs
		if (h.m_iSettlement >= 0 && gm.GetFacts().TownOwner(h.m_iSettlement).IsHeldBy(RZ_EFaction.BANDITS))
			return false;

		Place(e, RZ_Mission.LandPointNear(h.m_vPos, 80, 150), h.m_sName);
		e.m_iValue = house;
		e.m_sSubject = RZ_WorldFacts.Safehouse(house) + ".tolls";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (w.m_iIndex == 0)
		{
			if (gm.GetSafehouses())
				gm.GetSafehouses().RemovePopulation(e.m_iValue, 1);

			gm.AddInfamy(playerId, RZ_EFaction.SURVIVORS, 5, "named a house's lads to the crews");
			return;
		}

		if (w.m_iIndex == 2)
			gm.GetFacts().Memory().Lies().Tell(gm, playerId, "errand.wrong_way", e.m_sSubject, RZ_EFaction.SURVIVORS, e.m_sGiver, e.m_vSubject, RZ_ELieTest.TICK, 5,
				e.Fill("{subject_place} counted its tolls. They came out short by a stranger's cut, and the lads remember the stranger."));
	}
}
