//! The Jar Run (docs/ERRANDS.md, the Long Table): a Red Hand lad, when the farm stands unexposed within
//! 1.5 km of his camp. Never while its questlines or Cargo run, and never for family or the crossed.
//! Buy a jar (6 Favor, and the lads say nothing at all), say nobody was home (a lie), or ask where the
//! farmer gets his meat: the lad tells you not to, and it counts as something noticed. Never described.
class RZ_Errand_JarRun : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "jar_run";
		m_sTitle = "The Jar Run";
		m_sFamily = "table";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		Offer("Run up to the farm at {subject_place} and get us a jar. The boss likes a jar. Don't ask what's in it.",
			"A jar from the farm at {subject_place}. Pay what they ask. Don't stay for supper.");
		Over("Never mind the jar. Nobody's going up there today.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Fetch a jar from the farm";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 30;
		w.m_sStep = "Buy a jar at the farm door {subject_place} (10 s there), then bring it to {giver}.";
		w.m_iFavor = 6;
		w.Done("He takes the jar and doesn't look at it. Nobody says anything at all.");

		w = Way("lie");
		w.m_sLabel = "Say nobody was home";
		w.m_iFavor = 3;
		w.Done("\"Nobody home. Right.\" He looks at you a moment too long.");

		w = Way("talk");
		w.m_sLabel = "Ask where the farmer gets his meat";
		w.Done("\"Don't.\" That's all. He goes himself.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		RZ_LongTable table = dealer.GetGM().GetLongTable();
		if (!table)
			return false;

		return !table.Remembers(playerId, RZ_LongTable.MEM_FAMILY) && !table.Remembers(playerId, RZ_LongTable.MEM_CROSSED);
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_LongTable table = gm.GetLongTable();
		if (!table || !table.Exists() || table.IsExposed() || !table.IsNearHouse(e.m_vGiver, 1500))
			return false;

		RZ_Questlines q = gm.GetQuestlines();
		if (q && (q.IsRunning("table") || q.IsRunning("place") || q.IsRunning("cargo")))
			return false;

		Place(e, RZ_Mission.LandPointNear(table.GetHousePos(), 8, 16), "");
		e.m_sSubject = "table.farm";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (w.m_iIndex == 1)
		{
			gm.GetFacts().Memory().Lies().Tell(gm, playerId, "errand.jar", e.m_sSubject, RZ_EFaction.BANDITS, e.m_sGiver, e.m_vSubject, RZ_ELieTest.RUMOUR, 2,
				"The lad went up himself. Somebody was home. Somebody's always home.");
			return;
		}

		if (w.m_iIndex == 2 && gm.GetLongTable())
			gm.GetLongTable().Notice(playerId);
	}
}
