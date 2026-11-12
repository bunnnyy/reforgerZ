//! An opening's chain (docs/DESIGN.md, "Openings"): three steps on the errand engine
//! (Missions/Errands), handed straight to the player (RZ_Errands.Hand), so the tracker, the Quests
//! tab and the talk screen show them as they show any errand. One shape for every background:
//! 1. close by, quietly: search a house within 300 m and come back to the contact;
//! 2. a system: water from the nearest well, or something medical for the contact's hand;
//! 3. walk to where your background belongs (it ends there; nobody to go back to).
//! The steps pay nothing through the errand ledger: the chain pays its own Fame at the end
//! (RZ_Openings). Server only.

//------------------------------------------------------------------------------------------------
//! A step of the chain: its definition is the errand's, its done tells the opening.
class RZ_OpeningStepDef : RZ_ErrandDef
{
	int m_iStep;

	//------------------------------------------------------------------------------------------------
	void RZ_OpeningStepDef(string key, string title, int step)
	{
		m_sKey = key;
		m_sTitle = title;
		m_iStep = step;
		m_sFamily = "opening";
		m_bHanded = true;
		m_eAnchor = RZ_EErrandAnchor.WANDERER;
		Over("\"Never mind. It'll keep.\"");
	}

	//------------------------------------------------------------------------------------------------
	override void Resolve(RZ_Errands dealer, RZ_Errand e, RZ_ErrandWay w, int playerId, notnull array<string> said)
	{
		RZ_Openings openings = RZ_Openings.Get();
		if (openings)
			openings.OnStepDone(e);
	}
}

//------------------------------------------------------------------------------------------------
class RZ_OpeningChain
{
	static const string KEY_SEARCH = "opening.search";
	static const string KEY_WATER = "opening.water";
	static const string KEY_HAND = "opening.hand";
	static const string KEY_WALK = "opening.walk";

	ref RZ_OpeningStepDef m_Search;
	ref RZ_OpeningStepDef m_Water;
	ref RZ_OpeningStepDef m_Hand;
	ref array<ref RZ_OpeningStepDef> m_aWalk = {};		// by the ending faction (RZ_EFaction)

	//------------------------------------------------------------------------------------------------
	void RZ_OpeningChain()
	{
		m_Search = new RZ_OpeningStepDef(KEY_SEARCH, "Across the Road", 1);
		RZ_ErrandWay w = m_Search.Way("sneak");
		w.m_ePlace = RZ_EErrandPlace.SUBJECT;
		w.m_fSeconds = 20;
		w.m_fRadius = 10;
		w.m_bQuiet = true;
		w.m_sStep = "Search {place}, quietly. Then go back to {giver}.";
		w.Take("\"There's a house over the way I haven't dared. Go and look. Walk, don't run: they hear running.\"",
			"\"I need eyes on that house over there. Two days I've watched it. Nothing moves. That's what worries me.\"");
		w.Done("\"You came back. Most don't, from that side of the road.\"",
			"\"Anything? It doesn't matter. You went, and you came back. That's more than most manage.\"");

		m_Water = new RZ_OpeningStepDef(KEY_WATER, "Water", 2);
		RZ_ErrandWay ww = m_Water.Way("do");
		ww.m_ePlace = RZ_EErrandPlace.SUBJECT;
		ww.m_fSeconds = 15;
		ww.m_fRadius = 8;
		ww.m_sStep = "Fill what you can carry at {place}. Then bring it back to {giver}.";
		ww.Take("\"We're down to the last cup. There's a well. You know the one. Fill whatever you can carry.\"",
			"\"I haven't had a proper drink since Tuesday. The well's not far. I'd go myself, but you're quicker.\"");
		ww.Done("\"Clean enough. Boil it if you can. I can't, so I won't.\"",
			"\"Thank you. I mean it. I was starting to look at the rain barrel.\"");

		m_Hand = new RZ_OpeningStepDef(KEY_HAND, "The Cut", 2);
		RZ_ErrandWay wh = m_Hand.Way("give");
		wh.m_ePlace = RZ_EErrandPlace.HERE;
		wh.m_eItem = RZ_EErrandItem.MEDICAL;
		wh.m_iItems = 1;
		wh.m_bGiveThere = true;
		wh.m_fSeconds = 4;
		wh.m_fRadius = 6;
		wh.m_sStep = "Dress the cut on {giver}'s hand: a bandage, or anything medical you carry.";
		wh.Take("\"I cut my hand on the shutter getting in. It's gone hot. You've got something in that bag, haven't you?\"",
			"\"Look at this. A nail, getting in. You were always better at this than me.\"");
		wh.Done("\"That's better already. Or I'm telling myself it is. Either way.\"",
			"\"Neat. You've done that before. Thank you.\"");

		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			RZ_OpeningStepDef walk = new RZ_OpeningStepDef(KEY_WALK + "." + f.ToString(), RZ_OpeningText.WalkTitle(f), 3);
			walk.m_fLapseMinutes = 90;
			RZ_ErrandWay wk = walk.Way("do");
			wk.m_ePlace = RZ_EErrandPlace.SUBJECT;
			wk.m_fSeconds = 3;
			wk.m_fRadius = 40;
			wk.m_bEndsThere = true;
			wk.m_sStep = "Go to {place}.";
			wk.Take(RZ_OpeningText.SendOff(f));
			m_aWalk.Insert(walk);
		}
	}

	//------------------------------------------------------------------------------------------------
	RZ_OpeningStepDef Walk(int faction)
	{
		if (faction < 0 || faction >= m_aWalk.Count())
			return m_aWalk[RZ_EFaction.SURVIVORS];

		return m_aWalk[faction];
	}
}
