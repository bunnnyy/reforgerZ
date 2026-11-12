//! Somebody Should Tell Them, the epilogue deck's variant of the lost card (Cards/Lost/): a resident's relative runs with a pack now (a
//! named young, never a player's character). Read the marks below its ground and bring back what
//! you find: the name goes on Bring Them Home's list, with an ID card to carry home. Or tell them
//! plainly, and the house digs a grave with nothing in it. No keepsake from the giver, no player
//! names. Only while a pack with names walking with it is within 3 km.
class RZ_Errand_TellThemWeek : RZ_EpiCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "epi_tell";
		m_sTitle = "Somebody Should Tell Them";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("There's a family at this house still waiting for {name}. People say {name} was seen with the lot from {pack}. Somebody should tell them. I can't.",
			"{name}'s mother sets a place every night. Somebody's seen {name} with the ones from {pack}. Go and look. Then tell her, one way or another.");
		Over("Never mind. She knows. Somebody told her this morning.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Read the marks below {pack}";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 30;
		w.m_fRadius = 60;
		w.m_bUnseen = true;
		w.m_sStep = "Go below {pack} and read the ground for {name} (30 s unseen), then come back to {giver}.";
		w.m_iFavor = 5;
		w.m_iFame = 1;
		w.Done("\"An ID card. In the mud. Then it's true.\" A long breath. \"Thank you for bringing something.\"");

		w = Way("talk");
		w.m_sLabel = "Tell them plainly";
		w.Done("\"Plainly. Yes.\" They stand a while. \"We'll dig one by the fence. There's nothing to put in it. We'll dig it anyway.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool WhyCard(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Tribes tribes = dealer.GetGM().GetTribes();
		if (!tribes)
			return false;

		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			if (RZ_Util.DistXZ(t.m_vLair, e.m_vGiver) > 3000)
				continue;

			foreach (RZ_NamedYoung y : t.m_aNamed)
			{
				if (y.m_bPlayer || !y.m_sIdent.IsEmpty() || y.m_sName.IsEmpty())
					continue;

				e.m_vSubject = RZ_Mission.LandPointNear(t.m_vLair, 250, 350);
				e.m_iValue = t.m_iId;
				e.m_sValue = y.m_sName;
				e.m_sSubject = "epi.tell:" + y.m_sName;
				e.SetText("name", y.m_sName);
				e.SetText("pack", t.m_sPlace);
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_Epilogue epi = Week();
		if (!epi)
			return;

		if (w.m_iIndex == 0)
		{
			if (!NameCalled(e.m_sValue))
				epi.AddNamed(e.m_sValue, "their ID card", -1, e.m_vSubject, "", "", "");

			return;
		}

		epi.GetBeats().AddSince("grave." + e.m_sValue, e.m_sGiverName, e.m_vGiver, string.Format("A grave by the fence with nothing in it. A board: %1.", e.m_sValue), "An empty grave by a house fence.");
	}
}
