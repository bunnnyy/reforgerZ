//! Wildcard Money (docs/ERRANDS.md, the crews; the mixed picture): the crew's book-keeper trusts the
//! cut to someone the crews both fear and owe (Wildcard or Tolerated with the Red Hand). Carry it to
//! the next crew; skim a little (they expected it); or give it to a hungry Haven house nearby, which
//! the crew won't forget.
class RZ_Errand_WildcardMoney : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "wildcard_money";
		m_sTitle = "Wildcard Money";
		m_sFamily = "crews";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		Offer("The boss says nobody robs a lunatic. Carry the cut to the crew at {crew_place}. Don't look at me like that, it's a compliment.");
		Over("Never mind. They came for it themselves. Impatient lot.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Carry the cut to {crew_place}";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 40;
		w.m_sStep = "Carry the cut to the Red Hand crew at {crew_place} (at their fire), then come back to {giver}.";
		w.m_iFavor = 6;
		w.Done("All of it? Huh. The boss owes me a tin.");

		w = Way("do");
		w.m_sLabel = "Carry it and skim a little";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 40;
		w.m_sStep = "Carry the cut to the Red Hand crew at {crew_place} (at their fire), lighter by a bit, then come back to {giver}.";
		w.m_iFavor = 8;
		w.Done("Light. About as light as we guessed. You're predictable. That's worth something.");

		w = Way("do");
		w.m_sLabel = "Give it to the house at {house_place}";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT2;
		w.m_fSeconds = 5;
		w.m_fRadius = 40;
		w.m_iPayFaction = RZ_EFaction.SURVIVORS;
		w.m_sStep = "Take the cut to the hungry safehouse at {house_place} (at its gate) and leave it there.";
		w.m_iFame = 2;
		w.Done("You what? The whole cut, to the Haven? The boss will laugh. Then the boss will remember.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		RZ_ETier tier = dealer.GetGM().GetTier(playerId, RZ_EFaction.BANDITS);
		return tier == RZ_ETier.WILDCARD || tier == RZ_ETier.TOLERATED;
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Territory territory = gm.GetTerritory();
		if (!territory)
			return false;

		RZ_Holding crew;
		foreach (RZ_Holding h : territory.GetHoldings())
		{
			if (h.m_eFaction == RZ_EFaction.BANDITS && h.m_iId != e.m_iHolding && RZ_Util.DistXZ(h.m_vPos, e.m_vGiver) <= 1500)
			{
				crew = h;
				break;
			}
		}

		if (!crew)
			return false;

		e.m_vSubject = crew.m_vPos;
		e.m_sSubject = "holding:" + crew.m_iId.ToString() + ".cut";
		e.SetText("crew_place", crew.m_sName);
		e.SetText("subject_place", crew.m_sName);
		int house = HouseNear(gm, e.m_vGiver, 1500, -1, 1);
		if (house >= 0)
		{
			RZ_Safehouse sh = gm.GetSafehouses().Get(house);
			e.m_iValue = house;
			e.m_vSubject2 = sh.m_vPos;
			e.SetText("house_place", sh.m_sName);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Offers(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w)
	{
		return w.m_iIndex != 2 || e.m_iValue >= 0;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex != 2 || e.m_iValue < 0)
			return;

		RZ_GameModeComponent gm = dealer.GetGM();
		if (gm.GetSafehouses())
			gm.GetSafehouses().AddFood(e.m_iValue, 2);

		gm.AddInfamy(playerId, RZ_EFaction.BANDITS, 8, "gave the crew's cut away", RZ_RepFlag.ACT, "gave our cut to the Haven");
		gm.NotifyPlayer(playerId, RZ_Factions.GetName(RZ_EFaction.BANDITS), "+8 Infamy with the Red Hand.");
	}
}
