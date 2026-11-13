//! False Hope at the Line (docs/ERRANDS.md, the line): a man in a good coat at a Haven house's
//! gate, selling papers he says will get a family through the sealed checkpoint on the north road.
//! Nothing gets through. Buy a set off him as proof and carry it to the giver (Props > Forged
//! papers), see him off the line, or let him be, and a family from the house goes to the checkpoint.
class RZ_Errand_FalseHope : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "false_hope";
		m_sTitle = "False Hope at the Line";
		m_sFamily = "line";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("There's a man at the gate in a good coat, selling papers. Travel papers. He says they'll get a family through the checkpoint on the north road. Three families have asked me if they should sell their blankets for them. Nothing goes through that checkpoint. Nothing.",
			"A forger's working the line at our gate. Passes out, he says, stamped and signed. People are paying in food we haven't got. I can't make them stop believing him. Maybe you can make him stop selling.");
		Over("He's gone. Moved on to the next house, I expect. They always do.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Buy a set off him, for proof";
		w.m_ePlace = RZ_EErrandPlace.GATE;
		w.m_fSeconds = 8;
		w.m_fRadius = 15;
		w.m_sStep = "Buy a set of the forger's papers at the gate of {here} (8 s there), then carry them to {giver} yourself, so the house can see what they're paying for.";
		w.Work(RZ_EWork.HANDS, "Buy a set of papers");
		w.m_bCarry = true;
		w.m_iFavor = 5;
		w.Done("Look at the stamp. It's a potato. It's a cut potato, and they were going to sell their blankets for it. I'll pass these round the stove tonight.");

		w = Way("do");
		w.m_sLabel = "See him off the line";
		w.m_ePlace = RZ_EErrandPlace.GATE;
		w.m_fSeconds = 8;
		w.m_fRadius = 15;
		w.m_sStep = "See the forger off the line at the gate of {here} (8 s there), then tell {giver}.";
		w.Work(RZ_EWork.HANDS, "Send him on his way");
		w.m_iFavor = 3;
		w.Done("He's gone, and he took his hope with him. Some of them are angry with you. They'll be less angry in a week.");

		w = Way("leave");
		w.m_sLabel = "Leave him be. Hope's scarce";
		w.Done("Leave him be. Yes. I see. It's what they want to buy.");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		// A house with people enough to have a line, and a reason to want out
		RZ_HouseFacts house = RZ_WorldFacts.Get().House(e.m_iHouse);
		if (!house.m_bValid || house.m_iPeople < 5)
			return false;

		if (!house.m_bShort && house.m_iLost <= 0)
			return false;

		e.m_sSubject = RZ_WorldFacts.Safehouse(e.m_iHouse);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override ResourceName CarriedPrefab(RZ_Errand e)
	{
		return RZ_ErrandSettings.Get().Props().m_sForgedPapers;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_WorldMemory mem = RZ_WorldFacts.Get().Memory();
		array<int> who = {playerId};
		if (w.m_iIndex == 0)
		{
			mem.Record("errand.false_hope", e.m_sSubject, "shown", RZ_WorldMemory.SOURCE_ERRAND, true, who);
			return;
		}

		if (w.m_iIndex == 1)
		{
			mem.Record("errand.false_hope", e.m_sSubject, "sent off", RZ_WorldMemory.SOURCE_ERRAND, true, who);
			return;
		}

		// Let be: a family from the house walks to the checkpoint with his papers. The Zone is sealed
		mem.Record("errand.false_hope", e.m_sSubject, "let be", RZ_WorldMemory.SOURCE_ERRAND, true, who);
		RZ_Safehouses houses = dealer.GetGM().GetSafehouses();
		if (houses)
		{
			RZ_Safehouse h = houses.Get(e.m_iHouse);
			if (h && h.m_iPopulation > 4)
				houses.RemoveDead(e.m_iHouse, 1);
		}

		said.Insert("A family left this morning for the north road, with his papers in a tin. The guns on the checkpoint don't read papers.");
	}
}
