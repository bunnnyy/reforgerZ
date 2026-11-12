//! The Name on the Tag (docs/ERRANDS.md, the dead; its satchel variant): someone died within 1.5 km
//! of a safehouse in the last season day (the signal ring: their background and how long they
//! lasted, never their name). A satchel came in with no name on it. Leave it on a cairn where they
//! fell, or bring the line to the board. Never a player's name. Either way a world record with an
//! epilogue line on the board. Also a story job's fallout (Distress Call, Survivors Trapped): the
//! dead from that job, at its place, for its participants first.
class RZ_Errand_NameOnTheTag : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "name_on_tag";
		m_sTitle = "The Name on the Tag";
		m_sFamily = "dead";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		m_sRoles = "clerk";
		Offer("A satchel came in with no name in it. {who}. Somebody should put it where they fell, or on the board, so it's somewhere.");
		Over("Someone took the satchel out at first light. It is done.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Leave it on a cairn where they fell";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 40;
		w.m_sStep = "Carry the satchel {where_place} and build a small cairn for it (10 s there), then tell {giver}.";
		w.m_iFavor = 4;
		w.Done("A cairn. That's more than most get. That's a long time, now, out there.");

		// A fallout's dead at a job's site: the named body, its ID carried to the house or post it names
		w = Way("do");
		w.m_sLabel = "Take the ID from the body";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 10;
		w.m_fRadius = 40;
		w.m_sStep = "Find the body {where_place} and take the ID (10 s there), then carry it to {giver} yourself.";
		w.m_eProp = RZ_EErrandProp.EFFECTS;
		w.m_bCarry = true;
		w.m_iFavor = 0;
		w.Done("That's the name. We'll see it gets to the people it belongs to.");

		w = Way("talk");
		w.m_sLabel = "Pin the line to the board";
		w.Done("On the board, then. Someone will read it and know who it was. Or won't. It's there.");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		// A failed story job's fallout first (Distress Call, Survivors Trapped): their IDs to the house
		RZ_Fallout fo = dealer.FalloutFor(m_sKey, e.m_vGiver, e.m_iAsker);
		if (fo)
		{
			e.m_bFallout = true;
			e.m_iCap = fo.cap;
			e.m_vSubject = RZ_Util.GroundPos(fo.at);
			e.m_sSubject = "fell:" + RZ_SettlementScanner.GridReference(e.m_vSubject);
			e.m_sValue = fo.who;
			e.SetText("who", RZ_Util.Capitalize(fo.who));
			e.SetText("where_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
			e.SetText("subject_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
			return true;
		}

		RZ_Signal s = FreshSignal("death", e.m_vGiver, 1500, RZ_SeasonClock.DayMinutes() / 60, e.m_iAsker);
		if (!s)
			return false;

		e.m_vSubject = RZ_Util.GroundPos(Vector(s.x, 0, s.z));
		e.m_sSubject = "fell:" + RZ_SettlementScanner.GridReference(e.m_vSubject);
		e.m_sValue = s.text;
		e.SetText("who", RZ_Util.Capitalize(s.text));
		e.SetText("where_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
		e.SetText("subject_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The named body's way only for a job's own dead (a fallout).
	override bool Offers(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w)
	{
		return w.m_iIndex != 1 || e.m_bFallout;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		// The ID home: 4 Favor from the faction it names; the Red Hand doesn't bury its dead, so thanks only
		if (w.m_iIndex == 1)
		{
			int f = RZ_EFaction.SURVIVORS;
			if (e.m_sValue.Contains("Bastion"))
				f = RZ_EFaction.MILITARY;
			else if (e.m_sValue.Contains("crew"))
				f = RZ_EFaction.BANDITS;

			if (f == RZ_EFaction.BANDITS)
				said.Insert("The crew won't want it back. They don't bury theirs. We'll keep it on the board.");
			else
				dealer.SmallFavor(playerId, f, 4);
		}

		array<int> who = {playerId};
		string line = RZ_Util.Capitalize(e.m_sValue) + ". Somebody brought the satchel home.";
		RZ_WorldFacts.Get().Memory().Record("errand.name_on_tag", e.m_sSubject, e.m_sValue, RZ_WorldMemory.SOURCE_ERRAND, true, who, line, "board");
	}
}
