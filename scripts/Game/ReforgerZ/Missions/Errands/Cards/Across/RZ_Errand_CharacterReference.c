//! Character Reference (docs/ERRANDS.md, help across lines): you're Known with two factions, and one
//! asks what the other is really like. Your answer becomes a rumour their people repeat. Say what
//! you've seen (it follows the other side's real state), or speak well or ill of them whatever the
//! truth: then the answer comes back to you later (a callback).
class RZ_Errand_CharacterReference : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "character_reference";
		m_sTitle = "Character Reference";
		m_sFamily = "across";
		m_eAnchor = RZ_EErrandAnchor.ANY_HOLDING;
		Offer("\"You go to {other}. What are they actually like? Not the radio. You.\"",
			"\"Off the record. {other}. You've been in with them. Are they people we could deal with, or not?\"");
		Over("\"Never mind. We've made our minds up. Probably wrongly.\"");

		RZ_ErrandWay w = Way("talk");
		w.m_sLabel = "Say what you've seen";
		w.m_iFavor = 3;
		w.Done("\"Huh. That's not what I expected. That's probably why it's true.\"");

		w = Way("talk");
		w.m_sLabel = "Speak well of them";
		w.Done("\"Really? Well. If you say so.\"");

		w = Way("talk");
		w.m_sLabel = "Speak ill of them";
		w.Done("\"I knew it. I always knew it.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return RZ_Factions.IsValid(giver.m_iFaction) && RZ_Reputation.IsAtLeast(dealer.GetGM().GetTier(playerId, giver.m_iFaction), RZ_ETier.KNOWN);
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		for (int f = 0; f < 4; f++)
		{
			if (f == e.m_iFaction || !RZ_Reputation.IsAtLeast(gm.GetTier(e.m_iAsker, f), RZ_ETier.KNOWN))
				continue;

			e.m_iValue = f;
			e.SetText("other", RZ_Factions.GetNameInText(f));
			e.m_sSubject = "reference:" + e.m_iFaction.ToString() + "." + f.ToString();
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_GameModeComponent gm = dealer.GetGM();
		string other = RZ_Factions.GetNameInText(e.m_iValue);
		string mine = RZ_Factions.GetShortName(e.m_iFaction);
		string rumour;
		if (w.m_iIndex == 1)
		{
			rumour = string.Format("%1 people say %2 are decent, if you catch them on a good day. Someone who'd know told them.", mine, other);
		}
		else if (w.m_iIndex == 2)
		{
			rumour = string.Format("%1 people say %2 aren't to be trusted. Someone who's been inside told them so.", mine, other);
		}
		else
		{
			RZ_FactionLife life = gm.GetFactionLife();
			float morale = 50;
			if (life && life.IsActive())
				morale = life.GetMorale(e.m_iValue);

			if (morale < 40)
				rumour = string.Format("Word is %1 are frightened and short, and frightened people bite. Someone who's seen it told %2.", other, mine);
			else
				rumour = string.Format("Word is %1 are holding up, mostly. Someone who's seen it told %2.", other, mine);
		}

		if (gm.GetFacts())
			gm.GetFacts().AddRumour(rumour, 12);

		// An answer that wasn't what you'd seen comes back to you
		RZ_Shared shared = gm.GetShared();
		if (w.m_iIndex != 0 && shared)
			shared.GetCallbacks().Queue(playerId, e.m_sGiverTitle, string.Format("\"You told us what %1 are like. We went and looked. You owe us a better answer.\"", other), e.m_vGiver);
	}
}
