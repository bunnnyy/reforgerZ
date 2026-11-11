//! After the Noise (docs/ERRANDS.md, aftermath): a horde passed through a town within 1.5 km (the reason
//! horde.passed). The lookout wants to know what it left behind: walk its edge unseen and count what's
//! lying down, or, a Hunter, read the ground.
class RZ_Errand_AfterTheNoise : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "after_noise";
		m_sTitle = "After the Noise";
		m_sFamily = "aftermath";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "lookout";
		Offer("\"They went through {subject_place} last night. All of them. I want to know what they left behind, and if any of it is still standing.\"",
			"\"A horde passed {subject_place}. Walk the edge. Count what's lying down. Don't count what isn't.\"");
		Over("\"Never mind. Somebody's been. It's quiet.\"");

		RZ_ErrandWay w = Way("sneak");
		w.m_sLabel = "Walk the edge, unseen";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 20;
		w.m_fRadius = 60;
		w.m_bUnseen = true;
		w.m_sStep = "Walk the edge of {subject_place} unseen (20 s hidden there), then tell {giver} what's lying down.";
		w.m_iFavor = 5;
		w.Done("\"That many down. And the rest moved on. Right. We can breathe for a day.\"");

		w = Way("do");
		w.m_sLabel = "Read the ground";
		w.m_sBackground = "hunter";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 80;
		w.m_sStep = "Read the ground at the edge of {subject_place} (10 s there), then tell {giver} which way they went.";
		w.m_iFavor = 5;
		w.Done("\"North-east, and not coming back this way. You're sure? You look sure.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		array<RZ_Reason> reasons = {};
		RZ_WorldFacts.Get().Memory().ReasonsNear(e.m_vGiver, 1500, reasons, "horde.passed");
		if (reasons.IsEmpty())
			return false;

		RZ_Reason r = reasons[0];
		vector at = Vector(r.x, 0, r.z);
		RZ_Settlement s = dealer.GetGM().GetSettlement(dealer.GetGM().FindNearestSettlement(at));
		string name = "";
		if (s)
			name = s.m_sName;

		Place(e, RZ_Mission.LandPointNear(at, 0, 80), name);
		e.m_sSubject = r.subject + ".after";
		return true;
	}
}
