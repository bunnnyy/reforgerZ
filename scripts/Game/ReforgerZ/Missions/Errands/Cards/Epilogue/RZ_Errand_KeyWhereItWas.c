//! The Key Where It Was (the epilogue deck): a new face read Petr's note in the empty kitchen (the
//! newcomer's scenario). A Haven named person knows Petr: he's in the queue, and he forgot the key.
//! Take it up to him, or use it yourself: what's in his larder, and 5 Haven Infamy. Before
//! processing, while the queue stands.
class RZ_Errand_KeyWhereItWas : RZ_EpiCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "epi_key";
		m_sTitle = "The Key Where It Was";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("You read Petr's note on the door? That was his kitchen. He's gone up to the processing queue on the north road without his key, and it's still under the step where he always leaves it. There are photos in that house he'll want.",
			"Petr went up the north road to the queue without his key. If it's still under the step, he'd want it. His wife's photos are in that house, and the gates don't open twice.");
		Over("Never mind. Petr's neighbour took the key up to him.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Take the key up to Petr";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 40;
		w.First(RZ_EErrandPlace.HERE, "Take Petr's key from under his step (5 s there).", 5);
		w.m_sStep = "Carry the key up to Petr in the queue on the north road (10 s there), then come back to {giver}.";
		w.m_iFavor = 5;
		w.m_iFame = 1;
		w.Done("He took it? He'll hold onto it the whole way out. That's Petr.",
			"Good. A man should have his own key, even in a queue, even leaving.");

		w = Way("sneak");
		w.m_sLabel = "Use the key yourself";
		w.m_ePlace = RZ_EErrandPlace.EMPTY_VILLAGE;
		w.m_fSeconds = 30;
		w.m_fRadius = 40;
		w.m_sStep = "Let yourself into Petr's house in {place} with his key (30 s inside), then come back to {giver}.";
		w.Done("You went in. With his key. Petr would have shared, probably. That's not the point.");
	}

	//------------------------------------------------------------------------------------------------
	override bool WhyCard(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Epilogue epi = Week();
		vector q = Queue();
		if (!epi || epi.Phase() >= 3 || q == vector.Zero)
			return false;

		RZ_PlayerProfile p = dealer.GetGM().GetProfile(e.m_iAsker);
		if (!p || !epi.GetRest().ReadKitchen(p.m_sIdentity))
			return false;

		e.m_vSubject = q;
		e.m_sSubject = "epi.key:" + p.m_sIdentity;
		e.SetText("subject_place", "the queue on the north road");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex != 1)
			return;

		RZ_GameModeComponent gm = dealer.GetGM();
		gm.AddInfamy(playerId, RZ_EFaction.SURVIVORS, 5, "let themselves into a queued man's house", 0, "used Petr's key");
		said.Insert("His two tins and his plum jam, I expect. At least tell me you left the photos.");
	}
}
