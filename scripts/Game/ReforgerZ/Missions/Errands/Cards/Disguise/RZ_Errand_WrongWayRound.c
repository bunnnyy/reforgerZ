//! Wrong Way Round (docs/ERRANDS.md, crews): a Haven house sits on a road the Red Hand doesn't hold
//! (no Red Hand holding within 1.5 km of it). The house's lads take tolls under painted red hands to
//! keep raiders off, and painted them the wrong way round. Look for yourself (10 s at the house), then:
//! tell the boss (the lads are "settled": the house loses a person, +2 Red Hand Fame, +5 Haven
//! Infamy); take a cut of their takings (6 Haven Favor, 35% the house finds out: +5 Haven Infamy);
//! warn them off (+2 Haven Fame); or recruit them (+2 Red Hand Fame, the house loses a person).
class RZ_Errand_WrongWayRound : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "wrong_way_round";
		m_sTitle = "Wrong Way Round";
		m_sFamily = "crews";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		Offer("\"Somebody's taking tolls on the road by {house} under our hand. Painted it backwards. Go and look, then tell me what you want done about it.\"",
			"\"Red hands on a road we don't run, by {house}. Thumbs on the wrong side. That's either an insult or children.\"");
		Over("\"The hands by {house} got painted over. Somebody else had a word.\"");

		RZ_ErrandWay w = Way("report");
		w.m_sLabel = "Tell the boss";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 60;
		w.m_sStep = "Look at the tolls on the road by {house} (10 s there), then tell {giver} who's running them.";
		w.m_iFame = 2;
		w.Done("\"Lads from the house. Right. They'll be settled.\" Nobody says what that means.");

		w = Way("take");
		w.m_sLabel = "Take a cut of their takings";
		w.m_iPayFaction = RZ_EFaction.SURVIVORS;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 60;
		w.m_sStep = "Find the lads on the road by {house} and take your share of the tolls (10 s there). Then tell {giver} it was nobody.";
		w.m_iFavor = 6;
		w.Done("\"Nobody? Fine. Paint's paint.\"");

		w = Way("talk");
		w.m_sLabel = "Warn them off";
		w.m_iPayFaction = RZ_EFaction.SURVIVORS;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 60;
		w.m_sStep = "Find the lads on the road by {house} and tell them to paint over the hands before the crews come (10 s there). Then tell {giver} it was nothing.";
		w.m_iFame = 2;
		w.Done("\"Nothing? Then it'll be gone by the time I look.\"");

		w = Way("press");
		w.m_sLabel = "Recruit them";
		w.m_iMinTier = RZ_ETier.TRUSTED;
		w.m_iTierFaction = RZ_EFaction.BANDITS;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 60;
		w.m_sStep = "Find the lads on the road by {house} and tell them the Red Hand has work for people who can hold a road (10 s there). Then bring {giver} the news.";
		w.m_iFame = 2;
		w.Done("\"Keen, are they? Good. Teach them which way round a hand goes.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return RZ_Reputation.IsAtLeast(dealer.GetGM().GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.TRUSTED);
	}

	//------------------------------------------------------------------------------------------------
	//! A lived-in house within 2 km with no Red Hand holding within 1.5 km of it.
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		int house = HouseNear(gm, e.m_vGiver, 2000, -1, 0);
		if (house < 0)
			return false;

		RZ_Safehouse h = gm.GetSafehouses().Get(house);
		RZ_Territory territory = gm.GetTerritory();
		if (!h || (territory && territory.FindHolding(RZ_EFaction.BANDITS, h.m_vPos, 1500)))
			return false;

		Place(e, h.m_vPos, h.m_sName);
		e.m_iValue = house;
		e.m_sSubject = RZ_WorldFacts.Safehouse(house) + ".hands";
		e.SetText("house", h.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Safehouses houses = gm.GetSafehouses();
		RZ_Safehouse h;
		if (houses)
			h = houses.Get(e.m_iValue);

		array<int> who = {playerId};
		switch (w.m_iIndex)
		{
			case 0:
			{
				gm.AddInfamy(playerId, RZ_EFaction.SURVIVORS, 5, "gave the house's lads to the crews", RZ_RepFlag.ACT);
				if (h && h.m_iPopulation > 2)
					houses.RemoveDead(e.m_iValue, 1);

				dealer.Notify(e, "{house} is one short at supper. Nobody asks where the lad went. Everybody knows. +5 Haven Infamy.");
				break;
			}
			case 1:
			{
				if (Math.RandomFloat01() < 0.35)
				{
					gm.AddInfamy(playerId, RZ_EFaction.SURVIVORS, 5, "took a cut of the house's tolls", RZ_RepFlag.ACT);
					dealer.Notify(e, "{house} counted the tolls and found them short. They know whose hand was in it. +5 Haven Infamy.");
				}

				break;
			}
			case 3:
			{
				if (h && h.m_iPopulation > 2)
					houses.RemoveDead(e.m_iValue, 1);

				said.Insert("One of the lads packs a bag that night. {house} lays one fewer place.");
				break;
			}
		}

		RZ_WorldFacts.Get().Memory().Record("errand.wrong_way_round", e.m_sSubject, w.m_sWay, RZ_WorldMemory.SOURCE_ERRAND, false, who);
	}
}
