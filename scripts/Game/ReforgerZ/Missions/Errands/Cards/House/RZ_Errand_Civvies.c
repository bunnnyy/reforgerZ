//! Civvies (docs/ERRANDS.md, help across lines; re-angled): the Deserter wanderer is resting within
//! 1.5 km of the house, struck off Bastion's strength as KIA and his name still on the board. The
//! disguise in reverse: he wants to stop looking like a soldier. Fetch him civilian clothes from the next
//! town, or sell where he sleeps to the crews (he's gone by morning; a lie the rumour round finds).
class RZ_Errand_Civvies : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "civvies";
		m_sTitle = "Civvies";
		m_sFamily = "house";
		m_eAnchor = RZ_EErrandAnchor.HAVEN_HOUSE;
		m_iFaction = RZ_EFaction.SURVIVORS;
		Offer("\"{name}'s sleeping in our shed again. Still in their green. Every time a truck passes he goes white. Find him something else to wear. Do us a turn.\"",
			"\"There's a soldier who isn't one any more, near the house. He'd like to look like nobody. There'll be clothes in {place}.\"");
		Over("\"He's gone on. In his green. Every truck on that road will see it.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Find him clothes";
		w.m_ePlace = RZ_EErrandPlace.NEXT_TOWN;
		w.m_fSeconds = 20;
		w.m_fRadius = 60;
		w.m_sStep = "Find civilian clothes in a house at {place}, nothing with a faction's colours (20 s there), then bring them to {giver}.";
		w.m_iFavor = 4;
		w.m_iFame = 2;
		w.Done("\"He's put them on. He looks like a farmer. A very tidy farmer. Thank you.\"");

		w = Way("lie");
		w.m_sLabel = "Sell where he sleeps to the crews";
		w.m_iPayFaction = RZ_EFaction.BANDITS;
		w.m_iFavor = 6;
		w.Done("\"Clothes? I'll ask around.\" He's gone by morning. Nobody says where.");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Wanderers wanderers = dealer.GetGM().GetWanderers();
		if (!wanderers)
			return false;

		array<int> living = {};
		wanderers.GetLiving(RZ_Wanderers.RoleBit(RZ_EWandererRole.DESERTER), living);
		foreach (int id : living)
		{
			vector pos;
			if (!wanderers.GetPosition(id, pos) || RZ_Util.DistXZ(pos, e.m_vGiver) > 1500)
				continue;

			e.m_iValue = id;
			e.SetText("name", wanderers.GetName(id));
			e.m_sSubject = RZ_WorldFacts.Wanderer(id) + ".civvies";
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (w.m_iIndex == 1)
		{
			if (gm.GetWanderers())
				gm.GetWanderers().TakeOffRoad(e.m_iValue);

			gm.GetFacts().Memory().Lies().Tell(gm, playerId, "errand.civvies", e.m_sSubject, RZ_EFaction.SURVIVORS, e.m_sGiver, e.m_vGiver, RZ_ELieTest.RUMOUR, 6,
				e.Fill("Word comes round from the crews' side: {name} was sold for six favours, and the house knows who by."));
			return;
		}

		array<int> who = {playerId};
		RZ_WorldFacts.Get().Memory().Record("errand.civvies", e.GetText("name"), "civvies", RZ_WorldMemory.SOURCE_ERRAND, true, who);
		said.Insert("What happens to him now is his own story. If anyone takes his tag to a post, it's Bastion's.");
	}
}
