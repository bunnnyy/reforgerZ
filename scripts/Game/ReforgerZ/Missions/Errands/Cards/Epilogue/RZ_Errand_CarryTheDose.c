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
		Offer("\"There's one in a cold box with {holdout}'s name on it. {holdout} won't come in. Somebody has to go out.\"",
			"\"Sealed, labelled, one dose. {holdout} is holding out in a village nobody lives in. Take it to the door.\"");
		Over("\"Never mind. {holdout} walked in this morning.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Carry the box to {holdout}";
		w.m_ePlace = RZ_EErrandPlace.EMPTY_VILLAGE;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_sStep = "Take the cold box to {holdout} in {place} (at the door, 15 s), then come back to {giver}.";
		w.m_iFavor = 6;
		w.m_iFame = 1;
		w.Done("\"Signed for? In that handwriting? Then it's done.\"",
			"\"Good. One less name on the list of people we couldn't reach.\"");

		w = Way("sneak");
		w.m_sLabel = "Open the box yourself";
		w.Done("\"The seal's broken. Don't tell me why. We'll find out anyway.\"");
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
			said.Insert("Inside: one dose, a label with somebody else's name, and nothing you need today.");
			return;
		}

		epi.Data().dosed.Insert(lifeKey);
		gm.GetInfection().UseAntidote(playerId);
		said.Insert("You use it there and then. Somewhere out past the last village, somebody is still waiting.");
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
