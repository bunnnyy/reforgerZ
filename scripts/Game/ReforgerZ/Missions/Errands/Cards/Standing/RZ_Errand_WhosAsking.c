//! Who's Asking (docs/ERRANDS.md, the bad-standing deck): a Rogue leader's corporal, for a Haven
//! exile, a Rogue leader of the giver's faction within 2 km. "The boss doesn't care what the Haven
//! thinks of you. Neither do I." Carry a crate of tins to a Haven house's back door: +2 food there, 6 of
//! the giver faction's Favor. Seen by the Haven: +4 Haven Infamy. The exile stays an exile.
class RZ_Errand_WhosAsking : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "whos_asking";
		m_sTitle = "Who's Asking";
		m_sFamily = "standing";
		m_eAnchor = RZ_EErrandAnchor.ANY_HOLDING;
		Offer("The boss doesn't care what the Haven thinks of you. Neither do I. A crate of tins for the back door at {subject_place}. Don't let them see who brought it.",
			"You're out with the Haven. Handy. The boss sends them tins they're not supposed to know about. Take them round the back.");
		Over("Never mind the crate. The boss changed his mind. He does that.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Take the tins round the back";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 30;
		w.m_sStep = "Leave the crate at the back door of the house at {subject_place} without being seen (10 s there), then come back to {giver}.";
		w.m_iFavor = 6;
		w.Done("Good. The boss likes them fed. Fed people don't come asking.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		RZ_Safehouses houses = dealer.GetGM().GetSafehouses();
		return houses && houses.IsExiled(playerId) && giver.m_iFaction != RZ_EFaction.SURVIVORS;
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Leaders leaders = gm.GetLeaders();
		if (!leaders)
			return false;

		RZ_Leader rogue = leaders.FindNearest(e.m_iFaction, e.m_vGiver, 2000);
		if (!rogue || rogue.m_eLoyalty != RZ_ELoyalty.ROGUE)
			return false;

		int house = HouseNear(gm, e.m_vGiver, 1500, -1, 0);
		if (house < 0)
			return false;

		RZ_Safehouse h = gm.GetSafehouses().Get(house);
		Place(e, RZ_Mission.LandPointNear(h.m_vPos, 15, 25), h.m_sName);
		e.m_iValue = house;
		e.m_sSubject = RZ_WorldFacts.Safehouse(house) + ".backdoor";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (gm.GetSafehouses())
			gm.GetSafehouses().AddFood(e.m_iValue, 2);

		if (RZ_Witness.Seen(gm, playerId, e.m_vSubject, RZ_EFaction.SURVIVORS))
		{
			gm.AddInfamy(playerId, RZ_EFaction.SURVIVORS, 4, "came to the back door as an exile", 0, "came round a Haven back door while exiled");
			said.Insert("Someone at the house saw you at the door. They kept the tins. They didn't keep quiet.");
		}
	}
}
