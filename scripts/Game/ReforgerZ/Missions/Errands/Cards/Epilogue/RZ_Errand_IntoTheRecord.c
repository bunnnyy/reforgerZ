//! Into the Record (the epilogue deck): one of Halcyon's dead still lies out with their notebook.
//! Fetch it for the paper; the wall lists the name as home at the lab. Within 4 km of the giver.
class RZ_Errand_IntoTheRecord : RZ_EpiCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "epi_into_record";
		m_sTitle = "Into the Record";
		m_eAnchor = RZ_EErrandAnchor.HALCYON_STATION;
		m_iFaction = RZ_EFaction.INSTITUTE;
		Offer("{dead} went out {lies} and didn't come back. The notebook should still be with the body. The record goes outside with the cure, every notebook in it, and this one is missing.",
			"There's a notebook {lies}, with {dead}. The record of the cure goes out through the gates at the end of the week. Whatever's in that notebook goes in the appendix, every page.");
		Over("Somebody brought the notebook in. It's in the appendix box already.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Fetch {dead}'s notebook";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 30;
		w.m_sStep = "Find the body of {dead} {lies} and take the notebook (15 s there), then bring it to {giver}.";
		w.Work(RZ_EWork.TAKE, "Take the notebook");
		w.m_eProp = RZ_EErrandProp.EFFECTS;
		w.m_iFavor = 5;
		w.m_iFame = 1;
		w.Done("That's {dead}'s handwriting, dated to the last day. It goes in the box marked APPENDIX, and the name goes in the front.");
	}

	//------------------------------------------------------------------------------------------------
	override bool WhyCard(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_EpiName n = LyingNear(RZ_EFaction.INSTITUTE, e.m_vGiver, 4000);
		if (!n)
			return false;

		e.m_vSubject = RZ_Util.GroundPos(Vector(n.x, 0, n.z));
		e.m_sSubject = "epi.name:" + n.name;
		e.m_sValue = n.name;
		e.SetText("dead", n.name);
		e.SetText("lies", n.lies);
		e.SetText("subject_place", n.lies);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Still(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_EpiName n = NameCalled(e.m_sValue);
		return n && n.state == 0;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		NameHome(NameCalled(e.m_sValue), "the lab", playerId);
	}
}
