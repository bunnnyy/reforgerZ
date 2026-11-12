//! Civvies (docs/ERRANDS.md, help across lines; the disguise in reverse): a deserter struck off
//! Bastion's strength as KIA, the name still on the board, shelters at a house with a Bastion holding
//! within 3 km. A seeded name, never the finale's canon deserter. Bring civilian clothes from the next
//! town (20 s there): +2 Haven Fame, and the deserter stays as one of the house (+1 person). Or sell
//! where the deserter sleeps to the Red Hand (10 Red Hand Favor): gone by morning. What happens after
//! is Struck Off's story, not this card's; it never overlaps Desertion Route.
class RZ_Errand_Civvies : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "civvies";
		m_sTitle = "Civvies";
		m_sFamily = "across";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("There's someone sleeping in the back who used to be a soldier. {deserter}. The board at the post says KIA. Help them stop looking like one.",
			"{deserter} came in wearing green and won't take it off, because there's nothing else. Find something that isn't anybody's colours.");
		Over("{deserter} left before light. Took the bread and left the jacket.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Bring civilian clothes";
		w.m_ePlace = RZ_EErrandPlace.NEXT_TOWN;
		w.m_fSeconds = 20;
		w.m_fRadius = 60;
		w.m_sStep = "Find clothes in {place} that aren't anybody's colours (20 s there), then bring them to {giver}.";
		w.m_iFame = 2;
		w.Done("\"Look at that. Just a person.\" {deserter} folds the green and puts it under the stove.");

		w = Way("betray");
		w.m_sLabel = "Sell where they sleep to the Red Hand";
		w.m_iMinTier = RZ_ETier.KNOWN;
		w.m_iTierFaction = RZ_EFaction.BANDITS;
		w.m_iPayFaction = RZ_EFaction.BANDITS;
		w.m_ePlace = RZ_EErrandPlace.OUT;
		w.m_fSeconds = 10;
		w.m_fRadius = 30;
		w.m_sStep = "Meet a crew's runner {place} and say where {deserter} sleeps (10 s there). Then go back to {giver} as if nothing happened.";
		w.m_iFavor = 10;
		w.Done("\"You'll keep it to yourself, I know.\" By morning the back room is empty.");
	}

	//------------------------------------------------------------------------------------------------
	//! A house with a Bastion holding within 3 km; one deserter per house a season.
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Holding h = HoldingNear(gm, RZ_EFaction.MILITARY, e.m_vGiver, 3000);
		if (!h || e.m_iHouse < 0)
			return false;

		RZ_Rng rng = new RZ_Rng(RZ_WorldFacts.Get().GetSeed() + e.m_iHouse * 613 + 29);
		string name = RZ_FactionVoice.PersonName(rng, RZ_EFaction.MILITARY);
		if (name.Contains("Fiala"))
			return false;

		e.m_sSubject = RZ_WorldFacts.Safehouse(e.m_iHouse) + ".deserter";
		e.SetText("deserter", name);
		e.m_sValue = name;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		array<int> who = {playerId};
		if (w.m_iIndex == 0)
		{
			if (gm.GetSafehouses())
				gm.GetSafehouses().AddPopulationAt(e.m_iHouse, 1);

			RZ_WorldFacts.Get().Memory().Record("errand.civvies", e.m_sSubject, e.m_sValue, RZ_WorldMemory.SOURCE_ERRAND, false, who,
				"Someone at the house still folds a green jacket on Sundays and puts it back under the stove.", "voice");
			return;
		}

		RZ_WorldFacts.Get().AddRumour(e.Fill("Heard the crews took somebody out of {house} in the night. Somebody who used to wear green."), 6);
		RZ_WorldFacts.Get().Memory().Record("errand.civvies", e.m_sSubject, "sold", RZ_WorldMemory.SOURCE_ERRAND, false, who);
		RZ_Log.Info(string.Format("Errands: civvies sold to the Red Hand by %1", RZ_Util.PlayerName(playerId)));
	}
}
