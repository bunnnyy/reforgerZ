//! Carry the Dose (the epilogue deck): the route's own people send one sealed dose to a holdout who
//! won't come in from an empty village. Carry it, or open it yourself: a dose for you if you're
//! bitten (one outlet dose a life, as the outlets count), none for the holdout, 10 Infamy with the
//! people who packed it. Only where the route gives the cure out (its outlets).
class RZ_Errand_CarryTheDose : RZ_EpiCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "epi_carry_dose";
		m_sTitle = "Carry the Dose";
		m_eAnchor = RZ_EErrandAnchor.ANY_HOLDING;
		Offer("The cure's going out, and there's one dose in a cold box here with {holdout}'s name on it. {holdout} won't come in, not for the queue, not for us, not for anybody. So somebody has to go out with it, and it can't be me.",
			"One dose, sealed and labelled for {holdout}, who is holding out in a village nobody else lives in. Take the cold box to the door and see it signed for. Don't open it.");
		Over("Never mind. {holdout} walked in this morning, and took the dose here.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Carry the box to {holdout}";
		w.m_ePlace = RZ_EErrandPlace.EMPTY_VILLAGE;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_sStep = "Take the cold box to {holdout}'s door in {place} and see it signed for (15 s there), then come back to {giver}.";
		w.m_iFavor = 6;
		w.m_iFame = 1;
		w.Done("Signed for? In that handwriting? Then it's done, and that's one we didn't lose.",
			"Good. One less name on the list of people we couldn't reach. It's a long list.");

		w = Way("sneak");
		w.m_sLabel = "Open the box yourself";
		w.Done("The seal's broken. Don't tell me why. We'll find out anyway, and so will {holdout}.");
	}

	//------------------------------------------------------------------------------------------------
	override bool WhyCard(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Epilogue epi = Week();
		if (!epi || epi.Phase() >= 3 || epi.GetOutlets().GetList().IsEmpty())
			return false;

		if (e.m_iFaction != epi.Faction())
			return false;

		array<string> holdouts = {"Mr Dvorak", "the Bartos sisters", "a man called Hruby", "Mrs Kolar", "the Zeman lad", "old Pospisil"};
		int pick = RZ_Util.PickIndex(e.m_sGiver.Hash(), holdouts.Count());
		e.SetText("holdout", holdouts[pick]);
		e.m_sSubject = "epi.dose:" + e.m_sGiver;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_Epilogue epiDone = Week();
		if (epiDone)
			epiDone.GetRest().HoldoutDone(e.m_iId);

		if (w.m_iIndex != 1)
			return;

		RZ_GameModeComponent gm = dealer.GetGM();
		gm.AddInfamy(playerId, e.m_iFaction, 10, "opened a dose meant for a holdout", 0, "kept a holdout's dose");
		RZ_Epilogue epi = Week();
		RZ_PlayerProfile p = gm.GetProfile(playerId);
		string lifeKey = string.Format("%1|%2", p.m_sIdentity, p.m_iLives);
		if (!epi || !gm.GetInfection().SeemsInfected(playerId) || epi.Data().dosed.Contains(lifeKey))
		{
			said.Insert("One dose, a label with somebody else's name on it, and nothing you needed today. You opened it for nothing.");
			return;
		}

		epi.Data().dosed.Insert(lifeKey);
		gm.GetInfection().UseAntidote(playerId);
		said.Insert("You used it yourself, didn't you. Out past the last village, somebody is still waiting for that box.");
	}

	//------------------------------------------------------------------------------------------------
	//! The holdout stands at the door while the box is on its way (the epilogue's spots).
	override bool Still(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Epilogue epi = Week();
		if (epi && e.m_eState == RZ_EErrandState.HELD && e.m_iWay == 0 && e.m_vPlace != vector.Zero)
			epi.GetRest().Holdout(e.m_iId, e.GetText("holdout"), e.m_vPlace);

		return true;
	}
}
