//! Spot the Tells (docs/ERRANDS.md, Bastion): Known with Bastion, disguises on, a Red Hand camp within
//! 2 km. A crew scout in Bastion kit stands the gate with the post's own. Watch the gate and name him by
//! his tells (wrong boots, the wrong rifle, no cap), or take the scout's money to point at Private Hollis
//! (a lie found when Hollis is let out and the scout's still on the gate).
class RZ_Errand_SpotTheTells : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "spot_tells";
		m_sTitle = "Spot the Tells";
		m_sFamily = "bastion";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		Offer("\"One of the lads on the gate isn't one of the lads. I'd bet my rations. Watch them for me. You've got fresh eyes.\"",
			"\"Somebody on that gate's wearing our green and walking like the crews. Find him. Quietly.\"");
		Over("\"Never mind. We found him. Boots. It's always the boots.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Watch the gate, and name him";
		w.m_ePlace = RZ_EErrandPlace.GATE;
		w.m_fSeconds = 45;
		w.m_fRadius = 20;
		w.m_sStep = "Watch the gate at {place} (45 s there), then tell {giver} who.";
		w.m_iFavor = 5;
		w.m_iFame = 2;
		w.Done("\"Him? The one with the AK? Christ. Yes. Take him round the back.\"");

		w = Way("do");
		w.m_sLabel = "Pick him out fast";
		w.m_sBackground = "police,excon";
		w.m_ePlace = RZ_EErrandPlace.GATE;
		w.m_fSeconds = 15;
		w.m_fRadius = 20;
		w.m_sStep = "Pick him out at the gate {place} (15 s there), then tell {giver}.";
		w.m_iFavor = 5;
		w.m_iFame = 2;
		w.Done("\"That fast? Remind me never to play cards with you.\"");

		w = Way("lie");
		w.m_sLabel = "Take his money, point at Hollis";
		w.m_iPayFaction = RZ_EFaction.BANDITS;
		w.m_iFavor = 6;
		w.Done("\"Hollis? Hollis. Wrong boots, now you mention it. Lock him up.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Hostility hostility = gm.GetHostility();
		if (!hostility || !hostility.GetDisguise() || !hostility.GetDisguise().IsOn())
			return false;

		return RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.MILITARY), RZ_ETier.KNOWN);
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Holding camp = HoldingNear(dealer.GetGM(), RZ_EFaction.BANDITS, e.m_vGiver, 2000);
		if (!camp)
			return false;

		e.m_sSubject = "holding:" + e.m_iHolding.ToString() + ".gate";
		e.SetText("subject_place", camp.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (w.m_iIndex == 2)
		{
			gm.GetFacts().Memory().Lies().Tell(gm, playerId, "errand.spot_tells", e.m_sSubject, RZ_EFaction.MILITARY, e.m_sGiver, e.m_vGiver, RZ_ELieTest.VISIT, 5,
				"Private Hollis is out of the stockade, furious, and the man you didn't name is still on the gate in the wrong boots.");
			said.Insert("Private Hollis spends the night in the stockade for wearing the wrong boots. He's furious. He's also, technically, out of uniform.");
			return;
		}

		said.Insert("The scout goes to the stockade in his borrowed green. The crews will be short a pair of eyes, and Bust Out has a name to come for.");
	}
}
