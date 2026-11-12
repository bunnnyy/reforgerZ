//! Spot the Tells (docs/ERRANDS.md, disguises): two or more different players were caught in Bastion
//! kit near this post in the last 2 season days. Now a Red Hand scout in Bastion kit stands among the
//! post's people. Find the scout by the tells (the wrong boots, an AK, no cap): a Soldier or Police
//! background always does; anyone else two times in three, else Private Hollis spends the night in the
//! stockade. Or take the scout's 10 Red Hand Favor to point at Hollis (35% it comes out: +5 Bastion
//! Infamy).
class RZ_Errand_SpotTheTells : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "spot_the_tells";
		m_sTitle = "Spot the Tells";
		m_sFamily = "bastion";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		Offer("Two civilians in our green this week, and now the sergeant says one of ours doesn't look like one of ours. Walk the line. Look at boots, rifles, caps.",
			"Somebody here is wearing our kit and isn't ours. The sergeant can't tell. Can you?");
		Over("Never mind. The sergeant counted heads and found one too many. It's sorted.");

		RZ_ErrandWay w = Way("talk");
		w.m_sLabel = "Find the scout by the tells";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 20;
		w.m_fRadius = 30;
		w.m_sStep = "Talk to the three on duty at {here} and look at their kit (20 s there), then tell {giver} which one.";
		w.Done("\"That one?\" A look at the boots. \"That one.\"");

		w = Way("lie");
		w.m_sLabel = "Take the scout's money, point at Hollis";
		w.m_iMinTier = RZ_ETier.KNOWN;
		w.m_iTierFaction = RZ_EFaction.BANDITS;
		w.m_iPayFaction = RZ_EFaction.BANDITS;
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 20;
		w.m_fRadius = 30;
		w.m_sStep = "Talk to the three on duty at {here}. One of them presses Favor into your hand and nods at Private Hollis (20 s there). Then tell {giver}.";
		w.m_iFavor = 10;
		w.Done("\"Hollis? Hollis has been here since Day 1.\" A long look at the boy's boots.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		return gm.GetHostility() && gm.GetHostility().GetDisguise().IsOn() && RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.MILITARY), RZ_ETier.KNOWN);
	}

	//------------------------------------------------------------------------------------------------
	//! Different players caught in Bastion kit within 2.5 km in the last 2 season days: one friend's
	//! cheap catch can't open it.
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_DisguiseMarks marks = dealer.GetGM().GetHostility().GetDisguise().GetMarks();
		if (marks.DistinctCaught(RZ_EFaction.MILITARY, e.m_vGiver, 2500, 2) < 2)
			return false;

		e.m_sSubject = "tells:" + RZ_SettlementScanner.GridReference(e.m_vGiver);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		array<int> who = {playerId};
		if (w.m_iIndex == 1)
		{
			said.Insert("Private Hollis spends the night in the stockade for wearing the wrong boots. He's furious. He's also, technically, out of uniform.");
			if (Math.RandomFloat01() < 0.35)
			{
				gm.AddInfamy(playerId, RZ_EFaction.MILITARY, 5, "sold Hollis to the stockade", RZ_RepFlag.ACT);
				dealer.Notify(e, "The scout talked in the stockade. Bastion knows who pointed at Hollis. +5 Bastion Infamy.");
			}

			RZ_WorldFacts.Get().Memory().Record("errand.spot_the_tells", e.m_sSubject, "hollis", RZ_WorldMemory.SOURCE_ERRAND, false, who);
			RZ_Log.Info(string.Format("Errands: spot_the_tells sold to the scout by %1", RZ_Util.PlayerName(playerId)));
			return;
		}

		bool right = RZ_QuestApproach.HasBackground(gm, playerId, "soldier,police") || Math.RandomFloat01() < 0.67;
		if (!right)
		{
			said.Insert("Private Hollis spends the night in the stockade for wearing the wrong boots. He's furious. He's also, technically, out of uniform.");
			RZ_WorldFacts.Get().Memory().Record("errand.spot_the_tells", e.m_sSubject, "wrong", RZ_WorldMemory.SOURCE_ERRAND, false, who);
			return;
		}

		said.Insert("The scout goes to the stockade with the wrong boots on. The sergeant writes it up as a uniform inspection.");
		dealer.SmallFavor(playerId, RZ_EFaction.MILITARY, 5);
		dealer.SmallFame(playerId, RZ_EFaction.MILITARY, 2);
		RZ_WorldFacts.Get().Memory().Record("errand.spot_the_tells", e.m_sSubject, "found", RZ_WorldMemory.SOURCE_ERRAND, false, who);
	}
}
