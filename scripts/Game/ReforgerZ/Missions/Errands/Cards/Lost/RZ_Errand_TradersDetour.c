//! The Trader's Detour (docs/ERRANDS.md, the lost): a Trader's destination has just turned bad (the
//! town there is infested, or a job is under way at it). Scout his road 400-600 m ahead so he can go
//! round. He remembers who helped him (his helped-me prices).
class RZ_Errand_TradersDetour : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "traders_detour";
		m_sTitle = "The Trader's Detour";
		m_sFamily = "lost";
		m_eAnchor = RZ_EErrandAnchor.WANDERER;
		m_iWandererRole = RZ_EWandererRole.TRADER;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("\"I was for {town}, and I hear {town} isn't for anyone now. Walk my road a bit ahead of me? I'll make it worth your while. Slightly.\"");
		Over("\"Changed my mind. Going back the way I came.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Scout the road ahead";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 20;
		w.m_fRadius = 50;
		w.m_sStep = "Scout the road {where_place} (20 s there, keep your eyes open), then find {giver} again.";
		w.m_iFavor = 4;
		w.Done("\"Clear? Then I'll go round. You'll find my prices friendlier. Not friendly. Friendlier.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Wanderer w = gm.GetWanderers().FindById(e.m_iWanderer);
		if (!w || w.m_sDestName.IsEmpty())
			return false;

		int town = gm.FindNearestSettlement(w.m_vDest);
		RZ_Settlement s = gm.GetSettlement(town);
		if (!s || RZ_Util.DistXZ(s.m_vCenter, w.m_vDest) > 800)
			return false;

		bool bad = s.m_fInfection >= 60 || gm.GetMissions().IsSettlementBusy(town);
		if (!bad)
			return false;

		vector dir = w.m_vDest - w.m_vPos;
		dir[1] = 0;
		float len = dir.Length();
		if (len < 100)
			return false;

		dir = dir * (Math.Min(500, len * 0.5) / len);
		e.m_vSubject = RZ_Mission.LandPointNear(w.m_vPos + dir, 0, 40);
		e.m_sSubject = RZ_WorldFacts.Wanderer(w.m_iId);
		e.SetText("town", s.m_sName);
		e.SetText("where_place", RZ_Util.DirectionText(w.m_vPos, e.m_vSubject));
		e.SetText("subject_place", RZ_Util.DirectionText(w.m_vPos, e.m_vSubject));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Still(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Wanderers wanderers = dealer.GetGM().GetWanderers();
		return wanderers && wanderers.IsAlive(e.m_iWanderer);
	}
}
