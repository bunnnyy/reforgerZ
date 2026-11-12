//! His Effects, the epilogue's variant: The Wreck's Name. A faction that lost a truck this season
//! within 1.5 km of its holding wants the driver's papers off the wreck, so the name goes on the
//! roll, the wall or the book. The grey way: the fence buys the photo frame off the dashboard.
class RZ_Errand_WrecksName : RZ_EpiCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "epi_wreck";
		m_sTitle = "The Wreck's Name";
		m_eAnchor = RZ_EErrandAnchor.ANY_HOLDING;
		Offer("The truck for {dest} is still on its side {lies}. The driver's papers are in the cab. I'd like the name for the roll.",
			"Nobody went back for the truck to {dest}. The driver had papers. Somebody should know who drove it.");
		Over("Never mind. A patrol stripped the wreck this morning.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Fetch the driver's papers";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 20;
		w.m_fRadius = 30;
		w.m_sStep = "Search the wreck {lies} for the driver's papers (20 s there), then bring them to {giver}.";
		w.m_iFavor = 5;
		w.m_iFame = 1;
		w.Done("That's the name. I'd forgotten it. That's worse than anything.");

		w = Way("press");
		w.m_sLabel = "Sell the photo frame to the fence";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 20;
		w.m_fRadius = 30;
		w.m_iFavor = 5;
		w.m_iPayFaction = RZ_EFaction.BANDITS;
		w.m_sStep = "Search the wreck {lies} (20 s there). The fence pays for anything in a frame.";
		w.Done("\"No papers.\" A shrug that isn't one. \"Then the truck drove itself.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool WhyCard(RZ_Errands dealer, RZ_Errand e)
	{
		if (!RZ_Factions.IsValid(e.m_iFaction) || !Week())
			return false;

		RZ_WorldMemory mem = RZ_WorldFacts.Get().Memory();
		if (!mem)
			return false;

		array<RZ_Signal> trucks = {};
		mem.RecentSignals("truck." + e.m_iFaction.ToString(), e.m_vGiver, 1500, 60, trucks);
		if (trucks.IsEmpty())
			return false;

		RZ_Signal sig = trucks[trucks.Count() - 1];
		vector at = Vector(sig.x, 0, sig.z);
		e.m_vSubject = RZ_Util.GroundPos(at);
		e.m_sSubject = string.Format("epi.wreck:%1:%2", RZ_Util.RoundToInt(sig.x), RZ_Util.RoundToInt(sig.z));
		e.SetText("dest", sig.text);
		e.SetText("lies", Week().PlaceWords(at));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex == 0)
		{
			RZ_Epilogue epi = Week();
			if (epi)
				epi.AddHallLine(string.Format("The driver for %1 was named again, from the papers in the cab.", e.GetText("dest")));

			return;
		}

		dealer.GetGM().AddInfamy(playerId, e.m_iFaction, 5, "sold a dead driver's things", 0, "sold the photo frame off a wreck");
	}
}
