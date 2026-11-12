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
		Offer("\"{dead} went out {lies} and didn't come back. The notebook should be with them. It belongs in the record.\"",
			"\"There's a notebook {lies}, with {dead}. Whatever's in it goes in the appendix. Every page.\"");
		Over("\"Somebody brought the notebook in. It's in the box already.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Fetch {dead}'s notebook";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 30;
		w.m_sStep = "Find {dead} {lies} and take the notebook (15 s there), then bring it to {giver}.";
		w.m_eProp = RZ_EErrandProp.EFFECTS;
		w.m_iFavor = 5;
		w.m_iFame = 1;
		w.Done("\"{dead}'s handwriting. Dated to the last day.\" The notebook goes in a box marked APPENDIX.");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
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
