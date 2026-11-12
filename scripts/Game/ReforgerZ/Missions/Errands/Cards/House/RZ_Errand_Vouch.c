//! Vouch (docs/ERRANDS.md, the house; the two Vouch cards merged, one vouch mechanic):
//! - at a Haven door (Known with the Haven): a newcomer needs someone Known to speak for them. With
//!   the card's twist they steal a bit within the day: the voucher answers for it (+5 Haven Infamy);
//! - at a Red Hand camp (Trusted with the Red Hand; an Ex-Con knows him from inside): a new man. One
//!   time in three he's selling to Bastion, found out by events (a lie-store record on the voucher),
//!   and the crew remembers who vouched for him.
//! No pay. Writes `vouched.<name>`.
class RZ_Errand_Vouch : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "vouch";
		m_sTitle = "Vouch";
		m_sFamily = "house";
		m_eAnchor = RZ_EErrandAnchor.ANY_HOLDING;
		m_bTwists = true;
		Offer("There's someone at the door who needs someone we know to speak for them. That's you, if you'll do it. If they turn out wrong, it's on you too.",
			"New face, {name}. Nobody here knows them. Somebody has to say they're all right, and mean it.");
		Over("Never mind. {name} went on. Or someone else spoke up. Either way.");

		RZ_ErrandWay w = Way("talk");
		w.m_sLabel = "Speak for {name}";
		w.Done("Right. Your word, then. In you come, {name}.");

		w = Way("talk");
		w.m_sLabel = "Ask about inside first";
		w.m_sBackground = "excon";
		w.Done("Well? You'd know better than me.");

		w = Way("leave");
		w.m_sLabel = "Don't vouch";
		w.Done("Fair. Most don't. I wouldn't.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (giver.m_iFaction == RZ_EFaction.SURVIVORS)
			return giver.m_eKind == RZ_ENamedPlace.HOUSE && RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.SURVIVORS), RZ_ETier.KNOWN);

		if (giver.m_iFaction == RZ_EFaction.BANDITS)
			return RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.TRUSTED);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Rng rng = new RZ_Rng(RZ_WorldFacts.Get().GetSeed() + RZ_SeasonClock.Day() * 31 + e.m_iFaction * 7 + e.m_iHouse);
		string name = RZ_FactionVoice.PersonName(rng, e.m_iFaction);
		e.SetText("name", name);
		e.m_sValue = name;
		e.m_sSubject = "newcomer:" + name;
		if (e.m_iFaction == RZ_EFaction.BANDITS)
		{
			array<string> truths = {"straight", "informer"};
			array<float> weights = {0.67, 0.33};
			e.m_iValue = 0;
			if (RZ_WorldFacts.Get().Memory().DrawTruth(m_sKey, e.m_sSubject, truths, weights) == "informer")
				e.m_iValue = 1;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Offers(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w)
	{
		// Only the Red Hand's new man has been inside
		if (w.m_iIndex == 1)
			return e.m_iFaction == RZ_EFaction.BANDITS;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_WorldMemory mem = RZ_WorldFacts.Get().Memory();
		if (w.m_iIndex == 2)
			return;

		if (w.m_iIndex == 1)
		{
			mem.FoundTruth(m_sKey, e.m_sSubject, "excon");
			if (e.m_iValue == 1)
				said.Insert("Two questions about the landings at the county jail and he's sweating. He was inside with a man who sang for the guards. You know the tune. You don't vouch, and nobody asks you to.");
			else
				said.Insert("He knows the landings, the screws' names and the price of tobacco on B wing. He's what he says. You can speak for him if you want to.");

			return;
		}

		array<int> who = {playerId};
		mem.Record("vouched." + e.m_sValue, e.m_sSubject, gm.GetProfile(playerId).m_sIdentity, RZ_WorldMemory.SOURCE_ERRAND, false, who);
		if (e.m_iFaction == RZ_EFaction.SURVIVORS)
		{
			if (gm.GetSafehouses())
				gm.GetSafehouses().AddPopulationAt(e.m_iHouse, 1);

			if (e.m_bTwist)
			{
				gm.AddInfamy(playerId, RZ_EFaction.SURVIVORS, 5, "vouched for a thief", 0, "spoke for a newcomer who stole from the stores");
				if (gm.GetSafehouses())
					gm.GetSafehouses().AddFood(e.m_iHouse, -1);

				dealer.Notify(e, "By morning two tins are gone from the stores, and so is the bread for {name}'s kid. {house} knows who spoke for them.");
			}

			return;
		}

		// The Red Hand: one in three he's selling to Bastion, found out by events
		if (e.m_iValue == 1 && gm.GetFacts())
			mem.Lies().Tell(gm, playerId, "vouch." + e.m_sValue, e.m_sSubject, RZ_EFaction.BANDITS, e.m_sGiver, e.m_vGiver, RZ_ELieTest.RUMOUR, RZ_Lies.VOUCH_INFAMY,
				"The new man you spoke for was selling to Bastion. The crew remembers who vouched for him.");
	}
}
