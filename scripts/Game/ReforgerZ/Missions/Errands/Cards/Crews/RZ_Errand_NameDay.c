//! Name Day (docs/ERRANDS.md, the crews): a crew's book-keeper, for someone Known with the Red Hand
//! who has no crew name yet. Two dares, stupid on purpose: fetch the clock from the church tower in a
//! town the infected hold (Belfry), or one round in the pit while a Fight Pit runs (Ringside). The name
//! you get is whatever went wrong (reputation's crew names). The third dare is the Paint card.
class RZ_Errand_NameDay : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "name_day";
		m_sTitle = "Name Day";
		m_sFamily = "crews";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		Offer("You've no name with the crews yet, and a red hand isn't a name. Names are earned, and they're earned stupid. Pick a dare: the church clock in a town the infected hold, or a round in the pit. Whatever goes wrong, that's what we'll call you.",
			"It's your name day. Everybody in the crews gets one. Fetch the clock down from a church tower, or go one round in the pit. Don't worry about doing it well. Nobody does, and nobody's named for doing it well.");
		Over("Never mind the dare. The lads have found something to call you already. You won't like it.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Belfry: fetch the church clock";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 20;
		w.m_fRadius = 40;
		w.m_sStep = "Climb the church tower in {subject_place} and bring the clock down (20 s there). The infected hold the town, so mind the stairs. Then bring it to {giver}.";
		w.m_iFavor = 5;
		w.m_iFame = 1;
		w.Done("A clock. It's the wrong clock, but it's a clock. Legs, we'll call you. The lads watched you on those stairs through the glasses.");

		w = Way("do");
		w.m_sLabel = "Ringside: one round in the pit";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT2;
		w.m_fSeconds = 60;
		w.m_fRadius = 30;
		w.m_sStep = "Go one round in the pit at {pit} (a minute in the ring), then see {giver}.";
		w.m_iFavor = 5;
		w.m_iFame = 1;
		w.Done("You said sorry every time you hit him. Every single time. Sorry, then. That's your name now.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		return RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.KNOWN) && RZ_RepVoice.CrewName(gm.GetProfile(playerId)).IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		array<RZ_Reason> towns = {};
		RZ_WorldFacts.Get().Memory().ReasonsNear(e.m_vGiver, 1500, towns, "town.infested");
		if (!towns.IsEmpty())
		{
			vector at = Vector(towns[0].x, 0, towns[0].z);
			RZ_Settlement s = gm.GetSettlement(gm.FindNearestSettlement(at));
			string name = "the town";
			if (s)
				name = s.m_sName;

			Place(e, RZ_Mission.LandPointNear(at, 0, 30), name);
		}

		// Ringside: a Fight Pit running near
		foreach (RZ_Mission m : gm.GetMissions().GetActive())
		{
			if (RZ_Mission_FightPit.Cast(m) && RZ_Util.DistXZ(m.m_vPos, e.m_vGiver) <= 1500)
			{
				e.m_vSubject2 = m.m_vPos;
				e.SetText("pit", RZ_Util.DirectionText(e.m_vGiver, m.m_vPos));
			}
		}

		e.m_sSubject = "nameday:" + gm.GetProfile(e.m_iAsker).m_sIdentity;
		return e.m_vSubject != vector.Zero || e.m_vSubject2 != vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		int which = RZ_RepVoice.CREW_LEGS;
		if (w.m_iIndex == 1)
			which = RZ_RepVoice.CREW_SORRY;

		RZ_RepVoice.CrewEvent(dealer.GetGM(), playerId, which);
	}
}
