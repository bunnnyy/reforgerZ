//! Nothing Goes Out (docs/ERRANDS.md, the line): the perimeter is sealed, a sealed road checkpoint is
//! within 1.5 km of a Bastion post, and Bastion's morale is under 50. A private asks you to carry his
//! letter to the warning point, 150 m short of the checkpoint, and leave it under a stone. It never
//! goes out. The letter under its stone is a world record the epilogue can show.
class RZ_Errand_NothingGoesOut : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "nothing_goes_out";
		m_sTitle = "Nothing Goes Out";
		m_sFamily = "line";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		Offer("\"Letter for my sister. She's outside. Leave it at the warning point, under a stone, where someone coming in would see it. Nobody's coming in. Leave it anyway.\"");
		Over("\"Forget it. I burned it. It was badly spelled.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Leave it under a stone at the line";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 25;
		w.m_sStep = "Take the letter to the warning point, {where_place}, 150 m short of the checkpoint, and leave it under a stone (5 s there). Don't go further. Then report to {giver}.";
		w.m_iFavor = 3;
		w.Done("\"Under a stone. Good. Now it's somewhere.\"");

		w = Way("do");
		w.m_sLabel = "Read it on the way, then leave it";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 25;
		w.m_sStep = "Take the letter to the warning point, {where_place}, and leave it under a stone (5 s there). Then report to {giver}.";
		w.m_iFavor = 3;
		w.Done("\"You read it. I can tell. It says she should eat properly. She won't.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		RZ_Perimeter perimeter = gm.GetPerimeter();
		if (!facts.PerimeterSealed() || !perimeter || !perimeter.IsActive())
			return false;

		RZ_FactionLife life = gm.GetFactionLife();
		if (life && life.IsActive() && life.GetMorale(RZ_EFaction.MILITARY) >= 50)
			return false;

		vector cp;
		if (!perimeter.GetNearestCheckpoint(e.m_vGiver, cp) || RZ_Util.DistXZ(cp, e.m_vGiver) > 1650)
			return false;

		e.m_vSubject = perimeter.PointInside(cp, perimeter.InwardAt(cp), 150);
		e.m_sSubject = "line:" + RZ_SettlementScanner.GridReference(cp);
		e.SetText("where_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
		e.SetText("subject_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		array<int> who = {playerId};
		RZ_WorldFacts.Get().Memory().Record("errand.letter_at_line", e.m_sSubject, e.m_sGiverName, RZ_WorldMemory.SOURCE_ERRAND, true, who,
			"A letter under a stone at the warning point, addressed to someone outside. Nobody has moved it.", "prop");
	}
}
