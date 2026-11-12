//! The Champion cards (docs/ERRANDS.md, champions): people come because of your name. One per faction;
//! the Haven's is You're the One. Champion with the giver's faction. They pay nothing: a line, a
//! world record, a rumour. Errands give no nicknames and set no titles.

//------------------------------------------------------------------------------------------------
//! The Kid's Inspection (Bastion): a private who's heard of you wants you to look over his post before
//! the sergeant does. Pays nothing.
class RZ_Errand_KidsInspection : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "kids_inspection";
		m_sTitle = "The Kid's Inspection";
		m_sFamily = "champion";
		m_eAnchor = RZ_EErrandAnchor.BASTION_POST;
		m_iFaction = RZ_EFaction.MILITARY;
		Offer("\"You're the one from the board, aren't you. Would you look over my post? Before the sergeant does. If you say it's right, it's right.\"",
			"\"Private on the gate's been asking for you by what the board calls you. He wants his post looked over. By you, he says.\"");
		Over("\"The sergeant got there first. The private's doing press-ups. He's fine.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Look over his post";
		w.m_ePlace = RZ_EErrandPlace.GATE;
		w.m_fSeconds = 20;
		w.m_fRadius = 15;
		w.m_sStep = "Walk the private's post at the gate of {here} with him (20 s) and say what you see.";
		w.Done("He stands straighter for the rest of the shift. Nobody pays you for that, and nobody needs to.",
			"\"Arc of fire, check. Sandbags, check.\" He writes your name in the margin of his notebook, small.");

		w = Way("talk");
		w.m_sLabel = "Tell him you're nobody's sergeant";
		w.Done("\"That's what the good ones say.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return dealer.GetGM().GetTier(playerId, RZ_EFaction.MILITARY) == RZ_ETier.CHAMPION;
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		e.m_sSubject = "holding:" + e.m_iHolding.ToString() + ".inspection";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iIndex != 0 || !dealer.GetGM().GetFacts())
			return;

		dealer.GetGM().GetFacts().AddRumour(e.Fill("A private at {here} tells anyone who'll listen that his post was looked over by someone off the board."), 12);
	}
}

//------------------------------------------------------------------------------------------------
//! A Story at the Fire (the Red Hand): the crew want to hear it from you, the one they talk about.
//! Tell it straight, tell it bigger, or don't. Pays nothing; a rumour.
class RZ_Errand_StoryAtTheFire : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "story_at_the_fire";
		m_sTitle = "A Story at the Fire";
		m_sFamily = "champion";
		m_eAnchor = RZ_EErrandAnchor.REDHAND_CAMP;
		m_iFaction = RZ_EFaction.BANDITS;
		Offer("\"The lads have a bet on whether what they say about you is true. Sit at the fire and settle it. Loser buys. Nobody's buying.\"",
			"\"You're the one they talk about. Come and tell it yourself, so we can stop telling it wrong.\"");
		Over("\"Fire's out. They told it without you. You were taller in it.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Tell it straight";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 60;
		w.m_fRadius = 15;
		w.m_sStep = "Sit at the fire at {here} and tell it the way it happened (a minute).";
		w.m_iEffect = 1;
		w.Done("\"That's worse than what we had. Better, I mean.\" Somebody owes somebody a tin.");

		w = Way("do");
		w.m_sLabel = "Tell it bigger";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 60;
		w.m_fRadius = 15;
		w.m_sStep = "Sit at the fire at {here} and give them the version they want (a minute).";
		w.m_iEffect = 2;
		w.Done("\"Forty of them. With a spoon.\" Nobody believes it. Everybody repeats it.");

		w = Way("talk");
		w.m_sLabel = "Say there's nothing to tell";
		w.Done("\"Modest. That's new. We'll put that in too.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return dealer.GetGM().GetTier(playerId, RZ_EFaction.BANDITS) == RZ_ETier.CHAMPION;
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		e.m_sSubject = "holding:" + e.m_iHolding.ToString() + ".fire";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_WorldFacts facts = dealer.GetGM().GetFacts();
		if (!facts || w.m_iEffect == 0)
			return;

		if (w.m_iEffect == 1)
			facts.AddRumour(e.Fill("The crews at {here} have a story about someone off their own board. It's mostly true, which they find disappointing."), 12);
		else
			facts.AddRumour(e.Fill("The crews at {here} say someone they know cleared a town with a spoon. They say it every night now."), 12);
	}
}

//------------------------------------------------------------------------------------------------
//! The Visitors' Log (Halcyon): the field lead wants your name in the station's log, for the record.
//! Sign it, or ask to be put down as anonymous. A world record; pays nothing.
class RZ_Errand_VisitorsLog : RZ_ErrandCard
{
	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		m_sKey = "visitors_log";
		m_sTitle = "The Visitors' Log";
		m_sFamily = "champion";
		m_eAnchor = RZ_EErrandAnchor.HALCYON_STATION;
		m_iFaction = RZ_EFaction.INSTITUTE;
		Offer("\"The visitors' log. Would you sign it? For the record. Records outlive everyone in them. That's rather the point of them.\"",
			"\"A small request: your name in the station's log, in your own hand. My brother saw it on the list at the lab. I'd like it here as well.\"");
		Over("\"The log's gone to the lab for binding. Another time.\"");

		RZ_ErrandWay w = Way("do");
		w.m_sLabel = "Sign the log";
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 10;
		w.m_fRadius = 15;
		w.m_sStep = "Sign the visitors' log at {here} (10 s).";
		w.m_iEffect = 1;
		w.Done("She blots it. \"Entry four hundred and twelve. The first one we've had to underline.\"");

		w = Way("talk");
		w.m_sLabel = "Ask to be put down as anonymous";
		w.Done("\"Anonymous is a valid entry. It's the most common one, in fact.\"");
	}

	//------------------------------------------------------------------------------------------------
	override bool ForWhom(RZ_Errands dealer, int playerId, RZ_NamedPerson giver)
	{
		return dealer.GetGM().GetTier(playerId, RZ_EFaction.INSTITUTE) == RZ_ETier.CHAMPION;
	}

	//------------------------------------------------------------------------------------------------
	override bool Why(RZ_Errands dealer, RZ_Errand e)
	{
		e.m_sSubject = "holding:" + e.m_iHolding.ToString() + ".log";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		if (w.m_iEffect != 1)
			return;

		array<int> who = {playerId};
		RZ_WorldFacts.Get().Memory().Record("errand.visitors_log", e.m_sSubject, "signed", RZ_WorldMemory.SOURCE_ERRAND, false, who,
			e.Fill("{here}'s visitors' log, entry four hundred and twelve, underlined."), "board");
	}
}
