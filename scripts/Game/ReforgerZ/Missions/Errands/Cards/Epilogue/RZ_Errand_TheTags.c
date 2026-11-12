//! His Effects, the epilogue's variant: The Tags. A Bastion post that lost a man this season wants
//! his tags off him and on the nail before the gates open. Within 1.5 km of the post, a Bastion name
//! still lying out (Bring Them Home's list). The grey way: the fence buys the photo frame he
//! carried, and the name is never brought home (5 Bastion Infamy, Red Hand Favor).
class RZ_Errand_TheTags : RZ_EpiCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "epi_tags";
		m_sTitle = "The Tags";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		Offer("\"{dead} is still out {lies}. The tags belong on the nail before the gates open. Nobody here can leave the gate.\"",
			"\"We never brought {dead} in. The tags, at least. The roll's read with or without him.\"");
		Over("\"Never mind. Somebody brought {dead}'s tags in this morning.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Bring {dead}'s tags in";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 30;
		w.m_sStep = "Find {dead} {lies} and take the tags (15 s there), then bring them to {giver} for the nail.";
		w.m_iFavor = 6;
		w.m_iFame = 1;
		w.Done("\"On the nail. That name gets answered at the roll now, one way or the other.\"",
			"\"Thank you. I'll tell the lieutenant who brought them.\"");

		w = Way("press");
		w.m_sLabel = "Sell the photo frame to the fence";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 30;
		w.m_iFavor = 6;
		w.m_iPayFaction = RZ_EFaction.BANDITS;
		w.m_sStep = "Find {dead} {lies} (15 s there). The fence pays for silver frames.";
		w.Done("\"No tags?\" The sergeant looks at your hands, then away. \"Then he's still out there.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool WhyCard(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_EpiName n = LyingNear(RZ_EFaction.MILITARY, e.m_vGiver, 1500);
		if (!n)
			return false;

		e.m_vSubject = Vector(n.x, 0, n.z);
		e.m_sValue = n.name;
		e.m_sSubject = "epi.tags:" + n.name;
		e.SetText("dead", n.name);
		e.SetText("lies", n.lies);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_EpiName n = NameCalled(e.m_sValue);
		if (!n || n.state != 0)
			return;

		if (w.m_iIndex == 0)
		{
			NameHome(n, "the nail at " + e.m_sGiverName, playerId);
			return;
		}

		n.state = 3;
		dealer.GetGM().AddInfamy(playerId, RZ_EFaction.MILITARY, 5, "sold a dead soldier's things", 0, "sold the photo frame off a Bastion body");
	}
}
