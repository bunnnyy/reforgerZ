//! Notebook 31 (docs/ERRANDS.md, the dead; His Notebook merged in): a Halcyon researcher died where he
//! hid (Lost Colleague's fallout). His numbered coat and notebook are where he fell. Return both (the
//! authors' list gets his name), sell the notebook to the fence (a lie the rumour round finds), or keep
//! it: its number is a real cover detail for one Halcyon look, and the coat is no good to anyone.
class RZ_Errand_Notebook31 : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "notebook31";
		m_sTitle = "Notebook 31";
		m_sFamily = "dead";
		m_eAnchor = RZ_EErrandAnchor.HALCYON_STATION;
		m_iFaction = RZ_EFaction.INSTITUTE;
		Offer("\"{who}. Notebook thirty-one is still with him, {subject_place}. And his coat. The coat is numbered. A small request: bring both.\"",
			"\"We'd like {who}'s notebook back. And him on the authors' list, where he belongs. He's {subject_place}.\"");
		Over("\"Someone returned the notebook. His name goes on the list at the change of shift.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Bring back the coat and the notebook";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_sStep = "Find {who} {subject_place} and take the coat and the notebook (15 s there), then bring them to {giver}.";
		w.m_iFavor = 5;
		w.m_iFame = 2;
		w.Done("\"Thirty-one. The handwriting gets worse towards the end. They always do. Thank you.\"");

		w = Way("lie");
		w.m_sLabel = "Sell the notebook to the fence";
		w.m_iMinTier = RZ_ETier.KNOWN;
		w.m_iTierFaction = RZ_EFaction.BANDITS;
		w.m_iPayFaction = RZ_EFaction.BANDITS;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_sStep = "Take the notebook off {who} {subject_place} (15 s), sell it at the fence, and tell {giver} it wasn't there.";
		w.m_iFavor = 6;
		w.Done("\"Not there. I see. I'll put him down as 'notes lost'.\"");

		w = Way("do");
		w.m_sLabel = "Keep the notebook";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_sStep = "Take the notebook off {who} {subject_place} (15 s) and keep it. Tell {giver} what you like.";
		w.Done("\"You've kept it. Of course you have. It has a number. Numbers open doors here.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_Fallout fo = dealer.FalloutFor(m_sKey, e.m_vGiver, e.m_iAsker);
		if (!fo)
			return false;

		e.m_bFallout = true;
		e.m_iCap = fo.cap;
		Place(e, RZ_Util.GroundPos(fo.at), "");
		e.m_sSubject = "notebook31:" + RZ_SettlementScanner.GridReference(e.m_vSubject);
		e.SetText("who", fo.who);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		array<int> who = {playerId};
		if (w.m_iIndex == 0)
		{
			RZ_WorldFacts.Get().Memory().Record("errand.notebook31", e.GetText("who"), "returned", RZ_WorldMemory.SOURCE_ERRAND, true, who,
				e.Fill("{who} is on the paper as an author, deceased. Notebook thirty-one is in the lab's case, open at the last page."), "board");
			return;
		}

		if (w.m_iIndex == 1)
		{
			gm.GetFacts().Memory().Lies().Tell(gm, playerId, "errand.notebook31", e.m_sSubject, RZ_EFaction.INSTITUTE, e.m_sGiver, e.m_vSubject, RZ_ELieTest.RUMOUR, 5,
				e.Fill("A notebook numbered thirty-one turned up on a fence's table, priced in doses. The lab remembers who said it wasn't there."));
			return;
		}

		RZ_Deeds.AddLifeRecord(gm.GetProfile(playerId), "notebook31", false);
		said.Insert("Its number is on the cover. At Halcyon's gate, a number like that is something to say.");
	}
}
