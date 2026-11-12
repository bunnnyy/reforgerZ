//! Buy It Back (docs/ERRANDS.md, fallout): Crash Site failed with the black box not handed in. A
//! fence's runner is sitting on it at the wreck. Bastion (Known) asks you to buy it back: five tins
//! handed over there, or 25 Red Hand Favor. Bastion pays at most 30% of Crash Site's base (the
//! fallout cap). Only as Crash Site's fallout, at a post within 3 km, its participants first.
class RZ_Errand_BuyItBack : RZ_ErrandCard
{
	protected static const int RH_FAVOR = 25;

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "buy_back";
		m_sTitle = "Buy It Back";
		m_sFamily = "aftermath";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		Offer("The black box from the helicopter wreck {where_place} never came in. A Red Hand fence's runner is sitting on it at the wreck, waiting for someone to name a price. Command will want to know what that box heard. Buy it back for us. Don't tell me how.");
		Over("Forget the box. It's gone where boxes go, and the log will say weather.");

		RZ_ErrandWay w = Way("give");
		w.m_sLabel = "Pay the runner in tins";
		w.m_eItem = RZ_EErrandItem.FOOD;
		w.m_iItems = 5;
		w.m_bGiveThere = true;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 30;
		w.m_sStep = "Take five tins to the fence's runner at the wreck {where_place}, then bring the black box to {giver}.";
		w.m_eProp = RZ_EErrandProp.PARTS;
		w.m_iFavor = 6;
		w.Done("That's the box. I won't ask what it cost you. In the log it'll say recovered, weather permitting.");

		w = Way("pay");
		w.m_sLabel = "Pay the runner in crew money [25 Red Hand Favor]";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 30;
		w.m_sStep = "Settle with the fence's runner at the wreck {where_place} on the crew's account (25 Red Hand Favor), then bring the black box to {giver}.";
		w.m_eProp = RZ_EErrandProp.PARTS;
		w.m_iFavor = 6;
		w.Done("I don't want to know whose account it went on. The box is back. That's all I asked.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return RZ_Reputation.IsAtLeast(dealer.GetGM().GetTier(playerId, RZ_EFaction.MILITARY), RZ_ETier.KNOWN);
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Fallout fo = dealer.FalloutFor(m_sKey, e.m_vGiver, e.m_iAsker);
		if (!fo)
			return false;

		e.m_bFallout = true;
		e.m_iCap = fo.cap;
		e.m_vSubject = RZ_Util.GroundPos(fo.at);
		e.m_sSubject = "wreck:" + RZ_SettlementScanner.GridReference(e.m_vSubject);
		e.SetText("where_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
		e.SetText("subject_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Offers(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w)
	{
		if (w.m_iIndex != 1)
			return true;

		return dealer.GetGM().GetProfile(e.m_iAsker).GetFavor(RZ_EFaction.BANDITS) >= RH_FAVOR;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (w.m_iIndex == 1)
		{
			int favor = Math.Min(RH_FAVOR, gm.GetProfile(playerId).GetFavor(RZ_EFaction.BANDITS));
			if (favor > 0)
				gm.SpendFavor(playerId, RZ_EFaction.BANDITS, favor);
		}

		array<int> who = {playerId};
		RZ_WorldFacts.Get().Memory().Record("crash.box", e.m_sSubject, "bought_back", RZ_WorldMemory.SOURCE_ERRAND, false, who, "", "");
	}
}
