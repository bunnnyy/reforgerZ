//! Nobody Looks at a Stray (docs/ERRANDS.md, the bad-standing deck): a crew's book-keeper, for a
//! Haven exile. "Strays are useful. Nobody looks at a stray." Carry a parcel to the crossroads (a Red
//! Hand road post or another of their places within 1.5 km): 6 Favor. What's in it is grey.
class RZ_Errand_Stray : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "stray";
		m_sTitle = "Nobody Looks at a Stray";
		m_sFamily = "standing";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		Offer("\"The Haven threw you out. Good. Strays are useful. Nobody looks at a stray. Take this to the crossroads, and don't open it.\"",
			"\"A parcel, a crossroads, and no questions. You've a face nobody wants to look at. That's a qualification.\"");
		Over("\"Never mind. Somebody else took it. Somebody with an even worse face.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Carry it to the crossroads";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 5;
		w.m_fRadius = 30;
		w.m_sStep = "Take the parcel to the crossroads {subject_place} and leave it with whoever's there (5 s), then come back to {giver}.";
		w.m_iFavor = 6;
		w.Done("\"Delivered. See? Nobody looked at you once.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		RZ_Safehouses houses = dealer.GetGM().GetSafehouses();
		return houses && houses.IsExiled(playerId);
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Checkpoints posts = gm.GetCheckpoints();
		if (posts)
		{
			array<RZ_RoadPost> near = {};
			posts.PostsNear(RZ_EFaction.BANDITS, e.m_vGiver, 1500, near);
			foreach (RZ_RoadPost p : near)
			{
				if (RZ_Util.DistXZ(p.m_vPos, e.m_vGiver) >= 200)
				{
					Place(e, p.m_vPos, "");
					e.m_sSubject = "post:" + p.m_iId.ToString() + ".parcel";
					return true;
				}
			}
		}

		RZ_Holding h = HoldingNear(gm, RZ_EFaction.BANDITS, e.m_vGiver, 1500);
		if (!h)
			return false;

		Place(e, h.m_vPos, h.m_sName);
		e.m_sSubject = "holding:" + h.m_iId.ToString() + ".parcel";
		return true;
	}
}
