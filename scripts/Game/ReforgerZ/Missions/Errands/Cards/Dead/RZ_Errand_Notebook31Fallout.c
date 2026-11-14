//! Notebook 31 after Lost Colleague (docs/ERRANDS.md, the dead; fallout; the nail list's Notebook 31 is RZ_Errand_Notebook31): a Halcyon researcher died where he
//! hid (Lost Colleague's fallout). His numbered coat and notebook are where he fell. Return both (the
//! authors' list gets his name), sell the notebook to the fence (a lie the rumour round finds), or keep
//! it: its number is a real cover detail for one Halcyon look, and the coat is no good to anyone.
class RZ_Errand_Notebook31Fallout : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "notebook31";
		m_sTitle = "Notebook 31";
		m_sFamily = "dead";
		m_eAnchor = RZ_EErrandAnchor.HALCYON_STATION;
		m_iFaction = RZ_EFaction.INSTITUTE;
		Offer("We lost a colleague: {who}, {subject_place}. Hid there and didn't come out. Notebook thirty-one is still with the body, and the numbered coat. The notebook belongs to the lab, and the name belongs on the authors' list. A small request: bring both back.",
			"We'd like {who}'s notebook back, and the name on the authors' list, where it belongs. The body is {subject_place}. Every notebook is read into the record. This one is the only copy.");
		Over("Someone returned the notebook. The name goes on the authors' list at the change of shift.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Bring back the coat and the notebook";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_sStep = "Find the body {subject_place} and take the coat and notebook thirty-one (15 s there), then bring them to {giver}.";
		w.Work(RZ_EWork.TAKE, "Take the coat and notebook");
		w.m_eProp = RZ_EErrandProp.EFFECTS;
		w.m_iFavor = 5;
		w.m_iFame = 2;
		w.Done("Thirty-one. The handwriting gets worse towards the end. They always do. Thank you. It goes into the record tonight.");

		w = Way("lie");
		w.m_sLabel = "Sell the notebook to the fence";
		w.m_iMinTier = RZ_ETier.KNOWN;
		w.m_iTierFaction = RZ_EFaction.BANDITS;
		w.m_iPayFaction = RZ_EFaction.BANDITS;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_sStep = "Take the notebook from the body {subject_place} for the fence (15 s). Then tell {giver} it wasn't there.";
		w.Work(RZ_EWork.TAKE, "Take the notebook");
		w.m_eProp = RZ_EErrandProp.EFFECTS;
		w.m_iFavor = 6;
		w.Done("Not there. I see. I'll write 'notes lost' in the record.");

		w = Way("do");
		w.m_sLabel = "Keep the notebook";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_sStep = "Take the notebook from the body {subject_place} (15 s) and keep it. Tell {giver} what you like.";
		w.Work(RZ_EWork.TAKE, "Take the notebook");
		w.m_eProp = RZ_EErrandProp.EFFECTS;
		w.Done("You've kept it. Of course you have. It has a number on the cover, and numbers open doors at our gate.");
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
		said.Insert("The number's on the cover. At a Halcyon gate, a number like that is something to say.");
	}
}
