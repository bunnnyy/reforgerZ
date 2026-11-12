//! The Boy (the epilogue deck): the Long Table's boy got out, and he's the Kid on the roads now. A
//! council head wants him brought in before the gates open: to the queue (he goes with the Walk Out,
//! if it opens) or to a house (one more mouth there). Only while the Kid walks.
class RZ_Errand_TheBoy : RZ_EpiCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "epi_boy";
		m_sTitle = "The Boy";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("\"The boy from the farm is still on the roads. {kid}. Somebody has to bring him in before the gates open, and he won't come for me.\"",
			"\"{kid} sleeps in barns. Find him. The queue, or here: his choice, but not the road.\"");
		Over("\"Never mind. {kid} turned up at the gate this morning. Alone.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Walk {kid} to the queue";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 30;
		w.m_sStep = "Find {kid} {where} and talk the boy into the queue (15 s together), then tell {giver}.";
		w.m_iFavor = 5;
		w.m_iFame = 1;
		w.Done("\"The queue? Good. A line is somewhere to stand, at least.\"");

		w = Way("do");
		w.m_sLabel = "Bring {kid} to this house";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 30;
		w.m_sStep = "Find {kid} {where} and bring the boy here (15 s together), then tell {giver}.";
		w.m_iFavor = 5;
		w.m_iFame = 1;
		w.Done("\"Here. Good. We've a blanket and a bowl. That's what we've got.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool WhyCard(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Wanderers wanderers = dealer.GetGM().GetWanderers();
		if (!wanderers || Queue() == vector.Zero)
			return false;

		array<int> kids = {};
		wanderers.GetLiving(RZ_Wanderers.RoleBit(RZ_EWandererRole.KID), kids);
		foreach (int id : kids)
		{
			vector pos;
			if (!wanderers.GetPosition(id, pos) || RZ_Util.DistXZ(pos, e.m_vGiver) > 3000)
				continue;

			e.m_iValue = id;
			e.m_vSubject = pos;
			e.m_sSubject = "epi.boy";
			e.SetText("kid", wanderers.GetName(id));
			e.SetText("where", Week().PlaceWords(pos));
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Wanderers wanderers = gm.GetWanderers();
		if (!wanderers || !wanderers.IsAlive(e.m_iValue))
			return;

		wanderers.TakeOffRoad(e.m_iValue);
		if (w.m_iIndex == 1 && gm.GetSafehouses())
			gm.GetSafehouses().AddPopulation(e.m_vGiver, 1);

		RZ_WorldFacts.Get().SetFlag("table.boy_home", w.m_iIndex + 1);
	}
}
