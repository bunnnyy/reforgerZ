//! The Name on the Tape (docs/ERRANDS.md, the dead; Bastion's "bring their effects home"): one of
//! Bastion's people died near a post this season day (the nail list, RZ_DisguiseMarks) and is still
//! out there. Bring the tag and jacket to the post: the tag on the nail, the name on the nightly call.
//! Or sell the jacket to the Red Hand's fence ("A name the army can't check twice"): the board lists
//! "kit lost". Never to the player who killed them (the arsonist rule); once per dead name.
class RZ_Errand_NameOnTheTape : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "name_on_tape";
		m_sTitle = "The Name on the Tape";
		m_sFamily = "dead";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		Offer("{dead} went out {where_place} this morning and didn't come back. One tag stays with the body, the other goes on the nail behind the radio, and right now the nail has a space. I can't spare anyone off the wire to fetch it.",
			"We've one of ours down {where_place}: {dead}. Bring the tag and the jacket in, so the name can be read on tonight's call, and the form is one page instead of four.");
		Over("Someone brought {dead}'s tag in. It's on the nail. Thank you all the same.");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Bring the tag and jacket home";
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_sStep = "Find the body of {dead} {where_place} and take the tag and the jacket (15 s there), then bring them to {giver}.";
		w.Work(RZ_EWork.TAKE, "Take the tag and jacket");
		w.m_eProp = RZ_EErrandProp.EFFECTS;
		w.m_iFavor = 4;
		w.m_iFame = 1;
		w.Done("Tag on the nail, jacket in the stores. The name goes on tonight's call, and the whole post will answer for it.",
			"That's the tag. The jacket's been through worse. So had {dead}, more than once.");

		w = Way("do");
		w.m_sLabel = "Sell the jacket to the fence";
		w.m_iMinTier = RZ_ETier.KNOWN;
		w.m_iTierFaction = RZ_EFaction.BANDITS;
		w.m_iPayFaction = RZ_EFaction.BANDITS;
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 15;
		w.m_fRadius = 40;
		w.m_sStep = "Find the body of {dead} {where_place} and take the jacket for the Red Hand's fence, who pays well for a name the army can't check twice (15 s there). Then tell {giver} it was gone.";
		w.Work(RZ_EWork.TAKE, "Take the jacket");
		w.m_eProp = RZ_EErrandProp.EFFECTS;
		w.m_iFavor = 10;
		w.Done("Gone? Somebody got there first. {dead} goes on the board with kit lost, and the nail stays empty.");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		return gm.GetHostility() && gm.GetHostility().GetDisguise().IsOn() && RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.MILITARY), RZ_ETier.KNOWN);
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		array<RZ_NailSave> dead = {};
		gm.GetHostility().GetDisguise().GetMarks().OutThere(RZ_EFaction.MILITARY, e.m_vGiver, 1500, 0, dead);
		string ident = gm.GetProfile(e.m_iAsker).m_sIdentity;
		foreach (RZ_NailSave n : dead)
		{
			// The arsonist rule: never to the one who killed them
			if (n.killer == ident)
				continue;

			Place(e, RZ_Util.GroundPos(Vector(n.x, 0, n.z)), "");
			e.m_sSubject = "nail:" + n.id.ToString();
			e.m_iValue = n.id;
			e.SetText("dead", n.name);
			e.SetText("where_place", RZ_Util.DirectionText(e.m_vGiver, e.m_vSubject));
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Still out there (nobody brought the tag home or sold the kit).
	override bool Still(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_NailSave n = dealer.GetGM().GetHostility().GetDisguise().GetMarks().FindNail(e.m_iValue);
		return n && n.how < RZ_DisguiseMarks.HOW_HOME;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_DisguiseMarks marks = dealer.GetGM().GetHostility().GetDisguise().GetMarks();
		array<int> who = {playerId};
		if (w.m_iIndex == 0)
		{
			marks.SetFate(e.m_iValue, RZ_DisguiseMarks.HOW_HOME);
			RZ_WorldFacts.Get().Memory().Record("errand.name_on_tape", e.m_sSubject, "home", RZ_WorldMemory.SOURCE_ERRAND, true, who,
				"Tags on the nail behind the radio, one for each name read on the call.", "prop");
			return;
		}

		marks.SetFate(e.m_iValue, RZ_DisguiseMarks.HOW_LOST);
		RZ_WorldFacts.Get().Memory().Record("errand.name_on_tape", e.m_sSubject, "kit lost", RZ_WorldMemory.SOURCE_ERRAND, true, who);
	}
}
