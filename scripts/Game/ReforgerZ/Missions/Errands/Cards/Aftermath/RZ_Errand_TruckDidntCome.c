//! The Truck Didn't Come (docs/ERRANDS.md, aftermath): this house's Haven truck was lost within
//! 1.5 km in the last 2 played hours (the signal ring), and the next isn't in. Salvage what the
//! wreck still holds, or (Known with the Red Hand) ask a toll lad who did it, which becomes a rumour
//! naming the crew, never a player. The arsonist rule: never dealt to whoever robbed it, or their group.
class RZ_Errand_TruckDidntCome : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "truck_didnt_come";
		m_sTitle = "The Truck Didn't Come";
		m_sFamily = "aftermath";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "storekeeper";
		m_iNeed = RZ_ENeed.FOOD;
		Offer("Our truck never came in. It's on its side {where_place}, by the sound of it, and the driver's gone. Somebody robbed it first. Whatever's left in the back is the house's flour for the week, if you're quick.",
			"The house's supply truck is lying {where_place}. The driver's gone. The sacks might not be, and we're down to oats and hope.");
		Over("Leave it. The next truck came in. We'll eat.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Salvage what's left of the load";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 30;
		w.m_fRadius = 40;
		w.m_sStep = "Search the wrecked truck {where_place} for what the robbers left (30 s there), then bring it to {giver}.";
		w.m_eProp = RZ_EErrandProp.CRATE;
		w.m_iFavor = 4;
		w.Done("Two sacks, and one of them's flour. The other's flour too. That's bread for the week. We'll manage.");

		w = Way("talk");
		w.m_sLabel = "Ask a toll lad who did it";
		w.m_iMinTier = RZ_ETier.KNOWN;
		w.m_iTierFaction = RZ_EFaction.BANDITS;
		w.m_iFavor = 3;
		w.Done("A Red Hand crew did it, you say. Knowing doesn't fill a pot. It helps, though. I'll tell the next driver which road to avoid.");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Signal s = FreshSignal(TruckKind(RZ_EFaction.SURVIVORS), e.m_vGiver, 1500, 2, e.m_iAsker);
		if (!s || s.text != e.GetText("house"))
			return false;

		e.m_vSubject = Vector(s.x, 0, s.z);
		e.m_vSubject = RZ_Util.GroundPos(e.m_vSubject);
		e.m_sSubject = RZ_WorldFacts.Safehouse(e.m_iHouse);
		e.SetText("where_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
		e.SetText("subject_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (w.m_iIndex == 0 && gm.GetSafehouses())
			gm.GetSafehouses().AddFood(e.m_iHouse, 2);

		if (w.m_iIndex == 1)
			gm.GetFacts().AddRumour(string.Format("A Red Hand crew hit the Haven truck for %1. Nobody's saying which crew. Everybody knows.", e.GetText("house")), 6);
	}
}
