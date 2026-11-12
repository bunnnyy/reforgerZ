//! His Effects (docs/ERRANDS.md, the dead): a truck of the giver's faction was lost within 1.5 km in
//! the last season day (the signal ring). Bastion's operator calls it The Locker: bring the crew's
//! effects to the post, the tag goes on the nail and the rest in a box "for when the gates open".
//! Everyone else calls it The Wreck's Name: the driver's papers to the nearest outpost, and the name
//! read on the channel. Or sell the effects to the fence (Known with the Red Hand). The arsonist
//! rule: never to whoever robbed the truck.
class RZ_Errand_HisEffects : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "his_effects";
		m_sTitle = "His Effects";
		m_sFamily = "dead";
		m_eAnchor = RZ_EErrandAnchor.ANY_HOLDING;
		m_iPerLife = 3;
		Offer("One of our trucks went over {where_place} yesterday. The crew didn't make it. Their things are still in the cab: tags, papers, a jacket, whatever they carried. Somebody should bring them home before the fence's lads get there.",
			"There's a wreck of ours {where_place}, with the driver's papers still in it. We lost the truck and the man in one afternoon. I'd like his name said properly, and his things kept for when the gates open.");
		Over("Someone brought the crew's things in this morning. Thank you all the same.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Bring his things back";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_sStep = "Go to the wrecked truck {where_place} and gather the crew's things (15 s there), then bring them to {giver}.";
		w.Work(RZ_EWork.TAKE, "Gather their things");
		w.m_eProp = RZ_EErrandProp.EFFECTS;
		w.m_bCarry = true;
		w.m_iFavor = 5;
		w.m_iFame = 1;
		w.Done("I'll read the name tonight. The rest goes in a box with a tag on the lid, for when the gates open and somebody comes asking.",
			"These are the driver's papers. Lovely handwriting. You'd never know it from the driving.");

		w = Way("do");
		w.m_sLabel = "Sell them to the fence";
		w.m_iMinTier = RZ_ETier.KNOWN;
		w.m_iTierFaction = RZ_EFaction.BANDITS;
		w.m_iPayFaction = RZ_EFaction.BANDITS;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_sStep = "Go to the wrecked truck {where_place}, take the crew's things (15 s there), and sell the jacket and the photo frame at the fence. Then go back to {giver}.";
		w.Work(RZ_EWork.TAKE, "Take their things");
		w.m_eProp = RZ_EErrandProp.EFFECTS;
		w.m_bCarry = true;
		w.m_iFavor = 0;
		w.Done("Gone? Somebody got there first. I'll list it as kit lost. That's how it goes this season.");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Signal s = FreshSignal(TruckKind(e.m_iFaction), e.m_vGiver, 1500, RZ_SeasonClock.DayMinutes() / 60, e.m_iAsker);
		if (!s)
			return OnTrace(e);

		e.m_vSubject = RZ_Util.GroundPos(Vector(s.x, 0, s.z));
		e.m_sSubject = "wreck:" + RZ_SettlementScanner.GridReference(e.m_vSubject);
		e.m_sValue = s.cause;
		e.SetText("where_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
		e.SetText("subject_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
		if (e.m_iFaction == RZ_EFaction.MILITARY)
			e.SetText("title", "The Locker");
		else
			e.SetText("title", "The Wreck's Name");

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! No fresh loss: a Traces wreck (L5, a driver's papers) or a fallen post (V7, a soldier's tags)
	//! within 1.5 km of the giver.
	protected bool OnTrace(RZ_Errand e)
	{
		vector at;
		string key;
		if (!OnScene("L5,V7", e.m_vGiver, 1500, at, key))
			return false;

		e.m_vSubject = RZ_Util.GroundPos(at);
		e.m_sSubject = "scene:" + key;
		e.m_sValue = "";
		e.SetText("where_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
		e.SetText("subject_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
		if (e.m_iFaction == RZ_EFaction.MILITARY)
			e.SetText("title", "The Locker");
		else
			e.SetText("title", "The Wreck's Name");

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Offers(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w)
	{
		// The Red Hand's own people don't sell their own crew's things to their own fence
		return w.m_iIndex == 0 || e.m_iFaction != RZ_EFaction.BANDITS;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		array<int> who = {playerId};
		if (w.m_iIndex == 0)
		{
			string epilogue = "";
			string carrier = "";
			if (e.m_iFaction == RZ_EFaction.MILITARY)
			{
				epilogue = "Boxes stacked by the radio, each with a tag on the lid: for when the gates open.";
				carrier = "prop";
			}

			RZ_WorldFacts.Get().Memory().Record("errand.his_effects", e.m_sSubject, "brought home", RZ_WorldMemory.SOURCE_ERRAND, true, who, epilogue, carrier);
		}
		else
		{
			RZ_WorldFacts.Get().Memory().Record("errand.his_effects", e.m_sSubject, "kit lost", RZ_WorldMemory.SOURCE_ERRAND, true, who);
		}
	}
}
