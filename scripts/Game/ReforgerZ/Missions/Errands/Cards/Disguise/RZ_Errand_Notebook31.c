//! Notebook 31 (docs/ERRANDS.md, the dead; merged with His Notebook and Into the Record): one of
//! Halcyon's people died this season within 2 km (the nail list, RZ_DisguiseMarks) and the numbered
//! coat and notebook are still out there. For anyone. Return both (+2 Halcyon Fame; read into the
//! record, the name on the authors' list at the change of shift); sell the notebook to the fence (15
//! Red Hand Favor, 35% found out: +5 Halcyon Infamy); or keep it: its number is a real cover detail
//! at one Halcyon look. The coat is a dead man's either way. Never to the player who killed them.
class RZ_Errand_Notebook31 : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "notebook_31";
		m_sTitle = "Notebook 31";
		m_sFamily = "dead";
		m_eAnchor = RZ_EErrandAnchor.HALCYON_STATION;
		m_iFaction = RZ_EFaction.INSTITUTE;
		Offer("{dead} did not come back from {where_place}. Notebook {number} was in the coat. The notes are the only copy. Please.",
			"We have a coat and a notebook unaccounted for, {where_place}. Number {number}. And {dead}, of course. Mostly {dead}.");
		Over("Notebook {number} has been returned. The record is complete, for once.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Bring the coat and notebook back";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_sStep = "Find {dead} {where_place} and take the coat and notebook {number} (15 s there), then bring them to {giver}.";
		w.m_eProp = RZ_EErrandProp.EFFECTS;
		w.m_iFame = 2;
		w.Done("Notebook {number}. It goes into the record tonight, and {dead} goes on the authors' list.");

		w = Way("do");
		w.m_sLabel = "Sell the notebook to the fence";
		w.m_iMinTier = RZ_ETier.KNOWN;
		w.m_iTierFaction = RZ_EFaction.BANDITS;
		w.m_iPayFaction = RZ_EFaction.BANDITS;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_sStep = "Find {dead} {where_place}, take notebook {number} (15 s there), and sell it at the fence. Then tell {giver} it was gone.";
		w.m_eProp = RZ_EErrandProp.EFFECTS;
		w.m_iFavor = 15;
		w.Done("Gone. Then the work is gone. We will start again. We are good at that.");

		w = Way("lie");
		w.m_sLabel = "Keep the notebook";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_sStep = "Find {dead} {where_place} and pocket notebook {number} (15 s there). Then tell {giver} it was gone.";
		w.m_eProp = RZ_EErrandProp.EFFECTS;
		w.Done("\"Gone. I see.\" A pause. \"If you ever do see it, the number is {number}.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		if (!gm.GetHostility() || !gm.GetHostility().GetDisguise().IsOn())
			return false;

		array<RZ_NailSave> dead = {};
		gm.GetHostility().GetDisguise().GetMarks().OutThere(RZ_EFaction.INSTITUTE, e.m_vGiver, 2000, 9999, dead);
		string ident = gm.GetProfile(e.m_iAsker).m_sIdentity;
		foreach (RZ_NailSave n : dead)
		{
			if (n.killer == ident)
				continue;

			Place(e, RZ_Util.GroundPos(Vector(n.x, 0, n.z)), "");
			e.m_sSubject = "nail:" + n.id.ToString();
			e.m_iValue = n.id;
			e.SetText("dead", n.name);
			e.SetText("number", RZ_DisguiseMarks.CoatNumber(n).ToString());
			e.SetText("where_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool Still(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_NailSave n = dealer.GetGM().GetHostility().GetDisguise().GetMarks().FindNail(e.m_iValue);
		return n && n.how < RZ_DisguiseMarks.HOW_HOME;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		RZ_Disguise disguise = gm.GetHostility().GetDisguise();
		RZ_DisguiseMarks marks = disguise.GetMarks();
		RZ_NailSave n = marks.FindNail(e.m_iValue);
		array<int> who = {playerId};
		switch (w.m_iIndex)
		{
			case 0:
			{
				marks.SetFate(e.m_iValue, RZ_DisguiseMarks.HOW_HOME);
				RZ_WorldFacts.Get().Memory().Record("errand.notebook_31", e.m_sSubject, "returned", RZ_WorldMemory.SOURCE_ERRAND, true, who,
					"A shelf of numbered notebooks in the lab, one with a different hand at the end.", "prop");
				break;
			}
			case 1:
			{
				marks.SetFate(e.m_iValue, RZ_DisguiseMarks.HOW_LOST);
				if (Math.RandomFloat01() < 0.35)
				{
					gm.AddInfamy(playerId, RZ_EFaction.INSTITUTE, 5, "sold a dead researcher's notebook", RZ_RepFlag.ACT);
					dealer.Notify(e, e.Fill("Notebook {number} turned up on the Red Hand's channel, read aloud for laughs. The lab knows who carried it. +5 Halcyon Infamy."));
				}

				RZ_WorldFacts.Get().Memory().Record("errand.notebook_31", e.m_sSubject, "sold", RZ_WorldMemory.SOURCE_ERRAND, false, who);
				break;
			}
			case 2:
			{
				marks.SetFate(e.m_iValue, RZ_DisguiseMarks.HOW_LOST);
				disguise.GiveNotebook(playerId, RZ_DisguiseMarks.CoatNumber(n));
				said.Insert(e.Fill("Notebook {number} is in your pocket. At one Halcyon look, the number will do some talking for you."));
				RZ_WorldFacts.Get().Memory().Record("errand.notebook_31", e.m_sSubject, "kept", RZ_WorldMemory.SOURCE_ERRAND, false, who);
				break;
			}
		}
	}
}
